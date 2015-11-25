/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) Robert Staudinger <robsta@stereolyzer.net>
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
#include <gtk/gtk.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"
#include "xap_GtkSignalBlocker.h"

#include "xap_Dialog_Id.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Goto.h"
#include "ap_UnixDialog_Goto.h"

#include "GTKCommon.h"


static void AP_UnixDialog_Goto__onSwitchPage (GtkNotebook *notebook,
											  gpointer Page,
											  guint page,
											  gpointer data)
{
	UT_UNUSED(notebook);
	UT_UNUSED(Page);
	UT_DEBUGMSG(("_onSwitchPage() '%d'\n", page));

	if (page == 0)
	{
		AP_UnixDialog_Goto *dlg = static_cast<AP_UnixDialog_Goto *>(data);
		dlg->updatePosition();
	}
}

/*!
* Event dispatcher for spinbutton "page".
*/
gboolean 
AP_UnixDialog_Goto__onFocusPage (GtkWidget 		  * /*widget*/,
								 GdkEventFocus    *event,
								 gpointer 		  data) 
{
	UT_DEBUGMSG (("ROB: _onFocusPage () '%d', '%d'\n", event->type, event->in));
	if (event->type == GDK_FOCUS_CHANGE && event->in) {
		AP_UnixDialog_Goto *dlg = static_cast <AP_UnixDialog_Goto *>(data);
		dlg->updateCache (AP_JUMPTARGET_PAGE);
	}
	/* propagate further */
	return FALSE;
}

/*!
* Event dispatcher for spinbutton "line".
*/
gboolean 
AP_UnixDialog_Goto__onFocusLine (GtkWidget 		  * /*widget*/,
								 GdkEventFocus    *event,
								 gpointer 		  data) 
{
	UT_DEBUGMSG (("ROB: _onFocusLine () '%d', '%d'\n", event->type, event->in));
	if (event->type == GDK_FOCUS_CHANGE && event->in) {
		AP_UnixDialog_Goto *dlg = static_cast <AP_UnixDialog_Goto *>(data);
		dlg->updateCache (AP_JUMPTARGET_LINE);
	}
	/* propagate further */
	return FALSE;
}

/*!
* Event dispatcher for treeview "bookmarks".
*/
gboolean 
AP_UnixDialog_Goto__onFocusBookmarks (GtkWidget 	   * /*widget*/,
									  GdkEventFocus    *event,
									  gpointer 		   data) 
{
	UT_DEBUGMSG (("ROB: _onFocusBookmarks () '%d', '%d'\n", event->type, event->in));
	if (event->type == GDK_FOCUS_CHANGE && event->in) {
		AP_UnixDialog_Goto *dlg = static_cast <AP_UnixDialog_Goto *>(data);
		dlg->updateCache (AP_JUMPTARGET_BOOKMARK);
	}
	/* propagate further */
	return FALSE;
}
gboolean 
AP_UnixDialog_Goto__onFocusXMLIDs (GtkWidget 	   * /*widget*/,
									  GdkEventFocus    *event,
									  gpointer 		   data) 
{
	UT_DEBUGMSG (("MIQ: _onFocusXMLIDs () '%d', '%d'\n", event->type, event->in));
	if (event->type == GDK_FOCUS_CHANGE && event->in)
    {
		AP_UnixDialog_Goto *dlg = static_cast <AP_UnixDialog_Goto *>(data);
		dlg->updateCache (AP_JUMPTARGET_XMLID);
	}
	/* propagate further */
	return FALSE;
}
gboolean 
AP_UnixDialog_Goto__onFocusAnno (GtkWidget 	   * /*widget*/,
                                 GdkEventFocus    *event,
                                 gpointer 		   data) 
{
	UT_DEBUGMSG (("MIQ: _onFocusAnno () '%d', '%d'\n", event->type, event->in));
	if (event->type == GDK_FOCUS_CHANGE && event->in)
    {
		AP_UnixDialog_Goto *dlg = static_cast <AP_UnixDialog_Goto *>(data);
		dlg->updateCache (AP_JUMPTARGET_ANNOTATION);
	}
	/* propagate further */
	return FALSE;
}

