/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ap_Features.h"
#include "ut_path.h"
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
#include "xap_Toolbar_Layouts.h"
#include "fv_View.h"
#include "ap_FrameData.h"
#include "ap_Prefs.h"
#include "pd_Document.h"
#include "pd_DocumentRDF.h"
#include "ut_Script.h"

#ifdef ENABLE_SPELL
#include "spell_manager.h"
#endif

#include "ie_mailmerge.h"
#include "fp_TableContainer.h"
#include "fl_BlockLayout.h"

#ifdef TOOLKIT_WIN
#include "ap_Win32App.h" 
#endif

#define ABIWORD_VIEW  	FV_View * pView = static_cast<FV_View *>(pAV_View)

static char *s_escapeMenuString(char *p_str)
{
#ifdef TOOLKIT_WIN
	int l = strlen(p_str)+1;
	char *c = p_str, *d, *r;
	while (*c) if (*c++=='&') l++;
	d=r=(char*)g_malloc(l);
	c = p_str;
	while (*c) {
		if (*c=='&') *d++='&';
		*d++=*c++;
	}
	*d=0;
	
	return r;
#else
	return g_strdup(p_str);
#endif
}

/*****************************************************************/
/*****************************************************************/

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Toolbar)
{
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp && pLabel, NULL);

	UT_ASSERT_HARMLESS(id >= AP_MENU_ID_VIEW_TB_1);
	UT_ASSERT_HARMLESS(id <= AP_MENU_ID_VIEW_TB_4);

	UT_sint32 ndx = (id - AP_MENU_ID_VIEW_TB_1);
	const UT_GenericVector<UT_UTF8String*> & vec = pApp->getToolbarFactory()->getToolbarNames();


	if (ndx < vec.getItemCount())
	{
		const char * szFormat = pLabel->getMenuLabel();
		static char buf[128];

		const char * szRecent = vec.getNthItem(ndx)->utf8_str();

		snprintf(buf,sizeof(buf),szFormat,szRecent);
		return buf;
	}

	return NULL;
}


Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Recent)
{
	// Compute the menu label for _recent_1 thru _recent_9 on the menu.
	// We return a pointer to a static string (which will be overwritten
	// on the next call).

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp && pLabel, NULL);

	UT_ASSERT_HARMLESS(id >= AP_MENU_ID_FILE_RECENT_1);

	UT_sint32 ndx = (id - AP_MENU_ID_FILE_RECENT_1 + 1);

	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail (pPrefs, NULL);

    static char *buf = NULL;
    if (ndx <= pPrefs->getRecentCount())
    {
        const char * szFormat = pLabel->getMenuLabel();
        const char * szURI = pPrefs->getRecent(ndx);
        char *szFname = g_filename_from_uri(szURI, NULL, NULL);
        char *szRecent = g_filename_to_utf8(szFname, -1, NULL, NULL, NULL);
        char *szBasename = szRecent ? g_path_get_basename(szRecent) : g_strdup ("");
		char *szMenuname = s_escapeMenuString(szBasename);
        g_free(szFname);
        g_free(szRecent);
		g_free(szBasename);

        g_free(buf);
        buf = g_strdup_printf(szFormat, szMenuname);
        g_free(szMenuname);
        return buf;
    }

	// for the other slots, return a null string to tell
	// the menu code to remove this item from the menu.

	return NULL;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_About)
{
	UT_DEBUG_ONLY_ARG(id);
	// Compute the menu label for the _help_about item.

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp && pLabel, NULL);

	UT_ASSERT_HARMLESS(id == AP_MENU_ID_HELP_ABOUT);

	const char * szFormat = pLabel->getMenuLabel();
	static char buf[128];

	const char * szAppName = pApp->getApplicationName();

	sprintf(buf,szFormat,szAppName);
	return buf;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Contents)
{
	UT_DEBUG_ONLY_ARG(id);
	// Compute the menu label for the _help_contents item.

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp && pLabel, NULL);

	UT_ASSERT_HARMLESS(id == AP_MENU_ID_HELP_CONTENTS);

	const char * szFormat = pLabel->getMenuLabel();
	static char buf[128];

	const char * szAppName = pApp->getApplicationName();

	sprintf(buf,szFormat,szAppName);
	return buf;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Intro)
{
	UT_DEBUG_ONLY_ARG(id);
	// Compute the menu label for the _help_intro item.

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp && pLabel, NULL);

	UT_ASSERT_HARMLESS(id == AP_MENU_ID_HELP_INTRO);

	const char * szFormat = pLabel->getMenuLabel();
	static char buf[128];

	const char * szAppName = pApp->getApplicationName();

	sprintf(buf,szFormat,szAppName);
	return buf;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Search)
{
	UT_DEBUG_ONLY_ARG(id);

	// Compute the menu label for the _help_search item.

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp && pLabel, NULL);

	UT_ASSERT_HARMLESS(id == AP_MENU_ID_HELP_SEARCH);

	const char * szFormat = pLabel->getMenuLabel();
	static char buf[128];

	const char * szAppName = pApp->getApplicationName();

	sprintf(buf,szFormat,szAppName);
	return buf;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Checkver)
{
	UT_DEBUG_ONLY_ARG(id);
	// Compute the menu label for the about the check version item.

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp && pLabel, NULL);

	UT_ASSERT_HARMLESS(id == AP_MENU_ID_HELP_CHECKVER);

	const char * szFormat = pLabel->getMenuLabel();
	static char buf[128];

	const char * szAppName = pApp->getApplicationName();

	sprintf(buf,szFormat,szAppName);
	return buf;
}
/*****************************************************************/
/*****************************************************************/

