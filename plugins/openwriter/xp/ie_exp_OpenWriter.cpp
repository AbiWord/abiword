/* AbiSource Program Utilities
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2004 Robert Staudinger <robsta@stereolyzer.net>
 * Copyright (C) 2009 Hubert Figuiere
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

#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-outfile.h>
#include <gsf/gsf-outfile-zip.h>

#include <locale.h>

#include "pd_Style.h"
#include "ut_Language.h"
#include "ut_math.h"
#include "ut_std_string.h"
#include "ie_impexp_OpenWriter.h"
#include "ie_exp_OpenWriter.h"

#include "ut_debugmsg.h"

static void
oo_gsf_output_write(GsfOutput *output, size_t num_bytes, guint8 const *data)
{
  if (!gsf_output_write(output, num_bytes, data)) {
    UT_DEBUGMSG(("DOM: gsf_output_write() failed\n"));
    UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
  }
}

static void
oo_gsf_output_close(GsfOutput *output)
{
  if (!gsf_output_close(output)) {
    const GError * err = gsf_output_error(output);
    UT_DEBUGMSG(("DOM: gsf_output_close() failed\n"));
    if (err) {
      UT_DEBUGMSG(("DOM: reason: %s\n", err->message));
    }
    UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
  }
  g_object_unref(output);
}

/*****************************************************************************/
/*****************************************************************************/

IE_Exp_OpenWriter_Sniffer::IE_Exp_OpenWriter_Sniffer()
  : IE_ExpSniffer ("OpenWriter::SXW")
{
}

IE_Exp_OpenWriter_Sniffer::~IE_Exp_OpenWriter_Sniffer()
{
}

/*!
 * Recognize this suffix
 */
bool IE_Exp_OpenWriter_Sniffer::recognizeSuffix(const char * szSuffix)
{
  return (!g_ascii_strcasecmp(szSuffix,".sxw"));
}

/*!
 * Construct an importer for us
 */
UT_Error IE_Exp_OpenWriter_Sniffer::constructExporter(PD_Document * pDocument,
						      IE_Exp ** ppie)
{
  *ppie = new IE_Exp_OpenWriter (pDocument);
  return UT_OK;
}

/*!
 * Get the dialog labels
 */
bool IE_Exp_OpenWriter_Sniffer::getDlgLabels(const char ** pszDesc,
					     const char ** pszSuffixList,
					     IEFileType * ft)
{
  *pszDesc = "OpenOffice Writer (.sxw)";
  *pszSuffixList = "*.sxw";
  *ft = getFileType();
  return true;
}

/*****************************************************************************/
/*****************************************************************************/

/*!
 * Write out a message to the stream. Message is an array of content
 */
static void writeToStream (GsfOutput * stream, const char * const message [], size_t nElements)
{
    for(UT_uint32 k = 0; k < nElements; k++)
      oo_gsf_output_write(stream, strlen(message[k]), reinterpret_cast<const guint8 *>(message[k]));
}

static void writeString (GsfOutput * output, const UT_String & str)
{
   oo_gsf_output_write (output, str.length(), reinterpret_cast<const guint8*>(str.c_str()));
}

static void writeUTF8String (GsfOutput * output, const UT_UTF8String & str)
{
  oo_gsf_output_write (output, str.byteLength(), reinterpret_cast<const guint8*>(str.utf8_str()));
}

static void outputCharData (GsfOutput * output, const UT_UCSChar * data, UT_uint32 length)
{
   UT_UTF8String sBuf;
   const UT_UCSChar * pData;
   
   UT_ASSERT(sizeof(UT_Byte) == sizeof(char));
   
   sBuf.reserve(length);
   for (pData=data; (pData<data+length); /**/)
   {
      switch (*pData)
      {
      case '<':
	 sBuf += "&lt;";
	 pData++;
	 break;
	 
      case '>':
	 sBuf += "&gt;";
	 pData++;
	 break;
	 
      case '&':
	 sBuf += "&amp;";
	 pData++;
	 break;

      case UCS_LF: //line breaks
	 sBuf += "<text:line-break/>";
	 pData++;
	 break;

      case UCS_TAB:
	 sBuf += "<text:tab-stop/>";
	 pData++;
	 break;
	 
      default:
	 if (*pData < 0x20)         // Silently eat these characters.
	    pData++;
	 else
	 {
	    sBuf.appendUCS4 (pData, 1);
	    pData++;
	 }
      }
   }
   writeUTF8String (output, sBuf);
}


/*****************************************************************************/
/*****************************************************************************/

