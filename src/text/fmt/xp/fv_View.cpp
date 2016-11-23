/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (c) 2001,2002 Tomas Frydrych
 * Copyright (C) 2016 Hubert Figuière
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <glib.h>
#include "ut_locale.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "ev_Mouse.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_bytebuf.h"
#include "ut_timer.h"
#include "ie_imp_RTF.h"
#include "ie_exp_RTF.h"
#include "xav_View.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#ifdef ENABLE_SPELL
#include "fl_Squiggles.h"
#endif
#include "fl_SectionLayout.h"
#include "fl_FootnoteLayout.h"
#include "fl_AutoNum.h"
#include "fp_Page.h"
#include "fp_PageSize.h"
#include "fp_Column.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_TextRun.h"
#include "xap_Module.h"
#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "pd_Document.h"
#include "pd_DocumentRDF.h"
#include "pd_Style.h"
#include "pp_Property.h"
#include "pp_AttrProp.h"
#include "gr_Graphics.h"
#include "gr_DrawArgs.h"
#include "ie_types.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Clipboard.h"
#include "ap_Features.h"
#include "ap_TopRuler.h"
#include "ap_LeftRuler.h"
#include "ap_Prefs.h"
#include "ap_Strings.h"
#include "fd_Field.h"
#ifdef ENABLE_SPELL
#include "spell_manager.h"
#endif
#include "ut_rand.h"
#include "fp_TableContainer.h"
#include "fl_TableLayout.h"
#include "xap_Dlg_Zoom.h"
#include "ap_Frame.h"
#include "ap_FrameData.h"
#include "fp_FrameContainer.h"
#include "xap_EncodingManager.h"
#include "gr_Painter.h"
#include "xap_DialogFactory.h"
#include "ap_Preview_Annotation.h"
#include "ap_Dialog_Id.h"
#include "fv_ViewDoubleBuffering.h"

#include "pp_Revision.h"

#include "fv_View.h"

// NB -- irrespective of this size, the piecetable will store
// at max BOOKMARK_NAME_LIMIT of chars as defined in pf_Frag_Bookmark.h
#define BOOKMARK_NAME_SIZE 30
#define CHECK_WINDOW_SIZE if(getWindowHeight() < m_pG->tlu(20)) return;

// add 0.1 inch for a left border to click in for showBorder

#define FRAME_MARGIN 144

/****************************************************************/

fv_ChangeState::fv_ChangeState()
        : bUndo(false)
        , bRedo(false)
        , bDirty(false)
        , bSelection(false)
        , iColumn(0)
        , pCellLayout(nullptr)
{
}

fv_ChangeState::~fv_ChangeState()
{
}

/****************************************************************/

class ABI_EXPORT _fmtPair
{
public:
	_fmtPair(const gchar * p,
			 const PP_AttrProp * c, const PP_AttrProp * b, const PP_AttrProp * s,
			 PD_Document * pDoc, bool bExpandStyles)
		{
			m_prop = p;
			m_val  = PP_evalProperty(p,c,b,s, pDoc, bExpandStyles);
		}

	const gchar *	m_prop;
	const gchar *	m_val;
};

class ABI_EXPORT FV_Caret_Listener : public AV_Listener
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
			  pG->allCarets()->setInsertMode(pData->m_bInsertMode);
			  return true;
		  }
      }
      
	  if (
	  		// I hope I have all the signals here that makes sense for re-enabling the caret blink
	  		// after it was stopped by a cursor blink timeout timer - MARCM
		  ((mask & AV_CHG_KEYPRESSED) || 
	  		(mask & AV_CHG_TYPING) || 
		   (mask & AV_CHG_MOTION) ) && pG->allCarets()->getBaseCaret()
	     )
      {
		  pG->allCarets()->getBaseCaret()->resetBlinkTimeout();
		  return true;
      }

	  return false;
  }

  virtual AV_ListenerType    getType(void) {return AV_LISTENER_CARET;}

private:
  XAP_Frame   * m_pFrame;
};


fv_CaretProps::fv_CaretProps(FV_View * pView, PT_DocPosition InsPoint):
	m_iInsPoint(InsPoint),
	m_xPoint(0),
	m_yPoint(0),
	m_xPoint2(0),
	m_yPoint2(0),
	m_bPointDirection(false),
	m_bDefaultDirectionRtl(false),
	m_bUseHebrewContextGlyphs(false),
	m_bPointEOL(false),
	m_iPointHeight(0),
	m_caretColor(0,0,0),
	m_PropCaretListner(NULL),
	m_pCaret(NULL),
	m_ListenerID(0),
	m_pView(pView),
	m_iAuthorId(-1),
	m_sCaretID("")
{
}

fv_CaretProps::~fv_CaretProps(void)
{
	if(m_PropCaretListner != NULL)
	{
		DELETEP(m_PropCaretListner);
	}
}

/****************************************************************/
#ifdef _MSC_VER	// MSVC++ warns about using 'this' in initializer list.
#pragma warning(disable: 4355)
#endif
FV_View::FV_View(XAP_App * pApp, void* pParentData, FL_DocLayout* pLayout)
	:	AV_View(pApp, pParentData),
		m_iNumHorizPages(1), /* This should probably be grabbed from a preference or something */
		m_autoNumHorizPages(true), /* Same here */
		m_horizPageSpacing(500),
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
#ifdef EMBEDDED_TARGET
		m_viewMode(VIEW_NORMAL),
#else
		m_viewMode(VIEW_PRINT),
#endif
		m_previewMode(PREVIEW_NONE),
		m_bDontUpdateScreenOnGeneralUpdate(false),
		m_iPieceTableState(0),
		m_iMouseX(0),
		m_iMouseY(0),
		m_iViewRevision(0),
		m_bWarnedThatRestartNeeded(false),
		m_selImageRect(-1,-1,-1,-1),
		m_imageSelCursor(GR_Graphics::GR_CURSOR_IBEAM),
		m_ixResizeOrigin(0),
		m_iyResizeOrigin(0),
		m_bIsResizingImage(false),
		m_curImageSel(-1,-1,-1,-1),
#if XAP_DONTUSE_XOR
		m_curImageSelCache(NULL),
#endif
		m_bIsDraggingImage(false),
		m_pDraggedImageRun(NULL),
		m_dragImageRect(-1,-1,-1,-1),
		m_ixDragOrigin(0),
		m_iyDragOrigin(0),
		m_colorShowPara(127,127,127),
		m_colorSpellSquiggle(255, 0, 0),
		m_colorGrammarSquiggle(0, 192, 0),
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
		m_bShowRevisions(true),
		m_eBidiOrder(FV_Order_Visual),
		m_iFreePass(0),
		m_bDontNotifyListeners(false),
		m_pLocalBuf(NULL),
		m_iGrabCell(0),
		m_InlineImage(this),
		m_bInsertAtTablePending(false),
		m_iPosAtTable(0),
		m_bAnnotationPreviewActive(false),
		m_bAllowSmartQuoteReplacement(true),
		m_bubbleBlockerCount(0),
		m_iOldPageCount(-1),
		m_SelectionHandles(this, m_Selection),
		m_pViewDoubleBufferingObject(NULL)
{
	if(m_pDoc)
		m_sDocUUID = m_pDoc->getMyUUIDString();
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

	//
	m_colorAnnotations[0] = UT_RGBColor(171,4,254);
	m_colorAnnotations[1] = UT_RGBColor(171,20,119);
	m_colorAnnotations[2] = UT_RGBColor(255,151,8);
	m_colorAnnotations[3] = UT_RGBColor(158,179,69);
	m_colorAnnotations[4] = UT_RGBColor(15,179,5);
	m_colorAnnotations[5] = UT_RGBColor(8,179,248);
	m_colorAnnotations[6] = UT_RGBColor(4,206,195);
	m_colorAnnotations[7] = UT_RGBColor(4,133,195);
	m_colorAnnotations[8] = UT_RGBColor(7,18,195);
	m_colorAnnotations[9] = UT_RGBColor(255,0,0);	// catch-all

	//
	m_colorRDFAnchors[0] = UT_RGBColor(171,4,254);
	m_colorRDFAnchors[1] = UT_RGBColor(171,20,119);
	m_colorRDFAnchors[2] = UT_RGBColor(255,151,8);
	m_colorRDFAnchors[3] = UT_RGBColor(158,179,69);
	m_colorRDFAnchors[4] = UT_RGBColor(15,179,5);
	m_colorRDFAnchors[5] = UT_RGBColor(8,179,248);
	m_colorRDFAnchors[6] = UT_RGBColor(4,206,195);
	m_colorRDFAnchors[7] = UT_RGBColor(4,133,195);
	m_colorRDFAnchors[8] = UT_RGBColor(7,18,195);
	m_colorRDFAnchors[9] = UT_RGBColor(255,0,0);	// catch-all

	
	// initialize prefs cache
	pApp->getPrefsValueBool(AP_PREF_KEY_CursorBlink, &m_bCursorBlink);

   	const gchar * pszTmpColor = NULL;
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForShowPara), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorShowPara);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForSquiggle), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorSpellSquiggle);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForGrammarSquiggle), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorGrammarSquiggle);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForMargin), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorMargin);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForFieldOffset), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorFieldOffset);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForImage), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorImage);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForHyperLink), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorHyperLink);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForHdrFtr), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorHdrFtr);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForColumnLine), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorColumnLine);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision1), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[0]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision2), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[1]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision3), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[2]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision4), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[3]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision5), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[4]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision6), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[5]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision7), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[6]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision8), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[7]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision9), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[8]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision10), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRevisions[9]);
	}


	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForAnnotation1), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorAnnotations[0]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForAnnotation2), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorAnnotations[1]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForAnnotation3), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorAnnotations[2]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForAnnotation4), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorAnnotations[3]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForAnnotation5), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorAnnotations[4]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForAnnotation6), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorAnnotations[5]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForAnnotation7), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorAnnotations[6]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForAnnotation8), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorAnnotations[7]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForAnnotation9), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorAnnotations[8]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForAnnotation10), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorAnnotations[9]);
	}

	///////////////////
	///////////////////
	
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForRDFAnchor1), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRDFAnchors[0]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForRDFAnchor2), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRDFAnchors[1]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForRDFAnchor3), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRDFAnchors[2]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForRDFAnchor4), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRDFAnchors[3]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForRDFAnchor5), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRDFAnchors[4]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForRDFAnchor6), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRDFAnchors[5]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForRDFAnchor7), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRDFAnchors[6]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForRDFAnchor8), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRDFAnchors[7]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForRDFAnchor9), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRDFAnchors[8]);
	}
	if (pApp->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_ColorForRDFAnchor10), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, m_colorRDFAnchors[9]);
	}

	///////////////////
	///////////////////

	
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
		const PP_PropertyVector bidi_props = {
			"dom-dir", "rtl",
			"text-align", "right"
		};

		m_pDoc->addStyleProperties("Normal", bidi_props);
		PP_resetInitialBiDiValues("rtl");
	}
#else
	if(!m_bDefaultDirectionRtl)
	{
		const PP_PropertyVector bidi_props = {
			"dom-dir", "ltr",
			"text-align", "left"
		};

		m_pDoc->addStyleProperties("Normal", bidi_props);
		PP_resetInitialBiDiValues("ltr");
	}
#endif

	UT_UTF8String s = XAP_EncodingManager::get_instance()->getLanguageISOName();

	const char * pCountry
		= XAP_EncodingManager::get_instance()->getLanguageISOTerritory();
	
	if(pCountry)
	{
		s += "-";
		s += pCountry;
	}
		
	// do a fuzzy match for Times New Roman
	const char * pszFamily =
		GR_Graphics::findNearestFont ("Times New Roman",
									  "normal", "normal",
									  "normal", "normal",
									  "12pt", s.utf8_str());
	
	if (pszFamily)
		PP_setDefaultFontFamily(pszFamily);

	// should we display revisions?
	m_bShowRevisions = m_pDoc->isShowRevisions();
	m_iViewRevision =  m_pDoc->getShowRevisionId();
	

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
		m_pG->allCarets()->enable();
		if(m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
		{
			m_caretListener = new FV_Caret_Listener (pFrame);
			addListener(m_caretListener, &m_CaretListID);
			AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());		
			if(pFrameData && pFrameData->m_bIsWidget)
			{
				setViewMode(VIEW_NORMAL);
				//pFrame->toggleRuler(false);
					//pFrame->toggleLeftRuler(false);
			}
		}
		else
		{
			m_caretListener = NULL;
		}
	}

	// see what the document value is for bidi ordering ...
	const PP_AttrProp * pDocAP = m_pDoc->getAttrProp();
	if(pDocAP)
	{
		const gchar * szValue;
		pDocAP->getProperty("dom-dir",szValue);

		if(szValue)
		{
			if(0 == strcmp(szValue, "logical-ltr"))
			{
				m_eBidiOrder = FV_Order_Logical_LTR;
			}
			else if(0 == strcmp(szValue, "logical-rtl"))
			{
				m_eBidiOrder = FV_Order_Logical_RTL;
			}
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

	DELETEP(m_pLocalBuf);
	UT_VECTOR_PURGEALL(fv_CaretProps *,m_vecCarets);
}

bool FV_View::isActive(void) const
{
	if(!couldBeActive())
	        return false;
	const AV_View* pActiveView = NULL;
	XAP_Frame* lff = getApp()->getLastFocussedFrame();
	if(lff) 
	{
		pActiveView = lff->getCurrentView();
	}
	else 
	{
		pActiveView = this;
	}
	
	bool bAct = (pActiveView == this);
	if(!bAct)
		return false;
	UT_UTF8String sUUID =  m_pDoc->getMyUUIDString();
	if(m_sDocUUID == sUUID)
		return true;
	return false;
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
		m_pG->allCarets()->enable();
		XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
		m_caretListener = new FV_Caret_Listener (pFrame);
		addListener(m_caretListener, &m_CaretListID);
	}
	else
	{
		m_caretListener = NULL;
	}
}

bool FV_View::registerDoubleBufferingObject(FV_ViewDoubleBuffering *obj)
{
	if(m_pViewDoubleBufferingObject == NULL)
	{
		// ok, you're the top most one, I will register you
		m_pViewDoubleBufferingObject = obj;
		return true;
	}
	else return false;
}

bool FV_View::unregisterDoubleBufferingObject(FV_ViewDoubleBuffering *obj)
{
	if((void*)m_pViewDoubleBufferingObject == (void*)obj)
	{
		// you're the top most caller, I will unregister you
		m_pViewDoubleBufferingObject = NULL;
		return true;
	}
	else return false;
}

void FV_View:: fixInsertionPointCoords(void)
{
	_fixInsertionPointCoords();
}

void FV_View::updateCarets(PT_DocPosition docPos, UT_sint32 iLen)
{
	fv_CaretProps * pCaretProps = NULL;
	UT_sint32 iCount = m_vecCarets.getItemCount();
	UT_UTF8String sUUID = m_pDoc->getMyUUIDString();
	bool bLocal = (sUUID == m_sDocUUID);
	UT_sint32 i = 0;
	bool bFoundID = false;
	for(i=0; i<iCount;i++)
	{
			pCaretProps = m_vecCarets.getNthItem(i);
			pCaretProps->m_pCaret->resetBlinkTimeout();
			if((pCaretProps->m_sCaretID == sUUID) && (iLen > 0))
			{
				_setPoint(pCaretProps,docPos,iLen);
				bFoundID = true;
			}
			else if((docPos > 0) && (pCaretProps->m_iInsPoint >= docPos))
			{
				_setPoint(pCaretProps,pCaretProps->m_iInsPoint,iLen);
			}
			else if(docPos <= 0)
			{
				_setPoint(pCaretProps,pCaretProps->m_iInsPoint,iLen);
			}
	}
	if((iLen > 0) && !bFoundID && !bLocal)
	{
		UT_DEBUGMSG(("Could find a caret to match UUID! Creating one now! \n"));
		UT_sint32 iNewAuthor = m_pDoc->getLastAuthorInt();
		UT_DEBUGMSG(("Creating Caret for author %d UUID %s \n",iNewAuthor,sUUID.utf8_str()));
		addCaret(docPos, iNewAuthor);
	}
}

void FV_View::removeCaret(const std::string& sUUID)
{
	for (UT_sint32 i = 0; i < m_vecCarets.getItemCount(); i++)
	{
		fv_CaretProps* pCaretProps = m_vecCarets.getNthItem(i);
		UT_continue_if_fail(pCaretProps);

		if (pCaretProps->m_sCaretID == sUUID)
		{
			pCaretProps->m_pCaret->disable(false);
			m_pG->removeCaret(pCaretProps->m_sCaretID);
			removeListener(pCaretProps->m_ListenerID);
			DELETEP(pCaretProps);
			m_vecCarets.deleteNthItem(i);
			break;
		}
	}
}

void FV_View::addCaret(PT_DocPosition docPos,UT_sint32 iAuthorId)
{
	//
	// Don't add an extra caret for the local user
	//
	if(m_pDoc->getMyUUIDString() == m_sDocUUID)
		return;
	//
	// Check we're not adding a duplicated caret
	//
	fv_CaretProps* pCaretProps = NULL;
	UT_sint32 iCount = m_vecCarets.getItemCount();
	for (UT_sint32 i = 0; i < iCount; i++)
	{
		pCaretProps = m_vecCarets.getNthItem(i);
		if(pCaretProps->m_sCaretID == m_pDoc->getMyUUIDString())
		{
			return;
		}
	}
	pCaretProps = new fv_CaretProps(this,docPos);
	m_vecCarets.addItem(pCaretProps);
	UT_DEBUGMSG((" add caret num %d id %d position %d \n",m_vecCarets.getItemCount(),iAuthorId,docPos));	
	pCaretProps->m_sCaretID = m_pDoc->getMyUUIDString().utf8_str();
	pCaretProps->m_pCaret = m_pG->createCaret(pCaretProps->m_sCaretID );
	UT_DEBUGMSG(("m_sCaretID %s OrigDocID %s \n",pCaretProps->m_sCaretID.c_str(),m_sDocUUID.utf8_str()));
	XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
	pCaretProps->m_PropCaretListner = new FV_Caret_Listener (pFrame);
	addListener(pCaretProps->m_PropCaretListner,&pCaretProps->m_ListenerID);
	pCaretProps->m_pCaret->setBlink(true);
	pCaretProps->m_pCaret->enable();
	pCaretProps->m_iAuthorId = iAuthorId;
	UT_sint32 icnt = iAuthorId;
	pCaretProps->m_sCaretID = m_pDoc->getMyUUIDString().utf8_str();
	icnt = icnt % 12;
	if(m_pDoc->getMyAuthorInt() != iAuthorId)
	{
		pCaretProps->m_caretColor = getColorRevisions(icnt);
	}
	else
	{
		//
		// Primary author carets are black
		//
		pCaretProps->m_caretColor =  UT_RGBColor(0,0,0);
	}
	pCaretProps->m_pCaret->setRemoteColor(pCaretProps->m_caretColor);
	_setPoint(pCaretProps,docPos,0);

}

#if ENABLE_SPELL
UT_RGBColor	FV_View::getColorSquiggle(FL_SQUIGGLE_TYPE iSquiggleType) const
{
	if(iSquiggleType == FL_SQUIGGLE_SPELL)
	{
		return m_colorSpellSquiggle;
	}
	return m_colorGrammarSquiggle;
}
#endif

UT_RGBColor	FV_View::getColorAnnotation(const fp_Run * pRun) const
{
	fp_HyperlinkRun * pHRun = pRun->getHyperlink();
	fp_AnnotationRun * pARun = NULL;
	if(pHRun && pHRun->getHyperlinkType() == HYPERLINK_ANNOTATION)
	{
			pARun = static_cast<fp_AnnotationRun *>(pHRun);
	}
	else
	{
			return pRun->_getColorFG();
	}
	fp_Page * pPage = pARun->getLine()->getPage();
	xxx_UT_DEBUGMSG(("getColorAnnotation() page:%p\n", pPage ));
	if(!pPage)
			return pRun->_getColorFG();
	UT_uint32 pos = pPage->getAnnotationPos(pARun->getPID());
	if(pos > 9)
		pos = 9;
	return m_colorAnnotations[pos];
}


UT_RGBColor	FV_View::getColorAnnotation(fp_Page * pPage,UT_uint32 pid) const
{

	UT_uint32 pos = pPage->getAnnotationPos(pid);
	if(pos > 9)
		pos = 9;
	return m_colorAnnotations[pos];
}


UT_RGBColor	FV_View::getColorRDFAnchor(const fp_Run * pRun) const
{
	fp_Page* pPage = pRun->getLine()->getPage();
	xxx_UT_DEBUGMSG(("getColorRDFAnchor() page:%p\n", pPage ));
	if(!pPage)
		return pRun->_getColorFG();

	fp_HyperlinkRun * pHRun = pRun->getHyperlink();
//	fp_RDFAnchorRun * pARun = NULL;
	if(pHRun && pHRun->getHyperlinkType() == HYPERLINK_RDFANCHOR)
	{
//		pARun = static_cast<fp_RDFAnchorRun*>(pHRun);
	}
	else
	{
		return pRun->_getColorFG();
	}
//	UT_uint32 pos = pPage->getRDFAnchorPos(pARun->getPID());
	UT_uint32 pos = 1;
	if(pos > 9)
		pos = 9;
	return m_colorRDFAnchors[pos];
}



void FV_View::replaceGraphics(GR_Graphics * pG)
{
	DELETEP(m_pG);
	setGraphics(pG);
	m_pLayout->setGraphics(pG);
	XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
	if (pFrame && pFrame->getFrameData())
		reinterpret_cast<AP_FrameData*>(pFrame->getFrameData())->m_pG = pG;
	m_pLayout->rebuildFromHere(static_cast<fl_DocSectionLayout *>(m_pLayout->getFirstSection()));
}

//-------------------------
// Visual Drag stuff
//
void FV_View::cutVisualText(UT_sint32 /*x*/, UT_sint32 /*y*/)
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

void FV_View::btn0VisualDrag(UT_sint32 x, UT_sint32 y)
{
	m_xLastMouse = m_iMouseX;
	m_iMouseX = x;
	m_yLastMouse = m_iMouseY;
	m_iMouseY = y;
	setCursorToContext();
}

//
// Local copy stuff. This pastes from the local paste buffer into the document
//
void FV_View::_pasteFromLocalTo(PT_DocPosition pos)
{
	UT_return_if_fail(m_pLocalBuf);
	PD_DocumentRange docRange(m_pDoc, pos,pos);
	IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(m_pDoc);
	const unsigned char * pData = static_cast<const unsigned char *>(m_pLocalBuf->getPointer(0));
	UT_uint32 iLen = m_pLocalBuf->getLength();

	pImpRTF->pasteFromBuffer(&docRange,pData,iLen);

	delete pImpRTF;
}

void FV_View::pasteFromLocalTo(PT_DocPosition pos)
{
	UT_return_if_fail(m_pLocalBuf);
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();
	m_pDoc->disableListUpdates();
	m_pDoc->beginUserAtomicGlob();
	m_pDoc->setDoingPaste();
	setCursorWait();
	m_pDoc->setDontImmediatelyLayout(true);

	_pasteFromLocalTo(pos);
	clearCursorWait();
	m_pDoc->clearDoingPaste();
	m_pDoc->setDontImmediatelyLayout(false);

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();



	// Signal piceTable is stable again
	_restorePieceTableState();
	_generalUpdate();
	m_pDoc->endUserAtomicGlob();
	// Move insertion point out of field run if it is in one
	//
	_charMotion(true, 0);
	_fixInsertionPointCoords();
	if (isSelectionEmpty())
	{
		_ensureInsertionPointOnScreen();
	}
	notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR);
}

const UT_ByteBuf * FV_View::getLocalBuf(void) const
{
	return m_pLocalBuf;
}
//
// This copies a range into a local buffer.
//
void FV_View::copyToLocal(PT_DocPosition pos1, PT_DocPosition pos2)
{
	DELETEP(m_pLocalBuf);
	m_pLocalBuf = new UT_ByteBuf;
	IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(m_pDoc);
	PD_DocumentRange docRange(m_pDoc, pos1,pos2);

	pExpRtf->copyToBuffer(&docRange,m_pLocalBuf);
	delete pExpRtf;
}

//Creates a new document, inserts a string into it, selects all, and then copies it
//onto the system clipboard

void FV_View::copyTextToClipboard(const UT_UCS4String sIncoming, bool /*useClipboard*/)
{
	/* create a new hidden document */
  	PD_Document * pDoc = new PD_Document();
  	pDoc->newDocument();
	FL_DocLayout * pDocLayout = new FL_DocLayout(pDoc, m_pG);
	FV_View * pCopyLinkView = new FV_View(XAP_App::getApp(), 0, pDocLayout);
	
	/* assign the view to the doclayout */
	pDocLayout->setView(pCopyLinkView);
	
	/* fill its layout structures (which are quite empty, but still...) */
	pCopyLinkView->getLayout()->fillLayouts();
	pCopyLinkView->getLayout()->formatAll();
	
	/* insert the string in the new document, select it, and copy it */
	pCopyLinkView->cmdCharInsert(sIncoming.ucs4_str(), sIncoming.length(),false);
	pCopyLinkView->cmdSelect(0,0,FV_DOCPOS_BOD,FV_DOCPOS_EOD);
	pCopyLinkView->cmdCopy();

	/* we're done, release our resources */
	DELETEP(pCopyLinkView);
	DELETEP(pDocLayout);
	UNREFP(pDoc);
}

/*!
 * Logic for determining what state the Image and cursor should be in.
 */
void FV_View::btn0InlineImage(UT_sint32 x, UT_sint32 y)
{
	xxx_UT_DEBUGMSG(("btn0 called InlineImage mode %d \n",m_InlineImage.getInlineDragMode()));
	m_InlineImage.setDragType(x,y,false);
	setCursorToContext();
}

/*!
 * Deal with a left -mouse click on a inline-image. It might be the 
 * start of a drag or a resize.
 */
void FV_View::btn1InlineImage(UT_sint32 x, UT_sint32 y)
{
	m_InlineImage.mouseLeftPress(x,y);
}


/*!
 * Complete the drag which either finishes the drag of the image or
 * completes the resize of the image.
 */
void FV_View::releaseInlineImage(UT_sint32 x, UT_sint32 y)
{
	m_InlineImage.mouseRelease(x,y);
}



/*!
 * Drag on an image. Either drag the whole image or do a resize.
 */
void FV_View::dragInlineImage(UT_sint32 x, UT_sint32 y)
{
	m_InlineImage.mouseDrag(x,y);
}


/*!
 * Make a copy of the inline image. This gets called with cntrl-left mouse
 * click.
 */
void FV_View::btn1CopyImage(UT_sint32 x, UT_sint32 y)
{
	m_InlineImage.mouseCopy(x,y);
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

void FV_View::setFrameFormat(const PP_PropertyVector & properties)
{
	std::string dataID;
	setFrameFormat(properties, NULL, dataID);
}

/*!
 * This method converts a positioned object described by fp_FrameLayout * pFram
 * to an inline image. It then selects the inline image.
 */
bool FV_View::convertPositionedToInLine(fl_FrameLayout * pFrame)
{
	UT_GenericVector<fl_BlockLayout *> vecBlocks;
	fp_FrameContainer * pFC = static_cast<fp_FrameContainer *>(pFrame->getFirstContainer());
	pFC->getBlocksAroundFrame(vecBlocks);
	if(vecBlocks.getItemCount() == 0)
	{
		fp_Page * pPage = pFC->getPage();
		fp_Column * pCol = pPage->getNthColumnLeader(0);
		fp_Container * pCon = pCol->getFirstContainer();
		fl_BlockLayout * pB = NULL;
		if(pCon->getContainerType() == FP_CONTAINER_LINE)
		{
			pB = static_cast<fp_Line *>(pCon)->getBlock();
		}
		else
		{
			fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pCon->getSectionLayout());
			pB = pCL->getNextBlockInDocument();
		}
		vecBlocks.addItem(pB);
	}
	UT_sint32 iBlk = 0;
	fl_BlockLayout * pBL = vecBlocks.getNthItem(iBlk);
	fp_Line * pLine = static_cast<fp_Line *>(pBL->getFirstContainer());
	bool bLoop = true;
	while((pLine != NULL) && bLoop)
	{
			UT_sint32 xoffLine, yoffLine;
			fp_VerticalContainer * pVCon= (static_cast<fp_VerticalContainer *>(pLine->getContainer()));
			pVCon->getScreenOffsets(pLine, xoffLine, yoffLine);
			if(yoffLine + pLine->getHeight() >= pFC->getFullY())
			{
				bLoop = false;
				break;
			}
			pLine = static_cast<fp_Line *>(pLine->getNext());
			if(pLine == NULL)
			{
				iBlk++;
				if(iBlk < vecBlocks.getItemCount())
				{
					pBL = vecBlocks.getNthItem(iBlk);
					pLine = static_cast<fp_Line *>(pBL->getFirstContainer());
				}
			}
	}
	if(pLine == NULL)
	{
		pBL = vecBlocks.getNthItem(vecBlocks.getItemCount()-1);
		pLine = static_cast<fp_Line *>(pBL->getLastContainer());
		if(pLine == NULL)
			return false;
	}
	fp_Run * pRun = pLine->getLastRun();
	PT_DocPosition pos = pBL->getPosition() + pRun->getBlockOffset() + pRun->getLength();
	const PP_AttrProp* pAP = NULL;
	pFrame->getAP(pAP);
	if(pAP == NULL)
	{
		return false;
	}
	const gchar* szDataID = 0;
	const gchar* szTitle = 0;
	const gchar* szDescription = 0;
	const  gchar* szWidth = 0;
	const  gchar * szHeight = 0;
    bool bFound = pAP->getAttribute(PT_STRUX_IMAGE_DATAID,szDataID);
	if(!bFound)
	{
		return false;
	}
	bFound = pAP->getProperty("frame-width",szWidth);
	if(!bFound)
	{
		return false;
	}
	bFound = pAP->getProperty("frame-height",szHeight);
	if(!bFound)
	{
		return false;
	}
	bFound = pAP->getAttribute("title",szTitle);
	bFound = pAP->getAttribute("alt",szDescription);
	std::string sProps = "width:";
	sProps += szWidth;
	sProps += "; height:";
	sProps += szHeight;

	if(szTitle ==  0)
		szTitle = "";
	if(szDescription == 0)
		szDescription = "";

	const PP_PropertyVector attributes = {
		PT_IMAGE_DATAID, szDataID,
		PT_IMAGE_TITLE, szTitle,
		PT_IMAGE_DESCRIPTION, szDescription,
		PT_PROPS_ATTRIBUTE_NAME, sProps
	};
	if(pFrame->getPosition(true) < pos)
	{
		pos -= 2;
	}
	PT_DocPosition posEnd = 0;
	getEditableBounds(true,posEnd);
	while(!isPointLegal(pos) && pos <= posEnd)
	{
		pos++;
	}
	bool bMakeItLegal = false;
	if(pos > posEnd)
	{
		bMakeItLegal = true;
	}
	m_pDoc->beginUserAtomicGlob();
	m_FrameEdit.deleteFrame(pFrame);
	_saveAndNotifyPieceTableChange();
	if(bMakeItLegal)
	{
		setPoint(pos);
		pos = getPoint();
	}
	m_pDoc->insertObject(pos, PTO_Image, attributes, PP_NOPROPS);
	_restorePieceTableState();
	m_pDoc->endUserAtomicGlob();
	_updateInsertionPoint();
	_generalUpdate();
	cmdSelect(pos,pos+1);
	return true;
}

/*!
 * This method converts an image located position pos to a positioned object.
 */
void FV_View::convertInLineToPositioned(PT_DocPosition pos, const PP_PropertyVector & attributes)
{

	fl_BlockLayout * pBlock = getBlockAtPosition(pos);
	fp_Run *  pRun = NULL;
	bool bEOL,bDir;
	bEOL = false;
	UT_sint32 x1,y1,x2,y2,iHeight;
	if(pBlock)
	{
		pRun = pBlock->findPointCoords(pos,bEOL,x1,y1,x2,y2,iHeight,bDir);
		while(pRun && pRun->getType() != FPRUN_IMAGE)
		{
			pRun = pRun->getNextRun();
		}
		if(pRun && pRun->getType() == FPRUN_IMAGE)
		{
			UT_DEBUGMSG(("SEVIOR: Image run on pos \n"));
		}
		else
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return;
		}
	}
	//
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();
	m_pDoc->beginUserAtomicGlob();
	_deleteSelection();
	pf_Frag_Strux * pfFrame = NULL;
//
// This should place the the frame strux immediately after the block containing
// position posXY.
// It returns the Frag_Strux of the new frame.
//
	fl_BlockLayout * pBL = pBlock;
	if((pBL == NULL) || (pRun == NULL))
	{
	  return;
	}
	fl_BlockLayout * pPrevBL = pBL;
	while(pBL && ((pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_ENDNOTE) || (pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_FOOTNOTE) || (pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_ANNOTATION) || (pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_TOC)|| (pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_FRAME)))
	{
		UT_DEBUGMSG(("Skipping Block %p \n",pBL));
		pPrevBL = pBL;
		pBL = pBL->getPrevBlockInDocument();
	}
	if(pBL == NULL)
	{
	        pBL = pPrevBL;
	}
	UT_ASSERT((pBL->myContainingLayout()->getContainerType() != FL_CONTAINER_HDRFTR) 
		  && (pBL->myContainingLayout()->getContainerType() != FL_CONTAINER_SHADOW));
	pos = pBL->getPosition();
	m_pDoc->insertStrux(pos, PTX_SectionFrame, attributes, PP_NOPROPS, &pfFrame);
	PT_DocPosition posFrame = pfFrame->getPos();
	m_pDoc->insertStrux(posFrame+1,PTX_EndFrame);
	insertParaBreakIfNeededAtPos(posFrame+2);

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	m_pDoc->endUserAtomicGlob();
	_generalUpdate();
	setPoint(posFrame+2);
	if(!isPointLegal())
	{
		setPoint(posFrame);
	}
	_ensureInsertionPointOnScreen();
	notifyListeners(AV_CHG_MOTION | AV_CHG_ALL);
}

void FV_View::setFrameFormat(const PP_PropertyVector & properties, const FG_ConstGraphicPtr & pFG,
							 const std::string & sDataID, fl_BlockLayout * pNewBL)
{
	setCursorWait();
	//
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	fl_FrameLayout * pFrame = getFrameLayout();
	if(pFrame == NULL)
	{
		UT_DEBUGMSG(("No frame selected. Aborting! \n"));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

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
		const PP_PropertyVector attributes = {
			PT_STRUX_IMAGE_DATAID, ""
		};
		UT_DebugOnly<bool> bRet = m_pDoc->changeStruxFmt(PTC_RemoveFmt, posStart,
														 posStart, attributes, PP_NOPROPS, PTX_SectionFrame);
		UT_ASSERT(bRet);
	}

	if(pNewBL && (pFrame->getParentContainer() != pNewBL))
	{
		UT_DEBUGMSG(("BEGIN RELOCATION\n"));
		getLayout()->relocateFrame(pFrame, pNewBL, PP_NOPROPS, properties);
	}
	else
	{
		UT_DEBUGMSG(("BEGIN CHANGE\n"));
		//posStart = pFrame->getPosition(true)+1;
		//PT_DocPosition posEnd = posStart;
		UT_DebugOnly<bool> bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, posStart, posEnd, PP_NOPROPS,
														 properties, PTX_SectionFrame);
		UT_ASSERT(bRet);
	}

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	_generalUpdate();

	_ensureInsertionPointOnScreen();
	clearCursorWait();
	notifyListeners(AV_CHG_MOTION);
}

bool FV_View::getFrameStrings_view(UT_sint32 x, UT_sint32 y,fv_FrameStrings & FrameStrings,
								   fl_BlockLayout ** pCloseBL,fp_Page ** ppPage)
{
	return (m_FrameEdit.getFrameStrings(x,y,FrameStrings,pCloseBL,ppPage));
}



