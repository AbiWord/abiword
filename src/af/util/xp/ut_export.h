/* -*- mode: C; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef _UT_EXPORT_H_
#define _UT_EXPORT_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* This is about exporting symbols...*/

/* This MUST compile with plain C */

#if defined(_WIN32) /* && !defined(__MINGW32__) */
  #define ABI_PLUGIN_EXPORT __declspec(dllexport)
  #ifdef ABI_DLL
     /* we are building an AbiWord plugin and want to use something declared in a library */
     #define ABI_EXPORT __declspec(dllimport)
  #else
     /* we are building AbiWord and wish for its parts to be used by plugins */
     #define ABI_EXPORT __declspec(dllexport)
  #endif
#elif defined (DISABLE_EXPORTS)
  /* ignore DISABLE_EXPORTS until we have assigned all symbols proper
   * visibility */
  #define ABI_PLUGIN_EXPORT
  #define ABI_EXPORT
#else
  #define ABI_PLUGIN_EXPORT
  #define ABI_EXPORT
#endif

#endif
