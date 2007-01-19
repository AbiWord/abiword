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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <Pt.h>

#include "ut_string.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_QNXDlg_FileOpenSaveAs.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"
#include "xap_Strings.h"

/*****************************************************************/
XAP_Dialog * XAP_QNXDialog_FileOpenSaveAs::static_constructor(XAP_DialogFactory * pFactory,
															 XAP_Dialog_Id id)
{
	XAP_QNXDialog_FileOpenSaveAs * p = new XAP_QNXDialog_FileOpenSaveAs(pFactory,id);
	return p;
}

XAP_QNXDialog_FileOpenSaveAs::XAP_QNXDialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory,
														   XAP_Dialog_Id id)
	: XAP_Dialog_FileOpenSaveAs(pDlgFactory,id)
{
	m_szFinalPathnameCandidate = NULL;
}

XAP_QNXDialog_FileOpenSaveAs::~XAP_QNXDialog_FileOpenSaveAs(void)
{
	FREEP(m_szFinalPathnameCandidate);
}

/*****************************************************************/

void XAP_QNXDialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	char file_filter[80];	
	m_pQNXFrame = (XAP_QNXFrame *)pFrame;
	UT_ASSERT(m_pQNXFrame);

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_UTF8String s;
	
	// do we want to let this function handle stating the QNX
	// directory for writability?  Save/Export operations will want
	// this, open/import will not.

	bool bCheckWritePermission = true;
	const gchar * szTitle = NULL;
	int   flags = 0;

	switch (m_id)
	{
	case XAP_DIALOG_ID_FILE_SAVEAS:
	{
		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_SaveAsTitle,s);
		szTitle = g_strdup(s.utf8_str());
		bCheckWritePermission = true;
		/* Allow non-existant files to be selected and confirm overwrite */
		flags = Pt_FSR_NO_FCHECK | Pt_FSR_CONFIRM_EXISTING;
		break;
	}
	case XAP_DIALOG_ID_PRINTTOFILE:
	{
		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_PrintToFileTitle,s);
		szTitle = g_strdup(s.utf8_str());
		bCheckWritePermission = true;
		flags = Pt_FSR_NO_FCHECK | Pt_FSR_CONFIRM_EXISTING;
		break;
	}
		case XAP_DIALOG_ID_INSERT_PICTURE:
	  {
		  pSS->getValueUTF8(XAP_STRING_ID_DLG_IP_Title,s);
		  szTitle = g_strdup(s.utf8_str());
		  bCheckWritePermission = false;    
		  break;
	  }
	case XAP_DIALOG_ID_FILE_OPEN:
	{
		pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_OpenTitle,s);
		szTitle = g_strdup(s.utf8_str());
		bCheckWritePermission = false;
		break;
	}
	case XAP_DIALOG_ID_FILE_IMPORT:
	  {
		  pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ImportTitle,s);
		  szTitle = g_strdup(s.utf8_str());
		  bCheckWritePermission = false;
		  break;
	  }
	case XAP_DIALOG_ID_INSERT_FILE:
	  {
		  pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_InsertTitle,s);
		  szTitle = g_strdup(s.utf8_str());
		  bCheckWritePermission = false;
		  break;
	  }
	case XAP_DIALOG_ID_FILE_EXPORT:
	  {
		  pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ExportTitle,s);
		  szTitle = g_strdup(s.utf8_str());
		  bCheckWritePermission = true;
		  flags = Pt_FSR_NO_FCHECK | Pt_FSR_CONFIRM_EXISTING;
		  break;
	  }
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
}

	// use the persistence info and/or the suggested filename
	// to properly seed the dialog.
	
	char * szPersistDirectory = NULL;	// we must g_free this

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

			szPersistDirectory = g_strdup(m_szPersistPathname);
			char * pLastSlash = strrchr(szPersistDirectory, '/');
			if (pLastSlash)
				pLastSlash[1] = 0;
		}
		else
		{
			// no initial pathname given and we don't have
			// a pathname from a previous use, so just let
			// it come up in the current working directory.
			char thisdir[PATH_MAX];
			getcwd(thisdir, PATH_MAX);
			szPersistDirectory = g_strdup(thisdir);
		}
	}
	else
	{
		// we have an initial pathname (the name of the document
		// in the frame that we were invoked on).  if the caller
		// wanted us to suggest a filename, use the initial
		// pathname as is.  if not, use the directory portion of
		// it.
		
		//TODO: We need to make sure the fullpath of the 
		//      document is loaded up at start time.

		if (m_bSuggestName)
		{
			// use m_szInitialPathname
			szPersistDirectory = g_strdup(m_szInitialPathname);
		}
		else
		{
			// use directory(m_szInitialPathname)
			szPersistDirectory = g_strdup(m_szInitialPathname);
			char * pLastSlash = strrchr(szPersistDirectory, '/');
			if (pLastSlash)
				pLastSlash[1] = 0;
		}
	}

	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parent =	pQNXFrameImpl->getTopLevelWindow();	

	PtFileSelectionInfo_t info;
	int ret;

	memset(&info, 0, sizeof(info));

	//Put this in a local function in order to run it in a loop

	//TODO: Dynamically create the file specification
	file_filter[0] = '\0';
	{
		UT_ASSERT(g_strv_length((gchar **) m_szSuffixes) == g_strv_length((gchar **) m_szDescriptions));

		// measure one list, they should all be the same length
		UT_uint32 end = g_strv_length((gchar **) m_szDescriptions);

		for (UT_uint32 i = 0; i < end; i++) {
			UT_uint32 totallen;

			if(m_nTypeList[i] == m_nDefaultFileType) {
				printf("Missing FEATURE: No way to show default filter\n");
			}

#define PHOTON_HAS_STUPID_FILTER_LIMITATION
#if defined(PHOTON_HAS_STUPID_FILTER_LIMITATION)
			if(i >= 10) {
				printf("PHOTON BUG: Not enough room for filter \n");
				break;
			}
#endif

			totallen = strlen(file_filter) + strlen(m_szSuffixes[i]) + 2;
#if defined(PHOTON_SUPPORTS_COMBOFILTER)
			totallen += strlen(m_szDescriptions[i]) + 1;
#endif

			if (totallen > sizeof(file_filter)) {
				printf("PHOTON BUG: File filter string is limited to 80 characters \n");
				break;
			}

#if defined(PHOTON_SUPPORTS_COMBOFILTER)
			strcat(file_filter, "|");
			strcat(file_filter, m_szDescriptions[i]);
			strcat(file_filter, "|");
			strcat(file_filter, m_szSuffixes[i]);
#else	
			if (*file_filter) {
				strcat(file_filter, ",");
			}
			strcat(file_filter, m_szSuffixes[i]);
#endif
		}
	}
	char *tmp=file_filter;
