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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_hash.h"
#include "ut_units.h"
#include "xap_UnixDialogHelper.h"
#include "xap_UnixDlg_FontChooser.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "xap_EncodingManager.h"
#include "gr_UnixGraphics.h"

#define PREVIEW_BOX_BORDER_WIDTH_PIXELS 8
#define PREVIEW_BOX_HEIGHT_PIXELS	80

// your typographer's standard nonsense latin font phrase
#define PREVIEW_ENTRY_DEFAULT_STRING	"Lorem ipsum dolor sit amet, consectetaur adipisicing..."

//
// For Screen color picker
	enum
	{
		RED,
		GREEN,
		BLUE,
		OPACITY
	};

/*****************************************************************/
XAP_Dialog * XAP_UnixDialog_FontChooser::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	XAP_UnixDialog_FontChooser * p = new XAP_UnixDialog_FontChooser(pFactory,id);
	return p;
}

XAP_UnixDialog_FontChooser::XAP_UnixDialog_FontChooser(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id id)
	: XAP_Dialog_FontChooser(pDlgFactory,id)
{
	m_fontManager = NULL;
	m_fontList = NULL;
	m_styleList = NULL;
	m_sizeList = NULL;
	m_checkStrikeOut = NULL;
	m_checkUnderline = NULL;
	m_checkOverline = NULL;
	m_checkTransparency = NULL;
	m_colorSelector = NULL;
	m_bgcolorSelector = NULL;
	m_preview = NULL;

	m_lastFont = NULL;
	m_gc = NULL;
	m_pUnixFrame = NULL;
	m_doneFirstFont = false;
	for(UT_sint32 i = 0; i < 4;i++)
	{
		m_currentFGColor[i] = 0.0;
		m_currentBGColor[i] = -1.0;
		m_funkyColor[i] = -1.0;
	}

}

XAP_UnixDialog_FontChooser::~XAP_UnixDialog_FontChooser(void)
{
	DELETEP(m_gc);
	DELETEP(m_lastFont);
}


/*****************************************************************/

// This is a little UI hack.  Check the widget callback connect point
// for more info.

static gint s_color_wheel_clicked(GtkWidget * area,
                                  GdkEvent * event)
{
	GtkColorSelection * colorSelector;

	colorSelector = (GtkColorSelection *) g_object_get_data(G_OBJECT(area), "_GtkColorSelection");
	
    switch (event->type)
    {
    case GDK_BUTTON_PRESS:
#if 0
		// if the color is black (RGB:0,0,0), and someone clicked on
		// the wheel, and since the wheel has no black area,
		// snap the value up high

		// I first tried checking the hue, but I _always_ got the
		// "modified" version, which means it was never -1 but always
		// processed through the wheel coordinate system, even if
		// I did connect_after(), etc.  So I catch the RGB values,
		// since they're a direct product of the hue, but aren't updated
		// unless the value slider itself is.

		// the 'less than' case catches the state of the color selector
		// when no single color occupied the entire run of text
		if ((gdouble) GTK_COLOR_SELECTION(colorSelector)->values[RED] 	<= (gdouble) 0 &&
			(gdouble) GTK_COLOR_SELECTION(colorSelector)->values[GREEN] <= (gdouble) 0 &&
			(gdouble) GTK_COLOR_SELECTION(colorSelector)->values[BLUE] 	<= (gdouble) 0)

		{
			// snap the "value" slider high
			GTK_COLOR_SELECTION(colorSelector)->values[VALUE] = 1.0;
		}
#endif
		break;
	default:
		break;
    }

	return FALSE;
}

static gint s_color_update(GtkWidget * /* widget */,
						   GdkEvent * /* event */,
						   XAP_UnixDialog_FontChooser * dlg)
{
	UT_ASSERT(dlg);
	dlg->fgColorChanged();
	return FALSE;
}


static gint s_bgcolor_update(GtkWidget * /* widget */,
						   GdkEvent * /* event */,
						   XAP_UnixDialog_FontChooser * dlg)
{
	UT_ASSERT(dlg);
	dlg->bgColorChanged();
	return FALSE;
}

static void s_select_row_font(GtkWidget * /* widget */,
							  gint /* row */,
							  gint /* column */,
							  GdkEventButton * /* event */,
							  XAP_UnixDialog_FontChooser * dlg)
{
	UT_ASSERT(dlg);
    // update the row number and show the changed preview
	// redisplay the preview text
	dlg->fontRowChanged();
}

static void s_select_row_style(GtkWidget * /* widget */,
							   gint /* row */,
							   gint /* column */,
							   GdkEventButton * /* event */,
							   XAP_UnixDialog_FontChooser * dlg)
{
	UT_ASSERT(dlg);

	// redisplay the preview text
	dlg->styleRowChanged();
}

static void s_select_row_size(GtkWidget * /* widget */,
							  gint /* row */,
							  gint /* column */,
							  GdkEventButton * /* event */,
							  XAP_UnixDialog_FontChooser * dlg)
{
	UT_ASSERT(dlg);

	// redisplay the preview text
	dlg->sizeRowChanged();
}