OO_WriterImpl::OO_WriterImpl(GsfOutfile *pOutfile, OO_StylesContainer *pStylesContainer) : 
   OO_ListenerImpl(), m_pStylesContainer(pStylesContainer)
{ 
	m_pContentStream = gsf_outfile_new_child (pOutfile, "content.xml", FALSE);

	static const char * const preamble [] = 
	{
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n",
		"<!DOCTYPE office:document-content PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"office.dtd\">\n",
		"<office:document-content xmlns:office=\"http://openoffice.org/2000/office\" xmlns:style=\"http://openoffice.org/2000/style\" xmlns:text=\"http://openoffice.org/2000/text\" xmlns:table=\"http://openoffice.org/2000/table\" xmlns:draw=\"http://openoffice.org/2000/drawing\" xmlns:fo=\"http://www.w3.org/1999/XSL/Format\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:number=\"http://openoffice.org/2000/datastyle\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:chart=\"http://openoffice.org/2000/chart\" xmlns:dr3d=\"http://openoffice.org/2000/dr3d\" xmlns:math=\"http://www.w3.org/1998/Math/MathML\" xmlns:form=\"http://openoffice.org/2000/form\" xmlns:script=\"http://openoffice.org/2000/script\" office:class=\"text\" office:version=\"1.0\">\n",
		"<office:script/>\n"
	};
	writeToStream(m_pContentStream, preamble, G_N_ELEMENTS(preamble));

	// in my sample files content.xml, styles.xml font-decl sections are the same
	UT_UTF8String fontDecls = "<office:font-decls>\n";
	OO_StylesWriter::addFontDecls(fontDecls, *m_pStylesContainer);
	fontDecls += "</office:font-decls>\n";
	writeUTF8String(m_pContentStream, fontDecls);
	
	writeUTF8String(m_pContentStream, "<office:automatic-styles>\n");
	
	int *styleNum = NULL;
	UT_String styleString;

	// span styles
	UT_GenericVector<int*> *tempStylesValuesList = m_pStylesContainer->enumerateSpanStyles();
	UT_GenericVector<const UT_String*> *tempStylesKeysList = m_pStylesContainer->getSpanStylesKeys();
	
	for (UT_sint32 i=0; i<tempStylesValuesList->size(); i++) 
	{
	   styleNum = tempStylesValuesList->getNthItem(i);
	   const UT_String *styleProps = tempStylesKeysList->getNthItem(i);
	   styleString = UT_String_sprintf("<style:style style:name=\"S%i\" style:family=\"%s\"><style:properties %s/></style:style>\n", *styleNum, "text", styleProps->c_str());
	   writeString(m_pContentStream, styleString);
	   UT_DEBUGMSG(("%s", styleString.c_str()));
	   UT_DEBUGMSG(("\"%s\"\n", styleProps->c_str()));
	}
	DELETEP(tempStylesKeysList);
	DELETEP(tempStylesValuesList);

	// block styles
	UT_GenericVector<const UT_String*> *tempBlockStylesKeysList = m_pStylesContainer->getBlockStylesKeys();

	for (UT_sint32 i=0; i < tempBlockStylesKeysList->size(); i++) 
	{
		const UT_String * key = tempBlockStylesKeysList->getNthItem(i);
		const UT_String * val = m_pStylesContainer->pickBlockAtts(key);
		styleString = UT_String_sprintf("<style:style style:name=\"P%i\" %s style:family=\"paragraph\">", i, val->c_str());
		styleString += UT_String_sprintf("<style:properties %s/>", key->c_str());
		styleString += UT_String_sprintf("</style:style>");
		writeString(m_pContentStream, styleString);
	}
	DELETEP(tempBlockStylesKeysList);

    //m_acc.writeStylePreamble(m_contentStream);
    
    static const char * const midsection [] = 
       {
	  "</office:automatic-styles>\n",
	  "<office:body>\n",
	  "<text:sequence-decls>\n",
	  "<text:sequence-decl text:display-outline-level=\"0\" text:name=\"Illustration\"/>\n",
	  "<text:sequence-decl text:display-outline-level=\"0\" text:name=\"Table\"/>\n",
	  "<text:sequence-decl text:display-outline-level=\"0\" text:name=\"Text\"/>\n",
	  "<text:sequence-decl text:display-outline-level=\"0\" text:name=\"Drawing\"/>\n",
	  "</text:sequence-decls>\n"
       };

    writeToStream(m_pContentStream, midsection, G_N_ELEMENTS(midsection));
}

OO_WriterImpl::~OO_WriterImpl()
{
    static const char * const postamble [] = 
      {
	  "</office:body>\n",
	  "</office:document-content>\n"
      };
    writeToStream(m_pContentStream, postamble, G_N_ELEMENTS(postamble));

    oo_gsf_output_close(m_pContentStream);
}

void OO_WriterImpl::insertText(const UT_UCSChar * data, UT_uint32 length)
{
   outputCharData(m_pContentStream, data, length);
}

void OO_WriterImpl::openBlock(const std::string & styleAtts, const std::string & styleProps, const std::string & /*font*/, bool bIsHeading)
{
	UT_UTF8String tag, props;

	if (!styleAtts.empty() && !styleProps.empty()) 
	{
		// derive automatic style
		props = UT_UTF8String_sprintf("text:style-name=\"P%i\" ", m_pStylesContainer->getBlockStyleNum(styleAtts, styleProps));		
	}
	else 
	{
		props = styleAtts.c_str();
	}

	if (bIsHeading) 
	{
		tag = "<text:h " + props + ">";
		m_blockEnd = "</text:h>\n";
	}
	else
	{
		tag = "<text:p " + props + ">";
		m_blockEnd = "</text:p>\n";
	}	

	writeUTF8String(m_pContentStream, tag);
}

void OO_WriterImpl::closeBlock()
{	
	writeUTF8String (m_pContentStream, m_blockEnd);
	m_blockEnd.clear();
}

void OO_WriterImpl::openSpan(const std::string & props, const std::string & /*font*/)
{
   UT_UTF8String spanString = UT_UTF8String_sprintf("<text:span text:style-name=\"S%i\">",  
						    m_pStylesContainer->getSpanStyleNum(props));

   writeUTF8String(m_pContentStream, spanString);
}

void OO_WriterImpl::closeSpan()
{
   UT_UTF8String endSpan = "</text:span>";
   writeUTF8String(m_pContentStream, endSpan);
}

void OO_WriterImpl::openHyperlink(const PP_AttrProp* pAP)
{
	UT_return_if_fail(pAP);

	UT_UTF8String output = "<text:a ", escape;
	const gchar* pValue = NULL;

	if(pAP->getAttribute("xlink:href",pValue) && pValue)
	{
		escape = pValue;
		escape.escapeURL();

		if(escape.length())
		{
			output+="xlink:href=\"";
			output+=escape;
			output+="\">";
			writeUTF8String(m_pContentStream, output);
		}
	}
}

void OO_WriterImpl::closeHyperlink()
{
    UT_UTF8String output = "</text:a>";
    writeUTF8String(m_pContentStream, output);
}

void OO_StylesContainer::addSpanStyle(const std::string &key)
{
   //if (!m_spanStylesHash.contains(key.utf8_str(), temp)) 
   if (!m_spanStylesHash.pick(key.c_str())) 
   {
      UT_DEBUGMSG(("OO_AccumulatorImpl: props of this type: %s not yet found, adding style at num: %zi\n", key.c_str(), (m_spanStylesHash.size()+1)));
      int *val = new int;
      char *keyCopy = new char[strlen(key.c_str())+1];
      keyCopy = strcpy(keyCopy, key.c_str());
      *val = (int)m_spanStylesHash.size()+1;
      m_spanStylesHash.insert(keyCopy, val);
   }
   else 
   {
      UT_DEBUGMSG(("OO_AccumulatorImpl: props of this type: %s already there, forget it\n", key.c_str()));
   }
}

