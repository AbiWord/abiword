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

#include <stdio.h>
#include <windows.h>

#include "ut_png.h"
#include "ut_svg.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "ut_debugmsg.h"
#include "ut_string_class.h"
#include "ut_Win32LocaleString.h"

#include "xap_App.h"
#include "xap_Strings.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_FileOpenSaveAs.h"
#include "xap_Win32Dlg_FileOpenSaveAs.h"

#include "gr_Win32Image.h"
#include "gr_Win32Graphics.h"
#include "ap_Win32App.h"

#include "ie_types.h"
#include "ie_imp.h"
#include "ie_impGraphic.h"

#include "xap_Win32Resources.rc2"


#define MAX_DLG_INS_PICT_STRING 1030
/*****************************************************************/
XAP_Dialog * XAP_Win32Dialog_FileOpenSaveAs::static_constructor(XAP_DialogFactory * pFactory,
															  XAP_Dialog_Id id)
{
	XAP_Win32Dialog_FileOpenSaveAs * p = new XAP_Win32Dialog_FileOpenSaveAs(pFactory,id);
	return p;
}

XAP_Win32Dialog_FileOpenSaveAs::XAP_Win32Dialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory,
															 XAP_Dialog_Id id)
	: XAP_Dialog_FileOpenSaveAs(pDlgFactory,id)
{
	m_szDefaultExtension[0] = 0;
}

XAP_Win32Dialog_FileOpenSaveAs::~XAP_Win32Dialog_FileOpenSaveAs(void)
{
}

/*****************************************************************/

/*!
 Check for the presence of a file suffix in a list of suffixes

 \param haystack List of suffixes in the form "*a; *b; *c"
 \param needle Suffix to check for
 */
static bool SuffixInList(const char *haystack, const char *needle)
{
	int l = strlen(needle);
	const char *s = strstr(haystack, needle);
	if (s)
		if (s[l] == 0 || s[l] == ';')
			return true;
	return false;
}

/*!
 Retrieve the default extension to use if user does not provide one

 \param indx -- index into the filter list
 */
wchar_t * XAP_Win32Dialog_FileOpenSaveAs::_getDefaultExtension(UT_uint32 indx)
{
	static wchar_t abw_sfx[] = L"abw";
    char defaultExtension[DEFAULT_EXT_SIZE + 1];
	UT_Win32LocaleString def_ext;
	
	UT_uint32 end = g_strv_length((gchar **) m_szDescriptions);
	if(indx >= end)
		return abw_sfx;
	
	// copy at most DEFAULT_EXT_SIZE characters from the suffix;
	strncpy(defaultExtension, m_szSuffixes[indx] + 2, DEFAULT_EXT_SIZE);
	
	defaultExtension[DEFAULT_EXT_SIZE] = 0;
	
	// make sure that we get rid off the semicolon if it got copied
	char * semicolon = strchr(defaultExtension, ';');
	if(semicolon)
		*semicolon = 0;
	UT_DEBUGMSG(("Default sfx [%s], (from [%s]\n", m_szDefaultExtension,m_szSuffixes[indx]));
    def_ext.fromASCII (defaultExtension);
	wcscpy (m_szDefaultExtension, def_ext.c_str());
	return m_szDefaultExtension;
}

/*!
 Build a Windows filter list from Abi's filetypes

 \param sFilter Build the list in this string
 */
