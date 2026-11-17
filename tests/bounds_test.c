/* SPDX-License-Identifier: GPL-3.0-or-later */
/* The bounds of protocol.h's inline functions, under AddressSanitizer: a function given count words reads none past
 * them, and one given size bytes writes none past them (#71, #72). Each array is a heap block of exactly count words,
 * so a read past it stops the test; each buffer of size 0 is followed by a guard byte, so a write past it is seen. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "plugin-hostd/protocol.h"

#define GUARD 'G'

static int g_failures;

static void check(int ok, const char *what)
{
    if (!ok)
        g_failures++;
    printf("%s %s\n", ok ? "ok" : "FAIL", what);
}

/* a heap block of exactly count words: the word after the last is not the caller's to read. For count 0 it is the end
 * of a block of one word, since malloc(0) may return NULL */
static char **g_block;

static char **words_of(const char *const *from, int count)
{
    size_t n = count > 0 ? (size_t)count : 1;

    g_block = malloc(sizeof(char *) * n);
    if (!g_block)
        abort();
    if (count == 0)
        return g_block + 1;
    memcpy(g_block, from, sizeof(char *) * n);
    return g_block;
}

/* #72: a buffer of size 0 takes no byte, not even the NUL of an empty path */
static void pin_path_size_zero(void)
{
    char out[2] = { GUARD, GUARD };
    int r;

    r = phd_pin_path_decode("", out, 0);
    check(r == -1, "phd_pin_path_decode of \"\" into size 0 is refused");
    check(out[0] == GUARD, "phd_pin_path_decode of \"\" into size 0 writes no byte");
    out[0] = GUARD;
    r = phd_pin_path_decode("a", out, 0);
    check(r == -1 && out[0] == GUARD, "phd_pin_path_decode of a path into size 0 is refused, no byte written");
    r = phd_pin_path_decode("", out, 1);
    check(r == 0 && out[0] == '\0' && out[1] == GUARD, "phd_pin_path_decode of \"\" into size 1 writes its NUL only");
    out[0] = GUARD;
    r = phd_pin_path_encode("", out, 0);
    check(r == -1 && out[0] == GUARD, "phd_pin_path_encode of \"\" into size 0 is refused, no byte written");
}

/* #71: no word is read before count covers it */
static void note_words(void)
{
    static const char *const on[] = { OMX_VERB_NOTE_ON, "3", "0", "60", "100" };
    static const char *const all_off[] = { OMX_VERB_ALL_NOTES_OFF, "3" };
    char **words;
    int count;

    words = words_of(on, 5);
    check(phd_note_valid(words, 5) == 1, "phd_note_valid takes a note_on line");
    free(g_block);
    words = words_of(all_off, 2);
    check(phd_note_valid(words, 2) == 1, "phd_note_valid takes an all_notes_off line");
    free(g_block);
    for (count = 4; count >= 0; count--)
    {
        char what[96];

        snprintf(what, sizeof(what), "phd_note_valid refuses the first %d words of a note_on line", count);
        words = words_of(on, count);
        check(phd_note_valid(words, count) == 0, what);
        free(g_block);
    }
}

/* the same two patterns in the functions beside them: each is held to its count and its size here */
static void neighbours(void)
{
    static const char *const info[] = { "track_info", "3", "Kick", "-", "bus" };
    static const char *const add[] = { "add", "clap:/x/Surge", "XT.clap#id" };
    char joined[2] = { GUARD, GUARD };
    char **words;
    unsigned next;
    int count;

    for (count = 5; count >= 0; count--)
    {
        char what[96];

        snprintf(what, sizeof(what), "phd_track_info_valid of the first %d words is %d", count, count >= 4);
        words = words_of(info, count);
        check(phd_track_info_valid(words, count) == (count >= 4), what);
        free(g_block);
    }
    for (count = 3; count >= 0; count--)
    {
        char what[96];
        const char *uri;

        snprintf(what, sizeof(what), "phd_add_uri of the first %d words joins into no size 0", count);
        words = words_of(add, count);
        uri = phd_add_uri(words, (unsigned)count, joined, 0, &next);
        check(uri == (count < 2 ? NULL : words[1]) && next == 2 && joined[0] == GUARD, what);
        free(g_block);
    }
}

int main(void)
{
    /* each line is out before a sanitizer stops the test */
    setvbuf(stdout, NULL, _IOLBF, 0);
    pin_path_size_zero();
    note_words();
    neighbours();
    printf("%s\n", g_failures ? "bounds test FAILED" : "bounds test ok");
    return g_failures ? 1 : 0;
}
