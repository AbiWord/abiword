/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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
#include "ut_debugmsg.h"
#include "ap_Strings.h"
#include "ap_Menu_Id.h"
#include "ap_Menu_Functions.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "xap_App.h"
#include "xap_Clipboard.h"
#include "xap_Frame.h"
#include "xap_Prefs.h"
#include "xav_View.h"
#include "fv_View.h"
#include "ap_FrameData.h"
#include "ap_Prefs.h"

#define ABIWORD_VIEW  	FV_View * pView = static_cast<FV_View *>(pAV_View)

/*****************************************************************/
/*****************************************************************/

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Autotext)
{
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	UT_ASSERT(pLabel);

	const char * c = NULL;

	const XAP_StringSet * pss = pApp->getStringSet();
	c = pss->getValue(AP_STRING_ID_DLG_Spell_NoSuggestions);

	switch (id)
	  {
	  case AP_MENU_ID_AUTOTEXT_ATTN_1: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_ATTN_1); break;
	  case AP_MENU_ID_AUTOTEXT_ATTN_2: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_ATTN_2); break;

	  case AP_MENU_ID_AUTOTEXT_CLOSING_1: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_CLOSING_1); break;
	  case AP_MENU_ID_AUTOTEXT_CLOSING_2: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_CLOSING_2); break;
	  case AP_MENU_ID_AUTOTEXT_CLOSING_3: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_CLOSING_3); break;
	  case AP_MENU_ID_AUTOTEXT_CLOSING_4: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_CLOSING_4); break;
	  case AP_MENU_ID_AUTOTEXT_CLOSING_5: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_CLOSING_5); break;
	  case AP_MENU_ID_AUTOTEXT_CLOSING_6: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_CLOSING_6); break;
	  case AP_MENU_ID_AUTOTEXT_CLOSING_7: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_CLOSING_7); break;
	  case AP_MENU_ID_AUTOTEXT_CLOSING_8: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_CLOSING_8); break;
	  case AP_MENU_ID_AUTOTEXT_CLOSING_9: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_CLOSING_9); break;
	  case AP_MENU_ID_AUTOTEXT_CLOSING_10: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_CLOSING_10); break;
	  case AP_MENU_ID_AUTOTEXT_CLOSING_11: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_CLOSING_11); break;
	  case AP_MENU_ID_AUTOTEXT_CLOSING_12: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_CLOSING_12); break;

	  case AP_MENU_ID_AUTOTEXT_MAIL_1: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_MAIL_1); break;
	  case AP_MENU_ID_AUTOTEXT_MAIL_2: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_MAIL_2); break;
	  case AP_MENU_ID_AUTOTEXT_MAIL_3: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_MAIL_3); break;
	  case AP_MENU_ID_AUTOTEXT_MAIL_4: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_MAIL_4); break;
	  case AP_MENU_ID_AUTOTEXT_MAIL_5: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_MAIL_5); break;
	  case AP_MENU_ID_AUTOTEXT_MAIL_6: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_MAIL_6); break;
	  case AP_MENU_ID_AUTOTEXT_MAIL_7: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_MAIL_7); break;
	  case AP_MENU_ID_AUTOTEXT_MAIL_8: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_MAIL_8); break;

	  case AP_MENU_ID_AUTOTEXT_REFERENCE_1: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_REFERENCE_1); break;
	  case AP_MENU_ID_AUTOTEXT_REFERENCE_2: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_REFERENCE_2); break;
	  case AP_MENU_ID_AUTOTEXT_REFERENCE_3: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_REFERENCE_3); break;

	  case AP_MENU_ID_AUTOTEXT_SALUTATION_1: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_SALUTATION_1); break;
	  case AP_MENU_ID_AUTOTEXT_SALUTATION_2: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_SALUTATION_2); break;
	  case AP_MENU_ID_AUTOTEXT_SALUTATION_3: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_SALUTATION_3); break;
	  case AP_MENU_ID_AUTOTEXT_SALUTATION_4: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_SALUTATION_4); break;

	  case AP_MENU_ID_AUTOTEXT_SUBJECT_1: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_SUBJECT_1); break;

	  case AP_MENU_ID_AUTOTEXT_EMAIL_1: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_EMAIL_1); break;
	  case AP_MENU_ID_AUTOTEXT_EMAIL_2: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_EMAIL_2); break;
	  case AP_MENU_ID_AUTOTEXT_EMAIL_3: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_EMAIL_3); break;
	  case AP_MENU_ID_AUTOTEXT_EMAIL_4: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_EMAIL_4); break;
	  case AP_MENU_ID_AUTOTEXT_EMAIL_5: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_EMAIL_5); break;
	  case AP_MENU_ID_AUTOTEXT_EMAIL_6: 
	    c = pss->getValue(AP_STRING_ID_AUTOTEXT_EMAIL_6); break;
	    
	  default: 
	    c = "No clue"; break;
	  }

	return c;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Recent)
{
	// Compute the menu label for _recent_1 thru _recent_9 on the menu.
	// We return a pointer to a static string (which will be overwritten
	// on the next call).
	
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	UT_ASSERT(pLabel);

	UT_ASSERT(id >= AP_MENU_ID_FILE_RECENT_1);
	UT_ASSERT(id <= AP_MENU_ID_FILE_RECENT_9);
	
	UT_uint32 ndx = (id - AP_MENU_ID_FILE_RECENT_1 + 1);

	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);

	if (ndx <= pPrefs->getRecentCount())
	{
		const char * szFormat = pLabel->getMenuLabel();
		static char buf[128];	// BUGBUG: possible buffer overflow

		const char * szRecent = pPrefs->getRecent(ndx);

		sprintf(buf,szFormat,szRecent);	
		return buf;
	}
	
	// for the other slots, return a null string to tell
	// the menu code to remove this item from the menu.

	return NULL;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_About)
{
	// Compute the menu label for the _help_about item.
	
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	UT_ASSERT(pLabel);

	UT_ASSERT(id == AP_MENU_ID_HELP_ABOUT);
	
	const char * szFormat = pLabel->getMenuLabel();
	static char buf[128];

	const char * szAppName = pApp->getApplicationName();

	sprintf(buf,szFormat,szAppName);	
	return buf;

	return NULL;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Contents)
{
	// Compute the menu label for the _help_contents item.
	
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	UT_ASSERT(pLabel);

	UT_ASSERT(id == AP_MENU_ID_HELP_CONTENTS);
	
	const char * szFormat = pLabel->getMenuLabel();
	static char buf[128];

	const char * szAppName = pApp->getApplicationName();

	sprintf(buf,szFormat,szAppName);	
	return buf;

	return NULL;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Index)
{
	// Compute the menu label for the _help_index item.
	
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	UT_ASSERT(pLabel);

	UT_ASSERT(id == AP_MENU_ID_HELP_INDEX);
	
	const char * szFormat = pLabel->getMenuLabel();
	static char buf[128];

	const char * szAppName = pApp->getApplicationName();

	sprintf(buf,szFormat,szAppName);	
	return buf;

	return NULL;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Search)
{
	// Compute the menu label for the _help_search item.
	
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	UT_ASSERT(pLabel);

	UT_ASSERT(id == AP_MENU_ID_HELP_SEARCH);
	
	const char * szFormat = pLabel->getMenuLabel();
	static char buf[128];

	const char * szAppName = pApp->getApplicationName();

	sprintf(buf,szFormat,szAppName);	
	return buf;

	return NULL;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_AboutOS)
{
	// Compute the menu label for the about OS help item.
	
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	UT_ASSERT(pLabel);

	UT_ASSERT(id == AP_MENU_ID_HELP_ABOUTOS);
	
	const char * szFormat = pLabel->getMenuLabel();
	static char buf[128];

	const char * szAppName = pApp->getApplicationName();

	sprintf(buf,szFormat,szAppName);	
	return buf;

	return NULL;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Checkver)
{
	// Compute the menu label for the about the check version item.
	
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	UT_ASSERT(pLabel);

	UT_ASSERT(id == AP_MENU_ID_HELP_CHECKVER);
	
	const char * szFormat = pLabel->getMenuLabel();
	static char buf[128];

	const char * szAppName = pApp->getApplicationName();

	sprintf(buf,szFormat,szAppName);	
	return buf;

	return NULL;
}
/*****************************************************************/
/*****************************************************************/

