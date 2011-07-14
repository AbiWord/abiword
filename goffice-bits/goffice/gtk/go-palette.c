/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * go-palette.c :
 *
 * Copyright (C) 2006 Emmanuel Pacaud (emmanuel.pacaud@lapp.in2p3.fr)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <goffice/goffice-config.h>

#include "go-palette.h"

#include <gtk/gtkalignment.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkarrow.h>
#include <gtk/gtkcelllayout.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkcellview.h>
#include <gtk/gtkcombobox.h>
#include <gtk/gtkdrawingarea.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkiconview.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkseparatormenuitem.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkvseparator.h>
#include <glib/gi18n-lib.h>

#include <gdk/gdkkeysyms.h>

#include <math.h>

enum {
	GO_PALETTE_ACTIVATE,
	GO_PALETTE_AUTOMATIC_ACTIVATE,
	GO_PALETTE_CUSTOM_ACTIVATE,
	GO_PALETTE_LAST_SIGNAL
};

static guint go_palette_signals[GO_PALETTE_LAST_SIGNAL] = {0,};

static void 		go_palette_finalize   	 (GObject		*object);
static GtkWidget       *go_palette_menu_item_new (GOPalette *palette, int index);
static void 		cb_automatic_activate 	 (GtkWidget *item, GOPalette *palette);
static void 		cb_custom_activate 	 (GtkWidget *item, GOPalette *palette);

#define GO_PALETTE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GO_TYPE_PALETTE, GOPalettePrivate))

struct _GOPalettePrivate {
	int		 n_swatches;
	int 		 n_columns;

	int		 swatch_width;
	int		 swatch_height;

	GOPaletteSwatchRenderCallback swatch_render; 
	gpointer 	 data;
	GDestroyNotify	 destroy;

	gboolean 	 show_automatic;
	GtkWidget	*automatic;
	GtkWidget	*automatic_separator;
	char		*automatic_label;
	int		 automatic_index;
	
	gboolean	 show_custom;
	GtkWidget	*custom;
	GtkWidget	*custom_separator;
	char		*custom_label;
};

G_DEFINE_TYPE (GOPalette, go_palette, GTK_TYPE_MENU)

static void
go_palette_init (GOPalette *palette)
{
	GOPalettePrivate *priv;
	PangoLayout *layout;
	PangoRectangle rect;

	priv = GO_PALETTE_GET_PRIVATE (palette);

	palette->priv = priv;

	priv->n_swatches = 0;
	priv->n_columns = 1;

	priv->swatch_render = NULL;
	priv->data = NULL;
	priv->destroy = NULL;

	priv->automatic = NULL;
	priv->automatic_separator = NULL;
	priv->automatic_label = NULL;
	priv->custom = NULL;
	priv->custom_separator = NULL;
	priv->custom_label = NULL;
	priv->show_automatic = FALSE;
	priv->show_custom = FALSE;

	priv->automatic_index = 0;
	
	layout = gtk_widget_create_pango_layout (GTK_WIDGET (palette), "A");
	pango_layout_get_pixel_extents (layout, NULL, &rect);
	g_object_unref (layout);

	priv->swatch_height = rect.height + 2;
	priv->swatch_width = priv->swatch_height;
}

static void
go_palette_realize (GtkWidget *widget)
{
	GOPalette *palette = GO_PALETTE (widget);
	GOPalettePrivate *priv = palette->priv;
	GtkWidget *item;
	int i, row;

	for (i = 0; i < priv->n_swatches; i++) {
		item = go_palette_menu_item_new (GO_PALETTE (palette), i);
		gtk_menu_attach (GTK_MENU (palette), item, i % priv->n_columns, i % priv->n_columns + 1,
				 i / priv->n_columns + 2, i / priv->n_columns + 3);
		gtk_widget_show (item);
	}

	if (priv->show_automatic) {
		priv->automatic = gtk_menu_item_new_with_label (priv->automatic_label);
		gtk_menu_attach	(GTK_MENU (palette), priv->automatic, 0, priv->n_columns, 0, 1);
		g_signal_connect (priv->automatic, "activate", G_CALLBACK (cb_automatic_activate), palette); 
		priv->automatic_separator = gtk_separator_menu_item_new ();
		gtk_menu_attach	(GTK_MENU (palette), priv->automatic_separator, 0, priv->n_columns, 1, 2);
		gtk_widget_show (GTK_WIDGET (palette->priv->automatic));
		gtk_widget_show (GTK_WIDGET (palette->priv->automatic_separator));
	}

	if (priv->show_custom) {
		row = ((priv->n_swatches - 1) / priv->n_columns) + 3;

		priv->custom_separator = gtk_separator_menu_item_new ();
		gtk_menu_attach (GTK_MENU (palette), priv->custom_separator, 0, priv->n_columns, 
				 row, row + 1);
		priv->custom = gtk_menu_item_new_with_label (priv->custom_label);
		gtk_menu_attach (GTK_MENU (palette), priv->custom, 0, priv->n_columns, 
				 row + 1, row + 2);
		g_signal_connect (priv->custom, "activate", G_CALLBACK (cb_custom_activate), palette); 
		gtk_widget_show (GTK_WIDGET (palette->priv->custom));
		gtk_widget_show (GTK_WIDGET (palette->priv->custom_separator));
	}
	
	GTK_WIDGET_CLASS (go_palette_parent_class)->realize (widget);
}

