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
#include "ie_exp_GZipAbiWord.h"
#include "ut_assert.h"
#include "ut_string.h"

IE_Exp_GZipAbiWord::IE_Exp_GZipAbiWord(PD_Document * pDocument)
	: IE_Exp_AbiWord_1(pDocument)
{
    m_gzfp = 0;
}

IE_Exp_GZipAbiWord::~IE_Exp_GZipAbiWord()
{
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Exp_GZipAbiWord::RecognizeSuffix(const char * szSuffix)
{
    return (UT_stricmp(szSuffix,".zabw") == 0);
}

IEStatus IE_Exp_GZipAbiWord::StaticConstructor(PD_Document * pDocument,
	IE_Exp ** ppie)
{
    *ppie = new IE_Exp_GZipAbiWord(pDocument);
    return IES_OK;
}

UT_Bool	IE_Exp_GZipAbiWord::GetDlgLabels(const char ** pszDesc,
	const char ** pszSuffixList,
	IEFileType * ft)
{
    *pszDesc = "GZipped AbiWord (.zabw)";
    *pszSuffixList = "*.zabw";
    *ft = IEFT_GZipAbiWord;
    return UT_TRUE;
}

UT_Bool IE_Exp_GZipAbiWord::SupportsFileType(IEFileType ft)
{
    return (IEFT_GZipAbiWord == ft);
}

UT_Bool IE_Exp_GZipAbiWord::_openFile(const char * szFilename)
{
    UT_ASSERT(!m_gzfp);

    m_gzfp = (gzFile) gzopen(szFilename, "wb6");
    return (m_gzfp != 0);
}

UT_uint32 IE_Exp_GZipAbiWord::_writeBytes(const UT_Byte * pBytes, UT_uint32 length)
{
    UT_ASSERT(m_gzfp);
    UT_ASSERT(pBytes);
    UT_ASSERT(length);

    return gzwrite(m_gzfp, (void *) pBytes, sizeof(UT_Byte) * length);
}

UT_Bool IE_Exp_GZipAbiWord::_writeBytes(const UT_Byte * sz)
{
    UT_ASSERT(m_gzfp);
    UT_ASSERT(sz);
    int length = strlen((const char *)sz);
    UT_ASSERT(length);
    
    return (_writeBytes(sz,length)==(UT_uint32)length);
}

UT_Bool IE_Exp_GZipAbiWord::_closeFile(void)
{
    if (m_gzfp)
	gzclose(m_gzfp);

    m_gzfp = 0;
    return UT_TRUE;
}
