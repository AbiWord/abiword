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

#include <commctrl.h>

/*****************************************************************/

#include "XAP_Win32Frame.h"

class XAP_Win32Dialog
{
public:
	virtual BOOL					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam) = 0;
	virtual BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) = 0;
	virtual BOOL					_onDeltaPos(NM_UPDOWN * pnmud) = 0;
};

class XAP_Win32DialogHelper
{
public:
	XAP_Win32DialogHelper(XAP_Win32Dialog* p_dialog)
	:	m_pDialog(p_dialog),
		m_hDlg(0)
	{
	}


public:

	void				runModal(		XAP_Frame*				pFrame,
										XAP_Dialog_Id			dialog_id,
										UT_sint32				resource_id,
										XAP_Dialog*				p_dialog);
	
	void				 runModeless(	XAP_Frame*				pFrame,
										XAP_Dialog_Id			dialog_id,
										UT_sint32				resource_id,
										XAP_Dialog_Modeless*	p_dialog);

	static BOOL CALLBACK	s_dlgProc(	HWND	hWnd,
									UINT	msg,
									WPARAM	wParam,
									LPARAM	lParam);

	void				checkButton(UT_sint32 controlId, bool bChecked = true);
	void				enableControl(UT_sint32 controlId, bool bEnabled = true);
	void				destroyWindow();
	void				setDialogTitle(LPCSTR p_str);
	int					showWindow( int Mode );
	int					showControl(UT_sint32 controlId, int Mode);
	int					bringWindowToTop();

	// Combo boxes.

	void				addItemToCombo(UT_sint32 controlId, LPCSTR p_str);
	void				selectComboItem(UT_sint32 controlId, int index);
	int					getComboSelectedIndex(UT_sint32 controlId) const;
	void				resetComboContent(UT_sint32 controlId);

	// List boxes

	void				resetContent(UT_sint32 controlId);
	void				addItemToList(UT_sint32 controlId, LPCSTR p_str);
	int					getListSelectedIndex(UT_sint32 controlId) const;
	void				selectListItem(UT_sint32 controlId, int index);
	void				getListText(UT_sint32 controlId, int index, char *p_str) const;

	// Controls
	void				setControlText(UT_sint32 controlId, LPCSTR p_str);
	void				setControlInt(UT_sint32 controlId, int value);
	int					getControlInt(UT_sint32 controlId) const;
	int					isChecked(UT_sint32 controlId) const;
	void				getControlText(	UT_sint32	controlId,
										LPSTR		p_buffer,
										UT_sint32	Buffer_length) const;

	bool				isControlVisible(UT_sint32	controlId) const;

	bool				isParentFrame(const XAP_Win32Frame& frame) const;
	void				setParentFrame(const XAP_Win32Frame* pFrame);
	XAP_Win32Frame*		getParentFrame();

private:
	XAP_Win32Dialog	*			m_pDialog;
	HWND						m_hDlg;
};

#endif /* XAP_Win32DialogHelper_H */
