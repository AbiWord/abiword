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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_timer.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"
#include "fv_View.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Lists.h"
#include "ap_Win32Dialog_Lists.h"

#include "ap_Win32Resources.rc2"
#include "xap_Win32Toolbar_Icons.h"
#include "xap_Win32Dlg_FontChooser.h"

#ifdef _MSC_VER
// Microsoft still haven't got their act together to follow the
// international ISO standard regarding scoping of for loop variables.
// <sarcasm>
// Perhaps it's a bit much to ask for, the rules are only about five
// years old as of this writing, and the standard is only two years old.
// </sarcasm>
// This workaround works everywhere.
#define for if (0) {} else for
// MSVC++ warns about using 'this' in initializer list.
#pragma warning(disable: 4355)
#endif

/*****************************************************************/

XAP_Dialog*
AP_Win32Dialog_Lists::static_constructor(	XAP_DialogFactory* pFactory,
											XAP_Dialog_Id id)
{
	AP_Win32Dialog_Lists* p = new AP_Win32Dialog_Lists(pFactory,id);
	return p;
}

AP_Win32Dialog_Lists::AP_Win32Dialog_Lists(	XAP_DialogFactory* pDlgFactory,
											XAP_Dialog_Id id)
:	AP_Dialog_Lists(pDlgFactory,id),
	m_pAutoUpdateLists(0),
	_win32Dialog(this),
	m_pPreviewWidget(0),
	m_bDisplayCustomControls(UT_FALSE),
	m_hThisDlg(0)
{
	// Manually set this for now...
	m_newListType = m_iListType = NUMBERED_LIST;
	m_curStartValue = 1;
	m_iStartValue = 1;
	m_newStartValue = 1;
}

AP_Win32Dialog_Lists::~AP_Win32Dialog_Lists(void)
{
	if (m_pAutoUpdateLists)
	{
		m_pAutoUpdateLists->stop();
		DELETEP(m_pAutoUpdateLists);
	}
}

/*****************************************************************/

void AP_Win32Dialog_Lists::runModeless(XAP_Frame * pFrame)
{
	// Always revert to non-customized state when bringing it up.
	m_bDisplayCustomControls = UT_FALSE;
	m_bisCustomized = _isNewList();
	_customChanged();
	_setData();

	// raise the dialog
	_win32Dialog.runModeless(pFrame, AP_DIALOG_ID_LISTS, AP_RID_DIALOG_LIST, this);
}


