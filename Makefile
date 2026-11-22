# SPDX-License-Identifier: GPL-3.0-or-later
# compiler
CC ?= gcc
PKG_CONFIG ?= pkg-config

# the one version of the library: VERSION holds it, and `make version-check` holds the spec's Version, the newest
# debian/changelog entry and package.json's (the npm package of the reply shapes) to it
VERSION := $(strip $(shell cat VERSION))

# the library: soname libplugin-hostd-protocol.so.<major>, the file <major>.<minor>.<patch>. While the version is 0.x
# the soname is bumped on any ABI change, a field appended to host_backend_t included.
NAME = plugin-hostd-protocol
MAJOR = 0
LIB_SO = lib$(NAME).so
LIB_SONAME = $(LIB_SO).$(MAJOR)
LIB_FILE = $(LIB_SO).$(VERSION)

# install paths
PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include
DATADIR = $(PREFIX)/share

# the flags mod-host builds this code with, so the library behaves as libmod-host-protocol does
# -g: the library carries debug info, which abidw reads the types of abi/*.abi from (abi-check compares them)
CFLAGS += -g
CFLAGS += -O3 -Wall -Wextra -std=gnu99 -fPIC -D_GNU_SOURCE -pthread
CFLAGS += -Wno-deprecated-declarations
CFLAGS += -Werror=implicit-function-declaration -Werror=return-type
ifeq ($(DEBUG), 1)
   CFLAGS += -O0 -g -DDEBUG
endif
# omx-dsp's curve function (omx_curve.h, header only), which the curve verbs draw with: at or above the minor that
# declares it: the floor packaging/deps.txt declares, read back from the spec generated from it
OMXDSP_FLOOR := $(shell sed -n 's/^BuildRequires: *omx-dsp-devel *>= *\([0-9][0-9.]*\) *$$/\1/p' packaging/plugin-hostd-protocol.spec)
OMXDSP_CFLAGS := $(shell $(PKG_CONFIG) --cflags 'omxdsp >= $(OMXDSP_FLOOR)' 2>/dev/null)
# the shapes' targets read JSON and a generator, no header
ifeq ($(filter clean version-check deps deps-check test-reply-shapes render-shapes check-shapes-render test-shapes-sabotage test-ts-package test-ts-client test-no-epoch test-no-backcompat test-no-fedora44,$(MAKECMDGOALS)),)
ifeq ($(OMXDSP_CFLAGS),)
$(error no omx-dsp >= $(OMXDSP_FLOOR) for pkg-config (omx-dsp-devel / libomx-dsp-dev): the curve verbs draw with its omx_curve.h)
endif
endif
INCS = -Iinclude $(OMXDSP_CFLAGS)
LDFLAGS += -Wl,--no-undefined

SRC = src/socket.c src/protocol.c src/utils.c src/host-dispatch.c src/host-curve.c src/host-scenario.c
OBJ = $(SRC:.c=.o)
# the public headers, installed under <includedir>/plugin-hostd-protocol: the names libmod-host-protocol installs
# under <includedir>/mod-host, so a consumer changes only the package it asks pkg-config for
HEADERS = include/host-backend.h include/host-dispatch.h include/host-errors.h include/mod-host.h \
          include/host-scenario.h include/protocol.h include/socket.h include/utils.h

# the declared protocol (doc/protocol.md): plugin-hostd's protocol.h and the headers it includes, rendered by
# omx-contract from contract.d/plugin-hostd/plugin-hostd.json and from omx-clap's package and the domains of omx-clap and
# plugin-db in the contract directory; installed under
# <includedir>/plugin-hostd-protocol/plugin-hostd/, so a consumer includes <plugin-hostd/protocol.h> with this package's
# Cflags. src/protocol.c reads the protocol version, hello's line and the extensions hello grants from them.
RENDERED = include/plugin-hostd/plugin_hostd_protocol_host_verbs.h include/plugin-hostd/plugin_hostd_protocol_vocabulary.h \
           include/plugin-hostd/omx_clap_host_verbs.h include/plugin-hostd/omx_clap_vocabulary.h \
           include/plugin-hostd/plugin_db_vocabulary.h
# the probe domain's codes header (the render of the items of contract.d/probe marked x-c-codes), committed
PROBE_CODES = include/plugin-hostd/probe_codes.h
# the pin domain's codes header, the same way
PIN_CODES = include/plugin-hostd/pin_codes.h
# the estimate domain's codes header (the insert verdict's words), the same way
ESTIMATE_CODES = include/plugin-hostd/estimate_codes.h
PROTOCOL_HEADERS = include/plugin-hostd/protocol.h include/plugin-hostd/search_paths.h $(PROBE_CODES) $(PIN_CODES) $(ESTIMATE_CODES) $(RENDERED)

PC_LIBDIR = $(patsubst $(PREFIX)/%,$${prefix}/%,$(LIBDIR))
PC_INCLUDEDIR = $(patsubst $(PREFIX)/%,$${prefix}/%,$(INCLUDEDIR))
PC_DATADIR = $(patsubst $(PREFIX)/%,$${prefix}/%,$(DATADIR))

all: $(LIB_SO)

$(OBJ): $(HEADERS) $(PROTOCOL_HEADERS) src/session.h src/reply-words.h src/host-curve.h

src/%.o: src/%.c
	$(CC) $(INCS) $(CFLAGS) -c -o $@ $<

$(LIB_FILE): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -shared -Wl,-soname,$(LIB_SONAME) -lpthread -lm -o $@

$(LIB_SONAME): $(LIB_FILE)
	ln -sf $(LIB_FILE) $@

$(LIB_SO): $(LIB_SONAME)
	ln -sf $(LIB_SONAME) $@

# the protocol's Python packages into PYTHON3_SITELIB, both noarch: python3-plugin-hostd-client (plugin_hostd_client, the
# transport; issue 45) and python3-plugin-hostd-protocol (plugin_hostd_protocol, the codec of the declared commands and
# replies; issue 46). The codec's _installed.py is written here with where this package puts the files it reads, so the
# installed codec needs no -devel package to find them.
PYTHON3_SITELIB ?= $(shell python3 -c 'import sysconfig; print(sysconfig.get_path("purelib"))')
.PHONY: install-python install-python-client install-python-codec
install-python: install-python-client install-python-codec

install-python-client:
	install -d $(DESTDIR)$(PYTHON3_SITELIB)/plugin_hostd_client
	install -m 0644 clients/python/plugin_hostd_client/__init__.py $(DESTDIR)$(PYTHON3_SITELIB)/plugin_hostd_client/

install-python-codec:
	install -d $(DESTDIR)$(PYTHON3_SITELIB)/plugin_hostd_protocol
	install -m 0644 clients/python/plugin_hostd_protocol/__init__.py clients/python/plugin_hostd_protocol/__main__.py $(DESTDIR)$(PYTHON3_SITELIB)/plugin_hostd_protocol/
	printf '# GENERATED by make install-python: where this package put the files the codec reads\nTABLE = "%s"\nCONTRACT_DIRS = ("%s",)\nDATADIR = "%s"\nOMX_CONTRACT_DATADIR = "%s"\n' \
	    '$(DATADIR)/$(NAME)/plugin-hostd-protocol.json' '$(CONTRACTDIR)' '$(DATADIR)' '$(or $(shell $(PKG_CONFIG) --variable=datadir omx-contract 2>/dev/null),/usr/share/omx-contract)' \
	    > $(DESTDIR)$(PYTHON3_SITELIB)/plugin_hostd_protocol/_installed.py

