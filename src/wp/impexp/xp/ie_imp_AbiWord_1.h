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


#ifndef IE_IMP_ABIWORD_1_H
#define IE_IMP_ABIWORD_1_H

#include <stdio.h>
#include "xmlparse.h"
#include "ut_vector.h"
#include "ut_stack.h"
#include "ie_imp.h"
#include "ut_bytebuf.h"
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
	
	void				_startElement(const XML_Char *name, const XML_Char **atts);
	void				_endElement(const XML_Char *name);
	void				_charData(const XML_Char*, int);

	static UT_Bool		RecognizeSuffix(const char * szSuffix);
	static IEStatus		StaticConstructor(const char * szSuffix,
										  PD_Document * pDocument,
										  IE_Imp ** ppie);
	static UT_Bool		GetDlgLabels(const char ** pszDesc,
									 const char ** pszSuffixList);
	
protected:
	UT_uint32			_getInlineDepth(void) const;
	UT_Bool				_pushInlineFmt(const XML_Char ** atts);
	void				_popInlineFmt(void);
	const XML_Char *	_getDataItemName(const XML_Char ** atts);
	
	typedef enum _parseState { _PS_Init,
							   _PS_Doc,
							   _PS_Sec,
							   _PS_Block,
							   _PS_DataSec,
							   _PS_DataItem,
	} ParseState;

	IEStatus			m_iestatus;
	ParseState			m_parseState;
	XML_Char			m_charDataSeen[4];
	UT_uint32			m_lenCharDataSeen;
	UT_uint32			m_lenCharDataExpected;
	UT_Bool				m_bSeenCR;
	
	UT_Vector			m_vecInlineFmt;
	UT_Stack			m_stackFmtStartIndex;

	UT_ByteBuf			m_currentDataItem;
	XML_Char *			m_currentDataItemName;
};

#endif /* IE_IMP_ABIWORD_1_H */
