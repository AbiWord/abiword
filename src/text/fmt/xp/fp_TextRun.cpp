/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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
#include "gr_Graphics.h"
#include "pd_Document.h"
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
#ifdef BIDI_ENABLED
//#define CAPS_TEST
#include "AbiFriBiDi.h"
#include "ap_Prefs.h"
#endif

/*****************************************************************/

#ifdef BIDI_ENABLED
//inicialise the static members of the class
UT_UCSChar * fp_TextRun::s_pSpanBuff = 0;
UT_uint32    fp_TextRun::s_iClassInstanceCount = 0;
UT_uint32    fp_TextRun::s_iSpanBuffSize = 0;
#endif


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
	m_pField = NULL;
	m_pLanguage = NULL;
#ifdef BIDI_ENABLED
	m_iDirection = 2; //we will use this as an indication that the direction property has not been yet set
			  //normal values are -1,0,1 (neutral, ltr, rtl)
	m_iDirOverride = -1; //no override by default
#endif
	if (bLookupProperties)
	{
		lookupProperties();
	}

#ifdef BIDI_ENABLED
//in order to be able to print rtl runs on GUI that does not support this,
//we will need to make a copy of the run and strrev it; this is a static
//buffer shared by all the instances;
	if(!s_pSpanBuff)
	{
		s_pSpanBuff = new UT_UCSChar[MAX_SPAN_LEN];
		s_iSpanBuffSize = MAX_SPAN_LEN;
		UT_ASSERT(s_pSpanBuff);
	}
	s_iClassInstanceCount++;
#endif
}

fp_TextRun::~fp_TextRun()
{
#ifdef BIDI_ENABLED
	--s_iClassInstanceCount;
	if(!s_iClassInstanceCount)
	{
		delete[] s_pSpanBuff;
		s_pSpanBuff = (UT_UCSChar * ) 0;
	}
#endif
}

bool fp_TextRun::hasLayoutProperties(void) const
{
	return true;
}


void fp_TextRun::lookupProperties(void)
{
	clearScreen();
	
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
	m_pBL->getSpanAttrProp(m_iOffsetFirst,false,&pSpanAP);
	m_pBL->getAttrProp(&pBlockAP);
	static_cast<fl_Layout *>(m_pBL)->getField(m_iOffsetFirst,m_pField);
	// look for fonts in this DocLayout's font cache
	FL_DocLayout * pLayout = m_pBL->getDocLayout();

	PD_Document * pDoc = m_pBL->getDocument();

	const PP_PropertyTypeColor *p_color = (const PP_PropertyTypeColor *)PP_evalPropertyType("color",pSpanAP,pBlockAP,pSectionAP, Property_type_color, pDoc, true);
	UT_ASSERT(p_color);
	m_colorFG = p_color->getColor();


	getHighlightColor();
	getPageColor();

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

	GR_Font * pFont;

	pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);
	if (m_pScreenFont != pFont)
	  {
	    m_pScreenFont = pFont;
	    m_iAscent = m_pG->getFontAscent(m_pScreenFont);	
	    m_iDescent = m_pG->getFontDescent(m_pScreenFont);
	    m_iHeight = m_pG->getFontHeight(m_pScreenFont);
	  }

	pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_LAYOUT_RESOLUTION);
	if (m_pLayoutFont != pFont)
	  {
	    m_pLayoutFont = pFont;
	    m_iAscentLayoutUnits = m_pG->getFontAscent(m_pLayoutFont);	
	    UT_ASSERT(m_iAscentLayoutUnits);
	    m_iDescentLayoutUnits = m_pG->getFontDescent(m_pLayoutFont);
	    m_iHeightLayoutUnits = m_pG->getFontHeight(m_pLayoutFont);
	  }
#if 1
	m_pG->setFont(m_pScreenFont);
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

