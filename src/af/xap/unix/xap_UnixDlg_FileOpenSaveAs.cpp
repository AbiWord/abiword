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

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_dialogHelper.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_UnixDlg_FileOpenSaveAs.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)

/*****************************************************************/
AP_Dialog * AP_UnixDialog_FileOpenSaveAs::static_constructor(AP_DialogFactory * pFactory,
															 AP_Dialog_Id id)
{
	AP_UnixDialog_FileOpenSaveAs * p = new AP_UnixDialog_FileOpenSaveAs(pFactory,id);
	return p;
}

AP_UnixDialog_FileOpenSaveAs::AP_UnixDialog_FileOpenSaveAs(AP_DialogFactory * pDlgFactory,
														   AP_Dialog_Id id)
	: AP_Dialog_FileOpenSaveAs(pDlgFactory,id)
{
}

AP_UnixDialog_FileOpenSaveAs::~AP_UnixDialog_FileOpenSaveAs(void)
{
}

/*****************************************************************/

static void s_ok_clicked(GtkWidget * widget,
						 AP_Dialog_FileOpenSaveAs::tAnswer * answer)
{
	*answer = AP_Dialog_FileOpenSaveAs::a_OK;
	gtk_main_quit();
}

static void s_cancel_clicked(GtkWidget * widget,
							 AP_Dialog_FileOpenSaveAs::tAnswer * answer)
{
	*answer = AP_Dialog_FileOpenSaveAs::a_CANCEL;
	gtk_main_quit();
}

UT_Bool AP_UnixDialog_FileOpenSaveAs::_run_gtk_main(XAP_Frame * pFrame,
													void * pFSvoid,
													UT_Bool bCheckWritePermission)
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

	if (!bCheckWritePermission)
	{
		gtk_main();
		return (m_answer == a_OK);
	}		

	char * szTestFilename = NULL;		// we must free this
	char * pLastSlash;
	struct stat buf;
	int err;
		
	while(1)
	{
		gtk_main();
		if (m_answer == a_CANCEL)			// The easy way out
			return UT_FALSE;
		
		// Give us a filename we can mangle

		UT_cloneString(szTestFilename, gtk_file_selection_get_filename(pFS));
		UT_ASSERT(szTestFilename);

		err = stat(szTestFilename, &buf);
		UT_ASSERT(err == 0 || err == -1);
			
		// Does the filename already exist?

		if (err == 0 && S_ISREG(buf.st_mode))
		{
			// we have an existing file, ask to overwrite

			if (_askOverwrite_YesNo(pFrame, szTestFilename))
				goto ReturnTrue;

			goto ContinueLoop;
		}
			
		// Check for a directory entered as filename.  When true,
		// set the filter properly and continue in the selection

		if (err == 0 && S_ISDIR(buf.st_mode))
		{
			GString * s = g_string_new(szTestFilename);
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

		pLastSlash = rindex(szTestFilename,'/');
		if (!pLastSlash)
		{
			_notifyError_OKOnly(pFrame,"Invalid pathname.");
			goto ContinueLoop;
		}

		// Trim the pathname at beginning of the filename
		// keeping the trailing slash.
			
		pLastSlash[1] = 0;

		// Stat the directory left over

		err = stat(szTestFilename, &buf);
		UT_ASSERT(err == 0 || err == -1);

		// If this directory doesn't exist, we have been feed garbage
		// at some point.  Throw an error and continue with the selection.

		if (err == -1)
		{
			_notifyError_OKOnly(pFrame,
								"This file cannot be saved in this directory\n"
								"because the directory does not exist.");
			goto ContinueLoop;
		}

		// Since the stat passed the last test, we will make sure the
		// directory is suitable for writing, since we know it exists.

		UT_ASSERT(S_ISDIR(buf.st_mode));

		if (!access(szTestFilename, W_OK))
			goto ReturnTrue;

		// complain about write permission on the directory.
		// lop off ugly trailing slash only if we don't have
		// the root dir ('/') for a path

		if (pLastSlash > szTestFilename)
			*pLastSlash = 0;

		_notifyError_OKOnly(pFrame,
							"This file cannot be saved here because the directory\n'%s' is write-protected.",
							szTestFilename);
	ContinueLoop:
		FREEP(szTestFilename);
	}

	/*NOTREACHED*/

ReturnTrue:
	FREEP(szTestFilename);
	return UT_TRUE;
}


