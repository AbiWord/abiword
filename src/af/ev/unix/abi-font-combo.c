/*
 *  Copyright (C) 2005 Robert Staudinger
 *
 *  This software is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "abi-font-combo.h"



/* builtin GtkCellRendererText subclass */

#define ABI_TYPE_CELL_RENDERER_FONT                  (abi_cell_renderer_font_get_type ())
#define ABI_CELL_RENDERER_FONT(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), ABI_TYPE_CELL_RENDERER_FONT, AbiCellRendererFont))
#define ABI_CELL_RENDERER_FONT_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), ABI_TYPE_CELL_RENDERER_FONT, AbiCellRendererFontClass))
#define ABI_IS_CELL_RENDERER_FONT(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ABI_TYPE_CELL_RENDERER_FONT))
#define ABI_IS_CELL_RENDERER_FONT_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), ABI_TYPE_CELL_RENDERER_FONT))
#define ABI_CELL_RENDERER_FONT_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), ABI_TYPE_CELL_RENDERER_FONT, AbiCellRendererFontClass))

typedef struct _AbiCellRendererFont 		AbiCellRendererFont;
typedef struct _AbiCellRendererFontClass 	AbiCellRendererFontClass;

struct _AbiCellRendererFont {
	GtkCellRendererText 	 parent;
	GtkWidget		*parent_widget;
};

struct _AbiCellRendererFontClass {
	GtkCellRendererTextClass parent;

	void (* prelight_popup) (GtkCellRenderer *cell, 
				 const gchar 	 *text);
	void (* render_closed)  (GtkCellRenderer *cell);
};

GType abi_cell_renderer_font_get_type (void);

/* Signal IDs */
enum {
	PRELIGHT_POPUP,
	RENDER_CLOSED,
	RENDERER_LAST_SIGNAL
};

static guint cell_renderer_font_signals[RENDERER_LAST_SIGNAL] = { 0 };

static GtkCellRendererTextClass *abi_cell_renderer_font_parent_class = NULL;

void
abi_cell_renderer_font_render (GtkCellRenderer      *cell,
			       GdkDrawable          *window,
			       GtkWidget            *widget,
			       GdkRectangle         *background_area,
			       GdkRectangle         *cell_area,
			       GdkRectangle         *expose_area,
			       GtkCellRendererState  flags)
{
	AbiCellRendererFont	*self;
	gchar 			*text;

	self = ABI_CELL_RENDERER_FONT (cell);

	text = NULL;
	g_object_get (G_OBJECT (cell), 
		      "text", &text, 
		      NULL);

	GTK_CELL_RENDERER_CLASS (abi_cell_renderer_font_parent_class)->render (
					cell, window, widget, background_area, 
					cell_area, expose_area, flags);

	if ((GTK_CELL_RENDERER_PRELIT | GTK_CELL_RENDERER_PRELIT) & flags) {

		/* only fire prelight event if popup is open */
		if (!gtk_widget_is_ancestor (widget, self->parent_widget)) {
			g_signal_emit (G_OBJECT (cell), 
				       cell_renderer_font_signals[PRELIGHT_POPUP],
				       0, text);
		}
	}
	else if (gtk_widget_is_ancestor (widget, self->parent_widget)) {
		g_signal_emit (G_OBJECT (cell), 
			       cell_renderer_font_signals[RENDER_CLOSED],
			       0);
	}
}

static void
abi_cell_renderer_font_instance_init (AbiCellRendererFont *self)
{}

static void
abi_cell_renderer_font_instance_destroy (GtkObject *instance)
{
	GTK_OBJECT_CLASS (abi_cell_renderer_font_parent_class)->destroy (instance);
}

static void
abi_cell_renderer_font_class_init (AbiCellRendererFontClass *klass)
{
	GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS (klass);
	GtkCellRendererClass *cell_renderer_class = GTK_CELL_RENDERER_CLASS (klass);

	abi_cell_renderer_font_parent_class = (GtkCellRendererTextClass*) gtk_type_class (GTK_TYPE_CELL_RENDERER_TEXT);

	gtk_object_class->destroy = abi_cell_renderer_font_instance_destroy;

	cell_renderer_class->render = abi_cell_renderer_font_render;

	cell_renderer_font_signals[PRELIGHT_POPUP] =
		g_signal_new ("prelight-popup",
			G_OBJECT_CLASS_TYPE (klass),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET (AbiCellRendererFontClass, prelight_popup),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1,
			G_TYPE_STRING);

	cell_renderer_font_signals[RENDER_CLOSED] =
		g_signal_new ("render-closed",
			G_OBJECT_CLASS_TYPE (klass),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET (AbiCellRendererFontClass, render_closed),
			NULL, NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE, 0);
}

