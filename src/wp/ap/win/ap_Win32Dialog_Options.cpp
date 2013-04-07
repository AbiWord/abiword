/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *			 (c) 2002-2006 Jordi Mas i Hernàndez jmas@softcatala.org
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
#include "ut_Win32LocaleString.h"

#include "gr_Win32Graphics.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "xap_Prefs.h"
#include "xap_EncodingManager.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"
#include "xap_Toolbar_Layouts.h"
#include "ap_Strings.h"
#include "ap_Win32App.h"
#include "ap_Win32Resources.rc2"
#include "ap_Win32Dialog_Options.h"
#include "ap_Win32Dialog_Background.h"
#include "xap_Win32DialogHelper.h"
#include "ut_Language.h"



/*****************************************************************/

#define GWL(hwnd)		(AP_Win32Dialog_Options*)GetWindowLongPtrW((hwnd), DWLP_USER)
#define SWL(hwnd, d)	(AP_Win32Dialog_Options*)SetWindowLongPtrW((hwnd), DWLP_USER,(LONG_PTR)(d))
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
	m_langchanged = FALSE;	
	LOGFONTW	logFont;

	// Create bold font
	HFONT hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
	GetObjectW (hFont, sizeof(LOGFONTW), &logFont); 	
													
	logFont.lfWeight  = FW_BOLD;
	m_hFont = CreateFontIndirectW(&logFont);
}

AP_Win32Dialog_Options::~AP_Win32Dialog_Options(void)
{
	 DeleteObject (m_hFont);	
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void AP_Win32Dialog_Options::checkLanguageChange()
{
	UT_String sLang;

	_gatherUILanguage(sLang);	

	if (m_curLang!=sLang)
		m_langchanged = TRUE;						 	
		
}

void AP_Win32Dialog_Options::_initEnableControlsPlatformSpecific()
{
	_controlEnable( id_CHECK_LANG_WITH_KEYBOARD, true); 
	_controlEnable( id_CHECK_DIR_MARKER_AFTER_CLOSING_PARENTHESIS,_gatherLanguageWithKeyboard());  
}


void AP_Win32Dialog_Options::runModal(XAP_Frame * pFrame)
{	
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);	
	AP_Win32Dialog_Options_Sheet	sheet;	
	
	UT_return_if_fail (pFrame);	
	m_pFrame = pFrame;
	
	/* Create the property sheet and associate its pages*/		

	m_general.setContainer(this);	
	m_general.createPage(pWin32App, AP_RID_DIALOG_OPT_GENERAL, AP_STRING_ID_DLG_Options_Label_General);	
	sheet.addPage(&m_general);				

	m_document.setContainer(this);	
	m_document.createPage(pWin32App, AP_RID_DIALOG_OPT_DOCUMENTS, AP_STRING_ID_DLG_Options_Label_Documents);	
	sheet.addPage(&m_document);				
	
	m_spelling.setContainer(this);	
	m_spelling.createPage(pWin32App, AP_RID_DIALOG_OPT_SPELL, AP_STRING_ID_DLG_Options_TabLabel_Spelling);	
	sheet.addPage(&m_spelling);	

	m_smartquotes.setContainer(this);
	m_smartquotes.createPage(pWin32App, AP_RID_DIALOG_OPT_SMARTQUOTES, AP_STRING_ID_DLG_Options_TabLabel_SmartQuotes);
	sheet.addPage(&m_smartquotes);

	sheet.setApplyButton(true);       
	sheet.setParent(this);	       
	if (sheet.runModal(pWin32App, pFrame, AP_STRING_ID_DLG_Options_OptionsTitle)==IDOK)	
		m_answer = a_OK;
	else		
		m_answer = a_CANCEL;

	if (m_langchanged) 
	{
		const XAP_StringSet * pSS = getApp()->getStringSet();
		UT_Win32LocaleString str;
			
		str.fromUTF8 (pSS->getValue(AP_STRING_ID_DLG_Options_Prompt_YouMustRestart));
		::MessageBoxW(NULL, str.c_str(), L"Abiword",MB_OK);
	}
	
}	