void OO_StylesContainer::addBlockStyle(const std::string & styleAtts, const std::string & styleProps)
{
   if (!m_blockAttsHash.pick(styleProps.c_str())) 
   {
      UT_DEBUGMSG(("OO_AccumulatorImpl: block atts of this type: '%s' not yet found, adding style at pos: '%s'\n", styleAtts.c_str(), (styleProps.c_str())));
      UT_String *val = new UT_String(styleAtts);
      const char *key = g_strdup(styleProps.c_str());
      m_blockAttsHash.insert(key, val);
   }
   else 
   {
      UT_DEBUGMSG(("OO_AccumulatorImpl: block atts of this type: '%s' already there, forget it\n", styleAtts.c_str()));
   }
}

void OO_StylesContainer::addFont(const std::string & font) 
{
	if (!m_fontsHash.pick(font.c_str()))
	{
      UT_DEBUGMSG(("OO_AccumulatorImpl: font '%s' not yet found, adding \n", font.c_str()));
      int *val = new int;
      char *keyCopy = new char[strlen(font.c_str())+1];
      keyCopy = strcpy(keyCopy, font.c_str());
      *val = (int)m_fontsHash.size()+1;
      m_fontsHash.insert(keyCopy, val);
	} 
	else 
	{
      UT_DEBUGMSG(("OO_AccumulatorImpl: font '%s' already there, forget it\n", font.c_str()));
	}
}

int OO_StylesContainer::getSpanStyleNum(const std::string &key) const
{
   if (int *val = m_spanStylesHash.pick(key.c_str())) {
      return *val;
   }
   else
      return 0;
}

int OO_StylesContainer::getBlockStyleNum(const std::string & /*styleAtts*/, 
                                         const std::string & styleProps) const
{
	UT_GenericVector<const UT_String*> *keys = m_blockAttsHash.keys();

	for (UT_sint32 i = 0; i < keys->size(); i++) 
	{
		const UT_String * key = keys->getNthItem(i);
		if (key && (*key == styleProps))
			return i;
	}

	UT_ASSERT_NOT_REACHED();
	return -1;
}

UT_GenericVector<int*> * OO_StylesContainer::enumerateSpanStyles() const
{
   return m_spanStylesHash.enumerate();
}

UT_String * OO_StylesContainer::pickBlockAtts(const UT_String *key)
{
	return m_blockAttsHash.pick(key->c_str());
}

UT_GenericVector<const UT_String*> * OO_StylesContainer::getSpanStylesKeys() const
{
   return m_spanStylesHash.keys();
}

UT_GenericVector<const UT_String*> * OO_StylesContainer::getBlockStylesKeys() const
{
   return m_blockAttsHash.keys();
}

UT_GenericVector<const UT_String*> * OO_StylesContainer::getFontsKeys() const
{
   return m_fontsHash.keys();
}

void OO_AccumulatorImpl::openSpan(const std::string & props, const std::string & font)
{
   m_pStylesContainer->addSpanStyle(props);

	if (font.size())
		m_pStylesContainer->addFont(font);
}

void OO_AccumulatorImpl::openBlock(const std::string & styleAtts, const std::string & styleProps, const std::string & font, bool /*bIsHeading*/)
{
	if (!styleAtts.empty() && !styleProps.empty()) 
	{
		// custom props, need to derive automatic style
		m_pStylesContainer->addBlockStyle(styleAtts, styleProps);
	}

	if (font.size())
		m_pStylesContainer->addFont(font);
}


OO_Listener::OO_Listener (PD_Document * pDocument, OO_ListenerImpl *pListenerImpl)
   : PL_Listener (), m_pDocument(pDocument), m_pListenerImpl(pListenerImpl), m_bInBlock(false), m_bInSpan(false), m_bInHyperlink(false)
{
}

bool OO_Listener::populate(fl_ContainerLayout* /*sfh*/,
			   const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
		case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *>(pcr);
			PT_BufIndex bi = pcrs->getBufIndex();
			PT_AttrPropIndex api = pcr->getIndexAP();
			if (api)
			{
			   _openSpan(api);
			}   
			m_pListenerImpl->insertText(m_pDocument->getPointer(bi), pcrs->getLength());
			if (api)
			{
			   _closeSpan();
			}
			return true;
		}
		case PX_ChangeRecord::PXT_InsertObject:
		{
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
			{
				case PTO_Hyperlink:
				{
					_closeSpan();
					const PP_AttrProp* pAP = NULL;
					m_pDocument->getAttrProp(api,&pAP);
					const gchar* pValue = NULL;

					if(pAP && pAP->getAttribute("xlink:href",pValue) && pValue) {
						_openHyperlink(pAP);
					} else {
						_closeHyperlink();
					}

					return true;
				}
				default:
					return true;
			}
        }
	default:
	   break;
	}
	return true;
}
   
bool OO_Listener::populateStrux(pf_Frag_Strux* /*sdh*/,
				const PX_ChangeRecord * pcr,
				fl_ContainerLayout* * psfh)
{
   const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
   *psfh = 0;							// we don't need it.
    
   switch (pcrx->getStruxType())
   {
   case PTX_Block:
   {
      _closeSpan ();
      _closeHyperlink();
      _openBlock(pcr->getIndexAP());
      break;
   }
   default: break;
   }
   
   return true;
}

bool OO_Listener::change(fl_ContainerLayout* /*sfh*/,
			 const PX_ChangeRecord * /*pcr*/)
{
   UT_ASSERT_NOT_REACHED();
   return true;
}
   