install: $(LIB_FILE)
	install -d $(DESTDIR)$(LIBDIR)/pkgconfig $(DESTDIR)$(INCLUDEDIR)/$(NAME) $(DESTDIR)$(DATADIR)/$(NAME)
	install -m 755 $(LIB_FILE) $(DESTDIR)$(LIBDIR)/
	ln -sf $(LIB_FILE) $(DESTDIR)$(LIBDIR)/$(LIB_SONAME)
	ln -sf $(LIB_SONAME) $(DESTDIR)$(LIBDIR)/$(LIB_SO)
	install -m 644 $(HEADERS) $(DESTDIR)$(INCLUDEDIR)/$(NAME)/
	install -m 644 tests/host-scenarios.txt tests/host_scenarios.c doc/protocol.md $(DESTDIR)$(DATADIR)/$(NAME)/
	install -d $(DESTDIR)$(INCLUDEDIR)/$(NAME)/plugin-hostd
	install -m 644 $(PROTOCOL_HEADERS) $(DESTDIR)$(INCLUDEDIR)/$(NAME)/plugin-hostd/
	install -m 644 protocol/plugin-hostd-protocol.json $(DESTDIR)$(DATADIR)/$(NAME)/plugin-hostd-protocol.json
	install -m 644 grammar/plugin-hostd-protocol.abnf $(DESTDIR)$(DATADIR)/$(NAME)/plugin-hostd-protocol.abnf
	install -m 644 protocol/plugin-hostd.json $(DESTDIR)$(DATADIR)/$(NAME)/plugin-hostd.json
	install -m 644 protocol/plugin-hostd.schema.json $(DESTDIR)$(DATADIR)/$(NAME)/plugin-hostd.schema.json
	install -d $(DESTDIR)$(DATADIR)/$(NAME)/shapes
	install -m 644 shapes/*.schema.json $(DESTDIR)$(DATADIR)/$(NAME)/shapes/
	install -m 644 protocol/mod-host.json $(DESTDIR)$(DATADIR)/$(NAME)/mod-host.json
	install -m 644 tools/protocol-gen.c $(DESTDIR)$(DATADIR)/$(NAME)/protocol-gen.c
	install -m 644 clients/fixtures/codec.json $(DESTDIR)$(DATADIR)/$(NAME)/codec-fixtures.json
	sed -e 's,@PREFIX@,$(PREFIX),' -e 's,@LIBDIR@,$(PC_LIBDIR),' -e 's,@INCLUDEDIR@,$(PC_INCLUDEDIR),' \
	    -e 's,@DATADIR@,$(PC_DATADIR),' -e 's,@VERSION@,$(VERSION),' $(NAME).pc.in > $(DESTDIR)$(LIBDIR)/pkgconfig/$(NAME).pc
	chmod 644 $(DESTDIR)$(LIBDIR)/pkgconfig/$(NAME).pc

# ---------------------------------------------------------------------------------------------------------------------
# tests: none needs JACK, a plugin or a network beyond 127.0.0.1

TEST_CFLAGS = $(filter-out -O3,$(CFLAGS)) -O1 -g -Werror
# the sources are compiled into the sanitized tests as they are; a distribution's _FORTIFY_SOURCE flags their unchecked
# write() calls, which mod-host leaves unchecked too
SAN = -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -Wno-error=unused-result
TEST_PROGS = tests/feedback_lines tests/bounds_test tests/number_test tests/protocol_test tests/protocol_commands_test tests/host_scenarios tests/golden-run tests/fixes_test tests/forms_test \
             tests/hello_test tests/curve_test tests/monitor_test tests/runner_test \
             tests/socket-test tests/bind-test tests/adopt-test tests/utils-test

# the C documentation gate is doxygen (tools/Doxyfile over the headers): a warning that tools/doxygen.baseline does not
# list fails, and so does a listed one that is gone; the baseline only shrinks. Needs doxygen.
.PHONY: check-doxygen
check-doxygen:
	sh tools/doxygen_gate.sh

# ruff's own ratchet on the Python clients: D and the default rules (ruff.toml), today's findings carried by `# noqa`
# comments, so a new one is a plain ruff failure and fixing one deletes its noqa. Needs ruff.
.PHONY: check-ruff
check-ruff:
	ruff check clients/python

# the two gates only shrink, by mechanism (tools/doc_shrink.sh against the base branch: DOC_GATE_BASE, origin/main by
# default); tests/doc_gate_test.sh proves it, and that doxygen_gate.sh --update only lowers the baseline
.PHONY: check-doc-shrink test-doc-gates
check-doc-shrink:
	sh tools/doc_shrink.sh
test-doc-gates:
	sh tests/doc_gate_test.sh

test: test-protocol test-protocol-commands test-utils test-socket test-bind test-adopt test-golden test-golden-modhost \
      test-fixes test-fixes-sabotage test-forms test-forms-sabotage test-hello test-curve test-monitor test-runner test-runner-quiet test-hello-sabotage test-bind-sabotage test-install test-abi version-check \
      check-generated test-probe-words test-pin-words test-param-scales test-grouping-reasons test-estimate-words test-protocol-domain test-domain-renders check-protocol-doc test-feedback-args test-transient-codes test-search-paths test-property-rows test-reply-args test-verb-table test-mod-host-json test-generated-sabotage test-default-lv2-worker test-code-text test-client test-codec test-nul test-clap-core-range test-bounds test-number test-no-epoch test-no-backcompat test-no-fedora44 test-same-pass

# no workflow names Fedora 44 (the owner's ruling, 2026-11-19): tests/no_fedora44_test.py
test-no-fedora44:
	python3 tests/no_fedora44_test.py .

# no package of this source carries an epoch (the owner's ruling, 2026-11-18): tests/epoch_test.py
test-no-epoch:
	python3 tests/epoch_test.py .

# no backward-compatibility path is left in the tree (#109): tests/no-backcompat.sh
test-no-backcompat:
	sh tests/no-backcompat.sh .

# the release workflow reads the same-pass list as plugin-hostd does (.github/release-same-pass at the built commit)
test-same-pass:
	python3 tests/same_pass_test.py .

# every omx-clap-contract range admits the next omx-clap minor (the chain releases this protocol first)
test-clap-core-range:
	python3 tests/clap_core_range.py .

# the protocol, the dispatcher and the scenario corpus over a fake backend, as mod-host's test-protocol runs them
test-protocol: tests/protocol_test
	./tests/protocol_test

test-protocol-commands: tests/protocol_commands_test
	./tests/protocol_commands_test

tests/protocol_test tests/protocol_commands_test tests/host_scenarios: tests/%: tests/%.c $(OBJ)
	$(CC) $(INCS) $(TEST_CFLAGS) -o $@ $< $(OBJ) -lpthread -lm

# the input splitter, the NUL framing of the socket and the default bind address, with AddressSanitizer
test-utils: tests/utils-test
	./tests/utils-test
test-socket: tests/socket-test
	./tests/socket-test
test-bind: tests/bind-test
	./tests/bind-test
# one daemon on a port: tests/bind-sabotage.sh holds test-bind to failing when the rule is broken
test-bind-sabotage:
	sh tests/bind-sabotage.sh "$(CC)" "$(INCS) $(TEST_CFLAGS) $(SAN)"

# socket_adopt: the caller's bound socket served, a second bind on its port refused
test-adopt: tests/adopt-test
	./tests/adopt-test

tests/socket-test tests/bind-test tests/adopt-test: tests/%: tests/%.c src/socket.c include/socket.h
	$(CC) $< -std=gnu99 -D_GNU_SOURCE -O1 -g -Wall -Wextra -fsanitize=address -fno-omit-frame-pointer -Iinclude -lpthread -o $@
tests/utils-test: tests/utils-test.c src/utils.c include/utils.h
	$(CC) $< -std=gnu99 -D_GNU_SOURCE -O1 -g -Wall -Wextra -fsanitize=address -fno-omit-frame-pointer -Iinclude -lm -o $@

# The golden transcript (tests/golden.c): the scenario corpus and both fuzz seed corpora over a real socket, each
# command answered nine ways. test-golden holds the library to tests/golden/plugin-hostd-protocol.txt byte for byte;
# test-golden-modhost holds that file to tests/golden/mod-host-protocol.txt, what libmod-host-protocol 0.10.6-11.3
# answered: every exchange the same, except the ones tests/golden/deviations.txt names, which must differ.
GOLDEN_INPUTS = tests/host-scenarios.txt $(sort $(wildcard fuzz/corpus/protocol/*)) \
                $(addprefix dispatch:,$(sort $(wildcard fuzz/corpus/dispatch/*)))
GOLDEN_PORT ?= 15570

tests/golden-run: tests/golden.c $(LIB_SO)
	$(CC) $(INCS) $(TEST_CFLAGS) -o $@ $< -L. -l$(NAME) -Wl,-rpath,$(CURDIR) -lpthread -lm

test-golden: tests/golden-run
	@set -e; tmp=$$(mktemp); trap 'rm -f $$tmp' EXIT; \
	LC_ALL=C GOLDEN_PORT=$(GOLDEN_PORT) ./tests/golden-run $(GOLDEN_INPUTS) > $$tmp; \
	cmp $$tmp tests/golden/plugin-hostd-protocol.txt || { diff -u tests/golden/plugin-hostd-protocol.txt $$tmp | head -40; exit 1; }; \
	echo "golden: $$(grep -c '^#' $$tmp) exchanges, byte-identical to tests/golden/plugin-hostd-protocol.txt"

test-golden-modhost:
	python3 tests/golden-compare.py tests/golden/mod-host-protocol.txt tests/golden/plugin-hostd-protocol.txt \
	    tests/golden/deviations.txt

# rewrite our transcript after a deliberate change (and name each changed exchange in deviations.txt)
golden-update: tests/golden-run
	LC_ALL=C GOLDEN_PORT=$(GOLDEN_PORT) ./tests/golden-run $(GOLDEN_INPUTS) > tests/golden/plugin-hostd-protocol.txt

# mod-host's transcript, from the libmod-host-protocol that pkg-config finds (MODHOST_PREFIX=<its install prefix>),
# over the inputs MODHOST_INPUTS names (default: this tree's)
MODHOST_PREFIX ?=
MODHOST_INPUTS ?= $(GOLDEN_INPUTS)
golden-modhost:
	@set -e; tmp=$$(mktemp -d); trap 'rm -rf $$tmp' EXIT; \
	export PKG_CONFIG_PATH=$(if $(MODHOST_PREFIX),$(MODHOST_PREFIX)/lib/pkgconfig:)$$PKG_CONFIG_PATH; \
	$(CC) $$($(PKG_CONFIG) --cflags mod-host-protocol) $(TEST_CFLAGS) -o $$tmp/golden tests/golden.c \
	    $$($(PKG_CONFIG) --libs mod-host-protocol) $(if $(MODHOST_PREFIX),-Wl$(comma)-rpath$(comma)$(MODHOST_PREFIX)/lib) -lpthread -lm; \
	ldd $$tmp/golden | grep 'libmod-host-protocol'; \
	LC_ALL=C GOLDEN_PORT=$(GOLDEN_PORT) $$tmp/golden $(MODHOST_INPUTS) > tests/golden/mod-host-protocol.txt
comma := ,

# the fixes of plugin-hostd#371 and #372, and of #11 and #12 (tests/fixes_test.c), against the sources with
# AddressSanitizer and UndefinedBehaviorSanitizer
test-fixes: tests/fixes_test
	./tests/fixes_test

tests/fixes_test: tests/fixes_test.c $(SRC) $(HEADERS)
	$(CC) $(INCS) $(TEST_CFLAGS) $(SAN) -o $@ $< $(SRC) -lpthread -lm

# a verb of more than one form (the verb table's preset_show) runs the form its arguments fit (tests/forms_test.c), with
# AddressSanitizer and UndefinedBehaviorSanitizer; and the first-form behaviour put back must make it fail
test-forms: tests/forms_test
	./tests/forms_test

tests/forms_test: tests/forms_test.c $(SRC) $(HEADERS)
	$(CC) $(INCS) $(TEST_CFLAGS) $(SAN) -o $@ $< $(SRC) -lpthread -lm

test-forms-sabotage:
	sh tests/forms-sabotage.sh "$(CC)" "$(INCS) $(TEST_CFLAGS) $(SAN)"

# hello and request tags (tests/hello_test.c), over the library's own server and its feedback port, with
# AddressSanitizer and UndefinedBehaviorSanitizer: a client that never sends hello is answered as before
HELLO_PORT ?= 15590
test-hello: tests/hello_test
	HELLO_PORT=$(HELLO_PORT) ./tests/hello_test

# the scenario runner reads "not found" by the host's levels (tests/runner_test.c): a verb not required of the host is
# one it does not serve, a required one fails; test-hello-sabotage breaks this too
test-runner: tests/runner_test
	./tests/runner_test

# the demonstration asserts its refusal (an ok line naming it, no FAIL line), and an unexpected refusal still fails
# (tests/runner-quiet.sh, with its sabotage)
test-runner-quiet: tests/runner_test
	bash tests/runner-quiet.sh ./tests/runner_test

tests/runner_test: tests/runner_test.c $(SRC) $(HEADERS) $(PROTOCOL_HEADERS) src/session.h
	$(CC) $(INCS) $(TEST_CFLAGS) $(SAN) -o $@ $< $(SRC) -lpthread -lm

# a port of its own: under make -j it runs beside test-hello, and a port one daemon holds is refused to another
HELLO_SABOTAGE_PORT ?= 15594
test-hello-sabotage:
	HELLO_PORT=$(HELLO_SABOTAGE_PORT) sh tests/hello-sabotage.sh "$(CC)" "$(INCS) $(TEST_CFLAGS) $(SAN)"

tests/hello_test: tests/hello_test.c tests/test-client.h $(SRC) $(HEADERS) $(PROTOCOL_HEADERS) src/session.h
	$(CC) $(INCS) $(TEST_CFLAGS) $(SAN) -o $@ $< $(SRC) -lpthread -lm

# the curve verbs (tests/curve_test.c): curve_points is omx_curve_points over the values the instance holds, a binding
# kept, replaced, started again and dropped with its instance, each refusal by its code
CURVE_PORT ?= 15596
test-curve: tests/curve_test
	CURVE_PORT=$(CURVE_PORT) ./tests/curve_test

tests/curve_test: tests/curve_test.c tests/test-client.h $(SRC) $(HEADERS) $(PROTOCOL_HEADERS) src/host-curve.h
	$(CC) $(INCS) $(TEST_CFLAGS) $(SAN) -o $@ $< $(SRC) -lpthread -lm

# monitor_output_off (tests/monitor_test.c): a monitor ended once, one the connection did not set refused (-543), every
# monitor ended with the connection that set it, and an instance's gone with its remove
MONITOR_PORT ?= 15598
test-monitor: tests/monitor_test
	MONITOR_PORT=$(MONITOR_PORT) ./tests/monitor_test

tests/monitor_test: tests/monitor_test.c tests/test-client.h $(SRC) $(HEADERS) $(PROTOCOL_HEADERS) src/session.h
	$(CC) $(INCS) $(TEST_CFLAGS) $(SAN) -o $@ $< $(SRC) -lpthread -lm

# each fix taken out again in a scratch copy of the sources: fixes_test must fail on each
test-fixes-sabotage:
	sh tests/fixes-sabotage.sh "$(CC)" "$(INCS) $(TEST_CFLAGS) $(SAN)"

# install as a consumer meets it: links, soname, headers standing alone, the corpus, the .pc file, a consumer built
# from pkg-config alone, and mod-host's framing, command-table and bind checks against the installed library
test-install: $(LIB_FILE)
	sh tests/install-check.sh

# the exported symbols: the ones libmod-host-protocol.so.0 exports, no more and no fewer (abi/$(LIB_SONAME).symbols);
# with abidw and abidiff installed, abi-check compares the types as well against abi/$(LIB_SONAME).abi
ABI_SYMBOLS = abi/$(LIB_SONAME).symbols
ABI_BASELINE = abi/$(LIB_SONAME).abi
test-abi: $(LIB_FILE)
	@set -e; tmp=$$(mktemp); trap 'rm -f $$tmp' EXIT; \
	nm -D --defined-only $(LIB_FILE) | awk '$$2 ~ /^[TDBRVW]$$/ { print $$3 }' | LC_ALL=C sort > $$tmp; \
	diff -u $(ABI_SYMBOLS) $$tmp; echo "abi: $$(wc -l < $$tmp) exported symbols as $(ABI_SYMBOLS) lists"

abi-symbols: $(LIB_FILE)
	nm -D --defined-only $(LIB_FILE) | awk '$$2 ~ /^[TDBRVW]$$/ { print $$3 }' | LC_ALL=C sort > $(ABI_SYMBOLS)

abi-stage: $(LIB_FILE)
	rm -rf build/abi
	$(MAKE) install DESTDIR=$(CURDIR)/build/abi PREFIX=/usr LIBDIR=/usr/lib

abi-baseline: abi-stage
	abidw --headers-dir build/abi/usr/include/$(NAME) --out-file $(ABI_BASELINE) build/abi/usr/lib/$(LIB_FILE)

abi-check: abi-stage
	abidiff --headers-dir2 build/abi/usr/include/$(NAME) $(ABI_BASELINE) build/abi/usr/lib/$(LIB_FILE)

# ---------------------------------------------------------------------------------------------------------------------
# the declared protocol: the verb table, the JSON of protocol.h, mod-host's commands, the grammar and the protocol page,
# each written from the declarations by a tool of tools/, never by hand. `make gen` writes them, `make check-generated`
# and `make check-protocol-doc` fail on any drift.
#
#   protocol/plugin-hostd.json            tools/protocol-gen json, compiled against include/plugin-hostd/protocol.h
#   protocol/mod-host.json                tools/mod-host-json.py, from include/mod-host.h, protocol.h, host-errors.h
#   protocol/plugin-hostd-protocol.json   tools/protocol-table.py, the one table merged from the three above and
#                                         contract.d/plugin-hostd/plugin-hostd.json; it refuses a word of ours that changes a mod-host word
#   grammar/plugin-hostd-protocol.abnf    tools/abnf-gen.py, grammar/core.abnf and a rule per word of the table
#   fuzz/corpus-abnf/                     tools/abnf-corpus.py, seeds from the grammar (check-generated reads the
#                                         SHA-256 of the grammar they were written from)
#   src/reply-words.h                     tools/reply-words.py, the error word of each refusal number of the table
#   doc/overview.md                       the region host-verbs, tools/protocol-gen splice
#   doc/protocol.md                       tools/protocol-doc.py, from all of the above
#
# the parameter scales of protocol.h (PHD_PARAM_SCALES) are written by tools/param-scales-gen.py from omx-contract's domain
# param, read from the contract directory (OMX_CONTRACT_PATH, then the installed one): protocol-gen needs no omx-contract header.
GEN = build/protocol-gen

$(GEN): tools/protocol-gen.c $(PROTOCOL_HEADERS)
	@mkdir -p $(dir $@)
	$(CC) -Wall -Wextra -Werror -std=gnu99 -Iinclude -o $@ tools/protocol-gen.c

gen: $(GEN)
	$(GEN) json > protocol/plugin-hostd.json.new && mv protocol/plugin-hostd.json.new protocol/plugin-hostd.json
	python3 tools/mod-host-json.py include > protocol/mod-host.json.new && mv protocol/mod-host.json.new protocol/mod-host.json
	python3 tools/protocol-table.py > protocol/plugin-hostd-protocol.json.new && \
	    mv protocol/plugin-hostd-protocol.json.new protocol/plugin-hostd-protocol.json
	python3 tools/abnf-gen.py > grammar/plugin-hostd-protocol.abnf.new && \
	    mv grammar/plugin-hostd-protocol.abnf.new grammar/plugin-hostd-protocol.abnf
	python3 tools/reply-words.py > src/reply-words.h.new && mv src/reply-words.h.new src/reply-words.h
	python3 tools/ts-client-gen.py
	python3 tools/probe-words-gen.py
	python3 tools/pin-words-gen.py
	python3 tools/param-scales-gen.py
	python3 tools/estimate-words-gen.py
	$(MAKE) --no-print-directory render-domains
	$(GEN) splice doc/overview.md > doc/overview.md.new && mv doc/overview.md.new doc/overview.md
	$(MAKE) --no-print-directory protocol-doc

check-generated: $(GEN)
	@set -e; tmp=$$(mktemp -d); trap 'rm -rf $$tmp' EXIT; \
	$(GEN) json > $$tmp/json; cmp $$tmp/json protocol/plugin-hostd.json; \
	python3 tools/mod-host-json.py include > $$tmp/mod-host.json; cmp $$tmp/mod-host.json protocol/mod-host.json; \
	python3 tools/protocol-table.py --check; \
	python3 tools/abnf-gen.py --check; \
	python3 tools/reply-words.py --check; \
	python3 tools/ts-client-gen.py --check; \
	python3 tools/probe-words-gen.py --check; \
	python3 tools/param-scales-gen.py --check; \
	python3 tools/pin-words-gen.py --check; \
	python3 tools/estimate-words-gen.py --check; \
	$(MAKE) --no-print-directory check-domain-renders; \
	$(GEN) splice doc/overview.md > $$tmp/readme; cmp $$tmp/readme doc/overview.md; \
	sha256sum -c --quiet $(ABNF_CORPUS)/GRAMMAR.sha256 || { echo "$(ABNF_CORPUS) is not written from this grammar: run make fuzz-abnf-corpus"; exit 1; }; \
	$(GEN) regions > $$tmp/regions; test -s $$tmp/regions; \
	while read r; do grep -q "BEGIN GENERATED protocol:$$r\( .*\)\?$$" doc/overview.md || { echo "region $$r is in no file"; exit 1; }; done < $$tmp/regions; \
	echo "generated files are current"

protocol-doc:
	python3 tools/protocol-doc.py > doc/protocol.md.tmp && mv doc/protocol.md.tmp doc/protocol.md

check-protocol-doc:
	python3 tools/protocol-doc.py --check

# the host verb table of protocol.h has a row for every verb a controller sends, no more and no fewer; and each of its
# rules broken in a scratch copy of the header turns it red; the scenario list exercises every verb a console's host
# backend declares (tests/scenario_verbs.py)
# every feedback line of plugin-hostd's own declares its arguments by name and type in the table, and one line of each
# shape, printed through the daemon's own formats, decodes against them (plugin-hostd-protocol#17)
tests/feedback_lines: tests/feedback_lines.c $(PROTOCOL_HEADERS)
	$(CC) -Wall -Wextra -Werror -std=gnu99 -Iinclude -o $@ tests/feedback_lines.c

test-feedback-args: tests/feedback_lines
	./tests/feedback_lines > tests/feedback_lines.out
	python3 tests/feedback_args.py protocol/plugin-hostd-protocol.json tests/feedback_lines.out

# protocol.h's inline functions read no word past their count and write no byte past their size (tests/bounds_test.c),
# under AddressSanitizer: an array of exactly count words, a buffer of size 0 before a guard byte (#71, #72)
tests/bounds_test: tests/bounds_test.c $(PROTOCOL_HEADERS)
	$(CC) -Wall -Wextra -Werror -std=gnu99 -O1 -g $(SAN) -Iinclude -o $@ tests/bounds_test.c

test-bounds: tests/bounds_test
	./tests/bounds_test

# phd_number_in's grammar, one named arm a spelling: one spelling a value, the caller's range and no other, a word past
# the type refused (tests/number_test.c, #73); under the sanitizers, so an overflow in the reading is seen
tests/number_test: tests/number_test.c $(PROTOCOL_HEADERS)
	$(CC) -Wall -Wextra -Werror -std=gnu99 -O1 -g $(SAN) -Iinclude -o $@ tests/number_test.c

test-number: tests/number_test
	./tests/number_test

# every reply of this repository's verbs with a layout is typed and named in the table, and decodes (#19)
test-reply-args:
	python3 tests/reply_args.py protocol/plugin-hostd-protocol.json

# the protocol's Python client (clients/python/plugin_hostd_client, python3-plugin-hostd-client): NUL framing, split replies,
# timeout, close, and its newline sabotage (plugin-hostd-protocol issue 45: the owner ships the client)
.PHONY: test-client
test-client:
	python3 tests/client_test.py

# the Python client and codec refuse a NUL at their doors, and the sabotage that turns each off
.PHONY: test-nul
test-nul:
	python3 tests/nul_refusal_test.py
	sh tests/nul-sabotage.sh

# the protocol's codec (clients/python/plugin_hostd_protocol, python3-plugin-hostd-protocol) on its fixtures, reading the
# table of this tree and the vocabulary and catalogs this tree stages as install-contract installs them (issue 46)
.PHONY: test-codec
test-codec:
	@set -e; tmp=$$(mktemp -d); trap 'rm -rf $$tmp' EXIT; \
	$(MAKE) --no-print-directory install-contract DESTDIR=$$tmp >/dev/null; \
	PLUGIN_HOSTD_PROTOCOL_TABLE=protocol/plugin-hostd-protocol.json PLUGIN_HOSTD_PROTOCOL_DATADIR=$$tmp$(DATADIR) \
	  OMX_CONTRACT_PATH=$$tmp$(CONTRACTDIR):$(CONTRACTDIR) python3 tests/codec_test.py; \
	PLUGIN_HOSTD_PROTOCOL_TABLE=protocol/plugin-hostd-protocol.json PLUGIN_HOSTD_PROTOCOL_DATADIR=$$tmp$(DATADIR) \
	  OMX_CONTRACT_PATH=$$tmp$(CONTRACTDIR):$(CONTRACTDIR) python3 tests/client_codec_test.py; \
	PLUGIN_HOSTD_PROTOCOL_TABLE=protocol/plugin-hostd-protocol.json PLUGIN_HOSTD_PROTOCOL_DATADIR=$$tmp$(DATADIR) \
	  OMX_CONTRACT_PATH=$$tmp$(CONTRACTDIR):$(CONTRACTDIR) sh tests/client-codec-sabotage.sh

# which refusals are transient: a property of the code in contract.d/plugin-hostd/plugin-hostd.json, in the table and the rendered header (#23)
test-transient-codes:
	python3 tests/transient_codes.py protocol/plugin-hostd-protocol.json contract.d/plugin-hostd/plugin-hostd.json include/plugin-hostd/plugin_hostd_protocol_vocabulary.h

# the default search paths name the folders distributions install plugins in (plugin-hostd#82)
test-search-paths:
	python3 tests/search_paths_test.py

# the property verbs in the daemon's tables and the grammar (omx-clap-core#15)
test-property-rows:
	python3 tests/property_rows_test.py

test-verb-table:
	python3 tests/verb_table.py
	python3 tests/verb_table_sabotage.py
	python3 tests/scenario_verbs.py

# the default lv2_worker is omx-clap-host: mod-host runs an LV2 plugin only when lv2_worker or lv2_worker_set names it
test-default-lv2-worker:
	python3 tests/default_lv2_worker.py protocol/plugin-hostd.json

# mod-host.json follows its headers: a moved verb, code or message in a scratch copy moves the JSON
test-mod-host-json:
	python3 tests/mod_host_json.py include

# every code this repository owns has its source sentence, and its Catalan in catalogs/ca.json (tests/code_text.py); the installed
# omx-contract must be the schema that has `text`, and test-code-text-sabotage takes each rule out in a scratch copy
test-code-text:
	python3 tests/code_text.py contract.d/plugin-hostd/plugin-hostd.json protocol/plugin-hostd-protocol.json catalogs/ca.json $(wildcard $(shell $(PKG_CONFIG) --variable=datadir omx-contract 2>/dev/null)/omx-contract.schema.json)

test-code-text-sabotage:
	sh tests/code-text-sabotage.sh

# each generated file and each declaration it is written from, changed in a scratch copy of the tree: the checks above
# must fail on each (tests/generated-sabotage.sh)
test-generated-sabotage:
	sh tests/generated-sabotage.sh

# The checks that need tools a package build does not: run by CI (.github/workflows/ci.yml, job protocol).
#   check-rendered   the four rendered headers are what omx-contract renders from contract.d/plugin-hostd/plugin-hostd.json (staged as the
#                    package plugin-hostd-protocol of the contract directory) and from omx-clap's installed package;
#                    test-rendered-sabotage holds it to failing. A contract directory named first on OMX_CONTRACT_PATH
#                    outranks the installed one: CI stages omx-clap's data/ from its source there
#   check-contract-dir  that staged package validates with the installed contract directory
#   check-schema     protocol/plugin-hostd.json against its schema (python3-jsonschema)
#   test-grammar     the grammar against the scenario corpus, the golden transcript and the table (python abnf)
OMX_CONTRACT ?= omx-contract
PYTHON ?= python3
CONTRACT_SCHEMA_MAJOR = 3
# the domains this package's contract is (no data file: a domain-only owner): the words of the protocol, the probe's, the pin's
# and the estimate's
CONTRACT_DOMAINS = plugin-hostd probe pin estimate
CONTRACTDIR = $(or $(shell $(PKG_CONFIG) --variable=contractdir omx-contract 2>/dev/null),$(DATADIR)/omx-contract/contract.d)
INSTALL_CONTRACT_DIR = $(firstword $(wildcard $(shell $(PKG_CONFIG) --variable=datadir omx-contract 2>/dev/null)/install-contract-dir.sh) $(wildcard $(OMX_CONTRACT_DIR)/tools/install-contract-dir.sh))

check-protocol-tools: check-rendered test-rendered-sabotage check-schema test-grammar check-contract-dir test-abnf-corpus test-code-text-sabotage test-served-render-ci

# contract.d/plugin-hostd/plugin-hostd.json as the package plugin-hostd-protocol of omx-contract's contract directory,
# <contractdir>/<schema major>/plugin-hostd-protocol/, with the MANIFEST.json omx-contract's script writes
install-contract:
	@test -n "$(INSTALL_CONTRACT_DIR)" || { echo "install-contract: no install-contract-dir.sh: install omx-contract-devel, or set OMX_CONTRACT_DIR to a checkout" >&2; exit 1; }
	sh $(INSTALL_CONTRACT_DIR) --owner $(NAME) $(VERSION) $(CONTRACT_SCHEMA_MAJOR) $(DESTDIR)$(CONTRACTDIR) $(CONTRACT_DOMAINS)
	for d in $(CONTRACT_DOMAINS); do sh $(INSTALL_CONTRACT_DIR) --domain contract.d/$$d $(DESTDIR)$(CONTRACTDIR) || exit 1; done
	install -d $(DESTDIR)$(DATADIR)/$(NAME)/catalogs
	install -m 644 catalogs/*.json $(DESTDIR)$(DATADIR)/$(NAME)/catalogs/

# the staged package validates together with the installed contract directory; an absent omx-contract is red
check-contract-dir:
	@command -v $(OMX_CONTRACT) >/dev/null || { echo "$@: no $(OMX_CONTRACT) command: install omx-contract" >&2; exit 1; }
	@set -e; tmp=$$(mktemp -d); trap 'rm -rf $$tmp' EXIT; \
	$(MAKE) --no-print-directory install-contract DESTDIR=$$tmp >/dev/null; \
	OMX_CONTRACT_PATH=$$tmp$(CONTRACTDIR)$(if $(OMX_CONTRACT_PATH),:$(OMX_CONTRACT_PATH)):$(CONTRACTDIR) $(OMX_CONTRACT) validate --installed --catalogs catalogs

# the catalogs through omx-contract's release gate (translate --check, 3.16.0): every listed language has its catalog,
# no key is missing and no entry is stale. This tree's items name omx-clap's reply codes, so they are read from
# its contract staged as check-contract-dir stages it (#15).
check-catalogs:
	@set -e; tmp=$$(mktemp -d); trap 'rm -rf $$tmp' EXIT; \
	$(MAKE) --no-print-directory install-contract DESTDIR=$$tmp >/dev/null; \
	OMX_CONTRACT_PATH=$$tmp$(CONTRACTDIR)$(if $(OMX_CONTRACT_PATH),:$(OMX_CONTRACT_PATH)):$(CONTRACTDIR) $(OMX_CONTRACT) translate --check --installed --catalogs catalogs --package plugin-hostd-protocol

# a served contract package that changes the render turns main red when it is served: ci.yml's schedule (#27)
test-served-render-ci:
	python3 tests/served_render_ci.py

# check-rendered must fail on a rendered header changed by hand and on the domain plugin-hostd not rendered again
test-rendered-sabotage:
	sh tests/rendered-sabotage.sh

render-verbs check-rendered:
	@command -v $(OMX_CONTRACT) >/dev/null || { echo "$@: no $(OMX_CONTRACT) command: install omx-contract" >&2; exit 1; }
	@set -e; \
	OMX_CONTRACT_PATH=$(if $(OMX_CONTRACT_PATH),$(OMX_CONTRACT_PATH):)$(CONTRACTDIR) \
	  $(OMX_CONTRACT) render --target c --package $(NAME) --installed --with-domain contract.d/plugin-hostd --out include/plugin-hostd $(if $(filter check-rendered,$@),--check)
	@set -e; tmp=$$(mktemp -d); trap 'rm -rf $$tmp' EXIT; \
	wd=""; for n in host omx-clap; do for d in $$(printf %s "$(OMX_CONTRACT_PATH)" | tr : ' ') $(CONTRACTDIR); do \
	  if [ -f $$d/$$n/$$n.json ]; then wd="$$wd --with-domain $$d/$$n"; break; fi; done; done; \
	test -n "$$wd" || { echo "$@: no domain host or omx-clap of omx-clap's package in the contract directories" >&2; exit 1; }; \
	OMX_CONTRACT_PATH=$(if $(OMX_CONTRACT_PATH),$(OMX_CONTRACT_PATH):)$(CONTRACTDIR) \
	  $(OMX_CONTRACT) render --target c --package omx-clap --installed $$wd --out $$tmp; \
	for f in omx_clap_host_verbs.h omx_clap_vocabulary.h; do \
	  $(if $(filter check-rendered,$@),diff -u include/plugin-hostd/$$f $$tmp/$$f >&2 || { echo "include/plugin-hostd/$$f is not the render of omx-clap's package" >&2; exit 1; },cp $$tmp/$$f include/plugin-hostd/$$f); \
	done
	@set -e; tmp=$$(mktemp -d); trap 'rm -rf $$tmp' EXIT; \
	wd=""; for d in $$(printf %s "$(OMX_CONTRACT_PATH)" | tr : ' ') $(CONTRACTDIR); do \
	  if [ -f $$d/plugin-db/plugin-db.json ]; then wd="--with-domain $$d/plugin-db"; break; fi; done; \
	test -n "$$wd" || { echo "$@: no domain plugin-db of plugin-db's contract in the contract directories" >&2; exit 1; }; \
	OMX_CONTRACT_PATH=$(if $(OMX_CONTRACT_PATH),$(OMX_CONTRACT_PATH):)$(CONTRACTDIR) \
	  $(OMX_CONTRACT) render --target c --package plugin-db --installed $$wd --out $$tmp; \
	$(if $(filter check-rendered,$@),diff -u include/plugin-hostd/plugin_db_vocabulary.h $$tmp/plugin_db_vocabulary.h >&2 || { echo "include/plugin-hostd/plugin_db_vocabulary.h is not the render of plugin-db's package" >&2; exit 1; },cp $$tmp/plugin_db_vocabulary.h include/plugin-hostd/plugin_db_vocabulary.h); \
	echo "the rendered headers are current"

check-schema:
	python3 tests/protocol_schema.py protocol/plugin-hostd.json protocol/plugin-hostd.schema.json

# The reply shapes (doc/reply-shapes.md): shapes/reply_<verb>.schema.json, one per typed reply of the table, held to the
# table and to the replies themselves (test-reply-shapes, python3-jsonschema), and their TypeScript render, ts/shapes/,
# the one omx-contract's `render --target ts --shapes` writes with its pinned json-schema-to-typescript, committed and
# published as @openmixer/plugin-hostd-protocol. OMX_CONTRACT_DIR is a checkout of Open-Mixer/omx-contract with `npm ci`
# run in it (the generator is a dev dependency there, never an installed command).
#   test-reply-shapes     the schemas against the table, the golden transcript and a sample of every reply form
#   render-shapes         writes ts/shapes/ from shapes/
#   check-shapes-render   ts/shapes/ is what render-shapes writes, byte for byte
#   test-shapes-sabotage  each of the above is red on the fault it guards
#   test-ts-package       the packed npm package as a consumer imports it, compiled strict
test-reply-shapes:
	$(PYTHON) tests/reply_shapes.py protocol/plugin-hostd-protocol.json shapes tests/golden/plugin-hostd-protocol.txt

render-shapes check-shapes-render:
	@test -f "$(OMX_CONTRACT_DIR)/bin/omx-contract.mjs" || { echo "$@: set OMX_CONTRACT_DIR to a checkout of Open-Mixer/omx-contract with npm ci run in it" >&2; exit 1; }
	node "$(OMX_CONTRACT_DIR)/bin/omx-contract.mjs" render --target ts --shapes --shapes-dir shapes --out ts $(if $(filter check-shapes-render,$@),--check)

test-shapes-sabotage:
	PYTHON=$(PYTHON) MAKE=$(MAKE) sh tests/shapes-sabotage.sh

# npm pack, the tarball as a dependency, a strict TypeScript file importing the types by the package's exports
test-ts-package:
	sh tests/ts-package.sh

# the TypeScript client (doc/ts-client.md): the transport, the generated vocabulary, curvePoints; node, no dependency
test-ts-client:
	node --test tests/ts_client_test.mjs tests/ts_codec_vectors_test.mjs

# the table-driven TypeScript codec (clients/typescript/codec, the package's `.` export, moved from plugin-hostd's clients/ts):
# built with its locked toolchain, then run on clients/fixtures/codec.json, the file the Python codec runs on, reading the
# table of this tree and the vocabulary and catalogs this tree stages, as test-codec does. GroupingReason and RETRY_REASONS are
# held to the 16 golden reasons (clients/fixtures/grouping-reasons.txt)
.PHONY: test-ts-codec
test-ts-codec:
	@set -e; tmp=$$(mktemp -d); trap 'rm -rf $$tmp' EXIT; \
	$(MAKE) --no-print-directory install-contract DESTDIR=$$tmp >/dev/null; \
	npm --prefix clients/typescript/codec ci --no-audit --no-fund; npm --prefix clients/typescript/codec run build; \
	PLUGIN_HOSTD_PROTOCOL_TABLE=$(abspath protocol/plugin-hostd-protocol.json) PLUGIN_HOSTD_PROTOCOL_DATADIR=$$tmp$(DATADIR) \
	  OMX_CONTRACT_PATH=$$tmp$(CONTRACTDIR):$(CONTRACTDIR) npm --prefix clients/typescript/codec test

test-grammar:
	$(PYTHON) tests/grammar_test.py
	PYTHON=$(PYTHON) sh tests/grammar-sabotage.sh

# VERSION, the spec's Version and the newest debian/changelog entry say the same (the shared DEB workflow
# stamps an untagged build 0.1.0~git<sha> and a staged one <version>+git<sha>: only the leading version is compared)
version-check:
	@spec=$$(sed -n 's/^Version: *//p' packaging/$(NAME).spec); \
	deb=$$(sed -n '1s/^[^(]*(\([0-9][0-9.]*\).*/\1/p' debian/changelog); \
	npm=$$(sed -n 's/^  "version": *"\([^"]*\)".*/\1/p' package.json); \
	test "$$spec" = "$(VERSION)" && test "$$deb" = "$(VERSION)" && test "$$npm" = "$(VERSION)" \
	    || { echo "VERSION $(VERSION), spec $$spec, debian/changelog $$deb, package.json $$npm"; exit 1; }; \
	echo "version $(VERSION) in VERSION, the spec, debian/changelog and package.json"

