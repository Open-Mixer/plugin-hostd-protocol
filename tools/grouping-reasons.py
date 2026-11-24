#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tools/grouping-reasons.py: GroupingReason and RETRY_REASONS of both clients, written from plugin-hostd's domain
`plugin-hostd/grouping` (its items PLUGIN_HOSTD_GROUPING_REASONS and PLUGIN_HOSTD_GROUPING_REASON_GROUPS, read from the
contract directory tools/domain_path.py names). GroupingReason in clients/python/plugin_hostd_protocol/__init__.py and
in clients/typescript/codec/src/index.ts sits between BEGIN and END GENERATED markers, never edited by hand; so does
RETRY_REASONS: the reasons whose group the domain declares retry (every reason names its group, retry or final). Both
clients are written from the one domain, so they cannot differ (sorting pass, V17 PR 2).

  grouping-reasons.py            write both regions
  grouping-reasons.py --check    exit 1 when a region is missing or is not what the domain gives
"""
import os, re, sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from domain_path import read_domain  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PY = "clients/python/plugin_hostd_protocol/__init__.py"
TS = "clients/typescript/codec/src/index.ts"
NAME = "grouping-reasons"
DOMAIN = "plugin-hostd/grouping"
REASONS = "PLUGIN_HOSTD_GROUPING_REASONS"
GROUPS = "PLUGIN_HOSTD_GROUPING_REASON_GROUPS"


def fail(msg):
    sys.exit("grouping-reasons: " + msg)


def reasons(doc):
    try:
        items = doc["components"]["schemas"]
        groups = {w["const"] for w in items[GROUPS]["oneOf"]}
        words = items[REASONS]["oneOf"]
        if items[REASONS].get("x-groups") != GROUPS:
            fail("%s of the domain %s does not name %s as its groups" % (REASONS, DOMAIN, GROUPS))
    except (KeyError, TypeError) as e:
        fail("the domain %s lacks %s" % (DOMAIN, e))
    out = []
    for w in sorted(words, key=lambda w: w["x-code"]):
        key, word = w["title"], w["const"]
        if not re.fullmatch(r"[a-z]+(-[a-z]+)*", word) or not re.fullmatch(r"[a-z][A-Za-z]*", key):
            fail("%s: %r is no reason word" % (key, word))
        # the retry property: every reason names its group, one of the two the domain declares
        if groups != {"retry", "final"} or w.get("x-group") not in groups:
            fail("%s: no retry property (group is one of %s, got %r)" % (key, sorted(groups), w.get("x-group")))
        out.append((key, word, w["x-group"] == "retry"))
    if not out:
        fail("the domain %s has no reason" % DOMAIN)
    return out


def banner(comment):
    return "%s BEGIN GENERATED %s (tools/grouping-reasons.py, from plugin-hostd's domain %s): do not edit" % (comment, NAME, DOMAIN)


def python_region(rs):
    o = [banner("#"), "class GroupingReason:",
         '    """The reasons a plan gives (PLUGIN_HOSTD_GROUPING_REASONS): a controller compares against these and writes no',
         "    reason word of its own. plan_get answers them as instance:reason; a plan-refused is followed by a colon and the",
         '    number of the refusal."""  # noqa: D205, D209', ""]
    o += ['    %s = "%s"' % (w.upper().replace("-", "_"), w) for _, w, _r in rs]
    names = ", ".join("GroupingReason.%s" % w.upper().replace("-", "_") for _, w, r in rs if r)
    o += ["", "",
          "# the reasons of a plan that a later plan may not give again with the chain as it is: the reasons that declare",
          "# the group retry in the domain, read from it and chosen nowhere in code",
          "RETRY_REASONS = (%s)" % names, "# END GENERATED %s" % NAME]
    return "\n".join(o) + "\n"


def ts_region(rs):
    o = [banner("//"),
         "/** The reasons a plan gives (PLUGIN_HOSTD_GROUPING_REASONS), by the key the domain gives each: a controller compares",
         " * against these and writes no reason word of its own. A plan-refused is followed by a colon and the refusal's number. */",
         "export const GroupingReason = {"]
    o += ["  %s: '%s'," % (k, w) for k, w, _r in rs]
    names = ", ".join("GroupingReason.%s" % k for k, _, r in rs if r)
    o += ["} as const;", "/** One of the reasons a plan gives. */",
          "export type GroupingReasonWord = (typeof GroupingReason)[keyof typeof GroupingReason];",
          "/**",
          " * The reasons of a plan that a later plan may not give again with the chain as it is: the reasons the domain declares",
          " * in the group retry, read from it and chosen nowhere in code.",
          " */",
          "export const RETRY_REASONS: readonly string[] = [%s];" % names,
          "// END GENERATED %s" % NAME]
    return "\n".join(o) + "\n"


def splice(text, region, comment, path):
    pat = re.compile(r"^%s BEGIN GENERATED %s.*?^%s END GENERATED %s\n" % (re.escape(comment), NAME, re.escape(comment), NAME), re.S | re.M)
    if len(pat.findall(text)) != 1:
        fail("%s: not exactly one %s region" % (path, NAME))
    return pat.sub(lambda _: region, text)


def main():
    rs = reasons(read_domain(DOMAIN, "grouping-reasons"))
    bad = []
    for path, region, comment in ((PY, python_region(rs), "#"), (TS, ts_region(rs), "//")):
        full = os.path.join(ROOT, path)
        text = open(full, encoding="utf-8").read()
        new = splice(text, region, comment, path)
        if new != text:
            bad.append(path)
            if "--check" not in sys.argv:
                open(full, "w", encoding="utf-8").write(new)
    if "--check" in sys.argv and bad:
        print("grouping-reasons: %s is not what the domain %s gives: run make gen" % (", ".join(bad), DOMAIN))
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