/*!
* Event dispatcher for spinbutton "page".
*/
void 
AP_UnixDialog_Goto__onPageChanged (GtkSpinButton * /*spinbutton*/,
								   gpointer 	  data)
{
	AP_UnixDialog_Goto *dlg = static_cast <AP_UnixDialog_Goto *>(data);
	dlg->onPageChanged ();
}

/*!
* Event dispatcher for spinbutton "line".
*/
void 
AP_UnixDialog_Goto__onLineChanged (GtkSpinButton * /*spinbutton*/,
								   gpointer 	  data)
{
	AP_UnixDialog_Goto *dlg = static_cast <AP_UnixDialog_Goto *>(data);
	dlg->onLineChanged ();
}

/*!
* Event dispatcher for treeview "bookmarks".
*/
void
AP_UnixDialog_Goto__onBookmarkDblClicked (GtkTreeView       * /*tree*/,
										  GtkTreePath       * /*path*/,
										  GtkTreeViewColumn * /*col*/,
										  gpointer		    data)
{
	AP_UnixDialog_Goto *dlg = static_cast <AP_UnixDialog_Goto *>(data);
	dlg->onBookmarkDblClicked ();
}
void
AP_UnixDialog_Goto__onXMLIDDblClicked (GtkTreeView       * /*tree*/,
                                       GtkTreePath       * /*path*/,
                                       GtkTreeViewColumn * /*col*/,
                                       gpointer		    data)
{
	AP_UnixDialog_Goto *dlg = static_cast <AP_UnixDialog_Goto *>(data);
	dlg->onXMLIDDblClicked ();
}
void
AP_UnixDialog_Goto__onAnnoDblClicked (GtkTreeView       * /*tree*/,
                                      GtkTreePath       * /*path*/,
                                      GtkTreeViewColumn * /*col*/,
                                      gpointer		    data)
{
	AP_UnixDialog_Goto *dlg = static_cast <AP_UnixDialog_Goto *>(data);
	dlg->onAnnoDblClicked ();
}

/*!
* Event dispatcher for button "jump".
*/
void
AP_UnixDialog_Goto__onJumpClicked (GtkButton * /*button*/,
								   gpointer   data)
{
	AP_UnixDialog_Goto *dlg = static_cast <AP_UnixDialog_Goto *>(data);
	dlg->onJumpClicked ();
}

/*!
* Event dispatcher for button "prev".
*/
void
AP_UnixDialog_Goto__onPrevClicked (GtkButton * /*button*/,
								   gpointer   data)
{
	AP_UnixDialog_Goto *dlg = static_cast <AP_UnixDialog_Goto *>(data);
	dlg->onPrevClicked ();
}

/*!
* Event dispatcher for button "next".
*/
void
AP_UnixDialog_Goto__onNextClicked (GtkButton * /*button*/,
								   gpointer   data)
{
	AP_UnixDialog_Goto *dlg = static_cast <AP_UnixDialog_Goto *>(data);
	dlg->onNextClicked ();
}

/*!
* Event dispatcher for button "close".
*/
void
AP_UnixDialog_Goto__onDialogResponse (GtkDialog * /*dialog*/,
									  gint 		response,
									  gpointer  data)
{
	AP_UnixDialog_Goto *dlg = static_cast <AP_UnixDialog_Goto *>(data);
	if (response == GTK_RESPONSE_CLOSE) {
		dlg->destroy ();		
	}
}

/*!
* Event dispatcher for window.
*/
gboolean
AP_UnixDialog_Goto__onDeleteWindow (GtkWidget * /*widget*/,
									GdkEvent  * /*event*/,
									gpointer  data)
{
	AP_UnixDialog_Goto *dlg = static_cast <AP_UnixDialog_Goto *>(data);
	if (dlg->getWindow ()) {
		dlg->destroy ();
	}
	return TRUE;
}



/*!
* Static ctor.
*/
XAP_Dialog * 
AP_UnixDialog_Goto::static_constructor(XAP_DialogFactory *pFactory,
									   XAP_Dialog_Id 	 id)
{
	AP_UnixDialog_Goto *dlg = new AP_UnixDialog_Goto (pFactory, id);
	return dlg;
}

