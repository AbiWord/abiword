/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

/*
 * Port to Maemo Development Platform 
 * Author: INdT - Renato Araujo <renato.filho@indt.org.br>
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h> // this include seems to fix 12332 (it defines GDK_Escape)
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
#include "xap_GtkComboBoxHelpers.h"
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
#include "ut_path.h"
#include "ut_png.h"
#include "ut_svg.h"
#include "ut_misc.h"
#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "ie_impGraphic.h"

#include "gr_UnixImage.h"
#include "gr_Painter.h"
#include "gr_UnixCairoGraphics.h"
#include "ut_bytebuf.h"

#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#include <hildon/hildon-file-chooser-dialog.h>
#endif


#include <sys/stat.h>

#include "../../../wp/impexp/xp/ie_types.h"
#include "../../../wp/impexp/xp/ie_imp.h"
#include "../../../wp/impexp/xp/ie_exp.h"
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
  : XAP_Dialog_FileOpenSaveAs(pDlgFactory,id), m_FC(0), m_preview(0), m_bSave(true)
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
						XAP_Dialog_FileOpenSaveAs::tAnswer * ptr, bool bQuit = true)
{
	switch (answer)
	{
		case GTK_RESPONSE_CANCEL:
		case GTK_RESPONSE_ACCEPT:
		case GTK_RESPONSE_OK:
			if (answer == GTK_RESPONSE_CANCEL)
				*ptr = XAP_Dialog_FileOpenSaveAs::a_CANCEL;
			else
				*ptr = XAP_Dialog_FileOpenSaveAs::a_OK;
			if (bQuit)
				gtk_main_quit();
			break;
		default:
			// do nothing
			break;
	}
}

static void dialog_response(GtkWidget *widget,
							gint answer,
							XAP_Dialog_FileOpenSaveAs::tAnswer * ptr) {
	s_dialog_response(widget, answer, ptr);
}

static void s_delete_clicked(GtkWidget 	* /*widget*/, 
							 GdkEvent 	* /*event*/, 
							 gpointer 	 data)
{
	XAP_UnixDialog_FileOpenSaveAs *dlg = static_cast<XAP_UnixDialog_FileOpenSaveAs *>(data);
	dlg->onDeleteCancel();
	gtk_main_quit();
}

#if GTK_CHECK_VERSION(3,0,0)
static gint s_preview_draw(GtkWidget * /* widget */,
			      cairo_t * /* cr */,
			      gpointer ptr)
#else
static gint s_preview_exposed(GtkWidget * /* widget */,
						   GdkEventExpose*,
			      gpointer ptr)
#endif
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
fsel_key_event (GtkWidget * widget, GdkEventKey * event, XAP_Dialog_FileOpenSaveAs::tAnswer * answer)
{
	if (event->keyval == GDK_KEY_Escape) {
		g_signal_stop_emission_by_name (G_OBJECT (widget), "key_press_event");
		s_dialog_response(widget, GTK_RESPONSE_CANCEL, answer);
		return TRUE;
	}

	return FALSE;
}

static void s_file_activated(GtkWidget * w, XAP_Dialog_FileOpenSaveAs::tAnswer * answer)
{
	// whenever the "file-activated" signal is called, it will also be followed
	// (or preceded?) by a "response" signal. That "response" signal will manage
	// the closing of the dialog for us. Now we don't want to close the dialog 
	// twice, hence the last 'false' parameter.
	// Hardly elegant, but none of this code is :/ It fixes bug #11647 too - MARCM.
	s_dialog_response(w, GTK_RESPONSE_ACCEPT, answer, false);
}

static void file_selection_changed  (GtkTreeSelection  * /*selection*/,
                                    gpointer           ptr)
{
  XAP_UnixDialog_FileOpenSaveAs * dlg = static_cast<XAP_UnixDialog_FileOpenSaveAs *> (ptr);

  UT_ASSERT(dlg);
  dlg->previewPicture();
}

