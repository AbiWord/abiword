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

#include "ie_imp_BZ2Abw.h"
#include "ut_string.h"
#include "ut_types.h"

#ifdef ENABLE_PLUGINS

#include "xap_Module.h"

/*********************************/
/* General plugin stuff */
/*********************************/

ABI_PLUGIN_DECLARE("BZ2Abw")

// we use a reference-counted sniffer
static IE_Imp_BZ2AbiWord_Sniffer * m_impSniffer = 0;

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
  if (!m_impSniffer)
    {
      m_impSniffer = new IE_Imp_BZ2AbiWord_Sniffer ();
    }
  else
    {
      m_impSniffer->ref();
    }
  
  mi->name    = "BZ2AbiWord Import Filter";
  mi->desc    = "Import BZ2AbiWord Documents";
  mi->version = ABI_VERSION_STRING;
  mi->author  = "Dom Lachowicz";
  mi->usage   = "No Usage";
  
  IE_Imp::registerImporter (m_impSniffer);
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
  
  UT_ASSERT (m_impSniffer);

  IE_Imp::unregisterImporter (m_impSniffer);
  if (!m_impSniffer->unref())
    {
      m_impSniffer = 0;
    }

  return 1;
}

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
				 UT_uint32 release)
{
  return 1;
}

#endif //enable plugins

/*********************************/
/* Import Sniffer */
/*********************************/

bool IE_Imp_BZ2AbiWord_Sniffer::recognizeContents (const char * szBuf, 
						   UT_uint32 iNumbytes)
{
  if (!strncmp(szBuf, "BZh", 3)) // i think that this is the correct BZ2 header
       return true;
  return false;
}

bool IE_Imp_BZ2AbiWord_Sniffer::recognizeSuffix (const char * szSuffix)
{
  return (!UT_stricmp(szSuffix,".bzabw") || !UT_stricmp(szSuffix,".abw.bz2"));
}

bool IE_Imp_BZ2AbiWord_Sniffer::getDlgLabels (const char ** szDesc,
					      const char ** szSuffixList,
					      IEFileType * ft)
{
    *szDesc = "BZipped AbiWord (.bzabw)";
    *szSuffixList = "*.bzabw; *.abw.bz2";
    *ft = getFileType();
    return true;
}

UT_Error IE_Imp_BZ2AbiWord_Sniffer::constructImporter (PD_Document * pDocument,
						       IE_Imp ** ppie)
{
  *ppie = new IE_Imp_BZ2AbiWord(pDocument);;
  return UT_OK;
}

/*********************************/
/* Importer */
/*********************************/

bool IE_Imp_BZ2AbiWord::openFile (const char * szFilename) 
{
  UT_ASSERT (m_bzin == 0);

  m_fp = fopen (szFilename, "rb");
  if (!m_fp) return false;

  int d_error = 0;
  m_bzin = BZ2_bzReadOpen (&d_error, m_fp, 0, 0, NULL, 0);
  if (d_error != BZ_OK) return false;
  return (m_bzin != NULL);
}

UT_uint32 IE_Imp_BZ2AbiWord::readBytes (char * buffer, UT_uint32 length) 
{
  UT_ASSERT (m_bzin);

  int d_error = 0;
  return BZ2_bzRead (&d_error, m_bzin, buffer, length);
}

void IE_Imp_BZ2AbiWord::closeFile (void)
{
  if (m_bzin) {
    int d_error = 0;
    BZ2_bzReadClose (&d_error, m_bzin); /* Do we need to close m_fp as well? FIXME? Why else have m_fp as a class variable? */
    m_bzin = 0;
  }
}

IE_Imp_BZ2AbiWord::~IE_Imp_BZ2AbiWord ()
{
  int d_error = 0;
  if (m_bzin) BZ2_bzReadClose (&d_error, m_bzin); /* Do we need to close m_fp as well? FIXME? */
}

IE_Imp_BZ2AbiWord::IE_Imp_BZ2AbiWord (PD_Document * pDocument)
  : IE_Imp_AbiWord_1(pDocument),
    m_fp(0),
    m_bzin(0)
{
  setReader (this); // IE_Imp_BZ2AbiWord derives from UT_XML::Reader
}
