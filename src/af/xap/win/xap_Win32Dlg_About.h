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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef XAP_WIN32DIALOG_ABOUT_H
#define XAP_WIN32DIALOG_ABOUT_H

#include "xap_Dlg_About.h"
class GR_Win32Image;

/*****************************************************************/

class XAP_Win32Dialog_About: public XAP_Dialog_About
{
public:
	XAP_Win32Dialog_About(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Win32Dialog_About(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	static BOOL CALLBACK	s_dlgProc(HWND,UINT,WPARAM,LPARAM);
	
protected:
	static UT_Bool			s_bEventLoopDone;
	
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	GR_Win32Image *			m_pGrImageSidebar;
};

#endif /* XAP_WIN32DIALOG_ABOUT_H */
