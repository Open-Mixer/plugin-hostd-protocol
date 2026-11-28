# plugin-hostd-protocol

The plugin-hostd protocol, a superset of mod-host's text protocol: any mod-host client works unchanged.

`libplugin-hostd-protocol` is the socket server, the line protocol and the command dispatch that plugin-hostd and
omx-clap-host answer their controllers with. A host registers a backend (`host_backend_t`) and the library does the
rest: it listens on the command and feedback ports, splits the input into NUL-terminated commands, parses them against
the command table and answers `resp <status> [value]`, as mod-host does. It also holds the whole declared protocol:
the verbs plugin-hostd and omx-clap-host add, their replies and refusals, the events, and the one verb table and the
grammar written from them.

## Install

Fedora 45 (x86_64, aarch64):

```
sudo dnf install https://www.openmixer.org/rpm/openmixer-release.noarch.rpm
sudo dnf install plugin-hostd-protocol-devel
```

Debian trixie (amd64, arm64), also Raspberry Pi OS and Zynthian:

```
curl -fsSLO https://www.openmixer.org/deb/openmixer-release.deb && sudo apt install ./openmixer-release.deb
sudo apt install libplugin-hostd-protocol-dev
```

## Start

```
pkg-config --cflags --libs plugin-hostd-protocol
```

For Node, `nodejs-plugin-hostd-protocol` (rpm and deb) installs the npm package `@openmixer/plugin-hostd-protocol`
as `npm pack` packs it, under `/usr/lib/node_modules/@openmixer/plugin-hostd-protocol`: the table-driven TypeScript
codec (`Protocol`), the thin client over it (`@openmixer/plugin-hostd-protocol/client`: the transport and
`curvePoints`), the codec vectors, and the reply shapes as JSON Schema and TypeScript types. The codec reads the table
and the domains the other packages install, never a copy:

```
sudo dnf install nodejs-plugin-hostd-protocol     # Fedora
sudo apt install nodejs-plugin-hostd-protocol     # Debian
```

The headers keep the names and the API of `libmod-host-protocol` (`protocol.h`, `socket.h`, `host-dispatch.h`, ...),
so a program built against `mod-host-protocol` changes only the package name it asks pkg-config for.

## Documentation

<https://www.openmixer.org/projects/plugin-hostd-protocol>, and in this repository: the declared protocol, the build
and its tests, what differs from mod-host and the credit ([doc/overview.md](doc/overview.md)); the commands and the
error codes ([doc/protocol.md](doc/protocol.md)).

## Licence

GPL-3.0-or-later, as mod-host: [COPYING](COPYING). The library's code is mod-host's, by Filipe Coelho, MOD Audio UG
and the mod-host contributors.
