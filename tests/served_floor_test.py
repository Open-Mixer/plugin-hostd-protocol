#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/served_floor_test.py: tools/served-floor.py refuses a version below what the channel ever served (#180).

A channel repository is made here with a history: python3-plugin-hostd-protocol served at 0.20.6 by another source,
then pruned, then served at 0.8.1. Each arm runs the tool on it and must see what it is built to see; the arm that
plants nothing must pass. Usage: served_floor_test.py [root]
"""
import os
import subprocess
import sys
import tempfile

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOL = os.path.join(ROOT, "tools", "served-floor.py")
HEAD = "# package\tversion\tset\tcells\n"
failures = []


def check(ok, what):
    print(("ok   " if ok else "FAIL ") + what)
    if not ok:
        failures.append(what)


def git(repo, *args):
    subprocess.run(["git", "-C", repo, "-c", "user.name=t", "-c", "user.email=t@example.org", "-c", "commit.gpgsign=false"]
                   + list(args), check=True, capture_output=True)


def channel(tmp, commits):
    """a git repository whose deb/rpm ledgers go through the given states, one commit each"""
    repo = os.path.join(tmp, "chan")
    os.makedirs(os.path.join(repo, "deb"))
    os.makedirs(os.path.join(repo, "rpm"))
    git(repo, "init", "-q")
    for deb, rpm in commits:
        for kind, rows in (("deb", deb), ("rpm", rpm)):
            with open(os.path.join(repo, kind, "cells.tsv"), "w") as f:
                f.write(HEAD + "".join("%s\t%s\t-\ttrixie/arm64\n" % r for r in rows))
        git(repo, "add", "-A")
        git(repo, "commit", "-q", "--allow-empty", "-m", "publish")
    return repo


def run(repo, version):
    p = subprocess.run([sys.executable, TOOL, "--root", ROOT, "--channel", repo, "--version", version],
                       capture_output=True, text=True)
    return p.returncode, p.stdout + p.stderr


OLD = [("python3-plugin-hostd-protocol", "0.20.6"), ("plugin-hostd", "0.20.6")]
NOW = [("python3-plugin-hostd-protocol", "0.8.1"), ("libplugin-hostd-protocol0", "0.8.1")]
RPM = [("plugin-hostd-protocol-devel", "0.30.0")]

with tempfile.TemporaryDirectory() as tmp:
    pruned = channel(os.path.join(tmp, "a"), [(OLD, []), ([], []), (NOW, [])])
    rc, out = run(pruned, "0.8.1")
    check(rc == 1 and "python3-plugin-hostd-protocol 0.8.1 is below 0.20.6" in out,
          "a version pruned from the channel still counts: 0.8.1 is refused below the 0.20.6 served before")
    rc, out = run(pruned, "0.21.0")
    check(rc == 0, "0.21.0 is above every version served: passes (%s)" % out.strip())
    rc, out = run(pruned, "0.20.6")
    check(rc == 0, "the same version as the highest served is not refused here (the preflight refuses a republish)")
    rc, out = run(pruned, "0.20.6~rc1")
    check(rc == 1, "a ~ version sorts below its release: 0.20.6~rc1 is refused")
    check("plugin-hostd 0" not in out, "a package this source does not build is not judged (plugin-hostd)")

    rpm = channel(os.path.join(tmp, "b"), [(NOW, RPM)])
    rc, out = run(rpm, "0.21.0")
    check(rc == 1 and "plugin-hostd-protocol-devel 0.21.0 is below 0.30.0" in out,
          "the rpm ledger is read, and a spec subpackage is named <Name>-<sub>")

    tip = channel(os.path.join(tmp, "c"), [(OLD, [])])
    rc, out = run(tip, "0.8.1")
    check(rc == 1, "a ledger written in the first commit is read")

    empty = channel(os.path.join(tmp, "d"), [([], [])])
    rc, out = run(empty, "0.21.0")
    check(rc == 2, "a channel whose ledgers hold no row is not a pass")
    rc, out = run(os.path.join(tmp, "nowhere"), "0.21.0")
    check(rc == 2, "a channel that cannot be read is not a pass")

sys.exit(1 if failures else 0)
