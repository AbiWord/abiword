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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_units.h"
#include "ut_dialogHelper.h"
#include "xap_UnixDlg_FontChooser.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#define DELETEP(p)	do { if (p) delete(p); } while (0)
#define FREEP(p)	do { if (p) free(p); } while (0)

#define SIZE_STRING_SIZE	10

/*****************************************************************/
AP_Dialog * AP_UnixDialog_FontChooser::static_constructor(AP_DialogFactory * pFactory,
														 AP_Dialog_Id id)
{
	AP_UnixDialog_FontChooser * p = new AP_UnixDialog_FontChooser(pFactory,id);
	return p;
}

AP_UnixDialog_FontChooser::AP_UnixDialog_FontChooser(AP_DialogFactory * pDlgFactory,
												   AP_Dialog_Id id)
	: AP_Dialog_FontChooser(pDlgFactory,id)
{
}

AP_UnixDialog_FontChooser::~AP_UnixDialog_FontChooser(void)
{
}

/*****************************************************************/

static void s_delete_clicked(GtkWidget * widget, gpointer data,
							 AP_Dialog_FontChooser::tAnswer * answer)
{
	*answer = AP_Dialog_FontChooser::a_CANCEL;
	gtk_main_quit();
}

static void s_ok_clicked(GtkWidget * widget,
						 AP_Dialog_FontChooser::tAnswer * answer)
{
	*answer = AP_Dialog_FontChooser::a_OK;
	gtk_main_quit();
}

static void s_cancel_clicked(GtkWidget * widget,
							 AP_Dialog_FontChooser::tAnswer * answer)
{
	*answer = AP_Dialog_FontChooser::a_CANCEL;
	gtk_main_quit();
}

static void s_select_row_font(GtkWidget * widget,
							  gint row,
							  gint column,
							  GdkEventButton * event,
							  AP_UnixDialog_FontChooser * dlg)
{
	UT_ASSERT(widget);
	UT_ASSERT(dlg);

	// redisplay the preview text
	dlg->updatePreview();
}
static void s_select_row_style(GtkWidget * widget,
							   gint row,
							   gint column,
							   GdkEventButton * event,
							   AP_UnixDialog_FontChooser * dlg)
{
	UT_ASSERT(widget);
	UT_ASSERT(dlg);

	// redisplay the preview text
	dlg->updatePreview();
}
static void s_select_row_size(GtkWidget * widget,
							  gint row,
							  gint column,
							  GdkEventButton * event,
							  AP_UnixDialog_FontChooser * dlg)
{
	UT_ASSERT(widget);
	UT_ASSERT(dlg);

	// redisplay the preview text
	dlg->updatePreview();
}

static gint searchCList(GtkCList * clist, char * compareText)
{
	UT_ASSERT(clist);

	// if text is null, it's not found
	if (!compareText)
		return -1;
	
	gchar * text[2] = {NULL, NULL};
	
	for (gint i = 0; i < clist->rows; i++)
	{
		gtk_clist_get_text(clist, i, 0, text);
		if (text && text[0])
			if (!UT_stricmp(text[0], compareText))
				return i;
	}

	return -1;
}
	

/*****************************************************************/

// Glade helper function
GtkWidget * AP_UnixDialog_FontChooser::get_widget(GtkWidget * widget, gchar * widget_name)
{
	GtkWidget *found_widget;

	if (widget->parent)
		widget = gtk_widget_get_toplevel (widget);
	found_widget = (GtkWidget*) gtk_object_get_data (GTK_OBJECT (widget),
													 widget_name);
	if (!found_widget)
		g_warning ("Widget not found: %s", widget_name);
	return found_widget;
}

// Glade helper function
void AP_UnixDialog_FontChooser::set_notebook_tab(GtkWidget * notebook, gint page_num,
												 GtkWidget * widget)
{
	GtkNotebookPage *page;
	GtkWidget *notebook_page;

	page = (GtkNotebookPage*) g_list_nth (GTK_NOTEBOOK (notebook)->children, page_num)->data;
	notebook_page = page->child;
	gtk_widget_ref (notebook_page);
	gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), page_num);
	gtk_notebook_insert_page (GTK_NOTEBOOK (notebook), notebook_page,
							  widget, page_num);
	gtk_widget_unref (notebook_page);
}

