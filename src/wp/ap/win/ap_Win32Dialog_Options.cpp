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
#include "xap_Win32Frame.h"
#include "xap_Prefs.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"

#include "ap_Win32Resources.rc2"
#include "ap_Win32Dialog_Options.h"
#include "ap_Win32Dialog_Background.h"

#include "fp_PageSize.h"

/*****************************************************************/

#define GWL(hwnd)		(AP_Win32Dialog_Options*)GetWindowLong((hwnd), DWL_USER)
#define SWL(hwnd, d)	(AP_Win32Dialog_Options*)SetWindowLong((hwnd), DWL_USER,(LONG)(d))

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
}

AP_Win32Dialog_Options::~AP_Win32Dialog_Options(void)
{
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void AP_Win32Dialog_Options::runModal(XAP_Frame * pFrame)
{
	// save for use with event
	m_pFrame = pFrame;

	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == AP_DIALOG_ID_OPTIONS);

	lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_OPTIONS);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
								pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT((result != -1));
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

BOOL CALLBACK AP_Win32Dialog_Options::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.
	// This is the dialog procedure for the top-level dialog (that contains
	// the OK/Cancel buttons and the Tab-control).

	AP_Win32Dialog_Options * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_Options *)lParam;
		SWL(hWnd,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = GWL(hWnd);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	case WM_NOTIFY:
		pThis = GWL(hWnd);
		return pThis->_onNotify(hWnd,lParam);
		
	default:
		return 0;
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

BOOL AP_Win32Dialog_Options::_onNotify(HWND hWnd, LPARAM lParam)
{
	// This handles WM_NOTIFY messages for the top-level dialog.
	
	LPNMHDR pNmhdr = (LPNMHDR)lParam;

	switch (pNmhdr->code)
	{
	case TCN_SELCHANGING:
		// TODO: consider validating data before leaving page
		break;

	case TCN_SELCHANGE:
		{
			UT_uint32 iTo = TabCtrl_GetCurSel(pNmhdr->hwndFrom);  
			for (UT_uint32 k=0; k<m_vecSubDlgHWnd.getItemCount(); k++)
				ShowWindow((HWND)m_vecSubDlgHWnd.getNthItem(k), ((k==iTo) ? SW_SHOW : SW_HIDE));
			break;
		}
		
	// Process other notifications here
	default:
		break;
	} 

	return 0;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// this little struct gets passed into s_tabProc
// it's on the stack so don't rely on it to be valid later.
typedef struct _tabParam 
{
	AP_Win32Dialog_Options *	pThis;
	WORD which;
} TabParam;

BOOL CALLBACK AP_Win32Dialog_Options::s_tabProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.
	// This is a pseudo-dialog procedure for the tab-control.

	AP_Win32Dialog_Options * pThis;

	switch (msg)
	{
	case WM_INITDIALOG:
		{
			TabParam * pTP = (TabParam *) lParam;

			// from now on, we can just remember pThis 
			pThis = pTP->pThis;
			SWL(hWnd,pThis);
			return pThis->_onInitTab(hWnd,wParam,lParam);
		}
		
	case WM_COMMAND:
		pThis = GWL(hWnd);
		UT_DEBUGMSG(("s_tabProc: received WM_COMMAND [wParam 0x%08lx][lParam 0x%08lx]\n",wParam,lParam));
		return pThis->_onCommandTab(hWnd,wParam,lParam);

	default:
		return 0;
	}
}

struct {
	UT_Dimension  dim;
	int 		  id;
} s_aAlignUnit[] = 
{
	{ DIM_IN, XAP_STRING_ID_DLG_Unit_inch },
	{ DIM_CM, XAP_STRING_ID_DLG_Unit_cm },
	{ DIM_PT, XAP_STRING_ID_DLG_Unit_points },
	{ DIM_PI, XAP_STRING_ID_DLG_Unit_pico },
};
#define SIZE_aAlignUnit  (sizeof(s_aAlignUnit)/sizeof(s_aAlignUnit[0]))

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// As Tabbed Dialogs have problems with HotKeys, these macros have been replaced to remove &
//#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
//#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))
#define _DS(c,s)  { \
                    XML_Char* p = NULL; \
                    UT_XML_cloneNoAmpersands( p, pSS->getValue(AP_STRING_ID_##s));\
                    SetDlgItemText(hWnd,AP_RID_DIALOG_##c,p); \
					FREEP(p); \
                  }
#define _DSX(c,s) { \
                    XML_Char* p = NULL; \
                    UT_XML_cloneNoAmpersands( p, pSS->getValue(XAP_STRING_ID_##s));\
                    SetDlgItemText(hWnd,AP_RID_DIALOG_##c,p); \
					FREEP(p); \
                  }
#define _GV(s)		(pSS->getValue(AP_STRING_ID_##s))
#define _GVX(s) 	(pSS->getValue(XAP_STRING_ID_##s))

// the order of the tabs

#define TOOLBARS_INDEX		0
#define SPELL_INDEX 		1
#define LAYOUT_INDEX		2
#define PREF_INDEX			3

BOOL AP_Win32Dialog_Options::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// This handles the WM_INITDIALOG message for the top-level dialog.
	
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_Options_OptionsTitle));

	// localize controls
	_DS(OPTIONS_BTN_SAVE,			DLG_Options_Btn_Save);
	_DS(OPTIONS_BTN_DEFAULT,		DLG_Options_Btn_Default);
	_DSX(OPTIONS_BTN_OK,			DLG_OK);
	_DSX(OPTIONS_BTN_CANCEL,		DLG_Cancel);

	// setup the tabs
	{
		TabParam tp;
		TCITEM tie; 

		XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
		HINSTANCE hinst = pWin32App->getInstance();
		DLGTEMPLATE * pTemplate = NULL;
		HWND w = NULL;

		tp.pThis = this;

		// remember the windows we're using 
		
		m_hwndDlg = hWnd;
		m_hwndTab = GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_TAB);

		// add a tab for each of the child dialog boxes
	
		tie.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM; 
		tie.iImage = -1; 

		tie.pszText = (LPSTR) _GV(DLG_Options_Label_Toolbars); 
		tie.lParam = AP_RID_DIALOG_OPT_TOOLBARS;
		TabCtrl_InsertItem(m_hwndTab, TOOLBARS_INDEX, &tie); 

		tie.pszText = (LPSTR) _GV(DLG_Options_TabLabel_Spelling); 
		tie.lParam = AP_RID_DIALOG_OPT_SPELL;
		TabCtrl_InsertItem(m_hwndTab, SPELL_INDEX, &tie); 

		tie.pszText = (LPSTR) _GV(DLG_Options_Label_Layout); 
		tie.lParam = AP_RID_DIALOG_OPT_LAYOUT;
		TabCtrl_InsertItem(m_hwndTab, LAYOUT_INDEX, &tie); 

		tie.pszText = (LPSTR) _GV(DLG_Options_TabLabel_Preferences); 
		tie.lParam = AP_RID_DIALOG_OPT_PREF;
		TabCtrl_InsertItem(m_hwndTab, PREF_INDEX, &tie); 

		// finally, create the (modeless) child dialogs
		
		tp.which = AP_RID_DIALOG_OPT_TOOLBARS;
		pTemplate = UT_LockDlgRes(hinst, MAKEINTRESOURCE(tp.which));
		w = CreateDialogIndirectParam(hinst, pTemplate, m_hwndTab, 
										(DLGPROC)s_tabProc, (LPARAM)&tp);
		UT_ASSERT((w
				   && (m_vecSubDlgHWnd.getItemCount()>0)
				   && (w == m_vecSubDlgHWnd.getLastItem())));

		tp.which = AP_RID_DIALOG_OPT_SPELL;
		pTemplate = UT_LockDlgRes(hinst, MAKEINTRESOURCE(tp.which));
		w = CreateDialogIndirectParam(hinst, pTemplate, m_hwndTab, 
										(DLGPROC)s_tabProc, (LPARAM)&tp); 
		UT_ASSERT((w
				   && (m_vecSubDlgHWnd.getItemCount()>0)
				   && (w == m_vecSubDlgHWnd.getLastItem())));

		tp.which = AP_RID_DIALOG_OPT_LAYOUT;
		pTemplate = UT_LockDlgRes(hinst, MAKEINTRESOURCE(tp.which));
		w = CreateDialogIndirectParam(hinst, pTemplate, m_hwndTab, 
										(DLGPROC)s_tabProc, (LPARAM)&tp); 
		UT_ASSERT((w
				   && (m_vecSubDlgHWnd.getItemCount()>0)
				   && (w == m_vecSubDlgHWnd.getLastItem())));
		
		tp.which = AP_RID_DIALOG_OPT_PREF;
		pTemplate = UT_LockDlgRes(hinst, MAKEINTRESOURCE(tp.which));
		w = CreateDialogIndirectParam(hinst, pTemplate, m_hwndTab, 
										(DLGPROC)s_tabProc, (LPARAM)&tp); 
		UT_ASSERT((w
				   && (m_vecSubDlgHWnd.getItemCount()>0)
				   && (w == m_vecSubDlgHWnd.getLastItem())));
	}

	// let XP code tell us what all of the values should be.
	_populateWindowData();

	// This has to follow the call to _populateWindowData()
	_initializeTransperentToggle();

	// Use the InitialPageNum to determine which tab is selected.
	TabCtrl_SetCurSel( m_hwndTab, getInitialPageNum() );
	ShowWindow((HWND)m_vecSubDlgHWnd.getNthItem( getInitialPageNum() ), SW_SHOW);

	return 1;							// 1 == we did not call SetFocus()
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

#define _CAS(w,s)	SendMessage(w, CB_ADDSTRING, 0, (LPARAM) _GV(s))
#define _CASX(w,s)	SendMessage(w, CB_ADDSTRING, 0, (LPARAM) _GVX(s))
#define _CDB(c,i)	CheckDlgButton(hWnd,AP_RID_DIALOG_##c,_getCheckItemValue(i))

BOOL AP_Win32Dialog_Options::_onInitTab(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// This handles the WM_INITDIALOG message for the tab-control

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// position ourselves w.r.t. containing tab

	RECT r;
	GetClientRect(m_hwndTab, &r);
	TabCtrl_AdjustRect(m_hwndTab, FALSE, &r);
	SetWindowPos(hWnd, HWND_TOP, r.left, r.top, 0, 0, SWP_NOSIZE); 

	m_vecSubDlgHWnd.addItem(hWnd);
	
	TabParam * pTP = (TabParam *) lParam;
	switch (pTP->which)
	{
	case AP_RID_DIALOG_OPT_TOOLBARS:
		{
			_DS(OPTIONS_FRM_Toolbars,				DLG_Options_Label_Toolbars);
			_DS(OPTIONS_CHK_ViewShowStandardBar,	DLG_Options_Label_ViewStandardTB);
			_DS(OPTIONS_CHK_ViewShowFormatBar,		DLG_Options_Label_ViewFormatTB);
			_DS(OPTIONS_CHK_ViewShowExtraBar,		DLG_Options_Label_ViewExtraTB);
			_DS(OPTIONS_FRM_ButtonStyle,			DLG_Options_Label_Look);
			_DS(OPTIONS_RDO_Icons,					DLG_Options_Label_Icons);
			_DS(OPTIONS_RDO_Text,					DLG_Options_Label_Text);
			_DS(OPTIONS_RDO_IconsAndText,			DLG_Options_Label_Both);
			_DS(OPTIONS_CHK_ViewToolTips,			DLG_Options_Label_ViewTooltips);

			// TODO:  make the following 4 controls usable
			EnableWindow( GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_RDO_Icons),		false );
			EnableWindow( GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_RDO_Text), 		false );
			EnableWindow( GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_RDO_IconsAndText), false );
			EnableWindow( GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_CHK_ViewToolTips), false );
			
		}
		break;
			
	case AP_RID_DIALOG_OPT_SPELL:
		{
			// localize controls
			_DS(OPTIONS_FRM_SpellGeneral,			DLG_Options_Label_General);
			_DS(OPTIONS_CHK_SpellCheckAsType,		DLG_Options_Label_SpellCheckAsType);
			_DS(OPTIONS_CHK_SpellHideErrors,		DLG_Options_Label_SpellHideErrors);
			_DS(OPTIONS_CHK_SpellSuggest,			DLG_Options_Label_SpellSuggest);
			_DS(OPTIONS_CHK_SpellMainOnly,			DLG_Options_Label_SpellMainOnly);
			_DS(OPTIONS_FRM_SpellIgnore,			DLG_Options_Label_Ignore);
			_DS(OPTIONS_CHK_SpellUppercase, 		DLG_Options_Label_SpellUppercase);
			_DS(OPTIONS_CHK_SpellNumbers,			DLG_Options_Label_SpellNumbers);
			_DS(OPTIONS_CHK_SpellInternet,			DLG_Options_Label_SpellInternet);
			_DS(OPTIONS_LBL_CUSTOMDICT, 			DLG_Options_Label_SpellCustomDict);
			_DS(OPTIONS_BTN_CUSTOMDICT, 			DLG_Options_Btn_CustomDict);
			_DS(OPTIONS_LBL_IGNOREDWORD,			DLG_Options_Label_SpellIgnoredWord);
			_DS(OPTIONS_BTN_IGNOREDRESET,			DLG_Options_Btn_IgnoreReset);
			_DS(OPTIONS_BTN_IGNOREDEDIT,			DLG_Options_Btn_IgnoreEdit);

			// TODO need to populate values in the _COMBO_CUSTOMDICT
			HWND hwndDict = GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_COMBO_CUSTOMDICT);
			SendMessage(hwndDict, CB_ADDSTRING, 0, (LPARAM)"custom.dic");	// TODO - get from prefs / var
			SendMessage(hwndDict, CB_SETCURSEL, (WPARAM) 0, 0);

			// set initial state
