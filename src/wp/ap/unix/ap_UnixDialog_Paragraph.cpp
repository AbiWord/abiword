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
#include "ap_UnixDialog_Paragraph.h"

/*****************************************************************/

#define WIDGET_DIALOG_TAG "dialog"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Paragraph::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	AP_UnixDialog_Paragraph * p = new AP_UnixDialog_Paragraph(pFactory,id);
	return p;
}

AP_UnixDialog_Paragraph::AP_UnixDialog_Paragraph(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_Paragraph(pDlgFactory,id)
{
	m_unixGraphics = NULL;
}

AP_UnixDialog_Paragraph::~AP_UnixDialog_Paragraph(void)
{
	DELETEP(m_unixGraphics);
}

/*****************************************************************/

// sample callback function
static void s_ok_clicked(GtkWidget * widget, AP_UnixDialog_Paragraph * dlg)
{ UT_ASSERT(widget && dlg); dlg->event_OK(); }

static void s_cancel_clicked(GtkWidget * widget, AP_UnixDialog_Paragraph * dlg)
{ UT_ASSERT(widget && dlg); dlg->event_Cancel(); }

static void s_tabs_clicked(GtkWidget * widget, AP_UnixDialog_Paragraph * dlg)
{ UT_ASSERT(widget && dlg);	dlg->event_Tabs(); }

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 AP_UnixDialog_Paragraph * dlg)
{ UT_ASSERT(dlg); dlg->event_WindowDelete(); }

// spins and lists

/*
static void s_option_alignment_toggle(GtkWidget * widget, GtkCList * clist)
{
  gint i;

  if (!GTK_WIDGET_MAPPED (widget))
    return;

  RADIOMENUTOGGLED ((GtkRadioMenuItem *)
		    (((GtkOptionMenu *)clist_omenu)->menu_item), i);

  gtk_clist_set_selection_mode (clist, (GtkSelectionMode) (3-i));
}
*/

#if 0
static gboolean s_spin_editable_changed(GtkWidget * widget, AP_UnixDialog_Paragraph * dlg)
{
	UT_ASSERT(widget && dlg);

//	dlg->event_UpdateEntry(widget);
	
	// do NOT let GTK do its own update (which would erase the text we just
	// put in the entry area
	return FALSE;
}
#endif

#if 0
static gboolean s_spin_indent_changed(GtkAdjustment * adjustment, AP_UnixDialog_Paragraph * dlg)
{
	UT_ASSERT(adjustment && dlg);

	// GTK just prints the value of the adjustment into a string and slaps
	// it into the dialog.  We need to preserve the existing units while
	// doing something similar.

	GtkWidget * spinbutton = (GtkWidget *)
		gtk_object_get_data(GTK_OBJECT(adjustment), WIDGET_DIALOG_TAG);

	UT_ASSERT(dlg);

	// fire the event to the dialog
	dlg->event_UnitSpinButtonChanged(spinbutton, adjustment);

	// do NOT let GTK continue with its evaluation of the adjustment event
	// (which would set a silly new text in the spin button)
	return FALSE;
}
#endif

// toggle buttons

#if 0
static void s_check_widoworphancontrol_toggled(GtkWidget * widget, AP_UnixDialog_Paragraph * dlg)
{ UT_ASSERT(widget && dlg); dlg->event_WidowOrphanControlToggled(); }

static void s_check_keeplinestogether_toggled(GtkWidget * widget, AP_UnixDialog_Paragraph * dlg)
{ UT_ASSERT(widget && dlg); dlg->event_KeepLinesTogetherToggled(); }

static void s_check_keepwithnext_toggled(GtkWidget * widget, AP_UnixDialog_Paragraph * dlg)
{ UT_ASSERT(widget && dlg); dlg->event_KeepWithNextToggled(); }

static void s_check_pagebreakbefore_toggled(GtkWidget * widget, AP_UnixDialog_Paragraph * dlg)
{ UT_ASSERT(widget && dlg); dlg->event_PageBreakBeforeToggled(); }

static void s_check_suppresslinenumbers_toggled(GtkWidget * widget, AP_UnixDialog_Paragraph * dlg)
{ UT_ASSERT(widget && dlg); dlg->event_SuppressLineNumbersToggled(); }

static void s_check_nohyphenate_toggled(GtkWidget * widget, AP_UnixDialog_Paragraph * dlg)
{ UT_ASSERT(widget && dlg); dlg->event_NoHyphenateToggled(); }
#endif

// preview drawing area

static gint s_preview_exposed(GtkWidget * /* widget */,
							  GdkEventExpose * /* pExposeEvent */,
							  AP_UnixDialog_Paragraph * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_PreviewAreaExposed();

	return FALSE;
}

/*****************************************************************/

