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

/* See bug 1764
 * This product is not manufactured, approved, or supported by 
 * Corel Corporation or Corel Corporation Limited."
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ut_types.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_units.h"
#include "ut_growbuf.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_math.h" // for rint (font size)

#include "xap_Frame.h"
#include "xap_EncodingManager.h"

#include "pd_Document.h"
#include "pt_Types.h"

#include "ap_Strings.h"

#include "ie_imp_WordPerfect.h"

#define X_CheckFileError(v) if (v==EOF) { UT_DEBUGMSG(("X_CheckFileError: %d\n", __LINE__)); return UT_IE_IMPORTERROR; }
#define X_CheckFileReadElementError(v) if (v != 1) { UT_DEBUGMSG(("X_CheckFileReadElementError: %d\n", __LINE__)); return UT_IE_IMPORTERROR; } // makes sure that one element is read
#define X_CheckDocumentError(v) if (!v) { UT_DEBUGMSG(("X_CheckDocumentError: %d\n", __LINE__)); return UT_IE_IMPORTERROR; }
#define X_CheckWordPerfectError(v) if ((v != UT_OK)) { UT_DEBUGMSG(("X_CheckWordPerfectError: %d\n", __LINE__)); return UT_IE_IMPORTERROR; }

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
   
   m_fontSize = 12; // silly default. TODO: read from file.
}

static int wp_internationalCharacterMapping[32] = 
{ 229, // lower case 'a' with a small circle
     197, // upper case 'a' with a small circle
     230, // lower case 'ae'
     198, // upper case 'ae'
     228, // lower case 'a' with diathesis
     196, // upper case 'a' with diathesis
     224, // lower case 'a' with acute
     192, // lower case 'a' with grave
     226, // lower case 'a' with circonflex
     227, // lower case 'a' with tilde
     195, // upper case 'a' with tilde
     231, // lower case 'c' with hook
     199, // upper case 'c' with hook
     235, // lower case 'e' with diathesis
     232, // lower case 'e' with acute
     200, // upper case 'e' with acute
     233, // lower case 'e' with grave
     234, // lower case 'e' with circonflex
     236, // lower case 'i' with acute
     241, // lower case 'n' with tilde
     209, // upper case 'n' with tilde
     248, // lower case 'o' with stroke
     216, // upper case 'o' with stroke
     241, // lower case 'o' with tilde
     213, // upper case 'o' with tilde
     246, // lower case 'o' with diathesis
     214, // upper case 'o' with diathesis
     252, // lower case 'u' with diathesis
     220, // upper case 'u' with diathesis
     250, // lower case 'u' with acute
     249, // lower case 'u' with grave
     223 // double s
};

WordPerfectParagraphProperties::WordPerfectParagraphProperties()
{
   m_lineSpacing = 1;
   m_justificationMode = WordPerfectParagraphProperties::full;
}

WordPerfectByteTag::WordPerfectByteTag(unsigned char byte, UT_Error (IE_Imp_WordPerfect::*func)())
{
   m_byte = byte;
   m_func = func;
}

WordPerfectHeaderPacket::WordPerfectHeaderPacket(unsigned int PID, unsigned char type, long packetPosition, bool hasChildren)
{
   m_PID = PID;
   m_type = type;
   m_packetPosition = packetPosition;
   m_hasChildren = hasChildren;
}

/****************************************************************************/
/****************************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

ABI_PLUGIN_DECLARE("WordPerfect")
static IE_Imp_WordPerfect_Sniffer * m_sniffer = 0;

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Imp_WordPerfect_Sniffer ();
	}
	else
	{
		m_sniffer->ref();
	}

	UT_ASSERT (m_sniffer);

	mi->name    = "WordPerfect 6/7/8 (tm) Importer";
	mi->desc    = "WordPerfect 6/7/8 (tm) Documents";
	mi->version = ABI_VERSION_STRING;
	mi->author  = "Abi the Ant";
	mi->usage   = "No Usage";

	IE_Imp::registerImporter (m_sniffer);
	return 1;
}

ABI_FAR_CALL
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

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
								 UT_uint32 release)
{
  return 1;
}

#endif /* ENABLE_PLUGINS */

/****************************************************************************/
/****************************************************************************/

bool IE_Imp_WordPerfect_Sniffer::recognizeContents (const char * szBuf, 
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
	     int productType = (int) *(szBuf + WP_HEADER_PRODUCT_TYPE_OFFSET);
	     int fileType = (int) *(szBuf + WP_HEADER_FILE_TYPE_OFFSET);
	     int majorVersion = (int) *(szBuf + WP_HEADER_MAJOR_VERSION_OFFSET);
	     int minorVersion = (int) *(szBuf + WP_HEADER_MINOR_VERSION_OFFSET);
	     UT_DEBUGMSG(("product type: %i, file type: %i, major version: %i, minor version: %i\n", productType, fileType, majorVersion, minorVersion ));
	     // we only want to try parsing wordperfect 6/7/8 documents for now
	     if ((majorVersion != WP_WORDPERFECT678_EXPECTED_MAJOR_VERSION) || (fileType != WP_WORDPERFECT_DOCUMENT_FILE_TYPE))
	       return false;
	     
	     return true;
	  }
     }

   // ok, that didn't work, we'll try to dig through the OLE stream
   // (TODO)
   return false;
}

bool IE_Imp_WordPerfect_Sniffer::recognizeSuffix (const char * szSuffix)
{
	// We recognize both word documents and their template versions
	return (!UT_stricmp(szSuffix,".wpd"));
}

