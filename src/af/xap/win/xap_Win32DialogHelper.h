// xap_Win32DialogHelper.h

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

#ifndef XAP_Win32DialogHelper_H
#define XAP_Win32DialogHelper_H

/*****************************************************************/

#include "XAP_Win32Frame.h"

class XAP_Win32Dialog
{
public:
	virtual BOOL					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam) = 0;
	virtual BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) = 0;
};

class XAP_Win32DialogHelper
{
public:
	XAP_Win32DialogHelper(XAP_Win32Dialog *p_dialog)
	{
		m_pDialog = p_dialog;
	}


private:
	XAP_Win32Dialog	*			m_pDialog;
	HWND						m_hDlg;

public:

	void runModal(XAP_Frame * pFrame, XAP_Dialog_Id dialog_id, UT_sint32 resource_id, XAP_Dialog *p_dialog);
	void runModeless(XAP_Frame * pFrame, XAP_Dialog_Id dialog_id, UT_sint32 resource_id, XAP_Dialog_Modeless *p_dialog);
	static BOOL CALLBACK s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

	void				checkButton(UT_sint32 controlId, UT_Bool bState = UT_TRUE)
						{
							CheckDlgButton(m_hDlg, controlId, bState ? BST_CHECKED : BST_UNCHECKED);
						}
	void				enableControl(UT_sint32 controlId, UT_Bool bState = UT_TRUE)
						{
							EnableWindow(GetDlgItem(m_hDlg, controlId), bState);
						}
	void				destroyWindow(void)
						{
							DestroyWindow(m_hDlg);
						}
	void				setDialogTitle(LPCSTR p_str)
						{
							SetWindowText(m_hDlg, p_str);
						}
	int					showWindow( int Mode )
						{
							return ShowWindow(m_hDlg, Mode);
						}
	int					showControl(UT_sint32 controlId, int Mode )
						{
							return ShowWindow(GetDlgItem(m_hDlg, controlId), Mode);
						}
	int					bringWindowToTop()
						{
							return BringWindowToTop(m_hDlg);
						}
	void				addItemToCombo(UT_sint32 controlId, LPCSTR p_str)
						{
							SendDlgItemMessage(m_hDlg, controlId, CB_ADDSTRING, 0, (LPARAM)p_str);
						}
	void				selectComboItem(UT_sint32 controlId, int index)
						{
							SendDlgItemMessage(m_hDlg, controlId, CB_SETCURSEL, index, 0);
						}
	int					getComboSelectedIndex(UT_sint32 controlId)
						{
							return SendDlgItemMessage(m_hDlg, controlId, CB_GETCURSEL, 0, 0);
						}
	void				setControlText(UT_sint32 controlId, LPCSTR p_str)
						{
							SetDlgItemText(m_hDlg, controlId, p_str);
						}
	void				setControlInt(UT_sint32 controlId, int value)
						{
							SetDlgItemInt(m_hDlg, controlId, value, TRUE);
						}
	int					getControlInt(UT_sint32 controlId)
						{
							return GetDlgItemInt(m_hDlg, controlId, NULL, FALSE);
						}
	int					isChecked(UT_sint32 controlId)
						{
							return IsDlgButtonChecked(m_hDlg, controlId);
						}
};

#endif /* XAP_Win32DialogHelper_H */
