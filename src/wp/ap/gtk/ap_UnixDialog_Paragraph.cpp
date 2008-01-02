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

/*
 * Port to Maemo Development Platform
 * Author: INdT - Renato Araujo <renato.filho@indt.org.br>
 */

#undef GTK_DISABLE_DEPRECATED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Dialog_Id.h"
#include "ap_Strings.h"
#include "ap_EditMethods.h"

#include "ap_Preview_Paragraph.h"
#include "ap_UnixDialog_Paragraph.h"

#include "gr_UnixPangoGraphics.h"

/*****************************************************************/

#define WIDGET_MENU_PARENT_ID_TAG	"parentmenu"
#define WIDGET_MENU_VALUE_TAG		"menuvalue"
#define WIDGET_DIALOG_TAG 			"dialog"
#define WIDGET_ID_TAG				"id"

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
	m_bEditChanged = false;
}

AP_UnixDialog_Paragraph::~AP_UnixDialog_Paragraph(void)
{
	DELETEP(m_unixGraphics);
}

/*****************************************************************/
/* These are static callbacks for dialog widgets                 */
/*****************************************************************/

static gint s_spin_focus_out(GtkWidget * widget,
							 GdkEventFocus * /* event */,
							 AP_UnixDialog_Paragraph * dlg)
{
	dlg->event_SpinFocusOut(widget);

	// do NOT let GTK do its own update (which would erase the text we just
	// put in the entry area
	return FALSE;
}

static void s_spin_changed(GtkWidget * widget,
						   AP_UnixDialog_Paragraph * dlg)
{
	// notify the dialog that an edit has changed
	dlg->event_SpinChanged(widget);
}

static void s_menu_item_activate(GtkWidget * widget, AP_UnixDialog_Paragraph * dlg)
{
	UT_ASSERT(widget && dlg);

	dlg->event_MenuChanged(widget);
}

static void s_check_toggled(GtkWidget * widget, AP_UnixDialog_Paragraph * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_CheckToggled(widget);
}

static gboolean do_update(gpointer p)
{
//
// FIXME!!! Could get nasty crash if the dlg is destroyed while 
// a redraw is pending....
//
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	AP_UnixDialog_Paragraph * dlg = (AP_UnixDialog_Paragraph *) p;
	dlg->event_PreviewAreaExposed();
#endif
	return FALSE;
}

static gint s_preview_exposed(GtkWidget * /* widget */,
							  GdkEventExpose * /* pExposeEvent */,
							  AP_UnixDialog_Paragraph * dlg)
{
	UT_ASSERT(dlg);
	g_idle_add((GSourceFunc) do_update,(gpointer) dlg);
	return TRUE;
}

/*****************************************************************/

void AP_UnixDialog_Paragraph::runModal(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;

	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Populate the window's data items
	_populateWindowData();

	// Attach signals (after data settings, so we don't trigger
	// updates yet)
	_connectCallbackSignals();

	// Show the top level dialog,
	gtk_widget_show(mainWindow);

#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	// *** this is how we add the gc ***
	{
		// attach a new graphics context to the drawing area
		XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
		UT_ASSERT(unixapp);

		UT_ASSERT(m_drawingareaPreview && m_drawingareaPreview->window);

		// make a new Unix GC
		GR_UnixAllocInfo ai(m_drawingareaPreview->window);
		m_unixGraphics =
		    (GR_UnixPangoGraphics*) XAP_App::getApp()->newGraphics(ai);

		// let the widget materialize
		_createPreviewFromGC(m_unixGraphics,
							 (UT_uint32) m_drawingareaPreview->allocation.width,
							 (UT_uint32) m_drawingareaPreview->allocation.height);
	}

	// sync all controls once to get started
	// HACK: the first arg gets ignored
	_syncControls(id_MENU_ALIGNMENT, true);
#endif

	bool tabs;
	do {
		switch(abiRunModalDialog(GTK_DIALOG(mainWindow), pFrame, this, BUTTON_CANCEL, false))
		{
		case BUTTON_OK:
		  event_OK(); 
		  tabs = false;
		  break;
		case BUTTON_TABS:
		  event_Tabs ();
		  tabs = true;
		  break;
		default:
		  event_Cancel();
		  tabs = false;
		  break;
		}
	} while (tabs);
	
	abiDestroyWidget(mainWindow);
}

/*****************************************************************/

void AP_UnixDialog_Paragraph::event_OK(void)
{
	m_answer = AP_Dialog_Paragraph::a_OK;
}

void AP_UnixDialog_Paragraph::event_Cancel(void)
{
	m_answer = AP_Dialog_Paragraph::a_CANCEL;
}

void AP_UnixDialog_Paragraph::event_Tabs(void)
{
	AV_View *pView = m_pFrame->getCurrentView();
	s_doTabDlg(static_cast<FV_View*>(pView));
	m_answer = AP_Dialog_Paragraph::a_TABS;
}

