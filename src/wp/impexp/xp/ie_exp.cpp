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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_types.h"
#include "ie_types.h"
#include "ie_exp.h"
#include "ie_exp_AbiWord_1.h"
#include "ie_exp_Text.h"
#include "ie_exp_HTML.h"

/*****************************************************************/
/*****************************************************************/

struct _xp
{
	UT_Bool			(*fpRecognizeSuffix)(const char * szSuffix);

	IEStatus		(*fpStaticConstructor)(const char * szSuffix,
										   PD_Document * pDocument,
										   IE_Exp ** ppie);
	UT_Bool			(*fpGetDlgLabels)(const char ** szDesc,
									  const char ** szSuffixList);
};

#define DeclareExporter(n)	{ n::RecognizeSuffix, n::StaticConstructor, n::GetDlgLabels }

static struct _xp s_expTable[] =
{
	DeclareExporter(IE_Exp_AbiWord_1),
	DeclareExporter(IE_Exp_Text),
	DeclareExporter(IE_Exp_HTML),
};

/*****************************************************************/
/*****************************************************************/

IE_Exp::IE_Exp(PD_Document * pDocument)
{
	m_fp = 0;
	m_pDocument = pDocument;
}

IE_Exp::~IE_Exp()
{
	if (!m_fp)
		_closeFile();
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Exp::	_openFile(const char * szFilename)
{
	UT_ASSERT(!m_fp);

	// TODO add code to make a backup of the original file, if it exists.
	
	m_fp = fopen(szFilename,"w");
	return (m_fp != 0);
}

UT_uint32 IE_Exp::_writeBytes(const UT_Byte * pBytes, UT_uint32 length)
{
	UT_ASSERT(m_fp);
	UT_ASSERT(pBytes);
	UT_ASSERT(length);
	
	return fwrite(pBytes,sizeof(UT_Byte),length,m_fp);
}

UT_Bool IE_Exp::_writeBytes(const UT_Byte * sz)
{
	UT_ASSERT(m_fp);
	UT_ASSERT(sz);
	int length = strlen((const char *)sz);
	UT_ASSERT(length);
	
	return (_writeBytes(sz,length)==(UT_uint32)length);
}

UT_Bool IE_Exp::_closeFile(void)
{
	if (m_fp)
		fclose(m_fp);
	m_fp = 0;
	return UT_TRUE;
}

void IE_Exp::_abortFile(void)
{
	// abort the write.
	// TODO close the file and do any restore and/or cleanup.

	_closeFile();
	return;
}

/*****************************************************************/
/*****************************************************************/

IEStatus IE_Exp::constructExporter(PD_Document * pDocument,
								   const char * szFilename,
								   IEFileType /* ieft */,
								   IE_Exp ** ppie)
{
	// construct the right type of exporter.
	// caller is responsible for deleing the exporter object
	// when finished with it.

	UT_ASSERT(pDocument);
	UT_ASSERT(szFilename && *szFilename);
	UT_ASSERT(ppie);

	const char * pExt = strrchr(szFilename,'.');

	if (!pExt)
	{
		// no suffix -- what to do ??
		// assume it is our format and try to save it.

		*ppie = new IE_Exp_AbiWord_1(pDocument);
		return ((*ppie) ? IES_OK : IES_NoMemory);
	}

	for (UT_uint32 k=0; (k < NrElements(s_expTable)); k++)
	{
		struct _xp * s = &s_expTable[k];
		if (s->fpRecognizeSuffix(pExt))
			return s->fpStaticConstructor(pExt,pDocument,ppie);
	}

	return IES_UnknownType;
}

UT_Bool IE_Exp::enumerateDlgLabels(UT_uint32 ndx,
								   const char ** pszDesc,
								   const char ** pszSuffixList)
{
	if (ndx < NrElements(s_expTable))
		return s_expTable[ndx].fpGetDlgLabels(pszDesc,pszSuffixList);

	return UT_FALSE;
}

