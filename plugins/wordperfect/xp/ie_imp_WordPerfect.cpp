/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2002-2004 Marc Maurer (uwog@uwog.net)
 * Copyright (C) 2002-2005 William Lachance (william.lachance@sympatico.ca)
 * Copyright (C) 2006 Fridrich Strba (fridrich.strba@bluewin.ch)
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

/* See bug 1764
 * "This product is not manufactured, approved, or supported by 
 * Corel Corporation or Corel Corporation Limited."
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <map>
#include <string>
#include <gsf/gsf-utils.h>
#include <gsf/gsf-input-memory.h>
#include <gsf/gsf-input-stdio.h>

#include "ut_types.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_units.h"
#include "ut_growbuf.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_math.h" // for rint (font size)
#include "ut_rand.h"
#include "ut_locale.h"

#include "xap_Frame.h"
#include "xap_EncodingManager.h"

#include "pd_Document.h"
#include "pt_Types.h"

#include "fl_AutoLists.h"
#include "fl_AutoNum.h"

#include "ie_imp_WordPerfect.h"
#include "ie_impexp_WordPerfect.h"

// Stream class

#include <librevenge-stream/librevenge-stream.h>
#include <libwpd/libwpd.h>

#include <gsf/gsf-input.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-infile-zip.h>

#ifdef HAVE_LIBWPS
#include <libwps/libwps.h>
#endif

class AbiWordperfectInputStream : public librevenge::RVNGInputStream
{
public:
	AbiWordperfectInputStream(GsfInput *input);
	~AbiWordperfectInputStream();

	virtual bool isStructured();
	virtual unsigned subStreamCount();
	virtual const char* subStreamName(unsigned);
	bool existsSubStream(const char*);
	virtual librevenge::RVNGInputStream* getSubStreamByName(const char*);
	virtual librevenge::RVNGInputStream* getSubStreamById(unsigned);
	virtual const unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead);
	virtual int seek(long offset, librevenge::RVNG_SEEK_TYPE seekType);
	virtual long tell();
	virtual bool isEnd();

private:

	GsfInput *m_input;
	GsfInfile *m_ole;
	std::map<unsigned, std::string> m_substreams;
};

AbiWordperfectInputStream::AbiWordperfectInputStream(GsfInput *input) :
	librevenge::RVNGInputStream(),
	m_input(input),
	m_ole(NULL),
	m_substreams()
{
	g_object_ref(G_OBJECT(input));
}

AbiWordperfectInputStream::~AbiWordperfectInputStream()
{
	if (m_ole)
		g_object_unref(G_OBJECT(m_ole));

	g_object_unref(G_OBJECT(m_input));
}

const unsigned char * AbiWordperfectInputStream::read(unsigned long numBytes, unsigned long &numBytesRead)
{
	const unsigned char *buf = gsf_input_read(m_input, numBytes, NULL);

	if (buf == NULL)
		numBytesRead = 0;
	else
		numBytesRead = numBytes;

	return buf;
}

int AbiWordperfectInputStream::seek(long offset, librevenge::RVNG_SEEK_TYPE seekType) 
{
	GSeekType gsfSeekType = G_SEEK_SET;
	switch(seekType)
	{
	case librevenge::RVNG_SEEK_CUR:
		gsfSeekType = G_SEEK_CUR;
		break;
	case librevenge::RVNG_SEEK_SET:
		gsfSeekType = G_SEEK_SET;
		break;
	case librevenge::RVNG_SEEK_END:
		gsfSeekType = G_SEEK_END;
		break;
	}

	return gsf_input_seek(m_input, offset, gsfSeekType);
}

bool AbiWordperfectInputStream::isStructured()
{
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_msole_new (m_input, NULL)); 

	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_zip_new (m_input, NULL)); 
	
	if (m_ole)
		return true;

	return false;
}

unsigned AbiWordperfectInputStream::subStreamCount()
{
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_msole_new (m_input, NULL)); 
	
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_zip_new (m_input, NULL)); 
	
	if (m_ole)
		{
			int numChildren = gsf_infile_num_children(m_ole);
			if (numChildren > 0)
				return numChildren;
			return 0;
		}
	
	return 0;
}

const char * AbiWordperfectInputStream::subStreamName(unsigned id)
{
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_msole_new (m_input, NULL)); 
	
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_zip_new (m_input, NULL)); 
	
	if (m_ole)
		{
			if ((int)id >= gsf_infile_num_children(m_ole))
			{
				return 0;
			}
			std::map<unsigned, std::string>::iterator i = m_substreams.lower_bound(id);
			if (i == m_substreams.end() || m_substreams.key_comp()(id, i->first))
				{
					std::string name = gsf_infile_name_by_index(m_ole, (int)id);
					i = m_substreams.insert(i, std::map<unsigned, std::string>::value_type(id, name));
				}
			return i->second.c_str();
		}
	
	return 0;
}

bool AbiWordperfectInputStream::existsSubStream(const char * name)
{
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_msole_new (m_input, NULL)); 
	
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_zip_new (m_input, NULL)); 
	
	if (m_ole)
		{
			GsfInput *document = gsf_infile_child_by_name(m_ole, name);
			if (document) 
				{
					g_object_unref(G_OBJECT (document));
					return true;
				}
		}
	
	return false;
}

librevenge::RVNGInputStream * AbiWordperfectInputStream::getSubStreamByName(const char * name)
{
	librevenge::RVNGInputStream *documentStream = NULL;
	
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_msole_new (m_input, NULL)); 
	
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_zip_new (m_input, NULL)); 
	
	if (m_ole)
		{
			GsfInput *document = gsf_infile_child_by_name(m_ole, name);
			if (document) 
				{
					documentStream = new AbiWordperfectInputStream(document);
					g_object_unref(G_OBJECT (document)); // the only reference should be encapsulated within the new stream
				}
		}
	
	return documentStream;
}

librevenge::RVNGInputStream * AbiWordperfectInputStream::getSubStreamById(unsigned id)
{
	librevenge::RVNGInputStream *documentStream = NULL;
	
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_msole_new (m_input, NULL)); 
	
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_zip_new (m_input, NULL)); 
	
	if (m_ole)
		{
			GsfInput *document = gsf_infile_child_by_index(m_ole, (int)id);
			if (document) 
				{
					documentStream = new AbiWordperfectInputStream(document);
					g_object_unref(G_OBJECT (document)); // the only reference should be encapsulated within the new stream
				}
		}
	
	return documentStream;
}

long AbiWordperfectInputStream::tell()
{
	return gsf_input_tell(m_input);
}

bool AbiWordperfectInputStream::isEnd()
{
	return gsf_input_eof(m_input);
}

// This should probably be defined in pt_Types.h
static const UT_uint32 PT_MAX_ATTRIBUTES = 8;

ABI_ListDefinition::ABI_ListDefinition(int iOutlineHash) :
	m_iOutlineHash(iOutlineHash)
{
	for(int i=0; i<WP6_NUM_LIST_LEVELS; i++) 
	{
		m_iListIDs[i] = 0;
		m_listTypes[i] = BULLETED_LIST;
		m_iListNumbers[i] = 0;
		m_listLeftOffset[i] = 0.0f;
		m_listMinLabelWidth[i] = 0.0f;
	}
}

void ABI_ListDefinition::setListType(const int level, const char type)
{	
	switch (type)
	{
	case '1':
		m_listTypes[level-1] = NUMBERED_LIST;
		break;
	case 'a':
		m_listTypes[level-1] = LOWERCASE_LIST;
		break;
	case 'A':
		m_listTypes[level-1] = UPPERCASE_LIST;
		break;
	case 'i':
		m_listTypes[level-1] = LOWERROMAN_LIST;
		break;
	case 'I':
		m_listTypes[level-1] = UPPERROMAN_LIST;
		break;
	}
}

#define X_CheckDocumentError(v) if (!v) { UT_DEBUGMSG(("X_CheckDocumentError: %d\n", __LINE__)); }

IE_Imp_WordPerfect_Sniffer::IE_Imp_WordPerfect_Sniffer()
	: IE_ImpSniffer(IE_MIMETYPE_WP_6)
{
}

IE_Imp_WordPerfect_Sniffer::~IE_Imp_WordPerfect_Sniffer()
{
}

// supported suffixes
static IE_SuffixConfidence IE_Imp_WordPerfect_Sniffer__SuffixConfidence[] = {
	{ "wpd", 	UT_CONFIDENCE_PERFECT 	},
	{ "wp", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_WordPerfect_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_WordPerfect_Sniffer__SuffixConfidence;
}

UT_Confidence_t IE_Imp_WordPerfect_Sniffer::recognizeContents (GsfInput * input)
{
	AbiWordperfectInputStream gsfInput(input);

	libwpd::WPDConfidence confidence = libwpd::WPDocument::isFileFormatSupported(&gsfInput);
	
	switch (confidence)
	{
		case libwpd::WPD_CONFIDENCE_NONE:
			return UT_CONFIDENCE_ZILCH;
		case libwpd::WPD_CONFIDENCE_EXCELLENT:
			return UT_CONFIDENCE_PERFECT;
		default:
			return UT_CONFIDENCE_ZILCH;
	}
}

UT_Error IE_Imp_WordPerfect_Sniffer::constructImporter (PD_Document * pDocument,
							IE_Imp ** ppie)
{
	*ppie = new IE_Imp_WordPerfect(pDocument);
	return UT_OK;
}

bool IE_Imp_WordPerfect_Sniffer::getDlgLabels  (const char ** pszDesc,
						const char ** pszSuffixList,
						IEFileType * ft)
{
	*pszDesc = "WordPerfect (.wpd, .wp)";
	*pszSuffixList = "*.wpd; *.wp";
	*ft = getFileType();
	return true;
}

/****************************************************************************/
/****************************************************************************/

