 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
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
