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

/*
    Platform: MacOS X.
    Runtime arch.: Mach-O
 */



#include "xap_MacModule.h"
#include "ut_string.h"
#include "ut_assert.h"

XAP_MacModule::XAP_MacModule () :  m_bLoaded(false), m_szname (NULL), m_module (NULL)
{
}

XAP_MacModule::~XAP_MacModule (void)
{
  if(m_szname)
    free (m_szname);
}

bool XAP_MacModule::getModuleName (char ** dest) const
{
    if (m_szname)
    {
        *dest = (char *)UT_strdup (m_szname);
        return true;
    }
    return false;
}

bool XAP_MacModule::load (const char * name)
{
    if (m_bLoaded) {
        return false;
    }

#if 0
    m_module = g_module_open (name, (GModuleFlags)0);
#endif
    UT_ASSERT (UT_NOT_IMPLEMENTED);
    
    if (m_module) {
        m_bLoaded = true;
        return true;
    }
    else {
        return false;
    }
}

bool XAP_MacModule::unload (void)
{
#if 0
  if (m_bLoaded && m_module)
    {
      if (g_module_close (m_module))
	{
	  m_bLoaded = false;
	  return true;
	}
    }
  return false;
#endif
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}

bool XAP_MacModule::resolveSymbol (const char * symbol_name, void ** symbol)
{
#if 0
  return (g_module_symbol (m_module, symbol_name, symbol) ? true : false);
#endif
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}

bool XAP_MacModule::getErrorMsg (char ** dest) const
{
#if 0
    *dest = (char *)UT_strdup (g_module_error ());
    return true;
#endif
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}
