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

XAP_BeOSModule::XAP_BeOSModule (const char * file_name) : XAP_Module (file_name)
{
	m_module = load_add_on( file_name );
	m_szname = new char[strlen(file_name) + 1];
	m_bresident = UT_FALSE;
}

XAP_BeOSModule::~XAP_BeOSModule (void)
{
	FREEP(m_szname);
	if (!m_bresident) {
		unload_add_on(m_module);
	}
	m_module = -1;
}

char * XAP_BeOSModule::getModuleName (void) const
{
	return m_szname;
}

void XAP_BeOSModule::resolveSymbol (const char * symbol_name, void ** symbol)
{
	UT_ASSERT(m_module);
	UT_ASSERT(symbol && symbol_name);
	*symbol = NULL;

	if( m_module < 0 )
		get_image_symbol( m_module , symbol_name , B_SYMBOL_TYPE_TEXT , symbol );
}

void XAP_BeOSModule::makeResident (void)
{
  /* What does makeResident do? Does it hold it in memory?  */
  m_bresident = UT_TRUE;
}

char * XAP_BeOSModule::getErrorMsg (void) const
{
	if( m_module > 0)
		return "No Error"; // Is this the right way to return this?
	else
		return "Argument is not a valid image.";
}
