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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"
#include "xap_UnixDlg_MessageBox.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

/*****************************************************************/
XAP_Dialog * XAP_UnixDialog_MessageBox::static_constructor(XAP_DialogFactory * pFactory,
							   XAP_Dialog_Id id)
{
	XAP_UnixDialog_MessageBox * p = new XAP_UnixDialog_MessageBox(pFactory,id);
	return p;
}

XAP_UnixDialog_MessageBox::XAP_UnixDialog_MessageBox(XAP_DialogFactory * pDlgFactory,
						     XAP_Dialog_Id id)
	: XAP_Dialog_MessageBox(pDlgFactory,id)
{
}

XAP_UnixDialog_MessageBox::~XAP_UnixDialog_MessageBox(void)
{
}

/*****************************************************************/

void XAP_UnixDialog_MessageBox::runModal(XAP_Frame * pFrame)
{
	XAP_UnixFrameHelper * pUnixFrameHelper = static_cast<XAP_UnixFrameHelper *>(pFrame->getFrameHelper());
	UT_return_if_fail(pUnixFrameHelper);

	XAP_UnixApp * pApp = (XAP_UnixApp *)pFrame->getApp();
	UT_return_if_fail(pApp);

	GtkWidget * message ;
	GtkWindow * toplevel = GTK_WINDOW(pUnixFrameHelper->getTopLevelWindow());
	GtkWidget * label;
	GtkWidget * hbox;

	UT_String labelText, separator;
	
	switch (m_buttons)
	{
		case b_O:
	        // just put up an information box
			message = gtk_message_dialog_new ( toplevel, GTK_DIALOG_MODAL,
							   GTK_MESSAGE_INFO,
							   GTK_BUTTONS_OK,
							   m_szMessage ) ;

			break;

		case b_YN:
			// YES - NO - most certainly a question
			message = gtk_message_dialog_new ( toplevel, GTK_DIALOG_MODAL,
							   GTK_MESSAGE_QUESTION,
							   GTK_BUTTONS_YES_NO,
							   m_szMessage ) ;
			break;

		case b_YNC:
			// YES - NO - CANCEL
			// this is only used for saving files.
			// FIXME: bug 4146
			message = gtk_dialog_new_with_buttons("",
							      toplevel, 
							      GTK_DIALOG_MODAL,
							      "Close _without Saving",
							      GTK_RESPONSE_NO,
							      GTK_STOCK_CANCEL, 
							      GTK_RESPONSE_CANCEL, 
							      GTK_STOCK_SAVE, 
							      GTK_RESPONSE_YES,
							      NULL);
			
			label = gtk_label_new(NULL);
			if (m_szSecondaryMessage == NULL)
				separator =UT_String("");
			else
				separator =UT_String("\n\n");
			
			labelText = UT_String_sprintf(labelText, "<span weight=\"bold\" size=\"larger\">%s</span>%s%s", 
					m_szMessage, separator.c_str(), m_szSecondaryMessage);
			
			UT_DEBUGMSG(("SAM: text is %s\n", labelText.c_str()));
			gtk_label_set_markup(GTK_LABEL(label), labelText.c_str());

			hbox = gtk_hbox_new(FALSE, 12);
  
			gtk_box_pack_start (GTK_BOX (hbox), 
					    gtk_image_new_from_stock(GTK_STOCK_DIALOG_WARNING, 
								     GTK_ICON_SIZE_DIALOG),
					    FALSE, FALSE, 0);
			
			gtk_box_pack_start (GTK_BOX (hbox), label,
					    TRUE, TRUE, 0);

			gtk_box_pack_start (GTK_BOX (GTK_DIALOG (message)->vbox),
					    hbox,
					    FALSE, FALSE, 0);

			gtk_box_set_spacing(GTK_BOX(GTK_DIALOG (message)->vbox), 12);
			gtk_container_set_border_width(GTK_CONTAINER(hbox), 6);
			gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
			
			gtk_widget_show_all (hbox);

  			gtk_dialog_set_default_response (GTK_DIALOG(message),
							 GTK_RESPONSE_YES);
			gtk_dialog_set_has_separator(GTK_DIALOG(message), FALSE);
			UT_DEBUGMSG(("SAM: got here\n"));
			break;
			
		default:
			UT_ASSERT_NOT_REACHED();
	}

	// set the title to '', as per GNOME HIG, Section 3, Alerts
	gtk_window_set_title (GTK_WINDOW(message), "");

	switch ( abiRunModalDialog ( GTK_DIALOG(message), pFrame,
				     this, GTK_RESPONSE_OK, true ) )
	{
		case GTK_RESPONSE_OK:
			m_answer = XAP_Dialog_MessageBox::a_OK; break;
		case GTK_RESPONSE_CANCEL:
			m_answer = XAP_Dialog_MessageBox::a_CANCEL; break;
		case GTK_RESPONSE_YES:
			m_answer = XAP_Dialog_MessageBox::a_YES; break;
		case GTK_RESPONSE_NO:
			m_answer = XAP_Dialog_MessageBox::a_NO; break;
		default:
			UT_ASSERT_NOT_REACHED() ;
	}
}

