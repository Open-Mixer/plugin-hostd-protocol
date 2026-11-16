#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# phd_number_in holds the declared bound (tests/number_in_test.c): the old ceiling put back in a scratch copy of the
# header, and number_in_test built over it must fail.
#
#   number-in-sabotage.sh "<cc>" "<cflags>"
set -eu
cc=$1
cflags=$2
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
mkdir -p "$work/tree"
cp -r include tests "$work/tree/"
h="$work/tree/include/plugin-hostd/protocol.h"
grep -qF 'v > 1000000000LL' "$h" || { echo "FAIL the bound is not in protocol.h"; exit 1; }
sed -i 's/v > 1000000000LL/v > 100000/' "$h"
# shellcheck disable=SC2086
(cd "$work/tree" && $cc $cflags -o number_in_test tests/number_in_test.c) || { echo "FAIL the sabotaged tree does not build"; exit 1; }
if (cd "$work/tree" && ./number_in_test) >"$work/log" 2>&1; then
    echo "FAIL number_in_test passes over the old ceiling"
    exit 1
fi
echo "ok   the old 1,000,009 ceiling: number_in_test fails ($(grep -m1 '^FAIL' "$work/log" | head -c 120))"
echo "number-in sabotage ok"
