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

#include "xap_UnixModule.h"
#include "ut_string.h"

XAP_UnixModule::XAP_UnixModule () : m_module (NULL), m_szname (NULL)
{
}

XAP_UnixModule::~XAP_UnixModule (void)
{
  if(m_szname)
    g_free (m_szname);

  unload ();
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
  m_module = g_module_open (name, (GModuleFlags)0);

  return (m_module ? true : false);
}

bool XAP_UnixModule::unload (void)
{
  if (m_module)
    {
      return (bool)g_module_close (m_module);
    }
  return false;
}

bool XAP_UnixModule::resolveSymbol (const char * symbol_name, void ** symbol)
{
  return (bool) g_module_symbol (m_module, symbol_name, symbol);
}

bool XAP_UnixModule::getErrorMsg (char ** dest) const
{
  *dest = (char *)UT_strdup (g_module_error ());
  return true;
}
