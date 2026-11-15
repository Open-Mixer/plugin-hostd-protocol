#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/transient_codes.py <table> <verbs.json> <vocabulary header>: which refusals are transient is a property of the
code (plugin-hostd-protocol#23, rulings D22 and D23), declared once in data/verbs.json and read by a client from the
table, never a list of code names it keeps. A code declared transient reads transient in the table's reply_codes and
has its _TRANSIENT macro in the rendered header; a code not declared so carries neither. Transient means an identical
request may succeed later with no change by the caller: the worker not up, the storm bound spent, the verb dropped with
its worker. descriptor-absent and pin-refused, which say something about the plugin, are not. A planted fault (a flag
dropped, one added, a macro missing) must be seen.

The second class (ruled 2026-11-15): a refusal that LEAVES NO MARK on the plugin, declared `leavesNoMark: true` in
data/verbs.json, read `leaves_no_mark: true` in the table and `<CODE>_LEAVES_NO_MARK 1` in the header. pin-refused is
in it, and still not transient. The same three planted faults must be seen for it too."""
import copy
import json
import re
import sys

TRANSIENT = ("reply.plugin-hostd.worker-spawn", "reply.plugin-hostd.gave-up", "reply.plugin-hostd.verb-dropped")
# a refused pin is a verdict about the plugin: it needs a different pin or a re-qualification, never the same request
# again (D23)
NOT_TRANSIENT = ("reply.plugin-hostd.descriptor-absent", "reply.plugin-hostd.pin-refused")
# a refused pin leaves no mark on the plugin: no verdict and no history are written for it; descriptor-absent does
LEAVES_NO_MARK = ("reply.plugin-hostd.pin-refused",)
NOT_LEAVES_NO_MARK = ("reply.plugin-hostd.descriptor-absent",)
# each class: its key in data/verbs.json, in the table, its macro suffix, the codes in it, the codes never in it
CLASSES = (("transient", "transient", "TRANSIENT", TRANSIENT, NOT_TRANSIENT),
           ("leavesNoMark", "leaves_no_mark", "LEAVES_NO_MARK", LEAVES_NO_MARK, NOT_LEAVES_NO_MARK))


def declared(verbs, key="transient"):
    return {c["code"] for item in verbs.values() if isinstance(item, dict) and item.get("kind") == "codes"
            for c in item["codes"].values() if c.get(key) is True}


def macro(code, suffix="TRANSIENT"):
    return "OMX_" + re.sub(r"[^A-Za-z0-9]+", "_", code).upper() + "_" + suffix


def problems(table, verbs, header):
    out = []
    for key, col, suffix, yes, no in CLASSES:
        want = declared(verbs, key)
        read = {r["code"] for r in table["reply_codes"] if r.get(col) is True}
        for r in table["reply_codes"]:
            if col in r and r[col] is not True:
                out.append("%s carries %s %r: the table writes it only when it is true" % (r["code"], col, r[col]))
        if read != want:
            out.append("the table's %s codes %s differ from data/verbs.json's %s" % (col, sorted(read), sorted(want)))
        for code in yes:
            if code not in read:
                out.append("%s is not %s in the table" % (code, col))
        for code in no:
            if code in read:
                out.append("%s is %s in the table" % (code, col))
        macros = set(re.findall(r"^#define (OMX_\w+_%s) 1$" % suffix, header, re.M))
        if macros != {macro(c, suffix) for c in want}:
            out.append("the header's _%s macros %s differ from the declared codes'" % (suffix, sorted(macros)))
    return out


def main(argv):
    with open(argv[1]) as f:
        table = json.load(f)
    with open(argv[2]) as f:
        verbs = json.load(f)
    with open(argv[3]) as f:
        header = f.read()
    found = problems(table, verbs, header)
    for p in found:
        print("FAIL " + p)
    if found:
        return 1
    # the check sees a planted fault in each class: a flag dropped from the table, one added to a code never in the
    # class, a macro missing
    for key, col, suffix, yes, no in CLASSES:
        dropped = copy.deepcopy(table)
        next(r for r in dropped["reply_codes"] if r["code"] == yes[0]).pop(col, None)
        added = copy.deepcopy(table)
        next(r for r in added["reply_codes"] if r["code"] == no[0])[col] = True
        for name, fault in (("a flag dropped", (dropped, verbs, header)), ("a flag added", (added, verbs, header)),
                            ("a macro missing", (table, verbs, header.replace("#define %s 1\n" % macro(yes[0], suffix), "")))):
            if not problems(*fault):
                print("FAIL the check does not see %s (%s)" % (name, col))
                return 1
    print("reply classes: ok (%d transient, %d leaving no mark, each in the table and the header; 6 planted faults seen)"
          % (len(declared(verbs)), len(declared(verbs, "leavesNoMark"))))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
