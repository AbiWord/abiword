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
#include "ut_debugmsg.h"

// these tables have to be sorted by the first number !!!
static Letter s_table[] =
{
	// code, intial, medial, final, stand-alone
	{0x05DB, 0x05DB, 0x05DB, 0x05DA, 0x05DA},
	{0x05DE, 0x05DE, 0x05DE, 0x05DD, 0x05DD},
	{0x05E0, 0x05DE, 0x05DE, 0x05DF, 0x05DF},
	{0x05E4, 0x05E4, 0x05E4, 0x05E3, 0x05E3},
	{0x05E6, 0x05E6, 0x05E6, 0x05E5, 0x05E5}
};

static UCSRange s_ignore[] =
{
	{0x0591,0x05A1},
	{0x05A3,0x05B9},
	{0x05BB,0x05BD},
	{0x05BF,0x05BF},
	{0x05C1,0x05C2},
	{0x05C4,0x05C4}
};

// comparison function for bsearch
static int s_comp(const void *a, const void *b)
{
	const UT_UCSChar* A = (const UT_UCSChar*)a;
	const Letter* B = (const Letter*)b;
	
	return (int)*A - (int)B->code;
}

static int s_comp_ignore(const void *a, const void *b)
{
	const UT_UCSChar* A = (const UT_UCSChar*)a;
	const UCSRange* B = (const UCSRange*)b;
	if(*A < B->low)
		return -1;
	if(*A > B->high)
		return 1;
	
	return 0;
}

Letter* UT_contextGlyph::s_pGlyphTable = &s_table[0];
UCSRange * UT_contextGlyph::s_pIgnore = &s_ignore[0];


/*
	code - pointer to the character to interpret
	prev - pointer to the character before code
	next - NULL-terminated string of characters that follow
	
	returns the glyph to be drawn
*/
UT_UCSChar UT_contextGlyph::getGlyph(const UT_UCSChar * code,
									 const UT_UCSChar * prev,
									 const UT_UCSChar * next) const
{
	UT_ASSERT(code);

	Letter *pL = (Letter*) bsearch((void*)code, (void*)s_pGlyphTable, NrElements(s_table),sizeof(Letter),s_comp);
	
	// if the letter is not in our table, it means it has only one form
	// so we return it back
	if(!pL)
		return *code;

	// if we got no next and no prev, than this is stand-alone char	
	if(!next && !prev)
		return pL->alone;
		
	// if we got no next or previous pointers, a final or intial value is
	// required	
	if(!prev)
		return pL->initial;
	
	if(!next)
		return pL->final;
	
	//check if next is not a character that is to be ignored
	UT_UCSChar *myNext = next;
	
	while(*myNext && bsearch((void*)myNext, (void*)s_pIgnore, NrElements(s_ignore),sizeof(UCSRange),s_comp_ignore))
		myNext++;
			
	if(!*myNext)
		return pL->final; //we skipped to the end
		
	UT_DEBUGMSG(("UT_contexGlyph: myNext 0x%x\n", *myNext));
	// test whether *prev and *next are word delimiters
	bool bPrevWD = UT_isWordDelimiter(*prev, *code);
	bool bNextWD = UT_isWordDelimiter(*myNext, UCS_SPACE);
	
	// if both are not , then medial form is needed
	if(!bPrevWD && !bNextWD)
		return pL->medial;
	
	// if only *next is, than final form is needed
	if(bNextWD)
		return pL->final;
		
	// if *prev is, the initial form is needed
	if(bPrevWD)
		return pL->initial;
		
	// if we got here, both are delimiters, which means stand alone form is needed
	return pL->alone;
}