Defun_EV_GetMenuItemState_Fn(ap_GetState_Window)
{
	UT_return_val_if_fail (pAV_View, EV_MIS_Gray);

	UT_ASSERT_HARMLESS(id >= AP_MENU_ID_WINDOW_1);
	UT_ASSERT_HARMLESS(id <= AP_MENU_ID_WINDOW_9);

	UT_uint32 ndx = (id - AP_MENU_ID_WINDOW_1);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pAV_View->getParentData());
	UT_return_val_if_fail (pFrame, EV_MIS_Gray);
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, EV_MIS_Gray);

	if (pFrame == pApp->getFrame(ndx))
		s = EV_MIS_Toggled;

	return s;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Window)
{
	// Compute the menu label for _window_1 thru _window_9 on the menu.
	// We return a pointer to a static string (which will be overwritten
	// on the next call).

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp && pLabel, NULL);

	UT_ASSERT_HARMLESS(id >= AP_MENU_ID_WINDOW_1);
	UT_ASSERT_HARMLESS(id <= AP_MENU_ID_WINDOW_9);

	UT_sint32 ndx = (id - AP_MENU_ID_WINDOW_1);

	// use the applications window list and compute a menu label
	// for the window with the computed index.  use the static
	// menu label as as format string.

	if (ndx < pApp->getFrameCount())
	{
		const char * szFormat = pLabel->getMenuLabel();
		static char buf[128];

		XAP_Frame * pFrame = pApp->getFrame(ndx);
		UT_return_val_if_fail (pFrame, NULL);

		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), szFormat, pFrame->getTitle().utf8_str());
		buf[sizeof(buf) - 1] = '\0';
		return buf;
	}

	// for the other slots, return a null string to tell
	// the menu code to remove this item from the menu.

	return NULL;
}

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_WindowMore)
{
	UT_DEBUG_ONLY_ARG(id);

	// Compute the menu label for the _window_more ("More Windows...") item.

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp && pLabel, NULL);
	UT_ASSERT_HARMLESS(id == AP_MENU_ID_WINDOW_MORE);

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

#ifdef ENABLE_SPELL
Defun_EV_GetMenuItemState_Fn(ap_GetState_Spelling)
{
  UT_UNUSED(pAV_View);
  UT_UNUSED(id);
  EV_Menu_ItemState s = EV_MIS_ZERO ;

  XAP_Prefs *pPrefs = XAP_App::getApp()->getPrefs();
  UT_return_val_if_fail (pPrefs, EV_MIS_Gray);

  bool b = true ;
  pPrefs->getPrefsValueBool(static_cast<const gchar *>(AP_PREF_KEY_AutoSpellCheck),&b) ;

  // if there are no loaded dictionaries and we are spell checking
  // as we type
 if ( SpellManager::instance ().numLoadedDicts() == 0 && b )
   s = EV_MIS_Gray;

 // either have a loaded dictionary or want to spell-check manually. allow

 return s;
}
#endif

Defun_EV_GetMenuItemState_Fn(ap_GetState_ColumnsActive)
{
  UT_UNUSED(id);
  ABIWORD_VIEW ;
  UT_return_val_if_fail (pView, EV_MIS_Gray);

  EV_Menu_ItemState s = EV_MIS_ZERO ;

  if(pView->isHdrFtrEdit()  || pView->isInHdrFtr(pView->getPoint()))
    s = EV_MIS_Gray;

  return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_BookmarkOK)
{
	UT_UNUSED(id);
  ABIWORD_VIEW ;
  UT_return_val_if_fail (pView, EV_MIS_Gray);

	EV_Menu_ItemState s = EV_MIS_ZERO ;

	if(pView->isTOCSelected())
	{
	    s = EV_MIS_Gray ;
		return s;
	}
	PT_DocPosition posStart = pView->getPoint();
	PT_DocPosition posEnd = pView->getSelectionAnchor();
	fl_BlockLayout * pBL1 = pView->getBlockAtPosition(posStart);
	fl_BlockLayout * pBL2 = pView->getBlockAtPosition(posEnd);
	if((pBL1 == NULL) || (pBL2 == NULL)) // make sure we get valid blocks from selection beginning and end
	{
		s = EV_MIS_Gray;
		return s;
	}
	if(pBL1 != pBL2) // don't allow Insert bookmark if selection spans multiple blocks
	{
	    s = EV_MIS_Gray ;
		return s;
	}

	return s ;
}


Defun_EV_GetMenuItemState_Fn(ap_GetState_xmlidOK)
{
	UT_UNUSED(id);
  ABIWORD_VIEW ;
  UT_return_val_if_fail (pView, EV_MIS_Gray);

	EV_Menu_ItemState s = EV_MIS_ZERO ;

	if(pView->isTOCSelected())
	{
	    s = EV_MIS_Gray ;
		return s;
	}
	PT_DocPosition posStart = pView->getPoint();
	PT_DocPosition posEnd = pView->getSelectionAnchor();
	fl_BlockLayout * pBL1 = pView->getBlockAtPosition(posStart);
	fl_BlockLayout * pBL2 = pView->getBlockAtPosition(posEnd);
	if((pBL1 == NULL) || (pBL2 == NULL)) // make sure we get valid blocks from selection beginning and end
	{
		s = EV_MIS_Gray;
		return s;
	}
	if(pBL1 != pBL2) // don't allow Insert xml:id if selection spans multiple blocks
	{
	    s = EV_MIS_Gray ;
		return s;
	}

	return s ;
}