bool XAP_UnixDialog_FileOpenSaveAs::_run_gtk_main(XAP_Frame * pFrame,
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

	// if m_bSave is not set, we're looking to OPEN a file.
	// otherwise we are looking to SAVE a file.
	if (!m_bSave)
	{
		while (1)
		{
			gtk_main();
			if (m_answer == a_CANCEL)			// The easy way out
				return false;

			m_szFinalPathnameCandidate = gtk_file_chooser_get_uri(m_FC);
			UT_ASSERT(m_szFinalPathnameCandidate);
			return (m_answer == a_OK);
		}
	}		
	else
	{		
		while(1)
		{
			gtk_main();
			if (m_answer == a_CANCEL)			// The easy way out
				return false;
	
			// Give us a filename we can mangle
	
			szDialogFilename = gtk_file_chooser_get_uri(m_FC);
			if (!szDialogFilename)
				continue;
	
			// We append the suffix of the default type, so the user doesn't
			// have to.  This is adapted from the Windows front-end code
			// (xap_Win32Dlg_FileOpenSaveAs.cpp), since it should act the same.
			// If, however, the user doesn't want suffixes, they don't have to have them.  
			{
				//UT_uint32 end = g_strv_length(m_szSuffixes);
   				UT_sint32 nFileType = XAP_comboBoxGetActiveInt(GTK_COMBO_BOX(filetypes_pulldown));
	
				// set to first item, which should probably be auto detect
				// TODO : "probably" isn't very good.
				UT_uint32 nIndex = 0;
				
				// the index in the types table will match the index in the suffix
				// table.  nFileType is the data we are searching for.
				if(m_nTypeList != NULL)
				{
					for (UT_uint32 i = 0; m_nTypeList[i]; i++)
					{
						if (m_nTypeList[i] == nFileType)
						{
							nIndex = i;
							break;
						}
					}
				}
				
				bool wantSuffix = true;
				XAP_Prefs *pPrefs= XAP_App::getApp()->getPrefs();
				pPrefs->getPrefsValueBool(static_cast<const gchar *>(XAP_PREF_KEY_UseSuffix), &wantSuffix);
				UT_DEBUGMSG(("UseSuffix: %d\n", wantSuffix));

				if (nFileType > 0 && getDialogId() != XAP_DIALOG_ID_FILE_SAVE_IMAGE) // 0 means autodetect
				{
					if (!UT_pathSuffix(szDialogFilename).empty())
					{
						// warn if we have a suffix that doesn't match the selected file type
						IE_ExpSniffer* pSniffer = IE_Exp::snifferForFileType(m_nTypeList[nIndex]);
						if (pSniffer && !pSniffer->recognizeSuffix(UT_pathSuffix(szDialogFilename).c_str()))
						{
							std::string msg;
							const XAP_StringSet * pSS = m_pApp->getStringSet();
							pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ExtensionDoesNotMatch, msg);
							if (pFrame->showMessageBox(msg.c_str(), XAP_Dialog_MessageBox::b_YN, XAP_Dialog_MessageBox::a_NO) != XAP_Dialog_MessageBox::a_YES)
								goto ContinueLoop;
						}
						szFinalPathname = g_strdup(szDialogFilename);
					}
					else if (wantSuffix)                                
					{
						// if the file doesn't have a suffix already, and the file type
						// is normal (special types are negative, like auto detect),
						// and the user wants extensions, slap a suffix on it.						
						// add suffix based on selected file type
                        // UT_UTF8String suffix (IE_Exp::preferredSuffixForFileType(m_nTypeList[nIndex]));
						// UT_uint32 length = strlen(szDialogFilename) + suffix.size() + 1;
						
						// szFinalPathname = static_cast<char *>(UT_calloc(length,sizeof(char)));
						
						// if (szFinalPathname)                            						
						// {                                               
						// 	char * p = szFinalPathname;             
						// 	strcpy(p,szDialogFilename);             
						// 	strcat(p,suffix.utf8_str());                     
						// }                                               

                        std::string n = m_appendDefaultSuffixFunctor( szDialogFilename,
                                                                      m_nTypeList[nIndex] );
                        szFinalPathname = g_strdup( n.c_str() );
					}
					else
						szFinalPathname = g_strdup(szDialogFilename);
				}
				else                                                    
				{                                                       
					// the file type is special (auto detect)       
					// set to plain name, and let the auto detector in the
					// exporter figure it out                       
					szFinalPathname = g_strdup(szDialogFilename);
				}                                                       

				// g_free szDialogFilename since it's been put into szFinalPathname (with
				// or without changes) and it's invalid (missing an extension which
				// might have been appended)                            
				
				FREEP(szDialogFilename);
			}	   

			szFinalPathnameCopy = g_strdup(szFinalPathname);

			if (UT_go_file_exists(szFinalPathnameCopy))
			{
				// we have an existing file, ask to overwrite
				if (_askOverwrite_YesNo(pFrame, szFinalPathname))
				{
					m_szFinalPathnameCandidate = g_strdup(szFinalPathname);
					goto ReturnTrue;
				}
	
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

			m_szFinalPathnameCandidate = g_strdup(szFinalPathname);
			goto ReturnTrue;

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
	} /* if m_bSave */
	
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
	if (!m_bSave)
		return;

	UT_sint32 nFileType = XAP_comboBoxGetActiveInt(GTK_COMBO_BOX(w));
	UT_DEBUGMSG(("File type widget is %p filetype number is %d \n",w,nFileType));
    // I have no idea for 0, but XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO
    // definitely means "skip this"
	if((nFileType == 0) || (nFileType == XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO))
	{
		return;
	}

	gchar * filename = gtk_file_chooser_get_filename(m_FC);
	UT_String sFileName = filename;
	FREEP(filename);

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
	
	gtk_file_chooser_set_current_name(m_FC, UT_basename(sFileName.c_str()));
}

void XAP_UnixDialog_FileOpenSaveAs::onDeleteCancel() 
{
	if (m_FC != NULL && gtk_widget_has_grab(GTK_WIDGET (m_FC))) {
		gtk_grab_remove (GTK_WIDGET (m_FC));
	}
	m_FC = NULL;
	m_answer = a_CANCEL;
}

/*****************************************************************/

void XAP_UnixDialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
    std::string szTitle;
    std::string szFileTypeLabel;
	
	switch (m_id)
	{
		case XAP_DIALOG_ID_INSERT_PICTURE:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_IP_Title, szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel, szFileTypeLabel);
				m_bSave = false;    
				break;
			}
		case XAP_DIALOG_ID_FILE_OPEN:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_OpenTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel,szFileTypeLabel);
				m_bSave = false;
				break;
			}
		case XAP_DIALOG_ID_FILE_IMPORT:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ImportTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel,szFileTypeLabel);
				m_bSave = false;
				break;
			}
		case XAP_DIALOG_ID_INSERTMATHML:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_InsertMath,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileInsertMath,szFileTypeLabel);
				m_bSave = false;
				break;
			}
		case XAP_DIALOG_ID_INSERTOBJECT:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_InsertObject,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileInsertObject,szFileTypeLabel);
				m_bSave = false;
				break;
			}
		case XAP_DIALOG_ID_INSERT_FILE:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_InsertTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel,szFileTypeLabel);
				m_bSave = false;
				break;
			}
		case XAP_DIALOG_ID_FILE_SAVEAS:
		case XAP_DIALOG_ID_FILE_SAVE_IMAGE:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_SaveAsTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel,szFileTypeLabel);
				m_bSave = true;
				break;
			}
		case XAP_DIALOG_ID_FILE_EXPORT:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ExportTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel,szFileTypeLabel);
				m_bSave = true;
				break;
			}
		case XAP_DIALOG_ID_PRINTTOFILE:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_PrintToFileTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FilePrintTypeLabel,szFileTypeLabel);
				m_bSave = true;
				break;
			}
		case XAP_DIALOG_ID_RECORDTOFILE:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_RecordToFileTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_RecordToFileLabel,szFileTypeLabel);
				m_bSave = true;
				break;
			}
		case XAP_DIALOG_ID_REPLAYFROMFILE:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ReplayFromFileTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ReplayFromFileLabel,szFileTypeLabel);
				m_bSave = false;
				break;
			}
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			m_bSave = false;
			break;
	}

	// NOTE: we use our string mechanism to localize the dialog's
	// NOTE: title and the error/confirmation message boxes.  we
	// NOTE: let GTK take care of the localization of the actual
	// NOTE: buttons and labels on the FileSelection dialog.

	// Get the GtkWindow of the parent frame
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(pFrame->getFrameImpl());
	GtkWidget * parent = pUnixFrameImpl->getTopLevelWindow();

	if(parent && (gtk_widget_is_toplevel(parent) != TRUE))
	{
        parent = gtk_widget_get_toplevel (parent);
	}

