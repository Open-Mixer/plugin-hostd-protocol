#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/no_plugin_db_read.py: this tree's build reads nothing of plugin-db (#163, plugin-db#34 step 4).

plugin-db builds on this protocol, and this protocol rendered plugin-db's package (plugin_db_vocabulary.h) from
plugin-db's installed contract: a build cycle. Under ruling R5 the protocol is the lower layer: the pin words it shares
with plugin-db are its own domain pin (#164), plugin-db reads them from here, and plugin-hostd reads plugin-db's words
from plugin-db's own header. Refused, by file and line:
  - a package of plugin-db (plugin-db, plugin-db-*, libplugin-db*) among the relations of the spec (BuildRequires,
    Requires) or of debian/control (Build-Depends, Depends);
  - a Makefile line (not a comment) or a workflow line that names plugin-db's package, its contract or its header
    (plugin-db, plugin_db, PLUGIN_DB);
  - a header rendered from plugin-db's package (include/plugin-hostd/plugin_db_*.h), or a C file or a header that
    includes one;
  - a file installed from this tree under the name of plugin-db's header (tests/install-check.sh's list).
Usage: no_plugin_db_read.py [root]. Each fault is planted in memory, and must be seen, as a finding the tree itself
does not have, before the tree is trusted.
"""
import glob
import os
import re
import sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DB_PACKAGE = re.compile(r"(?<![\w-])(?:lib)?plugin-db(?:-[a-z0-9]+)*(?![\w-])")
# the check's own name (test-no-plugin-db-read, tests/no_plugin_db_read.py) is no read of plugin-db
DB_WORD = re.compile(r"(?<!no-)plugin-db|(?<!no_)plugin_db|PLUGIN_DB")
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
    for rel in ["packaging/plugin-hostd-protocol.spec", "debian/control", "Makefile", "tests/install-check.sh"]:
        files[rel] = read(rel)
    for pattern in [".github/workflows/*.yml", "src/*.c", "src/*.h", "include/*.h", "include/plugin-hostd/*.h",
                    "tests/*.c", "tools/*.c", "fuzz/*.c"]:
        for path in sorted(glob.glob(os.path.join(ROOT, pattern))):
            files[os.path.relpath(path, ROOT)] = read(os.path.relpath(path, ROOT))
    return files


def problems(files):
    out = []
    for n, line in live(files["packaging/plugin-hostd-protocol.spec"]):
        if re.match(r"(BuildRequires|Requires)\b", line) and DB_PACKAGE.search(line):
            out.append("packaging/plugin-hostd-protocol.spec:%d: a relation to plugin-db's: %s" % (n, line.strip()))
    relation = None
    for n, line in enumerate(files["debian/control"].splitlines(), 1):
        if re.match(r"[A-Za-z-]+:", line):
            relation = line.split(":", 1)[0]
        if relation in ("Build-Depends", "Depends") and DB_PACKAGE.search(line.split(":", 1)[-1]):
            out.append("debian/control:%d: a relation to plugin-db's: %s" % (n, line.strip()))
    for rel in ["Makefile"] + sorted(r for r in files if r.startswith(".github/workflows/")):
        for n, line in live(files[rel]):
            if DB_WORD.search(line):
                out.append("%s:%d: names plugin-db: %s" % (rel, n, line.strip()[:160]))
    for n, line in live(files["tests/install-check.sh"]):
        if "plugin_db_" in line:
            out.append("tests/install-check.sh:%d: installs a header of plugin-db's: %s" % (n, line.strip()[:160]))
    for rel in sorted(files):
        if re.match(r"include/plugin-hostd/plugin_db_[^/]*\.h$", rel):
            out.append("%s: a header rendered from plugin-db's package" % rel)
        elif rel.endswith((".c", ".h")):
            for n, line in enumerate(files[rel].splitlines(), 1):
                if re.match(r"\s*#\s*include\s*[<\"][^>\"]*plugin_db_", line):
                    out.append("%s:%d: includes a header of plugin-db's: %s" % (rel, n, line.strip()))
    return out


def planted(files, rel, text):
    """a copy of files with rel set to text"""
    copy = dict(files)
    copy[rel] = text
    return copy


files = tree()
faults = {
    "a BuildRequires": planted(files, "packaging/plugin-hostd-protocol.spec",
                               "BuildRequires: plugin-db-contract\n" + files["packaging/plugin-hostd-protocol.spec"]),
    "a Build-Depends": planted(files, "debian/control", files["debian/control"].replace(
        "Build-Depends: ", "Build-Depends: plugin-db-contract, ", 1)),
    "a Makefile render": planted(files, "Makefile", files["Makefile"]
                                 + "\t$(OMX_CONTRACT) render --target c --package plugin-db --installed --out x\n"),
    "a workflow install": planted(files, ".github/workflows/ci.yml", files.get(".github/workflows/ci.yml", "")
                                  + "          dnf -y install omx-contract plugin-db-contract\n"),
    "a rendered header": planted(files, "include/plugin-hostd/plugin_db_vocabulary.h", "/* rendered */\n"),
    "an include": planted(files, "include/plugin-hostd/protocol.h", '#include "plugin_db_vocabulary.h"\n'
                          + files["include/plugin-hostd/protocol.h"]),
    "an installed header": planted(files, "tests/install-check.sh", files["tests/install-check.sh"]
                                   + "for h in plugin_db_vocabulary.h; do :; done\n"),
}
bad = problems(files)
for what, copy in faults.items():
    check(bool(set(problems(copy)) - set(bad)), "the check sees %s that reads plugin-db" % what)
for b in bad:
    check(False, b)
if not bad:
    check(True, "the build reads nothing of plugin-db: no relation, render, install, header or include")
sys.exit(1 if failures else 0)
