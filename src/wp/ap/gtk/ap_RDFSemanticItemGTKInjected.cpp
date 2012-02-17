/* AbiWord
 * Copyright (C) Ben Martin 2012.
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

#include "GTKCommon.h"
#include "ap_RDFSemanticItemGTKInjected.h"
#include "pd_Document.h"
#include "fv_View.h"
#include "xap_UnixDialogHelper.h"

// compile regardless
 #if GTK_MAJOR_VERSION < 3
const gchar* gtk_combo_box_get_active_id( GtkComboBox* v )
{
    return 0;
}
#endif

void GDestroyNotify_GObjectSemItem(gpointer data)
{
    ap_GObjectSemItem* obj = (ap_GObjectSemItem*)data;
    delete obj;
}

PD_RDFSemanticItemHandle getHandle(GtkDialog* d)
{
    ap_GObjectSemItem* data = (ap_GObjectSemItem*)
        g_object_get_data( G_OBJECT(d), G_OBJECT_SEMITEM );
    return data->h;
}

void OnSemItemEdited ( GtkDialog* d, gint /*response_id*/, 
					   gpointer /*user_data*/)
{
    UT_DEBUGMSG(("OnSemItemEdited()\n"));
    PD_RDFSemanticItemHandle h = getHandle( d );
    h->updateFromEditorData();
    gtk_widget_destroy( GTK_WIDGET(d) );
}



