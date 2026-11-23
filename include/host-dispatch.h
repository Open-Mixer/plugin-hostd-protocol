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

/** @file host-dispatch.h
 * @brief The declarations of host-dispatch.h.
 */

#ifndef HOST_DISPATCH_H
#define HOST_DISPATCH_H


/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include "protocol.h"
#include "host-backend.h"

#include <stdint.h>


/*
************************************************************************************************************************
*           DATA TYPES
************************************************************************************************************************
*/

/** @brief the size in bytes, NUL included, of a string of a page of remote controls
 *
 * track_info refuses a name of this many bytes or more.
 */
#define HOST_INFO_STRING_SIZE 256
/** @brief the number of parameter slots in one page of remote controls */
#define HOST_REMOTE_PAGE_SLOTS 8

/** @brief what host_plugin_info_t.param_info answers about one parameter
 *
 * The param_info verb zeroes it, calls param_info, and when it returns 0 or more (SUCCESS is 0, but any value >= 0 is
 * accepted, not only SUCCESS) writes "resp 0" and then unit, scale, min, max, def, step and stable_symbol, each as it
 * is: the library checks none of them. A number is written as the shortest decimal that reads back to the same
 * double.
 */
typedef struct HOST_PARAM_INFO_T {
    /** the unit, one word: the standard unit id, a const of omx-contract's PARAM_UNITS ("dB", "Hz", "percent",
     * "none"), never a format's own spelling (plugin-hostd#215 D1); not NULL when param_info returns 0 or more, and
     * not freed */
    const char *unit;
    /** the scale, one word: "linear", "log" or "stepped"; not NULL when param_info
     * returns 0 or more, and not freed */
    const char *scale;
    /** the lowest value */
    double min, max, def, step;
    /** @var HOST_PARAM_INFO_T::max
     * the highest value */
    /** @var HOST_PARAM_INFO_T::def
     * the default value */
    /** @var HOST_PARAM_INFO_T::step
     * the step between two values, 0 for a continuous parameter */
    /** the name the parameter keeps across hosts, one word; not NULL when param_info returns 0 or more, and not
     * freed */
    const char *stable_symbol;
} host_param_info_t;

/** @brief one page of remote controls, as host_plugin_info_t.remote_page_get answers it
 *
 * The remote_page_get verb zeroes it, calls remote_page_get, and when it returns 0 or more (SUCCESS is 0, but any value
 * >= 0 is accepted, not only SUCCESS) writes "resp 0", the id, the section and the name in quotes, and the
 * HOST_REMOTE_PAGE_SLOTS slots. Every string is NUL-terminated.
 */
typedef struct HOST_REMOTE_PAGE_T {
    /** the id of the page */
    uint32_t id;
    /** the section the page belongs to; a '"' in it is written \\" */
    char section[HOST_INFO_STRING_SIZE];
    /** the name of the page; a '"' in it is written \\" */
    char name[HOST_INFO_STRING_SIZE];
    /** the parameter symbol of each slot, one word; an empty string is an empty slot, written "-" */
    char slots[HOST_REMOTE_PAGE_SLOTS][HOST_INFO_STRING_SIZE];
} host_remote_page_t;

/** @brief the callbacks behind the plugin information verbs, given to host_dispatch_register_plugin_info
 *
 * A NULL entry answers ERR_INVALID_OPERATION. A negative return of a callback is a code of host-errors.h and is the
 * verb's answer. The instance is the line's word read with atoi, so a callback checks it.
 */