GType
abi_cell_renderer_font_get_type (void)
{
        static GType type = 0;
        if (!type) {
                static const GTypeInfo info = {
                        sizeof (AbiCellRendererFontClass),
                        NULL,           /* base_init */
                        NULL,           /* base_finalize */
                        (GClassInitFunc) abi_cell_renderer_font_class_init,
                        NULL,           /* class_finalize */
                        NULL,           /* class_data */
                        sizeof (AbiCellRendererFont),
                        0,              /* n_preallocs */
                        (GInstanceInitFunc) abi_cell_renderer_font_instance_init,
                };
                type = g_type_register_static (GTK_TYPE_CELL_RENDERER_TEXT, 
					       "AbiCellRendererFont", &info, 
					       (GTypeFlags)0);
        }
        return type;
}

GtkCellRenderer *
abi_cell_renderer_font_new (GtkWidget *parent)
{
	GtkCellRenderer *self;
	self = (GtkCellRenderer *) g_object_new (ABI_TYPE_CELL_RENDERER_FONT, 
						 NULL);
	ABI_CELL_RENDERER_FONT (self)->parent_widget = parent;
	return self;
}



/* GtkComboBox subclass */

/* Signal IDs */
enum {
	PRELIGHT,
	POPUP_CLOSED,
	LAST_SIGNAL
};

static guint font_combo_signals[LAST_SIGNAL] = { 0 };

static GtkComboBoxClass *abi_font_combo_parent_class = NULL;

static void
prelight_popup_cb (AbiFontCombo		*self, 
		   const gchar		*text, 
		   GtkCellRenderer 	*renderer)
{
	g_signal_emit (G_OBJECT (self), font_combo_signals[PRELIGHT],
		       0, text);
}

static void
render_closed_cb (AbiFontCombo		*self, 
		  GtkCellRenderer 	*renderer)
{
	g_signal_emit (G_OBJECT (self), font_combo_signals[POPUP_CLOSED],
		       0);
}

static void
abi_font_combo_init (AbiFontCombo *self)
{}

static void
abi_font_combo_destroy (GtkObject *instance)
{
	GTK_OBJECT_CLASS (abi_font_combo_parent_class)->destroy (instance);
}

static void
abi_font_combo_class_init (AbiFontComboClass *klass)
{
	GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS (klass);

	abi_font_combo_parent_class = (GtkComboBoxClass*) gtk_type_class (GTK_TYPE_COMBO_BOX);

	gtk_object_class = (GtkObjectClass *)klass;
	gtk_object_class->destroy = abi_font_combo_destroy;

	font_combo_signals[PRELIGHT] =
		g_signal_new ("prelight",
			G_OBJECT_CLASS_TYPE (klass),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET (AbiFontComboClass, prelight),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1,
			G_TYPE_STRING);

	font_combo_signals[POPUP_CLOSED] =
		g_signal_new ("popup-closed",
			G_OBJECT_CLASS_TYPE (klass),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET (AbiFontComboClass, popup_closed),
			NULL, NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE, 0);
}

GType
abi_font_combo_get_type (void)
{
        static GType type = 0;
        if (!type) {
                static const GTypeInfo info = {
                        sizeof (AbiFontComboClass),
                        NULL,           /* base_init */
                        NULL,           /* base_finalize */
                        (GClassInitFunc) abi_font_combo_class_init,
                        NULL,           /* class_finalize */
                        NULL,           /* class_data */
                        sizeof (AbiFontCombo),
                        0,              /* n_preallocs */
                        (GInstanceInitFunc) abi_font_combo_init,
                };
                type = g_type_register_static (GTK_TYPE_COMBO_BOX, 
					       "AbiFontCombo", &info, 
					       (GTypeFlags)0);
        }
        return type;
}

GtkWidget *
abi_font_combo_new (void)
{
	GtkWidget 	*self;
	GtkListStore 	*store;
	GtkCellRenderer *cell;

	self = (GtkWidget *) g_object_new (ABI_TYPE_FONT_COMBO, NULL);
	store = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_combo_box_set_model (GTK_COMBO_BOX (self), GTK_TREE_MODEL (store));
	g_object_unref (store);

	cell = abi_cell_renderer_font_new (self);
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (self), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (self), cell,
					"text", 0,
					NULL);

	g_signal_connect_swapped (G_OBJECT (cell), "prelight-popup", 
				  G_CALLBACK (prelight_popup_cb), (gpointer) self);
	g_signal_connect_swapped (G_OBJECT (cell), "render-closed", 
				  G_CALLBACK (render_closed_cb), (gpointer) self);

	return self;
}