void FV_View::setFrameFormat(const PP_PropertyVector & attribs, const PP_PropertyVector & properties, fl_BlockLayout *pNewBL)
{
	setCursorWait();
	//
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();
	if(isHdrFtrEdit())
	{
		clearHdrFtrEdit();
		warpInsPtToXY(0,0,false);
	}
	fl_FrameLayout * pFrame = getFrameLayout();
	if(pFrame == NULL)
	{
		UT_DEBUGMSG(("No frame selected. Aborting! \n"));
		// should we assert ?
		return;
	}

	if(pNewBL && (pFrame->getParentContainer() != pNewBL))
	{
		getLayout()->relocateFrame(pFrame, pNewBL, attribs, properties);
	}
	else
	{
		PT_DocPosition posStart = pFrame->getPosition(true)+1;
		PT_DocPosition posEnd = posStart;
		UT_DebugOnly<bool> bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, posStart, posEnd,
														 attribs, properties, PTX_SectionFrame);
		UT_ASSERT(bRet);
	}
	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	_generalUpdate();

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

void FV_View::activateFrame(void)
{
	if(!m_FrameEdit.isActive())
	{
		m_FrameEdit.mouseLeftPress(m_iMouseX,m_iMouseY);
	}
}

void FV_View::copyFrame(bool b_keepFrame)
{
	if(!m_FrameEdit.isActive())
	{
		m_FrameEdit.mouseLeftPress(m_iMouseX,m_iMouseY);
	}
	fl_FrameLayout * pFL = getFrameLayout();
	if(pFL == NULL)
	{
		m_FrameEdit.setMode(FV_FrameEdit_NOT_ACTIVE);
		XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
		if(pFrame)
		{
			EV_Mouse * pMouse = pFrame->getMouse();
			if(pMouse)
			{
				pMouse->clearMouseContext();
			}
		}
		m_prevMouseContext = EV_EMC_TEXT;
		setCursorToContext();
		return;
	}
	PT_DocPosition posLow = pFL->getPosition(true);
	PT_DocPosition posHigh = posLow + pFL->getLength();
	PD_DocumentRange dr(m_pDoc,posLow,posHigh);
	XAP_App::getApp()->copyToClipboard(&dr, true);
	if (!b_keepFrame)
	{
		m_FrameEdit.deleteFrame();
	}
	notifyListeners(AV_CHG_CLIPBOARD);
}

void FV_View::selectFrame(void)
{
	_clearSelection();
	if(!m_FrameEdit.isActive())
	{
		m_FrameEdit.mouseLeftPress(m_iMouseX,m_iMouseY);
	}
	fl_FrameLayout * pFL = getFrameLayout();
	if(pFL == NULL)
	{
		m_FrameEdit.setMode(FV_FrameEdit_NOT_ACTIVE);
		XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
		if(pFrame)
		{
			EV_Mouse * pMouse = pFrame->getMouse();
			if(pMouse)
			{
				pMouse->clearMouseContext();
			}
		}
		m_prevMouseContext = EV_EMC_TEXT;
		setCursorToContext();
		return;
	}
	PT_DocPosition posLow = pFL->getPosition(true)+2;
	PT_DocPosition posHigh = pFL->getPosition(true) + pFL->getLength()-1;
	PD_DocumentRange dr(m_pDoc,posLow,posHigh);
	setPoint(posLow);
	_setSelectionAnchor();
	setPoint(posHigh);
	_drawSelection();
}

bool FV_View::isFrameSelected(void) const
{
	return(m_FrameEdit.isActive());
}

void FV_View::deleteFrame(void)
{
	if(!m_FrameEdit.isActive())
	{
		m_FrameEdit.mouseLeftPress(m_iMouseX,m_iMouseY);
	}
	if(getFrameLayout() == NULL)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		selectFrame(); // this will clear the frame context
		return;
	}
	UT_DEBUGMSG(("Doing Delete Frame \n"));
	m_FrameEdit.deleteFrame();
	XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
	if(pFrame)
	{
		EV_Mouse * pMouse = pFrame->getMouse();
		if(pMouse)
		{
			pMouse->clearMouseContext();
		}
	}
	m_prevMouseContext = EV_EMC_TEXT;
	setCursorToContext();
}

fl_FrameLayout * FV_View::getFrameLayout(void) const
{
        if(m_FrameEdit.isActive())
	{
	       return m_FrameEdit.getFrameLayout();
	}
	return getFrameLayout(getPoint());
}

fl_FrameLayout * FV_View::getFrameLayout(PT_DocPosition pos) const
{
	if(m_pDoc->isFrameAtPos(pos))
	{
		fl_ContainerLayout* psfh = NULL;
		m_pDoc->getStruxOfTypeFromPosition(getLayout()->getLID(),pos+1,
										   PTX_SectionFrame, &psfh);
		fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(psfh);
		UT_ASSERT(pFL->getContainerType() == FL_CONTAINER_FRAME);
		return pFL;
	}
	if(m_pDoc->isFrameAtPos(pos-1))
	{
		fl_ContainerLayout* psfh = NULL;
		m_pDoc->getStruxOfTypeFromPosition(getLayout()->getLID(),
											 pos,
											 PTX_SectionFrame, &psfh);
		fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(psfh);
		UT_ASSERT(pFL->getContainerType() == FL_CONTAINER_FRAME);
		return pFL;
	}

	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);
	
	if(pBlock)
	{
		fl_ContainerLayout * pCL = pBlock->myContainingLayout();
		while(pCL && (pCL->getContainerType() != FL_CONTAINER_FRAME) && (pCL->getContainerType() != FL_CONTAINER_DOCSECTION))
		{
			if(pCL == pCL->myContainingLayout())
				break;
			pCL = pCL->myContainingLayout();
		}
		if(pCL && pCL->getContainerType() == FL_CONTAINER_FRAME)
		{
			return static_cast<fl_FrameLayout *>(pCL);
		}
		if((pBlock->getPosition(true) < pos) && (pBlock->getPosition(true) + pBlock->getLength() + 1 < pos))
		{
			pBlock = pBlock->getNextBlockInDocument();
		}
		if(pBlock == NULL)
		{
			return NULL;
		}
		if((pBlock->getPosition(true) < pos) && (pBlock->getPosition(true) + pBlock->getLength() +1 < pos))
		{
			return NULL;
		}
		pCL = pBlock->myContainingLayout();
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
bool FV_View::isInFrame(PT_DocPosition pos) const
{
//
// If at exactly the frame return true
//
	if(m_pDoc->isFrameAtPos(pos))
	{
		return true;
	}
	if(m_pDoc->isFrameAtPos(pos-1) && !m_pDoc->isEndFrameAtPos(pos))
	{
		return true;
	}
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

/*!
 * Returns true if the suppiled position is not is a sepecial structure
 * like a frame or table or whatever.
 *
 * pos defaults to 0. Ifpos == 0, I assume you actually what the values of
 * getPoint()
 */

bool FV_View::isInDocSection(PT_DocPosition pos) const
{
	if(pos == 0)
	{
		pos = getPoint();
	}
	fl_BlockLayout * pBL = _findBlockAtPosition(pos);
	if(pBL && (pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_DOCSECTION))
	{
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
    const gchar * pszTmpColor = NULL;
    if (XAP_App::getApp()->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForSelBackground), &pszTmpColor))
      {
	UT_parseColor(pszTmpColor, bgcolor);
      }
    m_bgColorInitted = true;
  }
  
  return bgcolor;
}

UT_RGBColor FV_View::getColorSelForeground () const
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

	if(low < 2)
	{
		// the initial block strux / section strux is also selected, for example when
		// using ctrl+a
		low = 2;
	}
	
	// if this is an empty document, gracefully return
	if(low == high)
		return;

	UT_DEBUGMSG(("fv_View::toggleCase: low %d, high %d\n", low, high));

	fl_BlockLayout * pBL =	m_pLayout->findBlockAtPosition(low);
	fp_Run * pRun;
	if(low < pBL->getPosition())
	{
		low = pBL->getPosition();
	}
//
// skip blank lines
//
	while(pBL && (low == pBL->getPosition(true) + pBL->getLength()))
	{
		pBL = pBL->getNextBlockInDocument();
		if(pBL != NULL)
		{
			low = pBL->getPosition();
		}
	}
	if(pBL == NULL)
	{
		return;
	}
	// if this is an empty document, gracefully return
	if(low >= high)
		return;

	// create a temp buffer of a reasonable size (will g_try_realloc if too small)
	UT_sint32	 iTempLen = 150;
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

		if(offset == buffer.getLength())
		{
			// this is a special case when the selection starts at the very begining of a
			// block which is not the first block in the document -- the block that
			// findBlockAtPosition() returned to us is actually the block _before_ the one
			// we are interested in
			pBL = pBL->getNextBlockInDocument();
			UT_return_if_fail( pBL );
			buffer.truncate(0);
			pBL->getBlockBuf(&buffer);

			// move past the block strux
			++low;
			offset = 0;
		}
		
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
		if(pBL == NULL)
		{
			return;
		}
		if(pBL->getContainerType() != FL_CONTAINER_BLOCK)
		{
			pBL = static_cast<fl_ContainerLayout *>(pBL)->getNextBlockInDocument();
		}

		const PP_AttrProp * pSpanAPAfter = NULL;
		pBL->getSpanAP(offset,false,pSpanAPAfter);
		PP_AttrProp * pSpanAPNow = const_cast<PP_AttrProp *>(pSpanAPAfter);

		xxx_UT_DEBUGMSG(("fv_View::toggleCase: pBL 0x%x, offset %d, pSpanAPAfter 0x%x\n", pBL, offset, pSpanAPAfter));

		pRun = pBL->findRunAtOffset(offset);

		xxx_UT_DEBUGMSG(("fv_View::toggleCase: block offset %d, low %d, high %d, lastPos %%d\n", offset, low, high/*, lastPos*/));

		bool bBlockDone = false;
		while(!bBlockDone && (low < high) /*&& (low < lastPos)*/)
		{
			UT_sint32 iLenToCopy = UT_MIN(high - low, buffer.getLength() - offset);

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
					iLenToCopy -= pRun->getLength();
					pRun = pRun->getNextRun();
				}

				if(pRun && pRun->getBlockOffset() > offset)
				{
					// we skipped over an embeded section
					iLenToCopy -= pRun->getBlockOffset() - offset;
					
					offset = pRun->getBlockOffset();
					low = pBL->getPosition(false) + offset;
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

					if(pRun->getBlockOffset() > offset + iLen)
					{
						// we skipped over an embeded section
						// i.e., there is a discontinuity in the block buffer
						break;
					}
					
					UT_sint32 iDiff = UT_MIN((UT_sint32)pRun->getLength(), iLenToCopy);
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
				pBL->getSpanAP(offset+iLen,false,pSpanAPAfter);
				xxx_UT_DEBUGMSG(("fv_View::toggleCase: delete/insert: low %d, iLen %d, pSpanAPAfter 0x%x, pSpanAPNow 0x%x\n", low, iLen,pSpanAPAfter,pSpanAPNow));

#if 0
				UT_DEBUGMSG(("AP Now\n"));
				pSpanAPNow->miniDump(getDocument());
				UT_DEBUGMSG(("---------------------------------- \nAP After\n"));
				pSpanAPAfter->miniDump(getDocument());
#endif
				UT_uint32 iRealDeleteCount;
				m_pDoc->tellPTDoNotTweakPosition(true); // stop surrounding hyperlinks,
														// bookmarks, etc. getting deleted
				bool bResult = m_pDoc->deleteSpan(low, low + iLen,NULL,iRealDeleteCount);
				m_pDoc->tellPTDoNotTweakPosition(false);
				UT_ASSERT(bResult);

				//special handling is required for delete in revisions mode
				//where we have to move the insertion point
				if(isMarkRevisions())
				{
					UT_ASSERT( iRealDeleteCount <= iLen );
					_charMotion(true,iLen - iRealDeleteCount);
				}
				bResult = m_pDoc->insertSpan(low, pTemp, iLen, pSpanAPNow);
				UT_ASSERT_HARMLESS( bResult );

				if(!pSpanAPNow->getAttributes().empty() || !pSpanAPNow->getProperties().empty())
				{
					bResult &= m_pDoc->changeSpanFmt(PTC_SetFmt,low,low+iLen,
													 pSpanAPNow->getAttributes(),pSpanAPNow->getProperties());
				}
				
				// now remember the props for the next round
				pSpanAPNow = const_cast<PP_AttrProp*>(pSpanAPAfter);

				UT_ASSERT_HARMLESS(bResult);
				low += iLen;
				offset += iLen;

				// the piecetable operations might have invalidated the pRun pointer,
				// need to get it afresh

				pRun = pBL->findRunAtOffset(offset);
				if(!pRun && iLenToCopy && offset > 0)
				{
					// see if no run is not because of an embeded section
					// get a pointer to the run that holds the last
					// character we just changed, and from that get the next run
					pRun = pBL->findRunAtOffset(offset-1);
					if(pRun)
					{
						pRun = pRun->getNextRun();
					}

					if(pRun)
					{
						// so there are still runs in this block; readjust the offsets
						iLenToCopy -= (pRun->getBlockOffset() - offset);
						offset = pRun->getBlockOffset();
						low = offset + pBL->getPosition(false);
					}
				}
			}
		}
		pBL = pBL->getNextBlockInDocument();
		if ( pBL )
		  low = pBL->getPosition(false);
		else
		  break;
	}

	delete[] pTemp;
	_restorePieceTableState();
	_generalUpdate();
	m_pDoc->endUserAtomicGlob();
	if(origPos)
		_setPoint(origPos);
}

/*!
 * This method appends all the text in the current Block to the supplied
 * Growbuf.
 */
void FV_View::getTextInCurrentBlock(UT_GrowBuf & buf) const
{
	fl_BlockLayout * pBlock = getCurrentBlock();
	pBlock->appendTextToBuf(buf);
}


/*!
 * This method appends all the text in the current DocSection to the supplied
 * Growbuf.
 */
void FV_View:: getTextInCurrentSection(UT_GrowBuf & buf) const
{
	fl_BlockLayout * pBlock = getCurrentBlock();
	fl_DocSectionLayout * pDSL = pBlock->getDocSectionLayout();
	pDSL->appendTextToBuf(buf);
}


/*!
 * This method appends all the text in the Document to the supplied
 * Growbuf.
 */
void FV_View:: getTextInDocument(UT_GrowBuf & buf) const
{
	fl_ContainerLayout * pDSL = static_cast<fl_ContainerLayout *>(m_pLayout->getFirstSection());
	while(pDSL)
	{
		pDSL->appendTextToBuf(buf);
		pDSL = pDSL->getNext();
	}
}

/*!
 * This method returns the bounds of a line, given an offset.
 */
	bool FV_View::getLineBounds(PT_DocPosition pos, PT_DocPosition *start, PT_DocPosition *end)
{
	fl_BlockLayout *pBlock = NULL;
	fp_Run *pRun = NULL;
	UT_sint32 x, y;
	UT_uint32 height;
	UT_sint32 x2, y2;
	bool bDirection;
	_findPositionCoords(pos, FALSE, x, y, x2, y2, height, bDirection, &pBlock, &pRun);
	if (!pRun) return FALSE;
fp_Line *line = pRun->getLine();
	PT_DocPosition blockpos = pBlock->getPosition();
	if (start)
	{
		*start = blockpos + line->getFirstRun()->getBlockOffset();
	}
	if (end)
	{
		fp_Run *lastrun = line->getLastRun();
		*end = blockpos + lastrun->getBlockOffset() + lastrun->getLength();
	}
	return TRUE;
}

UT_UCSChar FV_View::getChar(PT_DocPosition pos, UT_sint32 *x, UT_sint32 *y, UT_uint32 *width, UT_uint32 *height)
{
  if (x || y || height)
  {
  UT_sint32 fp_x, fp_y;
  UT_uint32 fp_height;
  UT_sint32 x2, y2;
  bool bDirection;
    _findPositionCoords(pos, FALSE, fp_x, fp_y, x2, y2, fp_height, bDirection, NULL, NULL);
    if (x) *x = fp_x;
    if (y) *y = fp_y;
    if (height) *height = fp_height;
  }

  pt_PieceTable *piece = getDocument()->getPieceTable();
  pf_Frag *p;
  PT_BlockOffset offset;
  UT_UCSChar ret = 0;
  if (piece->getFragFromPosition(pos, &p, &offset))
  {
    if (p->getType() == 0) // PFT_Text)
    {
      pf_Frag_Text *pt = reinterpret_cast<pf_Frag_Text *>(p);
      PT_BufIndex bi = pt->getBufIndex();
      const UT_UCSChar *c = piece->getPointer(bi);
      ret = c[offset];
    }
  }
  if (ret && width) *width = getGraphics()->measureUnRemappedChar(ret);
  return ret;
}

/*!
 * Goes through the document and reformats any paragraphs that need this.
 */
void FV_View::updateLayout(void)
{
	m_pLayout->updateLayout();
}

void FV_View::setPaperColor(const gchar* clr)
{
	UT_DEBUGMSG(("DOM: color is: %s\n", clr));

	const PP_PropertyVector props = {
		"background-color", clr
	};
	setSectionFormat(props);
	// update the screen
	_draw(0, 0, getWindowWidth(), getWindowHeight(), false, false);
}

void FV_View::killBlink(void)
{
	m_pG->allCarets()->setBlink(false);
}

void FV_View::focusChange(AV_Focus focus)
{
	m_focus=focus;
	xxx_UT_DEBUGMSG(("fv_View:: Focus change focus = %d selection %d \n",focus,isSelectionEmpty()));
	switch(focus)
	{
	case AV_FOCUS_HERE:
		if(getPoint() > 0 && isSelectionEmpty())
		{
		  if(m_FrameEdit.getFrameEditMode() != FV_FrameEdit_WAIT_FOR_FIRST_CLICK_INSERT)
		  {
			m_pG->allCarets()->enable();
		  }
		  else
		  {
		        break;
		  }
		}
		if (isSelectionEmpty() && (getPoint() > 0))
		{
			m_pG->allCarets()->setBlink(m_bCursorBlink);
			_setPoint(getPoint());
		}
		m_pApp->rememberFocussedFrame(m_pParentData);
		_updateSelectionHandles();
		break;
	case AV_FOCUS_NEARBY:
		if (isSelectionEmpty() && (getPoint() > 0))
		{
			m_pG->allCarets()->disable(true);
			m_countDisable++;
		}
		break;
	case AV_FOCUS_MODELESS:
		if (isSelectionEmpty() && (getPoint() > 0))
		{
			m_pG->allCarets()->setBlink(false);
			_setPoint(getPoint());
		}
		break;
	case AV_FOCUS_NONE:
		m_SelectionHandles.hide();
		if (isSelectionEmpty() && (getPoint() > 0))
		{
			m_pG->allCarets()->disable(true);
			m_countDisable++;
		}
		break;
	}
	AV_View::notifyListeners(AV_CHG_FOCUS);
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
	if(isPreview()|| m_bDontNotifyListeners)
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

//
// Since we short circuit some operations we need to give some operations
// a "freepass"
//
	if(mask & m_iFreePass)
	{
		m_iFreePass = 0;
		return AV_View::notifyListeners(mask);
	}

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
		PP_PropertyVector propsBlock;
		getBlockFormat(propsBlock);

		bool bMatch = false;

		if (!propsBlock.empty() && !m_chg.propsBlock.empty()
			&& (propsBlock.size() == m_chg.propsBlock.size()))
		{
			bMatch = true;

			std::size_t i = 0;

			while (bMatch && i < propsBlock.size())
			{
				if (propsBlock[i] == m_chg.propsBlock[i])
				{
					bMatch = false;
					break;
				}

				i++;
			}
		}

		if (!bMatch)
		{
			m_chg.propsBlock = std::move(propsBlock);
		}
		else
		{
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
		PP_PropertyVector propsChar;
		getCharFormat(propsChar);

		bool bMatch = false;

		if (!propsChar.empty() && !m_chg.propsChar.empty()
			&& (propsChar.size() == m_chg.propsChar.size()))
		{
			bMatch = true;

			std::size_t i = 0;

			while (bMatch && i < propsChar.size())
			{
				if (propsChar[i] != m_chg.propsChar[i])
				{
					bMatch = false;
					break;
				}

				i++;
			}
		}

		if (!bMatch)
		{
			m_chg.propsChar = std::move(propsChar);
		}
		else
		{
			mask ^= AV_CHG_FMTCHAR;
		}
	}

	if (mask & AV_CHG_FMTSECTION)
	{
		/*
		  The following brute-force solution works, but is atrociously
		  expensive, so we should avoid using it whenever feasible.
		*/
		PP_PropertyVector propsSection;
		getSectionFormat(propsSection);

		bool bMatch = false;

		if (!propsSection.empty() && !m_chg.propsSection.empty()
			&& propsSection.size() == propsSection.size())
		{
			bMatch = true;

			std::size_t i = 0;

			while (bMatch && i < propsSection.size())
			{
				if (propsSection[i] != m_chg.propsSection[i])
				{
					bMatch = false;
					break;
				}

				i++;
			}
		}

		if (!bMatch)
		{
			m_chg.propsSection = std::move(propsSection);
		}
		else
		{
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
		// We keep track of the number of page in the document in order
		// to recalculate the vertical scrollbar only when needed
		// (this prevents bug #13355 (comment #17))

		UT_sint32 iNbPages = getLayout()->countPages();
		if (m_iOldPageCount == iNbPages)
		{
			mask ^= AV_CHG_PAGECOUNT;
		}
		else
		{
			m_iOldPageCount = iNbPages;
		}
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
			if(pContainer == NULL)
			{
				pBlock->needsReformat();
				return false;
			}
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
		else if(pContainer == NULL)
		{
			return false;
		}
	}

	if (mask & AV_CHG_CELL)
	{
		fl_BlockLayout * pBlock = _findBlockAtPosition(getPoint());
		fl_ContainerLayout * pCL = pBlock->myContainingLayout();
		if (pCL && (pCL->getContainerType() == FL_CONTAINER_CELL))
		{
			if (pCL != m_chg.pCellLayout)
			{
				m_chg.pCellLayout = static_cast<fl_CellLayout*>(pCL);
			}
			else
			{
				mask ^= AV_CHG_CELL;
			}
		}
		else if (m_chg.pCellLayout)
		{
			// Caret just moved out of a table
			m_chg.pCellLayout = NULL;
		}
		else
		{
			mask ^= AV_CHG_CELL;
		}
	}

	if (mask & AV_CHG_WINDOWSIZE)
		m_pG->allCarets()->setWindowSize(getWindowWidth(), getWindowHeight());

	// base class does the rest
	xxx_UT_DEBUGMSG(("FV_View: notifyListeners: this %x \n",this));
	return AV_View::notifyListeners(mask);
}


PT_DocPosition FV_View::saveSelectedImage (const char * toFile)
{
  UT_ConstByteBufPtr pBytes;

  PT_DocPosition dPos = saveSelectedImage(pBytes);

  if ( pBytes )
	{
	  pBytes->writeToURI ( toFile ) ;
	}

  return dPos ;
}

PT_DocPosition FV_View::mapDocPos( FV_DocPos dp ) {
	return ( _getDocPos( dp ));
	}

PT_DocPosition FV_View::mapDocPosSimple( FV_DocPos dp ) {
	return ( _getDocPos( dp, false ));
	}

PT_DocPosition FV_View::saveSelectedImage(UT_ConstByteBufPtr & pBytes)
{
	const char * dataId;
	PT_DocPosition pos = 0;
	if(m_prevMouseContext == EV_EMC_POSOBJECT)
	{
		fl_FrameLayout * pFrame = getFrameLayout();
		const PP_AttrProp* pAP = NULL;
		UT_return_val_if_fail(pFrame, 0);
		pFrame->getAP(pAP);
		if(pAP == NULL)
		{
			return 0;
		}
		pAP->getAttribute(PT_STRUX_IMAGE_DATAID, dataId);
		pos = pFrame->getPosition();
	}
	else
	{
		pos = getSelectedImage(&dataId);

	// if nothing selected or selection not an image
		if (pos == 0) return 0;
	}
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

PT_DocPosition FV_View::getSelectedImage(const char **dataId) const
{
	const fp_Run * pRun = NULL;
	return getSelectedImage(dataId,&pRun);
}

PT_DocPosition FV_View::getSelectedImage(const char **dataId, const fp_Run **pImRun) const
{
	// if nothing selected, then an image can't be
	if (!isSelectionEmpty())
	{
		PT_DocPosition pos = m_Selection.getSelectionAnchor();
		fp_Run* pRun = NULL;

		UT_GenericVector<fl_BlockLayout *> vBlock;
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
				pBlock = vBlock.getNthItem(i);
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
					*dataId = static_cast<fp_ImageRun *>(pRun)->getDataId();
				}
				*pImRun = pRun;
				return pos;
			}
		}
	}

	// if we made it here, then run type is not an image
	if (dataId != NULL) {
		*dataId = NULL;
	}
	pImRun = NULL;
	
	return 0;
}

/* If no object is selected returns NULL
 * Otherwise returns a nonzero value indicating the position of the object
 * and if dataId is not NULL will set value to the object's data ID
 */
fp_Run *FV_View::getSelectedObject() const
{
	// if nothing selected, then an image can't be
	if (!isSelectionEmpty())
	{
		PT_DocPosition pos = m_Selection.getSelectionAnchor();
		fp_Run* pRun = NULL;

		UT_GenericVector<fl_BlockLayout *> vBlock;
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
				pBlock = vBlock.getNthItem(i);
				pRun = pBlock->getFirstRun();
			}

			while(pRun && pRun->getType() != FPRUN_EMBED)
			{
				pRun = pRun->getNextRun();
			}
			if(pRun && pRun->getType() == FPRUN_EMBED)
			{
				return pRun;
			}
		}
	}

	return NULL;
}

PT_DocPosition FV_View::getSelectionAnchor(void) const
{
	if(m_Selection.isSelected())
	{
		return m_Selection.getSelectionAnchor();
	}
	return m_iInsPoint;
}

PT_DocPosition	FV_View::getSelectionLeftAnchor(void) const
{
	return m_Selection.getSelectionLeftAnchor();
}

PT_DocPosition	FV_View::getSelectionRightAnchor(void) const
{
	return m_Selection.getSelectionRightAnchor();
}

/*!
 * Returns true if a TOC is selected.
 */
bool FV_View::isTOCSelected(void) const
{
	return (m_Selection.getSelectionMode() == 	FV_SelectionMode_TOC);
}

/*!
 * This method assumes that pos points to exactly the location of 
 * PTX_SectionTOC. It should only really be called if the TOC is selected.
 */
bool FV_View::setTOCProps(PT_DocPosition pos, const char * szProps)
{
	bool bRet;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();
	const PP_PropertyVector atts = {
		"props", szProps
	};
	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, pos, pos, atts, PP_NOPROPS, PTX_SectionTOC);

	// Signal piceTable is stable again
	_restorePieceTableState();
	_generalUpdate();
	return bRet;
}