void AP_UnixDialog_Paragraph::event_MenuChanged(GtkWidget * widget)
{
	UT_ASSERT(widget);

	tControl id = (tControl) GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget),
								   WIDGET_MENU_PARENT_ID_TAG));

	UT_uint32 value = (UT_uint32) GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget),
									WIDGET_MENU_VALUE_TAG));

	_setMenuItemValue(id, value);
}

void AP_UnixDialog_Paragraph::event_SpinIncrement(GtkWidget * widget)
{
	UT_ASSERT(widget);
}

void AP_UnixDialog_Paragraph::event_SpinDecrement(GtkWidget * widget)
{
	UT_ASSERT(widget);
}

void AP_UnixDialog_Paragraph::event_SpinFocusOut(GtkWidget * widget)
{
	tControl id = (tControl) GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget),
												 WIDGET_ID_TAG));

	if (m_bEditChanged)
	{
		// this function will massage the contents for proper
		// formatting for spinbuttons that need it.  for example,
		// line spacing can't be negative.
		_setSpinItemValue(id, (const gchar *)
						  gtk_entry_get_text(GTK_ENTRY(widget)));

		// to ensure the massaged value is reflected back up
		// to the screen, we repaint from the member variable
		_syncControls(id);

		m_bEditChanged = false;
	}
}

void AP_UnixDialog_Paragraph::event_SpinChanged(GtkWidget * widget)
{
	m_bEditChanged = true;
}

void AP_UnixDialog_Paragraph::event_CheckToggled(GtkWidget * widget)
{
	UT_ASSERT(widget);

	tControl id = (tControl) GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget),
												 WIDGET_ID_TAG));

	gboolean state = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(widget)));

	tCheckState cs;

	// TODO : handle tri-state boxes !!!
	if (state == TRUE)
		cs = check_TRUE;
	else
		cs = check_FALSE;

	_setCheckItemValue(id, cs);
}

void AP_UnixDialog_Paragraph::event_PreviewAreaExposed(void)
{
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	if (m_paragraphPreview)
		m_paragraphPreview->draw();
#endif	
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_Paragraph::_constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	GtkWidget * windowParagraph;
	GtkWidget * windowContents;
	GtkWidget * vboxMain;

	GtkWidget * buttonTabs;
	GtkWidget * buttonOK;
	GtkWidget * buttonCancel;

	gchar * unixstr = NULL;

	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_ParaTitle,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	windowParagraph = abiDialogNew("paragraph dialog", TRUE, unixstr);
	FREEP(unixstr);

	vboxMain = GTK_DIALOG(windowParagraph)->vbox;
	gtk_container_set_border_width (GTK_CONTAINER(vboxMain), 10);

	windowContents = _constructWindowContents(windowParagraph);
	gtk_box_pack_start (GTK_BOX (vboxMain), windowContents, FALSE, TRUE, 5);
	buttonCancel = abiAddStockButton(GTK_DIALOG(windowParagraph), GTK_STOCK_CANCEL, BUTTON_CANCEL);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_ButtonTabs,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	buttonTabs = abiAddButton (GTK_DIALOG(windowParagraph), unixstr, BUTTON_TABS);
	FREEP(unixstr);
	buttonOK = abiAddStockButton(GTK_DIALOG(windowParagraph), GTK_STOCK_OK, BUTTON_OK);

	m_windowMain = windowParagraph;

	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;
	m_buttonTabs = buttonTabs;

	return windowParagraph;
}

