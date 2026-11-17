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

/** @file mod-host.h
 * @brief The declarations of mod-host.h.
 *
 * The forms of mod-host's commands, as protocol_add_command takes them: \%i takes a whole number, \%f a number, \%s
 * any word, and a last "..." any more words. What each command does and answers is in doc/protocol.md. The library
 * registers them in host-dispatch.h: host_dispatch_register and the registrations beside it serve some, and
 * host_dispatch_register_unsupported answers others ERR_INVALID_OPERATION; cpu_load, help and quit it registers
 * nowhere, a host that serves them registers them.
 */

#ifndef  MOD_HOST_H
#define  MOD_HOST_H


/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/


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

/** @brief the command port a host listens on when it is given none
 *
 * The library does not read it: a host passes it to socket_start.
 */
#define SOCKET_DEFAULT_PORT     5555
/** @brief the size in bytes of the buffer an answer or a feedback line is written in
 *
 * A param_info answer and an output_set line that do not fit it are refused; remote_page_get and curve_points write
 * theirs in four times it.
 */
#define SOCKET_MSG_BUFFER_SIZE  1024

/** @brief the form of add: the uri of the plugin, the instance and, optional, a client name
 *
 * The "..." lets the line end after the instance or go on: host_dispatch_register gives the word after the instance to
 * the backend's add as the client name, or NULL when there is none.
 */
#define EFFECT_ADD           "add %s %i ..."
/** @brief the form of remove: the instance, -1 for every instance */
#define EFFECT_REMOVE        "remove %i"
/** @brief the form of preset_load: the instance and the uri of the preset */
#define EFFECT_PRESET_LOAD   "preset_load %i %s"
/** @brief the form of preset_save: the instance, the name of the preset, the directory and the file name */
#define EFFECT_PRESET_SAVE   "preset_save %i %s %s %s"
/** @brief the form of preset_show: the uri of the preset */
#define EFFECT_PRESET_SHOW   "preset_show %s"
/** @brief the form of connect: the origin port and the destination port */
#define EFFECT_CONNECT       "connect %s %s"
/** @brief the form of disconnect: the origin port and the destination port */
#define EFFECT_DISCONNECT    "disconnect %s %s"
/** @brief the form of bypass: the instance and the bypass value, 1 to bypass it and 0 to process */
#define EFFECT_BYPASS        "bypass %i %i"
/** @brief the form of param_set: the instance, the symbol of the parameter and the value
 *
 * The value is taken as any word: host_dispatch_register reads it with atof.
 */
#define EFFECT_PARAM_SET     "param_set %i %s %s"
/** @brief the form of param_get: the instance and the symbol of the parameter */
#define EFFECT_PARAM_GET     "param_get %i %s"
/** @brief the form of param_monitor: the instance, the symbol, the condition operator and the value */
#define EFFECT_PARAM_MON     "param_monitor %i %s %s %f"
/** @brief the form of patch_get: the instance and the uri of the property */
#define EFFECT_PATCH_GET     "patch_get %i %s"
/** @brief the form of patch_set: the instance, the uri of the property and the value */
#define EFFECT_PATCH_SET     "patch_set %i %s %s"
/** @brief the form of licensee: the instance */
#define EFFECT_LICENSEE      "licensee %i"
/** @brief the form of set_bpm: the beats per minute */
#define EFFECT_SET_BPM       "set_bpm %f"
/** @brief the form of set_bpb: the beats per bar */
#define EFFECT_SET_BPB       "set_bpb %f"
/** @brief the form of monitor: the address, the port and the status, 1 to start and 0 to stop */
#define MONITOR_ADDR_SET     "monitor %s %i %i"
/** @brief the form of monitor_output: the instance and the symbol of the output */
#define MONITOR_OUTPUT       "monitor_output %i %s"
/** @brief the form of monitor_midi_program: the MIDI channel and whether to listen */
#define MONITOR_MIDI_PROGRAM "monitor_midi_program %i %i"
/** @brief the form of midi_learn: the instance, the symbol, the minimum and the maximum */
#define MIDI_LEARN           "midi_learn %i %s %f %f"
/** @brief the form of midi_map: the instance, the symbol, the MIDI channel, the MIDI controller, the minimum and the
 * maximum
 */
