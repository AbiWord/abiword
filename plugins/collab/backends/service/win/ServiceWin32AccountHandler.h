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

#ifndef __SERVICEWIN32ACCOUNTHANDLER__
#define __SERVICEWIN32ACCOUNTHANDLER__

#include <backends/service/xp/ServiceAccountHandler.h>

class XAP_Frame;

class ServiceWin32AccountHandler : public ServiceAccountHandler
{
public:
	ServiceWin32AccountHandler();

	static AccountHandler*					static_constructor();

	// dialog management 
	virtual void							embedDialogWidgets(void* pEmbeddingParent);
	virtual void							removeDialogWidgets(void* pEmbeddingParent);
	virtual void							loadProperties();
	virtual void							storeProperties();
	virtual BOOL							_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	virtual bool							shouldProcessFocus();

private:
	// handles
	HINSTANCE								m_hInstance;	
	HWND									m_hEmailEntry;
	HWND									m_hEmailLabel;
	HWND									m_hPasswordEntry;
	HWND									m_hPasswordLabel;
	HWND									m_hUrlButton;
};

#endif /* __SERVICEWIN32ACCOUNTHANDLER__ */