# ---------------------------------------------------------------------------------------------------------------------
# libFuzzer harnesses (fuzz/README.md), built with clang: `make fuzz`, `make fuzz-run FUZZ_TIME=60`
FUZZ_CC        ?= clang
FUZZ_BUILD     ?= build/fuzz
FUZZ_TIME      ?= 60
FUZZ_HARNESSES := protocol dispatch scenario session
FUZZ_BINS      := $(FUZZ_HARNESSES:%=$(FUZZ_BUILD)/fuzz_%)
FUZZ_CFLAGS    := -O1 -g -std=gnu99 -D_GNU_SOURCE -fno-omit-frame-pointer -fsanitize=fuzzer,address,undefined \
                  -fno-sanitize-recover=undefined
FUZZ_SRCS      := src/protocol.c src/utils.c fuzz/socket_fuzz.c src/host-dispatch.c src/host-curve.c src/host-scenario.c

$(FUZZ_BUILD)/fuzz_%: fuzz/fuzz_%.c fuzz/fuzz.h $(FUZZ_SRCS)
	@mkdir -p $(FUZZ_BUILD)
	$(FUZZ_CC) $(FUZZ_CFLAGS) $(INCS) -o $@ $< $(FUZZ_SRCS) -lpthread -lm

fuzz: $(FUZZ_BINS)