//			_CDB(OPTIONS_CHK_SpellCheckAsType,		id_CHECK_SPELL_CHECK_AS_TYPE);
//			_CDB(OPTIONS_CHK_SpellHideErrors,		id_CHECK_SPELL_HIDE_ERRORS);
//			_CDB(OPTIONS_CHK_SpellSuggest,			id_CHECK_SPELL_SUGGEST);
//			_CDB(OPTIONS_CHK_SpellMainOnly, 		id_CHECK_SPELL_MAIN_ONLY);
//			_CDB(OPTIONS_CHK_SpellUppercase,		id_CHECK_SPELL_UPPERCASE);
//			_CDB(OPTIONS_CHK_SpellNumbers,			id_CHECK_SPELL_NUMBERS);
//			_CDB(OPTIONS_CHK_SpellInternet, 		id_CHECK_SPELL_INTERNET);
		}
		break;

	case AP_RID_DIALOG_OPT_LAYOUT:
		{
			// localize controls
			_DS(OPTIONS_FRM_SHOWHIDE,				DLG_Options_Label_ViewShowHide);
			_DS(OPTIONS_CHK_ViewShowRuler,			DLG_Options_Label_ViewRuler);
			_DS(OPTIONS_CHK_ViewCursorBlink,		DLG_Options_Label_ViewCursorBlink);
			_DS(OPTIONS_CHK_ViewShowStatusBar,		DLG_Options_Label_ViewStatusBar);
			_DS(OPTIONS_FRM_VIEWFRAME,				DLG_Options_Label_ViewViewFrame);
			_DS(OPTIONS_CHK_ViewAll,				DLG_Options_Label_ViewAll);
			_DS(OPTIONS_CHK_ViewHiddenText, 		DLG_Options_Label_ViewHiddenText);
			_DS(OPTIONS_CHK_ViewUnprintable,		DLG_Options_Label_ViewUnprintable);
			_DS(OPTIONS_LBL_UNITS,					DLG_Options_Label_ViewUnits);
			_DS(OPTIONS_LBL_DefaultPageSize,		DLG_Options_Label_DefaultPageSize);
			_DS(OPTIONS_CHK_SmartQuotesEnable,		DLG_Options_Label_SmartQuotesEnable);
			_DS(OPTIONS_CHK_BGColorEnable,			DLG_Options_Label_CheckWhiteForTransparent);
			_DS(OPTIONS_BTN_BGColor,				DLG_Options_Label_ChooseForTransparent);

			// Populate values in the _COMBO_UNITS
			HWND hwndAlign = GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_COMBO_UNITS);
			for( int n1 = 0; n1 < SIZE_aAlignUnit; n1++ ) 
			{
				SendMessage(hwndAlign, CB_ADDSTRING, 0, (LPARAM)pSS->getValue(s_aAlignUnit[n1].id));
			}

			HWND hwndPaperSize = GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_COMBO_DefaultPageSize);
			for( int n2 = (int)fp_PageSize::_first_predefined_pagesize_; n2 < (int)fp_PageSize::_last_predefined_pagesize_dont_use_; n2++ ) 
			{
				SendMessage( hwndPaperSize, 
							 CB_INSERTSTRING, 
							 (WPARAM) n2,
							 (LPARAM) fp_PageSize::PredefinedToName( (fp_PageSize::Predefined)n2) );
			}
		}
		break;
	
	case AP_RID_DIALOG_OPT_PREF:
		{
			// localize controls
			_DS(OPTIONS_FRM_PreferenceScheme,		DLG_Options_Label_Schemes);
			_DS(OPTIONS_CHK_PrefsAutoSave,			DLG_Options_Label_PrefsAutoSave);
			_DS(OPTIONS_LBL_CURRENTSCHEME,			DLG_Options_Label_PrefsCurrentScheme);

			_DS(OPTIONS_FRM_AutoSaveOptions,		DLG_Options_Label_AutoSave);
			_DS(OPTIONS_CHK_AutoSaveFile,			DLG_Options_Label_AutoSaveCurrent);
			_DS(OPTIONS_LBL_AutoSaveMinutes,		DLG_Options_Label_Minutes);
			_DS(OPTIONS_LBL_AutoSaveExtension,		DLG_Options_Label_WithExtension);
			_DS(OPTIONS_CHK_ShowSplash, 			DLG_Options_Label_ShowSplash);

			// Set the starting period to 1 minute
			SetDlgItemInt(hWnd, AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod, 1, FALSE );

			// Set the range for the period to 1-360
			SendMessage(GetDlgItem(hWnd,AP_RID_DIALOG_OPTIONS_SPN_AutoSavePeriodSpin),UDM_SETRANGE,0,(WPARAM)MAKELONG(120,1));
			SendMessage(GetDlgItem(hWnd,AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod),EM_LIMITTEXT,(WPARAM)3,(WPARAM)0);

			// Limit the extension to 5 characters (plus the period)
			SendMessage(GetDlgItem(hWnd,AP_RID_DIALOG_OPTIONS_TXT_AutoSaveExtension),EM_LIMITTEXT,(WPARAM)6,(WPARAM)0);

			// Hidi Bidi Controls
			HWND hwndBidiBox = GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_FRM_BidiOptions);
			HWND hwndBidiChk = GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_CHK_OtherDirectionRtl);
			HWND hwndBidiUseContextGlyphs = GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_CHK_OtherUseContextGlyphs);
			HWND hwndBidiSaveContextGlyphs = GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_CHK_OtherSaveContextGlyphs);
			HWND hwndBidiHebrewContextGlyphs = GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_CHK_OtherHebrewContextGlyphs);

			ShowWindow( hwndBidiBox, SW_HIDE);
			ShowWindow( hwndBidiChk, SW_HIDE);
			ShowWindow( hwndBidiUseContextGlyphs, SW_HIDE);
			ShowWindow( hwndBidiHebrewContextGlyphs, SW_HIDE);
			ShowWindow( hwndBidiSaveContextGlyphs, SW_HIDE);