bool FV_View::isSelectionEmpty(void) const
{
	if(m_FrameEdit.isActive() && m_FrameEdit.isImageWrapper() )
	{
	        return false;
	}
	if(m_FrameEdit.isActive() && (m_FrameEdit. getFrameEditMode() >= FV_FrameEdit_RESIZE_INSERT))
	{
	        return false;
	}
	if (!m_Selection.isSelected())
	{
		return true;
	}
	if((m_Selection.getSelectionMode() != FV_SelectionMode_Single) &&
	   (m_Selection.getSelectionMode() != FV_SelectionMode_NONE))
	{
		if((m_Selection.getSelectionMode() ==  FV_SelectionMode_TableRow) &&
		   (getPoint() == getSelectionAnchor()) && (m_Selection.getSelectionLeftAnchor() ==
		   m_Selection.getSelectionLeftAnchor()))
		{
			return true;
		}
		return false;
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
	if((dp == FV_DOCPOS_EOD) && m_pDoc->isHdrFtrAtPos(iPos) &&
	   m_pDoc->isEndFrameAtPos(iPos-1))
	{
	     iPos--;
	     while(!isPointLegal(iPos))
	     {    
	          iPos--;
	     }
	}
	else if((dp == FV_DOCPOS_EOD) && m_pDoc->isEndFrameAtPos(iPos))
	{
	     iPos--;
	     while(!isPointLegal(iPos))
	     {    
	          iPos--;
	     }
	}
	if (iPos != getPoint())
	{
		bool bPointIsValid = (getPoint() >= _getDocPos(FV_DOCPOS_BOD));
		if (bPointIsValid)
			_clearIfAtFmtMark(getPoint());
	}

	_setPoint(iPos, (dp == FV_DOCPOS_EOL));
	_makePointLegal();
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
	_makePointLegal();
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


UT_uint32 FV_View::getCurrentPageNumber(void) const
{
	UT_sint32 iPageNum = 0;
	PT_DocPosition pos = getPoint();
	fl_BlockLayout * pBlock;
	UT_sint32 xPoint, yPoint, xPoint2, yPoint2;
	UT_uint32 iPointHeight;
	bool bDirection;
	fp_Run* pRun;
	_findPositionCoords(pos, m_bPointEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection,&pBlock, &pRun);
	if(!pRun)
	{
		return 1;
	}
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
	notifyListeners(AV_CHG_ALL);
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



bool FV_View::isCurrentListBlockEmpty(void) const
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
	UT_uint32 iTab = 0;
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
			else if(runtype == FPRUN_TAB)
			{
				iTab++;
				if(iTab > 1)
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

bool FV_View::isPointBeforeListLabel(void) const
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
bool FV_View::isNumberedHeadingHere(fl_BlockLayout * pBlock) const
{
	bool bHasNumberedHeading = false;
	if(pBlock == NULL)
	{
		return bHasNumberedHeading;
	}
	const PP_AttrProp * pBlockAP = NULL;
	pBlock->getAP(pBlockAP);
	const gchar* pszCurStyle = NULL;
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

	UT_GenericVector<fl_BlockLayout *> vBlock;
	getBlocksInSelection( &vBlock);

	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = m_Selection.getSelectionAnchor();
	if(posEnd < posStart)
	{
		PT_DocPosition swap = posStart;
		posStart = posEnd;
		posEnd = swap;
	}
	UT_sint32 diff  =0;
	bool bNoSelection = true;
//
// Turn off cursor
//
	if(!isSelectionEmpty())
	{
		bNoSelection = false;
		_clearSelection();
	}
	UT_sint32 i;
	m_pDoc->disableListUpdates();

	m_pDoc->beginUserAtomicGlob();

	char margin_left [] = "margin-left";
	char margin_right[] = "margin-right";
	UT_GenericVector<fl_BlockLayout *> vListBlocks;
	UT_GenericVector<fl_BlockLayout *> vNoListBlocks;

	for(i=0; i< vBlock.getItemCount(); i++)
	{
		fl_BlockLayout * pBlock =  vBlock.getNthItem(i);
		if(pBlock->isListItem())
		{
			vListBlocks.addItem(pBlock);
			diff -= 2;
		}
		else
		{
			vNoListBlocks.addItem(pBlock);
			diff += 2;
		}
	}
//
// Have to stop lists in reverse order so undo works!
//
	for(i = vListBlocks.getItemCount() -1; i>=0; i--)
	{
		UT_DEBUGMSG(("SEVIOR: Processing block %d \n",i));
		fl_BlockLayout * pBlock =  vListBlocks.getNthItem(i);
		PT_DocPosition posBlock = pBlock->getPosition();

		const PP_PropertyVector pListAttrs = {
			"listid", "",
			"parentid", "",
			"level", "",
		};

		// we also need to explicitely clear the list formating
		// properties, since their values are not necessarily part
		// of the style definition, so that cloneWithEliminationIfEqual
		// which we call later will not get rid off them
		const PP_PropertyVector pListProps = {
			"start-value", "",
			"list-style", "",
			(pBlock->getDominantDirection() == UT_BIDI_RTL) ? "margin-right" : "margin-left", ""
			"text-indent", "",
			"field-color", "",
			"list-delim", "",
			"field-font", "",
			"list-decimal", ""
			"list-tag", ""
		};
//
// Remove all the list related properties
//
		UT_DebugOnly<bool> bRet = m_pDoc->changeStruxFmt(PTC_RemoveFmt, posBlock, 
														 posBlock, pListAttrs, pListProps, PTX_Block);
		UT_ASSERT(bRet);
		fp_Run * pRun = pBlock->getFirstRun();
		while(pRun->getNextRun())
		{
			pRun = pRun->getNextRun();
		}
		PT_DocPosition lastPos = posBlock + pRun->getBlockOffset();
		bRet = m_pDoc->changeSpanFmt(PTC_RemoveFmt, posBlock, lastPos, pListAttrs, pListProps);
		UT_ASSERT(bRet);
	}
//
// Have to start lists in order so undo works!
//
	for(i=0; i<vNoListBlocks.getItemCount(); i++)
	{
		UT_DEBUGMSG(("Doing Block %d of %d \n",i,vNoListBlocks.getItemCount()));
		fl_BlockLayout * pBlock = vNoListBlocks.getNthItem(i);
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
			prevLeft = pPrev->getDominantDirection() == UT_BIDI_LTR
				? UT_convertToInches(pPrev->getProperty(margin_left,true))
				: UT_convertToInches(pPrev->getProperty(margin_right,true));
			
			blockLeft = pBlock->getDominantDirection() == UT_BIDI_LTR
				? UT_convertToInches(pBlock->getProperty(margin_left,true))
				: UT_convertToInches(pBlock->getProperty(margin_right,true));
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
			gchar* cType = pBlock->getListStyleString(listType);
			pBlock->StartList(cType);
		}
	}

	// closes bug # 1255 - unselect a list after creation
	//cmdUnselectSelection();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();



	// Signal piceTable is stable again
	_restorePieceTableState();
	_generalUpdate();
	m_pDoc->endUserAtomicGlob();
	if(!bNoSelection)
	{
		posEnd += diff;
		setPoint(posStart);
		_setSelectionAnchor();
		setPoint(posEnd);
		_drawSelection();
	}
	_fixInsertionPointCoords();
	if (isSelectionEmpty())
	{
		_ensureInsertionPointOnScreen();
	}
	notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR);
	UT_DEBUGMSG(("Point %d anchor %d \n",getPoint(),m_Selection.getSelectionAnchor()));
}

/*!
 * This returns the number of distinct columns in a selection. If part of the 
 * the selection is outside a table, it returns 0.
 */
UT_sint32 FV_View::getNumColumnsInSelection(void) const
{
	UT_GenericVector<fl_BlockLayout *> vecBlocks;
	getBlocksInSelection(&vecBlocks);
	UT_sint32 i =0;
	UT_sint32 iNumCols = 0;
	UT_sint32 iCurCol = -1;
	fl_BlockLayout * pBlock = NULL;
	fl_CellLayout * pCell = NULL;
	fp_CellContainer * pCellCon = NULL;
	for(i=0; i< vecBlocks.getItemCount();i++)
	{
		pBlock = vecBlocks.getNthItem(i);
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
UT_sint32 FV_View::getNumRowsInSelection(void) const
{
	UT_GenericVector<fl_BlockLayout *> vecBlocks;
	getBlocksInSelection(&vecBlocks);
	UT_sint32 i =0;
	UT_sint32 iNumRows = 0;
	UT_sint32 iCurRow = -1;
	fl_BlockLayout * pBlock = NULL;
	fl_CellLayout * pCell = NULL;
	fp_CellContainer * pCellCon = NULL;
	PT_DocPosition startpos = getPoint();
	PT_DocPosition endpos = startpos;
	if(!isSelectionEmpty())
	{
		if (m_Selection.getSelectionAnchor() > startpos)
		{
			endpos = m_Selection.getSelectionAnchor();
		}
		else
		{
			startpos = m_Selection.getSelectionAnchor();
		}
	}
	for(i=0; i< vecBlocks.getItemCount();i++)
	{
		pBlock = vecBlocks.getNthItem(i);
		if((getNumSelections() == 0) && ((pBlock->getPosition() + pBlock->getLength() - 1) <= startpos))
		{
			if((startpos == endpos) && ((pBlock->getPosition()  <= startpos)))
			{
				pCell = static_cast<fl_CellLayout *>(pBlock->myContainingLayout());
				pCellCon = static_cast<fp_CellContainer *>(pCell->getFirstContainer());
				if(pCellCon == NULL)
				{
						return 0;
				}
				return 1;

			}
			else
			{
				continue;
			}
		}
		if(pBlock->getPosition() > endpos)
		{
				break;
		}
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

/*
 * Create a vector of all blocks within the selection.
 * If bAllBlocks is false, the last block is not included if the selection stops right at the beginning
 * of the block.
 */

void FV_View::getBlocksInSelection( UT_GenericVector<fl_BlockLayout*>* vBlock, bool bAllBlocks) const
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
	bool bStop = false;
//
// tweak the start point of the selection if it is just before the current block
// strux. Clicking in the left margin moves the start point here but usually the
// user wants block after this.
//
	UT_sint32 iNumSelections = getNumSelections();
	UT_sint32 iSel =0;
	if(iNumSelections > 0)
	{
		PD_DocumentRange * pRange = getNthSelection(iSel);
		startpos = pRange->m_pos1;
		endpos = pRange->m_pos2;
		iNumSelections--;
	}
	while(!bStop)
	{
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
		while( pBlock != NULL && pBlock->getPosition(true) <= endpos)
		{
			if(pBlock->getContainerType()== FL_CONTAINER_BLOCK)
			{
				if (bAllBlocks || pBlock->getPosition(true) < endpos - 1)
				{
					vBlock->addItem(pBlock);
				}
			}
			pBlock = static_cast<fl_BlockLayout *>(pBlock->getNextBlockInDocument());
		}
		if(iNumSelections == 0)
		{
			bStop = true;
		}
		else
		{
			iNumSelections--;
			iSel++;
			PD_DocumentRange * pRange = getNthSelection(iSel);
			startpos = pRange->m_pos1;
			endpos = pRange->m_pos2;
		}
	}
	return;
}

void FV_View::insertParagraphBreak(void)
{
	STD_DOUBLE_BUFFERING_FOR_THIS_FUNCTION

	bool bBefore = false;
	bool bStopList = false;

	m_pDoc->beginUserAtomicGlob();

	// Prevent access to Piecetable for things like spellchecks until
	// paragraphs have stablized
	//
	_saveAndNotifyPieceTableChange();

	if (!isSelectionEmpty() && !m_FrameEdit.isActive())
	{
		//	m_pDoc->beginUserAtomicGlob();
		_deleteSelection();
	}
	else if(m_FrameEdit.isActive())
	{
	       m_FrameEdit.setPointInside();
	}
	if(m_bInsertAtTablePending)
	{
		m_pDoc->disableListUpdates();
		PT_DocPosition pos =  m_iPosAtTable;
		m_pDoc->insertStrux( m_iPosAtTable,PTX_Block);
		m_bInsertAtTablePending = false;
	// Signal piceTable is stable again
		_restorePieceTableState();

	// Signal piceTable is stable again
	// Signal PieceTable Changes have finished
		_generalUpdate();
	// restore updates and clean up dirty lists
		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();
		setPoint(pos+1);
		m_iPosAtTable = 0;
		_generalUpdate();
		m_pDoc->endUserAtomicGlob();
		return;
	}
	// insert a new paragraph with the same attributes/properties
	// as the previous (or none if the first paragraph in the section).
	//
	// But first check to see if we're in a list and the current block is
	// otherwise blank.
	//
	m_pDoc->disableListUpdates();
	fl_BlockLayout * pBlock = getCurrentBlock();
	pf_Frag_Strux* sdh = pBlock->getStruxDocHandle();
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

	const gchar* style = NULL;
	PD_Style* pStyle = NULL;
	if(getStyle(&style) && bAtEnd)
	{
		m_pDoc->getStyle(static_cast<const char*>(style), &pStyle);
		if(pStyle != NULL  && !bBefore)
		{
			const gchar* szFollow = NULL;
			pStyle->getAttribute("followedby",szFollow);
			if(szFollow && strcmp(szFollow,"Current Settings")!=0)
			{
				if(pStyle->getFollowedBy())
					pStyle = pStyle->getFollowedBy();

				const gchar* szValue = NULL;
//
// The name of the style is stored in the PT_NAME_ATTRIBUTE_NAME attribute within the
// style
//
				pStyle->getAttribute(PT_NAME_ATTRIBUTE_NAME, szValue);

				UT_ASSERT((szValue));
				getEditableBounds(true, posEOD);

				if ((getPoint() <= posEOD) && (strcmp(static_cast<const char *>(szValue), static_cast<const char *>(style)) != 0))
				{
					setStyle(szValue,true);
//
// Stop a List if "followed-by" is not a list style
//
					const gchar * szListType = NULL;
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


	// Signal piceTable is stable again
	_restorePieceTableState();

	// Signal piceTable is stable again
	// Signal PieceTable Changes have finished

	_generalUpdate();
	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();
	_generalUpdate();

	m_pDoc->endUserAtomicGlob();
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();
	notifyListeners(AV_CHG_MOTION | AV_CHG_ALL);
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

	_generalUpdate();

	if (bDidGlob)
		m_pDoc->endUserAtomicGlob();
	_ensureInsertionPointOnScreen();
}

bool FV_View::appendStyle(const PP_PropertyVector & style)
{
	return m_pDoc->appendStyle(style);
}

bool FV_View::setStyle(const gchar * style, bool bDontGeneralUpdate)
{
	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;
	return setStyleAtPos(style, posStart, posEnd, bDontGeneralUpdate);
}

bool FV_View::setStyleAtPos(const gchar * style, PT_DocPosition posStart1, PT_DocPosition posEnd1, bool bDontGeneralUpdate)
{
	bool bRet;

	PT_DocPosition posStart = posStart1;
	PT_DocPosition posEnd = posEnd1;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	// Turn off list updates
	m_pDoc->disableListUpdates();
//
// FIXME Handle table columns here
//
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
	const gchar * pszStyle = NULL;
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
	if(pBL == NULL)
	{
		m_pDoc->enableListUpdates();
		UT_DEBUGMSG(("restoring PieceTable state (3)\n"));
		_restorePieceTableState();
		return false;
	}
	if(bisListStyle)
	{
		if(pBL->isHdrFtr())
		{
			m_pDoc->enableListUpdates();
			UT_DEBUGMSG(("restoring PieceTable state (4)\n"));
			_restorePieceTableState();
			return false;
		}
		fl_ContainerLayout * pCL = pBL->myContainingLayout();
		while(pCL && (pCL->getContainerType() != FL_CONTAINER_DOCSECTION))
		{
			if((pCL->getContainerType() == FL_CONTAINER_HDRFTR) ||
			   (pCL->getContainerType() == FL_CONTAINER_SHADOW) ||
			   (pCL->getContainerType() == FL_CONTAINER_FOOTNOTE) ||
			   (pCL->getContainerType() == FL_CONTAINER_ANNOTATION) ||
			   (pCL->getContainerType() == FL_CONTAINER_ENDNOTE))
			{
				m_pDoc->enableListUpdates();
				UT_DEBUGMSG(("restoring PieceTable state (5)\n"));
				_restorePieceTableState();
				return false;
			}
			pCL = pCL->myContainingLayout();
		}
		if(pCL == NULL)
		{
			return false;
		}
	}
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
	UT_GenericVector<fl_BlockLayout *> vBlock;
	getBlocksInSelection( &vBlock);
	setScreenUpdateOnGeneralUpdate( false);
	bool bCharStyle = pStyle->isCharStyle();
//
// Need this to adjust the start and end positions of the style change.
//
	PT_DocPosition curPos=0;
	PT_DocPosition posNewStart = posStart;
	PT_DocPosition posNewEnd = posEnd;
	if(bisListStyle && !bCharStyle)
	{
//
// Stop the Lists contained in the current selection.
//
//
// Adjust region of style for the deletion of the Field/Tab required for each list
// element.
//
		UT_sint32 i;

		for(i=0; i< vBlock.getItemCount(); i++)
		{
			pBL = vBlock.getNthItem(i);
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
			else
			{
				if(curPos < posStart)
				{
					posNewStart += 2;
				}
				if(curPos < posEnd)
				{
					posNewEnd += 2;
				}
			}
			while(pBL->isListItem())
			{
				m_pDoc->StopList(pBL->getStruxDocHandle());
			}
		}
	}
	else if(!bCharStyle)
	{
		UT_sint32 i;

		for(i=0; i< vBlock.getItemCount(); i++)
		{
			pBL = vBlock.getNthItem(i);
			curPos = pBL->getPosition();
			if(pBL->isListItem())
			{
				if(curPos < posStart)
				{
					posNewStart -=2;
				}
				if(curPos < posEnd)
				{
					posNewEnd -= 2;
				}
			}
		}
	}
	const PP_PropertyVector attribs = {
		PT_STYLE_ATTRIBUTE_NAME, style
	};

	bool bHasNumberedHeading = false;
	const gchar * pszCurStyle = style;
	PD_Style * pCurStyle = pStyle;
	UT_uint32 depth = 0;
	if (bCharStyle)
	{
		_clearIfAtFmtMark(getPoint());	// TODO is this correct ?
		_eraseSelection();
		UT_DEBUGMSG(("Applying Character style: start %d, end %d\n", posStart, posEnd));
		bRet = m_pDoc->changeSpanFmt(PTC_AddStyle, posStart, posEnd, attribs, PP_NOPROPS);
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
		bRet = m_pDoc->changeStruxFmt(PTC_AddStyle, posStart, posEnd, attribs, PP_NOPROPS, PTX_Block);
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
		UT_sint32 i;
		for(i=0; i< vBlock.getItemCount(); i++)
		{
			pBL = vBlock.getNthItem(i);
			curPos = pBL->getPosition();
			if(i == 0)
				pBL->StartList(style);
			else
			{
				fl_BlockLayout * pPrevBL = pBL->getPrevBlockInDocument();
				if(pPrevBL)
				{
					pBL->resumeList(pPrevBL);

				}
			}
		}
	}
//
// Code to handle Numbered Heading styles...
//
	else if(bHasNumberedHeading)
	{
		fl_BlockLayout*  currBlock = vBlock.getNthItem(0);
		PT_DocPosition pos = currBlock->getPosition(true) -1;
		pf_Frag_Strux* curSdh = currBlock->getStruxDocHandle();
		if(pos < 2 )
			pos = 2;
//
// First look to see if this numbered heading style is embedded in a previous
// Numbered Heading piece of text.
//
		bool bAttach = false;
		pf_Frag_Strux* prevSDH = m_pDoc->getPrevNumberedHeadingStyle(curSdh);
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
					UT_sint32 count = m_pDoc->getListsCount();
					UT_sint32 i =0;
					const fl_AutoNum * pAuto = NULL;
					bool bFoundPrevList = false;
					for(i=0; (i< count) && !bFoundPrevList; i++)
					{
						pAuto = m_pDoc->getNthList(i);
						bFoundPrevList = (pAuto->getParentItem() == prevSDH);
					}
					if(bFoundPrevList && pAuto->getFirstItem())
					{
						pf_Frag_Strux* subSDH = pAuto->getFirstItem();
						fl_BlockLayout * pSubBlock = getBlockFromSDH(subSDH);
						UT_ASSERT(pSubBlock);
						if(pSubBlock == NULL)
							goto finish_up;

						for(i=0; i< vBlock.getItemCount(); i++)
						{
							pBL = vBlock.getNthItem(i);
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
							pBL = vBlock.getNthItem(i);
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
			pf_Frag_Strux* sdh = m_pDoc->findPreviousStyleStrux(style, pos);
			bFoundPrevHeadingBackwards = (sdh != NULL);
//
// If not, Look forward to see if there one ahead of this block
//
			if(!bFoundPrevHeadingBackwards || (curSdh == sdh))
			{
//
// Start looking from the block following and skip through to end of Doc.
//
				fl_BlockLayout * pNext = vBlock.getLastItem();
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
				for(UT_sint32 i=0; i< vBlock.getItemCount(); i++)
				{
					pBL = vBlock.getNthItem(i);
					if(i == 0)
						pBL->StartList(style);
					else
					{
						fl_ContainerLayout * pPrevL = pBL->getPrev();
						if(pPrevL && pPrevL->getContainerType() == FL_CONTAINER_BLOCK)
						{
							pBL->resumeList(static_cast<fl_BlockLayout *>(pPrevL));
						}
					}
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

				for(UT_sint32 j = 0; j < vBlock.getItemCount(); ++j)
				{
					pBL = vBlock.getNthItem(j);
					if(j == 0)
						pBL->resumeList(pBlock);
					else if(pBL->getPrev())
					{
						pBL->resumeList(static_cast<fl_BlockLayout *>(pBL->getPrev()));
					}
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
				for(UT_sint32 j = 0; j < vBlock.getItemCount(); j++)
				{
					pBL = vBlock.getNthItem(j);
					if (j == 0)
						pBL->prependList(pBlock);
					else if(pBL->getPrev())
					{
						pBL->resumeList(static_cast<fl_BlockLayout *>(pBL->getPrev()));
					}
				}
			}
		}
	}
 finish_up:
	setScreenUpdateOnGeneralUpdate( true);
	// Signal piceTable is stable again
	UT_DEBUGMSG(("restoring PieceTable state GeneralUpdate %d \n",bDontGeneralUpdate));
	_restorePieceTableState();
	if(!bDontGeneralUpdate)
	{
		_generalUpdate();
		// restore updates and clean up dirty lists
		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();
	}
	UT_DEBUGMSG(("SEVIOR: posNewStart %d posNewEnd %d at end \n",posStart,posEnd));

	m_pDoc->endUserAtomicGlob();
	if (posEnd != posStart)
	{
		_clearSelection();
		_setPoint(posNewStart);
		_setSelectionAnchor();
		_setPoint(posNewEnd);
		_drawSelection();
	}
	return bRet;
}

/*!
 * This method finds the appropiate matching block in the current view
 * for the StruxDocHandle (actually a pointer to a pf_frag_strux)
 */
fl_BlockLayout * FV_View::getBlockFromSDH(pf_Frag_Strux* sdh)
{
	fl_ContainerLayout* sfh = NULL;
	fl_BlockLayout * pBlock = NULL;
//
// Cast it into a fl_BlockLayout and we're done!
//
	sfh = m_pDoc->getNthFmtHandle(sdh, m_pLayout->getLID());
	if(sfh != NULL)
	{
		pBlock = static_cast<fl_BlockLayout *>(sfh);
		if(pBlock->getDocLayout() != m_pLayout)
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			pBlock = NULL;
		}
	}
	return pBlock;
}


static const gchar * x_getStyle(const PP_AttrProp * pAP, bool bBlock)
{
	UT_return_val_if_fail(pAP, NULL);
	const gchar* sz = NULL;

	pAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, sz);

	// TODO: should we have an explicit default for char styles?
	if (!sz && bBlock)
	{
		sz = "None";
	}
	return sz;
}

bool FV_View::getStyle(const gchar ** style) const
{
	bool bCharStyle = false;
	const gchar * szChar = NULL;
	const gchar * szBlock = NULL;

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
	fl_BlockLayout* pBlock2 = _findBlockAtPosition(posStart);
	if(pBlock2 == NULL)
	{
		return false;
	}
	pBlock2->getAP(pBlockAP);

	szBlock = x_getStyle(pBlockAP, true);

	// 2. prune if block style varies across selection
	if (!bSelEmpty)
	{
		fl_BlockLayout* pBlockEnd = _findBlockAtPosition(posEnd);

		while (pBlock2 && (pBlock2 != pBlockEnd))
		{
			const PP_AttrProp * pAP;
			bool bCheck = false;

			pBlock2 = pBlock2->getNextBlockInDocument();

			if (!pBlock2)
			{
				// at EOD, so just bail
				break;
			}

			// did block format change?
			pBlock2->getAP(pAP);
			if (pBlockAP != pAP)
			{
				pBlockAP = pAP;
				bCheck = true;
			}

			if (bCheck)
			{
				const gchar* sz = x_getStyle(pBlockAP, true);

				if (strcmp(sz, szBlock))
				{
					// doesn't match, so stop looking
					szBlock = NULL;
					pBlock2 = NULL;
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
		if(pBlock == NULL)
		{
			return false;
		}
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

		pBlock->getSpanAP( (posStart - blockPosition), bLeftSide, pSpanAP);

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
				pBlock->getSpanAP(pRun->getBlockOffset()+pRun->getLength(),true,pAP);
				if (pAP && (pSpanAP != pAP))
				{
					pSpanAP = pAP;
					bCheck = true;
				}

				if (bCheck)
				{
					const gchar* sz = x_getStyle(pSpanAP, true);
					bool bHere = (sz && sz[0]);

					if ((bCharStyle != bHere) || ((sz && szChar && strcmp(sz, szChar))))
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

bool FV_View::setCharFormat(const PP_PropertyVector & properties, const PP_PropertyVector & attribs)
{
	bool bRet = false;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if(1 < getNumSelections())
		{

			m_pDoc->beginUserAtomicGlob();
			UT_sint32 i = 0;
			PD_DocumentRange * pRange = NULL;
			for(i=0; i < getNumSelections(); i++)
			{
				pRange = getNthSelection(i); // don't delete!
				posStart = pRange->m_pos1;
				posEnd = pRange->m_pos2;
				while(!isPointLegal(posStart))
				{
					posStart++;
				}
				while(!isPointLegal(posEnd) && (posEnd > posStart) )
				{
					posEnd--;
				}
				posEnd++;
				if(posEnd < posStart)
				{
					posEnd= posStart;
				}
				bRet = m_pDoc->changeSpanFmt(PTC_AddFmt, posStart, posEnd, attribs, properties);
			}

			// Signal piceTable is stable again
			_restorePieceTableState();
			_generalUpdate();

			m_pDoc->endUserAtomicGlob();
			return bRet;
		}
		if (m_Selection.getSelectionAnchor() < posStart)
		{
			posStart = m_Selection.getSelectionAnchor();
		}
		else
		{
			posEnd = m_Selection.getSelectionAnchor();
		}
		if(m_pDoc->isEndFootnoteAtPos(posEnd))
		{
			posEnd++;
		}
	}

	m_pDoc->beginUserAtomicGlob();
	if(m_bInsertAtTablePending)
	{
		PT_DocPosition pos = m_iPosAtTable;
		m_pDoc->insertStrux( m_iPosAtTable,PTX_Block);
		m_bInsertAtTablePending = false;
		posStart = pos+1;
		posEnd = posStart;
		m_iPosAtTable = 0;
	}
	if((posStart == posEnd) && !isPointLegal(posStart))
	{
		_makePointLegal();
		posStart = getPoint();
		posEnd = posStart;
	}
	// Here the selection used to be cleared, but that prevented users
	// from making multiple changes to the same region.

	bRet = m_pDoc->changeSpanFmt(PTC_AddFmt,posStart,posEnd,attribs,properties);

	// Signal pieceTable is stable again
	_restorePieceTableState();
	_generalUpdate();

	m_pDoc->endUserAtomicGlob();
	_fixInsertionPointCoords();

	return bRet;
}

/*!
    clears props and attribs
    bAll specifies that everything is to go; if !bAll some selected props will be preserved
 
    Notes: This function is to be called in response to reset fmt command
    (ctrl+space). The idea is this: we use props and attributes to store things that are
    not strictly speaking text fmt. For example, spellchecker language is a property, but
    not strictly text fmt; when pressing ctrl+space, more often than not the user wants to
    get rid of bold, italics, etc., but not of the langauge markup. So we exercise a
    degree of discretion.
*/
bool FV_View::resetCharFormat(bool bAll)
{
	PP_AttrProp AP;

	if(!bAll)
	{
		const PP_AttrProp * pAP = getAttrPropForPoint();

		if(pAP)
		{
			UT_uint32 i = 0;
			const gchar * szName, *szValue;
			while(pAP->getNthProperty(i++,szName,szValue))
			{
				// add other props as required ...
				if(!strcmp(szName,"lang"))
				   AP.setProperty(szName,szValue);
			}
			
		}
	}
	
	m_pDoc->beginUserAtomicGlob();

	// first we reset everything ...
	// we have to do this, because setCharFormat() calls are cumulative (it uses
	// PTC_AddFmt)
	const PP_PropertyVector attrs_out = {
		"props", "",
		"style", ""
	};
	bool bRet = setCharFormat(PP_NOPROPS, attrs_out);

	// now if we have something to set, we do so ...
	if(AP.hasAttributes() || AP.hasProperties())
		bRet &= setCharFormat(AP.getProperties(), AP.getAttributes());
	
	m_pDoc->endUserAtomicGlob();
	return bRet;
}

const PP_AttrProp * FV_View::getAttrPropForPoint() const
{
	const fl_BlockLayout * pBL = getCurrentBlock();
	UT_return_val_if_fail( pBL, NULL);

	UT_uint32 blockOffset = getPoint() - pBL->getPosition();
	fp_Run * pRun = pBL->findRunAtOffset(blockOffset);

	UT_return_val_if_fail( pRun, NULL );
	bool bLeftSide = true;

	if(/*(pRun->getType() == FPRUN_TEXT || pRun->getType() == FPRUN_ENDOFPARAGRAPH) &&*/
	   pRun->getBlockOffset() == blockOffset &&
	   pRun->getPrevRun()
	   && pRun->getPrevRun()->getType() == FPRUN_TEXT)
	{
		// between two text frags, use the one on the left
		pRun = pRun->getPrevRun();

		blockOffset = pRun->getBlockOffset();
		bLeftSide = false;
	}
	
	const PP_AttrProp * pAP = NULL;
	getDocument()->getSpanAttrProp(pBL->getStruxDocHandle(), blockOffset, bLeftSide, &pAP);
#if 0
	if(pAP) pAP->miniDump(getDocument());
#endif
	return pAP;
}


bool FV_View::getAttributes(const PP_AttrProp ** ppSpanAP, const PP_AttrProp ** ppBlockAP, PT_DocPosition posStart) const
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
		pBlock->getSpanAP( (posStart - blockPosition), bLeftSide, *ppSpanAP);
	if (ppBlockAP)
		pBlock->getAP(*ppBlockAP);

	return true;
}

/* Get complete(?) set of properties for current position, or for the start of current
 * selection.
 */
bool FV_View::getAllAttrProp(const PP_AttrProp *& pSpanAP, const PP_AttrProp *& pBlockAP, const PP_AttrProp *& pSectionAP, const PP_AttrProp *& pDocAP) const
{
	pDocAP     = m_pDoc->getAttrProp();
	pSectionAP = 0;
	pBlockAP   = 0;
	pSpanAP    = 0;

	if(getLayout()->getFirstSection() == NULL)
	{
		UT_DEBUGMSG(("FV_View::getAllAttrProp: no first section!\n"));
		return false;
	}

	PT_DocPosition posStart = getPoint();

	if (!isSelectionEmpty())
	{
		if (m_Selection.getSelectionAnchor() < posStart)
			posStart = m_Selection.getSelectionAnchor();
	}
	if (posStart < 2)
		posStart = 2;

	if (fl_BlockLayout * pBlock = getBlockAtPosition(posStart))
	{
		bool bLeftSide = true; // looking to the left of the cursor, I think... does this work with bidi? [TODO: ??]

		pBlock->getAP(pBlockAP);

		if (fl_DocSectionLayout * pSection = pBlock->getDocSectionLayout())
			pSection->getAP(pSectionAP);

		pBlock->getSpanAP(posStart - pBlock->getPosition(), bLeftSide, pSpanAP);
	}
	else
	{
		UT_DEBUGMSG(("WARNING: FV_View::getAllAttrProp: No block at start of selection!\n"));
	}
	return true;
}

/* Find out whether a property has been defined explicitly for a given span, or for spans
 * within the current selection, rather than just in the document style or as a default;
 * either way, get the resultant value - and, if a selection, say whether spans differ
 * in value or explicitness.
 * 
 * NOTES:
 * 1. If a property is specified explicitly at block level, then we consider it to be
 *    explicit also at span level.
 * 2. In the case of a mixed selection, it returns szValue and bExplicitlyDefined for the
 *    start of the selection.
 * 
 * [Question: How does, e.g., paragraph background color export to other formats?]
 */
bool FV_View::queryCharFormat(const gchar * szProperty, UT_UTF8String & szValue, bool & bExplicitlyDefined, bool & bMixedSelection) const
{
	UT_return_val_if_fail(szProperty,false);

	bMixedSelection = false;

	if (isSelectionEmpty())
		return queryCharFormat(szProperty, szValue, bExplicitlyDefined, getPoint());

	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd   = posStart;

	if (m_Selection.getSelectionAnchor() < posStart)
		posStart = m_Selection.getSelectionAnchor();
	else
		posEnd   = m_Selection.getSelectionAnchor();

	if (posStart < 2)
		posStart = 2;

	PT_DocPosition position = posStart;

	bool okay  = true;
	bool first = true;

	bool bLeftSide = true; // looking to the left of the cursor, I think... does this work with bidi? [TODO: ??]

	bool bExplicitlyDefined_current;

	UT_UTF8String szValue_current;

	const PP_AttrProp *      pSpanAP = 0;
	const PP_AttrProp * prev_pSpanAP = 0;

	while (position < posEnd)
	{
		fl_BlockLayout * pBlock = getBlockAtPosition(position);
		if (!pBlock)
		{
			UT_DEBUGMSG(("FV_View::queryCharFormat(bool): no block at position!\n"));
			okay = false;
			break;
		}
		pBlock->getSpanAP(position - pBlock->getPosition(), bLeftSide, pSpanAP);

		if (first || (pSpanAP != prev_pSpanAP))
		{
			okay = queryCharFormat(szProperty, szValue_current, bExplicitlyDefined_current, position);
			if (!okay)
				break;

			if (first)
			{
				bExplicitlyDefined = bExplicitlyDefined_current;
				szValue = szValue_current;
			}
			else if (!bMixedSelection)
			{
				if ((bExplicitlyDefined_current != bExplicitlyDefined) || (szValue_current != szValue))
					bMixedSelection = true;
			}
			first = false;
			prev_pSpanAP = pSpanAP;
		}
		++position; // ??
	}
	return okay;
}


/*!
 * Returns true if the abigrammar plugin is loaded
 */
bool FV_View::isGrammarLoaded(void) const
{
	XAP_Module * pGrammar = m_pApp->getPlugin("abigrammar");
	if(pGrammar == NULL)
	{
		return false;
	}
	return true;
}

/*!
 * Returns true if the abimathview plugin is loaded
 */
bool FV_View::isMathLoaded(void) const
{
	XAP_Module * pMath = m_pApp->getPlugin("abimathview");
	if(pMath == NULL)
	{
		return false;
	}
	return true;
}

bool FV_View::queryCharFormat(const gchar * szProperty, UT_UTF8String & szValue, bool & bExplicitlyDefined, PT_DocPosition position) const
{
	UT_return_val_if_fail(szProperty,false);

	fl_BlockLayout * pBlock = getBlockAtPosition(position);
	if (!pBlock)
	{
		UT_DEBUGMSG(("FV_View::queryCharFormat(PT_DocPosition): no block at position!\n"));
		return false;
	}

	bool bLeftSide = true; // looking to the left of the cursor, I think... does this work with bidi? [TODO: ??]

	const PP_AttrProp * pSectionAP = 0;
	const PP_AttrProp * pBlockAP   = 0;
	const PP_AttrProp * pSpanAP    = 0;

	pBlock->getAP(pBlockAP);

	if (fl_DocSectionLayout * pSection = pBlock->getDocSectionLayout())
		pSection->getAP(pSectionAP);

	pBlock->getSpanAP(position - pBlock->getPosition(), bLeftSide, pSpanAP);

	bExplicitlyDefined = false;

	const gchar * szPropValue = 0;

	if (pSpanAP)
	{
		if (pSpanAP->getProperty(szProperty, szPropValue))
		{
			UT_DEBUGMSG(("Property \"%s\" defined at span level as \"%s\"\n",szProperty,szPropValue));
			szValue = szPropValue;
			bExplicitlyDefined = true;
		}
	}
	if (pBlockAP && !bExplicitlyDefined)
	{
		if (pBlockAP->getProperty(szProperty, szPropValue))
		{
			xxx_UT_DEBUGMSG(("Property \"%s\" defined at block level as \"%s\"\n",szProperty,szPropValue));
			szValue = szPropValue;
			bExplicitlyDefined = true;
		}
	}

	bool okay = true;

	if (!bExplicitlyDefined)
	{
		szPropValue = PP_evalProperty(szProperty, pSpanAP, pBlockAP, pSectionAP, m_pDoc, true);
		if (szPropValue != NULL)
		{
			xxx_UT_DEBUGMSG(("Property \"%s\" defined at style/document level as \"%s\"\n",szProperty,szPropValue));
			szValue = szPropValue;
		}
		else
		{
			/* PP_evalProperty returns NULL only if something is very wrong...
			 */
			szValue = "";
			okay = false;
		}
	}
	return okay;
}

bool FV_View::getCharFormat(PP_PropertyVector & props, bool bExpandStyles) const
{
	return getCharFormat(props, bExpandStyles, 0);
}

bool FV_View::getCharFormat(PP_PropertyVector & props, bool bExpandStyles, PT_DocPosition posStart) const
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance
	UT_GenericVector<_fmtPair *> v;
	UT_uint32 i;
	_fmtPair * f;

//
// Check we have a layout defined first. On start up we don't
//

	if(getLayout()->getFirstSection() == NULL)
	{
		return false;
	}
	if((AV_View::getTick() == m_CharProps.getTick()) && m_CharProps.isValid())
	{
		props = m_CharProps.getProps();
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
	if(pBlock == NULL)
	{
		// We want to return NULL here, because that makes it clear to the caller that the props are
		// not valid; in any case to return sometimes a dynamically allocated array and sometimes a
		// statically allocated one is not a good idea
		//
		// static const char * pszTmp[2] = {NULL,NULL};

		props = PP_NOPROPS;
		return false;
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

	pBlock->getSpanAP( (posStart - blockPosition), bLeftSide, pSpanAP);

	pBlock->getAP(pBlockAP);

	UT_uint32 iPropsCount = PP_getPropertyCount();

	for(UT_uint32 n = 0; n < iPropsCount; n++)
	{
		if((PP_getNthPropertyLevel(n) & PP_LEVEL_CHAR))
		{
			f = new _fmtPair(PP_getNthPropertyName(n),pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
			if(f->m_val != NULL)
				v.addItem(f);
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
				pBlock->getAP(pAP);
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
			pBlock->getSpanAP(pRun->getBlockOffset()+pRun->getLength(),true,pAP);
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
					f = v.getNthItem(i-1);

					const gchar * value = PP_evalProperty(f->m_prop,pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);

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
	i = v.getItemCount();
	while (i > 0)
	{
		f = v.getNthItem(i-1);
		i--;

		props.push_back(f->m_prop);
		props.push_back(f->m_val);

	}

	UT_VECTOR_PURGEALL(_fmtPair *,v);

	m_CharProps.fillProps(props);
	return true;
}


/*!
   This method fills a vector with all the blocks contained between the
   first and last blocks of a list structure.
   \param	v Pointer to Vector of all the blocks found
*/

void FV_View::getAllBlocksInList(UT_GenericVector<fl_BlockLayout *> * v) const
{
	//
	// get all the blocks in the list
	//
	fl_BlockLayout * pBlock;
	fl_AutoNum * pAuto = getCurrentBlock()->getAutoNum();
	if(pAuto == NULL)
	{
		pBlock = getCurrentBlock();
		v->addItem(pBlock);
		return;
	}
	pf_Frag_Strux* pFirstSdh = pAuto->getFirstItem();
	pf_Frag_Strux* pLastSdh = pAuto->getNthBlock(pAuto->getNumLabels()-1);
	fl_SectionLayout * pSl = getCurrentBlock()->getSectionLayout();
	pBlock = pSl->getNextBlockInDocument();
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
		if(foundFirst == true && (pBlock->getContainerType() == FL_CONTAINER_BLOCK))
			v->addItem(pBlock);
		if(pBlock->getStruxDocHandle() == pLastSdh)
			foundLast = true;
		pBlock = static_cast<fl_BlockLayout *>(pBlock->getNextBlockInDocument());
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
	UT_GenericVector<fl_BlockLayout *> v;
	UT_String szAlign;
	UT_String szIndent;
	double fIndent;
	bool bRet = true;
	UT_Dimension dim;
	double fAlign;
	fl_BlockLayout * pBlock;
	UT_sint32 i;
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

	PP_PropertyVector props = {
		"", "0.0in"
	};

	const gchar * indent;
	for(i = 0; i<v.getItemCount();i++)
	{
		pBlock = v.getNthItem(i);
		if(pBlock->getDominantDirection() == UT_BIDI_RTL) {
			indent = "margin-right";
		} else {
			indent = "margin-left";
		}

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
		pf_Frag_Strux* sdh = pBlock->getStruxDocHandle();
		PT_DocPosition iPos = m_pDoc->getStruxPosition(sdh)+fl_BLOCK_STRUX_OFFSET;
		props[0] = indent;
		props[1] = static_cast<const gchar *>(szNewAlign.c_str());
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, iPos, iPos, PP_NOPROPS, props, PTX_Block);
	}
	//
	// Moved outside the loop. Speeds things up and seems OK.
	//

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	_generalUpdate();

	m_pDoc->endUserAtomicGlob();
	_fixInsertionPointCoords();
	notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR);
	return bRet;
}

bool FV_View::removeStruxAttrProps(PT_DocPosition ipos1,
								   PT_DocPosition ipos2,
								   PTStruxType iStrux,
								   const PP_PropertyVector & attributes,
								   const PP_PropertyVector & properties)
{
	bool bRet;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	_clearIfAtFmtMark(getPoint());
	bRet = m_pDoc->changeStruxFmt(PTC_RemoveFmt,ipos1,ipos2,attributes,properties,iStrux);

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	_generalUpdate();
	_fixInsertionPointCoords();

	return bRet;
}

bool FV_View::isImageAtStrux(PT_DocPosition ipos1, PTStruxType iStrux)
{
	pf_Frag_Strux* sdh = NULL;
	bool  bret = m_pDoc->getStruxOfTypeFromPosition(ipos1, iStrux, &sdh);
	if(!bret)
	{
		return false;
	}	
	const gchar * pszDataID = NULL;
    bret = m_pDoc->getAttributeFromSDH(sdh, isShowRevisions(), getRevisionLevel(), PT_STRUX_IMAGE_DATAID,&pszDataID);
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

bool FV_View::setBlockFormat(const PP_PropertyVector & properties)
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
	UT_BidiCharType iDomDir = UT_BIDI_LTR;

	const std::string & rtl = PP_getAttribute("dom-dir", properties);
	if (!rtl.empty()) {
		bDomDirChange = true;
		if (rtl == "rtl") {
			iDomDir = UT_BIDI_RTL;
		}
	}

	if(bDomDirChange)
	{

		fl_BlockLayout * pBl = _findBlockAtPosition(posStart);
		fl_BlockLayout * pBl2 = _findBlockAtPosition(posEnd);

		if(pBl2)
			pBl2 = static_cast<fl_BlockLayout *>(pBl2->getNextBlockInDocument());

		while(pBl)
		{

			if(iDomDir == UT_BIDI_RTL)
			{
				static_cast<fp_Line *>(static_cast<fl_BlockLayout *>(pBl)->getLastContainer())->getLastRun()->setDirection(UT_BIDI_LTR);
			}
			else
			{
				static_cast<fp_Line *>(static_cast<fl_BlockLayout *>(pBl)->getLastContainer())->getLastRun()->setDirection(UT_BIDI_RTL);
			}

			pBl = static_cast<fl_BlockLayout *>(pBl->getNextBlockInDocument());
			if(pBl == pBl2)
				break;
		}
	}


	// We need to know if the selection is inside a table, because in that
	// case, not all blocks can be selected
	pf_Frag_Strux *tstart, *tend;
	if (m_pDoc->getStruxOfTypeFromPosition(posStart,PTX_SectionTable,&tstart)
	    && m_pDoc->getStruxOfTypeFromPosition(posEnd,PTX_SectionTable,&tend)
	    && tstart == tend)
	{
		bRet = false;
		UT_GenericVector<fl_BlockLayout*> vBlock;
		getBlocksInSelection(&vBlock);
		fl_ContainerLayout * pCL = NULL;
		UT_sint32 i =0;
		for(i=0; i<vBlock.getItemCount();i++)
		{
			fl_BlockLayout * pBL = vBlock.getNthItem(i);
			pCL = pBL->myContainingLayout();
			if(pCL->getContainerType() == FL_CONTAINER_CELL)
			{
				PT_DocPosition pos = pBL->getPosition();
				bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, pos, pos, PP_NOPROPS, properties,PTX_Block);
			}
		}
	}
	else
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, posStart, posEnd, PP_NOPROPS, properties, PTX_Block);

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_generalUpdate();

	notifyListeners(AV_CHG_MOTION | AV_CHG_ALL);

	_fixInsertionPointCoords();

	return bRet;
}


/*!
 * Collapse text to the level specified over the range of text given.
 */
bool FV_View::setCollapsedRange(PT_DocPosition posLow,
								PT_DocPosition posHigh,
								const PP_PropertyVector & properties)
{
	bool bRet;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	_clearIfAtFmtMark(getPoint());

	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, posLow, posHigh, PP_NOPROPS, properties);


	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	_generalUpdate();
	_fixInsertionPointCoords();

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
 * \param bIsFooter true if the user wants a pagenumber in a footer.
 * \param atts %PP_PropertyVector describing left , center or right
 * justification.
 */

bool FV_View::processPageNumber(HdrFtrType hfType, const PP_PropertyVector & atts)
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
		setPoint(oldpos);
		if(m_pDoc->isEndFrameAtPos(oldpos-1))
		{
			setPoint(oldpos-1);
		}
		return true;
	}
	else if(hfType == FL_HDRFTR_HEADER && pDSL->getHeader() == NULL)
	{
		insertPageNum(atts, hfType);
		setPoint(oldpos);
		if(m_pDoc->isEndFrameAtPos(oldpos-1))
		{
			setPoint(oldpos-1);
		}
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
	fl_BlockLayout * pBL = pHFSL->getNextBlockInDocument();
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

		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, pos, pos, PP_NOPROPS, atts, PTX_Block);

		if(bInsertFromHdrFtr)
		{
			_setPoint(oldpos);
			setHdrFtrEdit(pShadow);
		}

		// Signal PieceTable Changes have finished
		_restorePieceTableState();
		_generalUpdate();
		return bRet;
	}
//
// We're here if there's a header/footer with no page number
// Insert a page number with the correct formatting.
//

	const PP_PropertyVector f_attributes = {
		"type", "page_number"
	};
	pBL = pHFSL->getNextBlockInDocument();
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
	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, pos, pos, PP_NOPROPS, atts, PTX_Block);

	// Insert the page_number field with the requested attributes at the top
	// of the header/footer.

	bRet = m_pDoc->insertObject(pos, PTO_Field, f_attributes, PP_NOPROPS);
	m_pDoc->endUserAtomicGlob();

	if(bInsertFromHdrFtr)
	{
		_setPoint(oldpos);
		setHdrFtrEdit(pShadow);
	}

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_generalUpdate();
	return bRet;
}


void FV_View::changeListStyle(	fl_AutoNum* pAuto,
								FL_ListType lType,
								UT_uint32 startv,
								const gchar* pszDelim,
								const gchar* pszDecimal,
								const gchar* pszFont,
								float Align,
								float Indent)
{
	UT_sint32 i=0;
	gchar pszStart[80],pszAlign[20],pszIndent[20];
	UT_GenericVector<pf_Frag_Strux*> vb;
	pf_Frag_Strux* sdh2 = pAuto->getNthBlock(i);
	m_pDoc->beginUserAtomicGlob();

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	m_pDoc->disableListUpdates();

	if(lType == NOT_A_LIST)
	{
		// Stop lists in all elements
		i = 0;
		sdh2 = pAuto->getNthBlock(i);
		while(sdh2 != NULL)
		{
			vb.addItem(sdh2);
			i++;
			sdh2 = pAuto->getNthBlock(i);
		}
		for(i=0; i< vb.getItemCount(); ++i)
		{
			pf_Frag_Strux* sdh = vb.getNthItem(i);
			m_pDoc->listUpdate(sdh);
			m_pDoc->StopList(sdh);
		}

		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();

		// Signal PieceTable Changes have finished
		_restorePieceTableState();
		_generalUpdate();
		m_pDoc->endUserAtomicGlob();

		return;
	}

	const gchar * style = getCurrentBlock()->getListStyleString(lType);

	pAuto->setListType(lType);
	sprintf(pszStart, "%i" , startv);
	strncpy( pszAlign,
					UT_convertInchesToDimensionString(DIM_IN, Align, 0),
					sizeof(pszAlign));

	strncpy( pszIndent,
					UT_convertInchesToDimensionString(DIM_IN, Indent, 0),
					sizeof(pszIndent));

	PP_PropertyVector props = {
		"start-value", pszStart,
		"margin-left", pszAlign,
		"text-indent", pszIndent,
		"list-style", style
	};
	pAuto->setStartValue(startv);
	if(pszDelim != NULL)
	{
		props.push_back( "list-delim");
		props.push_back( pszDelim);
		pAuto->setDelim(pszDelim);
	}
	if(pszDecimal != NULL)
	{
		props.push_back( "list-decimal");
		props.push_back( pszDecimal);
		pAuto->setDecimal(pszDecimal);
	}
	if(pszFont != NULL)
	{
		props.push_back( "field-font");
		props.push_back( pszFont);
	}

	//
	// Now assemble the list properties
	//

	i = 0;
	sdh2 = pAuto->getNthBlock(i);
	while(sdh2 != NULL)
	{
		PT_DocPosition iPos = m_pDoc->getStruxPosition(sdh2)+fl_BLOCK_STRUX_OFFSET;
		UT_DebugOnly<bool> bRet;
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, iPos, iPos, PP_NOPROPS, props, PTX_Block);
		UT_ASSERT(bRet);
		i++;
		sdh2 = pAuto->getNthBlock(i);
		_generalUpdate();
	}

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_generalUpdate();
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();
	m_pDoc->endUserAtomicGlob();
	_ensureInsertionPointOnScreen();
}


bool FV_View::getSectionFormat(PP_PropertyVector & props) const
{
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;
	UT_GenericVector<_fmtPair *> v;
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
	if((AV_View::getTick() == m_SecProps.getTick()) && b)
	{
		xxx_UT_DEBUGMSG(("GOt a valid cache for section props \n"));
		props = m_SecProps.getProps();
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
	UT_return_val_if_fail(pBlock,false);
	fl_DocSectionLayout* pSection = pBlock->getDocSectionLayout();
	pSection->getAP(pSectionAP);

	UT_uint32 iPropsCount = PP_getPropertyCount();
	for(UT_uint32 n = 0; n < iPropsCount; n++)
	{
		if((PP_getNthPropertyLevel(n) & PP_LEVEL_SECT))
		{
			f = new _fmtPair(PP_getNthPropertyName(n),NULL,pBlockAP,pSectionAP,m_pDoc,false);
			if(f->m_val != NULL)
				v.addItem(f);
			else
				delete f;
		}
	}

	// 2. prune 'em as they vary across selection
	if (!isSelectionEmpty())
	{
		fl_BlockLayout* pBlockEnd = _findBlockAtPosition(posEnd);
		UT_ASSERT_HARMLESS( pBlockEnd );
		if(!pBlockEnd)
		{
			UT_VECTOR_PURGEALL(_fmtPair *,v);
			return false;
		}
		
		fl_DocSectionLayout *pSectionEnd = pBlockEnd->getDocSectionLayout();

		while (pSection && (pSection != pSectionEnd))
		{
			const PP_AttrProp * pAP;
			bool bCheck = false;

			pSection = pSection->getNextDocSection();
			if (!pSection)				// at EOD, so just bail
				break;

			// did block format change?
			pSection->getAP(pAP);
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
					f = v.getNthItem(i-1);

					const gchar * value = PP_evalProperty(f->m_prop,NULL,pBlockAP,pSectionAP,m_pDoc,false);

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
	props.clear();

	i = v.getItemCount();
	while (i > 0)
	{
		f = v.getNthItem(i-1);
		i--;

		props.push_back(f->m_prop);
		props.push_back(f->m_val);
	}
	UT_VECTOR_PURGEALL(_fmtPair *,v);

	m_SecProps.fillProps(props);
	b = m_SecProps.isValid();
	UT_ASSERT(b);

	return true;
}

/*!
 * Set a string with the cell properties of the cell located at position pos
 */
bool FV_View::getCellFormat(PT_DocPosition pos, UT_String & sCellProps) const
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
	pCell->getAP(pCellAP);

	UT_sint32 iPropsCount = PP_getPropertyCount();
	UT_sint32 n = 0;
	std::string sPropName;
	UT_String sPropVal;
	const gchar * pszPropVal;
	for(n = 0; n < iPropsCount; n++)
	{
		if((PP_getNthPropertyLevel(n) & PP_LEVEL_TABLE))
		{
			sPropName = PP_getNthPropertyName(n);
			sPropVal.clear();
			bool bFound = pCellAP->getProperty(sPropName, pszPropVal);
			if(bFound)
			{
				sPropVal = pszPropVal;
				UT_String_setProperty(sCellProps,sPropName,sPropVal);
			}
		}
	}
	return true;
}

bool FV_View::getBlockFormat(PP_PropertyVector & props,bool bExpandStyles) const
{
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;	// TODO do we care about section-level inheritance?
											// we do in the bidi version

//
// Check we have a layout defined first. On startup we don't
//
// fixme

	props.clear();

	if(getLayout()->getFirstSection() == NULL)
	{
		return false;
	}

	// currently there are 69 block level properties
	UT_GenericVector<_fmtPair *> v(69,4,true);
	UT_sint32 i;
	_fmtPair * f = NULL;

	/*
	  IDEA: We want to know block-level formatting properties, if
	  they're constant across the entire selection.  To do so, we start
	  at the beginning of the selection, load 'em all into a vector, and
	  then prune any property that collides.
	*/
	fl_BlockLayout * pBlock = _findBlockAtPosition(getPoint());
	if((AV_View::getTick() == m_BlockProps.getTick()) && m_BlockProps.isValid() &&
	   (pBlock == m_BlockProps.getCurrentCL()))
	{
		props = m_BlockProps.getProps();
		return true;
	}
	m_BlockProps.clearProps();
	m_BlockProps.setTick(AV_View::getTick());
	m_BlockProps.setCurrentCL(pBlock);

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
	pBlock = _findBlockAtPosition(posStart);
	if(pBlock == NULL)
	{
		return false;
	}
	pBlock->getAP(pBlockAP);

	fl_SectionLayout* pSection = pBlock->getSectionLayout();
	pSection->getAP(pSectionAP);


	UT_uint32 iPropsCount = PP_getPropertyCount();
	for(UT_uint32 n = 0; n < iPropsCount; n++)
	{
		if((PP_getNthPropertyLevel(n) & PP_LEVEL_BLOCK))
		{
			f = new _fmtPair(PP_getNthPropertyName(n),NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
			if(f->m_val != NULL)
				v.addItem(f);
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
			pBlock->getAP(pAP);
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
					f = v.getNthItem(i-1);

					const gchar * value = PP_evalProperty(f->m_prop,NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
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
	i = v.getItemCount();
	while (i > 0)
	{
		f = v.getNthItem(i-1);
		i--;

		props.push_back(f->m_prop);
		props.push_back(f->m_val);
	}
	UT_VECTOR_PURGEALL(_fmtPair *,v);

	m_BlockProps.fillProps(props);

	return true;
}

void FV_View::delTo(FV_DocPos dp)
{
	PT_DocPosition iPos = _getDocPos(dp);
	PT_DocPosition iPoint = getPoint();

	if (iPos == iPoint)
	{
		return;
	}
 
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	_extSelToPos(iPos);

	bool bCaretLeft = false;
	if(isMarkRevisions() && iPos < iPoint)
	{
		// move to the start of the original selection
		bCaretLeft = true;
	}
	
	_deleteSelection(NULL, false, bCaretLeft);

	
	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_generalUpdate();
	_fixInsertionPointCoords();

}

UT_uint32 FV_View::getSelectionLength(void) const
{
	return static_cast<UT_uint32>(labs(m_iInsPoint - m_Selection.getSelectionAnchor()));
}
	
/*
	This function is somewhat of a compromise.	It will return a new
	range of memory (destroy with g_free()) full of what's in the selection,
	but it will not cross block boundaries.  This is for convenience
	in implementation, but could probably be accomplished without
	too much more effort.  However, since an edit entry in a dialog
	box (1) only allows a bit of text and (2) has no concept of a
	block break anyway, I don't see a reason to make this behave
	differently.

	The caller must g_free the returned pointer !!!
*/
void FV_View::getSelectionText(UT_UCS4Char * & pText) const
{
	UT_ASSERT(!isSelectionEmpty());

	UT_GrowBuf buffer;

	UT_sint32 selLength = getSelectionLength();

	PT_DocPosition low;
	const fl_BlockLayout * block; // the current block the insertion point is in
	if (m_iInsPoint > m_Selection.getSelectionAnchor())
	{
		low = m_Selection.getSelectionAnchor();
		block = m_pLayout->findBlockAtPosition(low+1);
	}
	else
	{
		low = m_iInsPoint;
		block = m_pLayout->findBlockAtPosition(low);
	}

	if (block)
	{
		block->getBlockBuf(&buffer);

		UT_UCSChar * bufferSegment = NULL;

		PT_DocPosition offset = 0;
		if( low >= block->getPosition(false) )
		{
			offset = low - block->getPosition(false);
		}
		if( buffer.getLength() <= 0)
		{
			pText = NULL;
			return;
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

		if(!bufferSegment)
		{
			pText = NULL;
			return;
		}
		
		// copy it out
		memmove(bufferSegment, buffer.getPointer(offset), selLength * sizeof(UT_UCSChar));

		pText = bufferSegment;
		return;
	}

	pText = NULL;
}

/* this function has not been debugged

   The caller must delete [] the returned pointer !!!
 */
UT_UCSChar * FV_View::getTextBetweenPos(PT_DocPosition pos1, PT_DocPosition pos2) const
{
	UT_return_val_if_fail(pos2 > pos1, NULL);

	UT_GrowBuf buffer;

	UT_uint32 iLength = pos2 - pos1;
	
	PT_DocPosition curPos = pos1;

	// get the current block the insertion point is in
	const fl_BlockLayout * pBlock = m_pLayout->findBlockAtPosition(curPos);

	UT_UCSChar * bufferRet = new UT_UCSChar[iLength+1];

	UT_ASSERT(bufferRet);
	if(!bufferRet)
		return NULL;

	UT_UCSChar * buff_ptr = bufferRet;

	while(pBlock && curPos < pos2)
	{
		buffer.truncate(0);
		pBlock->getBlockBuf(&buffer);

		if (curPos < pBlock->getPosition(FALSE)) {
			curPos = pBlock->getPosition(FALSE);
		}
		PT_DocPosition offset = curPos - pBlock->getPosition(false);
		UT_uint32 iLenToCopy = UT_MIN(pos2 - curPos, buffer.getLength() - offset);
		if(curPos < pos2 && (curPos < pBlock->getPosition(false) + pBlock->getLength()))
		{
			memmove(buff_ptr, buffer.getPointer(offset), iLenToCopy * sizeof(UT_UCSChar));
			offset	 += iLenToCopy;
			buff_ptr += iLenToCopy;
			curPos	 += iLenToCopy;
			if (curPos < pos2)
			{
				*buff_ptr++ = '\n';
				curPos++;
			}
		}

		pBlock = static_cast<fl_BlockLayout *>(pBlock->getNextBlockInDocument());
	}

	UT_ASSERT(curPos == pos2);
	*buff_ptr = 0;
	return bufferRet;
}

bool FV_View::isTabListBehindPoint(UT_sint32 & iNumToDelete) const
{
	PT_DocPosition cpos = getPoint();
	PT_DocPosition ppos = cpos - 1;
	PT_DocPosition posBOD;
	bool bEOL = false;
	UT_uint32 iPointHeight;
	UT_sint32 xPoint, yPoint, xPoint2, yPoint2;
	bool   bDirection;
	iNumToDelete = 0;
	UT_DebugOnly<bool> bRes;
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
	while(pRun && pRun->getLength() == 0)
	{
		pRun = pRun->getPrevRun();
	}
	if(pRun == NULL)
	{
		return false;
	}
	if(pRun->getType() == FPRUN_FIELD)
	{
		const fp_FieldRun * pFRun = static_cast<const fp_FieldRun *>(pRun);
		if (pFRun->getFieldType() != FPFIELD_list_label)
		{
			return false;
		}
		iNumToDelete = 1;
		return true;
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
		const fp_FieldRun * pFRun = static_cast<const fp_FieldRun *>(pRun);
		if (pFRun->getFieldType() != FPFIELD_list_label)
		{
			return false;
		}
		iNumToDelete = 2;
		return true;
	}
}


bool FV_View::isTabListAheadPoint(void) const
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

	if (!pBlock)
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

	const fp_FieldRun * pFRun = static_cast<const fp_FieldRun *>(pRun);
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
	_moveInsPtNextPrevScreen(bNext,true);

	notifyListeners(AV_CHG_MOTION | AV_CHG_ALL);
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

	notifyListeners(AV_CHG_MOTION | AV_CHG_ALL);
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
	fp_Page * pPage = getCurrentPage();
	_moveInsPtNextPrevLine(bNext);
	if(getCurrentPage() != pPage)
	{
		notifyListeners(AV_CHG_MOTION | AV_CHG_ALL);
	}
	else
	{
		notifyListeners(AV_CHG_MOTION);
	}
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
		_moveInsPtNextPrevScreen(bNext,false);

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
		_moveInsPtNextPrevScreen(bNext,false);

		// top/bottom of doc - nowhere to go
		if (iOldPoint == getPoint())
			return;

		_extSel(iOldPoint);

		if (isSelectionEmpty())
		{
			_resetSelection();
		}
	}

	notifyListeners(AV_CHG_MOTION| AV_CHG_ALL);
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

	notifyListeners(AV_CHG_MOTION | AV_CHG_ALL);
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
	
	// Allow updates

	m_pDoc->setDontImmediatelyLayout(false);


	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	_generalUpdate();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();
	
	m_pDoc->endUserAtomicGlob();
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
	bool isTOC = false;
	bool bUseHdrFtr = true;
	if(bNotFrames)
	{
		bUseHdrFtr = false;
	}
	pPage->mapXYToPosition(bNotFrames,xClick, yClick, iNewPoint, bBOL, bEOL,isTOC, bUseHdrFtr,NULL);
	xxx_UT_DEBUGMSG((" point at (%d,%d) is docpos %d \n",xpos,ypos,iNewPoint));
	return iNewPoint;
}

void FV_View::setVisualSelectionEnabled(bool bActive)
{
	if (!bActive)
		m_SelectionHandles.hide();
	m_VisualSelectionActive=bActive;
}

void FV_View::extSelToXY(UT_sint32 xPos, UT_sint32 yPos, bool bDrag)
{
	// Figure out which page we clicked on.
	// Pass the click down to that page.
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);
	xxx_UT_DEBUGMSG((" Selected to x %d \n",xPos));

	PT_DocPosition iNewPoint;
	bool bBOL = false;
	bool bEOL = false;
	bool isTOC = false;
	pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL,isTOC, true);

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
				m_pAutoScrollTimer = UT_Timer::static_constructor(_autoScroll, this);
				if (m_pAutoScrollTimer)
					{
						m_pAutoScrollTimer->set(AUTO_SCROLL_MSECS);
					}
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
		_updateSelectionHandles();
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
	bool isTOC = false;
	pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL, isTOC,true);

	//UT_ASSERT(!isSelectionEmpty());

	xxx_UT_DEBUGMSG(("left anchor %d right %d \n",m_Selection.getSelectionLeftAnchor(),m_Selection.getSelectionRightAnchor()));

	PT_DocPosition iNewPointWord = 0;
	PT_DocPosition iTmpPointWord = 0;
	/*
Here is the Logic.
If No selection present:

If iNewPoint > getSelectionAnchor()
then:
1. Make sure selection anchor is at nearest Beginning of Word (BOW)
2. Make iNewPoint go to nearest End of Word (EOW)
else
1. Make selection anchor go to nearest End of Word (EOW)
2. Make iNewPoint go to nearest BOW
endif

else selection is present:

if oldpoint >= anchor and iNewPoint >= anchor then
 iNewPoint goes to nearest BOW
else if oldpoint > anchor and (iNewPoint < anchor) then

clear selection
iNewPoint goes to nearest BOW before anchor
anchor goes to nearest EOW to iNewPoint

else if oldpoint < anchor and iNewPoint < anchor then

iNewPoint goes to nears BOW

else if oldpoint < anchor and iNewPoint >= anchor then

clear selection
anchor goes to nearest BOW > iNewPoint
iNewPoint goes to nearest EOW > anchor

endif
	*/
	if(isSelectionEmpty())
	{
		if(iNewPoint > getPoint())
		{
			iNewPointWord = getPoint();
			if(!m_pDoc->isBlockAtPos(iNewPointWord) &&
			   !m_pDoc->isTableAtPos(iNewPointWord) &&
			   !m_pDoc->isCellAtPos(iNewPointWord) &&
			   !m_pDoc->isEndTableAtPos(iNewPointWord))
			{
				iNewPointWord = _getDocPosFromPoint(getPoint(), FV_DOCPOS_BOW, false);
			}
			m_Selection.setSelectionAnchor(iNewPointWord);
			iNewPointWord = iNewPoint;
			if(!m_pDoc->isBlockAtPos(iNewPointWord) &&
			   !m_pDoc->isTableAtPos(iNewPointWord) &&
			   !m_pDoc->isCellAtPos(iNewPointWord) &&
			   !m_pDoc->isEndTableAtPos(iNewPointWord))
			{

				iNewPointWord = _getDocPosFromPoint(iNewPoint, FV_DOCPOS_EOW_SELECT, false);
			}
		}
		else
		{
			iNewPointWord = getPoint();
			if(!m_pDoc->isBlockAtPos(iNewPointWord) &&
			   !m_pDoc->isTableAtPos(iNewPointWord) &&
			   !m_pDoc->isCellAtPos(iNewPointWord) &&
			   !m_pDoc->isEndTableAtPos(iNewPointWord))
			{
				iNewPointWord = _getDocPosFromPoint(getPoint(), FV_DOCPOS_EOW_SELECT, false);
			}

			m_Selection.setSelectionAnchor(iNewPointWord);
			iNewPointWord = iNewPoint;
			if(!m_pDoc->isBlockAtPos(iNewPointWord) &&
			   !m_pDoc->isTableAtPos(iNewPointWord) &&
			   !m_pDoc->isCellAtPos(iNewPointWord) &&
			   !m_pDoc->isEndTableAtPos(iNewPointWord))
			{
				iNewPointWord = _getDocPosFromPoint(iNewPoint, FV_DOCPOS_BOW, false);
			}
		}
	}
	else
	{
		if((getPoint() > m_Selection.getSelectionAnchor()) && (iNewPoint >=   m_Selection.getSelectionAnchor()))
		{
			iNewPointWord = iNewPoint;
			if(!m_pDoc->isBlockAtPos(iNewPointWord) &&
			   !m_pDoc->isTableAtPos(iNewPointWord) &&
			   !m_pDoc->isCellAtPos(iNewPointWord) &&
			   !m_pDoc->isEndTableAtPos(iNewPointWord))
			{
				iNewPointWord = _getDocPosFromPoint(iNewPoint, FV_DOCPOS_EOW_SELECT, false);	
			}
			iTmpPointWord = getSelectionAnchor();
			if(!m_pDoc->isBlockAtPos(iTmpPointWord) &&
			   !m_pDoc->isTableAtPos(iTmpPointWord) &&
			   !m_pDoc->isCellAtPos(iTmpPointWord) &&
			   !m_pDoc->isEndTableAtPos(iTmpPointWord))
			{
					iTmpPointWord = _getDocPosFromPoint(iTmpPointWord, FV_DOCPOS_BOW, false);
			}
			if(iTmpPointWord != getSelectionAnchor())
			{
				_clearSelection();
				m_Selection.setSelectionAnchor(iTmpPointWord);
			}
		}
		else if ((getPoint() > m_Selection.getSelectionAnchor()) && (iNewPoint <   m_Selection.getSelectionAnchor()))
		{
			iNewPointWord = _getDocPosFromPoint(m_Selection.getSelectionAnchor(), FV_DOCPOS_BOW, false);			
			_clearSelection();
			iNewPointWord = _getDocPosFromPoint(iNewPointWord, FV_DOCPOS_EOW_SELECT, false);			
			m_Selection.setSelectionAnchor(iNewPointWord);
			iNewPointWord = _getDocPosFromPoint(iNewPointWord, FV_DOCPOS_BOW, false);			
		}
		else if ((getPoint() <= m_Selection.getSelectionAnchor()) && (iNewPoint <   m_Selection.getSelectionAnchor()))
		{
			iNewPointWord = iNewPoint;
			if(!m_pDoc->isBlockAtPos(iNewPointWord) &&
			   !m_pDoc->isTableAtPos(iNewPointWord) &&
			   !m_pDoc->isCellAtPos(iNewPointWord) &&
			   !m_pDoc->isEndTableAtPos(iNewPointWord))
			{
				iNewPointWord = _getDocPosFromPoint(iNewPoint, FV_DOCPOS_BOW, false);	
			}
			iTmpPointWord = getSelectionAnchor();
			if(!m_pDoc->isBlockAtPos(iTmpPointWord) &&
			   !m_pDoc->isTableAtPos(iTmpPointWord) &&
			   !m_pDoc->isCellAtPos(iTmpPointWord) &&
			   !m_pDoc->isEndTableAtPos(iTmpPointWord))
			{
					iTmpPointWord = _getDocPosFromPoint(iTmpPointWord, FV_DOCPOS_EOW_SELECT, false);
			}
			if(iTmpPointWord != getSelectionAnchor())
			{
				_clearSelection();
				m_Selection.setSelectionAnchor(iTmpPointWord);
			}
		}
		else
		{
			iNewPointWord = iNewPoint;
			if(!m_pDoc->isBlockAtPos(iNewPointWord) &&
			   !m_pDoc->isTableAtPos(iNewPointWord) &&
			   !m_pDoc->isCellAtPos(iNewPointWord) &&
			   !m_pDoc->isEndTableAtPos(iNewPointWord))
			{
				iNewPointWord = _getDocPosFromPoint(iNewPoint, FV_DOCPOS_BOW, false);			
			}
			_clearSelection();
			m_Selection.setSelectionAnchor(iNewPointWord);
			iNewPointWord = _getDocPosFromPoint(iNewPointWord, FV_DOCPOS_EOW_SELECT, false);			
		}
	}

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
				m_pAutoScrollTimer = UT_Timer::static_constructor(_autoScroll, this);
				if (m_pAutoScrollTimer)
					{
						m_pAutoScrollTimer->set(AUTO_SCROLL_MSECS);
					}
			}
			else
			{
				m_pAutoScrollTimer->start();
			}

			// postpone selection until timer fires
			bPostpone = true;
		}
	}
	xxx_UT_DEBUGMSG(("anchor %d extend to point %d \n",getSelectionAnchor(), iNewPointWord));
	if (!bPostpone)
	{
		_extSelToPos(iNewPointWord);
		notifyListeners(AV_CHG_MOTION);
	}
	if(getPoint() > getSelectionAnchor())
	{
		m_Selection.setSelectionLeftAnchor(getSelectionAnchor());
		m_Selection.setSelectionRightAnchor(getPoint());
	}
	else
	{
		m_Selection.setSelectionRightAnchor(m_Selection.getSelectionAnchor());
		m_Selection.setSelectionLeftAnchor(getPoint());
	}
  	xxx_UT_DEBUGMSG(("final selection anchor %d extend to point %d \n",getSelectionAnchor(), iNewPointWord));
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
bool FV_View::gotoTarget(AP_JumpTarget type, const UT_UCSChar *data)
{
	char * numberString = static_cast<char *>(UT_calloc(UT_UCS4_strlen(data)*6 + 1, sizeof(char)));
	UT_return_val_if_fail(numberString, false);
	UT_UCS4String s(data,0);
	strcpy(numberString,s.utf8_str());
	
	bool result = gotoTarget(type, numberString);
	FREEP(numberString);
	return result;
}


bool FV_View::gotoTarget(AP_JumpTarget type, const char *numberString)
{
	UT_ASSERT(m_pLayout);
	bool inc = false;
	bool dec = false;

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
	if(type != AP_JUMPTARGET_BOOKMARK && type != AP_JUMPTARGET_XMLID)
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

		notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR);

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
			fl_BlockLayout * pBL = pSL->getNextBlockInDocument();
			if(pBL == NULL)
			{
				return false;
			}
			fp_Line* pLine = static_cast<fp_Line *>(pBL->getFirstContainer());
			fp_Line * pOldLine = pLine;
			for (UT_uint32 i = 1; i < number; i++)
			{
				if(pLine == NULL)
				{
					pLine = pOldLine;
					break;
				}
				pOldLine = pLine;
				pLine = static_cast<fp_Line *>(pLine->getNext ());
				if (pLine == NULL)
				{
					pBL = pBL->getNextBlockInDocument();
					if (pBL == NULL)
					{
						return false;
					}
					else
					{
						pLine = static_cast<fp_Line *>(pBL->getFirstContainer());
					}
				}
			}
			if(pLine == NULL)
			{
				return false;
			}
			fp_Run* frun = pLine->getFirstRun ();
			fl_BlockLayout* fblock = pLine->getBlock ();
			PT_DocPosition dp = frun->getBlockOffset () + fblock->getPosition ();
			moveInsPtTo (dp);
		}

		notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR);

		break;
/*	case AP_JUMPTARGET_PICTURE:
		// TODO
		break; */
    case AP_JUMPTARGET_XMLID:
	{
		std::string xmlid = numberString;
		PD_Document* doc = getDocument();
		if( PD_DocumentRDFHandle rdf = doc->getDocumentRDF() )
		{
			std::pair< PT_DocPosition, PT_DocPosition > range = rdf->getIDRange( xmlid );
			UT_DEBUGMSG(("jump xmlid:%s gives range:%d to %d\n",
						 xmlid.c_str(), range.first, range.second ));
			moveInsPtTo(range.second);
			selectRange( range.first, range.second );
		}
		break;
	}
    case AP_JUMPTARGET_ANNOTATION:
	{
		UT_uint32 iAnnotation = number;
		fl_AnnotationLayout* l = getAnnotationLayout( iAnnotation );
		selectAnnotation( l );
		break;
	}
	case AP_JUMPTARGET_BOOKMARK:
		{
			fl_SectionLayout * pSL = m_pLayout->getFirstSection();
			fl_BlockLayout * pBL;
			fp_Run* pRun;
			fp_BookmarkRun * pB[2] = {NULL,NULL};

			UT_uint32 i = 0;
			bool bFound = false;

			UT_DEBUGMSG(("fv_View::gotoTarget: bookmark [%s]\n",numberString));
			if(UT_go_path_is_uri(numberString))
			{
				XAP_App::getApp()->openURL(numberString);
				return false;
			}
			if(m_pDoc->isBookmarkUnique(static_cast<const gchar *>(numberString)))
				goto book_mark_not_found; //bookmark does not exist

			// TODO: Make this work inside tables
			while(pSL)
			{
				pBL = pSL->getNextBlockInDocument();

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
				PT_DocPosition dp1 = pB[0]->getBookmarkedDocPosition(true);
				PT_DocPosition dp2 = pB[1]->getBookmarkedDocPosition(false);

				if(dp2 - dp1 == 1)
					moveInsPtTo (dp2);
				else
				{
					moveInsPtTo(dp1);
					selectRange( dp2, dp1 );
					
					//make a selection
					// _setPoint(dp2);
					// _setSelectionAnchor();
					// setPoint(dp1);
					// _drawSelection();
				}

			}
			else

book_mark_not_found:
			{
				//bookmark not found
				XAP_Frame * pFrame = static_cast<XAP_Frame *>(getParentData());
				UT_return_val_if_fail(pFrame, false);

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
	if (m_startPosition <2) {
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

void
FV_View::findSetStartAt(PT_DocPosition pos)
{
	PT_DocPosition posEnd;
	m_pDoc->getBounds(true, posEnd);
	UT_return_if_fail(pos <= posEnd);

	m_startPosition = pos;
	m_wrappedEnd = false;
	m_doneFind = false;
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
	bool bRes = _findReplaceReverse(pPrefix, bDoneEntireDocument, false /* do update */);
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
	bool bRes = _findReplace(pPrefix, bDoneEntireDocument, false /* do update */);
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

	if (m_startPosition <2)
	{
		m_startPosition = 2;
	}

	bool bDoneEntireDocument = false;

	// find which part of the document is currently on screen -- we will only redraw and
	// send messages to listerners within these part of the document
	PT_DocPosition posVisibleStart = getDocPositionFromXY(0,0);
	PT_DocPosition posVisibleEnd   = getDocPositionFromXY(getWindowWidth(),getWindowHeight());

	// save point -- we will end where we started
	PT_DocPosition iPoint = getPoint();

	// this could take long -- show hourglass
	setCursorWait();
	
	// Compute search prefix
	UT_uint32* pPrefix = _computeFindPrefix(m_sFind);

	// Prime with the first match - _findReplace is really
	// replace-then-find.
	_findNext(pPrefix, bDoneEntireDocument);

	// Keep replacing until the end of the buffer is hit
	while (!bDoneEntireDocument)
	{
		bool bDontUpdate = getPoint() < posVisibleStart || getPoint() > posVisibleEnd;
		if(bDontUpdate)
		{
			m_bDontNotifyListeners = true;
		}
		
		_findReplace(pPrefix, bDoneEntireDocument, bDontUpdate);
		iReplaced++;
	}

	m_pDoc->endUserAtomicGlob();

	_resetSelection();
	setPoint(iPoint);

	// restore notifications if we stopped them
	if(m_bDontNotifyListeners)
	{
		m_bDontNotifyListeners = false;
		notifyListeners(AV_CHG_MOTION);
	}
	
	_updateInsertionPoint();
	_generalUpdate();
	draw();
	setCursorToContext();

	FREEP(pPrefix);
	return iReplaced;
}


fl_BlockLayout*
FV_View::getCurrentBlock(void) const
{
	return _findGetCurrentBlock();
}

void FV_View::insertSymbol(UT_UCSChar c, const gchar * symfont)
{

	// First check to see if there is a selection already.
	// if so delete it then get the current font
	m_pDoc->beginUserAtomicGlob();

	if (!isSelectionEmpty() && !m_FrameEdit.isActive())
	{
		_deleteSelection();
		_generalUpdate();
	}
	else if(m_FrameEdit.isActive())
	{
	       m_FrameEdit.setPointInside();
	}
	// We have to determine the current font so we can put it back after
	// Inserting the Symbol

	PP_PropertyVector props_in;
	getCharFormat(props_in);
	const std::string & currentfont = PP_getAttribute("font-family",props_in);
	std::string ssymfont = symfont;
	if(ssymfont.find(currentfont) == std::string::npos)
	{
		// Set the font
		PP_PropertyVector properties = {
			"font-family", ssymfont
		};
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
	bool isTOC = false;
	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL,isTOC, true, &pShadow);
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

	m_FrameEdit.setMode(FV_FrameEdit_NOT_ACTIVE);
	m_InlineImage.setMode(FV_InlineDrag_NOT_ACTIVE);
	_setPoint(pos, bEOL);
	_ensureInsertionPointOnScreen();
	setCursorToContext();
	_updateSelectionHandles();
	notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR ); // Sevior Put this in
//	notifyListeners(AV_CHG_HDRFTR );

}

/*!
  Calculate the position of the top left corner of a page relative to the screen window.
 */
void FV_View::getPageScreenOffsets(const fp_Page* pThePage, UT_sint32& xoff,UT_sint32& yoff) const
{
	_getPageXandYOffset(pThePage, xoff, yoff, false);
	yoff -= m_yScrollOffset;
	xoff -= m_xScrollOffset;
}


/*!
  Calculate the y coordinate of the top left corner of a page relative to the application frame.
 */
void FV_View::getPageYOffset(const fp_Page* pThePage, UT_sint32& yoff) const
{
	UT_sint32 xoff = 0;
	_getPageXandYOffset(pThePage, xoff, yoff, true);
}


UT_sint32 FV_View::getPageViewSep(void) const
{
	// return the amount of gray-space we draw above the top
	// of the paper in "Page View".  return zero if not in
	// "Page View".
	XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
	if (isPreview() || m_pG->queryProperties(GR_Graphics::DGP_PAPER))
		return 0;
	else if (pFrame && pFrame->isMenuScrollHidden())
	{
			return 0;
	}
	else if (getViewMode() != VIEW_PRINT)
	{
		return m_pG->tlu(1);
	}
	else
#ifdef EMBEDDED_TARGET
		return (int ) (0.2 * fl_PAGEVIEW_PAGE_SEP);
#else		
		return fl_PAGEVIEW_PAGE_SEP;
#endif
}


UT_sint32 FV_View::getPageViewLeftMargin(void) const
{
	// return the amount of gray-space we draw to the left
	// of the paper in "Page View".  return zero if not in
	// "Page View".
	XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
	if (isPreview() || m_pG->queryProperties(GR_Graphics::DGP_PAPER) || (getViewMode() != VIEW_PRINT))
		return 0;
	else if (pFrame && pFrame->isMenuScrollHidden())
	{
			return 0;
	}
	else if(m_pLayout->isQuickPrint())
	{
		return 0;
	}

#ifdef EMBEDDED_TARGET
		return (int) (0.2 * fl_PAGEVIEW_MARGIN_X);
#else	
		return fl_PAGEVIEW_MARGIN_X;
#endif		
}

UT_sint32 FV_View::getPageViewTopMargin(void) const
{
	// return the amount of gray-space we draw above the top
	// of the paper in "Page View".  return zero if not in
	// "Page View".
	XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
	if (isPreview() || m_pG->queryProperties(GR_Graphics::DGP_PAPER) || (getViewMode() != VIEW_PRINT))
		return 0;
	else if (pFrame && pFrame->isMenuScrollHidden())
	{
		return 0;
	}
	else if(m_pLayout->isQuickPrint())
	{
		return 0;
	}
	else
#ifdef EMBEDDED_TARGET
		return (int ) (0.2 * fl_PAGEVIEW_MARGIN_Y);
#else	
		return fl_PAGEVIEW_MARGIN_Y;
#endif

}

void FV_View::setXScrollOffset(UT_sint32 v) ///////////////////////TODO: Fix this for multiple pages
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
	_updateSelectionHandles();
}

void FV_View::setYScrollOffset(UT_sint32 v)
{
	CHECK_WINDOW_SIZE
	UT_sint32 dy = v - m_yScrollOffset;

	if (dy == 0)
		return;

	m_pG->scroll(0, dy);
	m_yScrollOffset = v;

	_fixInsertionPointCoords();
	_updateSelectionHandles();
}

void FV_View::draw(int page, dg_DrawArgs* da)
{
	UT_DEBUGMSG(("FV_View::draw_1: [page %d]\n",page));
	calculateNumHorizPages();

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

/*!
* Draw a 1px line in selection background color with optional resize handles.
* \param box Where to draw.
* \param drawHandles If handles are to be drawn.
*/
void FV_View::drawSelectionBox(UT_Rect & inBox, bool drawHandles) 
{
	GR_Graphics *pG = getGraphics();
	UT_sint32 boxSize = getImageSelInfo();
	m_InlineImage.setSelectionDrawn(true);
	UT_sint32 left = inBox.left;
	UT_sint32 top = inBox.top;
	UT_sint32 right = inBox.left + inBox.width;
	UT_sint32 bottom = inBox.top + inBox.height;
	xxx_UT_DEBUGMSG(("Draw selection box Top %d \n",top));
	// draw a line around the image
	pG->setLineProperties(pG->tluD(1.0),
						  GR_Graphics::JOIN_MITER,
						  GR_Graphics::CAP_PROJECTING,
						  GR_Graphics::LINE_SOLID);	

	UT_RGBColor color = getColorSelBackground();
	pG->setColor(color);
	{
		// Need the painter lock to be released at the end of these draws

		GR_Painter painter(pG);
		painter.drawLine(left, top, right, top);
		painter.drawLine(left, top, left, bottom);
		painter.drawLine(right, top, right, bottom);
		painter.drawLine(left, bottom, right, bottom);				
	}
	// now, draw the resize boxes around the image
	if (drawHandles) {
		UT_Rect box;
		box = UT_Rect(left, top, boxSize, boxSize);
		_drawResizeHandle(box);
		box = UT_Rect(left + (right - left)/2 - boxSize/2, top, boxSize, boxSize);
		_drawResizeHandle(box); // North
		box = UT_Rect(right-boxSize+pG->tlu(1), top, boxSize, boxSize);
		_drawResizeHandle(box); // North East
		box = UT_Rect(right-boxSize+pG->tlu(1), top + ((bottom - top) / 2) - boxSize/2, boxSize, boxSize);
		_drawResizeHandle(box); // East
		box = UT_Rect(right-boxSize+pG->tlu(1), bottom - boxSize + pG->tlu(1), boxSize, boxSize);
		_drawResizeHandle(box); // South East
		box = UT_Rect(left + (right - left)/2 - boxSize/2, bottom - boxSize + pG->tlu(1), boxSize, boxSize);
		_drawResizeHandle(box); // South
		box = UT_Rect(left, bottom - boxSize + pG->tlu(1), boxSize, boxSize);
		_drawResizeHandle(box); // South West
		box = UT_Rect(left, top + ((bottom - top) / 2) - boxSize/2, boxSize, boxSize);
		_drawResizeHandle(box); // West
	}
}

#define SELDOWNCOLOR(v,w)(v > w ? v - w : 0)
#define SELUPCOLOR(v,w)(v > (255-w) ? 255 : v + w)
/*!
* Draw a nice 3d resize handle at box.
* \param box Where to draw.
*/
inline void FV_View::_drawResizeHandle(UT_Rect & box)
{
	GR_Graphics * pG = getGraphics();
	UT_sint32 left = box.left;
	UT_sint32 top = box.top;
	UT_sint32 right = box.left + box.width - pG->tlu(1);
	UT_sint32 bottom = box.top + box.height - pG->tlu(1);

	GR_Painter painter(pG);
	
	pG->setLineProperties(pG->tluD(1.0),
								 GR_Graphics::JOIN_MITER,
								 GR_Graphics::CAP_PROJECTING,
								 GR_Graphics::LINE_SOLID);	
	
	UT_RGBColor color = getColorSelBackground();
	pG->setColor(color);

	UT_RGBColor downColor1 = UT_RGBColor(SELDOWNCOLOR(color.m_red, 40), SELDOWNCOLOR(color.m_grn, 40), SELDOWNCOLOR(color.m_blu, 40));
	UT_RGBColor downColor2 = UT_RGBColor(SELDOWNCOLOR(color.m_red, 20), SELDOWNCOLOR(color.m_grn, 20), SELDOWNCOLOR(color.m_blu, 20));
	UT_RGBColor upColor1 = UT_RGBColor(SELUPCOLOR(color.m_red, 40), SELUPCOLOR(color.m_grn, 40), SELUPCOLOR(color.m_blu, 40));
	UT_RGBColor upColor2 = UT_RGBColor(SELUPCOLOR(color.m_red, 20), SELUPCOLOR(color.m_grn, 20), SELUPCOLOR(color.m_blu, 20));

	painter.fillRect(color,box.left + pG->tlu(1), box.top + pG->tlu(1), box.width - pG->tlu(3), box.height - pG->tlu(3));

	// east & south
	pG->setColor(downColor1);
	painter.drawLine(right, top, right, bottom);
	painter.drawLine(left, bottom, right, bottom);
	pG->setColor(downColor2);
	painter.drawLine(right - pG->tlu(1), top + pG->tlu(1), right - pG->tlu(1), bottom - pG->tlu(1));
	painter.drawLine(left + pG->tlu(1), bottom - pG->tlu(1), right - pG->tlu(1), bottom - pG->tlu(1));

	// north & west
	pG->setColor(upColor1);
	painter.drawLine(left, top, right, top);
	painter.drawLine(left, top, left, bottom);
	pG->setColor(upColor2);
	painter.drawLine(left + pG->tlu(1), top + pG->tlu(1), right - pG->tlu(1), top + pG->tlu(1));
	painter.drawLine(left + pG->tlu(1), top + pG->tlu(1), left + pG->tlu(1), bottom - pG->tlu(1));

/* This is the original code, but rearranged above so we don't have to set the colour so often
	// west
	pG->setColor(UT_RGBColor(color.m_red - 40,color.m_grn - 40,color.m_blu - 40));
	painter.drawLine(right, top, right, bottom);
	pG->setColor(UT_RGBColor(color.m_red - 20,color.m_grn - 20,color.m_blu - 20));
	painter.drawLine(right - pG->tlu(1), top + pG->tlu(1), right - pG->tlu(1), bottom - pG->tlu(1));

	// south
	pG->setColor(UT_RGBColor(color.m_red - 40,color.m_grn - 40,color.m_blu - 40));
	painter.drawLine(left, bottom, right, bottom);
	pG->setColor(UT_RGBColor(color.m_red - 20,color.m_grn - 20,color.m_blu - 20));
	painter.drawLine(left + pG->tlu(1), bottom - pG->tlu(1), right - pG->tlu(1), bottom - pG->tlu(1));

	// north
	pG->setColor(UT_RGBColor(color.m_red + 40,color.m_grn + 40,color.m_blu + 40));
	painter.drawLine(left, top, right, top);
	pG->setColor(UT_RGBColor(color.m_red + 20,color.m_grn + 20,color.m_blu + 20));
	painter.drawLine(left + pG->tlu(1), top + pG->tlu(1), right - pG->tlu(1), top + pG->tlu(1));

	// east
	pG->setColor(UT_RGBColor(color.m_red + 40,color.m_grn + 40,color.m_blu + 40));
	painter.drawLine(left, top, left, bottom);
	pG->setColor(UT_RGBColor(color.m_red + 20,color.m_grn + 20,color.m_blu + 20));
	painter.drawLine(left + pG->tlu(1), top + pG->tlu(1), left + pG->tlu(1), bottom - pG->tlu(1));
*/
}

bool FV_View::isLeftMargin(UT_sint32 xPos, UT_sint32 yPos) const
{
	/*
	  Figure out which page we clicked on.
	  Pass the click down to that page.
	*/
	UT_sint32 xClick, yClick;
	const fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);

	PT_DocPosition iNewPoint;
	bool bBOL = false;
	bool bEOL = false;
	bool isTOC = false;
	pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL,isTOC, true);
	return bBOL;
}

void FV_View::ensureInsertionPointOnScreen(void)
{
	_ensureInsertionPointOnScreen();
}


void FV_View::setPoint(UT_uint32 pt)
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

PD_DocumentRange * FV_View::getNthSelection(UT_sint32 i) const
{
	return m_Selection.getNthSelection(i);
}

UT_sint32 FV_View::getNumSelections(void) const
{
	return m_Selection.getNumSelections();
}

void FV_View::setSelectionMode(FV_SelectionMode selMode)
{
	m_Selection.setMode(selMode);
}

void FV_View::getDocumentRangeOfCurrentSelection(PD_DocumentRange * pdr) const
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
							 UT_sint32 * pTop, UT_sint32 * pBot) const
{
	pf_Frag_Strux* cellSDH;
	bool bres = m_pDoc->getStruxOfTypeFromPosition(posCell,PTX_SectionCell,&cellSDH);
	if(!bres)
	{
		return false;
	}
	const char * pszLeft;
	const char * pszRight;
	const char * pszTop;
	const char * pszBot;
	m_pDoc->getPropertyFromSDH(cellSDH,isShowRevisions(),getRevisionLevel(),"left-attach",&pszLeft);
	if(pszLeft && *pszLeft)
	{
		*pLeft = atoi(pszLeft);
	}
	else
	{
		return false;
	}
	m_pDoc->getPropertyFromSDH(cellSDH,isShowRevisions(),getRevisionLevel(),"right-attach",&pszRight);
	if(pszRight && *pszRight)
	{
		*pRight = atoi(pszRight);
	}
	else
	{
		return false;
	}
	m_pDoc->getPropertyFromSDH(cellSDH,isShowRevisions(),getRevisionLevel(),"top-attach",&pszTop);
	if(pszTop && *pszTop)
	{
		*pTop = atoi(pszTop);
	}
	else
	{
		return false;
	}
	m_pDoc->getPropertyFromSDH(cellSDH,isShowRevisions(),getRevisionLevel(),"bot-attach",&pszBot);
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
							 UT_sint32 * pTop, UT_sint32 * pBot) const
{
	pf_Frag_Strux* cellSDH;
	bool bres = m_pDoc->getStruxOfTypeFromPosition(posCell,PTX_SectionCell,&cellSDH);
	if(!bres)
	{
		return false;
	}
	const char * pszLeft;
	const char * pszRight;
	const char * pszTop;
	const char * pszBot;
	m_pDoc->getPropertyFromSDH(cellSDH,isShowRevisions(),getRevisionLevel(),"left-style",&pszLeft);
	if(pszLeft && *pszLeft)
	{
		*pLeft = atoi(pszLeft);
	}
	else
	{
		*pLeft = -1;
	}
	m_pDoc->getPropertyFromSDH(cellSDH,isShowRevisions(),getRevisionLevel(),"right-style",&pszRight);
	if(pszRight && *pszRight)
	{
		*pRight = atoi(pszRight);
	}
	else
	{
		*pRight = -1;
	}
	m_pDoc->getPropertyFromSDH(cellSDH,isShowRevisions(),getRevisionLevel(),"top-style",&pszTop);
	if(pszTop && *pszTop)
	{
		*pTop = atoi(pszTop);
	}
	else
	{
		*pTop = -1;
	}
	m_pDoc->getPropertyFromSDH(cellSDH,isShowRevisions(),getRevisionLevel(),"bottom-style",&pszBot);
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
bool FV_View::setCellFormat(const PP_PropertyVector & properties, FormatTable applyTo, const FG_ConstGraphicPtr & pFG, UT_String & sDataID)
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
	pf_Frag_Strux* tableSDH;
	bRet = m_pDoc->getStruxOfTypeFromPosition(getPoint(),PTX_SectionTable,&tableSDH);
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
	posStart = (posTable > posStart)?posTable:posStart;

	// Need this to trigger a table update!
	_changeCellParams(posTable, tableSDH);
	
	// The Format Selection case needs some special attention
	if (applyTo == FORMAT_TABLE_SELECTION)
	{
		pf_Frag_Strux* cellSDH;
		PT_DocPosition posTemp = ((posTable == posStart) ? posStart + 2 : posStart);
		bRet = m_pDoc->getStruxOfTypeFromPosition(posTemp,PTX_SectionCell,&cellSDH);
		if(bRet)
		{
			posStart = m_pDoc->getStruxPosition(cellSDH)+1;
			//
			// Make sure posEnd is inside the Table.
			//
			pf_Frag_Strux* endTableSDH = m_pDoc->getEndTableStruxFromTablePos(posTable);
			UT_ASSERT(endTableSDH);
			if(endTableSDH == NULL)
			{
				return false;
			}
			PT_DocPosition posEndTable = m_pDoc->getStruxPosition(endTableSDH);
			if(posEnd > posEndTable)
			{
				posEnd = posEndTable -1;
			}
			// Do the actual change
			UT_GenericVector<fl_BlockLayout*> vBlock;
			getBlocksInSelection(&vBlock);
			fl_ContainerLayout * pCL = NULL;
			fl_CellLayout * pCell = NULL;
			UT_sint32 i =0;
			for(i=0; i<vBlock.getItemCount();i++)
			{
				fl_BlockLayout * pBL = vBlock.getNthItem(i);
				pCL = pBL->myContainingLayout();
				if(pCL->getContainerType() == FL_CONTAINER_CELL)
				{
					PT_DocPosition pos = pBL->getPosition();
					if (pos >= posStart && pos <= posEnd)
						bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, pos, pos, PP_NOPROPS, properties, PTX_SectionCell);
					if(static_cast<fl_CellLayout *>(pCL) != pCell)
					{
						if(pFG != NULL)
						{
							pCell = static_cast<fl_CellLayout *>(pCL);
							pFG->insertAtStrux(m_pDoc,72,pos,PTX_SectionCell,sDataID.c_str());
						}
						else
						{
							const PP_PropertyVector attributes = {
								PT_STRUX_IMAGE_DATAID, "",
							};
							bRet = m_pDoc->changeStruxFmt(PTC_RemoveFmt, pos, pos, attributes, PP_NOPROPS, PTX_SectionCell);
						}
					}
				}
			}
		}
	}
	else if(applyTo == FORMAT_TABLE_TABLE)
	{
        UT_DEBUGMSG(("Doing apply FORMAT_TABLE_TABLE \n"));
		// Loop through the table cells to adjust their formatting		
		// get the number of rows and columns in the current table

		UT_sint32 numRows;
		UT_sint32 numCols;
		bRet = m_pDoc->getRowsColsFromTableSDH(tableSDH, isShowRevisions(), getRevisionLevel(), &numRows, &numCols);

		UT_sint32 rowStart = 0;
		UT_sint32 rowEnd;
		UT_sint32 colStart = 0;
		UT_sint32 colEnd;
		
			colStart = 0;
			colEnd = numCols-1;
		
			rowStart = 0;
			rowEnd = numRows-1;
					
		// Loop through the table cells to adjust their formatting		
		UT_sint32 i;
		UT_sint32 j;
		for (j = rowStart; j <= rowEnd; j++)
		{
			for (i = colStart; i <= colEnd; i++)
			{
				pf_Frag_Strux* cellSDH = m_pDoc->getCellSDHFromRowCol(tableSDH, isShowRevisions(), getRevisionLevel(),
																		 j, i);
				if(cellSDH)
				{
					// Do the actual change
					posStart = m_pDoc->getStruxPosition(cellSDH)+1;
					bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, posStart, posStart,PP_NOPROPS, properties, PTX_SectionCell);
					if(pFG != NULL)
					{
						pFG->insertAtStrux(m_pDoc,72,posStart,
										   PTX_SectionCell,sDataID.c_str());
					}
					else
					{
						const PP_PropertyVector attributes = {
							PT_STRUX_IMAGE_DATAID, ""
						};
						bRet = m_pDoc->changeStruxFmt(PTC_RemoveFmt, posStart, posStart, attributes, PP_NOPROPS, PTX_SectionCell);
					}
				}
				else
				{
					UT_DEBUGMSG(("MARCM: Yikes! There is no cell at position (%dx%d)!\n", j, i));
				}
			}
		}
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
		bRet = m_pDoc->getRowsColsFromTableSDH(tableSDH, isShowRevisions(), getRevisionLevel(), &numRows, &numCols);
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
		UT_sint32 rowStart = 0;
		UT_sint32 rowEnd;
		UT_sint32 colStart = 0;
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
				pf_Frag_Strux* cellSDH = m_pDoc->getCellSDHFromRowCol(tableSDH, isShowRevisions(), getRevisionLevel(),
																		 j, i);
				if(cellSDH)
				{
					// Do the actual change
					posStart = m_pDoc->getStruxPosition(cellSDH)+1;
					bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, posStart, posStart, PP_NOPROPS, properties, PTX_SectionCell);
					if(pFG != NULL)
					{
						pFG->insertAtStrux(m_pDoc,72,posStart,
										   PTX_SectionCell,sDataID.c_str());
					}
					else
					{
						const PP_PropertyVector attributes = {
							PT_STRUX_IMAGE_DATAID, ""
						};
						bRet = m_pDoc->changeStruxFmt(PTC_RemoveFmt,
													  posStart,
													  posStart,
													  attributes,
													  PP_NOPROPS,
													  PTX_SectionCell);
					}
				}
				else
				{
					UT_DEBUGMSG(("MARCM: Yikes! There is no cell at position (%dx%d)!\n", j, i));
				}
			}
		}
	}
	// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
	_restoreCellParams(posTable,tableSDH);

	// Allow table updates
	m_pDoc->setDontImmediatelyLayout(false);

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_generalUpdate();
	m_pDoc->updateDirtyLists();

	_ensureInsertionPointOnScreen();
	clearCursorWait();
	notifyListeners(AV_CHG_MOTION);
	return bRet;
}

/*!
 Get the a particular property, such as the background color, of the cell containing the current cursor position
 \param col will be set to the cell to the property value, if the requested property exists
 \return True if succesful (ie. the property value is set), false otherwise
 */
bool FV_View::getCellProperty(PT_DocPosition posCell, const gchar * szPropName, gchar * &szPropValue) const
{
	pf_Frag_Strux* cellSDH;
	bool bres = m_pDoc->getStruxOfTypeFromPosition(posCell,PTX_SectionCell,&cellSDH);
	if(!bres)
	{
		return false;
	}
	m_pDoc->getPropertyFromSDH(cellSDH,isShowRevisions(),getRevisionLevel(),szPropName,const_cast<const char **>(&szPropValue));
	if(szPropValue && *szPropValue)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool FV_View::setTableFormat(const PP_PropertyVector & properties)
{
	PT_DocPosition pos = getPoint();
	return setTableFormat(pos, properties);
}

bool FV_View::setTableFormat(PT_DocPosition pos, const PP_PropertyVector & properties)
{
	bool bRet;

	PT_DocPosition posStart = pos;
	pf_Frag_Strux* tableSDH = NULL;
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
	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, posStart, posEnd, PP_NOPROPS, properties, PTX_SectionTable);

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_generalUpdate();
	_ensureInsertionPointOnScreen();
	clearCursorWait();
	AV_View::notifyListeners(AV_CHG_MOTION);
	return bRet;
}

bool FV_View::setSectionFormat(const PP_PropertyVector & properties)
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

	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, posStart, posEnd, PP_NOPROPS, properties, PTX_Section);

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
		m_iFreePass = AV_CHG_COLUMN | AV_CHG_FMTSECTION | AV_CHG_FMTBLOCK | AV_CHG_HDRFTR;
		return;
	}
	getTopRulerInfo(getPoint(), pInfo);
}

