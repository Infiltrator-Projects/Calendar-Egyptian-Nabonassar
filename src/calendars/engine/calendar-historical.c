/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 1993-2026 Shannon Smith
 *
 * Deterministic historical calendar models not supplied by ICU.
 *
 * Revised Julian follows Milanković's 1923 leap rule: ordinary fourth years
 * leap, century years leap only when their remainder modulo 900 is 200 or 600.
 * The arithmetic continuation is anchored to 2000-01-01, when Revised Julian
 * and Gregorian dates coincide.
 *
 * Byzantine Anno Mundi uses the Julian calendar, a 1 September year boundary
 * and the Constantinopolitan creation era whose first year begins 1 September
 * 5509 BC. This resource uses astronomical year numbering internally but
 * formats the historical Anno Mundi year without changing Julian month/day.
 *
 * Egyptian civil dates use Ptolemy's Nabonassar era as the absolute epoch:
 * 1 Thoth, year 1 = JDN 1448638 (26 February 747 BC in the proleptic Julian
 * calendar). This makes the otherwise wandering 365-day Egyptian civil year
 * deterministic. It must not be confused with pharaonic regnal-year dating.
 */

#include "calendar-historical.h"

#include "calendar-helpers.h"
#include "julian-day.h"

#include <infiltratr/arithmetic.h>
#include <glib/gi18n-lib.h>
#include <math.h>

#define floor_divide calendar_floor_divide
#define positive_modulo calendar_positive_modulo
#define gregorian_to_jdn calendar_gregorian_to_jdn
#define jdn_to_julian calendar_jdn_to_julian
#define julian_to_jdn calendar_julian_to_jdn

#define REVISED_JULIAN_ANCHOR_YEAR ((gint64)2000)
#define EGYPTIAN_NABONASSAR_EPOCH_JDN ((gint64)1448638)


static const gchar *const egyptian_months[] = {
    "", N_("Thoth"), N_("Phaophi"), N_("Hathyr"), N_("Choiak"),
    N_("Tybi"), N_("Mecheir"), N_("Phamenoth"), N_("Pharmouthi"),
    N_("Pachons"), N_("Payni"), N_("Epiphi"), N_("Mesore"),
    N_("Epagomenal days")
};


/*
 * Wandering 365-day years use one checked inverse. Saturation would make
 * INT64_MAX appear to round-trip even when the intermediate year*365 or
 * epoch addition overflowed. No provider may mistake that for a date.
 */
static gboolean
wandering_fields_to_jdn_checked(const CalendarCalendarFields *fields,
                               gint64 epoch, gint64 *result)
{
    gint64 years, days, year_start, candidate;
    gint offset;

    if (fields == NULL || result == NULL ||
        fields->month < 1 || fields->month > 13 ||
        fields->day < 1 ||
        fields->day > (fields->month == 13 ? 5 : 30))
        return FALSE;

    offset = (fields->month <= 12 ? (fields->month - 1) * 30 : 360) +
             fields->day - 1;
    if (!infiltratr_i64_add_checked(fields->year, -1, &years) ||
        !infiltratr_i64_multiply_checked(years, 365, &days) ||
        !infiltratr_i64_add_checked(epoch, days, &year_start) ||
        !infiltratr_i64_add_checked(year_start, offset, &candidate))
        return FALSE;

    *result = candidate;
    return TRUE;
}

gboolean
calendar_egyptian_jdn_reversible(gint64 jdn,
                                 const CalendarCalendarFields *fields)
{
    gint64 result;
    /* The private navigation engine reserves INT64_MAX as failure. It must
     * never also identify an admissible date at the executable ABI. */
    return jdn != G_MAXINT64 &&
        wandering_fields_to_jdn_checked(
            fields, EGYPTIAN_NABONASSAR_EPOCH_JDN, &result) &&
        result == jdn;
}

void
calendar_egyptian_fields_from_jdn(
    gint64 jdn,
    CalendarCalendarFields *fields)
{
    const gint64 elapsed = calendar_i64_subtract_saturating(
        jdn, EGYPTIAN_NABONASSAR_EPOCH_JDN);
    const gint64 year = calendar_i64_add_saturating(
        floor_divide(elapsed, 365), 1);
    const gint day_of_year = (gint)positive_modulo(elapsed, 365);

    g_return_if_fail(fields != NULL);

    *fields = (CalendarCalendarFields){
        .year = year,
        .month = day_of_year < 360 ? day_of_year / 30 + 1 : 13,
        .day = day_of_year < 360 ? day_of_year % 30 + 1 :
               day_of_year - 360 + 1,
        .auxiliary = 0,
        .special = day_of_year >= 360
    };
}

gint64
calendar_egyptian_fields_to_jdn(const CalendarCalendarFields *fields)
{
    gint64 result;
    return wandering_fields_to_jdn_checked(
        fields, EGYPTIAN_NABONASSAR_EPOCH_JDN, &result)
        ? result : G_MAXINT64;
}

gint
calendar_egyptian_month_length(
    const CalendarCalendarFields *fields)
{
    g_return_val_if_fail(fields != NULL, 0);
    g_return_val_if_fail(fields->month >= 1 && fields->month <= 13, 0);

    return fields->month == 13 ? 5 : 30;
}

gchar *
calendar_egyptian_format(
    const CalendarCalendarFields *fields,
    CalendarDatePart part)
{
    g_return_val_if_fail(fields != NULL, g_strdup(""));
    return calendar_format_named_date(fields, part, egyptian_months, G_N_ELEMENTS(egyptian_months), NULL);
}

/*
 * Traditional Armenian wandering year.
 *
 * The numbered Armenian era begins 11 July 552 in the Julian calendar.
 * Each year has twelve 30-day months followed by five Aweleacʿ epagomenal
 * days and no leap day; this is deliberately separate from Sarkawag's reform.
 */
#define ARMENIAN_EPOCH_JDN (calendar_julian_to_jdn(552, 7, 11))


