#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/no_fedora44_test.py: no workflow of this source names Fedora 44.

The owner's ruling (2026-11-19): the development matrix is Fedora 45 x86_64 and Debian trixie arm64, and Fedora 44 is
dropped everywhere. This refuses a fedora:44 image, a "44" in an rpm-releases / fedora list and an fc44 name in any
workflow, comments aside. Usage: no_fedora44_test.py [root]. Each fault is planted in a copy of a workflow and must
be seen before the tree is trusted.
"""
import glob
import os
import re
import sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
failures = []


def check(ok, what):
    print(("ok   " if ok else "FAIL ") + what)
    if not ok:
        failures.append(what)


def problems(name, text):
    out = []
    for n, line in enumerate(text.splitlines(), 1):
        if line.lstrip().startswith("#"):
            continue
        line = line.split(" #", 1)[0]
        if re.search(r"fedora:\s*44\b|fedora-44|\bfc44\b", line) \
                or re.search(r"(rpm-releases|fedora|releases)\s*:.*[\[\s,'\"]44\b", line):
            out.append("%s:%d: Fedora 44: %s" % (name, n, line.strip()))
    return out


files = sorted(glob.glob(os.path.join(ROOT, ".github", "workflows", "*.y*ml")))
check(bool(files), "there are workflows to read")
for path in files:
    with open(path) as f:
        text = f.read()
    found = problems(os.path.basename(path), text)
    check(not found, "%s names no Fedora 44%s" % (os.path.basename(path), "".join("\n     " + p for p in found)))

for what, plant in (("a fedora:44 image", "    container:\n      image: fedora:44\n"),
                    ("a 44 in rpm-releases", "          rpm-releases: '[\"44\", \"45\"]'\n"),
                    ("a 44 in a fedora matrix list", "        fedora: [44, 45]\n"),
                    ("an fc44 name", "        name: build fc44 x86_64\n")):
    check(bool(problems("planted", "jobs:\n" + plant)), "the guard sees " + what)
check(not problems("planted", "          rpm-releases: '[\"45\"]'\n    container:\n      image: fedora:45\n# fedora:44 in a comment\n"),
      "the guard lets fedora:45 and a comment through")

sys.exit(1 if failures else 0)
