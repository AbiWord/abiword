/* AbiWord
 * Copyright (C) 2001 Dom Lachowicz <doml@appligent.com>
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

#include "bz2abw.h"
#include "ut_string.h"
#include "ut_types.h"

#ifdef ENABLE_PLUGINS

/*********************************/
/* General plugin stuff */
/*********************************/

#include "xap_Module.h"

ABI_PLUGIN_DECLARE("BZ2Abw")

// we use a reference-counted sniffer
static IE_Exp_BZ2AbiWord_Sniffer * m_expSniffer = 0;

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
  if (!m_expSniffer)
    {
      m_expSniffer = new IE_Exp_BZ2AbiWord_Sniffer ();
    }
  else
    {
      m_expSniffer->ref();
    }

  mi->name    = "BZ2AbiWord Export Filter";
  mi->desc    = "Export BZ2AbiWord Documents";
  mi->version = ABI_VERSION_STRING;
  mi->author  = "Dom Lachowicz";
  mi->usage   = "No Usage";
  
  IE_Exp::registerExporter (m_expSniffer);
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
  
  UT_ASSERT (m_expSniffer);

  IE_Exp::unregisterExporter (m_expSniffer);
  if (!m_expSniffer->unref())
    {
      m_expSniffer = 0;
    }  

  return 1;
}

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
				 UT_uint32 release)
{
  return 1;
}

#endif //ENABLE_PLUGINS

/*********************************/
/* Export Sniffer */
/*********************************/

bool IE_Exp_BZ2AbiWord_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix,".bzabw") || !UT_stricmp(szSuffix, ".abw.bz2"));
}

UT_Error IE_Exp_BZ2AbiWord_Sniffer::constructExporter(PD_Document * pDocument,
													 IE_Exp ** ppie)
{
	IE_Exp_BZ2AbiWord * p = new IE_Exp_BZ2AbiWord(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_BZ2AbiWord_Sniffer::getDlgLabels(const char ** pszDesc,
											const char ** pszSuffixList,
											IEFileType * ft)
{
	*pszDesc = "BZ2 AbiWord (.bzabw)";
	*pszSuffixList = "*.abw.bz2; *.bzabw";
	*ft = getFileType();
	return true;
}

/*********************************/
/* Exporter */
/*********************************/

IE_Exp_BZ2AbiWord::IE_Exp_BZ2AbiWord (PD_Document * pDocument)
  : IE_Exp_AbiWord_1(pDocument, false), m_fp(0), m_bzout(0)
{
}

IE_Exp_BZ2AbiWord::~IE_Exp_BZ2AbiWord()
{
}

bool IE_Exp_BZ2AbiWord::_openFile(const char * szFilename)
{
    UT_ASSERT(!m_bzout);
    UT_ASSERT(!m_fp);

    int d_error = 0;

    m_fp = fopen(szFilename, "wb");

    if (!m_fp)
      {
	return false;
      }

    // smallest memory block-sizes, non-verbose output, default compression level
    m_bzout = BZ2_bzWriteOpen(&d_error, m_fp, 1, 0, 0);

    if (d_error != BZ_OK)
      {
	return false;
      }

    return (m_bzout != 0);
}

UT_uint32 IE_Exp_BZ2AbiWord::_writeBytes(const UT_Byte * pBytes, UT_uint32 length)
{
    UT_ASSERT(m_fp);
    UT_ASSERT(m_bzout);
    UT_ASSERT(pBytes);
    UT_ASSERT(length);

    int d_error = 0;
    BZ2_bzWrite(&d_error, m_bzout, (void*)pBytes, length);
    
    if(d_error != 0)
      {
	return 0;
      }
    return length;
}

bool IE_Exp_BZ2AbiWord::_closeFile(void)
{
  int d_error = 0;

  if (m_bzout) {
    // no need to get back byte counts... 0=NULL=don't report
    BZ2_bzWriteClose(&d_error, m_bzout, 0, 0, 0);
    m_bzout = 0;
  }
  return true;
}
