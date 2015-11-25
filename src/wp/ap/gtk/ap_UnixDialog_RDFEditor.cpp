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
#include "ut_go_file.h"

#include "xap_Dialog_Id.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_RDFEditor.h"
#include "ap_UnixDialog_RDFEditor.h"
#include "pd_RDFSupport.h"
#include "xap_GtkComboBoxHelpers.h"

#include <sstream>

const char* GOBJ_COL_NUM = "GOBJ_COL_NUM";


void
AP_UnixDialog_RDFEditor__onActionNew ( GtkAction*, gpointer data )
{
	AP_UnixDialog_RDFEditor *dlg = static_cast <AP_UnixDialog_RDFEditor*>(data);
    dlg->createStatement();
}
void
AP_UnixDialog_RDFEditor__onActionCopy ( GtkAction*, gpointer data )
{
	AP_UnixDialog_RDFEditor *dlg = static_cast <AP_UnixDialog_RDFEditor*>(data);
    dlg->copyStatement();
}


static void s_OnXMLIDChanged(GtkWidget * widget, AP_UnixDialog_RDFEditor* dlg)
{
	UT_UNUSED(widget);
	UT_ASSERT(widget && dlg);

    std::string xmlid = XAP_comboBoxGetActiveText( GTK_COMBO_BOX( widget ));
   	dlg->setRestrictedXMLID( xmlid );
}
    

void cell_edited_cb ( GtkCellRendererText *cell,
                      gchar *path_string,
                      gchar *new_text,
                      gpointer data )
{
	AP_UnixDialog_RDFEditor *dlg = static_cast <AP_UnixDialog_RDFEditor *>(data);
    int cidx = GPOINTER_TO_INT(g_object_get_data( G_OBJECT(cell), GOBJ_COL_NUM ));
    dlg->onCellEdited( cell, path_string, new_text, cidx );
}


void
AP_UnixDialog_RDFEditor__onActionDelete( GtkAction*, gpointer data )
{
    
	AP_UnixDialog_RDFEditor *dlg = static_cast <AP_UnixDialog_RDFEditor*>(data);
    dlg->onDelClicked();
}
void
AP_UnixDialog_RDFEditor__onActionImportRDFXML( GtkAction*, gpointer data )
{
    xxx_UT_DEBUGMSG(("_onActionImportRDFXML()\n"))
	AP_UnixDialog_RDFEditor *dlg = static_cast <AP_UnixDialog_RDFEditor*>(data);
    dlg->onImportRDFXML();
}
void
AP_UnixDialog_RDFEditor__onActionExportRDFXML( GtkAction*, gpointer data )
{
    
	AP_UnixDialog_RDFEditor *dlg = static_cast <AP_UnixDialog_RDFEditor*>(data);
    dlg->onExportRDFXML();
}



void
AP_UnixDialog_RDFEditor__onShowAllClicked ( GtkButton * /*button*/,
                                            gpointer   data )
{
	AP_UnixDialog_RDFEditor *dlg = static_cast <AP_UnixDialog_RDFEditor *>(data);
	dlg->onShowAllClicked ();
}



/*!
* Event dispatcher for button "close".
*/
void
AP_UnixDialog_RDFEditor__onDialogResponse ( GtkDialog * /*dialog*/,
                                            gint 		response,
                                            gpointer  data )
{
	AP_UnixDialog_RDFEditor *dlg = static_cast <AP_UnixDialog_RDFEditor *>(data);
	if (response == GTK_RESPONSE_CLOSE)
    {
		dlg->destroy ();		
	}
}

/*!
* Event dispatcher for window.
*/
gboolean
AP_UnixDialog_RDFEditor__onDeleteWindow ( GtkWidget * /*widget*/,
                                          GdkEvent  * /*event*/,
                                          gpointer  data )
{
	AP_UnixDialog_RDFEditor *dlg = static_cast <AP_UnixDialog_RDFEditor *>(data);
	if (dlg->getWindow ())
    {
		dlg->destroy ();
	}
	return TRUE;
}

gboolean
AP_UnixDialog_RDFEditor__onCursorChanged ( GtkTreeView*,
                                           gpointer  data )
{
	AP_UnixDialog_RDFEditor *dlg = static_cast <AP_UnixDialog_RDFEditor *>(data);
    dlg->onCursorChanged();
    return TRUE;
}