BOOL AP_Win32Dialog_Lists::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_hThisDlg = hWnd;

	// Default range for a spin control is 100 -- 0 (i.e. min > max),
	// making it go "the wrong way" (i.e. cursor up lowers the value).
	// It needs an UDM_SETRANGE to go the right way.
	static const int rgSpinIDs[] =
	{
		AP_RID_DIALOG_LIST_SPIN_LEVEL,			// UDS_SETBUDDYINT control
		AP_RID_DIALOG_LIST_SPIN_START_AT,		// UDS_SETBUDDYINT control
		AP_RID_DIALOG_LIST_SPIN_LIST_ALIGN,		// non-UDS_SETBUDDYINT
		AP_RID_DIALOG_LIST_SPIN_INDENT_ALIGN	// non-UDS_SETBUDDYINT
	};

	for (int i = 0; i < NrElements(rgSpinIDs); ++i)
	{
		HWND hWndSpin = GetDlgItem(hWnd, rgSpinIDs[i]);
		if (hWndSpin)
		{
			// TODO: Is this range reasonable?
			const WORD   nLower = 0;
			const WORD   nUpper = 100;
			const LPARAM range  = MAKELPARAM(nUpper, nLower);
			SendMessage(hWndSpin, UDM_SETRANGE, 0L, range);
		}
	}

	PopulateDialogData();

	_updateCaption();

	m_newListType = m_iListType = NUMBERED_LIST;

	const XAP_StringSet* pSS = m_pApp->getStringSet();

	UT_ASSERT(pSS);	// TODO: Would an error handler be more appropriate here?

	// Set the locale specific strings
	struct control_id_string_id {
		UT_sint32		controlId;
		XAP_String_Id	stringId;
	} static const rgMapping[] =
	{
		AP_RID_DIALOG_LIST_STATIC_TYPE			, AP_STRING_ID_DLG_Lists_Type,
		AP_RID_DIALOG_LIST_STATIC_STYLE			, AP_STRING_ID_DLG_Lists_Style,
// "Customize" button is special, so it's handled in _enableControls()
//		AP_RID_DIALOG_LIST_BUTTON_CUSTOMIZE		, AP_STRING_ID_DLG_Lists_Customize,
		AP_RID_DIALOG_LIST_STATIC_FORMAT		, AP_STRING_ID_DLG_Lists_Format,
		AP_RID_DIALOG_LIST_STATIC_FONT			, AP_STRING_ID_DLG_Lists_Font,
		AP_RID_DIALOG_LIST_STATIC_LEVEL			, AP_STRING_ID_DLG_Lists_Level,
		AP_RID_DIALOG_LIST_STATIC_START_AT		, AP_STRING_ID_DLG_Lists_Start,
		AP_RID_DIALOG_LIST_STATIC_LIST_ALIGN	, AP_STRING_ID_DLG_Lists_Align,
		AP_RID_DIALOG_LIST_STATIC_INDENT_ALIGN	, AP_STRING_ID_DLG_Lists_Indent,
		AP_RID_DIALOG_LIST_STATIC_PREVIEW		, AP_STRING_ID_DLG_Lists_Preview,
		AP_RID_DIALOG_LIST_BTN_APPLY			, XAP_STRING_ID_DLG_Apply,
		AP_RID_DIALOG_LIST_BTN_CLOSE			, XAP_STRING_ID_DLG_Close,
		AP_RID_DIALOG_LIST_RADIO_START_NEW_LIST	, AP_STRING_ID_DLG_Lists_Start_New,
		AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST, AP_STRING_ID_DLG_Lists_Apply_Current,
		AP_RID_DIALOG_LIST_RADIO_RESUME_PREV_LIST, AP_STRING_ID_DLG_Lists_Resume
	};

	for (int i = 0; i < NrElements(rgMapping); ++i)
	{
		_win32Dialog.setControlText(rgMapping[i].controlId,
									pSS->getValue(rgMapping[i].stringId));
	}

	_fillTypeList();
	_setListType(m_iListType);

	// Create a preview window.

	HWND hwndChild = GetDlgItem(hWnd, AP_RID_DIALOG_LIST_FRAME_PREVIEW);

	// If we don't get the preview control things are bad!
	UT_ASSERT(hwndChild);

	m_pPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),
													  hwndChild,
													  0);
	UT_uint32 w,h;
	m_pPreviewWidget->getWindowSize(&w,&h);
	_createPreviewFromGC(m_pPreviewWidget->getGraphics(), w, h);
	m_pPreviewWidget->setPreview(m_pListsPreview);

	_enableControls();
	_customChanged();
	_setData();

	m_pAutoUpdateLists = UT_Timer::static_constructor(autoupdateLists, this);
	m_pAutoUpdateLists->set(500);	// auto-updater at 1/2 Hz

	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_Lists::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const WORD wNotifyCode	= HIWORD(wParam);
	const WORD wId			= LOWORD(wParam);
	const HWND hWndCtrl		= (HWND)lParam;

	switch (wId)
	{
	case AP_RID_DIALOG_LIST_RADIO_START_NEW_LIST:
		_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST, FALSE);
		m_bisCustomized = UT_TRUE;
		m_newListType = m_iListType = NUMBERED_LIST;
		m_curStartValue = 1;
		m_iStartValue  = 1;
		_setListType(m_iListType);
		_enableControls();
		return 1;

	case AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST:
		_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_START_NEW_LIST, FALSE);
		m_bisCustomized = UT_FALSE;
		_enableControls();
		return 1;

	case IDCANCEL:	// also AP_RID_DIALOG_LIST_BTN_CLOSE
		_win32Dialog.showWindow(SW_HIDE);
		return 1;

	case AP_RID_DIALOG_LIST_BTN_APPLY:
		m_bisCustomized = UT_TRUE;
		_onApply();
		return 1;

	case AP_RID_DIALOG_LIST_BUTTON_CUSTOMIZE:
		m_bDisplayCustomControls = !m_bDisplayCustomControls;
		_customChanged();
		return 1;

	case AP_RID_DIALOG_LIST_COMBO_TYPE:
		if (wNotifyCode == LBN_SELCHANGE)
		{
			m_bisCustomized = UT_TRUE;
			_typeChanged();
			return 1;
		}
		return 0;

	case AP_RID_DIALOG_LIST_COMBO_STYLE:
		if (wNotifyCode == LBN_SELCHANGE)
		{
			m_bisCustomized = UT_TRUE;
			_styleChanged();
			return 1;
		}
		return 0;

	case AP_RID_DIALOG_LIST_BTN_FONT:
		_selectFont();
		return 1;						// return zero to let windows take care of it.

