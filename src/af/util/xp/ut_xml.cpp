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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_xml.h"

#include "xap_EncodingManager.h"

#ifdef HAVE_LIBXML2
// Please keep the "/**/" to stop MSVC dependency generator complaining.
#include /**/ <libxml/parser.h>
#include /**/ <libxml/parserInternals.h>

// Undo the mis-definition in ut_xml.h:
#ifdef XML_Char
#undef XML_Char
#endif
#define XML_Char xmlChar

#endif

class DefaultReader : public UT_XML::Reader
{
public:
  DefaultReader ();
  ~DefaultReader ();

  bool      openFile (const char * szFilename);
  UT_uint32 readBytes (char * buffer, UT_uint32 length);
  void      closeFile (void);

private:
  FILE * in;
};

DefaultReader::DefaultReader () :
  in(0)
{
}

DefaultReader::~DefaultReader ()
{
  if (in) fclose (in);
}

bool DefaultReader::openFile (const char * szFilename)
{
  in = fopen (szFilename, "r");
  return (in != NULL);
}

UT_uint32 DefaultReader::readBytes (char * buffer, UT_uint32 length)
{
  UT_ASSERT(in);
  UT_ASSERT(buffer);

  return fread (buffer, 1, length, in);
}

void DefaultReader::closeFile (void)
{
  if (in) fclose (in);
  in = 0;
}

UT_XML::UT_XML () :
  m_namespace(0),
  m_nslength(0),
  m_bSniffing(false),
  m_bValid(false),
  m_xml_type(0),
  m_ParseMode(pm_XML),
  m_bStopped(false),
  m_pListener(0),
  m_pReader(0),
  m_decoder(0)
{
}

UT_XML::~UT_XML ()
{
  FREEP (m_namespace);

  if (m_decoder)
    {
#ifdef HAVE_LIBXML2
      xmlFreeParserCtxt ((xmlParserCtxtPtr) m_decoder);
#else /* EXPAT */
      XML_ParserFree ((XML_Parser) m_decoder);
#endif
    }
}

/* these functions needs to be declared as plain C for MrCPP (Apple MPW C)
 */
#ifdef __MRC__
extern "C" {
#endif

static void _startElement (void * userData, const XML_Char * name, const XML_Char ** atts)
{
  UT_XML * pXML = (UT_XML *) userData;

  /* libxml2 can supply atts == 0, which is a little at variance to what is expected...
   */
  const XML_Char * ptr = 0;
  const XML_Char ** new_atts = atts;
  if (atts == 0) new_atts = &ptr;

  pXML->startElement ((const char *) name, (const char **) new_atts);
}

static void _endElement (void * userData, const XML_Char * name)
{
  UT_XML * pXML = (UT_XML *) userData;
  pXML->endElement ((const char *) name);
}

static void _charData (void * userData, const XML_Char * buffer, int length)
{
  UT_XML * pXML = (UT_XML *) userData;
  pXML->charData ((const char *) buffer, length);
}

#ifdef HAVE_LIBXML2
static xmlEntityPtr _getEntity (void * userData, const XML_Char * name)
{
  return xmlGetPredefinedEntity (name);
}
#endif /* HAVE_LIBXML2 */

#ifdef __MRC__
};
#endif

/* Declared in ut_xml.h as: void UT_XML::startElement (const XML_Char * name, const XML_Char ** atts);
 */
void UT_XML::startElement (const char * name, const char ** atts)
{
  if (m_bStopped) return;
  if (m_nslength)
    if (strncmp (name,m_namespace,m_nslength) == 0)
      {
	if (*(name + m_nslength) == ':') name += m_nslength + 1;
      }
  if (m_bSniffing)
    {
      if (strcmp (name,m_xml_type) == 0) m_bValid = true;
      stop (); // proceed no further - we don't have any listener
      return;
    }

  UT_ASSERT (m_pListener);
  m_pListener->startElement (name, atts);
}

