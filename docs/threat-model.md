<!-- SPDX-License-Identifier: GPL-3.0-or-later
     Copyright (C) 2026 Pau Aliagas -->

# Threat model

## What it is and where it runs

`libplugin-hostd-protocol` is a library, not a daemon. A host (plugin-hostd, omx-clap-host, or any mod-host style
host) links it, registers a backend (`include/host-backend.h`) and calls `socket_start()` or `socket_adopt()` and then
`socket_run()` (`src/socket.c`). The library then owns the listening sockets, the framing, the parser
(`src/protocol.c`) and the command dispatch (`src/host-dispatch.c`, `src/host-curve.c`); everything a command does
past parsing is the backend's.

- Sockets: two TCP/IPv4 listeners, a command port and an optional feedback port (`socket_start()`). No Unix socket.
- Bind address: loopback by default (`g_bind_address = INADDR_LOOPBACK` in `src/socket.c`). A host can widen it with
  `socket_set_bind_address()`, which takes any dotted IPv4 address including `0.0.0.0`. With `socket_adopt()` the
  caller binds the sockets itself and the library only checks that each fd is a bound IPv4 stream socket.
- One controller at a time: `socket_run()` accepts one command connection, then one feedback connection, serves it
  until it closes, and returns.
- Not real-time code. The socket loop and the command callbacks run on the host's socket thread and call `malloc`,
  `realloc` and blocking `send`. `socket_send_feedback()` may allocate. Nothing in the library is marked safe to call
  from an audio callback.
- `src/host-scenario.c` (`host_scenario_load()`, `host_scenario_run()`) is a test runner shipped in the library: it
  reads a scenario file and drives a host through the protocol.

## Inputs

| Input | Read by | Who can write it | Trusted |
| --- | --- | --- | --- |
| Command bytes on the command port (NUL-framed lines) | `socket_run()`, `protocol_parse()` | any process that can reach the port: local users on loopback; the LAN when bound beyond loopback | untrusted |
| Words inside a command: instance and page numbers, values, symbols | `host-dispatch.c` callbacks (`atoi`, `atof`, `strtod`) | as above | untrusted |
| URIs and paths inside a command: `add` uri (`phd_add_uri()`), `preset_load` uri, `state_load`/`state_save` directory, `connect`/`disconnect` port names | passed to the backend | as above | untrusted |
| Names and colors in `track_info` | `valid_name()`, `valid_color()` | as above | untrusted |
| `curve_bind` terms, `curve_points` counts | `parse_term()`, `curve_points_cb()` in `src/host-curve.c` | as above | untrusted |
| `hello` version and extension words, `@<tag>` request tags | `hello_cb()`, `protocol_parse()` | as above | untrusted |
| Connections on the feedback port | `socket_run()` (accepted, never read) | as above | untrusted |
| Values the backend returns (param values, page names, param info strings) | formatted into replies | the host and the plugins it loads (a plugin author) | trusted by the library |
| Bind address and ports | `socket_set_bind_address()`, `socket_start()` | the host program | trusted |
| Scenario file and its `$NAME` values | `host_scenario_load()` | whoever runs the test runner | trusted (test input) |
| Replies read by the scenario runner | `read_reply()` in `src/host-scenario.c` | the host under test | trusted (test input) |

## Trust boundary

The boundary is the command port. Controllers are not authenticated (the comment on `socket_set_bind_address()` in
`include/socket.h` says so), so anything that can open a TCP connection to the port can add and remove plugins,
set parameters and make the backend load or save state at a path it names. On the default loopback bind that is
every local user and process; once a host binds `0.0.0.0` or a LAN address, it is every host on that network. The
backend and the plugins it loads are inside the boundary: the library formats what they return without checks.

## What the code assumes and enforces

- Command length: at most `SOCKET_MSG_MAX_SIZE` (16 MiB, `include/socket.h`) including the NUL. A longer command,
  finished or not, closes the connection (`socket_run()`). The receive buffer doubles as needed and stays under twice
  that limit.
- An unterminated last command is still run when the client disconnects (`socket_run()`), as mod-host does.
- `SIGPIPE`: replies use `MSG_NOSIGNAL` (`socket_send()`), so a client that goes away does not kill the host.
- Command table: at most `PROTOCOL_MAX_COMMANDS` (128) registered forms (`protocol_add_command()`).
- Argument counts: `protocol_parse()` answers `few arguments`, `many arguments` or `not found` against the form's
  word count; `...` forms take any number of trailing words.