//	case AP_RID_DIALOG_LIST_EDIT_LIST_ALIGN:
//	case AP_RID_DIALOG_LIST_EDIT_INDENT_ALIGN:
	case AP_RID_DIALOG_LIST_EDIT_START_AT:
	case AP_RID_DIALOG_LIST_EDIT_LEVEL:
		if (wNotifyCode == EN_CHANGE)
		{
			_getData();
			_previewExposed();
			return 1;
		}
		return 0;

	default:	// we did not handle this notification
		if (_win32Dialog.isControlVisible(wId))
		{
			UT_DEBUGMSG(("AP_Win32Dialog_Lists::_onCommand, WM_Command for visible control ID %ld\n",wId));
		} else {
			UT_DEBUGMSG(("AP_Win32Dialog_Lists::_onCommand, WM_Command for invisible control ID %ld\n",wId));
		}
		return 0;	// return zero to let windows take care of it.
	}
}

BOOL AP_Win32Dialog_Lists::_onDeltaPos(NM_UPDOWN* pnmud)
{
	// This function gets called whenever the user tries to change the
	// value of a control with a connected (UDS_AUTOBUDDY) spin (up/down)
	// control. Since we have a few without the UDS_SETBUDDYINT style
	// we take care of that here.

	if (!pnmud)
	{
		// Should be impossible, but better safe than sorry.
		return 0;
	}

	UT_DEBUGMSG(("AP_Win32Dialog_Lists::_onDeltaPos()\n"));

	UT_sint32 controlID;

	switch (pnmud->hdr.idFrom)
	{
		case AP_RID_DIALOG_LIST_SPIN_LIST_ALIGN:
			controlID = AP_RID_DIALOG_LIST_EDIT_LIST_ALIGN;
			break;

		case AP_RID_DIALOG_LIST_SPIN_INDENT_ALIGN:
			controlID = AP_RID_DIALOG_LIST_EDIT_INDENT_ALIGN;
			break;
		default:
			return 0;
	}

	char szTmp[32];
	const UT_sint32 len = (UT_sint32)sizeof(szTmp);

	_win32Dialog.getControlText(controlID, szTmp, len);
	float fVal = (float)UT_convertDimensionless(szTmp) + 0.10f * pnmud->iDelta;
	_win32Dialog.setControlText(controlID,
								UT_convertToDimensionlessString(fVal, ".2"));

	m_bisCustomized = UT_TRUE;
	_getData();
	_previewExposed();

	return 0;
}

void AP_Win32Dialog_Lists::destroy(void)
{
	_win32Dialog.destroyWindow();
}

void AP_Win32Dialog_Lists::activate(void)
{
	int iResult;

	_updateCaption();

	iResult = _win32Dialog.showWindow( SW_SHOW );

	iResult = _win32Dialog.bringWindowToTop();

	UT_ASSERT((iResult != 0));
}

void AP_Win32Dialog_Lists::notifyActiveFrame(XAP_Frame *pFrame)
{
	XAP_Win32Frame* pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

	if (_win32Dialog.isParentFrame(*pWin32Frame))
	{
		return;
	}

	// change parent
	_win32Dialog.setParentFrame(pWin32Frame);
	_win32Dialog.bringWindowToTop();
	_updateCaption();

	if (!m_bisCustomized && !m_bDisplayCustomControls)
	{
		PopulateDialogData();
	}

	_setData();
	_previewExposed();
}

