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
#include "ut_assert.h"
#include "xap_QNXModule.h"

XAP_QNXModule::XAP_QNXModule (const char * file_name) : XAP_Module (file_name)
{
	m_module = dlopen(file_name, /* Flags? */ 0);
	m_szname = new char[strlen(file_name) + 1];
	m_bresident = UT_FALSE;
}

XAP_QNXModule::~XAP_QNXModule (void)
{
	FREEP(m_szname);
	if (!m_bresident) {
		dlclose(m_module);
	}
	m_module = NULL;
}

char * XAP_QNXModule::getModuleName (void) const
{
	return m_szname;
}

void XAP_QNXModule::resolveSymbol (const char * symbol_name, void ** symbol)
{
	UT_ASSERT(m_module);
	UT_ASSERT(symbol && symbol_name);
	*symbol = NULL;
	if (m_module) {
		*symbol = dlsym(m_module, symbol_name);
	}
}

void XAP_QNXModule::makeResident (void)
{
  /* What does makeResident do? Does it hold it in memory?  */
  m_bresident = UT_TRUE;
}

char * XAP_QNXModule::getErrorMsg (void) const
{
  return dlerror();
}
