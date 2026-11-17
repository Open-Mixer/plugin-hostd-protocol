/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * This file is part of mod-host.
 *
 * mod-host is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mod-host is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mod-host.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
************************************************************************************************************************
*
************************************************************************************************************************
*/

/** @file host-scenario.h
 * @brief The declarations of host-scenario.h.
 */

#ifndef HOST_SCENARIO_H
#define HOST_SCENARIO_H


/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include <stddef.h>
#include "host-backend.h"


/*
************************************************************************************************************************
*           CONFIGURATION DEFINES
************************************************************************************************************************
*/

/** @brief the most lines a scenario list holds
 *
 * host_scenario_load refuses a file with more.
 */
#define HOST_SCENARIO_MAX_LINES     128
/** @brief the size a caller gives its array of host_scenario_var_t
 *
 * The library does not read it: host_scenario_load takes the count it is given.
 */
#define HOST_SCENARIO_MAX_VARS      16
/** @brief the most verbs host_scenario_run remembers as not served
 *
 * A verb found not served after that many is not remembered, and the lines it guards are run.
 */
#define HOST_SCENARIO_MAX_ABSENT    16
/** @brief the size in bytes, NUL included, of the buffer host_scenario_run gives an exchange for the reply */
#define HOST_SCENARIO_REPLY_SIZE    256


/*
************************************************************************************************************************
*           DATA TYPES
************************************************************************************************************************
*/

/** @brief one $NAME the scenario file refers to, and its value for the host under test
 *
 * In a command or a reply, a '$' followed by the name is replaced with the value. The text after the '$' only has to
 * start with the name, and the first variable of the array that it starts with is the one used; a '$' no variable
 * matches is left as it is.
 */
typedef struct HOST_SCENARIO_VAR_T {
    /** the name, without the '$'; not NULL */
    const char *name;
    /** the text that replaces it; not NULL */
    const char *value;
} host_scenario_var_t;

/** @brief one line of the scenario file after substitution
 *
 * A line of the file is "[\@\<guard\> ]\<command\> => \<reply\>[|\<reply\>...]". The three strings are allocated by
 * host_scenario_load and freed by host_scenario_free.
 */
typedef struct HOST_SCENARIO_T {
    /** the command to send */
    char *command;
    /** the replies accepted, separated by '|'; the blanks around each are not compared */
    char *reply;
    /** the verb whose absence skips the line, or NULL: a verb an earlier line was answered "resp -902", or "not found"
     * when it is not required of the host */
    char *guard;
} host_scenario_t;

/** @brief the lines of a scenario file, filled by host_scenario_load */
typedef struct HOST_SCENARIO_LIST_T {
    /** the lines, in the order of the file; the first count are set */
    host_scenario_t lines[HOST_SCENARIO_MAX_LINES];
    /** how many lines there are */
    int count;
} host_scenario_list_t;

/** @brief the function host_scenario_run calls to send one command and read its reply
 *
 * host_scenario_socket_exchange and host_scenario_direct_exchange are the two the library has.
 *
 * @param ctx the context given to host_scenario_run.
 * @param[in] command the command, NUL-terminated.
 * @param[out] reply the buffer the reply is stored in, NUL-terminated.
 * @param size the size of reply in bytes.
 * @return 0, or -1 when the host went away or the reply cannot be given; host_scenario_run then stops.
 */
typedef int (*host_scenario_exchange_t)(void *ctx, const char *command, char *reply, size_t size);

/** @brief a backend in this process driven through the protocol parser over a socketpair
 *
 * host_scenario_direct_open fills it and host_scenario_direct_close closes it.
 */
typedef struct HOST_SCENARIO_DIRECT_T {
    /** the socketpair: protocol_parse answers on fd[0] and host_scenario_direct_exchange reads the reply from fd[1];
     * both are -1 after host_scenario_direct_close */
    int fd[2];
    /** the backend given to host_scenario_direct_open; the library sets it and does not read it */
    const host_backend_t *backend;
} host_scenario_direct_t;


/*
************************************************************************************************************************
*           FUNCTION PROTOTYPES
************************************************************************************************************************
*/

/** @brief reads a scenario file into a list
 *
 * A line is trimmed; an empty one and one that starts with '#' are skipped. Any other is cut at its first "=>" into a
 * command and the replies accepted, and a command that starts with '\@' gives its first word, without the '\@', as the
 * guard. The variables are replaced in the command and in the replies, not in the guard. A line is read in pieces of
 * 1023 bytes, so a longer one is read as several.
 *
 * The list is zeroed first, so one that holds lines is freed with host_scenario_free before it is loaded again. The
 * allocations are not checked: with no memory a command, a reply or a guard is left NULL.

 *
 * @pre path and list are not NULL; vars holds var_count variables, their names and values not NULL, and may be NULL
 *     when var_count is 0.
 * @param[in] path the file to read.
 * @param[in] vars the variables to replace.
 * @param var_count how many variables vars holds.
 * @param[out] list the list to fill; its lines are freed with host_scenario_free.
 * @return the number of lines read, which may be 0; -1, the list left empty, when the file cannot be opened, a line
 *     has no "=>", a guard has no command after it, or the file holds more than HOST_SCENARIO_MAX_LINES lines.
 */
int host_scenario_load(const char *path, const host_scenario_var_t *vars, int var_count, host_scenario_list_t *list);
/** @brief frees the lines of a list and leaves it empty
 *
 * @pre list is not NULL and was filled by host_scenario_load.
 * @param[in,out] list the list; its count is set to 0.
 */
