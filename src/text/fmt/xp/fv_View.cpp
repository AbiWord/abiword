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
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fl_Squiggles.h"
#include "fl_SectionLayout.h"
#include "fl_FootnoteLayout.h"
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
#include "fp_TableContainer.h"
#include "fl_TableLayout.h"
#include "xap_Dlg_Zoom.h"
#include "ap_Frame.h"
#include "ap_FrameData.h"
#include "fp_FrameContainer.h"
#include "xap_EncodingManager.h"
#include "gr_Painter.h"

#include "pp_Revision.h"

#include "fv_View.h"

// NB -- irrespective of this size, the piecetable will store
// at max BOOKMARK_NAME_LIMIT of chars as defined in pf_Frag_Bookmark.h
#define BOOKMARK_NAME_SIZE 30
#define CHECK_WINDOW_SIZE if(getWindowHeight() < m_pG->tlu(20)) return;

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

class FV_Caret_Listener : public AV_Listener
{
public:
  FV_Caret_Listener(XAP_Frame * pFrame) : m_pFrame (pFrame) {}
  virtual ~FV_Caret_Listener (){}

  virtual bool notify(AV_View * pView, const AV_ChangeMask mask)
  {
	  GR_Graphics *pG = static_cast<FV_View *>(pView)->getGraphics();
	  if (m_pFrame && (mask & (AV_CHG_INSERTMODE)))
      {
		  AP_FrameData * pData = static_cast<AP_FrameData *>(m_pFrame->getFrameData());
		  if (pData) 
		  {
			  pG->getCaret()->setInsertMode(pData->m_bInsertMode);
			  return true;
		  }
      }
	  return false;
  }

  virtual AV_ListenerType    getType(void) {return AV_LISTENER_CARET;}

private:
  XAP_Frame   * m_pFrame;
  GR_Graphics * m_pGraphics;
};

/****************************************************************/

FV_View::FV_View(XAP_App * pApp, void* pParentData, FL_DocLayout* pLayout)
	:	AV_View(pApp, pParentData),
		m_iInsPoint(0),
		m_xPoint(0),
		m_yPoint(0),
		m_xPoint2(0),
		m_yPoint2(0),
		m_bPointDirection(false) /* now what semantics is this? */,
		m_bDefaultDirectionRtl(false),
		m_bUseHebrewContextGlyphs(false),
		m_iPointHeight(0),
		m_xPointSticky(0),
		m_bPointVisible(false),
		m_bPointEOL(false),
		m_pLayout(pLayout),
		m_pDoc(pLayout->getDocument()),
		m_pG(m_pLayout->getGraphics()),
		m_pParentData(pParentData),
		m_pAutoScrollTimer(0),
		m_xLastMouse(0),
		m_yLastMouse(0),
		m_bCursorIsOn(false),
		m_bEraseSaysStopBlinking(false),
		m_wrappedEnd(false),
		m_startPosition(0),
		m_doneFind(false),
		m_bEditHdrFtr(false),
		m_pEditShadow(NULL),
		m_iSavedPosition(0),
		m_bNeedSavedPosition(false),
		m_bReverseFind(false),
		m_bWholeWord(false),
		m_bMatchCase(false),
		m_sFind(0),
		m_sReplace(0),
		m_bShowPara(false),
		m_viewMode(VIEW_PRINT),
		m_previewMode(PREVIEW_NONE),
		m_bDontUpdateScreenOnGeneralUpdate(false),
		m_iPieceTableState(0),
		m_iMouseX(0),
		m_iMouseY(0),
		m_iViewRevision(0),
		m_bWarnedThatRestartNeeded(false),
		m_selImageRect(-1,-1,-1,-1),
		m_iImageSelBoxSize(m_pG->tlu(10)),
		m_imageSelCursor(GR_Graphics::GR_CURSOR_IBEAM),
		m_ixResizeOrigin(0),
		m_iyResizeOrigin(0),
		m_bIsResizingImage(false),
		m_curImageSel(-1,-1,-1,-1),
		m_bIsDraggingImage(false),
		m_pDraggedImageRun(NULL),
		m_dragImageRect(-1,-1,-1,-1),
		m_ixDragOrigin(0),
		m_iyDragOrigin(0),
		m_colorShowPara(127,127,127),
		m_colorSquiggle(255, 0, 0),
		m_colorMargin(127, 127, 127),
		m_colorFieldOffset(10, 10, 10),
		m_colorImage(0, 0, 255),
		m_colorImageResize(0, 0, 0),
		m_colorHyperLink(0, 0, 255),
		m_colorHdrFtr(0, 0, 0),
		m_colorColumnLine(0, 0, 0),
		m_countDisable(0),
		m_bDragTableLine(false),
		m_prevMouseContext(EV_EMC_UNKNOWN),
		m_pTopRuler(NULL),
		m_pLeftRuler(NULL),
		m_bInFootnote(false),
		m_bgColorInitted(false),
		m_iLowDrawPoint(0),
		m_iHighDrawPoint(0),
		m_CaretListID(0),
		m_FrameEdit(this),
		m_VisualDragText(this),
		m_Selection(this),
		m_bShowRevisions(true)
{
	m_colorRevisions[0] = UT_RGBColor(171,4,254);
	m_colorRevisions[1] = UT_RGBColor(171,20,119);
	m_colorRevisions[2] = UT_RGBColor(255,151,8);
	m_colorRevisions[3] = UT_RGBColor(158,179,69);
	m_colorRevisions[4] = UT_RGBColor(15,179,5);
	m_colorRevisions[5] = UT_RGBColor(8,179,248);
	m_colorRevisions[6] = UT_RGBColor(4,206,195);
	m_colorRevisions[7] = UT_RGBColor(4,133,195);
	m_colorRevisions[8] = UT_RGBColor(7,18,195);
	m_colorRevisions[9] = UT_RGBColor(255,0,0);	// catch-all

	// initialize prefs cache
	pApp->getPrefsValueBool(AP_PREF_KEY_CursorBlink, &m_bCursorBlink);

   	const XML_Char * pszTmpColor = NULL;
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForShowPara), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorShowPara);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForSquiggle), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorSquiggle);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForMargin), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorMargin);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForFieldOffset), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorFieldOffset);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForImage), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorImage);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForHyperLink), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorHyperLink);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForHdrFtr), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorHdrFtr);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForColumnLine), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorColumnLine);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForRevision1), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[0]);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForRevision2), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[1]);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForRevision3), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[2]);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForRevision4), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[3]);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForRevision5), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[4]);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForRevision6), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[5]);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForRevision7), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[6]);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForRevision8), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[7]);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForRevision9), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[8]);
	}
	if (pApp->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForRevision10), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[9]);
	}

	// initialize prefs listener
	pApp->getPrefs()->addListener( _prefsListener, this );

	// Get View Mode
	if(m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		const char * szViewMode = NULL;
		pApp->getPrefsValue(static_cast<const char *>(AP_PREF_KEY_LayoutMode),&szViewMode);
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
		setCursorWait();
	}

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

		m_pDoc->addStyleProperties(static_cast<const XML_Char*>("Normal"), static_cast<const XML_Char**>(&bidi_props[0]));
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

		m_pDoc->addStyleProperties(static_cast<const XML_Char*>("Normal"), static_cast<const XML_Char**>(bidi_props));
		PP_resetInitialBiDiValues("ltr");
	}
#endif

	// findFont will do a fuzzy match, and return the nearest font in the system
	GR_Font* pFont = m_pG->findFont("Times New Roman", "normal", "", "normal", "", "12pt");
	const char * pszFamily = pFont->getFamily();
	if (pszFamily)
		PP_setDefaultFontFamily(pszFamily);

	// should we display revisions?
	m_bShowRevisions = m_pDoc->isShowRevisions();
	m_iViewRevision =  m_pDoc->getShowRevisionId();
	
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

	m_Selection.setSelectionAnchor(getPoint());
	_resetSelection();
//
// Update the combo boxes on the frame with this documents info.
//
	m_caretListener = NULL;
	XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
	if( pFrame )
	{
	    pFrame->repopulateCombos();
	    m_pG->createCaret();
		m_pG->getCaret()->enable();
		if(m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
		{
			m_caretListener = new FV_Caret_Listener (pFrame);
			addListener(m_caretListener, &m_CaretListID);
		}
		else
		{
			m_caretListener = NULL;
		}
	}
}

FV_View::~FV_View()
{
	// remove prefs listener
	m_pApp->getPrefs()->removeListener( _prefsListener, this );

	DELETEP(m_pAutoScrollTimer);
	if(m_caretListener != NULL)
	{
		DELETEP(m_caretListener);
	}

	FREEP(m_sFind);
	FREEP(m_sReplace);

	FREEP(m_chg.propsChar);
	FREEP(m_chg.propsBlock);
	FREEP(m_chg.propsSection);
}

void FV_View::setGraphics(GR_Graphics * pG)
{
	if(m_caretListener != NULL)
	{
		removeListener(m_CaretListID);
		DELETEP(m_caretListener);
	}
	m_pG = pG;
	if(m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		m_pG->createCaret();
		m_pG->getCaret()->enable();
		XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
		m_caretListener = new FV_Caret_Listener (pFrame);
		addListener(m_caretListener, &m_CaretListID);
	}
	else
	{
		m_caretListener = NULL;
	}
}

//-------------------------
// Visual Drag stuff
//
void FV_View::cutVisualText(UT_sint32 x, UT_sint32 y)
{
//
// Do nothing for now. Only cut on first drag event.
//	
}

void FV_View::copyVisualText(UT_sint32 x, UT_sint32 y)
{
	m_VisualDragText.mouseCopy(x,y);

}


void FV_View::dragVisualText(UT_sint32 x, UT_sint32 y)
{
	m_VisualDragText.mouseDrag(x,y);
}


void FV_View::pasteVisualText(UT_sint32 x, UT_sint32 y)
{
	m_VisualDragText.mouseRelease(x,y);
}



FV_FrameEdit * FV_View::getFrameEdit(void)
{
	return &m_FrameEdit;
}

/*!
 * Logic for determining what state the frame and cursor should be in.
 */
void FV_View::btn0Frame(UT_sint32 x, UT_sint32 y)
{
	xxx_UT_DEBUGMSG(("btn0 called frameEdit mode %d \n",m_FrameEdit.getFrameEditMode()));
	if(!m_FrameEdit.isActive())
	{
		getGraphics()->setCursor(GR_Graphics::GR_CURSOR_GRAB);
		return;
	}
	else if(m_FrameEdit.getFrameEditMode() == FV_FrameEdit_WAIT_FOR_FIRST_CLICK_INSERT)
	{
		getGraphics()->setCursor(GR_Graphics::GR_CURSOR_CROSSHAIR);
	}
	else if(m_FrameEdit.getFrameEditMode() == FV_FrameEdit_EXISTING_SELECTED)
	{
		m_FrameEdit.setDragType(x,y,false);
		setCursorToContext();
	}
}


void FV_View::btn1Frame(UT_sint32 x, UT_sint32 y)
{
	m_FrameEdit.mouseLeftPress(x,y);
}

void FV_View::setFrameFormat(const XML_Char * properties[])
{
	UT_String dataID("");
	setFrameFormat(properties,NULL,dataID);
}

void FV_View::setFrameFormat(const XML_Char * properties[], FG_Graphic * pFG,UT_String & sDataID)
{
	bool bRet;
	setCursorWait();
	//
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();
	if(isHdrFtrEdit())
	{
		clearHdrFtrEdit();
		warpInsPtToXY(0,0,false);
	}

	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_Selection.getSelectionAnchor() < posStart)
			posStart = m_Selection.getSelectionAnchor();
		else
			posEnd = m_Selection.getSelectionAnchor();
		if(posStart < 2)
		{
			posStart = 2;
		}
	}
	if(pFG != NULL)
	{
		pFG->insertAtStrux(m_pDoc,72,posStart,
						   PTX_SectionFrame,sDataID.c_str());
	}
	else
	{
		const XML_Char * attributes[3] = {
			PT_STRUX_IMAGE_DATAID,NULL,NULL};
		bRet = m_pDoc->changeStruxFmt(PTC_RemoveFmt,posStart,posStart,attributes,NULL,PTX_SectionFrame);	
						
	}

	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,posStart,posEnd,NULL,properties,PTX_SectionFrame);

	_generalUpdate();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_generalUpdate();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_ensureInsertionPointOnScreen();
	clearCursorWait();
	notifyListeners(AV_CHG_MOTION);
}

void FV_View::dragFrame(UT_sint32 x, UT_sint32 y)
{
	m_FrameEdit.mouseDrag(x,y);
}


void FV_View::releaseFrame(UT_sint32 x, UT_sint32 y)
{
	m_FrameEdit.mouseRelease(x,y);
}

void FV_View::deleteFrame(void)
{
	m_FrameEdit.deleteFrame();
	setCursorToContext();
}

fl_FrameLayout * FV_View::getFrameLayout(void)
{
	fl_BlockLayout* pBlock = _findBlockAtPosition(getPoint());

	if(pBlock)
	{
		fl_ContainerLayout * pCL = pBlock->myContainingLayout();
		while(pCL && (pCL->getContainerType() != FL_CONTAINER_FRAME) && (pCL->getContainerType() != FL_CONTAINER_DOCSECTION))
		{
			pCL = pCL->myContainingLayout();
		}
		if(pCL == NULL)
		{
			return NULL;
		}
		if(pCL->getContainerType() != FL_CONTAINER_FRAME)
		{
			return NULL;
		}
		return static_cast<fl_FrameLayout *>(pCL);
	}
	return NULL;
}


/*!
 * Returns true if the supplied Doc Position is inside a frame.
 */
bool FV_View::isInFrame(PT_DocPosition pos)
{
	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);

	if(pBlock)
	{
		fl_ContainerLayout * pCL = pBlock->myContainingLayout();
		while(pCL && (pCL->getContainerType() != FL_CONTAINER_FRAME) && (pCL->getContainerType() != FL_CONTAINER_DOCSECTION))
		{
			pCL = pCL->myContainingLayout();
		}
		if(pCL == NULL)
		{
			return false;
		}
		if(pCL->getContainerType() != FL_CONTAINER_FRAME)
		{
			return false;
		}
		return true;
	}
	return false;
}

UT_RGBColor FV_View::getColorSelBackground ()
{
  static UT_RGBColor bgcolor (192, 192, 192);

  XAP_Frame * pFrame = 0;
  
  if ((pFrame = static_cast<XAP_Frame*>(getParentData())) != NULL)
    return pFrame->getColorSelBackground ();
  
  if (!m_bgColorInitted) {
    const XML_Char * pszTmpColor = NULL;
    if (XAP_App::getApp()->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForSelBackground), &pszTmpColor))
      {
	UT_parseColor(pszTmpColor, bgcolor);
      }
    m_bgColorInitted = true;
  }
  
  return bgcolor;
}

UT_RGBColor FV_View::getColorSelForeground ()
{
  static UT_RGBColor fgcolor (255, 255, 255);

  XAP_Frame * pFrame = 0;
  
  if ((pFrame = static_cast<XAP_Frame*>(getParentData())) != NULL)
    return pFrame->getColorSelForeground ();
  
  return fgcolor;
}

// TODO i18n All of these case functions are too simplistic:
// TODO i18n German single-letter "sharp s" uppercases to double-letter "SS"
// TODO i18n Turkish "i" uppercases to capital "I" with dot
// TODO i18n Turkish "I" lowercases to lower "i" without dot
// TODO i18n Some ligatures have special versions with just the first letter
// TODO i18n uppercase

#if 0
// the code is not directly in the case function, as passing the
// arguments is too awkward
static void _toggleSentence (const UT_UCSChar * src,
				 UT_UCSChar * dest, UT_uint32 len, const UT_UCSChar * prev)
{
	if(!prev || (UT_UCS4_isSentenceSeparator(prev[0]) && UT_UCS4_isspace (prev[1])))
	{
		dest[0] = UT_UCS4_toupper (src[0]);
		dest[1] = src[1];
	}
	else
	{
		dest[0] = src[0];
		dest[1] = src[1];
	}

	for (UT_uint32 i = 2; i < len; i++)
	{
		if(UT_UCS4_isSentenceSeparator(src[i-2]) && UT_UCS4_isspace (src[i-1]))
			dest[i] = UT_UCS4_toupper (src[i]);
		else
			dest[i] = src[i];
	}
}
#endif

static void _toggleFirstCapital(const UT_UCSChar * src,
				 UT_UCSChar * dest, UT_uint32 len, const UT_UCSChar * prev)
{
	if(!prev || UT_UCS4_isspace(prev[0]))
	{
		dest[0] = UT_UCS4_toupper(src[0]);
	}
	else
	{
		dest[0] = UT_UCS4_tolower(src[0]);
	}

	for (UT_uint32 i = 1; i < len; i++)
	{
		if(UT_UCS4_isspace(src[i-1]))
			dest[i] = UT_UCS4_toupper (src[i]);
		else
			dest[i] = UT_UCS4_tolower (src[i]);
	}
}

// all gets set to lowercase
static void _toggleLower (const UT_UCSChar * src,
			  UT_UCSChar * dest, UT_uint32 len)
{
	for (UT_uint32 i = 0; i < len; i++)
	{
		dest[i] = UT_UCS4_tolower (src[i]);
	}
}