IE_Imp_WordPerfect::IE_Imp_WordPerfect(PD_Document * pDocument)
  : IE_Imp (pDocument),
	m_leftPageMargin(1.0f),
	m_rightPageMargin(1.0f),
	m_leftSectionMargin(0.0f),
	m_rightSectionMargin(0.0f),
	m_sectionColumnsCount(0),
	m_headerId(-1),
	m_footerId(-1),
	m_nextFreeId(0),
	m_leftMarginOffset(0.0f),
	m_rightMarginOffset(0.0f),
	m_pCurrentListDefinition(NULL),
	m_bParagraphChanged(false),
	m_bParagraphInSection(false),
	m_bInSection(false),
	m_bSectionChanged(false),
	m_bRequireBlock(false),
	m_iCurrentListLevel(0),
	m_bInCell(false),
	m_bHdrFtrOpenCount(0)
{
}

IE_Imp_WordPerfect::~IE_Imp_WordPerfect()
{
	//UT_HASH_PURGEDATA(ABI_ListDefinition *,&m_listStylesHash,delete); 
}

UT_Error IE_Imp_WordPerfect::_loadFile(GsfInput * input)
{
	AbiWordperfectInputStream gsfInput(input);
	libwpd::WPDResult error = libwpd::WPDocument::parse(&gsfInput, static_cast<librevenge::RVNGTextInterface *>(this), NULL);

	if (error != libwpd::WPD_OK)
	{
		UT_DEBUGMSG(("AbiWordPerfect: ERROR: %i!\n", (int)error));
		return UT_IE_IMPORTERROR;
	}

	return UT_OK;
}

void IE_Imp_WordPerfect::pasteFromBuffer (PD_DocumentRange *, 
					  unsigned char *, unsigned int, const char *)
{
	// nada
}

void IE_Imp_WordPerfect::setDocumentMetaData(const librevenge::RVNGPropertyList &propList)
{
	if (propList["dc:author"])
		getDoc()->setMetaDataProp(PD_META_KEY_CREATOR, propList["dc:author"]->getStr().cstr());
	if (propList["dc:subject"])
		getDoc()->setMetaDataProp(PD_META_KEY_SUBJECT, propList["dc:subject"]->getStr().cstr());
	if (propList["dc:publisher"])
		getDoc()->setMetaDataProp(PD_META_KEY_PUBLISHER, propList["dc:publisher"]->getStr().cstr());
	if (propList["dc:type"])
		getDoc()->setMetaDataProp(PD_META_KEY_TYPE, propList["dc:category"]->getStr().cstr());
	if (propList["librevenge:keywords"])
		getDoc()->setMetaDataProp(PD_META_KEY_KEYWORDS, propList["librevenge:keywords"]->getStr().cstr());
	if (propList["dc:language"])
		getDoc()->setMetaDataProp(PD_META_KEY_LANGUAGE, propList["dc:language"]->getStr().cstr());
	if (propList["librevenge:abstract"])
		getDoc()->setMetaDataProp(PD_META_KEY_DESCRIPTION, propList["librevenge:abstract"]->getStr().cstr());
}

void IE_Imp_WordPerfect::startDocument(const librevenge::RVNGPropertyList & /* propList */)
{
	UT_DEBUGMSG(("AbiWordPerfect: startDocument\n"));
}

void IE_Imp_WordPerfect::endDocument()
{
	UT_DEBUGMSG(("AbiWordPerfect: endDocument\n"));
}

