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

#ifndef XAP_ABIOBJECT_H
#define XAP_ABIOBJECT_H

#include <stdlib.h>	// size_t

// uncomment this for memory management (experimental)
//#define MANAGE_MEMORY 1

class XAP_AbiObject {

 public:

  XAP_AbiObject ();
  virtual ~XAP_AbiObject ();

  virtual size_t hashcode ();

  virtual bool equals (XAP_AbiObject * other);

  size_t ref ();
  size_t unref ();
  size_t count ();
  void   sink ();

#ifdef MANAGE_MEMORY
  static void * operator new (size_t nbytes);
  static void   operator delete (void * bytes, size_t nbytes);
#endif /* MANAGE_MEMORY */

 private:

  XAP_AbiObject (const XAP_AbiObject &);   // no impl
  void operator=(const XAP_AbiObject &);   // no impl

  size_t m_refs;
};

#endif /* XAP_ABIOBJECT_H */