void AP_UnixDialog_Paragraph::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Populate the window's data items
	_populateWindowData();
	
	// To center the dialog, we need the frame of its parent.
	XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(pUnixFrame);
	
	// Get the GtkWindow of the parent frame
	GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
    centerDialog(parentWindow, mainWindow);
	gtk_window_set_transient_for(GTK_WINDOW(mainWindow), GTK_WINDOW(parentWindow));

	// Show the top level dialog,
	gtk_widget_show(mainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(mainWindow);

	// *** this is how we add the gc ***
	{
		// attach a new graphics context to the drawing area
		XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
		UT_ASSERT(unixapp);

		UT_ASSERT(m_drawingareaPreview && m_drawingareaPreview->window);

		// make a new Unix GC
		m_unixGraphics = new GR_UnixGraphics(m_drawingareaPreview->window, unixapp->getFontManager());
		
		// let the widget materialize
		_createPreviewFromGC(m_unixGraphics,
							 (UT_uint32) m_drawingareaPreview->allocation.width,
							 (UT_uint32) m_drawingareaPreview->allocation.height);
	}

	// Run into the GTK event loop for this window.
	gtk_main();

	_storeWindowData();
	
	gtk_widget_destroy(mainWindow);
}

void AP_UnixDialog_Paragraph::event_OK(void)
{
	m_answer = AP_Dialog_Paragraph::a_OK;
	gtk_main_quit();
}

void AP_UnixDialog_Paragraph::event_Cancel(void)
{
	m_answer = AP_Dialog_Paragraph::a_CANCEL;
	gtk_main_quit();
}

void AP_UnixDialog_Paragraph::event_Tabs(void)
{
	m_answer = AP_Dialog_Paragraph::a_TABS;
	gtk_main_quit();
}

void AP_UnixDialog_Paragraph::event_WindowDelete(void)
{
	m_answer = AP_Dialog_Paragraph::a_CANCEL;	
	gtk_main_quit();
}

/****************************************/

// Alignment methods

void AP_UnixDialog_Paragraph::event_AlignmentChanged(void) { }

/****************************************/

// generic methods for spin buttons
void AP_UnixDialog_Paragraph::event_UpdateEntry(GtkWidget * widget)
{
#if 0
	gchar * oldtext = gtk_entry_get_text(GTK_ENTRY(widget));

	XML_Char * newtext = _filterUserInput((XML_Char *) oldtext);

	// we have to protect this section with a lock, so that updating
	// the text doesn't trigger this same event
	{
		gtk_object_set_data(GTK_OBJECT(widget), "updatelock", (void *) TRUE);
	
		if ( ((gboolean) gtk_object_get_data(GTK_OBJECT(widget), "updatelock")) == FALSE)
			gtk_entry_set_text(GTK_ENTRY(widget), (const gchar *) newtext);

		gtk_object_set_data(GTK_OBJECT(widget), "updatelock", (void *) FALSE);
	}
#endif
}

#if 0
void AP_UnixDialog_Paragraph::event_UnitSpinButtonChanged(GtkWidget * spinbutton, GtkAdjustment * adj)
{
	UT_ASSERT(spinbutton && adj);
	
	const char * newvalue =
		UT_convertToDimensionString(m_dim, (double) adj->value, ".1");

	UT_ASSERT(newvalue);

	GtkSpinButton * sb = GTK_SPIN_BUTTON(spinbutton);

	GtkEntry * entry = & sb->entry;
	
	gtk_entry_set_text(entry, newvalue);
}

void AP_UnixDialog_Paragraph::event_UnitlessSpinButtonChanged(GtkWidget * spinbutton, GtkAdjustment * adj)
{
	UT_ASSERT(spinbutton && adj);
	
	const char * newvalue =
		UT_convertToDimensionString(m_dim, (double) adj->value, ".1");

	UT_ASSERT(newvalue);

	GtkSpinButton * sb = GTK_SPIN_BUTTON(spinbutton);

	GtkEntry * entry = & sb->entry;
	
	gtk_entry_set_text(entry, newvalue);
}
#endif

/****************************************/

