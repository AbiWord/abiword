/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#include "fl_SectionLayout.h"
#include "fl_AutoNum.h"
#include "fp_Page.h"
#include "fp_Column.h"
#include "fp_Line.h"
#include "fp_Run.h"
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
#include "xap_Clipboard.h"
#include "ap_TopRuler.h"
#include "ap_LeftRuler.h"
#include "ap_Prefs.h"
#include "fd_Field.h"
#include "sp_spell.h"

#include "xap_EncodingManager.h"

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
		m_bShowPara(false)
{
//	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

	// initialize prefs cache
	pApp->getPrefsValueBool(AP_PREF_KEY_CursorBlink, &m_bCursorBlink);

	// initialize prefs listener
	pApp->getPrefs()->addListener( _prefsListener, this );

#ifdef BIDI_ENABLED
	pApp->getPrefsValueBool(AP_PREF_KEY_DefaultDirectionRtl, &m_bDefaultDirectionRtl);
		
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
		const XML_Char bidi_dir_name[] = "dir";
		const XML_Char bidi_dir_value[] = "rtl";
		const XML_Char bidi_domdir_name[] = "dom-dir";
		const XML_Char bidi_align_name[] = "text-align";
		const XML_Char bidi_align_value[] = "right";

		const XML_Char * bidi_props[7]= {bidi_dir_name, bidi_dir_value, bidi_domdir_name, bidi_dir_value, bidi_align_name, bidi_align_value,0};

		m_pDoc->setStyleProperties((const XML_Char*)"Normal", (const XML_Char**)bidi_props);
		PP_resetInitialBiDiValues("rtl");
	}
#else
	if(!m_bDefaultDirectionRtl)
	{
		const XML_Char bidi_dir_name[] = "dir";
		const XML_Char bidi_dir_value[] = "ltr";
		const XML_Char bidi_domdir_name[] = "dom-dir";
		const XML_Char bidi_align_name[] = "text-align";
		const XML_Char bidi_align_value[] = "left";

		const XML_Char * bidi_props[7]= {bidi_dir_name, bidi_dir_value, bidi_domdir_name, bidi_dir_value, bidi_align_name, bidi_align_value,0};
		
		m_pDoc->setStyleProperties((const XML_Char*)"Normal", (const XML_Char**)bidi_props);
		PP_resetInitialBiDiValues("ltr");
	}
#endif
#endif
	// initialize change cache
	m_chg.bUndo = false;
	m_chg.bRedo = false;
	m_chg.bDirty = false;
	m_chg.bSelection = false;
	m_chg.iColumn = 0;                       // current column number
	m_chg.propsChar = NULL;
	m_chg.propsBlock = NULL;
	m_chg.propsSection = NULL;

	pLayout->setView(this);

	pLayout->formatAll();
	//Test_Dump();
	moveInsPtTo(FV_DOCPOS_BOD);
	m_iSelectionAnchor = getPoint();
	_resetSelection();
	_clearOldPoint();
	_fixInsertionPointCoords();
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

