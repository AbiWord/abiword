/* AbiWord
 * Copyright (C) 2001, 2002 Dom Lachowicz
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

#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_unixDirent.h"
#include "ut_path.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_New.h"
#include "ap_UnixDialog_New.h"

#include "xap_Dlg_FileOpenSaveAs.h"
#include "ie_imp.h"

#include "ut_string_class.h"

/*************************************************************************/

XAP_Dialog * AP_UnixDialog_New::static_constructor(XAP_DialogFactory * pFactory,
												   XAP_Dialog_Id id)
{
	AP_UnixDialog_New * p = new AP_UnixDialog_New(pFactory,id);
	return p;
}

AP_UnixDialog_New::AP_UnixDialog_New(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
  : AP_Dialog_New(pDlgFactory,id), m_pFrame(0), mRow(0), mCol(0)
{
}

AP_UnixDialog_New::~AP_UnixDialog_New(void)
{
  UT_VECTOR_PURGEALL(UT_String*, mTemplates);
}

void AP_UnixDialog_New::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);

	m_pFrame = pFrame;
	
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);
	
	/*connectFocus(GTK_WIDGET(mainWindow),pFrame);

	// To center the dialog, we need the frame of its parent.
	XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
	
	// Get the GtkWindow of the parent frame
	GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
	UT_return_if_fail(parentWindow);*/
	switch ( abiRunModalDialog ( GTK_DIALOG(mainWindow), pFrame, this,
								 BUTTON_CANCEL, false ) )
	{
		case BUTTON_OK:
			event_Ok (); break ;
		default:
			event_Cancel () ; break ;
	}
	
	abiDestroyWidget ( mainWindow ) ;
}

/*************************************************************************/
/*************************************************************************/

void AP_UnixDialog_New::event_Ok ()
{
	setAnswer (AP_Dialog_New::a_OK);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_radioExisting)))
	{
		setOpenType(AP_Dialog_New::open_Existing);
	}
	else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_radioNew)))
	{
	  UT_String * tmpl = (UT_String*)mTemplates[mRow] ;
	  if (tmpl && tmpl->c_str())
	    {
	      setFileName (tmpl->c_str());
	      setOpenType(AP_Dialog_New::open_Template);
	    }
		else
		  {
		    // fall back
		    setOpenType(AP_Dialog_New::open_New);
		  }
	}
	else
	{
		setOpenType(AP_Dialog_New::open_New);
	}
}

void AP_UnixDialog_New::event_Cancel ()
{
	setAnswer (AP_Dialog_New::a_CANCEL);
}

void AP_UnixDialog_New::event_ToggleOpenExisting ()
{
	XAP_Dialog_Id id = XAP_DIALOG_ID_FILE_OPEN;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) m_pFrame->getDialogFactory();

	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	pDialog->setCurrentPathname(0);
	pDialog->setSuggestFilename(false);

	UT_uint32 filterCount = IE_Imp::getImporterCount();
	const char ** szDescList = (const char **) UT_calloc(filterCount + 1,
													  sizeof(char *));
	const char ** szSuffixList = (const char **) UT_calloc(filterCount + 1,
														sizeof(char *));
	IEFileType * nTypeList = (IEFileType *) UT_calloc(filterCount + 1,
												   sizeof(IEFileType));
	UT_uint32 k = 0;

	while (IE_Imp::enumerateDlgLabels(k, &szDescList[k], 
									  &szSuffixList[k], &nTypeList[k]))
			k++;

	pDialog->setFileTypeList(szDescList, szSuffixList, 
							 (const UT_sint32 *) nTypeList);

	pDialog->setDefaultFileType(IE_Imp::fileTypeForSuffix(".abw"));

	pDialog->runModal(m_pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const char * szResultPathname = pDialog->getPathname();
		if (szResultPathname && *szResultPathname)
		{
			// update the entry box
			gtk_entry_set_text (GTK_ENTRY(m_entryFilename), szResultPathname);
			setFileName (szResultPathname);
		}

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(m_radioExisting), TRUE);
	}

	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);
	
	pDialogFactory->releaseDialog(pDialog);
}

/*************************************************************************/
/*************************************************************************/

