#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/grouping_reasons_test.py: GroupingReason and RETRY_REASONS of both clients are written from plugin-hostd's domain
`plugin-hostd/grouping`, one generator for the two (sorting pass, V17 PR 2): this package keeps no hand copy of a reason.

(a) golden: the Python client's GroupingReason and RETRY_REASONS, and the same read from the TypeScript client's block, are the 16
    reasons (key, word, group) frozen from plugin-hostd's grouping data file at 455c6ba (clients/fixtures/grouping-reasons.txt);
(b) the domain (the contract directory OMX_CONTRACT_PATH names; this test names tests/fixtures/contract.d, the interface
    plugin-hostd's domain gives, until plugin-hostd 0.21.0 installs it) holds exactly those rows, and both blocks are the
    generator's output (--check);
(c) sabotage, in a scratch tree and contract directory: a reason removed, a word changed, a group flipped makes --check fail, and
    writing it changes what the clients hold; no domain at all is a refusal naming it;
(d) no reader of the old home is left: nothing in the tree names plugin-hostd's data file or the variable that pointed at it.

Exit 0 all green, 1 a FAIL line."""
import importlib.util, json, os, re, shutil, subprocess, sys, tempfile

sys.dont_write_bytecode = True
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FIXTURE = os.path.join(ROOT, "tests", "fixtures", "contract.d")
DOMAIN = os.path.join("plugin-hostd", "grouping", "grouping.json")
failed = 0


def check(cond, what):
    global failed
    print(("ok   " if cond else "FAIL ") + what)
    if not cond:
        failed += 1


def python_client(tree):
    spec = importlib.util.spec_from_file_location("phd_client_" + str(abs(hash(tree))), os.path.join(tree, "clients", "python", "plugin_hostd_protocol", "__init__.py"))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    words = {k: v for k, v in vars(mod.GroupingReason).items() if k.isupper()}
    return words, list(mod.RETRY_REASONS)


def ts_block(tree):
    text = open(os.path.join(tree, "clients", "typescript", "codec", "src", "index.ts")).read()
    m = re.search(r"export const GroupingReason = \{\n(.*?)\n\} as const;", text, re.S)
    rows = re.findall(r"^  (\w+): '([^']+)',$", m.group(1), re.M)
    retry = re.search(r"export const RETRY_REASONS: readonly string\[\] = \[(.*?)\];", text).group(1)
    return rows, re.findall(r"GroupingReason\.(\w+)", retry)


def gen(args, contract_path, cwd, **extra):
    env = dict(os.environ, OMX_CONTRACT_PATH=contract_path, **extra)
    return subprocess.run([sys.executable, os.path.join(cwd, "tools", "grouping-reasons.py")] + args, cwd=cwd, env=env, capture_output=True, text=True)


def main():
    gold = [l.split() for l in open(os.path.join(ROOT, "clients", "fixtures", "grouping-reasons.txt")).read().splitlines()]
    check(len(gold) == 16, "the golden file holds the 16 reasons")

    def snake(w):
        return w.upper().replace("-", "_")

    def clients_are_golden(tree):
        py, py_retry = python_client(tree)
        rows, retry = ts_block(tree)
        return (py == {snake(w): w for _, w, _g in gold} and py_retry == [w for _, w, g in gold if g == "retry"]
                and rows == [(k, w) for k, w, _g in gold] and retry == [k for k, _w, g in gold if g == "retry"])

    check(clients_are_golden(ROOT), "(a) both clients hold the golden 16 reasons and RETRY_REASONS = no-verdict, plan-refused")
    doc = json.load(open(os.path.join(FIXTURE, DOMAIN)))
    items = doc["components"]["schemas"]
    rows = [[w["title"], w["const"], w["x-group"]] for w in sorted(items["PLUGIN_HOSTD_GROUPING_REASONS"]["oneOf"], key=lambda w: w["x-code"])]
    check(rows == gold, "(b) the domain holds exactly the golden reasons and their groups")
    r = gen(["--check"], FIXTURE, ROOT)
    check(r.returncode == 0, "(b) both client blocks are the generator's output (--check) %s" % r.stdout.strip()[-120:])
    with tempfile.TemporaryDirectory() as tmp:
        arms = (("a reason removed", lambda i: i["PLUGIN_HOSTD_GROUPING_REASONS"]["oneOf"].pop(3)),
                ("a word changed", lambda i: i["PLUGIN_HOSTD_GROUPING_REASONS"]["oneOf"][0].update(const="refused-x")),
                ("a group flipped", lambda i: i["PLUGIN_HOSTD_GROUPING_REASONS"]["oneOf"][0].update({"x-group": "retry"})))
        for what, edit in arms:
            tag = what.replace(" ", "-")
            scratch = os.path.join(tmp, "tree-" + tag)
            shutil.copytree(ROOT, scratch, ignore=shutil.ignore_patterns(".git", "build", "node_modules"))
            d = json.load(open(os.path.join(FIXTURE, DOMAIN)))
            edit(d["components"]["schemas"])
            sdir = os.path.join(tmp, "dom-" + tag)
            os.makedirs(os.path.dirname(os.path.join(sdir, DOMAIN)))
            json.dump(d, open(os.path.join(sdir, DOMAIN), "w"), indent=2)
            check(gen(["--check"], sdir, scratch).returncode != 0, "(c) %s in the domain makes --check fail" % what)
            gen([], sdir, scratch)
            check(not clients_are_golden(scratch), "(c) writing it changes what the clients hold (%s)" % what)
        empty = os.path.join(tmp, "empty")
        os.makedirs(empty)
        # the installed place too: pkg-config answers a scratch omx-contract whose contractdir is empty, so a box where
        # plugin-hostd is installed (its domain under the real contractdir) still has no domain anywhere
        pc = os.path.join(tmp, "pc")
        os.makedirs(pc)
        open(os.path.join(pc, "omx-contract.pc"), "w").write("contractdir=%s\nName: omx-contract\nDescription: scratch\nVersion: 0\n" % empty)
        c = gen(["--check"], empty, ROOT, PKG_CONFIG_LIBDIR=pc, PKG_CONFIG_PATH=pc)
        check(c.returncode != 0 and "no domain plugin-hostd/grouping" in (c.stderr + c.stdout), "(c) no domain in the contract directories is a refusal naming it")
    old = ["data/" + "grouping.json", "PLUGIN_HOSTD_" + "GROUPING_JSON"]
    hits = subprocess.run(["git", "grep", "-n", "-F", "-e", old[0], "-e", old[1]], cwd=ROOT, capture_output=True, text=True).stdout.strip()
    check(hits == "", "(d) no reader of plugin-hostd's data file is left %s" % hits[:160])
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