// all gets set to uppercase
static void _toggleUpper (const UT_UCSChar * src,
			  UT_UCSChar * dest, UT_uint32 len)
{
	for (UT_uint32 i = 0; i < len; i++)
	{
		dest[i] = UT_UCS4_toupper (src[i]);
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
		if (wasSpace && !UT_UCS4_isspace (ch))
		{
			dest[i] = UT_UCS4_toupper (ch);
			wasSpace = false;
		}
		else if (UT_UCS4_isspace (ch))
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

		if (UT_UCS4_islower (ch))
			dest[i] = UT_UCS4_toupper (ch);
		else
			dest[i] = UT_UCS4_tolower (src[i]);
	}
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
		if(m_iInsPoint < m_Selection.getSelectionAnchor())
		{
			low  = m_iInsPoint;
			high = m_Selection.getSelectionAnchor();
		}
		else
		{
			high = m_iInsPoint;
			low  = m_Selection.getSelectionAnchor();
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
			UT_UCSChar * pT = reinterpret_cast<UT_UCSChar*>(buffer.getPointer(offset));
			xxx_UT_DEBUGMSG(("pT 0x%x, pT + 1 0x%x\n", *pT, *(pT+1)));

			if(pT && UT_UCS4_islower(*pT) && buffer.getLength() > 1 && UT_UCS4_islower (*(pT+1)))
			{
				// lowercase, make first capital
				xxx_UT_DEBUGMSG(("case 1\n"));
				c = CASE_FIRST_CAPITAL;
			}
			else if(pT && !UT_UCS4_islower (*pT) && buffer.getLength() > 1 && UT_UCS4_islower (*(pT+1)))
			{
				// first capital, make upper
				xxx_UT_DEBUGMSG(("case 2\n"));
				c = CASE_UPPER;
			}
			else if(pT && buffer.getLength() == 1 && UT_UCS4_islower (*pT))
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
		//fp_Run * pLastRun = static_cast<fp_Line *>(pBL->getLastContainer())->getLastRun();
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

			if(!pRun || pRun->getType() == FPRUN_ENDOFPARAGRAPH || iLenToCopy == 0)
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
					pRun = pRun->getNextRun();
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
					pRun = pRun->getNextRun();
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
							prev = reinterpret_cast<UT_UCSChar*>(buffer.getPointer(offset - 1));
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
					   UT_UCSChar * text = reinterpret_cast<UT_UCSChar*>(buffer.getPointer(0));
					   UT_uint32 i = 1;

					   // examine what preceedes this chunk of text
					   if(iOffset)
						   iOffset--;

					   while(iOffset && UT_UCS4_isspace(text[iOffset]))
					   {
						   iOffset--;
					   }

					   bool bStartOfSentence = !iOffset;

					   if(iOffset)
					   {
						   bStartOfSentence = UT_UCS4_isSentenceSeparator(text[iOffset]) && UT_UCS4_isalpha(text[iOffset-1]);
					   }

					   if(bStartOfSentence)
					   {
						   i = 0;
						   while( i < iLen && UT_UCS4_isspace(pTemp[i]))
							   i++;

						   if(i < iLen)
						   {
							   pTemp[i] = UT_UCS4_toupper (pTemp[i]);
							   i++;
						   }
					   }

					   for(; i < iLen; i++)
					   {
						   UT_ASSERT(i > 0);
						   while(i < iLen && !UT_UCS4_isSentenceSeparator(pTemp[i]))
							   i++;

						   // now i is either out of bounds, or points to the separator
						   // if it points to the separator, check that the character before it
						   // is a letter if not, start all over again
						   if(i < iLen && !UT_UCS4_isalpha(pTemp[i-1]))
							   continue;

						   // move past the separator
						   i++;

						   if(i < iLen)
						   {
							   while( i < iLen && UT_UCS4_isspace(pTemp[i]))
								   i++;

							   if(i < iLen)
								   pTemp[i] = UT_UCS4_toupper(pTemp[i]);
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

				UT_uint32 iRealDeleteCount;
				bool bResult = m_pDoc->deleteSpan(low, low + iLen,NULL,iRealDeleteCount);
				UT_ASSERT(bResult);

				//special handling is required for delete in revisions mode
				//where we have to move the insertion point
				if(isMarkRevisions())
				{
					UT_ASSERT( iRealDeleteCount <= iLen );
					_charMotion(true,iLen - iRealDeleteCount);
				}
				bResult = m_pDoc->insertSpan(low, pTemp, iLen, pSpanAPNow);

				// now remember the props for the next round
				pSpanAPNow = const_cast<PP_AttrProp*>(pSpanAPAfter);

				UT_ASSERT(bResult);
				low += iLen;
				offset += iLen;
			}
		}
		pBL = static_cast<fl_BlockLayout *>(pBL->getNext());
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
}

/*!
 * This method appends all the text in the current Block to the supplied
 * Growbuf.
 */
void FV_View::getTextInCurrentBlock(UT_GrowBuf & buf)
{
	fl_BlockLayout * pBlock = getCurrentBlock();
	pBlock->appendTextToBuf(buf);
}


/*!
 * This method appends all the text in the current DocSection to the supplied
 * Growbuf.
 */
void FV_View:: getTextInCurrentSection(UT_GrowBuf & buf)
{
	fl_BlockLayout * pBlock = getCurrentBlock();
	fl_DocSectionLayout * pDSL = pBlock->getDocSectionLayout();
	pDSL->appendTextToBuf(buf);
}


/*!
 * This method appends all the text in the Document to the supplied
 * Growbuf.
 */
void FV_View:: getTextInDocument(UT_GrowBuf & buf)
{
	fl_ContainerLayout * pDSL = static_cast<fl_ContainerLayout *>(m_pLayout->getFirstSection());
	while(pDSL)
	{
		pDSL->appendTextToBuf(buf);
		pDSL = pDSL->getNext();
	}
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
	// update the screen
	_draw(0, 0, getWindowWidth(), getWindowHeight(), false, false);
}

void FV_View::killBlink(void)
{
	m_pG->getCaret()->setBlink(false);
}

void FV_View::focusChange(AV_Focus focus)
{
	m_focus=focus;
	xxx_UT_DEBUGMSG(("fv_View:: Focus change focus = %d selection %d \n",focus,isSelectionEmpty()));
	switch(focus)
	{
	case AV_FOCUS_HERE:
		if (isSelectionEmpty() && (getPoint() > 0))
		{
			m_pG->getCaret()->setBlink(m_bCursorBlink);
		}
		m_pApp->rememberFocussedFrame(m_pParentData);
		break;
	case AV_FOCUS_NEARBY:
		if (isSelectionEmpty() && (getPoint() > 0))
		{
 			m_pG->getCaret()->setBlink(false);
		}
		break;
	case AV_FOCUS_MODELESS:
		if (isSelectionEmpty() && (getPoint() > 0))
		{
 			m_pG->getCaret()->setBlink(false);
		}
		break;
	case AV_FOCUS_NONE:
		if (isSelectionEmpty() && (getPoint() > 0))
		{
			m_pG->getCaret()->setBlink(false);
		}
		break;
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

				if (strcmp(propsBlock[i], m_chg.propsBlock[i]))
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

				if (strcmp(propsChar[i], m_chg.propsChar[i]))
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

				if (strcmp(propsSection[i], m_chg.propsSection[i]))
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
			pContainer = pRun->getLine()->getColumn();
		}

		UT_ASSERT(pContainer);
		if (pContainer && pContainer->getContainerType() == FP_CONTAINER_COLUMN)
		{
			fp_Column* pColumn = static_cast<fp_Column*>(pContainer);

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
		else if (pContainer && pContainer->getContainerType() == FP_CONTAINER_COLUMN_SHADOW)
		{
			// Hack the kludge:
			// Clearly you can't change columns while editing a header. -PL
			mask ^= AV_CHG_COLUMN;
		}
	}

	if (mask & AV_CHG_WINDOWSIZE)
		m_pG->getCaret()->setWindowSize(getWindowWidth(), getWindowHeight());

	// base class does the rest
	xxx_UT_DEBUGMSG(("FV_View: notifyListeners: this %x \n",this));
	return AV_View::notifyListeners(mask);
}


PT_DocPosition FV_View::saveSelectedImage (const char * toFile)
{
  const UT_ByteBuf * pBytes = NULL ;

  PT_DocPosition dPos = saveSelectedImage ( &pBytes ) ;

  if ( pBytes )
	{
	  pBytes->writeToFile ( toFile ) ;
	}

  return dPos ;
}

PT_DocPosition FV_View::mapDocPos( FV_DocPos dp ) {
	return ( _getDocPos( dp ));
	}

PT_DocPosition FV_View::saveSelectedImage (const UT_ByteBuf ** pBytes)
{
	const char * dataId;
	PT_DocPosition pos = getSelectedImage(&dataId);

	// if nothing selected or selection not an image
	if (pos == 0) return 0;

	if ( m_pDoc->getDataItemDataByName ( dataId, pBytes, NULL, NULL ) )
	  {
		return pos ;
	  }
	return 0 ;
}

/* If no image is selected returns 0
 * and if dataId is not NULL will set value to NULL
 * Otherwise returns a nonzero value indicating the position of the image
 * and if dataId is not NULL will set value to the image's data ID
 */
PT_DocPosition FV_View::getSelectedImage(const char **dataId)
{
	// if nothing selected, then an image can't be
	if (!isSelectionEmpty())
	{
		PT_DocPosition pos = m_Selection.getSelectionAnchor();
		fp_Run* pRun = NULL;

		UT_Vector vBlock;
		getBlocksInSelection( &vBlock);
		UT_uint32 count = vBlock.getItemCount();
		fl_BlockLayout * pBlock = NULL;
		for(UT_uint32 i=0; (i< count); i++)
		{
			if(i==0)
			{
				if(getPoint() < m_Selection.getSelectionAnchor())
				{
					pos = getPoint();
				}
				UT_sint32 x,y,x2,y2;
				UT_uint32 height;

				bool bEOL = false;
				bool bDirection;
				_findPositionCoords(pos,bEOL,x,y,x2,y2,height,bDirection,&pBlock,&pRun);
			}
			else
			{
				pBlock = static_cast<fl_BlockLayout *>(vBlock.getNthItem(i));
				pRun = pBlock->getFirstRun();
			}

			while(pRun && pRun->getType() != FPRUN_IMAGE)
			{
				pRun = pRun->getNextRun();
			}
			if(pRun && pRun->getType() == FPRUN_IMAGE)
			{
				pos = pBlock->getPosition() +  pRun->getBlockOffset();
				if (dataId != NULL)
				{
					fp_ImageRun * pImRun = static_cast<fp_ImageRun *>(pRun);
					*dataId = pImRun->getDataId();
				}
				return pos;
			}
		}
	}

	// if we made it here, then run type is not an image
	if (dataId != NULL) *dataId = NULL;
	return 0;
}

PT_DocPosition FV_View::getSelectionAnchor(void) const
{
	if(m_Selection.isSelected())
	{
		return m_Selection.getSelectionAnchor();
	}
	return m_iInsPoint;
}

bool FV_View::isSelectionEmpty(void) const
{
	if (!m_Selection.isSelected())
	{
		return true;
	}

	PT_DocPosition curPos = getPoint();
	if (curPos == m_Selection.getSelectionAnchor())
	{
		return true;
	}

	return false;
}


void FV_View::moveInsPtTo(FV_DocPos dp, bool bClearSelection)
{
	if(bClearSelection)
	{
		if (!isSelectionEmpty())
			_clearSelection();
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
		_ensureInsertionPointOnScreen();
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
	  is already in the _ensureInsertionPointOnScreen() function.
	  _fixInsertionPointCoords();
	  cmdScroll(AV_SCROLLCMD_LINEDOWN, static_cast<UT_uint32>(m_yPoint + m_iPointHeight/2 - getWindowHeight()/2));
	  cmdScroll(AV_SCROLLCMD_LINERIGHT, static_cast<UT_uint32>(m_xPoint - getWindowWidth()/2));
	  notifyListeners(AV_CHG_MOTION);
	*/
	_ensureInsertionPointOnScreen();
}


UT_uint32 FV_View::getCurrentPageNumber(void)
{
	UT_sint32 iPageNum = 0;
	PT_DocPosition pos = getPoint();
	fl_BlockLayout * pBlock;
	UT_sint32 xPoint, yPoint, xPoint2, yPoint2;
	UT_uint32 iPointHeight;
	bool bDirection;
	fp_Run* pRun;
	_findPositionCoords(pos, m_bPointEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection,&pBlock, &pRun);

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
		m_pDoc->beginUserAtomicGlob();
		_insertSectionBreak();
		m_pDoc->endUserAtomicGlob();
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

	// Signal PieceTable Changes have ended
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



bool FV_View::isCurrentListBlockEmpty(void)
{
	//
	// If the current block is a list and is otherwise empty return true
	//
	fl_BlockLayout * pBlock = getCurrentBlock();
	fl_BlockLayout * nBlock = static_cast<fl_BlockLayout *>(pBlock->getNext());
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
		FP_RUN_TYPE runtype = static_cast<FP_RUN_TYPE>(pRun->getType());
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
			pRun = pRun->getNextRun();
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
	pRun = pRun->getPrevRun();
	while(pRun != NULL && bBefore == true)
	{
		if(pRun->getType()== FPRUN_FIELD)
		{
			fp_FieldRun * pFRun = static_cast<fp_FieldRun *>(pRun);
			if (pFRun->getFieldType() == FPFIELD_list_label)
			{
				bBefore = false;
			}
		}
		pRun = pRun->getPrevRun();
	}
	return bBefore;
}

/*!
 * This method returns true if the block presented has a numbered heading
 * defined or in it's ancestray.
 */
bool FV_View::isNumberedHeadingHere(fl_BlockLayout * pBlock)
{
	bool bHasNumberedHeading = false;
	if(pBlock == NULL)
	{
		return bHasNumberedHeading;
	}
	const PP_AttrProp * pBlockAP = NULL;
	pBlock->getAttrProp(&pBlockAP);
	const XML_Char* pszCurStyle = NULL;
	pBlockAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, pszCurStyle);
	if(pszCurStyle == NULL)
	{
		return false;
	}
	PD_Style * pCurStyle = NULL;
	m_pDoc->getStyle(static_cast<const char*>(pszCurStyle), &pCurStyle);
	UT_uint32 depth = 0;
	while(pCurStyle && !bHasNumberedHeading && depth < 10)
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
	return bHasNumberedHeading;
}

void FV_View::processSelectedBlocks(FL_ListType listType)
{
	//
	// Update Lists in the selected region
	//

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	UT_Vector vBlock;
	getBlocksInSelection( &vBlock);
//
// Turn off cursor
//
	if(!isSelectionEmpty())
	{
		_clearSelection();
	}
	UT_uint32 i;
	m_pDoc->disableListUpdates();

	m_pDoc->beginUserAtomicGlob();

	char margin_left [] = "margin-left";
	char margin_right[] = "margin-right";

	for(i=0; i< vBlock.getItemCount(); i++)
	{
		UT_DEBUGMSG(("SEVIOR: Processing block %d \n",i));
		fl_BlockLayout * pBlock =  static_cast<fl_BlockLayout *>(vBlock.getNthItem(i));
		PL_StruxDocHandle sdh = pBlock->getStruxDocHandle();
		if(pBlock->isListItem() == true)
		{
			m_pDoc->StopList(sdh);
		}
		else
		{
			fl_BlockLayout * pPrev = static_cast<fl_BlockLayout *>(pBlock->getPrev());
			while(pPrev && (pPrev->getContainerType() != FL_CONTAINER_BLOCK))
			{
				pPrev = static_cast<fl_BlockLayout *>(pPrev->getPrev());
			}
//
// Only attach block to previous list if the margin of the current block < the
// previous block.
//
			double prevLeft = 0.0;
			double blockLeft = 0.0;
			if(pPrev != NULL)
			{
				prevLeft = pPrev->getDominantDirection() == FRIBIDI_TYPE_LTR
				  ? UT_convertToInches(pPrev->getProperty(margin_left,true))
				  : UT_convertToInches(pPrev->getProperty(margin_right,true));

				blockLeft = pBlock->getDominantDirection() == FRIBIDI_TYPE_LTR
				  ? UT_convertToInches(pBlock->getProperty(margin_left,true))
				  : UT_convertToInches(pBlock->getProperty(margin_right,true));;
			}
//
// Look for Numbered Heading in the prev block style or it's ancestry.
// If there is one there we don't attach this block to it.
//
			bool bHasNumberedHeading = false;
			if(pPrev != NULL)
			{
				bHasNumberedHeading = isNumberedHeadingHere(pPrev);
			}
//
// Don't resume if the previous block has a Numbered Heading Style
//
			if(!bHasNumberedHeading && (pBlock->isListItem()== false) && (pPrev != NULL) && (pPrev->isListItem()== true) && (pPrev->getAutoNum()->getType() == listType) && (blockLeft <= (prevLeft - 0.00001)))
			{
				pBlock->resumeList(pPrev);
			}
			else if(!pBlock->isListItem())
			{
				XML_Char* cType = pBlock->getListStyleString(listType);
				pBlock->StartList(cType);
			}
		}
	}

	// closes bug # 1255 - unselect a list after creation
	cmdUnselectSelection();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	m_pDoc->endUserAtomicGlob();

	_generalUpdate();

	// Signal piceTable is stable again
	_restorePieceTableState();
	if (isSelectionEmpty())
	{
		_ensureInsertionPointOnScreen();
	}
}

/*!
 * This returns the number of distinct columns in a selection. If part of the 
 * the selection is outside a table, it returns 0.
 */
UT_sint32 FV_View::getNumColumnsInSelection(void)
{
	UT_Vector vecBlocks;
	getBlocksInSelection(&vecBlocks);
	UT_sint32 i =0;
	UT_sint32 iNumCols = 0;
	UT_sint32 iCurCol = -1;
	fl_BlockLayout * pBlock = NULL;
	fl_CellLayout * pCell = NULL;
	fp_CellContainer * pCellCon = NULL;
	for(i=0; i< static_cast<UT_sint32>(vecBlocks.getItemCount());i++)
	{
		pBlock = static_cast<fl_BlockLayout *>(vecBlocks.getNthItem(i));
		if(pBlock->myContainingLayout()->getContainerType() != FL_CONTAINER_CELL)
		{
			return 0;
		}
		pCell = static_cast<fl_CellLayout *>(pBlock->myContainingLayout());
		pCellCon = static_cast<fp_CellContainer *>(pCell->getFirstContainer());
		if(pCellCon == NULL)
		{
			return 0;
		}
		if(pCellCon->getLeftAttach() > iCurCol)
		{
			iNumCols++;
			iCurCol = pCellCon->getLeftAttach();
		}
	}
	return iNumCols;
}

/*!
 * This returns the number of distinct rows in a selection. If part of the 
 * the selection is outside a table, it returns 0.
 */
UT_sint32 FV_View::getNumRowsInSelection(void)
{
	UT_Vector vecBlocks;
	getBlocksInSelection(&vecBlocks);
	UT_sint32 i =0;
	UT_sint32 iNumRows = 0;
	UT_sint32 iCurRow = -1;
	fl_BlockLayout * pBlock = NULL;
	fl_CellLayout * pCell = NULL;
	fp_CellContainer * pCellCon = NULL;
	for(i=0; i< static_cast<UT_sint32>(vecBlocks.getItemCount());i++)
	{
		pBlock = static_cast<fl_BlockLayout *>(vecBlocks.getNthItem(i));
		if(pBlock->myContainingLayout()->getContainerType() != FL_CONTAINER_CELL)
		{
			return 0;
		}
		pCell = static_cast<fl_CellLayout *>(pBlock->myContainingLayout());
		pCellCon = static_cast<fp_CellContainer *>(pCell->getFirstContainer());
		if(pCellCon == NULL)
		{
			return 0;
		}
		if(pCellCon->getTopAttach() > iCurRow)
		{
			iNumRows++;
			iCurRow = pCellCon->getTopAttach();
		}
	}
	return iNumRows;
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
	if (m_Selection.getSelectionAnchor() > startpos)
	{
		endpos = m_Selection.getSelectionAnchor();
	}
	else
	{
		startpos = m_Selection.getSelectionAnchor();
	}
//
// tweak the start point of the selection if it is just before the current block
// strux. Clicking in the left margin moves the start point here but usually the
// user wants block after this.
//
	fl_BlockLayout * pBlock = _findBlockAtPosition(startpos);
	fl_BlockLayout * pBlNext = NULL;
	PT_DocPosition posEOD = 0;
	getEditableBounds(true, posEOD);

	if(startpos < posEOD)
	{
		pBlNext = _findBlockAtPosition(startpos+1);
	}
	if((pBlNext != NULL) && (pBlNext != pBlock))
	{
		pBlock = pBlNext;
	}
	while( pBlock != NULL && pBlock->getPosition() <= endpos)
	{
		if(pBlock->getContainerType()== FL_CONTAINER_BLOCK)
		{
			vBlock->addItem(pBlock);
		}
		pBlock = static_cast<fl_BlockLayout *>(pBlock->getNextBlockInDocument());
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
		fl_BlockLayout * pPrev = static_cast<fl_BlockLayout *>(getCurrentBlock()->getPrev());
		sdh = pPrev->getStruxDocHandle();
		m_pDoc->StopList(sdh);
		_setPoint(getCurrentBlock()->getPosition());
	}

	const XML_Char* style = NULL;
	PD_Style* pStyle = NULL;
	if(getStyle(&style) && bAtEnd)
	{
		m_pDoc->getStyle(static_cast<const char*>(style), &pStyle);
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
				if (UT_strcmp(static_cast<const char *>(szValue), static_cast<const char *>(style)) != 0)
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

	_generalUpdate();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	m_pDoc->endUserAtomicGlob();

	_generalUpdate();

	// Signal piceTable is stable again
	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	m_iPieceTableState = 0;
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();
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

	// insert a new paragraph with the same attributes/properties
	// as the previous (or none if the first paragraph in the section).

	m_pDoc->insertStrux(getPoint(), PTX_Block);

	if (bDidGlob)
		m_pDoc->endUserAtomicGlob();

	_generalUpdate();
	_ensureInsertionPointOnScreen();
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
	if (!isSelectionEmpty())
	{
		if (m_Selection.getSelectionAnchor() < posStart)
		{
			posStart = m_Selection.getSelectionAnchor();
		}
		else
		{
			posEnd = m_Selection.getSelectionAnchor();
		}
		if(posStart < 2)
		{
			posStart = 2;
		}
	}

	// lookup the current style
	PD_Style * pStyle = NULL;
	m_pDoc->getStyle(static_cast<const char*>(style), &pStyle);
	if (!pStyle)
	{
		m_pDoc->enableListUpdates();
		UT_DEBUGMSG(("restoring PieceTable state (2)\n"));
		_restorePieceTableState();
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	if(strcmp(style,"None") == 0)
	{
		m_pDoc->enableListUpdates();
		UT_DEBUGMSG(("restoring PieceTable state (2)\n"));
		_restorePieceTableState();
		return true; // do nothing.
	}
	pStyle->used (1);
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
	pBL = _findBlockAtPosition(posStart+2);
	if((posStart == pBL->getPosition(true)) && (posEnd > posStart))
	{
		posStart++;
		if(posStart > posEnd)
		{
			posEnd=posStart;
		}
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
	setScreenUpdateOnGeneralUpdate( false);
	bool bCharStyle = pStyle->isCharStyle();
	const XML_Char * attribs[] = { PT_STYLE_ATTRIBUTE_NAME, 0, 0 };
	attribs[1] = style;
//
// Need this to adjust the start and end positions of the style change.
//
	PT_DocPosition curPos=0;
	if(bisListStyle)
	{
//
// Stop the Lists contained in the current selection.
//
//
// Adjust region of style for the deletion of the Field/Tab required for each list
// element.
//
		UT_uint32 i;

		for(i=0; i< vBlock.getItemCount(); i++)
		{
			pBL = static_cast<fl_BlockLayout *>(vBlock.getNthItem(i));
			curPos = pBL->getPosition();
			if(pBL->isListItem())
			{
				if(curPos < posStart)
				{
					posStart -=2;
				}
				if(curPos < posEnd)
				{
					posEnd -= 2;
				}
			}
			while(pBL->isListItem())
			{
				m_pDoc->StopList(pBL->getStruxDocHandle());
			}
		}
	}

	bool bHasNumberedHeading = false;
	const XML_Char * pszCurStyle = style;
	PD_Style * pCurStyle = pStyle;
	UT_uint32 depth = 0;
	if (bCharStyle)
	{
		_clearIfAtFmtMark(getPoint());	// TODO is this correct ?
		_eraseSelection();
		UT_DEBUGMSG(("Applying Character style: start %d, end %d\n", posStart, posEnd));
		bRet = m_pDoc->changeSpanFmt(PTC_AddStyle,posStart,posEnd,attribs,NULL);
		goto finish_up;
	}
	else
	{
		// set block-level style
		UT_DEBUGMSG(("Applying Block style: start %d, end %d\n", posStart, posEnd));

		_clearIfAtFmtMark(getPoint());	// TODO is this correct ?
		UT_DEBUGMSG(("Applying Paragraph style: start %d, end %d\n", posStart, posEnd));

		// PLAM this is the only place that PTC_AddStyle is used.
		// NB: clear explicit props at both block and char levels
		bRet = m_pDoc->changeStruxFmt(PTC_AddStyle,posStart,posEnd,attribs,NULL,PTX_Block);
	}
//
// Do the list elements processing
//
//
// Look for Numbered Heading in the style or it's ancestry.
//
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
			pBL = static_cast<fl_BlockLayout *>(vBlock.getNthItem(i));
			curPos = pBL->getPosition();
			if(curPos < posStart)
			{
				posStart +=2;
			}
			if(curPos < posEnd)
			{
				posEnd += 2;
			}
			if(i == 0)
				pBL->StartList(style);
			else
				pBL->resumeList(static_cast<fl_BlockLayout *>(pBL->getPrev()));
		}
	}
//
// Code to handle Numbered Heading styles...
//
	else if(bHasNumberedHeading)
	{
		fl_BlockLayout*  currBlock = static_cast<fl_BlockLayout *>(vBlock.getNthItem(0));
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
//
// Now look to see if the proposed parent item is already a list parent. In which
// case we just add this iem to the already existing list. The list class will get
// the order correct.
//
					UT_uint32 count = m_pDoc->getListsCount();
					UT_uint32 i =0;
					const fl_AutoNum * pAuto = NULL;
					bool bFoundPrevList = false;
					for(i=0; (i< count) && !bFoundPrevList; i++)
					{
						pAuto = m_pDoc->getNthList(i);
						bFoundPrevList = (pAuto->getParentItem() == prevSDH);
					}
					if(bFoundPrevList && pAuto->getFirstItem())
					{
						PL_StruxDocHandle subSDH = pAuto->getFirstItem();
						fl_BlockLayout * pSubBlock = getBlockFromSDH(subSDH);
						UT_ASSERT(pSubBlock);
						if(pSubBlock == NULL)
							goto finish_up;

						for(i=0; i< vBlock.getItemCount(); i++)
						{
							pBL = static_cast<fl_BlockLayout *>(vBlock.getNthItem(i));
							curPos = pBL->getPosition();
							if(curPos < posStart)
							{
								posStart +=2;
							}
							if(curPos < posEnd)
							{
								posEnd += 2;
							}
							pBL->prependList(pSubBlock);
						}
					}
//
// OK the previously found item with a numbered list is not a parent item
// even though this style is at a higher level of nesting than the previous
// style. Instead we have to start a new sublist with the previous item as
// the parent.
//
					else
					{
						for(i=0; i< vBlock.getItemCount(); i++)
						{
							pBL = static_cast<fl_BlockLayout *>(vBlock.getNthItem(i));
							curPos = pBL->getPosition();
							if(curPos < posStart)
							{
								posStart +=2;
							}
							if(curPos < posEnd)
							{
								posEnd += 2;
							}
							if(i == 0)
								pBL->StartList(style,prevSDH);
							else
								pBL->resumeList(static_cast<fl_BlockLayout *>(pBL->getPrev()));
						}
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
				fl_BlockLayout * pNext = static_cast<fl_BlockLayout *>(vBlock.getLastItem());
				pNext = static_cast<fl_BlockLayout *>(pNext->getNext());
				if(pNext)
				{
					PT_DocPosition nextPos = pNext->getPosition(false)+1;
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
					pBL = static_cast<fl_BlockLayout *>(vBlock.getNthItem(i));
					curPos = pBL->getPosition();
					if(curPos < posStart)
					{
						posStart +=2;
					}
					if(curPos < posEnd)
					{
						posEnd += 2;
					}
					if(i == 0)
						pBL->StartList(style);
					else
						pBL->resumeList(static_cast<fl_BlockLayout *>(pBL->getPrev()));
				}
			}
//
// Insert into pre-existing List.
//
			else if(bFoundPrevHeadingBackwards)
			{
				fl_BlockLayout * pBlock = getBlockFromSDH(sdh);
				UT_ASSERT(pBlock);
				if(pBlock == NULL)
					goto finish_up;

				for(UT_uint32 j = 0; j < vBlock.getItemCount(); ++j)
				{
					pBL = static_cast<fl_BlockLayout *>(vBlock.getNthItem(j));
					curPos = pBL->getPosition();
					if(curPos < posStart)
					{
						posStart +=2;
					}
					if(curPos < posEnd)
					{
						posEnd += 2;
					}
					if(j == 0)
						pBL->resumeList(pBlock);
					else
						pBL->resumeList(static_cast<fl_BlockLayout *>(pBL->getPrev()));
				}
			}
//
// Prepend onto a pre-existing List.
//
			else
			{
				fl_BlockLayout * pBlock = getBlockFromSDH(sdh);
				UT_ASSERT(pBlock);
				if(pBlock == NULL)
					goto finish_up;
				for(UT_uint32 j = 0; j < vBlock.getItemCount(); j++)
				{
					pBL = static_cast<fl_BlockLayout *>(vBlock.getNthItem(j));
					curPos = pBL->getPosition();
					if(curPos < posStart)
					{
						posStart +=2;
					}
					if(curPos < posEnd)
					{
						posEnd += 2;
					}
					if (j == 0)
						pBL->prependList(pBlock);
					else
						pBL->resumeList(static_cast<fl_BlockLayout *>(pBL->getPrev()));
				}
			}
		}
	}
 finish_up:
	setScreenUpdateOnGeneralUpdate( true);
	if(!bDontGeneralUpdate)
	{
		_generalUpdate();
		// restore updates and clean up dirty lists
		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();
	}

	m_pDoc->endUserAtomicGlob();
	// Signal piceTable is stable again
	UT_DEBUGMSG(("restoring PieceTable state\n"));
	_restorePieceTableState();
	UT_DEBUGMSG(("SEVIOR: posStart %d posEnd %d at end \n",posStart,posEnd));
	if (posEnd != posStart)
	{
		_clearSelection();
		_setPoint(posStart);
		_setSelectionAnchor();
		_setPoint(posEnd);
		_drawSelection();
	}
	return bRet;
}

/*!
 * This method finds the appropiate matching block in the current view
 * for the StruxDocHandle (actually a pointer to a pf_frag_strux)
 */
fl_BlockLayout * FV_View::getBlockFromSDH(PL_StruxDocHandle sdh)
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
			pBlock = const_cast<fl_BlockLayout *>(static_cast<const fl_BlockLayout *>(sfh));
			if(pBlock->getDocLayout() == m_pLayout)
			{
				bFound = true;
			}
		}
		else
		{
			pBlock = NULL;
			bFound = true;
		}
	}
	return pBlock;
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
		if (m_Selection.getSelectionAnchor() < posStart)
			posStart = m_Selection.getSelectionAnchor();
		else
			posEnd = m_Selection.getSelectionAnchor();
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

				if (strcmp(sz, szBlock))
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
		UT_sint32 xPoint, yPoint, xPoint2, yPoint2;
		UT_uint32 iPointHeight;
		bool bDirection;

		fl_BlockLayout* pBlock;
		fp_Run* pRun;

		_findPositionCoords(posStart, false, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection, &pBlock, &pRun);
		UT_uint32 blockPosition = pBlock->getPosition();
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
			fl_BlockLayout* pBlockEnd;
			fp_Run* pRunEnd;
			_findPositionCoords(posEnd, false, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection, &pBlockEnd, &pRunEnd);

			while (pRun && (pRun != pRunEnd))
			{
				const PP_AttrProp * pAP;
				bool bCheck = false;

				pRun = pRun->getNextRun();
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

					if ((bCharStyle != bHere) || (strcmp(sz, szChar)))
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

bool FV_View::setCharFormat(const XML_Char * properties[], const XML_Char * attribs[])
{
	bool bRet;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_Selection.getSelectionAnchor() < posStart)
		{
			posStart = m_Selection.getSelectionAnchor();
		}
		else
		{
			posEnd = m_Selection.getSelectionAnchor();
		}
	}

	// Here the selection used to be cleared, but that prevented users
	// from making multiple changes to the same region.

	bRet = m_pDoc->changeSpanFmt(PTC_AddFmt,posStart,posEnd,attribs,properties);

	// if there is a selection then we need to change the formatting also for any
	// completely selected block within the selection
	if(posStart != posEnd)
	{
		fl_BlockLayout * pBL1 = _findBlockAtPosition(posStart);
		fl_BlockLayout * pBL2 = _findBlockAtPosition(posEnd);
		bool bFormatStart = false;
		bool bFormatEnd = false;

		PT_DocPosition posBL1 = pBL1->getPosition(false);

		fp_Run * pLastRun2 = static_cast<fp_Line *>(pBL2->getLastContainer())->getLastRun();
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
			if(pBL2->getPrev()->getLastContainer()->getContainerType() == FP_CONTAINER_LINE)
			{
				pLastRun2 = static_cast<fp_Line *>(pBL2->getPrev()->getLastContainer())->getLastRun();
				posEnd = pBL2->getPrev()->getPosition(false) + pLastRun2->getBlockOffset() + pLastRun2->getLength() - 1;
			}
		}

		if(posEnd > posStart)
		    bFormatEnd = true;

		if(bFormatStart && bFormatEnd)
			bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,posStart,posEnd,attribs,properties,PTX_Block);
	}

	_generalUpdate();
	_fixInsertionPointCoords();

	// Signal piceTable is stable again
	_restorePieceTableState();

	return bRet;
}

