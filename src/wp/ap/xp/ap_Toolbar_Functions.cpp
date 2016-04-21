/* -*- mode: C++; tab-width: 4; c-basic-offset: 4;  indent-tabs-mode: t -*- */
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ap_Features.h"

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_units.h"
#include "ut_debugmsg.h"

#include "ap_Toolbar_Id.h"
#include "ap_Toolbar_Functions.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Labels.h"
#include "xap_App.h"
#include "xap_Prefs.h"
#include "xap_Clipboard.h"
#include "xap_Frame.h"
#include "fv_View.h"
#include "gr_Graphics.h"
#include "fl_AutoNum.h"
#include "fl_BlockLayout.h"
#include "ap_Prefs_SchemeIds.h"
#include "ap_FrameData.h"
#include "pd_Document.h"
#include "pd_DocumentRDF.h"
#include "ut_Script.h"

#ifdef ENABLE_SPELL
#include "spell_manager.h"
#endif

#include "ap_EditMethods.h"
#include "fp_TableContainer.h"

#define ABIWORD_VIEW  	FV_View * pView = static_cast<FV_View *>(pAV_View)

#if 0

static bool s_ToolbarFunctions_check_inc_load(FV_View * pView)
{
  // todo: we probably need to make this function smarter.
  // see ap_EditMethods.cpp around line 1024

  //XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
	UT_DEBUGMSG(("pView = %x \n",pView));
	if(pView && ((pView->getPoint() == 0) || pView->isLayoutFilling()))
    {
		return true;
    }

	return false ;
}

#define CHECK_INC_LOAD if(s_ToolbarFunctions_check_inc_load(pView)) return EV_TIS_Gray;
#else
#define CHECK_INC_LOAD
#endif

/****************************************************************/

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_ScriptsActive)
{
  UT_UNUSED(pAV_View);
  UT_UNUSED(id);
  UT_UNUSED(pszState);

  EV_Toolbar_ItemState s = EV_TIS_ZERO;

  UT_ScriptLibrary * instance = UT_ScriptLibrary::instance ();
  UT_uint32 filterCount = instance->getNumScripts ();

  if ( filterCount == 0 )
    s = EV_TIS_Gray;

  return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_BookmarkOK)
{
	ABIWORD_VIEW;
	CHECK_INC_LOAD;
	UT_UNUSED(id);
	UT_UNUSED(pszState);

	EV_Toolbar_ItemState s = EV_TIS_ZERO;

	if(pView->isTOCSelected())
	{
	    s = EV_TIS_Gray ;
		return s;
	}
	PT_DocPosition posStart = pView->getPoint();
	PT_DocPosition posEnd = pView->getSelectionAnchor();
	fl_BlockLayout * pBL1 = pView->getBlockAtPosition(posStart);
	fl_BlockLayout * pBL2 = pView->getBlockAtPosition(posEnd);
	if((pBL1 == NULL) || (pBL2 == NULL)) // make sure we get valid blocks from selection beginning and end
	{
		s = EV_TIS_Gray;
		return s;
	}
	if(pBL1 != pBL2) // don't allow Insert bookmark if selection spans multiple blocks
	{
	    s = EV_TIS_Gray ;
		return s;
	}

	return s ;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_HyperlinkOK)
{
	ABIWORD_VIEW ;
	CHECK_INC_LOAD;
	UT_UNUSED(id);
	UT_UNUSED(pszState);
	UT_return_val_if_fail (pView, EV_TIS_Gray);

	EV_Toolbar_ItemState s = EV_TIS_ZERO;
	if ( pView->isSelectionEmpty())
	  {
		  if(pView->getHyperLinkRun(pView->getPoint()) == NULL)
		  {
			  s = EV_TIS_Gray ;
			  return s;
		  }
		  return s;
	  }
	if(pView->isTOCSelected())
	{
	    s = EV_TIS_Gray ;
		return s;
	}
	PT_DocPosition posStart = pView->getPoint();
	PT_DocPosition posEnd = pView->getSelectionAnchor();
	fl_BlockLayout * pBL1 = pView->getBlockAtPosition(posStart);
	fl_BlockLayout * pBL2 = pView->getBlockAtPosition(posEnd);
	if((pBL1 == NULL) || (pBL2 == NULL)) // make sure we get valid blocks from selection beginning and end
	{
		s = EV_TIS_Gray;
		return s;
	}
	if(pBL1 != pBL2) // don't allow Insert Hyperlink if selection spans multiple blocks
	{
	    s = EV_TIS_Gray ;
		return s;
	}
	if(pBL1->getLength() == 1)
	{
	    s = EV_TIS_Gray ;
	    return s;
	}

	if(UT_MIN(posStart,posEnd) < pBL1->getPosition(true))
	{
	    s = EV_TIS_Gray ;
	    return s;
	}


	return s ;
}

