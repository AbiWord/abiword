/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * Copyright (c) 2001,2002 Tomas Frydrych
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
#include <string.h>
#include <time.h>

#include "fp_TextRun.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "pp_Property.h"
#include "pp_AttrProp.h"
#include "gr_Graphics.h"
#include "pd_Document.h"
#include "pd_Style.h"
#include "gr_DrawArgs.h"
#include "fv_View.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_growbuf.h"
#include "ut_units.h"

#include "xap_EncodingManager.h"

#include "ut_OverstrikingChars.h"
#include "ut_Language.h"
#include <fribidi/fribidi.h>
#include "ut_contextGlyph.h"
#include "ap_Prefs.h"

/*****************************************************************/

//we cache pref value UseContextGlyphs; the following defines how often
//to refresh the cache (in miliseconds)
#define PREFS_REFRESH_MSCS 2000
//inicialise the static members of the class
bool fp_TextRun::s_bUseContextGlyphs = true;
bool fp_TextRun::s_bSaveContextGlyphs = false;
bool fp_TextRun::s_bBidiOS = false;
UT_Timer * fp_TextRun::s_pPrefsTimer = 0;
UT_uint32  fp_TextRun::s_iClassInstanceCount = 0;
UT_sint32 * fp_TextRun::s_pCharAdvance = NULL;
UT_uint32  fp_TextRun::s_iCharAdvanceSize = 0;
UT_UCSChar getMirrorChar(UT_UCSChar c);


fp_TextRun::fp_TextRun(fl_BlockLayout* pBL,
					   GR_Graphics* pG,
					   UT_uint32 iOffsetFirst,
					   UT_uint32 iLen,
					   bool bLookupProperties)
:	fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_TEXT),
	m_fPosition(TEXT_POSITION_NORMAL),
	m_bSquiggled(false),
	m_pLanguage(NULL),
	m_bIsOverhanging(false),
	m_pJustifiedSpaces(NULL)
{
	_setField(NULL);

    // we will use this as an indication that the direction
    // property has not been yet set normal values are -1,0,1
    // (neutral, ltr, rtl)
	_setDirection(FRIBIDI_TYPE_UNSET);

    // no override by default
	m_iDirOverride = FRIBIDI_TYPE_UNSET;

	if (bLookupProperties)
	{
		lookupProperties();
	}

	_setRecalcWidth(true);
	markDrawBufferDirty();

	if(!s_pPrefsTimer)
	{
		XAP_App::getApp()->getPrefsValueBool((XML_Char*)XAP_PREF_KEY_UseContextGlyphs, &s_bUseContextGlyphs);
		XAP_App::getApp()->getPrefsValueBool((XML_Char*)XAP_PREF_KEY_SaveContextGlyphs, &s_bSaveContextGlyphs);
		s_pPrefsTimer = UT_Timer::static_constructor(_refreshPrefs, NULL);
		if(s_pPrefsTimer)
			s_pPrefsTimer->set(PREFS_REFRESH_MSCS);
	}

#ifndef WITH_PANGO
	m_pSpanBuff = new UT_UCSChar[getLength() + 1];
	m_iSpanBuffSize = getLength();
	UT_ASSERT(m_pSpanBuff);
#else
	m_pGlyphString = pango_glyph_string_new();
#endif

	if(!s_iClassInstanceCount)
	{
		s_bBidiOS = XAP_App::getApp()->theOSHasBidiSupport();
		UT_DEBUGMSG(("fp_TextRun size is %d\n",sizeof(fp_TextRun) ));
	}

	s_iClassInstanceCount++;
}

fp_TextRun::~fp_TextRun()
{
	--s_iClassInstanceCount;
	if(!s_iClassInstanceCount)
	{
		DELETEP(s_pPrefsTimer);
		delete [] s_pCharAdvance;
		s_pCharAdvance = NULL;
		s_iCharAdvanceSize = 0;
	}

#ifndef WITH_PANGO
	delete[] m_pSpanBuff;
#else
	pango_glyph_string_free();
#endif

	delete getRevisions();
	delete m_pJustifiedSpaces;
}

bool fp_TextRun::hasLayoutProperties(void) const
{
	return true;
}

void fp_TextRun::_lookupProperties(const PP_AttrProp * pSpanAP,
									const PP_AttrProp * pBlockAP,
									const PP_AttrProp * pSectionAP)
{
	// we should only need this if the props have changed
	//clearScreen();

	bool bChanged = false;
	

	fd_Field * fd = NULL;
	static_cast<fl_Layout *>(getBlock())->getField(getBlockOffset(),fd);
	_setField(fd);
	// look for fonts in this DocLayout's font cache
	FL_DocLayout * pLayout = getBlock()->getDocLayout();

	PD_Document * pDoc = getBlock()->getDocument();

	const PP_PropertyTypeColor *p_color = (const PP_PropertyTypeColor *)PP_evalPropertyType("color",pSpanAP,pBlockAP,pSectionAP, Property_type_color, pDoc, true);
	UT_ASSERT(p_color);
	_setColorFG(p_color->getColor());

	const XML_Char* pszStyle = NULL;
	if(pSpanAP && pSpanAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, pszStyle))
	{
		PD_Style *pStyle = NULL;
		pDoc->getStyle((const char*) pszStyle, &pStyle);
		if(pStyle) pStyle->used(1);
	}


	const XML_Char *pszFontStyle = PP_evalProperty("font-style",pSpanAP,pBlockAP,pSectionAP, pDoc, true);
	m_bIsOverhanging = (pszFontStyle && !UT_strcmp(pszFontStyle, "italic"));

	const XML_Char *pszDecor = PP_evalProperty("text-decoration",pSpanAP,pBlockAP,pSectionAP, pDoc, true);

	/*
	  TODO map line width to a property, not a hard-coded value
	*/
	bChanged |= _setLineWidth(getGR()->convertDimension("0.8pt"));

	UT_uint32 oldDecors = _getDecorations();
	_setDecorations(0);

	XML_Char* p;
	if (!UT_cloneString((char *&)p, pszDecor))
	{
		// TODO outofmem
	}
	UT_ASSERT(p || !pszDecor);
	XML_Char*	q = strtok(p, " ");

	while (q)
	{
		if (0 == UT_strcmp(q, "underline"))
		{
			_orDecorations(TEXT_DECOR_UNDERLINE);
		}
		else if (0 == UT_strcmp(q, "overline"))
		{
			_orDecorations(TEXT_DECOR_OVERLINE);
		}
		else if (0 == UT_strcmp(q, "line-through"))
		{
		    _orDecorations(TEXT_DECOR_LINETHROUGH);
		}
		else if (0 == UT_strcmp(q, "topline"))
		{
			_orDecorations(TEXT_DECOR_TOPLINE);
		}
		else if (0 == UT_strcmp(q, "bottomline"))
		{
			_orDecorations(TEXT_DECOR_BOTTOMLINE);
		}
		q = strtok(NULL, " ");
	}

	free(p);

	bChanged |= (_getDecorations() != oldDecors);
	
	const XML_Char * pszPosition = PP_evalProperty("text-position",pSpanAP,pBlockAP,pSectionAP, pDoc, true);

	UT_Byte oldPos = m_fPosition;
	
	if (0 == UT_strcmp(pszPosition, "superscript"))
	{
		m_fPosition = TEXT_POSITION_SUPERSCRIPT;
	}
	else if (0 == UT_strcmp(pszPosition, "subscript"))
	{
		m_fPosition = TEXT_POSITION_SUBSCRIPT;
	}
	else m_fPosition = TEXT_POSITION_NORMAL;

	bChanged |= (oldPos != m_fPosition);
	
#ifndef WITH_PANGO
	GR_Font * pFont;

	pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);
	if (_getScreenFont() != pFont)
	  {
		_setRecalcWidth(true);
		_setScreenFont(pFont);
		_setAscent(getGR()->getFontAscent(_getScreenFont()));
		_setDescent(getGR()->getFontDescent(_getScreenFont()));
		_setHeight(getGR()->getFontHeight(_getScreenFont()));
		bChanged = true;
	  }

	pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_LAYOUT_RESOLUTION);
	if (_getLayoutFont() != pFont)
	  {
		_setRecalcWidth(true);
		_setLayoutFont(pFont);
		_setAscentLayoutUnits(getGR()->getFontAscent(_getLayoutFont()));
		UT_ASSERT(getAscentInLayoutUnits());
		_setDescentLayoutUnits(getGR()->getFontDescent(_getLayoutFont()));
		_setHeightLayoutUnits(getGR()->getFontHeight(_getLayoutFont()));
		bChanged = true;
	  }
#else
	PangoFont * pFont;

	pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP);
	if (_getPangoFont() != pFont)
	  {
		_setRecalcWidth(true);
		_setPangoFont(pFont);
		_setAscent(getGR()->getFontAscent(_getScreenFont()));
		_setDescent(getGR()->getFontDescent(_getScreenFont()));
		_setHeight(getGR()->getFontHeight(_getScreenFont()));
		bChanged = true;
	  }
#endif

#ifndef WITH_PANGO
	getGR()->setFont(_getScreenFont());
#else
	getGR()->setFont(_getPangoFont());
#endif

	//set the language member
	UT_Language *lls = new UT_Language;
	const XML_Char * pszLanguage = PP_evalProperty("lang",pSpanAP,pBlockAP,pSectionAP, pDoc, true);

	const XML_Char * pszOldLanguage = m_pLanguage;
	m_pLanguage = lls->getPropertyFromProperty(pszLanguage);
	if(pszOldLanguage && m_pLanguage != pszOldLanguage)
	{
		
		getBlock()->getDocLayout()->queueBlockForBackgroundCheck((UT_uint32) FL_DocLayout::bgcrSpelling, getBlock());
		bChanged = true;
	}
	
	delete lls;

#ifdef SMART_RUN_MERGING
	FriBidiCharType iOldOverride = m_iDirOverride;
#endif
	FriBidiCharType iNewOverride;
	const XML_Char *pszDirection = PP_evalProperty("dir-override",pSpanAP,pBlockAP,pSectionAP, pDoc, true);
	// the way MS Word handles bidi is peculiar and requires that we allow
	// temporarily a non-standard value for the dir-override property
	// called "nobidi"
	if(!pszDirection)
		iNewOverride = FRIBIDI_TYPE_UNSET;
	else if(!UT_stricmp(pszDirection, "ltr"))
		iNewOverride = FRIBIDI_TYPE_LTR;
	else if(!UT_stricmp(pszDirection, "rtl"))
		iNewOverride = FRIBIDI_TYPE_RTL;
	else
		iNewOverride = FRIBIDI_TYPE_UNSET;

#ifdef SMART_RUN_MERGING

	bChanged |= (iOldOverride != iNewOverride);
	
	/*
		OK, if the previous direction override was strong, and the new one is not (i.e., if
		we are called because the user removed an override from us) we have to split this
		run into chunks with uniform Unicode directional property

		if the previous direction override was not strong, and the current one is, we have
		to break this run's neighbours
	*/
	if(iNewOverride == FRIBIDI_TYPE_UNSET && iOldOverride != FRIBIDI_TYPE_UNSET)
	{
		// we have to do this without applying the new override otherwise the
		// LTR and RTL run counters of the present line will be messed up;
		// breakMeAtDirBoundaries will take care of applying the new override
		breakMeAtDirBoundaries(iNewOverride);
	}
	else if(iNewOverride != FRIBIDI_TYPE_UNSET && iOldOverride == FRIBIDI_TYPE_UNSET)
	{
		// first we have to apply the new override
		setDirection(FRIBIDI_TYPE_UNSET, iNewOverride);
		// now do the breaking
		breakNeighborsAtDirBoundaries();
	}
	else
#endif
		setDirection(FRIBIDI_TYPE_UNSET, iNewOverride);

	if(bChanged)
		clearScreen();
	xxx_UT_DEBUGMSG(("fp_TextRun::lookupProperties: bChanged %d\n", (UT_uint32) bChanged));
}


bool fp_TextRun::canBreakAfter(void) const
{
	const UT_UCSChar* pSpan = NULL;
	UT_uint32 lenSpan = 0;
	UT_uint32 offset = getBlockOffset();
	UT_uint32 len = getLength();
	bool bContinue = true;

	if (len > 0)
	{
		while (bContinue)
		{
			bContinue = getBlock()->getSpanPtr(offset, &pSpan, &lenSpan);

			if(!bContinue)
			{
				// this block has got a text run but no span in the PT,
				// this is clearly a bug
				// not sure want we should return !!!
				UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
				return false;
			}
			
			UT_ASSERT(lenSpan>0);

			if (len <= lenSpan)
			{
				UT_ASSERT(len>0);

				if (XAP_EncodingManager::get_instance()->can_break_at(pSpan[len-1]))
				{
					return true;
				}

				bContinue = false;
			}
			else
			{
				offset += lenSpan;
				len -= lenSpan;
			}
		}
	}
	else if (!getNext())
	{
		return true;
	}

	if (getNext())
	{
		return getNext()->canBreakBefore();
	}

	return false;
}

bool fp_TextRun::canBreakBefore(void) const
{
	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;

	if (getLength() > 0)
	{
		if (getBlock()->getSpanPtr(getBlockOffset(), &pSpan, &lenSpan))
		{
			UT_ASSERT(lenSpan>0);

			if (XAP_EncodingManager::get_instance()->can_break_at(pSpan[0]))
			{
				return true;
			}
		}
	}
	else
	{
		if (getNext())
		{
			return getNext()->canBreakBefore();
		}
		else
		{
			return true;
		}
	}

	return false;
}

bool fp_TextRun::alwaysFits(void) const
{
	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;

	// TODO we need to fix this code to use getSpanPtr the way it is used elsewhere in this file.

	if (getLength() > 0)
	{
		if (getBlock()->getSpanPtr(getBlockOffset(), &pSpan, &lenSpan))
		{
			UT_ASSERT(lenSpan>0);

			for (UT_uint32 i=0; i<lenSpan; i++)
			{
				if (pSpan[i] != UCS_SPACE)
				{
					return false;
				}
			}

			return true;
		}

		return false;
	}

	// could assert here -- this should never happen, I think
	return true;
}

