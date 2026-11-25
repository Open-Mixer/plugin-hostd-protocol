#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# The served floor (tools/served-floor.py, #180): each arm breaks one part of the tool in a scratch copy of the tree and
# tests/served_floor_test.py, run over that copy, must fail.
#
#   served-floor-sabotage.sh
set -eu
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
failed=0
arm() { # <what> <sed script>
    what=$1 script=$2
    rm -rf "$work/tree"; mkdir -p "$work/tree"
    cp -r tools tests debian packaging VERSION "$work/tree/"
    cp "$work/tree/tools/served-floor.py" "$work/before"
    sed -i "$script" "$work/tree/tools/served-floor.py"
    if cmp -s "$work/before" "$work/tree/tools/served-floor.py"; then echo "FAIL $what: the sabotage changed nothing"; failed=1; return; fi
    if python3 "$work/tree/tests/served_floor_test.py" "$work/tree" >"$work/log" 2>&1; then
        echo "FAIL $what: served_floor_test passes"; failed=1
    else
        echo "ok   $what: served_floor_test fails ($(grep -m1 '^FAIL' "$work/log" | head -c 100))"
    fi
}
arm "the history of the ledgers" 's/line\[:1\] == "+" and/False and/'
arm "the tilde order" '0,/return -1$/s//return 1/'
arm "the spec's subpackage names" 's/"%s-%s" % (main, sub)/sub/'
arm "the rpm ledger" 's|, "rpm/cells.tsv"]|]|'
arm "an empty ledger is no pass" 's/    if not top:/    if False:/'
[ "$failed" = 0 ] && echo "served-floor sabotage ok" || { echo "served-floor sabotage FAILED"; exit 1; }
