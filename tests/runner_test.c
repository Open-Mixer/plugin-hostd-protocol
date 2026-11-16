/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * This file is part of plugin-hostd-protocol.
 *
 * The scenario runner and "not found": a declared verb not required of the host under test (hello, which no host is
 * required to answer; worker_list, which only plugin-hostd is) is a verb that host does not serve, and the lines it
 * guards are skipped; a declared verb required of the host fails, and does not skip what it guards. With no host
 * named, a verb required of any host fails. A host answers from a fixed list here, so nothing but the runner is tested.
 *
 * The refusals are the demonstration, so they are ASSERTED, never printed as failures: the runner's output is captured
 * around each run, and each refusal the run expects ("FAIL '<line>': not found, and <verb> is required of <host>") is
 * an "ok   host <host>: refused ..." line; a run whose counts differ from what it expects prints the runner's lines as
 * they are, FAIL at column 0, and fails. RUNNER_TEST_SABOTAGE=unexpected-refusal makes plugin-hostd's run expect no
 * failure, so its refusal is unexpected and must fail (tests/runner-quiet.sh).
 *
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <host-scenario.h>

static int g_failures;
static int g_sent_guarded;

/* the host: it knows neither worker_list nor hello */
static int exchange(void *ctx, const char *command, char *reply, size_t size)
{
    (void)ctx;
    if (strcmp(command, "remove 3") == 0 || strcmp(command, "remove 4") == 0)
        g_sent_guarded++;
    snprintf(reply, size, "%s", strncmp(command, "remove", 6) == 0 ? "resp 0" : "not found");
    return 0;
}

/* the runner's stdout during one run, in a temporary file: the captured text, or NULL */
static char *captured_run(host_scenario_list_t *list, int *failures)
{
    FILE *tmp = tmpfile();
    int saved;
    long size;
    char *text;

    fflush(stdout);
    saved = dup(STDOUT_FILENO);
    if (!tmp || saved < 0 || dup2(fileno(tmp), STDOUT_FILENO) < 0)
    {
        *failures = host_scenario_run(list, exchange, NULL, NULL);
        if (tmp)
            fclose(tmp);
        if (saved >= 0)
            close(saved);
        return NULL;
    }
    *failures = host_scenario_run(list, exchange, NULL, NULL);
    fflush(stdout);
    dup2(saved, STDOUT_FILENO);
    close(saved);
    size = ftell(tmp);
    text = calloc(1, size > 0 ? (size_t)size + 1 : 1);
    if (text && size > 0)
    {
        rewind(tmp);
        if (fread(text, 1, (size_t)size, tmp) != (size_t)size)
            text[0] = '\0';
    }
    fclose(tmp);
    return text;
}

static void run(const char *path, const char *host, int want_failures, int want_guarded)
{
    host_scenario_list_t list;
    int failures;
    char refusal[160];
    char *text, *line, *next;
    int refused = 0;

    if (host_scenario_load(path, NULL, 0, &list) <= 0)
    {
        printf("FAIL cannot load %s\n", path);
        g_failures++;
        return;
    }
    host_scenario_set_host(host);
    g_sent_guarded = 0;
    text = captured_run(&list, &failures);
    host_scenario_free(&list);
    snprintf(refusal, sizeof(refusal), "FAIL 'worker_list': not found, and worker_list is required of %s",
             host ? host : "a host");
    if (failures != want_failures || g_sent_guarded != want_guarded)
    {
        /* not what the demonstration expects: the runner's lines as they are, FAIL at column 0 */
        if (text)
            fputs(text, stdout);
        printf("FAIL host %s: %d failures and %d guarded lines sent, want %d and %d\n", host ? host : "(none)",
               failures, g_sent_guarded, want_failures, want_guarded);
        free(text);
        g_failures++;
        return;
    }
    for (line = text; line && *line; line = next)
    {
        next = strchr(line, '\n');
        if (next)
            *next++ = '\0';
        else
            next = line + strlen(line);
        if (want_failures > 0 && strcmp(line, refusal) == 0)
        {
            printf("ok   host %s: refused %s\n", host ? host : "(none)", line + strlen("FAIL "));
            refused++;
        }
        else if (strncmp(line, "FAIL", 4) == 0)
        {
            printf("%s\n", line);
            printf("FAIL host %s: a failure the demonstration does not expect\n", host ? host : "(none)");
            g_failures++;
        }
        else
            printf("%s\n", line);
    }
    free(text);
    if (refused != want_failures)
    {
        printf("FAIL host %s: %d refusals named, want %d\n", host ? host : "(none)", refused, want_failures);
        g_failures++;
        return;
    }
    printf("ok   host %s: %d failures, %d guarded lines sent\n", host ? host : "(none)", failures, g_sent_guarded);
}

int main(void)
{
    char path[] = "/tmp/runner-test-XXXXXX";
    const char *sab = getenv("RUNNER_TEST_SABOTAGE");
    int sabotage = sab && strcmp(sab, "unexpected-refusal") == 0;
    int fd = mkstemp(path);
    static const char scenarios[] =
        "worker_list => resp 0 | not found\n"
        "@worker_list remove 3 => resp 0\n"
        "hello 1 => resp 0 1 | not found\n"
        "@hello remove 4 => resp 0\n";

    if (fd < 0 || write(fd, scenarios, sizeof(scenarios) - 1) != (ssize_t)(sizeof(scenarios) - 1))
    {
        printf("FAIL cannot write %s\n", path);
        return 1;
    }
    close(fd);

    /* plugin-hostd is required to answer worker_list: a failure, and its guarded line is sent */
    run(path, "plugin-hostd", sabotage ? 0 : 1, 1);
    /* mod-host is not: worker_list and hello are verbs it does not serve, and both guarded lines are skipped */
    run(path, "mod-host", 0, 0);
    /* no host named: required of one host is required */
    run(path, NULL, 1, 1);
    unlink(path);

    printf("%s\n", g_failures == 0 ? "runner ok" : "runner FAILED");
    return g_failures == 0 ? 0 : 1;
}