/*
 Determine best split point in Run
 \param iMaxLeftWidth Width to split at
 \retval si Split information (left width, right width, and position)
 \param bForce Force a split at first opportunity (max width)
 \return True if split point was found in this Run, otherwise false.
*/
bool	fp_TextRun::findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, bool bForce)
{
	UT_GrowBuf * pgbCharWidths = getBlock()->getCharWidths()->getCharWidthsLayoutUnits();
	UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

	UT_sint32 iLeftWidth = 0;
	UT_sint32 iRightWidth = getWidthInLayoutUnits();

	si.iOffset = -1;

	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;
	UT_uint32 offset = getBlockOffset();
	UT_uint32 len = getLength();
	bool bContinue = true;

	while (bContinue)
	{
		bContinue = getBlock()->getSpanPtr(offset, &pSpan, &lenSpan);

		if(!bContinue)
		{
			// this block has got a text run but no span in the PT,
			// this is clearly a bug
			UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
			return false;
		}
		
		UT_ASSERT(lenSpan>0);

		if (lenSpan > len)
		{
			lenSpan = len;
		}

		for (UT_uint32 i=0; i<lenSpan; i++)
		{
			UT_sint32 iCW = pCharWidths[i + offset] > 0 ? pCharWidths[i + offset] : 0;
			iLeftWidth += iCW;
			iRightWidth -= iCW;
#if 0
	/*
	FIXME: this is a direct equivalent to HJ's patch, but other branch
	could be more correct than this one. - VH
	*/
			if ( (XAP_EncodingManager::get_instance()->can_break_at(pSpan[i]) && pSpan[i]!=UCS_SPACE) ||
				(
					(UCS_SPACE == pSpan[i])
#else
			if (
				(XAP_EncodingManager::get_instance()->can_break_at(pSpan[i])
#endif
					&& ((i + offset) != (getBlockOffset() + getLength() - 1))
					)
				|| bForce
				)
			{
				if (iLeftWidth - iCW <= iMaxLeftWidth)
				{
					si.iLeftWidth = iLeftWidth;
					si.iRightWidth = iRightWidth;
					si.iOffset = i + offset;
				}
				else
				{
					bContinue = false;
					break;
				}
			}
		}

		if (len <= lenSpan)
		{
			bContinue = false;
		}
		else
		{
			offset += lenSpan;
			len -= lenSpan;
		}
	}

	if (
		(si.iOffset == -1)
		|| (si.iLeftWidth == getWidthInLayoutUnits())
		)
	{
		// there were no split points which fit.
		return false;
	}


	return true;
}

void fp_TextRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/,
								 PT_DocPosition& pos, bool& bBOL, bool& bEOL)
{
	FriBidiCharType iVisDirection = getVisDirection();
	FriBidiCharType iDomDirection = getBlock()->getDominantDirection();

	if (x <= 0)
	{
		if(iVisDirection == FRIBIDI_TYPE_RTL)
		{
			pos = getBlock()->getPosition() + getBlockOffset() + getLength();
			if(iDomDirection == FRIBIDI_TYPE_RTL)
			{
				bEOL = true;
				bBOL = false;
			}
			else
			{
				bEOL = false;
				bBOL = true;
			}
		}
		else
		{
			pos = getBlock()->getPosition() + getBlockOffset();
			// don't set bBOL to false here
			bEOL = false;
		}

		return;
	}

	if (x >= getWidth())
	{
		if(iVisDirection == FRIBIDI_TYPE_RTL)
		{
			pos = getBlock()->getPosition() + getBlockOffset();

			if(iDomDirection == FRIBIDI_TYPE_RTL)
			{
				bEOL = false;
				bBOL = true;
			}
			else
			{
				bEOL = true;
				bBOL = false;
			}
		}
		else
		{
			pos = getBlock()->getPosition() + getBlockOffset() + getLength();
			// Setting bEOL fixes bug 1149. But bEOL has been set in the
			// past - probably somewhere else, so this is not necessarily
			// the correct place to do it.	2001.02.25 jskov
			bEOL = true;
		}

		return;
	}

	const UT_GrowBuf * pgbCharWidths = getBlock()->getCharWidths()->getCharWidths();
	const UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

	// catch the case of a click directly on the left half of the
	// first character in the run
	UT_sint32 iCW = pCharWidths[getBlockOffset()] > 0 ? pCharWidths[getBlockOffset()] : 0;
	
	if (x < (iCW / 2))
	{
		pos = getBlock()->getPosition() + getOffsetFirstVis();

		// if this character is RTL then clicking on the left side
		// means the user wants to postion the caret _after_ this char
		if(iVisDirection == FRIBIDI_TYPE_RTL)
			pos++;

		bBOL = false;
		bEOL = false;
		return;
	}

	UT_sint32 iWidth = 0;
	for (UT_uint32 i=getBlockOffset(); i<(getBlockOffset() + getLength()); i++)
	{
		// i represents VISUAL offset but the CharWidths array uses logical order of indexing
		UT_uint32 iLog = getOffsetLog(i);
		UT_uint32 iCW = pCharWidths[iLog] > 0 ? pCharWidths[iLog] : 0;

		iWidth += iCW;

		if (iWidth > x)
		{
			if (((iWidth - x) <= (pCharWidths[iLog] / 2)
				 && (iVisDirection == FRIBIDI_TYPE_LTR))
				|| (((iWidth - x) > (pCharWidths[iLog] / 2)
					 && (iVisDirection == FRIBIDI_TYPE_RTL))
					))
			{
				iLog++;
			}

			// NOTE: this allows inserted text to be coalesced in the PT
			bEOL = true;
			pos = getBlock()->getPosition() + iLog;
			return;
		}
	}

	UT_DEBUGMSG(("fp_TextRun::mapXYToPosition: x %d, m_iWidth %d\n", x,getWidth()));
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void fp_TextRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	UT_sint32 xoff;
	UT_sint32 yoff;
	UT_sint32 xoff2;
	UT_sint32 yoff2;
	UT_sint32 xdiff = 0;

	UT_ASSERT(getLine());
	getLine()->getOffsets(this, xoff, yoff);
	const UT_GrowBuf * pgbCharWidths = getBlock()->getCharWidths()->getCharWidths();
	const UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

	UT_uint32 offset = UT_MIN(iOffset, getBlockOffset() + getLength());

	for (UT_uint32 i=getBlockOffset(); i<offset; i++)
	{
		UT_uint32 iCW = pCharWidths[i] > 0 ? pCharWidths[i] : 0;
		xdiff += iCW;
	}

	if (m_fPosition == TEXT_POSITION_SUPERSCRIPT)
	{
		yoff -= getAscent() * 1/2;
	}
	else if (m_fPosition == TEXT_POSITION_SUBSCRIPT)
	{
		yoff += getDescent() /* * 3/2 */;
	}

	UT_sint32 iDirection = getVisDirection();
	UT_sint32 iNextDir = iDirection == FRIBIDI_TYPE_RTL ? FRIBIDI_TYPE_LTR : FRIBIDI_TYPE_RTL; //if this is last run we will anticipate the next to have *different* direction
	fp_Run * pRun = 0;	 //will use 0 as indicator that there is no need to deal with the second caret

	if(offset == (getBlockOffset() + getLength())) //this is the end of the run
	{
		pRun = getNext();

		if(pRun)
		{
			iNextDir = pRun->getVisDirection();
			pRun->getLine()->getOffsets(pRun, xoff2, yoff2);
			// if the next run is the end of paragraph marker,
			// we need to derive yoff2 from the offset of this
			// run instead of the marker
			if(pRun->getType() == FPRUN_ENDOFPARAGRAPH)
				yoff2 = yoff;
		}
	}

	if(iDirection == FRIBIDI_TYPE_RTL)				   //#TF rtl run
	{
		x = xoff + getWidth() - xdiff; //we want the caret right of the char
	}
	else
	{
		x = xoff + xdiff;
	}

	if(pRun && (iNextDir != iDirection)) //followed by run of different direction, have to split caret
	{
		x2 = (iNextDir == FRIBIDI_TYPE_LTR) ? xoff2 : xoff2 + pRun->getWidth();
		y2 = yoff2;
	}
	else
	{
		x2 = x;
		y2 = yoff;
	}

	bDirection = (iDirection != FRIBIDI_TYPE_LTR);
	y = yoff;
	height = getHeight();
}

bool fp_TextRun::canMergeWithNext(void)
{
	if (!getNext() ||
		!getLine() ||
		getNext()->getType() != FPRUN_TEXT ||
		!getNext()->getLine())
	{
		return false;
	}


	fp_TextRun* pNext = static_cast<fp_TextRun*>(getNext());

	UT_sint32 iSpaceWidth  = _getSpaceWidthBeforeJustification();
	UT_sint32 iNSpaceWidth = pNext->_getSpaceWidthBeforeJustification();
	
	if (
		(pNext->getBlockOffset() != (getBlockOffset() + getLength()))
		|| (pNext->_getDecorations() != _getDecorations())
		|| (pNext->_getScreenFont() != _getScreenFont())
		|| (pNext->_getLayoutFont() != _getLayoutFont())
		|| (getHeight() != pNext->getHeight())

		// with the new algorithm we can merge if at least one run does not use
		// justification or both have the same value (if one does not
		// use, it probably does not contain spaces, if it does the
		// merge comes a result of new line breaking and this will be
		// followed by block format in which the justification will be
		// recalculated anyway)
		|| (   iSpaceWidth  != JUSTIFICATION_NOT_USED
			&& iNSpaceWidth != JUSTIFICATION_NOT_USED
			&& iNSpaceWidth != iSpaceWidth
			)

		|| (pNext->getField() != getField())
		|| (pNext->m_pLanguage != m_pLanguage)	//this is not a bug
		|| (pNext->_getColorFG() != _getColorFG())
		|| (pNext->_getColorHL() != _getColorHL())
		|| (pNext->m_fPosition != m_fPosition)
#ifdef SMART_RUN_MERGING
		|| (pNext->getVisDirection() != getVisDirection())
		// we also want to test the override, because we do not want runs that have the same
		// visual direction but different override merged
#else
		|| (pNext->_getDirection() != _getDirection())  //#TF cannot merge runs of different direction of writing
#endif
		|| (pNext->m_iDirOverride != m_iDirOverride)

		// cannot merge two runs within different hyperlinks, but than
		// this can never happen, since between them alwasy will be at
		// least two hyperlink runs.

		/* the revision evaluation is a bit more complex*/
		|| ((  getRevisions() != pNext->getRevisions()) // non-identical and one is null
			&& (!getRevisions() || !pNext->getRevisions()))
		|| ((  getRevisions() && pNext->getRevisions())
		    && !(*getRevisions() == *(pNext->getRevisions()))) //
															   //non-null but different
		|| (pNext->isHidden() != isHidden())
		)
	{
		return false;
	}

    return true;
}

void fp_TextRun::mergeWithNext(void)
{
	UT_ASSERT(getNext() && (getNext()->getType() == FPRUN_TEXT));
	UT_ASSERT(getLine());
	UT_ASSERT(getNext()->getLine());

	fp_TextRun* pNext = (fp_TextRun*) getNext();

	UT_ASSERT(pNext->getBlockOffset() == (getBlockOffset() + getLength()));
	UT_ASSERT(pNext->_getScreenFont() == _getScreenFont());	// is this legal?
	UT_ASSERT(pNext->_getLayoutFont() == _getLayoutFont());	// is this legal?
	UT_ASSERT(pNext->_getDecorations() == _getDecorations());
	UT_ASSERT(getAscent() == pNext->getAscent());
	UT_ASSERT(getDescent() == pNext->getDescent());
	UT_ASSERT(getHeight() == pNext->getHeight());
	UT_ASSERT(_getLineWidth() == pNext->_getLineWidth());
	UT_ASSERT(m_pLanguage == pNext->m_pLanguage); //this is not a bug
	UT_ASSERT(m_fPosition == pNext->m_fPosition);
#ifndef SMART_RUN_MERGING
	UT_ASSERT(_getDirection() == pNext->_getDirection()); //#TF
#endif
	UT_ASSERT(m_iDirOverride == pNext->m_iDirOverride); //#TF
	//UT_ASSERT(m_iSpaceWidthBeforeJustification == pNext->m_iSpaceWidthBeforeJustification);

	_setField(pNext->getField());

	xxx_UT_DEBUGMSG(("fp_TextRun::mergeWithNext\n"));
	// first of all, make sure the X coordinance of the merged run is correct

	if(getX() > pNext->getX())
		_setX(pNext->getX());

	// here comes the the handling of justified runs
	if(m_pJustifiedSpaces && pNext->m_pJustifiedSpaces)
	{
		// this is the case where both runs are justified
		
		UT_uint32 iCount = m_pJustifiedSpaces->getItemCount();
		UT_uint32 iNCount = pNext->m_pJustifiedSpaces->getItemCount();
		UT_ASSERT( iCount && iNCount );

		if(_getSpaceWidthBeforeJustification() == JUSTIFICATION_FAKE)
		{
			// fake-ly justified runs can only be merged with other
			// fake-ly justified runs
			UT_ASSERT( pNext->_getSpaceWidthBeforeJustification() == JUSTIFICATION_FAKE );
			
			// there is nothing more to do, since these runs do not
			// carry any data with them
		}
		else
		{
			
			// get the last slot from our vector and the first slot from
			// the next vector and see if they are continuous
			//
			// I have been toying with the idea to optimize this by
			// reusing the longer of the two vectors, rather than the
			// one associated with the current run, but in the end I
			// decided against it. When we reuse the vector of the
			// current run, we only need to add data by
			// UT_Vector::addItem(), which is painless. If we were
			// reusing the vector from the pNext run, we would have to
			// be inserting data at the start of it using
			// UT_Vector::insertItemAt(), which always involves
			// shifting the entire contents of the vector to the right
			// and most likely would cost us more than it would save us
		
			// NB: the following offsets are relative to their
			// respective runs, not the block
			UT_sint32 iOrigSpaceWidth = (UT_sint32) m_pJustifiedSpaces->getNthItem(0);
			UT_sint32 iOrigOffset = (UT_sint32) m_pJustifiedSpaces->getNthItem(1);
			UT_uint32 iBlOffset = getBlockOffset();
			UT_sint32 iDelta = iBlOffset - iOrigOffset;
			UT_uint32 iSpaceOffset = (UT_uint32) m_pJustifiedSpaces->getNthItem(iCount - 4) + iDelta;
			UT_uint32 iSpaceLength = (UT_uint32) m_pJustifiedSpaces->getNthItem(iCount - 3);

			UT_sint32 iNOrigSpaceWidth = (UT_sint32) m_pJustifiedSpaces->getNthItem(0);
			UT_sint32 iNOrigOffset = (UT_sint32) pNext->m_pJustifiedSpaces->getNthItem(1);
			UT_uint32 iNBlOffset = pNext->getBlockOffset();
			UT_sint32 iNDelta = iNBlOffset - iNOrigOffset;
			UT_uint32 iNSpaceOffset = (UT_uint32) pNext->m_pJustifiedSpaces->getNthItem(2) + iNDelta;
			UT_uint32 iNSpaceLength = (UT_uint32) pNext->m_pJustifiedSpaces->getNthItem(3);

			UT_uint32 iVectIndx = 2;

			if(iSpaceOffset + iBlOffset + iSpaceLength == iNSpaceOffset + iNBlOffset)
			{
				// the two space slots are continuous, so we join them
				m_pJustifiedSpaces->setNthItem(iCount - 3, (void*) (iSpaceLength + iNSpaceLength),NULL);
				UT_uint32 iSpaceWidth = (UT_uint32) m_pJustifiedSpaces->getNthItem(iCount - 2);
				iSpaceWidth += (UT_uint32) pNext->m_pJustifiedSpaces->getNthItem(4);
				m_pJustifiedSpaces->setNthItem(iCount - 2, (void*) iSpaceWidth, NULL);

				iVectIndx = 6;
			}
			else
			{
				// the two slots are not continuous, so we only need
				// to append the contents of pNext->m_pJustifiedSpaces
				// to this->m_pJustifiedSpaces, but first we have to
				// adjust the text width of the first data segment
				// from pNext by the width of the span after the last
				// space in this run
				UT_uint32 iTextBlOffset = iSpaceOffset + iSpaceLength + iBlOffset;
				UT_uint32 iLastBlOffset = iBlOffset + getLength();
				UT_GrowBuf * pgbCharWidths = getBlock()->getCharWidths()->getCharWidths();
				UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);
				UT_uint32 iTextWidth = (UT_uint32)pNext->m_pJustifiedSpaces->getNthItem(iVectIndx+3);

				for(UT_uint32 k = iTextBlOffset; k < iLastBlOffset; k++)
				{
					UT_uint32 iCW = pCharWidths[k] > 0 ? pCharWidths[k] : 0;
					iTextWidth += iCW;
				}
				
				// we also have to adjust offsets so they are relative to
				// iDelta, not iNDelta
				UT_sint32 iMyOffset = (UT_uint32)pNext->m_pJustifiedSpaces->getNthItem(iVectIndx);
				iMyOffset += (2*iNBlOffset - iNOrigOffset - 2*iBlOffset + iOrigOffset);
				m_pJustifiedSpaces->addItem((void*)iMyOffset);

				// add the length and space width
				for(UT_uint32 j  = iVectIndx+1; j < iVectIndx+3; j++)
					m_pJustifiedSpaces->addItem(pNext->m_pJustifiedSpaces->getNthItem(j));

				// add the text width
				m_pJustifiedSpaces->addItem((void*)iTextWidth);

				iVectIndx += 4;
			}
			

			for(UT_uint32 i = iVectIndx; i < iNCount; i += 4)
			{
				// we have to adjust offsets so they are relative to
				// iDelta, not iNDelta
				UT_sint32 iMyOffset = (UT_uint32) pNext->m_pJustifiedSpaces->getNthItem(i);
				iMyOffset += (2*iNBlOffset - iNOrigOffset - 2*iBlOffset + iOrigOffset);
				m_pJustifiedSpaces->addItem((void*)iMyOffset);

				// copy the length, width and text width
				for(UT_uint32 j  = i+1; j < i+4; j++)
					m_pJustifiedSpaces->addItem(pNext->m_pJustifiedSpaces->getNthItem(j));
			}
		}
	}
	else
	{
		// we possibly have one run with justified spaces in it and
		// other without, we keep the info from the one that has them
		if(pNext->m_pJustifiedSpaces)
		{
			m_pJustifiedSpaces = pNext->m_pJustifiedSpaces;
			pNext->m_pJustifiedSpaces = NULL;

			// now we have to adjust the original offset, so that we
			// get correct iDelta in our calculations (but only if
			// this justification is not of JUSTIFICATION_FAKE kind)
			// 
			// to get the correct iDelta from the stored values we
			// have to  subtract twice the length of this run from
			// the stored original offset -- if you do not believe me,
			// here come the equations (where bo is block offset, so
			// is space offset, oo is original offset, of is
			// offset-first, L is length, index 1 refers to the
			// current run and index 2 to the next run):
			//
			//   bo = so + (of2 - oo2) + of2 = so + 2 of1 + 2 L1 - oo2
			//   bo = so + (of1 - oo1) + of1 = so + 2 of1 - oo1
			//   so + 2 of1 + 2 L1 - oo2 = so + 2 of1 - oo1
			//   2 L1 - oo2 = -oo1
			//   oo1 = oo2 - 2L1
			//
			// the consequence of this is, that the stored original
			// offset can be negative, even though negative block
			// offset is something that normally does not occur
			//
			// the other thing that we need to adjust here is the text
			// width in the first data segement; this is very simple,
			// since because this run originally did not have any
			// spaces with adjuste width, we add to the first text
			// width the width of this run
			
			if(_getSpaceWidthBeforeJustification() != JUSTIFICATION_FAKE)
			{
				UT_sint32 iOrigOffset = (UT_sint32)m_pJustifiedSpaces->getNthItem(1);
				iOrigOffset -= 2*getLength();
				m_pJustifiedSpaces->setNthItem(1, (void*) iOrigOffset, NULL);

				UT_uint32 iTextWidth = (UT_uint32)m_pJustifiedSpaces->getNthItem(5) + getWidth();
				m_pJustifiedSpaces->setNthItem(5, (void*) iTextWidth, NULL);
			}
		}
	}

	// can only adjust width after the justification has been handled
 	_setWidth(getWidth() + pNext->getWidth());
#ifndef WITH_PANGO
	_setWidthLayoutUnits(getWidthInLayoutUnits() +
						 pNext->getWidthInLayoutUnits());
#endif

	// join the two span buffers; this will save us refreshing the draw buffer
	// which is very expensive
	// however, first make sure that the buffers are uptodate (_refreshDrawBuffer
	// decides whether this is needed or not)
	_refreshDrawBuffer();
	pNext->_refreshDrawBuffer();

	// we need to take into consideration whether this run has been reversed
	// in which case the order of the concating needs to be reversed too
	FriBidiCharType iVisDirection = getVisDirection();

	bool bReverse = (!s_bBidiOS && iVisDirection == FRIBIDI_TYPE_RTL)
		  || (s_bBidiOS && m_iDirOverride == FRIBIDI_TYPE_LTR && _getDirection() == FRIBIDI_TYPE_RTL);

	if((m_iSpanBuffSize <= getLength() + pNext->getLength()) || (bReverse && (getLength() > pNext->getLength())))
	{
		xxx_UT_DEBUGMSG(("fp_TextRun::mergeWithNext: reallocating span buffer\n"));
		m_iSpanBuffSize = getLength() + pNext->getLength() + 1;
		UT_UCSChar * pSB = new UT_UCSChar[m_iSpanBuffSize];
		UT_ASSERT(pSB);
		if(bReverse)
		{
			UT_UCS4_strncpy(pSB, pNext->m_pSpanBuff, pNext->getLength());
			UT_UCS4_strncpy(pSB + pNext->getLength(),m_pSpanBuff, getLength());
		}
		else
		{
			UT_UCS4_strncpy(pSB,m_pSpanBuff, getLength());
			UT_UCS4_strncpy(pSB + getLength(), pNext->m_pSpanBuff, pNext->getLength());
		}

		*(pSB + getLength() + pNext->getLength()) = 0;
		delete [] m_pSpanBuff;
		m_pSpanBuff = pSB;
	}
	else
	{
		UT_DEBUGMSG(("fp_TextRun::mergeWithNext: reusing existin span buffer\n"));
		if(bReverse)
		{
			// can only shift the text directly in the existing buffer if
			// getLength() <= pNext->getLength()
			UT_ASSERT(getLength() <= pNext->getLength());
			UT_UCS4_strncpy(m_pSpanBuff + pNext->getLength(), m_pSpanBuff, getLength());
			UT_UCS4_strncpy(m_pSpanBuff, pNext->m_pSpanBuff, pNext->getLength());
		}
		else
		{
			UT_UCS4_strncpy(m_pSpanBuff + getLength(), pNext->m_pSpanBuff, pNext->getLength());
		}
		*(m_pSpanBuff + getLength() + pNext->getLength()) = 0;
	}

	setLength(getLength() + pNext->getLength());
	_setDirty(isDirty() || pNext->isDirty());
	setNext(pNext->getNext());
	if (getNext())
	{
		// do not mark anything dirty
		getNext()->setPrev(this, false);
	}

	pNext->getLine()->removeRun(pNext, false);

#ifdef SMART_RUN_MERGING
	// if appending a strong run onto a weak one, make sure the overall direction
	// is that of the strong run, and tell the line about this, since the call
	// to removeRun above decreased the line's direction counter
	if(!FRIBIDI_IS_STRONG(_getDirection()) && FRIBIDI_IS_STRONG(pNext->_getDirection()))
	{
		_setDirection(pNext->_getDirection());
		getLine()->addDirectionUsed(_getDirection());
	}
#endif

	delete pNext;
}

