/* AbiSource Application Framework
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

#ifndef XAP_MODULE_H
#define XAP_MODULE_H

// This is an abstract base class meant for dynamically
// Loading and unloading modules. To load a module,
// Pass the intended file name (dll, so, whatever)
// To a valid child instance of this class.

#include "ut_types.h"

class XAP_Module {

public:

   XAP_Module ();

   // Delete (and unload) the module
   virtual ~XAP_Module (void) = 0;

   // load this module into memory. UT_TRUE on success
   virtual UT_Bool load (const char * name) = 0;

   // unload this module from memory. UT_TRUE on success
   virtual UT_Bool unload (void) = 0;

   // passed a symbol name and a void ** symbol, 
   // *symbol refers to the actual representation of @symbol_name
   // i.e. resolveSymbol ("add", &func);
   // int result = func (1, 2);
   virtual UT_Bool resolveSymbol (const char * symbol_name, void ** symbol) =
0;

   // returns the name of this module, if it has one
   // if return is UT_TRUE, you must FREEP dest
   virtual UT_Bool getModuleName (char ** dest) const = 0;

   // returns the most recent error message from one of these
   // calls failing. If return is UT_TRUE, you must FREEP dest
   virtual UT_Bool getErrorMsg (char ** dest) const = 0;
};

#endif /* XAP_MODULE_H */
