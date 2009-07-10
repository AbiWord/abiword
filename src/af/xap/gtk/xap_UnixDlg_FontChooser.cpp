/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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

// 9/4/04 Updated to use GtkTreeView , Tim O'Brien (obrientimo@vuw.ac.nz)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <set>
#include <string>

#include <gtk/gtk.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_units.h"
#include "xap_UnixDialogHelper.h"
#include "xap_UnixDlg_FontChooser.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"
#include "xap_EncodingManager.h"
#include "xav_View.h"
#include "gr_UnixCairoGraphics.h"

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

//
// List store model , used as model for GtkTreeView
	enum
	{
		TEXT_COLUMN,
		N_COLUMNS
	};

static gint searchTreeView(GtkTreeView* tv, const char * compareText)
{
       GtkTreeModel* model;
       GtkTreeIter iter;
       char* text;

       UT_ASSERT(tv);

       // if text is null, it's not found
       if (!compareText)
               return -1;

       model = gtk_tree_view_get_model(GTK_TREE_VIEW(tv));
       if (! gtk_tree_model_get_iter_first(model, &iter) )
		return -1;

       gint i = 0;
       do {
       	       gtk_tree_model_get(model, &iter, TEXT_COLUMN, &text, -1);
	       if (!g_ascii_strcasecmp(text, compareText))
	                       return i;
	       i++;
       } while(gtk_tree_model_iter_next (model, &iter));

       return -1;
}

//
// Create GtkTreeView that is similar to a CList
// ie single text column, with no header
GtkWidget* createFontTabTreeView() 
{
	GtkWidget* treeView;
	GtkListStore* listStore;
	GtkTreeViewColumn* column;
	GtkCellRenderer* renderer;

	treeView = gtk_tree_view_new();
	listStore = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeView), GTK_TREE_MODEL(listStore));
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", TEXT_COLUMN, NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeView), FALSE);

	return treeView;
}


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
	m_fontList = NULL;
	m_styleList = NULL;
	m_sizeList = NULL;
	m_checkStrikeOut = NULL;
	m_checkUnderline = NULL;
	m_checkOverline = NULL;
	m_checkHidden = NULL;
	m_checkTransparency = NULL;
	m_checkSubScript = NULL;
	m_iSubScriptId = 0;
	m_checkSuperScript = NULL;
	m_iSuperScriptId = 0;
	m_colorSelector = NULL;
	m_bgcolorSelector = NULL;
	m_preview = NULL;

	m_gc = NULL;
	m_pFrame = NULL;
	m_doneFirstFont = false;

	memset(&m_currentFGColor, 0, sizeof(m_currentFGColor));
	memset(&m_currentBGColor, 0, sizeof(m_currentBGColor));
	m_currentBGColorTransparent = false;
	memset(&m_funkyColor, 0, sizeof(m_funkyColor));
}

XAP_UnixDialog_FontChooser::~XAP_UnixDialog_FontChooser(void)
{
	DELETEP(m_gc);
}


/*****************************************************************/

static gint s_color_update(GtkWidget * /* widget */,
			   XAP_UnixDialog_FontChooser * dlg)
{
	UT_return_val_if_fail(dlg,FALSE);
	dlg->fgColorChanged();
	return FALSE;
}

static gint s_bgcolor_update(GtkWidget * /* widget */,
						   XAP_UnixDialog_FontChooser * dlg)
{
	UT_return_val_if_fail(dlg,FALSE);
	dlg->bgColorChanged();
	return FALSE;
}

static void s_select_row_font(GtkTreeSelection * /* widget */, XAP_UnixDialog_FontChooser * dlg)
{
	UT_return_if_fail(dlg);
    // update the row number and show the changed preview
	// redisplay the preview text
	dlg->fontRowChanged();
}


static void s_select_row_style(GtkTreeSelection * /* widget */, XAP_UnixDialog_FontChooser * dlg)
{
	UT_return_if_fail(dlg);

	// redisplay the preview text
	dlg->styleRowChanged();
}

static void s_select_row_size(GtkTreeSelection * /* widget */, XAP_UnixDialog_FontChooser * dlg)
{
	UT_return_if_fail(dlg);

	// redisplay the preview text
	dlg->sizeRowChanged();
}

static gboolean s_drawing_area_expose(GtkWidget * w,
								  GdkEventExpose * /* pExposeEvent */)
{
	XAP_UnixDialog_FontChooser * dlg = 
		(XAP_UnixDialog_FontChooser *)g_object_get_data(G_OBJECT(w), "user-data");
	dlg->updatePreview();
//	g_idle_add(static_cast<GSourceFunc>(do_update),static_cast<gpointer>(dlg));
	return TRUE;
}

static void s_underline_toggled(GtkWidget * ,  XAP_UnixDialog_FontChooser * dlg)
{
	dlg->underlineChanged();
}


