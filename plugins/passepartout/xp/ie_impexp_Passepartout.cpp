/* AbiWord passepartout plugin
 * Copyright (C) 2004 David Bolack
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

#include "ie_exp_Passepartout.h"
#include "xap_Module.h"

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_passepartout_register
#define abi_plugin_unregister abipgn_passepartout_unregister
#define abi_plugin_supports_version abipgn_passepartout_supports_version
// dll exports break static linking
#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE("Passepartout")
#endif

#define PLUGIN_NAME "AbiPassepartout::Passepartout"

// we use a reference-counted sniffer
static IE_Exp_Passepartout_Sniffer * m_expSniffer = 0;

ABI_BUILTIN_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
	if (!m_expSniffer)
	{
		m_expSniffer = new IE_Exp_Passepartout_Sniffer (PLUGIN_NAME);
	}

	IE_Exp::registerExporter (m_expSniffer);

	mi->name = "Passepartout Exporter";
	mi->desc = "Export Passepartout's xml2ps format";
	mi->version = ABI_VERSION_STRING;
	mi->author = "David Bolack";
	mi->usage = "No Usage";
	return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;

	IE_Exp::unregisterExporter (m_expSniffer);
	delete m_expSniffer;
	m_expSniffer = 0;

	return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, 
								 UT_uint32 /*release*/)
{
  return 1;
}
