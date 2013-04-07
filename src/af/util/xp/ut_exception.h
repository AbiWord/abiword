/*
 * AbiSource Program Utilities
 * Copyright (C) 2001 Dom Lachowicz <dominicl@seas.upenn.edu>
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

#ifndef UT_EXCEPTION_H
#define UT_EXCEPTION_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

//
// I really want AbiWord to start using exceptions as soon as is possible
// but there might be platforms/compilers that are brain-dead and don't
// support exceptions as such yet, which really sucks. So this file
// provides a wrapper around standard C++ exception handling and offers
// a do-nothing fall-back on platforms where exceptions aren't handled.
// The idea is that the fall-back code goes away once we verify that every
// platform supports exceptions.
// '/me crosses his fingers'
//
// For starters, this should only initially be used in constructors
// where we can't return an error-code (and thus have our code work
// on both older and newer compilers via a UT_Error code). You can throw
// from inside of a constructor, so we can potentially catch errors there
// and verify which platforms support exceptions with 0 side-effects
// to those platforms where exceptions aren't supported. So those people
// with good compilers are now better off, and those with older, non-compliant
// ones are no worse off than before, and that's not too bad because they
// can't possibly do anything better/else anyway.
//
// -DAL-
//

/*
 * Public base-class which all of our own
 * exceptions should inherit from
 */
class ABI_EXPORT UT_Exception
{
 public:
  UT_Exception () {}
  virtual ~UT_Exception () {}
};

//
// UT_TRY will begin a 'try' block
// UT_CATCH will 'catch' a specific exception type
// UT_END_CATCH evaluates to nothing, basically, but use it
//              after each UT_CATCH under penalty of death
// UT_THROW will throw a new exception
// UT_CATCH_ANY will catch anything (...)
// UT_THROWS will declare a C++ method as throwing an exception
//           usage:
//           UT_THROWS((MyException1, MyException2))
// UT_RETHROW will rethrow a caught exception from within the
//            exception handler
//

#ifdef ABI_DOESNT_SUPPORT_EXCEPTIONS

// d'oh! please list platforms/compilers here which have
// issues with exceptions here for future reference

// MSVC5: has issues with Class::method() throw(ex)

// btw, this is nasty-as-shit for a reason -
// 1) first off, what i'm doing is just plain ugly
// 2) secondly, the preprocessor can't spit-out other preprocessor codes
// so we can't use an #if 0 ... #endif construct, which would be nicer/prettier

#define UT_TRY
#define UT_CATCH(x)     if(false) { \
                         x;
#define UT_END_CATCH    }
#define UT_THROW(x)     (void)0
#define UT_CATCH_ANY    (void)0
#define UT_RETHROW      (void)0

#else

// yay, this platform supports exceptions

#define UT_TRY           try
#define UT_CATCH(x)      catch(x)
#define UT_END_CATCH
#define UT_THROW(x)      throw(x)
#define UT_CATCH_ANY     ...
#define UT_RETHROW       throw

#endif

// yeah, so 99.9999/100 C++ compilers don't handle this properly. shucks.
// #define UT_THROWS(x) throw x
#define UT_THROWS(x)

#endif /* UT_EXCEPTION_H */
