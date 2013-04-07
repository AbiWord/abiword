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


#ifndef XAP_WIN32DROPTARGET_H
#define XAP_WIN32DROPTARGET_H

#include <stdlib.h>
#include <windows.h>
#include <commctrl.h>   // includes the common control header
#ifndef __MINGW32__
#include <crtdbg.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <ole2.h>

#include "xap_Frame.h"


/*****************************************************************
******************************************************************
** XAP_Win32DropTarget implements the OLE 2 Drop functionality in
** AbiWord
******************************************************************
*****************************************************************/

interface ABI_EXPORT XAP_Win32DropTarget : public IDropTarget
{

public:

	XAP_Win32DropTarget();
	virtual ~XAP_Win32DropTarget() {}

	// Ole Methods
	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppv);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();


	STDMETHODIMP DragEnter (LPDATAOBJECT pDataObj, DWORD grfKeyState,
			POINTL pt, LPDWORD pdwEffect);
	STDMETHODIMP DragOver  (DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	STDMETHODIMP DragLeave ();
	STDMETHODIMP Drop (LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt,
			LPDWORD pdwEffect);

	// Helper
	void setFrame(XAP_Frame* pFrame) {m_pFrame = pFrame;};
private:

	int   			m_nCount;                 // reference count
	UINT			m_uCF_RTF;
	bool			m_bSupportedFormat;
	XAP_Frame*		m_pFrame;


};


#endif
