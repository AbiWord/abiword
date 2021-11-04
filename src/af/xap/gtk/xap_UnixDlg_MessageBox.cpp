/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2021 Hubert Figuière
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

#include "ut_compiler.h"

ABI_W_NO_CONST_QUAL
#include <gtk/gtk.h>
ABI_W_POP
#include <gdk/gdkkeysyms.h>
#include <glib.h>

#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_debugmsg.h"
#include "ut_std_string.h"
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

    GtkWidget * message = nullptr;
    GtkWindow * toplevel;

    toplevel = GTK_WINDOW(pUnixFrameImpl->getTopLevelWindow());

    GtkDialogFlags dflFlags = GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);
    int dflResponse = GTK_RESPONSE_OK;

    switch (m_buttons)
    {
    case b_O:
	// just put up an information box
        message = gtk_message_dialog_new(toplevel, dflFlags,
                                         GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                         "%s", m_message.c_str());
	break;

    case b_YN:
	// YES - NO - most certainly a question
        message = gtk_message_dialog_new(toplevel, dflFlags,
                                         GTK_MESSAGE_QUESTION,
                                         GTK_BUTTONS_YES_NO,
                                         "%s", m_message.c_str());
	if (m_defaultAnswer == XAP_Dialog_MessageBox::a_YES) {
	    gtk_dialog_set_default_response(GTK_DIALOG(message), GTK_RESPONSE_YES);
	} else {
            gtk_dialog_set_default_response(GTK_DIALOG(message), GTK_RESPONSE_NO);
	}
	break;

    case b_YNC:
    {
	// YES - NO - CANCEL
	// this is only used for saving files.
        message = gtk_message_dialog_new(toplevel, dflFlags,
                                         GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
                                         "%s", m_message.c_str());

        std::string no, cancel, save;
        const XAP_StringSet * pSS = pApp->getStringSet();
        pSS->getValueUTF8(XAP_STRING_ID_DLG_Exit_CloseWithoutSaving, no);
        pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel, cancel);
        pSS->getValueUTF8(XAP_STRING_ID_DLG_Save, save);
        GtkWidget* close_button = gtk_dialog_add_button(GTK_DIALOG(message),
                                                        convertMnemonics(no).c_str(),
                                                        GTK_RESPONSE_NO);
        gtk_style_context_add_class(gtk_widget_get_style_context(close_button),
                                     "destructive-action");

        gtk_dialog_add_button(GTK_DIALOG(message), convertMnemonics(cancel).c_str(),
                              GTK_RESPONSE_CANCEL);
        gtk_dialog_add_button(GTK_DIALOG(message), convertMnemonics(save).c_str(),
                              GTK_RESPONSE_YES);

        gtk_dialog_set_default_response (GTK_DIALOG(message),
					 GTK_RESPONSE_YES);

	break;
    }
    default:
	UT_ASSERT_NOT_REACHED();
    }

    if (!m_secondaryMessage.empty()) {
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message),
                                                 "%s",
                                                 m_secondaryMessage.c_str());
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
