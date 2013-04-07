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

/*
 * Port to Maemo Development Platform
 * Author: INdT - Renato Araujo <renato.filho@indt.org.br>
 */


#ifndef __XAP_FEATURES_H__
#define __XAP_FEATURES_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef TOOLKIT_COCOA
# include "xap_CocoaFeatures.h"
#elif defined(TOOLKIT_WIN)
# include "xap_Win32Features.h"
#elif defined(TOOLKIT_GTK_ALL)
# include "xap_UnixFeatures.h"
#endif

#if defined(EMBEDDED_TARGET)
# include "xap_EmbeddedFeatures.h"
#endif



/**
	Quartz don't like XOR drawing. So choose an alternate method.
	But since X11 don't like altenate method, we have to keep XOR in the code.

	Platforms: pick up the one you prefer. By defautl XOR since it worked since
 */
#ifndef XAP_DONTUSE_XOR
# define XAP_DONTUSE_XOR 0
#endif

/**
	On MacOS (X), apps can be open without any window so we should allow this.
	This require some heavy changes to the framework. Disabled by default.
 */
#ifndef XAP_SINGLE_XAPAPP
# define XAP_SINGLE_XAPAPP 0
#endif


/**
	Tell if we must not ask before exiting AbiWord (quit)
 */
#ifndef XAP_DONT_CONFIRM_QUIT
# define XAP_DONT_CONFIRM_QUIT 0
#endif


/**
	Define to 1 if you don't inline XPMs into code but load them from disk files.
	MacOS X store them as PNG inside the bundle.

	In the future, this should be 1 for UNIX as well.
 */
#ifndef XAP_DONT_INLINE_XPM
# define XAP_DONT_INLINE_XPM 0
#endif

/**
    Define to 1 if Preferences should be under Tool
    XAP_PREFSMENU_UNDER_TOOLS
*/
#ifndef XAP_PREFSMENU_UNDER_TOOLS
# define XAP_PREFSMENU_UNDER_TOOLS 0
#endif

#endif
