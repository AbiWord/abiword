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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef XAP_WIN32DIALOG_ZOOM_H
#define XAP_WIN32DIALOG_ZOOM_H

#include <commctrl.h>
#include "xap_Dlg_Zoom.h"
#include "xap_Win32DialogBase.h"

class XAP_Frame;

/*****************************************************************/

class ABI_EXPORT XAP_Win32Dialog_Zoom: public XAP_Win32DialogBase, public XAP_Dialog_Zoom
{
public:
	XAP_Win32Dialog_Zoom(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Win32Dialog_Zoom(void);

	virtual void 		runModal(XAP_Frame * pFrame);

	static XAP_Dialog *	static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

protected:
	BOOL _onDlgMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL _onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL _onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL _onDeltaPos(NM_UPDOWN * pnmud);

	BOOL _getValueFromEditPct(int * pNewValue);

	bool							m_bEditPctEnabled;		// is edit-pct-box enabled (true iff radio_pct)
};

#endif /* XAP_WIN32DIALOG_ZOOM_H */
