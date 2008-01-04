/* AbiCollab - Code to enable the modification of remote documents.
 * Copyright (C) 2007 by Ryan Pavlik <abiryan@ryand.net>
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

#ifndef __TCPWIN32ACCOUNTHANDLER__
#define __TCPWIN32ACCOUNTHANDLER__

#include <string>
#include <ap_Win32App.h>
#include "xap_Win32DialogHelper.h"

#include <core/session/xp/AbiCollabSessionManager.h>
#include <core/dialogs/win/ap_Win32Dialog_CollaborationAddAccount.h>

#include <backends/tcp/xp/TCPAccountHandler.h>

#define ABI_RID_DIALOG_COLLABTCP_SERVERENTRY 201
#define ABI_RID_DIALOG_COLLABTCP_PORTENTRY 202
#define ABI_RID_DIALOG_COLLABTCP_SERVERLABEL 203
#define ABI_RID_DIALOG_COLLABTCP_PORTLABEL 204
#define ABI_RID_DIALOG_COLLABTCP_SERVERRADIO 205
#define ABI_RID_DIALOG_COLLABTCP_JOINRADIO 206
#define ABI_RID_DIALOG_COLLABTCP_AUTOCONNECTCHECK 207
#define ABI_RID_DIALOG_COLLABTCP_USESECURECHECK 208


class ABI_EXPORT TCPWin32AccountHandler : public TCPAccountHandler
{
public:
	TCPWin32AccountHandler();
	
	static AccountHandler*					static_constructor();

	// dialog management 
	virtual void							embedDialogWidgets(void* pEmbeddingParent);
	virtual void							removeDialogWidgets(void* pEmbeddingParent);
	virtual void							storeProperties();
	BOOL									_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	
private:
	XAP_Win32DialogHelper *					p_win32Dialog;
	
	bool									m_bEmbedded;
	
	// handles
	HINSTANCE								m_hInstance;
	HWND									m_hBox;
	HWND									m_hServerEntry;
	HWND									m_hPortEntry;
	HWND									m_hServerRadio;
	HWND									m_hJoinRadio;
	HWND									m_hServerLabel;
	HWND									m_hPortLabel;
	HWND									m_hAutoconnectCheck;
	HWND									m_hUseSecureCheck;
	
	HWND									m_hParentDlg;
	
	void									_checkButtonHwnd(HWND hCtrl, bool bChecked);
	bool									_isCheckedHwnd(HWND hCtrl);
	int										_getControlTextHwnd(HWND hCtrl, int iLen, const char * p_szBuf);


};

#endif /* __TCPWin32ACCOUNTHANDLER__ */
