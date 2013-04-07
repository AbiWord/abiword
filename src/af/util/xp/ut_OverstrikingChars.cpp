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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include "ut_OverstrikingChars.h"
#include "ut_assert.h"
#include <stdlib.h>

typedef struct {UT_UCSChar min; UT_UCSChar max; UT_uint32 dir;} char_bounds;

// the following table is heuristic; we will add to it more characters
// when we need to (we could try to generate this table for the whole
// UCS-2 space, but at the moment that might be overkill)

// the table contains pairs of UCS-2 values between which all characters
// are overstriking (this is inclusive of the two boundary values in the
// table)

// The 'rightness' and 'leftness' of an overstriking character is from
// the point of view of logical space, i.e., right-flushed character
// is one that is flushed with the near edge of its base character, and
// left-flushed character is one that is flushed with the far edge of
// its base character (in other words, if the characters are RTL,
// UT_OVERSTRIKING_LEFT indicates that the character alings with the
// *RIGHT* visual edge of the base character.

// !!! the table MUST be sorted to allow binary search !!!
char_bounds overstr_lut[]=
{
	//miscell
	{0x0300,0x0314, UT_OVERSTRIKING_LTR | UT_OVERSTRIKING_CENTRE},
	{0x0315,0x0315, UT_OVERSTRIKING_LTR | UT_OVERSTRIKING_RIGHT},
	{0x0316,0x031a, UT_OVERSTRIKING_LTR | UT_OVERSTRIKING_CENTRE},
	{0x031b,0x031b, UT_OVERSTRIKING_LTR | UT_OVERSTRIKING_RIGHT},
	{0x031c,0x0320, UT_OVERSTRIKING_LTR | UT_OVERSTRIKING_CENTRE},
	{0x0321,0x0322, UT_OVERSTRIKING_LTR | UT_OVERSTRIKING_RIGHT},
	{0x0323,0x033f, UT_OVERSTRIKING_LTR | UT_OVERSTRIKING_CENTRE},
	{0x0340,0x0340, UT_OVERSTRIKING_LTR | UT_OVERSTRIKING_LEFT},
	{0x0341,0x0341, UT_OVERSTRIKING_LTR | UT_OVERSTRIKING_RIGHT},
	{0x0342,0x034f, UT_OVERSTRIKING_LTR | UT_OVERSTRIKING_CENTRE},
	{0x0360,0x0362, UT_OVERSTRIKING_LTR | UT_OVERSTRIKING_RIGHT},
	{0x0363,0x036f, UT_OVERSTRIKING_LTR | UT_OVERSTRIKING_CENTRE},
	
	//Hebrew
	{0x0591,0x0598, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},
	{0x0599,0x0599, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_RIGHT},
	{0x059a,0x059a, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_LEFT},
	{0x059b,0x059b, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},
	{0x059d,0x059d, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_LEFT},
	{0x059e,0x059f, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},
	{0x05a0,0x05a0, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_LEFT},
	{0x05a1,0x05a1, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_RIGHT},
	{0x05a3,0x05a8, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},
	{0x05a9,0x05a9, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_RIGHT},
	{0x05aa,0x05ac, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},
	{0x05ad,0x05ad, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_LEFT},
	{0x05ae,0x05ae, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_RIGHT},
	{0x05af,0x05b8, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},
	{0x05b9,0x05b9, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_RIGHT},
	{0x05bb,0x05bd, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},
	{0x05bf,0x05bf, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},
	{0x05c1,0x05c1, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_LEFT},
	{0x05c2,0x05c2, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_RIGHT},
	{0x05c4,0x05c4, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},
	
	//Arabic
	{0x064b,0x0655, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},
	{0x0670,0x0670, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},
	{0x06d6,0x06e4, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},
	{0x06e7,0x06e8, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},
	{0x06ea,0x06ed, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},

	//Syriac
	{0x0711,0x0711, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},
	{0x0730,0x073f, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},
	{0x0740,0x0740, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_RIGHT},
	{0x0741,0x074a, UT_OVERSTRIKING_RTL | UT_OVERSTRIKING_CENTRE},

	// symbols -- not sure whether LTR/RTL is good enough for this
	// kind of stuff
	{0x20d0,0x20ea, UT_OVERSTRIKING_LTR | UT_OVERSTRIKING_CENTRE},

	// halfmarks
	{0xfe20,0xfe23, UT_OVERSTRIKING_LTR | UT_OVERSTRIKING_RIGHT}
};

static UT_sint32 s_compare(const void * a, const void * b)
{
	const UT_UCSChar  * c  = static_cast<const UT_UCSChar *>(a);
	const char_bounds * bn = static_cast<const char_bounds *>(b);

	if(*c < bn->min)
		return -1;

	if(*c > bn->max)
		return 1;

	return 0;
}

UT_uint32 UT_isOverstrikingChar(UT_UCSChar c)
{
	char_bounds * e;
	if((e = static_cast<char_bounds *>(bsearch(&c, overstr_lut, G_N_ELEMENTS(overstr_lut), sizeof(char_bounds), s_compare))))
	{
		return e->dir;
	}
	else
		return UT_NOT_OVERSTRIKING;
}

