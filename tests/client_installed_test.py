# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""The protocol's Python client as its consumers get it (issue 45): python3-plugin-hostd-client and the codec beside
it, python3-plugin-hostd-protocol, as make install-python puts them, imported with no source tree and no plugin-hostd
on the path. omx-clap-core and plugin-hostd run their tests this way; a client that still reached into a checkout, or
that could not find its codec once installed, would pass every test of this tree and fail theirs.

Run with python3 -I, so neither this tree nor the current directory nor PYTHONPATH is on sys.path:

  python3 -I tests/client_installed_test.py SITELIB TREE

  SITELIB   the directory install-python wrote plugin_hostd_client and plugin_hostd_protocol into
  TREE      this source tree: no sys.path entry may lie inside it

The table, vocabulary and catalogs come from PLUGIN_HOSTD_PROTOCOL_TABLE, PLUGIN_HOSTD_PROTOCOL_DATADIR and
OMX_CONTRACT_PATH, as for tests/codec_test.py. The line command() sends is written out here, never rebuilt with the
codec under test.
"""
import os
import socket
import sys
import threading


def inside(path, root):
    path, root = os.path.realpath(path), os.path.realpath(root)
    return path == root or path.startswith(root + os.sep)


def main():
    if len(sys.argv) != 3:
        print("usage: python3 -I tests/client_installed_test.py SITELIB TREE")
        return 2
    sitelib, tree = sys.argv[1], sys.argv[2]
    fails = []
    for p in sys.path:
        if p and (inside(p, tree) or p.rstrip("/").endswith("plugin-hostd/tests/lib")):
            fails.append("sys.path holds %s: run with python3 -I from outside the tree" % p)
    if fails:
        print("FAIL", *fails, sep="\n  ")
        return 1
    sys.path.insert(0, sitelib)
    try:
        import plugin_hostd_client
        import plugin_hostd_protocol
    except ImportError as e:
        print("FAIL the installed client does not import: %s" % e)
        return 1
    for m in (plugin_hostd_client, plugin_hostd_protocol):
        if not inside(m.__file__, sitelib):
            fails.append("%s comes from %s, not from %s" % (m.__name__, m.__file__, sitelib))

    srv = socket.socket()
    srv.bind(("127.0.0.1", 0))
    srv.listen(1)
    got = []

    def peer():
        s, _ = srv.accept()
        buf = b""
        while b"\0" not in buf:
            d = s.recv(4096)
            if not d:
                break
            buf += d
        got.append(buf.split(b"\0", 1)[0].decode())
        s.sendall(b"resp 0\0")
        s.close()

    t = threading.Thread(target=peer, daemon=True)
    t.start()
    c = plugin_hostd_client.Client.connect(srv.getsockname()[1])
    try:
        reply = c.command("remove", 3)
    except Exception as e:  # noqa: BLE001 - every failure of the round trip is the finding
        fails.append("command('remove', 3): %r" % e)
        reply = None
    t.join(5)
    if got != ["remove 3"]:
        fails.append("command('remove', 3) sent %r, wanted 'remove 3'" % got)
    if reply != "resp 0":
        fails.append("command('remove', 3) returned %r, wanted 'resp 0'" % reply)
    parsed = c.reply("resp 0")
    if (parsed.get("kind"), parsed.get("code")) != ("resp", 0):
        fails.append("reply('resp 0') gave %r" % parsed)
    c.close()
    srv.close()
    if fails:
        print("FAIL", *fails, sep="\n  ")
        return 1
    print("ok   the installed client and codec, alone on the path, send 'remove 3' and parse 'resp 0'")
    return 0


if __name__ == "__main__":
    sys.exit(main())