GtkWidget * AP_UnixDialog_Paragraph::_constructWindowContents(GtkWidget *windowMain)
{
	// grab the string set
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	GtkWidget * vboxContents;
	GtkWidget * tabMain;
	GtkWidget * boxSpacing;
	GtkWidget * hboxAlignment;
	GtkWidget * listAlignment;
	GtkWidget * listAlignment_menu;
	GtkWidget * glade_menuitem;
	GtkWidget * spinbuttonLeft;
	GtkWidget * spinbuttonRight;
	GtkWidget * listSpecial;
	GtkWidget * listSpecial_menu;
	GtkWidget * spinbuttonBy;
	GtkWidget * spinbuttonBefore;
	GtkWidget * spinbuttonAfter;
	GtkWidget * listLineSpacing;
	GtkWidget * listLineSpacing_menu;
	GtkWidget * spinbuttonAt;
	GtkWidget * labelAlignment;
	GtkWidget * labelBy;
	GtkWidget * hboxIndentation;
	GtkWidget * labelIndentation;
	GtkWidget * labelLeft;
	GtkWidget * labelRight;
	GtkWidget * labelSpecial;
	GtkWidget * hseparator3;
	GtkWidget * hboxSpacing;
	GtkWidget * labelSpacing;
	GtkWidget * labelAfter;
	GtkWidget * labelLineSpacing;
	GtkWidget * labelAt;

	GtkWidget * hboxPreview;
	GtkWidget * labelPreview;
	GtkWidget * hboxPreviewFrame;
	GtkWidget * framePreview;
	GtkWidget * drawingareaPreview;

	GtkWidget * hseparator4;
	GtkWidget * hseparator1;
	GtkWidget * labelBefore;
	GtkWidget * labelIndents;
	GtkWidget * boxBreaks;
	GtkWidget * hboxPagination;
	GtkWidget * labelPagination;
	GtkWidget * hseparator5;
	GtkWidget * checkbuttonWidowOrphan;
	GtkWidget * checkbuttonKeepLines;
	GtkWidget * checkbuttonPageBreak;
	GtkWidget * checkbuttonSuppress;
	GtkWidget * checkbuttonHyphenate;
	GtkWidget * hseparator6;
	GtkWidget * checkbuttonKeepNext;
	GtkWidget * labelBreaks;
	GtkWidget * checkbuttonDomDirection;

	gchar * unixstr = NULL;

	vboxContents = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vboxContents);

	tabMain = gtk_notebook_new ();
	gtk_widget_show (tabMain);
	gtk_box_pack_start (GTK_BOX (vboxContents), tabMain, FALSE, TRUE, 0);


	// "Indents and Spacing" page
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
	boxSpacing = gtk_table_new (3, 4, FALSE);
#else	
	boxSpacing = gtk_table_new (7, 4, FALSE);
#endif
	gtk_widget_show (boxSpacing);
	gtk_table_set_row_spacings (GTK_TABLE(boxSpacing), 5);
	gtk_table_set_col_spacings (GTK_TABLE(boxSpacing), 5);
	gtk_container_set_border_width (GTK_CONTAINER(boxSpacing), 5);

	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_TabLabelIndentsAndSpacing,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	labelIndents = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelIndents);

	gtk_notebook_append_page (GTK_NOTEBOOK (tabMain), boxSpacing, labelIndents);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_LabelAlignment,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	labelAlignment = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelAlignment);
	gtk_table_attach ( GTK_TABLE(boxSpacing), labelAlignment, 0,1, 0,1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (labelAlignment), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (labelAlignment), 1, 0.5);

	hboxAlignment = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hboxAlignment);
	listAlignment = gtk_option_menu_new ();
	/**/ g_object_set_data(G_OBJECT(listAlignment), WIDGET_ID_TAG, (gpointer) id_MENU_ALIGNMENT);
	gtk_widget_show (listAlignment);
	gtk_box_pack_start (GTK_BOX (hboxAlignment), listAlignment, FALSE, FALSE, 0);
	gtk_table_attach ( GTK_TABLE(boxSpacing), hboxAlignment, 1,2, 0,1,
                    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

	listAlignment_menu = gtk_menu_new ();
	
	glade_menuitem = gtk_menu_item_new_with_label(" "); // add an empty menu option to fix bug 594
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listAlignment_menu), glade_menuitem);
	
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_AlignLeft,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemLeft = glade_menuitem;
	/**/ g_object_set_data(G_OBJECT(m_menuitemLeft), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_ALIGNMENT);
	/**/ g_object_set_data(G_OBJECT(m_menuitemLeft), WIDGET_MENU_VALUE_TAG, (gpointer) align_LEFT);
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listAlignment_menu), glade_menuitem);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_AlignCentered,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemCentered = glade_menuitem;
	/**/ g_object_set_data(G_OBJECT(m_menuitemCentered), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_ALIGNMENT);
	/**/ g_object_set_data(G_OBJECT(m_menuitemCentered), WIDGET_MENU_VALUE_TAG, (gpointer) align_CENTERED);
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listAlignment_menu), glade_menuitem);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_AlignRight,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemRight = glade_menuitem;
	/**/ g_object_set_data(G_OBJECT(m_menuitemRight), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_ALIGNMENT);
	/**/ g_object_set_data(G_OBJECT(m_menuitemRight), WIDGET_MENU_VALUE_TAG, (gpointer) align_RIGHT);
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listAlignment_menu), glade_menuitem);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_AlignJustified,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemJustified = glade_menuitem;
	/**/ g_object_set_data(G_OBJECT(m_menuitemJustified), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_ALIGNMENT);
	/**/ g_object_set_data(G_OBJECT(m_menuitemJustified), WIDGET_MENU_VALUE_TAG, (gpointer) align_JUSTIFIED);
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listAlignment_menu), glade_menuitem);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (listAlignment), listAlignment_menu);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_DomDirection,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	checkbuttonDomDirection = gtk_check_button_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ g_object_set_data(G_OBJECT(checkbuttonDomDirection), WIDGET_ID_TAG, (gpointer) id_CHECK_DOMDIRECTION);
	gtk_widget_show (checkbuttonDomDirection);
	gtk_table_attach ( GTK_TABLE(boxSpacing), checkbuttonDomDirection, 3,4,0,1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0 );

	hboxIndentation = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hboxIndentation);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_LabelIndentation,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	labelIndentation = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelIndentation);
	gtk_box_pack_start (GTK_BOX (hboxIndentation), labelIndentation, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (labelIndentation), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelIndentation), 0, 0.5);

	hseparator3 = gtk_hseparator_new ();
	gtk_widget_show (hseparator3);
	gtk_box_pack_start (GTK_BOX (hboxIndentation), hseparator3, TRUE, TRUE, 0);
	gtk_table_attach ( GTK_TABLE(boxSpacing), hboxIndentation, 0,4, 1,2,
                    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_LabelLeft,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	labelLeft = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelLeft);
	gtk_table_attach ( GTK_TABLE(boxSpacing), labelLeft, 0,1, 2,3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (labelLeft), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (labelLeft), 1, 0.5);


