/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2001 Dom Lachowicz <doml@appligent.com>
 * Copyright (C) 2001 Hubert Figuiere
 * Copyright (C) 2004 Francis James Franklin
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

#include <glib.h>
#include <gmodule.h>

#include "ut_path.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_CocoaModule.h"
#include "xap_CocoaPlugin.h"

XAP_CocoaModule::XAP_CocoaModule () :
	m_szname("??"),
	m_module_path("??"),
	m_module(0),
	m_cocoa_plugin(0),
	m_bLoaded(false),
	m_bBundle(false),
	m_bCocoa(false)
{
	// 
}

XAP_CocoaModule::~XAP_CocoaModule (void)
{
	if (m_cocoa_plugin)
	{
		XAP_CocoaPlugin * cocoa_plugin = (XAP_CocoaPlugin *) m_cocoa_plugin;
		[cocoa_plugin release];
	}
}

bool XAP_CocoaModule::getModuleName (char ** dest) const
{
	if (dest)
	{
		*dest = (char *) UT_strdup(m_szname.utf8_str());
		return (*dest ? true : false);
	}
	return false;
}

bool XAP_CocoaModule::load (const char * name)
{
	if (m_bLoaded)
		return false;

	if (!name)
		return false;

	m_szname = UT_basename(name);

	m_module_path = name;

	XAP_CocoaPlugin * cocoa_plugin = [[XAP_CocoaPlugin alloc] initWithPath:[NSString stringWithUTF8String:(m_module_path.utf8_str())]];
	if (!cocoa_plugin)
		return false;

	if (m_module_path.byteLength() > 4)
		if (strcmp(m_module_path.utf8_str() + m_module_path.byteLength() - 4, ".Abi") == 0)
		{
			/* This is a bundle-plugin. [TODO]
			 */
			m_bLoaded = false;
			m_bBundle = true;
			m_bCocoa  = true;

			UT_ASSERT(UT_NOT_IMPLEMENTED);
			return false;
		}

	if (m_module_path.byteLength() > 7)
		if (strcmp(m_module_path.utf8_str() + m_module_path.byteLength() - 7, ".so-abi") == 0)
		{
			/* This is an ordinary plugin.
			 */
			m_bLoaded = false;
			m_bBundle = false;
			m_bCocoa  = false; // ??

			if (m_module = g_module_open(name, (GModuleFlags) 0))
			{
				m_bLoaded = true;
				UT_DEBUGMSG(("FJF: plugin loaded: \"%s\"\n", m_module_path.utf8_str()));

				int (*plugin_cocoa_func) (XAP_CocoaPlugin * cocoa_plugin);

				if (resolveSymbol("abi_plugin_cocoa", reinterpret_cast<void **>(&plugin_cocoa_func)))
				{
					m_bCocoa = true;
					plugin_cocoa_func(cocoa_plugin);
					m_cocoa_plugin = (void *) cocoa_plugin;
				}

				UT_UTF8String config_path(m_module_path.utf8_str(), m_module_path.byteLength() - 6);

				config_path += "config";

				if (FILE * config = fopen(config_path.utf8_str(), "r"))
				{
					fclose(config);
					UT_DEBUGMSG(("FJF: plugin config: \"%s\"\n", config_path.utf8_str()));

					int (*plugin_preconfigure_func) (const char * config_path);

					if (resolveSymbol("abi_plugin_preconfigure", reinterpret_cast<void **>(&plugin_preconfigure_func)))
					{
						plugin_preconfigure_func(config_path.utf8_str());
					}

					if (m_bCocoa)
					{
						[cocoa_plugin configure:[NSString stringWithUTF8String:(config_path.utf8_str())]];
					}
				}
			}
#if 0
			else
			{
				fprintf(stderr, "module failed to load: error=\"%s\"\n", g_module_error());
			}
#endif
		}

	if (!m_cocoa_plugin)
		[cocoa_plugin release];

	return m_bLoaded;
}

bool XAP_CocoaModule::unload (void)
{
	if (!m_bLoaded || !m_module)
		return false;

	if (m_bCocoa) // mustn't unload a plugin which has ObjC code
		return false;

	if (m_bBundle)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN); // bundles are Cocoa - see above
		return false;
	}

	if (g_module_close((GModule *) m_module))
	{
		m_bLoaded = false;
		m_module = 0;
		return true;
	}
	return false;
}

bool XAP_CocoaModule::resolveSymbol (const char * symbol_name, void ** symbol)
{
	if (!m_bLoaded || !m_module)
		return false;

	if (!symbol_name || !symbol)
		return false;

	bool bResolved = false;

	if (m_bBundle)
	{
		UT_ASSERT(UT_NOT_IMPLEMENTED);
	}
	else
	{
		bResolved = (g_module_symbol((GModule *) m_module, symbol_name, symbol) ? true : false);
	}
	return bResolved;
}

bool XAP_CocoaModule::getErrorMsg (char ** dest) const
{
	if (!m_bLoaded || !m_module)
		return false;

	if (!dest)
		return false;

	bool bError = false;

	if (m_bBundle)
	{
		UT_ASSERT(UT_NOT_IMPLEMENTED);
	}
	else
	{
		*dest = (char *) UT_strdup(g_module_error());
		bError = (*dest ? true : false);
	}
	return bError;
}