/* Declared in ut_xml.h as: void UT_XML::endElement (const XML_Char * name);
 */
void UT_XML::endElement (const char * name)
{
  if (m_bStopped) return;
  if (m_nslength)
    if (strncmp (name,m_namespace,m_nslength) == 0)
      {
	if (*(name + m_nslength) == ':') name += m_nslength + 1;
      }

  UT_ASSERT (m_pListener);
  m_pListener->endElement (name);
}

/* Declared in ut_xml.h as: void UT_XML::charData (const XML_Char * buffer, int length);
 */
void UT_XML::charData (const char * buffer, int length)
{
  if (m_bStopped) return;

  UT_ASSERT (m_pListener);
  m_pListener->charData (buffer, length);
}

/* I'm still very confused about XML namespaces so the handling here is likely to change a lot as I learn...
 */
void UT_XML::setNameSpace (const char * xml_namespace)
{
  FREEP (m_namespace);
  if (xml_namespace) m_namespace = UT_strdup (xml_namespace);

  m_nslength = 0;
  if (m_namespace) m_nslength = strlen (m_namespace);
}

bool UT_XML::sniff (const UT_ByteBuf * pBB, const char * xml_type)
{
  UT_ASSERT (pBB);
  UT_ASSERT (xml_type);

  const char * buffer = (const char *) pBB->getPointer (0);
  UT_uint32 length = pBB->getLength ();

  return sniff (buffer, length, xml_type);
}

bool UT_XML::sniff (const char * buffer, UT_uint32 length, const char * xml_type)
{
  UT_ASSERT (buffer);
  UT_ASSERT (xml_type);

  m_bSniffing = true; // This *must* be reset to false before returning
  m_bValid = true;

  m_xml_type = xml_type;

  bool valid = false;
  if (parse (buffer, length) == UT_OK) valid = m_bValid;

  m_bSniffing = false;
  return valid;
}