static struct {
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
#define _CDB(c,i)	checkDlgButton(AP_RID_DIALOG_##c,_getCheckItemValue(i))
#define _DS2(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX2(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

/*
	Gets the property page by its index
*/
HWND	AP_Win32Dialog_Options::getPage(PSH_PAGES page)
{
	HWND hWnd = NULL;
	
	switch (page)
	{			
		case PG_SPELL:
			hWnd = m_spelling.getHandle();
			break;				
		
		case PG_GENERAL:
			hWnd = m_general.getHandle();
			break;				
		
		case PG_DOCUMENT:
			hWnd = m_document.getHandle();
			break;				

		case PG_SMARTQUOTES:
			hWnd = m_smartquotes.getHandle();
			break;

		default:
			break;
	}
	
	UT_ASSERT_HARMLESS(hWnd!=NULL);	
	return hWnd;
}


/*

*/
void AP_Win32Dialog_Options::_controlEnable( tControl id, bool value )
{
	// This routine is called by XP code to enable/disable a particular field.

	switch (id)
	{
		case id_CHECK_ENABLE_OVERWRITE:
			EnableWindow(GetDlgItem((HWND)getPage(PG_GENERAL),AP_RID_DIALOG_OPTIONS_CHK_EnableOverwrite),value);
			return;

		case id_CHECK_LANG_WITH_KEYBOARD:
			EnableWindow(GetDlgItem((HWND)getPage(PG_GENERAL),AP_RID_DIALOG_OPTIONS_CHK_LanguageWithKeyboard),value);
			return;

		case id_CHECK_AUTO_LOAD_PLUGINS:
			EnableWindow(GetDlgItem((HWND)getPage(PG_GENERAL),AP_RID_DIALOG_OPTIONS_CHK_AutoLoadPlugins),value);
			return;

		case id_CHECK_AUTO_SAVE_FILE:
			EnableWindow(GetDlgItem((HWND)getPage(PG_DOCUMENT),AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile),value);
			return;			

		case id_CHECK_OTHER_DEFAULT_DIRECTION_RTL:
			EnableWindow(GetDlgItem((HWND)getPage(PG_DOCUMENT),AP_RID_DIALOG_OPTIONS_CHK_OtherDirectionRtl),value);
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

		case id_CHECK_GRAMMAR_CHECK:
			EnableWindow(GetDlgItem((HWND)getPage(PG_SPELL),AP_RID_DIALOG_OPTIONS_CHK_GrammarCheck),value);
			return;

		case id_CHECK_SMART_QUOTES_ENABLE:
			EnableWindow(GetDlgItem((HWND)getPage(PG_SMARTQUOTES),AP_RID_DIALOG_OPTIONS_CHK_CustomSmartQuotes),value);
			return;

		case id_CHECK_CUSTOM_SMART_QUOTES:
			EnableWindow(GetDlgItem((HWND)getPage(PG_SMARTQUOTES),AP_RID_DIALOG_OPTIONS_COMBO_OUTERQUOTE),value);
			EnableWindow(GetDlgItem((HWND)getPage(PG_SMARTQUOTES),AP_RID_DIALOG_OPTIONS_COMBO_INNERQUOTE),value);
			return;
				
		default:
			//UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return;
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// The following defines a specific, named "Get" and "Set" method
// for each CHECK box on all of the sub-dialogs.  These are called
// by XP code to set a particular value in the dialog presentation;
// they do not call back into XP code.


#define DEFINE_GET_SET_BOOL(index,button) \
bool AP_Win32Dialog_Options::_gather##button(void) { \
	return (IsDlgButtonChecked((HWND)getPage(index),AP_RID_DIALOG_OPTIONS_CHK_##button) == BST_CHECKED); \
} \
void AP_Win32Dialog_Options::_set##button(const bool b) { \
	CheckDlgButton((HWND)getPage(index),AP_RID_DIALOG_OPTIONS_CHK_##button,b); \
}

// dummy implementations
#define DEFINE_GET_SET_BOOL_DUMMY(Bool)						\
bool	AP_Win32Dialog_Options::_gather##Bool(void) { 		\
		return m_bool##Bool;								\
} 															\
void	AP_Win32Dialog_Options::_set##Bool(bool b) { 		\
	m_bool##Bool = b;										\
}

DEFINE_GET_SET_BOOL(PG_GENERAL,EnableOverwrite)
DEFINE_GET_SET_BOOL(PG_GENERAL,LanguageWithKeyboard)
DEFINE_GET_SET_BOOL(PG_GENERAL,AutoLoadPlugins)

DEFINE_GET_SET_BOOL(PG_DOCUMENT,OtherDirectionRtl)

DEFINE_GET_SET_BOOL(PG_SPELL,SpellCheckAsType)
DEFINE_GET_SET_BOOL(PG_SPELL,SpellHideErrors)
DEFINE_GET_SET_BOOL(PG_SPELL,SpellSuggest)
DEFINE_GET_SET_BOOL(PG_SPELL,SpellMainOnly)
DEFINE_GET_SET_BOOL(PG_SPELL,SpellUppercase)
DEFINE_GET_SET_BOOL(PG_SPELL,SpellNumbers)
DEFINE_GET_SET_BOOL(PG_SPELL,GrammarCheck)

DEFINE_GET_SET_BOOL(PG_SMARTQUOTES,SmartQuotes)
DEFINE_GET_SET_BOOL(PG_SMARTQUOTES,CustomSmartQuotes)

/* Not used */
DEFINE_GET_SET_BOOL_DUMMY (ViewCursorBlink)
DEFINE_GET_SET_BOOL_DUMMY (EnableSmoothScrolling)
DEFINE_GET_SET_BOOL_DUMMY (PrefsAutoSave)
DEFINE_GET_SET_BOOL_DUMMY (ViewAll)
DEFINE_GET_SET_BOOL_DUMMY (ViewHiddenText)
DEFINE_GET_SET_BOOL_DUMMY (ViewShowRuler)
DEFINE_GET_SET_BOOL_DUMMY (ViewShowStatusBar)
DEFINE_GET_SET_BOOL_DUMMY (ViewUnprintable)

#undef DEFINE_GET_SET_BOOL


bool AP_Win32Dialog_Options::_gatherAutoSaveFile(void)
{
	return (IsDlgButtonChecked((HWND)getPage(PG_DOCUMENT),AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile) == BST_CHECKED);
}

void AP_Win32Dialog_Options::_setAutoSaveFile(const bool b)
{
	BOOL bChecked;
	HWND hWnd = (HWND)getPage(PG_DOCUMENT);

	CheckDlgButton( hWnd, AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile, b );

	// Disable the input boxes if auto save is turned off
	bChecked = (IsDlgButtonChecked( hWnd, AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile ) == BST_CHECKED);
	EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod), bChecked );
	EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_SPN_AutoSavePeriodSpin), bChecked );
	EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_TXT_AutoSaveExtension), bChecked );
}