void XAP_Win32Dialog_FileOpenSaveAs::_buildFilterList(UT_String& sFilter)
{
	UT_String sAllSuffixes;

	const XAP_StringSet*  pSS	= XAP_App::getApp()->getStringSet();

	UT_ASSERT(g_strv_length((gchar **) m_szSuffixes) == g_strv_length((gchar **) m_szDescriptions));

	// measure one list, they should all be the same length
	UT_uint32 end = g_strv_length((gchar **) m_szDescriptions);

	for (UT_uint32 i = 0; i < end; i++)
	{
		sFilter += m_szDescriptions[i];
		sFilter += '\0';				// include the trailing 0
		sFilter += m_szSuffixes[i];
		sFilter += '\0';				// include the trailing 0

		// extract one suffix at a time
		const char *s1 = m_szSuffixes[i];
		while (1)
		{
			const char *s2 = s1;
			while (*s2 && *s2 != ';')
				++s2;
			UT_String sSuffix(s1, s2-s1);
			// build a complete list with no repeats
			if (!SuffixInList(sAllSuffixes.c_str(), sSuffix.c_str()))
			{
				if (!sAllSuffixes.empty())
					sAllSuffixes += "; ";
				sAllSuffixes += sSuffix;
			}
			if (*s2 == 0)
				break;
			UT_ASSERT(s2[0] == ';' && s2[1] == ' ');
			s1 = s2 + 2;
		}
	}

	// all supported files filter
	sFilter += m_id == XAP_DIALOG_ID_INSERT_PICTURE ? pSS->getValue(XAP_STRING_ID_DLG_FOSA_ALLIMAGES) : pSS->getValue(XAP_STRING_ID_DLG_FOSA_ALLDOCS);
	sFilter += " (" + sAllSuffixes + ")";
	sFilter += '\0';				// include the trailing 0
	sFilter += sAllSuffixes;
	sFilter += '\0';				// include the trailing 0

	// all files filter
	sFilter += pSS->getValue(XAP_STRING_ID_DLG_FOSA_ALL);
	sFilter += '\0';				// include the trailing 0
	sFilter += "*.*";
	sFilter += '\0';				// include the trailing 0
	
	sFilter += '\0';				// double 0 at the end
}

/*****************************************************************/

/*!  
  Gets the Windows File/Save common dialog box. To be able to get
  the places bar containing icons for commonly-used folders when we
  use hooking we need to use the new OPENFILENAME structure as defined
  in WINNT5 or better. To keep backward compability, first try
  with the new size of the structure, if it does not work it means the
  the common dialog box DLL installed in the system does not support
  the new features, and then we use the old one. In this situation, we
  user does not have a places bar capable common dialog box DLL
  anyway.
*/
BOOL  XAP_Win32Dialog_FileOpenSaveAs::GetSaveFileName_Hooked(OPENFILENAME_WIN50* lpofn, BOOL bSave)
{
		BOOL  bRslt;	
		
		lpofn->lStructSize = sizeof(OPENFILENAME_WIN50);		// Size of the new structure
		lpofn->pvReserved = NULL;
		lpofn->dwReserved = 0;
		lpofn->FlagsEx = 0;		
		
		if (bSave)				
			bRslt = GetSaveFileNameW((OPENFILENAMEW *)lpofn);
		else
			bRslt = GetOpenFileNameW((OPENFILENAMEW *)lpofn);
		
		if (!bRslt) // Error
		{			
			DWORD dwError = CommDlgExtendedError();
			
			if (dwError==CDERR_STRUCTSIZE)	// This system does not support the place bar
			{								
					//  Try with the old one
					lpofn->lStructSize = sizeof(OPENFILENAMEW);	
					if (bSave)				
						bRslt = GetSaveFileNameW((OPENFILENAMEW *)lpofn);
					else
						bRslt = GetOpenFileNameW((OPENFILENAMEW *)lpofn);
				
			}				
		}	
		
		return bRslt;
}


/*****************************************************************/