//	spinbuttonLeft_adj = gtk_adjustment_new (0, 0, 100, 0.1, 10, 10);
//	spinbuttonLeft = gtk_spin_button_new (NULL, 1, 1);
	spinbuttonLeft = gtk_entry_new();
	gtk_widget_ref (spinbuttonLeft);
	g_object_set_data_full (G_OBJECT (windowMain), "spinbuttonLeft", spinbuttonLeft,
							  (GtkDestroyNotify) gtk_widget_unref);
	/**/ g_object_set_data(G_OBJECT(spinbuttonLeft), WIDGET_ID_TAG, (gpointer) id_SPIN_LEFT_INDENT);
	gtk_widget_show (spinbuttonLeft);
	gtk_table_attach ( GTK_TABLE(boxSpacing), spinbuttonLeft, 1,2, 2,3,
                    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_LabelRight,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	labelRight = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelRight);
	gtk_table_attach ( GTK_TABLE(boxSpacing), labelRight, 0,1, 3,4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (labelRight), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (labelRight), 1, 0.5);

//	spinbuttonRight_adj = gtk_adjustment_new (0, 0, 100, 0.1, 10, 10);
//	spinbuttonRight = gtk_spin_button_new (NULL, 1, 1);
	spinbuttonRight = gtk_entry_new();
	/**/ g_object_set_data(G_OBJECT(spinbuttonRight), WIDGET_ID_TAG, (gpointer) id_SPIN_RIGHT_INDENT);
	gtk_widget_show (spinbuttonRight);
	gtk_table_attach ( GTK_TABLE(boxSpacing), spinbuttonRight, 1,2, 3,4,
                    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_LabelSpecial,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	labelSpecial = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelSpecial);
	gtk_table_attach ( GTK_TABLE(boxSpacing), labelSpecial, 2,3, 2,3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
	gtk_label_set_justify (GTK_LABEL (labelSpecial), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (labelSpecial), 1, 0.5);
#else
	gtk_label_set_justify (GTK_LABEL (labelSpecial), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelSpecial), 0, 0.5);
#endif

	listSpecial = gtk_option_menu_new ();
	/**/ g_object_set_data(G_OBJECT(listSpecial), WIDGET_ID_TAG, (gpointer) id_MENU_SPECIAL_INDENT);
	gtk_widget_show (listSpecial);
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
	gtk_table_attach ( GTK_TABLE(boxSpacing), listSpecial, 3, 4, 2, 3,
                    (GtkAttachOptions) (GTK_SHRINK),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
#else
	gtk_table_attach ( GTK_TABLE(boxSpacing), listSpecial, 2,3, 3,4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

#endif
	listSpecial_menu = gtk_menu_new ();

	glade_menuitem = gtk_menu_item_new_with_label(" "); // add an empty menu option to fix bug 594
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listSpecial_menu), glade_menuitem);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_SpecialNone,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemNone = glade_menuitem;
	/**/ g_object_set_data(G_OBJECT(m_menuitemNone), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_INDENT);
	/**/ g_object_set_data(G_OBJECT(m_menuitemNone), WIDGET_MENU_VALUE_TAG, (gpointer) indent_NONE);
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listSpecial_menu), glade_menuitem);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_SpecialFirstLine,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemFirstLine = glade_menuitem;
	/**/ g_object_set_data(G_OBJECT(m_menuitemFirstLine), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_INDENT);
	/**/ g_object_set_data(G_OBJECT(m_menuitemFirstLine), WIDGET_MENU_VALUE_TAG, (gpointer) indent_FIRSTLINE);
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listSpecial_menu), glade_menuitem);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_SpecialHanging,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemHanging = glade_menuitem;
	/**/ g_object_set_data(G_OBJECT(m_menuitemHanging), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_INDENT);
	/**/ g_object_set_data(G_OBJECT(m_menuitemHanging), WIDGET_MENU_VALUE_TAG, (gpointer) indent_HANGING);
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listSpecial_menu), glade_menuitem);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (listSpecial), listSpecial_menu);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_LabelBy,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	labelBy = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelBy);
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
	gtk_table_attach ( GTK_TABLE(boxSpacing), labelBy, 2, 3, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (labelBy), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (labelBy), 1, 0.5);
