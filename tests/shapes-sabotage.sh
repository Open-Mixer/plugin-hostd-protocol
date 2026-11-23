#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# The reply shapes' guards (doc/reply-shapes.md), each broken in a scratch copy of the tree: test-reply-shapes must go red
# on a schema that no longer says what the table or the replies say, and check-shapes-render on a TypeScript render that is
# not what the schemas render; both pass on the copy as it is.
#
#   OMX_CONTRACT_DIR=<omx-contract checkout, npm ci run> sh tests/shapes-sabotage.sh     (make test-shapes-sabotage)
set -eu
: "${OMX_CONTRACT_DIR:?set OMX_CONTRACT_DIR to a checkout of Open-Mixer/omx-contract with npm ci run in it}"
PYTHON=${PYTHON:-python3}
MAKE=${MAKE:-make}
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
failed=0

fresh() {
    rm -rf "$work/tree"
    mkdir -p "$work/tree/tests/golden" "$work/tree/packaging"
    cp -r Makefile VERSION shapes ts protocol "$work/tree/"
    cp packaging/plugin-hostd-protocol.spec "$work/tree/packaging/"   # the Makefile reads omx-dsp's floor from it
    cp tests/reply_shapes.py tests/reply_args.py "$work/tree/tests/"
    cp tests/golden/plugin-hostd-protocol.txt "$work/tree/tests/golden/"
}
# a run's log holds nothing but the target's own words: no tool complaining about a file the copy lacks, which would be read
# as the reason an arm went red
clean() { ! grep -q -i "no such file\|can't read\|cannot open" "$work/log"; }
run() { (cd "$work/tree" && $MAKE -s "$1" PYTHON="$PYTHON" OMX_CONTRACT_DIR="$OMX_CONTRACT_DIR" >"$work/log" 2>&1); }

# <name> <make target> <the words the guard must answer with>, on the tree as fresh-then-changed left it
verdict() {
    name=$1 target=$2 why=$3
    if run "$target"; then
        echo "FAIL $name: $target passes"
        failed=1
    elif ! clean || ! grep -q -F -- "$why" "$work/log"; then
        echo "FAIL $name: $target is red, but not because of the change (\"$why\" not in its log): $(grep -v '^ok' "$work/log" | head -n3 | tr '\n' '|' | head -c 400)"
        failed=1
    else
        echo "ok   $name: red ($(grep -F -- "$why" "$work/log" | head -n1 | head -c 110))"
    fi
}
# <name> <make target> <file> <text, as sed matches it> <what it becomes> <the words the guard must answer with>
arm() {
    name=$1 target=$2 file=$3 from=$4 to=$5 why=$6
    fresh
    grep -q "$from" "$work/tree/$file" || { echo "FAIL $name: \"$from\" is not in $file"; failed=1; return; }
    sed -i "0,/$from/s//$to/" "$work/tree/$file"
    verdict "$name" "$target" "$why"
}

fresh
for t in test-reply-shapes check-shapes-render; do
    if run $t && clean; then echo "ok   control: $t passes on the copy as it is"; elif clean; then echo "FAIL control: $t fails on the copy as it is"; cat "$work/log"; failed=1
    else echo "FAIL control: $t complains of a file the copy lacks"; cat "$work/log"; failed=1; fi
done
arm "a property dropped from a schema" test-reply-shapes shapes/reply_instance_info.schema.json '"crashes": {' '"crashes_": {' "are not the names ['worker', 'pid', 'state', 'crashes', 'quarantined']"
arm "a state word the table does not declare" test-reply-shapes shapes/reply_instance_info.schema.json 'up|starting|backoff|given-up' 'up|starting|backoff|sleeping' "are not the table's worker_states"
arm "a schema left open" test-reply-shapes shapes/reply_hello.schema.json '"additionalProperties": false' '"additionalProperties": true' "FAIL hello: not closed (additionalProperties)"
arm "a pid typed as a word" test-reply-shapes shapes/reply_instance_info.schema.json '"type": "integer"' '"type": "string"' "is not of type 'string' at pid"
arm "a record key renamed" test-reply-shapes shapes/reply_worker_list.schema.json '"place": {' '"where": {' "FAIL worker_list.workers: not the record worker-record of the table"
arm "the table renames a reply word" test-reply-shapes protocol/plugin-hostd-protocol.json '"crashes", "quarantined"' '"crashed", "quarantined"' "'crashed', 'quarantined'] of the longest reply form"
arm "a render edited by hand" check-shapes-render ts/shapes/reply_hello.d.ts 'version: number' 'version: string' "ts/shapes/reply_hello.d.ts differs"
arm "a schema changed and not rendered again" check-shapes-render shapes/reply_hello.schema.json 'The protocol version the daemon answers on' 'The version' "ts/shapes/reply_hello.d.ts differs"
fresh; rm "$work/tree/shapes/reply_hello.schema.json"
verdict "a schema deleted" test-reply-shapes "shapes/reply_hello.schema.json is absent"
fresh; echo '// extra' > "$work/tree/ts/shapes/reply_extra.d.ts"
verdict "an extra render" check-shapes-render "ts/shapes/reply_extra.d.ts is extra"

if [ "$failed" = 0 ]; then echo "shapes sabotage ok"; else echo "shapes sabotage FAILED"; exit 1; fi
