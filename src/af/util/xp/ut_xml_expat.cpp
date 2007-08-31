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

#if defined(_WIN32)
#define XML_STATIC
#define COMPILED_FROM_DSP
#endif /* _WIN32 */

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#ifndef gchar
typedef gchar gchar;
#endif

#include <expat.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_xml.h"
#include "xap_EncodingManager.h"


static void _startElement (void * userData, const gchar * name, const gchar ** atts)
{
  UT_XML * pXML = reinterpret_cast<UT_XML *>(userData);

  /* libxml2 can supply atts == 0, which is a little at variance to what is expected...
   */
  const gchar * ptr = 0;
  const gchar ** new_atts = atts;
  if (atts == 0) new_atts = &ptr;

  pXML->startElement (static_cast<const char *>(name), static_cast<const char **>(new_atts));
}

static void _endElement (void * userData, const gchar * name)
{
  UT_XML * pXML = reinterpret_cast<UT_XML *>(userData);
  pXML->endElement (static_cast<const char *>(name));
}

static void _charData (void * userData, const gchar * buffer, int length)
{
  UT_XML * pXML = reinterpret_cast<UT_XML *>(userData);
  pXML->charData (static_cast<const char *>(buffer), length);
}

static void _processingInstruction (void * userData, const gchar * target, const gchar * data)
{
  UT_XML * pXML = reinterpret_cast<UT_XML *>(userData);
  pXML->processingInstruction (static_cast<const char *>(target), static_cast<const char *>(data));
}

static void _comment (void * userData, const gchar * data)
{
  UT_XML * pXML = reinterpret_cast<UT_XML *>(userData);
  pXML->comment (static_cast<const char *>(data));
}

static void _startCdataSection (void * userData)
{
  UT_XML * pXML = reinterpret_cast<UT_XML *>(userData);
  pXML->cdataSection (true);
}

static void _endCdataSection (void * userData)
{
  UT_XML * pXML = reinterpret_cast<UT_XML *>(userData);
  pXML->cdataSection (false);
}

static void _default (void * userData, const gchar * buffer, int length)
{
  UT_XML * pXML = reinterpret_cast<UT_XML *>(userData);
  pXML->defaultData (static_cast<const char *>(buffer), length);
}


UT_Error UT_XML::parse (const char * szFilename)
{
  UT_ASSERT (m_pListener || m_pExpertListener);
  UT_ASSERT (szFilename);

  if ((szFilename == 0) || ((m_pListener == 0) && (m_pExpertListener == 0))) return UT_ERROR;
  if (!reset_all ()) return UT_OUTOFMEM;

  UT_Error ret = UT_OK;

  DefaultReader defaultReader;
  Reader * reader = &defaultReader;
  if (m_pReader)
	  reader = m_pReader;

  if (!reader->openFile (szFilename))
    {
      UT_DEBUGMSG (("Could not open file %s\n", szFilename));
      return UT_errnoToUTError ();
    }

  char buffer[2048];

  m_bStopped = false;

  XML_Parser parser = XML_ParserCreate (NULL);
  if (parser == NULL)
    {
      UT_DEBUGMSG (("Unable to create expat parser!\n"));
      reader->closeFile ();
      return UT_ERROR;
    }

  XML_SetUnknownEncodingHandler (parser, reinterpret_cast<XML_UnknownEncodingHandler>(XAP_EncodingManager::XAP_XML_UnknownEncodingHandler), 0);

  XML_SetElementHandler (parser, _startElement, _endElement);
  XML_SetCharacterDataHandler (parser, _charData);
  XML_SetProcessingInstructionHandler (parser, _processingInstruction);
  XML_SetCommentHandler (parser, _comment);
  XML_SetCdataSectionHandler (parser, _startCdataSection, _endCdataSection);
  XML_SetDefaultHandler (parser, _default);

  XML_SetUserData (parser, static_cast<void *>(this));

  int done = 0;
  while (!done && !m_bStopped)
    {
      size_t length = reader->readBytes (buffer, sizeof (buffer));
      done = (length < sizeof (buffer));

      if (!XML_Parse (parser, buffer, length, done))
	{
	  UT_WARNINGMSG(("Parse error loading file %s, %s at line %d\n", szFilename, XML_ErrorString (XML_GetErrorCode (parser)), XML_GetCurrentLineNumber (parser)));
	  ret = UT_IE_IMPORTERROR;
	  break;
	}
    }

  XML_ParserFree (parser);

  reader->closeFile ();

  return ret;
}

UT_Error UT_XML::parse (const char * buffer, UT_uint32 length)
{
  if (!m_bSniffing)
    {
      UT_ASSERT (m_pListener || m_pExpertListener);
      if ((m_pListener == 0) && (m_pExpertListener == 0)) return UT_ERROR;
    }
  UT_ASSERT (buffer);
  if (buffer == 0) return UT_ERROR;

  if (!reset_all ()) return UT_OUTOFMEM;

  UT_Error ret = UT_OK;

  XML_Parser parser = XML_ParserCreate (NULL);
  if (parser == NULL)
    {
      UT_DEBUGMSG (("Unable to create expat parser!\n"));
      return UT_ERROR;
    }

  XML_SetUnknownEncodingHandler (parser, reinterpret_cast<XML_UnknownEncodingHandler>(XAP_EncodingManager::XAP_XML_UnknownEncodingHandler), 0);

  XML_SetElementHandler (parser, _startElement, _endElement);
  XML_SetCharacterDataHandler (parser, _charData);
  XML_SetProcessingInstructionHandler (parser, _processingInstruction);
  XML_SetCommentHandler (parser, _comment);
  XML_SetCdataSectionHandler (parser, _startCdataSection, _endCdataSection);
  XML_SetDefaultHandler (parser, _default);

  XML_SetUserData (parser, static_cast<void *>(this));

  m_bStopped = false;

  if (!XML_Parse (parser, buffer, static_cast<int>(length), 1))
    {
      UT_WARNINGMSG(("Parse error, %s at line %d\n", XML_ErrorString (XML_GetErrorCode (parser)), XML_GetCurrentLineNumber (parser)));
      ret = UT_IE_IMPORTERROR;
    }

  XML_ParserFree (parser);

  return ret;
}

void UT_XML::_init()
{
}

void UT_XML::_cleanup()
{
}
