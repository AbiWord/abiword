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
#include "ut_Win32OS.h"

#include "gr_Win32Graphics.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Dialog_Id.h"
#include "ap_Strings.h"
#include "ap_Preview_Paragraph.h"
#include "ap_Win32Dialog_Paragraph.h"
#include "xap_Win32PreviewWidget.h"

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
	m_pPreviewWidget = NULL;
	m_bEditChanged = false;
}

AP_Win32Dialog_Paragraph::~AP_Win32Dialog_Paragraph(void)
{
	DELETEP(m_pPreviewWidget);
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

	// store frame for later use
	m_pFrame = pFrame;
	
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

	case WM_NOTIFY:
		pThis = GWL(hWnd);
		switch (((LPNMHDR)lParam)->code)
		{
		case UDN_DELTAPOS:		return pThis->_onDeltaPos((NM_UPDOWN *)lParam);
		default:				return 0;
		}

	default:
		return 0;
	}
}

/*****************************************************************/

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
		pTemplate = UT_LockDlgRes(hinst, MAKEINTRESOURCE(tp.which));
		w = CreateDialogIndirectParam(hinst, pTemplate, m_hwndTab, 
										(DLGPROC)s_tabProc, (LPARAM)&tp); 
		UT_ASSERT((w && (w == m_hwndSpacing)));

		tp.which = AP_RID_DIALOG_PARA_TAB2;
		pTemplate = UT_LockDlgRes(hinst, MAKEINTRESOURCE(tp.which));
		w = CreateDialogIndirectParam(hinst, pTemplate, m_hwndTab, 
										(DLGPROC)s_tabProc, (LPARAM)&tp); 
		UT_ASSERT((w && (w == m_hwndBreaks)));
	}

	// HACK: make sure the first tab is visible
	// TODO: trigger selchange logic instead
	ShowWindow(m_hwndSpacing, SW_SHOW);

	// sync all controls once to get started
	// HACK: the first arg gets ignored
	_syncControls(id_MENU_ALIGNMENT, true);

	return 1;							// 1 == we did not call SetFocus()
}

/*****************************************************************/

#define _CAS(w,s)	SendMessage(w, CB_ADDSTRING, 0, (LPARAM) _GV(s))
#define _SST(c,i)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,_getSpinItemValue(i))
#define _CDB(c,i)	CheckDlgButton(hWnd,AP_RID_DIALOG_##c,_getCheckItemValue(i))

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

			// Hide Bidi Check Box unless required
			{
				HWND hwndBidi = GetDlgItem(hWnd, AP_RID_DIALOG_PARA_CHECK_BIDI);
				ShowWindow(hwndBidi,SW_HIDE);
#ifdef BIDI_ENABLED
				ShowWindow(hwndBidi,SW_SHOW);
#endif
			}

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
#ifdef BIDI_ENABLED
			_DS(PARA_CHECK_BIDI,		DLG_Para_DomDirection);
#endif

			// populate fixed choices
			{
				HWND hwndAlign = GetDlgItem(hWnd, AP_RID_DIALOG_PARA_COMBO_ALIGN);  
				_CAS(hwndAlign, DLG_Para_AlignLeft);
				_CAS(hwndAlign, DLG_Para_AlignCentered);
				_CAS(hwndAlign, DLG_Para_AlignRight);
				_CAS(hwndAlign, DLG_Para_AlignJustified);
				SendMessage(hwndAlign, CB_SETCURSEL, (WPARAM) _getMenuItemValue(id_MENU_ALIGNMENT), 0);	

				HWND hwndHang = GetDlgItem(hWnd, AP_RID_DIALOG_PARA_COMBO_HANG);  
				_CAS(hwndHang, DLG_Para_SpecialNone);
				_CAS(hwndHang, DLG_Para_SpecialFirstLine);
				_CAS(hwndHang, DLG_Para_SpecialHanging);
				SendMessage(hwndHang, CB_SETCURSEL, (WPARAM) _getMenuItemValue(id_MENU_SPECIAL_INDENT), 0);	

				HWND hwndLead = GetDlgItem(hWnd, AP_RID_DIALOG_PARA_COMBO_LEAD);  
				_CAS(hwndLead, DLG_Para_SpacingSingle);
				_CAS(hwndLead, DLG_Para_SpacingHalf);
				_CAS(hwndLead, DLG_Para_SpacingDouble);
				_CAS(hwndLead, DLG_Para_SpacingAtLeast);
				_CAS(hwndLead, DLG_Para_SpacingExactly);
				_CAS(hwndLead, DLG_Para_SpacingMultiple);
				SendMessage(hwndLead, CB_SETCURSEL, (WPARAM) _getMenuItemValue(id_MENU_SPECIAL_SPACING), 0);	
			}		

			// set initial state
			_SST(PARA_EDIT_LEFT,	id_SPIN_LEFT_INDENT);
			_SST(PARA_EDIT_RIGHT,	id_SPIN_RIGHT_INDENT);
			_SST(PARA_EDIT_BY,		id_SPIN_SPECIAL_INDENT);
			_SST(PARA_EDIT_BEFORE,	id_SPIN_BEFORE_SPACING);
			_SST(PARA_EDIT_AFTER,	id_SPIN_AFTER_SPACING);
			_SST(PARA_EDIT_AT,		id_SPIN_SPECIAL_SPACING);
