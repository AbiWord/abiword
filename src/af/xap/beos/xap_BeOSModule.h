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

#ifndef XAP_BEOSMODULE_H
#define XAP_BEOSMODULE_H

#include "xap_Module.h"
#include <image.h>

class XAP_BeOSModule : public XAP_Module 
{

public:

   XAP_BeOSModule (const char * file_name) ;
   virtual ~XAP_BeOSModule (void);

   virtual void   resolveSymbol (const char * symbol_name, void ** symbol);
   virtual void   makeResident (void);
   virtual char * getModuleName (void) const;
   virtual char * getErrorMsg (void) const;

 private:
	char * m_szname;
	image_id m_module;//void * m_module;
	UT_Bool   m_bresident;
};

#endif /* XAP_BEOSMODULE_H */