typedef struct HOST_PLUGIN_INFO_T {
    /** track_info: name is valid UTF-8 of less than HOST_INFO_STRING_SIZE bytes with no control character, and may be
     * ""; color is "\#RRGGBB" or "-"; kind is "bus", "return", "master", or NULL for a line with no kind. The library
     * answers a line that breaks any of these, or has more words, ERR_INVALID_OPERATION and does not call it. The
     * strings are the line's words, not to be kept. */
    int (*track_info)(int instance, const char *name, const char *color, const char *kind);
    /** remote_pages: returns the number of pages of the instance, or a negative code; the return is the answer */
    int (*remote_pages)(int instance);
    /** remote_page_get: fills page_out, which arrives zeroed, with the page of that index (the line's word read with
     * atoi, unchecked); a return of 0 or more answers the page */
    int (*remote_page_get)(int instance, int page, host_remote_page_t *page_out);
    /** param_info: fills info, which arrives zeroed, for the parameter of that symbol; a return of 0 or more answers
     * it, so its three strings must then be set. An answer that does not fit SOCKET_MSG_BUFFER_SIZE bytes is refused
     * ERR_INVALID_OPERATION. */
    int (*param_info)(int instance, const char *symbol, host_param_info_t *info);
} host_plugin_info_t;


/*
************************************************************************************************************************
*           FUNCTION PROTOTYPES
************************************************************************************************************************
*/

/** @brief registers the verbs of a backend
 *
 * They are add, remove, preset_load, connect, disconnect, bypass, param_set, param_get, state_load and state_save, and
 * the bulk verbs over them (doc/protocol.md): param_get_many, the values of several parameters of an instance in one
 * line, over param_get; param_set_many, several values set all or nothing, over param_get and param_set. Each of the
 * ten calls the entry of its name and answers "resp" and what it returns, param_get also the value, written \%.04f; a
 * NULL entry answers ERR_INVALID_OPERATION. A remove that returns 0 or more also drops the instance's curve bindings
 * and the monitors kept for it.
 *
 * The table is not copied: the pointer is kept and read at every line, and the curve verbs read it too. The result of
 * protocol_add_command is not looked at: a verb the table of commands has no room for is not registered, and stderr
 * says which. Not thread-safe: it sets unlocked globals and uses the table of commands, so one thread calls the
 * library.
 *
 * @pre backend is not NULL and outlives every protocol_parse.
 * @param[in] backend the backend's entries.
 */
void host_dispatch_register(const host_backend_t *backend);
/** @brief registers monitor_output
 *
 * The verb answers as mod-host does: "resp 1" when monitor_output returns 0, "resp 0" when it returns anything else. A
 * NULL monitor_output answers ERR_INVALID_OPERATION. Once host_dispatch_register_monitor_output_off was given a
 * callback, an output that is monitored is kept until monitor_output_off or the end of the connection; with no memory
 * to keep it, that callback ends the monitor and the answer is ERR_MEMORY_ALLOCATION.
 *
 * Call it before host_dispatch_register_unsupported(), which registers the same verb: the first form registered is the
 * one run. Not thread-safe: it sets an unlocked global and uses the table of commands.
 *
 * @param monitor_output called with the instance (the line's word read with atoi) and the symbol, which is the line's
 *     word, not to be kept; it returns 0 once the output is monitored.
 */
void host_dispatch_register_monitor_output(int (*monitor_output)(int instance, const char *symbol));
/** @brief marks that this library has host_dispatch_register_monitor_output_off()
 *
 * A consumer that also builds against a library without it tests this before it calls it.
 */
#define HOST_DISPATCH_HAS_MONITOR_OFF 1
/** @brief registers monitor_output_off (doc/protocol.md, Monitors)
 *
 * The library keeps the outputs the connection monitored with monitor_output. monitor_output_off of one of them calls
 * monitor_output_off and answers what it returns: SUCCESS once the output's monitor has ended, and then the output is
 * no longer kept, or the code that refuses, and then it still is. One of an output the connection did not monitor is
 * answered -543 without calling it, and an instance that is no whole number ERR_INVALID_OPERATION. A monitor ends with
 * the connection that set it: socket_run calls monitor_output_off for each output left when the connection ends, its
 * answer unread. An instance that remove ends takes its monitors with it, and it is not called for them.
 *
 * Call it before host_dispatch_register_unsupported(), which registers the same verb and answers it
 * ERR_INVALID_OPERATION for a host that does not call this. With a NULL monitor_output_off no output is kept, and the
 * verb answers -543. Not thread-safe: it sets unlocked globals and uses the table of commands.
 *
 * @param monitor_output_off called with the instance and the symbol of an output the connection monitored; the symbol
 *     is not to be kept. It returns SUCCESS once the monitor has ended.
 */
