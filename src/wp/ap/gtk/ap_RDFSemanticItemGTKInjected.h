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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_RDFSEMANTICITEMGTKINJECTED_H
#define AP_RDFSEMANTICITEMGTKINJECTED_H

#include <gtk/gtk.h>

#include "pd_DocumentRDF.h"
#include "ut_std_string.h"
#include "xap_Dialog_Id.h"
#include "xap_App.h"
#include "xap_UnixDialogHelper.h"
#include "xap_Strings.h"

#define G_OBJECT_SEMITEM "G_OBJECT_SEMITEM"
#define G_OBJECT_SEMITEM_LIST "G_OBJECT_SEMITEM_LIST"
#define G_OBJECT_WINDOW  "G_OBJECT_WINDOW"
#define G_OBJECT_TREEVIEW "G_OBJECT_TREEVIEW"

class ap_GObjectSemItem
{
public:
    PD_RDFSemanticItemHandle h;
    ap_GObjectSemItem( PD_RDFSemanticItemHandle _h )
        : h(_h)
    {
    }
};

class ap_GObjectSemItem_List
{
public:
    PD_RDFSemanticItems cl;
    ap_GObjectSemItem_List( PD_RDFSemanticItems _cl )
        : cl(_cl)
    {
    }
};

void GDestroyNotify_GObjectSemItem(gpointer data);
PD_RDFSemanticItemHandle getHandle(GtkDialog* d);
void OnSemItemEdited ( GtkDialog* d, gint /*response_id*/,
					   gpointer /*user_data*/);

void GDestroyNotify_GObjectSemItem_List(gpointer data);
PD_RDFSemanticItems getSemItemListHandle(GtkDialog* d);
void OnSemItemListEdited ( GtkDialog* d, gint response_id,
						   gpointer /*user_data*/);




template < class ParentClass >
class ABI_EXPORT AP_RDFSemanticItemGTKInjected : public ParentClass
{
  public:
    typedef std::list< std::map< std::string, std::string > > PD_ResultBindings_t;
    AP_RDFSemanticItemGTKInjected( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator& it )
      : ParentClass( rdf, it )
    {
    }
    AP_RDFSemanticItemGTKInjected( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator& it, bool v )
      : ParentClass( rdf, it, v )
    {
    }