bool OO_Listener::insertStrux(fl_ContainerLayout* /*sfh*/,
			      const PX_ChangeRecord * /*pcr*/,
			      pf_Frag_Strux* /*sdh*/,
			      PL_ListenerId /*lid*/,
			      void (* /*pfnBindHandles*/)(pf_Frag_Strux* sdhNew,
						      PL_ListenerId lid,
						      fl_ContainerLayout* sfhNew))
{
   UT_ASSERT_NOT_REACHED();
   return true;
}
   
bool OO_Listener::signal(UT_uint32 /*iSignal*/)
{
   UT_ASSERT_NOT_REACHED();
   return true;
}

void OO_Listener::endDocument()
{
   _closeHyperlink();
   _closeBlock();
}

void OO_Listener::_openBlock (PT_AttrPropIndex api)
{
    if (m_bInBlock)
        _closeBlock();

    const PP_AttrProp * pAP = NULL;
    bool bHaveProp = m_pDocument->getAttrProp(api, &pAP);
    bool bIsHeading = false;
    std::string styleAtts, propAtts, font;

	if (bHaveProp && pAP)
	{
		UT_UTF8String sa, pa, f, escape;
		OO_StylesWriter::map(pAP, sa, pa, f);

		const gchar * szStyle = NULL;
		pAP->getAttribute("style", szStyle);

		if (szStyle && pa.size())
		{
			// custom properties, prepare to derive style
			escape = szStyle;
			sa += UT_UTF8String_sprintf("style:parent-style-name=\"%s\" ", escape.escapeXML().utf8_str());
		}
		else if (szStyle)
		{
			escape = szStyle;
			sa += UT_UTF8String_sprintf("text:style-name=\"%s\" ", escape.escapeXML().utf8_str());
		}
	
		// FIXME: hacky, but in sxw there is a distinction between heading- and text paragraphs
		// we should probably also care about text:level here
		if (szStyle && strstr(szStyle, "Heading"))
			bIsHeading = true;		

        styleAtts += sa.utf8_str();
        propAtts += pa.utf8_str();
        font += f.utf8_str();
	}

    m_pListenerImpl->openBlock(styleAtts, propAtts, font, bIsHeading);
    m_bInBlock = true;
}

void OO_Listener::_closeBlock ()
{
   if (!m_bInBlock)
      return;
   
   m_pListenerImpl->closeBlock();
   m_bInBlock = false;
}

void OO_Listener::_openSpan(PT_AttrPropIndex api)
{
   if (!m_bInBlock)
   {
      return;
   }
   const PP_AttrProp * pAP = NULL;
   bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
   
   std::string propAtts, font;

   if (bHaveProp && pAP)
   {
      UT_UTF8String sa, pa, f;
      OO_StylesWriter::map(pAP, sa, pa, f);

      if (sa.size()) {
        UT_DEBUGMSG(("OO_Listener::_openSpan(): style atts in span"));
      }

      propAtts += pa.utf8_str();
      font += f.utf8_str();	
   }
   
   m_pListenerImpl->openSpan(propAtts, font);
   m_bInSpan = true;
}

void OO_Listener::_closeSpan()
{
   if (m_bInSpan)
      m_pListenerImpl->closeSpan();
   m_bInSpan = false;
}

void OO_Listener::_openHyperlink(const PP_AttrProp* pAP)
{
   if (m_bInHyperlink || !pAP)
      return;

   m_pListenerImpl->openHyperlink(pAP);
   m_bInHyperlink = true;
}

void OO_Listener::_closeHyperlink()
{
   if (m_bInHyperlink)
      m_pListenerImpl->closeHyperlink();
   m_bInHyperlink = false;
}


/*****************************************************************************/
/*****************************************************************************/

/*!
 * Class holding 1 static member. Its sole duty is to write
 * out a OOo meta.xml file based on Abi's metadata.
 */
class OO_MetaDataWriter
{
public:
  
  static bool writeMetaData(PD_Document * pDoc, GsfOutfile * oo)
  {
    GsfOutput * meta = gsf_outfile_new_child (oo, "meta.xml", FALSE);

    static const char * const preamble [] = 
      {
	"<?xml version='1.0' encoding='UTF-8'?>\n",
	"<!DOCTYPE office:document-meta PUBLIC '-//OpenOffice.org//DTD OfficeDocument 1.0//EN' 'office.dtd'>\n",
	"<office:document-meta xmlns:office='http://openoffice.org/2000/office' xmlns:xlink='http://www.w3.org/1999/xlink' xmlns:dc='http://purl.org/dc/elements/1.1/' xmlns:meta='http://openoffice.org/2000/meta' office:version='1.0'>\n",
	"<office:meta>\n",
	"<meta:generator>AbiWord</meta:generator>\n"
      };

    static const char * const postamble [] =
      {
	"</office:meta>\n",
	"</office:document-meta>\n"
      };

    writeToStream(meta, preamble, G_N_ELEMENTS(preamble));

    std::string meta_val, val;

    if (pDoc->getMetaDataProp(PD_META_KEY_DATE, meta_val) && meta_val.size()) {
      val = UT_std_string_sprintf("<dc:date>%s</dc:date>\n", meta_val.c_str());
      oo_gsf_output_write(meta, val.size(), reinterpret_cast<const guint8*>(val.c_str()));
    }
    if (pDoc->getMetaDataProp(PD_META_KEY_LANGUAGE, meta_val) && meta_val.size()) {
      val = UT_std_string_sprintf("<dc:language>%s</dc:language>\n", UT_escapeXML(meta_val).c_str());
      oo_gsf_output_write(meta, val.size(), reinterpret_cast<const guint8*>(val.c_str()));
    }

    writeToStream(meta, postamble, G_N_ELEMENTS(postamble));

    oo_gsf_output_close(meta);

    return true;
  }

private:
  OO_MetaDataWriter ();
};

/*****************************************************************************/
/*****************************************************************************/

/*!
 * Class holding 1 static member. Its sole duty is to write
 * out a OOo settings file. Probably will just dump "standard"
 * info to the settings file, since Abi pretty much ignores
 * OOo's settings file on import.
 */
