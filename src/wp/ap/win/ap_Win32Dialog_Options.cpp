/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *			 (c) 2002 Jordi Mas i Hern�ndez jmas@softcatala.org
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
 
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Win32OS.h"

#include "gr_Win32Graphics.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "xap_Prefs.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"
#include "ap_Win32App.h"
#include "ap_Win32Resources.rc2"
#include "ap_Win32Dialog_Options.h"
#include "ap_Win32Dialog_Background.h"
#include "xap_Win32DialogHelper.h"
#include "fp_PageSize.h"
#include "ut_Language.h"

/*****************************************************************/

#define GWL(hwnd)		(AP_Win32Dialog_Options*)GetWindowLong((hwnd), DWL_USER)
#define SWL(hwnd, d)	(AP_Win32Dialog_Options*)SetWindowLong((hwnd), DWL_USER,(LONG)(d))
#define MINAUTOSAVEPERIOD	1
#define MAXAUTOSAVEPERIOD	120

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Options::static_constructor(XAP_DialogFactory * pFactory,
														XAP_Dialog_Id id)
{
	AP_Win32Dialog_Options * p = new AP_Win32Dialog_Options(pFactory,id);
	return p;
}

AP_Win32Dialog_Options::AP_Win32Dialog_Options(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_Options(pDlgFactory,id),m_pDialogFactory(pDlgFactory)
{
	m_pVecUILangs = NULL;
}

AP_Win32Dialog_Options::~AP_Win32Dialog_Options(void)
{
	if (m_pVecUILangs)
	{		
		for (UT_uint32 i=0; i < m_pVecUILangs->getItemCount(); i++)
			delete (char *)m_pVecUILangs->getNthItem(i);
			
		delete m_pVecUILangs;		
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void AP_Win32Dialog_Options::_initEnableControlsPlatformSpecific()
{
	_controlEnable( id_CHECK_LANG_WITH_KEYBOARD, true); 
	_controlEnable( id_CHECK_DIR_MARKER_AFTER_CLOSING_PARENTHESIS,_gatherLanguageWithKeyboard());  
}


void AP_Win32Dialog_Options::runModal(XAP_Frame * pFrame)
{	
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);	
	AP_Win32Dialog_Options_Sheet	sheet;	
	
	m_pFrame = pFrame;
	UT_ASSERT(pFrame);	
	
	/* Create the property sheet and associate its pages*/		
	m_toolbars.setContainer(this);	
	m_toolbars.createPage(pWin32App, AP_RID_DIALOG_OPT_TOOLBARS, AP_STRING_ID_DLG_Options_Label_Toolbars);	
	sheet.addPage(&m_toolbars);		
	
	m_lang.setContainer(this);	
	m_lang.createPage(pWin32App, AP_RID_DIALOG_OPT_LANGUAGE, AP_STRING_ID_DLG_Options_Label_Language);	
	sheet.addPage(&m_lang);			
	
	m_spelling.setContainer(this);	
	m_spelling.createPage(pWin32App, AP_RID_DIALOG_OPT_SPELL, AP_STRING_ID_DLG_Options_TabLabel_Spelling);	
	sheet.addPage(&m_spelling);		

	m_layout.setContainer(this);	
	m_layout.createPage(pWin32App, AP_RID_DIALOG_OPT_LAYOUT, AP_STRING_ID_DLG_Options_Label_Layout);	
	sheet.addPage(&m_layout);				

	
	m_pref.setContainer(this);	
	m_pref.createPage(pWin32App, AP_RID_DIALOG_OPT_PREF, AP_STRING_ID_DLG_Options_Label_Schemes);	
	sheet.addPage(&m_pref);				
	
	sheet.setApplyButton(true);       
	sheet.setParent(this);	       
	if (sheet.runModal(pWin32App, pFrame, AP_STRING_ID_DLG_Options_OptionsTitle)==IDOK)	
		m_answer = a_OK;
	else		
		m_answer = a_CANCEL;
}	

struct {
	UT_Dimension  dim;
	int 		  id;
} s_aAlignUnit[] =
{
	{ DIM_IN, XAP_STRING_ID_DLG_Unit_inch },
	{ DIM_CM, XAP_STRING_ID_DLG_Unit_cm },
	{ DIM_PT, XAP_STRING_ID_DLG_Unit_points },
	{ DIM_PI, XAP_STRING_ID_DLG_Unit_pica },
};
#define SIZE_aAlignUnit  (sizeof(s_aAlignUnit)/sizeof(s_aAlignUnit[0]))
#define _CDB(c,i)	CheckDlgButton(hWnd,AP_RID_DIALOG_##c,_getCheckItemValue(i))


/*
	Gets the property page by its index
*/
HWND	AP_Win32Dialog_Options::getPage(PSH_PAGES page)
{
	HWND hWnd = NULL;
	
	switch (page)
	{
		case PG_TOOLBARS:
		hWnd = m_toolbars.getHandle();
		break;					
		
		case PG_LANG:
		hWnd = m_lang.getHandle();
		break;				
		
		case PG_SPELL:
		hWnd = m_spelling.getHandle();
		break;				
		
		case PG_LAYOUT:
		hWnd = m_layout.getHandle();
		break;				
		
		case PG_PREF:
		hWnd = m_pref.getHandle();
		break;				
			
		default:
			break;
	}
	
	UT_ASSERT(hWnd!=NULL);
	
	return hWnd;
}


/*

*/
void AP_Win32Dialog_Options::_controlEnable( tControl id, bool value )
{
	// This routine is called by XP code to enable/disable a particular field.

	switch (id)
	{
	case id_CHECK_VIEW_SHOW_STANDARD_TOOLBAR:
		EnableWindow(GetDlgItem((HWND)getPage(PG_TOOLBARS),AP_RID_DIALOG_OPTIONS_CHK_ViewShowStandardBar),value);
		return;

	case id_CHECK_VIEW_SHOW_FORMAT_TOOLBAR:
		EnableWindow(GetDlgItem((HWND)getPage(PG_TOOLBARS),AP_RID_DIALOG_OPTIONS_CHK_ViewShowFormatBar),value);
		return;

	case id_CHECK_VIEW_SHOW_EXTRA_TOOLBAR:
		EnableWindow(GetDlgItem((HWND)getPage(PG_TOOLBARS),AP_RID_DIALOG_OPTIONS_CHK_ViewShowExtraBar),value);
		return;

	case id_CHECK_SPELL_CHECK_AS_TYPE:
		EnableWindow(GetDlgItem((HWND)getPage(PG_SPELL),AP_RID_DIALOG_OPTIONS_CHK_SpellCheckAsType),value);
		return;

	case id_CHECK_SPELL_HIDE_ERRORS:
		EnableWindow(GetDlgItem((HWND)getPage(PG_SPELL),AP_RID_DIALOG_OPTIONS_CHK_SpellHideErrors),value);
		return;

	case id_CHECK_SPELL_SUGGEST:
		EnableWindow(GetDlgItem((HWND)getPage(PG_SPELL),AP_RID_DIALOG_OPTIONS_CHK_SpellSuggest),value);
		return;

	case id_CHECK_SPELL_MAIN_ONLY:
		EnableWindow(GetDlgItem((HWND)getPage(PG_SPELL),AP_RID_DIALOG_OPTIONS_CHK_SpellMainOnly),value);
		return;

	case id_CHECK_SPELL_UPPERCASE:
		EnableWindow(GetDlgItem((HWND)getPage(PG_SPELL),AP_RID_DIALOG_OPTIONS_CHK_SpellUppercase),value);
		return;

	case id_CHECK_SPELL_NUMBERS:
		EnableWindow(GetDlgItem((HWND)getPage(PG_SPELL),AP_RID_DIALOG_OPTIONS_CHK_SpellNumbers),value);
		return;

	case id_CHECK_SPELL_INTERNET:
		EnableWindow(GetDlgItem((HWND)getPage(PG_SPELL),AP_RID_DIALOG_OPTIONS_CHK_SpellInternet),value);
		return;

	case id_CHECK_VIEW_SHOW_RULER:
		EnableWindow(GetDlgItem((HWND)getPage(PG_LAYOUT),AP_RID_DIALOG_OPTIONS_CHK_ViewShowRuler),value);
		return;

	case id_CHECK_VIEW_CURSOR_BLINK:
		EnableWindow(GetDlgItem((HWND)getPage(PG_LAYOUT),AP_RID_DIALOG_OPTIONS_CHK_ViewCursorBlink),value);
		return;

	case id_CHECK_VIEW_SHOW_STATUS_BAR:
		EnableWindow(GetDlgItem((HWND)getPage(PG_LAYOUT),AP_RID_DIALOG_OPTIONS_CHK_ViewShowStatusBar),value);
		return;

	case id_CHECK_VIEW_ALL:
		EnableWindow(GetDlgItem((HWND)getPage(PG_LAYOUT),AP_RID_DIALOG_OPTIONS_CHK_ViewAll),value);
		return;

	case id_CHECK_VIEW_HIDDEN_TEXT:
		EnableWindow(GetDlgItem((HWND)getPage(PG_LAYOUT),AP_RID_DIALOG_OPTIONS_CHK_ViewHiddenText),value);
		return;

	case id_CHECK_VIEW_UNPRINTABLE:
		EnableWindow(GetDlgItem((HWND)getPage(PG_LAYOUT),AP_RID_DIALOG_OPTIONS_CHK_ViewUnprintable),value);
		return;

	case id_CHECK_PREFS_AUTO_SAVE:
		EnableWindow(GetDlgItem((HWND)getPage(PG_PREF),AP_RID_DIALOG_OPTIONS_CHK_PrefsAutoSave),value);
		return;

	case id_CHECK_SMART_QUOTES_ENABLE:
		EnableWindow(GetDlgItem((HWND)getPage(PG_LAYOUT),id_CHECK_SMART_QUOTES_ENABLE),value);
		return;

	case id_SHOWSPLASH:
		EnableWindow(GetDlgItem((HWND)getPage(PG_PREF),AP_RID_DIALOG_OPTIONS_CHK_ShowSplash),value);
		return;

	case id_CHECK_OTHER_SAVE_CONTEXT_GLYPHS:
		EnableWindow(GetDlgItem((HWND)getPage(PG_LANG),AP_RID_DIALOG_OPTIONS_CHK_OtherSaveContextGlyphs),value);
		return;

	case id_CHECK_OTHER_HEBREW_CONTEXT_GLYPHS:
		EnableWindow(GetDlgItem((HWND)getPage(PG_LANG),AP_RID_DIALOG_OPTIONS_CHK_OtherHebrewContextGlyphs),value);
		return;

	case id_CHECK_AUTO_SAVE_FILE:
		EnableWindow(GetDlgItem((HWND)getPage(PG_PREF),AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile),value);
		return;

	case id_CHECK_LANG_WITH_KEYBOARD:
		EnableWindow(GetDlgItem((HWND)getPage(PG_LANG),AP_RID_DIALOG_OPTIONS_CHK_LanguageWithKeyboard),value);
		return;

		case id_CHECK_DIR_MARKER_AFTER_CLOSING_PARENTHESIS:
		EnableWindow(GetDlgItem((HWND)getPage(PG_LANG),AP_RID_DIALOG_OPTIONS_CHK_DirMarkerAfterClosingParenthesis),value);
		return;
		
		
	default:
//		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// The following defines a specific, named "Get" and "Set" method
// for each CHECK box on all of the sub-dialogs.  These are called
// by XP code to set a particular value in the dialog presentation;
// they do not call back into XP code.


#define DEFINE2_GET_SET_BOOL(index,button)																	\
	bool AP_Win32Dialog_Options::_gather##button(void)													\
	{ return (IsDlgButtonChecked((HWND)getPage(index),AP_RID_DIALOG_OPTIONS_CHK_##button) == BST_CHECKED); } \
	void AP_Win32Dialog_Options::_set##button(const bool b) 												\
	{ CheckDlgButton((HWND)getPage(index),AP_RID_DIALOG_OPTIONS_CHK_##button,b); }

DEFINE2_GET_SET_BOOL(PG_TOOLBARS,ViewShowStandardBar);
DEFINE2_GET_SET_BOOL(PG_TOOLBARS,ViewShowFormatBar);
DEFINE2_GET_SET_BOOL(PG_TOOLBARS,ViewShowExtraBar);


DEFINE2_GET_SET_BOOL(PG_SPELL,SpellCheckAsType);
DEFINE2_GET_SET_BOOL(PG_SPELL,SpellHideErrors);
DEFINE2_GET_SET_BOOL(PG_SPELL,SpellSuggest);
DEFINE2_GET_SET_BOOL(PG_SPELL,SpellMainOnly);
DEFINE2_GET_SET_BOOL(PG_SPELL,SpellUppercase);
DEFINE2_GET_SET_BOOL(PG_SPELL,SpellNumbers);
DEFINE2_GET_SET_BOOL(PG_SPELL,SpellInternet);

DEFINE2_GET_SET_BOOL(PG_LAYOUT,ViewShowRuler);
DEFINE2_GET_SET_BOOL(PG_LAYOUT,ViewShowStatusBar);
DEFINE2_GET_SET_BOOL(PG_LAYOUT,ViewCursorBlink);
DEFINE2_GET_SET_BOOL(PG_LAYOUT,ViewAll);
DEFINE2_GET_SET_BOOL(PG_LAYOUT,ViewHiddenText);
DEFINE2_GET_SET_BOOL(PG_LAYOUT,ViewUnprintable);
DEFINE2_GET_SET_BOOL(PG_LAYOUT,SmartQuotesEnable);

DEFINE2_GET_SET_BOOL(PG_PREF,PrefsAutoSave);
DEFINE2_GET_SET_BOOL(PG_PREF,ShowSplash);
DEFINE2_GET_SET_BOOL(PG_LANG,OtherDirectionRtl);
DEFINE2_GET_SET_BOOL(PG_LANG,OtherSaveContextGlyphs);
DEFINE2_GET_SET_BOOL(PG_LANG,OtherHebrewContextGlyphs);
DEFINE2_GET_SET_BOOL(PG_LANG,LanguageWithKeyboard);
DEFINE2_GET_SET_BOOL(PG_LANG,DirMarkerAfterClosingParenthesis);

#undef DEFINE_GET_SET_BOOL

// TODO:  Add these item to dialog and implement
bool AP_Win32Dialog_Options::_gatherAllowCustomToolbars(void)
{
	return false;
}

void AP_Win32Dialog_Options::_setAllowCustomToolbars(const bool b)
{
}

// TODO:  Add these item to dialog and impliment
bool AP_Win32Dialog_Options::_gatherAutoLoadPlugins(void)
{
	return false;
}

void AP_Win32Dialog_Options::_setAutoLoadPlugins(const bool b)
{
}

bool AP_Win32Dialog_Options::_gatherAutoSaveFile(void)
{
	return (IsDlgButtonChecked((HWND)getPage(PG_PREF),AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile) == BST_CHECKED);
}

void AP_Win32Dialog_Options::_setAutoSaveFile(const bool b)
{
	BOOL bChecked;
	HWND hWnd = (HWND)getPage(PG_PREF);

	CheckDlgButton( hWnd, AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile, b );

	// Disable the input boxes if auto save is turned off
	bChecked = (IsDlgButtonChecked( hWnd, AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile ) == BST_CHECKED);
	EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod), bChecked );
	EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_SPN_AutoSavePeriodSpin), bChecked );
	EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_TXT_AutoSaveExtension), bChecked );
}

void AP_Win32Dialog_Options::_gatherAutoSaveFileExt(UT_String &stRetVal)
{
	char szExtension[ 10 ];

	GetDlgItemTextA( (HWND)getPage(PG_PREF), AP_RID_DIALOG_OPTIONS_TXT_AutoSaveExtension, szExtension, 7 ); //!TODO Using ANSI function

	stRetVal = szExtension;
}

void AP_Win32Dialog_Options::_setAutoSaveFileExt(const UT_String &stExt)
{
	SetDlgItemTextA( (HWND)getPage(PG_PREF), AP_RID_DIALOG_OPTIONS_TXT_AutoSaveExtension, stExt.c_str() ); //!TODO Using ANSI function
}

void AP_Win32Dialog_Options::_gatherAutoSaveFilePeriod(UT_String &stRetVal)
{
	int iValue = GetDlgItemInt((HWND)getPage(PG_PREF), AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod, NULL, FALSE );
	char szTemp[10];

	snprintf( szTemp, 10, "%d", iValue );

	stRetVal = szTemp;
}

void AP_Win32Dialog_Options::_setAutoSaveFilePeriod(const UT_String &stPeriod)
{
	SetDlgItemInt((HWND)getPage(PG_PREF), AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod, atoi(stPeriod.c_str()), FALSE );
}

UT_Dimension AP_Win32Dialog_Options::_gatherViewRulerUnits(void)
{
	HWND hwndAlign = GetDlgItem((HWND)getPage(PG_LAYOUT), AP_RID_DIALOG_OPTIONS_COMBO_UNITS);
	int nSel = SendMessage(hwndAlign, CB_GETCURSEL, 0, 0);

	if( nSel != CB_ERR )
		return s_aAlignUnit[nSel].dim;
	return DIM_IN;
}

void  AP_Win32Dialog_Options::_setViewRulerUnits(UT_Dimension dim)
{
	int n1;
	for( n1 = 0; n1 < SIZE_aAlignUnit; n1++ )
		if( s_aAlignUnit[n1].dim == dim )
			break;
	if( n1 == SIZE_aAlignUnit )
		n1 = 0;

	HWND hwndAlign = GetDlgItem((HWND)getPage(PG_LAYOUT), AP_RID_DIALOG_OPTIONS_COMBO_UNITS);
	SendMessage(hwndAlign, CB_SETCURSEL, (WPARAM)n1, 0);
}

int AP_Win32Dialog_Options::_gatherNotebookPageNum(void)
{
	return 0;
}

void AP_Win32Dialog_Options::_setNotebookPageNum(const int pn)
{
}

void AP_Win32Dialog_Options::_setDefaultPageSize(fp_PageSize::Predefined pre)
{
	HWND hwndPaperSize = GetDlgItem((HWND)getPage(PG_LAYOUT),
									 AP_RID_DIALOG_OPTIONS_COMBO_DefaultPageSize);
	SendMessage(hwndPaperSize, CB_SETCURSEL, (WPARAM)pre, 0);
}

fp_PageSize::Predefined AP_Win32Dialog_Options::_gatherDefaultPageSize(void)
{
	HWND hwndPaperSize = GetDlgItem( (HWND)getPage(PG_LAYOUT),
									 AP_RID_DIALOG_OPTIONS_COMBO_DefaultPageSize);
	int nSel = SendMessage(hwndPaperSize, CB_GETCURSEL, 0, 0);

	if( nSel != CB_ERR )
		return (fp_PageSize::Predefined) nSel;
	return fp_PageSize::psLetter;
}

void AP_Win32Dialog_Options::_initializeTransperentToggle(void)
{
	HWND hWnd = (HWND)getPage(PG_LAYOUT);

	// Initialize the "Allow screen colors" checkbox
	if( UT_strcmp( m_CurrentTransparentColor, "ffffff" ) )
	{
		CheckDlgButton( hWnd, AP_RID_DIALOG_OPTIONS_CHK_BGColorEnable, true );
		EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_BTN_BGColor), true );
	}
	else
	{
		CheckDlgButton( hWnd, AP_RID_DIALOG_OPTIONS_CHK_BGColorEnable, false );
		EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_BTN_BGColor), false );
	}
}