void IE_Imp_WordPerfect::openPageSpan(const librevenge::RVNGPropertyList &propList)
{
	if (m_bHdrFtrOpenCount) return; // HACK
	UT_DEBUGMSG(("AbiWordPerfect: openPageSpan\n"));
	
	float marginLeft = 1.0f, marginRight = 1.0f;

	if (propList["fo:margin-left"])
		marginLeft = propList["fo:margin-left"]->getDouble();
	if (propList["fo:margin-right"])
		marginRight = propList["fo:margin-right"]->getDouble();

	if (marginLeft != m_leftPageMargin || marginRight != m_rightPageMargin /* || */
		/* marginTop != m_marginBottom || marginBottom != m_marginBottom */ )
		m_bSectionChanged = true; // margin properties are section properties in AbiWord

	m_leftPageMargin = marginLeft;
	m_rightPageMargin = marginRight;
		
}

void IE_Imp_WordPerfect::openHeader(const librevenge::RVNGPropertyList & /*propList*/)
{
	m_bHdrFtrOpenCount++;
	
	/*
	TODO: THIS CODE IS NOT!!!! USEFULL! - DON'T TOUCH IT - MARCM
	UT_String propBuffer;

	switch (headerFooterType)
	{
		case HEADER:
			m_headerId = m_nextFreeId;
			UT_String_sprintf(propBuffer,"id:%d; listid:0; parentid=0; type=header", m_headerId);
			break;
		case FOOTER:
			m_footerId = m_nextFreeId;
			UT_String_sprintf(propBuffer,"id:%d; listid:0; parentid=0; type=footer", m_footerId);
			break;
		default:
			UT_ASSERT(SHOULD_NOT_HAPPEN);
			break;
	}
 
	const gchar* propsArray[3];
	propsArray[0] = "props";
	propsArray[1] = propBuffer.c_str();
	propsArray[2] = NULL;	
	
    X_CheckDocumentError(appendStrux(PTX_Section, propsArray));
	m_bInSection = true;
	m_bSectionChanged = false;*/
}

void IE_Imp_WordPerfect::closeHeader()
{
	m_bHdrFtrOpenCount--;
	/*
	TODO: THIS CODE IS NOT!!!! USEFULL! - DON'T TOUCH IT - MARCM
	m_nextFreeId++;
	*/
}

void IE_Imp_WordPerfect::openFooter(const librevenge::RVNGPropertyList & /*propList*/)
{
	m_bHdrFtrOpenCount++;
	// see above comments re: openHeader
}

void IE_Imp_WordPerfect::closeFooter()
{
	m_bHdrFtrOpenCount--;
	// see above comments re: closeHeader
}

void IE_Imp_WordPerfect::openParagraph(const librevenge::RVNGPropertyList &propList)
{
	if (m_bHdrFtrOpenCount) return; // HACK
	UT_DEBUGMSG(("AbiWordPerfect: openParagraph()\n"));
	// for now, we always append these options
	float marginTop = 0.0f, marginBottom = 0.0f;
	float marginLeft = 0.0f, marginRight = 0.0f, textIndent = 0.0f;
	if (propList["fo:margin-top"])
	    marginTop = propList["fo:margin-top"]->getDouble();
	if (propList["fo:margin-bottom"])
	    marginBottom = propList["fo:margin-bottom"]->getDouble();
	if (propList["fo:margin-left"])
	    marginLeft = propList["fo:margin-left"]->getDouble();
	if (propList["fo:margin-right"])
	    marginRight = propList["fo:margin-right"]->getDouble();
	if (propList["fo:text-indent"])
	    textIndent = propList["fo:text-indent"]->getDouble();

	m_topMargin = marginTop;
	m_bottomMargin = marginBottom;
	m_leftMarginOffset = marginLeft;
	m_rightMarginOffset = marginRight;
	m_textIndent = textIndent;

	UT_String propBuffer;
	propBuffer += "text-align:";
	if (propList["fo:text-align"])
	{
		// AbiWord follows xsl:fo, except here, for some reason..
		if (propList["fo:text-align"]->getStr() == "end")
			propBuffer += "right";
		else
			propBuffer += propList["fo:text-align"]->getStr().cstr();
	}
	else
		propBuffer += "left";

	float lineSpacing = 1.0f;
	if (propList["fo:line-height"])
		lineSpacing = propList["fo:line-height"]->getDouble();
	
	UT_String tmpBuffer;
	UT_String_sprintf(tmpBuffer, "; margin-top:%dpt; margin-bottom:%dpt; margin-left:%.4fin; margin-right:%.4fin; text-indent:%.4fin; line-height:%.4f",
		(int)(m_topMargin*72), (int)(m_bottomMargin*72), m_leftMarginOffset, m_rightMarginOffset, m_textIndent, lineSpacing);
	propBuffer += tmpBuffer;
	
	const librevenge::RVNGPropertyListVector *tabStops = propList.child("style:tab-stops");
	
	if (tabStops && tabStops->count()) // Append the tabstop information
	{
		propBuffer += "; tabstops:";
		tmpBuffer = "";
		librevenge::RVNGPropertyListVector::Iter i(*tabStops);
		for (i.rewind(); i.next();)
		{
			propBuffer += tmpBuffer;
			if (i()["style:position"])
			{
				UT_String_sprintf(tmpBuffer, "%.4fin", i()["style:position"]->getDouble());
				propBuffer += tmpBuffer;
			}

			if (i()["style:type"])
				if (i()["style:type"]->getStr() == "right")
					propBuffer += "/R";
				else if (i()["style:type"]->getStr() == "center")
					propBuffer += "/C";
				else if (i()["style:type"]->getStr() == "char")
					propBuffer += "/D";
				else
					propBuffer += "/L";
			else // Left aligned is default
				propBuffer += "/L";

			if (i()["style:leader-text"])
				if (i()["style:leader-text"]->getStr() == "-")
					propBuffer += "2";
				else if (i()["style:leader-text"]->getStr() == "_")
					propBuffer += "3";
				else // default to dot leader if the given leader is dot or is not supported by AbiWord
					propBuffer += "1";
			else
				propBuffer += "0";

			tmpBuffer = ",";
		}
	}

	

	UT_DEBUGMSG(("AbiWordPerfect: Appending paragraph properties: %s\n", propBuffer.c_str()));
	const gchar* propsArray[3];
	propsArray[0] = "props";
	propsArray[1] = propBuffer.c_str();
	propsArray[2] = NULL;
	X_CheckDocumentError(appendStrux(PTX_Block, propsArray));
	m_bRequireBlock = false;

	if (propList["fo:break-before"])
	{
		if (strcmp(propList["fo:break-before"]->getStr().cstr(), "page") == 0)
		{
			UT_UCS4Char ucs = UCS_FF;
			X_CheckDocumentError(appendSpan(&ucs,1));			
		}
		else if (strcmp(propList["fo:break-before"]->getStr().cstr(), "column") == 0)
		{
			UT_UCS4Char ucs = UCS_VTAB;
			X_CheckDocumentError(appendSpan(&ucs,1));
		}
	}
}