class OO_SettingsWriter
{
public:

  static bool writeSettings(PD_Document * /*pDoc*/, GsfOutfile * oo)
  {
    GsfOutput * settings = gsf_outfile_new_child (oo, "settings.xml", FALSE);

    static const char * const preamble [] = 
      {
	"<?xml version='1.0' encoding='UTF-8'?>\n",
	"<!DOCTYPE office:document-settings PUBLIC '-//OpenOffice.org//DTD OfficeDocument 1.0//EN' 'office.dtd'>\n",
	"<office:document-settings xmlns:office='http://openoffice.org/2000/office' xmlns:xlink='http://www.w3.org/1999/xlink' xmlns:config='http://openoffice.org/2001/config' office:version='1.0'>\n",
	"<office:settings>\n",
	"</office:settings>\n",
	"</office:document-settings>"
      };

    writeToStream (settings, preamble, G_N_ELEMENTS(preamble));

    oo_gsf_output_close(settings);

    return true;
  }

private:
  OO_SettingsWriter ();
};

/*****************************************************************************/
/*****************************************************************************/

/*!
 * Class holding 1 static member. Its sole duty is to write
 * out any pictures from inside the Abi document to the
 * OOo pictures directory
 */
class OO_PicturesWriter
{
public:
  
  static bool writePictures(PD_Document * pDoc, GsfOutfile * oo)
  {
    const char * szName;
    std::string mimeType;
    UT_ConstByteBufPtr pByteBuf;

    // create Pictures directory
    GsfOutput * pictures = gsf_outfile_new_child(oo, "Pictures", TRUE);
    
    for (UT_uint32 k=0; (pDoc->enumDataItems(k, NULL, &szName, pByteBuf, &mimeType)); k++)
    {
        const char * extension = "png";
        // create individual pictures
        if(mimeType == "image/jpeg") {
            extension = "jpg";
        }
        std::string name = UT_std_string_sprintf("IMG-%d.%s", k, extension);
        GsfOutput * img = gsf_outfile_new_child(GSF_OUTFILE(pictures), name.c_str(), FALSE);	
        oo_gsf_output_write(img, pByteBuf->getLength(),  pByteBuf->getPointer(0));

        oo_gsf_output_close(img);
    }

    oo_gsf_output_close(pictures);

    return true;
  }

private:

  OO_PicturesWriter ();
};

/*****************************************************************************/
/*****************************************************************************/

/*!
 * Class holding 1 static member. Its sole duty is to create 
 * the OOo manifest file
 */
class OO_ManifestWriter
{
public:

  static bool writeManifest(PD_Document * pDoc, GsfOutfile * oo)
  {
    // create Pictures directory
    GsfOutput * meta_inf = gsf_outfile_new_child(oo, "META-INF", TRUE);
    GsfOutput * manifest = gsf_outfile_new_child(GSF_OUTFILE(meta_inf), "manifest.xml", FALSE);

    std::string name;

    static const char * const preamble [] = 
      {
	"<?xml version='1.0' encoding='UTF-8'?>\n",
	"<!DOCTYPE manifest:manifest PUBLIC '-//OpenOffice.org//DTD Manifest 1.0//EN' 'Manifest.dtd'>\n",
	"<manifest:manifest xmlns:manifest='http://openoffice.org/2001/manifest'>\n",
	"<manifest:file-entry manifest:media-type='application/vnd.sun.xml.writer' manifest:full-path='/'/>\n",
	"<manifest:file-entry manifest:media-type='text/xml' manifest:full-path='content.xml'/>\n",
	"<manifest:file-entry manifest:media-type='text/xml' manifest:full-path='styles.xml'/>\n",
	"<manifest:file-entry manifest:media-type='text/xml' manifest:full-path='meta.xml'/>\n",
	"<manifest:file-entry manifest:media-type='text/xml' manifest:full-path='settings.xml'/>\n"
      };

    static const char * const postamble [] =
      {
	"</manifest:manifest>\n"
      };

    writeToStream (manifest, preamble, G_N_ELEMENTS(preamble));

    const char * szName;
    std::string mimeType;
    UT_ConstByteBufPtr pByteBuf;
    for (UT_uint32 k = 0; (pDoc->enumDataItems(k, NULL, &szName, pByteBuf, &mimeType)); k++)
    {
        const char *extension = "png";
        if (mimeType == "image/jpeg") {
            extension = "jpg";
        }
        if (k == 0) {
            name = "<manifest:file-entry manifest:media-type='' manifest:full-path='Pictures/'/>\n";
            oo_gsf_output_write(manifest, name.size(), reinterpret_cast<const guint8 *>(name.c_str()));
        }
	  
        name = UT_std_string_sprintf("<manifest:file-entry manifest:media-type='%s' manifest:full-path='Pictures/IMG-%d.%s'/>\n", mimeType.c_str(), k, extension);
        oo_gsf_output_write (manifest, name.size(), reinterpret_cast<const guint8 *>(name.c_str()));
    }

    writeToStream (manifest, postamble, G_N_ELEMENTS(postamble));

    oo_gsf_output_close(manifest);
    oo_gsf_output_close(meta_inf);

    return true;
  }

private:
  OO_ManifestWriter ();
};

/*****************************************************************************/
/*****************************************************************************/

