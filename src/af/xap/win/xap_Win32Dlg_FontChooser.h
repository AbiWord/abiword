/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef XAP_WIN32DIALOG_FONTCHOOSER_H
#define XAP_WIN32DIALOG_FONTCHOOSER_H

#include "xap_Dlg_FontChooser.h"
#include "xap_Win32PreviewWidget.h"
#include "xap_Win32DialogBase.h"
#include "xap_Frame.h"

/*****************************************************************/

class ABI_EXPORT XAP_Win32Dialog_FontChooser : public XAP_Dialog_FontChooser, XAP_Win32DialogBase
{
public:
	XAP_Win32Dialog_FontChooser(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Win32Dialog_FontChooser(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	static UINT CALLBACK	s_hookProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	BOOL					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_callHelp();

	XAP_Frame *				m_pWin32Frame;
	XAP_Win32PreviewWidget* m_pPreviewWidget;

	bool					m_bWin32Overline;
//	bool					m_bWin32Topline;
//	bool					m_bWin32Bottomline;
	bool                    m_bWin32Hidden;
	bool					m_bWin32SuperScript;
	bool					m_bWin32SubScript;

	int                     m_iColorIndx;
	int                     m_iColorCount;
};

#endif /* XAP_WIN32DIALOG_FONTCHOOSER_H */
