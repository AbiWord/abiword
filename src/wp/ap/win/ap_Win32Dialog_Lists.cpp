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

#include <windows.h>

#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_timer.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "fv_View.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Lists.h"
#include "ap_Win32Dialog_Lists.h"

#include "gr_Win32Graphics.h"

#include "ap_Win32Resources.rc2"
#include "xap_Win32Toolbar_Icons.h"
#include "xap_Win32Dlg_FontChooser.h"
#include "xap_Win32DialogHelper.h"
#include "ap_Win32App.h"

#ifdef _MSC_VER
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
	m_bDestroy_says_stopupdating(false),
	m_bAutoUpdate_happening_now(false),
	m_pAutoUpdateLists(0),
	_win32Dialog(this),
	m_pPreviewWidget(0),
	m_bEnableCustomControls(true),
	m_hThisDlg(0)
{
}

AP_Win32Dialog_Lists::~AP_Win32Dialog_Lists(void)
{
	if (m_pAutoUpdateLists)
	{
		m_pAutoUpdateLists->stop();
		DELETEP(m_pAutoUpdateLists);
	}
	DELETEP(m_pPreviewWidget);
}

/*****************************************************************/

void AP_Win32Dialog_Lists::runModal(XAP_Frame * pFrame)
{
	clearDirty();
	setModal();
	_win32Dialog.runModal(pFrame, AP_DIALOG_ID_LISTS, AP_RID_DIALOG_LIST, this);
}

void AP_Win32Dialog_Lists::runModeless(XAP_Frame * pFrame)
{
	// Always revert to non-customized state when bringing it up.
	clearDirty();
	if ( m_hThisDlg )
	{
		setbisCustomized(false);
		_resetCustomValues();
		_setDisplayedData();
	}

	// raise the dialog
	_win32Dialog.runModeless(pFrame, AP_DIALOG_ID_LISTS, AP_RID_DIALOG_LIST, this);
}


#define _DS(c,s)       setDlgItemText(hWnd, AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)      setDlgItemText(hWnd, AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Lists::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	delete m_pPreviewWidget; m_pPreviewWidget = NULL;

	register unsigned int i;

	m_hThisDlg = hWnd;
	setbisCustomized(false);
	clearDirty();
	_resetCustomValues();
	_setDisplayedData();

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

	for (i = 0; i < G_N_ELEMENTS(rgSpinIDs); ++i)
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
	
	centerDialog();			

	const XAP_StringSet* pSS = m_pApp->getStringSet();
	setDialogTitle(pSS->getValue(AP_STRING_ID_DLG_Lists_Title));
	UT_return_val_if_fail (pSS,1);	// TODO: Would an error handler be more appropriate here?

	// Set the locale specific strings
	_DS(LIST_STATIC_TYPE, DLG_Lists_Type);
	_DS(LIST_STATIC_STYLE, DLG_Lists_Style);
	_DS(LIST_GROUP_CUSTOM, DLG_Lists_Customize);
	_DS(LIST_BUTTON_DEFAULT, DLG_Lists_SetDefault);
	_DS(LIST_STATIC_FORMAT, DLG_Lists_Format);
	_DS(LIST_STATIC_FONT, DLG_Lists_Font);
	_DS(LIST_BTN_FONT, DLG_Lists_ButtonFont);
	_DS(LIST_STATIC_DECIMAL, DLG_Lists_DelimiterString);
	_DS(LIST_STATIC_LEVEL, DLG_Lists_Level);
	_DS(LIST_STATIC_START_AT, DLG_Lists_Start);
	_DS(LIST_STATIC_LIST_ALIGN, DLG_Lists_Align);
	_DS(LIST_STATIC_INDENT_ALIGN, DLG_Lists_Indent);
	_DS(LIST_STATIC_PREVIEW, DLG_Lists_Preview);
	_DS(LIST_RADIO_START_NEW_LIST, DLG_Lists_Start_New);
	_DS(LIST_RADIO_APPLY_TO_CURRENT_LIST, DLG_Lists_Apply_Current);
	_DS(LIST_RADIO_RESUME_PREV_LIST, DLG_Lists_Resume);
	_DS(LIST_STATIC_FOLDING, DLG_Lists_FoldingLevelexp);

	pSS->getValue(isModal() ?
				_DSX(LIST_BTN_APPLY, DLG_OK):
							_DSX(LIST_BTN_APPLY, DLG_Apply));
	pSS->getValue(isModal() ?
				_DSX(LIST_BTN_CLOSE, DLG_Cancel):
							_DSX(LIST_BTN_CLOSE, DLG_Close));

	if (isModal())
	{
		// Hide the radio buttons if we're modal
		_win32Dialog.showControl(AP_RID_DIALOG_LIST_RADIO_START_NEW_LIST, SW_HIDE);
		_win32Dialog.showControl(AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST, SW_HIDE);
		_win32Dialog.showControl(AP_RID_DIALOG_LIST_RADIO_RESUME_PREV_LIST, SW_HIDE);
	}

	// Create a preview window.

	HWND hwndChild = GetDlgItem(hWnd, AP_RID_DIALOG_LIST_FRAME_PREVIEW);

	// If we don't get the preview control things are bad!
	UT_return_val_if_fail (hwndChild,1);

	m_pPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),
													  hwndChild,
													  0);
	UT_uint32 w,h;
	m_pPreviewWidget->getWindowSize(&w,&h);
	_createPreviewFromGC(m_pPreviewWidget->getGraphics(), w, h);
	m_pPreviewWidget->setPreview(getListsPreview());

	_fillTypeList();
	_fillFoldingList();
	setFoldLevelInGUI();

	// Creation and basic init done.
	// Continue with loading up the state it should display.

	activate();

	m_pAutoUpdateLists = UT_Timer::static_constructor(autoupdateLists, this);
	m_bDestroy_says_stopupdating = false;
	m_pAutoUpdateLists->set(500);	// auto-updater at 1/2 Hz
	
	

	return 1;							// 1 == we did not call SetFocus()
}


