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

/* there is a XML_Char typedef in expat.h */
#ifdef XML_Char
#undef XML_Char
#endif

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


UT_XML::~UT_XML ()
{
  FREEP (m_namespace);

  if (m_decoder)
    {
      XML_ParserFree ((XML_Parser) m_decoder);
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
  if (!m_bSniffing) UT_ASSERT (m_pListener);
  UT_ASSERT (buffer);

  if (m_ParseMode == pm_HTML) return html (buffer, length);

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

UT_Error UT_XML::html (const char * szFilename)
{
  UT_ASSERT (m_pListener);
  UT_ASSERT (szFilename);

  UT_Error ret = UT_OK;

  UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
  ret = UT_ERROR;

  return ret;
}

UT_Error UT_XML::html (const char * buffer, UT_uint32 length)
{
  if (!m_bSniffing) UT_ASSERT (m_pListener);
  UT_ASSERT (buffer);

  UT_Error ret = UT_OK;

  UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
  ret = UT_ERROR;

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
      UT_DEBUGMSG(("expat doesn't parse HTML\n"));
      ret = false;
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


bool UT_XML::startDecoder ()
{
  if (m_decoder) stopDecoder ();
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
  m_decoder = (void *) parser;

  return (parser != 0);
}

void UT_XML::stopDecoder ()
{
  if (m_decoder == 0) return;
  XML_ParserFree ((XML_Parser) m_decoder);
  m_decoder = 0;
}

/* Declared in ut_xml.h as: XML_Char * UT_XML::decode (const XML_Char * in);
 */
char * UT_XML::decode (const char * in)
{
  if (m_decoder == 0)
    if (!startDecoder ()) return 0;
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
}
