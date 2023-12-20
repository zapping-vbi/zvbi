// utf-8 (ü)

// MIT License
// Copyright © 2023 Olaf Simon
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the “Software”), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge, publish, distribute,
// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

/**
 * @file   LibOb_strptime.c
 * @author Olaf Simon
 * @date   06.10.2023
 * @brief  Function implementations of 'strftime' and 'strptime' being portable and implementing time zone parsing.
 *
 * \addtogroup LibOb_Time
 * @{
 * The formatting parameters of the format string are listed within the following table.
 * \anchor TIME_FORMATTING
 * <table>
 * <caption id="TIME_FORMATTING_TABLE">Time formatting parameters</caption>
 * <tr><td>\%Y    <td>year as 4 digits
 * <tr><td>\%y    <td>year as 2 digits, on reading the interpretation is from 1969 til 2068
 * <tr><td colspan="2">
 * <tr><td>\%m    <td>month [01-12]
 * <tr><td>\%1m   <td>month [1-12]
 * <tr><td>\%b    <td>abbreviated month (e.g. Oct)
 * <tr><td>\%B    <td>full month (e.g. October)
 * <tr><td colspan="2">
 * <tr><td>\%d    <td>day (of the month) [01-31]
 * <tr><td>\%1d   <td>day (of the month) [1-31]
 * <tr><td>\%e    <td>%1d
 * <tr><td colspan="2">
 * <tr><td>\%H    <td>hour [00 - 23]
 * <tr><td>\%1H   <td>hour [0 - 23]
 * <tr><td>\%I    <td>hour [00 - 12], On reading operation, AM is assumed if PM property is not specified
 * <tr><td>\%1I   <td>hour [0 - 12]
 * <tr><td>\%p    <td>PM or AM, if this property is not found during scanning AM is assumed.
 * <tr><td colspan="2">
 * <tr><td>\%M    <td>minute [00 - 59]
 * <tr><td>\%1M   <td>minute [0 - 59]
 * <tr><td colspan="2">
 * <tr><td>\%S    <td>second [00 - 59]
 * <tr><td>\%1S   <td>second [0 - 59]
 * <tr><td colspan="2">
 * <tr><td>\%U    <td>UTC / STD / DST where UTC indicates +0300 is UTC offset, STD standard time and +0300 is geographic time zone, DST is daylight saving time and +0300 is geographic time zone
 * <tr><td>\%z    <td>UTC time offset, e.g. +03:00 or geographic time zone (depending on tm_isdst)
 * <tr><td>\%Z    <td>Time zone abbreviation, e.g. (CEST for 'central european summer time')
 * <tr><td colspan="2">
 * <tr><td>\%a    <td>abbreviated week day name [not evaluated on reading operation], e.g. 'Fri' or 'So'
 * <tr><td>\%A    <td>full week day name [not evaluated on reading operation]
 * <tr><td>\%j    <td>day of year [not evaluated on reading operation]
 * <tr><td>[\%V]  <td>[not implemented yet] calendar week [00-53], 0=remaining week of previous year [not evaluated on reading operation, as can be calculated on other information], (week specified according to ISO 8601 - weed from mo to su, 1st week of the year includes first Thursday and 4th of January) [not evaluated on reading operation. Can be calculated on other information]
 * <tr><td>[\%1V] <td>[not implemented yet] calendar week [0, 1 ..., 53]
 * <tr><td colspan="2">
 * <tr><td>%%   = <td>'%' character
 * </table>
 *
 * The list of selected time zones, exactly one for each geographic zone as standard and daylight time, are listed in the following table.
 * \anchor TIME_ZONEABBREVIATIONS
 * <table>
 * <caption id="TIME_ZONEABBREVIATIONS_TABLE">Time zone abbreviations</caption>
 * <tr><th>Geograqhic zone time <th>Standard time   <th>Daylight saving time    <th> Zone name
 * <tr><td>-12                  <td>"IDLW"          <td>"DST_12"                <td> International Date Line West time zone
 * <tr><td>-11                  <td>"SST"           <td>"SDT"                   <td> Samoa Standard Time
 * <tr><td>-10                  <td>"HST"           <td>"HDT"                   <td> Hawaii–Aleutian Standard Time
 * <tr><td>-9                   <td>"AKST"          <td>"AKDT"                  <td> Alaska Standard Time
 * <tr><td>-8                   <td>"PST"           <td>"PDT"                   <td> Pacific Standard Time (North America)
 * <tr><td>-7                   <td>"MST"           <td>"MDT"                   <td> Malaysia Standard Time
 * <tr><td>-6                   <td>"EAST"          <td>"EASST"                 <td> Easter Island Standard Time
 * <tr><td>-5                   <td>"EST"           <td>"EDT"                   <td> Eastern Standard Time (North America)
 * <tr><td>-4                   <td>"CLT"           <td>"CLST"                  <td> Chile Standard Time
 * <tr><td>-3                   <td>"BRT"           <td>"BRST"                  <td> Brasília Time
 * <tr><td>-2                   <td>"GST"           <td>"DST_2"                 <td> Gulf Standard Time
 * <tr><td>-1                   <td>"EGT"           <td>"EGST"                  <td> Eastern Greenland Time
 * <tr><td>0                    <td>"GMT"           <td>"BST"                   <td> Greenwich Mean Time / British Summer Time
 * <tr><td>1                    <td>"CET"           <td>"CEST"                  <td> Central European Time
 * <tr><td>2                    <td>"CAT"           <td>"EEST"                  <td> Central Africa Time / Eastern European Summer Time
 * <tr><td>3                    <td>"EAT"           <td>"DST3"                  <td> East Africa Time
 * <tr><td>4                    <td>"GST"           <td>"DST4"                  <td> Gulf Standard Time
 * <tr><td>5                    <td>"YEKT"          <td>"DST5"                  <td> Yekaterinburg Time
 * <tr><td>6                    <td>"BIOT"          <td>"DST6"                  <td> British Indian Ocean Time
 * <tr><td>7                    <td>"ICT"           <td>"DST7"                  <td> Indochina Time
 * <tr><td>8                    <td>"CST"           <td>"CHOST"                 <td> China Standard Time / Choibalsan Summer Time
 * <tr><td>9                    <td>"JST"           <td>"DST9"                  <td> Japan Standard Time
 * <tr><td>10                   <td>"AET"           <td>"AEDT"                  <td> Australian Eastern Time
 * <tr><td>11                   <td>"SBT"           <td>"DST11"                 <td> Solomon Islands Time
 * <tr><td>12                   <td>"NZST"          <td>"NZDT"                  <td> New Zealand Standard Time
 * <tr><td colspan="4">
 * <tr><td>13                   <td>"TOT"           <td>                        <td> Tonga Time
 * <tr><td>14                   <td>"LINT"          <td>                        <td> Line Islands Time
 * <tr><td colspan="4">
 * <tr><td>-9:30                <td>"MIT"           <td>                        <td> Marquesas Islands Time
 * <tr><td>-3:30                <td>"NST"           <td>"NDT"                   <td> Newfoundland Standard Time
 * <tr><td>3:30                 <td>"IRST"          <td>"IRDT"                  <td> Iran Standard Time
 * <tr><td>4:30                 <td>"AFT"           <td>                        <td> Afghanistan Time
 * <tr><td>5:30                 <td>"IST"           <td>                        <td> Indian Standard Time
 * <tr><td>5:45                 <td>"NPT"           <td>                        <td> Nepal Time
 * <tr><td>6:30                 <td>"MMT"           <td>                        <td> Myanmar Standard Time
 * <tr><td>8:45                 <td>"CWST"          <td>                        <td> Central Western Standard Time (Australia) unofficial
 * <tr><td>9:30                 <td>"ACST"          <td>"ACDT"                  <td> Australian Central Standard Time
 * <tr><td>12:45                <td>"CHAST"         <td>"CHADT"                 <td> Chatham Standard Time
 * </table>
 *
 * other numeric zones and UTC are "abbreviated" as e.g. "STD#+04:15"\n
 * \n
 * The abbreviation 'DSTx' is used in case no daylight saving time is existing within that time zone!\n
 * \n
 * As time zones are defined by either a number (hours of standard time deviation from UTC) or by a name,
 * it is unavoidable to define a biunique convertible relationship of time zone number and names. This results
 * in a well defined selection of names out of the large variety of time zone names derived from different languages.
 * There can only be one zone name printed for a given time zone number at least as standard option, as long as the computer system locale
 * is not used (due to the reasons described before). For that reason a selection of exactly one zone name abbreviation
 * for each time zone number is necessary.\n
 * I tryed my best to select time zone abbreviations which represent each continent and as many population as possible.
 * Nevertheless, a fully 'fair' and 'undescriminating' choice from the viewpoint of everbody is impossible and there
 * are even zone names I hesitated to choose. There are also countries represented due to their geographic position
 * and not due to large populations. It has also been a criteria having paired standard and daylight saving / summer time abbreviations.\n
 * Maybe, just my intention being neutral and the scientific need of harmonization
 * leeds to an acceptance all over the world.\n
 * \n
 * The 'locale' is intentionally not being used, as the language strings are not well defined and even differ between operation
 * systems. International exchange is difficult (and not intended) as e.g. non ASCII characters and country specific expressions
 * are being used.
**/

