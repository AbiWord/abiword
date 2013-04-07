/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2000, 2001 Frodo Looijaard <frodol@dds.nl>
 * Copyright (C) 2002 Dom Lachowicz
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

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_psion_register
#define abi_plugin_unregister abipgn_psion_unregister
#define abi_plugin_supports_version abipgn_psion_supports_version
#endif

#include "ie_imp_Psion.h"
#include "ie_exp_Psion.h"
#include "ie_impexp_Psion.h"
#include "xap_Module.h"
#include "ut_debugmsg.h"

#include <psiconv/data.h>
#include <psiconv/error.h>
#include <psiconv/unicode.h>

ABI_PLUGIN_DECLARE("Psion")

#define PLUGIN_WORD "AbiPsion::Psion (Word)"
#define PLUGIN_TEXT "AbiPsion::Psion (Text)"

// we use a reference-counted sniffer
static IE_Exp_Psion_Word_Sniffer * m_expword_sniffer = 0;
static IE_Exp_Psion_TextEd_Sniffer * m_exptexted_sniffer = 0;
static IE_Imp_Psion_Word_Sniffer * m_impword_sniffer = 0;
static IE_Imp_Psion_TextEd_Sniffer * m_imptexted_sniffer = 0;

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_expword_sniffer && !m_exptexted_sniffer)
	{
		m_expword_sniffer = new IE_Exp_Psion_Word_Sniffer (PLUGIN_WORD);
		m_exptexted_sniffer = new IE_Exp_Psion_TextEd_Sniffer (PLUGIN_TEXT);
	}

	if (!m_impword_sniffer && !m_imptexted_sniffer)
	{
		m_impword_sniffer = new IE_Imp_Psion_Word_Sniffer (PLUGIN_WORD);
		m_imptexted_sniffer = new IE_Imp_Psion_TextEd_Sniffer (PLUGIN_TEXT);
	}

	mi->name = "Psion Import/Export";
	mi->desc = "Read and Write Psion Documents";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Exp::registerExporter (m_expword_sniffer);
	IE_Exp::registerExporter (m_exptexted_sniffer);

	IE_Imp::registerImporter (m_impword_sniffer);
	IE_Imp::registerImporter (m_imptexted_sniffer);

	return 1;
}

ABI_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;

	UT_ASSERT (m_expword_sniffer && m_exptexted_sniffer);

	IE_Exp::unregisterExporter (m_expword_sniffer);
	delete m_expword_sniffer;
	m_expword_sniffer = 0;

	IE_Exp::unregisterExporter (m_exptexted_sniffer);
	delete m_exptexted_sniffer;
	m_exptexted_sniffer = 0;

	UT_ASSERT (m_impword_sniffer && m_imptexted_sniffer);

	IE_Imp::unregisterImporter (m_impword_sniffer);
	delete m_impword_sniffer;
	m_impword_sniffer = 0;

	IE_Imp::unregisterImporter (m_imptexted_sniffer);
	delete m_imptexted_sniffer;
	m_imptexted_sniffer = 0;

	return 1;
}

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, 
								 UT_uint32 /*release*/)
{
  return 1;
}


// The error-handler we will use when calling the psiconv library
void psion_error_handler (int kind, psiconv_u32 /*off*/, const char *message)
{
	switch(kind) {
		case PSICONV_VERB_FATAL:
		case PSICONV_VERB_ERROR:
			UT_WARNINGMSG(("%s\n",message));
			break;
                                                                                
		//case PSICONV_VERB_DEBUG:
		//case PSICONV_VERB_WARN:
		//case PSICONV_VERB_PROGRESS:
		default:
			UT_DEBUGMSG(("PSION: %s\n",message));
	}
}

