#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Development keeps no backward-compatibility machinery (owner rule, 2026-11-16): the paths below are gone and stay gone.
#   sh tests/no-backcompat.sh      each line is ok or FAIL; exit 1 on any FAIL
cd "$(dirname "$0")/.." || exit 2
bad=0
gone() { # gone <what> <fixed-string> <file>...
    what=$1; pat=$2; shift 2
    if grep -qF -- "$pat" "$@" 2>/dev/null; then echo "FAIL $what is still there ($pat)"; bad=1; else echo "ok   $what is gone"; fi
}
gone "the Conflicts on a plugin-hostd-contract before 0.16.0" "plugin-hostd-contract" packaging/plugin-hostd-protocol.spec debian/control
gone "the Breaks/Replaces of a language pack on the contract package before 0.5.6" "(<< 0.5.6~)" debian/control
gone "the language pack 'moved' input" "moved:" .github/workflows/ci.yml
for v in OMX_VERB_PLAN OMX_VERB_PLAN_GET OMX_VERB_ESTIMATE; do
    n=$(grep -c "X($v, \"0\"," include/plugin-hostd/protocol.h)
    if [ "$n" -eq 1 ]; then echo "ok   $v has one reply form"; else echo "FAIL $v has $n reply forms, expected 1"; bad=1; fi
done
for f in tests/macro_compat.c; do
    if [ -e "$f" ]; then echo "FAIL $f is still there"; bad=1; else echo "ok   $f is gone"; fi
done
gone "the test-macro-compat target" "macro-compat" Makefile
for f in include/host-errors.h protocol/mod-host.json tests fuzz/fuzz_dispatch.c; do
    gone "the ERR_LV2_* aliases in $f" "ERR_LV2_" $( [ -d "$f" ] && echo "$f"/*.c || echo "$f" )
done
exit $bad
