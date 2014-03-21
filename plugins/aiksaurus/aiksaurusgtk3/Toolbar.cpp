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

#include "Toolbar.h"
#include "AiksaurusGTK_picbutton.h"
#include "DialogMediator.h"
#include <gdk/gdkkeysyms.h>

namespace AiksaurusGTK_impl
{

    Toolbar::Toolbar(DialogMediator& mediator, GtkWidget* window) throw(std::bad_alloc)
        : d_mediator(mediator),
          d_searchbar_words(12),
          d_ishistorymove(false),
          d_searchhack(false),
          d_window_ptr(window)
    {
        d_toolbar_ptr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,	0);

        // Create back button and menu
        d_backbutton_box_ptr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,	0);
        d_backbutton_ptr = new AiksaurusGTK_picbutton(d_window_ptr, "go-previous");
        d_backbutton_ptr->addMenu(d_history.list_back(), G_CALLBACK(_backMenuClicked), this);
        d_backbutton_ptr->limitVisibleOptions(10);
        _setTooltip(d_backbutton_ptr->getButton(), "Back");

        // Create forward button and menu
        d_forwardbutton_ptr = new AiksaurusGTK_picbutton(d_window_ptr, "go-next");
        d_forwardbutton_ptr->addMenu(d_history.list_forward(),
                                     G_CALLBACK(_forwardMenuClicked), this);
        d_forwardbutton_ptr->limitVisibleOptions(10);
        _setTooltip(d_forwardbutton_ptr->getButton(), "Forward");


        // Create search dropdown bar.
        d_searchbar_label_ptr = gtk_label_new("  Look up:");
        d_searchbar_ptr = gtk_combo_box_text_new_with_entry();
        _setTooltip(GTK_WIDGET(gtk_bin_get_child(GTK_BIN(d_searchbar_ptr))), "Enter word to look up");

        // Create search button
        d_searchbutton_ptr = new AiksaurusGTK_picbutton(d_window_ptr, "go-jump");
        _setTooltip(d_searchbutton_ptr->getButton(), "Find Synonyms");


        // Add all widgets to the toolbar layout box.
        gtk_box_pack_start(GTK_BOX(d_backbutton_box_ptr), d_backbutton_ptr->getButton(), 0, 0, 0);
        gtk_box_pack_start(GTK_BOX(d_backbutton_box_ptr), d_backbutton_ptr->getMenuButton(), 0, 0, 0);
        gtk_box_pack_start(GTK_BOX(d_toolbar_ptr), d_backbutton_box_ptr, 0, 0, 4);
        gtk_box_pack_start(GTK_BOX(d_toolbar_ptr), d_forwardbutton_ptr->getButton(), 0, 0, 0);
        gtk_box_pack_start(GTK_BOX(d_toolbar_ptr), d_forwardbutton_ptr->getMenuButton(), 0, 0, 0);
        gtk_box_pack_start(GTK_BOX(d_toolbar_ptr), d_searchbar_label_ptr, 0, 0, 5);
        gtk_box_pack_start(GTK_BOX(d_toolbar_ptr), d_searchbar_ptr, 1, 1, 5);
        gtk_box_pack_start(GTK_BOX(d_toolbar_ptr), d_searchbutton_ptr->getButton(), 0, 0, 4);

        // Connect all relevant signals.
        g_signal_connect(G_OBJECT(d_backbutton_ptr->getButton()), "clicked",
                           G_CALLBACK(_backClicked), this);
        g_signal_connect(G_OBJECT(d_forwardbutton_ptr->getButton()), "clicked",
                           G_CALLBACK(_forwardClicked), this);
        g_signal_connect(G_OBJECT(d_searchbutton_ptr->getButton()), "clicked",
                           G_CALLBACK(_searchClicked), this);
        g_signal_connect(G_OBJECT(gtk_bin_get_child(GTK_BIN(d_searchbar_ptr))), "activate",
                           G_CALLBACK(_searchBarActivate), this);
        g_signal_connect(G_OBJECT(GTK_COMBO_BOX(d_searchbar_ptr)), "popdown",
                           G_CALLBACK(_searchBarHide), this);
        g_signal_connect(G_OBJECT(gtk_bin_get_child(GTK_BIN(d_searchbar_ptr))), "changed",
                           G_CALLBACK(_searchBarChanged), this);

        _updateNavigation();
    }

    Toolbar::~Toolbar() throw()
    {

    }

    void Toolbar::_updateNavigation() throw(std::bad_alloc)
    {
        if (d_history.size_back())
            d_backbutton_ptr->enable();
        else
            d_backbutton_ptr->disable();

        if (d_history.size_forward())
            d_forwardbutton_ptr->enable();
        else
            d_forwardbutton_ptr->disable();

        _setTooltip(d_backbutton_ptr->getButton(), d_history.tip_back());
        _setTooltip(d_forwardbutton_ptr->getButton(), d_history.tip_forward());

        d_backbutton_ptr->updateMenuOptions();
        d_forwardbutton_ptr->updateMenuOptions();
    }

    void Toolbar::search(const char* str) throw(std::bad_alloc)
    {
        if (!d_ishistorymove)
            d_history.search(str);
		GtkComboBoxText *combo = GTK_COMBO_BOX_TEXT(d_searchbar_ptr);

        _updateNavigation();

        d_searchbar_words.addItem(str);
		gtk_combo_box_text_remove_all(combo);
		const GList *ptr = d_searchbar_words.list();
		for (; ptr; ptr = ptr->next)
			gtk_combo_box_text_append_text(combo, reinterpret_cast<const char*>(ptr->data));
    }

    void Toolbar::_setTooltip(GtkWidget* w, const char* str) throw()
    {
        gtk_widget_set_tooltip_text(w,	str);
    }

    void Toolbar::focus() throw()
    {
        gtk_window_set_focus(GTK_WINDOW(d_window_ptr), gtk_bin_get_child(GTK_BIN(d_searchbar_ptr)));
    }

    const char* Toolbar::getText() const throw()
    {
        return gtk_entry_get_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(d_searchbar_ptr))));
    }

    GtkWidget* Toolbar::getToolbar() throw()
    {
        return d_toolbar_ptr;
    }



    void Toolbar::_backClicked(GtkWidget*, gpointer data) throw()
    {
        Toolbar* tb = static_cast<Toolbar*>(data);
        tb->d_history.move_back();
        tb->d_ishistorymove = true;
        tb->d_mediator.eventSearch( tb->d_history.current() );
        tb->d_ishistorymove = false;
    }

    void Toolbar::_backMenuClicked(GList* element, gpointer data) throw()
    {
        Toolbar* tb = static_cast<Toolbar*>(data);
        tb->d_history.move_back_to(element);
        tb->d_ishistorymove = true;
        tb->d_mediator.eventSearch( tb->d_history.current() );
        tb->d_ishistorymove = false;
    }

    void Toolbar::_forwardClicked(GtkWidget*, gpointer data) throw()
    {
        Toolbar* tb = static_cast<Toolbar*>(data);
        tb->d_history.move_forward();
        tb->d_ishistorymove = true;
        tb->d_mediator.eventSearch( tb->d_history.current() );
        tb->d_ishistorymove = false;
    }

    void Toolbar::_forwardMenuClicked(GList* element, gpointer data) throw()
    {
        Toolbar* tb = static_cast<Toolbar*>(data);
        tb->d_history.move_forward_to(element);
        tb->d_ishistorymove = true;
        tb->d_mediator.eventSearch( tb->d_history.current() );
        tb->d_ishistorymove = false;
    }

    void Toolbar::_searchBarChanged(GtkWidget*, gpointer data) throw()
    {
        Toolbar* tb = static_cast<Toolbar*>(data);
		bool popup_visible;
		g_object_get (tb->d_searchbar_ptr, "popup-shown", &popup_visible, NULL);

        if (popup_visible)
            tb->d_searchhack = true;
    }

    void Toolbar::_searchBarHide(GtkWidget*, gpointer data) throw()
    {
        Toolbar* tb = static_cast<Toolbar*>(data);

        if (tb->d_searchhack)
            tb->d_mediator.eventSearch( tb->getText() );

        tb->d_searchhack = false;
    }

    void Toolbar::_searchBarActivate(GtkWidget* w, gpointer data) throw()
    {
      _searchClicked(w, data);
    }

    void Toolbar::_searchClicked(GtkWidget*, gpointer data) throw()
    {
        Toolbar* tb = static_cast<Toolbar*>(data);
        tb->d_mediator.eventSearch( tb->getText() );
    }

}