Defun_EV_GetMenuItemState_Fn(ap_GetState_TOCOK)
{
  UT_UNUSED(id);
  ABIWORD_VIEW ;
  UT_return_val_if_fail (pView, EV_MIS_Gray);

  EV_Menu_ItemState s = EV_MIS_ZERO ;

  if(pView->isHdrFtrEdit()  || pView->isInHdrFtr(pView->getPoint()))
  {
    s = EV_MIS_Gray;
  }
  if(pView->isInHdrFtr(pView->getPoint()))
  {
    s = EV_MIS_Gray;
  }

  else if(pView->isInTable()) // isintable includes first
  {
    s = EV_MIS_Gray;
  }
  else if(pView->isInTable(pView->getSelectionAnchor())) // isintable includes first
  {
    s = EV_MIS_Gray;
  }
  else if(pView->isInFrame(pView->getPoint()))
  {
    s = EV_MIS_Gray;
  }
  else if(pView->getFrameEdit()->isActive())
  {
    s = EV_MIS_Gray;
  }
  else if(pView->isInFrame(pView->getSelectionAnchor()))
  {
    s = EV_MIS_Gray;
  }
  else if(pView->isInFootnote())
  {
    s = EV_MIS_Gray;
  }
  else if(pView->isInAnnotation())
  {
    s = EV_MIS_Gray;
  }
  else if(pView->isInFootnote(pView->getSelectionAnchor()))
  {
    s = EV_MIS_Gray;
  }
  else if(pView->isInAnnotation(pView->getSelectionAnchor()))
  {
    s = EV_MIS_Gray;
  }
  else if(pView->isInEndnote())
  {
    s = EV_MIS_Gray;
  }
  else if(pView->isInEndnote(pView->getSelectionAnchor()))
  {
    s = EV_MIS_Gray;
  }
  else if(pView->isInTable() && (pView->getPoint() > 3) && pView->isInFootnote(pView->getPoint()-2))
  {
    s = EV_MIS_Gray;
  }
  else if(pView->isInTable() && (pView->getPoint() > 3) && pView->isInAnnotation(pView->getPoint()-2))
  {
    s = EV_MIS_Gray;
  }
  else if(pView->isInTable()  && (pView->getPoint() > 3) && pView->isInEndnote(pView->getPoint()-2))
  {
    s = EV_MIS_Gray;
  }
  else if(pView->getSelectionMode() >= FV_SelectionMode_Multiple)
  {
	  return EV_MIS_Gray;
  }
  else if(pView->getHyperLinkRun(pView->getPoint()) != NULL)
  {
	  return EV_MIS_Gray;
  }
  return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_SomethingSelected)
{
	UT_UNUSED(id);
	ABIWORD_VIEW ;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	EV_Menu_ItemState s = EV_MIS_ZERO ;

	if ( pView->isSelectionEmpty () )
	  {
	    s = EV_MIS_Gray ;
	  }

	return s ;
}


Defun_EV_GetMenuItemState_Fn(ap_GetState_TextToTableOK)
{
	UT_UNUSED(id);
	ABIWORD_VIEW ;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	EV_Menu_ItemState s = EV_MIS_ZERO ;

	if ( pView->isSelectionEmpty () || pView->isInTable() || pView->isInHdrFtr(pView->getPoint()))
	  {
	    s = EV_MIS_Gray ;
	  }

	return s ;
}


static EV_Menu_ItemState HyperLinkOK(FV_View * pView)
{
	EV_Menu_ItemState s = EV_MIS_ZERO ;

	if ( pView->isSelectionEmpty())
	  {
		  if(pView->getHyperLinkRun(pView->getPoint()) == NULL)
		  {
			  s = EV_MIS_Gray ;
			  return s;
		  }
		  return s;
	  }

	if(pView->isTOCSelected())
	{
	    s = EV_MIS_Gray ;
		return s;
	}
	PT_DocPosition posStart = pView->getPoint();
	PT_DocPosition posEnd = pView->getSelectionAnchor();
	fl_BlockLayout * pBL1 = pView->getBlockAtPosition(posStart);
	fl_BlockLayout * pBL2 = pView->getBlockAtPosition(posEnd);
	if((pBL1 == NULL) || (pBL2 == NULL)) // make sure we get valid blocks from selection beginning and end
	{
		s = EV_MIS_Gray;
		return s;
	}
	if(pBL1 != pBL2) // don't allow Insert Hyperlink if selection spans multiple blocks
	{
	    s = EV_MIS_Gray ;
		return s;
	}
	if(pBL1->getLength() == 1)
	{
	    s = EV_MIS_Gray ;
	    return s;
	}
	if(posStart > posEnd)
	{
		std::swap(posStart, posEnd);
	}
	if(posStart < pBL1->getPosition(true))
	{
	    s = EV_MIS_Gray ;
	    return s;
	}
	  
	return s ;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_AnnotationJumpOK)
{
	UT_UNUSED(id);
	ABIWORD_VIEW ;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	EV_Menu_ItemState s = HyperLinkOK(pView);
	if(s ==  EV_MIS_Gray)
	{
		return s;
	}
	UT_return_val_if_fail (pView->getLayout(),EV_MIS_Gray);
	if(!pView->getLayout()->displayAnnotations())
	{
		return EV_MIS_Gray;
	}
	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_HyperlinkOK)
{
	UT_UNUSED(id);
	ABIWORD_VIEW ;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	return HyperLinkOK(pView);
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_RDF_Query)
{
	UT_UNUSED(id);
	ABIWORD_VIEW ;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	return 
#ifdef WITH_REDLAND
	       EV_MIS_ZERO; 
#else
	       EV_MIS_Gray;
#endif
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_RDF_Contact)
{
	UT_UNUSED(id);
	ABIWORD_VIEW ;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	return 
#ifdef WITH_EVOLUTION_DATA_SERVER
	       EV_MIS_ZERO; 
#else
	       EV_MIS_Gray;
#endif
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_haveSemItems)
{
	ABIWORD_VIEW ;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail( pDoc, EV_MIS_Gray );
    PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();
	UT_return_val_if_fail( rdf, EV_MIS_Gray );

	/* The editors aren't working yet. Remove if they do work. */
	if (id == AP_MENU_ID_RDFANCHOR_EDITSEMITEM) 
		return EV_MIS_Gray;

#ifndef WITH_EVOLUTION_DATA_SERVER
	if (id == AP_MENU_ID_RDFANCHOR_EXPORTSEMITEM) 
		return EV_MIS_Gray;
#endif
    
	EV_Menu_ItemState s = EV_MIS_ZERO ;
    return s;

    
    PD_RDFContacts contacts = rdf->getContacts();

    std::set< std::string > xmlids;
    rdf->addRelevantIDsForPosition( xmlids, pView->getPoint() );
    
	if( xmlids.size() < 1 )
	{
		return EV_MIS_Gray;
	}
	return s;
}