- Argument types: `%i` and `%f` words are checked with `strtol`/`strtod` (`typed()`), but only to choose between
  forms of one verb. When no form matches by type, the first form that matches by count still runs, as in mod-host,
  so a non-numeric instance reaches `atoi()` and becomes 0.
- Numbers: instances, pages and `bypass` values go through `atoi()` and values through `atof()` with no range check;
  `nan` and `inf` are accepted as `param_set` values. `param_set_many` refuses a value that `strtod` does not consume
  whole and sets all pairs or none (`param_set_many_cb()`). `hello` refuses a version that is not a whole number of 1
  or more with -902. `curve_points` refuses fewer than 2 or more than `OMX_CURVE_POINTS_MAX` points (-539).
- Strings: `track_info` names must be valid UTF-8 (RFC 3629), under 256 bytes, with no C0/C1 control characters;
  colors are `#RRGGBB` or `-`; kinds are `bus`, `return` or `master` (-902 otherwise). Curve symbols are capped at
  `CURVE_SYMBOL_MAX`, shapes and slopes at `CURVE_SHAPES_MAX`, terms at `CURVE_TERMS_MAX` (`src/host-curve.c`).
- Request tags: only `[A-Za-z0-9._-]` (`tag_char()`), and only in a session `hello` granted `tags`. What `hello`
  grants is reset for each new connection (`session_start()`).
- Monitors: `monitor_output_off` of an output this connection did not monitor is answered -543 without calling the
  backend; a connection's monitors end with it.
- Paths and URIs: none. The `add` uri, preset uri, state directory and port names are handed to the backend as
  received; any path check is the backend's. `phd_add_uri()` only bounds the joined `clap:` uri to its buffer.
- Refusals: the codes are in `include/host-errors.h` and the protocol's own in `include/plugin-hostd/protocol.h`;
  a missing backend entry answers -902 (`ERR_INVALID_OPERATION`).

## Fuzz coverage

Harnesses are libFuzzer with AddressSanitizer and UndefinedBehaviorSanitizer (`fuzz/README.md`); CI runs each for
60 s on every push to main and every pull request (`.github/workflows/fuzz.yml`), with mutant records, sabotage
patches and coverage floors (`fuzz/coverage-floor.txt`). `fuzz/socket_fuzz.c` replaces `src/socket.c`, so no
harness opens a socket.

| Untrusted input | Harness |
| --- | --- |
| Command line splitting and form matching (`strarr_split()`, `protocol_parse()`) | `fuzz_protocol`, `fuzz_dispatch`, `fuzz_session` |
| Backend verbs, their numbers, uris and paths, `param_get_many`/`param_set_many`, `track_info`, `remote_page_get`, `param_info`, `monitor_output` | `fuzz_dispatch` |
| `hello`, request tags, feedback `seq` numbering, a new connection | `fuzz_session` |
| Scenario file and `$NAME` substitution (trusted, fuzzed anyway) | `fuzz_scenario` |

Gaps:

- Framing in `socket_run()` is not fuzzed: the NUL split, the buffer growth and the 16 MiB limit are covered only by
  unit tests (`tests/framing-test.c`, `tests/socket-test.c`).
- `curve_bind` and `curve_points` (`src/host-curve.c`) are not fuzzed: no harness calls
  `host_dispatch_register_curve()`. They have unit tests only (`tests/curve_test.c`).
- `monitor_output_off` is not fuzzed: no harness calls `host_dispatch_register_monitor_output_off()`
  (`tests/monitor_test.c` only).
- No authentication or authorisation of controllers; not enforced anywhere in the library.
- No range check on `atoi()`/`atof()` results, and non-finite `param_set` values reach the backend.
- No cap on the number of words in a command; a 16 MiB line of short words makes `strarr_split()` allocate a pointer
  per word.
- One controller at a time and no timeouts: a client that holds the command port open, or connects to it without
  ever connecting the feedback port, keeps every other controller waiting.
- Path, uri and port-name validation is left entirely to the backend.
- The fuzz runs are 60 s per harness per CI run; there is no long-running or continuous fuzzing set up in this
  repository.

## Reporting

Report a vulnerability privately through GitHub: the repository's Security tab, "Report a vulnerability".
The policy is the organisation's `SECURITY.md`.