fuzz-run: fuzz
	FUZZ_BUILD=$(FUZZ_BUILD) FUZZ_TIME=$(FUZZ_TIME) sh fuzz/run.sh $(FUZZ_HARNESSES)

fuzz-corpus:
	sh fuzz/gen-corpus.sh fuzz/corpus

# the committed seed corpus is what fuzz/gen-corpus.sh writes from the tree's own files (#33): a change to
# tests/host-scenarios.txt or the generator that is not followed by make fuzz-corpus is red in its own pull request
fuzz-corpus-check:
	@set -e; tmp=$$(mktemp -d); trap 'rm -rf $$tmp' EXIT; \
	sh fuzz/gen-corpus.sh $$tmp/corpus; \
	diff -r $$tmp/corpus fuzz/corpus || { echo "fuzz/corpus is not what fuzz/gen-corpus.sh writes: run make fuzz-corpus"; exit 1; }; \
	echo "the seed corpus is current"

# the seeds generated from the grammar (tools/abnf-corpus.py, python's abnf module): fuzz-run adds them to each
# harness's corpus. check-generated fails when the grammar changed since they were written; test-abnf-corpus (in
# check-protocol-tools) writes them again and compares
ABNF_CORPUS = fuzz/corpus-abnf
fuzz-abnf-corpus:
	$(PYTHON) tools/abnf-corpus.py $(ABNF_CORPUS)

