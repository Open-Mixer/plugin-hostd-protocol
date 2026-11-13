#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/transient_codes.py <table> <verbs.json> <vocabulary header>: which refusals are transient is a property of the
code (plugin-hostd-protocol#23, rulings D22 and D23), declared once in data/verbs.json and read by a client from the
table, never a list of code names it keeps. A code declared transient reads transient in the table's reply_codes and
has its _TRANSIENT macro in the rendered header; a code not declared so carries neither. Transient means an identical
request may succeed later with no change by the caller: the worker not up, the storm bound spent, the verb dropped with
its worker. descriptor-absent and pin-refused, which say something about the plugin, are not. A planted fault (a flag
dropped, one added, a macro missing) must be seen."""
import copy
import json
import re
import sys

TRANSIENT = ("reply.plugin-hostd.worker-spawn", "reply.plugin-hostd.gave-up", "reply.plugin-hostd.verb-dropped")
# a refused pin is a verdict about the plugin: it needs a different pin or a re-qualification, never the same request
# again (D23)
NOT_TRANSIENT = ("reply.plugin-hostd.descriptor-absent", "reply.plugin-hostd.pin-refused")


def declared(verbs):
    return {c["code"] for item in verbs.values() if isinstance(item, dict) and item.get("kind") == "codes"
            for c in item["codes"].values() if c.get("transient") is True}


def macro(code):
    return "OMX_" + re.sub(r"[^A-Za-z0-9]+", "_", code).upper() + "_TRANSIENT"


def problems(table, verbs, header):
    out = []
    want = declared(verbs)
    read = {r["code"] for r in table["reply_codes"] if r.get("transient") is True}
    for r in table["reply_codes"]:
        if "transient" in r and r["transient"] is not True:
            out.append("%s carries transient %r: the table writes it only when it is true" % (r["code"], r["transient"]))
    if read != want:
        out.append("the table's transient codes %s differ from data/verbs.json's %s" % (sorted(read), sorted(want)))
    for code in TRANSIENT:
        if code not in read:
            out.append("%s is not transient in the table" % code)
    for code in NOT_TRANSIENT:
        if code in read:
            out.append("%s is transient in the table" % code)
    macros = set(re.findall(r"^#define (OMX_\w+_TRANSIENT) 1$", header, re.M))
    if macros != {macro(c) for c in want}:
        out.append("the header's _TRANSIENT macros %s differ from the declared codes'" % sorted(macros))
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
    # the check sees a planted fault: a flag dropped from the table, one added to a plugin refusal, a macro missing
    first = next(r for r in table["reply_codes"] if r["code"] == TRANSIENT[0])
    dropped = copy.deepcopy(table)
    next(r for r in dropped["reply_codes"] if r["code"] == TRANSIENT[0]).pop("transient", None)
    added = copy.deepcopy(table)
    next(r for r in added["reply_codes"] if r["code"] == NOT_TRANSIENT[0])["transient"] = True
    for name, fault in (("a flag dropped", (dropped, verbs, header)), ("a flag added", (added, verbs, header)),
                        ("a macro missing", (table, verbs, header.replace("#define %s 1\n" % macro(first["code"]), "")))):
        if not problems(*fault):
            print("FAIL the check does not see %s" % name)
            return 1
    print("transient codes: ok (%d declared, each transient in the table and the header; 3 planted faults seen)"
          % len(declared(verbs)))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
