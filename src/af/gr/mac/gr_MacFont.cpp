/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
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
 
/*
   WARNING: this code make heavy use of ASTUI
   If you don't know ATSUI, please don't try to modify it
   If you know ATSUI, please feel free and correct me
 */
#include <ATSUnicode.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "gr_Graphics.h"
#include "gr_MacFont.h"



GR_MacFont::GR_MacFont (ATSUFontID atsFontId)
	: GR_Font ()
{
	UT_ASSERT (atsFontId != kATSUInvalidFontID);

	m_fontRef = 0;
	m_fontID = atsFontId;
	m_MeasurementText = NULL;
	m_pointSize = 0;
}


GR_MacFont::~GR_MacFont ()
{
	
}




UT_uint32 GR_MacFont::getAscent()
{
	OSStatus			status;
	ATSUTextMeasurement ascent;
	if (m_MeasurementText == NULL) {
		_initMeasurements ();
	}
	status = ATSUMeasureText(m_MeasurementText, 0, 1, NULL, NULL, &ascent, NULL);
	UT_ASSERT (status == noErr);
	
	return (UT_uint32)ascent;
}


UT_uint32 GR_MacFont::getDescent()
{
	OSStatus			status;
	ATSUTextMeasurement descent;
	if (m_MeasurementText == NULL) {
		_initMeasurements ();
	}
	status = ATSUMeasureText(m_MeasurementText, 0, 1, NULL, NULL, NULL, &descent);
	UT_ASSERT (status == noErr);
	
	return (UT_uint32)descent;
}


UT_uint32 GR_MacFont::getHeight()
{
	OSStatus			status;
	ATSUTextMeasurement ascent, descent;
	if (m_MeasurementText == NULL) {
		_initMeasurements ();
	}
	status = ATSUMeasureText(m_MeasurementText, 0, 1, NULL, NULL, &ascent, &descent);
	UT_ASSERT (status == noErr);
	
	return (UT_uint32)(descent + ascent);
}




void
GR_MacFont::_initMeasurements ()
{
	UniCharArrayPtr		uniText;
	UniCharCount		uniTextLen;
	ATSUStyle			style;				// Take the Style that belong to the font.
	UniCharCount		runLengths;
	OSStatus			status;
	
	_quickAndDirtySetUnicodeTextFromASCII_C_Chars(&uniText, &uniTextLen);
	runLengths = uniTextLen;
	status = ATSUCreateStyle(&style);
	UT_ASSERT (status == noErr);
	status = ATSUCreateTextLayoutWithTextPtr (uniText, 0, 2, uniTextLen, 1, &runLengths, &style, &m_MeasurementText);
	UT_ASSERT (status == noErr);
	::DisposePtr ((Ptr)uniText);
}
/*
	Quickly fill a Unicode text to some ASCII stuff... for text measurments
 */
void 
GR_MacFont::_quickAndDirtySetUnicodeTextFromASCII_C_Chars(UniCharArrayPtr *ucap, UniCharCount *ucc)
{
	static char *someText = "bq";
	
	TECObjectRef ec;
	OSStatus status = TECCreateConverter(&ec, kTextEncodingMacRoman, kTextEncodingUnicodeDefault);
	if (status != noErr)  {
		UT_DEBUGMSG(("TECCreateConverter failed"));
		UT_ASSERT (status == noErr);
	}
	ByteCount ail, aol, iLen = strlen (someText), oLen = 2 * iLen;
	Ptr buffer = NewPtr(oLen);
	status = TECConvertText(ec, (ConstTextPtr)someText, iLen, &ail, (TextPtr)buffer, oLen, &aol);
	if (status != noErr) {
		UT_DEBUGMSG(("TECConvertText failed\n"));
		UT_ASSERT (status == noErr);
	}
	status = TECDisposeConverter(ec);
	if (status != noErr) {
		UT_DEBUGMSG(("TECDisposeConverter failed\n"));
		UT_ASSERT (status == noErr);
	}
	*ucap = (UniCharArrayPtr)NewPtr(aol);
	BlockMove(buffer, (*ucap), aol);
	DisposePtr(buffer);
	*ucc = aol / 2;
}