test-abnf-corpus:
	$(PYTHON) tools/abnf-corpus.py --check $(ABNF_CORPUS)

# each fuzz/sabotage/ patch is a one-line bug the check_<property> it names must catch, and only that one
fuzz-sabotage:
	FUZZ_BUILD=$(FUZZ_BUILD) MAKE="$(MAKE)" sh fuzz/sabotage.sh

# every check's comment names a one-line change and the seed on which that check aborts, or why it is not a property;
# each change is applied alone to the plain harness (fuzz/records.sh)
fuzz-records:
	FUZZ_BUILD=$(FUZZ_BUILD) FUZZ_CC=$(FUZZ_CC) FUZZ_SRCS="$(FUZZ_SRCS)" FUZZ_INCS="$(INCS)" sh fuzz/records.sh

# the coverage witness: the same harnesses without sanitizers, instrumented for llvm-cov, each run once over its seed
# corpus (-runs=0 executes the corpus and nothing else), held to fuzz/coverage-floor.txt (fuzz/coverage.sh)
FUZZ_COV_BUILD := $(FUZZ_BUILD)/cov
FUZZ_COV_FLAGS := -O0 -g -std=gnu99 -D_GNU_SOURCE -fprofile-instr-generate -fcoverage-mapping
LLVM_COV       ?= llvm-cov
LLVM_PROFDATA  ?= llvm-profdata

