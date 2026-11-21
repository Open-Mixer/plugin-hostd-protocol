/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> */
/* pin_words_dump: every row of PHD_PIN_VERDICTS and PHD_PIN_POOLS of protocol.h, one line each,
 * "<list> <index> <id> <word> <meaning>", in the header's order, then the three verdict words and the two pool words by their
 * own names, "word <NAME> <value>" (tests/pin_words_test.py compares it with tests/golden/pin-words.txt, frozen from the
 * words plugin-db's data/pin.json wrote). */
#include <plugin-hostd/protocol.h>

#include <stdio.h>

int main(void)
{
    int i;

    i = 0;
#define X(id, word, meaning) printf("verdicts %d %s %s %s\n", i++, #id, word, meaning);
    PHD_PIN_VERDICTS(X)
#undef X
    i = 0;
#define X(id, word, meaning) printf("pools %d %s %s %s\n", i++, #id, word, meaning);
    PHD_PIN_POOLS(X)
#undef X
    printf("word PHD_PIN_VERDICT_IN_PROCESS %s\n", PHD_PIN_VERDICT_IN_PROCESS);
    printf("word PHD_PIN_VERDICT_ISOLATED %s\n", PHD_PIN_VERDICT_ISOLATED);
    printf("word PHD_PIN_VERDICT_REFUSED %s\n", PHD_PIN_VERDICT_REFUSED);
    printf("word PHD_PIN_POOL_CORES %s\n", PHD_PIN_POOL_CORES);
    printf("word PHD_PIN_POOL_OFF %s\n", PHD_PIN_POOL_OFF);
    return 0;
}
