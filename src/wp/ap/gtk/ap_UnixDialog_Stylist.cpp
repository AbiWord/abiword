/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
 * Copyright (C) 2004 Martin Sevior
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <string.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "pt_PieceTable.h"

#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_Stylist.h"

static gint s_compare (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer /*userdata*/)
{
	GtkTreePath *path;
	gint depth, row1, row2, res;
	gchar *style1, *style2;

	path = gtk_tree_model_get_path(model, a);	
	depth = gtk_tree_path_get_depth(path);
	
	if (depth == 1)
	{
		gtk_tree_model_get(model, a, 1, &row1, -1);
		gtk_tree_model_get(model, b, 1, &row2, -1);
		
		res = row1 - row2;
	}
	else
	{
		gtk_tree_model_get(model, a, 0, &style1, -1);
		gtk_tree_model_get(model, b, 0, &style2, -1);
	
		res = g_utf8_collate(style1, style2);
	
		g_free(style1);
		g_free(style2);
	}
	
	gtk_tree_path_free(path);
	
	return res;
}

static void s_types_clicked(GtkTreeView *treeview,
                            AP_UnixDialog_Stylist * dlg)
{
	UT_ASSERT(treeview && dlg);

	GtkTreeSelection * selection;
	GtkTreeIter iter;
	GtkTreeModel * model;
	UT_sint32 row,col;

	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(treeview) );
	if (!selection || !gtk_tree_selection_get_selected (selection, &model, &iter)) {
		return;
	}

	// Get the row and col number
	GValue value;
	memset(&value, 0, sizeof(value));
	gtk_tree_model_get_value (model, &iter,1,&value);
	row = g_value_get_int(&value);
	g_value_unset (&value);
	gtk_tree_model_get_value (model, &iter,2,&value);
	col = g_value_get_int(&value);
	dlg->styleClicked(row,col);
}

static gboolean
tree_select_filter (GtkTreeSelection * /*selection*/, GtkTreeModel * /*model*/,
								  GtkTreePath *path, gboolean /*path_selected*/,
								  gpointer /*data*/)
{
	if (gtk_tree_path_get_depth (path) > 1)
		return TRUE;
	return FALSE;
}

static void s_types_dblclicked(GtkTreeView *treeview,
							   GtkTreePath * /*arg1*/,
							   GtkTreeViewColumn * /*arg2*/,
							   AP_UnixDialog_Stylist * me)
{
	// simulate the effects of a single click
	s_types_clicked (treeview, me);
	me->event_Apply ();
}

static void s_delete_clicked(GtkWidget * wid, AP_UnixDialog_Stylist * /*me*/ )
{
    abiDestroyWidget( wid ) ;// will emit signals for us
}

static void s_destroy_clicked(GtkWidget * /*wid*/, AP_UnixDialog_Stylist * me )
{
   me->event_Close();
}

static void s_response_triggered(GtkWidget * widget, gint resp, AP_UnixDialog_Stylist * dlg)
{
	UT_return_if_fail(widget && dlg);
	
	if ( resp == GTK_RESPONSE_APPLY )
	  dlg->event_Apply();
	else if ( resp == GTK_RESPONSE_CLOSE )
	  abiDestroyWidget(widget);
}

XAP_Dialog * AP_UnixDialog_Stylist::static_constructor(XAP_DialogFactory * pFactory,
														  XAP_Dialog_Id id)
{
	return new AP_UnixDialog_Stylist(pFactory,id);
}

AP_UnixDialog_Stylist::AP_UnixDialog_Stylist(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id id)
	: AP_Dialog_Stylist(pDlgFactory,id), 
	  m_windowMain(NULL),
	  m_wStyleList(NULL),
	  m_wRenderer(NULL),
	  m_wModel(NULL),
	  m_wStyleListContainer(NULL)
{
}

AP_UnixDialog_Stylist::~AP_UnixDialog_Stylist(void)
{
}

void AP_UnixDialog_Stylist::event_Close(void)
{
	
	destroy();
}

