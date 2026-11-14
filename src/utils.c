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
*           INCLUDE FILES
************************************************************************************************************************
*/

#include "utils.h"
#include "plugin-hostd/protocol.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>


/*
************************************************************************************************************************
*           LOCAL DEFINES
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL CONSTANTS
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL DATA TYPES
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL MACROS
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL GLOBAL VARIABLES
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL FUNCTION PROTOTYPES
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL CONFIGURATION ERRORS
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL FUNCTIONS
************************************************************************************************************************
*/

/* a token's delimiting quotes out, and its escapes in (the quoted-escaped argument, grammar/plugin-hostd-protocol.abnf):
 * a marked \" is one quote and a marked \\ one backslash; the split marks them with \x01 and \x02, which a line never
 * holds (the grammar takes no control character) */
static void parse_quote(char *str)
{
    char *r = str, *w = str;

    for (; *r; r++)
    {
        if (*r == '"')
            continue;
        if (*r == '\x01' && r[1] == '"')
        {
            *w++ = '"';
            r++;
            continue;
        }
        if (*r == '\x02' && r[1] == '\\')
        {
            *w++ = '\\';
            r++;
            continue;
        }
        *w++ = *r;
    }
    *w = '\0';
}

static void trim_spaces(char *str)
{
    char *pstr = str + strlen(str);
    char c;

    /* stop at the start: an empty or all-blank string has nothing before it to look at */
    while (pstr > str)
    {
        c = *(pstr - 1);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') *(--pstr) = 0;
        else break;
    }
}

/*
************************************************************************************************************************
*           GLOBAL FUNCTIONS
************************************************************************************************************************
*/

char** strarr_split(char *str)
{
    uint32_t count;
    char *pstr, **list = NULL;
    const char token = ' ';
    uint8_t quote;

    if (!str) return list;

    trim_spaces(str);

    // count the tokens
    pstr = str;
    quote = 0;
    count = 1;
    while (*pstr)
    {
        if (*pstr == token && quote == 0)
        {
            count++;
            while (*(pstr + 1) && (*(pstr + 1) == token))
                pstr++;
        }
#ifdef ENABLE_QUOTATION_MARKS
        else if (*pstr == '\\' && (*(pstr+1) == '"' || (quote && *(pstr+1) == '\\')))
        {
            // an escaped quote, or inside quotes an escaped backslash: one character, never a delimiter
            pstr++;
        }
        else if (*pstr == '"')
        {
            if (quote == 0)
            {
                quote = 1;
            }
            else
            {
                if (*(pstr+1) == '"')
                    pstr++;
                else
                    quote = 0;
            }
        }
#endif
        pstr++;
    }

    // allocates memory to list
    list = MALLOC((count + 1) * sizeof(char *));
    if (list == NULL) return NULL;

    // fill the list pointers
    pstr = str;
    quote = 0;
    list[0] = pstr;
    count = 0;
    while (*pstr)
    {
        if (*pstr == token && quote == 0)
        {
            *pstr = '\0';
            while (*(pstr + 1) && (*(pstr + 1) == token))
                pstr++;
            list[++count] = pstr + 1;
        }
#ifdef ENABLE_QUOTATION_MARKS
        else if (*pstr == '\\' && *(pstr+1) == '"')
        {
            // an escaped quote: the backslash marks it, unescape() keeps it as a quote of the value
            *pstr++ = '\x01';
        }
        else if (quote && *pstr == '\\' && *(pstr+1) == '\\')
        {
            // inside quotes, an escaped backslash: unescape() keeps one
            *pstr++ = '\x02';
        }
        else if (*pstr == '"')
        {
            if (quote == 0)
            {
                quote = 1;
            }
            else
            {
                if (*(pstr+1) == '"')
                {
                    pstr++;
                }
                else
                {
                    *pstr = '\0';
                    quote = 0;
                }
            }
        }
#endif
        pstr++;
    }

    list[++count] = NULL;

#ifdef ENABLE_QUOTATION_MARKS
    count = 0;
    while (list[count]) parse_quote(list[count++]);
#endif

    return list;
}


uint32_t strarr_length(char** const str_array)
{
    uint32_t count = 0;

    if (str_array) while (str_array[count]) count++;
    return count;
}


char* strarr_join(char **str_array)
{
    uint32_t i, len = strarr_length(str_array);

    if (!str_array) return NULL;

    for (i = 1; i < len; i++)
    {
        (*(str_array[i] - 1)) = ' ';
    }

    return (*str_array);
}

char *str_duplicate(const char *str)
{
    char *copy = MALLOC(strlen(str) + 1);
    strcpy(copy, str);
    return copy;
}

void phd_headless_qt(void)
{
    if (!phd_has_display())
        setenv("QT_QPA_PLATFORM", PHD_HEADLESS_QT_PLATFORM, 0);
}
