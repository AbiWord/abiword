/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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

#undef GTK_DISABLE_DEPRECATED
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_UnixDialogHelper.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_UnixDlg_FileOpenSaveAs.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"
#include "xap_Strings.h"
#include "xap_Prefs.h"
#include "ut_debugmsg.h"
#include "ut_string_class.h"
#if GTK_CHECK_VERSION(2,4,0)
#include "ut_path.h"
#endif

#include "ut_png.h"
#include "ut_svg.h"
#include "ut_misc.h"
#include "gr_UnixGraphics.h"
#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"

#include "gr_UnixImage.h"
#include "gr_Painter.h"

#include <sys/stat.h>

#include "../../../wp/impexp/xp/ie_types.h"
#include "../../../wp/impexp/xp/ie_imp.h"
#include "../../../wp/impexp/xp/ie_impGraphic.h"

#define PREVIEW_WIDTH  100
#define PREVIEW_HEIGHT 100

/*****************************************************************/
XAP_Dialog * XAP_UnixDialog_FileOpenSaveAs::static_constructor(XAP_DialogFactory * pFactory,
															 XAP_Dialog_Id id)
{
	XAP_UnixDialog_FileOpenSaveAs * p = new XAP_UnixDialog_FileOpenSaveAs(pFactory,id);
	return p;
}

XAP_UnixDialog_FileOpenSaveAs::XAP_UnixDialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory,
														   XAP_Dialog_Id id)
#if GTK_CHECK_VERSION(2,4,0)
  : XAP_Dialog_FileOpenSaveAs(pDlgFactory,id), m_FC(0), m_preview(0), m_bExport(true)
#else
  : XAP_Dialog_FileOpenSaveAs(pDlgFactory,id), m_FS(0), m_preview(0), m_bExport(true)
#endif
{
	m_szFinalPathnameCandidate = NULL;
}

XAP_UnixDialog_FileOpenSaveAs::~XAP_UnixDialog_FileOpenSaveAs(void)
{
	FREEP(m_szFinalPathnameCandidate);
}

/*****************************************************************/

static void s_dialog_response(GtkWidget * /* widget */,
						gint answer,
						XAP_Dialog_FileOpenSaveAs::tAnswer * ptr)
{
	switch (answer)
	{
		case GTK_RESPONSE_CANCEL:
		case GTK_RESPONSE_ACCEPT:

			if (answer == GTK_RESPONSE_CANCEL)
				*ptr = XAP_Dialog_FileOpenSaveAs::a_CANCEL;
			else
				*ptr = XAP_Dialog_FileOpenSaveAs::a_OK;
			gtk_main_quit();
			break;
		default:
			// do nothing
			break;
	}
}

#if !GTK_CHECK_VERSION(2,4,0)
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
#endif

static void s_delete_clicked(GtkWidget * /* widget*/, gpointer /* data */, XAP_Dialog_FileOpenSaveAs::tAnswer * answer)
{
	*answer = XAP_Dialog_FileOpenSaveAs::a_CANCEL;
	gtk_main_quit();
}

static gint s_preview_exposed(GtkWidget * /* widget */,
			      GdkEventExpose * /* pExposeEvent */,
			      gpointer ptr)
{
        XAP_UnixDialog_FileOpenSaveAs * dlg = static_cast<XAP_UnixDialog_FileOpenSaveAs *> (ptr);
	UT_ASSERT(dlg);
	dlg->previewPicture();
	return FALSE;
}

static void s_filetypechanged(GtkWidget * w, gpointer p)
{
	XAP_UnixDialog_FileOpenSaveAs * dlg = static_cast<XAP_UnixDialog_FileOpenSaveAs *>(p);
	dlg->fileTypeChanged(w);
}

static gint
fsel_key_event (GtkWidget *widget, GdkEventKey *event, XAP_Dialog_FileOpenSaveAs::tAnswer * answer)
{
#ifdef GDK_Escape
	if (event->keyval == GDK_Escape) {
		g_signal_emit_stop_by_name (G_OBJECT (widget), "key_press_event");
		s_cancel_clicked ( widget, answer ) ;
		return TRUE;
	}
#endif

	return FALSE;
}