#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
	m_FC = GTK_FILE_CHOOSER( hildon_file_chooser_dialog_new(GTK_WINDOW(parent),
							(!m_bSave ? GTK_FILE_CHOOSER_ACTION_OPEN : GTK_FILE_CHOOSER_ACTION_SAVE))
							);
#else	
	m_FC = GTK_FILE_CHOOSER( gtk_file_chooser_dialog_new (szTitle.c_str(),
									GTK_WINDOW(parent),
									(!m_bSave ? GTK_FILE_CHOOSER_ACTION_OPEN : GTK_FILE_CHOOSER_ACTION_SAVE),
									GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
									(m_bSave ? GTK_STOCK_SAVE : GTK_STOCK_OPEN), GTK_RESPONSE_ACCEPT,
									(gchar*)NULL)
							);
#endif	
    
	gtk_file_chooser_set_local_only(m_FC, FALSE);

	abiSetupModalDialog(GTK_DIALOG(m_FC), pFrame, this, GTK_RESPONSE_ACCEPT);
	GtkWidget * filetypes_pulldown = NULL;

    std::string s;
	
	/*
	  Add a drop-down list of known types to facilitate a file-types selection. 
	  We store an indexer in the user data for each menu item in the popup, so 
	  we can read the type we need to return.
	*/

	if (m_id == XAP_DIALOG_ID_INSERT_PICTURE)
	{
		GtkWidget * preview = createDrawingArea ();
		gtk_widget_show (preview);
		m_preview = preview;			  
		gtk_widget_set_size_request (preview, PREVIEW_WIDTH, PREVIEW_HEIGHT);
		
		// place the preview area inside a container to get a nice border
		GtkWidget * preview_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_container_set_border_width  (GTK_CONTAINER(preview_hbox), 4);
		gtk_box_pack_start(GTK_BOX(preview_hbox), preview, TRUE, TRUE, 0);
		
		// attach the preview area to the dialog
		gtk_file_chooser_set_preview_widget (m_FC, preview_hbox);
		gtk_file_chooser_set_preview_widget_active (m_FC, true);
		
		// connect some signals
		g_signal_connect (m_FC, "update_preview",
								G_CALLBACK (file_selection_changed), static_cast<gpointer>(this));
		
#if GTK_CHECK_VERSION(3,0,0)
		g_signal_connect (preview, "draw",
								G_CALLBACK (s_preview_draw), static_cast<gpointer>(this));
#else
		g_signal_connect (preview, "expose_event",
								G_CALLBACK (s_preview_exposed), static_cast<gpointer>(this));
#endif
	}

