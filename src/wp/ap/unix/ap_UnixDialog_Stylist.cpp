/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
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

#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_Stylist.h"

static void s_types_clicked(GtkTreeView *treeview,
                            AP_UnixDialog_Stylist * dlg)
{
	UT_ASSERT(treeview && dlg);

	GtkTreeSelection * selection;
	GtkTreeIter iter;
	GtkTreeModel * model;

	UT_sint32 row,col;
	col = 0;
	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(treeview) );
	if (!selection || !gtk_tree_selection_get_selected (selection, &model, &iter)) {
		return;
	}

	// Update m_iTypeIndex with the row number
	gtk_tree_model_get (model, &iter, 1, &row, -1);

	dlg->styleClicked(row,col);
}

static void s_types_dblclicked(GtkTreeView *treeview,
							   GtkTreePath *arg1,
							   GtkTreeViewColumn *arg2,
							   AP_UnixDialog_Stylist * me)
{
	// simulate the effects of a single click
	s_types_clicked (treeview, me);
	me->event_Apply ();
}

static void s_delete_clicked(GtkWidget * wid, AP_UnixDialog_Stylist * me )
{
    abiDestroyWidget( wid ) ;// will emit signals for us
}


static void s_destroy_clicked(GtkWidget * wid, AP_UnixDialog_Stylist * me )
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
	else
		UT_DEBUGMSG(("Call Help!\n"));
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
	  m_wApply(NULL),
	  m_wClose(NULL),
	  m_wHelp(NULL)
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
	setStyleChanged(false);
	UT_ASSERT(0);
}

void AP_UnixDialog_Stylist::destroy(void)
{
	finalize();
	gtk_widget_destroy(m_windowMain);
	m_windowMain = NULL;
}

void AP_UnixDialog_Stylist::activate(void)
{
	UT_ASSERT (m_windowMain);
	gdk_window_raise (m_windowMain->window);
}

void AP_UnixDialog_Stylist::notifyActiveFrame(XAP_Frame *pFrame)
{
    UT_ASSERT(m_windowMain);
}

void AP_UnixDialog_Stylist::styleClicked(UT_sint32 row, UT_sint32 col)
{
	UT_UTF8String sStyle;
	getStyleTree()->getStyleAtRowCol(sStyle,row,col);
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

GtkWidget * AP_UnixDialog_Stylist::_constructWindow(void)
{
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_Stylist.glade";

	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );
	if (!xml)
		return NULL;
	
	const XAP_StringSet * pSS = m_pApp->getStringSet ();

	m_windowMain   = glade_xml_get_widget(xml, "ap_UnixDialog_Stylist");
	m_wStyleList  = glade_xml_get_widget(xml,"styleList");
	m_wApply = glade_xml_get_widget(xml,"btApply");
	m_wClose = glade_xml_get_widget(xml,"btClose");
	m_wHelp = glade_xml_get_widget(xml,"btHelp");
	// set the single selection mode for the TreeView
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (m_wStyleList)), GTK_SELECTION_SINGLE);	

	// set the dialog title
	abiDialogSetTitle(m_windowMain, pSS->getValueUTF8(AP_STRING_ID_DLG_Stylist_Title).utf8_str());
	
	// localize the strings in our dialog, and set tags for some widgets

	localizeButtonUnderline(m_wApply, pSS, AP_STRING_ID_DLG_ApplyButton);
	localizeButtonUnderline(m_wClose, pSS, AP_STRING_ID_DLG_CloseButton);
	localizeButtonUnderline(m_wHelp, pSS, AP_STRING_ID_DLG_HelpButton);
	return m_windowMain;
}

void  AP_UnixDialog_Stylist::event_Apply(void)
{
}

void  AP_UnixDialog_Stylist::_populateWindowData(void)
{
}

void  AP_UnixDialog_Stylist::_connectSignals(void)
{

	g_signal_connect_after(G_OBJECT(m_wStyleList),
						   "cursor-changed",
						   G_CALLBACK(s_types_clicked),
						   static_cast<gpointer>(this));

	g_signal_connect_after(G_OBJECT(m_wStyleList),
						   "row-activated",
						   G_CALLBACK(s_types_dblclicked),
						   static_cast<gpointer>(this));

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