static void s_file_activated(GtkWidget * w, XAP_Dialog_FileOpenSaveAs::tAnswer * answer)
{
	s_dialog_response(w, GTK_RESPONSE_ACCEPT, answer);
}

static void file_selection_changed  (GtkTreeSelection  *selection,
				     gpointer           ptr)
{
  XAP_UnixDialog_FileOpenSaveAs * dlg = static_cast<XAP_UnixDialog_FileOpenSaveAs *> (ptr);

  UT_ASSERT(dlg);
  dlg->previewPicture();
}

bool XAP_UnixDialog_FileOpenSaveAs::_run_gtk_main(XAP_Frame * pFrame,
													 bool bCheckWritePermission,
													 GtkWidget * filetypes_pulldown)
{
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

	char * szDialogFilename = NULL;		// this is the file name returned from the dialog
	char * szFinalPathname = NULL;		// this is the file name after suffix addition, if any
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
				return false;

			// TODO  check for symlinks, because even symlinks to dirs won't
			// TODO  show up with S_ISDIR().

			// TODO  check to make sure a file exists before we close off the
			// TODO  loop
			
			// We can't just return, because we might have some dialog work to
			// do.  For example, the user might have typed in a directory, not
			// a file, so we have to catch it, change the dialog, and not return
			// any filename yet.

#if GTK_CHECK_VERSION(2,4,0)
			UT_cloneString(szDialogFilename, gtk_file_chooser_get_filename(m_FC));
#else
			UT_cloneString(szDialogFilename, gtk_file_selection_get_filename(m_FS));
#endif
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
#if GTK_CHECK_VERSION(2,4,0)				
				gtk_file_chooser_set_filename(m_FC, s->str);
#else
				gtk_file_selection_set_filename(m_FS, s->str);
#endif
				g_string_free(s, TRUE);

				// free the string and continue along
				FREEP(szDialogFilename);
				continue;
			}

			UT_cloneString(m_szFinalPathnameCandidate, szDialogFilename);
			if(szDialogFilename) FREEP(szDialogFilename);
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
			return false;

		// Give us a filename we can mangle

#if GTK_CHECK_VERSION(2,4,0)	
		UT_cloneString(szDialogFilename, gtk_file_chooser_get_filename(m_FC));
#else
		UT_cloneString(szDialogFilename, gtk_file_selection_get_filename(m_FS));
#endif
		if (!szDialogFilename)
			continue;

		// We append the suffix of the default type, so the user doesn't
	        // have to.  This is adapted from the Windows front-end code
		// (xap_Win32Dlg_FileOpenSaveAs.cpp), since it should act the same.
		// If, however, the user doesn't want suffixes, they don't have to have them.  

		{
			//UT_uint32 end = UT_pointerArrayLength(static_cast<void **>(m_szSuffixes));

			GtkWidget * activeItem = gtk_menu_get_active(GTK_MENU(
				gtk_option_menu_get_menu(GTK_OPTION_MENU(filetypes_pulldown))));
			UT_ASSERT(activeItem);

			UT_sint32 nFileType = GPOINTER_TO_INT(g_object_get_data(
				G_OBJECT(activeItem), "user_data"));

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
			bool wantSuffix = true;
			
			XAP_Prefs *pPrefs= pFrame->getApp()->getPrefs();

			pPrefs->getPrefsValueBool(static_cast<const XML_Char *>(XAP_PREF_KEY_UseSuffix), &wantSuffix);

			UT_DEBUGMSG(("UseSuffix: %d\n", wantSuffix));

			// do not want suffix for directory names
			err = stat(szDialogFilename, &buf);
			if (S_ISDIR(buf.st_mode))
				wantSuffix = false;

			// if the file doesn't have a suffix already, and the file type
			// is normal (special types are negative, like auto detect),
			// and the user wants extensions, slap a suffix on it.   
			if ((!UT_pathSuffix(szDialogFilename)) && 
			    (nFileType > 0) && wantSuffix)                                
				{                                                       
					// add suffix based on selected file type       
					const char * szSuffix = UT_pathSuffix(m_szSuffixes[nIndex]);
					UT_ASSERT(szSuffix);                            
					UT_uint32 length = strlen(szDialogFilename) + strlen(szSuffix) + 1;
					
					szFinalPathname = static_cast<char *>(UT_calloc(length,sizeof(char)));
					
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
#if GTK_CHECK_VERSION(2,4,0)			
			gtk_file_chooser_set_filename(m_FC, s->str);
#else
			gtk_file_selection_set_filename(m_FS, s->str);
#endif
			g_string_free(s, TRUE);
			goto ContinueLoop;
		}

		// We have a string that may contain a path, and may have a file
		// at the end.  First, strip off a file (if it exists), and test
		// for a matching directory.  We can then proceed with the file
		// if another stat of that dir passes.

		if (szFinalPathnameCopy && strlen(szFinalPathnameCopy))
			pLastSlash = strrchr(szFinalPathnameCopy,'/');
		else
			pLastSlash = NULL;

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
	return true;
}


