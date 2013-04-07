/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#ifndef AP_Win32Dialog_InsertTable_H
#define AP_Win32Dialog_InsertTable_H

#include "ap_Dialog_InsertTable.h"
#include "xap_Win32DialogBase.h"

class ABI_EXPORT AP_Win32Dialog_InsertTable : public AP_Dialog_InsertTable, public XAP_Win32DialogBase
{
public:

	AP_Win32Dialog_InsertTable (XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_InsertTable (void);

	virtual void				runModal(XAP_Frame * pFrame);
	static XAP_Dialog *			static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	BOOL 						_onDeltaPos(NM_UPDOWN * pnmud);

protected:

	BOOL						_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL						_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	void 						getCtrlValues(void);
};

#endif

