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

#ifndef XAP_QNXMODULE_H
#define XAP_QNXMODULE_H

#include "xap_Module.h"

class XAP_QNXModule : public XAP_Module 
{

public:

   XAP_QNXModule (const char * file_name) ;
   virtual ~XAP_QNXModule (void);

   virtual void   resolveSymbol (const char * symbol_name, void ** symbol);
   virtual void   makeResident (void);
   virtual char * getModuleName (void) const;
   virtual char * getErrorMsg (void) const;

 private:
	char * m_szname;
	void * m_module;
	UT_Bool   m_bresident;
};

#endif /* XAP_QNXMODULE_H */