void AP_UnixDialog_Stylist::setStyleInGUI(void)
{
	GtkTreeIter child, parent;
	gboolean itering;
	gchar *entry;
	std::string sLocCurStyle;
	UT_UTF8String sCurStyle = *getCurStyle();

	if((getStyleTree() == NULL) || (sCurStyle.size() == 0))
		updateDialog();

	if(m_wStyleList == NULL)
		return;

	if(isStyleTreeChanged())
		_fillTree();

	pt_PieceTable::s_getLocalisedStyleName(sCurStyle.utf8_str(), sLocCurStyle);

	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_wStyleList));
	itering = gtk_tree_model_get_iter_first(model, &parent);

	while (itering)
	{
		if (gtk_tree_model_iter_children(model, &child, &parent))
		{
			do
			{
				gtk_tree_model_get(model, &child, 0, &entry, -1);

				if (sLocCurStyle.c_str() == entry)
				{
					itering = FALSE;
					break;
				}

				g_free(entry);

			}
			while (gtk_tree_model_iter_next(model, &child));
		}

		if (itering)
			itering = gtk_tree_model_iter_next(model, &parent);
	}

	GtkTreePath *gPathFull = gtk_tree_model_get_path(model, &child);
	GtkTreePath *gPathRow = gtk_tree_model_get_path(model, &parent);
	gtk_tree_view_expand_row( GTK_TREE_VIEW(m_wStyleList),gPathRow,TRUE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(m_wStyleList),gPathFull,NULL,TRUE,0.5,0.5);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(m_wStyleList),gPathFull,NULL,TRUE);
	setStyleChanged(false);
	gtk_tree_path_free(gPathRow);
	gtk_tree_path_free(gPathFull);
}

void AP_UnixDialog_Stylist::destroy(void)
{
	finalize();
	gtk_widget_destroy(m_windowMain);
	m_windowMain = NULL;
	m_wRenderer = NULL;
	m_wStyleList = NULL;
}

void AP_UnixDialog_Stylist::activate(void)
{
	UT_ASSERT (m_windowMain);
	gdk_window_raise (gtk_widget_get_window(m_windowMain));
}

void AP_UnixDialog_Stylist::notifyActiveFrame(XAP_Frame * /*pFrame*/)
{
    UT_ASSERT(m_windowMain);
}

/*!
 * Set the style in the XP layer from the selection in the GUI.
 */
void AP_UnixDialog_Stylist::styleClicked(UT_sint32 row, UT_sint32 col)
{
	UT_UTF8String sStyle;
	UT_DEBUGMSG(("row %d col %d clicked \n",row,col));

	if((col == 0) && (getStyleTree()->getNumCols(row) == 1))
		return;
	else if(col == 0)
		getStyleTree()->getStyleAtRowCol(sStyle,row,col);
	else
		getStyleTree()->getStyleAtRowCol(sStyle,row,col-1);

	UT_DEBUGMSG(("StyleClicked row %d col %d style %s \n",row,col,sStyle.utf8_str()));
	setCurStyle(sStyle);
}

void AP_UnixDialog_Stylist::runModeless(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	// Populate the window's data items
	_populateWindowData();
	_connectSignals();
	abiSetupModelessDialog(GTK_DIALOG(mainWindow),pFrame,this,GTK_RESPONSE_CLOSE);
	startUpdater();
}


void AP_UnixDialog_Stylist::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	m_bIsModal = true;
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	// Populate the window's data items
	_populateWindowData();
	_connectSignals();

	switch (abiRunModalDialog ( GTK_DIALOG(mainWindow), pFrame, this, GTK_RESPONSE_CLOSE,false ))
	{
	case GTK_RESPONSE_CLOSE:
		setStyleValid(false);
		break;
	case GTK_RESPONSE_OK:
		setStyleValid(true);
		break;
	default:
		setStyleValid(false);
		break;
	}
	abiDestroyWidget(mainWindow);
}


GtkWidget * AP_UnixDialog_Stylist::_constructWindow(void)
{
	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_Stylist.ui");

	const XAP_StringSet * pSS = m_pApp->getStringSet ();

	m_windowMain   = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_Stylist"));
	m_wStyleListContainer  = GTK_WIDGET(gtk_builder_get_object(builder, "TreeViewContainer"));

	if(m_bIsModal)
	{
		/*button =*/ gtk_dialog_add_button(GTK_DIALOG(m_windowMain), "gtk-ok", GTK_RESPONSE_OK);
	}
	else
	{
		/*button =*/ gtk_dialog_add_button(GTK_DIALOG(m_windowMain), "gtk-apply", GTK_RESPONSE_APPLY);
	}

	// set the dialog title
	std::string s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Stylist_Title,s);
	abiDialogSetTitle(m_windowMain, "%s", s.c_str());

	g_object_unref(G_OBJECT(builder));
	
	return m_windowMain;
}

void  AP_UnixDialog_Stylist::event_Apply(void)
{
	Apply();
}

/*!
 * Fill the GUI tree with the styles as defined in the XP tree.
 */
