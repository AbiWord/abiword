/* AbiSource Application Framework
 * Copyright (C) 1998-2002 AbiSource, Inc.
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
#include <stdio.h>
#include <string.h>
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_UnixDlg_Encoding.h"


XAP_Dialog * XAP_UnixDialog_Encoding::static_constructor(XAP_DialogFactory * pFactory,
							 XAP_Dialog_Id id)
{
  XAP_UnixDialog_Encoding * p = new XAP_UnixDialog_Encoding(pFactory,id);
  return p;
}

XAP_UnixDialog_Encoding::XAP_UnixDialog_Encoding(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_Encoding(pDlgFactory,id)
{ 
}

XAP_UnixDialog_Encoding::~XAP_UnixDialog_Encoding(void)
{
}

/*****************************************************************/

// These are all static callbacks, bound to GTK or GDK events.

void XAP_UnixDialog_Encoding::s_clist_event(GtkWidget * widget,
					    GdkEventButton * event,
					    XAP_UnixDialog_Encoding * dlg)
{
  UT_return_if_fail(widget && event && dlg);
  
  // Only respond to double clicks
  if (event->type == GDK_2BUTTON_PRESS)
    {
      dlg->event_DoubleClick();
    }
}

/*****************************************************************/

void XAP_UnixDialog_Encoding::runModal(XAP_Frame * pFrame)
{
  // Build the window's widgets and arrange them
  GtkWidget * mainWindow = _constructWindow();
  
  // Populate the window's data items
  _populateWindowData();
  
  switch ( abiRunModalDialog ( GTK_DIALOG(mainWindow), pFrame, this ) )
    {
    case BUTTON_OK:
      event_Ok (); break;
    default:
      event_Cancel (); break;
    }
  abiDestroyWidget ( mainWindow ) ;
}

void XAP_UnixDialog_Encoding::event_Ok(void)
{
  // Query the list for its selection.
  gint row = _getFromList();
  
  if (row >= 0)
    _setSelectionIndex((UT_uint32) row);
  
  _setEncoding (_getAllEncodings()[row]);
  _setAnswer (XAP_Dialog_Encoding::a_OK);
}

void XAP_UnixDialog_Encoding::event_Cancel(void)
{
  _setAnswer (XAP_Dialog_Encoding::a_CANCEL);
}

void XAP_UnixDialog_Encoding::event_DoubleClick(void)
{
  // Query the list for its selection.	
  gint row = _getFromList();
  
  // If it found something, return with it
  if (row >= 0)
    {
      _setSelectionIndex ((UT_uint32) row);
      gtk_dialog_response ( GTK_DIALOG(m_windowMain), BUTTON_OK ) ;
    }
}

/*****************************************************************/

gint XAP_UnixDialog_Encoding::_getFromList(void)
{
  // Grab the selected index and store it in the member data
  GList * selectedRow = GTK_CLIST(m_clistWindows)->selection;
  
  if (selectedRow)
    {
      gint rowNumber = GPOINTER_TO_INT(selectedRow->data);
      if (rowNumber >= 0)
	{
	  // Store the value
	  return rowNumber;
	}
      else
	{
	  // We have a selection but no rows in it...
	  // funny.
	  UT_ASSERT_NOT_REACHED();
	  return -1;
	}
    }
  
  // No selected rows
  return -1;
}

GtkWidget * XAP_UnixDialog_Encoding::_constructWindow(void)
{
  // This is the top level GTK widget, the window.
  // It's created with a "dialog" style.
  GtkWidget *windowMain;
  
  // This is the top level organization widget, which packs
  // things vertically
  GtkWidget *vboxMain;
  
  // The top item in the vbox is a simple label
  GtkWidget *labelActivate;
  
  // The second item in the vbox is a scrollable area
  GtkWidget *scrollWindows;
  
  // The child of the scrollable area is our list of windows
  GtkWidget *clistWindows;
  
  const XAP_StringSet * pSS = m_pApp->getStringSet();

  windowMain = abiDialogNew ( TRUE, pSS->getValue(XAP_STRING_ID_DLG_UENC_EncTitle) ) ;

  vboxMain = GTK_DIALOG(windowMain)->vbox ;


  gtk_dialog_add_button(GTK_DIALOG(windowMain), GTK_STOCK_OK, BUTTON_OK);
  gtk_dialog_add_button(GTK_DIALOG(windowMain), GTK_STOCK_CANCEL, BUTTON_CANCEL);
  
  labelActivate = gtk_label_new (pSS->getValue(XAP_STRING_ID_DLG_UENC_EncLabel));
  gtk_widget_show (labelActivate);
  gtk_box_pack_start (GTK_BOX (vboxMain), labelActivate, FALSE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (labelActivate), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (labelActivate), 0, 0);
  gtk_misc_set_padding (GTK_MISC (labelActivate), 10, 5);
  
  scrollWindows = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollWindows),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_show(scrollWindows);
  gtk_widget_set_usize(scrollWindows, 350, 210);
  gtk_container_set_border_width(GTK_CONTAINER(scrollWindows), 10);
  gtk_box_pack_start(GTK_BOX(vboxMain), scrollWindows, TRUE, TRUE, 0);

  clistWindows = gtk_clist_new (1);
  gtk_widget_show (clistWindows);
  gtk_container_add (GTK_CONTAINER(scrollWindows), clistWindows);
  gtk_clist_set_column_width (GTK_CLIST (clistWindows), 0, 80);
  gtk_clist_column_titles_hide (GTK_CLIST (clistWindows));

  /*
    After we construct our widgets, we attach callbacks to static
    callback functions so we can respond to their events.  In this
    dialog, we will want to respond to both buttons (OK and Cancel),
    double-clicks on the clist, which will be treated like a
    click on the OK button.
  */
  
  g_signal_connect(G_OBJECT(clistWindows),
		   "button_press_event",
		   G_CALLBACK(s_clist_event),
		   (gpointer) this);
  
  // Update member variables with the important widgets that
  // might need to be queried or altered later.
  
  m_windowMain   = windowMain;
  m_clistWindows = clistWindows;
  
  return windowMain;
}

void XAP_UnixDialog_Encoding::_populateWindowData(void)
{
  // We just do one thing here, which is fill the list with
  // all the windows.
  
  for (UT_uint32 i = 0; i < _getEncodingsCount(); i++)
    {
      const XML_Char* s = _getAllEncodings()[i];
      
      gint row = gtk_clist_append(GTK_CLIST(m_clistWindows), (gchar **) &s);
      gtk_clist_set_row_data(GTK_CLIST(m_clistWindows), row, GINT_TO_POINTER(i));
    } 
  
  // Select the one we're in
  gtk_clist_select_row(GTK_CLIST(m_clistWindows), _getSelectionIndex(), 0);
}
