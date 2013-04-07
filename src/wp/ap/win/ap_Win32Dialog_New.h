/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#ifndef AP_WIN32DIALOG_NEW_H
#define AP_WIN32DIALOG_NEW_H

#include "xap_Win32DialogHelper.h"
#include "xap_Win32DialogBase.h"
#include "ap_Dialog_New.h"

class XAP_Frame;
class XAP_Win32Frame;

/*****************************************************************/

class ABI_EXPORT AP_Win32Dialog_New: public AP_Dialog_New, XAP_Win32Dialog, XAP_Win32DialogBase
{
public:
	AP_Win32Dialog_New(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_New(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

private:
	HWND						m_hThisDlg;
	XAP_Win32DialogHelper		_win32Dialog;
	XAP_Frame *					m_pFrame;


protected:
	BOOL	_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL	_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL	_onDeltaPos(NM_UPDOWN * pnmud);

	void	_doChoose();
	void	_updateControls();
	void 	_setFileName( UT_sint32 nIndex );
};

#endif /* AP_WIN32DIALOG_NEW_H */
