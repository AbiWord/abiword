/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * go-color-selector.c - A color selector
 *
 * Copyright (c) 2006 Emmanuel pacaud (emmanuel.pacaud@lapp.in2p3.fr)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA.
 */

#include <goffice/goffice-config.h>

#include "go-color-selector.h"

#include <goffice/utils/go-color.h>
#include <goffice/gtk/go-color-group.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtkcolorsel.h>
#include <gtk/gtkcolorseldialog.h>

typedef struct _ColorNamePair ColorNamePair;

struct _ColorNamePair {
	GOColor	color;
	char const *name;	/* english name - eg. "white" */
};

static ColorNamePair const default_color_set [] = {
	{ RGBA_TO_UINT (0x00, 0x00, 0x00, 0xff), N_("black")},
	{ RGBA_TO_UINT (0x99, 0x33, 0x00, 0xff), N_("light brown")},
	{ RGBA_TO_UINT (0x33, 0x33, 0x00, 0xff), N_("brown gold")},
	{ RGBA_TO_UINT (0x00, 0x33, 0x00, 0xff), N_("dark green #2")},
	{ RGBA_TO_UINT (0x00, 0x33, 0x66, 0xff), N_("navy")},
	{ RGBA_TO_UINT (0x00, 0x00, 0x80, 0xff), N_("dark blue")},
	{ RGBA_TO_UINT (0x33, 0x33, 0x99, 0xff), N_("purple #2")},
	{ RGBA_TO_UINT (0x33, 0x33, 0x33, 0xff), N_("very dark gray")},

	{ RGBA_TO_UINT (0x80, 0x00, 0x00, 0xff), N_("dark red")},
	{ RGBA_TO_UINT (0xFF, 0x66, 0x00, 0xff), N_("red-orange")},
	{ RGBA_TO_UINT (0x80, 0x80, 0x00, 0xff), N_("gold")},
	{ RGBA_TO_UINT (0x00, 0x80, 0x00, 0xff), N_("dark green")},
	{ RGBA_TO_UINT (0x00, 0x80, 0x80, 0xff), N_("dull blue")},
	{ RGBA_TO_UINT (0x00, 0x00, 0xFF, 0xff), N_("blue")},
	{ RGBA_TO_UINT (0x66, 0x66, 0x99, 0xff), N_("dull purple")},
	{ RGBA_TO_UINT (0x80, 0x80, 0x80, 0xff), N_("dark gray")},

	{ RGBA_TO_UINT (0xFF, 0x00, 0x00, 0xff), N_("red")},
	{ RGBA_TO_UINT (0xFF, 0x99, 0x00, 0xff), N_("orange")},
	{ RGBA_TO_UINT (0x99, 0xCC, 0x00, 0xff), N_("lime")},
	{ RGBA_TO_UINT (0x33, 0x99, 0x66, 0xff), N_("dull green")},
	{ RGBA_TO_UINT (0x33, 0xCC, 0xCC, 0xff), N_("dull blue #2")},
	{ RGBA_TO_UINT (0x33, 0x66, 0xFF, 0xff), N_("sky blue #2")},
	{ RGBA_TO_UINT (0x80, 0x00, 0x80, 0xff), N_("purple")},
	{ RGBA_TO_UINT (0x96, 0x96, 0x96, 0xff), N_("gray")},

	{ RGBA_TO_UINT (0xFF, 0x00, 0xFF, 0xff), N_("magenta")},
	{ RGBA_TO_UINT (0xFF, 0xCC, 0x00, 0xff), N_("bright orange")},
	{ RGBA_TO_UINT (0xFF, 0xFF, 0x00, 0xff), N_("yellow")},
	{ RGBA_TO_UINT (0x00, 0xFF, 0x00, 0xff), N_("green")},
	{ RGBA_TO_UINT (0x00, 0xFF, 0xFF, 0xff), N_("cyan")},
	{ RGBA_TO_UINT (0x00, 0xCC, 0xFF, 0xff), N_("bright blue")},
	{ RGBA_TO_UINT (0x99, 0x33, 0x66, 0xff), N_("red purple")},
	{ RGBA_TO_UINT (0xC0, 0xC0, 0xC0, 0xff), N_("light gray")},

	{ RGBA_TO_UINT (0xFF, 0x99, 0xCC, 0xff), N_("pink")},
	{ RGBA_TO_UINT (0xFF, 0xCC, 0x99, 0xff), N_("light orange")},
	{ RGBA_TO_UINT (0xFF, 0xFF, 0x99, 0xff), N_("light yellow")},
	{ RGBA_TO_UINT (0xCC, 0xFF, 0xCC, 0xff), N_("light green")},
	{ RGBA_TO_UINT (0xCC, 0xFF, 0xFF, 0xff), N_("light cyan")},
	{ RGBA_TO_UINT (0x99, 0xCC, 0xFF, 0xff), N_("light blue")},
	{ RGBA_TO_UINT (0xCC, 0x99, 0xFF, 0xff), N_("light purple")},
	{ RGBA_TO_UINT (0xFF, 0xFF, 0xFF, 0xff), N_("white")},

	{ 0, NULL},

	/* Disable these for now, they are mostly repeats */
	{ RGBA_TO_UINT (0x99, 0x99, 0xFF, 0xff), N_("purplish blue")},
	{ RGBA_TO_UINT (0x99, 0x33, 0x66, 0xff), N_("red purple")},
	{ RGBA_TO_UINT (0xFF, 0xFF, 0xCC, 0xff), N_("light yellow")},
	{ RGBA_TO_UINT (0xCC, 0xFF, 0xFF, 0xff), N_("light blue")},
	{ RGBA_TO_UINT (0x66, 0x00, 0x66, 0xff), N_("dark purple")},
	{ RGBA_TO_UINT (0xFF, 0x80, 0x80, 0xff), N_("pink")},
	{ RGBA_TO_UINT (0x00, 0x66, 0xCC, 0xff), N_("sky blue")},
	{ RGBA_TO_UINT (0xCC, 0xCC, 0xFF, 0xff), N_("light purple")},

	{ RGBA_TO_UINT (0x00, 0x00, 0x80, 0xff), N_("dark blue")},
	{ RGBA_TO_UINT (0xFF, 0x00, 0xFF, 0xff), N_("magenta")},
	{ RGBA_TO_UINT (0xFF, 0xFF, 0x00, 0xff), N_("yellow")},
	{ RGBA_TO_UINT (0x00, 0xFF, 0xFF, 0xff), N_("cyan")},
	{ RGBA_TO_UINT (0x80, 0x00, 0x80, 0xff), N_("purple")},
	{ RGBA_TO_UINT (0x80, 0x00, 0x00, 0xff), N_("dark red")},
	{ RGBA_TO_UINT (0x00, 0x80, 0x80, 0xff), N_("dull blue")},
	{ RGBA_TO_UINT (0x00, 0x00, 0xFF, 0xff), N_("blue")},

	{ 0, NULL},
};

