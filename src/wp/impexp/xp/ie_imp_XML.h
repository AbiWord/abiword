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


#ifndef IE_IMP_XML_H
#define IE_IMP_XML_H

#include <stdio.h>

// current XML engines supported: XML2 and Expat 1.1
#ifdef HAVE_LIBXML2
#include <libxml/parser.h>
#else
#include "xmlparse.h"
#endif

#include "ut_vector.h"
#include "ut_stack.h"
#include "ie_imp.h"
#include "ut_bytebuf.h"
class PD_Document;

// The importer/reader for reading generic
// XML documents. The Abiword and WML importers
// Derive from this *abstract* base class

class IE_Imp_XML : public IE_Imp
{
public:
    IE_Imp_XML(PD_Document * pDocument, UT_Bool whiteSignificant);
    virtual ~IE_Imp_XML();

    virtual UT_Error	importFile(const char * szFilename);
    virtual void	pasteFromBuffer(PD_DocumentRange * pDocRange,
					unsigned char * pData, 
					UT_uint32 lenData);

    // the following are public only so that the
    // XML parser callback routines can access them.
#ifdef HAVE_LIBXML2
    void _scannode(xmlDocPtr dok, xmlNodePtr cur, int c);
#endif	

    // you *must* override these next two methods
    virtual void	_startElement(const XML_Char *name, 
				      const XML_Char **atts) = 0;
    virtual void        _endElement(const XML_Char *name) = 0;

    virtual void	_charData(const XML_Char*, int len);

protected:
    virtual UT_Bool			_openFile(const char * szFilename);
    virtual UT_uint32			_readBytes(char * buf, UT_uint32 length);
    virtual void			_closeFile(void);

    const XML_Char *            _getXMLPropValue(const XML_Char *name, const XML_Char **atts);

    UT_uint32			_getInlineDepth(void) const;
    UT_Bool			_pushInlineFmt(const XML_Char ** atts);
    void			_popInlineFmt(void);

    typedef enum _parseState { _PS_Init,
			       _PS_Doc,
			       _PS_Sec,
			       _PS_Block,
			       _PS_DataSec,
			       _PS_DataItem,
			       _PS_StyleSec,
			       _PS_Style,
			       _PS_ListSec,
			       _PS_List,
			       _PS_Field
    } ParseState;

    ParseState                  m_parseState;
    UT_Error			m_error;
    XML_Char			m_charDataSeen[4];
    UT_uint32			m_lenCharDataSeen;
    UT_uint32			m_lenCharDataExpected;
    UT_Bool			m_bSeenCR;
    UT_Bool                     m_bWhiteSignificant;
    UT_Bool                     m_bWasSpace;

    UT_Vector			m_vecInlineFmt;
    UT_Stack			m_stackFmtStartIndex;

    UT_ByteBuf			m_currentDataItem;
    XML_Char *			m_currentDataItemName;
    XML_Char *			m_currentDataItemMimeType;
    UT_Bool			m_currentDataItemEncoded;

    FILE *			m_fp;
};

#endif /* IE_IMP_XML_H */
