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

// override typedef in ut_xml.h:
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

UT_Error UT_XML::parse (const char * szFilename)
{
  UT_ASSERT (m_pListener);
  UT_ASSERT (szFilename);

  if ((szFilename == 0) || (m_pListener == 0)) return UT_ERROR;
  if (!reset_all ()) return UT_OUTOFMEM;

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
  xmlParserCtxtPtr ctxt = 0;

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
  if (!m_bSniffing)
    {
      UT_ASSERT (m_pListener);
      if (m_pListener == 0) return UT_ERROR;
    }
  UT_ASSERT (buffer);
  if (buffer == 0) return UT_ERROR;

  if (!reset_all ()) return UT_OUTOFMEM;

  UT_Error ret = UT_OK;

  xmlSAXHandler hdl;
  xmlParserCtxtPtr ctxt;

  memset(&hdl, 0, sizeof(hdl));

  hdl.getEntity = _getEntity;
  hdl.startElement = _startElement;
  hdl.endElement = _endElement;
  hdl.characters = _charData;

  ctxt = xmlCreateMemoryParserCtxt (const_cast<char *>(buffer), (int) length);
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