void IE_Imp_WordPerfect::openSpan(const librevenge::RVNGPropertyList &propList)
{
	if (m_bHdrFtrOpenCount) return; // HACK
	UT_DEBUGMSG(("AbiWordPerfect: Appending current text properties\n"));
	
	const gchar* pProps = "props";
	UT_String propBuffer;
	UT_String tempBuffer;
	
	// bold
	propBuffer += "font-weight:";
	propBuffer += (propList["fo:font-weight"] ? propList["fo:font-weight"]->getStr().cstr() : "normal");
	
	// italic
	propBuffer += "; font-style:";
	propBuffer += (propList["fo:font-style"] ? propList["fo:font-style"]->getStr().cstr() : "normal");
	
	// superscript or subscript
	if (propList["style:text-position"])
	{
		propBuffer += "; text-position:";
		if (strncmp(propList["style:text-position"]->getStr().cstr(), "super", 5) == 0)
			propBuffer += "superscript"; 
		else 
			propBuffer += "subscript";
	}

	if (propList["style:text-underline-type"] || propList["style:text-line-through-type"])
	{
		propBuffer += "; text-decoration:";
		if (propList["style:text-underline-type"])
			propBuffer += "underline ";
		if (propList["style:text-line-through-type"])
			propBuffer += "line-through";

	}
	
	if (propList["style:font-name"])
	{
		propBuffer += "; font-family:";
		propBuffer += propList["style:font-name"]->getStr().cstr();
	}

	// font face
	if (propList["fo:font-size"])
	{
		propBuffer += "; font-size:";
		propBuffer += propList["fo:font-size"]->getStr().cstr();
	}

	if (propList["fo:color"])
	{
		propBuffer += "; color:";
		propBuffer += propList["fo:color"]->getStr().cstr();
	}

	if (propList["fo:background-color"])
	{
		propBuffer += "; bgcolor:";
		propBuffer += propList["fo:background-color"]->getStr().cstr();
	}

	UT_DEBUGMSG(("AbiWordPerfect: Appending span format: %s\n", propBuffer.c_str()));
	const gchar* propsArray[5];
	
	propsArray[0] = pProps;
	propsArray[1] = propBuffer.c_str();
	propsArray[2] = NULL;
	X_CheckDocumentError(appendFmt(propsArray));
}

void IE_Imp_WordPerfect::openSection(const librevenge::RVNGPropertyList &propList)
{
	if (m_bHdrFtrOpenCount) return; // HACK
	UT_DEBUGMSG(("AbiWordPerfect: openSection\n"));

	float marginLeft = 0.0f, marginRight = 0.0f;
	const librevenge::RVNGPropertyListVector *columns = propList.child("style:columns");
	int columnsCount = ((!columns || !columns->count()) ? 1 : columns->count());

	// TODO: support spaceAfter
	if (propList["fo:start-indent"])
		marginLeft = propList["fo:start-indent"]->getDouble();
	if (propList["fo:end-indent"])
		marginRight = propList["fo:end-indent"]->getDouble();

	if (marginLeft != m_leftSectionMargin || marginRight != m_rightSectionMargin || m_sectionColumnsCount != columnsCount)
		m_bSectionChanged = true;

	m_leftSectionMargin = marginLeft;
	m_rightSectionMargin = marginRight;
	m_sectionColumnsCount = columnsCount;
	
	_appendSection(columnsCount, m_leftPageMargin + m_leftSectionMargin, m_rightPageMargin + m_rightSectionMargin); 
}

void IE_Imp_WordPerfect::insertTab()
{
	if (m_bHdrFtrOpenCount) return; // HACK
	UT_DEBUGMSG(("AbiWordPerfect: insertTab\n"));

	UT_UCS4Char ucs = UCS_TAB;
	X_CheckDocumentError(appendSpan(&ucs,1));	
}

void IE_Imp_WordPerfect::insertText(const librevenge::RVNGString &text)
{
	if (m_bHdrFtrOpenCount) return; // HACK
	if (text.len())
	{
		UT_DEBUGMSG(("AbiWordPerfect: insertText\n"));
		UT_UCS4String ucs4(text.cstr());
		X_CheckDocumentError(appendSpan(ucs4.ucs4_str(), ucs4.length()));
	}
}

void IE_Imp_WordPerfect::insertSpace()
{
	if (m_bHdrFtrOpenCount) return; // HACK
	UT_DEBUGMSG(("AbiWordPerfect: insertSpace\n"));

	UT_UCS4Char ucs = UCS_SPACE;
	X_CheckDocumentError(appendSpan(&ucs,1));	
}

void IE_Imp_WordPerfect::insertLineBreak()
{
	if (m_bHdrFtrOpenCount) return; // HACK
	UT_DEBUGMSG(("AbiWordPerfect: insertLineBreak\n"));

	UT_UCSChar ucs = UCS_LF;
	X_CheckDocumentError(appendSpan(&ucs,1));
}


void IE_Imp_WordPerfect::openOrderedListLevel(const librevenge::RVNGPropertyList &propList)
{
	if (m_bHdrFtrOpenCount) return; // HACK
	UT_DEBUGMSG(("AbiWordPerfect: openOrderedListLevel\n"));
	
	int listID = 0, startingNumber = 0, level = 1;
	char listType = '1';
	UT_UTF8String textBeforeNumber, textAfterNumber;
	float listLeftOffset = 0.0f;
	float listMinLabelWidth = 0.0f;
	
	if (propList["librevenge:id"])
		listID = propList["librevenge:id"]->getInt();
	if (propList["text:start-value"])
		startingNumber = propList["text:start-value"]->getInt();
	if (propList["librevenge:level"])
		level = propList["librevenge:level"]->getInt();
	if (propList["style:num-prefix"])
		textBeforeNumber += propList["style:num-prefix"]->getStr().cstr();
	if (propList["style:num-suffix"])
		textAfterNumber += propList["style:num-suffix"]->getStr().cstr();
	if (propList["style:num-format"])
		listType = propList["style:num-format"]->getStr().cstr()[0];
	if (propList["text:space-before"])
		listLeftOffset = propList["text:space-before"]->getDouble();
	if (propList["text:min-label-width"])
		listMinLabelWidth = propList["text:min-label-width"]->getDouble();

	if (!m_pCurrentListDefinition || 
		m_pCurrentListDefinition->getOutlineHash() != listID ||
		(m_pCurrentListDefinition->getLevelNumber(level) != startingNumber && 
		 level == 1))
	{
		if (m_pCurrentListDefinition)
			delete (m_pCurrentListDefinition);

		m_pCurrentListDefinition = new ABI_ListDefinition(listID);
	}
	
	if (!m_pCurrentListDefinition->getListID(level))
	{
		m_pCurrentListDefinition->setListType(level, listType);
		m_pCurrentListDefinition->setListID(level, UT_rand());
		m_pCurrentListDefinition->setListLeftOffset(level, listLeftOffset);
		m_pCurrentListDefinition->setListMinLabelWidth(level, listMinLabelWidth);
		_updateDocumentOrderedListDefinition(m_pCurrentListDefinition, level, listType, textBeforeNumber, textAfterNumber, startingNumber);
	}

	m_iCurrentListLevel++;
}