#ifdef BIDI_ENABLED
			ShowWindow( hwndBidiBox, SW_SHOW);
			ShowWindow( hwndBidiChk, SW_SHOW);
			ShowWindow( hwndBidiUseContextGlyphs, SW_SHOW);
			//ShowWindow( hwndBidiSaveContextGlyphs, SW_SHOW);
			ShowWindow( hwndBidiHebrewContextGlyphs, SW_SHOW);
			_DS(OPTIONS_FRM_BidiOptions,			DLG_Options_Label_BiDiOptions);
			_DS(OPTIONS_CHK_OtherDirectionRtl,		DLG_Options_Label_DirectionRtl);
			_DS(OPTIONS_CHK_OtherUseContextGlyphs,	DLG_Options_Label_UseContextGlyphs);
			_DS(OPTIONS_CHK_OtherSaveContextGlyphs, DLG_Options_Label_SaveContextGlyphs);
			_DS(OPTIONS_CHK_OtherHebrewContextGlyphs, DLG_Options_Label_HebrewContextGlyphs);
#endif

			// TODO need to populate values in the _COMBO_CURRENTSCHEME
//			HWND hwndScheme = GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_COMBO_CURRENTSCHEME);
//			_CDB(OPTIONS_CHK_PrefsAutoSave, 		id_CHECK_PREFS_AUTO_SAVE);

		}
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return 1;							// 1 == we did not call SetFocus()
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

