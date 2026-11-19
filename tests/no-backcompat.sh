#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# tests/no-backcompat.sh: the protocol carries no backward-compatibility machinery while it is developed. Each arm names
# one mechanism and fails, naming it, while it exists. Usage: no-backcompat.sh [root]. (#109)
root=${1:-.}
cd "$root" || exit 2
bad=0
gone() { echo "FAIL still there: $1" >&2; bad=1; }

# a verb has ONE reply form: no short form of an older daemon's reply
[ "$(sed -n '/^#define PHD_REPLY_ARGS(X)/,/^$/p' include/plugin-hostd/protocol.h | grep -cE '^[[:space:]]*X\(OMX_VERB_(PLAN|PLAN_GET|ESTIMATE),')" = 3 ] \
    || gone "plan, plan_get or estimate has more than one reply form (include/plugin-hostd/protocol.h PHD_REPLY_ARGS)"

# one name for a host error: no ERR_LV2_* alias of an ERR_HOST_* code in the header, its JSON, the fuzzer or the tests
P='include/host-errors.h protocol/mod-host.json fuzz tests'
if git grep -lE 'ERR_LV2_' -- $P ':!tests/no-backcompat.sh' >/dev/null 2>&1; then
    gone "an ERR_LV2_* alias of an ERR_HOST_* code ($(git grep -lE 'ERR_LV2_' -- $P ':!tests/no-backcompat.sh' | tr '\n' ' '))"
fi

# no package relation to the package this one replaced (plugin-hostd-contract, before 0.16.0)
P='packaging/plugin-hostd-protocol.spec debian/control .github/workflows'
if grep -rnE 'plugin-hostd-contract' $P >/dev/null 2>&1; then
    gone "a relation to plugin-hostd-contract, the package this one replaced ($(grep -rlE 'plugin-hostd-contract' $P | tr '\n' ' '))"
fi

# no build-compatibility check kept for consumers of older X-macro shapes
if [ -e tests/macro_compat.c ] || grep -qE 'macro[_-]compat' Makefile .gitignore; then
    gone "tests/macro_compat.c and its Makefile targets"
fi

# the commit-msg hook reads one spelling of its settings: no deprecated FREEMIXER_* names
if grep -q 'FREEMIXER_' .githooks/commit-msg; then
    gone "the deprecated FREEMIXER_* names of .githooks/commit-msg"
fi

[ "$bad" = 0 ] && echo "ok   no backward-compatibility path is left"
exit "$bad"
