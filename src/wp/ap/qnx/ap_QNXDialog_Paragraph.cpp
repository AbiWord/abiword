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
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for QNX dialogs,
// like centering them, measuring them, etc.
#include "gr_QNXGraphics.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Dialog_Id.h"

#include "ap_Strings.h"

#include "ap_Preview_Paragraph.h"
#include "ap_QNXDialog_Paragraph.h"

/*****************************************************************/

#define WIDGET_MENU_PARENT_ID_TAG	"parentmenu"
#define WIDGET_MENU_VALUE_TAG		"menuvalue"
#define WIDGET_DIALOG_TAG 			"dialog"
#define WIDGET_ID_TAG				"id"

/*****************************************************************/

XAP_Dialog * AP_QNXDialog_Paragraph::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	AP_QNXDialog_Paragraph * p = new AP_QNXDialog_Paragraph(pFactory,id);
	return p;
}

AP_QNXDialog_Paragraph::AP_QNXDialog_Paragraph(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_Paragraph(pDlgFactory,id)
{
	m_unixGraphics = NULL;
	m_bEditChanged = UT_FALSE;
}

AP_QNXDialog_Paragraph::~AP_QNXDialog_Paragraph(void)
{
	DELETEP(m_unixGraphics);
}

/*****************************************************************/
/* These are static callbacks for dialog widgets                 */
/*****************************************************************/

static void s_ok_clicked(PtWidget_t * widget, AP_QNXDialog_Paragraph * dlg)
{ UT_ASSERT(widget && dlg); dlg->event_OK(); }

static void s_cancel_clicked(PtWidget_t * widget, AP_QNXDialog_Paragraph * dlg)
{ UT_ASSERT(widget && dlg); dlg->event_Cancel(); }

static void s_tabs_clicked(PtWidget_t * widget, AP_QNXDialog_Paragraph * dlg)
{ UT_ASSERT(widget && dlg);	dlg->event_Tabs(); }

static void s_delete_clicked(PtWidget_t * /* widget */,
							 int pointer /* data */,
							 AP_QNXDialog_Paragraph * dlg)
{ UT_ASSERT(dlg); dlg->event_WindowDelete(); }

static int s_spin_focus_out(PtWidget_t * widget,
							 void * /* event */,
							 AP_QNXDialog_Paragraph * dlg)
{
	dlg->event_SpinFocusOut(widget);
	
	// do NOT let GTK do its own update (which would erase the text we just
	// put in the entry area
	return UT_FALSE;
}

static void s_spin_changed(PtWidget_t * widget,
						   AP_QNXDialog_Paragraph * dlg)
{
	// notify the dialog that an edit has changed
	dlg->event_SpinChanged(widget);
}

static void s_menu_item_activate(PtWidget_t * widget, AP_QNXDialog_Paragraph * dlg)
{
	UT_ASSERT(widget && dlg);
	
	dlg->event_MenuChanged(widget);
}

static void s_check_toggled(PtWidget_t * widget, AP_QNXDialog_Paragraph * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_CheckToggled(widget);
}

static int s_preview_exposed(PtWidget_t * /* widget */,
							  void * /* pExposeEvent */,
							  AP_QNXDialog_Paragraph * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_PreviewAreaExposed();
	return UT_FALSE;
}

/*****************************************************************/

void AP_QNXDialog_Paragraph::runModal(XAP_Frame * pFrame)
{
#if 0
	m_pFrame = pFrame;
	
	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Populate the window's data items
	_populateWindowData();

	// Attach signals (after data settings, so we don't trigger
	// updates yet)
	_connectCallbackSignals();

	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(pQNXFrame);
	
	// Get the GtkWindow of the parent frame
	PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
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
		XAP_QNXApp * unixapp = static_cast<XAP_QNXApp *> (m_pApp);
		UT_ASSERT(unixapp);

		UT_ASSERT(m_drawingareaPreview && m_drawingareaPreview->window);

		// make a new QNX GC
		m_unixGraphics = new GR_QNXGraphics(m_drawingareaPreview->window, unixapp->getFontManager());
		
		// let the widget materialize
		_createPreviewFromGC(m_unixGraphics,
							 (UT_uint32) m_drawingareaPreview->allocation.width,
							 (UT_uint32) m_drawingareaPreview->allocation.height);
	}

	// sync all controls once to get started
	// HACK: the first arg gets ignored
	_syncControls(id_MENU_ALIGNMENT, UT_TRUE);

	// Run into the GTK event loop for this window.
	gtk_main();

	gtk_widget_destroy(mainWindow);
#endif
}

/*****************************************************************/

void AP_QNXDialog_Paragraph::event_OK(void)
{
#if 0
	m_answer = AP_Dialog_Paragraph::a_OK;
	gtk_main_quit();
#endif
}

void AP_QNXDialog_Paragraph::event_Cancel(void)
{
#if 0
	m_answer = AP_Dialog_Paragraph::a_CANCEL;
	gtk_main_quit();
#endif
}

void AP_QNXDialog_Paragraph::event_Tabs(void)
{
#if 0
	m_answer = AP_Dialog_Paragraph::a_TABS;
	gtk_main_quit();
#endif
}

void AP_QNXDialog_Paragraph::event_WindowDelete(void)
{
#if 0
	m_answer = AP_Dialog_Paragraph::a_CANCEL;	
	gtk_main_quit();
#endif
}