UT_Error IE_Imp_WordPerfect_Sniffer::constructImporter (PD_Document * pDocument,
													  IE_Imp ** ppie)
{
	IE_Imp_WordPerfect * p = new IE_Imp_WordPerfect(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_WordPerfect_Sniffer::getDlgLabels (const char ** pszDesc,
												const char ** pszSuffixList,
												IEFileType * ft)
{
	*pszDesc = "WordPerfect 6/7/8 (.wpd)";
	*pszSuffixList = "*.wpd";
	*ft = getFileType();
	return true;
}

/****************************************************************************/
/****************************************************************************/

// just buffer sizes, arbitrarily chosen
#define DOC_TEXTRUN_SIZE 2048
#define DOC_PROPBUFFER_SIZE 1024

IE_Imp_WordPerfect::IE_Imp_WordPerfect(PD_Document * pDocument)
  : IE_Imp (pDocument)
{
   m_undoOn = false;
   m_paragraphChanged = true;

   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_SOFT_EOL, &IE_Imp_WordPerfect::_insertSpace));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_SOFT_SPACE, &IE_Imp_WordPerfect::_insertSpace));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_HARD_HYPHEN, &IE_Imp_WordPerfect::_insertHyphen));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_EXTENDED_CHARACTER, &IE_Imp_WordPerfect::_handleExtendedCharacter));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_DORMANT_HARD_RETURN, &IE_Imp_WordPerfect::_handleHardEndOfLine));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_HARD_EOL, &IE_Imp_WordPerfect::_handleHardEndOfLine));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_EOL_GROUP, &IE_Imp_WordPerfect::_handleEndOfLineGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_PAGE_GROUP, &IE_Imp_WordPerfect::_handlePageGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_COLUMN_GROUP, &IE_Imp_WordPerfect::_handleColumnGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_PARAGRAPH_GROUP, &IE_Imp_WordPerfect::_handleParagraphGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_SET_NUMBER_GROUP, &IE_Imp_WordPerfect::_handleSetNumberGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_NUMBERING_METHOD_GROUP, &IE_Imp_WordPerfect::_handleNumberingMethodGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_DISPLAY_NUMBER_REFERENCE_GROUP, &IE_Imp_WordPerfect::_handleDisplayNumberReferenceGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_INCREMENT_NUMBER_GROUP, &IE_Imp_WordPerfect::_handleIncrementNumberGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_DECREMENT_NUMBER_GROUP, &IE_Imp_WordPerfect::_handleDecrementNumberGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_STYLE_GROUP, &IE_Imp_WordPerfect::_handleStyleGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_BOX_GROUP, &IE_Imp_WordPerfect::_handleBoxGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_PLATFORM_GROUP, &IE_Imp_WordPerfect::_handlePlatformGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_FORMATTER_GROUP, &IE_Imp_WordPerfect::_handleFormatterGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_TAB_GROUP, &IE_Imp_WordPerfect::_handleTabGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_CHARACTER_GROUP, &IE_Imp_WordPerfect::_handleCharacterGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_CROSSREFERENCE_GROUP, &IE_Imp_WordPerfect::_handleCrossReferenceGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_HEADER_FOOTER_GROUP, &IE_Imp_WordPerfect::_handleHeaderFooterGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_FOOTENDNOTE_GROUP, &IE_Imp_WordPerfect::_handleFootEndNoteGroup));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_UNDO_GROUP, &IE_Imp_WordPerfect::_handleUndo));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_ATTRIBUTE_ON, &IE_Imp_WordPerfect::_handleAttributeOn));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_ATTRIBUTE_OFF, &IE_Imp_WordPerfect::_handleAttributeOff));
   m_wordPerfectDispatchBytes.addItem(new WordPerfectByteTag(WP_TOP_UNDO_GROUP, &IE_Imp_WordPerfect::_handleUndo));
}

IE_Imp_WordPerfect::~IE_Imp_WordPerfect() 
{
   UT_VECTOR_PURGEALL(WordPerfectFontDescriptor *, m_fontDescriptorList);
   UT_VECTOR_PURGEALL(WordPerfectHeaderPacket *, m_headerPacketList);
}

/****************************************************************************/
/****************************************************************************/

UT_Error IE_Imp_WordPerfect::importFile(const char * szFilename)
{
   m_importFile = fopen(szFilename, "rb");
   if (!m_importFile)
     {
	UT_DEBUGMSG(("Could not open file %s\n",szFilename));
	return UT_IE_FILENOTFOUND;
     }

   UT_Error error;  

   error = _parseHeader(); 
   if (error == UT_OK) 
     {
	error = _parseIndexHeader();
	if (error == UT_OK)
	  error = _parseDocument();   
     }
   
   fclose(m_importFile);
   return error;
}

void IE_Imp_WordPerfect::pasteFromBuffer (PD_DocumentRange *, 
										unsigned char *, unsigned int, const char *)
{
	// nada
}