void AP_Win32Dialog_Lists::notifyCloseFrame(XAP_Frame *pFrame)
{
	XAP_Win32Frame* pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);
	if (_win32Dialog.isParentFrame(*pWin32Frame))
	{
		m_bisCustomized = UT_FALSE;
		setActiveFrame(getActiveFrame());
	}
	_updateCaption();
}

void AP_Win32Dialog_Lists::_enableControls(void)
{
	UT_Bool bStartNew = _isNewList();
	UT_Bool bAppend   = _win32Dialog.isChecked(AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST);
	UT_Bool bStopCurr = _win32Dialog.isChecked(AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST);
	const int iType = _getTypeComboCurSel();

	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_RADIO_START_NEW_LIST, !bStopCurr);

	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_COMBO_TYPE, bStartNew);
	if (!bStartNew || !iType)
	{
		_win32Dialog.enableControl(AP_RID_DIALOG_LIST_COMBO_STYLE, UT_FALSE);
	}

//	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_EDIT_START_AT, bStartNew);

	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_EDIT_START_AT, m_isListAtPoint && !bStartNew && !bStopCurr);

	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST, m_isListAtPoint && !bStartNew);
	_win32Dialog.showControl(AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST,
								m_isListAtPoint ? SW_SHOW : SW_HIDE);

	// Apply button should only be enabled when we have a list type selected
	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_BTN_APPLY,
								iType ? UT_TRUE : UT_FALSE);

	_enableCustomControls(iType && bStartNew);

	// Font button should only be enabled when we have Numbered list.
	// This call must come after _enableCustomControls() since it's
	// part of the "Custom Controls" group.
	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_BTN_FONT, iType == 2);

	// Set the correct text for the "Customize" button.
	{
		const XAP_StringSet * pSS = m_pApp->getStringSet();
		UT_ASSERT(pSS);	// TODO: Would an error handler be more appropriate here?

		// TMN: I don't want to hard-code a size here. Besides, this function
		// should not be called frequently enough to be noticable
		// in a profiling session. If this allocation do show up as noticable,
		// something else is wrong.
		//
		// TMN: HACK HACK HACK !!!
		//
		// This code should probably be able to handle both Unicode
		// and 8-bit character sets. Currently, it only uses 8-bit charset
		// until some issues have been ironed out. If you see this code and
		// know how to use conditional DBCS or even unconditional DBCS
		// from the given data, please fix it.
		//
		// TMN: HACK HACK HACK !!!
		const XML_Char* pszCustomize =
			pSS->getValue(AP_STRING_ID_DLG_Lists_Customize);
		char* pszTmp = new char[strlen(pszCustomize) + 4];
		strcpy(pszTmp, pszCustomize);
		strcat(pszTmp, m_bDisplayCustomControls ? " <<" : " >>");
		_win32Dialog.setControlText(AP_RID_DIALOG_LIST_BUTTON_CUSTOMIZE,
									pszTmp);
	}
}

void AP_Win32Dialog_Lists::_onApply()
{
	m_bStartList = _isNewList();
	m_bStopList  = _win32Dialog.isChecked(AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST);
	m_bChangeStartValue = UT_FALSE;

	if(m_bStartList)
	{
		m_iListType = _getListType();
		m_newListType = m_iListType;

		m_bStartNewList   = UT_TRUE;
		m_bApplyToCurrent = UT_FALSE;
		m_bStartSubList   = UT_FALSE;

		_getData();

		// "Apply" button should be disabled while we have
		// List Type "None" selected.
		UT_ASSERT(!IS_NONE_LIST_TYPE(m_iListType));

		StartList();
	}
	else
	{

		if(m_bStopList)
		{
			StopList();
		}
		else
		{
			UT_uint32 newStartValue = (UT_uint32)_win32Dialog.getControlInt(AP_RID_DIALOG_LIST_EDIT_START_AT);
			UT_uint32 newListType = (UT_uint32)_getTypeComboCurSel();

			if (newStartValue != m_curStartValue &&
				newListType != (UT_uint32) m_iListType)
			{
				m_bChangeStartValue = UT_TRUE;
				m_curStartValue = newStartValue;
				m_newListType = (List_Type) newListType;
			}
		}
	}

	Apply();

	_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_START_NEW_LIST, FALSE);
	_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST, FALSE);
	_enableControls();
}


