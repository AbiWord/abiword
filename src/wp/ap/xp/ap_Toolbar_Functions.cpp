/* AbiWord
 * Copyright (C) 1998-2001 AbiSource, Inc.
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_debugmsg.h"

#include "ap_Toolbar_Id.h"
#include "ap_Toolbar_Functions.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Labels.h"
#include "xap_App.h"
#include "xap_Clipboard.h"
#include "xap_Frame.h"
#include "fv_View.h"
#include "gr_Graphics.h"
#include "fl_AutoNum.h"
#include "fl_BlockLayout.h"
#include "ap_Prefs_SchemeIds.h"
#include "ap_FrameData.h"
#include "pd_Document.h"
#include "ut_Script.h"
#include "spell_manager.h"
#include "ap_EditMethods.h"


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
  EV_Toolbar_ItemState s = EV_TIS_ZERO;

  UT_ScriptLibrary& instance = UT_ScriptLibrary::instance ();
  UT_uint32 filterCount = instance.getNumScripts ();

  if ( filterCount == 0 )
    s = EV_TIS_Gray;

  return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Spelling)
{
	//ABIWORD_VIEW;
  CHECK_INC_LOAD;

  EV_Toolbar_ItemState s = EV_TIS_ZERO;

  XAP_Prefs *pPrefs = XAP_App::getApp()->getPrefs();
  UT_ASSERT( pPrefs );

  bool b = true ;
  pPrefs->getPrefsValueBool((XML_Char*)AP_PREF_KEY_AutoSpellCheck,&b) ;

  // if there are no loaded dictionaries and we are spell checking
  // as we type
  if ( SpellManager::instance ().numLoadedDicts() == 0 && b )
    s = EV_TIS_Gray;

  // either have a loaded dictionary or want to spell-check manually. allow

  return s;
}

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
	case AP_TOOLBAR_ID_FILE_SAVE:
	  if (!pView->getDocument()->isDirty() || !pView->canDo(true) || pView->getDocument()->getFileName() == NULL)
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
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
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
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
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
	       !pView->isSelectionEmpty())
	    s = EV_TIS_ZERO;
	  else
	    s = EV_TIS_Gray;
	  break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}


Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_HdrFtr)
{
	ABIWORD_VIEW;
	UT_ASSERT(pView);

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
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
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
			const XML_Char * sz;
			if (!pView->getStyle(&sz))
			{
				static const char * sz2 ="None";
				*pszState = sz2;
				s = EV_TIS_UseString;
			}

			if (sz)
			{
				static const char * sz2;
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
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	return s;
}


Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Bullets)
{
	ABIWORD_VIEW;
	CHECK_INC_LOAD;

	EV_Toolbar_ItemState s = EV_TIS_ZERO;
	if(pView->getDocument()->areStylesLocked())
	{
		return EV_TIS_Gray;
	}

	if(pView->isHdrFtrEdit())
	{
		return EV_TIS_Gray;
	}

	fl_BlockLayout * pBlock = pView->getCurrentBlock();
	UT_ASSERT(pBlock);
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

	EV_Toolbar_ItemState s = EV_TIS_ZERO;
	if(pView->getDocument()->areStylesLocked())
	{
		return EV_TIS_Gray;
	}
	if(pView->isHdrFtrEdit())
	{
		return EV_TIS_Gray;
	}

	fl_BlockLayout * pBlock = pView->getCurrentBlock();
        UT_ASSERT(pBlock);
	if(pBlock->isListItem() == false)
	        return s;
	if(pBlock->getListType() == NUMBERED_LIST)
	        s = EV_TIS_Toggled;
        return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Indents)
{
	ABIWORD_VIEW;

	EV_Toolbar_ItemState s = EV_TIS_ZERO;

	double margin_left = 0., margin_right = 0., allowed = 0.,
		page_margin_left = 0., page_margin_right = 0.;

	s_getPageMargins(pView, margin_left, margin_right,
					 page_margin_left, page_margin_right);

	FriBidiCharType iBlockDir = pView->getCurrentBlock()->getDominantDirection();

	switch(id)
	{
		case AP_TOOLBAR_ID_INDENT:
			allowed = pView->getPageSize().Width (DIM_IN) - page_margin_left - page_margin_right;
			if (margin_left >= allowed)
				s = EV_TIS_Gray;
			break;
		case AP_TOOLBAR_ID_UNINDENT:
			allowed = iBlockDir == FRIBIDI_TYPE_LTR ? margin_left : margin_right;

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

	const XML_Char * prop = NULL;
	const XML_Char * val  = NULL;

	EV_Toolbar_ItemState s = EV_TIS_ZERO;

	if(pView->getDocument()->areStylesLocked()) {
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
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	if (prop && val)
	{
		// get current char properties from pView
		const XML_Char ** props_in = NULL;
		const XML_Char * sz = NULL;

		if (!pView->getCharFormat(&props_in))
			return s;

		// NB: maybe *no* properties are consistent across the selection
		if (props_in && props_in[0])
			sz = UT_getAttribute(prop, props_in);

		if (sz)
		{
			if (bSize)
			{
				static char buf[7];
				sprintf(buf, "%s", std_size_string((float)UT_convertToPoints(sz)));
				*pszState = buf;
				s = EV_TIS_UseString;
			}
			else if (bString)
			{
				static const char * sz2;
				sz2 = sz;
				*pszState = sz2;
				s = EV_TIS_UseString;
			}
			else if (bMultiple)
			{
				// some properties have multiple values
				if (strstr(sz, val))
					s = EV_TIS_Toggled;
			}
			else
			{
				if (0 == UT_strcmp(sz, val))
					s = EV_TIS_Toggled;
			}
		}

		free(props_in);
	}

	return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_SectionFmt)
{
	ABIWORD_VIEW;
	CHECK_INC_LOAD;

	if (pszState)
		*pszState = NULL;

	const XML_Char * prop = "";
	const XML_Char * val  = NULL;

	EV_Toolbar_ItemState s = EV_TIS_ZERO;

	if(pView->isHdrFtrEdit())
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
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	s = EV_TIS_ZERO;
	if (prop && val)
	{
		// get current block properties from pView
		const XML_Char ** props_in = NULL;
		const XML_Char * sz = NULL;

        bool bResult = pView->getSectionFormat(&props_in);

        if (!bResult)
		{
			return s;
		}
		// NB: maybe *no* properties are consistent across the selection
		if (props_in && props_in[0])
			sz = UT_getAttribute(prop, props_in);

		if (sz)
		{
			if (0 == UT_strcmp(sz, val))
			{
				s = EV_TIS_Toggled;
			}
			else
			{
				s = EV_TIS_ZERO;
			}
		}

		free(props_in);
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

	const XML_Char * prop = "text-align";
	const XML_Char * val  = NULL;

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
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	if (prop && val)
	{
		// get current block properties from pView
		const XML_Char ** props_in = NULL;
		const XML_Char * sz = NULL;

		if (!pView->getBlockFormat(&props_in))
			return s;

		// NB: maybe *no* properties are consistent across the selection
		if (props_in && props_in[0])
			sz = UT_getAttribute(prop, props_in);

		if (sz)
		{
			if (bPoints)
			{
				if (((int) UT_convertToPoints(sz)) == ((int) UT_convertToPoints(val)))
					s = EV_TIS_Toggled;
			}
			else
			{
				if (0 == UT_strcmp(sz, val))
					s = EV_TIS_Toggled;
			}
		}

		free(props_in);
	}

	return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Zoom)
{
	ABIWORD_VIEW;
	UT_ASSERT(pView);

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	EV_Toolbar_ItemState s = EV_TIS_UseString;

	static char buf[10];
	UT_uint32 iZoom = 0 ;

	switch(pFrame->getZoomType())
	{
	// special cases
	case XAP_Frame::z_PAGEWIDTH:
		sprintf(buf, "%s", pSS->getValue(XAP_STRING_ID_TB_Zoom_PageWidth, pFrame->getApp()->getDefaultEncoding()).c_str());
		break;
	case XAP_Frame::z_WHOLEPAGE:
		sprintf(buf, "%s", pSS->getValue(XAP_STRING_ID_TB_Zoom_WholePage, pFrame->getApp()->getDefaultEncoding()).c_str());
		break;
	default:
	  iZoom = pView->getGraphics()->getZoomPercentage();
	  sprintf(buf, "%d%%", iZoom);
	  break;
	}

	*pszState = buf;

	return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_View)
{
	ABIWORD_VIEW;
	UT_ASSERT(pView);

	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_ASSERT(pFrame);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
	UT_ASSERT(pFrameData);

	EV_Toolbar_ItemState s = EV_TIS_ZERO;

	switch(id)
	{
	case AP_TOOLBAR_ID_VIEW_SHOWPARA:
        if ( pFrameData->m_bShowPara )
            s = EV_TIS_Toggled;
        break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_StylesLocked)
{
        ABIWORD_VIEW;
        UT_ASSERT(pView);

        if(pView->getDocument()->areStylesLocked()) {
            return EV_TIS_Gray;
        }

        return EV_TIS_ZERO;
}

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Table)
{
  ABIWORD_VIEW;
  UT_ASSERT(pView);
  
  if(pView->isInTable())
    return EV_TIS_ZERO;
  
  return EV_TIS_Gray;
}

// HACK TO ALWAYS DISABLE A TOOLBAR ITEM... DELETE ME
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_AlwaysDisabled)
{
  return EV_TIS_Gray;
}