bool XAP_UnixDialog_FileOpenSaveAs::_askOverwrite_YesNo(XAP_Frame * pFrame, const char * fileName)
{
	return (pFrame->showMessageBox(XAP_STRING_ID_DLG_OverwriteFile,
				       XAP_Dialog_MessageBox::b_YN,
				       XAP_Dialog_MessageBox::a_NO, // should this be YES?
				       fileName)
		== XAP_Dialog_MessageBox::a_YES);
}
	
void XAP_UnixDialog_FileOpenSaveAs::_notifyError_OKOnly(XAP_Frame * pFrame, XAP_String_Id sid)
{
	pFrame->showMessageBox(sid,
			       XAP_Dialog_MessageBox::b_O,
			       XAP_Dialog_MessageBox::a_OK);
}

void XAP_UnixDialog_FileOpenSaveAs::_notifyError_OKOnly(XAP_Frame * pFrame,
							XAP_String_Id sid,
							const char * sz1)
{
	pFrame->showMessageBox(sid,
			       XAP_Dialog_MessageBox::b_O,
			       XAP_Dialog_MessageBox::a_OK,
			       sz1);
}

void XAP_UnixDialog_FileOpenSaveAs::fileTypeChanged(GtkWidget * w)
{
	if (!m_bExport)
		return;

	UT_sint32 nFileType = GPOINTER_TO_INT(g_object_get_data(
				G_OBJECT(w), "user_data"));
	UT_DEBUGMSG(("File type widget is %x filetype number is %d \n",w,nFileType));
	if(nFileType == 0)
	{
		return;
	}
#if GTK_CHECK_VERSION(2,4,0)			
	UT_String sFileName = 	gtk_file_chooser_get_filename(m_FC);
#else
	UT_String sFileName = 	gtk_file_selection_get_filename(m_FS);
#endif
	UT_String sSuffix = m_szSuffixes[nFileType-1];
	sSuffix = sSuffix.substr(1,sSuffix.length()-1);
	UT_sint32 i = 0;
	bool bFoundComma = false;
	for(i=0; i< static_cast<UT_sint32>(sSuffix.length()); i++)
	{
		if(sSuffix[i] == ';')
		{
			bFoundComma = true;
			break;
		}
	}
	if(bFoundComma)
	{
		sSuffix = sSuffix.substr(0,i);
	}
//
// Hard code a suffix
//
	if(strstr(sSuffix.c_str(),"gz") != NULL)
	{
		sSuffix = ".zabw";
	}
	bool bFoundSuffix = false;
	for(i= sFileName.length()-1; i> 0; i--)
	{
		if(sFileName[i] == '.')
		{
			bFoundSuffix = true;
			break;
		}
	}
	if(!bFoundSuffix)
	{
		return;
	}
	sFileName = sFileName.substr(0,i);
	sFileName += sSuffix;
	
#if GTK_CHECK_VERSION(2,4,0)
	if (!gtk_file_chooser_select_filename(m_FC,sFileName.c_str()))
	{
		gtk_file_chooser_set_current_name(m_FC, UT_basename(sFileName.c_str()));
	}
#else	
	gtk_file_selection_set_filename(m_FS,sFileName.c_str());
#endif
}

/*****************************************************************/