//
// current selection of the drop-list combo boxes
//

int AP_Win32Dialog_Lists::_getTypeComboCurSel() const
{
	return _win32Dialog.getComboSelectedIndex(AP_RID_DIALOG_LIST_COMBO_TYPE);
}

int AP_Win32Dialog_Lists::_getStyleComboCurSel() const
{
	return _win32Dialog.getComboSelectedIndex(AP_RID_DIALOG_LIST_COMBO_STYLE);
}

void AP_Win32Dialog_Lists::_setTypeComboCurSel(int iSel)
{
	_win32Dialog.selectComboItem(AP_RID_DIALOG_LIST_COMBO_TYPE, iSel);
}

void AP_Win32Dialog_Lists::_setStyleComboCurSel(int iSel)
{
	_win32Dialog.selectComboItem(AP_RID_DIALOG_LIST_COMBO_STYLE, iSel);
}


UT_Bool AP_Win32Dialog_Lists::_isNewList() const
{
	return _win32Dialog.isChecked(AP_RID_DIALOG_LIST_RADIO_START_NEW_LIST);
}

void AP_Win32Dialog_Lists::_fillTypeList()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	UT_ASSERT(pSS);	// TODO: Would an error handler be more appropriate here?

	static const XAP_String_Id rgIDs[] =
	{
		AP_STRING_ID_DLG_Lists_Type_none,
		AP_STRING_ID_DLG_Lists_Type_bullet,
		AP_STRING_ID_DLG_Lists_Type_numbered
	};

	for (int i = 0; i < NrElements(rgIDs); ++i)
	{
		_win32Dialog.addItemToCombo(AP_RID_DIALOG_LIST_COMBO_TYPE,
									pSS->getValue(rgIDs[i]));
	}
}

void AP_Win32Dialog_Lists::_fillStyleList(int iType)
{
	UT_ASSERT(iType >= -1 && iType <=2);

	const UT_sint32 idComboStyle = AP_RID_DIALOG_LIST_COMBO_STYLE;

	_win32Dialog.resetComboContent(idComboStyle);

	if (iType == -1 /* no combo selection */ ||
		iType == 0 /* List Type "None" selected */)
	{
		return;
	}

	// Dare Fight Bloat. Put the data in a read-only data segment.
	static const XAP_String_Id rgBulletIDs[] =
	{
		AP_STRING_ID_DLG_Lists_Bullet_List,
		AP_STRING_ID_DLG_Lists_Dashed_List,
		AP_STRING_ID_DLG_Lists_Square_List,
		AP_STRING_ID_DLG_Lists_Triangle_List,
		AP_STRING_ID_DLG_Lists_Diamond_List,
		AP_STRING_ID_DLG_Lists_Star_List,
		AP_STRING_ID_DLG_Lists_Implies_List,
		AP_STRING_ID_DLG_Lists_Tick_List,
		AP_STRING_ID_DLG_Lists_Box_List,
		AP_STRING_ID_DLG_Lists_Hand_List,
		AP_STRING_ID_DLG_Lists_Heart_List
	};

	static const XAP_String_Id rgNumberedIDs[] =
	{
		AP_STRING_ID_DLG_Lists_Numbered_List,
		AP_STRING_ID_DLG_Lists_Lower_Case_List,
		AP_STRING_ID_DLG_Lists_Upper_Case_List,
		AP_STRING_ID_DLG_Lists_Lower_Roman_List,
		AP_STRING_ID_DLG_Lists_Upper_Roman_List
	};

	const XAP_String_Id*	pIDs;
	size_t					nIDs;

	switch (iType)
	{
		case 1:	// bullet list
			pIDs = rgBulletIDs;
			nIDs = NrElements(rgBulletIDs);
			break;
		case 2:	// numbered list
			pIDs = rgNumberedIDs;
			nIDs = NrElements(rgNumberedIDs);
			break;
	}

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_ASSERT(pSS);	// TODO: Would an error handler be more appropriate here?

	for (size_t i = 0; i < nIDs; ++i)
	{
		_win32Dialog.addItemToCombo(idComboStyle, pSS->getValue(pIDs[i]));
	}
}

