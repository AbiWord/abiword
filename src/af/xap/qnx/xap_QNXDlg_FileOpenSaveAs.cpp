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
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_QNXDlg_FileOpenSaveAs.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"
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

#if 0
UT_Bool XAP_QNXDialog_FileOpenSaveAs::_run_gtk_main(XAP_Frame * pFrame,
													 void * pFSvoid,
													 UT_Bool bCheckWritePermission,
													 GtkWidget * filetypes_pulldown)
{
	GtkFileSelection * pFS = (GtkFileSelection *)pFSvoid;

	/*
	  Run the dialog in a loop to catch bad filenames.
	  The location of this check being in this dialog loop
	  could be considered temporary.  Doing this matches the Windows
	  common control behavior (where the dialog checks everything
	  for the programmer), but lacks flexibility for different
	  uses of this dialog (file export, print export, directory
	  (not file) selection).

	  This check might need to be moved into the ap code which calls
	  this dialog, and certain interfaces exposed so that the
	  dialog is displayed throughout the verification.

	  For right now you can signal this check on and off with
	  bCheckWritePermission.
	*/

	char * szDialogFilename = NULL;		// this is the string returned from the dialog
	char * szFinalPathname = NULL;		// this is the file after any suffix additions
	char * szFinalPathnameCopy = NULL;	// one to mangle when looking for dirs, etc.

	char * pLastSlash;
	struct stat buf;
	int err;

	// if bCheckWritePermission is not set, we're looking to OPEN a file.
	
	if (!bCheckWritePermission)
	{
		while (1)
		{
			gtk_main();
			if (m_answer == a_CANCEL)			// The easy way out
				return UT_FALSE;

			// TODO  check for symlinks, because even symlinks to dirs won't
			// TODO  show up with S_ISDIR().

			// TODO  check to make sure a file exists before we close off the
			// TODO  loop
			
			// We can't just return, because we might have some dialog work to
			// do.  For example, the user might have typed in a directory, not
			// a file, so we have to catch it, change the dialog, and not return
			// any filename yet.

			UT_cloneString(szDialogFilename, gtk_file_selection_get_filename(pFS));
			UT_ASSERT(szDialogFilename);

			err = stat(szDialogFilename, &buf);
			UT_ASSERT(err == 0 || err == -1);
			
			// Check for a directory entered as filename.  When true,
			// set the filter properly and continue in the selection
			if (err == 0 && S_ISDIR(buf.st_mode))
			{
				GString * s = g_string_new(szDialogFilename);
				if (s->str[s->len - 1] != '/')
				{
					g_string_append_c(s, '/');
				}
				gtk_file_selection_set_filename(pFS, s->str);
				g_string_free(s, TRUE);

				// free the string and continue along
				FREEP(szDialogFilename);
				continue;
			}

			UT_cloneString(m_szFinalPathnameCandidate, szDialogFilename);
			
			// if we got here, the text wasn't a directory, so it's a file,
			// and life is good
			return (m_answer == a_OK);
		}
	}		
		
	// if bCheckWritePermission is set, we're looking to SAVE a file.

	while(1)
	{
		gtk_main();
		if (m_answer == a_CANCEL)			// The easy way out
			return UT_FALSE;

		// Give us a filename we can mangle

		UT_cloneString(szDialogFilename, gtk_file_selection_get_filename(pFS));
		UT_ASSERT(szDialogFilename);

		// We append the suffix of the default type, so the user doesn't
		// have to.  This is adapted from the Windows front-end code
		// (xap_Win32Dlg_FileOpenSaveAs.cpp), since it should act the same.
		{
			//UT_uint32 end = UT_pointerArrayLength((void **) m_szSuffixes);

			GtkWidget * activeItem = gtk_menu_get_active(GTK_MENU(
				gtk_option_menu_get_menu(GTK_OPTION_MENU(filetypes_pulldown))));
			UT_ASSERT(activeItem);

			UT_sint32 nFileType = GPOINTER_TO_INT(gtk_object_get_user_data(
				GTK_OBJECT(activeItem)));

			// set to first item, which should probably be auto detect
			// TODO : "probably" isn't very good.
			UT_uint32 nIndex = 0;
			
			// the index in the types table will match the index in the suffix
			// table.  nFileType is the data we are searching for.
			for (UT_uint32 i = 0; m_nTypeList[i]; i++)
			{
				if (m_nTypeList[i] == nFileType)
				{
					nIndex = i;
					break;
				}
			}

			// if the file doesn't have a suffix already, and the file type
			// is normal (special types are negative, like auto detect),
			// slap a suffix on it.
			if ((!UT_pathSuffix(szDialogFilename)) &&
				(nFileType > 0))
			{
				// add suffix based on selected file type

				const char * szSuffix = UT_pathSuffix(m_szSuffixes[nIndex]);
				UT_ASSERT(szSuffix);

				UT_uint32 length = strlen(szDialogFilename) + strlen(szSuffix) + 1;
				szFinalPathname = (char *)calloc(length,sizeof(char));
				if (szFinalPathname)
				{
					char * p = szFinalPathname;

					strcpy(p,szDialogFilename);
					strcat(p,szSuffix);
				}
			}
			else
			{
				// the file type is special (auto detect)

				// set to plain name, and let the auto detector in the
				// exporter figure it out
				UT_cloneString(szFinalPathname,szDialogFilename);
			}
			// free szDialogFilename since it's been put into szFinalPathname (with
			// or without changes) and it's invalid (missing an extension which
			// might have been appended)
			FREEP(szDialogFilename);
		}

		UT_cloneString(szFinalPathnameCopy, szFinalPathname);
		
		err = stat(szFinalPathnameCopy, &buf);
		UT_ASSERT(err == 0 || err == -1);
			
		// Does the filename already exist?

		if (err == 0 && S_ISREG(buf.st_mode))
		{
			// we have an existing file, ask to overwrite

			if (_askOverwrite_YesNo(pFrame, szFinalPathname))
			{
				UT_cloneString(m_szFinalPathnameCandidate, szFinalPathname);
				goto ReturnTrue;
			}

			goto ContinueLoop;
		}
			
		// Check for a directory entered as filename.  When true,
		// set the filter properly and continue in the selection

		if (err == 0 && S_ISDIR(buf.st_mode))
		{
			GString * s = g_string_new(szFinalPathnameCopy);
			if (s->str[s->len - 1] != '/')
			{
				g_string_append_c(s, '/');
			}
			gtk_file_selection_set_filename(pFS, s->str);
			g_string_free(s, TRUE);
			goto ContinueLoop;
		}

		// We have a string that may contain a path, and may have a file
		// at the end.  First, strip off a file (if it exists), and test
		// for a matching directory.  We can then proceed with the file
		// if another stat of that dir passes.

		pLastSlash = rindex(szFinalPathnameCopy,'/');
		if (!pLastSlash)
		{
			_notifyError_OKOnly(pFrame,XAP_STRING_ID_DLG_InvalidPathname);
			goto ContinueLoop;
		}

		// Trim the pathname at beginning of the filename
		// keeping the trailing slash.
			
		pLastSlash[1] = 0;

		// Stat the directory left over

		err = stat(szFinalPathnameCopy, &buf);
		UT_ASSERT(err == 0 || err == -1);

		// If this directory doesn't exist, we have been feed garbage
		// at some point.  Throw an error and continue with the selection.

		if (err == -1)
		{
			_notifyError_OKOnly(pFrame,XAP_STRING_ID_DLG_NoSaveFile_DirNotExist);
			goto ContinueLoop;
		}

		// Since the stat passed the last test, we will make sure the
		// directory is suitable for writing, since we know it exists.

		UT_ASSERT(S_ISDIR(buf.st_mode));

		if (!access(szFinalPathnameCopy, W_OK))
		{
			// we've got what we need, save it to the candidate
			UT_cloneString(m_szFinalPathnameCandidate, szFinalPathname);
			goto ReturnTrue;
		}

		// complain about write permission on the directory.
		// lop off ugly trailing slash only if we don't have
		// the root dir ('/') for a path

		if (pLastSlash > szFinalPathnameCopy)
			*pLastSlash = 0;

		_notifyError_OKOnly(pFrame,XAP_STRING_ID_DLG_NoSaveFile_DirNotWriteable,
							szFinalPathname);
	ContinueLoop:
		FREEP(szFinalPathnameCopy);
	}

	/*NOTREACHED*/

ReturnTrue:
	FREEP(szFinalPathnameCopy);
	FREEP(szFinalPathname);
	return UT_TRUE;
}
#endif

