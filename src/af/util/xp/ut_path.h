/* AbiSource Program Utilities
 * Copyright (C) 1998-2001 AbiSource, Inc.
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
 * bool UT_isRegularFile(const char* filename);
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 
#ifndef UT_PATH_H
#define UT_PATH_H
#include "ut_types.h"	// for ABI_EXPORT

ABI_EXPORT const char* UT_basename(const char* path);

ABI_EXPORT bool UT_directoryExists(const char* dir);

ABI_EXPORT bool UT_isRegularFile(const char* filename);

#endif /* UT_PATH_H */