Defun_EV_GetMenuItemState_Fn(ap_GetState_Window)
{
	UT_ASSERT(pAV_View);

	UT_ASSERT(id >= AP_MENU_ID_WINDOW_1);
	UT_ASSERT(id <= AP_MENU_ID_WINDOW_9);
	
	UT_uint32 ndx = (id - AP_MENU_ID_WINDOW_1);

	EV_Menu_ItemState s = EV_MIS_ZERO;
	
	XAP_Frame * pFrame = (XAP_Frame *) pAV_View->getParentData();
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	if (pFrame == pApp->getFrame(ndx))
		s = EV_MIS_Toggled;

	return s;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Window)
{
	// Compute the menu label for _window_1 thru _window_9 on the menu.
	// We return a pointer to a static string (which will be overwritten
	// on the next call).
	
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	UT_ASSERT(pLabel);


	UT_ASSERT(id >= AP_MENU_ID_WINDOW_1);
	UT_ASSERT(id <= AP_MENU_ID_WINDOW_9);
	
	UT_uint32 ndx = (id - AP_MENU_ID_WINDOW_1);

	// use the applications window list and compute a menu label
	// for the window with the computed index.  use the static
	// menu label as as format string.

	if (ndx < pApp->getFrameCount())
	{
		const char * szFormat = pLabel->getMenuLabel();
		static char buf[128];

		XAP_Frame * pFrame = pApp->getFrame(ndx);
		UT_ASSERT(pFrame);

		const char * szTitle = pFrame->getTitle(128 - strlen(szFormat));

		sprintf(buf,szFormat,szTitle);	
		return buf;
	}
	
	// for the other slots, return a null string to tell
	// the menu code to remove this item from the menu.

	return NULL;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_WindowMore)
{
	// Compute the menu label for the _window_more ("More Windows...") item.
	
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	UT_ASSERT(pLabel);
	UT_ASSERT(id == AP_MENU_ID_WINDOW_MORE);
	
	// if we have more than 9 windows in our window list,
	// we return the static menu label.  if not, we return
	// null string to tell the menu code to remove this
	// item from the menu.
	if (8 < pApp->getFrameCount())
		return pLabel->getMenuLabel();

	return NULL;
}

