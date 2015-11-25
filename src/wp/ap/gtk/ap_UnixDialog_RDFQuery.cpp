/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2011 AbiSource, Inc.
 * Copyright (C) Ben Martin
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "GTKCommon.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"

#include "xap_Dialog_Id.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_RDFQuery.h"
#include "ap_UnixDialog_RDFQuery.h"

void
AP_UnixDialog_RDFQuery__onExecuteClicked ( GtkButton * /*button*/,
                                           gpointer   data )
{
	AP_UnixDialog_RDFQuery *dlg = static_cast <AP_UnixDialog_RDFQuery *>(data);
	dlg->onExecuteClicked ();
}
void
AP_UnixDialog_RDFQuery__onShowAllClicked ( GtkButton * /*button*/,
                                           gpointer   data )
{
	AP_UnixDialog_RDFQuery *dlg = static_cast <AP_UnixDialog_RDFQuery *>(data);
	dlg->onShowAllClicked ();
}



/*!
* Event dispatcher for button "close".
*/
void
AP_UnixDialog_RDFQuery__onDialogResponse (GtkDialog * /*dialog*/,
									  gint 		response,
									  gpointer  data)
{
	AP_UnixDialog_RDFQuery *dlg = static_cast <AP_UnixDialog_RDFQuery *>(data);
	if (response == GTK_RESPONSE_CLOSE) {
		dlg->destroy ();		
	}
}

/*!
* Event dispatcher for window.
*/
gboolean
AP_UnixDialog_RDFQuery__onDeleteWindow (GtkWidget * /*widget*/,
									GdkEvent  * /*event*/,
									gpointer  data)
{
	AP_UnixDialog_RDFQuery *dlg = static_cast <AP_UnixDialog_RDFQuery *>(data);
	if (dlg->getWindow ()) {
		dlg->destroy ();
	}
	return TRUE;
}



/*!
* Static ctor.
*/
XAP_Dialog * 
AP_UnixDialog_RDFQuery::static_constructor(XAP_DialogFactory *pFactory,
									   XAP_Dialog_Id 	 id)
{
	AP_UnixDialog_RDFQuery *dlg = new AP_UnixDialog_RDFQuery (pFactory, id);
	return dlg;
}

/*!
* Ctor.
*/
AP_UnixDialog_RDFQuery::AP_UnixDialog_RDFQuery(XAP_DialogFactory *pDlgFactory,
									   XAP_Dialog_Id 	 id)
	: AP_Dialog_RDFQuery   (pDlgFactory, id)
    , m_wDialog 	   (0)
    , m_btClose 	   (0)
    , m_btExecute      (0)
    , m_btShowAll      (0)
    , m_query          (0)
    , m_resultsView    (0)
	, m_resultsModel   (0)
    , m_status         (0)
{
}

/*!
* Dtor.
*/
AP_UnixDialog_RDFQuery::~AP_UnixDialog_RDFQuery ()
{
	UT_DEBUGMSG (("~AP_UnixDialog_RDFQuery ()\n"));
}



void
AP_UnixDialog_RDFQuery::clear()
{
    AP_Dialog_RDFQuery::clear();
    gtk_tree_store_clear( m_resultsModel );
}

void
AP_UnixDialog_RDFQuery::addStatement( const PD_RDFStatement& st )
{
    AP_Dialog_RDFQuery::addStatement(st);
}

void
AP_UnixDialog_RDFQuery::setupBindingsView( std::map< std::string, std::string >& b )
{
    if( b.size() >= C_COLUMN_ARRAY_SIZE )
    {
        return;
    }

    GType types[ C_COLUMN_ARRAY_SIZE ];
    for( int i = b.size() + 1; i >= 0; i-- )
        types[i] = G_TYPE_STRING;
    gint n_columns = b.size();
    
    
    GtkTreeStore* m = gtk_tree_store_newv( n_columns, types );
    gtk_tree_view_set_model( m_resultsView, GTK_TREE_MODEL( m ) );
    m_resultsModel = m;

    while( GtkTreeViewColumn* tvc = gtk_tree_view_get_column( GTK_TREE_VIEW( m_resultsView ), 0 ))
    {
        gtk_tree_view_remove_column( GTK_TREE_VIEW( m_resultsView ), tvc );
    }
    

    typedef std::list< std::pair< std::string, GtkTreeViewColumn* > > cols_t;
    cols_t cols;
    
    GtkCellRenderer* ren = 0;
    int colid = 0;
    for( std::map< std::string, std::string >::iterator iter = b.begin();
         iter != b.end(); ++iter, ++colid )
    {
        std::string cname = iter->first;
        
        ren = gtk_cell_renderer_text_new ();
        w_cols[ colid ] = gtk_tree_view_column_new_with_attributes( cname.c_str(), ren, "text", colid, NULL);
        gtk_tree_view_column_set_sort_column_id( w_cols[ colid ], colid );
        gtk_tree_view_column_set_resizable ( w_cols[ colid ], true );
//        gtk_tree_view_append_column( GTK_TREE_VIEW( m_resultsView ), w_cols[ colid ] );
        cols.push_back( make_pair( cname, w_cols[ colid ] ));
    }

    //
    // Make sure some columns appear in the desired order
    // which is not simply lexigraphical
    //
    typedef std::list< std::string > stringlist_t;
    stringlist_t hotColumns;
    hotColumns.push_back("o");
    hotColumns.push_back("p");
    hotColumns.push_back("s");
    hotColumns.push_back("object");
    hotColumns.push_back("predicate");
    hotColumns.push_back("subject");
    for( stringlist_t::iterator si = hotColumns.begin();
         si != hotColumns.end(); ++si )
    {
        std::string cname = *si;

        for( cols_t::iterator ci = cols.begin(); ci!=cols.end(); ++ci )
        {
            if( ci->first == cname )
            {
                cols.push_front( make_pair( ci->first, ci->second ));
                cols.erase( ci );
                break;
            }
        }
    }
    
    for( cols_t::iterator ci = cols.begin(); ci!=cols.end(); ++ci )
    {
        gtk_tree_view_append_column( GTK_TREE_VIEW( m_resultsView ), ci->second );
    }
    
}


