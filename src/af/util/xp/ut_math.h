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
 
#ifndef UTMATH_H
#define UTMATH_H

#include "ut_types.h"

#define UT_E             2.7182818284590452354   /* e */
#define UT_LOG2E         1.4426950408889634074   /* log 2e */
#define UT_LOG10E        0.43429448190325182765  /* log 10e */
#define UT_LN2           0.69314718055994530942  /* log e2 */
#define UT_LN10          2.30258509299404568402  /* log e10 */
#define UT_PI            3.14159265358979323846  /* pi */
#define UT_PI_2          1.57079632679489661923  /* pi/2 */
#define UT_PI_4          0.78539816339744830962  /* pi/4 */
#define UT_1_PI          0.31830988618379067154  /* 1/pi */
#define UT_2_PI          0.63661977236758134308  /* 2/pi */
#define UT_2_SQRTPI      1.12837916709551257390  /* 2/sqrt(pi) */
#define UT_SQRT2         1.41421356237309504880  /* sqrt(2) */
#define UT_SQRT1_2       0.70710678118654752440  /* 1/sqrt(2) */


#ifdef WIN32
#define finite _finite
#endif /* WIN32 */

#endif /* UTMATH_H */
