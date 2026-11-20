# SPDX-License-Identifier: GPL-3.0-or-later
"""The plugin-hostd line protocol from Python, the one client every test of plugin-hostd, omx-clap-host and the
controllers imports: a command is one line ended by a NUL, a reply is one NUL-ended line. The protocol's owner ships it:
the package python3-plugin-hostd-client (make install-python), importable as plugin_hostd_client; in this tree it is
clients/python/plugin_hostd_client. It carries the transport only; the codec of the declared commands and replies is
plugin_hostd_protocol (plugin-hostd-protocol issue 46 moves it here too). command() and reply() speak through that codec,
imported when first called, so the transport alone needs no codec installed.

    c = Client(sock)            a connected socket, or Client.connect(port)
    c.send(line)                the reply; ConnectionError when the peer closes, socket.timeout when none comes in time
    c.try_send(line, timeout)   (reply, None), or (None, 'closed' | 'timeout' | 'oserror <why>'): a test that expects a death
    c.expect(line, wanted)      the reply, AssertionError when it is not wanted
    c.command(verb, *args)      send(), its line built by plugin_hostd_protocol's Protocol().encode(verb, args)
    c.reply(line)               Protocol().parse(line): a reply already read, structured
"""  # noqa: D205
import socket
import time


_protocol = None


def protocol():
    """plugin_hostd_protocol.Protocol(), built once: the codec every command() and reply() goes through."""
    global _protocol
    if _protocol is None:
        import plugin_hostd_protocol
        _protocol = plugin_hostd_protocol.Protocol()
    return _protocol


class Client:  # noqa: D101
    def __init__(self, sock, timeout=10.0):  # noqa: D107
        self.sock = sock
        self.timeout = timeout
        self.buf = b""

    @classmethod
    def connect(cls, port, host="127.0.0.1", timeout=10.0):  # noqa: D102
        return cls(socket.create_connection((host, port), timeout=timeout), timeout)

    def send(self, line, timeout=None):  # noqa: D102
        self.sock.sendall(line.encode() + b"\0")
        end = time.monotonic() + (self.timeout if timeout is None else timeout)
        while b"\0" not in self.buf:
            left = end - time.monotonic()
            if left <= 0:
                raise socket.timeout("no reply to %r" % line)
            self.sock.settimeout(left)
            chunk = self.sock.recv(65536)
            if not chunk:
                raise ConnectionError("the peer closed on " + line)
            self.buf += chunk
        reply, self.buf = self.buf.split(b"\0", 1)
        return reply.decode(errors="replace")

    def try_send(self, line, timeout=None):  # noqa: D102
        try:
            return self.send(line, timeout), None
        except socket.timeout:
            return None, "timeout"
        except ConnectionError:
            return None, "closed"
        except OSError as e:
            return None, "oserror %s" % e

    def expect(self, line, wanted):  # noqa: D102
        reply = self.send(line)
        if reply != wanted:
            raise AssertionError("%s -> %r, wanted %r" % (line, reply, wanted))
        return reply

    def command(self, verb, *args, tag=None, timeout=None):
        """Send the line protocol().encode builds for verb and args; the reply, as send returns it."""
        return self.send(protocol().encode(verb, args, tag), timeout)

    def reply(self, line):
        """A reply already read, structured as protocol().parse gives it."""
        return protocol().parse(line)

    def close(self):  # noqa: D102
        self.sock.close()
