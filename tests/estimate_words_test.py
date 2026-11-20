#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/estimate_words_test.py: the insert verdict of estimate is the host's (coordinator ruling, 2026-11-20). Its
verdict and reason words are this package's domain `estimate` (contract.d/estimate/estimate.json), and
PHD_ESTIMATE_VERDICT_*, PHD_ESTIMATE_REASON_*, PHD_ESTIMATE_VERDICTS, PHD_ESTIMATE_REASONS and their two patterns of
protocol.h are generated from it, the meaning texts included; the reply of estimate ends with the two words.

(a) golden: the two lists, every word, the patterns, the two arg types, the reply as data/verbs.json declares it, its
    row of PHD_REPLY_ARGS and the CPU ceiling, expanded by tests/estimate_words_dump.c, are tests/golden/estimate-words.txt;
(b) the domain holds exactly those rows: word, code (its place), meaning (its description, first letter lowered, no final
    stop), include/plugin-hostd/estimate_codes.h is the render of its x-c-codes items, and protocol.h is the generator's
    output;
(c) sabotage, in a scratch copy of the tree: a word, or a meaning, changed in the domain makes
    tools/estimate-words-gen.py --check fail, and writing it changes what the call expands to.

Environment: OMX_CONTRACT (the command, default omx-contract), CC. Exit 0 all green, 1 a FAIL line."""
import json, os, re, shlex, shutil, subprocess, sys, tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OMX = shlex.split(os.environ.get("OMX_CONTRACT", "omx-contract"))
CC = shlex.split(os.environ.get("CC", "cc"))
LISTS = (("verdicts", "ESTIMATE_VERDICTS", "PHD_ESTIMATE_VERDICT_"), ("reasons", "ESTIMATE_REASONS", "PHD_ESTIMATE_REASON_"))
failed = 0


def check(cond, what):
    global failed
    print(("ok   " if cond else "FAIL ") + what)
    if not cond:
        failed += 1


def dump(tree, tmp):
    exe = os.path.join(tmp, "dump")
    b = subprocess.run(CC + ["-Wall", "-Wextra", "-Werror", "-I" + os.path.join(tree, "include"), "-o", exe,
                             os.path.join(ROOT, "tests", "estimate_words_dump.c")], capture_output=True, text=True)
    if b.returncode != 0:
        return None, b.stderr
    return subprocess.run([exe], capture_output=True, text=True).stdout, ""


def snake(title):
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", title).upper()


def meaning(w):
    text = w["description"].rstrip(".")
    first = text.split(" ")[0]
    return text if any(c.isupper() for c in first[1:]) else text[0].lower() + text[1:]


def domain_rows(doc):
    rows, words = [], []
    for name, item, prefix in LISTS:
        ws = sorted(doc["components"]["schemas"][item]["oneOf"], key=lambda w: w["x-code"])
        for i, w in enumerate(ws):
            rows.append("%s %d %s %s %s" % (name, i, snake(w["title"]), w["const"], meaning(w)))
            words.append("word %s%s %s" % (prefix, snake(w["title"]), w["const"]))
    return rows + words


def main():
    gold = open(os.path.join(ROOT, "tests", "golden", "estimate-words.txt")).read()
    with tempfile.TemporaryDirectory() as tmp:
        got, err = dump(ROOT, tmp)
        check(got == gold, "(a) the lists, words, patterns, types, reply and ceiling of protocol.h are the golden rows %s"
              % err.strip()[-300:])
        doc_path = os.path.join(ROOT, "contract.d", "estimate", "estimate.json")
        if not os.path.isfile(doc_path):
            check(False, "(b) contract.d/estimate/estimate.json does not exist")
        else:
            doc = json.load(open(doc_path))
            want = [l for l in gold.splitlines() if l.split(" ")[0] in ("verdicts", "reasons", "word")]
            check(domain_rows(doc) == want, "(b) the domain holds exactly the golden rows, meanings included")
            r = subprocess.run(OMX + ["render", "--target", "c", "--package", "estimate", "--contract-dir", "contract.d",
                                      "--out", "include/plugin-hostd", "--check"], cwd=ROOT, capture_output=True, text=True)
            check(r.returncode == 0, "(b) include/plugin-hostd/estimate_codes.h is the render of the domain: %s"
                  % (r.stderr.strip() or r.stdout.strip())[-200:])
        gen = os.path.join(ROOT, "tools", "estimate-words-gen.py")
        r = subprocess.run([sys.executable, gen, "--check"], cwd=ROOT, capture_output=True, text=True) if os.path.isfile(gen) else None
        check(r is not None and r.returncode == 0, "(b) protocol.h's estimate words are the generator's output (--check)")
        if os.path.isfile(gen) and os.path.isfile(doc_path):
            for what, edit, mark in (("a word", lambda w: w.update(const=w["const"] + "-x"), "warn-x"),
                                     ("a meaning", lambda w: w.update(description="Changed meaning."), "changed meaning")):
                scratch = os.path.join(tmp, "tree-" + what.split()[1])
                shutil.copytree(ROOT, scratch, ignore=shutil.ignore_patterns(".git", "build", "node_modules"))
                p = os.path.join(scratch, "contract.d", "estimate", "estimate.json")
                d = json.load(open(p))
                edit(d["components"]["schemas"]["ESTIMATE_VERDICTS"]["oneOf"][1])
                json.dump(d, open(p, "w"), indent=2)
                c = subprocess.run([sys.executable, os.path.join(scratch, "tools", "estimate-words-gen.py"), "--check"],
                                   cwd=scratch, capture_output=True, text=True)
                check(c.returncode != 0, "(c) %s changed in the domain makes --check fail" % what)
                subprocess.run([sys.executable, os.path.join(scratch, "tools", "estimate-words-gen.py")], cwd=scratch,
                               capture_output=True, text=True)
                out, _ = dump(scratch, tmp)
                check(out is not None and out != gold and mark in out,
                      "(c) writing it changes the macro the call expands to (%s)" % what)
        else:
            check(False, "(c) no generator or no domain to sabotage")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