void
AP_UnixDialog_RDFQuery::addBinding( std::map< std::string, std::string >& b )
{
    xxx_UT_DEBUGMSG(("addBinding() b.size(): %u\n", b.size()));
    if( b.size() >= C_COLUMN_ARRAY_SIZE )
    {
        return;
    }
    AP_Dialog_RDFQuery::addBinding(b);
    
    GtkTreeStore* m = m_resultsModel;
    GtkTreeIter giter;
    gtk_tree_store_append( m, &giter, 0 );

    std::map< std::string, std::string >::iterator iter = b.begin();
    std::map< std::string, std::string >::iterator  end = b.end();
    for( int i=0; iter != end; ++iter, ++i )
    {
        xxx_UT_DEBUGMSG(("addBinding() iter->second: %d\n", iter->second.c_str()));
        gtk_tree_store_set( m, &giter, i, uriToPrefixed(iter->second).c_str(), -1 );
    }
    
}



void
AP_UnixDialog_RDFQuery::onExecuteClicked()
{
    UT_DEBUGMSG(("onExecuteClicked() model1:%p\n", m_resultsModel ));
    UT_DEBUGMSG(("onExecuteClicked() model2:%p\n", gtk_tree_view_get_model( m_resultsView ) ));

    std::string q = tostr(GTK_TEXT_VIEW (m_query));
    executeQuery( q );
}

void
AP_UnixDialog_RDFQuery::onShowAllClicked()
{
    UT_DEBUGMSG(("onShowAllClicked()\n" ));
    showAllRDF();
}


void
AP_UnixDialog_RDFQuery::setStatus( const std::string& msg )
{
    gtk_label_set_text( GTK_LABEL(m_status), msg.c_str() );
}

void
AP_UnixDialog_RDFQuery::setQueryString( const std::string& sparql )
{
    GtkTextBuffer* b = gtk_text_view_get_buffer( GTK_TEXT_VIEW (m_query) );
    gtk_text_buffer_set_text( b, sparql.c_str(), -1 );
}