void XAP_UnixDialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	// do we want to let this function handle stating the Unix
	// directory for writability?  Save/Export operations will want
	// this, open/import will not.

	bool bCheckWritePermission = false;

	UT_UTF8String szTitle;
	UT_UTF8String szFileTypeLabel;
	switch (m_id)
	{
		case XAP_DIALOG_ID_INSERT_PICTURE:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_IP_Title, szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel, szFileTypeLabel);
				bCheckWritePermission = false;    
				break;
			}
		case XAP_DIALOG_ID_FILE_OPEN:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_OpenTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel,szFileTypeLabel);
				bCheckWritePermission = false;
				break;
			}
		case XAP_DIALOG_ID_FILE_IMPORT:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ImportTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel,szFileTypeLabel);
				bCheckWritePermission = false;
				break;
			}
		case XAP_DIALOG_ID_INSERT_FILE:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_InsertTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel,szFileTypeLabel);
				bCheckWritePermission = false;
				break;
			}
		case XAP_DIALOG_ID_FILE_SAVEAS:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_SaveAsTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel,szFileTypeLabel);
				bCheckWritePermission = true;
				break;
			}
		case XAP_DIALOG_ID_FILE_EXPORT:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ExportTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel,szFileTypeLabel);
				bCheckWritePermission = true;
				break;
			}
		case XAP_DIALOG_ID_PRINTTOFILE:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_PrintToFileTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FilePrintTypeLabel,szFileTypeLabel);
				bCheckWritePermission = true;
				break;
			}
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
	}
	m_bExport = bCheckWritePermission;

	// NOTE: we use our string mechanism to localize the dialog's
	// NOTE: title and the error/confirmation message boxes.  we
	// NOTE: let GTK take care of the localization of the actual
	// NOTE: buttons and labels on the FileSelection dialog.

#if GTK_CHECK_VERSION(2,4,0)
	// Get the GtkWindow of the parent frame
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(pFrame->getFrameImpl());
	GtkWidget * parent = pUnixFrameImpl->getTopLevelWindow();

	m_FC = GTK_FILE_CHOOSER( gtk_file_chooser_dialog_new (szTitle.utf8_str(),
									GTK_WINDOW(parent),
									(m_id == XAP_DIALOG_ID_FILE_OPEN || m_id == XAP_DIALOG_ID_INSERT_PICTURE || m_id == XAP_DIALOG_ID_INSERT_FILE ? GTK_FILE_CHOOSER_ACTION_OPEN : GTK_FILE_CHOOSER_ACTION_SAVE),
									GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
									GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
									NULL)
							);

	abiSetupModalDialog(GTK_DIALOG(m_FC), pFrame, this, GTK_RESPONSE_ACCEPT);
#else	
	GtkFileSelection *pFS = GTK_FILE_SELECTION(gtk_file_selection_new(szTitle.utf8_str()));
	m_FS = pFS;

	abiSetupModalDialog(GTK_DIALOG(pFS), pFrame, this, GTK_RESPONSE_CANCEL);
#endif
	GtkWidget * filetypes_pulldown = NULL;

	UT_UTF8String s;
	
	/*
	  To facilitate a file-types selection, we dig around (only for GTK < 2.4)
	  in some private data for the dialog layout, and add a drop-down list
	  of known types.  We store an indexer in the user data
	  for each menu item in the popup, so we can read the type
	  we need to return.
	*/
	{
#if !GTK_CHECK_VERSION(2,4,0)		
		GtkWidget * main_vbox = pFS->main_vbox;
		UT_ASSERT(main_vbox);
#endif
		
		// hbox for our pulldown menu (GTK does its pulldown this way */
		GtkWidget * pulldown_hbox = gtk_hbox_new(FALSE, 15);
#if !GTK_CHECK_VERSION(2,4,0)		
		gtk_box_pack_start(GTK_BOX(main_vbox), pulldown_hbox, TRUE, TRUE, 0);
#endif
		gtk_widget_show(pulldown_hbox);
#if GTK_CHECK_VERSION(2,4,0)
		gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER(m_FC), pulldown_hbox);
