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

XAP_UnixModule::XAP_UnixModule (const char * file_name) : XAP_Module (file_name)
{
  m_module = g_module_open(file_name, (GModuleFlags)0);
  m_szname = g_module_name (m_module);
}

XAP_UnixModule::~XAP_UnixModule (void)
{
  if(m_szname)
    g_free (m_szname);
  g_module_close (m_module);
}

char * XAP_UnixModule::getModuleName (void) const
{
  return m_szname;
}

void XAP_UnixModule::resolveSymbol (const char * symbol_name, void ** symbol)
{
  g_module_symbol (m_module, symbol_name, symbol);
}

void XAP_UnixModule::makeResident (void)
{
  g_module_make_resident (m_module);
}

char * XAP_UnixModule::getErrorMsg (void) const
{
  return g_module_error ();
}
