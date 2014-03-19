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

#ifndef INCLUDED_AIKSAURUS_GTK_REPLACEBAR_H
#define INCLUDED_AIKSAURUS_GTK_REPLACEBAR_H

#include <gtk/gtk.h>

namespace AiksaurusGTK_impl
{

    class DialogMediator;
    class Replacebar
    {
        private:
            DialogMediator& d_mediator;
		    GtkWidget *d_replacebar_ptr;           // layout hbox
            GtkWidget *d_replacewith_label_ptr;    // says "Replace With"
	    	GtkWidget *d_replacebutton_hold_ptr;   // for some padding
		    GtkWidget *d_replacebutton_ptr;        // replace button
    		GtkWidget *d_cancelbutton_ptr;         // cancel button
	    	GtkWidget *d_replacewith_ptr;          // text entry

            // GUI Callbacks.  These will invoke the mediator as needed.
            // Note that these functions might throw just about anything.
    		static void _cancelClicked(GtkWidget* w, gpointer data);
	    	static void _replaceClicked(GtkWidget* w, gpointer data);
		    static void _keyPressed(GtkWidget* w, GdkEventKey* k, gpointer data);

        public:

            Replacebar(DialogMediator& mediator) throw();

            // Warning: You need to call gtk_widget_destroy on getReplacebar(), or
            // destroy whatever widget you put it in.  The destructor will NOT free
            // up the memory used by the GUI widgets.
            ~Replacebar() throw();

            // getReplacebar(): return layout widget for replace bar.
            GtkWidget* getReplacebar() throw();

            // getText(): return current text of replace text field.
            const char* getText() const throw();

            // setText(): reset text of replace text field to something new.
            void setText(const char* str) throw();
    };

}

#endif // INCLUDED_AIKSAURUS_GTK_REPLACEBAR_H
