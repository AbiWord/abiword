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
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)pFrame;
	UT_return_if_fail(pUnixFrame);

	XAP_UnixApp * pApp = (XAP_UnixApp *)pUnixFrame->getApp();
	UT_return_if_fail(pApp);

	const char * szCaption = pApp->getApplicationTitleForTitleBar();

	GtkWidget * message ;
	GtkWindow * toplevel = GTK_WINDOW(pUnixFrame->getTopLevelWindow());
	
	switch (m_buttons)
	{
		case b_O:
	        // just put up an information box
			message = gtk_message_dialog_new ( toplevel, GTK_DIALOG_MODAL,
											   GTK_MESSAGE_INFO,
											   GTK_BUTTONS_OK,
											   m_szMessage ) ;
			break;

		case b_OC:
	        // OK - Cancel - most certainly a question
			message = gtk_message_dialog_new ( toplevel, GTK_DIALOG_MODAL,
											   GTK_MESSAGE_QUESTION,
											   GTK_BUTTONS_OK_CANCEL,
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
        	// this is only used for saving files. Warning looks good here, but possibly should change to a Question
	        message = gtk_message_dialog_new ( toplevel, GTK_DIALOG_MODAL,
											   GTK_MESSAGE_WARNING,
											   GTK_BUTTONS_NONE,
											   m_szMessage ) ;
			abiAddStockButton ( GTK_DIALOG(message), GTK_STOCK_CANCEL,
								GTK_RESPONSE_CANCEL ) ;
			abiAddStockButton ( GTK_DIALOG(message), GTK_STOCK_NO,
								GTK_RESPONSE_NO ) ;
			abiAddStockButton ( GTK_DIALOG(message), GTK_STOCK_YES,
								GTK_RESPONSE_YES ) ;			
			break;
			
		default:
			UT_ASSERT_NOT_REACHED();
	}

	gtk_window_set_title ( GTK_WINDOW(message), szCaption ) ;
	gtk_window_set_role (GTK_WINDOW(message), "message dialog");
						   	
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

