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

/*****************************************************************/
/*****************************************************************/

bool IE_Imp_GZipAbiWord::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
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

bool IE_Imp_GZipAbiWord::RecognizeSuffix(const char * szSuffix)
{
    return (UT_stricmp(szSuffix,".zabw") == 0);
}

UT_Error IE_Imp_GZipAbiWord::StaticConstructor(PD_Document * pDocument,
	IE_Imp ** ppie)
{
    *ppie = new IE_Imp_GZipAbiWord(pDocument);;
    return UT_OK;
}

bool	IE_Imp_GZipAbiWord::GetDlgLabels(const char ** pszDesc,
	const char ** pszSuffixList,
	IEFileType * ft)
{
    *pszDesc = "GZipped AbiWord (.zabw)";
    *pszSuffixList = "*.zabw";
    *ft = IEFT_GZipAbiWord;
    return true;
}

bool IE_Imp_GZipAbiWord::SupportsFileType(IEFileType ft)
{
    return (IEFT_GZipAbiWord == ft);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void IE_Imp_GZipAbiWord::pasteFromBuffer(PD_DocumentRange * /*pDocRange*/,
										 unsigned char * /*pData*/, UT_uint32 /*lenData*/)
{
    UT_ASSERT(UT_NOT_IMPLEMENTED);
}