void IE_Imp_WordPerfect::closeOrderedListLevel()
{
	if (m_bHdrFtrOpenCount) return; // HACK
	UT_DEBUGMSG(("AbiWordPerfect: closeOrderedListLevel (level: %i)\n", m_iCurrentListLevel));
	UT_ASSERT(m_iCurrentListLevel > 0); 
	
	// every time we close a list level, the level above it is normally renumbered to start at "1"
	// again. this code takes care of that.
	if (m_iCurrentListLevel < (WP6_NUM_LIST_LEVELS-1))
		m_pCurrentListDefinition->setLevelNumber(m_iCurrentListLevel + 1, 0);
	
	m_iCurrentListLevel--;
}

void IE_Imp_WordPerfect::openUnorderedListLevel(const librevenge::RVNGPropertyList &propList)
{
	if (m_bHdrFtrOpenCount) return; // HACK
	UT_DEBUGMSG(("AbiWordPerfect: openUNorderedListLevel\n"));
	
	int listID = 0, level = 1;
	librevenge::RVNGString textBeforeNumber, textAfterNumber;
	float listLeftOffset = 0.0f;
	float listMinLabelWidth = 0.0f;
	
	if (propList["librevenge:id"])
		listID = propList["librevenge:id"]->getInt();
	if (propList["librevenge:level"])
		level = propList["librevenge:level"]->getInt();
	if (propList["text:space-before"])
		listLeftOffset = propList["text:space-before"]->getDouble();
	if (propList["text:min-label-width"])
		listMinLabelWidth = propList["text:min-label-width"]->getDouble();

	if (!m_pCurrentListDefinition || m_pCurrentListDefinition->getOutlineHash() != listID)
	{
		if (m_pCurrentListDefinition)
			delete (m_pCurrentListDefinition);
		
		m_pCurrentListDefinition = new ABI_ListDefinition(listID);
	}

	if (!m_pCurrentListDefinition->getListID(level))
	{
		m_pCurrentListDefinition->setListID(level, UT_rand());
		m_pCurrentListDefinition->setListLeftOffset(level, listLeftOffset);
		m_pCurrentListDefinition->setListMinLabelWidth(level, listMinLabelWidth);
		_updateDocumentUnorderedListDefinition(m_pCurrentListDefinition, level);
	}

	m_iCurrentListLevel++;
}

void IE_Imp_WordPerfect::closeUnorderedListLevel()
{
	if (m_bHdrFtrOpenCount) return; // HACK
	UT_DEBUGMSG(("AbiWordPerfect: closeUnorderedListLevel (level: %i)\n", m_iCurrentListLevel));
	UT_ASSERT(m_iCurrentListLevel > 0); 
	
	m_iCurrentListLevel--;
}

// ASSUMPTION: We assume that unordered lists will always pass a number of "0". unpredictable behaviour
// may result otherwise
void IE_Imp_WordPerfect::openListElement(const librevenge::RVNGPropertyList &propList)
{
	if (m_bHdrFtrOpenCount) return; // HACK
	UT_DEBUGMSG(("AbiWordPerfect: openListElement\n"));
	
	UT_ASSERT(m_pCurrentListDefinition); // FIXME: ABI_LISTS_IMPORT throw an exception back to libwpd, if this fails
	
	// Paragraph properties for our list element
	UT_String szListID;
	UT_String szParentID;
	UT_String szLevel;
	UT_String_sprintf(szListID,"%d",m_pCurrentListDefinition->getListID(m_iCurrentListLevel));
	if (m_iCurrentListLevel > 1) 
		UT_String_sprintf(szParentID,"%d", m_pCurrentListDefinition->getListID((m_iCurrentListLevel-1)));
	else
		UT_String_sprintf(szParentID,"0"); 
	UT_String_sprintf(szLevel,"%d", m_iCurrentListLevel);
	
	const gchar* listAttribs[PT_MAX_ATTRIBUTES*2 + 1];
	UT_uint32 attribsCount=0;
	
	listAttribs[attribsCount++] = PT_LISTID_ATTRIBUTE_NAME;
	listAttribs[attribsCount++] = szListID.c_str();
	listAttribs[attribsCount++] = PT_PARENTID_ATTRIBUTE_NAME;
	listAttribs[attribsCount++] = szParentID.c_str();
	listAttribs[attribsCount++] = PT_LEVEL_ATTRIBUTE_NAME;
	listAttribs[attribsCount++] = szLevel.c_str();
	
	// Now handle the Abi List properties
	UT_String propBuffer;
	UT_String tempBuffer;
	UT_String_sprintf(tempBuffer,"list-style:%i;", m_pCurrentListDefinition->getListType(m_iCurrentListLevel));
	propBuffer += tempBuffer;

#if 0
	// FIXME: writing the list delimiter is kind of tricky and silly (because wordperfect wants to define
	// it within the document, while abi wants to (sensibly probably) define it in the list definition)
	// (we reset it each time but only for numbered lists)
	if (listDefinition->isLevelNumbered(m_iCurrentListLevel)) 
	{  
		UT_DEBUGMSG(("WordPerfect: Appending this list delim: %s\n", m_rightListDelim.c_str()));
		listDefinition->setListRightDelimText(m_iCurrentListLevel, m_rightListDelim.c_str());
		X_CheckWordPerfectError(_updateDocumentListDefinition(listDefinition, m_iCurrentListLevel));
	}
#endif

	if (m_pCurrentListDefinition->getListType(m_iCurrentListLevel) == BULLETED_LIST)
		UT_String_sprintf(tempBuffer, "field-font:Symbol; ");
	else
		UT_String_sprintf(tempBuffer, "field-font:NULL; ");
	
	m_pCurrentListDefinition->incrementLevelNumber(m_iCurrentListLevel);
	
	propBuffer += tempBuffer;
	UT_String_sprintf(tempBuffer, "start-value:%i; ", 1);
	propBuffer += tempBuffer;

	UT_String_sprintf(tempBuffer, "margin-left:%.4fin; ", m_pCurrentListDefinition->getListLeftOffset(m_iCurrentListLevel)
					+ m_pCurrentListDefinition->getListMinLabelWidth(m_iCurrentListLevel)
					- (propList["fo:text-indent"] ? propList["fo:text-indent"]->getDouble() : 0.0f));
	propBuffer += tempBuffer;
	UT_String_sprintf(tempBuffer, "text-indent:%.4fin", - m_pCurrentListDefinition->getListMinLabelWidth(m_iCurrentListLevel)
					+ (propList["fo:text-indent"] ? propList["fo:text-indent"]->getDouble() : 0.0f));
	propBuffer += tempBuffer;

	listAttribs[attribsCount++] = PT_PROPS_ATTRIBUTE_NAME;
	listAttribs[attribsCount++] = propBuffer.c_str();
	listAttribs[attribsCount++] = NULL;
	
	X_CheckDocumentError(appendStrux(PTX_Block, listAttribs));
	m_bRequireBlock = false;
	
	// hang text off of a list label
	getDoc()->appendFmtMark();
	UT_DEBUGMSG(("WordPerfect: LISTS - Appended a list tag def'n (character props)\n"));
	
	// append a list field label
	const gchar* fielddef[5];
	fielddef[0] ="type";
	fielddef[1] = "list_label";
	fielddef[2] = NULL;
	X_CheckDocumentError(appendObject(PTO_Field,fielddef));
	UT_DEBUGMSG(("WordPerfect: LISTS - Appended a field def'n\n"));
	
	// insert a tab
	UT_UCS4Char ucs = UCS_TAB;
	X_CheckDocumentError(appendSpan(&ucs,1));
}