#ifdef BIDI_ENABLED
	const XML_Char * pszDirection = PP_evalProperty("dir",pSpanAP,pBlockAP,pSectionAP, pDoc, true);
	//UT_DEBUGMSG(( "pszDirection = %s\n", pszDirection ));
	//UT_ASSERT((m_pLine));
	UT_sint32 prevDir = m_iDirection;
	if(!UT_stricmp(pszDirection, "rtl"))
	{
		m_iDirection = 1;
		//m_pLine->orDirectionsUsed(FP_LINE_DIRECTION_USED_RTL);
	}
	else if(!UT_stricmp(pszDirection, "ltr"))
	{
		m_iDirection = 0;
		//m_pLine->orDirectionsUsed(FP_LINE_DIRECTION_USED_LTR);
	}
	else
	{
		m_iDirection = -1; //whitespace
	}

	//UT_DEBUGMSG(("TextRun: lookupProperties, m_iDirection=%d (%s)\n", m_iDirection, pszDirection));
    pszDirection = PP_evalProperty("dir-override",pSpanAP,pBlockAP,pSectionAP, pDoc, true);
    if(!UT_stricmp(pszDirection, "rtl"))
    	m_iDirOverride = 1;
    else if(!UT_stricmp(pszDirection, "ltr"))
    	m_iDirOverride = 0;
    else
    	m_iDirOverride = -1;

    if(m_iDirOverride != -1)
    	m_iDirection = m_iDirOverride;
    	
   	if(m_iDirection != prevDir && m_pLine)
   	{
    	m_pLine->removeDirectionUsed(prevDir);
    	m_pLine->addDirectionUsed(m_iDirection);
    	m_pLine->setMapOfRunsDirty();
    }
	    //UT_DEBUGMSG(("TextRun::lookupProperties: m_iDirection=%d, m_iDirOverride=%d\n", m_iDirection, m_iDirOverride));
#endif
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
	UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

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
	if (x <= 0)
	{
		pos = m_pBL->getPosition() + m_iOffsetFirst;
		// don't set bBOL to false here
		bEOL = false;
		return;
	}

	if (x >= m_iWidth)
	{
		pos = m_pBL->getPosition() + m_iOffsetFirst + m_iLen;
		// Setting bEOL fixes bug 1149. But bEOL has been set in the
		// past - probably somewhere else, so this is not necessarily
		// the correct place to do it.  2001.02.25 jskov
		bEOL = true;
		return;
	}

	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
	const UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

	// catch the case of a click directly on the left half of the first character in the run
	if (x < (pCharWidths[m_iOffsetFirst] / 2))
	{
#ifdef BIDI_ENABLED
		pos = m_pBL->getPosition() + getOffsetFirstVis();
#else
		pos = m_pBL->getPosition() + m_iOffsetFirst;
#endif
		bBOL = false;
		bEOL = false;
		return;
	}
	
	UT_sint32 iWidth = 0;
	for (UT_uint32 i=m_iOffsetFirst; i<(m_iOffsetFirst + m_iLen); i++)
	{
#ifdef BIDI_ENABLED
		// i represents VISUAL offset but the CharWidths array uses logical order of indexing
		iWidth += pCharWidths[getOffsetLog(i)];
#else
		iWidth += pCharWidths[i];
#endif
		if (iWidth > x)
		{
			if ((iWidth - x) <= (pCharWidths[i] / 2))
			{
				i++;
			}

			// NOTE: this allows inserted text to be coalesced in the PT
			bEOL = true;
#ifdef BIDI_ENABLED
			pos = m_pBL->getPosition() + getOffsetLog(i);
#else
			pos = m_pBL->getPosition() + i;
#endif
			return;
		}
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void fp_TextRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	UT_sint32 xoff;
	UT_sint32 yoff;
#ifdef BIDI_ENABLED
	UT_sint32 xoff2;
	UT_sint32 yoff2;
	UT_sint32 xdiff = 0;
#endif

	UT_ASSERT(m_pLine);
	m_pLine->getOffsets(this, xoff, yoff);
	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
	const UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

	UT_uint32 offset = UT_MIN(iOffset, m_iOffsetFirst + m_iLen);
	
	for (UT_uint32 i=m_iOffsetFirst; i<offset; i++)
	{
#ifdef BIDI_ENABLED
			xdiff += pCharWidths[i];
#else
			xoff += pCharWidths[i];
#endif
	}
	if (m_fPosition == TEXT_POSITION_SUPERSCRIPT)
	{
		yoff -= m_iAscent * 1/2;
	}
	else if (m_fPosition == TEXT_POSITION_SUBSCRIPT)
	{
		yoff += m_iDescent /* * 3/2 */;
	}
#ifdef BIDI_ENABLED
	UT_sint32 iDirection = getVisDirection();
	UT_sint32 iNextDir = iDirection ? 0 : 1; //if this is last run we will anticipate the next to have *different* direction
	fp_Run * pRun = 0;   //will use 0 as indicator that there is no need to deal with the second caret
#ifdef UT_DEBUG	
	UT_uint32 rtype;
#endif
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
#ifdef UT_DEBUG	
	        rtype = pRun->getType();
#endif	
	    }
	}

	if(iDirection == 1)                 //#TF rtl run
	{
	    x = xoff + m_iWidth - xdiff; //we want the caret right of the char
	}
	else
	{
	    x = xoff + xdiff;
	}
	
    if(pRun && (iNextDir != iDirection)) //followed by run of different direction, have to split caret
	{
		x2 = (iNextDir == 0) ? xoff2 : xoff2 + pRun->getWidth();
		y2 = yoff2;
	}
	else
	{
	    x2 = x;
	    y2 = yoff;
	}
	bDirection = (iDirection != 0);