void AP_Win32Dialog_Options::_gatherAutoSaveFileExt(UT_String &stRetVal)
{
	UT_Win32LocaleString str;
	XAP_Win32DialogBase::getDlgItemText(getPage(PG_DOCUMENT), AP_RID_DIALOG_OPTIONS_TXT_AutoSaveExtension, str);
	stRetVal = str.utf8_str().utf8_str();
}

void AP_Win32Dialog_Options::_setAutoSaveFileExt(const UT_String &stExt)
{
	XAP_Win32DialogBase::setDlgItemText(getPage(PG_DOCUMENT), AP_RID_DIALOG_OPTIONS_TXT_AutoSaveExtension, stExt.c_str());
}

void AP_Win32Dialog_Options::_gatherAutoSaveFilePeriod(UT_String &stRetVal)
{
	int iValue = GetDlgItemInt((HWND)getPage(PG_DOCUMENT), AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod, NULL, FALSE );
	char szTemp[10];

	snprintf( szTemp, 10, "%d", iValue );

	stRetVal = szTemp;
}

void AP_Win32Dialog_Options::_setAutoSaveFilePeriod(const UT_String &stPeriod)
{
	SetDlgItemInt((HWND)getPage(PG_DOCUMENT), AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod, atoi(stPeriod.c_str()), FALSE );
}

UT_Dimension AP_Win32Dialog_Options::_gatherViewRulerUnits(void)
{
	HWND hwndAlign = GetDlgItem((HWND)getPage(PG_GENERAL), AP_RID_DIALOG_OPTIONS_COMBO_UNITS);
	int nSel = SendMessageW(hwndAlign, CB_GETCURSEL, 0, 0);

	if( nSel != CB_ERR )
		return s_aAlignUnit[nSel].dim;
	return DIM_IN;
}

void  AP_Win32Dialog_Options::_setViewRulerUnits(UT_Dimension dim)
{
	unsigned int n1;
	for( n1 = 0; n1 < SIZE_aAlignUnit; n1++ )
		if( s_aAlignUnit[n1].dim == dim )
			break;
	if( n1 == SIZE_aAlignUnit )
		n1 = 0;

	HWND hwndAlign = GetDlgItem((HWND)getPage(PG_GENERAL), AP_RID_DIALOG_OPTIONS_COMBO_UNITS);
	SendMessageW(hwndAlign, CB_SETCURSEL, (WPARAM)n1, 0);
}

int AP_Win32Dialog_Options::_gatherNotebookPageNum(void)
{
	return 0;
}

void AP_Win32Dialog_Options::_setNotebookPageNum(const int /*pn*/)
{

}

void AP_Win32Dialog_Options::_gatherUILanguage(UT_String &stRetVal)
{
	HWND hCtrlDocLang = GetDlgItem((HWND)getPage(PG_GENERAL), AP_RID_DIALOG_OPTIONS_COMBO_UILANG);
	UT_Language	lang;
	const char* pLang;
	int nIndex = SendMessageW(hCtrlDocLang,  CB_GETCURSEL , 0,0);
	
	if (nIndex!=CB_ERR)
	{
		int nID = SendMessageW(hCtrlDocLang,  CB_GETITEMDATA , nIndex,0);
		pLang =  (const char*)lang.getNthLangCode(nID);		
		stRetVal = pLang;
	}				
	else
		stRetVal = m_curLang;
}

