/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> */
/* probe_words_dump: every row of PHD_PROBE_REASONS, PHD_PROBE_OUTCOMES and PHD_PROBE_STEPS of protocol.h, one line each,
 * "<list> <index> <id> <word> <meaning>", in the header's order (tests/probe_words_test.py compares it with
 * tests/golden/probe-words.txt, frozen from the hand-written header). */
#include <plugin-hostd/protocol.h>

#include <stdio.h>

int main(void)
{
    int i;

    i = 0;
#define X(id, word, meaning) printf("reasons %d %s %s %s\n", i++, #id, word, meaning);
    PHD_PROBE_REASONS(X)
#undef X
    i = 0;
#define X(id, word, meaning) printf("outcomes %d %s %s %s\n", i++, #id, word, meaning);
    PHD_PROBE_OUTCOMES(X)
#undef X
    i = 0;
#define X(id, word, meaning) printf("steps %d %s %s %s\n", i++, #id, word, meaning);
    PHD_PROBE_STEPS(X)
#undef X
    return 0;
}