typedef struct {
	int n_swatches;
	GOColorGroup *color_group;
	GOColor default_color;
} GOColorSelectorState;

static void
go_color_selector_state_free (gpointer data)
{
	GOColorSelectorState *state = data;

	g_object_unref (state->color_group);
	g_free (state);
}

static int
get_index (int n_swatches, GOColorGroup *color_group, GOColor color)
{
	int i = 0;
	int index = -1;

	while (default_color_set[i].name != NULL) {
		if (default_color_set[i].color == color && index < 0) {
			index = i;
			continue;
		}
		i++;
	}
	if (index < 0) {
		go_color_group_add_color (color_group, color);
		index = n_swatches - 1;
	}

	if (index < 0) {
		g_warning ("[GOColorSelector::get_index] Didn't find color in history");
		return 0;
	}

	return index;
}

static GOColor
get_color (int n_swatches, GOColorGroup *color_group, int index)
{
	if (index < 0 || index >= (n_swatches))
		index = 0;

       	if (index < n_swatches - GO_COLOR_GROUP_HISTORY_SIZE)
		return default_color_set[index].color;
	else
		return color_group->history[index - (n_swatches - GO_COLOR_GROUP_HISTORY_SIZE)];
}

GOColor
go_color_selector_get_color (GOSelector *selector, gboolean *is_auto)
{
	GOColorSelectorState *state;
	int index;
	gboolean flag;

	g_return_val_if_fail (GO_IS_SELECTOR (selector), RGB_WHITE);

	index = go_selector_get_active (selector, &flag);
	state = go_selector_get_user_data (selector);
	if (is_auto != NULL)
		*is_auto = flag;

	/* In case default_color is in palette history and its index was moved by the
	 * successive color choices. */
	if (flag)
		return state->default_color;

	return get_color (state->n_swatches, state->color_group, index);
}

