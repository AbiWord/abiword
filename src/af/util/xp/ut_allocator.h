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

#ifndef UT_ALLOCATOR
#define UT_ALLOCATOR

/*!
 * This class is responsible for creating and destroying memory buffers
 * It provides a default implementation based on g_try_malloc/g_free
 */
class ABI_EXPORT UT_Allocator
{
 public:
  UT_Allocator ();
  virtual ~UT_Allocator ();

  virtual void * allocate (size_t nbytes);
  virtual void deallocate (void * pointer);

 private:
  UT_Allocator (const UT_Allocator &); // no impl
  UT_Allocator& operator=(const UT_Allocator &); // no impl
};

/*!
 * Returns zeroed memory, either via g_try_malloc&memset or UT_calloc
 */
class ABI_EXPORT UT_NullAllocator : public UT_Allocator
{
 public:
  UT_NullAllocator ();
  virtual ~UT_NullAllocator();

  virtual void * allocate (size_t nbytes);

 private:
  UT_NullAllocator (const UT_NullAllocator &); // no impl
  UT_NullAllocator& operator=(const UT_NullAllocator &); // no impl
};

#endif /* UT_ALLOCATOR */