void AP_Win32Dialog_Options::_gatherUILanguage(UT_String &stRetVal)
{
	HWND		hCtrlDocLang	= GetDlgItem( (HWND)getPage(PG_LANG), AP_RID_DIALOG_OPTIONS_COMBO_UILANG);	
	UT_Language	lang;
	const char* pLang;
	int nIndex;			 	
	
	nIndex = SendMessage(hCtrlDocLang,  CB_GETCURSEL , 0,0);
	
	if (nIndex!=CB_ERR)
	{
		int nID = SendMessage(hCtrlDocLang,  CB_GETITEMDATA , nIndex,0);
		pLang =  (const char*)lang.getNthLangCode(nID);		
		stRetVal = pLang;
	}				
	else
		stRetVal.clear();
}

void AP_Win32Dialog_Options::_setUILanguage(const UT_String &stExt)
{
	UT_Language	lang;
	int id = lang.getIndxFromCode(stExt.c_str());
	HWND hCtrlDocLang	= GetDlgItem((HWND)getPage(PG_LANG), AP_RID_DIALOG_OPTIONS_COMBO_UILANG);	

	int nCount = SendMessage(hCtrlDocLang, CB_GETCOUNT, 0, 0);		
	
	for (int i=0; i<nCount;i++)
	{
		if (SendMessage(hCtrlDocLang,  CB_GETITEMDATA , i,0)==id)
		{
			SendMessage(hCtrlDocLang, CB_SETCURSEL, i, 0);				
			break;
		}
	}		
}



