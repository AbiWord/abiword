/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "gr_Win32Graphics.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Dialog_Id.h"
#include "ap_Strings.h"
#include "ap_Preview_Paragraph.h"
#include "ap_Win32Dialog_Paragraph.h"

#include "ap_Win32Resources.rc2"

/*****************************************************************/

#define GWL(hwnd)		(AP_Win32Dialog_Paragraph*)GetWindowLong((hwnd), DWL_USER)
#define SWL(hwnd, d)	(AP_Win32Dialog_Paragraph*)SetWindowLong((hwnd), DWL_USER,(LONG)(d))

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Paragraph::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	AP_Win32Dialog_Paragraph * p = new AP_Win32Dialog_Paragraph(pFactory,id);
	return p;
}

AP_Win32Dialog_Paragraph::AP_Win32Dialog_Paragraph(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_Paragraph(pDlgFactory,id)
{
	m_pGPreview = NULL;
}

AP_Win32Dialog_Paragraph::~AP_Win32Dialog_Paragraph(void)
{
	DELETEP(m_pGPreview);
}


/*****************************************************************/

// TODO: move this to UTIL/WIN

// s_LockDlgRes - loads and locks a dialog template resource. 
// Returns the address of the locked resource. 
// lpszResName - name of the resource  
DLGTEMPLATE * WINAPI s_LockDlgRes(HINSTANCE hinst, LPCSTR lpszResName) 	
{ 
    HRSRC hrsrc = FindResource(NULL, lpszResName, RT_DIALOG); 
    HGLOBAL hglb = LoadResource(hinst, hrsrc); 
    return (DLGTEMPLATE *) LockResource(hglb); 	
} 

/*****************************************************************/

void AP_Win32Dialog_Paragraph::runModal(XAP_Frame * pFrame)
{
	/*
	  This dialog is non-persistent.
	  
	  This dialog should do the following:

	  - Construct itself to represent the paragraph properties
	    in the base class (AP_Dialog_Paragraph).

		The Unix one looks just like Microsoft Word 97's Paragraph
		dialog.

	  - The base class stores all the paragraph parameters in
	    m_paragraphData.

	  On "OK" (or during user-interaction) the dialog should:

	  - Save all the data to the m_paragraphData struct so it
	    can be queried by the caller (edit methods routines).
	  
	  On "Cancel" the dialog should:

	  - Just quit, the data items will be ignored by the caller.

	  On "Tabs..." the dialog should (?):

	  - Just quit, discarding changed data, and let the caller (edit methods)
	    invoke the Tabs dialog.

	*/

	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == AP_DIALOG_ID_PARAGRAPH);

	lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_PARAGRAPH);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
								pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT((result != -1));
}

BOOL CALLBACK AP_Win32Dialog_Paragraph::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	AP_Win32Dialog_Paragraph * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_Paragraph *)lParam;
		SWL(hWnd,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = GWL(hWnd);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	case WM_NOTIFY:
		{
			pThis = GWL(hWnd);

			switch (((LPNMHDR) lParam)->code) 
			{ 
			case TCN_SELCHANGING:
				{
					// TODO: validate data before leaving page
				}
				break;

			case TCN_SELCHANGE:             
				{             
					int iTo = TabCtrl_GetCurSel(pThis->m_hwndTab);  

					// a more general solution would be better here
					ShowWindow(pThis->m_hwndSpacing, (iTo ? SW_HIDE : SW_SHOW)); 
					ShowWindow(pThis->m_hwndBreaks, (!iTo ? SW_HIDE : SW_SHOW)); 
				}
				break;

			// Process other notifications here
			default:
				break;
			} 
		}
		return 0;
		
	default:
		return 0;
	}
}

// this little struct gets passed into s_tabProc
typedef struct _tabParam 
{
	AP_Win32Dialog_Paragraph *	pThis;
	WORD which;
} TabParam;

BOOL CALLBACK AP_Win32Dialog_Paragraph::s_tabProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	AP_Win32Dialog_Paragraph * pThis;

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
		return pThis->_onCommand(hWnd,wParam,lParam);

	default:
		return 0;
	}
}

#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))
#define _GV(s)		(pSS->getValue(AP_STRING_ID_##s))

