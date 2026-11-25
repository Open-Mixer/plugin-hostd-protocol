#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/same_pass_test.py: the release workflow reads the same-pass list as plugin-hostd does.

The channel-installability gate reports, rather than fails, a breakage that a package released LATER in the same pass
causes or suffers. The list comes from the same_pass dispatch input, else from .github/release-same-pass at the very
commit the run builds (the tag on a push, the source-ref when one is named), and reaches both shared build jobs. A
release workflow that drops any of those links would turn a same-pass breakage red again, so this refuses each one.
Usage: same_pass_test.py [root]. Each fault is planted in a copy of the release workflow and must be seen before the
workflow is trusted.
"""
import os
import re
import sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RELEASE = os.path.join(ROOT, ".github", "workflows", "release.yml")
PASSED = "same-pass: ${{ inputs.same_pass || needs.samepass.outputs.list }}"
failures = []


def check(ok, what):
    print(("ok   " if ok else "FAIL ") + what)
    if not ok:
        failures.append(what)


def job(text, name):
    """the lines of one job under jobs: (two-space indent), up to the next job"""
    out, inside = [], False
    for line in text.splitlines():
        m = re.match(r"^  ([A-Za-z0-9_-]+):\s*$", line)
        if m:
            if inside:
                break
            inside = m.group(1) == name
            continue
        if inside:
            out.append(line)
    return "\n".join(out)


def audit(text):
    """the names of the same-pass links the workflow text does not carry"""
    bad = []
    if not re.search(r"^      same_pass:\n        description: 'Packages other repositories release later", text, re.M):
        bad.append("the same_pass dispatch input")
    sp = job(text, "samepass")
    if not sp:
        bad.append("the samepass job")
    else:
        if not re.search(r"^\s+sparse-checkout: \.github/release-same-pass\s*$", sp, re.M):
            bad.append("samepass checks out .github/release-same-pass")
        if re.search(r"^\s+ref:", sp, re.M):
            bad.append("samepass pins its checkout to a ref (the built commit is read)")
        if not re.search(r"^\s+f=\.github/release-same-pass\s*$", sp, re.M):
            bad.append("samepass reads .github/release-same-pass")
        if "inputs.source-ref" not in sp:
            bad.append("samepass reads a named source-ref")
    for name in ("rpm", "deb"):
        body = job(text, name)
        # needs: samepass, or a list that names it (needs: [samepass, floor] since the served floor, #180)
        if not re.search(r"^\s+needs: (samepass|\[([^\]]*, *)?samepass( *,[^\]]*)?\])\s*$", body, re.M):
            bad.append(name + " waits for samepass")
        if PASSED not in body:
            bad.append(name + " passes the list to the shared gate")
    return bad


def last(text, old, new):
    """text with the last occurrence of old replaced by new"""
    i = text.rfind(old)
    return text if i < 0 else text[:i] + new + text[i + len(old):]


with open(RELEASE, encoding="utf-8") as fh:
    text = fh.read()

bad = audit(text)
check(not bad, "release.yml carries every same-pass link" + ("" if not bad else ": missing " + "; ".join(bad)))

# each fault is planted in the workflow and must be named by the audit
planted = [
    ("the dispatch input", "      same_pass:\n        description: 'Packages", "      xsame_pass:\n        description: 'Packages", None),
    ("the samepass job's checkout", "sparse-checkout-cone-mode: false", "sparse-checkout-cone-mode: false\n          ref: v0.0.1", None),
    ("the file the job reads", "f=.github/release-same-pass\n", "f=.github/release-same-pass-x\n", None),
    ("the deb job's list", PASSED, "same-pass: ''", "deb"),
    ("the rpm job's wait", "needs: [samepass, floor]", "needs: [floor]", None),
]
for what, old, new, which in planted:
    if which is None:
        mutant = text.replace(old, new, 1)
    else:
        mutant = last(text, old, new)
    if mutant == text:
        check(False, "fault not planted: " + what)
        continue
    check(bool(audit(mutant)), "the audit names a fault in " + what)

if failures:
    print(f"{len(failures)} failure(s)")
    sys.exit(1)
print("same-pass wiring: ok")