bool OO_StylesWriter::writeStyles(PD_Document * pDoc, GsfOutfile * oo, OO_StylesContainer & stylesContainer)
{
  GsfOutput * styleStream = gsf_outfile_new_child (oo, "styles.xml", FALSE);
  
  static const char * const preamble [] = 
    {	   
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n",
      "<!DOCTYPE office:document-styles PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"office.dtd\">\n",
      "<office:document-styles xmlns:office=\"http://openoffice.org/2000/office\" xmlns:style=\"http://openoffice.org/2000/style\" xmlns:text=\"http://openoffice.org/2000/text\" xmlns:table=\"http://openoffice.org/2000/table\" xmlns:draw=\"http://openoffice.org/2000/drawing\" xmlns:fo=\"http://www.w3.org/1999/XSL/Format\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:number=\"http://openoffice.org/2000/datastyle\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:chart=\"http://openoffice.org/2000/chart\" xmlns:dr3d=\"http://openoffice.org/2000/dr3d\" xmlns:math=\"http://www.w3.org/1998/Math/MathML\" xmlns:form=\"http://openoffice.org/2000/form\" xmlns:script=\"http://openoffice.org/2000/script\" office:version=\"1.0\">\n"
    };
  
  static const char * const midsection [] = 
    {	   
      "<office:styles>\n",
      "<style:default-style style:family=\"graphics\">\n",
      "<style:properties draw:start-line-spacing-horizontal=\"0.283cm\" draw:start-line-spacing-vertical=\"0.283cm\" draw:end-line-spacing-horizontal=\"0.283cm\" draw:end-line-spacing-vertical=\"0.283cm\" fo:color=\"#000000\" style:font-name=\"Nimbus Roman No9 L\" fo:font-size=\"12pt\" fo:language=\"en\" fo:country=\"US\" style:font-name-asian=\"HG Mincho Light J\" style:font-size-asian=\"12pt\" style:language-asian=\"none\" style:country-asian=\"none\" style:font-name-complex=\"Arial Unicode MS\" style:font-size-complex=\"12pt\" style:language-complex=\"none\" style:country-complex=\"none\" style:text-autospace=\"ideograph-alpha\" style:punctuation-wrap=\"simple\" style:line-break=\"strict\">\n",
      "<style:tab-stops/>\n",
      "</style:properties>\n",
      "</style:default-style>\n",
      "<style:default-style style:family=\"paragraph\">\n",
      "<style:properties fo:color=\"#000000\" style:font-name=\"Nimbus Roman No9 L\" fo:font-size=\"12pt\" fo:language=\"en\" fo:country=\"US\" style:font-name-asian=\"HG Mincho Light J\" style:font-size-asian=\"12pt\" style:language-asian=\"none\" style:country-asian=\"none\" style:font-name-complex=\"Arial Unicode MS\" style:font-size-complex=\"12pt\" style:language-complex=\"none\" style:country-complex=\"none\" fo:hyphenate=\"false\" fo:hyphenation-remain-char-count=\"2\" fo:hyphenation-push-char-count=\"2\" fo:hyphenation-ladder-count=\"no-limit\" style:text-autospace=\"ideograph-alpha\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:tab-stop-distance=\"2.205cm\"/>\n",
      "</style:default-style>\n"
    };
  
  UT_UTF8String styles;
  const PD_Style * pStyle = NULL;
  UT_GenericVector<PD_Style *> vecStyles;
  pDoc->getAllUsedStyles(&vecStyles);
  UT_sint32 k = 0;
  UT_UTF8String styleAtts, propAtts, font;
  for (k=0; k < vecStyles.getItemCount(); k++)
  {
      pStyle = vecStyles.getNthItem(k);
      PT_AttrPropIndex api = pStyle->getIndexAP();
      const PP_AttrProp * pAP = NULL;
      bool bHaveProp = pDoc->getAttrProp (api, &pAP);
      
      if (bHaveProp && pAP) 
      {
          OO_StylesWriter::map(pAP, styleAtts, propAtts, font);
	  
          styles += "<style:style " + styleAtts + ">\n";
          styles += "<style:properties " + propAtts + "/>\n";
          styles += "</style:style>\n";
      }
      
      if (font.size())
      {
          std::string f = font.utf8_str();
          stylesContainer.addFont(f);
          font.clear();
      }
  }
  
  static const char * const postamble [] = 
    {
      "<text:outline-style>\n",
      "<text:outline-level-style text:level=\"1\" style:num-format=\"\"/>\n",
      "<text:outline-level-style text:level=\"2\" style:num-format=\"\"/>\n",
      "<text:outline-level-style text:level=\"3\" style:num-format=\"\"/>\n",
      "<text:outline-level-style text:level=\"4\" style:num-format=\"\"/>\n",
      "<text:outline-level-style text:level=\"5\" style:num-format=\"\"/>\n",
      "<text:outline-level-style text:level=\"6\" style:num-format=\"\"/>\n",
      "<text:outline-level-style text:level=\"7\" style:num-format=\"\"/>\n",
      "<text:outline-level-style text:level=\"8\" style:num-format=\"\"/>\n",
      "<text:outline-level-style text:level=\"9\" style:num-format=\"\"/>\n",
      "<text:outline-level-style text:level=\"10\" style:num-format=\"\"/>\n",
      "</text:outline-style>\n",
      "<text:footnotes-configuration style:num-format=\"1\" text:start-value=\"0\" text:footnotes-position=\"page\" text:start-numbering-at=\"document\"/>\n",
      "<text:endnotes-configuration style:num-format=\"i\" text:start-value=\"0\"/>\n",
      "<text:linenumbering-configuration text:number-lines=\"false\" text:offset=\"0.499cm\" style:num-format=\"1\" text:number-position=\"left\" text:increment=\"5\"/>\n",
      "</office:styles>\n",
      "<office:automatic-styles>\n",
      "<style:page-master style:name=\"pm1\">\n",
      "<style:properties fo:page-width=\"21.59cm\" fo:page-height=\"27.94cm\" style:num-format=\"1\" style:print-orientation=\"portrait\" fo:margin-top=\"2.54cm\" fo:margin-bottom=\"2.54cm\" fo:margin-left=\"3.175cm\" fo:margin-right=\"3.175cm\" style:footnote-max-height=\"0cm\">\n",
      "<style:footnote-sep style:width=\"0.018cm\" style:distance-before-sep=\"0.101cm\" style:distance-after-sep=\"0.101cm\" style:adjustment=\"left\" style:rel-width=\"25%\" style:color=\"#000000\"/>\n",
      "</style:properties>\n",
      "<style:header-style/>\n",
      "<style:footer-style/>\n",
      "</style:page-master>\n",
      "</office:automatic-styles>\n",
      "<office:master-styles>\n",
      "<style:master-page style:name=\"Standard\" style:page-master-name=\"pm1\"/>\n",
      "</office:master-styles>\n",
      "</office:document-styles>\n"
    };
  
  writeToStream(styleStream, preamble, G_N_ELEMENTS(preamble));
  
  UT_UTF8String fontDecls = "<office:font-decls>\n";
  OO_StylesWriter::addFontDecls(fontDecls, stylesContainer);
  fontDecls += "</office:font-decls>\n";
  writeUTF8String(styleStream, fontDecls.utf8_str());
  
  writeToStream(styleStream, midsection, G_N_ELEMENTS(midsection));
  writeUTF8String(styleStream, styles.utf8_str());
  writeToStream(styleStream, postamble, G_N_ELEMENTS(postamble));
  
  oo_gsf_output_close(styleStream);
  
  return true;
}

