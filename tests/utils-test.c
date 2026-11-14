/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Checks that strarr_split() stays inside the string it splits, whatever it holds.
 *
 * Each string is copied into an allocation of exactly its size, so AddressSanitizer reports any byte read or
 * written outside it. Build and run with "make test-utils".
 */

#include "../src/utils.c"

#include <stdio.h>

static int g_failures;

static void check(const char *input, const char *first)
{
    size_t size = strlen(input) + 1;
    char *str = malloc(size);
    char **list;

    memcpy(str, input, size);
    list = strarr_split(str);

    if (list != NULL && strcmp(list[0], first) == 0)
    {
        printf("ok      \"%s\"\n", input);
    }
    else
    {
        printf("FAILED  \"%s\": expected first token \"%s\", got \"%s\"\n", input, first, list ? list[0] : "(null)");
        g_failures++;
    }

    free(list);
    free(str);
}

/* every token of `input`, NULL-terminated in `want`: a quoted-escaped value (\" a quote, \\ a backslash) arrives as its
 * bytes, the delimiting quotes gone and nothing else of it touched (the property verbs' values, doc/protocol.md) */
static void check_all(const char *input, const char *const *want)
{
    const size_t size = strlen(input) + 1;
    char *str = malloc(size), **list;
    uint32_t i;
    int ok;

    memcpy(str, input, size);
    list = strarr_split(str);
    ok = list != NULL;
    for (i = 0; ok && want[i]; i++)
        ok = list[i] && strcmp(list[i], want[i]) == 0;
    ok = ok && list[i] == NULL;
    if (ok)
        printf("ok      '%s'\n", input);
    else
    {
        printf("FAILED  '%s': token %u is '%s', expected '%s'\n", input, i ? i - 1 : 0, list && list[i ? i - 1 : 0] ? list[i ? i - 1 : 0] : "(null)",
               want[i ? i - 1 : 0] ? want[i ? i - 1 : 0] : "(end)");
        g_failures++;
    }
    free(list);
    free(str);
}

int main(void)
{
    /* line by line, so the results are out before a sanitizer report ends the process */
    setvbuf(stdout, NULL, _IOLBF, 0);

    check("cpu_load", "cpu_load");
    check("cpu_load   ", "cpu_load");
    check("param_get 0 gain", "param_get");
    check("", "");
    check("   ", "");
    check(" \t\r\n", "");

    {
        static const char *const set[] = { "property_set", "3", "label", "a \"b\" \\c", NULL };
        static const char *const path[] = { "property_set", "3", "gainfile", "/var/lib/omx/a file.txt", NULL };
        static const char *const empty[] = { "property_set", "3", "label", "", NULL };
        static const char *const utf8[] = { "property_set", "3", "label", "Sala gran \xc3\xa0 \xe2\x9c\x93", NULL };
        static const char *const bare[] = { "param_set", "0", "gain", "0.5", NULL };
        static const char *const lone[] = { "property_set", "3", "label", "C:\\dir", NULL };

        check_all("property_set 3 label \"a \\\"b\\\" \\\\c\"", set);
        check_all("property_set 3 gainfile \"/var/lib/omx/a file.txt\"", path);
        check_all("property_set 3 label \"\"", empty);
        check_all("property_set 3 label \"Sala gran \xc3\xa0 \xe2\x9c\x93\"", utf8);
        check_all("param_set 0 gain 0.5", bare);
        check_all("property_set 3 label \"C:\\\\dir\"", lone);
    }

    if (g_failures)
    {
        printf("%d check(s) failed\n", g_failures);
        return EXIT_FAILURE;
    }

    printf("all checks passed\n");
    return EXIT_SUCCESS;
}
