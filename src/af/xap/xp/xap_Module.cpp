/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001 Dom Lachowicz <cinamod@hotmail.com>
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

#include <string.h>
#include "xap_Module.h"
#include "xap_ModuleManager.h"
#include "ut_assert.h"

/*!
 * Private constructor
 */
XAP_Module::XAP_Module ()
	: m_creator (0), m_bLoaded(false), m_bRegistered(false)
{
	// zero this out
	memset (&m_info, 0, sizeof (m_info));
}

/*!
 * Private destructor
 */
XAP_Module::~XAP_Module ()
{
}

/*!
 * After loading a module, call this function to register
 * the plugin with AbiWord
 *
 * \return true on success, false on failure
 */
bool XAP_Module::registerThySelf ()
{
	UT_ASSERT (m_bLoaded && !m_bRegistered);

	int (*plugin_init_func) (XAP_ModuleInfo *);
	int result = 0;

	if (resolveSymbol ("abi_plugin_register", &(void *)plugin_init_func))
	{
		if (!plugin_init_func )
		{
			// damn this sucks. probably not an abiword plugin
			return false;
		}

		// assure that this is null
		memset (&m_info, 0, sizeof (m_info));
		result = plugin_init_func (&m_info);		
	}

	m_bRegistered = true;
	return (result ? true : false);
}

/*!
 * Before unloading a module, call this function to unregister
 * the plugin, so that it might free resources, de-init itself,
 * etc...
 *
 * \return true on success, false on failure
 */
bool XAP_Module::unregisterThySelf ()
{
	UT_ASSERT (m_bLoaded && m_bRegistered);

	int (*plugin_cleanup_func) (XAP_ModuleInfo *);
	int result = 0;

	if (resolveSymbol ("abi_plugin_unregister", &(void *)plugin_cleanup_func))
	{
		if (!plugin_cleanup_func)
		{
			// damn this sucks. probably not an abiword plugin
			return false;
		}

		result = plugin_cleanup_func (&m_info);
	}

	// reset this to 0
	memset (&m_info, 0, sizeof (m_info));
	m_bRegistered = false;
	return (result ? true : false);
}

/*!
 * Query if this plugin supports the requested AbiWord version
 *
 * \param major   - "0"
 * \param minor   - "7"
 * \param release - "15"
 * \return true if it supports the requested version, false otherwise
 */
bool XAP_Module::supportsAbiVersion (UT_uint32 major, UT_uint32 minor,
									 UT_uint32 release)
{
	UT_ASSERT (m_bLoaded && m_bRegistered);

	int (*plugin_supports_ver) (UT_uint32, UT_uint32, UT_uint32);
	int result = 0;

	if (resolveSymbol ("abi_plugin_supports_version", 
					   &(void *)plugin_supports_ver))
	{
		if (!plugin_supports_ver)
		{
			// damn this sucks. probably not an abiword plugin
			return false;
		}

		result = plugin_supports_ver (major, minor, release);
	}

	return (result ? true : false);
}