bool FV_View::getAttributes(const PP_AttrProp ** ppSpanAP, const PP_AttrProp ** ppBlockAP, PT_DocPosition posStart)
{
	if(getLayout()->getFirstSection() == NULL)
		return false;

	PT_DocPosition posEnd = posStart;
	bool bSelEmpty = true;
	if(posStart == 0)
	{
		posStart = getPoint();
		posEnd = posStart;
		bSelEmpty = isSelectionEmpty();

		if (!bSelEmpty)
		{
			if (m_Selection.getSelectionAnchor() < posStart)
				posStart = m_Selection.getSelectionAnchor();
			else
				posEnd = m_Selection.getSelectionAnchor();
		}
	}
	if(posStart < 2)
	{
		posStart = 2;
	}
	// 1. assemble complete set at insertion point
	UT_sint32 xPoint, yPoint, xPoint2, yPoint2;
	UT_uint32 iPointHeight;
	bool bDirection;

	fl_BlockLayout* pBlock;
	fl_BlockLayout* pNBlock = NULL;
	fp_Run* pRun;

	_findPositionCoords(posStart, false, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection, &pBlock, &pRun);

//
// Look if our selection starts just before a paragraph break
//
	if(posStart < posEnd)
	{
		pNBlock = _findBlockAtPosition(posStart+1);
		if(pNBlock != pBlock)
		{
			_findPositionCoords(posStart + 1,
								false,
								xPoint,
								yPoint,
								xPoint2,
								yPoint2,
								iPointHeight,
								bDirection,
								&pBlock,
								&pRun);
		}
	}

	UT_uint32 blockPosition = pBlock->getPosition();
	if(blockPosition > posStart)
	{
		posStart = blockPosition;
		if(posEnd < posStart)
			posEnd = posStart;
	}
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

	if (ppSpanAP)
		pBlock->getSpanAttrProp( (posStart - blockPosition), bLeftSide, ppSpanAP);
	if (ppBlockAP)
		pBlock->getAttrProp(ppBlockAP);

	return true;
}

bool FV_View::getCharFormat(const XML_Char *** pProps, bool bExpandStyles)
{
	return getCharFormat(pProps,bExpandStyles,0);
}

bool FV_View::getCharFormat(const XML_Char *** pProps, bool bExpandStyles, PT_DocPosition posStart)
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
//
// fixme
#if 0
	// NOTE: caller must free this, but not the referenced contents
	const XML_Char ** tprops = static_cast<const XML_Char **>(UT_calloc(3, sizeof(XML_Char *)));
	UT_DEBUGMSG(("charFormat \n"));
	tprops[0] = "fred";
	tprops[1] = "1";
	tprops[2] = NULL;
	*pProps = tprops;
#endif
	if((AV_View::getTick() == m_CharProps.getTick()) && m_CharProps.isValid())
	{
		*pProps = m_CharProps.getCopyOfProps();
		return true;
	}
	m_CharProps.clearProps();
	m_CharProps.setTick(AV_View::getTick());
	/*
	  IDEA: We want to know character-level formatting properties, if
	  they're constant across the entire selection.  To do so, we start
	  at the beginning of the selection, load 'em all into a vector, and
	  then prune any property that collides.
	*/
	PT_DocPosition posEnd = posStart;
	bool bSelEmpty = true;
	if(posStart == 0)
	{
		posStart = getPoint();
		posEnd = posStart;
		bSelEmpty = isSelectionEmpty();

		if (!bSelEmpty)
		{
			if (m_Selection.getSelectionAnchor() < posStart)
				posStart = m_Selection.getSelectionAnchor();
			else
				posEnd = m_Selection.getSelectionAnchor();
		}
	}
	if(posStart < 2)
	{
		posStart = 2;
	}
	// 1. assemble complete set at insertion point
	UT_sint32 xPoint, yPoint, xPoint2, yPoint2;
	UT_uint32 iPointHeight;
	bool bDirection;

	fl_BlockLayout* pBlock;
	fl_BlockLayout* pNBlock = NULL;
	fp_Run* pRun;

	_findPositionCoords(posStart, false, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection, &pBlock, &pRun);

//
// Look if our selection starts just before a paragraph break
//
	if(posStart < posEnd)
	{
		pNBlock = _findBlockAtPosition(posStart+1);
		if(pNBlock != pBlock)
		{
			_findPositionCoords(posStart + 1,
								false,
								xPoint,
								yPoint,
								xPoint2,
								yPoint2,
								iPointHeight,
								bDirection,
								&pBlock,
								&pRun);
		}
	}

	UT_uint32 blockPosition = pBlock->getPosition();
	if(blockPosition > posStart)
	{
		posStart = blockPosition;
		if(posEnd < posStart)
			posEnd = posStart;
	}
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

	UT_uint32 iPropsCount = PP_getPropertyCount();

	for(UT_uint32 n = 0; n < iPropsCount; n++)
	{
		if((PP_getNthPropertyLevel(n) & PP_LEVEL_CHAR))
		{
			f = new _fmtPair(PP_getNthPropertyName(n),pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
			if(f->m_val != NULL)
				v.addItem(static_cast<void *>(f));
			else
				delete f;
		}
	}
	
	// 2. prune 'em as they vary across selection
	if (!bSelEmpty)
	{
		fl_BlockLayout* pBlockEnd;
		fp_Run* pRunEnd;
		_findPositionCoords(posEnd, false, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection, &pBlockEnd, &pRunEnd);

		while (pRun && (pRun != pRunEnd))
		{
			const PP_AttrProp * pAP;
			bool bCheck = false;

			pRun = pRun->getNextRun();


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
					f = static_cast<_fmtPair *>(v.getNthItem(i-1));

					const XML_Char * value = PP_evalProperty(f->m_prop,pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);

					// prune anything that doesn't match
					if (value && strcmp(f->m_val, value))
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
	const XML_Char ** props = static_cast<const XML_Char **>(UT_calloc(count, sizeof(XML_Char *)));
	if (!props)
		return false;

	const XML_Char ** p = props;
	i = v.getItemCount();
	UT_uint32 numProps = count;
	while (i > 0)
	{
		f = static_cast<_fmtPair *>(v.getNthItem(i-1));
		i--;

		p[0] = f->m_prop;
		p[1] = f->m_val;
		p += 2;

	}
	p[0] = NULL;

	UT_VECTOR_PURGEALL(_fmtPair *,v);

	*pProps = props;
	m_CharProps.fillProps(numProps,props);
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
	pBlock = static_cast<fl_BlockLayout *>(pSl->getFirstLayout());
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
		pBlock = static_cast<fl_BlockLayout *>(pBlock->getNext());
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
	UT_String szAlign;
	UT_String szIndent;
	double fIndent;
	bool bRet = true;
	UT_Dimension dim;
	double fAlign;
	fl_BlockLayout * pBlock;
	UT_uint32 i;
	//
	// Signal PieceTable Change
	//
	_saveAndNotifyPieceTableChange();

	m_pDoc->beginUserAtomicGlob();
	//
	// OK now change the alignements of the blocks.
	//
	if(doLists)
		getAllBlocksInList(&v);
	else
		getBlocksInSelection(&v);
	const XML_Char * props[] = {NULL,"0.0in",NULL,NULL};
	const XML_Char ind_left [] = "margin-left";
	const XML_Char ind_right[] = "margin-right";
	const XML_Char * indent;
	
	for(i = 0; i<v.getItemCount();i++)
	{
		pBlock = static_cast<fl_BlockLayout *>(v.getNthItem(i));
		if(pBlock->getDominantDirection() == FRIBIDI_TYPE_RTL)
			indent = ind_right;
		else
			indent = ind_left;
		
		szAlign = pBlock->getProperty(indent);
		dim = UT_determineDimension(szAlign.c_str());
		fAlign = static_cast<double>(UT_convertToInches(szAlign.c_str()));
		szIndent = pBlock->getProperty("text-indent");
		fIndent = static_cast<double>(UT_convertToInches(szIndent.c_str()));
		if(fAlign + fIndent + indentChange < 0.0)
		{
			fAlign = -fIndent + 0.0001;
		}
		else if( fAlign + indentChange + fIndent > page_size)
		{
			fAlign = page_size - fIndent;
		}
		else
		{
			fAlign = fAlign + indentChange;
		}
		UT_String szNewAlign = UT_convertInchesToDimensionString (dim, fAlign);
		PL_StruxDocHandle sdh = pBlock->getStruxDocHandle();
		PT_DocPosition iPos = m_pDoc->getStruxPosition(sdh)+fl_BLOCK_STRUX_OFFSET;
		props[0] = indent;
		props[1] = static_cast<const XML_Char *>(szNewAlign.c_str());
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, iPos, iPos, NULL, props, PTX_Block);
	}
	//
	// Moved outside the loop. Speeds things up and seems OK.
	//
	_generalUpdate();
	_fixInsertionPointCoords();

	m_pDoc->endUserAtomicGlob();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	return bRet;
}

bool FV_View::removeStruxAttrProps(PT_DocPosition ipos1, 
								   PT_DocPosition ipos2, 
								   PTStruxType iStrux,
								   const XML_Char * attributes[] ,
								   const XML_Char * properties[])
{
	bool bRet;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	_clearIfAtFmtMark(getPoint());
	bRet = m_pDoc->changeStruxFmt(PTC_RemoveFmt,ipos1,ipos2,attributes,properties,iStrux);

	_generalUpdate();
	_fixInsertionPointCoords();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	return bRet;
}

bool FV_View::isImageAtStrux(PT_DocPosition ipos1, PTStruxType iStrux)
{
	PL_StruxDocHandle sdh = NULL;
	bool  bret = m_pDoc->getStruxOfTypeFromPosition(ipos1, iStrux, &sdh);
	if(!bret)
	{
		return false;
	}	
	const XML_Char * pszDataID = NULL;
    bret = m_pDoc->getAttributeFromSDH(sdh, PT_STRUX_IMAGE_DATAID,&pszDataID);
	if(!bret)
	{
		return false;
	}
	if(pszDataID == NULL)
	{
		return false;
	}
	return true;
}

bool FV_View::setBlockFormat(const XML_Char * properties[])
{
	bool bRet;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	_clearIfAtFmtMark(getPoint());

	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;
	if (!isSelectionEmpty())
	{
		if (m_Selection.getSelectionAnchor() < posStart)
		{
			posStart = m_Selection.getSelectionAnchor();
		}
		else
		{
			posEnd = m_Selection.getSelectionAnchor();
		}
	}
	if(posStart < 2)
	{
		posStart = 2;
	}


	// if the format change includes dom-dir, we need to force change
	// of direction for the last run in the block, the EndOfParagraph
	// run. (This should really not be necessary, the EndOfParagraph
	// run should lookup its properties)

	bool bDomDirChange = false;
	FriBidiCharType iDomDir = FRIBIDI_TYPE_LTR;

	const XML_Char ** p  = properties;

	while(*p)
	{
		if(!UT_strcmp(*p,"dom-dir"))
		{
			bDomDirChange = true;
			if(!UT_strcmp(*(p+1), "rtl"))
			{
				iDomDir = FRIBIDI_TYPE_RTL;
			}
			break;
		}
		p += 2;
	}

	if(bDomDirChange)
	{

		fl_BlockLayout * pBl = _findBlockAtPosition(posStart);
		fl_BlockLayout * pBl2 = _findBlockAtPosition(posEnd);

		if(pBl2)
			pBl2 = static_cast<fl_BlockLayout *>(pBl2->getNext());

		while(pBl)
		{

			if(iDomDir == FRIBIDI_TYPE_RTL)
			{
				static_cast<fp_Line *>(static_cast<fl_BlockLayout *>(pBl)->getLastContainer())->getLastRun()->setDirection(FRIBIDI_TYPE_LTR);
			}
			else
			{
				static_cast<fp_Line *>(static_cast<fl_BlockLayout *>(pBl)->getLastContainer())->getLastRun()->setDirection(FRIBIDI_TYPE_RTL);
			}

			pBl = static_cast<fl_BlockLayout *>(pBl->getNext());
			if(pBl == pBl2)
				break;
		}
	}

	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,posStart,posEnd,NULL,properties,PTX_Block);

	_generalUpdate();
	_fixInsertionPointCoords();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

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
	fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pHFSL->getFirstLayout());
	bool bFoundPageNumber = false;
	while(pBL != NULL && !bFoundPageNumber)
	{
		fp_Run * pRun = pBL->getFirstRun();
		while(pRun != NULL && !bFoundPageNumber)
		{
			if(pRun->getType() == FPRUN_FIELD)
			{
				fp_FieldRun * pFRun = static_cast<fp_FieldRun *>(pRun);
				bFoundPageNumber = (pFRun->getFieldType() == FPFIELD_page_number);
			}
			pRun = pRun->getNextRun();
		}
		if(!bFoundPageNumber)
			pBL = static_cast<fl_BlockLayout *>(pBL->getNext());
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

		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,pos,pos,NULL,atts,PTX_Block);

		if(bInsertFromHdrFtr)
		{
			_setPoint(oldpos);
			setHdrFtrEdit(pShadow);
		}

		// Signal PieceTable Changes have finished
		_generalUpdate();
		_restorePieceTableState();
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
	pBL = static_cast<fl_BlockLayout *>(pHFSL->getFirstLayout());
	pos = pBL->getPosition();

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
	return bRet;
}


void FV_View::changeListStyle(	fl_AutoNum* pAuto,
								FL_ListType lType,
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
			vb.addItem(sdh);
			i++;
			sdh = pAuto->getNthBlock(i);
		}
		for(i=0; i< vb.getItemCount(); ++i)
		{
			PL_StruxDocHandle sdh = static_cast<PL_StruxDocHandle>(vb.getNthItem(i));
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
//
// This is depeciated..
	va.addItem( PT_STYLE_ATTRIBUTE_NAME);	va.addItem( style);

	pAuto->setListType(lType);
	sprintf(pszStart, "%i" , startv);
	UT_XML_strncpy( pszAlign,
					sizeof(pszAlign),
					UT_convertInchesToDimensionString(DIM_IN, Align, 0));

	UT_XML_strncpy( pszIndent,
					sizeof(pszIndent),
					UT_convertInchesToDimensionString(DIM_IN, Indent, 0));

	vp.addItem( "start-value");	vp.addItem( pszStart);
	vp.addItem( "margin-left");	vp.addItem( pszAlign);
	vp.addItem( "text-indent");	vp.addItem( pszIndent);
	vp.addItem( "list-style"); 	vp.addItem( style);
	pAuto->setStartValue(startv);
	if(pszDelim != NULL)
	{
		vp.addItem( "list-delim"); vp.addItem( pszDelim);
		pAuto->setDelim(pszDelim);
	}
	if(pszDecimal != NULL)
	{
		vp.addItem( "list-decimal"); vp.addItem( pszDecimal);
		pAuto->setDecimal(pszDecimal);
	}
	if(pszFont != NULL)
	{
		vp.addItem( "field-font"); vp.addItem( pszFont);
	}
	//
	// Assemble the List attributes
	//
	UT_uint32 counta = va.getItemCount() + 1;
	const XML_Char ** attribs = static_cast<const XML_Char **>(UT_calloc(counta, sizeof(XML_Char *)));
	for(i=0; i<va.getItemCount();i++)
	{
		attribs[i] = static_cast<XML_Char *>(va.getNthItem(i));
	}
	attribs[i] = static_cast<XML_Char *>(NULL);
	//
	// Now assemble the list properties
	//
	UT_uint32 countp = vp.getItemCount() + 1;
	const XML_Char ** props = static_cast<const XML_Char **>(UT_calloc(countp, sizeof(XML_Char *)));
	for(i=0; i<vp.getItemCount();i++)
	{
		props[i] = static_cast<XML_Char *>(vp.getNthItem(i));
	}
	props[i] = static_cast<XML_Char *>(NULL);

	i = 0;
	sdh = static_cast<PL_StruxDocHandle>(pAuto->getNthBlock(i));
	while(sdh != NULL)
	{
		PT_DocPosition iPos = m_pDoc->getStruxPosition(sdh)+fl_BLOCK_STRUX_OFFSET;
//		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, iPos, iPos, attribs, props, PTX_Block);
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, iPos, iPos, NULL, props, PTX_Block);
		i++;
		sdh = static_cast<PL_StruxDocHandle>(pAuto->getNthBlock(i));
		_generalUpdate();
	}

	_generalUpdate();
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	_ensureInsertionPointOnScreen();
	FREEP(attribs);
	FREEP(props);
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
	bool b = m_SecProps.isValid();
	if((AV_View::getTick() == m_SecProps.getTick()) && m_SecProps.isValid())
	{
		xxx_UT_DEBUGMSG(("GOt a valid cache for section props \n"));
		*pProps = m_SecProps.getCopyOfProps();
		return true;
	}
	m_SecProps.clearProps();
	m_SecProps.setTick(AV_View::getTick());
	b = m_SecProps.isValid();
	
	if(getLayout()->getFirstSection() == NULL)
	{
		return false;
	}

	if (!isSelectionEmpty())
	{
		if (m_Selection.getSelectionAnchor() < posStart)
			posStart = m_Selection.getSelectionAnchor();
		else
			posEnd = m_Selection.getSelectionAnchor();
	}

	// 1. assemble complete set at insertion point
	fl_BlockLayout* pBlock = _findBlockAtPosition(posStart);
	fl_SectionLayout* pSection = pBlock->getSectionLayout();
	pSection->getAttrProp(&pSectionAP);

	UT_uint32 iPropsCount = PP_getPropertyCount();
	for(UT_uint32 n = 0; n < iPropsCount; n++)
	{
		if((PP_getNthPropertyLevel(n) & PP_LEVEL_SECT))
		{
			f = new _fmtPair(PP_getNthPropertyName(n),NULL,pBlockAP,pSectionAP,m_pDoc,false);
			if(f->m_val != NULL)
				v.addItem(static_cast<void *>(f));
			else
				delete f;
		}
	}

	// 2. prune 'em as they vary across selection
	if (!isSelectionEmpty())
	{
		fl_BlockLayout* pBlockEnd = _findBlockAtPosition(posEnd);
		fl_SectionLayout *pSectionEnd = pBlockEnd->getSectionLayout();

		while (pSection && (pSection != pSectionEnd))
		{
			const PP_AttrProp * pAP;
			bool bCheck = false;

			pSection = static_cast<fl_SectionLayout *>(pSection->getNext());
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
					f = static_cast<_fmtPair *>(v.getNthItem(i-1));

					const XML_Char * value = PP_evalProperty(f->m_prop,NULL,pBlockAP,pSectionAP,m_pDoc,false);

					// prune anything that doesn't match
					if(f->m_val == NULL  || value == NULL)
					{
						DELETEP(f);
						v.deleteNthItem(i-1);
					}
					else if (strcmp(f->m_val, value))
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
	const XML_Char ** props = static_cast<const XML_Char **>(UT_calloc(count, sizeof(XML_Char *)));
	if (!props)
		return false;

	const XML_Char ** p = props;

	i = v.getItemCount();
	UT_uint32 numProps = count;
	while (i > 0)
	{
		f = static_cast<_fmtPair *>(v.getNthItem(i-1));
		i--;

		p[0] = f->m_prop;
		p[1] = f->m_val;
		p += 2;
	}
	p[0] = NULL;
	UT_VECTOR_PURGEALL(_fmtPair *,v);

	*pProps = props;
	m_SecProps.fillProps(numProps,props);
	b = m_SecProps.isValid();

	return true;
}

/*!
 * Set a string with the cell properties of the cell located at position pos
 */
bool FV_View::getCellFormat(PT_DocPosition pos, UT_String & sCellProps)
{
	sCellProps.clear();
	if(!isInTable(pos))
	{
		return false;
	}
	const PP_AttrProp * pCellAP = NULL;
	fl_BlockLayout * pBL =  _findBlockAtPosition(pos);
	if(!pBL)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	fl_SectionLayout * pCell = static_cast<fl_CellLayout *>(pBL->myContainingLayout());
	if(!pCell)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	pCell->getAttrProp(&pCellAP);

	UT_sint32 iPropsCount = PP_getPropertyCount();
	UT_sint32 n = 0;
	UT_String sPropName;
	UT_String sPropVal;
	const XML_Char * pszPropVal;
	for(n = 0; n < iPropsCount; n++)
	{
		if((PP_getNthPropertyLevel(n) & PP_LEVEL_TABLE))
		{
			sPropName = PP_getNthPropertyName(n);
			sPropVal.clear();
			bool bFound = pCellAP->getProperty(sPropName.c_str(),pszPropVal);
			if(bFound)
			{
				sPropVal = pszPropVal;
				UT_String_setProperty(sCellProps,sPropName,sPropVal);
			}
		}
	}
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
// fixme

	*pProps = NULL;

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
	if((AV_View::getTick() == m_BlockProps.getTick()) && m_BlockProps.isValid())
	{
		*pProps = m_BlockProps.getCopyOfProps();
		return true;
	}
	m_BlockProps.clearProps();
	m_BlockProps.setTick(AV_View::getTick());
	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_Selection.getSelectionAnchor() < posStart)
			posStart = m_Selection.getSelectionAnchor();
		else
			posEnd = m_Selection.getSelectionAnchor();
	}

	// 1. assemble complete set at insertion point
	fl_BlockLayout* pBlock = _findBlockAtPosition(posStart);
	pBlock->getAttrProp(&pBlockAP);

	fl_SectionLayout* pSection = pBlock->getSectionLayout();
	pSection->getAttrProp(&pSectionAP);


	UT_uint32 iPropsCount = PP_getPropertyCount();
	for(UT_uint32 n = 0; n < iPropsCount; n++)
	{
		if((PP_getNthPropertyLevel(n) & PP_LEVEL_BLOCK))
		{
			f = new _fmtPair(PP_getNthPropertyName(n),NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
			if(f->m_val != NULL)
				v.addItem(static_cast<void *>(f));
			else
				delete f;
		}
	}

	// 2. prune 'em as they vary across selection
	if (!isSelectionEmpty())
	{
		fl_BlockLayout* pBlockEnd = _findBlockAtPosition(posEnd);

		while (pBlock && (pBlock != pBlockEnd))
		{
			const PP_AttrProp * pAP;
			bool bCheck = false;

			pBlock = static_cast<fl_BlockLayout *>(pBlock->getNextBlockInDocument());
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
					f = static_cast<_fmtPair *>(v.getNthItem(i-1));

					const XML_Char * value = PP_evalProperty(f->m_prop,NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
					UT_ASSERT(value);

					// prune anything that doesn't match
					if (strcmp(f->m_val, value))
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
	const XML_Char ** props = static_cast<const XML_Char **>(UT_calloc(count, sizeof(XML_Char *)));
	if (!props)
		return false;

	const XML_Char ** p = props;

	i = v.getItemCount();
	UT_uint32 numProps = count;
	while (i > 0)
	{
		f = static_cast<_fmtPair *>(v.getNthItem(i-1));
		i--;

		p[0] = f->m_prop;
		p[1] = f->m_val;
		p += 2;
	}
	p[0] = NULL;
	UT_VECTOR_PURGEALL(_fmtPair *,v);

	*pProps = props;
	m_BlockProps.fillProps(numProps,props);

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

	UT_sint32 selLength = static_cast<UT_sint32>(labs(m_iInsPoint - m_Selection.getSelectionAnchor()));

	PT_DocPosition low;
	if (m_iInsPoint > m_Selection.getSelectionAnchor())
	{
		low = m_Selection.getSelectionAnchor();
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

		PT_DocPosition offset = 0;
		if( low >= block->getPosition(false) )
		{
			offset = low - block->getPosition(false);
		}

		// allow no more than the rest of the block
		if (offset + static_cast<UT_uint32>(selLength) > buffer.getLength())
		{
			selLength = static_cast<UT_sint32>(buffer.getLength()) - static_cast<UT_sint32>(offset);
		}
		// give us space for our new chunk of selected text, add 1 so it
		// terminates itself
		if(selLength < 0)
		{
			selLength = 0;
		}
		bufferSegment = static_cast<UT_UCSChar *>(UT_calloc(selLength + 1, sizeof(UT_UCSChar)));

		// copy it out
		memmove(bufferSegment, buffer.getPointer(offset), selLength * sizeof(UT_UCSChar));

	 return bufferSegment;
	}

	return NULL;
}

// this function has not been debugged
UT_UCSChar * FV_View::getTextBetweenPos(PT_DocPosition pos1, PT_DocPosition pos2)
{
#if 1
	UT_DEBUGMSG(("DOM: getTextBetweenPos not fully debugged\n"));
	UT_return_val_if_fail(UT_TODO, NULL);

#else

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
		UT_uint32 iLenToCopy = UT_MIN(pos2 - curPos, buffer.getLength() - offset);
		while(curPos < pos2 && (curPos < pBlock->getPosition(false) + pBlock->getLength()))
		{
			memmove(buff_ptr, buffer.getPointer(offset), iLenToCopy * sizeof(UT_UCSChar));
			offset	 += iLenToCopy;
			buff_ptr += iLenToCopy;
			curPos	 += iLenToCopy;
		}

		pBlock = static_cast<fl_BlockLayout *>(pBlock->getNext());
	}

	UT_ASSERT(curPos == pos2);
	return bufferRet;
#endif
}

bool FV_View::isTabListBehindPoint(void)
{
	PT_DocPosition cpos = getPoint();
	PT_DocPosition ppos = cpos - 1;
	PT_DocPosition posBOD;
	bool bRes;
	bool bEOL = false;
	UT_uint32 iPointHeight;
	UT_sint32 xPoint, yPoint, xPoint2, yPoint2;
	bool   bDirection;

	bRes = getEditableBounds(false, posBOD);
	UT_ASSERT(bRes);
	if (cpos <= posBOD - 1)
	{
		return false;
	}

	fl_BlockLayout* pBlock;
	fp_Run * pRun;
	_findPositionCoords(cpos, bEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection, &pBlock, &pRun);

	if (!pBlock)
		return false;
	if(pBlock->isListItem() == false)
		return false;

	fl_BlockLayout* ppBlock;
	_findPositionCoords(ppos, bEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection, &ppBlock, &pRun);

	if (!ppBlock || pBlock != ppBlock)
	{
		return false;
	}

	if(pRun->getType() != FPRUN_TAB)
	{
		return false;
	}
	pRun = pRun->getPrevRun();
	while((pRun != NULL) && (pRun->getType()== FPRUN_FMTMARK))
	{
		pRun = pRun->getPrevRun();
	}
	if (!pRun || pRun->getType() != FPRUN_FIELD)
	{
		return false;
	}
	else
	{
		fp_FieldRun * pFRun = static_cast<fp_FieldRun *>(pRun);
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

	bool bEOL = false;
	UT_uint32 iPointHeight;
	UT_sint32 xPoint, yPoint, xPoint2, yPoint2;
	bool   bDirection;

	fl_BlockLayout* pBlock;
	fp_Run* pRun;
	_findPositionCoords(cpos, bEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection, &pBlock, &pRun);

	if (!pBlock || pBlock->isListItem() == false)
	{
		return false;
	}

	// Find first run that is not an FPRUN_FMTMARK
	while (pRun && (pRun->getType() == FPRUN_FMTMARK))
	{
		pRun = pRun->getNextRun();
	}

	if (!pRun || pRun->getType() != FPRUN_FIELD)
	{
		return false;
	}

	fp_FieldRun * pFRun = static_cast<fp_FieldRun *>(pRun);
	if(pFRun->getFieldType() != FPFIELD_list_label)
	{
		return false;
	}

	pRun = pRun->getNextRun();
	while (pRun && (pRun->getType()== FPRUN_FMTMARK))
	{
		pRun = pRun->getNextRun();
	}
	if (!pRun || pRun->getType() != FPRUN_TAB)
	{
		return false;
	}

	return true;
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
		return;
	}

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
		return;
	}

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
		return;
	}

	_resetSelection();
	_clearIfAtFmtMark(getPoint());
	_moveInsPtNextPrevLine(bNext);

	notifyListeners(AV_CHG_MOTION);
}