#ifdef ENABLE_SPELL
Defun_EV_GetMenuItemState_Fn(ap_GetState_Suggest)
{
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	UT_ASSERT_HARMLESS(id >= AP_MENU_ID_SPELL_SUGGEST_1);
	UT_ASSERT_HARMLESS(id <= AP_MENU_ID_SPELL_SUGGEST_9);

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

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp && pLabel, NULL);
	XAP_Frame * frame = pApp->getLastFocussedFrame();

	AV_View * pAV_View = frame->getCurrentView();
	ABIWORD_VIEW;

	UT_ASSERT_HARMLESS(id >= AP_MENU_ID_SPELL_SUGGEST_1);
	UT_ASSERT_HARMLESS(id <= AP_MENU_ID_SPELL_SUGGEST_9);

	UT_return_val_if_fail(pView != NULL, NULL);

	UT_uint32 ndx = (id - AP_MENU_ID_SPELL_SUGGEST_1 + 1);
	UT_UCSChar *p = pView->getContextSuggest(ndx);
	gchar * c = NULL;
	if (p && *p) {
		c = g_ucs4_to_utf8(p, -1, NULL, NULL, NULL);
	}
	else if (ndx == 1)
	{
		// placeholder when no suggestions
		const XAP_StringSet * pSS = pApp->getStringSet();
		std::string s;
		pSS->getValueUTF8(AP_STRING_ID_DLG_Spell_NoSuggestions,s);
		c = g_strdup(s.c_str());
	}

	FREEP(p);

	if (c && *c)
	{
		const char * szFormat = pLabel->getMenuLabel();
		static char buf[128];	// BUGBUG: possible buffer overflow

		sprintf(buf,szFormat,c);
		g_free (c); c = NULL;
		return buf;
	}

	// for the other slots, return a null string to tell
	// the menu code to remove this item from the menu.

	return NULL;
}
#endif
/****************************************************************/
/****************************************************************/

