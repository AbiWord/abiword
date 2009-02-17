/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* The AbiWord Document view Widget 
 *
 * Copyright (C) 2007 Michael Gorse
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

#include <string.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ap_DocView.h"
//#include "at_DocView.h"
#include "ut_debugmsg.h"
#include <atk/atk.h>

// our parent class
static GtkDrawingAreaClass * parent_class = 0;

/**************************************************************************/
/**************************************************************************/

#define GET_CLASS(instance) G_TYPE_INSTANCE_GET_CLASS (instance, AP_DOCVIEW_TYPE, ApDocViewClass)

static void
ap_DocView_class_init (ApDocViewClass *dv_class)
{

#ifdef LOGFILE
	fprintf(getlogfile(),"ap_DocView class init \n");
#endif

	GObjectClass * gobject_class;
	GtkObjectClass * object_class;
	GtkWidgetClass * widget_class;

	gobject_class = (GObjectClass *)dv_class;
	object_class = (GtkObjectClass *)dv_class;
	widget_class = (GtkWidgetClass *)dv_class;

	// set our parent class
	parent_class = (GtkDrawingAreaClass *)
		g_type_class_ref (gtk_drawing_area_get_type());
	
	// Disable focus handlers because they emit superfluous expose
	// events, causing flicker.
	widget_class->focus_in_event = NULL;
	widget_class->focus_out_event = NULL;
#if 0
	GType factory_type = AT_DocView_factory_get_type();
	if (factory_type)	// will return NULL if unable to find gail
	{
		atk_registry_set_factory_type(atk_get_default_registry(), ABI_TYPE_DOCVIEW, factory_type);
	}
#endif
}

extern "C" GType
ap_DocView_get_type (void)
{
	static GType abi_type = 0;

	if (!abi_type){
		GTypeInfo info = {
			sizeof (ApDocViewClass),
			NULL,
			NULL,
			(GClassInitFunc)ap_DocView_class_init,
			NULL,
			NULL,
			sizeof(ApDocView),
			0,
			NULL,
			NULL
		};
		
		abi_type = g_type_register_static (gtk_drawing_area_get_type (), "ApDocView",
										   &info, (GTypeFlags)0);
	}
	
	return abi_type;
}

/**
 * ap_DocView_new
 *
 * Creates a new ApDocView widget
 */
extern "C" GtkWidget *
ap_DocView_new (void)
{
	ApDocView * abi;
	UT_DEBUGMSG(("Constructing ApDocView \n"));
	abi = static_cast<ApDocView *>(g_object_new (ap_DocView_get_type (), NULL));

	return GTK_WIDGET (abi);
}

