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
#include "fribidi.h"
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
UT_UCSChar getMirrorChar(UT_UCSChar c);


fp_TextRun::fp_TextRun(fl_BlockLayout* pBL,
					   GR_Graphics* pG,
					   UT_uint32 iOffsetFirst,
					   UT_uint32 iLen,
					   bool bLookupProperties)
:	fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_TEXT),
	m_fPosition(TEXT_POSITION_NORMAL)
{
	m_fDecorations = 0;
	m_iLineWidth = 0;
	m_bSquiggled = false;
	m_iSpaceWidthBeforeJustification = JUSTIFICATION_NOT_USED;
	//m_iOldSpaceWidthBeforeJustification = JUSTIFICATION_NOT_USED;
	m_pField = NULL;
	m_pLanguage = NULL;
	m_bIsOverhanging = false;

	m_iDirection = FRIBIDI_TYPE_UNSET; //we will use this as an indication that the direction property has not been yet set
			  //normal values are -1,0,1 (neutral, ltr, rtl)
	m_iDirOverride = FRIBIDI_TYPE_UNSET; //no override by default

	if (bLookupProperties)
	{
		lookupProperties();
	}

	m_bRecalcWidth = true;
	m_bRefreshDrawBuffer = true;

	if(!s_pPrefsTimer)
	{
		XAP_App::getApp()->getPrefsValueBool((XML_Char*)XAP_PREF_KEY_UseContextGlyphs, &s_bUseContextGlyphs);
		XAP_App::getApp()->getPrefsValueBool((XML_Char*)XAP_PREF_KEY_SaveContextGlyphs, &s_bSaveContextGlyphs);
		s_pPrefsTimer = UT_Timer::static_constructor(_refreshPrefs, NULL);
		if(s_pPrefsTimer)
			s_pPrefsTimer->set(PREFS_REFRESH_MSCS);
	}

#ifndef WITH_PANGO
	m_pSpanBuff = new UT_UCSChar[m_iLen + 1];
	m_iSpanBuffSize = m_iLen;
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
	}

#ifndef WITH_PANGO
	delete[] m_pSpanBuff;
#else
	pango_glyph_string_free();
#endif

	delete m_pRevisions;
}

bool fp_TextRun::hasLayoutProperties(void) const
{
	return true;
}

/*! a private method used internally by lookupProperties() */
void fp_TextRun::_processProperties(const PP_AttrProp * pSpanAP,
									const PP_AttrProp * pBlockAP,
									const PP_AttrProp * pSectionAP)
{
	static_cast<fl_Layout *>(m_pBL)->getField(m_iOffsetFirst,m_pField);
	// look for fonts in this DocLayout's font cache
	FL_DocLayout * pLayout = m_pBL->getDocLayout();

	PD_Document * pDoc = m_pBL->getDocument();

	const PP_PropertyTypeColor *p_color = (const PP_PropertyTypeColor *)PP_evalPropertyType("color",pSpanAP,pBlockAP,pSectionAP, Property_type_color, pDoc, true);
	UT_ASSERT(p_color);
	m_colorFG = p_color->getColor();


	getHighlightColor();
	getPageColor();

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
	m_iLineWidth = m_pG->convertDimension("0.8pt");

	m_fDecorations = 0;

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
			m_fDecorations |= TEXT_DECOR_UNDERLINE;
		}
		else if (0 == UT_strcmp(q, "overline"))
		{
			m_fDecorations |= TEXT_DECOR_OVERLINE;
		}
		else if (0 == UT_strcmp(q, "line-through"))
		{
			m_fDecorations |= TEXT_DECOR_LINETHROUGH;
		}
		else if (0 == UT_strcmp(q, "topline"))
		{
			m_fDecorations |= TEXT_DECOR_TOPLINE;
		}
		else if (0 == UT_strcmp(q, "bottomline"))
		{
			m_fDecorations |= TEXT_DECOR_BOTTOMLINE;
		}
		q = strtok(NULL, " ");
	}

	free(p);

	const XML_Char * pszPosition = PP_evalProperty("text-position",pSpanAP,pBlockAP,pSectionAP, pDoc, true);

	if (0 == UT_strcmp(pszPosition, "superscript"))
	{
		m_fPosition = TEXT_POSITION_SUPERSCRIPT;
	}
	else if (0 == UT_strcmp(pszPosition, "subscript"))
	{
		m_fPosition = TEXT_POSITION_SUBSCRIPT;
	}
	else m_fPosition = TEXT_POSITION_NORMAL;

#ifndef WITH_PANGO
	GR_Font * pFont;

	pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);
	if (m_pScreenFont != pFont)
	  {
		m_bRecalcWidth = true;
		m_pScreenFont = pFont;
		m_iAscent = m_pG->getFontAscent(m_pScreenFont);
		m_iDescent = m_pG->getFontDescent(m_pScreenFont);
		m_iHeight = m_pG->getFontHeight(m_pScreenFont);
	  }

	pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_LAYOUT_RESOLUTION);
	if (m_pLayoutFont != pFont)
	  {
		m_bRecalcWidth = true;
		m_pLayoutFont = pFont;
		m_iAscentLayoutUnits = m_pG->getFontAscent(m_pLayoutFont);
		UT_ASSERT(m_iAscentLayoutUnits);
		m_iDescentLayoutUnits = m_pG->getFontDescent(m_pLayoutFont);
		m_iHeightLayoutUnits = m_pG->getFontHeight(m_pLayoutFont);
	  }
#else
	PangoFont * pFont;

	pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP);
	if (m_pPangoFont != pFont)
	  {
		m_bRecalcWidth = true;
		m_pPangoFont = pFont;
		m_iAscent = m_pG->getFontAscent(m_pScreenFont);
		m_iDescent = m_pG->getFontDescent(m_pScreenFont);
		m_iHeight = m_pG->getFontHeight(m_pScreenFont);
	  }
#endif

#ifndef WITH_PANGO
	m_pG->setFont(m_pScreenFont);
#else
	m_pG->setFont(m_pPangoFont);
#endif

	//set the language member
	UT_Language *lls = new UT_Language;
	const XML_Char * pszLanguage = PP_evalProperty("lang",pSpanAP,pBlockAP,pSectionAP, pDoc, true);

	const XML_Char * pszOldLanguage = m_pLanguage;
	m_pLanguage = lls->getPropertyFromProperty(pszLanguage);
	if(pszOldLanguage && m_pLanguage != pszOldLanguage)
		m_pBL->getDocLayout()->queueBlockForBackgroundCheck((UT_uint32) FL_DocLayout::bgcrSpelling, m_pBL);
	//UT_DEBUGMSG(("fp_TextRun::lookupProperties: m_pLanguage = %s\n", m_pLanguage));
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
}


void fp_TextRun::lookupProperties(void)
{
	clearScreen();

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
	bool bDelete;

	m_pBL->getAttrProp(&pBlockAP);

	// examining the m_pRevisions contents is too involved, it is
	// faster to delete it and create a new instance if needed
	if(m_pRevisions)
	{
		delete m_pRevisions;
		m_pRevisions = NULL;
	}

	// NB this call will recreate m_pRevisions for us
	getSpanAP(pSpanAP,bDelete);

	_processProperties(pSpanAP, pBlockAP, pSectionAP);

	//if we are responsible for deleting pSpanAP, then just do so
	if(bDelete)
		delete pSpanAP;
}