Defun_EV_GetMenuItemState_Fn(ap_GetState_Changes)
{
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_return_val_if_fail (pFrame, EV_MIS_Gray);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
	UT_return_val_if_fail (pFrameData, EV_MIS_Gray);

	switch(id)
	{
	case AP_MENU_ID_FILE_SAVE:
		if (!pView->getDocument()->isDirty() /*|| !pView->canDo(true)*/) // this caused bug 7580
	    s = EV_MIS_Gray;
	  break;
	case AP_MENU_ID_FILE_REVERT:
		if (!pView->getDocument()->isDirty() /*|| !pView->canDo(true)*/) // this cause bug 7580
	    s = EV_MIS_Gray;
	  break;
	case AP_MENU_ID_EDIT_UNDO:
		if (!pView->canDo(true))
			s = EV_MIS_Gray;
		break;

	case AP_MENU_ID_EDIT_REDO:
		if (!pView->canDo(false))
			s = EV_MIS_Gray;
		break;

	case AP_MENU_ID_EDIT_EDITHEADER:
		if (!pView->isHeaderOnPage())
			s = EV_MIS_Gray;
	  break;

	case AP_MENU_ID_EDIT_EDITFOOTER:
		if (!pView->isFooterOnPage())
			s = EV_MIS_Gray;
	  break;

	case AP_MENU_ID_INSERT_HEADER:
		if (pView->isHeaderOnPage())
			s = EV_MIS_Gray;
	  break;

	case AP_MENU_ID_INSERT_FOOTER:
		if (pView->isFooterOnPage())
			s = EV_MIS_Gray;
	  break;

	case AP_MENU_ID_EDIT_REMOVEHEADER:
		if (!pView->isHeaderOnPage())
			s = EV_MIS_Gray;
		break;

	case AP_MENU_ID_EDIT_REMOVEFOOTER:
		if (!pView->isFooterOnPage())
			s = EV_MIS_Gray;
		break;

	case AP_MENU_ID_TABLE_INSERT:
		if (pView->isHdrFtrEdit() || pView->isInHdrFtr(pView->getPoint()) || pView->isInHdrFtr(pView->getSelectionAnchor()) )
			s = EV_MIS_Gray;
		break;

	case AP_MENU_ID_TABLE_INSERT_TABLE:
		if (pView->isHdrFtrEdit() || pView->isInHdrFtr(pView->getPoint()) || pView->isInHdrFtr(pView->getSelectionAnchor()) )
			s = EV_MIS_Gray;
		break;

	case AP_MENU_ID_TABLE_INSERTTABLE:
		if (pView->isHdrFtrEdit()  || pView->isInHdrFtr(pView->getPoint()) || pView->isInHdrFtr(pView->getSelectionAnchor()) )
			s = EV_MIS_Gray;
		break;

	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_ScriptsActive)
{
  UT_UNUSED(pAV_View);
  UT_UNUSED(id);
  EV_Menu_ItemState s = EV_MIS_ZERO;

  UT_ScriptLibrary * instance = UT_ScriptLibrary::instance ();
  UT_uint32 filterCount = instance->getNumScripts ();

  if ( filterCount == 0 )
    s = EV_MIS_Gray;

  return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_Selection)
{
        ABIWORD_VIEW;
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, EV_MIS_Gray);

	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail (pPrefs, EV_MIS_Gray);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	switch(id)
	{
	case AP_MENU_ID_FMT_LANGUAGE:
	case AP_MENU_ID_EDIT_CUT:
	case AP_MENU_ID_EDIT_LATEXEQUATION:
	case AP_MENU_ID_EDIT_COPY:
	// RIVERA
	case AP_MENU_ID_TOOLS_ANNOTATIONS_INSERT_FROMSEL:
		if (pView->isSelectionEmpty())
			s = EV_MIS_Gray;
		break;
	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_Clipboard)
{
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	switch(id)
	{
	case AP_MENU_ID_EDIT_PASTE_SPECIAL:
	case AP_MENU_ID_EDIT_PASTE:
		s = ((XAP_App::getApp()->canPasteFromClipboard()) ? EV_MIS_ZERO : EV_MIS_Gray );
		break;

	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_Prefs)
{
    ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	XAP_App *pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, EV_MIS_Gray);

	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail (pPrefs, EV_MIS_Gray);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	bool b = true;

	switch (id)
	  {
	  case AP_MENU_ID_TOOLS_AUTOSPELL:
	    pPrefs->getPrefsValueBool(static_cast<const gchar *>(AP_PREF_KEY_AutoSpellCheck), &b);
	    s = (b ? EV_MIS_Toggled : EV_MIS_ZERO);
	    break;

	  default:
	    UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	    break;
	  }

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_FmtHdrFtr)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	EV_Menu_ItemState s = EV_MIS_ZERO;
	if(pView->getPoint() == 0)
	{
		return EV_MIS_Gray;
	}
	fp_Page * pPage = pView->getCurrentPage();
	if(pPage == NULL)
	{
		return EV_MIS_Gray;
	}
	fl_DocSectionLayout * pDSLP = pPage->getOwningSection();
	if(pDSLP == NULL)
	{
		return EV_MIS_Gray;
	}
	fl_BlockLayout * pBL = pView->getCurrentBlock();
	if(pBL == NULL)
	{
		return EV_MIS_Gray;
	}
	fl_DocSectionLayout	* pDSL = pBL->getDocSectionLayout();
	if(pDSL != pDSLP)
	{
		return EV_MIS_Gray;
	}
	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_CharFmt)
{
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	bool bMultiple = false;

	EV_Menu_ItemState s = EV_MIS_ZERO;

	const gchar * prop = NULL;
	const gchar * val  = NULL;

	if(pView->getDocument()->areStylesLocked() && !(AP_MENU_ID_FMT_SUPERSCRIPT == id || AP_MENU_ID_FMT_SUBSCRIPT == id)) {
          return EV_MIS_Gray;
	}

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

	case AP_MENU_ID_FMT_TOPLINE:
		prop = "text-decoration";
		val  = "topline";
		bMultiple = true;
		break;

	case AP_MENU_ID_FMT_BOTTOMLINE:
		prop = "text-decoration";
		val  = "bottomline";
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

	case AP_MENU_ID_FMT_DIRECTION_DO_RTL:
		prop = "dir-override";
		val  = "rtl";
		break;
		
	case AP_MENU_ID_FMT_DIRECTION_DO_LTR:
		prop = "dir-override";
		val  = "ltr";
		break;

	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	if (prop && val)
	{
		// get current font info from pView
		PP_PropertyVector props_in;

		if (!pView->getCharFormat(props_in))
			return s;

		const std::string & sz = PP_getAttribute(prop, props_in);
		if (!sz.empty())
		{
			if (bMultiple)
			{
				// some properties have multiple values
				if (sz.find(val) != std::string::npos)
					s = EV_MIS_Toggled;
			}
			else
			{
				if (sz == val)
					s = EV_MIS_Toggled;
			}
		}
	}


	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_BlockFmt)
{
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	const gchar * prop = "text-align";
	const gchar * val  = NULL;

	if(pView->getDocument()->areStylesLocked()) {
	    return EV_MIS_Gray;
	}

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

	case AP_MENU_ID_FMT_DIRECTION_DD_RTL:
		prop = "dom-dir";
		val  = "rtl";
		break;

	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	if (prop && val)
	{
		// get current font info from pView
		PP_PropertyVector props_in;

		if (!pView->getBlockFormat(props_in))
			return s;

		const std::string & sz = PP_getAttribute(prop, props_in);
		if (sz == val)
			s = EV_MIS_Toggled;
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_DocFmt)
{
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail( pDoc, EV_MIS_Gray );

	const PP_AttrProp * pAP = pDoc->getAttrProp();
	UT_return_val_if_fail( pAP, EV_MIS_Gray );
	
	const gchar * prop = NULL;
	const gchar * val  = NULL;

	if(pDoc->areStylesLocked()) {
	    return EV_MIS_Gray;
	}

	switch(id)
	{
		case AP_MENU_ID_FMT_DIRECTION_DOCD_RTL:
			prop = "dom-dir";
			val  = "rtl";
			break;

		default:
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			break;
	}

	if (prop && val)
	{
		const gchar * sz;

		if (!pAP->getProperty(prop, sz))
			return s;

		if (sz && (0 == strcmp(sz, val)))
			s = EV_MIS_Toggled;
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_SectFmt)
{
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	const gchar * prop = NULL;
	const gchar * val  = NULL;

	if(pView->getDocument()->areStylesLocked()) {
	    return EV_MIS_Gray;
	}

	switch(id)
	{
		case AP_MENU_ID_FMT_DIRECTION_SD_RTL:
			prop = "dom-dir";
			val  = "rtl";
			break;

		default:
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			break;
	}

	if (prop && val)
	{
		// get current font info from pView
		PP_PropertyVector props_in;

		if (!pView->getSectionFormat(props_in))
			return s;

		const std::string & sz = PP_getAttribute(prop, props_in);
		if (sz == val)
			s = EV_MIS_Toggled;
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_View)
{
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	XAP_Frame *pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_return_val_if_fail (pFrame, EV_MIS_Gray);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
	UT_return_val_if_fail (pFrameData, EV_MIS_Gray);

	XAP_App *pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, EV_MIS_Gray);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	switch(id)
	{
	case AP_MENU_ID_VIEW_RULER:
		if ( pFrameData->m_bShowRuler && !pFrameData->m_bIsFullScreen)
			s = EV_MIS_Toggled;
		else if( pFrameData->m_bIsFullScreen )
			s = EV_MIS_Gray;
		else
			s = EV_MIS_ZERO;
		break;
	case AP_MENU_ID_VIEW_SHOWPARA:
	        if ( pFrameData->m_bShowPara )
		  s = EV_MIS_Toggled;
		else
		  s = EV_MIS_ZERO;
		break;
	case AP_MENU_ID_VIEW_LOCKSTYLES:
		if ( pView->getDocument()->areStylesLocked() )
			s = EV_MIS_ZERO;
		else
			s = EV_MIS_Toggled;
		break;
	case AP_MENU_ID_VIEW_TB_1:
		if ( pFrameData->m_bShowBar[0] && !pFrameData->m_bIsFullScreen)
			s = EV_MIS_Toggled;
		else if( pFrameData->m_bIsFullScreen )
			s = EV_MIS_Gray;
		else
			s = EV_MIS_ZERO;
		break;
	case AP_MENU_ID_VIEW_TB_2:	
		if ( pFrameData->m_bShowBar[1] && !pFrameData->m_bIsFullScreen)
			s = EV_MIS_Toggled;
		else if( pFrameData->m_bIsFullScreen )
			s = EV_MIS_Gray;
		else
			s = EV_MIS_ZERO;
		break;
	case AP_MENU_ID_VIEW_TB_3:	
		if ( pFrameData->m_bShowBar[2] && !pFrameData->m_bIsFullScreen)
			s = EV_MIS_Toggled;
		else if( pFrameData->m_bIsFullScreen )
			s = EV_MIS_Gray;
		else
			s = EV_MIS_ZERO;
		break;
	case AP_MENU_ID_VIEW_TB_4:	
		if ( pFrameData->m_bShowBar[3] && !pFrameData->m_bIsFullScreen)
			s = EV_MIS_Toggled;
		else if( pFrameData->m_bIsFullScreen )
			s = EV_MIS_Gray;
		else
			s = EV_MIS_ZERO;
		break;
	case AP_MENU_ID_VIEW_LOCK_TB_LAYOUT:
		s = EV_MIS_Toggled;
		break;
	case AP_MENU_ID_VIEW_DEFAULT_TB_LAYOUT:
		s = EV_MIS_Gray;
		break;
	case AP_MENU_ID_VIEW_STATUSBAR:
              if ( pFrameData->m_bShowStatusBar &&
				   !pFrameData->m_bIsFullScreen )
				  s = EV_MIS_Toggled;
			  else if( pFrameData->m_bIsFullScreen )
				  s = EV_MIS_Gray;
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
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_InsTextBox)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if((pView->getViewMode() == VIEW_NORMAL) || (pView->getViewMode() == VIEW_WEB))
	{
            return EV_MIS_Gray;
	}

	return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_StylesLocked)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

        if(pView->getDocument()->areStylesLocked()) {
            return EV_MIS_Gray;
        }

        return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_History)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail( pDoc, EV_MIS_Gray );

	// disable for documents that have not been saved yet
	if(pDoc->getFilename().empty())
		return EV_MIS_Gray;
	
    return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_MarkRevisionsCheck)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if(pView->getDocument()->isAutoRevisioning())
	{
		return EV_MIS_Gray;
	}
	if(pView->getDocument()->isConnected())
	{
		return EV_MIS_Gray;
	}

	if(pView->isMarkRevisions())
	{
		return EV_MIS_Toggled;
	}

    return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_MarkRevisions)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if(pView->getDocument()->isAutoRevisioning())
	{
		return EV_MIS_Gray;
	}
	if(pView->getDocument()->isConnected())
	{
		return EV_MIS_Gray;
	}
	if(pView->isMarkRevisions())
	{
		return EV_MIS_ZERO;
	}

    return EV_MIS_Gray;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_RevisionsSelectLevel)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if(pView->getDocument()->isAutoRevisioning() || pView->isMarkRevisions())
	{
		return EV_MIS_Gray;
	}

    return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_HasRevisions)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if(pView->getDocument()->getHighestRevisionId() == 0)
		return EV_MIS_Gray;
	
	return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_AutoRevision)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	if(pView->getDocument()->isConnected())
	{
		return EV_MIS_Gray;
	}

	if(pView->getDocument()->isAutoRevisioning())
	{
		return (EV_Menu_ItemState) (EV_MIS_Toggled);
	}

	return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_ShowRevisions)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if(pView->getDocument()->isAutoRevisioning())
	{
		return EV_MIS_Gray;
	}
	if(pView->getDocument()->isConnected())
	{
		return EV_MIS_Gray;
	}
	   
	if(pView->getDocument()->getHighestRevisionId() == 0)
		return EV_MIS_Gray;
	
	if(pView->isShowRevisions())
	{
		return (EV_Menu_ItemState) (EV_MIS_Toggled | EV_MIS_Gray);
	}

	return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_ShowRevisionsAfter)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if(pView->getDocument()->isAutoRevisioning())
	{
		return EV_MIS_Gray;
	}
	if(pView->getDocument()->isConnected())
	{
		return EV_MIS_Gray;
	}

	if(pView->getDocument()->getHighestRevisionId() == 0)
		return EV_MIS_Gray;
	
	if(pView->isMarkRevisions())
	{
		if(pView->getRevisionLevel() == PD_MAX_REVISION)
			return EV_MIS_Toggled;
		else
			return EV_MIS_ZERO;
	}
	else if(!pView->isShowRevisions() && pView->getRevisionLevel() == PD_MAX_REVISION)
	{
		return (EV_Menu_ItemState) (EV_MIS_Toggled | EV_MIS_Gray);
	}

	return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_ShowRevisionsAfterPrev)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if(pView->getDocument()->isAutoRevisioning())
	{
		return EV_MIS_Gray;
	}

	if(pView->getDocument()->getHighestRevisionId() == 0)
		return EV_MIS_Gray;
	
	if(pView->isMarkRevisions())
	{
		if(pView->getDocument()->getHighestRevisionId() == pView->getRevisionLevel() + 1)
			return EV_MIS_Toggled;
		else
			return EV_MIS_ZERO;
	}
	else
		return EV_MIS_Gray;
	
	return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_ShowRevisionsBefore)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if(pView->getDocument()->isAutoRevisioning())
	{
		return EV_MIS_Gray;
	}

	if(pView->getDocument()->getHighestRevisionId() == 0)
		return EV_MIS_Gray;
	
	if(pView->isMarkRevisions())
	{
		// cannot hide revisions when in revisions mode
		return EV_MIS_Gray;
	}

	if(!pView->isShowRevisions() && pView->getRevisionLevel() == 0)
	{
		return (EV_Menu_ItemState) (EV_MIS_Toggled | EV_MIS_Gray);
	}

	return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_RevisionPresent)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if(!pView->doesSelectionContainRevision())
		return EV_MIS_Gray;

	return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_RevisionPresentContext)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if(pView->isMarkRevisions())
		return EV_MIS_Gray;

    return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_InTable)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if(pView->isInTable())
		return EV_MIS_ZERO;

    return EV_MIS_Gray;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_PointInTable)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if(pView->isInTable(pView->getPoint()))
		return EV_MIS_ZERO;

    return EV_MIS_Gray;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_PointOrAnchorInTable)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	bool bP = pView->isInTable(pView->getPoint());
	bool bA = pView->isInTable(pView->getSelectionAnchor());
	if(bP || bA)
		return EV_MIS_ZERO;

    return EV_MIS_Gray;
}


