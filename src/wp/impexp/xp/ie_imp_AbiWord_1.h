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
#include "ut_vector.h"
#include "ut_stack.h"
#include "ie_imp_XML.h"
#include "ut_bytebuf.h"
class PD_Document;

// The importer/reader for AbiWord file format version 1.

class IE_Imp_AbiWord_1_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_AbiWord_1_Sniffer() {}
	virtual ~IE_Imp_AbiWord_1_Sniffer() {}

	virtual bool recognizeContents (const char * szBuf, 
									UT_uint32 iNumbytes);
	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

};

class IE_Imp_AbiWord_1 : public IE_Imp_XML
{
public:
    IE_Imp_AbiWord_1(PD_Document * pDocument);
    virtual ~IE_Imp_AbiWord_1();

    void				_startElement(const XML_Char *name, const XML_Char **atts);
    void				_endElement(const XML_Char *name);

    virtual UT_Error	importFile(const char * szFilename);

protected:
    const XML_Char *	_getDataItemName(const XML_Char ** atts);
    const XML_Char *	_getDataItemMimeType(const XML_Char ** atts);
    bool		_getDataItemEncoded(const XML_Char ** atts);
    bool			m_bDocHasLists;
    bool			m_bDocHasPageSize;
	UT_uint32		m_iInlineStart;
};

#endif /* IE_IMP_ABIWORD_1_H */





