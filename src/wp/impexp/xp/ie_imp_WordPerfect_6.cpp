/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001 William Lachance (wlach@interlog.com)
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

#include "xap_EncodingManager.h"

#include "ut_string_class.h"
#include "ut_string.h"
#include "pd_Document.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_types.h"
#include "ut_growbuf.h"
#include "pt_Types.h"

#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_Frame.h"
#include "ap_Strings.h"

#include "ie_imp_WordPerfect_6.h"

#define X_CheckError(v)			do { if (!(v)) return 1; } while (0)

WordPerfectTextAttributes::WordPerfectTextAttributes()
{
   m_extraLarge = false;
   m_veryLarge = false;
   m_large = false;
   m_smallPrint = false;
   m_finePrint = false;
   m_superScript = false;
   m_subScript = false;
   m_outline = false;
   m_italics = false;
   m_shadow = false;
   m_redLine = false;
   m_bold = false;
   m_strikeOut = false;
   m_underLine = false;
   m_smallCaps = false;
   m_Blink = false;
   m_reverseVideo = false;
}



/****************************************************************************/
/****************************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

ABI_FAR extern "C"
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Imp_WordPerfect_6_Sniffer ();
	}
	else
	{
		m_sniffer->ref();
	}

	UT_ASSERT (m_sniffer);

	mi->name    = "WordPerfect (tm) Importer";
	mi->desc    = "WordPerfect (tm) Documents";
	mi->version = ABI_VERSION_STRING;
	mi->author  = "Abi the Ant";
	mi->usage   = "No Usage";

	IE_Imp::registerImporter (m_sniffer);
	return 1;
}

ABI_FAR extern "C"
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name    = 0;
	mi->desc    = 0;
	mi->version = 0;
	mi->author  = 0;
	mi->usage   = 0;

	UT_ASSERT (m_sniffer);

	IE_Imp::unregisterImporter (m_sniffer);
	if (!m_sniffer->unref())
	{
		m_sniffer = 0;
	}

	return 1;
}

ABI_FAR extern "C"
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
								 UT_uint32 release)
{
	return isCurrentAbiVersion (major, minor, release) ? 1 : 0;
}

#endif /* ENABLE_PLUGINS */

/****************************************************************************/
/****************************************************************************/

bool IE_Imp_WordPerfect_6_Sniffer::recognizeContents (const char * szBuf, 
						     UT_uint32 iNumbytes)
{
   
	char * magic    = 0;
	int magicoffset = 0;

	magic = "WPC";
	magicoffset = 1;
	if (iNumbytes > (magicoffset + strlen (magic)))
	{
	   if (!strncmp (szBuf + magicoffset, magic, strlen (magic)))
	     {
		return true;
	     }
	}

   // ok, that didn't work, we'll try to dig through the OLE stream
   // (TODO)
   return false;
}

bool IE_Imp_WordPerfect_6_Sniffer::recognizeSuffix (const char * szSuffix)
{
	// We recognize both word documents and their template versions
	return (!UT_stricmp(szSuffix,".wpd"));
}