UT_Error UT_XML::parse (const char * szFilename)
{
  UT_ASSERT (m_pListener);
  UT_ASSERT (szFilename);

  if (m_ParseMode == pm_HTML) return html (szFilename);

  UT_Error ret = UT_OK;

  DefaultReader defaultReader;
  Reader * reader = &defaultReader;
  if (m_pReader) reader = m_pReader;

  if (!reader->openFile (szFilename))
    {
      UT_DEBUGMSG (("Could not open file %s\n", szFilename));
      return UT_errnoToUTError ();
    }

  char buffer[2048];

  m_bStopped = false;

#ifdef HAVE_LIBXML2

  xmlSAXHandler hdl;
  xmlParserCtxtPtr ctxt;

  hdl.internalSubset = NULL;
  hdl.isStandalone = NULL;
  hdl.hasInternalSubset = NULL;
  hdl.hasExternalSubset = NULL;
  hdl.resolveEntity = NULL;
  hdl.getEntity = _getEntity;
  hdl.entityDecl = NULL;
  hdl.notationDecl = NULL;
  hdl.attributeDecl = NULL;
  hdl.elementDecl = NULL;
  hdl.unparsedEntityDecl = NULL;
  hdl.setDocumentLocator = NULL;
  hdl.startDocument = NULL;
  hdl.endDocument = NULL;
  hdl.startElement = _startElement;
  hdl.endElement = _endElement;
  hdl.reference = NULL;
  hdl.characters = _charData;
  hdl.ignorableWhitespace = NULL;
  hdl.processingInstruction = NULL;
  hdl.comment = NULL;
  hdl.warning = NULL;
  hdl.error = NULL;
  hdl.fatalError = NULL;
  hdl.getParameterEntity = NULL;
  hdl.cdataBlock = NULL;
  hdl.externalSubset = NULL;

  size_t length = reader->readBytes (buffer, sizeof (buffer));
  int done = (length < sizeof (buffer));

  if (length)
    {
      ctxt = xmlCreatePushParserCtxt (&hdl, (void *) this, buffer, (int) length, szFilename);
      if (ctxt == NULL)
	{
	  UT_DEBUGMSG (("Unable to create libxml2 push-parser context!\n"));
	  reader->closeFile ();
	  return UT_ERROR;
	}
    }
  while (!done && !m_bStopped)
    {
      length = reader->readBytes (buffer, sizeof (buffer));
      done = (length < sizeof (buffer));

      if (xmlParseChunk (ctxt, buffer, (int) length, 0))
	{
	  UT_DEBUGMSG (("Error parsing '%s'\n",szFilename));
	  ret = UT_IE_IMPORTERROR;
	  break;
	}
    }
  if (ret == UT_OK)
    if (!m_bStopped)
      if (xmlParseChunk (ctxt, buffer, 0, 1))
	{
	  UT_DEBUGMSG (("Error parsing '%s'\n",szFilename));
	  ret = UT_IE_IMPORTERROR;
	}
  if (ret == UT_OK)
    if (!ctxt->wellFormed && !m_bStopped) ret = UT_IE_IMPORTERROR; // How does stopping mid-file affect wellFormed?

  ctxt->sax = NULL;
  xmlFreeParserCtxt (ctxt);

#else /* EXPAT */

  XML_Parser parser = XML_ParserCreate (NULL);
  if (parser == NULL)
    {
      UT_DEBUGMSG (("Unable to create expat parser!\n"));
      reader->closeFile ();
      return UT_ERROR;
    }

  XML_SetUnknownEncodingHandler (parser, (XML_UnknownEncodingHandler) XAP_EncodingManager::XAP_XML_UnknownEncodingHandler, 0);

  XML_SetElementHandler (parser, _startElement, _endElement);
  XML_SetCharacterDataHandler (parser, _charData);

  XML_SetUserData (parser, (void *) this);

  int done = 0;
  while (!done && !m_bStopped)
    {
      size_t length = reader->readBytes (buffer, sizeof (buffer));
      done = (length < sizeof (buffer));

      if (!XML_Parse (parser, buffer, length, done))
	{
	  UT_DEBUGMSG (("%s at line %d\n", XML_ErrorString (XML_GetErrorCode (parser)), XML_GetCurrentLineNumber (parser)));
	  ret = UT_IE_IMPORTERROR;
	  break;
	}
    }

  XML_ParserFree (parser);

#endif

  reader->closeFile ();

  return ret;
}

UT_Error UT_XML::parse (const UT_ByteBuf * pBB)
{
  UT_ASSERT (m_pListener);
  UT_ASSERT (pBB);

  const char * buffer = (const char *) pBB->getPointer (0);
  UT_uint32 length = pBB->getLength ();

  return parse (buffer, length);
}