UT_Error IE_Imp_WordPerfect::_parseHeader()
{
   UT_DEBUGMSG(("WordPerfect: Parsing the Header \n"));
   UT_uint16 documentEncrypted;
   
   if (fseek(m_importFile, WP_HEADER_DOCUMENT_POINTER_POSITION, SEEK_SET) != 0)
     return UT_IE_IMPORTERROR;
   if (fread(&m_documentPointer, sizeof(UT_uint32), 1, m_importFile) != 1)
     return UT_IE_IMPORTERROR;

   if (fseek(m_importFile, WP_HEADER_DOCUMENT_SIZE_POSITION, SEEK_SET) != 0)
     return UT_IE_IMPORTERROR;
   if (fread(&m_documentEnd, sizeof(UT_uint32), 1, m_importFile) != 1)
     return UT_IE_IMPORTERROR;
   
   if (fseek(m_importFile, WP_HEADER_INDEX_HEADER_POSITION, SEEK_SET) != 0)
     return UT_IE_IMPORTERROR;
   if (fread(&m_indexPointer, sizeof(UT_uint16), 1, m_importFile) != 1)
     return UT_IE_IMPORTERROR;
   
   if (fseek(m_importFile, WP_HEADER_ENCRYPTION_POSITION, SEEK_SET) != 0)
     return UT_IE_IMPORTERROR;
   if (fread(&documentEncrypted, sizeof(UT_uint16), 1, m_importFile) != 1)
     return UT_IE_IMPORTERROR;
   
   UT_DEBUGMSG(("WordPerfect: Index Header Position = %i \n",(int)m_indexPointer));
   UT_DEBUGMSG(("WordPerfect: Document Pointer = %i \n",(int)m_documentPointer));
   UT_DEBUGMSG(("WordPerfect: Document End Position = %i \n",(int)m_documentEnd));
   
   if (documentEncrypted != 0)
     return UT_IE_PROTECTED;
   
   // sanity check
   if (m_documentPointer > m_documentEnd)
     return UT_IE_IMPORTERROR;
   
   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_parseIndexHeader()
{
   UT_DEBUGMSG(("WordPerfect: Parsing the Index Header \n"));
   UT_uint16 numIndices;
   
   if (fseek(m_importFile, (long) (m_indexPointer+WP_INDEX_HEADER_NUM_INDICES_POSITION), SEEK_SET) != 0)
     return UT_IE_IMPORTERROR;
   if (fread(&numIndices, sizeof(UT_uint16), 1, m_importFile) != 1)
     return UT_IE_IMPORTERROR;
   UT_DEBUGMSG(("WordPerfect: Index header has %i packet indices \n", (int) numIndices));
   
   if (fseek(m_importFile, (long) (m_indexPointer+WP_INDEX_HEADER_INDICES_POSITION), SEEK_SET) != 0)
     return UT_IE_IMPORTERROR;
   
   for (unsigned int i=1; i<numIndices; i++)
     {
	unsigned char flags, packetType;
	UT_uint16 packetUseCount, hiddenCount;
	UT_uint32 dataPacketSize, dataPointer;
	bool hasChildren = false;
	X_CheckFileReadElementError(fread(&flags, sizeof(unsigned char), 1, m_importFile));
	if (flags & WP_INDEX_HEADER_ELEMENT_CHILD_PACKET_BIT)
	  hasChildren = true;
	X_CheckFileReadElementError(fread(&packetType, sizeof(unsigned char), 1, m_importFile));
	X_CheckFileReadElementError(fread(&packetUseCount, sizeof(UT_uint16), 1, m_importFile));
	X_CheckFileReadElementError(fread(&hiddenCount, sizeof(UT_uint16), 1, m_importFile));
	X_CheckFileReadElementError(fread(&dataPacketSize, sizeof(UT_uint32), 1, m_importFile));
	X_CheckFileReadElementError(fread(&dataPointer, sizeof(UT_uint32), 1, m_importFile));
	UT_DEBUGMSG(("WordPerfect: (Packet Element %i: flags: %i, type: %i, use count: %i, hidden count: %i, size: %i, pointer: %i hasChildren: %i) \n", (int)i, (int)flags, (int)packetType, (int)packetUseCount, (int)hiddenCount, (int)dataPacketSize, (int)dataPointer, (int) hasChildren));

	switch (packetType)
	  {
	   case WP_INDEX_HEADER_FONT_TYPEFACE_DESCRIPTOR_POOL:
	   case WP_INDEX_HEADER_DESIRED_FONT_DESCRIPTOR_POOL:
	     X_CheckWordPerfectError(_parseFontDescriptorPacket(i, dataPacketSize, dataPointer));
	     break;
	   default:
	     break;
	  }
	
	// insert into a generic list of header packets, for later use
	// (we include the font descriptors in this list because it makes it
	// trivial to look up the right PID..)
	WordPerfectHeaderPacket *headerPacket = new WordPerfectHeaderPacket(i, packetType, dataPointer, hasChildren);
	m_headerPacketList.addItem(headerPacket); // todo: check for error? how?
     }
   
   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_parseFontDescriptorPacket(int packetID, UT_uint32 dataPacketSize, UT_uint32 dataPointer)
{
   UT_DEBUGMSG(("WordPerfect: Parsing a Font Descriptor\n"));
   UT_uint32 lastPosition = ftell(m_importFile);

   WordPerfectFontDescriptor *fontDescriptor = new WordPerfectFontDescriptor;
   UT_uint16 fontNameLength;
   
   if (fseek(m_importFile, (long) (dataPointer), SEEK_SET) != 0)
     return UT_IE_IMPORTERROR;

   // the packet number that this font is representative of
   // (this is what we use in the document to look up the correct font for a face/size change)
   fontDescriptor->m_packetID = packetID;

   // short sized characteristics
   X_CheckFileReadElementError(fread(&fontDescriptor->m_characterWidth, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptor->m_ascenderHeight, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptor->m_xHeight, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptor->m_descenderHeight, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptor->m_italicsAdjust, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptor->m_primaryFamilyId, sizeof(UT_Byte), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptor->m_primaryFamilyMemberId, sizeof(UT_Byte), 1, m_importFile));
   // byte sized characteristics
   X_CheckFileReadElementError(fread(&fontDescriptor->m_scriptingSystem, sizeof(UT_Byte), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptor->m_primaryCharacterSet, sizeof(UT_Byte), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptor->m_width, sizeof(UT_Byte), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptor->m_weight, sizeof(UT_Byte), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptor->m_attributes, sizeof(UT_Byte), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptor->m_generalCharacteristics, sizeof(UT_Byte), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptor->m_classification, sizeof(UT_Byte), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptor->m_fill, sizeof(UT_Byte), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptor->m_fontType, sizeof(UT_Byte), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptor->m_fontSourceFileType, sizeof(UT_Byte), 1, m_importFile));

   X_CheckFileReadElementError(fread(&fontNameLength, sizeof(UT_uint16), 1, m_importFile));
   if(fontNameLength > 0)
     {	
	fontDescriptor->m_fontName = new char[fontNameLength];
	if(fread(fontDescriptor->m_fontName, sizeof(char), fontNameLength, m_importFile) != fontNameLength)
	  return UT_IE_IMPORTERROR;
     }
   
   UT_DEBUGMSG(("WordPerfect: Read Font (primary family id: %i, font type: %i, font source file type: %i font name length: %i, font name: %s)\n", (int) fontDescriptor->m_primaryFamilyId, (int) fontDescriptor->m_fontType, (int) fontDescriptor->m_fontSourceFileType, (int) fontNameLength, fontDescriptor->m_fontName));
   
   m_fontDescriptorList.addItem(fontDescriptor); // todo: check for error? how?
   
   if (fseek(m_importFile, (long) (lastPosition), SEEK_SET) != 0)
     return UT_IE_IMPORTERROR;
   
   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_handleBoxGroupContent(int boxContentPID)
{
   // find the appropriate packet
   WordPerfectHeaderPacket *boxContentPacket = (WordPerfectHeaderPacket *)m_headerPacketList.getNthItem((boxContentPID - 1)); // wp packets start at 1, we start at 0
   
   // FIXME: we should really handle externally declared files. but we don't.
   if( !boxContentPacket->m_hasChildren )
	  return UT_IE_IMPORTERROR;
   
   long documentParsePosition = ftell(m_importFile); // keep track of where we were
	   
   if (fseek(m_importFile, boxContentPacket->m_packetPosition, SEEK_SET) != 0)
     return UT_IE_IMPORTERROR;
   
   UT_uint16 numChildPackets;
   X_CheckFileReadElementError(fread(&numChildPackets, sizeof(UT_uint16), 1, m_importFile));
   UT_uint16 *graphicContentPIDs = new UT_uint16[numChildPackets];
   for( unsigned int i=0; i<numChildPackets; i++ )
     X_CheckFileReadElementError(fread(&graphicContentPIDs[i], sizeof(UT_uint16), 1, m_importFile));
   
   UT_DEBUGMSG(("WordPerfect: Parsed box group content with %i childpackets\n", numChildPackets));
   
   // FIXME: for now, we only handle one packet of graphic content
   if( numChildPackets > 0 )
     X_CheckWordPerfectError(_handleGraphicsData(graphicContentPIDs[0]))
   
   // return to where we were before
   if (fseek(m_importFile, documentParsePosition, SEEK_SET) != 0)
     return UT_IE_IMPORTERROR;
   
   delete(graphicContentPIDs);
   
   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_handleGraphicsData(int graphicPID)
{
   // find the appropriate packet - wp packets start at 1, we start at 0
   //WordPerfectHeaderPacket *graphicPacket = (WordPerfectHeaderPacket *)m_headerPacketList.getNthItem((graphicPID - 1)); 
   // (TODO: Finish Me)
															    
   return UT_OK;
}


UT_Error IE_Imp_WordPerfect::_handleBoxGroupTemplate(int boxGroupTemplatePID)
{
   UT_DEBUGMSG(("WordPerfect: Handling a box group template (currently disabled)\n"));
   /*
   // find the appropriate packet
   WordPerfectHeaderPacket *boxTemplatePacket = (WordPerfectHeaderPacket *)m_headerPacketList.getNthItem((boxGroupTemplatePID - 1)); // wp packets start at 1, we start at 0
   
   // we're only going to bother with graphics for now
   if (boxTemplatePacket->m_type == WP_INDEX_HEADER_GRAPHICS_BOX_STYLE)
     {
	long documentParsePosition = ftell(m_importFile); // keep track of where we were
	   
	if (fseek(m_importFile, boxTemplatePacket->m_packetPosition, SEEK_SET) != 0)
	  return UT_IE_IMPORTERROR;
	
	// now we read out a list of (possible) child packets to parse
	UT_uint16 numPrefixPacketIDs;
	UT_uint16 dataPrefixIDs;
	UT_uint16 counterPrefixIDs;
	UT_uint16 positioningDataPrefixIDs;
	UT_uint16 contentPrefixIDs;
	UT_uint16 captionPrefixIDs;
	UT_uint16 borderPrefixIDs;
	UT_uint16 fillPrefixIDs;
	UT_uint16 wrapPrefixIDs;
	UT_uint16 hyperTextPrefixIDs;
	
  	X_CheckFileReadElementError(fread(&numPrefixPacketIDs, sizeof(UT_uint16), 1, m_importFile));   
	X_CheckFileReadElementError(fread(&dataPrefixIDs, sizeof(UT_uint16), 1, m_importFile));   
	X_CheckFileReadElementError(fread(&counterPrefixIDs, sizeof(UT_uint16), 1, m_importFile));
	X_CheckFileReadElementError(fread(&positioningDataPrefixIDs, sizeof(UT_uint16), 1, m_importFile));
	X_CheckFileReadElementError(fread(&contentPrefixIDs, sizeof(UT_uint16), 1, m_importFile));
	X_CheckFileReadElementError(fread(&captionPrefixIDs, sizeof(UT_uint16), 1, m_importFile));
	X_CheckFileReadElementError(fread(&borderPrefixIDs, sizeof(UT_uint16), 1, m_importFile));
	X_CheckFileReadElementError(fread(&fillPrefixIDs, sizeof(UT_uint16), 1, m_importFile));
	X_CheckFileReadElementError(fread(&wrapPrefixIDs, sizeof(UT_uint16), 1, m_importFile));
	X_CheckFileReadElementError(fread(&hyperTextPrefixIDs, sizeof(UT_uint16), 1, m_importFile));
	UT_DEBUGMSG(("WordPerfect: Read Box Template Packet Header (numPrefixPacketIDs: %i, dataPrefixIDs: %i, counterPrefixIDs: %i, positioningDataPrefixIDs: %i, contentPrefixIDs: %i, captionPrefixIDs: %i, borderPrefixIDs: %i, fillPrefixIDs: %i, wrapPrefixIDs: %i, hypertextPrefixIDs: %i\n",
		     (int)numPrefixPacketIDs, (int)dataPrefixIDs, (int)counterPrefixIDs, (int)positioningDataPrefixIDs, (int)contentPrefixIDs, (int)captionPrefixIDs, (int)borderPrefixIDs, (int)fillPrefixIDs, (int)wrapPrefixIDs, (int) hyperTextPrefixIDs));
	
	// return to where we were before
	if (fseek(m_importFile, documentParsePosition, SEEK_SET) != 0)
	  return UT_IE_IMPORTERROR;
     }
   */
   return UT_OK;
  
}

UT_Error IE_Imp_WordPerfect::_parseDocument()
{
   UT_DEBUGMSG(("WordPerfect: Parsing the Document \n"));

   X_CheckDocumentError(getDoc()->appendStrux(PTX_Section, NULL));
   
   if (fseek(m_importFile, m_documentPointer, SEEK_SET) != 0)
     return UT_IE_IMPORTERROR;
   
   while (ftell(m_importFile) < (int)m_documentEnd)
     {
	int readVal = fgetc(m_importFile);
	X_CheckFileError(readVal);
	
	if(readVal > 32 && readVal < 127 && !m_undoOn) // Standard ASCII characters
	  {
	     //UT_DEBUGMSG((" current char = %c \n",(char)readVal));
	     wchar_t wc = 0;
	     m_Mbtowc.mbtowc(wc, (char)readVal);
	     m_textBuf.append( (UT_uint16 *)&wc, 1);
	  }
	else if(readVal > 0 && readVal < 32 && !m_undoOn) // International Characters
	  {
	     UT_UCSChar internationalCharacter = wp_internationalCharacterMapping[(readVal-1)];
	     m_textBuf.append( &internationalCharacter, 1);	     
	  }
	else 
	  {
	     
	     for (unsigned int i=0; i<m_wordPerfectDispatchBytes.size(); i++)
	       {
		  WordPerfectByteTag *dispatchByte = (WordPerfectByteTag *) m_wordPerfectDispatchBytes.getNthItem( i );
		  if( dispatchByte->m_byte == (unsigned char) readVal )
		    {
		       (*this.*(dispatchByte->m_func)) ();
		       break;
		    }	  
	       }
	  }
     }
  
   UT_DEBUGMSG(("WordPerfect: File Pointer at %i equals or exceeds document length of %i\n", (int)ftell(m_importFile), (int)m_documentEnd));
   
   X_CheckWordPerfectError(_flushText());
   
   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_insertSpace()
{
   if(!m_undoOn)
     {
	wchar_t wc = 0;
	m_Mbtowc.mbtowc(wc, ' ');
	m_textBuf.append( (UT_uint16 *)&wc, 1);
     }
   
   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_insertHyphen()
{
   if(!m_undoOn)
     {		  
	wchar_t wc = 0;
	m_Mbtowc.mbtowc(wc, '-');
	m_textBuf.append( (UT_uint16 *)&wc, 1);
     }
   
   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_handleHardEndOfLine()
{
   // (TODO: eliminate a prev space if it's just before this)
   UT_DEBUGMSG(("WordPerfect: Handling a hard EOL \n"));
   if(!m_undoOn)
     {
	X_CheckWordPerfectError(_flushText());
	_appendCurrentParagraphProperties();		
     }
   
   return UT_OK;
}

// handles an end-of-line function that has formatting information embedded
// into it. Fortunately, only the first code is relevant for our purposes..
// the rest can be safely skipped (at least according to the developer documentation)
UT_Error IE_Imp_WordPerfect::_handleEndOfLineGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a EOL group\n"));

   unsigned char subGroup;   
   UT_uint16 size;
   long startPosition = ftell(m_importFile);

   X_CheckFileReadElementError(fread(&subGroup, sizeof(unsigned char), 1, m_importFile));
   X_CheckFileReadElementError(fread(&size, sizeof(UT_uint16), 1, m_importFile)); // I have no idea WHAT this var. does. but it's there.

   wchar_t wc = 0;

   if(!m_undoOn)
     {
	switch(subGroup)
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
	     m_textBuf.append( (UT_uint16 *)&wc, 1);
	     break;
	   case 4: // 0x04 (hard end-of-line)
	   case 5: // 0x05 (hard EOL at EOC) 
	   case 6: // 0x06 (hard EOL at EOP)
	   case 23: // 0x17 (deletable hard EOL)
	   case 24: // 0x18 (deletable hard EOL at EOC)
	   case 25: // 0x19 (deletable hard EOL at EOP)	
	     X_CheckWordPerfectError(_handleHardEndOfLine());
	     break;
	   case 9: // hard EOP
	   case 28: // deletable hard EOP
	     { 
	       X_CheckWordPerfectError(_flushText());
	       _appendCurrentParagraphProperties();
	       UT_UCSChar ucs = UCS_FF;
	       X_CheckDocumentError(getDoc()->appendSpan(&ucs,1));
	     }
	     break;
	   default: // something else we don't support yet
	     break;
	  }
     }
   
   X_CheckWordPerfectError(_skipGroup(startPosition, size));

   return UT_OK;
}

// handles a page group
// (TODO: not implemented, just skips over it)
UT_Error IE_Imp_WordPerfect::_handlePageGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a page group\n"));
   long startPosition;
   unsigned char subGroup;   
   UT_uint16 size;
   unsigned char flags;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));   
   X_CheckWordPerfectError(_skipGroup(startPosition, size));

   return UT_OK;
}

// handles a column group
// (TODO: not implemented, just skips over it)
UT_Error IE_Imp_WordPerfect::_handleColumnGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a column group\n"));
   long startPosition;
   unsigned char subGroup;   
   UT_uint16 size;
   unsigned char flags;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));   
   X_CheckWordPerfectError(_skipGroup(startPosition, size));
   
   return UT_OK;
}

