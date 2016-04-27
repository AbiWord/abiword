/* some common code to make GTK+ less of a PITA.
 *
 * Copyright (C) 2011 AbiSource, Inc.
 * Copyright (C) 2011 Ben Martin
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

#include "GTKCommon.h"

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_std_string.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <sstream>

std::string
tostr( long v )
{
    std::stringstream ss;
    ss << v;
    return ss.str();
}

std::string
tostr( GtkTextView* tv )
{
    GtkTextBuffer* b = gtk_text_view_get_buffer( tv );
    GtkTextIter begin;
    GtkTextIter end;
    
    gtk_text_buffer_get_start_iter( b, &begin );
    gtk_text_buffer_get_end_iter( b, &end );
    gboolean include_hidden_chars = false;
    
    gchar* d = gtk_text_buffer_get_text( b, &begin, &end, include_hidden_chars );
    std::string ret = d;
    g_free( d );
    return ret;
}
std::string
tostr( GtkEntry* e )
{
    if(!e)
        return "";
    std::string ret;
    ret = gtk_entry_get_text (GTK_ENTRY (e));
    return ret;
}

std::string
getSelectedText( GtkTreeView* tv, int colnum )
{
	std::string ret;

	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (tv));
	UT_return_val_if_fail (model != NULL, ret);

	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv));
	GtkTreeIter iter;
	gboolean haveSelected = gtk_tree_selection_get_selected (selection, &model, &iter);
	if (!haveSelected)
		return ret;

	gchar *label = NULL;
	gtk_tree_model_get (model, &iter, colnum, &label, -1);
    ret = label;
    g_free(label);
	return ret;
}

UT_uint32
getSelectedUInt( GtkTreeView* tv, int colnum )
{
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (tv));
	UT_return_val_if_fail (model != NULL, 0);

	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv));
	GtkTreeIter iter;
	gboolean haveSelected = gtk_tree_selection_get_selected (selection, &model, &iter);
	if (!haveSelected)
		return 0;

    UT_uint32 v = 0;
	gtk_tree_model_get (model, &iter, colnum, &v, -1);
	return v;
}

void selectNext( GtkTreeView* tv )
{
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (tv));
	UT_return_if_fail (model != NULL);

	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv));
	GtkTreeIter iter;

	// try to select next
	gboolean haveSelected = gtk_tree_selection_get_selected (selection, &model, &iter);
	if (haveSelected) {
		GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
		gtk_tree_path_next (path);
		gboolean haveNext = gtk_tree_model_get_iter (model, &iter, path);
		if (haveNext) {
			gtk_tree_selection_select_path (selection, path);
			gtk_tree_path_free (path);
			return;
		}
		gtk_tree_path_free (path);
	}

	// select first
	GtkTreePath *path = gtk_tree_path_new_first ();
	gtk_tree_selection_select_path (selection, path);
	gtk_tree_path_free (path);
}

GtkTreeIter getIterLast( GtkTreeView* tv )
{
    GtkTreeModel* model = gtk_tree_view_get_model( tv );
    GtkTreeIter ret;
    int valid = gtk_tree_model_get_iter_first( model, &ret );
    UT_DEBUGMSG((" getIterLast() start...\n"));
    for( GtkTreeIter iter = ret; valid; )
    {
        UT_DEBUGMSG((" getIterLast() loop...\n"));
        valid = gtk_tree_model_iter_next ( model, &iter );
        if( valid )
            ret = iter;
    }
    return ret;
}

void selectPrev( GtkTreeView* tv )
{
    UT_DEBUGMSG(("selectPrev() tv:%p\n", tv ));
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (tv));
	UT_return_if_fail (model != NULL);

	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv));
	GtkTreeIter iter;

	// try to select prev
	gboolean haveSelected = gtk_tree_selection_get_selected (selection, &model, &iter);
	if (haveSelected)
    {
		GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
		gboolean havePrev = gtk_tree_path_prev (path);
		havePrev &= gtk_tree_model_get_iter (model, &iter, path);
        UT_DEBUGMSG(("selectPrev() tv:%p have-prev:%d\n", tv, havePrev ));
		if (havePrev) {
			gtk_tree_selection_select_path (selection, path);
			gtk_tree_path_free (path);
			return;
		}
		gtk_tree_path_free (path);
	}

	// select last
    UT_DEBUGMSG(("selectPrev(L) tv:%p\n", tv ));
    GtkTreeIter last = getIterLast( tv );
	gtk_tree_selection_select_iter (selection, &last);
}

void append( GtkComboBoxText* combo, const std::list< std::string >& data )
{
    std::list<std::string>::const_iterator iter(data.begin());
    for( ; iter != data.end(); ++iter)
    {
        gtk_combo_box_text_append_text( combo, iter->c_str() );
    }
}

std::string tostr( GtkComboBox* combo )
{
    GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo)));
	UT_ASSERT(entry);
	const gchar *s = gtk_entry_get_text(entry);
	if(s && *s)
	{
        return s;
    }
    return "";
}

void setEntry( GtkWidget* w, const std::string& v )
{
    if( v.empty() )
        gtk_entry_set_text(GTK_ENTRY(w), "" );
    else 
        gtk_entry_set_text(GTK_ENTRY(w), v.c_str());
}

void setEntry( GtkEntry* w, const std::string& v )
{
    if( v.empty() )
        gtk_entry_set_text(GTK_ENTRY(w), "" );
    else 
        gtk_entry_set_text(GTK_ENTRY(w), v.c_str());
}
void setEntry( GtkEntry* w, time_t v )
{
    UT_DEBUGMSG(("setEntry(time) v:%ld str:%s\n", v, toTimeString(v).c_str()));
    gtk_entry_set_text(GTK_ENTRY(w), toTimeString(v).c_str());
}
void setEntry( GtkEntry* w, double v )
{
    UT_DEBUGMSG(("setEntry(double) v:%f str:%s\n", v, tostr(v).c_str()));
    gtk_entry_set_text(GTK_ENTRY(w), tostr(v).c_str());
}



static
void collect_cb_fe( GtkTreeModel * /*model*/, GtkTreePath * /*path*/, GtkTreeIter * iter, gpointer udata)
{
    list_gtktreeiter_t* x = (list_gtktreeiter_t*)udata;
    x->push_back( *iter );
}