/*!
* Ctor.
*/
AP_UnixDialog_Goto::AP_UnixDialog_Goto(XAP_DialogFactory *pDlgFactory,
									   XAP_Dialog_Id 	 id)
	: AP_Dialog_Goto   (pDlgFactory, id), 
	  m_wDialog 	   (NULL),
	  m_nbNotebook	   (NULL),
	  m_lbPage		   (NULL),
	  m_lbLine		   (NULL),
	  m_lbBookmarks    (NULL),
	  m_lbXMLids	   (NULL),
	  m_lbAnnotations  (NULL),
	  m_sbPage		   (NULL),
	  m_sbLine		   (NULL),
	  m_lvBookmarks	   (NULL),
	  m_btJump		   (NULL),
	  m_btPrev		   (NULL),
	  m_btNext		   (NULL),
      m_lvXMLIDs       (0),
      m_lvAnno         (0),
	  m_btClose 	   (NULL), 
	  m_iPageConnect (0),
	  m_iLineConnect (0),
	  m_JumpTarget	   (AP_JUMPTARGET_BOOKMARK)
{
}

/*!
* Dtor.
*/
AP_UnixDialog_Goto::~AP_UnixDialog_Goto ()
{
	UT_DEBUGMSG (("ROB: ~AP_UnixDialog_Goto ()\n"));
}



/*!
* Event handler for spinbutton "page".
*/
void 
AP_UnixDialog_Goto::onPageChanged () 
{
	UT_DEBUGMSG (("ROB: onPageChanged () maxpage='%d'\n", m_DocCount.page));
	m_JumpTarget = AP_JUMPTARGET_PAGE;
	UT_uint32 page = (UT_uint32)gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_sbPage));
	if (page > m_DocCount.page) {
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_sbPage), 1);
	}
	onJumpClicked();
}

/*!
* Event handler for spinbutton "line".
*/
void 
AP_UnixDialog_Goto::onLineChanged () 
{
	UT_DEBUGMSG (("ROB: onLineChanged () maxline='%d'\n", m_DocCount.line));
	m_JumpTarget = AP_JUMPTARGET_LINE;
	UT_uint32 line = (UT_uint32)gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_sbLine));
	if (line > m_DocCount.line) {
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_sbLine), 1);
	}
	if (line == 0) {
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_sbLine), m_DocCount.line);
	}
	onJumpClicked();
}

/*!
* Event handler for treeview "bookmarks".
*/
void
AP_UnixDialog_Goto::onBookmarkDblClicked ()
{
	m_JumpTarget = AP_JUMPTARGET_BOOKMARK;
	onJumpClicked();
}
void
AP_UnixDialog_Goto::onXMLIDDblClicked ()
{
	m_JumpTarget = AP_JUMPTARGET_XMLID;
	onJumpClicked();
}
void
AP_UnixDialog_Goto::onAnnoDblClicked ()
{
	m_JumpTarget = AP_JUMPTARGET_ANNOTATION;
	onJumpClicked();
}

/*!
* Event handler for button "jump".
*/
void 
AP_UnixDialog_Goto::onJumpClicked () 
{
    std::string text;
    XAP_GtkSignalBlocker b(G_OBJECT(m_sbLine), m_iLineConnect);

	switch (m_JumpTarget) {
		case AP_JUMPTARGET_PAGE:
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_sbLine), 0);
			text = tostr(GTK_ENTRY (m_sbPage));
			break;
		case AP_JUMPTARGET_LINE:
			text = tostr(GTK_ENTRY (m_sbLine));
			if (text == "0") return;
			break;
		case AP_JUMPTARGET_BOOKMARK:
			text = _getSelectedBookmarkLabel ();
			break;
		case AP_JUMPTARGET_XMLID:
			text = _getSelectedXMLIDLabel();
			break;
		case AP_JUMPTARGET_ANNOTATION:
			text = _getSelectedAnnotationLabel();
			break;
		default:
			UT_DEBUGMSG (("AP_UnixDialog_Goto::onJumpClicked () no jump target\n"));
			return;
			// UT_ASSERT_NOT_REACHED ();
	}

	if (text.empty())
		return;		

	performGoto(m_JumpTarget, text.c_str());
}

