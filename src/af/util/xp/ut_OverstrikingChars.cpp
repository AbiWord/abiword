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

#include "ut_OverstrikingChars.h"
#include "ut_assert.h"
#include <stdlib.h>

typedef struct {UT_UCSChar min; UT_UCSChar max; overstr_type dir;} char_bounds;

// the following table is heuristic; we will add to it more characters
// when we need to (we could try to generate this table for the whole
// UCS-2 space, but at the moment that might be an overkill)

// the table contains pairs of UCS-2 values between which all characters
// are overstriking (this is inclusive of the two boundary values in the
// table)

// !!! the table MUST be sorted to allow binary search !!!
char_bounds overstr_lut[]=
{
	//miscell
	{0x0300,0x036f, UT_OVERSTRIKING_LTR},
	
	//Greek
	{0x037a,0x037a, UT_OVERSTRIKING_LTR},

	//Hebrew
	{0x0591,0x05bd, UT_OVERSTRIKING_RTL}, //this includes two currently undefined characters 0x05a2 and 0x05ba
	{0x05bf,0x05bf, UT_OVERSTRIKING_RTL},
	{0x05c1,0x05c2, UT_OVERSTRIKING_RTL},
	{0x05c4,0x05c4, UT_OVERSTRIKING_RTL},

	//Arabic
	{0x064b,0x0655, UT_OVERSTRIKING_RTL},
	{0x0670,0x0670, UT_OVERSTRIKING_RTL},
	{0x06d6,0x06e4, UT_OVERSTRIKING_RTL},
	{0x06e7,0x06e8, UT_OVERSTRIKING_RTL},
	{0x06ea,0x06ed, UT_OVERSTRIKING_RTL},

	//Syriac
	{0x0711,0x0711, UT_OVERSTRIKING_RTL},
	{0x0730,0x074a, UT_OVERSTRIKING_RTL},

	// symbols -- not sure whether LTR/RTL is good enough for this
	// kind of stuff
	{0x20d0,0x20ea, UT_OVERSTRIKING_LTR},

	// halfmarks
	{0xfe20,0xfe23, UT_OVERSTRIKING_LTR}
};

static UT_sint32 s_compare(const void * a, const void * b)
{
	const UT_UCSChar  * c  = (const UT_UCSChar *)  a;
	const char_bounds * bn = (const char_bounds *) b;

	if(*c < bn->min)
		return -1;

	if(*c > bn->max)
		return 1;

	return 0;
}

overstr_type isOverstrikingChar(UT_UCSChar c)
{
	char_bounds * e;
	if((e = (char_bounds *) bsearch(&c, overstr_lut, NrElements(overstr_lut), sizeof(char_bounds), s_compare)))
	{
		return e->dir;
	}
	else
		return UT_NOT_OVERSTRIKING;
}

