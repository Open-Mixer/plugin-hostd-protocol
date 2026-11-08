/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * This file is part of plugin-hostd-protocol.
 *
 * monitor_output_off (doc/protocol.md, Monitors), through the library's own server on a real TCP socket:
 *   - it ends a monitor this connection set: resp 0, the backend's end called once, and a second one is -543;
 *   - an output this connection never monitored, or that a refused monitor_output did not set, is -543 and the backend
 *     is not called;
 *   - a monitor ends with the connection that set it: the backend's end is called for each output left, and the next
 *     connection cannot end what an earlier one set (-543);
 *   - remove takes the instance's monitors with it (remove -1 every one), the backend's end not called for them;
 *   - the backend refusing the end answers its code and the monitor stays;
 *   - a malformed line of the shape is refused, and a host that does not register the verb answers -902.
 *
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 */

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <host-backend.h>
#include <host-dispatch.h>
#include <host-errors.h>
#include <protocol.h>
#include <socket.h>

#include "test-client.h"

#define MONITOR_PORT_DEFAULT 15598
#define REPLY_MAX 256

static int g_failures;
static volatile int g_running = 1;

/* what the backend was asked, in order: "on(3,level) off(3,level) ..." */
static char g_log[1024];
static int g_end_refused;

static void logged(const char *what, int instance, const char *symbol)
{
    size_t len = strlen(g_log);

    snprintf(g_log + len, sizeof(g_log) - len, "%s(%d,%s) ", what, instance, symbol);
}

/* instances 3 and 4 have outputs; any other instance cannot be monitored */
static int b_monitor_output(int instance, const char *symbol)
{
    logged("on", instance, symbol);
    return instance == 3 || instance == 4 ? 0 : -1;
}

static int b_monitor_output_off(int instance, const char *symbol)
{
    logged("off", instance, symbol);
    return g_end_refused ? ERR_INSTANCE_NON_EXISTS : SUCCESS;
}

static int b_remove(int instance)
{
    return instance == 3 || instance == 4 || instance == -1 ? SUCCESS : ERR_INSTANCE_NON_EXISTS;
}

static const host_backend_t g_backend = { .remove = b_remove };

static void *server_thread(void *arg)
{
    (void)arg;
    while (g_running)
        socket_run(0);
    return NULL;
}

static void expect(conn_t *c, const char *what, const char *command, const char *want)
{
    char got[REPLY_MAX];

    exchange(c, command, got, sizeof(got));
    if (strcmp(got, want) != 0)
    {
        printf("FAIL %s '%.120s': got '%.200s', want '%.200s'\n", what, command, got, want);
        g_failures++;
        return;
    }
    printf("ok   %s '%.80s' -> '%.80s'\n", what, command, got);
}

/* the backend's calls so far, then forgotten */
static void expect_log(const char *what, const char *want)
{
    if (strcmp(g_log, want) != 0)
    {
        printf("FAIL %s: the backend was asked '%s', want '%s'\n", what, g_log, want);
        g_failures++;
    }
    else
        printf("ok   %s: backend asked '%s'\n", what, g_log);
    g_log[0] = '\0';
}

static void register_all(int with_off)
{
    protocol_remove_commands();
    host_dispatch_register(&g_backend);
    host_dispatch_register_monitor_output(b_monitor_output);
    if (with_off)
        host_dispatch_register_monitor_output_off(b_monitor_output_off);
    host_dispatch_register_unsupported();
}

