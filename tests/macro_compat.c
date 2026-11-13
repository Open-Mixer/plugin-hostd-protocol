/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * tests/macro_compat.c: a consumer of protocol.h's X-macro tables, each expanded with the arguments its consumers
 * expand it with (plugin-hostd's tools/protocol-gen.c, omx-clap-core, and from 0.5.6 the typed-argument tables). A
 * release may add rows, and new tables, but a table whose X takes one argument more or less breaks every consumer's
 * build within the same major: this file stops compiling. make test builds it. */
#include <stdio.h>

#include "plugin-hostd/protocol.h"

int main(void)
{
    unsigned n = 0;

#define X(name, fields, meaning) n += (unsigned)sizeof(name fields meaning);
    PHD_EVENTS(X)
#undef X
#define X(name, value, unit, meaning) n += (unsigned)(value) + (unsigned)sizeof(unit meaning);
    PHD_CONSTANTS(X)
#undef X
#define X(id, code, meaning) n += (unsigned)(code) + (unsigned)sizeof(meaning);
    PHD_ERRORS(X)
#undef X
#define X(id, name, args, reply, ledger, meaning) n += (unsigned)PHD_LEDGER_##ledger + (unsigned)sizeof(args reply meaning);
    PHD_INSTANCE_VERBS(X)
#undef X
#define X(name, source, lv2, clap, console, zynthian) n += (unsigned)sizeof(name);
    PHD_HOST_VERBS(X)
#undef X
#define X(id, name, args, meaning) n += (unsigned)sizeof(name args meaning PHD_NODE_METADATA_##id##_KEY);
    PHD_NODE_METADATA(X)
#undef X
#define X(id, name, pattern, rest, spelling) n += (unsigned)(rest) + (unsigned)sizeof(name pattern spelling);
    PHD_ARG_TYPES(X)
#undef X
#define X(name, args) n += (unsigned)sizeof(name args);
    PHD_EVENT_ARGS(X)
#undef X
#define X(verb, args) n += (unsigned)sizeof(verb args);
    PHD_COMMAND_FORMS(X)
#undef X
#define X(verb, resp, args) n += (unsigned)sizeof(verb resp args);
    PHD_REPLY_ARGS(X)
#undef X
#define X(name, item, separator, none, spelling) n += (unsigned)sizeof(name item separator none spelling);
    PHD_LIST_TYPES(X)
#undef X
#define X(name, separator, fields, spelling) n += (unsigned)sizeof(name separator fields spelling);
    PHD_RECORD_TYPES(X)
#undef X
#define X(name, example) n += (unsigned)sizeof(name example);
    PHD_ARG_EXAMPLES(X)
#undef X
    printf("%u\n", n);
    return 0;
}