static void s_overline_toggled(GtkWidget * ,  XAP_UnixDialog_FontChooser * dlg)
{
	dlg->overlineChanged();
}


static void s_subscript_toggled(GtkWidget * ,  XAP_UnixDialog_FontChooser * dlg) 
{ 
    dlg->subscriptChanged(); 
} 
 
 
static void s_superscript_toggled(GtkWidget * ,  XAP_UnixDialog_FontChooser * dlg) 
{ 
    dlg->superscriptChanged(); 
} 
 
 
static void s_strikeout_toggled(GtkWidget * ,  XAP_UnixDialog_FontChooser * dlg)
{
	dlg->strikeoutChanged();
}

static void s_hidden_toggled(GtkWidget * ,  XAP_UnixDialog_FontChooser * dlg)
{
	dlg->hiddenChanged();
}


static void s_transparency_toggled(GtkWidget * ,  XAP_UnixDialog_FontChooser * dlg)
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

 
void XAP_UnixDialog_FontChooser::subscriptChanged(void) 
{ 
    m_bSubScript = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkSubScript)); 
    m_bChangedSubScript = !m_bChangedSubScript; 
    if (m_bSubScript)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkSuperScript)))
		{
			g_signal_handler_block(G_OBJECT(m_checkSuperScript), m_iSuperScriptId);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkSuperScript), false);
			g_signal_handler_unblock(G_OBJECT(m_checkSuperScript), m_iSuperScriptId);
			m_bChangedSuperScript = !m_bChangedSuperScript;
			setSuperScript(false);
		}
	}
    setSubScript(m_bSubScript); 
    updatePreview(); 
} 
 
void XAP_UnixDialog_FontChooser::superscriptChanged(void) 
{ 
    m_bSuperScript = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkSuperScript)); 
    m_bChangedSuperScript = !m_bChangedSuperScript; 
    if (m_bSuperScript)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkSubScript)))
		{
			g_signal_handler_block(G_OBJECT(m_checkSubScript), m_iSubScriptId);
    		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkSubScript), false);
			g_signal_handler_unblock(G_OBJECT(m_checkSubScript), m_iSubScriptId);
			m_bChangedSubScript = !m_bChangedSubScript;
			setSubScript(false);
		}
	}
    setSuperScript(m_bSuperScript); 
    updatePreview(); 
} 
 
 
void XAP_UnixDialog_FontChooser::hiddenChanged(void)
{
	m_bHidden = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkHidden));
	m_bChangedHidden = !m_bChangedHidden;
}

void XAP_UnixDialog_FontChooser::transparencyChanged(void)
{
	bool bTrans = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkTransparency));
	if(bTrans)
	{
		addOrReplaceVecProp("bgcolor","transparent");
		m_currentBGColorTransparent = true;
	}
	updatePreview();
}

void XAP_UnixDialog_FontChooser::textTransformChanged(void)
{
#if 0
	// todo
	static char szTextTransform[60];
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *text;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_fontList));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_fontList));
	if ( gtk_tree_selection_get_selected (selection, &model, &iter) )
	{
		gtk_tree_model_get(model, &iter, TEXT_COLUMN, &text, -1);
		g_snprintf(szTextTransform, 50, "%s",text);
		g_free(text), text = NULL;
		addOrReplaceVecProp("text-transform",static_cast<gchar*>(szTextTransform));
	}
#endif

	updatePreview();
}

void XAP_UnixDialog_FontChooser::fontRowChanged(void)
{
	static char szFontFamily[60];
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *text;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_fontList));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_fontList));
	if ( gtk_tree_selection_get_selected (selection, &model, &iter) )
	{
		gtk_tree_model_get(model, &iter, TEXT_COLUMN, &text, -1);
		g_snprintf(szFontFamily, 50, "%s",text);
		g_free(text), text = NULL;
		addOrReplaceVecProp("font-family",static_cast<gchar*>(szFontFamily));
	}

	updatePreview();
}

void XAP_UnixDialog_FontChooser::styleRowChanged(void)
{
	GtkTreeSelection* selection;
	GtkTreeModel* model;
	GtkTreeIter iter;
	gint rowNumber;
	GtkTreePath* path;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_styleList));
	if ( gtk_tree_selection_get_selected (selection, &model, &iter) )
	{
		path = gtk_tree_model_get_path(model, &iter);
		rowNumber = gtk_tree_path_get_indices(path)[0];
		gtk_tree_path_free(path);

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
			UT_ASSERT_HARMLESS(0);
		}
	}
	updatePreview();
}