void AP_Win32Dialog_Lists::_typeChanged()
{
	const int iType = _getTypeComboCurSel();

	const List_Type type =
		iType == 1 ? BULLETED_LIST :
		iType == 2 ? NUMBERED_LIST :
		NOT_A_LIST;

	_setListType(type);
	_enableControls();
	_styleChanged();
}

void AP_Win32Dialog_Lists::_styleChanged()
{
	if (m_bisCustomized)
	{
		m_iListType = _getListType();
	}
	else
	{
		m_bguiChanged = UT_TRUE;
	}
	m_newListType = _getListType();

	_previewExposed();
}


//
// array of control IDs to show/hide or enable/disable
//
static const UT_sint32 rgCustomIds[] =
{
	AP_RID_DIALOG_LIST_GROUP_CUSTOM,
	AP_RID_DIALOG_LIST_STATIC_START_AT,
	AP_RID_DIALOG_LIST_EDIT_START_AT,
	AP_RID_DIALOG_LIST_SPIN_START_AT,
	AP_RID_DIALOG_LIST_STATIC_LIST_ALIGN,
	AP_RID_DIALOG_LIST_EDIT_LIST_ALIGN,
	AP_RID_DIALOG_LIST_SPIN_LIST_ALIGN,
	AP_RID_DIALOG_LIST_STATIC_INDENT_ALIGN,
	AP_RID_DIALOG_LIST_EDIT_INDENT_ALIGN,
	AP_RID_DIALOG_LIST_SPIN_INDENT_ALIGN,
	AP_RID_DIALOG_LIST_STATIC_FORMAT,
	AP_RID_DIALOG_LIST_EDIT_FORMAT,
	AP_RID_DIALOG_LIST_STATIC_FONT,
	AP_RID_DIALOG_LIST_STATIC_LEVEL,
	AP_RID_DIALOG_LIST_EDIT_LEVEL,
	AP_RID_DIALOG_LIST_SPIN_LEVEL,
	AP_RID_DIALOG_LIST_BTN_FONT
};

void AP_Win32Dialog_Lists::_customChanged()
{
	const int mode = m_bDisplayCustomControls ? SW_SHOW : SW_HIDE;
	for (int i = 0; i < NrElements(rgCustomIds); ++i)
	{
		_win32Dialog.showControl(rgCustomIds[i], mode);
	}

#if 0
	// TODO: Make the preview work as expected in this case
	if (!m_bisCustomized && !m_bDisplayCustomControls)
	{
		PopulateDialogData();
		_setData();
	}
#endif

	_previewExposed();
}

void AP_Win32Dialog_Lists::_enableCustomControls(UT_Bool bEnable)
{
	for (int i = 0; i < NrElements(rgCustomIds); ++i)
	{
		_win32Dialog.enableControl(rgCustomIds[i], bEnable);
	}
}


void AP_Win32Dialog_Lists::_updateCaption()
{
	ConstructWindowName();
	_win32Dialog.setDialogTitle(m_WindowName);
}

void AP_Win32Dialog_Lists::_previewExposed()
{
	if (!m_pPreviewWidget)
	{
		return;
	}

//	if(m_bDoExpose == UT_TRUE)
	{
		event_PreviewAreaExposed();
	}
//	m_bDoExpose = UT_TRUE;
}

