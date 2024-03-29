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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_xml.h"

// Please keep the "/**/" to stop MSVC dependency generator complaining.
#include <libxml/parser.h>
#include <libxml/parserInternals.h>

// override typedef in ut_xml.h:
#ifdef gchar
#undef gchar
#endif
#define gchar xmlChar

static void _fatalErrorSAXFunc(void *xmlp,
                               const char *msg,
                               ...) ABI_PRINTF_FORMAT(2, 3);
static void _errorSAXFunc(void *xmlp,
			  const char *msg,
			  ...)  ABI_PRINTF_FORMAT(2, 3);


static void _startElement (void * userData, const gchar * name, const gchar ** atts)
{
  UT_XML * pXML = reinterpret_cast<UT_XML *>(userData);

  /* libxml2 can supply atts == 0, which is a little at variance to what is expected...
   */
  static const gchar * ptr = nullptr;
  const gchar ** new_atts = atts;
  if (atts == nullptr)
    new_atts = &ptr;

  pXML->startElement (reinterpret_cast<const char *>(name), reinterpret_cast<const char **>(new_atts));
}

static void _endElement (void * userData, const gchar * name)
{
  UT_XML * pXML = reinterpret_cast<UT_XML *>(userData);
  pXML->endElement (reinterpret_cast<const char *>(name));
}

static xmlEntityPtr _getEntity (void * /*userData*/, const gchar * name)
{
  return xmlGetPredefinedEntity (name);
}

static void _charData (void * userData, const gchar * buffer, int length)
{
  UT_XML * pXML = reinterpret_cast<UT_XML *>(userData);
  pXML->charData (reinterpret_cast<const char *>(buffer), length);
}

static void _processingInstruction (void * userData, const gchar * target, const gchar * data)
{
  UT_XML * pXML = reinterpret_cast<UT_XML *>(userData);
  pXML->processingInstruction (reinterpret_cast<const char *>(target), reinterpret_cast<const char *>(data));
}

static void _comment (void * userData, const gchar * data)
{
  UT_XML * pXML = reinterpret_cast<UT_XML *>(userData);
  pXML->comment (reinterpret_cast<const char *>(data));
}

static void _cdata (void * userData, const gchar * buffer, int length)
{
  UT_XML * pXML = reinterpret_cast<UT_XML *>(userData);
  pXML->cdataSection (true);
  pXML->charData (reinterpret_cast<const char *>(buffer), length);
  pXML->cdataSection (false);
}

static void _errorSAXFunc(void *xmlp,
			  const char *msg,
			  ...)
{
  va_list args;
  va_start (args, msg);
  std::string errorMessage;
  UT_std_string_vprintf(errorMessage, msg, args);
  va_end (args);
  // Handle 'nbsp' here
  UT_XML * pXML = reinterpret_cast<UT_XML *>(xmlp);
  pXML->incMinorErrors();
  char * szErr = g_strdup(errorMessage.c_str() );
  if(strstr(szErr,"'nbsp' not defined") != nullptr)
  {
    UT_DEBUGMSG(("nbsp found in stream errs %d \n",pXML->getNumMinorErrors()));
      pXML->incRecoveredErrors();
      const char buffer []= { (char)0xa0};
      pXML->charData(buffer,1); 
  }
  else if(strstr(szErr,"not defined") != nullptr)
  {
      pXML->incRecoveredErrors();
  }
  else
  {
      UT_DEBUGMSG(("SAX function error here \n"));
      UT_DEBUGMSG(("%s", errorMessage.c_str()));
// This is a runtime error, an ASSERT is out of place.
//      UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
  }
  FREEP(szErr);
}

static void _fatalErrorSAXFunc(void *xmlp,
                               const char *msg,
                               ...)
{
  va_list args;
  va_start (args, msg);
  std::string errorMessage;
  UT_std_string_vprintf(errorMessage, msg, args);
  va_end (args);
  UT_DEBUGMSG((" fatal SAX function error here \n"));

  UT_DEBUGMSG(("%s", errorMessage.c_str()));
  UT_XML * pXML = reinterpret_cast<UT_XML *>(xmlp);
  UT_DEBUGMSG((" userData pointer is %p \n", (void*)pXML));
  UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
  pXML->stop();

}