/*****************************************************************/
/*****************************************************************/

Defun_EV_GetMenuItemState_Fn(ap_GetState_Suggest)
{
	ABIWORD_VIEW;
	UT_ASSERT(pView);

	UT_ASSERT(id >= AP_MENU_ID_SPELL_SUGGEST_1);
	UT_ASSERT(id <= AP_MENU_ID_SPELL_SUGGEST_9);
	
	UT_uint32 ndx = (id - AP_MENU_ID_SPELL_SUGGEST_1 + 1);

	EV_Menu_ItemState s = EV_MIS_Gray;

	const UT_UCSChar *p = pView->getContextSuggest(ndx);

	if (p)
	{
		s = EV_MIS_Bold;
	}

	FREEP(p);

	return s;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Suggest)
{
	// Compute the menu label for _suggest_1 thru _suggest_9 on the menu.
	// We return a pointer to a static string (which will be overwritten
	// on the next call).
	
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	UT_ASSERT(pLabel);

	AV_View * pAV_View = pFrame->getCurrentView();
	ABIWORD_VIEW;

	UT_ASSERT(id >= AP_MENU_ID_SPELL_SUGGEST_1);
	UT_ASSERT(id <= AP_MENU_ID_SPELL_SUGGEST_9);
	
	UT_uint32 ndx = (id - AP_MENU_ID_SPELL_SUGGEST_1 + 1);

	const char * c = NULL;
	const UT_UCSChar *p;
	static char cBuf[128];		// BUGBUG: possible buffer overflow
	UT_uint32 len = 0;

	p = pView->getContextSuggest(ndx);
	if (p && *p)
		len = UT_UCS_strlen(p);

	if (len)
	{
		// this is a suggestion
		UT_UCS_strcpy_to_char(cBuf, p);
		c = cBuf;
	}
	else if (ndx == 1)
	{
		// placeholder when no suggestions
		const XAP_StringSet * pSS = pApp->getStringSet();
		c = pSS->getValue(AP_STRING_ID_DLG_Spell_NoSuggestions);
	}

	FREEP(p);

	if (c && *c)
	{
		const char * szFormat = pLabel->getMenuLabel();
		static char buf[128];	// BUGBUG: possible buffer overflow

		sprintf(buf,szFormat,c);
		
		return buf;
	}

	// for the other slots, return a null string to tell
	// the menu code to remove this item from the menu.

	return NULL;
}