#else	    // ! BIDI_ENABLED
	x = xoff;
#endif
	y = yoff;
	height = m_iHeight;
	//UT_DEBUGMSG(("fintPointCoords: TextRun x,y,x2,y2=[%d, %d, %d, %d]\n", x,y,x2,y2));	

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
		|| (m_iSpaceWidthBeforeJustification != JUSTIFICATION_NOT_USED)
		|| (pNext->m_iSpaceWidthBeforeJustification != JUSTIFICATION_NOT_USED)
		|| (pNext->m_pField != m_pField)
		|| (pNext->m_pLanguage != m_pLanguage)  //this is not a bug
#ifdef BIDI_ENABLED
		|| (pNext->m_iDirection != m_iDirection)  //#TF cannot merge runs of different direction of writing
#endif
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
#ifdef BIDI_ENABLED
	UT_ASSERT(m_iDirection == pNext->m_iDirection); //#TF
#endif
//	UT_ASSERT(m_iSpaceWidthBeforeJustification == pNext->m_iSpaceWidthBeforeJustification);

	m_pField = pNext->m_pField; 	
    m_iWidth += pNext->m_iWidth;
	m_iWidthLayoutUnits += pNext->m_iWidthLayoutUnits;
	m_iLen += pNext->m_iLen;
	m_bDirty = m_bDirty || pNext->m_bDirty;
	m_pNext = pNext->getNext();
	if (m_pNext)
	{
		m_pNext->setPrev(this);
	}

	pNext->getLine()->removeRun(pNext, false);

	delete pNext;
}

bool fp_TextRun::split(UT_uint32 iSplitOffset)
{
	UT_ASSERT(iSplitOffset >= m_iOffsetFirst);
	UT_ASSERT(iSplitOffset < (m_iOffsetFirst + m_iLen));
	
	fp_TextRun* pNew = new fp_TextRun(m_pBL, m_pG, iSplitOffset, m_iLen - (iSplitOffset - m_iOffsetFirst), false);
	UT_ASSERT(pNew);
	pNew->m_pScreenFont = this->m_pScreenFont;
	pNew->m_pLayoutFont = this->m_pLayoutFont;
	pNew->m_fDecorations = this->m_fDecorations;
	pNew->m_colorFG = this->m_colorFG;
	pNew->m_pField = this->m_pField;
        pNew->m_fPosition = this->m_fPosition;

	pNew->m_iAscent = this->m_iAscent;
	pNew->m_iDescent = this->m_iDescent;
	pNew->m_iHeight = this->m_iHeight;
	pNew->m_iAscentLayoutUnits = this->m_iAscentLayoutUnits;
	UT_ASSERT(pNew->m_iAscentLayoutUnits);
	pNew->m_iDescentLayoutUnits = this->m_iDescentLayoutUnits;
	pNew->m_iHeightLayoutUnits = this->m_iHeightLayoutUnits;
	pNew->m_iLineWidth = this->m_iLineWidth;
	pNew->m_bDirty = this->m_bDirty;
	pNew->m_pLanguage = this->m_pLanguage;
#ifdef BIDI_ENABLED
	pNew->m_iDirection = this->m_iDirection; //#TF
#endif
//	pNew->m_iSpaceWidthBeforeJustification = this->m_iSpaceWidthBeforeJustification;

	pNew->m_pPrev = this;
	pNew->m_pNext = this->m_pNext;
	if (m_pNext)
	{
		m_pNext->setPrev(pNew);
	}
	m_pNext = pNew;

	m_iLen = iSplitOffset - m_iOffsetFirst;

	m_pLine->insertRunAfter(pNew, this);

	m_iWidth = simpleRecalcWidth(Width_type_display);
	m_iWidthLayoutUnits = simpleRecalcWidth(Width_type_layout_units);
#ifdef BIDI_ENABLED
	//bool bDomDirection = m_pBL->getDominantDirection();
	
	if(getVisDirection() == 0)
	{
		pNew->m_iX = m_iX + m_iWidth;
	}
	else
	{
	    pNew->m_iX = m_iX;
	    m_iX += pNew->m_iWidth;
	}
#else
	pNew->m_iX = m_iX + m_iWidth;
#endif
	pNew->m_iY = m_iY;
	pNew->m_iWidth = pNew->simpleRecalcWidth(Width_type_display);
	pNew->m_iWidthLayoutUnits = pNew->simpleRecalcWidth(Width_type_layout_units);
	
	return true;
}

