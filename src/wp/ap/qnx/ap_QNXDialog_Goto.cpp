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
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Dialog_Id.h"
#include "ap_Dialog_Goto.h"
#include "ap_QNXDialog_Goto.h"
#include "ut_qnxHelper.h"

/*****************************************************************/
XAP_Dialog * AP_QNXDialog_Goto::static_constructor(XAP_DialogFactory * pFactory,
												   XAP_Dialog_Id id)
{
	AP_QNXDialog_Goto * p = new AP_QNXDialog_Goto(pFactory,id);
	return p;
}

AP_QNXDialog_Goto::AP_QNXDialog_Goto(XAP_DialogFactory * pDlgFactory,
									   XAP_Dialog_Id id)
	: AP_Dialog_Goto(pDlgFactory,id)
{
#if 0
	m_gotoString = NULL;
	m_replaceString = NULL;
    m_matchCase = UT_TRUE;
#endif
}

AP_QNXDialog_Goto::~AP_QNXDialog_Goto(void)
{
}

static void s_gotoCallback(PtWidget_t * widget, AP_QNXDialog_Goto * repDialog)
{
#if 0
	UT_ASSERT(widget);
	UT_ASSERT(repDialog);

	char * findEntryText;

	findEntryText = (char *) gtk_entry_get_text(GTK_ENTRY(repDialog->findEntry));
	
	UT_DEBUGMSG(("Find entry contents: \"%s\"\n", ((findEntryText) ? findEntryText : "NULL")));

	UT_UCSChar * findString;

	UT_UCS_cloneString_char(&findString, findEntryText);
	
	repDialog->setFindString(findString);
	
	repDialog->findNext();

	FREEP(findString);
#endif
}

static void s_closeCallback(PtWidget_t * object, PtWidget_t * data)
{
#if 0
	UT_ASSERT(object);
	gtk_main_quit();
#endif
}

static void s_delete_clicked(PtWidget_t * widget, void * data, void * extra)
{
#if 0
	// just quit out of the dialog
	gtk_main_quit();
#endif
}

void AP_QNXDialog_Goto::activate() {
}

void AP_QNXDialog_Goto::destroy() {
}