void AP_Win32Dialog_Lists::setFoldLevelInGUI(void)
{
	_setFoldingComboCurSel(getCurrentFold());
}

bool AP_Win32Dialog_Lists::isPageLists(void)
{
	if(isModal())
	{
		return true;
	}

	return true;  //not optimal, but just return true for now to make list folding work
}

BOOL AP_Win32Dialog_Lists::_onCommand(HWND /*hWnd*/, WPARAM wParam, LPARAM /*lParam*/)
{
	const WORD wNotifyCode	= HIWORD(wParam);
	const WORD wId			= LOWORD(wParam);

	switch (wId)
	{
	case AP_RID_DIALOG_LIST_RADIO_START_NEW_LIST:
		_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST, false);
		_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_RESUME_PREV_LIST, false);
		setiStartValue(1);
		_setListType(_getListTypeFromCombos());
		setDirty();
		_enableControls();
		_previewExposed();
		return 1;

	case AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST:
		_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_START_NEW_LIST, false);
		_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_RESUME_PREV_LIST, false);
		PopulateDialogData();
		setDirty();
		_enableControls();
		setNewListType(getDocListType());
		return 1;

	case AP_RID_DIALOG_LIST_RADIO_RESUME_PREV_LIST:
		_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST, false);
		_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_START_NEW_LIST, false);
		PopulateDialogData();
		setDirty();
		_enableControls();
		setNewListType(getDocListType());
		return 1;

	case IDCANCEL:	// also AP_RID_DIALOG_LIST_BTN_CLOSE
		destroy ();
		return 1;

	case AP_RID_DIALOG_LIST_BTN_APPLY:
		_onApply();
		return 1;

	case AP_RID_DIALOG_LIST_BUTTON_DEFAULT:
		_resetCustomValues();
		return 1;

	case AP_RID_DIALOG_LIST_COMBO_TYPE:
		if (wNotifyCode == LBN_SELCHANGE)
		{
			_typeChanged();
			return 1;
		}
		return 0;

	case AP_RID_DIALOG_LIST_COMBO_STYLE:
		if (wNotifyCode == LBN_SELCHANGE)
		{
			_styleChanged();
			return 1;
		}
		return 0;

	case AP_RID_DIALOG_LIST_COMBO_FOLDING:
		if (wNotifyCode == LBN_SELCHANGE)
		{
			_foldingChanged();
			return 1;
		}
		return 0;

	case AP_RID_DIALOG_LIST_BTN_FONT:
		_selectFont();
		return 1;						// return zero to let windows take care of it.

	case AP_RID_DIALOG_LIST_EDIT_FORMAT:
	case AP_RID_DIALOG_LIST_EDIT_LIST_ALIGN:
	case AP_RID_DIALOG_LIST_EDIT_INDENT_ALIGN:
	case AP_RID_DIALOG_LIST_EDIT_START_AT:
	case AP_RID_DIALOG_LIST_EDIT_LEVEL:
	case AP_RID_DIALOG_LIST_EDIT_DECIMAL:
		if (wNotifyCode == EN_CHANGE)
		{
			_getDisplayedData(wId);
			setDirty();
			_enableControls();
			_previewExposed();
			return 1;
		}
		return 0;

	default:	// we did not handle this notification