static gint s_drawing_area_expose(GtkWidget * w,
								  GdkEventExpose * /* pExposeEvent */)
{
	XAP_UnixDialog_FontChooser * dlg = (XAP_UnixDialog_FontChooser *)
		                              gtk_object_get_user_data(GTK_OBJECT(w));

//
// Look if updates are blocked and quit if they are.
//
	if(dlg->m_blockUpdate)
		return FALSE;
	dlg->updatePreview();
	return FALSE;
}

static void s_underline_toggled(GtkWidget * w,  XAP_UnixDialog_FontChooser * dlg)
{
	dlg->underlineChanged();
}


static void s_overline_toggled(GtkWidget * w,  XAP_UnixDialog_FontChooser * dlg)
{
	dlg->overlineChanged();
}


static void s_strikeout_toggled(GtkWidget * w,  XAP_UnixDialog_FontChooser * dlg)
{
	dlg->strikeoutChanged();
}


static void s_transparency_toggled(GtkWidget * w,  XAP_UnixDialog_FontChooser * dlg)
{
	dlg->transparencyChanged();
}

/*****************************************************************/

void XAP_UnixDialog_FontChooser::underlineChanged(void)
{
	m_bUnderline = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkUnderline));
	m_bChangedUnderline = !m_bChangedUnderline;
	setFontDecoration(m_bUnderline,m_bOverline,m_bStrikeout,m_bTopline,m_bBottomline);
	updatePreview();
}


void XAP_UnixDialog_FontChooser::strikeoutChanged(void)
{
	m_bStrikeout = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkStrikeOut));
	m_bChangedStrikeOut = !m_bChangedStrikeOut;
	setFontDecoration(m_bUnderline,m_bOverline,m_bStrikeout,m_bTopline,m_bBottomline);
	updatePreview();
}


void XAP_UnixDialog_FontChooser::overlineChanged(void)
{
	m_bOverline = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkOverline));
	m_bChangedOverline = !m_bChangedOverline;
	setFontDecoration(m_bUnderline,m_bOverline,m_bStrikeout,m_bTopline,m_bBottomline);
	updatePreview();
}


void XAP_UnixDialog_FontChooser::transparencyChanged(void)
{
	bool bTrans = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkTransparency));
	if(bTrans)
	{
		addOrReplaceVecProp("bgcolor","transparent");
		m_currentBGColor[RED]  = -1;
		m_currentBGColor[GREEN] = -1;
		m_currentBGColor[BLUE] = -1;
	}
	updatePreview();
}

void XAP_UnixDialog_FontChooser::fontRowChanged(void)
{
	static char szFontFamily[60];
	// this is used many times below to grab pointers to
	// strings inside list elements
	gchar * text[2] = {NULL, NULL};

	GList * selectedRow = NULL;
	gint rowNumber = 0;

	selectedRow = GTK_CLIST(m_fontList)->selection;
	if (selectedRow)
	{
		rowNumber = GPOINTER_TO_INT(selectedRow->data);
		gtk_clist_get_text(GTK_CLIST(m_fontList), rowNumber, 0, text);
		UT_ASSERT(text && text[0]);
		g_snprintf(szFontFamily, 50, "%s",text[0]);
		addOrReplaceVecProp("font-family",(XML_Char*)szFontFamily);
	}
	updatePreview();
}

void XAP_UnixDialog_FontChooser::styleRowChanged(void)
{
	// this is used many times below to grab pointers to
	// strings inside list elements
	gchar * text[2] = {NULL, NULL};

	GList * selectedRow = NULL;
	gint rowNumber = 0;
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
			addOrReplaceVecProp("font-style","normal");
			addOrReplaceVecProp("font-weight","normal");
		}
		else if (rowNumber == LIST_STYLE_BOLD)
		{
			addOrReplaceVecProp("font-style","normal");
			addOrReplaceVecProp("font-weight","bold");
		}
		else if (rowNumber == LIST_STYLE_ITALIC)
		{
			addOrReplaceVecProp("font-style","italic");
			addOrReplaceVecProp("font-weight","normal");
		}
		else if (rowNumber == LIST_STYLE_BOLD_ITALIC)
		{
			addOrReplaceVecProp("font-style","italic");
			addOrReplaceVecProp("font-weight","bold");
		}
		else
		{
			UT_ASSERT(0);
		}
	}
	updatePreview();
}


void XAP_UnixDialog_FontChooser::sizeRowChanged(void)
{
	// used similarly to convert between text and numeric arguments
	static char szFontSize[50];
	// this is used many times below to grab pointers to
	// strings inside list elements
	gchar * text[2] = {NULL, NULL};

	GList * selectedRow = NULL;
	gint rowNumber = 0;
	selectedRow = GTK_CLIST(m_sizeList)->selection;
	if (selectedRow)
	{
		rowNumber = GPOINTER_TO_INT(selectedRow->data);
		gtk_clist_get_text(GTK_CLIST(m_sizeList), rowNumber, 0, text);
		UT_ASSERT(text && text[0]);

		g_snprintf(szFontSize, 50, "%spt",
				   (XML_Char *)XAP_EncodingManager::fontsizes_mapping.lookupByTarget(text[0]));

//		g_snprintf(szFontSize, 50, "%spt",(UT_convertToPoints(text[0])));
//		g_snprintf(szFontSize, 50, "%spt",text[0]);

		addOrReplaceVecProp("font-size",(XML_Char *)szFontSize);
	}
	updatePreview();
}

