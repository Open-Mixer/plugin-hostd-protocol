# The TypeScript client: thin, published with the protocol

`@openmixer/plugin-hostd-protocol` already carries the reply shapes ([reply-shapes.md](reply-shapes.md)). It also carries
the protocol's TypeScript client, the Node twin of `python3-plugin-hostd-client`
([client-library.md](client-library.md)), so that the console's server, a panel's server side and any web surface share
one client and none writes its own transport or retypes a verb.

## What it is

Three things and no more.

| Piece | File | What |
|---|---|---|
| the transport | `clients/typescript/client.mjs` | `connect`, then `send(line)`: one command line out, its one reply line back, as the Python client does |
| the codec | `clients/typescript/codec`, the package's `.` export | `Protocol`, over the table `protocol/plugin-hostd-protocol.json` (installed, or the one `connect({ protocol })` is given): every line the client writes and every reply it reads, and the wire's constants. The one TypeScript encoder of the protocol (#106) |
| one call | `curvePoints(instance, display, n)` | sends `curve_points <instance> <display> <n>` and answers the points and the marks, `{ points: [{ x, y }], marks: [{ freq, depth, q, origin }] }`, which plugin-hostd drew with omx-dsp's `omx_curve_points_marked`, a mark each band a feedback corrector planted |

The words of a line are the codec's: `call(verb, ...args)` is `Protocol.encode(verb, args)`, the verb's first form that
takes the arguments, sent, and the reply read by `Protocol.parse`; `curvePoints` is `call('curve_points', ...)` and
`Protocol.readReply('curve_points', ...)`, the table's reply form, its count held to the number of points. The command port
without `port` is the table's constant `default_command_port`, and the cap on a reply its `message_max`
(`SOCKET_MSG_MAX_SIZE`, written into the table by `tools/protocol-table.py`). A surface without TCP imports `Protocol` from
the package's `.` export; the client keeps no encoder, no parser and no vocabulary of its own
(`tests/one_ts_encoder_test.mjs`).

## Errors are typed

A reply that is not a success is a `HostdError`, never a string to compare. It has `kind`, `reply` (the line), `code` and
`word`:

- `kind: 'refused'`, a `resp <negative>`: `code` is that number and `word` the stable word the table gives it
  (`reply.host.instance-non-exists` for -3), or the word the host sent in a session granted `errors`;
- `kind: 'protocol'`, a parse refusal (`not found`, `few arguments`, ...): `word` is that text and `code` the table's
  number for it.

The transport's failures are `HostdError` kinds too, with `reply` null and the system's code, if any, in `word`:

- `connect`: the command or feedback port refused or unreachable;
- `closed`: the peer closed or reset the connection, before or during a reply;
- `timeout`: no reply in time; the connection is then closed, as a late reply would answer the next line;
- `oversize`: a reply past the table's `message_max`, 16 MiB (`SOCKET_MSG_MAX_SIZE`); the socket is closed and every later
  call answers the same error.

A reply the codec does not read as one the protocol declares is `kind: 'malformed'`. The one gap: a call the caller wrote
wrongly (an unknown verb, an argument count or type no form of the table takes, a control character in an argument)
throws the codec's `ProtocolError` before anything is sent. That is a fault of the caller's code, not of the connection or
the host, and it is not a `HostdError`.

## How a line is written

By the codec, `Protocol.encode`: the first form of the verb whose arguments fit, an `int` a JavaScript integer, a `float` a
finite number, any other type one word. A word is bare when it holds no whitespace or double quote; otherwise it is quoted,
a quote written `\"` and a backslash `\\`, which is how the daemon's tokenizer (`src/utils.c`) reads a quoted word; so a
`quoted-escaped` argument is the value, and the codec writes its quotes. A control character in any argument is refused by
`call`, whatever its type.

## What it does not do

- **No tags, no `hello`.** A client sends one line and waits for its reply, in order; it asks for no extension. A caller
  that wants tags writes the `hello` line with `call` and frames its own.
- **No reconnect, no retry, no cache, no pool.** A connection that closed is a connection that closed.
- **No second encoding of a verb.** A line the table does not declare is not written; `send(line)` is the one way
  to send a line it does not know, and it is the caller's.
- **No maths.** The curve is plugin-hostd's, drawn by omx-dsp's C; the client parses what it answers.
- **No compatibility layer** with another spelling of a verb or an older reply.

The transport follows [client-library.md](client-library.md): every line ends in one NUL and is split on the NUL only; the
command port is connected first and then the feedback port (the command port plus one), and nothing is sent before both
are open; `connect({ feedbackPort: null })` opens none, for a host started with `-n`. Feedback lines are read and passed to
`onFeedback`, if one is given, so that the host's writes do not back up.

## Versioned with the protocol

The client is files of this package and carries no version of its own: `package.json`'s version is `VERSION`
(`make version-check`), the protocol's. The client reads the table it is given, so a table change that adds, drops or
retypes a verb is spoken by the client with no change to it.

## Held, and how

- `make test-ts-client`: `tests/one_ts_encoder_test.mjs` holds one TypeScript encoder (no `./lines` or `./vocabulary`
  export, no file of theirs, no generator, a client that imports the codec and writes no line itself), each rule red on
  its fault planted back; then, the codec built, the transport against a scripted peer that answers the way the daemon
  does, in the replies of `tests/golden/plugin-hostd-protocol.txt`; a verb added to the table the client is given is
  sent, and one the table does not declare is refused before anything is sent; the port and the cap are the table's;
  `curvePoints` sends the words the table declares and reads the reply; each error is typed.
- `make test-ts-codec` runs the codec on `clients/fixtures/codec.json`, the one file the Python codec is tested on too; it
  is shipped in the npm package as `@openmixer/plugin-hostd-protocol/codec-vectors`, for a client elsewhere to run the
  same cases.
- `make test-ts-package` packs the package, imports the client and the codec from the tarball, and finds no `./lines` and
  no `./vocabulary`.

## The table-driven codec (the package's `.` export)

`@openmixer/plugin-hostd-protocol` also exports the codec plugin-hostd carried as its own TypeScript client
(`clients/typescript/codec`, moved here: this repository owns the protocol and its codecs, issue 46). `Protocol` reads the
installed table (`PLUGIN_HOSTD_PROTOCOL_TABLE`, else pkg-config's `table`), the vocabulary of omx-contract's contract
directory and the catalogs, and encodes, decodes and parses every command and feedback line, reads the typed replies (plan,
plan_get, estimate, worker_list, instance_info, hello...) and gives each reply code's sentence in a language. It has no
transport; the `./client` export above opens the socket. `plugin-hostd-codec` (`dist/cli.js`) answers a batch of cases from
stdin, as `python3 -m plugin_hostd_protocol` does.

It runs `clients/fixtures/codec.json`, the one set of vectors the Python codec runs on (`make test-ts-codec`), so the two
codecs are held to the same expected lines. `GroupingReason` and `RETRY_REASONS` are written by
`tools/grouping-reasons.py` from plugin-hostd's domain `plugin-hostd/grouping`, the one generator of both codecs, and the tests
hold them to the 16 golden reasons of `clients/fixtures/grouping-reasons.txt`. The library reads no environment variable and runs no
program: it takes options (`table`, `contractDirs`, `datadir`) and its installed default (`INSTALLED`); the command line
and the tests take theirs from `PLUGIN_HOSTD_PROTOCOL_TABLE`, `OMX_CONTRACT_PATH` and pkg-config (`src/environment.ts`).
The `./client` transport writes and reads its lines through it: the package has one TypeScript encoder (#106).