#ifdef ENABLE_SPELL
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Spelling)
{
	//ABIWORD_VIEW;
  CHECK_INC_LOAD;
  UT_UNUSED(pAV_View);
  UT_UNUSED(id);
  UT_UNUSED(pszState);

  EV_Toolbar_ItemState s = EV_TIS_ZERO;

  XAP_Prefs *pPrefs = XAP_App::getApp()->getPrefs();
  UT_return_val_if_fail ( pPrefs, EV_TIS_Gray );

  bool b = true ;
  pPrefs->getPrefsValueBool(static_cast<const gchar *>(AP_PREF_KEY_AutoSpellCheck),&b) ;

  // if there are no loaded dictionaries and we are spell checking
  // as we type
  if ( SpellManager::instance ().numLoadedDicts() == 0 && b )
    s = EV_TIS_Gray;

  // either have a loaded dictionary or want to spell-check manually. allow

  return s;
}
#endif

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Changes)
{
	ABIWORD_VIEW;
	CHECK_INC_LOAD;

	EV_Toolbar_ItemState s = EV_TIS_ZERO;
	
	if(!pView)
	{
		return s;
	}
	if (pszState)
		*pszState = NULL;

	switch (id)
	{
	case AP_TOOLBAR_ID_FILE_SAVE: // see bug 7580
	  if (!pView->getDocument()->isDirty() /*|| !pView->canDo(true)*/ /*|| pView->getDocument()->getFilename() == NULL */)
	    s = EV_TIS_Gray;
	  break;
	case AP_TOOLBAR_ID_EDIT_UNDO:
		if (!pView->canDo(true))
			s = EV_TIS_Gray;
		break;

	case AP_TOOLBAR_ID_EDIT_REDO:
		if (!pView->canDo(false))
			s = EV_TIS_Gray;
		break;

	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Selection)
{
	ABIWORD_VIEW;
	CHECK_INC_LOAD;

	if (pszState)
		*pszState = NULL;

	EV_Toolbar_ItemState s = EV_TIS_ZERO;

	switch (id)
	{
	case AP_TOOLBAR_ID_EDIT_CUT:
	case AP_TOOLBAR_ID_EDIT_COPY:
	case AP_TOOLBAR_ID_FMT_HYPERLINK:
		if (pView->isSelectionEmpty())
			s = EV_TIS_Gray;
		break;

	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Clipboard)
{
	ABIWORD_VIEW;

	if (pszState)
		*pszState = NULL;

	EV_Toolbar_ItemState s = EV_TIS_ZERO;

	switch (id)
	{
	case AP_TOOLBAR_ID_EDIT_PASTE:
		s = ((XAP_App::getApp()->canPasteFromClipboard()) ? EV_TIS_ZERO : EV_TIS_Gray );
		break;

	case AP_TOOLBAR_ID_FMTPAINTER:
	  if (pView && XAP_App::getApp()->canPasteFromClipboard() &&
	       !pView->isSelectionEmpty() && !pView->getDocument()->areStylesLocked())
	    s = EV_TIS_ZERO;
	  else
	    s = EV_TIS_Gray;
	  break;

	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}


Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_HdrFtr)
{
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_TIS_Gray);

	if (pszState)
		*pszState = NULL;

	EV_Toolbar_ItemState s = EV_TIS_ZERO;


	switch (id)
	{
	case AP_TOOLBAR_ID_EDIT_REMOVEHEADER:
		if (!pView->isHeaderOnPage())
		  s = EV_TIS_Gray;
		break;

	case AP_TOOLBAR_ID_EDIT_REMOVEFOOTER:
		if (!pView->isFooterOnPage())
		  s = EV_TIS_Gray;
		break;
	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Style)
{
	ABIWORD_VIEW;
	CHECK_INC_LOAD;

	EV_Toolbar_ItemState s = EV_TIS_ZERO;

	switch (id)
	{
	case AP_TOOLBAR_ID_FMT_STYLE:
		{
			const gchar * sz = NULL;
			if (!pView->getStyle(&sz))
			{
				static const char * sz2 ="None";
				*pszState = sz2;
				s = EV_TIS_UseString;
			}

			if (sz)
			{
				static const char * sz2 = "None";
				sz2 = sz;
				*pszState = sz2;
				s = EV_TIS_UseString;
			}
			else
			{
				static const char * sz2 ="None";
				*pszState = sz2;
				s = EV_TIS_UseString;
			}
			break;
		}

	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	return s;
}


Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Bullets)
{
	ABIWORD_VIEW;
	CHECK_INC_LOAD;
	UT_UNUSED(id);
	UT_UNUSED(pszState);

	EV_Toolbar_ItemState s = EV_TIS_ZERO;

	if(pView->isHdrFtrEdit()  || pView->isInHdrFtr(pView->getPoint()))
	{
		return EV_TIS_Gray;
	}

	fl_BlockLayout * pBlock = pView->getCurrentBlock();
	UT_return_val_if_fail (pBlock, EV_TIS_Gray);
	if(pBlock->isListItem() == false)
		return s;
	if(pBlock->getListType() == BULLETED_LIST)
		s = EV_TIS_Toggled;
	return s;
}


Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Numbers)
{
	ABIWORD_VIEW;
	CHECK_INC_LOAD;
	UT_UNUSED(id);
	UT_UNUSED(pszState);

	EV_Toolbar_ItemState s = EV_TIS_ZERO;
	if(pView->isHdrFtrEdit()  || pView->isInHdrFtr(pView->getPoint()))
	{
		return EV_TIS_Gray;
	}

	fl_BlockLayout * pBlock = pView->getCurrentBlock();
    UT_return_val_if_fail (pBlock, EV_TIS_Gray);
	if(pBlock->isListItem() == false)
	        return s;
	if(pBlock->getListType() == NUMBERED_LIST)
	        s = EV_TIS_Toggled;
        return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Indents)
{
	ABIWORD_VIEW;
	UT_UNUSED(pszState);

	EV_Toolbar_ItemState s = EV_TIS_ZERO;

	double margin_left = 0., margin_right = 0., allowed = 0.,
		page_margin_left = 0., page_margin_right = 0.,
		page_margin_top = 0., page_margin_bottom = 0.;

	s_getPageMargins(pView, margin_left, margin_right,
					 page_margin_left, page_margin_right,
					 page_margin_top, page_margin_bottom);

	UT_BidiCharType iBlockDir = UT_BIDI_LTR;
	if(pView->getCurrentBlock())
		iBlockDir = pView->getCurrentBlock()->getDominantDirection();

	switch(id)
	{
		case AP_TOOLBAR_ID_INDENT:
			allowed = pView->getPageSize().Width (DIM_IN) - page_margin_left - page_margin_right;
			if (margin_left >= allowed)
				s = EV_TIS_Gray;
			break;
		case AP_TOOLBAR_ID_UNINDENT:
			allowed = iBlockDir == UT_BIDI_LTR ? margin_left : margin_right;

			if (allowed <= 0.)
				s = EV_TIS_Gray;
			break;
	}

	return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_CharFmt)
{
	ABIWORD_VIEW;
	CHECK_INC_LOAD;

	bool bMultiple = false;
	bool bSize = false;
	bool bString = false;

	const gchar * prop = NULL;
	const gchar * val  = NULL;

	EV_Toolbar_ItemState s = EV_TIS_ZERO;

	// todo: rtl/ltr/dom-dir are legal
	if(pView->getDocument()->areStylesLocked() && !(AP_TOOLBAR_ID_FMT_SUPERSCRIPT == id || AP_TOOLBAR_ID_FMT_SUBSCRIPT == id)) {
	  return EV_TIS_Gray;
	}

	switch (id)
	{
	case AP_TOOLBAR_ID_FMT_FONT:
		prop = "font-family";
		val  = "";
		bString = true;
		break;

	case AP_TOOLBAR_ID_FMT_SIZE:
		prop = "font-size";
		val  = "";
		bSize = true;
		break;

	case AP_TOOLBAR_ID_FMT_BOLD:
		prop = "font-weight";
		val  = "bold";
		break;

	case AP_TOOLBAR_ID_FMT_ITALIC:
		prop = "font-style";
		val  = "italic";
		break;

	case AP_TOOLBAR_ID_FMT_UNDERLINE:
		prop = "text-decoration";
		val  = "underline";
		bMultiple = true;
		break;

	case AP_TOOLBAR_ID_FMT_OVERLINE:
		prop = "text-decoration";
		val  = "overline";
		bMultiple = true;
		break;

	case AP_TOOLBAR_ID_FMT_STRIKE:
		prop = "text-decoration";
		val  = "line-through";
		bMultiple = true;
		break;


	case AP_TOOLBAR_ID_FMT_TOPLINE:
		prop = "text-decoration";
		val  = "topline";
		bMultiple = true;
		break;


	case AP_TOOLBAR_ID_FMT_BOTTOMLINE:
		prop = "text-decoration";
		val  = "bottomline";
		bMultiple = true;
		break;

	case AP_TOOLBAR_ID_FMT_SUPERSCRIPT:
		prop = "text-position";
		val  = "superscript";
		bMultiple = true;
		break;

	case AP_TOOLBAR_ID_FMT_SUBSCRIPT:
		prop = "text-position";
		val  = "subscript";
		bMultiple = true;
		break;

	case AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR:
		prop = "dir-override";
		val  = "ltr";
		break;

	case AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL:
		prop = "dir-override";
		val  = "rtl";
		break;

	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	if (prop && val)
	{
		// get current char properties from pView
		PP_PropertyVector props_in;

		if (!pView->getCharFormat(props_in))
			return s;

		// NB: maybe *no* properties are consistent across the selection
		const std::string & sz = PP_getAttribute(prop, props_in);

		if (!sz.empty())
		{
			if (bSize)
			{
				static char buf[7];
				sprintf(buf, "%s", std_size_string(static_cast<float>(UT_convertToPoints(sz.c_str()))));
				*pszState = buf;
				s = EV_TIS_UseString;
			}
			else if (bString)
			{
				// XXX this and above: ugly local static.
				static std::string sz2;
				sz2 = sz;
				*pszState = sz2.c_str();
				s = EV_TIS_UseString;
			}
			else if (bMultiple)
			{
				// some properties have multiple values
				if (sz.find(val) != std::string::npos)
					s = EV_TIS_Toggled;
			}
			else
			{
				if (sz == val)
					s = EV_TIS_Toggled;
			}
		}
	}

	return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_SectionFmt)
{
	ABIWORD_VIEW;
	CHECK_INC_LOAD;

	if (pszState)
		*pszState = NULL;

	const gchar * prop = "";
	const gchar * val  = NULL;

	EV_Toolbar_ItemState s = EV_TIS_ZERO;

	if(pView->isHdrFtrEdit()  || pView->isInHdrFtr(pView->getPoint()))
	  {
	    switch(id)
	      {
		// only 1 column allowed
	      case AP_TOOLBAR_ID_1COLUMN:
		return EV_TIS_Toggled;

		// not allowed
	      case AP_TOOLBAR_ID_2COLUMN:
	      case AP_TOOLBAR_ID_3COLUMN:
		  case AP_TOOLBAR_ID_INSERT_TABLE:
	        return EV_TIS_Gray;
	      }
	  }
	if(id == AP_TOOLBAR_ID_INSERT_TABLE)
	{
		return EV_TIS_ZERO;
	}
	switch (id)
	{
	case AP_TOOLBAR_ID_1COLUMN:
		prop = "columns";
		val = "1";
		break;
	case AP_TOOLBAR_ID_2COLUMN:
		prop = "columns";
		val = "2";
		break;
	case AP_TOOLBAR_ID_3COLUMN:
		prop = "columns";
		val = "3";
		break;
	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	s = EV_TIS_ZERO;
	if (prop && val)
	{
		// get current block properties from pView
		PP_PropertyVector props_in;

        bool bResult = pView->getSectionFormat(props_in);

        if (!bResult)
		{
			return s;
		}
		// NB: maybe *no* properties are consistent across the selection
		const std::string & sz = PP_getAttribute(prop, props_in);

		if (!sz.empty())
		{
			if (sz == val)
			{
				s = EV_TIS_Toggled;
			}
			else
			{
				s = EV_TIS_ZERO;
			}
		}
	}

	return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_BlockFmt)
{
	ABIWORD_VIEW;
	CHECK_INC_LOAD;

	if (pszState)
		*pszState = NULL;

	bool bPoints = false;

	const gchar * prop = "text-align";
	const gchar * val  = NULL;

	EV_Toolbar_ItemState s = EV_TIS_ZERO;

        if(pView->getDocument()->areStylesLocked()) {
	    return EV_TIS_Gray;
        }

	switch (id)
	{
	case AP_TOOLBAR_ID_ALIGN_LEFT:
		val  = "left";
		break;

	case AP_TOOLBAR_ID_ALIGN_CENTER:
		val  = "center";
		break;

	case AP_TOOLBAR_ID_ALIGN_RIGHT:
		val  = "right";
		break;

	case AP_TOOLBAR_ID_ALIGN_JUSTIFY:
		val  = "justify";
		break;

	case AP_TOOLBAR_ID_PARA_0BEFORE:
		prop = "margin-top";
		val = "0pt";
		bPoints = true;
		break;

	case AP_TOOLBAR_ID_PARA_12BEFORE:
		prop = "margin-top";
		val = "12pt";
		bPoints = true;
		break;

	case AP_TOOLBAR_ID_SINGLE_SPACE:
		prop = "line-height";
		val = "1.0";
		break;

	case AP_TOOLBAR_ID_MIDDLE_SPACE:
		prop = "line-height";
		val = "1.5";
		break;

	case AP_TOOLBAR_ID_DOUBLE_SPACE:
		prop = "line-height";
		val = "2.0";
		break;

	case AP_TOOLBAR_ID_FMT_DOM_DIRECTION:
		prop = "dom-dir";
		val = "rtl";
		xxx_UT_DEBUGMSG(("ap_ToolbarGetState_BlockFmt: dom-dir\n"));
		break;

	default:
		UT_DEBUGMSG(("id=%d", id));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	if (prop && val)
	{
		// get current block properties from pView
		PP_PropertyVector props_in;

		if (!pView->getBlockFormat(props_in))
			return s;

		// NB: maybe *no* properties are consistent across the selection
		const std::string & sz = PP_getAttribute(prop, props_in);

		if (!sz.empty())
		{
			if (bPoints)
			{
				if ((static_cast<int>(UT_convertToPoints(sz.c_str()))) == (static_cast<int>(UT_convertToPoints(val))))
					s = EV_TIS_Toggled;
			}
			else
			{
				if (sz == val)
					s = EV_TIS_Toggled;
			}
		}
	}

	return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Zoom)
{
	ABIWORD_VIEW;
	UT_UNUSED(id);
	UT_return_val_if_fail (pView, EV_TIS_Gray);

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	EV_Toolbar_ItemState s = EV_TIS_UseString;

	UT_uint32 iZoom = 0 ;
	static std::string str;
	
	switch(pFrame->getZoomType())
	{
	// special cases
	case XAP_Frame::z_PAGEWIDTH:
		pSS->getValueUTF8(XAP_STRING_ID_TB_Zoom_PageWidth, str);
		break;
	case XAP_Frame::z_WHOLEPAGE:
		pSS->getValueUTF8(XAP_STRING_ID_TB_Zoom_WholePage, str);
		break;
	default:
	  iZoom = pView->getGraphics()->getZoomPercentage();
	  str = UT_std_string_sprintf("%d%%", iZoom);
	  break;
	}

	*pszState = str.c_str();

	return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_View)
{
	ABIWORD_VIEW;
	UT_UNUSED(pszState);
	UT_return_val_if_fail (pView, EV_TIS_Gray);

	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_return_val_if_fail (pFrame, EV_TIS_Gray);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
	UT_return_val_if_fail (pFrameData, EV_TIS_Gray);

	EV_Toolbar_ItemState s = EV_TIS_ZERO;

	switch(id)
	{
	case AP_TOOLBAR_ID_VIEW_SHOWPARA:
        if ( pFrameData->m_bShowPara )
            s = EV_TIS_Toggled;
        break;

	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_StylesLocked)
{
        ABIWORD_VIEW;
        UT_UNUSED(id);
        UT_UNUSED(pszState);
        UT_return_val_if_fail (pView, EV_TIS_Gray);

        if(pView->getDocument()->areStylesLocked()) {
            return EV_TIS_Gray;
        }

        return EV_TIS_ZERO;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Table)
{
  ABIWORD_VIEW;
  UT_UNUSED(id);
  UT_UNUSED(pszState);
  UT_return_val_if_fail (pView, EV_TIS_Gray);
  
  if(pView->isInTable() )
  {
	  return EV_TIS_ZERO;
  }
  return EV_TIS_Gray;
}


Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_TableOK)
{
  ABIWORD_VIEW;
  UT_UNUSED(id);
  UT_UNUSED(pszState);
  UT_return_val_if_fail (pView, EV_TIS_Gray);
  
  if(pView->isInTable() && (pView->isHdrFtrEdit() || pView->isInHdrFtr(pView->getPoint())))
  {
	  return EV_TIS_Gray;
  }
  else if(pView->isInFootnote())
  {
	  return EV_TIS_Gray;
  }
  else if(pView->isInAnnotation())
  {
	  return EV_TIS_Gray;
  }
  else if(pView->isInEndnote())
  {
	  return EV_TIS_Gray;
  } 
  else if(pView->getHyperLinkRun(pView->getPoint()) != NULL)
  {
	  return EV_TIS_Gray;
  }

  return EV_TIS_ZERO;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_TableMerged)
{
  ABIWORD_VIEW;
  UT_UNUSED(id);
  UT_UNUSED(pszState);
  UT_return_val_if_fail (pView, EV_TIS_Gray);
  
  if(pView->isInTable())
  {
	  return EV_TIS_ZERO;
  }
  return EV_TIS_Gray;
}

// HACK TO ALWAYS DISABLE A TOOLBAR ITEM... DELETE ME
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_AlwaysDisabled)
{
  UT_UNUSED(pAV_View);
  UT_UNUSED(id);
  UT_UNUSED(pszState);
  return EV_TIS_Gray;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_HasRevisions)
{
	UT_UNUSED(id);
	UT_UNUSED(pszState);
	ABIWORD_VIEW;
 	UT_return_val_if_fail (pView, EV_TIS_Gray);

	if(pView->getDocument()->getHighestRevisionId() == 0)
		return EV_TIS_Hidden;
	
	return EV_TIS_ZERO;
   
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_CursorInSemItem)
{
	UT_UNUSED(pszState);
	ABIWORD_VIEW;
 	UT_return_val_if_fail (pView, EV_TIS_Gray);

	/* The editors aren't working yet. Remove if they do work. */
	if (id == AP_TOOLBAR_ID_SEMITEM_EDIT) 
		return EV_TIS_Gray;

    xxx_UT_DEBUGMSG((" ap_ToolbarGetState_CursorInSemItem() pDoc:%p\n", pView->getDocument() ));
	if( PD_Document * pDoc = pView->getDocument() )
	{
		if( PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF() )
		{
            if( !rdf->haveSemItems() )
            {
                return EV_TIS_Gray;
            }
            
            xxx_UT_DEBUGMSG((" ap_ToolbarGetState_CursorInSemItem(have rdf) point:%d\n", pView->getPoint() ));
            std::set< std::string > col;
            rdf->addRelevantIDsForPosition( col, pView->getPoint() );
            if( col.empty() )
                rdf->addRelevantIDsForPosition( col, pView->getPoint()-1 );
            xxx_UT_DEBUGMSG((" ap_ToolbarGetState_CursorInSemItem(d) point:%ld col.sz:%ld\n",
                             pView->getPoint(), col.size() ));
            if( col.empty() )
            {
                return EV_TIS_Gray;
            }
        }
    }
    
	return EV_TIS_ZERO;
}