    void showEditorWindow( PD_RDFSemanticItemHandle c )
    {
        UT_DEBUGMSG(("showEditorWindow(INJECTED) name:%s linksubj:%s\n",
                     c->name().c_str(), c->linkingSubject().toString().c_str() ));
        XAP_StringSet* pSS = XAP_App::getApp()->getStringSet();
        std::string s;
        pSS->getValueUTF8(XAP_STRING_ID_DLG_OK, s);
        GtkWidget* d = gtk_dialog_new_with_buttons ("Message",
                                                    0,
                                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    convertMnemonics(s).c_str(),
                                                    GTK_RESPONSE_NONE,
                                                    NULL);
        GtkWidget* w = GTK_WIDGET(c->createEditor());
        g_object_set_data_full( G_OBJECT(w),
                                G_OBJECT_SEMITEM,
                                new ap_GObjectSemItem( c ),
                                GDestroyNotify_GObjectSemItem );
        /* g_object_set_data_full( G_OBJECT(d), */
        /*                         G_OBJECT_SEMITEM, */
        /*                         new struct G_OBJECT_SEMITEM( c ), */
        /*                         GDestroyNotify_G_OBJECT_SEMITEM ); */
        // reparent widget. Make sure to hold a temp reference on the widget.
        g_object_ref(w);
        GtkWidget* container = gtk_widget_get_parent(w);
        gtk_container_remove(GTK_CONTAINER(container), w);
        gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area( GTK_DIALOG (d))), w);
        g_object_unref(w);

        g_signal_connect (G_OBJECT(d), "response",  G_CALLBACK(OnSemItemEdited), c.get() );
        gtk_widget_show_all (d);
    }

    void showEditorWindow( PD_RDFSemanticItems cl )
    {
        UT_DEBUGMSG(("showEditorWindow() list... sz:%ld\n", cl.size() ));

        XAP_StringSet* pSS = XAP_App::getApp()->getStringSet();
        std::string s;
        pSS->getValueUTF8(XAP_STRING_ID_DLG_OK, s);
        GtkWidget* d = gtk_dialog_new_with_buttons ("Message",
                                                    0,
                                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    convertMnemonics(s).c_str(),
                                                    GTK_RESPONSE_NONE,
                                                    NULL);
        GtkNotebook* notebook = GTK_NOTEBOOK(gtk_notebook_new());
        gtk_container_add( GTK_CONTAINER(gtk_dialog_get_content_area( GTK_DIALOG (d))),
                           GTK_WIDGET(notebook) );
        for( PD_RDFSemanticItems::iterator ci = cl.begin(); ci != cl.end(); ++ci )
        {
            PD_RDFSemanticItemHandle c = *ci;
            GtkWidget* w = GTK_WIDGET(c->createEditor());
            g_object_set_data_full( G_OBJECT(w),
                                    G_OBJECT_SEMITEM,
                                    new ap_GObjectSemItem( c ),
                                    GDestroyNotify_GObjectSemItem );
            g_object_set_data_full( G_OBJECT(d),
                                    G_OBJECT_SEMITEM,
                                    new ap_GObjectSemItem( c ),
                                    GDestroyNotify_GObjectSemItem );

            std::string label = c->getDisplayLabel();
            GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            gtk_notebook_append_page( notebook, container, gtk_label_new( label.c_str() ));
            GtkWidget *oldContainer = gtk_widget_get_parent(w);
            g_object_ref(w);
            gtk_container_remove(GTK_CONTAINER(oldContainer), w);
            gtk_container_add(GTK_CONTAINER(container), w);
            g_object_unref(w);
        }
        g_object_set_data_full( G_OBJECT(d),
                                G_OBJECT_SEMITEM_LIST,
                                new ap_GObjectSemItem_List( cl ),
                                GDestroyNotify_GObjectSemItem_List );
        g_signal_connect (G_OBJECT(d), "response",  G_CALLBACK(OnSemItemListEdited), 0 );
        gtk_widget_show_all (d);
    }

    void importFromDataComplete( std::istream& /*iss*/,
                                PD_DocumentRDFHandle rdf,
                                PD_DocumentRDFMutationHandle m,
                                PD_DocumentRange * pDocRange = 0 )
    {
        // Create and populate and editor with the current data,
        // then update the Rdf from that editor.
        GtkWidget* objectEditor = (GtkWidget*)this->createEditor();
        this->updateFromEditorData( m );
        gtk_widget_destroy( GTK_WIDGET(objectEditor) );

        if (pDocRange)
        {
//        insert(host);
        }

        if (rdf)
        {
//        rdf->emitSemanticObjectAdded(this);
        }
    }

    std::string getImportFromFileName( const std::string& filename_const,
                                       std::list< std::pair< std::string, std::string> > types ) const
    {
        std::string ret = filename_const;

        UT_runDialog_AskForPathname afp( XAP_DIALOG_ID_FILE_IMPORT );
        if( !types.empty() )
        {
            std::list< std::pair< std::string, std::string> >::iterator iter = types.begin();
            afp.setDefaultFiletype( iter->first, iter->second );
        }
        for( std::list< std::pair< std::string, std::string> >::iterator iter = types.begin();
             iter != types.end(); ++iter )
        {
            afp.appendFiletype(    iter->first, iter->second );
        }

        if( afp.run( XAP_App::getApp()->getLastFocussedFrame() ) )
        {
            ret = afp.getPath();
            if( starts_with( ret, "file:" ))
                ret = ret.substr( strlen("file:") );
        }
        return ret;
    }

    std::string
        getExportToFileName( const std::string& filename_const,
                             std::string defaultExtension,
                             std::list< std::pair< std::string, std::string> > types ) const
    {
        std::string filename = filename_const;

        if( filename.empty() )
        {
            UT_runDialog_AskForPathname afp( XAP_DIALOG_ID_FILE_EXPORT );
            if( !types.empty() )
            {
                std::list< std::pair< std::string, std::string> >::iterator iter = types.begin();
                afp.setDefaultFiletype( iter->first, iter->second );
            }
            for( std::list< std::pair< std::string, std::string> >::iterator iter = types.begin();
                 iter != types.end(); ++iter )
            {
                afp.appendFiletype(    iter->first, iter->second );
            }

            if( afp.run( XAP_App::getApp()->getLastFocussedFrame() ) )
            {
                filename = afp.getPath();
                if( starts_with( filename, "file:" ))
                    filename = filename.substr( strlen("file:") );
                if( !ends_with( filename, defaultExtension ))
                    filename += defaultExtension;
            }
        }

        return filename;
    }


};
#endif