// handles a paragraph group
// (TODO: not completely implemented)
UT_Error IE_Imp_WordPerfect::_handleParagraphGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a paragraph group\n"));

   long startPosition;
   unsigned char subGroup, flags;
   UT_uint16 size;
   UT_uint16 nonDeletableInfoSize;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));   
   X_CheckFileReadElementError(fread(&nonDeletableInfoSize, sizeof(UT_uint16), 1, m_importFile));

   // dispatch to subgroup to handle the rest of the relevant properties within the
   // group (and thus, read more of the file-- so we keep this even if undo is 'on')
   switch (subGroup)
     {
      case WP_PARAGRAPH_GROUP_JUSTIFICATION:
	X_CheckWordPerfectError(_handleParagraphGroupJustification());
	break;
     }
   m_paragraphChanged = true;
	 
   X_CheckWordPerfectError(_skipGroup(startPosition, size));
   
   return UT_OK;
}


// handles a style group
// (TODO: not implemented, just skips over it)
UT_Error IE_Imp_WordPerfect::_handleStyleGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a style group\n"));
   long startPosition;
   unsigned char subGroup;   
   UT_uint16 size;
   unsigned char flags;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));   
   X_CheckWordPerfectError(_skipGroup(startPosition, size));
   
   return UT_OK;
}