UT_Error IE_Imp_WordPerfect_6_Sniffer::constructImporter (PD_Document * pDocument,
													  IE_Imp ** ppie)
{
	IE_Imp_WordPerfect_6 * p = new IE_Imp_WordPerfect_6(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_WordPerfect_6_Sniffer::getDlgLabels (const char ** pszDesc,
												const char ** pszSuffixList,
												IEFileType * ft)
{
	*pszDesc = "WordPerfect (.wpd)";
	*pszSuffixList = "*.wpd";
	*ft = getFileType();
	return true;
}

/****************************************************************************/
/****************************************************************************/

// just buffer sizes, arbitrarily chosen
#define DOC_TEXTRUN_SIZE 2048
#define DOC_PROPBUFFER_SIZE 1024

IE_Imp_WordPerfect_6::IE_Imp_WordPerfect_6(PD_Document * pDocument)
  : IE_Imp (pDocument)
{
}

/****************************************************************************/
/****************************************************************************/

UT_Error IE_Imp_WordPerfect_6::importFile(const char * szFilename)
{//Christian in Quebec: _654-2941
   FILE *fp = fopen(szFilename, "rb");
   if (!fp)
     {
	UT_DEBUGMSG(("Could not open file %s\n",szFilename));
	return UT_IE_FILENOTFOUND;
     }

   //ImportStream *pStream = v;
   //UT_UCSChar *hello = (UT_UCSChar *)"Hello, WP importer!";
   //getDoc()->appendSpan (hello, strlen((char *)hello));
   getDoc()->appendStrux(PTX_Section, NULL);
   getDoc()->appendStrux(PTX_Block, NULL);
   fseek(fp, 4, SEEK_SET); // push past the header
   long documentPointer;
   fread(&documentPointer, 4, 1, fp);
   fseek(fp, documentPointer, SEEK_SET);
   UT_DEBUGMSG(("WordPerfect: Document Pointer = %i \n",(int)documentPointer));
   extractFile(fp);
     fclose(fp);
   //delete(pStream);
   
   return UT_OK;
}

void IE_Imp_WordPerfect_6::pasteFromBuffer (PD_DocumentRange *, 
										unsigned char *, unsigned int, const char *)
{
	// nada
}

void IE_Imp_WordPerfect_6::extractFile(FILE *fp)
{
   wchar_t wc = 0;
   
   int readVal = fgetc(fp);

   while (readVal != EOF)
     {
	switch (readVal)
	  { 
	   case WP_TOP_HARD_HYPHEN:
	     m_Mbtowc.mbtowc(wc, '-');
	     m_textBuf.append( &(UT_uint16)wc, 1);
	     break;
	   case WP_TOP_SOFT_SPACE:
	     m_Mbtowc.mbtowc(wc, ' ');
	     m_textBuf.append( &(UT_uint16)wc, 1);
	     break;
	   case WP_TOP_SOFT_EOL:
	     m_Mbtowc.mbtowc(wc, ' ');
	     m_textBuf.append( &(UT_uint16)wc, 1);
	     break;
	   case WP_TOP_DORMANT_HARD_RETURN:
	   case WP_TOP_HARD_EOL: 
	     _handleHardEndOfLine();
	     break;
	   case WP_TOP_EOL_GROUP: 
	     _handleEndOfLineGroup(fp);
	     break;
	   case WP_TOP_CHARACTER_GROUP:
	     _handleCharacterGroup(fp);
	     break;
	   case WP_TOP_PAGE_GROUP:
	     _handlePageGroup(fp);
	     break;
	   case WP_TOP_COLUMN_GROUP:
	     _handleColumnGroup(fp);
	     break;
	   case WP_TOP_PARAGRAPH_GROUP:
	     _handleParagraphGroup(fp);
	     break;
	   case WP_TOP_FOOTENDNOTE_GROUP:
	     _handleFootEndNoteGroup(fp);
	     break;
	   case WP_TOP_STYLE_GROUP:
  	     _handleStyleGroup(fp);
	     break;
	   case WP_TOP_TAB_GROUP:
  	     _handleTabGroup(fp);
	     break;
	   case WP_TOP_EXTENDED_CHARACTER:
	     _handleExtendedCharacter(fp);
	     break;
	   case WP_TOP_ATTRIBUTE_ON:
	     _handleAttribute(fp, true);
	     break;
	   case WP_TOP_ATTRIBUTE_OFF:
	     _handleAttribute(fp, false);
	     break;
	   default:	     
	     if( readVal > 32 && readVal < 127 ) // ASCII characters
	       {
		  //UT_DEBUGMSG((" current char = %c \n",(char)readVal));
		  m_Mbtowc.mbtowc(wc, (char)readVal);
		  m_textBuf.append( &(UT_uint16)wc, 1);
	       }	     
	     break;
	  }
	
	readVal = fgetc(fp);
     }
   
   if(m_textBuf.getLength() > 0)   
     getDoc()->appendSpan(m_textBuf.getPointer(0), m_textBuf.getLength());
}

void IE_Imp_WordPerfect_6::_handleHardEndOfLine()
{
   // (TODO: eliminate a prev space if it's just before this)
   _flushText();
   getDoc()->appendStrux(PTX_Block, NULL);
}

// handles an end-of-line function that has formatting information embedded
// into it. Fortunately, only the first code is relevant for our purposes..
// the rest can be safely skipped (at least according to the developer documentation)
void IE_Imp_WordPerfect_6::_handleEndOfLineGroup(FILE *fp)
{
   int readVal = fgetc(fp); // TODO: handle case that we get eof?
   //UT_DEBUGMSG((" MB: %i\n", readVal));
   wchar_t wc = 0;
   
   switch (readVal)
     {
      case 0: // 0x00 (beginning of file)
	break; // ignore
      case 1: // 0x01 (soft EOL)
      case 2: // 0x02 (soft EOC) 
      case 3: // 0x03 (soft EOC at EOP) 
      case 20: // 0x014 (deletable soft EOL)
      case 21: // 0x15 (deletable soft EOC) 
      case 22: // 0x16 (deleteable soft EOC at EOP) 
	m_Mbtowc.mbtowc(wc, ' ');
	m_textBuf.append( &(UT_uint16)wc, 1);
	break;
      case 4: // 0x04 (hard end-of-line)
      case 5: // 0x05 (hard EOL at EOC) 
      case 6: // 0x06 (hard EOL at EOP)
      case 23: // 0x17 (deletable hard EOL)
      case 24: // 0x18 (deletable hard EOL at EOC)
      case 25: // 0x19 (deletable hard EOL at EOP)	
	_handleHardEndOfLine();
	break;
      case 9: // hard EOP (TODO: implement me)
      case 28: // deletable hard EOP (TODO: treat as a hard end-of-page)
      default: // something else we don't support yet
	break;
     }
   
   _skipGroup(fp, WP_TOP_EOL_GROUP);      
}

// handles a page group
// (TODO: not implemented, just skips over it)
void IE_Imp_WordPerfect_6::_handlePageGroup(FILE *fp)
{
   _skipGroup(fp, WP_TOP_PAGE_GROUP);
}

// handles a column group
// (TODO: not implemented, just skips over it)
void IE_Imp_WordPerfect_6::_handleColumnGroup(FILE *fp)
{
   _skipGroup(fp, WP_TOP_COLUMN_GROUP);
}

// handles a paragraph group
// (TODO: not implemented, just skips over it)
void IE_Imp_WordPerfect_6::_handleParagraphGroup(FILE *fp)
{
   _skipGroup(fp, WP_TOP_PARAGRAPH_GROUP);
}


// handles a style group
// (TODO: not implemented, just skips over it)
void IE_Imp_WordPerfect_6::_handleStyleGroup(FILE *fp)
{
   _skipGroup(fp, WP_TOP_STYLE_GROUP);
}

// handles a tab group
// (TODO: not implemented, just skips over it)
void IE_Imp_WordPerfect_6::_handleTabGroup(FILE *fp)
{
   _skipGroup(fp, WP_TOP_TAB_GROUP);
}

// handles a tab group
// (TODO: not implemented, just skips over it)
void IE_Imp_WordPerfect_6::_handleFootEndNoteGroup(FILE *fp)
{
   _skipGroup(fp, WP_TOP_FOOTENDNOTE_GROUP);
}

// handles an attribute byte in wordperfect. if attributeOn is true,
// turn the attribute on, if attributeOn is false, turn the attribute
// off.
void IE_Imp_WordPerfect_6::_handleAttribute(FILE *fp, bool attributeOn)
{
   // flush what's come before this change
   _flushText();

   int readVal = fgetc(fp); // TODO: handle case that we get eof?
   switch (readVal)
     { 
      case 14: // underline
	m_textAttributes.m_underLine = attributeOn;
	break;
      case 12: // bold
	m_textAttributes.m_bold = attributeOn;
	break;
      default: // something we don't support yet
	break;
     }
   _appendCurrentFormat();
   
   // read the ending byte
   readVal = fgetc(fp); // TODO: handle case that we get eof? in case we don't get a closing byte?
}

void IE_Imp_WordPerfect_6::_handleCharacterGroup(FILE *fp)
{
   // TODO: figure out what this function does, implement it
   // (right now it just skips over it)
   _skipGroup(fp, WP_TOP_CHARACTER_GROUP);
}

void IE_Imp_WordPerfect_6::_handleExtendedCharacter(FILE *fp)
{
   wchar_t wc = 0;
   int character = fgetc(fp);
   int characterSet = fgetc(fp);
   
   // TODO: hack, hack, hack
   // find out how to reliably map ALL characters between character sets and extended characters
   if( character == 28 && characterSet == 4 )
     wc = 39; // character: '
   else
     wc = m_Mbtowc.mbtowc(wc, ' ');
   
   m_textBuf.append( &(UT_uint16)wc, 1);
     
   int readVal = fgetc(fp); // TODO: check for eof and also that the end byte is the extended character flag
}

// skips over a group (sequences of bytes which begin with a value and end with a value) of
// bytes until we hit the end byte. This may be useful either for skipping entire groups
// outright or, more practically, skipping over garbage contained in the remainder of a group
void IE_Imp_WordPerfect_6::_skipGroup(FILE *fp, int groupByte)
{
   //UT_DEBUGMSG(("WP: Skipping the rest of a group: "));
   int readVal = fgetc(fp); // TODO: handle case that we get eof?
   while(readVal != groupByte) 
     {
	//UT_DEBUGMSG(("WP: %i\n",readVal));
	readVal = fgetc(fp);
     }

}

// insert the text in the current textbuf to the document, taking its
// style into account
void IE_Imp_WordPerfect_6::_flushText()
{
   if(m_textBuf.getLength() > 0)
     getDoc()->appendSpan(m_textBuf.getPointer(0), m_textBuf.getLength());
   m_textBuf.truncate(0);   
}

void IE_Imp_WordPerfect_6::_appendCurrentFormat()
{
   XML_Char* pProps = "props";
   XML_Char propBuffer[1024];	//TODO is this big enough?  better to make it a member and stop running all over the stack
   propBuffer[0] = 0;

   // bold
   strcat(propBuffer, "font-weight:");
   strcat(propBuffer, m_textAttributes.m_bold ? "bold" : "normal");
   // italic
   strcat(propBuffer, "; font-style:");
   strcat(propBuffer, m_textAttributes.m_italics ? "italic" : "normal");
   // underline & overline & strike-out
   strcat(propBuffer, "; text-decoration:");
   static UT_String decors;
   decors.clear();
   if (m_textAttributes.m_underLine)
     {
	decors += "underline ";
     }
   if (m_textAttributes.m_strikeOut)
     {
	decors += "line-through ";
     }
   if(!m_textAttributes.m_underLine  &&  
      !m_textAttributes.m_strikeOut)
     {
	decors = "none";
     }
   strcat(propBuffer, decors.c_str());

   const XML_Char* propsArray[3];
   propsArray[0] = pProps;
   propsArray[1] = propBuffer;
   propsArray[2] = NULL;
   getDoc()->appendFmt(propsArray);   
}

