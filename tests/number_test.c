/* SPDX-License-Identifier: GPL-3.0-or-later */
/* The number grammar of protocol.h's phd_number_in (#73), one arm a case, each named by its spelling: a value has one
 * spelling (decimal, no leading zero, no -0, no +), the range is the caller's lo and hi and nothing else, and a word
 * past what the type holds is refused. Built with the sanitizers, so an overflow in the reading stops the test. */

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "plugin-hostd/protocol.h"

#define NOTE_INSTANCE_MAX (1 << 20) /* the hi phd_note_valid gives its instance word */

static const struct
{
    const char *name, *word;
    int lo, hi, want;
} g_arms[] = {
    /* one spelling a value */
    { "0 is zero", "0", 0, 127, 1 },
    { "00 is refused: a leading zero", "00", 0, 127, 0 },
    { "007 is refused: leading zeros", "007", 0, 127, 0 },
    { "07 is refused: a leading zero", "07", 0, 127, 0 },
    { "-0 is refused: zero has no sign", "-0", -127, 127, 0 },
    { "-00 is refused: zero has no sign", "-00", -127, 127, 0 },
    { "-07 is refused: a leading zero after the minus", "-07", -127, 127, 0 },
    { "+1 is refused: no plus sign", "+1", 0, 127, 0 },
    { "+0 is refused: no plus sign", "+0", 0, 127, 0 },
    { "the empty word is refused", "", 0, 127, 0 },
    { "- alone is refused", "-", -127, 127, 0 },
    { "--1 is refused", "--1", -127, 127, 0 },
    { "1x is refused: not a digit", "1x", 0, 127, 0 },
    { " 1 is refused: a space", " 1", 0, 127, 0 },
    { "10 is ten: a zero after a digit", "10", 0, 127, 1 },
    { "100 is a hundred", "100", 0, 127, 1 },
    { "-7 is minus seven", "-7", -127, 127, 1 },
    { "-10 is minus ten", "-10", -127, 127, 1 },
    /* the range is lo and hi, no other limit */
    { "1000009 is in 0 to 1 << 20", "1000009", 0, NOTE_INSTANCE_MAX, 1 },
    { "1000010 is in 0 to 1 << 20", "1000010", 0, NOTE_INSTANCE_MAX, 1 },
    { "1048575 is in 0 to 1 << 20", "1048575", 0, NOTE_INSTANCE_MAX, 1 },
    { "1048576, hi itself, is in 0 to 1 << 20", "1048576", 0, NOTE_INSTANCE_MAX, 1 },
    { "1048577, hi + 1, is refused of 0 to 1 << 20", "1048577", 0, NOTE_INSTANCE_MAX, 0 },
    { "128 is refused of 0 to 127", "128", 0, 127, 0 },
    { "0 is refused of 1 to 127", "0", 1, 127, 0 },
    { "-1 is refused of 0 to 127", "-1", 0, 127, 0 },
    { "-128 is refused of -127 to 127", "-128", -127, 127, 0 },
    { "2147483647, INT_MAX, is in 0 to INT_MAX", "2147483647", 0, INT_MAX, 1 },
    { "-2147483648, INT_MIN, is in INT_MIN to 0", "-2147483648", INT_MIN, 0, 1 },
    /* past the range, and past the type that reads it */
    { "2147483648, INT_MAX + 1, is refused of 0 to INT_MAX", "2147483648", 0, INT_MAX, 0 },
    { "-2147483649, INT_MIN - 1, is refused of INT_MIN to 0", "-2147483649", INT_MIN, 0, 0 },
    { "9223372036854775808, past 63 bits, is refused: overflow", "9223372036854775808", INT_MIN, INT_MAX, 0 },
    { "18446744073709551617, 2^64 + 1, is refused: overflow, not 1", "18446744073709551617", 0, 127, 0 },
    { "-18446744073709551617 is refused: overflow, not -1", "-18446744073709551617", -127, 127, 0 },
    { "forty nines are refused: overflow", "9999999999999999999999999999999999999999", INT_MIN, INT_MAX, 0 },
};

int main(void)
{
    unsigned n, failures = 0;

    /* each line is out before a sanitizer stops the test */
    setvbuf(stdout, NULL, _IOLBF, 0);
    for (n = 0; n < sizeof(g_arms) / sizeof(g_arms[0]); n++)
    {
        int got;

        got = phd_number_in(g_arms[n].word, g_arms[n].lo, g_arms[n].hi) != 0;
        if (got != g_arms[n].want)
            failures++;
        printf("%s %s%s\n", got == g_arms[n].want ? "ok" : "FAIL", g_arms[n].name,
               got == g_arms[n].want ? "" : got ? " (it was accepted)" : " (it was refused)");
    }
    printf("%s\n", failures ? "number test FAILED" : "number test ok");
    return failures ? 1 : 0;
}
