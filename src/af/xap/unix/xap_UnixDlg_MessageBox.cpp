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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include "ut_assert.h"
#include "ut_vector.h"
#include "xap_UnixDialog_MessageBox.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

/*****************************************************************/
AP_Dialog * AP_UnixDialog_MessageBox::static_constructor(AP_DialogFactory * pFactory,
														 AP_Dialog_Id id)
{
	AP_UnixDialog_MessageBox * p = new AP_UnixDialog_MessageBox(pFactory,id);
	return p;
}

AP_UnixDialog_MessageBox::AP_UnixDialog_MessageBox(AP_DialogFactory * pDlgFactory,
												   AP_Dialog_Id id)
	: AP_Dialog_MessageBox(pDlgFactory,id)
{
}

AP_UnixDialog_MessageBox::~AP_UnixDialog_MessageBox(void)
{
	UT_VECTOR_PURGEALL(AP_UnixDialog_MessageBox::keyBinding *, m_keyBindings);
}

/*****************************************************************/

static gint s_key_pressed(GtkWidget * widget, GdkEventKey * e, AP_UnixDialog_MessageBox * box)
{
	UT_ASSERT(e);
	UT_ASSERT(box);

	const UT_Vector * bindings = box->_getBindingsVector();
	
	guint key = e->keyval;
	AP_UnixDialog_MessageBox::keyBinding * item = NULL;
	
	// find binding
	for (unsigned int i = 0; i < bindings->getItemCount(); i++)
	{
		item = (AP_UnixDialog_MessageBox::keyBinding *) bindings->getNthItem(i);
		UT_ASSERT(item);

		// execute action
		if (item->key == key)
		{
			switch(item->answer)
			{
			case AP_Dialog_MessageBox::a_OK:
				box->_setAnswer(AP_Dialog_MessageBox::a_OK);
				gtk_main_quit();
				return TRUE;
				break;
			case AP_Dialog_MessageBox::a_CANCEL:
				box->_setAnswer(AP_Dialog_MessageBox::a_CANCEL);
				gtk_main_quit();
				return TRUE;
				break;
			case AP_Dialog_MessageBox::a_YES:
				box->_setAnswer(AP_Dialog_MessageBox::a_YES);
				gtk_main_quit();
				return TRUE;
				break;
			case AP_Dialog_MessageBox::a_NO:
				box->_setAnswer(AP_Dialog_MessageBox::a_NO);
				gtk_main_quit();
				return TRUE;
				break;
			default:
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		}
	}
	
	return TRUE;
}

static void s_ok_clicked(GtkWidget * widget,
						 AP_Dialog_MessageBox::tAnswer * answer)
{
	*answer = AP_Dialog_MessageBox::a_OK;
	gtk_main_quit();
}

static void s_cancel_clicked(GtkWidget * widget,
							 AP_Dialog_MessageBox::tAnswer * answer)
{
	*answer = AP_Dialog_MessageBox::a_CANCEL;
	gtk_main_quit();
}

static void s_yes_clicked(GtkWidget * widget,
						  AP_Dialog_MessageBox::tAnswer * answer)
{
	*answer = AP_Dialog_MessageBox::a_YES;
	gtk_main_quit();
}

static void s_no_clicked(GtkWidget * widget,
						 AP_Dialog_MessageBox::tAnswer * answer)
{
	*answer = AP_Dialog_MessageBox::a_NO;
	gtk_main_quit();
}

/*****************************************************************/

void AP_UnixDialog_MessageBox::_bindKey(guint key, AP_Dialog_MessageBox::tAnswer answer)
{
	AP_UnixDialog_MessageBox::keyBinding * item = new AP_UnixDialog_MessageBox::keyBinding;

	item->key = key;
	item->answer = answer;
	
	m_keyBindings.addItem((void *) item);
}

/*****************************************************************/

// static callback helpers, they break encapsulation
UT_Vector * AP_UnixDialog_MessageBox::_getBindingsVector()
{
	return &m_keyBindings;
}

void AP_UnixDialog_MessageBox::_setAnswer(AP_Dialog_MessageBox::tAnswer answer)
{
	m_answer = answer;
}

void AP_UnixDialog_MessageBox::runModal(AP_Frame * pFrame)
{
	m_pUnixFrame = (AP_UnixFrame *)pFrame;
	UT_ASSERT(m_pUnixFrame);
	AP_UnixApp * pApp = (AP_UnixApp *)m_pUnixFrame->getApp();
	UT_ASSERT(pApp);

	const char * szCaption = pApp->getApplicationTitleForTitleBar();

	// New GTK+ dialog window
	GtkWidget * dialog_window = gtk_dialog_new();
	gtk_signal_connect_after (GTK_OBJECT (dialog_window),
							  "destroy",
							  GTK_SIGNAL_FUNC(s_cancel_clicked),
							  NULL);

	gtk_window_set_title (GTK_WINDOW (dialog_window), szCaption);
	gtk_widget_set_usize (dialog_window, 400, 110);

	// Intercept key strokes
	gtk_signal_connect(GTK_OBJECT(dialog_window),
					   "key_press_event",
					   GTK_SIGNAL_FUNC(s_key_pressed),
					   this);

	// Add our label string to the dialog in the message area
	GtkWidget * label = gtk_label_new (m_szMessage);
	gtk_misc_set_padding (GTK_MISC (label), 10, 10);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog_window)->vbox),
						label, TRUE, TRUE, 0);
	gtk_widget_show (label);

	// Build all the buttons, regardless of whether they're
	// used.  This is much easier than creating the ones we know we
	// need.  Trust me.

	GtkWidget *		ok_label;
	GtkWidget * 	ok_button;
	guint			ok_accel;
	
	GtkWidget *		cancel_label;
	GtkWidget *		cancel_button;
	guint 			cancel_accel;
	
	GtkWidget *		yes_label;
	GtkWidget *		yes_button;
	guint			yes_accel;
	
	GtkWidget *		no_label;
	GtkWidget *		no_button;
	guint			no_accel;
	
	// OK
	ok_label = gtk_label_new("SHOULD NOT APPEAR");
	ok_accel = gtk_label_parse_uline(GTK_LABEL(ok_label), "O_K");
	gtk_widget_show(ok_label);
	ok_button = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(ok_button), ok_label);
	gtk_signal_connect (GTK_OBJECT (ok_button),
						"clicked",
						GTK_SIGNAL_FUNC (s_ok_clicked),
						&m_answer);
	GTK_WIDGET_SET_FLAGS (ok_button, GTK_CAN_DEFAULT);
	// Cancel
	cancel_label = gtk_label_new("SHOULD NOT APPEAR");
	cancel_accel = gtk_label_parse_uline(GTK_LABEL(cancel_label), "_Cancel");
	gtk_widget_show(cancel_label);
	cancel_button = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(cancel_button), cancel_label);
	gtk_signal_connect (GTK_OBJECT (cancel_button),
						"clicked",
						GTK_SIGNAL_FUNC (s_cancel_clicked),
						&m_answer);
	GTK_WIDGET_SET_FLAGS (cancel_button, GTK_CAN_DEFAULT);
	// Yes
	yes_label = gtk_label_new("SHOULD NOT APPEAR");
	yes_accel = gtk_label_parse_uline(GTK_LABEL(yes_label), "_Yes");
	gtk_widget_show(yes_label);
	yes_button = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(yes_button), yes_label);
	gtk_signal_connect (GTK_OBJECT (yes_button),
						"clicked",
						GTK_SIGNAL_FUNC (s_yes_clicked),
						&m_answer);
	GTK_WIDGET_SET_FLAGS (yes_button, GTK_CAN_DEFAULT);
	// No
	no_label = gtk_label_new("SHOULD NOT APPEAR");
	no_accel = gtk_label_parse_uline(GTK_LABEL(no_label), "_No");
	gtk_widget_show(no_label);
	no_button = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(no_button), no_label);
	gtk_signal_connect (GTK_OBJECT (no_button),
						"clicked",
						GTK_SIGNAL_FUNC (s_no_clicked),
						&m_answer);
	GTK_WIDGET_SET_FLAGS (no_button, GTK_CAN_DEFAULT);

	switch (m_buttons)
	{
	case b_O:
		// OK
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog_window)->action_area),
							ok_button, TRUE, TRUE, 0);
		gtk_widget_grab_default (ok_button);
		_bindKey(ok_accel, a_OK);
		_bindKey(GDK_Escape, a_OK);
		gtk_widget_show (ok_button);
		break;

	case b_OC:
		// OK
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog_window)->action_area),
							ok_button, TRUE, TRUE, 0);
		if (m_defaultAnswer == a_OK)
			gtk_widget_grab_default (ok_button);
		_bindKey(ok_accel, a_OK);
		gtk_widget_show (ok_button);
		// Cancel
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog_window)->action_area),
							cancel_button, TRUE, TRUE, 0);
		if (m_defaultAnswer == a_NO)
			gtk_widget_grab_default (cancel_button);
		_bindKey(cancel_accel, a_CANCEL);
		_bindKey(GDK_Escape, a_CANCEL);
		gtk_widget_show (cancel_button);
		break;

	case b_YN:
		// Yes
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog_window)->action_area),
							yes_button, TRUE, TRUE, 0);
		if (m_defaultAnswer == a_YES)
			gtk_widget_grab_default (yes_button);
		_bindKey(yes_accel, a_YES);
		gtk_widget_show (yes_button);
		// No
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog_window)->action_area),
							no_button, TRUE, TRUE, 0);
		if (m_defaultAnswer == a_NO)
			gtk_widget_grab_default (no_button);
		_bindKey(no_accel, a_NO);
		_bindKey(GDK_Escape, a_NO);
		gtk_widget_show (no_button);
		break;

	case b_YNC:
		// Yes
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog_window)->action_area),
							yes_button, TRUE, TRUE, 0);
		if (m_defaultAnswer == a_YES)
			gtk_widget_grab_default (yes_button);
		_bindKey(yes_accel, a_YES);
		gtk_widget_show (yes_button);
		// No
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog_window)->action_area),
							no_button, TRUE, TRUE, 0);
		if (m_defaultAnswer == a_NO)
			gtk_widget_grab_default (no_button);
		_bindKey(no_accel, a_NO);
		gtk_widget_show (no_button);
		// Cancel
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog_window)->action_area),
							cancel_button, TRUE, TRUE, 0);
		if (m_defaultAnswer == a_CANCEL)
			gtk_widget_grab_default (cancel_button);
		gtk_widget_show (cancel_button);
		_bindKey(cancel_accel, a_CANCEL);
		_bindKey(GDK_Escape, a_CANCEL);
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	gtk_window_set_position(GTK_WINDOW(dialog_window), GTK_WIN_POS_CENTER);
	gtk_grab_add(GTK_WIDGET(dialog_window));
	gtk_widget_show (dialog_window);

	// TODO maybe add some key bindings so that escape will cancel, etc.
	gtk_main();

	gtk_widget_destroy(label);
	gtk_widget_destroy(ok_label);
	gtk_widget_destroy(ok_button);
	gtk_widget_destroy(cancel_label);
	gtk_widget_destroy(cancel_button);
	gtk_widget_destroy(yes_label);
	gtk_widget_destroy(yes_button);
	gtk_widget_destroy(no_label);
	gtk_widget_destroy(no_button);	
	gtk_widget_destroy(GTK_WIDGET(dialog_window));

	// answer should be set by the appropriate callback
	// the caller can get the answer from getAnswer().

	m_pUnixFrame = NULL;
}