/*!
* Event handler for button "prev".
*/
void 
AP_UnixDialog_Goto::onPrevClicked () 
{
	UT_DEBUGMSG (("ROB: onPrevClicked () '%d'\n", m_JumpTarget));

	UT_uint32 num = 0;
	switch (m_JumpTarget) {
		case AP_JUMPTARGET_PAGE:
			num = (UT_uint32)gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_sbPage));
			if (num == 1)
				num = m_DocCount.page;
			else
				num--;
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_sbPage), num);
			break;
		case AP_JUMPTARGET_LINE:
			num = (UT_uint32)gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_sbLine));
			if (num == 1)
				num = m_DocCount.line;
			else
				num--;
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_sbLine), num);
			break;
		case AP_JUMPTARGET_BOOKMARK:
			_selectPrevBookmark ();
			break;
		case AP_JUMPTARGET_XMLID:
            selectPrev(GTK_TREE_VIEW (m_lvXMLIDs));
			break;
		case AP_JUMPTARGET_ANNOTATION:
            selectPrev(GTK_TREE_VIEW (m_lvAnno));
			break;
		default:
			UT_DEBUGMSG (("ROB: AP_UnixDialog_Goto::onPrevClicked () no jump target\n"));
			return;
			// UT_ASSERT_NOT_REACHED ();
	}

	onJumpClicked ();
}

/*!
* Event handler for button "next".
*/
void 
AP_UnixDialog_Goto::onNextClicked () 
{
	UT_DEBUGMSG (("ROB: onNextClicked () '%d'\n", m_JumpTarget));

	UT_uint32 num = 0;
	switch (m_JumpTarget) {
		case AP_JUMPTARGET_PAGE:
			num = (UT_uint32)gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_sbPage));
			num++;
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_sbPage), num);
			break;
		case AP_JUMPTARGET_LINE:
			num = (UT_uint32)gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_sbLine));
			num++;
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_sbLine), num);
			break;
		case AP_JUMPTARGET_BOOKMARK:
			_selectNextBookmark ();
			break;
		case AP_JUMPTARGET_XMLID:
            selectNext(GTK_TREE_VIEW (m_lvXMLIDs));
			break;
		case AP_JUMPTARGET_ANNOTATION:
            selectNext(GTK_TREE_VIEW (m_lvAnno));
			break;
		default:
			UT_DEBUGMSG (("ROB: AP_UnixDialog_Goto::onNextClicked () no jump target\n"));
			return;
			// UT_ASSERT_NOT_REACHED ();
	}

	onJumpClicked ();
}

/*!
* Set jump target and update cached data like number of lines and pages.
* @see ap_types.h
*/
void 
AP_UnixDialog_Goto::updateCache (AP_JumpTarget target) 
{
	m_JumpTarget = target;
	updateDocCount ();
}

/*!
* Update cached data like number of lines and pages.
*/
void
AP_UnixDialog_Goto::updateDocCount ()
{
	m_DocCount = getView()->countWords(false); 
	UT_DEBUGMSG (("ROB: updateCache () page='%d' line='%d'\n", m_DocCount.page, m_DocCount.line));
}

void AP_UnixDialog_Goto::updatePosition (void)
{
	// pages, page increment of 10 is pretty arbitrary (set in the GtkBuilder UI file)
	UT_uint32 currentPage = getView()->getCurrentPageNumForStatusBar ();
	XAP_GtkSignalBlocker b1(G_OBJECT(m_sbPage), m_iPageConnect);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_sbPage), currentPage);

	// lines, line increment of 10 is pretty arbitrary (set in the GtkBuilder UI file)
	UT_uint32 currentLine = 0; /* FIXME get current line */
	XAP_GtkSignalBlocker b2(G_OBJECT(m_sbLine), m_iLineConnect);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_sbLine), currentLine);
}

