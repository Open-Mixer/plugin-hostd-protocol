# The reply shapes: a JSON Schema per reply document, and its TypeScript type

A reply is a line, `resp <code> <words>`, and a client turns the words into a document: a worker's record, an instance's
cost, a curve's points. Each client wrote that document's type for itself. The table types 21 of the replies
(`reply_forms` of `protocol/plugin-hostd-protocol.json`: the name and the type of each word, see
[protocol.md](protocol.md)), and each is now also a JSON Schema, the one source of the document's type in every language
(the shapes facility of Open-Mixer/omx-contract, `docs/shapes.md`, and the typed-contracts spec, section 3).

Three replies with words have no form yet (#85): `param_info`, whose bounds and default may be `NaN`, `Infinity` or
`-Infinity`, which a JSON number cannot hold; `audio_ports`, a leading `dry` that may be absent and a port record with
optional fields; and `idle_hold`, whose target and held are `off` or microseconds. Each needs a type the table does not
have.

## What is where

| Path | What |
|---|---|
| `shapes/reply_<verb>.schema.json` | the schema of the parsed reply of `<verb>`: JSON Schema 2020-12, written by hand, one per document; installed under `pkg-config --variable=shapesdir plugin-hostd-protocol` |
| `ts/shapes/reply_<verb>.d.ts` | the TypeScript render, `omx-contract render --target ts --shapes --shapes-dir shapes --out ts`, committed |
| `package.json` | the npm package `@openmixer/plugin-hostd-protocol`: `import type { InstanceInfoReply } from '@openmixer/plugin-hostd-protocol/shapes/reply_instance_info'`, and the schemas as `@openmixer/plugin-hostd-protocol/schemas/reply_instance_info` |

The title of each is the verb in capitals and `Reply` (`InstanceInfoReply`); a record inside it keeps the table's name
(`WorkerRecord`, `Point`, `Layout`, `Reason`, `MemberCode`, `HistoryEntry`). Titles are unique across the directory,
because the shapes facility refuses two that spell one macro stem.

## The parsed form

One property per word of the reply, in the order of the longest reply form, and the keys of the shortest are required
(`estimate` has six words and, from a daemon with the memory budget, ten; `plan` and `plan_get` have their `cores` from a
daemon with them). A list is an array and a record an object whose keys are the table's names; `uint` and `int` are
integers, `number` a number; a word in double quotes is unquoted (`\"` and `\\` undone). Where a word can be `-`
(`amount`, `core`, `worker-or-none`) it stays a string, since a number does not say none. A list's first none word is
the empty array and a further one stays its word: `history_get` answers `[]` for `-` (no entry) and `"unavailable"` while
the history file is blocked, so a client tells "no history yet" from "history off" (#86). **RULED (2026-11-23, #86):** for a
list type, the first none word decodes to the empty array and any further one decodes to itself, so `history_get`'s
document is `history: HistoryEntry[] | "unavailable"`; the wire and the table are unchanged. Held by the `read_reply`
vectors of `clients/fixtures/codec.json` (both codecs), by `tests/reply_shapes.py` on `reply_history_get.schema.json`
and its planted fault (a none word the table does not have), and by the rendered `ts/shapes/reply_history_get.d.ts`. A count that only counts the
records (`worker_list`) is not a property: the array's length is it. `preset_get`'s `name` is the rest of the line, as the
host wrote it (the table's `line` type): it may hold spaces and may be empty. `preset_list` answers only a count and has no
document.

`tests/reply_shapes.py` builds that form from the reply with `tests/reply_args.py`'s decoder and holds it to the schema.

## Held, and how

- `make test-reply-shapes` (python3-jsonschema): one schema per typed reply and no other; the properties, the required
  keys and every record's keys are the table's names; the state words are the table's `worker_states`; one sample of each
  reply form and every success reply of the golden transcript for these verbs validate; planted faults are refused.
- `make check-shapes-render OMX_CONTRACT_DIR=<checkout>`: `ts/shapes/` is what omx-contract renders from `shapes/`, byte for
  byte, with the `json-schema-to-typescript` that checkout pins. `make render-shapes` writes it. `OMX_CONTRACT_DIR` is a
  checkout of Open-Mixer/omx-contract with `npm ci` run in it: the generator is its dev dependency, never an installed
  command, and CI pins the checkout to a commit.
- `make test-shapes-sabotage`: each guard above broken in a scratch copy (a schema property dropped, a state word the table
  does not declare, a schema left open, a pid typed as a word, a record key renamed, the table renaming a reply word, a
  render edited by hand, a schema changed and not rendered again, a schema deleted, an extra render) goes red.
- `make test-ts-package`: `npm pack`, the tarball installed as a dependency, a strict TypeScript file importing the types by
  the package's exports; it compiles, and a pid that is a string is refused.
- `make version-check` holds `package.json`'s version to `VERSION`, as it holds the spec and the changelog.

Only the TypeScript render is committed: Python and C readers use the table (`pkg-config --variable=table`) and the codec
of `plugin_hostd_protocol`. A consumer that wants the TypedDicts or the X-macros renders them with the same command and
`--target py` or `--target c`.

## What the console deletes

The console's declarations package reads this protocol through two consumer-side generators:
`plugin-hostd-protocol-gen.mjs` (537 lines, reading the C macro tables of `plugin-hostd/protocol.h`) and
`mod-host-protocol-gen.mjs` (116 lines, reading `mod-host.json`), and writes `plugin-hostd-protocol.generated.ts` and
`mod-host-protocol.generated.ts` with them. The reply types of this package replace the ones the console's engine writes by
hand for the parsed replies (`WorkerRecord`, `InstanceInfo`, the words of `ModHostResponse`). The two generators also write the verb and constant
*values* (templates, codes, limits); those are the data of `plugin-hostd.json` and `mod-host.json`, whose own schema
(`protocol/plugin-hostd.schema.json`) predates the shapes rules (no titles or descriptions, open objects) and is not
yet rendered: that render is the step after this one, and until it lands the generators stay for the values.
