#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas
# Floors only: no dependency of this tree carries a ceiling. The protocol is released first in the chain and the
# packages above it read it, so a ceiling at a current minor would make their next release uninstallable beside this
# package; the floors are packaging/deps.txt's. The spec's BuildRequires and Requires and debian/control's
# Build-Depends and Depends are read; an upper bound (`with X < N`, `(<< N~)`, `(< N)`) is refused. Usage:
# floors_only.py [root]. The check proves it sees each fault before it trusts the tree.
import os
import re
import sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PKG = "omx-contract-data"


def problems(spec, control):
    out = []
    for n, line in enumerate(spec.splitlines(), 1):
        if re.match(r"(BuildRequires|Requires):", line) and re.search(r"\bwith\b|[a-z0-9)] *<=? *[0-9]", line):
            out.append("spec:%d: a ceiling: %s" % (n, line.strip()))
    for n, line in enumerate(control.splitlines(), 1):
        if re.match(r"(Build-Depends|Depends|Pre-Depends|Recommends):", line) and re.search(r"\((<<|<=) ", line):
            out.append("debian/control:%d: a ceiling: %s" % (n, line.strip()))
    return out


spec = open(os.path.join(ROOT, "packaging/plugin-hostd-protocol.spec")).read()
control = open(os.path.join(ROOT, "debian/control")).read()
# the check sees each fault: a rich-dependency ceiling, a plain one, a control ceiling
seen = all(problems(*f) for f in (
    (spec.replace("BuildRequires: %s >= " % PKG, "BuildRequires: (%s >= 0.1 with %s < 9.9) # " % (PKG, PKG), 1), control),
    (spec + "Requires: omx-contract < 4\n", control),
    (spec, control.replace("%s (>= " % PKG, "%s (<< 9~), %s (>= " % (PKG, PKG), 1))))
print(("ok  " if seen else "FAIL") + " the floors-only check sees a ceiling in each format")
bad = problems(spec, control)
for b in bad:
    print("FAIL " + b)
if not bad:
    print("ok   no dependency carries a ceiling")
sys.exit(0 if seen and not bad else 1)
