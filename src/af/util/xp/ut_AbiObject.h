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

#ifndef UT_ABIOBJECT_H
#define UT_ABIOBJECT_H

#include <stdlib.h>	// size_t

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

// uncomment this for memory management (experimental)
//#define MANAGE_MEMORY 1

class ABI_EXPORT UT_AbiObject {

 public:

  UT_AbiObject ();
  virtual ~UT_AbiObject ();

  virtual UT_uint32 hashcode () const;

  virtual bool equal (UT_AbiObject * other) const;

  UT_uint32 ref ();
  UT_uint32 unref ();
  UT_uint32 count ();
  void   sink ();

#ifdef MANAGE_MEMORY
  static void * operator new (size_t nbytes);
  static void   operator delete (void * bytes, size_t nbytes);
#endif /* MANAGE_MEMORY */

 private:

  UT_AbiObject (const UT_AbiObject &);   // no impl
  void operator=(const UT_AbiObject &);   // no impl

  UT_uint32 m_refs;
};

#endif /* UT_ABIOBJECT_H */