BOOL AP_Win32Dialog_Paragraph::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_Para_ParaTitle));

	// localize controls
	_DSX(PARA_BTN_OK,			DLG_OK);
	_DSX(PARA_BTN_CANCEL,		DLG_Cancel);

	_DS(PARA_BTN_TABS,			DLG_Para_ButtonTabs);

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
		m_hwndTab = GetDlgItem(hWnd, AP_RID_DIALOG_PARA_TAB);  

		// add a tab for each of the child dialog boxes
    
		tie.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM; 
		tie.iImage = -1; 
		tie.pszText = (LPSTR) _GV(DLG_Para_TabLabelIndentsAndSpacing); 
		tie.lParam = AP_RID_DIALOG_PARA_TAB1;
		TabCtrl_InsertItem(m_hwndTab, 0, &tie); 
		tie.pszText = (LPSTR) _GV(DLG_Para_TabLabelLineAndPageBreaks); 
		tie.lParam = AP_RID_DIALOG_PARA_TAB2;
		TabCtrl_InsertItem(m_hwndTab, 1, &tie); 

		// finally, create the (modeless) child dialogs
		
		tp.which = AP_RID_DIALOG_PARA_TAB1;
		pTemplate = s_LockDlgRes(hinst, MAKEINTRESOURCE(tp.which));
		w = CreateDialogIndirectParam(hinst, pTemplate, m_hwndTab, 
										(DLGPROC)s_tabProc, (LPARAM)&tp); 
		UT_ASSERT((w && (w == m_hwndSpacing)));

		tp.which = AP_RID_DIALOG_PARA_TAB2;
		pTemplate = s_LockDlgRes(hinst, MAKEINTRESOURCE(tp.which));
		w = CreateDialogIndirectParam(hinst, pTemplate, m_hwndTab, 
										(DLGPROC)s_tabProc, (LPARAM)&tp); 
		UT_ASSERT((w && (w == m_hwndBreaks)));
	}

	// HACK: make sure the first tab is visible
	// TODO: trigger selchange logic instead
	ShowWindow(m_hwndSpacing, SW_SHOW);

	return 1;							// 1 == we did not call SetFocus()
}

#define _ID(s)	(AP_STRING_ID_##s)

