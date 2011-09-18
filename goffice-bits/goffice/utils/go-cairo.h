/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * go-cairo.c
 *
 * Copyright (C) 2003-2004 Jody Goldberg (jody@gnome.org)
 * Copyright (C) 2000 Eazel, Inc.
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
 *
 * Author: Raph Levien <raph@artofcode.com>
 */

#ifndef GO_CAIRO_H
#define GO_CAIRO_H

#include <cairo.h>
#include <glib.h>
#include <math.h>

G_BEGIN_DECLS

/* This is a workaround for a cairo bug, due to its internal
 * handling of coordinates (16.16 fixed point). */
#define GO_CAIRO_CLAMP(x) CLAMP((x),-15000,15000)
#define GO_CAIRO_CLAMP_SNAP(x,even) GO_CAIRO_CLAMP(even ? floor (x + .5):floor (x) + .5)

void 	 go_cairo_emit_svg_path 		(cairo_t *cr, char const *path);
gboolean go_cairo_surface_is_vector 		(cairo_surface_t const *surface);
void	 go_cairo_convert_data_to_pixbuf 	(unsigned char *dst, unsigned char const *src,
						 int width, int height, int rowstride);
void	 go_cairo_convert_data_from_pixbuf 	(unsigned char *dst, unsigned char const *src,
						 int width, int height, int rowstride);

G_END_DECLS

#endif /* GO_CAIRO_H */
