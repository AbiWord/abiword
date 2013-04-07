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

#ifndef AP_WIN32TOPRULER_H
#define AP_WIN32TOPRULER_H

// Class for dealing with the horizontal ruler at the top of
// a document window.

/*****************************************************************/

#include "ut_types.h"
#include "ap_TopRuler.h"
#include "xap_Win32App.h"
#include "xap_Frame.h"


/*****************************************************************/

class ABI_EXPORT AP_Win32TopRuler : public AP_TopRuler
{
public:
	AP_Win32TopRuler(XAP_Frame * pFrame);
	virtual ~AP_Win32TopRuler(void);

	HWND					createWindow(HWND hwndContainer,
										 UT_uint32 left, UT_uint32 top,
										 UT_uint32 width);
	virtual void			setView(AV_View * pView);

	static bool				registerClass(XAP_Win32App * app);
	static LRESULT CALLBACK		_TopRulerWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

protected:
	HWND					m_hwndTopRuler;
};

#endif /* AP_WIN32TOPRULER_H */
