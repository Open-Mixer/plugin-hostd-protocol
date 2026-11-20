#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tools/estimate-words-gen.py: PHD_ESTIMATE_VERDICT_*, PHD_ESTIMATE_REASON_*, PHD_ESTIMATE_VERDICTS,
PHD_ESTIMATE_REASONS, PHD_ESTIMATE_VERDICT_PATTERN and PHD_ESTIMATE_REASON_PATTERN of include/plugin-hostd/protocol.h
written from this package's domain `estimate` (contract.d/estimate/estimate.json): one string define per word (the word
is its `const`, the name its title in upper snake case), one X(id, word, meaning) row per word, in the order of its
codes, and the pattern that matches one word of the item; the meaning is its `description` with the first letter
lowered and no final stop. Each item's defines and rows sit between BEGIN and END GENERATED markers. The scalar item
ESTIMATE_CPU_CEILING is written as PHD_ESTIMATE_CPU_CEILING (its x-value) and PHD_ESTIMATE_CPU_CEILING_UNIT (its x-unit, a
word of omx-contract's PARAM_UNITS) between its own markers, and --check refuses, by name, a #define of the ceiling outside
them: the domain is its one holder.

  estimate-words-gen.py            write protocol.h
  estimate-words-gen.py --check    exit 1 when protocol.h is not what the domain says
"""
import json, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HEADER = os.path.join(ROOT, "include", "plugin-hostd", "protocol.h")
DOMAIN = os.path.join(ROOT, "contract.d", "estimate", "estimate.json")
LISTS = (("PHD_ESTIMATE_VERDICTS", "PHD_ESTIMATE_VERDICT_", "ESTIMATE_VERDICTS"),
         ("PHD_ESTIMATE_REASONS", "PHD_ESTIMATE_REASON_", "ESTIMATE_REASONS"))


def snake(title):
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", title).upper()


def meaning(w):
    text = w["description"].rstrip(".")
    first = text.split(" ")[0]
    return text if any(c.isupper() for c in first[1:]) else text[0].lower() + text[1:]


def block(doc, macro, prefix, item):
    words = sorted(doc["components"]["schemas"][item]["oneOf"], key=lambda w: w["x-code"])
    defs = ['#define %s%s "%s" ///< %s' % (prefix, snake(w["title"]), w["const"], meaning(w)) for w in words]
    pattern = '#define %sPATTERN "%s" ///< one word of %s' % (prefix, "|".join(w["const"] for w in words), macro)
    rows = ['    X(%s, %s%s, "%s")' % (snake(w["title"]), prefix, snake(w["title"]), meaning(w)) for w in words]
    return ("/* BEGIN GENERATED %s: tools/estimate-words-gen.py, from contract.d/estimate/estimate.json */\n%s\n%s\n"
            "#define %s(X) \\\n%s\n/* END GENERATED %s */" % (macro, "\n".join(defs), pattern, macro, " \\\n".join(rows), macro))


CEILING = "PHD_ESTIMATE_CPU_CEILING"


def ceiling_block(doc):
    c = doc["components"]["schemas"]["ESTIMATE_CPU_CEILING"]
    if c.get("x-kind") != "scalar" or not isinstance(c.get("x-value"), (int, float)) or isinstance(c.get("x-value"), bool):
        sys.exit("estimate-words-gen: ESTIMATE_CPU_CEILING is not a scalar item with a number x-value")
    return ("/* BEGIN GENERATED %s: tools/estimate-words-gen.py, from contract.d/estimate/estimate.json */\n"
            "#define %s %s ///< %s\n#define %s_UNIT \"%s\" ///< the unit of %s, a word of omx-contract's PARAM_UNITS\n"
            "/* END GENERATED %s */" % (CEILING, CEILING, c["x-value"], meaning(c), CEILING, c["x-unit"], CEILING, CEILING))


def hand_defines(text):
    """the names of every #define of the ceiling outside the generated blocks"""
    outside = re.sub(r"/\* BEGIN GENERATED (\w+):.*?/\* END GENERATED \1 \*/", "", text, flags=re.S)
    return re.findall(r"#define\s+(%s\w*)" % CEILING, outside)


def render(text, doc):
    marked = re.compile(r"/\* BEGIN GENERATED %s:.*?/\* END GENERATED %s \*/" % (CEILING, CEILING), re.S)
    text, n = marked.subn(lambda m: ceiling_block(doc), text)
    if n != 1:
        sys.exit("estimate-words-gen: the generated block of %s is not once in %s" % (CEILING, HEADER))
    for macro, prefix, item in LISTS:
        marked = re.compile(r"/\* BEGIN GENERATED %s:.*?/\* END GENERATED %s \*/" % (macro, macro), re.S)
        text, n = marked.subn(lambda m: block(doc, macro, prefix, item), text)
        if n != 1:
            sys.exit("estimate-words-gen: the generated block of %s is not once in %s" % (macro, HEADER))
    return text


def main():
    doc = json.load(open(DOMAIN))
    text = open(HEADER).read()
    out = render(text, doc)
    if "--check" in sys.argv:
        hand = hand_defines(text)
        if hand:
            print("estimate-words-gen: include/plugin-hostd/protocol.h defines %s by hand, outside the generated blocks: "
                  "the ceiling's one holder is ESTIMATE_CPU_CEILING of contract.d/estimate/estimate.json" % ", ".join(hand))
            return 1
        if out != text:
            print("estimate-words-gen: include/plugin-hostd/protocol.h is not the render of contract.d/estimate/estimate.json: "
                  "run make gen")
            return 1
        return 0
    open(HEADER, "w").write(out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
