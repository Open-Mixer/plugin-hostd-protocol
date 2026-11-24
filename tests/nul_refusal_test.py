#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""A NUL is the protocol's line end, so a command that holds one is two commands and the replies desync: the Python
client refuses it at its door (Client.send), and the codec refuses it in a value it quotes or a line it splits
(contracts audit 2026-11-16, row 7). Each check below is also what tests/nul-sabotage.sh turns off.

  nul_refusal_test.py       0 when every door refuses; the first FAIL line otherwise
"""
import os, socket, sys

ROOT = os.environ.get("NUL_TREE") or os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "clients", "python"))
import plugin_hostd_client as client
import plugin_hostd_protocol as codec

failed = 0


def refuses(what, fn):
    global failed
    try:
        fn()
    except ValueError as e:
        print("ok   %s is refused (%s)" % (what, e))
    except Exception as e:
        failed += 1
        print("FAIL %s raised %s, not ValueError" % (what, type(e).__name__))
    else:
        failed += 1
        print("FAIL %s is taken" % what)


a, b = socket.socketpair()
c = client.Client(a, timeout=0.5)
refuses("a line with an embedded NUL sent by the client", lambda: c.send("add urn:x 1\0remove 1"))
b.settimeout(0.2)
try:
    got = b.recv(100)
except socket.timeout:
    got = b""
if got:
    failed += 1
    print("FAIL the client wrote %r to the socket before refusing" % got)
else:
    print("ok   nothing reached the socket")
refuses("a value with an embedded NUL quoted by the codec", lambda: codec.quote("a\0b"))
refuses("a line with an embedded NUL split by the codec", lambda: codec.words("add a\0b"))
sys.exit(1 if failed else 0)