void FV_View::extSelNextPrevLine(bool bNext)
{
	if (isSelectionEmpty())
	{
		_setSelectionAnchor();
		_clearIfAtFmtMark(getPoint());
		_moveInsPtNextPrevLine(bNext);
		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
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
		_setSelectionAnchor();
		_clearIfAtFmtMark(getPoint());
		_moveInsPtNextPrevScreen(bNext);

		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
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
		}
	}

	notifyListeners(AV_CHG_MOTION);
}

void FV_View::extSelNextPrevPage(bool bNext)
{
	if (isSelectionEmpty())
	{
		_setSelectionAnchor();
		_clearIfAtFmtMark(getPoint());
		_moveInsPtNextPrevPage(bNext);

		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
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
		}
	}

	notifyListeners(AV_CHG_MOTION);
}

void FV_View::extSelHorizontal(bool bForward, UT_uint32 count)
{
	if (isSelectionEmpty())
	{
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

	_ensureInsertionPointOnScreen();

	// It IS possible for the selection to be empty, even
	// after extending it.	If the charMotion fails, for example,
	// because we are at the end of a document, then the selection
	// will end up empty once again.

	if (isSelectionEmpty())
	{
		_resetSelection();
	}
	else
	{
		_drawSelection();
	}

	notifyListeners(AV_CHG_MOTION);
}

void FV_View::endDragSelection(UT_sint32 xpos, UT_sint32 ypos)
{
	//
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	// Turn off list updates

	m_pDoc->disableListUpdates();

	// turn off immediate layout of table

	m_pDoc->setDontImmediatelyLayout(true);

	
	m_pDoc->beginUserAtomicGlob();
			
	PT_DocPosition pos = getDocPositionFromXY(xpos, ypos);
		
	cmdCut();
	moveInsPtTo(pos);
	cmdPaste();
	
	m_pDoc->endUserAtomicGlob();
	
	// Allow updates

	m_pDoc->setDontImmediatelyLayout(false);

	_generalUpdate();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
}

void FV_View::extSelTo(FV_DocPos dp)
{
	PT_DocPosition iPos = _getDocPos(dp);

	_extSelToPos(iPos);

	if (!_ensureInsertionPointOnScreen())
	{
		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
		}
	}

	notifyListeners(AV_CHG_MOTION);
}


/*!
 * This method returns the document position at xpos,ypos on the screen.
 */
PT_DocPosition FV_View::getDocPositionFromXY(UT_sint32 xpos, UT_sint32 ypos, bool bNotFrames)
{
	// Figure out which page we clicked on.
	// Pass the click down to that page.
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xpos, ypos, xClick, yClick);

	PT_DocPosition iNewPoint;
	bool bBOL = false;
	bool bEOL = false;
	pPage->mapXYToPosition(bNotFrames,xClick, yClick, iNewPoint, bBOL, bEOL, true,NULL);
	return iNewPoint;
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
	pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL, true);

	bool bPostpone = false;

	if (bDrag)
	{
		// figure out whether we're still on screen
		bool bOnScreen = true;

		if ((xPos < 0 || xPos > getWindowWidth()) ||
			(yPos < 0 || yPos > getWindowHeight()))
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
	pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL, true);

	//UT_ASSERT(!isSelectionEmpty());

	if (iNewPoint <= m_Selection.getSelectionLeftAnchor())
	{
		m_Selection.setSelectionAnchor(m_Selection.getSelectionRightAnchor());
	}
	else
	{
		m_Selection.setSelectionAnchor(m_Selection.getSelectionLeftAnchor());
	}

	const FV_DocPos argDocPos =
		iNewPoint > m_Selection.getSelectionAnchor() ? FV_DOCPOS_EOW_SELECT : FV_DOCPOS_BOW;
	const PT_DocPosition iNewPointWord = _getDocPosFromPoint(iNewPoint, argDocPos,false);

	bool bPostpone = false;

	if (bDrag)
	{
		// figure out whether we're still on screen
		bool bOnScreen = true;

		if ((xPos < 0 || xPos > getWindowWidth()) ||
			(yPos < 0 || yPos > getWindowHeight()))
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

	if ((xPos < 0 || xPos > getWindowWidth()) ||
		(yPos < 0 || yPos > getWindowHeight()))
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

	char * numberString = static_cast<char *>(UT_calloc(UT_UCS4_strlen(data) + 1, sizeof(char)));
	UT_ASSERT(numberString);
	char * origNum = numberString;

	UT_UCS4_strcpy_to_char(numberString, data);
	if (!isSelectionEmpty())
	{
		_clearSelection();
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
	//	if (number < 0 || number > static_cast<UT_uint32>(m_pLayout->countPages()))
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
			fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pSL->getFirstLayout());
			fp_Line* pLine = static_cast<fp_Line *>(pBL->getFirstContainer());

			for (UT_uint32 i = 1; i < number; i++)
			{
				fp_Line* pOldLine = pLine;

				if ((pLine = static_cast<fp_Line *>(pLine->getNext ())) == NULL)
				{
					if ((pBL = static_cast<fl_BlockLayout *>(pBL->getNext ())) == NULL)
					{
						if ((pSL = static_cast<fl_SectionLayout *>(pSL->getNext ())) == NULL)
						{
							pLine = pOldLine;
							break;
						}
						else
							pBL = static_cast<fl_BlockLayout *>(pSL->getFirstLayout ());
					}
					else
					{
						pLine = static_cast<fp_Line *>(pBL->getFirstContainer());
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
				XAP_Frame * pFrame = static_cast<XAP_Frame *>(getParentData());
				UT_ASSERT((pFrame));
				pFrame->openURL(numberString);
				return false;
			}
			if(m_pDoc->isBookmarkUnique(static_cast<const XML_Char *>(numberString)))
				goto book_mark_not_found; //bookmark does not exist

			while(pSL)
			{
				pBL = static_cast<fl_BlockLayout *>(pSL->getFirstLayout());

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
						pRun = pRun->getNextRun();
					}
					if(bFound)
						break;
					pBL = static_cast<fl_BlockLayout *>(pBL->getNext());
				}
				if(bFound)
					break;
				pSL = static_cast<fl_SectionLayout *>(pSL->getNext());
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
					_setPoint(dp2);
					_setSelectionAnchor();
					setPoint(dp1);
					_drawSelection();
				}

			}
			else

book_mark_not_found:
			{
				//bookmark not found
				XAP_Frame * pFrame = static_cast<XAP_Frame *>(getParentData());
				UT_ASSERT((pFrame));

				pFrame->showMessageBox(AP_STRING_ID_MSG_BookmarkNotFound, 
						       XAP_Dialog_MessageBox::b_O, 
						       XAP_Dialog_MessageBox::a_OK, 
						       numberString);

				return true;
			}

		}

		notifyListeners(AV_CHG_MOTION);
		break;
	default:
		// TODO
		;
	}

	FREEP(origNum);

	_ensureInsertionPointOnScreen();

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
FV_View::findNext(const UT_UCSChar* pFind, bool& bDoneEntireDocument)

{
	findSetFindString(pFind);
	return findNext(bDoneEntireDocument);
}

bool
FV_View::findNext(bool& bDoneEntireDocument)
{
	if ((m_startPosition >=0) && (m_startPosition <2)) {
		m_startPosition = 2;
		setPoint(m_startPosition);
		}

	if (!isSelectionEmpty())
	{
		_clearSelection();
	}

	UT_uint32* pPrefix = _computeFindPrefix(m_sFind);
	bool bRes = _findNext(pPrefix, bDoneEntireDocument);
	FREEP(pPrefix);

	if (isSelectionEmpty())
	{
		_updateInsertionPoint();
	}
	else
	{
		_ensureInsertionPointOnScreen();
		_drawSelection();
	}

	// TODO do we need to do a notifyListeners(AV_CHG_MOTION) -- yes
	notifyListeners(AV_CHG_MOTION);
	return bRes;
}


bool
FV_View::findPrev(const UT_UCSChar* pFind, bool& bDoneEntireDocument)
{
	findSetFindString(pFind);
	return findPrev(bDoneEntireDocument);
}

bool
FV_View::findPrev(bool& bDoneEntireDocument)
{
	if (!isSelectionEmpty())
	{
		_clearSelection();
	}

	UT_uint32* pPrefix = _computeFindPrefix(m_sFind);
	bool bRes = _findPrev(pPrefix, bDoneEntireDocument);
	FREEP(pPrefix);

	if (isSelectionEmpty())
	{
		_updateInsertionPoint();
	}
	else
	{
		_ensureInsertionPointOnScreen();
		_drawSelection();
	}

	notifyListeners(AV_CHG_MOTION);
	return bRes;
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



void
FV_View::findSetFindString(const UT_UCSChar* pFind)
{
	FREEP(m_sFind);
	UT_UCS4_cloneString(&m_sFind, pFind);
}

UT_UCSChar * 
FV_View::findGetFindString(void)
{
	UT_UCSChar * string = NULL;
	if (m_sFind)
	{
		if (UT_UCS4_cloneString(&string, m_sFind))
			return string;
	}
	else
	{
		if (UT_UCS4_cloneString_char(&string, ""))
			return string;
	}

	return NULL;
}

void
FV_View::findSetReplaceString(const UT_UCSChar* pReplace)
{
	FREEP(m_sReplace);
	UT_UCS4_cloneString(&m_sReplace, pReplace);
}

UT_UCSChar * 
FV_View::findGetReplaceString(void)
{
	UT_UCSChar * string = NULL;
	if (m_sReplace)
	{
		if (UT_UCS4_cloneString(&string, m_sReplace))
			return string;
	}
	else
	{
		if (UT_UCS4_cloneString_char(&string, ""))
			return string;
	}

	return NULL;
}

void
FV_View::findSetReverseFind	(bool newValue)
{
	m_bReverseFind = newValue;
}

bool
FV_View::findGetReverseFind	()
{
	return m_bReverseFind;
}

void
FV_View::findSetMatchCase(bool newValue)
{
	m_bMatchCase = newValue;
}

bool
FV_View::findGetMatchCase()
{
	return m_bMatchCase;
}

void
FV_View::findSetWholeWord(bool newValue)
{
	m_bWholeWord = newValue;
}

bool
FV_View::findGetWholeWord()
{
	return m_bWholeWord;
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

	if (m_sFind && *m_sFind)
	{
		bool bTmp;
		if (m_bReverseFind)
		{
			bRes = findPrev(bTmp);
		}
		else
		{
			bRes = findNext(bTmp);
		}
			
		if (bRes)
		{
			_drawSelection();
		}
	}

	return bRes;
}

bool
FV_View::findReplaceReverse(bool& bDoneEntireDocument)
{
	UT_ASSERT(m_sFind && m_sReplace);

	UT_uint32* pPrefix = _computeFindPrefix(m_sFind);
	bool bRes = _findReplaceReverse(pPrefix, bDoneEntireDocument);
	FREEP(pPrefix);

	updateScreen();

	if (isSelectionEmpty())
	{
		_updateInsertionPoint();
	}
	else
	{
		_ensureInsertionPointOnScreen();
		_drawSelection();
	}

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
FV_View::findReplace(bool& bDoneEntireDocument)
{
	UT_ASSERT(m_sFind && m_sReplace);

	UT_uint32* pPrefix = _computeFindPrefix(m_sFind);
	bool bRes = _findReplace(pPrefix, bDoneEntireDocument);
	FREEP(pPrefix);

	updateScreen();

	if (isSelectionEmpty())
	{
		_updateInsertionPoint();
	}
	else
	{
		_ensureInsertionPointOnScreen();
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
FV_View::findReplaceAll()
{
	UT_uint32 iReplaced = 0;
	m_pDoc->beginUserAtomicGlob();

	if ((m_startPosition >=0) && (m_startPosition <2))
	{
		m_startPosition = 2;
	}

	bool bDoneEntireDocument = false;

	// Compute search prefix
	UT_uint32* pPrefix = _computeFindPrefix(m_sFind);

	// Prime with the first match - _findReplace is really
	// replace-then-find.
	_findNext(pPrefix, bDoneEntireDocument);

	// Keep replacing until the end of the buffer is hit
	while (!bDoneEntireDocument)
	{
		_findReplace(pPrefix, bDoneEntireDocument);
		iReplaced++;
	}

	m_pDoc->endUserAtomicGlob();

	_generalUpdate();

	if (isSelectionEmpty())
	{
		_updateInsertionPoint();
	}
	else
	{
		_ensureInsertionPointOnScreen();
	}

	FREEP(pPrefix);
	return iReplaced;
}


fl_BlockLayout*
FV_View::getCurrentBlock(void)
{
	return _findGetCurrentBlock();
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

		UT_sint32 xPoint, yPoint, xPoint2, yPoint2, iPointHeight;
		bool bDirection;

		fl_BlockLayout * pBL =	m_pLayout->findBlockAtPosition(getPoint());
		fp_Run * pRun;
		if(pBL == NULL)
		{
			return;
		}
		pRun = pBL->findPointCoords(getPoint(), false, xPoint,
							    yPoint, xPoint2, yPoint2,
							    iPointHeight, bDirection);
		if(pRun && pRun->getPrevRun())
		{
			pRun->getPrevRun()->markAsDirty();
		}
		_generalUpdate();
	}
	else
	{
		// Just insert the character

		cmdCharInsert(&c,1);

		UT_sint32 xPoint, yPoint, xPoint2, yPoint2, iPointHeight;
		bool bDirection;

		fl_BlockLayout * pBL =	m_pLayout->findBlockAtPosition(getPoint());
		fp_Run * pRun;
		if(pBL == NULL)
		{
			return;
		}
		pRun = pBL->findPointCoords(getPoint(), false, xPoint,
							    yPoint, xPoint2, yPoint2,
							    iPointHeight, bDirection);
		if(pRun && pRun->getPrevRun())
		{
			pRun->getPrevRun()->markAsDirty();
		}
	}
	m_pDoc->endUserAtomicGlob();
}


void FV_View::warpInsPtToXY(UT_sint32 xPos, UT_sint32 yPos, bool bClick = false)
{
	/*
	  Figure out which page we clicked on.
	  Pass the click down to that page.
	*/

	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);

	if (!isSelectionEmpty())
		_clearSelection();
	PT_DocPosition pos,posEnd;
	bool bBOL = false;
	bool bEOL = false;
	fl_HdrFtrShadow * pShadow=NULL;
	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL, true, &pShadow);
	if(bClick)
	{
		getEditableBounds(true,posEnd,true);
		if((pos > posEnd) && (pShadow != NULL))
		{
			if (pos != getPoint())
				_clearIfAtFmtMark(getPoint());
			setHdrFtrEdit(pShadow);
			bClick = true;
		}
		else if((pos > posEnd) && (pShadow == NULL))
		{
			bClick = false;
			pos = posEnd;
		}
		else if(pos <= posEnd) 
		{
			bClick = false;
			clearHdrFtrEdit();
		}
	}
	if ((pos != getPoint()) && !bClick)
		_clearIfAtFmtMark(getPoint());


	_setPoint(pos, bEOL);
	setCursorToContext();
	notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR ); // Sevior Put this in
//	notifyListeners(AV_CHG_HDRFTR );

}


void FV_View::getPageScreenOffsets(const fp_Page* pThePage, UT_sint32& xoff,
								   UT_sint32& yoff)
{
	UT_uint32 y = getPageViewTopMargin();

	const fp_Page* pPage = m_pLayout->getFirstPage();
	fl_DocSectionLayout * pDSL = pPage->getOwningSection();
//
// Note this code assumes the page size is the same throughout the document.
//
	UT_sint32 iPage = m_pLayout->findPage(const_cast<fp_Page *>(pThePage));
	UT_sint32 iDiff = pPage->getHeight() + getPageViewSep();
	if(getViewMode() != VIEW_PRINT)
	{
		iDiff = iDiff - pDSL->getTopMargin() - pDSL->getBottomMargin();
	}
	if(iPage > 0)
	{
		iDiff = iDiff*iPage;
	}
	else
	{
		iDiff = 0;
	}
	y += iDiff;
//
// This code will work for different page size but it's slow for big docs
//
#if 0
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
#endif
	yoff = y - m_yScrollOffset;
	xoff = getPageViewLeftMargin() - m_xScrollOffset;
}

void FV_View::getPageYOffset(fp_Page* pThePage, UT_sint32& yoff) const
{
	UT_uint32 y = getPageViewTopMargin();
//
// Note this code assumes the page size is the same throughout the document.
//
	UT_sint32 iPage = m_pLayout->findPage(pThePage);
	fp_Page* pPage = m_pLayout->getFirstPage();
	fl_DocSectionLayout * pDSL = pPage->getOwningSection();
	UT_sint32 iDiff = pPage->getHeight() + getPageViewSep();
	if(getViewMode() != VIEW_PRINT)
	{
		iDiff = iDiff - pDSL->getTopMargin() - pDSL->getBottomMargin();
	}
	if(iPage > 0)
	{
		iDiff = iDiff*iPage;
	}
	else
	{
		iDiff = 0;
	}
	y += iDiff;
//
// This code will work for different page size but it's slow for big docs
//
#if 0
	while (pPage)
	{
		if (pPage == pThePage)
		{
			break;
		}
		y += pPage->getHeight() + getPageViewSep();
		if(getViewMode() != VIEW_PRINT)
		{
			y = y - pDSL->getTopMargin() - pDSL->getBottomMargin();
		}

		pPage = pPage->getNext();
	}
#endif
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
		return m_pG->tlu(1);
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

void FV_View::setXScrollOffset(UT_sint32 v)
{
	CHECK_WINDOW_SIZE
	UT_sint32 dx = v - m_xScrollOffset;

	if (dx == 0)
		return;

	m_pG->scroll(dx, 0);
	m_xScrollOffset = v;

	UT_sint32 x1 = 0;
	UT_sint32 dx2 = getWindowWidth();

	if (dx > 0)
	{
		if (dx < getWindowWidth())
		{
			x1 = getWindowWidth() - dx;
			dx2 = dx;
		}
	}
	else
	{
		if (dx > -getWindowWidth())
		{
			dx2 = -dx;
		}
	}

	_draw(x1-m_pG->tlu(1), 0, dx2+m_pG->tlu(2), getWindowHeight(), false, true);

	_fixInsertionPointCoords();
}

void FV_View::setYScrollOffset(UT_sint32 v)
{
	CHECK_WINDOW_SIZE
	UT_sint32 dy = v - m_yScrollOffset;

	if (dy == 0)
		return;

	m_pG->scroll(0, dy);
	m_yScrollOffset = v;

	UT_sint32 y1 = 0;
	UT_sint32 dy2 = getWindowHeight();

	if (dy > 0)
	{
		if (dy < getWindowHeight())
		{
			y1 = getWindowHeight() - dy;
			dy2 = dy;
		}
	}
	else
	{
		if (dy > -getWindowHeight())
		{
			dy2 = -dy;
		}
	}

	_draw(0, y1, getWindowWidth(), dy2, false, true);

	_fixInsertionPointCoords();
}

void FV_View::draw(int page, dg_DrawArgs* da)
{
	UT_DEBUGMSG(("FV_View::draw_1: [page %ld]\n",page));

	if(getPoint() == 0) {
		return;
	}

	UT_ASSERT(da->pG);
	fp_Page* pPage = m_pLayout->getNthPage(page);
	if (pPage)
	{
		pPage->draw(da);
	}
}

/*!
    The rectangle is in device coordinances
*/
void FV_View::draw(const UT_Rect* pClipRect)
{
	if(getPoint() == 0) {
		return;
	}

	if (pClipRect)
	{
		_draw(pClipRect->left,
			  pClipRect->top,
			  pClipRect->width,
			  pClipRect->height,
			  false,true);
	}
	else
	{
		_draw(0,0,getWindowWidth(),getWindowHeight(),false,false);
	}
	_fixInsertionPointCoords();
}

void FV_View::updateScreen(bool bDirtyRunsOnly)
{
	_draw(0,0,getWindowWidth(),getWindowHeight(),bDirtyRunsOnly,false);
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
	pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL, true);
	return bBOL;
}

void FV_View::ensureInsertionPointOnScreen(void)
{
	_ensureInsertionPointOnScreen();
}


void FV_View::setPoint(PT_DocPosition pt)
{
	_setPoint(pt, m_bPointEOL);
}

/* Passes through to the document's don't-change-ins-point method. */
void FV_View::setDontChangeInsPoint(void)
{
	m_pDoc->setDontChangeInsPoint();
}

void FV_View::allowChangeInsPoint(void)
{
	m_pDoc->allowChangeInsPoint();
}

bool FV_View::canDo(bool bUndo) const
{
	return m_pDoc->canDo(bUndo);
}

UT_uint32 FV_View::undoCount (bool bUndo) const
{
  return m_pDoc->undoCount ( bUndo );
}

/*!
 * Returns true of the point presented is within a selection.
 */
bool FV_View::isPosSelected(PT_DocPosition pos) const
{
	return m_Selection.isPosSelected(pos);
}

FV_SelectionMode FV_View::getSelectionMode(void) const
{
	return m_Selection.getSelectionMode();
}

FV_SelectionMode FV_View::getPrevSelectionMode(void) const
{
	return m_Selection.getPrevSelectionMode();
}

PD_DocumentRange * FV_View::getNthSelection(UT_sint32 i)
{
	return m_Selection.getNthSelection(i);
}

UT_sint32 FV_View::getNumSelections(void) const
{
	return m_Selection.getNumSelections();
}

void FV_View::getDocumentRangeOfCurrentSelection(PD_DocumentRange * pdr)
{
	PT_DocPosition iPos1, iPos2;
	if (m_Selection.getSelectionAnchor() < getPoint())
	{
		iPos1 = m_Selection.getSelectionAnchor();
		iPos2 = getPoint();
	}
	else
	{
		iPos1 = getPoint();
		iPos2 = m_Selection.getSelectionAnchor();
	}

	pdr->set(m_pDoc,iPos1,iPos2);
	return;
}


/*!
 * Return the left,right,top and bottom attach points of the cell containing
 * the position cellPos
 */
bool FV_View::getCellParams(PT_DocPosition posCell, UT_sint32 * pLeft, UT_sint32 * pRight,
							 UT_sint32 * pTop, UT_sint32 * pBot)
{
	PL_StruxDocHandle cellSDH;
	bool bres = m_pDoc->getStruxOfTypeFromPosition(posCell,PTX_SectionCell,&cellSDH);
	if(!bres)
	{
		return false;
	}
	const char * pszLeft;
	const char * pszRight;
	const char * pszTop;
	const char * pszBot;
	m_pDoc->getPropertyFromSDH(cellSDH,"left-attach",&pszLeft);
	if(pszLeft && *pszLeft)
	{
		*pLeft = atoi(pszLeft);
	}
	else
	{
		return false;
	}
	m_pDoc->getPropertyFromSDH(cellSDH,"right-attach",&pszRight);
	if(pszRight && *pszRight)
	{
		*pRight = atoi(pszRight);
	}
	else
	{
		return false;
	}
	m_pDoc->getPropertyFromSDH(cellSDH,"top-attach",&pszTop);
	if(pszTop && *pszTop)
	{
		*pTop = atoi(pszTop);
	}
	else
	{
		return false;
	}
	m_pDoc->getPropertyFromSDH(cellSDH,"bot-attach",&pszBot);
	if(pszBot && *pszBot)
	{
		*pBot = atoi(pszBot);
	}
	else
	{
		return false;
	}
	return true;
}

/*!
 * Return the left,right,top and bottom line styles of the cell containing
 * the position cellPos. Values will be -1 if the style is not set.
 */
bool FV_View::getCellLineStyle(PT_DocPosition posCell, UT_sint32 * pLeft, UT_sint32 * pRight,
							 UT_sint32 * pTop, UT_sint32 * pBot)
{
	PL_StruxDocHandle cellSDH;
	bool bres = m_pDoc->getStruxOfTypeFromPosition(posCell,PTX_SectionCell,&cellSDH);
	if(!bres)
	{
		return false;
	}
	const char * pszLeft;
	const char * pszRight;
	const char * pszTop;
	const char * pszBot;
	m_pDoc->getPropertyFromSDH(cellSDH,"left-style",&pszLeft);
	if(pszLeft && *pszLeft)
	{
		*pLeft = atoi(pszLeft);
	}
	else
	{
		*pLeft = -1;
	}
	m_pDoc->getPropertyFromSDH(cellSDH,"right-style",&pszRight);
	if(pszRight && *pszRight)
	{
		*pRight = atoi(pszRight);
	}
	else
	{
		*pRight = -1;
	}
	m_pDoc->getPropertyFromSDH(cellSDH,"top-style",&pszTop);
	if(pszTop && *pszTop)
	{
		*pTop = atoi(pszTop);
	}
	else
	{
		*pTop = -1;
	}
	m_pDoc->getPropertyFromSDH(cellSDH,"bottom-style",&pszBot);
	if(pszBot && *pszBot)
	{
		*pBot = atoi(pszBot);
	}
	else
	{
		*pBot = -1;
	}
	return true;
}

