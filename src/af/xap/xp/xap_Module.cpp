/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <string.h>

#include "ut_assert.h"

#include "xap_Module.h"
#include "xap_ModuleManager.h"

/*!
 * Protected constructor
 */
XAP_Module::XAP_Module () :
	m_fnRegister(0),
	m_fnDeregister(0),
	m_fnSupportsVersion(0),
	m_creator (0),
	m_bLoaded(false),
	m_bRegistered(false),
	m_iStatus(0),
	m_szSPI(0)
{
	// zero this out
	memset (&m_info, 0, sizeof (m_info));
}

/*!
 * Protected destructor
 */
XAP_Module::~XAP_Module ()
{
}

/*!
 * marks the module as loaded; returns false if module is already loaded
 */
bool XAP_Module::setSymbols (XAP_Plugin_Registration fnRegister,
							 XAP_Plugin_Registration fnDeregister,
							 XAP_Plugin_VersionCheck fnSupportsVersion)
{
	UT_ASSERT (!m_bLoaded && fnRegister && fnDeregister && fnSupportsVersion);

	if (m_bLoaded || !(fnRegister && fnDeregister && fnSupportsVersion)) return false;

	m_fnRegister = fnRegister;
	m_fnDeregister = fnDeregister;
	m_fnSupportsVersion = fnSupportsVersion;

	m_bLoaded = true;

	return true;
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

	if (!m_bLoaded)     return false;
	if ( m_bRegistered) return false;

	m_bRegistered = true; // i.e., don't try to register again

	typedef int (*plugin_init_func_t) (XAP_ModuleInfo *) ;
	plugin_init_func_t plugin_init_func;
	plugin_init_func_t *pfunc = &plugin_init_func;

	m_iStatus = 0;

	if (m_fnRegister)
		{
			memset (&m_info, 0, sizeof (m_info)); // ensure that this is null
			m_iStatus = m_fnRegister (&m_info);
		}
	else if (resolveSymbol ("abi_plugin_register", reinterpret_cast<void **>(pfunc)))
		{
			if (!plugin_init_func)
				{
					// damn this sucks. probably not an abiword plugin
					return false;
				}
			memset (&m_info, 0, sizeof (m_info)); // ensure that this is null
			m_iStatus = plugin_init_func (&m_info);
		}
	return (m_iStatus ? true : false);
}

/*!
 * Whether the plugin is registered
 *
 * \return true if registered, false otherwise
 */
bool XAP_Module::registered ()
{
	if (!m_bLoaded) return false;

	if (!m_bRegistered) return false;
	return (m_iStatus ? true : false);
}

/*!
 * Before unloading a module, call this function to unregister
 * the plugin, so that it might g_free resources, de-init itself,
 * etc...
 *
 * \return true on success, false on failure
 */
bool XAP_Module::unregisterThySelf ()
{
	UT_ASSERT (m_bLoaded && m_bRegistered);

	bool result = true;

	if (registered ())
		{
			typedef int (*plugin_cleanup_func_t) (XAP_ModuleInfo *);
			plugin_cleanup_func_t plugin_cleanup_func;
			plugin_cleanup_func_t *pfunc = &plugin_cleanup_func;

			if (m_fnDeregister)
				{
					if (m_fnDeregister (&m_info) == 0) result = false;
				}
			else if (resolveSymbol ("abi_plugin_unregister", reinterpret_cast<void **>(pfunc)))
				{
					if (plugin_cleanup_func)
						{
							if (plugin_cleanup_func (&m_info) == 0) result = false;
						}
				}
		}

	// reset this to 0
	memset (&m_info, 0, sizeof (m_info));

	m_bRegistered = false;
	m_iStatus = 0;
	m_szSPI = 0;

	return result;
}

/*!
 * Query if this plugin supports the requested AbiWord version
 *
 * \param major   - "1"
 * \param minor   - "9"
 * \param release - "4"
 * \return true if it supports the requested version, false otherwise
 */
bool XAP_Module::supportsAbiVersion (UT_uint32 major, UT_uint32 minor, UT_uint32 release)
{
	UT_ASSERT (m_bLoaded && m_bRegistered);

	typedef int (*plugin_supports_ver_t) (UT_uint32, UT_uint32, UT_uint32);
	plugin_supports_ver_t plugin_supports_ver;
	plugin_supports_ver_t *pfunc = &plugin_supports_ver;
	int result = 0;

	if (m_fnSupportsVersion)
		{
			result = m_fnSupportsVersion (major, minor, release);
		}
	else if (resolveSymbol ("abi_plugin_supports_version", reinterpret_cast<void **>(pfunc)))
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