void
AP_UnixDialog_Goto::setupXMLIDList( GtkWidget* w )
{
	// Liststore and -view
	GtkListStore *store = gtk_list_store_new ( NUM_COLUMNS, G_TYPE_STRING );
	gtk_tree_view_set_model (GTK_TREE_VIEW (w), GTK_TREE_MODEL (store));
	g_object_unref (G_OBJECT (store));

	// Column Bookmark
	GtkCellRenderer *renderer = NULL;
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (w),
												-1, "Name", renderer,
												"text", COLUMN_NAME,
												NULL);
	GtkTreeViewColumn *column = gtk_tree_view_get_column (GTK_TREE_VIEW (w), 0);
	gtk_tree_view_column_set_sort_column_id (column, COLUMN_NAME);

	g_signal_connect (GTK_TREE_VIEW (w), "focus-in-event", 
					  G_CALLBACK (AP_UnixDialog_Goto__onFocusXMLIDs), static_cast <gpointer>(this)); 
	g_signal_connect (GTK_TREE_VIEW (w), "row-activated", 
					  G_CALLBACK (AP_UnixDialog_Goto__onXMLIDDblClicked), static_cast <gpointer>(this));
}


void
AP_UnixDialog_Goto::setupAnnotationList( GtkWidget* w )
{
	GtkTreeViewColumn *column = NULL;
	GtkCellRenderer *renderer = NULL;
	// Liststore and -view
	GtkListStore *store = gtk_list_store_new ( NUM_ANNO_COLUMNS,
                                               G_TYPE_INT,
                                               G_TYPE_STRING, G_TYPE_STRING );
	gtk_tree_view_set_model (GTK_TREE_VIEW (w), GTK_TREE_MODEL (store));
	g_object_unref (G_OBJECT (store));

	// localization
	const XAP_StringSet * pSS = m_pApp->getStringSet ();
	std::string id, title, author;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Goto_Column_ID, id);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Goto_Column_Title, title);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Goto_Column_Author, author);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (w),
												-1, id.c_str(), renderer,
												"text", COLUMN_ANNO_ID,
												NULL);
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (w), COLUMN_ANNO_ID );
	gtk_tree_view_column_set_sort_column_id (column, COLUMN_ANNO_ID );

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (w),
												-1, title.c_str(), renderer,
												"text", COLUMN_ANNO_TITLE,
												NULL);
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (w), COLUMN_ANNO_TITLE );
	gtk_tree_view_column_set_sort_column_id (column, COLUMN_ANNO_TITLE );


	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (w),
												-1, author.c_str(), renderer,
												"text", COLUMN_ANNO_AUTHOR,
												NULL);
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (w), COLUMN_ANNO_AUTHOR );
	gtk_tree_view_column_set_sort_column_id (column, COLUMN_ANNO_AUTHOR );
    

    
	g_signal_connect (GTK_TREE_VIEW (w), "focus-in-event", 
					  G_CALLBACK (AP_UnixDialog_Goto__onFocusAnno), static_cast <gpointer>(this)); 
	g_signal_connect (GTK_TREE_VIEW (w), "row-activated", 
					  G_CALLBACK (AP_UnixDialog_Goto__onAnnoDblClicked), static_cast <gpointer>(this));
}


