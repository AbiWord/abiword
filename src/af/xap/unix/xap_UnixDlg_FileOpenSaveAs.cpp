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
#include <strings.h>
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
#include "xap_Strings.h"

/*****************************************************************/
AP_Dialog * XAP_UnixDialog_FileOpenSaveAs::static_constructor(AP_DialogFactory * pFactory,
															 AP_Dialog_Id id)
{
	XAP_UnixDialog_FileOpenSaveAs * p = new XAP_UnixDialog_FileOpenSaveAs(pFactory,id);
	return p;
}

XAP_UnixDialog_FileOpenSaveAs::XAP_UnixDialog_FileOpenSaveAs(AP_DialogFactory * pDlgFactory,
														   AP_Dialog_Id id)
	: XAP_Dialog_FileOpenSaveAs(pDlgFactory,id)
{
}

XAP_UnixDialog_FileOpenSaveAs::~XAP_UnixDialog_FileOpenSaveAs(void)
{
}

/*****************************************************************/

static void s_ok_clicked(GtkWidget * /* widget */,
						 XAP_Dialog_FileOpenSaveAs::tAnswer * answer)
{
	*answer = XAP_Dialog_FileOpenSaveAs::a_OK;
	gtk_main_quit();
}

static void s_cancel_clicked(GtkWidget * /* widget */,
							 XAP_Dialog_FileOpenSaveAs::tAnswer * answer)
{
	*answer = XAP_Dialog_FileOpenSaveAs::a_CANCEL;
	gtk_main_quit();
}

static void s_delete_clicked(GtkWidget * /* widget*/, gpointer /* data */, XAP_Dialog_FileOpenSaveAs::tAnswer * answer)
{
	*answer = XAP_Dialog_FileOpenSaveAs::a_CANCEL;
	gtk_main_quit();
}

UT_Bool XAP_UnixDialog_FileOpenSaveAs::_run_gtk_main(XAP_Frame * pFrame,
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

	char * szTestFilename = NULL;		// we must free this
	char * pLastSlash;
	struct stat buf;
	int err;

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

			UT_cloneString(szTestFilename, gtk_file_selection_get_filename(pFS));
			UT_ASSERT(szTestFilename);

			err = stat(szTestFilename, &buf);
			UT_ASSERT(err == 0 || err == -1);
			
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

				// free the string and continue along
				FREEP(szTestFilename);
				continue;
			}

			// if we got here, the text wasn't a directory, so it's a file,
			// and life is good
			return (m_answer == a_OK);
		}
	}		
		
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
			_notifyError_OKOnly(pFrame,XAP_STRING_ID_DLG_InvalidPathname);
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
			_notifyError_OKOnly(pFrame,XAP_STRING_ID_DLG_NoSaveFile_DirNotExist);
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

		_notifyError_OKOnly(pFrame,XAP_STRING_ID_DLG_NoSaveFile_DirNotWriteable,
							szTestFilename);
	ContinueLoop:
		FREEP(szTestFilename);
	}

	/*NOTREACHED*/

ReturnTrue:
	FREEP(szTestFilename);
	return UT_TRUE;
}


UT_Bool XAP_UnixDialog_FileOpenSaveAs::_askOverwrite_YesNo(XAP_Frame * pFrame, const char * fileName)
{
	// return UT_TRUE if we should overwrite the file
	AP_DialogFactory * pDialogFactory
		= (AP_DialogFactory *)(pFrame->getDialogFactory());

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
}
	