Defun_EV_GetMenuItemState_Fn(ap_GetState_InTableIsRepeat)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if(pView->isInTable())
	{
	        fp_CellContainer * pCCon = pView->getCellAtPos(pView->getPoint());
		if(pCCon && pCCon->isRepeated())
	        {
		     return EV_MIS_ZERO;
		}
		return EV_MIS_Gray;
	}
    return EV_MIS_Gray;
}


Defun_EV_GetMenuItemState_Fn(ap_GetState_TableOK)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if(pView->isInTable() && (pView->isHdrFtrEdit() || pView->isInHdrFtr(pView->getPoint())))
	{
		return EV_MIS_Gray;
	}
	else if( !pView->isSelectionEmpty() &&  pView->isInTable(pView->getPoint()) && pView->isHdrFtrEdit() )
	{
		return EV_MIS_Gray;
	}
	else if(pView->isInFootnote())
	{
		return EV_MIS_Gray;
	}
	else if(pView->isInAnnotation())
	{
		return EV_MIS_Gray;
	}
	else if(pView->isInEndnote())
	{
		return EV_MIS_Gray;
	}
	else if(pView->getHyperLinkRun(pView->getPoint()) != NULL)
	{
		return EV_MIS_Gray;
	}
	else if(pView->getFrameEdit() && pView->getFrameEdit()->isActive())
	{
		fl_FrameLayout * pFL = pView->getFrameLayout();
		if(pFL && (pFL->getFrameType() == FL_FRAME_WRAPPER_IMAGE))
		{
			return EV_MIS_Gray;
		}
	}
    return EV_MIS_ZERO;
}


