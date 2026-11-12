#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/feedback_args.py <table> <lines>: every feedback line of plugin-hostd's own in the table (origin plugin-hostd)
declares its arguments by name and type, and every line of <lines> (tests/feedback_lines, printed through the formats
the daemon writes with) decodes against its declaration: each word matches its type's pattern in the table's
arg_types, or is one of the literal words a type that is not named there spells, a|b. A line of a shape no form
declares, or a plugin-hostd line with no line of <lines>, is a failure. The check is seen to fail: each planted fault
(a worker without its w, a status of an unknown kind, an instance list that is no list, a word too many) must not
decode. plugin-hostd-protocol#17."""
import json
import re
import sys


def forms_of(entry):
    return entry.get("forms") or ([{"args": entry["args"], "names": entry.get("names"), "tail": entry["tail"]}]
                                  if entry.get("args") is not None else [])


def word_matches(word, kind, types):
    if kind in types:
        return re.fullmatch(types[kind]["pattern"], word) is not None
    return word in kind.split("|")


def decode(line, table):
    """the named fields of a line, or None when no form of its declaration takes it"""
    verb, _, rest = line.partition(" ")
    entry = next((e for e in table["feedback"] if e["line"] == verb), None)
    if entry is None:
        return None
    types = table.get("arg_types", {})
    for form in forms_of(entry):
        args, names, tail = form["args"], form.get("names") or [None] * len(form["args"]), form["tail"]
        words = rest.split(" ") if rest else []
        if tail and args and types.get(args[-1], {}).get("rest"):
            head = words[:len(args) - 1]
            if len(words) < len(args):
                continue
            words = head + [" ".join(words[len(args) - 1:])]
        if len(words) != len(args):
            continue
        if all(word_matches(w, k, types) for w, k in zip(words, args)):
            return dict(zip(names, words))
    return None


def main():
    table = json.load(open(sys.argv[1]))
    lines = [l for l in open(sys.argv[2]).read().split("\n") if l]
    failures = []
    ours = [e for e in table["feedback"] if e["origin"] == "plugin-hostd"]
    for e in ours:
        forms = forms_of(e)
        if not forms:
            failures.append("%s: no arguments declared" % e["line"])
        for f in forms:
            if not f.get("names") or len(f["names"]) != len(f["args"]):
                failures.append("%s: a form whose arguments are not each named" % e["line"])
            for k in f["args"]:
                if k not in table.get("arg_types", {}) and "|" not in k and not re.fullmatch(r"[a-z][a-z0-9-]*", k):
                    failures.append("%s: %s is no declared type and no literal word" % (e["line"], k))
        if not any(l.split(" ")[0] == e["line"] for l in lines):
            failures.append("%s: no line of tests/feedback_lines shows it" % e["line"])
    for l in lines:
        if decode(l, table) is None:
            failures.append("does not decode against its declared arguments: %s" % l)
    planted = {
        "a worker without its w": ("worker_died w3 ", "worker_died 3 "),
        "a status of an unknown kind": ("signal:11", "kill:11"),
        "an instance list that is no list": (" 3,4", " 3;4"),
        "a word too many": ("worker_backoff w3 250", "worker_backoff w3 250 9"),
    }
    for name, (a, b) in planted.items():
        hit = [l for l in lines if a in l]
        if not hit:
            failures.append("planted fault %s: no line holds %r" % (name, a))
        elif decode(hit[0].replace(a, b, 1), table) is not None:
            failures.append("planted fault %s decodes: %s" % (name, hit[0].replace(a, b, 1)))
    for f in failures:
        print("FAIL %s" % f)
    if failures:
        return 1
    print("ok   %d plugin-hostd feedback lines declare named, typed arguments; %d lines decode against them; "
          "%d planted faults refused" % (len(ours), len(lines), len(planted)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