void XAP_Win32Dialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	UT_return_if_fail(pFrame->getFrameImpl());

	XAP_Win32App* pWin32App = static_cast<XAP_Win32App*>(XAP_App::getApp());

	HWND hFrame = static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow();

	wchar_t szFile[MAX_DLG_INS_PICT_STRING];	// buffer for filename
	wchar_t szDir[MAX_DLG_INS_PICT_STRING];	// buffer for directory
	UT_String sFilter;
	OPENFILENAME_WIN50 ofn;						// common dialog box structure
    UT_Win32LocaleString filter;

	ZeroMemory(szFile,sizeof(szFile));
	ZeroMemory(szDir,sizeof(szDir));
	ZeroMemory(&ofn, sizeof(OPENFILENAME_WIN50));

	_buildFilterList(sFilter);
    filter.fromASCII (sFilter.c_str(), sFilter.size());

	ofn.lStructSize = sizeof(OPENFILENAMEW);		// Old size
	ofn.hwndOwner = hFrame;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter.c_str();
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = _getDefaultExtension(0);
	ofn.lCustData = (DWORD)this;
	// use the persistence info and/or the suggested filename
	// to properly seed the dialog.

	if (!m_szInitialPathname || !*m_szInitialPathname)
	{
		// the caller did not supply initial pathname
		// (or supplied an empty one).	see if we have
		// some persistent info.

		UT_ASSERT(!m_bSuggestName);
		if (m_szPersistPathname)
		{
			// we have a pathname from a previous use,
			// extract the directory portion and start
			// the dialog there (but without a filename).

			// use directory(m_szPersistPathname)

            UT_Win32LocaleString path;
			path.fromUTF8 (m_szPersistPathname);

			wcscpy(szDir,path.c_str());
			wchar_t * pLastSlash = wcsrchr(szDir, L'/');
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
		// pathname as is.	if not, use the directory portion of
		// it.	either way, we need to cut the pathname into two
		// parts -- directory and file -- for the common dlg.

        UT_Win32LocaleString uri;
		const char * szURI = g_filename_from_uri(m_szInitialPathname, NULL, NULL);
		if(!szURI)
			szURI = static_cast<const char *> (UT_calloc (1, sizeof (char)));

        uri.fromUTF8 (szURI);

		wcscpy(szDir,uri.c_str());
		wchar_t * pLastSlash = wcsrchr(szDir, L'/');
		if (pLastSlash)
			pLastSlash[1] = 0;
		ofn.lpstrInitialDir = szDir;

		if (m_bSuggestName)
		{
			if (pLastSlash)
				wcscpy(szFile, uri.c_str() + (pLastSlash-szDir+1));
			else
				wcscpy(szFile, uri.c_str());

			// if the file name has an extension, remove it
			// (if we don't, and the document is of a different
			// type than the one initially selected in the dialogue
			// and the user just clicks OK, we get type - extension
			// mismatch)
			wchar_t * dot = wcsrchr(szFile, L'.');
			if(dot)
				*dot = 0;
		}

		FREEP(szURI);
	}

	// display the appropriate dialog box.

	BOOL bDialogResult;

	if( m_nDefaultFileType != XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO )
	{
		// Find the index of the type we were given
		UT_uint32 iCounter, iNumTypes = g_strv_length((gchar **) m_nTypeList);

		for( iCounter = 0; iCounter < iNumTypes; iCounter++ )
		{
			if( m_nTypeList[ iCounter ] == m_nDefaultFileType )
			{
				ofn.nFilterIndex = iCounter + 1;
				break;
			}
		}
	}

	const XAP_StringSet* pSS = XAP_App::getApp()->getStringSet();
    UT_Win32LocaleString title;

	switch (m_id)
	{
	case XAP_DIALOG_ID_FILE_OPEN:		
		title.fromUTF8 (pSS->getValue(XAP_STRING_ID_DLG_FOSA_OpenTitle));
		ofn.lpstrTitle = title.c_str();
		ofn.Flags |= OFN_FILEMUSTEXIST;
		ofn.nFilterIndex = g_strv_length((gchar **) m_szDescriptions) + 1;
		bDialogResult = GetOpenFileNameW((OPENFILENAMEW *)&ofn);
		break;

	case XAP_DIALOG_ID_PRINTTOFILE:
	    title.fromUTF8 (pSS->getValue(XAP_STRING_ID_DLG_FOSA_PrintToFileTitle));
		ofn.lpstrTitle = title.c_str();	
        ofn.Flags |= OFN_OVERWRITEPROMPT;
		bDialogResult = GetSaveFileNameW((OPENFILENAMEW *)&ofn);
		break;

	case XAP_DIALOG_ID_FILE_SAVEAS:
		title.fromUTF8 (pSS->getValue(XAP_STRING_ID_DLG_FOSA_SaveAsTitle));
		ofn.lpstrTitle = title.c_str();
		ofn.lpfnHook	   = (LPOFNHOOKPROC) s_hookSaveAsProc;
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		ofn.Flags |= OFN_EXPLORER;
		ofn.Flags |= OFN_ENABLEHOOK;
				
		bDialogResult = GetSaveFileName_Hooked(&ofn, TRUE);
		break;

	case XAP_DIALOG_ID_INSERT_PICTURE:
		title.fromUTF8 (pSS->getValue(XAP_STRING_ID_DLG_IP_Title));
		ofn.lpstrTitle = title.c_str();
		ofn.hInstance	   = pWin32App->getInstance();
		ofn.lpTemplateName = MAKEINTRESOURCEW(XAP_RID_DIALOG_INSERT_PICTURE);
		ofn.lpfnHook	   = (LPOFNHOOKPROC) s_hookInsertPicProc;
		ofn.nFilterIndex   = g_strv_length((gchar **) m_szDescriptions) + 1;
		ofn.Flags |= OFN_EXPLORER;
		ofn.Flags |= OFN_ENABLETEMPLATE;
		ofn.Flags |= OFN_ENABLEHOOK;
		bDialogResult = GetSaveFileName_Hooked(&ofn, FALSE);
		break;

	case XAP_DIALOG_ID_FILE_IMPORT:
        title.fromUTF8 (pSS->getValue(XAP_STRING_ID_DLG_FOSA_ImportTitle));
		ofn.lpstrTitle = title.c_str();
		ofn.nFilterIndex = g_strv_length((gchar **) m_szDescriptions) + 1;
		ofn.Flags |= OFN_FILEMUSTEXIST;
		bDialogResult = GetOpenFileNameW((OPENFILENAMEW *)&ofn);
		break;

	case XAP_DIALOG_ID_INSERTMATHML:
		title.fromUTF8 (pSS->getValue(XAP_STRING_ID_DLG_FOSA_InsertMath));
		ofn.lpstrTitle = title.c_str();
		ofn.nFilterIndex = g_strv_length((gchar **) m_szDescriptions) + 1;
		ofn.Flags |= OFN_FILEMUSTEXIST;
		bDialogResult = GetOpenFileNameW((OPENFILENAMEW *)&ofn);
		break;

	case XAP_DIALOG_ID_FILE_EXPORT:
		title.fromUTF8 (pSS->getValue(XAP_STRING_ID_DLG_FOSA_ExportTitle));
		ofn.lpstrTitle = title.c_str();
		ofn.lpfnHook	   = (LPOFNHOOKPROC) s_hookSaveAsProc;
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		ofn.Flags |= OFN_EXPLORER;
		ofn.Flags |= OFN_ENABLEHOOK;
		bDialogResult = GetSaveFileName_Hooked(&ofn, TRUE);
		break;

	case XAP_DIALOG_ID_INSERT_FILE:
		title.fromUTF8 (pSS->getValue(XAP_STRING_ID_DLG_FOSA_InsertTitle));
		ofn.lpstrTitle = title.c_str();		
		ofn.Flags |= OFN_FILEMUSTEXIST;
		ofn.nFilterIndex = g_strv_length((gchar **) m_szDescriptions) + 1;
		bDialogResult = GetOpenFileNameW((OPENFILENAMEW *)&ofn);
		break;

	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	// TODO how do cancels get reported...
	// TODO verify that current-working-directory is not changed...

	if (bDialogResult != FALSE)
	{
		UT_uint32 end = g_strv_length((gchar **) m_szSuffixes);
        char szFileA [MAX_DLG_INS_PICT_STRING];	// buffer for filename
		UT_Win32LocaleString sfile;
		sfile.fromLocale (szFile);
		
		strcpy (szFileA, sfile.ascii_str());

		if ((m_id == XAP_DIALOG_ID_FILE_SAVEAS) &&
			(!UT_pathSuffix(szFileA)))
		{
			// add suffix based on selected file type
			// if selected file is "all documents" or "all"
			// default to .abw, since that is how it will get saved
			UT_ASSERT(ofn.nFilterIndex > 0);

			const char * szSuffix = NULL;
			char abw_sfx[] = ".abw";
			
			if(ofn.nFilterIndex <= end)
				szSuffix = UT_pathSuffix(m_szSuffixes[ofn.nFilterIndex - 1]);
			else
				szSuffix = abw_sfx;
			
			UT_ASSERT(szSuffix);

			UT_uint32 length = strlen(szFileA) + strlen(szSuffix) + 1;
			m_szFinalPathname = (char *)UT_calloc(length,sizeof(char));
			if (m_szFinalPathname)
			{
				char * p = m_szFinalPathname;

				strcpy(p,szFileA);
				strcat(p,szSuffix);
			}
		}
		else
		{
			char *uri = UT_go_filename_to_uri(sfile.utf8_str().utf8_str());
			if(uri)
			{
				if(!(m_szFinalPathname = g_strdup(uri)))
				{
					UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
					m_szFinalPathname = NULL;
				}
				FREEP(uri);
			}
			else
			{
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				m_szFinalPathname = NULL;
			}
		}

		m_answer = a_OK;

		// Set filetype to auto if a generic filter was set
		if (ofn.nFilterIndex > g_strv_length((gchar **) m_szSuffixes))
			m_nFileType = XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO;
		else
			m_nFileType = ofn.nFilterIndex;
	}
	else
	{
		m_answer = a_CANCEL;
		UT_DEBUGMSG(("Didn't get a file: reason=0x%x\n", CommDlgExtendedError()));
	}
}

/*!
 Used for SaveAs case of FileOpenSaveAs dialog
 \param hDlg
 \param msg
 \param wParam
 \param lParam

 Static routine
 */
UINT CALLBACK XAP_Win32Dialog_FileOpenSaveAs::s_hookSaveAsProc(HWND hDlg, UINT msg, WPARAM /*wParam*/, LPARAM lParam)
{
	XAP_Win32Dialog_FileOpenSaveAs* pThis;
	static wchar_t buff[MAX_DLG_INS_PICT_STRING];
	switch(msg)
	{
		case WM_NOTIFY:
			{
				const OFNOTIFYW * pNotify = reinterpret_cast<OFNOTIFYW *>(lParam);
				switch (pNotify->hdr.code)
				{
					case CDN_TYPECHANGE:
						{
							UT_DEBUGMSG(("SaveAs filetype changed to %d\n", pNotify->lpOFN->nFilterIndex));
							pThis = (XAP_Win32Dialog_FileOpenSaveAs*)pNotify->lpOFN->lCustData;
							wchar_t * ext = pThis->_getDefaultExtension(pNotify->lpOFN->nFilterIndex - 1);
							// for some reason the  lpstrFile member of the struct will not be set properly
							// so we have to retrieve the text directly from the control (I could swear that
							// this used to work, and have no idea what changed)
							GetDlgItemTextW(GetParent(hDlg), edt1, buff, MAX_DLG_INS_PICT_STRING);
							wcscpy(buff,pNotify->lpOFN->lpstrFile);                 //was once commented
							wchar_t * dot = wcschr(buff, L'.');
							if(dot)
							{
								*(dot+1) = 0;
							}
							else if(*buff)
							{
								UT_ASSERT(wcslen(buff) < MAX_DLG_INS_PICT_STRING);
								dot = buff + wcslen(buff);
								*dot = L'.';
								*(dot+1) = 0;
							}

							if(dot)
							{
								UT_ASSERT(wcslen(buff) + wcslen(pNotify->lpOFN->lpstrDefExt) < MAX_DLG_INS_PICT_STRING);

								if(ext)
									wcscat(buff,ext);
								else
									*dot = 0;
						
								CommDlg_OpenSave_SetControlText(GetParent(hDlg), edt1, buff);
							}
							CommDlg_OpenSave_SetDefExt(GetParent(hDlg), ext);
						}
						break;
				}
			}
			break;

		default:
			return false;
	}
	return false;
}

/*!
 Used for Insert Picture case of FileOpenSaveAs dialog
 \param hDlg
 \param msg
 \param wParam
 \param lParam

 Static routine
 */
UINT CALLBACK XAP_Win32Dialog_FileOpenSaveAs::s_hookInsertPicProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	XAP_Win32Dialog_FileOpenSaveAs* pThis = (XAP_Win32Dialog_FileOpenSaveAs *) GetWindowLongW(hDlg,GWL_USERDATA);
	bool bPreviewImage = ( IsDlgButtonChecked( hDlg, XAP_RID_DIALOG_INSERT_PICTURE_CHECK_ACTIVATE_PREVIEW )
							  == BST_CHECKED );

	switch(msg)
	{

	case WM_PAINT:
		if (bPreviewImage) pThis->_previewPicture(hDlg);
		return false;
		break;

	case WM_NOTIFY:
		// Only bother if Preview Image is selected
		if ( !bPreviewImage ) return false;
		switch ( ((OFNOTIFYW*) lParam)->hdr.code )
		{
		case CDN_FOLDERCHANGE:
			UT_DEBUGMSG(("Folder Changed File Name Cleared\n"));
			SetDlgItemTextW( GetParent(hDlg), edt1,	NULL );
			SetDlgItemTextW( hDlg, XAP_RID_DIALOG_INSERT_PICTURE_TEXT_HEIGHT, NULL );
			SetDlgItemTextW( hDlg, XAP_RID_DIALOG_INSERT_PICTURE_TEXT_WIDTH, NULL );
			return true;
		case CDN_SELCHANGE:
			return ( pThis->_previewPicture(hDlg) );
		}
		return false;
		break;

	case WM_COMMAND:
		// Check box to Activate Image Preview
		UT_DEBUGMSG(("WM_COMMAND Received: wParam = %ld, lParam = %ld\n",wParam,lParam));
		if ( HIWORD(wParam) == BN_CLICKED ) 
		{
			switch ( LOWORD(wParam) ) 
			{
			case XAP_RID_DIALOG_INSERT_PICTURE_CHECK_ACTIVATE_PREVIEW: 
				if (bPreviewImage)
				{
					ShowWindow( GetDlgItem(hDlg,XAP_RID_DIALOG_INSERT_PICTURE_TEXT_HEIGHT),
								 SW_SHOW );
					ShowWindow( GetDlgItem(hDlg,XAP_RID_DIALOG_INSERT_PICTURE_TEXT_WIDTH),
								 SW_SHOW );
					pThis->_previewPicture(hDlg);
					return true;
				}
				else
				{
					ShowWindow( GetDlgItem(hDlg,XAP_RID_DIALOG_INSERT_PICTURE_TEXT_HEIGHT),
								 SW_HIDE );
					ShowWindow( GetDlgItem(hDlg,XAP_RID_DIALOG_INSERT_PICTURE_TEXT_WIDTH),
								 SW_HIDE );
					return ( pThis->_initPreviewDlg(hDlg) );
				}	
			} 
		}			  
		return false;
		break;

	case WM_INITDIALOG:
		return ( pThis->_initPreviewDlg(hDlg) );
		break;

	default:
		return false;
		break;

	}
	// Default Dialog handles all other issues
	return false;
}
	
