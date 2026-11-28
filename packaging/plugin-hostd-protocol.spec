Name: plugin-hostd-protocol
Version: 0.21.5
Release: 1%{?dist}
License: GPL-3.0-or-later
Summary: The plugin-hostd protocol library, a superset of mod-host's text protocol
URL: https://github.com/Open-Mixer/plugin-hostd-protocol

Source0: %{url}/archive/v%{version}/%{name}-%{version}.tar.gz

BuildRequires: gcc
BuildRequires: make
BuildRequires: pkgconfig
BuildRequires: binutils
# make test: the splitter, framing and fix tests run under AddressSanitizer and UndefinedBehaviorSanitizer, and the
# golden comparison is a python script
BuildRequires: libasan
BuildRequires: libubsan
BuildRequires: python3
# %%{python3_sitelib}, where the Python client installs (python3-plugin-hostd-client)
BuildRequires: python3-devel
# No version is typed on a requirement (owner ruling 2026-11-24): the build installs the lower levels' published
# packages and builds against them; the relations between this spec's own packages are generated (= %{version}).
# the declared protocol: protocol.h's parameter scales are written from omx-contract's domain param (omx-contract-data), and
# install-contract writes the contract directory's package with omx-contract's script
BuildRequires: omx-contract-devel
# the codec's tests (make test-codec) read omx-contract's catalog reader
BuildRequires: omx-contract-data
# omx_curve.h, the curve function the curve verbs draw with (header only)
BuildRequires: omx-dsp-devel
# nodejs-plugin-hostd-protocol: the TypeScript codec is built with its locked toolchain (npm ci) and the npm package
# packed (npm pack); %%check imports the packed tree with node, its @openmixer/omx-contract the omx-contract package's
BuildRequires: nodejs
BuildRequires: npm
BuildRequires: omx-contract

# The JavaScript is run by node, never a compiled module: no requirement is generated from the Node tree
%global __requires_exclude_from ^%{_prefix}/lib/node_modules/.*$

%description
libplugin-hostd-protocol is the socket server, line protocol and command
dispatch that plugin-hostd and omx-clap-host answer their controllers with:
mod-host's text protocol, so any mod-host client works unchanged. A host
registers its plugin backend and the library listens on the command and
feedback ports, parses each command and sends the reply.
Its documentation is on openmixer.org.

%package devel
Summary: Build a host that answers the plugin-hostd protocol
Requires: %{name}%{?_isa} = %{version}-%{release}

%description devel
Headers, the pkg-config file, the scenario corpus and its runner of
libplugin-hostd-protocol. The headers have the names and the API of
libmod-host-protocol's, so a host built against mod-host-protocol changes only
the package name it asks pkg-config for.
Its documentation is on openmixer.org.

It also carries the declared protocol: plugin-hostd/protocol.h and the headers
it includes, the one verb table, the grammar of a line in ABNF, and the
protocol as JSON, each named by a pkg-config variable.

%package -n python3-plugin-hostd-client
Summary: The Python client of the plugin-hostd line protocol
BuildArch: noarch
Requires: python3
# command() and reply() speak through the codec, built by this spec at this version: a floor at its own version
Requires: python3-plugin-hostd-protocol = %{version}-%{release}
# plugin-hostd-tests before 0.21.0 shipped the same module under /usr/libexec/plugin-hostd/tests/lib (not on sys.path:
# no file conflict); it now requires this package

%description -n python3-plugin-hostd-client
plugin_hostd_client, the Python client of the plugin-hostd line protocol: a
command is one line ended by a NUL, a reply one NUL-ended line; send, try_send
(a timeout or a closed peer named, never raised) and expect. The transport
every test of plugin-hostd, omx-clap-host and the controllers imports; the
codec of the declared commands and replies is plugin_hostd_protocol.
Its documentation is on openmixer.org.

%package -n python3-plugin-hostd-protocol
Summary: The Python codec of the plugin-hostd protocol
BuildArch: noarch
# Its source is this repository since 0.5.10.
Requires: python3
Requires: plugin-hostd-protocol = %{version}-%{release}
Requires: plugin-hostd-protocol-contract = %{version}-%{release}
Requires: omx-contract
# omx-contract's catalog reader, omx_contract_catalog.py, which reads a catalog entry whatever its shape
Requires: omx-contract-data

