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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_WindowMore.h"
#include "xap_UnixDlg_WindowMore.h"

#ifdef HAVE_GNOME
#include <gnome.h>
#endif

/*****************************************************************/

XAP_Dialog * XAP_UnixDialog_WindowMore::static_constructor(XAP_DialogFactory * pFactory,
							   XAP_Dialog_Id id)
{
  XAP_UnixDialog_WindowMore * p = new XAP_UnixDialog_WindowMore(pFactory,id);
  return p;
}

XAP_UnixDialog_WindowMore::XAP_UnixDialog_WindowMore(XAP_DialogFactory * pDlgFactory,
						     XAP_Dialog_Id id)
  : XAP_Dialog_WindowMore(pDlgFactory,id)
{
}

XAP_UnixDialog_WindowMore::~XAP_UnixDialog_WindowMore(void)
{
}

/*****************************************************************/

// These are all static callbacks, bound to GTK or GDK events.

void XAP_UnixDialog_WindowMore::s_clist_event(GtkWidget * widget,
					      GdkEventButton * event,
					      XAP_UnixDialog_WindowMore * dlg)
{
  UT_return_if_fail(widget && event && dlg);
  
  // Only respond to double clicks
  if (event->type == GDK_2BUTTON_PRESS)
    {
      dlg->event_DoubleClick();
    }
}
/*****************************************************************/

void XAP_UnixDialog_WindowMore::runModal(XAP_Frame * pFrame)
{
  // Initialize member so we know where we are now
  m_ndxSelFrame = m_pApp->findFrame(pFrame);
  UT_ASSERT_HARMLESS(m_ndxSelFrame >= 0);

  // Build the window's widgets and arrange them
  GtkWidget * mainWindow = _constructWindow();

  // Populate the window's data items
  _populateWindowData();

  switch ( abiRunModalDialog ( GTK_DIALOG(mainWindow), pFrame, this, false ) )
    {
    case BUTTON_OK:
      event_OK () ; break ;
    default:
      event_Cancel (); break ;
    }

  abiDestroyWidget ( mainWindow ) ;
}

void XAP_UnixDialog_WindowMore::event_OK(void)
{
  // Query the list for its selection.
  gint row = _GetFromList();
  
  if (row >= 0)
    m_ndxSelFrame = (UT_uint32) row;
  
  m_answer = XAP_Dialog_WindowMore::a_OK;
}

void XAP_UnixDialog_WindowMore::event_Cancel(void)
{
  m_answer = XAP_Dialog_WindowMore::a_CANCEL;
}

void XAP_UnixDialog_WindowMore::event_DoubleClick(void)
{
  // Query the list for its selection.	
  gint row = _GetFromList();
  
  // If it found something, return with it
  if (row >= 0)
    {
      m_ndxSelFrame = (UT_uint32) row;
      gtk_dialog_response ( GTK_DIALOG(m_windowMain), BUTTON_OK ) ;
    }
}

/*****************************************************************/

gint XAP_UnixDialog_WindowMore::_GetFromList(void)
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
	  UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	  return -1;
	}
    }
  
  // No selected rows
  return -1;
}

GtkWidget * XAP_UnixDialog_WindowMore::_constructWindow(void)
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

  // create dialog, buttons

  windowMain = abiDialogNew (TRUE, pSS->getValue(XAP_STRING_ID_DLG_MW_MoreWindows));  
  vboxMain = GTK_DIALOG(windowMain)->vbox;
  
  gtk_dialog_add_button(GTK_DIALOG(windowMain), GTK_STOCK_OK, BUTTON_OK);
  gtk_dialog_add_button(GTK_DIALOG(windowMain), GTK_STOCK_CANCEL, BUTTON_CANCEL);

  // create contents

  labelActivate = gtk_label_new (pSS->getValue(XAP_STRING_ID_DLG_MW_Activate));
  gtk_widget_show (labelActivate);
  gtk_box_pack_start (GTK_BOX (vboxMain), labelActivate, FALSE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (labelActivate), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (labelActivate), 0, 0);
  gtk_misc_set_padding (GTK_MISC (labelActivate), 10, 5);
  
  scrollWindows = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_show(scrollWindows);
  gtk_widget_set_usize(scrollWindows, 350, 210);
  gtk_container_set_border_width(GTK_CONTAINER(scrollWindows), 10);
  gtk_box_pack_start(GTK_BOX(vboxMain), scrollWindows, TRUE, TRUE, 0);
  
  clistWindows = gtk_clist_new (1);
  gtk_widget_show (clistWindows);
  gtk_container_add (GTK_CONTAINER(scrollWindows), clistWindows);
  gtk_clist_set_column_width (GTK_CLIST (clistWindows), 0, 80);
  gtk_clist_column_titles_hide (GTK_CLIST (clistWindows));
  
  g_signal_connect(G_OBJECT(clistWindows),
		   "button_press_event",
		   G_CALLBACK(s_clist_event),
		   (gpointer) this);
  
  // Update member variables with the important widgets that
  // might need to be queried or altered later.

  m_windowMain = windowMain;
  m_clistWindows = clistWindows;

  return windowMain;
}

void XAP_UnixDialog_WindowMore::_populateWindowData(void)
{
  // We just do one thing here, which is fill the list with
  // all the windows.
  
  for (UT_uint32 i = 0; i < m_pApp->getFrameCount(); i++)
    {
      XAP_Frame * f = m_pApp->getFrame(i);
      UT_return_if_fail(f);
      const char * s = f->getTitle(128);	// TODO: chop this down more? 
      
      gint row = gtk_clist_append(GTK_CLIST(m_clistWindows), (gchar **) &s);
      gtk_clist_set_row_data(GTK_CLIST(m_clistWindows), row, GINT_TO_POINTER(i));
    } 
  
  // Select the one we're in
  gtk_clist_select_row(GTK_CLIST(m_clistWindows), m_ndxSelFrame, 0);
}

