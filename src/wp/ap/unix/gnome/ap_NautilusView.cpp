/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

/* 
 * Copyright (C) 2000 Eazel, Inc
 * Copyright (C) 2002 Mertin Sevior <msevior@physics.unimelb.edu.au>
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors: Maciej Stachowiak <mjs@eazel.com>, Dom Lachowicz, and Martin Sevior.
 * Based on nautilus-sample-content-view.cpp
 */

/*
 This loads a wordprocessor file with the abiword widget.
 */

#define ABIWORD_INTERNAL

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#if 0
#include "ap_NautilusView.h"
#endif

#include <bonobo.h>
#include <string.h>

#if 0
#include <libnautilus/nautilus-bonobo-ui.h>
#include <bonobo/bonobo-control.h>
#include <libgnome/gnome-i18n.h>
#include <libgnomeui/gnome-stock.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtklabel.h>
#include <gtk/gtksignal.h>
#include "ap_UnixGnomeApp.h"
#include "abiwidget.h"

/* 
 * CHANGE: You probably want some different widget than a label to be
 * your main view contents.  
 */
struct NautilusAbiWordContentViewDetails 
{
	char *location;
	GtkWidget * abiword;
};

static void nautilus_abiword_content_view_initialize_class (NautilusAbiWordContentViewClass *klass);

static void nautilus_abiword_content_view_initialize       (NautilusAbiWordContentView      *view);

static void nautilus_abiword_content_view_destroy          (GtkObject                      *object);

static void abiword_load_location_callback                 (NautilusView                   *nautilus_view,
															const char                     *location,
															gpointer                        user_data);

static void abiword_merge_bonobo_items_callback            (BonoboControl                  *control,
															gboolean state,
															gpointer user_data);

static gpointer parent_class;
static FILE * logfile;                                                                                                        
GtkType
nautilus_abiword_content_view_get_type (void)
{
	logfile = fopen("/home/dom/abilogNaut","w");
	fprintf(logfile,"in create type \n");

	static GtkType type = 0 ;
	
	if (type == 0) 
		{	
			static GtkTypeInfo info = 
			{
				"NautilusAbiWordContentView",
				sizeof (NautilusAbiWordContentView),
				sizeof (NautilusAbiWordContentViewClass),
				(GtkClassInitFunc)nautilus_abiword_content_view_initialize_class,
				(GtkObjectInitFunc)nautilus_abiword_content_view_initialize,
				NULL,
				NULL,
				NULL
			};
			
			type = gtk_type_unique (NAUTILUS_TYPE_VIEW, &info);
			parent_class = gtk_type_class (NAUTILUS_TYPE_VIEW);
			
			fprintf(logfile,"created type '%d'\n",type);
		}
	return type;
}


static void
nautilus_abiword_content_view_initialize_class (NautilusAbiWordContentViewClass *klass)
{
	GtkObjectClass *object_class;
	g_assert (NAUTILUS_IS_ABIWORD_CONTENT_VIEW_CLASS (klass));
	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = nautilus_abiword_content_view_destroy;
}

static void
nautilus_abiword_content_view_initialize (NautilusAbiWordContentView *view)
{
	g_assert (NAUTILUS_IS_ABIWORD_CONTENT_VIEW (view));
	view->details = g_new0 (NautilusAbiWordContentViewDetails, 1);

	/*
	 * create a new AbiWidget instance
	 */
	
	AP_UnixApp * pApp = (AP_UnixApp *) XAP_App::getApp();
	UT_ASSERT(pApp);

	view->details->abiword = abi_widget_new_with_app (pApp);
	fprintf(logfile,"Using app %x created abiwidget %x\n",view->details->abiword);
	gtk_widget_show (view->details->abiword);
	
	nautilus_view_construct (NAUTILUS_VIEW (view), 
							 view->details->abiword);
	
	gtk_signal_connect (GTK_OBJECT (view), 
						"load_location",
						GTK_SIGNAL_FUNC(abiword_load_location_callback), 
						NULL);

	/* Get notified when our bonobo control is activated so we can
	 * merge menu & toolbar items into the shell's UI.
	 */
	gtk_signal_connect (GTK_OBJECT (nautilus_view_get_bonobo_control (NAUTILUS_VIEW (view))),
						"activate",
						GTK_SIGNAL_FUNC(abiword_merge_bonobo_items_callback),
						(gpointer) view);

	fprintf(logfile,"Signals connected \n");	
}

static void
nautilus_abiword_content_view_destroy (GtkObject *object)
{
	NautilusAbiWordContentView *view;
	
	view = NAUTILUS_ABIWORD_CONTENT_VIEW (object);
	
	g_free (view->details->location);
	g_free (view->details);
	(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);

	fclose ( logfile ) ;
}