void AP_UnixDialog_Paragraph::event_PreviewAreaExposed(void)
{
	if (m_paragraphPreview)
		m_paragraphPreview->draw();
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_Paragraph::_constructWindow(void)
{
	// grab the string set
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	GtkWidget * windowParagraph;
	GtkWidget * vboxMain;
	GtkWidget * fixedMain;
	GtkWidget * tabMain;
	GtkWidget * fixedSpacing;
	GtkWidget * listAlignment;
	GtkWidget * listAlignment_menu;
	GtkWidget * glade_menuitem;
	GtkObject * spinbuttonLeft_adj;
	GtkWidget * spinbuttonLeft;
	GtkObject * spinbuttonRight_adj;
	GtkWidget * spinbuttonRight;
	GtkWidget * listSpecial;
	GtkWidget * listSpecial_menu;
	GtkObject * spinbuttonBy_adj;
	GtkWidget * spinbuttonBy;
	GtkObject * spinbuttonBefore_adj;
	GtkWidget * spinbuttonBefore;
	GtkObject * spinbuttonAfter_adj;
	GtkWidget * spinbuttonAfter;
	GtkWidget * listLineSpacing;
	GtkWidget * listLineSpacing_menu;
	GtkObject * spinbuttonAt_adj;
	GtkWidget * spinbuttonAt;
	GtkWidget * labelAlignment;
	GtkWidget * labelBy;
	GtkWidget * labelIndentation;
	GtkWidget * labelLeft;
	GtkWidget * labelRight;
	GtkWidget * labelSpecial;
	GtkWidget * hseparator3;
	GtkWidget * labelSpacing;
	GtkWidget * labelAfter;
	GtkWidget * labelLineSpacing;
	GtkWidget * labelAt;
	GtkWidget * labelPreview;

	GtkWidget * framePreview;
	GtkWidget * drawingareaPreview;

	GtkWidget * hseparator4;
	GtkWidget * hseparator1;
	GtkWidget * labelBefore;
	GtkWidget * labelIndents;
	GtkWidget * fixedBreaks;
	GtkWidget * labelPagination;
	GtkWidget * hseparator5;
	GtkWidget * hseparator7;
	GtkWidget * labelPreview2;
	GtkWidget * checkbuttonWidowOrphan;
	GtkWidget * checkbuttonKeepLines;
	GtkWidget * checkbuttonPageBreak;
	GtkWidget * checkbuttonSuppress;
	GtkWidget * checkbuttonHyphenate;
	GtkWidget * hseparator6;
	GtkWidget * checkbuttonKeepNext;
	GtkWidget * labelBreaks;
	GtkWidget * hbox1;
	GtkWidget * hbuttonboxLeft;
	GtkWidget * buttonTabs;
	GtkWidget * hbox2;
	GtkWidget * hbuttonboxRight;
	GtkWidget * buttonOK;
	GtkWidget * buttonCancel;

	windowParagraph = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_object_set_data (GTK_OBJECT (windowParagraph), "windowParagraph", windowParagraph);
	gtk_widget_set_usize (windowParagraph, 441, -2);
	gtk_window_set_title (GTK_WINDOW (windowParagraph), pSS->getValue(AP_STRING_ID_DLG_Para_ParaTitle));
	gtk_window_set_policy (GTK_WINDOW (windowParagraph), FALSE, FALSE, FALSE);

	vboxMain = gtk_vbox_new (FALSE, 0);
	gtk_widget_ref (vboxMain);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "vboxMain", vboxMain,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (vboxMain);
	gtk_container_add (GTK_CONTAINER (windowParagraph), vboxMain);

	fixedMain = gtk_fixed_new ();
	gtk_widget_ref (fixedMain);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "fixedMain", fixedMain,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (fixedMain);
	gtk_box_pack_start (GTK_BOX (vboxMain), fixedMain, TRUE, TRUE, 0);

	tabMain = gtk_notebook_new ();
	gtk_widget_ref (tabMain);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "tabMain", tabMain,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (tabMain);
	gtk_fixed_put (GTK_FIXED (fixedMain), tabMain, 0, 0);
	gtk_widget_set_uposition (tabMain, 0, 0);
	gtk_widget_set_usize (tabMain, 440, 352);
	gtk_container_set_border_width (GTK_CONTAINER (tabMain), 10);

	fixedSpacing = gtk_fixed_new ();
	gtk_widget_ref (fixedSpacing);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "fixedSpacing", fixedSpacing,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (fixedSpacing);
	gtk_container_add (GTK_CONTAINER (tabMain), fixedSpacing);

	listAlignment = gtk_option_menu_new ();
	gtk_widget_ref (listAlignment);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "listAlignment", listAlignment,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (listAlignment);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), listAlignment, 104, 8);
	gtk_widget_set_uposition (listAlignment, 104, 8);
	gtk_widget_set_usize (listAlignment, 88, 24);
	listAlignment_menu = gtk_menu_new ();
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_AlignLeft));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listAlignment_menu), glade_menuitem);
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_AlignCentered));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listAlignment_menu), glade_menuitem);
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_AlignRight));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listAlignment_menu), glade_menuitem);
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_AlignJustified));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listAlignment_menu), glade_menuitem);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (listAlignment), listAlignment_menu);

	spinbuttonLeft_adj = gtk_adjustment_new (0, 0, 100, 0.1, 10, 10);
	spinbuttonLeft = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonLeft_adj), 1, 1);
	gtk_widget_ref (spinbuttonLeft);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "spinbuttonLeft", spinbuttonLeft,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (spinbuttonLeft);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), spinbuttonLeft, 104, 56);
	gtk_widget_set_uposition (spinbuttonLeft, 104, 56);
	gtk_widget_set_usize (spinbuttonLeft, 88, 24);
	// set info for callback
	gtk_object_set_data(GTK_OBJECT(spinbuttonLeft_adj), WIDGET_DIALOG_TAG, (gpointer) spinbuttonLeft);
	
	spinbuttonRight_adj = gtk_adjustment_new (0, 0, 100, 0.1, 10, 10);
	spinbuttonRight = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonRight_adj), 1, 1);
	gtk_widget_ref (spinbuttonRight);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "spinbuttonRight", spinbuttonRight,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (spinbuttonRight);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), spinbuttonRight, 104, 80);
	gtk_widget_set_uposition (spinbuttonRight, 104, 80);
	gtk_widget_set_usize (spinbuttonRight, 88, 24);
	// set info for callback
	gtk_object_set_data(GTK_OBJECT(spinbuttonRight_adj), WIDGET_DIALOG_TAG, (gpointer) spinbuttonRight);

	listSpecial = gtk_option_menu_new ();
	gtk_widget_ref (listSpecial);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "listSpecial", listSpecial,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (listSpecial);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), listSpecial, 216, 80);
	gtk_widget_set_uposition (listSpecial, 216, 80);
	gtk_widget_set_usize (listSpecial, 88, 24);
	listSpecial_menu = gtk_menu_new ();
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_SpecialNone));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listSpecial_menu), glade_menuitem);
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_SpecialFirstLine));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listSpecial_menu), glade_menuitem);
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_SpecialHanging));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listSpecial_menu), glade_menuitem);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (listSpecial), listSpecial_menu);

	spinbuttonBy_adj = gtk_adjustment_new (0.5, 0, 100, 0.1, 10, 10);
	spinbuttonBy = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonBy_adj), 1, 1);
	gtk_widget_ref (spinbuttonBy);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "spinbuttonBy", spinbuttonBy,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (spinbuttonBy);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), spinbuttonBy, 312, 80);
	gtk_widget_set_uposition (spinbuttonBy, 312, 80);
	gtk_widget_set_usize (spinbuttonBy, 88, 24);
	// set info for callback
	gtk_object_set_data(GTK_OBJECT(spinbuttonBy_adj), WIDGET_DIALOG_TAG, (gpointer) spinbuttonBy);

	spinbuttonBefore_adj = gtk_adjustment_new (0, 0, 1500, 0.1, 10, 10);
	spinbuttonBefore = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonBefore_adj), 1, 0);
	gtk_widget_ref (spinbuttonBefore);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "spinbuttonBefore", spinbuttonBefore,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (spinbuttonBefore);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), spinbuttonBefore, 104, 128);
	gtk_widget_set_uposition (spinbuttonBefore, 104, 128);
	gtk_widget_set_usize (spinbuttonBefore, 88, 24);
	// set info for callback
	gtk_object_set_data(GTK_OBJECT(spinbuttonBefore_adj), WIDGET_DIALOG_TAG, (gpointer) spinbuttonBefore);

	spinbuttonAfter_adj = gtk_adjustment_new (0, 0, 1500, 0.1, 10, 10);
	spinbuttonAfter = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonAfter_adj), 1, 0);
	gtk_widget_ref (spinbuttonAfter);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "spinbuttonAfter", spinbuttonAfter,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (spinbuttonAfter);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), spinbuttonAfter, 104, 152);
	gtk_widget_set_uposition (spinbuttonAfter, 104, 152);
	gtk_widget_set_usize (spinbuttonAfter, 88, 24);
	// set info for callback
	gtk_object_set_data(GTK_OBJECT(spinbuttonAfter_adj), WIDGET_DIALOG_TAG, (gpointer) spinbuttonAfter);

	listLineSpacing = gtk_option_menu_new ();
	gtk_widget_ref (listLineSpacing);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "listLineSpacing", listLineSpacing,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (listLineSpacing);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), listLineSpacing, 216, 152);
	gtk_widget_set_uposition (listLineSpacing, 216, 152);
	gtk_widget_set_usize (listLineSpacing, 88, 24);
	listLineSpacing_menu = gtk_menu_new ();
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_SpacingSingle));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listLineSpacing_menu), glade_menuitem);
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_SpacingHalf));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listLineSpacing_menu), glade_menuitem);
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_SpacingDouble));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listLineSpacing_menu), glade_menuitem);
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_SpacingAtLeast));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listLineSpacing_menu), glade_menuitem);
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_SpacingExactly));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listLineSpacing_menu), glade_menuitem);
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_SpacingMultiple));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listLineSpacing_menu), glade_menuitem);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (listLineSpacing), listLineSpacing_menu);

	spinbuttonAt_adj = gtk_adjustment_new (0.5, 0, 100, 0.1, 10, 10);
	spinbuttonAt = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonAt_adj), 1, 1);
	gtk_widget_ref (spinbuttonAt);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "spinbuttonAt", spinbuttonAt,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (spinbuttonAt);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), spinbuttonAt, 312, 152);
	gtk_widget_set_uposition (spinbuttonAt, 312, 152);
	gtk_widget_set_usize (spinbuttonAt, 88, 24);
	// set info for callback
	gtk_object_set_data(GTK_OBJECT(spinbuttonAt_adj), WIDGET_DIALOG_TAG, (gpointer) spinbuttonAt);

	labelAlignment = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_LabelAlignment));
	gtk_widget_ref (labelAlignment);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelAlignment", labelAlignment,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelAlignment);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelAlignment, 16, 8);
	gtk_widget_set_uposition (labelAlignment, 16, 8);
	gtk_widget_set_usize (labelAlignment, 80, 24);
	gtk_label_set_justify (GTK_LABEL (labelAlignment), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelAlignment), 7.45058e-09, 0.5);

	labelBy = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_LabelBy));
	gtk_widget_ref (labelBy);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelBy", labelBy,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelBy);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelBy, 312, 56);
	gtk_widget_set_uposition (labelBy, 312, 56);
	gtk_widget_set_usize (labelBy, 88, 24);
	gtk_label_set_justify (GTK_LABEL (labelBy), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelBy), 7.45058e-09, 0.5);

	labelIndentation = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_LabelIndentation));
	gtk_widget_ref (labelIndentation);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelIndentation", labelIndentation,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelIndentation);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelIndentation, 8, 32);
	gtk_widget_set_uposition (labelIndentation, 8, 32);
	gtk_widget_set_usize (labelIndentation, 104, 24);
	gtk_label_set_justify (GTK_LABEL (labelIndentation), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelIndentation), 0, 0.5);

	labelLeft = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_LabelLeft));
	gtk_widget_ref (labelLeft);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelLeft", labelLeft,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelLeft);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelLeft, 16, 56);
	gtk_widget_set_uposition (labelLeft, 16, 56);
	gtk_widget_set_usize (labelLeft, 80, 24);
	gtk_label_set_justify (GTK_LABEL (labelLeft), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelLeft), 0, 0.5);

	labelRight = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_LabelRight));
	gtk_widget_ref (labelRight);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelRight", labelRight,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelRight);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelRight, 16, 80);
	gtk_widget_set_uposition (labelRight, 16, 80);
	gtk_widget_set_usize (labelRight, 80, 24);
	gtk_label_set_justify (GTK_LABEL (labelRight), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelRight), 0, 0.5);

	labelSpecial = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_LabelSpecial));
	gtk_widget_ref (labelSpecial);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelSpecial", labelSpecial,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelSpecial);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelSpecial, 216, 56);
	gtk_widget_set_uposition (labelSpecial, 216, 56);
	gtk_widget_set_usize (labelSpecial, 88, 24);
	gtk_label_set_justify (GTK_LABEL (labelSpecial), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelSpecial), 7.45058e-09, 0.5);

	hseparator3 = gtk_hseparator_new ();
	gtk_widget_ref (hseparator3);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "hseparator3", hseparator3,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hseparator3);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), hseparator3, 64, 104);
	gtk_widget_set_uposition (hseparator3, 64, 104);
	gtk_widget_set_usize (hseparator3, 344, 24);

	labelSpacing = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_LabelSpacing));
	gtk_widget_ref (labelSpacing);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelSpacing", labelSpacing,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelSpacing);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelSpacing, 8, 104);
	gtk_widget_set_uposition (labelSpacing, 8, 104);
	gtk_widget_set_usize (labelSpacing, 104, 24);
	gtk_label_set_justify (GTK_LABEL (labelSpacing), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelSpacing), 0, 0.5);

	labelAfter = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_LabelAfter));
	gtk_widget_ref (labelAfter);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelAfter", labelAfter,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelAfter);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelAfter, 16, 152);
	gtk_widget_set_uposition (labelAfter, 16, 152);
	gtk_widget_set_usize (labelAfter, 80, 24);
	gtk_label_set_justify (GTK_LABEL (labelAfter), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelAfter), 0, 0.5);

	labelLineSpacing = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_LabelLineSpacing));
	gtk_widget_ref (labelLineSpacing);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelLineSpacing", labelLineSpacing,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelLineSpacing);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelLineSpacing, 216, 128);
	gtk_widget_set_uposition (labelLineSpacing, 216, 128);
	gtk_widget_set_usize (labelLineSpacing, 88, 24);
	gtk_label_set_justify (GTK_LABEL (labelLineSpacing), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelLineSpacing), 7.45058e-09, 0.5);

	labelAt = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_LabelAt));
	gtk_widget_ref (labelAt);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelAt", labelAt,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelAt);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelAt, 312, 128);
	gtk_widget_set_uposition (labelAt, 312, 128);
	gtk_widget_set_usize (labelAt, 88, 24);
	gtk_label_set_justify (GTK_LABEL (labelAt), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelAt), 7.45058e-09, 0.5);

	labelPreview = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_LabelPreview));
	gtk_widget_ref (labelPreview);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelPreview", labelPreview,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelPreview);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelPreview, 8, 176);
	gtk_widget_set_uposition (labelPreview, 8, 176);
	gtk_widget_set_usize (labelPreview, 104, 24);
	gtk_label_set_justify (GTK_LABEL (labelPreview), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelPreview), 0, 0.5);

	hseparator4 = gtk_hseparator_new ();
	gtk_widget_ref (hseparator4);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "hseparator4", hseparator4,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hseparator4);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), hseparator4, 64, 176);
	gtk_widget_set_uposition (hseparator4, 64, 176);
	gtk_widget_set_usize (hseparator4, 344, 24);

	hseparator1 = gtk_hseparator_new ();
	gtk_widget_ref (hseparator1);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "hseparator1", hseparator1,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hseparator1);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), hseparator1, 80, 32);
	gtk_widget_set_uposition (hseparator1, 80, 32);
	gtk_widget_set_usize (hseparator1, 328, 24);

	labelBefore = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_LabelBefore));
	gtk_widget_ref (labelBefore);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelBefore", labelBefore,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelBefore);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelBefore, 16, 128);
	gtk_widget_set_uposition (labelBefore, 16, 128);
	gtk_widget_set_usize (labelBefore, 80, 24);
	gtk_label_set_justify (GTK_LABEL (labelBefore), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelBefore), 0, 0.5);

	labelIndents = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_TabLabelIndentsAndSpacing));
	gtk_widget_ref (labelIndents);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelIndents", labelIndents,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelIndents);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (tabMain), gtk_notebook_get_nth_page (GTK_NOTEBOOK (tabMain), 0), labelIndents);

	fixedBreaks = gtk_fixed_new ();
	gtk_widget_ref (fixedBreaks);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "fixedBreaks", fixedBreaks,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (fixedBreaks);
	gtk_container_add (GTK_CONTAINER (tabMain), fixedBreaks);

	labelPagination = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_LabelPagination));
	gtk_widget_ref (labelPagination);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelPagination", labelPagination,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelPagination);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), labelPagination, 8, 8);
	gtk_widget_set_uposition (labelPagination, 8, 8);
	gtk_widget_set_usize (labelPagination, 104, 24);
	gtk_label_set_justify (GTK_LABEL (labelPagination), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelPagination), 0, 0.5);

	hseparator5 = gtk_hseparator_new ();
	gtk_widget_ref (hseparator5);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "hseparator5", hseparator5,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hseparator5);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), hseparator5, 72, 8);
	gtk_widget_set_uposition (hseparator5, 72, 8);
	gtk_widget_set_usize (hseparator5, 328, 24);

	hseparator7 = gtk_hseparator_new ();
	gtk_widget_ref (hseparator7);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "hseparator7", hseparator7,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hseparator7);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), hseparator7, 64, 176);
	gtk_widget_set_uposition (hseparator7, 64, 176);
	gtk_widget_set_usize (hseparator7, 344, 24);

	labelPreview2 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_LabelPreview));
	gtk_widget_ref (labelPreview2);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelPreview2", labelPreview2,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelPreview2);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), labelPreview2, 8, 176);
	gtk_widget_set_uposition (labelPreview2, 8, 176);
	gtk_widget_set_usize (labelPreview2, 104, 24);
	gtk_label_set_justify (GTK_LABEL (labelPreview2), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelPreview2), 0, 0.5);

	checkbuttonWidowOrphan = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_PushWidowOrphanControl));
	gtk_widget_ref (checkbuttonWidowOrphan);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "checkbuttonWidowOrphan", checkbuttonWidowOrphan,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonWidowOrphan);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), checkbuttonWidowOrphan, 16, 32);
	gtk_widget_set_uposition (checkbuttonWidowOrphan, 16, 32);
	gtk_widget_set_usize (checkbuttonWidowOrphan, 192, 24);

	checkbuttonKeepLines = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_PushKeepLinesTogether));
	gtk_widget_ref (checkbuttonKeepLines);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "checkbuttonKeepLines", checkbuttonKeepLines,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonKeepLines);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), checkbuttonKeepLines, 16, 56);
	gtk_widget_set_uposition (checkbuttonKeepLines, 16, 56);
	gtk_widget_set_usize (checkbuttonKeepLines, 192, 24);

	checkbuttonPageBreak = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_PushPageBreakBefore));
	gtk_widget_ref (checkbuttonPageBreak);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "checkbuttonPageBreak", checkbuttonPageBreak,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonPageBreak);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), checkbuttonPageBreak, 216, 56);
	gtk_widget_set_uposition (checkbuttonPageBreak, 216, 56);
	gtk_widget_set_usize (checkbuttonPageBreak, 192, 24);

	checkbuttonSuppress = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_PushSuppressLineNumbers));
	gtk_widget_ref (checkbuttonSuppress);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "checkbuttonSuppress", checkbuttonSuppress,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonSuppress);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), checkbuttonSuppress, 16, 96);
	gtk_widget_set_uposition (checkbuttonSuppress, 16, 96);
	gtk_widget_set_usize (checkbuttonSuppress, 192, 24);

	checkbuttonHyphenate = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_PushNoHyphenate));
	gtk_widget_ref (checkbuttonHyphenate);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "checkbuttonHyphenate", checkbuttonHyphenate,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonHyphenate);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), checkbuttonHyphenate, 16, 120);
	gtk_widget_set_uposition (checkbuttonHyphenate, 16, 120);
	gtk_widget_set_usize (checkbuttonHyphenate, 192, 24);

	hseparator6 = gtk_hseparator_new ();
	gtk_widget_ref (hseparator6);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "hseparator6", hseparator6,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hseparator6);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), hseparator6, 8, 80);
	gtk_widget_set_uposition (hseparator6, 8, 80);
	gtk_widget_set_usize (hseparator6, 392, 24);

	checkbuttonKeepNext = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_PushKeepWithNext));
	gtk_widget_ref (checkbuttonKeepNext);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "checkbuttonKeepNext", checkbuttonKeepNext,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonKeepNext);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), checkbuttonKeepNext, 216, 32);
	gtk_widget_set_uposition (checkbuttonKeepNext, 216, 32);
	gtk_widget_set_usize (checkbuttonKeepNext, 192, 24);

	labelBreaks = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Para_TabLabelLineAndPageBreaks));
	gtk_widget_ref (labelBreaks);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelBreaks", labelBreaks,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelBreaks);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (tabMain), gtk_notebook_get_nth_page (GTK_NOTEBOOK (tabMain), 1), labelBreaks);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_ref (hbox1);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "hbox1", hbox1,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vboxMain), hbox1, FALSE, TRUE, 0);

	hbuttonboxLeft = gtk_hbutton_box_new ();
	gtk_widget_ref (hbuttonboxLeft);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "hbuttonboxLeft", hbuttonboxLeft,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hbuttonboxLeft);
	gtk_box_pack_start (GTK_BOX (hbox1), hbuttonboxLeft, TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonboxLeft), GTK_BUTTONBOX_START);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonboxLeft), 0);
	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (hbuttonboxLeft), 0, 0);

	buttonTabs = gtk_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Para_ButtonTabs));
	gtk_widget_ref (buttonTabs);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "buttonTabs", buttonTabs,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonTabs);
	gtk_container_add (GTK_CONTAINER (hbuttonboxLeft), buttonTabs);
	GTK_WIDGET_SET_FLAGS (buttonTabs, GTK_CAN_DEFAULT);

	hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_widget_ref (hbox2);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "hbox2", hbox2,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hbox2);
	gtk_box_pack_start (GTK_BOX (hbox1), hbox2, TRUE, TRUE, 0);

	hbuttonboxRight = gtk_hbutton_box_new ();
	gtk_widget_ref (hbuttonboxRight);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "hbuttonboxRight", hbuttonboxRight,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hbuttonboxRight);
	gtk_box_pack_start (GTK_BOX (hbox2), hbuttonboxRight, TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonboxRight), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonboxRight), 0);
	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (hbuttonboxRight), 0, 0);

	buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_widget_ref (buttonOK);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "buttonOK", buttonOK,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonOK);
	gtk_container_add (GTK_CONTAINER (hbuttonboxRight), buttonOK);
	GTK_WIDGET_SET_FLAGS (buttonOK, GTK_CAN_DEFAULT);

	buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_widget_ref (buttonCancel);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "buttonCancel", buttonCancel,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonCancel);
	gtk_container_add (GTK_CONTAINER (hbuttonboxRight), buttonCancel);
	GTK_WIDGET_SET_FLAGS (buttonCancel, GTK_CAN_DEFAULT);

	// Our preview area hovers in a frame.  The frame and preview widgets are
	// drawn over the tab widgets by putting them on the fixed position widget
	// after the others.
	{
		framePreview = gtk_frame_new (NULL);
		gtk_widget_ref (framePreview);
		gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "framePreview", framePreview,
								  (GtkDestroyNotify) gtk_widget_unref);
		gtk_widget_show (framePreview);
		gtk_fixed_put (GTK_FIXED (fixedMain), framePreview, 26, 238);
		gtk_widget_set_uposition (framePreview, 26, 238);
		gtk_widget_set_usize (framePreview, 384, 96);
		gtk_container_set_border_width (GTK_CONTAINER (framePreview), 2);
		gtk_frame_set_shadow_type (GTK_FRAME (framePreview), GTK_SHADOW_NONE);

		drawingareaPreview = gtk_drawing_area_new ();
		gtk_widget_ref (drawingareaPreview);
		gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "drawingareaPreview", drawingareaPreview,
								  (GtkDestroyNotify) gtk_widget_unref);
		gtk_widget_show (drawingareaPreview);
		gtk_container_add (GTK_CONTAINER (framePreview), drawingareaPreview);
	}

	//////////////////////////////////////////////////////////////////////
	
	// the control buttons
	gtk_signal_connect(GTK_OBJECT(buttonOK),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(buttonCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   (gpointer) this);

	gtk_signal_connect(GTK_OBJECT(buttonTabs),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_tabs_clicked),
					   (gpointer) this);

	// we have to handle the changes in values for spin buttons
	// to preserve units