void XAP_UnixDialog_FontChooser::sizeRowChanged(void)
{
	// used similarly to convert between text and numeric arguments
	static char szFontSize[50];
	GtkTreeSelection* selection;
	GtkTreeModel* model;
	GtkTreeIter iter;
	gchar* text;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_sizeList));
	if ( gtk_tree_selection_get_selected (selection, &model, &iter) )
	{
		gtk_tree_model_get(model, &iter, TEXT_COLUMN, &text, -1);
		UT_ASSERT(text);
		g_snprintf(szFontSize, 50, "%spt",
				   static_cast<const gchar *>(XAP_EncodingManager::fontsizes_mapping.lookupByTarget(text)));
		g_free(text), text = NULL;
		addOrReplaceVecProp("font-size",static_cast<gchar *>(szFontSize));
	}
	updatePreview();
}

void XAP_UnixDialog_FontChooser::fgColorChanged(void)
{
	gtk_color_selection_get_current_color (GTK_COLOR_SELECTION(m_colorSelector), &m_currentFGColor);
	UT_RGBColor * rgbcolor = UT_UnixGdkColorToRGBColor(m_currentFGColor);
	UT_HashColor hash_color;
	const char * c = hash_color.setColor(*rgbcolor);
	addOrReplaceVecProp("color",  c + 1);
	delete rgbcolor;
	updatePreview();
}


void XAP_UnixDialog_FontChooser::bgColorChanged(void)
{
	gtk_color_selection_get_current_color (GTK_COLOR_SELECTION(m_bgcolorSelector), &m_currentBGColor);
	UT_RGBColor * rgbcolor = UT_UnixGdkColorToRGBColor(m_currentBGColor);
	UT_HashColor hash_color;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkTransparency), FALSE);
	m_currentBGColorTransparent = false;
	// test for funkyColor-has-been-changed-to-sane-color case
	addOrReplaceVecProp("bgcolor", hash_color.setColor(*rgbcolor) + 1);
	delete rgbcolor;
	updatePreview();
}

GtkWidget * XAP_UnixDialog_FontChooser::constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkWidget *windowFontSelection;
	GtkWidget *vboxMain;
	GtkWidget *vboxOuter;

	UT_UTF8String s;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_FontTitle,s);
	windowFontSelection = abiDialogNew ( "font dialog", TRUE, s.utf8_str() ) ;

	vboxOuter = GTK_DIALOG(windowFontSelection)->vbox;

	vboxMain = constructWindowContents(vboxOuter);
	gtk_box_pack_start (GTK_BOX (vboxOuter), vboxMain, TRUE, TRUE, 0);

	abiAddStockButton ( GTK_DIALOG(windowFontSelection), GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;
	abiAddStockButton ( GTK_DIALOG(windowFontSelection), GTK_STOCK_OK, BUTTON_OK ) ;

	return windowFontSelection;
}

// GtkBuilder generated dialog, using fixed widgets to closely match
// the Windows layout, with some changes for color selector
GtkWidget * XAP_UnixDialog_FontChooser::constructWindowContents(GtkWidget *parent)
{
	GtkTreeSelection *selection;
	GtkWidget *vboxMain;
	GtkWidget *notebookMain;
	GtkWidget *labelFont;
	GtkWidget *labelStyle;
	GtkWidget *listFonts;
	GtkWidget *labelSize;
	GtkWidget *frameEffects;
	GtkWidget *vboxEffectRows;
	GtkWidget *hboxDecorations;
	GtkWidget *hboxAdvDecorations;
	GtkWidget *checkbuttonStrikeout;
	GtkWidget *checkbuttonUnderline;
	GtkWidget *checkbuttonOverline;
	GtkWidget *checkbuttonHidden;
	GtkWidget *checkbuttonSubscript;
	GtkWidget *checkbuttonSuperscript;
 	GtkWidget *listStyles;
	GtkWidget *listSizes;
	GtkWidget *hbox1;
	GtkWidget *colorSelector;
	GtkWidget *colorBGSelector;
	GtkWidget *labelTabFont;
	GtkWidget *labelTabColor;
	GtkWidget *labelTabBGColor;
	GtkWidget *frame4;

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
	g_object_ref (G_OBJECT(table1));
	g_object_set_data_full (G_OBJECT (window1), "table1", table1,
							  reinterpret_cast<GDestroyNotify>(g_object_unref));
	gtk_widget_show (table1);

	UT_UTF8String s;
	// Label for first page of the notebook
	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_FontTab,s);
	labelTabFont = gtk_label_new (s.utf8_str());
	gtk_widget_show (labelTabFont);