static
gboolean collectall_cb_fe( GtkTreeModel * /*model*/, GtkTreePath * /*path*/, GtkTreeIter *iter, gpointer udata)
{
    list_gtktreeiter_t* x = (list_gtktreeiter_t*)udata;
    x->push_back( *iter );
    return 0;
}
    
list_gtktreeiter_t getIterList( GtkWidget* w_treeview, bool useSelection )
{
    GtkTreeModel* w_treemodel = gtk_tree_view_get_model( GTK_TREE_VIEW(w_treeview) );
    list_gtktreeiter_t giters;
    GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
    GtkTreeSelection *selection;

    if( useSelection )
    {
        selection = gtk_tree_view_get_selection ( tv );
        gtk_tree_selection_selected_foreach(
            selection, GtkTreeSelectionForeachFunc(collect_cb_fe), &giters );
    }
    else
    {
        gtk_tree_model_foreach( w_treemodel,
                                GtkTreeModelForeachFunc(collectall_cb_fe),
                                &giters );
    }
    return giters;
}

void clearSelection( GtkTreeView* tv )
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection ( tv );
    gtk_tree_selection_unselect_all( selection );
}

void selectIter( GtkTreeView* tv, GtkTreeIter* iter )
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv));
    gtk_tree_selection_select_iter( selection, iter );
}

