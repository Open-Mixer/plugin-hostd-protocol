#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/pin_words_test.py: the words of a pin line (the wire words of pin_set and of the pin_changed event) are this
package's domain `pin` (contract.d/pin/pin.json): the verdict and pool words (sorting pass V12), and the keys of a pin's
optional words and the words of what changed (step 1 of the protocol owning the pin words plugin-db rendered for it).
PHD_PIN_VERDICT_*, PHD_PIN_POOL_*, PHD_PIN_KEY_*, PHD_PIN_CHANGED_* and the lists PHD_PIN_VERDICTS, PHD_PIN_POOLS,
PHD_PIN_KEYS and PHD_PIN_CHANGES of protocol.h are generated from it, the meaning texts included; PHD_PIN_VERDICT_KEY and
PHD_PIN_POOL_KEY are its keys with the equals sign.

(a) golden: the four lists and the eleven words of protocol.h, expanded by tests/pin_words_dump.c, are the rows frozen from
    the words plugin-db's pin file wrote before this domain took them (tests/golden/pin-words.txt, 20 rows; the meanings
    of the two keys name the domain's own items);
(b) the domain holds exactly those rows: word, code (its place), meaning (its description, first letter lowered, no final
    stop), and include/plugin-hostd/pin_codes.h is the render of its x-c-codes items;
(c) sabotage, in a scratch copy of the tree: a word, or a meaning, changed in each item of the domain makes
    tools/pin-words-gen.py --check fail, and writing it changes what the call expands to;
(d) protocol.h reads no plugin-db header: with the vendored plugin_db_vocabulary.h gone from a scratch copy, protocol.h
    still compiles and the expansion is as it was.

Environment: OMX_CONTRACT (the command, default omx-contract), CC. Exit 0 all green, 1 a FAIL line."""
import json, os, re, shlex, shutil, subprocess, sys, tempfile

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "tools"))
from domain_path import render_env  # noqa: E402  the renders read contract.d, then the installed contract directory

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OMX = shlex.split(os.environ.get("OMX_CONTRACT", "omx-contract"))
CC = shlex.split(os.environ.get("CC", "cc"))
LISTS = (("verdicts", "PIN_VERDICTS"), ("pools", "PIN_POOLS"), ("keys", "PIN_KEYS"), ("changes", "PIN_CHANGES"))
failed = 0


def check(cond, what):
    global failed
    print(("ok   " if cond else "FAIL ") + what)
    if not cond:
        failed += 1


def dump(tree, tmp):
    exe = os.path.join(tmp, "dump")
    b = subprocess.run(CC + ["-Wall", "-Wextra", "-Werror", "-I" + os.path.join(tree, "include"), "-o", exe,
                             os.path.join(ROOT, "tests", "pin_words_dump.c")], capture_output=True, text=True)
    if b.returncode != 0:
        return None, b.stderr
    return subprocess.run([exe], capture_output=True, text=True).stdout, ""


def snake(title):
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", title).upper()


def meaning(w):
    text = w["description"].rstrip(".")
    first = text.split(" ")[0]
    return text if any(c.isupper() for c in first[1:]) else text[0].lower() + text[1:]


WORDS = {"PHD_PIN_VERDICT_IN_PROCESS": ("PIN_VERDICTS", "inProcess"), "PHD_PIN_VERDICT_ISOLATED": ("PIN_VERDICTS", "isolated"),
         "PHD_PIN_VERDICT_REFUSED": ("PIN_VERDICTS", "refused"), "PHD_PIN_POOL_CORES": ("PIN_POOLS", "cores"),
         "PHD_PIN_POOL_OFF": ("PIN_POOLS", "off"), "PHD_PIN_KEY_VERDICT": ("PIN_KEYS", "verdict"),
         "PHD_PIN_KEY_POOL": ("PIN_KEYS", "pool"), "PHD_PIN_CHANGED_FILES": ("PIN_CHANGES", "files"),
         "PHD_PIN_CHANGED_LAYOUT": ("PIN_CHANGES", "layout")}
# the keys of a pin line's optional words, each its key with the equals sign
KEYED = {"PHD_PIN_VERDICT_KEY": "verdict", "PHD_PIN_POOL_KEY": "pool"}


def words(doc, item):
    """The words of the domain's item in the order of their codes, none when the domain lacks the item."""
    return sorted(doc["components"]["schemas"].get(item, {}).get("oneOf", []), key=lambda w: w["x-code"])