//		UT_DEBUGMSG(("AP_Win32Dialog_Lists::_onCommand, WM_Command for %svisible control ID %ld\n",
//					_win32Dialog.isControlVisible(wId) ? "" : "in", wId));
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

	if (!isDirty())
	{
		setDirty();
		_enableControls();	// The Apply button will become enabled
	}

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

	_getDisplayedData(controlID);
	_previewExposed();

	return 0;
}

void AP_Win32Dialog_Lists::destroy(void)
{
	if(isModal())
	{
		setAnswer(AP_Dialog_Lists::a_QUIT);
		EndDialog(m_hThisDlg, 0);
		m_hThisDlg = 0;
		return;
	}

	// Check that our hWnd still is valid. This is not always the case,
	// such as when shutting down the application.
	m_bDestroy_says_stopupdating = true;
	m_pAutoUpdateLists->stop();
	if (!isModal())
		modeless_cleanup();
	setAnswer(AP_Dialog_Lists::a_CLOSE);
	if (IsWindow(m_hThisDlg))
	{
		_win32Dialog.destroyWindow();
	}

	m_hThisDlg = 0;
}

void AP_Win32Dialog_Lists::activate(void)
{
	const bool bAtList = getisListAtPoint();

	if (!isModal())
	{
		_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_START_NEW_LIST, !bAtList);
		_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST, bAtList);
		_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_RESUME_PREV_LIST, false);
	}

	PopulateDialogData();

	_updateCaption();

	if (bAtList)
	{
		setNewListType(getDocListType());
	}

	_setDisplayedData();	// must come before _enableControls()
	_enableControls();

	// we need this second call to PopulateDialogData() since
	// _setListType changes the combo selection, and then
	// fillUncustomizedValues() is called...
	PopulateDialogData();

	_previewExposed();

	clearDirty();

	UT_DebugOnly<int> iResult = _win32Dialog.showWindow( SW_SHOW );

	iResult = _win32Dialog.bringWindowToTop();

	UT_ASSERT_HARMLESS((iResult != 0));

	_enableControls();
}

void AP_Win32Dialog_Lists::notifyActiveFrame(XAP_Frame *pFrame)
{
	if (_win32Dialog.isParentFrame(*pFrame))
	{
		return;
	}

	// change parent
	_win32Dialog.setParentFrame(pFrame);
	_win32Dialog.bringWindowToTop();
	_updateCaption();

	if (!getbisCustomized())
	{
		PopulateDialogData();
	}

	_setDisplayedData();
	_previewExposed();
}


void AP_Win32Dialog_Lists::_enableControls(void)
{
	const bool bStartNew	= _isNewListChecked();
	const bool bApplyToCurr	= _isApplyToCurrentChecked();
	const bool bResumeList	= _isResumeListChecked();
	const bool bAnyRadio	= bStartNew || bApplyToCurr || bResumeList;
	const int iType			= _getTypeComboCurSel();


	// Note that we can end up here without any radio button yet checked,
	// while initializing the dialog.

	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_COMBO_TYPE, bAnyRadio);
	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_COMBO_STYLE, iType != 0);
	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST,
								getisListAtPoint() && !bStartNew);

	// Apply button should only be enabled when user has changed something.
	// Note that we allow "Apply" for Type "None".
	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_BTN_APPLY, isDirty());

	_enableCustomControls(iType != 0);

	// Font button and List Format should only be enabled when we have
	// Numbered list.
	// These calls must come after _enableCustomControls() since it's
	// part of the "Custom Controls" group.
	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_BTN_FONT, iType == 2);
	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_EDIT_FORMAT, iType == 2);

	// The "Set Default Values" button should only be enabled if
	// we have a Type selected.
	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_BUTTON_DEFAULT, iType != 0);
	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_COMBO_FOLDING, iType != 0);
}

