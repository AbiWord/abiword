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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef GR_CHARWIDTHS_H
#define GR_CHARWIDTHS_H

#include "ut_types.h"
#include "ut_vector.h"

//////////////////////////////////////////////////////////////////

class GR_CharWidths
{
public:
	GR_CharWidths(void);
	virtual ~GR_CharWidths(void);

	void			zeroWidths(void);
	void			setWidth(UT_UCSChar cIndex, UT_sint32 width);
	UT_sint32		getWidth(UT_UCSChar cIndex) const;

protected:

	typedef struct _a { UT_sint32 aCW[256]; } Array256;

	Array256		m_aLatin1;		// for speed, we don't use vector
	UT_Vector		m_vecHiByte;	// sparse vector<Array256 *>[hibyte]
};

#endif /* GR_CHARWIDTHS_H */