void AP_Win32Dialog_Options::_setUILanguage(const UT_String &stExt)
{
	UT_Language	lang;
	int id = lang.getIndxFromCode(stExt.c_str());
	bool bCustom = true;
	HWND hCtrlDocLang = GetDlgItem((HWND)getPage(PG_GENERAL), AP_RID_DIALOG_OPTIONS_COMBO_UILANG);

	int nCount = SendMessageW(hCtrlDocLang, CB_GETCOUNT, 0, 0);		
	
	for (int i=0; i<nCount;i++)
	{
		if (SendMessageW(hCtrlDocLang,  CB_GETITEMDATA , i,0)==id)
		{
			SendMessageW(hCtrlDocLang, CB_SETCURSEL, i, 0);
			bCustom = false;
			break;
		}
	}

	if (!bCustom && (nCount < 2))
		EnableWindow(hCtrlDocLang,FALSE);

	if (!m_curLang.size())
		m_curLang = stExt;
}

void AP_Win32Dialog_Options::_setOuterQuoteStyle(const gint index)
{
	HWND hCombo = GetDlgItem((HWND)getPage(PG_SMARTQUOTES), AP_RID_DIALOG_OPTIONS_COMBO_OUTERQUOTE);
	UT_return_if_fail(hCombo);

	int nCount = SendMessageW(hCombo, CB_GETCOUNT, 0, 0);
	UT_return_if_fail(index >= 0 && index < nCount);

	SendMessageW(hCombo, CB_SETCURSEL, index, 0);
}

void AP_Win32Dialog_Options::_setInnerQuoteStyle(const gint index)
{
	HWND hCombo = GetDlgItem((HWND)getPage(PG_SMARTQUOTES), AP_RID_DIALOG_OPTIONS_COMBO_INNERQUOTE);
	UT_return_if_fail(hCombo);

	int nCount = SendMessageW(hCombo, CB_GETCOUNT, 0, 0);
	UT_return_if_fail(index >= 0 && index < nCount);

	SendMessageW(hCombo, CB_SETCURSEL, index, 0);
}

gint AP_Win32Dialog_Options::_gatherOuterQuoteStyle()
{
	HWND hCombo = GetDlgItem((HWND)getPage(PG_SMARTQUOTES), AP_RID_DIALOG_OPTIONS_COMBO_OUTERQUOTE);
	UT_return_val_if_fail(hCombo,0);
	int nIndex = SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
	UT_return_val_if_fail(nIndex != CB_ERR, 0);

	return nIndex;
}

gint AP_Win32Dialog_Options::_gatherInnerQuoteStyle()
{
	HWND hCombo = GetDlgItem((HWND)getPage(PG_SMARTQUOTES), AP_RID_DIALOG_OPTIONS_COMBO_INNERQUOTE);
	UT_return_val_if_fail(hCombo,0);
	int nIndex = SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
	UT_return_val_if_fail(nIndex != CB_ERR, 0);

	return nIndex;	
}


/*

	Sheet
	
*/
AP_Win32Dialog_Options_Sheet::AP_Win32Dialog_Options_Sheet() :
XAP_Win32PropertySheet()
{
	m_pParent = NULL;
	setCallBack((PFNPROPSHEETCALLBACK)s_sheetInit);
}



/*
	Sheet window procedure
*/
BOOL AP_Win32Dialog_Options_Sheet::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wID = LOWORD(wParam); 
	
	if (wID==IDOK)
	{			
		AP_Win32Dialog_Options_Sheet * t = (AP_Win32Dialog_Options_Sheet *) GetWindowLongPtrW(hWnd, GWLP_USERDATA);					
		HWND hWndPref = t->getParent()->getPage(PG_DOCUMENT);
		AP_Win32Dialog_Options_Document * prefPag = (AP_Win32Dialog_Options_Document *) GetWindowLongPtrW(hWndPref, GWLP_USERDATA);							
				
		if (!prefPag->isAutoSaveInRange()) return 0;
		
		t->getParent()->_storeWindowData(); 					// remember current settings
		t->getParent()->checkLanguageChange();
		
	}
	
	if (wID==ID_APPLY_NOW)	// Save default button
	{
		AP_Win32Dialog_Options_Sheet * t = (AP_Win32Dialog_Options_Sheet *) GetWindowLongPtrW(hWnd, GWLP_USERDATA);					
		t->getParent()->_event_SetDefaults();				
		return 0;
	}
	
	return 1;	// The application did not process the message
}