$(FUZZ_COV_BUILD)/fuzz_%: fuzz/fuzz_%.c fuzz/fuzz.h $(FUZZ_SRCS)
	@mkdir -p $(FUZZ_COV_BUILD)
	$(FUZZ_CC) $(FUZZ_COV_FLAGS) -fsanitize=fuzzer $(INCS) -o $@ $< $(FUZZ_SRCS) -lpthread -lm

fuzz-coverage: $(FUZZ_HARNESSES:%=$(FUZZ_COV_BUILD)/fuzz_%)
	@set -e; for h in $(FUZZ_HARNESSES); do \
	  rm -f $(FUZZ_COV_BUILD)/$$h.profraw; \
	  LLVM_PROFILE_FILE=$(FUZZ_COV_BUILD)/$$h.profraw $(FUZZ_COV_BUILD)/fuzz_$$h -runs=0 fuzz/corpus/$$h/ \
	      >$(FUZZ_COV_BUILD)/$$h.log 2>&1 || \
	    { cat $(FUZZ_COV_BUILD)/$$h.log; echo "fuzz-coverage: fuzz_$$h failed on its own corpus"; exit 1; }; \
	  $(LLVM_PROFDATA) merge -sparse $(FUZZ_COV_BUILD)/$$h.profraw -o $(FUZZ_COV_BUILD)/$$h.profdata; \
	done
	@LLVM_COV=$(LLVM_COV) sh fuzz/coverage.sh $(FUZZ_COV_BUILD) $(FUZZ_HARNESSES)