BOOL AP_Win32Dialog_Options::_onCommandTab(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// This handles WM_COMMAND message for all of the sub-dialogs.
	BOOL bChecked;
	AP_Dialog_Background *pColorDialog;
	UT_RGBColor rgbColor;
	
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	// TOOLBARS TAB
	case AP_RID_DIALOG_OPTIONS_CHK_ViewShowStandardBar: _enableDisableLogic(id_CHECK_VIEW_SHOW_STANDARD_TOOLBAR);	return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_ViewShowFormatBar:	_enableDisableLogic(id_CHECK_VIEW_SHOW_FORMAT_TOOLBAR); 	return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_ViewShowExtraBar:	_enableDisableLogic(id_CHECK_VIEW_SHOW_EXTRA_TOOLBAR);		return 0;
	// TODO:  Enable the following 4 contrils														
//	case AP_RID_DIALOG_OPTIONS_RDO_Icons:				_enableDisableLogic(id_XXXX);								retrun 0;											
//	case AP_RID_DIALOG_OPTIONS_RDO_Text:				_enableDisableLogic(id_XXXX);								retrun 0;											
//	case AP_RID_DIALOG_OPTIONS_RDO_IconAndTexts:		_enableDisableLogic(id_XXXX);								retrun 0;											
//	case AP_RID_DIALOG_OPTIONS_CHK_ViewToolTips:		_enableDisableLogic(id_XXXX);								return 0;

	// SPELL TAB														
	case AP_RID_DIALOG_OPTIONS_CHK_SpellCheckAsType:	_enableDisableLogic(id_CHECK_SPELL_CHECK_AS_TYPE);	return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_SpellHideErrors: 	_enableDisableLogic(id_CHECK_SPELL_HIDE_ERRORS);	return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_SpellSuggest:		_enableDisableLogic(id_CHECK_SPELL_SUGGEST);		return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_SpellMainOnly:		_enableDisableLogic(id_CHECK_SPELL_MAIN_ONLY);		return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_SpellUppercase:		_enableDisableLogic(id_CHECK_SPELL_UPPERCASE);		return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_SpellNumbers:		_enableDisableLogic(id_CHECK_SPELL_NUMBERS);		return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_SpellInternet:		_enableDisableLogic(id_CHECK_SPELL_INTERNET);		return 0;
	case AP_RID_DIALOG_OPTIONS_COMBO_CUSTOMDICT:
		return 0;
	case AP_RID_DIALOG_OPTIONS_BTN_CUSTOMDICT:
		UT_DEBUGMSG(("WM_Command for BtnCustomDict\n"));
		return 0;
	case AP_RID_DIALOG_OPTIONS_BTN_IGNOREDRESET:
		//_event_IgnoreReset(); 
		UT_DEBUGMSG(("WM_Command for BtnIgnoreReset\n"));
		return 0;
	case AP_RID_DIALOG_OPTIONS_BTN_IGNOREDEDIT:
		UT_DEBUGMSG(("WM_Command for BtnIgnoreEdit\n"));
		return 0;

	// LAYOUT TAB
	case AP_RID_DIALOG_OPTIONS_CHK_ViewShowRuler:		_enableDisableLogic(id_CHECK_VIEW_SHOW_RULER);		return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_ViewCursorBlink: 	_enableDisableLogic(id_CHECK_VIEW_CURSOR_BLINK);	return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_ViewShowStatusBar:	_enableDisableLogic(id_CHECK_VIEW_SHOW_STATUS_BAR); return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_ViewAll: 			_enableDisableLogic(id_CHECK_VIEW_ALL); 			return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_ViewHiddenText:		_enableDisableLogic(id_CHECK_VIEW_HIDDEN_TEXT); 	return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_ViewUnprintable: 	_enableDisableLogic(id_CHECK_VIEW_UNPRINTABLE); 	return 0;
	case AP_RID_DIALOG_OPTIONS_COMBO_UNITS: 																return 0;
	case AP_RID_DIALOG_OPTIONS_COMBO_DefaultPageSize:														return 0;																											
	case AP_RID_DIALOG_OPTIONS_CHK_SmartQuotesEnable:	_enableDisableLogic(id_CHECK_SMART_QUOTES_ENABLE);	return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_BGColorEnable:
		bChecked = (IsDlgButtonChecked( hWnd, AP_RID_DIALOG_OPTIONS_CHK_BGColorEnable ) == BST_CHECKED);
		EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_BTN_BGColor), bChecked );
		return 0;
	case AP_RID_DIALOG_OPTIONS_BTN_BGColor:
		pColorDialog = (AP_Dialog_Background *)(m_pDialogFactory->requestDialog(AP_DIALOG_ID_BACKGROUND));
		UT_ASSERT(pColorDialog);

		UT_parseColor( m_CurrentTransparentColor, rgbColor );

		pColorDialog->setColor( rgbColor );

		pColorDialog->runModal( m_pFrame );

		if( pColorDialog->getAnswer() == AP_Dialog_Background::a_OK )
		{
			strcpy( m_CurrentTransparentColor, pColorDialog->getColor() );
		}
		m_pDialogFactory->releaseDialog( pColorDialog );

		return 0;

	// PREF TAB
	case AP_RID_DIALOG_OPTIONS_CHK_PrefsAutoSave:		_enableDisableLogic(id_CHECK_PREFS_AUTO_SAVE);		return 0;
	case AP_RID_DIALOG_OPTIONS_COMBO_CURRENTSCHEME:
		return 0;
