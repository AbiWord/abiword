/*
 * AiksaurusGTK - A GTK interface to the Aiksaurus library
 * Copyright (C) 2001 by Jared Davis
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

#ifndef INCLUDED_AIKSAURUS_GTK_TOOLBAR_H
#define INCLUDED_AIKSAURUS_GTK_TOOLBAR_H

#include "AiksaurusGTK_history.h"
#include "AiksaurusGTK_histlist.h"
#include <gtk/gtk.h>
#include <new>

class AiksaurusGTK_picbutton;

namespace AiksaurusGTK_impl 
{

    class DialogMediator;
    class Toolbar
    {
        private:

            Toolbar(const Toolbar& rhs);
            Toolbar& operator=(const Toolbar& rhs);

            DialogMediator& d_mediator;

            AiksaurusGTK_history d_history;
            AiksaurusGTK_histlist d_searchbar_words;
            bool d_ishistorymove;
            bool d_searchhack;

            GtkWidget* d_window_ptr;
            GtkWidget* d_toolbar_ptr;
            GtkWidget* d_backbutton_box_ptr;
            AiksaurusGTK_picbutton* d_backbutton_ptr;
            AiksaurusGTK_picbutton* d_forwardbutton_ptr;
            AiksaurusGTK_picbutton* d_searchbutton_ptr;
            GtkWidget* d_searchbar_ptr;
            GtkWidget* d_searchbar_label_ptr;

            void _updateNavigation() throw(std::bad_alloc);

            void _setTooltip(GtkWidget* w, const char* str) throw();

            static void _backClicked(GtkWidget* w, gpointer data) throw();
            static void _backMenuClicked(GList* element, gpointer data) throw();
            
            static void _forwardClicked(GtkWidget* w, gpointer data) throw();
            static void _forwardMenuClicked(GList* element, gpointer data) throw(); 

            static void _searchBarChanged(GtkWidget* w, gpointer data) throw();
            static void _searchBarShow(GtkWidget* w, gpointer data) throw();
            static void _searchBarHide(GtkWidget* w, gpointer data) throw();
            static void _searchBarActivate(GtkWidget* w, gpointer d) throw(); 
            
            static void _searchClicked(GtkWidget* w, gpointer data) throw();

        public:

            Toolbar(DialogMediator& mediator, GtkWidget* window) throw(std::bad_alloc);
            ~Toolbar() throw();

            GtkWidget* getToolbar() throw();
            const char* getText() const throw();
            void focus() throw();

            void search(const char* str) throw(std::bad_alloc);
    };

}

#endif // INCLUDED_AIKSAURUS_GTK_TOOLBAR_H