static void
go_palette_class_init (GOPaletteClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);
	
	object_class->finalize = go_palette_finalize;
	widget_class->realize = go_palette_realize;

	go_palette_signals[GO_PALETTE_ACTIVATE] =
		g_signal_new ("activate",
			      G_OBJECT_CLASS_TYPE (class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GOPaletteClass, activate),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__INT,
			      G_TYPE_NONE, 1, G_TYPE_INT);

	go_palette_signals[GO_PALETTE_AUTOMATIC_ACTIVATE] =
		g_signal_new ("automatic-activate",
			      G_OBJECT_CLASS_TYPE (class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GOPaletteClass, automatic_activate),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__INT,
			      G_TYPE_NONE, 1, G_TYPE_INT);
	
	go_palette_signals[GO_PALETTE_CUSTOM_ACTIVATE] =
		g_signal_new ("custom-activate",
			      G_OBJECT_CLASS_TYPE (class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GOPaletteClass, custom_activate),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
	
	g_type_class_add_private (object_class, sizeof (GOPalettePrivate));
}

static void
go_palette_finalize (GObject *object)
{
	GOPalettePrivate *priv;

	priv = GO_PALETTE_GET_PRIVATE (object);

	if (priv->data && priv->destroy)
		(priv->destroy) (priv->data);
	g_free (priv->automatic_label);
	g_free (priv->custom_label);

	(* G_OBJECT_CLASS (go_palette_parent_class)->finalize) (object);
}

static gboolean
cb_swatch_expose (GtkWidget *swatch, GdkEventExpose *event, GOPalette *palette)
{
	if (palette->priv->swatch_render) {
		cairo_t *cr = gdk_cairo_create (swatch->window);
		GdkRectangle area;
		int index;

		area.x = 0;
		area.y = 0;
		area.width = swatch->allocation.width;
		area.height = swatch->allocation.height;

		index = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (swatch), "index"));

		(palette->priv->swatch_render) (cr, &area, index, palette->priv->data);

		cairo_destroy (cr);
	}
	return TRUE;
}

GtkWidget *
go_palette_swatch_new (GOPalette *palette, int index)
{
	GtkWidget *swatch;
	
	swatch = gtk_drawing_area_new ();

	g_object_set_data (G_OBJECT (swatch), "index", GINT_TO_POINTER (index));
	g_signal_connect (G_OBJECT (swatch), "expose-event", G_CALLBACK (cb_swatch_expose), palette);
	gtk_widget_set_size_request (swatch, 
				     palette->priv->swatch_width, 
				     palette->priv->swatch_height);

	gtk_widget_show (swatch);

	return swatch;
}

static void
cb_menu_item_activate (GtkWidget *item, GOPalette *palette)
{
	int index;

	index = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (GTK_BIN (item)->child), "index"));
	g_signal_emit (palette, go_palette_signals[GO_PALETTE_ACTIVATE], 0, index);
}

static GtkWidget *
go_palette_menu_item_new (GOPalette *palette, int index)
{
	GtkWidget *swatch; 
	GtkWidget *item;

	item = gtk_menu_item_new ();
	swatch = go_palette_swatch_new (palette, index);
	gtk_container_add (GTK_CONTAINER (item), swatch);
	
	g_signal_connect (item, "activate", G_CALLBACK (cb_menu_item_activate), palette); 

	return item;
}

static void
cb_automatic_activate (GtkWidget *item, GOPalette *palette)
{
	g_signal_emit (palette, go_palette_signals[GO_PALETTE_AUTOMATIC_ACTIVATE], 0, 
		       palette->priv->automatic_index);
}

static void
cb_custom_activate (GtkWidget *item, GOPalette *palette)
{
	g_signal_emit (palette, go_palette_signals[GO_PALETTE_CUSTOM_ACTIVATE], 0);
}

/**
 * go_palette_new:
 *
 * Returns a new #GOPalette object.
 **/
GtkWidget *
go_palette_new (int n_swatches,
		double swatch_width,
		int n_columns,
		GOPaletteSwatchRenderCallback swatch_render,
		gpointer data,
		GDestroyNotify destroy)
			
{
	GOPalettePrivate *priv;
	GtkWidget *palette;

	palette = g_object_new (GO_TYPE_PALETTE, NULL);

	g_return_val_if_fail (n_swatches >= 1, palette);

	priv = GO_PALETTE (palette)->priv;

	priv->n_swatches = n_swatches;
	priv->swatch_render = swatch_render;
	priv->data = data;
	priv->destroy = destroy;
	if (swatch_width > 0.)
		priv->swatch_width = priv->swatch_height * swatch_width;

	if (n_columns < 1)
		n_columns = 1;
	priv->n_columns = n_columns;

	return palette;
}

void
go_palette_show_automatic (GOPalette *palette, 
			   int index,
			   char const *label)
{
	GOPalettePrivate *priv;

	g_return_if_fail (GO_IS_PALETTE (palette));

	priv = palette->priv;
	g_return_if_fail (!priv->show_automatic);

	priv->automatic_label = g_strdup (label == NULL ?  _("Automatic"): _(label));	
	priv->automatic_index = index;
	priv->show_automatic = TRUE;
}

void
go_palette_show_custom (GOPalette *palette, 
			char const *label)
{
	GOPalettePrivate *priv;
	
	g_return_if_fail (GO_IS_PALETTE (palette));

	priv = palette->priv;
	g_return_if_fail (!priv->show_custom);

	priv->custom_label = g_strdup (label == NULL ?  _("Custom...") : _(label));
	priv->show_custom = TRUE;
}

int
go_palette_get_n_swatches (GOPalette *palette)
{
	return palette->priv->n_swatches;
}

gpointer
go_palette_get_user_data (GOPalette *palette)
{
	g_return_val_if_fail (GO_IS_PALETTE (palette), NULL);
	
	return palette->priv->data;		      
}