#endif

		if (m_id == XAP_DIALOG_ID_INSERT_PICTURE)
		  {
			  GtkWidget * preview = createDrawingArea ();
		    gtk_widget_show (preview);
			m_preview = preview;			  
#if GTK_CHECK_VERSION(2,4,0)
			gtk_widget_set_size_request (preview, PREVIEW_WIDTH, PREVIEW_HEIGHT);

			// place the preview area inside a container to get a nice border
			GtkWidget * preview_hbox = gtk_hbox_new(FALSE, 0);
			gtk_container_set_border_width  (GTK_CONTAINER(preview_hbox), 4);
			gtk_box_pack_start(GTK_BOX(preview_hbox), preview, TRUE, TRUE, 0);

			// attach the preview area to the dialog
			gtk_file_chooser_set_preview_widget (m_FC, preview_hbox);
			gtk_file_chooser_set_preview_widget_active (m_FC, true);

			// connect some signals
			g_signal_connect (m_FC, "update_preview",
									G_CALLBACK (file_selection_changed), static_cast<gpointer>(this));

			g_signal_connect (preview, "expose_event",
									G_CALLBACK (s_preview_exposed), static_cast<gpointer>(this));
#else
			pSS->getValueUTF8(XAP_STRING_ID_DLG_IP_Activate_Label,s);
		    GtkWidget * frame = gtk_frame_new (s.utf8_str());
			gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
		    gtk_widget_show (frame);
		    gtk_container_add (GTK_CONTAINER(frame), preview);

		    gtk_box_pack_start(GTK_BOX(pulldown_hbox), frame, FALSE, TRUE, 0);
		    gtk_widget_set_size_request (frame, PREVIEW_WIDTH, PREVIEW_HEIGHT);

		    // the expose event off the preview
		    g_signal_connect(G_OBJECT(preview),
				       "expose_event",
				       G_CALLBACK(s_preview_exposed),
				       static_cast<gpointer>(this));

		    g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (pFS->file_list)), "changed", G_CALLBACK (file_selection_changed), this);
#endif
		  }

		// pulldown label
		GtkWidget * filetypes_label = gtk_label_new(szFileTypeLabel.utf8_str());
		gtk_label_set_justify(GTK_LABEL(filetypes_label), GTK_JUSTIFY_RIGHT);
		gtk_misc_set_alignment(GTK_MISC(filetypes_label), 1.0, 0.5);
		gtk_widget_show(filetypes_label);

		int VOFFSET = 0;
		if (m_id == XAP_DIALOG_ID_INSERT_PICTURE)
		  VOFFSET = 40;

		GtkWidget * vboxTmp = gtk_vbox_new (FALSE, 0);
		gtk_widget_show (vboxTmp);
		gtk_box_pack_start (GTK_BOX(vboxTmp), filetypes_label, FALSE, FALSE, VOFFSET);
		gtk_box_pack_start(GTK_BOX(pulldown_hbox), vboxTmp, FALSE, TRUE, 0);		

		// pulldown menu
		filetypes_pulldown = gtk_option_menu_new();
		gtk_widget_show(filetypes_pulldown);

		// hack so that i can make this widget small vertically
		vboxTmp = gtk_vbox_new (FALSE, 0);
		gtk_widget_show (vboxTmp);
		gtk_box_pack_start (GTK_BOX(vboxTmp), filetypes_pulldown, FALSE, FALSE, VOFFSET);
		gtk_box_pack_end(GTK_BOX(pulldown_hbox), vboxTmp, FALSE, TRUE, 0);

		// put it in the right spot.
		//gtk_box_reorder_child(GTK_BOX(main_vbox), pulldown_hbox, 3);

		// do filters
		{
			GtkWidget * menu = gtk_menu_new();
			UT_ASSERT(menu);

			GtkWidget * thismenuitem = NULL;

			char buffer[1024];

			// Auto-detect is always an option, but a special one, so we use
			// a pre-defined constant for the type, and don't use the user-supplied
			// types yet.
			pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileTypeAutoDetect,s);
			g_snprintf(buffer, 1024, "%s", s.utf8_str());
			thismenuitem = gtk_menu_item_new_with_label(buffer);
			g_object_set_data(G_OBJECT(thismenuitem), "user_data", GINT_TO_POINTER(XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO));
			gtk_widget_show(thismenuitem);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), thismenuitem);

			UT_uint32 activeItemIndex = 0;
			
			// add list items
			{
				UT_ASSERT(UT_pointerArrayLength(reinterpret_cast<void **>(const_cast<char **>(m_szSuffixes))) ==
						  UT_pointerArrayLength(reinterpret_cast<void **>(const_cast<char **>(m_szDescriptions))));
				
				// measure one list, they should all be the same length
				UT_uint32 end = UT_pointerArrayLength(reinterpret_cast<void **>(const_cast<char **>(m_szDescriptions)));
			  
				for (UT_uint32 i = 0; i < end; i++)
				{
					// If this type is default, save its index (i) for later use
					if (m_nTypeList[i] == m_nDefaultFileType)
						activeItemIndex = i;
					
					g_snprintf(buffer, 1024, "%s", m_szDescriptions[i]);
					thismenuitem = gtk_menu_item_new_with_label(buffer);
					g_object_set_data(G_OBJECT(thismenuitem), "user_data", GINT_TO_POINTER(m_nTypeList[i]));
					gtk_widget_show(thismenuitem);
					gtk_menu_shell_append(GTK_MENU_SHELL(menu), thismenuitem);
//
// Attach a callback when it is activated to change the file suffix
//
					g_signal_connect(G_OBJECT(thismenuitem), "activate",
									 G_CALLBACK(s_filetypechanged),	
									 reinterpret_cast<gpointer>(this));
				}
			}

			// Set menu item to default type from index (i) above if we're a SAVEAS
				
			gtk_widget_show(menu);
			
			// add menu to the option menu widget
			gtk_option_menu_set_menu(GTK_OPTION_MENU(filetypes_pulldown), menu);
			m_wFileTypes_PullDown = filetypes_pulldown;
			// dialog; open dialog always does auto-detect
			// TODO: should this also apply to the open dialog?
			if (m_id == XAP_DIALOG_ID_FILE_SAVEAS)
			  {
				gtk_menu_set_active(GTK_MENU(menu), activeItemIndex + 1);
				gtk_option_menu_set_history (GTK_OPTION_MENU(filetypes_pulldown), activeItemIndex + 1);
			  }
		}
	}
	
	// connect the signals for OK and CANCEL and the requisite clean-close signals