bool fp_TextRun::split(UT_uint32 iSplitOffset)
{
	UT_ASSERT(iSplitOffset >= getBlockOffset());
	UT_ASSERT(iSplitOffset < (getBlockOffset() + getLength()));

	FriBidiCharType iVisDirection = getVisDirection();
	fp_TextRun* pNew = new fp_TextRun(getBlock(), getGR(), iSplitOffset, getLength() - (iSplitOffset - getBlockOffset()), false);

	UT_ASSERT(pNew);
#ifndef WITH_PANGO
	pNew->_setScreenFont(this->_getScreenFont());
	pNew->_setLayoutFont(this->_getLayoutFont());
#else
	pNew->_setPangoFont(this->_getPangoFont());
#endif

	pNew->_setDecorations(this->_getDecorations());
	pNew->_setColorFG(_getColorFG());
	pNew->_setField(this->getField());
	pNew->m_fPosition = this->m_fPosition;

	pNew->_setAscent(this->getAscent());
	pNew->_setDescent(this->getDescent());
	pNew->_setHeight(this->getHeight());
#ifndef WITH_PANGO
	pNew->_setAscentLayoutUnits(this->getAscentInLayoutUnits());
	pNew->_setDescentLayoutUnits(this->getDescentInLayoutUnits());
	pNew->_setHeightLayoutUnits(this->getHeightInLayoutUnits());
#endif
	pNew->_setLineWidth(this->_getLineWidth());
	pNew->_setDirty(isDirty());
	pNew->m_pLanguage = this->m_pLanguage;
	pNew->_setDirection(this->_getDirection()); //#TF
	pNew->m_iDirOverride = this->m_iDirOverride;
#ifdef SMART_RUN_MERGING
	// set the visual direction to same as that of the old run
	pNew->setVisDirection(iVisDirection);
#endif

	pNew->_setHyperlink(this->getHyperlink());

	// when revisions are present, this gets bit trickier
	if(getRevisions() != NULL)
	{
		// the revisions object cannot be shared, we have to
		// recreate one
		// TODO -- this is not a very efficient way of doing
		// this, it would be preferable to design copy constructors
		// for PP_Revision and PP_RevisionAttr and use the copy
		// constructor, but for no this will do
		pNew->_setRevisions(new PP_RevisionAttr(getRevisions()->getXMLstring()));
	}

	pNew->setVisibility(this->isHidden());


	// handle justification
	if(m_pJustifiedSpaces)
	{
		// divide the justification info between the two runs
		if(_getSpaceWidthBeforeJustification() == JUSTIFICATION_FAKE)
		{
			// this run has only faked justification, so it does not
			// carry any real data
			pNew->m_pJustifiedSpaces = new UT_Vector;
			pNew->m_pJustifiedSpaces->addItem((void*)JUSTIFICATION_FAKE);
		}
		else
		{
			
			UT_sint32 iOrigOffset  = (UT_sint32)m_pJustifiedSpaces->getNthItem(1);
			UT_sint32 iDelta = getBlockOffset() - iOrigOffset;

			// calculate new stored offset to produce same iDelta --
			// for explanation of the adjustment see ::mergeWithNext()
			UT_sint32 iNOrigOffset = iOrigOffset + 2*iSplitOffset;

			for(UT_uint32 i = 2; i < m_pJustifiedSpaces->getItemCount(); i += 4)
			{
				UT_uint32 iSpaceOffset = (UT_uint32)m_pJustifiedSpaces->getNthItem(i) + iDelta;
				UT_uint32 iSpaceLength = (UT_uint32)m_pJustifiedSpaces->getNthItem(i+1);

				if(iSplitOffset >= iSpaceOffset + iSpaceLength)
				{
					// move on
				}
				else if(iSplitOffset <= iSpaceOffset)
				{
					// everything from this slot onwards belongs to the
					// new run

					// first try the special case where all the slots will
					// go to the new run
					if(i == 2)
					{
						pNew->m_pJustifiedSpaces = m_pJustifiedSpaces;
						m_pJustifiedSpaces = NULL;

					}
					else
					{
						// only some of the slots need to be transferred
						pNew->m_pJustifiedSpaces = new UT_Vector;
						pNew->m_pJustifiedSpaces->addItem(m_pJustifiedSpaces->getNthItem(0));
						pNew->m_pJustifiedSpaces->addItem(NULL); // dummy placeholder

						// cannot call getItemCount() directly in the
						// loop, since the number changes with each itteration
						UT_uint32 iCount = m_pJustifiedSpaces->getItemCount();
						
						for(UT_uint32 j = i; j < iCount; j++)
						{
							// as we delete the item immediately afterwards,
							// we are always working with the i index of the
							// old vector (not j!)
							pNew->m_pJustifiedSpaces->addItem(m_pJustifiedSpaces->getNthItem(i));
							m_pJustifiedSpaces->deleteNthItem(i);
						}
					}

					// now we have to adjust the stored original
					// offset and the text width stored in the first slot
					pNew->m_pJustifiedSpaces->setNthItem(1,(void*)iNOrigOffset,NULL);

					iSpaceOffset = i >= 6 ? (UT_uint32)m_pJustifiedSpaces->getNthItem(i-4) + iDelta : 0;
					iSpaceLength = i >= 6 ?(UT_uint32)m_pJustifiedSpaces->getNthItem(i-3):0;
					UT_uint32 iBlOffset = getBlockOffset();
					
					UT_uint32 iTextBlOffset =  iSpaceOffset + iSpaceLength + iBlOffset;
					UT_uint32 iLastBlOffset = iBlOffset + iSplitOffset;
					UT_GrowBuf * pgbCharWidths = getBlock()->getCharWidths()->getCharWidths();
					UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);
					UT_uint32 iTextWidth = (UT_uint32)pNew->m_pJustifiedSpaces->getNthItem(5);

					for(UT_uint32 k = iTextBlOffset; k < iLastBlOffset; k++)
					{
						UT_uint32 iCW = pCharWidths[k] > 0 ? pCharWidths[k] : 0;
						iTextWidth -= iCW;
					}
					
					pNew->m_pJustifiedSpaces->setNthItem(5, (void*)iTextWidth, NULL);
					

					break;
				}
				else
				{
					// this is the harder case, where the split offset
					// falls inside a space span, so we have to split the
					// slot 

					// first set the correct offsets and lengths
					// (NB the iSplitOffset has to be recorded with the same
					// iDelta as the rest of the offsets, so that adding
					// iDelta would result in getting the correct offset)
					pNew->m_pJustifiedSpaces = new UT_Vector;
					pNew->m_pJustifiedSpaces->addItem(m_pJustifiedSpaces->getNthItem(0));
					pNew->m_pJustifiedSpaces->addItem(NULL); //dummy placeholder

					pNew->m_pJustifiedSpaces->addItem((void*)(iSplitOffset - iDelta)); //offset
					UT_uint32 iNewLength = iSpaceLength - (iSplitOffset - iSpaceOffset);
					pNew->m_pJustifiedSpaces->addItem((void*)iNewLength); //length
					m_pJustifiedSpaces->setNthItem(i+1,(void*)(iSpaceOffset - iSplitOffset),NULL);//length

					// now divide the width of the spaces
					UT_uint32 iSpaceWidth = (UT_uint32)m_pJustifiedSpaces->getNthItem(i+2);
					UT_uint32 iSingleWidth = iSpaceWidth / iSpaceLength;
					pNew->m_pJustifiedSpaces->addItem((void*)(iSingleWidth * iNewLength));
					m_pJustifiedSpaces->setNthItem(i+2,(void*)(iSingleWidth * (iSpaceLength - iNewLength)),NULL);
					pNew->m_pJustifiedSpaces->addItem(NULL); //text width == 0


					// everything after this slot onwards belongs to the
					// new run

					// cannot call getItemCount() directly in the
					// loop, since the number changes with each itteration
					UT_uint32 iCount = m_pJustifiedSpaces->getItemCount();
					
					for(UT_uint32 j = i+4; j < iCount; j++)
					{
						// as we delete the item immediately afterwards,
						// we are always working with the i index of the
						// old vector (not j!)
						pNew->m_pJustifiedSpaces->addItem(m_pJustifiedSpaces->getNthItem(i));
						m_pJustifiedSpaces->deleteNthItem(i);
					}

					// now we have to adjust the stored original
					// offset
					pNew->m_pJustifiedSpaces->setNthItem(1,(void*)iNOrigOffset,NULL);

					break;
				}
			}
		}
	}


	pNew->setPrev(this);
	pNew->setNext(this->getNext());
	if (getNext())
	{
		// do not mark anything dirty
		getNext()->setPrev(pNew, false);
	}
	setNext(pNew);

	setLength(iSplitOffset - getBlockOffset());

	getLine()->insertRunAfter(pNew, this);

	// first of all, split the span buffer, this will save us refreshing it
	// which is very expensive (see notes on the mergeWithNext())

	// we have a dilema here, either we can leave the span buffer for this run
	// of the size it is now, or we delete it and allocate a shorter one
	// we will use the first option, the extra delete/new pair should not
	// cost us too much time
	m_iSpanBuffSize = getLength() + 1;
	UT_UCSChar * pSB = new UT_UCSChar[m_iSpanBuffSize];

	if((!s_bBidiOS && iVisDirection == FRIBIDI_TYPE_RTL)
	  || (s_bBidiOS && m_iDirOverride == FRIBIDI_TYPE_LTR && _getDirection() == FRIBIDI_TYPE_RTL)
	)
	{
		UT_UCS4_strncpy(pSB, m_pSpanBuff + pNew->getLength(), getLength());
		UT_UCS4_strncpy(pNew->m_pSpanBuff, m_pSpanBuff, pNew->getLength());
	}
	else
	{
		UT_UCS4_strncpy(pSB, m_pSpanBuff, getLength());
		UT_UCS4_strncpy(pNew->m_pSpanBuff, m_pSpanBuff + getLength(), pNew->getLength());
	}

	pSB[getLength()] = 0;
	pNew->m_pSpanBuff[pNew->getLength()] = 0;

	delete[] m_pSpanBuff;
	m_pSpanBuff = pSB;

	// we will use the _addupCharWidths() function here instead of recalcWidth(), since when
	// a run is split the info in the block's char-width array is not affected, so we do not
	//have to recalculate these
	_addupCharWidths();
	pNew->_addupCharWidths();


	//bool bDomDirection = getBlock()->getDominantDirection();

	if(iVisDirection == FRIBIDI_TYPE_LTR)
	{
		pNew->_setX(getX() + getWidth());
	}
	else
	{
		pNew->_setX(getX());
		_setX(getX() + pNew->getWidth());
	}

	pNew->_setY(getY());

	return true;
}

