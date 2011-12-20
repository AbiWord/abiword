


// header for gtk2 compatibility

#ifndef _XAP_GTK2COMPAT_H_
#define _XAP_GTK2COMPAT_H_

#include <gtk/gtk.h>

#if !GTK_CHECK_VERSION(3,0,0)
#include <gdk/gdkkeysyms.h>
#endif

#include <gdk/gdkkeysyms-compat.h>
// keynames 
#if !GTK_CHECK_VERSION(2,22,0)
#define GDK_KEY_Delete       GDK_Delete
#define GDK_KEY_BackSpace    GDK_BackSpace
#define GDK_KEY_Left         GDK_Left
#define GDK_KEY_VoidSymbol   GDK_VoidSymbol
#define GDK_KEY_Right        GDK_Right
#define GDK_KEY_KP_0         GDK_KP_0 
#define GDK_KEY_KP_9         GDK_KP_9
#define GDK_KEY_KP_Enter     GDK_KP_Enter
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

inline void gtk_widget_set_can_focus(GtkWidget* w, gboolean can_focus)
{
  GTK_WIDGET_SET_FLAGS(w, GTK_CAN_FOCUS);
}

inline void gtk_widget_set_visible(GtkWidget *widget, gboolean visible)
{
  visible ? gtk_widget_show(widget) : gtk_widget_hide(widget);
}
#endif

#if !GTK_CHECK_VERSION(2,20,0)
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