void IE_Imp_WordPerfect::openFootnote(const librevenge::RVNGPropertyList & /*propList*/)
{
	if (m_bHdrFtrOpenCount) return; // HACK

	if (!m_bInSection)
	{
		X_CheckDocumentError(appendStrux(PTX_Section, NULL));
		X_CheckDocumentError(appendStrux(PTX_Block,NULL));
		m_bInSection = true;
	}

	const gchar** propsArray = NULL;
	
	UT_String footnoteId;
	UT_String_sprintf(footnoteId,"%i",UT_rand());	
	
	propsArray = static_cast<const gchar **>(UT_calloc(7, sizeof(gchar *)));
	propsArray [0] = "type";
	propsArray [1] = "footnote_ref";
	propsArray [2] = "footnote-id";
	propsArray [3] = footnoteId.c_str();
	propsArray [4] = NULL;
	propsArray [5] = NULL;
	propsArray [6] = NULL;
	X_CheckDocumentError(appendObject(PTO_Field, propsArray));

	const gchar * attribs[3] ={"footnote-id", footnoteId.c_str(), NULL};
	X_CheckDocumentError(appendStrux(PTX_SectionFootnote,attribs));
	
	X_CheckDocumentError(appendStrux(PTX_Block,NULL));
	m_bRequireBlock = false;

	propsArray = static_cast<const gchar **>(UT_calloc(7, sizeof(gchar *)));
	propsArray [0] = "type";
	propsArray [1] = "footnote_anchor";
	propsArray [2] = "footnote-id";
	propsArray [3] = footnoteId.c_str();
	propsArray [4] = NULL;
	propsArray [5] = NULL;
	propsArray [6] = NULL;
	X_CheckDocumentError(appendObject(PTO_Field, propsArray));
}

void IE_Imp_WordPerfect::closeFootnote()
{
	if (m_bHdrFtrOpenCount) return; // HACK
	X_CheckDocumentError(appendStrux(PTX_EndFootnote,NULL));
}

void IE_Imp_WordPerfect::openEndnote(const librevenge::RVNGPropertyList & /*propList*/)
{
	if (m_bHdrFtrOpenCount) return; // HACK
	const gchar** propsArray = NULL;
	
	UT_String endnoteId;
	UT_String_sprintf(endnoteId,"%i",UT_rand());	
	
	propsArray = static_cast<const gchar **>(UT_calloc(7, sizeof(gchar *)));
	propsArray [0] = "type";
	propsArray [1] = "endnote_ref";
	propsArray [2] = "endnote-id";
	propsArray [3] = endnoteId.c_str();
	propsArray [4] = NULL;
	propsArray [5] = NULL;
	propsArray [6] = NULL;
	X_CheckDocumentError(appendObject(PTO_Field, propsArray));

	const gchar * attribs[3] ={"endnote-id", endnoteId.c_str(), NULL};
	X_CheckDocumentError(appendStrux(PTX_SectionEndnote,attribs));
	
	X_CheckDocumentError(appendStrux(PTX_Block,NULL));
	m_bRequireBlock = false;

	propsArray = static_cast<const gchar **>(UT_calloc(7, sizeof(gchar *)));
	propsArray [0] = "type";
	propsArray [1] = "endnote_anchor";
	propsArray [2] = "endnote-id";
	propsArray [3] = endnoteId.c_str();
	propsArray [4] = NULL;
	propsArray [5] = NULL;
	propsArray [6] = NULL;
	X_CheckDocumentError(appendObject(PTO_Field, propsArray));
}

void IE_Imp_WordPerfect::closeEndnote()
{
	if (m_bHdrFtrOpenCount) return; // HACK
	X_CheckDocumentError(appendStrux(PTX_EndEndnote,NULL));
}