void fp_TextRun::_fetchCharWidths(GR_Font* pFont, UT_GrowBufElement* pCharWidths)
{
	UT_ASSERT(pCharWidths);
	UT_ASSERT(pFont);

	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;
	UT_uint32 offset = getBlockOffset();
	UT_uint32 len = getLength();
	bool bContinue = true;

	getGR()->setFont(pFont);

	while (bContinue)
	{
		bContinue = getBlock()->getSpanPtr(offset, &pSpan, &lenSpan);

		if(!bContinue)
		{
			// a bug, we have block with text runs, but no span ptr !!!
			UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
			return;
		}
		
		UT_ASSERT(lenSpan>0);

		if (len <= lenSpan)
		{
			getGR()->measureString(pSpan, 0, len, pCharWidths + offset);

			bContinue = false;
		}
		else
		{
			getGR()->measureString(pSpan, 0, lenSpan, pCharWidths + offset);

			offset += lenSpan;
			len -= lenSpan;
		}
	}
}
void fp_TextRun::fetchCharWidths(fl_CharWidths * pgbCharWidths)
{
	if (getLength() == 0)
	{
		return;
	}

	if(!s_bUseContextGlyphs)
	{
		// if we are using context glyphs we will do nothing, since this
		// will be done by recalcWidth()
		UT_GrowBufElement* pCharWidths = pgbCharWidths->getCharWidths()->getPointer(0);

#ifndef WITH_PANGO
		_fetchCharWidths(_getScreenFont(), pCharWidths);
		pCharWidths = pgbCharWidths->getCharWidthsLayoutUnits()->getPointer(0);
		_fetchCharWidths(_getLayoutFont(), pCharWidths);
#else
		_fetchCharWidths(_getPangoFont(), pCharWidths);
#endif

	};
}

#ifndef WITH_PANGO
UT_sint32 fp_TextRun::simpleRecalcWidth(UT_sint32 iWidthType, UT_sint32 iLength)
#else
UT_sint32 fp_TextRun::simpleRecalcWidth(UT_sint32 iLength)
#endif
{

	if(iLength == Calculate_full_width)
	{
		iLength = getLength();
	}
	UT_ASSERT(iLength >= 0);

	UT_ASSERT((UT_uint32)iLength <= getLength());
	if((UT_uint32)iLength > getLength())
		iLength = (UT_sint32)getLength();

	if (iLength == 0)
		return 0;


	UT_GrowBuf * pgbCharWidths;
#ifndef WITH_PANGO
	switch(iWidthType)
	{
		case Width_type_display:
			pgbCharWidths = getBlock()->getCharWidths()->getCharWidths();
			break;

		case Width_type_layout_units:
			pgbCharWidths = getBlock()->getCharWidths()->getCharWidthsLayoutUnits();
			break;

		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return 0;
			break;
	}
#else
	pgbCharWidths = getBlock()->getCharWidths()->getCharWidths();
#endif

	UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

	UT_sint32 iWidth = 0;

	{
		_refreshDrawBuffer();

#ifndef WITH_PANGO
		if(iWidthType == Width_type_display)
			getGR()->setFont(_getScreenFont());
		else
			getGR()->setFont(_getLayoutFont());
#endif

		for (UT_sint32 i=0; i<iLength; i++)
		{
#ifndef WITH_PANGO
			// with PANGO this is taken care of by _refreshDrawBuffer()
			if(s_bUseContextGlyphs)
			{
				getGR()->measureString(m_pSpanBuff + i, 0, 1, (UT_GrowBufElement*)pCharWidths + getBlockOffset() + i);
			}
#endif
			UT_uint32 iCW = pCharWidths[i + getBlockOffset()] > 0 ? pCharWidths[i + getBlockOffset()] : 0;
			
			iWidth += iCW;
		}
#ifndef WITH_PANGO
		getGR()->setFont(_getScreenFont());
#endif
	}
	//UT_DEBUGMSG(("fp_TextRun (0x%x)::simpleRecalcWidth: width %d\n", this, iWidth));
	return iWidth;
}

bool fp_TextRun::recalcWidth(void)
{
/*
	The width can only change if the font has changed, the alignment has changed,
	or the run has changed physically, i.e., it has been either split or merged;
	all we need to test is the length -- if the length is the same, it could
	not have changed -- and the screen font, and the justification spaces

	In the Bidi build though when using the automatic glyph shape selection
	the width can also change when the context changes, i.e., we have a new
	next or new prev run. Unfortunately, when the ultimate character in the run
	is the first part of a ligature, we also need to check whether what follows
	the second part of that ligature has not changed -- we have to test for the
	change of the next run of our next run (we will do that for all runs, not
	just ligatures, since there is no fast test whether a character is a
	ligature, and going through the hoops to find out would beat the reasons
	why we test this in the first place).

	Further, we can seriously speed up things for the bidi build, if we
	calculate width here directly, rather than calling simpleRecalcWidth
	since it allows us to carry out only a sinle evaluation of the context
*/
	if(_getRecalcWidth())
	{
		_setRecalcWidth(false);

		// we will call _refreshDrawBuffer() to ensure that the cache of the
		// visual characters is uptodate and then we will measure the chars
		// in the cache
		_refreshDrawBuffer();

#ifndef WITH_PANGO
		// with Pango the width gets recalcuated in _refreshDrawBuffer()
		UT_GrowBuf * pgbCharWidthsDisplay = getBlock()->getCharWidths()->getCharWidths();
		UT_GrowBuf *pgbCharWidthsLayout  = getBlock()->getCharWidths()->getCharWidthsLayoutUnits();

		UT_GrowBufElement* pCharWidthsDisplay = pgbCharWidthsDisplay->getPointer(0);
		UT_GrowBufElement* pCharWidthsLayout = pgbCharWidthsLayout->getPointer(0);
		xxx_UT_DEBUGMSG(("fp_TextRun::recalcWidth: pCharWidthsDisplay 0x%x, pCharWidthsLayout 0x%x\n",pCharWidthsDisplay, pCharWidthsLayout));

		_setWidth(0);
		_setWidthLayoutUnits(0);

		FriBidiCharType iVisDirection = getVisDirection();

		bool bReverse = (!s_bBidiOS && iVisDirection == FRIBIDI_TYPE_RTL)
			|| (s_bBidiOS && m_iDirOverride == FRIBIDI_TYPE_LTR && _getDirection() == FRIBIDI_TYPE_RTL);

		UT_sint32 j,k;

		// the setFont() call is a major bottleneck; we will be better
		// of runing two separate loops for screen and layout fonts
		UT_uint32 i;
		getGR()->setFont(_getLayoutFont());
		
		for (i = 0; i < getLength(); i++)
		{
			// this is a bit tricky, since we want the resulting width array in
			// logical order, so if we reverse the draw buffer ourselves, we
			// have to address the draw buffer in reverse
			j = bReverse ? getLength() - i - 1 : i;
			//k = (!bReverse && iVisDirection == FRIBIDI_TYPE_RTL) ? getLength() - i - 1: i;
			k = i + getBlockOffset();

			if(s_bUseContextGlyphs)
			{
				getGR()->measureString(m_pSpanBuff + j, 0, 1, (UT_GrowBufElement*)pCharWidthsLayout + k);
			}
			UT_uint32 iCW = pCharWidthsLayout[k] > 0 ? pCharWidthsLayout[k] : 0;
			_setWidthLayoutUnits(getWidthInLayoutUnits() + iCW);
		}

		getGR()->setFont(_getScreenFont());
		for (i = 0; i < getLength(); i++)
		{
			// this is a bit tricky, since we want the resulting width array in
			// logical order, so if we reverse the draw buffer ourselves, we
			// have to address the draw buffer in reverse
			j = bReverse ? getLength() - i - 1 : i;
			//k = (!bReverse && iVisDirection == FRIBIDI_TYPE_RTL) ? getLength() - i - 1: i;
			k = i + getBlockOffset();

			if(s_bUseContextGlyphs)
			{
				getGR()->measureString(m_pSpanBuff + j, 0, 1, (UT_GrowBufElement*)pCharWidthsDisplay + k);
			}
			UT_uint32 iCW = pCharWidthsDisplay[k] > 0 ? pCharWidthsDisplay[k] : 0;
			_setWidth(getWidth() + iCW);
		}
		
#endif // #ifndef WITH_PANGO

		return true;
	}
	else
	{
		xxx_UT_DEBUGMSG(("fp_TextRun::recalcWidth (0x%x): the run has not changed\n",this));
		return false;
	}
}

// this function is just like recalcWidth, except it does not change the character width
// information kept by the block, but assumes that information is correct.
// the only place it is currently used is the split() function. Since spliting a run into
// two does not change the actual character sequence in the block, the width array can stay
// untouched
bool fp_TextRun::_addupCharWidths(void)
{
	UT_sint32 iWidth = 0;
	UT_GrowBuf *pgbCharWidthsDisplay = getBlock()->getCharWidths()->getCharWidths();
	UT_GrowBufElement* pCharWidthsDisplay = pgbCharWidthsDisplay->getPointer(0);
#ifndef WITH_PANGO
	UT_sint32 iWidthLayoutUnits = 0;
	UT_GrowBuf *pgbCharWidthsLayout  = getBlock()->getCharWidths()->getCharWidthsLayoutUnits();
	UT_GrowBufElement* pCharWidthsLayout  = pgbCharWidthsLayout->getPointer(0);
#endif
	xxx_UT_DEBUGMSG(("fp_TextRun::_addupCharWidths: pCharWidthsDisplay 0x%x, pCharWidthsLayout 0x%x\n",pCharWidthsDisplay, pCharWidthsLayout));


	for (UT_uint32 i = getBlockOffset(); i < getLength() + getBlockOffset(); i++)
	{
		UT_uint32 iCW = pCharWidthsDisplay[i] > 0 ? pCharWidthsDisplay[i] : 0;
		iWidth += iCW;
		
#ifndef WITH_PANGO
		iCW = pCharWidthsLayout[i] > 0 ? pCharWidthsLayout[i] : 0;
		iWidthLayoutUnits += iCW;
#endif
	}

	if(iWidth != getWidth())
	{
		_setWidth(iWidth);
#ifndef WITH_PANGO
		_setWidthLayoutUnits(iWidthLayoutUnits);
#endif
		return true;
	}

	return false;
}

void fp_TextRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!isDirty());
	UT_ASSERT(getGR()->queryProperties(GR_Graphics::DGP_SCREEN));

	if(!getLine()->isEmpty() && getLine()->getLastVisRun() == this)   //#TF must be last visual run
	{
		// Last run on the line so clear to end.

		getLine()->clearScreenFromRunToEnd(this);
	}
	else
	{
#ifndef WITH_PANGO
		getGR()->setFont(_getScreenFont());
#else
		getGR()->setFont(_getPangoFont());
#endif
		/*
		  TODO this should not be hard-coded.  We need to figure out
		  what the appropriate background color for this run is, and
		  use that.  Note that it could vary on a run-by-run basis,
		  since document facilities allow the background color to be
		  changed, for things such as table cells.
		*/
		// we need to use here page color, not highlight color, otherwise
		// we endup with higlighted margin
		//UT_RGBColor clrNormalBackground(m_colorHL.m_red, m_colorHL.m_grn, m_colorHL.m_blu);
		UT_RGBColor clrNormalBackground(_getColorPG());
		if (getField())
		{
		  clrNormalBackground -= _getView()->getColorFieldOffset();
		}
		getGR()->setColor(clrNormalBackground);

		UT_sint32 xoff = 0, yoff = 0;
		getLine()->getScreenOffsets(this, xoff, yoff);
		//
		// Handle case where character extend behind the left side
		// like italic Times New Roman f
		//
		fp_Line * thisLine = getLine();
		fp_Run * pPrev = getPrev();
		UT_sint32 leftClear = 0;
		if(thisLine != NULL)
		{
			while(pPrev != NULL && pPrev->getLine() == thisLine && pPrev->getLength() == 0)
				pPrev = pPrev->getPrev();

			leftClear = getDescent();
			if (pPrev != NULL && pPrev->getLine() == thisLine &&
				(pPrev->getType() == FPRUN_TEXT || pPrev->getType() == FPRUN_FIELD || pPrev->getType() == FPRUN_IMAGE))
 				leftClear = 0;
		}
		getGR()->fillRect(clrNormalBackground, xoff - leftClear, yoff, getWidth() + leftClear, getLine()->getHeight());
	}

}

void fp_TextRun::_draw(dg_DrawArgs* pDA)
{
	/*
	  Upon entry to this function, pDA->yoff is the BASELINE of this run, NOT
	  the top.
	*/
#ifndef NDEBUG
	FV_View* ppView = getBlock()->getDocLayout()->getView();
	if(ppView) UT_ASSERT(ppView && ppView->isCursorOn()==false);
#endif

	_refreshDrawBuffer();
	xxx_UT_DEBUGMSG(("fp_TextRun::_draw (0x%x): m_iVisDirection %d, _getDirection() %d\n", this, m_iVisDirection, _getDirection()));
	UT_sint32 yTopOfRun = pDA->yoff - getAscent() - 1; // Hack to remove
	UT_sint32 yTopOfSel = yTopOfRun + 1; // final character dirt

	/*
	  TODO We should add more possibilities for text placement here.
	  It shouldn't be too hard.  Just adjust the math a little.
	  See bug 1297
	*/

	if (m_fPosition == TEXT_POSITION_SUPERSCRIPT)
	{
		yTopOfRun -= getAscent() * 1/2;
	}
	else if (m_fPosition == TEXT_POSITION_SUBSCRIPT)
	{
		yTopOfRun += getDescent() /* * 3/2 */;
	}

	UT_ASSERT(pDA->pG == getGR());

	
	////////////////////////////////////////////////////////////////////
	// 
	// This section deals with calculating character advances so that
	// overstriking characters get handled correctly
	//
	
	const UT_GrowBuf * pgbCharWidths = getBlock()->getCharWidths()->getCharWidths();
	const UT_GrowBuf * pgbCharWidthsLayout = getBlock()->getCharWidths()->getCharWidthsLayoutUnits();
	UT_sint32 * pCWThis = getGR()->queryProperties(GR_Graphics::DGP_SCREEN) ?
		pgbCharWidths->getPointer(getBlockOffset()) :
		pgbCharWidthsLayout->getPointer(getBlockOffset());


	// should this prove to be too much of a performance bottleneck,
	// we will cache this in a member array

	UT_uint32 iLen = getLength();
	if(iLen > s_iCharAdvanceSize)
	{
		delete [] s_pCharAdvance;
		s_pCharAdvance = new UT_sint32 [iLen];
		UT_return_if_fail(s_pCharAdvance);
		s_iCharAdvanceSize = iLen;
	}

	UT_sint32 xoff_draw = pDA->xoff;
	
	if(!s_bBidiOS && getVisDirection()== FRIBIDI_TYPE_RTL )
	{
		for(UT_uint32 n = 0; n < iLen - 1; n++)
		{
			if(pCWThis[iLen - n - 1] < 0)
			{
				UT_sint32 iWidth = 0;
				UT_sint32 iCumAdvance = 0;

				UT_sint32 m = iLen - n - 2;
				while(m >= 0 && pCWThis[m] < 0)
					m--;

				if(m < 0)
				{
					// problem: this run does not contain the
					// character over which we are meant to be
					// overimposing our overstriking chars
					// we will have to set the offsets to 0
					for(UT_uint32 k = n; k < iLen - 1; k++)
						s_pCharAdvance[k] = 0;

					n = iLen - 1;
				}
				else
				{
					UT_uint32 k;
					for(k = n; k < iLen - m -1; k++)
					{
						UT_sint32 iAdv = (pCWThis[m] + pCWThis[iLen - k - 1])/2 - iCumAdvance;
						if(k == 0)
						{
							// k == 0, this is the leftmost character,
							// so we have no advance to set, but we
							// can adjust the starting point of the drawing
							xoff_draw += iAdv;
						}
						else if(k == n)
						{
							// this is a special case; we have already
							// calculated the advance in previous
							// round of the main loop, and this is
							// only adjustment
							s_pCharAdvance[k-1] += iAdv;
						}
						else
							s_pCharAdvance[k-1] = iAdv;

						iCumAdvance += iAdv;
					}

					s_pCharAdvance[k-1] = -iCumAdvance;
					s_pCharAdvance[k]   = pCWThis[m];
					n = k; // should be k+1, but there will be n++ in
					       // the for loop
				}

			}
			else
			{
				s_pCharAdvance[n] = pCWThis[iLen - n - 1];
			}
		}
	}
	else
	{
		for(UT_uint32 n = 0; n < iLen - 1; n++)
		{
			if(pCWThis[n+1] < 0)
			{
				// remember the width of the non-zero character
				UT_sint32 iWidth = pCWThis[n];
				UT_sint32 iCumAdvance = 0;
				
				// find the next non-zerow char
				UT_uint32 m  = n + 1;
				while(m < iLen && pCWThis[m] < 0)
				{
					// plus because pCharWidths[m] < 0
					// -1 because it is between m-1 and m
					s_pCharAdvance[m-1] = iWidth - (iWidth + pCWThis[m])/2 + iCumAdvance;
					iCumAdvance += s_pCharAdvance[m-1];
					m++;
				}

				n = m-1; // this is the last 0-width char
				s_pCharAdvance[n] = iWidth - iCumAdvance;
			}
			else
				s_pCharAdvance[n] = pCWThis[n];
		}

	}
	
	UT_uint32 iBase = getBlock()->getPosition();

	UT_RGBColor clrNormalBackground(_getColorHL());
	UT_RGBColor clrSelBackground = _getView()->getColorSelBackground();

	if (getField())
	{
		UT_RGBColor color_offset = _getView()->getColorFieldOffset();
		clrNormalBackground -= color_offset;
		clrSelBackground -= color_offset;
	}

	/*
		the old way of doing things was very inefficient; for each chunk of this
		run that had a different background we first drew the background rectangle,
		then drew the text over it, and then moved onto the next chunk. This is very
		involved since to draw the text in pieces we have to calculate the screen
		offset of each chunk using character widths.

		This is is what we will do instead:
		(1) draw the background using the background colour for the whole run in
			a single go
		(2) draw any selection background where needed over the basic background
		(3) draw the whole text in a single go over the composite background
	*/

	// draw the background for the whole run, but only in sreen context or if
	// it is not white
	UT_RGBColor clrWhite(255,255,255); // FIXME: should not be hardwired?!
	bool bDrawBckg = (getGR()->queryProperties(GR_Graphics::DGP_SCREEN)
					 || clrNormalBackground != clrWhite);

	if(bDrawBckg)
	{
		getGR()->fillRect( clrNormalBackground,
					pDA->xoff,
					yTopOfSel + getAscent() - getLine()->getAscent(),
					getWidth(),
					getLine()->getHeight());
	}

	UT_uint32 iRunBase = iBase + getBlockOffset();

	FV_View* pView = getBlock()->getDocLayout()->getView();
	UT_uint32 iSelAnchor = pView->getSelectionAnchor();
	UT_uint32 iPoint = pView->getPoint();

	UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
	UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);

	UT_ASSERT(iSel1 <= iSel2);

	if (pView->getFocus()!=AV_FOCUS_NONE && iSel1 != iSel2)
	{
		if (iSel1 <= iRunBase)
		{
			if (iSel2 > iRunBase)
			{
				if (iSel2 >= (iRunBase + getLength()))
				{
					// the whole run is selected
					_fillRect(clrSelBackground, pDA->xoff, yTopOfSel, getBlockOffset(), getLength(), pgbCharWidths);
				}
				else
				{
					// the first part is selected, the second part is not
					_fillRect(clrSelBackground, pDA->xoff, yTopOfSel, getBlockOffset(), iSel2 - iRunBase, pgbCharWidths);
				}
			}
		}
		else if (iSel1 < (iRunBase + getLength()))
		{
			if (iSel2 >= (iRunBase + getLength()))
			{
				// the second part is selected
				_fillRect(clrSelBackground, pDA->xoff, yTopOfSel, iSel1 - iBase, getLength() - (iSel1 - iRunBase), pgbCharWidths);
			}
			else
			{
				// a midle section is selected
				_fillRect(clrSelBackground, pDA->xoff, yTopOfSel, iSel1 - iBase, iSel2 - iSel1, pgbCharWidths);
			}
		}
	}

	/*
		if the text on either side of this run is in italics, there is a good
		chance that the background covered some of the text; to fix this we
		call _drawLastChar and _drawFirstChar for those runs, but we will not do
		so undiscriminately -- we need to do this only if the prev or next
		runs are overhanging (i.e., italicized) and if we are in a screen-like
		context (screen-like context is one in which drawing a white rectangle
		over a piece of text results in the text being erased; apart from
		the sreen, a Windows printer can behave like this).
	*/

	// first of all remember the current colour (this is necessary
	// because hyperlink and revision colors are set from the base
	// class and we must respect them
