/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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


#include <stdlib.h>
#include "ut_contextGlyph.h"
#include "ut_assert.h"

// this table has to be sorted by the first number !!!
static Letter s_table[] =
{
	// code, intial, medial, final, stand-alone
	{0x05DB, 0x05DB, 0x05DB, 0x05DA, 0x05DA},
	{0x05DE, 0x05DE, 0x05DE, 0x05DD, 0x05DD},
	{0x05E0, 0x05DE, 0x05DE, 0x05DF, 0x05DF},
	{0x05E4, 0x05E4, 0x05E4, 0x05E3, 0x05E3},
	{0x05E6, 0x05E6, 0x05E6, 0x05E5, 0x05E5}
};

// comparison function for bsearch
static int s_comp(const void *a, const void *b)
{
	const UT_UCSChar* A = (const UT_UCSChar*)a;
	const Letter* B = (const Letter*)b;
	
	return (int)*A - (int)B->code;
}

Letter* UT_contextGlyph::s_pGlyphTable = &s_table[0];


UT_UCSChar UT_contextGlyph::getGlyph(const UT_UCSChar * code, const UT_UCSChar * prev, const UT_UCSChar * next) const
{
	UT_ASSERT(code);
	
	Letter *pL = (Letter*) bsearch((void*)code, (void*)s_pGlyphTable, NrElements(s_table),sizeof(Letter),s_comp);
	
	// if the letter is not in our table, it means it has only one form
	// so we return it back
	if(!pL)
		return *code;

	// if we got now next and no prev, than this is stand-alone char	
	if(!next && !prev)
		return pL->alone;
		
	// if we got no next or previous pointers, a final or intial value is
	// required	
	if(!next)
		return pL->final;
	if(!prev)
		return pL->initial;
		
	// test whether *prev and *next are word delimiters
	bool bPrevWD = UT_isWordDelimiter(*prev, *code);
	bool bNextWD = UT_isWordDelimiter(*next, UCS_SPACE);
	
	// if both are not , then medial form is needed
	if(!bPrevWD && !bNextWD)
		return pL->medial;
	
	// if only *next is, than final form is needed
	if(bNextWD)
		return pL->final;
		
	// if *prev is, the initial form is needed
	if(bPrevWD)
		return pL->initial;
		
	// if we got here, both are initial, which means stand alone form is needed
	return pL->alone;
}
