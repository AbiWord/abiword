/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ut_string.h"
#include "ut_assert.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_CocoaDlg_FileOpenSaveAs.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "xap_Strings.h"
#include "xap_Prefs.h"
#include "ut_debugmsg.h"

#include "ut_png.h"
#include "ut_svg.h"
#include "ut_misc.h"
#include "gr_CocoaGraphics.h"
#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"

#include "gr_CocoaImage.h"

#include <sys/stat.h>

#include "../../../wp/impexp/xp/ie_types.h"
#include "../../../wp/impexp/xp/ie_imp.h"
#include "../../../wp/impexp/xp/ie_impGraphic.h"

#define PREVIEW_WIDTH  100
#define PREVIEW_HEIGHT 100

/*****************************************************************/
XAP_Dialog * XAP_CocoaDialog_FileOpenSaveAs::static_constructor(XAP_DialogFactory * pFactory,
															 XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_FileOpenSaveAs * p = new XAP_CocoaDialog_FileOpenSaveAs(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_FileOpenSaveAs::XAP_CocoaDialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory,
														   XAP_Dialog_Id dlgid)
  : XAP_Dialog_FileOpenSaveAs(pDlgFactory,dlgid)
{
}

XAP_CocoaDialog_FileOpenSaveAs::~XAP_CocoaDialog_FileOpenSaveAs(void)
{
}

/*****************************************************************/
#if 0
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

static gint s_preview_exposed(GtkWidget * /* widget */,
			      GdkEventExpose * /* pExposeEvent */,
			      gpointer ptr)
{
        XAP_CocoaDialog_FileOpenSaveAs * dlg = static_cast<XAP_CocoaDialog_FileOpenSaveAs *> (ptr);
	UT_ASSERT(dlg);
	dlg->previewPicture();
	return FALSE;
}

static gint s_filename_select (GtkCList * w,
			       gint row, gint col, GdkEvent * evt,
			       gpointer ptr)
{
  XAP_CocoaDialog_FileOpenSaveAs * dlg = static_cast<XAP_CocoaDialog_FileOpenSaveAs *> (ptr);

  UT_ASSERT(dlg);
  dlg->previewPicture();
  return FALSE;
}


bool XAP_CocoaDialog_FileOpenSaveAs::_run_gtk_main(XAP_Frame * pFrame,
													 void * pFSvoid,
													 bool bCheckWritePermission,
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

			UT_cloneString(szDialogFilename, gtk_file_selection_get_filename(pFS));
			UT_ASSERT(szDialogFilename);

			err = stat(szFinalPathname, &buf);
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
			return false;

		// Give us a filename we can mangle

		UT_cloneString(szDialogFilename, gtk_file_selection_get_filename(pFS));
		UT_ASSERT(szDialogFilename);

		// We append the suffix of the default type, so the user doesn't
	        // have to.  This is adapted from the Windows front-end code
		// (xap_Win32Dlg_FileOpenSaveAs.cpp), since it should act the same.
		// If, however, the user doesn't want suffixes, they don't have to have them.  

		{
			//UT_uint32 end = UT_pointerArrayLength((void **) m_szSuffixes);

			GtkWidget * activeItem = gtk_menu_get_active(GTK_MENU(
				gtk_option_menu_get_menu(GTK_OPTION_MENU(filetypes_pulldown))));
			UT_ASSERT(activeItem);

			UT_sint32 nFileType = GPOINTER_TO_INT(g_object_get_user_data(
				G_OBJECT(activeItem)));

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

			pPrefs->getPrefsValueBool((const XML_Char *)XAP_PREF_KEY_UseSuffix, &wantSuffix);

			UT_DEBUGMSG(("UseSuffix: %d\n", wantSuffix));

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
					
					szFinalPathname = (char *)UT_calloc(length,sizeof(char));
					
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

		pLastSlash = strrchr(szFinalPathnameCopy,'/');
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


bool XAP_CocoaDialog_FileOpenSaveAs::_askOverwrite_YesNo(XAP_Frame * pFrame, const char * fileName)
{
	return (pFrame->showMessageBox(XAP_STRING_ID_DLG_OverwriteFile,
										XAP_Dialog_MessageBox::b_YN,
										XAP_Dialog_MessageBox::a_NO, // should this be YES?
										fileName)
						== XAP_Dialog_MessageBox::a_YES);
}
	
void XAP_CocoaDialog_FileOpenSaveAs::_notifyError_OKOnly(XAP_Frame * pFrame, XAP_String_Id sid)
{
	pFrame->showMessageBox(sid,
							XAP_Dialog_MessageBox::b_O,
							XAP_Dialog_MessageBox::a_OK);
}

void XAP_CocoaDialog_FileOpenSaveAs::_notifyError_OKOnly(XAP_Frame * pFrame,
														XAP_String_Id sid,
														const char * sz1)
{
	pFrame->showMessageBox(sid,
							XAP_Dialog_MessageBox::b_O,
							XAP_Dialog_MessageBox::a_OK,
							sz1);
}
#endif

/*****************************************************************/

void XAP_CocoaDialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	m_pCocoaFrame = (XAP_CocoaFrame *)pFrame;
	UT_ASSERT(m_pCocoaFrame);
	
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	// do we want to let this function handle stating the Cocoa
	// directory for writability?  Save/Export operations will want
	// this, open/import will not.

	bool bCheckWritePermission = false;

	const XML_Char * szTitle = NULL;
	const XML_Char * szFileTypeLabel = NULL;
	switch (m_id)
	{
	case XAP_DIALOG_ID_INSERT_PICTURE:
	  {
		m_panel = [NSOpenPanel openPanel];			// don't touch that one. It is garbage collected by the class itself
	        szTitle = pSS->getValue(XAP_STRING_ID_DLG_IP_Title);
		szFileTypeLabel = pSS->getValue(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel);
		bCheckWritePermission = false;    
	  }
	case XAP_DIALOG_ID_FILE_OPEN:
	{
		m_panel = [NSOpenPanel openPanel];			// don't touch that one. It is garbage collected by the class itself
		szTitle = pSS->getValue(XAP_STRING_ID_DLG_FOSA_OpenTitle);
		szFileTypeLabel = pSS->getValue(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel);
		bCheckWritePermission = false;
		break;
	}
	case XAP_DIALOG_ID_FILE_SAVEAS:
	{
		m_panel = [NSSavePanel savePanel];			// don't touch that one. It is garbage collected by the class itself
		szTitle = pSS->getValue(XAP_STRING_ID_DLG_FOSA_SaveAsTitle);
		szFileTypeLabel = pSS->getValue(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel);
		bCheckWritePermission = true;
		break;
	}
	case XAP_DIALOG_ID_FILE_IMPORT:
	  {
	  	m_panel = [NSOpenPanel openPanel];			// don't touch that one. It is garbage collected by the class itself
		szTitle = pSS->getValue(XAP_STRING_ID_DLG_FOSA_ImportTitle);
		szFileTypeLabel = pSS->getValue(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel);
		bCheckWritePermission = false;
	    break;
	  }
	case XAP_DIALOG_ID_FILE_EXPORT:
	  {
		m_panel = [NSSavePanel savePanel];			// don't touch that one. It is garbage collected by the class itself
		szTitle = pSS->getValue(XAP_STRING_ID_DLG_FOSA_ExportTitle);
		szFileTypeLabel = pSS->getValue(XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel);
		bCheckWritePermission = true;
	    break;
	  }
	case XAP_DIALOG_ID_INSERT_FILE:
	  {
		m_panel = [NSOpenPanel openPanel];			// don't touch that one. It is garbage collected by the class itself
		szTitle = pSS->getValue(XAP_STRING_ID_DLG_FOSA_InsertTitle);
		szFileTypeLabel = pSS->getValue(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel);
		bCheckWritePermission = false;
		break;
	  }
	case XAP_DIALOG_ID_PRINTTOFILE:
	{
		m_panel = [NSSavePanel savePanel];			// don't touch that one. It is garbage collected by the class itself
		szTitle = pSS->getValue(XAP_STRING_ID_DLG_FOSA_PrintToFileTitle);
		szFileTypeLabel = pSS->getValue(XAP_STRING_ID_DLG_FOSA_FilePrintTypeLabel);
		bCheckWritePermission = true;
		break;
	}
	default:
		m_panel = nil;
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	UT_ASSERT (m_panel);
	// NOTE: we use our string mechanism to localize the dialog's
	// NOTE: title and the error/confirmation message boxes.  we
	// NOTE: let Cocoa take care of the localization of the actual
	// NOTE: buttons and labels on the FileSelection dialog.

	[m_panel setTitle:[NSString stringWithCString:szTitle]];		// autoreleased string

// TODO	
//	connectFocus(GTK_WIDGET(pFS),pFrame);

//	GtkWidget * filetypes_pulldown = NULL;
	
	/*
	  To facilitate a file-types selection, we dig around in some
	  private data for the dialog layout, and add a drop-down list
	  of known types.  We store an indexer in the user data
	  for each menu item in the popup, so we can read the type
	  we need to return.
	*/
//	[m_panel setAccessoryView];
#if 0	// TODO
	{
		GtkWidget * main_vbox = pFS->main_vbox;
		UT_ASSERT(main_vbox);

		// hbox for our pulldown menu (GTK does its pulldown this way */
		GtkWidget * pulldown_hbox = gtk_hbox_new(FALSE, 15);
		gtk_box_pack_start(GTK_BOX(main_vbox), pulldown_hbox, TRUE, TRUE, 0);
		gtk_widget_show(pulldown_hbox);

		if (m_id == XAP_DIALOG_ID_INSERT_PICTURE)
		  {
			  GtkWidget * preview = createDrawingArea ();
		    gtk_widget_show (preview);
		    m_preview = preview;

		    GtkWidget * frame = gtk_frame_new (pSS->getValue(XAP_STRING_ID_DLG_IP_Activate_Label));
		    gtk_widget_show (frame);
		    gtk_container_add (GTK_CONTAINER(frame), preview);

		    gtk_box_pack_start(GTK_BOX(pulldown_hbox), frame, FALSE, TRUE, 0);
		    gtk_widget_set_usize (frame, PREVIEW_WIDTH, PREVIEW_HEIGHT);

		    // the expose event off the preview
		    g_signal_connect(G_OBJECT(preview),
				       "expose_event",
				       G_CALLBACK(s_preview_exposed),
				       static_cast<gpointer>(this));

		    g_signal_connect(G_OBJECT(pFS->file_list),
				       "select-row",
				       G_CALLBACK(s_filename_select),
				       static_cast<gpointer>(this));
		  }

		// pulldown label
		GtkWidget * filetypes_label = gtk_label_new(szFileTypeLabel);
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
			g_snprintf(buffer, 1024, "%s", pSS->getValue(XAP_STRING_ID_DLG_FOSA_FileTypeAutoDetect));
			thismenuitem = gtk_menu_item_new_with_label(buffer);
			g_object_set_user_data(G_OBJECT(thismenuitem), GINT_TO_POINTER(XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO));
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
					g_object_set_user_data(G_OBJECT(thismenuitem), GINT_TO_POINTER(m_nTypeList[i]));
					gtk_widget_show(thismenuitem);
					gtk_menu_append(GTK_MENU(menu), thismenuitem);
				}
			}

			// Set menu item to default type from index (i) above if we're a SAVEAS
				
			gtk_widget_show(menu);
			
			// add menu to the option menu widget
			gtk_option_menu_set_menu(GTK_OPTION_MENU(filetypes_pulldown), menu);

			// dialog; open dialog always does auto-detect
			// TODO: should this also apply to the open dialog?
			if (m_id == XAP_DIALOG_ID_FILE_SAVEAS)
			  {
				gtk_menu_set_active(GTK_MENU(menu), activeItemIndex + 1);
				gtk_option_menu_set_history (GTK_OPTION_MENU(filetypes_pulldown), activeItemIndex + 1);
			  }
		}
	}
#endif

#if 0
	g_signal_connect(G_OBJECT(pFS->ok_button), "clicked",
					   G_CALLBACK(s_ok_clicked), &m_answer);
	g_signal_connect(G_OBJECT(pFS->cancel_button), "clicked",
					   G_CALLBACK(s_cancel_clicked), &m_answer);
#endif
#if 0
	if (m_id == XAP_DIALOG_ID_FILE_OPEN || m_id == XAP_DIALOG_ID_INSERT_PICTURE || m_id == XAP_DIALOG_ID_FILE_INSERT || m_id == XAP_DIALOG_ID_INSERT_FILE) // only hide the buttons if we're opening a file/picture
	  gtk_file_selection_hide_fileop_buttons(pFS);
#endif

	// use the persistence info and/or the suggested filename
	// to properly seed the dialog.
	
	NSString * szPersistDirectory = nil;	// will be autoreleased
	NSString * szPersistFile = nil;
	
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

			szPersistDirectory = [NSString stringWithCString:m_szPersistPathname];	// autoreleased
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
			szPersistDirectory = [NSString stringWithCString:m_szInitialPathname];
//			NSURL * url = [NSURL fileURLWithPath:szPersistDirectory];
			szPersistFile = [NSString string];
		}
		else
		{
			// use directory(m_szInitialPathname)
			szPersistDirectory = [NSString stringWithCString:m_szInitialPathname];
			szPersistFile = [NSString string];
		}
	}


	int result = [m_panel runModalForDirectory:szPersistDirectory file:szPersistFile];
	
	if (result == NSFileHandlingPanelOKButton)
	{
		NSString * str = [m_panel filename];	// will be autoreleased.
		FREEP (m_szFinalPathname);	// free before reassigning
		m_szFinalPathname = (char *)malloc ([str cStringLength] + 1);
		UT_ASSERT (m_szFinalPathname);
		[str getCString:m_szFinalPathname];
		m_answer = a_OK;
	}
			  
	FREEP(szPersistDirectory);
	m_pCocoaFrame = NULL;

	return;
}

