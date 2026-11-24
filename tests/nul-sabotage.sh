#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# The NUL refusals (tests/nul_refusal_test.py): each arm turns one door off in a scratch copy of the clients and the
# test, run over that copy, must fail.
#
#   nul-sabotage.sh
set -eu
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
failed=0
arm() { # <what> <file> <sed script>
    what=$1 file=$2 script=$3
    rm -rf "$work/tree"; mkdir -p "$work/tree"
    cp -r clients tests "$work/tree/"
    cp "$work/tree/$file" "$work/before"
    sed -i "$script" "$work/tree/$file"
    if cmp -s "$work/before" "$work/tree/$file"; then echo "FAIL $what: the sabotage changed nothing"; failed=1; return; fi
    if NUL_TREE="$work/tree" python3 "$work/tree/tests/nul_refusal_test.py" >"$work/log" 2>&1; then
        echo "FAIL $what: nul_refusal_test passes"; failed=1
    else
        echo "ok   $what: nul_refusal_test fails ($(grep -m1 '^FAIL' "$work/log" | head -c 100))"
    fi
}
arm "the client's door" clients/python/plugin_hostd_client/__init__.py 's/if "\\0" in line:/if False:/'
arm "the codec's NUL check" clients/python/plugin_hostd_protocol/__init__.py 's/    if "\\0" in text:/    if False:/'
[ "$failed" = 0 ] && echo "nul sabotage ok" || { echo "nul sabotage FAILED"; exit 1; }