/*!
 Set cells in a table to a given format. The formatting of the current selection, row, 
 column or the whole table can be changed.
 \param properties the new cell format
 \param applyTo the range to apply the changes to
 \return True if the operation was succesful, false otherwise
 */
bool FV_View::setCellFormat(const XML_Char * properties[], FormatTable applyTo, FG_Graphic * pFG,UT_String & sDataID)
{
	bool bRet;
	setCursorWait();

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	// Turn off list updates
	m_pDoc->disableListUpdates();

	// turn off immediate layout of table. uwog this stops the formatter from prematurely building the table.
	m_pDoc->setDontImmediatelyLayout(true);

	PT_DocPosition posTable = 0;
	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;
	if (!isSelectionEmpty())
	{
		if (m_Selection.getSelectionAnchor() < posStart)
			posStart = m_Selection.getSelectionAnchor();
		else
			posEnd = m_Selection.getSelectionAnchor();
		if(posStart < 2)
		{
			posStart = 2;
		}
	}
	
	// Find the enclosing table. If just look for the first one we can get fooled by nested tables.
	PL_StruxDocHandle tableSDH;
	bRet = m_pDoc->getStruxOfTypeFromPosition(posStart+1,PTX_SectionTable,&tableSDH);
	if(!bRet)
	{
		// Allow table updates
		m_pDoc->setDontImmediatelyLayout(false);

		// Signal PieceTable Changes have finished
		_restorePieceTableState();
		clearCursorWait();
		return false;
	}
	posTable = m_pDoc->getStruxPosition(tableSDH)+1;

	// Need this to trigger a table update!
	UT_sint32 iLineType = _changeCellParams(posTable, tableSDH);
	
	// The Format Selection case needs some special attention
	if (applyTo == FORMAT_TABLE_SELECTION)
	{
		PL_StruxDocHandle cellSDH;
        bRet = m_pDoc->getStruxOfTypeFromPosition(posStart,PTX_SectionCell,&cellSDH);
        if(!bRet)
        {
//
// Might have exactly selected the table start and end struxes
//
			bRet = m_pDoc->getStruxOfTypeFromPosition(posStart+2,PTX_SectionCell,&cellSDH);
			if(!bRet)
			{
                // Allow table updates
                m_pDoc->setDontImmediatelyLayout(false);
                                                                                                                                                                                                         
                // Signal PieceTable Changes have finished
                _restorePieceTableState();
				clearCursorWait();
                return false;
			}
		}
        posStart = m_pDoc->getStruxPosition(cellSDH)+1;
		
		// Do the actual change
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,posStart,posEnd,NULL,properties,PTX_SectionCell);	
		UT_Vector vBlock;
		getBlocksInSelection(&vBlock);
		fl_ContainerLayout * pCL = NULL;
		fl_CellLayout * pCell = NULL;
		UT_uint32 i =0;
		for(i=0; i<vBlock.getItemCount();i++)
		{
			fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(vBlock.getNthItem(i));
			pCL = pBL->myContainingLayout();
			if(pCL->getContainerType() == FL_CONTAINER_CELL)
			{
				if(static_cast<fl_CellLayout *>(pCL) != pCell)
				{
					if(pFG != NULL)
					{
						pCell = static_cast<fl_CellLayout *>(pCL);
						pFG->insertAtStrux(m_pDoc,72,pBL->getPosition(),
										   PTX_SectionCell,sDataID.c_str());
					}
					else
					{
						const XML_Char * attributes[3] = {
							PT_STRUX_IMAGE_DATAID,NULL,NULL};
						bRet = m_pDoc->changeStruxFmt(PTC_RemoveFmt,pBL->getPosition(),pBL->getPosition(),attributes,NULL,PTX_SectionCell);	
						
					}
				}
			}
		}
	}
	else if(applyTo == FORMAT_TABLE_TABLE)
	{
		// Loop through the table cells to adjust their formatting		
		// get the number of rows and columns in the current table
		UT_sint32 numRows;
		UT_sint32 numCols;
		bRet = m_pDoc->getRowsColsFromTableSDH(tableSDH, &numRows, &numCols);
		UT_sint32 i;
		UT_sint32 j;
		for (j = 0; j < numRows; j++)
		{
			for (i = 0; i < numCols; i++)
			{
				PL_StruxDocHandle cellSDH = m_pDoc->getCellSDHFromRowCol(tableSDH, j, i);
				if(cellSDH)
				{
					// Remove these properties from the cell
					posStart = m_pDoc->getStruxPosition(cellSDH)+1;
					bRet = m_pDoc->changeStruxFmt(PTC_RemoveFmt,posStart,posStart,NULL,properties,PTX_SectionCell);
				}
				else
					UT_DEBUGMSG(("MARCM: Yikes! There is no cell at position (%dx%d)!\n", j, i));
			}
		}

// Now set the table format

		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,posTable,posTable,NULL,properties,PTX_SectionTable);	
	}
	else
	{
		fp_CellContainer * cell = getCellAtPos(posStart);
		if (!cell)
		{
			// Allow table updates
			m_pDoc->setDontImmediatelyLayout(false);
	
			// Signal PieceTable Changes have finished
			_restorePieceTableState();
			return false;		
		}
		UT_DEBUGMSG(("MARCM: Current cell is at (%d,%d)\n", cell->getTopAttach(), cell->getLeftAttach()));
		
		// get the number of rows and columns in the current table
		UT_sint32 numRows;
		UT_sint32 numCols;
		bRet = m_pDoc->getRowsColsFromTableSDH(tableSDH, &numRows, &numCols);
		if(!bRet)
		{
			// Allow table updates
			m_pDoc->setDontImmediatelyLayout(false);
	
			// Signal PieceTable Changes have finished
			_restorePieceTableState();
			return false;
		}	
		UT_DEBUGMSG(("MARCM: Current table size is (%dx%d)\n", numRows, numCols));		
		
		// determine the range of the cells that should be adjusted
		UT_sint32 rowStart;
		UT_sint32 rowEnd;
		UT_sint32 colStart;
		UT_sint32 colEnd;
		
		if(applyTo == FORMAT_TABLE_ROW )
		{
			rowStart = cell->getTopAttach();
			rowEnd = cell->getTopAttach();
			colStart = 0;
			colEnd = numCols-1;
		}
		else if( applyTo == FORMAT_TABLE_COLUMN)
		{
			rowStart = 0;
			rowEnd = numRows-1;
			colStart = cell->getLeftAttach();
			colEnd = cell->getLeftAttach();
		}		
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			rowEnd = -1;
			colEnd = -1;
		}
		// Loop through the table cells to adjust their formatting		
		UT_sint32 i;
		UT_sint32 j;
		for (j = rowStart; j <= rowEnd; j++)
		{
			for (i = colStart; i <= colEnd; i++)
			{
				PL_StruxDocHandle cellSDH = m_pDoc->getCellSDHFromRowCol(tableSDH, j, i);
				if(cellSDH)
				{
					// Do the actual change
					posStart = m_pDoc->getStruxPosition(cellSDH)+1;
					bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,posStart,posStart,NULL,properties,PTX_SectionCell);
					if(pFG != NULL)
					{
						pFG->insertAtStrux(m_pDoc,72,posStart,
										   PTX_SectionCell,sDataID.c_str());
					}
					else
					{
						const XML_Char * attributes[3] = {
							PT_STRUX_IMAGE_DATAID,NULL,NULL};
						bRet = m_pDoc->changeStruxFmt(PTC_RemoveFmt,
													  posStart,
													  posStart,
													  attributes,
													  NULL,
													  PTX_SectionCell);	
					}
				}
				else
					UT_DEBUGMSG(("MARCM: Yikes! There is no cell at position (%dx%d)!\n", j, i));
			}
		}
	}
	// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
	// with the restored line-type property it has before.
	iLineType += 1;
	_restoreCellParams(posTable,iLineType);

	// Allow table updates
	m_pDoc->setDontImmediatelyLayout(false);

	_generalUpdate();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_ensureInsertionPointOnScreen();
	clearCursorWait();
	notifyListeners(AV_CHG_MOTION);
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();
	return bRet;
}

/*!
 Get the background color of the cell containing the current cursor position
 \param col will be set to the cell background color, if the background color exists
 \return True if succesful (ie. the background color is set), false otherwise
 */
bool FV_View::getCellBGColor(XML_Char * &color)
{
	PT_DocPosition posCell = getPoint();
	if (!isSelectionEmpty())
	{
		if (m_Selection.getSelectionAnchor() < posCell)
		{
			posCell = m_Selection.getSelectionAnchor();
		}
		if(posCell < 2)
		{
			posCell = 2;
		}
	}	
	
	PL_StruxDocHandle cellSDH;
	bool bres = m_pDoc->getStruxOfTypeFromPosition(posCell,PTX_SectionCell,&cellSDH);
	if(!bres)
	{
		return false;
	}
	m_pDoc->getPropertyFromSDH(cellSDH,"background-color",const_cast<const char **>(&color));
	if(color && *color)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool FV_View::setTableFormat(const XML_Char * properties[])
{
	PT_DocPosition pos = getPoint();
	return setTableFormat(pos, properties);
}

bool FV_View::setTableFormat(PT_DocPosition pos, const XML_Char * properties[])
{
	bool bRet;

	PT_DocPosition posStart = pos;
	PL_StruxDocHandle tableSDH = NULL;
	bRet = m_pDoc->getStruxOfTypeFromPosition(posStart, PTX_SectionTable, &tableSDH);
	if(!bRet)
	{
		UT_ASSERT(0);
		return false;
	}
	setCursorWait();
	//
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();
//
// Put in call here to get start of the table
//
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_Selection.getSelectionAnchor() < posStart)
			posStart = m_Selection.getSelectionAnchor();
		else
			posEnd = m_Selection.getSelectionAnchor();
		if(posStart < 2)
		{
			posStart = 2;
		}
	}
	posStart = m_pDoc->getStruxPosition(tableSDH) +1 ;
	posEnd = posStart+1;
	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,posStart,posEnd,NULL,properties,PTX_SectionTable);

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_generalUpdate();
	_ensureInsertionPointOnScreen();
	clearCursorWait();
	AV_View::notifyListeners(AV_CHG_MOTION);
	return bRet;
}

bool FV_View::setSectionFormat(const XML_Char * properties[])
{
	bool bRet;
	setCursorWait();
	//
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();
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
		if (m_Selection.getSelectionAnchor() < posStart)
			posStart = m_Selection.getSelectionAnchor();
		else
			posEnd = m_Selection.getSelectionAnchor();
		if(posStart < 2)
		{
			posStart = 2;
		}
	}

	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,posStart,posEnd,NULL,properties,PTX_Section);

	_generalUpdate();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_generalUpdate();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_ensureInsertionPointOnScreen();
	clearCursorWait();
	notifyListeners(AV_CHG_MOTION);
	return bRet;
}

/*****************************************************************/
/*****************************************************************/

void FV_View::getTopRulerInfo(AP_TopRulerInfo * pInfo)
{
	if(getPoint() == 0)
	{
		return;
	}
	getTopRulerInfo(getPoint(), pInfo);
}

void FV_View::getTopRulerInfo(PT_DocPosition pos,AP_TopRulerInfo * pInfo)
{

	fl_BlockLayout * pBlock = NULL;
	fp_Run * pRun = NULL;
	UT_sint32 xCaret, yCaret;
	UT_uint32 heightCaret;
	UT_sint32 xCaret2, yCaret2;
	bool bDirection;
	_findPositionCoords(pos, m_bPointEOL, xCaret, yCaret, xCaret2, yCaret2, heightCaret, bDirection, &pBlock, &pRun);

	UT_return_if_fail(pRun);

	fp_Line * pLine = pRun->getLine();
	UT_return_if_fail(pLine);

	fp_Container * pContainer = pLine->getContainer();
	UT_return_if_fail(pContainer);

	fl_SectionLayout * pSection = pContainer->getSectionLayout();
	UT_return_if_fail(pSection);
	if(pInfo->m_vecTableColInfo)
	{
		UT_sint32 count = static_cast<UT_sint32>(pInfo->m_vecTableColInfo->getItemCount());
		UT_sint32 i =0;
		for(i=0; i< count; i++)
		{
			delete static_cast<AP_TopRulerTableInfo *>(pInfo->m_vecTableColInfo->getNthItem(i));
		}
		delete pInfo->m_vecTableColInfo;
		pInfo->m_vecTableColInfo =NULL;
	}
	if(pInfo->m_vecFullTable)
	{
		UT_sint32 count = static_cast<UT_sint32>(pInfo->m_vecFullTable->getItemCount());
		UT_sint32 i =0;
		for(i=0; i< count; i++)
		{
			delete static_cast<AP_TopRulerTableInfo *>(pInfo->m_vecFullTable->getNthItem(i));
		}
		delete pInfo->m_vecFullTable;
		pInfo->m_vecFullTable =NULL;
	}
//
// Clear the rest of the info
//
	memset(pInfo,0,sizeof(*pInfo));
	if (pSection->getType() == FL_SECTION_DOC || pSection->getContainerType() == FL_CONTAINER_FOOTNOTE || pSection->getContainerType() == FL_CONTAINER_ENDNOTE)
	{
		fp_Column* pColumn = NULL;
		fl_DocSectionLayout* pDSL = NULL;
		
		if(pSection->getType() == FL_SECTION_DOC)
		{
			pColumn = static_cast<fp_Column*>(pContainer);
			pDSL = static_cast<fl_DocSectionLayout*>(pSection);
		}
		else
		{
			fp_Page * pPage = pContainer->getPage();
			if(pPage)
			{
				pDSL = pPage->getOwningSection();
				pColumn = pPage->getNthColumnLeader(0);
			}
			else
			{
				return;
			}
		}
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
		if(pSection->getContainerType() == FL_CONTAINER_FOOTNOTE)
		{
			pInfo->m_iCurrentColumn = 1;
			pInfo->m_iNumColumns = 1;
			pInfo->u.c.m_xColumnGap = 0;
			pInfo->u.c.m_xColumnWidth = pContainer->getWidth();
		}
		pInfo->m_mode = AP_TopRulerInfo::TRI_MODE_COLUMNS;

		pInfo->m_xrPoint = xCaret - pContainer->getX();
		pInfo->m_xrLeftIndent = UT_convertToLogicalUnits(pBlock->getProperty("margin-left"));
		pInfo->m_xrRightIndent = UT_convertToLogicalUnits(pBlock->getProperty("margin-right"));
		pInfo->m_xrFirstLineIndent = UT_convertToLogicalUnits(pBlock->getProperty("text-indent"));
		xxx_UT_DEBUGMSG(("ap_TopRuler: xrPoint %d LeftIndent %d RightIndent %d Firs %d \n",pInfo->m_xrPoint,pInfo->m_xrLeftIndent,pInfo->m_xrRightIndent,	pInfo->m_xrFirstLineIndent));
	}
	else if(isHdrFtrEdit())
	{
		fp_Column* pColumn = static_cast<fp_Column*>(pContainer);
		fl_DocSectionLayout* pDSL = static_cast<fl_DocSectionLayout*>(pSection);
		pDSL = m_pEditShadow->getHdrFtrSectionLayout()->getDocSectionLayout();

		pInfo->m_iCurrentColumn = 0;
		pInfo->m_iNumColumns = 1;

		pInfo->u.c.m_xaLeftMargin = pDSL->getLeftMargin();
		pInfo->u.c.m_xaRightMargin = pDSL->getRightMargin();
		pInfo->u.c.m_xColumnGap = pDSL->getColumnGap();
		pInfo->u.c.m_xColumnWidth = pColumn->getWidth();
		pInfo->m_mode = AP_TopRulerInfo::TRI_MODE_COLUMNS;

		pInfo->m_xrPoint = xCaret - pContainer->getX();
		pInfo->m_xrLeftIndent = UT_convertToLogicalUnits(pBlock->getProperty("margin-left"));
		pInfo->m_xrRightIndent = UT_convertToLogicalUnits(pBlock->getProperty("margin-right"));
		pInfo->m_xrFirstLineIndent = UT_convertToLogicalUnits(pBlock->getProperty("text-indent"));

	}
	else if(pSection->getContainerType() == FL_CONTAINER_CELL)
	{
		fp_CellContainer * pCell = static_cast<fp_CellContainer *>(pContainer);
		fl_DocSectionLayout* pDSL = pSection->getDocSectionLayout();
		fp_Column * pColumn = static_cast<fp_Column *>(pCell->getColumn(pLine));
		if(pColumn == NULL)
		{
			return;
		}
		fp_FrameContainer * pFrameC = NULL;
		UT_uint32 nCol=0;
		if(isInFrame(getPoint()))
		{
			fp_Container * pCon = pCell->getContainer();
			while(pCon && !pCon->isColumnType())
			{
				pCon = pCon->getContainer();
			}
			if(pCon == NULL)
			{
				return;
			}
			if(pCon->getContainerType() == FP_CONTAINER_FRAME)
			{
				pFrameC = static_cast<fp_FrameContainer *>(pCon);
			}
			fp_Page * pPage = pFrameC->getPage();
			if(pPage == NULL)
			{
				return;
			}
			pInfo->m_iCurrentColumn = 0;
			pInfo->m_iNumColumns = 1;
			pInfo->u.c.m_xaLeftMargin = pFrameC->getFullX();
			pInfo->u.c.m_xaRightMargin = pDSL->getRightMargin();
			pInfo->u.c.m_xColumnGap = 0;
			pInfo->u.c.m_xaRightMargin = pPage->getWidth() - pFrameC->getFullX() - pFrameC->getFullWidth();
			pInfo->m_xrPoint = xCaret - pFrameC->getX();
			pInfo->u.c.m_xColumnWidth = pFrameC->getFullWidth();
		}
		else
		{
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
			pInfo->m_xrPoint = xCaret - pContainer->getX();
		}
		pInfo->m_mode = AP_TopRulerInfo::TRI_MODE_TABLE;
		pInfo->m_xrLeftIndent = UT_convertToLogicalUnits(pBlock->getProperty("margin-left"));
		pInfo->m_xrRightIndent = UT_convertToLogicalUnits(pBlock->getProperty("margin-right"));
		pInfo->m_xrFirstLineIndent = UT_convertToLogicalUnits(pBlock->getProperty("text-indent"));
		fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pCell->getContainer());
		UT_sint32 row = pCell->getTopAttach();
		UT_sint32 numcols = pTab->getNumCols();
		UT_sint32 numrows = pTab->getNumRows();
		UT_sint32 i =0;
		fp_CellContainer * pCur = NULL;
		UT_sint32 iCellCount = 0;
		pInfo->m_vecTableColInfo = new UT_Vector();
		while( i < numcols)
		{
			pCur = pTab->getCellAtRowColumn(row,i);
			if(pCur == pCell)
			{
				pInfo->m_iCurCell = iCellCount;
			}
			UT_sint32 ioff_x = 0;
			fp_Container * pCon = static_cast<fp_Container*>(pTab->getContainer());
			while(!pCon->isColumnType())
			{
				ioff_x += pCon->getX();
				pCon = static_cast<fp_Container *>(pCon->getContainer());
			}
			if(pCur)
			{
				AP_TopRulerTableInfo *pTInfo = new  AP_TopRulerTableInfo;
				pTInfo->m_pCell = pCur;
				pTInfo->m_iLeftCellPos = pCur->getLeftPos() +ioff_x;
				pTInfo->m_iRightCellPos = pCur->getRightPos() +ioff_x;
				pTInfo->m_iLeftSpacing = (pCur->getX() - pCur->getLeftPos());
				pTInfo->m_iRightSpacing = ( pCur->getRightPos() - pCur->getX()
											- pCur->getWidth());
				pInfo->m_vecTableColInfo->addItem(static_cast<void *>(pTInfo));
				i = pCur->getRightAttach();
				iCellCount++;
			}
			else
			{
				i = numcols + 1;
			}
		}
		pInfo->m_iCells = pInfo->m_vecTableColInfo->getItemCount();
//
// Now fill the full vector including merged cells.
//
		pInfo->m_vecFullTable = new UT_Vector();
		for( i=0;i < numcols;i++)
		{
			pCur = pTab->getCellAtRowColumn(0,i);
			UT_sint32 j =0;
			while(pCur && ((pCur->getLeftAttach()+1) != pCur->getRightAttach()) && (j <= numrows))
			{
				pCur = pTab->getCellAtRowColumn(j,i);
				if(pCur)
				{
					j++;
				}
			}
			UT_sint32 ioff_x = 0;
			fp_Container * pCon = static_cast<fp_Container*>(pTab->getContainer());
			while(!pCon->isColumnType())
			{
				ioff_x += pCon->getX();
				pCon = static_cast<fp_Container *>(pCon->getContainer());
			}
			if(pCur)
			{
				AP_TopRulerTableInfo *pTInfo = new  AP_TopRulerTableInfo;
				pTInfo->m_pCell = pCur;
				pTInfo->m_iLeftCellPos = pCur->getLeftPos() +ioff_x;
				pTInfo->m_iRightCellPos = pCur->getRightPos() +ioff_x;
				pTInfo->m_iLeftSpacing = (pCur->getX() - pCur->getLeftPos());
				pTInfo->m_iRightSpacing = ( pCur->getRightPos() - pCur->getX()
											- pCur->getWidth());
				pInfo->m_vecFullTable->addItem(static_cast<void *>(pTInfo));
			}
		}
	}
	else if(pContainer->getContainerType() == FP_CONTAINER_FRAME)
	{
		fp_FrameContainer * pFC = static_cast<fp_FrameContainer *>(pContainer);
		fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(pSection);
		pInfo->m_mode = AP_TopRulerInfo::TRI_MODE_FRAME;
		fl_DocSectionLayout * pDSL = pFL->getDocSectionLayout();
		if(pDSL == NULL)
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return;
		}
		pInfo->m_iCurrentColumn = 0;
		pInfo->m_iNumColumns = 1;
		fp_Page * pPage = pFC->getPage();
		if(pPage == NULL)
		{
			return;
		}
		pInfo->u.c.m_xaLeftMargin = pFC->getFullX();
		pInfo->u.c.m_xColumnGap = 0;
		pInfo->u.c.m_xColumnWidth = pFC->getFullWidth();
		pInfo->u.c.m_xaRightMargin = pPage->getWidth() - pFC->getFullX() - pFC->getFullWidth();
		
		pInfo->m_xrPoint = xCaret - pFC->getX();
		pInfo->m_xrLeftIndent = UT_convertToLogicalUnits(pBlock->getProperty("margin-left"));
		pInfo->m_xrRightIndent = UT_convertToLogicalUnits(pBlock->getProperty("margin-right"));
		pInfo->m_xrFirstLineIndent = UT_convertToLogicalUnits(pBlock->getProperty("text-indent"));
		
	}
	else
	{
		// fill in the details
	}

	static UT_String buf;
	char * old_locale = setlocale(LC_NUMERIC,"C");
	buf = UT_String_sprintf ("%.4fin", m_pDoc->m_docPageSize.Width(DIM_IN));
	setlocale(LC_NUMERIC,old_locale); // restore original locale

	pInfo->m_xPaperSize = UT_convertToLogicalUnits(buf.c_str());
	pInfo->m_xPageViewMargin = getPageViewLeftMargin();

	pInfo->m_pfnEnumTabStops = fl_BlockLayout::s_EnumTabStops;
	pInfo->m_pVoidEnumTabStopsData = static_cast<void *>(pBlock);
	pInfo->m_iTabStops = static_cast<UT_sint32>(pBlock->getTabsCount());
	pInfo->m_iDefaultTabInterval = pBlock->getDefaultTabInterval();
	pInfo->m_pszTabStops = pBlock->getProperty("tabstops");

	return;
}

void FV_View::getLeftRulerInfo(AP_LeftRulerInfo * pInfo)
{
//
// Can't do this before the layouts are filled.
//
	if(getPoint()== 0)
	{
		return;
	}
	getLeftRulerInfo(getPoint(),pInfo);
}

