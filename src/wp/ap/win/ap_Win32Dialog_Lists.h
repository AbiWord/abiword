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

#ifndef AP_Win32Dialog_List_H
#define AP_Win32Dialog_List_H

#include "ap_Dialog_Lists.h"
#include "xap_Win32DialogHelper.h"
#include "xap_Win32PreviewWidget.h"


class XAP_Win32Frame;

/*****************************************************************/

class AP_Win32Dialog_Lists: public AP_Dialog_Lists, XAP_Win32Dialog
{
public:
	AP_Win32Dialog_Lists(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_Lists(void);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			destroy(void);
	virtual void			activate(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	static void				autoupdateLists(UT_Timer * pTimer);


protected:
	BOOL					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onDeltaPos(NM_UPDOWN * pnmud);
	
	void					_enableControls();
	void					_onApply();

private:
	// overridden virtual functions
	virtual void			notifyActiveFrame(XAP_Frame *pFrame);
	virtual void			notifyCloseFrame(XAP_Frame *pFrame);

	// current selection of the drop-list combo boxes
	int						_getTypeComboCurSel() const;
	int						_getStyleComboCurSel() const;
	void					_setTypeComboCurSel(int iSel);
	void					_setStyleComboCurSel(int iSel);

	UT_Bool					_isNewList() const;
	void					_fillTypeList();
	void					_fillStyleList(int iType);
	void					_typeChanged();
	void					_styleChanged();
	void					_customChanged();
	void					_enableCustomControls(UT_Bool bEnable = UT_TRUE);
	void					_updateCaption();
	void					_previewExposed();
	void					_setData();			// data -> "view"
	void					_getData();			// "view" -> data
	List_Type				_getListType() const;
	void					_setListType(List_Type type);
	void					_selectFont();
	virtual const XML_Char*	_getDingbatsFontName() const;

	UT_Timer*				m_pAutoUpdateLists;
	XAP_Win32DialogHelper	_win32Dialog;
	XAP_Win32PreviewWidget*	m_pPreviewWidget;
	UT_Bool					m_bDisplayCustomControls;
};

#endif /* AP_Win32Dialog_List_H */
