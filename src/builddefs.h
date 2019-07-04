/* - DownZemAll! - Copyright (C) 2019 Sebastien Vavassori
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BUILD_DEFS_H
#define BUILD_DEFS_H

// Example of __DATE__ string: "Jul 27 2012"
// Example of __TIME__ string: "21:06:19"

/*
 * http://stackoverflow.com/questions/11697820/how-to-use-date-and-time-predefined-macros-in-as-two-integers-then-stri
 * With the following test code, the above works great:
 *
 * printf("%04d-%02d-%02dT%02d:%02d:%02d\n",
 *            BUILD_YEAR, BUILD_MONTH, BUILD_DAY, BUILD_HOUR, BUILD_MIN, BUILD_SEC);
 *
 */

#define COMPUTE_BUILD_YEAR        \
(                                 \
    (__DATE__[ 7] - '0') * 1000 + \
    (__DATE__[ 8] - '0') *  100 + \
    (__DATE__[ 9] - '0') *   10 + \
    (__DATE__[10] - '0')          \
)


#define COMPUTE_BUILD_DAY                                   \
(                                                           \
    ((__DATE__[4] >= '0') ? (__DATE__[4] - '0') * 10 : 0) + \
    (__DATE__[5] - '0')                                     \
)

#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')


#define COMPUTE_BUILD_MONTH     \
(                               \
    (BUILD_MONTH_IS_JAN) ?  1 : \
    (BUILD_MONTH_IS_FEB) ?  2 : \
    (BUILD_MONTH_IS_MAR) ?  3 : \
    (BUILD_MONTH_IS_APR) ?  4 : \
    (BUILD_MONTH_IS_MAY) ?  5 : \
    (BUILD_MONTH_IS_JUN) ?  6 : \
    (BUILD_MONTH_IS_JUL) ?  7 : \
    (BUILD_MONTH_IS_AUG) ?  8 : \
    (BUILD_MONTH_IS_SEP) ?  9 : \
    (BUILD_MONTH_IS_OCT) ? 10 : \
    (BUILD_MONTH_IS_NOV) ? 11 : \
    (BUILD_MONTH_IS_DEC) ? 12 : \
    /* error default */  99     \
)

#define COMPUTE_BUILD_HOUR ((__TIME__[0] - '0') * 10 + __TIME__[1] - '0')
#define COMPUTE_BUILD_MIN  ((__TIME__[3] - '0') * 10 + __TIME__[4] - '0')
#define COMPUTE_BUILD_SEC  ((__TIME__[6] - '0') * 10 + __TIME__[7] - '0')

#define BUILD_DATE_IS_BAD (__DATE__[0] == '?')

#define BUILD_YEAR  ((BUILD_DATE_IS_BAD) ? 99 : COMPUTE_BUILD_YEAR)
#define BUILD_MONTH ((BUILD_DATE_IS_BAD) ? 99 : COMPUTE_BUILD_MONTH)
#define BUILD_DAY   ((BUILD_DATE_IS_BAD) ? 99 : COMPUTE_BUILD_DAY)

#define BUILD_TIME_IS_BAD (__TIME__[0] == '?')

#define BUILD_HOUR  ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_HOUR)
#define BUILD_MIN   ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_MIN)
#define BUILD_SEC   ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_SEC)


#endif // BUILD_DEFS_H
