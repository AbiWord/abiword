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

#include "ut_types.h"
#include "ut_string.h"
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
    : AP_Dialog_Options(pDlgFactory,id)
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

#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))
#define _GV(s)		(pSS->getValue(AP_STRING_ID_##s))
#define _GVX(s)		(pSS->getValue(XAP_STRING_ID_##s))

// the order of the tabs

#define SPELL_INDEX		0
#define PREF_INDEX		1
#define VIEW_INDEX		2
#define OTHER_INDEX 3

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

		tie.pszText = (LPSTR) _GV(DLG_Options_TabLabel_Spelling); 
		tie.lParam = AP_RID_DIALOG_OPT_SPL;
		TabCtrl_InsertItem(m_hwndTab, SPELL_INDEX, &tie); 

		tie.pszText = (LPSTR) _GV(DLG_Options_TabLabel_Preferences); 
		tie.lParam = AP_RID_DIALOG_OPT_PREF;
		TabCtrl_InsertItem(m_hwndTab, PREF_INDEX, &tie); 

		tie.pszText = (LPSTR) _GV(DLG_Options_TabLabel_View); 
		tie.lParam = AP_RID_DIALOG_OPT_VIEW;
		TabCtrl_InsertItem(m_hwndTab, VIEW_INDEX, &tie); 

		tie.pszText = (LPSTR) _GV(DLG_Options_TabLabel_Other); 
		tie.lParam = AP_RID_DIALOG_OPT_OTHER;
		TabCtrl_InsertItem(m_hwndTab, OTHER_INDEX, &tie); 

		// finally, create the (modeless) child dialogs
		
		tp.which = AP_RID_DIALOG_OPT_SPL;
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

		tp.which = AP_RID_DIALOG_OPT_VIEW;
		pTemplate = UT_LockDlgRes(hinst, MAKEINTRESOURCE(tp.which));
		w = CreateDialogIndirectParam(hinst, pTemplate, m_hwndTab, 
										(DLGPROC)s_tabProc, (LPARAM)&tp); 
		UT_ASSERT((w
				   && (m_vecSubDlgHWnd.getItemCount()>0)
				   && (w == m_vecSubDlgHWnd.getLastItem())));
		
		tp.which = AP_RID_DIALOG_OPT_OTHER;
		pTemplate = UT_LockDlgRes(hinst, MAKEINTRESOURCE(tp.which));
		w = CreateDialogIndirectParam(hinst, pTemplate, m_hwndTab, 
										(DLGPROC)s_tabProc, (LPARAM)&tp); 
		UT_ASSERT((w
				   && (m_vecSubDlgHWnd.getItemCount()>0)
				   && (w == m_vecSubDlgHWnd.getLastItem())));
	}

	// let XP code tell us what all of the values should be.
	_populateWindowData();
	
	// make sure first tab is selected.
	ShowWindow((HWND)m_vecSubDlgHWnd.getNthItem(0), SW_SHOW);

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
	case AP_RID_DIALOG_OPT_SPL:
		{
			// localize controls
			_DS(OPTIONS_CHK_SpellCheckAsType,		DLG_Options_Label_SpellCheckAsType);
			_DS(OPTIONS_CHK_SpellHideErrors,		DLG_Options_Label_SpellHideErrors);
			_DS(OPTIONS_CHK_SpellSuggest,			DLG_Options_Label_SpellSuggest);
			_DS(OPTIONS_CHK_SpellMainOnly,			DLG_Options_Label_SpellMainOnly);
			_DS(OPTIONS_CHK_SpellUppercase,			DLG_Options_Label_SpellUppercase);
			_DS(OPTIONS_CHK_SpellNumbers,			DLG_Options_Label_SpellNumbers);
			_DS(OPTIONS_CHK_SpellInternet,			DLG_Options_Label_SpellInternet);
			_DS(OPTIONS_LBL_CUSTOMDICT,				DLG_Options_Label_SpellCustomDict);
			_DS(OPTIONS_BTN_CUSTOMDICT,				DLG_Options_Btn_CustomDict);
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
//			_CDB(OPTIONS_CHK_SpellMainOnly,			id_CHECK_SPELL_MAIN_ONLY);
//			_CDB(OPTIONS_CHK_SpellUppercase,		id_CHECK_SPELL_UPPERCASE);
//			_CDB(OPTIONS_CHK_SpellNumbers,			id_CHECK_SPELL_NUMBERS);
//			_CDB(OPTIONS_CHK_SpellInternet,			id_CHECK_SPELL_INTERNET);
		}
		break;

	case AP_RID_DIALOG_OPT_PREF:
		{
			// localize controls
			_DS(OPTIONS_CHK_PrefsAutoSave,			DLG_Options_Label_PrefsAutoSave);
			_DS(OPTIONS_LBL_CURRENTSCHEME,			DLG_Options_Label_PrefsCurrentScheme);

			// TODO need to populate values in the _COMBO_CURRENTSCHEME
//			HWND hwndScheme = GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_COMBO_CURRENTSCHEME);

//			_CDB(OPTIONS_CHK_PrefsAutoSave,			id_CHECK_PREFS_AUTO_SAVE);
		}
		break;

	case AP_RID_DIALOG_OPT_VIEW:
		{
			// localize controls
			_DS(OPTIONS_CHK_ViewShowRuler,			DLG_Options_Label_ViewRuler);
            _DS(OPTIONS_CHK_ViewUnprintable,        DLG_Options_Label_ViewUnprintable);
			_DS(OPTIONS_CHK_ViewCursorBlink,		DLG_Options_Label_ViewCursorBlink);
			_DS(OPTIONS_CHK_ViewShowToolbars,		DLG_Options_Label_ViewToolbars);
			_DS(OPTIONS_CHK_ViewAll,				DLG_Options_Label_ViewAll);
			_DS(OPTIONS_CHK_ViewHiddenText,			DLG_Options_Label_ViewHiddenText);
			_DS(OPTIONS_CHK_ViewUnprintable,		DLG_Options_Label_ViewUnprintable);
			_DS(OPTIONS_LBL_UNITS,					DLG_Options_Label_ViewUnits);
			_DS(OPTIONS_LBL_SHOWHIDE,				DLG_Options_Label_ViewShowHide);
			_DS(OPTIONS_LBL_VIEWFRAME,				DLG_Options_Label_ViewViewFrame);

			// TODO need to populate values in the _COMBO_UNITS
			HWND hwndAlign = GetDlgItem(hWnd, AP_RID_DIALOG_OPTIONS_COMBO_UNITS);
/*			_CASX(hwndAlign, DLG_Unit_inch);
			_CASX(hwndAlign, DLG_Unit_cm);
			_CASX(hwndAlign, DLG_Unit_points);
			_CASX(hwndAlign, DLG_Unit_pico);
*/
			for( int n1 = 0; n1 < SIZE_aAlignUnit; n1++ ) 
				SendMessage(hwndAlign, CB_ADDSTRING, 0, (LPARAM)pSS->getValue(s_aAlignUnit[n1].id));
			SendMessage(hwndAlign, CB_SETCURSEL, (WPARAM) 0, 0);

//			_CDB(OPTIONS_CHK_ViewShowRuler,			id_CHECK_VIEW_SHOW_RULER);
//			_CDB(OPTIONS_CHK_ViewCursorBlink,		id_CHECK_VIEW_CURSOR_BLINK);
//			_CDB(OPTIONS_CHK_ViewShowToolbars,		id_CHECK_VIEW_SHOW_TOOLBARS);
//			_CDB(OPTIONS_CHK_ViewAll,				id_CHECK_VIEW_ALL);
//			_CDB(OPTIONS_CHK_ViewHiddenText,		id_CHECK_VIEW_HIDDEN_TEXT);
//			_CDB(OPTIONS_CHK_ViewUnprintable,		id_CHECK_VIEW_UNPRINTABLE);
		}
		break;
	
	case AP_RID_DIALOG_OPT_OTHER:
		{
			_DS(OPTIONS_CHK_SmartQuotesEnable,		DLG_Options_Label_SmartQuotesEnable);
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
	
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case AP_RID_DIALOG_OPTIONS_CHK_SpellCheckAsType:	_enableDisableLogic(id_CHECK_SPELL_CHECK_AS_TYPE); 	return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_SpellHideErrors:		_enableDisableLogic(id_CHECK_SPELL_HIDE_ERRORS);	return 0;
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

	case AP_RID_DIALOG_OPTIONS_CHK_PrefsAutoSave:		_enableDisableLogic(id_CHECK_PREFS_AUTO_SAVE);		return 0;

	case AP_RID_DIALOG_OPTIONS_CHK_SmartQuotesEnable:	_enableDisableLogic(id_CHECK_SMART_QUOTES_ENABLE);	return 0;

	case AP_RID_DIALOG_OPTIONS_COMBO_CURRENTSCHEME:
		return 0;

	case AP_RID_DIALOG_OPTIONS_CHK_ViewShowRuler:		_enableDisableLogic(id_CHECK_VIEW_SHOW_RULER);		return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_ViewCursorBlink:		_enableDisableLogic(id_CHECK_VIEW_CURSOR_BLINK);	return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_ViewShowToolbars:	_enableDisableLogic(id_CHECK_VIEW_SHOW_TOOLBARS);	return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_ViewAll:				_enableDisableLogic(id_CHECK_VIEW_ALL);				return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_ViewHiddenText:		_enableDisableLogic(id_CHECK_VIEW_HIDDEN_TEXT);		return 0;
	case AP_RID_DIALOG_OPTIONS_CHK_ViewUnprintable:		_enableDisableLogic(id_CHECK_VIEW_UNPRINTABLE);		return 0;

	case AP_RID_DIALOG_OPTIONS_COMBO_UNITS:
		return 0;

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

	case AP_RID_DIALOG_OPTIONS_BTN_DEFAULT:		// restore default (factory) settings
		UT_DEBUGMSG(("OptionsBtnDefault\n"));
		_event_SetDefaults();
		return 0;
		
	case IDCANCEL:								// also AP_RID_DIALOG_OPTIONS_BTN_CANCEL
		m_answer = a_CANCEL;
		EndDialog(hWnd,0);
		return 0;

	case IDOK:									// also AP_RID_DIALOG_OPTIONS_BTN_OK
		_storeWindowData();						// remember current settings
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

void AP_Win32Dialog_Options::_controlEnable( tControl id, UT_Bool value )
{
	// This routine is called by XP code to enable/disable a particular field.
	
	switch (id)
	{
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
		
	case id_CHECK_PREFS_AUTO_SAVE:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(PREF_INDEX),AP_RID_DIALOG_OPTIONS_CHK_PrefsAutoSave),value);
		return;
		
	case id_CHECK_VIEW_SHOW_RULER:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(VIEW_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ViewShowRuler),value);
		return;
		
	case id_CHECK_VIEW_CURSOR_BLINK:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(VIEW_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ViewCursorBlink),value);
		return;
		
	case id_CHECK_VIEW_SHOW_TOOLBARS:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(VIEW_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ViewShowToolbars),value);
		return;
		
	case id_CHECK_VIEW_ALL:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(VIEW_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ViewAll),value);
		return;
		
	case id_CHECK_VIEW_HIDDEN_TEXT:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(VIEW_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ViewHiddenText),value);
		return;
		
	case id_CHECK_VIEW_UNPRINTABLE:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(VIEW_INDEX),AP_RID_DIALOG_OPTIONS_CHK_ViewUnprintable),value);
		return;
		
	case id_CHECK_SMART_QUOTES_ENABLE:
		EnableWindow(GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(OTHER_INDEX),id_CHECK_SMART_QUOTES_ENABLE),value);
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
	UT_Bool AP_Win32Dialog_Options::_gather##button(void)													\
	{ return IsDlgButtonChecked((HWND)m_vecSubDlgHWnd.getNthItem(index),AP_RID_DIALOG_OPTIONS_CHK_##button); }	\
	void AP_Win32Dialog_Options::_set##button(UT_Bool b)													\
	{ CheckDlgButton((HWND)m_vecSubDlgHWnd.getNthItem(index),AP_RID_DIALOG_OPTIONS_CHK_##button,b); }

DEFINE_GET_SET_BOOL(SPELL_INDEX,SpellCheckAsType);
DEFINE_GET_SET_BOOL(SPELL_INDEX,SpellHideErrors);
DEFINE_GET_SET_BOOL(SPELL_INDEX,SpellSuggest);
DEFINE_GET_SET_BOOL(SPELL_INDEX,SpellMainOnly);
DEFINE_GET_SET_BOOL(SPELL_INDEX,SpellUppercase);
DEFINE_GET_SET_BOOL(SPELL_INDEX,SpellNumbers);
DEFINE_GET_SET_BOOL(SPELL_INDEX,SpellInternet);

DEFINE_GET_SET_BOOL(OTHER_INDEX,SmartQuotesEnable);

DEFINE_GET_SET_BOOL(PREF_INDEX,PrefsAutoSave);

DEFINE_GET_SET_BOOL(VIEW_INDEX,ViewShowRuler);
DEFINE_GET_SET_BOOL(VIEW_INDEX,ViewCursorBlink);
DEFINE_GET_SET_BOOL(VIEW_INDEX,ViewShowToolbars);

DEFINE_GET_SET_BOOL(VIEW_INDEX,ViewAll);
DEFINE_GET_SET_BOOL(VIEW_INDEX,ViewHiddenText);
DEFINE_GET_SET_BOOL(VIEW_INDEX,ViewUnprintable);

#undef DEFINE_GET_SET_BOOL

UT_Dimension AP_Win32Dialog_Options::_gatherViewRulerUnits(void) 
{
	HWND hwndAlign = GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(VIEW_INDEX), AP_RID_DIALOG_OPTIONS_COMBO_UNITS);
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

	HWND hwndAlign = GetDlgItem((HWND)m_vecSubDlgHWnd.getNthItem(VIEW_INDEX), AP_RID_DIALOG_OPTIONS_COMBO_UNITS);
	SendMessage(hwndAlign, CB_SETCURSEL, (WPARAM)n1, 0);
}

int AP_Win32Dialog_Options::_gatherNotebookPageNum(void) 
{
	return 0;
}

void    AP_Win32Dialog_Options::_setNotebookPageNum(int pn) 
{
}