void XAP_UnixDialog_FontChooser::fgColorChanged(void)
{
	static char buf_color[8];
	gtk_color_selection_get_color(GTK_COLOR_SELECTION(m_colorSelector), m_currentFGColor);

	// test for funkyColor-has-been-changed-to-sane-color case
	if (m_currentFGColor[RED] >= 0 &&
		m_currentFGColor[GREEN] >= 0 &&
		m_currentFGColor[BLUE] >= 0)
	{
		sprintf(buf_color, "%02x%02x%02x",
				(unsigned int) (m_currentFGColor[RED] 	* (gdouble) 255.0),
				(unsigned int) (m_currentFGColor[GREEN]	* (gdouble) 255.0),
				(unsigned int) (m_currentFGColor[BLUE] 	* (gdouble) 255.0));
		addOrReplaceVecProp("color",(XML_Char *)buf_color);
	}
	updatePreview();
}


void XAP_UnixDialog_FontChooser::bgColorChanged(void)
{
	static char buf_color[8];
	gtk_color_selection_get_color(GTK_COLOR_SELECTION(m_bgcolorSelector), m_currentBGColor);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkTransparency), FALSE);
	// test for funkyColor-has-been-changed-to-sane-color case
	if (m_currentBGColor[RED] >= 0 &&
		m_currentBGColor[GREEN] >= 0 &&
		m_currentBGColor[BLUE] >= 0)
	{
		sprintf(buf_color, "%02x%02x%02x",
				(unsigned int) (m_currentBGColor[RED] 	* (gdouble) 255.0),
				(unsigned int) (m_currentBGColor[GREEN]	* (gdouble) 255.0),
				(unsigned int) (m_currentBGColor[BLUE] 	* (gdouble) 255.0));

		addOrReplaceVecProp("bgcolor",(XML_Char *)buf_color);
	}
	updatePreview();
}

GtkWidget * XAP_UnixDialog_FontChooser::constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkWidget *windowFontSelection;
	GtkWidget *vboxMain;
	GtkWidget *vboxOuter;

	windowFontSelection = abiDialogNew ( FALSE, pSS->getValue(XAP_STRING_ID_DLG_UFS_FontTitle) ) ;

	vboxOuter = GTK_DIALOG(windowFontSelection)->vbox;

	vboxMain = constructWindowContents(vboxOuter);
	gtk_box_pack_start (GTK_BOX (vboxOuter), vboxMain, TRUE, TRUE, 0);

	abiAddStockButton ( GTK_DIALOG(windowFontSelection), GTK_STOCK_OK, BUTTON_OK ) ;
	abiAddStockButton ( GTK_DIALOG(windowFontSelection), GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;

	return windowFontSelection;
}

// Glade generated dialog, using fixed widgets to closely match
// the Windows layout, with some changes for color selector
GtkWidget * XAP_UnixDialog_FontChooser::constructWindowContents(GtkWidget *parent)
{
	GtkWidget *vboxMain;
	GtkWidget *notebookMain;
	GtkWidget *labelFont;
	GtkWidget *labelStyle;
	GtkWidget *listFonts;
	GtkWidget *labelSize;
	GtkWidget *frameEffects;
	GtkWidget *hboxDecorations;
	GtkWidget *checkbuttonStrikeout;
	GtkWidget *checkbuttonUnderline;
	GtkWidget *checkbuttonOverline;
//	GtkWidget *labelEncoding;
//	GtkWidget *comboEncoding;
	GtkWidget *listStyles;
	GtkWidget *listSizes;
	GtkWidget *hbox1;
	GtkWidget *colorSelector;
	GtkWidget *colorBGSelector;
	GtkWidget *labelTabFont;
	GtkWidget *labelTabColor;
	GtkWidget *labelTabBGColor;
	GtkWidget *frame4;

	/*
	  GtkWidget *fixedFont;
	  GtkWidget *frameStyle;
	  GtkWidget *frameFonts;
	  GtkWidget *frameSize;
	  GtkWidget *fixedColor;
	*/

	// the entry is a special drawing area full of one
	// of our graphics contexts
	GtkWidget *entryArea;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	vboxMain = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vboxMain);

	notebookMain = gtk_notebook_new ();
	gtk_widget_show (notebookMain);
	gtk_box_pack_start (GTK_BOX (vboxMain), notebookMain, 1, 1, 0);
	gtk_container_set_border_width (GTK_CONTAINER (notebookMain), 8);

	GtkWidget *window1 = parent;
  	GtkWidget *table1;
  	GtkWidget *vbox1;

 	GtkWidget *scrolledwindow1;
  	GtkWidget *vbox2;
  	GtkWidget *scrolledwindow2;
  	GtkWidget *vbox3;
  	GtkWidget *scrolledwindow3;
  	GtkWidget *vboxmisc;
