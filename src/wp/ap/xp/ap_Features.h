/* AbiSource Application Framework
 * Copyright (c) 2004 Hubert Figuiere
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


#ifndef __AP_FEATURES_H__
#define __AP_FEATURES_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xap_Features.h"

/*
   add platforms here as needed

   example (win32)

   #ifdef _WIN32
   #  include "ap_Win32Features.h"
   #endif
*/

#ifdef EMBEDDED_TARGET
#  include "ap_EmbeddedFeatures.h"
#endif


#ifndef XAP_SIMPLE_MENU
#  define XAP_SIMPLE_MENU 0
#endif

#ifndef XAP_SIMPLE_TOOLBAR
#  define XAP_SIMPLE_TOOLBAR 0
#endif

#endif
