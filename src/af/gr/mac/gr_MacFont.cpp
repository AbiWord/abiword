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
#include <FixMath.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "gr_Graphics.h"
#include "gr_MacFont.h"


GR_MacFont::GR_MacFont (const ATSUStyle atsFontStyle)
	: GR_Font ()
{
	OSStatus err;

	m_MeasurementText = NULL;
	m_pointSize = 0;
	err = ATSUCreateAndCopyStyle (atsFontStyle, &m_fontStyle);
	UT_ASSERT (err == noErr);
}

GR_MacFont::~GR_MacFont ()
{
	OSStatus err;
	if (m_fontStyle) {
		err = ATSUDisposeStyle (m_fontStyle);
		UT_ASSERT (err == noErr);
	}
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
	
	return (UT_uint32) FixRound (ascent);
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
	
	return (UT_uint32)FixRound (descent);
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
	
	return (UT_uint32)FixRound(descent + ascent);
}


UT_uint32 GR_MacFont::getTextWidth (const UT_UCSChar * text) const
{
	OSStatus			status;
	ATSUTextMeasurement begin, end;
	ATSUTextLayout      textToMeasure;
	UT_uint32           textlen;
	
	UT_ASSERT (text != NULL);
	textlen = UT_UCS_strlen (text);
	status = _UCSTextToATSUTextLayout (text, textlen,&textToMeasure);
	UT_ASSERT (status == noErr);
	status = ATSUMeasureText(textToMeasure, 0, textlen, &begin, &end, NULL, NULL);
	UT_ASSERT (status == noErr);
#ifdef DEBUG
	if (status != noErr) 
	{
		UT_DEBUGMSG(("ATSUMeasureText returned %d\n", status));
	}
#endif
	status = ATSUDisposeTextLayout (textToMeasure);
	UT_ASSERT (status == noErr);

	xxx_UT_DEBUGMSG (("HUB: GR_MacFont::getTextWidth() = %d, %d\n", FixRound(end), FixRound(begin)));
	xxx_UT_DEBUGMSG (("HUB: GR_MacFont::getTextWidth() = %d\n", (UT_uint32)FixRound(end - begin)));
	return (UT_uint32)FixRound(end - begin);
}


/*
  From a UNICODE null terminated string (UCS-2) with specified length, create an ATSUTextLayout

  layout is returned on exit. WARNING: you need to dispose it after use.
 */
OSStatus				 
GR_MacFont::_UCSTextToATSUTextLayout (const UT_UCSChar *text, UT_uint32 textlen, ATSUTextLayout * layout) const
{  
	ATSUStyle			style;				// Take the Style that belong to the font.
	UniCharCount		runLengths;
	OSStatus			status;
	
	UT_ASSERT (text != NULL);
	runLengths = textlen;
	status = ATSUCreateStyle(&style);
	UT_ASSERT (status == noErr);
	status = ATSUCreateTextLayoutWithTextPtr (text, 0, textlen, textlen, 1, &runLengths, &style, layout);
	UT_ASSERT (status == noErr);
#ifdef DEBUG
	if (status != noErr) 
	{
		UT_DEBUGMSG(("ATSUCreateTextLayoutWithTextPtr returned %d\n", status));
	}
#endif
	return status;
}


void
GR_MacFont::_initMeasurements ()
{
	// TODO: make sure the style used here is consistent...
	UniCharArrayPtr		uniText;
	UniCharCount		uniTextLen;
	UniCharCount		runLengths;
	OSStatus			status;
	
	_quickAndDirtySetUnicodeTextFromASCII_C_Chars(&uniText, &uniTextLen);
	runLengths = uniTextLen;
	status = ATSUCreateTextLayoutWithTextPtr (uniText, 0, 2, uniTextLen, 1, 
											  &runLengths, &m_fontStyle, &m_MeasurementText);
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


