/* AbiWord
 * Copyright (C) 2011-2013 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

// header for gtk2 compatibility

#ifndef _XAP_GTK2COMPAT_H_
#define _XAP_GTK2COMPAT_H_

#include "ut_types.h"

#include <gtk/gtk.h>

#if !GTK_CHECK_VERSION(3,0,0)
#include <gdk/gdkkeysyms.h>
#endif

// keynames
#if GTK_CHECK_VERSION(2,22,0)
#include <gdk/gdkkeysyms-compat.h>
#else
#define GDK_KEY_3270_Enter   GDK_3270_Enter
#define GDK_KEY_BackSpace    GDK_BackSpace
#define GDK_KEY_Delete       GDK_Delete
#define GDK_KEY_Down         GDK_Down
#define GDK_KEY_Escape       GDK_Escape
#define GDK_KEY_F1           GDK_F1
#define GDK_KEY_Help         GDK_Help
#define GDK_KEY_ISO_Enter    GDK_ISO_Enter
#define GDK_KEY_KP_0         GDK_KP_0
#define GDK_KEY_KP_9         GDK_KP_9
#define GDK_KEY_KP_Down      GDK_KP_Down
#define GDK_KEY_KP_Enter     GDK_KP_Enter
#define GDK_KEY_KP_Escape    GDK_KP_Escape
#define GDK_KEY_KP_Left      GDK_KP_Left
#define GDK_KEY_KP_Right     GDK_KP_Right
#define GDK_KEY_KP_Space     GDK_KP_Space
#define GDK_KEY_KP_Up        GDK_KP_Up
#define GDK_KEY_Left         GDK_Left
#define GDK_KEY_Return       GDK_Return
#define GDK_KEY_Right        GDK_Right
#define GDK_KEY_space        GDK_space
#define GDK_KEY_Tab          GDK_Tab
#define GDK_KEY_ISO_Left_Tab GDK_ISO_Left_Tab
#define GDK_KEY_Up           GDK_Up
#define GDK_KEY_VoidSymbol   GDK_VoidSymbol
#endif


// Gtk2 compatibility for GSEAL

#if !GTK_CHECK_VERSION(2,14,0)
inline GdkWindow* gtk_widget_get_window(GtkWidget* w)
{
  return w->window;
}

inline const guchar *gtk_selection_data_get_data(const GtkSelectionData *s)
{
	return s->data;
}

inline GdkAtom gtk_selection_data_get_target(const GtkSelectionData *s)
{
  return s->target;
}

inline gint gtk_selection_data_get_length(const GtkSelectionData *s)
{
  return s->length;
}

inline GtkWidget* gtk_dialog_get_action_area(GtkDialog* dialog)
{
  return dialog->action_area;
}

inline GtkWidget* gtk_dialog_get_content_area(GtkDialog* dialog)
{
  return dialog->vbox;
}

inline GtkWidget* gtk_color_selection_dialog_get_color_selection(GtkColorSelectionDialog* d)
{
  return d->colorsel;
}

inline void gtk_adjustment_configure(GtkAdjustment *adjustment,
				     gdouble value, gdouble lower,
				     gdouble upper, gdouble step_increment,
				     gdouble page_increment, gdouble page_size)
{
  adjustment->value = value;
  adjustment->lower = lower;
  adjustment->upper = upper;
  adjustment->step_increment = step_increment;
  adjustment->page_increment = page_increment;
  adjustment->page_size = page_size;
  g_signal_emit_by_name(G_OBJECT(adjustment), "changed");
}
#endif

#if !GTK_CHECK_VERSION(2,18,0)
inline void gtk_widget_get_allocation(GtkWidget* widget, GtkAllocation* alloc)
{
  *alloc = widget->allocation;
}

inline gboolean gtk_widget_has_focus(GtkWidget* widget)
{
  return GTK_WIDGET_HAS_FOCUS(widget);
}

inline gboolean gtk_widget_has_grab(GtkWidget* widget)
{
  return GTK_WIDGET_HAS_GRAB(widget);
}

inline gboolean gtk_widget_is_toplevel(GtkWidget* widget)
{
  return GTK_WIDGET_TOPLEVEL(widget);
}

inline gboolean gtk_widget_get_double_buffered(GtkWidget* widget)
{
  return GTK_WIDGET_DOUBLE_BUFFERED(widget);
}

inline void gtk_widget_set_can_default(GtkWidget* w, gboolean can_default)
{
  if (can_default) {
    GTK_WIDGET_SET_FLAGS(w, GTK_CAN_DEFAULT);
  }
  else {
    GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_DEFAULT);
  }
}

inline void gtk_widget_set_can_focus(GtkWidget* w, gboolean can_focus)
{
  if (can_focus) {
    GTK_WIDGET_SET_FLAGS(w, GTK_CAN_FOCUS);
  }
  else {
    GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_FOCUS);
  }
}

inline gboolean gtk_widget_get_sensitive(GtkWidget *widget)
{
  return GTK_WIDGET_SENSITIVE(widget);
}

inline gboolean gtk_widget_get_visible(GtkWidget *widget)
{
  return GTK_WIDGET_VISIBLE(widget);
}

inline void gtk_widget_set_visible(GtkWidget *widget, gboolean visible)
{
  visible ? gtk_widget_show(widget) : gtk_widget_hide(widget);
}
#endif

#if !GTK_CHECK_VERSION(2,20,0)
inline void gtk_widget_get_requisition(GtkWidget* widget, GtkRequisition* requisition)
{
  *requisition = widget->requisition;
}

inline gboolean gtk_widget_get_mapped(GtkWidget* w)
{
  return GTK_WIDGET_MAPPED(w);
}

inline gboolean gtk_widget_get_realized(GtkWidget* w)
{
  return GTK_WIDGET_REALIZED(w);
}
#endif

#if !GTK_CHECK_VERSION(2,24,0)

#define GTK_COMBO_BOX_TEXT GTK_COMBO_BOX
#define GtkComboBoxText    GtkComboBox

inline GtkWidget* gtk_combo_box_text_new()
{
  return gtk_combo_box_new_text();
}

inline GtkWidget* gtk_combo_box_text_new_with_entry()
{
  return gtk_combo_box_entry_new_text();
}

inline void gtk_combo_box_text_append_text(GtkComboBox* combo, const gchar* text)
{
  gtk_combo_box_append_text(combo, text);
}

inline gchar* gtk_combo_box_text_get_active_text(GtkComboBox* combo)
{
  return gtk_combo_box_get_active_text(combo);
}

inline void gtk_combo_box_text_remove(GtkComboBox* combo, gint position)
{
  gtk_combo_box_remove_text(combo, position);
}

inline GtkWidget * gtk_combo_box_new_with_model_and_entry(GtkTreeModel *model)
{
  return gtk_combo_box_entry_new_with_model(model,0);
}

inline GdkDisplay * gdk_window_get_display (GdkWindow *window)
{
  g_return_val_if_fail (GDK_IS_WINDOW (window), NULL);
  return gdk_drawable_get_display (GDK_DRAWABLE (window));
}
#endif

#if !GTK_CHECK_VERSION(3,0,0)

// UGLY
typedef GdkColor GdkRGBA;


inline GtkWidget* gtk_color_chooser_dialog_new(const gchar* title,
					       GtkWindow* parent)
{
  GtkWidget* w = gtk_color_selection_dialog_new (title);
  if(parent)
    gtk_window_set_transient_for(GTK_WINDOW(w), parent);
  return w;
}

// in Gtk 3 we use GtkRGBA.
inline void gtk_color_selection_get_current_rgba (GtkColorSelection *colorsel,
                                                  GdkColor *color)
{
  gtk_color_selection_get_current_color(colorsel, color);
}

inline void gtk_color_selection_set_current_rgba (GtkColorSelection *colorsel,
                                                  GdkColor *color)
{
  gtk_color_selection_set_current_color(colorsel, color);
}

inline void gdk_rgba_free(GdkColor* color)
{
  gdk_color_free(color);
}

inline void gtk_color_button_set_rgba(GtkColorButton* button,
				     const GdkColor* color)
{
  gtk_color_button_set_color(button, color);
}

inline void gtk_color_button_get_rgba(GtkColorButton* button,
				     GdkColor* color)
{
  gtk_color_button_get_color(button, color);
}

inline GtkWidget* gtk_box_new(GtkOrientation orientation, gint spacing)
{
  if(orientation == GTK_ORIENTATION_VERTICAL) {
    return gtk_vbox_new(FALSE, spacing);
  }
  return gtk_hbox_new(FALSE, spacing);
}

inline GtkWidget* gtk_scrollbar_new(GtkOrientation orientation, GtkAdjustment* adj)
{
  if(orientation == GTK_ORIENTATION_VERTICAL) {
    return gtk_vscrollbar_new(adj);
  }
  return gtk_hscrollbar_new(adj);
}

inline GtkWidget* gtk_separator_new(GtkOrientation orientation)
{
  if(orientation == GTK_ORIENTATION_VERTICAL) {
    return gtk_vseparator_new();
  }
  return gtk_hseparator_new();
}

inline void gtk_combo_box_text_remove_all(GtkComboBoxText *combo_box)
{
  gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combo_box))));
}

inline void gtk_widget_get_preferred_size(GtkWidget *widget,
					  GtkRequisition *minimum_size,
					  GtkRequisition * /*natural_size*/)
{
	return gtk_widget_size_request(widget, minimum_size);
}