#ifdef BIDI_ENABLED
			_CDB(PARA_CHECK_BIDI,	id_CHECK_DOMDIRECTION);
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
			_DS(PARA_CHECK_SUPPRESS,	DLG_Para_PushSuppressLineNumbers);
			_DS(PARA_CHECK_NOHYPHEN,	DLG_Para_PushNoHyphenate);

			// set initial state
			_CDB(PARA_CHECK_WIDOW,		id_CHECK_WIDOW_ORPHAN);
			_CDB(PARA_CHECK_NEXT,		id_CHECK_KEEP_NEXT);
			_CDB(PARA_CHECK_TOGETHER,	id_CHECK_KEEP_LINES);
			_CDB(PARA_CHECK_BREAK,		id_CHECK_PAGE_BREAK);
			_CDB(PARA_CHECK_SUPPRESS,	id_CHECK_SUPPRESS);
			_CDB(PARA_CHECK_NOHYPHEN,	id_CHECK_NO_HYPHENATE);
		}
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	// the following are common to each tab

	_DS(PARA_TEXT_PREVIEW,		DLG_Para_LabelPreview);

	if (!m_pPreviewWidget)
	{
		// for XP purposes, life is simplest if we only have one preview 
		// widget which "floats" above both tabs.  to get the window 
		// parentage right, we use the dimensions and location of the 
		// owner-draw control on the tab to position *another* dummy 
		// window which is parented by the main dialog instead.

		HWND hwndChild = GetDlgItem(hWnd, AP_RID_DIALOG_PARA_PREVIEW);
		HWND hwndFloater = GetDlgItem(m_hwndDlg, AP_RID_DIALOG_PARA_PREVIEW);

		RECT r2;
		GetWindowRect(hwndChild, &r2);

		POINT pt;
		pt.x = r2.left;
		pt.y = r2.top;
		ScreenToClient(m_hwndDlg, &pt);

		SetWindowPos(hwndFloater, HWND_TOP, pt.x, pt.y, 
					 r2.right - r2.left, r2.bottom - r2.top, SWP_NOREDRAW);

		// use this floater window as a parent to the widget that we create
		// here and thus have complete control of.

		m_pPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),
													  hwndFloater,
													  0);

		// instantiate the XP preview object using the win32 preview widget (window)
		// we just created.  we seem to have a mish-mash of terms here, sorry.
		
		UT_uint32 w,h;
		m_pPreviewWidget->getWindowSize(&w,&h);
		
		_createPreviewFromGC(m_pPreviewWidget->getGraphics(),w,h);
		m_pPreviewWidget->setPreview(m_paragraphPreview); // we need this to call draw() on WM_PAINTs
//		_updatePreview();
	}

	return 1;							// 1 == we did not call SetFocus()
}

/*****************************************************************/

#define _COMBO(c,i)				\
	case AP_RID_DIALOG_##c:		\
		switch (HIWORD(wParam))	\
		{						\
			case CBN_SELCHANGE:	\
				_setMenuItemValue(i,SendMessage(hWndCtrl,CB_GETCURSEL,0,0));	\
				return 1;	\
							\
			default:		\
				return 0;	\
		}					\
		break;				\

