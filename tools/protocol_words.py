# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tools/protocol_words.py: the words of the protocol, read from this tree's domain contract.d/plugin-hostd-protocol, the one
declaration of the verbs plugin-hostd adds, the protocol's own verbs, the reply codes only plugin-hostd answers and the
extensions hello grants. Each item comes back in the form omx-contract's data reader gives it (kind, since, doc, then
codes, verbs or words keyed as the data form keys them), so a tool reads `items()["PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS"]["verbs"]` as it
read data/verbs.json; `requires()` is the packages and domains the document may refer to (info.x-requires, a list of names). Nothing is read from a copy."""
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DOMAIN = "contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json"


def document(path=None):
    """The domain document: this tree's, or the one at `path` (a scratch copy a test has changed)."""
    path = path or os.path.join(ROOT, DOMAIN)
    try:
        with open(path, encoding="utf-8") as f:
            return json.load(f)
    except (OSError, ValueError) as e:
        sys.exit("protocol_words: %s: %s" % (path, e))


def _drop(d):
    return {k: v for k, v in d.items() if v is not None}


def _item(it):
    head = [("kind", it.get("x-kind", "codes")), ("since", it.get("x-since")), ("doc", it.get("description")),
            ("why", it.get("x-why"))]
    out = dict((k, v) for k, v in head if v is not None)
    words = it["oneOf"]
    if out["kind"] == "codes":
        out["codes"] = {w["x-name"]: _drop({"code": w["const"], "doc": w.get("description"),
                                            "text": {"en": w["x-text"]} if "x-text" in w else None, "args": w.get("x-args"),
                                            "resp": w.get("x-resp"), "transient": w.get("x-transient")}) for w in words}
    elif out["kind"] == "verbs":
        out["verbs"] = {w["title"]: _drop({"verb": w["const"], "doc": w.get("description"), "format": w.get("x-format"),
                                           "reply": w.get("x-reply"), "codes": w.get("x-codes"),
                                           "implementedBy": w.get("x-implemented-by"), "since": w.get("x-since")})
                        for w in words}
    elif out["kind"] == "words":
        out["words"] = {w["title"]: _drop({"word": w["const"], "doc": w.get("description"),
                                           "text": {"en": w["x-text"]} if "x-text" in w else None, "args": w.get("x-args")})
                        for w in words}
    else:
        sys.exit("protocol_words: %s: no reading of the kind %s" % (DOMAIN, out["kind"]))
    return out


def items(path=None):
    return {name: _item(it) for name, it in document(path)["components"]["schemas"].items()}


def requires(path=None):
    return document(path)["info"].get("x-requires", [])


def _word(kind, key, v, i):
    if kind == "codes":
        w = {"const": v["code"], "title": key, "description": v.get("doc"), "x-code": i,
             "x-text": v.get("text", {}).get("en"), "x-args": v.get("args"), "x-name": key, "x-resp": v.get("resp"),
             "x-transient": v.get("transient")}
    elif kind == "verbs":
        w = {"const": v["verb"], "title": key, "description": v.get("doc"), "x-code": i, "x-format": v.get("format"),
             "x-reply": v.get("reply"), "x-codes": v.get("codes"), "x-implemented-by": v.get("implementedBy"),
             "x-since": v.get("since")}
    else:
        w = {"const": v["word"], "title": key, "description": v.get("doc"), "x-code": i,
             "x-text": v.get("text", {}).get("en"), "x-args": v.get("args")}
    return _drop(w)


def domain_from(data, doc):
    """The document `doc` with its items written from `data`, items in the form items() returns: the way back, for a
    test that changes the words in that form and stages the changed document."""
    out = json.loads(json.dumps(doc))
    schemas = {}
    for name, it in data.items():
        kind = it["kind"]
        head = _drop({"description": it.get("doc"), "x-since": it.get("since"), "x-why": it.get("why"), "x-kind": kind,
                      "x-file": "verbs.json"})
        head["oneOf"] = [_word(kind, k, v, i) for i, (k, v) in enumerate(it[kind].items())]
        schemas[name] = head
    out["components"]["schemas"] = schemas
    return out
