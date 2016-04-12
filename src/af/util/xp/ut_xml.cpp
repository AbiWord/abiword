/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
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
#ifdef _WIN32
#include <windows.h>
#endif

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_xml.h"

#include "ut_misc.h"
#include "ut_string_class.h"

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
#ifdef _WIN32
	WCHAR wFilename[MAX_PATH];
	MultiByteToWideChar(CP_UTF8,0,szFilename,-1,wFilename,MAX_PATH);
	in = _wfopen (wFilename, L"r");
#else
	in = fopen (szFilename, "r");
#endif
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

UT_XML_BufReader::UT_XML_BufReader (const char * buffer, UT_uint32 length) :
  m_buffer(buffer),
  m_bufptr(0),
  m_length(length)
{
  // 
}

UT_XML_BufReader::~UT_XML_BufReader ()
{
  // 
}

bool UT_XML_BufReader::openFile (const char * /*szFilename*/)
{
  if ((m_buffer == 0) || (m_length == 0)) return false;
  m_bufptr = m_buffer;
  return true;
}

UT_uint32 UT_XML_BufReader::readBytes (char * buffer, UT_uint32 length)
{
  if ((buffer == 0) || (length == 0)) return 0;

  UT_uint32 bytes = (m_buffer + m_length) - m_bufptr;
  if (bytes > length) bytes = length;
  memcpy (buffer, m_bufptr, bytes);
  m_bufptr += bytes;

  return bytes;
}

void UT_XML_BufReader::closeFile ()
{
  m_bufptr = 0;
}

UT_XML::UT_XML () :
  m_is_chardata(true),
  m_chardata_buffer(0),
  m_chardata_length(0),
  m_chardata_max(0),
  m_iMinorErrors(0),
  m_iRecoveredErrors(0),
  m_namespace(0),
  m_nslength(0),
  m_bSniffing(false),
  m_bValid(false),
  m_xml_type(0),
  m_bStopped(false),
  m_pListener(0),
  m_pExpertListener(0),
  m_pReader(0)
{
}

UT_XML::~UT_XML ()
{
  if (m_chardata_buffer) g_free (m_chardata_buffer);

  FREEP (m_namespace);
}

bool UT_XML::grow (char *& buffer, UT_uint32 & length, UT_uint32 & max, UT_uint32 require)
{
  if (length + require + 1 <= max) return true;

  if (buffer == 0)
    {
      buffer = static_cast<char *>(g_try_malloc (require + 1));
      if (buffer == 0) return false;
      buffer[0] = 0;
      max = require + 1;
      return true;
    }
  char * more = static_cast<char *>(g_try_realloc (buffer, max + require + 1));
  if (more == 0) return false;
  buffer = more;
  max += require + 1;
  return true;
}

bool UT_XML::reset_all ()
{
  m_chardata_length = 0;
  // 

  if (!grow (m_chardata_buffer, m_chardata_length, m_chardata_max, 64)) return false;
  // 

  return true;
}

void UT_XML::flush_all ()
{
  if (m_chardata_length)
    {
      if (m_pListener && m_is_chardata)
		  m_pListener->charData (m_chardata_buffer, m_chardata_length);
      if (m_pExpertListener)
		  {
			  if (m_is_chardata)
				  m_pExpertListener->CharData (m_chardata_buffer, m_chardata_length);
			  else
				  m_pExpertListener->Default (m_chardata_buffer, m_chardata_length);
		  }
      m_chardata_length = 0;
    }
}

/* Declared in ut_xml.h as: void UT_XML::startElement (const gchar * name, const gchar ** atts);
 */
void UT_XML::startElement (const char * name, const char ** atts)
{
  if (m_bStopped) return;

  flush_all ();

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

  UT_ASSERT (m_pListener || m_pExpertListener);
  if (m_pListener)
	  m_pListener->startElement (name, atts);
  if (m_pExpertListener)
	  m_pExpertListener->StartElement (name, atts);
}

/* Declared in ut_xml.h as: void UT_XML::endElement (const gchar * name);
 */
void UT_XML::endElement (const char * name)
{
  if (m_bStopped) return;

  flush_all ();

  if (m_nslength)
    if (strncmp (name,m_namespace,m_nslength) == 0)
      {
	if (*(name + m_nslength) == ':') name += m_nslength + 1;
      }

  UT_ASSERT (m_pListener || m_pExpertListener);
  if (m_pListener)
	  m_pListener->endElement (name);
  if (m_pExpertListener)
	  m_pExpertListener->EndElement (name);
}

/* Declared in ut_xml.h as: void UT_XML::charData (const gchar * buffer, int length);
 */
void UT_XML::charData (const char * buffer, int length)
{
  if (m_bStopped) return;
  xxx_UT_DEBUGMSG(("In UT_XML::charData \n"));
  if (m_chardata_length && !m_is_chardata) flush_all ();

  m_is_chardata = true;

  if (!grow (m_chardata_buffer, m_chardata_length, m_chardata_max, length))
    {
      m_bStopped = true;
      return;
    }
  memcpy (m_chardata_buffer + m_chardata_length, buffer, length);
  m_chardata_length += length;
  m_chardata_buffer[m_chardata_length] = 0;
}

