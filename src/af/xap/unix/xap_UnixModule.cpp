/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
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

#include "xap_UnixModule.h"
#include "ut_string.h"

XAP_UnixModule::XAP_UnixModule () 
	:  m_bLoaded(false), m_szname (NULL), m_module (NULL)
{
}

XAP_UnixModule::~XAP_UnixModule (void)
{
	FREEP(m_szname);
}

bool XAP_UnixModule::getModuleName (char ** dest) const
{
  if (m_szname)
  {
      *dest = (char *)UT_strdup (m_szname);
      return true;
  }
  return false;
}

bool XAP_UnixModule::load (const char * name)
{
  if (m_bLoaded)
	  return false;
  
  m_module = g_module_open (name, (GModuleFlags)0);
  
  if (m_module)
  {
      m_bLoaded = true;
      return true;
  }
  else
  {
      return false;
  }
}

bool XAP_UnixModule::unload (void)
{
	if (m_bLoaded && m_module)
    {
		if (g_module_close (m_module))
		{
			m_bLoaded = false;
			return true;
		}
    }
	return false;
}

bool XAP_UnixModule::resolveSymbol (const char * symbol_name, void ** symbol)
{
	return (g_module_symbol (m_module, symbol_name, symbol) ? true : false);
}

bool XAP_UnixModule::getErrorMsg (char ** dest) const
{
	*dest = (char *)UT_strdup (g_module_error ());
	return true;
}