#if 0	
	gtk_signal_connect(GTK_OBJECT(spinbuttonLeft),
					   "changed",
					   GTK_SIGNAL_FUNC(s_spin_editable_changed),
					   (gpointer) this);
					   
	gtk_signal_connect(GTK_OBJECT(spinbuttonLeft_adj),
					   "value_changed",
					   GTK_SIGNAL_FUNC(s_spin_indent_changed),
					   (gpointer) this);
#endif
	// TODO : MORE CONNECTS FOR MORE SPINS!
	

	// the catch-alls
	gtk_signal_connect_after(GTK_OBJECT(windowParagraph),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(windowParagraph),
							 "destroy",
							 NULL,
							 NULL);

	// the expose event off the preview
	gtk_signal_connect(GTK_OBJECT(drawingareaPreview),
					   "expose_event",
					   GTK_SIGNAL_FUNC(s_preview_exposed),
					   (gpointer) this);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowParagraph;

	m_listAlignment = listAlignment;

	m_spinbuttonLeft_adj = spinbuttonLeft_adj;
	m_spinbuttonLeft = spinbuttonLeft;
	
	m_spinbuttonRight_adj = spinbuttonRight_adj;
	m_spinbuttonRight = spinbuttonRight;
	m_listSpecial = listSpecial;
	m_listSpecial_menu = listSpecial_menu;
	m_spinbuttonBy_adj = spinbuttonBy_adj;
	m_spinbuttonBy = spinbuttonBy;
	m_spinbuttonBefore_adj = spinbuttonBefore_adj;
	m_spinbuttonBefore = spinbuttonBefore;
	m_spinbuttonAfter_adj = spinbuttonAfter_adj;
	m_spinbuttonAfter = spinbuttonAfter;
	m_listLineSpacing = listLineSpacing;
	m_listLineSpacing_menu = listLineSpacing_menu;
	m_spinbuttonAt_adj = spinbuttonAt_adj;
	m_spinbuttonAt = spinbuttonAt;

	m_drawingareaPreview = drawingareaPreview;

	m_checkbuttonWidowOrphan = checkbuttonWidowOrphan;
	m_checkbuttonKeepLines = checkbuttonKeepLines;
	m_checkbuttonPageBreak = checkbuttonPageBreak;
	m_checkbuttonSuppress = checkbuttonSuppress;
	m_checkbuttonHyphenate = checkbuttonHyphenate;
	m_checkbuttonKeepNext = checkbuttonKeepNext;

	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;
	m_buttonTabs = buttonTabs;

	return windowParagraph;
}

