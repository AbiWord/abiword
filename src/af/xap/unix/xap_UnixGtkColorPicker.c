/*
 * Copyright (C) 1998, 1999 Red Hat, Inc.
 * All rights reserved.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/*
  @NOTATION@
 */
/* Color picker button (ported from GNOME to GTK2)
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 * Ported to GTK2 by: Marc Maurer <j.m.maurer@student.utwente.nl>
 */

#include <gtk/gtkmain.h>
#include <gtk/gtkalignment.h>
#include <gtk/gtkcolorsel.h>
#include <gtk/gtkcolorseldialog.h>
#include <gtk/gtkdnd.h>
#include <gtk/gtkdrawingarea.h>
#include <gtk/gtkframe.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkbutton.h>
#include "gtkintl.h"

#include "xap_UnixGtkColorPicker.h"
#include <gdk/gdkkeysyms.h>

/* These are the dimensions of the color sample in the color picker */
#define COLOR_PICKER_WIDTH  20
#define COLOR_PICKER_HEIGHT 12
#define COLOR_PICKER_PAD    1

/* Size of checks and gray levels for alpha compositing checkerboard*/
#define CHECK_SIZE  4
#define CHECK_DARK  (1.0 / 3.0)
#define CHECK_LIGHT (2.0 / 3.0)

struct _GtkColorPickerPrivate {
	GdkPixbuf *pixbuf;	/* Pixbuf for rendering dithered sample */
	GdkGC *gc;		/* GC for drawing */

	GtkWidget *drawing_area;/* Drawing area for color sample */
	GtkWidget *cs_dialog;	/* Color selection dialog */

	gchar *title;		/* Title for the color selection window */

	gdouble r, g, b, a;	/* Red, green, blue, and alpha values */

	guint dither : 1;	/* Dither or just paint a solid color? */
	guint use_alpha : 1;	/* Use alpha or not */
};

enum {
	PROP_0,
	PROP_DITHER,
	PROP_USE_ALPHA,
	PROP_TITLE,
	PROP_RED,
	PROP_GREEN,
	PROP_BLUE,
	PROP_ALPHA,
	PROP_COLOR,
	PROP_COLOR_GDK,
	PROP_COLOR_RGBA
};

enum {
	COLOR_SET,
	LAST_SIGNAL
};

static void gtk_color_picker_class_init (GtkColorPickerClass *class);
static void gtk_color_picker_instance_init (GtkColorPicker *cp);
static void gtk_color_picker_destroy    (GtkObject             *object);
static void gtk_color_picker_finalize   (GObject               *object);
static void gtk_color_picker_clicked    (GtkButton             *button);
static void gtk_color_picker_state_changed (GtkWidget *widget, GtkStateType previous_state);
static void gtk_color_picker_realize (GtkWidget *widget);
static void gtk_color_picker_style_set (GtkWidget *widget, GtkStyle *previous_style);
static void drag_data_get		(GtkWidget          *widget,
					 GdkDragContext     *context,
					 GtkSelectionData   *selection_data,
					 guint               info,
					 guint               time,
					 GtkColorPicker   *cpicker);
static void drag_data_received		(GtkWidget        *widget,
					 GdkDragContext   *context,
					 gint              x,
					 gint              y,
					 GtkSelectionData *selection_data,
					 guint             info,
					 guint32           time,
					 GtkColorPicker *cpicker);
static void gtk_color_picker_set_property (GObject            *object,
					     guint               param_id,
					     const GValue       *value,
					     GParamSpec         *pspec);
static void gtk_color_picker_get_property (GObject            *object,
					     guint               param_id,
					     GValue             *value,
					     GParamSpec         *pspec);


static GtkButtonClass *parent_class = NULL;
static guint color_picker_signals[LAST_SIGNAL] = { 0 };
static GtkTargetEntry drop_types[] = { { "application/x-color", 0, 0 } };

GtkType
gtk_color_picker_get_type (void)
{
  static GtkType color_picker_type = 0;

  if (!color_picker_type)
    {
      static const GTypeInfo color_picker_info =
      {
	sizeof (GtkColorPickerClass),
	NULL,		/* base_init */
	NULL,		/* base_finalize */
	(GClassInitFunc) gtk_color_picker_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data */
	sizeof (GtkColorPicker),
	16,		/* n_preallocs */
	(GInstanceInitFunc) gtk_color_picker_instance_init,
      };

      color_picker_type = g_type_register_static (GTK_TYPE_BUTTON, "GtkColorPicker", &color_picker_info, 0);
    }

  return color_picker_type;
}