#if GTK_CHECK_VERSION(2,4,0)
	g_signal_connect(G_OBJECT(m_FC),
							 "delete_event",
							 G_CALLBACK(s_delete_clicked),
							 &m_answer);
#else
	g_signal_connect(G_OBJECT(m_FS),
							 "delete_event",
							 G_CALLBACK(s_delete_clicked),
							 &m_answer);
#endif

#if GTK_CHECK_VERSION(2,4,0)
	g_signal_connect(G_OBJECT(m_FC),
			    "key_press_event",
			    G_CALLBACK(fsel_key_event), &m_answer);
#else
	g_signal_connect(G_OBJECT(m_FS),
			    "key_press_event",
			    G_CALLBACK(fsel_key_event), &m_answer);
#endif

#if GTK_CHECK_VERSION(2,4,0)
	g_signal_connect (G_OBJECT (m_FC),
				"response",
				G_CALLBACK(s_dialog_response), &m_answer);
#else
        g_signal_connect(G_OBJECT(pFS->ok_button), "clicked",
                                           G_CALLBACK(s_ok_clicked), &m_answer);
        g_signal_connect(G_OBJECT(pFS->cancel_button), "clicked",
                                           G_CALLBACK(s_cancel_clicked), &m_answer);
#endif
	
#if GTK_CHECK_VERSION(2,4,0)
	g_signal_connect (G_OBJECT (m_FC),
				"file-activated",
				G_CALLBACK(s_file_activated), &m_answer);	
#endif

#if !GTK_CHECK_VERSION(2,4,0)
	if (m_id == XAP_DIALOG_ID_FILE_OPEN || m_id == XAP_DIALOG_ID_INSERT_PICTURE || m_id == XAP_DIALOG_ID_FILE_EXPORT || m_id == XAP_DIALOG_ID_INSERT_FILE) // only hide the buttons if we're opening a file/picture
	  gtk_file_selection_hide_fileop_buttons(m_FS);
#endif

	// use the persistence info and/or the suggested filename
	// to properly seed the dialog.
	
	gchar * szPersistDirectory = NULL;	// we must free this

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

#if GTK_CHECK_VERSION(2,4,0)
			szPersistDirectory = g_path_get_dirname(m_szPersistPathname);
			gtk_file_chooser_set_current_folder(m_FC, szPersistDirectory);