/*

	Sheet
	
*/
AP_Win32Dialog_Options_Sheet::AP_Win32Dialog_Options_Sheet() :
XAP_Win32PropertySheet()
{
	m_pParent = NULL;
	setCallBack(s_sheetInit);
}



/*
	Sheet window procedure
*/
int AP_Win32Dialog_Options_Sheet::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wID = LOWORD(wParam); 
	
	if (wID==IDOK)
	{			
		AP_Win32Dialog_Options_Sheet * t = (AP_Win32Dialog_Options_Sheet *) GetWindowLong(hWnd, GWL_USERDATA);					
		HWND hWndPref = t->getParent()->getPage(PG_PREF);
		AP_Win32Dialog_Options_Pref * prefPag = (AP_Win32Dialog_Options_Pref *) GetWindowLong(hWndPref, GWL_USERDATA);							
		
		if (!prefPag->isAutoSaveInRange()) return 0;
		
		if(IsDlgButtonChecked((HWND)t->getParent()->getPage(PG_LAYOUT), AP_RID_DIALOG_OPTIONS_CHK_BGColorEnable ) != BST_CHECKED )
			t->getParent()->_setColorForTransparent("ffffff");
			
		t->getParent()->_storeWindowData(); 					// remember current settings
	}
	
	if (wID==ID_APPLY_NOW)	// Save default button
	{
		AP_Win32Dialog_Options_Sheet * t = (AP_Win32Dialog_Options_Sheet *) GetWindowLong(hWnd, GWL_USERDATA);					
		t->getParent()->_event_SetDefaults();				
		return 0;
	}
	
	return 1;	// The application did not process the message
}