bool fp_TextRun::canBreakAfter(void) const
{
	const UT_UCSChar* pSpan = NULL;
	UT_uint32 lenSpan = 0;
	UT_uint32 offset = m_iOffsetFirst;
	UT_uint32 len = m_iLen;
	bool bContinue = true;

	if (len > 0)
	{
		while (bContinue)
		{
			bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
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
	else if (!m_pNext)
	{
		return true;
	}

	if (m_pNext)
	{
		return m_pNext->canBreakBefore();
	}

	return false;
}

bool fp_TextRun::canBreakBefore(void) const
{
	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;

	if (m_iLen > 0)
	{
		if (m_pBL->getSpanPtr(m_iOffsetFirst, &pSpan, &lenSpan))
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
		if (m_pNext)
		{
			return m_pNext->canBreakBefore();
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

	if (m_iLen > 0)
	{
		if (m_pBL->getSpanPtr(m_iOffsetFirst, &pSpan, &lenSpan))
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
	UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidthsLayoutUnits();
	UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

	UT_sint32 iLeftWidth = 0;
	UT_sint32 iRightWidth = m_iWidthLayoutUnits;

	si.iOffset = -1;

	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;
	UT_uint32 offset = m_iOffsetFirst;
	UT_uint32 len = m_iLen;
	bool bContinue = true;

	while (bContinue)
	{
		bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
		UT_ASSERT(lenSpan>0);

		if (lenSpan > len)
		{
			lenSpan = len;
		}

		for (UT_uint32 i=0; i<lenSpan; i++)
		{
			iLeftWidth += pCharWidths[i + offset];
			iRightWidth -= pCharWidths[i + offset];
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
					&& ((i + offset) != (m_iOffsetFirst + m_iLen - 1))
					)
				|| bForce
				)
			{
				if (iLeftWidth - pCharWidths[i + offset] <= iMaxLeftWidth)
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
		|| (si.iLeftWidth == m_iWidthLayoutUnits)
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
	FriBidiCharType iDomDirection = m_pBL->getDominantDirection();

	if (x <= 0)
	{
		if(iVisDirection == FRIBIDI_TYPE_RTL)
		{
			pos = m_pBL->getPosition() + m_iOffsetFirst + m_iLen;
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
			pos = m_pBL->getPosition() + m_iOffsetFirst;
			// don't set bBOL to false here
			bEOL = false;
		}

		return;
	}

	if (x >= m_iWidth)
	{
		if(iVisDirection == FRIBIDI_TYPE_RTL)
		{
			pos = m_pBL->getPosition() + m_iOffsetFirst;

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
			pos = m_pBL->getPosition() + m_iOffsetFirst + m_iLen;
			// Setting bEOL fixes bug 1149. But bEOL has been set in the
			// past - probably somewhere else, so this is not necessarily
			// the correct place to do it.	2001.02.25 jskov
			bEOL = true;
		}

		return;
	}

	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
	const UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

	// catch the case of a click directly on the left half of the first character in the run
	if (x < (pCharWidths[m_iOffsetFirst] / 2))
	{
		pos = m_pBL->getPosition() + getOffsetFirstVis();

		// if this character is RTL then clicking on the left side
		// means the user wants to postion the caret _after_ this char
		if(iVisDirection == FRIBIDI_TYPE_RTL)
			pos++;

		bBOL = false;
		bEOL = false;
		return;
	}

	UT_sint32 iWidth = 0;
	for (UT_uint32 i=m_iOffsetFirst; i<(m_iOffsetFirst + m_iLen); i++)
	{
		// i represents VISUAL offset but the CharWidths array uses logical order of indexing
		UT_uint32 iLog = getOffsetLog(i);

		iWidth += pCharWidths[iLog];

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
			pos = m_pBL->getPosition() + iLog;
			return;
		}
	}

	UT_DEBUGMSG(("fp_TextRun::mapXYToPosition: x %d, m_iWidth %d\n", x,m_iWidth));
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void fp_TextRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	UT_sint32 xoff;
	UT_sint32 yoff;
	UT_sint32 xoff2;
	UT_sint32 yoff2;
	UT_sint32 xdiff = 0;

	UT_ASSERT(m_pLine);
	m_pLine->getOffsets(this, xoff, yoff);
	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
	const UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

	UT_uint32 offset = UT_MIN(iOffset, m_iOffsetFirst + m_iLen);

	for (UT_uint32 i=m_iOffsetFirst; i<offset; i++)
	{
			xdiff += pCharWidths[i];
	}

	if (m_fPosition == TEXT_POSITION_SUPERSCRIPT)
	{
		yoff -= m_iAscent * 1/2;
	}
	else if (m_fPosition == TEXT_POSITION_SUBSCRIPT)
	{
		yoff += m_iDescent /* * 3/2 */;
	}

	UT_sint32 iDirection = getVisDirection();
	UT_sint32 iNextDir = iDirection == FRIBIDI_TYPE_RTL ? FRIBIDI_TYPE_LTR : FRIBIDI_TYPE_RTL; //if this is last run we will anticipate the next to have *different* direction
	fp_Run * pRun = 0;	 //will use 0 as indicator that there is no need to deal with the second caret

	if(offset == (m_iOffsetFirst + m_iLen)) //this is the end of the run
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
		x = xoff + m_iWidth - xdiff; //we want the caret right of the char
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
	height = m_iHeight;
}

bool fp_TextRun::canMergeWithNext(void)
{
	if (!m_pNext ||
		!m_pLine ||
		m_pNext->getType() != FPRUN_TEXT ||
		!m_pNext->getLine())
	{
		return false;
	}


	fp_TextRun* pNext = static_cast<fp_TextRun*>(m_pNext);
	if (
		(pNext->m_iOffsetFirst != (m_iOffsetFirst + m_iLen))
		|| (pNext->m_fDecorations != m_fDecorations)
		|| (pNext->m_pScreenFont != m_pScreenFont)
		|| (pNext->m_pLayoutFont != m_pLayoutFont)
		|| (m_iHeight != pNext->m_iHeight)
		// runs that have been justified cannot be merged together, because
		// this would result in the screen text being out of sync with the actual
		// character positions
		|| (m_iSpaceWidthBeforeJustification != JUSTIFICATION_NOT_USED)
		|| (pNext->m_iSpaceWidthBeforeJustification != JUSTIFICATION_NOT_USED)
		|| (pNext->m_pField != m_pField)
		|| (pNext->m_pLanguage != m_pLanguage)	//this is not a bug
		|| (pNext->m_colorFG != m_colorFG)
		|| (pNext->m_colorHL != m_colorHL)
#ifdef SMART_RUN_MERGING
		|| (pNext->getVisDirection() != getVisDirection())
		// we also want to test the override, because we do not want runs that have the same
		// visual direction but different override merged
#else
		|| (pNext->m_iDirection != m_iDirection)  //#TF cannot merge runs of different direction of writing
#endif
		|| (pNext->m_iDirOverride != m_iDirOverride)

		/* the revision evaluation is a bit more complex*/
		|| ((  m_pRevisions != pNext->m_pRevisions) // non-identical and one is null
			&& (!m_pRevisions || !pNext->m_pRevisions))
		|| ((  m_pRevisions && pNext->m_pRevisions)
		    && !(*m_pRevisions == *(pNext->m_pRevisions))) //non-null but different
		)
	{
		return false;
	}

    return true;
}

void fp_TextRun::mergeWithNext(void)
{
	UT_ASSERT(m_pNext && (m_pNext->getType() == FPRUN_TEXT));
	UT_ASSERT(m_pLine);
	UT_ASSERT(m_pNext->getLine());

	fp_TextRun* pNext = (fp_TextRun*) m_pNext;

	UT_ASSERT(pNext->m_iOffsetFirst == (m_iOffsetFirst + m_iLen));
	UT_ASSERT(pNext->m_pScreenFont == m_pScreenFont);	// is this legal?
	UT_ASSERT(pNext->m_pLayoutFont == m_pLayoutFont);	// is this legal?
	UT_ASSERT(pNext->m_fDecorations == m_fDecorations);
	UT_ASSERT(m_iAscent == pNext->m_iAscent);
	UT_ASSERT(m_iDescent == pNext->m_iDescent);
	UT_ASSERT(m_iHeight == pNext->m_iHeight);
	UT_ASSERT(m_iLineWidth == pNext->m_iLineWidth);
	UT_ASSERT(m_pLanguage == pNext->m_pLanguage); //this is not a bug
#ifndef SMART_RUN_MERGING
	UT_ASSERT(m_iDirection == pNext->m_iDirection); //#TF
#endif
	UT_ASSERT(m_iDirOverride == pNext->m_iDirOverride); //#TF
	//UT_ASSERT(m_iSpaceWidthBeforeJustification == pNext->m_iSpaceWidthBeforeJustification);

	m_pField = pNext->m_pField;
	m_iWidth += pNext->m_iWidth;
#ifndef WITH_PANGO
	m_iWidthLayoutUnits += pNext->m_iWidthLayoutUnits;
#endif

	UT_DEBUGMSG(("fp_TextRun::mergeWithNext\n"));
	// first of all, make sure the X coordinance of the merged run is correct

	if(m_iX > pNext->m_iX)
		m_iX = pNext->m_iX;

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
		  || (s_bBidiOS && m_iDirOverride == FRIBIDI_TYPE_LTR && m_iDirection == FRIBIDI_TYPE_RTL);

	if((m_iSpanBuffSize <= m_iLen + pNext->m_iLen) || (bReverse && (m_iLen > pNext->m_iLen)))
	{
		UT_DEBUGMSG(("fp_TextRun::mergeWithNext: reallocating span buffer\n"));
		m_iSpanBuffSize = m_iLen + pNext->m_iLen + 1;
		UT_UCSChar * pSB = new UT_UCSChar[m_iSpanBuffSize];
		UT_ASSERT(pSB);
		if(bReverse)
		{
			UT_UCS4_strncpy(pSB, pNext->m_pSpanBuff, pNext->m_iLen);
			UT_UCS4_strncpy(pSB + pNext->m_iLen,m_pSpanBuff, m_iLen);
		}
		else
		{
			UT_UCS4_strncpy(pSB,m_pSpanBuff, m_iLen);
			UT_UCS4_strncpy(pSB + m_iLen, pNext->m_pSpanBuff, pNext->m_iLen);
		}

		*(pSB + m_iLen + pNext->m_iLen) = 0;
		delete [] m_pSpanBuff;
		m_pSpanBuff = pSB;
	}
	else
	{
		UT_DEBUGMSG(("fp_TextRun::mergeWithNext: reusing existin span buffer\n"));
		if(bReverse)
		{
			// can only shift the text directly in the existing buffer if
			// m_iLen <= pNext->m_iLen
			UT_ASSERT(m_iLen <= pNext->m_iLen);
			UT_UCS4_strncpy(m_pSpanBuff + pNext->m_iLen, m_pSpanBuff, m_iLen);
			UT_UCS4_strncpy(m_pSpanBuff, pNext->m_pSpanBuff, pNext->m_iLen);
		}
		else
		{
			UT_UCS4_strncpy(m_pSpanBuff + m_iLen, pNext->m_pSpanBuff, pNext->m_iLen);
		}
		*(m_pSpanBuff + m_iLen + pNext->m_iLen) = 0;
	}

	m_iLen += pNext->m_iLen;
	m_bDirty = m_bDirty || pNext->m_bDirty;
	m_pNext = pNext->getNext();
	if (m_pNext)
	{
		// do not mark anything dirty
		m_pNext->setPrev(this, false);
	}

	pNext->getLine()->removeRun(pNext, false);

#ifdef SMART_RUN_MERGING
	// if appending a strong run onto a weak one, make sure the overall direction
	// is that of the strong run, and tell the line about this, since the call
	// to removeRun above decreased the line's direction counter
	if(!FRIBIDI_IS_STRONG(m_iDirection) && FRIBIDI_IS_STRONG(pNext->m_iDirection))
	{
		m_iDirection = pNext->m_iDirection;
		m_pLine->addDirectionUsed(m_iDirection);
	}
#endif

	delete pNext;
}

bool fp_TextRun::split(UT_uint32 iSplitOffset)
{
	UT_ASSERT(iSplitOffset >= m_iOffsetFirst);
	UT_ASSERT(iSplitOffset < (m_iOffsetFirst + m_iLen));

	FriBidiCharType iVisDirection = getVisDirection();
	fp_TextRun* pNew = new fp_TextRun(m_pBL, m_pG, iSplitOffset, m_iLen - (iSplitOffset - m_iOffsetFirst), false);

	UT_ASSERT(pNew);
#ifndef WITH_PANGO
	pNew->m_pScreenFont = this->m_pScreenFont;
	pNew->m_pLayoutFont = this->m_pLayoutFont;
#else
	pNew->m_pPangoFont = this->m_pPangoFont;
#endif

	pNew->m_fDecorations = this->m_fDecorations;
	pNew->m_colorFG = this->m_colorFG;
	pNew->m_pField = this->m_pField;
		pNew->m_fPosition = this->m_fPosition;

	pNew->m_iAscent = this->m_iAscent;
	pNew->m_iDescent = this->m_iDescent;
	pNew->m_iHeight = this->m_iHeight;
#ifndef WITH_PANGO
	pNew->m_iAscentLayoutUnits = this->m_iAscentLayoutUnits;
	pNew->m_iDescentLayoutUnits = this->m_iDescentLayoutUnits;
	pNew->m_iHeightLayoutUnits = this->m_iHeightLayoutUnits;
#endif
	pNew->m_iLineWidth = this->m_iLineWidth;
	pNew->m_bDirty = this->m_bDirty;
	pNew->m_pLanguage = this->m_pLanguage;
	pNew->m_iDirection = this->m_iDirection; //#TF
	pNew->m_iDirOverride = this->m_iDirOverride;
#ifdef SMART_RUN_MERGING
	// set the visual direction to same as that of the old run
	pNew->setVisDirection(iVisDirection);
#endif
	// need to set this, so the split run could reset its justification correctly
	pNew->m_iSpaceWidthBeforeJustification = this->m_iSpaceWidthBeforeJustification;

	pNew->m_pPrev = this;
	pNew->m_pNext = this->m_pNext;
	if (m_pNext)
	{
		// do not mark anything dirty
		m_pNext->setPrev(pNew, false);
	}
	m_pNext = pNew;

	m_iLen = iSplitOffset - m_iOffsetFirst;

	m_pLine->insertRunAfter(pNew, this);

	// first of all, split the span buffer, this will save us refreshing it
	// which is very expensive (see notes on the mergeWithNext())

	// we have a dilema here, either we can leave the span buffer for this run
	// of the size it is now, or we delete it and allocate a shorter one
	// we will use the first option, the extra delete/new pair should not
	// cost us too much time
	m_iSpanBuffSize = m_iLen + 1;
	UT_UCSChar * pSB = new UT_UCSChar[m_iSpanBuffSize];

	if((!s_bBidiOS && iVisDirection == FRIBIDI_TYPE_RTL)
	  || (s_bBidiOS && m_iDirOverride == FRIBIDI_TYPE_LTR && m_iDirection == FRIBIDI_TYPE_RTL)
	)
	{
		UT_UCS4_strncpy(pSB, m_pSpanBuff + pNew->m_iLen, m_iLen);
		UT_UCS4_strncpy(pNew->m_pSpanBuff, m_pSpanBuff, pNew->m_iLen);
	}
	else
	{
		UT_UCS4_strncpy(pSB, m_pSpanBuff, m_iLen);
		UT_UCS4_strncpy(pNew->m_pSpanBuff, m_pSpanBuff + m_iLen, pNew->m_iLen);
	}

	pSB[m_iLen] = 0;
	pNew->m_pSpanBuff[pNew->m_iLen] = 0;

	delete[] m_pSpanBuff;
	m_pSpanBuff = pSB;

	// we will use the _addupCharWidths() function here instead of recalcWidth(), since when
	// a run is split the info in the block's char-width array is not affected, so we do not
	//have to recalculate these
	_addupCharWidths();
	pNew->_addupCharWidths();


	//bool bDomDirection = m_pBL->getDominantDirection();

	if(iVisDirection == FRIBIDI_TYPE_LTR)
	{
		pNew->m_iX = m_iX + m_iWidth;
	}
	else
	{
		pNew->m_iX = m_iX;
		m_iX += pNew->m_iWidth;
	}

	pNew->m_iY = m_iY;

	return true;
}

void fp_TextRun::_fetchCharWidths(GR_Font* pFont, UT_GrowBufElement* pCharWidths)
{
	UT_ASSERT(pCharWidths);
	UT_ASSERT(pFont);

	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;
	UT_uint32 offset = m_iOffsetFirst;
	UT_uint32 len = m_iLen;
	bool bContinue = true;

	while (bContinue)
	{
		bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
		UT_ASSERT(lenSpan>0);

		m_pG->setFont(pFont);

		if (len <= lenSpan)
		{
			m_pG->measureString(pSpan, 0, len, pCharWidths + offset);

			bContinue = false;
		}
		else
		{
			m_pG->measureString(pSpan, 0, lenSpan, pCharWidths + offset);

			offset += lenSpan;
			len -= lenSpan;
		}
	}
}
void fp_TextRun::fetchCharWidths(fl_CharWidths * pgbCharWidths)
{
	if (m_iLen == 0)
	{
		return;
	}

	if(!s_bUseContextGlyphs)
	{
		// if we are using context glyphs we will do nothing, since this
		// will be done by recalcWidth()
		UT_GrowBufElement* pCharWidths = pgbCharWidths->getCharWidths()->getPointer(0);

#ifndef WITH_PANGO
		_fetchCharWidths(m_pScreenFont, pCharWidths);
		pCharWidths = pgbCharWidths->getCharWidthsLayoutUnits()->getPointer(0);
		_fetchCharWidths(m_pLayoutFont, pCharWidths);
		m_pG->setFont(m_pScreenFont);
#else
		_fetchCharWidths(m_pPangoFont, pCharWidths);
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
		iLength = m_iLen;
	}
	UT_ASSERT(iLength >= 0);

	UT_ASSERT((UT_uint32)iLength <= m_iLen);
	if((UT_uint32)iLength > m_iLen)
		iLength = (UT_sint32)m_iLen;

	if (iLength == 0)
		return 0;


	UT_GrowBuf * pgbCharWidths;
#ifndef WITH_PANGO
	switch(iWidthType)
	{
		case Width_type_display:
			pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
			break;

		case Width_type_layout_units:
			pgbCharWidths = m_pBL->getCharWidths()->getCharWidthsLayoutUnits();
			break;

		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return 0;
			break;
	}
#else
	pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
#endif

	UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

	UT_sint32 iWidth = 0;

	{
		_refreshDrawBuffer();

#ifndef WITH_PANGO
		if(iWidthType == Width_type_display)
			m_pG->setFont(m_pScreenFont);
		else
			m_pG->setFont(m_pLayoutFont);
#endif

		for (UT_sint32 i=0; i<iLength; i++)
		{
#ifndef WITH_PANGO
			// with PANGO this is taken care of by _refreshDrawBuffer()
			if(s_bUseContextGlyphs)
			{
				m_pG->measureString(m_pSpanBuff + i, 0, 1, (UT_GrowBufElement*)pCharWidths + m_iOffsetFirst + i);
			}
#endif
			iWidth += pCharWidths[i + m_iOffsetFirst];
		}
#ifndef WITH_PANGO
		m_pG->setFont(m_pScreenFont);
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
	if(m_bRecalcWidth)
	{
		m_bRecalcWidth = false;

		// we will call _refreshDrawBuffer() to ensure that the cache of the
		// visual characters is uptodate and then we will measure the chars
		// in the cache
		_refreshDrawBuffer();

#ifndef WITH_PANGO
		// with Pango the width gets recalcuated in _refreshDrawBuffer()
		UT_GrowBuf * pgbCharWidthsDisplay = m_pBL->getCharWidths()->getCharWidths();
		UT_GrowBuf *pgbCharWidthsLayout  = m_pBL->getCharWidths()->getCharWidthsLayoutUnits();

		UT_GrowBufElement* pCharWidthsDisplay = pgbCharWidthsDisplay->getPointer(0);
		UT_GrowBufElement* pCharWidthsLayout = pgbCharWidthsLayout->getPointer(0);
		xxx_UT_DEBUGMSG(("fp_TextRun::recalcWidth: pCharWidthsDisplay 0x%x, pCharWidthsLayout 0x%x\n",pCharWidthsDisplay, pCharWidthsLayout));

		m_iWidth = 0;
		m_iWidthLayoutUnits = 0;

		FriBidiCharType iVisDirection = getVisDirection();

		bool bReverse = (!s_bBidiOS && iVisDirection == FRIBIDI_TYPE_RTL)
			|| (s_bBidiOS && m_iDirOverride == FRIBIDI_TYPE_LTR && m_iDirection == FRIBIDI_TYPE_RTL);

		UT_sint32 j,k;

		for (UT_uint32 i = 0; i < m_iLen; i++)
		{
			// this is a bit tricky, since we want the resulting width array in
			// logical order, so if we reverse the draw buffer ourselves, we
			// have to address the draw buffer in reverse
			j = bReverse ? m_iLen - i - 1 : i;
			//k = (!bReverse && iVisDirection == FRIBIDI_TYPE_RTL) ? m_iLen - i - 1: i;
			k = i + m_iOffsetFirst;

			if(s_bUseContextGlyphs)
			{
				m_pG->setFont(m_pLayoutFont);
				m_pG->measureString(m_pSpanBuff + j, 0, 1, (UT_GrowBufElement*)pCharWidthsLayout + k);
				m_pG->setFont(m_pScreenFont);
				m_pG->measureString(m_pSpanBuff + j, 0, 1, (UT_GrowBufElement*)pCharWidthsDisplay + k);

			}
				m_iWidth += pCharWidthsDisplay[k];
				m_iWidthLayoutUnits += pCharWidthsLayout[k];

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
	UT_GrowBuf *pgbCharWidthsDisplay = m_pBL->getCharWidths()->getCharWidths();
	UT_GrowBufElement* pCharWidthsDisplay = pgbCharWidthsDisplay->getPointer(0);
#ifndef WITH_PANGO
	UT_sint32 iWidthLayoutUnits = 0;
	UT_GrowBuf *pgbCharWidthsLayout  = m_pBL->getCharWidths()->getCharWidthsLayoutUnits();
	UT_GrowBufElement* pCharWidthsLayout  = pgbCharWidthsLayout->getPointer(0);
#endif
	xxx_UT_DEBUGMSG(("fp_TextRun::_addupCharWidths: pCharWidthsDisplay 0x%x, pCharWidthsLayout 0x%x\n",pCharWidthsDisplay, pCharWidthsLayout));


	for (UT_uint32 i = m_iOffsetFirst; i < m_iLen + m_iOffsetFirst; i++)
	{
		iWidth += pCharWidthsDisplay[i];
#ifndef WITH_PANGO
		iWidthLayoutUnits += pCharWidthsLayout[i];
#endif
	}

	if(iWidth != m_iWidth)
	{
		m_iWidth = iWidth;
#ifndef WITH_PANGO
		m_iWidthLayoutUnits = iWidthLayoutUnits;
#endif
		return true;
	}

	return false;
}

void fp_TextRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

	if(!m_pLine->isEmpty() && m_pLine->getLastVisRun() == this)   //#TF must be last visual run
	{
		// Last run on the line so clear to end.

		m_pLine->clearScreenFromRunToEnd(this);
	}
	else
	{
#ifndef WITH_PANGO
		m_pG->setFont(m_pScreenFont);
#else
		m_pG->setFont(m_pPangoFont);
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
		UT_RGBColor clrNormalBackground(m_colorPG.m_red, m_colorPG.m_grn, m_colorPG.m_blu);
		if (m_pField)
		{
		  UT_setColor (clrNormalBackground, 220, 220, 220);
		}
		m_pG->setColor(clrNormalBackground);

		UT_sint32 xoff = 0, yoff = 0;
		m_pLine->getScreenOffsets(this, xoff, yoff);
		//
		// Handle case where character extend behind the left side
		// like italic Times New Roman f
		//
		fp_Line * thisLine = getLine();
		fp_Run * pPrev = getPrev();
		UT_sint32 leftClear = 0;
		if(thisLine != NULL)
		{
			while(pPrev != NULL && pPrev->getLine() == thisLine && pPrev->getLength()== 0)
			{
				pPrev = pPrev->getPrev();
			}
			leftClear = getDescent();
			bool bthis = (pPrev != NULL) && (pPrev->getLine() ==thisLine);
			if(bthis && pPrev->getType() == FPRUN_TEXT)
				leftClear = 0;
			if(bthis  && pPrev->getType() == FPRUN_FIELD)
				leftClear = 0;
			if(bthis && pPrev->getType() == FPRUN_IMAGE)
				leftClear = 0;
		}
		m_pG->fillRect(clrNormalBackground,xoff-leftClear , yoff, m_iWidth+leftClear, m_pLine->getHeight());
	}

}

void fp_TextRun::_draw(dg_DrawArgs* pDA)
{
	/*
	  Upon entry to this function, pDA->yoff is the BASELINE of this run, NOT
	  the top.
	*/
#ifndef NDEBUG
	FV_View* ppView = m_pBL->getDocLayout()->getView();
	if(ppView) UT_ASSERT(ppView && ppView->isCursorOn()==false);
#endif

	_refreshDrawBuffer();
	xxx_UT_DEBUGMSG(("fp_TextRun::_draw (0x%x): m_iVisDirection %d, m_iDirection %d\n", this, m_iVisDirection, m_iDirection));

	UT_sint32 yTopOfRun = pDA->yoff - m_iAscent-1; // Hack to remove
	UT_sint32 yTopOfSel = yTopOfRun+1; // final character dirt

	/*
	  TODO We should add more possibilities for text placement here.
	  It shouldn't be too hard.  Just adjust the math a little.
	  See bug 1297
	*/

	if (m_fPosition == TEXT_POSITION_SUPERSCRIPT)
	{
		yTopOfRun -= m_iAscent * 1/2;
	}
	else if (m_fPosition == TEXT_POSITION_SUBSCRIPT)
	{
		yTopOfRun += m_iDescent /* * 3/2 */;
	}

	UT_ASSERT(pDA->pG == m_pG);
	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();

	UT_uint32 iBase = m_pBL->getPosition();

	/*
	  TODO this should not be hard-coded.  We should calculate an
	  appropriate selection background color based on the color
	  of the foreground text, probably.
	*/
	UT_RGBColor clrNormalBackground(m_colorHL.m_red, m_colorHL.m_grn, m_colorHL.m_blu);
	UT_RGBColor clrSelBackground(192, 192, 192);

	if (m_pField)
	{
		UT_setColor(clrNormalBackground,220, 220, 220);
		UT_setColor(clrSelBackground,112, 112, 112);
	}

	// test the nearness of our selection colour to text, highlight and page
	// colours
	if(clrSelBackground %= m_colorPG)
		clrSelBackground += 20;
	if(clrSelBackground %= m_colorHL)
		clrSelBackground += 20;
	if(clrSelBackground %= m_colorFG)
		clrSelBackground += 20;


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
	UT_RGBColor clrWhite(255,255,255);
	bool bDrawBckg = (m_pG->queryProperties(GR_Graphics::DGP_SCREEN)
					 || clrNormalBackground != clrWhite);

	if(bDrawBckg)
	{
		m_pG->fillRect( clrNormalBackground,
					pDA->xoff,
					yTopOfSel + m_iAscent - m_pLine->getAscent(),
					m_iWidth,
					m_pLine->getHeight());
	}

	UT_uint32 iRunBase = iBase + m_iOffsetFirst;

	FV_View* pView = m_pBL->getDocLayout()->getView();
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
				if (iSel2 >= (iRunBase + m_iLen))
				{
					// the whole run is selected
					_fillRect(clrSelBackground, pDA->xoff, yTopOfSel, m_iOffsetFirst, m_iLen, pgbCharWidths);
				}
				else
				{
					// the first part is selected, the second part is not
					_fillRect(clrSelBackground, pDA->xoff, yTopOfSel, m_iOffsetFirst, iSel2 - iRunBase, pgbCharWidths);
				}
			}
		}
		else if (iSel1 < (iRunBase + m_iLen))
		{
			if (iSel2 >= (iRunBase + m_iLen))
			{
				// the second part is selected
				_fillRect(clrSelBackground, pDA->xoff, yTopOfSel, iSel1 - iBase, m_iLen - (iSel1 - iRunBase), pgbCharWidths);
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
	m_pG->getColor(curColor);
#endif

	if(bDrawBckg && m_pG->queryProperties(GR_Graphics::DGP_OPAQUEOVERLAY))
	{
		fp_Run * pNext = getNextVisual();
		fp_Run * pPrev = getPrevVisual();

		if(pNext && pNext->getType() == FPRUN_TEXT)
		{
			fp_TextRun * pT = static_cast<fp_TextRun*>(pNext);
			UT_sint32 ytemp = pDA->yoff+(pT->getY()-getY())-pT->m_iAscent-1;
 			if (pT->m_fPosition == TEXT_POSITION_SUPERSCRIPT)
 			{
 				ytemp -= pT->m_iAscent * 1/2;
 			}
 			else if (pT->m_fPosition == TEXT_POSITION_SUBSCRIPT)
 			{
 				ytemp += pT->m_iDescent /* * 3/2 */;
 			}

              		if(pT->m_bIsOverhanging)
		                pT->_drawFirstChar(pDA->xoff + m_iWidth,ytemp);
		}

		if(pPrev && pPrev->getType() == FPRUN_TEXT)
		{
			fp_TextRun * pT = static_cast<fp_TextRun*>(pPrev);
			UT_sint32 ytemp = pDA->yoff+(pT->getY()-getY())-pT->m_iAscent-1;
 			if (pT->m_fPosition == TEXT_POSITION_SUPERSCRIPT)
 			{
 				ytemp -= pT->m_iAscent * 1/2;
 			}
 			else if (pT->m_fPosition == TEXT_POSITION_SUBSCRIPT)
 			{
 				ytemp += pT->m_iDescent /* * 3/2 */;
 			}

 			if(pT->m_bIsOverhanging)
			        pT->_drawLastChar(pDA->xoff,ytemp, pgbCharWidths);

		}
	}

	// now draw the whole string
#ifndef WITH_PANGO
	m_pG->setFont(m_pScreenFont);
#else
	m_pG->setFont(m_pPangoFont);
#endif

	m_pG->setColor(getFGColor()); // set colour just in case we drew a first/last char with a diff colour

	// since we have the visual string in the draw buffer, we just call m_pGr->drawChars()
	m_pG->drawChars(m_pSpanBuff, 0, m_iLen, pDA->xoff, yTopOfRun);

	drawDecors(pDA->xoff, yTopOfRun);

	if(pView->getShowPara())
	{
		_drawInvisibles(pDA->xoff, yTopOfRun);
	}

	// TODO: draw this underneath (ie, before) the text and decorations
	m_bSquiggled = false;
	m_pBL->findSquigglesForRun(this);
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
	FV_View* ppView = m_pBL->getDocLayout()->getView();
	if(ppView) UT_ASSERT(ppView && ppView->isCursorOn()==false);
#endif
	// we also need to support this in printing
	// NO! we do not and must not -- this is only used to draw selections
	// and field background and we do not want to see these printed
	if (m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		UT_Rect r;

		_getPartRect(&r, xoff, yoff, iPos1, iLen, pgbCharWidths);
		r.height = m_pLine->getHeight();
		r.top = r.top + m_iAscent - m_pLine->getAscent();
		m_pG->fillRect(clr, r.left, r.top, r.width, r.height);
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
	pRect->height = m_iHeight;
	pRect->width = 0;

	// that's enough for zero-length run
	if (m_iLen == 0)
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
	if (iStart > m_iOffsetFirst)
	{
		for (i=m_iOffsetFirst; i<iStart; i++)
		{
			pRect->left += pCharWidths[i];
		}
	}

	if(getVisDirection() == FRIBIDI_TYPE_LTR)
	{
		pRect->left += xoff; //if this is ltr then adding xoff is all that is needed
	}

	for (i=iStart; i<(iStart + iLen); i++)
	{
		pRect->width += pCharWidths[i];
	}

	//in case of rtl we are now in the position to calculate the position of the the left corner
	if(getVisDirection() == FRIBIDI_TYPE_RTL) pRect->left = xoff + m_iWidth - pRect->left - pRect->width;
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

	if(m_iOffsetFirst > 1)
	{
		if(m_pBL->getSpanPtr(offset - 2, &pPrev, &lenPrev))
		{
			prev[1] = *pPrev;
			if(lenPrev > 1)
				prev[0] = *(pPrev+1);
			else if(m_pBL->getSpanPtr(offset - 1, &pPrev, &lenPrev))
				prev[0] = *(pPrev+1);
		}
		else if(m_pBL->getSpanPtr(offset - 1, &pPrev, &lenPrev))
		{
			prev[0] = *pPrev;
		}
	}
	else if(m_iOffsetFirst > 0)
	{
		if(m_pBL->getSpanPtr(offset - 1, &pPrev, &lenPrev))
			prev[0] = *pPrev;
	}

	lenPrev = 2;

	xxx_UT_DEBUGMSG(("fp_TextRun::_getContext: prev[0] %d, prev[1] %d\n"
				 "		 m_iOffsetFirst %d, offset %d\n",
				 prev[0],prev[1],m_iOffsetFirst,offset));

	// how many characters at most can we retrieve?
	UT_sint32 iStop = (UT_sint32) CONTEXT_BUFF_SIZE < (UT_sint32)(lenSpan - len) ? CONTEXT_BUFF_SIZE : lenSpan - len;
	UT_sint32 i;
	// first, getting anything that might be in the span buffer
	for(i=0; i< iStop;i++)
		after[i] = pSpan[len+i];

	// for anything that we miss, we need to get it the hard way
	// as it is located in different spans

	while(i < CONTEXT_BUFF_SIZE && m_pBL->getSpanPtr(offset + len + i, &pNext, &lenAfter))
	{
		for(UT_uint32 j = 0; j < lenAfter && (UT_uint32)i < CONTEXT_BUFF_SIZE; j++,i++)
			after[i] = pNext[j];
	}

	// now we have our trailing chars, so we null-terminate the array
	after[i] = 0;
}


void fp_TextRun::_refreshDrawBuffer()
{
	if(m_bRefreshDrawBuffer)
	{
		m_bRefreshDrawBuffer = false;
		FriBidiCharType iVisDir = getVisDirection();

		const UT_UCSChar* pSpan = NULL;
		UT_uint32 lenSpan = 0;
		UT_uint32 offset = 0;
		UT_uint32 len = m_iLen;
		bool bContinue = true;

		UT_contextGlyph cg;

		if(m_iLen > m_iSpanBuffSize) //the buffer too small, reallocate
		{
			delete[] m_pSpanBuff;
			m_pSpanBuff = new UT_UCSChar[m_iLen + 1];
			m_iSpanBuffSize = m_iLen;
			UT_ASSERT(m_pSpanBuff);
		}

		for(;;) // retrive the span and do any necessary processing
		{
			bContinue = m_pBL->getSpanPtr(offset + m_iOffsetFirst, &pSpan, &lenSpan);

			//this sometimes happens with fields ...
			if(!bContinue)
				break;

			UT_uint32 iTrueLen = (lenSpan > len) ? len : lenSpan;

			if(m_iDirection != FRIBIDI_TYPE_ON || iVisDir != FRIBIDI_TYPE_RTL)
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
					_getContext(pSpan,lenSpan,len,offset+m_iOffsetFirst,&prev[0],&next[0]);
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
		  || (s_bBidiOS && m_iDirOverride == FRIBIDI_TYPE_LTR && m_iDirection == FRIBIDI_TYPE_RTL))
			UT_UCS4_strnrev(m_pSpanBuff, m_iLen);
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
		UT_uint32 len = m_iLen;
		bool bContinue = true;

		UT_UCSChar * pWholeSpan = new UT_UCSChar [m_iLen];
		UT_ASSERT(pWholeSpan);

		UT_UCSChar * pWholeSpanPtr = pWholeSpan;

		for(;;) // retrive the span and do any necessary processing
		{
			bContinue = m_pBL->getSpanPtr(offset + m_iOffsetFirst, &pSpan, &lenSpan);

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
		UT_UTF8String wholeStringUtf8 (pWholeString, m_iLen);

		// let Panog to analyse the string
		GList * pItems = pango_itemize(m_pG->getPangoContext(),
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

		UT_GrowBuf * pgbCharWidthsDisplay = m_pBL->getCharWidths()->getCharWidths();
		//UT_GrowBuf * pgbCharWidthsLayout  = m_pBL->getCharWidths()->getCharWidthsLayoutUnits();

		UT_GrowBufElement * pCharWidthsDisplay = pgbCharWidthsDisplay->getPointer(0) + m_iOffsetFirst;
		//UT_GrowBufElement * pCharWidthsLayout = pgbCharWidthsLayout->getPointer(0) + m_iOffsetFirst;

		m_iWidth = 0;
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
		m_iWidth = 0;

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
			pango_glyph_string_extents(pGString, m_pPangoFont, &ink_rect,NULL);
			m_iWidths += ink_rect.width;
#endif
			for(UT_sint32 j = 0; j < pItem->num_chars; j++)
				m_iWidth += pCharWidthsDisplayPtr[j];

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
	if(!m_iLen)
		return;

	// have to sent font (and colour!), since we were called from a run using different font
	m_pG->setFont(m_pScreenFont);
	m_pG->setColor(getFGColor());

	FriBidiCharType iVisDirection = getVisDirection();

	if(!s_bBidiOS)
	{
		// m_pSpanBuff is in visual order, so we just draw the last char
		UT_uint32 iPos = iVisDirection == FRIBIDI_TYPE_LTR ? m_iLen - 1 : 0;
		m_pG->drawChars(m_pSpanBuff, m_iLen - 1, 1, xoff - *(pgbCharWidths->getPointer(m_iOffsetFirst + iPos)), yoff);
	}
	else
	{
		UT_uint32 iPos = iVisDirection == FRIBIDI_TYPE_LTR ? m_iLen - 1 : 0;
		m_pG->drawChars(m_pSpanBuff, iPos, 1, xoff - *(pgbCharWidths->getPointer(m_iOffsetFirst + iPos)), yoff);
	}
}

void fp_TextRun::_drawFirstChar(UT_sint32 xoff, UT_sint32 yoff)
{
	if(!m_iLen)
		return;

	// have to sent font (and colour!), since we were called from a run using different font
	m_pG->setFont(m_pScreenFont);
	m_pG->setColor(getFGColor());

	if(!s_bBidiOS)
	{
		// m_pSpanBuff is in visual order, so we just draw the last char
		m_pG->drawChars(m_pSpanBuff, 0, 1, xoff, yoff);
	}
	else
	{
		UT_uint32 iPos = getVisDirection() == FRIBIDI_TYPE_RTL ? m_iLen - 1 : 0;
		m_pG->drawChars(m_pSpanBuff, iPos, 1, xoff, yoff);
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
	if (m_iLen == 0)
	{
		return;
	}

	UT_ASSERT(iStart >= m_iOffsetFirst);
	UT_ASSERT(iStart + iLen <= m_iOffsetFirst + m_iLen);

	UT_uint32 iLeftWidth = 0;
	const UT_GrowBufElement * pCharWidths = pgbCharWidths->getPointer(0);
	UT_ASSERT(pCharWidths);
	if (!pCharWidths) {
		UT_DEBUGMSG(("TODO: Investigate why pCharWidths is NULL?"));
		return;
	}


	UT_uint32 i;
	for (i=m_iOffsetFirst; i<iStart; i++)
	{
		iLeftWidth += pCharWidths[i]; //NB! in rtl text this is visually right width
	}

	// in the bidi build we keep a cache of the visual span of this run
	// this speeds things up
	// (the cache is refreshed by _refreshDrawBuff, it is the responsibility
	// of caller of _drawPart to ensure that the chache is uptodate)
	FriBidiCharType iVisDirection = getVisDirection();
	bool bReverse = (!s_bBidiOS && iVisDirection == FRIBIDI_TYPE_RTL)
		  || (s_bBidiOS && m_iDirOverride == FRIBIDI_TYPE_LTR && m_iDirection == FRIBIDI_TYPE_RTL);
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
		m_pG->drawChars(m_pSpanBuff + (m_iLen + m_iOffsetFirst - iStart) - iLen, 0, iLen, xoff + m_iWidth - iLeftWidth, yoff);

		// if the buffer is not reversed because this is simple LTR run, then
		// iLeftWidth is precisely that
	else if(iVisDirection == FRIBIDI_TYPE_LTR)
		m_pG->drawChars(m_pSpanBuff + iStart - m_iOffsetFirst, 0, iLen, xoff + iLeftWidth, yoff);
	else


		// if the buffer is not reversed because the OS will reverse it, then we
		// draw from it as if it was LTR buffer, but iLeftWidth is right width
		m_pG->drawChars(m_pSpanBuff + iStart - m_iOffsetFirst, 0, iLen, xoff + m_iWidth - iLeftWidth, yoff);
}


void fp_TextRun::_drawInvisibleSpaces(UT_sint32 xoff, UT_sint32 yoff)
{
	UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
	UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);
	const UT_UCSChar* pSpan = NULL;
	UT_uint32 lenSpan = 0;
	UT_uint32 len = m_iLen;
	UT_sint32 iWidth = 0;
	UT_sint32 cur_linewidth = 1+ (UT_MAX(10,m_iAscent)-10)/8;
	UT_sint32 iRectSize = cur_linewidth * 3 / 2;
	bool bContinue = true;
	UT_uint32 offset = m_iOffsetFirst;

	UT_RGBColor clrShowPara(127,127,127);

#ifndef NDEBUG
	FV_View* ppView = m_pBL->getDocLayout()->getView();
	if(ppView) UT_ASSERT(ppView && ppView->isCursorOn()==false);
#endif
	//UT_DEBUGMSG(("---------\n"));
	if(findCharacter(0, UCS_SPACE) > 0){
		while(bContinue){
			bContinue = m_pBL->getSpanPtr(offset,&pSpan,&lenSpan);
			//if(!bContinue)
			//	break; //no span found
#ifdef DEBUG
			if(lenSpan <= 0)
			{
				const UT_UCSChar* mypSpan = NULL;
				unsigned char buff[500];
				UT_uint32 mylenSpan = 0;
				m_pBL->getSpanPtr(m_iOffsetFirst,&mypSpan,&mylenSpan);
				for(UT_uint32 i = 0; i < mylenSpan; i++)
					buff[i] = (unsigned char) mypSpan[i];

				UT_DEBUGMSG(("fp_TextRun::_drawInvisibleSpaces (0x%x):\n"
							 "		 m_iOffsetFirst %d, m_iLen %d\n"
							 "		 mylenSpan %d, span: %s\n"
							 "		 lenSpan %d, len %d, offset %d\n",
							 this,m_iOffsetFirst,m_iLen,mylenSpan,buff,lenSpan, len, offset));

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
				   m_pG->fillRect(clrShowPara,xoff + iWidth + (pCharWidths[i + offset] - iRectSize) / 2,iy,iRectSize,iRectSize);
			   }
			   iWidth += pCharWidths[i + offset];
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
	FV_View* ppView = m_pBL->getDocLayout()->getView();
	if(ppView) UT_ASSERT(ppView && ppView->isCursorOn()==false);
#endif

		if (!(m_pG->queryProperties(GR_Graphics::DGP_SCREEN))){
				return;
		}
		_drawInvisibleSpaces(xoff,yoff);
}

void fp_TextRun::_drawSquiggle(UT_sint32 top, UT_sint32 left, UT_sint32 right)
{
	if (!(m_pG->queryProperties(GR_Graphics::DGP_SCREEN)))
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

	m_pG->polyLine(points, nPoints);

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
	UT_sint32 iAscent = m_pLine->getAscent();
	UT_sint32 iDescent = m_pLine->getDescent();

	// we'd prefer squiggle to leave one pixel below the baseline,
	// but we need to force all three pixels inside the descent
	// we cannot afford the 1pixel gap, it leave dirt on screen -- Tomas
	UT_sint32 iGap = (iDescent > 3) ?/*1*/0 : (iDescent - 3);

	UT_RGBColor clrSquiggle(255, 0, 0);
	m_pG->setColor(clrSquiggle);

	m_pLine->getScreenOffsets(this, xoff, yoff);

	UT_Rect r;
	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
	_getPartRect( &r, xoff, yoff, iOffset, iLen, pgbCharWidths);

	_drawSquiggle(r.top + iAscent + iGap, r.left, r.left + r.width);
}

UT_sint32 fp_TextRun::findCharacter(UT_uint32 startPosition, UT_UCSChar Character) const
{
	// NOTE: startPosition is run-relative
	// NOTE: return value is block-relative (don't ask me why)
	const UT_UCSChar* pSpan = NULL;
	UT_uint32 lenSpan = 0;
	UT_uint32 offset = m_iOffsetFirst + startPosition;
	UT_uint32 len = m_iLen - startPosition;
	bool bContinue = true;

	if ((m_iLen > 0) && (startPosition < m_iLen))
	{
		UT_uint32 i;

		while (bContinue)
		{
			bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
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

				UT_ASSERT(offset >= m_iOffsetFirst);
				UT_ASSERT(offset + len <= m_iOffsetFirst + m_iLen);
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

	UT_ASSERT(run_offset < m_iLen);

	if (m_iLen > 0)
	{
		if (m_pBL->getSpanPtr(run_offset + m_iOffsetFirst, &pSpan, &lenSpan))
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

	if (getCharacter(m_iLen - 1, c))
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
	if(m_iLen > 0)
	{
		const UT_UCSChar* pSpan = NULL;
		UT_uint32 lenSpan = 0;
		UT_uint32 offset = m_iOffsetFirst;
		UT_uint32 len = m_iLen;
		bool bContinue = true;

		UT_uint32 i;

		while (bContinue)
		{
			bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
			//if(!bContinue)
			//	break; //no span found
			if(lenSpan <= 0)
			{
				fp_Line * pLine = (fp_Line *) m_pBL->getFirstContainer();
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

				UT_ASSERT(offset >= m_iOffsetFirst);
				UT_ASSERT(offset + len <= m_iOffsetFirst + m_iLen);
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
	UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
	UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

	UT_sint32 iTrailingDistance = 0;

	if(m_iLen > 0)
	{
		UT_UCSChar c;

		UT_sint32 i;
		for (i = m_iLen - 1; i >= 0; i--)
		{
			if(getCharacter(i, c) && (UCS_SPACE == c))
			{
				iTrailingDistance += pCharWidths[i + m_iOffsetFirst];
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
	UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidthsLayoutUnits();
	UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

	UT_sint32 iTrailingDistance = 0;

	if(m_iLen > 0)
	{
		UT_UCSChar c;

		UT_sint32 i;
		for (i = m_iLen - 1; i >= 0; i--)
		{
			if(getCharacter(i, c) && (UCS_SPACE == c))
			{
				iTrailingDistance += pCharWidths[i + m_iOffsetFirst];
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
	if(m_iSpaceWidthBeforeJustification != JUSTIFICATION_NOT_USED
    && m_iSpaceWidthBeforeJustification != JUSTIFICATION_FAKE)
	{
		UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
		UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

		UT_sint32 i = findCharacter(0, UCS_SPACE);

		while (i >= 0)
		{
			if(pCharWidths[i] != m_iSpaceWidthBeforeJustification)
			{
				// set width of spaces back to normal.

				m_iWidth -= pCharWidths[i] - m_iSpaceWidthBeforeJustification;
				pCharWidths[i] = m_iSpaceWidthBeforeJustification;
			}

			// keep looping
			i = findCharacter(i+1-m_iOffsetFirst, UCS_SPACE);
		}
		m_bRecalcWidth = true;
	}

	m_iSpaceWidthBeforeJustification = JUSTIFICATION_NOT_USED;
}

void fp_TextRun::distributeJustificationAmongstSpaces(UT_sint32 iAmount, UT_uint32 iSpacesInRun)
{
	UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
	UT_GrowBufElement* pCharWidths = pgbCharWidths->getPointer(0);

	if(!iAmount || !iSpacesInRun)
	{
		// we still want to set the m_iSpaceWidthBeforeJustification member, to prevent such runs
		// from being merged; we will use JUSTIFICATION_FAKE  to indicated that no change was made,
		// but this run is still considered justified
		m_iSpaceWidthBeforeJustification = JUSTIFICATION_FAKE;
		return;
	}

	if(iSpacesInRun && m_iLen > 0)
	{
		m_iWidth += iAmount;

		UT_sint32 i = findCharacter(0, UCS_SPACE);

		while ((i >= 0) && (iSpacesInRun))
		{
			// remember how wide spaces in this run "really" were
			if(m_iSpaceWidthBeforeJustification == JUSTIFICATION_NOT_USED)
				m_iSpaceWidthBeforeJustification = pCharWidths[i];

			UT_sint32 iThisAmount = iAmount / iSpacesInRun;

			pCharWidths[i] += iThisAmount;

			iAmount -= iThisAmount;

			iSpacesInRun--;

			// keep looping
			i = findCharacter(i+1-m_iOffsetFirst, UCS_SPACE);
		}
		m_bRecalcWidth = true;
	}

	UT_ASSERT(iAmount == 0);
}

UT_uint32 fp_TextRun::countTrailingSpaces(void) const
{
	UT_uint32 iCount = 0;

	if(m_iLen > 0)
	{
		UT_UCSChar c;

		UT_sint32 i;
		for (i = m_iLen - 1; i >= 0; i--)
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

	if(m_iLen > 0)
	{
		UT_sint32 i = findCharacter(0, UCS_SPACE);
		iOldI = m_iOffsetFirst - 1;

		while (i >= 0)
		{
			if(iOldI < i-1) // i.e., something between the two spaces
				bNonBlank = true;

			iCount++;
			iOldI = i;

			// keep looping
			i = findCharacter(i+1-m_iOffsetFirst, UCS_SPACE);
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
	if (m_pField)
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
	UT_uint32 offset = m_iOffsetFirst;
	UT_uint32 len = m_iLen;
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
			bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
			UT_ASSERT(lenSpan>0);

			if (len <= lenSpan) 	//copy the entire len to pStr and return 0
			{
				UT_ASSERT(len>0);
						UT_UCS4_strncpy(pStrPos, pSpan, len);
						pStr[len] = 0;						  //make sure the string is 00-terminated
						iMax = m_iLen;
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
	if( !m_iLen
	|| (   dir == FRIBIDI_TYPE_UNSET
		&& m_iDirection != FRIBIDI_TYPE_UNSET
		&& dirOverride == m_iDirOverride
		)
	  )
		return; //ignore 0-length runs, let them be treated on basis of the app defaults

	FriBidiCharType prevDir = m_iDirOverride == FRIBIDI_TYPE_UNSET ? m_iDirection : m_iDirOverride;
	if(dir == FRIBIDI_TYPE_UNSET)
	{
		// only do this once
		if(m_iDirection == FRIBIDI_TYPE_UNSET)
		{
			UT_UCSChar firstChar;
			getCharacter(0, firstChar);

			m_iDirection = fribidi_get_type((FriBidiChar)firstChar);
		}
	}
	else //meaningfull value received
	{
		m_iDirection = dir;
	}

	xxx_UT_DEBUGMSG(("fp_TextRun (0x%x)::setDirection: %d (passed %d, override %d, prev. %d)\n", this, m_iDirection, dir, m_iDirOverride, prevDir));

	FriBidiCharType iOldOverride = m_iDirOverride;
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

	FriBidiCharType curDir = m_iDirOverride == FRIBIDI_TYPE_UNSET ? m_iDirection : m_iDirOverride;

	UT_ASSERT(curDir != FRIBIDI_TYPE_UNSET);

	if(curDir != prevDir)
	{
		clearScreen();
		m_bRefreshDrawBuffer = true;

		if(m_pLine)
		{
			m_pLine->changeDirectionUsed(prevDir,curDir,true);
			//m_pLine->setNeedsRedraw();
		}
	}

	//UT_DEBUGMSG(("TextRun::setDirection: direction=%d\n", m_iDirection));
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

	UT_uint32 offset = m_pBL->getPosition() + m_iOffsetFirst;
	getBlock()->getDocument()->changeSpanFmt(PTC_AddFmt,offset,offset + m_iLen,NULL,prop);

	UT_DEBUGMSG(("fp_TextRun::setDirOverride: offset=%d, len=%d, dir=\"%s\"\n", offset,m_iLen,prop[1]));
}

#ifdef SMART_RUN_MERGING
void fp_TextRun::breakNeighborsAtDirBoundaries()
{
	FriBidiCharType iPrevType, iType = FRIBIDI_TYPE_UNSET;
	FriBidiCharType iDirection = getDirection();

	fp_TextRun *pNext = NULL, *pPrev = NULL, *pOtherHalf;
	PT_BlockOffset curOffset;
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
		m_pBL->getSpanPtr((UT_uint32) curOffset, &pSpan, &lenSpan);
		iPrevType = fribidi_get_type((FriBidiChar)pSpan[0]);

		while(curOffset > pPrev->getBlockOffset() && !FRIBIDI_IS_STRONG(iType))
		{
			curOffset--;
			m_pBL->getSpanPtr((UT_uint32) curOffset, &pSpan, &lenSpan);
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
		m_pBL->getSpanPtr((UT_uint32) curOffset, &pSpan, &lenSpan);
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
				m_pBL->getSpanPtr((UT_uint32) curOffset, &pSpan, &lenSpan);
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
	UT_uint32 iLen = m_iLen;  // need to remember this, since m_iLen will change if we split
	PT_BlockOffset currOffset = m_iOffsetFirst;
	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan = 0;
	UT_uint32 spanOffset = 0;
	FriBidiCharType iPrevType, iType = FRIBIDI_TYPE_UNSET;
	m_pBL->getSpanPtr((UT_uint32) currOffset, &pSpan, &lenSpan);
	iPrevType = iType = fribidi_get_type((FriBidiChar)pSpan[spanOffset]);

	while((currOffset + spanOffset) < (m_iOffsetFirst + iLen))
	{
		while(iPrevType == iType && ((currOffset + spanOffset) < (m_iOffsetFirst + iLen - 1)))
		{
			spanOffset++;
			if(spanOffset >= lenSpan)
			{
				currOffset += spanOffset;
				spanOffset = 0;
				m_pBL->getSpanPtr((UT_uint32) currOffset, &pSpan, &lenSpan);
			}

			iType = fribidi_get_type((FriBidiChar)pSpan[spanOffset]);
		}

		// if we reached the end of the origianl run, then stop
		if((currOffset + spanOffset) >= (m_iOffsetFirst + iLen - 1))
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
