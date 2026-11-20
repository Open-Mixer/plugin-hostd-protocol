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

/** @file host-backend.h
 * @brief The declarations of host-backend.h.
 */

#ifndef HOST_BACKEND_H
#define HOST_BACKEND_H


/*
************************************************************************************************************************
*           DATA TYPES
************************************************************************************************************************
*/

/** @brief the plugin backend behind the socket protocol, given to host_dispatch_register
 *
 * Each entry serves the verb of its name (doc/protocol.md), and a NULL entry answers ERR_INVALID_OPERATION. An entry
 * returns SUCCESS or a negative code of host-errors.h, and the verb answers "resp" and that number. An instance and a
 * bypass value are the line's words read with atoi, so an entry checks them. A string is a word of the line: it is
 * valid during the call and is not to be kept or freed. The entries are called on the thread that calls
 * protocol_parse.
 */
typedef struct HOST_BACKEND_T {
    /** add: loads the plugin of uri as that instance; client_name is the word after the instance, or NULL when the
     * line has none. A clap: uri whose path the line split at its spaces arrives joined again. */
    int (*add)(const char *uri, int instance, const char *client_name);
    /** remove: ends the instance; a return of 0 or more also drops the curve bindings and the monitors the library
     * keeps for it, all of them when instance is -1 */
    int (*remove)(int instance);
    /** bypass: value is the line's second number */
    int (*bypass)(int instance, int value);
    /** param_set: sets the parameter of that symbol; value is the line's word read with atof, unchecked, so a word
     * that is no number arrives as 0. param_set_many calls it too, once a value, and again to set the ones before a
     * refused one back. */
    int (*param_set)(int instance, const char *symbol, float value);
    /** param_get: writes the parameter's value in value; a return of 0 or more answers that number and the value.
     * param_get_many, param_set_many and the curve verbs read the parameters with it too. */
    int (*param_get)(int instance, const char *symbol, float *value);
    /** preset_load: loads the preset of uri into the instance */
    int (*preset_load)(int instance, const char *uri);
    /** state_save: dir is the line's word */
    int (*state_save)(const char *dir);
    /** state_load: dir is the line's word */
    int (*state_load)(const char *dir);
    /** connect: the two port names, in the order of the line */
    int (*connect)(const char *port_a, const char *port_b);
    /** disconnect: the two port names, in the order of the line */
    int (*disconnect)(const char *port_a, const char *port_b);
    /** the band origin (enum omx_curve_origins, omx_curve.h) a panel description gives one of a plugin's parameters,
       for the curve verbs' marks: NULL, or a symbol this does not set the origin of, answers OMX_CURVE_ORIGINS_USER.
       A foreign plugin's host sets this on no symbol, so curve_points draws it no mark. */
    int (*param_origin)(int instance, const char *symbol, int *origin);
} host_backend_t;


/*
************************************************************************************************************************
*           END HEADER
************************************************************************************************************************
*/

#endif
