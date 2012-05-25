/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * go-palette.h :
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

#ifndef GO_PALETTE_H
#define GO_PALETTE_H

#include <gtk/gtkmenu.h>

G_BEGIN_DECLS

typedef void (*GOPaletteSwatchRenderCallback)	(cairo_t *cr,
						 GdkRectangle const *area,
						 int index,
						 gpointer data);

#define GO_TYPE_PALETTE			(go_palette_get_type ())
#define GO_PALETTE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GO_TYPE_PALETTE, GOPalette))
#define GO_PALETTE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GO_TYPE_PALETTE, GOPaletteClass))
#define GO_IS_PALETTE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GO_TYPE_PALETTE))
#define GO_IS_PALETTE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GO_TYPE_PALETTE))
#define GO_PALETTE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GO_TYPE_PALETTE, GOPaletteClass))

typedef struct _GOPalette GOPalette;
typedef struct _GOPalettePrivate GOPalettePrivate;
typedef struct _GOPaletteClass GOPaletteClass;

struct _GOPalette
{
	GtkMenu parent;

	GOPalettePrivate *priv;
};

struct _GOPaletteClass
{
	GtkMenuClass parent_class;

	/* signals */
	void (*activate)		(GtkWidget *palette, int index);
	void (*automatic_activate)	(GtkWidget *palette, int index);
	void (*custom_activate)		(GtkWidget *palette);
};

GType            go_palette_get_type 		(void) G_GNUC_CONST;
GtkWidget 	*go_palette_new 		(int n_swatches,
						 double swatch_width,
						 int n_colmuns,
						 GOPaletteSwatchRenderCallback swatch_render,
						 gpointer data,
						 GDestroyNotify destroy);
void 		 go_palette_show_automatic 	(GOPalette *palette, int index, char const *label);
void 		 go_palette_show_custom		(GOPalette *palette, char const *label);

gpointer 	 go_palette_get_user_data 	(GOPalette *palette);

GtkWidget 	*go_palette_swatch_new 		(GOPalette *palette, int index);
int		 go_palette_get_n_swatches 	(GOPalette *palette);
G_END_DECLS

#endif /* GO_PALETTE_H */
