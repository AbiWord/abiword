/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2002 Marc Maurer (uwog@uwog.net)
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

/* See bug 1764
 * "This product is not manufactured, approved, or supported by 
 * Corel Corporation or Corel Corporation Limited."
 */

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_wordperfect_register
#define abi_plugin_unregister abipgn_wordperfect_unregister
#define abi_plugin_supports_version abipgn_wordperfect_supports_version
#endif

#include "ie_impexp_WordPerfect.h"
#include <gsf/gsf-utils.h>

ABI_PLUGIN_DECLARE("WordPerfect")

static IE_Imp_WordPerfect_Sniffer * m_ImpSniffer = 0;

#ifdef HAVE_LIBWPS
static IE_Imp_MSWorks_Sniffer * m_MSWorks_ImpSniffer = 0;
#endif

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
	if (!m_ImpSniffer)
	{
		m_ImpSniffer = new IE_Imp_WordPerfect_Sniffer ();
	}

	UT_ASSERT (m_ImpSniffer);

#ifdef HAVE_LIBWPS
	if (!m_MSWorks_ImpSniffer)
	{
		m_MSWorks_ImpSniffer = new IE_Imp_MSWorks_Sniffer ();
	}

	UT_ASSERT (m_MSWorks_ImpSniffer);
	IE_Imp::registerImporter (m_MSWorks_ImpSniffer);
#endif

#ifdef HAVE_LIBWPS
	mi->name    = "WordPerfect(tm) and Microsoft Works Importer";
	mi->desc    = "Import WordPerfect(tm) and Microsoft Works Documents";
#else
	mi->name    = "WordPerfect(tm) Importer";
	mi->desc    = "Import WordPerfect(tm) Documents";
#endif
	mi->version = ABI_VERSION_STRING;
	mi->author  = "Marc Maurer, William Lachance";
	mi->usage   = "No Usage";

	IE_Imp::registerImporter (m_ImpSniffer);

	return 1;
}

ABI_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name    = 0;
	mi->desc    = 0;
	mi->version = 0;
	mi->author  = 0;
	mi->usage   = 0;

	UT_ASSERT (m_ImpSniffer);
	UT_ASSERT (m_ExpSniffer);

	IE_Imp::unregisterImporter (m_ImpSniffer);
	delete m_ImpSniffer;
	m_ImpSniffer = 0;
	
#ifdef HAVE_LIBWPS
	IE_Imp::unregisterImporter (m_MSWorks_ImpSniffer);
	delete m_MSWorks_ImpSniffer;
	m_MSWorks_ImpSniffer = 0;
#endif

	return 1;
}

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, 
								 UT_uint32 /*release*/)
{
  return 1;
}
