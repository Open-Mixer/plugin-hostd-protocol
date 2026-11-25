#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tools/served-floor.py: refuse a release below the highest version any of its packages was ever served at (#180).

python3-plugin-hostd-protocol was built by plugin-hostd up to 0.20.6 and moved here at 0.6.0; this source then released
it at 0.8.1, which apt takes only as a downgrade, so a plain upgrade kept 0.20.6. A version only goes forward: every
package this source builds, by its Debian and its RPM name, must be at or above the highest version the channel ever
served under that name, whichever source built it. The same version is not refused here (the release workflow's
preflight refuses a republish).

What the channel served is the history of its deb/cells.tsv and rpm/cells.tsv (one row per package, version and cell,
kept by every publish since 2026-11-13), read from the channel repository's git log, so a version pruned since is still
counted. The channel is a blobless clone made here, or a checkout named by --channel.

Usage: served-floor.py [--root DIR] [--version V] [--channel GITDIR | --url URL]
Exit 0: every package at or above its floor; 1: one is below, each named with the version it was served at; 2: the
channel could not be read (not a pass).
"""
import argparse
import os
import re
import subprocess
import sys
import tempfile

CHANNEL_URL = "https://github.com/Open-Mixer/open-mixer.github.io"
LEDGERS = ["deb/cells.tsv", "rpm/cells.tsv"]
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def built_names(root):
    """every binary package name of debian/control and of the spec"""
    names = set(re.findall(r"^Package:\s*(\S+)", open(os.path.join(root, "debian/control"), encoding="utf-8").read(), re.M))
    spec = open(os.path.join(root, "packaging/plugin-hostd-protocol.spec"), encoding="utf-8").read()
    main = re.search(r"^Name:\s*(\S+)", spec, re.M).group(1)
    names.add(main)
    for own, sub in re.findall(r"^%package\s+(-n\s+)?(\S+)", spec, re.M):
        names.add(sub if own else "%s-%s" % (main, sub))
    return names


def _order(c):
    """Debian's order of one character of a non-digit part: ~ below the end, letters below the rest"""
    if c == "~":
        return -1
    if c.isalpha():
        return ord(c)
    return ord(c) + 256


def _cmp_part(a, b):
    while a or b:
        na, nb = re.match(r"\D*", a).group(), re.match(r"\D*", b).group()
        a, b = a[len(na):], b[len(nb):]
        for i in range(max(len(na), len(nb))):
            x = _order(na[i]) if i < len(na) else 0
            y = _order(nb[i]) if i < len(nb) else 0
            if x != y:
                return -1 if x < y else 1
        da, db = re.match(r"\d*", a).group(), re.match(r"\d*", b).group()
        a, b = a[len(da):], b[len(db):]
        if int(da or 0) != int(db or 0):
            return -1 if int(da or 0) < int(db or 0) else 1
    return 0


def vercmp(a, b):
    """Debian version comparison of two versions (no epochs here: the house has none), revision after upstream"""
    ua, _, ra = a.partition("-")
    ub, _, rb = b.partition("-")
    return _cmp_part(ua, ub) or _cmp_part(ra, rb)


def served(gitdir):
    """{name: highest version} over every row the ledgers ever held: each was added once, removed since or not"""
    log = subprocess.run(["git", "-C", gitdir, "log", "-p", "--format=", "--"] + LEDGERS,
                         capture_output=True, text=True, check=True).stdout
    rows = [line[1:] for line in log.splitlines() if line[:1] == "+" and not line.startswith("+++")]
    for ledger in LEDGERS:  # the tip too: a ledger written in one commit with no diff before it is still read
        tip = subprocess.run(["git", "-C", gitdir, "show", "HEAD:" + ledger], capture_output=True, text=True)
        rows += tip.stdout.splitlines() if tip.returncode == 0 else []
    top = {}
    for row in rows:
        cols = row.split("\t")
        if len(cols) < 2 or cols[0].startswith("#") or not re.match(r"\d", cols[1]):
            continue
        if cols[0] not in top or vercmp(cols[1], top[cols[0]]) > 0:
            top[cols[0]] = cols[1]
    return top


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--root", default=ROOT)
    ap.add_argument("--version", help="the version released (default: VERSION of the tree)")
    ap.add_argument("--channel", help="a git checkout of the channel repository (default: a blobless clone of --url)")
    ap.add_argument("--url", default=CHANNEL_URL)
    args = ap.parse_args()
    version = args.version or open(os.path.join(args.root, "VERSION"), encoding="utf-8").read().strip()
    names = built_names(args.root)
    try:
        if args.channel:
            top = served(args.channel)
        else:
            with tempfile.TemporaryDirectory() as tmp:
                subprocess.run(["git", "clone", "-q", "--filter=blob:none", "--no-checkout", args.url, tmp],
                               check=True, capture_output=True, text=True)
                top = served(tmp)
    except (subprocess.CalledProcessError, OSError) as e:
        print("served-floor: cannot read the channel: %s" % (getattr(e, "stderr", "") or e), file=sys.stderr)
        return 2
    if not top:
        print("served-floor: the channel's ledgers hold no row: not read, not a pass", file=sys.stderr)
        return 2
    below = sorted(n for n in names if n in top and vercmp(version, top[n]) < 0)
    for n in below:
        print("FAIL %s %s is below %s, which the channel served: a version never goes back (no epochs)" % (n, version, top[n]))
    if not below:
        print("ok   %s: each of the %d packages at or above the highest version the channel ever served it at" % (version, len(names)))
    return 1 if below else 0


if __name__ == "__main__":
    sys.exit(main())