void host_dispatch_register_monitor_output_off(int (*monitor_output_off)(int instance, const char *symbol));
/** @brief registers track_info, remote_pages, remote_page_get and param_info
 *
 * The table is not copied: the pointer is kept and read at every line. A NULL info answers the four verbs
 * ERR_INVALID_OPERATION. Call it before host_dispatch_register_unsupported(), which registers the same verbs: the first
 * form registered is the one run. Not thread-safe: it sets an unlocked global and uses the table of commands.
 *
 * @pre info, when not NULL, outlives every protocol_parse.
 * @param[in] info the callbacks, or NULL.
 */
void host_dispatch_register_plugin_info(const host_plugin_info_t *info);
/** @brief what a host gives the curve verbs (doc/protocol.md, Curves), given to host_dispatch_register_curve
 *
 * The values of the parameters come from the backend's param_get.
 */
typedef struct HOST_CURVE_T {
    /** writes in rate the rate the instance runs at, in Hz, and returns SUCCESS, or returns the negative code that
     * refuses the instance (ERR_INSTANCE_NON_EXISTS), which is the verb's answer; NULL answers ERR_INVALID_OPERATION */
    int (*sample_rate)(int instance, double *rate);
} host_curve_t;
/** @brief marks that this library has host_dispatch_register_curve()
 *
 * A consumer that also builds against a library without it tests this before it calls it.
 */
#define HOST_DISPATCH_HAS_CURVE 1
/** @brief registers curve_bind and curve_points
 *
 * They read the parameters with the param_get of the backend host_dispatch_register() was given and the rate with
 * curve, and the points are drawn by omx-dsp's omx_curve_points. With a NULL curve both answer ERR_INVALID_OPERATION;
 * before host_dispatch_register(), or with a backend whose param_get is NULL, curve_bind binds nothing and answers it
 * too. A host that does not call this answers both ERR_INVALID_OPERATION once it calls
 * host_dispatch_register_unsupported(); call it before that one, which registers the same verbs.
 *
 * The table is not copied: the pointer is kept and read at every line. Not thread-safe: it sets an unlocked global and
 * uses the table of commands.
 *
 * @pre curve, when not NULL, outlives every protocol_parse.
 * @param[in] curve the host's entry, or NULL.
 */
void host_dispatch_register_curve(const host_curve_t *curve);
void host_dispatch_register_unsupported(void);
/** @brief the callback of a verb the host does not serve: it sets the response "resp -902" (ERR_INVALID_OPERATION)
 *
 * @pre proto is not NULL, and is the argument of the running callback.
 * @param[out] proto the argument of the callback, as for protocol_response.
 */
void host_dispatch_unsupported_cb(proto_t *proto);
/** @brief sends the feedback line "output_set \<instance\> \<symbol\> \<value\>" (OUTPUT_SET), its NUL included
 *
 * It takes no lock: socket_send_feedback reads the feedback connection unlocked while socket_run opens and closes it.
 *
 * @pre symbol is not NULL.
 * @param instance the instance of the output.
 * @param[in] symbol the symbol of the output, NUL-terminated.
 * @param value the value, written with \%f.
 * @return what socket_send_feedback returns: a positive number once the line is sent, -1 with no feedback client or
 *     when the send fails; -1 too, nothing sent, for a line that does not fit SOCKET_MSG_BUFFER_SIZE bytes.
 */
int host_dispatch_output_set(int instance, const char *symbol, float value);
/** @brief sends the feedback line "remote_pages_changed \<instance\>" (REMOTE_PAGES_CHANGED), its NUL included
 *
 * It takes no lock: socket_send_feedback reads the feedback connection unlocked while socket_run opens and closes it.
 *
 * @param instance the instance whose pages changed.
 * @return what socket_send_feedback returns: a positive number once the line is sent, -1 with no feedback client or
 *     when the send fails.
 */
int host_dispatch_remote_pages_changed(int instance);


/*
************************************************************************************************************************
*           END HEADER
************************************************************************************************************************
*/

#endif