UT_Error UT_XML::parse (const char * szFilename)
{
	UT_ASSERT (m_pListener || m_pExpertListener);
	UT_ASSERT (szFilename);
	
	if ((szFilename == nullptr) || ((m_pListener == nullptr) && (m_pExpertListener == nullptr)))
		return UT_ERROR;
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
	
	xmlSAXHandler hdl;
	xmlParserCtxtPtr ctxt = nullptr;
	
	memset(&hdl, 0, sizeof(hdl));
	
	hdl.getEntity    = _getEntity;
	hdl.startElement = _startElement;
	hdl.endElement   = _endElement;
	hdl.characters   = _charData;
	hdl.error        = _errorSAXFunc;
	hdl.fatalError   = _fatalErrorSAXFunc;
	hdl.processingInstruction = _processingInstruction;
	hdl.comment      = _comment;
	hdl.cdataBlock   = _cdata;

	size_t length = reader->readBytes (buffer, sizeof (buffer));
	int done = (length < sizeof (buffer));
	
	if (length != 0)
    {
		ctxt = xmlCreatePushParserCtxt (&hdl, static_cast<void *>(this), buffer, static_cast<int>(length), szFilename);
		if (ctxt == nullptr)
		{
			UT_DEBUGMSG (("Unable to create libxml2 push-parser context!\n"));
			reader->closeFile ();
			return UT_ERROR;
		}
		xmlSubstituteEntitiesDefault (1);
		UT_sint32 chucks = -1;
		while (!done && !m_bStopped)
		{
			chucks++;
			length = reader->readBytes (buffer, sizeof (buffer));
			UT_DEBUGMSG(("Done chunk %d length %zd \n",chucks,length));
			done = (length < sizeof (buffer));
			
			if (xmlParseChunk (ctxt, buffer, static_cast<int>(length), 0))
			{
			  if(getNumMinorErrors() > getNumRecoveredErrors())
			    {
				UT_DEBUGMSG (("Error - 1 parsing '%s' (Line: %d, Column: %d)\n", szFilename, xmlSAX2GetLineNumber(ctxt), xmlSAX2GetColumnNumber(ctxt)));
				ret = UT_IE_IMPORTERROR;
				break;
			    }
			}
		}
		if (ret == UT_OK)
		  if (!m_bStopped && (getNumMinorErrors() == 0))
			{
				if (xmlParseChunk (ctxt, "", 0, 1))
				{
					UT_DEBUGMSG (("Error -2 parsing '%s' (Line: %d, Column: %d)\n", szFilename, xmlSAX2GetLineNumber(ctxt), xmlSAX2GetColumnNumber(ctxt)));
					ret = UT_IE_IMPORTERROR;
				}
			}
		if (ret == UT_OK && (getNumMinorErrors() == 0))
			if (!ctxt->wellFormed && !m_bStopped) ret = UT_IE_IMPORTERROR; // How does stopping mid-file affect wellFormed?

		xmlDocPtr myXmlDoc = ctxt->myDoc;
		xmlFreeParserCtxt (ctxt);
		xmlFreeDoc(myXmlDoc);
    }
	else
    {
		UT_DEBUGMSG(("Empty file to parse - not sure how to proceed\n"));
    }
	
	reader->closeFile ();
	
	return ret;
}


UT_Error UT_XML::parse (const char * buffer, UT_uint32 length)
{
  if (!m_bSniffing)
    {
      UT_ASSERT (m_pListener || m_pExpertListener);
      if ((m_pListener == nullptr) && (m_pExpertListener == nullptr))
        return UT_ERROR;
    }
  UT_ASSERT (buffer);
  if (buffer == nullptr || length == 0)
    return UT_ERROR;

  if (!reset_all ()) return UT_OUTOFMEM;

  UT_Error ret = UT_OK;

  xmlParserCtxtPtr ctxt;

  xmlSAXHandler hdl;
  memset(&hdl, 0, sizeof(hdl));

  hdl.getEntity    = _getEntity;
  hdl.startElement = _startElement;
  hdl.endElement   = _endElement;
  hdl.characters   = _charData;
  hdl.error        = _errorSAXFunc;
  hdl.fatalError   = _fatalErrorSAXFunc;
  hdl.processingInstruction = _processingInstruction;
  hdl.comment      = _comment;
  hdl.cdataBlock   = _cdata;

  ctxt = xmlCreateMemoryParserCtxt (buffer, static_cast<int>(length));
  if (ctxt == nullptr)
    {
      UT_DEBUGMSG (("Unable to create libxml2 memory context!\n"));
      return UT_ERROR;
    }
  memcpy(ctxt->sax, &hdl, sizeof(hdl));
  ctxt->userData = static_cast<void *>(this);

  m_bStopped = false;

  xmlParseDocument (ctxt);

  if (!ctxt->wellFormed) ret = UT_IE_IMPORTERROR;

  xmlDocPtr myXmlDoc = ctxt->myDoc;
  xmlFreeParserCtxt (ctxt);
  xmlFreeDoc(myXmlDoc);

  return ret;
}
