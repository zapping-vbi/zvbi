/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#pragma once

#ifndef STRPTIME_H
#define STRPTIME_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Simplified version of strptime
 *
 * Parse the input string p according to the format string fmt and
 * store its results in the structure dt.
 * This implementation supports only a subset of the formats supported
 * by the standard strptime().
 *
 * The supported input field descriptors are listed below.
 * - `%%H`: the hour as a decimal number, using a 24-hour clock, in the
 *   range '00' through '23'
 * - `%%J`: hours as a decimal number, in the range '0' through INT_MAX
 * - `%%M`: the minute as a decimal number, using a 24-hour clock, in the
 *   range '00' through '59'
 * - `%%S`: the second as a decimal number, using a 24-hour clock, in the
 *   range '00' through '59'
 * - `%%Y`: the year as a decimal number, using the Gregorian calendar
 * - `%%m`: the month as a decimal number, in the range '1' through '12'
 * - `%%d`: the day of the month as a decimal number, in the range '1'
 *   through '31'
 * - `%%T`: alias for `%%H:%%M:%%S`
 * - `%%`: a literal `%`
 *
 * @return a pointer to the first character not processed in this function
 *         call. In case the input string contains more characters than
 *         required by the format string the return value points right after
 *         the last consumed input character. In case the whole input string
 *         is consumed the return value points to the null byte at the end of
 *         the string. On failure NULL is returned.
 */
char *strptime(const char *p, const char *fmt, struct tm *dt);

#ifdef __cplusplus
}
#endif

#endif /* STRPTIME_H */