/*!
* Build dialog.
*/
void 
AP_UnixDialog_RDFQuery::_constructWindow (XAP_Frame * /*pFrame*/) 
{
	UT_DEBUGMSG (("ROB: _constructWindow ()\n"));		

	const XAP_StringSet *pSS = m_pApp->getStringSet();
	std::string text;

	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_RDFQuery.ui");

	m_wDialog = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_RDFQuery"));
	m_btClose = GTK_WIDGET(gtk_builder_get_object(builder, "btClose"));
	m_btExecute = GTK_WIDGET(gtk_builder_get_object(builder, "btExecute"));
    m_btShowAll = GTK_WIDGET(gtk_builder_get_object(builder, "btShowAll"));
    m_query     = GTK_WIDGET(gtk_builder_get_object(builder, "query"));
	m_resultsView  = GTK_TREE_VIEW(gtk_builder_get_object(builder, "resultsView"));
    m_status    = GTK_WIDGET(gtk_builder_get_object(builder, "status"));

    // localization
    localizeButton(m_btShowAll, pSS, AP_STRING_ID_DLG_RDF_Query_ShowAll); 
    localizeButton(m_btExecute, pSS, AP_STRING_ID_DLG_RDF_Query_Execute); 
    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_query));
    gtk_text_buffer_get_iter_at_line(buffer, &iter, 0);
    pSS->getValueUTF8(AP_STRING_ID_DLG_RDF_Query_Comment, text);
    gtk_text_buffer_insert(buffer, &iter, text.c_str(), -1);     

    GObject *selection;
    selection = G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (m_resultsView)));
    gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_MULTIPLE);
    gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW (m_resultsView), true );
    
    GtkTreeStore* m = gtk_tree_store_new( C_COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );
    gtk_tree_view_set_model( m_resultsView, GTK_TREE_MODEL( m ) );
    m_resultsModel = m;

    int colid = 0;
    GtkCellRenderer* ren = 0;

    colid = C_SUBJ_COLUMN;
    ren = gtk_cell_renderer_text_new ();
    pSS->getValueUTF8(AP_STRING_ID_DLG_RDF_Query_Column_Subject, text);
    w_cols[ colid ] = gtk_tree_view_column_new_with_attributes( text.c_str(), ren, "text", colid, NULL);
    gtk_tree_view_append_column( GTK_TREE_VIEW( m_resultsView ), w_cols[ colid ] );
    gtk_tree_view_column_set_sort_column_id( w_cols[ colid ], colid );
    gtk_tree_view_column_set_resizable ( w_cols[ colid ], true );
    
    colid = C_PRED_COLUMN;
    ren = gtk_cell_renderer_text_new ();
    pSS->getValueUTF8(AP_STRING_ID_DLG_RDF_Query_Column_Predicate, text);
    w_cols[ colid ] = gtk_tree_view_column_new_with_attributes( text.c_str(), ren, "text", colid, NULL);
    gtk_tree_view_append_column( GTK_TREE_VIEW( m_resultsView ), w_cols[ colid ] );
    gtk_tree_view_column_set_sort_column_id( w_cols[ colid ], colid );
    gtk_tree_view_column_set_resizable ( w_cols[ colid ], true );

    colid = C_OBJ_COLUMN;
    ren = gtk_cell_renderer_text_new ();
    pSS->getValueUTF8(AP_STRING_ID_DLG_RDF_Query_Column_Object, text);
    w_cols[ colid ] = gtk_tree_view_column_new_with_attributes( text.c_str(), ren, "text", colid, NULL);
    gtk_tree_view_append_column( GTK_TREE_VIEW( m_resultsView ), w_cols[ colid ] );
    gtk_tree_view_column_set_sort_column_id( w_cols[ colid ], colid );
    gtk_tree_view_column_set_resizable ( w_cols[ colid ], true );

    /////////////
	/// Signals
    ///
	g_signal_connect (GTK_BUTTON (m_btExecute), "clicked", 
					  G_CALLBACK (AP_UnixDialog_RDFQuery__onExecuteClicked), static_cast <gpointer>(this));
	g_signal_connect (GTK_BUTTON (m_btShowAll), "clicked", 
					  G_CALLBACK (AP_UnixDialog_RDFQuery__onShowAllClicked), static_cast <gpointer>(this));
	g_signal_connect (GTK_DIALOG (m_wDialog), "response",
					  G_CALLBACK (AP_UnixDialog_RDFQuery__onDialogResponse), static_cast <gpointer>(this));
	g_signal_connect (m_wDialog, "delete-event",
					  G_CALLBACK (AP_UnixDialog_RDFQuery__onDeleteWindow), static_cast <gpointer>(this));

#ifndef WITH_REDLAND
	gtk_widget_set_sensitive(m_btExecute, FALSE);  
	gtk_widget_set_sensitive(m_btShowAll, FALSE);  
#endif

	g_object_unref(G_OBJECT(builder));
}

/*!
* Update dialog's data.
*/
void 
AP_UnixDialog_RDFQuery::_updateWindow ()
{
    UT_DEBUGMSG(("RDFQuery::_updateWindow()\n"));
	ConstructWindowName ();
	gtk_window_set_title (GTK_WINDOW (m_wDialog), m_WindowName.c_str() );
}

void 
AP_UnixDialog_RDFQuery::runModeless (XAP_Frame * pFrame)
{
	UT_DEBUGMSG (("MIQ: runModeless ()\n"));
	_constructWindow (pFrame);
	UT_ASSERT (m_wDialog);
	_updateWindow ();
	abiSetupModelessDialog (GTK_DIALOG (m_wDialog), pFrame, this, GTK_RESPONSE_CLOSE);
	gtk_widget_show_all (m_wDialog);
	gtk_window_present (GTK_WINDOW (m_wDialog));
}

void 
AP_UnixDialog_RDFQuery::notifyActiveFrame (XAP_Frame * /*pFrame*/)
{
	UT_DEBUGMSG (("MIQ: notifyActiveFrame ()\n"));
	UT_ASSERT (m_wDialog);
	_updateWindow ();
}

void 
AP_UnixDialog_RDFQuery::activate (void)
{
	UT_ASSERT (m_wDialog);
	UT_DEBUGMSG (("MIQ: AP_UnixDialog_RDFQuery::activate ()\n"));
	_updateWindow ();
	gtk_window_present (GTK_WINDOW (m_wDialog));
}

void 
AP_UnixDialog_RDFQuery::destroy ()
{
	UT_DEBUGMSG (("MIQ: AP_UnixDialog_RDFQuery::destroy ()\n"));
	modeless_cleanup ();
	if (m_wDialog) {
		gtk_widget_destroy (m_wDialog);
		m_wDialog = NULL;
	}
}

