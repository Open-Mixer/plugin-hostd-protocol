/* SPDX-License-Identifier: GPL-3.0-or-later */
/* The conf keys protocol.h declares (PHD_CONF_STRINGS): the daemon reads qualifications through libplugin-qualification-db,
 * so there is no key naming a qualification directory, no subdirectory constant for one and no environment default for it.
 * The key is not kept as a refused name: nothing declares it (the protocol carries no backward-compatibility machinery
 * while it is developed). */

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
    int has_descriptor_dir = 0, has_qualification_key = 0, has_qualification_env = 0, keys = 0;
#define X(key, type, def, env, meaning) \
    keys++; \
    if (strcmp(#key, "descriptor_dir") == 0) has_descriptor_dir = 1; \
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
    printf("conf keys test %s\n", g_failures ? "FAILED" : "ok");
    return g_failures ? 1 : 0;
}