%description -n python3-plugin-hostd-protocol
plugin_hostd_protocol, the Python codec of the plugin-hostd protocol: one codec
over the files plugin-hostd-protocol installs, which encodes and decodes every
command the protocol declares, parses the replies, feedback lines and protocol
errors, and gives each reply code its sentence. A controller speaks the
protocol through it and restates nothing; the npm package
@openmixer/plugin-hostd-client is the same codec in TypeScript. The transport
is plugin_hostd_client.
Its documentation is on openmixer.org.

%package -n nodejs-plugin-hostd-protocol
Summary: The TypeScript codec and client of the plugin-hostd protocol, for Node
BuildArch: noarch
Requires: nodejs
# the codec reads the table plugin-hostd-protocol installs and the domains of plugin-hostd-protocol-contract, built by
# this spec at this version
Requires: plugin-hostd-protocol = %{version}-%{release}
Requires: plugin-hostd-protocol-contract = %{version}-%{release}
# its @openmixer/omx-contract: node_modules/@openmixer/omx-contract is a link to the omx-contract package's tree
# (/usr/lib/omx-contract, the catalog reader), and the sentences read omx-contract's own domains (omx-contract-data)
Requires: omx-contract
Requires: omx-contract-data

%description -n nodejs-plugin-hostd-protocol
The npm package @openmixer/plugin-hostd-protocol as `npm pack` packs it,
installed under /usr/lib/node_modules/@openmixer/plugin-hostd-protocol, where
a Node program links its installed @openmixer packages from: the table-driven
TypeScript codec (the package's own export, Protocol), the thin client over it
(./client: the transport and curvePoints), the codec vectors
(./codec-vectors), the reply shapes as JSON Schema (./schemas/*) and as
TypeScript types (./shapes/*). The codec reads the files
plugin-hostd-protocol and plugin-hostd-protocol-contract install; its
@openmixer/omx-contract is the omx-contract package's tree, never a copy.
Its documentation is on openmixer.org.

%package contract
Summary: The plugin-hostd protocol's verbs in omx-contract's contract directory
BuildArch: noarch
Requires: omx-contract

%description contract
The domain plugin-hostd-protocol, the verbs and reply codes
plugin-hostd adds to mod-host's protocol, in omx-contract's contract directory,
under share/omx-contract/contract.d/plugin-hostd-protocol, beside its domains
host (the refusals, track kinds and feedback lines every host of the protocol
answers with), hosting-verb (the hosts a verb names), probe, pin and estimate.
Its documentation is on openmixer.org.

%prep
%autosetup -p1

%build
%set_build_flags
%make_build
# the TypeScript codec's dist/, which the npm package's own export is
make build-node

%install
%make_install PREFIX=%{_prefix} LIBDIR=%{_libdir} INCLUDEDIR=%{_includedir} DATADIR=%{_datadir}
# the domains of plugin-hostd-protocol (contract.d/plugin-hostd-protocol, host, hosting-verb, probe, pin, estimate), as omx-contract's contract directory holds them
%make_build install-contract DESTDIR=%{buildroot} PREFIX=%{_prefix}
# the Python packages, python3-plugin-hostd-client and python3-plugin-hostd-protocol
%make_build install-python DESTDIR=%{buildroot} PREFIX=%{_prefix} DATADIR=%{_datadir} PYTHON3_SITELIB=%{python3_sitelib}
# the npm package's packed tree, nodejs-plugin-hostd-protocol
make install-node DESTDIR=%{buildroot} PREFIX=%{_prefix} NODEDIR=%{_prefix}/lib/node_modules

%check
make test
# the packed tree as a Node consumer imports it: the bare name and ./client resolve, every codec vector is met
make test-node-installed PREFIX=%{_prefix}

%files
%license COPYING
%doc README.md
%{_libdir}/libplugin-hostd-protocol.so.0
%{_libdir}/libplugin-hostd-protocol.so.%{version}
# the declared protocol a program reads at run time: the verb table, the grammar, the protocol's JSON and mod-host's
%dir %{_datadir}/plugin-hostd-protocol
%{_datadir}/plugin-hostd-protocol/plugin-hostd-protocol.json
%{_datadir}/plugin-hostd-protocol/plugin-hostd-protocol.abnf
%{_datadir}/plugin-hostd-protocol/plugin-hostd.json
%{_datadir}/plugin-hostd-protocol/mod-host.json

%files devel
%license COPYING
%doc README.md
%{_libdir}/libplugin-hostd-protocol.so
%{_libdir}/pkgconfig/plugin-hostd-protocol.pc
%{_includedir}/plugin-hostd-protocol/
%{_datadir}/plugin-hostd-protocol/host-scenarios.txt
%{_datadir}/plugin-hostd-protocol/host_scenarios.c
%{_datadir}/plugin-hostd-protocol/protocol.md
%{_datadir}/plugin-hostd-protocol/plugin-hostd.schema.json
%{_datadir}/plugin-hostd-protocol/shapes/
%{_datadir}/plugin-hostd-protocol/protocol-gen.c
%{_datadir}/plugin-hostd-protocol/codec-fixtures.json

%files -n python3-plugin-hostd-client
%license COPYING
%{python3_sitelib}/plugin_hostd_client/

%files -n python3-plugin-hostd-protocol
%license COPYING
%{python3_sitelib}/plugin_hostd_protocol/

%files -n nodejs-plugin-hostd-protocol
%license COPYING
%dir %{_prefix}/lib/node_modules/@openmixer/
%{_prefix}/lib/node_modules/@openmixer/plugin-hostd-protocol/

%files contract
%license COPYING
%doc README.md
%dir %{_datadir}/omx-contract/contract.d/plugin-hostd-protocol/
%{_datadir}/omx-contract/contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json
%{_datadir}/omx-contract/contract.d/host/
%{_datadir}/omx-contract/contract.d/hosting-verb/
%{_datadir}/omx-contract/contract.d/probe/
%{_datadir}/omx-contract/contract.d/pin/
%{_datadir}/omx-contract/contract.d/estimate/
%dir %{_datadir}/plugin-hostd-protocol/catalogs

# BEGIN langpacks: generated by langpacks.py, do not edit
%package langpack-ca
Summary: The ca catalog of plugin-hostd-protocol's codes
BuildArch: noarch
Requires: plugin-hostd-protocol-contract = %{version}-%{release}
Supplements: (plugin-hostd-protocol-contract and langpacks-ca)

%description langpack-ca
The sentences of plugin-hostd-protocol's codes in the language ca, read by
every surface that shows them on a host in that locale; without it a surface
shows the English source sentence.

%files langpack-ca
%{_datadir}/plugin-hostd-protocol/catalogs/ca.json

%package langpack-es
Summary: The es catalog of plugin-hostd-protocol's codes
BuildArch: noarch
Requires: plugin-hostd-protocol-contract = %{version}-%{release}
Supplements: (plugin-hostd-protocol-contract and langpacks-es)

%description langpack-es
The sentences of plugin-hostd-protocol's codes in the language es, read by
every surface that shows them on a host in that locale; without it a surface
shows the English source sentence.

%files langpack-es
%{_datadir}/plugin-hostd-protocol/catalogs/es.json

%package langpack-fr
Summary: The fr catalog of plugin-hostd-protocol's codes
BuildArch: noarch
Requires: plugin-hostd-protocol-contract = %{version}-%{release}
Supplements: (plugin-hostd-protocol-contract and langpacks-fr)

%description langpack-fr
The sentences of plugin-hostd-protocol's codes in the language fr, read by
every surface that shows them on a host in that locale; without it a surface
shows the English source sentence.

%files langpack-fr
%{_datadir}/plugin-hostd-protocol/catalogs/fr.json

%package langpack-de
Summary: The de catalog of plugin-hostd-protocol's codes
BuildArch: noarch
Requires: plugin-hostd-protocol-contract = %{version}-%{release}
Supplements: (plugin-hostd-protocol-contract and langpacks-de)

%description langpack-de
The sentences of plugin-hostd-protocol's codes in the language de, read by
every surface that shows them on a host in that locale; without it a surface
shows the English source sentence.

%files langpack-de
%{_datadir}/plugin-hostd-protocol/catalogs/de.json

%package langpack-it
Summary: The it catalog of plugin-hostd-protocol's codes
BuildArch: noarch
Requires: plugin-hostd-protocol-contract = %{version}-%{release}
Supplements: (plugin-hostd-protocol-contract and langpacks-it)

%description langpack-it
The sentences of plugin-hostd-protocol's codes in the language it, read by
every surface that shows them on a host in that locale; without it a surface
shows the English source sentence.

%files langpack-it
%{_datadir}/plugin-hostd-protocol/catalogs/it.json

%package langpack-pt
Summary: The pt catalog of plugin-hostd-protocol's codes
BuildArch: noarch
Requires: plugin-hostd-protocol-contract = %{version}-%{release}
Supplements: (plugin-hostd-protocol-contract and langpacks-pt)

%description langpack-pt
The sentences of plugin-hostd-protocol's codes in the language pt, read by
every surface that shows them on a host in that locale; without it a surface
shows the English source sentence.

%files langpack-pt
%{_datadir}/plugin-hostd-protocol/catalogs/pt.json
# END langpacks

%changelog
* Sat Nov 28 2026 Pau Aliagas <linuxnow@gmail.com> - 0.21.5-1
- nodejs-plugin-hostd-protocol, a new noarch package: the npm package
  @openmixer/plugin-hostd-protocol as npm pack packs it, under
  /usr/lib/node_modules/@openmixer/plugin-hostd-protocol, its
  @openmixer/omx-contract a link to the omx-contract package's tree, so a
  Node consumer builds against the published package, never the registry's.
- The npm package no longer packs the codec's build tree
  (clients/typescript/codec/node_modules: typescript, ajv and the registry's
  @openmixer/omx-contract, which the codec read before its consumer's) nor
  its compiled tests.
- python3-plugin-hostd-protocol's description names the TypeScript package
  as it is, @openmixer/plugin-hostd-protocol, not @openmixer/plugin-hostd-client.

* Fri Nov 27 2026 Pau Aliagas <linuxnow@gmail.com> - 0.21.4-1
- release-same-pass no longer names plugin-hostd-full: plugin-hostd builds
  no such package and no channel serves it, so the release gate refused the
  name and the 0.21.3 tag release could not pass (#198).

* Fri Nov 27 2026 Pau Aliagas <linuxnow@gmail.com> - 0.21.3-1
- The debs are built with their generated floors, and the .changes and
  .buildinfo describe the debs as built; 0.21.1 and 0.21.2 reached staging
  as RPMs only.

* Fri Nov 27 2026 Pau Aliagas <linuxnow@gmail.com> - 0.21.2-1
- The debs are built with the generated floors in the built package only
  (the tree's debian/control keeps bare names, as tests/no_floors.py
  requires); 0.21.1 reached staging as RPMs only.

* Thu Nov 26 2026 Pau Aliagas <linuxnow@gmail.com> - 0.21.1-1
- The RPMs are built for Fedora aarch64 as well as x86_64: the release
  matrix is Open-Mixer/.github's shared releases and architectures, with no
  list of this repository's own.
- Published by the devel release of the full matrix, with every change
  merged to devel since 0.21.0.

* Wed Nov 25 2026 Pau Aliagas <linuxnow@gmail.com> - 0.21.0-1
- 0.21.0, above every version python3-plugin-hostd-protocol had when plugin-hostd
  built it (0.20.6): this source released it at 0.8.1, below it (#180).
- tools/served-floor.py, run on every pull request: the release refuses a version below the highest the
  channel ever served any of this source's packages at (#180).
- The domain hosting-verb carries its translations (contract.d/hosting-verb/
  catalogs, installed with it): the three HOSTING_VERB_HOSTS words moved from
  omx-clap-core's catalogs word for word (ruling R5), held by check-catalogs.
- plugin-db's package is no longer rendered here (#163): plugin_db_vocabulary.h,
  its render, its check and its install are gone, and the build requires no
  plugin-db package. The pin words are this package's domain pin (#164).
- One TypeScript encoder (#106): the npm package's ./client writes and reads
  its lines through the codec's Protocol, its command port and reply cap the
  table's constants (message_max, from SOCKET_MSG_MAX_SIZE, is one now); the
  ./lines and ./vocabulary exports, their files and tools/ts-client-gen.py are
  gone.
- V16 as ruled 2026-11-23: the protocol's words are the domain
  plugin-hostd-protocol, installed as
  contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json, every item named
  PLUGIN_HOSTD_PROTOCOL_* (REPLY_CODES, DAEMON_VERBS, LIBRARY_VERBS, EXTENSIONS);
  it no longer shares <contractdir>/plugin-hostd/ with plugin-hostd's package
  and nested domains. No C macro changes.
- The TypeScript codec reads the domains in one loop, as the Python codec.
- The build reads nothing of omx-clap-core (#67): what the two share is this
  package's. The domains host (HOST_REPLY_CODES, HOST_TRACK_KINDS, HOST_EVENTS)
  and hosting-verb (HOSTING_VERB_HOSTS) are installed with this contract, and
  the verbs omx-clap-host shares with plugin-hostd and the ones it adds are the
  items PLUGIN_HOSTD_PROTOCOL_SHARED_VERBS and
  PLUGIN_HOSTD_PROTOCOL_CLAP_HOST_VERBS of the domain plugin-hostd-protocol.
  Their macros are rendered into plugin_hostd_protocol_host_verbs.h and
  plugin_hostd_protocol_vocabulary.h; omx_clap_host_verbs.h and
  omx_clap_vocabulary.h are gone, and so is the omx-clap-contract dependency.
- The descriptions store's ask words (#128): add's optional last word
  ask | no-ask, the verb description_get <uri> answered
  <sha256> <lv2|clap> <uri> <base|system|user>, the event description_changed
  <uri>; -529 and -530 also answer description_get.
- The setting ask_unknown (#157): auto (the default), 1 or 0, a word of
  PHD_CONF_STRINGS with its words in PHD_ASK_UNKNOWN_MODES; the add word
  ask | no-ask overrides it for that add.


* Tue Nov 24 2026 Pau Aliagas <linuxnow@gmail.com> - 0.8.1-1
- tests: the empty-contract arms of param_scales_test and grouping_reasons_test
  hold the installed omx-contract out by PKG_CONFIG_PATH as well as
  PKG_CONFIG_LIBDIR, and check that they did, so %check passes on a box with
  omx-contract installed (#146). 0.8.0 never reached the rpm channel.

* Sat Nov 21 2026 Pau Aliagas <linuxnow@gmail.com> - 0.8.0-1
- The vocabulary sorting pass: the probe words (outcomes, reasons, steps, signals)
  and the pin verdict and pool words are domains of this package (probe, pin) and
  protocol.h generates its lists from them; the parameter scales are read from
  omx-contract's domain param; GroupingReason and RETRY_REASONS of both clients are
  written from plugin-hostd's domain plugin-hostd/grouping; the shared verbs and
  host words are read from omx-clap's domains, under the new omx-clap names.

* Fri Nov 20 2026 Pau Aliagas <linuxnow@gmail.com> - 0.7.0-1
- The backward-compatibility removal: the ERR_LV2_* spellings of the error
  words are gone from host-errors.h, and plan, plan_get and estimate answer
  with their one reply form only; their reply schemas require every word of it
  (#110). This removes API, hence a new minor.
- The qualify_requested event and the qualify spool: a plugin with no verdict
  added under require_pins with pin_mode first-use says qualify_requested with
  its uri, and one request for its quick qualification is written in the spool
  (#107).
- The qualification_dir conf key, its OMX_QUALIFICATION_DIR default and the
  subdirectory constant are dropped: plugin-hostd reads qualifications through
  libplugin-qualification-db, so the key has no reader (#112).
- The plugin-db words of the rendered vocabulary come from plugin-db 0.4.7's
  domain (#111); the plugin-db-contract floor stays 0.4.4.
- The table-constants check joins verb_table.py (#108).
- The Debian packages build on arm64 runners and Fedora 45 is the newest cell,
  Fedora 44 dropped; the ABI baseline carries the types, not only the symbols
  (#113).
- The commit identity check is on for pull requests (#115).
- The Breaks and Replaces of the ca language pack against plugin-hostd-
  protocol-contract (<< 0.5.6) are gone: floors only, no Breaks, Replaces or
  Conflicts between our packages.
- The soname stays libplugin-hostd-protocol.so.0: the exported symbols and the
  layout of every type are as in 0.6.0, the removed names were enumerators and
  macros that a consumer resolves when it compiles.

* Thu Nov 19 2026 Pau Aliagas <linuxnow@gmail.com> - 0.6.0-1
- Floors only, no ceilings, declared once in packaging/deps.txt; the spec and
  debian/control are generated from it and the floors check refuses drift
  (#64): omx-dsp-devel >= 0.32.4 (0.5.9 declared 0.32.1), omx-contract 3.19.1
  (0.5.9 declared 3.18.0: the release the headers are rendered with), and
  plugin-db 0.4.4 as before.
- python3-plugin-hostd-client, a new noarch package: the protocol's Python
  client ships from here (#47, #45).
- python3-plugin-hostd-protocol, the Python codec, ships from here (#48, #46).
- The grammar's quoted-escaped form, a quoted value that is text, for the
  property verbs (#39).
- The panel file declares its keys, meters among them (#51).
- The typed replies have a JSON Schema each, installed under shapesdir, and
  the TypeScript render is published as an npm package (#82, #81); the
  TypeScript client of the protocol, its transport, the generated vocabulary
  and curvePoints, ships in the same package (#87).
- The protocol schemas take https://openmixer.org/schema/plugin-hostd-
  protocol/ as their $id (#99).
- An untagged empty line, and a whitespace-only one, gets no reply and changes
  no state; a tagged one keeps its answer (#79, #83).
- phd_note_valid reads no word past count, phd_pin_path_decode writes nothing
  into a size of 0 (#74), and phd_number_in reads one spelling per number, in
  the caller's lo and hi only (#75).
- The command and feedback markers of mod-host.h that the generator reads are
  back (#80).
- The installed C headers document their public API with Doxygen, and the
  documentation gates only shrink (#62, #69, #76, #78).
- The curve families of omx-dsp 0.32.4: the delay and reverb curve families
  and the 37 roles curve_bind takes, with time_response, which a curve verb
  draws as omx-dsp does (#93).
- rpc_timeout_ms: a silent worker gets SIGTERM first, so it closes its JACK
  client, and SIGKILL only if it has not left two seconds later (#89).
- No epoch on any package of this source (#95, #97).
- CI: the heavy workflows run by dispatch and tags, the fast checks alone on a
  pull request (#43); each cell takes its cells from check-matrix (#37).

* Sat Nov 14 2026 Pau Aliagas <linuxnow@gmail.com> - 0.5.9-1
- Built against plugin-db >= 0.4.4 and below 0.5 and omx-dsp >= 0.32.1 and below
  0.33, the 3.18.0 pass's newest of each, with their ceilings (D30).
- CI renders against what the channel serves every day, the render check alone,
  and names the served contract packages when it fails (#27, #32).
- The fuzz seed corpus is held to its generator on every pull request, and the
  generator writes the same corpus in every locale (LC_ALL=C); the corpus and
  both golden transcripts are written again over it (#33, #34).
- The default CLAP search path holds the folders distributions install plugins
  in, /usr/local/lib/clap, /usr/lib64/clap and /usr/local/lib64/clap, after
  clap/entry.h's: a fresh plugin-hostd learns a first-use pin there
  (plugin-hostd#82, #36).

* Sat Nov 14 2026 Pau Aliagas <linuxnow@gmail.com> - 0.5.8-1
- A reply code declares itself transient: an identical request may succeed
  later with no change by the caller. worker-spawn, gave-up and verb-dropped
  are; pin-refused, a verdict about the plugin, is not. The table's reply_codes
  and the rendered header carry it, so a client asks the code and keeps no list
  of names (#23, rulings D22, D23). omx-contract >= 3.18.0, the release that
  carries the property.
- CI builds against the channel alone, never the staging channel, and names the
  repository each contract package came from (#28); the vocabulary header is
  the served omx-clap-contract 0.32.3's.
- check-rendered prints the difference when a header is not the render (#26).
- CI installs plugin-db-contract from the 0.4 line the channel serves (#22).
- CI covers the quick cells while OMX_RELEASE_MATRIX is quick: bookworm joins
  the build matrix at the full release (ruling D25).

* Fri Nov 13 2026 Pau Aliagas <linuxnow@gmail.com> - 0.5.7-1
- 0.5.6 was staged and never released: its changes (node_label, the feedback
  lines' typed arguments, the table's constants, formats and words) ship in this
  release.
- The table types and names the replies of plugin-hostd's and the protocol's own
  verbs (reply_forms on each command, resp 0 or a count, a last argument
  repeated over the rest), and arg_types gains list and record types, each
  declared once with a pattern written from its parts (#19).

* Thu Nov 12 2026 Pau Aliagas <linuxnow@gmail.com> - 0.5.6-1
- node_label: a client's label joins a hosted node's pretty-name, <label>:
  <plugin name> (<instance>); no words clears it; -545
  reply.plugin-hostd.label-invalid refuses a label over 64 bytes or not UTF-8
  (plugin-hostd#35).
- The table names and types the arguments of every plugin-hostd feedback line
  (arg_types, each line's forms), the forms of add and supervisor_reset, and the
  words a consumer recognises: placements, all, -, * and the worker states
  (#17).
- The table carries the wire's constants, protocol_version (the version hello
  answers) among them, the pin's separators, excluded bytes and escape, the
  formats a worker holds, and the LV2 workers.

* Thu Nov 12 2026 Pau Aliagas <linuxnow@gmail.com> - 0.5.5-1
- node_metadata: the JACK metadata a worker sets on each instance's client, its
  pretty name and the instance key a client matches a hosted node on
* Thu Nov 12 2026 Pau Aliagas <linuxnow@gmail.com> - 0.5.4-1
- PRESET_NOT_APPLIED (-544): the plugin took the preset and had not run once
  since when the host's wait for that ran out; its sentence and the Catalan
- the release workflow builds a staged pair into the staging channel on demand

* Tue Nov 10 2026 Pau Aliagas <linuxnow@gmail.com> - 0.5.3-1
- the scenario list exercises preset_save, refused on an instance that does not
  exist; a test holds it to the verbs a console's host backend declares

* Mon Nov 09 2026 Pau Aliagas <linuxnow@gmail.com> - 0.5.2-1
- the codes' source sentences beside the codes, the Catalan in catalogs/ca.json
  (omx-contract 3.12.0); the rendered vocabularies and the protocol page follow

* Sun Nov 08 2026 Pau Aliagas <linuxnow@gmail.com> - 0.5.1-1
- the golden deviations name only public references (monitor_output_off's
  exchanges cite omx-dsp#301); the CI runs the public-sources checks on every
  pull request

* Sun Nov 08 2026 Pau Aliagas <linuxnow@gmail.com> - 0.5.0-1
- curve_bind and curve_points: a display of an instance's panel description
  bound to the instance's parameters, and drawn from the values the instance
  holds at its rate by omx-dsp's omx_curve_points; codes -536 to -539
- host_dispatch_register_curve(): a host answers both over its backend and the
  rate it gives; one that does not answers -902
- codes -540 (durable-write-failed) and -541 (writer-queue-full) for the
  daemon's writer thread; history_clear, pin_clear and pin_accept declare them
- curve_points answers -542 (curve-value-not-finite) for a value that is not
  finite; a head filter's slope comes from its enumeration (@12|24)
- socket_adopt(): a host serves sockets its caller already bound; a second
  bind on the port is refused
- monitor_output_off <instance> <symbol> ends the monitor monitor_output set: a
  monitor ends with the connection that set it, an output this connection did
  not monitor is refused with -543 (output-not-monitored), and remove takes an
  instance's monitors with it
- host_dispatch_register_monitor_output_off(): the library keeps what each
  connection monitored and calls the host's end for an output that is ended, or
  left when the connection ends
- every code the protocol owns (-501 to -543) carries the sentence a person
  reads, in English and in Catalan, with its placeholders; the declaration needs
  omx-contract 3.11.0, which validates them
- builds with omx-dsp-devel >= 0.32.0 and < 0.33

* Sat Nov 07 2026 Pau Aliagas <linuxnow@gmail.com> - 0.4.5-1
- show_external_ui and hide_external_ui are optional of every profile: the
  daemon sends each to the worker that holds the instance and passes its
  resp back; a daemon with no display session refuses both with -902
  hosting.window.no-display; the ledger keeps the latest of the two
* Sat Nov 07 2026 Pau Aliagas <linuxnow@gmail.com> - 0.4.4-1
- A second daemon on a busy port is refused instead of silently sharing it: the ports are
  opened with SO_REUSEADDR, never SO_REUSEPORT, so a port the daemon before left in
  TIME_WAIT is still taken again at once
* Fri Nov 06 2026 Pau Aliagas <linuxnow@gmail.com> - 0.4.3-1
- The descriptor name rule says bytes: each byte of an id's UTF-8 text
  outside [A-Za-z0-9._-] is one PHD_DESCRIPTOR_SAFE_CHAR, as plugin-hostd
  reads it and plugin-qualify now writes it
* Fri Nov 06 2026 Pau Aliagas <linuxnow@gmail.com> - 0.4.2-1
- The protocol page and protocol.h say how a pin is keyed: by its clap: uri's
  path decoded, so the uri written with the space as it is and written encoded
  name one pin; '%' is always an escape, and a name holding one writes it %25
* Fri Nov 06 2026 Pau Aliagas <linuxnow@gmail.com> - 0.4.1-1
- A path of a pin, and the path of a clap: uri, is written percent-encoded:
  a separator of pin_set, whitespace, '%', a quote, a backslash and a control
  byte are %XX, so any file name is one word. protocol.h declares the escape
  and phd_pin_path_encode() and phd_pin_path_decode()
- add: a clap: uri whose path holds a space and is written bare is one uri;
  phd_add_uri() is that reading, for every host that answers add
- phd_headless_qt(): with no display, a host gives Qt the offscreen platform
  unless the user set one, so a Qt-linked plugin (synthv1) does not abort
  as it is instantiated

* Fri Nov 06 2026 Pau Aliagas <linuxnow@gmail.com> - 0.4.0-1
- lv2_worker auto is no longer a word of the protocol: a plugin the chosen LV2
  worker refuses is answered with its code and never handed to another host.
  lv2_worker and lv2_worker_set take mod-host or omx-clap-host, and
  lv2_adapter_refused names omx-clap-host only
- pin_accept of a plugin whose pin carries verdict=refused (pins_dir or
  pin_file) is the user's choice to load it anyway, alone in a worker of its
  own; pin_clear takes it back; -516 is no longer its answer
- plugin_db_vocabulary.h is the render of plugin-db 0.2.0's words
* Wed Nov 04 2026 Pau Aliagas <linuxnow@gmail.com> - 0.3.0-1
- hello: the protocol version and the extensions a host grants; a client that
  sends no hello, or whose hello is refused, is answered as mod-host answers it
- Request tags, error words and event sequence numbers, each an extension a
  session asks for in its hello
- param_get_many and param_set_many: several values of one instance in one line,
  param_set_many all or nothing
- The default lv2_worker is omx-clap-host; mod-host runs an LV2 plugin only when
  named (#13)
- worker_list and plan: the core field reads -, a worker pinned to the whole
  audio set (#15)
- A track_info name is held to UTF-8 without C1 controls (#11); a word "..."
  where a pattern ends in "..." is an argument (#12)
- Fuzz harnesses with records, sabotage arms and a coverage floor run in CI
- The packages install their licence and a short README and no other markdown
- protocol_add_hello() and host_scenario_set_host() are the exported symbols added
- The words of a pin in protocol.h are the ones plugin-db declares
  (plugin_db_vocabulary.h, rendered from plugin-db 0.1.0's package)
- Dependencies carry a floor and a ceiling: omx-contract >= 3.9.0 and < 4 to
  build, >= 3.2.0 and < 4 to install the contract package

* Tue Nov 03 2026 Pau Aliagas <linuxnow@gmail.com> - 0.2.0-1
- protocol_parse() reads every form registered for a verb and runs the first whose
  arguments fit in number and type, so both forms of preset_show dispatch through the
  library (#5)
- The whole declared protocol moves here from plugin-hostd: plugin-hostd/protocol.h
  and the headers omx-contract renders for it, data/verbs.json,
  protocol/plugin-hostd.json and its schema, protocol/mod-host.json, and the
  generators and checks that keep them current
- One verb table, protocol/plugin-hostd-protocol.json: every command, each once,
  with its origin, its argument types, its reply and its refusals; a word of ours
  that changes a mod-host word is refused
- The grammar of a line in ABNF, generated from the table, and doc/protocol.md,
  generated from the table and the declarations; the scenario corpus and every
  reply of the golden transcript are held to the grammar
- New package plugin-hostd-protocol-contract: data/verbs.json as the package
  plugin-hostd-protocol of omx-contract's contract directory
- The library package installs the table, the grammar and the JSON of the
  protocol and of mod-host's, which programs read at run time; the devel package
  installs the headers under plugin-hostd/, the schema and the generator's
  source; pkg-config variables name each
- The library's exported symbols and types are those of 0.1.0

* Mon Nov 02 2026 Pau Aliagas <linuxnow@gmail.com> - 0.1.0-1
- First release: mod-host's socket server, line protocol and command dispatch
  (FreeMixer/mod-host 013ac7d, the code of mod-host-protocol 0.10.6-11.3) as a
  library of its own, the same API under the name plugin-hostd-protocol
- param_info writes NaN, Infinity and -Infinity for a value that is not finite,
  where it wrote past a stack array (plugin-hostd#371)
- a command whose first word is not a verb's no longer runs that verb's
  callback (plugin-hostd#372)
