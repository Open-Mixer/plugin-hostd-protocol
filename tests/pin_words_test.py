#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/pin_words_test.py: the words of a pin's verdict and pool key (the wire words of pin_set) are this package's domain
`pin` (contract.d/pin/pin.json), and PHD_PIN_VERDICT_*, PHD_PIN_POOL_*, PHD_PIN_VERDICTS and PHD_PIN_POOLS of protocol.h are
generated from it, the meaning texts included (sorting pass V12).

(a) golden: the two lists and the five words of protocol.h, expanded by tests/pin_words_dump.c, are the rows frozen from the
    words plugin-db's pin file wrote before its domain took them (tests/golden/pin-words.txt, 10 rows);
(b) the domain holds exactly those rows: word, code (its place), meaning (its description, first letter lowered, no final
    stop), and include/plugin-hostd/pin_codes.h is the render of its x-c-codes items;
(c) sabotage, in a scratch copy of the tree: a word, or a meaning, changed in the domain makes tools/pin-words-gen.py --check
    fail, and writing it changes what the call expands to;
(d) the vendored plugin_db_vocabulary.h no longer feeds them: its verdict and pool words changed in a scratch copy leave
    the expansion as it was.

Environment: OMX_CONTRACT (the command, default omx-contract), CC. Exit 0 all green, 1 a FAIL line."""
import json, os, re, shlex, shutil, subprocess, sys, tempfile

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "tools"))
from domain_path import render_env  # noqa: E402  the renders read contract.d, then the installed contract directory

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OMX = shlex.split(os.environ.get("OMX_CONTRACT", "omx-contract"))
CC = shlex.split(os.environ.get("CC", "cc"))
LISTS = (("verdicts", "PIN_VERDICTS"), ("pools", "PIN_POOLS"))
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
         "PHD_PIN_POOL_OFF": ("PIN_POOLS", "off")}


def main():
    gold = open(os.path.join(ROOT, "tests", "golden", "pin-words.txt")).read()
    with tempfile.TemporaryDirectory() as tmp:
        got, err = dump(ROOT, tmp)
        check(got == gold, "(a) the lists and words of protocol.h are the golden 10 rows %s" % err.strip()[-200:])
        doc_path = os.path.join(ROOT, "contract.d", "pin", "pin.json")
        if not os.path.isfile(doc_path):
            check(False, "(b) contract.d/pin/pin.json does not exist")
        else:
            doc = json.load(open(doc_path))
            rows = []
            for name, item in LISTS:
                ws = sorted(doc["components"]["schemas"][item]["oneOf"], key=lambda w: w["x-code"])
                for i, w in enumerate(ws):
                    rows.append("%s %d %s %s %s" % (name, i, snake(w["title"]), w["const"], meaning(w)))
            for macro, (item, title) in WORDS.items():
                w = [w for w in doc["components"]["schemas"][item]["oneOf"] if w["title"] == title][0]
                rows.append("word %s %s" % (macro, w["const"]))
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
            for what, edit in (("a word", lambda w: w.update(const=w["const"] + "-x")),
                               ("a meaning", lambda w: w.update(description="Changed meaning."))):
                scratch = os.path.join(tmp, "tree-" + what.split()[1])
                shutil.copytree(ROOT, scratch, ignore=shutil.ignore_patterns(".git", "build", "node_modules"))
                p = os.path.join(scratch, "contract.d", "pin", "pin.json")
                d = json.load(open(p))
                edit(d["components"]["schemas"]["PIN_VERDICTS"]["oneOf"][1])
                json.dump(d, open(p, "w"), indent=2)
                c = subprocess.run([sys.executable, os.path.join(scratch, "tools", "pin-words-gen.py"), "--check"], cwd=scratch, capture_output=True, text=True)
                check(c.returncode != 0, "(c) %s changed in the domain makes --check fail" % what)
                subprocess.run([sys.executable, os.path.join(scratch, "tools", "pin-words-gen.py")], cwd=scratch, capture_output=True, text=True)
                out, _ = dump(scratch, tmp)
                check(out is not None and out != gold and ("isolated-x" in out or "changed meaning" in out),
                      "(c) writing it changes the macro the call expands to (%s)" % what)
        else:
            check(False, "(c) no generator or no domain to sabotage")
        # (d) the vendored plugin-db header does not feed the words
        scratch = os.path.join(tmp, "tree-vendored")
        shutil.copytree(ROOT, scratch, ignore=shutil.ignore_patterns(".git", "build", "node_modules"))
        vp = os.path.join(scratch, "include", "plugin-hostd", "plugin_db_vocabulary.h")
        text = open(vp).read()
        for old in ('"in-process"', '"isolated"', '"refused"', '"cores"', '"off"'):
            text = text.replace(old, old[:-1] + 'X"')
        open(vp, "w").write(text)
        out, err = dump(scratch, tmp)
        check(out == gold, "(d) plugin_db_vocabulary.h with every verdict and pool word changed leaves the expansion as it was %s" % err.strip()[-200:])
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
