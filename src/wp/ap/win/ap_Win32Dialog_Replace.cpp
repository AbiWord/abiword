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

// TODO make this entire file work -- as is, it's just a
// Unix hacker's stubs, do with this file as you wish.  Even
// worse, this code was branched before the Unix replace dialog
// got finished, so there's no interaction with the document.  

// here's a start?  :)
#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Dialog_Replace.h"
#include "ap_Win32Dialog_Replace.h"

#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)

/*****************************************************************/
AP_Dialog * AP_Win32Dialog_Replace::static_constructor(AP_DialogFactory * pFactory,
													   AP_Dialog_Id id)
{
	AP_Win32Dialog_Replace * p = new AP_Win32Dialog_Replace(pFactory,id);
	return p;
}

AP_Win32Dialog_Replace::AP_Win32Dialog_Replace(AP_DialogFactory * pDlgFactory,
											   AP_Dialog_Id id)
	: AP_Dialog_Replace(pDlgFactory,id)
{

	findString = (char *) 0;
	replaceString = (char *) 0;
    matchCase = true;
}

AP_Win32Dialog_Replace::~AP_Win32Dialog_Replace(void)
{
}

#if 0
// FindCallback()  - called when user hits enter in the Find text field
//  				 or when the <Find Next> button is hit
static void FindCallback(GtkWidget *widget, AP_Win32Dialog_Replace  *repDialog)
{
	gchar *entryText;

	/* TODO:  know who allocates and frees this memory returned from
              gtk_entry_get_text()
    */
	entryText = gtk_entry_get_text(GTK_ENTRY(repDialog->findEntry));
	printf("Entry contents: \"%s\"\n", ((entryText) ? entryText : "NULL"));

/*
	AdvancePointer();
	Highlight();
*/
}
#endif


#if 0
static void ReplaceCallback(GtkWidget *widget, 
							AP_Win32Dialog_Replace  *repDialog)
{
	gchar *replaceText;

	/* TODO:  know who allocates and frees this memory returned from
              gtk_entry_get_text()
    */
	printf("ReplaceCallback... called\n");

	printf("findEntry=%p\n", repDialog->findEntry);

	replaceText = gtk_entry_get_text(GTK_ENTRY(repDialog->replaceEntry));
	printf("replace contents: \"%s\"\n",((replaceText) ? replaceText : "NULL"));
}
#endif

#if 0
static void ReplaceAllCallback(GtkWidget *widget, 
							AP_Win32Dialog_Replace  *repDialog)
{
	gchar *replaceText;

	/* TODO:  know who allocates and frees this memory returned from
              gtk_entry_get_text()
    */
	printf("ReplaceCallback... called\n");
	replaceText = gtk_entry_get_text(GTK_ENTRY(repDialog->replaceEntry));
	printf("replace contents: \"%s\"\n", replaceText);
}
#endif

#if 0
static void MatchCaseCallback(GtkWidget *checkbutton, GtkWidget *entry)
{
	gtk_entry_set_visibility(GTK_ENTRY(entry),
						GTK_TOGGLE_BUTTON(checkbutton)->active);
	printf("MatchCaseCallback(): I've been called\n");
}

static void CancelCallback(GtkWidget *button, GtkWidget *entry)
{
	printf("Cancel button called \n");
}
#endif



