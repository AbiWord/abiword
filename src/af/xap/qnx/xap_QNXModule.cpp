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
#include <dlfcn.h>
#include "ut_types.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_QNXModule.h"

XAP_QNXModule::XAP_QNXModule () : m_module (NULL), m_szname (NULL)
{
}

XAP_QNXModule::~XAP_QNXModule (void)
{
	FREEP(m_szname);
}

bool XAP_QNXModule::load (const char * file_name)
{
	m_module = dlopen(file_name, /* Flags? */ 0);
	m_szname = new char[strlen(file_name) + 1];

	return (m_module ? true : false);
}

bool XAP_QNXModule::unload (void)
{
	if (m_module) {
		dlclose(m_module);
	}
	m_module = NULL;
	return true;
}

bool XAP_QNXModule::getModuleName (char ** dest) const
{
	if (m_szname)
	  {
	    *dest = (char *)UT_strdup (m_szname);
	    return true;
	  }
	return false;
}

bool XAP_QNXModule::resolveSymbol (const char * symbol_name, void ** symbol)
{
	UT_ASSERT(m_module);
	UT_ASSERT(symbol && symbol_name);
	*symbol = NULL;
	if (m_module) {
		*symbol = dlsym(m_module, symbol_name);
	}
	return (*symbol ? true : false);
}

bool XAP_QNXModule::getErrorMsg (char ** dest) const
{
  if (m_module)
    {
      *dest = (char *)UT_strdup (dlerror());
      return true;
    }
  return false;
}
