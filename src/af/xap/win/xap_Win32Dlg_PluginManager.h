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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef XAP_WIN32DIALOG_PLUGIN_MANAGER_H
#define XAP_WIN32DIALOG_PLUGIN_MANAGER_H

#include "xap_Dlg_PluginManager.h"
#include "xap_Frame.h"

/*****************************************************************/

class XAP_Win32Dialog_PluginManager: public XAP_Dialog_PluginManager
{
public:
	XAP_Win32Dialog_PluginManager(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Win32Dialog_PluginManager(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	static BOOL CALLBACK		s_dlgProc(HWND,UINT,WPARAM,LPARAM);	

protected:

	BOOL						_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL						_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);   
	BOOL						_onNotify(HWND hWnd, LPARAM lParam);
	
	static BOOL CALLBACK			s_tabProc(HWND,UINT,WPARAM,LPARAM);
	BOOL						_onInitTab(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL						_onCommandTab(HWND hWnd, WPARAM wParam, LPARAM lParam);

	void						event_Load();
	void						refresh_Tab1();

	UT_sint32					m_curSelection;
	HWND						m_hwndDlg;		// parent dialog
	HWND						m_hwndTab;		// tab control in parent dialog

	int						m_nrSubDlgs;		// number of tabs on tab control
	UT_Vector					m_vecSubDlgHWnd;	// hwnd to each sub-dialog
	XAP_Frame*					m_pFrame;


};

#endif /* XAP_WIN32DIALOG_PLUGIN_MANAGER_H */