//  	GtkWidget *hboxForEncoding;
	table1 = gtk_table_new (2, 3, FALSE);
	gtk_widget_set_name (table1, "table1");
	gtk_widget_ref (table1);
	g_object_set_data_full (G_OBJECT (window1), "table1", table1,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (table1);

    // Label for first page of the notebook
	labelTabFont = gtk_label_new (pSS->getValue(XAP_STRING_ID_DLG_UFS_FontTab));
	gtk_widget_show (labelTabFont);
//
// Make first page of the notebook
//
    gtk_notebook_append_page(GTK_NOTEBOOK(notebookMain), table1,labelTabFont);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_set_name (vbox1, "vbox1");
	gtk_widget_ref (vbox1);
	g_object_set_data_full (G_OBJECT (window1), "vbox1", vbox1,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (vbox1);
	gtk_table_attach (GTK_TABLE (table1), vbox1, 0, 1, 0, 2,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	labelFont = gtk_label_new (pSS->getValue(XAP_STRING_ID_DLG_UFS_FontLabel));
	gtk_widget_set_name (labelFont, "labelFont");
	gtk_widget_ref (labelFont);
	g_object_set_data_full (G_OBJECT (window1), "labelFont", labelFont,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelFont);
	gtk_box_pack_start (GTK_BOX (vbox1), labelFont, FALSE, FALSE, 0);

	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_name (scrolledwindow1, "scrolledwindow1");
	gtk_widget_ref (scrolledwindow1);
	g_object_set_data_full (G_OBJECT (window1), "scrolledwindow1", scrolledwindow1,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (scrolledwindow1);
	gtk_box_pack_start (GTK_BOX (vbox1), scrolledwindow1, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow1), 3);

	listFonts = gtk_clist_new (1);
	gtk_widget_set_name (listFonts, "listFonts");
	gtk_widget_ref (listFonts);
	g_object_set_data_full (G_OBJECT (window1), "listFonts", listFonts,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (listFonts);
	gtk_container_add (GTK_CONTAINER (scrolledwindow1), listFonts);
	gtk_clist_set_column_auto_resize (GTK_CLIST(listFonts), 0, TRUE);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_set_name (vbox2, "vbox2");
	gtk_widget_ref (vbox2);
	g_object_set_data_full (G_OBJECT (window1), "vbox2", vbox2,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (vbox2);
	gtk_table_attach (GTK_TABLE (table1), vbox2, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (GTK_FILL), 0, 0);

	labelStyle = gtk_label_new (pSS->getValue(XAP_STRING_ID_DLG_UFS_StyleLabel));
	gtk_widget_set_name (labelStyle, "labelStyle");
	gtk_widget_ref (labelStyle);
	g_object_set_data_full (G_OBJECT (window1), "labelStyle", labelStyle,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelStyle);
	gtk_box_pack_start (GTK_BOX (vbox2), labelStyle, FALSE, FALSE, 0);

	scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_name (scrolledwindow2, "scrolledwindow2");
	gtk_widget_ref (scrolledwindow2);
	g_object_set_data_full (G_OBJECT (window1), "scrolledwindow2", scrolledwindow2,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (scrolledwindow2);
	gtk_box_pack_start (GTK_BOX (vbox2), scrolledwindow2, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow2), 3);

	listStyles = gtk_clist_new (1);
	gtk_widget_set_name (listStyles, "listStyles");
	gtk_widget_ref (listStyles);
	g_object_set_data_full (G_OBJECT (window1), "listStyles", listStyles,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (listStyles);
	gtk_container_add (GTK_CONTAINER (scrolledwindow2), listStyles);
	gtk_clist_set_column_auto_resize (GTK_CLIST(listStyles), 0, TRUE);

	vbox3 = gtk_vbox_new (FALSE, 0);
	gtk_widget_set_name (vbox3, "vbox3");
	gtk_widget_ref (vbox3);
	g_object_set_data_full (G_OBJECT (window1), "vbox3", vbox3,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (vbox3);
	gtk_table_attach (GTK_TABLE (table1), vbox3, 2, 3, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (GTK_FILL), 0, 0);

	labelSize = gtk_label_new (pSS->getValue(XAP_STRING_ID_DLG_UFS_SizeLabel));
	gtk_widget_set_name (labelSize, "labelSize");
	gtk_widget_ref (labelSize);
	g_object_set_data_full (G_OBJECT (window1), "labelSize", labelSize,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelSize);
	gtk_box_pack_start (GTK_BOX (vbox3), labelSize, FALSE, FALSE, 0);

	scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_name (scrolledwindow3, "scrolledwindow3");
	gtk_widget_ref (scrolledwindow3);
	g_object_set_data_full (G_OBJECT (window1), "scrolledwindow3", scrolledwindow3,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (scrolledwindow3);
	gtk_box_pack_start (GTK_BOX (vbox3), scrolledwindow3, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow3), 3);

	listSizes = gtk_clist_new (1);
	gtk_widget_set_name (listSizes, "listSizes");
	gtk_widget_ref (listSizes);
	g_object_set_data_full (G_OBJECT (window1), "listSizes", listSizes,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (listSizes);
	gtk_container_add (GTK_CONTAINER (scrolledwindow3), listSizes);
	gtk_clist_set_column_auto_resize (GTK_CLIST(listSizes), 0, TRUE);

	vboxmisc = gtk_vbox_new (FALSE, 0);
	gtk_widget_set_name (vboxmisc, "vboxmisc");
	gtk_widget_ref (vboxmisc);
	g_object_set_data_full (G_OBJECT (window1), "vboxmisc", vboxmisc,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (vboxmisc);
	gtk_table_attach (GTK_TABLE (table1), vboxmisc, 1, 3, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (GTK_FILL), 0, 0);

	frameEffects = gtk_frame_new (pSS->getValue(XAP_STRING_ID_DLG_UFS_EffectsFrameLabel));
	gtk_widget_show (frameEffects);
	gtk_box_pack_start(GTK_BOX (vboxmisc), frameEffects, 0,0, 2);

	hboxDecorations = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hboxDecorations);
	gtk_container_add (GTK_CONTAINER (frameEffects), hboxDecorations);

	checkbuttonStrikeout = gtk_check_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_UFS_StrikeoutCheck));
	gtk_container_border_width (GTK_CONTAINER (checkbuttonStrikeout), 5);
	gtk_widget_show (checkbuttonStrikeout);
	gtk_box_pack_start (GTK_BOX (hboxDecorations), checkbuttonStrikeout, TRUE, TRUE, 0);

	checkbuttonUnderline = gtk_check_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_UFS_UnderlineCheck));
	gtk_container_border_width (GTK_CONTAINER (checkbuttonUnderline), 5);
	gtk_widget_show (checkbuttonUnderline);
	gtk_box_pack_start (GTK_BOX (hboxDecorations), checkbuttonUnderline, TRUE, TRUE, 0);

	checkbuttonOverline = gtk_check_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_UFS_OverlineCheck));
	gtk_container_border_width (GTK_CONTAINER (checkbuttonOverline), 5);
	gtk_widget_show (checkbuttonOverline);
	gtk_box_pack_start (GTK_BOX (hboxDecorations), checkbuttonOverline, TRUE, TRUE, 0);


#if 0
	hboxForEncoding = gtk_hbox_new (FALSE, 0);
	gtk_widget_set_name (hboxForEncoding, "hboxForEncoding");
	gtk_widget_ref (hboxForEncoding);
	g_object_set_data_full (G_OBJECT (window1), "hboxForEncoding", hboxForEncoding,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hboxForEncoding);
	gtk_box_pack_start (GTK_BOX (vboxmisc), hboxForEncoding, TRUE, TRUE, 0);

	labelEncoding = gtk_label_new (pSS->getValue(XAP_STRING_ID_DLG_UFS_EncodingLabel));
	g_object_set_data (parent, "labelEncoding", labelEncoding);
	gtk_widget_show (labelEncoding);
	gtk_box_pack_start (GTK_BOX (hboxForEncoding), labelEncoding, 1,1, 2);
	gtk_label_set_justify (GTK_LABEL (labelEncoding), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelEncoding), 0, 0.5);

	comboEncoding = gtk_combo_new ();
	g_object_set_data (parent, "comboEncoding", comboEncoding);
	gtk_widget_show (comboEncoding);
	gtk_box_pack_start(GTK_BOX (hboxForEncoding), comboEncoding, 1, 1, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (GTK_COMBO (comboEncoding)->popup),
									GTK_POLICY_NEVER,
									GTK_POLICY_AUTOMATIC);
	GList * comboEncoding_items = NULL;
	comboEncoding_items = g_list_append (comboEncoding_items, (void *) "Placeholder");
//	comboEncoding_items = g_list_append (comboEncoding_items, "Canadian");
//	comboEncoding_items = g_list_append (comboEncoding_items, "British");
//	comboEncoding_items = g_list_append (comboEncoding_items, "Irish");
//	comboEncoding_items = g_list_append (comboEncoding_items, "Broken English");
//	comboEncoding_items = g_list_append (comboEncoding_items, "These Are Bogus");
	gtk_combo_set_popdown_strings (GTK_COMBO (comboEncoding), comboEncoding_items);
	g_list_free (comboEncoding_items);

#endif

	/* Notebook page for ForeGround Color Selector */

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);

    // Label for second page of the notebook

	labelTabColor = gtk_label_new (pSS->getValue(XAP_STRING_ID_DLG_UFS_ColorTab));
	gtk_widget_show (labelTabColor);

