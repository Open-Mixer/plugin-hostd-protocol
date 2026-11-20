# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""The Python client's command() and reply() speak through the codec (#34): the line command() sends is the one the
protocol's own fixtures (clients/fixtures/codec.json) give for the same call, written out here as fixed strings, never
rebuilt with the codec under test, and reply() is the codec's parse. A command() that quotes by hand sends another
line for the quoted cases and fails; tests/client-codec-sabotage.sh does that to a scratch copy and must see this go red.

  client_codec_test.py        0 when command() and reply() go through the codec
  CLIENT_CODEC_TREE           another tree holding clients/python (the sabotage's scratch copy)
"""
import json
import os
import socket
import sys
import threading

ROOT = os.environ.get("CLIENT_CODEC_TREE") or os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
sys.path.insert(0, os.path.join(ROOT, "clients", "python"))
import plugin_hostd_client as client  # noqa: E402

# (verb, args, tag, the line the fixtures give for it)
CASES = [
    ("add", ("lv2:urn:example:gain", 3), None, "add lv2:urn:example:gain 3"),
    ("param_get", (9, "gain"), "t1", "@t1 param_get 9 gain"),
    ("preset_load", (3, "/home/user/my presets/a.json"), None, 'preset_load 3 "/home/user/my presets/a.json"'),
    ("preset_load", (3, 'say "hi"'), None, 'preset_load 3 "say \\"hi\\""'),
    ("param_set", (3, "", 0.25), None, 'param_set 3 "" 0.25'),
]
failed = 0


def check(what, got, want):
    global failed
    if got == want:
        print("ok   %s" % what)
    else:
        failed += 1
        print("FAIL %s: %r, wanted %r" % (what, got, want))


def sent(verb, args, tag):
    """the line the daemon would read when command() sends verb and args; a peer answers resp 0"""
    a, b = socket.socketpair()
    seen = []
    b.settimeout(3)

    def peer():
        buf = b""
        try:
            while b"\0" not in buf:
                buf += b.recv(4096)
            seen.append(buf.split(b"\0", 1)[0].decode())
            b.sendall(b"resp 0\0")
        except OSError:
            pass

    t = threading.Thread(target=peer, daemon=True)
    t.start()
    try:
        client.Client(a, timeout=5).command(verb, *args, tag=tag)
    except AttributeError as e:
        print("FAIL command() is missing: %s" % e)
    finally:
        t.join(5)
        a.close()
        b.close()
    return seen[0] if seen else None


fixtures = {(c["in"][0], tuple(c["in"][1]), c["in"][2] if len(c["in"]) > 2 else None): c["expect"]
            for c in json.load(open(os.path.join(ROOT, "clients", "fixtures", "codec.json")))["cases"]
            if c["op"] == "encode" and "expect" in c}
for verb, args, tag, line in CASES:
    check("the fixtures hold %s %r" % (verb, args), fixtures.get((verb, args, tag)), line)
    check("command(%s, %r) sends the fixtures' line" % (verb, args), sent(verb, args, tag), line)
a, b = socket.socketpair()
c = client.Client(a, timeout=5)
check("reply() is the codec's parse", c.reply("resp 0"), client.protocol().parse("resp 0"))
a.close()
b.close()
sys.exit(1 if failed else 0)