void OO_StylesWriter::addFontDecls(UT_UTF8String & buffer, OO_StylesContainer & stylesContainer)
{
	UT_GenericVector<const UT_String*> *vecFonts = stylesContainer.getFontsKeys();
	for (UT_sint32 i = 0; i < vecFonts->size(); i++) {
		// FIXME ATM only variable width fonts
		// check here by using pango?
		const gchar * pitch = "variable";
		const UT_String * font = vecFonts->getNthItem(i);
		buffer += UT_UTF8String_sprintf("<style:font-decl style:name=\"%s\" fo:font-family=\"'%s'\" style:font-pitch=\"%s\"/>\n", 
						font->c_str(), font->c_str(), pitch);
	}
	DELETEP(vecFonts);
}

void OO_StylesWriter::map(const PP_AttrProp * pAP, UT_UTF8String & styleAtts, UT_UTF8String & propAtts, UT_UTF8String & font) 
{		
	UT_UTF8String escape;
	const gchar * szValue = NULL;
	styleAtts.clear();
	propAtts.clear();

	// Attributes
	if (pAP->getAttribute("name", szValue))
	{ 
		escape = szValue;
		styleAtts += UT_UTF8String_sprintf("style:name=\"%s\" ", escape.escapeXML().utf8_str());
	}

	if (pAP->getAttribute("type", szValue)) 
		if (!strcmp(szValue, "P"))
		{
			styleAtts += UT_UTF8String_sprintf("style:family=\"paragraph\" ");
			styleAtts += UT_UTF8String_sprintf("style:class=\"text\" ");
		}

	if (pAP->getAttribute("basedon", szValue)) 
	{
		escape = szValue;
		styleAtts += UT_UTF8String_sprintf("style:parent-style-name=\"%s\" ", escape.escapeXML().utf8_str());
	}
	
	if (pAP->getAttribute("followedby", szValue)) 
	{
		// ignore, current style is default
		if (strcmp(szValue, "Current Settings"))
		{
			escape = szValue;
			styleAtts += UT_UTF8String_sprintf("style:next-style-name=\"%s\" ", escape.escapeXML().utf8_str());
		}
	}

	// Properties
	// please keep alphabetic order
	if (pAP->getProperty("bgcolor", szValue)) 
	{
		propAtts += UT_UTF8String_sprintf("style:text-background-color=\"#%s\" ", szValue); // # is eaten unless escaped
	}

	if (pAP->getProperty("color", szValue)) 
	{
		propAtts += UT_UTF8String_sprintf("fo:color=\"#%s\" ", szValue); // # is eaten unless escaped
	}

	if (pAP->getProperty("dom-dir", szValue)) //:ltr; 
		if (!strcmp(szValue, "rtl"))
		{
			// FIXME some of these parameters are mentioned more than once
			propAtts += UT_UTF8String_sprintf("fo:text-align", "end");
			propAtts += UT_UTF8String_sprintf("style:justify-single-word", "false");
			propAtts += UT_UTF8String_sprintf("style:writing-mode", "rl-tb");
		}

	if (pAP->getProperty("font-family", szValue)) 
	{
		propAtts += UT_UTF8String_sprintf("style:font-name=\"%s\" ", szValue);
		propAtts += UT_UTF8String_sprintf("style:font-name-asian=\"%s\" ", szValue);
		propAtts += UT_UTF8String_sprintf("style:font-name-complex=\"%s\" ", szValue);

		// store font for font-decls
		font = szValue;
	}

	if (pAP->getProperty("font-size", szValue)) 
	{
		propAtts += UT_UTF8String_sprintf("fo:font-size=\"%gpt\" ", UT_convertToPoints(szValue));
		propAtts += UT_UTF8String_sprintf("style:font-size-asian=\"%gpt\" ", UT_convertToPoints(szValue));
		propAtts += UT_UTF8String_sprintf("style:font-size-complex=\"%gpt\" ", UT_convertToPoints(szValue));
	}

	if (pAP->getProperty("font-stretch", szValue)) 
	{
		/*TODO*/
		// is this "fo:letter-spacing" ?
	}

	if (pAP->getProperty("font-style", szValue)) 
	{
		// fo: style: style: according to spec
		propAtts += UT_UTF8String_sprintf("fo:font-style=\"%s\" ", szValue);
		propAtts += UT_UTF8String_sprintf("style:font-style-asian=\"%s\" ", szValue);
		propAtts += UT_UTF8String_sprintf("style:font-style-complex=\"%s\" ", szValue);
	}

	if (pAP->getProperty("font-variant", szValue)) 
		propAtts += UT_UTF8String_sprintf("fo:font-variant=\"%s\" ", szValue);

	if (pAP->getProperty("font-weight", szValue)) 
	{
		propAtts += UT_UTF8String_sprintf("fo:font-weight=\"%s\" ", szValue);
		propAtts += UT_UTF8String_sprintf("style:font-weight-asian=\"%s\" ", szValue);
		propAtts += UT_UTF8String_sprintf("style:font-weight-complex=\"%s\" ", szValue);
	}

	if (pAP->getProperty("keep-with-next", szValue)) 
	{/*TODO*/}

	if (pAP->getProperty("line-height", szValue))
	{
		if (szValue[strlen(szValue)] == '+')
			propAtts += UT_UTF8String_sprintf("style:line-height-at-least=\"%fcm\" ", UT_convertToDimension(szValue, DIM_CM));
		else if (UT_determineDimension(szValue, DIM_none) == DIM_none)
			propAtts += UT_UTF8String_sprintf("fo:line-height=\"%d%%\" ", rint(atof(szValue) * 100));
		else
			propAtts += UT_UTF8String_sprintf("fo:line-height=\"%fcm\" ", UT_convertToDimension(szValue, DIM_CM));
			// propAtts += UT_UTF8String_sprintf("fo:style:line-spacing=\"%d\%\" ", rint(atof(szValue) * 100));
	}

	if (pAP->getProperty("margin-left", szValue))
		propAtts += UT_UTF8String_sprintf("fo:margin-left=\"%s\" ", szValue);			

	if (pAP->getProperty("margin-top", szValue))
		propAtts += UT_UTF8String_sprintf("fo:margin-top=\"%s\" ", szValue);			

	if (pAP->getProperty("margin-right", szValue))
		propAtts += UT_UTF8String_sprintf("fo:margin-right=\"%s\" ", szValue);			

	if (pAP->getProperty("margin-bottom", szValue))
		propAtts += UT_UTF8String_sprintf("fo:margin-bottom=\"%s\" ", szValue);			

	if (pAP->getProperty("text-align", szValue)) //left center right -> start end left right center justify
	{
		if (strcmp(szValue, "left")) // skip default
		{
			propAtts += UT_UTF8String_sprintf("style:justify-single-word=\"false\" ");

			if (!strcmp(szValue, "right"))
				propAtts += UT_UTF8String_sprintf("fo:text-align=\"end\" ");
			else
				propAtts += UT_UTF8String_sprintf("fo:text-align=\"%s\" ", szValue);
		}
	}

	if (pAP->getProperty("text-decoration", szValue))
	{
		if (strstr(szValue, "underline"))
		{
			propAtts += "style:text-underline=\"single\" ";
			propAtts += "style:text-underline-color=\"font-color\" ";
		}

		if (strstr(szValue, "line-through"))
			propAtts += "style:text-crossing-out=\"single-line\" ";
	}

	if (pAP->getProperty("text-indent", szValue))
	{
		propAtts += UT_UTF8String_sprintf("fo:text-indent=\"%s\" ", szValue);
		propAtts += UT_UTF8String_sprintf("style:auto-text-indent=\"false\" ");
	}

	if (pAP->getProperty("text-position", szValue))
	{
		if (!strcmp(szValue, "superscript"))
			propAtts += "style:text-position=\"super 58%\" ";
		else if (!strcmp(szValue, "subscript"))
			propAtts += "style:text-position=\"sub 58%\" ";
	}

	if (pAP->getProperty("widows", szValue))
	{/*TODO*/}
}