int CALLBACK AP_Win32Dialog_Options_Sheet::s_sheetInit(HWND hwnd,  UINT uMsg,  LPARAM lParam)
{	
	if (uMsg==PSCB_INITIALIZED)
	{	
		/* Force the creation of all pages*/
		PropSheet_SetCurSel(hwnd, 0,0);
		PropSheet_SetCurSel(hwnd, 0,1);
		PropSheet_SetCurSel(hwnd, 0,2);
		PropSheet_SetCurSel(hwnd, 0,3);
		PropSheet_SetCurSel(hwnd, 0,4);					
	}			
	return 	0;
}


/*
	
*/
void AP_Win32Dialog_Options_Sheet::_onInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam)
{		
	UT_DEBUGMSG(("AP_Win32Dialog_Options_Sheet::_onInitDialog"));
	// let XP code tell us what all of the values should be.
	getParent()->_populateWindowData();
	getParent()->_initializeTransperentToggle();	
	
	// Apply button -> save 
	const XAP_StringSet * pSS = getParent()->getApp()->getStringSet();	
	EnableWindow(GetDlgItem(hwnd, ID_APPLY_NOW), TRUE);
	SetWindowText(GetDlgItem(hwnd, ID_APPLY_NOW), XAP_Win32App::getWideString(pSS->getValue(AP_STRING_ID_DLG_Options_Btn_Default))); 
	SetWindowText(GetDlgItem(hwnd, IDOK), XAP_Win32App::getWideString(pSS->getValue(XAP_STRING_ID_DLG_OK))); 
	SetWindowText(GetDlgItem(hwnd, IDCANCEL), XAP_Win32App::getWideString(pSS->getValue(XAP_STRING_ID_DLG_Cancel)));	

	AP_Win32Dialog_Options*	 pParent=  (AP_Win32Dialog_Options*)getParent();		
	int nDefPage = pParent->getInitialPageNum();		
	PropSheet_SetCurSel(hwnd, 0, nDefPage);	
	
}