// handles a tab group
// (TODO: partially implemented, only basic tab import is done)
UT_Error IE_Imp_WordPerfect::_handleTabGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a tab group\n"));
 
   long startPosition = ftell(m_importFile);
   unsigned char tabDefinition;   
   UT_uint16 size;
   
   X_CheckFileReadElementError(fread(&tabDefinition, sizeof(unsigned char), 1, m_importFile));
   X_CheckFileReadElementError(fread(&size, sizeof(UT_uint16), 1, m_importFile)); // I have no idea WHAT this var. does. but it's there.

   if(!m_undoOn)
     {
        // TODO: soft type (tab, align, centerm and so forth), (uses tab set definition type)
        if (tabDefinition & 0x01) { }
        // TODO: dot leader 
        if (tabDefinition & 0x02) { }
        // TODO: generic search (only used in search code, ignore bits 0 and 1 when comparing)
        if (tabDefinition & 0x04) { }    
	
        switch ((tabDefinition & 0xF8) >> 3)
	  {
	   case  0: // 00000b = back tab
	   case  1: // 00001b = table tab
	   case  2: // 00010b = left tab
	   case  4: // 00100b = bar tab
	   case  6: // 00110b = left indent
	   case  7: // 00111b = left/right indent
	   case  8: // 01000b = center on margins
	   case  9: // 01001b = center on current position
	   case 10: // 01010b = center tab
	   case 16: // 10000b = flush right
	   case 18: // 10010b = right tab
	   case 26: // 11010b = decimal tab
	     // TODO: fix stupid default implementation of adding just a TAB char without looking what it actually should be
	     {
		wchar_t wc = 0;
	        m_Mbtowc.mbtowc(wc, '\t');
	        m_textBuf.append( (UT_uint16 *)&wc, 1);
	     }
	     break;
	   default: // something else shouldn't be possible according to the documentation
	     break;
	  }
     }

   X_CheckWordPerfectError(_skipGroup(startPosition, size));

   return UT_OK;
}

// handles a platform group
// (TODO: not implemented, just skips over it)
UT_Error IE_Imp_WordPerfect::_handlePlatformGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a platform group\n"));
   unsigned char subGroup, flags;
   long startPosition;
   UT_uint16 size;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));
   X_CheckWordPerfectError(_skipGroup(startPosition, size));
   
   return UT_OK;
}

// handles a formatter group
// (undocumented in corel's documentation, we have to skip)
UT_Error IE_Imp_WordPerfect::_handleFormatterGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a formatter group\n"));
   unsigned char subGroup, flags;
   long startPosition;
   UT_uint16 size;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));
   X_CheckWordPerfectError(_skipGroup(startPosition, size));

   return UT_OK;
}