#endif

#if !GTK_CHECK_VERSION(3,4,0)

#define GTK_COLOR_CHOOSER GTK_COLOR_SELECTION

typedef GtkColorSelection GtkColorChooserWidget;
typedef GtkColorSelection GtkColorChooser;

inline GtkWidget* gtk_color_chooser_widget_new()
{
  return gtk_color_selection_new();
}

inline void gtk_color_chooser_set_use_alpha(GtkColorSelection* chooser,
					    gboolean opacity)
{
  gtk_color_selection_set_has_opacity_control(chooser,opacity);
}

inline void gtk_color_chooser_get_rgba(GtkColorSelection *chooser,
				       GdkRGBA *color)
{
  gtk_color_selection_get_current_rgba (chooser, color);
}

inline void gtk_color_chooser_set_rgba(GtkColorSelection *chooser,
				       GdkRGBA *color)
{
  gtk_color_selection_set_current_rgba (chooser, color);
}

#endif

#if GTK_CHECK_VERSION(3,0,0)

typedef GdkWindow GdkDrawable;

#endif


// wrapper because gtk_color_button_set_rgba is deprecated.
inline void XAP_gtk_color_button_set_rgba(GtkColorButton* colorbtn,
				     const GdkRGBA* color)
{
#if GTK_CHECK_VERSION(3,4,0)
  gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(colorbtn), color);