#else
	gtk_table_attach ( GTK_TABLE(boxSpacing), labelBy, 3,4, 2,3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (labelBy), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelBy), 0, 0.5);
#endif
//	spinbuttonBy_adj = gtk_adjustment_new (0.5, 0, 100, 0.1, 10, 10);
//	spinbuttonBy = gtk_spin_button_new (NULL, 1, 1);
	spinbuttonBy = gtk_entry_new();
	/**/ g_object_set_data(G_OBJECT(spinbuttonBy), WIDGET_ID_TAG, (gpointer) id_SPIN_SPECIAL_INDENT);
	gtk_widget_show (spinbuttonBy);
	gtk_table_attach ( GTK_TABLE(boxSpacing), spinbuttonBy, 3,4, 3,4,
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
                    (GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND),
#else
                    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
#endif
                    (GtkAttachOptions) (GTK_FILL), 0, 0);


	hboxSpacing = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hboxSpacing);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_LabelSpacing,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	labelSpacing = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_box_pack_start (GTK_BOX (hboxSpacing), labelSpacing, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (labelSpacing), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelSpacing), 0, 0.5);

	hseparator1 = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (hboxSpacing), hseparator1, TRUE, TRUE, 0);
	gtk_table_attach ( GTK_TABLE(boxSpacing), hboxSpacing, 0,4, 4,5,
                    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_LabelBefore,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	labelBefore = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_table_attach ( GTK_TABLE(boxSpacing), labelBefore, 0,1, 5,6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (labelBefore), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (labelBefore), 1, 0.5);

//	spinbuttonBefore_adj = gtk_adjustment_new (0, 0, 1500, 0.1, 10, 10);
//	spinbuttonBefore = gtk_spin_button_new (NULL, 1, 1);
	spinbuttonBefore = gtk_entry_new();
	/**/ g_object_set_data(G_OBJECT(spinbuttonBefore), WIDGET_ID_TAG, (gpointer) id_SPIN_BEFORE_SPACING);
	gtk_table_attach ( GTK_TABLE(boxSpacing), spinbuttonBefore, 1,2, 5,6,
                    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_LabelAfter,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	labelAfter = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_table_attach ( GTK_TABLE(boxSpacing), labelAfter, 0,1, 6,7,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (labelAfter), GTK_JUSTIFY_RIGHT);
	gtk_misc_set_alignment (GTK_MISC (labelAfter), 1, 0.5);

//	spinbuttonAfter_adj = gtk_adjustment_new (0, 0, 1500, 0.1, 10, 10);
//	spinbuttonAfter = gtk_spin_button_new (NULL, 1, 1);
	spinbuttonAfter = gtk_entry_new();
	/**/ g_object_set_data(G_OBJECT(spinbuttonAfter), WIDGET_ID_TAG, (gpointer) id_SPIN_AFTER_SPACING);
	gtk_table_attach ( GTK_TABLE(boxSpacing), spinbuttonAfter, 1,2, 6,7,
                    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_LabelLineSpacing,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	labelLineSpacing = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_table_attach ( GTK_TABLE(boxSpacing), labelLineSpacing, 2,3, 5,6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (labelLineSpacing), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelLineSpacing), 0, 0.5);

	listLineSpacing = gtk_option_menu_new ();
	/**/ g_object_set_data(G_OBJECT(listLineSpacing), WIDGET_ID_TAG, (gpointer) id_MENU_SPECIAL_SPACING);
	gtk_table_attach ( GTK_TABLE(boxSpacing), listLineSpacing, 2,3, 6,7,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
	listLineSpacing_menu = gtk_menu_new ();
	
	glade_menuitem = gtk_menu_item_new_with_label(" "); // add an empty menu option to fix bug 594
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listLineSpacing_menu), glade_menuitem);	
	
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_SpacingSingle,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemSingle = glade_menuitem;
	/**/ g_object_set_data(G_OBJECT(m_menuitemSingle), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_SPACING);
	/**/ g_object_set_data(G_OBJECT(m_menuitemSingle), WIDGET_MENU_VALUE_TAG, (gpointer) spacing_SINGLE);
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listLineSpacing_menu), glade_menuitem);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_SpacingHalf,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemOneAndHalf = glade_menuitem;
	/**/ g_object_set_data(G_OBJECT(m_menuitemOneAndHalf), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_SPACING);
	/**/ g_object_set_data(G_OBJECT(m_menuitemOneAndHalf), WIDGET_MENU_VALUE_TAG, (gpointer) spacing_ONEANDHALF);
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listLineSpacing_menu), glade_menuitem);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_SpacingDouble,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemDouble = glade_menuitem;
	/**/ g_object_set_data(G_OBJECT(m_menuitemDouble), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_SPACING);
	/**/ g_object_set_data(G_OBJECT(m_menuitemDouble), WIDGET_MENU_VALUE_TAG, (gpointer) spacing_DOUBLE);
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listLineSpacing_menu), glade_menuitem);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_SpacingAtLeast,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemAtLeast = glade_menuitem;
	/**/ g_object_set_data(G_OBJECT(m_menuitemAtLeast), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_SPACING);
	/**/ g_object_set_data(G_OBJECT(m_menuitemAtLeast), WIDGET_MENU_VALUE_TAG, (gpointer) spacing_ATLEAST);
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listLineSpacing_menu), glade_menuitem);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_SpacingExactly,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemExactly = glade_menuitem;
	/**/ g_object_set_data(G_OBJECT(m_menuitemExactly), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_SPACING);
	/**/ g_object_set_data(G_OBJECT(m_menuitemExactly), WIDGET_MENU_VALUE_TAG, (gpointer) spacing_EXACTLY);
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listLineSpacing_menu), glade_menuitem);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_SpacingMultiple,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	glade_menuitem = gtk_menu_item_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ m_menuitemMultiple = glade_menuitem;
	/**/ g_object_set_data(G_OBJECT(m_menuitemMultiple), WIDGET_MENU_PARENT_ID_TAG, (gpointer) id_MENU_SPECIAL_SPACING);
	/**/ g_object_set_data(G_OBJECT(m_menuitemMultiple), WIDGET_MENU_VALUE_TAG, (gpointer) spacing_MULTIPLE);
	gtk_widget_show (glade_menuitem);
	gtk_menu_shell_append (GTK_MENU_SHELL (listLineSpacing_menu), glade_menuitem);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (listLineSpacing), listLineSpacing_menu);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_LabelAt,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	labelAt = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_table_attach ( GTK_TABLE(boxSpacing), labelAt, 3,4, 5,6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (labelAt), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelAt), 0, 0.5);