void AP_Win32Dialog_Lists::_onApply()
{
	const bool bStartNewList   = _isNewListChecked();
	const bool bApplyToCurrent = _isApplyToCurrentChecked();
	const bool bResumeList     = _isResumeListChecked();

	// There can be only one.
	UT_return_if_fail ((int(bStartNewList) + int(bApplyToCurrent) + int(bResumeList)) == 1);

	setbStartNewList(bStartNewList);
	setbApplyToCurrent(bApplyToCurrent);
	setbResumeList(bResumeList);
	setCurrentFold(_getFoldingComboCurSel());

	_getDisplayedData();
	_previewExposed();

	Apply();

	if(isModal())
	{
		setAnswer(AP_Dialog_Lists::a_OK);
		EndDialog(m_hThisDlg, 0);
		m_hThisDlg = 0;
		return;
	}

	// Since it's now applied, we obviously are at a list and should check
	// the "Apply to current" checkbox.
	_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_START_NEW_LIST, false);
	_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST, true);
	_win32Dialog.checkButton(AP_RID_DIALOG_LIST_RADIO_RESUME_PREV_LIST, false);

	_enableControls();

	PopulateDialogData();

	clearDirty();
	_setDisplayedData();
	_enableControls();
	_previewExposed();
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

int AP_Win32Dialog_Lists::_getFoldingComboCurSel() const
{
	return _win32Dialog.getComboSelectedIndex(AP_RID_DIALOG_LIST_COMBO_FOLDING);
}

void AP_Win32Dialog_Lists::_setTypeComboCurSel(int iSel)
{
	_win32Dialog.selectComboItem(AP_RID_DIALOG_LIST_COMBO_TYPE, iSel);
}

void AP_Win32Dialog_Lists::_setStyleComboCurSel(int iSel)
{
	_win32Dialog.selectComboItem(AP_RID_DIALOG_LIST_COMBO_STYLE, iSel);
}

void AP_Win32Dialog_Lists::_setFoldingComboCurSel(int iSel)
{
	_win32Dialog.selectComboItem(AP_RID_DIALOG_LIST_COMBO_FOLDING, iSel);
}

bool AP_Win32Dialog_Lists::_isNewListChecked() const
{
	return _win32Dialog.isChecked(AP_RID_DIALOG_LIST_RADIO_START_NEW_LIST) != 0;
}

bool AP_Win32Dialog_Lists::_isApplyToCurrentChecked() const
{
	return _win32Dialog.isChecked(AP_RID_DIALOG_LIST_RADIO_APPLY_TO_CURRENT_LIST) != 0;
}

bool AP_Win32Dialog_Lists::_isResumeListChecked() const
{
	return _win32Dialog.isChecked(AP_RID_DIALOG_LIST_RADIO_RESUME_PREV_LIST) != 0;
}


void AP_Win32Dialog_Lists::_fillTypeList()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	UT_return_if_fail (pSS);	// TODO: Would an error handler be more appropriate here?

	static const XAP_String_Id rgIDs[] =
	{
		AP_STRING_ID_DLG_Lists_Type_none,
		AP_STRING_ID_DLG_Lists_Type_bullet,
		AP_STRING_ID_DLG_Lists_Type_numbered
	};

	for (unsigned int i = 0; i < G_N_ELEMENTS(rgIDs); ++i)
	{
		_win32Dialog.addItemToCombo(AP_RID_DIALOG_LIST_COMBO_TYPE,
									pSS->getValue(rgIDs[i]));
	}
}