// Glade generated dialog, using fixed widgets to closely match
// the Windows layout, with some changes for color selector
GtkWidget * AP_UnixDialog_FontChooser::create_windowFontSelection(void)
{
	GtkWidget *windowFontSelection;
	GtkWidget *vboxMain;
	GtkWidget *notebookMain;
	GtkWidget *fixedFont;
	GtkWidget *labelFont;
	GtkWidget *labelStyle;
	GtkWidget *frameFonts;
	GtkWidget *listFonts;
	GtkWidget *labelSize;
	GtkWidget *frameEffects;
	GtkWidget *vbox2;
	GtkWidget *checkbuttonStrikeout;
	GtkWidget *checkbuttonUnderline;
	GtkWidget *frameStyle;
	GtkWidget *listStyles;
	GtkWidget *frameSize;
	GtkWidget *listSizes;
	GtkWidget *fixedColor;
	GtkWidget *hbox1;
	GtkWidget *colorSelector;
	GtkWidget *labelTabFont;
	GtkWidget *labelTabColor;
	GtkWidget *frame4;
	GtkWidget *entryPreview;
	GtkWidget *fixedButtons;
	GtkWidget *buttonOK;
	GtkWidget *buttonCancel;

	windowFontSelection = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "windowFontSelection", windowFontSelection);
	gtk_window_set_title (GTK_WINDOW (windowFontSelection), "Font");
	gtk_window_set_policy (GTK_WINDOW (windowFontSelection), FALSE, FALSE, FALSE);

	vboxMain = gtk_vbox_new (FALSE, 0);
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "vboxMain", vboxMain);
	gtk_widget_show (vboxMain);
	gtk_container_add (GTK_CONTAINER (windowFontSelection), vboxMain);
	gtk_widget_set_usize (vboxMain, 469, -1);

	notebookMain = gtk_notebook_new ();
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "notebookMain", notebookMain);
	gtk_widget_show (notebookMain);
	gtk_box_pack_start (GTK_BOX (vboxMain), notebookMain, FALSE, FALSE, 0);
	gtk_widget_set_usize (notebookMain, 418, 247);
	gtk_container_border_width (GTK_CONTAINER (notebookMain), 8);

	fixedFont = gtk_fixed_new ();
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "fixedFont", fixedFont);
	gtk_widget_show (fixedFont);
	gtk_container_add (GTK_CONTAINER (notebookMain), fixedFont);
	gtk_widget_set_usize (fixedFont, -1, 191);

	labelFont = gtk_label_new ("Font:");
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "labelFont", labelFont);
	gtk_widget_show (labelFont);
	gtk_fixed_put (GTK_FIXED (fixedFont), labelFont, 8, 8);
	gtk_widget_set_usize (labelFont, 34, 16);

	labelStyle = gtk_label_new ("Style:");
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "labelStyle", labelStyle);
	gtk_widget_show (labelStyle);
	gtk_fixed_put (GTK_FIXED (fixedFont), labelStyle, 216, 8);
	gtk_widget_set_usize (labelStyle, 34, 16);

	frameFonts = gtk_scrolled_window_new(NULL, NULL);
	gtk_object_set_data(GTK_OBJECT(windowFontSelection), "frameFonts", frameFonts);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(frameFonts), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_fixed_put(GTK_FIXED(fixedFont), frameFonts, 8, 24);
	gtk_widget_set_usize(frameFonts, 195, 167);
	gtk_widget_show(frameFonts);

	listFonts = gtk_clist_new (1);
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "listFonts", listFonts);
	gtk_clist_set_selection_mode (GTK_CLIST(listFonts), GTK_SELECTION_SINGLE);
	gtk_clist_set_shadow_type (GTK_CLIST(listFonts), GTK_SHADOW_IN);
	gtk_clist_set_column_auto_resize (GTK_CLIST(listFonts), 0, TRUE);
	gtk_widget_show (listFonts);
	gtk_container_add (GTK_CONTAINER (frameFonts), listFonts);

	labelSize = gtk_label_new ("Size:");
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "labelSize", labelSize);
	gtk_widget_show (labelSize);
	gtk_fixed_put (GTK_FIXED (fixedFont), labelSize, 356, 8);
	gtk_widget_set_usize (labelSize, 34, 16);

	frameEffects = gtk_frame_new ("Effects");
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "frameEffects", frameEffects);
	gtk_widget_show (frameEffects);
	gtk_fixed_put (GTK_FIXED (fixedFont), frameEffects, 216, 127);
	gtk_widget_set_usize (frameEffects, 206, 65);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "vbox2", vbox2);
	gtk_widget_show (vbox2);
	gtk_container_add (GTK_CONTAINER (frameEffects), vbox2);

	checkbuttonStrikeout = gtk_check_button_new_with_label ("Strikeout");
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "checkbuttonStrikeout", checkbuttonStrikeout);
	gtk_widget_show (checkbuttonStrikeout);
	gtk_box_pack_start (GTK_BOX (vbox2), checkbuttonStrikeout, TRUE, TRUE, 0);

	checkbuttonUnderline = gtk_check_button_new_with_label ("Underline");
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "checkbuttonUnderline", checkbuttonUnderline);
	gtk_widget_show (checkbuttonUnderline);
	gtk_box_pack_start (GTK_BOX (vbox2), checkbuttonUnderline, TRUE, TRUE, 0);

	frameStyle = gtk_scrolled_window_new(NULL, NULL);
	gtk_object_set_data(GTK_OBJECT(windowFontSelection), "frameStyle", frameStyle);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(frameStyle), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_fixed_put(GTK_FIXED(fixedFont), frameStyle, 216, 24);
	gtk_widget_set_usize(frameStyle, 126, 95);
	gtk_widget_show(frameStyle);

	listStyles = gtk_clist_new (1);
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "listStyles", listStyles);
	gtk_clist_set_selection_mode (GTK_CLIST(listStyles), GTK_SELECTION_SINGLE);
	gtk_clist_set_shadow_type (GTK_CLIST(listStyles), GTK_SHADOW_IN);
	gtk_clist_set_column_auto_resize (GTK_CLIST(listStyles), 0, TRUE);
	gtk_widget_show (listStyles);
	gtk_container_add (GTK_CONTAINER (frameStyle), listStyles);

	frameSize = gtk_scrolled_window_new(NULL, NULL);
	gtk_object_set_data(GTK_OBJECT(windowFontSelection), "frameSize", frameSize);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(frameSize), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_fixed_put(GTK_FIXED(fixedFont), frameSize, 356, 24);
	gtk_widget_set_usize(frameSize, 66, 95);
	gtk_widget_show(frameSize);

	listSizes = gtk_clist_new (1);
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "listSizes", listSizes);
	gtk_clist_set_selection_mode (GTK_CLIST(listSizes), GTK_SELECTION_SINGLE);
	gtk_clist_set_shadow_type (GTK_CLIST(listSizes), GTK_SHADOW_IN);
	gtk_clist_set_column_auto_resize (GTK_CLIST(listSizes), 0, TRUE);
	gtk_widget_show (listSizes);
	gtk_container_add (GTK_CONTAINER (frameSize), listSizes);

	fixedColor = gtk_fixed_new ();
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "fixedColor", fixedColor);
	gtk_widget_show (fixedColor);
	gtk_container_add (GTK_CONTAINER (notebookMain), fixedColor);
	gtk_widget_set_usize (fixedColor, 421, 187);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "hbox1", hbox1);
	gtk_widget_show (hbox1);
	gtk_fixed_put (GTK_FIXED (fixedColor), hbox1, 8, 8);
	gtk_widget_set_usize (hbox1, 425, 190);

	colorSelector = gtk_color_selection_new ();
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "colorSelector", colorSelector);
	gtk_widget_show (colorSelector);
	gtk_box_pack_start (GTK_BOX (hbox1), colorSelector, TRUE, TRUE, 0);

	labelTabFont = gtk_label_new ("   Font   ");
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "labelTabFont", labelTabFont);
	gtk_widget_show (labelTabFont);
	set_notebook_tab (notebookMain, 0, labelTabFont);

	labelTabColor = gtk_label_new ("   Color   ");
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "labelTabColor", labelTabColor);
	gtk_widget_show (labelTabColor);
	set_notebook_tab (notebookMain, 1, labelTabColor);

	frame4 = gtk_frame_new (NULL);
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "frame4", frame4);
	gtk_widget_show (frame4);
	gtk_box_pack_start (GTK_BOX (vboxMain), frame4, FALSE, TRUE, 0);
	gtk_widget_set_usize (frame4, -1, 69);
	gtk_container_border_width (GTK_CONTAINER (frame4), 8);
	gtk_frame_set_shadow_type (GTK_FRAME (frame4), GTK_SHADOW_NONE);

	entryPreview = gtk_entry_new ();
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "entryPreview", entryPreview);
	gtk_widget_show (entryPreview);
	gtk_container_add (GTK_CONTAINER (frame4), entryPreview);
	gtk_widget_set_usize (entryPreview, -1, 56);
	gtk_entry_set_editable (GTK_ENTRY (entryPreview), FALSE);
	gtk_widget_set_sensitive(entryPreview, FALSE);
	gtk_entry_set_text (GTK_ENTRY (entryPreview), "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz");

	fixedButtons = gtk_fixed_new ();
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "fixedButtons", fixedButtons);
	gtk_widget_show (fixedButtons);
	gtk_box_pack_start (GTK_BOX (vboxMain), fixedButtons, FALSE, TRUE, 0);
	gtk_widget_set_usize (fixedButtons, -1, 43);

	buttonOK = gtk_button_new_with_label ("OK");
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "buttonOK", buttonOK);
	gtk_widget_show (buttonOK);
	gtk_fixed_put (GTK_FIXED (fixedButtons), buttonOK, 276, 0);
	gtk_widget_set_usize (buttonOK, 82, 35);
	GTK_WIDGET_SET_FLAGS (buttonOK, GTK_CAN_DEFAULT);
	gtk_widget_grab_default (buttonOK);

	buttonCancel = gtk_button_new_with_label ("Cancel");
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "buttonCancel", buttonCancel);
	gtk_widget_show (buttonCancel);
	gtk_fixed_put (GTK_FIXED (fixedButtons), buttonCancel, 369, 6);
	gtk_widget_set_usize (buttonCancel, 72, 24);

	// save out to members for callback and class access
    m_fontList = listFonts;
	m_styleList = listStyles;
	m_sizeList = listSizes;
	m_colorSelector = colorSelector;
	m_previewEntry = entryPreview;
	m_checkStrikeOut = checkbuttonStrikeout;
	m_checkUnderline = checkbuttonUnderline;

	// bind signals to things
	gtk_signal_connect_after(GTK_OBJECT(windowFontSelection),
							  "destroy",
							  NULL,
							  NULL);
	gtk_signal_connect_after(GTK_OBJECT(windowFontSelection),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 (void *) &m_answer);

	gtk_signal_connect(GTK_OBJECT(buttonOK),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (void *) &m_answer);
	gtk_signal_connect(GTK_OBJECT(buttonCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   (void *) &m_answer);
	gtk_signal_connect(GTK_OBJECT(listFonts),
					   "select_row",
					   GTK_SIGNAL_FUNC(s_select_row_font),
					   (void *) this);
	gtk_signal_connect(GTK_OBJECT(listStyles),
					   "select_row",
					   GTK_SIGNAL_FUNC(s_select_row_style),
					   (void *) this);
	gtk_signal_connect(GTK_OBJECT(listSizes),
					   "select_row",
					   GTK_SIGNAL_FUNC(s_select_row_size),
					   (void *) this);
	
	GTK_WIDGET_SET_FLAGS(listFonts, GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS(listStyles, GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS(listSizes, GTK_CAN_FOCUS);

	gchar * text[2] = {NULL, NULL};

	// update the styles list
	gtk_clist_clear(GTK_CLIST(m_styleList));
	text[0] = "Regular"; 		gtk_clist_append(GTK_CLIST(m_styleList), text);
	text[0] = "Italic"; 		gtk_clist_append(GTK_CLIST(m_styleList), text);
	text[0] = "Bold"; 			gtk_clist_append(GTK_CLIST(m_styleList), text);
	text[0] = "Bold Italic"; 	gtk_clist_append(GTK_CLIST(m_styleList), text);	
	
    gtk_clist_clear(GTK_CLIST(m_sizeList));
	// TODO perhaps populate the list based on the selected font/style?
	text[0] = "8"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	text[0] = "9"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	text[0] = "10"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	text[0] = "11"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	text[0] = "12"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	text[0] = "14"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	text[0] = "16"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	text[0] = "18"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	text[0] = "20"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	text[0] = "22"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	text[0] = "24"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	text[0] = "26"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	text[0] = "28"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	text[0] = "36"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	text[0] = "48"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	text[0] = "72"; gtk_clist_append(GTK_CLIST(m_sizeList), text);
	
	return windowFontSelection;
}

// the real runModal()	
void AP_UnixDialog_FontChooser::runModal(XAP_Frame * pFrame)
{
	m_pUnixFrame = (XAP_UnixFrame *)pFrame;
	UT_ASSERT(m_pUnixFrame);
	AP_UnixApp * pApp = (AP_UnixApp *)m_pUnixFrame->getApp();
	UT_ASSERT(pApp);

	UT_DEBUGMSG(("FontChooserStart: Family[%s] Size[%s] Weight[%s] Style[%s] Color[%s] Underline[%d] StrikeOut[%d]\n",
				 ((m_pFontFamily) ? m_pFontFamily : ""),
				 ((m_pFontSize) ? m_pFontSize : ""),
				 ((m_pFontWeight) ? m_pFontWeight : ""),
				 ((m_pFontStyle) ? m_pFontStyle : ""),
				 ((m_pColor) ? m_pColor : "" ),
				 (m_bUnderline),
				 (m_bStrikeOut)));

	// These define the color element offsets in a vector
	guint RED = 0;
	guint GREEN = 1;
	guint BLUE = 2;
	gdouble currentColor[3] = { 0, 0, 0 };
	gdouble funkyColor[3] = { -1, -1, -1 };

	// this is used many times below to grab pointers to
	// strings inside list elements
	gchar * text[2] = {NULL, NULL};
	// used similarly to convert between text and numeric arguments
	char sizeString[SIZE_STRING_SIZE];
	
	// Set up our own color space so we work well on 8-bit
	// displays.
    gtk_widget_push_visual(gtk_preview_get_visual());
    gtk_widget_push_colormap(gtk_preview_get_cmap());

	// build the dialog
	GtkWidget * cf = create_windowFontSelection();
	UT_ASSERT(cf);

	// freeze updates of the preview
	m_blockUpdate = UT_TRUE;
	
	// Retrieve all the fonts
	AP_App * app = m_pUnixFrame->getApp();
	AP_UnixApp * unixapp = static_cast<AP_UnixApp *> (app);
	m_fontManager = unixapp->getFontManager();

	gtk_clist_clear(GTK_CLIST(m_fontList));

	// to sort out dupes
	UT_HashTable fontHash(256);

	// throw them in the hash save duplicates
	AP_UnixFont ** fonts = m_fontManager->getAllFonts();
	for (UT_uint32 i = 0; i < m_fontManager->getCount(); i++)
	{
		if (!fontHash.findEntry(fonts[i]->getName()))
			fontHash.addEntry((char *) fonts[i]->getName(),
							  (char *) fonts[i]->getName(), NULL);
	}
	DELETEP(fonts);

	// fetch them out
	UT_HashTable::UT_HashEntry * entry;
	for (UT_uint32 k = 0; k < (UT_uint32) fontHash.getEntryCount(); k++)
	{
		entry = fontHash.getNthEntry((int) k);
		UT_ASSERT(entry);
		text[0] = (gchar *) entry->pszLeft;
		gtk_clist_append(GTK_CLIST(m_fontList), text);
	}

	// Set the defaults in the list boxes according to dialog data
	gint foundAt = 0;

	// is this safe with an XML_Char * string?
	foundAt = searchCList(GTK_CLIST(m_fontList), (char *) m_pFontFamily);

	if (foundAt >= 0)
	{
		gtk_clist_select_row(GTK_CLIST(m_fontList), foundAt, 0);
		gtk_clist_moveto(GTK_CLIST(m_fontList), foundAt, 0, 0, -1);
	}
	
	// this is pretty messy
	listStyle st = LIST_STYLE_NORMAL;
	if (!m_pFontStyle || !m_pFontWeight)
	{
		// select nothing
	}
	else if (!UT_stricmp(m_pFontStyle, "normal") &&
			 !UT_stricmp(m_pFontWeight, "normal"))
	{
		st = LIST_STYLE_NORMAL;
	}
	else if (!UT_stricmp(m_pFontStyle, "normal") &&
			 !UT_stricmp(m_pFontWeight, "bold"))
	{
		st = LIST_STYLE_BOLD;
	}
	else if (!UT_stricmp(m_pFontStyle, "italic") &&
			 !UT_stricmp(m_pFontWeight, "normal"))
	{
		st = LIST_STYLE_ITALIC;		
	}
	else if (!UT_stricmp(m_pFontStyle, "italic") &&
			 !UT_stricmp(m_pFontWeight, "bold"))
	{
		st = LIST_STYLE_BOLD_ITALIC;		
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	gtk_clist_select_row(GTK_CLIST(m_styleList), st, 0);
	gtk_clist_moveto(GTK_CLIST(m_fontList), st, 0, 0, -1);
	
	double size = UT_convertToPoints(m_pFontSize);
	g_snprintf(sizeString, SIZE_STRING_SIZE, "%ld", (long) size);
	foundAt = searchCList(GTK_CLIST(m_sizeList), sizeString);

	if (foundAt >= 0)
	{
		gtk_clist_select_row(GTK_CLIST(m_sizeList), foundAt, 0);
		gtk_clist_moveto(GTK_CLIST(m_fontList), foundAt, 0, 0, -1);
	}
	
	// Set color in the color selector
	if (m_pColor)
	{
		UT_RGBColor c;
		UT_parseColor(m_pColor, c);

		currentColor[RED] = ((gdouble) c.m_red / (gdouble) 255.0);
		currentColor[GREEN] = ((gdouble) c.m_grn / (gdouble) 255.0);
		currentColor[BLUE] = ((gdouble) c.m_blu / (gdouble) 255.0);

		gtk_color_selection_set_color(GTK_COLOR_SELECTION(m_colorSelector), currentColor);
	}
	else
	{
		// if we have no color, use a placeholder of funky values
		// the user can't pick interactively.  This catches ALL
		// the cases except where the user specifically enters -1 for
		// all Red, Green and Blue attributes manually.  This user
		// should expect it not to touch the color.  :)
		gtk_color_selection_set_color(GTK_COLOR_SELECTION(m_colorSelector), funkyColor);
	}

	// set the strikeout and underline check buttons
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkStrikeOut), m_bStrikeOut);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkUnderline), m_bUnderline);	
	
	// get top level window and its GtkWidget *
	XAP_UnixFrame * frame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(frame);
	GtkWidget * parent = frame->getTopLevelWindow();
	UT_ASSERT(parent);
	// center it
    centerDialog(parent, GTK_WIDGET(cf));
	gtk_window_set_transient_for(GTK_WINDOW(cf), GTK_WINDOW(parent));
	
	// Run the dialog
	gtk_widget_show(GTK_WIDGET(cf));
	gtk_grab_add(GTK_WIDGET(cf));

	// unfreeze updates of the preview
	m_blockUpdate = UT_FALSE;
	// manually trigger an update
	updatePreview();
	
	gtk_main();

	if (m_answer == AP_Dialog_FontChooser::a_OK)
	{
		GList * selectedRow = NULL;
		gint rowNumber = 0;
		
		selectedRow = GTK_CLIST(m_fontList)->selection;
		if (selectedRow)
		{
			rowNumber = GPOINTER_TO_INT(selectedRow->data);
			gtk_clist_get_text(GTK_CLIST(m_fontList), rowNumber, 0, text);
			UT_ASSERT(text && text[0]);
			if (!m_pFontFamily || UT_stricmp(m_pFontFamily, text[0]))
			{
				setFontFamily(text[0]);
				m_bChangedFontFamily = UT_TRUE;
			}
		}
		
		selectedRow = GTK_CLIST(m_styleList)->selection;
		if (selectedRow)
		{
			rowNumber = GPOINTER_TO_INT(selectedRow->data);
			gtk_clist_get_text(GTK_CLIST(m_styleList), rowNumber, 0, text);
			UT_ASSERT(text && text[0]);

		// perhaps these attributes really should be smashed
		// into bitfields.  :)
			if (rowNumber == LIST_STYLE_NORMAL)
			{
				if (!m_pFontStyle || UT_stricmp(m_pFontStyle, "normal"))
				{
					setFontStyle("normal");
					m_bChangedFontStyle = UT_TRUE;
				}
				if (!m_pFontWeight || UT_stricmp(m_pFontWeight, "normal"))
				{
					setFontWeight("normal");
					m_bChangedFontWeight = UT_TRUE;
				}
			}
			else if (rowNumber == LIST_STYLE_BOLD)
			{
				if (!m_pFontStyle || UT_stricmp(m_pFontStyle, "normal"))
				{
					setFontStyle("normal");
					m_bChangedFontStyle = UT_TRUE;
				}
				if (!m_pFontWeight || UT_stricmp(m_pFontWeight, "bold"))
				{
					setFontWeight("bold");
					m_bChangedFontWeight = UT_TRUE;
				}
			}
			else if (rowNumber == LIST_STYLE_ITALIC)
			{
				if (!m_pFontStyle || UT_stricmp(m_pFontStyle, "italic"))
				{
					setFontStyle("italic");
					m_bChangedFontStyle = UT_TRUE;
				}
				if (!m_pFontWeight || UT_stricmp(m_pFontWeight, "normal"))
				{
					setFontWeight("normal");
					m_bChangedFontWeight = UT_TRUE;
				}
			}
			else if (rowNumber == LIST_STYLE_BOLD_ITALIC)
			{
				if (!m_pFontStyle || UT_stricmp(m_pFontStyle, "italic"))
				{
					setFontStyle("italic");
					m_bChangedFontStyle = UT_TRUE;
				}
				if (!m_pFontWeight || UT_stricmp(m_pFontWeight, "bold"))
				{
					setFontWeight("bold");
					m_bChangedFontWeight = UT_TRUE;
				}
			}
			else
			{
				UT_ASSERT(0);
			}
		}
		
		selectedRow = GTK_CLIST(m_sizeList)->selection;
		if (selectedRow)
		{
			rowNumber = GPOINTER_TO_INT(selectedRow->data);
			gtk_clist_get_text(GTK_CLIST(m_sizeList), rowNumber, 0, text);
			UT_ASSERT(text && text[0]);

			g_snprintf(sizeString, SIZE_STRING_SIZE, "%spt", text[0]);

			if (!m_pFontSize || UT_stricmp(m_pFontSize, sizeString))
			{
				setFontSize(sizeString);
				m_bChangedFontSize = UT_TRUE;
			}
		}
		
		gtk_color_selection_get_color(GTK_COLOR_SELECTION(m_colorSelector), currentColor);

		// test for funkyColor-has-been-changed-to-sane-color case
		if (currentColor[RED] >= 0 &&
			currentColor[GREEN] >= 0 &&
			currentColor[BLUE] >= 0)
		{
			char buf_color[6];
			sprintf(buf_color, "%02x%02x%02x",
					(unsigned int) (currentColor[RED] 	* (gdouble) 255.0),
					(unsigned int) (currentColor[GREEN]	* (gdouble) 255.0),
					(unsigned int) (currentColor[BLUE] 	* (gdouble) 255.0));
		
			if (!m_pColor || UT_stricmp(m_pColor, buf_color))
			{
				setColor(buf_color);
				m_bChangedColor = UT_TRUE;
			}
		}

		m_bChangedStrikeOut = (m_bStrikeOut != gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkStrikeOut)));
		m_bChangedUnderline = (m_bUnderline != gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkUnderline)));
		if (m_bChangedStrikeOut)
			m_bStrikeOut = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkStrikeOut));
		if (m_bChangedUnderline)
			m_bUnderline = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkUnderline));
	}

	gtk_widget_destroy (GTK_WIDGET(cf));

    gtk_widget_pop_visual();
    gtk_widget_pop_colormap();
	
	UT_DEBUGMSG(("FontChooserEnd: Family[%s%s] Size[%s%s] Weight[%s%s] Style[%s%s] Color[%s%s] Underline[%d%s] StrikeOut[%d%s]\n",
				 ((m_pFontFamily) ? m_pFontFamily : ""),	((m_bChangedFontFamily) ? "(chg)" : ""),
				 ((m_pFontSize) ? m_pFontSize : ""),		((m_bChangedFontSize) ? "(chg)" : ""),
				 ((m_pFontWeight) ? m_pFontWeight : ""),	((m_bChangedFontWeight) ? "(chg)" : ""),
				 ((m_pFontStyle) ? m_pFontStyle : ""),		((m_bChangedFontStyle) ? "(chg)" : ""),
				 ((m_pColor) ? m_pColor : "" ),				((m_bChangedColor) ? "(chg)" : ""),
				 (m_bUnderline),							((m_bChangedUnderline) ? "(chg)" : ""),
				 (m_bStrikeOut),							((m_bChangedStrikeOut) ? "(chg)" : "")));

	// answer should be set by the appropriate callback
	// the caller can get the answer from getAnswer().

	m_pUnixFrame = NULL;
}