#if 0
int 
XAP_CocoaDialog_FileOpenSaveAs::previewPicture (void)
{
        UT_ASSERT (m_FS && m_preview);

	XAP_CocoaApp * unixapp = static_cast<XAP_CocoaApp *> (m_pApp);
	UT_ASSERT(unixapp);

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	// attach and clear the area immediately
	GR_CocoaGraphics* pGr = new GR_CocoaGraphics(m_preview->window, unixapp->getFontManager(), m_pApp);
	pGr->clearArea(0, 0, m_preview->allocation.width, m_preview->allocation.height);

	gchar * buf = gtk_file_selection_get_filename (m_FS);

	GR_Font * fnt = pGr->findFont("Times New Roman", "normal", "", "normal", "", "12pt");
	UT_ASSERT (fnt);		// not good if we don't have it.
	pGr->setFont(fnt);

	const XML_Char * str = pSS->getValue(XAP_STRING_ID_DLG_IP_No_Picture_Label);
	int len = strlen (str);
	UT_UCSChar * ucstext = new UT_UCSChar [len + 1]; 
	UT_UCS4_strcpy_char (ucstext, str);

	int answer = 0;

	UT_ByteBuf *pBB = NULL;
	FG_Graphic * pGraphic = 0;
	IE_ImpGraphic* pIEG = NULL;
	UT_Error errorCode = UT_OK;
	GR_Image *pImage = NULL;

	double		scale_factor = 0.0;
	UT_sint32     scaled_width,scaled_height;
	UT_sint32     iImageWidth,iImageHeight;

	if (!buf)
	  {
	    pGr->drawChars (ucstext, 0, len, 12, 35);
	    goto Cleanup;
	  }

	// are we dealing with a file or directory here?
	struct stat st;
	if (!stat (buf, &st)) {
		if (!S_ISREG(st.st_mode)) {
			pGr->drawChars (ucstext, 0, len, 12, 35);
			goto Cleanup;
		}
	}
	else {
		pGr->drawChars (ucstext, 0, len, 12, 35);
		goto Cleanup;
	}

	// Load File into memory
	pBB     = new UT_ByteBuf(0);
	pBB->insertFromFile(0, buf);

	// Build an Import Graphic based on file type
	errorCode = IE_ImpGraphic::constructImporter(buf, IEGFT_Unknown, &pIEG);
	if ((errorCode != UT_OK) || !pIEG)
	{
		DELETEP(pBB);
		pGr->drawChars (ucstext, 0, len, 12, 35);
		goto Cleanup;
	}

	errorCode = pIEG->importGraphic (pBB, &pGraphic);

	if ((errorCode != UT_OK) || !pGraphic)
	  {
	    pGr->drawChars (ucstext, 0, len, 12, 35);
	    goto Cleanup;
	  }

	if ( FGT_Raster == pGraphic->getType () )
	{
		pImage = new GR_CocoaImage(NULL);
		UT_ByteBuf * png = static_cast<FG_GraphicRaster*>(pGraphic)->getRaster_PNG();
		UT_PNG_getDimensions (png, iImageWidth, iImageHeight);

		if (m_preview->allocation.width >= iImageWidth && m_preview->allocation.height >= iImageHeight)
		  scale_factor = 1.0;
		else
		  scale_factor = MIN( (double) m_preview->allocation.width/iImageWidth,
				      (double) m_preview->allocation.height/iImageHeight);
		
		scaled_width  = (int)(scale_factor * iImageWidth);
		scaled_height = (int)(scale_factor * iImageHeight);

		pImage->convertFromBuffer(png, scaled_width, scaled_height);
		
		pGr->drawImage(pImage,
			       (int)((m_preview->allocation.width  - scaled_width ) / 2),
			       (int)((m_preview->allocation.height - scaled_height) / 2));
		
		answer = 1;
	}
	else // if ( FGT_Vector == pGraphic->getType () )
	{
	  //pImage = new GR_VectorImage(NULL);
	}

 Cleanup:
	DELETEP(pImage);
	DELETEP(pGr);
	DELETEP(fnt);
	DELETEPV(ucstext);

	return answer;
}
#endif

