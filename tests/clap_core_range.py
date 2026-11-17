#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Every omx-clap-core-contract range this tree declares (the spec's BuildRequires and Requires, debian/control's
# Build-Depends and Depends) is a floor and nothing else (owner rule 2026-11-16: floors only, no ceilings): the floor
# admits the omx-clap-core minor released after this one, and no ceiling can make that release uninstallable beside this
# package. Usage: clap_core_range.py [root]. The check proves it sees each fault before it trusts the tree.
import os, re, sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PKG = "omx-clap-core-contract"
ADMIT = "0.33.0"  # the release every range must admit
WANT = {"spec": 2, "control": 2}  # declarations: spec BuildRequires + Requires; control Build-Depends + one Depends


def ver(v):
    return tuple(int(x) for x in v.split("."))


def problems(spec, control):
    out = []
    rs = re.findall(r"^(?:Build)?Requires: %s >= ([0-9.]+)$" % PKG, spec, re.M)
    if len(rs) != WANT["spec"]:
        out.append("spec: %d floors of %s, want %d" % (len(rs), PKG, WANT["spec"]))
    for lo in rs:
        if ver(lo) > ver(ADMIT):
            out.append("spec: floor %s is above %s" % (lo, ADMIT))
    if re.findall(r"%s *(?:<|\(<<|\(<=)" % PKG, spec + control) or re.search(r"%s[^\n]* with %s <" % (PKG, PKG), spec):
        out.append("a ceiling on %s: floors only" % PKG)
    lows = re.findall(r"%s \(>= ([0-9.]+)~\)" % PKG, control)
    if len(lows) != WANT["control"]:
        out.append("debian/control: %d floors of %s, want %d" % (len(lows), PKG, WANT["control"]))
    for lo in lows:
        if ver(lo) > ver(ADMIT):
            out.append("debian/control: floor %s is above %s" % (lo, ADMIT))
    return out


spec = open(os.path.join(ROOT, "packaging/plugin-hostd-protocol.spec")).read()
control = open(os.path.join(ROOT, "debian/control")).read()
# the check sees each fault: a ceiling in the spec, a ceiling in the control, a floor above the next release, a range gone
seen = all(problems(*f) for f in (
    (spec + "Requires: %s >= 0.32.3 with %s < 0.34\n" % (PKG, PKG), control),
    (spec, control.replace("%s (>= 0.32.3~)," % PKG, "%s (>= 0.32.3~), %s (<< 0.34~)," % (PKG, PKG), 1)),
    (re.sub(r"%s >= [0-9.]+" % PKG, PKG + " >= 0.34.0", spec), control),
    (re.sub(r"BuildRequires: %s[^\n]*\n" % PKG, "", spec), control)))
print(("ok  " if seen else "FAIL") + " the clap-core range check sees a ceiling, a high floor and a missing range")
bad = problems(spec, control)
for b in bad:
    print("FAIL " + b)
if not bad:
    print("ok   every %s range is a floor admitting %s" % (PKG, ADMIT))
sys.exit(0 if seen and not bad else 1)