/*

	Toolbar page
	
*/

AP_Win32Dialog_Options_Toolbars::AP_Win32Dialog_Options_Toolbars()
{
	setDialogProc(s_pageWndProc);	
	m_nCentered = 0;
}

AP_Win32Dialog_Options_Toolbars::~AP_Win32Dialog_Options_Toolbars()
{
	
}


/*
	
*/	
int CALLBACK AP_Win32Dialog_Options_Toolbars::s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	
	if (msg==WM_NOTIFY)
	{
		AP_Win32Dialog_Options_Toolbars *pThis = (AP_Win32Dialog_Options_Toolbars *) GetWindowLong(hWnd, GWL_USERDATA);					
		
		
		NMHDR* pHdr = (NMHDR*)lParam;

		if (pHdr->code==PSN_SETACTIVE)			
		{
			if (pThis->m_nCentered<2)
			{
			   	pThis->m_nCentered++;
				XAP_Win32DialogHelper::s_centerDialog(GetParent(hWnd));			
			}
		}
	}   	
	
	return XAP_Win32PropertyPage::s_pageWndProc(hWnd, msg, wParam,lParam);
}
/*
	
*/	
BOOL AP_Win32Dialog_Options_Toolbars::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;	
	AP_Win32Dialog_Options*	 pParent=  (AP_Win32Dialog_Options*)getContainer();	
	
	switch (wId)
	{		
		case AP_RID_DIALOG_OPTIONS_CHK_ViewShowStandardBar: 	
			pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_VIEW_SHOW_STANDARD_TOOLBAR);	
			return 0;
		case AP_RID_DIALOG_OPTIONS_CHK_ViewShowFormatBar:	
			pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_VIEW_SHOW_FORMAT_TOOLBAR); 	
			return 0;
		case AP_RID_DIALOG_OPTIONS_CHK_ViewShowExtraBar:		
			pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_VIEW_SHOW_EXTRA_TOOLBAR);		
			return 0;
		
		default:
		break;
	}
	return 1;
}
/*
	
*/	
BOOL AP_Win32Dialog_Options_Toolbars::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{				
	const XAP_StringSet * pSS = getApp()->getStringSet();
	setStringSet(pSS);
	setHandle(getHandle());
	localizeControlText(AP_RID_DIALOG_OPTIONS_FRM_Toolbars,			AP_STRING_ID_DLG_Options_Label_Toolbars);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_ViewShowStandardBar,	AP_STRING_ID_DLG_Options_Label_ViewStandardTB);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_ViewShowFormatBar,	AP_STRING_ID_DLG_Options_Label_ViewFormatTB);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_ViewShowExtraBar,		AP_STRING_ID_DLG_Options_Label_ViewExtraTB);
	localizeControlText(AP_RID_DIALOG_OPTIONS_FRM_ButtonStyle,		AP_STRING_ID_DLG_Options_Label_Look);
	localizeControlText(AP_RID_DIALOG_OPTIONS_RDO_Icons,			AP_STRING_ID_DLG_Options_Label_Icons);
	localizeControlText(AP_RID_DIALOG_OPTIONS_RDO_Text,			AP_STRING_ID_DLG_Options_Label_Text);
	localizeControlText(AP_RID_DIALOG_OPTIONS_RDO_IconsAndText,		AP_STRING_ID_DLG_Options_Label_Both);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_ViewToolTips,		AP_STRING_ID_DLG_Options_Label_ViewTooltips);
	
	// TODO:  make the following 4 controls usable
	EnableWindow( GetDlgItem(getHandle(), AP_RID_DIALOG_OPTIONS_RDO_Icons),		false );
	EnableWindow( GetDlgItem(getHandle(), AP_RID_DIALOG_OPTIONS_RDO_Text), 		false );
	EnableWindow( GetDlgItem(getHandle(), AP_RID_DIALOG_OPTIONS_RDO_IconsAndText), false );
	EnableWindow( GetDlgItem(getHandle(), AP_RID_DIALOG_OPTIONS_CHK_ViewToolTips), false );
	
	SetWindowLong(getHandle(), GWL_USERDATA, (LONG)this);
	return 0;	
			
}

/*

	Spelling page
	
*/
AP_Win32Dialog_Options_Spelling::AP_Win32Dialog_Options_Spelling()
{
	
}

AP_Win32Dialog_Options_Spelling::~AP_Win32Dialog_Options_Spelling()
{
	
}

/*
	
*/	
BOOL AP_Win32Dialog_Options_Spelling::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	AP_Win32Dialog_Options*	 pParent=  (AP_Win32Dialog_Options*)getContainer();	
	
	switch (wId)
	{		
		case AP_RID_DIALOG_OPTIONS_CHK_SpellCheckAsType:	pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_SPELL_CHECK_AS_TYPE);	return 0;
		case AP_RID_DIALOG_OPTIONS_CHK_SpellHideErrors: 	pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_SPELL_HIDE_ERRORS);	return 0;
		case AP_RID_DIALOG_OPTIONS_CHK_SpellSuggest:		pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_SPELL_SUGGEST);		return 0;
		case AP_RID_DIALOG_OPTIONS_CHK_SpellMainOnly:		pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_SPELL_MAIN_ONLY);		return 0;
		case AP_RID_DIALOG_OPTIONS_CHK_SpellUppercase:		pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_SPELL_UPPERCASE);		return 0;
		case AP_RID_DIALOG_OPTIONS_CHK_SpellNumbers:		pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_SPELL_NUMBERS);		return 0;
		case AP_RID_DIALOG_OPTIONS_CHK_SpellInternet:		pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_SPELL_INTERNET);		return 0;
		default:
		break;
	}
	return 1;
}