INT_PTR CALLBACK AP_Win32Dialog_Options_Sheet::s_sheetInit(HWND hwnd,  UINT uMsg,  LPARAM /*lParam*/)
{	
	if (uMsg==PSCB_INITIALIZED)
	{	
		/* Force the creation of all pages*/
		PropSheet_SetCurSel(hwnd, 0,0);
		PropSheet_SetCurSel(hwnd, 0,1);
		PropSheet_SetCurSel(hwnd, 0,2);		
		PropSheet_SetCurSel(hwnd, 0,3);
	}			
	return 	0;
}


/*
	
*/
void AP_Win32Dialog_Options_Sheet::_onInitDialog(HWND hwnd)
{		

	// let XP code tell us what all of the values should be.
	getParent()->_populateWindowData();	
	
	// Apply button -> save 
	const XAP_StringSet * pSS = getParent()->getApp()->getStringSet();	
	EnableWindow(GetDlgItem(hwnd, ID_APPLY_NOW), TRUE);
	XAP_Win32DialogBase::setWindowText(GetDlgItem(hwnd, ID_APPLY_NOW), pSS->getValue(AP_STRING_ID_DLG_Options_Btn_Default));
	XAP_Win32DialogBase::setWindowText(GetDlgItem(hwnd, IDOK), pSS->getValue(XAP_STRING_ID_DLG_OK));
	XAP_Win32DialogBase::setWindowText(GetDlgItem(hwnd, IDCANCEL), pSS->getValue(XAP_STRING_ID_DLG_Cancel));	

	AP_Win32Dialog_Options*	 pParent=  (AP_Win32Dialog_Options*)getParent();		
	int nDefPage = pParent->getInitialPageNum();		
	PropSheet_SetCurSel(hwnd, 0, nDefPage);	
	
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

//void AP_Win32Dialog_Options_Spelling::_onCommand(HWND /*hWnd*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
//{

//}

/*
	
*/	
void AP_Win32Dialog_Options_Spelling::_onInitDialog()
{				
	const XAP_StringSet * pSS = getApp()->getStringSet();	
	int i;
	
	// localize controls
	_DS2(OPTIONS_FRM_SpellGeneral,			DLG_Options_Label_General);
	_DS2(OPTIONS_CHK_SpellCheckAsType,		DLG_Options_Label_SpellCheckAsType);
	_DS2(OPTIONS_CHK_SpellHideErrors,		DLG_Options_Label_SpellHideErrors);
	_DS2(OPTIONS_CHK_SpellSuggest,			DLG_Options_Label_SpellSuggest);
	_DS2(OPTIONS_CHK_SpellMainOnly,			DLG_Options_Label_SpellMainOnly);
	_DS2(OPTIONS_FRM_SpellIgnore,			DLG_Options_Label_Ignore);
	_DS2(OPTIONS_CHK_SpellUppercase, 		DLG_Options_Label_SpellUppercase);
	_DS2(OPTIONS_CHK_SpellNumbers,			DLG_Options_Label_SpellNumbers);
	_DS2(OPTIONS_LBL_CUSTOMDICT, 			DLG_Options_Label_SpellCustomDict);
	_DS2(OPTIONS_BTN_CUSTOMDICT, 			DLG_Options_Btn_CustomDict);
	_DS2(OPTIONS_LBL_IGNOREDWORD,			DLG_Options_Label_SpellIgnoredWord);
	_DS2(OPTIONS_BTN_IGNOREDRESET,			DLG_Options_Btn_IgnoreReset);
	_DS2(OPTIONS_BTN_IGNOREDEDIT,			DLG_Options_Btn_IgnoreEdit);
	_DS2(OPTIONS_CHK_GrammarCheck,			DLG_Options_Label_GrammarCheck);

	_DS2(OPTIONS_STATIC_General,			DLG_Options_Label_General);
	_DS2(OPTIONS_STATIC_SpellIgnoreWords,	DLG_Options_Label_SpellIgnoreWords);
	_DS2(OPTIONS_STATIC_SpellDictionaries,	DLG_Options_Label_SpellDictionaries);
	_DS2(OPTIONS_STATIC_Grammar,			DLG_Options_Label_Grammar);
	_DS2(OPTIONS_CHK_HighLight,				DLG_Options_Label_SpellHighlightMisspelledWords);	

	// Setup bold font for some controls	
	UINT boldFields[]={AP_RID_DIALOG_OPTIONS_STATIC_General, AP_RID_DIALOG_OPTIONS_STATIC_SpellIgnoreWords, 
			AP_RID_DIALOG_OPTIONS_STATIC_SpellDictionaries, AP_RID_DIALOG_OPTIONS_STATIC_Grammar, 0};

	for(i=0; boldFields[i]; i++) 
	{
		SendMessageW(GetDlgItem(getHandle(), boldFields[i]), WM_SETFONT, 
			(WPARAM)(AP_Win32Dialog_Options*)getContainer()->getBoldFontHandle(), MAKELPARAM(FALSE /* Redraw */, 0));
	}
		
}



/*

	General page
	
*/

AP_Win32Dialog_Options_General::AP_Win32Dialog_Options_General()
{
	setDialogProc(s_pageWndProc);
	m_nCentered = 0;
	m_pVecUILangs = NULL;
}

AP_Win32Dialog_Options_General::~AP_Win32Dialog_Options_General()
{
	if (m_pVecUILangs)
	{		
		for (UT_sint32 i=0; i < m_pVecUILangs->getItemCount(); i++)
			g_free ((void *)m_pVecUILangs->getNthItem(i));
			
		delete m_pVecUILangs;		
	}	
}

/*
	
*/	
BOOL AP_Win32Dialog_Options_General::_onCommand(HWND /*hWnd*/, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);
	AP_Win32Dialog_Options*	 pParent=  (AP_Win32Dialog_Options*)getContainer();	
	AP_Dialog_Background *pColorDialog;
	UT_RGBColor rgbColor;
	
	switch (wId)
	{					
		case AP_RID_DIALOG_OPTIONS_BTN_BGColor:
		{
			pColorDialog = (AP_Dialog_Background *)(pParent->getDialogFactory()->requestDialog(AP_DIALOG_ID_BACKGROUND));
            if (pColorDialog == NULL)
		    	return FALSE;
	
			UT_parseColor(pParent->_gatherColorForTransparent(), rgbColor );
			pColorDialog->setColor(rgbColor);
			pColorDialog->runModal(pParent->getFrame());
	
			if( pColorDialog->getAnswer() == AP_Dialog_Background::a_OK )
				pParent->_setColorForTransparent(pColorDialog->getColor());
				
			pParent->getDialogFactory()->releaseDialog(pColorDialog);
            return TRUE;
		}

		default:
			break;
	}
    return FALSE;
}