/*!
* Build dialog.
*/
void 
AP_UnixDialog_Goto::_constructWindow (XAP_Frame * /*pFrame*/) 
{
	UT_DEBUGMSG (("ROB: _constructWindow ()\n"));		

	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_Goto.ui");

	m_wDialog = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_Goto"));
	m_nbNotebook = GTK_WIDGET(gtk_builder_get_object(builder, "nbNotebook"));
	m_lbPage = GTK_WIDGET(gtk_builder_get_object(builder, "lbPage"));
	m_lbLine = GTK_WIDGET(gtk_builder_get_object(builder, "lbLine"));
	m_lbBookmarks = GTK_WIDGET(gtk_builder_get_object(builder, "lbBookmarks"));
	m_lbXMLids = GTK_WIDGET(gtk_builder_get_object(builder, "lbXMLids"));
	m_lbAnnotations = GTK_WIDGET(gtk_builder_get_object(builder, "lbAnnotations"));
	m_sbPage = GTK_WIDGET(gtk_builder_get_object(builder, "sbPage"));
	m_sbLine = GTK_WIDGET(gtk_builder_get_object(builder, "sbLine"));
	m_lvBookmarks = GTK_WIDGET(gtk_builder_get_object(builder, "lvBookmarks"));
	m_btJump = GTK_WIDGET(gtk_builder_get_object(builder, "btJump"));
	m_btPrev = GTK_WIDGET(gtk_builder_get_object(builder, "btPrev"));
	m_btNext = GTK_WIDGET(gtk_builder_get_object(builder, "btNext"));
	m_lvXMLIDs = GTK_WIDGET(gtk_builder_get_object(builder, "lvXMLIDs"));
	m_lvAnno   = GTK_WIDGET(gtk_builder_get_object(builder, "lvAnno"));
	m_btClose = GTK_WIDGET(gtk_builder_get_object(builder, "btClose"));


	// localise	
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbPosition")), pSS, AP_STRING_ID_DLG_Goto_Label_Position);
	/* FIXME jump targets localised in xp land, make sure they work for non ascii characters */
	const gchar **targets = getJumpTargets ();
	const gchar *text = NULL;
	if ((text = targets[AP_JUMPTARGET_PAGE]) != NULL)
		gtk_label_set_text (GTK_LABEL (m_lbPage), text);
	if ((text = targets[AP_JUMPTARGET_LINE]) != NULL)
		gtk_label_set_text (GTK_LABEL (m_lbLine), text);
	if ((text = targets[AP_JUMPTARGET_BOOKMARK]) != NULL)
		gtk_label_set_text (GTK_LABEL (m_lbBookmarks), text);
	if ((text = targets[AP_JUMPTARGET_XMLID]) != NULL)
		gtk_label_set_text (GTK_LABEL (m_lbXMLids), text);
	if ((text = targets[AP_JUMPTARGET_ANNOTATION]) != NULL)
		gtk_label_set_text (GTK_LABEL (m_lbAnnotations), text);


    setupXMLIDList( m_lvXMLIDs );
    setupAnnotationList( m_lvAnno );
    
	// Liststore and -view
	GtkListStore *store = gtk_list_store_new (NUM_COLUMNS, G_TYPE_STRING);
	gtk_tree_view_set_model (GTK_TREE_VIEW (m_lvBookmarks), GTK_TREE_MODEL (store));
	g_object_unref (G_OBJECT (store));

	// Column Bookmark
	GtkCellRenderer *renderer = NULL;
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (m_lvBookmarks),
												-1, "Name", renderer,
												"text", COLUMN_NAME,
												NULL);
	GtkTreeViewColumn *column = gtk_tree_view_get_column (GTK_TREE_VIEW (m_lvBookmarks), 0);
	gtk_tree_view_column_set_sort_column_id (column, COLUMN_NAME);

	// Signals
	g_signal_connect (GTK_NOTEBOOK (m_nbNotebook), "switch-page", 
					  G_CALLBACK (AP_UnixDialog_Goto__onSwitchPage), static_cast <gpointer>(this)); 
	g_signal_connect (GTK_SPIN_BUTTON (m_sbPage), "focus-in-event", 
					  G_CALLBACK (AP_UnixDialog_Goto__onFocusPage), static_cast <gpointer>(this)); 
	m_iPageConnect = g_signal_connect (GTK_SPIN_BUTTON (m_sbPage), "value-changed",
					  G_CALLBACK (AP_UnixDialog_Goto__onPageChanged), static_cast <gpointer>(this)); 

	g_signal_connect (GTK_SPIN_BUTTON (m_sbLine), "focus-in-event", 
					  G_CALLBACK (AP_UnixDialog_Goto__onFocusLine), static_cast <gpointer>(this)); 
	m_iLineConnect = g_signal_connect (GTK_SPIN_BUTTON (m_sbLine), "value-changed",
					  G_CALLBACK (AP_UnixDialog_Goto__onLineChanged), static_cast <gpointer>(this)); 

	g_signal_connect (GTK_TREE_VIEW (m_lvBookmarks), "focus-in-event", 
					  G_CALLBACK (AP_UnixDialog_Goto__onFocusBookmarks), static_cast <gpointer>(this)); 
	g_signal_connect (GTK_TREE_VIEW (m_lvBookmarks), "row-activated", 
					  G_CALLBACK (AP_UnixDialog_Goto__onBookmarkDblClicked), static_cast <gpointer>(this));

	g_signal_connect (GTK_BUTTON (m_btJump), "clicked", 
					  G_CALLBACK (AP_UnixDialog_Goto__onJumpClicked), static_cast <gpointer>(this));
	g_signal_connect (GTK_BUTTON (m_btPrev), "clicked", 
					  G_CALLBACK (AP_UnixDialog_Goto__onPrevClicked), static_cast <gpointer>(this));
	g_signal_connect (GTK_BUTTON (m_btNext), "clicked", 
					  G_CALLBACK (AP_UnixDialog_Goto__onNextClicked), static_cast <gpointer>(this));

	g_signal_connect (GTK_DIALOG (m_wDialog), "response",
					  G_CALLBACK (AP_UnixDialog_Goto__onDialogResponse), static_cast <gpointer>(this));
	g_signal_connect (m_wDialog, "delete-event",
					  G_CALLBACK (AP_UnixDialog_Goto__onDeleteWindow), static_cast <gpointer>(this));

	g_object_unref(G_OBJECT(builder));
}

