/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> */
/* estimate_words_dump: every row of PHD_ESTIMATE_VERDICTS and PHD_ESTIMATE_REASONS of protocol.h, one line each,
 * "<list> <index> <id> <word> <meaning>", in the header's order, then each word by its own name, "word <NAME> <value>",
 * the two patterns, the arg types the reply is typed with, the reply as data/verbs.json declares it and its row of
 * PHD_REPLY_ARGS, then the CPU ceiling and its unit (tests/estimate_words_test.py compares it with tests/golden/estimate-words.txt). */
#include <plugin-hostd/protocol.h>

#include <stdio.h>
#include <string.h>

int main(void)
{
    int i;

    i = 0;
#define X(id, word, meaning) printf("verdicts %d %s %s %s\n", i++, #id, word, meaning);
    PHD_ESTIMATE_VERDICTS(X)
#undef X
    i = 0;
#define X(id, word, meaning) printf("reasons %d %s %s %s\n", i++, #id, word, meaning);
    PHD_ESTIMATE_REASONS(X)
#undef X
#define X(id, word, meaning) printf("word PHD_ESTIMATE_VERDICT_%s %s\n", #id, PHD_ESTIMATE_VERDICT_##id);
    PHD_ESTIMATE_VERDICTS(X)
#undef X
#define X(id, word, meaning) printf("word PHD_ESTIMATE_REASON_%s %s\n", #id, PHD_ESTIMATE_REASON_##id);
    PHD_ESTIMATE_REASONS(X)
#undef X
    printf("pattern verdict %s\n", PHD_ESTIMATE_VERDICT_PATTERN);
    printf("pattern reason %s\n", PHD_ESTIMATE_REASON_PATTERN);
#define X(id, name, pattern, rest, spelling) \
    if (!strncmp(name, "estimate-", 9)) \
        printf("type %s %s\n", name, pattern);
    PHD_ARG_TYPES(X)
#undef X
    printf("reply %s\n", OMX_VERB_ESTIMATE_REPLY);
#define X(verb, resp, args) \
    if (!strcmp(verb, OMX_VERB_ESTIMATE)) \
        printf("args %s %s\n", resp, args);
    PHD_REPLY_ARGS(X)
#undef X
    printf("ceiling %d %s\n", PHD_ESTIMATE_CPU_CEILING, PHD_ESTIMATE_CPU_CEILING_UNIT);
    return 0;
}
