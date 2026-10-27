// SPDX-License-Identifier: GPL-3.0-or-later
/* ABI adapter only: Common owns every civil coordinate calculation. */
#include "julian-day.h"
#include <infiltratr/temporal.h>
gboolean calendar_gregorian_is_leap(gint64 y) { return infiltratr_temporal_gregorian_is_leap(y); }


gint64 calendar_gregorian_to_jdn(gint64 y, gint m, gint d) { return infiltratr_temporal_gregorian_to_jdn(y, m, d); }


gint calendar_iso_weekday(gint64 jdn) { return infiltratr_temporal_iso_weekday(jdn); }

