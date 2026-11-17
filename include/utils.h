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

/** @file utils.h
 * @brief The declarations of utils.h.
 */

#ifndef UTILS_H
#define UTILS_H


/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <math.h>
#include <float.h>


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

/** @brief turns on the double quotes and their escapes in strarr_split
 *
 * The library is built with it defined; without it strarr_split knows no quote and cuts at the spaces alone.
 */
#define ENABLE_QUOTATION_MARKS


/*
************************************************************************************************************************
*           DATA TYPES
************************************************************************************************************************
*/

/** @brief one received line and the connection it came on, the argument of protocol_parse */
typedef struct MSG_T {
    /** the connection the line came on, a file descriptor; protocol_parse answers on it */
    int sender_id;
    /** the line, NUL-terminated; protocol_parse writes in it */
    char *data;
    /** the length of data in bytes, its NUL not counted; socket_run sets it and the library does not read it */
    size_t data_size;
} msg_t;


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

/** @brief the allocation the library makes its strings and arrays with: malloc(n) */
#define MALLOC(n)       malloc(n)
/** @brief the release of what MALLOC allocated: free(pv) */
#define FREE(pv)        free(pv)


/*
************************************************************************************************************************
*           FUNCTION PROTOTYPES
************************************************************************************************************************
*/

/** @brief cuts a line into its words, in place, and returns the array of them
 *
 * The blanks at the end of the line (space, tab, CR, LF) are cut off first. A word ends at a space, a run of spaces
 * counting as one; a tab ends none. A line that starts with a space has an empty first word, and an empty line is one
 * empty word.
 *
 * With ENABLE_QUOTATION_MARKS, a part in double quotes keeps its spaces and loses the quotes, so "" alone is an empty
 * word. Inside the quotes "" is one quote and \\\\ one backslash, and \\" is a quote inside them or outside. What
 * follows a closing quote, up to the next space, is dropped.
 *
 * The words point into str, which is written: the caller frees the array with FREE, not the words, and keeps str for
 * as long as it reads them.
 *
 * @param[in,out] str the line, NUL-terminated, or NULL.
 * @return the array of words, ended by a NULL; NULL when str is NULL or there is no memory for the array.
 */
char** strarr_split(char *str);
/** @brief counts the strings of an array ended by a NULL
 *
 * @param[in] str_array the array, as strarr_split returns it, or NULL.
 * @return how many strings come before the NULL; 0 for NULL.
 */
uint32_t strarr_length(char **str_array);
// joins a string array in a single string
char* strarr_join(char** const str_array);
/** @brief copies a string into memory allocated with MALLOC
 *
 * The allocation is not checked: with no memory the copy is written through a NULL pointer.
 *
 * @pre str is not NULL.
 * @param[in] str the string, NUL-terminated.
 * @return the copy, which the caller frees with FREE.
 */
char *str_duplicate(const char *str);

// safely compare 2 float values, inlined for speed
static inline
bool floats_differ_enough(float a, float b)
{
    return fabsf(a - b) >= FLT_EPSILON;
}

static inline
bool doubles_differ_enough(double a, double b)
{
    return fabs(a - b) >= DBL_EPSILON;
}

// clamp a value to be within a certain range
static inline
int clamp(int value, int min, int max)
{
    return value < min ? min : (value > max ? max : value);
}

// clamp a value to be within a certain range (float version)
static inline
float clampf(float value, float min, float max)
{
    return value < min ? min : (value > max ? max : value);
}


/** @brief gives Qt a platform that needs no display, in a host that has none
 *
 * When neither DISPLAY nor WAYLAND_DISPLAY is set and not empty (phd_has_display), it sets QT_QPA_PLATFORM to
 * PHD_HEADLESS_QT_PLATFORM (plugin-hostd/protocol.h), unless the environment already holds a QT_QPA_PLATFORM: without
 * one, a Qt-linked plugin aborts as it is instantiated. Not thread-safe: it writes the environment with setenv.
 */
void phd_headless_qt(void);



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
