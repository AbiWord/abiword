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

#ifndef AP_WIN32DIALOG_STYLIST_H
#define AP_WIN32DIALOG_STYLIST_H

#include "ap_Dialog_Stylist.h"
#include "xap_Frame.h"
#include "xap_Win32DialogBase.h"

/*****************************************************************/

class ABI_EXPORT AP_Win32Dialog_Stylist: public AP_Dialog_Stylist, XAP_Win32DialogBase
{
public:
	AP_Win32Dialog_Stylist(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_Stylist(void);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	virtual void            activate(void);
	virtual void            destroy(void);
	virtual void            notifyActiveFrame(XAP_Frame * pFrame);
	static BOOL CALLBACK	s_dlgProc(HWND,UINT,WPARAM,LPARAM);
	static BOOL CALLBACK 	s_treeProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	virtual void            setStyleInGUI(void);
	virtual void *  		pGetWindowHandle(void) {  return (void *) m_hDlg; }

protected:
	BOOL					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void					_populateWindowData(void);
	void					_fillTree(void);
	BOOL					_styleClicked(void);

};

#endif /* AP_WIN32DIALOG_STYLIST_H */
