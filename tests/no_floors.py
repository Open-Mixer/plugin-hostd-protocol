#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas
# No hand-kept version (owner ruling 2026-11-24): a lower level publishes its packages and this tree builds and installs
# against them, so no requirement types a version. The spec's BuildRequires, Requires and Recommends, debian/control's
# Build-Depends, Depends, Pre-Depends and Recommends, and the @openmixer dependencies of every package.json are read:
# a version on any of them is refused (a floor, a ceiling, a rich `with`), except the generated relations between this
# tree's own packages, `= %{version}-%{release}` and `(= ${source:Version})` / `(= ${binary:Version})`, and debhelper's
# compatibility level. packaging/deps.txt, the old home of the floors, must not exist. Usage: no_floors.py [root].
# The check proves it sees each fault before it trusts the tree.
import json, os, re, sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SPEC = "packaging/plugin-hostd-protocol.spec"
CONTROL = "debian/control"
PACKAGES = ("package.json", "clients/typescript/codec/package.json")
GENERATED_RPM = re.compile(r"= %\{version\}-%\{release\}")
GENERATED_DEB = re.compile(r"\(= \$\{(source|binary):Version\}\)|^debhelper-compat \(= \d+\)$")


def problems(spec, control, packages, deps_txt):
    out = []
    if deps_txt:
        out.append("packaging/deps.txt exists: no floor is kept by hand")
    for n, line in enumerate(spec.split("%changelog")[0].splitlines(), 1):
        if re.match(r"(BuildRequires|Requires|Recommends)(\([^)]*\))?:", line):
            rest = GENERATED_RPM.sub("", line.split("#")[0])
            if re.search(r"\bwith\b|[<>=]", rest.split(":", 1)[1]):
                out.append("%s:%d: a typed version: %s" % (SPEC, n, line.strip()))
    field = None
    for n, line in enumerate(control.splitlines(), 1):
        m = re.match(r"([A-Za-z-]+):(.*)", line)
        if m:
            field, value = m.group(1), m.group(2)
        elif line.startswith(" ") and field:
            value = line
        else:
            field = None
            continue
        if field in ("Build-Depends", "Build-Depends-Indep", "Build-Depends-Arch", "Depends", "Pre-Depends", "Recommends"):
            for dep in (d.strip() for d in re.split(r"[,|]", value)):
                if "(" in dep and not GENERATED_DEB.search(dep):
                    out.append("%s:%d: a typed version: %s" % (CONTROL, n, dep))
    for rel, text in packages.items():
        doc = json.loads(text)
        for key in ("dependencies", "peerDependencies", "devDependencies", "optionalDependencies"):
            for name, want in (doc.get(key) or {}).items():
                if name.startswith("@openmixer/") and want != "*":
                    out.append("%s: %s %s is %r: an OpenMixer package is named, never versioned (\"*\")" % (rel, key, name, want))
    return out


def read(rel):
    with open(os.path.join(ROOT, rel)) as f:
        return f.read()


spec, control = read(SPEC), read(CONTROL)
packages = {p: read(p) for p in PACKAGES}
deps_txt = os.path.exists(os.path.join(ROOT, "packaging/deps.txt"))
# the check sees each fault: a spec floor, a rich ceiling, a control floor, an npm range, a deps.txt back
seen = all(problems(*f) for f in (
    (spec.replace("BuildRequires: omx-dsp-devel\n", "BuildRequires: omx-dsp-devel >= 0.34.0\n", 1), control, packages, False),
    (spec.replace("BuildRequires: omx-dsp-devel\n", "BuildRequires: (omx-dsp-devel with omx-dsp-devel < 1)\n", 1), control, packages, False),
    (spec, control.replace("libomxdsp-dev,", "libomxdsp-dev (>= 0.34.0~),", 1), packages, False),
    (spec, control, dict(packages, **{"package.json": packages["package.json"].replace('"@openmixer/omx-contract": "*"', '"@openmixer/omx-contract": ">=3.18.0"', 1)}), False),
    (spec, control, packages, True)))
print(("ok  " if seen else "FAIL") + " the check sees a typed version in the spec, a rich one, one in debian/control, an npm range and a deps.txt")
bad = problems(spec, control, packages, deps_txt)
for b in bad:
    print("FAIL " + b)
if not bad:
    print("ok   no requirement types a version; the own-package relations are generated; no packaging/deps.txt")
sys.exit(0 if seen and not bad else 1)
