/* AbiSource
 * 
 * Copyright (C) 2011 Volodymyr Rudyj <vladimir.rudoy@gmail.com>
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

#include <gsf/gsf-utils.h>
#include "xap_Module.h"
#include "ie_impexp_EPUB.h"
#ifdef _WIN32
#include "ap_Win32Dialog_EpubExportOptions.h"
#endif

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_epub_register
#define abi_plugin_unregister abipgn_epub_unregister
#define abi_plugin_supports_version abipgn_epub_supports_version

#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE("EPUB")
#endif

/*****************************************************************************/
/*****************************************************************************/

static IE_Imp_EPUB_Sniffer * m_imp_sniffer = 0;
static IE_Exp_EPUB_Sniffer * m_exp_sniffer = 0;

ABI_BUILTIN_FAR_CALL
int abi_plugin_register(XAP_ModuleInfo * mi)
{
    if (!m_imp_sniffer)
        m_imp_sniffer = new IE_Imp_EPUB_Sniffer();
    IE_Imp::registerImporter(m_imp_sniffer);

    if (!m_exp_sniffer)
        m_exp_sniffer = new IE_Exp_EPUB_Sniffer();
    IE_Exp::registerExporter(m_exp_sniffer);

#ifdef _WIN32
	AP_Win32Dialog_EpubExportOptions::setInstance((HINSTANCE)s_hModule);
#endif

    mi->name = "EPUB Filter";
    mi->desc = "Import/Export EPUB documents";
    mi->version = ABI_VERSION_STRING;
    mi->author = "Volodymyr Rudyj <vladimir.rudoy@gmail.com>";
    mi->usage = "No Usage";

    return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_unregister(XAP_ModuleInfo * mi)
{
    mi->name = 0;
    mi->desc = 0;
    mi->version = 0;
    mi->author = 0;
    mi->usage = 0;

    IE_Imp::unregisterImporter(m_imp_sniffer);
    delete m_imp_sniffer;
    m_imp_sniffer = 0;

    IE_Exp::unregisterExporter(m_exp_sniffer);
    delete m_exp_sniffer;
    m_exp_sniffer = 0;

    return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_supports_version(UT_uint32 /*major*/, UT_uint32 /*minor*/,
        UT_uint32 /*release*/)
{
    return 1;
}

