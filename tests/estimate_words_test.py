#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/estimate_words_test.py: the insert verdict of estimate is the host's (coordinator ruling, 2026-11-20). Its
verdict and reason words are this package's domain `estimate` (contract.d/estimate/estimate.json), and
PHD_ESTIMATE_VERDICT_*, PHD_ESTIMATE_REASON_*, PHD_ESTIMATE_VERDICTS, PHD_ESTIMATE_REASONS and their two patterns of
protocol.h are generated from it, the meaning texts included; the reply of estimate ends with the two words.

(a) golden: the two lists, every word, the patterns, the two arg types, the reply as contract.d/plugin-hostd/plugin-hostd.json declares it, its
    row of PHD_REPLY_ARGS and the CPU ceiling, expanded by tests/estimate_words_dump.c, are tests/golden/estimate-words.txt;
(b) the domain holds exactly those rows: word, code (its place), meaning (its description, first letter lowered, no final
    stop), include/plugin-hostd/estimate_codes.h is the render of its x-c-codes items, and protocol.h is the generator's
    output;
(c) sabotage, in a scratch copy of the tree: a word, or a meaning, changed in the domain makes
    tools/estimate-words-gen.py --check fail, and writing it changes what the call expands to;
(d) the CPU ceiling has one holder, the domain's scalar item ESTIMATE_CPU_CEILING (x-kind scalar, x-unit a word of
    omx-contract's PARAM_UNITS, x-value): protocol.h's PHD_ESTIMATE_CPU_CEILING and _UNIT are its render; a value changed in
    the domain changes the rendered protocol.h; a hand #define of the ceiling outside the generated blocks is refused by
    --check, by name.

Environment: OMX_CONTRACT (the command, default omx-contract), CC, OMX_CONTRACT_PATH (the contract directories
tools/domain_path.py reads omx-contract's domain param from, the holder of PARAM_UNITS; then the installed one). Exit 0
all green, 1 a FAIL line."""
import json, os, re, shlex, shutil, subprocess, sys, tempfile

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "tools"))
from domain_path import read_domain, render_env  # noqa: E402  omx-contract's domain param; the renders' contract path

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
            r = subprocess.run(OMX + ["render", "--target", "c", "--package", "estimate", "--installed",
                                      "--out", "include/plugin-hostd", "--check"], cwd=ROOT, env=render_env(ROOT),
                                     capture_output=True, text=True)
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
        ceiling(gold, tmp, doc_path, gen)
    return 1 if failed else 0


def ceiling(gold, tmp, doc_path, gen):
    """(d) the ceiling is the domain's scalar, rendered by the generator and by nothing else"""
    item = None
    if os.path.isfile(doc_path):
        item = json.load(open(doc_path))["components"]["schemas"].get("ESTIMATE_CPU_CEILING")
    want = [l for l in gold.splitlines() if l.startswith("ceiling ")][0].split()
    check(item is not None and item.get("x-kind") == "scalar" and "oneOf" not in item,
          "(d) the domain holds ESTIMATE_CPU_CEILING, a scalar item")
    if item is None:
        check(False, "(d) no ceiling in the domain to hold to the golden row, sabotage or guard")
        return
    units = read_domain("param", "estimate_words_test")["components"]["schemas"]["PARAM_UNITS"]["oneOf"]
    check(item.get("x-unit") in [w["const"] for w in units],
          "(d) its x-unit %r is a word of omx-contract's PARAM_UNITS" % item.get("x-unit"))
    check([str(item.get("x-value")), item.get("x-unit")] == want[1:], "(d) its value and unit are the golden %s" % want[1:])
    text = open(os.path.join(ROOT, "include", "plugin-hostd", "protocol.h")).read()
    outside = re.sub(r"/\* BEGIN GENERATED (\w+):.*?/\* END GENERATED \1 \*/", "", text, flags=re.S)
    check(not re.search(r"#define\s+PHD_ESTIMATE_CPU_CEILING", outside), "(d) protocol.h defines no ceiling by hand")
    if not os.path.isfile(gen):
        check(False, "(d) no generator to sabotage")
        return
    # a value changed in the domain changes the rendered protocol.h, and the expansion
    scratch = os.path.join(tmp, "tree-ceiling")
    shutil.copytree(ROOT, scratch, ignore=shutil.ignore_patterns(".git", "build", "node_modules"))
    p = os.path.join(scratch, "contract.d", "estimate", "estimate.json")
    d = json.load(open(p))
    d["components"]["schemas"]["ESTIMATE_CPU_CEILING"]["x-value"] = 75
    json.dump(d, open(p, "w"), indent=2)
    c = subprocess.run([sys.executable, os.path.join(scratch, "tools", "estimate-words-gen.py"), "--check"], cwd=scratch,
                       capture_output=True, text=True)
    check(c.returncode != 0, "(d) the ceiling's value changed in the domain makes --check fail")
    subprocess.run([sys.executable, os.path.join(scratch, "tools", "estimate-words-gen.py")], cwd=scratch, capture_output=True,
                   text=True)
    out, _ = dump(scratch, tmp)
    check(out is not None and "ceiling 75 percent" in out, "(d) writing it changes the ceiling protocol.h expands to")
    # a hand #define of the ceiling outside the generated blocks is refused by name
    for name in ("PHD_ESTIMATE_CPU_CEILING_PCT", "PHD_ESTIMATE_CPU_CEILING"):
        scratch = os.path.join(tmp, "tree-hand-" + name)
        shutil.copytree(ROOT, scratch, ignore=shutil.ignore_patterns(".git", "build", "node_modules"))
        h = os.path.join(scratch, "include", "plugin-hostd", "protocol.h")
        text = open(h).read()
        open(h, "w").write(text.replace("#endif", "#define %s 60\n#endif" % name, 1) if name.endswith("_PCT") else
                           text.replace("/* ---------------------------------------------------------------- instance verbs */",
                                        "#define %s 60\n/* ---------------------------------------------------------------- "
                                        "instance verbs */" % name, 1))
        c = subprocess.run([sys.executable, os.path.join(scratch, "tools", "estimate-words-gen.py"), "--check"], cwd=scratch,
                           capture_output=True, text=True)
        check(c.returncode != 0 and name in (c.stdout + c.stderr),
              "(d) a hand #define %s is refused by --check, by name: %s" % (name, (c.stdout + c.stderr).strip()[-160:]))


if __name__ == "__main__":
    sys.exit(main())
