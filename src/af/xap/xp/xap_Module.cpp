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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <string.h>

#include "ut_assert.h"
#include "ut_spi.h"

#include "xap_Spider.h"
#include "xap_Module.h"
#include "xap_ModuleManager.h"

/*!
 * Protected constructor
 */
XAP_Module::XAP_Module ()
  : m_spider(0), m_creator (0), m_bLoaded(false), m_bRegistered(false), m_iStatus(0), m_szSPI(0)
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

	if (m_spider)
		if ((m_szSPI = m_spider->add_spi (this)) != 0) return true; // register properly later

	int (*plugin_init_func) (XAP_ModuleInfo *);

	if (resolveSymbol ("abi_plugin_register", (void **)&plugin_init_func))
	{
		if (!plugin_init_func )
		{
			// damn this sucks. probably not an abiword plugin
			return false;
		}

		// assure that this is null
		memset (&m_info, 0, sizeof (m_info));
		m_iStatus = plugin_init_func (&m_info);		
	}

	return (m_iStatus ? true : false);
}

/*!
 * SPI plugins require a further registration step. Call this *after*
 * XAP_Spider::register_spies() - it's safe to call this for modules
 * which aren't spies.
 *
 * \return true on success, false on failure
 */
bool XAP_Module::registerPending ()
{
	if (m_szSPI  == 0) return true;
	if (m_spider == 0) return false; // huh?

	UT_SPI * spi = m_spider->lookup_spi (m_szSPI);
	if (spi == 0) return false;

	m_info.name    = const_cast<char *>(spi->plugin_name ());
	m_info.desc    = const_cast<char *>(spi->plugin_desc ());
	m_info.version = const_cast<char *>(spi->plugin_version ());
	m_info.author  = const_cast<char *>(spi->plugin_author ());
	m_info.usage   = const_cast<char *>(spi->plugin_usage ());

	return true;
}

/*!
 * Whether the plugin is registered
 *
 * \return true if registered, false otherwise
 */
bool XAP_Module::registered ()
{
	if (!m_bLoaded) return false;

	if (m_szSPI)
	{
		if (m_spider == 0) return false; // huh?
		return m_spider->spi_registered (m_szSPI);
	}
	else
	{
		if (!m_bRegistered) return false;
		return (m_iStatus ? true : false);
	}
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

	bool result = true;

	if (registered ())
	{
		int (*plugin_cleanup_func) (XAP_ModuleInfo *);

		if (m_szSPI)
		{
			m_spider->unregister_spi (m_szSPI);
		}
		else if (resolveSymbol ("abi_plugin_unregister", (void **) &plugin_cleanup_func))
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
 * \param major   - "0"
 * \param minor   - "9"
 * \param release - "4"
 * \return true if it supports the requested version, false otherwise
 */
bool XAP_Module::supportsAbiVersion (UT_uint32 major, UT_uint32 minor,
				     UT_uint32 release)
{
	UT_ASSERT (m_bLoaded && m_bRegistered);

	int (*plugin_supports_ver) (UT_uint32, UT_uint32, UT_uint32);
	int result = 0;

	if (resolveSymbol ("abi_plugin_supports_version", 
			   (void **)&plugin_supports_ver))
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