UT_sint32 FV_View::getFrameMargin(void) const
{
	return FRAME_MARGIN;// add 0.1 inch for a left border to click in
}

UT_sint32 FV_View::getNormalModeXOffset(void) const
{
	UT_ASSERT(getViewMode() != VIEW_PRINT);  
	UT_sint32 iX = getTabToggleAreaWidth();
	XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
	if(pFrame)
	{
		if(static_cast<AP_Frame *>(pFrame)->isShowMargin() && (getViewMode() != VIEW_WEB)  )
		{
			iX += FRAME_MARGIN; // add 0.1 inch for a left border to click in
		}
	}
	return iX;
}

UT_uint32 FV_View::getTabToggleAreaWidth() const
{
		if(m_pTopRuler)
			return m_pTopRuler->getTabToggleAreaWidth();
		else
#ifdef EMBEDDED_TARGET
			return (UT_uint32) ((float)m_pG->tlu(AP_TopRuler::getFixedWidth()) * 0.1);
#else
			return m_pG->tlu(AP_TopRuler::getFixedWidth());
#endif
}

void FV_View::setViewMode (ViewMode vm)
{
	bool bPrevWeb= (m_viewMode == VIEW_WEB);
	m_viewMode = vm;
	
	UT_return_if_fail( m_pLayout );
	UT_DEBUGMSG(("View mode set calling updateviewonmodechange \n"));
	m_pLayout->updateOnViewModeChange();
	if(bPrevWeb)
	{
		rebuildLayout();
		m_pLayout->formatAll();
		_generalUpdate();
	}
	else
	{
		for(UT_sint32 i = 0; i < m_pLayout->countPages(); i++)
		{
				fp_Page * pPage = m_pLayout->getNthPage(i);
				UT_return_if_fail( pPage );

				pPage->updateColumnX();
		}
	}
	_fixInsertionPointCoords();
	
}

