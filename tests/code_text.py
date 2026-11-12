# SPDX-License-Identifier: GPL-3.0-or-later
# Every code this repository owns carries the sentence a person reads for it (omx-contract schema 3.12.0, spec 7.11):
# `text` with the English source sentence alone, and `args`, the ordered placeholder names it uses as {name}; and its
# Catalan in catalogs/ca.json, keyed by the code. The owner writes both here, and omx-contract's validator refuses what
# this test refuses (a placeholder outside args, a catalog key that is no code of ours); this test says it before the
# validator runs, names the code, and also holds what the validator cannot know:
#   - the code is every reply.plugin-hostd.* word the protocol's table names (nothing declared in protocol.h goes without);
#   - every code has its Catalan, which uses the same placeholders, and the catalog holds nothing else;
#   - the sentence is one sentence, plain, with no code name inside it, and ca is no copy of en;
#   - the declaration says it needs the schema that has catalogs (omx-contract-schema >= 3.12.0), and so does the
#     installed omx-contract, when a schema is given.
#
#   code_text.py <data/verbs.json> <protocol/plugin-hostd-protocol.json> <catalogs/ca.json> [<installed omx-contract.schema.json>]
import json
import re
import sys

FLOOR = (3, 12, 0)
PLACEHOLDER = re.compile(r"\{([^{}]*)\}")
LOWER_CAMEL = re.compile(r"^[a-z][A-Za-z0-9]*$")
failures = []


def fail(message):
    failures.append(message)
    print("FAIL " + message)


def version(text):
    return tuple(int(n) for n in re.findall(r"\d+", text)[:3])


declaration = json.load(open(sys.argv[1]))
table = json.load(open(sys.argv[2]))
catalog = json.load(open(sys.argv[3]))
if (catalog.get("kind"), catalog.get("lang"), catalog.get("owner")) != ("catalog", "ca", "plugin-hostd-protocol"):
    fail("catalogs/ca.json is not the Catalan catalog of plugin-hostd-protocol: %r" % ([catalog.get(k) for k in ("kind", "lang", "owner")],))
messages = catalog.get("messages", {})

floor = declaration.get("requires", {}).get("omx-contract-schema", "")
if version(floor) < FLOOR:
    fail("data/verbs.json requires omx-contract-schema %r: `text` came with %s" % (floor, ".".join(map(str, FLOOR))))
if len(sys.argv) > 4:
    have = json.load(open(sys.argv[4])).get("x-omx-schema-version", "")
    if version(have) < FLOOR:
        fail("the installed omx-contract is schema %r: catalogs need %s" % (have, ".".join(map(str, FLOOR))))

owned = {}
for group, items in declaration.items():
    if isinstance(items, dict) and items.get("kind") == "codes":
        for name, code in items["codes"].items():
            owned[code["code"]] = ("%s.%s" % (group, name), code)

# every reply word of the protocol that this repository names is declared here, with its sentence
named = [c["code"] for c in table["reply_codes"] if c["code"].startswith("reply.plugin-hostd.")]
for word in named:
    if word not in owned:
        fail("%s is in the protocol's table and in no codes item of data/verbs.json" % word)
if not named:
    fail("the protocol's table names no reply.plugin-hostd code: the check read nothing")

for word, (where, code) in sorted(owned.items(), key=lambda kv: kv[1][1].get("resp", 0), reverse=True):
    text = code.get("text")
    if not isinstance(text, dict):
        fail("%s (%s): no text" % (where, word))
        continue
    if sorted(text) != ["en"]:
        fail("%s: text has %s, want the source sentence en alone (a translation is catalogs/<lang>.json)" % (where, sorted(text)))
        continue
    if word not in messages:
        fail("%s (%s): no Catalan in catalogs/ca.json" % (where, word))
        continue
    # an entry is the sentence, or {text, by[, model, source]} with its provenance (omx-contract 3.16.0)
    en, ca = text["en"], messages[word] if isinstance(messages[word], str) else messages[word].get("text", "")
    if not en.strip() or not ca.strip():
        fail("%s: a sentence is empty" % where)
        continue
    args = code.get("args")
    if not isinstance(args, list):
        fail("%s: no args (an empty list when the sentence uses no placeholder)" % where)
        continue
    if len(set(args)) != len(args) or not all(isinstance(a, str) and LOWER_CAMEL.match(a) for a in args):
        fail("%s: args %r are not lowerCamel names listed once" % (where, args))
    en_names, ca_names = set(PLACEHOLDER.findall(en)), set(PLACEHOLDER.findall(ca))
    if en_names != ca_names:
        fail("%s: en uses %s and ca %s" % (where, sorted(en_names), sorted(ca_names)))
    for name in sorted(en_names | ca_names):
        if name not in args:
            fail("%s: {%s} is not in args" % (where, name))
    for name in args:
        if name not in en_names:
            fail("%s: arg %s is in no sentence" % (where, name))
    if ca == en:
        fail("%s: ca is a copy of en" % where)
    for lang, sentence in (("en", en), ("ca", ca)):
        if sentence[-1] not in ".!?":
            fail("%s: the %s sentence does not end with a full stop" % (where, lang))
        if len(re.findall(r"[.!?](?:\s|$)", sentence)) != 1:
            fail("%s: the %s text is not one sentence: %r" % (where, lang, sentence))
        # a person reads no code name, no wire number, no verb with an underscore
        if re.search(r"reply\.|resp |-5\d\d|\b[a-z]+_[a-z_]+\b", sentence):
            fail("%s: the %s sentence names a code, a number or a verb: %r" % (where, lang, sentence))
        if not 20 <= len(sentence) <= 220:
            fail("%s: the %s sentence is %d characters" % (where, lang, len(sentence)))

for key in sorted(set(messages) - set(owned)):
    fail("catalogs/ca.json: %s is no code of ours" % key)

if failures:
    print("code text: %d failure(s)" % len(failures))
    sys.exit(1)
print("ok   %d codes, each with its source sentence and its Catalan in catalogs/ca.json, the same placeholders, all in args" % len(owned))
