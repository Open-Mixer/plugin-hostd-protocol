#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Every omx-clap-core-contract range this tree declares (the spec's BuildRequires and Requires, debian/control's
# Build-Depends and Depends) admits the omx-clap-core minor released after this one: the protocol is released first in
# the chain and the omx-clap-core release that follows it reads the protocol, so a ceiling at the current minor would
# make the next omx-clap-core uninstallable beside this package. Each range is a floor and a ceiling; the ceiling is
# NEXT. Usage: clap_core_range.py [root]. The check proves it sees each fault before it trusts the tree.
import os, re, sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PKG = "omx-clap-core-contract"
NEXT = "0.34"     # the ceiling: the minor after the one the chain releases next (0.33)
ADMIT = "0.33.0"  # the release every range must admit
WANT = {"spec": 2, "control": 2}  # declarations: spec BuildRequires + Requires; control Build-Depends + one Depends


def ver(v):
    return tuple(int(x) for x in v.split("."))


def problems(spec, control):
    out = []
    rs = re.findall(r"\(%s >= ([0-9.]+) with %s < ([0-9.]+)\)" % (PKG, PKG), spec)
    if len(rs) != WANT["spec"]:
        out.append("spec: %d ranges of %s, want %d" % (len(rs), PKG, WANT["spec"]))
    for lo, hi in rs:
        if hi != NEXT or not ver(lo) <= ver(ADMIT) < ver(hi):
            out.append("spec: %s >= %s < %s does not admit %s below %s" % (PKG, lo, hi, ADMIT, NEXT))
    lows = re.findall(r"%s \(>= ([0-9.]+)~\)" % PKG, control)
    highs = re.findall(r"%s \(<< ([0-9.]+)~\)" % PKG, control)
    if len(lows) != WANT["control"] or len(highs) != WANT["control"]:
        out.append("debian/control: %d floors and %d ceilings of %s, want %d of each" % (len(lows), len(highs), PKG, WANT["control"]))
    for lo in lows:
        if ver(lo) > ver(ADMIT):
            out.append("debian/control: floor %s is above %s" % (lo, ADMIT))
    for hi in highs:
        if hi != NEXT:
            out.append("debian/control: ceiling << %s~, want << %s~" % (hi, NEXT))
    return out


spec = open(os.path.join(ROOT, "packaging/plugin-hostd-protocol.spec")).read()
control = open(os.path.join(ROOT, "debian/control")).read()
# the check sees each fault: a ceiling at the current minor, a floor above the next release, a range gone
seen = all(problems(*f) for f in (
    (spec.replace("< %s)" % NEXT, "< 0.33)"), control),
    (spec, control.replace("(<< %s~)" % NEXT, "(<< 0.33~)")),
    (re.sub(r"%s >= [0-9.]+" % PKG, PKG + " >= 0.34.0", spec), control),
    (re.sub(r"BuildRequires: \(%s[^\n]*\n" % PKG, "", spec), control)))
print(("ok  " if seen else "FAIL") + " the clap-core range check sees a current-minor ceiling, a high floor and a missing range")
bad = problems(spec, control)
for b in bad:
    print("FAIL " + b)
if not bad:
    print("ok   every %s range admits %s below %s" % (PKG, ADMIT, NEXT))
sys.exit(0 if seen and not bad else 1)