void FV_View::getTopRulerInfo(PT_DocPosition pos,AP_TopRulerInfo * pInfo)
{
	if(m_pDoc->isPieceTableChanging())
	{
		m_iFreePass = AV_CHG_COLUMN | AV_CHG_FMTSECTION | AV_CHG_FMTBLOCK | AV_CHG_HDRFTR;
		return;
	}

	fl_BlockLayout * pBlock = NULL;
	fp_Run * pRun = NULL;
	UT_sint32 xCaret, yCaret;
	UT_uint32 heightCaret;
	UT_sint32 xCaret2, yCaret2;
	bool bDirection;
	_findPositionCoords(pos, m_bPointEOL, xCaret, yCaret, xCaret2, yCaret2, heightCaret, bDirection, &pBlock, &pRun);

	//UT_return_if_fail(pRun);
	if(!pRun) return;

	fp_Line * pLine = pRun->getLine();
	UT_return_if_fail(pLine);

	fp_Container * pContainer = pLine->getContainer();
	UT_return_if_fail(pContainer);

	fl_SectionLayout * pSection = pContainer->getSectionLayout();
	UT_return_if_fail(pSection);

	/* clear pInfo */
	pInfo->~AP_TopRulerInfo();
	new(pInfo) AP_TopRulerInfo();

	if (pSection->getType() == FL_SECTION_DOC || pSection->getContainerType() == FL_CONTAINER_FOOTNOTE || pSection->getContainerType() == FL_CONTAINER_ANNOTATION || pSection->getContainerType() == FL_CONTAINER_ENDNOTE)
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
		if(!pColumn)
			return;
		UT_uint32 nCol=0;
		fp_Column * pNthColumn=pColumn->getLeader();
		while (pNthColumn && (pNthColumn != pColumn))
		{
			nCol++;
			pNthColumn = pNthColumn->getFollower();
		}
		pInfo->m_iCurrentColumn = nCol;
		pInfo->m_iNumColumns = pDSL->getNumColumns();

		if((getViewMode() == VIEW_NORMAL) || (getViewMode() == VIEW_WEB))
		{
			pInfo->u.c.m_xaLeftMargin = getNormalModeXOffset();
			UT_sint32 iExtra = 72; 
			pInfo->u.c.m_xaRightMargin = iExtra;
		}
		else
		{
			pInfo->u.c.m_xaLeftMargin = pDSL->getLeftMargin();
			pInfo->u.c.m_xaRightMargin = pDSL->getRightMargin();
		}
		
		pInfo->u.c.m_xColumnGap = pDSL->getColumnGap();
		pInfo->u.c.m_xColumnWidth = pColumn->getWidth();
		if(pSection->getContainerType() == FL_CONTAINER_FOOTNOTE  || pSection->getContainerType() == FL_CONTAINER_ANNOTATION )
		{
			pInfo->m_iCurrentColumn = 0;
			pInfo->m_iNumColumns = 1;
			pInfo->u.c.m_xColumnGap = 0;
			pInfo->u.c.m_xColumnWidth = pContainer->getWidth();
		}
		pInfo->m_mode = AP_TopRulerInfo::TRI_MODE_COLUMNS;
		pInfo->m_xrLeftIndent = pBlock->getLeftMargin();
		pInfo->m_xrRightIndent = pBlock->getRightMargin();
		pInfo->m_xrFirstLineIndent = pBlock->getTextIndent();
		xxx_UT_DEBUGMSG(("ap_TopRuler: xrPoint %d LeftIndent %d RightIndent %d Firs %d \n",pInfo->m_xrPoint,pInfo->m_xrLeftIndent,pInfo->m_xrRightIndent,	pInfo->m_xrFirstLineIndent));
	}
	else if(isHdrFtrEdit() && (pSection->getContainerType() != FL_CONTAINER_CELL))
	{
		fp_Column* pColumn = static_cast<fp_Column*>(pContainer);
		fl_DocSectionLayout* pDSL = static_cast<fl_DocSectionLayout*>(pSection);
		pDSL = m_pEditShadow->getHdrFtrSectionLayout()->getDocSectionLayout();
			
		pInfo->m_iCurrentColumn = 0;
		pInfo->m_iNumColumns = 1;

		if((getViewMode() == VIEW_NORMAL) || (getViewMode() == VIEW_WEB))
		{
			pInfo->u.c.m_xaLeftMargin = getNormalModeXOffset();
			UT_sint32 iExtra = 72; 
			pInfo->u.c.m_xaRightMargin = iExtra;
		}
		else
		{
			pInfo->u.c.m_xaLeftMargin = pDSL->getLeftMargin();
			pInfo->u.c.m_xaRightMargin = pDSL->getRightMargin();
		}
		
		pInfo->u.c.m_xColumnGap = pDSL->getColumnGap();
		pInfo->u.c.m_xColumnWidth = pColumn->getWidth();
		pInfo->m_mode = AP_TopRulerInfo::TRI_MODE_COLUMNS;

		pInfo->m_xrPoint = xCaret - pContainer->getX();
		pInfo->m_xrLeftIndent = pBlock->getLeftMargin();
		pInfo->m_xrRightIndent = pBlock->getRightMargin();
		pInfo->m_xrFirstLineIndent = pBlock->getTextIndent();
	}
	else if(pSection->getContainerType() == FL_CONTAINER_CELL)
	{
		fp_CellContainer * pCell = static_cast<fp_CellContainer *>(pContainer);
		fl_DocSectionLayout* pDSL = pSection->getDocSectionLayout();
		fp_VerticalContainer * pColumn = static_cast<fp_Column *>(pCell->getColumn(pLine));
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
			else
			{
				return;
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
			
			if(pColumn->getContainerType() == FP_CONTAINER_COLUMN)
			{
				fp_Column * pNthColumn=static_cast<fp_Column *>(pColumn)->getLeader();
				while (pNthColumn && (pNthColumn != pColumn))
				{
					nCol++;
					pNthColumn = pNthColumn->getFollower();
				}
				pInfo->m_iCurrentColumn = nCol;
				pInfo->m_iNumColumns = pDSL->getNumColumns();
			}
			else
			{
				pInfo->m_iCurrentColumn = 0;
				pInfo->m_iNumColumns = 1;

			}

			if((getViewMode() == VIEW_NORMAL) || (getViewMode() == VIEW_WEB))
			{
				pInfo->u.c.m_xaLeftMargin = m_pTopRuler ? m_pTopRuler->getTabToggleAreaWidth() : 0;
				pInfo->u.c.m_xaRightMargin = 0;
			}
			else
			{
				pInfo->u.c.m_xaLeftMargin = pDSL->getLeftMargin();
				pInfo->u.c.m_xaRightMargin = pDSL->getRightMargin();
			}
			
			pInfo->u.c.m_xColumnGap = pDSL->getColumnGap();
			pInfo->u.c.m_xColumnWidth = pColumn->getWidth();
			pInfo->m_xrPoint = xCaret - pContainer->getX();
		}
		pInfo->m_mode = AP_TopRulerInfo::TRI_MODE_TABLE;
		pInfo->m_xrLeftIndent = pBlock->getLeftMargin();
		pInfo->m_xrRightIndent = pBlock->getRightMargin();
		pInfo->m_xrFirstLineIndent = pBlock->getTextIndent();
		fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pCell->getContainer());
		fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pTab->getSectionLayout());
		if (!pTL || !pTL->isInitialLayoutCompleted())
		{
			// The table initialization is not yet completed
			return;
		}
		UT_sint32 row = pCell->getTopAttach();
		UT_sint32 numcols = pTab->getNumCols();
		//UT_sint32 numrows = pTab->getNumRows();
		UT_sint32 i =0;
		fp_CellContainer * pCur = NULL;
		UT_sint32 iCellCount = 0;
		pInfo->m_vecTableColInfo = new UT_GenericVector<AP_TopRulerTableInfo*>();
		while( i < numcols)
		{
			pCur = pTab->getCellAtRowColumn(row,i);
			if(pCur == pCell)
			{
				pInfo->m_iCurCell = iCellCount;
			}
			UT_sint32 ioff_x = pTab->getX();
			fp_Container * pCon = static_cast<fp_Container*>(pTab->getContainer());
			while(pCon && !pCon->isColumnType())
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
				pInfo->m_vecTableColInfo->addItem(pTInfo);
				xxx_UT_DEBUGMSG(("TableColInfo RightPos %d LeftPos %d \n", pTInfo->m_iRightCellPos,pTInfo->m_iLeftCellPos));
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
		pInfo->m_vecFullTable = new UT_GenericVector<AP_TopRulerTableInfo *>();
		fp_TableRowColumn * pRC = NULL;
		UT_sint32 iCum = 0;
		UT_sint32 ioff_x = 0;
		fp_Container * pCon = static_cast<fp_Container*>(pTab->getContainer());
		while(!pCon->isColumnType())
		{
			ioff_x += pCon->getX();
			pCon = static_cast<fp_Container *>(pCon->getContainer());
		}
		xxx_UT_DEBUGMSG(("Initial X %d \n",ioff_x));
		pCur = pTab->getCellAtRowColumn(0,0);
		UT_return_if_fail(pCur);
		ioff_x += pCur->getLeftPos();
		pRC = pTab->getNthCol(0);
		xxx_UT_DEBUGMSG(("Tab X %d LeftPos %d Spacing %d \n",pTab->getX(),pCur->getLeftPos(),pRC->spacing));
		for( i=0;i < numcols;i++)
		{
			pCur = pTab->getCellAtRowColumn(0,i);
			pRC = pTab->getNthCol(i);
			UT_sint32 width = pRC->allocation + pRC->spacing;
			if(pCur)
			{
				AP_TopRulerTableInfo *pTInfo = new  AP_TopRulerTableInfo;
				pTInfo->m_pCell = pCur;
				pTInfo->m_iLeftCellPos = iCum +ioff_x;
				pTInfo->m_iRightCellPos = iCum + width +ioff_x;
				pTInfo->m_iLeftSpacing = pRC->spacing/2;
				pTInfo->m_iRightSpacing = pRC->spacing/2;
				if(i== (numcols -1))
				{
					//					pTInfo->m_iRightCellPos -= pTab->getBorderWidth()/2;
					pTInfo->m_iRightCellPos -= pTInfo->m_iRightSpacing;
					xxx_UT_DEBUGMSG(("FullTable RightPos %d Spacing %d \n", pTInfo->m_iRightCellPos,pTInfo->m_iRightSpacing));
				}
				pInfo->m_vecFullTable->addItem(pTInfo);
				xxx_UT_DEBUGMSG(("FullTable RightPos %d LeftPos %d \n", pTInfo->m_iRightCellPos,pTInfo->m_iLeftCellPos));
			}
			iCum += width;
		}
	}
	else if(pContainer->getContainerType() == FP_CONTAINER_FRAME && (getViewMode() != VIEW_NORMAL)  && (getViewMode() != VIEW_WEB))
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
		pInfo->m_xrLeftIndent = pBlock->getLeftMargin();
		pInfo->m_xrRightIndent = pBlock->getRightMargin();
		pInfo->m_xrFirstLineIndent = pBlock->getTextIndent();
		
	}
	else
	{
		// fill in the details
	}

	static UT_String buf;

	{
		UT_LocaleTransactor t(LC_NUMERIC, "C");
		buf = UT_String_sprintf ("%.4fin", m_pDoc->m_docPageSize.Width(DIM_IN));
	}

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
		m_iFreePass = AV_CHG_COLUMN | AV_CHG_FMTSECTION | AV_CHG_FMTBLOCK | AV_CHG_HDRFTR;
		return;
	}
	getLeftRulerInfo(getPoint(),pInfo);
}

