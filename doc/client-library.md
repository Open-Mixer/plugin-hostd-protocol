# A client library and a fake daemon, from the protocol's own table

## The question

This repository declares the protocol and serves it in C, and ships no client. Every controller and test writes its
own: a shared test client in plugin-hostd (`tests/lib/plugin_hostd_client.py`), Zynthian's engine client, the
console's transport and encoders, plugin-qualify's, and about twenty framing loops in tools and benches. They differ
in how they frame, quote and connect. The note fixes four points for a Python library, its TypeScript twin and a fake
daemon generated from `protocol/plugin-hostd-protocol.json`. The existing call sites and Zynthian's client then move
to the library, and Zynthian's tests read back from the fake.

## What is ruled

- The protocol is this repository's: one table, `protocol/plugin-hostd-protocol.json`, and the grammar written from
  it. A client derives its words from them and types none.
- A declaration and its twin live with their owner, as plugin-db keeps its reader's TypeScript twin in its own `ts/`.

## The four points, from the code

**The feedback port.** plugin-hostd reads no command of a controller until that controller has opened the feedback
port (plugin-hostd's `src/main.c`, `receive`); a host on this library accepts the command connection, then the
feedback one, before it reads anything (`src/socket.c`, `socket_run`). So the client connects the command port, then
the feedback port, the order plugin-hostd uses for its own workers, and sends nothing before both are open. The feedback port is the
command port plus one unless the host was started with `-f`; with `-n` and no `-f` there is none and none is opened.

**Framing.** Every command, reply and feedback line ends in one NUL. The client splits on the NUL only, never on a
newline, and keeps the bytes after it for the next message: one read may hold several replies or part of one. A peer
that closes before the NUL is an error, never a wait. The unterminated tail is capped at the library's own limit,
`SOCKET_MSG_MAX_SIZE`, 16 MiB, and the connection is closed past it, as the library does with a command.

**`word()`.** From `grammar/core.abnf`: a value with no space, control character or double quote goes bare; any
other goes in double quotes, a quote inside written `\"`, nothing else escaped, a backslash standing for itself. A
value holding a control character, a newline included, has no spelling and is refused, as is one past `string_max`
(255 bytes). A reply is `resp <status> [words]`: a negative status is a refusal, named by `reply_codes`; a parse
refusal is one of `protocol_errors`. The client sends `hello` with the extensions it wants and uses only those
granted.

**Where the TypeScript lives.** In this repository, beside the JSON it is generated from, with the Python.

## Divergences, as the code stands

1. The console's transport splits on a NUL or a newline and has no cap; a change under review frames on the NUL
   alone with the 16 MiB cap, and trims a CR or LF before the NUL, which the library and the grammar do not.
2. Several framing loops wait for a buffer that ends in a NUL, so a second reply in the same read is lost; one loops
   on a closed peer. The shared test client keeps the tail, and opens no feedback port.
3. Three quoting rules: the console's encoder refuses whitespace, plugin-qualify quotes, Zynthian quotes and doubles
   backslashes, which the grammar does not undo, so `a\b` arrives as `a\\b`.
4. The 16 MiB cap is a C macro in `include/socket.h`; the JSON's `line_max` (4096) is the initial buffer.
5. The tokenizer (`src/utils.c`) reads `""` inside a quoted word as a quote; the grammar has no such form.
6. Fakes exist apart: plugin-hostd's `fake-host`, Zynthian's `tests/fake_plugin_hostd.py`, two in the console.

## Open

- The npm package name, and whether it is published.
- Where the 16 MiB cap is declared in the JSON.
- A quoted value that ends in a backslash: the grammar reads it, the tokenizer does not.
- What the fake answers for the 44 of 92 commands with no declared reply, and whether it keeps state or plays a
  script; and whether it ships here or in `plugin-hostd-tests`.
