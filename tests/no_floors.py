#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas
# No hand-kept version (owner ruling 2026-11-24): a lower level publishes its packages and this tree builds and installs
# against them, so no requirement types a version. The spec's BuildRequires, Requires and Recommends, debian/control's
# Build-Depends, Depends, Pre-Depends and Recommends, and the @openmixer dependencies of every package.json are read:
# a version on any of them is refused (a floor, a ceiling, a rich `with`), except the generated relations between this
# tree's own packages, `= %{version}-%{release}` and `(= ${source:Version})` / `(= ${binary:Version})`, and debhelper's
# compatibility level. The Makefile asks pkg-config for a package by name: a version (`'x >= 1.0'`,
# --atleast-version, --exact-version, --max-version) is refused. A domain's info.x-requires and a data file's requires
# name packages and domains, never a version: the old map of floors is refused. Prose is no exception: "<our package>
# X.Y.Z or later" (or newer, or above) in the spec (outside %changelog), debian/control, the Makefile, the headers, the
# sources, the tools, the domains, the protocol documents and doc/ is refused by file and line. packaging/deps.txt,
# the old home of the floors, must not exist. Usage: no_floors.py [root].
# The check proves it sees each fault before it trusts the tree.
import glob, json, os, re, sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SPEC = "packaging/plugin-hostd-protocol.spec"
CONTROL = "debian/control"
PACKAGES = ("package.json", "clients/typescript/codec/package.json")
GENERATED_RPM = re.compile(r"= %\{version\}-%\{release\}")
GENERATED_DEB = re.compile(r"\(= \$\{(source|binary):Version\}\)|^debhelper-compat \(= \d+\)$")


PKG_CONFIG_LINE = re.compile(r"pkg-config|PKG_CONFIG\)")
PKG_CONFIG_VERSION = re.compile(r"--(atleast|exact|max)-version|(>=|<=|<|>) *[0-9$]|[a-z0-9_.+-] += +[0-9$]")


OUR = r"(?:omx-[a-z-]+|omxdsp|libomx[a-z-]*|plugin-hostd[a-z-]*|plugin-db[a-z-]*|plugin-qualif[a-z-]*|openmixer[a-z-]*)"
PROSE_FLOOR = re.compile(r"\b%s[ -]+v?[0-9]+\.[0-9]+(?:\.[0-9]+)? +or +(?:later|newer|above)" % OUR, re.I)
PROSE_GLOBS = ("include/*.h", "include/plugin-hostd/*.h", "src/*.c", "src/*.h", "tools/*.py", "tools/*.c",
               "contract.d/*/*.json", "protocol/*.json", "doc/*.md")


def prose(texts):
    out = []
    for rel, text in sorted(texts.items()):
        if rel.endswith(".spec"):
            text = text.split("%changelog")[0]
        for n, line in enumerate(text.splitlines(), 1):
            m = PROSE_FLOOR.search(line)
            if m:
                out.append("%s:%d: a version in prose: %s" % (rel, n, m.group(0)))
    return out


def problems(spec, control, packages, deps_txt, makefile="", domains=None, texts=None):
    out = prose(texts or {})
    for n, line in enumerate(makefile.splitlines(), 1):
        if not line.lstrip().startswith("#") and PKG_CONFIG_LINE.search(line) and PKG_CONFIG_VERSION.search(line):
            out.append("Makefile:%d: pkg-config asked for a version: %s" % (n, line.strip()[:120]))
    for rel, text in sorted((domains or {}).items()):
        doc = json.loads(text)
        for where, req in (("info.x-requires", doc.get("info", {}).get("x-requires")), ("requires", doc.get("requires"))):
            if req is None:
                continue
            if not isinstance(req, list) or not req or any(not isinstance(x, str) or re.search(r"[<>=]|\d+\.\d+", x) for x in req):
                out.append("%s: %s is %s: a list of names, never a version (and never [])" % (rel, where, json.dumps(req)[:80]))
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


spec, control, makefile = read(SPEC), read(CONTROL), read("Makefile")
domains = {os.path.relpath(p, ROOT): read(os.path.relpath(p, ROOT))
           for p in sorted(glob.glob(os.path.join(ROOT, "contract.d/*/*.json")) + glob.glob(os.path.join(ROOT, "data/*.json")))}
texts = {SPEC: spec, CONTROL: control, "Makefile": makefile}
for pattern in PROSE_GLOBS:
    for path in sorted(glob.glob(os.path.join(ROOT, pattern))):
        texts[os.path.relpath(path, ROOT)] = read(os.path.relpath(path, ROOT))
PROTOCOL_H = "include/plugin-hostd/protocol.h"
domain = "contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json"
as_map = json.loads(domains[domain])
as_map["info"]["x-requires"] = {n: ">= 1.0.0" for n in as_map["info"].get("x-requires", ["host"])}
as_empty = json.loads(domains[domain])
as_empty["info"]["x-requires"] = []
packages = {p: read(p) for p in PACKAGES}
deps_txt = os.path.exists(os.path.join(ROOT, "packaging/deps.txt"))
# the check sees each fault: a spec floor, a rich ceiling, a control floor, an npm range, a deps.txt back
seen = all(problems(*f) for f in (
    (spec.replace("BuildRequires: omx-dsp-devel\n", "BuildRequires: omx-dsp-devel >= 0.34.0\n", 1), control, packages, False, makefile, domains),
    (spec.replace("BuildRequires: omx-dsp-devel\n", "BuildRequires: (omx-dsp-devel with omx-dsp-devel < 1)\n", 1), control, packages, False, makefile, domains),
    (spec, control.replace("libomxdsp-dev,", "libomxdsp-dev (>= 0.34.0~),", 1), packages, False, makefile, domains),
    (spec, control, dict(packages, **{"package.json": packages["package.json"].replace('"@openmixer/omx-contract": "*"', '"@openmixer/omx-contract": ">=3.18.0"', 1)}), False, makefile, domains),
    (spec, control, packages, True, makefile, domains),
    (spec, control, packages, False, makefile.replace("--cflags omxdsp ", "--cflags 'omxdsp >= 0.34.0' ", 1), domains),
    (spec, control, packages, False, makefile.replace("--cflags omxdsp ", "--atleast-version=0.34.0 --cflags omxdsp ", 1), domains),
    (spec, control, packages, False, makefile, dict(domains, **{domain: json.dumps(as_map)})),
    (spec, control, packages, False, makefile, dict(domains, **{domain: json.dumps(as_empty)})),
    (spec, control, packages, False, makefile, domains, dict(texts, **{PROTOCOL_H: texts[PROTOCOL_H].replace(
        "relayed from an omx-clap-host worker,", "relayed from a worker (omx-clap-host 0.26.2 or later),", 1)}))))
print(("ok  " if seen else "FAIL") + " the check sees a typed version in the spec, a rich one, one in debian/control, an npm range,"
      " a deps.txt, a pkg-config floor in the Makefile (two forms), an x-requires map, an empty x-requires and a"
      " version in prose")
bad = problems(spec, control, packages, deps_txt, makefile, domains, texts)
for b in bad:
    print("FAIL " + b)
if not bad:
    print("ok   no requirement types a version; the own-package relations are generated; no packaging/deps.txt;"
          " pkg-config is asked by name; every x-requires is a list of names; no prose states a version")
sys.exit(0 if seen and not bad else 1)
