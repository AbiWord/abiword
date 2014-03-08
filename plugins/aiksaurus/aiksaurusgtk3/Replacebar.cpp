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

#include "Replacebar.h"
#include "DialogMediator.h"
#include <gdk/gdkkeysyms.h>

namespace AiksaurusGTK_impl 
{

    Replacebar::Replacebar(DialogMediator& mediator) throw()
        : d_mediator(mediator)
    {
        // Build GUI widgets.
	    d_replacebar_ptr = gtk_hbox_new(false, 4);
    	d_replacewith_label_ptr = gtk_label_new("  Replace with:");
	    d_replacewith_ptr = gtk_entry_new();
    	d_replacebutton_hold_ptr = gtk_hbox_new(true, 4);
	    d_replacebutton_ptr = gtk_button_new_with_label("  Replace  ");
        d_cancelbutton_ptr = gtk_button_new_with_label("Cancel");
        
        // Add them to the main hbox.
        gtk_box_pack_start(GTK_BOX(d_replacebar_ptr), d_replacewith_label_ptr, 0, 0, 0);
        gtk_box_pack_start(GTK_BOX(d_replacebar_ptr), d_replacewith_ptr, 0, 0, 2);
	    gtk_box_pack_end(GTK_BOX(d_replacebar_ptr),	d_replacebutton_hold_ptr, 0, 0, 5);
    	gtk_box_pack_start(GTK_BOX(d_replacebutton_hold_ptr), d_replacebutton_ptr, 0, 0, 0);
        gtk_box_pack_start(GTK_BOX(d_replacebutton_hold_ptr), d_cancelbutton_ptr, 1, 1, 0);
   
        // Connect relevant signals. 
        g_signal_connect(
            G_OBJECT(d_replacewith_ptr), "key-press-event",
	    	G_CALLBACK(_keyPressed), this	
    	);
	    g_signal_connect(
    		G_OBJECT(d_replacebutton_ptr), "clicked",
    		G_CALLBACK(_replaceClicked), this	
    	);
	    g_signal_connect(
            G_OBJECT(d_cancelbutton_ptr), "clicked", 
            G_CALLBACK(_cancelClicked), this	
    	);
    }

    
    Replacebar::~Replacebar() throw()
    {

    }

    void Replacebar::_cancelClicked(GtkWidget* w, gpointer data)
    {
        Replacebar* rb = static_cast<Replacebar*>(data);
        rb->d_mediator.eventCancel();
    }

    
    void Replacebar::_replaceClicked(GtkWidget* w, gpointer data)
    {   
        Replacebar* rb = static_cast<Replacebar*>(data);
        rb->d_mediator.eventReplace( rb->getText() );
    }

    
    void Replacebar::_keyPressed(GtkWidget* w, GdkEventKey* k, gpointer data)
    {
        if (k->keyval == GDK_KEY_Return)
            _replaceClicked(w, data);
    }

    
    GtkWidget* Replacebar::getReplacebar() throw()
    {
        return d_replacebar_ptr;
    }
    
    
    const char* Replacebar::getText() const throw()
    {
        return gtk_entry_get_text(GTK_ENTRY(d_replacewith_ptr));
    }

    
    void Replacebar::setText(const char* word) throw() 
    {
        gtk_entry_set_text(GTK_ENTRY(d_replacewith_ptr), word);
    }

}
