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
| the generated vocabulary | `clients/typescript/vocabulary.mjs`, `vocabulary.d.mts` | every verb with its forms (argument types, names, tail), its refusal codes and its reply; the argument types; the reply codes and their words; the parse refusals. Written by `tools/ts-client-gen.py` from `protocol/plugin-hostd-protocol.json`, the table `make gen` writes from the headers; never typed by hand |
| one call | `curvePoints(instance, display, n)` | sends `curve_points <instance> <display> <n>` and answers the points, `[{ x, y }]`, which plugin-hostd drew with omx-dsp's `omx_curve_points` |

The words of a line come from the vocabulary: `call(verb, ...args)` finds the verb's form by the number of arguments,
refuses an argument that is not of its declared type, and writes the line; `curvePoints` is `call('curve_points', ...)`
and a parse of the count and the points. The line is encoded and the reply parsed by `clients/typescript/lines.mjs`, which opens
no socket, so a surface without TCP imports the vocabulary, the encoder and the parsers from the package alone
(`@openmixer/plugin-hostd-protocol/lines`).

## Errors are typed

A reply that is not a success is a `HostdError`, never a string to compare. It has `kind`, `reply` (the line), `code` and
`word`:

- `kind: 'refused'`, a `resp <negative>`: `code` is that number and `word` the stable word the table gives it
  (`reply.host.instance-non-exists` for -3), or the word the host sent in a session granted `errors`;
- `kind: 'protocol'`, a parse refusal (`not found`, `few arguments`, ...): `word` is that text and `code` the table's
  number for it.

A peer that closes before the reply, a reply that does not come in time (the connection is then closed: a late reply
would answer the next line) and a line past 16 MiB are errors too.

## What it does not do

- **No tags, no `hello`.** A client sends one line and waits for its reply, in order; it asks for no extension. A caller
  that wants tags writes the `hello` line with `call` and frames its own.
- **No reconnect, no retry, no cache, no pool.** A connection that closed is a connection that closed.
- **No second encoding of a verb.** A line the vocabulary does not declare is not written; `send(line)` is the one way
  to send a line it does not know, and it is the caller's.
- **No maths.** The curve is plugin-hostd's, drawn by omx-dsp's C; the client parses what it answers.
- **No compatibility layer** with another spelling of a verb or an older reply.

The transport follows [client-library.md](client-library.md): every line ends in one NUL and is split on the NUL only; the
command port is connected first and then the feedback port (the command port plus one), and nothing is sent before both
are open; `connect({ feedbackPort: null })` opens none, for a host started with `-n`. Feedback lines are read and passed to
`onFeedback`, if one is given, so that the host's writes do not back up.

## Versioned with the protocol

The client is files of this package and carries no version of its own: `package.json`'s version is `VERSION`
(`make version-check`), the protocol's. A table change that adds, drops or retypes a verb changes the vocabulary in the
same commit, and a package version is the vocabulary of that protocol version.

## Held, and how

- `make test-ts-client` (Node 20 or later, no dependency): the transport against a scripted peer that answers the way the
  daemon does, in the replies of `tests/golden/plugin-hostd-protocol.txt`; the vocabulary written again from a table with
  one verb more shows it; `curvePoints` sends the words the table declares and parses the reply; each error is typed.
- `make check-generated` holds `clients/typescript/vocabulary.mjs` and `vocabulary.d.mts` to what `tools/ts-client-gen.py` writes,
  byte for byte; `make gen` writes them.
- `make test-ts-package` packs the package and imports the client from the tarball.
