/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#include "ut_string.h"
#include "ie_exp_AWT.h"

/*****************************************************************/
/*****************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

#define SUPPORTS_ABI_VERSION(a,b,c) (((a==0)&&(b==7)&&(c==15)) ? 1 : 0)

// we use a reference-counted sniffer
static IE_Exp_AWT_Sniffer * m_sniffer = 0;

ABI_FAR extern "C"
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Exp_AWT_Sniffer ();
	}
	else
	{
		m_sniffer->ref();
	}

	mi->name = "AbiWord Template Exporter";
	mi->desc = "Export AbiWord Documents as Templates";
	mi->version = "0.7.15";
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Exp::registerExporter (m_sniffer);
	return 1;
}

ABI_FAR extern "C"
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;

	UT_ASSERT (m_sniffer);

	IE_Exp::unregisterExporter (m_sniffer);
	if (!m_sniffer->unref())
	{
		m_sniffer = 0;
	}

	return 1;
}

ABI_FAR extern "C"
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
								 UT_uint32 release)
{
	return SUPPORTS_ABI_VERSION(major, minor, release);
}

#endif

/*****************************************************************/
/*****************************************************************/

bool IE_Exp_AWT_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix,".awt"));
}

UT_Error IE_Exp_AWT_Sniffer::constructExporter(PD_Document * pDocument,
													 IE_Exp ** ppie)
{
	IE_Exp_AWT * p = new IE_Exp_AWT(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_AWT_Sniffer::getDlgLabels(const char ** pszDesc,
											const char ** pszSuffixList,
											IEFileType * ft)
{
	*pszDesc = "AbiWord Template (.awt)";
	*pszSuffixList = "*.awt";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_AWT::IE_Exp_AWT (PD_Document * pDocument)
	: IE_Exp_AbiWord_1 (pDocument, true)
{
}

IE_Exp_AWT::~IE_Exp_AWT()
{
}