void FV_View::getLeftRulerInfo(PT_DocPosition pos, AP_LeftRulerInfo * pInfo)
{
//
// Clear out the old table info
//
	if(pInfo->m_vecTableRowInfo)
	{
		UT_sint32 count = static_cast<UT_sint32>(pInfo->m_vecTableRowInfo->getItemCount());
		UT_sint32 i =0;
		for(i=0; i< count; i++)
		{
			delete static_cast<AP_LeftRulerTableInfo *>(pInfo->m_vecTableRowInfo->getNthItem(i));
		}
		delete pInfo->m_vecTableRowInfo;
		pInfo->m_vecTableRowInfo =NULL;
	}
	memset(pInfo,0,sizeof(*pInfo));
	xxx_UT_DEBUGMSG(("ap_LeftRulerInfo: get Leftruler info \n"));

	if (1)					
	{
		// we assume that we are in a column context (rather than a table)

		pInfo->m_mode = AP_LeftRulerInfo::TRI_MODE_COLUMNS;

		fl_BlockLayout * pBlock = NULL;
		fp_Run * pRun = NULL;
		UT_sint32 xCaret, yCaret;
		UT_uint32 heightCaret;
		UT_sint32 xCaret2, yCaret2;
		bool bDirection;
		_findPositionCoords(pos, m_bPointEOL, xCaret, yCaret, xCaret2, yCaret2, heightCaret, bDirection, &pBlock, &pRun);

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
		fl_SectionLayout * pSection = NULL;
		fl_DocSectionLayout * pDSL = NULL;
		fp_Container * pContainer = pRun->getLine()->getContainer();
		if(pContainer == NULL)
		{
			pInfo->m_yPageStart = 0;
			pInfo->m_yPageSize = 0;
			pInfo->m_yPoint = 0;
			pInfo->m_yTopMargin = 0;
			pInfo->m_yBottomMargin = 0;
			return;
		}

		bool isFootnote = false;
		bool isEndnote = false;
		xxx_UT_DEBUGMSG(("ap_leftRulerInfo: container type %d \n",pContainer->getContainerType()));
		fp_Page * pPage =  pRun->getLine()->getPage();
		if(pPage == NULL)
		{
			pInfo->m_yPageStart = 0;
			pInfo->m_yPageSize = 0;
			pInfo->m_yPoint = 0;
			pInfo->m_yTopMargin = 0;
			pInfo->m_yBottomMargin = 0;
			return;
		}
 
		if(pContainer->getContainerType() == FP_CONTAINER_FOOTNOTE)
		{
			pSection = pPage->getOwningSection();
			pDSL = static_cast<fl_DocSectionLayout *>(pSection);
			isFootnote = true;
			xxx_UT_DEBUGMSG(("ap_LeftRulerInfo: Found footnote at point \n"));
		}
		else if(pContainer->getContainerType() == FP_CONTAINER_ENDNOTE)
		{
			pSection = pPage->getOwningSection();
			pDSL = static_cast<fl_DocSectionLayout *>(pSection);
			isEndnote = true;
			xxx_UT_DEBUGMSG(("ap_LeftRulerInfo: Found footnote at point \n"));
		}
		else
		{
			pSection = pPage->getOwningSection();
			pDSL = static_cast<fl_DocSectionLayout*>(pSection);
		}
		pInfo->m_yPoint = yCaret - pContainer->getY();

		if ((isFootnote || isEndnote || pContainer->getContainerType() == FP_CONTAINER_COLUMN) && !isHdrFtrEdit())
		{
			UT_sint32 yoff = 0;
			getPageYOffset(pPage, yoff);
			pInfo->m_yPageStart = static_cast<UT_uint32>(yoff);
			pInfo->m_yPageSize = pPage->getHeight();

			pInfo->m_yTopMargin = pDSL->getTopMargin();
			UT_ASSERT(pInfo->m_yTopMargin>= 0);

			pInfo->m_yBottomMargin = pDSL->getBottomMargin();
		}
		else if(isHdrFtrEdit())
		{
			fl_HdrFtrSectionLayout * pHF =	m_pEditShadow->getHdrFtrSectionLayout();
			pDSL = pHF->getDocSectionLayout();
			UT_sint32 yoff = 0;
			getPageYOffset(pPage, yoff);
			pInfo->m_yPageStart = static_cast<UT_uint32>(yoff);
			pInfo->m_yPageSize = pPage->getHeight();

			if(pHF->getHFType() == FL_HDRFTR_FOOTER)
			{
				pInfo->m_yTopMargin = pPage->getHeight() - pDSL->getBottomMargin();
				UT_ASSERT(pInfo->m_yTopMargin>= 0);
				pInfo->m_yBottomMargin = pDSL->getFooterMargin();
			}
			else
			{
				pInfo->m_yTopMargin = pDSL->getHeaderMargin();
				UT_ASSERT(pInfo->m_yTopMargin>= 0);
				pInfo->m_yBottomMargin = pPage->getHeight() - pDSL->getTopMargin();
			}

		}
		else if(pContainer->getContainerType() == FP_CONTAINER_CELL)
		{
			fp_CellContainer * pCell = static_cast<fp_CellContainer *>(pContainer);
			fl_ContainerLayout * pCL = pSection->myContainingLayout();
			pInfo->m_mode = AP_LeftRulerInfo::TRI_MODE_TABLE;
			while(pCL && pCL->getContainerType() != FL_CONTAINER_DOCSECTION)
			{
				pCL = pCL->myContainingLayout();
			}
			fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(pCL);
			if(pDSL == NULL)
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				return;
			}

			UT_sint32 yoff = 0;
			getPageYOffset(pPage, yoff);
			pInfo->m_yPageStart = static_cast<UT_uint32>(yoff);
			pInfo->m_yPageSize = pPage->getHeight();
			pDSL = pPage->getOwningSection();
			if(!isInFrame(getPoint()))
			{
				pInfo->m_yTopMargin = pDSL->getTopMargin();
				UT_ASSERT(pInfo->m_yTopMargin>= 0);

				pInfo->m_yBottomMargin = pDSL->getBottomMargin();
			}
			else
			{
				getPageYOffset(pPage, yoff);
				pInfo->m_yPageStart = static_cast<UT_uint32>(yoff);
				pInfo->m_yPageSize = pPage->getHeight();
				fp_FrameContainer * pFC = static_cast<fp_FrameContainer *>(getFrameLayout()->getFirstContainer());

				pInfo->m_yTopMargin = pFC->getFullY();
				UT_ASSERT(pInfo->m_yTopMargin>= 0);

				pInfo->m_yBottomMargin = pPage->getHeight() - pFC->getFullY() - pFC->getFullHeight();
			}
			fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pCell->getContainer());
			UT_sint32 col = pCell->getLeftAttach();
			UT_sint32 numrows = pTab->getNumRows();
			UT_sint32 i =0;
			fp_CellContainer * pCur = NULL;
			pInfo->m_vecTableRowInfo = new UT_Vector();
			pCur = pTab->getCellAtRowColumn(0,col);
			fp_CellContainer * pPrev = pCur;	
			while( i < numrows)
			{
				bool bFound = false;
				pPrev = pCur;
				while(pCur && !bFound)
				{
					if(pCur->getLeftAttach() <= col && 
					   pCur->getRightAttach() > col &&
					   pCur->getTopAttach() <= i && 
					   pCur->getBottomAttach() > i)
					{
						bFound = true;
					}
					else
					{
						pCur = static_cast<fp_CellContainer *>(pCur->getNext());
					}
				}
				if(pCur == pCell)
				{
					pInfo->m_iCurrentRow = i;
				}
				if(pCur)
				{
					AP_LeftRulerTableInfo *pLInfo = new  AP_LeftRulerTableInfo;
					pLInfo->m_pCell = pCur;
					pLInfo->m_iTopCellPos = pCur->getStartY();
					pLInfo->m_iBotCellPos = pCur->getStopY();
					pLInfo->m_iTopSpacing = pCur->getY() - pCur->getStartY();
					pLInfo->m_iBotSpacing = pCur->getStopY() - (pCur->getY() + pCur->getHeight());
					pInfo->m_vecTableRowInfo->addItem(static_cast<void *>(pLInfo));
					i = pCur->getBottomAttach();
				}
				else
				{
					pCur= pPrev;
					i = numrows + 1;
				}
			}
			pInfo->m_iNumRows = pInfo->m_vecTableRowInfo->getItemCount();
		}
		else if(pContainer->getContainerType() == FP_CONTAINER_FRAME)
		{
			fp_FrameContainer * pFC = static_cast<fp_FrameContainer *>(pContainer);
			fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(pSection);
			pInfo->m_mode = AP_LeftRulerInfo::TRI_MODE_FRAME;
			fl_DocSectionLayout * pDSL = pFL->getDocSectionLayout();
			if(pDSL == NULL)
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				return;
			}
			UT_sint32 yoff = 0;
			getPageYOffset(pPage, yoff);
			pInfo->m_yPageStart = static_cast<UT_uint32>(yoff);
			pInfo->m_yPageSize = pPage->getHeight();

			pInfo->m_yTopMargin = pFC->getFullY();
			UT_ASSERT(pInfo->m_yTopMargin>= 0);

			pInfo->m_yBottomMargin = pPage->getHeight() - pFC->getFullY() - pFC->getFullHeight();
		}
		else
		{
		}
	}
	else
	{
		// other yet to be written contexts (frames?)
	}
	UT_ASSERT(pInfo->m_yTopMargin>= 0);	
	return;
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
	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL, true);

	return isPosSelected(pos);
}

/*!
 * Returns a pointer to the cell container surrounding the supplied point
 * Return NULL if there isn't one.
 */
fp_CellContainer * FV_View::getCellAtPos(PT_DocPosition pos)
{
	bool bEOL = false;
	UT_uint32 iPointHeight;
	UT_sint32 xPoint, yPoint, xPoint2, yPoint2;
	bool bDirection;
	fl_BlockLayout* pBlock;
	fp_Run* pRun;
	_findPositionCoords(pos, bEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection, &pBlock, &pRun);
	fp_CellContainer * pCell = NULL;
	if(isInTable(pos))
	{
		fp_Line * pLine = pRun->getLine();
		if(pLine)
		{
			pCell = static_cast<fp_CellContainer *>(pLine->getContainer());
			if(pCell && pCell->getContainerType() == FP_CONTAINER_CELL)
			{
				return pCell;
			}
		}
	}
	return NULL;
}

EV_EditMouseContext FV_View::getMouseContext(UT_sint32 xPos, UT_sint32 yPos)
{
	xxx_UT_DEBUGMSG(("layout view mouse pos x %x pos y %d \n",xPos,yPos));
	UT_sint32 xClick, yClick;
	PT_DocPosition pos;
	bool bBOL = false;
	bool bEOL = false;
	UT_uint32 iPointHeight;
	UT_sint32 xPoint, yPoint, xPoint2, yPoint2;
	bool bDirection;
	m_iMouseX = xPos;
	m_iMouseY = yPos;
	if(getPoint() == 0) // We haven't loaded any layouts yet
	{
		return EV_EMC_UNKNOWN;
	}
	if(m_bDragTableLine)
	{
		return m_prevMouseContext;
	}
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);
	if (!pPage)
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (1)\n"));
		m_prevMouseContext = EV_EMC_UNKNOWN;
		return EV_EMC_UNKNOWN;
	}


	if (   (yClick < 0)
		   || (xClick < 0)
		   || (xClick > pPage->getWidth()) )
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (2)\n"));
		m_prevMouseContext = EV_EMC_UNKNOWN;
		return EV_EMC_UNKNOWN;
	}
	if(m_FrameEdit.isActive())
	{
		return EV_EMC_FRAME;
	}
	UT_sint32 ires = 40;
	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL, true);
	fl_BlockLayout* pBlock;
	fp_Run* pRun;
	_findPositionCoords(pos, bEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection, &pBlock, &pRun);
//
// Look if we're inside a frame
//
	if(isInFrame(pos))
	{
		//
		// OK find the coordinates of the frame.
		//
		UT_sint32 xPage,yPage;
		getPageScreenOffsets(pPage,xPage,yPage);
		fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(pBlock->myContainingLayout());
		fp_FrameContainer * pFCon = static_cast<fp_FrameContainer *>(pFL->getFirstContainer());
		UT_sint32 iLeft = xPage + pFCon->getFullX();
		UT_sint32 iRight = xPage + pFCon->getFullX() + pFCon->getFullWidth();
		UT_sint32 iTop = yPage + pFCon->getFullY();
		UT_sint32 iBot = yPage + pFCon->getFullY() + pFCon->getFullHeight();
		bool bLeft = (iLeft - xPos < ires) && (xPos - iLeft < ires);
		bool bRight = (iRight - xPos < ires) && (xPos - iRight < ires);
		bool bTop = (iTop - yPos < ires) && (yPos - iTop < ires);
		bool bBot = (iBot - yPos < ires) && (yPos - iBot < ires);
		bool bX = (iLeft - ires < xPos) && (xPos < iRight + ires);
		bool bY = (iTop - ires < yPos) && (iBot + ires > yPos);
		if( (bLeft || bRight) && bY)
		{
// TODO put in some code to indicate which control of the frame is being
// dragged. Maybe reuse topRuler stuff???
//
			xxx_UT_DEBUGMSG(("getContext: Found left frame line \n"));
			m_prevMouseContext = EV_EMC_FRAME;
			return EV_EMC_FRAME;
		}
		if( (bTop || bBot) && bX)
		{
// TODO put in some code to indicate which control of the frame is being
// dragged. Maybe reuse topRuler stuff???
//
			xxx_UT_DEBUGMSG(("getContext: Found left frame line \n"));
			m_prevMouseContext = EV_EMC_FRAME;
			return EV_EMC_FRAME;
		}

	}	
	if(isInTable(pos))
	{
		fp_Line * pLine = pRun->getLine();
		if(pLine)
		{
			fp_CellContainer * pCell = static_cast<fp_CellContainer *>(pLine->getContainer());
			if(pCell && pCell->getContainerType() == FP_CONTAINER_CELL)
			{
				xxx_UT_DEBUGMSG(("getcontext: Looking at Table \n"));
				UT_sint32 iLeft = pCell->getLeftPos();
				UT_sint32 iRight = pCell->getRightPos();
				UT_sint32 iTop = pCell->getStartY();
				UT_sint32 iBot = pCell->getStopY();
				UT_sint32 offy =0;
				UT_sint32 offx =0;
				fp_Column * pCol = static_cast<fp_Column *>(pCell->getColumn(pLine));
				UT_sint32 col_x =0;
				UT_sint32 col_y =0;
				pPage->getScreenOffsets(pCol, col_x,col_y);
//
// Now find the brokentable the line is in.
//
				fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pCell->getContainer());
				bool bNested = false;
				if(pTab->getContainer()->getContainerType() == FP_CONTAINER_CELL)
				{
					bNested = true;
				}
				fp_TableContainer * pBroke = pTab->getFirstBrokenTable();
				if(!bNested)
				{
					UT_sint32 i =0;
					offx = pTab->getX();
					while(pBroke && !pBroke->isInBrokenTable(pCell,pLine))
					{
						i++;
						pBroke = static_cast<fp_TableContainer *>(pBroke->getNext());
					}
					if(i==0)
					{
						offy = pTab->getY();
					}
					else if(pBroke)
					{
						offy -= pBroke->getYBreak();
					}
				}
				else
				{
					fp_Container * pConTmp = pTab;
					while(pConTmp && !pConTmp->isColumnType())
					{
						offy += pConTmp->getY();
						offx += pConTmp->getX();
						pConTmp = pConTmp->getContainer();
					}
				}
				iLeft += col_x + offx;
				iRight += col_x + offx;
				iTop += col_y +  offy;
				iBot += col_y + offy;
				xxx_UT_DEBUGMSG(("getContext: xPos %d yPos %d iLeft %d iRight %d iTop %d iBot %d \n",xPos,yPos,iLeft,iRight,iTop,iBot));
				if((iLeft - xPos < ires) && (xPos - iLeft < ires))
				{

// TODO put in some code to indicate which control of which cell is being
// dragged. Maybe reuse topRuler stuff???
//
					xxx_UT_DEBUGMSG(("getContext: Found left cell \n"));
					m_prevMouseContext = EV_EMC_VLINE;
					return EV_EMC_VLINE;
				}
				if((iRight - xPos < ires) && (xPos - iRight < ires))
				{
					xxx_UT_DEBUGMSG(("getContext: Found right cell \n"));
					m_prevMouseContext = EV_EMC_VLINE;
					return EV_EMC_VLINE;
				}
				if((iTop - yPos < ires) && (yPos - iTop < ires))
				{
					xxx_UT_DEBUGMSG(("getContext: Found top cell \n"));
					m_prevMouseContext = EV_EMC_HLINE;
					return EV_EMC_HLINE;
				}
				if((iBot - yPos < ires) && (yPos - iBot < ires))
				{
					xxx_UT_DEBUGMSG(("getContext: Found bot cell \n"));
					m_prevMouseContext = EV_EMC_HLINE;
					return EV_EMC_HLINE;
				}
			}
		}
	}
	if (!pBlock)
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (5)\n"));
		m_prevMouseContext = EV_EMC_UNKNOWN;
		return EV_EMC_UNKNOWN;
	}

	if (isLeftMargin(xPos,yPos))
	{
		UT_ASSERT(pBlock);
		xxx_UT_DEBUGMSG(("Entered BOL margin: dir %d\n", pBlock->getDominantDirection()));
		if(pBlock->getDominantDirection() == FRIBIDI_TYPE_RTL)
		{
			xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (3)\n"));
			m_prevMouseContext = EV_EMC_RIGHTOFTEXT;
			return EV_EMC_RIGHTOFTEXT;
		}
		else
		{
			xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (4)\n"));
			m_prevMouseContext = EV_EMC_LEFTOFTEXT;
			return EV_EMC_LEFTOFTEXT;
		}
	}

	while(pRun && pRun->getType() ==  FPRUN_FMTMARK)
	{
		pRun = pRun->getNextRun();
	}

	if (!pRun)
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (6)\n"));
		m_prevMouseContext = EV_EMC_UNKNOWN;
		return EV_EMC_UNKNOWN;
	}
	if(pRun->containsRevisions())
	{
		m_prevMouseContext = EV_EMC_REVISION;
		return EV_EMC_REVISION;
	}

	if(pRun->getHyperlink() != NULL)
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (7), run type %d\n", pRun->getType()));
		m_prevMouseContext = EV_EMC_HYPERLINK;
		return EV_EMC_HYPERLINK;
	}
	if(!isSelectionEmpty())
	{
		if(pRun->getType() == FPRUN_IMAGE)
		{
			// clear the image selection rect
			setImageSelRect(UT_Rect(-1,-1,-1,-1));
		
			// check if this image is selected
			UT_uint32 iRunBase = pRun->getBlock()->getPosition() + pRun->getBlockOffset();
	
			UT_uint32 iSelAnchor = getSelectionAnchor();
			UT_uint32 iPoint = getPoint();
	
			UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
			UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);
	
			UT_ASSERT(iSel1 <= iSel2);
	
			if (
			    /* getFocus()!=AV_FOCUS_NONE && */
				(iSel1 <= iRunBase)
				&& (iSel2 > iRunBase)
				)
			{
				// This image is selected. Now get the image size.
				
				UT_sint32 xoff = 0, yoff = 0;
				pRun->getLine()->getScreenOffsets(pRun, xoff, yoff);
	
				// Sevior's infamous + 1....
				yoff += pRun->getLine()->getAscent() - pRun->getAscent() + 1;				
				
				// Set the image size in the image selection rect
				m_selImageRect = UT_Rect(xoff,yoff,pRun->getWidth(),pRun->getHeight());
			}
		
			if (isOverImageResizeBox(m_imageSelCursor, xPos, yPos))
			{
				m_prevMouseContext = EV_EMC_IMAGESIZE;
				return EV_EMC_IMAGESIZE;
			}
			else
			{
				m_prevMouseContext = EV_EMC_IMAGE;
				return EV_EMC_IMAGE;
			}
		}
		if(m_Selection.isPosSelected(pos))
		{
			m_prevMouseContext = EV_EMC_VISUALTEXTDRAG;
			return EV_EMC_VISUALTEXTDRAG;
		}
	}
	
	switch (pRun->getType())
	{
	case FPRUN_TEXT:
		if (!isPosSelected(pos))
			if (pBlock->getSquiggles()->get(pos - pBlock->getPosition()))
			{
				xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (8)\n"));
				m_prevMouseContext = EV_EMC_MISSPELLEDTEXT;
				return EV_EMC_MISSPELLEDTEXT;
			}
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (9)\n"));
		m_prevMouseContext = EV_EMC_TEXT;
		return EV_EMC_TEXT;

	case FPRUN_IMAGE:
		{
			// clear the image selection rect
			setImageSelRect(UT_Rect(-1,-1,-1,-1));
		
			// check if this image is selected
			UT_uint32 iRunBase = pRun->getBlock()->getPosition() + pRun->getBlockOffset();
	
			UT_uint32 iSelAnchor = getSelectionAnchor();
			UT_uint32 iPoint = getPoint();
	
			UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
			UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);
	
			UT_ASSERT(iSel1 <= iSel2);
	
			if (
			    /* getFocus()!=AV_FOCUS_NONE && */
				(iSel1 <= iRunBase)
				&& (iSel2 > iRunBase)
				)
			{
				// This image is selected. Now get the image size.
				
				UT_sint32 xoff = 0, yoff = 0;
				pRun->getLine()->getScreenOffsets(pRun, xoff, yoff);
	
				// Sevior's infamous + 1....
				yoff += pRun->getLine()->getAscent() - pRun->getAscent() + 1;				
				
				// Set the image size in the image selection rect
				m_selImageRect = UT_Rect(xoff,yoff,pRun->getWidth(),pRun->getHeight());
			}
		
			if (isOverImageResizeBox(m_imageSelCursor, xPos, yPos))
			{
				m_prevMouseContext = EV_EMC_IMAGESIZE;
				return EV_EMC_IMAGESIZE;
			}
			else
			{
				m_prevMouseContext = EV_EMC_IMAGE;
				return EV_EMC_IMAGE;
			}
		}
	case FPRUN_TAB:
	case FPRUN_FORCEDLINEBREAK:
	case FPRUN_FORCEDCOLUMNBREAK:
	case FPRUN_FORCEDPAGEBREAK:
	case FPRUN_ENDOFPARAGRAPH:
	case FPRUN_FMTMARK:
	case FPRUN_BOOKMARK:
	case FPRUN_HYPERLINK:
	case FPRUN_DIRECTIONMARKER:
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (10): %d\n", pRun->getType()));
		m_prevMouseContext = EV_EMC_TEXT;
		return EV_EMC_TEXT;

	case FPRUN_FIELD:
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (11)\n"));
		m_prevMouseContext = EV_EMC_FIELD;
		return EV_EMC_FIELD;

	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (12)\n"));
		m_prevMouseContext = EV_EMC_UNKNOWN;
		return EV_EMC_UNKNOWN;
	}

	/*NOTREACHED*/
	UT_ASSERT(0);
	xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (13)\n"));
	m_prevMouseContext = EV_EMC_UNKNOWN;
	return EV_EMC_UNKNOWN;
}

/*!
 * Return the document position from the most recent mouse positions
 */
PT_DocPosition FV_View::getDocPositionFromLastXY(void)
{
	return getDocPositionFromXY(m_iMouseX,m_iMouseY);
}

/*!
 * This sets the mouse pointer to show the little watch or some other icon if
 * if a long operation is in progress.
 */
void FV_View::setCursorWait(void)
{
	if (!getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
		return;

	m_pG->setCursor(GR_Graphics::GR_CURSOR_WAIT);
	XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
	if(pFrame)
	{
		pFrame->setCursor(GR_Graphics::GR_CURSOR_WAIT);
	}
}

/*!
 * The clears the "waiting" icon from the mouse pointer.
 */
void FV_View::clearCursorWait(void)
{
	if (!getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
		return;

	setCursorToContext();
	XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
	if(pFrame)
		pFrame->setCursor(GR_Graphics::GR_CURSOR_DEFAULT);
}

/*!
 * This method sets the cursor to match current context.
 */
void FV_View::setCursorToContext()
{
	if (!getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
		return;

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
		cursor = m_imageSelCursor;
		break;	
	case EV_EMC_FIELD:
		cursor = GR_Graphics::GR_CURSOR_DEFAULT;
		break;
	case EV_EMC_HYPERLINK:
		cursor = GR_Graphics::GR_CURSOR_LINK;
		break;
	case EV_EMC_RIGHTOFTEXT:
		cursor = GR_Graphics::GR_CURSOR_LEFTARROW;
		break;
	case EV_EMC_HYPERLINKTEXT:
		cursor = GR_Graphics::GR_CURSOR_LINK;
		break;
	case EV_EMC_HYPERLINKMISSPELLED:
		cursor = GR_Graphics::GR_CURSOR_LINK;
		break;
	case EV_EMC_VLINE:
		UT_DEBUGMSG(("setCursor: Set to VLINE_DRAG \n"));
		cursor = GR_Graphics::GR_CURSOR_VLINE_DRAG;
		break;
	case EV_EMC_HLINE:
		cursor = GR_Graphics::GR_CURSOR_HLINE_DRAG;
		break;
	case EV_EMC_VISUALTEXTDRAG:
		cursor = GR_Graphics::GR_CURSOR_IMAGE;
		break;
	case EV_EMC_FRAME:
		if(m_FrameEdit.getFrameEditMode() == FV_FrameEdit_WAIT_FOR_FIRST_CLICK_INSERT)
		{
			cursor = GR_Graphics::GR_CURSOR_CROSSHAIR;
		}
		else if(m_FrameEdit.getFrameEditDragWhat() ==FV_FrameEdit_DragTopLeftCorner)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_NW;
		}
		else if(m_FrameEdit.getFrameEditDragWhat() ==FV_FrameEdit_DragTopRightCorner)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_NE;
		}
		else if(m_FrameEdit.getFrameEditDragWhat() ==FV_FrameEdit_DragBotLeftCorner)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_SW;
		}
		else if(m_FrameEdit.getFrameEditDragWhat() ==FV_FrameEdit_DragBotRightCorner)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_SE;
		}
		else if(m_FrameEdit.getFrameEditDragWhat() ==FV_FrameEdit_DragLeftEdge)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_E;
		}
		else if(m_FrameEdit.getFrameEditDragWhat() ==FV_FrameEdit_DragTopEdge)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_N;
		}
		else if(m_FrameEdit.getFrameEditDragWhat() ==FV_FrameEdit_DragRightEdge)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_W;
		}
		else if(m_FrameEdit.getFrameEditDragWhat() ==FV_FrameEdit_DragBotEdge)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_S;
		}
		else if(m_FrameEdit.getFrameEditDragWhat() ==FV_FrameEdit_DragWholeFrame)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGE;
		}
		else
		{
			cursor = GR_Graphics::GR_CURSOR_GRAB;
		}
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

	fl_BlockLayout* pBlock;
	fp_Run* pRun;
	UT_sint32 x, y, x2, y2;
	UT_uint32 h;
	bool b;

	_findPositionCoords(m_iInsPoint, false, x, y, x2, y2, h, b,&pBlock, &pRun);

	if (!pBlock)
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (5)\n"));
		return EV_EMC_UNKNOWN;
	}

	if (!pRun)
	{
		return EV_EMC_UNKNOWN;
	}

	if(pRun->containsRevisions())
	{
		return EV_EMC_REVISION;
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
		{
			// clear the image selection rect
			setImageSelRect(UT_Rect(-1,-1,-1,-1));
		
			// check if this image is selected
			UT_uint32 iRunBase = pRun->getBlock()->getPosition() + pRun->getBlockOffset();
	
			UT_uint32 iSelAnchor = getSelectionAnchor();
			UT_uint32 iPoint = getPoint();
	
			UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
			UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);
	
			UT_ASSERT(iSel1 <= iSel2);
	
			if (
			    /* getFocus()!=AV_FOCUS_NONE && */
				(iSel1 <= iRunBase)
				&& (iSel2 > iRunBase)
				)
			{
				// This image is selected. Now get the image size.
				
				UT_sint32 xoff = 0, yoff = 0;
				pRun->getLine()->getScreenOffsets(pRun, xoff, yoff);
	
				// Sevior's infamous + 1....
				yoff += pRun->getLine()->getAscent() - pRun->getAscent() + 1;				
				
				// Set the image size in the image selection rect
				m_selImageRect = UT_Rect(xoff,yoff,pRun->getWidth(),pRun->getHeight());
			}
		
			if (isOverImageResizeBox(m_imageSelCursor, m_xPoint, m_yPoint + m_iPointHeight))
			{
				return EV_EMC_IMAGESIZE;
			}
			else
			{
				return EV_EMC_IMAGE;
			}
		}			
	case FPRUN_TAB:
	case FPRUN_FORCEDLINEBREAK:
	case FPRUN_FORCEDCOLUMNBREAK:
	case FPRUN_FORCEDPAGEBREAK:
	case FPRUN_ENDOFPARAGRAPH:
	case FPRUN_FMTMARK:
	case FPRUN_BOOKMARK:
	case FPRUN_HYPERLINK:
	case FPRUN_DIRECTIONMARKER:
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
	UT_sint32 xPoint, yPoint, xPoint2, yPoint2;
	UT_uint32 pos = getPoint();
	UT_uint32 iPointHeight;
	bool bDirection;