#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
	filetypes_pulldown = gtk_combo_box_new();
	gtk_widget_show(filetypes_pulldown);
	GtkWidget * pulldown_hbox = filetypes_pulldown;
#else
	// hbox for our pulldown menu (GTK does its pulldown this way */
	GtkWidget * pulldown_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
	gtk_widget_show(pulldown_hbox);

	// pulldown label
	GtkWidget * filetypes_label = gtk_label_new_with_mnemonic(convertMnemonics(szFileTypeLabel).c_str());
	gtk_label_set_justify(GTK_LABEL(filetypes_label), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment(GTK_MISC(filetypes_label), 1.0, 0.5);
	gtk_widget_show(filetypes_label);
	gtk_box_pack_start(GTK_BOX(pulldown_hbox), filetypes_label, TRUE, TRUE, 0);		

	// pulldown menu
	filetypes_pulldown = gtk_combo_box_new();
	gtk_widget_show(filetypes_pulldown);
	gtk_box_pack_end(GTK_BOX(pulldown_hbox), filetypes_pulldown, TRUE, TRUE, 0);
    gtk_label_set_mnemonic_widget(GTK_LABEL(filetypes_label), filetypes_pulldown);
#endif
	//
	// add the filters to the dropdown list
	//
	GtkComboBox* combo = GTK_COMBO_BOX(filetypes_pulldown);
	XAP_makeGtkComboBoxText(combo, G_TYPE_INT);

	// Auto-detect is always an option, but a special one, so we use
	// a pre-defined constant for the type, and don't use the user-supplied
	// types yet.
	pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileTypeAutoDetect,s);
	XAP_appendComboBoxTextAndInt(combo, s.c_str(), XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO);

	UT_sint32 activeItemIndex = -1;
	
	// add list items
	if (m_szSuffixes)
	{
		UT_ASSERT(g_strv_length((gchar **) m_szSuffixes) == g_strv_length((gchar **) m_szDescriptions));
		
		// measure one list, they should all be the same length
		UT_uint32 end = g_strv_length((gchar **) m_szDescriptions);
	  
		for (UT_uint32 i = 0; i < end; i++)
		{
			// If this type is default, save its index (i) for later use
			if (m_nTypeList[i] == m_nDefaultFileType)
				activeItemIndex = i;
			
			XAP_appendComboBoxTextAndInt(combo, m_szDescriptions[i], m_nTypeList[i]);
//
// Attach a callback when it is activated to change the file suffix
//
//			g_signal_connect(G_OBJECT(thismenuitem), "activate",
//							 G_CALLBACK(s_filetypechanged),	
//							 reinterpret_cast<gpointer>(this));
		}
	}

	m_wFileTypes_PullDown = filetypes_pulldown;
	// dialog; open dialog always does auto-detect
	// TODO: should this also apply to the open dialog?
	if (m_id == XAP_DIALOG_ID_FILE_SAVEAS
        || m_id == XAP_DIALOG_ID_FILE_SAVE_IMAGE
        || ( m_id == XAP_DIALOG_ID_FILE_EXPORT && activeItemIndex >= 0 )
        )
	{
		gtk_combo_box_set_active(combo, activeItemIndex + 1);
	}
	else
	{ 
		gtk_combo_box_set_active(combo, 0);
	}
	
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
	hildon_file_chooser_dialog_add_extra ((HildonFileChooserDialog*)m_FC,
                                          pulldown_hbox);
