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

#include "ut_types.h"
#include "ut_math.h"

/*!
 * Guaranteed to return a unique new number quickly
 * on each call, for the first 4.2 billion calls
 * or so
 */
UT_uint32 UT_newNumber ()
{
  static UT_uint32 theNumber = 10000;
  //TODO: MUTEX_PROTECT this variable someday soon
  return theNumber++;
}

#if defined(_WIN32) && !defined(__MINGW32__)

double rint(double x) 
{
	double y, z;
	int n;
 
	if(x >= 0) 
	{
		y = x + 0.5;
		z = floor(y);
		n = static_cast<int>(z);
		if (y == z && n % 2) --z;
	} 
	else 
	{
		y = x - 0.5;
		z = ceil(y);
		n = static_cast<int>(z);
		if(y == z && n % 2) ++z;
	}
	return z;
}

#endif