//
// Make second page of the notebook
//
    gtk_notebook_append_page(GTK_NOTEBOOK(notebookMain), hbox1,labelTabColor);

	colorSelector = gtk_color_selection_new ();
	gtk_widget_show (colorSelector);
	gtk_box_pack_start (GTK_BOX (hbox1), colorSelector, TRUE, TRUE, 0);


	/*Notebook page for Background Color Selector*/

	GtkWidget * vboxBG = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vboxBG);

    // Label for third page of the notebook

	labelTabBGColor = gtk_label_new (pSS->getValue(XAP_STRING_ID_DLG_UFS_BGColorTab));
	gtk_widget_show (labelTabBGColor);
//
// Make third page of the notebook
//
    gtk_notebook_append_page(GTK_NOTEBOOK(notebookMain), vboxBG,labelTabBGColor);

	colorBGSelector = gtk_color_selection_new ();
	gtk_widget_show (colorBGSelector);
	gtk_box_pack_start (GTK_BOX (vboxBG), colorBGSelector, TRUE, TRUE, 0);

//
// Make a toggle button to set hightlight color transparent
//
	GtkWidget * checkbuttonTrans = gtk_check_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_UFS_TransparencyCheck));
	gtk_widget_show (checkbuttonTrans);
	gtk_box_pack_start (GTK_BOX (vboxBG), checkbuttonTrans, TRUE, TRUE, 0);

	/* frame with preview */

	frame4 = gtk_frame_new (NULL);
	gtk_widget_show (frame4);
	gtk_box_pack_start (GTK_BOX (vboxMain), frame4, FALSE, FALSE, PREVIEW_BOX_BORDER_WIDTH_PIXELS);
	// setting the height takes into account the border applied on all
	// sides, so we need to double the single border width
	gtk_widget_set_usize (frame4, -1, PREVIEW_BOX_HEIGHT_PIXELS + (PREVIEW_BOX_BORDER_WIDTH_PIXELS * 2));
	gtk_container_border_width (GTK_CONTAINER (frame4), PREVIEW_BOX_BORDER_WIDTH_PIXELS);
	gtk_frame_set_shadow_type (GTK_FRAME (frame4), GTK_SHADOW_IN);

	entryArea = createDrawingArea ();
	gtk_widget_set_events(entryArea, GDK_EXPOSURE_MASK);
	g_signal_connect(G_OBJECT(entryArea), "expose_event",
					   G_CALLBACK(s_drawing_area_expose), NULL);
	gtk_widget_set_usize (entryArea, -1, PREVIEW_BOX_HEIGHT_PIXELS);
	gtk_widget_show (entryArea);
	gtk_container_add (GTK_CONTAINER (frame4), entryArea);


	// save out to members for callback and class access
    m_fontList = listFonts;
	m_styleList = listStyles;
	m_sizeList = listSizes;
	m_colorSelector = colorSelector;
	m_bgcolorSelector = colorBGSelector;
	m_preview = entryArea;
	m_checkStrikeOut = checkbuttonStrikeout;
	m_checkUnderline = checkbuttonUnderline;
	m_checkOverline = checkbuttonOverline;
	m_checkTransparency = checkbuttonTrans;

	// bind signals to things
	g_signal_connect(G_OBJECT(m_checkUnderline),
					   "toggled",
					   G_CALLBACK(s_underline_toggled),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_checkOverline),
					   "toggled",
					   G_CALLBACK(s_overline_toggled),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_checkStrikeOut),
					   "toggled",
					   G_CALLBACK(s_strikeout_toggled),
					   (gpointer) this);


	g_signal_connect(G_OBJECT(m_checkTransparency),
					   "toggled",
					   G_CALLBACK(s_transparency_toggled),
					   (gpointer) this);


	g_signal_connect(G_OBJECT(listFonts),
					   "select_row",
					   G_CALLBACK(s_select_row_font),
					   (gpointer) this);
	g_signal_connect(G_OBJECT(listStyles),
					   "select_row",
					   G_CALLBACK(s_select_row_style),
					   (gpointer) this);
	g_signal_connect(G_OBJECT(listSizes),
					   "select_row",
					   G_CALLBACK(s_select_row_size),
					   (gpointer) this);