void fp_TextRun::_fetchCharWidths(GR_Font* pFont, UT_uint16* pCharWidths)
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

	UT_uint16* pCharWidths = pgbCharWidths->getCharWidths()->getPointer(0);
	_fetchCharWidths(m_pScreenFont, pCharWidths);

	pCharWidths = pgbCharWidths->getCharWidthsLayoutUnits()->getPointer(0);
	_fetchCharWidths(m_pLayoutFont, pCharWidths);

	m_pG->setFont(m_pScreenFont);

}

UT_sint32 fp_TextRun::simpleRecalcWidth(UT_sint32 iWidthType, UT_sint32 iLength) const
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

	UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);
	
	UT_sint32 iWidth = 0;

	{
		const UT_UCSChar* pSpan;
		UT_uint32 lenSpan;
		UT_uint32 offset = m_iOffsetFirst;
		UT_uint32 len = iLength;
		bool bContinue = true;

		while (bContinue)
		{
			bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
			if (!bContinue)		// if this fails, we are out of sync with the PTbl.
			{					// this probably means we are the right half of a split
				break;			// made to break a paragraph.
			}
			UT_ASSERT(lenSpan>0);

			if (lenSpan > len)
			{
				lenSpan = len;
			}
			for (UT_uint32 i=0; i<lenSpan; i++)
			{
				iWidth += pCharWidths[i + offset];
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
	}

	return iWidth;
}

bool fp_TextRun::recalcWidth(void)
{
	UT_sint32 iWidth = simpleRecalcWidth(Width_type_display);
	
	if (iWidth == m_iWidth)
	{
		return false;
	}

	if (m_iWidth)
	{
		clearScreen();
	}
	
	m_iWidth = iWidth;
	m_iWidthLayoutUnits = simpleRecalcWidth(Width_type_layout_units);

	return true;
}

void fp_TextRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

#ifdef BIDI_ENABLED
	if(!m_pLine->isEmpty() && m_pLine->getLastVisRun() == this)   //#TF must be last visual run
#else
	if(!m_pLine->isEmpty() && m_pLine->getLastRun() == this)
#endif
	{
		// Last run on the line so clear to end.

		m_pLine->clearScreenFromRunToEnd(m_pLine->countRuns() - 1);
	}
	else
	{
		m_pG->setFont(m_pScreenFont);
		
		/*
		  TODO this should not be hard-coded.  We need to figure out
		  what the appropriate background color for this run is, and
		  use that.  Note that it could vary on a run-by-run basis,
		  since document facilities allow the background color to be
		  changed, for things such as table cells.
		*/
		UT_RGBColor clrNormalBackground(m_colorHL.m_red, m_colorHL.m_grn, m_colorHL.m_blu);
		
		if (m_pField)
		{
		  UT_setColor (clrNormalBackground, 220, 220, 220);
		}
		m_pG->setColor(clrNormalBackground);
		
		UT_sint32 xoff = 0, yoff = 0;
		m_pLine->getScreenOffsets(this, xoff, yoff);
		m_pG->fillRect(clrNormalBackground,xoff, yoff, m_iWidth, m_pLine->getHeight());
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

	m_pG->setFont(m_pScreenFont);
	m_pG->setColor(m_colorFG);

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

	UT_uint32 iRunBase = iBase + m_iOffsetFirst;

	FV_View* pView = m_pBL->getDocLayout()->getView();
	UT_uint32 iSelAnchor = pView->getSelectionAnchor();
	UT_uint32 iPoint = pView->getPoint();

	UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
	UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);
	
	UT_ASSERT(iSel1 <= iSel2);
	
	if (pView->getFocus()==AV_FOCUS_NONE || iSel1 == iSel2)
	{
		// nothing in this run is selected
		_fillRect(clrNormalBackground, pDA->xoff, yTopOfSel, m_iOffsetFirst, m_iLen, pgbCharWidths);
		_drawPart(pDA->xoff, yTopOfRun, m_iOffsetFirst, m_iLen, pgbCharWidths);
	}
	else if (iSel1 <= iRunBase)
	{
		if (iSel2 <= iRunBase)
		{
			// nothing in this run is selected
			_fillRect(clrNormalBackground, pDA->xoff, yTopOfSel, m_iOffsetFirst, m_iLen, pgbCharWidths);
			_drawPart(pDA->xoff, yTopOfRun, m_iOffsetFirst, m_iLen, pgbCharWidths);
		}
		else if (iSel2 >= (iRunBase + m_iLen))
		{
			// the whole run is selected			
			_fillRect(clrSelBackground, pDA->xoff, yTopOfSel, m_iOffsetFirst, m_iLen, pgbCharWidths);
			_drawPart(pDA->xoff, yTopOfRun, m_iOffsetFirst, m_iLen, pgbCharWidths);
		}
		else
		{
			// the first part is selected, the second part is not
			_fillRect(clrSelBackground, pDA->xoff, yTopOfSel, m_iOffsetFirst, iSel2 - iRunBase, pgbCharWidths);
			_drawPart(pDA->xoff, yTopOfRun, m_iOffsetFirst, iSel2 - iRunBase, pgbCharWidths);
            _fillRect(clrNormalBackground, pDA->xoff, yTopOfSel, iSel2 - iBase, m_iLen - (iSel2 - iRunBase), pgbCharWidths);
			_drawPart(pDA->xoff, yTopOfRun, iSel2 - iBase, m_iLen - (iSel2 - iRunBase), pgbCharWidths);
		}
	}
	else if (iSel1 >= (iRunBase + m_iLen))
	{
		// nothing in this run is selected
        _fillRect(clrNormalBackground, pDA->xoff, yTopOfSel, m_iOffsetFirst, m_iLen, pgbCharWidths);
		_drawPart(pDA->xoff, yTopOfRun, m_iOffsetFirst, m_iLen, pgbCharWidths);
	}
	else
	{
        _fillRect(clrNormalBackground, pDA->xoff, yTopOfSel, m_iOffsetFirst, iSel1 - iRunBase, pgbCharWidths);
		_drawPart(pDA->xoff, yTopOfRun, m_iOffsetFirst, iSel1 - iRunBase, pgbCharWidths);
		
		if (iSel2 >= (iRunBase + m_iLen))
		{
			_fillRect(clrSelBackground, pDA->xoff, yTopOfSel, iSel1 - iBase, m_iLen - (iSel1 - iRunBase), pgbCharWidths);
			_drawPart(pDA->xoff, yTopOfRun, iSel1 - iBase, m_iLen - (iSel1 - iRunBase), pgbCharWidths);
		}
		else
		{
			_fillRect(clrSelBackground, pDA->xoff, yTopOfSel, iSel1 - iBase, iSel2 - iSel1, pgbCharWidths);
			_drawPart(pDA->xoff, yTopOfRun, iSel1 - iBase, iSel2 - iSel1, pgbCharWidths);
            _fillRect(clrNormalBackground, pDA->xoff, yTopOfSel, iSel2 - iBase, m_iLen - (iSel2 - iRunBase), pgbCharWidths);
			_drawPart(pDA->xoff, yTopOfRun, iSel2 - iBase, m_iLen - (iSel2 - iRunBase), pgbCharWidths);
		}
	}

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
	//if (m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
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
#ifndef BIDI_ENABLED	
	pRect->left = xoff;
#endif
	pRect->top = yoff;
	pRect->height = m_iHeight;
	pRect->width = 0;

	// that's enough for zero-length run
	if (m_iLen == 0)
	{
#ifdef BIDI_ENABLED
		pRect->left = xoff;
#endif
		return;
	}

	//Should this be an error condition? I treat it as a zero length run, but other might not	
	const UT_uint16 * pCharWidths = pgbCharWidths->getPointer(0);

#ifdef BIDI_ENABLED
	pRect->left = 0;//#TF need 0 because of BiDi, need to calculate the width of the non-selected
			//section first rather than the abs pos of the left corner
#endif
	
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
	
#ifdef BIDI_ENABLED
	if(getVisDirection() == 0)
	{
		pRect->left += xoff; //if this is ltr then adding xoff is all that is needed
	}
#endif
	for (i=iStart; i<(iStart + iLen); i++)
	{
		pRect->width += pCharWidths[i];
	}
#ifdef BIDI_ENABLED
	//in case of rtl we are now in the position to calculate the position of the the left corner
	if(getVisDirection() == 1) pRect->left = xoff + m_iWidth - pRect->left - pRect->width;
#endif
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
	
	const UT_UCSChar* pSpan = NULL;
	UT_uint32 lenSpan = 0;
	UT_uint32 offset = iStart;
	UT_uint32 len = iLen;
	bool bContinue = true;

	// don't even try to draw a zero-length run
	if (m_iLen == 0)
	{
		return;
	}

	UT_ASSERT(offset >= m_iOffsetFirst);
	UT_ASSERT(offset + len <= m_iOffsetFirst + m_iLen);

	UT_uint32 iLeftWidth = 0;
	const UT_uint16 * pCharWidths = pgbCharWidths->getPointer(0);
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

#ifdef BIDI_ENABLED
	if(getVisDirection() == 1)
	{
		iLeftWidth = m_iWidth - iLeftWidth;
	}
	
	for(;;)
	{
		bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
		UT_ASSERT(lenSpan>0);

		//we need special buffer for the rtl text so we could draw reversed
		if(lenSpan > s_iSpanBuffSize) //the buffer too small, reallocate
		{
			delete[] s_pSpanBuff;
			s_pSpanBuff = new UT_UCSChar[lenSpan + 1];
			s_iSpanBuffSize = lenSpan + 1;
			UT_ASSERT(s_pSpanBuff);
		}
	
		UT_uint32 iTrueLen = (lenSpan > len) ? len : lenSpan;
		UT_UCS_strncpy(s_pSpanBuff, pSpan, iTrueLen);
	
	
		
		if(getVisDirection() == 1) //rtl: determine the width of the text we are to print
		{
			UT_UCS_strnrev(s_pSpanBuff, iTrueLen);
			for (i= 0; i < iTrueLen; i++)
			{
				iLeftWidth -= pCharWidths[offset + i];
			}
		}
			
		m_pG->drawChars(s_pSpanBuff, 0, iTrueLen, xoff + iLeftWidth, yoff);
			
		if((iTrueLen == len) || !bContinue)
		{
			break;
		}

		if(getVisDirection() == 0)
		{
			for(i = 0; i < iTrueLen; i++)
			{
				iLeftWidth += pCharWidths[offset + i];
			}
		}

		offset += iTrueLen;
		len -= iTrueLen;

		UT_ASSERT(offset >= m_iOffsetFirst);
		UT_ASSERT(offset + len <= m_iOffsetFirst + m_iLen);
	}
#else
	while (bContinue)
	{
		bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
		UT_ASSERT(lenSpan>0);

		if (len <= lenSpan)
		{
			m_pG->drawChars(pSpan, 0, len, xoff + iLeftWidth, yoff);

			bContinue = false;
		}
		else
		{
			m_pG->drawChars(pSpan, 0, lenSpan, xoff + iLeftWidth, yoff);
			for(i = 0; i < lenSpan; i++)
			{
				iLeftWidth += pCharWidths[offset + i];
			}


			offset += lenSpan;
			len -= lenSpan;

			UT_ASSERT(offset >= m_iOffsetFirst);
			UT_ASSERT(offset + len <= m_iOffsetFirst + m_iLen);
		}
	}
#endif
}