void AP_QNXDialog_Paragraph::event_MenuChanged(PtWidget_t * widget)
{
#if 0
	UT_ASSERT(widget);

	tControl id = (tControl) gtk_object_get_data(GTK_OBJECT(widget),
												 WIDGET_MENU_PARENT_ID_TAG);

	UT_uint32 value = (UT_uint32) gtk_object_get_data(GTK_OBJECT(widget),
													  WIDGET_MENU_VALUE_TAG);

	_setMenuItemValue(id, value);
#endif
}

void AP_QNXDialog_Paragraph::event_SpinIncrement(PtWidget_t * widget)
{
	UT_ASSERT(widget);
}

void AP_QNXDialog_Paragraph::event_SpinDecrement(PtWidget_t * widget)
{
	UT_ASSERT(widget);
}

void AP_QNXDialog_Paragraph::event_SpinFocusOut(PtWidget_t * widget)
{
#if 0
	tControl id = (tControl) gtk_object_get_data(GTK_OBJECT(widget),
												 WIDGET_ID_TAG);

	if (m_bEditChanged)
	{
		// this function will massage the contents for proper
		// formatting for spinbuttons that need it.  for example,
		// line spacing can't be negative.
		_setSpinItemValue(id, (const XML_Char *)
						  gtk_entry_get_text(GTK_ENTRY(widget)));

		// to ensure the massaged value is reflected back up
		// to the screen, we repaint from the member variable
		_syncControls(id);
		
		m_bEditChanged = UT_FALSE;
	}
#endif
}

void AP_QNXDialog_Paragraph::event_SpinChanged(PtWidget_t * widget)
{
#if 0
	tControl id = (tControl) gtk_object_get_data(GTK_OBJECT(widget),
												 WIDGET_ID_TAG);
	m_bEditChanged = UT_TRUE;
#endif
}
	   
void AP_QNXDialog_Paragraph::event_CheckToggled(PtWidget_t * widget)
{
#if 0
	UT_ASSERT(widget);

	tControl id = (tControl) gtk_object_get_data(GTK_OBJECT(widget),
												 WIDGET_ID_TAG);

	gboolean state = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(widget)));

	tCheckState cs;

	// TODO : handle tri-state boxes !!!
	if (state == TRUE)
		cs = check_TRUE;
	else
		cs = check_FALSE;
	
	_setCheckItemValue(id, cs);
#endif
}

void AP_QNXDialog_Paragraph::event_PreviewAreaExposed(void)
{
	if (m_paragraphPreview)
		m_paragraphPreview->draw();
}

/*****************************************************************/

#if 0
PtWidget_t * AP_QNXDialog_Paragraph::_constructWindow(void)
{
	// grab the string set
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	PtWidget_t * windowParagraph;
	PtWidget_t * vboxMain;
	PtWidget_t * fixedMain;
	PtWidget_t * tabMain;
	PtWidget_t * fixedSpacing;
	PtWidget_t * listAlignment;
	PtWidget_t * listAlignment_menu;
	PtWidget_t * glade_menuitem;
	PtWidget_t * spinbuttonLeft;
	PtWidget_t * spinbuttonRight;
	PtWidget_t * listSpecial;
	PtWidget_t * listSpecial_menu;
	PtWidget_t * spinbuttonBy;
	PtWidget_t * spinbuttonBefore;
	PtWidget_t * spinbuttonAfter;
	PtWidget_t * listLineSpacing;
	PtWidget_t * listLineSpacing_menu;
	PtWidget_t * spinbuttonAt;
	PtWidget_t * labelAlignment;
	PtWidget_t * labelBy;
	PtWidget_t * labelIndentation;
	PtWidget_t * labelLeft;
	PtWidget_t * labelRight;
	PtWidget_t * labelSpecial;
	PtWidget_t * hseparator3;
	PtWidget_t * labelSpacing;
	PtWidget_t * labelAfter;
	PtWidget_t * labelLineSpacing;
	PtWidget_t * labelAt;
	PtWidget_t * labelPreview;

	PtWidget_t * framePreview;
	PtWidget_t * drawingareaPreview;

	PtWidget_t * hseparator4;
	PtWidget_t * hseparator1;
	PtWidget_t * labelBefore;
	PtWidget_t * labelIndents;
	PtWidget_t * fixedBreaks;
	PtWidget_t * labelPagination;
	PtWidget_t * hseparator5;
	PtWidget_t * hseparator7;
	PtWidget_t * labelPreview2;
	PtWidget_t * checkbuttonWidowOrphan;
	PtWidget_t * checkbuttonKeepLines;
	PtWidget_t * checkbuttonPageBreak;
	PtWidget_t * checkbuttonSuppress;
	PtWidget_t * checkbuttonHyphenate;
	PtWidget_t * hseparator6;
	PtWidget_t * checkbuttonKeepNext;
	PtWidget_t * labelBreaks;
	PtWidget_t * hbox1;
	PtWidget_t * hbuttonboxLeft;
	PtWidget_t * buttonTabs;
	PtWidget_t * hbox2;
	PtWidget_t * hbuttonboxRight;
	PtWidget_t * buttonOK;
	PtWidget_t * buttonCancel;

	XML_Char * unixstr = NULL;
	
	windowParagraph = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_object_set_data (GTK_OBJECT (windowParagraph), "windowParagraph", windowParagraph);
	gtk_widget_set_usize (windowParagraph, 441, -2);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_ParaTitle));
	gtk_window_set_title (GTK_WINDOW (windowParagraph), unixstr);
	FREEP(unixstr);
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
	/**/ gtk_object_set_data(GTK_OBJECT(listAlignment), WIDGET_ID_TAG, (gpointer) id_MENU_ALIGNMENT);
	gtk_widget_show (listAlignment);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), listAlignment, 104, 8);
	gtk_widget_set_uposition (listAlignment, 104, 8);
	gtk_widget_set_usize (listAlignment, 88, 24);
	listAlignment_menu = gtk_menu_new ();
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignLeft));
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemLeft = glade_menuitem;
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemLeft), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_ALIGNMENT);
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemLeft), WIDGET_MENU_VALUE_TAG, (gpointer) align_LEFT);	
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listAlignment_menu), glade_menuitem);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignCentered));
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemCentered = glade_menuitem;
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemCentered), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_ALIGNMENT);
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemCentered), WIDGET_MENU_VALUE_TAG, (gpointer) align_CENTERED);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listAlignment_menu), glade_menuitem);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignRight));
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemRight = glade_menuitem;
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemRight), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_ALIGNMENT);
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemRight), WIDGET_MENU_VALUE_TAG, (gpointer) align_RIGHT);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listAlignment_menu), glade_menuitem);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignJustified));
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemJustified = glade_menuitem;
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemJustified), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_ALIGNMENT);
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemJustified), WIDGET_MENU_VALUE_TAG, (gpointer) align_JUSTIFIED);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listAlignment_menu), glade_menuitem);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (listAlignment), listAlignment_menu);

