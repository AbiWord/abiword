/* AbiSource Application Framework
 * Copyright (C) 2001 Dom Lachowicz <doml@appligent.com>
 * Copyright (C) 2001 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>

#include "xap_CocoaModule.h"
#include "ut_string.h"
#include "ut_assert.h"

XAP_CocoaModule::XAP_CocoaModule () 
	:  m_bLoaded(false), m_szname (NULL), m_module (NULL)
{
}

XAP_CocoaModule::~XAP_CocoaModule (void)
{
	FREEP(m_szname);
}

bool XAP_CocoaModule::getModuleName (char ** dest) const
{
  if (m_szname)
  {
      *dest = (char *)UT_strdup (m_szname);
      return true;
  }
  return false;
}

bool XAP_CocoaModule::load (const char * name)
{
  if (m_bLoaded)
	  return false;
  UT_ASSERT (UT_NOT_IMPLEMENTED);
#if 0
  m_module = g_module_open (name, (GModuleFlags)0);
#endif  
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

bool XAP_CocoaModule::unload (void)
{
	if (m_bLoaded && m_module)
    {
		UT_ASSERT (UT_NOT_IMPLEMENTED);
#if 0
		if (g_module_close (m_module))
		{
			m_bLoaded = false;
			return true;
		}
#endif
    }
	return false;
}

bool XAP_CocoaModule::resolveSymbol (const char * symbol_name, void ** symbol)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
#if 0
	return (g_module_symbol (m_module, symbol_name, symbol) ? true : false);
#else
	return false;
#endif
}

bool XAP_CocoaModule::getErrorMsg (char ** dest) const
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
#if 0
	*dest = (char *)UT_strdup (g_module_error ());
#endif
	return true;
}
