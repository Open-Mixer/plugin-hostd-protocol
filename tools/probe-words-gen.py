#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tools/probe-words-gen.py: PHD_PROBE_REASONS, PHD_PROBE_OUTCOMES and PHD_PROBE_STEPS of include/plugin-hostd/protocol.h
written from this package's domain `probe` (contract.d/probe/probe.json): one X(id, word, meaning) row per word, in the
order of its codes. The id is the word's title in upper snake case (an outcome's without its leading PROBE_), the word its
`const`, the meaning its `description` with the first letter lowered (unless the first word has another capital: NaN) and no final stop.
The rows sit between BEGIN and END GENERATED markers; the doc comment above each stays the header's own.

  probe-words-gen.py            write protocol.h
  probe-words-gen.py --check    exit 1 when protocol.h is not what the domain says
"""
import json, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HEADER = os.path.join(ROOT, "include", "plugin-hostd", "protocol.h")
DOMAIN = os.path.join(ROOT, "contract.d", "probe", "probe.json")
LISTS = (("PHD_PROBE_REASONS", "PROBE_REASONS", False), ("PHD_PROBE_OUTCOMES", "PROBE_OUTCOMES", True),
         ("PHD_PROBE_STEPS", "PROBE_STEPS", False))


def snake(title):
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", title).upper()


def meaning(w):
    text = w["description"].rstrip(".")
    first = text.split(" ")[0]
    return text if any(c.isupper() for c in first[1:]) else text[0].lower() + text[1:]


def block(doc, macro, item, strip):
    words = sorted(doc["components"]["schemas"][item]["oneOf"], key=lambda w: w["x-code"])
    rows = []
    for w in words:
        ident = snake(w["title"])
        if strip:
            ident = re.sub(r"^PROBE_", "", ident)
        rows.append('    X(%s, "%s", "%s")' % (ident, w["const"], meaning(w)))
    return ("/* BEGIN GENERATED %s: tools/probe-words-gen.py, from contract.d/probe/probe.json */\n#define %s(X) \\\n%s\n"
            "/* END GENERATED %s */" % (macro, macro, " \\\n".join(rows), macro))


def render(text, doc):
    for macro, item, strip in LISTS:
        new = block(doc, macro, item, strip)
        marked = re.compile(r"/\* BEGIN GENERATED %s:.*?\*/\n#define %s\(X\)(?:[^\n]*\\\n)*[^\n]*\n/\* END GENERATED %s \*/" % (macro, macro, macro))
        plain = re.compile(r"#define %s\(X\)(?:[^\n]*\\\n)*[^\n]*" % macro)
        text, n = marked.subn(lambda m: new, text)
        if n == 0:
            text, n = plain.subn(lambda m: new, text)
        if n != 1:
            sys.exit("probe-words-gen: %s is not once in %s" % (macro, HEADER))
    return text


def main():
    doc = json.load(open(DOMAIN))
    text = open(HEADER).read()
    out = render(text, doc)
    if "--check" in sys.argv:
        if out != text:
            print("probe-words-gen: include/plugin-hostd/protocol.h is not the render of contract.d/probe/probe.json: run make gen")
            return 1
        return 0
    open(HEADER, "w").write(out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
