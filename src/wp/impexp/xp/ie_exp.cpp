/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
#include <ctype.h>

#include "ut_assert.h"
#include "ut_types.h"
#include "ut_misc.h"
#include "ut_bytebuf.h"
#include "ut_vector.h"

#include "ut_debugmsg.h"

#include "ie_exp.h"
#include "ie_exp_AbiWord_1.h"

#include "pd_Document.h"
#include "ut_string_class.h"

static UT_Vector m_sniffers(20);

/*****************************************************************/
/*****************************************************************/

IE_ExpSniffer::IE_ExpSniffer (const char * name)
	: m_name(name),
	  m_type(IEFT_Bogus)
{
}

IE_ExpSniffer::~IE_ExpSniffer ()
{
}

/*****************************************************************/
/*****************************************************************/

void IE_Exp::registerExporter (IE_ExpSniffer * s)
{
	UT_uint32 ndx = 0;
	UT_Error err = m_sniffers.addItem (s, &ndx);

	UT_return_if_fail(err == UT_OK);
	UT_return_if_fail(ndx >= 0);

	s->setFileType(ndx+1);
}

void IE_Exp::unregisterExporter (IE_ExpSniffer * s)
{
	UT_uint32 ndx = 0;

	ndx = s->getFileType(); // 1:1 mapping

	UT_return_if_fail(ndx >= 0);

	m_sniffers.deleteNthItem (ndx-1);

	// Refactor the indexes
	IE_ExpSniffer * pSniffer = 0;
	UT_uint32 size  = m_sniffers.size();
	UT_uint32 i     = 0;
	for( i = ndx-1; i < size; i++)
	{
		pSniffer = static_cast <IE_ExpSniffer *>(m_sniffers.getNthItem(i));
		if (pSniffer)
        	pSniffer->setFileType(i+1);
	}
}

void IE_Exp::unregisterAllExporters ()
{
	IE_ExpSniffer * pSniffer = 0;
	UT_uint32 size = m_sniffers.size();

	for (UT_uint32 i = 0; i < size; i++)
	{
		pSniffer = static_cast <IE_ExpSniffer *>(m_sniffers.getNthItem(i));
		if (pSniffer)
			pSniffer->unref();
	}
}

/*****************************************************************/
/*****************************************************************/

IE_Exp::IE_Exp(PD_Document * pDocument)
	: m_pDocument(pDocument),
	  m_pDocRange (0), m_pByteBuf(0),
	  m_szFileName(0), m_error(false), m_fp(0)
{
}

IE_Exp::~IE_Exp()
{
	if (m_fp)
		_closeFile();
	if (m_szFileName)
	  delete [] m_szFileName;
}

/*****************************************************************/
/*****************************************************************/

bool IE_Exp::_openFile(const char * szFilename)
{
	UT_return_val_if_fail(!m_fp, false);
	UT_return_val_if_fail(szFilename, false);

	m_szFileName = new char[strlen(szFilename) + 1];
	strcpy(m_szFileName, szFilename);

	// TODO add code to make a backup of the original file, if it exists.

	// Open file in binary mode or UCS-2 output will be mangled.
	m_fp = fopen(szFilename,"wb+");
	return (m_fp != 0);
}

UT_uint32 IE_Exp::_writeBytes(const UT_Byte * pBytes, UT_uint32 length)
{
	if(!pBytes || !length)
	  return 0;

	return fwrite(pBytes,sizeof(UT_Byte),length,m_fp);
}

bool IE_Exp::_writeBytes(const UT_Byte * sz)
{
	int length = strlen((const char *)sz);
	return (_writeBytes(sz,length)==(UT_uint32)length);
}

bool IE_Exp::_closeFile(void)
{
	if (m_fp)
		fclose(m_fp);
	m_fp = 0;
	return true;
}