/*
	
*/	
BOOL AP_Win32Dialog_Options_Spelling::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{				
	const XAP_StringSet * pSS = getApp()->getStringSet();
	setStringSet(pSS);
	setHandle(getHandle());
	// localize controls
	localizeControlText(AP_RID_DIALOG_OPTIONS_FRM_SpellGeneral,	AP_STRING_ID_DLG_Options_Label_General);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_SpellCheckAsType,	AP_STRING_ID_DLG_Options_Label_SpellCheckAsType);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_SpellHideErrors,	AP_STRING_ID_DLG_Options_Label_SpellHideErrors);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_SpellSuggest,	AP_STRING_ID_DLG_Options_Label_SpellSuggest);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_SpellMainOnly,	AP_STRING_ID_DLG_Options_Label_SpellMainOnly);
	localizeControlText(AP_RID_DIALOG_OPTIONS_FRM_SpellIgnore,	AP_STRING_ID_DLG_Options_Label_Ignore);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_SpellUppercase,	AP_STRING_ID_DLG_Options_Label_SpellUppercase);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_SpellNumbers,	AP_STRING_ID_DLG_Options_Label_SpellNumbers);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_SpellInternet,	AP_STRING_ID_DLG_Options_Label_SpellInternet);
	localizeControlText(AP_RID_DIALOG_OPTIONS_LBL_CUSTOMDICT,	AP_STRING_ID_DLG_Options_Label_SpellCustomDict);
	localizeControlText(AP_RID_DIALOG_OPTIONS_BTN_CUSTOMDICT,	AP_STRING_ID_DLG_Options_Btn_CustomDict);
	localizeControlText(AP_RID_DIALOG_OPTIONS_LBL_IGNOREDWORD,	AP_STRING_ID_DLG_Options_Label_SpellIgnoredWord);
	localizeControlText(AP_RID_DIALOG_OPTIONS_BTN_IGNOREDRESET,	AP_STRING_ID_DLG_Options_Btn_IgnoreReset);
	localizeControlText(AP_RID_DIALOG_OPTIONS_BTN_IGNOREDEDIT,	AP_STRING_ID_DLG_Options_Btn_IgnoreEdit);

	// TODO need to populate values in the _COMBO_CUSTOMDICT
	HWND hwndDict = GetDlgItem(getHandle(), AP_RID_DIALOG_OPTIONS_COMBO_CUSTOMDICT);
	SendMessage(hwndDict, CB_ADDSTRING, 0, (LPARAM)L"custom.dic");	// TODO - get from prefs / var
	SendMessage(hwndDict, CB_SETCURSEL, (WPARAM) 0, 0);
	
	return 0;
		
}


/*

	Language page
	
*/
AP_Win32Dialog_Options_Lang::AP_Win32Dialog_Options_Lang()
{
	
}

AP_Win32Dialog_Options_Lang::~AP_Win32Dialog_Options_Lang()
{
	
}

/*
	
*/	
BOOL AP_Win32Dialog_Options_Lang::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;	
	AP_Win32Dialog_Options*	 pParent=  (AP_Win32Dialog_Options*)getContainer();	
	
	switch (wId)
	{
		case AP_RID_DIALOG_OPTIONS_CHK_LanguageWithKeyboard:
			pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_DIR_MARKER_AFTER_CLOSING_PARENTHESIS);
			return 0;
			
		default:
			break;
	}
	return 1;
}

/*
	
*/	
BOOL AP_Win32Dialog_Options_Lang::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{				
	const XAP_StringSet * pSS = getApp()->getStringSet();
	setStringSet(pSS);
	setHandle(getHandle());
	localizeControlText(AP_RID_DIALOG_OPTIONS_FRM_BidiOptions,			AP_STRING_ID_DLG_Options_Label_BiDiOptions);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_OtherDirectionRtl,		AP_STRING_ID_DLG_Options_Label_DirectionRtl);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_OtherSaveContextGlyphs,		AP_STRING_ID_DLG_Options_Label_SaveContextGlyphs);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_OtherSaveContextGlyphs,		AP_STRING_ID_DLG_Options_Label_HebrewContextGlyphs);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_LanguageWithKeyboard,		XAP_STRING_ID_DLG_Options_Label_LangWithKeyboard);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_DirMarkerAfterClosingParenthesis,	XAP_STRING_ID_DLG_Options_Label_DirMarkerAfterClosingParenthesis);
	localizeControlText(AP_RID_DIALOG_OPTIONS_TEXT_UILANG,				AP_STRING_ID_DLG_Options_Label_UILang);
	localizeControlText(AP_RID_DIALOG_OPTIONS_LANGSETTINGS,				AP_STRING_ID_DLG_Options_Label_LangSettings);
				
	/* Fill up document language*/			
	{			
		HWND	hCtrlUILang		= GetDlgItem(getHandle(), AP_RID_DIALOG_OPTIONS_COMBO_UILANG);
		
		AP_Win32App * pApp = static_cast<AP_Win32App*>(XAP_App::getApp()); 
		
		// TODO: Vector should be global and deleted on destroy (free the elements...)
		m_pVecUILangs = pApp->getInstalledUILanguages();			
		int nIndex;
		const XML_Char *pLangCode;
		const XML_Char *pLang;
		UT_Language	lang;
		

		if (m_pVecUILangs->getItemCount())		
		{
		
			/* Fill all up languages names for UI*/
			for (UT_uint32 i=0; i < m_pVecUILangs->getItemCount(); i++)
			{
				pLangCode = (const char *) m_pVecUILangs->getNthItem(i);
				
				int id = lang.getIndxFromCode(pLangCode);
				pLang  = lang.getNthLangName(id);
				UT_DEBUGMSG(("Inserting language %s", pLang));
				
				nIndex = SendMessage(hCtrlUILang, CB_ADDSTRING, 0, (LPARAM)XAP_Win32App::getWideString(pLang));
				UT_DEBUGMSG(("Inserted language %S", XAP_Win32App::getWideString(pLang)));
				SendMessage(hCtrlUILang, CB_SETITEMDATA, nIndex, id);
			}							
		}
		else
			EnableWindow(hCtrlUILang, FALSE);
	}
	return 0;
	
}