UT_Bool AP_UnixDialog_FileOpenSaveAs::_askOverwrite_YesNo(XAP_Frame * pFrame, const char * fileName)
{
	// return UT_TRUE if we should overwrite the file
	AP_DialogFactory * pDialogFactory
		= (AP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_MessageBox * pDialog
		= (AP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	pDialog->setMessage("File already exists.  Overwrite file '%s'?", fileName);
	pDialog->setButtons(AP_Dialog_MessageBox::b_YN);
	pDialog->setDefaultAnswer(AP_Dialog_MessageBox::a_NO);	// should this be YES?

	pDialog->runModal(pFrame);

	AP_Dialog_MessageBox::tAnswer ans = pDialog->getAnswer();

	pDialogFactory->releaseDialog(pDialog);

	return (ans == AP_Dialog_MessageBox::a_YES);
}

	
void AP_UnixDialog_FileOpenSaveAs::_notifyError_OKOnly(XAP_Frame * pFrame, const char * message)
{
	AP_DialogFactory * pDialogFactory
		= (AP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_MessageBox * pDialog
		= (AP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	pDialog->setMessage(message);
	pDialog->setButtons(AP_Dialog_MessageBox::b_O);
	pDialog->setDefaultAnswer(AP_Dialog_MessageBox::a_OK);

	pDialog->runModal(pFrame);

//	AP_Dialog_MessageBox::tAnswer ans = pDialog->getAnswer();

	pDialogFactory->releaseDialog(pDialog);
}

void AP_UnixDialog_FileOpenSaveAs::_notifyError_OKOnly(XAP_Frame * pFrame,
													   const char * message, const char * sz1)
{
	AP_DialogFactory * pDialogFactory
		= (AP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_MessageBox * pDialog
		= (AP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	pDialog->setMessage(message,sz1);
	pDialog->setButtons(AP_Dialog_MessageBox::b_O);
	pDialog->setDefaultAnswer(AP_Dialog_MessageBox::a_OK);

	pDialog->runModal(pFrame);

//	AP_Dialog_MessageBox::tAnswer ans = pDialog->getAnswer();

	pDialogFactory->releaseDialog(pDialog);
}


/*****************************************************************/

void AP_UnixDialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	m_pUnixFrame = (XAP_UnixFrame *)pFrame;
	UT_ASSERT(m_pUnixFrame);

	// do we want to let this function handle stating the Unix
	// directory for writability?  Save/Export operations will want
	// this, open/import will not.

	UT_Bool bCheckWritePermission;
	
	char * szTitle;
	switch (m_id)
	{
	case XAP_DIALOG_ID_FILE_OPEN:
	{
		szTitle = "Open File";
		bCheckWritePermission = UT_FALSE;
		break;
	}
	case XAP_DIALOG_ID_FILE_SAVEAS:
	{
		szTitle = "Save File As";
		bCheckWritePermission = UT_TRUE;
		break;
	}
	case XAP_DIALOG_ID_PRINTTOFILE:
	{
		szTitle = "Print To File";
		bCheckWritePermission = UT_TRUE;
		break;
	}
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	
	GtkFileSelection *pFS = (GtkFileSelection *)gtk_file_selection_new(szTitle);
	
	/* Connect the signals for OK and CANCEL */

	gtk_signal_connect(GTK_OBJECT(pFS->ok_button), "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked), &m_answer);
	gtk_signal_connect(GTK_OBJECT(pFS->cancel_button), "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked), &m_answer);

	gtk_file_selection_hide_fileop_buttons(pFS);

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
			char * pLastSlash = rindex(szPersistDirectory, '/');
			if (pLastSlash)
				pLastSlash[1] = 0;
			gtk_file_selection_set_filename(pFS,szPersistDirectory);
		}
		else
		{
			// no initial pathname given and we don't have
			// a pathname from a previous use, so just let
			// it come up in the current working directory.
		}
	}
	else
	{
		// we have an initial pathname (the name of the document
		// in the frame that we were invoked on).  if the caller
		// wanted us to suggest a filename, use the initial
		// pathname as is.  if not, use the directory portion of
		// it.

		if (m_bSuggestName)
		{
			// use m_szInitialPathname
			
			gtk_file_selection_set_filename(pFS, m_szInitialPathname);
		}
		else
		{
			// use directory(m_szInitialPathname)
			
			UT_cloneString(szPersistDirectory,m_szInitialPathname);
			char * pLastSlash = rindex(szPersistDirectory, '/');
			if (pLastSlash)
				pLastSlash[1] = 0;
			gtk_file_selection_set_filename(pFS,szPersistDirectory);
		}
	}

	// get top level window and it's GtkWidget *
	XAP_UnixFrame * frame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(frame);
	GtkWidget * parent = frame->getTopLevelWindow();
	UT_ASSERT(parent);
	// center it
    centerDialog(parent, GTK_WIDGET(pFS));
	gtk_window_set_transient_for(GTK_WINDOW(pFS), GTK_WINDOW(parent));
	
	gtk_widget_show(GTK_WIDGET(pFS));
	gtk_grab_add(GTK_WIDGET(pFS));

	UT_Bool bResult = _run_gtk_main(pFrame,pFS,bCheckWritePermission);
	
	if (bResult)
		UT_cloneString(m_szFinalPathname, gtk_file_selection_get_filename(pFS));		
			   
	gtk_widget_destroy (GTK_WIDGET(pFS));

	FREEP(szPersistDirectory);
	m_pUnixFrame = NULL;

	return;
}