UINT XAP_Win32Dialog_FileOpenSaveAs::_previewPicture(HWND hDlg)
{
	HWND hFOSADlg	= GetParent(hDlg);
	HWND hThumbnail = GetDlgItem(hDlg,XAP_RID_DIALOG_INSERT_PICTURE_IMAGE_PREVIEW);
    UT_Win32LocaleString str;

	const XAP_StringSet*  pSS	= XAP_App::getApp()->getStringSet();
	UT_return_val_if_fail(pSS, false);

	// Check if File Name is for a file
	wchar_t buf[MAX_DLG_INS_PICT_STRING];
	SendMessageW( hFOSADlg, CDM_GETFILEPATH, sizeof(buf), (LPARAM) buf );
	// If a Directory stop
	if ( GetFileAttributesW( buf ) == FILE_ATTRIBUTE_DIRECTORY )
	{
		return false;
	}

    str.fromLocale (buf);
	// Pass only files that can be openned
	FILE* fitxer = fopen (str.ascii_str(),"r");

	if (fitxer)
		fclose(fitxer);
	else
		return false;

	UT_DEBUGMSG(("File Selected is %s\n", buf));

	// Load File into memory
	UT_ByteBuf* pBB 	= new UT_ByteBuf(NULL);
	UT_ByteBuf* pTempBB = new UT_ByteBuf(NULL);
	pBB->insertFromFile(0, str.ascii_str());

	// Build an Import Graphic based on file type
	IEGraphicFileType iegft = IEGFT_Unknown;
	IE_ImpGraphic* pIEG;
	UT_Error errorCode = UT_ERROR;
	char *uri = UT_go_filename_to_uri(str.ascii_str());
	if(uri)
		errorCode = IE_ImpGraphic::constructImporter(uri, iegft, &pIEG);

	if (errorCode)
	{
		DELETEP(pBB);
		DELETEP(pTempBB);
		return false;
	}
	iegft = pIEG->fileTypeForContents( (const char *) pBB->getPointer(0), 50);

	// Skip import if PNG or SVG file
	if (iegft != IEGFT_PNG && iegft != IEGFT_SVG)
	{
		// Convert to PNG or SVG (pBB Memoried freed in function
		errorCode = pIEG->convertGraphic(pBB, &pTempBB);  
		pBB = pTempBB;
		if (errorCode)
		{
			DELETEP(pIEG);
			DELETEP(pBB);
			DELETEP(pTempBB);
			FREEP(uri);
			return false;
		}
	}
	// Reset file type based on conversion
	iegft = pIEG->fileTypeForContents( (const char *) pBB->getPointer(0), 50);
	DELETEP(pIEG);
	FREEP(uri);

	double		scale_factor = 0.0;
	UT_sint32	scaled_width,scaled_height;
	UT_sint32	iImageWidth,iImageHeight;
	RECT		r;

	GetClientRect (hThumbnail, &r);
	InvalidateRect(hThumbnail, &r, true);

	if (iegft == IEGFT_PNG)
	{
		UT_PNG_getDimensions(pBB, iImageWidth, iImageHeight);
	}
	else
	{
	//	TODO Fix this
	//	UT_sint32 layoutWidth;
	//	UT_sint32 layoutHeight;
	//	UT_SVG_getDimensions(pBB, pGr, iImageWidth, iImageHeight, layoutWidth, layoutHeight);
		iImageWidth = 0;
		iImageHeight = 0;
	}

	// Update Height and Width Strings
    str.fromUTF8 (pSS->getValue(XAP_STRING_ID_DLG_IP_Height_Label));
	swprintf( buf, 
			  L"%s %d",
			  str.c_str(), 
			  iImageHeight );
	SetDlgItemTextW( hDlg,
					XAP_RID_DIALOG_INSERT_PICTURE_TEXT_HEIGHT,
					buf );

    str.fromUTF8 (pSS->getValue(XAP_STRING_ID_DLG_IP_Width_Label));
	swprintf( buf, 
			 L"%s %d",
			 str.c_str(), 
			  iImageWidth );
	SetDlgItemTextW( hDlg,
					XAP_RID_DIALOG_INSERT_PICTURE_TEXT_WIDTH,
					buf );

	// Reset String for Height and Width
	if (r.right >= iImageWidth && r.bottom >= iImageHeight)
		scale_factor = 1.0;
	else
		scale_factor = UT_MIN( (double) r.right/iImageWidth,
							(double) r.bottom/iImageHeight );

	scaled_width  = (int) (scale_factor * iImageWidth);
	scaled_height = (int) (scale_factor * iImageHeight);

	GR_Win32Image* pImage = new GR_Win32Image(NULL);
	pImage->convertFromBuffer(pBB, scaled_width, scaled_height);

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hThumbnail, &ps);
	FillRect(hdc, &r, GetSysColorBrush(COLOR_BTNFACE));

	GR_Win32AllocInfo ai(hdc,hThumbnail);
	GR_Win32Graphics *pGr = (GR_Win32Graphics *)XAP_App::getApp()->newGraphics(ai);
	UT_return_val_if_fail(pGr, false);
	
	pGr->drawImage(pImage,
				   pGr->tlu((r.right	- scaled_width ) / 2),
				   pGr->tlu((r.bottom - scaled_height) / 2));
	EndPaint(hThumbnail,&ps);

	DELETEP(pBB);
	DELETEP(pImage);
	DELETEP(pGr);

	return true;
}