void FV_View::getLeftRulerInfo(PT_DocPosition pos, AP_LeftRulerInfo * pInfo)
{
//
// Clear out the old table info
//
	if(m_pDoc->isPieceTableChanging())
	{
		m_iFreePass = AV_CHG_COLUMN | AV_CHG_FMTSECTION | AV_CHG_FMTBLOCK | AV_CHG_HDRFTR;
		return;
	}

	/* clear pInfo */
	pInfo->~AP_LeftRulerInfo();
	new(pInfo) AP_LeftRulerInfo();

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
		fl_DocSectionLayout * pDSL2 = NULL;
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
		bool isAnnotation = false;
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
			pDSL2 = static_cast<fl_DocSectionLayout *>(pSection);
			isFootnote = true;
			xxx_UT_DEBUGMSG(("ap_LeftRulerInfo: Found footnote at point \n"));
		}
		else if(pContainer->getContainerType() == FP_CONTAINER_ENDNOTE)
		{
			pSection = pPage->getOwningSection();
			pDSL2 = static_cast<fl_DocSectionLayout *>(pSection);
			isEndnote = true;
			xxx_UT_DEBUGMSG(("ap_LeftRulerInfo: Found footnote at point \n"));
		}
		else if(pContainer->getContainerType() == FP_CONTAINER_ANNOTATION)
		{
			pSection = pPage->getOwningSection();
			pDSL2 = static_cast<fl_DocSectionLayout *>(pSection);
			isAnnotation = true;
			xxx_UT_DEBUGMSG(("ap_LeftRulerInfo: Found footnote at point \n"));
		}
		else
		{
			pSection = pPage->getOwningSection();
			pDSL2 = static_cast<fl_DocSectionLayout*>(pSection);
		}
		pInfo->m_yPoint = yCaret - pContainer->getY();

		if ((isFootnote || isAnnotation || isEndnote || pContainer->getContainerType() == FP_CONTAINER_COLUMN) && !isHdrFtrEdit())
		{
			UT_sint32 yoff = 0;
			getPageYOffset(pPage, yoff);
			pInfo->m_yPageStart = static_cast<UT_uint32>(yoff);
			pInfo->m_yPageSize = pPage->getHeight();

			pInfo->m_yTopMargin = pDSL2->getTopMargin();
			UT_ASSERT(pInfo->m_yTopMargin>= 0);

			pInfo->m_yBottomMargin = pDSL2->getBottomMargin();
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
			xxx_UT_DEBUGMSG(("In getLeftRulerInfo pCell %x pTab %x \n",pCell,pTab));
			UT_sint32 col = pCell->getLeftAttach();
			UT_sint32 numrows = pTab->getNumRows();
			UT_sint32 i =0;
			fp_CellContainer * pCur = NULL;
			pInfo->m_vecTableRowInfo = new UT_GenericVector<AP_LeftRulerTableInfo*>();
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
					pInfo->m_vecTableRowInfo->addItem(pLInfo);
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
			fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(pFC->getSectionLayout());
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
		else if(isHdrFtrEdit())
		{
			fl_HdrFtrSectionLayout * pHF =	m_pEditShadow->getHdrFtrSectionLayout();
			pDSL2 = pHF->getDocSectionLayout();
			UT_sint32 yoff = 0;
			getPageYOffset(pPage, yoff);
			pInfo->m_yPageStart = static_cast<UT_uint32>(yoff);
			pInfo->m_yPageSize = pPage->getHeight();

			if(pHF->getHFType() >= FL_HDRFTR_FOOTER)
			{
				pInfo->m_yTopMargin = pPage->getHeight() - pDSL2->getBottomMargin();
				UT_ASSERT(pInfo->m_yTopMargin>= 0);
				pInfo->m_yBottomMargin = pDSL2->getFooterMargin();
			}
			else
			{
				pInfo->m_yTopMargin = pDSL2->getHeaderMargin();
				UT_ASSERT(pInfo->m_yTopMargin>= 0);
				pInfo->m_yBottomMargin = pPage->getHeight() - pDSL2->getTopMargin();
			}

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
	bool bBOL, bEOL,isTOC;
	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL,isTOC, true);

	return isPosSelected(pos);
}

/*!
 * Returns a pointer to the cell container surrounding the supplied point
 * Return NULL if there isn't one.
 */
fp_CellContainer * FV_View::getCellAtPos(PT_DocPosition pos) const
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
		const fl_ContainerLayout * pCL = pBlock->myContainingLayout();
		if((pCL->getContainerType() == FL_CONTAINER_FOOTNOTE) ||
		   (pCL->getContainerType() == FL_CONTAINER_ANNOTATION)||
		   (pCL->getContainerType() == FL_CONTAINER_ENDNOTE))
		{
			pBlock = pBlock->getEnclosingBlock();
			if(pBlock == NULL)
			{
				return NULL;
			}
			pCL = pBlock->myContainingLayout();
			if(pCL->getContainerType() == FL_CONTAINER_CELL)
			{
				return static_cast<fp_CellContainer *>(pCL->getFirstContainer());
			}
		}
	}
	return NULL;
}

fp_Run * FV_View::getHyperLinkRun(PT_DocPosition pos)
{
	fl_BlockLayout* pBlock =_findBlockAtPosition(pos);
	fp_Run* pRun = NULL;

	if(pBlock)
	{
		UT_uint32 blockOffset = pos - pBlock->getPosition();
		pRun = pBlock->findRunAtOffset(blockOffset);
	}
	xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(top) block:%p run:%p\n", pBlock, pRun ));
	if( pRun )
	{
		xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(top) run.t:%d\n", pRun->getType() ));
		xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(top) run.w:%d\n", pRun->getWidth() ));
		xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(top) run.hl:%p\n", pRun->getHyperlink() ));
	}
	// Make sure we found the starting pRun for an annotation which is
	// a single point in the document.
	if( pRun && pRun->getType() == FPRUN_HYPERLINK && pRun->getWidth() == 0 )
	{
		pRun = pRun->getPrevRun();
	}
	
	// 
	// If we didn't find the hyperlink run, then we might be dealing
	// with an annotation that is a single point rather than a range.
	// As such, grab the previous run and check if it is a hyperlink,
	// if so then make sure we return the run for the start of the hyperlink
	// 
	if( pRun && pRun->getType() != FPRUN_HYPERLINK && !pRun->getHyperlink() )
	{
		xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(1) run.x:%d\n", pRun->getX() ));
		xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(1) run.w:%d\n", pRun->getWidth() ));
		xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(1) run.t:%d\n", pRun->getType() ));
		xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(1) run.nl:%p\n", pRun->getHyperlink() ));
		if( pRun->getPrevRun() && pRun->getPrevRun()->getType() == FPRUN_HYPERLINK )
		{
			xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(prev) run.x:%d\n", pRun->getX() ));
			pRun = pRun->getPrevRun();
			if( pRun->getWidth() == 0 )
				pRun = pRun->getPrevRun();
		}
		else if( pRun->getNextRun() && pRun->getNextRun()->getType() == FPRUN_HYPERLINK )
		{
			xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(next) run.x:%d\n", pRun->getX() ));
			pRun = pRun->getNextRun();
			xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(next) run.type:%d\n", pRun->getType() ));
			xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(next) run.  hl:%p\n", pRun->getHyperlink() ));
		}
		
		if( pRun )
		{
			xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(2) run.x:%d\n", pRun->getX() ));
			xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(2) run.w:%d\n", pRun->getWidth() ));
			xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(2) run.t:%d\n", pRun->getType() ));
		}
		
		if( pRun && pRun->getType() != FPRUN_HYPERLINK )
		{
			xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(3 reset) run.x:%d\n", pRun->getX() ));
			pRun = 0;
		}
	}
	if(pRun && pRun->getHyperlink() != NULL)
	{
		xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(ret) run.hl:%p\n", pRun->getHyperlink() ));
		return pRun->getHyperlink();
	}
	
	xxx_UT_DEBUGMSG(("FV_View::getHyperLinkRun(end fail)\n" ));
	return NULL;
}

EV_EditMouseContext FV_View::getLastMouseContext(void)
{
	return m_prevMouseContext;
}

void  FV_View::getMousePos(UT_sint32 *x, UT_sint32 * y)
{
	*x = m_iMouseX;
	*y = m_iMouseY;
}

// #undef   xxx_UT_DEBUGMSG
// #define  xxx_UT_DEBUGMSG(x) UT_DEBUGMSG(x)

EV_EditMouseContext FV_View::getMouseContext(UT_sint32 xPos, UT_sint32 yPos)
{
	xxx_UT_DEBUGMSG(("layout view mouse pos x %d pos y %d \n",xPos,yPos));
	EV_EditMouseContext emc = _getMouseContext(xPos,yPos);

	if (isAnnotationPreviewActive() && (emc != EV_EMC_HYPERLINK))
	{
		// kill the annotation preview popup
		UT_DEBUGMSG(("getMouseContext: Deleting previous annotation preview...\n"));
		killAnnotationPreview();
	}
	if(emc == EV_EMC_HYPERLINK)
	{
		xxx_UT_DEBUGMSG(("In Hyperlink \n"));
	}
	xxx_UT_DEBUGMSG(("Mouse Context %d \n",emc));
	return emc;
}


EV_EditMouseContext FV_View::_getMouseContext(UT_sint32 xPos, UT_sint32 yPos)
{
	xxx_UT_DEBUGMSG(("layout view mouse pos x %d pos y %d \n",xPos,yPos));
	UT_sint32 xClick, yClick;
	PT_DocPosition pos = 0;
	bool bBOL = false;
	bool bEOL = false;
	bool isTOC = false;
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
	//UT_uint32 iPageNumber m_pLayout->findPage(pPage);
	//UT_uint32 iRow = iPageNumber/getNumHorizPages();
	
	
	if (!pPage)
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (1)\n"));
		m_prevMouseContext = EV_EMC_UNKNOWN;
		return EV_EMC_UNKNOWN;
	}

	if (   (yClick < 0)
		   || (xClick < 0)
		   || (xClick > static_cast<UT_sint32>(getWidthPagesInRow(pPage))) )
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (2)\n"));
		m_prevMouseContext = EV_EMC_UNKNOWN;
		return EV_EMC_UNKNOWN;
	}
	if(m_FrameEdit.isActive())
	{
		fl_FrameLayout * pFL = m_FrameEdit.getFrameLayout();
		if(pFL && (pFL->getFrameType() >= FL_FRAME_WRAPPER_IMAGE))
		{
			m_prevMouseContext = EV_EMC_POSOBJECT;
			return EV_EMC_POSOBJECT;
		}
		return EV_EMC_FRAME;
	}
	if(m_InlineImage.isActive())
	{
		xxx_UT_DEBUGMSG(("getMouseContext emc set to IMAGESIZE \n"));
			m_prevMouseContext = EV_EMC_IMAGESIZE;
			return EV_EMC_IMAGESIZE;
	}
	UT_sint32 ires = 40;
	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL,isTOC, true);
	fl_BlockLayout* pBlock;
	fp_Run* pRun;
	_findPositionCoords(pos, bEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection, &pBlock, &pRun);
	xxx_UT_DEBUGMSG(("Current Pos %d \n",pos));
//
// Look if we're inside a frame
//
	if(isInFrame(pos))
	{
		//
// Handle case of an image only as a backdrop to a frame. Then the frame
// has no content.
//
		xxx_UT_DEBUGMSG(("In Frame \n"));
		if(m_pDoc->isFrameAtPos(pos))
		{
			fl_ContainerLayout* psfh = NULL;
			m_pDoc->getStruxOfTypeFromPosition(m_pLayout->getLID(),pos+1,
											   PTX_SectionFrame, &psfh);
			fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(psfh);
			UT_ASSERT(pFL->getContainerType() == FL_CONTAINER_FRAME);
			if(pFL->getFrameType() >= FL_FRAME_WRAPPER_IMAGE)
			{
				m_prevMouseContext = EV_EMC_POSOBJECT;
				xxx_UT_DEBUGMSG(("Over positioned object \n"));
				return EV_EMC_POSOBJECT;
			}
		}
 
		//TODO: this needs fixing for multipage, I think?
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
	if(isTOC)
	{
		m_prevMouseContext = EV_EMC_TOC;
		return EV_EMC_TOC;
	}
	if(isInTable(pos))
	{
		if(!pRun)
			return EV_EMC_UNKNOWN;

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
				UT_sint32 iTopAttach = pCell->getTopAttach();
				UT_sint32 offy =0;
				UT_sint32 offx =0;
				fp_VerticalContainer * pCol = static_cast<fp_Column *>(pCell->getColumn(pLine));
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
					if(((iTop - ires) < yPos) && ((iBot+ires)> yPos))
					{
						m_prevMouseContext = EV_EMC_VLINE;
						return EV_EMC_VLINE;
					}
				}
				if((iRight - xPos < ires) && (xPos - iRight < ires))
				{
					if(((iTop - ires) < yPos) && ((iBot+ires)> yPos))
					{
						xxx_UT_DEBUGMSG(("getContext: Found right cell \n"));
						m_prevMouseContext = EV_EMC_VLINE;
						return EV_EMC_VLINE;
					}
				}
				if((iTop - yPos < 2*ires) && (yPos - iTop < 2*ires))
				{
//
// Now look to see if the cursor is over the first row of the table.
//

// TODO put in some code to indicate which control of which cell is being
// dragged. Maybe reuse topRuler stuff???
//
					if(iTopAttach == 0)
					{
						m_prevMouseContext = EV_EMC_TOPCELL;
						return EV_EMC_TOPCELL;
					}		
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
		if(pBlock->getDominantDirection() == UT_BIDI_RTL)
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

#if 0 //defined(DEBUG)
	xxx_UT_DEBUGMSG(("fv_View::getMouseContext: () Run.type:%d\n", pRun->getType() ));
	if( fp_Run* t = pRun->getNextRun() )
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: () next.type:%d\n", t->getType() ));
	}
	if( fp_Run* t = pRun->getPrevRun() )
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: () prev.type:%d\n", t->getType() ));
	}
#endif

	if( pRun )
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: () pRun.x  %ld\n", pRun->getX() ));
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: () pRun.r  %ld\n", pRun->getX() + pRun->getWidth() ));
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: () pRun.t  %ld\n", pRun->getType() ));
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: () pRun.hl %p\n",  pRun->getHyperlink() ));
	}

	//
	// Try to find the hyperlink for this mouse position. If the user
	// has clicked directly on a hyperlink or annotation then pRun
	// will already be able to produce the hyperlink. For annotations
	// that are on a single point in the document like the "(1)"
	// marker in "foo (1) bar" then we have to poke around a bit to
	// set pHyperRun properly.
	//
	fp_Run* pHyperRun = pRun;
	if(pHyperRun && pHyperRun->getHyperlink())
	{
		//
		// The hyperlink has actual text content which was clicked,
		// nothing to do.
		//
	}
	else
	{
		//
		// Sniff around for a (1) style marker before and after the
		// click point which has no contained content and can not
		// contain the point.
		//
		pHyperRun = pRun->getNextRun();
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (a) pHyperRun %p\n", pHyperRun ));
		if(pHyperRun) {
			xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (a)      type %ld\n", pHyperRun->getType() ));
		}
		if( !pHyperRun || pHyperRun->getType() != FPRUN_HYPERLINK )
		{
			pHyperRun = pRun->getPrevRun();
			xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (b) pHyperRun %p\n", pHyperRun ));
			if(pHyperRun) {
				xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (b)      type %ld\n", pHyperRun->getType() ));
			}

			if( pHyperRun && pHyperRun->getType() == FPRUN_HYPERLINK )
			{
				if( pHyperRun->getWidth() == 0 )
				{
					pHyperRun = pHyperRun->getPrevRun();
					if( pHyperRun && pHyperRun->getType() != FPRUN_HYPERLINK )
						pHyperRun = 0;
				}
			}
			else
			{
				pHyperRun = 0;
			}
		}

		//
		// As we searched around for the pHyperRun then we should check that
		// the mouse is actually over the pHyperRun we have found.
		//
		if( pHyperRun )
		{
			if( fp_Line * pLine = pHyperRun->getLine() )
			{
				std::unique_ptr<UT_Rect> pRec( pLine->getScreenRect() );
				UT_sint32 xPosAdj = xPos - pRec->left;
				xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (x), xPosAdj %ld\n", xPosAdj ));
				xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (x), yPos    %ld\n", yPos ));
				xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (x), top     %ld\n", pRec->top ));
				xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (x), bot     %ld\n", pRec->top+pRec->height ));
				xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (x), run.x   %ld\n", pHyperRun->getX() ));
				xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (x), run.r   %ld\n", pHyperRun->getX()+pHyperRun->getWidth() ));

				if( pHyperRun->getX() < xPosAdj && xPosAdj < (pHyperRun->getX() + pHyperRun->getWidth()))
				{
					// Everything is fine, pHyperRun is under the mouse.
				}
				else
				{
					// We found a run that is not under the mouse
					pHyperRun = 0;
				}
			}
		}
	}

	//
	// If we are over a hyperlink then return that context
	//
	if(pHyperRun && pHyperRun->getHyperlink() != NULL)
	{
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (7), run type %d\n", pHyperRun->getType()));
		
		if(m_prevMouseContext != EV_EMC_HYPERLINK)
		{
			UT_DEBUGMSG(("Mouse context is changed to hyperlink \n"));
		}
		fp_Line * pLine=  pHyperRun->getLine();
		if(pLine)
		{
			std::unique_ptr<UT_Rect> pRec( pLine->getScreenRect() );
			UT_DebugOnly<UT_sint32> xPosAdj = xPos - pRec->left;
			xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (7), xPosAdj %ld\n", (UT_sint32)xPosAdj ));
			xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (7), yPos    %ld\n", yPos ));
			xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (7), top     %ld\n", pRec->top ));
			xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (7), bot     %ld\n", pRec->top+pRec->height ));
			xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (7), run.x   %ld\n", pHyperRun->getX() ));
			xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (7), run.r   %ld\n", pHyperRun->getX()+pHyperRun->getWidth() ));

			if((pRec->top <= yPos) && ((pRec->top + pRec->height) >= yPos))
			{
				xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (7), HYPERLINK!\n" ));
				m_prevMouseContext = EV_EMC_HYPERLINK;
				return EV_EMC_HYPERLINK;
			}
		}
	}
	
	if(!isSelectionEmpty())
	{
		if(pRun->getType() == FPRUN_IMAGE)
		{
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
			xxx_UT_DEBUGMSG(("Set ImageSize Context \n"));

			m_prevMouseContext = EV_EMC_IMAGESIZE;
			//m_InlineImage.mouseLeftPress(xPos, yPos);
			return EV_EMC_IMAGESIZE;
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
#ifdef ENABLE_SPELL
		if (!isPosSelected(pos))
		{
			if (pBlock->getSpellSquiggles()->get(pos - pBlock->getPosition()))
			{
				xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (8) misspelt pos %d \n",pos));
				m_prevMouseContext = EV_EMC_MISSPELLEDTEXT;
				return EV_EMC_MISSPELLEDTEXT;
			}
		}
		else
#endif
		{
			xxx_UT_DEBUGMSG(("pos selected \n"));
		}
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (9) text pos %d \n",pos));
		goto handle_revisions;

	case FPRUN_IMAGE:
		{
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
		
			FV_DragWhat dragWhat = m_InlineImage.getDragWhat();
			if (dragWhat!=FV_DragNothing && dragWhat!=FV_DragWhole)
			{
				m_prevMouseContext = EV_EMC_IMAGESIZE;
				xxx_UT_DEBUGMSG(("Set ImageSize Context  \n"));
				return EV_EMC_IMAGESIZE;
			}
			else
			{
				m_prevMouseContext = EV_EMC_IMAGE;
				xxx_UT_DEBUGMSG(("Set Image Context \n"));
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
		goto handle_revisions;

	case FPRUN_FIELD:
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (11)\n"));
		m_prevMouseContext = EV_EMC_FIELD;
		return EV_EMC_FIELD;

	case FPRUN_MATH:
		m_prevMouseContext = EV_EMC_MATH;
		return EV_EMC_MATH;
	case FPRUN_EMBED:
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: EMBED \n"));
		m_prevMouseContext = EV_EMC_EMBED;
		return EV_EMC_EMBED;
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (12)\n"));
		m_prevMouseContext = EV_EMC_UNKNOWN;
		return EV_EMC_UNKNOWN;
	}

	// should  be last evaluated context, so that other context menus do not get overshadowed when
	// document history is on; this code is only reached by jump from the above switch (we really
	// need these values to be orable, so that menus can be combined)
 handle_revisions:	
	if(pRun->containsRevisions())
	{
		m_prevMouseContext = EV_EMC_REVISION;
		return EV_EMC_REVISION;
	}
	else
	{
		m_prevMouseContext = EV_EMC_TEXT;
		return EV_EMC_TEXT;
	}
	
	
	
	/*NOTREACHED*/
	UT_ASSERT(0);
	xxx_UT_DEBUGMSG(("fv_View::getMouseContext: (13)\n"));
	m_prevMouseContext = EV_EMC_UNKNOWN;
	return EV_EMC_UNKNOWN;
}


// #undef   xxx_UT_DEBUGMSG
// #define  xxx_UT_DEBUGMSG(x) 


/*!
 * Returns true if the (x,y) location on the screen is over a selected
 * fp_MathRun.
 */
bool FV_View::isMathSelected(UT_sint32 x, UT_sint32 y, PT_DocPosition  & pos) const
{
	if(isSelectionEmpty())
	{
			return false;
	}
	// Figure out which page we clicked on.
	// Pass the click down to that page.
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(x,y, xClick, yClick);
	bool bBOL = false;
	bool bEOL = false;
	bool isTOC = false;
	bool bUseHdrFtr = true;
	pPage->mapXYToPosition(false,xClick, yClick, pos, bBOL, bEOL,isTOC, bUseHdrFtr,NULL);
	fl_BlockLayout * pBlock = NULL;
	fp_Run * pRun = NULL;
	UT_sint32 xCaret, yCaret;
	UT_uint32 heightCaret;
	UT_sint32 xCaret2, yCaret2;
	bool bDirection;
	_findPositionCoords(pos, m_bPointEOL, xCaret, yCaret, xCaret2, yCaret2, heightCaret, bDirection, &pBlock, &pRun);
	if(pRun && pRun->getType() == FPRUN_MATH)
	{
		if(pos >= getPoint() && pos <= getSelectionAnchor())
			return true;
		if(pos >= getSelectionAnchor() && pos <= getPoint())
			return true;
	}  
	return false;
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

	EV_EditMouseContext evMC = getMouseContext(m_iMouseX,m_iMouseY);
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
		xxx_UT_DEBUGMSG(("Imagesize context \n"));
		if(m_InlineImage.getDragWhat() == FV_DragTopLeftCorner)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_NW;
		}
		else if(m_InlineImage.getDragWhat() ==FV_DragTopRightCorner)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_NE;
		}
		else if(m_InlineImage.getDragWhat() ==FV_DragBotLeftCorner)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_SW;
		}
		else if(m_InlineImage.getDragWhat() ==FV_DragBotRightCorner)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_SE;
		}
		else if(m_InlineImage.getDragWhat() ==FV_DragLeftEdge)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_W;
		}
		else if(m_InlineImage.getDragWhat() ==FV_DragTopEdge)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_N;
		}
		else if(m_InlineImage.getDragWhat() ==FV_DragRightEdge)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_E;
		}
		else if(m_InlineImage.getDragWhat() ==FV_DragBotEdge)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_S;
		}
		else if(!m_InlineImage.isActive())
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGE;
		}
		else
		{
			cursor = GR_Graphics::GR_CURSOR_GRAB;
		}
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
	// RIVERA
	case EV_EMC_ANNOTATIONTEXT:
		cursor = GR_Graphics::GR_CURSOR_LINK;
		break;
	case EV_EMC_ANNOTATIONMISSPELLED:
		cursor = GR_Graphics::GR_CURSOR_LINK;
		break;
	case EV_EMC_VLINE:
		UT_DEBUGMSG(("setCursor: Set to VLINE_DRAG \n"));
		cursor = GR_Graphics::GR_CURSOR_VLINE_DRAG;
		break;
	case EV_EMC_HLINE:
		cursor = GR_Graphics::GR_CURSOR_HLINE_DRAG;
		break;
	case EV_EMC_TOPCELL:
		UT_DEBUGMSG(("setCursor: Set to select Col \n"));
		cursor = GR_Graphics::GR_CURSOR_DOWNARROW;
		break;
	case EV_EMC_VISUALTEXTDRAG:
		cursor = GR_Graphics::GR_CURSOR_IMAGE;
		break;

	case EV_EMC_FRAME:
	case EV_EMC_POSOBJECT:
		if(m_FrameEdit.getFrameEditMode() == FV_FrameEdit_WAIT_FOR_FIRST_CLICK_INSERT)
		{
			cursor = GR_Graphics::GR_CURSOR_CROSSHAIR;
		}
		else if(m_FrameEdit.getDragWhat() ==FV_DragTopLeftCorner)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_NW;
		}
		else if(m_FrameEdit.getDragWhat() ==FV_DragTopRightCorner)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_NE;
		}
		else if(m_FrameEdit.getDragWhat() ==FV_DragBotLeftCorner)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_SW;
		}
		else if(m_FrameEdit.getDragWhat() ==FV_DragBotRightCorner)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_SE;
		}
		else if(m_FrameEdit.getDragWhat() ==FV_DragLeftEdge)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_W;
		}
		else if(m_FrameEdit.getDragWhat() ==FV_DragTopEdge)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_N;
		}
		else if(m_FrameEdit.getDragWhat() ==FV_DragRightEdge)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_E;
		}
		else if(m_FrameEdit.getDragWhat() ==FV_DragBotEdge)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGESIZE_S;
		}
		else if(m_FrameEdit.isActive() && m_FrameEdit.getDragWhat() ==FV_DragWhole)
		{
			cursor = GR_Graphics::GR_CURSOR_IMAGE;
		}
		else
		{
			cursor = GR_Graphics::GR_CURSOR_GRAB;
		}
		break;
	case EV_EMC_MATH:
		cursor = GR_Graphics::GR_CURSOR_IMAGE;
		break;
	case EV_EMC_EMBED:
		cursor = GR_Graphics::GR_CURSOR_IMAGE;
		break;
	default:
		break;
	}
	getGraphics()->setCursor(cursor);
}

#ifdef ENABLE_SPELL
bool FV_View::isTextMisspelled() const
{
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);
	if(pBlock == NULL)
	{
		return false;
	}
	if (!isPosSelected(pos))
		if (pBlock->getSpellSquiggles()->get(pos - pBlock->getPosition()))
		{
			return true;
		}

	return false;
}
#endif

bool FV_View::doesSelectionContainRevision() const
{
	fl_BlockLayout* pBlock;
	fp_Run* pRun;
	UT_sint32 x, y, x2, y2;
	UT_uint32 h;
	bool b;

	UT_uint32 iPos1 = UT_MIN(m_iInsPoint, getSelectionAnchor());
	UT_uint32 iPos2 = UT_MAX(m_iInsPoint, getSelectionAnchor());

	_findPositionCoords(iPos1, false, x, y, x2, y2, h, b,&pBlock, &pRun);

	if (!pBlock)
		return false;

	if (!pRun)
		return false;

	while(pBlock)
	{
		if(!pRun)
			pRun = pBlock->getFirstRun();
		
		while(pRun)
		{
			if(pRun->getBlockOffset() + pBlock->getPosition() >= iPos2)
				return false;
					
			if(pRun->containsRevisions())
				return true;

			pRun = pRun->getNextRun();
		}

		pBlock = pBlock->getNextBlockInDocument();
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
#ifdef ENABLE_SPELL
		if (!isPosSelected(m_iInsPoint))
			if (pBlock->getSpellSquiggles()->get(m_iInsPoint - pBlock->getPosition()))
			{
				return EV_EMC_MISSPELLEDTEXT;
			}
#endif
		return EV_EMC_TEXT;

	case FPRUN_IMAGE:
		{
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
			UT_DEBUGMSG(("Insertion Point Context IMAGE \n"));
			return EV_EMC_IMAGE;
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

		fl_BlockLayout * pBL = pRun->getBlock();
		fp_Page* pPointPage = NULL;
		if(pBL->isHdrFtr())
		{
			UT_DEBUGMSG(("Yikes! What page am I on? \n"));
		}
		else
		{
			pPointPage = pRun->getLine()->getPage();
		}
		return pPointPage;
	}

	return NULL;
}

/*!
 * Returns true if there is some Layout classes defined. Not true at startup.
 */
bool FV_View::isDocumentPresent(void) const
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

#ifdef ENABLE_SPELL
// ndx is one-based, not zero-based
UT_UCSChar * FV_View::getContextSuggest(UT_uint32 ndx)
{
	// locate the squiggle
	PT_DocPosition pos = 0 ;
	fl_BlockLayout* pBL = NULL ;
	fl_PartOfBlock* pPOB = NULL ;

	pos = getPoint();
	pBL = _findBlockAtPosition(pos);
	UT_return_val_if_fail(pBL,NULL);
	
	PT_DocPosition epos = 0;
	getDocument()->getBounds(true, epos);
	UT_DEBUGMSG(("end bound is %d\n", epos));
	pPOB = pBL->getSpellSquiggles()->get(pos - pBL->getPosition());
	UT_return_val_if_fail(pPOB, NULL);

	// grab the suggestion
	return _lookupSuggestion(pBL, pPOB, ndx);
}
#endif

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
FV_DocCount FV_View::countWords(bool bActuallyCountWords)
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

	fl_BlockLayout * pBL = _findBlockAtPosition(low);
	if (pBL && pBL->isEmbeddedType())
	{
		fl_EmbedLayout * pEmbedL= static_cast<fl_EmbedLayout*>(pBL->myContainingLayout()); 
		pBL = pEmbedL->getContainingBlock();
	}
	if(pBL == NULL)
	{
		return wCount;
	}
	// Selection may start inside first block
	fp_Line* pLine = static_cast<fp_Line *>(pBL->getFirstContainer());
	if(!pLine->getContainer())
	{
		return (wCount);
	}

	UT_sint32 iStartOffset = 0;
	if (low > pBL->getPosition(false))
	{
		iStartOffset = low - pBL->getPosition(false);
		if (!isSelectionEmpty() && (iStartOffset == pBL->getLength() - 1))
		{
			// If the selection begin at the complete end of a line, we want to start
			// counting at the next block. This allows to match the user perception of
			// the number of lines with the actual line count.
			// Note: the first line is visibly part of the selection only when the formatting
			// markers are shown.
			pBL = static_cast<fl_BlockLayout *>(pBL->getNextBlockInDocument());
			iStartOffset = 0;
			if (pBL) {
				pLine = static_cast<fp_Line *>(pBL->getFirstContainer());
			}
		}
		// Find first line in selection
		fp_Line* pNextLine = NULL;
		while (pLine)
		{
			pNextLine = static_cast<fp_Line*>(pLine->getNext());
			if (!pNextLine || (pNextLine->getFirstRun()->getBlockOffset() > static_cast<UT_uint32>(iStartOffset)))
			{
				break;
			}
			else
			{
				pLine = pNextLine;
			}
		}
	}

	fp_Page* pPage = pLine->getPage();
	fp_Page* pOldPage = pPage;
	wCount.page++;

	bool bFirstBlock = true;
	while (pBL)
	{
		if (pBL->getPosition(false) >= high)
		{
			break;
		}
		bool bEndInThisBlock = false;
		UT_uint32 iMaxOffset = 0;
		if (pBL->getPosition(false) + pBL->getLength() > high)
		{
			bEndInThisBlock = true;
			iMaxOffset = high - pBL->getPosition(false);
		}
		UT_GrowBuf gb(1024);
		pBL->getBlockBuf(&gb);
		const UT_UCSChar * pSpan = reinterpret_cast<UT_UCSChar*>(gb.getPointer(0));
		UT_uint32 len = gb.getLength();

		// Count lines and pages
		while (pLine && (!bEndInThisBlock || (pLine->getFirstRun()->getBlockOffset() < iMaxOffset)))
		{
			wCount.line++;

			// If this line is on a new page, increment page count
			pPage = pLine->getPage();
			if ((pOldPage != pPage) && (pOldPage->getPageNumber() < pPage->getPageNumber()))
			{
				wCount.page++;
				pOldPage = pPage;
			}

			pLine = static_cast<fp_Line *>(pLine->getNext());
		}

		if (bActuallyCountWords)
		{
			bool newWord = false;
			bool delim = true;

			UT_sint32 i = (bFirstBlock ? iStartOffset : 0);
			UT_sint32 imax = len;
			if (bEndInThisBlock)
			{
				UT_ASSERT_HARMLESS(iMaxOffset <= len);
				imax = UT_MIN(iMaxOffset,len);
			}
			fl_ContainerLayout * pNoteCL = NULL;
			UT_sint32 iNote = pBL->getEmbeddedOffset(0,pNoteCL);
			UT_sint32 iNoteEnd = -1;
			if (pNoteCL)
			{
				iNoteEnd = iNote + static_cast<fl_EmbedLayout*>(pNoteCL)->getLength() - 1;
			}
			for (; i < imax; i++)
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
				UT_UCSChar followChar, prevChar;
				followChar = (static_cast<UT_uint32>(i+1) < len) ? pSpan[i+1] : UCS_UNKPUNK;
				prevChar = i > 0 ? pSpan[i-1] : UCS_UNKPUNK;

				newWord = (delim && !UT_isWordDelimiter(pSpan[i], followChar, prevChar));

				delim = UT_isWordDelimiter(pSpan[i], followChar, prevChar);

				if (delim) { // debatable...
					if (pSpan[i]=='-' || pSpan[i]=='_') delim = false;
				}

				// CJK-FIXME: this can work incorrectly under CJK locales
				// since it can give 'true' for UCS with value >0xff (like
				// quotes, etc).
				if (newWord || XAP_EncodingManager::get_instance()->is_cjk_letter(pSpan[i]))
				{
					wCount.word++;
					wCount.words_no_notes++;
					if ((iNote >= 0) && (i >= iNote))
					{
						while ((iNote >= 0) && i > iNoteEnd)
						{
							iNote = pBL->getEmbeddedOffset(iNoteEnd + 1,pNoteCL);
							if (pNoteCL)
							{
								iNoteEnd = iNote + static_cast<fl_EmbedLayout*>(pNoteCL)->getLength() - 1;
							}
						}

						if (iNote >= 0 && (i >= iNote) && (i < iNoteEnd))
						{
							// We are inside a note
							wCount.words_no_notes--;
						}
					}
				}
			}
		}

		if (isPara)
		{
			wCount.para++;
			isPara = false;
		}

		// Get next block, we want to include footnotes so can't use getNextBlockLayout
		pBL = static_cast<fl_BlockLayout *>(pBL->getNextBlockInDocument());
		bFirstBlock = false;
		if (pBL)
		{
			pLine = static_cast<fp_Line *>(pBL->getFirstContainer());
		}
	}

	return (wCount);
}

void FV_View::setShowPara(bool bShowPara)
{
	if (bShowPara != m_bShowPara)
	{
		m_bShowPara = bShowPara;
		m_pDoc->setDontChangeInsPoint();
		// FIXME: test if next line can be safely removed
		//m_pLayout->rebuildFromHere(static_cast<fl_DocSectionLayout *>(m_pLayout->getFirstSection()));
		m_pDoc->allowChangeInsPoint();
		if(getPoint() > 0)
		{
			draw();
		}
	}
}


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
	setScreenUpdateOnGeneralUpdate(true);
	_generalUpdate();
	m_pDoc->endUserAtomicGlob(); // End the big undo block
	_updateInsertionPoint();
}


/*!
 * Remove the Header/Footer type specified by hfType
\param HdrFtrType hfType the type of the Header/Footer to be removed.
\param bool bSkipPTSaves if true don't save the PieceTable stuff
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

		// Signal PieceTable Changes have finished
		_restorePieceTableState();
		_generalUpdate();
		updateScreen (); // fix 1803, force screen update/redraw

		_updateInsertionPoint();
		m_pDoc->endUserAtomicGlob();
	}
	clearCursorWait();
}

/*!
 * Returns true if the point at at the end of a section or document and the
 * previous strux is not a block
 */