/*
	
*/	
void AP_Win32Dialog_Options_General::_onInitDialog()
{	
	const gchar *pLangCode;
	const gchar *pLang;
	UT_Language	lang;	
	int nIndex;
	const XAP_StringSet * pSS = getApp()->getStringSet();	
	AP_Win32App * pApp = static_cast<AP_Win32App*>(XAP_App::getApp());
	
	// localize controls	
	_DS2(OPTIONS_CHK_EnableOverwrite,		DLG_Options_Label_EnableOverwrite);
	_DS2(OPTIONS_LBL_UNITS,					DLG_Options_Label_ViewUnits);	
	_DS2(OPTIONS_BTN_BGColor,				DLG_Options_Label_ChooseForTransparent);
	_DS2(OPTIONS_CHK_AutoLoadPlugins,		DLG_Options_Label_CheckAutoLoadPlugins);
	_DS2(OPTIONS_STATIC_UI,					DLG_Options_Label_UI);
	_DS2(OPTIONS_STATIC_APPSTARTUP,			DLG_Options_Label_AppStartup);
	_DSX2(OPTIONS_CHK_LanguageWithKeyboard,	DLG_Options_Label_LangWithKeyboard);	
	_DS2(OPTIONS_TEXT_UILANG,				DLG_Options_Label_UILang);
	_DS2(OPTIONS_STATIC_LANGUAGE, 			DLG_Options_Label_LangSettings);
	
	// Populate values in the _COMBO_UNITS

	for (UT_uint32 i = 0; i < SIZE_aAlignUnit; i++)
	{
		addItemToCombo(AP_RID_DIALOG_OPTIONS_COMBO_UNITS, pSS->getValue(s_aAlignUnit[i].id));
	}	
	
	// Setup bold font for some controls	
	UINT boldFields[]={AP_RID_DIALOG_OPTIONS_STATIC_UI, AP_RID_DIALOG_OPTIONS_STATIC_APPSTARTUP, 
		AP_RID_DIALOG_OPTIONS_STATIC_LANGUAGE, 0};

	for(UT_uint32 i = 0; boldFields[i]; i++) 
	{
		SendMessageW(GetDlgItem(getHandle(), boldFields[i]), WM_SETFONT, 
			(WPARAM)(AP_Win32Dialog_Options*)getContainer()->getBoldFontHandle(), MAKELPARAM(FALSE /* Redraw */, 0));
	}

	/* Fill up document language */
	m_pVecUILangs = pApp->getInstalledUILanguages();
	if (m_pVecUILangs->getItemCount())		
	{	
		/* Fill all up languages names for UI*/
		for (UT_sint32 i = 0; i < m_pVecUILangs->getItemCount(); i++)
		{
			pLangCode = (const char *) m_pVecUILangs->getNthItem(i);
			
			int id = lang.getIndxFromCode(pLangCode);
			pLang  = lang.getNthLangName(id);
			
			nIndex = addItemToCombo (AP_RID_DIALOG_OPTIONS_COMBO_UILANG, pLang);
			setComboDataItem (AP_RID_DIALOG_OPTIONS_COMBO_UILANG, nIndex, id);		
		}							
	}
	else
		EnableWindow(GetDlgItem(getHandle(), AP_RID_DIALOG_OPTIONS_COMBO_UILANG), FALSE);


	SetWindowLongPtrW(getHandle(), GWLP_USERDATA, (LONG_PTR)this);
    SendMessageW(GetDlgItem (m_hDlg, AP_RID_DIALOG_OPTIONS_COMBO_UNITS), 
		WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		
	SendMessageW(GetDlgItem (m_hDlg, AP_RID_DIALOG_OPTIONS_COMBO_UILANG), 
		WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));	
	
}