/*!
* Static ctor.
*/
XAP_Dialog * 
AP_UnixDialog_RDFEditor::static_constructor( XAP_DialogFactory *pFactory,
                                             XAP_Dialog_Id 	 id )
{
	AP_UnixDialog_RDFEditor *dlg = new AP_UnixDialog_RDFEditor (pFactory, id);
	return dlg;
}

/*!
* Ctor.
*/
AP_UnixDialog_RDFEditor::AP_UnixDialog_RDFEditor( XAP_DialogFactory *pDlgFactory,
                                                  XAP_Dialog_Id 	 id )
	: AP_Dialog_RDFEditor   (pDlgFactory, id)
    , m_wDialog 	   (0)
    , m_btClose 	   (0)
    , m_btShowAll      (0)
    , m_resultsView    (0)
	, m_resultsModel   (0)
    , m_status         (0)
    , m_anewtriple     (0)
    , m_acopytriple    (0)
    , m_adeletetriple  (0)
    , m_aimportrdfxml  (0)
    , m_aexportrdfxml  (0)
    , m_selectedxmlid  (0)
    , m_restrictxmlidhidew (0)
{
}

/*!
* Dtor.
*/
AP_UnixDialog_RDFEditor::~AP_UnixDialog_RDFEditor ()
{
	UT_DEBUGMSG (("~AP_UnixDialog_RDFEditor ()\n"));
}



void
AP_UnixDialog_RDFEditor::clear()
{
    AP_Dialog_RDFEditor::clear();
    gtk_tree_store_clear( m_resultsModel );
}

void
AP_UnixDialog_RDFEditor::addStatement( const PD_RDFStatement& stc )
{
    AP_Dialog_RDFEditor::addStatement(stc);
    PD_RDFStatement st = stc.uriToPrefixed( getModel() );
    
    GtkTreeStore* m = m_resultsModel;
    GtkTreeIter iter;
    gtk_tree_store_append( m, &iter, 0 );
    gtk_tree_store_set( m, &iter,
                        C_SUBJ_COLUMN, st.getSubject().  toString().c_str(),
                        C_PRED_COLUMN, st.getPredicate().toString().c_str(),
                        C_OBJ_COLUMN,  st.getObject().   toString().c_str(),
                        -1 );
}

PD_RDFStatement
AP_UnixDialog_RDFEditor::GIterToStatement( GtkTreeIter* giter )
{
    gchar* s;
    gchar* p;
    gchar* o;
    gtk_tree_model_get( GTK_TREE_MODEL(m_resultsModel), giter,
                        C_SUBJ_COLUMN, &s,
                        C_PRED_COLUMN, &p,
                        C_OBJ_COLUMN,  &o,
                        -1 );
    PD_RDFStatement st( getModel(), PD_URI(s), PD_URI(p), PD_Object(o) );
    return st;
}

GtkTreeIter
AP_UnixDialog_RDFEditor::getGIter( PD_RDFStatement st )
{
    GtkTreeModel* model = GTK_TREE_MODEL( m_resultsModel );
     
    GtkTreeIter ret;
    memset( &ret, 0, sizeof(GtkTreeIter));
    gtk_tree_model_get_iter_first( model, &ret );
    for( ; true; )
    {
        PD_RDFStatement stg = GIterToStatement( &ret );
        if( stg == st )
        {
            return ret;
        }
        
        if( !gtk_tree_model_iter_next( model, &ret) )
            break;
    }
    
    return ret;
}


void
AP_UnixDialog_RDFEditor::setSelection( const std::list< PD_RDFStatement >& l )
{
    for( std::list< PD_RDFStatement >::const_iterator iter = l.begin();
         iter != l.end(); ++iter )
    {
        GtkTreeIter giter = getGIter( *iter );
        selectIter( m_resultsView, &giter );
    }

    if( !l.empty() )
    {
        std::list< PD_RDFStatement >::const_iterator iter = l.begin();
        GtkTreeIter giter = getGIter( *iter );
        scrollToIter( m_resultsView, &giter );
    }
}


