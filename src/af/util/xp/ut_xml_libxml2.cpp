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

// Please keep the "/**/" to stop MSVC dependency generator complaining.
#include <libxml/parser.h>
#include <libxml/parserInternals.h>

// Undo the mis-definition in ut_xml.h:
#ifdef XML_Char
#undef XML_Char
#endif
#define XML_Char xmlChar


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

static xmlEntityPtr _getEntity (void * userData, const XML_Char * name)
{
  return xmlGetPredefinedEntity (name);
}

static void _charData (void * userData, const XML_Char * buffer, int length)
{
  UT_XML * pXML = (UT_XML *) userData;
  pXML->charData ((const char *) buffer, length);
}

#ifdef __MRC__
};
#endif

UT_XML::~UT_XML ()
{
  FREEP (m_namespace);

  if (m_decoder)
    {
      xmlFreeParserCtxt ((xmlParserCtxtPtr) m_decoder);
    }
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

  xmlSAXHandler hdl;
  xmlParserCtxtPtr ctxt;

  memset(&hdl, 0, sizeof(hdl));

  hdl.getEntity    = _getEntity;
  hdl.startElement = _startElement;
  hdl.endElement   = _endElement;
  hdl.characters   = _charData;

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
      xmlSubstituteEntitiesDefault (1);
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

  reader->closeFile ();

  return ret;
}


UT_Error UT_XML::parse (const char * buffer, UT_uint32 length)
{
  if (!m_bSniffing) UT_ASSERT (m_pListener);
  UT_ASSERT (buffer);

  if (m_ParseMode == pm_HTML) return html (buffer, length);

  UT_Error ret = UT_OK;

  xmlSAXHandler hdl;
  xmlParserCtxtPtr ctxt;

  memset(&hdl, 0, sizeof(hdl));

  hdl.getEntity = _getEntity;
  hdl.startElement = _startElement;
  hdl.endElement = _endElement;
  hdl.characters = _charData;

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

  return ret;
}

UT_Error UT_XML::html (const char * szFilename)
{
  UT_ASSERT (m_pListener);
  UT_ASSERT (szFilename);

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

  xmlSAXHandler hdl;
  xmlParserCtxtPtr ctxt;

  memset(&hdl, 0, sizeof(hdl));

  hdl.getEntity    = _getEntity;
  hdl.startElement = _startElement;
  hdl.endElement   = _endElement;
  hdl.characters   = _charData;

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
      xmlSubstituteEntitiesDefault (1);
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

  return ret;
}

UT_Error UT_XML::html (const char * buffer, UT_uint32 length)
{
  if (!m_bSniffing) UT_ASSERT (m_pListener);
  UT_ASSERT (buffer);

  UT_Error ret = UT_OK;

  m_bStopped = false;

  xmlSAXHandler hdl;
  xmlParserCtxtPtr ctxt;

  memset (&hdl, 0, sizeof(hdl));

  hdl.getEntity = _getEntity;
  hdl.startElement = _startElement;
  hdl.endElement = _endElement;
  hdl.characters = _charData;

  if (length)
    {
      ctxt = htmlCreatePushParserCtxt (&hdl, (void *) this, buffer, (int) length, 0, XML_CHAR_ENCODING_NONE);
      if (ctxt == NULL)
	{
	  UT_DEBUGMSG (("Unable to create libxml2 (HTML) push-parser context!\n"));
	  return UT_ERROR;
	}
      xmlSubstituteEntitiesDefault (1);
    }
  if (!m_bStopped)
    if (htmlParseChunk (ctxt, buffer, 0, 1))
      {
	UT_DEBUGMSG (("Error parsing buffer\n"));
	ret = UT_IE_IMPORTERROR;
      }

  ctxt->sax = NULL;
  htmlFreeParserCtxt (ctxt);

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
      m_ParseMode = pm_HTML;
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


bool UT_XML::startDecoder ()
{
  if (m_decoder) stopDecoder ();

  xmlParserCtxtPtr parser = xmlCreatePushParserCtxt (0, 0, "<?xml version=\"1.0\"?>", 21, 0);
  xmlSubstituteEntitiesDefault (1);

  m_decoder = (void *) parser;

  return (parser != 0);
}

void UT_XML::stopDecoder ()
{
  if (m_decoder == 0) return;
  xmlFreeParserCtxt ((xmlParserCtxtPtr) m_decoder);
  m_decoder = 0;
}

/* Declared in ut_xml.h as: XML_Char * UT_XML::decode (const XML_Char * in);
 */
char * UT_XML::decode (const char * in)
{
  if (m_decoder == 0)
    if (!startDecoder ()) return 0;

  return (char *) xmlStringDecodeEntities ((xmlParserCtxtPtr) m_decoder, (XML_Char *) in, XML_SUBSTITUTE_BOTH, 0, 0, 0);
}
