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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_Dialog_Id.h"
#include "xap_Dialog_MessageBox.h"
#include "xap_UnixDialog_FileOpenSaveAs.h"
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

UT_Bool AP_UnixDialog_FileOpenSaveAs::askOverwrite_YesNo(AP_Frame * pFrame, const char * fileName)
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

	
void AP_UnixDialog_FileOpenSaveAs::notifyError_OKOnly(AP_Frame * pFrame, const char * message)
{
	AP_DialogFactory * pDialogFactory
		= (AP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_MessageBox * pDialog
		= (AP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	pDialog->setMessage(message);
	pDialog->setButtons(AP_Dialog_MessageBox::b_O);
//	pDialog->setDefaultAnswer(AP_Dialog_MessageBox::a_YES);

	pDialog->runModal(pFrame);

//	AP_Dialog_MessageBox::tAnswer ans = pDialog->getAnswer();

	pDialogFactory->releaseDialog(pDialog);
}


/*****************************************************************/

void AP_UnixDialog_FileOpenSaveAs::runModal(AP_Frame * pFrame)
{
	m_pUnixFrame = (AP_UnixFrame *)pFrame;
	UT_ASSERT(m_pUnixFrame);

	// do we want to let this function handle stating the Unix
	// directory for writability?  Save/Export operations will want
	// this, open/import will not.
	bool bCheckWritePermission;
	
	char * szTitle;
	switch (m_id)
	{
	case XAP_DIALOG_ID_FILE_OPEN:
	{
		szTitle = "Open File";
		bCheckWritePermission = false;
		break;
	}
	case XAP_DIALOG_ID_FILE_SAVEAS:
	{
		szTitle = "Save File As";
		bCheckWritePermission = true;
		break;
	}
	case XAP_DIALOG_ID_PRINTTOFILE:
	{
		szTitle = "Print To File";
		bCheckWritePermission = true;
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

	// TODO Do we really want to position at the cursor?
	// gtk_window_position(GTK_WINDOW(pFS), GTK_WIN_POS_MOUSE);

	gtk_file_selection_hide_fileop_buttons(pFS);

	// use the persistence info and/or the suggested filename
	// to properly seed the dialog.
	
	char * szPersistDirectory = NULL;
	
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

	// TODO decide if we should put in a default wildcard suffix...

	gtk_widget_show(GTK_WIDGET(pFS));
	gtk_grab_add(GTK_WIDGET(pFS));
	
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

	bool bWriteIt = false;
	if (bCheckWritePermission)
	{
		char * szTestFilename;		
		struct stat buf;
		int err;
		
		while(1)
		{
			gtk_main();

			// The easy way out
			if (m_answer == a_CANCEL)
				break;

			// Give us a filename we can mangle
			UT_cloneString(szTestFilename, gtk_file_selection_get_filename(pFS));
			UT_ASSERT(szTestFilename);

			err = stat(szTestFilename, &buf);
			UT_ASSERT(err == 0 || err == -1);
			
			// Does the filename already exist?
			if (err == 0 && S_ISREG(buf.st_mode))
			{
				// we have an existing file, ask to overwrite
				if (askOverwrite_YesNo(pFrame, szTestFilename))
				{
					bWriteIt = true;
					break;
				}
				else
					continue;
				// we could add a "cancel" here, but as it works,
				// if the user chickens out and doesn't overwrite,
				// the cancel button on the save dialog still works
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
				continue;
			}

			// We have a string that may contain a path, and may have a file
			// at the end.  First, strip off a file (if it exists), and test
			// for a matching directory.  We can then proceed with the file
			// if another stat of that dir passes.
			
			// TODO : check that strlen() won't walk into garbage,
			// and it shouldn't if GTK always returns a terminated string.
			int i = 0;
			for (i = strlen(szTestFilename); i > 0; i--)
				if (szTestFilename[i] == '/')
					break;
			szTestFilename[i+1] = NULL;
			
			// Stat the directory left over
			err = stat(szTestFilename, &buf);
			UT_ASSERT(err == 0 || err == -1);

			// If this directory doesn't exist, we have been feed garbage
			// at some point.  Throw an error and continue with the selection.
			if (err == -1)
			{
				notifyError_OKOnly(pFrame,
								   "This file cannot be saved in this directory\n"
								   "because the directory does not exist.");
				continue;
			}

			// Since the stat passed the last test, we will make sure the
			// directory is suitable for writing, since we know it exists.
			UT_ASSERT(S_ISDIR(buf.st_mode));

			if (!access(szTestFilename, W_OK))
			{
				bWriteIt = true;
				break;
			}
			else
			{
				unsigned int length = strlen(szTestFilename);
				
				char message[length + 512];

				// lop off ugly trailing slash only if we don't have
				// the root dir ('/') for a path
				if (length > 1)
					szTestFilename[length - 1] = NULL;
				
				sprintf(message, "This file cannot be saved here because the directory\n"
						"'%s' is write-protected.", szTestFilename);
				
				notifyError_OKOnly(pFrame, message);

				continue;
			}

			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}
	else
		// with no wrapper
		gtk_main();
			
	if (bWriteIt)
		UT_cloneString(m_szFinalPathname, gtk_file_selection_get_filename(pFS));		
			   
	gtk_widget_destroy (GTK_WIDGET(pFS));

	FREEP(szPersistDirectory);
	m_pUnixFrame = NULL;

	return;
}
