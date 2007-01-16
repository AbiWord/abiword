/* AbiWord - Win32 PageNumbers Dialog
 * Copyright (C) 2001 Mike Nordell
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

#ifndef AP_WIN32DIALOG_PAGENUMBERS_H
#define AP_WIN32DIALOG_PAGENUMBERS_H

#include "ap_Dialog_PageNumbers.h"
#include "xap_Win32DialogHelper.h"

// fwd decl.
class XAP_Win32PreviewWidget;

/*****************************************************************/

class AP_Win32Dialog_PageNumbers : public AP_Dialog_PageNumbers, XAP_Win32Dialog
{
public:
	AP_Win32Dialog_PageNumbers(XAP_DialogFactory* pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_PageNumbers();

	static XAP_Dialog*	static_constructor(XAP_DialogFactory*, XAP_Dialog_Id id);

private:
	// TMN: Respect "private" scope. No "please", no nothing. Respect it!

	// implemented for AP_Dialog_PageNumbers
	virtual void		runModal(XAP_Frame* pFrame);

	// implemented for XAP_Win32Dialog
	virtual BOOL		_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	virtual BOOL		_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	virtual BOOL		_onDeltaPos(NM_UPDOWN * pnmud) { return FALSE; };

	void				_createPreviewWidget();

	HWND                    m_hThisDlg;
	XAP_Win32DialogHelper	m_helper;
	XAP_Win32PreviewWidget*	m_pPreviewWidget;
};

#endif	// AP_WIN32DIALOG_PAGENUMBERS_H