INT_PTR CALLBACK AP_Win32Dialog_Options_General::s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{ 	
	if (msg==WM_NOTIFY)
	{
		AP_Win32Dialog_Options_General *pThis = (AP_Win32Dialog_Options_General *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);					
		
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

	Document page
	
*/
AP_Win32Dialog_Options_Document::AP_Win32Dialog_Options_Document()
{
	
}

AP_Win32Dialog_Options_Document::~AP_Win32Dialog_Options_Document()
{
	
}

/*
	
*/	
BOOL AP_Win32Dialog_Options_Document::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);	
	AP_Win32Dialog_Options*	 pParent=  (AP_Win32Dialog_Options*)getContainer();	
	BOOL bChecked;	
	
	switch (wId)
	{		
	
		case AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile:
			bChecked = (IsDlgButtonChecked( hWnd, AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile ) == BST_CHECKED);
			EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod), bChecked );
			EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_SPN_AutoSavePeriodSpin), bChecked );
			EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_TXT_AutoSaveExtension), bChecked );
		return TRUE;

	    case AP_RID_DIALOG_OPTIONS_CHK_LanguageWithKeyboard:  
            pParent->_enableDisableLogic(AP_Dialog_Options::id_CHECK_LANG_WITH_KEYBOARD);
            return TRUE;

		
		default:
		    break;
	}
    return FALSE;
}

/*
	
*/	
void AP_Win32Dialog_Options_Document::_onInitDialog()
{				
	int i;
	const XAP_StringSet * pSS = getApp()->getStringSet();	
	
	// localize controls	
	_DS2(OPTIONS_CHK_AutoSaveFile,				DLG_Options_Label_AutoSaveUnderline);
	_DS2(OPTIONS_STATIC_Interval,				DLG_Options_Label_AutoSaveInterval);
	_DS2(OPTIONS_LBL_AutoSaveMinutes,			DLG_Options_Label_Minutes);	
	_DS2(OPTIONS_STATIC_BidiOptions,			DLG_Options_Label_BiDiOptions);
	_DS2(OPTIONS_CHK_OtherDirectionRtl,			DLG_Options_Label_DirectionRtl);
	_DS2(OPTIONS_LBL_AutoSaveExtension,			DLG_Options_Label_FileExtension);
	
	
	// Set the starting period to 1 minute
	SetDlgItemInt(getHandle(), AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod, 1, FALSE );
	
	// Set the range for the period to 1-360
	SendMessageW(GetDlgItem(getHandle(),AP_RID_DIALOG_OPTIONS_SPN_AutoSavePeriodSpin),UDM_SETBUDDY, (WPARAM) GetDlgItem(getHandle(),AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod),0);	
	SendMessageW(GetDlgItem(getHandle(),AP_RID_DIALOG_OPTIONS_SPN_AutoSavePeriodSpin),UDM_SETRANGE,0,(WPARAM)MAKELONG(MAXAUTOSAVEPERIOD,MINAUTOSAVEPERIOD));
	SendMessageW(GetDlgItem(getHandle(),AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod),EM_LIMITTEXT,(WPARAM)3,(WPARAM)0);
	
	// Limit the extension to 5 characters (plus the period)
	SendMessage(GetDlgItem(getHandle(),AP_RID_DIALOG_OPTIONS_TXT_AutoSaveExtension),EM_LIMITTEXT,(WPARAM)6,(WPARAM)0);
	SetWindowLongPtrW(getHandle(), GWLP_USERDATA, (LONG_PTR)this);	
	
	// Setup bold font for some controls	
	UINT boldFields[]={AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile, AP_RID_DIALOG_OPTIONS_STATIC_BidiOptions, 0};

	for(i=0; boldFields[i]; i++) 
	{
		SendMessageW(GetDlgItem(getHandle(), boldFields[i]), WM_SETFONT, 
			(WPARAM)(AP_Win32Dialog_Options*)getContainer()->getBoldFontHandle(), MAKELPARAM(FALSE /* Redraw */, 0));
	}

}

