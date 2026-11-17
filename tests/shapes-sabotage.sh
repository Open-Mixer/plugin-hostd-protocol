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
    mkdir -p "$work/tree/tests/golden"
    cp -r Makefile VERSION shapes ts protocol "$work/tree/"
    cp tests/reply_shapes.py tests/reply_args.py "$work/tree/tests/"
    cp tests/golden/plugin-hostd-protocol.txt "$work/tree/tests/golden/"
}
run() { (cd "$work/tree" && $MAKE -s "$1" PYTHON="$PYTHON" OMX_CONTRACT_DIR="$OMX_CONTRACT_DIR" >"$work/log" 2>&1); }

# <name> <make target> <file> <text, as sed matches it> <what it becomes>
arm() {
    name=$1 target=$2 file=$3 from=$4 to=$5
    fresh
    grep -q "$from" "$work/tree/$file" || { echo "FAIL $name: \"$from\" is not in $file"; failed=1; return; }
    sed -i "0,/$from/s//$to/" "$work/tree/$file"
    if run "$target"; then
        echo "FAIL $name: $target passes with $file changed"
        failed=1
    else
        echo "ok   $name: red ($(grep -v '^ok' "$work/log" | head -n1 | head -c 110))"
    fi
}

fresh
for t in test-reply-shapes check-shapes-render; do
    if run $t; then echo "ok   control: $t passes on the copy as it is"; else echo "FAIL control: $t fails on the copy as it is"; cat "$work/log"; failed=1; fi
done
arm "a property dropped from a schema" test-reply-shapes shapes/reply_instance_info.schema.json '"crashes": {' '"crashes_": {'
arm "a state word the table does not declare" test-reply-shapes shapes/reply_instance_info.schema.json 'up|starting|backoff|given-up' 'up|starting|backoff|sleeping'
arm "a schema left open" test-reply-shapes shapes/reply_hello.schema.json '"additionalProperties": false' '"additionalProperties": true'
arm "a pid typed as a word" test-reply-shapes shapes/reply_instance_info.schema.json '"type": "integer"' '"type": "string"'
arm "a record key renamed" test-reply-shapes shapes/reply_worker_list.schema.json '"place": {' '"where": {'
arm "the table renames a reply word" test-reply-shapes protocol/plugin-hostd-protocol.json '"crashes", "quarantined"' '"crashed", "quarantined"'
arm "a render edited by hand" check-shapes-render ts/shapes/reply_hello.d.ts 'version: number' 'version: string'
arm "a schema changed and not rendered again" check-shapes-render shapes/reply_hello.schema.json 'The protocol version the daemon answers on' 'The version'
fresh; rm "$work/tree/shapes/reply_hello.schema.json"
if run test-reply-shapes; then echo "FAIL a schema deleted: test-reply-shapes passes"; failed=1; else echo "ok   a schema deleted: red"; fi
fresh; echo '// extra' > "$work/tree/ts/shapes/reply_extra.d.ts"
if run check-shapes-render; then echo "FAIL an extra render: check-shapes-render passes"; failed=1; else echo "ok   an extra render: red"; fi

[ "$failed" = 0 ] && echo "shapes sabotage ok" || { echo "shapes sabotage FAILED"; exit 1; }