// first character of selection gets capitalized.
// TODO: make me respect sentence boundaries
static void _toggleSentence (const UT_UCSChar * src, 
			     UT_UCSChar * dest, UT_uint32 len)
{
    dest[0] = UT_UCS_toupper (src[0]);

    for (UT_uint32 i = 1; i < len; i++)
      {
	dest[i] = src[i];
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
static void _toggleTitle (const UT_UCSChar * src, 
			  UT_UCSChar * dest, UT_uint32 len)
{
  bool wasSpace = false;

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

// all gets set to its opposite
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

void FV_View::toggleCase (ToggleCase c)
{
  // 1. get selection
  // 2. toggle case
  // 3. clear and replace

  // TODO: we currently lose *all* formatting information. Fix this.

  if (isSelectionEmpty())
    return;

  UT_UCSChar * cur, * replace;

  cur = getSelectionText();

  if (!cur)
    return;

  UT_uint32 replace_len = UT_UCS_strlen (cur);
  replace = new UT_UCSChar [replace_len + 1];

  UT_UCS_strcpy(replace,cur);

  switch (c)
    {

    case CASE_SENTENCE: _toggleSentence (cur, replace, replace_len);
      break;

    case CASE_LOWER: _toggleLower (cur, replace, replace_len);
      break;

    case CASE_UPPER: _toggleUpper (cur, replace, replace_len);
      break;

    case CASE_TITLE: _toggleTitle (cur, replace, replace_len);
      break;

    case CASE_TOGGLE: _toggleToggle (cur, replace, replace_len);
      break;

    default:
      UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
    }

  m_pDoc->notifyPieceTableChangeStart();
  m_pDoc->beginUserAtomicGlob();

  cmdCharInsert (replace, replace_len, true);

  m_pDoc->endUserAtomicGlob();
  _generalUpdate();
  m_pDoc->notifyPieceTableChangeEnd();

  FREEP(cur);
  delete[] replace;
}

void FV_View::setPaperColor(UT_RGBColor & rgb)
{

	char clr[8];
	sprintf(clr, "#%02x%02x%02x", rgb.m_red, rgb.m_grn, rgb.m_blu);

	UT_DEBUGMSG(("DOM: color is: %s\n", clr));

	const XML_Char * props [3];
	props [0] = "background-color";
	props [1] = clr;
	props [2] = 0;

	setSectionFormat(props);
	_eraseInsertionPoint();
	// update the screen
	_draw(0,0,m_iWindowWidth,m_iWindowHeight,false,false);
	_fixInsertionPointCoords();
	_drawInsertionPoint();
}

void FV_View::focusChange(AV_Focus focus)
{
	m_focus=focus;
	switch(focus)
	{
	case AV_FOCUS_HERE:
		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
		m_pApp->rememberFocussedFrame( m_pParentData);
		break;
	case AV_FOCUS_NEARBY:
		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
		break;
	case AV_FOCUS_MODELESS:
		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
		break;
	case AV_FOCUS_NONE:
		if (isSelectionEmpty())
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
		fl_BlockLayout * pBlock = pRun->getBlock();
		if(pBlock->getSectionLayout()->getType() ==  FL_SECTION_HDRFTR)
		{
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
	}
	
	// base class does the rest
	return AV_View::notifyListeners(mask);
}

void FV_View::_swapSelectionOrientation(void)
{
	// reverse the direction of the current selection
	// without changing the screen.

	UT_ASSERT(!isSelectionEmpty());
	_fixInsertionPointCoords();
	PT_DocPosition curPos = getPoint();
	UT_ASSERT(curPos != m_iSelectionAnchor);
	_setPoint(m_iSelectionAnchor);
	m_iSelectionAnchor = curPos;
}
	
void FV_View::_moveToSelectionEnd(bool bForward)
{
	// move to the requested end of the current selection.
	// NOTE: this must clear the selection.
	// NOTE: we do not draw the insertion point
	//		after clearing the selection.

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
        
        bool bInWord = !UT_isWordDelimiter(pSpan[bKeepLooking ? offset-1 : offset], UCS_UNKPUNK);
        
        for (offset--; offset > 0; offset--)
        {
            if (UT_isWordDelimiter(pSpan[offset], UCS_UNKPUNK))
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
        
        bool bBetween = UT_isWordDelimiter(pSpan[offset], UCS_UNKPUNK);
        
        // Needed so ctrl-right arrow will work
        // This is the code that was causing bug 10
        // There is still some weird behavior that should be investigated
        
        for (; offset < pgb.getLength(); offset++)
        {
            UT_UCSChar followChar;
            followChar = ((offset + 1) < pgb.getLength()) ? pSpan[offset+1] : UCS_UNKPUNK;
            if (!UT_isWordDelimiter(pSpan[offset], followChar))
                break;
        }
        
        for (; offset < pgb.getLength(); offset++)
        {
            UT_UCSChar followChar;
            followChar = ((offset + 1) < pgb.getLength()) ? pSpan[offset+1] : UCS_UNKPUNK;
            if (!UT_isWordDelimiter(pSpan[offset], followChar))
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

        bool bBetween = UT_isWordDelimiter(pSpan[offset], UCS_UNKPUNK);
			
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
            UT_UCSChar followChar;
            followChar = ((offset + 1) < pgb.getLength()) ? pSpan[offset+1] : UCS_UNKPUNK;
            if (UT_isWordDelimiter(pSpan[offset], followChar))
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

void FV_View::moveInsPtTo(FV_DocPos dp)
{
	if (!isSelectionEmpty())
		_clearSelection();
	else
		_eraseInsertionPoint();
	
	PT_DocPosition iPos = _getDocPos(dp);

	if (iPos != getPoint())
	{
		bool bPointIsValid = (getPoint() >= _getDocPos(FV_DOCPOS_BOD));
		if (bPointIsValid)
			_clearIfAtFmtMark(getPoint());
	}

	_setPoint(iPos, (dp == FV_DOCPOS_EOL));

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	notifyListeners(AV_CHG_MOTION);
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
	_ensureThatInsertionPointIsOnScreen();
	
}



void FV_View::cmdCharMotion(bool bForward, UT_uint32 count)
{
	if (!isSelectionEmpty())
	{
		_moveToSelectionEnd(bForward);
		// Note: _moveToSelectionEnd() clears the selection
		//		but does not redraw the insertion point.
		_drawInsertionPoint();
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
	if(pBL->isHdrFtr())
	{
		fl_HdrFtrSectionLayout * pSSL = (fl_HdrFtrSectionLayout *) pBL->getSectionLayout();
		pBL = pSSL->getFirstShadow()->findMatchingBlock(pBL);
		UT_DEBUGMSG(("<<<<SEVIOR>>>: getfirstshadow in view \n"));
	}
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
	m_pDoc->notifyPieceTableChangeStart();

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
//				if(IS_NUMBERED_LIST_TYPE(curType) == true)
				{
					UT_uint32 curlevel = pBlock->getLevel();
					UT_uint32 currID = pBlock->getAutoNum()->getID();
					curlevel++;
					fl_AutoNum * pAuto = pBlock->getAutoNum();
					const XML_Char * pszAlign = pBlock->getProperty("margin-left",true);
					const XML_Char * pszIndent = pBlock->getProperty("text-indent",true);
					float fAlign = (float)atof(pszAlign);
					float fIndent = (float)atof(pszIndent);
					fAlign += (float) LIST_DEFAULT_INDENT;
					pBlock->StartList(curType,pAuto->getStartValue32(),pAuto->getDelim(),pAuto->getDecimal(),"NULL",fAlign,fIndent, currID,curlevel);
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
	m_pDoc->notifyPieceTableChangeEnd();

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	return bResult;
}


void FV_View::insertSectionBreak(BreakSectionType type)
{
	// if Type = 0 "continuous" section break
	// if Type = 1 "next page" section break
	// if Type = 2 "even page" section break
	// if Type = 3 "odd page" section break

	// Signal PieceTable Change
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
		cmdCharInsert(&c,1);
		_insertSectionBreak();
		m_pDoc->endUserAtomicGlob();
		break;
	case BreakSectionEvenPage:
		m_pDoc->beginUserAtomicGlob();
		cmdCharInsert(&c,1);
		iPageNum = getCurrentPageNumber();
		if( (iPageNum & 1) == 1)
		{
			cmdCharInsert(&c,1);
			_insertSectionBreak();
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
			cmdCharInsert(&c,1);
			_insertSectionBreak();
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

	// Signal piceTable is stable again
	m_pDoc->notifyPieceTableChangeEnd();

}

void FV_View::insertSectionBreak(void)
{
	m_pDoc->beginUserAtomicGlob();

	_insertSectionBreak();
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

	// insert a new paragraph with the same attributes/properties
	// as the previous (or none if the first paragraph in the section).
	// before inserting a section break, we insert a block break
	UT_uint32 iPoint = getPoint();
	
	m_pDoc->insertStrux(iPoint, PTX_Block);
	m_pDoc->insertStrux(iPoint, PTX_Section);

	_generalUpdate();

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
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
		   (runtype == FPRUN_FMTMARK))
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
	m_pDoc->notifyPieceTableChangeStart();

	UT_Vector vBlock;
	getBlocksInSelection( &vBlock);
	UT_uint32 i;
	m_pDoc->disableListUpdates();

	m_pDoc->beginUserAtomicGlob();

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
			if(pBlock->isListItem()== NULL && pPrev != NULL && pPrev->isListItem()== true && pPrev->getAutoNum()->getType() == listType)
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

	_generalUpdate();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal piceTable is stable again
	m_pDoc->notifyPieceTableChangeEnd();

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
		startpos =  m_iSelectionAnchor;
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
	m_pDoc->notifyPieceTableChangeStart();

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
	m_pDoc->endUserAtomicGlob();

	_generalUpdate();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal piceTable is stable again
	m_pDoc->notifyPieceTableChangeEnd();

	_generalUpdate();
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	m_pLayout->considerPendingSmartQuoteCandidate();
	_checkPendingWordForSpell();
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
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
}

bool FV_View::appendStyle(const XML_Char ** style)
{
	return m_pDoc->appendStyle(style);
}

bool FV_View::setStyle(const XML_Char * style, bool bDontGeneralUpdate)
{
	bool bRet;

	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;

	// Signal PieceTable Change
	m_pDoc->notifyPieceTableChangeStart();

	// Turn off list updates
	m_pDoc->disableListUpdates();

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

	// lookup the current style
	PD_Style * pStyle = NULL;
	m_pDoc->getStyle((char*)style, &pStyle);
	if (!pStyle)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}

	bool bCharStyle = pStyle->isCharStyle();

	const XML_Char * attribs[] = { PT_STYLE_ATTRIBUTE_NAME, 0, 0 };
	attribs[1] = style;

	if (bCharStyle)
	{
		// set character-level style
		if (isSelectionEmpty())
		{
			_eraseInsertionPoint();
		}

		_clearIfAtFmtMark(getPoint());	// TODO is this correct ??
		_eraseSelection();

		bRet = m_pDoc->changeSpanFmt(PTC_AddStyle,posStart,posEnd,attribs,NULL);
#ifdef BIDI_ENABLED
		/*	
			we need to restore the direction of each run
			to do so we will make use of the fact that the previous operation
			did not clear m_iDirection of the runs involved
		*/
		fl_BlockLayout* prevBlock = NULL;
		fl_BlockLayout* currBlock = NULL;
		for(UT_uint32 i = posStart; i<=posEnd; i++)
		{
			currBlock = _findBlockAtPosition(i);
			if(currBlock == prevBlock)
				continue;
				
			PT_DocPosition blockPos = currBlock->getPosition();
			fp_Run * currRun = currBlock->getFirstRun();
			PT_DocPosition runStart, runEnd;
				
			UT_uint32 j;
			for(j = i; j <= posEnd; j++)
			{
				runStart = blockPos + currRun->getBlockOffset();
				runEnd = runStart + currRun->getLength();
				if(runStart > posEnd)
					break;
				if(runStart <= j && runEnd >= j)
				{
					currRun->setDirectionProperty(currRun->getDirection());
					j = runEnd;
				}
				currRun = currRun->getNext();
				if(currRun->getBlock() != currBlock)
					break;
			}
			
			if(runStart > posEnd)
				break;
			i = j;
			prevBlock = currBlock;
		}
#endif		
	}
	else
	{
		// set block-level style
		_eraseInsertionPoint();

		_clearIfAtFmtMark(getPoint());	// TODO is this correct ??

		// NB: clear explicit props at both block and char levels
		bRet = m_pDoc->changeStruxFmt(PTC_AddStyle,posStart,posEnd,attribs,NULL,PTX_Block);
#ifdef BIDI_ENABLED
		/*	
			we need to restore the direction of each run
			to do so we will used the direction information chached
			in m_iDirection of each run, which was not reset by
			the previous operation
		*/
		fl_BlockLayout* prevBlock = NULL;
		fl_BlockLayout* currBlock = NULL;
		for(UT_uint32 i = posStart; i<=posEnd; i++)
		{
			currBlock = _findBlockAtPosition(i);
			UT_ASSERT((currBlock));
			if(currBlock == prevBlock)
				continue;
				
			fp_Run * currRun = currBlock->getFirstRun();
			UT_ASSERT((currRun));
			fl_BlockLayout* currRunBlock = currRun->getBlock();
			while(currBlock == currRunBlock)
			{
				if(currRun->getType() == FPRUN_ENDOFPARAGRAPH)
					break;
				currRun->setDirectionProperty(currRun->getDirection());
				currRun = currRun->getNext();
				if(currRun)
					currRunBlock = currRun->getBlock();
				else
					break;
			}
			
			prevBlock = currBlock;
		}
#endif		
	}

	if(!bDontGeneralUpdate)
	{
		_generalUpdate();
		// restore updates and clean up dirty lists
		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();

		// Signal piceTable is stable again
		m_pDoc->notifyPieceTableChangeEnd();

		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
	return bRet;
}


static const XML_Char * x_getStyle(const PP_AttrProp * pAP, bool bBlock)
{
	UT_ASSERT(pAP);
	const XML_Char* sz = NULL;

	pAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, sz);

	// TODO: should we have an explicit default for char styles?
	if (!sz && bBlock)
		sz = "Normal";

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
	m_pDoc->notifyPieceTableChangeStart();

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

	_eraseSelection();
	
	bRet = m_pDoc->changeSpanFmt(PTC_AddFmt,posStart,posEnd,NULL,properties);

	_generalUpdate();

	if (isSelectionEmpty())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	// Signal piceTable is stable again
	m_pDoc->notifyPieceTableChangeEnd();

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

	v.addItem(new _fmtPair("font-family",	pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("font-size",		pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("font-weight",	pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("font-style",		pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("text-decoration",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("text-position",	pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("color",			pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("bgcolor",		pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
#ifdef BIDI_ENABLED
	v.addItem(new _fmtPair("dir",		pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("dir-override",	pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
#endif
	v.addItem(new _fmtPair("lang",		pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));

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

				if (!pBlock) 			// at EOD, so just bail
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
   \param   v Pointer to Vector of all the blocks found
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

   \param   doList true if you want to indents all the blocks in the list
            of which the current block is a member. If false just those 
            blocks within the current selected range. 
   \param   indentChange +-ve value by which the block will be indented.
   \param   page_size width of the page in inches.
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
	m_pDoc->notifyPieceTableChangeStart();

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
	m_pDoc->notifyPieceTableChangeEnd();

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
	m_pDoc->notifyPieceTableChangeStart();


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
	m_pDoc->notifyPieceTableChangeEnd();

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
\param const  XML_Char ** atts  const string describing left , center or right
 * justification.
 */

bool FV_View::processPageNumber(bool bIsFooter, const XML_Char ** atts)
{
	fl_DocSectionLayout * pDSL = getCurrentPage()->getOwningSection();
	if(bIsFooter && pDSL->getFooter() == NULL)
	{
//
// Quick hack to stop a segfault if a user tries to insert a header from
// within a footer. 
//
		if(isHdrFtrEdit())
		{
			clearHdrFtrEdit();
			warpInsPtToXY(0,0,false);
		}
		insertPageNum(atts, bIsFooter);
		return true;
	}
	else if(!bIsFooter && pDSL->getHeader() == NULL)
	{
//
// Quick hack to stop a segfault if a user tries to insert a header from
// within a footer. 
//
		if(isHdrFtrEdit())
		{
			clearHdrFtrEdit();
			warpInsPtToXY(0,0,false);
		}
		insertPageNum(atts, bIsFooter);
		return true;
	}
//
// OK we're here if we want to insert a page number into a pre-existing 
// header/footer. Let's get the header/footer now.
//
	fl_HdrFtrSectionLayout * pHFSL = NULL;
	if(bIsFooter)
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

	m_pDoc->notifyPieceTableChangeStart();
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


		// Signal PieceTable Changes have finished
		_generalUpdate();
		m_pDoc->notifyPieceTableChangeEnd();
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

	const XML_Char*	f_attributes[] = {
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

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	_generalUpdate();
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
	m_pDoc->notifyPieceTableChangeStart();

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
		m_pDoc->notifyPieceTableChangeEnd();

		return;
	}

	XML_Char * style = getCurrentBlock()->getListStyleString(lType);
	_eraseInsertionPoint();
	va.addItem( (void *) "style");	va.addItem( (void *) style);

	pAuto->setListType(lType);
	sprintf(pszStart, "%i" , startv);
	UT_XML_strncpy(	pszAlign,
					sizeof(pszAlign),
					UT_convertInchesToDimensionString(DIM_IN, Align, 0));

	UT_XML_strncpy(	pszIndent,
					sizeof(pszIndent),
					UT_convertInchesToDimensionString(DIM_IN, Indent, 0));

	vp.addItem( (void *) "start-value");	vp.addItem( (void *) pszStart);
	vp.addItem( (void *) "margin-left");	vp.addItem( (void *) pszAlign);
	vp.addItem( (void *) "text-indent");	vp.addItem( (void *) pszIndent);
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

 	//const XML_Char * attrib_list[] = {"style", style, 0 };
	_eraseInsertionPoint();
	i = 0;
	sdh = (PL_StruxDocHandle) pAuto->getNthBlock(i);
	while(sdh != NULL)
	{
		PT_DocPosition iPos = m_pDoc->getStruxPosition(sdh)+fl_BLOCK_STRUX_OFFSET;
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, iPos, iPos, attribs, props, PTX_Block);
		i++;
		sdh = (PL_StruxDocHandle) pAuto->getNthBlock(i);
		_generalUpdate();
	}

	_generalUpdate();
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

	_ensureThatInsertionPointIsOnScreen();
	DELETEP(attribs);
	DELETEP(props);
}

bool FV_View::cmdStopList(void)
{


	// Signal PieceTable Change
	m_pDoc->notifyPieceTableChangeStart();

	m_pDoc->beginUserAtomicGlob();
	fl_BlockLayout * pBlock = getCurrentBlock();
	m_pDoc->StopList(pBlock->getStruxDocHandle());
	m_pDoc->endUserAtomicGlob();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

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
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
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

	v.addItem(new _fmtPair("text-align",				NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("text-indent",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("margin-left",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("margin-right",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("margin-top",				NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("margin-bottom",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("line-height",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("tabstops",				NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("default-tab-interval",	NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("keep-together",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("keep-with-next",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("orphans",				NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("widows",					NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
#ifdef BIDI_ENABLED
	v.addItem(new _fmtPair("dom-dir",		NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
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
	m_pDoc->notifyPieceTableChangeStart();

	if (iPos == getPoint())
	{
		return;
	}

	_extSelToPos(iPos);
	_deleteSelection();

	_generalUpdate();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

	_fixInsertionPointCoords();
	_drawInsertionPoint();
}

/*
	This function is somewhat of a compromise.  It will return a new
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
	m_pDoc->notifyPieceTableChangeStart();

	if (!isSelectionEmpty())
	{
		m_pDoc->disableListUpdates();

		_deleteSelection();

		_generalUpdate();

		// restore updates and clean up dirty lists
		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();

		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
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
				count = 2;
				bisList = true;
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
			// restore updates and clean up dirty lists
			m_pDoc->enableListUpdates();
			m_pDoc->updateDirtyLists();

			if(fontFlag)
			{
				setCharFormat(properties);
			}
		}

		_generalUpdate();
		free(props_in);

		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

}

void FV_View::_moveInsPtNextPrevLine(bool bNext)
{
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
	UT_sint32 iLineHeight;
	UT_sint32 xPoint2;
	UT_sint32 yPoint2;
	bool bDirection;
	/*
	  This function moves the IP up or down one line, attempting to get
	  as close as possible to the prior "sticky" x position.  The notion
	  of "next" is strictly physical, not logical.

	  For example, instead of always moving from the last line of one block
	  to the first line of the next, you might wind up skipping over a
	  bunch of blocks to wind up in the first line of the second column.
	*/
	UT_sint32 xOldSticky = m_xPointSticky;

	// first, find the line we are on now
	UT_uint32 iOldPoint = getPoint();

	fl_BlockLayout* pOldBlock = _findBlockAtPosition(iOldPoint);
	fp_Run* pOldRun = pOldBlock->findPointCoords(getPoint(), m_bPointEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);
	fl_SectionLayout* pOldSL = pOldBlock->getSectionLayout();
	fp_Line* pOldLine = pOldRun->getLine();
	fp_Container* pOldContainer = pOldLine->getContainer();
	fp_Page* pOldPage = pOldContainer->getPage();
	bool bDocSection = (pOldSL->getType() == FL_SECTION_DOC);

	fp_Column* pOldLeader = NULL;
	if (bDocSection)
	{
		pOldLeader = ((fp_Column*) (pOldContainer))->getLeader();
	}

	UT_sint32 iPageOffset;
	getPageYOffset(pOldPage, iPageOffset);

	UT_sint32 iLineX = 0;
	UT_sint32 iLineY = 0;

	pOldContainer->getOffsets(pOldLine, iLineX, iLineY);
	yPoint = iLineY;

	iLineHeight = pOldLine->getHeight();

	bool bNOOP = false;

	if (bNext)
	{
		if (pOldLine != pOldContainer->getLastLine())
		{
			// just move off this line
			// Sevior TODO the +2 is a work around. The problem is somewhere else
			yPoint += (iLineHeight + pOldLine->getMarginAfter()+2);
		}
		else if (bDocSection && (((fp_Column*) (pOldSL->getLastContainer()))->getLeader() == pOldLeader))
		{
			// move to next section
			fl_SectionLayout* pSL = pOldSL->getNext();
			if (pSL)
			{
				yPoint = pSL->getFirstContainer()->getY();
			}
			else
			{
				bNOOP = true;
			}
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
		if (pOldLine != pOldContainer->getFirstLine())
		{
			// just move off this line
			// Sevior TODO the +2 is a work around. The problem is somewhere else
			yPoint -= (pOldLine->getMarginBefore() + 2);
		}
		else if (bDocSection && (pOldSL->getFirstContainer() == pOldLeader))
		{
			// move to prev section
			fl_SectionLayout* pSL = pOldSL->getPrev();
			if (pSL)
			{
				fp_Column* pTmpCol = ((fp_Column*) (pSL->getLastContainer()))->getLeader();
				yPoint = pTmpCol->getY();

				UT_sint32 iMostHeight = 0;
				while (pTmpCol)
				{
					iMostHeight = UT_MAX(iMostHeight, pTmpCol->getHeight());

					pTmpCol = pTmpCol->getFollower();
				}

				yPoint += (iMostHeight - 1);
			}
			else
			{
				bNOOP = true;
			}
		}
		else
		{
			// move to prev page
			fp_Page* pPage = pOldPage->getPrev();
			if (pPage)
			{
				getPageYOffset(pPage, iPageOffset);
				yPoint = pPage->getBottom();
			}
			else
			{
				bNOOP = true;
			}
		}
	}

	if (bNOOP)
	{
		// cannot move.  should we beep?
		_drawInsertionPoint();
		return;
	}

	// change to screen coordinates
	xPoint = m_xPointSticky - m_xScrollOffset + fl_PAGEVIEW_MARGIN_X;
	yPoint += iPageOffset - m_yScrollOffset;

	// hit-test to figure out where that puts us
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPoint, yPoint, xClick, yClick);

	PT_DocPosition iNewPoint;
	bool bBOL = false;
	bool bEOL = false;
	fl_HdrFtrShadow * pShadow=NULL;
	pPage->mapXYToPositionClick(xClick, yClick, iNewPoint,pShadow, bBOL, bEOL);
//
// Check we're not moving out of allowed region.
//
	PT_DocPosition posBOD,posEOD;
	getEditableBounds(false,posBOD);
	getEditableBounds(true,posEOD);

	UT_DEBUGMSG(("iNewPoint=%d, iOldPoint=%d, xClick=%d, yClick=%d\n",iNewPoint, iOldPoint, xClick, yClick));
	UT_ASSERT(iNewPoint != iOldPoint);
	if(iNewPoint >= posBOD && iNewPoint <= posEOD)
		_setPoint(iNewPoint, bEOL);

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	// this is the only place where we override changes to m_xPointSticky
	m_xPointSticky = xOldSticky;
}

bool FV_View::_ensureThatInsertionPointIsOnScreen(void)
{
	bool bRet = false;

	if (m_iWindowHeight <= 0)
	{
		return false;
	}

	_fixInsertionPointCoords();

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
		cmdScroll(AV_SCROLLCMD_LINELEFT, (UT_uint32) (-(m_xPoint) + fl_PAGEVIEW_MARGIN_X/2));
		bRet = true;
	}
	else if (((UT_uint32) (m_xPoint)) >= ((UT_uint32) m_iWindowWidth))
	{
		cmdScroll(AV_SCROLLCMD_LINERIGHT, (UT_uint32)(m_xPoint - m_iWindowWidth + fl_PAGEVIEW_MARGIN_X/2));
		bRet = true;
	}
	if(bRet == false)
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

	// try to locate next/prev page
	fp_Page* pPage = (bNext ? pOldPage->getNext() : pOldPage->getPrev());

	// if couldn't move, go to top of this page instead
	if (!pPage)
		pPage = pOldPage;

	_moveInsPtToPage(pPage);
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

	iPageOffset -= fl_PAGEVIEW_PAGE_SEP /2;
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
	if (!_ensureThatInsertionPointIsOnScreen() && !bVScroll)
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
}

void FV_View::warpInsPtNextPrevPage(bool bNext)
{
	if (!isSelectionEmpty())
		_moveToSelectionEnd(bNext);
	else
		_eraseInsertionPoint();

	_resetSelection();
	_clearIfAtFmtMark(getPoint());
	_moveInsPtNextPrevPage(bNext);
	notifyListeners(AV_CHG_MOTION);
}

void FV_View::warpInsPtNextPrevLine(bool bNext)
{
	if (!isSelectionEmpty())
		_moveToSelectionEnd(bNext);
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
	
	_ensureThatInsertionPointIsOnScreen();

	// It IS possible for the selection to be empty, even
	// after extending it.  If the charMotion fails, for example,
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

	if (!_ensureThatInsertionPointIsOnScreen())
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

void FV_View::_autoScroll(UT_Timer * pTimer)
{
	UT_ASSERT(pTimer);
	
	// this is a static callback method and does not have a 'this' pointer.

	FV_View * pView = (FV_View *) pTimer->getInstanceData();
	UT_ASSERT(pView);

	if(pView->getLayout()->getDocument()->isPieceTableChanging() == true)
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
		if (!pView->_ensureThatInsertionPointIsOnScreen())
		{
			pView->_fixInsertionPointCoords();
//			pView->_drawInsertionPoint();
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
	xClick = xPos + m_xScrollOffset - fl_PAGEVIEW_MARGIN_X;
	yClick = yPos + m_yScrollOffset - fl_PAGEVIEW_MARGIN_Y;
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		UT_sint32 iPageHeight = pPage->getHeight();
		if (yClick < iPageHeight)
		{
			// found it
			break;
		}
		else
		{
			yClick -= iPageHeight + fl_PAGEVIEW_PAGE_SEP;
		}
		pPage = pPage->getNext();
	}

	if (!pPage)
	{
		// we're below the last page
		pPage = m_pLayout->getLastPage();

		UT_sint32 iPageHeight = pPage->getHeight();
		yClick += iPageHeight + fl_PAGEVIEW_PAGE_SEP;
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

	UT_uint32 number = atol(numberString);

	if (dec || inc)
		numberString--;
	FREEP(numberString);

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
				_moveInsPtNextPrevLine (bNext);	// HACK: A like the quick hacks... :)
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
	default:
		// TODO
		;
	}

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

	return false;
}

// ---------------- start find and replace ---------------
	
bool FV_View::findNext(const UT_UCSChar * find, bool matchCase, bool * bDoneEntireDocument)
{
	if (!isSelectionEmpty())
	{
		_clearSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	bool bRes = _findNext(find, matchCase, bDoneEntireDocument);

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

	// TODO do we need to do a notifyListeners(AV_CHG_MOTION) ??
	return bRes;
}

bool FV_View::_findNext(const UT_UCSChar * find, bool matchCase, bool * bDoneEntireDocument)
{
	UT_ASSERT(find);

	fl_BlockLayout* block = NULL;
	PT_DocPosition offset = 0;
	
	block = _findGetCurrentBlock();
	offset = _findGetCurrentOffset();

	UT_UCSChar * buffer = NULL;

	//Now we compute the static prefix function
	//Which can be done based soley on the find string

	UT_uint32	m = UT_UCS_strlen(find);
	UT_uint32	*prefix;
	UT_uint32	k = 0;
	UT_uint32	q = 1;

	prefix = (UT_uint32*) calloc (m, sizeof(UT_uint32));

	prefix[0] = 0; //Must be this reguardless of the string

	if (matchCase)
	{
		for (q = 1; q < m; q++)
		{
			while (k > 0 && find[k] != find[q])
				k = prefix[k - 1];
			if(find[k] == find[q])
				k++;
			prefix[q] = k;
		}
	}
	else //!matchCase
	{
		for (q = 1; q < m; q++)
		{
			while (k > 0 && UT_UCS_tolower(find[k]) != UT_UCS_tolower(find[q]))
				k = prefix[k - 1];
			if(UT_UCS_tolower(find[k]) == UT_UCS_tolower(find[q]))
				k++;
			prefix[q] = k;
		}
	}

	//Now we use this prefix function (stored as an array)
	//to search through the document text.
	while ((buffer = _findGetNextBlockBuffer(&block, &offset)))
	{
		
		// magic number; range of UT_sint32 falls short of extremely large docs
		UT_sint32	foundAt = -1;
		UT_uint32	i = 0;
		UT_uint32	t = 0;

		if (matchCase)
		{
			while (buffer[i] /*|| foundAt == -1*/)
			{
				while (t > 0 && find[t] != buffer[i])
					t = prefix[t-1];
				if (find[t] == buffer[i])
					t++;
				i++;
				if (t == m)
				{
					foundAt = i - m;
					break;
				}
			}
		}
		else //!matchCase
		{
			while (buffer[i] /*|| foundAt == -1*/)
			{
				while (t > 0 && UT_UCS_tolower(find[t]) != UT_UCS_tolower(buffer[i]))
					t = prefix[t-1];
				if (UT_UCS_tolower(find[t]) == UT_UCS_tolower(buffer[i]))
					t++;
				i++;
				if (t == m)
				{
					foundAt = i - m;
					break;
				}
			}
		}


		if (foundAt != -1)
		{
			_setPoint(block->getPosition(false) + offset + foundAt);
			_setSelectionAnchor();
			_charMotion(true, UT_UCS_strlen(find));

			m_doneFind = true;

			FREEP(buffer);
			FREEP(prefix);
			return true;
		}

		// didn't find anything, so set the offset to the end
		// of the current area
		offset += UT_UCS_strlen(buffer);

		// must clean up buffer returned for search
		FREEP(buffer);
	}

	if (bDoneEntireDocument)
	{
		*bDoneEntireDocument = true;
	}

	// reset wrap for next time
	m_wrappedEnd = false;

	FREEP(prefix);
	
	return false;
}

//Does exactly the same as the previous function except that the prefix
//function is passed in as an agrument rather than computed within
//the function body.
bool FV_View::_findNext(const UT_UCSChar * find, UT_uint32 *prefix,
						bool matchCase, bool * bDoneEntireDocument)
{
	UT_ASSERT(find);

	fl_BlockLayout*	block = NULL;
	PT_DocPosition	offset = 0;
	
	block = _findGetCurrentBlock();
	offset = _findGetCurrentOffset();

	UT_UCSChar * buffer = NULL;
	UT_uint32	m = UT_UCS_strlen(find);

	

	//Now we use the prefix function (stored as an array)
	//to search through the document text.
	while ((buffer = _findGetNextBlockBuffer(&block, &offset)))
	{
		
		// magic number; range of UT_sint32 falls short of extremely large docs
		UT_sint32	foundAt = -1;
		UT_uint32	i = 0;
		UT_uint32	t = 0;

		if (matchCase)
		{
			while (buffer[i] /*|| foundAt == -1*/)
			{
				while (t > 0 && find[t] != buffer[i])
					t = prefix[t-1];
				if (find[t] == buffer[i])
					t++;
				i++;
				if (t == m)
				{
					foundAt = i - m;
					break;
				}
			}
		}
		else //!matchCase
		{
			while (buffer[i] /*|| foundAt == -1*/)
			{
				while (t > 0 && UT_UCS_tolower(find[t]) != UT_UCS_tolower(buffer[i]))
					t = prefix[t-1];
				if (UT_UCS_tolower(find[t]) == UT_UCS_tolower(buffer[i]))
					t++;
				i++;
				if (t == m)
				{
					foundAt = i - m;
					break;
				}
			}
		}


		if (foundAt != -1)
		{
			_setPoint(block->getPosition(false) + offset + foundAt);
			_setSelectionAnchor();
			_charMotion(true, UT_UCS_strlen(find));

			m_doneFind = true;

			FREEP(buffer);
			return true;
		}

		// didn't find anything, so set the offset to the end
		// of the current area
		offset += UT_UCS_strlen(buffer);

		// must clean up buffer returned for search
		FREEP(buffer);
	}

	if (bDoneEntireDocument)
	{
		*bDoneEntireDocument = true;
	}

	// reset wrap for next time
	m_wrappedEnd = false;
	
	return false;
}

void FV_View::findSetStartAtInsPoint(void)
{
	m_startPosition = m_iInsPoint;
	m_wrappedEnd = false;
	m_doneFind = false;
}

PT_DocPosition FV_View::_BlockOffsetToPos(fl_BlockLayout * block, PT_DocPosition offset)
{
	UT_ASSERT(block);
	return block->getPosition(false) + offset;
}

UT_UCSChar * FV_View::_findGetNextBlockBuffer(fl_BlockLayout ** block, PT_DocPosition * offset)
{
	UT_ASSERT(m_pLayout);

	// this assert doesn't work, since the startPosition CAN legitimately be zero
	UT_ASSERT(m_startPosition >= 2);	// the beginning of the first block in any document
	
	UT_ASSERT(block);
	UT_ASSERT(*block);

	UT_ASSERT(offset);
	
	fl_BlockLayout * newBlock = NULL;
	PT_DocPosition newOffset = 0;

	UT_uint32 bufferLength = 0;
	
	UT_GrowBuf buffer;

	// check early for completion, from where we left off last, and bail
	// if we are now at or past the start position
	if (m_wrappedEnd && _BlockOffsetToPos(*block, *offset) >= m_startPosition)
	{
		// we're done
		return NULL;
	}

	if (!(*block)->getBlockBuf(&buffer))
	{
		UT_DEBUGMSG(("Block %p has no associated buffer.\n", *block));
		UT_ASSERT(0);
	}
	
	// have we already searched all the text in this buffer?
	if (*offset >= buffer.getLength())
	{
		// then return a fresh new block's buffer
		newBlock = (*block)->getNextBlockInDocument();

		// are we at the end of the document?
		if (!newBlock)
		{
			// then wrap (fetch the first block in the doc)
			PT_DocPosition startOfDoc;
			getEditableBounds(false, startOfDoc);
			
			newBlock = m_pLayout->findBlockAtPosition(startOfDoc);

			m_wrappedEnd = true;
			
			UT_ASSERT(newBlock);
		}

		// re-assign the buffer contents for our new block
		buffer.truncate(0);
		// the offset starts at 0 for a fresh buffer
		newOffset = 0;
		
		if (!newBlock->getBlockBuf(&buffer))
		{
			UT_DEBUGMSG(("Block %p (a ->next block) has no buffer.\n", newBlock));
			UT_ASSERT(0);
		}

		// good to go with a full buffer for our new block
	}
	else	// we have some left to go in this buffer
	{
		// buffer is still valid, just copy pointers
		newBlock = *block;
		newOffset = *offset;
	}

	// are we going to run into the start position in this buffer?
	// if so, we need to size our length accordingly
	if (m_wrappedEnd && _BlockOffsetToPos(newBlock, newOffset) + buffer.getLength() >= m_startPosition)
	{
		bufferLength = (m_startPosition - (newBlock)->getPosition(false)) - newOffset;
	}
	else
	{
		bufferLength = buffer.getLength() - newOffset;
	}
	
	// clone a buffer (this could get really slow on large buffers!)
	UT_UCSChar * bufferSegment = NULL;

	// remember, the caller gets to free this memory
	bufferSegment = (UT_UCSChar*)calloc(bufferLength + 1, sizeof(UT_UCSChar));
	UT_ASSERT(bufferSegment);
	
	memmove(bufferSegment, buffer.getPointer(newOffset),
			(bufferLength) * sizeof(UT_UCSChar));

	// before we bail, hold up our block stuff for next round
	*block = newBlock;
	*offset = newOffset;
		
	return bufferSegment;
}

bool FV_View::findSetNextString(UT_UCSChar * string, bool matchCase)
{
	UT_ASSERT(string);

	// update case matching
	_m_matchCase = matchCase;

	// update string
	FREEP(_m_findNextString);
	return UT_UCS_cloneString(&_m_findNextString, string);
}

bool FV_View::findAgain()
{
	if (_m_findNextString && *_m_findNextString)
	{
		bool bRes = findNext(_m_findNextString, _m_matchCase, NULL);
		if (bRes)
		{
			_drawSelection();
		}

		return bRes;
	}
	
	return false;
}

bool	FV_View::findReplace(const UT_UCSChar * find, const UT_UCSChar * replace,
							 bool matchCase, bool * bDoneEntireDocument)
{
	UT_ASSERT(find && replace);
	
	bool bRes = _findReplace(find, replace, matchCase, bDoneEntireDocument);

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

bool	FV_View::_findReplace(const UT_UCSChar * find, const UT_UCSChar * replace,
							  bool matchCase, bool * bDoneEntireDocument)
{
	UT_ASSERT(find && replace);

	// if we have done a find, and there is a selection, then replace what's in the
	// selection and move on to next find (batch run, the common case)
	if ((m_doneFind == true) && (!isSelectionEmpty()))
	{
		bool result = true;

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

		// if we have a string with length, do an insert, else let it hang
		// from the delete above
		if (*replace)
			result = m_pDoc->insertSpan(getPoint(),
										replace,
										UT_UCS_strlen(replace),
										&AttrProp_Before);

		_generalUpdate();

		// if we've wrapped around once, and we're doing work before we've
		// hit the point at which we started, then we adjust the start
		// position so that we stop at the right spot.
		if (m_wrappedEnd && !*bDoneEntireDocument)
			m_startPosition += ((long) UT_UCS_strlen(replace) - (long) UT_UCS_strlen(find));

		UT_ASSERT(m_startPosition >= 2);

		// do not increase the insertion point index, since the insert span will
		// leave us at the correct place.
		
		_findNext(find, matchCase, bDoneEntireDocument);
		return result;
	}

	// if we have done a find, but there is no selection, do a find for them
	// but no replace
	if (m_doneFind == true && isSelectionEmpty() == true)
	{
		_findNext(find, matchCase, bDoneEntireDocument);
		return false;
	}
	
	// if we haven't done a find yet, do a find for them
	if (m_doneFind == false)
	{
		_findNext(find, matchCase, bDoneEntireDocument);
		return false;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}


bool	FV_View::_findReplace(const UT_UCSChar * find, const UT_UCSChar * replace,
							  UT_uint32 *prefix, bool matchCase, bool * bDoneEntireDocument)
{
	UT_ASSERT(find && replace);

	// if we have done a find, and there is a selection, then replace what's in the
	// selection and move on to next find (batch run, the common case)
	if ((m_doneFind == true) && (!isSelectionEmpty()))
	{
		bool result = true;

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

		// if we have a string with length, do an insert, else let it hang
		// from the delete above
		if (*replace)
			result = m_pDoc->insertSpan(getPoint(),
										replace,
										UT_UCS_strlen(replace),
										&AttrProp_Before);

		_generalUpdate();

		// if we've wrapped around once, and we're doing work before we've
		// hit the point at which we started, then we adjust the start
		// position so that we stop at the right spot.
		if (m_wrappedEnd && !*bDoneEntireDocument)
			m_startPosition += ((long) UT_UCS_strlen(replace) - (long) UT_UCS_strlen(find));

		UT_ASSERT(m_startPosition >= 2);

		// do not increase the insertion point index, since the insert span will
		// leave us at the correct place.
		
		_findNext(find, prefix, matchCase, bDoneEntireDocument);
		return result;
	}

	// if we have done a find, but there is no selection, do a find for them
	// but no replace
	if (m_doneFind == true && isSelectionEmpty() == true)
	{
		_findNext(find, prefix, matchCase, bDoneEntireDocument);
		return false;
	}
	
	// if we haven't done a find yet, do a find for them
	if (m_doneFind == false)
	{
		_findNext(find, prefix, matchCase, bDoneEntireDocument);
		return false;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}

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
	m_pDoc->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);

	/*
	  TODO note that we are far too heavy handed with the mask we
	  send here.  I ripped out all the individual calls to notifyListeners
	  which appeared within fl_BlockLayout, and they all now go through
	  here.  For that reason, I made the following mask into the union
	  of all the masks I found.  I assume that this is inefficient, but
	  functionally correct.

	  TODO WRONG! WRONG! WRONG! notifyListener() must be called in
	  TODO WRONG! WRONG! WRONG! fl_BlockLayout in response to a change
	  TODO WRONG! WRONG! WRONG! notification and not here.  this call
	  TODO WRONG! WRONG! WRONG! will only update the current window.
	  TODO WRONG! WRONG! WRONG! having the notification in fl_BlockLayout
	  TODO WRONG! WRONG! WRONG! will get each view on the document.
	*/
	notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR | AV_CHG_FMTBLOCK);
}

UT_uint32 FV_View::findReplaceAll(const UT_UCSChar * find, const UT_UCSChar * replace,
								  bool matchCase)
{
	UT_uint32 numReplaced = 0;
	m_pDoc->beginUserAtomicGlob();

	bool bDoneEntireDocument = false;

	//Now we compute the static prefix function
	//Which can be done based soley on the find string
	//we compute this here rather than inside the _findNext
	//function so that it is not needlessy re-computed
	//this will save a lot on large documents with many
	//hits of the find word.

	UT_uint32	m = UT_UCS_strlen(find);
	UT_uint32	*prefix;
	UT_uint32	k = 0;
	UT_uint32	q = 1;

	prefix = (UT_uint32*) calloc (m, sizeof(UT_uint32));

	prefix[0] = 0; //Must be this reguardless of the string

	if (matchCase)
	{
		for (q = 1; q < m; q++)
		{
			while (k > 0 && find[k] != find[q])
				k = prefix[k - 1];
			if(find[k] == find[q])
				k++;
			prefix[q] = k;
		}
	}
	else //!matchCase
	{
		for (q = 1; q < m; q++)
		{
			while (k > 0 && UT_UCS_tolower(find[k]) != UT_UCS_tolower(find[q]))
				k = prefix[k - 1];
			if(UT_UCS_tolower(find[k]) == UT_UCS_tolower(find[q]))
				k++;
			prefix[q] = k;
		}
	}
	
	// prime it with a find
	if (!_findNext(find, prefix, matchCase, &bDoneEntireDocument))
	{
		// can't find a single thing, we're done
		m_pDoc->endUserAtomicGlob();
		return numReplaced;
	}
	
	// while we've still got buffer
	while (bDoneEntireDocument == false)
	{
		// if it returns false, it found nothing more before
		// it hit the end of the document
		if (!_findReplace(find, replace, prefix, matchCase, &bDoneEntireDocument))
		{
			m_pDoc->endUserAtomicGlob();
			return numReplaced;
		}
		numReplaced++;
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

	//Clean up the prefix function array
	FREEP(prefix);

	return numReplaced;
}

fl_BlockLayout * FV_View::_findGetCurrentBlock(void)
{
	return _findBlockAtPosition(m_iInsPoint);
}


fl_BlockLayout * FV_View::getCurrentBlock(void)
{
	return _findGetCurrentBlock();
}

PT_DocPosition FV_View::_findGetCurrentOffset(void)
{
	return (m_iInsPoint - _findGetCurrentBlock()->getPosition(false));
}

//
// A simple strstr search of the buffer.
//
UT_sint32 FV_View::_findBlockSearchDumbCase(const UT_UCSChar * haystack, const UT_UCSChar * needle)
{
	UT_ASSERT(haystack);
	UT_ASSERT(needle);
		
	UT_UCSChar * at = UT_UCS_strstr(haystack, needle);

	return (at) ? (at - haystack) : -1;
}

//
// Pierre Sarrazin <ps@cam.org> supplied the Unicode stricmp comparison function,
// which works for Latin-1 at the moment.
//
UT_sint32 FV_View::_findBlockSearchDumbNoCase(const UT_UCSChar * haystack, const UT_UCSChar * needle)
{
	UT_ASSERT(haystack);
	UT_ASSERT(needle);
		
	UT_UCSChar * at = UT_UCS_stristr(haystack, needle);

	return (at) ? (at - haystack) : -1;
}


//
// Any takers?
//
UT_sint32 FV_View::_findBlockSearchRegexp(const UT_UCSChar * /* haystack */, const UT_UCSChar * /* needle */)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	
	return -1;
}

// ---------------- end find and replace ---------------

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
	m_pDoc->notifyPieceTableChangeStart();

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
	
	_setPoint(pos, bEOL);
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	notifyListeners(AV_CHG_MOTION);

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
}


void FV_View::getPageScreenOffsets(fp_Page* pThePage, UT_sint32& xoff,
								   UT_sint32& yoff)
{
	UT_uint32 y = fl_PAGEVIEW_MARGIN_Y;
	
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		if (pPage == pThePage)
		{
			break;
		}
		y += pPage->getHeight() + fl_PAGEVIEW_PAGE_SEP;

		pPage = pPage->getNext();
	}

	yoff = y - m_yScrollOffset;
	xoff = fl_PAGEVIEW_MARGIN_Y - m_xScrollOffset;
}

void FV_View::getPageYOffset(fp_Page* pThePage, UT_sint32& yoff)
{
	UT_uint32 y = fl_PAGEVIEW_MARGIN_Y;
	
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		if (pPage == pThePage)
		{
			break;
		}
		y += pPage->getHeight() + fl_PAGEVIEW_PAGE_SEP;

		pPage = pPage->getNext();
	}

	yoff = y;
}

UT_uint32 FV_View::getPageViewLeftMargin(void) const
{
	// return the amount of gray-space we draw to the left
	// of the paper in "Page View".  return zero if not in
	// "Page View".

	return fl_PAGEVIEW_MARGIN_X;
}

UT_uint32 FV_View::getPageViewTopMargin(void) const
{
	// return the amount of gray-space we draw above the top
	// of the paper in "Page View".  return zero if not in
	// "Page View".

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
	//       makes locating runs to clear more difficult.  For now, I'm
	//       playing it safe and trying to just handle these cases here.
	//       The real solution may be to just bail if *either* is NULL,
	//       but I'm not sure.
	//
	//       If you're interested in investigating this alternative
	//       approach, play with the following asserts.

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
	// dereferencing NULL.  Dom 11.9.00
	if(!pBlock)
	{
		// Do the assert. Want to know from debug builds when this happens.
		UT_ASSERT(pBlock);

		x = x2 = 0;
		y = y2 = 0;
		
		height = 0;
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
		fp_Page* pPointPage = pRun->getLine()->getContainer()->getPage();

		UT_sint32 iPageOffset;
		getPageYOffset(pPointPage, iPageOffset);

		yPoint += iPageOffset;
		xPoint += fl_PAGEVIEW_MARGIN_X;
#ifdef BIDI_ENABLED
		yPoint2 += iPageOffset;
		xPoint2 += fl_PAGEVIEW_MARGIN_X;
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
	_saveCurrentPoint();
	// hang onto this for _moveInsPtNextPrevLine()
	m_xPointSticky = m_xPoint + m_xScrollOffset - fl_PAGEVIEW_MARGIN_X;
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
	if (m_iPointHeight > 0 )
	{
	  UT_RGBColor * pClr = getCurrentPage()->getOwningSection()->getPaperColor();
	  m_pG->setColor(*pClr);
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
	if(m_focus==AV_FOCUS_NONE)
		return;
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

void FV_View::_autoDrawPoint(UT_Timer * pTimer)
{
	UT_ASSERT(pTimer);

	FV_View * pView = (FV_View *) pTimer->getInstanceData();
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
	if (dx > 0)
	{
		if (dx >= m_iWindowWidth)
		{
			_draw(0, 0, m_iWindowWidth, m_iWindowHeight, false, true);
		}
		else
		{
			_draw(m_iWindowWidth - dx, 0, m_iWindowWidth, m_iWindowHeight, false, true);
		}
	}
	else
	{
		if (dx <= -m_iWindowWidth)
		{
			_draw(0, 0, m_iWindowWidth, m_iWindowHeight, false, true);
		}
		else
		{
			_draw(0, 0, -dx, m_iWindowHeight, false, true);
		}
	}
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
	if (dy > 0)
	{
		if (dy >= m_iWindowHeight)
		{
			_draw(0, 0, m_iWindowWidth, m_iWindowHeight, false, true);
		}
		else
		{
			_draw(0, m_iWindowHeight - dy, m_iWindowWidth, dy, false, true);
		}
	}
	else
	{
		if (dy <= -m_iWindowHeight)
		{
			_draw(0, 0, m_iWindowWidth, m_iWindowHeight, false, true);
		}
		else
		{
			_draw(0, 0, m_iWindowWidth, -dy, false, true);
		}
	}
	_fixInsertionPointCoords();
	_drawInsertionPoint();
}

void FV_View::draw(int page, dg_DrawArgs* da)
{
	xxx_UT_DEBUGMSG(("FV_View::draw_1: [page %ld]\n",page));

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
	_draw(0,0,m_iWindowWidth,m_iWindowHeight,bDirtyRunsOnly,false);
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

	if ((width <= 0) || (height <= 0))
	{
		UT_DEBUGMSG(("fv_View::draw() called with zero width or height expose.\n"));
		return;
	}

	if (bClip)
	{
		UT_Rect r;

		r.left = x;
		r.top = y;
		r.width = width;
		r.height = height;

		m_pG->setClipRect(&r);
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

	UT_RGBColor clrMargin(127,127,127);		// dark gray

	if (!bDirtyRunsOnly)
	{
		if (m_xScrollOffset < fl_PAGEVIEW_MARGIN_X)
		{
			// fill left margin
			m_pG->fillRect(clrMargin, 0, 0, fl_PAGEVIEW_MARGIN_X - m_xScrollOffset, m_iWindowHeight);
		}

		if (m_yScrollOffset < fl_PAGEVIEW_MARGIN_Y)
		{
			// fill top margin
			m_pG->fillRect(clrMargin, 0, 0, m_iWindowWidth, fl_PAGEVIEW_MARGIN_Y - m_yScrollOffset);
		}
	}

	UT_sint32 curY = fl_PAGEVIEW_MARGIN_Y;
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		UT_sint32 iPageWidth		= pPage->getWidth();
		UT_sint32 iPageHeight		= pPage->getHeight();
		UT_sint32 adjustedTop		= curY - m_yScrollOffset;
		UT_sint32 adjustedBottom	= adjustedTop + iPageHeight + fl_PAGEVIEW_PAGE_SEP;
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
			// don't need to draw them either.  exit loop now.
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
			da.xoff = fl_PAGEVIEW_MARGIN_X - m_xScrollOffset;
			da.yoff = adjustedTop;

			UT_sint32 adjustedLeft	= fl_PAGEVIEW_MARGIN_X - m_xScrollOffset;
			UT_sint32 adjustedRight	= adjustedLeft + iPageWidth;

			adjustedBottom -= fl_PAGEVIEW_PAGE_SEP;

			if (!bDirtyRunsOnly || pPage->needsRedraw())
			{	
			  UT_RGBColor * pClr = pPage->getOwningSection()->getPaperColor();
			  m_pG->fillRect(*pClr,adjustedLeft+1,adjustedTop+1,iPageWidth-1,iPageHeight-1);
			}

			pPage->draw(&da);

			// draw page decorations
			UT_RGBColor clr(0,0,0);		// black
			m_pG->setColor(clr);

			// one pixel border
			m_pG->drawLine(adjustedLeft, adjustedTop, adjustedRight, adjustedTop);
			m_pG->drawLine(adjustedRight, adjustedTop, adjustedRight, adjustedBottom);
			m_pG->drawLine(adjustedRight, adjustedBottom, adjustedLeft, adjustedBottom);
			m_pG->drawLine(adjustedLeft, adjustedBottom, adjustedLeft, adjustedTop);

			// fill to right of page
			if (m_iWindowWidth - (adjustedRight + 1) > 0)
			{
				m_pG->fillRect(clrMargin, adjustedRight + 1, adjustedTop, m_iWindowWidth - (adjustedRight + 1), iPageHeight + 1);
			}

			// fill separator below page
			if (m_iWindowHeight - (adjustedBottom + 1) > 0)
			{
				m_pG->fillRect(clrMargin, adjustedLeft, adjustedBottom + 1, m_iWindowWidth - adjustedLeft, fl_PAGEVIEW_PAGE_SEP);
			}
			
			// two pixel drop shadow
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

		curY += iPageHeight + fl_PAGEVIEW_PAGE_SEP;

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

#if 0
	{
		// Some test code for the graphics interface.
		UT_RGBColor clrRed(255,0,0);
		m_pG->setColor(clrRed);
		m_pG->drawLine(10,10,20,10);
		m_pG->drawLine(20,11,30,11);
		m_pG->fillRect(clrRed,50,10,10,10);
		m_pG->fillRect(clrRed,60,20,10,10);
	}
#endif

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
		UT_sint32 iDocHeight = fl_PAGEVIEW_MARGIN_Y;
		while (pPage)
		{
			iDocHeight += pPage->getHeight() + fl_PAGEVIEW_PAGE_SEP;
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

void FV_View::cmdSelect(UT_sint32 xPos, UT_sint32 yPos, FV_DocPos dpBeg, FV_DocPos dpEnd)
{
	UT_DEBUGMSG(("SEVIOR: Double click on mouse \n"));
//
// Code to handle footer/header insertion 
//
	fp_Page * pPage = getCurrentPage();
	fl_DocSectionLayout * pDSL = pPage->getOwningSection();
	if(pPage->getHeaderP() == NULL)
	{
//
// No header. Look to see if the user has clicked in the header region.
//
		if(xPos >=0 && yPos >=0 && yPos < pDSL->getTopMargin())
		{
//
// Yes so insert a header put the cursor there and return
//
			insertHeaderFooter(false); // false for header
			return;
		}
	}
	if(pPage->getFooterP() == NULL)
	{
//
// No Footer. Look to see if the user has clicked in the footer region.
//
		UT_DEBUGMSG(("SEVIOR: ypos = %d pPage->getBottom() = %d pDSL->getBottomMargin() %d \n",yPos,pPage->getBottom(),pDSL->getBottomMargin()));
		if(xPos >=0 && yPos < (pPage->getBottom() + pDSL->getBottomMargin()) && yPos > pPage->getBottom())
		{
//
// Yes so insert a footer put the cursor there and return
//
			insertHeaderFooter(true); // true for footer
			return;
		}
	}

	warpInsPtToXY(xPos, yPos,true);

	//_eraseInsertionPoint();

	PT_DocPosition iPosLeft = _getDocPos(dpBeg, false);
	PT_DocPosition iPosRight = _getDocPos(dpEnd, false);

	cmdSelect (iPosLeft, iPosRight);
}

void FV_View::_setPoint(PT_DocPosition pt, bool bEOL)
{

	if (!m_pDoc->getAllowChangeInsPoint())
	{
		return;
	}
	m_iInsPoint = pt;
	m_bPointEOL = bEOL;
	if(m_pDoc->isPieceTableChanging() == false)
	{	
		m_pLayout->considerPendingSmartQuoteCandidate();
		_checkPendingWordForSpell();
	}
}

void FV_View::setPoint(PT_DocPosition pt)
{
	if (!m_pDoc->getAllowChangeInsPoint())
	{
		return;
	}
	m_iInsPoint = pt;
	if(m_pDoc->isPieceTableChanging() == false)
	{	
		m_pLayout->considerPendingSmartQuoteCandidate();
		_checkPendingWordForSpell();
	}
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

void FV_View::_checkPendingWordForSpell(void)
{
	if(m_pDoc->isPieceTableChanging() == true)
	{
		return;
	}
	// deal with pending word, if any
	if (m_pLayout->isPendingWordForSpell())
	{
		fl_BlockLayout* pBL = _findBlockAtPosition(m_iInsPoint);
		if (pBL)
		{
			UT_uint32 iOffset = m_iInsPoint - pBL->getPosition();

			if (!m_pLayout->touchesPendingWordForSpell(pBL, iOffset, 0))
			{
				// no longer there, so check it
				if (m_pLayout->checkPendingWordForSpell())
					updateScreen();
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
	}

	UT_ASSERT(bRes);

	if ((UT_sint32) m_iInsPoint < (UT_sint32) posBOD)
	{
		m_iInsPoint = posBOD;
		if (m_iInsPoint != posOld)
		{
			m_pLayout->considerPendingSmartQuoteCandidate();
			_checkPendingWordForSpell();
			_clearIfAtFmtMark(posOld);
			notifyListeners(AV_CHG_MOTION);
		}
		
		return false;
	}

	if ((UT_sint32) m_iInsPoint > (UT_sint32) posEOD)
	{
		m_iInsPoint = posEOD;

		if (m_iInsPoint != posOld)
		{
			m_pLayout->considerPendingSmartQuoteCandidate();
			_checkPendingWordForSpell();
			_clearIfAtFmtMark(posOld);
			notifyListeners(AV_CHG_MOTION);
		}

		return false;
	}

	if (m_iInsPoint != posOld)
	{
		m_pLayout->considerPendingSmartQuoteCandidate();
		_checkPendingWordForSpell();
		_clearIfAtFmtMark(posOld);
		notifyListeners(AV_CHG_MOTION);
	}

	return true;
}
// -------------------------------------------------------------------------

bool FV_View::canDo(bool bUndo) const
{
	return m_pDoc->canDo(bUndo);
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

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
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

	m_pDoc->redoCmd(count);

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	_generalUpdate();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	
	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
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

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	
	_fixInsertionPointCoords();
	_drawInsertionPoint();
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

void FV_View::cmdPaste(void)
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
	_doPaste(true);

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	m_pDoc->clearDoingPaste();

	m_pDoc->endUserAtomicGlob();
}

void FV_View::cmdPasteSelectionAt(UT_sint32 xPos, UT_sint32 yPos)
{
	// this is intended for the X11 middle mouse paste trick.
	//
	// if this view has the selection, we need to remember it
	// before we warp to the given (x,y) -- or else there won't
	// be a selection to paste when get there.  this is sort of
	// back door hack and should probably be re-thought.
	
	// set UAG markers around everything that the actual paste does
	// so that undo/redo will treat it as one step.
	
	m_pDoc->beginUserAtomicGlob();

	// Signal PieceTable Change
	m_pDoc->notifyPieceTableChangeStart();

	if (!isSelectionEmpty())
		m_pApp->cacheCurrentSelection(this);
	warpInsPtToXY(xPos,yPos,true);
	_doPaste(false);
	m_pApp->cacheCurrentSelection(NULL);

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

	m_pDoc->endUserAtomicGlob();
}

void FV_View::_doPaste(bool bUseClipboard)
{
	// internal portion of paste operation.
	
	if (!isSelectionEmpty())
		_deleteSelection();
	else
		_eraseInsertionPoint();

	_clearIfAtFmtMark(getPoint());
	PD_DocumentRange dr(m_pDoc,getPoint(),getPoint());
	m_pApp->pasteFromClipboard(&dr,bUseClipboard);

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
	m_pDoc->notifyPieceTableChangeStart();
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
	m_pDoc->notifyPieceTableChangeEnd();

	_fixInsertionPointCoords();
	if (isSelectionEmpty())
	{
		_drawInsertionPoint();
	}

	return bRet;
}

/*****************************************************************/
/*****************************************************************/

void FV_View::getTopRulerInfo(AP_TopRulerInfo * pInfo)
{
	memset(pInfo,0,sizeof(*pInfo));

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
		pInfo->m_xPaperSize = m_pG->convertDimension("8.5in"); // TODO eliminate this constant
		pInfo->m_xPageViewMargin = fl_PAGEVIEW_MARGIN_X;

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

		UT_ASSERT(pRun);
		///
		/// Bug Here!! Can be triggered by doing stuff in header/footer
		/// region. Try to recover..
		///
		if(!pRun)
		{
			PT_DocPosition posEOD = 0;
			getEditableBounds(true,posEOD);
			_findPositionCoords(posEOD, m_bPointEOL, xCaret, yCaret, xCaret2, yCaret2, heightCaret, bDirection, &pBlock, &pRun);
		}
		UT_ASSERT(pRun->getLine());

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
			fl_HdrFtrSectionLayout * pHF =  m_pEditShadow->getHdrFtrSectionLayout();
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

/*****************************************************************/
UT_Error FV_View::cmdInsertField(const char* szName)
{
	bool bResult;
	const XML_Char*	attributes[] = {
		"type", szName,
		NULL, NULL
	};

/*
  currently unused
  fl_BlockLayout* pBL = _findBlockAtPosition(getPoint());
*/

	// Signal PieceTable Change
	m_pDoc->notifyPieceTableChangeStart();

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
		bResult = m_pDoc->insertObject(getPoint(), PTO_Field, attributes, NULL, &pField);
		if(pField != NULL)
		{
			pField->update();
		}
	}
	_generalUpdate();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

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

	double fDPI = m_pG->getResolution() * 100 / m_pG->getZoomPercentage();
	return pFG->insertIntoDocument(m_pDoc, fDPI, getPoint(), szName);
}

UT_Error FV_View::cmdInsertGraphic(FG_Graphic* pFG, const char* pszName)
{
	bool bDidGlob = false;

	// Signal PieceTable Change
	m_pDoc->notifyPieceTableChangeStart();

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
	char szName[GR_IMAGE_MAX_NAME_LEN + 10 + 1];
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
	m_pDoc->notifyPieceTableChangeEnd();

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

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

	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);
	if (!pPage)
		return EV_EMC_UNKNOWN;

	if (   (yClick < 0)
		   || (xClick < 0)
		   || (xClick > pPage->getWidth()) )
	{
		return EV_EMC_UNKNOWN;
	}

	if (isLeftMargin(xPos,yPos))
	{
		return EV_EMC_LEFTOFTEXT;
	}

	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL);
	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);
	if (!pBlock)
		return EV_EMC_UNKNOWN;
	fp_Run* pRun = pBlock->findPointCoords(pos, bEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);
	
	if (!pRun)
		return EV_EMC_UNKNOWN;

	switch (pRun->getType())
	{
	case FPRUN_TEXT:
		if (!isPosSelected(pos))
			if (pBlock->getSquiggle(pos - pBlock->getPosition()))
				return EV_EMC_MISSPELLEDTEXT;

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

EV_EditMouseContext FV_View::getInsertionPointContext(UT_sint32 * pxPos, UT_sint32 * pyPos)
{
	// compute an EV_EMC_ context for the position
	// of the current insertion point and return
	// the window coordinates of the insertion point.
	// this is to allow a keyboard binding to raise
	// a context menu.

	EV_EditMouseContext emc = EV_EMC_TEXT;

	// TODO compute the correct context based upon the
	// TODO current insertion point and/or the current
	// TODO selection region.
	
	if (pxPos)
		*pxPos = m_xPoint;
	if (pyPos)
		*pyPos = m_yPoint + m_iPointHeight;
	return emc;
}

fp_Page* FV_View::getCurrentPage(void) const
{
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
	UT_uint32 pos = getPoint();
	UT_sint32 xPoint2, yPoint2;
	bool bDirection;
	
	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);
	UT_ASSERT(pBlock);
	fp_Run* pRun = pBlock->findPointCoords(pos, m_bPointEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);

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

UT_uint32 FV_View::getCurrentPageNumForStatusBar(void) const
{
	fp_Page* pCurrentPage = getCurrentPage();
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

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return 0;
}

void FV_View::_clearIfAtFmtMark(PT_DocPosition dpos)
{
	m_pDoc->clearIfAtFmtMark(dpos);
	_generalUpdate();
}

// ndx is one-based, not zero-based
UT_UCSChar * FV_View::getContextSuggest(UT_uint32 ndx)
{
	// locate the squiggle
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBL = _findBlockAtPosition(pos);
	UT_ASSERT(pBL);
	fl_PartOfBlock* pPOB = pBL->getSquiggle(pos - pBL->getPosition());
	UT_ASSERT(pPOB);

	// grab the suggestion
	return _lookupSuggestion(pBL, pPOB, ndx);
}

// NB: returns a UCS string that the caller needs to FREEP
UT_UCSChar * FV_View::_lookupSuggestion(fl_BlockLayout* pBL, fl_PartOfBlock* pPOB, UT_uint32 ndx)
{
	// grab a copy of the word
	UT_GrowBuf pgb(1024);
	bool bRes = pBL->getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar * pWord = pgb.getPointer(pPOB->iOffset);
	UT_UCSChar * szSuggest = NULL;

	// lookup suggestions
	sp_suggestions sg;
	memset(&sg, 0, sizeof(sg));

	UT_UCSChar theWord[101];
	// convert smart quote apostrophe to ASCII single quote to be compatible with ispell
	for (UT_uint32 ldex=0; ldex<pPOB->iLength && ldex<100; ++ldex)
	{
		UT_UCSChar currentChar;
		currentChar = *(pWord + ldex);
		if (currentChar == UCS_RQUOTE) currentChar = '\'';
		theWord[ldex] = currentChar;
	}
				
	SpellCheckSuggestNWord16(theWord, pPOB->iLength, &sg);

	// we currently return all requested suggestions
	// TODO: prune lower-weighted ones??
	if ((sg.count) &&
		((int) ndx <= sg.count))
	{
		UT_UCS_cloneString(&szSuggest, (UT_UCSChar *) sg.word[ndx-1]);
	}

	// clean up
	for (int i = 0; i < sg.count; i++)
		FREEP(sg.word[i]);
	FREEP(sg.word);
	FREEP(sg.score);

	return szSuggest;
}

void FV_View::cmdContextSuggest(UT_uint32 ndx)
{
	// locate the squiggle
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBL = _findBlockAtPosition(pos);
	UT_ASSERT(pBL);
	fl_PartOfBlock* pPOB = pBL->getSquiggle(pos - pBL->getPosition());
	UT_ASSERT(pPOB);

	// grab the suggestion
	UT_UCSChar * replace = _lookupSuggestion(pBL, pPOB, ndx);

	// make the change
	UT_ASSERT(isSelectionEmpty());

	moveInsPtTo((PT_DocPosition) (pBL->getPosition() + pPOB->iOffset));
	extSelHorizontal(true, pPOB->iLength);
	cmdCharInsert(replace, UT_UCS_strlen(replace));

	FREEP(replace);
}

void FV_View::cmdContextIgnoreAll(void)
{
	// locate the squiggle
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBL = _findBlockAtPosition(pos);
	UT_ASSERT(pBL);
	fl_PartOfBlock* pPOB = pBL->getSquiggle(pos - pBL->getPosition());
	UT_ASSERT(pPOB);

	// grab a copy of the word
	UT_GrowBuf pgb(1024);
	bool bRes = pBL->getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar * pBuf = pgb.getPointer(pPOB->iOffset);

	// make the change
	if (m_pDoc->appendIgnore(pBuf, pPOB->iLength))
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
	fl_PartOfBlock* pPOB = pBL->getSquiggle(pos - pBL->getPosition());
	UT_ASSERT(pPOB);

	// grab a copy of the word
	UT_GrowBuf pgb(1024);
	bool bRes = pBL->getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar * pBuf = pgb.getPointer(pPOB->iOffset);

	// make the change
	if (m_pApp->addWordToDict(pBuf, pPOB->iLength))
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


/*static*/ void FV_View::_prefsListener( XAP_App * /*pApp*/, XAP_Prefs *pPrefs, UT_AlphaHashTable * /*phChanges*/, void *data )
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

		// if currently blinking, turn it off
		if ( pView->m_bCursorBlink == false && pView->m_pAutoCursorTimer )
			pView->m_pAutoCursorTimer->stop();

		// this is an attempt for force the cursors to draw, don't know if it actually helps
		if ( !pView->m_bCursorBlink && pView->m_bCursorIsOn )
			pView->_drawInsertionPoint();

		pView->_updateInsertionPoint();
	}
#ifdef BIDI_ENABLED	
	if ( pPrefs->getPrefsValueBool((XML_Char*)AP_PREF_KEY_DefaultDirectionRtl, &b) && b != pView->m_bDefaultDirectionRtl)
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
		const char *pMsg1 = pSS->getValue(AP_STRING_ID_MSG_DefaultDirectionChg);
		const char *pMsg2 = pSS->getValue(AP_STRING_ID_MSG_AfterRestartNew);
		
		UT_ASSERT((pMsg1 && pMsg2));
		
		UT_uint32 len = strlen(pMsg1) + strlen(pMsg2) + 2;
		
		char * szMsg = new char[len];
		UT_ASSERT((szMsg));
		
		sprintf(szMsg, "%s\n%s", pMsg1, pMsg2);
		
		pFrame->showMessageBox(szMsg, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);

		delete[](szMsg);
	
		/*
		  UT_DEBUGMSG(("View: Resetting default direction to %s\n", b ?"rtl" :"ltr"));
		
		  if(b)
		  {
		  const XML_Char bidi_dir_name[] = "dir";
		  const XML_Char bidi_dir_value[] = "rtl";
		  const XML_Char bidi_domdir_name[] = "dom-dir";
		  const XML_Char bidi_align_name[] = "text-align";
		  const XML_Char bidi_align_value[] = "right";
			
		  const XML_Char * bidi_props[7]= {bidi_dir_name, bidi_dir_value, bidi_domdir_name, bidi_dir_value, bidi_align_name, bidi_align_value,0};
		  UT_DEBUGMSG(("calling setStyleProperties ... "));
		  pView->m_pDoc->setStyleProperties((const XML_Char*)"Normal", (const XML_Char**)bidi_props);
		  UT_DEBUGMSG(("done.\n"));			
		  }
		  else
		  {
		  const XML_Char bidi_dir_name[] = "dir";
		  const XML_Char bidi_dir_value[] = "ltr";
		  const XML_Char bidi_domdir_name[] = "dom-dir";
		  const XML_Char bidi_align_name[] = "text-align";
		  const XML_Char bidi_align_value[] = "left";
			
		  const XML_Char * bidi_props[7]= {bidi_dir_name, bidi_dir_value, bidi_domdir_name, bidi_dir_value, bidi_align_name, bidi_align_value,0};
		  UT_DEBUGMSG(("calling setStyleProperties ... "));
		  pView->m_pDoc->setStyleProperties((const XML_Char*)"Normal", (const XML_Char**)bidi_props);
		  UT_DEBUGMSG(("done.\n"));			
		  }
		
		  UT_DEBUGMSG(("calling PP_resetInitialBiDiValues ..."));
		  PP_resetInitialBiDiValues(b ?"rtl" :"ltr");
		  UT_DEBUGMSG(("done.\n"));			
		  pView->m_bDefaultDirectionRtl = b;
		  pView->_generalUpdate();
		*/
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

FV_DocCount FV_View::countWords(void)
{
	FV_DocCount wCount;
	memset(&wCount,0,sizeof(wCount));

	bool isPara = false;

	fl_SectionLayout * pSL = m_pLayout->getFirstSection();
	while (pSL)
	{
		fl_BlockLayout * pBL = pSL->getFirstBlock();
		while (pBL)
		{
			UT_GrowBuf gb(1024);
			pBL->getBlockBuf(&gb);
			const UT_UCSChar * pSpan = gb.getPointer(0);
			UT_uint32 len = gb.getLength();
			
			// count words in pSpan[0..len]
			UT_uint32 i;
			bool newWord = false;
			bool delim = true;
			for (i = 0; i < len; i++)
			{
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
				UT_UCSChar followChar;
				followChar = (i+1 < len) ? pSpan[i+1] : UCS_UNKPUNK;
				newWord = (delim && !UT_isWordDelimiter(pSpan[i], followChar));
				
				delim = UT_isWordDelimiter(pSpan[i], followChar);
				
				/*
				  CJK-FIXME: this can work incorrectly under CJK locales since it can
				  give 'true' for UCS with value >0xff (like quotes, etc).
				*/
				if (newWord || XAP_EncodingManager::instance->is_cjk_letter(pSpan[i]))
					wCount.word++;
			
			}

		
			// count lines

			fp_Line * pLine = pBL->getFirstLine();
			while (pLine)
			{
				wCount.line++;
				pLine = pLine->getNext();
			}
			
			if (isPara)
			{
				wCount.para++;
				isPara = false;
			}
	
			pBL = pBL->getNext();
		}
		pSL = pSL->getNext();

	}
	// count pages
	wCount.page = m_pLayout->countPages();
	
	return (wCount);
}

void FV_View::setShowPara(bool bShowPara)
{
	if (bShowPara != m_bShowPara)
	{
		m_bShowPara = bShowPara;
		draw();
	}
};

/*!
 *  This method sets a bool variable which tells getEditableBounds we are
 *  editting a header/Footer.
 *  \param  pSectionLayout pointer to the SectionLayout being editted.
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
 *  This method sets a bool variable which tells getEditableBounds we are
 *  editting a header/Footer.
 *  \param  pSectionLayout pointer to the SectionLayout being editted.
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
 *   Returns the pointer to the current shadow.
 */
fl_HdrFtrShadow *  FV_View::getEditShadow(void)
{
	return m_pEditShadow;
}

/*!
 *   Returns true if we're currently edditting a HdrFtr section.
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

   \param   isEnd true to get the end of the document. False gets the beginning
   \param   posEnd is the value of the doc position at the beginning and end 
            of the doc
   \param   bOveride if true the EOD is made within the edittable region
   \return  true if succesful
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
		pSL = (fl_SectionLayout *)  m_pLayout->getLastSection();
		while(pSL->getNext() != NULL && pSL->getType() == FL_SECTION_DOC)
		{
			pSL  = pSL->getNext();
		}
		if( pSL->getType() == FL_SECTION_DOC)
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
	PT_DocPosition posDocEnd;
	m_pDoc->getBounds(isEnd,posDocEnd);
	fp_Run * pRun = pBL->getFirstRun();
	while( pRun->getNext() != NULL)
	{
		pRun = pRun->getNext();
	}
	posEOD += pRun->getBlockOffset();
	while(_findBlockAtPosition(posEOD) == pBL && posEOD <= posDocEnd)
		posEOD++;
	posEOD--;
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
	fp_HdrFtrContainer * pHFCon = NULL;
	pHFCon = pPage->getHeaderP();
	if(pHFCon == NULL)
	{
		insertHeaderFooter(false);
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
	fp_HdrFtrContainer * pHFCon = NULL;
	pHFCon = pPage->getFooterP();
	if(pHFCon == NULL)
	{
		insertHeaderFooter(true);
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


void FV_View::insertHeaderFooter(bool ftr)
{
	const XML_Char*	block_props[] = {
		"text-align", "left",
		NULL, NULL
	};
//
// insert the header/footer and leave the cursor in there. Set us in header/footer
//	edit mode.
//
	if(isHdrFtrEdit())
		clearHdrFtrEdit();
	m_pDoc->beginUserAtomicGlob(); // Begin the big undo block

	// Signal PieceTable Changes have Started
	m_pDoc->notifyPieceTableChangeStart();
	m_pDoc->disableListUpdates();

	insertHeaderFooter(block_props, ftr); // cursor is now in the header/footer

	
	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have Ended

	m_pDoc->notifyPieceTableChangeEnd();

	m_pDoc->endUserAtomicGlob(); // End the big undo block

// Update Layout everywhere. This actually creates the header/footer container
//	m_pLayout->updateLayout(); // Update document layout everywhere
//	updateScreen();
//
// Now extract the shadow section from this.
//
	fp_Page * pPage = getCurrentPage();
	fl_HdrFtrShadow * pShadow = NULL;
	fp_HdrFtrContainer * pHFCon = NULL;
	if(ftr)
		pHFCon = pPage->getFooterP();
	else
		pHFCon = pPage->getHeaderP();
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


bool FV_View::insertHeaderFooter(const XML_Char ** props, bool ftr)
{

	/*
	  This inserts a header/footer at the end of the document,
	  and leaves the insertion point there.
	  This provides NO undo stuff.  Do it yourself.
	*/

	const XML_Char* szString = ftr ? "footer" : "header";

	// TODO: This stuff shouldn't be hardcoded
	// TODO: The fact that it is hardcoded means that only
	// TODO: one section can have footers at one time, currently

	static XML_Char sid[15];
	UT_uint32 id = 0;
	while(id < AUTO_LIST_RESERVED)
		id = rand();
	sprintf(sid, "%i", id);

	const XML_Char*	sec_attributes1[] = {
		"type", szString,
		"id",sid,"listid","0","parentid","0",
		NULL, NULL
	};

	const XML_Char*	sec_attributes2[] = {
		szString, sid,
		NULL, NULL
	};


	const XML_Char*	block_props[] = {
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
	fp_Page* pCurrentPage = getCurrentPage();
	fl_DocSectionLayout * pDocL = pCurrentPage->getOwningSection();
//
// Now find the position of this section
//
	fl_BlockLayout * pBL = pDocL->getFirstBlock();
	PT_DocPosition posSec = pBL->getPosition();

	// change the section to point to the footer which doesn't exist yet.

	m_pDoc->changeStruxFmt(PTC_AddFmt, posSec, posSec, sec_attributes2, NULL, PTX_Section);

	moveInsPtTo(FV_DOCPOS_EOD);	// Move to the end, where we will create the page numbers

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
// Next set the style to Normal so weird properties at the end of the doc aren't
// inherited into the header.
//
	setStyle("Normal",true);
//
// This moves the insertion point and screws up other commands to the piecetable
// so put them back
//
//	m_pDoc->notifyPieceTableChangeStart();
//	m_pDoc->disableListUpdates();

//	if(isSelectionEmpty())
//		_eraseInsertionPoint();

	//
	// Now Insert the footer section.
	// Doing things this way will grab the previously inserted block
	// and put into the footer section.

	m_pDoc->insertStrux(iPoint, PTX_SectionHdrFtr);
	
//
// Give the Footer section the properties it needs to attach itself to the
// correct DocSectionLayout.
//

      	m_pDoc->changeStruxFmt(PTC_AddFmt, getPoint(), getPoint(), sec_attributes1, NULL, PTX_SectionHdrFtr);

	// Change the formatting of the new footer appropriately 
    //(currently just center it)

	m_pDoc->changeStruxFmt(PTC_AddFmt, getPoint(), getPoint(), NULL, props, PTX_Block);

// OK it's in!

	return true;
}

bool FV_View::insertPageNum(const XML_Char ** props, bool ftr)
{
	
	/*
	  This code implements some hardcoded hacks to insert a page number.
	  It allows you to set the properties, but nothing else.  Use that
	  to center, etc.

	  Warning: this code assumes that _insertFooter() leaves the insertion
	  point in a place where it can write the page_num field.
	*/

	const XML_Char*	f_attributes[] = {
		"type", "page_number",
		NULL, NULL
	};

	m_pDoc->beginUserAtomicGlob(); // Begin the big undo block

	// Signal PieceTable Changes have Started
	m_pDoc->notifyPieceTableChangeStart();
	m_pDoc->disableListUpdates();

	_eraseInsertionPoint();

	UT_uint32 oldPos = getPoint();	// This ends up being redundant, but it's neccessary
	bool bResult = insertHeaderFooter(props, ftr);

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

	// Signal PieceTable Changes have Ended

	m_pDoc->notifyPieceTableChangeEnd();
	_generalUpdate();
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
	double pageWidth = pageSize.Width(fp_PageSize::inch);
	
	// Set graphics zoom to 100 so we can get the display resolution.
	GR_Graphics *pG = getGraphics();
	UT_uint32 temp_zoom = pG->getZoomPercentage();
	pG->setZoomPercentage(100);
	UT_uint32 resolution = pG->getResolution();
	pG->setZoomPercentage(temp_zoom);

	// Verify scale as a positive non-zero number else return old zoom
	if ( ( getWindowWidth() - 2 * fl_PAGEVIEW_MARGIN_X ) <= 0 )
		return temp_zoom;

	double scale = (double)(getWindowWidth() - 2 * fl_PAGEVIEW_MARGIN_X) / 
		(pageWidth * (double)resolution);
	return (UT_uint32)(scale * 100.0);
}

UT_uint32 FV_View::calculateZoomPercentForPageHeight()
{

	const fp_PageSize pageSize = getPageSize();
	double pageHeight = pageSize.Height(fp_PageSize::inch);
	
	// Set graphics zoom to 100 so we can get the display resolution.
	GR_Graphics *pG = getGraphics();
	UT_uint32 temp_zoom = pG->getZoomPercentage();
	pG->setZoomPercentage(100);
	UT_uint32 resolution = pG->getResolution();
	pG->setZoomPercentage(temp_zoom);

	// Verify scale as a positive non-zero number else return old zoom
	if ( ( getWindowHeight() - 2 * fl_PAGEVIEW_MARGIN_Y ) <= 0 )
		return temp_zoom;

	double scale = (double)(getWindowHeight() - 2 * fl_PAGEVIEW_MARGIN_Y) /
		(pageHeight * (double)resolution);
	return (UT_uint32)(scale * 100.0);
}

UT_uint32 FV_View::calculateZoomPercentForWholePage()
{
	return MyMin(	calculateZoomPercentForPageWidth(),
					calculateZoomPercentForPageHeight());
}