void AP_UnixDialog_Paragraph::_populateWindowData(void)
{
#if 0
	// alignment option menu 
	UT_ASSERT(m_listAlignment);
	gtk_option_menu_set_history(GTK_OPTION_MENU(m_listAlignment),
								(gint) m_paragraphData.m_alignmentType);

	// indent and paragraph margins
	UT_ASSERT(m_spinbuttonLeft);
	gtk_entry_set_text(GTK_ENTRY(m_spinbuttonLeft),
					   (const gchar *) m_paragraphData.m_leftIndent);

	UT_ASSERT(m_spinbuttonRight);
	gtk_entry_set_text(GTK_ENTRY(m_spinbuttonRight),
					   (const gchar *) m_paragraphData.m_rightIndent);

	UT_ASSERT(m_spinbuttonBy);
	gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBy),
					   (const gchar *) m_paragraphData.m_specialIndent);

	UT_ASSERT(m_listSpecial);
	gtk_option_menu_set_history(GTK_OPTION_MENU(m_listSpecial),
								(gint) m_paragraphData.m_specialIndentType);
	
	// if m_specialIndentType is "(none)" (ParagraphDialogData::indent_NONE)
	// then the "By" spin should be disabled

	if (m_paragraphData.m_specialIndentType == ParagraphDialogData::indent_NONE)
		gtk_widget_set_sensitive(GTK_WIDGET(m_spinbuttonBy), FALSE);
	else
		gtk_widget_set_sensitive(GTK_WIDGET(m_spinbuttonBy), TRUE);
#endif	
}

void AP_UnixDialog_Paragraph::_storeWindowData(void)
{
	// store away percentage; the base class decides if it's important when
	// the caller requests the percent
/*	m_zoomPercent = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_spinPercent)); */
}