#if 0
	UT_RGBColor curColor;
	getGR()->getColor(curColor);
#endif

	if(bDrawBckg && getGR()->queryProperties(GR_Graphics::DGP_OPAQUEOVERLAY))
	{
		fp_Run * pNext = getNextVisual();
		fp_Run * pPrev = getPrevVisual();

		if(pNext && pNext->getType() == FPRUN_TEXT)
		{
			fp_TextRun * pT = static_cast<fp_TextRun*>(pNext);
			UT_sint32 ytemp = pDA->yoff+(pT->getY()-getY())-pT->getAscent()-1;
 			if (pT->m_fPosition == TEXT_POSITION_SUPERSCRIPT)
 			{
 				ytemp -= pT->getAscent() * 1/2;
 			}
 			else if (pT->m_fPosition == TEXT_POSITION_SUBSCRIPT)
 			{
 				ytemp += pT->getDescent() /* * 3/2 */;
 			}

			if(pT->m_bIsOverhanging)
				pT->_drawFirstChar(pDA->xoff + getWidth(),ytemp);
		}

		if(pPrev && pPrev->getType() == FPRUN_TEXT)
		{
			fp_TextRun * pT = static_cast<fp_TextRun*>(pPrev);
			UT_sint32 ytemp = pDA->yoff+(pT->getY()-getY())-pT->getAscent()-1;
 			if (pT->m_fPosition == TEXT_POSITION_SUPERSCRIPT)
 			{
 				ytemp -= pT->getAscent() * 1/2;
 			}
 			else if (pT->m_fPosition == TEXT_POSITION_SUBSCRIPT)
 			{
 				ytemp += pT->getDescent() /* * 3/2 */;
 			}

 			if(pT->m_bIsOverhanging)
			        pT->_drawLastChar(pDA->xoff,ytemp, pgbCharWidths);

		}
	}

	// now draw the whole string
#ifndef WITH_PANGO
	getGR()->setFont(_getScreenFont());
#else
	getGR()->setFont(_getPangoFont());
#endif

	getGR()->setColor(getFGColor()); // set colour just in case we drew a first/last char with a diff colour

	// this code handles spaces in justified runs
	UT_uint32 iSpaceCount = 0;

	if(m_pJustifiedSpaces)
		iSpaceCount = m_pJustifiedSpaces->getItemCount();


	if(iSpaceCount < 2)
	{
		// this is the case of non-justified run or fakly-justified run
		// since we have the visual string in the draw buffer, we just call getGR()r->drawChars()
		getGR()->drawChars(m_pSpanBuff, 0, getLength(), xoff_draw, yTopOfRun,s_pCharAdvance);
	}
	else
	{
		// because the data stored in our vector does not get
		// modified when text is inserted or deleted we have to
		// calculate the adjustment for the stored offsets

		// note that the original offset can be negative to allow us
		// to create positive iDelta (see the note in ::mergeWithNext())
		UT_sint32 iOrigOffset  = (UT_uint32) m_pJustifiedSpaces->getNthItem(1);
		UT_uint32 iCurrOffset  = getBlockOffset();
		UT_sint32 iDelta       = iCurrOffset - iOrigOffset;

		UT_uint32 iSpaceOffset;
		UT_uint32 iSpaceLength;
		UT_uint32 iSpaceWidth;
		UT_uint32 iTextWidth;

		UT_uint32 iOffset = 0;
		UT_uint32 iLength = getLength();
		UT_sint32 iX = xoff_draw;
		UT_uint32 i = 2;

		do
		{
			iSpaceOffset = (UT_uint32) m_pJustifiedSpaces->getNthItem(i) + iDelta;
			iSpaceLength = (UT_uint32) m_pJustifiedSpaces->getNthItem(i+1);
			iSpaceWidth  = (UT_uint32) m_pJustifiedSpaces->getNthItem(i+2);
			iTextWidth   = (UT_uint32) m_pJustifiedSpaces->getNthItem(i+3);

			getGR()->drawChars(m_pSpanBuff, iOffset, iSpaceOffset - iOffset, iX, yTopOfRun, s_pCharAdvance + iOffset);
			xxx_UT_DEBUGMSG(( "fp_TextRun::_draw: iOffset %d, iSpaceOffset %d, iSpaceLength %d, iDelta %d, iCurrOffset %d, iSpaceWidth %d, iTextWidth %d\n", iOffset, iSpaceOffset, iSpaceLength, iDelta, iCurrOffset, iSpaceWidth, iTextWidth ));

			iOffset = iSpaceOffset + iSpaceLength;
			iX += iTextWidth + iSpaceWidth;

			i += 4;

		}while (i < iSpaceCount);

		if(iOffset < iLength)
		{
			// draw the section of the run past the last space segment
			getGR()->drawChars(m_pSpanBuff, iOffset, iLength - iOffset, iX, yTopOfRun, s_pCharAdvance + iOffset);
		}
	}


	drawDecors(pDA->xoff, yTopOfRun);

	if(pView->getShowPara())
	{
		_drawInvisibles(pDA->xoff, yTopOfRun);
	}

	// TODO: draw this underneath (ie, before) the text and decorations
	m_bSquiggled = false;
	getBlock()->findSquigglesForRun(this);
}

void fp_TextRun::_fillRect(UT_RGBColor& clr,
						   UT_sint32 xoff,
						   UT_sint32 yoff,
						   UT_uint32 iPos1,
						   UT_uint32 iLen,
						   const UT_GrowBuf* pgbCharWidths)
{
	/*
	  Upon entry to this function, yoff is the TOP of the run,
	  NOT the baseline.
	*/
#ifndef NDEBUG
	FV_View* ppView = getBlock()->getDocLayout()->getView();
	if(ppView) UT_ASSERT(ppView && ppView->isCursorOn()==false);
#endif
	// we also need to support this in printing
	// NO! we do not and must not -- this is only used to draw selections
	// and field background and we do not want to see these printed
	if (getGR()->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		UT_Rect r;

		_getPartRect(&r, xoff, yoff, iPos1, iLen, pgbCharWidths);
		r.height = getLine()->getHeight();
		r.top = r.top + getAscent() - getLine()->getAscent();
		getGR()->fillRect(clr, r.left, r.top, r.width, r.height);
	}
}

void fp_TextRun::_getPartRect(UT_Rect* pRect,
							  UT_sint32 xoff,
							  UT_sint32 yoff,
							  UT_uint32 iStart,
							  UT_uint32 iLen,
							  const UT_GrowBuf * pgbCharWidths)
{
	/*
	  Upon entry to this function, yoff is the TOP of the run,
	  NOT the baseline.
	*/
	pRect->top = yoff;
	pRect->height = getHeight();
	pRect->width = 0;

	// that's enough for zero-length run
	if (getLength() == 0)
	{
		pRect->left = xoff;
		return;
	}

	//Should this be an error condition? I treat it as a zero length run, but other might not
	const UT_GrowBufElement * pCharWidths = pgbCharWidths->getPointer(0);

	pRect->left = 0;//#TF need 0 because of BiDi, need to calculate the width of the non-selected
			//section first rather than the abs pos of the left corner

	UT_ASSERT(pCharWidths);
	if (!pCharWidths) {
		UT_DEBUGMSG(("TODO: Investigate why pCharWidths is NULL?"));
		return;
	}

	UT_uint32 i;
	if (iStart > getBlockOffset())
	{
		for (i=getBlockOffset(); i<iStart; i++)
		{
			UT_uint32 iCW = pCharWidths[i] > 0 ? pCharWidths[i] : 0;
			pRect->left += iCW;
		}
	}

	if(getVisDirection() == FRIBIDI_TYPE_LTR)
	{
		pRect->left += xoff; //if this is ltr then adding xoff is all that is needed
	}

	for (i=iStart; i<(iStart + iLen); i++)
	{
		UT_uint32 iCW = pCharWidths[i] > 0 ? pCharWidths[i] : 0;
		pRect->width += iCW;
	}

	//in case of rtl we are now in the position to calculate the position of the the left corner
	if(getVisDirection() == FRIBIDI_TYPE_RTL) pRect->left = xoff + getWidth() - pRect->left - pRect->width;
}

inline UT_UCSChar fp_TextRun::_getContextGlyph(const UT_UCSChar * pSpan,
										UT_uint32 len,
										UT_uint32 offset,
										UT_UCSChar *prev,
										UT_UCSChar *after) const
{
	UT_sint32 iStop2 = (UT_sint32) CONTEXT_BUFF_SIZE < (UT_sint32)(len - 1) ? CONTEXT_BUFF_SIZE : len - 1;
	UT_contextGlyph cg;
	UT_UCSChar next[CONTEXT_BUFF_SIZE + 1];

	xxx_UT_DEBUGMSG(("fp_TextRun::_getContextGlyph: len %d, offset %d\n",
					len,offset));


	// now for each character in this fragment we create
	// an array of the chars that follow it, get the char
	// that precedes it and then translate it using our
	// UT_contextGlyph class, and we are done
	UT_sint32 i;
	for(i=0; i < iStop2; i++)
		next[i] = pSpan[i+offset+1];
	for(UT_sint32 j = 0; after[j]!=0 && (UT_uint32)i < CONTEXT_BUFF_SIZE; i++,j++)
		next[i] = after[j];
	next[i] = 0;

	switch(offset)
	{
		case 0: break;
		case 1: prev[1] = prev[0]; prev[0] = pSpan[0];
				break;
		default: prev[1] = pSpan[offset-2]; prev[0] = pSpan[offset-1];
	}

	return cg.getGlyph(&pSpan[offset],&prev[0],&next[0]);
}

inline void fp_TextRun::_getContext(const UT_UCSChar *pSpan,
									UT_uint32 lenSpan,
									UT_uint32 len,
									UT_uint32 offset,
									UT_UCSChar * prev,
									UT_UCSChar * after) const
{
	// first the char preceding this part of the run, which is simple
	// NB. cannot call getSpanPtr with negative offset, or we will get
	// into the undo buffer
	const UT_UCSChar * pPrev = 0, * pNext = 0;
	UT_uint32 lenPrev, lenAfter;
	prev[0] = 0;
	prev[1] = 0;
	prev[2] = 0;

	if(getBlockOffset() > 1)
	{
		if(getBlock()->getSpanPtr(offset - 2, &pPrev, &lenPrev))
		{
			prev[1] = *pPrev;
			if(lenPrev > 1)
				prev[0] = *(pPrev+1);
			else if(getBlock()->getSpanPtr(offset - 1, &pPrev, &lenPrev))
				prev[0] = *(pPrev+1);
		}
		else if(getBlock()->getSpanPtr(offset - 1, &pPrev, &lenPrev))
		{
			prev[0] = *pPrev;
		}
	}
	else if(getBlockOffset() > 0)
	{
		if(getBlock()->getSpanPtr(offset - 1, &pPrev, &lenPrev))
			prev[0] = *pPrev;
	}

	lenPrev = 2;

	xxx_UT_DEBUGMSG(("fp_TextRun::_getContext: prev[0] %d, prev[1] %d\n"
				 "		 getBlockOffset() %d, offset %d\n",
				 prev[0],prev[1],getBlockOffset(),offset));

	// how many characters at most can we retrieve?
	UT_sint32 iStop = (UT_sint32) CONTEXT_BUFF_SIZE < (UT_sint32)(lenSpan - len) ? CONTEXT_BUFF_SIZE : lenSpan - len;
	UT_sint32 i;
	// first, getting anything that might be in the span buffer
	for(i=0; i< iStop;i++)
		after[i] = pSpan[len+i];

	// for anything that we miss, we need to get it the hard way
	// as it is located in different spans

	while(i < CONTEXT_BUFF_SIZE && getBlock()->getSpanPtr(offset + len + i, &pNext, &lenAfter))
	{
		for(UT_uint32 j = 0; j < lenAfter && (UT_uint32)i < CONTEXT_BUFF_SIZE; j++,i++)
			after[i] = pNext[j];
	}

	// now we have our trailing chars, so we null-terminate the array
	after[i] = 0;
}