void
AP_UnixDialog_RDFEditor::hideRestrictionXMLID( bool v )
{
    AP_Dialog_RDFEditor::hideRestrictionXMLID( v );

	// check that the UI is actually loaded.
	if(!m_wDialog) {
		return;
	}

    if( v )
    {
        UT_DEBUGMSG(("AP_UnixDialog_RDFEditor, no restriction HIDING! w:%p\n", m_restrictxmlidhidew ));
        gtk_widget_hide( m_restrictxmlidhidew );
        gtk_widget_hide( GTK_WIDGET(m_selectedxmlid) );
    }
    else
    {
        PD_RDFModelHandle model;
        std::set< std::string > xmlids;
        getRDF()->addRelevantIDsForPosition( xmlids, getView()->getPoint() );
        UT_DEBUGMSG(("AP_UnixDialog_RDFEditor, have restricted xmlids size:%lu\n", (long unsigned)xmlids.size() ));
        
		/// FIXME...
		setRestrictedModel( model );
    }
    
}





void
AP_UnixDialog_RDFEditor::onShowAllClicked()
{
    UT_DEBUGMSG(("onShowAllClicked()\n" ));
    showAllRDF();
}

PD_RDFStatement
AP_UnixDialog_RDFEditor::next( const PD_RDFStatement& st )
{
    GtkTreeIter giter = getGIter( st );
    if( gtk_tree_model_iter_next( GTK_TREE_MODEL(m_resultsModel), &giter ) )
    {
        PD_RDFStatement ret = GIterToStatement( &giter );
        return ret;
    }
    
    // no good old chum
    PD_RDFStatement ret;
    return ret;
}

void
AP_UnixDialog_RDFEditor::onDelClicked()
{
    UT_DEBUGMSG(("onDelClicked()\n" ));
    std::list< PD_RDFStatement > l = getSelection();
    if( l.empty() )
        return;

    PD_RDFStatement n;
    if( l.size() == 1 )
    {
        n = next( l.front() );
    }
    
    PD_DocumentRDFMutationHandle m = getModel()->createMutation();
    for( std::list< PD_RDFStatement >::iterator iter = l.begin(); iter != l.end(); ++iter )
    {
        const PD_RDFStatement& st = *iter;
        xxx_UT_DEBUGMSG(("onDelClicked() removing statement: %s\n", st.toString().utf8_str()));
        m->remove( st );
        removeStatement( st );
        m_count--;
    }
    m->commit();
//    showAllRDF();

    if( n.isValid() )
    {
        std::list< PD_RDFStatement > zz;
        zz.push_back( n );
        setSelection( zz );
    }

    statusIsTripleCount();
}


void
AP_UnixDialog_RDFEditor::onCellEdited( GtkCellRendererText * /*cell*/,
                                       gchar *path_string,
                                       gchar *new_text,
                                       int cidx )
{
    xxx_UT_DEBUGMSG(("onCellEdited() nt: %s\n", new_text));

    GtkTreeIter giter;
    GtkTreeModel* model = GTK_TREE_MODEL( m_resultsModel );
    GtkTreePath*  path  = gtk_tree_path_new_from_string (path_string);
    gtk_tree_model_get_iter (model, &giter, path);

    PD_URI n( new_text );
    n = n.prefixedToURI( getModel() );
    
    PD_RDFStatement oldst = GIterToStatement( &giter );
    PD_RDFStatement newst;
    switch( cidx )
    {
        case C_SUBJ_COLUMN:
            newst = PD_RDFStatement( n, oldst.getPredicate(), oldst.getObject() );
            break;
        case C_PRED_COLUMN:
            newst = PD_RDFStatement( oldst.getSubject(), n, oldst.getObject() );
            break;
        case C_OBJ_COLUMN:
            newst = PD_RDFStatement( oldst.getSubject(), oldst.getPredicate(), PD_Object( n.toString() ) );
            break;
        default:
            UT_ASSERT_NOT_REACHED();
    }
    
    PD_DocumentRDFMutationHandle m = getModel()->createMutation();
    if( m->add( newst ) )
    {
        m->remove( oldst );
        m->commit();
        gtk_tree_store_set (GTK_TREE_STORE (model), &giter, cidx, new_text, -1);
    }
    gtk_tree_path_free (path);
    
}

static std::string tostr( GsfInput* gsf )
{
    gsf_off_t sz = gsf_input_size( gsf );
    guint8 const * d =  gsf_input_read( gsf, sz, 0 );
    std::string ret = std::string((char*)d);
    return ret;
}