/*

	Layout page
	
*/

AP_Win32Dialog_Options_Layout::AP_Win32Dialog_Options_Layout()
{
	
}

AP_Win32Dialog_Options_Layout::~AP_Win32Dialog_Options_Layout()
{
	
}

/*
	
*/	
BOOL AP_Win32Dialog_Options_Layout::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;	
	AP_Win32Dialog_Options*	 pParent=  (AP_Win32Dialog_Options*)getContainer();	
	bool bChecked;
	AP_Dialog_Background *pColorDialog;
	UT_RGBColor rgbColor;
	
	switch (wId)
	{
		
		case AP_RID_DIALOG_OPTIONS_CHK_ViewShowRuler:		pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_VIEW_SHOW_RULER);		return 0;
		case AP_RID_DIALOG_OPTIONS_CHK_ViewCursorBlink: 	pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_VIEW_CURSOR_BLINK);	return 0;
		case AP_RID_DIALOG_OPTIONS_CHK_ViewShowStatusBar:	pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_VIEW_SHOW_STATUS_BAR); return 0;
		case AP_RID_DIALOG_OPTIONS_CHK_ViewAll: 			pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_VIEW_ALL); 			return 0;
		case AP_RID_DIALOG_OPTIONS_CHK_ViewHiddenText:		pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_VIEW_HIDDEN_TEXT); 	return 0;
		case AP_RID_DIALOG_OPTIONS_CHK_ViewUnprintable: 	pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_VIEW_UNPRINTABLE); 	return 0;
		case AP_RID_DIALOG_OPTIONS_COMBO_UNITS: 																return 0;
		case AP_RID_DIALOG_OPTIONS_COMBO_DefaultPageSize:														return 0;
		case AP_RID_DIALOG_OPTIONS_CHK_SmartQuotesEnable:	pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_SMART_QUOTES_ENABLE);	return 0;
		case AP_RID_DIALOG_OPTIONS_CHK_BGColorEnable:
			bChecked = (IsDlgButtonChecked( hWnd, AP_RID_DIALOG_OPTIONS_CHK_BGColorEnable ) == BST_CHECKED);
			EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_BTN_BGColor), bChecked );
			return 0;
		
		case AP_RID_DIALOG_OPTIONS_BTN_BGColor:
		{
			pColorDialog = (AP_Dialog_Background *)(pParent->getDialogFactory()->requestDialog(AP_DIALOG_ID_BACKGROUND));
			UT_ASSERT(pColorDialog);
	
			UT_parseColor(pParent->_gatherColorForTransparent(), rgbColor );
			pColorDialog->setColor(rgbColor);
			pColorDialog->runModal(pParent->getFrame());
	
			if( pColorDialog->getAnswer() == AP_Dialog_Background::a_OK )
				pParent->_setColorForTransparent(pColorDialog->getColor());
				
			pParent->getDialogFactory()->releaseDialog(pColorDialog);
			return 0;
		}

		default:
			break;
	}
	return 1;
}

/*
	
*/	
BOOL AP_Win32Dialog_Options_Layout::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{				
	const XAP_StringSet * pSS = getApp()->getStringSet();
	setStringSet(pSS);
	setHandle(getHandle());
	// localize controls
	localizeControlText(AP_RID_DIALOG_OPTIONS_FRM_SHOWHIDE,			AP_STRING_ID_DLG_Options_Label_ViewShowHide);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_ViewShowRuler,		AP_STRING_ID_DLG_Options_Label_ViewRuler);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_ViewCursorBlink,		AP_STRING_ID_DLG_Options_Label_ViewCursorBlink);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_ViewShowStatusBar,	AP_STRING_ID_DLG_Options_Label_ViewStatusBar);
	localizeControlText(AP_RID_DIALOG_OPTIONS_FRM_VIEWFRAME,		AP_STRING_ID_DLG_Options_Label_ViewViewFrame);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_ViewAll,			AP_STRING_ID_DLG_Options_Label_ViewAll);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_ViewHiddenText,		AP_STRING_ID_DLG_Options_Label_ViewHiddenText);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_ViewUnprintable,		AP_STRING_ID_DLG_Options_Label_ViewUnprintable);
	localizeControlText(AP_RID_DIALOG_OPTIONS_LBL_UNITS,			AP_STRING_ID_DLG_Options_Label_ViewUnits);
	localizeControlText(AP_RID_DIALOG_OPTIONS_LBL_DefaultPageSize,		AP_STRING_ID_DLG_Options_Label_DefaultPageSize);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_SmartQuotesEnable,	AP_STRING_ID_DLG_Options_Label_SmartQuotesEnable);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_BGColorEnable,		AP_STRING_ID_DLG_Options_Label_CheckWhiteForTransparent);
	localizeControlText(AP_RID_DIALOG_OPTIONS_BTN_BGColor,			AP_STRING_ID_DLG_Options_Label_ChooseForTransparent);
	
	// Populate values in the _COMBO_UNITS
	HWND hwndAlign = GetDlgItem(getHandle(), AP_RID_DIALOG_OPTIONS_COMBO_UNITS);
	for( int n1 = 0; n1 < SIZE_aAlignUnit; n1++ )
	{
		SendMessage(hwndAlign, CB_ADDSTRING, 0, (LPARAM)XAP_Win32App::getWideString(pSS->getValue(s_aAlignUnit[n1].id)));
	}
	
	HWND hwndPaperSize = GetDlgItem(getHandle(), AP_RID_DIALOG_OPTIONS_COMBO_DefaultPageSize);
	for( int n2 = (int)fp_PageSize::_first_predefined_pagesize_; n2 < (int)fp_PageSize::_last_predefined_pagesize_dont_use_; n2++ )
	{
	SendMessage( hwndPaperSize,
				 CB_INSERTSTRING,
				 (WPARAM) n2,
				 (LPARAM) XAP_Win32App::getWideString(fp_PageSize::PredefinedToName( (fp_PageSize::Predefined)n2) ));
	}	
	return 0;
}