Defun_EV_GetMenuItemState_Fn(ap_GetState_InTOC)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	if(pView->isTOCSelected())
	{
		return EV_MIS_ZERO;
	}
	return EV_MIS_Gray;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_InTableMerged)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	if(pView->isInTable())
	{
		return EV_MIS_ZERO;
	}
    return EV_MIS_Gray;
}


Defun_EV_GetMenuItemState_Fn(ap_GetState_InFootnote)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	if(pView->getHyperLinkRun(pView->getPoint()) != NULL)
	{
		return EV_MIS_Gray;
	}
	else if(pView->getEmbedDepth(pView->getPoint()) > 0)
	{
		return EV_MIS_Gray;
	}
	else if(pView->getFrameEdit()->isActive())
	{
	        return EV_MIS_Gray;
	}
	if(!pView->isInFootnote() && !pView->isInAnnotation() && !pView->isHdrFtrEdit() && !pView->isInHdrFtr(pView->getPoint()) && !pView->isInFrame(pView->getPoint()) 
		&& !pView->isTOCSelected())
	{
		return EV_MIS_ZERO;
	}
	return EV_MIS_Gray;

}


Defun_EV_GetMenuItemState_Fn(ap_GetState_InAnnotation)
{
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	if((id==AP_MENU_ID_TOOLS_ANNOTATIONS_INSERT_FROMSEL) &&
	   (pView->isSelectionEmpty()))
	{
		return EV_MIS_Gray;
	}
	PT_DocPosition point = pView->getPoint();
	PT_DocPosition anchor = pView->getSelectionAnchor();
	if((pView->getHyperLinkRun(point) != NULL) || (pView->getHyperLinkRun(anchor) != NULL))
	{
		return EV_MIS_Gray;
	}
	else if((pView->getEmbedDepth(point) > 0) || (pView->getEmbedDepth(anchor) > 0))
	{
		return EV_MIS_Gray;
	}
	else if(pView->getFrameEdit()->isActive())
	{
	        return EV_MIS_Gray;
	}
	if(!pView->isInFootnote() &&!pView->isInAnnotation() && !pView->isHdrFtrEdit() && !pView->isInHdrFtr(point) && !pView->isInFrame(point)  && !pView->isInFrame(anchor) 
		&& !pView->isTOCSelected())
	{
		return EV_MIS_ZERO;
	}
	return EV_MIS_Gray;

}

// RIVERA
Defun_EV_GetMenuItemState_Fn(ap_GetState_ToggleAnnotations)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	XAP_App *pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, EV_MIS_Gray);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail (pPrefs, EV_MIS_Gray);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_return_val_if_fail(pScheme, EV_MIS_Gray);
	bool b = false;
	pScheme->getValueBool(static_cast<const gchar *>(AP_PREF_KEY_DisplayAnnotations), &b );
	return (b ? EV_MIS_Toggled : EV_MIS_ZERO);
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_ToggleRDFAnchorHighlight)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	XAP_App *pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, EV_MIS_Gray);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail (pPrefs, EV_MIS_Gray);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_return_val_if_fail(pScheme, EV_MIS_Gray);
	bool b = false;
	pScheme->getValueBool(static_cast<const gchar *>(AP_PREF_KEY_DisplayRDFAnchors), &b );
	return (b ? EV_MIS_Toggled : EV_MIS_ZERO);
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_InImage)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	if(pView->isImageSelected())
	{
		return EV_MIS_ZERO;
	}
	if(pView->getFrameEdit()->isActive())
	{
	        fl_FrameLayout * pFL = pView->getFrameLayout();
		if(pFL && pFL->getFrameType() == FL_FRAME_TEXTBOX_TYPE)
		{
		        return EV_MIS_Gray;
		}
		return EV_MIS_ZERO;
	}
	return EV_MIS_Gray;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_SetPosImage)
{
	UT_UNUSED(id);
        ABIWORD_VIEW;
  	UT_return_val_if_fail (pView, EV_MIS_Gray);
	bool bCont= false;
	if(pView->isImageSelected())
	{
		bCont = true;
	}
	if(!bCont && pView->getFrameEdit()->isActive())
	{
	        fl_FrameLayout * pFL = pView->getFrameLayout();
		if(pFL && pFL->getFrameType() == FL_FRAME_TEXTBOX_TYPE)
		{
		        return EV_MIS_Gray;
		}
		bCont = true;
	}
	if(!bCont)
	{
	  return EV_MIS_Gray;
	}
	if(pView->isHdrFtrEdit() || pView->isInHdrFtr(pView->getPoint()))
	{
		return EV_MIS_Gray;
	}
	return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_InFrame)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	if(pView->isInFrame(pView->getPoint()))
	{
		return EV_MIS_ZERO;
	}
	return EV_MIS_Gray;
}