void fp_TextRun::_drawInvisibleSpaces(UT_sint32 xoff, UT_sint32 yoff)
{
    UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
    UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);
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

    if(findCharacter(0, UCS_SPACE) > 0){
        while(bContinue){
            bContinue = m_pBL->getSpanPtr(offset,&pSpan,&lenSpan);
            UT_ASSERT(lenSpan > 0);


            if(lenSpan > len){
                lenSpan = len;
            }

            UT_uint32 iy = yoff + getAscent() * 2 / 3;

            for (UT_uint32 i = 0;i < lenSpan;i++){
               if(pSpan[i] == UCS_SPACE){
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
			structure.  They're all x, y but different widths.  Bummer.
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
		// I think this is safe, although it begs the question, why did we get called if iLen is zero?  TODO
		return;
	}

	UT_sint32 xoff = 0, yoff = 0;
	UT_sint32 iAscent = m_pLine->getAscent();
	UT_sint32 iDescent = m_pLine->getDescent();

	// we'd prefer squiggle to leave one pixel below the baseline,
	// but we need to force all three pixels inside the descent
	UT_sint32 iGap = (iDescent > 3) ? 1 : (iDescent - 3);

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
			UT_ASSERT(lenSpan>0);

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
			if(lenSpan <= 0)
			{ 
				fp_Line * pLine = m_pBL->getFirstLine();
				UT_DEBUGMSG(("SEVIOR: Line for this run = %x \n",getLine()));
				UT_DEBUGMSG(("SEVIOR: Line list Run %x Block %x pLine %x \n",this,m_pBL,pLine));
				while(pLine)
				{
					UT_DEBUGMSG(("SEVIOR: Line list Run %x Block %x pLine %x \n",this,m_pBL,pLine));
					pLine = pLine->getNext();
				}
			}
			UT_ASSERT(lenSpan>0);

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
	UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

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

UT_sint32 fp_TextRun::findTrailingSpaceDistanceInLayoutUnits(void) const
{
	UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidthsLayoutUnits();
	UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

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

void fp_TextRun::resetJustification()
{
	if(m_iSpaceWidthBeforeJustification != JUSTIFICATION_NOT_USED)
	{
		UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
		UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

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
	}
	
	m_iSpaceWidthBeforeJustification = JUSTIFICATION_NOT_USED;
}

void fp_TextRun::distributeJustificationAmongstSpaces(UT_sint32 iAmount, UT_uint32 iSpacesInRun)
{
	UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
	UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

	UT_ASSERT(iSpacesInRun);

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

UT_uint32 fp_TextRun::countJustificationPoints(void) const
{
	UT_uint32 iCount = 0;

	if(m_iLen > 0)
	{
		UT_sint32 i = findCharacter(0, UCS_SPACE);

		while (i >= 0)
		{
			iCount++;

			// keep looping
			i = findCharacter(i+1-m_iOffsetFirst, UCS_SPACE);
		}
	}

	return iCount;
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

#ifdef BIDI_ENABLED
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

			if (len <= lenSpan)     //copy the entire len to pStr and return 0
			{
				UT_ASSERT(len>0);
                		UT_UCS_strncpy(pStrPos, pSpan, len);
                		pStr[len] = 0;                        //make sure the string is 00-terminated
                		iMax = m_iLen;
                		return(false);
			}
			else  //copy what we have got and move on to the next span
			{
				UT_UCS_strncpy(pStrPos, pSpan, lenSpan);
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

/*
	If it receives -1,0, or 1, it will set direction to this value.
	If it receives -2, it will workout the value either using Unicode
	if in Unicode mode, or it will just return.
*/

void fp_TextRun::setDirection(UT_sint32 dir)
{
	if(!m_iLen)
	{
		return; //ignore 0-length runs, let them be treated on basis of the app defaults
	}

	UT_sint32 prevDir = m_iDirection;	
	if(dir == -2)
	{
		if(m_iDirOverride == -1)
		{
			UT_UCSChar firstChar;
			getCharacter(0, firstChar);

			switch (isUCharRTL(firstChar))
			{
	    		case 1:
	        		m_iDirection = 1;
	        		break;
	        		
	    		case 0:
	        		m_iDirection = 0;
	        		break;
	        		
		    	default:
		        	m_iDirection = -1;
  			}
 			
		}
		else
		{
			m_iDirection = m_iDirOverride;
		}
	}
	else //meaningfull value received
	{
		m_iDirection = dir;
	}
	
	setDirectionProperty(m_iDirection);
	
	/*
		if this run belongs to a line we have to notify the line that
		that it now contains a run of this direction, if it does not belong
		to a line this will be taken care of by the fp_Line:: member function
		used to add the run to the line (generally, we set it here if this
		is a run that is being typed in and it gets set in the member
		functions when the run is loaded from a document on the disk.)
	*/
	
	if(m_iDirection != prevDir)
	{
		if(m_pLine)
			m_pLine->addDirectionUsed(m_iDirection);
		clearScreen();
	}
	
	//UT_DEBUGMSG(("TextRun::setDirection: direction=%d\n", m_iDirection));
}
#endif
