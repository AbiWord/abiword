/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * goffice-gtk.h - Misc GTK+ utilities
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
#ifndef _GOFFICE_GTK_H_
#define _GOFFICE_GTK_H_

#include <gtk/gtkmessagedialog.h>
#if 0 /* dropped, since abiword doesn't depend on libglade */
#include <glade/glade-xml.h>
#endif
#include <gtk/gtkfilechooser.h>
#include <goffice/app/goffice-app.h>
#include <goffice/utils/go-image.h>

G_BEGIN_DECLS

void	   go_gtk_editable_enters (GtkWindow *window, GtkWidget *w);

#if 0 /* dropped, since abiword doesn't depend on libglade */
GladeXML  *go_libglade_new (char const *gladefile, char const *root,
			    char const *domain, GOCmdContext *cc);
#endif

GdkPixbuf *go_pixbuf_new_from_file	(char const *filename);
GdkPixbuf *go_pixbuf_intelligent_scale	(GdkPixbuf *pixbuf, 
					 guint width, guint height);

int	   go_pango_measure_string	(PangoContext *context,
					 PangoFontDescription const *font_desc,
					 char const *str);

gint       go_gtk_dialog_run		(GtkDialog *dialog, GtkWindow *parent);
GtkWidget *go_gtk_dialog_add_button	(GtkDialog *dialog, char const *text, char const *stock_id,
					 gint response_id);
void       go_gtk_notice_dialog		(GtkWindow *parent, GtkMessageType type, 
					 const gchar *format, ...);
void       go_gtk_notice_nonmodal_dialog (GtkWindow *parent, GtkWidget **ref,
					  GtkMessageType type, const gchar *format, ...);
gboolean   go_gtk_query_yes_no		(GtkWindow *toplevel, gboolean default_answer,
					 const gchar *format, ...);

GtkWidget *go_gtk_button_new_with_stock (char const *text,
					 char const *stock_id);
void	   go_gtk_widget_disable_focus	(GtkWidget *w);
void       go_gtk_window_set_transient  (GtkWindow *parent,   GtkWindow *window);
void	   go_gtk_help_button_init	(GtkWidget *w, char const *data_dir,
					 char const *app, char const *link);
void       go_gtk_nonmodal_dialog	(GtkWindow *toplevel, GtkWindow *dialog);
gboolean   go_gtk_file_sel_dialog	(GtkWindow *toplevel, GtkWidget *w);
char	  *go_gtk_select_image		(GtkWindow *toplevel, const char *initial);
#if 0 /* dropped, since abiword doesn't depend on libglade */
char      *gui_get_image_save_info 	(GtkWindow *toplevel, GSList *supported_formats,
					 GOImageFormat *ret_format, double *dpi);
#endif

gboolean   go_gtk_url_is_writeable	(GtkWindow *parent, char const *url,
					 gboolean overwrite_by_default);

void	   go_atk_setup_label	 	(GtkWidget *label, GtkWidget *target);

void       go_dialog_guess_alternative_button_order (GtkDialog *dialog);

G_END_DECLS

#endif /* _GOFFICE_GTK_H_ */