void
AP_UnixDialog_RDFEditor::onImportRDFXML()
{
    xxx_UT_DEBUGMSG(("onImportRDFXML()\n"));

    UT_runDialog_AskForPathname afp( XAP_DIALOG_ID_FILE_IMPORT );
    afp.appendFiletype( "RDF/XML Triple File", "rdf" );

    if( afp.run( getActiveFrame() ) )
    {
        xxx_UT_DEBUGMSG(("onImportRDFXML() path: %s", afp.getPath().utf8_str()));
        GError* err = 0;
        GsfInput* gsf = UT_go_file_open( afp.getPath().c_str(), &err );
        std::string rdfxml = tostr( gsf );
        g_object_unref (G_OBJECT (gsf));

        xxx_UT_DEBUGMSG(("rdfxml: %s\n", rdfxml.c_str()));
        PD_DocumentRDFMutationHandle m = getModel()->createMutation();
		// FIXME check the error code
        /* UT_Error e =*/ loadRDFXML( m, rdfxml );
        m->commit();

        xxx_UT_DEBUGMSG(("count of triples: %ld\n", getModel()->size()));
        showAllRDF();
    }
    gtk_window_present( GTK_WINDOW( m_wDialog ));
}

void
AP_UnixDialog_RDFEditor::onExportRDFXML()
{
    xxx_UT_DEBUGMSG(("onExportRDFXML()\n"));

    UT_runDialog_AskForPathname afp( XAP_DIALOG_ID_FILE_EXPORT );
    afp.appendFiletype( "RDF/XML Triple File", "rdf" );
    afp.setDefaultFiletype( "RDF/XML Triple File" );

    if( afp.run( getActiveFrame() ) )
    {
        xxx_UT_DEBUGMSG(("onExportRDFXML() path: %s\n", afp.getPath().utf8_str()));
        std::string rdfxml = toRDFXML( getModel() );
        GError* err = 0;
        GsfOutput* gsf = UT_go_file_create( afp.getPath().c_str(), &err );
        gsf_output_write( gsf, rdfxml.size(), (const guint8*)rdfxml.data() );
        gsf_output_close( gsf );
    }
    gtk_window_present( GTK_WINDOW( m_wDialog ));
}



void
AP_UnixDialog_RDFEditor::setStatus( const std::string& msg )
{
    gtk_label_set_text( GTK_LABEL(m_status), msg.c_str() );
}



