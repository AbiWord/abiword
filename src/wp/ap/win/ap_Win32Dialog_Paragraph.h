/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

#ifndef AP_WIN32DIALOG_PARAGRAPH_H
#define AP_WIN32DIALOG_PARAGRAPH_H

#include "ap_Dialog_Paragraph.h"

class GR_Win32Graphics;
class XAP_Win32Frame;
class XAP_Win32PreviewWidget;

/*****************************************************************/

class AP_Win32Dialog_Paragraph: public AP_Dialog_Paragraph
{
public:
	AP_Win32Dialog_Paragraph(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_Paragraph(void);

	virtual void				runModal(XAP_Frame * pFrame);

	static XAP_Dialog *			static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	static BOOL CALLBACK		s_dlgProc(HWND,UINT,WPARAM,LPARAM);
	
protected:
	BOOL						_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL						_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);   

	static BOOL CALLBACK		s_tabProc(HWND,UINT,WPARAM,LPARAM);

	BOOL						_onInitTab(HWND hWnd, WPARAM wParam, LPARAM lParam);

	XAP_Win32PreviewWidget *	m_pPreviewWidget;

	HWND						m_hwndDlg;		// parent dialog
	HWND						m_hwndTab;		// tab control
	HWND						m_hwndSpacing;	// subdialog for first tab
	HWND						m_hwndBreaks;	// subdialog for second tab

};

#endif /* XAP_WIN32DIALOG_PARAGRAPH_H */
