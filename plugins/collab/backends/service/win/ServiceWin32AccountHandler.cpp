/* Copyright (C) 2008 AbiSource Corporation B.V.
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

#include "xap_App.h"
#include "ServiceWin32AccountHandler.h"

#define ABI_RID_DIALOG_COLLABSERVICE_EMAILENTRY 201
#define ABI_RID_DIALOG_COLLABSERVICE_EMAILLABEL 202
#define ABI_RID_DIALOG_COLLABSERVICE_PASSWORDENTRY 203
#define ABI_RID_DIALOG_COLLABSERVICE_PASSWORDLABEL 204
#define ABI_RID_DIALOG_COLLABSERVICE_URLBUTTON 205

AccountHandlerConstructor ServiceAccountHandlerConstructor = &ServiceWin32AccountHandler::static_constructor;

AccountHandler * ServiceWin32AccountHandler::static_constructor()
{
	return static_cast<AccountHandler *>(new ServiceWin32AccountHandler());
}

ServiceWin32AccountHandler::ServiceWin32AccountHandler()
	: ServiceAccountHandler(),
	m_hInstance(NULL),	
	m_hEmailEntry(NULL),
	m_hEmailLabel(NULL),
	m_hPasswordEntry(NULL),
	m_hPasswordLabel(NULL),
	m_hUrlButton(NULL)
{
	AbiCollabSessionManager * pSessionManager= AbiCollabSessionManager::getManager();
	if (pSessionManager)
	{
		m_hInstance = pSessionManager->getInstance();
	}
}

void ServiceWin32AccountHandler::embedDialogWidgets(void* pEmbeddingParent)
{
	UT_DEBUGMSG(("ServiceWin32AccountHandler::embedDialogWidgets()\n"));
	UT_return_if_fail(pEmbeddingParent);

	// our enclosing window must be a HWND
	HWND hBox = reinterpret_cast<HWND>(pEmbeddingParent);
	
	/* Non-Tabbable Labels */

	m_hEmailLabel = CreateWindowExW(WS_EX_NOPARENTNOTIFY, L"STATIC", L"E-mail address:", SS_LEFT | WS_CHILD | WS_VISIBLE,
			15, 20, 51, 15, hBox,  (HMENU) ABI_RID_DIALOG_COLLABSERVICE_EMAILLABEL,  m_hInstance, 0);
	UT_return_if_fail(m_hEmailLabel);

	m_hPasswordLabel = CreateWindowExW(WS_EX_NOPARENTNOTIFY, L"STATIC", L"Password:", SS_LEFT | WS_CHILD | WS_VISIBLE,
			15, 40, 51, 15, hBox,  (HMENU) ABI_RID_DIALOG_COLLABSERVICE_PASSWORDLABEL,  m_hInstance, 0);
	UT_return_if_fail(m_hPasswordLabel);
	
	/* Tabbable */
	m_hEmailEntry = CreateWindowExW(WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE, L"EDIT", L"", ES_AUTOHSCROLL | ES_LEFT | WS_CHILD | WS_BORDER | WS_VISIBLE | WS_TABSTOP,
			80, 20, 121, 20, hBox,  (HMENU) ABI_RID_DIALOG_COLLABSERVICE_EMAILENTRY,  m_hInstance, 0);
	UT_return_if_fail(m_hEmailEntry);
	SendMessage(m_hEmailEntry, EM_SETLIMITTEXT, 255*sizeof(TCHAR), 0);

	m_hPasswordEntry = CreateWindowExW(WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE, L"EDIT", L"", ES_AUTOHSCROLL | ES_LEFT | WS_CHILD | WS_BORDER | WS_VISIBLE | WS_TABSTOP,
			80, 40, 121, 20, hBox,  (HMENU) ABI_RID_DIALOG_COLLABSERVICE_PASSWORDENTRY,  m_hInstance, 0);
	UT_return_if_fail(m_hPasswordEntry);
	SendMessage(m_hPasswordEntry, EM_SETPASSWORDCHAR, '*', 0);
	SendMessage(m_hPasswordEntry, EM_SETLIMITTEXT, 255*sizeof(TCHAR), 0);

	m_hUrlButton = CreateWindowExW(WS_EX_NOPARENTNOTIFY, L"BUTTON", L"Get a free abicollab.net account", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
			15, 64, 186, 20, hBox, (HMENU) ABI_RID_DIALOG_COLLABSERVICE_URLBUTTON, m_hInstance, 0);

	// Font setting code borrowed from XAP_Win32Dlg_About
	LOGFONTW lf = { 0 };
	wcscpy(lf.lfFaceName, L"MS Sans Serif");
	lf.lfHeight = 12;
	lf.lfWeight = 0;
	HFONT hfontPrimary = CreateFontIndirectW(&lf);
	HWND rgFontReceivers[] =
		{ m_hEmailLabel, m_hEmailEntry, m_hPasswordLabel, m_hPasswordEntry, m_hUrlButton};
	for (UT_uint32 iWnd = 0; iWnd < G_N_ELEMENTS(rgFontReceivers); iWnd++)
		SendMessage(rgFontReceivers[iWnd], WM_SETFONT, (WPARAM) hfontPrimary, 0);
}

#define DESTROY_WINDOW(M) if (M) { int res = DestroyWindow(M); UT_ASSERT_HARMLESS(res); M = 0; }

void ServiceWin32AccountHandler::removeDialogWidgets(void* pEmbeddingParent)
{
	UT_DEBUGMSG(("ServiceWin32AccountHandler::removeDialogWidgets()\n"));
	UT_return_if_fail(pEmbeddingParent);

	// our enclosing window must be a HWND
	HWND hBox = reinterpret_cast<HWND>(pEmbeddingParent);
	
	DESTROY_WINDOW(m_hEmailLabel);
	DESTROY_WINDOW(m_hEmailEntry);
	DESTROY_WINDOW(m_hPasswordLabel);
	DESTROY_WINDOW(m_hPasswordEntry);
	DESTROY_WINDOW(m_hUrlButton);
	
	// destroying a window does not repaint the area it owned, so do it here
	RedrawWindow(GetParent(hBox), NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);	
}

#define READ_STRING(E, S) std::string S(255*sizeof(TCHAR), ' '); S.resize(SendMessage(E, WM_GETTEXT, S.size()-1, (LPARAM)&S[0]));

void ServiceWin32AccountHandler::loadProperties()
{
	UT_DEBUGMSG(("ServiceWin32AccountHandler::loadProperties()\n"));	

	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
}

void ServiceWin32AccountHandler::storeProperties()
{
	UT_DEBUGMSG(("ServiceWin32AccountHandler::storeProperties()\n"));	

	UT_return_if_fail(m_hEmailEntry || m_hPasswordEntry);

	addProperty("uri", "https://abicollab.net/soap/");
	
	READ_STRING(m_hEmailEntry, email);
	addProperty("email", email);

	READ_STRING(m_hPasswordEntry, password);
	addProperty("password", password);

	// TODO: implement autoconnect
	addProperty("autoconnect", "true" );
}

// return true if we process the command, false otherwise
BOOL ServiceWin32AccountHandler::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
		case ABI_RID_DIALOG_COLLABSERVICE_URLBUTTON:
			XAP_App::getApp()->openURL(SERVICE_REGISTRATION_URL);
			return true;
	}

	return false;
}

bool ServiceWin32AccountHandler::shouldProcessFocus()
{
	return GetFocus() != m_hUrlButton;
}
