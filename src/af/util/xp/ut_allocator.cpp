/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001 Dom Lachowicz <cinamod@hotmail.com>
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

#include "ut_types.h"
#include "ut_allocator.h"

UT_Allocator::UT_Allocator ()
{
}

UT_Allocator::~UT_Allocator ()
{
}

void * UT_Allocator::allocate (size_t nbytes)
{
  if (0 == nbytes) // be safe & not g_try_malloc a 0-byte block
    return NULL;
  return static_cast<void *>(g_try_malloc(nbytes));
}

void UT_Allocator::deallocate (void * pointer)
{
  if (NULL == pointer) // be sure not to deallocate a null block
    return;
  g_free(pointer);
}

UT_NullAllocator::UT_NullAllocator ()
  : UT_Allocator ()
{
}

UT_NullAllocator::~UT_NullAllocator()
{
}

void * UT_NullAllocator::allocate (size_t nbytes)
{
  if (0 == nbytes) // make sure not to allocate a 0 byte block
    return NULL;

#ifdef DOESNT_HAVE_CALLOC
  void * ptr = static_cast<void *>(g_try_malloc(nbytes));
  memset (ptr, 0, nbytes);
  return ptr;
#else
  return UT_calloc(1, nbytes);
#endif
}
