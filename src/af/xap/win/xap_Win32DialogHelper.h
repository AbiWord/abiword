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
	static BOOL CALLBACK s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

	void						checkButton(UT_sint32 controlId, UT_Bool bState = UT_TRUE)
									{
									CheckDlgButton(m_hDlg, controlId, bState ? BST_CHECKED : BST_UNCHECKED);
									}
	void						enableControl(UT_sint32 controlId, UT_Bool bState = UT_TRUE)
									{
									EnableWindow(GetDlgItem(m_hDlg, controlId), bState);
									}
};

#endif /* XAP_Win32DialogHelper_H */
