#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/reply_args.py <table>: every reply of this repository's verbs (origin plugin-hostd or plugin-hostd-protocol)
whose layout has a placeholder declares it typed and named (plugin-hostd-protocol#19), as the table's reply_forms: resp
0 or a count, then each word's name and type, a last one repeated over the rest of the line (a count then counts those
repeated words). A verb may have forms of different lengths (estimate's six words of older daemons and its ten). A type is one of
arg_types, which may be a list (items of a type joined by a separator, or a none word) or a record (named fields joined
by a separator), or literal words a|b. One sample reply per form, written from its types, decodes into its names, and
the prose layout names as many words as the form does. Planted faults must not decode: a record a field short, a list
with another separator, a count that is not the number of words. The worker-state words a type spells are the
table's worker_states."""
import json
import re
import sys


def decode_word(word, kind, types):
    """the value of one word of a type: a str, a list of values or a dict of fields; None when it is not of the type"""
    t = types.get(kind)
    if t is None:
        return word if word in kind.split("|") else None
    if "items" in t:
        if t["items"].get("none") and word in t["items"]["none"].split("|"):
            return []
        parts = [decode_word(w, t["items"]["type"], types) for w in word.split(t["items"]["separator"])]
        return None if any(p is None for p in parts) else parts
    if "fields" in t:
        # a field may hold the record's own separator (a place holds ':'): the record's pattern, each field's pattern
        # captured in order, says where each one ends; all the table's patterns group without capturing
        f = t["fields"]
        whole = re.escape(f["separator"]).join("(%s)" % pattern_of(k, types) for k in f["args"])
        m = re.fullmatch(whole, word)
        if not m:
            return None
        vals = [decode_word(p, k, types) for p, k in zip(m.groups(), f["args"])]
        return None if any(v is None for v in vals) else dict(zip(f["names"], vals))
    return word if re.fullmatch(t["pattern"], word) else None


def pattern_of(kind, types):
    return types[kind]["pattern"] if kind in types else "|".join(re.escape(w) for w in kind.split("|"))


def decode_reply(verb, line, table):
    """{name: value} of a reply line by the first of the verb's reply_forms that takes it, or None"""
    cmd = next(c for c in table["commands"] if c["verb"] == verb)
    for form in cmd.get("reply_forms") or []:
        got = decode_form(form, line, table["arg_types"])
        if got is not None:
            return got
    return None


def decode_form(form, line, types):
    words = line.split(" ")
    if words[0] != "resp" or len(words) < 2 or not re.fullmatch(r"-?[0-9]+", words[1]):
        return None
    words = words[2:]
    args, names = form["args"], form["names"]
    if args and types.get(args[-1], {}).get("rest"):
        if len(words) < len(args) - 1:
            return None
        words = words[:len(args) - 1] + [" ".join(words[len(args) - 1:])]
    if form["repeat"]:
        head = len(args) - 1
        if len(words) < head:
            return None
        fixed, rest = words[:head], words[head:]
    else:
        if len(words) != len(args):
            return None
        fixed, rest = words, []
    out = {}
    for w, k, n in zip(fixed, args, names):
        v = decode_word(w, k, types)
        if v is None:
            return None
        out[n] = v
    if form["repeat"]:
        vals = [decode_word(w, args[-1], types) for w in rest]
        if any(v is None for v in vals):
            return None
        out[names[-1]] = vals
    if form["resp"] == "count" and form["repeat"] and int(line.split(" ")[1]) != len(rest):
        return None
    return out


def sample_word(kind, types, n=0):
    t = types.get(kind)
    if t is None:
        return kind.split("|")[n % len(kind.split("|"))]
    if "items" in t:
        return t["items"]["separator"].join(sample_word(t["items"]["type"], types, k) for k in range(2))
    if "fields" in t:
        return t["fields"]["separator"].join(sample_word(k, types) for k in t["fields"]["args"])
    return t["example"]


def sample_reply(form, types):
    words = [sample_word(k, types) for k in form["args"]]
    if form["repeat"]:
        words += [sample_word(form["args"][-1], types, 1)]
    count = (len(words) - (len(form["args"]) - 1) if form["repeat"] else 3) if form["resp"] == "count" else 0
    return " ".join(["resp", str(count)] + words)


def main():
    table = json.load(open(sys.argv[1]))
    types = table.get("arg_types", {})
    failures = []
    ours = [c for c in table["commands"] if c["origin"] in ("plugin-hostd", "plugin-hostd-protocol")]
    typed = count_only = 0
    for c in ours:
        prose = c.get("reply") or ""
        placeholders = len(re.findall(r"<[^>]+>|\.\.\.", prose))
        forms = c.get("reply_forms") or []
        if placeholders and not forms:
            failures.append("%s: its reply %r has no typed form" % (c["verb"], prose))
            continue
        if not forms:
            continue
        typed += 1
        count_only += all(not f["args"] for f in forms)
        longest = max(forms, key=lambda f: len(f["args"]))
        # the prose's words after resp and its number, a standalone ... being the repetition and no word of its own
        prose_words = [w for w in prose.split(" ")[2:] if w != "..."]
        if len(prose_words) != len(longest["args"]):
            failures.append("%s: the prose %r names %d words, the longest form %d" % (c["verb"], prose, len(prose_words),
                                                                                   len(longest["args"])))
        for form in forms:
            if len(form["names"]) != len(form["args"]):
                failures.append("%s: a reply argument without its name" % c["verb"])
            line = sample_reply(form, types)
            if decode_reply(c["verb"], line, table) is None:
                failures.append("%s: the sample %r does not decode against %r" % (c["verb"], line, form))
    states = {s["name"] for s in table.get("worker_states", [])}
    # the state type's pattern is written from PHD_WORKER_STATES: (?:<name>|...|(?!)), the last alternative never matching
    state_words = set(re.sub(r"^\(\?:|\)$", "", types.get("state", {}).get("pattern", "")).split("|")) - {"(?!)"}
    if types.get("state") and state_words != states:
        failures.append("the state type's words are not the table's worker_states %s" % sorted(states))
    wl = next((c for c in ours if c["verb"] == "worker_list"), None)
    if wl and wl.get("reply_forms"):
        good = sample_reply(wl["reply_forms"][0], types)
        record = good.split(" ")[2]
        planted = {
            "a record a field short": good.replace(record, record.rsplit(":", 1)[0], 1),
            "a list with another separator": good.replace(",", ";", 1) if "," in good else good + " x",
            "a count that is not the number of words": "resp 9 " + " ".join(good.split(" ")[2:]),
        }
        for name, line in planted.items():
            if decode_reply("worker_list", line, table) is not None:
                failures.append("planted fault %s decodes: %s" % (name, line))
    else:
        failures.append("worker_list: no typed reply to plant faults in")
    # the bare lists: a list decoder of its own is seen, apart from any record's pattern. A reply of nested lists
    # decodes into lists of lists, and an item that is not of the list's type, or another separator, is refused
    # the cores are the segments' shape, each member the core of its worker or - for none, as the daemon writes them
    # (plan_cores: 7,8;9 gives -,-;-)
    nested = decode_reply("plan_get", "resp 0 7,8;9 7:budget,9:plan-refused -,-;-", table)
    if not nested or nested.get("segments") != [["7", "8"], ["9"]] or nested.get("cores") != [["-", "-"], ["-"]]:
        failures.append("plan_get: 7,8;9 and -,-;- are not read as lists of lists: %r" % (nested,))
    pinned = decode_reply("plan", "resp 0 7,8;9 2,-;3", table)
    if not pinned or pinned.get("cores") != [["2", "-"], ["3"]]:
        failures.append("plan: the cores 2,-;3, a member with a core and one with none, are not read: %r" % (pinned,))
    for verb, line in (("plan_get", "resp 0 7,x;9 - -"), ("plan_get", "resp 0 7,8|9 - -"),
                       ("estimate", "resp 0 128 12.5 - 2 1 0:-701,bad"), ("estimate", "resp 0 128 12.5 - 2 1 0;-701")):
        if decode_reply(verb, line, table) is not None:
            failures.append("planted fault in a bare list decodes: %s" % line)
    for f in failures:
        print("FAIL %s" % f)
    if failures:
        return 1
    print("ok   %d replies of this repository's verbs are typed and named (%d with words, %d a count alone); each sample "
          "decodes; nested lists read as lists; 7 planted faults refused" % (typed, typed - count_only, count_only))
    return 0


if __name__ == "__main__":
    sys.exit(main())