void  AP_UnixDialog_Stylist::_fillTree(void)
{
	Stylist_tree * pStyleTree = getStyleTree();
	if(pStyleTree == NULL)
	{
		updateDialog();
		pStyleTree = getStyleTree();
	}
	if(pStyleTree->getNumRows() == 0)
	{
		updateDialog();
		pStyleTree = getStyleTree();
	}
	UT_DEBUGMSG(("Number of rows of styles in document %d \n",pStyleTree->getNumRows()));
	if(m_wRenderer)
	{
//		g_object_unref (G_OBJECT (m_wRenderer));
		gtk_widget_destroy (m_wStyleList);
	}

	GtkTreeIter iter;
	GtkTreeIter child_iter;
	GtkTreeSelection *sel;
	UT_sint32 row,col, page;

	m_wModel = gtk_tree_store_new (3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);

	page = 0;
	std::string sTmp(""), sLoc;
	for(row= 0; row < pStyleTree->getNumRows();row++)
	{
		gtk_tree_store_append (m_wModel, &iter, NULL);
		if(!pStyleTree->getNameOfRow(sTmp,row))
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		}
		if(pStyleTree->getNumCols(row) > 0)
		{
			xxx_UT_DEBUGMSG(("Adding Heading %s at row %d \n",sTmp.utf8_str(),row));

			gtk_tree_store_set (m_wModel, &iter, 0, sTmp.c_str(), 1, row,2,0, -1);
			for(col =0 ; col < pStyleTree->getNumCols(row); col++)
			{
				gtk_tree_store_append (m_wModel, &child_iter, &iter);
				UT_UTF8String style;
				if(!pStyleTree->getStyleAtRowCol(style,row,col))
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					break;
				}
				pt_PieceTable::s_getLocalisedStyleName(sTmp.c_str(), sLoc);
				xxx_UT_DEBUGMSG(("Adding style %s at row %d col %d \n", sLoc.c_str(), row, col + 1));
				gtk_tree_store_set(m_wModel, &child_iter, 0, sLoc.c_str(), 1, row, 2, col + 1, -1);
				page++;
			}
		}
		else
		{
			pt_PieceTable::s_getLocalisedStyleName(sTmp.c_str(), sLoc);
			xxx_UT_DEBUGMSG(("Adding style %s at row %d \n", sLoc.utf8_str(), row));
			gtk_tree_store_set(m_wModel, &iter, 0, sLoc.c_str(), 1, row, 2, 0, -1);
			page++;
		}
	}

	// create a new treeview
	GtkTreeSortable *sort = GTK_TREE_SORTABLE(m_wModel);
	gtk_tree_sortable_set_sort_func(sort, 0, s_compare, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(sort, 0, GTK_SORT_ASCENDING);     
	m_wStyleList = gtk_tree_view_new_with_model (GTK_TREE_MODEL (sort));
	g_object_unref (G_OBJECT (m_wModel));

	// get the current selection
	sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (m_wStyleList));
	gtk_tree_selection_set_mode (sel, GTK_SELECTION_BROWSE);
	gtk_tree_selection_set_select_function (sel, tree_select_filter,
														 NULL, NULL);
	
	const XAP_StringSet * pSS = m_pApp->getStringSet ();
	m_wRenderer = gtk_cell_renderer_text_new ();
	std::string s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Stylist_Styles,s);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (m_wStyleList),
												 -1, s.c_str(),
												 m_wRenderer, "text", 0, NULL); 	

	gtk_tree_view_collapse_all (GTK_TREE_VIEW (m_wStyleList));
	gtk_container_add (GTK_CONTAINER (m_wStyleListContainer), m_wStyleList);

	g_signal_connect_after(G_OBJECT(m_wStyleList),
						   "cursor-changed",
						   G_CALLBACK(s_types_clicked),
						   static_cast<gpointer>(this));

	g_signal_connect_after(G_OBJECT(m_wStyleList),
						   "row-activated",
						   G_CALLBACK(s_types_dblclicked),
						   static_cast<gpointer>(this));
	gtk_widget_show_all(m_wStyleList);
	setStyleTreeChanged(false);
}

void  AP_UnixDialog_Stylist::_populateWindowData(void)
{
	_fillTree();
	setStyleInGUI();
}

void  AP_UnixDialog_Stylist::_connectSignals(void)
{
	g_signal_connect(G_OBJECT(m_windowMain), "response", 
					 G_CALLBACK(s_response_triggered), this);
	// the catch-alls
	// Dont use gtk_signal_connect_after for modeless dialogs
	g_signal_connect(G_OBJECT(m_windowMain),
			   "destroy",
			   G_CALLBACK(s_destroy_clicked),
			   (gpointer) this);
	g_signal_connect(G_OBJECT(m_windowMain),
			   "delete_event",
			   G_CALLBACK(s_delete_clicked),
			   (gpointer) this);
}
