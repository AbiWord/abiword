/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 



#ifndef IE_EXP_H
#define IE_EXP_H

#include <stdio.h>

#include "ut_types.h"
#include "ie_types.h"
class PD_Document;

// IE_Exp defines the base class for file exporters.

class IE_Exp
{
public:
	// constructs an exporter of the right type based upon
	// either the filename or the key given.  caller is
	// responsible for destroying the exporter when finished
	// with it.

	static IEStatus		constructExporter(PD_Document * pDocument,
										  const char * szFilename,
										  IEFileType ieft,
										  IE_Exp ** ppie);
	
public:
	IE_Exp(PD_Document * pDocument);
	virtual ~IE_Exp();
	virtual IEStatus	writeFile(const char * szFilename) = 0;

protected:
	// derived classes should use these to open/close
	// and write data to the actual file.  this will
	// let us handle file backups, etc.
	
	UT_Bool				_openFile(const char * szFilename);
	UT_uint32			_writeBytes(const UT_Byte * pBytes, UT_uint32 length);
	UT_Bool				_writeBytes(const UT_Byte * sz);
	UT_Bool				_closeFile(void);
	void				_abortFile(void);

	PD_Document *		m_pDocument;

private:
	FILE *				m_fp;

};


#endif /* IE_EXP_H */