void fp_TextRun::_refreshDrawBuffer()
{
	if(_getRefreshDrawBuffer())
	{
		_setRefreshDrawBuffer(false);
		FriBidiCharType iVisDir = getVisDirection();

		const UT_UCSChar* pSpan = NULL;
		UT_uint32 lenSpan = 0;
		UT_uint32 offset = 0;
		UT_uint32 len = getLength();
		bool bContinue = true;

		UT_contextGlyph cg;

		if(getLength() > m_iSpanBuffSize) //the buffer too small, reallocate
		{
			delete[] m_pSpanBuff;
			m_pSpanBuff = new UT_UCSChar[getLength() + 1];
			m_iSpanBuffSize = getLength();
			UT_ASSERT(m_pSpanBuff);
		}

		for(;;) // retrive the span and do any necessary processing
		{
			bContinue = getBlock()->getSpanPtr(offset + getBlockOffset(), &pSpan, &lenSpan);

			//this sometimes happens with fields ...
			if(!bContinue)
				break;

			UT_uint32 iTrueLen = (lenSpan > len) ? len : lenSpan;

			if(_getDirection() != FRIBIDI_TYPE_ON || iVisDir != FRIBIDI_TYPE_RTL)
			{
				/*
					here we need to handle context glyph shapes. For that we will
					use our class UT_contextGlyph, for which we need to retrieve
					one character before the one in question and an unspecified
					number of chars to follow.
				*/
				if(s_bUseContextGlyphs)
				{
					// now we will retrieve 5 characters that follow this part of the run
					// and two chars that precede it

					// three small buffers, one to keep the chars past this part
					// and one that we will use to create the "trailing" chars
					// for each character in this fragment
					UT_UCSChar next[CONTEXT_BUFF_SIZE + 1];
					UT_UCSChar prev[CONTEXT_BUFF_SIZE + 1];


					// NB: _getContext requires block offset
					_getContext(pSpan,lenSpan,len,offset+getBlockOffset(),&prev[0],&next[0]);
					cg.renderString(pSpan, &m_pSpanBuff[offset],iTrueLen,&prev[0],&next[0]);
				}
				else //do not use context glyphs
				{
					UT_UCS4_strncpy(m_pSpanBuff + offset, pSpan, iTrueLen);
				}
			}
			else
			{
				//this is 'other neutral' visually rtl span, we need to deal
				//with mirror characters
				for (UT_uint32 i = 0; i < iTrueLen; i++)
					m_pSpanBuff[offset + i] = getMirrorChar(pSpan[i]);
			}

			if(iTrueLen == len)
			{
				break;
			}

			offset += iTrueLen;
			len -= iTrueLen;

		} // for(;;)

		// if we are on a non-bidi OS, we have to reverse any RTL runs
		// if we are on bidi OS, we have to reverse RTL runs that have direction
		// override set to LTR, to preempty to OS reversal of such text
		if((!s_bBidiOS && iVisDir == FRIBIDI_TYPE_RTL)
		  || (s_bBidiOS && m_iDirOverride == FRIBIDI_TYPE_LTR && _getDirection() == FRIBIDI_TYPE_RTL))
			UT_UCS4_strnrev(m_pSpanBuff, getLength());
	} //if(m_bRefreshDrawBuffer)
}


#ifdef WITH_PANGO
void fp_TextRun::shape()
{
	if(m_bRefreshDrawBuffer)
	{
		m_bRefreshDrawBuffer = false;
		_freeGlyphString();

		FriBidiCharType iVisDir = getVisDirection();

		const UT_UCSChar* pSpan = NULL;
		UT_uint32 lenSpan = 0;
		UT_uint32 offset = 0;
		UT_uint32 len = getLength();
		bool bContinue = true;

		UT_UCSChar * pWholeSpan = new UT_UCSChar [getLength()];
		UT_ASSERT(pWholeSpan);

		UT_UCSChar * pWholeSpanPtr = pWholeSpan;

		for(;;) // retrive the span and do any necessary processing
		{
			bContinue = getBlock()->getSpanPtr(offset + getBlockOffset(), &pSpan, &lenSpan);

			//this sometimes happens with fields ...
			if(!bContinue)
				break;

			UT_uint32 iTrueLen = (lenSpan > len) ? len : lenSpan;
		 	UT_UCS4_strncpy(pWholeSpanPtr, iTrueLen);

			if(iTrueLen == len)
			{
				break;
			}

			offset += iTrueLen;
			len -= iTrueLen;

		} // for(;;)

		// now convert the whole span into utf8
		UT_UTF8String wholeStringUtf8 (pWholeString, getLength());

		// let Pango to analyse the string
		GList * pItems = pango_itemize(getGR()->getPangoContext(),
									   wholeStringUtf8.utf8_str(),
									   0,
									   wholeStringUtf8.byteLength(),
									   NULL, // PangoAttrList, not sure about this
									   NULL
									   );

		// now do the shaping
		GList * pListItem = g_list_first(pItems);
		UT_ASSERT(m_pGlyphString == NULL);

		while(pListItem)
		{
			PangoItem * pItem = (PangoItem*) pListItem->data;
			PangoGlyphString * pGString = pango_glyph_string_new();

			pango_shape(wholeStringUtf8.utf8_str(),
						pItem->offset,
						pItem->length,
						&pItem->analysis,
						pGString);

			m_pGlyphString = g_list_append((gpointer) pGString);
			pListItem = pListItem->next;
		}

		// next we need to refresh our character widths
		m_bRecalcWidth = false;

		UT_GrowBuf * pgbCharWidthsDisplay = getBlock()->getCharWidths()->getCharWidths();
		//UT_GrowBuf * pgbCharWidthsLayout  = getBlock()->getCharWidths()->getCharWidthsLayoutUnits();

		UT_GrowBufElement * pCharWidthsDisplay = pgbCharWidthsDisplay->getPointer(0) + getBlockOffset();
		//UT_GrowBufElement * pCharWidthsLayout = pgbCharWidthsLayout->getPointer(0) + getBlockOffset();

		_setWidth(0);
#ifndef WITH_PANGO
		m_iWidthLayoutUnits = 0;
#endif

		// the display width are easy ...
		GList * pListGlyph = g_list_first(m_pGlyphString);
		pListItem = g_list_first(pItems);

		UT_GrowBufElement * pCharWidthDisplayPtr = pCharWidthsDisplay;
		const gchar * text = wholeStringUtf8.utf8_str();
		PangoRectangle ink_rect;
#if 0
		m_iWidthLayoutUnits = 0;
#endif
		_setWidth(0);

		while(pListGlyph)
		{
			UT_ASSERT(pListItem);

			PangoGlyphString * pGString = (PangoGlyphString *)pListGlyph->data;
			PangoItem *        pItem    = (PangoItem *)pListItem->data;

			pango_glyph_string_get_logical_widths(pGString,
											  text,
											  pItem->length,
											  iVisDir == FRIBIDI_TYPE_RTL ? 1 : 0,
											  pCharWidthsDisplayPtr);

#if 0
			// not sure whether we need to do this, or can just add up below ...
			pango_glyph_string_extents(pGString, _getPangoFont(), &ink_rect,NULL);
			_setWidth(getWidth() + ink_rect.width);
#endif
			for(UT_sint32 j = 0; j < pItem->num_chars; j++)
				_setWidth(getWidth() + pCharWidthsDisplayPtr[j]);

			text += pItem->length;
			pCharWidthsDisplayPtr += pItem->num_chars;

			pListGlyph = pListGlyph->next;
			pListItem = pListItem->next;
		}

		// now clear up the item's list
		g_list_foreach(pItems, UT_free1PangoItem, NULL);
		g_list_free(pItems);

	} //if(m_bRefreshDrawBuffer)
}
#endif


/*
	xoff is the right edge of this run !!!
*/
void fp_TextRun::_drawLastChar(UT_sint32 xoff, UT_sint32 yoff,const UT_GrowBuf * pgbCharWidths)
{
	if(!getLength())
		return;

	// have to set font (and colour!), since we were called from a run using different font
	getGR()->setFont(_getScreenFont());
	getGR()->setColor(getFGColor());

	FriBidiCharType iVisDirection = getVisDirection();

	if(!s_bBidiOS)
	{
		// m_pSpanBuff is in visual order, so we just draw the last char
		UT_uint32 iPos = iVisDirection == FRIBIDI_TYPE_LTR ? getLength() - 1 : 0;
		getGR()->drawChars(m_pSpanBuff, getLength() - 1, 1, xoff - *(pgbCharWidths->getPointer(getBlockOffset() + iPos)), yoff);
	}
	else
	{
		UT_uint32 iPos = iVisDirection == FRIBIDI_TYPE_LTR ? getLength() - 1 : 0;
		getGR()->drawChars(m_pSpanBuff, iPos, 1, xoff - *(pgbCharWidths->getPointer(getBlockOffset() + iPos)), yoff);
	}
}

void fp_TextRun::_drawFirstChar(UT_sint32 xoff, UT_sint32 yoff)
{
	if(!getLength())
		return;

	// have to sent font (and colour!), since we were called from a run using different font
	getGR()->setFont(_getScreenFont());
	getGR()->setColor(getFGColor());

	if(!s_bBidiOS)
	{
		// m_pSpanBuff is in visual order, so we just draw the last char
		getGR()->drawChars(m_pSpanBuff, 0, 1, xoff, yoff);
	}
	else
	{
		UT_uint32 iPos = getVisDirection() == FRIBIDI_TYPE_RTL ? getLength() - 1 : 0;
		getGR()->drawChars(m_pSpanBuff, iPos, 1, xoff, yoff);
	}
}

void fp_TextRun::_drawPart(UT_sint32 xoff,
						   UT_sint32 yoff,
						   UT_uint32 iStart,
						   UT_uint32 iLen,
						   const UT_GrowBuf * pgbCharWidths)
{
	/*
	  Upon entry to this function, yoff is the TOP of the run,
	  NOT the baseline.
	*/

	// don't even try to draw a zero-length run
	if (getLength() == 0)
	{
		return;
	}

	UT_ASSERT(iStart >= getBlockOffset());
	UT_ASSERT(iStart + iLen <= getBlockOffset() + getLength());

	UT_uint32 iLeftWidth = 0;
	const UT_GrowBufElement * pCharWidths = pgbCharWidths->getPointer(0);
	UT_ASSERT(pCharWidths);
	if (!pCharWidths) {
		UT_DEBUGMSG(("TODO: Investigate why pCharWidths is NULL?"));
		return;
	}


	UT_uint32 i;
	for (i=getBlockOffset(); i<iStart; i++)
	{
		iLeftWidth += pCharWidths[i]; //NB! in rtl text this is visually right width
	}

	// in the bidi build we keep a cache of the visual span of this run
	// this speeds things up
	// (the cache is refreshed by _refreshDrawBuff, it is the responsibility
	// of caller of _drawPart to ensure that the chache is uptodate)
	FriBidiCharType iVisDirection = getVisDirection();
	bool bReverse = (!s_bBidiOS && iVisDirection == FRIBIDI_TYPE_RTL)
		  || (s_bBidiOS && m_iDirOverride == FRIBIDI_TYPE_LTR && _getDirection() == FRIBIDI_TYPE_RTL);
	if(iVisDirection == FRIBIDI_TYPE_RTL) //rtl: determine the width of the text we are to print
	{
		for (; i < iLen + iStart; i++)
		{
			iLeftWidth += pCharWidths[i];
		}
	}

	if(bReverse)
		// since the draw buffer is reversed, we need to calculate the position
		// of the last character in this fragement in it, for that is where we
		// start drawing from
		getGR()->drawChars(m_pSpanBuff + (getLength() + getBlockOffset() - iStart) - iLen, 0, iLen, xoff + getWidth() - iLeftWidth, yoff);

		// if the buffer is not reversed because this is simple LTR run, then
		// iLeftWidth is precisely that
	else if(iVisDirection == FRIBIDI_TYPE_LTR)
		getGR()->drawChars(m_pSpanBuff + iStart - getBlockOffset(), 0, iLen, xoff + iLeftWidth, yoff);
	else


		// if the buffer is not reversed because the OS will reverse it, then we
		// draw from it as if it was LTR buffer, but iLeftWidth is right width
		getGR()->drawChars(m_pSpanBuff + iStart - getBlockOffset(), 0, iLen, xoff + getWidth() - iLeftWidth, yoff);
}


void fp_TextRun::_drawInvisibleSpaces(UT_sint32 xoff, UT_sint32 yoff)
{
	UT_GrowBuf * pgbCharWidths = getBlock()->getCharWidths()->getCharWidths();
	UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);
	const UT_UCSChar* pSpan = NULL;
	UT_uint32 lenSpan = 0;
	UT_uint32 len = getLength();
	UT_sint32 iWidth = 0;
	UT_sint32 cur_linewidth = 1+ (UT_MAX(10,getAscent())-10)/8;
	UT_sint32 iRectSize = cur_linewidth * 3 / 2;
	bool bContinue = true;
	UT_uint32 offset = getBlockOffset();

	FV_View* pView = getBlock()->getDocLayout()->getView();
#ifndef NDEBUG
	if(pView) UT_ASSERT(pView && pView->isCursorOn()==false);
#endif
	//UT_DEBUGMSG(("---------\n"));
	if(findCharacter(0, UCS_SPACE) > 0){
		while(bContinue){
			bContinue = getBlock()->getSpanPtr(offset,&pSpan,&lenSpan);
			//if(!bContinue)
			//	break; //no span found
#ifdef DEBUG
			if(lenSpan <= 0)
			{
				const UT_UCSChar* mypSpan = NULL;
				unsigned char buff[500];
				UT_uint32 mylenSpan = 0;
				getBlock()->getSpanPtr(getBlockOffset(),&mypSpan,&mylenSpan);
				for(UT_uint32 i = 0; i < mylenSpan; i++)
					buff[i] = (unsigned char) mypSpan[i];

				UT_DEBUGMSG(("fp_TextRun::_drawInvisibleSpaces (0x%x):\n"
							 "		 getBlockOffset() %d, getLength() %d\n"
							 "		 mylenSpan %d, span: %s\n"
							 "		 lenSpan %d, len %d, offset %d\n",
							 this,getBlockOffset(),getLength(),mylenSpan,buff,lenSpan, len, offset));

			}
#endif
			//UT_ASSERT(lenSpan > 0);

			if(lenSpan > len){
				lenSpan = len;
			}

			UT_uint32 iy = yoff + getAscent() * 2 / 3;

			for (UT_uint32 i = 0;i < lenSpan;i++){
			   if(pSpan[i] == UCS_SPACE)
			   {
				   getGR()->fillRect(pView->getColorShowPara(), xoff + iWidth + (pCharWidths[i + offset] - iRectSize) / 2,iy,iRectSize,iRectSize);
			   }
			   UT_uint32 iCW = pCharWidths[i + offset] > 0 ? pCharWidths[i + offset] : 0;
			   iWidth += iCW;
			}
			if (len <= lenSpan){
				bContinue = false;
			}else{
				offset += lenSpan;
				len -= lenSpan;
			}

		}
	}
}

void fp_TextRun::_drawInvisibles(UT_sint32 xoff, UT_sint32 yoff)
{
#ifndef NDEBUG
	FV_View* ppView = getBlock()->getDocLayout()->getView();
	if(ppView) UT_ASSERT(ppView && ppView->isCursorOn()==false);
#endif

		if (!(getGR()->queryProperties(GR_Graphics::DGP_SCREEN))){
				return;
		}
		_drawInvisibleSpaces(xoff,yoff);
}

void fp_TextRun::_drawSquiggle(UT_sint32 top, UT_sint32 left, UT_sint32 right)
{
	if (!(getGR()->queryProperties(GR_Graphics::DGP_SCREEN)))
	{
		return;
	}

	m_bSquiggled = true;

	UT_sint32 nPoints = (right - left + 3)/2;
	UT_ASSERT(nPoints >= 1); //can be 1 for overstriking chars

	/*
		NB: This array gets recopied inside the polyLine implementation
			to move the coordinates into a platform-specific point
			structure.	They're all x, y but different widths.	Bummer.
	*/
	UT_Point * points, scratchpoints[100];
	if ((unsigned)nPoints < (sizeof(scratchpoints)/sizeof(scratchpoints[0])))
	{
		points = scratchpoints;
	}
	else
	{
		points = new UT_Point[nPoints];
	}
	UT_ASSERT(points);

	points[0].x = left;
	points[0].y = top;

	bool bTop = false;

	for (UT_sint32 i = 1; i < nPoints; i++, bTop = !bTop)
	{
		points[i].x = points[i-1].x + 2;
		points[i].y = (bTop ? top : top + 2);
	}

	if (points[nPoints-1].x > right)
	{
		points[nPoints-1].x = right;
		points[nPoints-1].y = top + 1;
	}

	getGR()->polyLine(points, nPoints);

	if (points != scratchpoints) delete points;
}

