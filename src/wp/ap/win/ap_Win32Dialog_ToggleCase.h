/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef AP_WIN32DIALOG_TOGGLECASE_H
#define AP_WIN32DIALOG_TOGGLECASE_H

#include "ap_Dialog_ToggleCase.h"
#include "xap_Win32DialogHelper.h"


class XAP_Win32Frame;

/*****************************************************************/

class AP_Win32Dialog_ToggleCase: public AP_Dialog_ToggleCase, XAP_Win32Dialog
{
public:
	AP_Win32Dialog_ToggleCase(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_ToggleCase(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	
protected:
	BOOL					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onDeltaPos(NM_UPDOWN * pnmud);

private:
	HWND					m_hThisDlg;
	XAP_Win32DialogHelper	m_helper;
	ToggleCase				m_iWhichCase;

};

#endif /* AP_WIN32DIALOG_TOGGLECASE_H */
