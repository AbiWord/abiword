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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_WIN32DIALOG_WORDCOUNT_H
#define AP_WIN32DIALOG_WORDCOUNT_H

#include <commctrl.h>
#include "ap_Dialog_WordCount.h"
#include "ut_timer.h"
#include "xap_Frame.h"
#include "xap_Win32DialogBase.h"

/*****************************************************************/

class ABI_EXPORT AP_Win32Dialog_WordCount: public AP_Dialog_WordCount, public XAP_Win32DialogBase
{
public:
	AP_Win32Dialog_WordCount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_WordCount(void);

	virtual void			runModal(XAP_Frame * pFrame);
	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			destroy(void);
	virtual void			activate(void);
	virtual void			notifyActiveFrame(XAP_Frame *pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	static void				autoupdateWC(UT_Worker * pTimer);
	virtual void			setUpdateCounter( UT_uint32 );
	virtual void			event_Update(void);
	void *					pGetWindowHandle( void ) { return (void*)m_hDlg; }
	virtual BOOL 			_onDlgMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	BOOL					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void 					_setDlgItemInt(UINT nCtrl, int nValue);
	void 					_updateWindowData(void);

	bool					m_bAutoWC;

	UT_Timer *				m_pAutoUpdateWC;
	bool					m_bDestroy_says_stopupdating;
	bool					m_bAutoUpdate_happening_now;
	UT_uint32				m_iUpdateRate;
};

#endif /* AP_WIN32DIALOG_WORDCOUNT_H */