/*!
* Update dialog's data.
*/
void 
AP_UnixDialog_Goto::_updateWindow ()
{
	UT_DEBUGMSG (("ROB: _updateWindow () #bookmarks='%d', mapped='%d'\n", getExistingBookmarksCount(), gtk_widget_get_mapped(m_wDialog)));

	ConstructWindowName ();
	gtk_window_set_title (GTK_WINDOW (m_wDialog), m_WindowName);

	// position: pages and lines
	updatePosition();

	// bookmarks, detaching model for faster updates
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (m_lvBookmarks));
	g_object_ref (G_OBJECT (model));
	gtk_tree_view_set_model (GTK_TREE_VIEW (m_lvBookmarks), NULL);
	gtk_list_store_clear (GTK_LIST_STORE (model));

	GtkTreeIter iter;
	UT_uint32 numBookmarks = getExistingBookmarksCount();
	for (UT_uint32 i = 0; i < numBookmarks; i++) {

		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		const std::string & name = getNthExistingBookmark(i);
		UT_DEBUGMSG (("    ROB: '%s'\n", name.c_str()));
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
							COLUMN_NAME, name.c_str(), /* 
							COLUMN_PAGE, "0", 
							COLUMN_NUMBER, 0, */
							-1);
	}
	gtk_tree_view_set_model (GTK_TREE_VIEW (m_lvBookmarks), model);
	g_object_unref (G_OBJECT (model));

    updateXMLIDList( m_lvXMLIDs );
    updateAnnotationList( m_lvAnno );
	updateDocCount ();
}


void
AP_UnixDialog_Goto::updateXMLIDList( GtkWidget* w )
{
	// detaching model for faster updates
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (w));
	g_object_ref (G_OBJECT (model));
	gtk_tree_view_set_model (GTK_TREE_VIEW (w), NULL);
	gtk_list_store_clear (GTK_LIST_STORE (model));

    if( PD_DocumentRDFHandle rdf = getRDF() )
    {
        GtkTreeIter iter;
        std::set< std::string > xmlids;
        rdf->getAllIDs( xmlids );
        UT_DEBUGMSG (("MIQ: xmlids.sz:%lu\n", (long unsigned)xmlids.size() ));

        for( std::set< std::string >::iterator xiter = xmlids.begin();
             xiter != xmlids.end(); ++xiter )
        {
            gtk_list_store_append (GTK_LIST_STORE (model), &iter);
            std::string name = *xiter;
            UT_DEBUGMSG (("    MIQ: '%s'\n", name.c_str()));
            gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
                                COLUMN_NAME, name.c_str(), -1);
        }
    }
    
	gtk_tree_view_set_model (GTK_TREE_VIEW (w), model);
	g_object_unref (G_OBJECT (model));
    
}


