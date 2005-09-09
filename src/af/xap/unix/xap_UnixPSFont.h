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

#ifndef XAP_UNIXPSFONT_H
#define XAP_UNIXPSFONT_H

#include "ut_types.h"
#include "gr_Graphics.h"

#include "xap_UnixFont.h"

#include "xap_UnixPSParseAFM.h"

/*****************************************************************/
/*****************************************************************/

class PSFont : public GR_Font
{
public:
	explicit PSFont(XAP_UnixFont * hFont, UT_uint32 size);
	virtual ~PSFont(void);

	XAP_UnixFont * 		getUnixFont(void);
	UT_uint32			getSize(void) { return m_pointSize; };
	void				setIndex(UT_uint32 ndx) { m_index = ndx; }
	UT_uint32			getIndex(void) { return m_index; };	
	
	virtual const UT_String & hashKey(void) const;
	virtual UT_sint32   measureUnremappedCharForCache(UT_UCSChar cChar) const;
	float               measureUnRemappedChar(const UT_UCSChar c, UT_uint32 iSize) const;
        virtual bool        glyphBox(UT_UCS4Char g, UT_Rect & rec, GR_Graphics * pG);

	virtual bool doesGlyphExist(UT_UCS4Char g);

private:
	PSFont();
	PSFont (const PSFont & other);
	PSFont & operator= (const PSFont & other);

	XAP_UnixFont * 		m_hFont;
	UT_uint32			m_pointSize;
	UT_uint32			m_index;
};

#endif /* XAP_UNIXPSFONT_H */
