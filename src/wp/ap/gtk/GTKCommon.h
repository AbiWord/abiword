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
#ifndef ABI_GTK_COMMON_H
#define ABI_GTK_COMMON_H

#include <string>
#include <list>
#include <gtk/gtk.h>
#include "ut_types.h"

std::string tostr( long v );
std::string tostr( GtkTextView* tv );
std::string tostr( GtkEntry* e );
std::string getSelectedText( GtkTreeView* tv, int colnum = 0 );
UT_uint32   getSelectedUInt( GtkTreeView* tv, int colnum = 0 );
void selectNext( GtkTreeView* tv );
void selectPrev( GtkTreeView* tv );
void append( GtkComboBoxText* combo, const std::list< std::string >& data );
std::string tostr( GtkComboBox* combo );
void setEntry( GtkWidget* w, const std::string& v );
void setEntry( GtkEntry* w, const std::string& v );
void setEntry( GtkEntry* w, time_t v );
void setEntry( GtkEntry* w, double v );

typedef std::list< GtkTreeIter > list_gtktreeiter_t;
/**
 * Get an STL list of all the selected GtkTreeIter* items in the view
 * if useSelection is false then all of the GtkTreeIter* in the view are returned.
 */
list_gtktreeiter_t getIterList( GtkWidget* w_treeview, bool useSelection = false );

void clearSelection( GtkTreeView* tv );
void selectIter( GtkTreeView* tv, GtkTreeIter* iter );
void scrollToIter( GtkTreeView* tv, GtkTreeIter* iter, int colnum = -1, gboolean start_editing = false );



#include "xap_Dlg_FileOpenSaveAs.h"
#include "ie_types.h"

class UT_runDialog_AskForPathname
{
public:

    struct Filetype
    {
        std::string m_desc;
        std::string m_ext;
        UT_sint32   m_number;
        Filetype( std::string desc, std::string ext, UT_sint32 number )
        : m_desc(desc), m_ext(ext), m_number(number)
        {
        }
    };
    typedef std::list< Filetype > FiletypeList_t;


    UT_runDialog_AskForPathname( XAP_Dialog_Id id,
                                 const std::string& suggestedName = "" );

    int appendFiletype( const std::string desc,
                        const std::string ext,
                        UT_sint32 n = 0 );
    void setDefaultFiletype( const std::string desc,
                             const std::string ext = "" );
    bool run( XAP_Frame * pFrame = 0 );

    const std::string & getPath() const;
    IEFileType getType();



  private:
    std::string m_pathname;
    IEFileType m_ieft;
    XAP_Dialog_Id m_dialogId;
    bool m_saveAs;
    std::string m_suggestedName;
    FiletypeList_t m_filetypes;
    UT_sint32      m_defaultFiletype;

    std::string appendDefaultSuffixFunctor( std::string dialogFilename, UT_sint32 n );

};


#endif