//
// Make first page of the notebook
//
	gtk_notebook_append_page(GTK_NOTEBOOK(notebookMain), table1,labelTabFont);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_set_name (vbox1, "vbox1");
	g_object_ref (G_OBJECT(vbox1));
	g_object_set_data_full (G_OBJECT (window1), "vbox1", vbox1,
							  reinterpret_cast<GDestroyNotify>(g_object_unref));
	gtk_widget_show (vbox1);
	gtk_table_attach (GTK_TABLE (table1), vbox1, 0, 1, 0, 2,
					  static_cast<GtkAttachOptions>(GTK_EXPAND | GTK_FILL),
					  static_cast<GtkAttachOptions>(GTK_EXPAND | GTK_FILL), 0, 0);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_FontLabel,s);
	labelFont = gtk_label_new (s.utf8_str());
	gtk_widget_set_name (labelFont, "labelFont");
	g_object_ref (labelFont);
	g_object_set_data_full (G_OBJECT (window1), "labelFont", labelFont,
							  reinterpret_cast<GDestroyNotify>(g_object_unref));
	gtk_widget_show (labelFont);
	gtk_box_pack_start (GTK_BOX (vbox1), labelFont, FALSE, FALSE, 0);

	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_name (scrolledwindow1, "scrolledwindow1");
	g_object_ref (scrolledwindow1);
	g_object_set_data_full (G_OBJECT (window1), "scrolledwindow1", scrolledwindow1,
							  reinterpret_cast<GDestroyNotify>(g_object_unref));
	gtk_widget_show (scrolledwindow1);
	gtk_box_pack_start (GTK_BOX (vbox1), scrolledwindow1, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow1), 3);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow1), GTK_SHADOW_IN);

	listFonts = createFontTabTreeView();
	gtk_widget_set_name (listFonts, "listFonts");
	g_object_ref (listFonts);
	g_object_set_data_full (G_OBJECT (window1), "listFonts", listFonts,
							  reinterpret_cast<GDestroyNotify>(g_object_unref));
	gtk_widget_show (listFonts);
	gtk_container_add (GTK_CONTAINER (scrolledwindow1), listFonts);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_set_name (vbox2, "vbox2");
	g_object_ref (vbox2);
	g_object_set_data_full (G_OBJECT (window1), "vbox2", vbox2,
							  reinterpret_cast<GDestroyNotify>(g_object_unref));
	gtk_widget_show (vbox2);
	gtk_table_attach (GTK_TABLE (table1), vbox2, 1, 2, 0, 1,
					  static_cast<GtkAttachOptions>(GTK_FILL),
					  static_cast<GtkAttachOptions>(GTK_FILL), 0, 0);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_StyleLabel,s);
	labelStyle = gtk_label_new (s.utf8_str());
	gtk_widget_set_name (labelStyle, "labelStyle");
	g_object_ref (labelStyle);
	g_object_set_data_full (G_OBJECT (window1), "labelStyle", labelStyle,
							  reinterpret_cast<GDestroyNotify>(g_object_unref));
	gtk_widget_show (labelStyle);
	gtk_box_pack_start (GTK_BOX (vbox2), labelStyle, FALSE, FALSE, 0);

	scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_name (scrolledwindow2, "scrolledwindow2");
	g_object_ref (scrolledwindow2);
	g_object_set_data_full (G_OBJECT (window1), "scrolledwindow2", scrolledwindow2,
							  reinterpret_cast<GDestroyNotify>(g_object_unref));
	gtk_widget_show (scrolledwindow2);
	gtk_box_pack_start (GTK_BOX (vbox2), scrolledwindow2, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow2), 3);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow2), GTK_SHADOW_IN);

	listStyles = createFontTabTreeView();
	gtk_widget_set_name (listStyles, "listStyles");
	g_object_ref (listStyles);
	g_object_set_data_full (G_OBJECT (window1), "listStyles", listStyles,
							  reinterpret_cast<GDestroyNotify>(g_object_unref));
	gtk_widget_show (listStyles);
	gtk_container_add (GTK_CONTAINER (scrolledwindow2), listStyles);

	vbox3 = gtk_vbox_new (FALSE, 0);
	gtk_widget_set_name (vbox3, "vbox3");
	g_object_ref (vbox3);
	g_object_set_data_full (G_OBJECT (window1), "vbox3", vbox3,
							  reinterpret_cast<GDestroyNotify>(g_object_unref));
	gtk_widget_show (vbox3);
	gtk_table_attach (GTK_TABLE (table1), vbox3, 2, 3, 0, 1,
					  static_cast<GtkAttachOptions>(GTK_FILL),
					  static_cast<GtkAttachOptions>(GTK_FILL), 0, 0);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_SizeLabel,s);
	labelSize = gtk_label_new (s.utf8_str());
	gtk_widget_set_name (labelSize, "labelSize");
	g_object_ref (labelSize);
	g_object_set_data_full (G_OBJECT (window1), "labelSize", labelSize,
							  reinterpret_cast<GDestroyNotify>(g_object_unref));
	gtk_widget_show (labelSize);
	gtk_box_pack_start (GTK_BOX (vbox3), labelSize, FALSE, FALSE, 0);

	scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_name (scrolledwindow3, "scrolledwindow3");
	g_object_ref (scrolledwindow3);
	g_object_set_data_full (G_OBJECT (window1), "scrolledwindow3", scrolledwindow3,
							  reinterpret_cast<GDestroyNotify>(g_object_unref));
	gtk_widget_show (scrolledwindow3);
	gtk_box_pack_start (GTK_BOX (vbox3), scrolledwindow3, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow3), 3);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow3), GTK_SHADOW_IN);

	listSizes = createFontTabTreeView();
	gtk_widget_set_name (listSizes, "listSizes");
	g_object_ref (listSizes);
	g_object_set_data_full (G_OBJECT (window1), "listSizes", listSizes,
							  reinterpret_cast<GDestroyNotify>(g_object_unref));
	gtk_widget_show (listSizes);
	gtk_container_add (GTK_CONTAINER (scrolledwindow3), listSizes);

	vboxmisc = gtk_vbox_new (FALSE, 0);
	gtk_widget_set_name (vboxmisc, "vboxmisc");
	g_object_ref (vboxmisc);
	g_object_set_data_full (G_OBJECT (window1), "vboxmisc", vboxmisc,
							  reinterpret_cast<GDestroyNotify>(g_object_unref));
	gtk_widget_show (vboxmisc);
	gtk_table_attach (GTK_TABLE (table1), vboxmisc, 1, 3, 1, 2,
					  static_cast<GtkAttachOptions>(GTK_FILL),
					  static_cast<GtkAttachOptions>(GTK_FILL), 0, 0);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_EffectsFrameLabel,s);
	frameEffects = gtk_frame_new (s.utf8_str());
	gtk_frame_set_shadow_type(GTK_FRAME(frameEffects), GTK_SHADOW_NONE);
	gtk_widget_show (frameEffects);
	gtk_box_pack_start(GTK_BOX (vboxmisc), frameEffects, 0,0, 2);

	vboxEffectRows = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vboxEffectRows);
	gtk_container_add (GTK_CONTAINER (frameEffects), vboxEffectRows);

	hboxDecorations = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hboxDecorations);
	gtk_box_pack_start (GTK_BOX (vboxEffectRows), hboxDecorations, FALSE, FALSE, 0);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_StrikeoutCheck,s);
	checkbuttonStrikeout = gtk_check_button_new_with_label (s.utf8_str());
	gtk_container_set_border_width (GTK_CONTAINER (checkbuttonStrikeout), 5);
	gtk_widget_show (checkbuttonStrikeout);
	gtk_box_pack_start (GTK_BOX (hboxDecorations), checkbuttonStrikeout, TRUE, TRUE, 0);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_UnderlineCheck,s);
	checkbuttonUnderline = gtk_check_button_new_with_label (s.utf8_str());
	gtk_container_set_border_width (GTK_CONTAINER (checkbuttonUnderline), 5);
	gtk_widget_show (checkbuttonUnderline);
	gtk_box_pack_start (GTK_BOX (hboxDecorations), checkbuttonUnderline, TRUE, TRUE, 0);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_OverlineCheck,s);
	checkbuttonOverline = gtk_check_button_new_with_label (s.utf8_str());
	gtk_container_set_border_width (GTK_CONTAINER (checkbuttonOverline), 5);
	gtk_widget_show (checkbuttonOverline);
	gtk_box_pack_start (GTK_BOX (hboxDecorations), checkbuttonOverline, TRUE, TRUE, 0);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_HiddenCheck,s);
	checkbuttonHidden = gtk_check_button_new_with_label (s.utf8_str());
	gtk_container_set_border_width (GTK_CONTAINER (checkbuttonHidden), 5);
	gtk_widget_show (checkbuttonHidden);
	gtk_box_pack_start (GTK_BOX (hboxDecorations), checkbuttonHidden, TRUE, TRUE, 0);

	/* subscript/superscript */

	hboxAdvDecorations = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hboxAdvDecorations);
	gtk_box_pack_start (GTK_BOX (vboxEffectRows), hboxAdvDecorations, FALSE, FALSE, 0);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_SubScript,s);
	checkbuttonSubscript = gtk_check_button_new_with_label (s.utf8_str());
	gtk_container_set_border_width (GTK_CONTAINER (checkbuttonSubscript), 5);
	gtk_widget_show (checkbuttonSubscript);
	gtk_box_pack_start (GTK_BOX (hboxAdvDecorations), checkbuttonSubscript, TRUE, TRUE, 0);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_SuperScript,s);
	checkbuttonSuperscript = gtk_check_button_new_with_label (s.utf8_str());
	gtk_container_set_border_width (GTK_CONTAINER (checkbuttonSuperscript), 5);
	gtk_widget_show (checkbuttonSuperscript);
	gtk_box_pack_start (GTK_BOX (hboxAdvDecorations), checkbuttonSuperscript, TRUE, TRUE, 0);

	/* Notebook page for ForeGround Color Selector */

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);

    // Label for second page of the notebook

	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_ColorTab,s);
	labelTabColor = gtk_label_new (s.utf8_str());
	gtk_widget_show (labelTabColor);