// to ensure the accurate value is displayed to the user
// we call "_syncControls(i)" after we set the variable, to
// catch any changes the _setSpinItemValue() call might
// have done to validate data
#define _EDIT(c,i)				\
	case AP_RID_DIALOG_##c:		\
		switch (wNotifyCode)	\
		{						\
			case EN_CHANGE:		\
				m_bEditChanged = true;		\
				return 1;		\
								\
			case EN_KILLFOCUS:	\
				char buf[SPIN_BUF_TEXT_SIZE];	\
				GetWindowText(hWndCtrl,buf,SPIN_BUF_TEXT_SIZE);		\
				_setSpinItemValue(i,buf);		\
				_syncControls(i);				\
				m_bEditChanged = false;		\
				return 1;	\
							\
			default:		\
				return 0;	\
		}					\
		break;				\

#define _CHECK(c,i)				\
	case AP_RID_DIALOG_##c:		\
		_setCheckItemValue(i,(tCheckState) IsDlgButtonChecked(hWnd,AP_RID_DIALOG_##c));	\
		return 1;			\

/*****************************************************************/

BOOL AP_Win32Dialog_Paragraph::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	_COMBO(PARA_COMBO_ALIGN,	id_MENU_ALIGNMENT);
	_COMBO(PARA_COMBO_HANG,		id_MENU_SPECIAL_INDENT);
	_COMBO(PARA_COMBO_LEAD,		id_MENU_SPECIAL_SPACING);

	_EDIT(PARA_EDIT_LEFT,		id_SPIN_LEFT_INDENT);
	_EDIT(PARA_EDIT_RIGHT,		id_SPIN_RIGHT_INDENT);
	_EDIT(PARA_EDIT_BY,			id_SPIN_SPECIAL_INDENT);
	_EDIT(PARA_EDIT_BEFORE,		id_SPIN_BEFORE_SPACING);
	_EDIT(PARA_EDIT_AFTER,		id_SPIN_AFTER_SPACING);
	_EDIT(PARA_EDIT_AT,			id_SPIN_SPECIAL_SPACING);

	_CHECK(PARA_CHECK_WIDOW,	id_CHECK_WIDOW_ORPHAN);
	_CHECK(PARA_CHECK_NEXT,		id_CHECK_KEEP_NEXT);
	_CHECK(PARA_CHECK_TOGETHER,	id_CHECK_KEEP_LINES);
	_CHECK(PARA_CHECK_BREAK,	id_CHECK_PAGE_BREAK);
	_CHECK(PARA_CHECK_SUPPRESS,	id_CHECK_SUPPRESS);
	_CHECK(PARA_CHECK_NOHYPHEN,	id_CHECK_NO_HYPHENATE);
#ifdef BIDI_ENABLED
	_CHECK(PARA_CHECK_BIDI,		id_CHECK_DOMDIRECTION);
#endif

	case IDCANCEL:						// also AP_RID_DIALOG_PARA_BTN_CANCEL
		m_answer = a_CANCEL;
		EndDialog(hWnd,0);
		return 1;

	case IDOK:							// also AP_RID_DIALOG_PARA_BTN_OK
		m_answer = a_OK;
		EndDialog(hWnd,0);
		return 1;

	case AP_RID_DIALOG_PARA_BTN_TABS:
		m_answer = a_TABS;
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

/*****************************************************************/

#define _SPIN(w,c,i)					\
	case AP_RID_DIALOG_PARA_SPIN_##c:	\
		if (m_bEditChanged)				\
		{								\
			GetDlgItemText(w,AP_RID_DIALOG_PARA_EDIT_##c,buf,SPIN_BUF_TEXT_SIZE);	\
			_setSpinItemValue(i,buf);	\
			m_bEditChanged = false;	\
		}								\
		_doSpin(i, (0 - (UT_sint32) pnmud->iDelta));	\
		break;							\

BOOL AP_Win32Dialog_Paragraph::_onDeltaPos(NM_UPDOWN * pnmud)
{
	// respond to WM_NOTIFY/UDN_DELTAPOS message
	// return TRUE to prevent the change from happening
	// return FALSE to allow it to occur
	// we may alter the change by changing the fields in pnmud.

	UT_DEBUGMSG(("onDeltaPos: [idFrom %d][iPos %d][iDelta %d]\n",
				 pnmud->hdr.idFrom,pnmud->iPos,pnmud->iDelta));
				
	char buf[SPIN_BUF_TEXT_SIZE];

	switch(pnmud->hdr.idFrom)
	{
	_SPIN(m_hwndSpacing, LEFT,		id_SPIN_LEFT_INDENT);
	_SPIN(m_hwndSpacing, RIGHT,		id_SPIN_RIGHT_INDENT);
	_SPIN(m_hwndSpacing, BY,		id_SPIN_SPECIAL_INDENT);
	_SPIN(m_hwndSpacing, BEFORE,	id_SPIN_BEFORE_SPACING);
	_SPIN(m_hwndSpacing, AFTER,		id_SPIN_AFTER_SPACING);
	_SPIN(m_hwndSpacing, AT,		id_SPIN_SPECIAL_SPACING);

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return FALSE;
}

/*****************************************************************/

#define _syncSPIN(w,c,i)	\
		case i:				\
			SetDlgItemText(w,AP_RID_DIALOG_##c,_getSpinItemValue(i));	\
			break;			\

void AP_Win32Dialog_Paragraph::_syncControls(tControl changed, bool bAll /* = false */)
{ 
	// let parent sync any member variables first

	AP_Dialog_Paragraph::_syncControls(changed, bAll);

	// sync the display

	// 1.  link the "hanging indent by" combo and spinner

	if (bAll || (changed == id_SPIN_SPECIAL_INDENT))
	{
		// typing in the control can change the associated combo
		if (_getMenuItemValue(id_MENU_SPECIAL_INDENT) == indent_FIRSTLINE)
		{
			HWND h = GetDlgItem(m_hwndSpacing, AP_RID_DIALOG_PARA_COMBO_HANG);							
			SendMessage(h, CB_SETCURSEL, (WPARAM) _getMenuItemValue(id_MENU_SPECIAL_INDENT), 0);
		}
	}
	if (bAll || (changed == id_MENU_SPECIAL_INDENT))
	{
		switch(_getMenuItemValue(id_MENU_SPECIAL_INDENT))
		{
		case indent_NONE:
			// clear the spin control
			SetDlgItemText(m_hwndSpacing, AP_RID_DIALOG_PARA_EDIT_BY, NULL);
			break;

		default:
			// set the spin control
			SetDlgItemText(m_hwndSpacing, AP_RID_DIALOG_PARA_EDIT_BY, _getSpinItemValue(id_SPIN_SPECIAL_INDENT));
			break;
		}
	}

	// 2.  link the "line spacing at" combo and spinner

	if (bAll || (changed == id_SPIN_SPECIAL_SPACING))
	{
		// typing in the control can change the associated combo
		if (_getMenuItemValue(id_MENU_SPECIAL_SPACING) == spacing_MULTIPLE)
		{
			HWND h = GetDlgItem(m_hwndSpacing, AP_RID_DIALOG_PARA_COMBO_LEAD);							
			SendMessage(h, CB_SETCURSEL, (WPARAM) _getMenuItemValue(id_MENU_SPECIAL_SPACING), 0);
		}
	}
	if (bAll || (changed == id_MENU_SPECIAL_SPACING))
	{
		switch(_getMenuItemValue(id_MENU_SPECIAL_SPACING))
		{
		case spacing_SINGLE:
		case spacing_ONEANDHALF:
		case spacing_DOUBLE:
			// clear the spin control
			SetDlgItemText(m_hwndSpacing, AP_RID_DIALOG_PARA_EDIT_AT, NULL);
			break;

		default:
			// set the spin control
			SetDlgItemText(m_hwndSpacing, AP_RID_DIALOG_PARA_EDIT_AT, _getSpinItemValue(id_SPIN_SPECIAL_SPACING));
			break;
		}
	}

	// 3.  move results of _doSpin() back to screen

	if (!bAll)
	{
		// spin controls only sync when spun
		switch (changed)
		{
		_syncSPIN(m_hwndSpacing, PARA_EDIT_LEFT,	id_SPIN_LEFT_INDENT)
		_syncSPIN(m_hwndSpacing, PARA_EDIT_RIGHT,	id_SPIN_RIGHT_INDENT)
		_syncSPIN(m_hwndSpacing, PARA_EDIT_BY,		id_SPIN_SPECIAL_INDENT)
		_syncSPIN(m_hwndSpacing, PARA_EDIT_BEFORE,	id_SPIN_BEFORE_SPACING)
		_syncSPIN(m_hwndSpacing, PARA_EDIT_AFTER,	id_SPIN_AFTER_SPACING)
		_syncSPIN(m_hwndSpacing, PARA_EDIT_AT,		id_SPIN_SPECIAL_SPACING)
		default:
			break;
		}
	}
}
