/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#ifndef XAP_WIN32DIALOG_DOCCOMPARISON_H
#define XAP_WIN32DIALOG_DOCCOMPARISON_H

#include "xap_Dlg_DocComparison.h"
#include "xap_Frame.h"


/*****************************************************************/

class ABI_EXPORT XAP_Win32Dialog_DocComparison: public XAP_Dialog_DocComparison
{
public:
	XAP_Win32Dialog_DocComparison(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Win32Dialog_DocComparison(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	static BOOL CALLBACK	s_dlgProc(HWND,UINT,WPARAM,LPARAM);

protected:
	BOOL					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

};

#endif /* XAP_WIN32DIALOG_DOCCOMPARISON_H */