void AP_Win32Dialog_Lists::_fillStyleList(int iType)
{
	UT_return_if_fail (iType >= -1 && iType <=2);

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
		AP_STRING_ID_DLG_Lists_Heart_List,
		AP_STRING_ID_DLG_Lists_Arrowhead_List
	};

	static const XAP_String_Id rgNumberedIDs[] =
	{
		AP_STRING_ID_DLG_Lists_Numbered_List,
		AP_STRING_ID_DLG_Lists_Lower_Case_List,
		AP_STRING_ID_DLG_Lists_Upper_Case_List,
		AP_STRING_ID_DLG_Lists_Lower_Roman_List,
		AP_STRING_ID_DLG_Lists_Upper_Roman_List,
		AP_STRING_ID_DLG_Lists_Arabic_List,
		AP_STRING_ID_DLG_Lists_Hebrew_List
	};

	const XAP_String_Id*	pIDs;
	size_t					nIDs;

	switch (iType)
	{
		case 1:	// bullet list
			pIDs = rgBulletIDs;
			nIDs = G_N_ELEMENTS(rgBulletIDs);
			break;
		case 2:	// numbered list
			pIDs = rgNumberedIDs;
			nIDs = G_N_ELEMENTS(rgNumberedIDs);
			break;
	}

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_return_if_fail (pSS);	// TODO: Would an error handler be more appropriate here?

	HWND hComboStyle = GetDlgItem(m_hThisDlg, AP_RID_DIALOG_LIST_COMBO_STYLE);
	UT_return_if_fail (hComboStyle);
	int nMaxWidth = 0;

	// Get the HDC of the droplist to be able to get the
	// width of each of the strings.
	HDC hDCcombo = GetDC(hComboStyle);
	HDC hDC = CreateCompatibleDC(hDCcombo);
	ReleaseDC(hComboStyle, hDCcombo);

	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	SelectObject(hDC, hFont);

	for (size_t i = 0; i < nIDs; ++i)
	{
		LPCSTR psz = pSS->getValue(pIDs[i]);
		_win32Dialog.addItemToCombo(idComboStyle, psz);

		if (hDC)
		{
			SIZE size;
			::GetTextExtentPoint32(hDC, psz, strlen(psz), &size);
			if (size.cx > nMaxWidth)
			{
				nMaxWidth = size.cx;
			}
		}
	}

	DeleteDC(hDC);

	if (nMaxWidth > 50 && nMaxWidth < 500 /* sanity check*/)
	{
		SendMessageW(hComboStyle, CB_SETDROPPEDWIDTH,
					(WPARAM)nMaxWidth + 8, 0);
	}
}

void AP_Win32Dialog_Lists::_fillFoldingList()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	UT_return_if_fail (pSS);	// TODO: Would an error handler be more appropriate here?

	static const XAP_String_Id rgIDs[] =
	{
		AP_STRING_ID_DLG_Lists_FoldingLevel0,
		AP_STRING_ID_DLG_Lists_FoldingLevel1,
		AP_STRING_ID_DLG_Lists_FoldingLevel2,
		AP_STRING_ID_DLG_Lists_FoldingLevel3,
		AP_STRING_ID_DLG_Lists_FoldingLevel4
	};

	for (unsigned int i = 0; i < G_N_ELEMENTS(rgIDs); ++i)
	{
		_win32Dialog.addItemToCombo(AP_RID_DIALOG_LIST_COMBO_FOLDING,
									pSS->getValue(rgIDs[i]));
	}
}

void AP_Win32Dialog_Lists::_typeChanged()
{
	const int iType = _getTypeComboCurSel();

	const FL_ListType type =
		iType == 1 ? BULLETED_LIST :
		iType == 2 ? NUMBERED_LIST :
		NOT_A_LIST;

	_setListType(type);
	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_EDIT_START_AT, type == NUMBERED_LIST);
	_styleChanged();
	_enableControls();
	fillUncustomizedValues();	// Set defaults
	_setDisplayedData();
}

void AP_Win32Dialog_Lists::_styleChanged()
{
  	setDirty();
	setNewListType(_getListTypeFromCombos());
	setbisCustomized(false);
/*
	// have to do this, since for instance the decimal delimter can change

	fillUncustomizedValues();
*/
	_previewExposed();
	_setDisplayedData();
	_enableControls();
}

