/* AbiSource Program Utilities
 * Copyright (C) 2001 Tomas Frydrych
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef UT_OVERCHARS_H
#define UT_OVERCHARS_H

#include "ut_types.h"

typedef enum {UT_NOT_OVERSTRIKING, UT_OVERSTRIKING_RTL, UT_OVERSTRIKING_LTR} overstr_type;

overstr_type isOverstrikingChar(UT_UCSChar c);
#endif

