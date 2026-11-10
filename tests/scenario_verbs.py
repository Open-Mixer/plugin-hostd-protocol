#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
# The scenario list (tests/host-scenarios.txt) exercises every verb a console's host backend declares: a controller
# that holds its backend to the list (openmixer's mix_host_backend test) finds each of them in it. preset_save was
# declared there and missing here, and that test failed at every rate. Each verb must also be a row of the host verb
# table (protocol.h's PHD_HOST_VERBS).
#   scenario_verbs.py [<scenario list>]
import os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CONSOLE_VERBS = ("add", "remove", "bypass", "param_set", "param_get", "patch_set", "preset_load", "preset_save",
                 "connect", "disconnect", "state_save", "state_load")


def exercised(path):
    verbs = set()
    for line in open(path, encoding="utf-8"):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        words = line.split("=>", 1)[0].split()
        while words and words[0].startswith("@"):  # a guard (@verb) or a tag (@t1)
            words = words[1:]
        if words:
            verbs.add(words[0])
    return verbs


def table():
    h = open(os.path.join(ROOT, "include", "plugin-hostd", "protocol.h"), encoding="utf-8").read()
    i = h.index("#define PHD_HOST_VERBS(X)")
    return set(re.findall(r'X\("([a-z_]+)"', h[i:h.index("\n\n", i)]))


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "tests", "host-scenarios.txt")
    have, rows = exercised(path), table()
    bad = 0
    for verb in CONSOLE_VERBS:
        if verb not in rows:
            print(f"FAIL {verb} is not a row of the host verb table")
            bad += 1
        elif verb not in have:
            print(f"FAIL {verb}, a verb a console's host backend declares, is not exercised by {os.path.basename(path)}")
            bad += 1
    if not bad:
        print(f"ok   the scenario list exercises the {len(CONSOLE_VERBS)} verbs a console's host backend declares")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