#else
	gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER(m_FC), pulldown_hbox);
#endif

	// connect the signals for OK and CANCEL and the requisite clean-close signals
	g_signal_connect(G_OBJECT(m_FC),
							 "delete-event",
							 G_CALLBACK(s_delete_clicked),
							 this);

	g_signal_connect(G_OBJECT(m_FC),
			    "key_press_event",
			    G_CALLBACK(fsel_key_event), &m_answer);

	g_signal_connect (G_OBJECT (m_FC),
				"response",
				G_CALLBACK(dialog_response), &m_answer);
	
	g_signal_connect (G_OBJECT (m_FC),
				"file-activated",
				G_CALLBACK(s_file_activated), &m_answer);	

	g_signal_connect(G_OBJECT(filetypes_pulldown), "changed",
					 G_CALLBACK(s_filetypechanged),	
					 reinterpret_cast<gpointer>(this));

	// use the persistence info and/or the suggested filename
	// to properly seed the dialog.
	
	gchar * szPersistDirectory = NULL;	// we must g_free this

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

			szPersistDirectory = UT_go_dirname_from_uri(m_szPersistPathname, FALSE);
			gtk_file_chooser_set_current_folder_uri(m_FC, szPersistDirectory);
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
			xxx_UT_DEBUGMSG(("Iniitial filename is %s \n",m_szInitialPathname));
#if 0
			if (!g_path_is_absolute (m_szInitialPathname)) { // DAL: todo: is this correct?
				gchar *dir = g_get_current_dir ();
				gchar *file = m_szInitialPathname;
				gchar *filename = g_build_filename (dir, file, (gchar *)NULL);
				m_szInitialPathname = UT_go_filename_to_uri(filename);
				g_free(filename);
				g_free (dir);
				g_free (file);
			}
