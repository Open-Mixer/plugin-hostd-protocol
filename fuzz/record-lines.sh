#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# make test-fuzz-record-lines: EVERY RECORD STILL NAMES ITS LINE. Each `mutant: <path>:<line> `<old>` -> `<new>``
# record of the harnesses (fuzz/checks.awk) must find <old> exactly once on <path>:<line>, the test make fuzz-records
# applies before it builds a mutant. This one needs no compiler, so make test holds it: a change to src/ that moves
# a line a record names is red here, not first at the tag's fuzz run (make fuzz-records, clang, tag-only).
#
# Usage: sh fuzz/record-lines.sh
set -u
ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT" || exit 1
CHECKED="fuzz/fuzz_protocol.c fuzz/fuzz_dispatch.c fuzz/fuzz_scenario.c fuzz/fuzz_session.c fuzz/fuzz.h"

list=$(for f in $CHECKED; do
    awk '{ printf "%d|1|%s\n", NR, $0 }' "$f" | awk -v file="$f" -v records=1 -f fuzz/checks.awk ||
        { echo "make test-fuzz-record-lines: RED -- $f has a check with no record (above)" >&2; exit 1; }
done) || exit 1
[ -n "$list" ] || { echo "make test-fuzz-record-lines: RED -- no record"; exit 1; }

n=0
red=
us=$(printf '\037')
while IFS="$us" read -r file at path line old _new _input; do
    n=$((n + 1))
    if [ ! -f "$path" ]; then
        echo "$file:$at: $path is not a file of the tree"
        red="$red $file:$at"
        continue
    fi
    # <old> exactly once on line <line>, as fuzz/records.sh requires before it applies the change
    if ! OLD=$old awk -v l="$line" '
        BEGIN { old = ENVIRON["OLD"]; bad = 1 }
        NR == l {
            n = 0; t = $0
            while ((i = index(t, old)) > 0) { n++; t = substr(t, i + length(old)) }
            bad = (n != 1)
        }
        END { exit bad }' "$path"; then
        at_now=$(grep -n -F -- "$old" "$path" | cut -d: -f1 | paste -sd' ')
        echo "$file:$at: \`$old\` is not once on $path:$line (it is on line ${at_now:-none})"
        red="$red $file:$at"
    fi
done <<EOF
$list
EOF

if [ -n "$red" ]; then
    echo "make test-fuzz-record-lines: RED -- $n records;$red"
    exit 1
fi
echo "make test-fuzz-record-lines: all $n records name a line that holds their text once"
