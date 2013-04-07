/* AbiSource Program Utilities
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

#ifndef EV_WIN32MENU_H
#define EV_WIN32MENU_H

#include <windows.h>
#include "ut_types.h"
#include "xap_Types.h"
#include "ev_Menu.h"
#include "ut_color.h"
#include "ut_misc.h"
#include <vector>
//#include "ap_Win32FrameImpl.h"

class AV_View;
class XAP_Win32App;
class EV_EditEventMapper;
class XAP_Frame;

/*****************************************************************/

typedef struct
{
	XAP_Menu_Id 	id;					// Menu ID
	char			szName[255];		// BitmapName
} EV_Menu_Bitmap;


class ABI_EXPORT EV_Win32Menu : public EV_Menu
{
public:
	EV_Win32Menu(XAP_Win32App * pWin32App,
				 const EV_EditEventMapper * pEEM,
				 const char * szMenuLayoutName,
				 const char * szMenuLabelSetName);
	~EV_Win32Menu();

	void				destroy();

	bool				synthesizeMenu(XAP_Frame * pFrame, HMENU menuRoot);
	bool				onCommand(AV_View * pView, HWND hWnd, WPARAM wParam);
	bool				onInitMenu(XAP_Frame * pFrame, AV_View * pView, HWND hWnd, HMENU hMenuBar);
	bool				onMenuSelect(XAP_Frame * pFrame, AV_View * pView,
									 HWND hWnd, HMENU hMenu, WPARAM wParam);

	HMENU				getMenuHandle() const				{ return m_myMenu; }
	XAP_Menu_Id			MenuIdFromWmCommand(UINT cmd)		{ return (XAP_Menu_Id)(cmd - WM_USER); }
	UINT				WmCommandFromMenuId(XAP_Menu_Id id)	{ return (id + WM_USER); }

	virtual bool		_doAddMenuItem(UT_uint32 /*id*/) { UT_ASSERT_HARMLESS(UT_TODO); return false;/* TODO */ }
	void				onDrawItem(HWND hwnd, WPARAM wParam, LPARAM lParam);
	void				onMeasureItem(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LPARAM				onMenuChar(HWND hwnd, WPARAM wParam, LPARAM lParam);
	void				setTrackMenu(bool bTrack){m_bTrack=bTrack;};

protected:

	static bool					_isAMenuBar(XAP_Menu_Id	id,HMENU hMenu);
	static HBITMAP				_loadBitmap(XAP_Menu_Id id, int width, int height, const UT_RGBColor & color);
	void		 				_setBitmapforID(HMENU hMenu, XAP_Menu_Id id, UINT cmd);

	XAP_Win32App *				m_pWin32App;
	const EV_EditEventMapper *	m_pEEM;
	EV_Menu_Bitmap*				m_pArMenuBitmaps;
	HMENU						m_myMenu;
	UINT						m_nBitmapCX, m_nBitmapCY;
	HFONT						m_hFont;
	UT_Vector					m_vecItems;
	bool						m_bTrack;
	UINT                        m_iDIR;
	std::vector <HBITMAP>		m_vechBitmaps;

};


typedef struct
{
	XAP_Menu_Id 	id;					// Menu ID
	wchar_t			szText[256];		// Text
	EV_Win32Menu*	pMenu;
} EV_Menu_Item;

/*****************************************************************/

class ABI_EXPORT EV_Win32MenuBar : public EV_Win32Menu
{
public:
	EV_Win32MenuBar(XAP_Win32App * pWin32App,
					const EV_EditEventMapper * pEEM,
					const char * szMenuLayoutName,
					const char * szMenuLabelSetName);
	~EV_Win32MenuBar();

	bool				synthesizeMenuBar(XAP_Frame * pFrame);
};

/*****************************************************************/

class ABI_EXPORT EV_Win32MenuPopup : public EV_Win32Menu
{
public:
	EV_Win32MenuPopup(XAP_Win32App * pWin32App,
					  const char * szMenuLayoutName,
					  const char * szMenuLabelSetName);
	~EV_Win32MenuPopup();

	bool				synthesizeMenuPopup(XAP_Frame * pFrame);
};

#endif /* EV_WIN32MENU_H */
