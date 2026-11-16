/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * This file is part of plugin-hostd-protocol.
 *
 * phd_number_in (include/plugin-hostd/protocol.h) holds the bound its caller declares: a note verb's frame 1<<20 is
 * taken, 1<<20 + 1 is refused, whatever its digit count (the door once refused everything above 1,000,009 while
 * declaring 1<<20), and a word that is no number, an overlong one included, is refused without overflowing.
 *
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 */
#include <stdio.h>
#include <plugin-hostd/protocol.h>

static int g_failures;

static void expect(const char *word, int lo, int hi, int want)
{
    int got = phd_number_in(word, lo, hi);

    if (got != want)
    {
        printf("FAIL phd_number_in(\"%s\", %d, %d) is %d, want %d\n", word, lo, hi, got, want);
        g_failures++;
    }
}

int main(void)
{
    expect("1048576", 0, 1 << 20, 1);              /* the declared limit itself */
    expect("1048577", 0, 1 << 20, 0);              /* one past it */
    expect("1000009", 0, 1 << 20, 1);
    expect("1000010", 0, 1 << 20, 1);              /* above the old 1,000,009 ceiling, inside the declared limit */
    expect("1048575", 0, 1 << 20, 1);
    expect("-1048576", -(1 << 20), 1 << 20, 1);
    expect("-1048577", -(1 << 20), 1 << 20, 0);
    expect("99999999999999999999999", 0, 1 << 20, 0); /* overlong: refused, no overflow */
    expect("00000000000000000000000001", 0, 1 << 20, 1);
    expect("-0", -5, 5, 0);
    expect("", 0, 5, 0);
    expect("1x", 0, 5, 0);
    if (g_failures)
        return 1;
    printf("ok   phd_number_in holds the declared bound\n");
    return 0;
}