#else
  gtk_color_button_set_rgba (colorbtn, color);
#endif
}

// wrapper because gtk_color_button_get_rgba is deprecated.
inline void XAP_gtk_color_button_get_rgba(GtkColorButton* colorbtn,
					  GdkRGBA* color)
{
#if GTK_CHECK_VERSION(3,4,0)
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(colorbtn), color);
#else
  gtk_color_button_get_rgba (colorbtn, color);
#endif
}


// Now onto the device were the deprecated functions are to be
// with several line of code.
inline
GdkGrabStatus XAP_gdk_keyboard_grab(GdkWindow *window,
				    gboolean owner_events,
				    guint32 time_)
{
#if GTK_CHECK_VERSION(3,0,0)
	GdkDeviceManager *manager
		= gdk_display_get_device_manager(gdk_display_get_default());
	GdkDevice *device = gdk_device_manager_get_client_pointer (manager);

	return gdk_device_grab (gdk_device_get_associated_device(device),
				window,
				GDK_OWNERSHIP_WINDOW,
				owner_events,
				GDK_ALL_EVENTS_MASK,
				NULL,
				time_);
#else
	return gdk_keyboard_grab(window, owner_events, time_);
#endif
}

inline
GdkGrabStatus XAP_gdk_pointer_grab(GdkWindow *window,
				   gboolean owner_events,
				   GdkEventMask event_mask,
				   GdkWindow *confine_to,
				   GdkCursor *cursor,
				   guint32 time_)
{
#if GTK_CHECK_VERSION(3,0,0)
	GdkDeviceManager *manager
		= gdk_display_get_device_manager(gdk_display_get_default());
	GdkDevice *device = gdk_device_manager_get_client_pointer (manager);
	UT_UNUSED(confine_to);
	return gdk_device_grab (device, window,
				GDK_OWNERSHIP_WINDOW,
				owner_events, event_mask,
				cursor, time_);
#else
	return gdk_pointer_grab(window, owner_events, event_mask,
				confine_to, cursor, time_);
#endif
}


// http://permalink.gmane.org/gmane.comp.gnome.svn/520942
inline
void XAP_gdk_keyboard_ungrab(guint32 t)
{
#if GTK_CHECK_VERSION(3,0,0)
	GdkDeviceManager *manager
		= gdk_display_get_device_manager(gdk_display_get_default());
	GdkDevice *device = gdk_device_manager_get_client_pointer (manager);
	gdk_device_ungrab (gdk_device_get_associated_device(device),
			   t);
#else
	gdk_keyboard_ungrab(t);
#endif
}

inline
void XAP_gdk_pointer_ungrab(guint32 t)
{
#if GTK_CHECK_VERSION(3,0,0)
	GdkDeviceManager *manager
		= gdk_display_get_device_manager(gdk_display_get_default());
	GdkDevice *device = gdk_device_manager_get_client_pointer (manager);
	gdk_device_ungrab (device, t);
#else
	gdk_pointer_ungrab(t);
#endif
}

#endif
