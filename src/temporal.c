// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file temporal.c
 * @brief Portable temporal presentation policy implementation.
 */
#include "infiltratr/temporal.h"

#include "infiltratr/arithmetic.h"
#include "infiltratr/config.h"
#include "infiltratr/core.h"
#include "infiltratr/format.h"
#include "infiltratr/temporal_id.h"
#include "infiltratr/timing.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define MICROSECONDS_PER_SECOND INT64_C(1000000)
#define SECONDS_PER_DAY INT64_C(86400)
#define MICROSECONDS_PER_DAY (SECONDS_PER_DAY * MICROSECONDS_PER_SECOND)

typedef struct ClockProfileEntry {
    InfiltratrClockProfile profile;
    const char *id;
    const char *name;
} ClockProfileEntry;


/* ABI-preserved legacy symbol: authoritative clock faces require native resources. */


/* The normal OS Gregorian calendar is always available. Custom chronology is
 * added only by validated installed resources. This is metadata, not an engine. */


/*
 * Private migration only. This is deliberately not an exported v2 ABI.
 * It exists solely so a presentation.conf already written by System Settings
 * 0.2.0 can be upgraded without losing the user's selected clock/calendar.
 * secondary-calendar is intentionally ignored.
 */


/* Neutral civil/JDN coordinates shared by providers and consumers. These are
 * not a selectable chronology or a custom presentation fallback. */
static int64_t temporal_coordinate_floor(int64_t value, int64_t divisor)
{
    int64_t quotient = 0;
    return infiltratr_i64_floor_divmod(value, divisor, &quotient, NULL) ? quotient : 0;
}
static int64_t temporal_coordinate_modulo(int64_t value, int64_t divisor)
{
    int64_t remainder = 0;
    return infiltratr_i64_floor_divmod(value, divisor, NULL, &remainder) ? remainder : 0;
}
bool
infiltratr_temporal_gregorian_is_leap(int64_t year)
{
    return temporal_coordinate_modulo(year, 4) == 0 &&
           (temporal_coordinate_modulo(year, 100) != 0 ||
            temporal_coordinate_modulo(year, 400) == 0);
}


int64_t
infiltratr_temporal_gregorian_to_jdn(int64_t year,
                               int32_t month,
                               int32_t day)
{
    const int64_t a = temporal_coordinate_floor(14 - (int64_t)month, 12);
    const int64_t y = infiltratr_i64_subtract_saturating(
        infiltratr_i64_add_saturating(year, 4800), a);
    const int64_t m = infiltratr_i64_subtract_saturating(
        infiltratr_i64_add_saturating(
            month, infiltratr_i64_multiply_saturating(12, a)),
        3);
    int64_t result = day;

    result = infiltratr_i64_add_saturating(
        result,
        temporal_coordinate_floor(
            infiltratr_i64_add_saturating(
                infiltratr_i64_multiply_saturating(153, m), 2),
            5));
    result = infiltratr_i64_add_saturating(
        result, infiltratr_i64_multiply_saturating(365, y));
    result = infiltratr_i64_add_saturating(
        result, temporal_coordinate_floor(y, 4));
    result = infiltratr_i64_subtract_saturating(
        result, temporal_coordinate_floor(y, 100));
    result = infiltratr_i64_add_saturating(
        result, temporal_coordinate_floor(y, 400));
    return infiltratr_i64_subtract_saturating(result, 32045);
}


int32_t
infiltratr_temporal_iso_weekday(int64_t jdn)
{
    return (int32_t)temporal_coordinate_modulo(
               jdn,
               7) + 1;
}