#else
			UT_cloneString(szPersistDirectory,m_szPersistPathname);
			char * pLastSlash = strrchr(szPersistDirectory, '/');
			if (pLastSlash)
				pLastSlash[1] = 0;
			gtk_file_selection_set_filename(m_FS,szPersistDirectory);
#endif
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
#if GTK_CHECK_VERSION(2,4,0)			
			if (!g_path_is_absolute (m_szInitialPathname)) {
				gchar *dir = g_get_current_dir ();
				gchar *file = m_szInitialPathname;
				m_szInitialPathname = g_build_filename (dir, file, NULL);
				g_free (dir);
				g_free (file);
			}
			gtk_file_chooser_set_filename(m_FC, m_szInitialPathname);
#else
			gtk_file_selection_set_filename(m_FS, m_szInitialPathname);
#endif
		}
		else
		{
			// use directory(m_szInitialPathname)
#if GTK_CHECK_VERSION(2,4,0)	
			szPersistDirectory = g_path_get_dirname(m_szInitialPathname);
			gtk_file_chooser_set_current_folder(m_FC, szPersistDirectory);
#else
			UT_cloneString(szPersistDirectory,m_szInitialPathname);
			char * pLastSlash = strrchr(szPersistDirectory, '/');
			if (pLastSlash)
				pLastSlash[1] = 0;
			gtk_file_selection_set_filename(m_FS,szPersistDirectory);
#endif
		}
	}

#if GTK_CHECK_VERSION(2,4,0)
	// center the dialog
	centerDialog(parent, GTK_WIDGET(m_FC));

	gtk_widget_show(GTK_WIDGET(m_FC));
	gtk_grab_add(GTK_WIDGET(m_FC));
#else
	// get top level window and its GtkWidget *
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(pFrame->getFrameImpl());
	UT_ASSERT(pUnixFrameImpl);
	GtkWidget * parent = pUnixFrameImpl->getTopLevelWindow();
	UT_ASSERT(parent);

	// center it
	centerDialog(parent, GTK_WIDGET(pFS));
	
	gtk_widget_show(GTK_WIDGET(pFS));
	gtk_grab_add(GTK_WIDGET(pFS));
#endif
	
	bool bResult = _run_gtk_main(pFrame,bCheckWritePermission,filetypes_pulldown);
	
	if (bResult)
	{
		UT_ASSERT(m_szFinalPathnameCandidate);
		
		// store final path name and file type
		UT_cloneString(m_szFinalPathname, m_szFinalPathnameCandidate);

		FREEP(m_szFinalPathnameCandidate);
		
		// what a long ugly line of code
		GtkWidget * activeItem = gtk_menu_get_active(GTK_MENU(gtk_option_menu_get_menu(GTK_OPTION_MENU(filetypes_pulldown))));
		UT_ASSERT(activeItem);
		m_nFileType = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(activeItem), "user_data"));
	}

#if GTK_CHECK_VERSION(2,4,0)
	if(m_FC && GTK_IS_WIDGET(m_FC))
	  gtk_widget_destroy (GTK_WIDGET(m_FC));
#else	
	if(m_FS && GTK_IS_WIDGET(m_FS))
	  gtk_widget_destroy (GTK_WIDGET(m_FS));
#endif

	FREEP(szPersistDirectory);

	return;
}

