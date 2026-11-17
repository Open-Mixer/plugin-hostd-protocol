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

/** @file protocol.h
 * @brief The declarations of protocol.h.
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H


/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include "utils.h"
#include "socket.h"

/*
************************************************************************************************************************
*           DO NOT CHANGE THESE DEFINES
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           CONFIGURATION DEFINES
************************************************************************************************************************
*/

/** @brief the most commands the table holds
 *
 * protocol_add_command refuses the next one.
 */
#define PROTOCOL_MAX_COMMANDS       128

// error messages configuration
/** @brief the answer for a line no registered command matches (code PROTOCOL_NOT_FOUND) */
#define MESSAGE_COMMAND_NOT_FOUND   "not found"
/** @brief the answer for a line with more arguments than its command takes (code PROTOCOL_MANY_ARGUMENTS) */
#define MESSAGE_MANY_ARGUMENTS      "many arguments"
/** @brief the answer for a line with fewer arguments than its command takes (code PROTOCOL_FEW_ARGUMENTS) */
#define MESSAGE_FEW_ARGUMENTS       "few arguments"
/** @brief the answer for an invalid argument
 *
 * Its code is PROTOCOL_INVALID_ARGUMENT; protocol_error_message returns it, and protocol_parse never chooses this code.
 */
#define MESSAGE_INVALID_ARGUMENT    "invalid argument"

// protocol_parse() errors, answered with protocol_error_message()
/** @brief the code of a line that matches no command, answered with MESSAGE_COMMAND_NOT_FOUND */
#define PROTOCOL_NOT_FOUND          (-1)
/** @brief the code of a line with more arguments than its command takes, answered with MESSAGE_MANY_ARGUMENTS */
#define PROTOCOL_MANY_ARGUMENTS     (-2)
/** @brief the code of a line with fewer arguments than its command takes, answered with MESSAGE_FEW_ARGUMENTS */
#define PROTOCOL_FEW_ARGUMENTS      (-3)
/** @brief the code of an invalid argument, answered with MESSAGE_INVALID_ARGUMENT */
#define PROTOCOL_INVALID_ARGUMENT   (-4)

/** @brief marks that this library answers hello, request tags and error words (doc/protocol.md, Sessions)
 *
 * A host calls protocol_add_hello() to enable them; a consumer that also builds against mod-host's library tests this
 * before it calls it.
 */
#define PROTOCOL_HELLO              1


/*
************************************************************************************************************************
*           DATA TYPES
************************************************************************************************************************
*/

/** @brief the argument of a command's callback: the words of the line that matched, and the response it sets */
typedef struct PROTO_T {
    /** the words of the line, the verb first; the array is owned by protocol_parse, which frees it after the callback
     * returns, and the words point into msg->data, so the callback must not free them or keep them */
    char **list;
    /** how many words list holds */
    uint32_t list_count;
    /** the response, a NUL-terminated string set by protocol_response or protocol_response_int; NULL when the callback
     * sets none, and then no answer is sent. protocol_parse sends it and frees it */
    char *response;
    /** the length of response in bytes, its NUL not counted */
    uint32_t response_size;
} proto_t;


/*
************************************************************************************************************************
*           GLOBAL VARIABLES
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           MACRO'S
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           FUNCTION PROTOTYPES
************************************************************************************************************************
*/

/** @brief parses one received line, runs the callback of the command it matches and sends the answer
 *
 * With the tags extension granted by hello, a first word \@\<tag\> is the request's tag and the answer follows it. The
 * first form of a verb the arguments fit in number and in types is run; else the first that fits in number; else the
 * first form's refusal. A refusal is sent as the message of PROTOCOL_NOT_FOUND, PROTOCOL_MANY_ARGUMENTS or
 * PROTOCOL_FEW_ARGUMENTS. A matching command with a NULL callback is matched and nothing is sent.
 *
 * An empty or whitespace-only line is ignored and gets no reply, as mod-host does, and changes nothing, unless it is
 * tagged, when the answer is PROTOCOL_NOT_FOUND so every tagged request has its answer.
 *
 * The answer goes to msg->sender_id: written to it when it is standard output (unless built with SKIP_READLINE), else
 * sent with socket_send.
 *
 * msg->data is modified in place: strarr_split trims the trailing blanks and cuts the line into words by writing NULs,
 * and the words of proto_t point into it.
 *
 * Not thread-safe: it uses the table of commands and the session state, which are unlocked, so one thread calls the
 * library.
 *
 * @pre msg is not NULL, msg->data is a NUL-terminated line that may be written, and msg->sender_id is the connection to
 *     answer on.
 * @param[in,out] msg the received line.
 */