//
// Make second page of the notebook
//
    gtk_notebook_append_page(GTK_NOTEBOOK(notebookMain), hbox1,labelTabColor);

	colorSelector = gtk_color_selection_new ();
	gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colorSelector), FALSE);
	gtk_widget_show (colorSelector);
	gtk_box_pack_start (GTK_BOX (hbox1), colorSelector, TRUE, TRUE, 0);

	/*Notebook page for Background Color Selector*/

	GtkWidget * vboxBG = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vboxBG);

    // Label for third page of the notebook

	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_BGColorTab,s);
	labelTabBGColor = gtk_label_new (s.utf8_str());
	gtk_widget_show (labelTabBGColor);
//
// Make third page of the notebook
//
    gtk_notebook_append_page(GTK_NOTEBOOK(notebookMain), vboxBG,labelTabBGColor);

	colorBGSelector = gtk_color_selection_new ();
	gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colorBGSelector), FALSE);
	gtk_widget_show (colorBGSelector);
	gtk_box_pack_start (GTK_BOX (vboxBG), colorBGSelector, TRUE, TRUE, 0);

//
// Make a toggle button to set hightlight color transparent
//
	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_TransparencyCheck,s);
	GtkWidget * checkbuttonTrans = gtk_check_button_new_with_label (s.utf8_str());
	gtk_widget_show (checkbuttonTrans);
	gtk_box_pack_start (GTK_BOX (vboxBG), checkbuttonTrans, TRUE, TRUE, 0);

	/* frame with preview */

	frame4 = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame4), GTK_SHADOW_NONE);
	gtk_widget_show (frame4);
	gtk_box_pack_start (GTK_BOX (vboxMain), frame4, FALSE, FALSE, PREVIEW_BOX_BORDER_WIDTH_PIXELS);
	// setting the height takes into account the border applied on all
	// sides, so we need to double the single border width
	gtk_widget_set_size_request (frame4, -1, PREVIEW_BOX_HEIGHT_PIXELS + (PREVIEW_BOX_BORDER_WIDTH_PIXELS * 2));
	gtk_container_set_border_width (GTK_CONTAINER (frame4), PREVIEW_BOX_BORDER_WIDTH_PIXELS);

	entryArea = createDrawingArea ();
	gtk_widget_set_events(entryArea, GDK_EXPOSURE_MASK);
	g_signal_connect(G_OBJECT(entryArea), "expose_event",
					   G_CALLBACK(s_drawing_area_expose), NULL);
	gtk_widget_set_size_request (entryArea, -1, PREVIEW_BOX_HEIGHT_PIXELS);
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
	m_checkSubScript = checkbuttonSubscript;
	m_checkSuperScript = checkbuttonSuperscript;
	m_checkHidden = checkbuttonHidden;
	m_checkTransparency = checkbuttonTrans;

	// bind signals to things
	g_signal_connect(G_OBJECT(m_checkUnderline),
					   "toggled",
					   G_CALLBACK(s_underline_toggled),
					   static_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_checkOverline),
					   "toggled",
					   G_CALLBACK(s_overline_toggled),
					   static_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_checkStrikeOut),
					   "toggled",
					   G_CALLBACK(s_strikeout_toggled),
					   static_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_checkHidden),
					   "toggled",
					   G_CALLBACK(s_hidden_toggled),
					   static_cast<gpointer>(this));

	m_iSubScriptId = g_signal_connect(G_OBJECT(m_checkSubScript),
					   "toggled",
					   G_CALLBACK(s_subscript_toggled),
					   static_cast<gpointer>(this));

	m_iSuperScriptId = g_signal_connect(G_OBJECT(m_checkSuperScript),
					   "toggled",
					   G_CALLBACK(s_superscript_toggled),
					   static_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_checkTransparency),
					   "toggled",
					   G_CALLBACK(s_transparency_toggled),
					   static_cast<gpointer>(this));

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(listFonts));
	g_signal_connect(G_OBJECT(selection),
					   "changed",
					   G_CALLBACK(s_select_row_font),
					   static_cast<gpointer>(this));
	selection = NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(listStyles));
	g_signal_connect(G_OBJECT(selection),
					   "changed",
					   G_CALLBACK(s_select_row_style),
					   static_cast<gpointer>(this));
	selection = NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(listSizes));
	g_signal_connect(G_OBJECT(selection),
					   "changed",
					   G_CALLBACK(s_select_row_size),
					   static_cast<gpointer>(this));
	selection = NULL;

	// This is a catch-all color selector callback which catches any
	// real-time updating of the color so we can refresh our preview
	// text
	g_signal_connect(G_OBJECT(colorSelector),
			 "color-changed", //"event",
			 G_CALLBACK(s_color_update),
			 static_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(colorBGSelector),
			 "color-changed", //"event",
			 G_CALLBACK(s_bgcolor_update),
			 static_cast<gpointer>(this));

	GTK_WIDGET_SET_FLAGS(listFonts, GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS(listStyles, GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS(listSizes, GTK_CAN_FOCUS);

	// Make the tab focus list more sensible
	// font -> syle -> size -> other options ...
	GList* focusList = NULL;

	focusList = g_list_append(focusList, vbox1);
	focusList = g_list_append(focusList, vbox2);
	focusList = g_list_append(focusList, vbox3);
	focusList = g_list_append(focusList, vboxmisc);
	gtk_container_set_focus_chain(GTK_CONTAINER(table1), focusList);
	g_list_free(focusList);
	gtk_widget_grab_focus(scrolledwindow1);

	
	const gchar * text;
	GtkTreeModel* model;
	GtkTreeIter iter;

	// update the styles list
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_styleList));
	gtk_list_store_clear(GTK_LIST_STORE(model));
	
	text = pSS->getValue(XAP_STRING_ID_DLG_UFS_StyleRegular); 
	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, TEXT_COLUMN, text, -1);
	text = pSS->getValue(XAP_STRING_ID_DLG_UFS_StyleItalic);
	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, TEXT_COLUMN, text, -1);
	text = pSS->getValue(XAP_STRING_ID_DLG_UFS_StyleBold);
	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, TEXT_COLUMN, text, -1);
	text = pSS->getValue(XAP_STRING_ID_DLG_UFS_StyleBoldItalic);  
	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, TEXT_COLUMN, text, -1);



	model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_sizeList));
	gtk_list_store_clear(GTK_LIST_STORE(model));
	// TODO perhaps populate the list based on the selected font/style?
	{
		int sz = XAP_EncodingManager::fontsizes_mapping.size();
		for (int i = 0; i < sz; ++i)
		{
			text = XAP_EncodingManager::fontsizes_mapping.nth2(i);
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
			gtk_list_store_set(GTK_LIST_STORE(model), &iter, TEXT_COLUMN, text, -1);
	    }
	}

	return vboxMain;
}