//	spinbuttonLeft_adj = gtk_adjustment_new (0, 0, 100, 0.1, 10, 10);
//	spinbuttonLeft = gtk_spin_button_new (NULL, 1, 1);
	spinbuttonLeft = gtk_entry_new();
	gtk_widget_ref (spinbuttonLeft);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "spinbuttonLeft", spinbuttonLeft,
							  (GtkDestroyNotify) gtk_widget_unref);
	/**/ gtk_object_set_data(GTK_OBJECT(spinbuttonLeft), WIDGET_ID_TAG, (gpointer) id_SPIN_LEFT_INDENT);
	gtk_widget_show (spinbuttonLeft);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), spinbuttonLeft, 104, 56);
	gtk_widget_set_uposition (spinbuttonLeft, 104, 56);
	gtk_widget_set_usize (spinbuttonLeft, 88, 24);
	
//	spinbuttonRight_adj = gtk_adjustment_new (0, 0, 100, 0.1, 10, 10);
//	spinbuttonRight = gtk_spin_button_new (NULL, 1, 1);
	spinbuttonRight = gtk_entry_new();
	gtk_widget_ref (spinbuttonRight);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "spinbuttonRight", spinbuttonRight,
							  (GtkDestroyNotify) gtk_widget_unref);
	/**/ gtk_object_set_data(GTK_OBJECT(spinbuttonRight), WIDGET_ID_TAG, (gpointer) id_SPIN_RIGHT_INDENT);
	gtk_widget_show (spinbuttonRight);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), spinbuttonRight, 104, 80);
	gtk_widget_set_uposition (spinbuttonRight, 104, 80);
	gtk_widget_set_usize (spinbuttonRight, 88, 24);

	listSpecial = gtk_option_menu_new ();
	gtk_widget_ref (listSpecial);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "listSpecial", listSpecial,
							  (GtkDestroyNotify) gtk_widget_unref);
	/**/ gtk_object_set_data(GTK_OBJECT(listSpecial), WIDGET_ID_TAG, (gpointer) id_MENU_SPECIAL_INDENT);
	gtk_widget_show (listSpecial);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), listSpecial, 216, 80);
	gtk_widget_set_uposition (listSpecial, 216, 80);
	gtk_widget_set_usize (listSpecial, 88, 24);
	listSpecial_menu = gtk_menu_new ();
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpecialNone));
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemNone = glade_menuitem;
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemNone), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_INDENT);
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemNone), WIDGET_MENU_VALUE_TAG, (gpointer) indent_NONE);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listSpecial_menu), glade_menuitem);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpecialFirstLine));
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemFirstLine = glade_menuitem;
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemFirstLine), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_INDENT);
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemFirstLine), WIDGET_MENU_VALUE_TAG, (gpointer) indent_FIRSTLINE);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listSpecial_menu), glade_menuitem);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpecialHanging));
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemHanging = glade_menuitem;
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemHanging), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_INDENT);
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemHanging), WIDGET_MENU_VALUE_TAG, (gpointer) indent_HANGING);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listSpecial_menu), glade_menuitem);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (listSpecial), listSpecial_menu);