void AP_Win32Dialog_Replace::runModal(AP_Frame * pFrame)
{
	// this is probably completely useless, right?
#if 0
	GtkWidget *topLevel;
	GtkWidget *vbox;
	GtkWidget *findBox;
	GtkWidget *findLabel;
	GtkWidget *toggleBox;
	GtkWidget *replaceBox;
	GtkWidget *replaceLabel;
	GtkWidget *separator;
	GtkWidget *buttonBox;
	GtkWidget *findButton;
	GtkWidget *replaceButton;
	GtkWidget *replaceAllButton;
	GtkWidget *cancelButton;

	/* create the top level widget */
	topLevel = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_usize( GTK_WIDGET (topLevel), 300, 140);
	gtk_window_set_title(GTK_WINDOW (topLevel), "Replace");
/*Fix this */
	gtk_signal_connect(GTK_OBJECT (topLevel), "delete_event",
						(GtkSignalFunc) gtk_exit, NULL);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (topLevel), vbox);
	gtk_widget_show (vbox);

	/* container for Find text field */
	findBox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), findBox, TRUE, TRUE, 10);
	gtk_widget_show (findBox);

	findEntry = gtk_entry_new_with_max_length (50);
	gtk_signal_connect(GTK_OBJECT(findEntry), "activate",
						GTK_SIGNAL_FUNC(FindCallback),
						GTK_OBJECT(this));

	/************************************************
	// set text to what was in persistent
	if (findString && strlen(findString))
	{
		gtk_entry_set_text (GTK_ENTRY (findEntry), findString);
		gtk_entry_select_region (GTK_ENTRY (findEntry),
				0, GTK_ENTRY(findEntry)->text_length);
	}
	 ************************************************/

	gtk_box_pack_end (GTK_BOX (findBox), findEntry, TRUE, TRUE, 10);
	gtk_widget_show (findEntry);

	findLabel = gtk_label_new("            Find:");
	gtk_box_pack_end (GTK_BOX (findBox), findLabel, TRUE, TRUE, 0);
	gtk_widget_show (findLabel);




	/* create container for Match Case Toggle */
	toggleBox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), toggleBox, TRUE, TRUE, 5);
	gtk_widget_show (toggleBox);

	matchCaseCheck = gtk_check_button_new_with_label("Match Case");
	gtk_box_pack_end (GTK_BOX (toggleBox), matchCaseCheck, FALSE, TRUE, 100);

	gtk_signal_connect (GTK_OBJECT(matchCaseCheck), "toggled",
						GTK_SIGNAL_FUNC(MatchCaseCallback), this);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(matchCaseCheck), matchCase);
	gtk_widget_show (matchCaseCheck);




	/* continer for Replace text field */
	replaceBox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), replaceBox, TRUE, TRUE, 5);
	gtk_widget_show (replaceBox);

	replaceEntry = gtk_entry_new_with_max_length (50);
	gtk_signal_connect(GTK_OBJECT(replaceEntry), "activate",
						GTK_SIGNAL_FUNC(ReplaceCallback),
						this);

	if (replaceString && strlen(replaceString))
	{
		gtk_entry_set_text (GTK_ENTRY (replaceEntry), replaceString);
	}
	gtk_box_pack_end (GTK_BOX (replaceBox), replaceEntry, TRUE, TRUE, 10);
	gtk_widget_show (replaceEntry);

	replaceLabel = gtk_label_new("Replace With:");
	gtk_box_pack_end (GTK_BOX (replaceBox), replaceLabel, TRUE, TRUE, 0);
	gtk_widget_show (replaceLabel);


	separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, TRUE, 5);
	gtk_widget_show (separator);


	/* container for buttons */
	buttonBox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), buttonBox, FALSE, TRUE, 5);
	gtk_widget_show (buttonBox);

	findButton = gtk_button_new_with_label (" Find Next ");
	gtk_signal_connect_object (GTK_OBJECT (findButton), "clicked",
								GTK_SIGNAL_FUNC(FindCallback),
								GTK_OBJECT (this));
	printf("findEntry=%p\n",findEntry);
	
	gtk_box_pack_start (GTK_BOX (buttonBox), findButton, TRUE, FALSE, 0);
	gtk_widget_show (findButton);

	replaceButton = gtk_button_new_with_label (" Replace ");
	gtk_signal_connect_object (GTK_OBJECT (replaceButton), "clicked",
								GTK_SIGNAL_FUNC(ReplaceCallback),
								GTK_OBJECT (this));
	gtk_box_pack_start (GTK_BOX (buttonBox), replaceButton, TRUE, FALSE, 0);
	gtk_widget_show (replaceButton);

	replaceAllButton = gtk_button_new_with_label (" Replace All ");
	gtk_box_pack_start (GTK_BOX (buttonBox), replaceAllButton, TRUE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (replaceButton), "clicked",
								GTK_SIGNAL_FUNC(ReplaceAllCallback),
								GTK_OBJECT (this));
	gtk_widget_show (replaceAllButton);


	cancelButton = gtk_button_new_with_label (" Cancel ");
	gtk_signal_connect_object(GTK_OBJECT (cancelButton), "clicked",
								GTK_SIGNAL_FUNC(CancelCallback),
								GTK_OBJECT (this));
	gtk_signal_connect_object(GTK_OBJECT (cancelButton), "clicked",
								(GtkSignalFunc)gtk_widget_hide,
								GTK_OBJECT (topLevel));

	gtk_box_pack_start (GTK_BOX (buttonBox), cancelButton, TRUE, FALSE, 0);
	gtk_widget_show (cancelButton);


	gtk_box_pack_start (GTK_BOX (vbox), buttonBox, TRUE, TRUE, 0);

	GTK_WIDGET_SET_FLAGS (findButton, GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS (replaceButton, GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS (replaceAllButton, GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS (cancelButton, GTK_CAN_DEFAULT);
//  gtk_widget_grab_default (findButton);

 	gtk_widget_show(topLevel);

#endif
}


