/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * go-color-selector.h - A color selector
 *
 * Copyright (c) 2006 Emmanuel Pacaud (emmanuel.pacaud@lapp.in2p3.fr)
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

#ifndef GO_COLOR_SELECTOR_H
#define GO_COLOR_SELECTOR_H

#include <goffice/utils/go-color.h>
#include <goffice/gtk/go-selector.h>
#include <gtk/gtkwidget.h>

G_BEGIN_DECLS

GtkWidget *go_color_selector_new	(GOColor initial_color,
					 GOColor default_color,
					 char const *group);
GOColor    go_color_selector_get_color 	(GOSelector *selector, gboolean *is_auto);
gboolean   go_color_selector_set_color  (GOSelector *selector, GOColor color);

G_END_DECLS

#endif /* GO_COLOR_SELECTOR_H */
