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

#include <expat.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_xml.h"
#include "xap_EncodingManager.h"

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

  return ret;
}