clean:
	rm -rf src/*.o $(LIB_SO) $(LIB_SONAME) $(LIB_FILE) $(TEST_PROGS) build

.PHONY: test-no-fedora44 gen check-generated protocol-doc check-protocol-doc test-verb-table test-transient-codes test-search-paths test-property-rows test-mod-host-json test-generated-sabotage test-default-lv2-worker test-served-render-ci \
        check-protocol-tools install-contract check-contract-dir render-verbs check-rendered test-rendered-sabotage \
        check-schema test-grammar test-code-text test-code-text-sabotage test-reply-shapes render-shapes check-shapes-render test-shapes-sabotage test-ts-package test-ts-client
.PHONY: all install test test-protocol test-protocol-commands test-utils test-socket test-bind test-golden \
        test-golden-modhost golden-update golden-modhost test-fixes test-fixes-sabotage test-forms test-forms-sabotage test-hello test-runner test-hello-sabotage test-bind-sabotage test-install test-abi \
        abi-symbols abi-stage abi-baseline abi-check version-check fuzz fuzz-run fuzz-corpus fuzz-corpus-check fuzz-abnf-corpus test-abnf-corpus fuzz-sabotage fuzz-records fuzz-coverage clean

# The dependency floors are written once, in packaging/deps.txt; the spec and debian/control are generated from it with
# Open-Mixer/.github's deps-render. make deps writes them, make deps-check refuses drift (name a checkout of that
# repository: make deps-check DOTGITHUB=<dir>).
DOTGITHUB ?=
.PHONY: deps deps-check
deps:
	@test -n "$(DOTGITHUB)" || { echo "$@: name a checkout of Open-Mixer/.github: make $@ DOTGITHUB=<dir>" >&2; exit 2; }
	$(DOTGITHUB)/.github/actions/deps-render/render.sh --write .
deps-check:
	@test -n "$(DOTGITHUB)" || { echo "$@: name a checkout of Open-Mixer/.github: make $@ DOTGITHUB=<dir>" >&2; exit 2; }
	$(DOTGITHUB)/.github/actions/deps-render/render.sh --check .

# the conf keys declare no qualification directory: the daemon reads qualifications through libplugin-qualification-db
TEST_PROGS += tests/conf_keys_test
tests/conf_keys_test: tests/conf_keys_test.c $(PROTOCOL_HEADERS)
	$(CC) -Wall -Wextra -Werror -std=gnu99 -Iinclude -o $@ tests/conf_keys_test.c

test-conf-keys: tests/conf_keys_test
	./tests/conf_keys_test

test: test-conf-keys
.PHONY: test-conf-keys

# the probe words of protocol.h (PHD_PROBE_REASONS, PHD_PROBE_OUTCOMES, PHD_PROBE_STEPS) are generated from the package's
# domain probe (contract.d/probe), meanings included, and held to the 34 golden rows (tests/probe_words_test.py)
.PHONY: test-probe-words
test-probe-words:
	OMX_CONTRACT='$(OMX_CONTRACT)' CC='$(CC)' python3 tests/probe_words_test.py

# the pin words of protocol.h (PHD_PIN_VERDICT_*, PHD_PIN_POOL_*, PHD_PIN_VERDICTS, PHD_PIN_POOLS) are generated from the
# package's domain pin (contract.d/pin), meanings included, and held to the 10 golden rows (tests/pin_words_test.py)
.PHONY: test-pin-words
test-pin-words:
	OMX_CONTRACT='$(OMX_CONTRACT)' CC='$(CC)' python3 tests/pin_words_test.py

# the parameter scales of protocol.h (PHD_PARAM_SCALES) are generated from omx-contract's domain param, and held to the
# 3 golden rows (tests/param_scales_test.py); OMX_CONTRACT_PATH names the contract directories the domain is read from
# (tests/fixtures/contract.d until omx-contract 3.21.0 installs it)
.PHONY: test-param-scales
test-param-scales:
	CC='$(CC)' OMX_CONTRACT_PATH='$(CURDIR)/tests/fixtures/contract.d' python3 tests/param_scales_test.py

# GroupingReason and RETRY_REASONS of both clients are written by tools/grouping-reasons.py from plugin-hostd's domain
# plugin-hostd/grouping, and held to the 16 golden reasons (tests/grouping_reasons_test.py, which reads
# tests/fixtures/contract.d, the interface of the domain, until plugin-hostd installs it).
#   grouping-reasons         writes both blocks from the installed domain (OMX_CONTRACT_PATH, then the installed directory)
#   check-grouping-domain    both blocks are what the INSTALLED domain gives; not part of `make test`: plugin-hostd builds against
#                            this package, so the installed plugin-hostd-contract carries the domain only after plugin-hostd's release
.PHONY: test-grouping-reasons grouping-reasons check-grouping-domain
test-grouping-reasons:
	OMX_CONTRACT_PATH='$(CURDIR)/tests/fixtures/contract.d' python3 tests/grouping_reasons_test.py
grouping-reasons:
	python3 tools/grouping-reasons.py
check-grouping-domain:
	python3 tools/grouping-reasons.py --check

# the insert verdict of estimate (PHD_ESTIMATE_VERDICTS, PHD_ESTIMATE_REASONS, their words and patterns) is generated from
# the package's domain estimate (contract.d/estimate), meanings included, and the reply ends with the two words
# (tests/estimate_words_test.py)
# the words of the protocol are the domain plugin-hostd (contract.d/plugin-hostd), read from there by the tools and the codec, and
# data/verbs.json is gone (tests/protocol_domain_test.py)
.PHONY: test-protocol-domain
test-protocol-domain:
	python3 tests/protocol_domain_test.py

# the C renders of this tree's own domains pin, estimate and probe (include/plugin-hostd/{pin,estimate,probe}_codes.h):
# render-domains writes them, check-domain-renders holds them byte for byte (check-generated runs it). They read this
# tree's contract.d first, then the contract directories render-verbs reads (OMX_CONTRACT_PATH, then the installed
# one): the domain plugin-hostd names omx-clap's HOST_REPLY_CODES, so omx-clap's package must be installed beside it, at
# the floor the domain's info.x-requires states (omx-clap-contract, packaging/deps.txt; data only, the protocol <->
# omx-clap cycle of a chain is bootstrapped from the clap chain head's contract)
DOMAIN_RENDERS = pin estimate probe
.PHONY: render-domains check-domain-renders test-domain-renders
render-domains check-domain-renders:
	@command -v $(OMX_CONTRACT) >/dev/null || { echo "$@: no $(OMX_CONTRACT) command: install omx-contract" >&2; exit 1; }
	@set -e; for p in $(DOMAIN_RENDERS); do \
	  OMX_CONTRACT_PATH=$(abspath contract.d)$(if $(OMX_CONTRACT_PATH),:$(OMX_CONTRACT_PATH))$(if $(wildcard $(CONTRACTDIR)),:$(CONTRACTDIR)) \
	    $(OMX_CONTRACT) render --target c --package $$p --installed --out include/plugin-hostd $(if $(filter check-domain-renders,$@),--check); \
	done

# check-domain-renders resolves a reference of this tree's domains to an installed owner's domain (the domain
# plugin-hostd names omx-clap's HOST_REPLY_CODES), and refuses one whose owner is not installed by name
test-domain-renders:
	sh tests/domain-renders-test.sh

.PHONY: test-estimate-words
test-estimate-words:
	OMX_CONTRACT='$(OMX_CONTRACT)' CC='$(CC)' python3 tests/estimate_words_test.py