void IE_Exp::_abortFile(void)
{
	// abort the write.
	// TODO close the file and do any restore and/or cleanup.
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	_closeFile();
	return;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

PD_Document * IE_Exp::getDoc () const
{
  return m_pDocument;
}

PD_DocumentRange * IE_Exp::getDocRange() const
{
  return m_pDocRange;
}

UT_Error IE_Exp::writeFile(const char * szFilename)
{
	UT_return_val_if_fail(m_pDocument, UT_IE_COULDNOTWRITE);
	UT_return_val_if_fail(szFilename && *szFilename, UT_IE_COULDNOTWRITE);

	if (!_openFile(szFilename))
		return UT_IE_COULDNOTWRITE;

	UT_Error error = _writeDocument();

	if (UT_OK == error)
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

	UT_return_val_if_fail(m_pDocument == pDocRange->m_pDoc, UT_ERROR);
	
	m_pDocRange = pDocRange;
	m_pByteBuf = pBuf;

	UT_Error err = _writeDocument();

  	// write trailing zero to byte buffer (not required for file)
  	write("",1);

	return err;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void IE_Exp::write(const char * sz)
{
	if (m_error)
		return;

	if (!sz)
		return;

	if (m_pByteBuf)
		m_error |= (m_pByteBuf->append((UT_Byte *)sz,strlen(sz)) != true);
	else
		m_error |= ! _writeBytes((UT_Byte *)sz);

	return;
}

/*! 
 * This method deletes the last char in the output buffer. 
 * The value of the character at the last position is returned.
 */
char IE_Exp::rewindChar(void)
{
	UT_uint32 len = m_pByteBuf->getLength();
	char * pchr = (char *) m_pByteBuf->getPointer(len-1);
	char chr = *pchr;
	m_pByteBuf->del(len-1,1);
	return chr;
}

void IE_Exp::write(const char * sz, UT_uint32 length)
{
	if (m_error)
		return;

	if (!sz || !length)
		return;

	if (m_pByteBuf)
		m_error |= (m_pByteBuf->append((UT_Byte *)sz,length) != true);
	else
		m_error |= (_writeBytes((UT_Byte *)sz,length) != length);
	
	return;
}

/*****************************************************************/
/*****************************************************************/

/*! 
  Find the filetype for the given suffix.
 \param szSuffix File suffix

 Returns IEFT_AbiWord_1 if no exporter knows this suffix.
 Note that more than one exporter may support a suffix.
 We return the first one we find.
 This function should closely resemble IE_Exp::fileTypeForSuffix()
*/
IEFileType IE_Exp::fileTypeForSuffix(const char * szSuffix)
{
	if (!szSuffix)
		return IE_Exp::fileTypeForSuffix(".abw");
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getExporterCount();

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ExpSniffer * s = static_cast<IE_ExpSniffer*>(m_sniffers.getNthItem(k));
		if (s->recognizeSuffix(szSuffix))
		{
			for (UT_uint32 a = 0; a < nrElements; a++)
			{
				if (s->supportsFileType(static_cast<IEFileType>(a+1)))
					return static_cast<IEFileType>(a+1);
			}

			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			// Hm... an exporter has registered for the given suffix,
			// but refuses to support any file type we request.
			// Default to native format.
			return IE_Exp::fileTypeForSuffix(".abw");
		}
	}

	// No filter is registered for that extension, try native format
	// for default export.
	return IE_Exp::fileTypeForSuffix(".abw");
	
}

IEFileType IE_Exp::fileTypeForSuffixes(const char * suffixList)
{
	IEFileType ieft = IEFT_Unknown;
	if (!suffixList)
		return ieft;

	UT_String utSuffix (suffixList);
	const size_t len = strlen(suffixList);
	size_t i = 0;

	while (true)
		{
			while (i < len && suffixList[i] != '.')
				i++;

			// will never have all-space extension

			const size_t start = i;
			while (i < len && suffixList[i] != ';')
				i++;

			if (i <= len) {
				UT_String suffix (utSuffix.substr(start, i-start).c_str());
				UT_DEBUGMSG(("DOM: suffix: %s\n", suffix.c_str()));
				
				ieft = fileTypeForSuffix (suffix.c_str());
				if (ieft != IEFT_Unknown || i == len)
					return ieft;

				i++;
			}
		}
	return ieft;
}

/*! 
  Find the filetype for the given filetype description.
 \param szDescription Filetype description

 Returns IEFT_Unknown if no importer has this description.
 This function should closely resemble IE_Exp::fileTypeForDescription()
*/
IEFileType IE_Exp::fileTypeForDescription(const char * szDescription)
{
	IEFileType ieft = IEFT_Unknown;

	if (!szDescription)
		return ieft;
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getExporterCount();

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ExpSniffer * pSniffer = static_cast<IE_ExpSniffer *>(m_sniffers.getNthItem(k));

		const char * szDummy;
		const char * szDescription2 = 0;

		if (pSniffer->getDlgLabels(&szDescription2,&szDummy,&ieft))
		{
			if (!UT_strcmp(szDescription,szDescription2))
				return ieft;
		}
		else
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	return ieft;
}

/*! 
  Find the filetype sniffer for the given filetype.
 \param ieft Filetype

 Returns 0 if no exporter knows this filetype.
 This function should closely resemble IE_Exp::snifferForFileType()
*/
IE_ExpSniffer * IE_Exp::snifferForFileType(IEFileType ieft)
{
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getExporterCount();

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ExpSniffer * s = static_cast<IE_ExpSniffer*>(m_sniffers.getNthItem(k));
		if (s->supportsFileType(ieft))
			return s;
	}

	// The passed in filetype is invalid.
	return 0;
}

