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


#ifndef IE_IMP_XHTML_1_H
#define IE_IMP_XHTML_1_H

#include <stdio.h>
#include "xmlparse.h"
#include "ut_vector.h"
#include "ut_stack.h"
#include "ie_imp_XML.h"
#include "ut_bytebuf.h"
class PD_Document;

// The importer/reader for XHTML 1.0

class IE_Imp_XHTML : public IE_Imp_XML
{
public:
    IE_Imp_XHTML(PD_Document * pDocument);
    ~IE_Imp_XHTML();

    static UT_Bool		RecognizeContents(const char * szBuf, UT_uint32 iNumbytes);
    static UT_Bool		RecognizeSuffix(const char * szSuffix);
    static UT_Error		StaticConstructor(PD_Document * pDocument,
	    IE_Imp ** ppie);
    static UT_Bool		GetDlgLabels(const char ** pszDesc,
	    const char ** pszSuffixList,
	    IEFileType * ft);
    static UT_Bool 		SupportsFileType(IEFileType ft);

    void			_startElement(const XML_Char *name, 
					      const XML_Char **atts);
    void			_endElement(const XML_Char *name);

	
 protected:
    typedef struct _fmtStruct {
	    UT_Bool isBold;
	    UT_Bool isItalic;
	    UT_Bool isStrike;
    } FmtStruct;

    FmtStruct                   m_currentFmt;

};

#endif /* IE_IMP_XHTML_H */