void XAP_UnixDialog_FontChooser::runModal(XAP_Frame * pFrame)
{
	m_pFrame = static_cast<XAP_Frame *>(pFrame);

	// used similarly to convert between text and numeric arguments
	static char sizeString[50];

	// build the dialog
	GtkWidget * cf = constructWindow();

	// freeze updates of the preview
	m_blockUpdate = true;

	// to sort out dupes
    std::set<std::string> fontSet;

	GtkTreeModel* model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_fontList));
	gtk_list_store_clear(GTK_LIST_STORE(model));

	GR_GraphicsFactory * pGF = XAP_App::getApp()->getGraphicsFactory();
	if(!pGF)
	{
		return;
	}

	const std::vector<std::string> & names = GR_CairoGraphics::getAllFontNames();
	
	for (std::vector<std::string>::const_iterator  i = names.begin();
		 i != names.end(); ++i)
	{
		const std::string & fName = *i;
			
		if (fontSet.find(fName) == fontSet.end())
		{
            fontSet.insert(fName);

		    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		    gtk_list_store_set(GTK_LIST_STORE(model), &iter, TEXT_COLUMN, 
                               fName.c_str(), -1);
		    
		  }
	}

	// Set the defaults in the list boxes according to dialog data
	gint foundAt = 0;

	const std::string sFontFamily = getVal("font-family");
	foundAt = searchTreeView(GTK_TREE_VIEW(m_fontList), sFontFamily.c_str());

	// select and scroll to font name
	if (foundAt >= 0) {
		GtkTreePath* path = gtk_tree_path_new_from_indices(foundAt, -1);
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(m_fontList), path, NULL, FALSE);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(m_fontList), path, NULL, TRUE, 0.5 , 0.0);
		gtk_tree_path_free(path);
	}

	// this is pretty messy
	listStyle st = LIST_STYLE_NORMAL;
	const std::string sWeight = getVal("font-weight");
	const std::string sStyle = getVal("font-style");
	if (sStyle.empty() || sWeight.empty())
		st = LIST_STYLE_NONE;
	else {
		bool isBold = !g_ascii_strcasecmp(sWeight.c_str(), "bold");
		bool isItalic = !g_ascii_strcasecmp(sStyle.c_str(), "italic");
		if (!isBold && !isItalic) {
			st = LIST_STYLE_NORMAL;
		}
		else if (!isItalic && isBold) {
			st = LIST_STYLE_BOLD;
		}
		else if (isItalic && !isBold) {
			st = LIST_STYLE_ITALIC;
		}
		else if (isItalic && isBold) {
			st = LIST_STYLE_BOLD_ITALIC;
		}
		else {
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
	}

	// select and scroll to style name
	if (st != LIST_STYLE_NONE) {
		GtkTreePath* path = gtk_tree_path_new_from_indices(st, -1);
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(m_styleList), path, NULL, FALSE);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(m_styleList), path, NULL, TRUE, 0.5 , 0.0);
		gtk_tree_path_free(path);
	}

	g_snprintf(sizeString, 60, "%s", std_size_string(UT_convertToPoints(getVal("font-size").c_str())));
	foundAt = searchTreeView(GTK_TREE_VIEW(m_sizeList), 
				 XAP_EncodingManager::fontsizes_mapping.lookupBySource(sizeString));

	// select and scroll to size name
	if (foundAt >= 0) {
		GtkTreePath* path = gtk_tree_path_new_from_indices(foundAt, -1);
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(m_sizeList), path, NULL, FALSE);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(m_sizeList), path, NULL, TRUE, 0.5 , 0.0);
		gtk_tree_path_free(path);	
	}

	// Set color in the color selector
	const std::string sColor = getVal("color");
	if (!sColor.empty())
	{
		UT_RGBColor c;
		UT_parseColor(sColor.c_str(), c);

		GdkColor *color = UT_UnixRGBColorToGdkColor(c);
		m_currentFGColor = *color;
		gdk_color_free(color);
		gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(m_colorSelector), &m_currentFGColor);
	}
	else
	{
		// if we have no color, use a placeholder of funky values
		// the user can't pick interactively.  This catches ALL
		// the cases except where the user specifically enters -1 for
		// all Red, Green and Blue attributes manually.  This user
		// should expect it not to touch the color.  :)
		gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(m_colorSelector), &m_funkyColor);
	}

	// Set color in the color selector
	const std::string sBGCol = getVal("bgcolor");
	if (!sBGCol.empty() && strcmp(sBGCol.c_str(),"transparent") != 0)
	{
		UT_RGBColor c;
		UT_parseColor(sBGCol.c_str(), c);

		GdkColor *color = UT_UnixRGBColorToGdkColor(c);
		m_currentBGColor = *color;
		gdk_color_free(color);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkTransparency), FALSE);
		gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(m_bgcolorSelector), &m_currentBGColor);
	}
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkTransparency), TRUE);

	// fix for GTK's questionable gtk_toggle_set_active behaviour (emits when setting TRUE)
	m_bChangedStrikeOut = m_bStrikeout;
	m_bChangedUnderline = m_bUnderline;
	m_bChangedOverline = m_bOverline;
	m_bChangedHidden = m_bHidden;
	m_bChangedSubScript = m_bSubScript;
	m_bChangedSuperScript = m_bSuperScript;

	// set the strikeout, underline, overline, and hidden check buttons
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkStrikeOut), m_bStrikeout);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkUnderline), m_bUnderline);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkOverline), m_bOverline);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkHidden), m_bHidden);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkSubScript), m_bSubScript);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkSuperScript), m_bSuperScript);

	m_doneFirstFont = true;

	// attach a new graphics context
	gtk_widget_show ( cf ) ;
	
	GR_UnixCairoAllocInfo ai(m_preview->window);
	m_gc = (GR_CairoGraphics*) XAP_App::getApp()->newGraphics(ai);

	_createFontPreviewFromGC(m_gc,m_preview->allocation.width,m_preview->allocation.height);