//	spinbuttonBy_adj = gtk_adjustment_new (0.5, 0, 100, 0.1, 10, 10);
//	spinbuttonBy = gtk_spin_button_new (NULL, 1, 1);
	spinbuttonBy = gtk_entry_new();
	gtk_widget_ref (spinbuttonBy);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "spinbuttonBy", spinbuttonBy,
							  (GtkDestroyNotify) gtk_widget_unref);
	/**/ gtk_object_set_data(GTK_OBJECT(spinbuttonBy), WIDGET_ID_TAG, (gpointer) id_SPIN_SPECIAL_INDENT);
	gtk_widget_show (spinbuttonBy);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), spinbuttonBy, 312, 80);
	gtk_widget_set_uposition (spinbuttonBy, 312, 80);
	gtk_widget_set_usize (spinbuttonBy, 88, 24);

//	spinbuttonBefore_adj = gtk_adjustment_new (0, 0, 1500, 0.1, 10, 10);
//	spinbuttonBefore = gtk_spin_button_new (NULL, 1, 1);
	spinbuttonBefore = gtk_entry_new();
	gtk_widget_ref (spinbuttonBefore);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "spinbuttonBefore", spinbuttonBefore,
							  (GtkDestroyNotify) gtk_widget_unref);
	/**/ gtk_object_set_data(GTK_OBJECT(spinbuttonBefore), WIDGET_ID_TAG, (gpointer) id_SPIN_BEFORE_SPACING);
	gtk_widget_show (spinbuttonBefore);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), spinbuttonBefore, 104, 128);
	gtk_widget_set_uposition (spinbuttonBefore, 104, 128);
	gtk_widget_set_usize (spinbuttonBefore, 88, 24);

//	spinbuttonAfter_adj = gtk_adjustment_new (0, 0, 1500, 0.1, 10, 10);
//	spinbuttonAfter = gtk_spin_button_new (NULL, 1, 1);
	spinbuttonAfter = gtk_entry_new();
	gtk_widget_ref (spinbuttonAfter);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "spinbuttonAfter", spinbuttonAfter,
							  (GtkDestroyNotify) gtk_widget_unref);
	/**/ gtk_object_set_data(GTK_OBJECT(spinbuttonAfter), WIDGET_ID_TAG, (gpointer) id_SPIN_AFTER_SPACING);
	gtk_widget_show (spinbuttonAfter);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), spinbuttonAfter, 104, 152);
	gtk_widget_set_uposition (spinbuttonAfter, 104, 152);
	gtk_widget_set_usize (spinbuttonAfter, 88, 24);

	listLineSpacing = gtk_option_menu_new ();
	gtk_widget_ref (listLineSpacing);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "listLineSpacing", listLineSpacing,
							  (GtkDestroyNotify) gtk_widget_unref);
	/**/ gtk_object_set_data(GTK_OBJECT(listLineSpacing), WIDGET_ID_TAG, (gpointer) id_MENU_SPECIAL_SPACING);
	gtk_widget_show (listLineSpacing);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), listLineSpacing, 216, 152);
	gtk_widget_set_uposition (listLineSpacing, 216, 152);
	gtk_widget_set_usize (listLineSpacing, 88, 24);
	listLineSpacing_menu = gtk_menu_new ();
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingSingle));
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemSingle = glade_menuitem;
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemSingle), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_SPACING);
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemSingle), WIDGET_MENU_VALUE_TAG, (gpointer) spacing_SINGLE);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listLineSpacing_menu), glade_menuitem);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingHalf));
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemOneAndHalf = glade_menuitem;
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemOneAndHalf), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_SPACING);
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemOneAndHalf), WIDGET_MENU_VALUE_TAG, (gpointer) spacing_ONEANDHALF);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listLineSpacing_menu), glade_menuitem);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingDouble));
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemDouble = glade_menuitem;
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemDouble), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_SPACING);
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemDouble), WIDGET_MENU_VALUE_TAG, (gpointer) spacing_DOUBLE);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listLineSpacing_menu), glade_menuitem);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingAtLeast));
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemAtLeast = glade_menuitem;
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemAtLeast), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_SPACING);
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemAtLeast), WIDGET_MENU_VALUE_TAG, (gpointer) spacing_ATLEAST);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listLineSpacing_menu), glade_menuitem);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingExactly));
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemExactly = glade_menuitem;
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemExactly), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_SPACING);
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemExactly), WIDGET_MENU_VALUE_TAG, (gpointer) spacing_EXACTLY);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listLineSpacing_menu), glade_menuitem);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingMultiple));
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemMultiple = glade_menuitem;
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemMultiple), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_SPACING);
	/**/ gtk_object_set_data(GTK_OBJECT(m_menuitemMultiple), WIDGET_MENU_VALUE_TAG, (gpointer) spacing_MULTIPLE);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listLineSpacing_menu), glade_menuitem);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (listLineSpacing), listLineSpacing_menu);