UT_Error UT_XML::parse (const char * buffer, UT_uint32 length)
{
  if (!m_bSniffing) UT_ASSERT (m_pListener);
  UT_ASSERT (buffer);

  if (m_ParseMode == pm_HTML) return html (buffer, length);

  UT_Error ret = UT_OK;

#ifdef HAVE_LIBXML2

  xmlSAXHandler hdl;
  xmlParserCtxtPtr ctxt;

  hdl.internalSubset = NULL;
  hdl.isStandalone = NULL;
  hdl.hasInternalSubset = NULL;
  hdl.hasExternalSubset = NULL;
  hdl.resolveEntity = NULL;
  hdl.getEntity = _getEntity;
  hdl.entityDecl = NULL;
  hdl.notationDecl = NULL;
  hdl.attributeDecl = NULL;
  hdl.elementDecl = NULL;
  hdl.unparsedEntityDecl = NULL;
  hdl.setDocumentLocator = NULL;
  hdl.startDocument = NULL;
  hdl.endDocument = NULL;
  hdl.startElement = _startElement;
  hdl.endElement = _endElement;
  hdl.reference = NULL;
  hdl.characters = _charData;
  hdl.ignorableWhitespace = NULL;
  hdl.processingInstruction = NULL;
  hdl.comment = NULL;
  hdl.warning = NULL;
  hdl.error = NULL;
  hdl.fatalError = NULL;
  hdl.getParameterEntity = NULL;
  hdl.cdataBlock = NULL;
  hdl.externalSubset = NULL;

  ctxt = xmlCreateMemoryParserCtxt (buffer, (int) length);
  if (ctxt == NULL)
    {
      UT_DEBUGMSG (("Unable to create libxml2 memory context!\n"));
      return UT_ERROR;
    }
  ctxt->sax = &hdl;
  ctxt->userData = (void *) this;

  m_bStopped = false;

  xmlParseDocument (ctxt);

  if (!ctxt->wellFormed) ret = UT_IE_IMPORTERROR;

  ctxt->sax = NULL;
  xmlFreeParserCtxt (ctxt);

#else /* EXPAT */

  XML_Parser parser = XML_ParserCreate (NULL);
  if (parser == NULL)
    {
      UT_DEBUGMSG (("Unable to create expat parser!\n"));
      return UT_ERROR;
    }

  XML_SetUnknownEncodingHandler (parser, (XML_UnknownEncodingHandler) XAP_EncodingManager::XAP_XML_UnknownEncodingHandler, 0);

  XML_SetElementHandler (parser, _startElement, _endElement);
  XML_SetCharacterDataHandler (parser, _charData);

  XML_SetUserData (parser, (void *) this);

  m_bStopped = false;

  if (!XML_Parse (parser, buffer, (int) length, 1))
    {
      UT_DEBUGMSG (("%s at line %d\n", XML_ErrorString (XML_GetErrorCode (parser)), XML_GetCurrentLineNumber (parser)));
      ret = UT_IE_IMPORTERROR;
    }

  XML_ParserFree (parser);

#endif

  return ret;
}

UT_Error UT_XML::html (const char * szFilename)
{
  UT_ASSERT (m_pListener);
  UT_ASSERT (szFilename);

  UT_Error ret = UT_OK;

#ifdef HAVE_LIBXML2

  DefaultReader defaultReader;
  Reader * reader = &defaultReader;
  if (m_pReader) reader = m_pReader;

  if (!reader->openFile (szFilename))
    {
      UT_DEBUGMSG (("Could not open file %s\n", szFilename));
      return UT_errnoToUTError ();
    }

  char buffer[2048];

  m_bStopped = false;

  htmlSAXHandler hdl;
  htmlParserCtxtPtr ctxt;

  hdl.internalSubset = NULL;
  hdl.isStandalone = NULL;
  hdl.hasInternalSubset = NULL;
  hdl.hasExternalSubset = NULL;
  hdl.resolveEntity = NULL;
  hdl.getEntity = _getEntity;
  hdl.entityDecl = NULL;
  hdl.notationDecl = NULL;
  hdl.attributeDecl = NULL;
  hdl.elementDecl = NULL;
  hdl.unparsedEntityDecl = NULL;
  hdl.setDocumentLocator = NULL;
  hdl.startDocument = NULL;
  hdl.endDocument = NULL;
  hdl.startElement = _startElement;
  hdl.endElement = _endElement;
  hdl.reference = NULL;
  hdl.characters = _charData;
  hdl.ignorableWhitespace = NULL;
  hdl.processingInstruction = NULL;
  hdl.comment = NULL;
  hdl.warning = NULL;
  hdl.error = NULL;
  hdl.fatalError = NULL;
  hdl.getParameterEntity = NULL;
  hdl.cdataBlock = NULL;
  hdl.externalSubset = NULL;

  size_t length = reader->readBytes (buffer, sizeof (buffer));
  int done = (length < sizeof (buffer));

  if (length)
    {
      ctxt = htmlCreatePushParserCtxt (&hdl, (void *) this, buffer, (int) length, szFilename, XML_CHAR_ENCODING_NONE);
      if (ctxt == NULL)
	{
	  UT_DEBUGMSG (("Unable to create libxml2 (HTML) push-parser context!\n"));
	  reader->closeFile ();
	  return UT_ERROR;
	}
    }
  while (!done && !m_bStopped)
    {
      length = reader->readBytes (buffer, sizeof (buffer));
      done = (length < sizeof (buffer));

      if (htmlParseChunk (ctxt, buffer, (int) length, 0))
	{
	  UT_DEBUGMSG (("Error parsing '%s'\n",szFilename));
	  ret = UT_IE_IMPORTERROR;
	  break;
	}
    }
  if (ret == UT_OK)
    if (!m_bStopped)
      if (htmlParseChunk (ctxt, buffer, 0, 1))
	{
	  UT_DEBUGMSG (("Error parsing '%s'\n",szFilename));
	  ret = UT_IE_IMPORTERROR;
	}

  ctxt->sax = NULL;
  htmlFreeParserCtxt (ctxt);

  reader->closeFile ();

#else /* EXPAT */

  UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
  ret = UT_ERROR;

#endif

  return ret;
}

