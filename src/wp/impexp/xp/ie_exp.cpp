
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_types.h"
#include "ie_types.h"
#include "ie_exp.h"
#include "ie_exp_AbiWord_1.h"

// TODO make this use NSPR

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
								   IEFileType ieft,
								   IE_Exp ** ppie)
{
	// construct the right type of exporter.
	// caller is responsible for deleing the exporter object
	// when finished with it.

	UT_ASSERT(pDocument);
	UT_ASSERT(szFilename && *szFilename);
	UT_ASSERT(ppie);

	switch (ieft)
	{
	case IEFT_AbiWord_1:
		{
			IE_Exp_AbiWord_1 * p = new IE_Exp_AbiWord_1(pDocument);
			if (!p)
				return IES_NoMemory;
			*ppie = p;
		}
		return IES_OK;
		
	case IEFT_Unknown:
		// TODO add logic to try to guess a file format from the filename.
		return IES_Error;

	default:
		UT_ASSERT(0);
		return IES_Error;
	}
}