//	spinbuttonAt_adj = gtk_adjustment_new (0.5, 0, 100, 0.1, 10, 10);
//	spinbuttonAt = gtk_spin_button_new (NULL, 1, 1);
	spinbuttonAt = gtk_entry_new();
	gtk_widget_ref (spinbuttonAt);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "spinbuttonAt", spinbuttonAt,
							  (GtkDestroyNotify) gtk_widget_unref);
	/**/ gtk_object_set_data(GTK_OBJECT(spinbuttonAt), WIDGET_ID_TAG, (gpointer) id_SPIN_SPECIAL_SPACING);
	gtk_widget_show (spinbuttonAt);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), spinbuttonAt, 312, 152);
	gtk_widget_set_uposition (spinbuttonAt, 312, 152);
	gtk_widget_set_usize (spinbuttonAt, 88, 24);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelAlignment));
	labelAlignment = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (labelAlignment);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelAlignment", labelAlignment,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelAlignment);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelAlignment, 16, 8);
	gtk_widget_set_uposition (labelAlignment, 16, 8);
	gtk_widget_set_usize (labelAlignment, 80, 24);
	gtk_label_set_justify (GTK_LABEL (labelAlignment), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelAlignment), 7.45058e-09, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelBy));
	labelBy = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (labelBy);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelBy", labelBy,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelBy);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelBy, 312, 56);
	gtk_widget_set_uposition (labelBy, 312, 56);
	gtk_widget_set_usize (labelBy, 88, 24);
	gtk_label_set_justify (GTK_LABEL (labelBy), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelBy), 7.45058e-09, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelIndentation));
	labelIndentation = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (labelIndentation);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelIndentation", labelIndentation,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelIndentation);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelIndentation, 8, 32);
	gtk_widget_set_uposition (labelIndentation, 8, 32);
	gtk_widget_set_usize (labelIndentation, 104, 24);
	gtk_label_set_justify (GTK_LABEL (labelIndentation), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelIndentation), 0, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelLeft));
	labelLeft = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (labelLeft);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelLeft", labelLeft,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelLeft);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelLeft, 16, 56);
	gtk_widget_set_uposition (labelLeft, 16, 56);
	gtk_widget_set_usize (labelLeft, 80, 24);
	gtk_label_set_justify (GTK_LABEL (labelLeft), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelLeft), 0, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelRight));
	labelRight = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (labelRight);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelRight", labelRight,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelRight);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelRight, 16, 80);
	gtk_widget_set_uposition (labelRight, 16, 80);
	gtk_widget_set_usize (labelRight, 80, 24);
	gtk_label_set_justify (GTK_LABEL (labelRight), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelRight), 0, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelSpecial));
	labelSpecial = gtk_label_new (unixstr);
	FREEP(unixstr);
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

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelSpacing));
	labelSpacing = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (labelSpacing);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelSpacing", labelSpacing,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelSpacing);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelSpacing, 8, 104);
	gtk_widget_set_uposition (labelSpacing, 8, 104);
	gtk_widget_set_usize (labelSpacing, 104, 24);
	gtk_label_set_justify (GTK_LABEL (labelSpacing), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelSpacing), 0, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelAfter));
	labelAfter = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (labelAfter);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelAfter", labelAfter,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelAfter);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelAfter, 16, 152);
	gtk_widget_set_uposition (labelAfter, 16, 152);
	gtk_widget_set_usize (labelAfter, 80, 24);
	gtk_label_set_justify (GTK_LABEL (labelAfter), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelAfter), 0, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelLineSpacing));
	labelLineSpacing = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (labelLineSpacing);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelLineSpacing", labelLineSpacing,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelLineSpacing);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelLineSpacing, 216, 128);
	gtk_widget_set_uposition (labelLineSpacing, 216, 128);
	gtk_widget_set_usize (labelLineSpacing, 88, 24);
	gtk_label_set_justify (GTK_LABEL (labelLineSpacing), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelLineSpacing), 7.45058e-09, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelAt));
	labelAt = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (labelAt);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelAt", labelAt,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelAt);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelAt, 312, 128);
	gtk_widget_set_uposition (labelAt, 312, 128);
	gtk_widget_set_usize (labelAt, 88, 24);
	gtk_label_set_justify (GTK_LABEL (labelAt), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelAt), 7.45058e-09, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelPreview));
	labelPreview = gtk_label_new (unixstr);
	FREEP(unixstr);
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

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelBefore));
	labelBefore = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (labelBefore);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelBefore", labelBefore,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelBefore);
	gtk_fixed_put (GTK_FIXED (fixedSpacing), labelBefore, 16, 128);
	gtk_widget_set_uposition (labelBefore, 16, 128);
	gtk_widget_set_usize (labelBefore, 80, 24);
	gtk_label_set_justify (GTK_LABEL (labelBefore), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelBefore), 0, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_TabLabelIndentsAndSpacing));
	labelIndents = gtk_label_new (unixstr);
	FREEP(unixstr);
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

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelPagination));
	labelPagination = gtk_label_new (unixstr);
	FREEP(unixstr);
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

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelPreview));
	labelPreview2 = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (labelPreview2);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "labelPreview2", labelPreview2,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelPreview2);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), labelPreview2, 8, 176);
	gtk_widget_set_uposition (labelPreview2, 8, 176);
	gtk_widget_set_usize (labelPreview2, 104, 24);
	gtk_label_set_justify (GTK_LABEL (labelPreview2), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelPreview2), 0, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_PushWidowOrphanControl));
	checkbuttonWidowOrphan = gtk_check_button_new_with_label (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (checkbuttonWidowOrphan);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "checkbuttonWidowOrphan", checkbuttonWidowOrphan,
							  (GtkDestroyNotify) gtk_widget_unref);
	/**/ gtk_object_set_data(GTK_OBJECT(checkbuttonWidowOrphan), WIDGET_ID_TAG, (gpointer) id_CHECK_WIDOW_ORPHAN);
	gtk_widget_show (checkbuttonWidowOrphan);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), checkbuttonWidowOrphan, 16, 32);
	gtk_widget_set_uposition (checkbuttonWidowOrphan, 16, 32);
	gtk_widget_set_usize (checkbuttonWidowOrphan, 192, 24);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_PushKeepLinesTogether));
	checkbuttonKeepLines = gtk_check_button_new_with_label (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (checkbuttonKeepLines);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "checkbuttonKeepLines", checkbuttonKeepLines,
							  (GtkDestroyNotify) gtk_widget_unref);
	/**/ gtk_object_set_data(GTK_OBJECT(checkbuttonKeepLines), WIDGET_ID_TAG, (gpointer) id_CHECK_KEEP_LINES);
	gtk_widget_show (checkbuttonKeepLines);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), checkbuttonKeepLines, 16, 56);
	gtk_widget_set_uposition (checkbuttonKeepLines, 16, 56);
	gtk_widget_set_usize (checkbuttonKeepLines, 192, 24);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_PushPageBreakBefore));
	checkbuttonPageBreak = gtk_check_button_new_with_label (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (checkbuttonPageBreak);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "checkbuttonPageBreak", checkbuttonPageBreak,
							  (GtkDestroyNotify) gtk_widget_unref);
	/**/ gtk_object_set_data(GTK_OBJECT(checkbuttonPageBreak), WIDGET_ID_TAG, (gpointer) id_CHECK_PAGE_BREAK);
	gtk_widget_show (checkbuttonPageBreak);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), checkbuttonPageBreak, 216, 56);
	gtk_widget_set_uposition (checkbuttonPageBreak, 216, 56);
	gtk_widget_set_usize (checkbuttonPageBreak, 192, 24);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_PushSuppressLineNumbers));
	checkbuttonSuppress = gtk_check_button_new_with_label (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (checkbuttonSuppress);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "checkbuttonSuppress", checkbuttonSuppress,
							  (GtkDestroyNotify) gtk_widget_unref);
	/**/ gtk_object_set_data(GTK_OBJECT(checkbuttonSuppress), WIDGET_ID_TAG, (gpointer) id_CHECK_SUPPRESS);
	gtk_widget_show (checkbuttonSuppress);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), checkbuttonSuppress, 16, 96);
	gtk_widget_set_uposition (checkbuttonSuppress, 16, 96);
	gtk_widget_set_usize (checkbuttonSuppress, 192, 24);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_PushNoHyphenate));
	checkbuttonHyphenate = gtk_check_button_new_with_label (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (checkbuttonHyphenate);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "checkbuttonHyphenate", checkbuttonHyphenate,
							  (GtkDestroyNotify) gtk_widget_unref);
	/**/ gtk_object_set_data(GTK_OBJECT(checkbuttonHyphenate), WIDGET_ID_TAG, (gpointer) id_CHECK_NO_HYPHENATE);
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

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_PushKeepWithNext));
	checkbuttonKeepNext = gtk_check_button_new_with_label (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (checkbuttonKeepNext);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "checkbuttonKeepNext", checkbuttonKeepNext,
							  (GtkDestroyNotify) gtk_widget_unref);
	/**/ gtk_object_set_data(GTK_OBJECT(checkbuttonKeepNext), WIDGET_ID_TAG, (gpointer) id_CHECK_KEEP_NEXT);
	gtk_widget_show (checkbuttonKeepNext);
	gtk_fixed_put (GTK_FIXED (fixedBreaks), checkbuttonKeepNext, 216, 32);
	gtk_widget_set_uposition (checkbuttonKeepNext, 216, 32);
	gtk_widget_set_usize (checkbuttonKeepNext, 192, 24);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_TabLabelLineAndPageBreaks));
	labelBreaks = gtk_label_new (unixstr);
	FREEP(unixstr);
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

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_ButtonTabs));
	buttonTabs = gtk_button_new_with_label (unixstr);
	FREEP(unixstr);
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

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_OK));
	buttonOK = gtk_button_new_with_label (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (buttonOK);
	gtk_object_set_data_full (GTK_OBJECT (windowParagraph), "buttonOK", buttonOK,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonOK);
	gtk_container_add (GTK_CONTAINER (hbuttonboxRight), buttonOK);
	GTK_WIDGET_SET_FLAGS (buttonOK, GTK_CAN_DEFAULT);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	buttonCancel = gtk_button_new_with_label (unixstr);
	FREEP(unixstr);
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

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowParagraph;

	m_listAlignment = listAlignment;

//	m_spinbuttonLeft_adj = spinbuttonLeft_adj;
	m_spinbuttonLeft = spinbuttonLeft;
	
//	m_spinbuttonRight_adj = spinbuttonRight_adj;
	m_spinbuttonRight = spinbuttonRight;
	m_listSpecial = listSpecial;
	m_listSpecial_menu = listSpecial_menu;
//	m_spinbuttonBy_adj = spinbuttonBy_adj;
	m_spinbuttonBy = spinbuttonBy;
//	m_spinbuttonBefore_adj = spinbuttonBefore_adj;
	m_spinbuttonBefore = spinbuttonBefore;
//	m_spinbuttonAfter_adj = spinbuttonAfter_adj;
	m_spinbuttonAfter = spinbuttonAfter;
	m_listLineSpacing = listLineSpacing;
	m_listLineSpacing_menu = listLineSpacing_menu;
//	m_spinbuttonAt_adj = spinbuttonAt_adj;
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

#define CONNECT_SPIN_SIGNAL_CHANGED(w)				\
        do {												\
	        gtk_signal_connect(GTK_OBJECT(w), "changed",	\
                GTK_SIGNAL_FUNC(s_spin_changed),			\
                (gpointer) this);							\
        } while (0)

#define CONNECT_SPIN_SIGNAL_FOCUS_OUT(w)			\
        do {												\
	        gtk_signal_connect(GTK_OBJECT(w), "focus_out_event",	\
                GTK_SIGNAL_FUNC(s_spin_focus_out),			\
                (gpointer) this);							\
        } while (0)

#define CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(w)				\
        do {												\
	        gtk_signal_connect(GTK_OBJECT(w), "activate",	\
                GTK_SIGNAL_FUNC(s_menu_item_activate),		\
                (gpointer) this);							\
        } while (0)

void AP_QNXDialog_Paragraph::_connectCallbackSignals(void)
{
	// the control buttons
	gtk_signal_connect(GTK_OBJECT(m_buttonOK),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(m_buttonCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   (gpointer) this);

	gtk_signal_connect(GTK_OBJECT(m_buttonTabs),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_tabs_clicked),
					   (gpointer) this);

	// we have to handle the changes in values for spin buttons
	// to preserve units
	CONNECT_SPIN_SIGNAL_CHANGED(m_spinbuttonLeft);
	CONNECT_SPIN_SIGNAL_CHANGED(m_spinbuttonRight);
	CONNECT_SPIN_SIGNAL_CHANGED(m_spinbuttonBy);
	CONNECT_SPIN_SIGNAL_CHANGED(m_spinbuttonBefore);
	CONNECT_SPIN_SIGNAL_CHANGED(m_spinbuttonAfter);	
	CONNECT_SPIN_SIGNAL_CHANGED(m_spinbuttonAt);
	
	CONNECT_SPIN_SIGNAL_FOCUS_OUT(m_spinbuttonLeft);
	CONNECT_SPIN_SIGNAL_FOCUS_OUT(m_spinbuttonRight);
	CONNECT_SPIN_SIGNAL_FOCUS_OUT(m_spinbuttonBy);
	CONNECT_SPIN_SIGNAL_FOCUS_OUT(m_spinbuttonBefore);
	CONNECT_SPIN_SIGNAL_FOCUS_OUT(m_spinbuttonAfter);	
	CONNECT_SPIN_SIGNAL_FOCUS_OUT(m_spinbuttonAt);

	// connect to option menus
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemLeft);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemCentered);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemRight);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemJustified);

	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemNone);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemFirstLine);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemHanging);	
	
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemSingle);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemOneAndHalf);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemDouble);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemAtLeast);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemExactly);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemMultiple);
	
	// all the checkbuttons
	gtk_signal_connect(GTK_OBJECT(m_checkbuttonWidowOrphan), "toggled",
					   GTK_SIGNAL_FUNC(s_check_toggled), (gpointer) this);
	gtk_signal_connect(GTK_OBJECT(m_checkbuttonKeepLines), "toggled",
					   GTK_SIGNAL_FUNC(s_check_toggled), (gpointer) this);
	gtk_signal_connect(GTK_OBJECT(m_checkbuttonPageBreak), "toggled",
					   GTK_SIGNAL_FUNC(s_check_toggled), (gpointer) this);
	gtk_signal_connect(GTK_OBJECT(m_checkbuttonSuppress), "toggled",
					   GTK_SIGNAL_FUNC(s_check_toggled), (gpointer) this);
	gtk_signal_connect(GTK_OBJECT(m_checkbuttonHyphenate), "toggled",
					   GTK_SIGNAL_FUNC(s_check_toggled), (gpointer) this);
	gtk_signal_connect(GTK_OBJECT(m_checkbuttonKeepNext), "toggled",
					   GTK_SIGNAL_FUNC(s_check_toggled), (gpointer) this);
	
	// the catch-alls
	gtk_signal_connect_after(GTK_OBJECT(m_windowMain),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(m_windowMain),
							 "destroy",
							 NULL,
							 NULL);

	// the expose event off the preview
	gtk_signal_connect(GTK_OBJECT(m_drawingareaPreview),
					   "expose_event",
					   GTK_SIGNAL_FUNC(s_preview_exposed),
					   (gpointer) this);
}

