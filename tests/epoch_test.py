#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/epoch_test.py: no package of this source carries an epoch.

The owner's ruling (2026-11-18) is NO epoch: the published rpm channel carries no package with one. An epoch in this
source made the staged repository hold python3-plugin-hostd-protocol twice, as 0.6.0 and as 1:0.6.0, and the protocol
0.6.0 release dry run failed on it (44291974917). This refuses an Epoch in the spec, an epoch macro, an N: version in a
relation of the spec or debian/control, a -vN: in debian/rules and an N: version at the top of debian/changelog.
Usage: epoch_test.py [root]. Each fault is planted in a copy of the tree and must be seen before the tree is trusted.
"""
import os
import re
import sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
failures = []


def check(ok, what):
    print(("ok   " if ok else "FAIL ") + what)
    if not ok:
        failures.append(what)


def live(text):
    """(number, line) of each line of a file that is not a comment, numbered as the file numbers it"""
    return [(n, line) for n, line in enumerate(text.splitlines(), 1) if not line.lstrip().startswith("#")]


def problems(spec, control, rules, changelog):
    out = []
    for n, line in live(spec):
        if re.match(r"\s*Epoch\s*:", line, re.I):
            out.append("packaging/plugin-hostd-protocol.spec:%d: an Epoch: %s" % (n, line.strip()))
        if re.search(r"%\{\??epoch\}", line):
            out.append("packaging/plugin-hostd-protocol.spec:%d: an epoch macro: %s" % (n, line.strip()))
        if re.match(r"(Requires|BuildRequires|Provides|Obsoletes|Conflicts|Recommends|Suggests|Supplements|Enhances)\b", line) \
                and re.search(r"[<>=]=?\s*\d+:", line):
            out.append("packaging/plugin-hostd-protocol.spec:%d: an N: version relation: %s" % (n, line.strip()))
    for n, line in live(control):
        if re.search(r"[<>=]=?\s*\d+:\d", line):
            out.append("debian/control:%d: an N: version relation: %s" % (n, line.strip()))
    for n, line in live(rules):
        if re.search(r"-v\s*\d+:", line):
            out.append("debian/rules:%d: an N: version passed to dh_gencontrol: %s" % (n, line.strip()))
    top = next((l for l in changelog.splitlines() if l.strip()), "")
    if re.match(r"\S+ \(\d+:", top):
        out.append("debian/changelog: the top version carries an epoch: %s" % top.strip())
    return out


def plant_top_epoch(changelog):
    """the changelog with an epoch planted into its FIRST version, whatever that version is (no-op if none)"""
    m = re.search(r"^(\S+ \()([^)\s]+)(\))", changelog, re.M)
    if not m:
        return changelog
    return changelog[:m.start()] + m.group(1) + "1:" + m.group(2) + m.group(3) + changelog[m.end():]


def read(*parts):
    with open(os.path.join(ROOT, *parts), encoding="utf-8") as f:
        return f.read()


spec = read("packaging", "plugin-hostd-protocol.spec")
control = read("debian", "control")
rules = read("debian", "rules")
changelog = read("debian", "changelog")

# the changelog plant goes into the FIRST version, whatever it is: a release commit's top is not always 0.5.9
planted = plant_top_epoch(changelog)
check(planted != changelog, "the changelog plant changed the text (a no-op plant would test nothing)")

# the check sees each fault: an Epoch line, an epoch macro, a relation with N:, a control relation, rules and changelog
seen = all(problems(*f) for f in (
    (spec + "Epoch: 1\n", control, rules, changelog),
    (spec + "Requires: x = %{epoch}:%{version}\n", control, rules, changelog),
    (spec + "Requires: plugin-hostd-protocol >= 1:0.5.8\n", control, rules, changelog),
    (spec, control + "Depends: libx (= 1:0.5~)\n", rules, changelog),
    (spec, control, rules + "\tdh_gencontrol -ppython3-x -- -v1:$(DEB_VERSION)\n", changelog),
    (spec, control, rules, planted),
))
check(seen, "the epoch check sees an epoch in each file and in each form")

bad = problems(spec, control, rules, changelog)
for b in bad:
    check(False, b)
check(not bad, "no package of this source carries an epoch")

sys.exit(1 if failures else 0)