#endif
			if(m_id == XAP_DIALOG_ID_FILE_SAVEAS)
			{
				std::string szInitialSuffix = UT_pathSuffix(m_szInitialPathname);
				std::string szSaveTypeSuffix = IE_Exp::preferredSuffixForFileType(m_nDefaultFileType).utf8_str();
				if(!szInitialSuffix.empty() && !szSaveTypeSuffix.empty() 
					&& (szSaveTypeSuffix != szInitialSuffix))
				{
					std::string sFileName = m_szInitialPathname;
					std::string::size_type i = sFileName.find_last_of('.');
                    
					if(i != std::string::npos)
					{
						// erase to the end()
						sFileName.erase(i);
						sFileName += szSaveTypeSuffix;
						FREEP(m_szInitialPathname);
						m_szInitialPathname = g_strdup(sFileName.c_str());
					}
				}
			}
			if (UT_go_path_is_uri(m_szInitialPathname) || UT_go_path_is_path(m_szInitialPathname))
			{
				gtk_file_chooser_set_uri(m_FC, m_szInitialPathname);
			}
		}
		else
		{
			if (UT_go_path_is_uri(m_szInitialPathname) || UT_go_path_is_path(m_szInitialPathname))
			{
				szPersistDirectory = UT_go_dirname_from_uri(m_szInitialPathname, FALSE);
				gtk_file_chooser_set_current_folder_uri(m_FC, szPersistDirectory);
			}
			else
			{
				// we are dealing with a plain filename, not an URI or path, so 
				// just let it come up in the current working directory.
			}
		}
	}

	// center the dialog
	xxx_UT_DEBUGMSG(("before center IS WIDGET_TOP_LEVL %d \n",(GTK_WIDGET_TOPLEVEL(parent))));
	xxx_UT_DEBUGMSG(("before center IS WIDGET WINDOW %d \n",(GTK_IS_WINDOW(parent))));
	centerDialog(parent, GTK_WIDGET(m_FC));
	xxx_UT_DEBUGMSG(("After center IS WIDGET WINDOW %d \n",(GTK_IS_WINDOW(parent))));

	gtk_widget_show(GTK_WIDGET(m_FC));
	gtk_grab_add(GTK_WIDGET(m_FC));
	
	bool bResult = _run_gtk_main(pFrame,filetypes_pulldown);
	
	if (bResult)
	{
		UT_ASSERT(m_szFinalPathnameCandidate);
		
		// store final path name and file type
		m_szFinalPathname = g_strdup(m_szFinalPathnameCandidate);

		FREEP(m_szFinalPathnameCandidate);

		// what a long ugly line of code
		m_nFileType = XAP_comboBoxGetActiveInt(GTK_COMBO_BOX(filetypes_pulldown));
	}

	if (m_FC != NULL) {
		gtk_grab_remove (GTK_WIDGET(m_FC));
		gtk_widget_destroy (GTK_WIDGET(m_FC));
		m_FC = NULL;
		FREEP(szPersistDirectory);
	}

	return;
}

