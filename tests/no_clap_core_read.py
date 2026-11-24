#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/no_clap_core_read.py: this tree's build reads nothing of omx-clap-core (#67).

The release chain publishes inside out: this protocol first, omx-clap-core after it, reading it. A build of this tree
that reads omx-clap-core is a cycle the chain can only bootstrap from an already-served release, so what the two share
(the domain host, the hosts a verb names, the verbs both hosts answer) is this tree's, and omx-clap-core reads it from
here. Refused, by file and line:
  - a package of omx-clap-core (omx-clap-*) among the floors of packaging/deps.txt, the relations of the spec
    (BuildRequires, Requires) or of debian/control (Build-Depends, Depends);
  - an info.x-requires of a domain under contract.d that names a domain this tree does not ship;
  - a Makefile line (not a comment) or a workflow line that names omx-clap-core, its package or its contract
    (omx-clap, omx_clap, contract.d/omx-clap, CLAP_CORE_REF, clap-core-ref); the worker's name omx-clap-host is a
    word of the protocol, not a read, and so is this check's own name;
  - a header rendered from omx-clap's package (include/plugin-hostd/omx_clap_*.h), or a C file that includes one;
  - a source of the table protocol/plugin-hostd-protocol.json that is no file of this tree.
Usage: no_clap_core_read.py [root]. Each fault is planted in memory, and must be seen, as a finding the tree itself does not have, before the tree is
trusted.
"""
import glob
import json
import os
import re
import sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CLAP_PACKAGE = re.compile(r"(?<![\w-])omx-clap(?:-[a-z0-9]+)*")
# the check's own name (test-no-clap-core-read, tests/no_clap_core_read.py) is no read of omx-clap-core
CLAP_WORD = re.compile(r"omx[-_]clap(?!-host\b)|(?<!no-)clap-core|(?<!no_)clap_core|CLAP_CORE")
failures = []


def check(ok, what):
    print(("ok   " if ok else "FAIL ") + what)
    if not ok:
        failures.append(what)


def read(rel):
    with open(os.path.join(ROOT, rel), encoding="utf-8") as f:
        return f.read()


def live(text):
    """(number, line) of each line that is not a comment, numbered as the file numbers it"""
    return [(n, line) for n, line in enumerate(text.splitlines(), 1) if not line.lstrip().startswith("#")]


def tree():
    """the files the checks read, {relative path: text}"""
    files = {}
    for rel in ["packaging/deps.txt", "packaging/plugin-hostd-protocol.spec", "debian/control", "Makefile",
                "protocol/plugin-hostd-protocol.json"]:
        files[rel] = read(rel)
    for pattern in ["contract.d/*/*.json", ".github/workflows/*.yml", "src/*.c", "src/*.h", "include/*.h",
                    "include/plugin-hostd/*.h", "tests/*.c", "tools/*.c", "fuzz/*.c"]:
        for path in sorted(glob.glob(os.path.join(ROOT, pattern))):
            files[os.path.relpath(path, ROOT)] = read(os.path.relpath(path, ROOT))
    return files


def problems(files):
    out = []
    for n, line in live(files["packaging/deps.txt"]):
        if CLAP_PACKAGE.search(line):
            out.append("packaging/deps.txt:%d: a floor of omx-clap-core's: %s" % (n, line.strip()))
    for n, line in live(files["packaging/plugin-hostd-protocol.spec"]):
        if re.match(r"(BuildRequires|Requires)\b", line) and CLAP_PACKAGE.search(line.replace("omx-clap-host", "")):
            out.append("packaging/plugin-hostd-protocol.spec:%d: a relation to omx-clap-core's: %s" % (n, line.strip()))
    relation = None
    for n, line in enumerate(files["debian/control"].splitlines(), 1):
        if re.match(r"[A-Za-z-]+:", line):
            relation = line.split(":", 1)[0]
        if relation in ("Build-Depends", "Depends") and CLAP_PACKAGE.search(line.split(":", 1)[-1]):
            out.append("debian/control:%d: a relation to omx-clap-core's: %s" % (n, line.strip()))
    domains = {rel.split("/")[1] for rel in files if re.match(r"contract\.d/[^/]+/[^/]+\.json$", rel)}
    for rel in sorted(files):
        if re.match(r"contract\.d/[^/]+/[^/]+\.json$", rel):
            for name in json.loads(files[rel]).get("info", {}).get("x-requires", {}):
                if name not in domains:
                    out.append("%s: info.x-requires names %s, a domain this tree does not ship" % (rel, name))
    for rel in ["Makefile"] + sorted(r for r in files if r.startswith(".github/workflows/")):
        for n, line in live(files[rel]):
            if CLAP_WORD.search(line):
                out.append("%s:%d: names omx-clap-core: %s" % (rel, n, line.strip()[:160]))
    for rel in sorted(files):
        if re.match(r"include/plugin-hostd/omx_clap_[^/]*\.h$", rel):
            out.append("%s: a header rendered from omx-clap's package" % rel)
        elif rel.endswith((".c", ".h")):
            for n, line in enumerate(files[rel].splitlines(), 1):
                if re.match(r"\s*#\s*include\s*[<\"][^>\"]*omx_clap_", line):
                    out.append("%s:%d: includes a header of omx-clap's: %s" % (rel, n, line.strip()))
    for source in json.loads(files["protocol/plugin-hostd-protocol.json"]).get("sources", []):
        if source not in files and not os.path.isfile(os.path.join(ROOT, source)):
            out.append("protocol/plugin-hostd-protocol.json: the source %s is no file of this tree" % source)
    return out


def planted(files, rel, text):
    """a copy of files with rel set to text"""
    copy = dict(files)
    copy[rel] = text
    return copy


files = tree()
table = json.loads(files["protocol/plugin-hostd-protocol.json"])
table["sources"] = table.get("sources", []) + ["contract.d/omx-clap/omx-clap.json"]
domain = "contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json"
requires = json.loads(files[domain])
requires.setdefault("info", {}).setdefault("x-requires", {})["omx-clap"] = ">= 0.34.0"
faults = {
    "a floor in deps.txt": planted(files, "packaging/deps.txt", files["packaging/deps.txt"]
                                   + "omx-clap-contract          omx-clap-contract        0.34.0\n"),
    "a BuildRequires": planted(files, "packaging/plugin-hostd-protocol.spec",
                               "BuildRequires: omx-clap-contract >= 0.34.0\n" + files["packaging/plugin-hostd-protocol.spec"]),
    "a Build-Depends": planted(files, "debian/control", files["debian/control"].replace(
        "Build-Depends: ", "Build-Depends: omx-clap-contract (>= 0.34.0~), ", 1)),
    "an x-requires": planted(files, domain, json.dumps(requires)),
    "a Makefile render": planted(files, "Makefile", files["Makefile"]
                                 + "\t$(OMX_CONTRACT) render --target c --package omx-clap --installed --out x\n"),
    "a workflow clone": planted(files, ".github/workflows/ci.yml", files.get(".github/workflows/ci.yml", "")
                                + "          url=https://github.com/Open-Mixer/omx-clap-core\n"),
    "a rendered header": planted(files, "include/plugin-hostd/omx_clap_vocabulary.h", "/* rendered */\n"),
    "an include": planted(files, "src/protocol.c", '#include "plugin-hostd/omx_clap_host_verbs.h"\n'
                          + files["src/protocol.c"]),
    "a table source": planted(files, "protocol/plugin-hostd-protocol.json", json.dumps(table)),
}
bad = problems(files)
for what, copy in faults.items():
    check(bool(set(problems(copy)) - set(bad)), "the check sees %s that reads omx-clap-core" % what)
for b in bad:
    check(False, b)
if not bad:
    check(True, "the build reads nothing of omx-clap-core: no floor, relation, domain, render, clone, header or source")
sys.exit(1 if failures else 0)