/*

	Pref page
	
*/
AP_Win32Dialog_Options_Pref::AP_Win32Dialog_Options_Pref()
{
	
}

AP_Win32Dialog_Options_Pref::~AP_Win32Dialog_Options_Pref()
{
	
}

/*
	
*/	
BOOL AP_Win32Dialog_Options_Pref::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);	
	AP_Win32Dialog_Options*	 pParent=  (AP_Win32Dialog_Options*)getContainer();	
	BOOL bChecked;	
	
	switch (wId)
	{		

	case AP_RID_DIALOG_OPTIONS_CHK_PrefsAutoSave:		
			pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_PREFS_AUTO_SAVE);		
			return 0;
			
	case AP_RID_DIALOG_OPTIONS_COMBO_CURRENTSCHEME:
		return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_OtherDirectionRtl:	
		pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_OTHER_DEFAULT_DIRECTION_RTL);	
		return 0;

	case AP_RID_DIALOG_OPTIONS_CHK_OtherSaveContextGlyphs:	pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_OTHER_SAVE_CONTEXT_GLYPHS);return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_OtherHebrewContextGlyphs:  pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_OTHER_HEBREW_CONTEXT_GLYPHS);return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile:
		pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_AUTO_SAVE_FILE);

		bChecked = (IsDlgButtonChecked( hWnd, AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile ) == BST_CHECKED);
		EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod), bChecked );
		EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_SPN_AutoSavePeriodSpin), bChecked );
		EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_TXT_AutoSaveExtension), bChecked );
		return 0;

	case AP_RID_DIALOG_OPTIONS_CHK_ShowSplash:			
		pParent->_enableDisableLogic(AP_Dialog_Options::id_SHOWSPLASH); 	
		return 0;	
	
	case AP_RID_DIALOG_OPTIONS_CHK_LanguageWithKeyboard:  pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_LANG_WITH_KEYBOARD);return 0;

	case AP_RID_DIALOG_OPTIONS_CHK_DirMarkerAfterClosingParenthesis:  pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_DIR_MARKER_AFTER_CLOSING_PARENTHESIS);return 0;
		
		default:
		break;
	}
	return 1;

}

/*
	
*/	
BOOL AP_Win32Dialog_Options_Pref::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{				
	const XAP_StringSet * pSS = getApp()->getStringSet();
	setStringSet(pSS);
	setHandle(getHandle());
	// localize controls
	localizeControlText(AP_RID_DIALOG_OPTIONS_FRM_PreferenceScheme,		AP_STRING_ID_DLG_Options_Label_Schemes);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_PrefsAutoSave,		AP_STRING_ID_DLG_Options_Label_PrefsAutoSave);
	localizeControlText(AP_RID_DIALOG_OPTIONS_LBL_CURRENTSCHEME,		AP_STRING_ID_DLG_Options_Label_PrefsCurrentScheme);
	localizeControlText(AP_RID_DIALOG_OPTIONS_FRM_AutoSaveOptions,		AP_STRING_ID_DLG_Options_Label_AutoSave);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile,		AP_STRING_ID_DLG_Options_Label_AutoSaveCurrent);
	localizeControlText(AP_RID_DIALOG_OPTIONS_LBL_AutoSaveMinutes,		AP_STRING_ID_DLG_Options_Label_Minutes);
	localizeControlText(AP_RID_DIALOG_OPTIONS_LBL_AutoSaveExtension,	AP_STRING_ID_DLG_Options_Label_WithExtension);
	localizeControlText(AP_RID_DIALOG_OPTIONS_CHK_ShowSplash,		AP_STRING_ID_DLG_Options_Label_ShowSplash);
	
	// Set the starting period to 1 minute
	SetDlgItemInt(getHandle(), AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod, 1, FALSE );
	
	// Set the range for the period to 1-360
	SendMessage(GetDlgItem(getHandle(),AP_RID_DIALOG_OPTIONS_SPN_AutoSavePeriodSpin),UDM_SETBUDDY, (WPARAM) GetDlgItem(getHandle(),AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod),0);	
	
	SendMessage(GetDlgItem(getHandle(),AP_RID_DIALOG_OPTIONS_SPN_AutoSavePeriodSpin),UDM_SETRANGE,0,(WPARAM)MAKELONG(MAXAUTOSAVEPERIOD,MINAUTOSAVEPERIOD));
	SendMessage(GetDlgItem(getHandle(),AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod),EM_LIMITTEXT,(WPARAM)3,(WPARAM)0);
	
	// Limit the extension to 5 characters (plus the period)
	SendMessage(GetDlgItem(getHandle(),AP_RID_DIALOG_OPTIONS_TXT_AutoSaveExtension),EM_LIMITTEXT,(WPARAM)6,(WPARAM)0);
	SetWindowLong(getHandle(), GWL_USERDATA, (LONG)this);	
	
	// TODO need to populate values in the _COMBO_CURRENTSCHEME
	//			HWND hwndScheme = GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_COMBO_CURRENTSCHEME);
	//			_CDB(OPTIONS_CHK_PrefsAutoSave, 		id_CHECK_PREFS_AUTO_SAVE);

	

	return 0;
}

bool AP_Win32Dialog_Options_Pref::isAutoSaveInRange()
{
	int iValue = GetDlgItemInt(getHandle(), AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod, NULL, FALSE);
	AP_Win32Dialog_Options*	 pParent=  (AP_Win32Dialog_Options*)getContainer();		
	char szTemp[10];
	snprintf( szTemp, 10, "%d", iValue);	
	
	if (iValue<MINAUTOSAVEPERIOD || iValue>MAXAUTOSAVEPERIOD)
	{				
			const XAP_StringSet * pSS = getApp()->getStringSet();	
			::MessageBox(NULL, XAP_Win32App::getWideString(pSS->getValue(AP_STRING_ID_DLG_Options_Label_InvalidRangeForAutoSave)), 
				L"",MB_OK);
		
		return false;
	}
	
	return true;	
}