void protocol_parse(msg_t *msg);
/** @brief registers a command
 *
 * The command is a line of words; a word with a '%' is a wildcard (\%i a whole number, \%f a number, any other takes
 * any word), and a last word of "..." takes any more words. A verb may be registered more than once, one form each. A
 * NULL callback is accepted: a matching line is matched and nothing is sent.
 *
 * Not thread-safe: it uses the table of commands and the session state, which are unlocked, so one thread calls the
 * library.
 *
 * @pre command is not NULL.
 * @param[in] command the command line; it is copied.
 * @param callback called with the words of a matching line; it sets a response with protocol_response or
 *     protocol_response_int, or none.
 * @return 0, or -1 when the table already holds PROTOCOL_MAX_COMMANDS commands: the command is not registered, and
 *     stderr says which.
 */
int protocol_add_command(const char *command, void (*callback)(proto_t *proto));
/** @brief sets the response of a callback to a copy of a string
 *
 * protocol_parse sends the response and frees it, so the callback does not. A response set twice leaks the first.
 *
 * @pre response and proto are not NULL, and proto is the argument of the running callback.
 * @param[in] response the text, NUL-terminated.
 * @param[out] proto the argument of the callback; its response and response_size are set.
 */
void protocol_response(const char *response, proto_t *proto);
/** @brief sets the response of a callback to "resp \<resp\>"
 *
 * @pre proto is not NULL, and is the argument of the running callback.
 * @param resp the number to answer.
 * @param[out] proto the argument of the callback, as for protocol_response.
 */
void protocol_response_int(int resp, proto_t *proto);
/** @brief removes every registered command and starts a new session
 *
 * Nothing is granted afterwards, and every line is answered as mod-host answers it. Not thread-safe: it uses the table
 * of commands and the session state, which are unlocked, so one thread calls the library.
 */
void protocol_remove_commands(void);
/** @brief registers hello, which the library answers itself
 *
 * The client names its protocol version and the extensions it asks for, and the reply names the protocol version and
 * the extensions granted, from the declared table (doc/protocol.md, Sessions). A session granted tags may tag a line,
 * and its answer carries the tag; one granted errors reads the declared error word after a refusal's number. What hello
 * grants lasts until the next hello or the next connection; a client that never sends hello is answered as before.
 *
 * The session hooks of the socket are installed even when the registration fails and -1 is returned. Not thread-safe:
 * it uses the table of commands and the session state, which are unlocked, so one thread calls the library.
 *
 * @return 0, or -1 as protocol_add_command.
 */
int protocol_add_hello(void);
/** @brief turns on or off the lines protocol_parse prints to standard output about what it received and answered
 *
 * Not thread-safe: it sets an unlocked global.
 *
 * @param verbose nonzero to print them, 0 not to.
 */
void protocol_verbose(int verbose);
/** @brief gives the message of a protocol_parse error code
 *
 * @param code PROTOCOL_NOT_FOUND, PROTOCOL_MANY_ARGUMENTS, PROTOCOL_FEW_ARGUMENTS or PROTOCOL_INVALID_ARGUMENT.
 * @return the message, a static string not to be freed; NULL for any other code.
 */
const char *protocol_error_message(int code);


/*
************************************************************************************************************************
*           CONFIGURATION ERRORS
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           END HEADER
************************************************************************************************************************
*/

#endif