while((tmp=strchr(tmp,';'))) 
	tmp[0]=',';
		
	ret = PtFileSelection(parent,											/* Parent */
						  NULL,												/* Position */
						  szTitle,											/* Title */
						  szPersistDirectory,								/* Root directory */
						  file_filter /* "*.abw, *"*/ , 					/* File spec */
						   (m_id == XAP_DIALOG_ID_FILE_OPEN)   ? "Open"  :  /* Button 1 name */
						  ((m_id == XAP_DIALOG_ID_FILE_SAVEAS) ? "Save"  :
						  ((m_id == XAP_DIALOG_ID_PRINTTOFILE) ? "Print" : NULL)),
						  NULL,												/* Button 2 name */
						  "nkd", 							/* Format (Name, Kb Size, date) */
						  &info,							/* Returned info structure */
						  flags);							/* Flags we might want */
	if (ret != -1 && (info.ret == Pt_FSDIALOG_BTN1)) {
		m_answer = XAP_Dialog_FileOpenSaveAs::a_OK;
		
		m_nFileType= XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO;

		if (bCheckWritePermission)	 {  //Save/Print dialogs
			//If the file didn't have a suffix, then append one
			//We would normally do this based on the filter type ...

			//In photon this is kind of screwy since we have filters.
			if ((!UT_pathSuffix(info.path))) {
				//strcat(info.path, UT_pathSuffix(m_szSuffixes[0]);
				strcat(info.path, ".abw");
			}

			//Check our access on the file: 
			//- if can't write then try again
			//- if file exists then prompt for overwrite
		}

		// store final path name and file type
		m_szFinalPathname = g_strdup(info.path);

		//Not sure if I should do this here or not
		if (m_szPersistPathname)
			FREEP(m_szPersistPathname);
		m_szPersistPathname = g_strdup(info.path);

		//Store other info here too ....
		//info.format, info.fspec
	}
	else {
		m_answer = XAP_Dialog_FileOpenSaveAs::a_CANCEL;
	}
	
		   
	FREEP(szPersistDirectory);
	FREEP(szTitle);
	m_pQNXFrame = NULL;

	return;
}