#define MIDI_MAP             "midi_map %i %s %i %i %f %f"
/** @brief the form of midi_unmap: the instance and the symbol */
#define MIDI_UNMAP           "midi_unmap %i %s"
/** @brief the form of cc_map: the instance, the symbol and the Control Chain actuator to map to it
 *
 * After the symbol come the device id, the actuator id, the label, the value, the minimum, the maximum, the steps, the
 * extra flags, the unit, the number of scale points and, with the "...", the scale points.
 */
#define CC_MAP               "cc_map %i %s %i %i %s %f %f %f %i %i %s %i ..."
/** @brief the form of cc_value_set: the instance, the symbol and the value */
#define CC_VALUE_SET         "cc_value_set %i %s %f"
/** @brief the form of cc_unmap: the instance and the symbol */
#define CC_UNMAP             "cc_unmap %i %s"
/** @brief the form of cv_map: the instance, the symbol, the name of the source port, the minimum, the maximum and the
 * operational mode
 */
#define CV_MAP               "cv_map %i %s %s %f %f %s"
/** @brief the form of cv_unmap: the instance and the symbol */
#define CV_UNMAP             "cv_unmap %i %s"
/** @brief the form of hmi_map: the instance, the symbol and nine more arguments, of the types the form gives */
#define HMI_MAP              "hmi_map %i %s %i %i %i %i %i %s %f %f %i"
/** @brief the form of hmi_unmap: the instance and the symbol */
#define HMI_UNMAP            "hmi_unmap %i %s"
/** @brief the form of cpu_load, which takes no argument */
#define CPU_LOAD             "cpu_load"
/** @brief the form of load: the name of the file of commands */
#define LOAD_COMMANDS        "load %s"
/** @brief the form of save: the name of the file of commands */
#define SAVE_COMMANDS        "save %s"
/** @brief the form of bundle_add: the path of the bundle */
#define BUNDLE_ADD           "bundle_add %s"
/** @brief the form of bundle_remove: the path of the bundle and the resource */
#define BUNDLE_REMOVE        "bundle_remove %s %s"
/** @brief the form of state_load: the directory, given to the backend's state_load */
#define STATE_LOAD           "state_load %s"
/** @brief the form of state_save: the directory, given to the backend's state_save */
#define STATE_SAVE           "state_save %s"
/** @brief the form of state_tmpdir: the directory */
#define STATE_TMPDIR         "state_tmpdir %s"
/** @brief the form of feature_enable: the name of the feature and whether to enable it */
#define FEATURE_ENABLE       "feature_enable %s %i"
/** @brief the form of transport: whether it rolls, the beats per bar and the beats per minute */
#define TRANSPORT            "transport %i %f %f"
/** @brief the form of transport_sync: the mode */
#define TRANSPORT_SYNC       "transport_sync %s"
/** @brief the form of show_external_ui: the instance */
#define SHOW_EXTERNAL_UI     "show_external_ui %i"
/** @brief the form of output_data_ready, which takes no argument */
#define OUTPUT_DATA_READY    "output_data_ready"
/** @brief the form of track_info: the instance, the name, the color and, optional, the kind
 *
 * The "..." lets the line end after the color or go on; the track_info of host_dispatch_register_plugin_info refuses a
 * line with more than one word after the color.
 */
#define TRACK_INFO           "track_info %i %s %s ..."
/** @brief the form of remote_pages: the instance */
#define REMOTE_PAGES         "remote_pages %i"
/** @brief the form of remote_page_get: the instance and the index of the page */
#define REMOTE_PAGE_GET      "remote_page_get %i %i"
/** @brief the form of param_info: the instance and the symbol of the parameter */
#define PARAM_INFO           "param_info %i %s"
/** @brief the form of help, which takes no argument */
#define HELP                 "help"
/** @brief the form of quit, which takes no argument */
#define QUIT                 "quit"

/** @brief the printf format of the feedback line output_set: the instance, the symbol of the output and its value
 *
 * host_dispatch_output_set sends it.
 */
#define OUTPUT_SET           "output_set %i %s %f"
/** @brief the printf format of the feedback line remote_pages_changed: the instance
 *
 * host_dispatch_remote_pages_changed sends it.
 */
#define REMOTE_PAGES_CHANGED "remote_pages_changed %i"


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
