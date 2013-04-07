/* AbiSource Application Framework
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef GR_CHARWIDTHS_H
#define GR_CHARWIDTHS_H

#include "ut_types.h"
#include "ut_vector.h"

//////////////////////////////////////////////////////////////////
// we want GR_CHAR_WIDTH_UNKNOWN to be a large negative number
// with the four bytes indentical to be able to use memset
// so we choose 0x80808080
#define GR_UNKNOWN_BYTE 0x80
#define GR_CW_UNKNOWN (UT_sint32)0x80808080

// the following value should be used to indicate that the glyph is
// absent from the font
#define GR_CW_ABSENT (GR_CW_UNKNOWN + 1)

/* EXPLANATION OF THE OVERSTRIKING (or COMBINING) CHARACTERS MECHANISM
   (Tomas, Jan 26, 2003)

   We classify overstriking characters into three categories:

   1. Characters to be flushed with the near edge of the previous base
      character, or, from the LTR point of view, right-flushed
   2. Characters to be flushed with the far edge of the previous base
      character, or, from the LTR point of view, left-flushed
   3. Characters to be centered over the previous base character

   Whether a Unicode character is overstriking, and if so of what
   type, can be detemined by using the UT_isOverstrikingChar()
   function. The width of the overstriking characters should be reported
   as follows:

      Type 1 (right-flushed): width should be set to 0

      Type 2 (left-flushed):  the width of the glyph should be or'ed
                              with GR_OC_LEFT_FLUSHED (defined in gr_Graphics.h)

      Type 3 (centered):      the with of the glyph should be reported
                              as a negative number

*/
class ABI_EXPORT GR_CharWidths
{
public:
	GR_CharWidths(void);
	virtual ~GR_CharWidths(void);

	void			zeroWidths(void);
	void			setWidth(UT_UCSChar cIndex, UT_sint32 width);
	UT_sint32		getWidth(UT_UCSChar cIndex) const;

private:
	GR_CharWidths(GR_CharWidths &cp); // no impl

	typedef struct _a { UT_sint32 aCW[256]; } Array256;

	Array256		m_aLatin1;		// for speed, we don't use vector
	UT_GenericVector<Array256*>	m_vecHiByte;	// sparse vector<Array256 *>[hibyte]
};

#endif /* GR_CHARWIDTHS_H */