void AP_QNXDialog_Goto::runModeless(XAP_Frame * pFrame)
{
#if 0
	PtWidget_t * topLevel;
	PtWidget_t * vbox;
	
	PtWidget_t * targetList;
	PtWidget_t * findLabel;
	PtWidget_t * toggleBox;
	PtWidget_t * replaceBox;
	PtWidget_t * replaceLabel;
	PtWidget_t * separator;
	PtWidget_t * buttonBox;
	PtWidget_t * findButton;
	PtWidget_t * replaceButton;
	PtWidget_t * replaceAllButton;
	PtWidget_t * cancelButton;

	// create a top level window, the actual dialog
	topLevel = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_signal_connect_after(GTK_OBJECT(topLevel),
							 "destroy",
							 NULL,
							 NULL);
	gtk_signal_connect_after(GTK_OBJECT(topLevel),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 NULL);

	// don't let user shrink or expand, but auto-size to
	// contents initially
    gtk_window_set_policy(GTK_WINDOW(topLevel),
						  FALSE,
						  FALSE,
						  TRUE);

	// create a vertical stacked box to put our widgets in
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(topLevel), vbox);
	gtk_widget_show(vbox);

	// create a container for the target list
	findBox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox), findBox, TRUE, TRUE, 10);
	gtk_widget_show(findBox);

	findEntry = gtk_entry_new_with_max_length(50);
	gtk_box_pack_end(GTK_BOX(findBox), findEntry, TRUE, TRUE, 10);
	gtk_widget_show(findEntry);

	gtk_signal_connect(GTK_OBJECT(findEntry),
					   "activate",
					   GTK_SIGNAL_FUNC(FindCallback),
					   this);

	// this dialog is persistent, so we set our text to what
	// it was last time
	{
		UT_UCSChar * bufferUnicode = getFindString();
		char * bufferNormal = (char *) calloc(UT_UCS_strlen(bufferUnicode) + 1, sizeof(char));
		UT_UCS_strcpy_to_char(bufferNormal, bufferUnicode);
		FREEP(bufferUnicode);
		
		gtk_entry_set_text(GTK_ENTRY(findEntry), bufferNormal);
		gtk_entry_select_region(GTK_ENTRY(findEntry), 0, GTK_ENTRY(findEntry)->text_length);

		FREEP(bufferNormal);
	}

	// create the find label
	findLabel = gtk_label_new("Find: ");
	gtk_label_set_justify(GTK_LABEL(findLabel), GTK_JUSTIFY_RIGHT);
	gtk_widget_set_usize(findLabel, 150, 0);
	gtk_box_pack_end(GTK_BOX(findBox), findLabel, TRUE, TRUE, 0);
	gtk_widget_show(findLabel);

	// create container for Match Case Toggle
	toggleBox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), toggleBox, TRUE, TRUE, 5);
	gtk_widget_show (toggleBox);

	// optional toggle switch for case
	matchCaseCheck = gtk_check_button_new_with_label("Match Case");
	gtk_box_pack_end(GTK_BOX(toggleBox), matchCaseCheck, FALSE, TRUE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(matchCaseCheck), m_matchCase);
	gtk_widget_show(matchCaseCheck);

	// catch the toggled
	gtk_signal_connect(GTK_OBJECT(matchCaseCheck),
					   "toggled",
					   GTK_SIGNAL_FUNC(MatchCaseCallback),
					   (void *) this);

	if (m_id == AP_DIALOG_ID_REPLACE)
	{
		// container for Replace text field
		replaceBox = gtk_hbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), replaceBox, TRUE, TRUE, 5);
		gtk_widget_show(replaceBox);

		replaceEntry = gtk_entry_new_with_max_length(50);
	
		{
			UT_UCSChar * bufferUnicode = getReplaceString();
			char * bufferNormal = (char *) calloc(UT_UCS_strlen(bufferUnicode) + 1, sizeof(char));
			UT_UCS_strcpy_to_char(bufferNormal, bufferUnicode);
			FREEP(bufferUnicode);
		
			gtk_entry_set_text(GTK_ENTRY(replaceEntry), bufferNormal);

			FREEP(bufferNormal);
		}
		
		gtk_box_pack_end (GTK_BOX (replaceBox), replaceEntry, TRUE, TRUE, 10);
		gtk_widget_show (replaceEntry);

		gtk_signal_connect(GTK_OBJECT(replaceEntry),
						   "activate",
						   GTK_SIGNAL_FUNC(ReplaceCallback),
						   this);
	
		replaceLabel = gtk_label_new("Replace With: ");
		gtk_label_set_justify(GTK_LABEL(replaceLabel), GTK_JUSTIFY_RIGHT);
		gtk_widget_set_usize(replaceLabel, 150, 0);
		gtk_box_pack_end(GTK_BOX(replaceBox), replaceLabel, TRUE, TRUE, 0);
		gtk_widget_show(replaceLabel);
	}
	
	// pretty seperator for the action area
	separator = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, TRUE, 5);
	gtk_widget_show(separator);

	// container for buttons
	buttonBox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), buttonBox, FALSE, TRUE, 5);
	gtk_widget_show(buttonBox);

	findButton = gtk_button_new_with_label("Find Next");
	gtk_widget_set_usize(findButton, DEFAULT_BUTTON_WIDTH, 0);
	gtk_box_pack_start(GTK_BOX(buttonBox), findButton, FALSE, FALSE, 0);
	gtk_widget_show(findButton);

	gtk_signal_connect(GTK_OBJECT(findButton),
					   "clicked",
					   GTK_SIGNAL_FUNC(FindCallback),
					   this);
		
	if (m_id == AP_DIALOG_ID_REPLACE)
	{
		replaceButton = gtk_button_new_with_label("Replace");
		gtk_widget_set_usize(replaceButton, DEFAULT_BUTTON_WIDTH, 0);
		gtk_box_pack_start(GTK_BOX(buttonBox), replaceButton, FALSE, FALSE, 0);
		gtk_widget_show(replaceButton);

		gtk_signal_connect(GTK_OBJECT(replaceButton),
						   "clicked",
						   GTK_SIGNAL_FUNC(ReplaceCallback),
						   this);

		replaceAllButton = gtk_button_new_with_label("Replace All");
		gtk_widget_set_usize(replaceAllButton, DEFAULT_BUTTON_WIDTH, 0);
		gtk_box_pack_start(GTK_BOX(buttonBox), replaceAllButton, FALSE, FALSE, 0);
		gtk_widget_show(replaceAllButton);

		gtk_signal_connect(GTK_OBJECT(replaceAllButton),
						   "clicked",
						   GTK_SIGNAL_FUNC(ReplaceAllCallback),
						   this);
	}
	
	cancelButton = gtk_button_new_with_label("Cancel");
	gtk_widget_set_usize(cancelButton, DEFAULT_BUTTON_WIDTH, 0);
	gtk_box_pack_start(GTK_BOX(buttonBox), cancelButton, FALSE, FALSE, 0);
	gtk_widget_show(cancelButton);

	gtk_signal_connect_object(GTK_OBJECT(cancelButton),
							  "clicked",
							  GTK_SIGNAL_FUNC(CancelCallback),
							  GTK_OBJECT(topLevel));

	GTK_WIDGET_SET_FLAGS(findButton, GTK_CAN_DEFAULT);

	if (m_id == AP_DIALOG_ID_REPLACE)
	{
		GTK_WIDGET_SET_FLAGS(replaceButton, GTK_CAN_DEFAULT);
		GTK_WIDGET_SET_FLAGS(replaceAllButton, GTK_CAN_DEFAULT);
	}
	GTK_WIDGET_SET_FLAGS(cancelButton, GTK_CAN_DEFAULT);
	
	// get top level window and it's PtWidget_t *
	XAP_QNXFrame * frame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(frame);
	PtWidget_t * parent = frame->getTopLevelWindow();
	UT_ASSERT(parent);
	// center it
    centerDialog(parent, topLevel);
	gtk_window_set_transient_for(GTK_WINDOW(topLevel), GTK_WINDOW(parent));
	
	if (m_id == AP_DIALOG_ID_FIND)
		gtk_widget_grab_default(findButton);
	else
		gtk_widget_grab_default(replaceButton);

	// Find entry should have focus, for immediate typing
	gtk_widget_grab_focus(findEntry);
	gtk_grab_add(topLevel);

	gtk_widget_show(topLevel);

	// set up search data through base class
	setView(static_cast<FV_View *> (pFrame->getCurrentView()) );
	
	// go
	gtk_main();

	// clean up
	gtk_widget_destroy(topLevel);

#endif
}