#if ABI_GTK_DEPRECATED
	// we catch the color tab's wheel click event so we can do some nasty
	// trickery with the value slider, so that when the user first does
	// some wheel work, the value soars to 100%, instead of 0%, so the
	// color they get is not always black
	//
	// also, we must do connect_after() so we get the information before
	// any other native color selector events get the chance to change
	// things on us
	g_signal_connect_after(G_OBJECT(GTK_COLOR_SELECTION(colorSelector)->wheel_area),
			       "event",
			       G_CALLBACK(s_color_wheel_clicked),
			       (gpointer) GTK_COLOR_SELECTION(colorSelector)->wheel_area);
	
	g_signal_connect_after(G_OBJECT(GTK_COLOR_SELECTION(colorBGSelector)->wheel_area),
			       "event",
			       G_CALLBACK(s_color_wheel_clicked),
			       (gpointer) GTK_COLOR_SELECTION(colorSelector)->wheel_area);
#endif //ABI_GTK_DEPRECATED	


	// This is a catch-all color selector callback which catches any
	// real-time updating of the color so we can refresh our preview
	// text
	g_signal_connect(G_OBJECT(colorSelector),
			 "event",
			 G_CALLBACK(s_color_update),
			 (gpointer) this);

	g_signal_connect(G_OBJECT(colorBGSelector),
					   "event",
					   G_CALLBACK(s_bgcolor_update),
					   (gpointer) this);

	GTK_WIDGET_SET_FLAGS(listFonts, GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS(listStyles, GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS(listSizes, GTK_CAN_FOCUS);

	gchar * text[2] = {NULL, NULL};

	// update the styles list
	gtk_clist_freeze(GTK_CLIST(m_styleList));
	gtk_clist_clear(GTK_CLIST(m_styleList));
	text[0] = (gchar *) pSS->getValue(XAP_STRING_ID_DLG_UFS_StyleRegular); 		gtk_clist_append(GTK_CLIST(m_styleList), text);
	text[0] = (gchar *) pSS->getValue(XAP_STRING_ID_DLG_UFS_StyleItalic); 		gtk_clist_append(GTK_CLIST(m_styleList), text);
	text[0] = (gchar *) pSS->getValue(XAP_STRING_ID_DLG_UFS_StyleBold); 	   	gtk_clist_append(GTK_CLIST(m_styleList), text);
	text[0] = (gchar *) pSS->getValue(XAP_STRING_ID_DLG_UFS_StyleBoldItalic);  	gtk_clist_append(GTK_CLIST(m_styleList), text);
	gtk_clist_thaw(GTK_CLIST(m_styleList));

	gtk_clist_freeze(GTK_CLIST(m_sizeList));
	gtk_clist_clear(GTK_CLIST(m_sizeList));
	// TODO perhaps populate the list based on the selected font/style?
	{
		int sz = XAP_EncodingManager::fontsizes_mapping.size();
		for (int i = 0; i < sz; ++i)
		{
			text[0]=(char*)XAP_EncodingManager::fontsizes_mapping.nth2(i);
			gtk_clist_append(GTK_CLIST(m_sizeList), text);
	    }
	}
	gtk_clist_thaw(GTK_CLIST(m_sizeList));

	return vboxMain;
}

void XAP_UnixDialog_FontChooser::runModal(XAP_Frame * pFrame)
{
	m_pUnixFrame = (XAP_UnixFrame *)pFrame;

	// this is used many times below to grab pointers to
	// strings inside list elements
	gchar * text[2] = {NULL, NULL};
	// used similarly to convert between text and numeric arguments
	static char sizeString[50];

	// Set up our own color space so we work well on 8-bit
	// displays.

	// establish the font manager before dialog creation
#ifndef WITH_PANGO
	XAP_App * app = m_pUnixFrame->getApp();
	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (app);
	m_fontManager = unixapp->getFontManager();
#else
	XAP_App *pApp = m_pUnixFrame->getApp();
	m_gc = new GR_UnixGraphics(m_preview->window, pApp);
	m_fontManager = m_gc->getFontManager();
#endif

	// build the dialog
	GtkWidget * cf = constructWindow();

	// freeze updates of the preview
	m_blockUpdate = true;

	// to sort out dupes
	UT_StringPtrMap fontHash(256);

	gtk_clist_freeze(GTK_CLIST(m_fontList));
	gtk_clist_clear(GTK_CLIST(m_fontList));

	// throw them in the hash save duplicates
#ifndef WITH_PANGO
	UT_Vector * fonts = m_fontManager->getAllFonts();
	for (UT_uint32 i = 0; i < fonts->size(); i++)
	{
		XAP_UnixFont * pFont = (XAP_UnixFont *)fonts->getNthItem(i);
		const char * fName = pFont->getName();
#else
	for(UT_uint32 i = 0;
		i < m_fontManager->getAvailableFontFamiliesCount();
		i++)
	{
		const char * fName = m_fontManager->getNthAvailableFontFamily(i);
#endif

		if (!fontHash.contains(fName, NULL))
		  {
		    fontHash.insert(fName,
				    (void *) fName);
		    text[0] = (gchar*)fName;
		    gtk_clist_append(GTK_CLIST(m_fontList), text);
		  }
	}

#ifndef WITH_PANGO
	DELETEP(fonts);
#endif

	gtk_clist_thaw(GTK_CLIST(m_fontList));

	// Set the defaults in the list boxes according to dialog data
	gint foundAt = 0;

	// is this safe with an XML_Char * string?
	foundAt = searchCList(GTK_CLIST(m_fontList), (char *) getVal("font-family"));

	if (foundAt >= 0)
	{
		gtk_clist_select_row(GTK_CLIST(m_fontList), foundAt, 0);
	}

	// this is pretty messy
	listStyle st = LIST_STYLE_NORMAL;
	if (!getVal("font-style") || !getVal("font-weight"))
	{
	        st = LIST_STYLE_NONE;
	}
	else if (!UT_stricmp(getVal("font-style"), "normal") &&
			 !UT_stricmp(getVal("font-weight"), "normal"))
	{
		st = LIST_STYLE_NORMAL;
	}
	else if (!UT_stricmp(getVal("font-style"), "normal") &&
			 !UT_stricmp(getVal("font-weight"), "bold"))
	{
		st = LIST_STYLE_BOLD;
	}
	else if (!UT_stricmp(getVal("font-style"), "italic") &&
			 !UT_stricmp(getVal("font-weight"), "normal"))
	{
		st = LIST_STYLE_ITALIC;
	}
	else if (!UT_stricmp(getVal("font-style"), "italic") &&
			 !UT_stricmp(getVal("font-weight"), "bold"))
	{
		st = LIST_STYLE_BOLD_ITALIC;
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	if (st != LIST_STYLE_NONE)
		gtk_clist_select_row(GTK_CLIST(m_styleList), st, 0);

	g_snprintf(sizeString, 60, "%s", std_size_string(UT_convertToPoints(getVal("font-size"))));
	foundAt = searchCList(GTK_CLIST(m_sizeList), (char *)XAP_EncodingManager::fontsizes_mapping.lookupBySource(sizeString));

	if (foundAt >= 0)
	{
		gtk_clist_select_row(GTK_CLIST(m_sizeList), foundAt, 0);
	}

	// Set color in the color selector
	if (getVal("color"))
	{
		UT_RGBColor c;
		UT_parseColor(getVal("color"), c);

		m_currentFGColor[RED] = ((gdouble) c.m_red / (gdouble) 255.0);
		m_currentFGColor[GREEN] = ((gdouble) c.m_grn / (gdouble) 255.0);
		m_currentFGColor[BLUE] = ((gdouble) c.m_blu / (gdouble) 255.0);

		gtk_color_selection_set_color(GTK_COLOR_SELECTION(m_colorSelector), m_currentFGColor);
	}
	else
	{
		// if we have no color, use a placeholder of funky values
		// the user can't pick interactively.  This catches ALL
		// the cases except where the user specifically enters -1 for
		// all Red, Green and Blue attributes manually.  This user
		// should expect it not to touch the color.  :)
		gtk_color_selection_set_color(GTK_COLOR_SELECTION(m_colorSelector), m_funkyColor);
	}

	// Set color in the color selector
	const XML_Char * pszBGCol = getVal("bgcolor");
	if (pszBGCol && strcmp(pszBGCol,"transparent") != 0)
	{
		UT_RGBColor c;
		UT_parseColor(getVal("bgcolor"), c);

		m_currentBGColor[RED] = ((gdouble) c.m_red / (gdouble) 255.0);
		m_currentBGColor[GREEN] = ((gdouble) c.m_grn / (gdouble) 255.0);
		m_currentBGColor[BLUE] = ((gdouble) c.m_blu / (gdouble) 255.0);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkTransparency), FALSE);
		gtk_color_selection_set_color(GTK_COLOR_SELECTION(m_bgcolorSelector), m_currentBGColor);
	}
	else
	{
		// if we have no color, use a placeholder of funky values
		// the user can't pick interactively.  This catches ALL
		// the cases except where the user specifically enters -1 for
		// all Red, Green and Blue attributes manually.  This user
		// should expect it not to touch the color.  :)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkTransparency), TRUE);
		gtk_color_selection_set_color(GTK_COLOR_SELECTION(m_bgcolorSelector), m_funkyColor);
	}

	// set the strikeout and underline check buttons
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkStrikeOut), m_bStrikeout);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkUnderline), m_bUnderline);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkOverline), m_bOverline);

	m_doneFirstFont = true;

	// attach a new graphics context
