#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/reply_shapes.py <table> <shapes dir> <golden transcript>: the reply schemas (shapes/reply_<verb>.schema.json, one
per verb whose reply the table types as reply_forms, docs/shapes.md of Open-Mixer/omx-contract) hold the parsed form of
every reply they describe.

  * a schema for each such verb and no other; each is a JSON Schema 2020-12 document;
  * its properties are the names of the verb's longest reply form, in order, its required keys those of the shortest,
    and a record's keys the names of the table's record type, so a table that changes a reply goes red here;
  * every success reply of the golden transcript whose verb has a schema, and one sample written from each reply form,
    decoded by tests/reply_args.py and typed (a whole number or a decimal one a number, a list an array, a record an
    object, a quoted word unquoted), validates against its schema;
  * planted faults do not validate: a word of another type, a missing key, a key more, a state the table does not
    declare, a record a key short."""
import copy
import json
import os
import re
import sys

try:
    import jsonschema
except ImportError:
    sys.exit("reply_shapes needs python3-jsonschema (dnf install python3-jsonschema, apt install python3-jsonschema)")

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from reply_args import decode_form, sample_reply  # noqa: E402

# the words a decoded value is converted by, the rest stay the words the daemon wrote
UNQUOTE = {"quoted": lambda w: w[1:-1], "quoted-escaped": lambda w: re.sub(r'\\(["\\])', r"\1", w[1:-1])}


def typed(value, kind, types):
    """a decoded word, list or record as the document the schema describes"""
    t = types.get(kind)
    if t and "items" in t:
        return [typed(v, t["items"]["type"], types) for v in value]
    if t and "fields" in t:
        return {n: typed(value[n], k, types) for n, k in zip(t["fields"]["names"], t["fields"]["args"])}
    if kind in ("int", "uint"):
        return int(value)
    if kind == "number":
        return float(value)
    return UNQUOTE.get(kind, lambda w: w)(value)


def document(verb, line, table):
    """the typed document of a reply line by the first of the verb's reply forms that takes it, or None"""
    cmd = next(c for c in table["commands"] if c["verb"] == verb)
    for form in cmd.get("reply_forms") or []:
        got = decode_form(form, line, table["arg_types"])
        if got is not None:
            repeated = form["names"][-1] if form["repeat"] else None
            return {n: [typed(x, k, table["arg_types"]) for x in got[n]] if n == repeated else typed(got[n], k, table["arg_types"])
                    for n, k in zip(form["names"], form["args"])}
    return None


def record_names(schema, kind, types, where, failures):
    """a record's keys in the schema are the table's names, in the table's order, all required, and nothing else"""
    t = types[kind]
    if schema.get("type") != "object" or list(schema.get("properties", {})) != t["fields"]["names"] \
            or schema.get("required") != t["fields"]["names"] or schema.get("additionalProperties") is not False:
        failures.append("%s: not the record %s of the table, %s" % (where, kind, t["fields"]["names"]))
        return
    for n, k in zip(t["fields"]["names"], t["fields"]["args"]):
        walk(schema["properties"][n], k, types, "%s.%s" % (where, n), failures)


def walk(schema, kind, types, where, failures):
    t = types.get(kind)
    if t and "items" in t:
        if schema.get("type") != "array":
            failures.append("%s: the list %s is not an array" % (where, kind))
        else:
            walk(schema["items"], t["items"]["type"], types, where + "[]", failures)
    elif t and "fields" in t:
        record_names(schema, kind, types, where, failures)