//
// Change display to view dialog internal data
//
void AP_Win32Dialog_Lists::_setData()
{
	_win32Dialog.setControlInt(AP_RID_DIALOG_LIST_EDIT_START_AT,
								m_curStartValue);

	_win32Dialog.setControlText(AP_RID_DIALOG_LIST_EDIT_LIST_ALIGN	,
								UT_convertToDimensionlessString(m_fAlign, ".2"));

	_win32Dialog.setControlText(AP_RID_DIALOG_LIST_EDIT_INDENT_ALIGN,
								UT_convertToDimensionlessString(m_fIndent, ".2"));

	_win32Dialog.setControlText(AP_RID_DIALOG_LIST_EDIT_FORMAT	, m_pszDelim);
	_win32Dialog.setControlInt(AP_RID_DIALOG_LIST_EDIT_LEVEL	, m_iLevel);

	_setListType(_isNewList() ? m_iListType : m_newListType);

	// Now it's time to (possibly) change the text of two of the three
	// radio buttons.

	struct control_id_string_id {
		UT_sint32		controlId;
		XAP_String_Id	stringId;
	};

	static const control_id_string_id rgMappingActiveList[] =
	{
		AP_RID_DIALOG_LIST_RADIO_START_NEW_LIST  , AP_STRING_ID_DLG_Lists_Stop_Current_List,
		AP_RID_DIALOG_LIST_RADIO_RESUME_PREV_LIST, AP_STRING_ID_DLG_Lists_Start_Sub
	};
	static const control_id_string_id rgMappingNoList[] =
	{
		AP_RID_DIALOG_LIST_RADIO_START_NEW_LIST  , AP_STRING_ID_DLG_Lists_Start_New,
		AP_RID_DIALOG_LIST_RADIO_RESUME_PREV_LIST, AP_STRING_ID_DLG_Lists_Resume
	};

	const control_id_string_id* rgMapping =
		m_isListAtPoint ? rgMappingActiveList : rgMappingNoList;

	const XAP_StringSet* pSS = m_pApp->getStringSet();

	UT_ASSERT(pSS);	// TODO: Would an error handler be more appropriate here?

	for (int i = 0; i < 2; ++i)
	{
		_win32Dialog.setControlText(rgMapping[i].controlId,
									pSS->getValue(rgMapping[i].stringId));
	}

	// Resume/Start Sub should only be enabled if the cursor is at/in a list
	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_RADIO_RESUME_PREV_LIST,
								m_isListAtPoint);
}

//
// Set dialog internal data from displayed values
//
void AP_Win32Dialog_Lists::_getData()
{
	char szTmp[32];
	
	_win32Dialog.getControlText(AP_RID_DIALOG_LIST_EDIT_LIST_ALIGN	, szTmp, 30);
	m_fAlign = (float)UT_convertDimensionless(szTmp);

	_win32Dialog.getControlText(AP_RID_DIALOG_LIST_EDIT_INDENT_ALIGN, szTmp, 30);
	m_fIndent = (float)UT_convertDimensionless(szTmp);

	m_curStartValue = _win32Dialog.getControlInt(AP_RID_DIALOG_LIST_EDIT_START_AT);
	m_newListLevel = _win32Dialog.getControlInt(AP_RID_DIALOG_LIST_EDIT_LEVEL);

	if (m_bisCustomized)
	{
		m_iStartValue = m_curStartValue;
	}

	_win32Dialog.getControlText(AP_RID_DIALOG_LIST_EDIT_FORMAT, m_pszDelim, sizeof(m_pszDelim));
}

//
// Returns the List_Type of the currently selected Type/Style combination.
// Returns NOT_A_LIST on failure.
//
List_Type AP_Win32Dialog_Lists::_getListType() const
{
	const int iType  = _getTypeComboCurSel();
	const int iStyle = _getStyleComboCurSel();

	if (iType == 0	/* List Type "None" selected	*/ ||
		iType < 0	/* no selection at all ???		*/ ||
		iType > 2	/* Illegal value!				*/)
	{
		return NOT_A_LIST;
	}

	if (iType == 1 /* bullet list */)
	{
		return (List_Type)(iStyle + BULLETED_LIST);
	}

	// Numbered List, but just to make really REALLY sure, we assert it
	UT_ASSERT(IS_NUMBERED_LIST_TYPE(iStyle));

	return (List_Type)iStyle;
}

