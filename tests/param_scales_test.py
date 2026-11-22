#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/param_scales_test.py: the parameter scales of protocol.h (PHD_PARAM_SCALES) are omx-contract's PARAM_SCALES, read from
its domain `param` (sorting pass, V21 PR 3, row M1): this package keeps no copy of the three words.

(a) golden: the rows of PHD_PARAM_SCALES, expanded by tests/param_scales_dump.c, are the rows frozen from the hand-written header
    at ce565fa (tests/golden/param-scales.txt, 3 rows);
(b) the domain param, read as tools/param-scales-gen.py reads it (the contract directory OMX_CONTRACT_PATH names; this test
    names tests/fixtures/contract.d, the interface omx-contract's domain gives, until omx-contract 3.21.0 installs it) holds
    exactly those rows, and protocol.h is the generator's output (--check);
(c) sabotage, in a scratch contract directory: a scale word removed, or changed, makes --check fail, and writing it changes
    what the call expands to; no domain at all is a refusal naming the domain;
(d) no reader of the old home is left: tools/protocol-gen.c includes no omx-contract header and spells no OMX_PARAM_SCALES.

Environment: CC. Exit 0 all green, 1 a FAIL line."""
import json, os, re, shlex, shutil, subprocess, sys, tempfile

sys.dont_write_bytecode = True
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CC = shlex.split(os.environ.get("CC", "cc"))
FIXTURE = os.path.join(ROOT, "tests", "fixtures", "contract.d")
GEN = os.path.join(ROOT, "tools", "param-scales-gen.py")
failed = 0


def check(cond, what):
    global failed
    print(("ok   " if cond else "FAIL ") + what)
    if not cond:
        failed += 1


def dump(tree, tmp):
    exe = os.path.join(tmp, "dump")
    b = subprocess.run(CC + ["-Wall", "-Wextra", "-Werror", "-I" + os.path.join(tree, "include"), "-o", exe,
                             os.path.join(ROOT, "tests", "param_scales_dump.c")], capture_output=True, text=True)
    if b.returncode != 0:
        return None, b.stderr
    return subprocess.run([exe], capture_output=True, text=True).stdout, ""


def snake(title):
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", title).upper()


def meaning(w):
    text = w["description"].rstrip(".")
    first = text.split(" ")[0]
    return text if any(c.isupper() for c in first[1:]) else text[0].lower() + text[1:]


def gen(args, contract_path, cwd):
    env = dict(os.environ, OMX_CONTRACT_PATH=contract_path)
    return subprocess.run([sys.executable, os.path.join(cwd, "tools", "param-scales-gen.py")] + args, cwd=cwd, env=env,
                          capture_output=True, text=True)


def main():
    gold = open(os.path.join(ROOT, "tests", "golden", "param-scales.txt")).read()
    with tempfile.TemporaryDirectory() as tmp:
        got, err = dump(ROOT, tmp)
        check(got == gold, "(a) PHD_PARAM_SCALES of protocol.h is the golden 3 rows %s" % err.strip()[-200:])
        doc_path = os.path.join(FIXTURE, "param", "param.json")
        doc = json.load(open(doc_path))
        ws = sorted(doc["components"]["schemas"]["PARAM_SCALES"]["oneOf"], key=lambda w: w["x-code"])
        rows = "".join("scales %d %s %s %s\n" % (i, snake(w["title"]), w["const"], meaning(w)) for i, w in enumerate(ws))
        check(rows == gold, "(b) the domain param holds exactly the golden rows, meanings included")
        r = gen(["--check"], FIXTURE, ROOT)
        check(r.returncode == 0, "(b) protocol.h's PHD_PARAM_SCALES is the generator's output (--check) %s" % r.stdout.strip()[-120:])
        # (c) sabotage in a scratch tree and a scratch contract directory
        for what, edit in (("a word removed", lambda d: d["components"]["schemas"]["PARAM_SCALES"]["oneOf"].pop(1)),
                           ("a word changed", lambda d: d["components"]["schemas"]["PARAM_SCALES"]["oneOf"][1].update(const="log-x"))):
            scratch = os.path.join(tmp, "tree-" + what.replace(" ", "-"))
            shutil.copytree(ROOT, scratch, ignore=shutil.ignore_patterns(".git", "build", "node_modules"))
            d = json.load(open(doc_path))
            edit(d)
            sdir = os.path.join(tmp, "dom-" + what.replace(" ", "-"))
            os.makedirs(os.path.join(sdir, "param"))
            json.dump(d, open(os.path.join(sdir, "param", "param.json"), "w"), indent=2)
            c = gen(["--check"], sdir, scratch)
            check(c.returncode != 0, "(c) %s in the domain makes --check fail" % what)
            gen([], sdir, scratch)
            out, _ = dump(scratch, tmp)
            check(out is not None and out != gold, "(c) writing it changes the macro the call expands to (%s)" % what)
        empty = os.path.join(tmp, "empty")
        os.makedirs(empty)
        # the installed omx-contract is searched after OMX_CONTRACT_PATH (tools/domain_path.py): a box that installs one
        # carrying the param domain would answer this arm, so pkg-config names the empty directory as the installed one
        pc = os.path.join(tmp, "pc")
        os.makedirs(pc)
        open(os.path.join(pc, "omx-contract.pc"), "w").write("contractdir=%s\nName: omx-contract\nDescription: none\nVersion: 0\n" % empty)
        os.environ["PKG_CONFIG_LIBDIR"] = pc
        try:
            c = gen(["--check"], empty, ROOT)
        finally:
            del os.environ["PKG_CONFIG_LIBDIR"]
        check(c.returncode != 0 and "no domain param" in (c.stderr + c.stdout), "(c) no domain param in the contract directories is a refusal naming it")
    src = open(os.path.join(ROOT, "tools", "protocol-gen.c")).read()
    check("omxcontract/" not in src and "OMX_PARAM_SCALES" not in src, "(d) tools/protocol-gen.c reads no omx-contract header and spells no OMX_PARAM_SCALES")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
