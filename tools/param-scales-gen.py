#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tools/param-scales-gen.py: PHD_PARAM_SCALES of include/plugin-hostd/protocol.h written from omx-contract's domain `param`
(its item PARAM_SCALES, read from the contract directory tools/domain_path.py names), one X(id, name, meaning) row per
word in the order of its codes: the id the word's title in upper snake case, the name its `const`, the meaning its
`description` with the first letter lowered and no final stop. The rows sit between BEGIN and END GENERATED markers; the
doc comment above stays the header's own. protocol.h spells no scale word of its own (sorting pass, V21).

  param-scales-gen.py            write protocol.h
  param-scales-gen.py --check    exit 1 when protocol.h is not what the domain says
"""
import os, re, sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from domain_path import read_domain  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HEADER = os.path.join(ROOT, "include", "plugin-hostd", "protocol.h")
MACRO = "PHD_PARAM_SCALES"


def snake(title):
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", title).upper()


def meaning(w):
    text = w["description"].rstrip(".")
    first = text.split(" ")[0]
    return text if any(c.isupper() for c in first[1:]) else text[0].lower() + text[1:]


def block(doc):
    words = sorted(doc["components"]["schemas"]["PARAM_SCALES"]["oneOf"], key=lambda w: w["x-code"])
    rows = ['    X(%s, "%s", "%s")' % (snake(w["title"]), w["const"], meaning(w)) for w in words]
    return ("/* BEGIN GENERATED %s: tools/param-scales-gen.py, from omx-contract's domain param, item PARAM_SCALES */\n"
            "#define %s(X) \\\n%s\n/* END GENERATED %s */" % (MACRO, MACRO, " \\\n".join(rows), MACRO))


def render(text, doc):
    marked = re.compile(r"/\* BEGIN GENERATED %s:.*?\*/\n#define %s\(X\)(?:[^\n]*\\\n)*[^\n]*\n/\* END GENERATED %s \*/" % (MACRO, MACRO, MACRO))
    plain = re.compile(r"#define %s\(X\)(?:[^\n]*\\\n)*[^\n]*" % MACRO)
    new = block(doc)
    out, n = marked.subn(lambda m: new, text)
    if n == 0:
        out, n = plain.subn(lambda m: new, text)
    if n != 1:
        sys.exit("param-scales-gen: %s is not once in %s" % (MACRO, HEADER))
    return out


def main():
    doc = read_domain("param", "param-scales-gen")
    text = open(HEADER).read()
    out = render(text, doc)
    if "--check" in sys.argv:
        if out != text:
            print("param-scales-gen: include/plugin-hostd/protocol.h is not the render of omx-contract's domain param: run make gen")
            return 1
        return 0
    open(HEADER, "w").write(out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
