/* Copyright (C) 2008 AbiSource Corporation B.V.
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

#ifndef __AP_WIN32DIALOG_GENERICPROGRESS_H__
#define __AP_WIN32DIALOG_GENERICPROGRESS_H__

#include <backends/service/xp/ap_Dialog_GenericProgress.h>

class XAP_Frame;

class AP_Win32Dialog_GenericProgress : public AP_Dialog_GenericProgress
{
public:
	AP_Win32Dialog_GenericProgress(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	static XAP_Dialog * static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id);
	void						runModal(XAP_Frame * pFrame);
	static BOOL CALLBACK		s_dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL						_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL 						_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	virtual void				close(bool cancel = false);
	virtual void				setProgress(UT_uint32 progress);

private:

	XAP_Win32DialogHelper *		m_pWin32Dialog;
	HINSTANCE 					m_hInstance;
	HWND						m_hWnd;
	HWND						m_hProgress;
};

#endif /* __AP_WIN32DIALOG_GENERICPROGRESS_H__ */
