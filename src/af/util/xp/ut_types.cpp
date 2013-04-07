/* AbiSource Program Utilities
 * Copyright (C) 2001 Dom Lachowicz
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

#include <errno.h>
#include "ut_types.h"

/*!
 * Convert errno to a UT_Error code
 *
 */
UT_Error UT_errnoToUTError (void)
{
  switch (errno)
    {
    case EACCES:
      return UT_IE_PROTECTED;

    case ENOENT:
      return UT_IE_FILENOTFOUND;
      
    case ENOMEM:
      return UT_OUTOFMEM;

    case EMFILE:
      return UT_IE_COULDNOTOPEN;

    case EROFS:
    case ENOSPC:
      return UT_IE_COULDNOTWRITE;

      /**
       * Valid  error  numbers are all non-zero; errno is never set
       * to zero by any library  function.   All  the  error  names
       * specified by POSIX.1 must have distinct values.
       */
    case 0:
      return UT_OK;

    case EINVAL:
    default: // generic case
      return UT_ERROR;
    }
}