/*****************************************************************************/
/*****************************************************************************/


IE_Exp_OpenWriter::IE_Exp_OpenWriter (PD_Document * pDoc)
  : IE_Exp (pDoc), m_oo(0)
{
}

IE_Exp_OpenWriter::~IE_Exp_OpenWriter()
{
}

#define SXW_MIMETYPE "application/vnd.sun.xml.writer"

/*!
 * This writes out our AbiWord file as an OpenOffice
 * compound document
 */
UT_Error IE_Exp_OpenWriter::_writeDocument(void)
{
  UT_return_val_if_fail (getFp(), UT_ERROR);
  m_oo = GSF_OUTFILE (gsf_outfile_zip_new (getFp(), NULL));
  UT_return_val_if_fail(m_oo, UT_ERROR);

  {
    GsfOutput * mimetype = gsf_outfile_new_child (m_oo, "mimetype", FALSE);
    if (!mimetype)
      {
	oo_gsf_output_close(GSF_OUTPUT(m_oo));
	return UT_ERROR;
      }

    oo_gsf_output_write(mimetype, strlen(SXW_MIMETYPE), (const guint8 *)SXW_MIMETYPE);
    oo_gsf_output_close(mimetype);
  }

  if (!OO_MetaDataWriter::writeMetaData(getDoc(), m_oo))
    {
      oo_gsf_output_close(GSF_OUTPUT(m_oo));
      return UT_ERROR;
    }

  if (!OO_SettingsWriter::writeSettings(getDoc(), m_oo))
    {
      oo_gsf_output_close(GSF_OUTPUT(m_oo));
      return UT_ERROR;
    }

  if (!OO_PicturesWriter::writePictures(getDoc(), m_oo))
    {
      oo_gsf_output_close(GSF_OUTPUT(m_oo));
      return UT_ERROR;
    }

  if (!OO_ManifestWriter::writeManifest(getDoc(), m_oo))
    {
      oo_gsf_output_close(GSF_OUTPUT(m_oo));
      return UT_ERROR;
    }

  OO_StylesContainer stylesContainer;
  OO_AccumulatorImpl accumulatorImpl(&stylesContainer);
  OO_Listener listener1(getDoc(), &accumulatorImpl);
  if (!getDoc()->tellListener(static_cast<PL_Listener *>(&listener1)))
    {
      oo_gsf_output_close(GSF_OUTPUT(m_oo));
      return UT_ERROR;
    }

  if (!OO_StylesWriter::writeStyles(getDoc(), m_oo, stylesContainer))
    {
      oo_gsf_output_close(GSF_OUTPUT(m_oo));
      return UT_ERROR;
    }

  {
    OO_WriterImpl writerImpl(m_oo, &stylesContainer);
    OO_Listener listener2(getDoc(), &writerImpl);
    if (!getDoc()->tellListener(static_cast<PL_Listener *>(&listener2)))
      {
	oo_gsf_output_close(GSF_OUTPUT(m_oo));
	return UT_ERROR;
      }

    listener2.endDocument();
  }

  oo_gsf_output_close(GSF_OUTPUT(m_oo));
  return UT_OK;
}

/*****************************************************************************/
/*****************************************************************************/