/*!
* Build dialog.
*/
void 
AP_UnixDialog_RDFEditor::_constructWindow (XAP_Frame * /*pFrame*/) 
{
	UT_DEBUGMSG (("MIQ: _constructWindow ()\n"));		

	const XAP_StringSet *pSS = m_pApp->getStringSet();
	std::string text;

	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_RDFEditor.ui");

	m_wDialog = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_RDFEditor"));
	m_btClose = GTK_WIDGET(gtk_builder_get_object(builder, "btClose"));
    m_btShowAll = GTK_WIDGET(gtk_builder_get_object(builder, "btShowAll"));
	m_resultsView   = GTK_TREE_VIEW(gtk_builder_get_object(builder, "resultsView"));
    m_status        = GTK_WIDGET(gtk_builder_get_object(builder, "status"));
    m_anewtriple    = GTK_ACTION(gtk_builder_get_object(builder, "anewtriple"));
    m_acopytriple   = GTK_ACTION(gtk_builder_get_object(builder, "acopytriple"));
    m_adeletetriple = GTK_ACTION(gtk_builder_get_object(builder, "adeletetriple"));
    m_aimportrdfxml = GTK_ACTION(gtk_builder_get_object(builder, "aimportrdfxml"));
    m_aexportrdfxml = GTK_ACTION(gtk_builder_get_object(builder, "aexportrdfxml"));
    m_selectedxmlid = GTK_COMBO_BOX(gtk_builder_get_object(builder, "selectedxmlid"));
    m_restrictxmlidhidew = GTK_WIDGET(gtk_builder_get_object(builder, "restrictxmlidhidew"));

    // localization
    localizeMenuItem(GTK_WIDGET(gtk_builder_get_object(builder, "filemenuitem")), pSS, AP_STRING_ID_DLG_RDF_Editor_Menu_File);
    localizeMenuItem(GTK_WIDGET(gtk_builder_get_object(builder, "editmenuitem")), pSS, AP_STRING_ID_DLG_RDF_Editor_Menu_Triple);
    localizeButton(m_btShowAll, pSS, AP_STRING_ID_DLG_RDF_Editor_ShowAll);
    localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbRestrict")), pSS, AP_STRING_ID_DLG_RDF_Editor_Restrict);

    // add three dots to menu items raising a dialog
    text = gtk_action_get_label(m_aimportrdfxml);
    text += "...";
    gtk_action_set_label(m_aimportrdfxml, text.c_str());
    text = gtk_action_get_label(m_aexportrdfxml);
    text += "...";
    gtk_action_set_label(m_aexportrdfxml, text.c_str());

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
    g_object_set(ren, "editable", 1, 0, NULL);
    g_object_set_data( G_OBJECT(ren), GOBJ_COL_NUM,  GINT_TO_POINTER(colid));
    g_signal_connect_data( G_OBJECT( ren ), "edited",
                           G_CALLBACK (cell_edited_cb),
                           (gpointer)this, 0, GConnectFlags(0));
    pSS->getValueUTF8(AP_STRING_ID_DLG_RDF_Query_Column_Subject, text);
    w_cols[ colid ] = gtk_tree_view_column_new_with_attributes( text.c_str(), ren, "text", colid, NULL);
    gtk_tree_view_append_column( GTK_TREE_VIEW( m_resultsView ), w_cols[ colid ] );
    gtk_tree_view_column_set_sort_column_id( w_cols[ colid ], colid );
    gtk_tree_view_column_set_resizable ( w_cols[ colid ], true );
    
    colid = C_PRED_COLUMN;
    ren = gtk_cell_renderer_text_new ();
    g_object_set(ren, "editable", 1, 0, NULL );
    g_object_set_data( G_OBJECT(ren), GOBJ_COL_NUM, GINT_TO_POINTER(colid) );
    g_signal_connect_data( G_OBJECT( ren ), "edited",
                           G_CALLBACK (cell_edited_cb),
                           (gpointer)this, 0, GConnectFlags(0));
    pSS->getValueUTF8(AP_STRING_ID_DLG_RDF_Query_Column_Predicate, text);
    w_cols[ colid ] = gtk_tree_view_column_new_with_attributes( text.c_str(), ren, "text", colid, NULL);
    gtk_tree_view_append_column( GTK_TREE_VIEW( m_resultsView ), w_cols[ colid ] );
    gtk_tree_view_column_set_sort_column_id( w_cols[ colid ], colid );
    gtk_tree_view_column_set_resizable ( w_cols[ colid ], true );

    colid = C_OBJ_COLUMN;
    ren = gtk_cell_renderer_text_new ();
    g_object_set(ren, "editable", 1, 0, NULL );
    g_object_set_data( G_OBJECT(ren), GOBJ_COL_NUM, GINT_TO_POINTER(colid) );
    g_signal_connect_data( G_OBJECT( ren ), "edited",
                           G_CALLBACK (cell_edited_cb),
                           (gpointer)this, 0, GConnectFlags(0));
    pSS->getValueUTF8(AP_STRING_ID_DLG_RDF_Query_Column_Object, text);
    w_cols[ colid ] = gtk_tree_view_column_new_with_attributes( text.c_str(), ren, "text", colid, NULL);
    gtk_tree_view_append_column( GTK_TREE_VIEW( m_resultsView ), w_cols[ colid ] );
    gtk_tree_view_column_set_sort_column_id( w_cols[ colid ], colid );
    gtk_tree_view_column_set_resizable ( w_cols[ colid ], true );


    if( m_hideRestrictionXMLID )
    {
        UT_DEBUGMSG(("AP_UnixDialog_RDFEditor, no restriction HIDING! w:%p\n", m_restrictxmlidhidew ));
        if( GtkWidget* w = GTK_WIDGET(gtk_builder_get_object(builder, "topvbox")))
        {
            gtk_container_remove( GTK_CONTAINER(w),  m_restrictxmlidhidew );
        }
//        gtk_widget_hide( m_restrictxmlidhidew );
//        gtk_widget_hide( GTK_WIDGET(m_selectedxmlid) );
        setRestrictedXMLID( "" );
    }
    else
    {
        XAP_makeGtkComboBoxText( m_selectedxmlid, G_TYPE_INT );

        PT_DocPosition point = getView()->getPoint();
        if( PD_DocumentRDFHandle rdf = getRDF() )
        {
            std::set< std::string > xmlids;
            rdf->addRelevantIDsForPosition( xmlids, point );

            bool combined = false; 
            std::stringstream combinedxmlidss;
            for( std::set< std::string >::const_iterator iter = xmlids.begin();
                 iter != xmlids.end(); ++iter )
            {
                if( iter != xmlids.begin() )
                {
                    combinedxmlidss << ",";
                    combined = true;
                }
                combinedxmlidss << *iter;
            }
            XAP_appendComboBoxTextAndInt( m_selectedxmlid, combinedxmlidss.str().c_str(), 0 );
            setRestrictedXMLID( combinedxmlidss.str() );
            
            if (combined)
            {
                int idx = 1;
                for( std::set< std::string >::const_iterator iter = xmlids.begin();
                     iter != xmlids.end(); ++iter, ++idx )
                {
                    XAP_appendComboBoxTextAndInt( m_selectedxmlid, iter->c_str(), idx );
                }

                gtk_combo_box_set_active( m_selectedxmlid, 0 );
        
                // std::list< std::string > xmlids;
                // getRDF()->addRelevantIDsForPosition( xmlids, getView()->getPoint() );
                // UT_DEBUGMSG(("AP_UnixDialog_RDFEditor, have restricted xmlids size:%d\n", xmlids.size() ));
                // if( !xmlids.empty() )
                // {
                //     setRestrictedXMLID( xmlids.front() );
                // }
                
                g_signal_connect(G_OBJECT(m_selectedxmlid),
                                 "changed",
                                 G_CALLBACK(s_OnXMLIDChanged),
                                 (gpointer) this);
            }
            else gtk_container_remove(GTK_CONTAINER(gtk_builder_get_object(builder, "topvbox")),  m_restrictxmlidhidew);
        }
    }
    
    
    /////////////
	/// Signals
    ///
	g_signal_connect (GTK_BUTTON (m_btShowAll), "clicked", 
					  G_CALLBACK (AP_UnixDialog_RDFEditor__onShowAllClicked), static_cast <gpointer>(this));
	g_signal_connect (m_anewtriple, "activate", 
					  G_CALLBACK (AP_UnixDialog_RDFEditor__onActionNew), static_cast <gpointer>(this));
	g_signal_connect (m_acopytriple, "activate", 
					  G_CALLBACK (AP_UnixDialog_RDFEditor__onActionCopy), static_cast <gpointer>(this));
	g_signal_connect (m_adeletetriple, "activate", 
					  G_CALLBACK (AP_UnixDialog_RDFEditor__onActionDelete), static_cast <gpointer>(this));
	g_signal_connect (m_aimportrdfxml, "activate", 
					  G_CALLBACK (AP_UnixDialog_RDFEditor__onActionImportRDFXML), static_cast <gpointer>(this));
	g_signal_connect (m_aexportrdfxml, "activate", 
					  G_CALLBACK (AP_UnixDialog_RDFEditor__onActionExportRDFXML), static_cast <gpointer>(this));
    g_signal_connect (GTK_DIALOG (m_wDialog), "response",
					  G_CALLBACK (AP_UnixDialog_RDFEditor__onDialogResponse), static_cast <gpointer>(this));
	g_signal_connect (m_wDialog, "delete-event",
					  G_CALLBACK (AP_UnixDialog_RDFEditor__onDeleteWindow), static_cast <gpointer>(this));
	g_signal_connect (m_resultsView, "cursor-changed",
					  G_CALLBACK (AP_UnixDialog_RDFEditor__onCursorChanged), static_cast <gpointer>(this));

#ifndef WITH_REDLAND
	gtk_action_set_sensitive(m_aimportrdfxml, FALSE);  
	gtk_action_set_sensitive(m_aexportrdfxml, FALSE);  
#endif

	g_object_unref(G_OBJECT(builder));
}