static cairo_status_t
draw_check (cairo_surface_t *surface, int width, int height)
{
    cairo_t *cr;
    cairo_status_t status;

    cr = cairo_create (surface);
    cairo_set_source_rgb (cr, 0.75, 0.75, 0.75); /* light gray */
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0.25, 0.25, 0.25); /* dark gray */
    cairo_rectangle (cr, width / 2,  0, width / 2, height / 2);
    cairo_rectangle (cr, 0, height / 2, width / 2, height / 2);
    cairo_fill (cr);

    status = cairo_status (cr);

    cairo_destroy (cr);

    return status;
}

static void
go_color_palette_render_func (cairo_t *cr,
			      GdkRectangle const *area,
			      int index,
			      gpointer data)
{
	GOColor color;
	GOColorSelectorState *state = data;
	cairo_surface_t *check;

	color = get_color (state->n_swatches, state->color_group, index);

	check = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 12, 12);
	draw_check (check, 12, 12);

	cairo_save (cr);
	cairo_set_source_rgb (cr, 1., 1., 1.);
	cairo_paint (cr);
	cairo_set_source_surface (cr, check, 0, 0);
	cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_NEAREST);
	cairo_pattern_set_extend (cairo_get_source (cr), CAIRO_EXTEND_REPEAT);
	cairo_move_to (cr, area->x, area->y + area->height);
	cairo_rel_line_to (cr, area->width, 0);
	cairo_rel_line_to (cr, 0, -area->height);
	cairo_close_path (cr);
	cairo_fill (cr);
	cairo_restore (cr);

	cairo_surface_destroy (check);

	cairo_set_line_width (cr, 1);
	cairo_set_source_rgba (cr,
			       DOUBLE_RGBA_R(color),
			       DOUBLE_RGBA_G(color),
			       DOUBLE_RGBA_B(color),
			       DOUBLE_RGBA_A(color));
	cairo_rectangle (cr, area->x + .5 , area->y + .5 , area->width - 1, area->height - 1);
	cairo_fill_preserve (cr);
	cairo_set_source_rgb (cr, .75, .75, .75);
	cairo_stroke (cr);
}

static void
cb_color_dialog_response (GtkColorSelectionDialog *color_dialog,
			  gint response,
			  GOSelector *selector)
{
	GtkWidget *color_selection;

	color_selection = GTK_COLOR_SELECTION_DIALOG (color_dialog)->colorsel;

	if (response == GTK_RESPONSE_OK) {
		GOColorSelectorState *state;
		GdkColor gdk_color;
		GOColor color;
		guint16 alpha;

		gtk_color_selection_get_current_color (GTK_COLOR_SELECTION (color_selection),
						       &gdk_color);
		alpha = gtk_color_selection_get_current_alpha (GTK_COLOR_SELECTION (color_selection));
		state = go_selector_get_user_data (selector);

		color = GDK_TO_UINT (gdk_color);
		alpha >>= 8;
		color = UINT_RGBA_CHANGE_A (color, alpha);
		if (!go_color_selector_set_color (selector, color))
			/* Index is not necessarly changed, but swatch may change */
			go_selector_activate (selector);
	}

	g_object_set_data (G_OBJECT (selector), "color-dialog", NULL);
}

static void
cb_combo_custom_activate (GOPalette *palette, GOSelector *selector)
{
	GtkWidget *color_dialog;
	GtkWidget *color_selection;
	GdkColor gdk_color;
	GOColor color;

	color_dialog = g_object_get_data (G_OBJECT (selector), "color-dialog");
	if (color_dialog != NULL) {
		color_selection = GTK_COLOR_SELECTION_DIALOG (color_dialog)->colorsel;
		color = go_color_selector_get_color (selector, NULL);
		gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (color_selection),
						       go_color_to_gdk (color, &gdk_color));
		gtk_color_selection_set_current_alpha (GTK_COLOR_SELECTION (color_selection),
						       UINT_RGBA_A (color) * 257);
		gtk_window_present (GTK_WINDOW (color_dialog));
		return;
	}

	color_dialog = gtk_color_selection_dialog_new (_("Custom color..."));
	color_selection = GTK_COLOR_SELECTION_DIALOG (color_dialog)->colorsel;
	g_object_set_data_full (G_OBJECT (selector), "color-dialog", color_dialog,
				(GDestroyNotify) gtk_widget_destroy);
	color = go_color_selector_get_color (selector, NULL);
	gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (color_selection),
					       go_color_to_gdk (color, &gdk_color));
	gtk_color_selection_set_has_opacity_control (GTK_COLOR_SELECTION (color_selection), TRUE);
	gtk_color_selection_set_current_alpha (GTK_COLOR_SELECTION (color_selection),
					       UINT_RGBA_A (color) * 257);
	g_signal_connect (color_dialog, "response", G_CALLBACK (cb_color_dialog_response), selector);
       	gtk_widget_show (color_dialog);
}

