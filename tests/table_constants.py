#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/table_constants.py <table> <protocol json>: the installed table carries the wire's constants as protocol.h
declares them, so a client reads them there and keeps no copy: the protocol version hello sends and answers, the
longest line and the most words of one, the ports and bounds of PHD_CONSTANTS, the words of a pin (its separators,
the bytes a path cannot hold as they are and the escape that writes them), the formats a worker holds (the
<format> of worker_list, which a console keys its pools on) and the LV2 workers lv2_worker_set names. Each is held to
protocol/plugin-hostd.json, which protocol-gen writes compiled against protocol.h. A planted fault (a constant
dropped, a separator changed, a format renamed) must be seen. plugin-hostd-protocol#18."""
import json
import sys


def problems(table, phd):
    out = []
    consts = {c["name"]: c for c in table.get("constants") or []}
    for c in phd["constants"]:
        if consts.get(c["name"], {}).get("value") != c["value"]:
            out.append("constants: %s is not %r" % (c["name"], c["value"]))
    if consts.get("protocol_version", {}).get("value") != phd["version"]:
        out.append("constants: protocol_version is not the version hello answers, %r" % phd["version"])
    if table.get("pin") != phd["pin"]:
        out.append("pin: %r is not protocol.h's %r" % (table.get("pin"), phd["pin"]))
    if table.get("formats") != phd["formats"] or not phd["formats"]:
        out.append("formats: %r is not protocol.h's %r" % (table.get("formats"), phd["formats"]))
    if table.get("lv2_workers") != phd["lv2_workers"] or not phd["lv2_workers"]:
        out.append("lv2_workers: %r is not protocol.h's %r" % (table.get("lv2_workers"), phd["lv2_workers"]))
    return out


def main():
    table = json.load(open(sys.argv[1]))
    phd = json.load(open(sys.argv[2]))
    bad = problems(table, phd)
    dropped = dict(table, constants=[c for c in table.get("constants") or [] if c["name"] != "line_max"])
    moved = dict(table, pin=dict(table.get("pin") or {}, digest_separator="#"))
    renamed = dict(table, formats=[f.upper() for f in table.get("formats") or []])
    if not problems(dropped, phd) or not problems(moved, phd) or not problems(renamed, phd):
        bad.append("a planted fault (line_max dropped, the digest separator changed, a format renamed) was not seen")
    for b in bad:
        print("FAIL %s" % b)
    if bad:
        return 1
    print("ok   the table carries %d constants, protocol_version among them, the pin's words, %d formats and %d LV2 "
          "workers, as protocol.h declares them (3 planted faults seen)" % (len(table["constants"]), len(table["formats"]),
                                                                         len(table["lv2_workers"])))
    return 0


if __name__ == "__main__":
    sys.exit(main())