gint XAP_UnixDialog_FileOpenSaveAs::previewPicture (void)
{
	UT_ASSERT (m_FC && m_preview);

	UT_ASSERT(XAP_App::getApp());

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_return_val_if_fail( pSS, 0 );

	/* do not try to scale an image to a 1x1 pibuf, this might happen when
	   the widget size has notbeen allocated or if there is no room for it,
	   that might freeze gdk_pixbuf */
	GtkAllocation allocation;
	gtk_widget_get_allocation (m_preview, &allocation);
	if (allocation.width <= 1)
		return 0;
	
	// attach and clear the area immediately
	GR_UnixCairoAllocInfo ai(m_preview);
	GR_CairoGraphics* pGr =
		(GR_CairoGraphics*) XAP_App::getApp()->newGraphics(ai);

	const gchar * file_name = gtk_file_chooser_get_uri (m_FC);
	
	GR_Font * fnt = pGr->findFont("Times New Roman",
								  "normal", "", "normal",
								  "", "12pt",
								  pSS->getLanguageName());
	pGr->setFont(fnt);

	std::string s;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_IP_No_Picture_Label, s);
	UT_UTF8String str(s);

	int answer = 0;

	FG_Graphic * pGraphic = 0;
	GR_Image *pImage = NULL;

	double		scale_factor = 0.0;
	UT_sint32     scaled_width,scaled_height;
	UT_sint32     iImageWidth,iImageHeight;

	{
	GR_Painter painter(pGr);
	GtkAllocation alloc;
	gtk_widget_get_allocation(m_preview, &alloc);
	painter.clearArea(0, 0, pGr->tlu(alloc.width), pGr->tlu(alloc.height));

	if (!file_name)
	{
		painter.drawChars (str.ucs4_str().ucs4_str(), 0, str.size(), pGr->tlu(12), pGr->tlu(static_cast<int>(alloc.height / 2)) - pGr->getFontHeight(fnt)/2);
	    goto Cleanup;
	}

	// are we dealing with a file or directory here?
	struct stat st;
	if (!stat (file_name, &st)) 
	{
		if (!S_ISREG(st.st_mode)) 
		{
			painter.drawChars (str.ucs4_str().ucs4_str(), 0, str.size(), pGr->tlu(12), pGr->tlu(static_cast<int>(alloc.height / 2)) - pGr->getFontHeight(fnt)/2);
			goto Cleanup;
		}
	}

	GsfInput * input = NULL;
	UT_DEBUGMSG(("file_name %s \n",file_name));
	input = UT_go_file_open (file_name, NULL);
	if (!input)
		goto Cleanup;
	char Buf[4097] = "";  // 4096+nul ought to be enough
	UT_uint32 iNumbytes = UT_MIN(4096, gsf_input_size(input));
	gsf_input_read(input, iNumbytes, (guint8 *)(Buf));
	Buf[iNumbytes] = '\0';

	IEGraphicFileType ief = IE_ImpGraphic::fileTypeForContents(Buf,4096);
	if((ief == IEGFT_Unknown) || (ief == IEGFT_Bogus))
	{
		    painter.drawChars (str.ucs4_str().ucs4_str(), 0, str.size(), pGr->tlu(12), pGr->tlu(static_cast<int>(alloc.height / 2)) - pGr->getFontHeight(fnt)/2);
			g_object_unref (G_OBJECT (input));
			goto Cleanup;
	}
	g_object_unref (G_OBJECT (input));
	input = UT_go_file_open (file_name, NULL);
	size_t num_bytes = gsf_input_size(input);
	UT_Byte * bytes = (UT_Byte *) gsf_input_read(input, num_bytes,NULL );
	if(bytes == NULL)
	{
		    painter.drawChars (str.ucs4_str().ucs4_str(), 0, str.size(), pGr->tlu(12), pGr->tlu(static_cast<int>(alloc.height / 2)) - pGr->getFontHeight(fnt)/2);
			g_object_unref (G_OBJECT (input));
			goto Cleanup;
	}
	UT_ByteBuf * pBB = new UT_ByteBuf();
	pBB->append(bytes,num_bytes);
	g_object_unref (G_OBJECT (input));
	//
	// OK load the data into a GdkPixbuf
	//
	// Jean: comented out next line (and an other one below), we don't use it
	// bool bLoadFailed = false;
	//
	GdkPixbuf * pixbuf = pixbufForByteBuf ( pBB);
	delete pBB;
	if(pixbuf == NULL)
	{
		//
		// Try a fallback loader here.
		//
		painter.drawChars (str.ucs4_str().ucs4_str(), 0, str.size(), pGr->tlu(12), pGr->tlu(static_cast<int>(alloc.height / 2)) - pGr->getFontHeight(fnt)/2);
		// bLoadFailed = true;
	    goto Cleanup;
	}

	pImage = new GR_UnixImage(NULL,pixbuf);

	iImageWidth = gdk_pixbuf_get_width (pixbuf);
	iImageHeight = gdk_pixbuf_get_height (pixbuf);
	if (alloc.width >= iImageWidth && alloc.height >= iImageHeight)
		scale_factor = 1.0;
	else
		scale_factor = MIN( static_cast<double>(alloc.width)/iImageWidth,
							static_cast<double>(alloc.height)/iImageHeight);
		
	scaled_width  = static_cast<int>(scale_factor * iImageWidth);
	scaled_height = static_cast<int>(scale_factor * iImageHeight);

	static_cast<GR_UnixImage *>(pImage)->scale(scaled_width,scaled_height);	
	painter.drawImage(pImage,
					  pGr->tlu(static_cast<int>((alloc.width  - scaled_width ) / 2)),
					  pGr->tlu(static_cast<int>((alloc.height - scaled_height) / 2)));
		
	answer = 1;
	}
	
 Cleanup:
	FREEP(file_name);
	DELETEP(pImage);
	DELETEP(pGr);
	DELETEP(pGraphic);

	return answer;
}