void XAP_UnixDialog_FileOpenSaveAs::_notifyError_OKOnly(XAP_Frame * pFrame, XAP_String_Id sid)
{
	AP_DialogFactory * pDialogFactory
		= (AP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_MessageBox * pDialog
		= (XAP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	pDialog->setMessage(pSS->getValue(sid));
	pDialog->setButtons(XAP_Dialog_MessageBox::b_O);
	pDialog->setDefaultAnswer(XAP_Dialog_MessageBox::a_OK);

	pDialog->runModal(pFrame);

	pDialogFactory->releaseDialog(pDialog);
}

void XAP_UnixDialog_FileOpenSaveAs::_notifyError_OKOnly(XAP_Frame * pFrame,
														XAP_String_Id sid,
														const char * sz1)
{
	AP_DialogFactory * pDialogFactory
		= (AP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_MessageBox * pDialog
		= (XAP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	pDialog->setMessage(pSS->getValue(sid),sz1);
	pDialog->setButtons(XAP_Dialog_MessageBox::b_O);
	pDialog->setDefaultAnswer(XAP_Dialog_MessageBox::a_OK);

	pDialog->runModal(pFrame);

	pDialogFactory->releaseDialog(pDialog);
}

/*****************************************************************/

void XAP_UnixDialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	m_pUnixFrame = (XAP_UnixFrame *)pFrame;
	UT_ASSERT(m_pUnixFrame);

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	// do we want to let this function handle stating the Unix
	// directory for writability?  Save/Export operations will want
	// this, open/import will not.

	UT_Bool bCheckWritePermission;

	const XML_Char * szTitle;
	const XML_Char * szFileTypeLabel;
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
		break;
	}
	case XAP_DIALOG_ID_PRINTTOFILE:
	{
		szTitle = pSS->getValue(XAP_STRING_ID_DLG_FOSA_PrintToFileTitle);
		szFileTypeLabel = pSS->getValue(XAP_STRING_ID_DLG_FOSA_FilePrintTypeLabel);
		bCheckWritePermission = UT_TRUE;
		break;
	}
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	// NOTE: we use our string mechanism to localize the dialog's
	// NOTE: title and the error/confirmation message boxes.  we
	// NOTE: let GTK take care of the localization of the actual
	// NOTE: buttons and labels on the FileSelection dialog.
	
	GtkFileSelection *pFS = (GtkFileSelection *)gtk_file_selection_new(szTitle);

	GtkWidget * filetypes_pulldown = NULL;
	
	/*
	  To facilitate a file-types selection, we dig around in some
	  private data for the dialog layout, and add a drop-down list
	  of known types.  We store an indexer in the user data
	  for each menu item in the popup, so we can read the type
	  we need to return.
	*/
	{
		GtkWidget * main_vbox = pFS->main_vbox;
		UT_ASSERT(main_vbox);

		// hbox for our pulldown menu (GTK does its pulldown this way */
		GtkWidget * pulldown_hbox = gtk_hbox_new(TRUE, 10);
		gtk_box_pack_start(GTK_BOX(main_vbox), pulldown_hbox, FALSE, FALSE, 0);
		gtk_widget_show(pulldown_hbox);

		// pulldown label
		GtkWidget * filetypes_label = gtk_label_new(szFileTypeLabel);
		gtk_label_set_justify(GTK_LABEL(filetypes_label), GTK_JUSTIFY_RIGHT);
		gtk_misc_set_alignment(GTK_MISC(filetypes_label), 1.0, 0.5);
		gtk_widget_show(filetypes_label);
		gtk_box_pack_start(GTK_BOX(pulldown_hbox), filetypes_label, FALSE, TRUE, 0);
		
		// pulldown menu
		filetypes_pulldown = gtk_option_menu_new();
		gtk_widget_show(filetypes_pulldown);
		gtk_box_pack_end(GTK_BOX(pulldown_hbox), filetypes_pulldown, FALSE, TRUE, 0);

		// put it in the right spot.  3 might not be the right spot
		// in the future, near or far.  Oh well.
		gtk_box_reorder_child(GTK_BOX(main_vbox), pulldown_hbox, 3);

		// do filters
		{
			GtkWidget * menu = gtk_menu_new();
			UT_ASSERT(menu);

			GtkWidget * thismenuitem = NULL;

			char buffer[1024];

			// Auto-detect is always an option, but a special one, so we use
			// a pre-defined constant for the type, and don't use the user-supplied
			// types yet.
			g_snprintf(buffer, 1024, "%s", pSS->getValue(XAP_STRING_ID_DLG_FOSA_FileTypeAutoDetect));
			thismenuitem = gtk_menu_item_new_with_label(buffer);
			gtk_object_set_user_data(GTK_OBJECT(thismenuitem), GINT_TO_POINTER(XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO));
			gtk_widget_show(thismenuitem);
			gtk_menu_append(GTK_MENU(menu), thismenuitem);

			// add list items
			{
				UT_ASSERT(UT_pointerArrayLength((void **) m_szSuffixes) ==
						  UT_pointerArrayLength((void **) m_szDescriptions));
				
				// measure one list, they should all be the same length
				UT_uint32 end = UT_pointerArrayLength((void **) m_szDescriptions);
			  
				for (UT_uint32 i = 0; i < end; i++)
				{
					g_snprintf(buffer, 1024, "%s", m_szDescriptions[i]);
					thismenuitem = gtk_menu_item_new_with_label(buffer);
					gtk_object_set_user_data(GTK_OBJECT(thismenuitem), GINT_TO_POINTER(m_nTypeList[i]));
					gtk_widget_show(thismenuitem);
					gtk_menu_append(GTK_MENU(menu), thismenuitem);
				}
			}

			gtk_widget_show(menu);
			
			// add menu to the option menu widget
			gtk_option_menu_set_menu(GTK_OPTION_MENU(filetypes_pulldown), menu);
		}
	}
	
	// connect the signals for OK and CANCEL and the requisite clean-close signals
	gtk_signal_connect_after(GTK_OBJECT(pFS),
							 "destroy",
							 NULL,
							 NULL);
	gtk_signal_connect_after(GTK_OBJECT(pFS),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 &m_answer);

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
	{
		// store final path name and file type
		UT_cloneString(m_szFinalPathname, gtk_file_selection_get_filename(pFS));

		// what a long ugly line of code
		GtkWidget * activeItem = gtk_menu_get_active(GTK_MENU(gtk_option_menu_get_menu(GTK_OPTION_MENU(filetypes_pulldown))));
		UT_ASSERT(activeItem);
		m_nFileType = GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(activeItem)));
	}
			   
	gtk_widget_destroy (GTK_WIDGET(pFS));

	FREEP(szPersistDirectory);
	m_pUnixFrame = NULL;

	return;
}