void GDestroyNotify_GObjectSemItem_List(gpointer data)
{
    ap_GObjectSemItem_List* obj = (ap_GObjectSemItem_List*)data;
    delete obj;
}
PD_RDFSemanticItems getSemItemListHandle(GtkDialog* d)
{
    ap_GObjectSemItem_List* data = (ap_GObjectSemItem_List*)
        g_object_get_data( G_OBJECT(d), G_OBJECT_SEMITEM_LIST );
    return data->cl;
}
void OnSemItemListEdited ( GtkDialog* d, gint response_id, 
						   gpointer /*user_data*/)
{
    UT_DEBUGMSG(("OnSemItemListEdited() response_id:%d\n", response_id ));
    if( response_id != GTK_RESPONSE_DELETE_EVENT )
    {
        PD_RDFSemanticItems cl = getSemItemListHandle( d );
        for( PD_RDFSemanticItems::iterator ci = cl.begin(); ci != cl.end(); ++ci )
        {
            PD_RDFSemanticItemHandle c = *ci;
            c->updateFromEditorData();
        }
    }
    gtk_widget_destroy( GTK_WIDGET(d) );
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


static void
OnSemanticStylesheetsDialogResponse( GtkWidget* dialog,
                                     GtkTreeView* /*tree*/,
                                     FV_View* pView )
{
    PD_Document* pDoc = pView->getDocument();
    PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();
    gtk_widget_destroy(dialog);
}


static void
ApplySemanticStylesheets( const std::string& semItemClassRestriction,
                          const std::string& ssName )
{
    // set the RDF linking to the stylesheets
    std::list< AD_Document* > dl = XAP_App::getApp()->getDocuments();
    for( std::list< AD_Document* >::iterator diter = dl.begin(); diter != dl.end(); ++diter )
    {
        PD_Document* pDoc = dynamic_cast<PD_Document*>(*diter);
        pDoc->beginUserAtomicGlob();
        
        PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();
        PD_RDFSemanticItems   sl = rdf->getAllSemanticObjects( semItemClassRestriction );

        for( PD_RDFSemanticItems::iterator siter = sl.begin(); siter != sl.end(); ++siter )
        {
            PD_RDFSemanticItemHandle si = *siter;
            PD_RDFSemanticStylesheetHandle ss = si->findStylesheetByName(
                PD_RDFSemanticStylesheet::stylesheetTypeSystem(), ssName );

            std::set< std::string > xmlids = si->getXMLIDs();
            for( std::set< std::string >::iterator xiter = xmlids.begin(); xiter != xmlids.end(); ++xiter )
            {
                std::string xmlid = *xiter;
                PD_RDFSemanticItemViewSite vs( si, xmlid );
                vs.setStylesheetWithoutReflow( ss );
            }
        }
        pDoc->endUserAtomicGlob();
    }

    UT_DEBUGMSG(("ApplySemanticStylesheets(reflowing)\n" ));

    // reflow all the viewsites
    for( std::list< AD_Document* >::iterator diter = dl.begin(); diter != dl.end(); ++diter )
    {
        PD_Document* pDoc = dynamic_cast<PD_Document*>(*diter);
        pDoc->beginUserAtomicGlob();
        pDoc->notifyPieceTableChangeStart();
        pDoc->setDontImmediatelyLayout(true);
        
        PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();
        PD_RDFSemanticItems   sl = rdf->getAllSemanticObjects( semItemClassRestriction );

        std::list<AV_View*> vl = pDoc->getAllViews();
        for( std::list<AV_View*>::iterator viter = vl.begin(); viter != vl.end(); ++viter )
        {
            FV_View* pView = dynamic_cast<FV_View*>(*viter);

            for( PD_RDFSemanticItems::iterator siter = sl.begin(); siter != sl.end(); ++siter )
            {
                PD_RDFSemanticItemHandle si = *siter;
                std::set< std::string > xmlids = si->getXMLIDs();
                for( std::set< std::string >::iterator xiter = xmlids.begin(); xiter != xmlids.end(); ++xiter )
                {
                    std::string xmlid = *xiter;
                    PD_RDFSemanticItemViewSite vs( si, xmlid );
                    vs.reflowUsingCurrentStylesheet( pView );
                }
            }
            break;
        }
        
        pDoc->setDontImmediatelyLayout(false);
        pDoc->notifyPieceTableChangeEnd();
        pDoc->endUserAtomicGlob();
    }

    UT_DEBUGMSG(("ApplySemanticStylesheets(done)\n" ));
}


static gboolean
OnSemanticStylesheetsSetContacts_cb( GtkWidget* /*w*/, GdkEvent* /*event*/, 
									 GtkComboBoxText *combo_box )
{
    const gchar * t = gtk_combo_box_get_active_id( GTK_COMBO_BOX(combo_box) );
    std::string ssName = t ? t : "name";

    UT_DEBUGMSG(("OnSemanticStylesheetsSetContacts_cb() ssName:%s\n", ssName.c_str() ));
    UT_DEBUGMSG(("OnSemanticStylesheetsSetContacts_cb() combo:%p\n", combo_box ));
    UT_DEBUGMSG(("OnSemanticStylesheetsSetContacts_cb() t:%s\n", t ));

    ApplySemanticStylesheets( "Contact", ssName );
    
    return false;
}


static gboolean
OnSemanticStylesheetsSetEvents_cb( GtkWidget* /*w*/, GdkEvent* /*event*/, 
								   GtkComboBoxText *combo_box )
{
    const gchar * t = gtk_combo_box_get_active_id( GTK_COMBO_BOX(combo_box) );
    std::string ssName = t ? t : "name";

    UT_DEBUGMSG(("OnSemanticStylesheetsSetEvents_cb() ssName:%s\n", ssName.c_str() ));
    UT_DEBUGMSG(("OnSemanticStylesheetsSetEvents_cb() combo:%p\n", combo_box ));
    UT_DEBUGMSG(("OnSemanticStylesheetsSetEvents_cb() t:%s\n", t ));

    ApplySemanticStylesheets( "Event", ssName );
    return false;
}

static gboolean
OnSemanticStylesheetsSetLocations_cb( GtkWidget* /*w*/, GdkEvent* /*event*/, 
									  GtkComboBoxText *combo_box )
{
    const gchar * t = gtk_combo_box_get_active_id( GTK_COMBO_BOX(combo_box) );
    std::string ssName = t ? t : "name";

    UT_DEBUGMSG(("OnSemanticStylesheetsSetLocations_cb() ssName:%s\n", ssName.c_str() ));
    UT_DEBUGMSG(("OnSemanticStylesheetsSetLocations_cb() combo:%p\n", combo_box ));
    UT_DEBUGMSG(("OnSemanticStylesheetsSetLocations_cb() t:%s\n", t ));

    ApplySemanticStylesheets( "Location", ssName );
    return false;
}

/******************************/
/******************************/
/******************************/

enum {
    COLUMN_REFDLG_NAME = 0,
    NUM_REFDLG_COLUMNS
};

static void
OnInsertReferenceBase( GtkWidget* dialog,
                   GtkTreeView* tree,
                   FV_View* pView )
{
    PD_Document* pDoc = pView->getDocument();
    PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();

    std::string n = getSelectedText( GTK_TREE_VIEW (tree), COLUMN_REFDLG_NAME );
    UT_DEBUGMSG(("clicked on: %s\n", n.c_str() ));

    bool found = false;
    PD_RDFContacts clist = rdf->getContacts();
    for( PD_RDFContacts::iterator ci = clist.begin(); ci != clist.end(); ++ci )
    {
        PD_RDFContactHandle obj = *ci;
        if( obj->name() == n )
        {
            obj->insert( pView );
            found = true;
            break;
        }
    }
    if( found )
        gtk_widget_destroy(dialog);

}

static void OnInsertReference( GtkDialog* d, gint /*response_id*/, gpointer user_data)
{
    UT_DEBUGMSG(("OnInsertReference()\n"));
    FV_View* pView = (FV_View*)user_data;

    GtkTreeView* tv = GTK_TREE_VIEW( g_object_get_data( G_OBJECT(d), G_OBJECT_TREEVIEW ));
    OnInsertReferenceBase( GTK_WIDGET(d), tv, pView );
}

static void
OnInsertReferenceDblClicked( GtkTreeView       * tree,
                             GtkTreePath       * /*path*/,
                             GtkTreeViewColumn * /*col*/,
                             gpointer		    user_data )
{
    FV_View* pView = (FV_View*)user_data;

    GtkWidget* d = GTK_WIDGET(g_object_get_data( G_OBJECT(tree), G_OBJECT_WINDOW ));
    OnInsertReferenceBase( d, tree, pView );
}


/******************************/
/******************************/
/******************************/

class PD_RDFDialogsGTK : public PD_RDFDialogs
{
public:
    PD_RDFDialogsGTK()
    {
        PD_DocumentRDF::setRDFDialogs( this );
    }
    virtual void runSemanticStylesheetsDialog( FV_View* pView )
    {
#if GTK_MAJOR_VERSION < 3
        return;
#endif
        GtkBuilder* builder   = newDialogBuilder("ap_UnixDialog_SemanticStylesheets.ui");
        GtkWidget*  window    = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
        GtkWidget*  contacts  = GTK_WIDGET(gtk_builder_get_object(builder, "contacts"));
        GtkWidget*  events    = GTK_WIDGET(gtk_builder_get_object(builder, "events"));
        GtkWidget*  locations = GTK_WIDGET(gtk_builder_get_object(builder, "locations"));
        GtkWidget*  setContacts  = GTK_WIDGET(gtk_builder_get_object(builder, "setContacts"));
        GtkWidget*  setEvents    = GTK_WIDGET(gtk_builder_get_object(builder, "setEvents"));
        GtkWidget*  setLocations = GTK_WIDGET(gtk_builder_get_object(builder, "setLocations"));
        GtkWidget*  setAll       = GTK_WIDGET(gtk_builder_get_object(builder, "setAll"));

        PD_Document* pDoc = pView->getDocument();
        PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();

        UT_DEBUGMSG(("runSemanticStylesheetsDialog_cb() combo:%p\n", contacts ));
        g_signal_connect (setContacts,  "button-release-event", G_CALLBACK (OnSemanticStylesheetsSetContacts_cb),  contacts );
        g_signal_connect (setEvents,    "button-release-event", G_CALLBACK (OnSemanticStylesheetsSetEvents_cb),    events );
        g_signal_connect (setLocations, "button-release-event", G_CALLBACK (OnSemanticStylesheetsSetLocations_cb), locations );

        g_signal_connect (setAll, "button-release-event", G_CALLBACK (OnSemanticStylesheetsSetContacts_cb),  contacts );
        g_signal_connect (setAll, "button-release-event", G_CALLBACK (OnSemanticStylesheetsSetEvents_cb),    events );
        g_signal_connect (setAll, "button-release-event", G_CALLBACK (OnSemanticStylesheetsSetLocations_cb), locations );
    
    
    
        g_signal_connect (G_OBJECT(window), "response",  G_CALLBACK(OnSemanticStylesheetsDialogResponse), pView );
        gtk_widget_show_all (window);
        
    }
    std::pair< PT_DocPosition, PT_DocPosition > runInsertReferenceDialog( FV_View* pView )
    {
        GtkBuilder* builder = newDialogBuilder("pd_RDFInsertReference.ui");
        GtkWidget*  window  = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
        GtkWidget*  tv      = GTK_WIDGET(gtk_builder_get_object(builder, "tv"));

        PD_Document* pDoc = pView->getDocument();
        PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();

        {
            GtkTreeStore *store = gtk_tree_store_new ( NUM_REFDLG_COLUMNS, G_TYPE_STRING );
            gtk_tree_view_set_model (GTK_TREE_VIEW (tv), GTK_TREE_MODEL (store));
            g_object_unref (G_OBJECT (store));
        }
    
        GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (tv));

        GtkTreeViewColumn *column = NULL;
        GtkCellRenderer *renderer = NULL;
        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tv),
                                                     -1, "Name", renderer,
                                                     "text", COLUMN_REFDLG_NAME,
                                                     NULL);
        column = gtk_tree_view_get_column (GTK_TREE_VIEW (tv), COLUMN_REFDLG_NAME );
        gtk_tree_view_column_set_sort_column_id (column, COLUMN_REFDLG_NAME );
    
        PD_RDFContacts l = rdf->getContacts();
        GtkTreeIter giter;
        GtkTreeIter parentiter;
        gtk_tree_store_append (GTK_TREE_STORE (model), &parentiter, 0);
        gtk_tree_store_set (GTK_TREE_STORE (model), &parentiter, 
                            COLUMN_REFDLG_NAME, "(Contacts)",
                            -1);
    
        for( PD_RDFContacts::iterator iter = l.begin(); iter != l.end(); ++iter )
        {
            PD_RDFContactHandle c = *iter;
            gtk_tree_store_append (GTK_TREE_STORE (model), &giter, &parentiter );
            gtk_tree_store_set (GTK_TREE_STORE (model), &giter, 
                                COLUMN_REFDLG_NAME, c->name().c_str(),
                                -1);

        }
        gtk_tree_view_expand_all(GTK_TREE_VIEW(tv));
        g_object_set_data( G_OBJECT(tv),     G_OBJECT_WINDOW,   window );
        g_object_set_data( G_OBJECT(window), G_OBJECT_TREEVIEW, tv );
    
        g_signal_connect (GTK_TREE_VIEW (tv), "row-activated", 
                          G_CALLBACK (OnInsertReferenceDblClicked), static_cast <gpointer>(pView));
        g_signal_connect (G_OBJECT(window), "response",  G_CALLBACK(OnInsertReference), pView );
        gtk_widget_show_all (window);

        std::pair< PT_DocPosition, PT_DocPosition > ret;
        return ret;
    }
    
};

namespace 
{
    PD_RDFDialogsGTK __obj;
};