/*!
 * Actually do the loading here. This is trivial using the load_file
 * argument of AbiWidget.
 */

static void
load_location (NautilusAbiWordContentView *view,
	       const char *location)
{
	fprintf(logfile,"attempting to load %s \n",location);

	g_assert (NAUTILUS_IS_ABIWORD_CONTENT_VIEW (view));
	g_assert (location != NULL);
	g_free (view->details->location);
	view->details->location = g_strdup (location);
	GtkObject * abi = GTK_OBJECT(view->details->abiword);
	gtk_object_set(abi,"AbiWidget::load_file",(gchar *) location,NULL);
}

/*!
 * Boiler plate wrapper to load a file into a view.
 */
static void
abiword_load_location_callback (NautilusView *nautilus_view, 
			       const char *location,
			       gpointer user_data)
{
	NautilusAbiWordContentView *view;
	
	g_assert (NAUTILUS_IS_VIEW (nautilus_view));
	g_assert (location != NULL);
	
	view = NAUTILUS_ABIWORD_CONTENT_VIEW (nautilus_view);
	
	/* It's mandatory to send an underway message once the
	 * component starts loading, otherwise nautilus will assume it
	 * failed. In a real component, this will probably happen in
	 * some sort of callback from whatever loading mechanism it is
	 * using to load the data; this component loads no data, so it
	 * gives the progress update here.
	 */
	nautilus_view_report_load_underway (nautilus_view);
	
	/* Do the actual load. */
	load_location (view, location);
	
	/* It's mandatory to call report_load_complete once the
	 * component is done loading successfully, or
	 * report_load_failed if it completes unsuccessfully. In a
	 * real component, this will probably happen in some sort of
	 * callback from whatever loading mechanism it is using to
	 * load the data; this component loads no data, so it gives
	 * the progress update here.
	 */
	nautilus_view_report_load_complete (nautilus_view);
}

/*!
 * Do the Menu/toolbar merging here. Right now there is just placeholders
 * Later on I'll put in some genuine commands to control the display of 
 * the view. The actual execution of the toolbar/menu items happens here.
 * user_data is actually the pointer to the view.
 */
static void
bonobo_abiword_callback (BonoboUIComponent *ui, 
						 gpointer           user_data, 
						 const char        *verb)
{
 	NautilusAbiWordContentView *view;
	char *label_text;

	g_assert (BONOBO_IS_UI_COMPONENT (ui));
	g_assert (verb != NULL);
		
	view = NAUTILUS_ABIWORD_CONTENT_VIEW (user_data);

	if (strcmp (verb, "AbiWord Menu Item") == 0) 
	{
	       label_text = g_strdup_printf ("%s \n","You selected the AbiWord menu item.");
	}
	else if( strcmp (verb, "AbiWord Dock Item") == 0)
	{
		label_text = g_strdup_printf ("%s \n","You clicked the AbiWord toolbar button.");
	}
	g_free (label_text);
}

/*!
 * Boilerplate wrapper for the Nautilus view. The file:
 * "/home/msevior/abidir/abi-cvs/abidistfiles/nautilus-abiword-content-view-ui.xml",
 * be replaced with a location somewhere in the distribution files of
 * abiword. Maybe /usr/share/AbiSuite/AbiWord...
 */
static void
abiword_merge_bonobo_items_callback (BonoboControl *control, 
				    gboolean       state, 
				    gpointer       user_data)
{
 	NautilusAbiWordContentView *view;
	BonoboUIComponent *ui_component;
	BonoboUIVerb verbs [] = {
		BONOBO_UI_VERB ("AbiWord Menu Item", bonobo_abiword_callback),
		BONOBO_UI_VERB ("AbiWord Dock Item", bonobo_abiword_callback),
		BONOBO_UI_VERB_END
	};

	g_assert (BONOBO_IS_CONTROL (control));
	
	view = NAUTILUS_ABIWORD_CONTENT_VIEW (user_data);

	fprintf(logfile,"attempting to merge UI data \n");

	if (state) 
		{
			ui_component = nautilus_view_set_up_ui (NAUTILUS_VIEW (view),
													NULL,
													/* "nautilus-abiword-content-view-ui.xml"*/ "",
													"nautilus-abiword-content-view");
			
			bonobo_ui_component_add_verb_list_with_data (ui_component, verbs, view);
			
			fprintf(logfile, "merged UI data\n");
		}

        /* Note that we do nothing if state is FALSE. Nautilus content
         * views are activated when installed, but never explicitly
         * deactivated. When the view changes to another, the content
         * view object is destroyed, which ends up calling
         * bonobo_ui_handler_unset_container, which removes its merged
         * menu & toolbar items.
		 */
}
#endif
