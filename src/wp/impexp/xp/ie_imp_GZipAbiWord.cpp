/* AbiWord
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

#include <zlib.h>
#include "ie_imp_GZipAbiWord.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_types.h"

/*****************************************************************/
/*****************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

#define SUPPORTS_ABI_VERSION(a,b,c) (((a==0)&&(b==7)&&(c==15)) ? 1 : 0)

// we use a reference-counted sniffer
static IE_Imp_GZipAbiWord_Sniffer * m_sniffer = 0;
static UT_sint32 m_refs = 0;

ABI_FAR extern "C"
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_refs && !m_sniffer)
	{
		m_sniffer = new IE_Imp_GZipAbiWord_Sniffer ();
		m_refs++;
	}
	else if (m_refs && m_sniffer)
	{
		m_refs++;
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	mi->name = "GZipAbiWord Importer";
	mi->desc = "Import GZipAbiWord Documents";
	mi->version = "0.7.15";
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Imp::registerImporter (m_sniffer);
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

	UT_ASSERT (m_refs && m_sniffer);

	m_refs--;
	IE_Imp::unregisterImporter (m_sniffer);
	if (!m_refs)
	{
		delete m_sniffer;
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

bool IE_Imp_GZipAbiWord_Sniffer::recognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	// TODO: This is a hack.  Since we're just passed in some
	// TODO: some data, and not the actual filename, there isn't
	// TODO: much we can do other than verify that it is gzip'ed
	// TODO: data.  For the time being, assume that if it is
	// TODO: gzip'ed, it's gzip'ed abiword.  This assumption will
	// TODO: be false if and when we support any other compressed
	// TODO: formats.
	if ( iNumbytes < 2 ) return(false);
	if ( ( szBuf[0] == (char)0x1f ) && ( szBuf[1] == (char)0x8b ) )
	{
		return(true);
	}
	return(false);
}

bool IE_Imp_GZipAbiWord_Sniffer::recognizeSuffix(const char * szSuffix)
{
    return (!UT_stricmp(szSuffix,".zabw") || !UT_stricmp(szSuffix,".abw.gz"));
}

UT_Error IE_Imp_GZipAbiWord_Sniffer::constructImporter(PD_Document * pDocument,
													   IE_Imp ** ppie)
{
    *ppie = new IE_Imp_GZipAbiWord(pDocument);;
    return UT_OK;
}

bool IE_Imp_GZipAbiWord_Sniffer::getDlgLabels(const char ** pszDesc,
											  const char ** pszSuffixList,
											  IEFileType * ft)
{
    *pszDesc = "GZipped AbiWord (.zabw)";
    *pszSuffixList = "*.zabw; *.abw.gz";
    *ft = getFileType();
    return true;
}

/*****************************************************************/
/*****************************************************************/

bool IE_Imp_GZipAbiWord::_openFile(const char * szFilename) 
{
    m_gzfp = gzopen(szFilename, "rb");
    return (m_gzfp != NULL);
}

UT_uint32 IE_Imp_GZipAbiWord::_readBytes(char * buf, UT_uint32 length) 
{
    return gzread(m_gzfp, buf, length);
}

void IE_Imp_GZipAbiWord::_closeFile(void) 
{
    if (m_gzfp) {
	gzclose(m_gzfp);
    }
}

IE_Imp_GZipAbiWord::~IE_Imp_GZipAbiWord()
{
}

IE_Imp_GZipAbiWord::IE_Imp_GZipAbiWord(PD_Document * pDocument)
	: IE_Imp_AbiWord_1(pDocument)
{
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void IE_Imp_GZipAbiWord::pasteFromBuffer(PD_DocumentRange * /*pDocRange*/,
										 unsigned char * /*pData*/, UT_uint32 /*lenData*/)
{
    UT_ASSERT(UT_NOT_IMPLEMENTED);
}