void fp_TextRun::drawSquiggle(UT_uint32 iOffset, UT_uint32 iLen)
{
//	UT_ASSERT(iLen > 0);
	if (iLen == 0)
	{
		// I think this is safe, although it begs the question, why did we get called if iLen is zero?	TODO
		return;
	}
	if(getLine())
	{
		getLine()->setScreenCleared(false);
	}
	UT_sint32 xoff = 0, yoff = 0;
	UT_sint32 iAscent = getLine()->getAscent();
	UT_sint32 iDescent = getLine()->getDescent();

	// we'd prefer squiggle to leave one pixel below the baseline,
	// but we need to force all three pixels inside the descent
	// we cannot afford the 1pixel gap, it leave dirt on screen -- Tomas
	UT_sint32 iGap = (iDescent > 3) ?/*1*/0 : (iDescent - 3);

	getGR()->setColor(_getView()->getColorSquiggle());

	getLine()->getScreenOffsets(this, xoff, yoff);

	UT_Rect r;
	const UT_GrowBuf * pgbCharWidths = getBlock()->getCharWidths()->getCharWidths();
	_getPartRect( &r, xoff, yoff, iOffset, iLen, pgbCharWidths);

	_drawSquiggle(r.top + iAscent + iGap, r.left, r.left + r.width);
}

UT_sint32 fp_TextRun::findCharacter(UT_uint32 startPosition, UT_UCSChar Character) const
{
	// NOTE: startPosition is run-relative
	// NOTE: return value is block-relative (don't ask me why)
	const UT_UCSChar* pSpan = NULL;
	UT_uint32 lenSpan = 0;
	UT_uint32 offset = getBlockOffset() + startPosition;
	UT_uint32 len = getLength() - startPosition;
	bool bContinue = true;

	if ((getLength() > 0) && (startPosition < getLength()))
	{
		UT_uint32 i;

		while (bContinue)
		{
			bContinue = getBlock()->getSpanPtr(offset, &pSpan, &lenSpan);
			//if(!bContinue)
			//	break;
			//UT_ASSERT(lenSpan>0);

			if (len <= lenSpan)
			{
				for(i = 0; i < len; i++)
				{
					if (pSpan[i] == Character)
						return offset + i;
				}

				bContinue = false;
			}
			else
			{
				for(i = 0; i < lenSpan; i++)
				{
					if (pSpan[i] == Character)
						return offset + i;
				}

				offset += lenSpan;
				len -= lenSpan;

				UT_ASSERT(offset >= getBlockOffset());
				UT_ASSERT(offset + len <= getBlockOffset() + getLength());
			}
		}
	}

	// not found

	return -1;
}

bool fp_TextRun::getCharacter(UT_uint32 run_offset, UT_UCSChar &Character) const
{
	const UT_UCSChar* pSpan = NULL;
	UT_uint32 lenSpan = 0;

	UT_ASSERT(run_offset < getLength());

	if (getLength() > 0)
	{
		if (getBlock()->getSpanPtr(run_offset + getBlockOffset(), &pSpan, &lenSpan))
		{
			UT_ASSERT(lenSpan>0);

			Character = pSpan[0];
			return true;
		}
	}

	// not found

	return false;
}

bool fp_TextRun::isLastCharacter(UT_UCSChar Character) const
{
	UT_UCSChar c;

	if (getCharacter(getLength() - 1, c))
		return c == Character;

	// not found

	return false;
}

bool fp_TextRun::isFirstCharacter(UT_UCSChar Character) const
{
	UT_UCSChar c;

	if (getCharacter(0, c))
		return c == Character;

	// not found

	return false;
}


bool	fp_TextRun::doesContainNonBlankData(void) const
{
	if(getLength() > 0)
	{
		const UT_UCSChar* pSpan = NULL;
		UT_uint32 lenSpan = 0;
		UT_uint32 offset = getBlockOffset();
		UT_uint32 len = getLength();
		bool bContinue = true;

		UT_uint32 i;

		while (bContinue)
		{
			bContinue = getBlock()->getSpanPtr(offset, &pSpan, &lenSpan);
			//if(!bContinue)
			//	break; //no span found
			if(lenSpan <= 0)
			{
				fp_Line * pLine = (fp_Line *) getBlock()->getFirstContainer();
				while(pLine)
				{
					pLine = (fp_Line *) pLine->getNext();
				}
			}
			//UT_ASSERT(lenSpan>0);

			if (len <= lenSpan)
			{
				for(i = 0; i < len; i++)
				{
					if (pSpan[i] != UCS_SPACE)
						return true;
				}

				bContinue = false;
			}
			else
			{
				for(i = 0; i < lenSpan; i++)
				{
					if (pSpan[i] != UCS_SPACE)
						return true;
				}

				offset += lenSpan;
				len -= lenSpan;

				UT_ASSERT(offset >= getBlockOffset());
				UT_ASSERT(offset + len <= getBlockOffset() + getLength());
			}
		}
	}

	// Only spaces found;

	return false;
}

inline bool fp_TextRun::isSuperscript(void) const
{
	return (m_fPosition == TEXT_POSITION_SUPERSCRIPT);
}

inline bool fp_TextRun::isSubscript(void) const
{
	return (m_fPosition == TEXT_POSITION_SUBSCRIPT);
}

UT_sint32 fp_TextRun::findTrailingSpaceDistance(void) const
{
	UT_GrowBuf * pgbCharWidths = getBlock()->getCharWidths()->getCharWidths();
	UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

	UT_sint32 iTrailingDistance = 0;

	if(getLength() > 0)
	{
		UT_UCSChar c;

		UT_sint32 i;
		for (i = getLength() - 1; i >= 0; i--)
		{
			if(getCharacter(i, c) && (UCS_SPACE == c))
			{
				iTrailingDistance += pCharWidths[i + getBlockOffset()];
			}
			else
			{
				break;
			}
		}

	}

	return iTrailingDistance;
}

#ifndef WITH_PANGO
UT_sint32 fp_TextRun::findTrailingSpaceDistanceInLayoutUnits(void) const
{
	UT_GrowBuf * pgbCharWidths = getBlock()->getCharWidths()->getCharWidthsLayoutUnits();
	UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

	UT_sint32 iTrailingDistance = 0;

	if(getLength() > 0)
	{
		UT_UCSChar c;

		UT_sint32 i;
		for (i = getLength() - 1; i >= 0; i--)
		{
			if(getCharacter(i, c) && (UCS_SPACE == c))
			{
				iTrailingDistance += pCharWidths[i + getBlockOffset()];
			}
			else
			{
				break;
			}
		}

	}

	return iTrailingDistance;
}
#endif

void fp_TextRun::resetJustification()
{
	UT_sint32 iSpaceWidthBefore = _getSpaceWidthBeforeJustification();
	
	if(iSpaceWidthBefore != JUSTIFICATION_NOT_USED
    && iSpaceWidthBefore != JUSTIFICATION_FAKE)
	{
		UT_GrowBuf * pgbCharWidths = getBlock()->getCharWidths()->getCharWidths();
		UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

		UT_sint32 i = findCharacter(0, UCS_SPACE);
		
		// if for some reason the width after justification is the
		// same as it was, we do not need to do this at all
		while (i >= 0)
		{
			// not all spaces have been necessarily adjusted,
			// sometimes, due to rounding errors, we run out of the extra
			// width before we reach the start of the line
			if(pCharWidths[i] != iSpaceWidthBefore)
			{
				_setWidth(getWidth() - (pCharWidths[i] - iSpaceWidthBefore));
				pCharWidths[i] = iSpaceWidthBefore;

				_setRecalcWidth(true);

			}

			// keep looping
			i = findCharacter(i+1-getBlockOffset(), UCS_SPACE);
		}
	}
	
	if(m_pJustifiedSpaces)
	{
		delete m_pJustifiedSpaces;
		m_pJustifiedSpaces = NULL;
	}
}

/*!
    This metod distributes an extra width needed to make the line
    justified betten the spaces in this run
    
    \param UT_sint32 iAmount      : the extra width to distribute
    \param UT_uint32 iSpacesInRun : the number of spaces in this run
    
    \note The new justification algorithm requires a few words of
    explanation; the old algorithm worked like this: if the run was to
    be justified, we split it so that each continous span of spaces
    was contained in its own run. We then divided the extra width
    needed to justify our run between the spaces and adjusted the
    withds of the space-only runs accordingly. This was costly in
    terms of memory and drawing performance; each justification point
    required two runs to represent it, i.e., 2 * sizeof(fp_TextRun),
    368 bytes (as of now)

    The new algorithm avoids the costly spliting of runs by doing only
    a notional split. Every continous span of spaces is described by a
    record of four int's: offset of the spaces from the start of the
    run, length of the span, its (adjusted) width and the width of
    the text span that precedes it. This information is stored in the
    vector pointed to by m_pJustifiedSpaces and is processed by
    fp_TextRun::_draw(). Thus the overall memory requirenment for each
    justification point is 4 * sizeof(void*), 16 bytes; this amounts
    to saving 352 bytes per justification point, i.e., in a fully
    justified document of 5000 words we use 1.7 MB of memory less !!!
    A further advantage of this is that since the extra advance on the
    screen is handled inside of a singe call to fp_TextRun::_draw(),
    rather than in between separate calls to the routine, our drawing
    performance improves because we get rid of the overhead that the
    separate calls required (drawing selection background, setting up
    colours, etc.). In return ::split() and mergeWithNext() become
    slightly more complex.
    
    The one problem with representing the justification points with
    the 4 int's is that when the runs get merged or split, the
    position of the spaces within the run (and when text gets inserted
    or deleted also within the block) changes. To avoid having to
    recalculated all the offsets all the time, we store in our vector
    the block position of the run with which the run-relative offsets
    were calculated; by adjusting this position and using it to
    calculate a difference iDelta we can avoid needing to modify the
    actual offsets of the space segements we keep most of the time.
    
    Also, as the first element of our vector, we keep the original
    width of a space before we applied justification; this does away
    with the need for separate m_iSpaceWidthBeforeJustification member.

    The contents of m_pJustifiedSpaces look like this:

	    offset    |    content
	    -----------------------------------------------------------
		* the vector header
	    0         |    original space width
	    1         |    block offset used to calculate space offsets
	                   (NB: this number is signed !!!)
	    
	    * the space segment records
	    2         |    space segment offset (run-relative)
	    3         |    space segment length
	    4         |    space segment width
	    5         |    width of text immediately before the spaces
	    ...
	    count - 4 |    space segment offset (run-relative)
	    count - 3 |    space segment length
	    count - 2 |    space segment width
	    count - 1 |    width of text immediately before the spaces

    (The reasons why we store the individual numbers sequentially in
    the vector rather than in some kind of a struct is to avoid
    allocation and deallocation of such a structure.)

	It should be noted that the original block offset (OBO) stored at
    index 1 can be not only positive, but also negative; it is merely
    a notional block offset which can only be used for calculating
    difference iDelta from the current block offset (CBO):

        iDelta = CBO - OBO

    iDelta is then used to calculate the real space offset (RSO) from
    the space offset stored in the vector (SO):

        RSO = SO + iDelta

    RSO is run-relative, and where block offset is required,
    adjustement by the block offset of the run is necessary.
    
*/
void fp_TextRun::distributeJustificationAmongstSpaces(UT_sint32 iAmount, UT_uint32 iSpacesInRun)
{
	UT_GrowBuf * pgbCharWidths = getBlock()->getCharWidths()->getCharWidths();
	UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

	if(!iAmount && !iSpacesInRun)
	{
		// I suspect this might now be redundant ...
		_setSpaceWidthBeforeJustification(JUSTIFICATION_FAKE);
		return;
	}

	if(!iAmount)
	{
		// this can happend near the start of the line (the line is
		// processed from back to front) due to rounding errors in
		// the  algorithm; we simply mark the run as one that does not
		// use justification

		// not needed, since we are always called after ::resetJustification()
		// resetJustification();
		return;
	}
	
	UT_uint32 iBlockOffset = getBlockOffset();
	
	if(iSpacesInRun && getLength() > 0)
	{
		_setWidth(getWidth() + iAmount);

		// NB: i is block offset, not run offset !!!
		UT_sint32 i = findCharacter(0, UCS_SPACE);

#if 0 // not needed, since we always follow resetJustification()
		if(m_pJustifiedSpaces)
			m_pJustifiedSpaces->clear();
#endif
		
		UT_ASSERT( i>=0 );
		_setSpaceWidthBeforeJustification(pCharWidths[i]);

		UT_uint32 iVectIndx = 2;
		UT_ASSERT( m_pJustifiedSpaces->getItemCount() == 1 );

		m_pJustifiedSpaces->addItem((void*)iBlockOffset);

		// add the first four slots to our space vector
		for(UT_uint32 j = 1; j < 5; j++)
			m_pJustifiedSpaces->addItem(NULL);


		while ((i >= 0) && (iSpacesInRun))
		{
			UT_sint32 iThisAmount = iAmount / iSpacesInRun;

			pCharWidths[i] += iThisAmount;

			// find the data slot for this space in our vector
			UT_uint32 iSpaceOffset = (UT_uint32)m_pJustifiedSpaces->getNthItem(iVectIndx);
			UT_uint32 iSpaceLength = (UT_uint32)m_pJustifiedSpaces->getNthItem(iVectIndx + 1);
			
			if(iVectIndx == 2 && !iSpaceOffset && !iSpaceLength)
			{
				// this is the first empty slot, just use it
				m_pJustifiedSpaces->setNthItem(2, (void*) (i - iBlockOffset), NULL); // offset
				m_pJustifiedSpaces->setNthItem(3, (void*) 1, NULL); // length
			}
			else
			{
				// we have a proper slot, see if our space fits in it
				UT_uint32 iRunOffset = i - iBlockOffset;
				
				UT_ASSERT( iRunOffset >= (UT_sint32)(iSpaceOffset + iSpaceLength));
				if(iRunOffset == (UT_sint32)(iSpaceOffset + iSpaceLength))
				{
					// this space is immediately after the end of this
					// slot, so we append it
					m_pJustifiedSpaces->setNthItem(iVectIndx + 1, (void*) (iSpaceLength + 1), NULL); // length
				}
				else
				{
					// we need a new data segment
					// add the first four slots to our space vector
					m_pJustifiedSpaces->addItem((void*)(iRunOffset));
					m_pJustifiedSpaces->addItem((void*)1);

					for(UT_uint32 j = 0; j < 2; j++)
						m_pJustifiedSpaces->addItem(NULL); //dummy placeholders

					iVectIndx += 4;

				}

			}


			// now adjust the width
			UT_uint32 iSpaceWidth = (UT_uint32)m_pJustifiedSpaces->getNthItem(iVectIndx + 2);
			m_pJustifiedSpaces->setNthItem(iVectIndx + 2,(void*)(iSpaceWidth + pCharWidths[i]),NULL);

			iAmount -= iThisAmount;

			iSpacesInRun--;

			// keep looping
			i = findCharacter(i+1-iBlockOffset, UCS_SPACE);
		}
		_setRecalcWidth(true);
	}

	UT_ASSERT(iAmount == 0);

	// now we need to calculate the width of the text segments between
	// the space segments to speed up fp_TextRun::_draw()
	UT_uint32 iCount = m_pJustifiedSpaces->getItemCount();
	UT_ASSERT( iCount > 2);
	UT_uint32 iOffset = iBlockOffset;

	//NB: since we just calculated these offsets, we know that they
	//have iDelta == 0, so we do not need to bother with that
	for(UT_uint32 j = 2; j < iCount; j += 4)
	{
		UT_uint32 iSpaceOffset = (UT_uint32)m_pJustifiedSpaces->getNthItem(j);
		UT_uint32 iSpaceLength = (UT_uint32)m_pJustifiedSpaces->getNthItem(j+1);
		UT_uint32 iTextWidth = 0;

		for(UT_uint32 k = iOffset; k < iSpaceOffset + iBlockOffset; k++)
			iTextWidth += pCharWidths[k];

		m_pJustifiedSpaces->setNthItem(j+3, (void*)iTextWidth, NULL);
		iOffset = iBlockOffset + iSpaceOffset + iSpaceLength;
	}
}

