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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef AP_WIN32DIALOG_GOTO_H
#define AP_WIN32DIALOG_GOTO_H

#include "ap_Dialog_Goto.h"
class XAP_Win32Frame;

/*****************************************************************/

class AP_Win32Dialog_Goto: public AP_Dialog_Goto
{
public:
	AP_Win32Dialog_Goto(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_Goto(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			destroy(void);
	virtual void			activate(void);
	virtual void			notifyActiveFrame(XAP_Frame *pFrame);
	virtual void			notifyCloseFrame(XAP_Frame *pFrame);
	static BOOL CALLBACK	s_dlgProc(HWND,UINT,WPARAM,LPARAM);
	void					GoTo (const char *number);
	void					setSelectedRow (int row);
	int						getSelectedRow (void);

	void *					pGetWindowHandle( void ) { return (void*)m_hWnd; }

protected:
	BOOL					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	int                     m_iRow;
	HWND                    m_hWnd;

	char *					m_pszOldValue;
};

#endif /* AP_WIN32DIALOG_GOTO_H */