int main(void)
{
    const char *env = getenv("MONITOR_PORT");
    pthread_t thread;
    conn_t c;

    setvbuf(stdout, NULL, _IOLBF, 0);
    signal(SIGPIPE, SIG_IGN);
    g_port = env ? atoi(env) : MONITOR_PORT_DEFAULT;
    if (socket_start(g_port, g_port + 1, 8192) != 0)
    {
        printf("FAIL socket_start on ports %d and %d\n", g_port, g_port + 1);
        return 1;
    }
    socket_set_receive_cb(protocol_parse);
    pthread_create(&thread, NULL, server_thread, NULL);

    /* -- a host that does not register the verb */
    register_all(0);
    c = open_session();
    expect(&c, "no hook", "monitor_output 3 level", "resp 1");
    expect(&c, "no hook", "monitor_output_off 3 level", "resp -902");
    close_session(&c);
    expect_log("no hook", "on(3,level) ");

    register_all(1);
    c = open_session();

    /* -- it ends a monitor this connection set, once */
    expect(&c, "never monitored", "monitor_output_off 3 level", "resp -543");
    expect_log("never monitored", "");
    expect(&c, "monitor", "monitor_output 3 level", "resp 1");
    expect(&c, "end it", "monitor_output_off 3 level", "resp 0");
    expect(&c, "end it again", "monitor_output_off 3 level", "resp -543");
    expect_log("end once", "on(3,level) off(3,level) ");

    /* -- one monitor is one, however often it was asked for; another output of the instance stays */
    expect(&c, "monitor", "monitor_output 3 level", "resp 1");
    expect(&c, "monitor again", "monitor_output 3 level", "resp 1");
    expect(&c, "monitor another output", "monitor_output 3 peak", "resp 1");
    expect(&c, "end the first", "monitor_output_off 3 level", "resp 0");
    expect(&c, "the first is gone after one end", "monitor_output_off 3 level", "resp -543");
    expect(&c, "the other stays", "monitor_output_off 4 peak", "resp -543");
    expect_log("one monitor", "on(3,level) on(3,level) on(3,peak) off(3,level) ");

    /* -- a monitor the backend refused was never set */
    expect(&c, "refused monitor", "monitor_output 9 level", "resp 0");
    expect(&c, "end the refused", "monitor_output_off 9 level", "resp -543");
    expect_log("refused monitor", "on(9,level) ");

    /* -- the backend refusing the end: its code, and the monitor stays until it ends */
    g_end_refused = 1;
    expect(&c, "backend refuses the end", "monitor_output_off 3 peak", "resp -3");
    g_end_refused = 0;
    expect(&c, "the monitor stayed", "monitor_output_off 3 peak", "resp 0");
    expect_log("refused end", "off(3,peak) off(3,peak) ");

    /* -- a monitor ends with the connection that set it */
    expect(&c, "monitor", "monitor_output 3 level", "resp 1");
    expect(&c, "monitor", "monitor_output 4 peak", "resp 1");
    close_session(&c);
    expect_log("the connection ended", "on(3,level) on(4,peak) off(4,peak) off(3,level) ");
    c = open_session();
    expect(&c, "an earlier connection's monitor", "monitor_output_off 3 level", "resp -543");
    expect(&c, "an earlier connection's monitor", "monitor_output_off 4 peak", "resp -543");
    expect_log("next connection", "");

    /* -- remove takes the instance's monitors with it, the backend not asked to end them */
    expect(&c, "monitor 3", "monitor_output 3 level", "resp 1");
    expect(&c, "monitor 4", "monitor_output 4 level", "resp 1");
    expect(&c, "remove 3", "remove 3", "resp 0");
    expect(&c, "3's monitor went with it", "monitor_output_off 3 level", "resp -543");
    expect(&c, "4's stays", "monitor_output_off 4 level", "resp 0");
    expect_log("remove", "on(3,level) on(4,level) off(4,level) ");
    expect(&c, "monitor 3", "monitor_output 3 level", "resp 1");
    expect(&c, "monitor 4", "monitor_output 4 level", "resp 1");
    expect(&c, "remove every instance", "remove -1", "resp 0");
    expect(&c, "3 went with remove -1", "monitor_output_off 3 level", "resp -543");
    expect(&c, "4 went with remove -1", "monitor_output_off 4 level", "resp -543");
    expect_log("remove -1", "on(3,level) on(4,level) ");
    expect(&c, "a refused remove", "remove 8", "resp -3");

    /* -- a malformed line of the shape */
    expect(&c, "too few words", "monitor_output_off 3", "few arguments");
    expect(&c, "too many words", "monitor_output_off 3 level 1", "many arguments");
    expect(&c, "an instance that is no number", "monitor_output_off 3x level", "resp -902");
    expect(&c, "no words", "monitor_output_off", "few arguments");
    expect_log("malformed", "");
    close_session(&c);
    g_log[0] = '\0';

    g_running = 0;
    printf("%s: %d failure(s)\n", g_failures ? "FAIL" : "ok", g_failures);
    return g_failures ? 1 : 0;
}