def const(doc, item, title):
    found = [w["const"] for w in words(doc, item) if w["title"] == title]
    return found[0] if found else "(absent)"


def main():
    gold = open(os.path.join(ROOT, "tests", "golden", "pin-words.txt")).read()
    with tempfile.TemporaryDirectory() as tmp:
        got, err = dump(ROOT, tmp)
        check(got == gold, "(a) the lists and words of protocol.h are the golden 20 rows %s" % err.strip()[-200:])
        doc_path = os.path.join(ROOT, "contract.d", "pin", "pin.json")
        if not os.path.isfile(doc_path):
            check(False, "(b) contract.d/pin/pin.json does not exist")
        else:
            doc = json.load(open(doc_path))
            rows = []
            for name, item in LISTS:
                for i, w in enumerate(words(doc, item)):
                    rows.append("%s %d %s %s %s" % (name, i, snake(w["title"]), w["const"], meaning(w)))
            for macro, (item, title) in WORDS.items():
                rows.append("word %s %s" % (macro, const(doc, item, title)))
            for macro, title in KEYED.items():
                rows.append("word %s %s=" % (macro, const(doc, "PIN_KEYS", title)))
            check("\n".join(rows) + "\n" == gold, "(b) the domain holds exactly the golden rows, meanings included")
            r = subprocess.run(OMX + ["render", "--target", "c", "--package", "pin", "--installed",
                                      "--out", "include/plugin-hostd", "--check"], cwd=ROOT, env=render_env(ROOT),
                                     capture_output=True, text=True)
            check(r.returncode == 0, "(b) include/plugin-hostd/pin_codes.h is the render of the domain: %s" % (r.stderr.strip() or r.stdout.strip())[-200:])
        gen = os.path.join(ROOT, "tools", "pin-words-gen.py")
        r = subprocess.run([sys.executable, gen, "--check"], cwd=ROOT, capture_output=True, text=True) if os.path.isfile(gen) else None
        check(r is not None and r.returncode == 0, "(b) protocol.h's pin words are the generator's output (--check)")
        # (c) sabotage in a scratch copy
        if os.path.isfile(gen) and os.path.isfile(doc_path):
            for _, item in LISTS:
                for what, edit in (("a word", lambda w: w.update(const=w["const"] + "-x")),
                                   ("a meaning", lambda w: w.update(description="Changed meaning."))):
                    scratch = os.path.join(tmp, "tree-%s-%s" % (item, what.split()[1]))
                    shutil.copytree(ROOT, scratch, ignore=shutil.ignore_patterns(".git", "build", "node_modules"))
                    p = os.path.join(scratch, "contract.d", "pin", "pin.json")
                    d = json.load(open(p))
                    if len(words(d, item)) < 2:
                        check(False, "(c) the domain has no item %s to sabotage" % item)
                        continue
                    sabotaged = d["components"]["schemas"][item]["oneOf"][1]
                    edit(sabotaged)
                    json.dump(d, open(p, "w"), indent=2)
                    c = subprocess.run([sys.executable, os.path.join(scratch, "tools", "pin-words-gen.py"), "--check"], cwd=scratch, capture_output=True, text=True)
                    check(c.returncode != 0, "(c) %s changed in %s of the domain makes --check fail" % (what, item))
                    subprocess.run([sys.executable, os.path.join(scratch, "tools", "pin-words-gen.py")], cwd=scratch, capture_output=True, text=True)
                    out, _ = dump(scratch, tmp)
                    mark = "changed meaning" if what == "a meaning" else " %s " % sabotaged["const"]
                    check(out is not None and out != gold and mark in out,
                          "(c) writing it changes the macro the call expands to (%s of %s)" % (what, item))
        else:
            check(False, "(c) no generator or no domain to sabotage")
        # (d) protocol.h reads no plugin-db header
        scratch = os.path.join(tmp, "tree-vendored")
        shutil.copytree(ROOT, scratch, ignore=shutil.ignore_patterns(".git", "build", "node_modules"))
        os.unlink(os.path.join(scratch, "include", "plugin-hostd", "plugin_db_vocabulary.h"))
        out, err = dump(scratch, tmp)
        check(out == gold, "(d) with plugin_db_vocabulary.h gone, protocol.h compiles and the expansion is as it was %s" % err.strip()[-200:])
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