static void s_choose_clicked (GtkWidget * w, AP_UnixDialog_New * dlg)
{
	dlg->event_ToggleOpenExisting();
}

static void
s_clist_clicked (GtkWidget *w, gint row, gint col, 
		 GdkEvent *evt, gpointer d)
{
	AP_UnixDialog_New * dlg = static_cast <AP_UnixDialog_New *>(d);
	dlg->event_ClistClicked (row, col);
}

/*************************************************************************/
/*************************************************************************/

extern "C" {

	// return > 0 for directory entries ending in ".awt" and ".dot"
#if defined (__APPLE__) || defined (__FreeBSD__) || defined (__OpenBSD__) || defined(_AIX)
	static int awt_only (struct dirent *d)
#else
	static int awt_only (const struct dirent *d)
#endif
	{
		const char * name = d->d_name;
		
		if ( name )
		{
			int len = strlen (name);
			
			if (len >= 4)
			{
				if(!strcmp(name+(len-4), ".awt") || !strcmp(name+(len-4), ".dot") )
					return 1;
			}
		}
		return 0;
	}
} // extern "C" block

/*************************************************************************/
/*************************************************************************/

GtkWidget * AP_UnixDialog_New::_constructWindow ()
{
	GtkWidget *mainWindow;
	GtkWidget *dialog_vbox1;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	mainWindow = abiDialogNew ( "new dialog", TRUE, pSS->getValueUTF8(AP_STRING_ID_DLG_NEW_Title).c_str());
	dialog_vbox1 = GTK_DIALOG (mainWindow)->vbox;
	gtk_widget_show (dialog_vbox1);

	abiAddStockButton ( GTK_DIALOG(mainWindow), GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;
	abiAddStockButton ( GTK_DIALOG(mainWindow), GTK_STOCK_OK, BUTTON_OK ) ;
	
	// assign pointers to widgets
	m_mainWindow   = mainWindow;
	
	// construct the window contents
	_constructWindowContents (dialog_vbox1);

	return mainWindow;
}

void AP_UnixDialog_New::_constructWindowContents (GtkWidget * container)
{
	GtkWidget *vbox1;
	GtkWidget *hbox1;

	GtkWidget *radio_new;
	GtkWidget *radio_existing;
	GtkWidget *radio_empty;
	GSList    *vbox1_group = NULL;

	GtkWidget *choicesList;

	GtkWidget *hseparator1;
	GtkWidget *hseparator2;
	GtkWidget *hseparator3;

	GtkWidget *entry_filename;
	GtkWidget *choose_btn;

	GtkWidget *scrolledwindow1;
	GtkWidget *viewport1;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	vbox1 = gtk_vbox_new (FALSE, 10);
	gtk_widget_show (vbox1);
	gtk_box_pack_start (GTK_BOX (container), vbox1, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), 6);

	hseparator1 = gtk_hseparator_new ();
	gtk_widget_show (hseparator1);
	gtk_box_pack_start (GTK_BOX (vbox1), hseparator1, TRUE, TRUE, 0);

	radio_new = gtk_radio_button_new_with_label (vbox1_group, pSS->getValueUTF8(AP_STRING_ID_DLG_NEW_Create).c_str());
	vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio_new));
	gtk_widget_show (radio_new);
	gtk_box_pack_start (GTK_BOX (vbox1), radio_new, FALSE, FALSE, 0);

	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow1);
	gtk_box_pack_start (GTK_BOX (vbox1), scrolledwindow1, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	viewport1 = gtk_viewport_new (NULL, NULL);
	gtk_widget_show (viewport1);
	gtk_container_add (GTK_CONTAINER (scrolledwindow1), viewport1);

	choicesList = gtk_clist_new ( 1 );
	gtk_clist_column_titles_passive (GTK_CLIST(choicesList));
	gtk_widget_show (choicesList);
	gtk_container_add (GTK_CONTAINER (viewport1), choicesList);

	// TODO: find better size
	gtk_widget_set_usize ( choicesList, 150, 150 ) ;

	UT_String templateList[2];
	UT_String templateDir;

	// the locally installed templates (per-user basis)
	templateDir = XAP_App::getApp()->getUserPrivateDirectory();
	templateDir += "/templates/";
	templateList[0] = templateDir;
	
	// the globally installed templates
	templateDir = XAP_App::getApp()->getAbiSuiteLibDir();
	templateDir += "/templates/";
	templateList[1] = templateDir;

	gtk_clist_freeze (GTK_CLIST (choicesList));
	gtk_clist_clear (GTK_CLIST (choicesList));

	unsigned int howManyAdded = 0 ;

	for ( unsigned int i = 0; i < (sizeof(templateList)/sizeof(templateList[0])); i++ )
	  {
	    struct dirent **namelist = NULL;
	    UT_sint32 n = 0;
	    templateDir = templateList[i];

	    n = scandir(templateDir.c_str(), &namelist, awt_only, alphasort);

	    if (n > 0)
	      {
		while(n-- > 0) 
		  {
		    UT_String myTemplate (templateDir + namelist[n]->d_name);
		    
		    UT_String * myTemplateCopy = new UT_String ( myTemplate ) ;
		    mTemplates.addItem ( myTemplateCopy ) ;

			  myTemplate = myTemplate.substr ( 0, myTemplate.size() - 4 ) ;
			  gchar * txt[2];
			  txt[0] = (gchar*) UT_basename ( myTemplate.c_str() );
			  txt[1] = NULL;

			  gtk_clist_append ( GTK_CLIST(choicesList), txt ) ;
			  free (namelist[n]);

			  howManyAdded++ ;
		  }
	      }
	  }

	gtk_clist_thaw (GTK_CLIST (choicesList));
	gtk_clist_select_row (GTK_CLIST (choicesList), 0, 0);

	// connect the select_row signal to the clist
	g_signal_connect (G_OBJECT (choicesList), "select-row",
			    G_CALLBACK (s_clist_clicked), (gpointer)this);

	// TODO? Put clist inside of scrolled window

	hseparator2 = gtk_hseparator_new ();
	gtk_widget_show (hseparator2);
	gtk_box_pack_start (GTK_BOX (vbox1), hseparator2, TRUE, TRUE, 0);

	radio_existing = gtk_radio_button_new_with_label (vbox1_group, pSS->getValueUTF8(AP_STRING_ID_DLG_NEW_Open).c_str());
	vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio_existing));
	gtk_widget_show (radio_existing);
	gtk_box_pack_start (GTK_BOX (vbox1), radio_existing, FALSE, FALSE, 0);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

	entry_filename = gtk_entry_new ();
	gtk_widget_show (entry_filename);
	gtk_box_pack_start (GTK_BOX (hbox1), entry_filename, TRUE, TRUE, 0);
	gtk_entry_set_editable (GTK_ENTRY (entry_filename), FALSE);
	gtk_entry_set_text (GTK_ENTRY (entry_filename), pSS->getValueUTF8(AP_STRING_ID_DLG_NEW_NoFile).c_str());

	choose_btn = gtk_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_NEW_Choose).c_str());
	gtk_widget_show (choose_btn);
	gtk_box_pack_start (GTK_BOX (hbox1), choose_btn, FALSE, FALSE, 0);

	hseparator3 = gtk_hseparator_new ();
	gtk_widget_show (hseparator3);
	gtk_box_pack_start (GTK_BOX (vbox1), hseparator3, TRUE, TRUE, 0);

	radio_empty = gtk_radio_button_new_with_label (vbox1_group, 
						       pSS->getValueUTF8(AP_STRING_ID_DLG_NEW_StartEmpty).c_str());
	vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio_empty));
	gtk_widget_show (radio_empty);
	gtk_box_pack_start (GTK_BOX (vbox1), radio_empty, FALSE, FALSE, 0);

	// make this one the default
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_empty), TRUE);

	// connect signals
	g_signal_connect (G_OBJECT(choose_btn), 
						"clicked",
						G_CALLBACK(s_choose_clicked), 
						(gpointer)this);

	// set the private pointers
	m_radioNew      = radio_new;
	m_radioExisting = radio_existing;
	m_radioEmpty    = radio_empty;
	m_entryFilename = entry_filename;
	m_choicesList   = choicesList;
}
