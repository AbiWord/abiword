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

#ifndef AP_WIN32DIALOG_WORDCOUNT_H
#define AP_WIN32DIALOG_WORDCOUNT_H

#include <commctrl.h>
#include "ap_Dialog_WordCount.h"
#include "ut_timer.h"

class XAP_Win32Frame;

/*****************************************************************/

class AP_Win32Dialog_WordCount: public AP_Dialog_WordCount
{
public:
	AP_Win32Dialog_WordCount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_WordCount(void);

	virtual void			runModal(XAP_Frame * pFrame);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			destroy(void);
	virtual void			activate(void);
	virtual void			notifyActiveFrame(XAP_Frame *pFrame);
	virtual void			notifyCloseFrame(XAP_Frame *pFrame) {};

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	static void				autoupdateWC(UT_Timer * pTimer);
	virtual void			setUpdateCounter( UT_uint32 );
	virtual void			event_Update(void);
	static BOOL CALLBACK	s_dlgProc(HWND,UINT,WPARAM,LPARAM);
	void *					pGetWindowHandle( void ) { return (void*)m_hWnd; }
	
protected:
	BOOL					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	void 					_updateWindowData(void);       

	HWND                    m_hWnd;
	UT_Bool					m_bAutoWC;

	UT_Timer *				m_pAutoUpdateWC;
	UT_Bool					m_bDestroy_says_stopupdating;
	UT_Bool					m_bAutoUpdate_happening_now;
	UT_uint32				m_iUpdateRate;
};

#endif /* AP_WIN32DIALOG_WORDCOUNT_H */