/****************************************************************/
/****************************************************************/

Defun_EV_GetMenuItemState_Fn(ap_GetState_Changes)
{
	ABIWORD_VIEW;
	UT_ASSERT(pView);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	switch(id)
	{
	case AP_MENU_ID_EDIT_UNDO:
		if (!pView->canDo(true))
			s = EV_MIS_Gray;
		break;

	case AP_MENU_ID_EDIT_REDO:
		if (!pView->canDo(false))
			s = EV_MIS_Gray;
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_Selection)
{
        ABIWORD_VIEW;
	XAP_App * pApp = pView->getApp();
	UT_ASSERT(pApp);

	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	switch(id)
	{
	case AP_MENU_ID_EDIT_CUT:
	case AP_MENU_ID_EDIT_COPY:
		if (pView->isSelectionEmpty())
			s = EV_MIS_Gray;
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_Clipboard)
{
	ABIWORD_VIEW;
	UT_ASSERT(pView);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	switch(id)
	{
	case AP_MENU_ID_EDIT_PASTE:
		s = ( (pView->getApp()->canPasteFromClipboard()) ? EV_MIS_ZERO : EV_MIS_Gray );
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_Prefs)
{
        ABIWORD_VIEW;
	UT_ASSERT(pView);

	XAP_App *pApp = pView->getApp();
	UT_ASSERT(pApp);

	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	bool b = true;

	switch (id)
	  {
	  case AP_MENU_ID_TOOLS_AUTOSPELL:
	    pPrefs->getPrefsValueBool((XML_Char*)AP_PREF_KEY_AutoSpellCheck, &b);
	    s = (b ? EV_MIS_Toggled : EV_MIS_ZERO);
	    break;

	  default:
	    UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	    break;
	  }

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_CharFmt)
{
	ABIWORD_VIEW;
	UT_ASSERT(pView);
	bool bMultiple = false;

	EV_Menu_ItemState s = EV_MIS_ZERO;

	const XML_Char * prop = NULL;
	const XML_Char * val  = NULL;

	switch(id)
	{
	case AP_MENU_ID_FMT_BOLD:
		prop = "font-weight";
		val  = "bold";
		break;

	case AP_MENU_ID_FMT_ITALIC:
		prop = "font-style";
		val  = "italic";
		break;

	case AP_MENU_ID_FMT_UNDERLINE:
		prop = "text-decoration";
		val  = "underline";
		bMultiple = true;
		break;

	case AP_MENU_ID_FMT_OVERLINE:
		prop = "text-decoration";
		val  = "overline";
		bMultiple = true;
		break;

	case AP_MENU_ID_FMT_STRIKE:
		prop = "text-decoration";
		val  = "line-through";
		bMultiple = true;
		break;
	case AP_MENU_ID_FMT_SUPERSCRIPT:
		prop = "text-position";
		val  = "superscript";
		break;

	case AP_MENU_ID_FMT_SUBSCRIPT:
		prop = "text-position";
		val  = "subscript";
		break;
	
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	if (prop && val)
	{
		// get current font info from pView
		const XML_Char ** props_in = NULL;
		const XML_Char * sz;

		if (!pView->getCharFormat(&props_in))
			return s;

		sz = UT_getAttribute(prop, props_in);
		if (sz)
		{
			if (bMultiple)
			{	
				// some properties have multiple values
				if (strstr(sz, val))
					s = EV_MIS_Toggled;
			}
			else
			{
				if (0 == UT_strcmp(sz, val))
					s = EV_MIS_Toggled;
			}
		}
		
		free(props_in);
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_BlockFmt)
{
	ABIWORD_VIEW;
	UT_ASSERT(pView);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	const XML_Char * prop = "text-align";
	const XML_Char * val  = NULL;

	switch(id)
	{
	case AP_MENU_ID_ALIGN_LEFT:
		val  = "left";
		break;

	case AP_MENU_ID_ALIGN_CENTER:
		val  = "center";
		break;

	case AP_MENU_ID_ALIGN_RIGHT:
		val  = "right";
		break;

	case AP_MENU_ID_ALIGN_JUSTIFY:
		val  = "justify";
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	if (prop && val)
	{
		// get current font info from pView
		const XML_Char ** props_in = NULL;
		const XML_Char * sz;

		if (!pView->getBlockFormat(&props_in))
			return s;

		sz = UT_getAttribute(prop, props_in);
		if (sz && (0 == UT_strcmp(sz, val)))
			s = EV_MIS_Toggled;
		
		free(props_in);
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_View)
{
	ABIWORD_VIEW;
	UT_ASSERT(pView);

	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_ASSERT(pFrame);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
	UT_ASSERT(pFrameData);

	EV_Menu_ItemState s = EV_MIS_ZERO;


	switch(id)
	{
	case AP_MENU_ID_VIEW_RULER:
		if ( pFrameData->m_bShowRuler && !pFrameData->m_bIsFullScreen) 
			s = EV_MIS_Toggled;
		else
			s = EV_MIS_ZERO;
		break;
	case AP_MENU_ID_VIEW_SHOWPARA:
	        if ( pFrameData->m_bShowPara )
		  s = EV_MIS_Toggled;
		else
		  s = EV_MIS_ZERO;
		break;
//	case AP_MENU_ID_VIEW_HEADFOOT:
//		// TODO: implement view methods to check, toggle state
//		s = EV_MIS_Gray;
//		break;
	case AP_MENU_ID_VIEW_TB_STD:
		if ( pFrameData->m_bShowBar[0] && !pFrameData->m_bIsFullScreen) 
			s = EV_MIS_Toggled;
		else
			s = EV_MIS_ZERO;
		break;
	case AP_MENU_ID_VIEW_TB_FORMAT:
		if ( pFrameData->m_bShowBar[1] && !pFrameData->m_bIsFullScreen) 
			s = EV_MIS_Toggled;
		else
			s = EV_MIS_ZERO;
		break;
	case AP_MENU_ID_VIEW_TB_EXTRA:
		if ( pFrameData->m_bShowBar[2] && !pFrameData->m_bIsFullScreen) 
			s = EV_MIS_Toggled;
		else
			s = EV_MIS_ZERO;
		break;
	case AP_MENU_ID_VIEW_STATUSBAR:
              if ( pFrameData->m_bShowStatusBar && 
				   !pFrameData->m_bIsFullScreen )
				  s = EV_MIS_Toggled;
			  else
				  s = EV_MIS_ZERO;
			  break;

	case AP_MENU_ID_VIEW_FULLSCREEN:
	        if ( pFrameData->m_bIsFullScreen )
			s = EV_MIS_Toggled;
		else
			s = EV_MIS_ZERO;
		break;

	case AP_MENU_ID_VIEW_NORMAL:
	  if ( pFrameData->m_pViewMode == VIEW_NORMAL) 
	    s = EV_MIS_Toggled;
	  else
	    s = EV_MIS_ZERO;
	  break;

	case AP_MENU_ID_VIEW_WEB: 
	  if ( pFrameData->m_pViewMode == VIEW_WEB) 
	    s = EV_MIS_Toggled;
	  else
	    s = EV_MIS_ZERO;
	  break;

	case AP_MENU_ID_VIEW_PRINT: 
	  if ( pFrameData->m_pViewMode == VIEW_PRINT) 
	    s = EV_MIS_Toggled;
	  else
	    s = EV_MIS_ZERO;
	  break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}
