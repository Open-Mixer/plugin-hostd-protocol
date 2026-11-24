#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas
# The probe output is format 2.0 (#172): its cost unit became nsPerSample, a changed code, so the major went to 2 and
# the minor started again at 0. The readers (plugin-qualify, plugin-db-qualification, the catalog) read format 2 only,
# so a header that says 1.x, or a 2.x the readers were not cut for, is a release they cannot read. protocol.h's
# PHD_PROBE_FORMAT_MAJOR and PHD_PROBE_FORMAT_MINOR are held to 2 and 0 here; moving either is a deliberate change of
# this test and of the readers together. The check proves it sees a major and a minor moved before it trusts the tree.
# This tree defines the number and reads no probe output (no tool or client of it parses formatVersion), so a 1.x
# document is refused where it is read: plugin-qualify's readProbeOutput, by the code probe.format.major-unknown.
import os, re, sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HEADER = os.path.join(ROOT, "include/plugin-hostd/protocol.h")
WANT = (2, 0)


def version(text):
    found = {}
    for part in ("MAJOR", "MINOR"):
        m = re.findall(r"^#define PHD_PROBE_FORMAT_%s (\d+)$" % part, text, re.M)
        if len(m) != 1:
            return None
        found[part] = int(m[0])
    return found["MAJOR"], found["MINOR"]


text = open(HEADER).read()
seen = all(version(t) != WANT for t in (
    text.replace("#define PHD_PROBE_FORMAT_MAJOR 2\n", "#define PHD_PROBE_FORMAT_MAJOR 1\n", 1),
    text.replace("#define PHD_PROBE_FORMAT_MINOR 0\n", "#define PHD_PROBE_FORMAT_MINOR 4\n", 1),
    text.replace("#define PHD_PROBE_FORMAT_MINOR 0\n", "", 1)))
print(("ok  " if seen else "FAIL") + " the check sees the major set back to 1, the minor moved to 4 and the minor gone")
have = version(text)
ok = have == WANT
print(("ok  " if ok else "FAIL") + " protocol.h's probe output format is %s, the readers read %d.%d"
      % ("%d.%d" % have if have else "not declared once", *WANT))
sys.exit(0 if seen and ok else 1)
