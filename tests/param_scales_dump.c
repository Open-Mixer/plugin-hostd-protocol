/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> */
/* param_scales_dump: every row of PHD_PARAM_SCALES of protocol.h, one line each, "scales <index> <id> <word> <meaning>", in the
 * header's order (tests/param_scales_test.py compares it with tests/golden/param-scales.txt, frozen from the hand-written
 * header at ce565fa). */
#include <plugin-hostd/protocol.h>

#include <stdio.h>

int main(void)
{
    int i = 0;

#define X(id, word, meaning) printf("scales %d %s %s %s\n", i++, #id, word, meaning);
    PHD_PARAM_SCALES(X)
#undef X
    return 0;
}