void IE_Imp_WordPerfect::openTable(const librevenge::RVNGPropertyList &propList)
{
	if (m_bHdrFtrOpenCount) return; // HACK
	// TODO: handle 'marginLeftOffset' and 'marginRightOffset'
	UT_DEBUGMSG(("AbiWordPerfect: openTable\n"));
	
	UT_String propBuffer;

	if (propList["table:align"])
	{
		// no need to support left: default behaviour

		//if (strcmp(propList["table:align"]->getStr().cstr(), "right"))
		// abiword does not support this I think
		//if (strcmp(propList["table:align"]->getStr().cstr(), "center"))
		// abiword does not support this I think
		//if (strcmp(propList["table:align"]->getStr().cstr(), "margins"))
		// abiword does not support this I think
		if (strcmp(propList["table:align"]->getStr().cstr(), "margins"))
		{
			if (propList["fo:margin-left"])
				UT_String_sprintf(propBuffer, "table-column-leftpos:%s; ", propList["fo:margin-left"]->getStr().cstr());
		}
	}
	
	const librevenge::RVNGPropertyListVector *columns = propList.child("librevenge:table-columns");
	if (columns)
	{
		propBuffer += "table-column-props:";
		librevenge::RVNGPropertyListVector::Iter i(*columns);
		for (i.rewind(); i.next();)
		{
			UT_String tmpBuffer;
			if (i()["style:column-width"])
				UT_String_sprintf(tmpBuffer, "%s/", i()["style:column-width"]->getStr().cstr());
			propBuffer += tmpBuffer;
		}
	}

	const gchar* propsArray[3];
	propsArray[0] = "props";
	propsArray[1] = propBuffer.c_str();
	propsArray[2] = NULL;

	X_CheckDocumentError(appendStrux(PTX_SectionTable, propsArray));
}

void IE_Imp_WordPerfect::openTableRow(const librevenge::RVNGPropertyList & /*propList*/)
{
	if (m_bHdrFtrOpenCount) return; // HACK
	UT_DEBUGMSG(("AbiWordPerfect: openRow\n"));
	if (m_bInCell)
	{		
		X_CheckDocumentError(appendStrux(PTX_EndCell, NULL));
	}
	
	m_bInCell = false;
}

void IE_Imp_WordPerfect::openTableCell(const librevenge::RVNGPropertyList &propList)
{
	if (m_bHdrFtrOpenCount) return; // HACK
	int col =0,  row = 0, colSpan = 0, rowSpan = 0;
	if (propList["librevenge:column"])
		col = propList["librevenge:column"]->getInt();
	if (propList["librevenge:row"])
		row = propList["librevenge:row"]->getInt();
	if (propList["table:number-columns-spanned"])
		colSpan = propList["table:number-columns-spanned"]->getInt();
	if (propList["table:number-rows-spanned"])
		rowSpan = propList["table:number-rows-spanned"]->getInt();

	UT_DEBUGMSG(("AbiWordPerfect: openCell(col: %d, row: %d, colSpan: %d, rowSpan: %d\n", col, row, colSpan, rowSpan));
	if (m_bInCell)
	{
		X_CheckDocumentError(appendStrux(PTX_EndCell, NULL));
	}
	
	UT_String propBuffer;
	UT_String_sprintf(propBuffer, "left-attach:%d; right-attach:%d; top-attach:%d; bot-attach:%d",
					col, col+colSpan, row, row+rowSpan);
	
	UT_String borderStyle;
	// we only support bg-style:1 for now
	bool borderLeftSolid = false;
	bool borderRightSolid = false;
	bool borderTopSolid = false;
	bool borderBottomSolid = false;
	if (propList["fo:border-left"])
		borderLeftSolid = strncmp(propList["fo:border-left"]->getStr().cstr(), "0.0inch", 7);
	if (propList["fo:border-right"])
		borderRightSolid = strncmp(propList["fo:border-right"]->getStr().cstr(), "0.0inch", 7);
	if (propList["fo:border-top"])
		borderTopSolid = strncmp(propList["fo:border-top"]->getStr().cstr(), "0.0inch", 7);
	if (propList["fo:border-bottom"])
		borderBottomSolid = strncmp(propList["fo:border-bottom"]->getStr().cstr(), "0.0inch", 7);

	UT_String_sprintf(borderStyle, "; left-style:%s; right-style:%s; top-style:%s; bot-style:%s", 
					  (borderLeftSolid ? "solid" : "none"),
					  (borderRightSolid ? "solid" : "none"), 
					  (borderTopSolid ? "solid" : "none"), 
					  (borderBottomSolid ? "solid" : "none"));
	propBuffer += borderStyle;
		
	// we only support bg-style:1 for now
	if (propList["fo:background-color"])
	{
		UT_String bgCol;
		UT_String_sprintf(bgCol, "; bg-style:1; background-color:%s", &(propList["fo:background-color"]->getStr().cstr()[1]));
		propBuffer += bgCol;
	}
	
	UT_DEBUGMSG(("AbiWordPerfect: Inserting a Cell definition: %s\n", propBuffer.c_str()));
	
	const gchar* propsArray[3];
	propsArray[0] = "props";
	propsArray[1] = propBuffer.c_str();
	propsArray[2] = NULL;
	
	X_CheckDocumentError(appendStrux(PTX_SectionCell, propsArray));
	m_bInCell = true;
}

void IE_Imp_WordPerfect::closeTable()
{
	if (m_bHdrFtrOpenCount) return; // HACK
	UT_DEBUGMSG(("AbiWordPerfect: Closing table\n"));
	
	if (m_bInCell)
	{
		X_CheckDocumentError(appendStrux(PTX_EndCell, NULL));
	}
	X_CheckDocumentError(appendStrux(PTX_EndTable, NULL));
	m_bInCell = false;
	
	// we need to open a new paragraph after a table, since libwpd does NOT do it
	// FIXME: NEED TO PASS THE CURRENT PROPERTIES INSTEAD OF NULL
	// NOTE: THIS SUCKS.........
	X_CheckDocumentError(appendStrux(PTX_Block, NULL));
	m_bRequireBlock = false;
}

UT_Error IE_Imp_WordPerfect::_appendSection(int numColumns, const float marginLeft, const float marginRight)
{
	UT_DEBUGMSG(("AbiWordPerfect: Appending section\n"));
	
	UT_String myProps("") ;
	UT_LocaleTransactor lt(LC_NUMERIC, "C");
	myProps += UT_String_sprintf("columns:%d; page-margin-left:%.4fin; page-margin-right:%.4fin", numColumns, marginLeft, marginRight);

	if(m_bInSection && m_bRequireBlock) // AbiWord will hang on an empty <section>
	{
		X_CheckDocumentError(appendStrux(PTX_Block,NULL));
	}
	
	const gchar * propsArray[3];
	propsArray[0] = "props";
	propsArray[1] = myProps.c_str();
	propsArray[2] = NULL ;
	X_CheckDocumentError(appendStrux(PTX_Section, propsArray));
	
	m_bInSection = true;
	m_bRequireBlock = true;

	m_bSectionChanged = false;
	
	return UT_OK;
}