// handles a foot or end note group
// (TODO: not implemented, just skips over it)
UT_Error IE_Imp_WordPerfect::_handleFootEndNoteGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a foot/endnote group\n"));
   unsigned char subGroup, flags;
   long startPosition;
   UT_uint16 size;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));
   X_CheckWordPerfectError(_skipGroup(startPosition, size));
   
   return UT_OK;
}

// handles a set number group
// (TODO: not implemented, just skips over it)
UT_Error IE_Imp_WordPerfect::_handleSetNumberGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a set number group\n"));
   unsigned char subGroup, flags;
   long startPosition;
   UT_uint16 size;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));
   X_CheckWordPerfectError(_skipGroup(startPosition, size));
   
   return UT_OK;
}

// handles a numbering method group
// (TODO: not implemented, just skips over it)
UT_Error IE_Imp_WordPerfect::_handleNumberingMethodGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a numbering method group\n"));
   unsigned char subGroup, flags;
   long startPosition;
   UT_uint16 size;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));
   X_CheckWordPerfectError(_skipGroup(startPosition, size));
   
   return UT_OK;
}


// handles a display number reference group
// (TODO: not implemented, just skips over it)
UT_Error IE_Imp_WordPerfect::_handleDisplayNumberReferenceGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a display number rerference group\n"));
   unsigned char subGroup, flags;
   long startPosition;
   UT_uint16 size;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));
   X_CheckWordPerfectError(_skipGroup(startPosition, size));
   
   return UT_OK;
}

// handles a increment number group
// (TODO: not implemented, just skips over it)
UT_Error IE_Imp_WordPerfect::_handleIncrementNumberGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling an increment number group\n"));
   unsigned char subGroup, flags;
   long startPosition;
   UT_uint16 size;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));
   X_CheckWordPerfectError(_skipGroup(startPosition, size));
   
   return UT_OK;
}


// handles a decrement number reference group
// (TODO: not implemented, just skips over it)
UT_Error IE_Imp_WordPerfect::_handleDecrementNumberGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a decrement number group\n"));
   unsigned char subGroup, flags;
   long startPosition;
   UT_uint16 size;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));
   X_CheckWordPerfectError(_skipGroup(startPosition, size));
   
   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_handleMergeGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a merge group\n"));
   unsigned char subGroup, flags;
   long startPosition;
   UT_uint16 size;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));
   X_CheckWordPerfectError(_skipGroup(startPosition, size));
   
   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_handleBoxGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a box group\n"));
   unsigned char subGroup, prefixIndex;
   long startPosition;
   UT_uint16 size;

   unsigned char numPIDs;
   UT_uint16 numNonDeletableBytes;
   // (14 reserved bytes)
   UT_uint16 sizeOverrideAndWrapRectangleData;
   UT_uint16 sizeOverrideData;
   UT_uint16 boxOverrideFlags;
   UT_uint16 numWrapRectangles;
   UT_uint16 leftOffset;
   UT_uint16 rightOffset;
   UT_uint16 topOffset;
   UT_uint16 bottomOffset;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, prefixIndex));
   
   X_CheckFileReadElementError(fread(&numPIDs, sizeof(unsigned char), 1, m_importFile));
   if( numPIDs < 1 )
     return UT_IE_IMPORTERROR;
   UT_uint16 *boxPIDs = new UT_uint16[numPIDs];
   for( unsigned int i=0; i<numPIDs; i++ )
        X_CheckFileReadElementError(fread(&boxPIDs[i], sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&numNonDeletableBytes, sizeof(UT_uint16), 1, m_importFile));
   if (fseek(m_importFile, WP_BOX_GROUP_NUM_RESERVED_BYTES, SEEK_CUR) != 0)
     return UT_IE_IMPORTERROR;
   X_CheckFileReadElementError(fread(&sizeOverrideAndWrapRectangleData, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&sizeOverrideData, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&boxOverrideFlags, sizeof(UT_uint16), 1, m_importFile));   
   
   X_CheckFileReadElementError(fread(&numWrapRectangles, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&leftOffset, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&rightOffset, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&topOffset, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&bottomOffset, sizeof(UT_uint16), 1, m_importFile));
   
   UT_DEBUGMSG(("WordPerfect: Box Group: subGroup: %i, prefixIndex: %i, numPIDs: %i\n", (int) subGroup, (int)prefixIndex, (int) numPIDs));
   UT_DEBUGMSG(("WordPerfect: Box Group: sizeOverrideAndWrapRectangleData: %i, sizeOverrideData: %i, boxOverrideFlags: %i, numWrapRectangles: %i\n", (int) sizeOverrideAndWrapRectangleData, (int) sizeOverrideData, (int)boxOverrideFlags, (int) numWrapRectangles));
   UT_DEBUGMSG(("WordPerfect: Box Group: leftOffset: %i, rightOffset: %i, topOffset: %i, bottomOffset: %i\n", (int) leftOffset, (int) rightOffset, (int) topOffset, (int) bottomOffset));
   
   int j=1;
   if( boxOverrideFlags & WP_BOX_GROUP_OVERRIDE_FLAGS_BOX_CONTENT )
     {
	UT_DEBUGMSG(("WordPerfect: Override flags set for content. Content PID: %i\n", boxPIDs[j]));
	j++;
     }
   
   X_CheckWordPerfectError(_handleBoxGroupTemplate(boxPIDs[0]));
   X_CheckWordPerfectError(_skipGroup(startPosition, size));
   
   return UT_OK;
}

// handles an attribute "on" group in wordperfect. turns a formatting/style
// property on.
UT_Error IE_Imp_WordPerfect::_handleAttributeOn()
{   
   UT_DEBUGMSG(("WordPerfect: Handling an attribute ON\n"));
   long startPosition = ftell(m_importFile);
   
   int readVal = fgetc(m_importFile); // TODO: handle case that we get eof?
   X_CheckFileError(readVal);
   
   if(!m_undoOn)
     {
	// flush what's come before this change (even if it's nothing, which
	// IS a case we have to be worried about in case we are writing the first
	// paragraph)
	X_CheckWordPerfectError(_flushText());

	switch (readVal)
	  { 
	   case 14: // underline
	     m_textAttributes.m_underLine = true;
	     break;
	   case 12: // bold
	     m_textAttributes.m_bold = true;
	     break;
	   default: // something we don't support yet
	     break;
	  }
        
 	X_CheckWordPerfectError(_appendCurrentTextProperties());
     }
      
   X_CheckWordPerfectError(_skipGroup(startPosition, WP_ATTRIBUTE_ON_GROUP_SIZE));
   return UT_OK;
}