void AP_Win32Dialog_Lists::_foldingChanged()
{
  	setDirty();
	setFoldingLevelChanged(true);
	_enableControls();
}

//
// array of control IDs to enable/disable
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
	AP_RID_DIALOG_LIST_STATIC_DECIMAL,
	AP_RID_DIALOG_LIST_EDIT_DECIMAL,
	//AP_RID_DIALOG_LIST_EDIT_LEVEL,
	//AP_RID_DIALOG_LIST_SPIN_LEVEL,
	AP_RID_DIALOG_LIST_BTN_FONT
};

void AP_Win32Dialog_Lists::_resetCustomValues()
{
	setbisCustomized(false);
	setDirty();
	fillUncustomizedValues();
	_setDisplayedData();
	_enableControls();
	_previewExposed();
}

void AP_Win32Dialog_Lists::_enableCustomControls(bool bEnable)
{
	for (unsigned int i = 0; i < G_N_ELEMENTS(rgCustomIds); ++i)
	{
		_win32Dialog.enableControl(rgCustomIds[i], bEnable);
	}
}


void AP_Win32Dialog_Lists::_updateCaption()
{
	ConstructWindowName();
	_win32Dialog.setDialogTitle(getWindowName());	
}

void AP_Win32Dialog_Lists::_previewExposed()
{
	UT_return_if_fail(IsWindow(m_hThisDlg));

	if (m_pPreviewWidget)
	{
		setbisCustomized(true);
		event_PreviewAreaExposed();
	}
}


// The following two functions are used to not update controls
// if they don't need to be updated since every update, from
// user actions or from API update calls generates EN_CHANGE
// messages.
static void updateControlValue(XAP_Win32DialogHelper& helper, UT_sint32 id, int val)
{
	int nOld = helper.getControlInt(id);
	if (nOld != val)
	{
		helper.setControlInt(id, val);
	}
}

static void updateControlValue(XAP_Win32DialogHelper& helper, UT_sint32 id, LPCSTR val)
{
	char szBuff[100];
	helper.getControlText(id, szBuff, sizeof(szBuff));
	if (strcmp(val, szBuff))
	{
		helper.setControlText(id, val);
	}
}

//
// dialog internal data -> displayed values
//
void AP_Win32Dialog_Lists::_setDisplayedData()
{
	updateControlValue(_win32Dialog, AP_RID_DIALOG_LIST_EDIT_START_AT,getiStartValue());
	updateControlValue(_win32Dialog, AP_RID_DIALOG_LIST_EDIT_FORMAT	, getDelim().c_str());
	updateControlValue(_win32Dialog, AP_RID_DIALOG_LIST_EDIT_LEVEL	, getiLevel());
	updateControlValue(_win32Dialog, AP_RID_DIALOG_LIST_EDIT_DECIMAL, getDecimal().c_str());

	updateControlValue(_win32Dialog,AP_RID_DIALOG_LIST_EDIT_LIST_ALIGN,
						UT_convertToDimensionlessString(getfAlign(), ".2"));

	updateControlValue(_win32Dialog, AP_RID_DIALOG_LIST_EDIT_INDENT_ALIGN,
						UT_convertToDimensionlessString(getfIndent(), ".2"));

	_setListType(getNewListType());

	// Resume should only be enabled if the cursor is at/in a list
	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_RADIO_RESUME_PREV_LIST,
								getisListAtPoint());
}