UT_Error UT_XML::html (const char * buffer, UT_uint32 length)
{
  if (!m_bSniffing) UT_ASSERT (m_pListener);
  UT_ASSERT (buffer);

  UT_Error ret = UT_OK;

#ifdef HAVE_LIBXML2

  m_bStopped = false;

  htmlSAXHandler hdl;
  htmlParserCtxtPtr ctxt;

  hdl.internalSubset = NULL;
  hdl.isStandalone = NULL;
  hdl.hasInternalSubset = NULL;
  hdl.hasExternalSubset = NULL;
  hdl.resolveEntity = NULL;
  hdl.getEntity = _getEntity;
  hdl.entityDecl = NULL;
  hdl.notationDecl = NULL;
  hdl.attributeDecl = NULL;
  hdl.elementDecl = NULL;
  hdl.unparsedEntityDecl = NULL;
  hdl.setDocumentLocator = NULL;
  hdl.startDocument = NULL;
  hdl.endDocument = NULL;
  hdl.startElement = _startElement;
  hdl.endElement = _endElement;
  hdl.reference = NULL;
  hdl.characters = _charData;
  hdl.ignorableWhitespace = NULL;
  hdl.processingInstruction = NULL;
  hdl.comment = NULL;
  hdl.warning = NULL;
  hdl.error = NULL;
  hdl.fatalError = NULL;
  hdl.getParameterEntity = NULL;
  hdl.cdataBlock = NULL;
  hdl.externalSubset = NULL;

  if (length)
    {
      ctxt = htmlCreatePushParserCtxt (&hdl, (void *) this, buffer, (int) length, 0, XML_CHAR_ENCODING_NONE);
      if (ctxt == NULL)
	{
	  UT_DEBUGMSG (("Unable to create libxml2 (HTML) push-parser context!\n"));
	  return UT_ERROR;
	}
    }
  if (!m_bStopped)
    if (htmlParseChunk (ctxt, buffer, 0, 1))
      {
	UT_DEBUGMSG (("Error parsing buffer\n"));
	ret = UT_IE_IMPORTERROR;
      }

  ctxt->sax = NULL;
  htmlFreeParserCtxt (ctxt);

#else /* EXPAT */

  UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
  ret = UT_ERROR;

#endif

  return ret;
}

bool UT_XML::setParseMode (ParseMode pm) // returns false if the mode isn't supported.
{
  bool ret = true;
  switch (pm)
    {
    case pm_XML:
      m_ParseMode = pm_XML;
      break;

    case pm_HTML:
#ifdef HAVE_LIBXML2
      m_ParseMode = pm_HTML;
#else /* EXPAT */
      UT_DEBUGMSG(("expat doesn't parse HTML\n"));
      ret = false;
#endif
      break;
    }
  return ret;
}