// For later extension (ms):
// SYSTEMTIME st;
// GetSystemTime(&st);
// vsnprintf_s

#include <sec_api/string_s.h>
#include <stdio.h>
#include "LibOb_strptime.h"

#define ZONE_SIZE 67                            ///< Number of used named time zones.
#define LibCpp_SECONDSPERHOUR 3600              ///< Constant.

/* enLanguage dependent constant definitions ------------------------------------------------------------------------------ */

#define enLanguageSize 2                        ///< Number of implemented languages.

const char* dayNames[enLanguageSize][8] =       ///< Day names.
{
    {"", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"},
    {"", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag", "Sonntag"}
};

const char* dayNameAbbreviations[enLanguageSize][8] =   ///< Day name abbreviations.
{
    {"", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"},
    {"", "Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"}
};

const char* monthNames[enLanguageSize][13] =            ///< Month names.
{
    {"", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"},
    {"", "Januar", "Februar", "Maerz", "April", "Mai", "Juni", "July", "August", "September", "Oktober", "November", "Dezember"}
};

const char* monthNameAbbreviations[enLanguageSize][13] =    ///< Month name abbreviations
{
    {"", "Jan", "Feb", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"},
    {"", "Jan", "Feb", "Mai", "Jun", "Jul", "Aug", "Sepr", "Okt", "Nov", "Dez"}
 };

const char* dstNames[3] = {"UTC", "STD", "DST"};                       ///< Strings used for dst codes

/** List of time zone abbreviations */
const stZoneAbbreviation zones[ZONE_SIZE] =
{
    {"IDLW", 0, {-12,0}}, {"DST_12", 1, {-12,0}}, {"SST", 0, {-11,0}}, {"SDT", 1, {-11,0}}, {"HST", 0, {-10,0}}, {"HDT", 1, {-10,0}},
    {"AKST", 0, {-9,0}}, {"AKDT", 1, {-9,0}}, {"PST", 0, {-8,0}}, {"PDT", 1, {-8,0}}, {"MST", 0, {-7,0}}, {"MDT", 1, {-7,0}},
    {"EAST", 0, { -6,0}}, {"EASST", 1, {-6,0}}, {"EST", 0, {-5,0}}, {"EDT", 1, {-5,0}}, {"CLT", 0, {-4,0}}, {"CLST", 1, {-4,0}},
    {"BRT", 0, {-3,0}}, {"BRST", 1, {-3,0}}, {"GST", 0, {-2,0}}, {"DST_2", 1, {-2,0}}, {"EGT", 0, {-1,0}}, {"EGST", 1, {-1,0}},
    {"GMT", 0, {0,0}}, {"BST", 1, {0,0}}, {"CET", 0, {1,0}}, {"CEST", 1, {1,0}}, {"CAT", 0, {2,0}}, {"EEST", 1, {2,0}},
    {"EAT", 0, {3,0}}, {"DST3", 1, {3,0}}, {"GST", 0, {4,0}}, {"DST4", 1, {4,0}}, {"YEKT", 0, {5,0}}, {"DST5", 1, {5,0}},
    {"BIOT", 0, {6,0}}, {"DST6", 1, {6,0}}, {"ICT", 0, {7,0}}, {"DST7", 1, {7,0}}, {"CST", 0, {8,0}}, {"CHOST", 1, {8,0}},
    {"JST", 0, {9,0}}, {"DST9", 1, {9,0}}, {"AET", 0, {10,0}}, {"AEDT", 1, {10,0}}, {"SBT", 0, {11,0}}, {"DST11", 1, {11,0}},
    {"NZST", 0, {12,0}}, {"NZDT", 1, {12,0}}, {"TOT", 0, {13,0}}, {"NZDT", 1, {13,0}}, {"LINT", 0, {14,0}}, {"IRST", 0, {3,30}},
    {"ACST", 0, {9,30}}, {"ACDT", 1, {9,30}}, {"CHAST", 0, {12,45}}, {"CHADT", 1, {12,45}}, {"NST", 0, {-3,30}}, {"NDT", 1, {-3,30}},
    {"IRDT", 1, {3,30}}, {"AFT", 0, {4,30}}, {"IST", 0, {5,30}}, {"NPT", 0, {5,45}}, {"MMT", 0, {6,30}}, {"CWST", 0, {8,45}},
    {"MIT", 0, {-9,30}}
};                                                                      ///< List of zone names, dst and zone data.

/* Constants ------------------------------------------------------------------------------ */
const uint8_t DAYSOFMONTH[2][13] =                                                          ///< List of days within a month
    {{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
     {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};
enum enLanguage LibOb_GLOBALLANGUAGE = enLanguage_en_US;                                    ///< Global language variable

const tm_t tm_Ini = {0, 0, 0, 0, 0, 0, 0, 0, 0};  ///< Initializer for struct tm variables. The order of elements is undefined, thus tm_isdst cannot be initialized to -1
const tm_t tm_Invalid = {INT_INVALID, INT_INVALID, INT_INVALID, INT_INVALID, INT_INVALID, INT_INVALID, INT_INVALID, INT_INVALID, INT_INVALID};  ///< Initializer for invalid indication of struct tm variables.
const stTimeZone stTimeZone_Ini = {0, 0};                                       ///< Initialization for stTimeZone
const stTimeZone stTimeZone_Invalid = {0x80, 0};                                ///< Initialization for stTimeZone
const stZoneAbbreviation stZoneAbbreviation_Ini = {0, 0, {0, 0}};               ///< Initialization for stZoneAbbreviation
const stZoneAbbreviation stZoneAbbreviation_Invalid = {"INVAL", INT8_INVALID, {INT8_INVALID, 0}};    ///< Error code of stZoneAbbreviation
const char* unnamedZone = "None";                                               ///< Sting used as name in stZoneAbbreviation in case no time zone name exists.

//! @cond Doxygen_Suppress
int8_t      scanDst(const char* dstStr, int startsWith);
const char* scanTimeZone(const char* name, stTimeZone* pTimeZone);
const char* scanZone(const char* name, stZoneAbbreviation* pResultZone);
const char* scanTime(const char* source, struct tm* tp, stTimeZone* pTimeZone);
const char* scanDate(const char* source, struct tm* tp, stTimeZone* pTimeZone);
const char* scanCalendar(const char* source, struct tm* tp, stTimeZone* pTimeZone);
//! @endcond

/* General string related scan functions -------------------------------------------------- */

/**
 * @brief Limits the language index for this module
 * The language to be used is defined by a Pointer to an enum enLanguage.
 * This way you can define a global language variable that might be changed during run time.
 * When zero pointer is used, the define LibOb_GLOBALLANGUAGE which is enLanguage_en_US is used.
 * You can change this at compile time by redefining LibOb_GLOBALLANGUAGE within LibConfig.h.
 * In case the requested language is not implemented enLanguage_en_US is used.
 * @param pLanguage Pointer to a enum enLanguage
 * @return An enum enLanguage limited to the modules language implementations (enLanguageSize)
 */
enum enLanguage enLanguageIndex(enum enLanguage* pLanguage)
{
    if (!pLanguage) return LibOb_GLOBALLANGUAGE;
    if (*pLanguage>enLanguageSize) return enLanguage_en_US;
    return *pLanguage;
}

/**
 * @brief Checks a character for being used in number strings.
 * Returns 1 in case 'character' is a digit or '.' ',' '+' '-'.\n
 * The characters 'e' or 'E' are not indicated being a number. This decision would require access to leading or lagging
 * characters and thus has to be checked by the calling code.
 * (Following the isdigit() standard c++ function.)
 * @param character Character to be checked.
 * @return 1 for true
 */
int isnumber(unsigned char character)
{
    return character!=0 && (isdigit(character) || character==',' || character=='.'  || character=='+' || character=='-');
}

/**
 * @brief Scans a character string until a non white space character is found
 * @param source Character string to be scanned.
 * @return Pointer to first character not being white space
 */
const char* scipSpace(const char* source)
{
    while (isspace(*source)) source++;
    return source;
}

/**
 * @brief Scans a character string until a non digit character is found
 * Accepts a sign '+' or '-' as digit at the first position. Used to scan past a integer number.
 * @param source Character string to be scanned.
 * @return Pointer to first character not being an digit
 */
const char* scipDigits(const char* source)
{
    if (*source=='+' || *source=='-') source++;
    while (isdigit(*source)) source++;
    return source;
}

/**
 * @brief Scans a character string until a digit or letter character is found
 * @param source Character string to be scanned.
 * @param pType Pointer to an integer to store the value 1 in case a digit or 2 in case a letter is being detected. 0 if end of string is found. May be set to 0.
 * @return Pointer to first character being a digit or character letter or end of string (0);
 */
const char* scipNonLetters(const char* source, int* pType)
{
    while (*source != 0 && isnumber(*source) && !isdigit(*source)) source++;
    while (*source != 0)
    {
        if (isnumber(*source))
        {
            if (pType) *pType=1;
            return source;
        }
        if (isupper(*source) || islower(*source))
        {
            if (pType) *pType=2;
            return source;
        }
        source++;
    }
    if (pType) *pType=0;
    return source;
}

/**
 * @brief Finds and converts a number string to a unsigned integer
 * If the number has a leading '+' and parameter allowPlus is false the number is not accepted as unsigned!
 * A lagging ',', 'e' or 'E' assumes a float, thus the number is not accepted as integer.
 * A lagging dot is only accepted if the parameter 'allowLaggingDot' is set.
 * The lagging dot is not being consumed, the returned pointer will point to that dot
 * in order to allow following scan functions to know if a dot has been detected.
 * This way a 1. may be interpreted in the sense of 1st or as a float 1.0.
 * (This function following the sscanf() standard c++ function.)
 * @param source String containing the unsigned integer string.
 * @param result Pointer to unsigned integer to store the result.
 * @param scip If set, leading characters not being sign or digits are scipped.
 * @param allowPlus A number string beginning with a '+' character is accepted as unsigned int.
 * @param allowLaggingDot A lagging '.' character is allowed and not interpreted to indicate a float value.
 * @return Character pointer to the first sign not being consumed by the number conversion or nullptr if the converion failed.
 */
const char* scanUint(const char* source, unsigned int* result, int scip, int allowPlus, int allowLaggingDot)
{
    char previousChar = *source;
    while (scip && !isdigit(*source)) {previousChar = *source; source++;}
    if (*source && (previousChar!='+' || allowPlus) && previousChar!='-')
    {
        int res = sscanf_s(source, "%u", result);
        if (res!=1) return 0;
        source = scipDigits(source);
        if (*source==0)
            return source;
        if (*source==',' || *source=='e' || *source=='E')
            return 0;
        if (*source=='.')
        {
            if (!allowLaggingDot)
                return 0;
            return source;
        }
        return source;
    }
    return 0;
}

/**
 * @brief Finds and converts a number string to an integer
 * If 'requiredPlus' is set number without leading '+' are interpreted as unsigned int and are not accepted as being a signed number!
 * A lagging ',', 'e' or 'E' assumes a float, thus the number is not accepted as integer.
 * A lagging dot is only accepted if the parameter 'allowLaggingDot' is set and the dot is not followed by a digit.
 * The lagging dot is not being consumed, the returned pointer will point to that dot
 * in order to allow following scan functions to know if a dot has been detected.
 * This way a 1. may be interpreted in the sense of 1st or as a float 1.0 .
 * (This function follows the sscanf() standard c++ function.)
 * @param source String containing the unsigned integer string.
 * @param result Pointer to unsigned integer to store the result.
 * @param scip If set, leading characters not being sign or digits are scipped.
 * @param requiredPlus If set only numbers starting with plus are interpreded as positive signed integer
 * @param allowLaggingDot A lagging '.', ',', 'e' or 'E' character is allowed and not interpreted to indicate a float value.
 * @return Character pointer to the first sign not being consumed by the number conversion or nullptr if the converion failed.
 */
const char* scanInt(const char* source, int* result, int scip, int requiredPlus, int allowLaggingDot)
{
    char previousChar = *source;
    while (scip && !isdigit(*source)) {previousChar = *source; source++;}
    if (*source && (previousChar=='-' || previousChar!='+' || !requiredPlus))
    {
        int res = sscanf_s(source, "%d", result);         // atoi() is not standard C
        if (res!=1) return 0;
        source = scipDigits(source);
        if (*source==0)
            return source;
        if (*source=='.' || *source==',' || *source=='e' || *source=='E')
        {
            if (!allowLaggingDot)
                return 0;
            return source;
        }
        return source;
    }
    return 0;
}

/**
 * @brief Scans source for an expression and copies it to destination
 * @param source
 * @param destination If destination is zero, the scan is executed but the expression is not copied.
 * @param destinationSize
 * @param scip If set, leading characters not being a begin of an expression (e.g. white space, numbers) are scipped.
 * @param allowNumber If set, after the first letter character following numbers are included into the expression
 * @return First character not part of the expression (or not being scanned due to undersized destination). Zero in case no expression is found.
 */
const char* scanExpression(const char* source, char* destination, size_t destinationSize, int scip, int allowNumber)
{
    size_t cnt=0;
    while (scip && !(isupper(*source) || islower(*source))) source++;
    if (*source!=0)
    {
        while ((isupper(*source) || islower(*source) || *source=='_' || (isdigit(*source) && allowNumber)) && cnt<destinationSize-2)
        {
            *destination = *source;
            destination++;
            source++;
            cnt++;
        }
        *destination = 0;
        return source;
    }
    return 0;
}

/**
 * @brief Copies characters from format string beginning at formatBegin to destinationBegin until the next format escape sign '%' is found.
 * @param formatBegin Pointer to position in format string
 * @param pDestinationBegin Pointer to a char* containing the start pointer for copying.
 * @param destinationEnd Pointer to end of buffer, reserved for 0 termination.
 * @return Pointer to format character (character after '%') or to zero if no format escape is found til end of string. *pDestinationBegin is set one byte after the last copy operation and 0 is written to the destination at that position.
 */
const char* strcpyfmt(const char* formatBegin, char** pDestinationBegin, const char* destinationEnd)
{
    while (*formatBegin!=0 && *formatBegin!='%' && *pDestinationBegin<destinationEnd-1)
    {
        **pDestinationBegin = *formatBegin;
        *pDestinationBegin += 1;
        formatBegin++;
    }
    if (*formatBegin=='%')
        formatBegin++;
    **pDestinationBegin = 0;
    return formatBegin;
}

/**
 * @brief Checks characters from format string beginning at formatBegin being identical with the source string starting at sourceBegin until the next format escape sign '%' is found.
 * @param formatPosition Pointer to position within the format string.
 * @param ppSourcePosition Pointer to a pointer indicating the position within the source string.
 * @return Pointer to the format character (character after '%'). It points to zero in case the end of the format string is reached. The return value is zero in case the source content differs from the format content. *pSourcePosition is set to the position at which the next formatted data begins.
 */
const char* strchkfmt(const char* formatPosition, const char** ppSourcePosition)
{
    const char* sourceEnd;
    if (!ppSourcePosition) return 0;
    sourceEnd = *ppSourcePosition + strlen(*ppSourcePosition);
    while (*formatPosition!=0 && *formatPosition!='%' && *ppSourcePosition<sourceEnd)
    {
        if (*formatPosition != **ppSourcePosition) return 0;
        *ppSourcePosition += 1;
        formatPosition++;
    }
    if (*formatPosition=='%')
        formatPosition++;
    return formatPosition;
}

/**
 * @brief Scans (checks) 'string' to begin with the 'compare' string
 * @param string    Character string to be checked
 * @param compare   Reference string the 'string' should start with
 * @return          0 if the check failes, pointer to the character following the compare sequence within 'string' (may point to a zero value in case 'string' and 'compare' are equal)
 */
const char* strstartswith(const char* string, const char* compare)
{
    while (*string!=0 && *compare!=0)
    {
        if (*string!=*compare)
            return 0;
        string += 1;
        compare += 1;
    }
    if (*compare!=0)
        return 0;
    return string;
}

/* Time related functions -------------------------------------------------- */

/**
 * @brief Returns the dst code (as used in 'struct tm') from a given string 'UTC', 'STD', 'DST' or time zone abbreviation e.g. 'CET'.
 * The time zone abbreviation 'UTC#+01:15' is valid, also.
 * @param dstStr String indication the dst value as given in the dstString array.
 * @param startsWith If set, only the first characters are evaluated.
 * @return dst Code of dst or 0x80 in case of failure
 */
int8_t LibOb_dst(const char* dstStr, int startsWith)
{
    stZoneAbbreviation zone;
    char buffer[8];
    int8_t dst;
    if (dstStr==0) return 0;
    if (*dstStr==0) return 0;
    dst = scanDst(dstStr, startsWith);
    if (dst != (int8_t)0x80)
        return dst;
    strcpy_s(buffer, 8, dstStr);
    scanZone(buffer, &zone);
    if (zone.dst == (int8_t)0x80) return 0;
    return zone.dst;
}

/**
 * @brief Returns the time zone struct and dst code from a given time zone string or abbreviation string e.g. 'CET'.
 * In this case only the first three characters are evaluated.
 * @param timeZoneStr Time zone abbreiation string like 'CET' or 'UTC#+01:15'.
 * @param pDst [output] If set, the output of the dst corresponding to the time zone string.
 * @return Time zone in numeric format, stTimeZone_INVALID on failure.
 */
stTimeZone LibOb_timeZone(const char* timeZoneStr, int8_t* pDst)
{
    stZoneAbbreviation zone;
    if (timeZoneStr==0) return stTimeZone_Ini;
    if (*timeZoneStr==0) return stTimeZone_Ini;
    scanZone(timeZoneStr, &zone);
    if (zone.dst == (int8_t)0x80) return stTimeZone_Ini;
    if (pDst) *pDst = zone.dst;
    return zone.zone;
}

/**
 * @brief Finds a zone name (e.g. CET) out of the 'zones' array.
 * This function might fail in case not entry in zones matches 'timeZone'.\n
 * See function createZoneName to avoid this behavior.
 * @param timeZone
 * @param dst
 * @return The zone name or zero in case no name is existing.
 */
const char* findZoneName(stTimeZone timeZone, int8_t dst)
{
    if (dst<0) return "UTC";
    for (int i=0; i<ZONE_SIZE; i++)
        if (zones[i].zone.hours == timeZone.hours && zones[i].zone.minutes == timeZone.minutes && zones[i].dst == dst)
            return zones[i].name;
    return 0;
}

/**
 * @brief Finds the zone name to a given time zone or generates an alternative string expression
 * If no time zone name for the given time zone struct is existing the extended zone expression
 * is generated, e.g. 'UTC#+02:15'
 * @param buffer
 * @param bufferSize
 * @param timeZone
 * @param dst
 */
void createZoneName(char* buffer, size_t bufferSize, stTimeZone timeZone, int8_t dst)
{
    const char* zoneAbbr = findZoneName(timeZone, dst);
    *buffer = 0;
    if (dst<-1 || dst>1) return;
    if (zoneAbbr)
        strcpy_s(buffer, bufferSize, zoneAbbr);
    else
    {
        sprintf_s(buffer, bufferSize, "%s#%+03d:%02u", dstNames[dst+1], timeZone.hours, timeZone.minutes);
    }
}

/**
 * @brief Returns the dst code (as used in 'struct tm') from a given string 'UTC', 'STD' or 'DST'.
 * The function works in the sense of 'strstartwith' if parameter 'startWith' is set
 * In this case only the first three characters are evaluated.
 * @param dstStr String indication the dst value as given in the dstString array.
 * @param startsWith If set, only the first characters are evaluated.
 * @return dst Code of dst or 0x80 in case of failure
 */
int8_t scanDst(const char* dstStr, int startsWith)
{
    char buffer[8];
    buffer[0] = dstStr[0];
    buffer[1] = dstStr[1];
    buffer[2] = dstStr[2];
    buffer[3] = dstStr[3];
    buffer[4] = 0;
    if (startsWith) buffer[3] = 0;
    if (strcmp(buffer, dstNames[0])==0) return -1;
    if (strcmp(buffer, dstNames[1])==0) return 0;
    if (strcmp(buffer, dstNames[2])==0) return 1;
    return 0x80;
}

/**
 * @brief Finds the time zone data from a given string like '+01', '+0115', '+01:15'
 * @param name Formatted time zone number string.
 * @param pTimeZone stTimeZone as a result.
 * @return Pointer to a character pointer to store the position of the first character not being consumed. Zero in case of failure.
 */
const char* scanTimeZone(const char* name, stTimeZone* pTimeZone)
{
    if (pTimeZone) *pTimeZone = stTimeZone_Invalid;
    const char* pos = name;
    int len;
    int hours = 0;
    unsigned int minutes = 0;
    if ((len=sscanf_s(pos, "%d", &hours)))
    {
        pos = scipDigits(pos);
        if (len >= 4)
        {
            if (pTimeZone)
            {
                pTimeZone->hours = hours/100;
                pTimeZone->minutes = hours%100;
            }
            return pos;
        }
        else
        {
            if (pTimeZone) pTimeZone->hours = hours;
            if (*pos == ':')
            {
                pos++;
                if ((len=sscanf_s(pos, "%u", &minutes)))
                {
                    pos = scipDigits(pos);
                    if (len>0 && pTimeZone)
                    {
                        pTimeZone->minutes = minutes;
                    }
                }
                return pos;
            }
        }
    }
    return 0;
}

/**
 * @brief Finds the stZoneAbbreviation from a given zone abbreviation string.
 * The time zone name might be either one of the abbreviations defined in the array 'zones'
 * like 'CET' or and extended expression like 'UTC#+02:15' or 'UCT +02:15'.
 * @param name Time zone abbreviation string or extended string.
 * @param pResultZone stZoneAbbreviation of the given zone abbreviation. In case of failure the dst entry is set to 0x80.
 * @return Pointer to a character pointer to store the position of the first character not being consumed. Zero in case of failure.
 */
const char* scanZone(const char* name, stZoneAbbreviation* pResultZone)
{
    int i;
    char buffer[16];
    const char* scanPosition;
    const char* scanResult;
    if (pResultZone) *pResultZone = stZoneAbbreviation_Invalid;
    scanResult = scanExpression(name, buffer, 16, 0, 0);
    if (!scanResult)
    {
        if (pResultZone) *pResultZone = stZoneAbbreviation_Invalid;
        return 0;
    }
    scanPosition = scanResult;
    for (i=0; i<ZONE_SIZE; i++)
        if (strcmp(buffer, zones[i].name)==0)
        {
            if (pResultZone) *pResultZone = zones[i];
            return scanPosition;
        }
    int8_t dst = scanDst(name, 1);
    if (dst!=INT8_INVALID)
    {
        int type = 0;
        if (pResultZone) pResultZone->dst = dst;
        scanPosition = scipNonLetters(scanPosition, &type);
        if (type == 1)
        {
            stTimeZone tz;
            scanResult = scanTimeZone(scanPosition, &tz);
            if (scanResult)
            {
                scanPosition = scanResult;
                pResultZone->zone = tz;
            }
        }
        return scanPosition;
    }
    if (pResultZone) *pResultZone = stZoneAbbreviation_Invalid;
    return 0;
}

/**
 * @brief dayNumber Finds the number of day within the week from a day string (full name or abbreviation).
 * @param dayName Day string.
 * @return Day nummber within the week [1..7] = Mon-Sun, 0 if unsuccessful.
 */
unsigned int findDay(const char* dayName)
{
    int i,j;
    for (i=0; i<enLanguageSize; i++)
        for (j=1; j<=7; j++)
        {
            if (strcmp(dayName, dayNames[i][j])==0) return j;
        }
    return 0;
}

/**
 * @brief dayNumber Finds the number of month within the year from a month string (full name or abbreviation).
 * @param monthName Month string.
 * @return Number of month within the year [1..12] = Jan-Dec, 0 if unsuccessful.
 */
unsigned int findMonth(const char* monthName)
{
    int i,j;
    for (i=0; i<enLanguageSize; i++)
        for (j=1; j<=12; j++)
        {
            if (strcmp(monthName, monthNames[i][j])==0) return j;
        }
    return 0;
}

/**
 * @brief Checks a struct tm to contain valid entities.
 * The parameter tmCheckConfig defines which entries shall be checked. Entries of 'pTm' are checked in case the corresponding
 * entry in 'tmCheckConfig' is set to INT_INVALID. Set 'tmCheckConfig' to 'tm_Invalid' in order to check all entries. Use
 * 'tm_Ini' and set 'isDuration' in case to interprete 'pTm' as duration. In this case all unread entries being invalid
 * are set to values being converted to a unix time of zero. In case special entries are expected to be read, these
 * entries can be set to 'INT_INVALID' in 'tmCheckConfig.\n
 * In case of an invalid entry in 'pTm' but a valid entry in 'tmCheckConfig' the entry of 'tmCheckConfig' is copied to
 * 'pTm'. Entries of 'pTm' carrying numbers appart from the allowed range are set to INT_INVALID to allow the user
 * to identify false enties. The informative entries tm_wday and tm_yday are newly calculated independent of their settings, in case all
 * other entries are valid.
 * @param pTm Pointer to the struct tm to be tested.
 * @param tmCheckConfig Control of entries to be checked.
 * @param isDuration Checks only entities valid for durations and does not limit the entry 'tm_mday' carrying day durations.
 * @return 1 for valid
 */
int LibOb_checkStructTm(struct tm* pTm, struct tm tmCheckConfig, int isDuration)
{
    int valid = 1;
    int leap = 1;
    if (isDuration)
    {
        if (tmCheckConfig.tm_year!=INT_INVALID) tmCheckConfig.tm_year = -1900;
        if (tmCheckConfig.tm_mon!=INT_INVALID) tmCheckConfig.tm_mon = 0;
        if (tmCheckConfig.tm_mday!=INT_INVALID) tmCheckConfig.tm_mday = 1;
        if (tmCheckConfig.tm_hour!=INT_INVALID) tmCheckConfig.tm_hour = 0;
        if (tmCheckConfig.tm_min!=INT_INVALID) tmCheckConfig.tm_min = 0;
        if (tmCheckConfig.tm_sec!=INT_INVALID) tmCheckConfig.tm_sec = 0;
        if (tmCheckConfig.tm_isdst!=INT_INVALID) tmCheckConfig.tm_isdst = 0;
    }
    if (pTm->tm_year == INT_INVALID)
    {
        if (tmCheckConfig.tm_year == INT_INVALID)
            valid = 0;
        else
            pTm->tm_year = tmCheckConfig.tm_year;
    }
    if (pTm->tm_mon == INT_INVALID)
    {
        if (tmCheckConfig.tm_mon == INT_INVALID)
            valid = 0;
        else
            pTm->tm_mon = tmCheckConfig.tm_mon;
    }
    if (pTm->tm_mon < 0 || pTm->tm_mon > 11)
    {
        pTm->tm_mon = INT_INVALID;
        valid = 0;
    }
    if (pTm->tm_mday == INT_INVALID)
    {
        if (tmCheckConfig.tm_mday == INT_INVALID)
            valid = 0;
        else
            pTm->tm_mday = tmCheckConfig.tm_mday;
    }
    if (pTm->tm_year!=INT_INVALID)
        leap = LibOb_isLeapYear(pTm->tm_year);
    if (pTm->tm_mday < 0 || pTm->tm_mday>DAYSOFMONTH[leap][pTm->tm_mon])
    {
        pTm->tm_mday = INT_INVALID;
        valid = 0;
    }
    if (pTm->tm_hour == INT_INVALID)
    {
        if (tmCheckConfig.tm_hour == INT_INVALID)
            valid = 0;
        else
            pTm->tm_hour = tmCheckConfig.tm_hour;
    }
    if (pTm->tm_hour < 0 || pTm->tm_hour > 11)
    {
        pTm->tm_hour = INT_INVALID;
        valid = 0;
    }
    if (pTm->tm_min == INT_INVALID)
    {
        if (tmCheckConfig.tm_min == INT_INVALID)
            valid = 0;
        else
            pTm->tm_min = tmCheckConfig.tm_min;
    }
    if (pTm->tm_min < 0 || pTm->tm_min > 59)
    {
        pTm->tm_min = INT_INVALID;
        valid = 0;
    }
    if (pTm->tm_sec == INT_INVALID)
    {
        if (tmCheckConfig.tm_sec == INT_INVALID)
            valid = 0;
        else
            pTm->tm_sec = tmCheckConfig.tm_sec;
    }
    if (pTm->tm_sec < 0 || pTm->tm_sec > 59)
    {
        pTm->tm_sec = INT_INVALID;
        valid = 0;
    }
    if (pTm->tm_isdst == INT_INVALID)
    {
        if (tmCheckConfig.tm_isdst == INT_INVALID)
            valid = 0;
        else
            pTm->tm_isdst = tmCheckConfig.tm_isdst;
    }
    if (pTm->tm_isdst < -1 || pTm->tm_isdst > 1)
    {
        pTm->tm_isdst = INT_INVALID;
        valid = 0;
    }
    return valid;
}

/**
 * @brief Returns the local time zone and dst information (if pointer is set) according to system clock settings.
 * In case dst is unknown (dst=-1) the time zone is the UTC time offset.
 * @param pDst Pointer to dst value [output]
 * @return Geographic time zone.
 */
stTimeZone LibOb_localTimeZone(int8_t* pDst)
{
    stTimeZone zone;
    time_t lTime = time(0);
    struct tm gt = tm_Ini;
    struct tm lt = tm_Ini;
    gt.tm_isdst = 0;
    lt.tm_isdst = -1;
    gmtime_s(&gt, &lTime);
    localtime_s(&lt, &lTime);
    time_t gtm = mktime(&gt);
    time_t ltm = mktime(&lt);
    time_t zoneValue = (((ltm-gtm)+LibCpp_SECONDSPERHOUR/8) / (LibCpp_SECONDSPERHOUR/4));
    zone.hours = (int8_t)(zoneValue>>2);
    zone.minutes = ((uint8_t)(zoneValue&3))*15;
    if (gt.tm_isdst>0) zone.hours -= 1;
    if (lt.tm_isdst>0) zone.hours += 1;
    if (pDst)
        *pDst = lt.tm_isdst;
    return zone;
}

/**
 * @brief Converts a calendrical time dataset to a character string.
 * @param destination String buffer.
 * @param destinationSize String buffer size.
 * @param format Format string for string conversion.
 * @param tp Calendrical time dataset
 * @param pTimeZone Pointer to time zone hours and minutes supplementing the struct tp calendrical time dataset.
 * @param pLanguage Pointer to a language choice information.
 * @return Number of bytes written to destination
 */
size_t LibOb_strftime(char* destination, size_t destinationSize, const char* format, const struct tm* tp, stTimeZone* pTimeZone, enum enLanguage* pLanguage)
{
    const char* formatPosition = format;
    char* destinationPosition = destination;
    char* lastDestionationPosition = destination+destinationSize-2;
    const char* stdFormat = "%Y-%m-%d#%H:%M:%S#%U#%z";
    if (!format)
        formatPosition = stdFormat;
    if (*formatPosition==0)
        formatPosition = stdFormat;
    *destination = 0;
    if (!tp) return 0;
    while(*formatPosition && destinationPosition<lastDestionationPosition)
    {
        formatPosition = strcpyfmt(formatPosition, &destinationPosition, destination+destinationSize-1);
        if (*formatPosition)
        {
            int len = 0;
            if (*formatPosition=='1') {len = 1; formatPosition++;}
            if (*formatPosition=='2') {len = 2; formatPosition++;}
            char formatSymbol = *formatPosition;
            switch (formatSymbol)
            {
            case 'Y':
                if (tp->tm_year != INT_INVALID)
                    destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%04d", (int)tp->tm_year+1900);
                break;
            case 'y':
                if (tp->tm_year != INT_INVALID)
                    destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%02d", (int)(tp->tm_year+1900)%100);
                break;
            case 'm':
                if (tp->tm_mon != INT_INVALID)
                {
                    if (len!=1)
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%02u", (unsigned int)tp->tm_mon+1);
                    else
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%u", (unsigned int)tp->tm_mon+1);
                }
                break;
            case 'b':
                if (tp->tm_mon != INT_INVALID)
                    destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%s", dayNameAbbreviations[enLanguageIndex(pLanguage)][tp->tm_mon+1]);
                break;
            case 'B':
                if (tp->tm_mon != INT_INVALID)
                    destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%s", dayNames[enLanguageIndex(pLanguage)][tp->tm_mon+1]);
                break;
            case 'e':
            case 'd':
                if (tp->tm_mday != INT_INVALID)
                {
                    if (len!=1 && formatSymbol!='e')
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%02u", (unsigned int)tp->tm_mday);
                    else
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%u", (unsigned int)tp->tm_mday);
                }
                break;
            case 'H':
                if (tp->tm_hour != INT_INVALID)
                {
                    if (len!=1)
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%02u", (unsigned int)tp->tm_hour);
                    else
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%u", (unsigned int)tp->tm_hour);
                }
                break;
            case 'I':
            {
                if (tp->tm_hour != INT_INVALID)
                {
                    unsigned int hour = (unsigned int)tp->tm_hour;
                    if (hour>12) hour = hour % 12;
                    if (len!=1)
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%02u", hour);
                    else
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%u", hour);
                }
                break;
            }
            case 'p':
                if (tp->tm_hour != INT_INVALID)
                {
                    if (tp->tm_hour>=12)
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%s", "PM");
                    else
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%s", "AM");
                }
                break;
            case 'M':
                if (tp->tm_min != INT_INVALID)
                {
                    if (len!=1)
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%02u", (unsigned int)tp->tm_min);
                    else
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%u", (unsigned int)tp->tm_min);
                }
                break;
            case 'S':
                if (tp->tm_sec != INT_INVALID)
                {
                    if (len!=1)
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%02u", (unsigned int)tp->tm_sec);
                    else
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%u", (unsigned int)tp->tm_sec);
                }
                break;
            case 'U':
                if (tp->tm_isdst != INT_INVALID)
                {
                    if (tp->tm_isdst == -1)
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%s", "UTC");
                    if (tp->tm_isdst == 0)
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%s", "STD");
                    if (tp->tm_isdst == 1)
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%s", "DST");
                }
                break;
            case 'z':
                if (pTimeZone)
                    if (pTimeZone->hours != INT8_INVALID)
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%+03d:%02u", (int)(*pTimeZone).hours, (unsigned int)(*pTimeZone).minutes);
                break;
            case 'Z':
                if (pTimeZone)
                    if (pTimeZone->hours != INT8_INVALID)
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%s", findZoneName(*pTimeZone, tp->tm_isdst));
                break;
            case 'a':
            {
                if (tp->tm_wday != INT_INVALID)
                {
                    unsigned int index = tp->tm_wday;
                    if (index>0 && index<=6)
                    {
                        if (index==0) index = 7;
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%s", dayNameAbbreviations[enLanguageIndex(pLanguage)][index]);
                    }
                }
                break;
            }
            case 'A':
            {
                if (tp->tm_wday != INT_INVALID)
                {
                    unsigned int index = tp->tm_wday;
                    if (index>0 && index<=6)
                    {
                        if (index==0) index = 7;
                        destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%s", dayNames[enLanguageIndex(pLanguage)][index]);
                    }
                }
                break;
            }
            case 'j':
                if (tp->tm_yday != INT_INVALID)
                    destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%u", tp->tm_yday+1);
                break;
            case '%':
                destinationPosition += sprintf_s(destinationPosition, lastDestionationPosition-destinationPosition, "%s", "%");
                break;
            default:
                formatPosition--;
            }
            if (*formatPosition) formatPosition++;
        }
    }
    *destinationPosition = 0;
    return 0;
}

/**
 * @brief Converts a string containing calendrical time data to a numeric calendrical time data set.
 * @param source Input character string.
 * @param format Format string. If set to zero an automatic scan is executed.
 * @param tp Output of numeric data set.
 * @param pTimeZone Pointer to time zone data supplementing 'tp'.
 * @return Pointer to the first character of source that is not being consumed by the format string.
 */
const char* LibOb_strptime(const char* source, const char* format, struct tm* tp, stTimeZone* pTimeZone)
{
    const char* formatPosition = format;
    const char* sourcePosition = source;
    const char* lastSourcePosition = source+strlen(source);
    int PMdetected = 0;
    if (!tp) return source;
    *tp = tm_Invalid;
    if (pTimeZone)
        *pTimeZone = stTimeZone_Invalid;
    if (!format)
        return scanCalendar(source, tp, pTimeZone);

    while(*formatPosition && sourcePosition<lastSourcePosition)
    {
        formatPosition = strchkfmt(formatPosition, &sourcePosition);
        if (*formatPosition)
        {
            if (*formatPosition=='1') formatPosition++;
            if (*formatPosition=='2') formatPosition++;
            char formatSymbol = *formatPosition;
            switch (formatSymbol)
            {
            case 'Y':
            {
                int year = 0;
                const char* scanResult =  scanInt(sourcePosition, &year, 0, 0, 0);
                if (scanResult)
                {
                    tp->tm_year = year - 1900;
                    sourcePosition = scanResult;
                }
                break;
            }
            case 'y':
            {
                int year = 0;
                const char* next =  scanInt(sourcePosition, &year, 0, 0, 0);
                if (next)
                {
                    if (year>=0 && year<100)
                    {
                        year += 2000;
                        if (year>2068) year -= 100;
                    }
                    tp->tm_year = year - 1900;
                    sourcePosition = next;
                }
                break;
            }
            case 'm':
            {
                unsigned int month = 0;
                const char* next =  scanUint(sourcePosition, &month, 0, 0, 1);
                if (next)
                {
                    tp->tm_mon = month - 1;
                    sourcePosition = next;
                }
                break;
            }
            case 'b':
            case 'B':
            {
                char buffer[64] = "";
                int month = INT_INVALID;
                const char* next =  scanExpression(sourcePosition, buffer, 64, 0, 0);
                if (next)
                {
                    month = findMonth(buffer);
                    if (month == 0) month = INT_INVALID;
                    tp->tm_mon = month - 1;
                    sourcePosition = next;
                }
                break;
            }
            case 'e':
            case 'd':
            {
                unsigned int day = 0;
                const char* next =  scanUint(sourcePosition, &day, 0, 0, 1);
                if (next)
                {
                    tp->tm_mday = day;
                    sourcePosition = next;
                }
                break;
            }
            case 'H':
            {
                unsigned int hour = 0;
                const char* next =  scanUint(sourcePosition, &hour, 0, 0, 0);
                if (next)
                {
                    tp->tm_hour = hour;
                    sourcePosition = next;
                }
                break;
            }
            case 'I':
            {
                unsigned int hour = 0;
                const char* next =  scanUint(sourcePosition, &hour, 0, 0, 0);
                if (next)
                {
                    if (PMdetected)
                    {
                        hour += 12;
                        if (hour==24) hour=0;
                    }
                    else
                    {
                        if (hour>12)
                            PMdetected = 1;
                    }
                    tp->tm_hour = hour;
                    sourcePosition = next;
                }
                break;
            }
            case 'p':
                if (*sourcePosition == 'P' && *(sourcePosition+1) == 'M')
                {
                    PMdetected = 1;
                    if (tp->tm_hour != INT_INVALID && tp->tm_hour != 12)
                        tp->tm_hour += 12;
                    sourcePosition += 2;
                }
                break;
            case 'M':
            {
                unsigned int minute = 0;
                const char* next =  scanUint(sourcePosition, &minute, 0, 0, 0);
                if (next)
                {
                    tp->tm_min = minute;
                    sourcePosition = next;
                }
                break;
            }
            case 'S':
            {
                unsigned int second = 0;
                const char* next =  scanUint(sourcePosition, &second, 0, 0, 0);
                if (next)
                {
                    tp->tm_sec = second;
                    sourcePosition = next;
                }
                break;
            }
            case 'U':
            {
                int8_t dst = scanDst(source, 1);
                if (dst!=(int8_t)0x80)
                {
                    sourcePosition += 3;
                    tp->tm_isdst = dst;
                }
                break;
            }
            case 'z':
            {
                stTimeZone result = stTimeZone_Ini;
                const char* scanResult = scanTimeZone(sourcePosition, &result);
                if (scanResult)
                {
                    sourcePosition = scanResult;
                    if (pTimeZone) *pTimeZone = result;
                }
                break;
            }
            case 'Z':
            {
                stZoneAbbreviation result;
                const char* scanResult = scanZone(sourcePosition, &result);
                if (scanResult)
                {
                    sourcePosition = scanResult;
                    if (pTimeZone) *pTimeZone = result.zone;
                }
                break;
            }
            case 'a':
            case 'A':
            case 'j':
            case '%':
                break;
            default:
                formatPosition--;
            }
            if (*formatPosition) formatPosition++;
        }
    }
    return sourcePosition;
}

/**
 * @brief Scans a calendar string and converts it to calendar data 'struct tm' and 'stTimeZone' without need of a format string.
 * The string will be completely scanned until 0 is reached.
 * @param source String to be scanned.
 * @param tp Resulting calendar data.
 * @param pTimeZone Resulting time zone. Might be set to zero.
 * @return
 */
const char* scanCalendar(const char* source, struct tm* tp, stTimeZone* pTimeZone)
{
    char buffer[64];
    const char* scanPos = source;
    const char* resultPos = source;
    int type = 0;
    int cnt = 0;

    if (!tp) return source;
    *tp = tm_Invalid;
    if (pTimeZone)
        *pTimeZone = stTimeZone_Invalid;

    scanPos = scipNonLetters(scanPos, &type);
    while (*scanPos && cnt++<100)
    {
        if (type == 2)
        {   // string
            resultPos = scanExpression(scanPos, buffer, 64, 0, 0);
            if (resultPos)
            {
                int8_t month;
                int pm=0;
                stZoneAbbreviation zone = stZoneAbbreviation_Invalid;

                scanZone(scanPos, &zone);
                if (buffer[0]=='P' && buffer[1]=='M' && buffer[2]==0) pm=1;
                month = findMonth(buffer);

                if (month && tp->tm_mon==INT_INVALID)
                    tp->tm_mon = month-1;
                else if (pm && tp->tm_hour!=INT_INVALID)
                {
                    tp->tm_hour += 12;
                    if (tp->tm_hour == 24)
                        tp->tm_hour = 0;
                }
                else if (zone.dst != (int8_t)0x80)
                {
                    tp->tm_isdst = zone.dst;
                    if (pTimeZone) *pTimeZone = zone.zone;
                }
                scanPos = resultPos;
            }
        }
        else
        {   // number (type=1)
            int number = 0;
            resultPos = scanInt(scanPos, &number, 0, 0, 1);
            if (resultPos)
            {
                int len = resultPos - scanPos;
                if (*scanPos == '-' || *scanPos == '+')
                {
                    scanPos++;
                    len--;
                }
                if (len>=3 && tp->tm_isdst==INT_INVALID)
                {   // year
                    if (tp->tm_year==INT_INVALID) tp->tm_year = number - 1900;
                }
                else
                {   // year (2 digits), day, month, time
                    if (*(scanPos-1)!=':' && *resultPos!=':' && tp->tm_isdst==INT_INVALID)
                    {
                        int isDay = 0;
                        if ((*resultPos=='s' && *(resultPos+1)=='t') || (*resultPos=='n' && *(resultPos+1)=='d') || (*resultPos=='r' && *(resultPos+1)=='d') || (*resultPos=='t' && *(resultPos+1)=='h')) isDay = 1;
                        if (number<0) number = -number;
                        // day?
                        if (tp->tm_mday==INT_INVALID && (isDay || *resultPos=='.' || *resultPos==',' || (*(scanPos-1)=='-' && *resultPos!='-') || (*(scanPos-1)=='/' && *resultPos=='/')))
                        {
                            tp->tm_mday = number;
                        }
                        // month?
                        else if (tp->tm_mon==INT_INVALID && ((tp->tm_mday!=INT_INVALID && *resultPos=='.') || (*(scanPos-1)=='-' && *resultPos=='-') || (*(scanPos-1)!='/' && *resultPos=='/')))
                        {
                            tp->tm_mon = number-1;
                        }
                        // year?
                        else if (tp->tm_year==INT_INVALID && (*(scanPos-1)=='.' || *(scanPos-1)=='\'' || (*(scanPos-1)!='-' && *resultPos=='-') || (*(scanPos-1)=='/' && *resultPos!='/')))
                        {
                            number+=2000;
                            if (number>2068) number -= 100;
                            tp->tm_year = number - 1900;
                        }
                        if (*resultPos=='.')
                            resultPos++;
                    }
                    if ((*(scanPos-1)==':' || *resultPos==':') && tp->tm_isdst==INT_INVALID)
                    {
                        if (tp->tm_hour==INT_INVALID && *(scanPos-1)!=':' && *resultPos==':')
                            tp->tm_hour = number;
                        if (tp->tm_min==INT_INVALID && *(scanPos-1)==':' && *resultPos==':')
                            tp->tm_min = number;
                        if (tp->tm_sec==INT_INVALID && *(scanPos-1)==':' && *resultPos!=':')
                        {
                            if (tp->tm_min == INT_INVALID && *(scanPos-2)!=':')
                                tp->tm_min = number;
                            else
                                tp->tm_sec = number;
                        }
                        if (*resultPos=='+' || *resultPos=='-')
                        {
                            const char* resultPosZone;
                            stTimeZone timeZone;
                            resultPosZone = scanTimeZone(resultPos, &timeZone);
                            if (resultPosZone)
                            {
                                if (pTimeZone) *pTimeZone = timeZone;
                                tp->tm_isdst = -1;
                                resultPos = resultPosZone;
                            }
                        }
                    }
                }
                scanPos = resultPos;
            }
        }
        scanPos = scipNonLetters(scanPos, &type);
    }
    return scanPos;
}

/** @} */