#ifdef BIDI_ENABLED
	case AP_RID_DIALOG_OPTIONS_CHK_OtherDirectionRtl:	_enableDisableLogic(id_CHECK_OTHER_DEFAULT_DIRECTION_RTL);	return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_OtherUseContextGlyphs:	_enableDisableLogic(id_CHECK_OTHER_USE_CONTEXT_GLYPHS); return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_OtherSaveContextGlyphs:	_enableDisableLogic(id_CHECK_OTHER_SAVE_CONTEXT_GLYPHS);return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_OtherHebrewContextGlyphs:  _enableDisableLogic(id_CHECK_OTHER_HEBREW_CONTEXT_GLYPHS);return 0;
#endif
	case AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile:
		_enableDisableLogic(id_CHECK_AUTO_SAVE_FILE);

		bChecked = (IsDlgButtonChecked( hWnd, AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile ) == BST_CHECKED);
		EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod), bChecked );
		EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_SPN_AutoSavePeriodSpin), bChecked );
		EnableWindow( GetDlgItem( hWnd, AP_RID_DIALOG_OPTIONS_TXT_AutoSaveExtension), bChecked );
		return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_ShowSplash:			_enableDisableLogic(id_SHOWSPLASH); 	return 0;

	default:
		UT_DEBUGMSG(("WM_Command for id %ld for sub-dialog\n",wId));
		return 0;
	}
}