//
// displayed values -> dialog internal data
//
void AP_Win32Dialog_Lists::_getDisplayedData(UT_sint32 controlId)
{
	char szTmp[32];

	if (controlId == -1 || controlId == AP_RID_DIALOG_LIST_EDIT_LIST_ALIGN)
	{
		_win32Dialog.getControlText(AP_RID_DIALOG_LIST_EDIT_LIST_ALIGN, szTmp, 30);
		setfAlign((float)UT_convertDimensionless(szTmp));
	}

	if (controlId == -1 || controlId == AP_RID_DIALOG_LIST_EDIT_INDENT_ALIGN)
	{
		_win32Dialog.getControlText(AP_RID_DIALOG_LIST_EDIT_INDENT_ALIGN, szTmp, 30);
		setfIndent((float)UT_convertDimensionless(szTmp));
	}

	if (controlId == -1 ||
		controlId == AP_RID_DIALOG_LIST_EDIT_LIST_ALIGN ||
		controlId == AP_RID_DIALOG_LIST_EDIT_INDENT_ALIGN)
	{
		if( getfAlign() < 0.0 )
		{
			setfAlign(0.0);
			_win32Dialog.setControlText(AP_RID_DIALOG_LIST_EDIT_INDENT_ALIGN,
								UT_convertToDimensionlessString(0.0, ".2"));
		}

		if ( (getfIndent() + getfAlign()) < 0.0 )
		{
			setfIndent(-getfAlign());
			_win32Dialog.setControlText(AP_RID_DIALOG_LIST_EDIT_INDENT_ALIGN,
								UT_convertToDimensionlessString(0.0, ".2"));
		}

	}

	if (getbisCustomized())
	{
		if (controlId == -1 || controlId == AP_RID_DIALOG_LIST_EDIT_START_AT)
		{
			setiStartValue(_win32Dialog.getControlInt(AP_RID_DIALOG_LIST_EDIT_START_AT));
		}
		if (controlId == -1 || controlId == AP_RID_DIALOG_LIST_EDIT_LEVEL)
		{
			setiLevel(_win32Dialog.getControlInt(AP_RID_DIALOG_LIST_EDIT_LEVEL));
		}
	}

	if (controlId == -1 || controlId == AP_RID_DIALOG_LIST_EDIT_FORMAT)
	{
		_win32Dialog.getControlText(AP_RID_DIALOG_LIST_EDIT_FORMAT, szTmp, sizeof(szTmp));
		copyCharToDelim(szTmp);
	}
	if (controlId == -1 || controlId == AP_RID_DIALOG_LIST_EDIT_DECIMAL)
	{
		_win32Dialog.getControlText(AP_RID_DIALOG_LIST_EDIT_DECIMAL, szTmp, sizeof(szTmp));
		copyCharToDecimal(szTmp);
	}
}

//
// Returns the FL_ListType of the currently selected Type/Style combination.
// Returns NOT_A_LIST on failure.
//
FL_ListType AP_Win32Dialog_Lists::_getListTypeFromCombos() const
{
	const int iType  = _getTypeComboCurSel();
	int iStyle = _getStyleComboCurSel();

	if (iType == 0	/* List Type "None" selected	*/ ||
		iType < 0	/* no selection at all ???		*/ ||
		iType > 2	/* Illegal value!				*/)
	{
		return NOT_A_LIST;
	}

	if (iType == 1 /* bullet list */)
	{
		return (FL_ListType)(iStyle + BULLETED_LIST);
	}

	if(iStyle >= BULLETED_LIST)
		iStyle += (OTHER_NUMBERED_LISTS - BULLETED_LIST + 1);

	// Numbered List, but just to make really REALLY sure, we assert it
	UT_ASSERT_HARMLESS(IS_NUMBERED_LIST_TYPE(iStyle));

	return (FL_ListType)iStyle;
}

