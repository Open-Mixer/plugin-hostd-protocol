#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# The dependency ranges this tree declares (the spec's BuildRequires and Requires, debian/control's Build-Depends and
# Depends) are each a floor and a ceiling, and the ceiling is the floor's NEXT MINOR: a range spans one minor, never
# two (dep audit 2026-11-16: omx-clap-core-contract '>= 0.32.3 < 0.34' spanned 0.32 and 0.33). The protocol is
# released first in the chain and the omx-clap-core release that follows it reads the protocol, so the range moves to
# the next minor in the pass that releases it (the coordinator rules what rides a wave), never by widening.
# python3-plugin-hostd-protocol's omx-contract floor is what its verbs.json asks for ("omx-contract-schema": ">= X"),
# in the spec and in debian/control alike. Usage: clap_core_range.py [root]. The check proves it sees each fault before
# it trusts the tree.
import os, re, sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PKG = "omx-clap-core-contract"
WANT = {"spec": 2, "control": 2}  # declarations: spec BuildRequires + Requires; control Build-Depends + one Depends
PY_PKG = "python3-plugin-hostd-protocol"


def ver(v):
    return tuple(int(x) for x in v.split("."))


def next_minor(lo):
    major, minor = ver(lo)[:2]
    return "%d.%d" % (major, minor + 1)


def stanza(control, name):
    for s in re.split(r"\n\s*\n", control):
        if re.search(r"^Package:\s*%s\s*$" % re.escape(name), s, re.M):
            return s
    return ""


def spec_package(spec, name):
    m = re.search(r"^%%package -n %s\n(.*?)(?=^%%|\Z)" % re.escape(name), spec, re.M | re.S)
    return m.group(1) if m else ""


def problems(spec, control, verbs_floor):
    out = []
    rs = re.findall(r"\(%s >= ([0-9.]+) with %s < ([0-9.]+)\)" % (PKG, PKG), spec)
    if len(rs) != WANT["spec"]:
        out.append("spec: %d ranges of %s, want %d" % (len(rs), PKG, WANT["spec"]))
    for lo, hi in rs:
        if hi != next_minor(lo):
            out.append("spec: %s >= %s < %s spans more than one minor (want < %s)" % (PKG, lo, hi, next_minor(lo)))
    lows = re.findall(r"%s \(>= ([0-9.]+)~\)" % PKG, control)
    highs = re.findall(r"%s \(<< ([0-9.]+)~\)" % PKG, control)
    if len(lows) != WANT["control"] or len(highs) != WANT["control"]:
        out.append("debian/control: %d floors and %d ceilings of %s, want %d of each" % (len(lows), len(highs), PKG, WANT["control"]))
    for lo, hi in zip(lows, highs):
        if hi != next_minor(lo):
            out.append("debian/control: %s (>= %s~) (<< %s~) spans more than one minor (want << %s~)" % (PKG, lo, hi, next_minor(lo)))
    # python3-plugin-hostd-protocol's omx-contract floor is verbs.json's
    sp = re.search(r"\(omx-contract >= ([0-9.]+) with omx-contract < ", spec_package(spec, PY_PKG))
    if not sp or ver(sp.group(1)) != ver(verbs_floor):
        out.append("spec: %s requires omx-contract >= %s, verbs.json asks >= %s" % (PY_PKG, sp.group(1) if sp else "nothing", verbs_floor))
    cp = re.search(r"omx-contract \(>= ([0-9.]+)~\)", stanza(control, PY_PKG))
    if not cp or ver(cp.group(1)) != ver(verbs_floor):
        out.append("debian/control: %s depends omx-contract (>= %s~), verbs.json asks >= %s" % (PY_PKG, cp.group(1) if cp else "nothing", verbs_floor))
    return out


spec = open(os.path.join(ROOT, "packaging/plugin-hostd-protocol.spec")).read()
control = open(os.path.join(ROOT, "debian/control")).read()
# the floor verbs.json asks for, wherever it sits in the document
m = re.search(r'"omx-contract-schema":\s*">=\s*([0-9.]+)"', open(os.path.join(ROOT, "data/verbs.json")).read())
verbs_floor = m.group(1) if m else "0"
# the check sees each fault: a two-minor range (spec, control), a floor below verbs.json's (spec, control), a range gone
wide_spec = re.sub(r"(\(%s >= [0-9]+\.[0-9]+)\.[0-9]+ with %s < [0-9.]+\)" % (PKG, PKG), lambda m_: "%s.0 with %s < 9.9)" % (m_.group(1), PKG), spec, count=1)
wide_control = control.replace("%s (<< " % PKG, "%s (<< 9." % PKG, 1)
low_spec = re.sub(r"\(omx-contract >= [0-9.]+ with omx-contract < ", "(omx-contract >= 0.0.1 with omx-contract < ", spec, count=1)
low_control = re.sub(r"omx-contract \(>= [0-9.]+~\)", "omx-contract (>= 0.0.1~)", control, count=1)
gone = re.sub(r"BuildRequires: \(%s[^\n]*\n" % PKG, "", spec)
seen = all(problems(*f, verbs_floor) for f in ((wide_spec, control), (spec, wide_control), (low_spec, control), (spec, low_control), (gone, control)))
print(("ok  " if seen else "FAIL") + " the range check sees a two-minor range, a floor below verbs.json's and a missing range")
bad = problems(spec, control, verbs_floor)
for b in bad:
    print("FAIL " + b)
if not bad:
    print("ok   every %s range spans one minor; %s's omx-contract floor is verbs.json's (>= %s)" % (PKG, PY_PKG, verbs_floor))
sys.exit(0 if seen and not bad else 1)
