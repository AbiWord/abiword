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
#include <gnome.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "gr_UnixGraphics.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Dialog_Id.h"

#include "ap_Strings.h"

#include "ap_Preview_Paragraph.h"
#include "ap_UnixGnomeDialog_Paragraph.h"

/*****************************************************************/

XAP_Dialog * AP_UnixGnomeDialog_Paragraph::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	AP_UnixGnomeDialog_Paragraph * p = new AP_UnixGnomeDialog_Paragraph(pFactory,id);
	return p;
}

AP_UnixGnomeDialog_Paragraph::AP_UnixGnomeDialog_Paragraph(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_UnixDialog_Paragraph(pDlgFactory,id)
{
}

AP_UnixGnomeDialog_Paragraph::~AP_UnixGnomeDialog_Paragraph(void)
{
}

/*****************************************************************/
/* These are static callbacks for dialog widgets                 */
/*****************************************************************/

static void s_ok_clicked(GtkWidget * widget, AP_UnixGnomeDialog_Paragraph * dlg)
{ UT_ASSERT(widget && dlg); dlg->event_OK(); }

static void s_cancel_clicked(GtkWidget * widget, AP_UnixGnomeDialog_Paragraph * dlg)
{ UT_ASSERT(widget && dlg); dlg->event_Cancel(); }

static void s_tabs_clicked(GtkWidget * widget, AP_UnixGnomeDialog_Paragraph * dlg)
{ UT_ASSERT(widget && dlg);	dlg->event_Tabs(); }

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 AP_UnixGnomeDialog_Paragraph * dlg)
{ UT_ASSERT(dlg); dlg->event_WindowDelete(); }

/*****************************************************************/

GtkWidget * AP_UnixGnomeDialog_Paragraph::_constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	GtkWidget * windowParagraph;
	GtkWidget * windowContents;

	GtkWidget * buttonTabs;
	GtkWidget * buttonOK;
	GtkWidget * buttonCancel;

	XML_Char * unixstr = NULL;
	

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_ParaTitle) );
	windowParagraph = gnome_dialog_new (unixstr, NULL);

	gtk_object_set_data (GTK_OBJECT (windowParagraph), "windowParagraph", windowParagraph);
	gtk_widget_set_usize (windowParagraph, 441, -2);
	FREEP(unixstr);
	gtk_window_set_policy (GTK_WINDOW (windowParagraph), FALSE, FALSE, FALSE);


	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_ButtonTabs));
	gnome_dialog_append_button(GNOME_DIALOG(windowParagraph), unixstr);
	FREEP(unixstr);

	gnome_dialog_append_button(GNOME_DIALOG(windowParagraph), GNOME_STOCK_BUTTON_OK);
	gnome_dialog_append_button(GNOME_DIALOG(windowParagraph), GNOME_STOCK_BUTTON_CANCEL);


	windowContents = _constructWindowContents(windowParagraph);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (windowParagraph)->vbox), windowContents, TRUE, TRUE, 0);

	buttonTabs = GTK_WIDGET (g_list_nth_data (GNOME_DIALOG (windowParagraph)->buttons, 0) );
	gtk_widget_ref (buttonTabs);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "buttonTabs", buttonTabs,
							  (GtkDestroyNotify) gtk_widget_unref);

	buttonOK = GTK_WIDGET (g_list_nth_data (GNOME_DIALOG (windowParagraph)->buttons, 1) );
	gtk_widget_ref (buttonOK);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "buttonOK", buttonOK,
							  (GtkDestroyNotify) gtk_widget_unref);

	buttonCancel = GTK_WIDGET (g_list_nth_data (GNOME_DIALOG (windowParagraph)->buttons, 2) );
	gtk_widget_ref (buttonCancel);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "buttonCancel", buttonCancel,
							  (GtkDestroyNotify) gtk_widget_unref);

	m_windowMain = windowParagraph;

	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;
	m_buttonTabs = buttonTabs;

	return windowParagraph;
}