//	spinbuttonAt_adj = gtk_adjustment_new (0.5, 0, 100, 0.1, 10, 10);
//	spinbuttonAt = gtk_spin_button_new (NULL, 1, 1);
	spinbuttonAt = gtk_entry_new();
	/**/ g_object_set_data(G_OBJECT(spinbuttonAt), WIDGET_ID_TAG, (gpointer) id_SPIN_SPECIAL_SPACING);
	gtk_table_attach ( GTK_TABLE(boxSpacing), spinbuttonAt, 3,4, 6,7,
                    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

//TODO: In hildon only hide components for future features
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	gtk_widget_show (labelSpacing);
	gtk_widget_show (hseparator1);
	gtk_widget_show (labelBefore);
	gtk_widget_show (spinbuttonBefore);
	gtk_widget_show (labelAfter);
	gtk_widget_show (spinbuttonAfter);
	gtk_widget_show (labelLineSpacing);
	gtk_widget_show (listLineSpacing);
	gtk_widget_show (labelAt);	
	gtk_widget_show (spinbuttonAt);
#endif /* HAVE_HILDON */ 	

	// The "Line and Page Breaks" page
	boxBreaks = gtk_table_new (6, 2, FALSE);
	gtk_widget_show (boxBreaks);
	gtk_table_set_row_spacings (GTK_TABLE(boxBreaks), 5);
	gtk_table_set_col_spacings (GTK_TABLE(boxBreaks), 5);
	gtk_container_set_border_width (GTK_CONTAINER(boxBreaks), 5);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_TabLabelLineAndPageBreaks,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	labelBreaks = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelBreaks);

	gtk_notebook_append_page (GTK_NOTEBOOK (tabMain), boxBreaks, labelBreaks);


	// Pagination headline
	hboxPagination = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hboxPagination);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_LabelPagination,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	labelPagination = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelPagination);
	gtk_box_pack_start (GTK_BOX (hboxPagination), labelPagination, FALSE, FALSE, 0);

	hseparator5 = gtk_hseparator_new ();
	gtk_widget_show (hseparator5);
	gtk_box_pack_start (GTK_BOX (hboxPagination), hseparator5, TRUE, TRUE, 0);

	gtk_table_attach ( GTK_TABLE(boxBreaks), hboxPagination, 0,2, 0,1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);


	// Pagination toggles
	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_PushWidowOrphanControl,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	checkbuttonWidowOrphan = gtk_check_button_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ g_object_set_data(G_OBJECT(checkbuttonWidowOrphan), WIDGET_ID_TAG, (gpointer) id_CHECK_WIDOW_ORPHAN);
	gtk_widget_show (checkbuttonWidowOrphan);
	gtk_table_attach ( GTK_TABLE(boxBreaks), checkbuttonWidowOrphan, 0,1, 1,2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0 );

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_PushKeepWithNext,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	checkbuttonKeepNext = gtk_check_button_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ g_object_set_data(G_OBJECT(checkbuttonKeepNext), WIDGET_ID_TAG, (gpointer) id_CHECK_KEEP_NEXT);
	gtk_widget_show (checkbuttonKeepNext);
	gtk_table_attach ( GTK_TABLE(boxBreaks), checkbuttonKeepNext, 1,2, 1,2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0 );

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_PushKeepLinesTogether,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	checkbuttonKeepLines = gtk_check_button_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ g_object_set_data(G_OBJECT(checkbuttonKeepLines), WIDGET_ID_TAG, (gpointer) id_CHECK_KEEP_LINES);
	gtk_widget_show (checkbuttonKeepLines);
	gtk_table_attach ( GTK_TABLE(boxBreaks), checkbuttonKeepLines, 0,1, 2,3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0 );

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_PushPageBreakBefore,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	checkbuttonPageBreak = gtk_check_button_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ g_object_set_data(G_OBJECT(checkbuttonPageBreak), WIDGET_ID_TAG, (gpointer) id_CHECK_PAGE_BREAK);
	gtk_widget_show (checkbuttonPageBreak);
	gtk_table_attach ( GTK_TABLE(boxBreaks), checkbuttonPageBreak, 1,2, 2,3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0 );


	hseparator6 = gtk_hseparator_new ();
	gtk_widget_show (hseparator6);
	gtk_table_attach ( GTK_TABLE(boxBreaks), hseparator6, 0,2, 3,4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0 );

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_PushSuppressLineNumbers,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	checkbuttonSuppress = gtk_check_button_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ g_object_set_data(G_OBJECT(checkbuttonSuppress), WIDGET_ID_TAG, (gpointer) id_CHECK_SUPPRESS);
	gtk_widget_show (checkbuttonSuppress);
	gtk_table_attach ( GTK_TABLE(boxBreaks), checkbuttonSuppress, 0,1, 4,5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0 );

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_PushNoHyphenate,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	checkbuttonHyphenate = gtk_check_button_new_with_label (unixstr);
	FREEP(unixstr);
	/**/ g_object_set_data(G_OBJECT(checkbuttonHyphenate), WIDGET_ID_TAG, (gpointer) id_CHECK_NO_HYPHENATE);
	gtk_widget_show (checkbuttonHyphenate);
	gtk_table_attach ( GTK_TABLE(boxBreaks), checkbuttonHyphenate, 0,1, 5,6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0 );

	// End of notebook. Next comes the preview area.
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	hboxPreview = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hboxPreview);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Para_LabelPreview,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	labelPreview = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelPreview);
	gtk_box_pack_start (GTK_BOX (hboxPreview), labelPreview, FALSE, TRUE, 0);
	gtk_label_set_justify (GTK_LABEL (labelPreview), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelPreview), 0, 0.5);

	hseparator4 = gtk_hseparator_new ();
	gtk_widget_show (hseparator4);
	gtk_box_pack_start (GTK_BOX (hboxPreview), hseparator4, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vboxContents), hboxPreview, TRUE, TRUE, 0);


	hboxPreviewFrame = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hboxPreviewFrame);

	framePreview = gtk_frame_new (NULL);
	gtk_widget_show (framePreview);

	gtk_box_pack_start (GTK_BOX (hboxPreviewFrame), framePreview, TRUE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vboxContents), hboxPreviewFrame, FALSE, TRUE, 0);
	gtk_widget_set_size_request (framePreview, 400, 150);
	gtk_frame_set_shadow_type (GTK_FRAME (framePreview), GTK_SHADOW_NONE);

	drawingareaPreview = createDrawingArea ();
	gtk_widget_show (drawingareaPreview);
	gtk_container_add (GTK_CONTAINER (framePreview), drawingareaPreview);
