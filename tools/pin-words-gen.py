#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tools/pin-words-gen.py: PHD_PIN_VERDICT_*, PHD_PIN_POOL_*, PHD_PIN_VERDICTS and PHD_PIN_POOLS of
include/plugin-hostd/protocol.h written from this package's domain `pin` (contract.d/pin/pin.json): one string define per
word (the word is its `const`, the name its title in upper snake case) and one X(id, word, meaning) row per word, in the
order of its codes; the meaning is its `description` with the first letter lowered and no final stop. Each item's defines
and rows sit between BEGIN and END GENERATED markers.

  pin-words-gen.py            write protocol.h
  pin-words-gen.py --check    exit 1 when protocol.h is not what the domain says
"""
import json, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HEADER = os.path.join(ROOT, "include", "plugin-hostd", "protocol.h")
DOMAIN = os.path.join(ROOT, "contract.d", "pin", "pin.json")
LISTS = (("PHD_PIN_VERDICTS", "PHD_PIN_VERDICT_", "PIN_VERDICTS"), ("PHD_PIN_POOLS", "PHD_PIN_POOL_", "PIN_POOLS"))


def snake(title):
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", title).upper()


def meaning(w):
    text = w["description"].rstrip(".")
    first = text.split(" ")[0]
    return text if any(c.isupper() for c in first[1:]) else text[0].lower() + text[1:]


def block(doc, macro, prefix, item):
    words = sorted(doc["components"]["schemas"][item]["oneOf"], key=lambda w: w["x-code"])
    defs = ['#define %s%s "%s" ///< %s' % (prefix, snake(w["title"]), w["const"], meaning(w)) for w in words]
    rows = ['    X(%s, %s%s, "%s")' % (snake(w["title"]), prefix, snake(w["title"]), meaning(w)) for w in words]
    return ("/* BEGIN GENERATED %s: tools/pin-words-gen.py, from contract.d/pin/pin.json */\n%s\n/** @brief the rows of the list: X(id, word, meaning)\n *\n * @param X the macro each row is passed to.\n */\n#define %s(X) \\\n%s\n"
            "/* END GENERATED %s */" % (macro, "\n".join(defs), macro, " \\\n".join(rows), macro))


def render(text, doc):
    for macro, prefix, item in LISTS:
        marked = re.compile(r"/\* BEGIN GENERATED %s:.*?/\* END GENERATED %s \*/" % (macro, macro), re.S)
        text, n = marked.subn(lambda m: block(doc, macro, prefix, item), text)
        if n != 1:
            sys.exit("pin-words-gen: the generated block of %s is not once in %s" % (macro, HEADER))
    return text


def main():
    doc = json.load(open(DOMAIN))
    text = open(HEADER).read()
    out = render(text, doc)
    if "--check" in sys.argv:
        if out != text:
            print("pin-words-gen: include/plugin-hostd/protocol.h is not the render of contract.d/pin/pin.json: run make gen")
            return 1
        return 0
    open(HEADER, "w").write(out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
