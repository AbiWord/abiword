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
#include <image.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "xap_BeOSModule.h"

XAP_BeOSModule::XAP_BeOSModule () : m_module (-1), m_szname (NULL)
{
}

XAP_BeOSModule::~XAP_BeOSModule (void)
{
	FREEP(m_szname);
	unload ();
}

UT_Bool XAP_BeOSModule::unload (void)
{
  if (m_module != -1)
    {
	unload_add_on(m_module);
	m_module = -1;
	return UT_TRUE;
    }
  return UT_FALSE;
}

UT_Bool XAP_BeOSModule::load (const char * file_name)
{
	m_module = load_add_on( file_name );
	m_szname = new char[strlen(file_name) + 1];

	return (m_module != -1 ? UT_TRUE : UT_FALSE);
}

UT_Bool XAP_BeOSModule::getModuleName (char ** dest) const
{
	if (m_szname)
	  {
	    *dest = UT_strdup (m_szname);
	    return UT_TRUE;
	  }
	return UT_FALSE;
}

UT_Bool XAP_BeOSModule::resolveSymbol (const char * symbol_name, void ** symbol)
{
	UT_ASSERT(m_module);
	UT_ASSERT(symbol && symbol_name);
	*symbol = NULL;

	if( m_module < 0 )
		get_image_symbol( m_module , symbol_name , B_SYMBOL_TYPE_TEXT , symbol );

	return (*symbol ? UT_TRUE : UT_FALSE);
}

UT_Bool XAP_BeOSModule::getErrorMsg (char **dest) const
{
	if( m_module > 0)
		*dest = UT_strdup("No Error");
	else
		*dest = UT_strdup ("Argument is not a valid image.");

	return UT_TRUE;
}