def main():
    table = json.load(open(sys.argv[1]))
    shapes_dir, golden = sys.argv[2], sys.argv[3]
    types = table["arg_types"]
    failures = []
    verbs = {c["verb"]: [f for f in c["reply_forms"] if f["args"]] for c in table["commands"] if c.get("reply_forms")}
    verbs = {v: f for v, f in verbs.items() if f}
    have = {f[len("reply_"):-len(".schema.json")] for f in os.listdir(shapes_dir) if f.startswith("reply_") and f.endswith(".schema.json")}
    for v in sorted(set(verbs) - have):
        failures.append("%s: the table types its reply and shapes/reply_%s.schema.json is absent" % (v, v))
    for v in sorted(have - set(verbs)):
        failures.append("%s: shapes/reply_%s.schema.json describes a reply the table does not type" % (v, v))
    schemas = {}
    for v in sorted(have & set(verbs)):
        forms = verbs[v]
        s = schemas[v] = json.load(open(os.path.join(shapes_dir, "reply_%s.schema.json" % v)))
        try:
            jsonschema.Draft202012Validator.check_schema(s)
        except jsonschema.SchemaError as e:
            failures.append("%s: not a JSON Schema 2020-12: %s" % (v, e.message))
            continue
        longest = max(forms, key=lambda f: len(f["args"]))
        shortest = min(forms, key=lambda f: len(f["args"]))
        if list(s["properties"]) != longest["names"]:
            failures.append("%s: the properties %s are not the names %s of the longest reply form" % (v, list(s["properties"]), longest["names"]))
        if s["required"] != shortest["names"]:
            failures.append("%s: the required keys %s are not the names %s of the shortest reply form" % (v, s["required"], shortest["names"]))
        if s.get("additionalProperties") is not False:
            failures.append("%s: not closed (additionalProperties)" % v)
        for n, k in zip(longest["names"], longest["args"]):
            p = s["properties"].get(n)
            if p is None:
                continue  # named above
            walk(p.get("items", {}) if longest["repeat"] and n == longest["names"][-1] else p, k, types, "%s.%s" % (v, n), failures)
    # the worker states of the table are the words the state property takes
    states = {s["name"] for s in table["worker_states"]}
    for v in ("instance_info", "worker_list"):
        s = schemas.get(v)
        if s:
            p = s["properties"]["state"] if v == "instance_info" else s["properties"]["workers"]["items"]["properties"]["state"]
            if set(re.sub(r"^\^\(\?:|\)\$$", "", p["pattern"]).split("|")) != states:
                failures.append("%s: the state words of %s are not the table's worker_states %s" % (v, p["pattern"], sorted(states)))
    validators = {v: jsonschema.Draft202012Validator(s) for v, s in schemas.items()}

    def validate(v, line, what):
        doc = document(v, line, table)
        if doc is None:
            failures.append("%s: %s does not decode: %s" % (v, what, line[:80]))
            return None
        errs = sorted(validators[v].iter_errors(doc), key=lambda e: list(e.path))
        if errs:
            failures.append("%s: %s does not validate (%s at %s): %s" % (v, what, errs[0].message[:90], "/".join(map(str, errs[0].path)), line[:80]))
            return None
        return doc

    # every sample reply of every form, then the replies of the golden transcript, the real ones of this library
    samples = 0
    for v, forms in sorted(verbs.items()):
        if v not in validators:
            continue
        for f in forms:
            samples += validate(v, sample_reply(f, types), "the sample of a form") is not None
    real = {}
    cmd = None
    for line in open(golden, encoding="utf-8"):
        line = line.rstrip("\n")
        if line.startswith("> "):
            cmd = line[2:].split(" ")[0]
        elif line.startswith("< resp 0") and cmd in validators:
            real[cmd] = real.get(cmd, 0) + (validate(cmd, line[2:], "a reply of the golden transcript") is not None)
    # replies as the host writes them, beyond the samples: preset_get's name is the rest of the line, raw, so it may hold
    # spaces and may be empty (a preset with no name answers "resp 0 <key> " with nothing after the key)
    for v, line, want in (("preset_get", "resp 0 urn:p:1 Bright Mix", "Bright Mix"), ("preset_get", "resp 0 urn:p:1 ", ""),
                          ("preset_get", "resp 0 urn:p:1", "")):
        doc = validate(v, line, "a reply as the host writes it") if v in validators else None
        if doc is not None and doc.get("name") != want:
            failures.append("%s: the name of %r is %r, not %r" % (v, line, doc.get("name"), want))
    for name in ("Bright Mix", ""):
        if "preset_get" in validators and not validators["preset_get"].is_valid({"load_key": "urn:p:1", "name": name}):
            failures.append("preset_get: the schema refuses the name %r" % name)
    served = {"hello", "curve_points", "param_get_many"}
    for v in sorted(served & set(verbs)):
        if real.get(v, 0) < 1:
            failures.append("%s: the golden transcript holds no reply of it to validate" % v)
    # planted faults: each must be refused by the schema it breaks
    good = {v: document(v, sample_reply(max(f, key=lambda x: len(x["args"])), types), table) for v, f in verbs.items() if v in validators}
    planted = 0

    def plant(v, what, mutate):
        nonlocal planted
        if v not in good:
            failures.append("planted fault %s: %s has no schema to plant it in" % (what, v))
            return
        doc = copy.deepcopy(good[v])
        mutate(doc)
        if validators[v].is_valid(doc):
            failures.append("planted fault %s validates: %s %s" % (what, v, json.dumps(doc)[:80]))
        planted += 1

    plant("instance_info", "a pid that is a word", lambda d: d.update(pid="3"))
    plant("instance_info", "a state the table does not declare", lambda d: d.update(state="sleeping"))
    plant("instance_info", "a key more", lambda d: d.update(extra=1))
    plant("instance_info", "a key less", lambda d: d.pop(list(d)[-1]))
    plant("worker_list", "a record a key short", lambda d: d["workers"][0].pop("place"))
    plant("worker_list", "a worker that is not w<k>", lambda d: d["workers"][0].update(worker="x1"))
    plant("curve_points", "a point that is a word", lambda d: d["points"].append({"x": "1", "y": 2.0}))
    plant("hello", "a version that is a word", lambda d: d.update(version="1"))
    plant("estimate", "a ram key that is not a number or -", lambda d: d.update(ram_bytes="lots"))
    plant("estimate", "a verdict that is no word of the domain estimate", lambda d: d.update(verdict="fits"))
    plant("pin_layout", "a sha256 that is short", lambda d: d["layout"].update(sha256="ab"))
    plant("plan", "a segment instance that is a word", lambda d: d.update(segments=[["x"]]))
    for f in failures:
        print("FAIL %s" % f)
    if failures:
        return 1
    print("ok   %d reply schemas, one per typed reply of the table, each holding its names and record keys; %d samples and %d "
          "golden replies (%s) validate; %d planted faults refused"
          % (len(schemas), samples, sum(real.values()), ", ".join("%s %d" % kv for kv in sorted(real.items())), planted))
    return 0


if __name__ == "__main__":
    sys.exit(main())