void host_scenario_free(host_scenario_list_t *list);

/** @brief runs every line of a list and prints one line per exchange on standard output
 *
 * A line whose guard names a verb found not served is skipped. For any other the command is sent and its reply read,
 * and the line fails when the reply is none of the replies accepted. A verb is found not served when a line of it is
 * answered "resp -902", or "not found" while it is a declared verb not required of the host under test; a declared
 * verb required of it and answered "not found" fails. A verb is the first word of the command, its first 31 bytes.
 *
 * When table is given, an accepted "resp -902" from a verb the table serves, or any other accepted reply from a verb
 * it leaves NULL, is a failure too; the verbs are the ten of host_backend_t.
 *
 * Not thread-safe: it reads the host under test, an unlocked global host_scenario_set_host sets.
 *
 * @pre list and exchange are not NULL, and every line of list has its command and its reply.
 * @param[in] list the lines to run.
 * @param exchange sends one command and stores its reply, in a buffer of HOST_SCENARIO_REPLY_SIZE bytes.
 * @param ctx the context passed to exchange as it is.
 * @param[in] table the backend's table to check the replies against, or NULL for no such check.
 * @return the number of lines that failed; -1 as soon as exchange does not return 0, the lines after it not run.
 */
int host_scenario_run(const host_scenario_list_t *list, host_scenario_exchange_t exchange, void *ctx,
                      const host_backend_t *table);

/** @brief sets the host under test, which decides what a declared verb answered "not found" means
 *
 * It is a word of the declared hosts ("plugin-hostd", "omx-clap-host", "mod-host"): a declared verb it answers "not
 * found" is a verb it does not serve when it is not required of that host, and a failure when it is. NULL, the
 * default, holds every verb required of any host. With a word that is no declared host, no verb is required.
 *
 * The string is not copied: the pointer is kept. Not thread-safe: it sets an unlocked global.
 *
 * @pre host, when not NULL, outlives every host_scenario_run.
 * @param[in] host the host's word, or NULL.
 */
void host_scenario_set_host(const char *host);

/** @brief connects a TCP socket to a host's command port
 *
 * @pre host is not NULL.
 * @param[in] host the address, a dotted IPv4 address; a name is not resolved.
 * @param port the port.
 * @return the connected socket, which the caller closes; -1 when there is no socket, the address is no dotted IPv4
 *     address, or the connection is refused.
 */
int host_scenario_socket_open(const char *host, int port);
/** @brief the exchange over a socket: sends the command with its NUL and reads the reply up to its NUL
 *
 * It waits for the reply with no time limit, so a command the host does not answer blocks it. A reply that does not
 * fit is cut at size - 1 bytes and the rest is left unread on the socket.
 *
 * @pre ctx points to the int that holds the socket of host_scenario_socket_open; command and reply are not NULL; size
 *     is 1 or more.
 * @param[in] ctx a pointer to the socket.
 * @param[in] command the command, NUL-terminated.
 * @param[out] reply the buffer the reply is stored in, NUL-terminated when 0 is returned or when it was cut.
 * @param size the size of reply in bytes.
 * @return 0; -1 when the send fails, the host closes the connection before the NUL, or the reply does not fit.
 */
int host_scenario_socket_exchange(void *ctx, const char *command, char *reply, size_t size);

/** @brief opens a direct exchange with a backend in this process
 *
 * It makes the socketpair and registers, in the library's one table of commands, the backend's verbs
 * (host_dispatch_register), the unsupported ones (host_dispatch_register_unsupported) and hello (protocol_add_hello);
 * what these return is not looked at. Not thread-safe: it uses the table of commands and the session state.
 *
 * @pre direct and backend are not NULL, and backend outlives the exchange.
 * @param[out] direct the exchange to fill.
 * @param[in] backend the backend to drive; it is not copied.
 * @return 0, or -1 when the socketpair cannot be made: nothing is registered then.
 */
int host_scenario_direct_open(host_scenario_direct_t *direct, const host_backend_t *backend);
/** @brief the direct exchange: gives a copy of the command to protocol_parse and reads the reply up to its NUL
 *
 * It waits for the reply with no time limit, so a command protocol_parse does not answer blocks it. A reply that does
 * not fit is cut at size - 1 bytes and the rest is left unread. Not thread-safe: protocol_parse is not.
 *
 * @pre ctx is the host_scenario_direct_t that host_scenario_direct_open filled; command and reply are not NULL; size
 *     is 1 or more.
 * @param[in] ctx the direct exchange.
 * @param[in] command the command, NUL-terminated; it is not modified.
 * @param[out] reply the buffer the reply is stored in, NUL-terminated when 0 is returned or when it was cut.
 * @param size the size of reply in bytes.
 * @return 0; -1 when there is no memory for the copy, nothing run then, or the reply does not fit.
 */
int host_scenario_direct_exchange(void *ctx, const char *command, char *reply, size_t size);
/** @brief closes a direct exchange
 *
 * It removes every command of the library's table with protocol_remove_commands, the ones the caller registered
 * included, and closes both ends of the socketpair. Not thread-safe: it uses the table of commands and the session
 * state.
 *
 * @pre direct is not NULL and was filled by host_scenario_direct_open.
 * @param[in,out] direct the exchange; both of its fd are set to -1.
 */
void host_scenario_direct_close(host_scenario_direct_t *direct);


/*
************************************************************************************************************************
*           END HEADER
************************************************************************************************************************
*/

#endif