gint XAP_UnixDialog_FileOpenSaveAs::previewPicture (void)
{
#if GTK_CHECK_VERSION(2,4,0)	
	UT_ASSERT (m_FC && m_preview);
#else
	UT_ASSERT (m_FS && m_preview);
#endif

	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
	UT_ASSERT(unixapp);

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	// attach and clear the area immediately
	//GR_UnixGraphics* pGr = new GR_UnixGraphics(m_preview->window, unixapp->getFontManager(), m_pApp);
	GR_UnixAllocInfo ai(m_preview->window, unixapp->getFontManager(), m_pApp);
	GR_UnixGraphics* pGr = (GR_UnixGraphics*) XAP_App::getApp()->newGraphics(ai);

#if GTK_CHECK_VERSION(2,4,0)	
	const gchar * file_name = gtk_file_chooser_get_filename (m_FC);
#else
	const gchar * file_name = gtk_file_selection_get_filename (m_FS);
#endif
	
	GR_Font * fnt = pGr->findFont("Times New Roman", "normal", "", "normal", "", "12pt");
	pGr->setFont(fnt);

	UT_UTF8String str;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_IP_No_Picture_Label, str);

	int answer = 0;

	UT_ByteBuf *pBB = NULL;
	FG_Graphic * pGraphic = 0;
	IE_ImpGraphic* pIEG = NULL;
	UT_Error errorCode = UT_OK;
	GR_Image *pImage = NULL;

	double		scale_factor = 0.0;
	UT_sint32     scaled_width,scaled_height;
	UT_sint32     iImageWidth,iImageHeight;

	{
	GR_Painter painter(pGr);
	painter.clearArea(0, 0, pGr->tlu(m_preview->allocation.width), pGr->tlu(m_preview->allocation.height));

	if (!file_name)
	  {
		painter.drawChars (str.ucs4_str().ucs4_str(), 0, str.size(), pGr->tlu(12), pGr->tlu(static_cast<int>(m_preview->allocation.height / 2)) - pGr->getFontHeight(fnt)/2);
	    goto Cleanup;
	  }

	// are we dealing with a file or directory here?
	struct stat st;
	if (!stat (file_name, &st)) {
		if (!S_ISREG(st.st_mode)) {
			painter.drawChars (str.ucs4_str().ucs4_str(), 0, str.size(), pGr->tlu(12), pGr->tlu(static_cast<int>(m_preview->allocation.height / 2)) - pGr->getFontHeight(fnt)/2);
			goto Cleanup;
		}
	}
	else {
		painter.drawChars (str.ucs4_str().ucs4_str(), 0, str.size(), pGr->tlu(12), pGr->tlu(static_cast<int>(m_preview->allocation.height / 2)) - pGr->getFontHeight(fnt)/2);
		goto Cleanup;
	}

	// Load File into memory
	pBB     = new UT_ByteBuf(0);
	pBB->insertFromFile(0, file_name);

	// Build an Import Graphic based on file type
	errorCode = IE_ImpGraphic::constructImporter(file_name, IEGFT_Unknown, &pIEG);
	if ((errorCode != UT_OK) || !pIEG)
	{
		DELETEP(pBB);
		painter.drawChars (str.ucs4_str().ucs4_str(), 0, str.size(), pGr->tlu(12), pGr->tlu(static_cast<int>(m_preview->allocation.height / 2)) - pGr->getFontHeight(fnt)/2);
		goto Cleanup;
	}

	errorCode = pIEG->importGraphic (pBB, &pGraphic);

	if ((errorCode != UT_OK) || !pGraphic)
	  {
		painter.drawChars (str.ucs4_str().ucs4_str(), 0, str.size(), pGr->tlu(12), pGr->tlu(static_cast<int>(m_preview->allocation.height / 2)) - pGr->getFontHeight(fnt)/2);
	    goto Cleanup;
	  }

	if ( FGT_Raster == pGraphic->getType () )
	{
		pImage = new GR_UnixImage(NULL);

		UT_ByteBuf * png = static_cast<FG_GraphicRaster*>(pGraphic)->getRaster_PNG();
		UT_PNG_getDimensions (png, iImageWidth, iImageHeight);

		if (m_preview->allocation.width >= iImageWidth && m_preview->allocation.height >= iImageHeight)
		  scale_factor = 1.0;
		else
		  scale_factor = MIN( static_cast<double>(m_preview->allocation.width)/iImageWidth,
				      static_cast<double>(m_preview->allocation.height)/iImageHeight);
		
		scaled_width  = static_cast<int>(scale_factor * iImageWidth);
		scaled_height = static_cast<int>(scale_factor * iImageHeight);

		pImage->convertFromBuffer(png, scaled_width, scaled_height);
		
		painter.drawImage(pImage,
			       pGr->tlu(static_cast<int>((m_preview->allocation.width  - scaled_width ) / 2)),
			       pGr->tlu(static_cast<int>((m_preview->allocation.height - scaled_height) / 2)));
		
		answer = 1;
	}
	else
	{
	  UT_ASSERT_NOT_REACHED ();
	}
	}
	
 Cleanup:
	DELETEP(pImage);
	DELETEP(pGr);
	DELETEP(pIEG);
	DELETEP(pGraphic);

	return answer;
}
