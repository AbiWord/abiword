/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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

#include <string.h>

#include <CoreFoundation/CoreFoundation.h>

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_MacFiles.h"
#include "xap_Dialog_Id.h"
#include "xap_MacDlg_FileOpenSaveAs.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"
//#include "ie_imp.h"
//#include "ie_exp.h"

/*****************************************************************/
XAP_Dialog * XAP_MacDialog_FileOpenSaveAs::static_constructor(XAP_DialogFactory * pFactory,
															  XAP_Dialog_Id id)
{
	XAP_MacDialog_FileOpenSaveAs * p = new XAP_MacDialog_FileOpenSaveAs(pFactory,id);
	return p;
}

XAP_MacDialog_FileOpenSaveAs::XAP_MacDialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory,
															 XAP_Dialog_Id id)
	: XAP_Dialog_FileOpenSaveAs(pDlgFactory,id)
{
}

XAP_MacDialog_FileOpenSaveAs::~XAP_MacDialog_FileOpenSaveAs(void)
{
}

/*****************************************************************/


void XAP_MacDialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	// do we want to let this function handle stating the Unix
	// directory for writability?  Save/Export operations will want
	// this, open/import will not.

	bool bCheckWritePermission = false;
	const XML_Char * szTitle = NULL;
	const XML_Char * szFileTypeLabel = NULL;


	switch (m_id) {
	case XAP_DIALOG_ID_INSERT_PICTURE:
        szTitle = pSS->getValue(XAP_STRING_ID_DLG_IP_Title);
		szFileTypeLabel = pSS->getValue(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel);
		bCheckWritePermission = false;    
		break;
	case XAP_DIALOG_ID_FILE_OPEN:
		szTitle = pSS->getValue(XAP_STRING_ID_DLG_FOSA_OpenTitle);
		szFileTypeLabel = pSS->getValue(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel);
		bCheckWritePermission = false;
		break;
	case XAP_DIALOG_ID_FILE_SAVEAS: 
		szTitle = pSS->getValue(XAP_STRING_ID_DLG_FOSA_SaveAsTitle);
		szFileTypeLabel = pSS->getValue(XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel);
		bCheckWritePermission = true;
		break;
	case XAP_DIALOG_ID_PRINTTOFILE:
		szTitle = pSS->getValue(XAP_STRING_ID_DLG_FOSA_PrintToFileTitle);
		szFileTypeLabel = pSS->getValue(XAP_STRING_ID_DLG_FOSA_FilePrintTypeLabel);
		bCheckWritePermission = true;
		break;
	default:
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	}

	bool bResult = false;
	OSErr	anErr;
    NavDialogOptions    dialogOptions;
    AEDesc              defaultLocation;
    NavEventUPP         eventProc = NULL;
    NavObjectFilterUPP  filterProc = NULL;

	FSRef               documentFSRef;
						
    //  Specify default options for dialog box
    anErr = ::NavGetDefaultDialogOptions(&dialogOptions);
    if (anErr == noErr)
    {
        //  Adjust the options to fit our needs
        //  Set default location option
        dialogOptions.dialogOptionFlags |= kNavSelectDefaultLocation;
        //  Clear preview option
        dialogOptions.dialogOptionFlags ^= kNavAllowPreviews;
		
//		dialogOptions.message
//		dialogOptions.windowTitle
//		dialogOptions.clientName

		// Get 'open' resource. A nil handle being returned is OK,
		// this simply means no automatic file filtering.
		NavTypeListHandle typeList = NULL; //currently we use NULL
		//(NavTypeListHandle)GetResource(
		//                            'open', 128);
		NavReplyRecord reply;
		
		// Call NavGetFile() with specified options and
		// declare our app-defined functions and type list
		anErr = ::NavGetFile (NULL, &reply, &dialogOptions,
							NULL, NULL, NULL,
							NULL, NULL);
		m_answer = a_CANCEL;
		if (anErr == noErr && reply.validRecord)
		{
			//  Deal with multiple file selection
			long    count;
			
			anErr = ::AECountItems(&(reply.selection), &count);
			// Set up index for file list
			if (anErr == noErr)
			{
				long index;
				
				for (index = 1; index <= count; index++)
				{
					AEKeyword   theKeyword;
					DescType    actualType;
					Size        actualSize;
					
					// Get a pointer to selected file
					anErr = ::AEGetNthPtr(&(reply.selection), index,
										typeFSRef, &theKeyword,
										&actualType,&documentFSRef,
										sizeof(documentFSRef),
										&actualSize);
					if (anErr == noErr)
					{
						//extract the name of the file.
						//anErr = DoOpenFile(&documentFSSpec);
						bResult = true;
						break; //FIXME. Get 1 file. Period.
					}
				}
			}
			//  Dispose of NavReplyRecord, resources, descriptors
			anErr = ::NavDisposeReply(&reply);
		}
//            if (typeList != NULL)
//            {
//                ReleaseResource( (Handle)typeList);
//            }
    }
	
	// FIXME check that pathHandle does not leak....
	if (bResult)
	{
	#if defined(CARBON_ON_MACH_O) && CARBON_ON_MACH_O == 1
		m_szFinalPathname = UT_FSRefToUNIXPath (&documentFSRef);
	#else
		Handle              pathHandle = NULL;
		short               pathLen = 0;
		FSSpec				documentFSSpec;
		OSErr err = ::FSGetCatalogInfo (&documentFSRef, kFSCatInfoNone, NULL, NULL, &documentFSSpec, NULL);
		UT_ASSERT (err == noErr);		// FIXIT bad assert use.
		::FSpGetFullPath(&documentFSSpec, &pathLen, &pathHandle);
		UT_ASSERT (pathHandle);
		::HLock (pathHandle);
		m_szFinalPathname = (char *)malloc (sizeof(char) * (pathLen + 2));
		memcpy (m_szFinalPathname, *pathHandle, sizeof(char) * pathLen);
		::DisposeHandle (pathHandle);
	#endif
		UT_DEBUGMSG (("m_szFinalPathname == %s\n", m_szFinalPathname));
		m_answer = a_OK;
	}

	return;
}






