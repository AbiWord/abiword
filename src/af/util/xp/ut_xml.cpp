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

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_xml.h"

#include "xap_EncodingManager.h"

#ifdef HAVE_LIBXML2
// Please keep the "/**/" to stop MSVC dependency generator complaining.
#include /**/ <libxml/parserInternals.h>
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
  m_pListener(0),
  m_pReader(0)
{
}

UT_XML::~UT_XML ()
{
}

static void _startElement (void * userData, const XML_Char * name, const XML_Char ** atts)
{
  UT_XML::Listener * pListener = (UT_XML::Listener *) userData;

  /* libxml2 can supply atts == 0, which is a little at variance to what is expected...
   */
  const XML_Char * ptr = 0;
  const XML_Char ** new_atts = atts;
  if (atts == 0) new_atts = &ptr;

  pListener->startElement (name, new_atts);
}

static void _endElement (void * userData, const XML_Char * name)
{
  UT_XML::Listener * pListener = (UT_XML::Listener *) userData;
  pListener->endElement (name);
}

static void _charData (void * userData, const XML_Char * buffer, int length)
{
  UT_XML::Listener * pListener = (UT_XML::Listener *) userData;
  pListener->charData (buffer, length);
}

#ifdef HAVE_LIBXML2
static xmlEntityPtr _getEntity (void * userData, const XML_Char * name)
{
  return xmlGetPredefinedEntity (name);
}
#endif /* HAVE_LIBXML2 */

UT_Error UT_XML::load_then_parse (const char * szFilename)
{
  UT_ASSERT (m_pListener);
  UT_ASSERT (szFilename);

  DefaultReader defaultReader;
  Reader * reader = &defaultReader;
  if (m_pReader) reader = m_pReader;

  if (!reader->openFile (szFilename))
    {
      UT_DEBUGMSG (("Could not open file %s\n", szFilename));
      return UT_errnoToUTError ();
    }

  UT_ByteBuf BB;
  UT_Error ret = UT_OK;

  char buffer[4096];

  int done = 0;
  while (!done)
    {
      size_t length = reader->readBytes (buffer, sizeof (buffer));
      done = (length < sizeof (buffer));

      if (!BB.append ((const UT_Byte *) buffer, (UT_uint32) length))
	{
	  ret = UT_OUTOFMEM;
	  break;
	}
    }

  reader->closeFile ();

  if (ret == UT_OK) ret = parse (&BB);

  return ret;
}

UT_Error UT_XML::parse (const char * szFilename)
{
  UT_ASSERT (m_pListener);
  UT_ASSERT (szFilename);

  UT_Error ret = UT_OK;

#ifdef HAVE_LIBXML2

  if (m_pReader) return load_then_parse (szFilename);

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

  ctxt = xmlCreateFileParserCtxt (szFilename);
  if (ctxt == NULL)
    {
      UT_DEBUGMSG (("Unable to create libxml2 file context!\n"));
      return UT_ERROR;
    }
  ctxt->sax = &hdl;
  ctxt->userData = (void *) m_pListener;

  xmlParseDocument (ctxt);

  if (!ctxt->wellFormed) ret = UT_IE_IMPORTERROR;

  ctxt->sax = NULL;
  xmlFreeParserCtxt (ctxt);

#else /* EXPAT */

  DefaultReader defaultReader;
  Reader * reader = &defaultReader;
  if (m_pReader) reader = m_pReader;

  if (!reader->openFile (szFilename))
    {
      UT_DEBUGMSG (("Could not open file %s\n", szFilename));
      return UT_errnoToUTError ();
    }

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

  XML_SetUserData (parser, (void *) m_pListener);

  char buffer[4096];

  int done = 0;
  while (!done)
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

  reader->closeFile ();

#endif

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
  UT_ASSERT (m_pListener);
  UT_ASSERT (buffer);

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

  ctxt = xmlCreateMemoryParserCtxt (buffer, (int) length);
  if (ctxt == NULL)
    {
      UT_DEBUGMSG (("Unable to create libxml2 memory context!\n"));
      return UT_ERROR;
    }
  ctxt->sax = &hdl;
  ctxt->userData = (void *) m_pListener;

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

  XML_SetUserData (parser, (void *) m_pListener);

  if (!XML_Parse (parser, buffer, (int) length, 1))
    {
      UT_DEBUGMSG (("%s at line %d\n", XML_ErrorString (XML_GetErrorCode (parser)), XML_GetCurrentLineNumber (parser)));
      ret = UT_IE_IMPORTERROR;
    }

  XML_ParserFree (parser);

#endif

  return ret;
}
