/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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
 
/* patform: MacOS 8/9 and MacOS X */
 
 

#ifndef GR_MACFONT_h
#define GR_MACFONT_h

#include <ATSUnicode.h>

#include "ut_misc.h"
#include "gr_Graphics.h"



class GR_MacFont : public GR_Font
{
public:
	GR_MacFont(int font, int face, int pointSize);
    // constructor from ASTUI font
	GR_MacFont (ATSUFontID atsuId);
    
    virtual ~GR_MacFont ();
    UT_uint32 getAscent();
	UT_uint32 getDescent();
	UT_uint32 getHeight();
    UT_uint32 getSize ()
		{ return m_pointSize; };
    ATSFontRef getFontRef ()
        { return m_fontRef; };
	ATSUFontID getFontID ()
		{ return m_fontID; };
protected:
	ATSUTextLayout		m_MeasurementText;
	
static void _quickAndDirtySetUnicodeTextFromASCII_C_Chars(UniCharArrayPtr *ucap, UniCharCount *ucc);
private:
	ATSUFontID	m_fontID;
    ATSFontRef m_fontRef;
	UT_uint32	m_pointSize;
void _initMeasurements ();
};


#endif