bool FV_View::isParaBreakNeededAtPos(PT_DocPosition pos) const
{

  PT_DocPosition posEOD = 0;
  getEditableBounds(true, posEOD);
  if(m_pDoc->isEndFrameAtPos(pos) && m_pDoc->isEndTableAtPos(pos-1))
  {
	  return true;
  }
  if(!m_pDoc->isSectionAtPos(pos) && !m_pDoc->isHdrFtrAtPos(pos) &&
     (pos < posEOD))
  {
	  return false;
  }
  pf_Frag * pf = m_pDoc->getFragFromPosition(pos);
  while(pf && pf->getType() != pf_Frag::PFT_Strux)
  {
	  pf = pf->getPrev();
  }
  if(pf == NULL)
  {  
	  return false;
  }
  pf_Frag_Strux * pfs2 = static_cast<pf_Frag_Strux *>(pf);
  if(pfs2->getStruxType() == PTX_EndTOC)
  {
	  return true;
  }
  if((pfs2->getStruxType() == PTX_EndFootnote) || 
     (pfs2->getStruxType() == PTX_EndAnnotation)|| 
     (pfs2->getStruxType() == PTX_EndEndnote) || 
     (pfs2->getStruxType() == PTX_Block) )
  {
	  return false;
  }
  if((pfs2->getStruxType() == PTX_Section) || (pfs2->getStruxType() == PTX_SectionHdrFtr))
  {
	  if(pfs2->getPos() < pos)
	  {
		  return true;
	  }
	  pf = pf->getPrev();
	  while(pf && pf->getType() != pf_Frag::PFT_Strux)
	  {
		  pf = pf->getPrev();
	  }
	  if(pf == NULL)
	  {  
		  return false;
	  }
	  pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
	  if((pfs->getStruxType() == PTX_EndFootnote) || 
		 (pfs->getStruxType() == PTX_EndAnnotation) || 
		 (pfs->getStruxType() == PTX_EndEndnote) || 
		 (pfs->getStruxType() == PTX_Block) )
	  {
		  return false;
	  }
  }
  return true;
}

/*!
 * Insrts a block and returns true if the point is at the end of a 
 * section or document and the
 * previous strux is not a block
 */
bool FV_View::insertParaBreakIfNeededAtPos(PT_DocPosition pos)
{
  if(!isParaBreakNeededAtPos(pos))
  {
    return false;
  }
  m_pDoc->insertStrux(pos,PTX_Block);
  return true;
}

/*!
 *	 Insert the header/footer. Save the cursor position before we do this and
 *	 restore it to where it was before we did this.
\param HdrFtrType hfType
\param bool bSkipPTSaves if true don't save the PieceTable stuff
 */
