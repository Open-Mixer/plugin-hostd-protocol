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

/** @file socket.h
 * @brief The declarations of socket.h.
 */

#ifndef SOCKET_H
#define SOCKET_H

/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include "utils.h"       // just for get the msg_t struct


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

/** @brief the size in bytes, its NUL included, of the longest command accepted on the socket
 *
 * A longer one closes the connection: socket_run says so on stderr and returns.
 */
#define SOCKET_MSG_MAX_SIZE     (16 * 1024 * 1024)

/*
************************************************************************************************************************
*           DATA TYPES
************************************************************************************************************************
*/


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

/** @brief sets the address the next socket_start binds both ports to
 *
 * Until this is called it is the loopback address, so only this machine reaches the ports. "0.0.0.0" is every address
 * of the machine. A controller is not authenticated: widen this only on a network whose hosts are trusted. Not
 * thread-safe: it sets an unlocked global.
 *
 * @param[in] address a dotted IPv4 address, NUL-terminated.
 * @return 0, or -1, the setting unchanged, for NULL and for anything that is no dotted IPv4 address.
 */
int socket_set_bind_address(const char *address);
/** @brief binds the command port and the feedback port and puts them to listen
 *
 * Both are TCP sockets bound to the address of socket_set_bind_address, with SO_REUSEADDR set where the system is not
 * Windows. The receive callback is forgotten, so socket_set_receive_cb is called after this. The sockets of an earlier
 * socket_start are not closed: socket_finish comes first. Not thread-safe: it sets unlocked globals.
 *
 * @pre buffer_size is 1 or more.
 * @param socket_port the port the commands are received on.
 * @param feedback_port the port the feedback lines are sent on, or 0 for no feedback socket.
 * @param buffer_size the size in bytes socket_run starts its receive buffer with; it doubles it as a command needs.
 * @return 0, or -1 when a socket cannot be made, bound or put to listen: stderr says which, and a socket already made
 *     stays open until socket_finish.
 */
int socket_start(int socket_port, int feedback_port, int buffer_size);
// serves sockets the caller already bound instead of binding its own, as socket_start serves the ports it binds: a
// supervisor or a test harness binds the command port (and the feedback port, or -1 for none), keeps the address its
// own while it starts the program, and hands it the fds, so no port is ever closed and bound again in between (the
// bind 0, close, rebind a harness did let another process take the port). Each fd is a TCP socket bound to an IPv4
// address and port; its options are the caller's (socket_start sets SO_REUSEPORT, this does not), and it is put to
// listen. socket_finish closes them. Answers 0, or -1 for an fd that is no bound TCP socket, nothing served then
#define SOCKET_HAS_ADOPT 1
int socket_adopt(int socket_fd, int feedback_fd, int buffer_size);
/** @brief stops serving: closes the listening sockets and shuts down the connections of the controller
 *
 * The connections are shut down, not closed: the socket_run that serves them ends its loop and closes them. With no
 * listening socket it does nothing. It takes no lock: the sockets are unlocked globals, which it marks closed before
 * it closes them.
 */
void socket_finish(void);
/** @brief sets the function socket_run gives each command it receives
 *
 * socket_start and socket_adopt forget it. Not thread-safe: it sets an unlocked global.
 *
 * @param receive_cb called with one command: msg->sender_id is the connection it came on, msg->data the command,
 *     NUL-terminated, and msg->data_size its length, the NUL not counted. msg and its data belong to socket_run and
 *     are valid during the call; the callback may write in the data, as protocol_parse does. With NULL the commands
 *     are dropped.
 */
void socket_set_receive_cb(void (*receive_cb)(msg_t *msg));
/** @brief sets the function socket_run calls between commands
 *
 * It is called after each read that brought data, once the commands in it were given to the receive callback, and,
 * where the system is not Windows and an idle interval is set, each time that interval passes with nothing to read.
 * Not thread-safe: it sets an unlocked global.
 *
 * @param idle_cb the function, or NULL for none.
 */
void socket_set_idle_cb(void (*idle_cb)(void));
/** @brief sets how long socket_run waits for a command before it calls the idle callback
 *
 * socket_run reads it before each read. Not thread-safe: it sets an unlocked global.
 *
 * @param interval_ms the interval in milliseconds; with 0 or less socket_run waits for a command with no limit, as
 *     before this is called.
 */
void socket_set_idle_interval(int interval_ms);
/** @brief sends size bytes on a connection, in as many sends as it takes
 *
 * A connection that went away does not raise SIGPIPE where the system has MSG_NOSIGNAL. It keeps no state.
 *
 * @pre buffer holds size bytes.
 * @param destination the connection, as msg_t.sender_id gives it.
 * @param[in] buffer the bytes to send.
 * @param size how many bytes to send.
 * @return a positive number, the bytes the last send took, once all were sent; -1 when a send fails, which stderr
 *     says, and, nothing sent, when size is 0 or less.
 */
int socket_send(int destination, const char *buffer, int size);
/** @brief sends one line, with its NUL, on the feedback connection
 *
 * In a session that hello granted seq, the line follows its number, "\#\<n\> ", in the same send. It takes no lock:
 * the number is counted atomically, and the feedback connection is read unlocked while socket_run opens and closes it.
 *
 * @pre buffer is not NULL.
 * @param[in] buffer the line, NUL-terminated.
 * @return what socket_send returns, a positive number once the line is sent; -1 when no controller holds the feedback
 *     connection, when a send fails, and when there is no memory for a numbered line, whose number is then missing.
 */
int socket_send_feedback(const char *buffer);
/** @brief serves one controller, from its connection to its end
 *
 * It waits for a connection on the command port and then, when there is a feedback port, for one on it. A command ends
 * with a NUL; each is given to the receive callback, and the idle callback is called as socket_set_idle_cb says. A
 * last command the controller did not end is given too when it disconnects. It returns, both connections closed, when
 * the controller disconnects, when a command reaches SOCKET_MSG_MAX_SIZE, and after socket_finish; a host calls it
 * again for the next controller.
 *
 * What a controller set for its connection starts and ends here: once protocol_add_hello was called, the grants of
 * hello are dropped as it connects; once host_dispatch_register_monitor_output_off was called, the monitors left are
 * ended as it disconnects. One thread calls it: it uses unlocked globals.

 *
 * @pre socket_start or socket_adopt returned 0.
 * @param exit_on_failure what a failure does (no memory for the buffer, a connection not accepted, a read that
 *     fails): nonzero says it on stderr and ends the process with exit(EXIT_FAILURE); 0 returns.
 */
void socket_run(int exit_on_failure);



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
