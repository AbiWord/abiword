/*
 * AiksaurusGTK - A GTK interface to the AikSaurus library
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

#ifndef INCLUDED_GPL_JARED_AIKSAURUSGTK_PICBUTTON_H
#define INCLUDED_GPL_JARED_AIKSAURUSGTK_PICBUTTON_H

#include <glib.h>
#include <gtk/gtk.h>

#include "AiksaurusGTK_strlist.h"

class AiksaurusGTK_menudata;
class AiksaurusGTK_picbutton
{
	private:

	// Possible button states

		bool d_hashover;
		bool d_hasmenu;
		bool d_mouseover;
		bool d_menushowing;
		bool d_enabled;


	// Member Widgets

		GtkWidget* d_window_ptr;
		GtkWidget* d_button_ptr;
		GtkWidget* d_pixmap_ptr;

        int d_numVisible;

		GtkWidget* d_menu_ptr;
		GtkWidget* d_menu_button_ptr;
		GdkPixbuf* d_menu_mask_ptr;
		GtkWidget* d_menu_pixmap_widget_ptr;

        GCallback d_onclick_function;
        gpointer d_onclick_data;

        AiksaurusGTK_menudata* d_menu_data_ptr;
        AiksaurusGTK_strlist* d_menu_options_ptr;

	// Callback Functions

		static void cbHover(GtkWidget* w, gpointer data);
		static void cbUnhover(GtkWidget* w, gpointer data);
		static void cbPopMenu(GtkWidget* w, gpointer data);
		static void cbPopupFunction(GtkMenu* menu, int* x, int* y, gboolean* notUsedBool, gpointer data);
		static void cbSelectionDone(GtkMenuShell* menushell, gpointer data);
        static void cbMenuActivate(GtkMenuItem* item, gpointer data);

	// Mouse Hovering Actions

		void handleRelief();
		void hover();
		void unhover();


	// Menu Activation and Deactivation

		void popMenu();
		void popupFunction(int* x, int* y);
		void selectionDone();
        void menuActivate(gpointer item);

        void menuCreate();


	// Prevent Copying and Assignment

		AiksaurusGTK_picbutton(const AiksaurusGTK_picbutton&);
		const AiksaurusGTK_picbutton& operator=(const AiksaurusGTK_picbutton&);



	public:

	// Construction and Destruction

		AiksaurusGTK_picbutton(GtkWidget *window, const char** normal);
		AiksaurusGTK_picbutton(GtkWidget *window, const char* stock);
		~AiksaurusGTK_picbutton();


	// Manipulation

		void setHoverPicture(const char** hover);
		void addMenu(const AiksaurusGTK_strlist& options, GCallback onClick, gpointer onClickData);
        void updateMenuOptions();
        void limitVisibleOptions(int numVisible);

		void disable();
		void enable();

        GtkWidget* getButton();
		GtkWidget* getMenuButton();
};

#endif // INCLUDED_GPL_JARED_AIKSAURUSGTK_PICBUTTON_H
