/* AbiSource Program Utilities
 * Copyright (C) 2001,2002 Francis James Franklin <fjf@alinameridon.com>
 * Copyright (C) 2001,2002 AbiSource, Inc.
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

#ifndef XML_Char
typedef char XML_Char;
#endif

ABI_EXPORT char * UT_XML_Decode( const char * inKey );

class ABI_EXPORT UT_XML
{
 public:
  UT_XML();
  ~UT_XML();

  /* Strip "svg:" from "svg:svg" etc. in element names, pass any other namespace indicators
   */
  void setNameSpace (const char * xml_namespace);

 private:
  char * m_chardata_buffer;

  UT_uint32 m_chardata_length;
  UT_uint32 m_chardata_max;

  bool grow (char *& buffer, UT_uint32 & length, UT_uint32 & max, UT_uint32 require);

  bool reset_all ();
  void flush_all ();

  const char * m_namespace;
  int m_nslength;

 public:
  /* Returns true iff the name of the first element is xml_type or opt_namespace:xml_type
   */
  bool sniff (const UT_ByteBuf * pBB, const char * xml_type);
  bool sniff (const char * buffer, UT_uint32 length, const char * xml_type);

 private:
  bool m_bSniffing;
  bool m_bValid;

  const char * m_xml_type;

 public:
  UT_Error parse (const char * szFilename);
  UT_Error parse (const UT_ByteBuf * pBB);
  UT_Error parse (const char * buffer, UT_uint32 length);

 public:
  void stop () { m_bStopped = true; } // call this to stop callbacks and to stop the feed to the parser

 private:
  bool m_bStopped;

 public:
  class ABI_EXPORT Listener
    {
    public:
      virtual ~Listener () { };

      virtual void startElement (const XML_Char * name, const XML_Char ** atts) = 0;
      virtual void endElement (const XML_Char * name) = 0;
      virtual void charData (const XML_Char * buffer, int length) = 0;
    };

  void setListener (Listener * pListener) { m_pListener = pListener; }

 private:
  Listener * m_pListener;

 public:
  class ABI_EXPORT Reader
    {
    public:
      virtual ~Reader () { };

      virtual bool	openFile (const char * szFilename) = 0;
      virtual UT_uint32	readBytes (char * buffer, UT_uint32 length) = 0;
      virtual void	closeFile (void) = 0;
    };

  void setReader (Reader * pReader) { m_pReader = pReader; }

 private:
  Reader * m_pReader;

 public:
  /* For UT_XML internal use only.
   * 
   * However, it should be possible to set up redirections from one UT_XML into another, if multiple
   * namespaces require it - maybe something like:
   * 
   * UT_XML xhtml;
   * xhtml.setListener (xhtml_listener);
   * xhtml.setNameSpace ("xhtml");
   * 
   * UT_XML svg;
   * svg.setListener (svg_listener);
   * svg.setNameSpace ("svg");
   * svg.redirectNameSpace (&xhtml,"xhtml"); // Not yet implemented...
   * svg.parse (buffer);
   * 
   * or vice versa... ?
   */
  void startElement (const XML_Char * name, const XML_Char ** atts);
  void endElement (const XML_Char * name);
  void charData (const XML_Char * buffer, int length);
};

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
