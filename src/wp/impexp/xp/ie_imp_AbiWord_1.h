 
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

#ifndef IE_IMP_ABIWORD_1_H
#define IE_IMP_ABIWORD_1_H

#include <stdio.h>
#include "xmlparse.h"
#include "ut_vector.h"
#include "ut_stack.h"
#include "ie_imp.h"
class PD_Document;

// The importer/reader for AbiWord file format version 1.

class IE_Imp_AbiWord_1 : public IE_Imp
{
public:
	IE_Imp_AbiWord_1(PD_Document * pDocument);
	~IE_Imp_AbiWord_1();

	IEStatus			importFile(const char * szFilename);

	// the following are public only so that the
	// XML parser callback routines can access them.
	
	void _startElement(const XML_Char *name, const XML_Char **atts);
	void _endElement(const XML_Char *name);
	void _charData(const XML_Char*, int);

protected:
	UT_uint32		_getInlineDepth(void) const;
	UT_Bool			_pushInlineFmt(const XML_Char ** atts);
	void			_popInlineFmt(void);
	
protected:
	typedef enum _parseState { _PS_Init,
							   _PS_Doc,
							   _PS_Sec,
							   _PS_ColSet,
							   _PS_Col,
							   _PS_Block } ParseState;

	IEStatus		m_iestatus;
	ParseState		m_parseState;

	UT_Vector		m_vecInlineFmt;
	UT_Stack		m_stackFmtStartIndex;
};

#endif /* IE_IMP_ABIWORD_1_H */