// handles an attribute "off" byte in wordperfect. turns a formatting/style
// property off.
UT_Error IE_Imp_WordPerfect::_handleAttributeOff()
{   
   UT_DEBUGMSG(("WordPerfect: Handling an attribute OFF\n"));
   long startPosition = ftell(m_importFile);

   int readVal = fgetc(m_importFile); // TODO: handle case that we get eof?
   X_CheckFileError(readVal);
   
   if(!m_undoOn)
     {
	// flush what's come before this change (even if it's nothing, which
	// IS a case we have to be worried about in case we are writing the first
	// paragraph)
	X_CheckWordPerfectError(_flushText());

	switch (readVal)
	  { 
	   case 14: // underline
	     m_textAttributes.m_underLine = false;
	     break;
	   case 12: // bold
	     m_textAttributes.m_bold = false;
	     break;
	   default: // something we don't support yet
	     break;
	  }
        
 	X_CheckWordPerfectError(_appendCurrentTextProperties());
     }
      
   X_CheckWordPerfectError(_skipGroup(startPosition, WP_ATTRIBUTE_OFF_GROUP_SIZE));
   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_handleCharacterGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a character group\n"));

   long startPosition;
   unsigned char subGroup;
   UT_uint16 size;
   unsigned char flags;
   
   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));
     
   switch(subGroup)
     {
      case WP_CHARACTER_GROUP_FONT_FACE_CHANGE:
	X_CheckWordPerfectError(_handleFontFaceChange());
	break;	
      case WP_CHARACTER_GROUP_FONT_SIZE_CHANGE:
	X_CheckWordPerfectError(_handleFontSizeChange());
	break;
      default:
	break;
     }
			       
   X_CheckWordPerfectError(_skipGroup(startPosition, size));
   return UT_OK;
}

// handles a cross reference group
// (TODO: not implemented, just skips over it)
UT_Error IE_Imp_WordPerfect::_handleCrossReferenceGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a crossreference group\n"));
   unsigned char subGroup, flags;
   long startPosition;
   UT_uint16 size;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));
   X_CheckWordPerfectError(_skipGroup(startPosition, size));

   return UT_OK;
}