bool AP_Win32Dialog_Options_Document::isAutoSaveInRange()
{
	int iValue = GetDlgItemInt(getHandle(), AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod, NULL, FALSE);
	char szTemp[10];
	snprintf( szTemp, 10, "%d", iValue);	
	
	if (iValue<MINAUTOSAVEPERIOD || iValue>MAXAUTOSAVEPERIOD)
	{				
		UT_Win32LocaleString str;			
			const XAP_StringSet * pSS = getApp()->getStringSet();
			
			str.fromUTF8 (pSS->getValue(AP_STRING_ID_DLG_Options_Label_InvalidRangeForAutoSave));
			MessageBoxW(NULL, str.c_str(),L"Abiword",MB_OK);
		
		return false;
	}
	
	return true;	
}


/*

	Smart Quotes page
	
*/
AP_Win32Dialog_Options_SmartQuotes::AP_Win32Dialog_Options_SmartQuotes()
{
	
}


AP_Win32Dialog_Options_SmartQuotes::~AP_Win32Dialog_Options_SmartQuotes()
{
	
}


/*
	
*/	
void AP_Win32Dialog_Options_SmartQuotes::_onInitDialog()
{
	const XAP_StringSet * pSS = getApp()->getStringSet();
	UT_return_if_fail(pSS);

	// localize controls
	_DS2(OPTIONS_CHK_SmartQuotes,			DLG_Options_Label_SmartQuotes);
	_DS2(OPTIONS_CHK_CustomSmartQuotes,		DLG_Options_Label_CustomSmartQuotes);
	_DS2(OPTIONS_LBL_OuterQuoteStyle,		DLG_Options_Label_OuterQuoteStyle);
	_DS2(OPTIONS_LBL_InnerQuoteStyle,		DLG_Options_Label_InnerQuoteStyle);
	
	UT_UCSChar buf[4];

	HWND hQIn  = GetDlgItem(m_hDlg, AP_RID_DIALOG_OPTIONS_COMBO_INNERQUOTE);
	HWND hQOut = GetDlgItem(m_hDlg, AP_RID_DIALOG_OPTIONS_COMBO_OUTERQUOTE);

	for(size_t i = 0; XAP_EncodingManager::smartQuoteStyles[i].leftQuote != (UT_UCSChar)0; i++)
	{
		buf[0] = XAP_EncodingManager::smartQuoteStyles[i].leftQuote;
		buf[1] = 'O';
		buf[2] = XAP_EncodingManager::smartQuoteStyles[i].rightQuote;
		buf[3] = 0;

		LPWSTR szDisplayString = (LPWSTR) g_ucs4_to_utf16(buf, -1, NULL, NULL, NULL);
		if(szDisplayString)
		{			
			SendMessageW(hQIn,  CB_ADDSTRING, 0, (LPARAM)szDisplayString);	
			SendMessageW(hQOut, CB_ADDSTRING, 0, (LPARAM)szDisplayString);	

			FREEP(szDisplayString);
		}
	}
}

/*
	
*/	
BOOL AP_Win32Dialog_Options_SmartQuotes::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);
	
	switch (wId)
	{					
		case AP_RID_DIALOG_OPTIONS_CHK_SmartQuotes:
		{
			bool bChecked = (IsDlgButtonChecked( hWnd, AP_RID_DIALOG_OPTIONS_CHK_SmartQuotes ) == BST_CHECKED);
			EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_CHK_CustomSmartQuotes), bChecked );
			if (!bChecked) {
				EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_COMBO_OUTERQUOTE), bChecked );
				EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_COMBO_INNERQUOTE), bChecked );
			}
			return TRUE;
		}

		case AP_RID_DIALOG_OPTIONS_CHK_CustomSmartQuotes:
		{
			bool bChecked = (IsDlgButtonChecked( hWnd, AP_RID_DIALOG_OPTIONS_CHK_CustomSmartQuotes ) == BST_CHECKED);
			EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_COMBO_OUTERQUOTE), bChecked );
			EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_COMBO_INNERQUOTE), bChecked );
			return TRUE;
		}

		default:
			break;
	}
    return FALSE;
}
