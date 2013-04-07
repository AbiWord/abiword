/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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

#ifndef UT_ASSERT_H
#define UT_ASSERT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// TODO move these declarations into platform directories.

#if (defined (WIN32) || defined (_WIN32) || defined (_WIN64))
// The 'cool' win32 assert, at least with VC6, corrups memory (probably calling sprintf on
// a static buffer without checking bounds), so we implement our own assert dialog, which
// is even cooler. TF

#include "ut_types.h"

#ifdef NDEBUG
#  define UT_ASSERT(x)
#else
// This function is implemented in ut_Win32Misc.cpp. It is not declared in any header file (it is
// only to be referenced from here and we want to reduce the files we include here to a
// bare minimum for performance reasons, as this file gets included from pretty much
// everywhere).
extern int ABI_EXPORT UT_Win32ThrowAssert(const char * pCondition, const char * pFile, int iLine, int iCount);

// The best way of stepping into debugger is by generating the appropriate interupt
// instruction, so if we are on Intel x86, we will issue int 3.  If we are not on x86, we
// will use the win32 DebugBreak() function (the disadvantage of that is that the
// execution is interupted not in our code, but in one of the system libs, so you have to
// step out of that system function to get into our code; its rather non-intuitive.

#  undef UT_DEBUG_BREAK

// I am not sure which is the standard GCC macro for the x86 platform; we are supposed to
// be able to get all the predefined macros by running 'cpp -dM' but it would not work on
// my cygwin
#  if defined(__GNUC__) && (defined(_X86) || defined(__i386) || defined(i386))
     // Inline assembly for GCC on x86
#    define UT_DEBUG_BREAK asm("int3");
#  elif defined(__GNUC__) && (defined(__ia64) || defined(ia64))
#    error "This branch has not been tested."
     // On Itanium we use the intrinsic function __break(); defined in ia64intrin.h
     // I am not sure whether we need to tell the compiler this one is intrinsic (MSVC
     // uses a #pragma for this
     void __break(int);
#    define UT_DEBUG_BREAK __break(0x80016);
#  elif defined(_MSC_VER) && defined(_M_IX86)
     // inline assmebly for MSVC on x86
#    define UT_DEBUG_BREAK _asm {int 3}
#  elif defined(_MSC_VER) && (defined(_M_IA64) || defined(_M_AMD64))
#    error "This branch has not been tested."
     // On Itanium we use the intrinsic function __break();
     // I assume this will also work for AMD64, but I am not 100% sure
     void __break(int);
     // this forces __break() to be generated as inline code (see MSDN)
#    pragma intrinsic (__break)
#    define UT_DEBUG_BREAK __break(0x80016);
#  endif

# ifndef UT_DEBUG_BREAK
// Some compiler/architecture for which we do not know how to generate inline assembly to pass
// control to the debugger; we use win32s DebugBreak().
//
// TODO !!! This currently does not work; if someone one day wants to build AW on non-x86
// win32 architecture, they will need to fix this (including <windows.h> from here, which
// is what would work, screws up things in MS Word importer, because the wv library
// redefines some of the win32 structures; but we probably do not want to include
// windows.h from here anyway because of the overhead -- ut_assert.h gets included in
// almost everything.). Really, we want to do something similar as we do for x86.
#  error "Proper implementation needed for this compiler/architecture"

#    ifndef _WINBASE_
       __declspec(dllimport) void __stdcall DebugBreak(void);
#    endif /* ifdef _WINBASE_ */

#    define UT_DEBUG_BREAK DebugBreak();
#  endif /* ifndef UT_DEBUG_BREAK */

// We want to track the number of times we have been through this assert and have the
// option of disabling this assert for the rest of the session; we use the __iCount and
// __bOnceOnly vars for this (this adds a few bytes to the code and footprint, but on
// large scale of things, this is quite negligible for the debug build).
#define UT_ASSERT(x)                                                        \
{                                                                           \
	static bool __bOnceOnly = false;                                        \
	static long __iCount = 0;                                               \
	if(!__bOnceOnly && !(x))                                                \
	{                                                                       \
		__iCount++;                                                         \
		int __iRet = UT_Win32ThrowAssert(#x,__FILE__, __LINE__, __iCount);  \
        if(__iRet == 0)                                                     \
		{                                                                   \
		   UT_DEBUG_BREAK                                                   \
		}                                                                   \
		else if(__iRet < 0)                                                 \
		{                                                                   \
			__bOnceOnly = true;                                             \
		}                                                                   \
	}                                                                       \
}

#endif // ifdef NDEBUG

#else

	// A Unix variant, possibly Gnome.

#	ifdef NDEBUG

		// When NDEBUG is defined, assert() does nothing.
		// So we let the system header files take care of it.
#       if 0 //defined(TOOLKIT_COCOA)
// Please keep the "/**/" to stop MSVC dependency generator complaining.
#			include /**/ "xap_CocoaAssert.h"
#			define UT_ASSERT(expr)								\
			((void) ((expr) ||								\
				(XAP_CocoaAssertMsg(#expr,					\
								  __FILE__, __LINE__),		\
				 0)))

#       else
#			include <assert.h>
#			define UT_ASSERT assert
#		endif
#	else
		// Otherwise, we want a slighly modified behavior.
		// We'd like assert() to ask us before crashing.
		// We treat asserts as logic flaws, which are sometimes
		// recoverable, but that should be noted.

#		include <assert.h>
// Please keep the "/**/" to stop MSVC dependency generator complaining.
#		include /**/ "ut_unixAssert.h"
#		define UT_ASSERT(expr)								\
		{										\
			static bool __bOnceOnly = false;					\
			if (!__bOnceOnly && !(expr))						\
				if (UT_UnixAssertMsg(#expr, __FILE__, __LINE__) == -1)		\
					__bOnceOnly = true;					\
		}
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
#define UT_NOT_REACHED 0

/*!
 * This line of code should not be reached
 */
#define UT_ASSERT_NOT_REACHED() UT_ASSERT(UT_NOT_REACHED)

/*!
 * Trigger a debug assertion, but let the normal flow of code progress
 */
#define UT_ASSERT_HARMLESS(cond) UT_ASSERT(cond)

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

/*!
 * Continue if this condition fails
 */
#define UT_continue_if_fail(cond) if (!(cond)) { UT_ASSERT(cond); continue; }

#endif /* UT_ASSERT_H */
