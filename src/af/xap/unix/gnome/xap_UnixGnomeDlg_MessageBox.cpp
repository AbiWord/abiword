/* AbiSource Application Framework
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

#include <gnome.h>
#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_dialogHelper.h"
#include "xap_UnixDlg_MessageBox.h"
#include "xap_UnixGnomeDlg_MessageBox.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

/*****************************************************************/
XAP_Dialog * XAP_UnixGnomeDialog_MessageBox::static_constructor(XAP_DialogFactory * pFactory,
																XAP_Dialog_Id id)
{
	XAP_UnixGnomeDialog_MessageBox * p = new XAP_UnixGnomeDialog_MessageBox(pFactory,id);
	return p;
}

XAP_UnixGnomeDialog_MessageBox::XAP_UnixGnomeDialog_MessageBox(XAP_DialogFactory * pDlgFactory,
															   XAP_Dialog_Id id)
	: XAP_UnixDialog_MessageBox(pDlgFactory,id)
{
}

XAP_UnixGnomeDialog_MessageBox::~XAP_UnixGnomeDialog_MessageBox(void)
{
}

/*****************************************************************/

void XAP_UnixGnomeDialog_MessageBox::runModal(XAP_Frame * pFrame)
{
	GtkWidget * dialog_window;
	GtkWidget * ok_button;
	GtkWidget *	cancel_button;
	GtkWidget *	yes_button;
	GtkWidget *	no_button;
	// answer[0]  -> the default answer
	// answer[1]  -> the answer that correspond to the first button
	// answer[2]  -> the answer that correspond to the second button...
	XAP_Dialog_MessageBox::tAnswer answer[4];

	m_pUnixFrame = (XAP_UnixFrame *)pFrame;
	UT_ASSERT(m_pUnixFrame);
	XAP_UnixApp * pApp = (XAP_UnixApp *)m_pUnixFrame->getApp();
	UT_ASSERT(pApp);

	const char * szCaption = pApp->getApplicationTitleForTitleBar();

	switch (m_buttons)
	{
	case b_O:
		// we never put up an ok dialog if it's not an error condition
	        // TODO: make a b_error so we can make the distinction between "OK" Ok and "Error" Ok
		dialog_window = gnome_message_box_new(m_szMessage, GNOME_MESSAGE_BOX_ERROR, 
						      GNOME_STOCK_BUTTON_OK, NULL);

		ok_button = GTK_WIDGET (g_list_last (GNOME_DIALOG (dialog_window)->buttons)->data);
		gtk_widget_grab_focus (ok_button);
		answer[0] = XAP_Dialog_MessageBox::a_OK;
		answer[1] = XAP_Dialog_MessageBox::a_OK;
		break;

	case b_OC:
	        // OK - Cancel
	        // most certainly a question
		dialog_window = gnome_message_box_new(m_szMessage, GNOME_MESSAGE_BOX_QUESTION,
						      GNOME_STOCK_BUTTON_OK, GNOME_STOCK_BUTTON_CANCEL,
						      NULL);

		ok_button = GTK_WIDGET (g_list_first (GNOME_DIALOG (dialog_window)->buttons)->data);
		cancel_button = GTK_WIDGET (g_list_last (GNOME_DIALOG (dialog_window)->buttons)->data);

		if (m_defaultAnswer == a_OK)
			gtk_widget_grab_focus (ok_button);

		if (m_defaultAnswer == a_NO)
			gtk_widget_grab_focus (cancel_button);

		answer[0] = XAP_Dialog_MessageBox::a_CANCEL;
		answer[1] = XAP_Dialog_MessageBox::a_OK;
		answer[2] = XAP_Dialog_MessageBox::a_CANCEL;
		break;

	case b_YN:
		// YES - NO
	        // most certainly a question
	        dialog_window = gnome_message_box_new(m_szMessage, GNOME_MESSAGE_BOX_QUESTION,
						      GNOME_STOCK_BUTTON_YES, GNOME_STOCK_BUTTON_NO,
						      NULL);

		yes_button = GTK_WIDGET (g_list_first (GNOME_DIALOG (dialog_window)->buttons)->data);
		no_button = GTK_WIDGET (g_list_last (GNOME_DIALOG (dialog_window)->buttons)->data);

		if (m_defaultAnswer == a_YES)
			gtk_widget_grab_focus (yes_button);

		if (m_defaultAnswer == a_NO)
			gtk_widget_grab_default (no_button);

		answer[0] = XAP_Dialog_MessageBox::a_NO;
		answer[1] = XAP_Dialog_MessageBox::a_YES;
		answer[2] = XAP_Dialog_MessageBox::a_NO;
		break;

	case b_YNC:
		// YES - NO - CANCEL
        	// this is used for saving files. the message: Unsaved files. Warning looks good here.
	        dialog_window = gnome_message_box_new(m_szMessage, GNOME_MESSAGE_BOX_WARNING,
						      GNOME_STOCK_BUTTON_YES,
						      GNOME_STOCK_BUTTON_NO,
						      GNOME_STOCK_BUTTON_CANCEL,
						      NULL);

		yes_button = GTK_WIDGET (g_list_first (GNOME_DIALOG (dialog_window)->buttons)->data);
		no_button = GTK_WIDGET (g_list_next (g_list_first (GNOME_DIALOG (dialog_window)->buttons))->data);
		cancel_button = GTK_WIDGET (g_list_last (GNOME_DIALOG (dialog_window)->buttons)->data);

		if (m_defaultAnswer == a_YES)
			gtk_widget_grab_focus (yes_button);

		if (m_defaultAnswer == a_NO)
			gtk_widget_grab_default (no_button);

		if (m_defaultAnswer == a_CANCEL)
			gtk_widget_grab_default (cancel_button);

		answer[0] = XAP_Dialog_MessageBox::a_CANCEL;
		answer[1] = XAP_Dialog_MessageBox::a_YES;
		answer[2] = XAP_Dialog_MessageBox::a_NO;
		answer[3] = XAP_Dialog_MessageBox::a_CANCEL;
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	// get top level window and it's GtkWidget *
	XAP_UnixFrame * frame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(frame);
	GtkWidget * parent = frame->getTopLevelWindow();
	UT_ASSERT(parent);

	gtk_window_set_title(GTK_WINDOW(dialog_window), szCaption);
	centerDialog(parent, dialog_window);

	m_answer = answer[gnome_dialog_run_and_close (GNOME_DIALOG (dialog_window)) + 1];

	// the caller can get the answer from getAnswer().

	m_pUnixFrame = NULL;
}