BOOL AP_Win32Dialog_Options::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// This handles WM_COMMAND message for the top-level dialog.
	
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case AP_RID_DIALOG_OPTIONS_BTN_SAVE:		// save current settings to disk
		UT_DEBUGMSG(("TODO OptionsBtnSave\n"));
		return 0;

	case AP_RID_DIALOG_OPTIONS_BTN_DEFAULT: 	// restore default (factory) settings
		UT_DEBUGMSG(("OptionsBtnDefault\n"));
		_event_SetDefaults();
		return 0;
		
	case IDCANCEL:								// also AP_RID_DIALOG_OPTIONS_BTN_CANCEL
		m_answer = a_CANCEL;
		EndDialog(hWnd,0);
		return 0;

	case IDOK:									// also AP_RID_DIALOG_OPTIONS_BTN_OK
		if( IsDlgButtonChecked( (HWND)m_vecSubDlgHWnd.getNthItem(LAYOUT_INDEX), AP_RID_DIALOG_OPTIONS_CHK_BGColorEnable ) != BST_CHECKED )
			strcpy( m_CurrentTransparentColor, "ffffff" );

		_storeWindowData(); 					// remember current settings
		m_answer = a_OK;
		EndDialog(hWnd,0);
		return 0;

	default:									// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;								// return zero to let windows take care of it.
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void AP_Win32Dialog_Options::_controlEnable( tControl id, bool value )
{
	// This routine is called by XP code to enable/disable a particular field.

	switch (id)
	{
	case id_CHECK_VIEW_SHOW_STANDARD_TOOLBAR:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(TOOLBARS_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ViewShowStandardBar),value);
		return;

	case id_CHECK_VIEW_SHOW_FORMAT_TOOLBAR:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(TOOLBARS_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ViewShowFormatBar),value);
		return;

	case id_CHECK_VIEW_SHOW_EXTRA_TOOLBAR:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(TOOLBARS_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ViewShowExtraBar),value);
		return;

	case id_CHECK_SPELL_CHECK_AS_TYPE:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(SPELL_INDEX),AP_RID_DIALOG_OPTIONS_CHK_SpellCheckAsType),value);
		return;

	case id_CHECK_SPELL_HIDE_ERRORS:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(SPELL_INDEX),AP_RID_DIALOG_OPTIONS_CHK_SpellHideErrors),value);
		return;

	case id_CHECK_SPELL_SUGGEST:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(SPELL_INDEX),AP_RID_DIALOG_OPTIONS_CHK_SpellSuggest),value);
		return;

	case id_CHECK_SPELL_MAIN_ONLY:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(SPELL_INDEX),AP_RID_DIALOG_OPTIONS_CHK_SpellMainOnly),value);
		return;

	case id_CHECK_SPELL_UPPERCASE:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(SPELL_INDEX),AP_RID_DIALOG_OPTIONS_CHK_SpellUppercase),value);
		return;

	case id_CHECK_SPELL_NUMBERS:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(SPELL_INDEX),AP_RID_DIALOG_OPTIONS_CHK_SpellNumbers),value);
		return;

	case id_CHECK_SPELL_INTERNET:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(SPELL_INDEX),AP_RID_DIALOG_OPTIONS_CHK_SpellInternet),value);
		return;

	case id_CHECK_VIEW_SHOW_RULER:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(LAYOUT_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ViewShowRuler),value);
		return;

	case id_CHECK_VIEW_CURSOR_BLINK:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(LAYOUT_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ViewCursorBlink),value);
		return;

	case id_CHECK_VIEW_SHOW_STATUS_BAR:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(LAYOUT_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ViewShowStatusBar),value);
		return;

	case id_CHECK_VIEW_ALL:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(LAYOUT_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ViewAll),value);
		return;

	case id_CHECK_VIEW_HIDDEN_TEXT:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(LAYOUT_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ViewHiddenText),value);
		return;

	case id_CHECK_VIEW_UNPRINTABLE:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(LAYOUT_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ViewUnprintable),value);
		return;

	case id_CHECK_PREFS_AUTO_SAVE:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(PREF_INDEX),AP_RID_DIALOG_OPTIONS_CHK_PrefsAutoSave),value);
		return;

	case id_CHECK_SMART_QUOTES_ENABLE:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(LAYOUT_INDEX),id_CHECK_SMART_QUOTES_ENABLE),value);
		return;

	case id_SHOWSPLASH:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(PREF_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ShowSplash),value);
		return;

