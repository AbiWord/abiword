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

#include <windows.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "xap_Dialog_Id.h"
#include "xap_Win32Dlg_FileOpenSaveAs.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

/*****************************************************************/
AP_Dialog * XAP_Win32Dialog_FileOpenSaveAs::static_constructor(AP_DialogFactory * pFactory,
															  AP_Dialog_Id id)
{
	XAP_Win32Dialog_FileOpenSaveAs * p = new XAP_Win32Dialog_FileOpenSaveAs(pFactory,id);
	return p;
}

XAP_Win32Dialog_FileOpenSaveAs::XAP_Win32Dialog_FileOpenSaveAs(AP_DialogFactory * pDlgFactory,
															 AP_Dialog_Id id)
	: XAP_Dialog_FileOpenSaveAs(pDlgFactory,id)
{
}

XAP_Win32Dialog_FileOpenSaveAs::~XAP_Win32Dialog_FileOpenSaveAs(void)
{
}

/*****************************************************************/

void XAP_Win32Dialog_FileOpenSaveAs::_buildFilterList(char * szFilter)
{
	char * p = szFilter;

	UT_ASSERT(UT_pointerArrayLength((void **) m_szSuffixes) ==
			  UT_pointerArrayLength((void **) m_szDescriptions));

	// measure one list, they should all be the same length
	UT_uint32 end = UT_pointerArrayLength((void **) m_szDescriptions);
	
	for (UT_uint32 i = 0; i < end; i++)
	{
		strcpy(p,m_szDescriptions[i]);
		p += strlen(p)+1;				// include the trailing 0
		strcpy(p,m_szSuffixes[i]);
		p += strlen(p)+1;				// include the trailing 0
	}

	// this will always be an option
	strcpy(p,"All (*.*)");
	p += strlen(p)+1;					// include the trailing 0
	strcpy(p,"*.*");
	p += strlen(p)+1;					// include the trailing 0
	
	*p = 0;								// double 0 at the end
}

/*****************************************************************/

void XAP_Win32Dialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	m_pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);
	UT_ASSERT(m_pWin32Frame);

	HWND hwnd = m_pWin32Frame->getTopLevelWindow();

	char szFile[1030];      // buffer for filename
	char szDir[1030];		// buffer for directory
	char szFilter[1030];	// buffer for building suffix list
	OPENFILENAME ofn;       // common dialog box structure

	ZeroMemory(szFile,sizeof(szFile));
	ZeroMemory(szDir,sizeof(szDir));
	ZeroMemory(szFilter,sizeof(szFilter));
	ZeroMemory(&ofn, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
			   
	// use the persistence info and/or the suggested filename
	// to properly seed the dialog.

	if (!m_szInitialPathname || !*m_szInitialPathname)
	{
		// the caller did not supply initial pathname
		// (or supplied an empty one).  see if we have
		// some persistent info.
		
		UT_ASSERT(!m_bSuggestName);
		if (m_szPersistPathname)
		{
			// we have a pathname from a previous use,
			// extract the directory portion and start
			// the dialog there (but without a filename).

			// use directory(m_szPersistPathname)
			strcpy(szDir,m_szPersistPathname);
			char * pLastSlash = strrchr(szDir, '\\');
			if (pLastSlash)
				pLastSlash[1] = 0;
			ofn.lpstrInitialDir = szDir;
		}
		else
		{
			// no initial pathname given and we don't have
			// a pathname from a previous use, so just let
			// it come up in the current working directory.
			// since we set OFN_NOCHANGEDIR we don't have to
			// to worry about where this is.
		}
	}
	else
	{
		// we have an initial pathname (the name of the document
		// in the frame that we were invoked on).  if the caller
		// wanted us to suggest a name, use the initial
		// pathname as is.  if not, use the directory portion of
		// it.  either way, we need to cut the pathname into two
		// parts -- directory and file -- for the common dlg.

		strcpy(szDir,m_szInitialPathname);
		char * pLastSlash = strrchr(szDir, '\\');
		if (pLastSlash)
			pLastSlash[1] = 0;
		ofn.lpstrInitialDir = szDir;
		
		if (m_bSuggestName)
		{
			if (pLastSlash)
				strcpy(szFile, m_szInitialPathname + (pLastSlash-szDir+1));
			else
				strcpy(szFile, m_szInitialPathname);
		}
	}
		
	// display the appropriate dialog box.

	BOOL bDialogResult;

	_buildFilterList(szFilter);
		
	switch (m_id)
	{
	case XAP_DIALOG_ID_FILE_OPEN:
		ofn.Flags |= OFN_FILEMUSTEXIST;
		bDialogResult = GetOpenFileName(&ofn);
		break;

	case XAP_DIALOG_ID_PRINTTOFILE:
		ofn.lpstrTitle = "Print To File";
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		bDialogResult = GetSaveFileName(&ofn);
		break;

	case XAP_DIALOG_ID_FILE_SAVEAS:
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		bDialogResult = GetSaveFileName(&ofn);
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	// TODO how do cancels get reported...
	// TODO verify that current-working-directory is not changed...
	
	if (bDialogResult != FALSE)
	{
		UT_cloneString(m_szFinalPathname,szFile);
		m_answer = a_OK;

		// set file type to auto-detect, since the Windows common
		// dialog doesn't let the user specifically choose to open
		// or save as a specific type (the type is always tied to
		// the suffix pattern)
		m_nFileType = XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO;
	}
	else
	{
		m_answer = a_CANCEL;
		UT_DEBUGMSG(("Didn't get a file: reason=0x%x\n", CommDlgExtendedError()));
	}
	
	m_pWin32Frame = NULL;
	return;
}

