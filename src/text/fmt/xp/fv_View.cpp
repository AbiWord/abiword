
/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <locale.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_timer.h"

#include "xav_View.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fl_Squiggles.h"
#include "fl_SectionLayout.h"
#include "fl_AutoNum.h"
#include "fp_Page.h"
#include "fp_PageSize.h"
#include "fp_Column.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_TextRun.h"
#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "pd_Document.h"
#include "pd_Style.h"
#include "pp_Property.h"
#include "pp_AttrProp.h"
#include "gr_Graphics.h"
#include "gr_DrawArgs.h"
#include "ie_types.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Clipboard.h"
#include "ap_TopRuler.h"
#include "ap_LeftRuler.h"
#include "ap_Prefs.h"
#include "fd_Field.h"
#include "spell_manager.h"
#include "ut_rand.h"

#include "xap_EncodingManager.h"

#if 1
// todo: work around to remove the INPUTWORDLEN restriction for pspell
#include "ispell_def.h"
#endif

// NB -- irrespective of this size, the piecetable will store
// at max BOOKMARK_NAME_LIMIT of chars as defined in pf_Frag_Bookmark.h
#define BOOKMARK_NAME_SIZE 30

/****************************************************************/

class _fmtPair
{
public:
	_fmtPair(const XML_Char * p, 
			 const PP_AttrProp * c, const PP_AttrProp * b, const PP_AttrProp * s, 
			 PD_Document * pDoc, bool bExpandStyles)
		{
			m_prop = p;
			m_val  = PP_evalProperty(p,c,b,s, pDoc, bExpandStyles);
		}

	const XML_Char *	m_prop;
	const XML_Char *	m_val;
};

/****************************************************************/

FV_View::FV_View(XAP_App * pApp, void* pParentData, FL_DocLayout* pLayout)
	:	AV_View(pApp, pParentData),
		m_iInsPoint(0),
		m_xPoint(0),
		m_yPoint(0),
		m_iPointHeight(0),
		m_oldxPoint(0),
		m_oldyPoint(0),
		m_oldiPointHeight(0),
		m_xPointSticky(0),
		m_bPointVisible(false),
		m_bPointEOL(false),
		m_pLayout(pLayout),
		m_pDoc(pLayout->getDocument()),
		m_pG(m_pLayout->getGraphics()),
		m_pParentData(pParentData),
		m_iSelectionAnchor(0),
		m_iSelectionLeftAnchor(0),
		m_iSelectionRightAnchor(0),
		m_bSelection(false),
		m_pAutoScrollTimer(0),
		m_xLastMouse(0),
		m_yLastMouse(0),
		m_pAutoCursorTimer(0),
		m_bCursorIsOn(false),
		m_bEraseSaysStopBlinking(false),
		m_wrappedEnd(false),
		m_startPosition(0),
		m_doneFind(false),
		m_bEditHdrFtr(false),
		m_pEditShadow(NULL),
		m_iSavedPosition(0),
		m_bNeedSavedPosition(false),
		_m_matchCase(false),
		_m_findNextString(0),
		m_bShowPara(false),
		m_viewMode(VIEW_PRINT),
		m_previewMode(PREVIEW_NONE),
		m_bDontUpdateScreenOnGeneralUpdate(false),
		m_iPieceTableState(0),
		m_iMouseX(0),
		m_iMouseY(0)
{
	// initialize prefs cache
	pApp->getPrefsValueBool(AP_PREF_KEY_CursorBlink, &m_bCursorBlink);

	// initialize prefs listener
	pApp->getPrefs()->addListener( _prefsListener, this );

	// Get View Mode
	if(m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		const char * szViewMode = NULL;
		pApp->getPrefsValue((const char *) AP_PREF_KEY_LayoutMode,&szViewMode);
		if(strcmp(szViewMode,"1") == 0)
		{
			setViewMode(VIEW_PRINT);
		}
		if(strcmp(szViewMode,"2") == 0)
		{
			setViewMode(VIEW_NORMAL);
		}
		if(strcmp(szViewMode,"3") == 0)
		{
			setViewMode(VIEW_WEB);
		}
		m_pG->setCursor(GR_Graphics::GR_CURSOR_WAIT);
	}
#ifdef BIDI_ENABLED
	pApp->getPrefsValueBool(AP_PREF_KEY_DefaultDirectionRtl, &m_bDefaultDirectionRtl);
	pApp->getPrefsValueBool(XAP_PREF_KEY_UseHebrewContextGlyphs, &m_bUseHebrewContextGlyphs);
	/*
		If the default direction indicated by the preferences is different
		than the direction with which we were compiled, we need to modify
		the default values stored in _props[] and also the direction
		related properties of the Normal style (dir, dom-dir,
		and text-align)
	*/

#ifndef BIDI_RTL_DOMINANT
	if(m_bDefaultDirectionRtl)
	{
		//const XML_Char bidi_dir_name[] = "dir"; ###TF
		const XML_Char bidi_dir_value[] = "rtl";
		const XML_Char bidi_domdir_name[] = "dom-dir";
		const XML_Char bidi_align_name[] = "text-align";
		const XML_Char bidi_align_value[] = "right";

		const XML_Char * bidi_props[5]= {/*bidi_dir_name, bidi_dir_value, */bidi_domdir_name, bidi_dir_value, bidi_align_name, bidi_align_value,0};

		m_pDoc->addStyleProperties((const XML_Char*)"Normal", (const XML_Char**)bidi_props);
		PP_resetInitialBiDiValues("rtl");
	}
#else
	if(!m_bDefaultDirectionRtl)
	{
		//const XML_Char bidi_dir_name[] = "dir";  ###TF
		const XML_Char bidi_dir_value[] = "ltr";
		const XML_Char bidi_domdir_name[] = "dom-dir";
		const XML_Char bidi_align_name[] = "text-align";
		const XML_Char bidi_align_value[] = "left";

		const XML_Char * bidi_props[5]= {/*bidi_dir_name, bidi_dir_value, */bidi_domdir_name, bidi_dir_value, bidi_align_name, bidi_align_value,0};
		
		m_pDoc->addStyleProperties((const XML_Char*)"Normal", (const XML_Char**)bidi_props);
		PP_resetInitialBiDiValues("ltr");
	}
#endif
#endif
	// initialize change cache
	m_chg.bUndo = false;
	m_chg.bRedo = false;
	m_chg.bDirty = false;
	m_chg.bSelection = false;
	m_chg.iColumn = 0;						 // current column number
	m_chg.propsChar = NULL;
	m_chg.propsBlock = NULL;
	m_chg.propsSection = NULL;

	pLayout->setView(this);

	//Test_Dump();

	m_iSelectionAnchor = getPoint();
	_resetSelection();
	_clearOldPoint();
//
// Update the combo boxes on the frame with this documents info.
//
	XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
	if( pFrame )
	  pFrame->repopulateCombos();
}

FV_View::~FV_View()
{
	// remove prefs listener
	m_pApp->getPrefs()->removeListener( _prefsListener, this );

	DELETEP(m_pAutoScrollTimer);
	DELETEP(m_pAutoCursorTimer);
	
	FREEP(_m_findNextString);
	
	FREEP(m_chg.propsChar);
	FREEP(m_chg.propsBlock);
	FREEP(m_chg.propsSection);
}


#if 0
// the code is not directly in the case function, as passing the 
// arguments is too awkward
static void _toggleSentence (const UT_UCSChar * src, 
				 UT_UCSChar * dest, UT_uint32 len, const UT_UCSChar * prev)
{
	if(!prev || (UT_UCS_isSentenceSeparator(prev[0]) && UT_UCS_isspace (prev[1])))
	{
		dest[0] = UT_UCS_toupper (src[0]);
		dest[1] = src[1];
	}
	else
	{
		dest[0] = src[0];
		dest[1] = src[1];
	}

	for (UT_uint32 i = 2; i < len; i++)
	{
		if(UT_UCS_isSentenceSeparator(src[i-2]) && UT_UCS_isspace (src[i-1]))
			dest[i] = UT_UCS_toupper (src[i]);
		else
			dest[i] = src[i];
	}
}
#endif

static void _toggleFirstCapital(const UT_UCSChar * src,
				 UT_UCSChar * dest, UT_uint32 len, const UT_UCSChar * prev)
{
	if(!prev || UT_UCS_isspace(prev[0]))
	{
		dest[0] = UT_UCS_toupper(src[0]);
	}
	else
	{
		dest[0] = UT_UCS_tolower(src[0]);
	}
	
	for (UT_uint32 i = 1; i < len; i++)
	{
		if(UT_UCS_isspace(src[i-1]))
			dest[i] = UT_UCS_toupper (src[i]);
		else
			dest[i] = UT_UCS_tolower (src[i]);
	}
}

// all gets set to lowercase
static void _toggleLower (const UT_UCSChar * src, 
			  UT_UCSChar * dest, UT_uint32 len)
{
	for (UT_uint32 i = 0; i < len; i++)
	{
		dest[i] = UT_UCS_tolower (src[i]);
	}
}

// all gets set to uppercase
static void _toggleUpper (const UT_UCSChar * src, 
			  UT_UCSChar * dest, UT_uint32 len)
{
	for (UT_uint32 i = 0; i < len; i++)
	{
		dest[i] = UT_UCS_toupper (src[i]);
	}
}

// first character after each space gets capitalized
// NB the title case algorithm here is too simplistic to
// produce satisfactory results; basically we would need
// language-dependent word lists to handle this correctly
// and I see little need for it; so for now it is going to
// be turned off
#if 0
static void _toggleTitle (const UT_UCSChar * src, 
			  UT_UCSChar * dest, UT_uint32 len,
						  bool spaceBeforeFirstChar)
{
	bool wasSpace = spaceBeforeFirstChar;

	UT_UCSChar ch;
	
	for (UT_uint32 i = 0; i < len; i++)
	{
		ch = src[i];
		if (wasSpace && !UT_UCS_isspace (ch))
		{
			dest[i] = UT_UCS_toupper (ch);
			wasSpace = false;
		}
		else if (UT_UCS_isspace (ch))
		{
			dest[i] = ch;
			wasSpace = true;
		}
		else
		{
			dest[i] = ch;
		}
	}
}
#endif

// all gets set to its opposite
// I have my doubts about usufulness of this, but the concensus on
// the mailing list was to leave it, so it stays.
static void _toggleToggle (const UT_UCSChar * src, 
			   UT_UCSChar * dest, UT_uint32 len)
{
	UT_UCSChar ch;
	for (UT_uint32 i = 0; i < len; i++)
	{
		ch = src[i];

		if (UT_UCS_islower (ch))
			dest[i] = UT_UCS_toupper (ch);
		else
			dest[i] = UT_UCS_tolower (src[i]);
	}
}

// returns true iff the character BEFORE pos is a space.
// Special cases:
// -returns true if pos is at the beginning of the document
// -returns false if pos is not within the document
bool FV_View::_isSpaceBefore(PT_DocPosition pos)
{
	UT_GrowBuf buffer;
	
	fl_BlockLayout * block = m_pLayout->findBlockAtPosition(pos);
	if (block)
	{

		PT_DocPosition offset = pos - block->getPosition(false);
		// Just look at the previous character in this block, if there is one...
		if (offset > 0)
		{
			block->getBlockBuf(&buffer);
			return (UT_UCS_isspace(*(UT_UCSChar *)buffer.getPointer(offset - 1)));
		}
		else
		{	   
			return true;
		}
	}
	else
		return false;
}

void FV_View::toggleCase (ToggleCase c)
{
	// if there is no selection, apply to the current word	
	PT_DocPosition origPos = 0;
	PT_DocPosition low, high;
	
	if (isSelectionEmpty())
	{
		origPos = m_iInsPoint;
		low = _getDocPos(FV_DOCPOS_BOW, false);
		high = _getDocPos(FV_DOCPOS_EOW_SELECT, false);
	}
	else
	{	
		if(m_iInsPoint < m_iSelectionAnchor)
		{
			low  = m_iInsPoint;
			high = m_iSelectionAnchor;
		}
		else
		{
			high = m_iInsPoint;
			low  = m_iSelectionAnchor;
		}
	}

	// if this is an empty document, gracefully return	
	if(low == high)
		return;
		
	UT_DEBUGMSG(("fv_View::toggleCase: low %d, high %d\n", low, high));
	
	UT_sint32 xPoint, yPoint, xPoint2, yPoint2, iPointHeight;
	bool bDirection;
	
	fl_BlockLayout * pBL =	m_pLayout->findBlockAtPosition(low);
	fp_Run * pRun;

	// create a temp buffer of a reasonable size (will realloc if too small)
	UT_uint32	 iTempLen = 150;
	UT_UCSChar * pTemp = new UT_UCSChar[iTempLen];
	UT_ASSERT(pTemp);
	
	_saveAndNotifyPieceTableChange();
	m_pDoc->beginUserAtomicGlob();

	UT_UCSChar * prev;
	
	while(pBL && low < high)
	{
		UT_GrowBuf buffer;
		pBL->getBlockBuf(&buffer);

		PT_DocPosition offset = low - pBL->getPosition(false);

		if(c == CASE_ROTATE)
		{
			// workout the current case
			UT_UCSChar * pT = buffer.getPointer(offset);
			xxx_UT_DEBUGMSG(("pT 0x%x, pT + 1 0x%x\n", *pT, *(pT+1)));
			
			if(pT && UT_UCS_islower(*pT) && buffer.getLength() > 1 && UT_UCS_islower (*(pT+1)))
			{
				// lowercase, make first capital
				xxx_UT_DEBUGMSG(("case 1\n"));
				c = CASE_FIRST_CAPITAL;
			}
			else if(pT && !UT_UCS_islower (*pT) && buffer.getLength() > 1 && UT_UCS_islower (*(pT+1)))
			{
				// first capital, make upper
				xxx_UT_DEBUGMSG(("case 2\n"));
				c = CASE_UPPER;
			}
			else if(pT && buffer.getLength() == 1 && UT_UCS_islower (*pT))
			{
				// single lowercase letter
				xxx_UT_DEBUGMSG(("case 3\n"));
				c = CASE_UPPER;
			}
			else
			{
				UT_DEBUGMSG(("case 4\n"));
				c = CASE_LOWER;
			}
			
			xxx_UT_DEBUGMSG(("CASE_ROTATE translated to %d\n", c));
			
		}
				
		const PP_AttrProp * pSpanAPAfter = NULL;
		pBL->getSpanAttrProp(offset,false,&pSpanAPAfter);
		PP_AttrProp * pSpanAPNow = const_cast<PP_AttrProp *>(pSpanAPAfter);

		xxx_UT_DEBUGMSG(("fv_View::toggleCase: pBL 0x%x, offset %d, pSpanAPAfter 0x%x\n", pBL, offset, pSpanAPAfter));
		//fp_Run * pLastRun = pBL->getLastLine()->getLastRun();
		//PT_DocPosition lastPos = pBL->getPosition(false) + pLastRun->getBlockOffset() + pLastRun->getLength() - 1;
		pRun = pBL->findPointCoords(low, false, xPoint,
										   yPoint, xPoint2, yPoint2,
										   iPointHeight, bDirection);
		xxx_UT_DEBUGMSG(("fv_View::toggleCase: block offset %d, low %d, high %d, lastPos %%d\n", offset, low, high/*, lastPos*/));
		
		bool bBlockDone = false;
			
		while(!bBlockDone && (low < high) /*&& (low < lastPos)*/)
		{
			UT_uint32 iLenToCopy = UT_MIN(high - low, buffer.getLength() - offset);

			xxx_UT_DEBUGMSG(("fv_View::toggleCase: iLenToCopy %d, low %d\n", iLenToCopy, low));
				
			if(!pRun || pRun->getType() == FPRUN_ENDOFPARAGRAPH)
				break;
									
			if(iLenToCopy > iTempLen)
			{
				delete[] pTemp;
				pTemp = new UT_UCSChar[iLenToCopy];
				iTempLen = iLenToCopy;
				UT_ASSERT(pTemp);
			}
			
			while(pRun && iLenToCopy > 0)
			{
				UT_uint32 iLen = 0;

				UT_ASSERT(pRun);
		
				while( pRun
					&& pRun->getType() != FPRUN_TEXT)
				{
					offset += pRun->getLength();
					low += pRun->getLength();
					pRun = pRun->getNext();
				}
				
				fp_TextRun * pPrevTR = NULL;
				
				while(pRun
					&& iLenToCopy > 0
					&& pRun->getType() == FPRUN_TEXT)
				{
					// in order not to loose formating, we will only replace
					// runs that can be merged in a single go	
					if(pPrevTR && !pPrevTR->canMergeWithNext())
						break;
					UT_uint32 iDiff = UT_MIN(pRun->getLength(), iLenToCopy);
					iLen += iDiff;
					iLenToCopy -= iDiff;
					pPrevTR = static_cast<fp_TextRun*>(pRun);
					pRun = pRun->getNext();
				}
				
				if(!iLen) // sequence of 0-len runs only
				{
					UT_DEBUGMSG(("fv_View::toggleCase: sequence of 0-width runs only, next run type %d\n", pRun ? pRun->getType():-1));
					continue;
				}
				
				UT_ASSERT(iLen + offset + pBL->getPosition(false) <= high);
				
				memmove(pTemp, buffer.getPointer(offset), iLen * sizeof(UT_UCSChar));
			
				switch (c)
				{
					case CASE_FIRST_CAPITAL:
						if(offset == 0)
							prev = NULL;
						else
							prev = buffer.getPointer(offset - 1);
						_toggleFirstCapital(pTemp,pTemp, iLen, prev);
						break;
						
					case CASE_SENTENCE:
#if 0
						if(offset < 2)
							prev = NULL;
						else
							prev = buffer.getPointer(offset - 2);
						_toggleSentence (pTemp, pTemp, iLen, prev);
#endif
                   {
					   UT_uint32 iOffset = offset;
					   UT_UCSChar * text = buffer.getPointer(0);
					   UT_uint32 i = 1;

					   // examine what preceedes this chunk of text
					   if(iOffset)
						   iOffset--;

					   while(iOffset && UT_UCS_isspace(text[iOffset]))
					   {
						   iOffset--;
					   }

					   bool bStartOfSentence = !iOffset;

					   if(iOffset)
					   {
						   bStartOfSentence = UT_UCS_isSentenceSeparator(text[iOffset]) && UT_UCS_isalpha(text[iOffset-1]);
					   }

					   if(bStartOfSentence)
					   {
						   i = 0;
						   while( i < iLen && UT_UCS_isspace(pTemp[i]))
							   i++;
						   
						   if(i < iLen)
						   {
							   pTemp[i] = UT_UCS_toupper (pTemp[i]);
							   i++;
						   }
					   }

					   for(; i < iLen; i++)
					   {
						   UT_ASSERT(i > 0);
						   while(i < iLen && !UT_UCS_isSentenceSeparator(pTemp[i]))
							   i++;

						   // now i is either out of bounds, or points to the separator
						   // if it points to the separator, check that the character before it
						   // is a letter if not, start all over again
						   if(i < iLen && !UT_UCS_isalpha(pTemp[i-1]))
							   continue;
							   
						   // move past the separator
						   i++;

						   if(i < iLen)
						   {
							   while( i < iLen && UT_UCS_isspace(pTemp[i]))
								   i++;

							   if(i < iLen)
								   pTemp[i] = UT_UCS_toupper(pTemp[i]);
						   }
					   }
				   }
				   break;
	
					case CASE_LOWER:
						_toggleLower (pTemp, pTemp, iLen);
						break;

					case CASE_UPPER:
						_toggleUpper (pTemp, pTemp, iLen);
						break;
#if 0
// see comments before _toggleTitle()
					case CASE_TITLE:
						_toggleTitle (pTemp, pTemp, iLen, _isSpaceBefore(low));
						break;
#endif
					case CASE_TOGGLE:
						_toggleToggle (pTemp, pTemp, iLen);
						break;

					default:
						UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				}

				// get the props after our segment
				pBL->getSpanAttrProp(offset+iLen,false,&pSpanAPAfter);
				xxx_UT_DEBUGMSG(("fv_View::toggleCase: delete/insert: low %d, iLen %d, pSpanAPAfter 0x%x, pSpanAPNow 0x%x\n", low, iLen,pSpanAPAfter,pSpanAPNow));
				
				bool bResult = m_pDoc->deleteSpan(low, low + iLen,NULL);
				UT_ASSERT(bResult);
				bResult = m_pDoc->insertSpan(low, pTemp, iLen, pSpanAPNow);
				
				// now remember the props for the next round
				pSpanAPNow = const_cast<PP_AttrProp*>(pSpanAPAfter);
				
				UT_ASSERT(bResult);
				low += iLen;
				offset += iLen;
			}
		}
		pBL = pBL->getNext();
		if ( pBL )
		  low = pBL->getPosition(false);
		else
		  break;
	}

	delete[] pTemp;
	m_pDoc->endUserAtomicGlob();
	_generalUpdate();
	_restorePieceTableState();
	if(origPos)
		_setPoint(origPos);
	_fixInsertionPointCoords();
	_drawInsertionPoint();
	
}

/*!
 * Goes through the document and reformats any paragraphs that need this.
 */
void FV_View::updateLayout(void)
{
	m_pLayout->updateLayout();
}

void FV_View::setPaperColor(const XML_Char* clr)
{
	UT_DEBUGMSG(("DOM: color is: %s\n", clr));

	const XML_Char * props[3];
	props[0] = "background-color";
	props[1] = clr;
	props[2] = 0;

	setSectionFormat(props);
	_eraseInsertionPoint();
	// update the screen
	_draw(0, 0, m_iWindowWidth, m_iWindowHeight, false, false);
	_fixInsertionPointCoords();
	_drawInsertionPoint();
}

void FV_View::focusChange(AV_Focus focus)
{
	m_focus=focus;
	switch(focus)
	{
	case AV_FOCUS_HERE:
		if (isSelectionEmpty() && (getPoint() > 0))
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
		m_pApp->rememberFocussedFrame( m_pParentData);
		break;
	case AV_FOCUS_NEARBY:
		if (isSelectionEmpty() && (getPoint() > 0))
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
		break;
	case AV_FOCUS_MODELESS:
		if (isSelectionEmpty() && (getPoint() > 0))
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
		break;
	case AV_FOCUS_NONE:
		if (isSelectionEmpty() && (getPoint() > 0))
		{
			_eraseInsertionPoint();
			_saveCurrentPoint();
		}
	}
}

FL_DocLayout* FV_View::getLayout() const
{
	return m_pLayout;
}

bool FV_View::notifyListeners(const AV_ChangeMask hint)
{
//
// No need to update stuff if we're in preview mode
//
	if(isPreview())
		return true;
//
// Sevior check if we need this?
// For some complex operations we can't update stuff in the middle.
// In particular insert headers/footers
//
//	if(!_shouldScreenUpdateOnGeneralUpdate())
//		return false;
	/*
	  IDEA: The view caches its change state as of the last notification,
	  to minimize noise from duplicate notifications.
	*/
	UT_ASSERT(hint != AV_CHG_NONE);
	AV_ChangeMask mask = hint;

	if (mask & AV_CHG_DO)
	{
		bool bUndo = canDo(true);
		bool bRedo = canDo(false);

		if ((m_chg.bUndo == bUndo) && (m_chg.bRedo == bRedo))
		{
			mask ^= AV_CHG_DO;
		}
		else
		{
			if (m_chg.bUndo != bUndo)
				m_chg.bUndo = bUndo;
			if (m_chg.bRedo != bRedo)
				m_chg.bRedo = bRedo;
		}
	}

	if (mask & AV_CHG_DIRTY)
	{
		bool bDirty = m_pDoc->isDirty();

		if (m_chg.bDirty != bDirty)
		{
			m_chg.bDirty = bDirty;
		}
		else
		{
			mask ^= AV_CHG_DIRTY;
		}
	}

	if (mask & AV_CHG_EMPTYSEL)
	{
		bool bSel = !isSelectionEmpty();

		if (m_chg.bSelection != bSel)
			m_chg.bSelection = bSel;
		else
			mask ^= AV_CHG_EMPTYSEL;
	}

	if (mask & AV_CHG_FILENAME)
	{
		// NOTE: we don't attempt to filter this
	}

	if (mask & AV_CHG_FMTBLOCK)
	{
		/*
		  The following brute-force solution works, but is atrociously
		  expensive, so we should avoid using it whenever feasible.
		*/
		const XML_Char ** propsBlock = NULL;
		getBlockFormat(&propsBlock);

		bool bMatch = false;

		if (propsBlock && m_chg.propsBlock)
		{
			bMatch = true;

			int i=0;

			while (bMatch)
			{
				if (!propsBlock[i] || !m_chg.propsBlock[i])
				{
					bMatch = (propsBlock[i] == m_chg.propsBlock[i]);
					break;
				}

				if (UT_stricmp(propsBlock[i], m_chg.propsBlock[i]))
				{
					bMatch = false;
					break;
				}

				i++;
			}
		}

		if (!bMatch)
		{
			FREEP(m_chg.propsBlock);
			m_chg.propsBlock = propsBlock;
		}
		else
		{
			FREEP(propsBlock);
			mask ^= AV_CHG_FMTBLOCK;
		}
	}

	if (mask & AV_CHG_FMTCHAR)
	{
		/*
		  The following brute-force solution works, but is atrociously
		  expensive, so we should avoid using it whenever feasible.

		  TODO: devise special case logic for (at minimum) char motion
		*/
		const XML_Char ** propsChar = NULL;
		getCharFormat(&propsChar);

		bool bMatch = false;

		if (propsChar && m_chg.propsChar)
		{
			bMatch = true;

			int i=0;

			while (bMatch)
			{
				if (!propsChar[i] || !m_chg.propsChar[i])
				{
					bMatch = (propsChar[i] == m_chg.propsChar[i]);
					break;
				}

				if (UT_stricmp(propsChar[i], m_chg.propsChar[i]))
				{
					bMatch = false;
					break;
				}

				i++;
			}
		}

		if (!bMatch)
		{
			FREEP(m_chg.propsChar);
			m_chg.propsChar = propsChar;
		}
		else
		{
			FREEP(propsChar);
			mask ^= AV_CHG_FMTCHAR;
		}
	}

	if (mask & AV_CHG_FMTSECTION)
	{
		/*
		  The following brute-force solution works, but is atrociously
		  expensive, so we should avoid using it whenever feasible.
		*/
		const XML_Char ** propsSection = NULL;
		getSectionFormat(&propsSection);

		bool bMatch = false;

		if (propsSection && m_chg.propsSection)
		{
			bMatch = true;

			int i=0;

			while (bMatch)
			{
				if (!propsSection[i] || !m_chg.propsSection[i])
				{
					bMatch = (propsSection[i] == m_chg.propsSection[i]);
					break;
				}

				if (UT_stricmp(propsSection[i], m_chg.propsSection[i]))
				{
					bMatch = false;
					break;
				}

				i++;
			}
		}

		if (!bMatch)
		{
			FREEP(m_chg.propsSection);
			m_chg.propsSection = propsSection;
		}
		else
		{
			FREEP(propsSection);
			mask ^= AV_CHG_FMTSECTION;
		}
	}

	if (mask & AV_CHG_FMTSTYLE)
	{
		// NOTE: we don't attempt to filter this
		// TODO: we probably should
	}

	if (mask & AV_CHG_PAGECOUNT)
	{
		// NOTE: we don't attempt to filter this
	}

	if (mask & AV_CHG_COLUMN)
	{
		// computing which column the cursor is in is rather expensive,
		// i'm not sure it's worth the effort here...

		fp_Run * pRun = NULL;
		UT_sint32 xCaret, yCaret;
		UT_uint32 heightCaret;
		UT_sint32 xCaret2, yCaret2;
		bool bDirection;
		_findPositionCoords(getPoint(), m_bPointEOL, xCaret, yCaret, xCaret2, yCaret2, heightCaret, bDirection, NULL, &pRun);

		//
		// Handle Headers/Footers This is a kludge for now
		//
		fp_Container * pContainer = NULL;
		if(pRun == NULL)
		{
			return false; // bail out
		}
		fl_BlockLayout * pBlock = pRun->getBlock();
		if(pBlock->getSectionLayout()->getType() == FL_SECTION_HDRFTR)
		{
			// General question for msevior: When is this triggered?
			// Usually we have an FL_SECTION_SHADOW. - PL
			// The answer:

			// That if was put in to guard against an extremely
			// convoluted crasher that occured once. I forget the
			// details and you really don't want to know. Something
			// like calling a _generalUpdate() from an insertStrux
			// while editting the first header and it had been
			// modified or something...

			if(m_bEditHdrFtr)
			{
				pContainer = m_pEditShadow->getFirstContainer();
			}
			else
				pContainer = pBlock->getSectionLayout()->getFirstContainer();
		}
		else
		{
			pContainer = pRun->getLine()->getContainer();
		}

		if (pContainer->getType() == FP_CONTAINER_COLUMN)
		{
			fp_Column* pColumn = (fp_Column*) pContainer;

			UT_uint32 nCol=0;
			fp_Column * pNthColumn = pColumn->getLeader();
			while (pNthColumn && (pNthColumn != pColumn))
			{
				nCol++;
				pNthColumn = pNthColumn->getFollower();
			}

			if (nCol != m_chg.iColumn)
			{
				m_chg.iColumn = nCol;
			}
			else
			{
				mask ^= AV_CHG_COLUMN;
			}
		}
		else if (pContainer->getType() == FP_CONTAINER_SHADOW)
		{
			// Hack the kludge:
			// Clearly you can't change columns while editing a header. -PL
			mask ^= AV_CHG_COLUMN;
		}
	}

	// base class does the rest
	return AV_View::notifyListeners(mask);
}

/*!
  Reverse the direction of the current selection
  Does so without changing the screen.
*/
void FV_View::_swapSelectionOrientation(void)
{
	UT_ASSERT(!isSelectionEmpty());
	_fixInsertionPointCoords();
	PT_DocPosition curPos = getPoint();
	UT_ASSERT(curPos != m_iSelectionAnchor);
	_setPoint(m_iSelectionAnchor);
	m_iSelectionAnchor = curPos;
}

/*!
  Move point to requested end of selection and clear selection
  \param bForward True if point should be moved to the forward position

  \note Do not draw the insertion point after clearing the
		selection.
  \fixme BIDI broken?
*/
void FV_View::_moveToSelectionEnd(bool bForward)
{
	UT_ASSERT(!isSelectionEmpty());
	
	PT_DocPosition curPos = getPoint();
	_fixInsertionPointCoords();
	UT_ASSERT(curPos != m_iSelectionAnchor);
	bool bForwardSelection = (m_iSelectionAnchor < curPos);
	
	if (bForward != bForwardSelection)
	{
		_swapSelectionOrientation();
	}

	_clearSelection();

	return;
}

void FV_View::_eraseSelection(void)
{
	_fixInsertionPointCoords();
	if (!m_bSelection)
	{
		_resetSelection();
		return;
	}

	UT_uint32 iPos1, iPos2;

	if (m_iSelectionAnchor < getPoint())
	{
		iPos1 = m_iSelectionAnchor;
		iPos2 = getPoint();
	}
	else
	{
		iPos1 = getPoint();
		iPos2 = m_iSelectionAnchor;
	}

	_clearBetweenPositions(iPos1, iPos2, true);
}

void FV_View::_clearSelection(void)
{
	_fixInsertionPointCoords();
	if (!m_bSelection)
	{
		_resetSelection();
		return;
	}

	UT_uint32 iPos1, iPos2;

	if (m_iSelectionAnchor < getPoint())
	{
		iPos1 = m_iSelectionAnchor;
		iPos2 = getPoint();
	}
	else
	{
		iPos1 = getPoint();
		iPos2 = m_iSelectionAnchor;
	}

	bool bres = _clearBetweenPositions(iPos1, iPos2, true);
	if(!bres)
		return;
	_resetSelection();

	_drawBetweenPositions(iPos1, iPos2);
}

void FV_View::_resetSelection(void)
{
	m_bSelection = false;
	m_iSelectionAnchor = getPoint();
}

void FV_View::cmdUnselectSelection(void)
{
	_clearSelection();
}

void FV_View::_drawSelection()
{
	UT_ASSERT(!isSelectionEmpty());

	if (m_iSelectionAnchor < getPoint())
	{
		_drawBetweenPositions(m_iSelectionAnchor, getPoint());
	}
	else
	{
		_drawBetweenPositions(getPoint(), m_iSelectionAnchor);
	}
}

void FV_View::_setSelectionAnchor(void)
{
	m_bSelection = true;
	m_iSelectionAnchor = getPoint();
}

void FV_View::_deleteSelection(PP_AttrProp *p_AttrProp_Before)
{
	// delete the current selection.
	// NOTE: this must clear the selection.

	UT_ASSERT(!isSelectionEmpty());

	PT_DocPosition iPoint = getPoint();
	UT_ASSERT(iPoint != m_iSelectionAnchor);

	UT_uint32 iSelAnchor = m_iSelectionAnchor;
	
	_eraseSelection();
	_resetSelection();

	bool bForward = (iPoint < iSelAnchor);
	if (bForward)
	{
		m_pDoc->deleteSpan(iPoint, iSelAnchor, p_AttrProp_Before);
	}
	else
	{
		m_pDoc->deleteSpan(iSelAnchor, iPoint, p_AttrProp_Before);
	}
//
// Can't leave list-tab on a line
//
	if(isTabListAheadPoint() == true)
	{
		m_pDoc->deleteSpan(getPoint(), getPoint()+2, p_AttrProp_Before);
	}
}

bool FV_View::isSelectionEmpty(void) const
{
	if (!m_bSelection)
	{
		return true;
	}
	
	PT_DocPosition curPos = getPoint();
	if (curPos == m_iSelectionAnchor)
	{
		return true;
	}

	return false;
}

PT_DocPosition FV_View::_getDocPos(FV_DocPos dp, bool bKeepLooking)
{
	return _getDocPosFromPoint(getPoint(),dp,bKeepLooking);
}

PT_DocPosition FV_View::_getDocPosFromPoint(PT_DocPosition iPoint, FV_DocPos dp, bool bKeepLooking)
{
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
	UT_sint32 xPoint2;
	UT_sint32 yPoint2;
	bool bDirection;

	PT_DocPosition iPos;

	// this gets called from ctor, so get out quick
	if (dp == FV_DOCPOS_BOD)
	{
		bool bRes = getEditableBounds(false, iPos);
		UT_ASSERT(bRes);
		
		return iPos;
	}

	// TODO: could cache these to save a lookup if point doesn't change
	fl_BlockLayout* pBlock = _findBlockAtPosition(iPoint);
	fp_Run* pRun = pBlock->findPointCoords(iPoint, m_bPointEOL, xPoint,
										   yPoint, xPoint2, yPoint2,
										   iPointHeight, bDirection);

	fp_Line* pLine = pRun->getLine();

	// be pessimistic
	iPos = iPoint;

	switch (dp)
	{
	case FV_DOCPOS_BOL:
	{
		fp_Run* pFirstRun = pLine->getFirstRun();
		
		iPos = pFirstRun->getBlockOffset() + pBlock->getPosition();
	}
	break;
	
	case FV_DOCPOS_EOL:
	{
		// Ignore forced breaks and EOP when finding EOL.
		fp_Run* pLastRun = pLine->getLastRun();
		while (!pLastRun->isFirstRunOnLine() 
			   && (pLastRun->isForcedBreak()
				   || (FPRUN_ENDOFPARAGRAPH == pLastRun->getType())))
		{
			pLastRun = pLastRun->getPrev();
		}
		
		if (pLastRun->isForcedBreak()
			|| (FPRUN_ENDOFPARAGRAPH == pLastRun->getType()))
		{
			iPos = pBlock->getPosition() + pLastRun->getBlockOffset();
		}
		else
		{
			iPos = pBlock->getPosition() + pLastRun->getBlockOffset() + pLastRun->getLength();
		}
	}
	break;

	case FV_DOCPOS_EOD:
	{
		bool bRes = getEditableBounds(true, iPos);
		UT_ASSERT(bRes);
	}
	break;

	case FV_DOCPOS_BOB:
	{
#if 1

// DOM: This used to be an #if 0. I changed it to #if 1
// DOM: because after enabling this code, I can no
// DOM: longer reproduce bug 403 (the bug caused by this
// DOM: code being if 0'd) or bug 92 (the bug that if 0'ing
// DOM: this code supposedly fixes)

// TODO this piece of code attempts to go back
// TODO to the previous block if we are on the
// TODO edge.  this causes bug #92 (double clicking
// TODO on the first line of a paragraph selects
// TODO current paragraph and the previous paragraph).
// TODO i'm not sure why it is here.
// TODO
// TODO it's here because it makes control-up-arrow
// TODO when at the beginning of paragraph work. this
// TODO problem is logged as bug #403.
// TODO
		// are we already there?
		if (iPos == pBlock->getPosition())
		{
			// yep.  is there a prior block?
			if (!pBlock->getPrevBlockInDocument())
				break;

			// yep.  look there instead
			pBlock = pBlock->getPrevBlockInDocument();
		}
#endif /* 0 */
			
		iPos = pBlock->getPosition();
	}
	break;

	case FV_DOCPOS_EOB:
	{
		if (pBlock->getNextBlockInDocument())
		{
			// BOB for next block
			pBlock = pBlock->getNextBlockInDocument();
			iPos = pBlock->getPosition();
		}
		else
		{
			// EOD
			bool bRes = getEditableBounds(true, iPos);
			UT_ASSERT(bRes);
		}
	}
	break;
	
	case FV_DOCPOS_BOW:
	{
		UT_GrowBuf pgb(1024);
		
		bool bRes = pBlock->getBlockBuf(&pgb);
		UT_ASSERT(bRes);
		
		const UT_UCSChar* pSpan = pgb.getPointer(0);
		
		UT_ASSERT(iPos >= pBlock->getPosition());
		UT_uint32 offset = iPos - pBlock->getPosition();
		UT_ASSERT(offset <= pgb.getLength());
		
		if (offset == 0)
		{
			if (!bKeepLooking)
				break;
			
			// is there a prior block?
			pBlock = pBlock->getPrevBlockInDocument();
			
			if (!pBlock)
				break;
			
			// yep.  look there instead
			pgb.truncate(0);
			bRes = pBlock->getBlockBuf(&pgb);
			UT_ASSERT(bRes);
			
			pSpan = pgb.getPointer(0);
			offset = pgb.getLength();
			
			if (offset == 0)
			{
				iPos = pBlock->getPosition();
				break;
			}
		}

		UT_uint32 iUseOffset = bKeepLooking ? offset-1 : offset;
		
		bool bInWord = !UT_isWordDelimiter(pSpan[iUseOffset], UCS_UNKPUNK, iUseOffset > 0 ? pSpan[iUseOffset - 1] : UCS_UNKPUNK);
		
		for (offset--; offset > 0; offset--)
		{
			if (UT_isWordDelimiter(pSpan[offset], UCS_UNKPUNK,pSpan[offset-1]))
			{
				if (bInWord)
					break;
			}
			else
				bInWord = true;
		}
		
		if ((offset > 0) && (offset < pgb.getLength()))
			offset++;
		
		iPos = offset + pBlock->getPosition();
	}
	break;
	
	case FV_DOCPOS_EOW_MOVE:
	{
		UT_GrowBuf pgb(1024);
		
		bool bRes = pBlock->getBlockBuf(&pgb);
		UT_ASSERT(bRes);
		
		const UT_UCSChar* pSpan = pgb.getPointer(0);
		
		UT_ASSERT(iPos >= pBlock->getPosition());
		UT_uint32 offset = iPos - pBlock->getPosition();
		UT_ASSERT(offset <= pgb.getLength());
						
		if (offset == pgb.getLength())
		{
			if (!bKeepLooking)
				break;
			
			// is there a next block?
			pBlock = pBlock->getNextBlockInDocument();
			
			if (!pBlock)
				break;
			
			// yep.  look there instead
			pgb.truncate(0);
			bRes = pBlock->getBlockBuf(&pgb);
			UT_ASSERT(bRes);

			pSpan = pgb.getPointer(0);
			offset = 0;
			
			if (pgb.getLength() == 0)
			{
				iPos = pBlock->getPosition();
				break;
			}
		}
		
		bool bBetween = UT_isWordDelimiter(pSpan[offset], UCS_UNKPUNK, offset > 0 ? pSpan[offset - 1] : UCS_UNKPUNK);
		
		// Needed so ctrl-right arrow will work
		// This is the code that was causing bug 10
		// There is still some weird behavior that should be investigated
		
		for (; offset < pgb.getLength(); offset++)
		{
			UT_UCSChar followChar, prevChar;
			
			followChar = ((offset + 1) < pgb.getLength()) ? pSpan[offset+1] : UCS_UNKPUNK;
			prevChar = offset > 0 ? pSpan[offset - 1] : UCS_UNKPUNK;
			
			if (!UT_isWordDelimiter(pSpan[offset], followChar, prevChar))
				break;
		}
		
		for (; offset < pgb.getLength(); offset++)
		{
			UT_UCSChar followChar, prevChar;
			
			followChar = ((offset + 1) < pgb.getLength()) ? pSpan[offset+1] : UCS_UNKPUNK;
			prevChar = offset > 0 ? pSpan[offset - 1] : UCS_UNKPUNK;
			
			if (!UT_isWordDelimiter(pSpan[offset], followChar, prevChar))
			{
				if (bBetween)
				{
					break;
				}
			}
			else if (pSpan[offset] != ' ')
			{
				break;
			}
			else
			{
				bBetween = true;
			}
		}
		
		iPos = offset + pBlock->getPosition();
	}
	break;
	
	case FV_DOCPOS_EOW_SELECT:
	{
		UT_GrowBuf pgb(1024);
		
		bool bRes = pBlock->getBlockBuf(&pgb);
		UT_ASSERT(bRes);
		
		const UT_UCSChar* pSpan = pgb.getPointer(0);
		
		UT_ASSERT(iPos >= pBlock->getPosition());
		UT_uint32 offset = iPos - pBlock->getPosition();
		UT_ASSERT(offset <= pgb.getLength());
		
		if (offset == pgb.getLength())
		{
			if (!bKeepLooking)
				break;
			
			// is there a next block?
			pBlock = pBlock->getNextBlockInDocument();
			
			if (!pBlock)
				break;

			// yep.  look there instead
			pgb.truncate(0);
			bRes = pBlock->getBlockBuf(&pgb);
			UT_ASSERT(bRes);

			pSpan = pgb.getPointer(0);
			offset = 0;

			if (pgb.getLength() == 0)
			{
				iPos = pBlock->getPosition();
				break;
			}
		}

		bool bBetween = UT_isWordDelimiter(pSpan[offset], UCS_UNKPUNK, offset > 0 ? pSpan[offset - 1] : UCS_UNKPUNK);
			
		// Needed so ctrl-right arrow will work
		// This is the code that was causing bug 10
		// There is still some weird behavior that should be investigated
		/*
		  for (; offset < pgb.getLength(); offset++)
		  {
		  if (!UT_isWordDelimiter(pSpan[offset]))
		  break;
		  }
		*/
		for (; offset < pgb.getLength(); offset++)
		{
			UT_UCSChar followChar, prevChar;
			
			followChar = ((offset + 1) < pgb.getLength()) ? pSpan[offset+1] : UCS_UNKPUNK;
			prevChar = offset > 0 ? pSpan[offset - 1] : UCS_UNKPUNK;
			
			if (UT_isWordDelimiter(pSpan[offset], followChar, prevChar))
			{
				if (bBetween)
					break;
			}
			else if (pSpan[offset] == ' ')
				break;
			else
				bBetween = true;
		}

		iPos = offset + pBlock->getPosition();
	}
	break;


	case FV_DOCPOS_BOP: 
	{
		fp_Container* pContainer = pLine->getContainer();
		fp_Page* pPage = pContainer->getPage();

		iPos = pPage->getFirstLastPos(true);
	}
	break;

	case FV_DOCPOS_EOP:
	{
		fp_Container* pContainer = pLine->getContainer();
		fp_Page* pPage = pContainer->getPage();

		iPos = pPage->getFirstLastPos(false);
	}
	break;

	case FV_DOCPOS_BOS: 
	case FV_DOCPOS_EOS:
		UT_ASSERT(UT_TODO);
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return iPos;
}

void FV_View::moveInsPtTo(FV_DocPos dp, bool bClearSelection)
{
	if(bClearSelection)
	{
		if (!isSelectionEmpty())
			_clearSelection();
		else
			_eraseInsertionPoint();
	}
	
	
	PT_DocPosition iPos = _getDocPos(dp);

	if (iPos != getPoint())
	{
		bool bPointIsValid = (getPoint() >= _getDocPos(FV_DOCPOS_BOD));
		if (bPointIsValid)
			_clearIfAtFmtMark(getPoint());
	}

	_setPoint(iPos, (dp == FV_DOCPOS_EOL));

//
// Check we have a layout defined first. On startup we don't
//
	if(getLayout()->getFirstSection())
	{
		_ensureThatInsertionPointIsOnScreen();
		notifyListeners(AV_CHG_MOTION);
	}
}

void FV_View::moveInsPtTo(PT_DocPosition dp)
{
	if (dp != getPoint())
		_clearIfAtFmtMark(getPoint());

	_setPoint(dp, /* (dp == FV_DOCPOS_EOL) */ false);	// is this bool correct?
	/*TF CHANGE: Why do we automatically scroll?  We should only scroll the window
	  if the point to be displayed is not already on the screen.  If it
	  is already on the screen then we should just leave it in place and
	  not do any scrolling.  Instead of the code below, we use the code which
	  is already in the _ensureThatInsertionPointIsOnScreen() function.
	  _fixInsertionPointCoords();
	  cmdScroll(AV_SCROLLCMD_LINEDOWN, (UT_uint32) (m_yPoint + m_iPointHeight/2 - m_iWindowHeight/2));
	  cmdScroll(AV_SCROLLCMD_LINERIGHT, (UT_uint32) (m_xPoint - m_iWindowWidth/2));
	  notifyListeners(AV_CHG_MOTION);
	*/
	_ensureThatInsertionPointIsOnScreen(false);
}


/*!
  Move point a number of character positions
  \param bForward True if moving forward
  \param count Number of char positions to move

  \note Cursor movement while there's a selection has the effect of
		clearing the selection. And only that. See bug 993.
*/
void FV_View::cmdCharMotion(bool bForward, UT_uint32 count)
{
	if (!isSelectionEmpty())
	{
		_moveToSelectionEnd(bForward);
		_drawInsertionPoint();
		return;
	}

	PT_DocPosition iPoint = getPoint();
	if (!_charMotion(bForward, count))
	{
		_setPoint(iPoint);
	}
	else
	{
		PT_DocPosition iPoint1 = getPoint();
		if ( iPoint1 == iPoint )
		{
			if(!_charMotion(bForward, count))
			{
				_eraseInsertionPoint();
				_setPoint(iPoint);
				notifyListeners(AV_CHG_MOTION);
				return;
			}
		}
		_updateInsertionPoint();
	}
//
// No need to update screen for 1 seconds.
//
	m_pLayout->setSkipUpdates(2);
	notifyListeners(AV_CHG_MOTION);
	
}

/*!
  Find block at document position. This version is looks outside the 
  header region if we get a null block.
  \param pos Document position
  \return Block at specified posistion, or the first block to the
		  rigth of that position. May return NULL.
  \see m_pLayout->findBlockAtPosition 
*/
fl_BlockLayout* FV_View::_findBlockAtPosition(PT_DocPosition pos) const
{
	fl_BlockLayout * pBL=NULL;
	if(m_bEditHdrFtr && m_pEditShadow != NULL)
	{
		pBL = m_pEditShadow->findBlockAtPosition(pos);
		if(pBL != NULL)
			return pBL;
	}
	pBL = m_pLayout->findBlockAtPosition(pos);
	UT_ASSERT(pBL);
	if(!pBL)
		return NULL;
	
//
// Sevior should remove this after a while..
//
#if(1)
	if(pBL->isHdrFtr())
	{
//		  fl_HdrFtrSectionLayout * pSSL = (fl_HdrFtrSectionLayout *) pBL->getSectionLayout();
//		  pBL = pSSL->getFirstShadow()->findMatchingBlock(pBL);
		  UT_DEBUGMSG(("<<<<SEVIOR>>>: getfirstshadow in view \n"));
		  UT_ASSERT(0);
	}
#endif
	return pBL;
}

UT_uint32 FV_View::getCurrentPageNumber(void)
{
	UT_sint32 iPageNum = 0;
	PT_DocPosition pos = getPoint();
	fl_BlockLayout * pBlock = _findBlockAtPosition(pos);
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
	UT_sint32 xPoint2;
	UT_sint32 yPoint2;
	bool bDirection;
	fp_Run* pRun = pBlock->findPointCoords(pos, m_bPointEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);
	fp_Line * pLine = pRun->getLine();
	if (pLine && pLine->getContainer() && pLine->getContainer()->getPage())
	{
		fp_Page* pPage = pLine->getContainer()->getPage();
		FL_DocLayout* pDL = pPage->getDocLayout();

		UT_uint32 iNumPages = pDL->countPages();
		for (UT_uint32 i=0; i<iNumPages; i++)
		{
			fp_Page* pPg = pDL->getNthPage(i);

			if (pPg == pPage)
			{
				iPageNum = i + 1;
				break;
			}
		}
	}
	else
	{
		iPageNum = 0;
	}
	return iPageNum;
}

bool FV_View::cmdCharInsert(UT_UCSChar * text, UT_uint32 count, bool bForce)
{
	bool bResult = true;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	// Turn off list updates
	m_pDoc->disableListUpdates();

	if (!isSelectionEmpty())
	{
	        m_pDoc->beginUserAtomicGlob();
		PP_AttrProp AttrProp_Before;
		_deleteSelection(&AttrProp_Before);
		bResult = m_pDoc->insertSpan(getPoint(), text, count, &AttrProp_Before);
		m_pDoc->endUserAtomicGlob();
	}
	else
	{
		_eraseInsertionPoint();

		bool bOverwrite = (!m_bInsertMode && !bForce);

		if (bOverwrite)
		{
			// we need to glob when overwriting
			m_pDoc->beginUserAtomicGlob();
			cmdCharDelete(true,count);
		}
		bool doInsert = true;
		if(text[0] == UCS_TAB && count == 1)
		{
			//
			// Were inserting a TAB. Handle special case of TAB
			// right after a list-label combo
			//
			if((isTabListBehindPoint() == true || isTabListAheadPoint() == true) && getCurrentBlock()->isFirstInList() == false)
			{
				//
				// OK now start a sublist of the same type as the
				// current list if the list type is of numbered type
				fl_BlockLayout * pBlock = getCurrentBlock();
				List_Type curType = pBlock->getListType();
//
// Now increase list level for bullet lists too
//
				{
					UT_uint32 curlevel = pBlock->getLevel();
					UT_uint32 currID = pBlock->getAutoNum()->getID();
					curlevel++;
					fl_AutoNum * pAuto = pBlock->getAutoNum();
					const XML_Char * pszAlign = pBlock->getProperty("margin-left",true);
					const XML_Char * pszIndent = pBlock->getProperty("text-indent",true);
					const XML_Char * pszFieldF = pBlock->getProperty("field-font",true);
					float fAlign = (float)atof(pszAlign);
					float fIndent = (float)atof(pszIndent);
//
// Convert pixels to inches.
//
					float maxWidthIN = (float)(((float) pBlock->getFirstLine()->getContainer()->getWidth())/100. -0.6);
					if(fAlign + (float) LIST_DEFAULT_INDENT < maxWidthIN)
					{
						fAlign += (float) LIST_DEFAULT_INDENT;
					}
					pBlock->StartList(curType,pAuto->getStartValue32(),pAuto->getDelim(),pAuto->getDecimal(),pszFieldF,fAlign,fIndent, currID,curlevel);
					doInsert = false;
				}
			}
		}
		if (doInsert == true)
		{
			bResult = m_pDoc->insertSpan(getPoint(), text, count);
			if(!bResult)
			{
				const fl_BlockLayout * pBL = getCurrentBlock();
				const PP_AttrProp *pBlockAP = NULL;
				pBL->getAttrProp(&pBlockAP);
				bResult = m_pDoc->insertSpan(getPoint(), text, count,const_cast<PP_AttrProp *>(pBlockAP));
				UT_ASSERT(bResult);
			}
		}

		if (bOverwrite)
		{
			m_pDoc->endUserAtomicGlob();
		}
	}

	_generalUpdate();


	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_ensureThatInsertionPointIsOnScreen();
	return bResult;
}


void FV_View::insertSectionBreak(BreakSectionType type)
{
	// if Type = 0 "continuous" section break
	// if Type = 1 "next page" section break
	// if Type = 2 "even page" section break
	// if Type = 3 "odd page" section break


	// Signal PieceTable Changes have Started
	m_pDoc->notifyPieceTableChangeStart();

	UT_UCSChar c = UCS_FF;
	UT_uint32 iPageNum = 0;
	switch(type)
	{
	case BreakSectionContinuous:
		_insertSectionBreak();
		break;
	case BreakSectionNextPage:
		m_pDoc->beginUserAtomicGlob();
		_insertSectionBreak();
		cmdCharInsert(&c,1);
		m_pDoc->endUserAtomicGlob();
		break;
	case BreakSectionEvenPage:
		m_pDoc->beginUserAtomicGlob();
		cmdCharInsert(&c,1);
		iPageNum = getCurrentPageNumber();
		if( (iPageNum & 1) == 1)
		{
			_insertSectionBreak();
			cmdCharInsert(&c,1);
		}
		else
		{
			_insertSectionBreak();
		}
		m_pDoc->endUserAtomicGlob();
		break;
	case BreakSectionOddPage:
		m_pDoc->beginUserAtomicGlob();
		cmdCharInsert(&c,1);
		iPageNum = getCurrentPageNumber();
		if( (iPageNum & 1) == 0)
		{
			_insertSectionBreak();
			cmdCharInsert(&c,1);
		}
		else
		{
			_insertSectionBreak();
		}
		m_pDoc->endUserAtomicGlob();
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	// Signal PieceTable Changes have Started
	m_pDoc->notifyPieceTableChangeEnd();
	m_iPieceTableState = 0;
}

void FV_View::insertSectionBreak(void)
{
	m_pDoc->beginUserAtomicGlob();

	// Signal PieceTable Changes have Started
	m_pDoc->notifyPieceTableChangeStart();

	_insertSectionBreak();

	// Signal PieceTable Changes have ended
	m_pDoc->notifyPieceTableChangeEnd();
	m_iPieceTableState = 0;
	m_pDoc->endUserAtomicGlob();
}


void FV_View::_insertSectionBreak(void)
{
	if (!isSelectionEmpty())
	{
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}
	//
	// Get preview DocSectionLayout so we know what header/footers we have 
		// to insert here.
	//
	fl_DocSectionLayout * pPrevDSL = (fl_DocSectionLayout *) getCurrentBlock()->getSectionLayout();

	// insert a new paragraph with the same attributes/properties
	// as the previous (or none if the first paragraph in the section).
	// before inserting a section break, we insert a block break
	UT_uint32 iPoint = getPoint();
	
	m_pDoc->insertStrux(iPoint, PTX_Block);
	m_pDoc->insertStrux(iPoint, PTX_Section);
		  
	_generalUpdate();
	_ensureThatInsertionPointIsOnScreen();
	UT_uint32 oldPoint = getPoint();
	fl_DocSectionLayout * pCurDSL = (fl_DocSectionLayout *) getCurrentBlock()->getSectionLayout();
	//
	// Duplicate previous header/footers for this section.
	//
	UT_Vector vecPrevHdrFtr;
	pPrevDSL->getVecOfHdrFtrs( &vecPrevHdrFtr);
	UT_uint32 i =0;
	const XML_Char* block_props[] = {
		"text-align", "left",
		NULL, NULL
	};
	HdrFtrType hfType;
	fl_HdrFtrSectionLayout * pHdrFtrSrc = NULL;
	fl_HdrFtrSectionLayout * pHdrFtrDest = NULL;
	for(i=0; i< vecPrevHdrFtr.getItemCount(); i++)
	{
		  pHdrFtrSrc = (fl_HdrFtrSectionLayout *) vecPrevHdrFtr.getNthItem(i);	
		  hfType = pHdrFtrSrc->getHFType();
		  insertHeaderFooter(block_props, hfType, pCurDSL); // cursor is now in the header/footer
		  if(hfType == FL_HDRFTR_HEADER)
		  {
			  pHdrFtrDest = pCurDSL->getHeader();
		  }
		  else if(hfType == FL_HDRFTR_FOOTER)
		  {
			  pHdrFtrDest = pCurDSL->getFooter();
		  }
		  else if(hfType == FL_HDRFTR_HEADER_FIRST)
		  {
			  pHdrFtrDest = pCurDSL->getHeaderFirst();
		  }
		  else if( hfType == FL_HDRFTR_HEADER_EVEN)
		  {
			  pHdrFtrDest = pCurDSL->getHeaderEven();
		  }
		  else if( hfType == FL_HDRFTR_HEADER_LAST)
		  {
			  pHdrFtrDest = pCurDSL->getHeaderLast();
		  }
		  else if(hfType == FL_HDRFTR_FOOTER_FIRST)
		  {
			  pHdrFtrDest = pCurDSL->getFooterFirst();
		  }
		  else if( hfType == FL_HDRFTR_FOOTER_EVEN)
		  {
			  pHdrFtrDest = pCurDSL->getFooterEven();
		  }
		  else if( hfType == FL_HDRFTR_FOOTER_LAST)
		  {
			  pHdrFtrDest = pCurDSL->getFooterLast();
		  }
		  _populateThisHdrFtr(pHdrFtrSrc,pHdrFtrDest);
	}

	_setPoint(oldPoint);	  
	_generalUpdate();

	_ensureThatInsertionPointIsOnScreen();
}

bool FV_View::isCurrentListBlockEmpty(void)
{
	//
	// If the current block is a list and is otherwise empty return true
	//
	fl_BlockLayout * pBlock = getCurrentBlock();
	fl_BlockLayout * nBlock = pBlock->getNext();
	bool bEmpty = true;
	if(pBlock->isListItem() == false || (nBlock!= NULL && nBlock->isListItem()==true))
	{
		return false;
	}
	
	//
	// Now look to see if the current block is otherwise empty
	//
	fp_Run * pRun = pBlock->getFirstRun();
	UT_uint32 ifield =0;
	while((bEmpty == true) && (pRun != NULL))
	{
		FP_RUN_TYPE runtype = (FP_RUN_TYPE) pRun->getType();	
		if((runtype == FPRUN_TAB) ||
		   ( runtype == FPRUN_FIELD)  ||
		   (runtype == FPRUN_FMTMARK) ||
			(runtype == FPRUN_ENDOFPARAGRAPH))
		{
			if(runtype == FPRUN_FIELD)
			{
				ifield++;
				if(ifield > 1)
				{
					bEmpty = false;
					break;
				}
			}
			pRun = pRun->getNext();
		}
		else
		{
			bEmpty = false;
		}
	}
	return bEmpty;
}

bool FV_View::isPointBeforeListLabel(void)
{
	//
	// If the current point is in a list block and the point is before the list label
	// return true
	//
	fl_BlockLayout * pBlock = getCurrentBlock();
	bool bBefore = true;
	if(pBlock->isListItem() == false)
	{
		return false;
	}
	
	//
	// Now look to see if the point is before the list label
	//
	PT_DocPosition pos = getPoint();
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
	UT_sint32 xPoint2;
	UT_sint32 yPoint2;
	bool   bDirection;
	
	fp_Run* pRun = pBlock->findPointCoords(pos, m_bPointEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);
	pRun = pRun->getPrev();
	while(pRun != NULL && bBefore == true)
	{
		if(pRun->getType()== FPRUN_FIELD)
		{
			fp_FieldRun * pFRun = (fp_FieldRun *) pRun;
			if (pFRun->getFieldType() == FPFIELD_list_label)
			{
				bBefore = false;
			}
		}
		pRun = pRun->getPrev();
	}
	return bBefore;
}

void FV_View::processSelectedBlocks(List_Type listType)
{
	//
	// Update Lists in the selected region
	//

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

//
// Turn off cursor
//
	if(isSelectionEmpty())
		_eraseInsertionPoint();

	UT_Vector vBlock;
	getBlocksInSelection( &vBlock);
	UT_uint32 i;
	m_pDoc->disableListUpdates();

	m_pDoc->beginUserAtomicGlob();

	char margin_left [] = "margin-left";
#ifdef BIDI_ENABLED	
	char margin_right[] = "margin-right";
#endif
	for(i=0; i< vBlock.getItemCount(); i++)
	{
		fl_BlockLayout * pBlock =  (fl_BlockLayout *) vBlock.getNthItem(i);
		PL_StruxDocHandle sdh = pBlock->getStruxDocHandle();
		if(pBlock->isListItem() == true)
		{
			m_pDoc->StopList(sdh);
		}
		else
		{
			fl_BlockLayout * pPrev = pBlock->getPrev();
//
// Only attach block to previous list if the margin of the current block < the 
// previous block.
//
			double prevLeft = 0.0;
			double blockLeft = 0.0;
			if(pPrev != NULL)
			{
#ifdef BIDI_ENABLED
				prevLeft = pPrev->getDominantDirection() == FRIBIDI_TYPE_LTR 
				  ? UT_convertToInches(pPrev->getProperty(margin_left,true)) 
				  : UT_convertToInches(pPrev->getProperty(margin_right,true));

				blockLeft = pBlock->getDominantDirection() == FRIBIDI_TYPE_LTR 
				  ? UT_convertToInches(pBlock->getProperty(margin_left,true)) 
				  : UT_convertToInches(pBlock->getProperty(margin_right,true));;
#else
				prevLeft = UT_convertToInches(pPrev->getProperty(margin_left,true));
				blockLeft = UT_convertToInches(pBlock->getProperty(margin_left,true));
#endif
			}
			if(pBlock->isListItem()== NULL && pPrev != NULL && pPrev->isListItem()== true && pPrev->getAutoNum()->getType() == listType && (blockLeft <= (prevLeft - 0.00001)))
			{
				pBlock->resumeList(pPrev);
			}			
			else if(pBlock->isListItem()== NULL)
			{
				XML_Char* cType = pBlock->getListStyleString(listType);
				pBlock->StartList(cType);
			}
		}
	}
	m_pDoc->endUserAtomicGlob();

	// closes bug # 1255 - unselect a list after creation
	cmdUnselectSelection();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	_generalUpdate();

	// Signal piceTable is stable again
	_restorePieceTableState();
	if (isSelectionEmpty())
	{
		_ensureThatInsertionPointIsOnScreen();
	}
}


void FV_View::getBlocksInSelection( UT_Vector * vBlock)
{
	PT_DocPosition startpos = getPoint();
	PT_DocPosition endpos = startpos;
	if(isSelectionEmpty())
	{
		vBlock->addItem(getCurrentBlock());
		return;
	}
	if (m_iSelectionAnchor > startpos)
	{
		endpos = m_iSelectionAnchor;
	}
	else
	{
		startpos =	m_iSelectionAnchor;
	}
	fl_BlockLayout * pBlock = _findBlockAtPosition(startpos);
	while( pBlock != NULL && pBlock->getPosition() <= endpos)
	{
		vBlock->addItem(pBlock);
		pBlock = pBlock->getNext();
	}
	return;
}

void FV_View::insertParagraphBreak(void)
{
	bool bDidGlob = false;
	bool bBefore = false;
	bool bStopList = false;

	m_pDoc->beginUserAtomicGlob();

	// Prevent access to Piecetable for things like spellchecks until
	// paragraphs have stablized
	//
	_saveAndNotifyPieceTableChange();

	if (!isSelectionEmpty())
	{
		bDidGlob = true;
		//	m_pDoc->beginUserAtomicGlob();
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	// insert a new paragraph with the same attributes/properties
	// as the previous (or none if the first paragraph in the section).
	//
	// But first check to see if we're in a list and the current block is
	// otherwise blank.
	//
	m_pDoc->disableListUpdates();
	fl_BlockLayout * pBlock = getCurrentBlock();
	PL_StruxDocHandle sdh = pBlock->getStruxDocHandle();
//
// If we're at the end of the block set new style to followed-by. Look for this
// condition before we do insertStrux
//
	bool bAtEnd = false;
	PT_DocPosition posEOD = 0;
	getEditableBounds(true, posEOD);
	if(getPoint() != posEOD)
	{
		bAtEnd = _findBlockAtPosition(getPoint()+1) != _findBlockAtPosition(getPoint());
	}
	else
	{
		bAtEnd = true;
	}
	if(isCurrentListBlockEmpty() == true)
	{
		m_pDoc->StopList(sdh);
		bStopList = true;
	}
	else if(isPointBeforeListLabel() == true)
	{
		//
		// Now deal with the case of entering a line before a list label
		// We flag were entering a new line and delete the current list label. After the we
		// insert the line break (which automatically write a new list label) we stop the list
		// in preceding block.
		//
		bBefore = true;
		pBlock->deleteListLabel();
	}
	if(bStopList == false)
		m_pDoc->insertStrux(getPoint(), PTX_Block);
	if(bBefore == true)
	{
		fl_BlockLayout * pPrev = getCurrentBlock()->getPrev();
		sdh = pPrev->getStruxDocHandle();
		m_pDoc->StopList(sdh);
		_setPoint(getCurrentBlock()->getPosition());
	}

	const XML_Char* style = NULL;
	PD_Style* pStyle = NULL;
	if(getStyle(&style) && bAtEnd)
	{
		m_pDoc->getStyle((char*) style, &pStyle);
		if(pStyle != NULL  && !bBefore)
		{
			const XML_Char* szFollow = NULL;
			pStyle->getAttribute("followedby",szFollow);
			if(szFollow && strcmp(szFollow,"Current Settings")!=0)
			{
				if(pStyle->getFollowedBy())
					pStyle = pStyle->getFollowedBy();

				const XML_Char* szValue = NULL;
//
// The name of the style is stored in the PT_NAME_ATTRIBUTE_NAME attribute within the
// style
//
				pStyle->getAttribute(PT_NAME_ATTRIBUTE_NAME, szValue);

				UT_ASSERT((szValue));
				if (UT_strcmp((const char *) szValue, (const char *) style) != 0)
				{
					setStyle(szValue);
//
// Stop a List if "followed-by" is not a list style
//
					const XML_Char * szListType = NULL;
					pStyle->getProperty("list-style",szListType);
					bool bisListStyle = false;
					if(szListType)
					{
						bisListStyle = (NOT_A_LIST != getCurrentBlock()->getListTypeFromStyle( szListType));
					}
					sdh = getCurrentBlock()->getStruxDocHandle();
					while(!bisListStyle && getCurrentBlock()->isListItem())
					{
						m_pDoc->StopList(sdh);
					}	  
				}
			}
		}
	}

	m_pDoc->endUserAtomicGlob();

	_generalUpdate();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	_generalUpdate();

	// Signal piceTable is stable again
	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	m_iPieceTableState = 0;

	_ensureThatInsertionPointIsOnScreen();
	m_pLayout->considerPendingSmartQuoteCandidate();
}


void FV_View::insertParagraphBreaknoListUpdate(void)
{
	bool bDidGlob = false;

	if (!isSelectionEmpty())
	{
		bDidGlob = true;
		m_pDoc->beginUserAtomicGlob();
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	// insert a new paragraph with the same attributes/properties
	// as the previous (or none if the first paragraph in the section).

	m_pDoc->insertStrux(getPoint(), PTX_Block);

	if (bDidGlob)
		m_pDoc->endUserAtomicGlob();

	_generalUpdate();
	_ensureThatInsertionPointIsOnScreen();
}

bool FV_View::appendStyle(const XML_Char ** style)
{
	return m_pDoc->appendStyle(style);
}

bool FV_View::setStyle(const XML_Char * style, bool bDontGeneralUpdate)
{
	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;
	return setStyleAtPos(style, posStart, posEnd, bDontGeneralUpdate);
}

bool FV_View::setStyleAtPos(const XML_Char * style, PT_DocPosition posStart1, PT_DocPosition posEnd1, bool bDontGeneralUpdate)
{
	bool bRet;

	PT_DocPosition posStart = posStart1;
	PT_DocPosition posEnd = posEnd1;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	// Turn off list updates
	m_pDoc->disableListUpdates();
	UT_DEBUGMSG(("SEVIUOR: Setting style %s \n",style));
	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
		{
			posStart = m_iSelectionAnchor;
		}
		else
		{
			posEnd = m_iSelectionAnchor;
		}
	}

	UT_DEBUGMSG(("Style to set %s \n",style));
	
	// lookup the current style
	PD_Style * pStyle = NULL;
	m_pDoc->getStyle((char*)style, &pStyle);
	if (!pStyle)
	{
		m_pDoc->enableListUpdates();
		UT_DEBUGMSG(("restoring PieceTable state (2)\n"));
		_restorePieceTableState();
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	UT_DEBUGMSG(("SEVIOR: Setting style %s \n",style));
	if(strcmp(style,"None") == 0)
	{
		m_pDoc->enableListUpdates();
		UT_DEBUGMSG(("restoring PieceTable state (2)\n"));
		_restorePieceTableState();
		return true; // do nothing.
	}
//
// Get This info before it's lost from the following processing
// 
	fl_BlockLayout * pBL = getCurrentBlock();
	const XML_Char * pszStyle = NULL;
	pStyle->getProperty("list-style",pszStyle);
	bool bisListStyle = false;
	if(pszStyle)
		bisListStyle = (NOT_A_LIST != pBL->getListTypeFromStyle( pszStyle));
//
// Can't handle lists inside header/Footers. Bail out
//
	if(bisListStyle && isHdrFtrEdit())
	{
		m_pDoc->enableListUpdates();
		UT_DEBUGMSG(("restoring PieceTable state (2)\n"));
		_restorePieceTableState();
		return false;
	}
//
// Glob piecetable changes together.
//
	m_pDoc->beginUserAtomicGlob();
	UT_Vector vBlock;
	if(bisListStyle)
	{
		getBlocksInSelection( &vBlock);
	}

	bool bCharStyle = pStyle->isCharStyle();
	const XML_Char * attribs[] = { PT_STYLE_ATTRIBUTE_NAME, 0, 0 };
	attribs[1] = style;
	if(bisListStyle)
	{
//
// Stop the Lists contained in the current selection.
//
		UT_uint32 i;
		for(i=0; i< vBlock.getItemCount(); i++)
		{
			pBL = (fl_BlockLayout *)  vBlock.getNthItem(i);
			while(pBL->isListItem())
			{
				m_pDoc->StopList(pBL->getStruxDocHandle());
			} 
		}
	}

	if (bCharStyle)
	{
		// set character-level style
		if (isSelectionEmpty())
		{
			_eraseInsertionPoint();
		}
		_clearIfAtFmtMark(getPoint());	// TODO is this correct ??
		_eraseSelection();
		UT_DEBUGMSG(("Applying Character style: start %d, end %d\n", posStart, posEnd));
		bRet = m_pDoc->changeSpanFmt(PTC_AddStyle,posStart,posEnd,attribs,NULL);
	}
	else
	{
		// set block-level style
		UT_DEBUGMSG(("Applying Block style: start %d, end %d\n", posStart, posEnd));

		_eraseInsertionPoint();

		_clearIfAtFmtMark(getPoint());	// TODO is this correct ??

		// NB: clear explicit props at both block and char levels
		bRet = m_pDoc->changeStruxFmt(PTC_AddStyle,posStart,posEnd,attribs,NULL,PTX_Block);
	}
//
// Do the list elements processing
//
//
// Look for Numbered Heading in the style or it's ancestry.
//
	bool bHasNumberedHeading = false;
	const XML_Char * pszCurStyle = style;
	PD_Style * pCurStyle = pStyle;
	UT_uint32 depth = 0;
	while(bisListStyle && pCurStyle && !bHasNumberedHeading && depth < 10)
	{
		bHasNumberedHeading = (strstr(pszCurStyle,"Numbered Heading") != NULL);
		if(!bHasNumberedHeading)
		{
			pCurStyle = pCurStyle->getBasedOn();
			if(pCurStyle)
				pszCurStyle = pCurStyle->getName();
			depth++;
		}
	}

	if(bisListStyle && !bHasNumberedHeading)
	{
		UT_uint32 i;
		for(i=0; i< vBlock.getItemCount(); i++)
		{
			pBL = (fl_BlockLayout *)  vBlock.getNthItem(i);
			while(pBL->isListItem())
			{
				m_pDoc->StopList(pBL->getStruxDocHandle());
			} 
			if(i == 0)
				pBL->StartList(style);
			else
				pBL->resumeList(pBL->getPrev());
		}
	}
//
// Code to handle Numbered Heading styles...
//
	else if(bHasNumberedHeading)
	{
		fl_BlockLayout*  currBlock = (fl_BlockLayout *) vBlock.getNthItem(0);
		PT_DocPosition pos = currBlock->getPosition(true) -1;
		PL_StruxDocHandle curSdh = currBlock->getStruxDocHandle();
		if(pos < 2 )
			pos = 2;
//
// First look to see if this numbered heading style is embedded in a previous
// Numbered Heading piece of text.
//
		bool bAttach = false;
		PL_StruxDocHandle prevSDH = m_pDoc->getPrevNumberedHeadingStyle(curSdh);
		if(prevSDH != NULL)
		{
			PD_Style * pPrevStyle = m_pDoc->getStyleFromSDH(prevSDH);
			if(pPrevStyle != NULL)
			{
//
// See if it's of shallower depth then current numbered Heading.
//
				UT_uint32 icurDepth = UT_HeadingDepth(style);
				UT_uint32 iprevDepth = UT_HeadingDepth(pPrevStyle->getName());
//
// Start a sublist with the previous heading as the parent.
//
				if(iprevDepth < icurDepth)
				{
					for(UT_uint32 i=0; i< vBlock.getItemCount(); i++)
					{
						pBL = (fl_BlockLayout *)  vBlock.getNthItem(i);
						if(i == 0)
							pBL->StartList(style,prevSDH);
						else
							pBL->resumeList(pBL->getPrev());
					}
					bAttach = true;
				}
			}
		}
		if(!bAttach)
		{	
//
// Look backwards to see if there is a heading before here.
//
			bool bFoundPrevHeadingBackwards = false;
			PL_StruxDocHandle sdh = m_pDoc->findPreviousStyleStrux(style, pos);
			bFoundPrevHeadingBackwards = (sdh != NULL);
//
// If not, Look forward to see if there one ahead of this block
//
			if(!bFoundPrevHeadingBackwards || (curSdh == sdh))
			{
//
// Start looking from the block following and skip through to end of Doc.
//
				fl_BlockLayout * pNext = (fl_BlockLayout *) vBlock.getLastItem();
				pNext = pNext->getNext();
				if(pNext)
				{
					PT_DocPosition nextPos = pNext->getPosition(false);
					sdh = m_pDoc->findForwardStyleStrux(style, nextPos);
				}
			}
//
// OK put this heading style into any pre-exsiting Numbering Headings
//
			if(sdh == NULL || (sdh == curSdh))
			{
				for(UT_uint32 i=0; i< vBlock.getItemCount(); i++)
				{
					pBL = (fl_BlockLayout *)  vBlock.getNthItem(i);
					if(i == 0)
						pBL->StartList(style);
					else
						pBL->resumeList(pBL->getPrev());
				}
			}
//
// Insert into pre-existing List.
//
			else if(bFoundPrevHeadingBackwards)
			{
				PL_StruxFmtHandle sfh = NULL;
				UT_uint32 i;
				bool bFound = false;
				fl_BlockLayout * pBlock = NULL;
//
// Loop through all the format handles that match our sdh until we find the one
// in our View. (Not that it really matter I suppose.)
//
				for(i = 0; !bFound; ++i)
				{
//
// Cast it into a fl_BlockLayout and we're done!
//
					sfh = m_pDoc->getNthFmtHandle(sdh, i);
					if(sfh != NULL)
					{
						pBlock = (fl_BlockLayout *) sfh;
						if(pBlock->getDocLayout() == m_pLayout)
						{
							bFound = true;
						}
					}
					else
					{
						bFound = true;
					}
				}
				UT_ASSERT(sfh);
				for(UT_uint32 j = 0; j < vBlock.getItemCount(); ++j)
				{
					pBL = (fl_BlockLayout *)  vBlock.getNthItem(j);
					if(j == 0)
						pBL->resumeList(pBlock);
					else
						pBL->resumeList(pBL->getPrev());
				}
			}
//
// Prepend onto a pre-existing List.
//
			else
			{
				PL_StruxFmtHandle sfh = NULL;
				UT_uint32 i;
				bool bFound = false;
				fl_BlockLayout * pBlock = NULL;
				for(i=0; !bFound ; i++)
				{
//
// Loop through all the format handles that match our sdh until we find the one
// in our View. (Not that it really matters I suppose.)
//
					sfh = m_pDoc->getNthFmtHandle(sdh, i);
					if(sfh != NULL)
					{
//
// Cast it into a fl_BlockLayout and we're done!
//
						pBlock = (fl_BlockLayout *) sfh;
						if(pBlock->getDocLayout() == m_pLayout)
						{
							bFound = true;
						}
					}
					else
					{
						bFound = true;
					}
				}
				UT_ASSERT(sfh);
				for(UT_uint32 j = 0; j < vBlock.getItemCount(); j++)
				{
					pBL = (fl_BlockLayout *)  vBlock.getNthItem(j);
					if (j == 0)
						pBL->prependList(pBlock);
					else
						pBL->resumeList(pBL->getPrev());
				}
			}
		}
	}
	if(!bDontGeneralUpdate)
	{
		_generalUpdate();
		// restore updates and clean up dirty lists
		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();

		m_pDoc->endUserAtomicGlob();
		// Signal piceTable is stable again
		UT_DEBUGMSG(("restoring PieceTable state (1)\n"));
		_restorePieceTableState();
		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
		return bRet;
	}
	m_pDoc->endUserAtomicGlob();
	UT_DEBUGMSG(("restoring PieceTable state (2)\n"));
	_restorePieceTableState();
	return bRet;
}


static const XML_Char * x_getStyle(const PP_AttrProp * pAP, bool bBlock)
{
	UT_ASSERT(pAP);
	const XML_Char* sz = NULL;

	pAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, sz);

	// TODO: should we have an explicit default for char styles?
	if (!sz && bBlock)
	{
		sz = "None";
	}
	return sz;
}

bool FV_View::getStyle(const XML_Char ** style)
{
	bool bCharStyle = false;
	const XML_Char * szChar = NULL;
	const XML_Char * szBlock = NULL;

	const PP_AttrProp * pBlockAP = NULL;

	/*
	  IDEA: We want to know the style, if it's constant across the
	  entire selection.  Usually, this will be a block-level style.
	  However, if the entire span has the same char-level style,
	  we'll report that instead.  */
	
//
// Check we have a layout defined first. On start up we don't
//
	if(getLayout()->getFirstSection() == NULL)
	{
		return false;
	}
	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;
	bool bSelEmpty = isSelectionEmpty();

	if (!bSelEmpty)
	{
		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;
	}

	// 1. get block style at insertion point
	fl_BlockLayout* pBlock = _findBlockAtPosition(posStart);
	pBlock->getAttrProp(&pBlockAP);

	szBlock = x_getStyle(pBlockAP, true);

	// 2. prune if block style varies across selection
	if (!bSelEmpty)
	{
		fl_BlockLayout* pBlockEnd = _findBlockAtPosition(posEnd);

		while (pBlock && (pBlock != pBlockEnd))
		{
			const PP_AttrProp * pAP;
			bool bCheck = false;

			pBlock = pBlock->getNextBlockInDocument();

			if (!pBlock)
			{
				// at EOD, so just bail
				break;
			}

			// did block format change?
			pBlock->getAttrProp(&pAP);
			if (pBlockAP != pAP)
			{
				pBlockAP = pAP;
				bCheck = true;
			}

			if (bCheck)
			{
				const XML_Char* sz = x_getStyle(pBlockAP, true);
				
				if (UT_stricmp(sz, szBlock))
				{
					// doesn't match, so stop looking
					szBlock = NULL;
					pBlock = NULL;
					break;
				}
			}
		}
	}

	// NOTE: if block style varies, no need to check char style

	if (szBlock && szBlock[0])
	{
		const PP_AttrProp * pSpanAP = NULL;

		// 3. locate char style at insertion point
		UT_sint32 xPoint;
		UT_sint32 yPoint;
		UT_sint32 iPointHeight;
		UT_sint32 xPoint2;
		UT_sint32 yPoint2;
		bool bDirection;
		
		fl_BlockLayout* pBlock = _findBlockAtPosition(posStart);
		UT_uint32 blockPosition = pBlock->getPosition();
		fp_Run* pRun = pBlock->findPointCoords(posStart, false, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);
		bool bLeftSide = true;

		// TODO consider adding indexAP from change record to the
		// TODO runs so that we can just use it here.

		if (!bSelEmpty)
		{
			// we want the interior of the selection -- and not the
			// format to the left of the start of the selection.
			bLeftSide = false;
			
			/*
			  Likewise, findPointCoords will return the run to the right
			  of the specified position, so we need to stop looking one
			  position to the left.
			*/
			posEnd--;
		}

		pBlock->getSpanAttrProp( (posStart - blockPosition), bLeftSide, &pSpanAP);

		if (pSpanAP)
		{
			szChar = x_getStyle(pSpanAP, false);
			bCharStyle = (szChar && szChar[0]);
		}

		// 4. prune if char style varies across selection
		if (!bSelEmpty)
		{
			fl_BlockLayout* pBlockEnd = _findBlockAtPosition(posEnd);
			fp_Run* pRunEnd = pBlockEnd->findPointCoords(posEnd, false, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);
			
			while (pRun && (pRun != pRunEnd))
			{
				const PP_AttrProp * pAP;
				bool bCheck = false;

				pRun = pRun->getNext();
				if (!pRun)
				{
					// go to first run of next block
					pBlock = pBlock->getNextBlockInDocument();
					if (!pBlock)		// at EOD, so just bail
						break;
					pRun = pBlock->getFirstRun();
				}

				// did span format change?

				pAP = NULL;
				pBlock->getSpanAttrProp(pRun->getBlockOffset()+pRun->getLength(),true,&pAP);
				if (pAP && (pSpanAP != pAP))
				{
					pSpanAP = pAP;
					bCheck = true;
				}

				if (bCheck)
				{
					const XML_Char* sz = x_getStyle(pSpanAP, true);
					bool bHere = (sz && sz[0]);

					if ((bCharStyle != bHere) || (UT_stricmp(sz, szChar)))
					{
						// doesn't match, so stop looking
						bCharStyle = false;
						szChar = NULL;
						pRun = NULL;
						break;
					}
				}
			}
		}
	}

	*style = (bCharStyle ? szChar : szBlock);

	return true;
}

bool FV_View::setCharFormat(const XML_Char * properties[])
{
	bool bRet;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	if (isSelectionEmpty())
	{
		_eraseInsertionPoint();
	}

	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
		{
			posStart = m_iSelectionAnchor;
		}
		else
		{
			posEnd = m_iSelectionAnchor;
		}
	}

	// Here the selection used to be cleared, but that prevented users
	// from making multiple changes to the same region.

	bRet = m_pDoc->changeSpanFmt(PTC_AddFmt,posStart,posEnd,NULL,properties);

	// if there is a selection then we need to change the formatting also for any
	// completely selected block within the selection
	if(posStart != posEnd)
	{
		fl_BlockLayout * pBL1 = _findBlockAtPosition(posStart);
		fl_BlockLayout * pBL2 = _findBlockAtPosition(posEnd);
		bool bFormatStart = false;
		bool bFormatEnd = false;

		PT_DocPosition posBL1 = pBL1->getPosition(false);

		fp_Run * pLastRun2 = pBL2->getLastLine()->getLastRun();
		PT_DocPosition posBL2 = pBL2->getPosition(false) + pLastRun2->getBlockOffset() + pLastRun2->getLength() - 1;
	
		if(posBL1 == posStart)
		{
			bFormatStart = true;
		}
		else if(posBL1 < posStart && pBL1->getNext())
		{
			posStart = pBL1->getNext()->getPosition(false);
			if(posStart < posEnd)
				bFormatStart = true;
		}

		// TODO: We have got a problem with handling the end of the paragraph.
		// Basically, if only the text in the paragraph is selected, we
		// do not want to apply the format to the paragraph; if the pilcrow
		// too is selected, we want to apply the format to the paragraph as well.
		// The problem lies in the fact that although the pilcrow represents the
		// block, it is located at the end of the paragraph, while the strux
		// that contains the formatting is at the start. Consequently the when the
		// pilcrow is selected, the end of the selection is already in the next
		// paragraph. If there is a next paragraph, we can handle this, but if
		// there is not a next pragraph, it is not currently possible to select the
		// pilcrow, i.e., we cannot change the font (for instance) used by the last
		// paragraph in the document; there is no simple solution at the moment, since
		// if we allow the selection to go across the last pilcrow, it will be possible
		// to insert text into the block _after_ the pilcrow, and we do not want that.
		// we would need to make some changes to the block insertion mechanism, so that
		// if insertion is requested to be past the pilcrow, it would be adjusted to
		// happen just before it -- Tomas 19/01/2002
		
		if(posBL2 > posEnd && pBL2->getPrev())
		{
			pLastRun2 = pBL2->getPrev()->getLastLine()->getLastRun();
			posEnd = pBL2->getPrev()->getPosition(false) + pLastRun2->getBlockOffset() + pLastRun2->getLength() - 1;
			if(posEnd > posStart)
				bFormatEnd = true;
		}
		
		if(bFormatStart && bFormatEnd)
			bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,posStart,posEnd,NULL,properties,PTX_Block);
	}
	
	_generalUpdate();

	// Signal piceTable is stable again
	_restorePieceTableState();

	if (isSelectionEmpty())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
	return bRet;
}

bool FV_View::getCharFormat(const XML_Char *** pProps, bool bExpandStyles)
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance
	UT_Vector v;
	UT_uint32 i;
	_fmtPair * f;
	
//
// Check we have a layout defined first. On start up we don't
//

	if(getLayout()->getFirstSection() == NULL)
	{
		return false;
	}
	/*
	  IDEA: We want to know character-level formatting properties, if
	  they're constant across the entire selection.  To do so, we start
	  at the beginning of the selection, load 'em all into a vector, and
	  then prune any property that collides.
	*/
	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;
	bool bSelEmpty = isSelectionEmpty();

	if (!bSelEmpty)
	{
		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;
	}

	// 1. assemble complete set at insertion point
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
	UT_sint32 xPoint2;
	UT_sint32 yPoint2;
	bool bDirection;
	
	fl_BlockLayout* pBlock = _findBlockAtPosition(posStart);
	UT_uint32 blockPosition = pBlock->getPosition();
	if(blockPosition > posStart)
	{
		posStart = blockPosition;
		if(posEnd < posStart)
			posEnd = posStart;
	}
	fp_Run* pRun = pBlock->findPointCoords(posStart, false, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);
	bool bLeftSide = true;

	// TODO consider adding indexAP from change record to the
	// TODO runs so that we can just use it here.

	if (!bSelEmpty)
	{
		// we want the interior of the selection -- and not the
		// format to the left of the start of the selection.
		bLeftSide = false;

		/*
		  Likewise, findPointCoords will return the run to the right
		  of the specified position, so we need to stop looking one
		  position to the left.
		*/
		posEnd--;
	}

	pBlock->getSpanAttrProp( (posStart - blockPosition), bLeftSide, &pSpanAP);

	pBlock->getAttrProp(&pBlockAP);

	f = new _fmtPair("font-family",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem((void *) f);
	else
		delete f;

	f = new _fmtPair("font-size",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("font-weight",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("font-style",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("text-decoration",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("text-position",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("color",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("bgcolor",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

#ifdef BIDI_ENABLED
	// ###TF f = new _fmtPair("dir",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	//if(f->m_val != NULL)
	//	v.addItem( (void *) f);
	//else
	//	delete f;

	f = new _fmtPair("dir-override",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

#endif
	f = new _fmtPair("lang",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

#if 1
	f = new _fmtPair("width",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;


	f = new _fmtPair("height",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

#endif
	// 2. prune 'em as they vary across selection
	if (!bSelEmpty)
	{
		fl_BlockLayout* pBlockEnd = _findBlockAtPosition(posEnd);
		fp_Run* pRunEnd = pBlockEnd->findPointCoords(posEnd, false, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);
		
		while (pRun && (pRun != pRunEnd))
		{
			const PP_AttrProp * pAP;
			bool bCheck = false;

			pRun = pRun->getNext();
			if (!pRun)
			{
				// go to first run of next block
				pBlock = pBlock->getNextBlockInDocument();

				if (!pBlock)			// at EOD, so just bail
					break;

				// did block format change?
				pBlock->getAttrProp(&pAP);
				if (pBlockAP != pAP)
				{
					pBlockAP = pAP;
					bCheck = true;
				}

				pRun = pBlock->getFirstRun();
			}

			// FIXME: Don't look in EOP Runs. This is because they are
			// not represented in the PT, so when the PT is asked for
			// the properties of the EOP's position, it returns the
			// default properties. What it should return is the
			// properties of the previous Run. But just skipping the
			// check altogether is easier (and faster).
			if (pRun->getType() == FPRUN_ENDOFPARAGRAPH)
				continue;


			// did span format change?

			pAP = NULL;
			pBlock->getSpanAttrProp(pRun->getBlockOffset()+pRun->getLength(),true,&pAP);
			if (pSpanAP != pAP)
			{
				pSpanAP = pAP;
				bCheck = true;
			}

			if (bCheck)
			{
				i = v.getItemCount();

				while (i > 0)
				{
					f = (_fmtPair *)v.getNthItem(i-1);

					const XML_Char * value = PP_evalProperty(f->m_prop,pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
#ifndef BIDI_ENABLED
					// the bidi property dir-override has no default value, so that if its
					// not explicitely present in the span formatting, we will get a NULL here
					UT_ASSERT(value);
#endif			

					// prune anything that doesn't match
					if (value && UT_stricmp(f->m_val, value))
					{
						DELETEP(f);
						v.deleteNthItem(i-1);
					}

					i--;
				}

				// when vector is empty, stop looking
				if (0 == v.getItemCount())
				{
					pRun = NULL;
					break;
				}
			}
		}
	}

	// 3. export whatever's left
	UT_uint32 count = v.getItemCount()*2 + 1;

	// NOTE: caller must free this, but not the referenced contents
	const XML_Char ** props = (const XML_Char **) calloc(count, sizeof(XML_Char *));
	if (!props)
		return false;

	const XML_Char ** p = props;

	i = v.getItemCount();

	while (i > 0)
	{
		f = (_fmtPair *)v.getNthItem(i-1);
		i--;

		p[0] = f->m_prop;
		p[1] = f->m_val;
		p += 2;
	}

	UT_VECTOR_PURGEALL(_fmtPair *,v);

	*pProps = props;

	return true;
}


/*!
   This method fills a vector with all the blocks contained between the 
   first and last blocks of a list structure.
   \param	v Pointer to Vector of all the blocks found
*/

void FV_View::getAllBlocksInList(UT_Vector * v)
{
	//
	// get all the blocks in the list
	//
	fl_BlockLayout * pBlock;
	fl_AutoNum * pAuto = getCurrentBlock()->getAutoNum();
	UT_ASSERT(pAuto);
	PL_StruxDocHandle pFirstSdh = pAuto->getFirstItem();
	PL_StruxDocHandle pLastSdh = pAuto->getNthBlock(pAuto->getNumLabels()-1);
	fl_SectionLayout * pSl = getCurrentBlock()->getSectionLayout();
	pBlock = pSl->getFirstBlock();
	bool foundLast = false;
	bool foundFirst = false;

	//
	// Now collect all all the blocks between the first and last list elements
	// in a vector
	//
	while (pBlock != NULL && foundLast == false)
	{
		if(pBlock->getStruxDocHandle() == pFirstSdh)
		{
			foundFirst = true;
		}
		if(foundFirst == true)
			v->addItem(pBlock);
		if(pBlock->getStruxDocHandle() == pLastSdh)
			foundLast = true;
		pBlock = pBlock->getNext();
	}
}

/*!

   This method increases or decreases the indents of a range of blocks. 
   The blocks can be either all those contained by a list structure or 
   just those in a selection.

   \param	doList true if you want to indents all the blocks in the list
			of which the current block is a member. If false just those 
			blocks within the current selected range. 
   \param	indentChange +-ve value by which the block will be indented.
   \param	page_size width of the page in inches.
*/

bool FV_View::setBlockIndents(bool doLists, double indentChange, double page_size)
{
	//
	// indentChange is the increment to the current alignment.
	//
	UT_Vector v;
	XML_Char pszAlign[20];
	bool bRet = true;
	UT_Dimension dim;
	double fAlign;	
	fl_BlockLayout * pBlock;
	UT_uint32 i;
	//
	// Signal PieceTable Change
	//
	_saveAndNotifyPieceTableChange();

	_eraseInsertionPoint();
	m_pDoc->beginUserAtomicGlob();
	//
	// OK now change the alignements of the blocks.
	//
	if(doLists)
		getAllBlocksInList(&v);
	else
		getBlocksInSelection(&v);
	const XML_Char * props[] = {"margin-left","0.0in",NULL,NULL};
	for(i = 0; i<v.getItemCount();i++)
	{
		pBlock = (fl_BlockLayout *)v.getNthItem(i);
		UT_XML_strncpy((XML_Char *) pszAlign,20,pBlock->getProperty("margin-left"));
		dim = UT_determineDimension( (char *) pszAlign);
		fAlign = UT_convertToInches((char *) pszAlign);
		if(fAlign + indentChange < 0.0)
		{
			fAlign = 0.0;
		}
		else if( fAlign + indentChange > page_size)
		{
			fAlign = page_size;
		}
		else
		{
			fAlign = fAlign + indentChange;
		}
		char * pszNewAlign = UT_strdup (UT_convertInchesToDimensionString (dim, fAlign));
		PL_StruxDocHandle sdh = pBlock->getStruxDocHandle();		
		PT_DocPosition iPos = m_pDoc->getStruxPosition(sdh)+fl_BLOCK_STRUX_OFFSET;
		props[1] = (XML_Char *) pszNewAlign;
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, iPos, iPos, NULL, props, PTX_Block);
		FREEP(pszNewAlign); 
	}
	//
	// Moved outside the loop. Speeds things up and seems OK.
	//
	_generalUpdate();

	m_pDoc->endUserAtomicGlob();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	if (isSelectionEmpty())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
	return bRet;
}

bool FV_View::setBlockFormat(const XML_Char * properties[])
{
	bool bRet;


	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();


	_clearIfAtFmtMark(getPoint());

	_eraseInsertionPoint();

	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;
	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
		{
			posStart = m_iSelectionAnchor;
		}
		else
		{
			posEnd = m_iSelectionAnchor;
		}
	}

	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,posStart,posEnd,NULL,properties,PTX_Block);

	_generalUpdate();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	if (isSelectionEmpty())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
	return bRet;
}

/*!
 * This method does the insert Page Number logic.
 * It inserts with this logic. For the sake of not writing header/footer every
 * time I will just write "footer"
 * 1. If no footer exists one will be created and a page number inserted.
 * 2. If a footer with a page number exists the paragraph containing the footer
 * will be right center or left justified as requested by the user.
 * 3. If a footer with no page number exists a new paragraph containg the 
 * page number will be inserted at the top of the container.
 *
\param bIsFooter true if the user wants a pagenumber in a footer.
\param const  XML_Char ** atts	const string describing left , center or right
 * justification.
 */

bool FV_View::processPageNumber(HdrFtrType hfType, const XML_Char ** atts)
{
//
// Quick hack to stop a segfault if a user tries to insert a header from
// within a header/footer.
//
	bool bInsertFromHdrFtr = false;
	PT_DocPosition oldpos = getPoint();
	fl_HdrFtrShadow * pShadow = NULL;
	if(isHdrFtrEdit())
	{
		bInsertFromHdrFtr = true;
		pShadow = m_pEditShadow;
		clearHdrFtrEdit();
		warpInsPtToXY(0,0,false);
	}
//
// Handle simple cases of inserting into non-existing header/footers.
//
	fl_DocSectionLayout * pDSL = getCurrentPage()->getOwningSection();
	if(hfType == FL_HDRFTR_FOOTER && pDSL->getFooter() == NULL)
	{
		insertPageNum(atts, hfType);
		return true;
	}
	else if(hfType == FL_HDRFTR_HEADER && pDSL->getHeader() == NULL)
	{
		insertPageNum(atts, hfType);
		return true;
	}
//
// OK we're here if we want to insert a page number into a pre-existing 
// header/footer. Let's get the header/footer now.
//
	fl_HdrFtrSectionLayout * pHFSL = NULL;
	if(hfType >= FL_HDRFTR_FOOTER)
		pHFSL = pDSL->getFooter();
	else
		pHFSL = pDSL->getHeader();
//
// Scan the layout for a pre-existing page number.
//
	fl_BlockLayout * pBL = pHFSL->getFirstBlock();
	bool bFoundPageNumber = false;
	while(pBL != NULL && !bFoundPageNumber)
	{
		fp_Run * pRun = pBL->getFirstRun();
		while(pRun != NULL && !bFoundPageNumber)
		{
			if(pRun->getType() == FPRUN_FIELD)
			{
				fp_FieldRun * pFRun = (fp_FieldRun *) pRun;
				bFoundPageNumber = (pFRun->getFieldType() == FPFIELD_page_number);
			}
			pRun = pRun->getNext();
		}
		if(!bFoundPageNumber) 
			pBL = pBL->getNext();
	}

	// Signal PieceTable Change

	_saveAndNotifyPieceTableChange();
//
// Just set the format of the Block if a PageNumber has been found.
//
	bool bRet;
	PT_DocPosition pos;
	if(bFoundPageNumber)
	{
		pos = pBL->getPosition();
		if (isSelectionEmpty())
			_eraseInsertionPoint();

		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,pos,pos,NULL,atts,PTX_Block);

		if(bInsertFromHdrFtr)
		{
			_setPoint(oldpos);
			setHdrFtrEdit(pShadow);
		}

		// Signal PieceTable Changes have finished
		_generalUpdate();
		_restorePieceTableState();
		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
		return bRet;
	}
//
// We're here if there's a header/footer with no page number
// Insert a page number with the correct formatting.
// 

	const XML_Char* f_attributes[] = {
		"type", "page_number",
		NULL, NULL
	};
	pBL = pHFSL->getFirstBlock();
	pos = pBL->getPosition();
	if (isSelectionEmpty())
		_eraseInsertionPoint();

	//Glob it all together so it can be undone with one
	// click

	m_pDoc->beginUserAtomicGlob();

	//
	// Insert a blank paragraph at the beginning of the Section
	//
	
	m_pDoc->insertStrux(pos, PTX_Block);

	// 
	// Set the formatting of the paragraph to the Users request
	//
	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,pos,pos,NULL,atts,PTX_Block);

	// Insert the page_number field with the requested attributes at the top
	// of the header/footer. 

	bRet = m_pDoc->insertObject(pos, PTO_Field, f_attributes, NULL);
	m_pDoc->endUserAtomicGlob();

	if(bInsertFromHdrFtr)
	{
		_setPoint(oldpos);
		setHdrFtrEdit(pShadow);
	}

	_generalUpdate();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	if (isSelectionEmpty())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
	return bRet;
}

bool FV_View::cmdStartList(const XML_Char * style)
{
	m_pDoc->beginUserAtomicGlob();
	fl_BlockLayout * pBlock = getCurrentBlock();
	pBlock->StartList( style);
	m_pDoc->endUserAtomicGlob();

	return true;
}

void FV_View::changeListStyle(	fl_AutoNum* pAuto,
								List_Type lType,
								UT_uint32 startv,
								const XML_Char* pszDelim,
								const XML_Char* pszDecimal,
								const XML_Char* pszFont,
								float Align,
								float Indent)
{
	bool bRet;
	UT_uint32 i=0;
	XML_Char pszStart[80],pszAlign[20],pszIndent[20];
	UT_Vector va,vp,vb;
	PL_StruxDocHandle sdh = pAuto->getNthBlock(i);

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	m_pDoc->disableListUpdates();

	if(lType == NOT_A_LIST)
	{
		// Stop lists in all elements
		i = 0;
		sdh = pAuto->getNthBlock(i);
		while(sdh != NULL)
		{
			vb.addItem((void *) sdh);
			i++;
			sdh = pAuto->getNthBlock(i);
		}
		for(i=0; i< vb.getItemCount(); ++i)
		{
			PL_StruxDocHandle sdh = (PL_StruxDocHandle) vb.getNthItem(i);
			m_pDoc->listUpdate(sdh);
			m_pDoc->StopList(sdh);
		}

		_generalUpdate();

		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();

		// Signal PieceTable Changes have finished
		_restorePieceTableState();
		return;
	}

	XML_Char * style = getCurrentBlock()->getListStyleString(lType);
	_eraseInsertionPoint();
//
// This is depeciated..
	va.addItem( (void *) PT_STYLE_ATTRIBUTE_NAME);	va.addItem( (void *) style);

	pAuto->setListType(lType);
	sprintf(pszStart, "%i" , startv);
	UT_XML_strncpy( pszAlign,
					sizeof(pszAlign),
					UT_convertInchesToDimensionString(DIM_IN, Align, 0));

	UT_XML_strncpy( pszIndent,
					sizeof(pszIndent),
					UT_convertInchesToDimensionString(DIM_IN, Indent, 0));

	vp.addItem( (void *) "start-value");	vp.addItem( (void *) pszStart);
	vp.addItem( (void *) "margin-left");	vp.addItem( (void *) pszAlign);
	vp.addItem( (void *) "text-indent");	vp.addItem( (void *) pszIndent);
	vp.addItem( (void *) "list-style"); 	vp.addItem( (void *) style);
	pAuto->setStartValue(startv);
	if(pszDelim != NULL)
	{
		vp.addItem( (void *) "list-delim"); vp.addItem( (void *) pszDelim);
		pAuto->setDelim(pszDelim);
	}
	if(pszDecimal != NULL)
	{
		vp.addItem( (void *) "list-decimal"); vp.addItem( (void *) pszDecimal);
		pAuto->setDecimal(pszDecimal);
	}
	if(pszFont != NULL)
	{
		vp.addItem( (void *) "field-font"); vp.addItem( (void *) pszFont);
	}
	//
	// Assemble the List attributes
	//
	UT_uint32 counta = va.getItemCount() + 1;
	const XML_Char ** attribs = (const XML_Char **) calloc(counta, sizeof(XML_Char *));
	for(i=0; i<va.getItemCount();i++)
	{
		attribs[i] = (XML_Char *) va.getNthItem(i);
	}
	attribs[i] = (XML_Char *) NULL;
	//
	// Now assemble the list properties
	//
	UT_uint32 countp = vp.getItemCount() + 1;
	const XML_Char ** props = (const XML_Char **) calloc(countp, sizeof(XML_Char *));
	for(i=0; i<vp.getItemCount();i++)
	{
		props[i] = (XML_Char *) vp.getNthItem(i);
	}
	props[i] = (XML_Char *) NULL;

	_eraseInsertionPoint();
	i = 0;
	sdh = (PL_StruxDocHandle) pAuto->getNthBlock(i);
	while(sdh != NULL)
	{
		PT_DocPosition iPos = m_pDoc->getStruxPosition(sdh)+fl_BLOCK_STRUX_OFFSET;
//		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, iPos, iPos, attribs, props, PTX_Block);
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, iPos, iPos, NULL, props, PTX_Block);
		i++;
		sdh = (PL_StruxDocHandle) pAuto->getNthBlock(i);
		_generalUpdate();
	}

	_generalUpdate();
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();


	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	_ensureThatInsertionPointIsOnScreen(false);
	DELETEP(attribs);
	DELETEP(props);
}

bool FV_View::cmdStopList(void)
{


	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	m_pDoc->beginUserAtomicGlob();
	fl_BlockLayout * pBlock = getCurrentBlock();
	m_pDoc->StopList(pBlock->getStruxDocHandle());
	m_pDoc->endUserAtomicGlob();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	return true;
}

bool FV_View::getSectionFormat(const XML_Char ***pProps)
{
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;
	UT_Vector v;
	UT_uint32 i;
	_fmtPair * f;

	/*
	  IDEA: We want to know block-level formatting properties, if
	  they're constant across the entire selection.  To do so, we start
	  at the beginning of the selection, load 'em all into a vector, and
	  then prune any property that collides.
	*/
	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;

//
// Check we have a layout defined first. On start up we don't
//
	if(getLayout()->getFirstSection() == NULL)
	{
		return false;
	}

	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;
	}
	
	// 1. assemble complete set at insertion point
	fl_BlockLayout* pBlock = _findBlockAtPosition(posStart);
	fl_SectionLayout* pSection = pBlock->getSectionLayout();
	pSection->getAttrProp(&pSectionAP);

	v.addItem(new _fmtPair("columns", NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("column-line", NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("column-gap",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("section-space-after",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("section-max-column-height",NULL,pBlockAP,pSectionAP,m_pDoc,false));	
	v.addItem(new _fmtPair("section-restart",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("section-restart-value",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("footer",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("footer-even",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("footer-first",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("footer-last",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("header",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("header-even",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("header-first",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("header-last",NULL,pBlockAP,pSectionAP,m_pDoc,false));
#ifdef BIDI_ENABLED
	//v.addItem(new _fmtPair("column-order",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	//N.B. we do not want this property to propagate up from the block formating
	v.addItem(new _fmtPair("dom-dir",NULL,NULL,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("text-align",NULL,NULL,pSectionAP,m_pDoc,false));
#endif
	v.addItem(new _fmtPair("page-margin-left",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("page-margin-top",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("page-margin-right",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("page-margin-bottom",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("page-margin-footer",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("page-margin-header",NULL,pBlockAP,pSectionAP,m_pDoc,false));
	v.addItem(new _fmtPair("bgcolor",NULL,pBlockAP,pSectionAP,m_pDoc,false));

	// 2. prune 'em as they vary across selection
	if (!isSelectionEmpty())
	{
		fl_BlockLayout* pBlockEnd = _findBlockAtPosition(posEnd);
		fl_SectionLayout *pSectionEnd = pBlockEnd->getSectionLayout();

		while (pSection && (pSection != pSectionEnd))
		{
			const PP_AttrProp * pAP;
			bool bCheck = false;

			pSection = pSection->getNext();
			if (!pSection)				// at EOD, so just bail
				break;

			// did block format change?
			pSection->getAttrProp(&pAP);
			if (pSectionAP != pAP)
			{
				pSectionAP = pAP;
				bCheck = true;
			}

			if (bCheck)
			{
				i = v.getItemCount();

				while (i > 0)
				{
					f = (_fmtPair *)v.getNthItem(i-1);

					const XML_Char * value = PP_evalProperty(f->m_prop,NULL,pBlockAP,pSectionAP,m_pDoc,false);

					// prune anything that doesn't match
					if(f->m_val == NULL  || value == NULL)
					{
						DELETEP(f);
						v.deleteNthItem(i-1);
					}
					else if (UT_stricmp(f->m_val, value))
					{
						DELETEP(f);
						v.deleteNthItem(i-1);
					}

					i--;
				}

				// when vector is empty, stop looking
				if (0 == v.getItemCount())
				{
					pSection = NULL;
					break;
				}
			}
		}
	}

	// 3. export whatever's left
	UT_uint32 count = v.getItemCount()*2 + 1;

	// NOTE: caller must free this, but not the referenced contents
	const XML_Char ** props = (const XML_Char **) calloc(count, sizeof(XML_Char *));
	if (!props)
		return false;

	const XML_Char ** p = props;

	i = v.getItemCount();

	while (i > 0)
	{
		f = (_fmtPair *)v.getNthItem(i-1);
		i--;

		p[0] = f->m_prop;
		p[1] = f->m_val;
		p += 2;
	}

	UT_VECTOR_PURGEALL(_fmtPair *,v);

	*pProps = props;

	return true;
}

bool FV_View::getBlockFormat(const XML_Char *** pProps,bool bExpandStyles)
{
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;	// TODO do we care about section-level inheritance?
											// we do in the bidi version

//
// Check we have a layout defined first. On startup we don't
//
	if(getLayout()->getFirstSection() == NULL)
	{
		return false;
	}
	UT_Vector v;
	UT_uint32 i;
	_fmtPair * f = NULL;

	/*
	  IDEA: We want to know block-level formatting properties, if
	  they're constant across the entire selection.  To do so, we start
	  at the beginning of the selection, load 'em all into a vector, and
	  then prune any property that collides.
	*/
	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;
	}

	// 1. assemble complete set at insertion point
	fl_BlockLayout* pBlock = _findBlockAtPosition(posStart);
	pBlock->getAttrProp(&pBlockAP);

#ifdef BIDI_ENABLED
	fl_SectionLayout* pSection = pBlock->getSectionLayout();
	pSection->getAttrProp(&pSectionAP);
#endif	

	f = new _fmtPair("text-align",NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("text-indent",NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("margin-left",NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("margin-right",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("margin-top",				NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("margin-bottom",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("line-height", 		NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("tabstops",				NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("default-tab-interval",	NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("keep-together",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("keep-with-next",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("orphans", 			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

	f = new _fmtPair("widows",					NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

#ifdef BIDI_ENABLED
	f = new _fmtPair("dom-dir", 	NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
	if(f->m_val != NULL)
		v.addItem( (void *) f);
	else
		delete f;

#endif

	// 2. prune 'em as they vary across selection
	if (!isSelectionEmpty())
	{
		fl_BlockLayout* pBlockEnd = _findBlockAtPosition(posEnd);

		while (pBlock && (pBlock != pBlockEnd))
		{
			const PP_AttrProp * pAP;
			bool bCheck = false;

			pBlock = pBlock->getNextBlockInDocument();
			if (!pBlock)				// at EOD, so just bail
				break;

			// did block format change?
			pBlock->getAttrProp(&pAP);
			if (pBlockAP != pAP)
			{
				pBlockAP = pAP;
				bCheck = true;
			}

			if (bCheck)
			{
				i = v.getItemCount();

				while (i > 0)
				{
					f = (_fmtPair *)v.getNthItem(i-1);

					const XML_Char * value = PP_evalProperty(f->m_prop,NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
					UT_ASSERT(value);

					// prune anything that doesn't match
					if (UT_stricmp(f->m_val, value))
					{
						DELETEP(f);
						v.deleteNthItem(i-1);
					}

					i--;
				}

				// when vector is empty, stop looking
				if (0 == v.getItemCount())
				{
					pBlock = NULL;
					break;
				}
			}
		}
	}

	// 3. export whatever's left
	UT_uint32 count = v.getItemCount()*2 + 1;

	// NOTE: caller must free this, but not the referenced contents
	const XML_Char ** props = (const XML_Char **) calloc(count, sizeof(XML_Char *));
	if (!props)
		return false;

	const XML_Char ** p = props;

	i = v.getItemCount();

	while (i > 0)
	{
		f = (_fmtPair *)v.getNthItem(i-1);
		i--;

		p[0] = f->m_prop;
		p[1] = f->m_val;
		p += 2;
	}

	UT_VECTOR_PURGEALL(_fmtPair *,v);

	*pProps = props;

	return true;
}

void FV_View::delTo(FV_DocPos dp)
{
	PT_DocPosition iPos = _getDocPos(dp);


	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	if (iPos == getPoint())
	{
		return;
	}

	_extSelToPos(iPos);
	_deleteSelection();

	_generalUpdate();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_fixInsertionPointCoords();
	_drawInsertionPoint();
}

/*
	This function is somewhat of a compromise.	It will return a new
	range of memory (destroy with free()) full of what's in the selection,
	but it will not cross block boundaries.  This is for convenience
	in implementation, but could probably be accomplished without
	too much more effort.  However, since an edit entry in a dialog
	box (1) only allows a bit of text and (2) has no concept of a
	block break anyway, I don't see a reason to make this behave
	differently.
*/
UT_UCSChar * FV_View::getSelectionText(void)
{
	UT_ASSERT(!isSelectionEmpty());
  
	UT_GrowBuf buffer;	
  
	UT_uint32 selLength = labs(m_iInsPoint - m_iSelectionAnchor);
  
	PT_DocPosition low;
	if (m_iInsPoint > m_iSelectionAnchor)
	{
		low = m_iSelectionAnchor;
	}
	else
	{
		low = m_iInsPoint;
	}
	
	// get the current block the insertion point is in
	fl_BlockLayout * block = m_pLayout->findBlockAtPosition(low);

	if (block)
	{
		block->getBlockBuf(&buffer);

		UT_UCSChar * bufferSegment = NULL;

		PT_DocPosition offset = low - block->getPosition(false);

		// allow no more than the rest of the block
		if (offset + selLength > buffer.getLength())
			selLength = buffer.getLength() - offset;
		// give us space for our new chunk of selected text, add 1 so it
		// terminates itself
		bufferSegment = (UT_UCSChar *) calloc(selLength + 1, sizeof(UT_UCSChar));

		// copy it out
		memmove(bufferSegment, buffer.getPointer(offset), selLength * sizeof(UT_UCSChar));

	 return bufferSegment;
	}

	return NULL;
}
#if 0
// this function has not been debugged
UT_UCSChar *	FV_View::getTextBetweenPos(PT_DocPosition pos1, PT_DocPosition pos2)
{

	UT_ASSERT(pos2 > pos1);
	
	UT_GrowBuf buffer;	

	UT_uint32 iLength = pos2 - pos1;
	UT_uint32 curLen = 0;
	
	PT_DocPosition curPos = pos1;

	// get the current block the insertion point is in
	fl_BlockLayout * pBlock = m_pLayout->findBlockAtPosition(curPos);

	UT_UCSChar * bufferRet = new UT_UCSChar[iLength];
	
	UT_ASSERT(bufferRet);
	if(!bufferRet)
		return NULL;
		
	UT_UCSChar * buff_ptr = bufferRet;
	
	while(pBlock && curPos < pos2)
	{
		pBlock->getBlockBuf(&buffer);

		PT_DocPosition offset = curPos - pBlock->getPosition(false);
		UT_uint32 iLenToCopy = MIN(pos2 - curPos, buffer.getLength() - offset);
		while(curPos < pos2 && (curPos < pBlock->getPosition(false) + pBlock->getLength()))
		{
			memmove(buff_ptr, buffer.getPointer(offset), iLenToCopy * sizeof(UT_UCSChar));
			offset	 += iLenToCopy;
			buff_ptr += iLenToCopy;
			curPos	 += iLenToCopy;
		}
		
		pBlock = pBlock->getNext();
	}
	
	UT_ASSERT(curPos == pos2);
	return bufferRet;
}
#endif

bool FV_View::isTabListBehindPoint(void)
{
	PT_DocPosition cpos = getPoint();
	PT_DocPosition ppos = cpos - 1;
	PT_DocPosition posBOD;
	bool bRes;
	bool bEOL = false;
	UT_sint32 xPoint, yPoint, iPointHeight;
	UT_sint32 xPoint2, yPoint2;
	bool   bDirection;

	bRes = getEditableBounds(false, posBOD);
	UT_ASSERT(bRes);
	if (cpos <= posBOD - 1)
	{
		return false;
	}
	
	fl_BlockLayout* pBlock = _findBlockAtPosition(cpos);
	if (!pBlock)
		return false;
	if(pBlock->isListItem() == false)
		return false;
	fl_BlockLayout* ppBlock = _findBlockAtPosition(ppos);
	if (!ppBlock || pBlock != ppBlock)
	{
		return false;
	}


	fp_Run* pRun = pBlock->findPointCoords(ppos, bEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);
	
	if(pRun->getType() != FPRUN_TAB)
	{
		return false;
	}
	pRun = pRun->getPrev();
	while((pRun != NULL) && (pRun->getType()== FPRUN_FMTMARK))
	{
		pRun = pRun->getPrev();
	}
	if (!pRun || pRun->getType() != FPRUN_FIELD)
	{
		return false;
	}
	else
	{
		fp_FieldRun * pFRun = (fp_FieldRun *) pRun;
		if (pFRun->getFieldType() != FPFIELD_list_label)
		{
			return false;
		}
		return true;
	}
}


bool FV_View::isTabListAheadPoint(void)
{
	//
	// Return TRUE if the point is immediately ahead of a list label - TAB combination
	//

	PT_DocPosition cpos = getPoint();
	
	fl_BlockLayout* pBlock = _findBlockAtPosition(cpos);
	if (!pBlock || pBlock->isListItem() == false)
	{
		return false;
	}

	bool bEOL = false;
	UT_sint32 xPoint, yPoint, iPointHeight;
	UT_sint32 xPoint2, yPoint2;
	bool   bDirection;
	
	fp_Run* pRun = pBlock->findPointCoords(cpos, bEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);

	// Find first run that is not an FPRUN_FMTMARK
	while (pRun && (pRun->getType() == FPRUN_FMTMARK))
	{
		pRun = pRun->getNext();
	}

	if (!pRun || pRun->getType() != FPRUN_FIELD)
	{
		return false;
	}

	fp_FieldRun * pFRun = (fp_FieldRun *) pRun;
	if(pFRun->getFieldType() != FPFIELD_list_label)
	{
		return false;
	}

	pRun = pRun->getNext();
	while (pRun && (pRun->getType()== FPRUN_FMTMARK))
	{
		pRun = pRun->getNext();
	}
	if (!pRun || pRun->getType() != FPRUN_TAB)
	{
		return false;
	}

	return true;
}

void FV_View::cmdCharDelete(bool bForward, UT_uint32 count)
{
	const XML_Char * properties[] = { "font-family", NULL, 0};
	const XML_Char ** props_in = NULL;
	const XML_Char * currentfont;
	bool bisList = false;
	fl_BlockLayout * curBlock = NULL;
	fl_BlockLayout * nBlock = NULL;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	if (!isSelectionEmpty())
	{
		m_pDoc->disableListUpdates();

		_deleteSelection();

		_generalUpdate();

		// restore updates and clean up dirty lists
		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();

		_ensureThatInsertionPointIsOnScreen();
	}
	else
	{
		//
		// Look to see if there is a tab - list label deal with these together
		//
		if((bForward == false) && (count == 1))
		{
			if(isTabListBehindPoint() == true)
			{
				curBlock = _findBlockAtPosition(getPoint());
				nBlock = _findBlockAtPosition(getPoint()-2);
				if(nBlock == curBlock)
				{
					count = 2;
					bisList = true;
				}
			}
		}
		if((bForward == true) && (count == 1))
		{
			if(isTabListAheadPoint() == true)
			{
//
// Check we're at the start of a block
//
				if(getPoint() == getCurrentBlock()->getPosition())
				{
					bisList = true;
					count = 2;
				}
			}

		}
		// Code to deal with font boundary problem.
		// TODO: This should really be fixed by someone who understands
		// how this code works! In the meantime save current font to be
		// restored after character is deleted.

		getCharFormat(&props_in);
		currentfont = UT_getAttribute("font-family",props_in);
		properties[1] = currentfont;

		_eraseInsertionPoint();
		UT_uint32 amt = count;
		UT_uint32 posCur = getPoint();
		UT_uint32 nposCur = getPoint();
		bool fontFlag = false;

		if (!bForward)
		{

			if (!_charMotion(bForward,count))
			{
				UT_ASSERT(getPoint() <= posCur);
				amt = posCur - getPoint();
			}

			posCur = getPoint();
			// Code to deal with change of font boundaries:
			if((posCur == nposCur) && (posCur > 0))
			{
				fontFlag = true;
				posCur--;
			}
		}
		else
		{
			PT_DocPosition posEOD;
			bool bRes;

			bRes = getEditableBounds(true, posEOD);
			UT_ASSERT(bRes);
			UT_ASSERT(posCur <= posEOD);

			if (posEOD < (posCur+amt))
			{
				amt = posEOD - posCur;
			}
		}

		if (amt > 0)
		{
			m_pDoc->disableListUpdates();

			nBlock = _findBlockAtPosition(getPoint());
			fl_AutoNum * pAuto = nBlock->getAutoNum();
			if(pAuto != NULL )
			{
				PL_StruxDocHandle sdh = nBlock->getStruxDocHandle();
				if((bisList == true) && (pAuto->getFirstItem() == sdh || pAuto->getLastItem() == sdh))
				{
					m_pDoc->StopList(sdh);
					PT_DocPosition listPoint,posEOD;
					getEditableBounds(true, posEOD);
					listPoint = getPoint();
					fl_AutoNum * pAuto = nBlock->getAutoNum();
					if(pAuto != NULL)
					{
						if(listPoint + 2 <= posEOD)
							_setPoint(listPoint+2);
						else
							_setPoint(posEOD);
					}
				}
				else if(bisList == true)
				{
					m_pDoc->deleteSpan(posCur, posCur+amt);
					nBlock->remItemFromList();
				}
				else
				{
					m_pDoc->deleteSpan(posCur, posCur+amt);
				}
			}
			else
			{
				m_pDoc->deleteSpan(posCur, posCur+amt);
			}

			if(fontFlag)
			{
				setCharFormat(properties);
			}
		}
//
// Dont leave a List field - tab on a line.
//
		if(isTabListAheadPoint())
		{
			m_pDoc->deleteSpan(getPoint(), getPoint()+2);
		}

		// restore updates and clean up dirty lists
		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();

		_generalUpdate();
		free(props_in);

		_ensureThatInsertionPointIsOnScreen();
	}

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
}

/*!
  Move insertion point to previous or next line
  \param bNext True if moving to next line

  This function moves the IP up or down one line, attempting to get as
  close as possible to the prior "sticky" x position.  The notion of
  "next" is strictly physical, not logical.

  For example, instead of always moving from the last line of one
  block to the first line of the next, you might wind up skipping over
  a bunch of blocks to wind up in the first line of the second column.	
*/
void FV_View::_moveInsPtNextPrevLine(bool bNext)
{
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
	UT_sint32 iLineHeight;
	UT_sint32 xPoint2;
	UT_sint32 yPoint2;
	bool bDirection;

//
// No need to to do background updates for 1 seconds.
//
	m_pLayout->setSkipUpdates(2);
	UT_sint32 xOldSticky = m_xPointSticky;

	// first, find the line we are on now
	UT_uint32 iOldPoint = getPoint();

	fl_BlockLayout* pOldBlock = _findBlockAtPosition(iOldPoint);
	fp_Run* pOldRun = pOldBlock->findPointCoords(getPoint(), m_bPointEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);
	fl_SectionLayout* pOldSL = pOldBlock->getSectionLayout();
	fp_Line* pOldLine = pOldRun->getLine();
	fp_Container* pOldContainer = pOldLine->getContainer();
	fp_Column * pOldColumn = NULL;
	fp_Column * pOldLeader = NULL;
	fp_Page* pOldPage = pOldContainer->getPage();
	bool bDocSection = (pOldSL->getType() == FL_SECTION_DOC) ||
					   (pOldSL->getType() == FL_SECTION_ENDNOTE);

	if (bDocSection)
	{
		pOldLeader = ((fp_Column*) (pOldContainer))->getLeader();
	}
	if(bDocSection)
	{
		pOldColumn = (fp_Column *) pOldContainer;
	}

	UT_sint32 iPageOffset;
	getPageYOffset(pOldPage, iPageOffset);

	UT_sint32 iLineX = 0;
	UT_sint32 iLineY = 0;

	pOldContainer->getOffsets(pOldLine, iLineX, iLineY);
	yPoint = iLineY;

	iLineHeight = pOldLine->getHeight();

	bool bNOOP = false;

	xxx_UT_DEBUGMSG(("fv_View::_moveInsPtNextPrevLine: old line 0x%x\n", pOldLine));
	
	if (bNext)
	{
		if (pOldLine != pOldContainer->getLastLine())
		{
			UT_sint32 iAfter = 1;
			yPoint += (iLineHeight + iAfter);
		}
		else if (bDocSection)
		{
			UT_sint32 count = (UT_sint32) pOldPage->countColumnLeaders();
			UT_sint32 i = 0;
			for(i =0; i < count ;i++) 
			{
				if( (fp_Column *) pOldPage->getNthColumnLeader(i) == pOldLeader)
				{
					break;
				}
			}
			if((i + 1) < count)
			{
				// Move to next container
				yPoint = pOldPage->getNthColumnLeader(i+1)->getY();
			}
			else
			{
				// move to next page
				fp_Page* pPage = pOldPage->getNext();
				if (pPage)
				{
					getPageYOffset(pPage, iPageOffset);
					yPoint = 0;
				}
				else
				{
					bNOOP = true;
				}
			}
		}
		else
		{
			bNOOP = true;
		}
	}
	else
	{
		if (pOldLine != pOldContainer->getFirstLine())
		{
			// just move off this line
			yPoint -= (pOldLine->getMarginBefore() + 1);
		}
		else if (bDocSection)
		{
			UT_sint32 count = (UT_sint32) pOldPage->countColumnLeaders();
			UT_sint32 i = 0;
			for(i =0; i < count ;i++) 
			{
				if( (fp_Column *) pOldPage->getNthColumnLeader(i) == pOldLeader)
				{
					break;
				}
			}
			if( (i> 0) && (i < count))
			{
				// Move to prev container
				yPoint = pOldPage->getNthColumnLeader(i-1)->getLastLine()->getY();
				yPoint +=  pOldPage->getNthColumnLeader(i-1)->getY()+2;
			}
			else
			{
				// move to prev page
				fp_Page* pPage = pOldPage->getPrev();
				if (pPage)
				{
					getPageYOffset(pPage, iPageOffset);
					yPoint = pPage->getBottom();
					if(getViewMode() != VIEW_PRINT)
					{
						fl_DocSectionLayout * pDSL = pPage->getOwningSection();
						yPoint = yPoint - pDSL->getTopMargin() -2;
					}
				}
				else
				{
					bNOOP = true;
				}
			}
		}
		else
		{
			bNOOP = true;
		}
	}

	if (bNOOP)
	{
		// cannot move.  should we beep?
		_drawInsertionPoint();
		return;
	}

	// change to screen coordinates
	xPoint = m_xPointSticky - m_xScrollOffset + getPageViewLeftMargin();
	yPoint += iPageOffset - m_yScrollOffset;

	// hit-test to figure out where that puts us
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPoint, yPoint, xClick, yClick);

	PT_DocPosition iNewPoint;
	bool bBOL = false;
	bool bEOL = false;
	fl_HdrFtrShadow * pShadow=NULL;
//
// If we're not in a Header/Footer we can't get off the page with the click
// version of mapXYToPosition
//
	if(isHdrFtrEdit())
	{
		pPage->mapXYToPositionClick(xClick, yClick, iNewPoint,pShadow, bBOL, bEOL);
	}
	else
	{
		pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL);
	}
//
// Check we're not moving out of allowed region.
//
	PT_DocPosition posBOD,posEOD;
	getEditableBounds(false,posBOD);
	getEditableBounds(true,posEOD);

	UT_DEBUGMSG(("iNewPoint=%d, iOldPoint=%d, xClick=%d, yClick=%d\n",iNewPoint, iOldPoint, xClick, yClick));
	UT_ASSERT(iNewPoint != iOldPoint);
	if((iNewPoint >= posBOD) && (iNewPoint <= posEOD) && 
	   ((bNext && (iNewPoint >= iOldPoint)) 
		|| (!bNext && (iNewPoint <= iOldPoint))))
	{
		_setPoint(iNewPoint, bEOL);
	}

	_ensureThatInsertionPointIsOnScreen();

	// this is the only place where we override changes to m_xPointSticky
	m_xPointSticky = xOldSticky;
}

bool FV_View::_ensureThatInsertionPointIsOnScreen(bool bDrawIP)
{
	bool bRet = false;

	if (m_iWindowHeight <= 0)
	{
		return false;
	}

	_fixInsertionPointCoords();
//
// If ==0 no layout information is present. Don't scroll.
//
	if( m_iPointHeight == 0)
	{
		return false;
	}

	//UT_DEBUGMSG(("_ensure: [xp %ld][yp %ld][ph %ld] [w %ld][h %ld]\n",m_xPoint,m_yPoint,m_iPointHeight,m_iWindowWidth,m_iWindowHeight));

	if (m_yPoint < 0)
	{
		cmdScroll(AV_SCROLLCMD_LINEUP, (UT_uint32) (-(m_yPoint)));
		bRet = true;
	}
	else if (((UT_uint32) (m_yPoint + m_iPointHeight)) >= ((UT_uint32) m_iWindowHeight))
	{
		cmdScroll(AV_SCROLLCMD_LINEDOWN, (UT_uint32)(m_yPoint + m_iPointHeight - m_iWindowHeight));
		bRet = true;
	}

	/*
	  TODO: we really ought to try to do better than this.
	*/
	if (m_xPoint < 0)
	{
		cmdScroll(AV_SCROLLCMD_LINELEFT, (UT_uint32) (-(m_xPoint) + getPageViewLeftMargin()/2));
		bRet = true;
	}
	else if (((UT_uint32) (m_xPoint)) >= ((UT_uint32) m_iWindowWidth))
	{
		cmdScroll(AV_SCROLLCMD_LINERIGHT, (UT_uint32)(m_xPoint - m_iWindowWidth + getPageViewLeftMargin()/2));
		bRet = true;
	}
	if(bRet == false && bDrawIP)
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	return bRet;
}

void FV_View::_moveInsPtNextPrevPage(bool bNext)
{
#if 0
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
#endif

	fp_Page* pOldPage = _getCurrentPage();

	// TODO when moving to the prev page, we should move to its end, not begining
	// try to locate next/prev page
	fp_Page* pPage = (bNext ? pOldPage->getNext() : pOldPage->getPrev());

	// if couldn't move, go to top of this page if we are looking for the previous page
	// or the end of this page if we are looking for the next page
	if (!pPage)
	{
		if(!bNext)
		{
			pPage = pOldPage;
		}
		else
		{
			moveInsPtTo(FV_DOCPOS_EOD,false);
			return;
		}
	}

	_moveInsPtToPage(pPage);
}

void FV_View::_moveInsPtNextPrevScreen(bool bNext)
{
	fl_BlockLayout * pBlock;
	fp_Run * pRun;
	UT_sint32 x,y,x2,y2;
	UT_uint32 iHeight;
	bool bDirection;
	
	_findPositionCoords(getPoint(),false,x,y,x2,y2,iHeight,bDirection,&pBlock,&pRun);
	if(!pRun)
		return;

	fp_Line * pLine = pRun->getLine();
	UT_ASSERT(pLine);

	fp_Line * pOrigLine = pLine;
	fp_Line * pPrevLine = pLine;
	fp_Container * pCont = pLine->getContainer();
	fp_Page * pPage  = pCont->getPage();
	getPageYOffset(pPage, y);
			
	y += pCont->getY() + pLine->getY();
	
	if(bNext)
		y-= pLine->getHeight();

	UT_uint32 iLineCount = 0;
	
	while(pLine)
	{
		iLineCount++;
		pCont = pLine->getContainer();
		pPage = pCont->getPage();
		getPageYOffset(pPage, y2);
		y2 += pCont->getY() + pLine->getY();
		if(!bNext)
			y2 -= pLine->getHeight();
				
		if(abs(y - y2) >=  m_iWindowHeight)
			break;
		
		pPrevLine = pLine;
		pLine = bNext ? pLine->getNext() : pLine->getPrev();
		if(!pLine)
		{
			fl_BlockLayout * pPrevBlock = pBlock;
			pBlock = bNext ? pPrevLine->getBlock()->getNext() : pPrevLine->getBlock()->getPrev();
			if(!pBlock)
			{
				// see if there is another section after/before this block
				fl_SectionLayout* pSection = bNext ? pPrevBlock->getSectionLayout()->getNext()
					                               : pPrevBlock->getSectionLayout()->getPrev();
				
				if(pSection && (pSection->getType() == FL_SECTION_DOC || pSection->getType() == FL_SECTION_ENDNOTE))
					pBlock = bNext ? pSection->getFirstBlock() : pSection->getLastBlock();
			}
			
			if(pBlock)
				pLine = bNext ? pBlock->getFirstLine() : pBlock->getLastLine();
		}
	}

	// if we do not have pLine, we will use pPrevLine
	// also, if we processed less than 3 lines, i.e, there is
	// a wide gap between our line an the next line, we want to
	// move to the next line even though we will not be able to
	// see the current line on screen any more
	if(iLineCount > 2 || !pLine)
		pLine  = pPrevLine;
	
	UT_ASSERT(pLine);
	if(!pLine)
		return;
	
	if(pLine == pOrigLine)
	{
		// need to do this, since the caller might have erased the point and will
		// expect us to draw in the new place
		_ensureThatInsertionPointIsOnScreen(true);
		return;
	}
	
	
	pRun = pLine->getFirstRun();
	UT_ASSERT(pRun);
	if(!pRun)
		return;
	
	pBlock = pRun->getBlock();
	UT_ASSERT(pBlock);
	if(!pBlock)
		return;
	
	moveInsPtTo(pBlock->getPosition(false) + pRun->getBlockOffset());
	_ensureThatInsertionPointIsOnScreen(true);
}


fp_Page *FV_View::_getCurrentPage(void)
{
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
	UT_sint32 xPoint2;
	UT_sint32 yPoint2;
	bool bDirection;
	/*
	  This function moves the IP to the beginning of the previous or
	  next page (ie not this one).
	*/

	// first, find the page we are on now
	UT_uint32 iOldPoint = getPoint();

	fl_BlockLayout* pOldBlock = _findBlockAtPosition(iOldPoint);
	fp_Run* pOldRun = pOldBlock->findPointCoords(getPoint(), m_bPointEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);
	fp_Line* pOldLine = pOldRun->getLine();
	fp_Container* pOldContainer = pOldLine->getContainer();
	fp_Page* pOldPage = pOldContainer->getPage();

	return pOldPage;
}

void FV_View::_moveInsPtNthPage(UT_uint32 n)
{
	fp_Page *page = m_pLayout->getFirstPage();

	if (n > m_pLayout->countPages ())
		n = m_pLayout->countPages ();

	for (UT_uint32 i = 1; i < n; i++)
	{
		page = page->getNext ();
	}

	_moveInsPtToPage(page);
}

void FV_View::_moveInsPtToPage(fp_Page *page)
{
	// move to the first pos on this page
	PT_DocPosition iNewPoint = page->getFirstLastPos(true);
	_setPoint(iNewPoint, false);

	// explicit vertical scroll to top of page
	UT_sint32 iPageOffset;
	getPageYOffset(page, iPageOffset);

	iPageOffset -= getPageViewSep() /2;
	iPageOffset -= m_yScrollOffset;
	
	bool bVScroll = false;
	if (iPageOffset < 0)
	{
		_eraseInsertionPoint(); 
		cmdScroll(AV_SCROLLCMD_LINEUP, (UT_uint32) (-iPageOffset));
		bVScroll = true;
	}
	else if (iPageOffset > 0)
	{
		_eraseInsertionPoint(); 
		cmdScroll(AV_SCROLLCMD_LINEDOWN, (UT_uint32)(iPageOffset));
		bVScroll = true;
	}

	// also allow implicit horizontal scroll, if needed
	if (!_ensureThatInsertionPointIsOnScreen(false) && !bVScroll)
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
}

/*!
  Move point by one screen
  \param bNext True if moving to next screen
*/
void FV_View::warpInsPtNextPrevScreen(bool bNext)
{
	if (!isSelectionEmpty())
	{
		_moveToSelectionEnd(bNext);
		_drawInsertionPoint();
		return;
	}
	else
		_eraseInsertionPoint();

	_resetSelection();
	_clearIfAtFmtMark(getPoint());
	_moveInsPtNextPrevScreen(bNext);

	notifyListeners(AV_CHG_MOTION);
}



/*!
  Move point to next or previous page
  \param bNext True if moving to next page

  \note Cursor movement while there's a selection has the effect of
		clearing the selection. And only that. See bug 993.
*/
void FV_View::warpInsPtNextPrevPage(bool bNext)
{
	if (!isSelectionEmpty())
	{
		_moveToSelectionEnd(bNext);
		_drawInsertionPoint();
		return;
	}
	else
		_eraseInsertionPoint();

	_resetSelection();
	_clearIfAtFmtMark(getPoint());
	_moveInsPtNextPrevPage(bNext);
	notifyListeners(AV_CHG_MOTION);
}

/*!
  Move point to next or previous line
  \param bNext True if moving to next line

  \note Cursor movement while there's a selection has the effect of
		clearing the selection. And only that. See bug 993.
*/
void FV_View::warpInsPtNextPrevLine(bool bNext)
{
	if (!isSelectionEmpty())
	{
		_moveToSelectionEnd(bNext);
		_drawInsertionPoint();
		return;
	}
	else
		_eraseInsertionPoint();

	_resetSelection();
	_clearIfAtFmtMark(getPoint());
	_moveInsPtNextPrevLine(bNext);
	notifyListeners(AV_CHG_MOTION);
}

void FV_View::extSelNextPrevLine(bool bNext)
{
	if (isSelectionEmpty())
	{
		_eraseInsertionPoint();
		_setSelectionAnchor();
		_clearIfAtFmtMark(getPoint());
		_moveInsPtNextPrevLine(bNext);
		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
		else
		{
			_drawSelection();
		}
	}
	else
	{
		PT_DocPosition iOldPoint = getPoint();
		_moveInsPtNextPrevLine(bNext);
		PT_DocPosition iNewPoint = getPoint();

		// top/bottom of doc - nowhere to go
		if (iOldPoint == iNewPoint)
			return;

		_extSel(iOldPoint);
		
		if (isSelectionEmpty())
		{
			_resetSelection();
			_drawInsertionPoint();
		}
	}

	notifyListeners(AV_CHG_MOTION);
}

// TODO preferably we should implement a new function for the simple
// page Up/Down that actually moves the insertion point (currently
// the PgUp and PgDown keys are bound to scrolling the window, but
// they do not move the insertion point, which is a real nuisance)
// once we fix that, we can use it in the function bellow to get
// a consistent behaviour
// 
// the number 100 is heuristic, it is to get the end of the selection
// on screen, but it does not work well with large gaps in the text
// and on page boundaries
#define TOP_OF_PAGE_OFFSET 100

void FV_View::extSelNextPrevScreen(bool bNext)
{
	if (isSelectionEmpty())
	{
		_eraseInsertionPoint();
		_setSelectionAnchor();
		_clearIfAtFmtMark(getPoint());
		_moveInsPtNextPrevScreen(bNext);

		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
		else
		{
			_drawSelection();
		}
	}
	else
	{
		PT_DocPosition iOldPoint = getPoint();
		_moveInsPtNextPrevScreen(bNext);

		// top/bottom of doc - nowhere to go
		if (iOldPoint == getPoint())
			return;

		_extSel(iOldPoint);
		
		if (isSelectionEmpty())
		{
			_resetSelection();
			_drawInsertionPoint();
		}
	}

	notifyListeners(AV_CHG_MOTION);
}

void FV_View::extSelNextPrevPage(bool bNext)
{
	if (isSelectionEmpty())
	{
		_eraseInsertionPoint();
		_setSelectionAnchor();
		_clearIfAtFmtMark(getPoint());
		_moveInsPtNextPrevPage(bNext);
		
		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
		else
		{
			_drawSelection();
		}
	}
	else
	{
		PT_DocPosition iOldPoint = getPoint();
		_moveInsPtNextPrevPage(bNext);

		// top/bottom of doc - nowhere to go
		if (iOldPoint == getPoint())
			return;

		_extSel(iOldPoint);
		
		if (isSelectionEmpty())
		{
			_resetSelection();
			_drawInsertionPoint();
		}
	}

	notifyListeners(AV_CHG_MOTION);
}

void FV_View::extSelHorizontal(bool bForward, UT_uint32 count)
{
	if (isSelectionEmpty())
	{
		_eraseInsertionPoint();
		_setSelectionAnchor();
		_charMotion(bForward, count);
	}
	else
	{
		PT_DocPosition iOldPoint = getPoint();

		if (_charMotion(bForward, count) == false)
		{
			_setPoint(iOldPoint);
			return;
		}
		
		_extSel(iOldPoint);
	}
	
	_ensureThatInsertionPointIsOnScreen(false);

	// It IS possible for the selection to be empty, even
	// after extending it.	If the charMotion fails, for example,
	// because we are at the end of a document, then the selection
	// will end up empty once again.

	if (isSelectionEmpty())
	{
		_resetSelection();
		_drawInsertionPoint();
	}
	else
	{
		_drawSelection();
	}

	notifyListeners(AV_CHG_MOTION);
}

void FV_View::extSelTo(FV_DocPos dp)
{
	PT_DocPosition iPos = _getDocPos(dp);

	_extSelToPos(iPos);

	if (!_ensureThatInsertionPointIsOnScreen(false))
	{
		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}

	notifyListeners(AV_CHG_MOTION);
}

#define AUTO_SCROLL_MSECS	100

void FV_View::_autoScroll(UT_Worker * pWorker)
{
	UT_ASSERT(pWorker);
	
	// this is a static callback method and does not have a 'this' pointer.

	FV_View * pView = (FV_View *) pWorker->getInstanceData();
	UT_ASSERT(pView);

	if(pView->getLayout()->getDocument()->isPieceTableChanging())
	{
		return;
	}

	PT_DocPosition iOldPoint = pView->getPoint();

	/*
	  NOTE: We update the selection here, so that the timer can keep
	  triggering autoscrolls even if the mouse doesn't move.
	*/
	pView->extSelToXY(pView->m_xLastMouse, pView->m_yLastMouse, false);

	if (pView->getPoint() != iOldPoint)
	{
		// do the autoscroll
		if (!pView->_ensureThatInsertionPointIsOnScreen(false))
		{
			pView->_fixInsertionPointCoords();
		}
	}
	else
	{
		// not far enough to change the selection ... do we still need to scroll?
		UT_sint32 xPos = pView->m_xLastMouse;
		UT_sint32 yPos = pView->m_yLastMouse;

		// TODO: clamp xPos, yPos to viewable area??

		bool bOnScreen = true;

		if ((xPos < 0 || xPos > pView->m_iWindowWidth) ||
			(yPos < 0 || yPos > pView->m_iWindowHeight))
			bOnScreen = false;
		
		if (!bOnScreen)
		{
			// yep, do it manually

			// TODO currently we blindly send these auto scroll events without regard
			// TODO to whether the window can scroll any further in that direction.
			// TODO we could optimize this a bit and check the scroll range before we
			// TODO fire them, but that knowledge is only stored in the frame and we
			// TODO don't have a backpointer to it.
			// UT_DEBUGMSG(("_auto: [xp %ld][yp %ld] [w %ld][h %ld]\n",
			//			 xPos,yPos,pView->m_iWindowWidth,pView->m_iWindowHeight));
			//
			// Sevior: Is This what you wanted? Uncomment these lines when
			// needed.
			//
			//XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
			//UT_ASSERT((pFrame));

			if (yPos < 0)
			{
				pView->_eraseInsertionPoint();	
				pView->cmdScroll(AV_SCROLLCMD_LINEUP, (UT_uint32) (-(yPos)));
			}
			else if (((UT_uint32) (yPos)) >= ((UT_uint32) pView->m_iWindowHeight))
			{
				pView->_eraseInsertionPoint();	
				pView->cmdScroll(AV_SCROLLCMD_LINEDOWN, (UT_uint32)(yPos - pView->m_iWindowHeight));
			}

			if (xPos < 0)
			{
				pView->_eraseInsertionPoint();	
				pView->cmdScroll(AV_SCROLLCMD_LINELEFT, (UT_uint32) (-(xPos)));
			}
			else if (((UT_uint32) (xPos)) >= ((UT_uint32) pView->m_iWindowWidth))
			{
				pView->_eraseInsertionPoint();	
				pView->cmdScroll(AV_SCROLLCMD_LINERIGHT, (UT_uint32)(xPos - pView->m_iWindowWidth));
			}
		}
	}
}


fp_Page* FV_View::_getPageForXY(UT_sint32 xPos, UT_sint32 yPos, UT_sint32& xClick, UT_sint32& yClick) const
{
	xClick = xPos + m_xScrollOffset - getPageViewLeftMargin();
	yClick = yPos + m_yScrollOffset - getPageViewTopMargin();
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		UT_sint32 iPageHeight = pPage->getHeight();
		if(getViewMode() != VIEW_PRINT)
		{
			iPageHeight = iPageHeight - pPage->getOwningSection()->getTopMargin() - 
				pPage->getOwningSection()->getBottomMargin();
		}
		if (yClick < iPageHeight)
		{
			// found it
			break;
		}
		else
		{
			yClick -= iPageHeight + getPageViewSep();
		}
		pPage = pPage->getNext();
	}

	if (!pPage)
	{
		// we're below the last page
		pPage = m_pLayout->getLastPage();

		UT_sint32 iPageHeight = pPage->getHeight();
		yClick += iPageHeight + getPageViewSep();
	}

	return pPage;
}

void FV_View::extSelToXY(UT_sint32 xPos, UT_sint32 yPos, bool bDrag)
{
	// Figure out which page we clicked on.
	// Pass the click down to that page.
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);

	PT_DocPosition iNewPoint;
	bool bBOL = false;
	bool bEOL = false;
	fl_HdrFtrShadow * pShadow = NULL;
	pPage->mapXYToPositionClick(xClick, yClick, iNewPoint,pShadow, bBOL, bEOL);

	bool bPostpone = false;

	if (bDrag)
	{
		// figure out whether we're still on screen
		bool bOnScreen = true;

		if ((xPos < 0 || xPos > m_iWindowWidth) ||
			(yPos < 0 || yPos > m_iWindowHeight))
			bOnScreen = false;
		// is autoscroll timer set properly?
		if (bOnScreen)
		{
			if (m_pAutoScrollTimer)
			{
				// timer not needed any more, so stop it
				m_pAutoScrollTimer->stop();
			}
		}
		else
		{
			// remember where mouse is
			m_xLastMouse = xPos;
			m_yLastMouse = yPos;

			// offscreen ==> make sure it's set
			if (!m_pAutoScrollTimer)
			{
				m_pAutoScrollTimer = UT_Timer::static_constructor(_autoScroll, this, m_pG);
				if (m_pAutoScrollTimer)
					m_pAutoScrollTimer->set(AUTO_SCROLL_MSECS);
			}
			else
			{
				m_pAutoScrollTimer->start();
			}
			
			// postpone selection until timer fires
			bPostpone = true;
		}
	}
	
	if (!bPostpone)
	{
		_extSelToPos(iNewPoint);
		notifyListeners(AV_CHG_MOTION);
	}
}

void FV_View::extSelToXYword(UT_sint32 xPos, UT_sint32 yPos, bool bDrag)
{
	// extend the current selection to
	// include the WORD at the given XY.
	// this should behave just like extSelToXY()
	// but with WORD-level granularity.
	
	// Figure out which page we clicked on.
	// Pass the click down to that page.
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);

	PT_DocPosition iNewPoint;
	bool bBOL, bEOL;

	bBOL = bEOL = false;
	fl_HdrFtrShadow * pShadow = NULL;
	pPage->mapXYToPositionClick(xClick, yClick, iNewPoint,pShadow, bBOL, bEOL);

	//UT_ASSERT(!isSelectionEmpty());

	if (iNewPoint <= m_iSelectionLeftAnchor)
	{
		m_iSelectionAnchor = m_iSelectionRightAnchor;
	}
	else
	{
		m_iSelectionAnchor = m_iSelectionLeftAnchor;
	}

	const FV_DocPos argDocPos =
		iNewPoint > m_iSelectionAnchor ? FV_DOCPOS_EOW_SELECT : FV_DOCPOS_BOW;
	const PT_DocPosition iNewPointWord = _getDocPosFromPoint(iNewPoint, argDocPos,false);

	bool bPostpone = false;

	if (bDrag)
	{
		// figure out whether we're still on screen
		bool bOnScreen = true;

		if ((xPos < 0 || xPos > m_iWindowWidth) ||
			(yPos < 0 || yPos > m_iWindowHeight))
			bOnScreen = false;
		
		// is autoscroll timer set properly?
		if (bOnScreen)
		{
			if (m_pAutoScrollTimer)
			{
				// timer not needed any more, so stop it
				m_pAutoScrollTimer->stop();
			}
		}
		else
		{
			// remember where mouse is
			m_xLastMouse = xPos;
			m_yLastMouse = yPos;

			// offscreen ==> make sure it's set
			if (!m_pAutoScrollTimer)
			{
				m_pAutoScrollTimer = UT_Timer::static_constructor(_autoScroll, this, m_pG);
				if (m_pAutoScrollTimer)
					m_pAutoScrollTimer->set(AUTO_SCROLL_MSECS);
			}
			else
			{
				m_pAutoScrollTimer->start();
			}
			
			// postpone selection until timer fires
			bPostpone = true;
		}
	}
	
	if (!bPostpone)
	{
		_extSelToPos(iNewPointWord);
		notifyListeners(AV_CHG_MOTION);
	}
}

void FV_View::endDrag(UT_sint32 xPos, UT_sint32 yPos)
{
	if (!m_pAutoScrollTimer)
		return;

	// figure out whether we're still on screen
	bool bOnScreen = true;

	if ((xPos < 0 || xPos > m_iWindowWidth) ||
		(yPos < 0 || yPos > m_iWindowHeight))
		bOnScreen = false;
	
	if (!bOnScreen)
	{
		// remember where mouse is
		m_xLastMouse = xPos;
		m_yLastMouse = yPos;

		// finish pending autoscroll
		m_pAutoScrollTimer->fire();
	}

	// timer not needed any more, so stop it
	m_pAutoScrollTimer->stop();
}

// ---------------- start goto ---------------

bool FV_View::gotoTarget(AP_JumpTarget type, UT_UCSChar *data)
{
	UT_ASSERT(m_pLayout);
	bool inc = false;
	bool dec = false;

	char * numberString = (char *) calloc(UT_UCS_strlen(data) + 1, sizeof(char));
	UT_ASSERT(numberString);

	UT_UCS_strcpy_to_char(numberString, data);
	if (!isSelectionEmpty())
	{
		_clearSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	switch (numberString[0])
	{
	case '+':
		inc = true;
		numberString++;
		break;
	case '-':
		dec = true;
		numberString++;
		break;
	}

	UT_uint32 number = 0;
	if(type != AP_JUMPTARGET_BOOKMARK)
		number = atol(numberString);

	if (dec || inc)
		numberString--;

	// check for range
	//	if (number < 0 || number > (UT_uint32) m_pLayout->countPages())
	//		return false;

	switch (type)
	{
	case AP_JUMPTARGET_PAGE:
	{
		if (!inc && !dec)
			_moveInsPtNthPage (number);
		else
		{
			fp_Page* pOldPage = _getCurrentPage();
			fp_Page* pPage = pOldPage;
			fp_Page* pTmpPage = pOldPage;

			if (inc) // TODO:  What if number passes the number of pages?
				for (UT_uint32 i = 0; i < number; i++)
				{
					if ((pTmpPage = pPage->getNext ()) != NULL)
						pPage = pTmpPage;
					else
						break;
				}
			else
				for (UT_uint32 i = 0; i < number; i++)
				{
					if ((pTmpPage = pPage->getPrev ()) != NULL)
						pPage = pTmpPage;
					else
						break;
				}

			if (!pPage)
				pPage = pOldPage;

			_moveInsPtToPage (pPage);
		}

		notifyListeners(AV_CHG_MOTION);

		break;
	}
	case AP_JUMPTARGET_LINE:
		if (inc || dec)
		{
			bool bNext;
			bNext = inc;

			for (UT_uint32 i = 0; i < number; i++)
			{
				_moveInsPtNextPrevLine (bNext); // HACK: A like the quick hacks... :)
			}
		}
		else
		{
			//UT_uint32 line = 0;
			fl_SectionLayout * pSL = m_pLayout->getFirstSection();
			fl_BlockLayout * pBL = pSL->getFirstBlock();
			fp_Line* pLine = pBL->getFirstLine();

			for (UT_uint32 i = 1; i < number; i++)
			{
				fp_Line* pOldLine = pLine;

				if ((pLine = pLine->getNext ()) == NULL)
				{
					if ((pBL = pBL->getNext ()) == NULL)
					{
						if ((pSL = pSL->getNext ()) == NULL)
						{
							pLine = pOldLine;
							break;
						}
						else
							pBL = pSL->getFirstBlock ();
					}
					else
					{
						pLine = pBL->getFirstLine ();
					}
				}
			}

			fp_Run* frun = pLine->getFirstRun ();
			fl_BlockLayout* fblock = pLine->getBlock ();
			PT_DocPosition dp = frun->getBlockOffset () + fblock->getPosition ();
			moveInsPtTo (dp);
		}

		notifyListeners(AV_CHG_MOTION);

		break;
	case AP_JUMPTARGET_PICTURE:
		// TODO
		break;
	case AP_JUMPTARGET_BOOKMARK:
		{
			fl_SectionLayout * pSL = m_pLayout->getFirstSection();
			fl_BlockLayout * pBL;
			fp_Run* pRun;
			fp_BookmarkRun * pB[2] = {NULL,NULL};

			UT_uint32 i = 0;
			bool bFound = false;

			UT_DEBUGMSG(("fv_View::gotoTarget: bookmark [%s]\n",numberString));
			if(UT_isUrl(numberString))
			{
				XAP_Frame * pFrame = (XAP_Frame *) getParentData();
				UT_ASSERT((pFrame));
				pFrame->openURL(numberString);
				return false;
			}
			if(m_pDoc->isBookmarkUnique((const XML_Char *)numberString))
				goto book_mark_not_found; //bookmark does not exist

			while(pSL)
			{
				pBL = pSL->getFirstBlock();

				while(pBL)
				{
					pRun = pBL->getFirstRun();

					while(pRun)
					{
						if(pRun->getType()== FPRUN_BOOKMARK)
						{
							fp_BookmarkRun * pBR = static_cast<fp_BookmarkRun*>(pRun);
							if(!strcmp(pBR->getName(),numberString))
							{
								pB[i] = pBR;
								i++;
								if(i>1)
								{
									bFound = true;
									break;
								}
							}
						}
						if(bFound)
							break;
						pRun = pRun->getNext();
					}
					if(bFound)
						break;
					pBL = pBL->getNext();
				}
				if(bFound)
					break;
				pSL = pSL->getNext();
			}

			if(pB[0] && pB[1])
			{
				_clearSelection();
				PT_DocPosition dp1 = pB[0]->getBlock()->getPosition(false) + pB[0]->getBlockOffset();
				PT_DocPosition dp2 = pB[1]->getBlock()->getPosition(false) + pB[1]->getBlockOffset();

				if(dp2 - dp1 == 1)
					moveInsPtTo (dp2);
				else
				{
					//make a selection
					m_bSelection = true;
					m_iSelectionAnchor = dp2;
					setPoint(dp1);
					_drawSelection();
				}

			}
			else

book_mark_not_found:
			{
				//bookmark not found
				XAP_Frame * pFrame = (XAP_Frame *) getParentData();
				UT_ASSERT((pFrame));

				const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
				const char *pMsg1 = pSS->getValue(AP_STRING_ID_MSG_BookmarkNotFound);

				UT_ASSERT(pMsg1);

				char * szMsg = new char[strlen(pMsg1) + BOOKMARK_NAME_SIZE + 10];
				UT_ASSERT((szMsg));

				sprintf(szMsg, pMsg1, numberString);

				pFrame->showMessageBox(szMsg, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);

				delete[] szMsg;
				return true;
			}

		}

		notifyListeners(AV_CHG_MOTION);
		break;
	default:
		// TODO
		;
	}

	FREEP(numberString);

	if (isSelectionEmpty())
	{
		_ensureThatInsertionPointIsOnScreen();
	}
	else
	{
		_ensureThatInsertionPointIsOnScreen(false);
	}

	return false;
}

// ---------------- start find and replace ---------------

/*!
 Find next occurrence of string
 \param pFind String to find
 \param bMatchCase True to match case, false to ignore case
 \result bDoneEntireDocument True if entire document searched,
		 false otherwise
 \return True if string was found, false otherwise
*/
bool
FV_View::findNext(const UT_UCSChar* pFind, bool bMatchCase,
				  bool& bDoneEntireDocument)
{
	if (!isSelectionEmpty())
	{
		_clearSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	UT_uint32* pPrefix = _computeFindPrefix(pFind, bMatchCase);
	bool bRes = _findNext(pFind, pPrefix, bMatchCase, bDoneEntireDocument);
	FREEP(pPrefix);

	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
	else
	{
		_ensureThatInsertionPointIsOnScreen();
		_drawSelection();
	}

	// TODO do we need to do a notifyListeners(AV_CHG_MOTION) -- yes
	notifyListeners(AV_CHG_MOTION);
	return bRes;
}

/*!
 Compute prefix function for search
 \param pFind String to find
 \param bMatchCase True to match case, false to ignore case
*/
UT_uint32*
FV_View::_computeFindPrefix(const UT_UCSChar* pFind, bool bMatchCase)
{
	UT_uint32 m = UT_UCS_strlen(pFind);
	UT_uint32 k = 0, q = 1;
	UT_uint32 *pPrefix = (UT_uint32*) calloc(m, sizeof(UT_uint32));
	UT_ASSERT(pPrefix);

	pPrefix[0] = 0; // Must be this regardless of the string

	if (bMatchCase)
	{
		for (q = 1; q < m; q++)
		{
			while (k > 0 && pFind[k] != pFind[q])
				k = pPrefix[k - 1];
			if(pFind[k] == pFind[q])
				k++;
			pPrefix[q] = k;
		}
	}
	else
	{
		for (q = 1; q < m; q++)
		{
			while (k > 0
				   && UT_UCS_tolower(pFind[k]) != UT_UCS_tolower(pFind[q]))
				k = pPrefix[k - 1];
			if(UT_UCS_tolower(pFind[k]) == UT_UCS_tolower(pFind[q]))
				k++;
			pPrefix[q] = k;
		}
	}

	return pPrefix;
}

/*!
 Find next occurrence of string
 \param pFind String to find
 \param True to match case, false to ignore case
 \result bDoneEntireDocument True if entire document searched,
		 false otherwise
 \return True if string was found, false otherwise

 \fixme The conversion of UCS_RQUOTE should happen in some generic
		function - it is presently done lot's of places in the code.
*/
bool
FV_View::_findNext(const UT_UCSChar* pFind, UT_uint32* pPrefix,
				   bool bMatchCase, bool& bDoneEntireDocument)
{
	UT_ASSERT(pFind);

	fl_BlockLayout* block = _findGetCurrentBlock();
	PT_DocPosition offset = _findGetCurrentOffset();
	UT_UCSChar* buffer = NULL;
	UT_uint32 m = UT_UCS_strlen(pFind);

	// Clone the search string, converting it to lowercase is search
	// should ignore case.
	UT_UCSChar* pFindStr = (UT_UCSChar*) calloc(m, sizeof(UT_UCSChar));
	UT_ASSERT(pFindStr);
	if (!pFindStr)
		return false;
	UT_uint32 j;
	if (bMatchCase)
	{
		for (j = 0; j < m; j++)
			pFindStr[j] = pFind[j];
	}
	else
	{
		for (j = 0; j < m; j++)
			pFindStr[j] = UT_UCS_tolower(pFind[j]);
	}

	// Now we use the prefix function (stored as an array) to search
	// through the document text.
	while ((buffer = _findGetNextBlockBuffer(&block, &offset)))
	{
		UT_sint32 foundAt = -1;
		UT_uint32 i = 0, t = 0;

		if (bMatchCase)
		{
			UT_UCSChar currentChar;

			while ((currentChar = buffer[i]) /*|| foundAt == -1*/)
			{
				// Convert smart quote apostrophe to ASCII single quote to
				// match seach input
				if (currentChar == UCS_RQUOTE) currentChar = '\'';

				while (t > 0 && pFindStr[t] != currentChar)
					t = pPrefix[t-1];
				if (pFindStr[t] == currentChar)
					t++;
				i++;
				if (t == m)
				{
					foundAt = i - m;
					break;
				}
			}
		}
		else
		{
			UT_UCSChar currentChar;

			while ((currentChar = buffer[i]) /*|| foundAt == -1*/)
			{
				// Convert smart quote apostrophe to ASCII single quote to
				// match seach input
				if (currentChar == UCS_RQUOTE) currentChar = '\'';

				currentChar = UT_UCS_tolower(currentChar);

				while (t > 0 && pFindStr[t] != currentChar)
					t = pPrefix[t-1];
				if (pFindStr[t] == currentChar)
					t++;
				i++;
				if (t == m)
				{
					foundAt = i - m;
					break;
				}
			}
		}


		// Select region of matching string if found
		if (foundAt != -1)
		{
			_setPoint(block->getPosition(false) + offset + foundAt);
			_setSelectionAnchor();
			_charMotion(true, m);

			m_doneFind = true;

			FREEP(pFindStr);
			FREEP(buffer);
			return true;
		}

		// Didn't find anything, so set the offset to the end of the
		// current area
		offset += UT_UCS_strlen(buffer);

		// Must clean up buffer returned for search
		FREEP(buffer);
	}

	bDoneEntireDocument = true;

	// Reset wrap for next time
	m_wrappedEnd = false;
	
	FREEP(pFindStr);

	return false;
}

/*!
 Find operation reset

 This function is called at the start of a new find operation to reset
 the search location parameters. 
*/
void
FV_View::findSetStartAtInsPoint(void)
{
	m_startPosition = m_iInsPoint;
	m_wrappedEnd = false;
	m_doneFind = false;
}

PT_DocPosition
FV_View::_BlockOffsetToPos(fl_BlockLayout * block, PT_DocPosition offset)
{
	UT_ASSERT(block);
	return block->getPosition(false) + offset;
}

UT_UCSChar*
FV_View::_findGetNextBlockBuffer(fl_BlockLayout** pBlock, 
								 PT_DocPosition* pOffset)
{
	UT_ASSERT(m_pLayout);

	// This assert doesn't work, since the startPosition CAN
	// legitimately be zero
	// The beginning of the first block in any document
	UT_ASSERT(m_startPosition >= 2);
	
	UT_ASSERT(pBlock);
	UT_ASSERT(*pBlock);

	UT_ASSERT(pOffset);
	
	fl_BlockLayout* newBlock = NULL;
	PT_DocPosition newOffset = 0;

	UT_uint32 bufferLength = 0;
	
	UT_GrowBuf pBuffer;

	// Check early for completion, from where we left off last, and
	// bail if we are now at or past the start position
	if (m_wrappedEnd
		&& _BlockOffsetToPos(*pBlock, *pOffset) >= m_startPosition)
	{
		// We're done
		return NULL;
	}

	if (!(*pBlock)->getBlockBuf(&pBuffer))
	{
		UT_DEBUGMSG(("Block %p has no associated buffer.\n", *pBlock));
		UT_ASSERT(0);
	}
	
	// Have we already searched all the text in this buffer?
	if (*pOffset >= pBuffer.getLength())
	{
		// Then return a fresh new block's buffer
		newBlock = (*pBlock)->getNextBlockInDocument();

		// Are we at the end of the document?
		if (!newBlock)
		{
			// Then wrap (fetch the first block in the doc)
			PT_DocPosition startOfDoc;
			getEditableBounds(false, startOfDoc);
			
			newBlock = m_pLayout->findBlockAtPosition(startOfDoc);

			m_wrappedEnd = true;
			
			UT_ASSERT(newBlock);
		}

		// Re-assign the buffer contents for our new block
		pBuffer.truncate(0);
		// The offset starts at 0 for a fresh buffer
		newOffset = 0;
		
		if (!newBlock->getBlockBuf(&pBuffer))
		{
			UT_DEBUGMSG(("Block %p (a ->next block) has no buffer.\n", 
						 newBlock));
			UT_ASSERT(0);
		}

		// Good to go with a full buffer for our new block
	}
	else
	{
		// We have some left to go in this buffer.	Buffer is still
		// valid, just copy pointers
		newBlock = *pBlock;
		newOffset = *pOffset;
	}

	// Are we going to run into the start position in this buffer?	If
	// so, we need to size our length accordingly
	if (m_wrappedEnd && _BlockOffsetToPos(newBlock, newOffset) + pBuffer.getLength() >= m_startPosition)
	{
		bufferLength = (m_startPosition - (newBlock)->getPosition(false)) - newOffset;
	}
	else
	{
		bufferLength = pBuffer.getLength() - newOffset;
	}
	
	// clone a buffer (this could get really slow on large buffers!)
	UT_UCSChar* bufferSegment = NULL;

	// remember, the caller gets to free this memory
	bufferSegment = (UT_UCSChar*)calloc(bufferLength + 1, sizeof(UT_UCSChar));
	UT_ASSERT(bufferSegment);
	
	memmove(bufferSegment, pBuffer.getPointer(newOffset),
			(bufferLength) * sizeof(UT_UCSChar));

	// before we bail, hold up our block stuff for next round
	*pBlock = newBlock;
	*pOffset = newOffset;
		
	return bufferSegment;
}

/*!
 Set find-next string
 \param pFind String to find
 \param bMatchCase True to match case, false to ignore case
 \return True if there was enough memory to clone the string
 This function is used for non-dialog search operations.
*/
bool
FV_View::findSetNextString(UT_UCSChar* pFind, bool bMatchCase)
{
	UT_ASSERT(pFind);

	// update case matching
	_m_matchCase = bMatchCase;

	// update string
	FREEP(_m_findNextString);
	return UT_UCS_cloneString(&_m_findNextString, pFind);
}

/*!
 Find next occurrence of last searched string
 \return True if found, otherwise false
 This function is used for non-dialog search operations.
*/
bool
FV_View::findAgain(void)
{
	bool bRes = false;

	if (_m_findNextString && *_m_findNextString)
	{
		bool bTmp;
		bRes = findNext(_m_findNextString, _m_matchCase, bTmp);
		if (bRes)
		{
			_drawSelection();
		}
	}

	return bRes;
}

/*!
 Find and replace text unit
 \param pFind String to find
 \param pReplace String to replace it with
 \param pPrefix Search prefix function
 \param bMatchCase True to match case, false to ignore case
 \result bDoneEntireDocument True if entire document searched,
		 false otherwise
 \return True if string was replaced, false otherwise

 This function will replace an existing selection with pReplace. It
 will then do a search for pFind.
*/
bool
FV_View::_findReplace(const UT_UCSChar* pFind, const UT_UCSChar* pReplace,
					  UT_uint32* pPrefix, bool bMatchCase,
					  bool& bDoneEntireDocument)
{
	UT_ASSERT(pFind && pReplace);

	bool bRes = false;

	_saveAndNotifyPieceTableChange();
	m_pDoc->beginUserAtomicGlob();

	// Replace selection if it's due to a find operation
	if (m_doneFind && !isSelectionEmpty())
	{
		bRes = true;

		PP_AttrProp AttrProp_Before;

		if (!isSelectionEmpty())
		{
			_eraseInsertionPoint();
			_deleteSelection(&AttrProp_Before);
		}
		else
		{
			_eraseInsertionPoint();
		}

		// If we have a string with length, do an insert, else let it
		// hang from the delete above
		if (*pReplace)
			bRes = m_pDoc->insertSpan(getPoint(), pReplace,
									  UT_UCS_strlen(pReplace),
									  &AttrProp_Before);

		// Do not increase the insertion point index, since the insert
		// span will leave us at the correct place.

		_generalUpdate();

		// If we've wrapped around once, and we're doing work before
		// we've hit the point at which we started, then we adjust the
		// start position so that we stop at the right spot.
		if (m_wrappedEnd && !bDoneEntireDocument)
		{
			m_startPosition += (long) UT_UCS_strlen(pReplace);
			m_startPosition -= (long) UT_UCS_strlen(pFind);
		}

		UT_ASSERT(m_startPosition >= 2);
	}

	m_pDoc->endUserAtomicGlob();
	_restorePieceTableState();

	// Find next occurrence in document
	_findNext(pFind, pPrefix, bMatchCase, bDoneEntireDocument);
	return bRes;
}

/*!
 Find and replace string
 \param pFind String to find
 \param pReplace String to replace it with
 \param bMatchCase True to match case, false to ignore case
 \result bDoneEntireDocument True if entire document searched,
		 false otherwise
 \return True if string was replaced, false otherwise
*/ 
bool
FV_View::findReplace(const UT_UCSChar* pFind, const UT_UCSChar* pReplace,
					 bool bMatchCase, bool& bDoneEntireDocument)
{
	UT_ASSERT(pFind && pReplace);

	UT_uint32* pPrefix = _computeFindPrefix(pFind, bMatchCase);
	bool bRes = _findReplace(pFind, pReplace, pPrefix, 
							 bMatchCase, bDoneEntireDocument);
	FREEP(pPrefix);

	updateScreen();
	
	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
	else
	{
		_ensureThatInsertionPointIsOnScreen();
		_drawSelection();
	}

	return bRes;
}

/*!
 Find and replace all occurrences of string
 \param pFind String to find
 \param pReplace String to replace it with
 \param bMatchCase True to match case, false to ignore case
 \return Number of replacements made
*/ 
UT_uint32
FV_View::findReplaceAll(const UT_UCSChar* pFind, const UT_UCSChar* pReplace,
						bool bMatchCase)
{
	UT_uint32 iReplaced = 0;
	m_pDoc->beginUserAtomicGlob();

	bool bDoneEntireDocument = false;

	// Compute search prefix
	UT_uint32* pPrefix = _computeFindPrefix(pFind, bMatchCase);
	
	// Prime with the first match - _findReplace is really
	// replace-then-find.
	_findNext(pFind, pPrefix, bMatchCase, bDoneEntireDocument);

	// Keep replacing until the end of the buffer is hit
	while (!bDoneEntireDocument)
	{
		_findReplace(pFind, pReplace, pPrefix, bMatchCase,bDoneEntireDocument);
		iReplaced++;
	}

	m_pDoc->endUserAtomicGlob();
	
	_generalUpdate();
	
	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
	else
	{
		_ensureThatInsertionPointIsOnScreen();
	}

	FREEP(pPrefix);
	return iReplaced;
}

fl_BlockLayout*
FV_View::_findGetCurrentBlock(void)
{
	return _findBlockAtPosition(m_iInsPoint);
}


fl_BlockLayout*
FV_View::getCurrentBlock(void)
{
	return _findGetCurrentBlock();
}

PT_DocPosition
FV_View::_findGetCurrentOffset(void)
{
	return (m_iInsPoint - _findGetCurrentBlock()->getPosition(false));
}

// Any takers?
UT_sint32
FV_View::_findBlockSearchRegexp(const UT_UCSChar* /* haystack */,
								const UT_UCSChar* /* needle */)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	
	return -1;
}

// ---------------- end find and replace ---------------

void FV_View::insertSymbol(UT_UCSChar c, XML_Char * symfont)
{

	// First check to see if there is a selection already.
	// if so delete it then get the current font
	m_pDoc->beginUserAtomicGlob();

	if (!isSelectionEmpty())
	{
		_deleteSelection();
		_generalUpdate();
	}
	// We have to determine the current font so we can put it back after
	// Inserting the Symbol

	const XML_Char ** props_in = NULL;
	const XML_Char * currentfont;
	getCharFormat(&props_in);
	currentfont = UT_getAttribute("font-family",props_in);
	free(props_in);

	if(strstr(symfont,currentfont) == NULL)
	{
		// Set the font
		const XML_Char* properties[] = { "font-family", 0, 0 };
		properties[1] = symfont ;
		setCharFormat(properties);

		// Insert the character
		cmdCharInsert(&c,1);

		// Change the font back to its original value
		properties[1] = currentfont;
		setCharFormat(properties);
		_generalUpdate();
	}
	else
	{
		// Just insert the character

		cmdCharInsert(&c,1);
	}
	m_pDoc->endUserAtomicGlob();
}

/*
  After most editing commands, it is necessary to call this method,
  _generalUpdate, in order to fix everything.
*/
void FV_View::_generalUpdate(void)
{
	if(!shouldScreenUpdateOnGeneralUpdate())
		return;
	m_pDoc->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
	
	// now we have updated the overall layout, next we have to
	// update all layout-dependent fields, if the size of any of the fields
	// changes, then we have to redo the layout again ... we should really
	// do this in a loop, but if it does not change in the second go, we will
	// leave it for the time being
    //
    // Sevior I moved what's n
#if 0	
	bool bChange =	false;
	fl_SectionLayout * pSL = m_pLayout->getFirstSection();
	while(pSL)
	{
		bChange |= pSL->recalculateFields(true);
		pSL  = pSL->getNext();
	}
	
	if(bChange)
	{
		UT_DEBUGMSG(("fv_View::_generalUpdate: width of some fields changed, pass 2\n"));
		m_pDoc->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
	}
#endif
//
// No need to update other stuff if we're doing a preview
//
	if(isPreview())
		return;
	/*
	  TODO note that we are far too heavy handed with the mask we
	  send here.  I ripped out all the individual calls to notifyListeners
	  which appeared within fl_BlockLayout, and they all now go through
	  here.  For that reason, I made the following mask into the union
	  of all the masks I found.  I assume that this is inefficient, but
	  functionally correct.

	  TODO WRONG! WRONG! WRONG! notifyListener() must be called in
	  TODO WRONG! WRONG! WRONG! fl_BlockLayout in response to a change
	  TODO WRONG! WRONG! WRONG! notification and not here.	this call
	  TODO WRONG! WRONG! WRONG! will only update the current window.
	  TODO WRONG! WRONG! WRONG! having the notification in fl_BlockLayout
	  TODO WRONG! WRONG! WRONG! will get each view on the document.
	*/
//
//	notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR | AV_CHG_FMTBLOCK );
	notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR | AV_CHG_FMTBLOCK | AV_CHG_PAGECOUNT | AV_CHG_FMTSTYLE );
}


void FV_View::_extSel(UT_uint32 iOldPoint)
{
	/*
	  We need to calculate the differences between the old
	  selection and new one.

	  Anything which was selected, and now is not, should
	  be fixed on screen, back to normal.

	  Anything which was NOT selected, and now is, should
	  be fixed on screen, to show it in selected state.

	  Anything which was selected, and is still selected,
	  should NOT be touched.

	  And, obviously, anything which was not selected, and
	  is still not selected, should not be touched.
	*/
	bool bres;
	UT_uint32 iNewPoint = getPoint();

	PT_DocPosition posBOD,posEOD,dNewPoint,dOldPoint;
	dNewPoint = (PT_DocPosition) iNewPoint;
	dOldPoint = (PT_DocPosition) iOldPoint;
	getEditableBounds(false,posBOD);
	getEditableBounds(true,posEOD);
	if(dNewPoint < posBOD || dNewPoint > posEOD || dOldPoint < posBOD
	   || dNewPoint > posEOD)
	{
		return;
	}
	if (iNewPoint == iOldPoint)
	{
		return;
	}

	if (iNewPoint < iOldPoint)
	{
		if (iNewPoint < m_iSelectionAnchor)
		{
			if (iOldPoint < m_iSelectionAnchor)
			{
				/*
				  N O A
				  The selection got bigger.  Both points are
				  left of the anchor.
				*/
				_drawBetweenPositions(iNewPoint, iOldPoint);
			}
			else
			{
				/*
				  N A O
				  The selection flipped across the anchor to the left.
				*/
				bres = _clearBetweenPositions(m_iSelectionAnchor, iOldPoint, true);
				if(bres)
					_drawBetweenPositions(iNewPoint, iOldPoint);
			}
		}
		else
		{
			UT_ASSERT(iOldPoint >= m_iSelectionAnchor);

			/*
			  A N O
			  The selection got smaller.  Both points are to the
			  right of the anchor
			*/

			bres = _clearBetweenPositions(iNewPoint, iOldPoint, true);
			if(bres)
				_drawBetweenPositions(iNewPoint, iOldPoint);
		}
	}
	else
	{
		UT_ASSERT(iNewPoint > iOldPoint);

		if (iNewPoint < m_iSelectionAnchor)
		{
			UT_ASSERT(iOldPoint <= m_iSelectionAnchor);

			/*
			  O N A
			  The selection got smaller.  Both points are
			  left of the anchor.
			*/

			bres =_clearBetweenPositions(iOldPoint, iNewPoint, true);
			if(bres)
				_drawBetweenPositions(iOldPoint, iNewPoint);
		}
		else
		{
			if (iOldPoint < m_iSelectionAnchor)
			{
				/*
				  O A N
				  The selection flipped across the anchor to the right.
				*/

				bres = _clearBetweenPositions(iOldPoint, m_iSelectionAnchor, true);
				if(bres)
					_drawBetweenPositions(iOldPoint, iNewPoint);
			}
			else
			{
				/*
				  A O N
				  The selection got bigger.  Both points are to the
				  right of the anchor
				*/
				_drawBetweenPositions(iOldPoint, iNewPoint);
			}
		}
	}
}

void FV_View::_extSelToPos(PT_DocPosition iNewPoint)
{
	PT_DocPosition iOldPoint = getPoint();
	if (iNewPoint == iOldPoint)
		return;

	PT_DocPosition posBOD,posEOD,dNewPoint,dOldPoint;
	dNewPoint = (PT_DocPosition) iNewPoint;
	dOldPoint = (PT_DocPosition) iOldPoint;
	getEditableBounds(false,posBOD);
	getEditableBounds(true,posEOD);
	if(dNewPoint < posBOD || dNewPoint > posEOD || dOldPoint < posBOD 
	   || dNewPoint > posEOD)
	{
		return;
	}

	if (isSelectionEmpty())
	{
		_fixInsertionPointCoords();
		_clearIfAtFmtMark(getPoint());
		_setSelectionAnchor();
	}

	_setPoint(iNewPoint);
	_extSel(iOldPoint);
	
	if (isSelectionEmpty())
	{
		_resetSelection();
		_drawInsertionPoint();
	}

	notifyListeners(AV_CHG_MOTION);
}


void FV_View::warpInsPtToXY(UT_sint32 xPos, UT_sint32 yPos, bool bClick = false)
{
	/*
	  Figure out which page we clicked on.
	  Pass the click down to that page.
	*/

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);

	if (!isSelectionEmpty())
		_clearSelection();
	else
		_eraseInsertionPoint();
	PT_DocPosition pos,posEnd;
	bool bBOL = false;
	bool bEOL = false;
	fl_HdrFtrShadow * pShadow=NULL;
	if(bClick)
			pPage->mapXYToPositionClick(xClick, yClick, pos, pShadow, bBOL, bEOL);
	else
			pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL);
	if(bClick)
	{
		getEditableBounds(true,posEnd,true);
		if(pos > posEnd)
		{
			if (pos != getPoint())
				_clearIfAtFmtMark(getPoint());
			setHdrFtrEdit(pShadow);
			bClick = true;
		}
		else
		{
			bClick = false;
			clearHdrFtrEdit();
		}
	}
	if ((pos != getPoint()) && !bClick)
		_clearIfAtFmtMark(getPoint());


	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_setPoint(pos, bEOL);
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
	notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR ); // Sevior Put this in
//	notifyListeners(AV_CHG_HDRFTR );

}


void FV_View::getPageScreenOffsets(fp_Page* pThePage, UT_sint32& xoff,
								   UT_sint32& yoff)
{
	UT_uint32 y = getPageViewTopMargin();
	
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		if (pPage == pThePage)
		{
			break;
		}
		y += pPage->getHeight() + getPageViewSep();
		fl_DocSectionLayout * pDSL = pPage->getOwningSection();
		if(getViewMode() != VIEW_PRINT)
		{
			y = y - pDSL->getTopMargin() - pDSL->getBottomMargin();
		}
		pPage = pPage->getNext();
	}

	yoff = y - m_yScrollOffset;
	xoff = getPageViewTopMargin() - m_xScrollOffset;
}

void FV_View::getPageYOffset(fp_Page* pThePage, UT_sint32& yoff)
{
	UT_uint32 y = getPageViewTopMargin();
	
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		if (pPage == pThePage)
		{
			break;
		}
		y += pPage->getHeight() + getPageViewSep();
		fl_DocSectionLayout * pDSL = pPage->getOwningSection();
		if(getViewMode() != VIEW_PRINT)
		{
			y = y - pDSL->getTopMargin() - pDSL->getBottomMargin();
		}

		pPage = pPage->getNext();
	}

	yoff = y;
}

UT_sint32 FV_View::getPageViewSep(void) const
{
	// return the amount of gray-space we draw above the top
	// of the paper in "Page View".  return zero if not in
	// "Page View".
	if (isPreview() || m_pG->queryProperties(GR_Graphics::DGP_PAPER))
		return 0;
	else if (getViewMode() != VIEW_PRINT)
	{
		return 1;
	}
	else
		return fl_PAGEVIEW_PAGE_SEP;
}


UT_sint32 FV_View::getPageViewLeftMargin(void) const
{
	// return the amount of gray-space we draw to the left
	// of the paper in "Page View".  return zero if not in
	// "Page View".
	if (isPreview() || m_pG->queryProperties(GR_Graphics::DGP_PAPER) || (getViewMode() != VIEW_PRINT))
		return 0;
	else
		return fl_PAGEVIEW_MARGIN_X;
}

UT_sint32 FV_View::getPageViewTopMargin(void) const
{
	// return the amount of gray-space we draw above the top
	// of the paper in "Page View".  return zero if not in
	// "Page View".
	if (isPreview() || m_pG->queryProperties(GR_Graphics::DGP_PAPER) || (getViewMode() != VIEW_PRINT))
		return 0;
	else
		return fl_PAGEVIEW_MARGIN_Y;
}

/*
  This method simply iterates over every run between two doc positions
  and draws each one.
*/
void FV_View::_drawBetweenPositions(PT_DocPosition iPos1, PT_DocPosition iPos2)
{
	UT_ASSERT(iPos1 < iPos2);

	fp_Run* pRun1;
	fp_Run* pRun2;
	UT_sint32 xoff;
	UT_sint32 yoff;
	UT_uint32 uheight;

	_fixInsertionPointCoords();
	{
		UT_sint32 x;
		UT_sint32 y;
		UT_sint32 x2;
		UT_sint32 y2;
		bool bDirection;
		fl_BlockLayout* pBlock1;
		fl_BlockLayout* pBlock2;

		/*
		  we don't really care about the coords.  We're calling these
		  to get the Run pointer
		*/
		_findPositionCoords(iPos1, false, x, y, x2, y2, uheight, bDirection, &pBlock1, &pRun1);
		_findPositionCoords(iPos2, false, x, y, x2, y2, uheight, bDirection, &pBlock2, &pRun2);
	}

	bool bDone = false;
	bool bIsDirty = false;
	fp_Run* pCurRun = pRun1;

	while ((!bDone || bIsDirty) && pCurRun)
	{
		if (pCurRun == pRun2)
		{
			bDone = true;
		}

		fl_BlockLayout* pBlock = pCurRun->getBlock();
		UT_ASSERT(pBlock);

		fp_Line* pLine = pCurRun->getLine();
		if(pLine == NULL || (pLine->getContainer()->getPage()== NULL))
		{
			return;
		}
		pLine->getScreenOffsets(pCurRun, xoff, yoff);

		dg_DrawArgs da;

		da.pG = m_pG;
		da.xoff = xoff;
		da.yoff = yoff + pLine->getAscent();

		pCurRun->draw(&da);

		pCurRun = pCurRun->getNext();
		if (!pCurRun)
		{
			fl_BlockLayout* pNextBlock;

			pNextBlock = pBlock->getNextBlockInDocument();
			if (pNextBlock)
			{
				pCurRun = pNextBlock->getFirstRun();
			}
		}
		if (!pCurRun)
		{
			bIsDirty = false;
		}
		else
		{
			bIsDirty = pCurRun->isDirty();
		}
	}
}

/*
  This method simply iterates over every run between two doc positions
  and draws each one.
*/
bool FV_View::_clearBetweenPositions(PT_DocPosition iPos1, PT_DocPosition iPos2, bool bFullLineHeightRect)
{
	if (iPos1 >= iPos2)
	{
		return true;
	}
	
	fp_Run* pRun1;
	fp_Run* pRun2;
	UT_uint32 uheight;

	_fixInsertionPointCoords();
	{
		UT_sint32 x;
		UT_sint32 y;
		UT_sint32 x2;
		UT_sint32 y2;
		bool bDirection;
		fl_BlockLayout* pBlock1;
		fl_BlockLayout* pBlock2;

		/*
		  we don't really care about the coords.  We're calling these
		  to get the Run pointer
		*/
		_findPositionCoords(iPos1, false, x, y, x2, y2, uheight, bDirection, &pBlock1, &pRun1);
		_findPositionCoords(iPos2, false, x, y, x2, y2, uheight, bDirection, &pBlock2, &pRun2);
	}

	if (!pRun1 && !pRun2)
	{
		// no formatting info for either block, so just bail
		// this can happen during spell, when we're trying to invalidate
		// a new squiggle before the block has been formatted
		return false;
	}

	// HACK: In certain editing cases only one of these is NULL, which
	//		 makes locating runs to clear more difficult.  For now, I'm
	//		 playing it safe and trying to just handle these cases here.
	//		 The real solution may be to just bail if *either* is NULL,
	//		 but I'm not sure.
	//
	//		 If you're interested in investigating this alternative
	//		 approach, play with the following asserts.

//	UT_ASSERT(pRun1 && pRun2);
	UT_ASSERT(pRun2);

	bool bDone = false;
	fp_Run* pCurRun = (pRun1 ? pRun1 : pRun2);


	while (!bDone)
	{
		if (pCurRun == pRun2)
		{
			bDone = true;
		}
		
		pCurRun->clearScreen(bFullLineHeightRect);
		
		if (pCurRun->getNext())
		{
			pCurRun = pCurRun->getNext();
		}
		else
		{
			fl_BlockLayout* pNextBlock;
			
			fl_BlockLayout* pBlock = pCurRun->getBlock();
			UT_ASSERT(pBlock);

			pNextBlock = pBlock->getNextBlockInDocument();
			if (pNextBlock)
			{
				pCurRun = pNextBlock->getFirstRun();
			}
			else
				bDone = true;
			// otherwise we get fun
			// infinte loops
		}
	}
	return true;
}

void FV_View::_findPositionCoords(PT_DocPosition pos,
								  bool bEOL,
								  UT_sint32& x,
								  UT_sint32& y,
								  UT_sint32& x2,
								  UT_sint32& y2,
								  UT_uint32& height,
								  bool& bDirection,
								  fl_BlockLayout** ppBlock,
								  fp_Run** ppRun)
{
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 xPoint2;
	UT_sint32 yPoint2;
	UT_sint32 iPointHeight;

	// Get the previous block in the document. _findBlockAtPosition
	// will iterate forwards until it actually find a block if there
	// isn't one previous to pos.
	// (Removed code duplication. Jesper, 2001.01.25)
	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);
	
	// probably an empty document, return instead of
	// dereferencing NULL.	Dom 11.9.00
	if(!pBlock)
	{
		// Do the assert. Want to know from debug builds when this happens.
		UT_ASSERT(pBlock);

		x = x2 = 0;
		y = y2 = 0;
		
		height = 0;
		if(ppBlock)
			*ppBlock = 0;
		return;
	}

	// If block is actually to the right of the requested position
	// (this happens in an empty document), update the pos with the
	// start pos of the block.
	PT_DocPosition iBlockPos = pBlock->getPosition(false);
	if (iBlockPos > pos)
	{
		pos = iBlockPos;
	}

	fp_Run* pRun = pBlock->findPointCoords(pos, bEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);

	// NOTE prior call will fail if the block isn't currently formatted,
	// NOTE so we won't be able to figure out more specific geometry

	if (pRun)
	{
		// we now have coords relative to the page containing the ins pt
	        fp_Line * pLine =  pRun->getLine();
	        if(!pLine)
		{
		    x = x2 = 0;
		    y = y2 = 0;
		
		    height = 0;
		    if(ppBlock)
		      *ppBlock = 0;
		    return;
		}

		fp_Page* pPointPage = pLine->getContainer()->getPage();

		UT_sint32 iPageOffset;
		getPageYOffset(pPointPage, iPageOffset);

		yPoint += iPageOffset;
		xPoint += getPageViewLeftMargin();
#ifdef BIDI_ENABLED
		yPoint2 += iPageOffset;
		xPoint2 += getPageViewLeftMargin();
#endif
		// now, we have coords absolute, as if all pages were stacked vertically
		xPoint -= m_xScrollOffset;
		yPoint -= m_yScrollOffset;
#ifdef BIDI_ENABLED
		xPoint2 -= m_xScrollOffset;
		yPoint2 -= m_yScrollOffset;
#endif

		// now, return the results
		x = xPoint;
		y = yPoint;
#ifdef BIDI_ENABLED
		x2 = xPoint2;
		y2 = yPoint2;
#endif
		height = iPointHeight;
	}

	if (ppBlock)
	{
		*ppBlock = pBlock;
	}
	
	if (ppRun)
	{
		*ppRun = pRun;
	}
}

void FV_View::_fixInsertionPointCoords()
{
	_eraseInsertionPoint();
	_findPositionCoords(getPoint(), m_bPointEOL, m_xPoint, m_yPoint, m_xPoint2, m_yPoint2, m_iPointHeight, m_bPointDirection, NULL, NULL);
	xxx_UT_DEBUGMSG(("SEVIOR: m_yPoint = %d m_iPointHeight = %d \n",m_yPoint,m_iPointHeight));
	_saveCurrentPoint();
	// hang onto this for _moveInsPtNextPrevLine()
	m_xPointSticky = m_xPoint + m_xScrollOffset - getPageViewLeftMargin();
}

void FV_View::_updateInsertionPoint()
{
	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
}

bool FV_View::_hasPointMoved(void)
{
	if( m_xPoint == m_oldxPoint && m_yPoint == m_oldyPoint && m_iPointHeight ==  m_oldiPointHeight)
	{
		return false;
	}

	return true;
}

void  FV_View::_saveCurrentPoint(void)
{
	m_oldxPoint = m_xPoint;
	m_oldyPoint = m_yPoint;
	m_oldiPointHeight = m_iPointHeight;
#ifdef BIDI_ENABLED
	m_oldxPoint2 = m_xPoint2;
	m_oldyPoint2 = m_yPoint2;
#endif
}

void  FV_View::_clearOldPoint(void)
{
	m_oldxPoint = -1;
	m_oldyPoint = -1;
	m_oldiPointHeight = 0;
#if BIDI_ENABLED
	m_oldxPoint2 = -1;
	m_oldyPoint2 = -1;
#endif
}

void FV_View::_xorInsertionPoint()
{
	if (NULL == getCurrentPage())
		return;

	UT_ASSERT(getCurrentPage()->getOwningSection());

	if (m_iPointHeight > 0 )
	{
		fp_Page * pPage = getCurrentPage();

		if (pPage)
		{
			UT_RGBColor * pClr = pPage->getOwningSection()->getPaperColor();
			m_pG->setColor(*pClr);
		}

		m_pG->xorLine(m_xPoint-1, m_yPoint+1, m_xPoint-1, m_yPoint + m_iPointHeight+1);
		m_pG->xorLine(m_xPoint, m_yPoint+1, m_xPoint, m_yPoint + m_iPointHeight+1);
	  m_bCursorIsOn = !m_bCursorIsOn;
#ifdef BIDI_ENABLED
	  if((m_xPoint != m_xPoint2) || (m_yPoint != m_yPoint2))
	  {
		  // #TF the caret will have a small flag at the top indicating the direction of
		  // writing
		  if(m_bPointDirection) //rtl flag
		  {
			  m_pG->xorLine(m_xPoint-3, m_yPoint+1, m_xPoint-1, m_yPoint+1);
			  m_pG->xorLine(m_xPoint-2, m_yPoint+2, m_xPoint-1, m_yPoint+2);
		  }
		  else
		  {
			  m_pG->xorLine(m_xPoint+1, m_yPoint+1, m_xPoint+3, m_yPoint+1);
			  m_pG->xorLine(m_xPoint+1, m_yPoint+2, m_xPoint+2, m_yPoint+2);
		  }
		  
		  
		  //this is the second caret on ltr-rtl boundary
		  m_pG->xorLine(m_xPoint2-1, m_yPoint2+1, m_xPoint2-1, m_yPoint2 + m_iPointHeight + 1);
		  m_pG->xorLine(m_xPoint2, m_yPoint2+1, m_xPoint2, m_yPoint2 + m_iPointHeight + 1);
		  //this is the line that links the two carrets
		  m_pG->xorLine(m_xPoint, m_yPoint + m_iPointHeight + 1, m_xPoint2, m_yPoint2 + m_iPointHeight + 1);
		
		  if(m_bPointDirection)
		  {
			  m_pG->xorLine(m_xPoint2+1, m_yPoint2+1, m_xPoint2+3, m_yPoint2+1);
			  m_pG->xorLine(m_xPoint2+1, m_yPoint2+2, m_xPoint2+2, m_yPoint2+2);
		  }
		  else
		  {
			  m_pG->xorLine(m_xPoint2-3, m_yPoint2+1, m_xPoint2-1, m_yPoint2+1);
			  m_pG->xorLine(m_xPoint2-2, m_yPoint2+2, m_xPoint2-1, m_yPoint2+2);
		  }
	  }
#endif
	}
	if(_hasPointMoved() == true)
	{
		m_bCursorIsOn = true;
	}
	_saveCurrentPoint();
}

bool FV_View::isCursorOn(void)
{
	return m_bCursorIsOn;
}

void FV_View::eraseInsertionPoint(void)
{
	_eraseInsertionPoint();
}

void FV_View::_eraseInsertionPoint()
{
	m_bEraseSaysStopBlinking = true;
	if (_hasPointMoved() == true)
	{
		UT_DEBUGMSG(("Insertion Point has moved before erasing \n"));
		if (m_pAutoCursorTimer)
			m_pAutoCursorTimer->stop();
		m_bCursorIsOn = false;
		_saveCurrentPoint();
		return;
	}

	//	if (m_pAutoCursorTimer)
	//		m_pAutoCursorTimer->stop();

	
	if (m_bCursorIsOn && isSelectionEmpty())
	{
		_xorInsertionPoint();
	}
	m_bCursorIsOn = false;
}

void FV_View::drawInsertionPoint()
{
	_drawInsertionPoint();
}

void FV_View::_drawInsertionPoint()
{
	if(m_focus==AV_FOCUS_NONE || !shouldScreenUpdateOnGeneralUpdate())
	{
		return;
	}
	if (m_bCursorBlink && (m_focus==AV_FOCUS_HERE || m_focus==AV_FOCUS_MODELESS || AV_FOCUS_NEARBY))
	{
		if (m_pAutoCursorTimer == NULL)
		{
			m_pAutoCursorTimer = UT_Timer::static_constructor(_autoDrawPoint, this, m_pG);
			m_pAutoCursorTimer->set(AUTO_DRAW_POINT);
			m_bCursorIsOn = false;
		}
		m_pAutoCursorTimer->stop();
		m_pAutoCursorTimer->start();
	}
	m_bEraseSaysStopBlinking = false;
	if (m_iWindowHeight <= 0)
	{
		return;
	}
	
	if (!isSelectionEmpty())
	{
		return;
	}
	UT_ASSERT(m_bCursorIsOn == false);
	if (m_bCursorIsOn == false)
	{
		_xorInsertionPoint();
	}
}

void FV_View::_autoDrawPoint(UT_Worker * pWorker)
{
	UT_ASSERT(pWorker);

	FV_View * pView = (FV_View *) pWorker->getInstanceData();
	UT_ASSERT(pView);

	if (pView->m_iWindowHeight <= 0)
	{
		return;
	}
	
	if (!pView->isSelectionEmpty())
	{
		return;
	}
	if (pView->m_bEraseSaysStopBlinking == false)
	{
		pView->_xorInsertionPoint();
	}
}

void FV_View::setXScrollOffset(UT_sint32 v)
{
	UT_sint32 dx = v - m_xScrollOffset;

	if (dx == 0)
		return;
	_fixInsertionPointCoords();
	m_pG->scroll(dx, 0);
	m_xScrollOffset = v;
	
	UT_sint32 x1 = 0;
	UT_sint32 dx2 = m_iWindowWidth;

	if (dx > 0)
	{
		if (dx < m_iWindowWidth)
		{
			x1 = m_iWindowWidth - dx;
			dx2 = dx;
		}
	}
	else
	{
		if (dx > -m_iWindowWidth)
		{
			dx2 = -dx;
		}
	}

	_draw(x1, 0, dx2, m_iWindowHeight, false, true);

	_fixInsertionPointCoords();
	_drawInsertionPoint();

}

void FV_View::setYScrollOffset(UT_sint32 v)
{
	UT_sint32 dy = v - m_yScrollOffset;

	if (dy == 0)
		return;

	_fixInsertionPointCoords();
	m_pG->scroll(0, dy);
	m_yScrollOffset = v;

	UT_sint32 y1 = 0;
	UT_sint32 dy2 = m_iWindowHeight;

	if (dy > 0)
	{
		if (dy < m_iWindowHeight)
		{
			y1 = m_iWindowHeight - dy;
			dy2 = dy;
		}
	}
	else
	{
		if (dy > -m_iWindowHeight)
		{
			dy2 = -dy;
		}
	}

	_draw(0, y1, m_iWindowWidth, dy2, false, true);

	_fixInsertionPointCoords();
	_drawInsertionPoint();
}

void FV_View::draw(int page, dg_DrawArgs* da)
{
	UT_DEBUGMSG(("FV_View::draw_1: [page %ld]\n",page));

	da->pG = m_pG;
	fp_Page* pPage = m_pLayout->getNthPage(page);
	if (pPage)
	{
		pPage->draw(da);
	}
}

void FV_View::draw(const UT_Rect* pClipRect)
{
	_fixInsertionPointCoords();
	if (pClipRect)
	{
		_draw(pClipRect->left,pClipRect->top,pClipRect->width,pClipRect->height,false,true);
	}
	else
	{
		_draw(0,0,m_iWindowWidth,m_iWindowHeight,false,false);
	}
	_fixInsertionPointCoords();
	_drawInsertionPoint();
}

void FV_View::updateScreen(bool bDirtyRunsOnly)
{
	bool bCursor = isCursorOn();
	_eraseInsertionPoint();
	_draw(0,0,m_iWindowWidth,m_iWindowHeight,bDirtyRunsOnly,false);
	if(bCursor)
	{
		_drawInsertionPoint();
	}
}


void FV_View::_draw(UT_sint32 x, UT_sint32 y,
					UT_sint32 width, UT_sint32 height,
					bool bDirtyRunsOnly, bool bClip)
{
	xxx_UT_DEBUGMSG(("FV_View::draw_3 [x %ld][y %ld][w %ld][h %ld][bClip %ld]\n"
					 "\t\twith [yScrollOffset %ld][windowHeight %ld]\n",
					 x,y,width,height,bClip,
					 m_yScrollOffset,m_iWindowHeight));

	// this can happen when the frame size is decreased and
	// only the toolbars show...
	if ((m_iWindowWidth <= 0) || (m_iWindowHeight <= 0))
	{
		UT_DEBUGMSG(("fv_View::draw() called with zero drawing area.\n"));
		return;
	}
//	UT_ASSERT(bClip);
	if ((width <= 0) || (height <= 0))
	{
		UT_DEBUGMSG(("fv_View::draw() called with zero width or height expose.\n"));
		return;
	}

	// TMN: Leave this rect at function scope!
	// gr_Graphics only stores a _pointer_ to it!
	UT_Rect rClip;
	if (bClip)
	{
		rClip.left = x;
		rClip.top = y;
		rClip.width = width;
		rClip.height = height;
		m_pG->setClipRect(&rClip);
	}

	// figure out where pages go, based on current window dimensions
	// TODO: don't calc for every draw
	// HYP:  cache calc results at scroll/size time
	UT_sint32 iDocHeight = m_pLayout->getHeight();

	// TODO: handle positioning within oversized viewport
	// TODO: handle variable-size pages (envelope, landscape, etc.)

	/*
	  In page view mode, so draw outside decorations first, then each
	  page with its decorations.
	*/

	UT_RGBColor clrMargin(127,127,127); 	// dark gray

	if (!bDirtyRunsOnly)
	{
		if ((m_xScrollOffset < getPageViewLeftMargin()) && (getViewMode() == VIEW_PRINT))
		{
			// fill left margin
			m_pG->fillRect(clrMargin, 0, 0, getPageViewLeftMargin() - m_xScrollOffset, m_iWindowHeight);
		}

		if (m_yScrollOffset < getPageViewTopMargin() && (getViewMode() == VIEW_PRINT))
		{
			// fill top margin
			m_pG->fillRect(clrMargin, 0, 0, m_iWindowWidth, getPageViewTopMargin() - m_yScrollOffset);
		}
	}

	UT_sint32 curY = getPageViewTopMargin();
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		UT_sint32 iPageWidth		= pPage->getWidth();
		UT_sint32 iPageHeight		= pPage->getHeight();
		UT_sint32 adjustedTop		= curY - m_yScrollOffset;
		fl_DocSectionLayout * pDSL = pPage->getOwningSection();
		if(getViewMode() != VIEW_PRINT)
		{
			iPageHeight = iPageHeight - pDSL->getTopMargin() - pDSL->getBottomMargin();
		}

		UT_sint32 adjustedBottom = adjustedTop + iPageHeight + getPageViewSep();

		if (adjustedTop > m_iWindowHeight)
		{
			// the start of this page is past the bottom
			// of the window, so we don't need to draw it.

			xxx_UT_DEBUGMSG(("not drawing page A: iPageHeight=%d curY=%d nPos=%d m_iWindowHeight=%d\n",
							 iPageHeight,
							 curY,
							 m_yScrollOffset,
							 m_iWindowHeight));

			// since all other pages are below this one, we
			// don't need to draw them either.	exit loop now.
			break;
		}
		else if (adjustedBottom < 0)
		{
			// the end of this page is above the top of
			// the window, so we don't need to draw it.

			xxx_UT_DEBUGMSG(("not drawing page B: iPageHeight=%d curY=%d nPos=%d m_iWindowHeight=%d\n",
							 iPageHeight,
							 curY,
							 m_yScrollOffset,
							 m_iWindowHeight));
		}
		else if (adjustedTop > y + height)
		{
			// the top of this page is beyond the end
			// of the clipping region, so we don't need
			// to draw it.

			xxx_UT_DEBUGMSG(("not drawing page C: iPageHeight=%d curY=%d nPos=%d m_iWindowHeight=%d y=%d h=%d\n",
							 iPageHeight,
							 curY,
							 m_yScrollOffset,
							 m_iWindowHeight,
							 y,height));
		}
		else if (adjustedBottom < y)
		{
			// the bottom of this page is above the top
			// of the clipping region, so we don't need
			// to draw it.

			xxx_UT_DEBUGMSG(("not drawing page D: iPageHeight=%d curY=%d nPos=%d m_iWindowHeight=%d y=%d h=%d\n",
							 iPageHeight,
							 curY,
							 m_yScrollOffset,
							 m_iWindowHeight,
							 y,height));
			//TF NOTE: Can we break out here?
		}
		else
		{
			// this page is on screen and intersects the clipping region,
			// so we *DO* draw it.

			xxx_UT_DEBUGMSG(("drawing page E: iPageHeight=%d curY=%d nPos=%d m_iWindowHeight=%d y=%d h=%d\n",
							 iPageHeight,curY,m_yScrollOffset,m_iWindowHeight,y,height));

			dg_DrawArgs da;

			da.bDirtyRunsOnly = bDirtyRunsOnly;
			da.pG = m_pG;
			da.xoff = getPageViewLeftMargin() - m_xScrollOffset;
			da.yoff = adjustedTop;

			UT_sint32 adjustedLeft	= getPageViewLeftMargin() - m_xScrollOffset;
			UT_sint32 adjustedRight = adjustedLeft + iPageWidth;

			adjustedBottom -= getPageViewSep();

			if (!bDirtyRunsOnly || pPage->needsRedraw() && (getViewMode() == VIEW_PRINT))
			{	
			  UT_RGBColor * pClr = pPage->getOwningSection()->getPaperColor();
			  m_pG->fillRect(*pClr,adjustedLeft+1,adjustedTop+1,iPageWidth-1,iPageHeight-1);
			}
			pPage->draw(&da);

			// draw page decorations
			UT_RGBColor clr(0,0,0); 	// black
			m_pG->setColor(clr);

			// one pixel border a
			if(!isPreview() && (getViewMode() == VIEW_PRINT))
			{
				m_pG->drawLine(adjustedLeft, adjustedTop, adjustedRight, adjustedTop);
				m_pG->drawLine(adjustedRight, adjustedTop, adjustedRight, adjustedBottom);
				m_pG->drawLine(adjustedRight, adjustedBottom, adjustedLeft, adjustedBottom);
				m_pG->drawLine(adjustedLeft, adjustedBottom, adjustedLeft, adjustedTop);
			}

//
// Draw page seperator
//
			UT_RGBColor paperColor = *(pPage->getOwningSection()->getPaperColor());

			// only in NORMAL MODE - draw a line across the screen
			// at a page boundary. Not used in online/web and print
			// layout modes
			if(getViewMode() == VIEW_NORMAL)
			{
				UT_RGBColor clrPageSep(192,192,192);		// light gray
				m_pG->setColor(clrPageSep);
				m_pG->drawLine(adjustedLeft, adjustedBottom, adjustedRight, adjustedBottom);
				adjustedBottom += 1;
				m_pG->setColor(clr);
			}
			// fill to right of page
			if (m_iWindowWidth - (adjustedRight + 1) > 0)
			{
				// In normal mode, the right margin is
				// white (since the whole screen is white).
				if(getViewMode() != VIEW_PRINT)
				{
					m_pG->fillRect(paperColor, adjustedRight, adjustedTop, m_iWindowWidth - (adjustedRight), iPageHeight + 1);
				}
				// Otherwise, the right margin is the
				// margin color (gray).
				else
				{
					m_pG->fillRect(clrMargin, adjustedRight + 1, adjustedTop, m_iWindowWidth - (adjustedRight + 1), iPageHeight + 1);
				}
			}

			// fill separator below page
			if ((m_iWindowHeight - (adjustedBottom + 1) > 0) && (VIEW_PRINT == getViewMode()) )
			{
				if(pPage->getNext() != NULL)
				{
					m_pG->fillRect(clrMargin, adjustedLeft, adjustedBottom + 1, m_iWindowWidth - adjustedLeft, getPageViewSep());
				}
				else
				{
					UT_sint32 botfill = getWindowHeight() - adjustedBottom - 1 ;
					m_pG->fillRect(clrMargin, adjustedLeft, adjustedBottom + 1, m_iWindowWidth - adjustedLeft, botfill);
				}
			}

			// two pixel drop shadow
			
			if(!isPreview() && (getViewMode() == VIEW_PRINT))
			{
				adjustedLeft += 3;
				adjustedBottom += 1;
				m_pG->drawLine(adjustedLeft, adjustedBottom, adjustedRight+1, adjustedBottom);

				adjustedBottom += 1;
				m_pG->drawLine(adjustedLeft, adjustedBottom, adjustedRight+1, adjustedBottom);

				adjustedTop += 3;
				adjustedRight += 1;
				m_pG->drawLine(adjustedRight, adjustedTop, adjustedRight, adjustedBottom);

				adjustedRight += 1;
				m_pG->drawLine(adjustedRight, adjustedTop, adjustedRight, adjustedBottom);
			}
		}

		curY += iPageHeight + getPageViewSep();

		pPage = pPage->getNext();
	}

	if (curY < iDocHeight)
	{
		// fill below bottom of document
		UT_sint32 y = curY - m_yScrollOffset + 1;
		UT_sint32 h = m_iWindowHeight - y;

		m_pG->fillRect(clrMargin, 0, y, m_iWindowWidth, h);
	}

	if (bClip)
	{
		m_pG->setClipRect(NULL);
	}

}

void FV_View::cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos)
{
#define HACK_LINE_HEIGHT				20 // TODO Fix this!!
	
	UT_sint32 lineHeight = iPos;
	UT_sint32 docHeight = 0;
	bool bVertical = false;
	bool bHorizontal = false;
	
	docHeight = m_pLayout->getHeight();
	
	if (lineHeight == 0)
	{
		lineHeight = HACK_LINE_HEIGHT;
	}
	
	UT_sint32 yoff = m_yScrollOffset;
	UT_sint32 xoff = m_xScrollOffset;
	
	switch(cmd)
	{
	case AV_SCROLLCMD_PAGEDOWN:
		yoff += m_iWindowHeight - HACK_LINE_HEIGHT;
		bVertical = true;
		break;
	case AV_SCROLLCMD_PAGEUP:
		yoff -= m_iWindowHeight - HACK_LINE_HEIGHT;
		bVertical = true;
		break;
	case AV_SCROLLCMD_PAGELEFT:
		xoff -= m_iWindowWidth;
		bHorizontal = true;
		break;
	case AV_SCROLLCMD_PAGERIGHT:
		xoff += m_iWindowWidth;
		bHorizontal = true;
		break;
	case AV_SCROLLCMD_LINEDOWN:
		yoff += lineHeight;
		bVertical = true;
		break;
	case AV_SCROLLCMD_LINEUP:
		yoff -= lineHeight;
		bVertical = true;
		break;
	case AV_SCROLLCMD_LINELEFT:
		xoff -= lineHeight;
		bHorizontal = true;
		break;
	case AV_SCROLLCMD_LINERIGHT:
		xoff += lineHeight;
		bHorizontal = true;
		break;
	case AV_SCROLLCMD_TOTOP:
		yoff = 0;
		bVertical = true;
		break;
	case AV_SCROLLCMD_TOPOSITION:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		break;
	case AV_SCROLLCMD_TOBOTTOM:
		fp_Page* pPage = m_pLayout->getFirstPage();
		UT_sint32 iDocHeight = getPageViewTopMargin();
		while (pPage)
		{
			iDocHeight += pPage->getHeight() + getPageViewSep();
			pPage = pPage->getNext();
		}
		yoff = iDocHeight;
		bVertical = true;
		break;
	}

	if (yoff < 0)
	{
		yoff = 0;
	}

	bool bRedrawPoint = true;
		
	if (bVertical && (yoff != m_yScrollOffset))
	{
		sendVerticalScrollEvent(yoff);
		if ((cmd != AV_SCROLLCMD_PAGEUP 
			 && cmd != AV_SCROLLCMD_PAGEDOWN))
		  bRedrawPoint = false;
	}

	if (xoff < 0)
	{
		xoff = 0;
	}
		
	if (bHorizontal && (xoff != m_xScrollOffset))
	{
		sendHorizontalScrollEvent(xoff);
		bRedrawPoint = false;
	}

	if (bRedrawPoint)
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	
}

bool FV_View::isLeftMargin(UT_sint32 xPos, UT_sint32 yPos)
{
	/*
	  Figure out which page we clicked on.
	  Pass the click down to that page.
	*/
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);

	PT_DocPosition iNewPoint;
	bool bBOL = false;
	bool bEOL = false;
	fl_HdrFtrShadow * pShadow=NULL;
	pPage->mapXYToPositionClick(xClick, yClick, iNewPoint,pShadow, bBOL, bEOL);

	return bBOL;
}

void FV_View::cmdSelect(PT_DocPosition dpBeg, PT_DocPosition dpEnd)
{

	_eraseInsertionPoint();

	if (!isSelectionEmpty())
	{
		_clearSelection();
	}

	m_iSelectionAnchor = dpBeg;
	m_iSelectionLeftAnchor = dpBeg;
	m_iSelectionRightAnchor = dpEnd;

	_setPoint (dpEnd);

	if (dpBeg == dpEnd)
	{
		_drawInsertionPoint();
		return;
	}

	m_bSelection = true;
	
	_drawSelection();

	notifyListeners(AV_CHG_EMPTYSEL);
}

#define IS_SELECTALL(a, b) ((a) == FV_DOCPOS_BOD && (b) == FV_DOCPOS_EOD)

void FV_View::cmdSelect(UT_sint32 xPos, UT_sint32 yPos, FV_DocPos dpBeg, FV_DocPos dpEnd)
{
	UT_DEBUGMSG(("Double click on mouse \n"));

	warpInsPtToXY(xPos, yPos,true);

	//_eraseInsertionPoint();

	PT_DocPosition iPosLeft = _getDocPos(dpBeg, false);
	PT_DocPosition iPosRight = _getDocPos(dpEnd, false);

	cmdSelect (iPosLeft, iPosRight);
}

void FV_View::cmdHyperlinkJump(UT_sint32 xPos, UT_sint32 yPos)
{
	_clearSelection();
	warpInsPtToXY(xPos, yPos,true);

	//_eraseInsertionPoint();
	fl_BlockLayout * pBlock = getCurrentBlock();
	PT_DocPosition iRelPos = getPoint() - pBlock->getPosition(false);

	fp_Run *pRun = pBlock->getFirstRun();
	while (pRun && pRun->getBlockOffset()+ pRun->getLength() < iRelPos)
		pRun= pRun->getNext();
		
	UT_ASSERT(pRun);
	pRun->getPrev();

	UT_ASSERT(pRun);
#if 0
	if(pRun->getType()== FPRUN_FMTMARK || pRun->getType()== FPRUN_HYPERLINK || pRun->getType()== FPRUN_BOOKMARK)
		pRun  = pRun->getNext();

	UT_ASSERT(pRun);
#endif
	fp_HyperlinkRun * pH = pRun->getHyperlink();

	UT_ASSERT(pH);

	const XML_Char * pTarget = pH->getTarget();

	if(*pTarget == '#')
		pTarget++;

	UT_uint32 iTargetLen = UT_XML_strlen(pTarget);
	UT_UCSChar * pTargetU = new UT_UCSChar[iTargetLen+1];

	UT_ASSERT(pTargetU);

	UT_UCSChar * pJump = pTargetU;

	for (UT_uint32 i = 0; i < iTargetLen; i++)
		*pTargetU++ = (UT_UCSChar) *pTarget++;
	*pTargetU = 0;

	gotoTarget(AP_JUMPTARGET_BOOKMARK, pJump);
	
	delete [] pJump;
}

void FV_View::_setPoint(PT_DocPosition pt, bool bEOL)
{
	if (!m_pDoc->getAllowChangeInsPoint())
	{
		return;
	}
	m_iInsPoint = pt;
	m_bPointEOL = bEOL;
	if(!m_pDoc->isPieceTableChanging())
	{	
		m_pLayout->considerPendingSmartQuoteCandidate();
	}
	_checkPendingWordForSpell();
}

void FV_View::setPoint(PT_DocPosition pt)
{
	_setPoint(pt, m_bPointEOL);
}

void FV_View::setDontChangeInsPoint(void)
{
	m_pDoc->setDontChangeInsPoint();
}

void FV_View::allowChangeInsPoint(void)
{
	m_pDoc->allowChangeInsPoint();
}


bool FV_View::isDontChangeInsPoint(void)
{
	return !m_pDoc->getAllowChangeInsPoint();
}

/*!
 Spell-check pending word
 If the IP does not touch the pending word, spell-check it.

 \note This function used to exit if PT was changing - but that
	   prevents proper squiggle behavior during undo, so the check has
	   been removed. This means that the pending word POB must be
	   updated to reflect the PT changes before the IP is moved.
 */
void
FV_View::_checkPendingWordForSpell(void)
{
	if (!m_pLayout->isPendingWordForSpell()) return;

	// Find block at IP
	fl_BlockLayout* pBL = _findBlockAtPosition(m_iInsPoint);
	if (pBL)
	{
		UT_uint32 iOffset = m_iInsPoint - pBL->getPosition();

		// If it doesn't touch the pending word, spell-check it
		if (!m_pLayout->touchesPendingWordForSpell(pBL, iOffset, 0))
		{
			// no longer there, so check it
			if (m_pLayout->checkPendingWordForSpell())
			{
				// FIXME:jskov Without this updateScreen call, the
				// just squiggled word remains deleted. It's overkill
				// (surely we should have a requestUpdateScreen() that
				// does so after all operations have completed), but
				// works. Unfortunately it causes a small screen
				// artifact when pressing undo, since some runs may be
				// redrawn before they have their correct location
				// recalculated. In other words, make the world a
				// better place by adding requestUpdateScreen or
				// similar.
				_eraseInsertionPoint();
				updateScreen();
				_drawInsertionPoint();
			}
		}
	}
}

UT_uint32 FV_View::_getDataCount(UT_uint32 pt1, UT_uint32 pt2)
{
	UT_ASSERT(pt2>=pt1);
	return pt2 - pt1;
}


bool FV_View::_charMotion(bool bForward,UT_uint32 countChars)
{
	// advance(backup) the current insertion point by count characters.
	// return false if we ran into an end (or had an error).

	PT_DocPosition posOld = m_iInsPoint;
	fp_Run* pRun = NULL;
	fl_BlockLayout* pBlock = NULL;
	UT_sint32 x;
	UT_sint32 y;
	UT_sint32 x2;
	UT_sint32 y2;
	bool bDirection;
	UT_uint32 uheight;
	m_bPointEOL = false;

	/*
	  we don't really care about the coords.  We're calling these
	  to get the Run pointer
	*/
	PT_DocPosition posBOD;
	PT_DocPosition posEOD;
	bool bRes;

	bRes = getEditableBounds(false, posBOD);
	bRes = getEditableBounds(true, posEOD);
	UT_ASSERT(bRes);

	// FIXME:jskov want to rewrite this code to use simplified
	// versions of findPositionCoords. I think there's been some bugs
	// due to that function being overloaded to be used from this
	// code.

	

	if (bForward)
	{
		m_iInsPoint += countChars;
		_findPositionCoords(m_iInsPoint-1, false, x, y, x2,y2,uheight, bDirection, &pBlock, &pRun);
		//		while(pRun != NULL && (pRun->isField() == true || pRun->getType() == FPRUN_FIELD && m_iInsPoint < posEOD))
		while(pRun != NULL && pRun->isField() == true && m_iInsPoint < posEOD)
		{
			m_iInsPoint++;
			_findPositionCoords(m_iInsPoint, false, x, y, x2,y2,uheight, bDirection, &pBlock, &pRun);
		}
	}
	else
	{
		m_iInsPoint -= countChars;
		_findPositionCoords(m_iInsPoint, false, x, y, x2,y2,uheight, bDirection, &pBlock, &pRun);
		while(pRun != NULL && pRun->isField() == true && m_iInsPoint > posBOD)
		{
			m_iInsPoint--;
			_findPositionCoords(m_iInsPoint-1, false, x, y, x2,y2,uheight, bDirection, &pBlock, &pRun);
		}

		// if the run which declared itself for our position is end of paragraph run,
		// we need to ensure that the position is just before the run, not after it
		// (fixes bug 1120)
		if(pRun && pRun->getType() == FPRUN_ENDOFPARAGRAPH 
		   && (pRun->getBlockOffset() + pRun->getBlock()->getPosition()) < m_iInsPoint)
 	    {
		    m_iInsPoint--;
		}
	}

	UT_ASSERT(bRes);

	bRes = true;

	if ((UT_sint32) m_iInsPoint < (UT_sint32) posBOD)
	{
		m_iInsPoint = posBOD;
		bRes = false;
	}
	else if ((UT_sint32) m_iInsPoint > (UT_sint32) posEOD)
	{
		m_iInsPoint = posEOD;
		bRes = false;
	}

	if (m_iInsPoint != posOld)
	{
		m_pLayout->considerPendingSmartQuoteCandidate();
		_checkPendingWordForSpell();
		_clearIfAtFmtMark(posOld);
		notifyListeners(AV_CHG_MOTION);
	}

	return bRes;
}
// -------------------------------------------------------------------------

bool FV_View::canDo(bool bUndo) const
{
	return m_pDoc->canDo(bUndo);
}

UT_uint32 FV_View::undoCount (bool bUndo) const
{
  return m_pDoc->undoCount ( bUndo );
}

void FV_View::cmdUndo(UT_uint32 count)
{
	if (!isSelectionEmpty())
		_clearSelection();
	else
		_eraseInsertionPoint();

	// Signal PieceTable Change
	m_pDoc->notifyPieceTableChangeStart();

	// Turn off list updates
	m_pDoc->disableListUpdates();

	// Remember the current position, We might need it later.
	rememberCurrentPosition();

	m_pDoc->undoCmd(count);
	allowChangeInsPoint();
//
// Now do a general update to make everything look good again.
//
	_generalUpdate();
	
	notifyListeners(AV_CHG_DIRTY);

// Look to see if we need the saved insertion point after the undo
	if(needSavedPosition())
	{
//
// We do, so restore insertion point to that value.
//
		_setPoint(getSavedPosition());
		clearSavedPosition();
	}

	// Move insertion point out of field run if it is in one
	//
	_charMotion(true, 0);

//
// Do a complete update coz who knows what happened in the undo!
//
	notifyListeners(AV_CHG_ALL);

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();


	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	m_iPieceTableState = 0;
}

void FV_View::cmdRedo(UT_uint32 count)
{
	if (!isSelectionEmpty())
		_clearSelection();
	else
		_eraseInsertionPoint();

	// Signal PieceTable Change
	m_pDoc->notifyPieceTableChangeStart();

	// Turn off list updates
	m_pDoc->disableListUpdates();
	// Remember the current position, We might need it later.
	rememberCurrentPosition();

	m_pDoc->redoCmd(count);
	allowChangeInsPoint();

// Look to see if we need the saved insertion point after the undo
	if(needSavedPosition())
	{
//
// We do, so restore insertion point to that value.
//
		_setPoint(getSavedPosition());
		clearSavedPosition();
	}

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	_generalUpdate();

//
// Do a complete update coz who knows what happened in the undo!
//
	notifyListeners(AV_CHG_ALL);

	
	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	m_iPieceTableState = 0;
}

UT_Error FV_View::cmdSave(void)
{
	UT_Error tmpVar;
	tmpVar = m_pDoc->save();
	if (!tmpVar)
	{
		notifyListeners(AV_CHG_SAVE);
	}
	return tmpVar;
}

UT_Error FV_View::cmdSaveAs(const char * szFilename, int ieft, bool cpy)
{
	UT_Error tmpVar;
	tmpVar = m_pDoc->saveAs(szFilename, ieft, cpy);
	if (!tmpVar && cpy)
	{
		notifyListeners(AV_CHG_SAVE);
	}
	return tmpVar;
}

UT_Error FV_View::cmdSaveAs(const char * szFilename, int ieft)
{
  return cmdSaveAs(szFilename, ieft, true);
}


void FV_View::cmdCut(void)
{
	if (isSelectionEmpty())
	{
		// clipboard does nothing if there is no selection
		return;
	}
	// Signal PieceTable Change
	m_pDoc->notifyPieceTableChangeStart();

	//
	// Disable list updates until after we've finished
	//
	m_pDoc->disableListUpdates();
	cmdCopy();
	_deleteSelection();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	_generalUpdate();

	
	_fixInsertionPointCoords();
	_drawInsertionPoint();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	m_iPieceTableState = 0;
}

void FV_View::getDocumentRangeOfCurrentSelection(PD_DocumentRange * pdr)
{
	PT_DocPosition iPos1, iPos2;
	if (m_iSelectionAnchor < getPoint())
	{
		iPos1 = m_iSelectionAnchor;
		iPos2 = getPoint();
	}
	else
	{
		iPos1 = getPoint();
		iPos2 = m_iSelectionAnchor;
	}

	pdr->set(m_pDoc,iPos1,iPos2);
	return;
}

void FV_View::cmdCopy(void)
{
	if (isSelectionEmpty())
	{
		// clipboard does nothing if there is no selection
		return;
	}
	
	PD_DocumentRange dr;
	getDocumentRangeOfCurrentSelection(&dr);
	m_pApp->copyToClipboard(&dr);
	notifyListeners(AV_CHG_CLIPBOARD);
}

void FV_View::cmdPaste(bool bHonorFormatting)
{
	// set UAG markers around everything that the actual paste does
	// so that undo/redo will treat it as one step.
	
	m_pDoc->beginUserAtomicGlob();

	// Signal PieceTable Change
	m_pDoc->notifyPieceTableChangeStart();

	//
	// Disable list updates until after we've finished
	//
	m_pDoc->disableListUpdates();
	m_pDoc->setDoingPaste();
	m_pG->setCursor(GR_Graphics::GR_CURSOR_WAIT);
	_doPaste(true, bHonorFormatting);

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();
	setCursorToContext();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	m_iPieceTableState = 0;

	m_pDoc->clearDoingPaste();

	m_pDoc->endUserAtomicGlob();
}

void FV_View::cmdPasteSelectionAt(UT_sint32 xPos, UT_sint32 yPos)
{
	// this is intended for the X11 middle mouse paste trick.
	//
	// if this view has the selection, we need to remember it
	// before we warp to the given (x,y) -- or else there won't
	// be a selection to paste when get there.	this is sort of
	// back door hack and should probably be re-thought.
	
	// set UAG markers around everything that the actual paste does
	// so that undo/redo will treat it as one step.
	
	m_pDoc->beginUserAtomicGlob();

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	if (!isSelectionEmpty())
		m_pApp->cacheCurrentSelection(this);
	warpInsPtToXY(xPos,yPos,true);
	_doPaste(false, true);
	m_pApp->cacheCurrentSelection(NULL);

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	m_pDoc->endUserAtomicGlob();
}

void FV_View::_doPaste(bool bUseClipboard, bool bHonorFormatting)
{
	// internal portion of paste operation.
	
	if (!isSelectionEmpty())
		_deleteSelection();
	else
		_eraseInsertionPoint();

	_clearIfAtFmtMark(getPoint());
	PD_DocumentRange dr(m_pDoc,getPoint(),getPoint());
	m_pApp->pasteFromClipboard(&dr,bUseClipboard,bHonorFormatting);

	_generalUpdate();
	
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
}

bool FV_View::setSectionFormat(const XML_Char * properties[])
{
	bool bRet;

	//
	// Signal PieceTable Change 
	_saveAndNotifyPieceTableChange();
	_eraseInsertionPoint();
	if(isHdrFtrEdit())
	{
		clearHdrFtrEdit();
		warpInsPtToXY(0,0,false);
	}

	//	_clearIfAtFmtMark(getPoint()); TODO:	This was giving problems
	//											caused bug 431 when changing
	//											columns.
	
	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;
	}

	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,posStart,posEnd,NULL,properties,PTX_Section);

	_generalUpdate();
	
	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_generalUpdate();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		if (isSelectionEmpty())
		{
			_drawInsertionPoint();
		}
	}
	notifyListeners(AV_CHG_MOTION);
	return bRet;
}

/*****************************************************************/
/*****************************************************************/

void FV_View::getTopRulerInfo(AP_TopRulerInfo * pInfo)
{
	memset(pInfo,0,sizeof(*pInfo));
	if(getPoint() == 0)
	{
		return;
	}
	if (1)		// TODO support tables
	{
		// we are in a column context

		fl_BlockLayout * pBlock = NULL;
		fp_Run * pRun = NULL;
		UT_sint32 xCaret, yCaret;
		UT_uint32 heightCaret;
		UT_sint32 xCaret2, yCaret2;
		bool bDirection;
		_findPositionCoords(getPoint(), m_bPointEOL, xCaret, yCaret, xCaret2, yCaret2, heightCaret, bDirection, &pBlock, &pRun);

		fp_Container * pContainer = pRun->getLine()->getContainer();
		fl_SectionLayout * pSection = pContainer->getSectionLayout();
		if (pSection->getType() == FL_SECTION_DOC)
		{
			fp_Column* pColumn = (fp_Column*) pContainer;
			fl_DocSectionLayout* pDSL = (fl_DocSectionLayout*) pSection;
			
			UT_uint32 nCol=0;
			fp_Column * pNthColumn=pColumn->getLeader();
			while (pNthColumn && (pNthColumn != pColumn))
			{
				nCol++;
				pNthColumn = pNthColumn->getFollower();
			}
			pInfo->m_iCurrentColumn = nCol;
			pInfo->m_iNumColumns = pDSL->getNumColumns();

			pInfo->u.c.m_xaLeftMargin = pDSL->getLeftMargin();
			pInfo->u.c.m_xaRightMargin = pDSL->getRightMargin();
			pInfo->u.c.m_xColumnGap = pDSL->getColumnGap();
			pInfo->u.c.m_xColumnWidth = pColumn->getWidth();
		}
		else if(isHdrFtrEdit())
		{
			fp_Column* pColumn = (fp_Column*) pContainer;
			fl_DocSectionLayout* pDSL = (fl_DocSectionLayout*) pSection;
			pDSL = m_pEditShadow->getHdrFtrSectionLayout()->getDocSectionLayout();
			
			pInfo->m_iCurrentColumn = 0;
			pInfo->m_iNumColumns = 1;

			pInfo->u.c.m_xaLeftMargin = pDSL->getLeftMargin();
			pInfo->u.c.m_xaRightMargin = pDSL->getRightMargin();
			pInfo->u.c.m_xColumnGap = pDSL->getColumnGap();
			pInfo->u.c.m_xColumnWidth = pColumn->getWidth();
	
		}
		else
		{

		// fill in the details
		}

		pInfo->m_mode = AP_TopRulerInfo::TRI_MODE_COLUMNS;

		static char buf[20];
		setlocale(LC_NUMERIC,"C");
		snprintf(buf, sizeof(buf), "%.4fin", m_pDoc->m_docPageSize.Width(DIM_IN));
		setlocale(LC_NUMERIC,""); // restore original locale

		pInfo->m_xPaperSize = m_pG->convertDimension(buf);
		pInfo->m_xPageViewMargin = getPageViewLeftMargin();

		pInfo->m_xrPoint = xCaret - pContainer->getX();
		pInfo->m_xrLeftIndent = m_pG->convertDimension(pBlock->getProperty("margin-left"));
		pInfo->m_xrRightIndent = m_pG->convertDimension(pBlock->getProperty("margin-right"));
		pInfo->m_xrFirstLineIndent = m_pG->convertDimension(pBlock->getProperty("text-indent"));

		pInfo->m_pfnEnumTabStops = pBlock->s_EnumTabStops;
		pInfo->m_pVoidEnumTabStopsData = (void *)pBlock;
		pInfo->m_iTabStops = (UT_sint32) pBlock->getTabsCount();
		pInfo->m_iDefaultTabInterval = pBlock->getDefaultTabInterval();
		pInfo->m_pszTabStops = pBlock->getProperty("tabstops");

	}
	else
	{
		// TODO support tables
	}

	return;
}

void FV_View::getLeftRulerInfo(AP_LeftRulerInfo * pInfo)
{
	memset(pInfo,0,sizeof(*pInfo));

//
// Can't do this before the layouts are filled.
//
	if(getPoint()== 0)
	{
		return;
	}
	if (1)								// TODO support tables
	{
		// we assume that we are in a column context (rather than a table)

		pInfo->m_mode = AP_LeftRulerInfo::TRI_MODE_COLUMNS;

		fl_BlockLayout * pBlock = NULL;
		fp_Run * pRun = NULL;
		UT_sint32 xCaret, yCaret;
		UT_uint32 heightCaret;
		UT_sint32 xCaret2, yCaret2;
		bool bDirection;
		//PT_DocPosition pos = getPoint();
		_findPositionCoords(getPoint(), m_bPointEOL, xCaret, yCaret, xCaret2, yCaret2, heightCaret, bDirection, &pBlock, &pRun);

//		UT_ASSERT(pRun); // We always recover after this so no assert.
//
// No useful info here. Just return 0
//
		if(!pRun)
		{
			pInfo->m_yPageStart = 0;
			pInfo->m_yPageSize = 0;
			pInfo->m_yPoint = 0;
			pInfo->m_yTopMargin = 0;
			pInfo->m_yBottomMargin = 0;
			return;
		}
//
// No useful info here. Just return 0
//
		UT_ASSERT(pRun->getLine());
		if(!pRun->getLine())
		{
			pInfo->m_yPageStart = 0;
			pInfo->m_yPageSize = 0;
			pInfo->m_yPoint = 0;
			pInfo->m_yTopMargin = 0;
			pInfo->m_yBottomMargin = 0;
			return;
		}

		fp_Container * pContainer = pRun->getLine()->getContainer();

		pInfo->m_yPoint = yCaret - pContainer->getY();

		fl_SectionLayout * pSection = pContainer->getSectionLayout();
		fl_DocSectionLayout* pDSL = (fl_DocSectionLayout*) pSection;
		if (pSection->getType() == FL_SECTION_DOC && !isHdrFtrEdit())
		{
			fp_Column* pColumn = (fp_Column*) pContainer;
			fl_DocSectionLayout* pDSL = (fl_DocSectionLayout*) pSection;
			fp_Page * pPage = pColumn->getPage();

			UT_sint32 yoff = 0;
			getPageYOffset(pPage, yoff);
			pInfo->m_yPageStart = (UT_uint32)yoff;
			pInfo->m_yPageSize = pPage->getHeight();

			pInfo->m_yTopMargin = pDSL->getTopMargin();
			pInfo->m_yBottomMargin = pDSL->getBottomMargin();
		}
		else if(isHdrFtrEdit())
		{
			fp_Column* pColumn = (fp_Column*) pContainer;
			fp_Page * pPage = pColumn->getPage();
			fl_HdrFtrSectionLayout * pHF =	m_pEditShadow->getHdrFtrSectionLayout();
			pDSL = pHF->getDocSectionLayout();
			UT_sint32 yoff = 0;
			getPageYOffset(pPage, yoff);
			pInfo->m_yPageStart = (UT_uint32)yoff;
			pInfo->m_yPageSize = pPage->getHeight();

			if(pHF->getHFType() == FL_HDRFTR_FOOTER)
			{
				pInfo->m_yTopMargin = pPage->getHeight() + pDSL->getFooterMargin() - pDSL->getBottomMargin();
				pInfo->m_yBottomMargin = 0;
			}
			else
			{
				pInfo->m_yTopMargin = pDSL->getHeaderMargin();
				pInfo->m_yBottomMargin = pPage->getHeight() - pDSL->getTopMargin();
			}

		}
		else
		{
		}
	}
	else
	{
		// TODO support tables
	}

	return;
}

UT_Error FV_View::cmdDeleteBookmark(const char* szName)
{
	PT_DocPosition i,j;
	return _deleteBookmark(szName,true,i,j);
}

UT_Error FV_View::cmdDeleteHyperlink()
{
	PT_DocPosition pos = getPoint();
	UT_DEBUGMSG(("fv_View::cmdDeleteHyperlink: pos %d\n", pos));
	return _deleteHyperlink(pos,true);
}


UT_Error FV_View::_deleteBookmark(const char* szName, bool bSignal, PT_DocPosition &pos1, PT_DocPosition &pos2)
{
	if(!m_pDoc->isBookmarkUnique((const XML_Char *)szName))
	{
		// even though we will only send out a single explicit deleteSpan
		// call, we need to find out where both of the markers are in the
		// document, so that the caller can adjust any stored doc positions
		// if necessary
		
		fp_BookmarkRun * pB1;
		UT_uint32 bmBlockOffset[2];
		fl_BlockLayout * pBlock[2];
		UT_uint32 i = 0;
		
		fl_BlockLayout *pBL;
		fl_SectionLayout *pSL = m_pLayout->getFirstSection();
		fp_Run * pRun;
		bool bFound = false;
		
		//find the first of the two bookmarks
		while(pSL)
		{
			pBL = pSL->getFirstBlock();
			
			while(pBL)	
			{
				pRun = pBL->getFirstRun();
				
				while(pRun)
				{
					if(pRun->getType()== FPRUN_BOOKMARK)
					{
						pB1 = static_cast<fp_BookmarkRun*>(pRun);
						if(!UT_XML_strcmp((const XML_Char *)szName, pB1->getName()))
						{
							bmBlockOffset[i] = pRun->getBlockOffset();
							pBlock[i] = pRun->getBlock();
							i++;
							if(i>1)
							{
								bFound = true;
								break;
							}
						}
					}
					if(bFound)
						break;
					pRun = pRun->getNext();
				}
				if(bFound)
					break;
				pBL = pBL->getNext();
			}
			if(bFound)
				break;
			pSL = pSL->getNext();
		}
		
		UT_ASSERT(pRun && pRun->getType()==FPRUN_BOOKMARK && pBlock || pBlock);
		if(!pRun || pRun->getType()!=FPRUN_BOOKMARK || !pBlock || !pBlock)
			return false;
			
		// Signal PieceTable Change
		if(bSignal)
			_saveAndNotifyPieceTableChange();
			
		UT_DEBUGMSG(("fv_View::cmdDeleteBookmark: bl pos [%d,%d], bmOffset [%d,%d]\n",
					 pBlock[0]->getPosition(false), pBlock[1]->getPosition(false),bmBlockOffset[0],bmBlockOffset[1]));
					
		pos1 = pBlock[0]->getPosition(false) + bmBlockOffset[0];
		pos2 = pBlock[1]->getPosition(false) + bmBlockOffset[1];

		m_pDoc->deleteSpan(pos1,pos1 + 1);
		
		// Signal PieceTable Changes have finished
		if(bSignal)
		{
			_generalUpdate();
			_restorePieceTableState();
		}
	}
	else
		UT_DEBUGMSG(("fv_View::cmdDeleteBookmark: bookmark \"%s\" does not exist\n",szName));
	return true;
}

UT_Error FV_View::cmdHyperlinkStatusBar(UT_sint32 xPos, UT_sint32 yPos)
{
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);

	PT_DocPosition pos;
	bool bBOL = false;
	bool bEOL = false;
	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL);

	// now get the run at the position and the hyperlink run
	fp_HyperlinkRun * pH1 = 0;
		
	fl_BlockLayout *pBlock = _findBlockAtPosition(pos);
	PT_DocPosition curPos = pos - pBlock->getPosition(false);
	
	fp_Run * pRun = pBlock->getFirstRun();
		
	//find the run at pos1
	while(pRun && pRun->getBlockOffset() <= curPos)
		pRun = pRun->getNext();

	// this sometimes happens, not sure why 	
	//UT_ASSERT(pRun);
	if(!pRun)
		return false;
	
	// now we have the run immediately after the run in question, so
	// we step back
	pRun = pRun->getPrev();
	UT_ASSERT(pRun);
	if(!pRun)
		return false;

	xxx_UT_DEBUGMSG(("fv_View::cmdHyperlinkStatusBar: run 0x%x, type %d\n", pRun,pRun->getType()));
	pH1 = pRun->getHyperlink();

	// this happens after a deletion of a hyperlink 
	// the mouse processing is in the state of belief
	// that the processing has not finished yet -- this is not specific
	// to hyperlinks, it happens with anything on the context menu, except
	// it goes unobserved since the cursor does not change
	//UT_ASSERT(pH1);
	if(!pH1)
		return false;
	xxx_UT_DEBUGMSG(("fv_View::cmdHyperlinkStatusBar: msg [%s]\n",pH1->getTarget()));		
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (getParentData());		
	pFrame->setStatusMessage(pH1->getTarget());
	return true;
}
/*
	NB: this function assumes that the position it is passed is inside a
	hyperlink and will assert if it is not so.
*/

UT_Error FV_View::_deleteHyperlink(PT_DocPosition &pos1, bool bSignal)
{
	fp_HyperlinkRun * pH1 = 0;
		
	fl_BlockLayout *pBlock = _findBlockAtPosition(pos1);
	PT_DocPosition curPos = pos1 - pBlock->getPosition(false);
	
	fp_Run * pRun = pBlock->getFirstRun();
		
	//find the run at pos1
	while(pRun && pRun->getBlockOffset() <= curPos)
		pRun = pRun->getNext();
		
	UT_ASSERT(pRun);
	if(!pRun)
		return false;
	
	// now we have the run immediately after the run in question, so
	// we step back
	pRun = pRun->getPrev();
	UT_ASSERT(pRun);
	if(!pRun)
		return false;
	
	pH1 = pRun->getHyperlink();
	
	UT_ASSERT(pH1);
	if(!pH1)
		return false;

	pos1 = pH1->getBlock()->getPosition(false) + pH1->getBlockOffset();

	// now reset the hyperlink member for the runs that belonged to this
	// hyperlink

	pRun = pH1->getNext();
	UT_ASSERT(pRun);
	while(pRun && pRun->getHyperlink() != NULL)
	{
		UT_DEBUGMSG(("fv_View::_deleteHyperlink: reseting run 0x%x\n", pRun));
		pRun->setHyperlink(NULL);
		pRun = pRun->getNext();
	}

	UT_ASSERT(pRun);

	// Signal PieceTable Change
	if(bSignal)
		_saveAndNotifyPieceTableChange();
			
	UT_DEBUGMSG(("fv_View::cmdDeleteHyperlink: position [%d]\n",
				pos1));
					
	m_pDoc->deleteSpan(pos1,pos1 + 1);
		
	// Signal PieceTable Changes have finished
	if(bSignal)
	{
		_generalUpdate();
		_restorePieceTableState();
	}
	return true;
}

/******************************************************************/

UT_Error FV_View::cmdInsertHyperlink(const char * szName)
{
	bool bRet;

	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;
	
	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
		{
			posStart = m_iSelectionAnchor;
		}
		else
		{
			posEnd = m_iSelectionAnchor;
		}
	}
	else
	{
		//No selection
		XAP_Frame * pFrame = (XAP_Frame *) getParentData();
		UT_ASSERT((pFrame));
		
		const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
		const char *pMsg1 = pSS->getValue(AP_STRING_ID_MSG_HyperlinkNoSelection);
		
		UT_ASSERT(pMsg1);
		
		pFrame->showMessageBox(pMsg1, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
		return false;
	}
	
	if(!UT_isUrl(szName) && m_pDoc->isBookmarkUnique(szName))
	{
		//No bookmark of that name in document
		XAP_Frame * pFrame = (XAP_Frame *) getParentData();
		UT_ASSERT((pFrame));
		
		const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
		const char *pMsg1 = pSS->getValue(AP_STRING_ID_MSG_HyperlinkNoBookmark);
		UT_ASSERT(pMsg1);
		
		char * szMsg = new char[strlen(pMsg1)+strlen(szName)+1];
		UT_ASSERT(szMsg);
		sprintf(szMsg,pMsg1,szName);
		
		pFrame->showMessageBox(szMsg, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
		delete[] szMsg;
	}

	// the selection has to be within a single block
	// we could implement hyperlinks spaning arbitrary part of the document
	// but then we could not use <a href=> </a> in the output and
	// I see no obvious need for hyperlinks to span more than a single block
	fl_BlockLayout * pBl1 = _findBlockAtPosition(posStart);
	fl_BlockLayout * pBl2 = _findBlockAtPosition(posEnd);

	if(pBl1 != pBl2)
	{
		XAP_Frame * pFrame = (XAP_Frame *) getParentData();
		UT_ASSERT((pFrame));
		
		const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
		const char *pMsg1 = pSS->getValue(AP_STRING_ID_MSG_HyperlinkCrossesBoundaries);
		UT_ASSERT(pMsg1);
		
		pFrame->showMessageBox(pMsg1, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
		
		return false;
	}

	
	XML_Char * pAttr[4];
	const XML_Char ** pAt = (const XML_Char **)&pAttr[0];

	UT_uint32 target_len = UT_XML_strlen(szName);
	XML_Char * target  = new XML_Char[ target_len+ 2];
	
	if(UT_isUrl(szName))
	{
		UT_XML_strncpy(target, target_len + 1, (XML_Char*)szName);
	}
	else
	{
		target[0] =  '#';
		UT_XML_strncpy(target + 1, target_len + 1, (XML_Char*)szName);
	}

	XML_Char target_l[]  = "xlink:href";
	pAttr [0] = &target_l[0];
	pAttr [1] = &target[0];
	pAttr [2] = 0;
	pAttr [3] = 0;
	
	UT_DEBUGMSG(("fv_View::cmdInsertHyperlink: target \"%s\"\n", target));
	
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	// we first insert the end run, so that we can use it as a stop
	// after inserting the start run when marking the runs in between
	// as a hyperlink	
	bRet = m_pDoc->insertObject(posEnd, PTO_Hyperlink, NULL, NULL);
	
	
	if(bRet)
	{
		bRet = m_pDoc->insertObject(posStart, PTO_Hyperlink, pAt, NULL);
	}
	
	delete [] target;
	
	_generalUpdate();

	// Signal piceTable is stable again
	_restorePieceTableState();

	return bRet;

}

/******************************************************************/
UT_Error FV_View::cmdInsertBookmark(const char * szName)
{
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();
	bool bRet;

	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;
	PT_DocPosition pos1 = 0xFFFFFFFF,pos2 = 0xFFFFFFFF;
	
	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
		{
			posStart = m_iSelectionAnchor;
		}
		else
		{
			posEnd = m_iSelectionAnchor;
		}
	}

	posEnd++;
		
	if(!m_pDoc->isBookmarkUnique((XML_Char*)szName))
	{
		//bookmark already exists -- remove it and then reinsert
		UT_DEBUGMSG(("fv_View::cmdInsertBookmark: bookmark \"%s\" exists - removing\n", szName));
		_deleteBookmark((XML_Char*)szName, false,pos1,pos2);
	}


	// if the bookmark we just deleted was before the current insertion
	// position we have to adjust our positions correspondingly
	if(posStart > pos1)
		posStart--;
	if(posStart > pos2)
		posStart--;
	if(posEnd > pos1)
		posEnd--;
	if(posEnd > pos2)
		posEnd--;
	
	XML_Char * pAttr[6];
	const XML_Char ** pAt = (const XML_Char **)&pAttr[0];

	XML_Char name_l [] = "name";
	XML_Char type_l [] = "type";
	XML_Char name[BOOKMARK_NAME_SIZE + 1];
	UT_XML_strncpy(name, BOOKMARK_NAME_SIZE, (XML_Char*)szName);
	name[BOOKMARK_NAME_SIZE] = 0;
	
	XML_Char type[] = "start";
	pAttr [0] = &name_l[0];
	pAttr [1] = &name[0];
	pAttr [2] = &type_l[0];
	pAttr [3] = &type[0];
	pAttr [4] = 0;
	pAttr [5] = 0;
	
	UT_DEBUGMSG(("fv_View::cmdInsertBookmark: szName \"%s\"\n", szName));
	
	bRet = m_pDoc->insertObject(posStart, PTO_Bookmark, pAt, NULL);
	
	
	if(bRet)
	{
		UT_XML_strncpy(type, 3,(XML_Char*)"end");
		type[3] = 0;
		bRet = m_pDoc->insertObject(posEnd, PTO_Bookmark, pAt, NULL);
	}
	
	_generalUpdate();

	// Signal piceTable is stable again
	_restorePieceTableState();

	return bRet;

}


/*****************************************************************/
UT_Error FV_View::cmdInsertField(const char* szName)
{
	return cmdInsertField(szName, NULL);
}

UT_Error FV_View::cmdInsertField(const char* szName, const XML_Char ** extra_attrs)
{
	bool bResult;

	int attrCount = 0;
	while (extra_attrs && extra_attrs[attrCount] != NULL)
	{
		attrCount++;
	}

	const XML_Char ** attributes = new const XML_Char*[attrCount+4];

	int i = 0;
	while (extra_attrs && extra_attrs[i] != NULL)
	{
		attributes[i] = extra_attrs[i];
		i++;
	}
	attributes[i++] = "type";
	attributes[i++] = szName;
	attributes[i++] = NULL;
	attributes[i++] = NULL;
	
/*
  currently unused
  fl_BlockLayout* pBL = _findBlockAtPosition(getPoint());
*/

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	fd_Field * pField = NULL;
	if (!isSelectionEmpty())
	{
		m_pDoc->beginUserAtomicGlob();
		_deleteSelection();
		bResult = m_pDoc->insertObject(getPoint(), PTO_Field, attributes, NULL,&pField);
		if(pField != NULL)
		{
			pField->update();
		}
		m_pDoc->endUserAtomicGlob();
	}
	else
	{
		_eraseInsertionPoint();
		bResult = m_pDoc->insertObject(getPoint(), PTO_Field, attributes, NULL, &pField);		if(pField != NULL)
		{
			pField->update();
		}
	}

	delete [] attributes;
	
	_generalUpdate();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
	return bResult;
}

UT_Error FV_View::_insertGraphic(FG_Graphic* pFG, const char* szName)
{
	UT_ASSERT(pFG);
	UT_ASSERT(szName);

	if (!pFG)
	  return UT_ERROR;

	double fDPI = m_pG->getResolution() * 100. / m_pG->getZoomPercentage();
	return pFG->insertIntoDocument(m_pDoc, fDPI, getPoint(), szName);
}

UT_Error FV_View::cmdInsertGraphic(FG_Graphic* pFG, const char* pszName)
{
	bool bDidGlob = false;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	if (!isSelectionEmpty())
	{
		bDidGlob = true;
		m_pDoc->beginUserAtomicGlob();
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	/*
	  First, find a unique name for the data item.
	*/
	char *szName = new char [strlen (pszName) + 64 + 1];
	UT_uint32 ndx = 0;
	for (;;)
	{
		sprintf(szName, "%s_%d", pszName, ndx);
		if (!m_pDoc->getDataItemDataByName(szName, NULL, NULL, NULL))
		{
			break;
		}
		ndx++;
	}

	UT_Error errorCode = _insertGraphic(pFG, szName);

	if (bDidGlob)
		m_pDoc->endUserAtomicGlob();

	_generalUpdate();

	_restorePieceTableState();
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	delete [] szName;

	return errorCode;
}

bool FV_View::isPosSelected(PT_DocPosition pos) const
{
	if (!isSelectionEmpty())
	{
		PT_DocPosition posStart = getPoint();
		PT_DocPosition posEnd = posStart;

		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;

		return ((pos >= posStart) && (pos <= posEnd));
	}

	return false;
}

bool FV_View::isXYSelected(UT_sint32 xPos, UT_sint32 yPos) const
{
	if (isSelectionEmpty())
		return false;

	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);
	if (!pPage)
		return false;

	if (   (yClick < 0)
		   || (xClick < 0)
		   || (xClick > pPage->getWidth()) )
	{
		return false;
	}

	PT_DocPosition pos;
	bool bBOL, bEOL;
	fl_HdrFtrShadow * pShadow=NULL;
	pPage->mapXYToPositionClick(xClick, yClick, pos, pShadow, bBOL, bEOL);

	return isPosSelected(pos);
}

EV_EditMouseContext FV_View::getMouseContext(UT_sint32 xPos, UT_sint32 yPos)
{
	UT_sint32 xClick, yClick;
	PT_DocPosition pos;
	bool bBOL = false;
	bool bEOL = false;
	UT_sint32 xPoint, yPoint, iPointHeight;
	UT_sint32 xPoint2, yPoint2;
	bool bDirection;
	m_iMouseX = xPos;
	m_iMouseY = yPos;
	if(getPoint() == 0) // We haven't loaded any layouts yet
	{
		return EV_EMC_UNKNOWN;
	}
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);
	if (!pPage)
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (1)\n"));
		return EV_EMC_UNKNOWN;
	}


	if (   (yClick < 0)
		   || (xClick < 0)
		   || (xClick > pPage->getWidth()) )
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (2)\n"));
		return EV_EMC_UNKNOWN;
	}

#ifdef BIDI_ENABLED 
	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL);
	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);
#endif
	
	if (isLeftMargin(xPos,yPos))
	{
#ifdef BIDI_ENABLED
		UT_ASSERT(pBlock);
		xxx_UT_DEBUGMSG(("Entered BOL margin: dir %d\n", pBlock->getDominantDirection()));
		if(pBlock->getDominantDirection() == FRIBIDI_TYPE_RTL)
		{
			xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (3)\n"));
			return EV_EMC_RIGHTOFTEXT;
		}
		else
		{
			xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (4)\n"));
			return EV_EMC_LEFTOFTEXT;
		}
#else
		return EV_EMC_LEFTOFTEXT;
#endif
	}

#ifndef BIDI_ENABLED	
	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL);
	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);
#endif
	if (!pBlock)
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (5)\n"));
		return EV_EMC_UNKNOWN;
	}
	fp_Run* pRun = pBlock->findPointCoords(pos, bEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);
	
	if (!pRun)
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (6)\n"));
		return EV_EMC_UNKNOWN;
	}

	if(pRun->getHyperlink() != NULL)
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (7), run type %d\n", pRun->getType()));
		return EV_EMC_HYPERLINK;
	}
		
	switch (pRun->getType())
	{
	case FPRUN_TEXT:
		if (!isPosSelected(pos))
			if (pBlock->getSquiggles()->get(pos - pBlock->getPosition()))
			{
				xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (8)\n"));
				return EV_EMC_MISSPELLEDTEXT;
			}
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (9)\n"));
		return EV_EMC_TEXT;
		
	case FPRUN_IMAGE:
		// TODO see if the image is selected and current x,y
		// TODO is over the image border or the border handles.
		// TODO if so, return EV_EMC_IMAGESIZE
		return EV_EMC_IMAGE;
		
	case FPRUN_TAB:
	case FPRUN_FORCEDLINEBREAK:
	case FPRUN_FORCEDCOLUMNBREAK:
	case FPRUN_FORCEDPAGEBREAK:
	case FPRUN_FMTMARK:
	case FPRUN_ENDOFPARAGRAPH:
	case FPRUN_BOOKMARK:
	case FPRUN_HYPERLINK:
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (10)\n"));
		return EV_EMC_TEXT;
		
	case FPRUN_FIELD:
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (11)\n"));
		return EV_EMC_FIELD;
		
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (12)\n"));
		return EV_EMC_UNKNOWN;
	}

	/*NOTREACHED*/
	UT_ASSERT(0);
	xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (13)\n"));
	return EV_EMC_UNKNOWN;
}

void FV_View::setCursorToContext()
{
	EV_EditMouseContext evMC = getMouseContext(m_iMouseY,m_iMouseY);
	GR_Graphics::Cursor cursor = GR_Graphics::GR_CURSOR_DEFAULT;
	switch (evMC)
	{
	case EV_EMC_UNKNOWN:
		break;
	case EV_EMC_TEXT:
		cursor = GR_Graphics::GR_CURSOR_IBEAM;
		break;
	case EV_EMC_LEFTOFTEXT:
		cursor = GR_Graphics::GR_CURSOR_RIGHTARROW;
		break;
	case EV_EMC_MISSPELLEDTEXT:
		cursor = GR_Graphics::GR_CURSOR_IBEAM;
		break;
	case EV_EMC_IMAGE:
		cursor = GR_Graphics::GR_CURSOR_IMAGE;
		break;
	case EV_EMC_IMAGESIZE:
		cursor = GR_Graphics::GR_CURSOR_IMAGE;
		break;
	case EV_EMC_FIELD:
		cursor = GR_Graphics::GR_CURSOR_DEFAULT;
		break;
	case EV_EMC_HYPERLINK:
		cursor = GR_Graphics::GR_CURSOR_LINK;
		break;

#ifdef BIDI_ENABLED
		case EV_EMC_RIGHTOFTEXT:
			cursor = GR_Graphics::GR_CURSOR_LEFTARROW;
			break;
#endif
	case EV_EMC_HYPERLINKTEXT:
		cursor = GR_Graphics::GR_CURSOR_LINK;
		break;
	case EV_EMC_HYPERLINKMISSPELLED:
		cursor = GR_Graphics::GR_CURSOR_LINK;
		break;
	default:
		break;
	}
	getGraphics()->setCursor(cursor);
}

bool FV_View::isTextMisspelled() const
{
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);
	
	if (!isPosSelected(pos))
		if (pBlock->getSquiggles()->get(pos - pBlock->getPosition()))
		{
			return true;
		}
	
	return false;
}


EV_EditMouseContext FV_View::getInsertionPointContext(UT_sint32 * pxPos, UT_sint32 * pyPos)
{
	// compute an EV_EMC_ context for the position
	// of the current insertion point and return
	// the window coordinates of the insertion point.
	// this is to allow a keyboard binding to raise
	// a context menu.

	if (pxPos)
		*pxPos = m_xPoint;
	if (pyPos)
		*pyPos = m_yPoint + m_iPointHeight;
	
	fl_BlockLayout* pBlock = _findBlockAtPosition(m_iInsPoint);

	if (!pBlock)
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (5)\n"));
		return EV_EMC_UNKNOWN;
	}

	UT_sint32 x, y, x2, y2, h;
	bool b;
	
	fp_Run* pRun = pBlock->findPointCoords(m_iInsPoint, false, x, y, x2, y2, h, b);
	
	if (!pRun)
	{
		return EV_EMC_UNKNOWN;
	}

	if(pRun->getHyperlink() != NULL)
	{
		return EV_EMC_HYPERLINK;
	}
		
	switch (pRun->getType())
	{
	case FPRUN_TEXT:
		if (!isPosSelected(m_iInsPoint))
			if (pBlock->getSquiggles()->get(m_iInsPoint - pBlock->getPosition()))
			{
				return EV_EMC_MISSPELLEDTEXT;
			}
		return EV_EMC_TEXT;
		
	case FPRUN_IMAGE:
		// TODO see if the image is selected and current x,y
		// TODO is over the image border or the border handles.
		// TODO if so, return EV_EMC_IMAGESIZE
		return EV_EMC_IMAGE;
		
	case FPRUN_TAB:
	case FPRUN_FORCEDLINEBREAK:
	case FPRUN_FORCEDCOLUMNBREAK:
	case FPRUN_FORCEDPAGEBREAK:
	case FPRUN_FMTMARK:
	case FPRUN_ENDOFPARAGRAPH:
	case FPRUN_BOOKMARK:
	case FPRUN_HYPERLINK:
		return EV_EMC_TEXT;
		
	case FPRUN_FIELD:
		return EV_EMC_FIELD;
		
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		return EV_EMC_UNKNOWN;
	}

	/*NOTREACHED*/
	UT_ASSERT(0);
	return EV_EMC_UNKNOWN;
}

fp_Page* FV_View::getCurrentPage(void) const
{
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
	UT_uint32 pos = getPoint();
	UT_sint32 xPoint2, yPoint2;
	bool bDirection;
	
//
// Check we have a vaild layout. At startup we don't
//
	if(getLayout()->getFirstSection() == NULL)
	{
		return NULL;
	}
	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);
	UT_ASSERT(pBlock);
	fp_Run* pRun = pBlock->findPointCoords(pos, m_bPointEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);

//
// Detect if we have no Lines at the curren tpoint and bail out.
//
	if(iPointHeight == 0)
	{
		return NULL;
	}

	// NOTE prior call will fail if the block isn't currently formatted,
	// NOTE so we won't be able to figure out more specific geometry

	if (pRun)
	{
		// we now have coords relative to the page containing the ins pt
		fp_Page* pPointPage = pRun->getLine()->getContainer()->getPage();

		return pPointPage;
	}

	return NULL;
}

/*!
 * Returns true if there is some Layout classes defined. Not true at startup.
 */
bool FV_View::isDocumentPresent(void)
{
	return (getLayout()->getFirstSection() != NULL);
}

UT_uint32 FV_View::getCurrentPageNumForStatusBar(void) const
{
	fp_Page* pCurrentPage = getCurrentPage();
	if(pCurrentPage == NULL)
	{
		return 0;
	}
	UT_uint32 ndx = 1;

	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		if (pPage == pCurrentPage)
		{
			return ndx;
		}

		ndx++;
		pPage = pPage->getNext();
	}
	UT_DEBUGMSG(("Current page not found - could be has been deleted. Hopefully we can recover \n"));
//	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return 0;
}

void FV_View::_clearIfAtFmtMark(PT_DocPosition dpos)
{
	// Check to see if we're at the beginning of the line. If we
	// aren't, then it's safe to delete the FmtMark. Else we could
	// wipe out the placeholder FmtMark for our attributes.
	// Fix for Bug #863 
	if ( ( dpos != _getDocPosFromPoint(dpos,FV_DOCPOS_BOL) )) 
	{
		m_pDoc->clearIfAtFmtMark(dpos);
		_generalUpdate();//  Sevior: May be able to live with notify.. always
	}
	else
	{
		notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR | AV_CHG_FMTBLOCK);
	}
}

// ndx is one-based, not zero-based
UT_UCSChar * FV_View::getContextSuggest(UT_uint32 ndx)
{
	// locate the squiggle
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBL = _findBlockAtPosition(pos);
	UT_ASSERT(pBL);
	fl_PartOfBlock* pPOB = pBL->getSquiggles()->get(pos - pBL->getPosition());
	UT_ASSERT(pPOB);

	// grab the suggestion
	return _lookupSuggestion(pBL, pPOB, ndx);
}

// NB: returns a UCS string that the caller needs to FREEP
UT_UCSChar * FV_View::_lookupSuggestion(fl_BlockLayout* pBL, 
										fl_PartOfBlock* pPOB, UT_uint32 ndx)
{
	// mega caching - are these assumptions valid?
	UT_UCSChar * szSuggest = NULL;

	static fl_BlockLayout * pLastBL = 0;
	static fl_PartOfBlock * pLastPOB = 0;
	static UT_Vector * pSuggestionCache = 0;

	if (pBL == pLastBL && pLastPOB == pPOB)
	{
		if ((pSuggestionCache->getItemCount()) &&
			( ndx <= pSuggestionCache->getItemCount()))
		{
			UT_UCS_cloneString(&szSuggest, 
							   (UT_UCSChar *) pSuggestionCache->getNthItem(ndx-1));
		}
		return szSuggest;
	}

	if (pSuggestionCache) // got here, so we need to invalidate the cache
	{
		// clean up
		for (UT_uint32 i = 0; i < pSuggestionCache->getItemCount(); i++)
		{
			UT_UCSChar * sug = (UT_UCSChar *)pSuggestionCache->getNthItem(i);
			FREEP(sug);
		}

		pLastBL = 0;
		pLastPOB = 0;
		DELETEP(pSuggestionCache);
	}

	// grab a copy of the word
	UT_GrowBuf pgb(1024);
	bool bRes = pBL->getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar * pWord = pgb.getPointer(pPOB->getOffset());

	// lookup suggestions
	UT_Vector * sg = 0;

	UT_UCSChar theWord[INPUTWORDLEN + 1];
	// convert smart quote apostrophe to ASCII single quote to be
	// compatible with ispell
	UT_uint32 len = pPOB->getLength();
	for (UT_uint32 ldex=0; ldex < len && ldex < INPUTWORDLEN; ldex++)
	{
		UT_UCSChar currentChar;
		currentChar = *(pWord + ldex);
		if (currentChar == UCS_RQUOTE) currentChar = '\'';
		theWord[ldex] = currentChar;
	}

	{
		SpellChecker * checker = NULL;
		const char * szLang = NULL;

		const XML_Char ** props_in = NULL;
	
		if (getCharFormat(&props_in))
		{
			szLang = UT_getAttribute("lang", props_in);
			FREEP(props_in);
		}
		
		if (szLang)
		{
			// we get smart and request the proper dictionary
			checker = SpellManager::instance().requestDictionary(szLang);
		}
		else
		{
			// we just (dumbly) default to the last dictionary
			checker = SpellManager::instance().lastDictionary();
		}

		sg = checker->suggestWord (theWord, pPOB->getLength());
		
	}

	if (!sg)
	{
		UT_DEBUGMSG(("DOM: no suggestions returned\n"));
		DELETEP(sg);
		return 0;
	}

	// we currently return all requested suggestions
	if ((sg->getItemCount()) &&
		( ndx <= sg->getItemCount()))
	{
		UT_UCS_cloneString(&szSuggest, (UT_UCSChar *) sg->getNthItem(ndx-1));
	}

	pSuggestionCache = sg;
	pLastBL = pBL;
	pLastPOB = pPOB;

	return szSuggest;
}

void FV_View::cmdContextSuggest(UT_uint32 ndx, fl_BlockLayout * ppBL,
								fl_PartOfBlock * ppPOB)
{
	// locate the squiggle
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBL;
	fl_PartOfBlock* pPOB;

	if (!ppBL)
		pBL = _findBlockAtPosition(pos);
	else
		pBL = ppBL;
	UT_ASSERT(pBL);

	if (!ppPOB)
		pPOB = pBL->getSquiggles()->get(pos - pBL->getPosition());
	else
		pPOB = ppPOB;
	UT_ASSERT(pPOB);

	// grab the suggestion
	UT_UCSChar * replace = _lookupSuggestion(pBL, pPOB, ndx);

	if (!replace)
		return;

	// make the change
	UT_ASSERT(isSelectionEmpty());

	moveInsPtTo((PT_DocPosition) (pBL->getPosition() + pPOB->getOffset()));
	extSelHorizontal(true, pPOB->getLength());
	cmdCharInsert(replace, UT_UCS_strlen(replace));

	FREEP(replace);
}

void FV_View::cmdContextIgnoreAll(void)
{
	// locate the squiggle
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBL = _findBlockAtPosition(pos);
	UT_ASSERT(pBL);
	fl_PartOfBlock* pPOB = pBL->getSquiggles()->get(pos - pBL->getPosition());
	UT_ASSERT(pPOB);

	// grab a copy of the word
	UT_GrowBuf pgb(1024);
	bool bRes = pBL->getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar * pBuf = pgb.getPointer(pPOB->getOffset());

	// make the change
	if (m_pDoc->appendIgnore(pBuf, pPOB->getLength()))
	{
		// remove the squiggles, too
		fl_DocSectionLayout * pSL = m_pLayout->getFirstSection();
		while (pSL)
		{
			fl_BlockLayout* b = pSL->getFirstBlock();
			while (b)
			{
				// TODO: just check and remove matching squiggles
				// for now, destructively recheck the whole thing
				m_pLayout->queueBlockForBackgroundCheck(FL_DocLayout::bgcrSpelling, b);
				b = b->getNext();
			}
			pSL = (fl_DocSectionLayout *) pSL->getNext();
		}
	}
}

void FV_View::cmdContextAdd(void)
{
	// locate the squiggle
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBL = _findBlockAtPosition(pos);
	UT_ASSERT(pBL);
	fl_PartOfBlock* pPOB = pBL->getSquiggles()->get(pos - pBL->getPosition());
	UT_ASSERT(pPOB);

	// grab a copy of the word
	UT_GrowBuf pgb(1024);
	bool bRes = pBL->getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar * pBuf = pgb.getPointer(pPOB->getOffset());

	// make the change
	if (m_pApp->addWordToDict(pBuf, pPOB->getLength()))
	{
		// remove the squiggles, too
		fl_DocSectionLayout * pSL = m_pLayout->getFirstSection();
		while (pSL)
		{
			fl_BlockLayout* b = pSL->getFirstBlock();
			while (b)
			{
				// TODO: just check and remove matching squiggles
				// for now, destructively recheck the whole thing
				m_pLayout->queueBlockForBackgroundCheck(FL_DocLayout::bgcrSpelling, b);
				b = b->getNext();
			}
			pSL = (fl_DocSectionLayout *) pSL->getNext();
		}
	}
}


/*static*/ void FV_View::_prefsListener( XAP_App * /*pApp*/, XAP_Prefs *pPrefs, UT_StringPtrMap * /*phChanges*/, void *data )
{
	FV_View *pView = (FV_View *)data;
	bool b;
	UT_ASSERT(data && pPrefs);
	if ( pPrefs->getPrefsValueBool((XML_Char*)AP_PREF_KEY_CursorBlink, &b) && b != pView->m_bCursorBlink )
	{
		UT_DEBUGMSG(("FV_View::_prefsListener m_bCursorBlink=%s m_bCursorIsOn=%s\n",
					 pView->m_bCursorBlink ? "TRUE" : "FALSE",
					 pView->m_bCursorIsOn ? "TRUE" : "FALSE"));

		pView->m_bCursorBlink = b;

		if ( pView->m_bCursorBlink ) 
		{
			// start the cursor blinking
			pView->_eraseInsertionPoint();
			pView->_drawInsertionPoint();
		}
		else
		{
			// stop blinking and make sure the cursor is drawn
			if ( pView->m_pAutoCursorTimer )
				pView->m_pAutoCursorTimer->stop();

			if ( !pView->m_bCursorIsOn )
				pView->_drawInsertionPoint();
		}
	}
#ifdef BIDI_ENABLED 
	if (( pPrefs->getPrefsValueBool((XML_Char*)AP_PREF_KEY_DefaultDirectionRtl, &b) && b != pView->m_bDefaultDirectionRtl)
		 || (pPrefs->getPrefsValueBool((XML_Char*)XAP_PREF_KEY_UseHebrewContextGlyphs, &b) && b != pView->m_bUseHebrewContextGlyphs)
		)
	{
		/*	It is possible to change this at runtime, but it may impact the
			way the document is displayed in an unexpected way (from the user's
			point of view). It is therefore probably better to apply this change
			only when AW will be restarted or a new document is created and
			notify the user about that.
		*/
		XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
		UT_ASSERT((pFrame));
		
		const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
		//const char *pMsg1 = pSS->getValue(AP_STRING_ID_MSG_DefaultDirectionChg);
		const char *pMsg2 = pSS->getValue(AP_STRING_ID_MSG_AfterRestartNew);
		
		UT_ASSERT((/*pMsg1 && */pMsg2));
		
		//UT_uint32 len = /*strlen(pMsg1)*/ + strlen(pMsg2) + 2;
		
		//char * szMsg = new char[len];
		//UT_ASSERT((szMsg));
		
		//sprintf(szMsg, "%s\n%s", pMsg1, pMsg2);
		
		pFrame->showMessageBox(pMsg2, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);

		//delete[](szMsg);
	
	}
#endif	
}

/******************************************************
 *****************************************************/
bool s_notChar(UT_UCSChar c)
{
	bool res = false;

	switch (c)
	{
	case UCS_TAB:
	case UCS_LF:
	case UCS_VTAB:
	case UCS_FF:
	case UCS_CR:
	{
		res = true;
		break;
	}
	default:
		break;
	}
	return res;
}

/*!
 Count words
 \return structure with word counts
 Count words in document (or selection).
*/
FV_DocCount
FV_View::countWords(void)
{
	FV_DocCount wCount;
	memset(&wCount,0,sizeof(wCount));

	bool isPara = false;

	PT_DocPosition low, high;

	// Find selection bounds - if no selection, use bounds of entire
	// document
	if (isSelectionEmpty())
	{
		getEditableBounds(false, low);
		getEditableBounds(true, high);
	}
	else
	{
		if(m_iInsPoint < m_iSelectionAnchor)
		{
			low  = m_iInsPoint;
			high = m_iSelectionAnchor;
		}
		else
		{
			high = m_iInsPoint;
			low  = m_iSelectionAnchor;
		}
	}

	UT_sint32 iSelLen = high - low;
	fl_BlockLayout * pBL = _findBlockAtPosition(low);

	// Selection may start inside first block
	UT_sint32 iStartOffset = 0, iLineOffset = 0, iCount = 0;
	fp_Line* pLine = pBL->getFirstLine();
	fp_Run* pRun = pLine->getFirstRun();
	fp_Page* pPage = pLine->getContainer()->getPage();
	wCount.page = 1;
	if (low > pBL->getPosition())
	{
		iStartOffset = low - pBL->getPosition();
		// Find first line in selection
		if (pLine && iLineOffset < iStartOffset)
		{
			fp_Run* pPrevRun = NULL;
			while (pRun && iLineOffset < iStartOffset)
			{
				iLineOffset += pRun->getLength();
				pPrevRun = pRun;
				pRun = pRun->getNext();
			}
			UT_ASSERT(pRun);
			if (!pRun)
			{
				pRun = pBL->getNext()->getFirstRun();
			}

			iLineOffset -= iStartOffset;
			UT_ASSERT(iLineOffset >= 0);
			pLine = pRun->getLine();
			pPage = pLine->getContainer()->getPage();
			if (pPrevRun->getLine() != pLine)
			{
				wCount.line++;
				if (pPrevRun->getLine()->getContainer()->getPage() != pPage)
					wCount.page++;
			}
		}
	}

	while (NULL != pBL && iCount < iSelLen)
	{
		UT_GrowBuf gb(1024);
		pBL->getBlockBuf(&gb);
		const UT_UCSChar * pSpan = gb.getPointer(0);
		UT_uint32 len = gb.getLength();

		UT_uint32 i = iStartOffset;
		iStartOffset = 0;

		// Count lines and pages
		while (pLine && iLineOffset < iSelLen)
		{
			wCount.line++;

			// If this line is on a new page, increment page count
			if (pLine->getContainer()->getPage() != pPage)
			{
				wCount.page++;
				pPage = pLine->getContainer()->getPage();
			}

			// Add length for runs on this line
			while (pRun && pRun->getLine() == pLine)
			{
				iLineOffset += pRun->getLength();
				pRun = pRun->getNext();
			}
			pLine = pLine->getNext();
			UT_ASSERT((pLine && pRun) || ((void*)pLine == (void*)pRun));
			UT_ASSERT(!pLine || pLine->getFirstRun() == pRun);
		}

		bool newWord = false;
		bool delim = true;
		for (; i < len; i++)
		{
			if (++iCount > iSelLen) break;

			if (!s_notChar(pSpan[i]))
			{
				wCount.ch_sp++;
				isPara = true;

				switch (pSpan[i])
				{
				case UCS_SPACE:
				case UCS_NBSP:
				case UCS_EN_SPACE:
				case UCS_EM_SPACE:
					break;
					
				default:
					wCount.ch_no++;
				}
			}
			UT_UCSChar followChar, prevChar;
			followChar = (i+1 < len) ? pSpan[i+1] : UCS_UNKPUNK;
			prevChar = i > 0 ? pSpan[i-1] : UCS_UNKPUNK;
			
			newWord = (delim && !UT_isWordDelimiter(pSpan[i], followChar, prevChar));
			
			delim = UT_isWordDelimiter(pSpan[i], followChar, prevChar);
				
			// CJK-FIXME: this can work incorrectly under CJK locales
			// since it can give 'true' for UCS with value >0xff (like
			// quotes, etc).
			if (newWord || 
				XAP_EncodingManager::get_instance()->is_cjk_letter(pSpan[i]))
				wCount.word++;
		}

		if (isPara)
		{
			wCount.para++;
			isPara = false;
		}

		// Get next block
		fl_BlockLayout* pNextBlock = pBL->getNext();
		if (NULL == pNextBlock)
		{
			// If NULL, go to next section
			fl_SectionLayout* pSL = pBL->getSectionLayout()->getNext();
			if (pSL)
				pNextBlock = pSL->getFirstBlock();
		}
		pBL = pNextBlock;
		pLine = NULL;
		pRun = NULL;
		if (pBL)
			pLine = pBL->getFirstLine();
		if (pLine)
			pRun = pLine->getFirstRun();
	}

	return (wCount);
}

void FV_View::setShowPara(bool bShowPara)
{
	if (bShowPara != m_bShowPara)
	{
		m_bShowPara = bShowPara;
		if(getPoint() > 0)
		{
		    draw();
		}
	}
};


/*!
 * This method combines all the things we need to save before doing
 * extensive piecetable manipulations.
 */
void FV_View::SetupSavePieceTableState(void)
{
//
// Fix up the insertion point stuff.
//
	if (isSelectionEmpty())
		_eraseInsertionPoint();
	else
		_clearSelection();
	
	m_pDoc->beginUserAtomicGlob();
	_saveAndNotifyPieceTableChange();
	m_pDoc->disableListUpdates();
	setScreenUpdateOnGeneralUpdate( false); 	
}

/*!
 * This method combines all the things we need to save before doing
 * extensive piecetable manipulations.
 */
void FV_View::RestoreSavedPieceTableState(void)
{
	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have Ended

	m_pDoc->notifyPieceTableChangeEnd();
	m_iPieceTableState = 0;
	m_pDoc->endUserAtomicGlob(); // End the big undo block
	setScreenUpdateOnGeneralUpdate(true);	
	_generalUpdate();
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
}


/*!
 * Remove the Header/Footer type specified by hfType
\params HdrFtrType hfType the type of the Header/Footer to be removed.
\params bool bSkipPTSaves if true don't save the PieceTable stuff
 */
void FV_View::removeThisHdrFtr(HdrFtrType hfType, bool bSkipPTSaves)
{
	if(!bSkipPTSaves)
	{
//
// Fix up the insertion point stuff.
//
		if (isSelectionEmpty())
			_eraseInsertionPoint();
		else
			_clearSelection();
	
		m_pDoc->beginUserAtomicGlob();

		_saveAndNotifyPieceTableChange();
	}
//
// Save current document position.
//
	PT_DocPosition curPoint = getPoint();

	fl_DocSectionLayout * pDSL = (fl_DocSectionLayout *) getCurrentBlock()->getDocSectionLayout();

	if(hfType == FL_HDRFTR_HEADER)
	{
		_removeThisHdrFtr(pDSL->getHeader());
	}
	else if(hfType == FL_HDRFTR_HEADER_EVEN)
	{
		_removeThisHdrFtr(pDSL->getHeaderEven());
	}
	else if(hfType == FL_HDRFTR_HEADER_LAST)
	{
		_removeThisHdrFtr(pDSL->getHeaderLast());
	}
	else if(hfType == FL_HDRFTR_HEADER_FIRST)
	{
		_removeThisHdrFtr(pDSL->getHeaderFirst());
	}
	else if(hfType == FL_HDRFTR_FOOTER)
	{
		_removeThisHdrFtr(pDSL->getFooter());
	}
	else if(hfType == FL_HDRFTR_FOOTER_EVEN)
	{
		_removeThisHdrFtr(pDSL->getFooterEven());
	}
	else if(hfType == FL_HDRFTR_FOOTER_LAST)
	{
		_removeThisHdrFtr(pDSL->getFooterLast());
	}
	else if(hfType == FL_HDRFTR_FOOTER_FIRST)
	{
		_removeThisHdrFtr(pDSL->getFooterFirst());
	}
//
// After erarsing the cursor, Restore to the point before all this mess started.
//
	_eraseInsertionPoint();
	_setPoint(curPoint);

	if(!bSkipPTSaves)
	{
		_generalUpdate();

		// Signal PieceTable Changes have finished
		_restorePieceTableState();
		updateScreen (); // fix 1803, force screen update/redraw

		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
		m_pDoc->endUserAtomicGlob();
	}
}


/*!
 *	 Insert the header/footer. Save the cursor position before we do this and
 *	 restore it to where it was before we did this.
\params HdrFtrType hfType
\params bool bSkipPTSaves if true don't save the PieceTable stuff
 */
void FV_View::createThisHdrFtr(HdrFtrType hfType, bool bSkipPTSaves)
{
	const XML_Char* block_props[] = {
		"text-align", "left",
		NULL, NULL
	};
	PT_DocPosition oldPos = getPoint();
	if(!bSkipPTSaves)
	{
		if(isHdrFtrEdit())
			clearHdrFtrEdit();
//
// Fix up the insertion point stuff.
//
		if (isSelectionEmpty())
			_eraseInsertionPoint();
		else
			_clearSelection();
		m_pDoc->beginUserAtomicGlob(); // Begin the big undo block
		
		// Signal PieceTable Changes have Started
		m_pDoc->notifyPieceTableChangeStart();

		m_pDoc->disableListUpdates();
	}
	insertHeaderFooter(block_props, hfType); // cursor is now in the header/footer
	
	// restore updates and clean up dirty lists
	if(!bSkipPTSaves)
	{
		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();

		// Signal PieceTable Changes have Ended

		m_pDoc->notifyPieceTableChangeEnd();
		m_iPieceTableState = 0;
		m_pDoc->endUserAtomicGlob(); // End the big undo block
	}
//
// Restore old point
//
	_setPoint(oldPos);

	// restore updates and clean up dirty lists
	if(!bSkipPTSaves)
	{
		_generalUpdate();
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
}


/*!
 * OK, This method copies the content from either the header or footer to
 * the HdrFtrType specified here. This is used by the set HdrFtr properties
 * types in the GUI.
\params HdrFtrType hfType
\params bool bSkipPTSaves if true don't save the PieceTable stuff
 */
void FV_View::populateThisHdrFtr(HdrFtrType hfType, bool bSkipPTSaves)
{
//
// Fix up the insertion point stuff.
//
	if(!bSkipPTSaves)
	{
		if (isSelectionEmpty())
			_eraseInsertionPoint();
		else
			_clearSelection();

		m_pDoc->beginUserAtomicGlob(); // Begin the big undo block

		// Signal PieceTable Changes have Started
		m_pDoc->notifyPieceTableChangeStart();

		m_pDoc->disableListUpdates();
	}
//
// Save Old Position
//
	PT_DocPosition oldPos = getPoint();

	fl_DocSectionLayout * pDSL = (fl_DocSectionLayout *) getCurrentBlock()->getSectionLayout();
	fl_HdrFtrSectionLayout * pHdrFtrSrc = NULL;
	fl_HdrFtrSectionLayout * pHdrFtrDest = NULL;
	if(hfType < FL_HDRFTR_FOOTER)
	{
		pHdrFtrSrc = pDSL->getHeader();
	}
	else
	{
		pHdrFtrSrc = pDSL->getFooter();
	}
	if(hfType == FL_HDRFTR_HEADER_FIRST)
	{
		pHdrFtrDest = pDSL->getHeaderFirst();
	}
	else if( hfType == FL_HDRFTR_HEADER_EVEN)
	{
		pHdrFtrDest = pDSL->getHeaderEven();
	}
	else if( hfType == FL_HDRFTR_HEADER_LAST)
	{
		pHdrFtrDest = pDSL->getHeaderLast();
	}
	else if(hfType == FL_HDRFTR_FOOTER_FIRST)
	{
		pHdrFtrDest = pDSL->getFooterFirst();
	}
	else if( hfType == FL_HDRFTR_FOOTER_EVEN)
	{
		pHdrFtrDest = pDSL->getFooterEven();
	}
	else if( hfType == FL_HDRFTR_FOOTER_LAST)
	{
		pHdrFtrDest = pDSL->getFooterLast();
	}

	_populateThisHdrFtr(pHdrFtrSrc, pHdrFtrDest);
	_setPoint(oldPos);

	// restore updates and clean up dirty lists
	if(!bSkipPTSaves)
	{
		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have Ended

		m_pDoc->notifyPieceTableChangeEnd();
		m_iPieceTableState = 0;
		m_pDoc->endUserAtomicGlob(); // End the big undo block
		_generalUpdate();
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
}

/*!
 * Copy a header/footer from a pHdrFtrSrc to an empty pHdrFtrDest.
 * into a new type of header/footer in the same section.
 */ 
void FV_View::_populateThisHdrFtr(fl_HdrFtrSectionLayout * pHdrFtrSrc, fl_HdrFtrSectionLayout * pHdrFtrDest)
{
	PD_DocumentRange dr_source;
	PT_DocPosition iPos1,iPos2;
	iPos1 = m_pDoc->getStruxPosition(pHdrFtrSrc->getFirstBlock()->getStruxDocHandle());
	fl_BlockLayout * pLast = pHdrFtrSrc->getLastBlock();
	iPos2 = pLast->getPosition(false);
//
// This code assumes there is an End of Block run at the end of the Block. 
// Thanks to Jesper, there always is!
//
	while(pLast->getNext() != NULL)
	{
		pLast = pLast->getNext();
	}
	fp_Run * pRun = pLast->getFirstRun();
	while( pRun->getNext() != NULL)
	{
		pRun = pRun->getNext();
	}
	iPos2 += pRun->getBlockOffset();
//
// OK got the doc range for the source. Set it and copy it.
//
	dr_source.set(m_pDoc,iPos1,iPos2);
//
// Copy to and from clipboard to populate the header/Footer
//
	UT_DEBUGMSG(("SEVIOR: Copy to clipboard making header/footer \n"));
	m_pApp->copyToClipboard(&dr_source);
	PT_DocPosition posDest = 0;
	posDest = pHdrFtrDest->getFirstBlock()->getPosition(true);
	PD_DocumentRange dr_dest(m_pDoc,posDest,posDest);
	UT_DEBUGMSG(("SEVIOR: Pasting to clipboard making header/footer \n"));
	m_pApp->pasteFromClipboard(&dr_dest,true,true);
}

/*!
 * Remove all the Headers or footers from the section owning the current Page.
\params bool isHeader remove the header if true, the footer if false.
*/
void FV_View::cmdRemoveHdrFtr( bool isHeader)
{
//
// Branch to Header/Footer sections.
//
	fp_ShadowContainer * pHFCon = NULL;
	fl_HdrFtrShadow * pShadow = NULL;
	fl_HdrFtrSectionLayout * pHdrFtr = NULL;

	if(isHeader)
	{
		fp_Page * pPage = getCurrentPage();
		pHFCon = pPage->getHdrFtrP(FL_HDRFTR_HEADER);
		if(pHFCon == NULL)
		{
			return;
		}
//
// Now see if we are in the header to be removed. If so, jump out.
//
		if (isSelectionEmpty())
			_eraseInsertionPoint();
		else
			_clearSelection();
		if(isHdrFtrEdit())
		{
			clearHdrFtrEdit();
			_setPoint(pPage->getFirstLastPos(true));
		}
	}
	else
	{
		fp_Page * pPage = getCurrentPage();
		pHFCon = pPage->getHdrFtrP(FL_HDRFTR_FOOTER);
		if(pHFCon == NULL)
		{
			return;
		}
//
// Now see if we are in the Footer to be removed. If so, jump out.
//
		if (isSelectionEmpty())
			_eraseInsertionPoint();
		else
			_clearSelection();
		if(isHdrFtrEdit())
		{
			clearHdrFtrEdit();
			_setPoint(pPage->getFirstLastPos(false));
		}
	}
	pShadow = pHFCon->getShadow();
	UT_ASSERT(pShadow);
	if(!pShadow)
		return;
	
	m_pDoc->beginUserAtomicGlob();

	_saveAndNotifyPieceTableChange();
//
// Save current document position.
//
	PT_DocPosition curPoint = getPoint();
//
// Get the hdrftrSectionLayout
// Get it's position.
// Find the last run in the Section and get it's position.
//
// Need code here to remove all the header/footers.
//
	pHdrFtr = pShadow->getHdrFtrSectionLayout();
	fl_DocSectionLayout * pDSL = pHdrFtr->getDocSectionLayout();
//
// Repeat this code 4 times to remove all the DocSection Layouts.
//
	_eraseInsertionPoint();
	if(isHeader)
	{
		_removeThisHdrFtr(pDSL->getHeaderFirst());
		_removeThisHdrFtr(pDSL->getHeaderLast());
		_removeThisHdrFtr(pDSL->getHeaderEven());
		_removeThisHdrFtr(pDSL->getHeader());
	}
	else
	{
		_removeThisHdrFtr(pDSL->getFooterFirst());
		_removeThisHdrFtr(pDSL->getFooterLast());
		_removeThisHdrFtr(pDSL->getFooterEven());
		_removeThisHdrFtr(pDSL->getFooter());
	}
//
// After erarsing the cursor, Restore to the point before all this mess started.
//
	_eraseInsertionPoint();
	_setPoint(curPoint);

	_generalUpdate();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	updateScreen (); // fix 1803, force screen update/redraw

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
	m_pDoc->endUserAtomicGlob();
}

/*!
 * This method removes the HdrFtr pHdrFtr
 */
void FV_View::_removeThisHdrFtr(fl_HdrFtrSectionLayout * pHdrFtr)
{
	if(pHdrFtr == NULL)
	{
		return;
	}
	PT_DocPosition	posBOS = m_pDoc->getStruxPosition(pHdrFtr->getStruxDocHandle());
	fl_BlockLayout * pLast = pHdrFtr->getLastBlock();
	PT_DocPosition posEOS = pLast->getPosition(false);
//
// This code assumes there is an End of Block run at the end of the Block. Thanks
// to Jesper, there always is!
//
	while(pLast->getNext() != NULL)
	{
		pLast = pLast->getNext();
	}
	fp_Run * pRun = pLast->getFirstRun();
	while( pRun->getNext() != NULL)
	{
		pRun = pRun->getNext();
	}
	posEOS += pRun->getBlockOffset();
//
// deleteSpan covers from char to last char + 1;
// 
//	 n.e.x.t.
//	 ^.......^
// pos1    pos2
//
// Location of the start of the text in the header/footer.
//
	PT_DocPosition posBOT = pHdrFtr->getFirstBlock()->getPosition(false);
//
// Ok Now we've got a document range to delete that covers exactly this header.
// Do the Deed!
//
// I'm going to do exactly the reverse of how a header is made since Undo on 
// creating a section works.
//
// First delete all text in header.
//
	if(posEOS > posBOT)
		m_pDoc->deleteSpan(posBOT,posEOS,NULL);
//
// Now delete the header strux.
//
	m_pDoc->deleteSpan(posBOS,posBOS+2,NULL);
//
// This Header Footer is gone!
//
}

/*!
 * This function returns true if there is a header on the current page.
\returns true if is there a header on the current page.
*/
bool FV_View::isHeaderOnPage(void)
{
	fp_Page * pPage = getCurrentPage();
	UT_ASSERT(pPage);
	if(pPage == NULL)
	{
		return false;
	}
	fp_ShadowContainer * pHFCon = NULL;
	pHFCon = pPage->getHdrFtrP(FL_HDRFTR_HEADER);
	if(pHFCon == NULL)
	{
		return false;
	}
	return true;
}

/*!
 * This function returns true if there is a footer on the current page.
\returns true if is there a footer on the current page.
*/
bool FV_View::isFooterOnPage(void)
{
	fp_Page * pPage = getCurrentPage();
	UT_ASSERT(pPage);
	if(pPage == NULL)
	{
		return false;
	}
	fp_ShadowContainer * pHFCon = NULL;
	pHFCon = pPage->getHdrFtrP(FL_HDRFTR_FOOTER);
	if(pHFCon == NULL)
	{
		return false;
	}
	return true;
}

/*!
 *	This method sets a bool variable which tells getEditableBounds we are
 *	editting a header/Footer.
 *	\param	pSectionLayout pointer to the SectionLayout being editted.
*/
void FV_View::setHdrFtrEdit(fl_HdrFtrShadow * pShadow)
{
	m_bEditHdrFtr = true;
	m_pEditShadow = pShadow;
//
// Draw the decorations around the Header/Footer
//
	updateScreen();
}


/*!
 *	This method sets a bool variable which tells getEditableBounds we are
 *	editting a header/Footer.
 *	\param	pSectionLayout pointer to the SectionLayout being editted.
*/
void FV_View::clearHdrFtrEdit(void)
{
	m_bEditHdrFtr = false;
	m_pEditShadow = NULL;
//
// Remove the decorations around the Header/Footer
//
	updateScreen();
}

/*!
 *	 Returns the pointer to the current shadow.
 */
fl_HdrFtrShadow *  FV_View::getEditShadow(void)
{
	return m_pEditShadow;
}

/*!
 *	 Returns true if we're currently editting a HdrFtr section.
 */
bool FV_View::isHdrFtrEdit(void)
{
	return m_bEditHdrFtr;
}

/*!
 * This method saves the current insertion point of the insertion point in the
 * doc. This is needed during undo's of insert Page Number. Maybe elsewhere 
 * later.
 */
void FV_View::rememberCurrentPosition(void)
{
	m_iSavedPosition = getPoint();
	m_bNeedSavedPosition = false;
}

/*!
 * This method returns the saved insertion point of the insertion point in the
 * doc. This is needed during undo's of insert Page Number. Maybe elsewhere 
 * later.
 */

PT_DocPosition FV_View::getSavedPosition(void)
{
	return m_iSavedPosition;
}

/*!
 * This method clears the saved position of the insertion point in the
 * doc. 
 */
void FV_View::clearSavedPosition(void)
{
	m_iSavedPosition = (PT_DocPosition) 0;
	m_bNeedSavedPosition = false;
}

/*!
 * This method tells us we need the old position after an undo because of 
 * header/footer undo's. Might be useful for other stuff later.
 */
bool FV_View::needSavedPosition(void)
{
	return m_bNeedSavedPosition;
}

/*!
 * This method tells the undo to use the remembered position.
 */
void FV_View::markSavedPositionAsNeeded(void)
{
	m_bNeedSavedPosition = true;
}


/*!

   This method is a replacement for getBounds which returns the beginning 
   and end points of the document. It keeps the insertion point out of the 
   header/footer region of the document by not counting the size of the 
   header/footer region in the document length.
   HOWEVER if m_bHdrFtr is true this means we are editting in the Header/Footer
   region so the insertion piont is constrained to be in shadow section UNLESS
   bOverride is true in which case we return the Edittable region again.
   
   We need all this so that we can jump into a Header/Footer region, stay
   there with simple keyboard motions and jump out again with a cursor click
   outside the header/footer.

   \param	isEnd true to get the end of the document. False gets the beginning
   \param	posEnd is the value of the doc position at the beginning and end 
			of the doc
   \param	bOveride if true the EOD is made within the edittable region
   \return	true if succesful
   \todo speed this up by finding clever way to cache the size of the 
		 header/footer region so we can just subtract it off.
*/
bool FV_View::getEditableBounds(bool isEnd, PT_DocPosition &posEOD, bool bOveride)
{
	bool res=true;
	fl_SectionLayout * pSL = NULL;
	fl_BlockLayout * pBL = NULL;
	if(!isEnd && (!m_bEditHdrFtr || bOveride))
	{
		res = m_pDoc->getBounds(isEnd,posEOD);
		return res;
	}
	if(!m_bEditHdrFtr || bOveride)
	{	
		pSL = (fl_SectionLayout *)	m_pLayout->getLastSection();

		if(!pSL)
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			res = m_pDoc->getBounds(isEnd,posEOD);
			return res;
		}
		
		// So if there are no HdrFtr sections, return m_pDoc->getBounds.

		while(pSL->getNext() != NULL && (pSL->getType() == FL_SECTION_DOC ||
										 pSL->getType() == FL_SECTION_ENDNOTE))
		{
			pSL  = pSL->getNext();
		}
		if(pSL->getType() == FL_SECTION_DOC || 
		   pSL->getType() == FL_SECTION_ENDNOTE)
		{
			res = m_pDoc->getBounds(isEnd,posEOD);
			return res;
		}
//
// Now loop through all the HdrFtrSections, find the first in the doc and 
// use that to get the end of editttable region.
//
		PT_DocPosition posFirst = pSL->getFirstBlock()->getPosition(true) - 1;
		PT_DocPosition posNext;
		while(pSL->getNext() != NULL)
		{
			pSL = pSL->getNext();
			posNext = pSL->getFirstBlock()->getPosition(true) - 1;
			if(posNext < posFirst)
				posFirst = posNext;
		}
		posEOD = posFirst;
		return res;
	}
//
// Constrain insertion point to the header/Footer Layout
//
	if(!isEnd)
	{
		posEOD = m_pEditShadow->getFirstBlock()->getPosition();
		return true;
	}
	pBL = m_pEditShadow->getLastBlock();
	posEOD = pBL->getPosition(false);
	fp_Run * pRun = pBL->getFirstRun();
	while( pRun->getNext() != NULL)
	{
		pRun = pRun->getNext();
	}
	posEOD += pRun->getBlockOffset();
	return true;
}
/*!
 * Method start edit header mode. If there is no header one will be inserted.
 * otherwise start editting the header on the current page.
 */
void FV_View::cmdEditHeader(void)
{
	if(isHdrFtrEdit())
		clearHdrFtrEdit();
	fp_Page * pPage = getCurrentPage();
//
// If there is no header, insert it and start to edit it.
//
	fl_HdrFtrShadow * pShadow = NULL;
	fp_ShadowContainer * pHFCon = NULL;
	pHFCon = pPage->getHdrFtrP(FL_HDRFTR_HEADER);
	if(pHFCon == NULL)
	{
		insertHeaderFooter(FL_HDRFTR_HEADER);
		return;
	}
	pShadow = pHFCon->getShadow();
	UT_ASSERT(pShadow);
//
// Put the insertion point at the beginning of the header
//
	fl_BlockLayout * pBL = pShadow->getFirstBlock();
	if (isSelectionEmpty())
		_eraseInsertionPoint();
	else
		_clearSelection();

	_setPoint(pBL->getPosition());
//
// Set Header/footer mode and we're done! Easy :-)
//
	setHdrFtrEdit(pShadow);
	_generalUpdate();
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
}

/*!
 * Method start edit footer mode. If there is no footer one will be inserted.
 * otherwise start editting the footer on the current page.
 */
void FV_View::cmdEditFooter(void)
{
	if(isHdrFtrEdit())
		clearHdrFtrEdit();
	fp_Page * pPage = getCurrentPage();
//
// If there is no header, insert it and start to edit it.
//
	fl_HdrFtrShadow * pShadow = NULL;
	fp_ShadowContainer * pHFCon = NULL;
	pHFCon = pPage->getHdrFtrP(FL_HDRFTR_FOOTER);
	if(pHFCon == NULL)
	{
		insertHeaderFooter(FL_HDRFTR_FOOTER);
		return;
	}
	pShadow = pHFCon->getShadow();
	UT_ASSERT(pShadow);
//
// Put the insertion point at the beginning of the header
//
	fl_BlockLayout * pBL = pShadow->getFirstBlock();
	if (isSelectionEmpty())
		_eraseInsertionPoint();
	else
		_clearSelection();

	_setPoint(pBL->getPosition());
//
// Set Header/footer mode and we're done! Easy :-)
//
	setHdrFtrEdit(pShadow);
	_generalUpdate();
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
}

/*	the problem with using bool to store the PT state is that
	when we make two successive calls to _saveAndNotifyPieceTableChange
	all subsequent calls to _restorePieceTableState will end up in the
	else branch, i.e, the PT will remain in state of change. Thus,
	the new implementation uses int instead of bool and actually keeps
	count of the calls to _save...;
*/
void FV_View::_saveAndNotifyPieceTableChange(void)
{
	//UT_DEBUGMSG(("notifying PieceTableChange start [%d]\n", m_iPieceTableState));
	if(m_pDoc->isPieceTableChanging())
		m_iPieceTableState++;
	m_pDoc->notifyPieceTableChangeStart();
}

void FV_View::_restorePieceTableState(void)
{
	if(m_iPieceTableState > 0)
	{
		//UT_DEBUGMSG(("notifying PieceTableChange (restore/start) [%d]\n", m_iPieceTableState));
		m_pDoc->notifyPieceTableChangeStart();
		m_iPieceTableState--;
	}
	else
	{
		//UT_DEBUGMSG(("notifying PieceTableChange (restore/end) [%d]\n", m_iPieceTableState));
		m_pDoc->notifyPieceTableChangeEnd();
	}
}

void FV_View::insertHeaderFooter(HdrFtrType hfType)
{
	const XML_Char* block_props[] = {
		"text-align", "left",
		NULL, NULL
	};
//
// insert the header/footer and leave the cursor in there. Set us in header/footer
//	edit mode.
//
	if(isHdrFtrEdit())
		clearHdrFtrEdit();
	UT_uint32 iPageNo = getCurrentPageNumber() - 1;

	m_pDoc->beginUserAtomicGlob(); // Begin the big undo block

	// Signal PieceTable Changes have Started
	m_pDoc->notifyPieceTableChangeStart();

	m_pDoc->disableListUpdates();

	insertHeaderFooter(block_props, hfType); // cursor is now in the header/footer
	
	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have Ended

	m_pDoc->notifyPieceTableChangeEnd();
	m_iPieceTableState = 0;
	m_pDoc->endUserAtomicGlob(); // End the big undo block

// Update Layout everywhere. This actually creates the header/footer container

	m_pLayout->updateLayout(); // Update document layout everywhere
//
// Now extract the shadow section from this.
//
	fp_Page * pPage = m_pLayout->getNthPage(iPageNo);
	fl_HdrFtrShadow * pShadow = NULL;
	fp_ShadowContainer * pHFCon = NULL;
	if(hfType >= FL_HDRFTR_FOOTER)
		pHFCon = pPage->getHdrFtrP(FL_HDRFTR_FOOTER);
	else if (hfType < FL_HDRFTR_FOOTER)
		pHFCon = pPage->getHdrFtrP(FL_HDRFTR_HEADER);
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	UT_ASSERT(pHFCon);
	pShadow = pHFCon->getShadow();
	UT_ASSERT(pShadow);
//
// Set Header/footer mode and we're done! Easy :-)
//
	setHdrFtrEdit(pShadow);

	_generalUpdate();
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
}

bool FV_View::insertHeaderFooter(const XML_Char ** props, HdrFtrType hfType, fl_DocSectionLayout * pDSL)
{
//	UT_ASSERT(hfType == FL_HDRFTR_HEADER || hfType == FL_HDRFTR_FOOTER);

	/*
	  This inserts a header/footer at the end of the document,
	  and leaves the insertion point there.
	  This provides NO undo stuff.	Do it yourself.
	*/

	UT_String szString;
	if(hfType == FL_HDRFTR_HEADER)
	{
		szString = "header";
	}
	else if( hfType == FL_HDRFTR_HEADER_EVEN)
	{
		szString = "header-even";
	}
	else if( hfType == FL_HDRFTR_HEADER_FIRST)
	{
		szString = "header-first";
	}
	else if( hfType == FL_HDRFTR_HEADER_LAST)
	{
		szString = "header-last";
	}
	else if(hfType == FL_HDRFTR_FOOTER)
	{
		szString = "footer";
	}
	else if( hfType == FL_HDRFTR_FOOTER_EVEN)
	{
		szString = "footer-even";
	}
	else if( hfType == FL_HDRFTR_FOOTER_FIRST)
	{
		szString = "footer-first";
	}
	else if( hfType == FL_HDRFTR_FOOTER_LAST)
	{
		szString = "footer-last";
	}

	static XML_Char sid[15];
	UT_uint32 id = 0;
	while(id < AUTO_LIST_RESERVED)
		id = UT_rand();
	sprintf(sid, "%i", id);

	const XML_Char* sec_attributes1[] = {
		"type", szString.c_str(),
		"id",sid,"listid","0","parentid","0",
		NULL, NULL
	};

	const XML_Char* sec_attributes2[] = {
		szString.c_str(), sid,
		NULL, NULL
	};


	const XML_Char* block_props[] = {
		"text-align", "center",
		NULL, NULL
	};

	if(!props)
		props = block_props; // use the defaults

	if (isSelectionEmpty())
	{
		_eraseInsertionPoint();
	}
//
// Find the section that owns this page. 
//
	fl_DocSectionLayout * pDocL = pDSL;
	if(pDocL == NULL)
	{
		fp_Page* pCurrentPage = getCurrentPage();
		pDocL = pCurrentPage->getOwningSection();
	}
//
// Now find the position of this section
//
	fl_BlockLayout * pBL = pDocL->getFirstBlock();
	PT_DocPosition posSec = pBL->getPosition();

	// change the section to point to the footer which doesn't exist yet.

	m_pDoc->changeStruxFmt(PTC_AddFmt, posSec, posSec, sec_attributes2, NULL, PTX_Section);

 
	moveInsPtTo(FV_DOCPOS_EOD); // Move to the end, where we will create the page numbers

	// Now create the footer section
	// First Do a block break to finish the last section.

	UT_uint32 iPoint = getPoint();
	m_pDoc->insertStrux(getPoint(), PTX_Block);
	//
	// If there is a list item here remove it!
	//
	fl_BlockLayout* pBlock = _findBlockAtPosition(getPoint());
	PL_StruxDocHandle sdh = pBlock->getStruxDocHandle();
	if(pBlock && pBlock->isListItem())
	{
		while(pBlock->isListItem())
		{
			m_pDoc->StopList(sdh);
		}	  
	} 
//
// Next set the style to Normal Clean so weird properties at the end of the 
// doc aren't
// inherited into the header.
//
	setStyle("Normal",true);

	// Now Insert the footer section.
	// Doing things this way will grab the previously inserted block
	// and put into the footer section.

	m_pDoc->insertStrux(iPoint, PTX_SectionHdrFtr);
	
//
// Give the Footer section the properties it needs to attach itself to the
// correct DocSectionLayout.
//
	m_iInsPoint++;
	m_pDoc->changeStruxFmt(PTC_AddFmt, getPoint(), getPoint(), sec_attributes1, NULL, PTX_SectionHdrFtr);

	// Change the formatting of the new footer appropriately 
	//(currently just center it)

	m_pDoc->changeStruxFmt(PTC_AddFmt, getPoint(), getPoint(), NULL, props, PTX_Block);

// OK it's in!

	return true;
}

bool FV_View::insertEndnote()
{
	fl_DocSectionLayout * pDSL = getCurrentPage()->getOwningSection();

	// add field for endnote reference
	// first, make up an id for this endnote.
	static XML_Char enpid[15];
	UT_uint32 pid = 0;
	while (pid < AUTO_LIST_RESERVED)
		pid = UT_rand();
	sprintf(enpid, "%i", pid);

	// You know, I'm not sure if endnote-id should be a prop or attr on
	// the endnote_ref field.  I'm thinking it should be a prop, but
	// currently it's an attr; that's easier to add.  Is that bogus? -PL
	const XML_Char* attrs[] = {
		"endnote-id", enpid,
		NULL, NULL
	};

	//get ready to apply Endonote Reference style
	/*	NOTES ON THE ENDNOTE STYLE MECHANISM
		I have tried to ways. (1) apply the character style at insertion
		point and insert the Endnote reference; this does not work, the
		field does not honour the style. (2) Insert the endnote, in the
		process remembering the postions where the styles should be applied, then
		when the insertion is finished, apply the styles together. This
		works, but the endonote fields fail to adjust their sizes to the
		new formating. Also, there are some redrawing problems.
	*/
	
	PT_DocPosition ErefStart = getPoint();
	PT_DocPosition ErefEnd = ErefStart + 1;
	PT_DocPosition EanchStart;
	PT_DocPosition EanchEnd;
	PT_DocPosition EbodyEnd;
	
	//const XML_Char *cur_style;
	//getStyle(&cur_style);

	m_pDoc->beginUserAtomicGlob();		   
	
	if (cmdInsertField("endnote_ref", attrs)==false)
		return false;

	//UT_DEBUGMSG(("reseting style [%s] after endnote reference, start %d, end %d, point %d\n", cur_style, ErefStart, ErefEnd, getPoint()));
	//setStyleAtPos(cur_style, ErefEnd, ErefEnd,true);
			
	// Current bogosity: c type="endnote_ref".	What's up with that?
	// Also endnote-id should not follow to next paras.
	
	fl_BlockLayout * pBL;
	fl_DocSectionLayout * pEndnoteSL;

	if (pDSL->getEndnote() == NULL)
	{
		if (!insertEndnoteSection(enpid))
			return false;
		pEndnoteSL = pDSL->getEndnote();
	}
	else
	{
		// warp to endnote section.
		pEndnoteSL = pDSL->getEndnote();

		_eraseInsertionPoint();

		// We know the position of the reference mark, so we will search from
		// the start up to this position for all endnote_ref runs and count
		// them. From this we will work out our position and insert the block
		// at the appropriate place, this will automatically ensure that
		// the field resolves to correct number
		
		fl_SectionLayout * pSL = m_pLayout->getFirstSection();
		UT_sint32 enoteCount = 0;
		fp_Run * pRun;
		bool bFinished = false; 	
		
		while (pSL)
		{
			pBL = pSL->getFirstBlock();
			
			while(pBL)
			{
				pRun = pBL->getFirstRun();
				
				while(pRun) 		
				{
					if( (pBL->getPosition(false)+ pRun->getBlockOffset()) >= ErefStart)
					{
						bFinished = true;
						break;
					}
					
					if(pRun->getType() == FPRUN_FIELD)
					{
						fp_FieldRun * pF = static_cast<fp_FieldRun *>(pRun);
#ifdef DEBUG
						if(pF->getFieldType() == FPFIELD_endnote_ref)
#endif
							enoteCount++;
					}
					
					pRun = pRun->getNext();
				}
				
				if(bFinished)
					break;
					
				pBL = pBL->getNext();
			}
			
			if(bFinished)
				break;
				
			pSL = pSL->getNext();
		}
		
	
		pBL = pEndnoteSL->getFirstBlock();
	
		// now we will find the block just after us, move the start of it
		// and insert a new block
		UT_DEBUGMSG(("fv_View::insertEndnote: ErefStart %d, enoteCount %d\n", ErefStart, enoteCount));

		// TODO HACK until we stop propagating endnote-ids.
		XML_Char * previd = NULL;
		
		while(pBL)
		{
			// skip any non-endnote blocks
			UT_DEBUGMSG(("enoteCount %d\n", enoteCount));
			
			UT_ASSERT(pBL);
			const PP_AttrProp *pp;
			bool bRes = pBL->getAttrProp(&pp);
			if (!bRes)
				break;
				
			const XML_Char * someid;
			pp->getAttribute("endnote-id", someid);
			
#if 0
			//do not need this anymore
			while(!someid || UT_strcmp(someid, "") == 0)
			{
				pBL = pBL->getNext();
				bRes = pBL->getAttrProp(&pp);
				if (!bRes)
					break;
				pp->getAttribute("endnote-id", someid);
			}
#endif
			if (!previd || (previd && someid && UT_strcmp(someid, previd) != 0))
			{
				enoteCount--;
				if (previd != NULL)
					free(previd);

				previd = UT_strdup(someid);
			}
						
			if(enoteCount < 0)
				break;
			
			pBL = pBL->getNext();
		}

		if (previd != NULL)
			free(previd);
				
		// now if we do not have a block, we are inserting the last endnote
		// so we will just get the last block in the section and move to the end
		// if we do have a block, we move to the start of it
		PT_DocPosition dp;
		
		if(!pBL)
		{
			UT_DEBUGMSG(("no block\n"));
			pBL = pEndnoteSL->getLastBlock();
#ifdef DEBUG
			const XML_Char * someid;
			const PP_AttrProp *pp;
			bool bRes = pBL->getAttrProp(&pp);
			if (bRes)
			{
				pp->getAttribute("endnote-id", someid);
				UT_DEBUGMSG(("	   enpid [%s], someid [%s]\n",enpid,someid));		
			}
#endif
			dp = pBL->getPosition();
			_setPoint(dp, false);
			moveInsPtTo(FV_DOCPOS_EOB);
			dp = getPoint();
		}
		else
		{
			// we want the position of the actual start of the block, not
			// of the runlist
			UT_DEBUGMSG(("found block\n"));
#ifdef DEBUG
			const XML_Char * someid;
			const PP_AttrProp *pp;
			bool bRes = pBL->getAttrProp(&pp);
			if (bRes)
			{
				pp->getAttribute("endnote-id", someid);
				UT_DEBUGMSG(("	   enpid [%s], someid [%s]\n",enpid,someid));		
			}
#endif
			dp = pBL->getPosition(true);
			_setPoint(dp,false);
		}
				
		
		// add new block.
		const XML_Char* block_attrs[] = {
			"endnote-id", enpid,
			NULL, NULL
		};
	
	// do not want any hardcoded block properties, let them default
#if 0
		const XML_Char* block_props[] = {
			"text-align", "left",
			NULL, NULL
		};
#else
		const XML_Char**	block_props = NULL;
#endif

		m_pDoc->insertStrux(dp, PTX_Block);
		// the dp+1 is needed because the postion on boundary between two
		// blocks is considered as belonging to the PREVIOUS block,
		// i.e., dp only reformats the wrong block
		m_pDoc->changeStruxFmt(PTC_AddFmt, dp+1, dp+1, block_attrs, block_props, PTX_Block);

		_drawInsertionPoint();
	}
	
	// add endnote anchor
	//get ready to apply Endnote Reference style
	EanchStart = getPoint();

	// if the block after which were inserted was not the last block
	// we have to adjust the postion, because we are now siting at the
	// start of the next block instead of our own
	UT_DEBUGMSG(("fv_View::insertEndnote: EanchStart %d\n",EanchStart));
	
	if (cmdInsertField("endnote_anchor", attrs)==false)
		return false;
	EanchEnd = getPoint();
	
	//insert a space after the anchor
	UT_UCSChar space = UCS_SPACE;
	m_pDoc->insertSpan(EanchEnd, &space, 1);

	EbodyEnd = getPoint();
	
	xxx_UT_DEBUGMSG(("applying [Endnote Reference] style to endnote, start %d, end %d, point %d\n", ErefStart, ErefEnd, getPoint()));
	setStyleAtPos("Endnote Reference", ErefStart, ErefEnd,true);

	xxx_UT_DEBUGMSG(("applying [Endnote Text] style to endnote body, start %d, end %d, point %d\n", EbodyEnd, EbodyEnd, getPoint()));
	setStyleAtPos("Endnote Text", EbodyEnd, EbodyEnd,true);
	
	xxx_UT_DEBUGMSG(("applying [Endnote Reference] style to anchor, start %d, end %d, point %d\n", EanchStart, EanchEnd, getPoint()));
	setStyleAtPos("Endnote Reference", EanchStart, EanchEnd,true);

	/*	some magic to make the endnote reference and anchor recalculate
		its widths
	*/
	pBL = _findBlockAtPosition(ErefStart);
	UT_ASSERT(pBL != 0);
	UT_sint32 x, y, x2, y2, height;
	bool bDirection;

	fp_Run* pRun = pBL->findPointCoords(ErefStart, false, x, y, x2, y2, height, bDirection);
	UT_ASSERT(pRun != 0);
	bool bWidthChange = pRun->recalcWidth();
	xxx_UT_DEBUGMSG(("run type %d, width change %d\n", pRun->getType(),bWidthChange));
	if(bWidthChange) pBL->setNeedsReformat();


	pBL = _findBlockAtPosition(EanchStart);
	UT_ASSERT(pBL != 0);
	bWidthChange = pBL->getFirstLine()->getFirstRun()->getNext()->recalcWidth();
	xxx_UT_DEBUGMSG(("run type %d, width change %d\n", pBL->getFirstLine()->getFirstRun()->getNext()->getType(),bWidthChange));
	if(bWidthChange) pBL->setNeedsReformat();
	
	m_pDoc->endUserAtomicGlob();
	_generalUpdate();
	return true;
}

bool FV_View::insertEndnoteSection(const XML_Char * enpid)
{
	const XML_Char* block_attrs[] = {
		"endnote-id", enpid,
		NULL, NULL
	};

	// do not want any hardcode block properties, let them default
#if 0
	const XML_Char* block_props[] = {
		"text-align", "left",
		NULL, NULL
	};
#else
	const XML_Char**	block_props = NULL;
#endif

	m_pDoc->beginUserAtomicGlob(); // Begin the big undo block

	// Signal PieceTable Changes have Started
	//UT_DEBUGMSG(("insertEndnoteSection: about to save and notify\n"));
	_saveAndNotifyPieceTableChange();
	m_pDoc->disableListUpdates();

	if (!insertEndnoteSection(block_props, block_attrs)) // cursor is now in the endnotes
		return false;
	
	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();


	m_pDoc->endUserAtomicGlob(); // End the big undo block

	_generalUpdate();

	// Signal PieceTable Changes have Ended
	//UT_DEBUGMSG(("insertEndnoteSection: about to restore\n"));
	_restorePieceTableState();
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	return true;
}

bool FV_View::insertEndnoteSection(const XML_Char ** blkprops, const XML_Char ** blkattrs)
{
	/*
	  This inserts an endnote at the end of the document,
	  and leaves the insertion point there.
	  This provides NO undo stuff.	Do it yourself.
	*/

	static XML_Char sid[15];
	UT_uint32 id = 0;
	while(id < AUTO_LIST_RESERVED)
		id = UT_rand();
	sprintf(sid, "%i", id);

	const XML_Char* sec_attributes1[] = {
		"id",sid,"listid","0","parentid","0","type","endnote",
		NULL, NULL
	};

	const XML_Char* sec_attributes2[] = {
		"endnote", sid,
		NULL, NULL
	};

	// do not want any hardcode block properties, let them default
#if 0
	const XML_Char* block_props[] = {
		"text-align", "left",
		NULL, NULL
	};
	
	if(!blkprops)
		blkprops = block_props; // use the defaults
#endif


	if (isSelectionEmpty())
	{
		_eraseInsertionPoint();
	}

//
// Find the section that owns this page.
//
	fp_Page* pCurrentPage = getCurrentPage();
	fl_DocSectionLayout * pDocL = pCurrentPage->getOwningSection();
//
// Now find the position of this section
//
	fl_BlockLayout * pBL = pDocL->getFirstBlock();
	PT_DocPosition posSec = pBL->getPosition();

	// change the section to point to the endnote which doesn't exist yet.

	m_pDoc->changeStruxFmt(PTC_AddFmt, posSec, posSec, sec_attributes2, NULL, PTX_Section);

	// Move to the end, where we will create the endnotes
	moveInsPtTo(FV_DOCPOS_EOD); 

	// Now create the endnotes section
	// First, do a block break to finish the last section.

	UT_uint32 iPoint = getPoint();
	m_pDoc->insertStrux(getPoint(), PTX_Block);

	// If there is a list item here remove it!

	fl_BlockLayout* pBlock = _findBlockAtPosition(getPoint());
	PL_StruxDocHandle sdh = pBlock->getStruxDocHandle();
	if(pBlock && pBlock->isListItem())
	{
		while(pBlock->isListItem())
		{
			m_pDoc->StopList(sdh);
		}	  
	} 

	// Next set the style to Normal so weird properties at the 
	// end of the doc aren't inherited into the header.

	setStyle("Normal",true);

	// Now Insert the endnotes section.
	// Doing things this way will grab the newly inserted block
	// and put into the endnotes section.

	m_pDoc->insertStrux(iPoint, PTX_SectionEndnote);

	// Give the endnotes section the properties it needs to attach 
	// itself to the correct DocSectionLayout.
	m_pDoc->changeStruxFmt(PTC_AddFmt, getPoint(), getPoint(), sec_attributes1, NULL, PTX_SectionEndnote);

	// Change the formatting of the new endnote appropriately 
	m_pDoc->changeStruxFmt(PTC_AddFmt, getPoint(), getPoint(), blkattrs, blkprops, PTX_Block);

	m_pDoc->signalListeners(PD_SIGNAL_REFORMAT_LAYOUT);
	return true;
}


bool FV_View::insertPageNum(const XML_Char ** props, HdrFtrType hfType)
{
	
	/*
	  This code implements some hardcoded hacks to insert a page number.
	  It allows you to set the properties, but nothing else.  Use that
	  to center, etc.

	  Warning: this code assumes that _insertFooter() leaves the insertion
	  point in a place where it can write the page_num field.
	*/

	const XML_Char* f_attributes[] = {
		"type", "page_number",
		NULL, NULL
	};

	m_pDoc->beginUserAtomicGlob(); // Begin the big undo block

	// Signal PieceTable Changes have Started
	_saveAndNotifyPieceTableChange();
	m_pDoc->disableListUpdates();

	_eraseInsertionPoint();

	UT_uint32 oldPos = getPoint();	// This ends up being redundant, but it's neccessary
	bool bResult = insertHeaderFooter(props, hfType);

	//
	// after this call the insertion point is at the position where stuff
	// can be inserted into the header/footer
	//
	if(!bResult) 
		return false;
	
	// Insert the page_number field
	bResult = m_pDoc->insertObject(getPoint(), PTO_Field, f_attributes, NULL);

	moveInsPtTo(oldPos);	// Get back to where you once belonged.

	m_pLayout->updateLayout(); // Update document layout everywhere
	m_pDoc->endUserAtomicGlob(); // End the big undo block
	
	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	_generalUpdate();

	// Signal PieceTable Changes have Ended
	_restorePieceTableState();
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
	return bResult;
}

const fp_PageSize & FV_View::getPageSize(void) const
{
	return m_pLayout->getFirstPage()->getPageSize();
}
	
		
UT_uint32 FV_View::calculateZoomPercentForPageWidth()
{

	const fp_PageSize pageSize = getPageSize();
	double pageWidth = pageSize.Width(DIM_IN);
	
	// Set graphics zoom to 100 so we can get the display resolution.
	GR_Graphics *pG = getGraphics();
	UT_uint32 temp_zoom = pG->getZoomPercentage();
	pG->setZoomPercentage(100);
	UT_uint32 resolution = pG->getResolution();
	pG->setZoomPercentage(temp_zoom);

	// Verify scale as a positive non-zero number else return old zoom
	if ( ( getWindowWidth() - 2 * getPageViewLeftMargin() ) <= 0 )
		return temp_zoom;

	double scale = (double)(getWindowWidth() - 2 * getPageViewLeftMargin()) / 
		(pageWidth * (double)resolution);
	return (UT_uint32)(scale * 100.0);
}

UT_uint32 FV_View::calculateZoomPercentForPageHeight()
{

	const fp_PageSize pageSize = getPageSize();
	double pageHeight = pageSize.Height(DIM_IN);
	
	// Set graphics zoom to 100 so we can get the display resolution.
	GR_Graphics *pG = getGraphics();
	UT_uint32 temp_zoom = pG->getZoomPercentage();
	pG->setZoomPercentage(100);
	UT_uint32 resolution = pG->getResolution();
	pG->setZoomPercentage(temp_zoom);

	// Verify scale as a positive non-zero number else return old zoom
	if ( ( getWindowHeight() - 2 * getPageViewTopMargin() ) <= 0 )
		return temp_zoom;

	double scale = (double)(getWindowHeight() - 2 * getPageViewTopMargin()) /
		(pageHeight * (double)resolution);
	return (UT_uint32)(scale * 100.0);
}

UT_uint32 FV_View::calculateZoomPercentForWholePage()
{
	return MyMin(	calculateZoomPercentForPageWidth(),
					calculateZoomPercentForPageHeight());
}