Defun_EV_GetMenuItemState_Fn(ap_GetState_BreakOK)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	
	if(pView->isInFootnote())
	{
		return EV_MIS_Gray;
	}
	if(pView->isInFootnote(pView->getSelectionAnchor()))
	{
		return EV_MIS_Gray;
	}
	if(pView->isInAnnotation())
	{
		return EV_MIS_Gray;
	}
	if(pView->isInAnnotation(pView->getSelectionAnchor()))
	{
		return EV_MIS_Gray;
	}
	if(pView->isInEndnote())
	{
		return EV_MIS_Gray;
	}
	if(pView->isInEndnote(pView->getSelectionAnchor()))
	{
		return EV_MIS_Gray;
	}
	else if(pView->isInFrame(pView->getPoint()))
	{
		return EV_MIS_Gray;
	}
	else if(pView->isInFrame(pView->getSelectionAnchor()))
	{
		return EV_MIS_Gray;
	}
	else if(pView->isInTable())
	{
		return EV_MIS_Gray;
	}
	else if(pView->getFrameEdit()->isActive())
	{
	        return EV_MIS_Gray;
	}
	else if(pView->isInTable(pView->getSelectionAnchor()))
	{
		return EV_MIS_Gray;
	}
	else if(pView->isHdrFtrEdit() || pView->isInHdrFtr(pView->getPoint()))
	{
		return EV_MIS_Gray;
	}
	else if(pView->getSelectionMode() >= FV_SelectionMode_Multiple)
	{
		return EV_MIS_Gray;
	}
	else if(pView->getHyperLinkRun(pView->getPoint()) != NULL)
	{
		return EV_MIS_Gray;
	}
	else
	{
		return EV_MIS_ZERO;
	}
}

// HACK TO ALWAYS DISABLE A MENU ITEM... DELETE ME
Defun_EV_GetMenuItemState_Fn(ap_GetState_AlwaysDisabled)
{
	UT_UNUSED(pAV_View);
	UT_UNUSED(id);
    return EV_MIS_Gray;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_Recent)
{
	UT_UNUSED(pAV_View);
	UT_UNUSED(id);
  // ABIWORD_VIEW;
  // UT_return_val_if_fail(pView, EV_MIS_ZERO);

	XAP_Prefs *pPrefs = XAP_App::getApp()->getPrefs();
	UT_return_val_if_fail(pPrefs, EV_MIS_ZERO);

	if(pPrefs->getRecentCount() > 0)
		return EV_MIS_ZERO;
	
	return EV_MIS_Gray;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_Zoom)
{
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, EV_MIS_ZERO);

	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_return_val_if_fail (pFrame, EV_MIS_Gray);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	switch(id)
	{
	case AP_MENU_ID_VIEW_ZOOM_200:
		if (pFrame->getZoomPercentage() == 200 && (pFrame->getZoomType() == XAP_Frame::z_PERCENT || pFrame->getZoomType() == XAP_Frame::z_200))
			s = EV_MIS_Toggled;
		break;
	case AP_MENU_ID_VIEW_ZOOM_100:
		if (pFrame->getZoomPercentage() == 100 && (pFrame->getZoomType() == XAP_Frame::z_PERCENT || pFrame->getZoomType() == XAP_Frame::z_100))
			s = EV_MIS_Toggled;
		break;
	case AP_MENU_ID_VIEW_ZOOM_75:
		if (pFrame->getZoomPercentage() == 75 && (pFrame->getZoomType() == XAP_Frame::z_PERCENT || pFrame->getZoomType() == XAP_Frame::z_75))
			s = EV_MIS_Toggled;
		break;
	case AP_MENU_ID_VIEW_ZOOM_50:
		if (pFrame->getZoomPercentage() == 50 && pFrame->getZoomType() == XAP_Frame::z_PERCENT)
			s = EV_MIS_Toggled;
		break;
	case AP_MENU_ID_VIEW_ZOOM_WHOLE:
		xxx_UT_DEBUGMSG(("Whole: %d %d\n", pFrame->getZoomPercentage(), pView->calculateZoomPercentForWholePage()));
		if (pFrame->getZoomType() == XAP_Frame::z_WHOLEPAGE)
			s = EV_MIS_Toggled;
		break;
	case AP_MENU_ID_VIEW_ZOOM_WIDTH:
		xxx_UT_DEBUGMSG(("Width: %d %d\n", pFrame->getZoomPercentage(), pView->calculateZoomPercentForPageWidth()));
		if (pFrame->getZoomType() == XAP_Frame::z_PAGEWIDTH)
			s = EV_MIS_Toggled;
		break;
	default:
		UT_ASSERT_NOT_REACHED ();
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_Lists)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, EV_MIS_ZERO);

	if(pView->getDocument()->areStylesLocked() || pView->isHdrFtrEdit()  || pView->isInHdrFtr(pView->getPoint()))
	{
		return EV_MIS_Gray;
	}

	return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_MailMerge)
{
	UT_UNUSED(id);
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, EV_MIS_ZERO);
	
	if (0 == IE_MailMerge::getMergerCount ())
		return EV_MIS_Gray;
	return EV_MIS_ZERO;
}