#ifndef WITH_PANGO
	XAP_App *pApp = pFrame->getApp();
	m_gc = new GR_UnixGraphics(m_preview->window, m_fontManager, pApp);
#endif
	_createFontPreviewFromGC(m_gc,m_preview->allocation.width,m_preview->allocation.height);
//
// This enables callbacks on the preview area with a widget pointer to
// access this dialog.
//
	gtk_object_set_user_data(GTK_OBJECT(m_preview), this);

	// unfreeze updates of the preview
	m_blockUpdate = false;
	// manually trigger an update
	updatePreview();

	switch ( abiRunModalDialog ( GTK_DIALOG(cf), pFrame, this, BUTTON_CANCEL, true ) )
	  {
	  case BUTTON_OK:
	    {
	      m_answer = a_OK;
	      break ;
	    }
	  default:
	    {
	      m_answer = a_CANCEL;
	      break;
	    }
	  }

	// these dialogs are cached around through the dialog framework,
	// and this variable needs to get set back
	m_doneFirstFont = false;

	UT_DEBUGMSG(("FontChooserEnd: Family[%s%s] Size[%s%s] Weight[%s%s] Style[%s%s] Color[%s%s] Underline[%d%s] StrikeOut[%d%s]\n",
				 ((getVal("font-family")) ? getVal("font-family") : ""),	((m_bChangedFontFamily) ? "(chg)" : ""),
				 ((getVal("font-size")) ? getVal("font-size") : ""),		((m_bChangedFontSize) ? "(chg)" : ""),
				 ((getVal("font-weight")) ? getVal("font-weight") : ""),	((m_bChangedFontWeight) ? "(chg)" : ""),
				 ((getVal("font-style")) ? getVal("font-style") : ""),		((m_bChangedFontStyle) ? "(chg)" : ""),
				 ((getVal("color")) ? getVal("color") : "" ),				((m_bChangedColor) ? "(chg)" : ""),
				 (m_bUnderline),							((m_bChangedUnderline) ? "(chg)" : ""),
				 (m_bStrikeout),							((m_bChangedStrikeOut) ? "(chg)" : "")));

	// answer should be set by the appropriate callback
	// the caller can get the answer from getAnswer().

	m_pUnixFrame = NULL;
}

void XAP_UnixDialog_FontChooser::updatePreview(void)
{
	// if we don't have anything yet, just ignore this request
	if (!m_gc)
		return;
	// if a font has been set since this dialog was launched, draw things with it
	if (m_doneFirstFont)
	{
	  const UT_UCSChar * entryString = getDrawString ();

	  if (!entryString)
		  return;

	  event_previewExposed(entryString);
	}
	else
	{
		event_previewClear();
	}
}