/*!
* Update dialog's data.
*/
void 
AP_UnixDialog_RDFEditor::_updateWindow ()
{
    UT_DEBUGMSG(("RDFEditor::_updateWindow()\n"));
	ConstructWindowName ();
	gtk_window_set_title (GTK_WINDOW (m_wDialog), m_WindowName.c_str() );
}

void 
AP_UnixDialog_RDFEditor::runModeless (XAP_Frame * pFrame)
{
	UT_DEBUGMSG (("MIQ: runModeless ()\n"));
	_constructWindow (pFrame);
	UT_ASSERT (m_wDialog);
	_updateWindow ();
	abiSetupModelessDialog (GTK_DIALOG (m_wDialog), pFrame, this, GTK_RESPONSE_CLOSE);
    showAllRDF();
	gtk_widget_show_all (m_wDialog);
	gtk_window_present (GTK_WINDOW (m_wDialog));
}

void 
AP_UnixDialog_RDFEditor::notifyActiveFrame (XAP_Frame * /*pFrame*/)
{
	UT_DEBUGMSG (("MIQ: notifyActiveFrame ()\n"));
	UT_ASSERT (m_wDialog);
	_updateWindow ();
}

void 
AP_UnixDialog_RDFEditor::activate (void)
{
	UT_ASSERT (m_wDialog);
	UT_DEBUGMSG (("MIQ: AP_UnixDialog_RDFEditor::activate ()\n"));
	_updateWindow ();
	gtk_window_present (GTK_WINDOW (m_wDialog));
}

