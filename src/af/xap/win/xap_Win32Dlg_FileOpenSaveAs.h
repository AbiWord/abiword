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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef XAP_WIN32DIALOG_FILEOPENSAVEAS_H
#define XAP_WIN32DIALOG_FILEOPENSAVEAS_H

#include "xap_Dlg_FileOpenSaveAs.h"

class UT_String;
#include "xap_Frame.h"

#define DEFAULT_EXT_SIZE 15

/*****************************************************************/

class ABI_EXPORT XAP_Win32Dialog_FileOpenSaveAs : public XAP_Dialog_FileOpenSaveAs
{
public:
	XAP_Win32Dialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Win32Dialog_FileOpenSaveAs(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog * 	static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	static UINT CALLBACK	s_hookSaveAsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static UINT CALLBACK	s_hookInsertPicProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
			
protected:
	UINT _previewPicture(HWND hwnd);
	UINT _initPreviewDlg(HWND hwnd);

	void _buildFilterList(UT_String& sFilter);
private:
	char * _getDefaultExtension(UT_uint32 indx);
	char m_szDefaultExtension[DEFAULT_EXT_SIZE + 1];
	
	//
	// This the new OPENFILENAME struct included in the most
	// recent Plataforms SDK.  
	//
	typedef struct OPENFILENAME_WIN50 
	{			
			DWORD         lStructSize; 
			HWND          hwndOwner; 
			HINSTANCE     hInstance; 
			LPCTSTR       lpstrFilter; 
			LPTSTR        lpstrCustomFilter; 
			DWORD         nMaxCustFilter; 
			DWORD         nFilterIndex; 
			LPTSTR        lpstrFile; 
			DWORD         nMaxFile; 
			LPTSTR        lpstrFileTitle; 
			DWORD         nMaxFileTitle; 
			LPCTSTR       lpstrInitialDir; 
			LPCTSTR       lpstrTitle; 
			DWORD         Flags; 
			WORD          nFileOffset; 
			WORD          nFileExtension; 
			LPCTSTR       lpstrDefExt; 
			LPARAM        lCustData; 
			LPOFNHOOKPROC lpfnHook; 
			LPCTSTR       lpTemplateName; 
			
			//#if (_WIN32_WINNT >= 0x0500)			
			void *        pvReserved;
			DWORD         dwReserved;
			DWORD         FlagsEx;
			//#endif // (_WIN32_WINNT >= 0x0500)		
	};
	
	BOOL GetSaveFileName_Hooked(OPENFILENAME_WIN50* lpofn,  BOOL bSave);
	
};

#endif /* XAP_WIN32DIALOG_FILEOPENSAVEAS_H */