//
// Check we have a vaild layout. At startup we don't
//
	if(getLayout()->getFirstSection() == NULL)
	{
		return NULL;
	}

	fl_BlockLayout * pBlock;
	fp_Run * pRun;

	_findPositionCoords(pos,
						m_bPointEOL,
						xPoint,
						yPoint,
						xPoint2,
						yPoint2,
						iPointHeight,
						bDirection,
						&pBlock,
						&pRun);

//
// Detect if we have no Lines at the curren tpoint and bail out.
//
 	if(pRun == NULL || pRun->getLine() == NULL || iPointHeight == 0)
 	{
 		return NULL;
 	}

	// NOTE prior call will fail if the block isn't currently formatted,
	// NOTE so we won't be able to figure out more specific geometry

	if (pRun)
	{
		// we now have coords relative to the page containing the ins pt
//		fp_Page* pPointPage = pRun->getLine()->getContainer()->getPage();
		fp_Page* pPointPage = pRun->getLine()->getPage();

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

// ndx is one-based, not zero-based
UT_UCSChar * FV_View::getContextSuggest(UT_uint32 ndx)
{
	// locate the squiggle
	PT_DocPosition pos = 0 ;
	fl_BlockLayout* pBL = NULL ;
	fl_PartOfBlock* pPOB = NULL ;

	pos = getPoint();
	pBL = _findBlockAtPosition(pos);
	UT_ASSERT(pBL);

	PT_DocPosition epos = 0;
	getDocument()->getBounds(true, epos);
	UT_DEBUGMSG(("end bound is %d\n", epos));
	pPOB = pBL->getSquiggles()->get(pos - pBL->getPosition());
	UT_return_val_if_fail(pPOB, NULL);

	// grab the suggestion
	return _lookupSuggestion(pBL, pPOB, ndx);
}

static bool s_notChar(UT_UCSChar c)
{
    switch (c)
    {
    case UCS_TAB:
    case UCS_LF:
    case UCS_VTAB:
    case UCS_FF:
    case UCS_CR:
    {
        return true;
    }
    default:
      return false;
    }
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
		m_pDoc->getBounds(false, low); // Need the whole document otherwise we just
		m_pDoc->getBounds(true, high); // word in the header/footer while editting there
	}
	else
	{
		if(m_iInsPoint < m_Selection.getSelectionAnchor())
		{
			low  = m_iInsPoint;
			high = m_Selection.getSelectionAnchor();
		}
		else
		{
			high = m_iInsPoint;
			low  = m_Selection.getSelectionAnchor();
		}
	}

	UT_sint32 iSelLen = high - low;
	fl_BlockLayout * pBL = _findBlockAtPosition(low);

	// Selection may start inside first block
	UT_sint32 iStartOffset = 0, iLineOffset = 0, iCount = 0;
	fp_Line* pLine = static_cast<fp_Line *>(pBL->getFirstContainer());
	fp_Run* pRun = pLine->getFirstRun();
	fp_Container * pColumn = pLine->getContainer();
	if(pColumn == NULL)
	{
		return (wCount);
	}
	fp_Page* pPage = pColumn->getPage();
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
				pRun = pRun->getNextRun();
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
		const UT_UCSChar * pSpan = reinterpret_cast<UT_UCSChar*>(gb.getPointer(0));
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
				pRun = pRun->getNextRun();
			}
			pLine = static_cast<fp_Line *>(pLine->getNext());
			UT_ASSERT((pLine && pRun) || (static_cast<void*>(pLine) == static_cast<void*>(pRun)));
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
		fl_ContainerLayout* pNextBlock = pBL->getNextBlockInDocument();
		pBL = static_cast<fl_BlockLayout *>(pNextBlock);
		pLine = NULL;
		pRun = NULL;
		if (pBL)
			pLine = static_cast<fp_Line *>(pBL->getFirstContainer());
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

		m_pLayout->rebuildFromHere(static_cast<fl_DocSectionLayout *>(m_pLayout->getFirstSection()));
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
	if (!isSelectionEmpty())
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
	_updateInsertionPoint();
}


/*!
 * Remove the Header/Footer type specified by hfType
\params HdrFtrType hfType the type of the Header/Footer to be removed.
\params bool bSkipPTSaves if true don't save the PieceTable stuff
 */
void FV_View::removeThisHdrFtr(HdrFtrType hfType, bool bSkipPTSaves)
{
	setCursorWait();
	if(!bSkipPTSaves)
	{
//
// Fix up the insertion point stuff.
//
		if (!isSelectionEmpty())
			_clearSelection();

		m_pDoc->beginUserAtomicGlob();

		_saveAndNotifyPieceTableChange();
	}
//
// Save current document position.
//
	PT_DocPosition curPoint = getPoint();

	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(getCurrentBlock()->getDocSectionLayout());

	if(hfType == FL_HDRFTR_HEADER)
	{
		UT_DEBUGMSG(("View: Remove HDRFTR HEADER \n"));
		_removeThisHdrFtr(pDSL->getHeader());
	}
	else if(hfType == FL_HDRFTR_HEADER_EVEN)
	{
		UT_DEBUGMSG(("View: Remove HDRFTR HEADER_EVEN \n"));
		_removeThisHdrFtr(pDSL->getHeaderEven());
	}
	else if(hfType == FL_HDRFTR_HEADER_LAST)
	{
		UT_DEBUGMSG(("View: Remove HDRFTR HEADER_LAST \n"));
		_removeThisHdrFtr(pDSL->getHeaderLast());
	}
	else if(hfType == FL_HDRFTR_HEADER_FIRST)
	{
		UT_DEBUGMSG(("View: Remove HDRFTR HEADER_FIRST \n"));
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
	_setPoint(curPoint);

	if(!bSkipPTSaves)
	{
		_generalUpdate();

		// Signal PieceTable Changes have finished
		_restorePieceTableState();
		updateScreen (); // fix 1803, force screen update/redraw

		_updateInsertionPoint();
		m_pDoc->endUserAtomicGlob();
	}
	clearCursorWait();
}


/*!
 *	 Insert the header/footer. Save the cursor position before we do this and
 *	 restore it to where it was before we did this.
\params HdrFtrType hfType
\params bool bSkipPTSaves if true don't save the PieceTable stuff
 */
void FV_View::createThisHdrFtr(HdrFtrType hfType, bool bSkipPTSaves)
{
	setCursorWait();
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
		if (!isSelectionEmpty())
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
		_updateInsertionPoint();
	}
	clearCursorWait();
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
	setCursorWait();
	if(!bSkipPTSaves)
	{
		if (!isSelectionEmpty())
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

	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(getCurrentBlock()->getSectionLayout());
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
//
// Make sure we have everythimng formatted.
//
	pHdrFtrSrc->format();
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
	UT_ASSERT(pHdrFtrDest);
	if(pHdrFtrDest)
	{
		_populateThisHdrFtr(pHdrFtrSrc, pHdrFtrDest);
		_setPoint(oldPos);
	}
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
		_updateInsertionPoint();
	}
	clearCursorWait();
}

/*!
 * This function returns true if there is a header on the current page.
\returns true if is there a header on the current page.
*/
bool FV_View::isHeaderOnPage(void)
{
	fp_Page * pPage = getCurrentPage();
	UT_return_val_if_fail(pPage, false);
	return (pPage->getHdrFtrP(FL_HDRFTR_HEADER) != NULL);
}

/*!
 * This function returns true if there is a footer on the current page.
\returns true if is there a footer on the current page.
*/
bool FV_View::isFooterOnPage(void)
{
	fp_Page * pPage = getCurrentPage();
	UT_return_val_if_fail(pPage, false);
	return (pPage->getHdrFtrP(FL_HDRFTR_FOOTER) != NULL);
}

/*!
 *	This method sets a bool variable which tells getEditableBounds we are
 *	editing a header/Footer.
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
 *	This method clears the bool variable which tells
 *  getEditableBounds we are
 *	editting a header/Footer.
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
	m_iSavedPosition = static_cast<PT_DocPosition>(0);
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
bool FV_View::getEditableBounds(bool isEnd, PT_DocPosition &posEOD, bool bOveride) const
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
		pSL = static_cast<fl_SectionLayout *>(m_pLayout->getLastSection());

		if(!pSL)
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			res = m_pDoc->getBounds(isEnd,posEOD);
			return res;
		}

		// So if there are no HdrFtr sections, return m_pDoc->getBounds.

		while(pSL->getNext() != NULL && (pSL->getContainerType() != FL_CONTAINER_HDRFTR))
		{
			pSL = static_cast<fl_SectionLayout *>(pSL->getNext());
		}
		if(pSL->getContainerType() != FL_CONTAINER_HDRFTR)
		{
			res = m_pDoc->getBounds(isEnd,posEOD);
			return res;
		}
//
// Now loop through all the HdrFtrSections, find the first in the doc and
// use that to get the end of editttable region.
//
		fl_BlockLayout * pFirstBL = static_cast<fl_BlockLayout *>(pSL->getFirstLayout());
		if(pFirstBL == NULL)
		{
			res = m_pDoc->getBounds(isEnd,posEOD);
			return res;
		}

		PT_DocPosition posFirst = pFirstBL->getPosition(true) - 1;
		PT_DocPosition posNext;
		while((pSL->getNext() != NULL) && (pSL->getFirstLayout() != NULL))
		{
			pSL = static_cast<fl_SectionLayout *>(pSL->getNext());
			pFirstBL = static_cast<fl_BlockLayout *>(pSL->getFirstLayout());
			// Make sure the first fl_BlockLayout of this new
			// fl_SectionLayout is valid.
			if (pFirstBL)
			{
				posNext = pFirstBL->getPosition(true) - 1;
				if(posNext < posFirst)
					posFirst = posNext;
			}
		}
		posEOD = posFirst;
		return res;
	}
//
// Constrain insertion point to the header/Footer Layout
//
	if(!isEnd)
	{
		posEOD = m_pEditShadow->getFirstLayout()->getPosition();
		return true;
	}
	pBL = static_cast<fl_BlockLayout *>(m_pEditShadow->getLastLayout());
	posEOD = pBL->getPosition(false);
	fp_Run * pRun = pBL->getFirstRun();
	while( pRun->getNextRun() != NULL)
	{
		pRun = pRun->getNextRun();
	}
	posEOD += pRun->getBlockOffset();
	return true;
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
	setCursorWait();
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

// Update Layout everywhere. This actually creates the header/footer container

	m_pLayout->updateLayout(); // Update document layout everywhere
	m_pDoc->endUserAtomicGlob(); // End the big undo block
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
	_updateInsertionPoint();
	clearCursorWait();
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

	UT_return_val_if_fail(m_pDoc,false);
	UT_uint32 id = m_pDoc->getUID(UT_UniqueId::HeaderFtr);
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
	fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pDocL->getFirstLayout());
	PT_DocPosition posSec = pBL->getPosition();

	// change the section to point to the footer which doesn't exist yet.

	m_pDoc->changeStruxFmt(PTC_AddFmt, posSec, posSec, sec_attributes2, NULL, PTX_Section);


	moveInsPtTo(FV_DOCPOS_EOD); // Move to the end, where we will create the header/footer


// insert the Header/Footer
	PT_DocPosition posBlock = getPoint() +1;
	m_pDoc->insertStrux(getPoint(), PTX_SectionHdrFtr,sec_attributes1, NULL);

// Now the block strux for the content

	m_pDoc->insertStrux(posBlock, PTX_Block,NULL, props);

// OK it's in!
 	m_pDoc->signalListeners(PD_SIGNAL_REFORMAT_LAYOUT);
	return true;
}

/*!
 * This method returns the depth of the embedding level at the requested point.
 * If the point is not inside a footnote or Endnote, we return zero. If the 
 * point is inside a footnote or endnote return 1, if the  point is inside
 * an endnote inside a footnote return 2 etc.
 */
UT_sint32 FV_View::getEmbedDepth(PT_DocPosition pos)
{
	fl_BlockLayout * pBL =	m_pLayout->findBlockAtPosition(pos);
	fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pBL->myContainingLayout());
	bool bStop = false;
	UT_sint32 count =-1;
	while(!bStop && pCL)
	{
		count++;
		bStop = ((pCL->getContainerType() != FL_CONTAINER_FOOTNOTE) && (pCL->getContainerType() != FL_CONTAINER_ENDNOTE));
		pCL = static_cast<fl_ContainerLayout *>(pCL->myContainingLayout());
	}
	return count;
}
/*!
 * This method returns the closest footnote before or that contains the 
 * requested doc position. If the is no footnote before the doc position, NULL
 * is returned.
 */
fl_FootnoteLayout * FV_View::getClosestFootnote(PT_DocPosition pos)
{
	fl_FootnoteLayout * pFL = NULL;
	fl_FootnoteLayout * pClosest = NULL;
	UT_sint32 i = 0;
	for(i = 0; i< static_cast<UT_sint32>(m_pLayout->countFootnotes());i++)
	{
		pFL = m_pLayout->getNthFootnote(i);
		if(pFL->getDocPosition() <= pos)
		{
			if(pClosest == NULL)
			{
				pClosest = pFL;
			}
			else if( pClosest->getDocPosition() < pFL->getDocPosition())
			{
				pClosest = pFL;
			}
		}
	}
	return pClosest;
}


/*!
 * This method returns the closest endnote before or that contains the 
 * requested doc position. If the is no footnote before the doc position, NULL
 * is returned.
 */
fl_EndnoteLayout * FV_View::getClosestEndnote(PT_DocPosition pos)
{
	fl_EndnoteLayout * pFL = NULL;
	fl_EndnoteLayout * pClosest = NULL;
	UT_sint32 i = 0;
	for(i = 0; i< static_cast<UT_sint32>(m_pLayout->countEndnotes());i++)
	{
		pFL = m_pLayout->getNthEndnote(i);
		if(pFL->getDocPosition() <= pos)
		{
			if(pClosest == NULL)
			{
				pClosest = pFL;
			}
			else if( pClosest->getDocPosition() < pFL->getDocPosition())
			{
				pClosest = pFL;
			}
		}
	}
	return pClosest;
}

/*! 
 * Returns true if the point is located with a block so stuff can be typed
 * or inserted.
 */
bool FV_View::isPointLegal(PT_DocPosition pos)
{
	PL_StruxDocHandle prevSDH = NULL;
	PL_StruxDocHandle nextSDH = NULL;
	PT_DocPosition nextPos =0;
//
// Special case which would otherwise fail..
//
	if(m_pDoc->isEndFootnoteAtPos(pos))
	{
		return true;
	}
	bool bres = m_pDoc->getStruxOfTypeFromPosition(pos,PTX_Block,&prevSDH);
	if(!bres)
	{
		return false;
	}
	bres = m_pDoc->getNextStrux(prevSDH,&nextSDH);
	if(!bres)
	{
		return true;
	}
	nextPos =  m_pDoc->getStruxPosition(nextSDH);
	if(pos > nextPos && (m_pDoc->getStruxType(nextSDH) != PTX_Block))
	{
		return false;
	}
	return true;
}

bool FV_View::isPointLegal(void)
{
	return isPointLegal(getPoint());
}

/*!
 * Returns true if the current insertion point is inside a footnote.
 */
bool FV_View::isInFootnote(void)
{
	return isInFootnote(getPoint());
}

/*!
 * Returns true if the requested position is inside a footnote.
 */
bool FV_View::isInFootnote(PT_DocPosition pos)
{
	fl_FootnoteLayout * pFL = getClosestFootnote(pos);
	if(pFL == NULL)
	{
		return false;
	}
	if((pFL->getDocPosition() <= pos) && ((pFL->getDocPosition() + pFL->getLength()) > pos))
	{
		return true;
	}
	return false;
}

/*!
 * Returns true if the current insertion point is inside a footnote.
 */
bool FV_View::isInEndnote(void)
{
	return isInEndnote(getPoint());
}

/*!
 * Returns true if the requested position is inside a endnote.
 */
bool FV_View::isInEndnote(PT_DocPosition pos)
{
	fl_EndnoteLayout * pFL = getClosestEndnote(pos);
	if(pFL == NULL)
	{
		return false;
	}
	if((pFL->getDocPosition() <= pos) && ((pFL->getDocPosition() + pFL->getLength()) > pos))
	{
		return true;
	}
	return false;
}

bool FV_View::insertFootnote(bool bFootnote)
{
	// can only insert Footnote into an FL_SECTION_DOC or a Table
	fl_SectionLayout * pSL =  _findBlockAtPosition(getPoint())->getSectionLayout();
	if ( (pSL->getContainerType() != FL_CONTAINER_DOCSECTION) && (pSL->getContainerType() != FL_CONTAINER_CELL) )
		return false;
//
// Do this first
//
	const XML_Char ** props_in = NULL;
	getCharFormat(&props_in);

	// add field for footnote reference
	// first, make up an id for this footnote.
	UT_String footpid;
	UT_return_val_if_fail(m_pDoc, false);
	UT_uint32 pid = m_pDoc->getUID(bFootnote ? UT_UniqueId::Footnote : UT_UniqueId::Endnote);
	UT_String_sprintf(footpid,"%d",pid);
	
	const XML_Char* attrs[] = {
		"footnote-id", footpid.c_str(),
		NULL, NULL
	};
	if(!bFootnote)
	{
		attrs[0] = "endnote-id";
	}

	/*	Apply the character style at insertion point and insert the
		Footnote reference. */

	PT_DocPosition FrefStart = getPoint();
	PT_DocPosition FrefEnd = FrefStart + 1;
	PT_DocPosition FanchStart;
	PT_DocPosition FanchEnd;
	PT_DocPosition FbodyEnd;

	const XML_Char *cur_style;
	getStyle(&cur_style);

	m_pDoc->beginUserAtomicGlob();
	_saveAndNotifyPieceTableChange();
	bool bCreatedFootnoteSL = false;

	PT_DocPosition dpFT = 0;
	const XML_Char * dumProps[3] = {"list-tag","123",NULL};
	PT_DocPosition dpBody = getPoint();
//
// This does a rebuild of the affected paragraph
//
	m_pDoc->changeStruxFmt(PTC_AddFmt,dpBody,dpBody,NULL,dumProps,PTX_Block);
	if (!insertFootnoteSection(bFootnote,footpid.c_str()))
	{
		m_pDoc->endUserAtomicGlob();
		_restorePieceTableState();
		return false;
	}
	dpFT = getPoint(); // Points right at the EndFootnote strux
	bCreatedFootnoteSL = true;
	_setPoint(dpBody);
	FrefStart = dpBody;
	bool bRet = false;
	if(bFootnote)
	{
		if (cmdInsertField("footnote_ref", attrs)==false)
			return false;
		FrefEnd = FrefStart+1;
		setStyleAtPos("Footnote Reference", FrefStart, FrefEnd,true);
//
// Put the character format back to it previous value
//
		bRet = m_pDoc->changeSpanFmt(PTC_AddFmt,getPoint(),getPoint(),NULL,props_in);
		setCharFormat(props_in);
	}
	else
	{
		if (cmdInsertField("endnote_ref", attrs)==false)
			return false;
		FrefEnd = FrefStart+1;
		setStyleAtPos("Endnote Reference", FrefStart, FrefEnd,true);
//
// Put the character format back to it previous value
//
		bRet = m_pDoc->changeSpanFmt(PTC_AddFmt,getPoint(),getPoint(),NULL,props_in);
	}
	free(props_in);
	fl_BlockLayout * pBL;

//
// Now set the insertion point inside the Footnote and insert the anchor field
// there.
//

	// add footnote anchor, inside footnote section
	//get ready to apply Footnote Reference style
	FanchStart = dpFT;

	// if the block after which were inserted was not the last block
	// we have to adjust the postion, because we are now siting at the
	// start of the next block instead of our own
	UT_DEBUGMSG(("fv_View::insertFootnote: FanchStart %d\n",FanchStart));

	_clearSelection();
	_setPoint(FanchStart);
	if(bFootnote)
	{
		cmdInsertField("footnote_anchor", attrs);
	}
	else
	{
		cmdInsertField("endnote_anchor", attrs);
	}
//
// Place a format mark before the field so we can select the field.
//
	const XML_Char * propListTag[] = {"list-tag",NULL,NULL};
	static XML_Char sid[15];
	UT_uint32 id = m_pDoc->getUID(UT_UniqueId::HeaderFtr);
	sprintf(sid, "%i", id);
	propListTag[1] = sid;
	bRet = m_pDoc->changeSpanFmt(PTC_AddFmt,FanchStart,FanchStart,NULL,propListTag);

	FanchEnd = FanchStart+1;
	UT_DEBUGMSG(("insertFootnote: Inserting space after anchor field \n"));
	//insert a space after the anchor
	UT_UCSChar space = UCS_SPACE;
	m_pDoc->insertSpan(FanchEnd, &space, 1);


	// apply footnote text style to the body of the footnote and the
	// reference style to the anchor follows it
	propListTag[0]="text-position";
	propListTag[1]="superscript";
	if(bFootnote)
	{
		setStyleAtPos("Footnote Text", FanchStart, FanchEnd, true);
//
// Superscript the anchor
//
		bRet = m_pDoc->changeSpanFmt(PTC_AddFmt,FanchStart,FanchStart+1,NULL,propListTag);

	}
	else
	{
		setStyleAtPos("Endnote Text", FanchStart, FanchEnd, true);
//
// Superscript the anchor
//
		bRet = m_pDoc->changeSpanFmt(PTC_AddFmt,FanchStart,FanchStart+1,NULL,propListTag);
	}

	_setPoint(FanchEnd+1);
	_resetSelection(); // needed because of the setStyle calls ...
	FbodyEnd = getPoint();
	
	/*	some magic to make the endnote reference and anchor recalculate
		its widths
	*/
	fp_Run* pRun;
	UT_sint32 x, y, x2, y2;
	UT_uint32 height;
	bool bDirection;
	_findPositionCoords(FrefStart, false, x, y, x2, y2, height, bDirection,&pBL,&pRun);

	UT_ASSERT(pBL != 0);
	UT_ASSERT(pRun != 0);

	bool bWidthChange = pRun->recalcWidth();
	xxx_UT_DEBUGMSG(("run type %d, width change %d\n", pRun->getType(),bWidthChange));
	pBL->setNeedsReformat();

	pBL = _findBlockAtPosition(FanchStart);
	UT_ASSERT(pBL != 0);

	if (pBL->getFirstRun()->getNextRun())
	{
		bWidthChange = pBL->getFirstRun()->getNextRun()->recalcWidth();
		xxx_UT_DEBUGMSG(("run type %d, width change %d\n", pBL->getFirstRun()->getNextRun()->getType(),bWidthChange));
		pBL->setNeedsReformat();
	}
//
// This does a rebuild of the affected paragraph
//
	m_pDoc->changeStruxFmt(PTC_RemoveFmt,dpBody,dpBody,NULL,dumProps,PTX_Block);
	m_pDoc->endUserAtomicGlob();
	setScreenUpdateOnGeneralUpdate( true);
	_restorePieceTableState(); // clean up remaining measures
	_updateInsertionPoint();
	_ensureInsertionPointOnScreen();
	_generalUpdate();
	_fixInsertionPointCoords();
//
// Lets have a peek at the doc structure, shall we?
//
//	m_pDoc->miniDump(pBL->getStruxDocHandle(),8);
	return true;
}

