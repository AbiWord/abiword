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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_WIN32DIALOG_Background_H
#define AP_WIN32DIALOG_Background_H

#include "ap_Dialog_Background.h"

class XAP_Frame;

/*****************************************************************/

class ABI_EXPORT AP_Win32Dialog_Background: public AP_Dialog_Background
{
public:
	AP_Win32Dialog_Background(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_Background(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

private:
	static UINT CALLBACK s_hookProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

	void _centerDialog();
	void _callHelp();

	HWND m_hDlg;
};

#endif /* AP_WIN32DIALOG_Background_H */