void AP_Win32Dialog_Lists::_setListType(List_Type type)
{
	int iType;
	int iStyle;

	if (IS_NONE_LIST_TYPE(type))
	{
		iType = 0;
		iStyle = -1;
	}
	else if (IS_NUMBERED_LIST_TYPE(type))
	{
		iType = 2;
		iStyle = type;
	}
	else
	{
		UT_ASSERT(IS_BULLETED_LIST_TYPE(type));
		iType = 1;
		iStyle = type - BULLETED_LIST;
	}

	_setTypeComboCurSel(iType);
	_fillStyleList(iType);
	_setStyleComboCurSel(iStyle);
	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_COMBO_STYLE,
								iType ? UT_TRUE : UT_FALSE);

	if (m_bisCustomized)
	{
		m_iListType = type;
	}
	else
	{
		m_bguiChanged = UT_TRUE;
	}
	m_newListType = type;
}


void AP_Win32Dialog_Lists::_selectFont()
{
	if (_getTypeComboCurSel() != 2)
	{
		// What the...? The Font select button should only be active
		// when Numbered list type is selected!
	}

	XAP_Frame* pFrame = getActiveFrame();
	FV_View* pView = getView();
	if (!pFrame || !pView)
	{
		// take the easy way out for now
		MessageBeep(MB_ICONASTERISK);
//		_win32Dialog.enableControl(AP_RID_DIALOG_LIST_BTN_FONT, UT_FALSE);
		return;
	}

	XAP_DialogFactory* pDialogFactory = pFrame->getDialogFactory();
	UT_ASSERT(pDialogFactory);

	XAP_Dialog_FontChooser* pDialog
		= (XAP_Dialog_FontChooser *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_FONT));
	UT_ASSERT(pDialog);

	if (!pDialog)	// runtime failsafe
	{
		// Disable the Font button. If we couldn't get the dialog
		// this time, chances are we never will. :-(
		_win32Dialog.enableControl(AP_RID_DIALOG_LIST_BTN_FONT, UT_FALSE);
		MessageBeep(MB_ICONASTERISK);
		return;
	}

	const XML_Char** props_in = NULL;
	if (pView->getCharFormat(&props_in))
	{
		// stuff font properties into the dialog.

		pDialog->setFontFamily(m_pszFont);
		pDialog->setFontSize(UT_getAttribute("font-size", props_in));
		pDialog->setFontWeight(UT_getAttribute("font-weight", props_in));
		pDialog->setFontStyle(UT_getAttribute("font-style", props_in));
		pDialog->setColor(UT_getAttribute("color", props_in));

		// these behave a little differently since they are
		// probably just check boxes and we don't have to
		// worry about initializing a combo box with a choice
		// (and because they are all stuck under one CSS attribute).

		UT_Bool bUnderline = UT_FALSE;
		UT_Bool bOverline = UT_FALSE;
		UT_Bool bStrikeOut = UT_FALSE;
		const XML_Char * s = UT_getAttribute("text-decoration", props_in);
		if (s)
		{
			bUnderline = (strstr(s, "underline") != NULL);
			bOverline = (strstr(s, "overline") != NULL);
			bStrikeOut = (strstr(s, "line-through") != NULL);
		}
		pDialog->setFontDecoration(bUnderline,bOverline,bStrikeOut);

		free(props_in);
	}

	pDialog->setGraphicsContext(pView->getGraphics());
	pDialog->runModal(pFrame);

	if (pDialog->getAnswer() != XAP_Dialog_FontChooser::a_OK && 
		pDialog->getAnswer() != XAP_Dialog_FontChooser::a_YES)
	{
		return;
	}

	const XML_Char* pszFont;

	if (pDialog->getChangedFontFamily(&pszFont))
	{
		UT_XML_strncpy(m_pszFont, sizeof(m_pszFont), pszFont);
		_previewExposed();
	}
}


void AP_Win32Dialog_Lists::autoupdateLists(UT_Timer * pTimer)
{
	UT_ASSERT(pTimer);
	// this is a static callback method and does not have a 'this' pointer.
	AP_Win32Dialog_Lists* pDialog =
		reinterpret_cast<AP_Win32Dialog_Lists*>(pTimer->getInstanceData());

	UT_ASSERT(pDialog);

	if (pDialog)	// runtime failsafe
	{
		pDialog->_enableControls();
		pDialog->_previewExposed();
	}
}

const XML_Char* AP_Win32Dialog_Lists::_getDingbatsFontName() const
{
	return "Wingdings";
}