bool FV_View::insertFootnoteSection(bool bFootnote,const XML_Char * enpid)
{
	const XML_Char* block_attrs[] = {
		"footnote-id", enpid,
		NULL, NULL
	};
	if(!bFootnote)
	{
		block_attrs[0] = "endnote-id";
	}
	const XML_Char* block_attrs2[] = {
		"footnote-id", enpid,
		"style", "Footnote Text", // xxx 'Footnote Body'
		NULL, NULL
	};
	if(!bFootnote)
	{
		block_attrs2[0] = "endnote-id";
		block_attrs2[3] = "Endnote Text";
	}
	m_pDoc->beginUserAtomicGlob(); // Begin the big undo block

	// Signal PieceTable Changes have Started
	//UT_DEBUGMSG(("insertFootnoteSection: about to save and notify\n"));
	_saveAndNotifyPieceTableChange();
	m_pDoc->disableListUpdates();

	bool e = false;

	/*
	  This inserts a footnote at the current point
	  and leaves the insertion point there.
	*/
    PT_DocPosition pointBreak = 0;

	pointBreak = getPoint();
 	PT_DocPosition pointFootnote = 0;
//
// Insert the footnote strux at this spot. The footnote strux in the piecetable
// will make all the text inside the footnote section invisible so that only
// we can place the footnote section inside the block containing the reference.
//
	UT_DEBUGMSG(("insertFootnoetSection: about to insert footnote section at %d\n",pointBreak));
	if(bFootnote)
	{
		e |= m_pDoc->insertStrux(pointBreak,PTX_SectionFootnote,block_attrs,NULL);
	}
	else
	{
		e |= m_pDoc->insertStrux(pointBreak,PTX_SectionEndnote,block_attrs,NULL);
	}

	pointFootnote = pointBreak+1;
	UT_DEBUGMSG(("insertFootnoteSection: about to insert block for footnote section at %d \n",pointFootnote));
 	e |= m_pDoc->insertStrux(pointFootnote,PTX_Block,block_attrs2,NULL);
	pointFootnote++;
	UT_DEBUGMSG(("insertFootnoteSection: about to insert end footnote at %d \n",pointFootnote));
	if(bFootnote)
	{
		e |= m_pDoc->insertStrux(pointFootnote,PTX_EndFootnote,block_attrs,NULL);
	}
	else
	{
		e |= m_pDoc->insertStrux(pointFootnote,PTX_EndEndnote,block_attrs,NULL);
	}
    pointFootnote++;
	_setPoint(pointFootnote);
	UT_DEBUGMSG(("insertFootnoteSection: inserted everything final point is %d \n",getPoint()));

//	m_pDoc->signalListeners(PD_SIGNAL_REFORMAT_LAYOUT);

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	m_pDoc->endUserAtomicGlob(); // End the big undo block

	_generalUpdate();

	// Signal PieceTable Changes have Ended
	_restorePieceTableState();
	_updateInsertionPoint();
	UT_DEBUGMSG(("insertFootnoteSection: Finished point is %d \n",getPoint()));

	return e;
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
	_updateInsertionPoint();
	return bResult;
}

const fp_PageSize & FV_View::getPageSize(void) const
{
	return m_pDoc->m_docPageSize;
}


UT_uint32 FV_View::calculateZoomPercentForPageWidth()
{
	const fp_PageSize pageSize = getPageSize();
	double pageWidth = pageSize.Width(DIM_IN);

	// Verify scale as a positive non-zero number else return old zoom
	UT_uint32 iZoom = 100;
	UT_sint32 iWindowWidth =  getWindowWidth();
	if(iWindowWidth == 0)
	{
	// Get fall-back defaults for zoom from prefs
		const XML_Char * szZoom = NULL;
		m_pApp->getPrefsValue(XAP_PREF_KEY_ZoomPercentage,
							  static_cast<const XML_Char**>(&szZoom));
		UT_DEBUGMSG(("!!!! Zoom percentage  %s \n",szZoom));
		if(szZoom)
		{
			iZoom = atoi(szZoom);
			if(iZoom < XAP_DLG_ZOOM_MINIMUM_ZOOM) 
				iZoom = 100;
			else if (iZoom > XAP_DLG_ZOOM_MAXIMUM_ZOOM) 
				iZoom = 100;
			return iZoom;
		}
		return getGraphics()->getZoomPercentage();
	}
	if ( ( iWindowWidth - 2 * getPageViewLeftMargin() ) <= 0 )
		return getGraphics()->getZoomPercentage();

	double scale = (getWindowWidth() - 2 * getPageViewLeftMargin()) /
		(pageWidth * static_cast<double>(getGraphics()->getResolution() / 
								   getGraphics()->getZoomPercentage() * 100.0));

	// Don't do the change if it's less than 3% from current percentage - PL
	if (abs(static_cast<int>(scale * 100.0) - 
			getGraphics()->getZoomPercentage()) < 3)
		return getGraphics()->getZoomPercentage();

	return static_cast<UT_uint32>(scale * 100.0);
}

UT_uint32 FV_View::calculateZoomPercentForPageHeight()
{

	const fp_PageSize pageSize = getPageSize();
	double pageHeight = pageSize.Height(DIM_IN);
	UT_uint32 iZoom = 100;
	UT_sint32 iWindowHeight =  getWindowHeight();
	if(iWindowHeight == 0)
	{
	// Get fall-back defaults for zoom from prefs
		const XML_Char * szZoom = NULL;
		m_pApp->getPrefsValue(XAP_PREF_KEY_ZoomPercentage,
							  static_cast<const XML_Char**>(&szZoom));
		if(szZoom)
		{
			iZoom = atoi(szZoom);
			UT_DEBUGMSG(("!!!! Zoom percentage  %s \n",szZoom));
			if(iZoom < XAP_DLG_ZOOM_MINIMUM_ZOOM) 
				iZoom = 100;
			else if (iZoom > XAP_DLG_ZOOM_MAXIMUM_ZOOM) 
				iZoom = 100;
			return iZoom;
		}
		return getGraphics()->getZoomPercentage();
	}
	// Verify scale as a positive non-zero number else return old zoom
	if ( ( iWindowHeight - 2 * getPageViewTopMargin() ) <= 0 )
		return getGraphics()->getZoomPercentage();

	double scale = (getWindowHeight() - 2 * getPageViewTopMargin()) /
		(pageHeight * static_cast<double>(getGraphics()->getResolution() /
		                         getGraphics()->getZoomPercentage() * 100.0));

	// Don't do the change if it's less than 10% from current percentage - PL
	if (abs(static_cast<int>(scale * 100.0) -
			getGraphics()->getZoomPercentage()) < 10)
		return getGraphics()->getZoomPercentage();

	return static_cast<UT_uint32>(scale * 100.0);
}

UT_uint32 FV_View::calculateZoomPercentForWholePage()
{
	return UT_MIN(	calculateZoomPercentForPageWidth(),
					calculateZoomPercentForPageHeight());
}

/* Revision related functions */
void FV_View::toggleMarkRevisions()
{
	m_pDoc->toggleMarkRevisions();

	// if we turned the revisions off, we want to remove any revisions
	// attribute from the formatting at the insertion point so as not
	// to have any newly entered text marked as revision
	// we will only do this if there is no selection ...
	if(!isMarkRevisions() && isSelectionEmpty())
	{
		bool bRet;

		// Signal PieceTable Change
		_saveAndNotifyPieceTableChange();

		PT_DocPosition posStart = getPoint();
		PT_DocPosition posEnd = posStart;

		const XML_Char rev[] = "revision";
		const XML_Char val[] = "";
		const XML_Char * attr[3] = {rev,val,NULL};

		bRet = m_pDoc->changeSpanFmt(PTC_RemoveFmt,posStart,posEnd,attr,NULL);

		// Signal piceTable is stable again
		_restorePieceTableState();

		// might need to do general update here; leave it off for now
		// _generalUpdate();
		_fixInsertionPointCoords();
	}

	if(isMarkRevisions() && !m_bShowRevisions)
	{
		// make sure we see what we are doing ...
		setShowRevisions(true);
	}
}

void FV_View::setShowRevisions(bool bShow)
{
	if(m_bShowRevisions != bShow)
	{
		m_bShowRevisions = bShow;

		// set the doc value as well, so that on save we preserve the last
		// used setting
		m_pDoc->setShowRevisions(bShow);

		// now we have to re-do document layout from bottom up
		m_pLayout->rebuildFromHere(static_cast<fl_DocSectionLayout *>(m_pLayout->getFirstSection()));

		_fixInsertionPointCoords();
	}
}

void FV_View::toggleShowRevisions()
{
	setShowRevisions(!m_bShowRevisions);
}

/*!
    same as cmdSetRevisionLevel() except without layout rebuild
 */
void FV_View::setRevisionLevel(UT_uint32 i)
{
	m_pDoc->setShowRevisionId(i);
	m_iViewRevision = i;
}

bool FV_View::isMarkRevisions()
{
	return m_pDoc->isMarkRevisions();
}

/* Table related functions */
/*!
 * returns true if the current insertion point is inside a table.
 */
bool FV_View::isInTable()
{
	PT_DocPosition pos;

	if (isSelectionEmpty())
	{
		pos = getPoint();
	}
	else
	{
		pos = (m_iInsPoint < m_Selection.getSelectionAnchor() ? m_iInsPoint : m_Selection.getSelectionAnchor());
	}
	return isInTable(pos);
}

fl_TableLayout * FV_View::getTableAtPos(PT_DocPosition pos)
{
	fl_BlockLayout * pBL =	m_pLayout->findBlockAtPosition(pos);
	if(!pBL)
	{
		return NULL;
	}
	fl_ContainerLayout * pCL = pBL->myContainingLayout();
	if(!pCL)
	{
		return NULL;
	}
	if(pCL->getContainerType() == FL_CONTAINER_CELL)
	{
		pCL = pCL->myContainingLayout();
		if(!pCL)
		{
			return NULL;
		}
		if(pCL->getContainerType() != FL_CONTAINER_TABLE)
		{
			return NULL;
		}
		fl_TableLayout * pTab = static_cast<fl_TableLayout *>(pCL);
		return pTab;
	}
	return NULL;
}
/*!
 * Returns true if the point supplied is inside a Table.
 */
bool FV_View::isInTable( PT_DocPosition pos)
{
	fl_BlockLayout * pBL =	m_pLayout->findBlockAtPosition(pos);
	if(!pBL)
	{
		return false;
	}
	fl_ContainerLayout * pCL = pBL->myContainingLayout();
	if(!pCL)
	{
		return false;
	}
	if(pCL->getContainerType() == FL_CONTAINER_CELL)
	{
		return true;
	}
	pCL = pBL->getNext();
	if(pCL == NULL)
	{
		return false;
	}
	if(pCL->getContainerType() == FL_CONTAINER_TABLE)
	{
		PT_DocPosition posTable = m_pDoc->getStruxPosition(pCL->getStruxDocHandle());
		if(posTable == pos)
		{
			return true;
		}
	}
	pCL = pBL->getPrev();
	if(pCL == NULL)
	{
		return false;
	}
	if(pCL->getContainerType() == FL_CONTAINER_TABLE)
	{
		PL_StruxDocHandle sdh = pCL->getStruxDocHandle();
		PL_StruxDocHandle sdhEnd = m_pDoc->getEndTableStruxFromTableSDH(sdh);
		if(sdhEnd != NULL)
		{
			PT_DocPosition posEnd =  m_pDoc->getStruxPosition(sdhEnd);
			if(posEnd == pos)
			{
				return true;
			}
		}
	}
	return false;
}

/*!
 * Returns the position of the cell strux of cell specified by (row,col) within the
 * Table surrounding the supplied point.
 */
PT_DocPosition FV_View::findCellPosAt(PT_DocPosition posTable, UT_sint32 row, UT_sint32 col)
{
	PL_StruxDocHandle cellSDH,tableSDH;
	bool bRes = m_pDoc->getStruxOfTypeFromPosition(posTable,PTX_SectionTable,&tableSDH);
	if(!bRes)
	{
		return 0;
	}
	cellSDH = m_pDoc->getCellSDHFromRowCol(tableSDH, row,col);
	if(cellSDH == NULL)
	{
		return 0;
	}
	return 	m_pDoc->getStruxPosition(cellSDH);
}

/*! find out which pages in the document are visible on the screen and
    calculate the rectangles of their view-ports (the rectangles are
    relative to the top-left corner of the page, not to the screen
    the caller must use UT_VECTOR_PURGEALL() on vRect to delete the
    objects allocated by this function, but NOT on vPages
    \param vRect -- vector where to store UT_Rect* referring to vieports of pages in vPages
    \param vPage -- vector where to store pointers to currently visible pages
*/
void FV_View:: getVisibleDocumentPagesAndRectangles(UT_Vector &vRect, UT_Vector &vPages) const
{
	UT_sint32 curY = getPageViewTopMargin();
	fp_Page * pPage = m_pLayout->getFirstPage();

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

		if (adjustedTop > getWindowHeight())
		{
			// the start of this page is past the bottom
			// of the window, so we don't need to draw it.

			xxx_UT_DEBUGMSG(("page below port: PageHeight=%d curY=%d nPos=%d WindowHeight=%d\n",
							 iPageHeight,
							 curY,
							 m_yScrollOffset,
							 getWindowHeight()));

			// since all other pages are below this one, we
			// don't need to draw them either.	exit loop now.
			break;
		}
		else if (adjustedBottom < 0)
		{
			// the end of this page is above the top of
			// the window, so we don't need to draw it.

			xxx_UT_DEBUGMSG(("page above port: PageHeight=%d curY=%d nPos=%d WindowHeight=%d\n",
							 iPageHeight,
							 curY,
							 m_yScrollOffset,
							 getWindowHeight()));
		}
		else
		{
			// this page is on screen
			xxx_UT_DEBUGMSG(("page visible: height=%d curY=%d nPos=%d WindowHeight=%d\n",
						 iPageHeight,
						 curY,
						 m_yScrollOffset,
						 getWindowHeight()));


			vPages.addItem(static_cast<void*>(pPage));

			// now create the rectangle
			// NB the adjustedTop is relative to the screen, but we
			// want the rect to be relative to the top left page
			// corner

			UT_sint32 iLeftGrayWidth = getPageViewLeftMargin() - m_xScrollOffset;
			UT_uint32 iPortTop       = adjustedTop >= 0 ? 0 : -adjustedTop;
			UT_uint32 iPortLeft      = iLeftGrayWidth >= 0 ? 0 : -iLeftGrayWidth;
			UT_uint32 iWindowWidth   = getWindowWidth() - iLeftGrayWidth > 0 ? getWindowWidth() - iLeftGrayWidth : 0;
			UT_uint32 iPortHeight;
			if( adjustedBottom <= getWindowHeight() && adjustedTop >=0)
			{
				iPortHeight = adjustedBottom - adjustedTop;
			}
			else if(adjustedBottom <= getWindowHeight() && adjustedTop <= 0)
			{
				iPortHeight = adjustedBottom;
			}
			else if(adjustedBottom >= getWindowHeight() && adjustedTop >=0)
			{
				iPortHeight = getWindowHeight() - adjustedTop;
			}
			else if(adjustedBottom >= getWindowHeight() && adjustedTop <=0)
			{
				iPortHeight = getWindowHeight();
			}
			else
				UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
			
			
			
			UT_uint32 iPortWidth = UT_MIN(static_cast<UT_uint32>(iPageWidth), iWindowWidth);

			UT_Rect * pRect = new UT_Rect(iPortLeft,
										  iPortTop,
										  iPortWidth,
										  iPortHeight);

			vRect.addItem(static_cast<void*>(pRect));
		}

		curY += iPageHeight + getPageViewSep();

		pPage = pPage->getNext();
	}
}

void FV_View::setImageSelRect(UT_Rect r)
{
	m_selImageRect = r;
}

UT_Rect FV_View::getImageSelRect()
{
	return m_selImageRect;
}

/*! Returns the size of the image selection boxes
*/
UT_sint32 FV_View::getImageSelInfo()
{
	return m_iImageSelBoxSize;
}

GR_Graphics::Cursor FV_View::getImageSelCursor()
{
	return m_imageSelCursor;
}

/*! Returns true if the given coords are in an image selection box, false otherwise.

    \param cur -- Will be set to the appropriate mouse cursor, if the given xPos and yPos are 
                  in a selection box. Will be left unchanged otherwise
    \param xPos -- x position to check
    \param xPos -- y position to check
*/
bool FV_View::isOverImageResizeBox(GR_Graphics::Cursor &cur, UT_uint32 xPos, UT_uint32 yPos)
{
	if (UT_Rect(m_selImageRect.left, m_selImageRect.top, m_iImageSelBoxSize, m_iImageSelBoxSize).containsPoint(xPos, yPos))
	{
		cur = GR_Graphics::GR_CURSOR_IMAGESIZE_NW; // North West
		return true;
	}

	if (UT_Rect(m_selImageRect.left + (m_selImageRect.width/2) - m_iImageSelBoxSize/2, m_selImageRect.top, m_iImageSelBoxSize, m_iImageSelBoxSize).containsPoint(xPos, yPos))
	{
		cur  = GR_Graphics::GR_CURSOR_IMAGESIZE_N; // North
		return true;
	}
	
	if (UT_Rect(m_selImageRect.left + m_selImageRect.width - m_iImageSelBoxSize, m_selImageRect.top, m_iImageSelBoxSize, m_iImageSelBoxSize).containsPoint(xPos, yPos))
	{
		cur  = GR_Graphics::GR_CURSOR_IMAGESIZE_NE; // North East
		return true;
	}
	
	if (UT_Rect(m_selImageRect.left + m_selImageRect.width - m_iImageSelBoxSize, m_selImageRect.top + m_selImageRect.height/2 - m_iImageSelBoxSize/2, m_iImageSelBoxSize, m_iImageSelBoxSize).containsPoint(xPos, yPos))
	{
		cur  = GR_Graphics::GR_CURSOR_IMAGESIZE_E; // East
		return true;
	}

	if (UT_Rect(m_selImageRect.left + m_selImageRect.width - m_iImageSelBoxSize, m_selImageRect.top + m_selImageRect.height - m_iImageSelBoxSize, m_iImageSelBoxSize, m_iImageSelBoxSize).containsPoint(xPos, yPos))
	{
		cur  = GR_Graphics::GR_CURSOR_IMAGESIZE_SE; // South East
		return true;
	}

	if (UT_Rect(m_selImageRect.left + (m_selImageRect.width/2) - m_iImageSelBoxSize/2, m_selImageRect.top + m_selImageRect.height - m_iImageSelBoxSize, m_iImageSelBoxSize, m_iImageSelBoxSize).containsPoint(xPos, yPos))
	{
		cur  = GR_Graphics::GR_CURSOR_IMAGESIZE_S; // South
		return true;
	}

	if (UT_Rect(m_selImageRect.left, m_selImageRect.top + m_selImageRect.height - m_iImageSelBoxSize, m_iImageSelBoxSize, m_iImageSelBoxSize).containsPoint(xPos, yPos))
	{
		cur  = GR_Graphics::GR_CURSOR_IMAGESIZE_SW; // South West
		return true;
	}

	if (UT_Rect(m_selImageRect.left, m_selImageRect.top + m_selImageRect.height/2 - m_iImageSelBoxSize/2, m_iImageSelBoxSize, m_iImageSelBoxSize).containsPoint(xPos, yPos))
	{
		cur = GR_Graphics::GR_CURSOR_IMAGESIZE_W; // West
		return true;
	}
	
	return false;
}

void FV_View::startImageResizing(UT_sint32 xPos, UT_sint32 yPos)
{
	m_ixResizeOrigin = xPos;
	m_iyResizeOrigin = yPos;
	m_bIsResizingImage = true;
}

void FV_View::stopImageResizing()
{
	m_bIsResizingImage = false;
}

bool FV_View::isResizingImage()
{
	return m_bIsResizingImage;
}

void FV_View::getResizeOrigin(UT_sint32 &xOrigin, UT_sint32 &yOrigin)
{
	xOrigin = m_ixResizeOrigin;
	yOrigin = m_iyResizeOrigin;
}

void FV_View::setCurImageSel(UT_Rect r)
{
	m_curImageSel = r;
}

UT_Rect FV_View::getCurImageSel()
{
	return m_curImageSel;
}

bool FV_View::isDraggingImage()
{
	return m_bIsDraggingImage;
}

void FV_View::startImageDrag(fp_Run * pRun, UT_sint32 xPos, UT_sint32 yPos)
{
	UT_ASSERT(pRun);
	
	m_pDraggedImageRun = pRun;
	
	UT_sint32 xoff = 0, yoff = 0;
	pRun->getLine()->getScreenOffsets(pRun, xoff, yoff);

	// Sevior's infamous + 1....
	yoff += pRun->getLine()->getAscent() - pRun->getAscent() + m_pG->tlu(1);				
	
	// Set the image size in the image selection rect
	m_dragImageRect = UT_Rect(xoff,yoff,pRun->getWidth(),pRun->getHeight());
	
	// Set the current image position to the original image position
	m_curImageSel = m_dragImageRect;
	
	m_ixDragOrigin = xPos;
	m_iyDragOrigin = yPos;
	
	m_bIsDraggingImage = true;
}

void FV_View::drawDraggedImage(UT_sint32 xPos, UT_sint32 yPos)
{
	UT_ASSERT(m_pDraggedImageRun);
	GR_Image * pImage = (static_cast<fp_ImageRun *>(m_pDraggedImageRun))->getImage();
	UT_ASSERT(pImage);

	GR_Painter painter(getGraphics());

	// calculate the new image boundaries
	UT_Rect bounds = UT_Rect(xPos - (m_ixDragOrigin - m_dragImageRect.left), yPos - (m_iyDragOrigin - m_dragImageRect.top), m_dragImageRect.width, m_dragImageRect.height);
	
	/*
	
	Now we need to intelligently calculate which parts of the view we need to redraw.
	See the following picture:
	
	+--------+
	|***1****|  <- Old Image Position
	|---+--------+
	|***|        |
	|*2*|  New   |
	|***| Image  |
	+---|Position|
	    |        |
	    +--------+
	
	* = denotes an area that needs to be redrawn
	
	We see that there are two squares that need to be redrawn, square 1 and square 2. The code
	below calculates those positions and redraws them.
	
	*/
	
	UT_Rect clipRect;
	
	// redraw the 1st square
	clipRect = UT_Rect( 
						m_curImageSel.left,
						(m_curImageSel.top <= bounds.top ? m_curImageSel.top : bounds.top + bounds.height),
						m_curImageSel.width,
						abs(bounds.top - m_curImageSel.top)
					  );
	draw(&clipRect);
	
	// redraw the 2nd square
	clipRect = UT_Rect( 
						(m_curImageSel.left <= bounds.left ? m_curImageSel.left : bounds.left + bounds.width),
						UT_MAX(m_curImageSel.top, bounds.top),
						abs(bounds.left - m_curImageSel.left),
						m_curImageSel.height - abs(bounds.top - m_curImageSel.top)
					  );
	draw(&clipRect);
	
	painter.drawImage(pImage, bounds.left, bounds.top);
	m_curImageSel = bounds;
}

void FV_View::stopImageDrag(UT_sint32 xPos, UT_sint32 yPos)
{
	m_bIsDraggingImage = false;
	
	// clear the last dragged image
	draw(&m_curImageSel);	
	
	//
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	// Turn off list updates

	m_pDoc->disableListUpdates();

	// turn off immediate layout of table

	m_pDoc->setDontImmediatelyLayout(true);

	
	m_pDoc->beginUserAtomicGlob();
			
	PT_DocPosition pos = getDocPositionFromXY(xPos, yPos);
		
	cmdCut();
	moveInsPtTo(pos);
	cmdPaste();
	
	m_pDoc->endUserAtomicGlob();
	
	// Allow updates

	m_pDoc->setDontImmediatelyLayout(false);

	_generalUpdate();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
}



SpellChecker * FV_View::getDictForSelection ()
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

	return checker;
}

fv_PropCache::fv_PropCache(void):
	m_iTick(0),
	m_iNumProps(0),
	m_pszProps(NULL)
{
}

fv_PropCache::~fv_PropCache(void)
{
	clearProps();
}

void fv_PropCache::setTick(UT_uint32 iTick)
{
	m_iTick = iTick;
}

UT_uint32 fv_PropCache::getTick(void) const
{
	return m_iTick;
}

const XML_Char ** fv_PropCache::getCopyOfProps(void) const
{
	const XML_Char ** props = static_cast<const XML_Char **>(UT_calloc(m_iNumProps+1, sizeof(XML_Char *))); 
	UT_uint32 i =0;
	const XML_Char ** p = props;
	for(i =0; i< m_iNumProps;i++)
	{
		p[i] = m_pszProps[i];
		xxx_UT_DEBUGMSG((" copy i %d m_pszProps[i] %x m_pszProps %s props %x props %s\n",i,m_pszProps[i],m_pszProps[i],props[i],props[i]));
	}
	p[m_iNumProps] = NULL;
	xxx_UT_DEBUGMSG(("getCopy: props %x m_pszProps %x \n",props,m_pszProps));
	UT_ASSERT(NULL != m_pszProps);
	return props;
}

void fv_PropCache::fillProps(UT_uint32 numProps, const XML_Char ** props)
{
	m_iNumProps = numProps;
	m_pszProps = static_cast<XML_Char **>(UT_calloc(m_iNumProps, sizeof(XML_Char *))); 
	UT_uint32 i = 0;
	xxx_UT_DEBUGMSG(("m_pszProps %x props %x ",m_pszProps,props));
	for(i =0; i< m_iNumProps && (props[i] != NULL);i++)
	{
		if(props[i] != NULL)
		{
			m_pszProps[i] = const_cast<XML_Char *>(props[i]);
			xxx_UT_DEBUGMSG((" i %d m_pszProps[i] %x m_pszProps %s \n",i,m_pszProps[i],m_pszProps[i]));
		}
		else
		{
			m_pszProps[i] = NULL;
		}
	}
	UT_ASSERT(NULL != m_pszProps);
}

bool fv_PropCache::isValid(void) const
{
	bool b = ((m_iNumProps > 0) && (NULL != m_pszProps));
	if(m_iNumProps > 0)
	{
		UT_ASSERT(NULL != m_pszProps);
	}
	xxx_UT_DEBUGMSG(("Numprops %d m_pszProps %x \n",m_iNumProps,m_pszProps));
	return b;
}	

void fv_PropCache::clearProps(void)
{
	xxx_UT_DEBUGMSG(("clearing props NumProps %d m_pszProps %x \n",m_iNumProps,m_pszProps));
	FREEP(m_pszProps);
	m_pszProps = NULL;
	m_iNumProps = 0;
	xxx_UT_DEBUGMSG(("clearing props numProps %d \n",m_iNumProps));
}