GdkPixbuf *  XAP_UnixDialog_FileOpenSaveAs::_loadXPM(UT_ByteBuf * pBB)
{
	GdkPixbuf * pixbuf = NULL;
	const char * pBC = reinterpret_cast<const char *>(pBB->getPointer(0));

	UT_GenericVector<char*> vecStr;
	UT_sint32 k =0;
	UT_sint32 iBase =0;

	//
	// Find dimension line to start with.
	//
	UT_sint32 length = static_cast<UT_sint32>(pBB->getLength());
	for(k =0; ( k < length) && (*(pBC+k) != '"'); k++) {

	}

	if(k >= length)
	{
		return NULL;
	}

	k++;
	iBase = k;
	for( ; (k < length) && (*(pBC+k) != '"') ; k++) {

	}
	if(k >= length)
	{
		return NULL;
	}

	char * sz = NULL;
	UT_sint32 kLen = k-iBase+1;
	sz = static_cast<char *>(UT_calloc(kLen,sizeof(char)));
	UT_sint32 i =0;

	for(i=0; i< (kLen -1); i++)
	{
		*(sz+i) = *(pBC+iBase+i);
	}
	*(sz+i) = 0;
	vecStr.addItem(sz);

	//
	// Now loop through all the lines until we get to "}" outside the
	// '"'
	while((k < length) && (*(pBC+k) != '}'))
	{
		k++;

		//
		// Load a single string of data into our vector.
		// 
		if(*(pBC+k) =='"')
		{
			//
			// Start of a line
			//
			k++;
			iBase = k;
			for( ; (k < length) && (*(pBC+k) != '"'); k++) {

			}
			if(k >= length)
			{
				return NULL;
			}
			sz = NULL;
			kLen = k-iBase+1;
			sz = static_cast<char *>(UT_calloc(kLen,sizeof(char)));
			for(i=0; i<(kLen -1); i++)
			{
				*(sz+i) = *(pBC+iBase+i);
			}
			*(sz +i) = 0;
			vecStr.addItem(sz);
		}
	}

	if(k >= length)
	{
		for(i=0; i<vecStr.getItemCount(); i++)
		{
			char * psz = vecStr.getNthItem(i);
			FREEP(psz);
		}
		return NULL;
	}

	const char ** pszStr = static_cast<const char **>(UT_calloc(vecStr.getItemCount(),sizeof(char *)));
	for(i=0; i<vecStr.getItemCount(); i++)
		pszStr[i] = vecStr.getNthItem(i);
	pixbuf = gdk_pixbuf_new_from_xpm_data(pszStr);
	DELETEP(pszStr);
	return pixbuf;
}

GdkPixbuf *  XAP_UnixDialog_FileOpenSaveAs::pixbufForByteBuf (UT_ByteBuf * pBB)
{
	if ( !pBB || !pBB->getLength() )
		return NULL;

	GdkPixbuf * pixbuf = NULL;

	bool bIsXPM = false;
	const char * szBuf = reinterpret_cast<const char *>(pBB->getPointer(0));
	if((pBB->getLength() > 9) && (strncmp (szBuf, "/* XPM */", 9) == 0))
	{
		bIsXPM = true;
	}

	if(bIsXPM)
	{
		pixbuf = _loadXPM(pBB);
	}
	else
	{
		GError * err = 0;
		GdkPixbufLoader * ldr = 0;

		ldr = gdk_pixbuf_loader_new ();
		if (!ldr)
			{
				UT_DEBUGMSG (("GdkPixbuf: couldn't create loader! WTF?\n"));
				UT_ASSERT (ldr);
				return NULL ;
			}

		if ( FALSE== gdk_pixbuf_loader_write (ldr, static_cast<const guchar *>(pBB->getPointer (0)),
											  static_cast<gsize>(pBB->getLength ()), &err) )
			{
				UT_DEBUGMSG(("DOM: couldn't write to loader: %s\n", err->message));
				g_error_free(err);
				gdk_pixbuf_loader_close (ldr, NULL);
				g_object_unref (G_OBJECT(ldr));
				return NULL ;
			}
		
		gdk_pixbuf_loader_close (ldr, NULL);
		pixbuf = gdk_pixbuf_loader_get_pixbuf (ldr);

		// ref before closing the loader
		if ( pixbuf )
			g_object_ref (G_OBJECT(pixbuf));

		g_object_unref (G_OBJECT(ldr));
	}

	return pixbuf;
}