//
// This enables callbacks on the preview area with a widget pointer to
// access this dialog.
//
	g_object_set_data(G_OBJECT(m_preview), "user-data", this);

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

	UT_DEBUGMSG(("FontChooserEnd: Family[%s%s] Size[%s%s] Weight[%s%s] Style[%s%s] Color[%s%s] Underline[%d%s] StrikeOut[%d%s] SubScript[%d%s] SuperScript[%d%s]\n",
				 getVal("font-family").c_str(),			((m_bChangedFontFamily) ? "(chg)" : ""),
				 getVal("font-size").c_str(),			((m_bChangedFontSize) ? "(chg)" : ""),
				 getVal("font-weight").c_str(),			((m_bChangedFontWeight) ? "(chg)" : ""),
				 getVal("font-style").c_str(),			((m_bChangedFontStyle) ? "(chg)" : ""),
				 getVal("color").c_str(),				((m_bChangedColor) ? "(chg)" : ""),
				 m_bUnderline,							((m_bChangedUnderline) ? "(chg)" : ""),
				 m_bStrikeout,							((m_bChangedStrikeOut) ? "(chg)" : ""),
				 m_bSubScript,							((m_bChangedSubScript) ? "(chg)" : ""),
				 m_bSuperScript,						((m_bChangedSuperScript) ? "(chg)" : "")
	            ));

	// answer should be set by the appropriate callback
	// the caller can get the answer from getAnswer().

	m_pFrame = NULL;
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
		event_previewClear();
}