static void
gtk_color_picker_class_init (GtkColorPickerClass *class)
{
	GtkObjectClass *object_class;
	GObjectClass *gobject_class;
	GtkWidgetClass *widget_class;
	GtkButtonClass *button_class;

	object_class = (GtkObjectClass *) class;
	gobject_class = (GObjectClass *) class;
	button_class = (GtkButtonClass *) class;
	widget_class = (GtkWidgetClass *) class;
	parent_class = g_type_class_peek_parent (class);
	
	color_picker_signals[COLOR_SET] =
    gtk_signal_new ("color_set",
                    GTK_RUN_FIRST,
                    GTK_CLASS_TYPE (object_class),
                    GTK_SIGNAL_OFFSET (GtkColorPickerClass, color_set),
					g_cclosure_marshal_VOID__VOID/*_gtk_marshal_VOID__VOID*/,
					GTK_TYPE_NONE, 0);

	gobject_class->get_property = gtk_color_picker_get_property;
	gobject_class->set_property = gtk_color_picker_set_property;

        g_object_class_install_property
                (gobject_class,
                 PROP_DITHER,
                 g_param_spec_boolean ("dither", NULL, 
			               "Whether or not to dither to color",
				       TRUE,
				       (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_USE_ALPHA,
                 g_param_spec_boolean ("use_alpha", NULL, 
			               "Whether or not to give the color an alpha value",
				       FALSE,
				       (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_TITLE,
                 g_param_spec_string ("title", NULL, 
			              "The title to give to the color picker",
				      NULL,
				      (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_RED,
                 g_param_spec_uint ("red", NULL,
			            "The amount of red in the chosen color",
				    0, 255, 0,
				    (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_GREEN,
                 g_param_spec_uint ("green", NULL, 
			            "The amount of green in the chosen color",
				    0, 255, 0,
				    (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_BLUE,
                 g_param_spec_uint ("blue", NULL, 
			            "The amount of blue in the chosen color",
				    0, 255, 0,
				    (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_ALPHA,
                 g_param_spec_uint ("alpha", NULL, 
			            "The alpha value of the chosen color",
				    0, 255, 0,
				    (G_PARAM_READABLE | G_PARAM_WRITABLE)));

	object_class->destroy = gtk_color_picker_destroy;
	gobject_class->finalize = gtk_color_picker_finalize;
	widget_class->state_changed = gtk_color_picker_state_changed;
	widget_class->realize = gtk_color_picker_realize;
	widget_class->style_set = gtk_color_picker_style_set;
	button_class->clicked = gtk_color_picker_clicked;

	class->color_set = NULL;
}

/* Renders the pixmap for the case of dithered or use_alpha */
static void
render_dither (GtkColorPicker *cp)
{
	gint dark_r, dark_g, dark_b;
	gint light_r, light_g, light_b;
	gint i, j, rowstride;
	gint c1[3], c2[3];
	guchar *pixels;
	guint8 insensitive_r = 0;
	guint8 insensitive_g = 0;
	guint8 insensitive_b = 0;

	/* Compute dark and light check colors */

	insensitive_r = GTK_WIDGET(cp)->style->bg[GTK_STATE_INSENSITIVE].red >> 8;
	insensitive_g = GTK_WIDGET(cp)->style->bg[GTK_STATE_INSENSITIVE].green >> 8;
	insensitive_b = GTK_WIDGET(cp)->style->bg[GTK_STATE_INSENSITIVE].blue >> 8;

	if (cp->_priv->use_alpha) {
		dark_r = (int) ((CHECK_DARK + (cp->_priv->r - CHECK_DARK) * cp->_priv->a) * 255.0 + 0.5);
		dark_g = (int) ((CHECK_DARK + (cp->_priv->g - CHECK_DARK) * cp->_priv->a) * 255.0 + 0.5);
		dark_b = (int) ((CHECK_DARK + (cp->_priv->b - CHECK_DARK) * cp->_priv->a) * 255.0 + 0.5);

		light_r = (int) ((CHECK_LIGHT + (cp->_priv->r - CHECK_LIGHT) * cp->_priv->a) * 255.0 + 0.5);
		light_g = (int) ((CHECK_LIGHT + (cp->_priv->g - CHECK_LIGHT) * cp->_priv->a) * 255.0 + 0.5);
		light_b = (int) ((CHECK_LIGHT + (cp->_priv->b - CHECK_LIGHT) * cp->_priv->a) * 255.0 + 0.5);
	} else {
		dark_r = light_r = (int) (cp->_priv->r * 255.0 + 0.5);
		dark_g = light_g = (int) (cp->_priv->g * 255.0 + 0.5);
		dark_b = light_b = (int) (cp->_priv->b * 255.0 + 0.5);
	}

	/* Fill image buffer */

	pixels = gdk_pixbuf_get_pixels (cp->_priv->pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (cp->_priv->pixbuf);
	for (j = 0; j < COLOR_PICKER_HEIGHT; j++) {
		if ((j / CHECK_SIZE) & 1) {
			c1[0] = dark_r;
			c1[1] = dark_g;
			c1[2] = dark_b;

			c2[0] = light_r;
			c2[1] = light_g;
			c2[2] = light_b;
		} else {
			c1[0] = light_r;
			c1[1] = light_g;
			c1[2] = light_b;

			c2[0] = dark_r;
			c2[1] = dark_g;
			c2[2] = dark_b;
		}

		for (i = 0; i < COLOR_PICKER_WIDTH; i++) {
			if (!GTK_WIDGET_SENSITIVE (GTK_WIDGET (cp)) && (i+j)%2) {
				*(pixels + j * rowstride + i * 3) = insensitive_r;
				*(pixels + j * rowstride + i * 3 + 1) = insensitive_g;
				*(pixels + j * rowstride + i * 3 + 2) = insensitive_b;
			} else if ((i / CHECK_SIZE) & 1) {
				*(pixels + j * rowstride + i * 3) = c1[0];
				*(pixels + j * rowstride + i * 3 + 1) = c1[1];
				*(pixels + j * rowstride + i * 3 + 2) = c1[2];
			} else {
				*(pixels + j * rowstride + i * 3) = c2[0];
				*(pixels + j * rowstride + i * 3 + 1) = c2[1];
				*(pixels + j * rowstride + i * 3 + 2) = c2[2];
			}
		}
	}
	if (cp->_priv->drawing_area->window)
		gdk_pixbuf_render_to_drawable (cp->_priv->pixbuf,
					       cp->_priv->drawing_area->window,
					       cp->_priv->gc,
					       0, 0, 0, 0,
					       COLOR_PICKER_WIDTH,
					       COLOR_PICKER_HEIGHT,
					       GDK_RGB_DITHER_MAX,
					       0, 0);
}

/* Renders the pixmap with the contents of the color sample */
static void
render (GtkColorPicker *cp)
{
	if (cp->_priv->dither || cp->_priv->use_alpha)
		render_dither (cp);
	else {
		gint i, j, rowstride;
		guint8 insensitive_r = 0;
		guint8 insensitive_g = 0;
		guint8 insensitive_b = 0;
		guchar *pixels;

		pixels = gdk_pixbuf_get_pixels (cp->_priv->pixbuf);
		rowstride = gdk_pixbuf_get_rowstride (cp->_priv->pixbuf);
		insensitive_r = GTK_WIDGET (cp)->style->bg[GTK_STATE_INSENSITIVE].red >> 8;
		insensitive_g = GTK_WIDGET (cp)->style->bg[GTK_STATE_INSENSITIVE].green >> 8;
		insensitive_b = GTK_WIDGET (cp)->style->bg[GTK_STATE_INSENSITIVE].blue >> 8;

		for (i = 0; i < COLOR_PICKER_WIDTH; i++) {
			for (j = 0; j < COLOR_PICKER_HEIGHT; j++) {
				if (!GTK_WIDGET_SENSITIVE (GTK_WIDGET (cp)) && (i+j)%2) {
					*(pixels + j * rowstride + i * 3) = insensitive_r;
					*(pixels + j * rowstride + i * 3 + 1) = insensitive_g;
					*(pixels + j * rowstride + i * 3 + 2) = insensitive_b;
				} else {
					*(pixels + j * rowstride + i * 3) = cp->_priv->r * 255.0 + 0.5;
					*(pixels + j * rowstride + i * 3 + 1) = cp->_priv->g * 255.0 + 0.5;
					*(pixels + j * rowstride + i * 3 + 2) = cp->_priv->b * 255.0 + 0.5;
				}
			}
		}
	}
}

/* Handle exposure events for the color picker's drawing area */
static gint
expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	GtkColorPicker *cp;

	cp = GTK_COLOR_PICKER (data);

	gdk_pixbuf_render_to_drawable (cp->_priv->pixbuf,
				       widget->window,
				       cp->_priv->gc,
				       event->area.x,
				       event->area.y,
				       event->area.x,
				       event->area.y,
				       event->area.width,
				       event->area.height,
				       GDK_RGB_DITHER_MAX,
				       event->area.x,
				       event->area.y);
	return FALSE;
}
static void
gtk_color_picker_realize (GtkWidget *widget)
{
	GtkColorPicker *cp = GTK_COLOR_PICKER (widget);

	if (GTK_WIDGET_CLASS (parent_class)->realize)
		GTK_WIDGET_CLASS (parent_class)->realize (widget);

	if (cp->_priv->gc == NULL)
		cp->_priv->gc = gdk_gc_new (widget->window);

	render (cp);
}

static void
gtk_color_picker_style_set (GtkWidget *widget, GtkStyle *previous_style)
{
	if (GTK_WIDGET_CLASS (parent_class)->style_set)
		GTK_WIDGET_CLASS (parent_class)->style_set (widget, previous_style);	

	if (GTK_WIDGET_REALIZED (widget))
		render (GTK_COLOR_PICKER (widget));
}

static void
gtk_color_picker_state_changed (GtkWidget *widget, GtkStateType previous_state)
{
	if (widget->state == GTK_STATE_INSENSITIVE || previous_state == GTK_STATE_INSENSITIVE)
		render (GTK_COLOR_PICKER (widget));
}

static void
drag_data_received (GtkWidget        *widget,
		    GdkDragContext   *context,
		    gint              x,
		    gint              y,
		    GtkSelectionData *selection_data,
		    guint             info,
		    guint32           time,
		    GtkColorPicker   *cpicker)
{
	guint16 *dropped;

	g_return_if_fail (cpicker != NULL);
	g_return_if_fail (GTK_IS_COLOR_PICKER (cpicker));

	if (selection_data->length < 0)
		return;

	if ((selection_data->format != 16) ||
	    (selection_data->length != 8)) {
		g_warning (_("Received invalid color data\n"));
		return;
	}


	dropped = (guint16 *)selection_data->data;

	gtk_color_picker_set_i16(cpicker, dropped[0], dropped[1],
				   dropped[2], dropped[3]);
	g_signal_emit (cpicker, color_picker_signals[COLOR_SET], 0,
		       dropped[0], dropped[1], dropped[2], dropped[3]);
		
}

static void
drag_data_get  (GtkWidget          *widget,
		GdkDragContext     *context,
		GtkSelectionData   *selection_data,
		guint               info,
		guint               time,
		GtkColorPicker     *cpicker)
{
	gushort r, g, b, a;
	guint16 dropped[4];

	gtk_color_picker_get_i16(cpicker, &r, &g, &b, &a);

	dropped[0] = r;
	dropped[1] = g;
	dropped[2] = b;
	dropped[3] = a;

	gtk_selection_data_set (selection_data,
				selection_data->target,
				16/*fromat*/, (guchar *)dropped, 8/*length*/);
}

static void
gtk_color_picker_instance_init (GtkColorPicker *cp)
{
	GtkWidget *alignment;
	GtkWidget *frame;

	/* Create the widgets */
	cp->_priv = g_new0(GtkColorPickerPrivate, 1);

	alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
	gtk_container_set_border_width (GTK_CONTAINER (alignment), COLOR_PICKER_PAD);
	gtk_container_add (GTK_CONTAINER (cp), alignment);
	gtk_widget_show (alignment);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
	gtk_container_add (GTK_CONTAINER (alignment), frame);
	gtk_widget_show (frame);

	gtk_widget_push_colormap (gdk_rgb_get_colormap ());

	cp->_priv->drawing_area = gtk_drawing_area_new ();

	gtk_widget_set_size_request (cp->_priv->drawing_area, COLOR_PICKER_WIDTH, COLOR_PICKER_HEIGHT);
	g_signal_connect (cp->_priv->drawing_area, "expose_event",
			  G_CALLBACK (expose_event), cp);
	gtk_container_add (GTK_CONTAINER (frame), cp->_priv->drawing_area);
	gtk_widget_show (cp->_priv->drawing_area);

	cp->_priv->title = g_strdup (_("Pick a color")); /* default title */

	/* Create the buffer for the image so that we can create an image.  Also create the
	 * picker's pixmap.
	 */

	cp->_priv->pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, COLOR_PICKER_WIDTH, COLOR_PICKER_HEIGHT);

	cp->_priv->gc = NULL;
	gtk_widget_pop_colormap ();

	/* Start with opaque black, dither on, alpha disabled */

	cp->_priv->r = 0.0;
	cp->_priv->g = 0.0;
	cp->_priv->b = 0.0;
	cp->_priv->a = 1.0;
	cp->_priv->dither = TRUE;
	cp->_priv->use_alpha = FALSE;

	gtk_drag_dest_set (GTK_WIDGET (cp),
			   GTK_DEST_DEFAULT_MOTION |
			   GTK_DEST_DEFAULT_HIGHLIGHT |
			   GTK_DEST_DEFAULT_DROP,
			   drop_types, 1, GDK_ACTION_COPY);
	gtk_drag_source_set (GTK_WIDGET(cp),
			     GDK_BUTTON1_MASK|GDK_BUTTON3_MASK,
			     drop_types, 1,
			     GDK_ACTION_COPY);
	g_signal_connect (cp, "drag_data_received",
			  G_CALLBACK (drag_data_received), cp);
	g_signal_connect (cp, "drag_data_get",
			  G_CALLBACK (drag_data_get), cp);
}

static void
gtk_color_picker_destroy (GtkObject *object)
{
	GtkColorPicker *cp;

	/* remember, destroy can be run multiple times! */

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_COLOR_PICKER (object));

	cp = GTK_COLOR_PICKER (object);

	if (cp->_priv->gc != NULL)
		g_object_unref (G_OBJECT (cp->_priv->gc));
	cp->_priv->gc = NULL;

	if (cp->_priv->cs_dialog != NULL)
		gtk_widget_destroy (cp->_priv->cs_dialog);
	cp->_priv->cs_dialog = NULL;

	(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gtk_color_picker_finalize (GObject *object)
{
	GtkColorPicker *cp;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_COLOR_PICKER (object));

	cp = GTK_COLOR_PICKER (object);

	if (cp->_priv->pixbuf != NULL)
		g_object_unref (G_OBJECT (cp->_priv->pixbuf));
	cp->_priv->pixbuf = NULL;

	g_free (cp->_priv->title);
	cp->_priv->title = NULL;

	g_free (cp->_priv);
	cp->_priv = NULL;
}


/**
 * gtk_color_picker_new
 *
 * Creates a new GTK color picker widget. This returns a widget in the form of a small button
 * containing a swatch representing the current selected color. When the button is clicked,
 * a color-selection dialog will open, allowing the user to select a color. The swatch will be
 * updated to reflect the new color when the user finishes.
 *
 * Returns:
 * Pointer to new GTK color picker widget.
 */

GtkWidget *
gtk_color_picker_new (void)
{
	return g_object_new (GTK_TYPE_COLOR_PICKER, NULL);
}

/* Callback used when the color selection dialog is destroyed */
static gboolean
cs_destroy (GtkWidget *widget, gpointer data)
{
	GtkColorPicker *cp;

	cp = GTK_COLOR_PICKER (data);

	cp->_priv->cs_dialog = NULL;

	return FALSE;
}

/* Callback for when the OK button in the color selection dialog is clicked */
static void
cs_ok_clicked (GtkWidget *widget, gpointer data)
{
	GtkColorPicker *cp;
	GtkColorSelection *cs;
	GdkColor color;
	guint16 alpha;
	gushort r, g, b, a;

	cp = GTK_COLOR_PICKER (data);

	cs = GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (cp->_priv->cs_dialog)->colorsel);

	gtk_color_selection_get_current_color (cs, &color);
	alpha = gtk_color_selection_get_current_alpha (cs);

	gtk_widget_destroy (cp->_priv->cs_dialog);

	cp->_priv->r = color.red / 65535.0;
	cp->_priv->g = color.green / 65535.0;
	cp->_priv->b = color.blue / 65535.0;
	cp->_priv->a = cp->_priv->use_alpha ? (alpha / 65535.0) : 1.0;

	render (cp);
	gtk_widget_queue_draw (cp->_priv->drawing_area);

	/* Notify the world that the color was set */

	gtk_color_picker_get_i16 (cp, &r, &g, &b, &a);
	g_signal_emit (cp, color_picker_signals[COLOR_SET], 0,
		       r, g, b, a);
}

static int
key_pressed (GtkWidget *widget, GdkEventKey *event, GtkColorPicker *cp)
{
	if (event->keyval == GDK_Escape){
		gtk_button_clicked (GTK_BUTTON (GTK_COLOR_SELECTION_DIALOG (widget)->cancel_button));
		return 1;
	}
	return 0;
}

static void
gtk_color_picker_clicked (GtkButton *button)
{
	GtkColorPicker *cp;
	GtkColorSelectionDialog *csd;
	GdkColor color;
	guint16 alpha;

	g_return_if_fail (button != NULL);
	g_return_if_fail (GTK_IS_COLOR_PICKER (button));

	cp = GTK_COLOR_PICKER (button);

	/*if dialog already exists, make sure it's shown and raised*/
	if(cp->_priv->cs_dialog) {
		csd = GTK_COLOR_SELECTION_DIALOG (cp->_priv->cs_dialog);
		gtk_widget_show (cp->_priv->cs_dialog);
		if (cp->_priv->cs_dialog->window)
			gdk_window_raise(cp->_priv->cs_dialog->window);
	} else {
		/* Create the dialog and connects its buttons */
                GtkWidget *parent;

                parent = gtk_widget_get_toplevel(GTK_WIDGET(cp));

		cp->_priv->cs_dialog = gtk_color_selection_dialog_new (cp->_priv->title);

                if (parent)
                        gtk_window_set_transient_for(GTK_WINDOW(cp->_priv->cs_dialog),
                                                     GTK_WINDOW(parent));

		csd = GTK_COLOR_SELECTION_DIALOG (cp->_priv->cs_dialog);
		g_signal_connect (cp->_priv->cs_dialog, "destroy",
				  G_CALLBACK (cs_destroy), cp);

		g_signal_connect (cp->_priv->cs_dialog, "key_press_event",
				  G_CALLBACK (key_pressed), cp);
		g_signal_connect (csd->ok_button, "clicked",
				  G_CALLBACK (cs_ok_clicked), cp);

		g_signal_connect_swapped (csd->cancel_button, "clicked",
					  G_CALLBACK (gtk_widget_destroy),
					  cp->_priv->cs_dialog);

		/* FIXME: do something about the help button */

		gtk_window_set_position (GTK_WINDOW (cp->_priv->cs_dialog), GTK_WIN_POS_MOUSE);

		/* If there is a grabed window, set new dialog as modal */
		if (gtk_grab_get_current())
			gtk_window_set_modal(GTK_WINDOW(cp->_priv->cs_dialog),TRUE);
	}
	gtk_color_selection_set_has_opacity_control (GTK_COLOR_SELECTION (csd->colorsel),
						     cp->_priv->use_alpha);

	color.red   = (int) (cp->_priv->r * 65535 + 0.5);
	color.green = (int) (cp->_priv->g * 65535 + 0.5);
	color.blue  = (int) (cp->_priv->b * 65535 + 0.5);
	alpha       = cp->_priv->use_alpha ? (int) (cp->_priv->a * 65535 + 0.5) : 65535;

	gtk_color_selection_set_previous_color (GTK_COLOR_SELECTION (csd->colorsel), &color);
	gtk_color_selection_set_previous_alpha (GTK_COLOR_SELECTION (csd->colorsel), alpha);

	gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (csd->colorsel), &color);
	gtk_color_selection_set_current_alpha (GTK_COLOR_SELECTION (csd->colorsel), alpha);

	gtk_widget_show (cp->_priv->cs_dialog);
}


/**
 * gtk_color_picker_set_d
 * @cp: Pointer to GTK color picker widget.
 * @r: Red color component, values are in [0.0, 1.0]
 * @g: Green color component, values are in [0.0, 1.0]
 * @b: Blue color component, values are in [0.0, 1.0]
 * @a: Alpha component, values are in [0.0, 1.0]
 *
 * Description:
 * Set color shown in the color picker widget using floating point values.
 */

void
gtk_color_picker_set_d (GtkColorPicker *cp, gdouble r, gdouble g, gdouble b, gdouble a)
{
	g_return_if_fail (cp != NULL);
	g_return_if_fail (GTK_IS_COLOR_PICKER (cp));
	g_return_if_fail ((r >=	0.0) &&	(r <= 1.0));
	g_return_if_fail ((g >=	0.0) &&	(g <= 1.0));
	g_return_if_fail ((b >=	0.0) && (b <= 1.0));
	g_return_if_fail ((a >=	0.0) && (a <= 1.0));

	cp->_priv->r = r;
	cp->_priv->g = g;
	cp->_priv->b = b;
	cp->_priv->a = a;

	render (cp);
	gtk_widget_queue_draw (cp->_priv->drawing_area);
}


/**
 * gtk_color_picker_get_d
 * @cp: Pointer to GTK color picker widget.
 * @r: Output location of red color component, values are in [0.0, 1.0]
 * @g: Output location of green color component, values are in [0.0, 1.0]
 * @b: Output location of blue color component, values are in [0.0, 1.0]
 * @a: Output location of alpha color component, values are in [0.0, 1.0]
 *
 * Description:
 * Retrieve color currently selected in the color picker widget in the form of floating point values.
 */

void
gtk_color_picker_get_d (GtkColorPicker *cp, gdouble *r, gdouble *g, gdouble *b, gdouble *a)
{
	g_return_if_fail (cp != NULL);
	g_return_if_fail (GTK_IS_COLOR_PICKER (cp));

	if (r)
		*r = cp->_priv->r;

	if (g)
		*g = cp->_priv->g;

	if (b)
		*b = cp->_priv->b;

	if (a)
		*a = cp->_priv->a;
}


/**
 * gtk_color_picker_set_i8
 * @cp: Pointer to GTK color picker widget.
 * @r: Red color component, values are in [0, 255]
 * @g: Green color component, values are in [0, 255]
 * @b: Blue color component, values are in [0, 255]
 * @a: Alpha component, values are in [0, 255]
 *
 * Description:
 * Set color shown in the color picker widget using 8-bit integer values.
 */

void
gtk_color_picker_set_i8 (GtkColorPicker *cp, guint8 r, guint8 g, guint8 b, guint8 a)
{
	g_return_if_fail (cp != NULL);
	g_return_if_fail (GTK_IS_COLOR_PICKER (cp));
	/* Don't check range of r,g,b,a since it's a 8 bit unsigned type. */

	cp->_priv->r = r / 255.0;
	cp->_priv->g = g / 255.0;
	cp->_priv->b = b / 255.0;
	cp->_priv->a = a / 255.0;

	render (cp);
	gtk_widget_queue_draw (cp->_priv->drawing_area);
}


/**
 * gtk_color_picker_get_i8
 * @cp: Pointer to GTK color picker widget.
 * @r: Output location of red color component, values are in [0, 255]
 * @g: Output location of green color component, values are in [0, 255]
 * @b: Output location of blue color component, values are in [0, 255]
 * @a: Output location of alpha color component, values are in [0, 255]
 *
 * Description:
 * Retrieve color currently selected in the color picker widget in the form of 8-bit integer values.
 */

void
gtk_color_picker_get_i8 (GtkColorPicker *cp, guint8 *r, guint8 *g, guint8 *b, guint8 *a)
{
	g_return_if_fail (cp != NULL);
	g_return_if_fail (GTK_IS_COLOR_PICKER (cp));

	if (r)
		*r = (guint8) (cp->_priv->r * 255.0 + 0.5);

	if (g)
		*g = (guint8) (cp->_priv->g * 255.0 + 0.5);

	if (b)
		*b = (guint8) (cp->_priv->b * 255.0 + 0.5);

	if (a)
		*a = (guint8) (cp->_priv->a * 255.0 + 0.5);
}


/**
 * gtk_color_picker_set_i16
 * @cp: Pointer to GTK color picker widget.
 * @r: Red color component, values are in [0, 65535]
 * @g: Green color component, values are in [0, 65535]
 * @b: Blue color component, values are in [0, 65535]
 * @a: Alpha component, values are in [0, 65535]
 *
 * Description:
 * Set color shown in the color picker widget using 16-bit integer values.
 */

void
gtk_color_picker_set_i16 (GtkColorPicker *cp, gushort r, gushort g, gushort b, gushort a)
{
	g_return_if_fail (cp != NULL);
	g_return_if_fail (GTK_IS_COLOR_PICKER (cp));
	/* Don't check range of r,g,b,a since it's a 16 bit unsigned type. */

	cp->_priv->r = r / 65535.0;
	cp->_priv->g = g / 65535.0;
	cp->_priv->b = b / 65535.0;
	cp->_priv->a = a / 65535.0;

	render (cp);
	gtk_widget_queue_draw (cp->_priv->drawing_area);
}


/**
 * gtk_color_picker_get_i16
 * @cp: Pointer to GTK color picker widget.
 * @r: Output location of red color component, values are in [0, 65535]
 * @g: Output location of green color component, values are in [0, 65535]
 * @b: Output location of blue color component, values are in [0, 65535]
 * @a: Output location of alpha color component, values are in [0, 65535]
 *
 * Description:
 * Retrieve color currently selected in the color picker widget in the form of 16-bit integer values.
 */

void
gtk_color_picker_get_i16 (GtkColorPicker *cp, gushort *r, gushort *g, gushort *b, gushort *a)
{
	g_return_if_fail (cp != NULL);
	g_return_if_fail (GTK_IS_COLOR_PICKER (cp));

	if (r)
		*r = (gushort) (cp->_priv->r * 65535.0 + 0.5);

	if (g)
		*g = (gushort) (cp->_priv->g * 65535.0 + 0.5);

	if (b)
		*b = (gushort) (cp->_priv->b * 65535.0 + 0.5);

	if (a)
		*a = (gushort) (cp->_priv->a * 65535.0 + 0.5);
}


/**
 * gtk_color_picker_set_dither
 * @cp: Pointer to GTK color picker widget.
 * @dither: %TRUE if color sample should be dithered, %FALSE if not.
 *
 * Description:
 * Sets whether the picker should dither the color sample or just paint
 * a solid rectangle.
 */

void
gtk_color_picker_set_dither (GtkColorPicker *cp, gboolean dither)
{
	g_return_if_fail (cp != NULL);
	g_return_if_fail (GTK_IS_COLOR_PICKER (cp));

	cp->_priv->dither = dither ? TRUE : FALSE;

	render (cp);
	gtk_widget_queue_draw (cp->_priv->drawing_area);
}


/**
 * gtk_color_picker_get_dither
 * @cp: Pointer to GTK color picker widget.
 *
 * Description:
 * Does the picker dither the color sample or just paint
 * a solid rectangle.
 *
 * Returns: %TRUE if color sample is dithered, %FALSE if not.
 */

gboolean
gtk_color_picker_get_dither (GtkColorPicker *cp)
{
	g_return_val_if_fail (cp != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_COLOR_PICKER (cp), FALSE);

	return cp->_priv->dither ? TRUE : FALSE;
}


/**
 * gtk_color_picker_set_use_alpha
 * @cp: Pointer to GTK color picker widget.
 * @use_alpha: %TRUE if color sample should use alpha channel, %FALSE if not.
 *
 * Description:
 * Sets whether or not the picker should use the alpha channel.
 */

void
gtk_color_picker_set_use_alpha (GtkColorPicker *cp, gboolean use_alpha)
{
	g_return_if_fail (cp != NULL);
	g_return_if_fail (GTK_IS_COLOR_PICKER (cp));

	cp->_priv->use_alpha = use_alpha ? TRUE : FALSE;

	render (cp);
	gtk_widget_queue_draw (cp->_priv->drawing_area);
}

/**
 * gtk_color_picker_get_use_alpha
 * @cp: Pointer to GTK color picker widget.
 *
 * Description:  Does the picker use the alpha channel?
 *
 * Returns:  %TRUE if color sample uses alpha channel, %FALSE if not.
 */

gboolean
gtk_color_picker_get_use_alpha (GtkColorPicker *cp)
{
	g_return_val_if_fail (cp != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_COLOR_PICKER (cp), FALSE);

	return cp->_priv->use_alpha ? TRUE : FALSE;
}


/**
 * gtk_color_picker_set_title
 * @cp: Pointer to GTK color picker widget.
 * @title: String containing new window title.
 *
 * Description:
 * Sets the title for the color selection dialog.
 */

void
gtk_color_picker_set_title (GtkColorPicker *cp, const gchar *title)
{
	g_return_if_fail (cp != NULL);
	g_return_if_fail (GTK_IS_COLOR_PICKER (cp));
	g_return_if_fail (title != NULL);

	g_free (cp->_priv->title);
	cp->_priv->title = g_strdup (title);

	if (cp->_priv->cs_dialog)
		gtk_window_set_title (GTK_WINDOW (cp->_priv->cs_dialog), cp->_priv->title);
}

/**
 * gtk_color_picker_get_title
 * @cp: Pointer to GTK color picker widget.
 *
 * Description:
 * Gets the title of the color selection dialog.
 *
 * Returns:  An internal string, do not free the return value
 */

const char *
gtk_color_picker_get_title (GtkColorPicker *cp)
{
	g_return_val_if_fail (cp != NULL, NULL);
	g_return_val_if_fail (GTK_IS_COLOR_PICKER (cp), NULL);

	return cp->_priv->title;
}

static void
gtk_color_picker_set_property (GObject            *object,
				 guint               param_id,
				 const GValue       *value,
				 GParamSpec         *pspec)
{
	GtkColorPicker *self;
	gushort r, g, b, a;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_COLOR_PICKER (object));

	self = GTK_COLOR_PICKER (object);

	switch (param_id) {
	case PROP_DITHER:
		gtk_color_picker_set_dither(self, g_value_get_boolean (value));
		break;
	case PROP_USE_ALPHA:
		gtk_color_picker_set_use_alpha(self, g_value_get_boolean (value));
		break;
	case PROP_TITLE:
		gtk_color_picker_set_title(self, g_value_get_string (value));
		break;
	case PROP_RED:
		gtk_color_picker_get_i16(self, &r, &g, &b, &a);
		gtk_color_picker_set_i16(self,
					   g_value_get_uint (value), g, b, a);
		break;
	case PROP_GREEN:
		gtk_color_picker_get_i16(self, &r, &g, &b, &a);
		gtk_color_picker_set_i16(self,
					   r, g_value_get_uint (value), b, a);
		break;
	case PROP_BLUE:
		gtk_color_picker_get_i16(self, &r, &g, &b, &a);
		gtk_color_picker_set_i16(self,
					   r, g, g_value_get_uint (value), a);
		break;
	case PROP_ALPHA:
		gtk_color_picker_get_i16(self, &r, &g, &b, &a);
		gtk_color_picker_set_i16(self,
					   r, g, b, g_value_get_uint (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gtk_color_picker_get_property (GObject            *object,
				 guint               param_id,
				 GValue             *value,
				 GParamSpec         *pspec)
{
	GtkColorPicker *self;
	gushort val;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_COLOR_PICKER (object));

	self = GTK_COLOR_PICKER (object);

	switch (param_id) {
	case PROP_DITHER:
		g_value_set_boolean (value, gtk_color_picker_get_dither(self));
		break;
	case PROP_USE_ALPHA:
		g_value_set_boolean (value, gtk_color_picker_get_use_alpha(self));
		break;
	case PROP_TITLE:
		g_value_set_string (value, self->_priv->title);
		break;
	case PROP_RED:
		gtk_color_picker_get_i16(self, &val, NULL, NULL, NULL);
		g_value_set_uint (value, val);
		break;
	case PROP_GREEN:
		gtk_color_picker_get_i16(self, NULL, &val, NULL, NULL);
		g_value_set_uint (value, val);
		break;
	case PROP_BLUE:
		gtk_color_picker_get_i16(self, NULL, NULL, &val, NULL);
		g_value_set_uint (value, val);
		break;
	case PROP_ALPHA:
		gtk_color_picker_get_i16(self, NULL, NULL, NULL, &val);
		g_value_set_uint (value, val);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}
