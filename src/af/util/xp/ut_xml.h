/* AbiSource Program Utilities
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

/* Pseudoheader to include the right XML headers */

#ifndef UTXML_H
#define UTXML_H

#include "ut_types.h"
#include "ut_bytebuf.h"

#ifdef HAVE_LIBXML2
  // Please keep the "/**/" to stop MSVC dependency generator complaining.
  #include /**/ <libxml/parser.h>

  /* XML_Char is the expat char def, xmlChar is the libxml2 def. I think they have
   * different sign - which should be okay so long as expat isn't working in Unicode
   * mode (in which case we're stuck)
   */

  #ifdef XML_Char
  #undef XML_Char
  #endif
  #define XML_Char xmlChar

#else /* EXPAT */
  #include <expat.h>

#endif

class ABI_EXPORT UT_XML
{
 public:
  UT_XML();
  ~UT_XML();

  UT_Error parse (const char * szFilename);
  UT_Error parse (const UT_ByteBuf * pBB);
  UT_Error parse (const char * buffer, UT_uint32 length);

  class Listener
    {
    public:
      virtual ~Listener () { };

      virtual void startElement (const XML_Char * name, const XML_Char ** atts) = 0;
      virtual void endElement (const XML_Char * name) = 0;
      virtual void charData (const XML_Char * buffer, int length) = 0;
    };

  void setListener (Listener * pListener) { m_pListener = pListener; }

  class Reader
    {
    public:
      virtual ~Reader () { };

      virtual bool	openFile (const char * szFilename) = 0;
      virtual UT_uint32	readBytes (char * buffer, UT_uint32 length) = 0;
      virtual void	closeFile (void) = 0;
    };

  void setReader (Reader * pReader) { m_pReader = pReader; }

 protected:
  UT_Error load_then_parse (const char * szFilename);

 private:
  Listener * m_pListener;
  Reader * m_pReader;
};

/* Kudos (sp?) to Joaquin Cuenca Abela for the good bits; the bad bits are likely my fault... (fjf)
 * 
 * class IE_Imp_AbiWord_1 : public IE_Imp, public UT_XML::Listener
 * {
 * 
 * public:
 * 
 *    void some_kind_of_start_the_whole_thing_method()
 *    { ... xmlParser.setListener(this);
 *          xmlParser.parse("blah.xml"); // and magically my methods are called
 *      ...
 *    }
 * 
 *    virtual void startElement(const XML_Char* name, const XML_Char **atts)
 *    {...}
 * 
 *    virtual void endElement(const XML_Char *name)
 *    {...}
 * 
 *    virtual void charData(const XML_Char*, int)
 *    {...}
 * 
 * private:
 *    UT_XML xmlParser;
 * 
 * };
 */

/* The following was in ie_imp_XML.cpp:
 */
        // TODO - remove this then not needed anymore. In ver 0.7.7 and erlier, AbiWord export inserted 
        // chars below 0x20. Most of these are invalid XML and can't be imported.
        // See bug #762.
/*      for( UT_uint32 n1 = 0; n1 < len; n1++ )
 *	        if( buf[n1] >= 0x00 && buf[n1] < 0x20 && buf[n1] != 0x09 && buf[n1] != 0x0a && buf[n1] != 0x0d )
 *		        buf[n1] = 0x0d;
 * 
 * This work around is not included in ut_xml.cpp, but could be re-added to the expat importer.
 * So: Is it still needed?
 */

#endif
