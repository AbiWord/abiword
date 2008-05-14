/* AbiWord
 * Copyright (C) 2003 Jordi Mas i Hernàdez, jmas@softcatala.org
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

#ifndef AP_WIN32DIALOG_MAILMERGE_H
#define AP_WIN32DIALOG_MAILMERGE_H

#include "ap_Dialog_MailMerge.h"
#include "xap_Frame.h"

/*****************************************************************/

class ABI_EXPORT AP_Win32Dialog_MailMerge: public AP_Dialog_MailMerge
{
public:
	AP_Win32Dialog_MailMerge(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_MailMerge(void);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void            destroy(void);
	virtual void *			pGetWindowHandle( void ) { return  (void *) m_hwndDlg; }

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	
	virtual BOOL			_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK	s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	virtual BOOL 			_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	
protected:

	virtual void setFieldList();
	
	HWND							m_hwndDlg;	//  dialog box Windows
};

#endif /* AP_WIN32DIALOG_MAILMERGE_H */