void scrollToIter( GtkTreeView* tv, GtkTreeIter* iter, int colnum, gboolean start_editing )
{
    GtkTreeModel*      model = gtk_tree_view_get_model( tv );
    GtkTreePath*       path = 0;
    GtkTreeViewColumn* focus_column = 0;
    if( colnum >= 0 )
    {
        focus_column = gtk_tree_view_get_column( tv, colnum );
    }
    path = gtk_tree_model_get_path( model, iter );
    gtk_tree_view_set_cursor( tv, path, focus_column, start_editing );
    gtk_tree_path_free( path );
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

#include "ap_Frame.h"
#include "xap_App.h"
#include "xap_DialogFactory.h"
#include "xad_Document.h"
#include "pd_Document.h"
#include "ut_path.h"
#include "xap_Dialog_Id.h"

#include <sstream>

UT_runDialog_AskForPathname::UT_runDialog_AskForPathname( XAP_Dialog_Id id,
                                                          const std::string& suggestedName )
    : m_pathname( "" )
    , m_ieft( IEFT_Bogus )
    , m_dialogId( id )
    , m_saveAs( false )
    , m_suggestedName( suggestedName )
    , m_defaultFiletype( -1 )
{
    switch( id )
    {
        case XAP_DIALOG_ID_FILE_SAVEAS:
        case XAP_DIALOG_ID_FILE_EXPORT:
            m_saveAs = true;
            break;
    }
}

int
UT_runDialog_AskForPathname::appendFiletype( const std::string desc,
                                             const std::string ext,
                                             UT_sint32 n )
{
    if( !n )
        n = m_filetypes.size();
    
    m_filetypes.push_back( Filetype( desc, ext, n ) );
    return n;
}

void
UT_runDialog_AskForPathname::setDefaultFiletype( const std::string desc,
                                                 const std::string ext )
{
    for( FiletypeList_t::const_iterator iter = m_filetypes.begin();
         iter != m_filetypes.end(); ++iter )
    {
        if( !desc.empty() && iter->m_desc == desc )
        {
            m_defaultFiletype = iter->m_number;
            return;
        }
        if( !ext.empty() && iter->m_ext == ext )
        {
            m_defaultFiletype = iter->m_number;
            return;
        }
    }
}


const std::string &
UT_runDialog_AskForPathname::getPath() const
{
    return m_pathname;
}

IEFileType
UT_runDialog_AskForPathname::getType()
{
    return m_ieft;
}

struct FileTypeArray
{
    const char **    szDescList;
    const char **    szSuffixList;
    UT_sint32* nTypeList;
    
public:
    FileTypeArray( int len )
        : szDescList(0)
        , szSuffixList(0)
        , nTypeList(0)
    {
        szDescList   = static_cast<const char **>(UT_calloc(len + 1, sizeof(char *)));
        szSuffixList = static_cast<const char **>(UT_calloc(len + 1, sizeof(char *)));
        nTypeList    = static_cast<IEFileType *>(UT_calloc(len + 1, sizeof(IEFileType)));
        if(!szDescList || !szSuffixList || !nTypeList)
        {
            throw;
        }
    }
    
    
    ~FileTypeArray()
    {
		FREEP(nTypeList);
		FREEP(szDescList);
		FREEP(szSuffixList);
    }
    
    void setup( const UT_runDialog_AskForPathname::FiletypeList_t& filetypes )
    {
        int i = 0;
        for( UT_runDialog_AskForPathname::FiletypeList_t::const_iterator iter = filetypes.begin();
             iter != filetypes.end(); ++iter, ++i )
        {
            szDescList[i]   = iter->m_desc.c_str();
            szSuffixList[i] = iter->m_ext.c_str();
            nTypeList[i] = iter->m_number;
        }
    }
    

};



std::string
UT_runDialog_AskForPathname::appendDefaultSuffixFunctor( std::string dialogFilename, UT_sint32 /*n*/ )
{
    std::stringstream ss;
    ss << dialogFilename << ".zzz";
    return ss.str();
}


bool
UT_runDialog_AskForPathname::run( XAP_Frame * pFrame )
{
    // This is a bit like s_AskForPathname().
    // 
    // raise the file-open or file-save-as dialog.
    // return a_OK or a_CANCEL depending on which button
    // the user hits.

    UT_DEBUGMSG(("runDialog_AskForPathname: frame %p, bSaveAs %d, suggest=[%s]\n",
                 pFrame,m_saveAs,m_suggestedName.c_str() ));

    // if (pFrame) {
    //     pFrame->raise();
    // }

    XAP_DialogFactory * pDialogFactory
        = static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

    XAP_Dialog_FileOpenSaveAs * pDialog
        = static_cast<XAP_Dialog_FileOpenSaveAs *>(pDialogFactory->requestDialog(m_dialogId));
    UT_return_val_if_fail (pDialog, false);

    pDialog->setAppendDefaultSuffixFunctor(
        boost::bind(
            &UT_runDialog_AskForPathname::appendDefaultSuffixFunctor,
            this, _1, _2 ));
    
    if (!m_suggestedName.empty())
    {
        // if caller wants to suggest a name, use it and seed the
        // dialog in that directory and set the filename.
        pDialog->setCurrentPathname(m_suggestedName);
        pDialog->setSuggestFilename(true);
    }
    else if (pFrame)
    {
        // if caller does not want to suggest a name, seed the dialog
        // to the directory containing this document (if it has a
        // name), but don't put anything in the filename portion.
        PD_Document * pDoc = static_cast<PD_Document*>(pFrame->getCurrentDoc());
	std::string title;

        if (pDoc->getMetaDataProp (PD_META_KEY_TITLE, title) && !title.empty())
        {
            UT_legalizeFileName(title);
            pDialog->setCurrentPathname(title);
            pDialog->setSuggestFilename(true);
        } else {
            pDialog->setCurrentPathname(pFrame->getFilename());
            pDialog->setSuggestFilename(false);
        }
    }
    else
    {
        // we don't have a frame. This is likely that we are going to open
        // so don't need to suggest a name.
        pDialog->setSuggestFilename(false);
    }

    // FIXME: file types
    FileTypeArray fta( m_filetypes.size() );
    fta.setup( m_filetypes );
    pDialog->setFileTypeList( fta.szDescList,
                              fta.szSuffixList,
                              fta.nTypeList );


    UT_DEBUGMSG(("m_defaultFiletype:%d\n", m_defaultFiletype));
    // set the default filetype
    if( -1 != m_defaultFiletype )
    {
        pDialog->setDefaultFileType( m_defaultFiletype );
    }
    
    UT_DEBUGMSG(("About to runModal on FileOpen \n"));
    pDialog->runModal(pFrame);

    XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
    bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

    
    if (bOK)
    {
        const std::string & resultPathname = pDialog->getPathname();
        if (!resultPathname.empty()) {
            m_pathname = resultPathname;
        }

        UT_sint32 type = pDialog->getFileType();

        // If the number is negative, it's a special type.
        // Some operating systems which depend solely on filename
        // suffixes to identify type (like Windows) will always
        // want auto-detection.
        if (type < 0)
            switch (type)
            {
                case XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO:
                    // do some automagical detecting
                    m_ieft = IEFT_Unknown;
                    break;
                default:
                    // it returned a type we don't know how to handle
                    UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
            }
        else
            m_ieft = static_cast<IEFileType>(pDialog->getFileType());
    }
    pDialog->setAppendDefaultSuffixFunctor( getAppendDefaultSuffixFunctorUsing_IE_Exp_preferredSuffixForFileType() );
    pDialogFactory->releaseDialog(pDialog);
    return bOK;
}
