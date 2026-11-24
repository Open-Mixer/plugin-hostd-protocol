#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""The grammar's curve words are omx-dsp's: curve-origin is OMX_CURVE_ORIGIN_WORDS minus `user` (a mark is a band whose
origin is not user), curve-leg is OMX_CURVE_LEG_WORDS, both read from the installed omx_curve.h (the -I directories
given, as the Makefile passes OMXDSP_CFLAGS, then the compiler's), never a second copy typed here.
tests/curve-words-sabotage.sh drops and adds an alternative in a scratch grammar and this must go red.

  curve_words_test.py -I<dir> ...   0 when the two rules equal the header's words
  CURVE_WORDS_GRAMMAR               another grammar/core.abnf (the sabotage's scratch copy)
"""
import os
import re
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
GRAMMAR = os.environ.get("CURVE_WORDS_GRAMMAR") or os.path.join(ROOT, "grammar", "core.abnf")
# the -I directories given, then the C compiler's own, where omx-dsp-devel installs omxdsp/ when pkg-config names no -I
dirs = [a[2:] for a in sys.argv[1:] if a.startswith("-I")] + ["/usr/local/include", "/usr/include"]
header = next((os.path.join(d, "omxdsp", "omx_curve.h") for d in dirs if os.path.isfile(os.path.join(d, "omxdsp", "omx_curve.h"))), None)
if header is None:
    sys.exit("FAIL no omxdsp/omx_curve.h under the -I directories given: %s" % dirs)
text = open(header).read()


def words(table):
    m = re.search(r"%s\[[^\]]*\]\s*=\s*\{(.*?)\};" % table, text, re.S)
    if not m:
        sys.exit("FAIL %s is not in %s" % (table, header))
    return re.findall(r'"([^"]*)"', m.group(1))


def rule(name):
    m = re.search(r"^%s\s*=\s*(.*)$" % re.escape(name), open(GRAMMAR).read(), re.M)
    if not m:
        sys.exit("FAIL rule %s is not in %s" % (name, GRAMMAR))
    return re.findall(r'%s"([^"]*)"', m.group(1))


failed = 0
for name, want in (("curve-origin", [w for w in words("OMX_CURVE_ORIGIN_WORDS") if w != "user"]),
                   ("curve-leg", words("OMX_CURVE_LEG_WORDS"))):
    got = rule(name)
    if sorted(got) == sorted(want):
        print("ok   %s is the header's %s" % (name, want))
    else:
        failed += 1
        print("FAIL %s is %s, the header's words are %s" % (name, got, want))
sys.exit(1 if failed else 0)
