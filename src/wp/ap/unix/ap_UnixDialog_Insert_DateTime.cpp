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

#include <stdlib.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Insert_DateTime.h"
#include "ap_UnixDialog_Insert_DateTime.h"

/*****************************************************************/

#define	LIST_ITEM_INDEX_KEY "index"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Insert_DateTime::static_constructor(XAP_DialogFactory * pFactory,
															   XAP_Dialog_Id id)
{
	AP_UnixDialog_Insert_DateTime * p = new AP_UnixDialog_Insert_DateTime(pFactory,id);
	return p;
}

AP_UnixDialog_Insert_DateTime::AP_UnixDialog_Insert_DateTime(XAP_DialogFactory * pDlgFactory,
															 XAP_Dialog_Id id)
	: AP_Dialog_Insert_DateTime(pDlgFactory,id)
{
	m_windowMain = NULL;
	m_listFormats = NULL;
}

AP_UnixDialog_Insert_DateTime::~AP_UnixDialog_Insert_DateTime(void)
{
}

/*****************************************************************/
/*****************************************************************/

void AP_UnixDialog_Insert_DateTime::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	// Populate the window's data items
	_populateWindowData();

	switch(abiRunModalDialog(GTK_DIALOG(mainWindow), pFrame, this,
							 BUTTON_CANCEL, false ))
	{
		case BUTTON_OK:
			event_OK(); break;
		default:
			event_Cancel(); break ;
	}

	abiDestroyWidget ( mainWindow ) ;
}

void AP_UnixDialog_Insert_DateTime::event_OK(void)
{
	UT_ASSERT(m_windowMain && m_listFormats);
	
	// find item selected in list box, save it to m_iFormatIndex

	GList * listitem = GTK_LIST(m_listFormats)->selection;

	// if there is no selection, or the selection's data (GtkListItem widget)
	// is empty, return cancel.  GTK can make this happen.
	if (!(listitem) || !(listitem)->data)
	{
		m_answer = AP_Dialog_Insert_DateTime::a_CANCEL;
		return;
	}

	// since we only do single mode selection, there is only one
	// item in the GList we just got back

	gint indexdata = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(listitem->data), LIST_ITEM_INDEX_KEY));

	m_iFormatIndex = indexdata;
	
	m_answer = AP_Dialog_Insert_DateTime::a_OK;
}

void AP_UnixDialog_Insert_DateTime::event_Cancel(void)
{
	m_answer = AP_Dialog_Insert_DateTime::a_CANCEL;
}

/*****************************************************************/
GtkWidget * AP_UnixDialog_Insert_DateTime::_constructWindow(void)
{
  GtkWidget *vbox;
  GtkWidget *contents;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  m_windowMain = abiDialogNew ( true, pSS->getValue(AP_STRING_ID_DLG_DateTime_DateTimeTitle));
  
  vbox = GTK_DIALOG(m_windowMain)->vbox;
  contents = _constructWindowContents();
  gtk_box_pack_start (GTK_BOX (vbox), contents, TRUE, TRUE, 0);

  abiAddStockButton ( GTK_DIALOG(m_windowMain), GTK_STOCK_OK, BUTTON_OK ) ;
  abiAddStockButton ( GTK_DIALOG(m_windowMain), GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;
  return m_windowMain;
}

GtkWidget * AP_UnixDialog_Insert_DateTime::_constructWindowContents(void)
{
        GtkWidget *vboxFormats;
	GtkWidget *labelFormats;
	GtkWidget *scrolledwindowFormats;
	GtkWidget *viewportFormats;
	GtkWidget *listFormats;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions

	vboxFormats = gtk_vbox_new (FALSE, 0);
	//gtk_widget_ref (vboxFormats);
	//gtk_widget_show (vboxFormats);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_DateTime_AvailableFormats));
	labelFormats = gtk_label_new (unixstr);
	FREEP(unixstr);
	//gtk_widget_ref (labelFormats);
	//gtk_widget_show (labelFormats);
	gtk_box_pack_start (GTK_BOX (vboxFormats), labelFormats, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (labelFormats), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelFormats), 0, 0.5);

	scrolledwindowFormats = gtk_scrolled_window_new (NULL, NULL);
	//gtk_widget_ref (scrolledwindowFormats);
	//gtk_widget_show (scrolledwindowFormats);
	gtk_box_pack_start (GTK_BOX (vboxFormats), scrolledwindowFormats, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindowFormats), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(scrolledwindowFormats,-1,240);

	viewportFormats = gtk_viewport_new (NULL, NULL);
	//gtk_widget_ref (viewportFormats);
	//gtk_widget_show (viewportFormats);
	gtk_container_add (GTK_CONTAINER (scrolledwindowFormats), viewportFormats);

	listFormats = gtk_list_new ();
	//gtk_widget_ref (listFormats);
	gtk_list_set_selection_mode (GTK_LIST(listFormats), GTK_SELECTION_SINGLE);
	//gtk_widget_show (listFormats);
	gtk_container_add (GTK_CONTAINER (viewportFormats), listFormats);

	// *must set this here*
	m_listFormats = listFormats;

	return vboxFormats;
}

void AP_UnixDialog_Insert_DateTime::_populateWindowData(void)
{
	UT_ASSERT(m_windowMain && m_listFormats);

	// NOTE : this code is similar to the Windows dialog code to do
	// NOTE : the same thing.  if you are implementing this dialog
	// NOTE : for a new front end, this is the formatting logic 
	// NOTE : you'll want to use to populate your list
	
	int i;

	// this constant comes from ap_Dialog_Insert_DateTime.h
    char szCurrentDateTime[CURRENT_DATE_TIME_SIZE];
	
    time_t tim = time(NULL);
	
    struct tm *pTime = localtime(&tim);

	GList * list = NULL;
	GtkWidget * listitem;
	
	// build GList of gtk_list_items
    for (i = 0; InsertDateTimeFmts[i] != NULL; i++)
	{
        strftime(szCurrentDateTime, CURRENT_DATE_TIME_SIZE, InsertDateTimeFmts[i], pTime);

		// allocate a list item with that string
		listitem = gtk_list_item_new_with_label(szCurrentDateTime);
		UT_ASSERT(listitem);

		gtk_widget_show(listitem);
		
		// store index in data pointer
		g_object_set_data(G_OBJECT(listitem), LIST_ITEM_INDEX_KEY, GINT_TO_POINTER(i));

		// add to list
	    list = g_list_append(list, (void *) listitem);
	}

	// TODO : Does the gtk_list free this list for me?  I think it does.
	
	// add GList items to list box
	gtk_list_append_items(GTK_LIST(m_listFormats), list);

	// now select first item in box
	if (i > 0)
		gtk_list_select_item(GTK_LIST(m_listFormats), 0);

}

