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


#include "string.h"

#include "ie_exp_GZipAbiWord.h"
#include "ut_assert.h"
#include "ut_string.h"

#include <gsf/gsf-output-gzip.h>

/*****************************************************************/
/*****************************************************************/

IE_Exp_GZipAbiWord_Sniffer::IE_Exp_GZipAbiWord_Sniffer ()
	: IE_ExpSniffer(IE_IMPEXPNAME_AWML11GZ)
{
	// 
}

bool IE_Exp_GZipAbiWord_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix,".zabw") || !UT_stricmp(szSuffix, ".abw.gz"));
}

UT_Error IE_Exp_GZipAbiWord_Sniffer::constructExporter(PD_Document * pDocument,
													 IE_Exp ** ppie)
{
	*ppie = new IE_Exp_GZipAbiWord(pDocument);
	return UT_OK;
}

bool IE_Exp_GZipAbiWord_Sniffer::getDlgLabels(const char ** pszDesc,
											const char ** pszSuffixList,
											IEFileType * ft)
{
	*pszDesc = "GZipped AbiWord (.zabw)";

#ifdef WIN32  
	*pszSuffixList = "*.zabw" ;  
#else  
	*pszSuffixList = "*.abw.gz; *.zabw";
#endif

	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_GZipAbiWord::IE_Exp_GZipAbiWord(PD_Document * pDocument)
  : IE_Exp_AbiWord_1(pDocument)
{
}

IE_Exp_GZipAbiWord::~IE_Exp_GZipAbiWord()
{
}

GsfOutput* IE_Exp_GZipAbiWord::_openFile(const char * szFilename)
{
  GsfOutput * output = UT_go_file_create(szFilename, NULL);
  if(!output)
    return NULL;

  return gsf_output_gzip_new(output, NULL);
}