UINT XAP_Win32Dialog_FileOpenSaveAs::_initPreviewDlg(HWND hDlg)
{
	HWND hFOSADlg	= GetParent(hDlg);
	UT_Win32LocaleString str;
	setHandle (hDlg);

	const XAP_StringSet*  pSS		  = XAP_App::getApp()->getStringSet();
	UT_return_val_if_fail(pSS, false);

    setDlgItemText( XAP_RID_DIALOG_INSERT_PICTURE_IMAGE_PREVIEW,
					pSS->getValue(XAP_STRING_ID_DLG_IP_No_Picture_Label));
  
    setDlgItemText( XAP_RID_DIALOG_INSERT_PICTURE_CHECK_ACTIVATE_PREVIEW,
					pSS->getValue(XAP_STRING_ID_DLG_IP_Activate_Label));

    setDlgItemText( XAP_RID_DIALOG_INSERT_PICTURE_TEXT_HEIGHT,
					pSS->getValue(XAP_STRING_ID_DLG_IP_Height_Label));
    
    setDlgItemText( XAP_RID_DIALOG_INSERT_PICTURE_TEXT_WIDTH,
					pSS->getValue(XAP_STRING_ID_DLG_IP_Width_Label));
    
    str.fromUTF8 (pSS->getValue(XAP_STRING_ID_DLG_IP_Button_Label));
    SetDlgItemTextW( hFOSADlg,
					IDOK,
					str.c_str());

	return true;

}
