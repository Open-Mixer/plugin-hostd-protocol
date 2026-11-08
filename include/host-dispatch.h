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

#define HOST_INFO_STRING_SIZE 256
#define HOST_REMOTE_PAGE_SLOTS 8

/* what param_info answers: unit an LV2 unit name ("db", "hz", ...) or "none", scale "linear", "log" or
   "stepped", step 0 for continuous, stable_symbol the name the parameter keeps across hosts */
typedef struct HOST_PARAM_INFO_T {
    const char *unit;
    const char *scale;
    double min, max, def, step;
    const char *stable_symbol;
} host_param_info_t;

/* one page of remote controls: eight parameter symbols, "-" for an empty slot */
typedef struct HOST_REMOTE_PAGE_T {
    uint32_t id;
    char section[HOST_INFO_STRING_SIZE];
    char name[HOST_INFO_STRING_SIZE];
    char slots[HOST_REMOTE_PAGE_SLOTS][HOST_INFO_STRING_SIZE];
} host_remote_page_t;

/* What a host can tell about a plugin. A NULL entry answers ERR_INVALID_OPERATION.
   track_info gets a checked name (may be ""), a color "#RRGGBB" or "-", and a kind "bus", "return", "master"
   or NULL for an input channel. remote_pages answers the page count. */
typedef struct HOST_PLUGIN_INFO_T {
    int (*track_info)(int instance, const char *name, const char *color, const char *kind);
    int (*remote_pages)(int instance);
    int (*remote_page_get)(int instance, int page, host_remote_page_t *page_out);
    int (*param_info)(int instance, const char *symbol, host_param_info_t *info);
} host_plugin_info_t;


/*
************************************************************************************************************************
*           FUNCTION PROTOTYPES
************************************************************************************************************************
*/

/* Registers the backend's verbs, and the bulk verbs over them (doc/protocol.md): param_get_many, the values of several
   parameters of an instance in one line, over param_get; param_set_many, several values set all or nothing, over
   param_get and param_set. */
void host_dispatch_register(const host_backend_t *backend);
/* Registers monitor_output, answered as mod-host does; NULL answers ERR_INVALID_OPERATION.
   Call it before host_dispatch_register_unsupported(). */
void host_dispatch_register_monitor_output(int (*monitor_output)(int instance, const char *symbol));
/* Registers monitor_output_off (doc/protocol.md, Monitors): the library keeps the outputs this connection monitored with
   monitor_output, and monitor_output_off of one of them calls monitor_output_off, which answers SUCCESS once the
   output's monitor has ended (or the negative code that refuses), while one of an output this connection did not
   monitor is answered -543 without calling it. A monitor ends with the connection that set it: it is called for each
   output left when the connection ends, its answer unread; an instance that remove ends takes its monitors with it and
   it is not called for them. Call it after host_dispatch_register_monitor_output() and before
   host_dispatch_register_unsupported(); a host that does not answers monitor_output_off ERR_INVALID_OPERATION. */
#define HOST_DISPATCH_HAS_MONITOR_OFF 1
void host_dispatch_register_monitor_output_off(int (*monitor_output_off)(int instance, const char *symbol));
/* Registers track_info, remote_pages, remote_page_get and param_info.
   Call it before host_dispatch_register_unsupported(). */
void host_dispatch_register_plugin_info(const host_plugin_info_t *info);
/* What a host gives the curve verbs (doc/protocol.md, Curves): the rate an instance runs at, in Hz, answered SUCCESS,
   or the instance's refusal (ERR_INSTANCE_NON_EXISTS). The values come from the backend's param_get. */
typedef struct HOST_CURVE_T {
    int (*sample_rate)(int instance, double *rate);
} host_curve_t;
#define HOST_DISPATCH_HAS_CURVE 1
/* Registers curve_bind and curve_points over the backend host_dispatch_register() was given and the rate curve gives,
   drawn by omx-dsp's omx_curve_points; a host that does not call it answers both ERR_INVALID_OPERATION once it calls
   host_dispatch_register_unsupported(). Call it before host_dispatch_register_unsupported(). */
void host_dispatch_register_curve(const host_curve_t *curve);
void host_dispatch_register_unsupported(void);
void host_dispatch_unsupported_cb(proto_t *proto);
/* Sends OUTPUT_SET on the feedback socket; -1 without a feedback client or for a symbol too long. */
int host_dispatch_output_set(int instance, const char *symbol, float value);
/* Sends REMOTE_PAGES_CHANGED on the feedback socket; -1 without a feedback client. */
int host_dispatch_remote_pages_changed(int instance);


/*
************************************************************************************************************************
*           END HEADER
************************************************************************************************************************
*/

#endif
