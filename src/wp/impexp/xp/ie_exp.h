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



#ifndef IE_EXP_H
#define IE_EXP_H

#include <stdio.h>

#include "ut_types.h"
#include "ie_types.h"

class PD_Document;
class PD_DocumentRange;
class UT_ByteBuf;

//////////////////////////////////////////////////////////////////
// IE_Exp defines the base class for file exporters.
//////////////////////////////////////////////////////////////////

class IE_Exp
{
public:
	// constructs an exporter of the right type based upon
	// either the filename or the key given.  caller is
	// responsible for destroying the exporter when finished
	// with it.

	static IEFileType	fileTypeForSuffix(const char * szSuffix);
	
	static UT_Error		constructExporter(PD_Document * pDocument,
										  const char * szFilename,
										  IEFileType ieft,
										  IE_Exp ** ppie);
	static UT_Bool		enumerateDlgLabels(UT_uint32 ndx,
										   const char ** pszDesc,
										   const char ** pszSuffixList,
										   IEFileType * ft);
	static UT_uint32	getExporterCount(void);
	
public:
	IE_Exp(PD_Document * pDocument);
	virtual ~IE_Exp();

	virtual UT_Error	writeFile(const char * szFilename);
	virtual UT_Error	copyToBuffer(PD_DocumentRange * pDocRange, UT_ByteBuf * pBuf);
	virtual void		write(const char * sz);
	virtual void		write(const char * sz, UT_uint32 length);

protected:
	virtual UT_Error	_writeDocument(void) = 0;
	
	// derived classes should use these to open/close
	// and write data to the actual file.  this will
	// let us handle file backups, etc.
	virtual UT_Bool		_openFile(const char * szFilename);
	virtual UT_uint32	_writeBytes(const UT_Byte * pBytes, UT_uint32 length);
	virtual UT_Bool		_writeBytes(const UT_Byte * sz);
	virtual UT_Bool		_closeFile(void);
	virtual void		_abortFile(void);
	
	PD_Document *		m_pDocument;
	PD_DocumentRange *	m_pDocRange;
	UT_ByteBuf *		m_pByteBuf;

public:
	UT_Bool				m_error;

private:
	FILE *				m_fp;

};


#endif /* IE_EXP_H */
