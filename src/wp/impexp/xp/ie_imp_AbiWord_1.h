/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_hash.h"

#include "ie_imp_XML.h"


class PD_Document;

// The importer/reader for AbiWord file format version 1.

class ABI_EXPORT IE_Imp_AbiWord_1_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_AbiWord_1_Sniffer();
	virtual ~IE_Imp_AbiWord_1_Sniffer() {}

	UT_Confidence_t supportsMIME (const char * szMIME);

	virtual UT_Confidence_t recognizeContents (const char * szBuf, 
									UT_uint32 iNumbytes);
	virtual UT_Confidence_t recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

};

class ABI_EXPORT IE_Imp_AbiWord_1 : public IE_Imp_XML
{
public:
    IE_Imp_AbiWord_1(PD_Document * pDocument);

    virtual ~IE_Imp_AbiWord_1();

    void				startElement(const XML_Char *name, const XML_Char **atts);
    void				endElement(const XML_Char *name);

    virtual UT_Error	importFile(const char * szFilename);
	virtual bool        supportsLoadStylesOnly() {return true;}

protected:
    const XML_Char *	_getDataItemName(const XML_Char ** atts);
    const XML_Char *	_getDataItemMimeType(const XML_Char ** atts);
    bool				_getDataItemEncoded(const XML_Char ** atts);

    bool				_handleImage(const XML_Char ** atts);
    bool				_handleResource(const XML_Char ** atts, bool isResource);

 private:
    bool				m_bWroteSection;
    bool				m_bWroteParagraph;
    bool				m_bDocHasLists;
    bool				m_bDocHasPageSize;

    UT_uint32			m_iInlineStart;

	UT_GenericStringMap<UT_UTF8String *> *	m_refMap;
	bool                m_bAutoRevisioning;
	bool                m_bInMath;
};

#endif /* IE_IMP_ABIWORD_1_H */