// handles a header/footer group
// (TODO: not implemented, just skips over it)
UT_Error IE_Imp_WordPerfect::_handleHeaderFooterGroup()
{
   UT_DEBUGMSG(("WordPerfect: Handling a header/footer group\n"));
   unsigned char subGroup, flags;
   long startPosition;
   UT_uint16 size;

   X_CheckWordPerfectError(_handleVariableGroupHeader(startPosition, subGroup, size, flags));
   X_CheckWordPerfectError(_skipGroup(startPosition, size));

   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_handleExtendedCharacter()
{
   UT_DEBUGMSG(("WordPerfect: Handling an extended character\n"));

   wchar_t wc = 0;
   int character = fgetc(m_importFile);
   X_CheckFileError(character);
   int characterSet = fgetc(m_importFile);
   X_CheckFileError(characterSet);
   
   if(!m_undoOn)
     {
	// TODO: hack, hack, hack
	// find out how to reliably map ALL characters between character sets and extended characters
	if(character == 28 && characterSet == 4)
	  wc = 39; // character: '
	else if((character == 31 || character == 32) && characterSet == 4)
	  {
	     wc = 34; // character: "
	  }	
	else
	  wc = 0; // whitespace
   
	m_textBuf.append( (UT_uint16 *)&wc, 1);
     }
   
   int readVal = fgetc(m_importFile); // TODO: check for eof and also that the end byte is the extended character flag
   if(readVal != WP_TOP_EXTENDED_CHARACTER)
     return UT_IE_IMPORTERROR;
       
   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_handleFontFaceChange()
{
   UT_DEBUGMSG(("WordPerfect: Handling a Font Face Change\n"));
   
   unsigned char numPIDs;
   UT_uint16 fontDescriptorPID; 
   UT_uint16 nonDeletableInfoSize;
   UT_uint16 oldMatchedPointSize;
   UT_uint16 hash;
   UT_uint16 matchedFontIndex;
   UT_uint16 matchedFontPointSize;
   
   X_CheckFileReadElementError(fread(&numPIDs, sizeof(unsigned char), 1, m_importFile));
   X_CheckFileReadElementError(fread(&fontDescriptorPID, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&nonDeletableInfoSize, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&oldMatchedPointSize, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&hash, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&matchedFontIndex, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&matchedFontPointSize, sizeof(UT_uint16), 1, m_importFile));
   
   UT_DEBUGMSG(("WordPerfect: Got this font face change info: (num PIDS: %i, font descriptor PID: %i, old matched point size: %i, hash: %i, matched font index: %i, matched font point size: %i)\n",
		(int) numPIDs, (int) fontDescriptorPID, (int) oldMatchedPointSize, (int) hash, (int) matchedFontIndex, (int) matchedFontPointSize));

   m_textAttributes.m_fontSize = (UT_uint16) rint((double)((((float)matchedFontPointSize)/100.0f)*2.0f)); // fixme: ghastly magic numbers;
   X_CheckWordPerfectError(_flushText());
   X_CheckWordPerfectError(_appendCurrentTextProperties());

   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_handleFontSizeChange()
{
   UT_DEBUGMSG(("WordPerfect: Handling a Font Size Change\n"));
   
   unsigned char numPIDs;
   UT_uint16 oldDesiredDescriptorPID; 
   UT_uint16 nonDeletableInfoSize;
   UT_uint16 desiredPointSize;
   UT_uint16 hash;
   UT_uint16 matchedFontIndex;
   UT_uint16 matchedFontPointSize;
   
   X_CheckFileReadElementError(fread(&numPIDs, sizeof(unsigned char), 1, m_importFile));
   X_CheckFileReadElementError(fread(&oldDesiredDescriptorPID, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&nonDeletableInfoSize, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&desiredPointSize, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&hash, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&matchedFontIndex, sizeof(UT_uint16), 1, m_importFile));
   X_CheckFileReadElementError(fread(&matchedFontPointSize, sizeof(UT_uint16), 1, m_importFile));
   
   UT_DEBUGMSG(("WordPerfect: Got this font size change info: (num PIDS: %i, old typeface PID: %i, desired point size: %i, hash: %i, matched font index: %i, matched font point size: %i)\n",
		(int) numPIDs, (int) oldDesiredDescriptorPID, (int) desiredPointSize, (int) hash, (int) matchedFontIndex, (int) matchedFontPointSize));
   
   m_textAttributes.m_fontSize = (UT_uint16) rint((double)((((float)desiredPointSize)/100.0f)*2.0f)); // fixme: ghastly magic numbers;
   X_CheckWordPerfectError(_flushText());
   X_CheckWordPerfectError(_appendCurrentTextProperties());
   
   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_handleUndo()
{
   // this function isn't very well documented and could very well be buggy
   // it is based off of my interpretation of a single test file
   UT_DEBUGMSG(("WordPerfect: Handling an undo group\n"));
   long startPosition = ftell(m_importFile);

   int undoType = fgetc(m_importFile);
   X_CheckFileError(undoType);
      
   if(undoType==0 && !m_undoOn)
     {	
	m_undoOn=true;
	UT_DEBUGMSG(("WordPerfect: undo is now ON\n"));
     }
   
   else if(undoType==1 && m_undoOn)
     {	
	m_undoOn=false;
	UT_DEBUGMSG(("WordPerfect: undo is now OFF\n"));
     }

   X_CheckWordPerfectError(_skipGroup(startPosition, WP_UNDO_GROUP_SIZE));
   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_handleParagraphGroupJustification()
{
   UT_DEBUGMSG(("WordPerfect: Handling a paragraph group's justification\n"));
 
   unsigned char paragraphJustification;
   X_CheckFileReadElementError(fread(&paragraphJustification, sizeof(unsigned char), 1, m_importFile));

   if(!m_undoOn)
     {	
	switch(paragraphJustification)
	  {
	   case WP_PARAGRAPH_GROUP_JUSTIFICATION_LEFT:
	     m_paragraphProperties.m_justificationMode = WordPerfectParagraphProperties::left;
	     break;
	   case WP_PARAGRAPH_GROUP_JUSTIFICATION_FULL:
	     m_paragraphProperties.m_justificationMode = WordPerfectParagraphProperties::full;
	     break;
	   case WP_PARAGRAPH_GROUP_JUSTIFICATION_CENTER:
	     m_paragraphProperties.m_justificationMode = WordPerfectParagraphProperties::center;
	     break;
	   case WP_PARAGRAPH_GROUP_JUSTIFICATION_RIGHT:
	     m_paragraphProperties.m_justificationMode = WordPerfectParagraphProperties::right;
	     break;
	   case WP_PARAGRAPH_GROUP_JUSTIFICATION_FULL_ALL_LINES:
	     m_paragraphProperties.m_justificationMode = WordPerfectParagraphProperties::fullAllLines;
	     break;
	   case WP_PARAGRAPH_GROUP_JUSTIFICATION_RESERVED:
	     m_paragraphProperties.m_justificationMode = WordPerfectParagraphProperties::reserved;
	     break;
	  }
    	UT_DEBUGMSG(("WordPerfect: Paragraph Justification is now: %i\n", paragraphJustification));
     }
   
   return UT_OK;
}

// _handleVariableGroupHeader: gets the info common to all groups
UT_Error IE_Imp_WordPerfect::_handleVariableGroupHeader(long &startPosition, unsigned char &subGroup, UT_uint16 &size, unsigned char &flags)
{
   startPosition = ftell(m_importFile);
   
   X_CheckFileReadElementError(fread(&subGroup, sizeof(unsigned char), 1, m_importFile));
   X_CheckFileReadElementError(fread(&size, sizeof(UT_uint16), 1, m_importFile)); 
   X_CheckFileReadElementError(fread(&flags, sizeof(unsigned char), 1, m_importFile));
   
   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_skipGroup(long startPosition, UT_uint16 groupSize)
{
   UT_DEBUGMSG(("WordPerfect: Skipping a group\n"));
   if (fseek(m_importFile, (startPosition + groupSize - 1), SEEK_SET) != 0)
     return UT_IE_IMPORTERROR;

   return UT_OK;
}


// insert the text in the current textbuf to the document, taking its
// style into account
UT_Error IE_Imp_WordPerfect::_flushText()
{
   UT_DEBUGMSG(("WordPerfect: Flushing Text\n"));
   	
   // append the current paragraph properties if they are changed; m_paragraphChanged is initialized with true, so the first time
   // we will have a structure to insert into
   if(m_paragraphChanged)
     {	
	_appendCurrentParagraphProperties();
     }
   
   if(m_textBuf.getLength() > 0)
     X_CheckDocumentError(getDoc()->appendSpan(m_textBuf.getPointer(0), m_textBuf.getLength()));   
   m_textBuf.truncate(0);
   UT_DEBUGMSG(("WordPerfect: Text flushed\n"));
     
   
   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_appendCurrentTextProperties()
{
   UT_DEBUGMSG(("WordPerfect: Appending current text properties\n"));
   
   XML_Char* pProps = "props";
   XML_Char propBuffer[1024];	//TODO is this big enough?  better to make it a member and stop running all over the stack
   XML_Char tempBuffer[128];
   propBuffer[0] = 0;
   
   // bold 418
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
   
   sprintf(tempBuffer, "; font-size:%spt", std_size_string((float)m_textAttributes.m_fontSize));
   strcat(propBuffer, tempBuffer);
   
   UT_DEBUGMSG(("Appending Format: %s\n", propBuffer));
   const XML_Char* propsArray[3];
   propsArray[0] = pProps;
   propsArray[1] = propBuffer;
   propsArray[2] = NULL;
   
   X_CheckDocumentError(getDoc()->appendFmt(propsArray));

   return UT_OK;
}

UT_Error IE_Imp_WordPerfect::_appendCurrentParagraphProperties()
{
   UT_DEBUGMSG(("WordPerfect: Appending Paragraph Properties\n"));

   XML_Char* pProps = "props";
   XML_Char propBuffer[1024];	//TODO is this big enough?  better to make it a member and stop running all over the stack
   propBuffer[0] = 0;

   strcat( propBuffer, "text-align:");
   switch( m_paragraphProperties.m_justificationMode )
     {
      case WordPerfectParagraphProperties::left:
	strcat(propBuffer, "left");
	break;
      case WordPerfectParagraphProperties::right:
	strcat(propBuffer, "right");
	break;
      case WordPerfectParagraphProperties::center:
	strcat(propBuffer, "center");
	break;
      case WordPerfectParagraphProperties::full:  	// either normal justification or something I don't understand yet. same deal.
      case WordPerfectParagraphProperties::fullAllLines:
      case WordPerfectParagraphProperties::reserved:
	strcat(propBuffer, "justify");
	break;
     }
   
   UT_DEBUGMSG(("Appending Style: %s\n", propBuffer));
   const XML_Char* propsArray[3];
   propsArray[0] = pProps;
   propsArray[1] = propBuffer;
   propsArray[2] = NULL;
   m_paragraphChanged = false;
   
   X_CheckDocumentError(getDoc()->appendStrux(PTX_Block, propsArray));   
   
   return UT_OK;
}

