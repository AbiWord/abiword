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

#ifndef AP_Win32Dialog_PageSetup_H
#define AP_Win32Dialog_PageSetup_H

#include "ap_Dialog_PageSetup.h"

class AP_Win32Dialog_PageSetup : public AP_Dialog_PageSetup
{
public:
	AP_Win32Dialog_PageSetup (XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_PageSetup (void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	static BOOL CALLBACK	s_dlgProc(HWND,UINT,WPARAM,LPARAM);	

protected:
	UT_sint32					m_curSelection;


	BOOL						_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL						_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);   
	BOOL						_onNotify(HWND hWnd, LPARAM lParam);
	
	static BOOL CALLBACK			s_tabProc(HWND,UINT,WPARAM,LPARAM);
	BOOL						_onInitTab(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL						_onCommandTab(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL						_onNotifyTab(HWND hWnd, LPARAM lParam);

	void						doSpinControl(UT_uint32 id, UT_sint32 delta);
	void						updatePageSize();
	void						updateWidth();
	void						updateHeight();
	void						updateMargins();
	void						updateTopMargin();
	void						updateBottomMargin();
	void						updateLeftMargin();
	void						updateRightMargin();
	void						updateHeaderMargin();
	void						updateFooterMargin();
	void						updatePreview();

	HWND						m_hwndDlg;		// parent dialog
	HWND						m_hwndTab;		// tab control in parent dialog

	int						m_nrSubDlgs;		// number of tabs on tab control
	UT_Vector					m_vecSubDlgHWnd;	// hwnd to each sub-dialog

	HBITMAP					m_bmpLandscape;
	HBITMAP					m_bmpPortrait;
	HBITMAP					m_bmpPreview;
	
	fp_PageSize					m_PageSize;

	XAP_Frame *					m_pWin32Frame;			
};

#endif