void AP_QNXDialog_Paragraph::_populateWindowData(void)
{

	// alignment option menu 
	UT_ASSERT(m_listAlignment);
	gtk_option_menu_set_history(GTK_OPTION_MENU(m_listAlignment),
								(int) _getMenuItemValue(id_MENU_ALIGNMENT));

	// indent and paragraph margins
	UT_ASSERT(m_spinbuttonLeft);
	gtk_entry_set_text(GTK_ENTRY(m_spinbuttonLeft),
					   (const gchar *) _getSpinItemValue(id_SPIN_LEFT_INDENT));

	UT_ASSERT(m_spinbuttonRight);
	gtk_entry_set_text(GTK_ENTRY(m_spinbuttonRight),
					   (const gchar *) _getSpinItemValue(id_SPIN_RIGHT_INDENT));

	UT_ASSERT(m_spinbuttonBy);
	gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBy),
					   (const gchar *) _getSpinItemValue(id_SPIN_SPECIAL_INDENT));

	UT_ASSERT(m_listSpecial);
	gtk_option_menu_set_history(GTK_OPTION_MENU(m_listSpecial),
								(int) _getMenuItemValue(id_MENU_SPECIAL_INDENT));

	// spacing
	UT_ASSERT(m_spinbuttonLeft);
	gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBefore),
					   (const gchar *) _getSpinItemValue(id_SPIN_BEFORE_SPACING));

	UT_ASSERT(m_spinbuttonRight);
	gtk_entry_set_text(GTK_ENTRY(m_spinbuttonAfter),
					   (const gchar *) _getSpinItemValue(id_SPIN_AFTER_SPACING));

	UT_ASSERT(m_spinbuttonAt);
	gtk_entry_set_text(GTK_ENTRY(m_spinbuttonAt),
					   (const gchar *) _getSpinItemValue(id_SPIN_SPECIAL_SPACING));

	UT_ASSERT(m_listLineSpacing);
	gtk_option_menu_set_history(GTK_OPTION_MENU(m_listLineSpacing),
								(int) _getMenuItemValue(id_MENU_SPECIAL_SPACING));

	// set the check boxes
	// TODO : handle tri-state boxes !!!

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(m_checkbuttonWidowOrphan)),
								 (_getCheckItemValue(id_CHECK_WIDOW_ORPHAN) == check_TRUE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(m_checkbuttonKeepLines)),
								 (_getCheckItemValue(id_CHECK_KEEP_LINES) == check_TRUE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(m_checkbuttonPageBreak)),
								 (_getCheckItemValue(id_CHECK_PAGE_BREAK) == check_TRUE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(m_checkbuttonSuppress)),
								 (_getCheckItemValue(id_CHECK_SUPPRESS) == check_TRUE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(m_checkbuttonHyphenate)),
								 (_getCheckItemValue(id_CHECK_NO_HYPHENATE) == check_TRUE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(m_checkbuttonKeepNext)),
								 (_getCheckItemValue(id_CHECK_KEEP_NEXT) == check_TRUE));
}

