#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# The grammar's curve words follow omx-dsp's: in a scratch grammar, each of an alternative dropped from curve-origin, one
# added to curve-origin, one dropped from curve-leg and one added to curve-leg must turn tests/curve_words_test.py red.
#
#   curve-words-sabotage.sh -I<dir> ...
set -eu
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
failed=0
what1="an origin dropped"; what2="an origin added"; what3="a leg dropped"; what4="a leg added"
s1='/^curve-origin/s# / %s"hrp"##'
s2='/^curve-origin/s#$# / %s"made-up"#'
s3='/^curve-leg/s# / %s"right"##'
s4='/^curve-leg/s#$# / %s"centre"#'
n=0
for s in "$s1" "$s2" "$s3" "$s4"; do
    n=$((n + 1)); eval "w=\$what$n"
    cp grammar/core.abnf "$work/core.abnf"; sed -i "$s" "$work/core.abnf"
    if cmp -s grammar/core.abnf "$work/core.abnf"; then echo "FAIL $w: the sabotage changed nothing"; failed=1; continue; fi
    if CURVE_WORDS_GRAMMAR="$work/core.abnf" python3 tests/curve_words_test.py "$@" >"$work/log" 2>&1; then
        echo "FAIL $w: curve_words_test passes"; failed=1
    else
        echo "ok   $w: curve_words_test fails ($(grep -m1 '^FAIL' "$work/log" | head -c 90))"
    fi
done
[ "$failed" = 0 ] && echo "curve words sabotage ok" || { echo "curve words sabotage FAILED"; exit 1; }
