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
#if !defined(_MSC_VER)
#	include <assert.h>
#	define UT_ASSERT assert
#else
// TMN: But the default Microsoft version is not thread-safe, and to add to
// the "coolness" factor, and usability, the following is way cooler.
#	include <stdlib.h>
#	include <crtdbg.h>
#	define UT_ASSERT _ASSERTE
#endif

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
// Please keep the "/**/" to stop MSVC dependency generator complaining.
#		include /**/ "ut_BeOSAssert.h"
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
// Please keep the "/**/" to stop MSVC dependency generator complaining.
#		include /**/ "ut_qnxAssert.h"
#		define UT_ASSERT(expr)			\
				((void) ((expr) ||	\
				(UT_QNXAssertMsg(#expr,\
				 __FILE__, __LINE__),	\
				 0)))
#	endif
/* above only useful on Carbon target if build as Mach-O. CFM use alert and Coco ause UNIX */
#elif (defined(XP_MAC_TARGET_CARBON) && XP_MAC_TARGET_CARBON) && (!defined(CARBON_ON_MACH_O) || (CARBON_ON_MACH_O == 0)) // Carbon on Mach-O as UNIX

#     ifdef NDEBUG
              // When NDEBUG is defined, assert() does nothing.
              // So we let the system header files take care of it.
#             include <assert.h>
#             define UT_ASSERT assert
#   else
              // Otherwise, we want a slighly modified behavior.
              // We'd like assert() to ask us before crashing.
              // We treat asserts as logic flaws, which are sometimes
              // recoverable, but that should be noted.

              // On MacOS this requires toolbox to be initialized. Otherwise, 
              // expect MacBug or a crash if MacBug is not here.

#             include <assert.h>
// Please keep the "/**/" to stop MSVC dependency generator complaining.
#             include /**/ "ut_MacAssert.h"
#             define UT_ASSERT(expr)                  \
                      ((void) ((expr) ||      \
                      (UT_MacAssertMsg(#expr,\
                       __FILE__, __LINE__),   \
                       0)))
#   endif

#else

	// A Unix variant, possibly Gnome.

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
// Please keep the "/**/" to stop MSVC dependency generator complaining.
#		include /**/ "ut_unixAssert.h"
#		define UT_ASSERT(expr)								\
			((void) ((expr) ||								\
				(UT_UnixAssertMsg(#expr,					\
								  __FILE__, __LINE__),		\
				 0)))
#	endif

#endif


/*!
 * This part is not implemented. Same as UT_TODO
 */
#define UT_NOT_IMPLEMENTED		0

/*!
 * This really shouldn't happen
 */
#define UT_SHOULD_NOT_HAPPEN	0

/*!
 * This part is left TODO
 */
#define UT_TODO					0

/*!
 * This line of code should not be reached
 */
#define UT_ASSERT_NOT_REACHED() UT_ASSERT(UT_SHOULD_NOT_HAPPEN)

/*!
 * Trigger a debug assertion, but let the normal flow of code progress
 */
#define UT_ASSERT_HARMLESS(cond) UT_ASSERT(cond)

#ifndef UT_DISABLE_CHECKS

/*!
 * Just return from a function if this condition fails
 */
#define UT_return_if_fail(cond) if (!(cond)) { UT_ASSERT(cond); return; }

/*!
 * Just return this value from a function if this condition fails
 */
#define UT_return_val_if_fail(cond, val) if (!(cond)) { UT_ASSERT(cond); return (val); }

/*!
 * Throw if this condition fails
 */
#define UT_throw_if_fail(cond, val) if (!(cond)) { UT_ASSERT(cond); throw val; }

#else

//disable these checks (NOTE: NOT NDEBUG!!!)

#define UT_return_if_fail(cond)
#define UT_return_val_if_fail(cond, val)
#define UT_throw_if_fail(cond, val)
#endif

#endif /* UT_ASSERT_H */
