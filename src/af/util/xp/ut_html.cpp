/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 2001-2003 AbiSource, Inc.
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

#include "ut_std_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_html.h"

// Please keep the "/**/" to stop MSVC dependency generator complaining.
#include <libxml/parser.h>
#include <libxml/parserInternals.h>

// override typedef in ut_xml.h:
#ifdef gchar
#undef gchar
#endif
#define gchar xmlChar

UT_HTML::UT_HTML (const char * szEncoding)
{
	if (szEncoding && *szEncoding)
		{
			m_encoding = szEncoding;
			// we should be safe as it is ASCII.
			UT_tolower(m_encoding);
		}
}

UT_HTML::~UT_HTML ()
{
	// 
}

static void _startElement (void * userData, const gchar * name, const gchar ** atts)
{
	UT_HTML * pXML = static_cast<UT_HTML *>(userData);

	/* libxml2 can supply atts == 0, which is a little at variance to what is expected...
	 */
	const gchar * ptr = 0;
	const gchar ** new_atts = atts;
	if (atts == 0) new_atts = &ptr;

	pXML->startElement (reinterpret_cast<const char *>(name),
						reinterpret_cast<const char **>(new_atts));
}

static void _endElement (void * userData, const gchar * name)
{
	UT_HTML * pXML = static_cast<UT_HTML *>(userData);
	pXML->endElement (reinterpret_cast<const char *>(name));
}

static xmlEntityPtr _getEntity (void * /*userData*/, const gchar * name)
{
	return xmlGetPredefinedEntity (name);
}

static void _charData (void * userData, const gchar * buffer, int length)
{
	UT_HTML * pXML = static_cast<UT_HTML *>(userData);
	pXML->charData (reinterpret_cast<const char *>(buffer), length);
}

static void _errorSAXFunc (void * /*ctx*/, const char *msg, ...)
{
	va_list args;
	va_start (args, msg);

	std::string errorMessage;
	UT_std_string_vprintf (errorMessage, msg, args);
	va_end (args);

	UT_DEBUGMSG(("libxml2/html: error: %s", errorMessage.c_str()));
}

static void _fatalErrorSAXFunc (void * /*ctx*/, const char *msg, ...)
{
	va_list args;
	va_start (args, msg);

	std::string errorMessage;
	UT_std_string_vprintf (errorMessage, msg, args);

	va_end (args);

	UT_DEBUGMSG(("libxml2/html: fatal: %s", errorMessage.c_str()));
}


UT_Error UT_HTML::parse (const char * szFilename)
{
	if ((szFilename == 0) || (m_pListener == 0)) return UT_ERROR;

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

	htmlSAXHandler hdl;
	htmlParserCtxtPtr ctxt = 0;

	memset (&hdl, 0, sizeof (hdl));

	hdl.getEntity    = _getEntity;
	hdl.startElement = _startElement;
	hdl.endElement   = _endElement;
	hdl.characters   = _charData;
	hdl.error        = _errorSAXFunc;
	hdl.fatalError   = _fatalErrorSAXFunc;

	size_t length = reader->readBytes (buffer, sizeof (buffer));
	int done = (length < sizeof (buffer));

	if (length != 0)
		{
			xmlCharEncoding encoding = xmlParseCharEncoding (m_encoding.c_str());

			ctxt = htmlCreatePushParserCtxt (&hdl, static_cast<void *>(this),
											 buffer, static_cast<int>(length),
											 szFilename, encoding);
			if (ctxt == NULL)
				{
					UT_DEBUGMSG (("Unable to create libxml2 push-parser context!\n"));
					reader->closeFile ();
					return UT_ERROR;
				}
			xmlSubstituteEntitiesDefault (1);

			while (!done && !m_bStopped)
				{
					length = reader->readBytes (buffer, sizeof (buffer));
					done = (length < sizeof (buffer));
	  
					if (htmlParseChunk (ctxt, buffer, static_cast<int>(length), 0))
						{
							UT_DEBUGMSG (("Error parsing '%s' (Line: %d, Column: %d)\n",
										  szFilename, xmlSAX2GetLineNumber(ctxt),
										  xmlSAX2GetColumnNumber(ctxt)));
							ret = UT_IE_IMPORTERROR;
							break;
						}
				}
			if (ret == UT_OK)
				if (!m_bStopped)
					{
						if (htmlParseChunk (ctxt, 0, 0, 1))
							{
								UT_DEBUGMSG (("Error parsing '%s' (Line: %d, Column: %d)\n",
											  szFilename, xmlSAX2GetLineNumber(ctxt),
											  xmlSAX2GetColumnNumber(ctxt)));
								ret = UT_IE_IMPORTERROR;
							}
					}
			if (ret == UT_OK)
				if (!ctxt->wellFormed && !m_bStopped)
					ret = UT_IE_IMPORTERROR; // How does stopping mid-file affect wellFormed?

			ctxt->sax = NULL;
			htmlFreeParserCtxt (ctxt);
		}
	else
		{
			UT_DEBUGMSG(("Empty file to parse - not sure how to proceed\n"));
		}

	reader->closeFile ();

	return ret;
}

UT_Error UT_HTML::parse (const char * buffer, UT_uint32 length)
{
	if ((buffer == 0) || (length < 6) || (m_pListener == 0)) return UT_ERROR;

	UT_XML::Reader * reader = m_pReader;

	UT_XML_BufReader wrapper(buffer,length);
	setReader (&wrapper);

	UT_Error ret = parse ("");

	setReader (reader);

	return ret;
}
