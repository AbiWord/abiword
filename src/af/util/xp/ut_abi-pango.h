/* AbiWord
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#ifndef UT_ABI_PANGO_H
#define UT_ABI_PANGO_H

/*
 * I have been having problems with including the pango/glib files; it seems that
 * the layout created by ./configure, make, make install is different than the
 * installed header files assume. This works for me, but something more protable
 * will be needed later ...
 */

// if you wish to use Pango installed on the system, comment out the following line
#define USE_SYSTEM_PANGO

// this is used internally in Pango headers and without it things do not work
// (I wish the documentation of Pango went beyond the function reference)
#define PANGO_ENABLE_BACKEND

#include <glib-2.0/glib.h>

#ifdef USE_SYSTEM_PANGO
#include <pango/pango.h>
#include <pango/pangoft2.h>
#else
#include "pango/pango.h"
#include "pango/pangoft2.h"
#endif

// some helper functions we use often with glists ...
void UT_free1PangoGlyphString(gpointer data, gpointer /*unused*/);
void UT_free1PangoItem(gpointer data, gpointer /*unused*/);

#endif