#endif

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowContents = vboxContents;

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

#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
	m_drawingareaPreview = NULL;
#else
	m_drawingareaPreview = drawingareaPreview;
#endif 

	m_checkbuttonWidowOrphan = checkbuttonWidowOrphan;
	m_checkbuttonKeepLines = checkbuttonKeepLines;
	m_checkbuttonPageBreak = checkbuttonPageBreak;
	m_checkbuttonSuppress = checkbuttonSuppress;
	m_checkbuttonHyphenate = checkbuttonHyphenate;
	m_checkbuttonKeepNext = checkbuttonKeepNext;
	m_checkbuttonDomDirection = checkbuttonDomDirection;

	return vboxContents;
}

#define CONNECT_SPIN_SIGNAL_CHANGED(w)				\
        do {												\
	        g_signal_connect(G_OBJECT(w), "changed",	\
                G_CALLBACK(s_spin_changed),			\
                (gpointer) this);							\
        } while (0)

#define CONNECT_SPIN_SIGNAL_FOCUS_OUT(w)			\
        do {												\
	        g_signal_connect(G_OBJECT(w), "focus_out_event",	\
                G_CALLBACK(s_spin_focus_out),			\
                (gpointer) this);							\
        } while (0)

#define CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(w)				\
        do {												\
	        g_signal_connect(G_OBJECT(w), "activate",	\
                G_CALLBACK(s_menu_item_activate),		\
                (gpointer) this);							\
        } while (0)

