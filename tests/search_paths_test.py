#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/search_paths_test.py: the default plugin search paths name the folders the distributions install plugins in.

A fresh plugin-hostd (no settings, no CLAP_PATH) learns a plugin's pin on its first add only inside the declared search
path, which is this default. CLAP's was clap/entry.h's alone (~/.clap, /usr/lib/clap), so a plugin a distribution
installs (/usr/lib64/clap on Fedora, /usr/local/lib/clap on the Raspberry Pi image) was outside it and its first add
refused with -508 (plugin-hostd#82). The CLAP default holds entry.h's folders first and then, as the LV2 default does,
/usr/local/lib, /usr/lib64 and /usr/local/lib64; the LV2 default's own such folders are the positive control.
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HEADER = os.path.join(ROOT, "include", "plugin-hostd", "search_paths.h")
failures = []


def check(ok, what):
    print(("ok   " if ok else "FAIL ") + what)
    if not ok:
        failures.append(what)


def default(text, name):
    """the colon-separated folders of one #define, its adjacent string literals joined"""
    m = re.search(r"#define %s((?:\s*\\?\s*\"[^\"]*\")+)" % name, text)
    if not m:
        return None
    return "".join(re.findall(r"\"([^\"]*)\"", m.group(1))).split(":")


text = open(HEADER).read()
lv2 = default(text, "PHD_DEFAULT_LV2_PATH")
clap = default(text, "PHD_DEFAULT_CLAP_PATH")
check(lv2 is not None and clap is not None, "both defaults are read from search_paths.h")
lv2, clap = lv2 or [], clap or []

distro = ["/usr/local/lib/%s", "/usr/lib64/%s", "/usr/local/lib64/%s"]
check(all(d % "lv2" in lv2 for d in distro), "positive control: the LV2 default names /usr/local/lib, /usr/lib64 and "
      "/usr/local/lib64: %s" % lv2)
check(clap[:2] == ["~/.clap", "/usr/lib/clap"], "the CLAP default starts with clap/entry.h's folders, in their order: %s" % clap)
missing = [d % "clap" for d in distro if d % "clap" not in clap]
check(not missing, "the CLAP default names the folders distributions install CLAP plugins in%s"
      % ("" if not missing else ": missing " + ", ".join(missing)))
check(len(clap) == len(set(clap)), "no folder twice in the CLAP default")

print("search paths: %s" % ("ok" if not failures else "%d failed" % len(failures)))
sys.exit(1 if failures else 0)
