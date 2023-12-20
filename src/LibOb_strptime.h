// utf-8 (ü)
/**
 * @file   LibOb_strptime.h
 * @author Olaf Simon
 * @date   20.09.2023
 * @brief  See LibOb_strptime.c
 *
 * \addtogroup LibOb_Time
 * @{
**/

#ifndef LIBOB_STRPTIME_H
#define LIBOB_STRPTIME_H

#define INT_INVALID ((int)((UINT_MAX>>1)+1))            ///< Invalid int
#define UINT_INVALID UINT_MAX                           ///< Invalid uint
#define INT8_INVALID ((int8_t)0x80)                     ///< Invalid int8_t
#define UINT8_INVALID 0xFF                              ///< Invalid uint8_t
#define INT16_INVALID ((int16_t)0x8000)                 ///< Invalid int16_t
#define UINT16_INVALID 0xFFFF                           ///< Invalid uint16_t
#define INT32_INVALID ((int32_t)0x80000000)             ///< Invalid int32_t
#define UINT32_INVALID 0xFFFFFFFF                       ///< Invalid uint32_t
#define INT64_INVALID ((int64_t)0x8000000000000000)     ///< Invalid int64_t
#define UINT64_INVALID 0xFFFFFFFFFFFFFFFF               ///< Invalid uint64_t

#define LibOb_isLeapYear(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0) ///< Checks a year being a leap year

#include <ctype.h>
#include <time.h>
#include <stdint.h>
#include <limits.h>

#ifdef LibConfig
    #include <LibConfig.h>
#endif

#ifdef LibOb_LANGUAGE
    #include "../../LibOb/CppCommon/Language/Language.h"
#else
    enum enLanguage             ///< List of standardized implemented languages.
    {
        enLanguage_en_US = 0    ///< English language
    };
#endif

extern enum enLanguage LibOb_GLOBALLANGUAGE;

typedef struct tm tm_t;             ///< tm_t
extern const tm_t tm_Ini;           ///< tm_Ini
extern const tm_t tm_Invalid;       ///< tm_Invalid

/**
 * @brief Struct for time zone in hours and minutes
**/
#pragma pack(1)
typedef struct _stTimeZone
{
    int8_t  hours;      ///< Time zone hours
    uint8_t minutes;    ///< Time zone minutes
} stTimeZone;
#pragma pack()

extern const stTimeZone stTimeZone_Ini;             ///< Initializer for stTimeZone
extern const stTimeZone stTimeZone_Invalid;         ///< Invalidates stTimeZone

/**
 * @brief Struct for time zone, dst and time zone name
**/
typedef struct _stZoneAbbreviation
{
    const char* name;       ///< Name of time zone
    int8_t dst;             ///< dst of time zone
    stTimeZone zone;        ///< Time zone in numeric format
} stZoneAbbreviation;

extern const stZoneAbbreviation stZoneAbbreviation_Ini;         ///< Initializer for stZoneAbbreviation
extern const stZoneAbbreviation stZoneAbbreviation_Invalid;     ///< Invalidates all entries of stZoneAbbreviation

#ifdef __cplusplus
extern "C" {
#endif

/* strftime / strptime -------------------------------------------------------------------- */

size_t      LibOb_strftime(char* destination, size_t destinationSize, const char* format, const struct tm* tp, stTimeZone* pTimeZone, enum enLanguage* pLanguage); ///< Converts struct tm to formatted character string
const char* LibOb_strptime(const char* source, const char* format, struct tm* tp, stTimeZone* pTimeZone);   ///< Converts a time string to calendrical time data stored in 'struct tm'.
stTimeZone  LibOb_localTimeZone(int8_t* pDst);                                                              ///< Retrieves time zone and dst from local system clock settings

int         LibOb_checkStructTm(struct tm* pTm, struct tm tmCheckConfig, int isDuration);                   ///< Checks struct tm having valid entries.
int8_t      LibOb_dst(const char* dstStr, int startsWith);                                                  ///< Returns dst code number from a given string 'UTC', 'STD' or 'DST' or time zone abbreviation e.g. 'CET'.
stTimeZone  LibOb_timeZone(const char* timeZoneStr, int8_t* pDst);                                          ///< Returns time zone struct and dst code from a given time zone string or abbreviation e.g. 'CET', 'UTC +01'.

/* General string related scan functions -------------------------------------------------- */

int         isnumber(unsigned char character);                                                              ///< Checks a character being part of a number string. See isnumber .
const char* scipSpace(const char* source);                                                                  ///< Scans until no more white space is found.
const char* scipDigits(const char* source);                                                                 ///< Scans until no more digits are found.
const char* scipNonLetters(const char* source, int* type);                                                  ///< Scans until a digit or letter character is found
const char* scanUint(const char* source, unsigned int* result, int scip, int allowPlus, int allowLaggingDot);           ///< Converts a character string to a unsigned integer.
const char* scanInt(const char* source, int* result, int scip, int requiredPlus, int allowLaggingDot);      ///< Converts a character string to an integer.
const char* scanExpression(const char* source, char* destination, size_t destinationSize, int scip, int allowNumber); ///< Scans (and copies) the next expression.

#ifdef __cplusplus
}
#endif

#endif
/** @} */