void
AP_UnixDialog_Goto::updateAnnotationList( GtkWidget* w )
{
	// detaching model for faster updates
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (w));
	g_object_ref (G_OBJECT (model));
	gtk_tree_view_set_model (GTK_TREE_VIEW (w), NULL);
	gtk_list_store_clear (GTK_LIST_STORE (model));

    GtkTreeIter iter;
    FV_View* pView = getView();
	UT_uint32 max = pView->countAnnotations();
    for( UT_uint32 i=0; i<max; ++i )
    {
        gtk_list_store_append (GTK_LIST_STORE (model), &iter);
        std::string name   = tostr(i);
        std::string title  = pView->getAnnotationTitle(i);
        std::string author = pView->getAnnotationAuthor(i);
        
        UT_DEBUGMSG (("    MIQ: '%s'\n", name.c_str()));
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
                            COLUMN_ANNO_ID,     i,
                            COLUMN_ANNO_TITLE,  title.c_str(),
                            COLUMN_ANNO_AUTHOR, author.c_str(),
                            -1);
    }
    
	gtk_tree_view_set_model (GTK_TREE_VIEW (w), model);
	g_object_unref (G_OBJECT (model));
    
}



void 
AP_UnixDialog_Goto::runModeless (XAP_Frame * pFrame)
{
	UT_DEBUGMSG (("ROB: runModeless ()\n"));
	_constructWindow (pFrame);
	UT_ASSERT (m_wDialog);
	_updateWindow ();
	abiSetupModelessDialog (GTK_DIALOG (m_wDialog), pFrame, this, GTK_RESPONSE_CLOSE);
	gtk_widget_show_all (m_wDialog);
	gtk_window_present (GTK_WINDOW (m_wDialog));
}

void 
AP_UnixDialog_Goto::notifyActiveFrame (XAP_Frame * /*pFrame*/)
{
	UT_DEBUGMSG (("ROB: notifyActiveFrame ()\n"));
	UT_ASSERT (m_wDialog);

	_updateWindow ();
	/* default to page */
	m_JumpTarget = AP_JUMPTARGET_PAGE;
}

void 
AP_UnixDialog_Goto::activate (void)
{
	UT_ASSERT (m_wDialog);
	UT_DEBUGMSG (("ROB: AP_UnixDialog_Goto::activate ()\n"));
	_updateWindow ();
	gtk_window_present (GTK_WINDOW (m_wDialog));
}

void 
AP_UnixDialog_Goto::destroy ()
{
	UT_DEBUGMSG (("ROB: AP_UnixDialog_Goto::destroy ()\n"));
	modeless_cleanup ();
	if (m_wDialog) {
		gtk_widget_destroy (m_wDialog);
		m_wDialog = NULL;
	}
}

/**
* Try to select the bookmark before the current one.
* If none is selected the last in the list is picked.
* Wraps around.
*/
void
AP_UnixDialog_Goto::_selectPrevBookmark () 
{
	UT_DEBUGMSG (("ROB: AP_UnixDialog_Goto::_selectPrevBookmark ()\n"));
    selectPrev(GTK_TREE_VIEW (m_lvBookmarks));
}

/**
* Try to select the bookmark after the current one.
* If none is selected the first in the list is picked.
* Wraps around.
*/
void
AP_UnixDialog_Goto::_selectNextBookmark () 
{
	UT_DEBUGMSG (("ROB: AP_UnixDialog_Goto::_selectNextBookmark ()\n"));
    selectNext(GTK_TREE_VIEW (m_lvBookmarks));
}


/*!
* Get the label of the currently selected bookmark in the list.
*/
std::string
AP_UnixDialog_Goto::_getSelectedBookmarkLabel () 
{
	UT_DEBUGMSG (("ROB: AP_UnixDialog_Goto::_getSelectedBookmarkLabel ()\n"));
    std::string ret = getSelectedText( GTK_TREE_VIEW (m_lvBookmarks), COLUMN_NAME );
	return ret;
}

std::string
AP_UnixDialog_Goto::_getSelectedXMLIDLabel()
{
	UT_DEBUGMSG (("MIQ: AP_UnixDialog_Goto::_getSelectedXMLIDLabel ()\n"));
    std::string ret = getSelectedText( GTK_TREE_VIEW (m_lvXMLIDs), COLUMN_NAME );
	return ret;
}

std::string
AP_UnixDialog_Goto::_getSelectedAnnotationLabel()
{
	UT_DEBUGMSG (("MIQ: AP_UnixDialog_Goto::_getSelectedAnnotationLabel ()\n"));
    std::string ret = tostr(getSelectedUInt( GTK_TREE_VIEW (m_lvAnno), COLUMN_ANNO_ID ));
	return ret;
}