// NB: AbiWord-2.0 doesn't properly support nested lists with different nested styles: only "1" style
// really looks proper. We hack around this be only using the style given at level "1"
// NB: AbiWord-2.0 doesn't properly support setting list delimeters at levels greater than 1,
// we hack around this by using only "plain" (e.g.: NULL) list delimeters on levels greater than 1.
UT_Error IE_Imp_WordPerfect::_updateDocumentOrderedListDefinition(ABI_ListDefinition *pListDefinition, int iLevel, 
																  const char /*listType*/, const UT_UTF8String &sTextBeforeNumber, 
																  const UT_UTF8String &sTextAfterNumber, int iStartingNumber)
{
	UT_DEBUGMSG(("AbiWordPerfect: Updating document list definition (iLevel: %i)\n", iLevel));

	if (iLevel > 1) {
        UT_DEBUGMSG(("WLACH: Parent's list id is.. %i\n", pListDefinition->getListID((iLevel-1))));
    }

	// finally, set the document's list identification info..
	fl_AutoNum * pAuto = getDoc()->getListByID(pListDefinition->getListID(iLevel));
	// not in document yet, we should create a list for it
	if (pAuto == NULL) 
	{	
		UT_DEBUGMSG(("AbiWordPerfect: pAuto is NULL: creating a list\n"));
		if (iLevel > 1) 
		{	
			pAuto = new fl_AutoNum(pListDefinition->getListID(iLevel), 
								   pListDefinition->getListID((iLevel-1)), 
								   pListDefinition->getListType(1), 
								   iStartingNumber, 
								   const_cast<gchar*>(reinterpret_cast<const gchar*>("%L")), 
								   ".", 
								   getDoc(), 
								   NULL);
		}   
		else 
		{
			UT_UTF8String sNumberingString;
			UT_UTF8String sNumber("%L", (size_t)0);
			
			sNumberingString += sTextBeforeNumber;
			sNumberingString += sNumber;
			sNumberingString += sTextAfterNumber;
	
			pAuto = new fl_AutoNum(pListDefinition->getListID(iLevel), 0, pListDefinition->getListType(iLevel), iStartingNumber, 
								   const_cast<gchar*>(reinterpret_cast<const gchar*>(sNumberingString.utf8_str())), ".", getDoc(), NULL);
		}
		getDoc()->addList(pAuto);
	}
	// we should update what we have
	else 
	{
		UT_DEBUGMSG(("AbiWordPerfect: pAuto already exists\n"));
	}

	pAuto->fixHierarchy();

	return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_updateDocumentUnorderedListDefinition(ABI_ListDefinition *pListDefinition, int iLevel)
{
	UT_DEBUGMSG(("AbiWordPerfect: Updating document list definition (iLevel: %i)\n", iLevel));
	
	// finally, set the document's list identification info..
	fl_AutoNum * pAuto = getDoc()->getListByID(pListDefinition->getListID(iLevel));
	// not in document yet, we should create a list for it
	if (pAuto == NULL) 
	{	
		UT_DEBUGMSG(("AbiWordPerfect: pAuto is NULL: creating a list\n"));
		if (iLevel > 1) 
		{	
			pAuto = new fl_AutoNum(pListDefinition->getListID(iLevel), pListDefinition->getListID((iLevel-1)), 
								   pListDefinition->getListType(1), 0, const_cast<gchar*>(reinterpret_cast<const gchar*>("%L")), ".", getDoc(), NULL);
		}   
		else
			pAuto = new fl_AutoNum(pListDefinition->getListID(iLevel), 0, pListDefinition->getListType(iLevel), 0, 
								   const_cast<gchar*>(reinterpret_cast<const gchar*>("%L")), ".", getDoc(), NULL);
		  
		getDoc()->addList(pAuto);
	}
	// we should update what we have
	else 
	{	
		UT_DEBUGMSG(("AbiWordPerfect: pAuto already exists\n"));
	}

	pAuto->fixHierarchy();

	return UT_OK;
}

#ifdef HAVE_LIBWPS

class IE_Imp_MSWorks : public IE_Imp_WordPerfect
{
public:

    IE_Imp_MSWorks(PD_Document * pDocument)
		: IE_Imp_WordPerfect(pDocument)
	{
	}

    ~IE_Imp_MSWorks()
	{
	}
    
protected:
    virtual UT_Error _loadFile(GsfInput * input)
	{
		AbiWordperfectInputStream gsfInput(input);
		libwps::WPSResult error = libwps::WPSDocument::parse(&gsfInput, static_cast<librevenge::RVNGTextInterface *>(this), NULL, NULL);

		if (error != libwps::WPS_OK)
			{
				UT_DEBUGMSG(("AbiMSWorks: ERROR: %i!\n", (int)error));
				return UT_IE_IMPORTERROR;
			}
		
		return UT_OK;
	}
};

/****************************************************************************************/
/****************************************************************************************/

IE_Imp_MSWorks_Sniffer::IE_Imp_MSWorks_Sniffer()
	: IE_ImpSniffer("application/vnd.ms-works")
{
}

IE_Imp_MSWorks_Sniffer::~IE_Imp_MSWorks_Sniffer()
{
}

// supported suffixes
static IE_SuffixConfidence IE_Imp_MSWorks_Sniffer__SuffixConfidence[] = {
	{ "wps", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_MSWorks_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_MSWorks_Sniffer__SuffixConfidence;
}

UT_Confidence_t IE_Imp_MSWorks_Sniffer::recognizeContents (GsfInput * input) 
{
	AbiWordperfectInputStream gsfInput(input);

	libwps::WPSCreator creator;
	libwps::WPSKind kind;
	bool needEncoding;
	libwps::WPSConfidence confidence = libwps::WPSDocument::isFileFormatSupported(&gsfInput, kind, creator, needEncoding);
	
	if (kind != libwps::WPS_TEXT)
		confidence = libwps::WPS_CONFIDENCE_NONE;

	switch (confidence)
	{
		case libwps::WPS_CONFIDENCE_NONE:
			return UT_CONFIDENCE_ZILCH;
		case libwps::WPS_CONFIDENCE_EXCELLENT:
			return UT_CONFIDENCE_PERFECT;
		default:
			return UT_CONFIDENCE_ZILCH;
	}
}

UT_Error IE_Imp_MSWorks_Sniffer::constructImporter (PD_Document * pDocument,
							IE_Imp ** ppie)
{
	*ppie = new IE_Imp_MSWorks(pDocument);
	return UT_OK;
}

bool IE_Imp_MSWorks_Sniffer::getDlgLabels  (const char ** pszDesc,
						const char ** pszSuffixList,
						IEFileType * ft)
{
	*pszDesc = "Microsoft Works (.wps)";
	*pszSuffixList = "*.wps";
	*ft = getFileType();
	return true;
}

#endif
