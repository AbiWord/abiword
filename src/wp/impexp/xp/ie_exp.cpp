
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
#include "ut_misc.h"
#include "ut_bytebuf.h"
#include "ie_exp.h"
#include "ie_exp_AbiWord_1.h"
#include "ie_exp_GZipAbiWord.h"

#ifdef DEBUG
// Don't include until it works
#include "ie_exp_MsWord_97.h"
#endif

#include "ie_exp_RTF.h"
#include "ie_exp_Text.h"
#include "ie_exp_HTML.h"
#include "ie_exp_UTF8.h"
#include "ie_exp_LaTeX.h"
#include "ie_exp_PalmDoc.h"
#include "ie_exp_WML.h"
#include "ie_exp_DocBook.h"

/*****************************************************************/
/*****************************************************************/

struct _xp
{
	UT_Bool			(*fpRecognizeSuffix)(const char * szSuffix);

	UT_Error		(*fpStaticConstructor)(PD_Document * pDocument,
										   IE_Exp ** ppie);
	UT_Bool			(*fpGetDlgLabels)(const char ** szDesc,
									  const char ** szSuffixList,
									  IEFileType * ft);
	UT_Bool			(*fpSupportsFileType)(IEFileType ft);
};

#define DeclareExporter(n)	{ n::RecognizeSuffix, n::StaticConstructor, n::GetDlgLabels, n::SupportsFileType }
#define DeclareExporter_sub(n,subfmt)	{ n::RecognizeSuffix_##subfmt, n::StaticConstructor_##subfmt, n::GetDlgLabels_##subfmt, n::SupportsFileType_##subfmt }

static struct _xp s_expTable[] =
{
	DeclareExporter(IE_Exp_AbiWord_1),
	DeclareExporter(IE_Exp_GZipAbiWord),
#ifdef DEBUG
	DeclareExporter(IE_Exp_MsWord_97),
	//	Don't declare until it works
#endif
	DeclareExporter(IE_Exp_RTF),
	DeclareExporter_sub(IE_Exp_RTF,attic),
	DeclareExporter(IE_Exp_Text),
	DeclareExporter(IE_Exp_UTF8),
	DeclareExporter(IE_Exp_HTML),
	DeclareExporter(IE_Exp_LaTeX),
	DeclareExporter(IE_Exp_PalmDoc),
	DeclareExporter(IE_Exp_WML),
	DeclareExporter(IE_Exp_DocBook)
};

/*****************************************************************/
/*****************************************************************/

IE_Exp::IE_Exp(PD_Document * pDocument)
{
	m_fp = 0;
	m_pDocument = pDocument;
	m_pDocRange = NULL;
	m_pByteBuf = NULL;
}

IE_Exp::~IE_Exp()
{
	if (m_fp)
		_closeFile();
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Exp::_openFile(const char * szFilename)
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

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_Error IE_Exp::writeFile(const char * szFilename)
{
	UT_ASSERT(m_pDocument);
	UT_ASSERT(szFilename && *szFilename);

	if (!_openFile(szFilename))
		return UT_IE_COULDNOTWRITE;

	UT_Error error = _writeDocument();
	if (!error)
		_closeFile();
	else
		_abortFile();

	// Note: we let our caller worry about resetting the dirty bit
	// Note: on the document and possibly updating the filename.
	
	return error;
}

UT_Error IE_Exp::copyToBuffer(PD_DocumentRange * pDocRange, UT_ByteBuf * pBuf)
{
	// copy selected range of the document into the provided
	// byte buffer.  (this will be given to the clipboard later)

	UT_ASSERT(m_pDocument == pDocRange->m_pDoc);
	
	m_pDocRange = pDocRange;
	m_pByteBuf = pBuf;

	return _writeDocument();
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void IE_Exp::write(const char * sz)
{
	if (m_error)
		return;

	if (m_pByteBuf)
		m_error |= (m_pByteBuf->append((UT_Byte *)sz,strlen(sz)) != UT_TRUE);
	else
		m_error |= ! _writeBytes((UT_Byte *)sz);

	return;
}

void IE_Exp::write(const char * sz, UT_uint32 length)
{
	if (m_error)
		return;

	if (m_pByteBuf)
		m_error |= (m_pByteBuf->append((UT_Byte *)sz,length) != UT_TRUE);
	else
		m_error |= (_writeBytes((UT_Byte *)sz,length) != length);
	
	return;
}

/*****************************************************************/
/*****************************************************************/

IEFileType IE_Exp::fileTypeForSuffix(const char * szSuffix)
{
	if (!szSuffix)
		return IEFT_AbiWord_1;
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	for (UT_uint32 k=0; (k < NrElements(s_expTable)); k++)
	{
		struct _xp * s = &s_expTable[k];
		if (s->fpRecognizeSuffix(szSuffix))
		{
			for (UT_uint32 a = 0; a < (int) IEFT_LAST_BOGUS; a++)
			{
				if (s->fpSupportsFileType((IEFileType) a))
					return (IEFileType) a;
			}

			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			// Hm... an exporter has registered for the given suffix,
			// bug refuses to support any file type we request.
			// Default to native format.
			return IEFT_AbiWord_1;
		}
	}

	// No filter is registered for that extension, try native format
	// for default export.
	return IEFT_AbiWord_1;
	
}

UT_Error IE_Exp::constructExporter(PD_Document * pDocument,
				   const char * szFilename,
				   IEFileType ieft,
				   IE_Exp ** ppie,
				   IEFileType * pieft)
{
	// construct the right type of exporter.
	// caller is responsible for deleing the exporter object
	// when finished with it.

	UT_ASSERT(pDocument);
	UT_ASSERT(szFilename && *szFilename);
	UT_ASSERT(ppie);

	// no filter will support IEFT_Unknown, so we detect from the
	// suffix of the filename, the real exporter to use and assign
	// that back to ieft.
	if (ieft == IEFT_Unknown)
	{
		ieft = IE_Exp::fileTypeForSuffix(UT_pathSuffix(szFilename));
	}

	UT_ASSERT(ieft != IEFT_Unknown);

   	// let the caller know what kind of exporter they're getting
   	if (pieft != NULL) *pieft = ieft;
   
	// use the exporter for the specified file type
	for (UT_uint32 k=0; (k < NrElements(s_expTable)); k++)
	{
		struct _xp * s = &s_expTable[k];
		if (s->fpSupportsFileType(ieft))
			return s->fpStaticConstructor(pDocument,ppie);
	}

	// if we got here, no registered exporter handles the
	// type of file we're supposed to be writing.
	// assume it is our format and try to write it.
	// if that fails, just give up.
	*ppie = new IE_Exp_AbiWord_1(pDocument);
	if (pieft != NULL) *pieft = IEFT_AbiWord_1;
 	return ((*ppie) ? UT_OK : UT_IE_NOMEMORY);
}

UT_Bool IE_Exp::enumerateDlgLabels(UT_uint32 ndx,
								   const char ** pszDesc,
								   const char ** pszSuffixList,
								   IEFileType * ft)
{
	if (ndx < NrElements(s_expTable))
		return s_expTable[ndx].fpGetDlgLabels(pszDesc,pszSuffixList,ft);

	return UT_FALSE;
}

UT_uint32 IE_Exp::getExporterCount(void)
{
	return NrElements(s_expTable);
}