void AP_Win32Dialog_Lists::_setListType(FL_ListType type)
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
		if(iStyle > BULLETED_LIST)
			iStyle -= (OTHER_NUMBERED_LISTS + 1 - BULLETED_LIST);
	}
	else
	{
		UT_ASSERT_HARMLESS(IS_BULLETED_LIST_TYPE(type));
		iType = 1;
		iStyle = type - BULLETED_LIST;
	}

	_setTypeComboCurSel(iType);
	_fillStyleList(iType);
	_setStyleComboCurSel(iStyle);
	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_COMBO_STYLE, iType != 0);

	setNewListType(type);
}	// Do NOT call _setDisplayedData() from here. It will recurse!


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
//		_win32Dialog.enableControl(AP_RID_DIALOG_LIST_BTN_FONT, false);
		return;
	}

	XAP_DialogFactory* pDialogFactory = pFrame->getDialogFactory();
	UT_return_if_fail (pDialogFactory);

	XAP_Dialog_FontChooser* pDialog
		= (XAP_Dialog_FontChooser *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_FONT));
	UT_ASSERT_HARMLESS(pDialog);

	if (!pDialog)	// runtime failsafe
	{
		// Disable the Font button. If we couldn't get the dialog
		// this time, chances are we never will. :-(
		_win32Dialog.enableControl(AP_RID_DIALOG_LIST_BTN_FONT, false);
		MessageBeep(MB_ICONASTERISK);
		return;
	}

	PP_PropertyVector props_in;

	bool bUnderline = false;

	bool bOverline = false;

	bool bStrikeOut = false;

	bool bTopLine = false;
	bool bBottomLine = false;


	if (pView->getCharFormat(props_in))
	{
		// stuff font properties into the dialog.

		const std::string & sFontFamily = PP_getAttribute("font-family", props_in);
		const std::string & sFontSize = PP_getAttribute("font-size", props_in);
		const std::string & sFontWeight = PP_getAttribute("font-weight", props_in);
		const std::string & sFontStyle = PP_getAttribute("font-style", props_in);
		const std::string & sColor = PP_getAttribute("color", props_in);

		pDialog->setFontFamily(sFontFamily);
		pDialog->setFontSize(sFontSize);
		pDialog->setFontWeight(sFontWeight);
		pDialog->setFontStyle(sFontStyle);
		pDialog->setColor(sColor);

		// these behave a little differently since they are
		// probably just check boxes and we don't have to
		// worry about initializing a combo box with a choice
		// (and because they are all stuck under one CSS attribute).

                const std::string & s = PP_getAttribute("text-decoration", props_in);
		if (!s.empty())
		{
			bUnderline = s.find("underline") != std::string::npos;
			bOverline = s.find("overline") != std::string::npos;
			bStrikeOut = s.find("line-through") != std::string::npos;

			bTopLine = s.find("topline") != std::string::npos;

			bBottomLine = s.find("bottomline") != std::string::npos;
		}
		pDialog->setFontDecoration(bUnderline,bOverline,bStrikeOut,false,false);
	}

	pDialog->setGraphicsContext(pView->getGraphics());
	pDialog->runModal(pFrame);

	if (pDialog->getAnswer() != XAP_Dialog_FontChooser::a_OK &&
		pDialog->getAnswer() != XAP_Dialog_FontChooser::a_YES)
	{
		return;
	}

	std::string szFont;


	if (pDialog->getChangedFontFamily(szFont))

/*

		|| pDialog->getChangedFontSize(szFont)

		|| pDialog->getChangedFontWeight(szFont)

		|| pDialog->getChangedFontStyle(szFont)

		|| pDialog->getChangedBGColor(szFont)

		|| pDialog->getChangedColor(szFont)

		|| pDialog->getChangedUnderline(&bUnderline)

		|| pDialog->getChangedOverline(&bOverline)

		|| pDialog->getChangedStrikeOut(&bStrikeOut)

		|| pDialog->getChangedTopline(&bTopLine)

		|| pDialog->getChangedBottomline(&bBottomLine)

		)

*/
	{

		setDirty();
		copyCharToFont(szFont);
		_previewExposed();

		_enableControls();
	}
}


void AP_Win32Dialog_Lists::autoupdateLists(UT_Worker * pWorker)
{
	UT_return_if_fail (pWorker);
	// this is a static callback method and does not have a 'this' pointer.
	AP_Win32Dialog_Lists* pDialog =
		reinterpret_cast<AP_Win32Dialog_Lists*>(pWorker->getInstanceData());
	// Handshaking code. Plus only update if something in the document
	// changed.

	UT_ASSERT_HARMLESS(pDialog);

	if (pDialog)	// runtime failsafe
	{
		if (pDialog->isDirty())
		{
			return;
		}
		AV_View* pView = pDialog->getAvView();
		if (!pView || pView->getTick() != pDialog->getTick())
		{
			pDialog->setTick(pView ? pView->getTick() : 0);
			pDialog->PopulateDialogData();
			pDialog->_setDisplayedData();
			pDialog->clearDirty();
			// This is done improperly by _setDisplayedData()
			// since it leaves the Apply button enabled.
			pDialog->_enableControls();
			// This is done by _setDisplayedData();
			//pDialog->_previewExposed();

		}
	}
}
