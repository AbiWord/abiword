/* AbiSource Program Utilities
 * Copyright (C) 2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
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
   This file must allow itself to be included multiple times !!!

   This is a bit cumbersome, but since macros cannot expand to
   preprocessing directives, we cannot handle it by macro

   To generate a compiler warning via #pragma you do

   #define UT_WARNING "my message"
   #include "ut_warning.h"

   this will generate message "warning: my message\n"

   I could not find a way to get __FILE__ and __LINE__ into the
   output, as these will always expand to the location of #pragma in
   this file
*/


#ifdef UT_WARNING

#define __UT_WARNING "warning: " UT_WARNING "\n"

#if defined(_WIN32)
#pragma message (__UT_WARNING)

/*
   Add platform-specfic implementations here
*/
#else
#pragma warning __UT_WARNING
#endif

/* now clean up */
#undef UT_WARNING
#undef __UT_WARNING
#endif
