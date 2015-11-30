/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_debugmsg.h"
#include "ut_std_string.h"
#include "xap_Gtk2Compat.h"
#include "xap_UnixDialogHelper.h"
#include "xap_UnixDlg_MessageBox.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"

#include "xap_Strings.h"

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
    XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(pFrame->getFrameImpl());
    UT_return_if_fail(pUnixFrameImpl);

    XAP_UnixApp * pApp = static_cast<XAP_UnixApp *>(XAP_App::getApp());
    UT_return_if_fail(pApp);

    GtkWidget * message = 0;	// initialize to prevent compiler warning
    GtkWindow * toplevel;

    toplevel = GTK_WINDOW(pUnixFrameImpl->getTopLevelWindow());

    int dflFlags = GTK_DIALOG_MODAL;
    int dflResponse = GTK_RESPONSE_OK;

    switch (m_buttons)
    {
    case b_O:
	// just put up an information box
	message = gtk_message_dialog_new ( toplevel, GTK_DIALOG_MODAL,
					   GTK_MESSAGE_INFO,
					   GTK_BUTTONS_OK,
					   "%s",
					   m_szMessage ) ;

	break;

    case b_YN:
	// YES - NO - most certainly a question
	message = gtk_message_dialog_new ( toplevel, GTK_DIALOG_MODAL,
					   GTK_MESSAGE_QUESTION,
					   GTK_BUTTONS_YES_NO,
					   "%s",
					   m_szMessage ) ;
	if(m_defaultAnswer == XAP_Dialog_MessageBox::a_YES)
	{
	    gtk_dialog_set_default_response (GTK_DIALOG(message),
					     GTK_RESPONSE_YES);
	}
	else
	{
	    gtk_dialog_set_default_response (GTK_DIALOG(message),
					     GTK_RESPONSE_NO);
	}
	break;

    case b_YNC:
    {
	// YES - NO - CANCEL
	// this is only used for saving files.
#ifndef EMBEDDED_TARGET
	std::string no, cancel, save;
	std::string labelText;
	const XAP_StringSet * pSS = pApp->getStringSet ();

	message = gtk_dialog_new_with_buttons("",
					      toplevel,
					      static_cast<GtkDialogFlags>(dflFlags),
					      NULL, NULL);
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Exit_CloseWithoutSaving, no);
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel, cancel);
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Save, save);
	gtk_dialog_add_buttons(GTK_DIALOG(message),
			       convertMnemonics(no).c_str(),
			       GTK_RESPONSE_NO,
			       convertMnemonics(cancel).c_str(),
			       GTK_RESPONSE_CANCEL,
			       convertMnemonics(save).c_str(),
			       GTK_RESPONSE_YES,
			       NULL);

	dflResponse = GTK_RESPONSE_YES;

	GtkWidget * label = gtk_label_new(NULL);
	const char * separator;
	separator = m_szSecondaryMessage ? "\n\n" : "";

	gchar     * msg = g_markup_escape_text (m_szMessage, -1);
	labelText = UT_std_string_sprintf(
	    "<span weight=\"bold\" size=\"larger\">%s</span>%s%s",
	    msg, separator, m_szSecondaryMessage);
	g_free (msg); msg = NULL;

	gtk_label_set_markup(GTK_LABEL(label), labelText.c_str());

	GtkWidget * hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);

	gtk_box_pack_start (GTK_BOX (hbox),
			    gtk_image_new_from_icon_name("dialog-warning",
							 GTK_ICON_SIZE_DIALOG),
			    FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (hbox), label,
			    TRUE, TRUE, 0);

	GtkBox *content_area = GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG(message)));
	gtk_box_pack_start (content_area, hbox, FALSE, FALSE, 0);

	gtk_box_set_spacing(content_area, 12);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 6);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);

	gtk_widget_show_all (hbox);

#else
	message = gtk_message_dialog_new (toplevel,
					  static_cast<GtkDialogFlags>(dflFlags),
					  GTK_MESSAGE_QUESTION,
					  GTK_BUTTONS_NONE,
					  "%s",
					  m_szMessage);

	gtk_dialog_add_buttons(GTK_DIALOG(message),
			       GTK_STOCK_NO,
			       GTK_RESPONSE_NO,
			       GTK_STOCK_CANCEL,
			       GTK_RESPONSE_CANCEL,
			       GTK_STOCK_YES,
			       GTK_RESPONSE_YES,
			       NULL);
#endif
	gtk_dialog_set_default_response (GTK_DIALOG(message),
					 GTK_RESPONSE_CANCEL);

	break;
    }
    default:
	UT_ASSERT_NOT_REACHED();
    }

    // set the title to '', as per GNOME HIG, Section 3, Alerts
    gtk_window_set_title (GTK_WINDOW(message), "");

    UT_ASSERT(message);

    switch ( abiRunModalDialog ( GTK_DIALOG(message), pFrame,
				 this, dflResponse, true, ATK_ROLE_ALERT ) )
    {
    case GTK_RESPONSE_OK:
	m_answer = XAP_Dialog_MessageBox::a_OK;
	break;
    case GTK_RESPONSE_YES:
	m_answer = XAP_Dialog_MessageBox::a_YES;
	break;
    case GTK_RESPONSE_NO:
	m_answer = XAP_Dialog_MessageBox::a_NO;
	break;
    case GTK_RESPONSE_CANCEL:
    default:
	m_answer = XAP_Dialog_MessageBox::a_CANCEL; break;
    }
}
