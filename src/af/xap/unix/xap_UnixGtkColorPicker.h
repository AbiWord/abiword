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
/* Color picker button for GTK2
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 * Ported to GTK2 by: Marc Maurer <j.m.maurer@student.utwente.nl>
 */

#ifndef __GTK_COLOR_PICKER_H__
#define __GTK_COLOR_PICKER_H__


#include <gtk/gtkenums.h>
#include <gtk/gtkbutton.h>
#include <gdk-pixbuf/gdk-pixbuf.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* The GtkColorPicker widget is a simple color picker in a button.  The button displays a sample
 * of the currently selected color.  When the user clicks on the button, a color selection dialog
 * pops up.  The color picker emits the "color_changed" signal when the color is set
 *
 * By default, the color picker does dithering when drawing the color sample box.  This can be
 * disabled for cases where it is useful to see the allocated color without dithering.
 */

#define GTK_TYPE_COLOR_PICKER              (gtk_color_picker_get_type ())
#define GTK_COLOR_PICKER(obj)              (GTK_CHECK_CAST ((obj), GTK_TYPE_COLOR_PICKER, GtkColorPicker))
#define GTK_COLOR_PICKER_CLASS(klass)      (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_COLOR_PICKER, GtkColorPickerClass))
#define GTK_IS_COLOR_PICKER(obj)           (GTK_CHECK_TYPE ((obj), GTK_TYPE_COLOR_PICKER))
#define GTK_IS_COLOR_PICKER_CLASS(klass)   (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_COLOR_PICKER))
#define GTK_COLOR_PICKER_GET_CLASS(obj)    (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_COLOR_PICKER, GtkColorPickerClass))


typedef struct _GtkColorPicker        GtkColorPicker;
typedef struct _GtkColorPickerPrivate GtkColorPickerPrivate;
typedef struct _GtkColorPickerClass   GtkColorPickerClass;

struct _GtkColorPicker {
	GtkButton button;

	/*< private >*/
	GtkColorPickerPrivate *_priv;
};

struct _GtkColorPickerClass {
	GtkButtonClass parent_class;

	/* Signal that is emitted when the color is set.  The rgba values
	 * are in the [0, 65535] range.  If you need a different color
	 * format, use the provided functions to get the values from the
	 * color picker.
	 */
        /*  (should be gushort, but Gtk can't marshal that.) */
	void (* color_set) (GtkColorPicker *cp, guint r, guint g, guint b, guint a);

	/* Padding for possible expansion */
	gpointer padding1;
	gpointer padding2;
};


/* Standard Gtk function */
GtkType gtk_color_picker_get_type (void) G_GNUC_CONST;

/* Creates a new color picker widget */
GtkWidget *gtk_color_picker_new (void);

/* Set/get the color in the picker.  Values are in [0.0, 1.0] */
void gtk_color_picker_set_d (GtkColorPicker *cp, gdouble r, gdouble g, gdouble b, gdouble a);
void gtk_color_picker_get_d (GtkColorPicker *cp, gdouble *r, gdouble *g, gdouble *b, gdouble *a);

/* Set/get the color in the picker.  Values are in [0, 255] */
void gtk_color_picker_set_i8 (GtkColorPicker *cp, guint8 r, guint8 g, guint8 b, guint8 a);
void gtk_color_picker_get_i8 (GtkColorPicker *cp, guint8 *r, guint8 *g, guint8 *b, guint8 *a);

/* Set/get the color in the picker.  Values are in [0, 65535] */
void gtk_color_picker_set_i16 (GtkColorPicker *cp, gushort r, gushort g, gushort b, gushort a);
void gtk_color_picker_get_i16 (GtkColorPicker *cp, gushort *r, gushort *g, gushort *b, gushort *a);

/* Sets whether the picker should dither the color sample or just paint a solid rectangle */
void gtk_color_picker_set_dither (GtkColorPicker *cp, gboolean dither);
gboolean gtk_color_picker_get_dither (GtkColorPicker *cp);

/* Sets whether the picker should use the alpha channel or not */
void gtk_color_picker_set_use_alpha (GtkColorPicker *cp, gboolean use_alpha);
gboolean gtk_color_picker_get_use_alpha (GtkColorPicker *cp);

/* Sets the title for the color selection dialog */
void gtk_color_picker_set_title (GtkColorPicker *cp, const gchar *title);
const char * gtk_color_picker_get_title (GtkColorPicker *cp);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_COLOR_PICKER_H__ */
