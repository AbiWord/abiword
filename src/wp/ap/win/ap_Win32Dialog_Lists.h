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

#ifndef AP_Win32Dialog_List_H
#define AP_Win32Dialog_List_H

#include "ap_Dialog_Lists.h"
#include "xap_Win32DialogHelper.h"
#include "xap_Win32PreviewWidget.h"
#include "xap_Frame.h"
#include "xap_Win32DialogBase.h"

/*****************************************************************/

class ABI_EXPORT AP_Win32Dialog_Lists: public AP_Dialog_Lists, XAP_Win32Dialog, public XAP_Win32DialogBase
{
public:
	AP_Win32Dialog_Lists(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_Lists(void);

	virtual void			runModal(XAP_Frame* pFrame);
	virtual void			runModeless(XAP_Frame* pFrame);
	virtual void			destroy();
	virtual void			activate();

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	static void				autoupdateLists(UT_Worker * pTimer);
	virtual void            setFoldLevelInGUI(void);
	virtual bool            isPageLists(void);

protected:
	BOOL					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onDeltaPos(NM_UPDOWN * pnmud);

	void					_enableControls();
	void					_onApply();

private:
	// overridden virtual functions
	virtual void			notifyActiveFrame(XAP_Frame *pFrame);
	virtual void * 			pGetWindowHandle() { return (void *) m_hThisDlg; }

	// current selection of the drop-list combo boxes
	int						_getTypeComboCurSel() const;
	int						_getStyleComboCurSel() const;
	int						_getFoldingComboCurSel() const;
	void					_setTypeComboCurSel(int iSel);
	void					_setStyleComboCurSel(int iSel);
	void					_setFoldingComboCurSel(int iSel);

	bool					_isNewListChecked() const;
	bool					_isApplyToCurrentChecked() const;
	bool					_isResumeListChecked() const;
	void					_fillTypeList();
	void					_fillStyleList(int iType);
	void					_fillFoldingList();
	void					_typeChanged();
	void					_styleChanged();
	void					_foldingChanged();
	void					_resetCustomValues();
	void					_enableCustomControls(bool bEnable = true);
	void					_updateCaption();
	void					_previewExposed();
	void					_setDisplayedData();	// data -> "view"
	void					_getDisplayedData(UT_sint32 controlId = -1);	// "view" -> data
	FL_ListType				_getListTypeFromCombos() const;
	void					_setListType(FL_ListType type);
	void					_selectFont();

	bool					m_bDestroy_says_stopupdating;
	bool					m_bAutoUpdate_happening_now;
	UT_Timer*				m_pAutoUpdateLists;
	XAP_Win32DialogHelper	_win32Dialog;
	XAP_Win32PreviewWidget*	m_pPreviewWidget;
	bool					m_bEnableCustomControls;
	HWND					m_hThisDlg;
};

#endif /* AP_Win32Dialog_List_H */
