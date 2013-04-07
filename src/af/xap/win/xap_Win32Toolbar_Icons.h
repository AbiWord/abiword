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

#ifndef XAP_WIN32TOOLBARICONS_H
#define XAP_WIN32TOOLBARICONS_H

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_color.h"
#include "xap_Toolbar_Icons.h"
#include <windows.h>

/*****************************************************************/

class ABI_EXPORT XAP_Win32Toolbar_Icons : public XAP_Toolbar_Icons
{
public:
	XAP_Win32Toolbar_Icons(void);
	~XAP_Win32Toolbar_Icons(void);

#if defined (EXPORT_XPM_TO_BMP)
	static bool getBitmapForIconFromXPM(HWND hwnd,
									UT_uint32 maxWidth,
									UT_uint32 maxHeight,
									UT_RGBColor * pColor,
									const char * szIconName,
									HBITMAP * pBitmap);

	static bool saveBitmap (const char *szFilename);

#endif

	static bool getBitmapForIcon(HWND hwnd,
									UT_uint32 maxWidth,
									UT_uint32 maxHeight,
									const UT_RGBColor * pColor,
									const char * szIconName,
									HBITMAP * pBitmap);

	static bool getAlphaBitmapForIcon(HWND hwnd,
									UT_uint32 maxWidth,
									UT_uint32 maxHeight,
									const char * szIconName,
									HBITMAP * pBitmap);
};

#endif /* XAP_WIN32TOOLBARICONS_H */
