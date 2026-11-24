#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas
# info.x-requires is a sorted list of distinct names, or absent (owner rulings 2026-11-24: no hand-kept floor, and a
# file that requires nothing writes no field). tools/protocol_words.requires() is the one reader the tools and the
# protocol document use: it must give the list, give [] when the field is absent, and refuse by name the old map of
# floors, an empty list and an unsorted one. Each arm runs the reader on a scratch copy of the domain.
import json, os, subprocess, sys, tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DOMAIN = os.path.join(ROOT, "contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json")
failures = []


def run(info_change):
    doc = json.load(open(DOMAIN))
    info_change(doc["info"])
    with tempfile.TemporaryDirectory() as d:
        path = os.path.join(d, "domain.json")
        json.dump(doc, open(path, "w"))
        code = "import sys; sys.path.insert(0, %r); import protocol_words; print(protocol_words.requires(%r))" % (
            os.path.join(ROOT, "tools"), path)
        r = subprocess.run([sys.executable, "-c", code], capture_output=True, text=True)
        return r.returncode, r.stdout.strip(), r.stderr.strip()


def check(ok, what):
    print(("ok   " if ok else "FAIL ") + what)
    if not ok:
        failures.append(what)


rc, out, _ = run(lambda i: None)
check(rc == 0 and out == repr(json.load(open(DOMAIN))["info"]["x-requires"]), "the tree's list is read as written")
rc, out, _ = run(lambda i: i.pop("x-requires", None))
check(rc == 0 and out == "[]", "an absent x-requires reads as no name")
for what, change, word in (
        ("the old map of floors", lambda i: i.__setitem__("x-requires", {"host": ">= 1.0.0"}), "not a list"),
        ("an empty list", lambda i: i.__setitem__("x-requires", []), "is []"),
        ("an unsorted list", lambda i: i.__setitem__("x-requires", ["hosting-verb", "host"]), "not a sorted list")):
    rc, _, err = run(change)
    check(rc != 0 and word in err and "x-requires" in err, "%s is refused by name (%s)" % (what, err[:90]))
sys.exit(1 if failures else 0)
