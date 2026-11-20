#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/probe_words_test.py: the words of the probe output document (reasons, outcomes, steps, signals) are this package's
domain `probe` (contract.d/probe/probe.json), and the rows of protocol.h's PHD_PROBE_REASONS, PHD_PROBE_OUTCOMES and
PHD_PROBE_STEPS are generated from it, the meaning texts included (sorting pass V8).

(a) golden: the three lists of protocol.h, expanded by tests/probe_words_dump.c, are the rows frozen from the hand-written
    header (tests/golden/probe-words.txt, 34 rows);
(b) the domain holds exactly those rows: the word, the code (its place), the meaning (its description, first letter
    lowered, no final stop), and include/plugin-hostd/probe_codes.h is the render of its x-c-codes items;
(c) sabotage, in a scratch copy of the tree: a word, or a meaning, changed in the domain makes tools/probe-words-gen.py
    --check fail, and writing it changes the macro the call expands to.

Environment: OMX_CONTRACT (the command, default omx-contract), CC. Exit 0 all green, 1 a FAIL line."""
import json, os, re, shlex, shutil, subprocess, sys, tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OMX = shlex.split(os.environ.get("OMX_CONTRACT", "omx-contract"))
CC = shlex.split(os.environ.get("CC", "cc"))
LISTS = (("reasons", "PROBE_REASONS"), ("outcomes", "PROBE_OUTCOMES"), ("steps", "PROBE_STEPS"))
failed = 0


def check(cond, what):
    global failed
    print(("ok   " if cond else "FAIL ") + what)
    if not cond:
        failed += 1


def dump(tree, tmp):
    exe = os.path.join(tmp, "dump")
    b = subprocess.run(CC + ["-Wall", "-Wextra", "-Werror", "-I" + os.path.join(tree, "include"), "-o", exe,
                             os.path.join(ROOT, "tests", "probe_words_dump.c")], capture_output=True, text=True)
    if b.returncode != 0:
        return None, b.stderr
    return subprocess.run([exe], capture_output=True, text=True).stdout, ""


def meaning(w):
    text = w["description"].rstrip(".")
    return text[0].lower() + text[1:] if len(text) > 1 and text[1].islower() else text


def main():
    gold = open(os.path.join(ROOT, "tests", "golden", "probe-words.txt")).read()
    with tempfile.TemporaryDirectory() as tmp:
        got, err = dump(ROOT, tmp)
        check(got == gold, "(a) the three lists of protocol.h are the golden 34 rows %s" % err.strip()[-200:])
        doc_path = os.path.join(ROOT, "contract.d", "probe", "probe.json")
        if not os.path.isfile(doc_path):
            check(False, "(b) contract.d/probe/probe.json does not exist")
        else:
            doc = json.load(open(doc_path))
            rows = []
            for name, item in LISTS:
                ws = sorted(doc["components"]["schemas"][item]["oneOf"], key=lambda w: w["x-code"])
                for i, w in enumerate(ws):
                    rows.append("%s %d %s %s %s" % (name, i, re.sub(r"^PROBE_", "", re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", w["title"]).upper())
                                                    if name == "outcomes" else re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", w["title"]).upper(),
                                                    w["const"], meaning(w)))
            check("\n".join(rows) + "\n" == gold, "(b) the domain holds exactly the golden rows, meanings included")
            r = subprocess.run(OMX + ["render", "--target", "c", "--package", "probe", "--contract-dir", "contract.d",
                                      "--out", "include/plugin-hostd", "--check"], cwd=ROOT, capture_output=True, text=True)
            check(r.returncode == 0, "(b) include/plugin-hostd/probe_codes.h is the render of the domain: %s" % (r.stderr.strip() or r.stdout.strip())[-200:])
        gen = os.path.join(ROOT, "tools", "probe-words-gen.py")
        r = subprocess.run([sys.executable, gen, "--check"], cwd=ROOT, capture_output=True, text=True) if os.path.isfile(gen) else None
        check(r is not None and r.returncode == 0, "(b) protocol.h's three lists are the generator's output (--check)")
        # (c) sabotage in a scratch copy
        if os.path.isfile(gen) and os.path.isfile(doc_path):
            for what, edit in (("a word", lambda w: w.update(const=w["const"] + "-x")),
                               ("a meaning", lambda w: w.update(description="Changed meaning."))):
                scratch = os.path.join(tmp, "tree-" + what.split()[1])
                shutil.copytree(ROOT, scratch, ignore=shutil.ignore_patterns(".git", "build", "node_modules"))
                p = os.path.join(scratch, "contract.d", "probe", "probe.json")
                d = json.load(open(p))
                edit(d["components"]["schemas"]["PROBE_OUTCOMES"]["oneOf"][1])
                json.dump(d, open(p, "w"), indent=2)
                c = subprocess.run([sys.executable, os.path.join(scratch, "tools", "probe-words-gen.py"), "--check"], cwd=scratch, capture_output=True, text=True)
                check(c.returncode != 0, "(c) %s changed in the domain makes --check fail" % what)
                subprocess.run([sys.executable, os.path.join(scratch, "tools", "probe-words-gen.py")], cwd=scratch, capture_output=True, text=True)
                out, _ = dump(scratch, tmp)
                check(out is not None and out != gold and ("probe.crashed-x" in out or "Changed meaning" in out.replace("changed meaning", "Changed meaning")),
                      "(c) writing it changes the macro the call expands to (%s)" % what)
        else:
            check(False, "(c) no generator or no domain to sabotage")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