#ifdef BIDI_ENABLED
	case id_CHECK_OTHER_SAVE_CONTEXT_GLYPHS:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(PREF_INDEX),AP_RID_DIALOG_OPTIONS_CHK_OtherSaveContextGlyphs),value);
		return;

	case id_CHECK_OTHER_HEBREW_CONTEXT_GLYPHS:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(PREF_INDEX),AP_RID_DIALOG_OPTIONS_CHK_OtherHebrewContextGlyphs),value);
		return;
#endif

	case id_CHECK_AUTO_SAVE_FILE:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(PREF_INDEX),AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile),value);
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

#define DEFINE_GET_SET_BOOL(index,button)																	\
	bool AP_Win32Dialog_Options::_gather##button(void)													\
	{ return (IsDlgButtonChecked((HWND)m_vecSubDlgHWnd.getNthItem(index),AP_RID_DIALOG_OPTIONS_CHK_##button) == BST_CHECKED); } \
	void AP_Win32Dialog_Options::_set##button(const bool b) 												\
	{ CheckDlgButton((HWND)m_vecSubDlgHWnd.getNthItem(index),AP_RID_DIALOG_OPTIONS_CHK_##button,b); }

DEFINE_GET_SET_BOOL(TOOLBARS_INDEX,ViewShowStandardBar);									   
DEFINE_GET_SET_BOOL(TOOLBARS_INDEX,ViewShowFormatBar);										   
DEFINE_GET_SET_BOOL(TOOLBARS_INDEX,ViewShowExtraBar);

DEFINE_GET_SET_BOOL(SPELL_INDEX,SpellCheckAsType);
DEFINE_GET_SET_BOOL(SPELL_INDEX,SpellHideErrors);
DEFINE_GET_SET_BOOL(SPELL_INDEX,SpellSuggest);
DEFINE_GET_SET_BOOL(SPELL_INDEX,SpellMainOnly);
DEFINE_GET_SET_BOOL(SPELL_INDEX,SpellUppercase);
DEFINE_GET_SET_BOOL(SPELL_INDEX,SpellNumbers);
DEFINE_GET_SET_BOOL(SPELL_INDEX,SpellInternet);

DEFINE_GET_SET_BOOL(LAYOUT_INDEX,ViewShowRuler);
DEFINE_GET_SET_BOOL(LAYOUT_INDEX,ViewShowStatusBar);
DEFINE_GET_SET_BOOL(LAYOUT_INDEX,ViewCursorBlink);
DEFINE_GET_SET_BOOL(LAYOUT_INDEX,ViewAll);
DEFINE_GET_SET_BOOL(LAYOUT_INDEX,ViewHiddenText);
DEFINE_GET_SET_BOOL(LAYOUT_INDEX,ViewUnprintable);
DEFINE_GET_SET_BOOL(LAYOUT_INDEX,SmartQuotesEnable);

DEFINE_GET_SET_BOOL(PREF_INDEX,PrefsAutoSave);
DEFINE_GET_SET_BOOL(PREF_INDEX,ShowSplash);
#ifdef BIDI_ENABLED
DEFINE_GET_SET_BOOL(PREF_INDEX,OtherDirectionRtl);
DEFINE_GET_SET_BOOL(PREF_INDEX,OtherUseContextGlyphs);
DEFINE_GET_SET_BOOL(PREF_INDEX,OtherSaveContextGlyphs);
DEFINE_GET_SET_BOOL(PREF_INDEX,OtherHebrewContextGlyphs);
#endif

#undef DEFINE_GET_SET_BOOL

// TODO:  Add these item to dialog and impliment
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
	return (IsDlgButtonChecked((HWND)m_vecSubDlgHWnd.getNthItem(PREF_INDEX),AP_RID_DIALOG_OPTIONS_CHK_AutoSaveFile) == BST_CHECKED);
}

void AP_Win32Dialog_Options::_setAutoSaveFile(const bool b)
{
	BOOL bChecked;
	HWND hWnd = (HWND)m_vecSubDlgHWnd.getNthItem(PREF_INDEX);

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
	
	GetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(PREF_INDEX), AP_RID_DIALOG_OPTIONS_TXT_AutoSaveExtension, szExtension, 7 );

	stRetVal = szExtension;
}

void AP_Win32Dialog_Options::_setAutoSaveFileExt(const UT_String &stExt)
{
	SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(PREF_INDEX), AP_RID_DIALOG_OPTIONS_TXT_AutoSaveExtension, stExt.c_str() );
}

void AP_Win32Dialog_Options::_gatherAutoSaveFilePeriod(UT_String &stRetVal)
{
	int iValue = GetDlgItemInt((HWND)m_vecSubDlgHWnd.getNthItem(PREF_INDEX), AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod, NULL, FALSE );
	char szTemp[10];

	snprintf( szTemp, 10, "%d", iValue );											 

	stRetVal = szTemp;
}

void AP_Win32Dialog_Options::_setAutoSaveFilePeriod(const UT_String &stPeriod)
{
	SetDlgItemInt((HWND)m_vecSubDlgHWnd.getNthItem(PREF_INDEX), AP_RID_DIALOG_OPTIONS_TXT_AutoSavePeriod, atoi(stPeriod.c_str()), FALSE );
}

UT_Dimension AP_Win32Dialog_Options::_gatherViewRulerUnits(void) 
{
	HWND hwndAlign = GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(LAYOUT_INDEX), AP_RID_DIALOG_OPTIONS_COMBO_UNITS);
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

	HWND hwndAlign = GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(LAYOUT_INDEX), AP_RID_DIALOG_OPTIONS_COMBO_UNITS);
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
	HWND hwndPaperSize = GetDlgItem( (HWND)m_vecSubDlgHWnd.getNthItem(LAYOUT_INDEX), 
									 AP_RID_DIALOG_OPTIONS_COMBO_DefaultPageSize);
	SendMessage(hwndPaperSize, CB_SETCURSEL, (WPARAM)pre, 0);
}

fp_PageSize::Predefined AP_Win32Dialog_Options::_gatherDefaultPageSize(void)
{
	HWND hwndPaperSize = GetDlgItem( (HWND)m_vecSubDlgHWnd.getNthItem(LAYOUT_INDEX), 
									 AP_RID_DIALOG_OPTIONS_COMBO_DefaultPageSize);
	int nSel = SendMessage(hwndPaperSize, CB_GETCURSEL, 0, 0);

	if( nSel != CB_ERR )
		return (fp_PageSize::Predefined) nSel;
	return fp_PageSize::psLetter;
}

void AP_Win32Dialog_Options::_initializeTransperentToggle(void)
{
	HWND hWnd = (HWND)m_vecSubDlgHWnd.getNthItem(LAYOUT_INDEX);

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
