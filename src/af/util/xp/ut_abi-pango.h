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

/* I have been having real problems with the Pango include files
 * just including pangof2.h or pango.h does not include glib.h properly
 * leaving undefined macros, etc. in the sources; this file includes
 * everything necessary
 */

#ifndef UT_ABI_PANGO_H
#define UT_ABI_PANGO_H

/*
   I have been having problems with including the pango/glib files; it seems that the layout created by
   ./configure, make, make install is different than the installed header files assume. This works for
   me, but something more protable will be needed later ...
*/

/*
   this is used internally by Pango and without it things do not work (I wish the documentation
   of Pango was bit better)
*/
#define PANGO_ENABLE_BACKEND
#include <glib-2.0/glib.h>
#include <pango/pango.h>
#include <pango/pangoft2.h>
#endif