UT_Bool XAP_QNXDialog_FileOpenSaveAs::_askOverwrite_YesNo(XAP_Frame * pFrame, const char * fileName)
{
#if 0
	// return UT_TRUE if we should overwrite the file
	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_MessageBox * pDialog
		= (XAP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	pDialog->setMessage(pSS->getValue(XAP_STRING_ID_DLG_OverwriteFile), fileName);
	pDialog->setButtons(XAP_Dialog_MessageBox::b_YN);
	pDialog->setDefaultAnswer(XAP_Dialog_MessageBox::a_NO);	// should this be YES?

	pDialog->runModal(pFrame);

	XAP_Dialog_MessageBox::tAnswer ans = pDialog->getAnswer();

	pDialogFactory->releaseDialog(pDialog);

	return (ans == XAP_Dialog_MessageBox::a_YES);
#endif
	return(UT_FALSE);
}
	
void XAP_QNXDialog_FileOpenSaveAs::_notifyError_OKOnly(XAP_Frame * pFrame, XAP_String_Id sid)
{
#if 0
	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_MessageBox * pDialog
		= (XAP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	pDialog->setMessage(pSS->getValue(sid));
	pDialog->setButtons(XAP_Dialog_MessageBox::b_O);
	pDialog->setDefaultAnswer(XAP_Dialog_MessageBox::a_OK);

	pDialog->runModal(pFrame);

	pDialogFactory->releaseDialog(pDialog);
#endif
}

void XAP_QNXDialog_FileOpenSaveAs::_notifyError_OKOnly(XAP_Frame * pFrame,
														XAP_String_Id sid,
														const char * sz1)
{
#if 0
	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_MessageBox * pDialog
		= (XAP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	pDialog->setMessage(pSS->getValue(sid),sz1);
	pDialog->setButtons(XAP_Dialog_MessageBox::b_O);
	pDialog->setDefaultAnswer(XAP_Dialog_MessageBox::a_OK);

	pDialog->runModal(pFrame);

	pDialogFactory->releaseDialog(pDialog);
#endif
}

/*****************************************************************/

void XAP_QNXDialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	m_pQNXFrame = (XAP_QNXFrame *)pFrame;
	UT_ASSERT(m_pQNXFrame);

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	// do we want to let this function handle stating the QNX
	// directory for writability?  Save/Export operations will want
	// this, open/import will not.

	UT_Bool bCheckWritePermission;

	const XML_Char * szTitle;
	const XML_Char * szFileTypeLabel;
	int   flags = 0;
	switch (m_id)
	{
	case XAP_DIALOG_ID_FILE_OPEN:
	{
		szTitle = pSS->getValue(XAP_STRING_ID_DLG_FOSA_OpenTitle);
		szFileTypeLabel = pSS->getValue(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel);
		bCheckWritePermission = UT_FALSE;
		break;
	}
	case XAP_DIALOG_ID_FILE_SAVEAS:
	{
		szTitle = pSS->getValue(XAP_STRING_ID_DLG_FOSA_SaveAsTitle);
		szFileTypeLabel = pSS->getValue(XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel);
		bCheckWritePermission = UT_TRUE;
		flags = Pt_FSDIALOG_NO_FCHECK;
		break;
	}
	case XAP_DIALOG_ID_PRINTTOFILE:
	{
		szTitle = pSS->getValue(XAP_STRING_ID_DLG_FOSA_PrintToFileTitle);
		szFileTypeLabel = pSS->getValue(XAP_STRING_ID_DLG_FOSA_FilePrintTypeLabel);
		bCheckWritePermission = UT_TRUE;
		flags = Pt_FSDIALOG_NO_FCHECK;
		break;
	}
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	// use the persistence info and/or the suggested filename
	// to properly seed the dialog.
	
	char * szPersistDirectory = NULL;	// we must free this
	
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

			UT_cloneString(szPersistDirectory,m_szPersistPathname);
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
			UT_cloneString(szPersistDirectory, thisdir);
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
			UT_cloneString(szPersistDirectory, m_szInitialPathname);
		}
		else
		{
			// use directory(m_szInitialPathname)
			UT_cloneString(szPersistDirectory,m_szInitialPathname);
			char * pLastSlash = strrchr(szPersistDirectory, '/');
			if (pLastSlash)
				pLastSlash[1] = 0;
		}
	}

	// get top level window and its GtkWidget *
	XAP_QNXFrame * frame = (XAP_QNXFrame *)(pFrame);
	UT_ASSERT(frame);
	PtWidget_t * parent = frame->getTopLevelWindow();
	UT_ASSERT(parent);

	PtFileSelectionInfo_t info;
	int ret;

	memset(&info, 0, sizeof(info));

#if 0
	/*
	  To facilitate a file-types selection, we dig around in some
	  private data for the dialog layout, and add a drop-down list
	  of known types.  We store an indexer in the user data
	  for each menu item in the popup, so we can read the type
	  we need to return.
	*/
			char buffer[1024];

			// Auto-detect is always an option, but a special one, so we use
			// a pre-defined constant for the type, and don't use the user-supplied
			// types yet.
			g_snprintf(buffer, 1024, "%s", pSS->getValue(XAP_STRING_ID_DLG_FOSA_FileTypeAutoDetect));
			thismenuitem = gtk_menu_item_new_with_label(buffer);
			gtk_object_set_user_data(GTK_OBJECT(thismenuitem), GINT_TO_POINTER(XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO));
			gtk_widget_show(thismenuitem);
			gtk_menu_append(GTK_MENU(menu), thismenuitem);

			UT_uint32 activeItemIndex = 0;
			
			// add list items
			{
				UT_ASSERT(UT_pointerArrayLength((void **) m_szSuffixes) ==
						  UT_pointerArrayLength((void **) m_szDescriptions));
				
				// measure one list, they should all be the same length
				UT_uint32 end = UT_pointerArrayLength((void **) m_szDescriptions);
			  
				for (UT_uint32 i = 0; i < end; i++)
				{
					// If this type is default, save its index (i) for later use
					if (m_nTypeList[i] == m_nDefaultFileType)
						activeItemIndex = i;
					
					g_snprintf(buffer, 1024, "%s", m_szDescriptions[i]);
					thismenuitem = gtk_menu_item_new_with_label(buffer);
					gtk_object_set_user_data(GTK_OBJECT(thismenuitem), GINT_TO_POINTER(m_nTypeList[i]));
					gtk_widget_show(thismenuitem);
					gtk_menu_append(GTK_MENU(menu), thismenuitem);
				}
			}
#endif

//Put this in a local function in order to run it in a loop
again:
	ret = PtFileSelection(parent,
						  NULL,
						  szTitle,
						  szPersistDirectory,
						  "*.abw, *", /* File spec  should get from below */
						  (m_id == XAP_DIALOG_ID_FILE_OPEN) ? "Open" :
						  ((m_id == XAP_DIALOG_ID_FILE_SAVEAS) ? "Save" :
						  ((m_id == XAP_DIALOG_ID_PRINTTOFILE) ? "Print" : NULL)),
						  NULL,
						  "nkd", /* Format (Name, Kb Size, date) */
						  &info,
						  flags);

	if (ret != -1 && (info.ret == Pt_FSDIALOG_BTN1)) {
		m_answer = XAP_Dialog_FileOpenSaveAs::a_OK;

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
		UT_cloneString(m_szFinalPathname, info.path);

		//Not sure if I should do this here or not
		if (m_szPersistPathname)
			FREEP(szPersistDirectory);
		UT_cloneString(m_szPersistPathname, info.path);

		//Store other info here too ....
		//info.format, info.fspec
	}
	else {
		m_answer = XAP_Dialog_FileOpenSaveAs::a_CANCEL;
	}
	
#if 0
	/*
	  Run the dialog in a loop to catch bad filenames.
	  The location of this check being in this dialog loop
	  could be considered temporary.  Doing this matches the Windows
	  common control behavior (where the dialog checks everything
	  for the programmer), but lacks flexibility for different
	  uses of this dialog (file export, print export, directory
	  (not file) selection).

	  This check might need to be moved into the ap code which calls
	  this dialog, and certain interfaces exposed so that the
	  dialog is displayed throughout the verification.

	  For right now you can signal this check on and off with
	  bCheckWritePermission.
	*/

	char * szDialogFilename = NULL;		// this is the string returned from the dialog
	char * szFinalPathname = NULL;		// this is the file after any suffix additions
	char * szFinalPathnameCopy = NULL;	// one to mangle when looking for dirs, etc.

	char * pLastSlash;
	struct stat buf;
	int err;

	// if bCheckWritePermission is not set, we're looking to OPEN a file.
	
	if (!bCheckWritePermission)
	{
		while (1)
		{
			if (m_answer == a_CANCEL)			// The easy way out
				return UT_FALSE;

			// TODO  check for symlinks, because even symlinks to dirs won't
			// TODO  show up with S_ISDIR().

			// TODO  check to make sure a file exists before we close off the
			// TODO  loop
			
			// We can't just return, because we might have some dialog work to
			// do.  For example, the user might have typed in a directory, not
			// a file, so we have to catch it, change the dialog, and not return
			// any filename yet.

			UT_cloneString(szDialogFilename, gtk_file_selection_get_filename(pFS));
			UT_ASSERT(szDialogFilename);

			err = stat(szDialogFilename, &buf);
			UT_ASSERT(err == 0 || err == -1);
			
			// Check for a directory entered as filename.  When true,
			// set the filter properly and continue in the selection
			if (err == 0 && S_ISDIR(buf.st_mode))
			{
				GString * s = g_string_new(szDialogFilename);
				if (s->str[s->len - 1] != '/')
				{
					g_string_append_c(s, '/');
				}
				gtk_file_selection_set_filename(pFS, s->str);
				g_string_free(s, TRUE);

				// free the string and continue along
				FREEP(szDialogFilename);
				continue;
			}

			UT_cloneString(m_szFinalPathnameCandidate, szDialogFilename);
			
			// if we got here, the text wasn't a directory, so it's a file,
			// and life is good
			return (m_answer == a_OK);
		}
	}		
		
	// if bCheckWritePermission is set, we're looking to SAVE a file.

	while(1)
	{
		gtk_main();
		if (m_answer == a_CANCEL)			// The easy way out
			return UT_FALSE;

		// Give us a filename we can mangle

		UT_cloneString(szDialogFilename, gtk_file_selection_get_filename(pFS));
		UT_ASSERT(szDialogFilename);

		// We append the suffix of the default type, so the user doesn't
		// have to.  This is adapted from the Windows front-end code
		// (xap_Win32Dlg_FileOpenSaveAs.cpp), since it should act the same.
		{
			//UT_uint32 end = UT_pointerArrayLength((void **) m_szSuffixes);

			GtkWidget * activeItem = gtk_menu_get_active(GTK_MENU(
				gtk_option_menu_get_menu(GTK_OPTION_MENU(filetypes_pulldown))));
			UT_ASSERT(activeItem);

			UT_sint32 nFileType = GPOINTER_TO_INT(gtk_object_get_user_data(
				GTK_OBJECT(activeItem)));

			// set to first item, which should probably be auto detect
			// TODO : "probably" isn't very good.
			UT_uint32 nIndex = 0;
			
			// the index in the types table will match the index in the suffix
			// table.  nFileType is the data we are searching for.
			for (UT_uint32 i = 0; m_nTypeList[i]; i++)
			{
				if (m_nTypeList[i] == nFileType)
				{
					nIndex = i;
					break;
				}
			}

			// if the file doesn't have a suffix already, and the file type
			// is normal (special types are negative, like auto detect),
			// slap a suffix on it.
			if ((!UT_pathSuffix(szDialogFilename)) &&
				(nFileType > 0))
			{
				// add suffix based on selected file type

				const char * szSuffix = UT_pathSuffix(m_szSuffixes[nIndex]);
				UT_ASSERT(szSuffix);

				UT_uint32 length = strlen(szDialogFilename) + strlen(szSuffix) + 1;
				szFinalPathname = (char *)calloc(length,sizeof(char));
				if (szFinalPathname)
				{
					char * p = szFinalPathname;

					strcpy(p,szDialogFilename);
					strcat(p,szSuffix);
				}
			}
			else
			{
				// the file type is special (auto detect)

				// set to plain name, and let the auto detector in the
				// exporter figure it out
				UT_cloneString(szFinalPathname,szDialogFilename);
			}
			// free szDialogFilename since it's been put into szFinalPathname (with
			// or without changes) and it's invalid (missing an extension which
			// might have been appended)
			FREEP(szDialogFilename);
		}

		UT_cloneString(szFinalPathnameCopy, szFinalPathname);
		
		err = stat(szFinalPathnameCopy, &buf);
		UT_ASSERT(err == 0 || err == -1);
			
		// Does the filename already exist?

		if (err == 0 && S_ISREG(buf.st_mode))
		{
			// we have an existing file, ask to overwrite

			if (_askOverwrite_YesNo(pFrame, szFinalPathname))
			{
				UT_cloneString(m_szFinalPathnameCandidate, szFinalPathname);
				goto ReturnTrue;
			}

			goto ContinueLoop;
		}
			
		// Check for a directory entered as filename.  When true,
		// set the filter properly and continue in the selection

		if (err == 0 && S_ISDIR(buf.st_mode))
		{
			GString * s = g_string_new(szFinalPathnameCopy);
			if (s->str[s->len - 1] != '/')
			{
				g_string_append_c(s, '/');
			}
			gtk_file_selection_set_filename(pFS, s->str);
			g_string_free(s, TRUE);
			goto ContinueLoop;
		}

		// We have a string that may contain a path, and may have a file
		// at the end.  First, strip off a file (if it exists), and test
		// for a matching directory.  We can then proceed with the file
		// if another stat of that dir passes.

		pLastSlash = rindex(szFinalPathnameCopy,'/');
		if (!pLastSlash)
		{
			_notifyError_OKOnly(pFrame,XAP_STRING_ID_DLG_InvalidPathname);
			goto ContinueLoop;
		}

		// Trim the pathname at beginning of the filename
		// keeping the trailing slash.
			
		pLastSlash[1] = 0;

		// Stat the directory left over

		err = stat(szFinalPathnameCopy, &buf);
		UT_ASSERT(err == 0 || err == -1);

		// If this directory doesn't exist, we have been feed garbage
		// at some point.  Throw an error and continue with the selection.

		if (err == -1)
		{
			_notifyError_OKOnly(pFrame,XAP_STRING_ID_DLG_NoSaveFile_DirNotExist);
			goto ContinueLoop;
		}

		// Since the stat passed the last test, we will make sure the
		// directory is suitable for writing, since we know it exists.

		UT_ASSERT(S_ISDIR(buf.st_mode));

		if (!access(szFinalPathnameCopy, W_OK))
		{
			// we've got what we need, save it to the candidate
			UT_cloneString(m_szFinalPathnameCandidate, szFinalPathname);
			goto ReturnTrue;
		}

		// complain about write permission on the directory.
		// lop off ugly trailing slash only if we don't have
		// the root dir ('/') for a path

		if (pLastSlash > szFinalPathnameCopy)
			*pLastSlash = 0;

		_notifyError_OKOnly(pFrame,XAP_STRING_ID_DLG_NoSaveFile_DirNotWriteable,
							szFinalPathname);
	ContinueLoop:
		FREEP(szFinalPathnameCopy);
	}

	/*NOTREACHED*/

ReturnTrue:
	FREEP(szFinalPathnameCopy);
	FREEP(szFinalPathname);
	return UT_TRUE;

#endif	

		   
	FREEP(szPersistDirectory);
	m_pQNXFrame = NULL;

	return;
}
