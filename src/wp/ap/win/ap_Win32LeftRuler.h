/* AbiWord
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

#ifndef AP_WIN32LEFTRULER_H
#define AP_WIN32LEFTRULER_H

// Class for dealing with the horizontal ruler at the top of
// a document window.

/*****************************************************************/

#include "ut_types.h"
#include "ap_LeftRuler.h"

class XAP_Win32App;
class XAP_Frame;

/*****************************************************************/

class ABI_EXPORT AP_Win32LeftRuler : public AP_LeftRuler
{
public:
	AP_Win32LeftRuler(XAP_Frame * pFrame);
	virtual ~AP_Win32LeftRuler(void);

	HWND					createWindow(HWND hwndContainer,
										 UT_uint32 left, UT_uint32 top,
										 UT_uint32 height);
	virtual void			setView(AV_View * pView);

	static bool			registerClass(XAP_Win32App * app);
	static LRESULT CALLBACK	_LeftRulerWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

protected:
	HWND					m_hwndLeftRuler;
};

#endif /* AP_WIN32LEFTRULER_H */
