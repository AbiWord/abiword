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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 
#ifndef UT_ASSERT_H
#define UT_ASSERT_H

// TODO move these declarations into platform directories.

#ifdef WIN32

	// Win32 assert() is cool, so we use it as is.

#	include <assert.h>
#	define UT_ASSERT assert

#elif defined(__BEOS__)
	// A BeOS variant.
#	ifdef NDEBUG
		// When NDEBUG is defined, assert() does nothing.
		// So we let the system header files take care of it.
#		include <assert.h>
#		define UT_ASSERT assert
#	else
		// Otherwise, we want a slighly modified behavior.
		// We'd like assert() to ask us before crashing.
		// We treat asserts as logic flaws, which are sometimes
		// recoverable, but that should be noted.
#		include <assert.h>
#		include "ut_BeOSAssert.h"
#		define UT_ASSERT(expr)			\
				((void) ((expr) ||	\
				(UT_BeOSAssertMsg(#expr,\
				 __FILE__, __LINE__),	\
				 0)))
#	endif

#elif defined(__QNX__)
	// A QNX variant.
#	ifdef NDEBUG
		// When NDEBUG is defined, assert() does nothing.
		// So we let the system header files take care of it.
#		include <assert.h>
#		define UT_ASSERT assert
#	else
		// Otherwise, we want a slighly modified behavior.
		// We'd like assert() to ask us before crashing.
		// We treat asserts as logic flaws, which are sometimes
		// recoverable, but that should be noted.
#		include <assert.h>
#		include "ut_qnxAssert.h"
#		define UT_ASSERT(expr)			\
				((void) ((expr) ||	\
				(UT_QNXAssertMsg(#expr,\
				 __FILE__, __LINE__),	\
				 0)))
#	endif

#elif defined(TARGET_OS_MAC) && TARGET_OS_MAC // ?SBK

	// MAC assert() is not cool, but I will fix it later.

#	include <assert.h>
#	define UT_ASSERT assert

#elif defined(HAVE_GNOME)
#ifdef NDEBUG
#include <assert.h>
#define UT_ASSERT assert
#else
#include <glib.h>
#define UT_ASSERT g_assert
#endif
#else

	// A Unix variant.

#	ifdef NDEBUG

		// When NDEBUG is defined, assert() does nothing.
		// So we let the system header files take care of it.

#		include <assert.h>
#		define UT_ASSERT assert

#	else
		// Otherwise, we want a slighly modified behavior.
		// We'd like assert() to ask us before crashing.
		// We treat asserts as logic flaws, which are sometimes
		// recoverable, but that should be noted.

#		include <assert.h>
#		include "ut_unixAssert.h"
#		define UT_ASSERT(expr)								\
			((void) ((expr) ||								\
				(UT_UnixAssertMsg(#expr,					\
								  __FILE__, __LINE__),		\
				 0)))
#	endif

#endif


#define UT_NOT_IMPLEMENTED		0
#define UT_SHOULD_NOT_HAPPEN	0
#define UT_TODO					0

#endif /* UT_ASSERT_H */
