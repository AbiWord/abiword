
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
#include "ut_vector.h"

#include "ut_debugmsg.h"

#include "ie_exp.h"
#include "ie_exp_AbiWord_1.h"

#include "pd_Document.h"

#define IEFT_AbiWord_1 IE_Exp::fileTypeForSuffix(".abw")

static UT_Vector m_sniffers(20);

/*****************************************************************/
/*****************************************************************/

IE_ExpSniffer::IE_ExpSniffer ()
	: m_type (IEFT_Bogus)
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

	UT_ASSERT(err == UT_OK);
	UT_ASSERT(ndx >= 0);

	s->setFileType(ndx+1);
}

void IE_Exp::unregisterExporter (IE_ExpSniffer * s)
{
	UT_uint32 ndx = 0;

	ndx = s->getFileType(); // 1:1 mapping

	UT_ASSERT(ndx >= 0);

	m_sniffers.deleteNthItem (ndx-1);
}

/*****************************************************************/
/*****************************************************************/

IE_Exp::IE_Exp(PD_Document * pDocument)
	: m_pDocument(pDocument),
	  m_pDocRange (0), m_pByteBuf(0),
	  m_szFileName(0), m_fp(0)
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
	UT_ASSERT(!m_fp);
	UT_ASSERT(szFilename);

	m_szFileName = new char[strlen(szFilename) + 1];
	strcpy(m_szFileName, szFilename);

	// TODO add code to make a backup of the original file, if it exists.

#ifndef HAVE_GNOMEVFS
	m_fp = fopen(szFilename,"w");
	return (m_fp != 0);
#else
	GnomeVFSResult result;
	GnomeVFSURI * uri = gnome_vfs_uri_new (szFilename);

	if (!uri)
	  {
	    UT_DEBUGMSG(("GnomeVFS could not open the uri: %s\n", szFilename));
	    return false;
	  }

	result = gnome_vfs_create_uri (&m_fp, uri, GNOME_VFS_OPEN_WRITE, FALSE, 0644);
	if (result != GNOME_VFS_OK)
	  {
	    UT_DEBUGMSG(("DOM: could not open file for writing!\n"));
	    UT_DEBUGMSG(("DOM - reason: %s\n", gnome_vfs_result_to_string (result)));
	    return false;
	  }
	return true;
#endif
}

UT_uint32 IE_Exp::_writeBytes(const UT_Byte * pBytes, UT_uint32 length)
{
	UT_ASSERT(m_fp);

	if(!pBytes || !length)
	  return 0;

#ifndef HAVE_GNOMEVFS	
	return fwrite(pBytes,sizeof(UT_Byte),length,m_fp);
#else
	GnomeVFSResult result;
	GnomeVFSFileSize temp;
	result = gnome_vfs_write (m_fp, pBytes, length,
				  &temp);

	if (result != GNOME_VFS_OK)
	  return 0;
	return (UT_uint32)temp;
#endif
}

bool IE_Exp::_writeBytes(const UT_Byte * sz)
{
	UT_ASSERT(m_fp);
	if(!sz)
	  return true;
	
	int length = strlen((const char *)sz);
	return (_writeBytes(sz,length)==(UT_uint32)length);
}

bool IE_Exp::_closeFile(void)
{
#ifndef HAVE_GNOMEVFS
	if (m_fp)
		fclose(m_fp);
#else
	if (m_fp)
	  gnome_vfs_close (m_fp);
#endif
	m_fp = 0;
	return true;
}

void IE_Exp::_abortFile(void)
{
	// abort the write.
	// TODO close the file and do any restore and/or cleanup.
        UT_ASSERT(0);
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

IEFileType IE_Exp::fileTypeForSuffix(const char * szSuffix)
{
	if (!szSuffix)
		return IEFT_AbiWord_1;
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getExporterCount();

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ExpSniffer * s = (IE_ExpSniffer*) m_sniffers.getNthItem(k);
		if (s->recognizeSuffix(szSuffix))
		{
			for (UT_uint32 a = 0; a < nrElements; a++)
			{
				if (s->supportsFileType((IEFileType) a+1))
					return (IEFileType) a+1;
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
   	if (pieft != NULL) 
		*pieft = ieft;
   
	// use the exporter for the specified file type
	UT_uint32 nrElements = getExporterCount ();
	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ExpSniffer * s = (IE_ExpSniffer*) m_sniffers.getNthItem (k);
		if (s->supportsFileType(ieft))
		{
			return s->constructImporter (pDocument, ppie);
		}
	}

	// if we got here, no registered exporter handles the
	// type of file we're supposed to be writing.
	// assume it is our format and try to write it.
	// if that fails, just give up.
	*ppie = new IE_Exp_AbiWord_1(pDocument);
	if (pieft != NULL) 
		*pieft = IEFT_AbiWord_1;
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