void AP_UnixDialog_FontChooser::updatePreview(void)
{
	if (m_blockUpdate)
		return;
	
	gchar * fontText[2] = {NULL, NULL};
	gchar * sizeText[2] = {NULL, NULL};
	long sizeNumber = 0;
	AP_UnixFont::style styleNumber;

	GList * selectedRow = NULL;
	gint rowNumber = 0;

	guint RED = 0;
	guint GREEN = 1;
	guint BLUE = 2;
	gdouble currentColor[3] = { 0, 0, 0 };
		
	selectedRow = GTK_CLIST(m_fontList)->selection;
	if (selectedRow)
	{
		rowNumber = GPOINTER_TO_INT(selectedRow->data);
		gtk_clist_get_text(GTK_CLIST(m_fontList), rowNumber, 0, fontText);
		UT_ASSERT(fontText && fontText[0]);
	}
	else
	{
		// if they don't have a font selected, don't do an update
		return;
	}
		
	selectedRow = GTK_CLIST(m_styleList)->selection;
	if (selectedRow)
	{
		gint style = GPOINTER_TO_INT(selectedRow->data);
		if (style == LIST_STYLE_NORMAL)
			styleNumber = AP_UnixFont::STYLE_NORMAL;
		else if (style == LIST_STYLE_BOLD)
			styleNumber = AP_UnixFont::STYLE_BOLD;
		else if (style == LIST_STYLE_ITALIC)
			styleNumber = AP_UnixFont::STYLE_ITALIC;
		else if (style == LIST_STYLE_BOLD_ITALIC)
			styleNumber = AP_UnixFont::STYLE_BOLD_ITALIC;
		else
		{
			UT_ASSERT(0);
		}
	}
	else
	{
		styleNumber = AP_UnixFont::STYLE_NORMAL;
	}
			
	selectedRow = GTK_CLIST(m_sizeList)->selection;
	if (selectedRow)
	{
		rowNumber = GPOINTER_TO_INT(selectedRow->data);
		gtk_clist_get_text(GTK_CLIST(m_sizeList), rowNumber, 0, sizeText);
		UT_ASSERT(sizeText && sizeText[0]);

		sizeNumber = atol(sizeText[0]);
	}
	else
	{
		// this should be had from system-wide preferences
		sizeNumber = 10;
	}
	
		
	gtk_color_selection_get_color(GTK_COLOR_SELECTION(m_colorSelector), currentColor);
			
	char buf_color[6];
	sprintf(buf_color, "%02x%02x%02x",
			(unsigned int) (currentColor[RED] 	* (gdouble) 255.0),
			(unsigned int) (currentColor[GREEN]	* (gdouble) 255.0),
			(unsigned int) (currentColor[BLUE] 	* (gdouble) 255.0));

	// TODO what will we do with these?
//	UT_Bool bStrikeOut = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkStrikeOut));
//	UT_Bool bUnderline = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkUnderline));

	AP_UnixFont * tempUnixFont = m_fontManager->getFont((const char *) fontText[0], styleNumber);
	UT_ASSERT(tempUnixFont);
	
	GdkFont * tempGdkFont = tempUnixFont->getGdkFont(sizeNumber);
	UT_ASSERT(tempGdkFont);

	GtkStyle * style = gtk_style_new();
	gdk_font_unref(style->font);
	style->font = tempGdkFont;
	gdk_font_ref(style->font);

	// this looks kinda dangerous (it makes the text color like normal, even
	// though it can't be edited).
	style->fg[GTK_STATE_INSENSITIVE] = style->fg[GTK_STATE_ACTIVE];

	gtk_widget_set_style(m_previewEntry, style);
	gtk_style_unref(style);

	return;
}