void FV_View::createThisHdrFtr(HdrFtrType hfType, bool bSkipPTSaves)
{
	setCursorWait();
	const PP_PropertyVector block_props = {
		"text-align", "left"
	};
	if(!isSelectionEmpty())
	{
		_clearSelection();
	}
	PT_DocPosition oldPos = getPoint();
	fp_Page * pPage = getCurrentPage();
	if(pPage == NULL)
	{
		clearCursorWait();
		return;
	}
	fl_DocSectionLayout * pDSLP = pPage->getOwningSection();
	fl_DocSectionLayout * pDSL = getCurrentBlock()->getDocSectionLayout();
	if(pDSL != pDSLP)
	{
		clearCursorWait();
		return;
	}
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
\param HdrFtrType hfType
\param bool bSkipPTSaves if true don't save the PieceTable stuff
 */
void FV_View::populateThisHdrFtr(HdrFtrType hfType, bool bSkipPTSaves)
{
//
// This won't work if we're not allowed to insert a HdrFtr on the current page
// Detect and abort if so.
//
	fp_Page * pPage = getCurrentPage();
	if(pPage == NULL)
	{
		return;
	}
	fl_DocSectionLayout * pDSLP = pPage->getOwningSection();
	fl_DocSectionLayout * pDSL = getCurrentBlock()->getDocSectionLayout();
	if(pDSL != pDSLP)
	{
		return;
	}

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

	if(pPage)
	{
		pDSL = pDSLP;
	}

	UT_ASSERT(pDSL->getContainerType() == FL_CONTAINER_DOCSECTION);
	fl_HdrFtrSectionLayout * pHdrFtrSrc = NULL;
	fl_HdrFtrSectionLayout * pHdrFtrDest = NULL;
	if(pDSL && (hfType < FL_HDRFTR_FOOTER))
	{
		pHdrFtrSrc = pDSL->getHeader();
	}
	else if(pDSL)
	{
		pHdrFtrSrc = pDSL->getFooter();
	}
	if(pHdrFtrSrc == NULL)
	{

	// restore updates and clean up dirty lists
		if(!bSkipPTSaves)
		{
			m_pDoc->enableListUpdates();
			m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have Ended

			m_pDoc->notifyPieceTableChangeEnd();
			m_iPieceTableState = 0;
			_generalUpdate();
			m_pDoc->endUserAtomicGlob(); // End the big undo block
			_updateInsertionPoint();
		}
		clearCursorWait();
		return;
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
		_generalUpdate();
		m_pDoc->endUserAtomicGlob(); // End the big undo block
		_updateInsertionPoint();
	}
	clearCursorWait();
}

/*!
 * This function returns true if there is a header on the current page.
\returns true if is there a header on the current page.
*/
bool FV_View::isHeaderOnPage(void) const
{
	const fp_Page * pPage = getCurrentPage();
	//UT_return_val_if_fail(pPage, false);
	if(!pPage) 
		return false;
	return (pPage->getHdrFtrP(FL_HDRFTR_HEADER) != NULL);
}

/*!
 * This function returns true if there is a footer on the current page.
\returns true if is there a footer on the current page.
*/
bool FV_View::isFooterOnPage(void) const
{
	const fp_Page * pPage = getCurrentPage();
	//UT_return_val_if_fail(pPage, false);
	if(!pPage) 
		return false;
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
fl_HdrFtrShadow *  FV_View::getEditShadow(void) const
{
	return m_pEditShadow;
}

/*!
 *	 Returns true if we're currently editting a HdrFtr section.
 */
bool FV_View::isHdrFtrEdit(void) const
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

PT_DocPosition FV_View::getSavedPosition(void) const
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
bool FV_View::needSavedPosition(void) const
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
		fl_ContainerLayout * pFirstCL = pSL->getFirstLayout();
		if(pFirstCL == NULL)
		{
			res = m_pDoc->getBounds(isEnd,posEOD);
			return res;
		}

		PT_DocPosition posFirst = pFirstCL->getPosition(true) - 1;
		PT_DocPosition posNext;
		while((pSL->getNext() != NULL) && (pSL->getNextBlockInDocument() != NULL))
		{
			pSL = static_cast<fl_SectionLayout *>(pSL->getNext());
			pFirstCL = pSL->getFirstLayout();
			// Make sure the first fl_BlockLayout of this new
			// fl_SectionLayout is valid.
			if (pFirstCL)
			{
				posNext = pFirstCL->getPosition(true) - 1;
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
		UT_return_val_if_fail(m_pEditShadow->getFirstLayout(), false);
		posEOD = m_pEditShadow->getFirstLayout()->getPosition();
		return true;
	}
	pBL = static_cast<fl_BlockLayout *>(m_pEditShadow->getLastLayout());
	UT_return_val_if_fail(pBL, false);
	posEOD = pBL->getPosition(false);
	fp_Run * pRun = pBL->getFirstRun();
	while( pRun && pRun->getNextRun() != NULL)
	{
		pRun = pRun->getNextRun();
	}
	if(pRun)
	{
		posEOD += pRun->getBlockOffset();
	}
	return true;
}


void FV_View::insertHeaderFooter(HdrFtrType hfType)
{
	const PP_PropertyVector block_props = {
		"text-align", "left"
	};
	if(!isSelectionEmpty())
	{
		_clearSelection();
	}
//
// insert the header/footer and leave the cursor in there. Set us in header/footer
//	edit mode.
//
	setCursorWait();
	UT_uint32 iPageNo = getCurrentPageNumber() - 1;

	m_pDoc->beginUserAtomicGlob(); // Begin the big undo block

	// Signal PieceTable Changes have Started
	m_pDoc->notifyPieceTableChangeStart();

	m_pDoc->disableListUpdates();

	insertHeaderFooter(block_props, hfType); // cursor is now in the header/footer
	if(isHdrFtrEdit())
		clearHdrFtrEdit();

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
	UT_return_if_fail(pHFCon);
	pShadow = pHFCon->getShadow();
	UT_ASSERT(pShadow);
//
// Set Header/footer mode and we're done! Easy :-)
//
	setHdrFtrEdit(pShadow);

	_generalUpdate();	
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();
	_fixInsertionPointCoords();
	clearCursorWait();
	notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR );
}

bool FV_View::insertHeaderFooter(const PP_PropertyVector & props, HdrFtrType hfType, fl_DocSectionLayout * pDSL)
{
//	UT_ASSERT(hfType == FL_HDRFTR_HEADER || hfType == FL_HDRFTR_FOOTER);

	/*
	  This inserts a header/footer at the end of the document,
	  and leaves the insertion point there.
	  This provides NO undo stuff.	Do it yourself.
	*/

	std::string szString;
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

	UT_return_val_if_fail(m_pDoc,false);
	UT_uint32 id = m_pDoc->getUID(UT_UniqueId::HeaderFtr);

	std::string sid = UT_std_string_sprintf("%i", id);

	const PP_PropertyVector sec_attributes1 = {
		"type", szString,
		"id", sid,
		"listid", "0",
		"parentid", "0"
	};

	const PP_PropertyVector sec_attributes2 = {
		szString, sid
	};

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
	fl_BlockLayout * pBL = pDocL->getNextBlockInDocument();
	PT_DocPosition posSec = pBL->getPosition();

	// change the section to point to the footer which doesn't exist yet.

	m_pDoc->changeStruxFmt(PTC_AddFmt, posSec, posSec, sec_attributes2, PP_NOPROPS, PTX_Section);
	PT_DocPosition iPos = _getDocPos(FV_DOCPOS_EOD);
	_setPoint(iPos); // Move to the end, where we will create the header/footer


// insert the Header/Footer
	PT_DocPosition posBlock = getPoint() +1;
	m_pDoc->insertStrux(getPoint(), PTX_SectionHdrFtr, sec_attributes1, PP_NOPROPS);

// Now the block strux for the content

	m_pDoc->insertStrux(posBlock, PTX_Block, PP_NOPROPS,
						props.empty() ? PP_PropertyVector({ "text-align", "center" }) : props);
	setPoint(posBlock+1);
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
UT_sint32 FV_View::getEmbedDepth(PT_DocPosition pos) const
{
	fl_BlockLayout * pBL =	m_pLayout->findBlockAtPosition(pos);
	if(pBL == NULL)
	{
		return 0;
	}
	fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pBL->myContainingLayout());
	bool bStop = false;
	UT_sint32 count =-1;
	while(!bStop && pCL)
	{
		count++;
		bStop = ((pCL->getContainerType() != FL_CONTAINER_FOOTNOTE) && 
				 (pCL->getContainerType() != FL_CONTAINER_ENDNOTE) && 
				 (pCL->getContainerType() != FL_CONTAINER_ANNOTATION));
		pCL = static_cast<fl_ContainerLayout *>(pCL->myContainingLayout());
	}
	return count;
}
/*!
 * This method returns the closest footnote before or that contains the 
 * requested doc position. If the is no footnote before the doc position, NULL
 * is returned.
 */
fl_FootnoteLayout * FV_View::getClosestFootnote(PT_DocPosition pos) const
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
fl_EndnoteLayout * FV_View::getClosestEndnote(PT_DocPosition pos) const
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
 * This method returns the closest annotation before or that contains the 
 * requested doc position. If the is no annnotation before the doc position, NULL
 * is returned.
 */
fl_AnnotationLayout * FV_View::getClosestAnnotation(PT_DocPosition pos) const
{
	fl_AnnotationLayout * pAL = NULL;
	fl_AnnotationLayout * pClosest = NULL;
	UT_sint32 i = 0;
	for(i = 0; i< static_cast<UT_sint32>(m_pLayout->countAnnotations());i++)
	{
		pAL = m_pLayout->getNthAnnotation(i);
		if(pAL->getDocPosition() <= pos)
		{
			if(pClosest == NULL)
			{
				pClosest = pAL;
			}
			else if( pClosest->getDocPosition() < pAL->getDocPosition())
			{
				pClosest = pAL;
			}
		}
	}
	return pClosest;
}

bool FV_View::isInHdrFtr(PT_DocPosition pos) const
{
	fl_BlockLayout * pBL = _findBlockAtPosition(pos);
	if(pBL == NULL)
	{
		return false;
	}
	fl_ContainerLayout * pCL = pBL->myContainingLayout();
	while(pCL && ((pCL->getContainerType() != FL_CONTAINER_DOCSECTION)
				  && (pCL->getContainerType() != FL_CONTAINER_HDRFTR)
				  && (pCL->getContainerType() != FL_CONTAINER_SHADOW)))
	{
		pCL = pCL->myContainingLayout();
	}
	if(pCL && ( (pCL->getContainerType() == FL_CONTAINER_HDRFTR)
				|| (pCL->getContainerType() == FL_CONTAINER_SHADOW)))
	{
		return true;
	}
	return false;
}

/*! 
 * Returns true if the point is located with a block so stuff can be typed
 * or inserted.
 */
bool FV_View::isPointLegal(PT_DocPosition pos) const
{
	pf_Frag_Strux* prevSDH = NULL;
	pf_Frag_Strux* nextSDH = NULL;
	PT_DocPosition nextPos =0;
	
//
// Special cases which would otherwise fail..
//
	if(m_pDoc->isEndFootnoteAtPos(pos))
	{
		return true;
	}
	if(m_pDoc->isFootnoteAtPos(pos))
	{
		return true;
	}
	fl_BlockLayout * pBL = _findBlockAtPosition(pos);
	if(pBL == NULL)
	{
	        return false;
	}

	if(!pBL->canContainPoint())
	{
		return false;
	}
	
	bool bres = m_pDoc->getStruxOfTypeFromPosition(pos,PTX_Block,&prevSDH);
	if(!bres)
	{
		return false;
	}
//
// Can't place point at endToc
//
	if( m_pDoc->isTOCAtPos(pos-1) && m_pDoc->isTOCAtPos(pos))
	{
		return false;
	}
	//
	// Can't place between frames and the next paragraph
	//
	if(m_pDoc->isEndFrameAtPos(pos) &&  m_pDoc->isFrameAtPos(pos-1))
	{
	  return false;
	}

 // another corner case

	if(m_pDoc->isEndTableAtPos(pos-1) && m_pDoc->isEndFrameAtPos(pos))	
	{
		return false;
	}
	if(m_pDoc->isEndFrameAtPos(pos) &&  !m_pDoc->isFrameAtPos(pos-1))
	{
	  return true; // This is how we insert into frames
	}

	if(m_pDoc->isEndFrameAtPos(pos-1) &&  m_pDoc->isFrameAtPos(pos))
	{
	  return false;
	}
	PT_DocPosition posEnd = 0;
	getEditableBounds(true, posEnd);
	if(pos > posEnd)
	{
	  return false;
	}
	if(pos == posEnd && m_pDoc->isEndFrameAtPos(pos-1))
	{
	  return false;
	}
	if((pos+1) == posEnd && m_pDoc->isEndFrameAtPos(pos))
	{
	  return false;
	}
	if(((pos+1) == posEnd) && m_pDoc->isTOCAtPos(pos-1))
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
	if((pos > pBL->getPosition(true)) && (pos <= (pBL->getPosition(true) + pBL->getLength())))
	{
		return true;
	}
	return false;
}

bool FV_View::isPointLegal(void) const
{
	return isPointLegal(getPoint());
}

/*!
 * Returns true if the current insertion point is inside a footnote.
 */
bool FV_View::isInFootnote(void) const
{
	return isInFootnote(getPoint());
}

/*!
 * Returns true if the requested position is inside a footnote.
 */
bool FV_View::isInFootnote(PT_DocPosition pos) const
{
	fl_FootnoteLayout * pFL = getClosestFootnote(pos);
	if(pFL == NULL)
	{
		return false;
	}
	if(!pFL->isEndFootnoteIn())
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
bool FV_View::isInEndnote(void) const
{
	return isInEndnote(getPoint());
}

/*!
 * Returns true if the requested position is inside a endnote.
 */
bool FV_View::isInEndnote(PT_DocPosition pos) const
{
	fl_EndnoteLayout * pFL = getClosestEndnote(pos);
	if(pFL == NULL)
	{
		return false;
	}
	if(!pFL->isEndFootnoteIn())
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
 * Returns true if the current insertion point is inside a annotation.
 */
bool FV_View::isInAnnotation(void) const
{
	return isInAnnotation(getPoint());
}

/*!
 * Returns true if the requested position is inside a endnote.
 */
bool FV_View::isInAnnotation(PT_DocPosition pos) const
{
	fl_AnnotationLayout * pAL = getClosestAnnotation(pos);
	if(pAL == NULL)
	{
		return false;
	}
	if(!pAL->isEndFootnoteIn())
	{
		return false;
	}
	if((pAL->getDocPosition() <= pos) && ((pAL->getDocPosition() + pAL->getLength()) > pos))
	{
		return true;
	}
	return false;
}

fl_AnnotationLayout * FV_View::getAnnotationLayout(UT_uint32 iAnnotation) const
{
	fl_AnnotationLayout * pAnn = 	m_pLayout->findAnnotationLayout(iAnnotation);
	return pAnn;
}


UT_uint32 FV_View::countAnnotations(void) const
{
	return m_pLayout->countAnnotations();
}


std::string FV_View::getAnnotationText(UT_uint32 iAnnotation) const
{
	std::string ret;
	if(!getAnnotationText( iAnnotation, ret ))
		ret = "";
	return ret;
}



/*!
 * Return the plain text content of the annotation specified by iAnnotaion
 * Content is returned in UT_UTF8String sText.
 * Returns true if a valid annotation was found with valid content.
 */
bool FV_View::getAnnotationText(UT_uint32 iAnnotation, std::string & sText) const
{
	fl_AnnotationLayout * pAL = getAnnotationLayout(iAnnotation);
	if(!pAL)
		return false;
	pf_Frag_Strux* sdhStart = pAL->getStruxDocHandle();
	PT_DocPosition posStart = getDocument()->getStruxPosition(sdhStart)+1; // Pos of Block o Text
	UT_GrowBuf buffer;
	fl_BlockLayout * block; 
	
	block = m_pLayout->findBlockAtPosition(posStart+1);
	fp_Run * pRun = NULL;
	while(block && (static_cast<fl_AnnotationLayout *>(block->myContainingLayout()) == pAL))
	{
			UT_GrowBuf tmp;
			block->getBlockBuf(&tmp);
			pRun = block->getFirstRun();
			while(pRun)
			{
				if(pRun->getType() == FPRUN_TEXT)
				{
					buffer.append(tmp.getPointer(pRun->getBlockOffset()),pRun->getLength());
				}
				pRun = pRun->getNextRun();
			}
			tmp.truncate(0);
			block = block->getNextBlockInDocument();
	}
	UT_UCS4String uText(reinterpret_cast<const UT_UCS4Char *>( buffer.getPointer(0)),
						buffer.getLength());
	sText = uText.utf8_str();
	return true;
}

/*!
 * Select the text that is covered by the annotation
 */
bool FV_View::selectAnnotation(fl_AnnotationLayout * pAL)
{
		pf_Frag_Strux* sdhAnn = pAL->getStruxDocHandle();
		pf_Frag_Strux* sdhEnd = NULL;
		getDocument()->getNextStruxOfType(sdhAnn,PTX_EndAnnotation, &sdhEnd);
		
		UT_return_val_if_fail(sdhEnd != NULL, false);
		//
		// Start of the text covered by the annotations
		//
		PT_DocPosition posStart = getDocument()->getStruxPosition(sdhEnd); 
		posStart++;
		fp_Run * pRun = getHyperLinkRun(posStart);
		UT_DEBUGMSG(("FV_View::selectAnnotation() pRun:%p\n", pRun ));
		UT_return_val_if_fail(pRun, false);
		pRun = pRun->getNextRun();
		while(pRun && (pRun->getType() != FPRUN_HYPERLINK))
		  pRun = pRun->getNextRun();
		UT_return_val_if_fail(pRun, false);
		UT_return_val_if_fail(pRun->getType() == FPRUN_HYPERLINK, false);
		PT_DocPosition posEnd = pRun->getBlock()->getPosition(false) + pRun->getBlockOffset();
		if(posStart> posEnd)
		  posStart = posEnd;
		setPoint(posEnd);
		_ensureInsertionPointOnScreen();
		setCursorToContext();
		cmdSelect(posStart,posEnd);
		notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR ); 

		return true;
}

/*!
 * Set the content of the annotation to the plain text supplied by
 *  UT_UTF8String sText.
 * Returns true if a valid annotation was found with valid content.
 */
bool FV_View::setAnnotationText(UT_uint32 iAnnotation, const std::string & sText)
{
	fl_AnnotationLayout * pAL = getAnnotationLayout(iAnnotation);
	if(!pAL)
		return false;
	pf_Frag_Strux* sdhStart = pAL->getStruxDocHandle();
	pf_Frag_Strux* sdhEnd = NULL;
	getDocument()->getNextStruxOfType(sdhStart,PTX_EndAnnotation, &sdhEnd);
	
	UT_return_val_if_fail(sdhEnd != NULL, false);
	PT_DocPosition posStart = getDocument()->getStruxPosition(sdhStart)+1; // Pos of Block o Text
	PT_DocPosition posEnd = getDocument()->getStruxPosition(sdhEnd); 
	//
	// First set up a glob
	//
	m_pDoc->beginUserAtomicGlob();
	_saveAndNotifyPieceTableChange();
	m_pDoc->disableListUpdates();
	//
	// Cut out current content
	//
	UT_uint32 iRealDeleteCount2;
	PP_AttrProp * pAttrProp_Before = NULL;
	m_pDoc->deleteSpan(posStart+1, posEnd, pAttrProp_Before, iRealDeleteCount2);
	//
	// Insert the new text
	//
	UT_UCS4String text(sText);
	m_pDoc->insertSpan(posStart+1, text.ucs4_str(),text.size(),pAttrProp_Before);
	m_pDoc->endUserAtomicGlob();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	_generalUpdate();
	return true;

}


/*!
 * Set the content of the annotation to the plain text supplied by
 *  UT_UTF8String sText.
 * Returns true if a valid annotation was found with valid content.
 */
bool FV_View::setAnnotationText(UT_uint32 iAnnotation, const std::string & sText, 
                                const std::string & sAuthor, const std::string & sTitle)
{
	fl_AnnotationLayout * pAL = getAnnotationLayout(iAnnotation);
	if(!pAL)
		return false;
	pf_Frag_Strux* sdhStart = pAL->getStruxDocHandle();
	pf_Frag_Strux* sdhEnd = NULL;
	getDocument()->getNextStruxOfType(sdhStart,PTX_EndAnnotation, &sdhEnd);
	
	UT_return_val_if_fail(sdhEnd != NULL, false);
	PT_DocPosition posStart = getDocument()->getStruxPosition(sdhStart)+1; // Pos of Block o Text
	PT_DocPosition posEnd = getDocument()->getStruxPosition(sdhEnd); 
	//
	// First set up a glob
	//
	m_pDoc->beginUserAtomicGlob();
	_saveAndNotifyPieceTableChange();
	m_pDoc->disableListUpdates();
	//
	// Cut out current content (if any, as "" is allowed as annotation content)
	//
	PP_AttrProp * pAttrProp_Before = NULL;
	if (posStart+1 < posEnd)
	{
		UT_uint32 iRealDeleteCount2;
		m_pDoc->deleteSpan(posStart+1, posEnd, pAttrProp_Before, iRealDeleteCount2);
	}
	//
	// Insert the new text
	//
	UT_UCS4String text(sText);
	m_pDoc->insertSpan(posStart+1, text.ucs4_str(), text.size(),pAttrProp_Before);
	//
	// Set the annotation properties
	//
	PP_PropertyVector propsAnn = {
		"annotation-author", sAuthor,
		"annotation-title", sTitle,
		"annotation-date", ""
	};
	GDate  gDate;
	g_date_set_time_t (&gDate, time (NULL));
	propsAnn[5] = UT_std_string_sprintf("%d-%d-%d",gDate.month,gDate.day,gDate.year);
	xxx_UT_DEBUGMSG((" Set Author %s Title %s posStart %d \n", sAuthor.c_str(),sTitle.c_str(),posStart));
	m_pDoc->changeStruxFmt(PTC_AddFmt, posStart, posStart, PP_NOPROPS, propsAnn, PTX_SectionAnnotation);

	m_pDoc->endUserAtomicGlob();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	_generalUpdate();
	return true;

}

std::string FV_View::getAnnotationTitle(UT_uint32 iAnnotation) const
{
	std::string ret;
	if( !getAnnotationTitle( iAnnotation, ret ))
		ret = "";
	return ret;
}


// TODO getters and setters to implement/change/add as judged necessary
bool FV_View::getAnnotationTitle(UT_uint32 iAnnotation, std::string & sTitle) const
{
	fl_AnnotationLayout * pAL = getAnnotationLayout(iAnnotation);
	if(!pAL)
		return false;
	sTitle = pAL->getTitle();
	return true;
}
bool FV_View::setAnnotationTitle(UT_uint32 iAnnotation, const std::string & sTitle)
{
	fl_AnnotationLayout * pAL = getAnnotationLayout(iAnnotation);
	if(!pAL)
		return false;
	pf_Frag_Strux* sdhAnn = pAL->getStruxDocHandle();
	PT_DocPosition posAnn = m_pDoc->getStruxPosition(sdhAnn);
	const PP_PropertyVector propsAnn = {
		"annotation-title", sTitle
	};
	m_pDoc->changeStruxFmt(PTC_AddFmt, posAnn, posAnn, PP_NOPROPS, propsAnn, PTX_SectionAnnotation);
	return true;
}
std::string FV_View::getAnnotationAuthor(UT_uint32 iAnnotation) const
{
	std::string ret;
	if(!getAnnotationAuthor(iAnnotation,ret))
		ret = "";
	return ret;
}

bool FV_View::getAnnotationAuthor(UT_uint32 iAnnotation, std::string & sAuthor) const
{
	fl_AnnotationLayout * pAL = getAnnotationLayout(iAnnotation);
	if(!pAL)
		return false;
	sAuthor = pAL->getAuthor();
	return true;
}
bool FV_View::setAnnotationAuthor(UT_uint32 iAnnotation, const std::string  & sAuthor)
{
	fl_AnnotationLayout * pAL = getAnnotationLayout(iAnnotation);
	if(!pAL)
		return false;
	pf_Frag_Strux* sdhAnn = pAL->getStruxDocHandle();
	PT_DocPosition posAnn = m_pDoc->getStruxPosition(sdhAnn);
	const PP_PropertyVector propsAnn = {
		"annotation-author", sAuthor
	};
	m_pDoc->changeStruxFmt(PTC_AddFmt, posAnn, posAnn, PP_NOPROPS, propsAnn, PTX_SectionAnnotation);
	return true;
}

/*!
 * Insert annotation number iAnnotation across the current selection.
 * The text of the annotation is contained in pStr.
 * If pstr is NULL default text is inserted.
 * If bCopy is true, the current selection is copied and placed into
 * an annotation.
 */
bool FV_View::insertAnnotation(UT_sint32 iAnnotation,
							   const std::string & sDescr,
							   const std::string & sAuthor,
							   const std::string & sTitle,
							   bool bCopy)
{
	// can only apply an Annotation to an FL_SECTION_DOC or a Table
	// TODO allow applying to empty selection (cursor position)
	fl_BlockLayout * pBlock =  _findBlockAtPosition(getPoint());
	if(pBlock == NULL)
	{
		return false;
	}
	fl_SectionLayout * pSL =  pBlock->getSectionLayout();

	if ( (pSL->getContainerType() != FL_CONTAINER_DOCSECTION) && (pSL->getContainerType() != FL_CONTAINER_CELL) )
		return false;
	if(getHyperLinkRun(getPoint()) != NULL)
	{
		return false;
	}
	if(m_FrameEdit.isActive())
	{
	        return false;
	}
//
// Do this first
//
	if(m_pDoc->isTOCAtPos(getPoint()-1))
	{
		if(getPoint() ==2 || (pSL->getPosition(true) >= (getPoint() -2)))
		{
			return false;
		}
		setPoint(getPoint()-1);
	}
	UT_GenericVector<fl_BlockLayout*>  vBlocks;

	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;

	if (m_Selection.getSelectionAnchor() < posStart)
	{
		posStart = m_Selection.getSelectionAnchor();
	}
	else
	{
		posEnd = m_Selection.getSelectionAnchor();
	}

	// Hack for bug 2940
	if (posStart <= 1) posStart=2;

	//
	// Look to see if the selection spans multiple blocks. If so, pick the
	// Block containing the largest amount of selected text.
	//
	PT_DocPosition posAnnStart;
	PT_DocPosition posAnnEnd;
	getBlocksInSelection(&vBlocks);
	if(vBlocks.getItemCount() > 1)
	{
		fl_BlockLayout * pBMax = NULL;
		UT_sint32 iMaxSize = 0;
		UT_sint32 j = 0;
		for(j=0; j<vBlocks.getItemCount(); j++)
		{
			UT_sint32 iBSize = 0;
			fl_BlockLayout * pB = vBlocks.getNthItem(j);
			iBSize = pB->getLength();
			if(j == 0)
			{
				iBSize = iBSize - (posStart - pB->getPosition(true));
			}
			else if(j == (vBlocks.getItemCount() - 1))
			{
				iBSize = posEnd - pB->getPosition(true);
			}
			if(iBSize > iMaxSize)
			{
				iMaxSize = iBSize;
				pBMax = pB;
			}
		}
		posAnnStart = pBMax->getPosition();
		posAnnEnd = pBMax->getPosition(true) + pBMax->getLength();
		if(posAnnStart < posStart)
			posAnnStart = posStart;
		if(posAnnEnd > posEnd)
			posAnnEnd = posEnd;
		posStart = posAnnStart;
		posEnd = posAnnEnd;
	}

	// the selection has to be within a single block
	// we could implement hyperlinks spaning arbitrary part of the document
	// but then we could not use <a href=> </a> in the output and
	// I see no obvious need for hyperlinks to span more than a single block
	fl_BlockLayout * pBl1 = _findBlockAtPosition(posStart);
	fl_BlockLayout * pBl2 = _findBlockAtPosition(posEnd);
//
// Only allow annotations within a single block - for now
//
	if(pBl1 != pBl2)
	{
		return false;
	}
	// Silently fail (TODO: pop up message) if we try to nest annotations or hyperlinks.
	if (_getHyperlinkInRange(posStart, posEnd) != NULL)
		return false;
//
// Under sum1 induced conditions posEnd could give the same block pointer
// despite being past the end of the block. This extra fail-safe code
// prevents this.
//
	if((pBl1->getPosition() + pBl1->getLength() -1) < posEnd)
	{
		return false;
	}
	std::string sNum = UT_std_string_sprintf("%d", iAnnotation);
	const PP_PropertyVector pAttr = {
		PT_ANNOTATION_NUMBER, sNum
	};
	//
	// First set up a glob
	//
	m_pDoc->beginUserAtomicGlob();
	_saveAndNotifyPieceTableChange();
	m_pDoc->disableListUpdates();
	//
	// Cut out current selection.
	//
	if(bCopy)
	{
		copyToLocal(posStart,posEnd);
	}
	_clearSelection();
	//
	// Now insert the Hyperlink-like field
	//

	// Now insert the annotation end run, so that we can use it as a stop
	// after inserting the start run when marking the runs in between
	// as a hyperlink
	bool bRet = m_pDoc->insertObject(posEnd, PTO_Annotation, PP_NOPROPS, PP_NOPROPS);
	if(bRet)
	{
		bRet = m_pDoc->insertObject(posStart, PTO_Annotation, pAttr, PP_NOPROPS);
	}

	//
	// We insert the annotations struxes right after the annotation start
	//
	PT_DocPosition posAnnotation = posStart+1;
	PP_PropertyVector ann_attrs = {
		"annotation-id", sNum
	};
	GDate gDate;
	g_date_set_time_t (&gDate, time (NULL));
	PP_PropertyVector propsAnn = {
		"annotation-author", sAuthor,
		"annotation-title", sTitle,
		"annotation-date", UT_std_string_sprintf("%d-%d-%d", gDate.month, gDate.day, gDate.year)
	};
	PP_PropertyVector block_atts = {
		PT_STYLE_ATTRIBUTE_NAME, "Normal"
	};
	//
	//
	// OK now insert the Annotation struxes
	//
	m_pDoc->insertStrux(posAnnotation, PTX_SectionAnnotation, ann_attrs, propsAnn);
	m_pDoc->insertStrux(posAnnotation + 1, PTX_Block, block_atts, PP_NOPROPS);
	m_pDoc->insertStrux(posAnnotation + 2, PTX_EndAnnotation, PP_NOPROPS, PP_NOPROPS);
	//
	// OK now Insert the text into the annotation strux
	//
	if(bCopy)
	{
		//
		// Paste in the selected text
		//
		_pasteFromLocalTo(posAnnotation+2);
	}
	else
	{
		UT_UCS4String sUCS4(sDescr);
		const PP_AttrProp * pSpanAP = NULL;
		const PP_AttrProp * pBlockAP = NULL;
		getAttributes(&pSpanAP,&pBlockAP,posAnnotation+2);
		bRet = m_pDoc->insertSpan(posAnnotation+2, sUCS4.ucs4_str(),sUCS4.length(),const_cast<PP_AttrProp *>(pSpanAP));

	}
	// Signal piceTable is stable again and close off the glob

	_restorePieceTableState();
	_generalUpdate();
	m_pDoc->endUserAtomicGlob();
	m_pDoc->enableListUpdates();
	fl_AnnotationLayout * pAL = getClosestAnnotation(posAnnotation+2);
	selectAnnotation(pAL);

	return true;
}

// RIVERA
void FV_View::killAnnotationPreview()
{
	UT_DEBUGMSG(("killAnnotationPreview: Deleting annotation preview...\n"));
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(getParentData());
	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Preview_Annotation * pPview
		= static_cast<AP_Preview_Annotation *>(pDialogFactory->requestDialog(	AP_DIALOG_ID_ANNOTATION_PREVIEW));
	UT_ASSERT(pPview);
    pDialogFactory->releaseDialog(pPview);
	pPview->destroy();
	setAnnotationPreviewActive(false);
}

FV_View_BubbleBlocker
FV_View::getBubbleBlocker()
{
	if(isAnnotationPreviewActive())
		killAnnotationPreview();

	UT_DEBUGMSG(("FV_View::getBubbleBlocker()\n"));
	
	FV_View_BubbleBlocker ret( this );
	return ret;
}

bool
FV_View::bubblesAreBlocked() const
{
	UT_DEBUGMSG(("bubblesAreBlocked() v:%d\n", m_bubbleBlockerCount ));
	return m_bubbleBlockerCount > 0;
}



void
FV_View::incremenetBubbleBlockerCount()
{
	m_bubbleBlockerCount++;
	UT_DEBUGMSG(("FV_View::inrremenetBubbleBlockerCount() %d\n",m_bubbleBlockerCount));
}
void
FV_View::decremenetBubbleBlockerCount()
{
	m_bubbleBlockerCount--;
	UT_DEBUGMSG(("FV_View::decremenetBubbleBlockerCount() %d\n",m_bubbleBlockerCount));
}

FV_View_BubbleBlocker::FV_View_BubbleBlocker( FV_View* pView )
	:
	m_pView( pView )
{
	if( m_pView )
		m_pView->incremenetBubbleBlockerCount();
}
FV_View_BubbleBlocker::~FV_View_BubbleBlocker()
{
	if( m_pView )
		m_pView->decremenetBubbleBlockerCount();
}
    
FV_View_BubbleBlocker&
FV_View_BubbleBlocker::operator=( const FV_View_BubbleBlocker& r )
{
    if( this != &r )
    {
		if( m_pView )
			m_pView->decremenetBubbleBlockerCount();
		
		m_pView = r.m_pView;
		
		if( m_pView )
			m_pView->incremenetBubbleBlockerCount();
	}
	return *this;
}




bool FV_View::insertFootnote(bool bFootnote)
{
	// can only insert Footnote into an FL_SECTION_DOC or a Table
	fl_BlockLayout * pBlock =  _findBlockAtPosition(getPoint());
	if(pBlock == NULL)
	{
		return false;
	}
	fl_SectionLayout * pSL =  pBlock->getSectionLayout();

	if ( (pSL->getContainerType() != FL_CONTAINER_DOCSECTION) && (pSL->getContainerType() != FL_CONTAINER_CELL) )
		return false;
	if(getHyperLinkRun(getPoint()) != NULL)
	{
		return false;
	}
	if(m_FrameEdit.isActive())
	{
	        return false;
	}
//
// Do this first
//
	if(m_pDoc->isTOCAtPos(getPoint()-1))
	{
		if(getPoint() ==2 || (pSL->getPosition(true) >= (getPoint() -2)))
		{
			return false;
		}
		setPoint(getPoint()-1);
	}

	_saveAndNotifyPieceTableChange();
	m_pDoc->beginUserAtomicGlob();
	if (!isSelectionEmpty() && !m_FrameEdit.isActive())
	{
		_deleteSelection();  // TODO use this text as content of footnote instead of simply deleting it...
	}
	else if(m_FrameEdit.isActive())
	{
	       m_FrameEdit.setPointInside();
	}
	_makePointLegal();
	const PP_AttrProp * pAP_in = getAttrPropForPoint();

	// add field for footnote reference
	// first, make up an id for this footnote.
	std::string footpid;
	UT_return_val_if_fail(m_pDoc, false);
	UT_uint32 pid = m_pDoc->getUID(bFootnote ? UT_UniqueId::Footnote : UT_UniqueId::Endnote);
	footpid = UT_std_string_sprintf("%d",pid);

	/*	Apply the character style at insertion point and insert the
		Footnote reference. */


	PT_DocPosition FrefStart = getPoint();
	PT_DocPosition FanchStart;
	PT_DocPosition FanchEnd;

	PT_DocPosition dpFT = 0;
	const PP_PropertyVector dumProps = {
		"list-tag","123"
	};
	PT_DocPosition dpBody = getPoint();
//
// This does a rebuild of the affected paragraph
//
	m_pDoc->changeStruxFmt(PTC_AddFmt, dpBody, dpBody, PP_NOPROPS, dumProps, PTX_Block);
	if (!insertFootnoteSection(bFootnote,footpid.c_str()))
	{
		m_pDoc->endUserAtomicGlob();
		_restorePieceTableState();
		return false;
	}
	dpFT = getPoint(); // Points right at the EndFootnote strux
	_setPoint(dpBody);
	FrefStart = dpBody;
	UT_DebugOnly<bool> bRet = false;
	PP_PropertyVector attrs = {
		bFootnote ? "footnote-id" : "endnote-id", footpid,
		"style", ""
	};

	if(bFootnote)
	{
		attrs[3] = "Footnote Reference";
		if (_insertField("footnote_ref", attrs) == false)
			return false;
	}
	else
	{
		attrs[3] = "Endnote Reference";
		if (_insertField("endnote_ref", attrs) == false)
			return false;
	}
	attrs.resize(2);
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

	_resetSelection();
	_setPoint(FanchStart);
	if(bFootnote)
	{
		_insertField("footnote_anchor", attrs);
	}
	else
	{
		_insertField("endnote_anchor", attrs);
	}
//
// Place a format mark before the field so we can select the field.
//
	UT_uint32 id = m_pDoc->getUID(UT_UniqueId::HeaderFtr);
	const PP_PropertyVector propListTag = {
		"list-tag", UT_std_string_sprintf("%i", id)
	};
	bRet = m_pDoc->changeSpanFmt(PTC_AddFmt, FanchStart, FanchStart, PP_NOPROPS, propListTag);

	FanchEnd = FanchStart+1;
	UT_DEBUGMSG(("insertFootnote: Inserting space after anchor field \n"));
	//insert a TAB after the anchor
	UT_UCSChar tab = UCS_TAB;
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	getAttributes(&pSpanAP,&pBlockAP,FanchStart);

	m_pDoc->insertSpan(FanchEnd, &tab, 1, const_cast<PP_AttrProp *>(pSpanAP));

	//
	// Put the character format after footnote back to its previous value
	//
	// But if we are the first thing in the document, we have no attr prop.
	if (pAP_in) {
		PP_AttrProp * pAP_after = pAP_in->createExactly(pAP_in->getAttributes(),pAP_in->getProperties());
		bRet = m_pDoc->insertFmtMark(PTC_AddFmt,FanchEnd+2,pAP_after);
		UT_ASSERT(bRet);
	}

	_setPoint(FanchEnd+1);

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

	UT_DebugOnly<bool> bWidthChange = pRun->recalcWidth();
	xxx_UT_DEBUGMSG(("run type %d, width change %d\n", pRun->getType(),bWidthChange));
	pBL->setNeedsReformat(pBL);

	pBL = _findBlockAtPosition(FanchStart);
	UT_ASSERT(pBL != 0);

	if (pBL->getFirstRun()->getNextRun())
	{
		bWidthChange = pBL->getFirstRun()->getNextRun()->recalcWidth();
		xxx_UT_DEBUGMSG(("run type %d, width change %d\n", pBL->getFirstRun()->getNextRun()->getType(),bWidthChange));
		pBL->setNeedsReformat(pBL);
	}
//
// This does a rebuild of the affected paragraph
//
	m_pDoc->changeStruxFmt(PTC_RemoveFmt, dpBody, dpBody, PP_NOPROPS, dumProps, PTX_Block);
	setScreenUpdateOnGeneralUpdate(true);
	_restorePieceTableState(); // clean up remaining measures
	_updateInsertionPoint();
	_generalUpdate();
	m_pDoc->endUserAtomicGlob();
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();
	notifyListeners(AV_CHG_MOTION | AV_CHG_ALL);
//
// Lets have a peek at the doc structure, shall we?
//
//	m_pDoc->miniDump(pBL->getStruxDocHandle(),8);
	return true;
}

bool FV_View::insertFootnoteSection(bool bFootnote,const gchar * enpid)
{
	PP_PropertyVector block_attrs = {
		bFootnote ? "footnote-id" : "endnote-id", enpid
	};
	PP_PropertyVector block_attrs2 = {
		bFootnote ? "footnote-id" : "endnote-id", enpid,
		"style", bFootnote ? "Footnote" : "Endnote" // xxx 'Footnote Body'
	};

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
	UT_DEBUGMSG(("insertFootnoteSection: about to insert footnote section at %d\n",pointBreak));
	if(bFootnote)
	{
		e |= m_pDoc->insertStrux(pointBreak, PTX_SectionFootnote, block_attrs, PP_NOPROPS);
	}
	else
	{
		e |= m_pDoc->insertStrux(pointBreak, PTX_SectionEndnote, block_attrs, PP_NOPROPS);
	}

	pointFootnote = pointBreak+1;
	UT_DEBUGMSG(("insertFootnoteSection: about to insert block for footnote section at %d \n",pointFootnote));
	e |= m_pDoc->insertStrux(pointFootnote, PTX_Block, block_attrs2, PP_NOPROPS);
	pointFootnote++;
	UT_DEBUGMSG(("insertFootnoteSection: about to insert end footnote at %d \n",pointFootnote));
	if(bFootnote)
	{
		e |= m_pDoc->insertStrux(pointFootnote, PTX_EndFootnote, block_attrs, PP_NOPROPS);
	}
	else
	{
		e |= m_pDoc->insertStrux(pointFootnote, PTX_EndEndnote, block_attrs, PP_NOPROPS);
	}
    pointFootnote++;
	_setPoint(pointFootnote);
	UT_DEBUGMSG(("insertFootnoteSection: inserted everything final point is %d \n",getPoint()));

//	m_pDoc->signalListeners(PD_SIGNAL_REFORMAT_LAYOUT);

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have Ended
	_restorePieceTableState();

	_generalUpdate();

	m_pDoc->endUserAtomicGlob(); // End the big undo block
	_updateInsertionPoint();
	UT_DEBUGMSG(("insertFootnoteSection: Finished point is %d \n",getPoint()));

	return e;
}

bool FV_View::insertPageNum(const PP_PropertyVector & props, HdrFtrType hfType)
{

	/*
	  This code implements some hardcoded hacks to insert a page number.
	  It allows you to set the properties, but nothing else.  Use that
	  to center, etc.

	  Warning: this code assumes that _insertFooter() leaves the insertion
	  point in a place where it can write the page_num field.
	*/
	PP_PropertyVector  f_attributes = {
		"type", "page_number"
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

	bResult = m_pDoc->insertObject(getPoint(), PTO_Field, f_attributes, PP_NOPROPS);

	moveInsPtTo(oldPos);	// Get back to where you once belonged.

	m_pLayout->updateLayout(); // Update document layout everywhere

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have Ended
	_restorePieceTableState();

	_generalUpdate();
	m_pDoc->endUserAtomicGlob(); // End the big undo block
	_updateInsertionPoint();
	return bResult;
}

const fp_PageSize & FV_View::getPageSize(void) const
{
	return m_pDoc->m_docPageSize;
}


UT_uint32 FV_View::calculateZoomPercentForPageWidth() const
{
	const fp_PageSize pageSize = getPageSize();
	double pageWidth = pageSize.Width(DIM_IN);

	// Verify scale as a positive non-zero number else return old zoom
	UT_uint32 iZoom = 100;
	UT_sint32 iWindowWidth =  getWindowWidth();
	if(iWindowWidth == 0)
	{
	// Get fall-back defaults for zoom from prefs
		const gchar * szZoom = NULL;
		m_pApp->getPrefsValue(XAP_PREF_KEY_ZoomPercentage,
							  static_cast<const gchar**>(&szZoom));
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
	if ( ( iWindowWidth - (2 * getPageViewLeftMargin()) ) <= 0 )
		return getGraphics()->getZoomPercentage();

	double scale = (getWindowWidth() - (2 * getPageViewLeftMargin())) /
		(pageWidth * (static_cast<double>(getGraphics()->getResolution())/
					  								   static_cast<double>(getGraphics()->getZoomPercentage()) * 100.0));
	//				  (100.0 * 100.0)));
	//
	// Fill the whole width for non-Print view
	//
	if(getViewMode() != VIEW_PRINT) 
	{
		fl_DocSectionLayout *pDSL = m_pLayout->getFirstSection();
		UT_sint32 iLeft = pDSL->getLeftMargin();
		UT_sint32 iRight = pDSL->getRightMargin();
		UT_sint32 iNormalOffset = getNormalModeXOffset();
		UT_sint32 iExtra = 72; // extra 0.5 inches for rounding errors
		xxx_UT_DEBUGMSG(("Doing extra calculation Left %d Right %d \n",iLeft,iRight));
		scale = (getWindowWidth() - 2 * getPageViewLeftMargin() + iLeft +iRight - iExtra - iNormalOffset) /
			(pageWidth * (static_cast<double>(getGraphics()->getResolution()) / 
						  static_cast<double>(getGraphics()->getZoomPercentage()) * 100.0));
	}
	return static_cast<UT_uint32>(scale * 100.0);
}

UT_uint32 FV_View::calculateZoomPercentForPageHeight() const
{

	const fp_PageSize pageSize = getPageSize();
	double pageHeight = pageSize.Height(DIM_IN);
	UT_uint32 iZoom = 100;
	UT_sint32 iWindowHeight =  getWindowHeight();
	if(iWindowHeight == 0)
	{
	// Get fall-back defaults for zoom from prefs
		const gchar * szZoom = NULL;
		m_pApp->getPrefsValue(XAP_PREF_KEY_ZoomPercentage,
							  static_cast<const gchar**>(&szZoom));
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
	if ( ( iWindowHeight - (2 * getPageViewTopMargin())) <= 0 )
		return getGraphics()->getZoomPercentage();

	double scale = (getWindowHeight() - (2 * getPageViewTopMargin())) /
		(pageHeight * (static_cast<double>(getGraphics()->getResolution()) /
		                         static_cast<double>(getGraphics()->getZoomPercentage()) * 100.0));

	return static_cast<UT_uint32>(scale * 100.0);
}

UT_uint32 FV_View::calculateZoomPercentForWholePage() const
{
	return UT_MIN(	calculateZoomPercentForPageWidth(),
					calculateZoomPercentForPageHeight());
}

/* Revision related functions */
void FV_View::toggleMarkRevisions()
{
	m_pDoc->toggleMarkRevisions();

	// force screen update to fix 7673
	updateScreen();
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

		/* have to force redraw -- see 10486 */
		draw(NULL);
		
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
	UT_return_if_fail( i <= PD_MAX_REVISION );
	m_pDoc->setShowRevisionId(i);
	m_iViewRevision = i;
}

/*!
    Revision level i means that the document is to be shown as it looks _after_ revision with id ==
    i. In non-revisioning mode, any revision level is OK, but when marking revisions legal values
    are >= current revisioning level - 1.
*/
UT_uint32 FV_View::getRevisionLevel()const
{
	if(m_iViewRevision && isMarkRevisions())
	{
		UT_uint32 iRevLevel = m_pDoc->getHighestRevisionId();

		if(!iRevLevel)
			return 0;
		
		--iRevLevel;
	
		if(m_iViewRevision < iRevLevel)
			return PD_MAX_REVISION;
	}
	
	return m_iViewRevision;
}

bool FV_View::isMarkRevisions() const
{
	return m_pDoc->isMarkRevisions();
}
/*!
    This function brings our revision settings into sync with the document-wide settings and is
    called by fl_DocListener in response to PD_SIGNAL_REVISION_MODE_CHANGED. This funciton does not
    initiate layout rebuild, that is the responsibility of the caller.
*/
void FV_View::updateRevisionMode()
{
	if(m_pDoc->isAutoRevisioning())
	{
		// when in auto-revisioning mode, we respect all document-wide settings -- basically this
		// means that the doc in all views looks like a normal document without the revision marking
		// highlighted.
		m_iViewRevision = m_pDoc->getShowRevisionId();
		m_bShowRevisions = m_pDoc->isShowRevisions();
	}

	// in non-auto mode, we ignore document wide settings -- this allows different views to show
	// different state of the document

	// make sure we have no left-over revision attribs, etc., at the insertion point
	_fixInsertionPointAfterRevision();
}



/* Table related functions */
/*!
 * returns true if the current insertion point is inside a table.
 */
bool FV_View::isInTable() const
{
	PT_DocPosition pos = getPoint();

	if (isSelectionEmpty())
	{
		return isInTable(pos);
	}
	else
	{
  		PT_DocPosition posA = getSelectionAnchor();
		return (isInTable(posA) && isInTable(pos));
	}
}

fl_TableLayout * FV_View::getTableAtPos(PT_DocPosition pos) const
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
bool FV_View::isInTable( PT_DocPosition pos) const
{
	xxx_UT_DEBUGMSG(("Look in table at pos %d \n",pos));
	if(m_pDoc->isTableAtPos(pos))
	{
		//
		// This could be the start of nested table. If so return true!
		//
		pf_Frag_Strux* sdhTable = NULL;
		bool bRes = m_pDoc->getStruxOfTypeFromPosition(pos+1,PTX_SectionTable,&sdhTable);
		UT_return_val_if_fail(bRes,false);
		UT_ASSERT(sdhTable && sdhTable->getPos() == pos);
		fl_TableLayout * pTL = static_cast<fl_TableLayout *>(m_pDoc->getNthFmtHandle(sdhTable,m_pLayout->getLID()));
		UT_return_val_if_fail(pTL,false);
		fp_TableContainer *pTable = static_cast <fp_TableContainer*>(pTL->getFirstContainer());
		if (pTable && pTable->getContainer() && 
			(pTable->getContainer()->getContainerType() == FP_CONTAINER_CELL))
		{
			return true;
		}
		return false;
	}
	if(m_pDoc->isCellAtPos(pos))
	{
		xxx_UT_DEBUGMSG(("As cell pos in table pos %d \n",pos));
		return true;
	}
	fl_BlockLayout * pBL =	m_pLayout->findBlockAtPosition(pos,true);
	xxx_UT_DEBUGMSG((" Got Bokc at pos %d looking at pos %d \n",pBL->getPosition(true),pos));
	if(!pBL)
	{
		xxx_UT_DEBUGMSG(("Not in table \n"));
		return false;
	}
	UT_ASSERT(pBL->getContainerType() == FL_CONTAINER_BLOCK);
	fl_ContainerLayout * pCL = pBL->myContainingLayout();
	if(!pCL)
	{
		xxx_UT_DEBUGMSG(("Not in table \n"));
		return false;
	}
	UT_ASSERT(pCL->getContainerType() != FL_CONTAINER_TABLE);
	xxx_UT_DEBUGMSG(("Containing Layout is %s  pos %d \n",pCL->getContainerString(),pos));
	if((pCL->getContainerType() == FL_CONTAINER_FOOTNOTE) || (pCL->getContainerType() == FL_CONTAINER_ENDNOTE) || (pCL->getContainerType() == FL_CONTAINER_ANNOTATION))
	{
		pBL = pBL->getEnclosingBlock();
		if(pBL == NULL)
		{
			return false;
		}
		pCL = pBL->myContainingLayout();
	}
	if(pCL->getContainerType() == FL_CONTAINER_CELL)
	{
		xxx_UT_DEBUGMSG(("Inside Table cell pos %d this pos %d \n",pCL->getPosition(),pos));
		fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pCL->myContainingLayout());
		if (!pTL)
		{
			return false;
		}
		return true;
	}
	pCL = pBL->getNext();
	if(pCL == NULL)
	{
		xxx_UT_DEBUGMSG(("Get Next is NULL \n"));
		return false;
	}
	xxx_UT_DEBUGMSG(("Get Next Containing Layout is %s \n",pCL->getContainerString()));
	if(pCL->getContainerType() == FL_CONTAINER_TABLE)
	{
		PT_DocPosition posTable = m_pDoc->getStruxPosition(pCL->getStruxDocHandle());
		if(posTable <= pos) // TODO CHECK THIS very carefully!!
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	pCL = pBL->getPrev();
	if(pCL == NULL)
	{
		return false;
	}
	if(pCL->getContainerType() == FL_CONTAINER_TABLE)
	{
		pf_Frag_Strux* sdh = pCL->getStruxDocHandle();
		pf_Frag_Strux* sdhEnd = m_pDoc->getEndTableStruxFromTableSDH(sdh);
		if(sdhEnd != NULL)
		{
			PT_DocPosition posEnd =  m_pDoc->getStruxPosition(sdhEnd);
			if(posEnd == pos)
			{
				xxx_UT_DEBUGMSG(("Exactly at end of table \n"));
				return true;
			}
		}
	}
	xxx_UT_DEBUGMSG(("Last Not in table \n"));
	return false;
}

/*!
 * Returns the position of the cell strux of cell specified by (row,col) within the
 * Table surrounding the supplied point.
 */
PT_DocPosition FV_View::findCellPosAt(PT_DocPosition posTable, UT_sint32 row, UT_sint32 col) const
{
	pf_Frag_Strux* cellSDH,*tableSDH;
	bool bRes = m_pDoc->getStruxOfTypeFromPosition(posTable,PTX_SectionTable,&tableSDH);
	if(!bRes)
	{
		return 0;
	}
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(m_pDoc->getNthFmtHandle(tableSDH,m_pLayout->getLID()));
	fp_TableContainer * pTC = static_cast<fp_TableContainer *>(pTL->getFirstContainer());
//
// This is MUCH faster than linearly searching through the Piecetable.
//
	if(pTC != NULL)
	{
		fp_CellContainer * pCell = pTC->getCellAtRowColumn(row,col);
		if(pCell)
		{
			fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pCell->getSectionLayout());
			if(pCL)
			{
				return pCL->getPosition(true);
			}
		}
	}
	cellSDH = m_pDoc->getCellSDHFromRowCol(tableSDH, isShowRevisions(), getRevisionLevel(), row,col);
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
void FV_View:: getVisibleDocumentPagesAndRectangles(UT_GenericVector<UT_Rect*> &vRect, 
													UT_GenericVector<fp_Page*> &vPages) const
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


			vPages.addItem(pPage);

			// now create the rectangle
			// NB the adjustedTop is relative to the screen, but we
			// want the rect to be relative to the top left page
			// corner

			UT_sint32 iLeftGrayWidth = getPageViewLeftMargin() - m_xScrollOffset;
			UT_uint32 iPortTop       = adjustedTop >= 0 ? 0 : -adjustedTop;
			UT_uint32 iPortLeft      = iLeftGrayWidth >= 0 ? 0 : -iLeftGrayWidth;
			UT_uint32 iWindowWidth   = getWindowWidth() - iLeftGrayWidth > 0 ? getWindowWidth() - iLeftGrayWidth : 0;
			UT_uint32 iPortHeight = 0;
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
			{
				UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
			}
			
			
			UT_uint32 iPortWidth = UT_MIN(static_cast<UT_uint32>(iPageWidth), iWindowWidth);

			UT_Rect * pRect = new UT_Rect(iPortLeft,
										  iPortTop,
										  iPortWidth,
										  iPortHeight);

			vRect.addItem(pRect);
		}

		curY += iPageHeight + getPageViewSep();

		pPage = pPage->getNext();
		UT_sint32 iPage = m_pLayout->findPage(pPage);
		if(iPage < 0)
			break;
	}
}

/*! Returns the size of the image selection boxes
*/
UT_sint32 FV_View::getImageSelInfo() const
{
	return getGraphics()->tlu(m_InlineImage.getImageSelBoxSize());
}

GR_Graphics::Cursor FV_View::getImageSelCursor() const
{
	return m_imageSelCursor;
}

/*!
  Check that an image is currently selected
  
  \return true if an image is selected otherwise false.
  \todo eventually make it faster by not fetching the image data ID.
 */
bool FV_View::isImageSelected(void) const
{
	const char * dataId;
	PT_DocPosition pos = getSelectedImage(&dataId);

	if (pos == 0) {
		return false;
	}
	else { 
		return true;
	}
	return false;
}

#ifdef ENABLE_SPELL
SpellChecker * FV_View::getDictForSelection () const
{
	SpellChecker * checker = NULL;
	std::string lang;

	PP_PropertyVector props_in;
	if (getCharFormat(props_in))
	{
		lang = PP_getAttribute("lang", props_in);
	}

	if (!lang.empty())
	{
		// we get smart and request the proper dictionary
		checker = SpellManager::instance().requestDictionary(lang.c_str());
	}
	else
	{
		// we just (dumbly) default to the last dictionary
		checker = SpellManager::instance().lastDictionary();
	}

	return checker;
}
#endif

/*
   There are some 'document' properties that are specific for a given
   view, but which should be transferred into the document at point of save.
   For example, whether the document is displayed in Normal view / Web
   view, etc, or whether the document should be layout in visual or
   logical order

*/
PP_PropertyVector FV_View::getViewPersistentProps() const
{
	PP_PropertyVector pProps;
	if(m_eBidiOrder == FV_Order_Logical_LTR)
	{
		pProps.push_back("dom-dir");
		pProps.push_back("logical-ltr");
	}
	else if(m_eBidiOrder == FV_Order_Logical_RTL)
	{
		pProps.push_back("dom-dir");
		pProps.push_back("logical-rtl");
	}
	return pProps;
}

void FV_View::rebuildLayout()
{
	m_pLayout->rebuildFromHere(static_cast<fl_DocSectionLayout *>(m_pLayout->getFirstSection()));	
}

fv_PropCache::fv_PropCache(void):
	m_iTick(0),
	m_pCurrentCL(NULL)
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

void fv_PropCache::setCurrentCL(fl_ContainerLayout* pCL)
{
	m_pCurrentCL = pCL;
}

fl_ContainerLayout* fv_PropCache::getCurrentCL(void) const
{
	return m_pCurrentCL;
}

void fv_PropCache::fillProps(const PP_PropertyVector & props)
{
	m_props = props;
}

void fv_PropCache::clearProps(void)
{
	m_props.clear();
}

/*!
    This method forces remeasuring of widths for all characters in the document.
    It is called on zoom to allow us to adjust positioning of individual chars in response
    to changes to metrics of screen font.

    NB: this function does not force rebuild; on the zoom the actual character sizes
    remain the same, and so does the overall layout.
*/
void FV_View::remeasureCharsWithoutRebuild()                                                  
{                                                                               
    fl_BlockLayout * pBL = getBlockAtPosition(2);

    while(pBL)
    {
        fp_Run * pRun = pBL->getFirstRun();

		while(pRun)
        {
			if(pRun->getType() == FPRUN_TEXT)
			{
				fp_TextRun * pTR = (fp_TextRun*) pRun;
				pTR->measureCharWidths();
			}
			
            pRun = pRun->getNextRun();
        }
        pBL = pBL->getNextBlockInDocument();
    }

	updateLayout();
}

/*!
    This function is called when the font metrics for the view change. This happens for
    example when on win32 the user changes the currently selected printer. In order to
    maintain WYSIWYG behaviour, we have to remeasure and rebuild
*/
void FV_View::fontMetricsChange()
{
    fl_BlockLayout * pBL = getBlockAtPosition(2);

    while(pBL)
    {
        fp_Run * pRun = pBL->getFirstRun();

		while(pRun)
        {
			// The order here matters; marking width dirty before call to
			// updateVerticalMetric() allows some fp_Run subclasses to clear the width
			// flag in the updateVerticalMetric() call (see fp_EmbedRun for example)
			
			pRun->markWidthDirty();  // width will be recalculated during rebuild
			pRun->updateVerticalMetric();
            pRun = pRun->getNextRun();
        }
        pBL = pBL->getNextBlockInDocument();
    }

	m_pLayout->rebuildFromHere(static_cast<fl_DocSectionLayout *>(m_pLayout->getFirstSection()));	
}

UT_uint32 FV_View::getNumHorizPages() const
{
	if(!getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
		return 1;
	return m_iNumHorizPages;
}

void FV_View::calculateNumHorizPages()
{
	UT_sint32 scrollbarWidth = 1000; //Because my scrollbar is about this wide.
	UT_sint32 windowWidth = getWindowWidth() - scrollbarWidth;
	UT_sint32 iOldNo = m_iNumHorizPages;
	if(windowWidth < 0)
	{
		m_iNumHorizPages = 1;
		return;
	}
	if(!getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		m_iNumHorizPages = 1;
		return;
	}
	xxx_UT_DEBUGMSG(("Initial Number horizontal pages %d \n",iOldNo));
	if (!m_autoNumHorizPages || getViewMode() != VIEW_PRINT || m_iNumHorizPages < 1)
	{
		m_iNumHorizPages = 1; //TODO: get this from the default or prefrences.
	}
	else if (m_iNumHorizPages > 20) //20 seems like a reasonable max...
	{
		m_iNumHorizPages = 20;
	}
	else if(getWindowWidth() < m_pLayout->getFirstPage()->getWidth())
	{
		m_iNumHorizPages = 1;
	}
	else //Find the new m_iNumPages
	{
		m_getNumHorizPagesCachedWindowWidth = windowWidth;
		UT_sint32 widthPagesInRow = 0;
		UT_sint32 iFirstPageInRow = 0; //iRow * m_iNumHorizPages; //TODO:make this use the row of the current page.
		fp_Page * pPage = m_pLayout->getNthPage(iFirstPageInRow);
		
		widthPagesInRow = getWidthPagesInRow(pPage);
		
		if ( (windowWidth < widthPagesInRow) && (m_iNumHorizPages > 1) )
		{
			do
			{
				m_iNumHorizPages--;
				widthPagesInRow = getWidthPagesInRow(pPage);
			}while ((windowWidth < widthPagesInRow) && (m_iNumHorizPages > 1));
		}
		else if ((windowWidth > widthPagesInRow) && (widthPagesInRow + pPage->getWidth() + static_cast<UT_sint32>(getHorizPageSpacing()) < windowWidth))
		{
			do
			{
				m_iNumHorizPages++;
				widthPagesInRow = getWidthPagesInRow(pPage);
			} while ((windowWidth > widthPagesInRow) && (widthPagesInRow + static_cast<UT_sint32>(pPage->getWidth()) + static_cast<UT_sint32>(getHorizPageSpacing()) < windowWidth) && (static_cast<UT_sint32>(m_iNumHorizPages) <= m_pLayout->countPages()));
		}
		xxx_UT_DEBUGMSG(("m_iNumHorizPages %d | windowWidth %d | widthPagesInRow %d | pPage->getWidth() %d\n", m_iNumHorizPages,windowWidth,widthPagesInRow,pPage->getWidth()));
	}
	if (m_iNumHorizPages > 20) //20 seems like a reasonable max...
	{
		m_iNumHorizPages = 20;
	}
	if(static_cast<UT_sint32>(m_iNumHorizPages) > m_pLayout->countPages())
	{
		m_iNumHorizPages = m_pLayout->countPages();
	}
	if (m_iNumHorizPages > 1)
	{
		XAP_App::getApp()->setEnableSmoothScrolling(false);
	}
	else
	{
		XAP_App::getApp()->setEnableSmoothScrolling(true);
	}
	if(iOldNo != static_cast<UT_sint32>(m_iNumHorizPages))
	{
		UT_uint32 iPrevYOffset = m_yScrollOffset;
		XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());
		pFrame->setYScrollRange();
		pFrame->nullUpdate();
		pFrame->nullUpdate();
		double totOffset = static_cast<double>(iPrevYOffset)*static_cast<double>(iOldNo);
		xxx_UT_DEBUGMSG(("Number horizontal pages changed totoffset %f \n",totOffset));
		UT_uint32 newOffset = static_cast<UT_uint32>(totOffset / static_cast<double>(m_iNumHorizPages));
		UT_sint32 idiff = newOffset -  m_yScrollOffset;

		if(idiff > 0)
		{
			cmdScroll(AV_SCROLLCMD_LINEDOWN,idiff);
		}
		else
		{
			cmdScroll(AV_SCROLLCMD_LINEUP,-idiff);
		}
		pFrame->nullUpdate();
		pFrame->nullUpdate();
		_ensureInsertionPointOnScreen();
	}
	return;
}

UT_uint32 FV_View::getMaxHeight(UT_uint32 iRow) const
{
	fp_Page * pPage = m_pLayout->getNthPage(iRow * getNumHorizPages());
	if(!pPage)
	{
		pPage = m_pLayout->getNthPage(0);
	}
	UT_sint32 iMaxPageHeight = 0;
	if(!pPage)
	{
		fl_DocSectionLayout * pDSL =m_pLayout->getFirstSection();
		iMaxPageHeight = pDSL->getMaxSectionColumnHeight();
		if(getViewMode() == VIEW_PRINT)
		{
				iMaxPageHeight += (pDSL->getTopMargin() +  pDSL->getBottomMargin());
		}
		return iMaxPageHeight;
	}
	fl_DocSectionLayout * pDSL = pPage->getOwningSection();
	
	for(unsigned int i = 0; i < getNumHorizPages(); i++)
	{
		UT_sint32 iPageHeight = pPage->getHeight();
		if(getViewMode() != VIEW_PRINT)
		{
			iPageHeight = iPageHeight - pDSL->getTopMargin() - pDSL->getBottomMargin();
		}
		if (iPageHeight > iMaxPageHeight)
		{
			iMaxPageHeight = iPageHeight;
		}
		
		if(pPage->getNext())
		{	
			pPage = pPage->getNext();
		}
		else
		{
			break;
		}
	}
	
	
	return iMaxPageHeight;
}

UT_uint32 FV_View::getWidthPrevPagesInRow(UT_uint32 iPageNumber) const
{
	if (getNumHorizPages() == 1)
	{
		return 0;
	}
	
	UT_sint32 totalWidth = 0;
	UT_sint32 iRow = iPageNumber/getNumHorizPages(); //yay truncation.
	UT_sint32 iFirstPageInRow = 0;
	UT_sint32 diff = 0; //diff between current & prev pages in row
	
	if(!rtlPages())
	{
		iFirstPageInRow = iRow * getNumHorizPages();
		diff = iPageNumber - iFirstPageInRow;
	}
	else
	{
		iFirstPageInRow = (iRow * getNumHorizPages()) + (getNumHorizPages() -1);
		diff = iFirstPageInRow - iPageNumber;
	}
	if(diff < 0)
		diff = 0;
	if (iFirstPageInRow != static_cast<UT_sint32>(iPageNumber))
	{
		fp_Page * pPage = 0;
		
		if (m_pLayout->getNthPage(iFirstPageInRow))
		{
			pPage = m_pLayout->getNthPage(iFirstPageInRow);
			for (UT_sint32 i = 0; i < diff; i++)
			{
				totalWidth += getHorizPageSpacing() + pPage->getWidth();
				
				if (pPage->getNext())
				{
					pPage = pPage->getNext();
				}
				else
				{
					break;
				}
			}
		}
	}
	return totalWidth;
}

UT_uint32 FV_View::getWidthPagesInRow(fp_Page *page) const
{
	UT_sint32 iPageNumber	= m_pLayout->findPage(page);
	if(iPageNumber < 0)
	{
		fp_Page * pPage = m_pLayout->getFirstPage();
		if(pPage)
			return pPage->getWidth();
		else
			return m_pLayout->getFirstSection()->getWidth();
	}	
	fp_Page * pPage = m_pLayout->getNthPage(iPageNumber);
	UT_uint32 iRow = iPageNumber/getNumHorizPages();
	UT_uint32 iLastPageInRow = 0;
	
	if(!rtlPages())
	{
		iLastPageInRow = iRow * getNumHorizPages() + (getNumHorizPages() - 1);
	}
	else
	{
		iLastPageInRow = iRow * getNumHorizPages();
	}
	
	return (getWidthPrevPagesInRow(iLastPageInRow) + pPage->getWidth());
}

UT_uint32 FV_View::getHorizPageSpacing() const
{
	//TODO: Make this change depending on the amount of free space when m_autoNumHorizPages == false
	return m_horizPageSpacing;
}

bool FV_View::rtlPages() const
{
	/*bool bRTL; //This doesn't seem to be working.
	XAP_App::getApp()->getPrefsValueBool(static_cast<const gchar *>(AP_PREF_KEY_DefaultDirectionRtl), &bRTL);
	return bRTL;*/
	
	return FALSE;
}

void
FV_View::selectRange( PT_DocPosition start, PT_DocPosition end )
{
	PT_DocPosition prev_start, prev_end;

	prev_start = m_Selection.getSelectionLeftAnchor();
	prev_end = m_Selection.getSelectionRightAnchor();

	if (prev_start == start && prev_end == end)
		return;

	_clearSelection(false);
	_setPoint(start);
	m_Selection.setSelectionLeftAnchor(start);
	_setSelectionAnchor();
	setPoint(end);
	m_Selection.setSelectionRightAnchor(end);

	_drawBetweenPositions(MIN (prev_start, start), MAX (prev_end, end));
	_updateSelectionHandles();
}

void
FV_View::selectRange( const std::pair< PT_DocPosition, PT_DocPosition >& range )
{
	selectRange( range.first, range.second );
}

/*!
  Reverse the direction of the current selection
  Does so without changing the screen.
*/
void FV_View::swapSelectionOrientation(void)
{
	UT_ASSERT(!isSelectionEmpty());
	_fixInsertionPointCoords();
	PT_DocPosition curPos = getPoint();
	UT_ASSERT(curPos != m_Selection.getSelectionAnchor());
	_setPoint(m_Selection.getSelectionAnchor());
	m_Selection.setSelectionAnchor(curPos);
}