/* Decoder
 * =======
 * 
 * Hmm. This next bit was in ut_string.cpp, but I want to localize libxml2 stuff here in ut_xml.cpp
 * so that I can define 'XML_Char' as 'char' not as 'xmlChar' which is 'unsigned char' which really
 * screws up everything... though it was written with this in mind, I think.
 * 
 * And I'm making parser creation & destruction one-time events; it's the principle of the thing...
 * 
 * BTW, there's expat stuff in XAP_EncodingManager, but that's less critical. - fjf
 */

#ifndef HAVE_LIBXML2

#ifdef __MRC__
extern "C" {
#endif

static void _startBlah (void * userData, const XML_Char * name, const XML_Char ** atts)
{
  char ** pout = (char **) userData;
  //What do we do with this, is this cast safe!?
  //Why wouldn't it be? - fjf
  if (atts && (*name == 'f'))
    if (*atts)
      *pout = UT_strdup ((char *) atts[1]);
}

static void _endBlah (void * userData, const XML_Char * name)
{
  //
}

#ifdef __MRC__
};
#endif

#endif /* HAVE_LIBXML2 */

bool UT_XML::startDecoder ()
{
  if (m_decoder) stopDecoder ();
#ifdef HAVE_LIBXML2
  xmlParserCtxtPtr parser = xmlCreatePushParserCtxt (0, 0, "<?xml version=\"1.0\"?>", 21, 0);
#else /* EXPAT */
  const char s[] = "<?xml version=\"1.0\"?>\n<decoder>\n";
  XML_Parser parser = XML_ParserCreate (0);
  if (parser)
    {
      XML_SetElementHandler (parser, _startBlah, _endBlah);
      if (!XML_Parse (parser, s, sizeof (s)-1, 0))
	{
	  UT_DEBUGMSG (("XML string-decode parse error %s\n", XML_ErrorString (XML_GetErrorCode (parser))));
	  XML_ParserFree (parser);
	  parser = 0;
	}
    }
#endif
  m_decoder = (void *) parser;

  return (parser != 0);
}

void UT_XML::stopDecoder ()
{
  if (m_decoder == 0) return;
#ifdef HAVE_LIBXML2
  xmlFreeParserCtxt ((xmlParserCtxtPtr) m_decoder);
#else /* EXPAT */
  XML_ParserFree ((XML_Parser) m_decoder);
#endif
  m_decoder = 0;
}

/* Declared in ut_xml.h as: XML_Char * UT_XML::decode (const XML_Char * in);
 */
char * UT_XML::decode (const char * in)
{
  if (m_decoder == 0)
    if (!startDecoder ()) return 0;
#ifdef HAVE_LIBXML2
  return (char *) xmlStringDecodeEntities ((xmlParserCtxtPtr) m_decoder, (XML_Char *) in, XML_SUBSTITUTE_BOTH, 0, 0, 0);
#else /* EXPAT */
  // There has *got* to be an easier way to do this with expat, but I
  // didn't spot it from looking at the expat source code.  Anyhow, this
  // is just used during init to chomp the preference default value
  // strings, so the amount of work done probably doesn't matter too
  // much.
  const char s1[] = "<fake blah=\"";
  const char s2[] = "\"/>";

  XML_Parser parser = (XML_Parser) m_decoder;

  char * out = 0;
  XML_SetUserData (parser, (void *) &out);

  if (!XML_Parse (parser, s1, sizeof (s1)-1, 0)
   || !XML_Parse (parser, in, strlen (in), 0)
   || !XML_Parse (parser, s2, sizeof (s2)-1, 0))
    {
      UT_DEBUGMSG (("XML string-decode parse error %s\n", XML_ErrorString (XML_GetErrorCode (parser))));
      stopDecoder (); // Should restart automatically...
    }
  // TODO: who owns the storage for this?
  // TMN: The caller of this function.

  return out;
#endif
}
