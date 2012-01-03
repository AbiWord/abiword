/* AbiWord
 * Copyright (C) 2011 Hub Figuiere
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

// header for gtk2 compatibility

#ifndef _XAP_GTK2COMPAT_H_
#define _XAP_GTK2COMPAT_H_

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
#define GDK_KEY_Up           GDK_Up
#define GDK_KEY_VoidSymbol   GDK_VoidSymbol
#endif


// Gtk2 compatibility for GSEAL

#if !GTK_CHECK_VERSION(2,14,0)
inline GdkWindow* gtk_widget_get_window(GtkWidget* w)
{
  return w->window;
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

#endif

#if GTK_CHECK_VERSION(3,0,0)

typedef GdkWindow GdkDrawable;

#endif

#endif