void AP_UnixDialog_Paragraph::_connectCallbackSignals(void)
{
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
	g_signal_connect(G_OBJECT(m_checkbuttonWidowOrphan), "toggled",
					   G_CALLBACK(s_check_toggled), (gpointer) this);
	g_signal_connect(G_OBJECT(m_checkbuttonKeepLines), "toggled",
					   G_CALLBACK(s_check_toggled), (gpointer) this);
	g_signal_connect(G_OBJECT(m_checkbuttonPageBreak), "toggled",
					   G_CALLBACK(s_check_toggled), (gpointer) this);
	g_signal_connect(G_OBJECT(m_checkbuttonSuppress), "toggled",
					   G_CALLBACK(s_check_toggled), (gpointer) this);
	g_signal_connect(G_OBJECT(m_checkbuttonHyphenate), "toggled",
					   G_CALLBACK(s_check_toggled), (gpointer) this);
	g_signal_connect(G_OBJECT(m_checkbuttonKeepNext), "toggled",
					   G_CALLBACK(s_check_toggled), (gpointer) this);
	g_signal_connect(G_OBJECT(m_checkbuttonDomDirection), "toggled",
					   G_CALLBACK(s_check_toggled), (gpointer) this);

#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	// the expose event off the preview
	g_signal_connect(G_OBJECT(m_drawingareaPreview),
					   "expose_event",
					   G_CALLBACK(s_preview_exposed),
					   (gpointer) this);
#endif
}

void AP_UnixDialog_Paragraph::_populateWindowData(void)
{

	// alignment option menu
	UT_ASSERT(m_listAlignment);
	gtk_option_menu_set_history(GTK_OPTION_MENU(m_listAlignment),
								(gint) _getMenuItemValue(id_MENU_ALIGNMENT));

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
								(gint) _getMenuItemValue(id_MENU_SPECIAL_INDENT));

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
								(gint) _getMenuItemValue(id_MENU_SPECIAL_SPACING));

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
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(m_checkbuttonDomDirection)),
								 (_getCheckItemValue(id_CHECK_DOMDIRECTION) == check_TRUE));
}

void AP_UnixDialog_Paragraph::_syncControls(tControl changed, bool bAll /* = false */)
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
										(gint) _getMenuItemValue(id_MENU_SPECIAL_INDENT));
		}
	}
	if (bAll || (changed == id_MENU_SPECIAL_INDENT))
	{
		switch(_getMenuItemValue(id_MENU_SPECIAL_INDENT))
		{
		case indent_NONE:
			// clear the spin control
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBy), "");
			gtk_widget_set_sensitive(m_spinbuttonBy, FALSE);
			break;

		default:
			// set the spin control
			gtk_widget_set_sensitive(m_spinbuttonBy, TRUE);
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
										(gint) _getMenuItemValue(id_MENU_SPECIAL_SPACING));
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
			gtk_widget_set_sensitive(m_spinbuttonAt, FALSE);
			break;

		default:
			// set the spin control
			gtk_widget_set_sensitive(m_spinbuttonAt, TRUE);
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
			break;
		case id_SPIN_RIGHT_INDENT:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonRight), 	_getSpinItemValue(id_SPIN_RIGHT_INDENT));
			break;
		case id_SPIN_SPECIAL_INDENT:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBy), 		_getSpinItemValue(id_SPIN_SPECIAL_INDENT));
			break;
		case id_SPIN_BEFORE_SPACING:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBefore), 	_getSpinItemValue(id_SPIN_BEFORE_SPACING));
			break;
		case id_SPIN_AFTER_SPACING:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonAfter), 	_getSpinItemValue(id_SPIN_AFTER_SPACING));
			break;
		case id_SPIN_SPECIAL_SPACING:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonAt), 		_getSpinItemValue(id_SPIN_SPECIAL_SPACING));
			break;
		default:
			break;
		}
	}
}