UT_uint32 fp_TextRun::countTrailingSpaces(void) const
{
	UT_uint32 iCount = 0;

	if(getLength() > 0)
	{
		UT_UCSChar c;

		UT_sint32 i;
		for (i = getLength() - 1; i >= 0; i--)
		{
			if(getCharacter(i, c) && (UCS_SPACE == c))
			{
				iCount++;
			}
			else
			{
				break;
			}
		}
	}

	return iCount;
}

/*
   if this run contains only spaces, we will return the count
   as a negative value, this will save us having to call
   doesContainNonBlankData() in a couple of loops in fp_Line
*/

UT_sint32 fp_TextRun::countJustificationPoints(void) const
{
	UT_sint32 iCount = 0;
	bool bNonBlank = false;
	UT_sint32 iOldI;

	if(getLength() > 0)
	{
		UT_sint32 i = findCharacter(0, UCS_SPACE);
		iOldI = getBlockOffset() - 1;

		while (i >= 0)
		{
			if(iOldI < i-1) // i.e., something between the two spaces
				bNonBlank = true;

			iCount++;
			iOldI = i;

			// keep looping
			i = findCharacter(i+1-getBlockOffset(), UCS_SPACE);
		}
	}

	UT_ASSERT(iCount >= 0);

	if(!bNonBlank)
	{
		return -iCount;
	}
	else
	{
		return iCount;
	}
}

bool fp_TextRun::canContainPoint(void) const
{
	if (getField())
	{
		return false;
	}
	else
	{
		return true;
	}
}

//fill str of size iMax with the text of the run, return 0 on success,
//size of str required if not not enough space, -1 otherwise
//it further returns the len copied (or desired to be copied) into pStr in iMax

UT_sint32 fp_TextRun::getStr(UT_UCSChar * pStr, UT_uint32 &iMax)
{
	const UT_UCSChar* pSpan = NULL;
	UT_uint32 lenSpan = 0;
	UT_uint32 offset = getBlockOffset();
	UT_uint32 len = getLength();
	bool bContinue = true;
	UT_UCSChar * pStrPos = pStr;

	if(iMax <= len)
	{
		iMax = len;
		return(len);//if there is not enough space in the passed string
			//return size needed
	}

	if (len > 0)
	{
		while (bContinue)
		{
			bContinue = getBlock()->getSpanPtr(offset, &pSpan, &lenSpan);
			UT_ASSERT(lenSpan>0);

			if (len <= lenSpan) 	//copy the entire len to pStr and return 0
			{
				UT_ASSERT(len>0);
						UT_UCS4_strncpy(pStrPos, pSpan, len);
						pStr[len] = 0;						  //make sure the string is 00-terminated
						iMax = getLength();
						return(false);
			}
			else  //copy what we have got and move on to the next span
			{
				UT_UCS4_strncpy(pStrPos, pSpan, lenSpan);
				offset += lenSpan;
				len -= lenSpan;
				pStrPos += lenSpan;
			}
		}
	}
	else //this run is empty, fill pStr with 00 and return 0
	{
		*pStr = 0;
		iMax = 0;
		return false;
	}
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}


void fp_TextRun::setDirection(FriBidiCharType dir, FriBidiCharType dirOverride)
{
	if( !getLength()
	|| (   dir == FRIBIDI_TYPE_UNSET
		&& _getDirection() != FRIBIDI_TYPE_UNSET
		&& dirOverride == m_iDirOverride
		)
	  )
		return; //ignore 0-length runs, let them be treated on basis of the app defaults

	FriBidiCharType prevDir = m_iDirOverride == FRIBIDI_TYPE_UNSET ? _getDirection() : m_iDirOverride;
	if(dir == FRIBIDI_TYPE_UNSET)
	{
		// only do this once
		if(_getDirection() == FRIBIDI_TYPE_UNSET)
		{
			UT_UCSChar firstChar;
			getCharacter(0, firstChar);

			_setDirection(fribidi_get_type((FriBidiChar)firstChar));
		}
	}
	else //meaningfull value received
	{
		_setDirection(dir);
	}

	xxx_UT_DEBUGMSG(("fp_TextRun (0x%x)::setDirection: %d (passed %d, override %d, prev. %d)\n", this, _getDirection(), dir, m_iDirOverride, prevDir));

// 	FriBidiCharType iOldOverride = m_iDirOverride;
	m_iDirOverride = dirOverride;

	// if we set dir override to a strong value, set also visual direction
	// if we set it to UNSET, and the new direction is srong, then we set
	// it to that direction, if it is weak, we have to make the line
	// to calculate it

	if(dirOverride != FRIBIDI_TYPE_UNSET)
		setVisDirection(dirOverride);

	/*
		if this run belongs to a line we have to notify the line that
		that it now contains a run of this direction, if it does not belong
		to a line this will be taken care of by the fp_Line:: member function
		used to add the run to the line (generally, we set it here if this
		is a run that is being typed in and it gets set in the member
		functions when the run is loaded from a document on the disk.)
	*/

	FriBidiCharType curDir = m_iDirOverride == FRIBIDI_TYPE_UNSET ? _getDirection() : m_iDirOverride;

	UT_ASSERT(curDir != FRIBIDI_TYPE_UNSET);

	if(curDir != prevDir)
	{
		// TODO -- not sure this is necessary
		clearScreen();
		markDrawBufferDirty();

		if(getLine())
		{
			getLine()->changeDirectionUsed(prevDir,curDir,true);
			//getLine()->setNeedsRedraw();
		}
	}

	//UT_DEBUGMSG(("TextRun::setDirection: direction=%d\n", _getDirection()));
}

UT_UCSChar getMirrorChar(UT_UCSChar c)
{
	//got to do this, otherwise bsearch screws up
	FriBidiChar fbc = (FriBidiChar) c, mfbc;

	if (fribidi_get_mirror_char (/* Input */ fbc, /* Output */&mfbc))
		return (UT_UCSChar) mfbc;
	else
		return c;
}

void fp_TextRun::_refreshPrefs(UT_Worker * /*pWorker*/)
{
	XAP_App::getApp()->getPrefsValueBool((XML_Char*)XAP_PREF_KEY_UseContextGlyphs, &s_bUseContextGlyphs);
	XAP_App::getApp()->getPrefsValueBool((XML_Char*)XAP_PREF_KEY_SaveContextGlyphs, &s_bSaveContextGlyphs);
	xxx_UT_DEBUGMSG(("fp_TextRun::_refreshPrefs: new values %d, %d\n", s_bUseContextGlyphs, s_bSaveContextGlyphs));
}

/*
	NB !!!
	This function will set the m_iDirOverride member and change the properties
	in the piece table correspondingly; however, note that if dir ==
	FRIBIDI_TYPE_UNSET, this function must immediately return.

	Because of this specialised behaviour and since it does not do any updates, etc.,
	its usability is limited -- its main purpose is to allow to set this property
	in response to inserting a Unicode direction token in fl_BlockLayout _immediately_
	after the run is created. For all other purposes one of the standard
	edit methods should be used.
*/
void fp_TextRun::setDirOverride(FriBidiCharType dir)
{
	if(dir == FRIBIDI_TYPE_UNSET || dir == m_iDirOverride)
		return;

	const XML_Char * prop[] = {NULL, NULL, 0};
	const XML_Char direction[] = "dir-override";
	const XML_Char rtl[] = "rtl";
	const XML_Char ltr[] = "ltr";

	prop[0] = (XML_Char*) &direction;

	switch(dir)
	{
		case FRIBIDI_TYPE_LTR:
			prop[1] = (XML_Char*) &ltr;
			break;
		case FRIBIDI_TYPE_RTL:
			prop[1] = (XML_Char*) &rtl;
			break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	};

	m_iDirOverride = dir;

	UT_uint32 offset = getBlock()->getPosition() + getBlockOffset();
	getBlock()->getDocument()->changeSpanFmt(PTC_AddFmt,offset,offset + getLength(),NULL,prop);

	UT_DEBUGMSG(("fp_TextRun::setDirOverride: offset=%d, len=%d, dir=\"%s\"\n", offset,getLength(),prop[1]));
}

#ifdef SMART_RUN_MERGING
void fp_TextRun::breakNeighborsAtDirBoundaries()
{
	FriBidiCharType iPrevType, iType = FRIBIDI_TYPE_UNSET;
	FriBidiCharType iDirection = getDirection();

	fp_TextRun *pNext = NULL, *pPrev = NULL, *pOtherHalf;
	PT_BlockOffset curOffset = 0;
	const UT_UCSChar* pSpan;
	UT_uint32 spanOffset = 0;
	UT_uint32 lenSpan = 0;

	if(  getPrev()
	  && getPrev()->getType() == FPRUN_TEXT
	  && getPrev()->getVisDirection() != iDirection)
	{
		pPrev = static_cast<fp_TextRun*>(getPrev());
		curOffset = pPrev->getBlockOffset() + pPrev->getLength() - 1;
	}

	while(pPrev)
	{
		getBlock()->getSpanPtr((UT_uint32) curOffset, &pSpan, &lenSpan);
		if ( pSpan == (UT_UCSChar *)NULL || !lenSpan )
			break;
		iPrevType = fribidi_get_type((FriBidiChar)pSpan[0]);

		while(curOffset > pPrev->getBlockOffset() && !FRIBIDI_IS_STRONG(iType))
		{
			curOffset--;
			getBlock()->getSpanPtr((UT_uint32) curOffset, &pSpan, &lenSpan);
			iType = fribidi_get_type((FriBidiChar)pSpan[0]);
			if(iType != iPrevType)
			{
				pPrev->split(curOffset+1);

				//now we want to reset the direction of the second half
				UT_ASSERT(pPrev->getNext()->getType() == FPRUN_TEXT);
				pOtherHalf = static_cast<fp_TextRun*>(pPrev->getNext());
				pOtherHalf->setDirection(iPrevType, pOtherHalf->getDirOverride());
				iPrevType = iType;
				// we do not want to break here, since pPrev still points to the
				// left part, so we can carry on leftwards
			}
		}

		if(FRIBIDI_IS_STRONG(iPrevType))
			break;

		// if we got this far, the whole previous run is weak, so we want to
		// reset its direction and proceed with the run before it
		pPrev->setDirection(iPrevType, pPrev->getDirOverride());

		if(pPrev->getPrev() && pPrev->getPrev()->getType() == FPRUN_TEXT)
		{
			pPrev = static_cast<fp_TextRun*>(pPrev->getPrev());
			curOffset = pPrev->getBlockOffset() + pPrev->getLength() - 1;
		}
		else
			break;

	}

	// now do the same thing with the following run
	if(  getNext()
	  && getNext()->getType() == FPRUN_TEXT
	  && getNext()->getVisDirection() != iDirection)
	{
		pNext = static_cast<fp_TextRun*>(getNext());
		curOffset = pNext->getBlockOffset();
	}

	iType = FRIBIDI_TYPE_UNSET;
	while(pNext)
	{
		getBlock()->getSpanPtr((UT_uint32) curOffset, &pSpan, &lenSpan);
		if ( pSpan == (UT_UCSChar *)NULL || !lenSpan )
			break;
		iPrevType = fribidi_get_type((FriBidiChar)pSpan[0]);
		bool bDirSet = false;
		spanOffset = 0;
		while(curOffset + spanOffset < pNext->getBlockOffset() + pNext->getLength() - 1 && !FRIBIDI_IS_STRONG(iType))
		{
			spanOffset++;
			if(spanOffset >= lenSpan)
			{
				curOffset += spanOffset;
				spanOffset = 0;
				getBlock()->getSpanPtr((UT_uint32) curOffset, &pSpan, &lenSpan);
			}
			iType = fribidi_get_type((FriBidiChar)pSpan[spanOffset]);
			if(iType != iPrevType)
			{
				pNext->split(curOffset + spanOffset);
				pNext->setDirection(iPrevType, pNext->getDirOverride());

				// now set direction of the second half
				UT_ASSERT(pNext->getNext()->getType() == FPRUN_TEXT);
				pOtherHalf = static_cast<fp_TextRun*>(pNext->getNext());

				pOtherHalf->setDirection(iType, pOtherHalf->getDirOverride());
				bDirSet = true;
				iPrevType = iType; // not needed

				// since pNext points now to the left half, the right-ward processing
				// cannot continue, but insteds we need to proceed with the new run
				// on the right
				break;
			}
		}

		if(FRIBIDI_IS_STRONG(iPrevType))
			break;

		// if we got this far, the whole next run is weak, so we want to
		// reset its direction, unless we split it, in which case it has already
		// been set
		// then proceed with the run after it
		if(!bDirSet)
			pNext->setDirection(iPrevType, pNext->getDirOverride());

		if(pNext->getNext() && pNext->getNext()->getType() == FPRUN_TEXT)
		{
			pNext = static_cast<fp_TextRun*>(pNext->getNext());
			curOffset = pNext->getBlockOffset();
		}
		else
			break;

	}
}

void fp_TextRun::breakMeAtDirBoundaries(FriBidiCharType iNewOverride)
{
	// we cannot use the draw buffer here because in case of ligatures it might
	// contain characters of misleading directional properties
	fp_TextRun * pRun = this;
	UT_uint32 iLen = getLength();  // need to remember this, since getLength() will change if we split
	PT_BlockOffset currOffset = getBlockOffset();
	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan = 0;
	UT_uint32 spanOffset = 0;
	FriBidiCharType iPrevType, iType = FRIBIDI_TYPE_UNSET;
	getBlock()->getSpanPtr((UT_uint32) currOffset, &pSpan, &lenSpan);
	iPrevType = iType = fribidi_get_type((FriBidiChar)pSpan[spanOffset]);

	while((currOffset + spanOffset) < (getBlockOffset() + iLen))
	{
		while(iPrevType == iType && ((currOffset + spanOffset) < (getBlockOffset() + iLen - 1)))
		{
			spanOffset++;
			if(spanOffset >= lenSpan)
			{
				currOffset += spanOffset;
				spanOffset = 0;
				getBlock()->getSpanPtr((UT_uint32) currOffset, &pSpan, &lenSpan);
			}

			iType = fribidi_get_type((FriBidiChar)pSpan[spanOffset]);
		}

		// if we reached the end of the origianl run, then stop
		if((currOffset + spanOffset) >= (getBlockOffset() + iLen - 1))
		{
			pRun->setDirection(iPrevType, iNewOverride);
			break;
		}

		// so we know where the continuos fragment ends ...
		pRun->split(currOffset + spanOffset);
		pRun->setDirection(iPrevType, iNewOverride);
		UT_ASSERT(pRun->getNext() && pRun->getNext()->getType() == FPRUN_TEXT);
		pRun = static_cast<fp_TextRun*>(pRun->getNext());
		iPrevType = iType;
	}
}
#endif


#ifdef WITH_PANGO
void fp_TextRun::_freeGlyphString()
{
	if(m_pGlyphString)
	{
		g_list_foreach(m_pGlyphString, UT_free1PangoGlyphString, NULL);
		g_list_free(m_pGlyphString);
		m_pGlyphString = NULL;
	}
}
#endif

/*!
    returns the original width of space before justification or -1 if
    justification is not used
*/
UT_sint32 fp_TextRun::_getSpaceWidthBeforeJustification()
{
	if(!m_pJustifiedSpaces)
		return JUSTIFICATION_NOT_USED;

	UT_return_val_if_fail(m_pJustifiedSpaces->getItemCount() > 0, JUSTIFICATION_NOT_USED);

	return (UT_sint32) m_pJustifiedSpaces->getNthItem(0);
}

/*!
    sets the width of space before justification to iWidth
    \param UT_sint32 iWidth -- width of spaces in this run before
    justification was applied
*/
void fp_TextRun::_setSpaceWidthBeforeJustification(UT_sint32 iWidth)
{
	if(!m_pJustifiedSpaces)
	{
		m_pJustifiedSpaces = new UT_Vector;
	}
	
	if(m_pJustifiedSpaces->getItemCount() == 0)
		m_pJustifiedSpaces->addItem(NULL);

	m_pJustifiedSpaces->setNthItem(0, (void*) iWidth, NULL);
}
