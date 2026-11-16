#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# The protocol's Python client (clients/python/plugin_hostd_client, the package python3-plugin-hostd-client) against a peer
# that answers in NUL-ended replies split across writes: send returns the reply, expect raises on the wrong one, try_send
# names a timeout and a closed peer. Then its sabotage: a copy that frames by newline instead of NUL must fail the same
# checks, so the checks can tell a client that frames wrongly. Moved here from plugin-hostd's tests/protocol_lib.py with
# the client (plugin-hostd-protocol issue 45): the protocol's owner ships the protocol's client.
#
#   client_test.py            0 when the client passes and its newline copy fails
#   PLUGIN_HOSTD_CLIENT_DIR   another directory holding plugin_hostd_client (an installed copy under test)
import os, socket, sys, tempfile, threading, time

HERE = os.path.dirname(os.path.abspath(__file__))
CLIENT_DIR = os.environ.get("PLUGIN_HOSTD_CLIENT_DIR") or os.path.join(HERE, "..", "clients", "python")


def serve(srv, stop):
    """echo "resp <line>" as one reply, written in two pieces; "mute" is never answered; "bye" closes"""
    while not stop.is_set():
        try:
            c, _ = srv.accept()
        except OSError:
            return
        buf = b""
        while True:
            try:
                d = c.recv(4096)
            except OSError:
                break
            if not d:
                break
            buf += d
            while b"\0" in buf:
                line, buf = buf.split(b"\0", 1)
                if line == b"mute":
                    continue
                if line == b"bye":
                    c.close()
                    break
                reply = b"resp " + line + b"\0"
                c.sendall(reply[:3])
                time.sleep(0.02)
                c.sendall(reply[3:])
        c.close()


def check_python(libdir, port, failures):
    sys.path.insert(0, libdir)
    sys.modules.pop("plugin_hostd_client", None)
    try:
        import plugin_hostd_client as m
    except ImportError as e:
        failures.append("import from %s: %s" % (libdir, e))
        sys.path.pop(0)
        return
    c = m.Client.connect(port)
    if c.send("add x 0") != "resp add x 0":
        failures.append("python send")
    if c.expect("ping", "resp ping") != "resp ping":
        failures.append("python expect")
    try:
        c.expect("ping", "resp pong")
        failures.append("python expect of the wrong reply raised nothing")
    except AssertionError:
        pass
    if c.try_send("mute", 0.2) != (None, "timeout"):
        failures.append("python timeout")
    c2 = m.Client.connect(port)
    if c2.try_send("bye", 1) != (None, "closed"):
        failures.append("python closed")
    sys.path.pop(0)


def main():
    srv = socket.socket()
    srv.bind(("127.0.0.1", 0))
    srv.listen(16)
    port = srv.getsockname()[1]
    stop = threading.Event()
    for _ in range(8):
        threading.Thread(target=serve, args=(srv, stop), daemon=True).start()
    src = os.path.join(CLIENT_DIR, "plugin_hostd_client", "__init__.py")
    if not os.path.isfile(src):
        print("FAIL no client at %s (PLUGIN_HOSTD_CLIENT_DIR, or clients/python of this tree)" % src)
        return 1
    good = []
    check_python(CLIENT_DIR, port, good)
    if good:
        print("FAIL the client:", *good, sep="\n  ")
        return 1
    print("ok: the python client frames by NUL, joins split replies, times out and sees a close")
    # sabotage: frame by newline, and the same checks must fail
    with tempfile.TemporaryDirectory() as d:
        py = open(src).read().replace('b"\\0"', 'b"\\n"')
        if py == open(src).read():
            print("FAIL the sabotage changed nothing")
            return 1
        os.mkdir(os.path.join(d, "plugin_hostd_client"))
        open(os.path.join(d, "plugin_hostd_client", "__init__.py"), "w").write(py)
        bad = []
        try:
            check_python(d, port, bad)
        except Exception as e:
            bad.append(repr(e))
        if not bad:
            print("FAIL a client that frames by newline passed")
            return 1
    print("ok: a client that frames by newline fails the same checks")
    stop.set()
    srv.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