BOOL AP_Win32Dialog_Paragraph::_onInitTab(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// position ourselves w.r.t. containing tab

	RECT r;
	GetClientRect(m_hwndTab, &r);
	TabCtrl_AdjustRect(m_hwndTab, FALSE, &r);
    SetWindowPos(hWnd, HWND_TOP, r.left, r.top, 0, 0, SWP_NOSIZE); 

	// remember which window is which tab
	
	TabParam * pTP = (TabParam *) lParam;
	switch (pTP->which)
	{
	case AP_RID_DIALOG_PARA_TAB1:		// first tab
		{
			m_hwndSpacing = hWnd;

			// localize controls
			_DS(PARA_TEXT_ALIGN,		DLG_Para_LabelAlignment);
			_DS(PARA_TEXT_INDENT,		DLG_Para_LabelIndentation);
			_DS(PARA_TEXT_LEFT,			DLG_Para_LabelLeft);
			_DS(PARA_TEXT_RIGHT,		DLG_Para_LabelRight);
			_DS(PARA_TEXT_HANG,			DLG_Para_LabelSpecial);
			_DS(PARA_TEXT_BY,			DLG_Para_LabelBy);
			_DS(PARA_TEXT_SPACING,		DLG_Para_LabelSpacing);
			_DS(PARA_TEXT_BEFORE,		DLG_Para_LabelBefore);
			_DS(PARA_TEXT_AFTER,		DLG_Para_LabelAfter);
			_DS(PARA_TEXT_LEAD,			DLG_Para_LabelLineSpacing);
			_DS(PARA_TEXT_AT,			DLG_Para_LabelAt);

			// set initial state
			{
				HWND hwndAlign = GetDlgItem(hWnd, AP_RID_DIALOG_PARA_COMBO_ALIGN);  

				UT_uint32 i = 0;

				// TODO: macroize this better
				// TODO: decide whether to use the ID as key, or XP enum
				SendMessage(hwndAlign, LB_ADDSTRING, 0, (LPARAM) _GV(DLG_Para_AlignLeft)); 
				SendMessage(hwndAlign, LB_SETITEMDATA, i, (LPARAM) _ID(DLG_Para_AlignLeft));  
				i++;

				SendMessage(hwndAlign, LB_ADDSTRING, 0, (LPARAM) _GV(DLG_Para_AlignCentered)); 
				SendMessage(hwndAlign, LB_SETITEMDATA, i, (LPARAM) _ID(DLG_Para_AlignCentered));  
				i++;

				SendMessage(hwndAlign, LB_ADDSTRING, 0, (LPARAM) _GV(DLG_Para_AlignRight)); 
				SendMessage(hwndAlign, LB_SETITEMDATA, i, (LPARAM) _ID(DLG_Para_AlignRight));  
				i++;

				SendMessage(hwndAlign, LB_ADDSTRING, 0, (LPARAM) _GV(DLG_Para_AlignJustified)); 
				SendMessage(hwndAlign, LB_SETITEMDATA, i, (LPARAM) _ID(DLG_Para_AlignJustified));  
				i++;
			}		

#if 0

#define AP_RID_DIALOG_PARA_COMBO_HANG			1022
dcl(DLG_Para_SpecialNone,		"(none)")
dcl(DLG_Para_SpecialFirstLine,	"First line")
dcl(DLG_Para_SpecialHanging,	"Hanging")

#define AP_RID_DIALOG_PARA_COMBO_LEAD			1034
dcl(DLG_Para_SpacingSingle,		"Single")
dcl(DLG_Para_SpacingHalf,		"1.5 lines")
dcl(DLG_Para_SpacingDouble,		"Double")
dcl(DLG_Para_SpacingAtLeast,	"At least")
dcl(DLG_Para_SpacingExactly,	"Exactly")
dcl(DLG_Para_SpacingMultiple,	"Multiple")

#define AP_RID_DIALOG_PARA_EDIT_LEFT			1016
#define AP_RID_DIALOG_PARA_SPIN_LEFT			1017
#define AP_RID_DIALOG_PARA_EDIT_RIGHT			1019
#define AP_RID_DIALOG_PARA_SPIN_RIGHT			1020
#define AP_RID_DIALOG_PARA_EDIT_BY				1024
#define AP_RID_DIALOG_PARA_SPIN_BY				1025
#define AP_RID_DIALOG_PARA_EDIT_BEFORE			1028
#define AP_RID_DIALOG_PARA_SPIN_BEFORE			1029
#define AP_RID_DIALOG_PARA_EDIT_AFTER			1031
#define AP_RID_DIALOG_PARA_SPIN_AFTER			1032
#define AP_RID_DIALOG_PARA_EDIT_AT				1036
#define AP_RID_DIALOG_PARA_SPIN_AT				1037

#endif
		}
		break;

	case AP_RID_DIALOG_PARA_TAB2:		// second tab
		{
			m_hwndBreaks = hWnd;

			// localize controls
			_DS(PARA_TEXT_PAGE,			DLG_Para_LabelPagination);
			_DS(PARA_CHECK_WIDOW,		DLG_Para_PushWidowOrphanControl);
			_DS(PARA_CHECK_NEXT,		DLG_Para_PushKeepWithNext);
			_DS(PARA_CHECK_TOGETHER,	DLG_Para_PushKeepLinesTogether);
			_DS(PARA_CHECK_BREAK,		DLG_Para_PushPageBreakBefore);
			_DS(PARA_CHECK_SUPPRESS,	DLG_Para_PushSupressLineNumbers);
			_DS(PARA_CHECK_NOHYPHEN,	DLG_Para_PushNoHyphenate);

			// set initial state
		}
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	// the following are common to each tab

	_DS(PARA_TEXT_PREVIEW,		DLG_Para_LabelPreview);

#if 0

#define AP_RID_DIALOG_PARA_PREVIEW				1004

#endif

	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_Paragraph::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case IDCANCEL:						// also AP_RID_DIALOG_PARA_BTN_CANCEL
		m_paragraphData.m_answer = a_CANCEL;
		EndDialog(hWnd,0);
		return 1;

	case IDOK:							// also AP_RID_DIALOG_PARA_BTN_OK
		// TODO: update rest of m_paragraphData (here, or per-change)
		m_paragraphData.m_answer = a_OK;
		EndDialog(hWnd,0);
		return 1;

	case AP_RID_DIALOG_PARA_BTN_TABS:
		// TODO: shouldn't we update settings, too?
		m_paragraphData.m_answer = a_TABS;
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

