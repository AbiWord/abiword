/* AbiSource
 *
 * Copyright (C) 2011 Urmas Dren <davian818@gmail.com>
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
#ifndef AP_WIN32DIALOG_EPUBEXPORTOPTIONS_H
#define	AP_WIN32DIALOG_EPUBEXPORTOPTIONS_H

#include "ap_Dialog_EpubExportOptions.h"
#include "xap_Frame.h"
#include "xap_Win32App.h"
#include "xap_Win32DialogHelper.h"

class AP_Win32Dialog_EpubExportOptions: public AP_Dialog_EpubExportOptions
{
public:

	AP_Win32Dialog_EpubExportOptions(XAP_DialogFactory *pDlgFactory,
	                                 XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_EpubExportOptions(void);
	virtual void runModal(XAP_Frame *pFrame);
	static XAP_Dialog *static_constructor(XAP_DialogFactory *pDF,
	                                      XAP_Dialog_Id id);

	static INT_PTR CALLBACK s_dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	INT_PTR _onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	INT_PTR _onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	static void setInstance(HINSTANCE hModule)
	{
		m_hModule = hModule;
	}
	static HINSTANCE getInstance()
	{
		return m_hModule;
	}
private:
	static HINSTANCE m_hModule;

	HWND m_hDlg;
	void setvalues();
	void gathervalues();
};

#endif	/* AP_WIN32DIALOG_EPUBEXPORTOPTIONS_H */