static void
go_color_selector_drag_data_received (GOSelector *selector, gpointer data)
{
	GOColor color = RGBA_WHITE;
	guint16 *color_data = data;

	color = UINT_RGBA_CHANGE_R (color, color_data[0] >> 8);
	color = UINT_RGBA_CHANGE_G (color, color_data[1] >> 8);
	color = UINT_RGBA_CHANGE_B (color, color_data[2] >> 8);
	color = UINT_RGBA_CHANGE_A (color, color_data[3] >> 8);

	go_color_selector_set_color (selector, color);
}

static gpointer
go_color_selector_drag_data_get (GOSelector *selector)
{
	GOColorSelectorState *state;
	GOColor color;
	int index;
	guint16 *data;

	state = go_selector_get_user_data (selector);

	data = g_new0 (guint16, 4);
	index = go_selector_get_active (selector, NULL);
	color = get_color (state->n_swatches, state->color_group, index);

	data[0] = UINT_RGBA_R (color) << 8;
	data[1] = UINT_RGBA_G (color) << 8;
	data[2] = UINT_RGBA_B (color) << 8;
	data[3] = UINT_RGBA_A (color) << 8;

	return data;
}

static void
go_color_selector_drag_fill_icon (GOSelector *selector, GdkPixbuf *pixbuf)
{
	GOColorSelectorState *state;
	GOColor color;
	int index;

	state = go_selector_get_user_data (selector);
	index = go_selector_get_active (selector, NULL);
	color = get_color (state->n_swatches, state->color_group, index);

	gdk_pixbuf_fill (pixbuf, color);
}

GtkWidget *
go_color_selector_new (GOColor initial_color,
		       GOColor default_color,
		       char const *group)
{
	GtkWidget *palette;
	GtkWidget *selector;
	GOColorSelectorState *state;
	int count = 0;
	int initial_index;
	int default_index;

	state = g_new (GOColorSelectorState, 1);
	state->default_color = default_color;

	while (default_color_set[count].name != NULL) count ++;
	state->n_swatches = count + GO_COLOR_GROUP_HISTORY_SIZE;
	state->color_group = go_color_group_fetch (group, NULL);

	get_index (state->n_swatches, state->color_group, initial_color);
	default_index = get_index (state->n_swatches, state->color_group, default_color);
	/* In case default_color is not in standard palette */
	initial_index = get_index (state->n_swatches, state->color_group, initial_color);

	palette = go_palette_new (state->n_swatches, 1.0, 8,
				  go_color_palette_render_func,
				  state, go_color_selector_state_free);
	go_palette_show_automatic (GO_PALETTE (palette), default_index, NULL);
	go_palette_show_custom (GO_PALETTE (palette), "Custom color...");
	selector = go_selector_new (GO_PALETTE (palette));
	go_selector_set_active (GO_SELECTOR (selector), initial_index);
	g_signal_connect (palette, "custom-activate", G_CALLBACK (cb_combo_custom_activate), selector);

	go_selector_setup_dnd (GO_SELECTOR (selector), "application/x-color", 8,
			       go_color_selector_drag_data_get,
			       go_color_selector_drag_data_received,
			       go_color_selector_drag_fill_icon);

	return selector;
}

gboolean
go_color_selector_set_color (GOSelector *selector, GOColor color)
{
	GOColorSelectorState *state;
	int index;

	g_return_val_if_fail (GO_IS_SELECTOR (selector), FALSE);

	state =  go_selector_get_user_data (selector);
	g_return_val_if_fail (state != NULL, FALSE);

	index = get_index (state->n_swatches, state->color_group, color);

	return go_selector_set_active (selector, index);
}
