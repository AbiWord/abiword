 
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

#ifndef IE_IMP_H
#define IE_IMP_H

#include "ut_types.h"
#include "ie_types.h"
class PD_Document;

// IE_Imp defines the abstract base class for file importers.

class IE_Imp
{
public:

	// constructs an importer of the right type based upon
	// either the filename or sniffing the file.  caller is
	// responsible for destroying the importer when finished
	// with it.
	
	static IEStatus		constructImporter(PD_Document * pDocument,
										  const char * szFilename,
										  IE_Imp ** ppie);

public:
	IE_Imp(PD_Document * pDocument);
	virtual ~IE_Imp();
	virtual IEStatus	importFile(const char * szFilename) = 0;

protected:
	PD_Document *		m_pDocument;
};

#endif /* IE_IMP_H */