void AP_QNXDialog_Paragraph::_syncControls(tControl changed, UT_Bool bAll /* = UT_FALSE */)
{
	// let parent sync any member variables first
	AP_Dialog_Paragraph::_syncControls(changed, bAll);

	// sync the display

	// 1.  link the "hanging indent by" combo and spinner
	if (bAll || (changed == id_SPIN_SPECIAL_INDENT))
	{
		// typing in the control can change the associated combo
		if (_getMenuItemValue(id_MENU_SPECIAL_INDENT) == indent_FIRSTLINE)
		{
			gtk_option_menu_set_history(GTK_OPTION_MENU(m_listSpecial),
										(int) _getMenuItemValue(id_MENU_SPECIAL_INDENT));
		}
	}
	if (bAll || (changed == id_MENU_SPECIAL_INDENT))
	{
		switch(_getMenuItemValue(id_MENU_SPECIAL_INDENT))
		{
		case indent_NONE:
			// clear the spin control
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBy), "");
			break;

		default:
			// set the spin control
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBy), _getSpinItemValue(id_SPIN_SPECIAL_INDENT));			
			break;
		}
	}

	// 2.  link the "line spacing at" combo and spinner

	if (bAll || (changed == id_SPIN_SPECIAL_SPACING))
	{
		// typing in the control can change the associated combo
		if (_getMenuItemValue(id_MENU_SPECIAL_SPACING) == spacing_MULTIPLE)
		{
			gtk_option_menu_set_history(GTK_OPTION_MENU(m_listLineSpacing),
										(int) _getMenuItemValue(id_MENU_SPECIAL_SPACING));
		}
	}
	if (bAll || (changed == id_MENU_SPECIAL_SPACING))
	{
		switch(_getMenuItemValue(id_MENU_SPECIAL_SPACING))
		{
		case spacing_SINGLE:
		case spacing_ONEANDHALF:
		case spacing_DOUBLE:
			// clear the spin control
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonAt), "");
			break;

		default:
			// set the spin control
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonAt), _getSpinItemValue(id_SPIN_SPECIAL_SPACING));
			break;
		}
	}

	// 3.  move results of _doSpin() back to screen

	if (!bAll)
	{
		// spin controls only sync when spun
		switch (changed)
		{
		case id_SPIN_LEFT_INDENT:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonLeft), 	_getSpinItemValue(id_SPIN_LEFT_INDENT));
		case id_SPIN_RIGHT_INDENT:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonRight), 	_getSpinItemValue(id_SPIN_RIGHT_INDENT));
		case id_SPIN_SPECIAL_INDENT:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBy), 		_getSpinItemValue(id_SPIN_SPECIAL_INDENT));
		case id_SPIN_BEFORE_SPACING:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBefore), 	_getSpinItemValue(id_SPIN_BEFORE_SPACING));
		case id_SPIN_AFTER_SPACING:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonAfter), 	_getSpinItemValue(id_SPIN_AFTER_SPACING));
		case id_SPIN_SPECIAL_SPACING:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonAt), 		_getSpinItemValue(id_SPIN_SPECIAL_SPACING));
		default:
			break;
		}
	}
}
#endif
