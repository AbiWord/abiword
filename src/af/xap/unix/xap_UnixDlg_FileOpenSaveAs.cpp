/* AbiWord
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
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_Dialog_Id.h"
#include "ap_UnixDialog_FileOpenSaveAs.h"
#include "ap_UnixApp.h"
#include "ap_UnixFrame.h"

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

/*****************************************************************/

void AP_UnixDialog_FileOpenSaveAs::runModal(AP_Frame * pFrame)
{
	m_pUnixFrame = (AP_UnixFrame *)pFrame;
	UT_ASSERT(m_pUnixFrame);

	char * szTitle;
	switch (m_id)
	{
	case XAP_DIALOG_ID_FILE_OPEN:
		szTitle = "Open File";
		break;

	case XAP_DIALOG_ID_FILE_SAVEAS:
		szTitle = "Save File As";
		break;

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
	
	/* Run the dialog */

	gtk_widget_show(GTK_WIDGET(pFS));
	gtk_grab_add(GTK_WIDGET(pFS));
	gtk_main();

	if (m_answer == a_OK)
		UT_cloneString(m_szFinalPathname,
					   gtk_file_selection_get_filename(pFS));

	gtk_widget_destroy (GTK_WIDGET(pFS));

	FREEP(szPersistDirectory);
	m_pUnixFrame = NULL;

	return;
}