void UT_XML::processingInstruction (const char * target, const char * data)
{
  if (m_bStopped) return;
  if (m_pExpertListener)
	{
	  if (m_chardata_length) flush_all ();
	  m_pExpertListener->ProcessingInstruction (target, data);
	}
}

void UT_XML::comment (const char * data)
{
  if (m_bStopped) return;
  if (m_pExpertListener)
	{
	  if (m_chardata_length) flush_all ();
	  m_pExpertListener->Comment (data);
	}
}

void UT_XML::cdataSection (bool start)
{
  if (m_bStopped) return;
  if (m_pExpertListener)
	{
	  if (m_chardata_length) flush_all ();
	  if (start)
		m_pExpertListener->StartCdataSection ();
	  else
		m_pExpertListener->EndCdataSection ();
	}
}

/* Declared in ut_xml.h as: void UT_XML::defaultData (const gchar * buffer, int length);
 */
void UT_XML::defaultData (const char * buffer, int length)
{
  if (m_bStopped) return;
  if (m_pExpertListener == 0) return;

  if (m_chardata_length && m_is_chardata) flush_all ();

  m_is_chardata = false;

  if (!grow (m_chardata_buffer, m_chardata_length, m_chardata_max, length))
    {
      m_bStopped = true;
      return;
    }
  memcpy (m_chardata_buffer + m_chardata_length, buffer, length);
  m_chardata_length += length;
  m_chardata_buffer[m_chardata_length] = 0;
}

/* I'm still very confused about XML namespaces so the handling here is likely to change a lot as I learn...
 */
void UT_XML::setNameSpace (const char * xml_namespace)
{
  FREEP (m_namespace);
  if (xml_namespace) m_namespace = g_strdup (xml_namespace);

  m_nslength = 0;
  if (m_namespace) m_nslength = strlen (m_namespace);
}

bool UT_XML::sniff (const UT_ByteBuf * pBB, const char * xml_type)
{
  UT_ASSERT (pBB);
  UT_ASSERT (xml_type);

  if ((pBB == 0) || (xml_type == 0)) return false;

  const char * buffer = reinterpret_cast<const char *>(pBB->getPointer (0));
  UT_uint32 length = pBB->getLength ();

  return sniff (buffer, length, xml_type);
}

bool UT_XML::sniff (const char * buffer, UT_uint32 length, const char * xml_type)
{
  UT_ASSERT (buffer);
  UT_ASSERT (xml_type);

  if ((buffer == 0) || (xml_type == 0)) return false;

  m_bSniffing = true; // This *must* be reset to false before returning
  m_bValid = true;

  m_xml_type = xml_type;

  bool valid = false;
  if (parse (buffer, length) == UT_OK) valid = m_bValid;

  m_bSniffing = false;
  return valid;
}

UT_Error UT_XML::parse (const UT_ByteBuf * pBB)
{
  UT_ASSERT (m_pListener || m_pExpertListener);
  UT_ASSERT (pBB);

  if ((pBB == 0) || ((m_pListener == 0) && (m_pExpertListener == 0))) return UT_ERROR;
  if (!reset_all ()) return UT_OUTOFMEM;

  const char * buffer = reinterpret_cast<const char *>(pBB->getPointer (0));
  UT_uint32 length = pBB->getLength ();

  return parse (buffer, length);
}

/**************************************************************/
/**************************************************************/

class ABI_EXPORT UT_XML_Decoder : public UT_XML::Listener
{
public:

	UT_XML_Decoder () {}
	virtual ~UT_XML_Decoder () {}

	virtual void startElement (const gchar * /*name*/, const gchar ** atts)
	{
		mKey = UT_getAttribute ( "k", atts ) ;
	}
	
	virtual void endElement (const gchar * /*name*/)
	{
	}
	
	virtual void charData (const gchar * /*buffer*/, int /*length*/)
	{
	}
	
	const UT_String & getKey () const { return mKey ; }
	
private:
	UT_String mKey ;
} ;

char * UT_XML_Decode ( const char * inKey )
{
	UT_XML parser ;

	UT_XML_Decoder decoder ;
	
	parser.setListener ( &decoder ) ;

	UT_String toDecode ;

	toDecode = "<?xml version=\"1.0\"?>\n" ;
	toDecode += "<d k=\"";
	toDecode += inKey ;
	toDecode += "\"/>" ;
	
	parser.parse ( toDecode.c_str(), toDecode.size () ) ;

	char * to_return = g_strdup(decoder.getKey ().c_str());
	xxx_UT_DEBUGMSG(("DOM: returning %s from %s\n", to_return, inKey));	
	return to_return ;
}
