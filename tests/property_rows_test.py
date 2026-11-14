#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/property_rows_test.py: the property verbs (omx-clap-core#15, lane 2) in the daemon's tables and the grammar.

omx-clap-core declares property_list, property_info, property_get and property_set and renders them here
(omx_clap_core_host_verbs.h); this repository makes them instance verbs the daemon relays to the worker that holds the
instance, with a level per host profile, and the words a sender writes: a value is a quoted-escaped word (\\" a quote,
\\\\ a backslash), never a bare one, so a property's text round-trips byte for byte.
"""
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
failures = []


def check(ok, what):
    print(("ok   " if ok else "FAIL ") + what)
    if not ok:
        failures.append(what)


table = json.load(open(os.path.join(ROOT, "protocol", "plugin-hostd.json")))
grammar = open(os.path.join(ROOT, "grammar", "plugin-hostd-protocol.abnf")).read()
header = open(os.path.join(ROOT, "include", "plugin-hostd", "omx_clap_core_host_verbs.h")).read()

VERBS = ["property_list", "property_info", "property_get", "property_set"]
for v in VERBS:
    check('#define OMX_VERB_%s "%s"' % (v.upper(), v) in header, "%s is rendered from omx-clap-core's declarations" % v)
    rows = [r for r in table["instance_verbs"] if r["name"] == v]
    check(len(rows) == 1, "%s is an instance verb the daemon relays" % v)
    levels = [r for r in table["host_verbs"] if r["name"] == v]
    check(len(levels) == 1 and set(levels[0]["levels"].values()) == {"optional"},
          "%s has a level per host profile, optional for each: %s" % (v, levels[0]["levels"] if levels else None))
    check(re.search(r"^cmd-%s\s+= " % v.replace("_", "-"), grammar, re.M) is not None, "%s has its own grammar rule" % v)

ledger = {r["name"]: r.get("ledger") for r in table["instance_verbs"] if r["name"] in VERBS}
check(all(ledger.get(v) == "none" for v in ("property_list", "property_info", "property_get")),
      "the reads are not kept by the ledger: %s" % ledger)

forms = {f["verb"]: f["args"] for f in table["command_forms"] if f["verb"].startswith("property_")}
check(forms.get("property_set", "").split()[-1:] == ["value:quoted-escaped"],
      "property_set's value is a quoted-escaped word: %s" % forms.get("property_set"))
replies = {r["verb"]: r["args"] for r in table["reply_args"] if r["verb"].startswith("property_")}
check(replies.get("property_get") == "value:quoted-escaped", "property_get answers its value as a quoted-escaped word: %s"
      % replies.get("property_get"))
check(re.search(r'^cmd-property-set\s+= %s"property_set" SP int SP word SP quoted-escaped\s*$', grammar, re.M) is not None,
      "the grammar's property_set ends in a quoted-escaped value")

print("property rows: %s" % ("ok" if not failures else "%d failed" % len(failures)))
sys.exit(1 if failures else 0)
