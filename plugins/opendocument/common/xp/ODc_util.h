/* Copyright (C) 2012 Abisource.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef ODC_UTIL_H
#define	ODC_UTIL_H

#include "ut_string.h"
#include "ut_assert.h"

// Converts specified date from MM-DD-YYYY to YYYY-MM-DD(ISO8601)
// or vice versa and returns new string representation
std::string ODc_reorderDate(const std::string &dateStr, bool fromISO);


#endif	/* ODC_UTIL_H */

