//* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#ifndef IE_IMP_XML_H
#define IE_IMP_XML_H

#include <stdio.h>
#include <string>
#include <map>
#include <memory>

#include "ut_xml.h"

#include "ut_vector.h"
#include "ut_stack.h"
#include "ie_imp.h"
#include "ut_bytebuf.h"
#include "ut_string_class.h"

class PD_Document;

class PD_DocumentRDFMutation;
typedef std::shared_ptr<PD_DocumentRDFMutation> PD_DocumentRDFMutationHandle;


struct ABI_EXPORT xmlToIdMapping {
  const char *m_name;
  int m_type;
};

// The importer/reader for reading generic
// XML documents. Currently, the following classes derive from this:
//
// ABW, AWT, GZABW
// DBK
// WML
// XHTML
// XSL-FO
// KWORD 1 && 2 (soon)

class ABI_EXPORT IE_Imp_XML : public IE_Imp, public UT_XML::Listener
{
public:
    IE_Imp_XML(PD_Document * pDocument, bool whiteSignificant);
    virtual ~IE_Imp_XML();
    virtual UT_Error	importFile(const char * data, UT_uint32 length);
	virtual UT_Error    importFile(const UT_ByteBuf * data);

	virtual bool		pasteFromBuffer(PD_DocumentRange * pDocRange,
										const unsigned char * pData,
										UT_uint32 lenData,
										const char * szEncoding = 0);

    /* (Partial) Implementation of UT_XML::Listener
     *
     * You *must* override these next two methods:
     */
    virtual void startElement (const gchar * name, const gchar ** atts);
    virtual void endElement (const gchar * name);
    /*
     * but you get this one for free:
     */
    virtual void charData (const gchar * buffer, int length);

    /* If you don't wish the XML parser to use the standard/default file handler, you
     * can provide your own via an implementation of UT_XML::Reader here:
     */
protected:
    void setReader (UT_XML::Reader * pReader) { m_pReader = pReader; }
private:
    UT_XML::Reader * m_pReader;

    /* If you wish to use a non-standard parser (e.g., for HTML), then maybe this
     * is useful...
     */
protected:
    void setParser (UT_XML * pParser) { m_pParser = pParser; }
	void stopParser(void) {if(m_pParser) m_pParser->stop();}
private:
    UT_XML * m_pParser;

public:
    void		    incOperationCount(void) { m_iOperationCount++; }
    UT_uint32		getOperationCount(void) const { return m_iOperationCount; }

protected:

    virtual UT_Error	_loadFile(GsfInput * input);
    int             _mapNameToToken (const char * name, xmlToIdMapping * idlist, int len);

    const gchar* _getXMLPropValue(const gchar *name, const gchar **atts);

    UT_uint32		_getInlineDepth(void) const;
    bool			_pushInlineFmt(const PP_PropertyVector & atts);
    void			_popInlineFmt(void);

    typedef enum _parseState { _PS_Init,
			       _PS_Doc,
			       _PS_Sec,
			       _PS_Block,
			       _PS_DataSec,
			       _PS_DataItem,
			       _PS_StyleSec,
			       _PS_Style,
			       _PS_IgnoredWordsSec,
			       _PS_IgnoredWordsItem,
			       _PS_ListSec,
			       _PS_List,
			       _PS_Field,
			       _PS_PageSize,
			       _PS_MetaData,
				   _PS_Meta,
				   _PS_RevisionSec,
				   _PS_Revision,
				   _PS_AuthorSec,
				   _PS_Author,
				   _PS_HistorySec,
				   _PS_Table,
				   _PS_Cell,
				   _PS_Version,
				   _PS_RDFTriple,
				   _PS_RDFData,
    } ParseState;

 protected:

    // TODO: make us private, refactor code
    UT_Error        m_error;
    ParseState      m_parseState;

    gchar		m_charDataSeen[4];
    UT_uint32		m_lenCharDataSeen;
    UT_uint32		m_lenCharDataExpected;
    UT_uint32		m_iOperationCount;
    bool			m_bSeenCR;
    bool            m_bWhiteSignificant;
    bool            m_bWasSpace;

    PP_PropertyVector m_vecInlineFmt;
    UT_NumberStack		m_nstackFmtStartIndex;

    UT_ByteBuf		m_currentDataItem;
    std::string		m_currentDataItemName;
    std::string		m_currentDataItemMimeType;
    bool			m_currentDataItemEncoded;

	const char *	m_szFileName;

	std::string		m_currentMetaDataName;
	UT_uint32       m_currentRevisionId;
	time_t          m_currentRevisionTime;
	UT_uint32       m_currentRevisionVersion;

    // For reading RDF triples
    std::string     m_rdfSubject;
    std::string     m_rdfPredicate;
    std::string     m_rdfXSDType;
    int             m_rdfObjectType;
    PD_DocumentRDFMutationHandle m_rdfMutation;

	typedef std::map<std::string, UT_sint32> token_map_t;
	token_map_t m_tokens;

private:
	UT_uint32	m_iCharCount;
	bool		m_bStripLeading;
protected:
	UT_uint32	_data_CharCount () const { return m_iCharCount; }
	void		_data_NewBlock ();
};

#endif /* IE_IMP_XML_H */
