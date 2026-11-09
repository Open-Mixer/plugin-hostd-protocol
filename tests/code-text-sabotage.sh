#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# tests/code_text.py against a scratch copy of data/verbs.json with one rule broken in each arm: it must fail on each, and the
# copy left as it is must pass, so a check that holds nothing is caught. The last arms stage the broken declaration as the
# package of omx-contract's contract directory and the validator itself must refuse it (make check-contract-dir), when
# omx-contract is installed.
#
#   sh tests/code-text-sabotage.sh          (make test-code-text-sabotage)
set -eu
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
failed=0
PYTHON=${PYTHON:-python3}
MAKE=${MAKE:-make}

# a scratch tree whose data/verbs.json is edited by python: <python that edits the codes c, or the whole declaration d>
scratch() {
    rm -rf "$work/tree"
    mkdir -p "$work/tree"
    cp -r Makefile VERSION data protocol catalogs "$work/tree/"
    $PYTHON - "$work/tree/data/verbs.json" "$work/tree/catalogs/ca.json" "$1" <<'PY'
import json, sys
path, catpath, edit = sys.argv[1], sys.argv[2], sys.argv[3]
d = json.load(open(path))
c = d["PLUGIN_HOSTD_REPLY_CODES"]["codes"]
cat = json.load(open(catpath))
m = cat["messages"]
exec(edit)
json.dump(d, open(path, "w"), indent=2, ensure_ascii=False)
json.dump(cat, open(catpath, "w"), indent=2, ensure_ascii=False)
PY
}

# <name> <edit>: code_text.py must fail on the edited copy
arm() {
    scratch "$2"
    if $PYTHON tests/code_text.py "$work/tree/data/verbs.json" protocol/plugin-hostd-protocol.json "$work/tree/catalogs/ca.json" >"$work/log" 2>&1; then
        echo "FAIL $1: code_text.py passes"
        failed=1
    else
        echo "ok   $1: red ($(grep -m1 '^FAIL' "$work/log" | head -c 110))"
    fi
}

# <name> <edit>: the staged package must be refused by omx-contract's validator itself (make check-contract-dir)
validator() {
    if ! command -v "${OMX_CONTRACT:-omx-contract}" >/dev/null; then
        echo "skip $1: no omx-contract"
        return
    fi
    scratch "$2"
    if (cd "$work/tree" && $MAKE -s check-contract-dir) >"$work/log" 2>&1; then
        echo "FAIL $1: omx-contract validate passes"
        failed=1
    else
        echo "ok   $1: omx-contract refuses it ($(grep -m1 'catalog-\|code-text\|text\|required' "$work/log" | head -c 110))"
    fi
}

# the copy as it is passes
if $PYTHON tests/code_text.py data/verbs.json protocol/plugin-hostd-protocol.json catalogs/ca.json >"$work/log" 2>&1; then
    echo "ok   control: the declaration as it is passes"
else
    echo "FAIL control: the declaration as it is fails"; head -3 "$work/log"; failed=1
fi

arm "a code with no text" 'del c["noCell"]["text"]'
arm "a code with no args" 'del c["noCell"]["args"]'
arm "a code with no Catalan in the catalog" 'del m["reply.plugin-hostd.no-cell"]'
arm "a Catalan sentence left in the data" 'c["noCell"]["text"]["ca"] = m["reply.plugin-hostd.no-cell"]'
arm "a code with no en" 'del c["noCell"]["text"]["en"]'
arm "ca is a copy of en" 'm["reply.plugin-hostd.no-cell"] = c["noCell"]["text"]["en"]'
arm "a placeholder only en uses" 'c["noCell"]["text"]["en"] = "No cell for {plugin}; try another."; c["noCell"]["args"] = ["plugin"]'
arm "a placeholder outside args" 'c["noCell"]["text"]["en"] = "No cell for {plugin}; try another."; m["reply.plugin-hostd.no-cell"] = "Cap cel·la per a {plugin}; proveu-ne una altra."'
arm "a catalog key that is no code of ours" 'm["reply.plugin-hostd.zz-gone"] = "Ja no hi és."'
arm "an arg no sentence uses" 'c["noCell"]["args"] = ["plugin"]'
arm "two sentences in one text" 'c["noCell"]["text"]["en"] = "There is no cell. Try another one."'
arm "a code name inside the sentence" 'c["noCell"]["text"]["en"] = "The reply.plugin-hostd.no-cell code came back; try another."'
arm "a new code of ours with no sentence" 'c["zzNew"] = {"code": "reply.plugin-hostd.zz-new", "resp": -599, "doc": "x"}'
arm "a code of the table missing from the declaration" 'del c["unknownPlugin"]'
arm "the schema floor left at 3.3.0" 'd["requires"]["omx-contract-schema"] = ">= 3.3.0"'
validator "the validator refuses a catalog key that is no code of ours" 'm["reply.plugin-hostd.zz-gone"] = "Ja no hi és."'
validator "the validator refuses a catalog placeholder outside args" 'm["reply.plugin-hostd.no-cell"] = "Cap cel·la per a {plugin}; proveu-ne una altra."'
validator "the validator refuses a placeholder outside args" 'c["noCell"]["text"]["en"] = "No cell for {plugin}; try another."'

[ "$failed" = 0 ] && echo "code text sabotage ok" || { echo "code text sabotage FAILED"; exit 1; }
