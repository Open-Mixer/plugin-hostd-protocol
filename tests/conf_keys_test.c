/* SPDX-License-Identifier: GPL-3.0-or-later */
/* The conf keys protocol.h declares (PHD_CONF_STRINGS): the daemon reads qualifications through libplugin-qualification-db,
 * so there is no key naming a qualification directory, no subdirectory constant for one and no environment default for it.
 * The key is not kept as a refused name: nothing declares it (the protocol carries no backward-compatibility machinery
 * while it is developed).
 *
 * The descriptions store's ask (omx-panel docs/descriptions-store.md section 7) is set by ask_unknown, a word setting:
 * auto (the default) asks when plugin-qualify-sync is on the path, 1 always asks, 0 never does. The daemon reads its
 * settings only from this table (R-095), so the key, its words and its default are declared here before it reads them. */

#include <stdio.h>
#include <string.h>

#include "plugin-hostd/protocol.h"

static int g_failures;

static void check(int ok, const char *what)
{
    if (!ok)
        g_failures++;
    printf("%s %s\n", ok ? "ok" : "FAIL", what);
}

int main(void)
{
    int has_descriptor_dir = 0, has_qualification_key = 0, has_qualification_env = 0, keys = 0, ask_unknown = 0;
    const char *ask_type = "", *ask_default = "", *ask_env = NULL;
#define X(key, type, def, env, meaning) \
    keys++; \
    if (strcmp(#key, "descriptor_dir") == 0) has_descriptor_dir = 1; \
    if (strcmp(#key, "ask_unknown") == 0) { ask_unknown++; ask_type = #type; ask_default = def; ask_env = env; } \
    if (strstr(#key, "qualification") != NULL) has_qualification_key = 1; \
    if (strstr(env, "QUALIFICATION") != NULL) has_qualification_env = 1;
    PHD_CONF_STRINGS(X)
#undef X
    check(keys > 0 && has_descriptor_dir, "the conf keys are read, and descriptor_dir is one of them");
    check(!has_qualification_key, "no conf key names a qualification directory");
    check(!has_qualification_env, "no conf key has a QUALIFICATION environment variable");
#ifdef PHD_DIR_QUALIFICATION_SUBDIR
    check(0, "PHD_DIR_QUALIFICATION_SUBDIR is not defined");
#else
    check(1, "PHD_DIR_QUALIFICATION_SUBDIR is not defined");
#endif
    check(ask_unknown == 1, "ask_unknown is one row of PHD_CONF_STRINGS");
    check(strcmp(ask_type, "word") == 0, "ask_unknown is a word setting");
    check(strcmp(ask_default, "auto") == 0, "the default ask_unknown is auto");
    check(ask_env != NULL && ask_env[0] == '\0', "ask_unknown has no environment variable");
#ifdef PHD_ASK_UNKNOWN_MODES
    {
        static const char *const want[] = { "auto", "1", "0" };
        const char *got[8];
        int n = 0, same = 1, i;
#define X(id, name, meaning) if (n < 8) got[n] = name; n++;
        PHD_ASK_UNKNOWN_MODES(X)
#undef X
        same = n == 3;
        for (i = 0; same && i < 3; i++)
            same = strcmp(got[i], want[i]) == 0;
        check(same, "PHD_ASK_UNKNOWN_MODES are auto, 1 and 0, in that order");
        check(strcmp(PHD_DEFAULT_ASK_UNKNOWN, PHD_ASK_UNKNOWN_AUTO) == 0, "PHD_DEFAULT_ASK_UNKNOWN is PHD_ASK_UNKNOWN_AUTO");
    }
#else
    check(0, "PHD_ASK_UNKNOWN_MODES lists the words of ask_unknown");
#endif
    printf("conf keys test %s\n", g_failures ? "FAILED" : "ok");
    return g_failures ? 1 : 0;
}
