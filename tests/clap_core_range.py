#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas
# Floors only: no dependency of this tree carries a ceiling, omx-clap-core-contract's included. The protocol is released
# first in the chain and the next omx-clap-core release reads it, so a ceiling at the current minor would make that
# release uninstallable beside this package; the floors are packaging/deps.txt's. The spec's BuildRequires and Requires
# and debian/control's Build-Depends and Depends are read; an upper bound (`with X < N`, `(<< N~)`, `(< N)`) is refused,
# and the omx-clap-core-contract floor must be there. Usage: clap_core_range.py [root]. The check proves it sees each
# fault before it trusts the tree.
import os, re, sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PKG = "omx-clap-core-contract"


def problems(spec, control):
    out = []
    for n, line in enumerate(spec.splitlines(), 1):
        if re.match(r"(BuildRequires|Requires):", line) and re.search(r"\bwith\b|[a-z0-9)] *<=? *[0-9]", line):
            out.append("spec:%d: a ceiling: %s" % (n, line.strip()))
    for n, line in enumerate(control.splitlines(), 1):
        if re.match(r"(Build-Depends|Depends|Pre-Depends|Recommends):", line) and re.search(r"\((<<|<=) ", line):
            out.append("debian/control:%d: a ceiling: %s" % (n, line.strip()))
    if not re.search(r"^BuildRequires: %s >= [0-9.]+$" % PKG, spec, re.M):
        out.append("spec: no floor of %s" % PKG)
    if not re.search(r"%s \(>= [0-9.]+~\)" % PKG, control):
        out.append("debian/control: no floor of %s" % PKG)
    return out


spec = open(os.path.join(ROOT, "packaging/plugin-hostd-protocol.spec")).read()
control = open(os.path.join(ROOT, "debian/control")).read()
# the check sees each fault: a rich-dependency ceiling, a plain one, a control ceiling, a floor gone
seen = all(problems(*f) for f in (
    (spec.replace("BuildRequires: %s >= " % PKG, "BuildRequires: (%s >= 0.1 with %s < 0.9) # " % (PKG, PKG), 1), control),
    (spec + "Requires: omx-contract < 4\n", control),
    (spec, control.replace("%s (>= " % PKG, "%s (<< 9~), %s (>= " % (PKG, PKG), 1)),
    (re.sub(r"BuildRequires: %s[^\n]*\n" % PKG, "", spec), control)))
print(("ok  " if seen else "FAIL") + " the floors-only check sees a ceiling in each format and a missing floor")
bad = problems(spec, control)
for b in bad:
    print("FAIL " + b)
if not bad:
    print("ok   no dependency carries a ceiling; %s has its floor" % PKG)
sys.exit(0 if seen and not bad else 1)