void 
AP_UnixDialog_RDFEditor::destroy ()
{
	UT_DEBUGMSG (("MIQ: AP_UnixDialog_RDFEditor::destroy ()\n"));
	modeless_cleanup ();
	if (m_wDialog) {
		gtk_widget_destroy (m_wDialog);
		m_wDialog = NULL;
	}
}

void
AP_UnixDialog_RDFEditor::removeStatement( const PD_RDFStatement& st )
{
    GtkTreeIter giter = getGIter( st );
    gtk_tree_store_remove( m_resultsModel, &giter );
}

std::list< PD_RDFStatement >
AP_UnixDialog_RDFEditor::getSelection()
{
    std::list< PD_RDFStatement > ret;
    GtkTreeModel* model = GTK_TREE_MODEL(m_resultsModel);
    list_gtktreeiter_t l = getIterList( GTK_WIDGET(m_resultsView), true );
    for( list_gtktreeiter_t::iterator iter = l.begin(); iter != l.end(); ++iter )
    {
        GtkTreeIter giter = *iter;
        gchar* s;
        gchar* p;
        gchar* o;
        
        gtk_tree_model_get( model, &giter,
                            C_SUBJ_COLUMN, &s,
                            C_PRED_COLUMN, &p,
                            C_OBJ_COLUMN,  &o,
                            -1 );

        PD_RDFStatement st( getModel(), PD_URI(s), PD_URI(p), PD_Object(o) );
        ret.push_back( st );
        xxx_UT_DEBUGMSG(("getSelection() st: %s\n", st.toString().utf8_str()));
    }
    
    return ret;
}

void
AP_UnixDialog_RDFEditor::onCursorChanged()
{
    xxx_UT_DEBUGMSG(("onCursorChanged()\n"));
    PD_URI pkg_idref("http://docs.oasis-open.org/opendocument/meta/package/common#idref");
    PD_DocumentRDFHandle rdf = getRDF();
    PD_RDFModelHandle  model = getModel();
    
    std::list< PD_RDFStatement > sl = getSelection();
    if( !sl.empty() )
    {
        for( std::list< PD_RDFStatement >::iterator siter = sl.begin();
             siter != sl.end(); ++siter )
        {
            xxx_UT_DEBUGMSG((" subj: %s\n", siter->getSubject().toString().utf8_string()));
            PD_ObjectList ul = model->getObjects( siter->getSubject(), pkg_idref );
            for( PD_ObjectList::iterator uiter = ul.begin(); uiter != ul.end(); ++uiter )
            {
                std::string xmlid = uiter->toString();
                
                xxx_UT_DEBUGMSG((" xmlid: %s\n", xmlid.c_str()));
                std::pair< PT_DocPosition, PT_DocPosition > range = rdf->getIDRange( xmlid );
                xxx_UT_DEBUGMSG((" start: %d end: %d\n", range.first, range.second));
                getView()->cmdSelect( range );
                
            }
        }
    }
}