/*! 
  Find the suffixes for the given filetype.
 \param szSuffix File suffix

 Returns 0 if no exporter knows this filetype.
 This function should closely resemble IE_Exp::suffixesForFileType()
*/
const char * IE_Exp::suffixesForFileType(IEFileType ieft)
{
	const char * szDummy;
	const char * szSuffixes = 0;
	IEFileType ieftDummy;

	IE_ExpSniffer * pSniffer = snifferForFileType(ieft);

	if (pSniffer->getDlgLabels(&szDummy,&szSuffixes,&ieftDummy))
		return szSuffixes;
	else
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	// The passed in filetype is invalid.
	return 0;
}

/*! 
  Find the description for the given filetype.
 \param ieft Numerical "export filetype" ID

 Returns 0 if filetype doesn't exist.
 This function should closely resemble IE_Exp::descriptionForFileType()
*/
const char * IE_Exp::descriptionForFileType(IEFileType ieft)
{
	const char * szDummy;
	const char * szDescription = 0;
	IEFileType ieftDummy;

	IE_ExpSniffer * pSniffer = snifferForFileType(ieft);

	if (pSniffer->getDlgLabels(&szDescription,&szDummy,&ieftDummy))
		return szDescription;
	else
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	// The passed in filetype is invalid.
	return 0;
}

/*! 
  Construct an exporter of the right type.
 \param pDocument Document
 \param szFilename Name of file - optional
 \param ieft Desired filetype - pass IEFT_Unknown for best guess
 \param ppie Pointer to return importer in
 \param pieft Pointer to fill in actual filetype

 Caller is responsible for deleting the exporter object
 when finished with it.
 This function should closely match IE_Imp::contructImporter()
*/
UT_Error IE_Exp::constructExporter(PD_Document * pDocument,
								   const char * szFilename,
								   IEFileType ieft,
								   IE_Exp ** ppie,
								   IEFileType * pieft)
{
	UT_return_val_if_fail(pDocument, UT_ERROR);
	UT_return_val_if_fail(ieft != IEFT_Unknown || (szFilename && *szFilename), UT_ERROR);
	UT_return_val_if_fail(ieft != IEFT_Bogus || (szFilename && *szFilename), UT_ERROR);
	UT_return_val_if_fail(ppie, UT_ERROR);

	// no filter will support IEFT_Unknown, so we detect from the
	// suffix of the filename, the real exporter to use and assign
	// that back to ieft.
	if ( (ieft == IEFT_Unknown || ieft == IEFT_Bogus) && szFilename && *szFilename)
	{
		ieft = IE_Exp::fileTypeForSuffix(UT_pathSuffix(szFilename));
	}

	UT_ASSERT(ieft != IEFT_Unknown);
	UT_ASSERT(ieft != IEFT_Bogus);

   	// let the caller know what kind of exporter they're getting
   	if (pieft != NULL) 
		*pieft = ieft;
   
	// use the exporter for the specified file type
	UT_uint32 nrElements = getExporterCount ();
	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ExpSniffer * s = (IE_ExpSniffer*) m_sniffers.getNthItem (k);
		if (s->supportsFileType(ieft))
		{
			return s->constructExporter (pDocument, ppie);
		}
	}

	// if we got here, no registered exporter handles the
	// type of file we're supposed to be writing.
	// assume it is our format and try to write it.
	// if that fails, just give up.
	*ppie = new IE_Exp_AbiWord_1(pDocument);
	if (pieft != NULL) 
		*pieft = IE_Exp::fileTypeForSuffix(".abw");
 	return ((*ppie) ? UT_OK : UT_IE_NOMEMORY);
}

bool IE_Exp::enumerateDlgLabels(UT_uint32 ndx,
								const char ** pszDesc,
								const char ** pszSuffixList,
								IEFileType * ft)
{

	if (ndx < getExporterCount())
	{
		IE_ExpSniffer * s = (IE_ExpSniffer*) m_sniffers.getNthItem (ndx);
		return s->getDlgLabels(pszDesc,pszSuffixList,ft);
	}

	return false;
}

UT_uint32 IE_Exp::getExporterCount(void)
{
	return m_sniffers.size();
}

