/* AbiWord
 * Copyright (C) 1998-2001 AbiSource, Inc.
 * Copyright (C) 2001 Dom Lachowicz <dominicl@seas.upenn.edu>
 * Copyright (C) 2001-2003 Tomas Frydrych
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
#include <locale.h>

#include "wv.h"

#include "ut_string_class.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_units.h"
#include "ut_math.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_stack.h"

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_EncodingManager.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_Password.h"

#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "fg_GraphicVector.h"

#include "pd_Document.h"

#include "ie_imp_MsWord_97.h"
#include "ie_impGraphic.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"

#include "pf_Frag_Strux.h"
#include "pt_PieceTable.h"
#include "pd_Style.h"

#include "fp_PageSize.h"

#include "ut_Language.h"

#ifdef DEBUG
#define IE_IMP_MSWORD_DUMP
#include "ie_imp_MsWord_dump.h"
#undef IE_IMP_MSWORD_DUMP
#endif

#define X_CheckError(v) 		do { if (!(v)) return 1; } while (0)

// undef this to disable support for older images (<= Word95)
#define SUPPORTS_OLD_IMAGES 1

#include <fribidi/fribidi.h>

//#define BIDI_DEBUG
//
// Forward decls. to wv's callbacks
//
static int charProc (wvParseStruct *ps, U16 eachchar, U8 chartype, U16 lid);
static int specCharProc (wvParseStruct *ps, U16 eachchar, CHP* achp);
static int eleProc (wvParseStruct *ps, wvTag tag, void *props, int dirty);
static int docProc (wvParseStruct *ps, wvTag tag);

/*!
    Translates MS numerical id's for standard styles into our names
	The style names that have been commented out are those that do not
    have currently a localised equivalent in AW
*/
static const XML_Char * s_translateStyleId(UT_uint32 id)
{
	if(id >= 4094)
	{
		return NULL;
	}

	// The style names that have been commented out are those that do
	// not currently have a localised equivalent in AW
	switch(id)
	{
		case 0:  return "Normal";
		case 1:  return "Heading 1";
		case 2:  return "Heading 2";
		case 3:  return "Heading 3";
		case 4:  return NULL /*"Heading 4"*/;
		case 5:  return NULL /*"Heading 5"*/;
		case 6:  return NULL /*"Heading 6"*/;
		case 7:  return NULL /*"Heading 7"*/;
		case 8:  return NULL /*"Heading 8"*/;
		case 9:  return NULL /*"Heading 9"*/;
		case 10: return NULL /*"Index 1"*/;
		case 11: return NULL /*"Index 2"*/;
		case 12: return NULL /*"Index 3"*/;
		case 13: return NULL /*"Index 4"*/;
		case 14: return NULL /*"Index 5"*/;
		case 15: return NULL /*"Index 6"*/;
		case 16: return NULL /*"Index 7"*/;
		case 17: return NULL /*"Index 8"*/;
		case 18: return NULL /*"Index 9"*/;
		case 19: return NULL /*"TOC 1"*/;
		case 20: return NULL /*"TOC 2"*/;
		case 21: return NULL /*"TOC 3"*/;
		case 22: return NULL /*"TOC 4"*/;
		case 23: return NULL /*"TOC 5"*/;
		case 24: return NULL /*"TOC 6"*/;
		case 25: return NULL /*"TOC 7"*/;
		case 26: return NULL /*"TOC 8"*/;
		case 27: return NULL /*"TOC 9"*/;
		case 28: return NULL /*"Normal Indent"*/;
		case 29: return "Footnote Text";
		case 30: return NULL /*"Comment Text"*/;
		case 31: return NULL /*"Header"*/;			
		case 32: return NULL /*"Footer"*/;
		case 33: return NULL /*"Index Heading"*/;
		case 34: return NULL /*"Caption"*/;
		case 35: return NULL /*"Table of Figures"*/;
		case 36: return NULL /*"Envelope Address"*/;
		case 37: return NULL /*"Envelope Return"*/;
		case 38: return "Footnote Reference";
		case 39: return NULL /*"Comment Reference"*/;
		case 40: return NULL /*"Line Number"*/;
		case 41: return NULL /*"Page Number"*/;
		case 42: return "Endnote Reference";
		case 43: return "Endnote Text";
		case 44: return NULL /*"Index of Authorities"*/;
		case 45: return NULL /*"Macro Text"*/;
		case 46: return NULL /*"TOA Heading"*/;
		case 47: return NULL /*"List"*/;
		case 48: return "Bulleted List";
		case 49: return "Numbered List";
		case 50: return NULL /*"List 2"*/;
		case 51: return NULL /*"List 3"*/;
		case 52: return NULL /*"List 4"*/;
		case 53: return NULL /*"List 5"*/;
		case 54: return NULL /*"List Bullet 2"*/;
		case 55: return NULL /*"List Bullet 3"*/;
		case 56: return NULL /*"List Bullet 4"*/;
		case 57: return NULL /*"List Bullet 5"*/;
		case 58: return NULL /*"List Number 2"*/;
		case 59: return NULL /*"List Number 3"*/;
		case 60: return NULL /*"List Number 4"*/;
		case 61: return NULL /*"List Number 5"*/;
		case 62: return NULL /*"Title"*/;
		case 63: return NULL /*"Closing"*/;	
		case 64: return NULL /*"Signature"*/;
		case 65: return NULL /*"Default Paragraph Font"*/;
		case 66: return NULL /*"Body Text"*/;
		case 67: return NULL /*"Body Text Indent"*/;
		case 68: return NULL /*"List Continue"*/;
		case 69: return NULL /*"List Continue 2"*/;
		case 70: return NULL /*"List Continue 3"*/;
		case 71: return NULL /*"List Continue 4"*/;
		case 72: return NULL /*"List Continue 5"*/;
		case 73: return NULL /*"Message Header"*/;
		case 74: return NULL /*"Subtitle"*/;
		case 75: return NULL /*"Salutation"*/;
		case 76: return NULL /*"Date"*/;
		case 77: return NULL /*"Body Text First Indent"*/;
		case 78: return NULL /*"Body Text First Indent 2"*/;
		case 79: return NULL /*"Note Heading"*/;
		case 80: return NULL /*"Body Text 2"*/;
		case 81: return NULL /*"Body Text 3"*/;
		case 82: return NULL /*"Body Text Indent 2"*/;
		case 83: return NULL /*"Body Text Indent 3"*/;
		case 84: return "Block Text";
		case 85: return NULL /*"Hyperlink"*/;
		case 86: return NULL /*"FollowedHyperlink"*/;
		case 87: return NULL /*"Strong"*/;
		case 88: return NULL /*"Emphasis"*/;
		case 89: return NULL /*"Document Map"*/;
		case 90: return "Plain Text";
		case 91: return NULL /*"Email Signature"*/;
			
		case 94: return NULL /*"Normal (Web)"*/;
		case 95: return NULL /*"HTML Acronym"*/;
		case 96: return NULL /*"HTML Address"*/;
		case 97: return NULL /*"HTML Cite"*/;
		case 98: return NULL /*"HTML Code"*/;
		case 99: return NULL /*"HTML Definition"*/;
		case 100: return NULL /*"HTML Keyboard"*/;
		case 101: return NULL /*"HTML Preformatted"*/;
		case 102: return NULL /*"HTML Sample"*/;
		case 103: return NULL /*"HTML Typewriter"*/;
		case 104: return NULL /*"HTML Variable"*/;
		case 105: return NULL /*"Table Normal"*/;

		case 107: return NULL /*"No List"*/;

		case 153: return NULL /*"Table of Authorities"*/;

		default:
			UT_DEBUGMSG(("Unknown style Id [%d]; Please submit this document with a bug report!\n", id));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return NULL;
	}
	return NULL;
}

/*!
    Strip characters that would confuse either the xml parser or our
    property parser; caller is responsible to free the returned pointer
*/
static char * s_stripDangerousChars(const char *s)
{
	UT_uint32 j, k;
	if(!s)
		return NULL;
	
	char * t = (char*) malloc(strlen(s)+1);
	UT_return_val_if_fail(t,NULL);
	
	for(j = 0, k = 0; j < strlen(s); )
	{
		switch(s[j])
		{
			default:
				t[k++] = s[j++];
				break;

				// characters that would confuse the
				// xml parser or our own property parser
			case '<':
			case '>':
			case ':':
			case ';':
			case '&':
			case '\"':
				j++;
				break;
		}
	}
	
	t[k] = 0;
	
	return t;
}

//
// DOC uses an unsigned int color index
//
typedef UT_uint32 Doc_Color_t;

//
// A mapping between Word's colors and Abi's RGB color scheme
//
static Doc_Color_t word_colors [][3] = {
	{0x00, 0x00, 0x00}, /* black */
	{0x00, 0x00, 0xff}, /* blue */
	{0x00, 0xff, 0xff}, /* cyan */
	{0x00, 0xff, 0x00}, /* green */
	{0xff, 0x00, 0xff}, /* magenta */
	{0xff, 0x00, 0x00}, /* red */
	{0xff, 0xff, 0x00}, /* yellow */
	{0xff, 0xff, 0xff}, /* white */
	{0x00, 0x00, 0x80}, /* dark blue */
	{0x00, 0x80, 0x80}, /* dark cyan */
	{0x00, 0x80, 0x00}, /* dark green */
	{0x80, 0x00, 0x80}, /* dark magenta */
	{0x80, 0x00, 0x00}, /* dark red */
	{0x80, 0x80, 0x00}, /* dark yellow */
	{0x80, 0x80, 0x80}, /* dark gray */
	{0xc0, 0xc0, 0xc0}, /* light gray */
};

static UT_String sMapIcoToColor (UT_uint16 ico)
{
  return UT_String_sprintf("%02x%02x%02x",
			   word_colors[ico-1][0],
			   word_colors[ico-1][1],
			   word_colors[ico-1][2]);
}

//
// Field Ids that are useful later for mapping
//
typedef enum {
	F_TIME,
	F_DATE,
	F_EDITTIME,
	F_AUTHOR,
	F_PAGE,
	F_NUMCHARS,
	F_NUMPAGES,
	F_NUMWORDS,
	F_FILENAME,
	F_HYPERLINK,
	F_PAGEREF,
	F_EMBED,
	F_TOC,
	F_DateTimePicture,
	F_TOC_FROM_RANGE,
	F_DATEINAME,
	F_SPEICHERDAT,
	F_OTHER
} Doc_Field_t;

struct field
{
	UT_UCS2Char command [FLD_SIZE];
	UT_UCS2Char argument [FLD_SIZE];
	UT_UCS2Char *fieldWhich;
	UT_sint32	fieldI;
	char *		fieldC;
	UT_sint32   fieldRet;
	Doc_Field_t type;
};


//
// A mapping between DOC's field names and our given IDs
//
typedef struct
{
	char * m_name;
	Doc_Field_t m_id;
} Doc_Field_Mapping_t;

/*
 * This next bit of code enables us to import many of Word's fields
 */

static Doc_Field_Mapping_t s_Tokens[] =
{
	{"TIME",	   F_TIME},
	{"EDITTIME",   F_EDITTIME},
	{"DATE",	   F_DATE},
	{"date",	   F_DATE},
	{"DATEINAME",      F_DATE}, // F_DATEINAME
	{"SPEICHERDAT",    F_DATE}, // F_SPEICHERDAT
	{"\\@", 	   F_DateTimePicture},

	{"FILENAME",   F_FILENAME},
	{"\\filename", F_FILENAME},
	{"PAGE",	   F_PAGE},
	{"NUMCHARS",   F_NUMCHARS},
	{"NUMWORDS",   F_NUMWORDS},

	// these below aren't handled by AbiWord, but they're known about
	{"HYPERLINK",  F_HYPERLINK},
	{"PAGEREF",    F_PAGEREF},
	{"EMBED",	   F_EMBED},
	{"TOC", 	   F_TOC},
	{"\\o", 	   F_TOC_FROM_RANGE},
	{"AUTHOR",	   F_AUTHOR},

	{ "*",		   F_OTHER}
};

#define FieldMappingSize (sizeof(s_Tokens)/sizeof(s_Tokens[0]))

static Doc_Field_t
s_mapNameToField (const char * name)
{
	for (unsigned int k = 0; k < FieldMappingSize; k++)
	{
		// field names can be sometimes in lower-case
		if (!UT_stricmp(s_Tokens[k].m_name,name))
			return s_Tokens[k].m_id;
	}
	return F_OTHER;
}

#undef FieldMappingSize

static const char *
s_mapPageIdToString (UT_uint16 id)
{
	// TODO: make me way better when we determine code names

	switch (id)
	{
		case 0:  return "Letter";
		case 5:  return "Legal";
		case 7:  return NULL; //"Executive";
		case 9:  return "A4";
		case 11: return "A5";
		case 13: return "Folio";
		case 14: return NULL; // in Word this is "B5" but the size
							  // does not correspond to AW's B5
		case 20: return "Envelope No10";
		case 27: return "DL Envelope";
		case 28: return "C5";
		case 34: return "B5"; // in Word this is B5 Envelope ...
		case 37: return NULL; //"Monarch Envelope";

		case 0xffff:
			// this is a value that wv uses to indicate that page size
			// is customised, just return NULL
			return NULL;
			
		default:
			UT_DEBUGMSG(("Unknow page size: please submit this document with a bug report\n"));
			UT_ASSERT( 0 );
			return 0;
	}
}

/*!
  Surprise, surprise, there are more list numerical formats than the 5 the
  MS documentation states happens to mention, so here I will put what I found
  out (later we will move it to some better place)
*/
typedef enum
{
  WLNF_INVALID		   = -1,
  WLNF_EUROPEAN_ARABIC = 0,
  WLNF_UPPER_ROMAN	   = 1,
  WLNF_LOWER_ROMAN	   = 2,
  WLNF_UPPER_LETTER    = 3,
  WLNF_LOWER_LETTER    = 4,
  WLNF_ORDINAL		   = 5,
  WLNF_BULLETS		   = 23,
  WLNF_HEBREW_NUMBERS  = 45
} MSWordListIdType;

typedef struct{
  UT_uint32 listId;
  UT_uint32 level;
} ListIdLevelPair;

/*!
 * Map msword list enums back to abi's
 */
static const char *
s_mapDocToAbiListId (MSWordListIdType id)
{
  switch (id)
	{
	case WLNF_UPPER_ROMAN: // upper roman
	  return "4";

	case WLNF_LOWER_ROMAN: // lower roman
	  return "3";

	case WLNF_UPPER_LETTER: // upper letter
	  return "2";

	case WLNF_LOWER_LETTER: // lower letter
	  return "1";

	case WLNF_BULLETS: // bullet list
	  return "5";

	case WLNF_HEBREW_NUMBERS:
	  return "129";

	case WLNF_EUROPEAN_ARABIC:
	case WLNF_ORDINAL: // ordinal
	default:
	  return "0";
	}
}

/*!
 * form AW list deliminator string
 */
static void s_mapDocToAbiListDelim (UT_uint16 * pStr, UT_uint32 iLen, UT_String &sDelim)
{
	// the Word format string looks like this
	//    prefix '\0' suffix
	// and the '\0' represents the location of the list number/bullet
	UT_uint16 * pPfx = NULL;
	UT_uint16 * pSfx = NULL;

	if(iLen && *pStr)
		pPfx = pStr;

	UT_sint32 i;
	for(i = 0; i < (UT_sint32)iLen - 1; i++)
	{
		if(pStr[i] == 0)
		{
			pSfx = pStr + i + 1;
			break;
		}
	}
	
	UT_UTF8String sUtf8Pfx;
	UT_UTF8String sUtf8Sfx;

	i= 0;
	while(pPfx && *pPfx && i < (UT_sint32)iLen)
	{
		UT_UCS4Char c = *pPfx;
		sUtf8Pfx.appendUCS4(&c,1);
		i++;
		pPfx++;
	}

	i++; // move past the '\0' divider
	while(pSfx && *pSfx && i < (UT_sint32)iLen)
	{
		UT_UCS4Char c = *pSfx;
		sUtf8Sfx.appendUCS4(&c,1);
		i++;
		pSfx++;
	}

	sDelim.clear();

	UT_String_sprintf(sDelim, "%s%%L%s",sUtf8Pfx.utf8_str(), sUtf8Sfx.utf8_str());
}

/*!
 * Map msword list enums back to abi's list styles
 */
static const char *
s_mapDocToAbiListStyle (MSWordListIdType id)
{
  switch (id)
	{
	case WLNF_UPPER_ROMAN: // upper roman
	  return "Upper Roman List";

	case WLNF_LOWER_ROMAN: // lower roman
	  return "Lower Roman List";

	case WLNF_UPPER_LETTER: // upper letter
	  return "Upper Case List";

	case WLNF_LOWER_LETTER: // lower letter
	  return "Lower Case List";

	case WLNF_BULLETS: // bullet list
	  return "Bullet List";

	case WLNF_EUROPEAN_ARABIC:
	case WLNF_ORDINAL: // ordinal
	default:
	  return "Numbered List";
	}
}

/*!
 * Map msword list enums back to abi's field font for that given style
 */
static const char *
s_fieldFontForListStyle (MSWordListIdType id)
{
  switch (id)
	{
	case WLNF_UPPER_ROMAN: // upper roman
	  return "NULL";

	case WLNF_LOWER_ROMAN: // lower roman
	  return "NULL";

	case WLNF_UPPER_LETTER: // upper letter
	  return "Times New Roman";

	case WLNF_LOWER_LETTER: // lower letter
	  return "Times New Roman";

	case WLNF_BULLETS: // bullet list
		UT_DEBUGMSG(("Fieldfont set to symbol \n"));
	  return "Symbol";

	case WLNF_EUROPEAN_ARABIC:
	case WLNF_ORDINAL: // ordinal
		return "Times New Roman";
		
	default:
		UT_DEBUGMSG(("unknown list type %d field-font set to Times New Roman \n",id));
	  return "Times New Roman";
	}
}

#if 0

// MS Word uses the langauge codes as explicit overrides when treating
// weak characters; this function translates language id to the
// overrided direction
static bool s_isLanguageRTL(short unsigned int lid)
{
	const char * s = wvLIDToLangConverter (lid);
	UT_Language l;
	return (UTLANG_RTL == l.getOrderFromProperty(s));
}

static FootnoteType s_convertNoteType(UT_uint32 t)
{
	return 	FOOTNOTE_TYPE_NUMERIC;
}

#endif

/****************************************************************************/
/****************************************************************************/

IE_Imp_MsWord_97_Sniffer::IE_Imp_MsWord_97_Sniffer ()
	: IE_ImpSniffer(IE_IMPEXPNAME_MSWORD97)
{
	//
}

UT_Confidence_t IE_Imp_MsWord_97_Sniffer::supportsMIME (const char * szMIME)
{
	if (UT_strcmp (IE_FileInfo::mapAlias (szMIME), IE_MIME_MSWord) == 0)
		{
			return UT_CONFIDENCE_GOOD;
		}
	return UT_CONFIDENCE_ZILCH;
}

UT_Confidence_t IE_Imp_MsWord_97_Sniffer::recognizeContents (const char * szBuf,
												  UT_uint32 iNumbytes)
{
	char * magic	= 0;
	int magicoffset = 0;

	magic = "Microsoft Word 6.0 Document";
	magicoffset = 2080;
	if (iNumbytes > (magicoffset + strlen (magic)))
	{
		if (!strncmp (szBuf + magicoffset, magic, strlen (magic)))
		{
			return UT_CONFIDENCE_PERFECT;
		}
	}

	magic = "Documento Microsoft Word 6";
	magicoffset = 2080;
	if (iNumbytes > (magicoffset + strlen (magic)))
	{
		if (!strncmp(szBuf + magicoffset, magic, strlen (magic)))
		{
			return UT_CONFIDENCE_PERFECT;
		}
	}

	magic = "MSWordDoc";
	magicoffset = 2112;
	if (iNumbytes > (magicoffset + strlen (magic)))
	{
		if (!strncmp (szBuf + magicoffset, magic, strlen (magic)))
		{
			return UT_CONFIDENCE_PERFECT;
		}
	}

	// ok, that didn't work, we'll try to dig through the OLE stream
	if (iNumbytes > 8)
	{
	        // this code is too generic - also picks up .wri documents
		if (szBuf[0] == static_cast<char>(0x31)
			&& static_cast< unsigned char>(szBuf[1]) == static_cast< unsigned char>(0xbe)
			&&  szBuf[2] == static_cast<char>(0)
			&& szBuf[3] == static_cast<char>(0))
		{
		  return UT_CONFIDENCE_SOSO; //POOR
		}

		// this identifies staroffice dox as well
		if (static_cast< unsigned char>(szBuf[0]) == static_cast<unsigned char>(0xd0)
			&& static_cast< unsigned char>(szBuf[1]) == static_cast<unsigned char>(0xcf)
			&& szBuf[2] == static_cast<char>(0x11)
			&& static_cast< unsigned char>(szBuf[3]) == static_cast<unsigned char>(0xe0)
			&& static_cast< unsigned char>(szBuf[4]) == static_cast<unsigned char>(0xa1)
			&& static_cast< unsigned char>(szBuf[5]) == static_cast<unsigned char>(0xb1)
			&& szBuf[6] == static_cast<char>(0x1a)
			&& static_cast< unsigned char>(szBuf[7]) == static_cast<unsigned char>(0xe1))
		{
		  return UT_CONFIDENCE_SOSO; // POOR
		}

		if (szBuf[0] == 'P' && szBuf[1] == 'O' &&
			szBuf[2] == '^' && szBuf[3] == 'Q' && szBuf[4] == '`')
		{
			return UT_CONFIDENCE_POOR;
		}
		if (static_cast< unsigned char>(szBuf[0]) == static_cast<unsigned char>(0xfe)
			&& szBuf[1] == static_cast<char>(0x37)
			&& szBuf[2] == static_cast<char>(0)
			&& szBuf[3] == static_cast<char>(0x23))
		{
			return UT_CONFIDENCE_POOR;
		}

		if (static_cast< unsigned char>(szBuf[0]) == static_cast<unsigned char>(0xdb)
			&& static_cast< unsigned char>(szBuf[1]) == static_cast<unsigned char>(0xa5)
			&& szBuf[2] == static_cast<char>(0x2d)
			&& szBuf[3] == static_cast<char>(0)
			&& szBuf[4] == static_cast<char>(0) && szBuf[5] == static_cast<char>(0))
		{
			return UT_CONFIDENCE_POOR;
		}
	}
	return UT_CONFIDENCE_ZILCH;
}

UT_Confidence_t IE_Imp_MsWord_97_Sniffer::recognizeSuffix (const char * szSuffix)
{
	// We recognize both word documents and their template versions
	if (!UT_stricmp(szSuffix,".doc") ||
			!UT_stricmp(szSuffix,".dot"))
	  return UT_CONFIDENCE_PERFECT;
	return UT_CONFIDENCE_ZILCH;
}

UT_Error IE_Imp_MsWord_97_Sniffer::constructImporter (PD_Document * pDocument,
													  IE_Imp ** ppie)
{
	IE_Imp_MsWord_97 * p = new IE_Imp_MsWord_97(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_MsWord_97_Sniffer::getDlgLabels (const char ** pszDesc,
												const char ** pszSuffixList,
												IEFileType * ft)
{
	*pszDesc = "Microsoft Word (.doc, .dot)";
	*pszSuffixList = "*.doc; *.dot";
	*ft = getFileType();
	return true;
}

/****************************************************************************/
/****************************************************************************/

// just buffer sizes, arbitrarily chosen
#define DOC_TEXTRUN_SIZE 2048
#define DOC_PROPBUFFER_SIZE 1024

IE_Imp_MsWord_97::~IE_Imp_MsWord_97()
{
	if(m_pBookmarks)
	{
		// free the names from the bookmarks
		for(UT_uint32 i = 0; i < m_iBookmarksCount; i++)
		{
			// make sure we do not delete any name twice
			if(m_pBookmarks[i].name && m_pBookmarks[i].start)
			{
			   delete[] m_pBookmarks[i].name;
			   m_pBookmarks[i].name = NULL;
			}
		}
		delete [] m_pBookmarks;
	}

	UT_VECTOR_PURGEALL(ListIdLevelPair *, m_vLists);
	UT_VECTOR_PURGEALL(emObject *, m_vecEmObjects);

	if(m_pFootnotes)
		delete [] m_pFootnotes;

	if(m_pEndnotes)
		delete [] m_pEndnotes;

	if(m_pHeaders)
		delete [] m_pHeaders;
}

IE_Imp_MsWord_97::IE_Imp_MsWord_97(PD_Document * pDocument)
  : IE_Imp (pDocument),
	m_nSections(0),
	m_bSetPageSize(false),
	m_bIsLower(false),
	m_bInSect(false),
	m_bInPara(false),
	m_bLTRCharContext(true),
	m_bLTRParaContext(true),
	m_iOverrideIssued(FRIBIDI_TYPE_UNSET),
	m_bBidiMode(false),
	m_pBookmarks(NULL),
	m_iBookmarksCount(0),
	m_pFootnotes(NULL),
	m_iFootnotesCount(0),
	m_pEndnotes(NULL),
	m_iEndnotesCount(0),
	m_iMSWordListId(0),
    m_bEncounteredRevision(false),
    m_bInTable(false),
	m_iRowsRemaining(0),
    m_iCellsRemaining(0),
    m_iCurrentRow(0),
    m_iCurrentCell(0),
    m_bRowOpen(false),
	m_bCellOpen(false),
	m_iFootnotesStart(0xffffffff),
	m_iFootnotesEnd(0xffffffff),
	m_iEndnotesStart(0xffffffff),
	m_iEndnotesEnd(0xffffffff),
	m_iNextFNote(0),
	m_iNextENote(0),
	m_bInFNotes(false),
	m_bInENotes(false),
	m_pNotesEndSection(NULL),
	m_pHeaders(NULL),
	m_iHeadersCount(0),
	m_iHeadersStart(0xffffffff),
	m_iHeadersEnd(0xffffffff),
	m_iCurrentHeader(0),
	m_bInHeaders(false),
	m_iCurrentSectId(0),
	m_iAnnotationsStart(0xffffffff),
	m_iAnnotationsEnd(0xffffffff),
	m_iMacrosStart(0xffffffff),
	m_iMacrosEnd(0xffffffff),
	m_iTextStart(0xffffffff),
	m_iTextEnd(0xffffffff),
	m_bPageBreakPending(false),
	m_bSymbolFont(false),
	m_dim(DIM_IN)
{
  for(UT_uint32 i = 0; i < 9; i++)
	  m_iListIdIncrement[i] = 0;
}

/****************************************************************************/
/****************************************************************************/

#define ErrCleanupAndExit(code)  do {wvOLEFree (&ps); return(code);} while(0)

#define GetPassword() _getPassword ( getDoc()->getApp()->getLastFocussedFrame() )

#define ErrorMessage(x) do { XAP_Frame *_pFrame = getDoc()->getApp()->getLastFocussedFrame(); if ( _pFrame ) _errorMessage (_pFrame, (x)); } while (0)

static UT_UTF8String _getPassword (XAP_Frame * pFrame)
{
  UT_UTF8String password ( "" );

  if ( pFrame )
    {
      pFrame->raise ();

      XAP_DialogFactory * pDialogFactory
		  = (XAP_DialogFactory *)(pFrame->getDialogFactory());

      XAP_Dialog_Password * pDlg = static_cast<XAP_Dialog_Password*>(pDialogFactory->requestDialog(XAP_DIALOG_ID_PASSWORD));
      UT_return_val_if_fail(pDlg, password);

      pDlg->runModal (pFrame);

      XAP_Dialog_Password::tAnswer ans = pDlg->getAnswer();
      bool bOK = (ans == XAP_Dialog_Password::a_OK);

      if (bOK)
		  password = pDlg->getPassword ();

      pDialogFactory->releaseDialog(pDlg);
    }

  return password;
}

static void _errorMessage (XAP_Frame * pFrame, int id)
{
  UT_return_if_fail(pFrame);

  const XAP_StringSet * pSS = XAP_App::getApp ()->getStringSet ();

  const char * text = pSS->getValue (id, pFrame->getApp()->getDefaultEncoding()).c_str();

  pFrame->showMessageBox (text, XAP_Dialog_MessageBox::b_O,
						  XAP_Dialog_MessageBox::a_OK);
}

UT_Error IE_Imp_MsWord_97::importFile(const char * szFilename)
{
  wvParseStruct ps;

  int ret = wvInitParser (&ps, const_cast<char *>(szFilename));
  const char * password = NULL;

  // HACK!!
  bool decrypted = false ;

  if (ret & 0x8000)		/* Password protected? */
    {
      UT_UTF8String pass (GetPassword());
      if ( pass.size () != 0 )
		  password = pass.utf8_str();

      if ((ret & 0x7fff) == WORD8)
	{
	  ret = 0;
	  if (password == NULL)
	    {
			//ErrorMessage(AP_STRING_ID_WORD_PassRequired);
	      ErrCleanupAndExit(UT_IE_PROTECTED);
	    }
	  else
	    {
	      wvSetPassword (password, &ps);
	      if (wvDecrypt97 (&ps))
		{
			//ErrorMessage(AP_STRING_ID_WORD_PassInvalid);
		  ErrCleanupAndExit(UT_IE_PROTECTED);
		}
	      decrypted = true ;
	    }
	}
      else if (((ret & 0x7fff) == WORD7) || ((ret & 0x7fff) == WORD6))
	{
	  ret = 0;
	  if (password == NULL)
	    {
			//ErrorMessage(AP_STRING_ID_WORD_PassRequired);
	      ErrCleanupAndExit(UT_IE_PROTECTED);
	    }
	  else
	    {
	      wvSetPassword (password, &ps);
	      if (wvDecrypt95 (&ps))
		{
		  //("Incorrect Password\n"));
		  ErrCleanupAndExit(UT_IE_PROTECTED);
		}
	      decrypted = true ;
	    }
	}
    }

  if (ret) {
    ErrCleanupAndExit(UT_IE_BOGUSDOCUMENT);
  }

  // register ourself as the userData
  ps.userData = this;

  // register callbacks
  wvSetElementHandler (&ps, eleProc);
  wvSetCharHandler (&ps, charProc);
  wvSetSpecialCharHandler(&ps, specCharProc);
  wvSetDocumentHandler (&ps, docProc);

  // need to init doc props
  getDoc()->setAttrProp(NULL);
  
  UT_DEBUGMSG(("DOM: wvText\n"));

  wvText(&ps);

  UT_DEBUGMSG(("DOM: about to get summary information\n"));

  // now get the summary information, if available
  ret = wvQuerySupported (&ps.fib, NULL);

  // word 2 used an OLE like mechanism inside of a FILE*, but
  // good luck trying to ms_ole_summary_open something using that...
  if (WORD2 != ret)
    {
      MsOleSummary *summary = ms_ole_summary_open (ps.ole_file);
      if (summary)
	{
	  UT_DEBUGMSG(("DOM: getting summary information\n"));

	  UT_UTF8String prop_str;
	  gboolean found = FALSE;

	  // title
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_TITLE, &found);
	  if (found && prop_str.size())
	    getDoc()->setMetaDataProp ( PD_META_KEY_TITLE, prop_str ) ;

	  // subject
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_SUBJECT, &found);
	  if (found && prop_str.size())
	    getDoc()->setMetaDataProp ( PD_META_KEY_SUBJECT, prop_str ) ;

	  // author
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_AUTHOR, &found);
	  if (found && prop_str.size())
	    getDoc()->setMetaDataProp ( PD_META_KEY_CREATOR, prop_str ) ;

	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_LASTAUTHOR, &found);
	  if (found && prop_str.size())
	    getDoc()->setMetaDataProp ( PD_META_KEY_CONTRIBUTOR, prop_str ) ;

	  // keywords
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_KEYWORDS, &found);
	  if (found && prop_str.size())
	    getDoc()->setMetaDataProp ( PD_META_KEY_KEYWORDS, prop_str ) ;

	  // comments
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_COMMENTS, &found);
	  if (found && prop_str.size())
	    getDoc()->setMetaDataProp ( PD_META_KEY_DESCRIPTION, prop_str ) ;

	  // below this line are from Document Summary Information

	  ms_ole_summary_close (summary);
	  summary = ms_ole_docsummary_open(ps.ole_file);

	  if(summary){
	    // category
	    prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_CATEGORY, &found);
	    if (found && prop_str.size())
	      getDoc()->setMetaDataProp ( PD_META_KEY_TYPE, prop_str ) ;

	    // organization
	    prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_COMPANY, &found);
	    if (found && prop_str.size())
	      getDoc()->setMetaDataProp ( PD_META_KEY_PUBLISHER, prop_str ) ;

	    ms_ole_summary_close (summary);
	  }
	}
    }

  UT_DEBUGMSG(("DOM: finished summary info\n"));

  // HACK - this will do until i sort out some global stream ugliness in wv
  if ( !decrypted && WORD2 != ret )
    wvOLEFree(&ps);

  // We can't be in a good state if we didn't add any sections!
  if (m_nSections == 0)
    return UT_IE_BOGUSDOCUMENT;

  return UT_OK;
}

void IE_Imp_MsWord_97::_flush ()
{
  if(!m_pTextRun.size())
	return;

  // we've got to ensure that we're inside of a section & paragraph
  if (!m_bInSect)
	{
	  // append a blank default section - assume it works
	  UT_DEBUGMSG(("#TF: _flush: appending default section\n"));
	  _appendStrux(PTX_Section, NULL);
	  m_bInSect = true;
	  m_nSections++;
	}

  if(!m_bInPara)
  {
	  // append a blank defaul paragraph - assume it works
	  UT_DEBUGMSG(("#TF: _flush: appending default block\n"));
	  _appendStrux(PTX_Block, NULL);
	  m_bInPara = true;
	  emObject * pObject = NULL;
	  if(m_vecEmObjects.getItemCount() > 0)
	  {
		  UT_sint32 i =0;
		  for(i=0;i< static_cast<UT_sint32>(m_vecEmObjects.getItemCount()); i++)
		  {
			  pObject = static_cast<emObject *>(m_vecEmObjects.getNthItem(i));
			  const XML_Char* propsArray[5];
			  if(pObject->objType == PTO_Bookmark)
			  {
				  propsArray[0] = static_cast<const XML_Char *>("name");
				  propsArray[1] = static_cast<const XML_Char *>(pObject->props1.c_str());
				  propsArray[2] = static_cast<const XML_Char *>("type");
				  propsArray[3] = static_cast<const XML_Char *>(pObject->props2.c_str());
				  propsArray[4] = static_cast<const XML_Char *>(NULL);
				  _appendObject (PTO_Bookmark, propsArray);
			  }
			  else
			  {
				  UT_DEBUGMSG(("MSWord 97 _flush: Object not handled \n"));
				  UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			  }
			  delete pObject;
		  }
		  m_vecEmObjects.clear();
	  }
  }

  if (m_pTextRun.size())
  {
	  // bidi adjustments for neutrals
	  // 
	  // We have a problem in bidi documents caused by the fact that
	  // Word does not use the Unicode bidi algorithm, but rather one of
	  // its own, which adds keyboard language to the equation. We get
	  // around this by issuing an explicit direction override on the
	  // neutral characters. We do it here in the _flush() function
	  // because when we have both left and right context available
	  // for these characters we can tell if the override is
	  // superfluous, which it is most of the time; omitting the
	  // sufperfluous overrides allows us to import documents in a
	  // manner that will make them feel more like native AW docs.
	  // (This does not get rid of all the unnecessary overrides, for
	  // that we would need to have the text of an entire paragraph)
	  //
	  // I goes without saying that it would be highly desirable to be
	  // able to determine at the start if a document is pure LTR (as
	  // we do in the RTF importer), since that would save us lot of
	  // extra processing
	  // Tomas, May 8, 2003
	  
	  if(m_bBidiMode)
	  {
		  const XML_Char* pProps = "props";
		  UT_String prop_basic = m_charProps;

		  UT_String prop_ltr = prop_basic;
		  UT_String prop_rtl = prop_basic;

		  if(prop_basic.size())
		  {
			  prop_ltr += ";";
			  prop_rtl += ";";
		  }
		  else
		  {
			  // if the char props are empty, we need replace them
			  // with the following to avoid asserts in PP_AttrProp
			  prop_basic = "dir-override:";
		  }
		  
		  
		  prop_ltr += "dir-override:ltr";
		  prop_rtl += "dir-override:rtl";
	
		  const XML_Char rev[] ="revision";

		  const XML_Char* propsArray[5];
		  propsArray[0] = pProps;
		  propsArray[1] = prop_basic.c_str();
		  propsArray[2] = NULL;
		  propsArray[3] = NULL;
		  propsArray[4] = NULL;

		  UT_uint32 iEmptyAttrib = 2;

		  if(m_charRevs.size())
		  {
			  propsArray[iEmptyAttrib++] = &rev[0];
			  propsArray[iEmptyAttrib++] = m_charRevs.c_str();
		  }
		  
		  const UT_UCS4Char * p;
		  const UT_UCS4Char * pStart = m_pTextRun.ucs4_str();
		  UT_uint32 iLen = m_pTextRun.size();
		  
		  FriBidiCharType iOverride = FRIBIDI_TYPE_UNSET, cType, cLastType = FRIBIDI_TYPE_UNSET, cNextType;
		  UT_uint32 iLast = 0;
		  UT_UCS4Char c = *pStart;
	
		  cType = fribidi_get_type(c);
	
		  for(UT_uint32 i = 0; i < iLen; i++)
		  {
			  if(i < iLen - 1 )
			  {
				  c = *(pStart+i+1);
				  cNextType = fribidi_get_type(c);
			  }
			  else
			  {
				  cNextType = FRIBIDI_TYPE_UNSET;
			  }
		
		
			  if(FRIBIDI_IS_NEUTRAL(cType))
			  {
				  if(m_bLTRCharContext
					 && iOverride != FRIBIDI_TYPE_LTR
					 && (cLastType != FRIBIDI_TYPE_LTR || cNextType != FRIBIDI_TYPE_LTR))
				  {
					  if(i - iLast > 0)
					  {
						  p = pStart + iLast;
						  if(!_appendFmt(propsArray))
							  return;
					
						  if(!_appendSpan(p, i - iLast))
							  return;
					  }
					  iOverride = FRIBIDI_TYPE_LTR;
					  propsArray[1] = prop_ltr.c_str();
					  iLast = i;
				  }
				  else if(!m_bLTRCharContext
						  && iOverride != FRIBIDI_TYPE_RTL
						  && (cLastType != FRIBIDI_TYPE_RTL || cNextType != FRIBIDI_TYPE_RTL))
				  {
					  if(i - iLast > 0)
					  {
						  p = pStart + iLast;
						  if(!_appendFmt(propsArray))
							  return;

						  if(!_appendSpan(p, i - iLast))
							  return;
					  }
					  iOverride = FRIBIDI_TYPE_RTL;
					  propsArray[1] = prop_rtl.c_str();
					  iLast = i;
				  }
			  }
			  else
			  {
				  // strong character; if we previously issued an override,
				  // we need to cancel it
				  if(iOverride != FRIBIDI_TYPE_UNSET)
				  {
					  if(i - iLast > 0)
					  {
						  p = pStart + iLast;
						  if(!_appendFmt(propsArray))
							  return;
					
						  if(!_appendSpan(p, i - iLast))
							  return;
					  }
					  iOverride = FRIBIDI_TYPE_UNSET;
					  propsArray[1] = prop_basic.c_str();
					  iLast = i;
				  }
			  }

			  cLastType = cType;
			  cType = cNextType;
		  }

		  // insert what is left over
		  if(iLen - iLast > 0)
		  {
			  p = pStart + iLast;
			  if(!_appendFmt(propsArray))
				  return;
					
			  if(!_appendSpan(p, iLen - iLast))
				  return;
		  }
	  }
	  else
	  {
		  // non-bidi document, just do it the easy way
		  if (!_appendSpan(m_pTextRun.ucs4_str(), m_pTextRun.size()))
		  {
			  UT_DEBUGMSG(("DOM: error appending text run\n"));
			  return;
		  }
	  }
	  
	  m_pTextRun.clear();
  }
}

void IE_Imp_MsWord_97::_appendChar (UT_UCSChar ch)
{
  if (m_bInTable) {
    switch (ch) {
    case 7:			// eat tab characters
      return;
    case 30:		// ??
      ch = '-';
		  break;
    }
  }

  if ( m_bIsLower )
    ch = UT_UCS4_tolower ( ch );
  m_pTextRun += ch;
}

/****************************************************************************/
/****************************************************************************/

static int s_cmp_bookmarks_qsort(const void * a, const void * b)
{
	const bookmark * A = static_cast<const bookmark *>(a);
	const bookmark * B = static_cast<const bookmark *>(b);

	if(A->pos != B->pos)
		return (A->pos - B->pos);
	else
		// for bookmarks with identical position we want any start bookmarks to be
		// before end bookmarks.
		return static_cast<UT_sint32>(B->start) - static_cast<UT_sint32>(A->start);
}

static int s_cmp_bookmarks_bsearch(const void * a, const void * b)
{
	UT_uint32 A = *static_cast<const UT_uint32 *>(a);
	const bookmark * B = static_cast<const bookmark *>(b);

	return (A - B->pos);
}

XML_Char * IE_Imp_MsWord_97::_getBookmarkName(const wvParseStruct * ps, UT_uint32 pos)
{
	XML_Char *str;
	UT_iconv_t ic_handle;
	// word bookmarks can be at most 30 characters, so make a reasonable buffer
	// for the UTF-8 version
	char buff[200];
	char *buff_ptr = &buff[0];
	const char *in_ptr;
	size_t out_left = 200, in_left;

	if (!XAP_EncodingManager::get_instance()->cjk_locale()
	   &&(XAP_EncodingManager::get_instance()->try_nativeToU(0xa1) != 0xa1))
	{
		ic_handle = UT_iconv_open(XAP_EncodingManager::get_instance()->getNativeEncodingName(), "UCS-2");
	}
	else
	{
		// use UTF-8
		ic_handle = UT_iconv_open("UTF-8", "UCS-2LE");
	}

	if(ps->Sttbfbkmk.extendedflag == 0xFFFF)
	{
		// 16 bit stuff
		in_ptr = reinterpret_cast<const char *>(ps->Sttbfbkmk.u16strings[pos]);

		// TODO is this really UCS-2 or UTF-16?
		// TODO and are we using strlen for the number of 16-bit words
		// TODO or the number of characters?
		// TODO Because UTF-16 characters are sometimes expressed as 2 words
		in_left = 2 * UT_UCS2_strlen(static_cast<const UT_UCS2Char*>(ps->Sttbfbkmk.u16strings[pos])) + 2;

		UT_iconv( ic_handle, &in_ptr, &in_left, &buff_ptr,&out_left);
		str = new XML_Char[200 - out_left];
		strcpy(str, buff);
	}
	else
	{
		// 8 bit stuff
		// there is a bug in wv, and the table gets incorrectly retrieved
		// if it contains 8-bit strings
		if(ps->Sttbfbkmk.s8strings[pos])
		{
			UT_uint32 len = strlen(ps->Sttbfbkmk.s8strings[pos]);
			str = new XML_Char[len + 1];
			UT_uint32 i = 0;
			for(i = 0; i < len; i++)
				str[i] = ps->Sttbfbkmk.s8strings[pos][i];
			str[i] = 0;
		}
		else
			str = NULL;
	}

	return str;
}

int IE_Imp_MsWord_97::_docProc (wvParseStruct * ps, UT_uint32 tag)
{
	// flush out any pending character data
	this->_flush ();

	switch (static_cast<wvTag>(tag))
	{
	case DOCBEGIN:

		// test the bidi nature of this document
#ifdef BIDI_DEBUG
		m_bBidiMode = wvIsBidiDocument(ps);
		UT_DEBUGMSG(("IE_Imp_MsWord_97::_docProc: complex %d, bidi %d\n",
					 ps->fib.fComplex,m_bBidiMode));
#else
		// for now we will assume that all documents are bidi
		// documents (Tomas, Apr 12, 2003)
		
		m_bBidiMode = false;
#endif
		// import styles
		_handleStyleSheet(ps);

		// deal with bookmarks
		_handleBookmarks(ps);

		// deal with footnotes and endnotes, headers
		// first, get the doc offsets of the foot/endnote text
		// (We are interested in the offset of this in the document,
		// not in the data stream; therefore, we do not add
		// ps->fib.fcMin for the simple doc
		// Tthere are some strange docs around that have invalid
		// values for the end of endnote section (e.g. the doc from
		// bug 3283); that's what the if's are about.
		m_iTextStart      = 0;
		m_iTextEnd        = ps->fib.ccpText;
		if(m_iTextEnd == 0xffffffff)
			m_iTextEnd = m_iTextStart;
		
		m_iFootnotesStart = m_iTextEnd;
		m_iFootnotesEnd   = m_iFootnotesStart + ps->fib.ccpFtn;
		if(m_iFootnotesEnd == 0xffffffff)
			m_iFootnotesEnd = m_iFootnotesStart;

		m_iHeadersStart   = m_iFootnotesEnd;
		m_iHeadersEnd     = m_iHeadersStart + ps->fib.ccpHdr;
		if(m_iHeadersEnd == 0xffffffff)
			m_iHeadersEnd = m_iHeadersStart;

		m_iMacrosStart    = m_iHeadersEnd;
		m_iMacrosEnd      = m_iMacrosStart + ps->fib.ccpMcr;
		if(m_iMacrosEnd == 0xffffffff)
			m_iMacrosEnd = m_iMacrosStart;

		m_iAnnotationsStart = m_iMacrosEnd;
		m_iAnnotationsEnd = m_iAnnotationsStart + ps->fib.ccpAtn;
		if(m_iAnnotationsEnd == 0xffffffff)
			m_iAnnotationsEnd = m_iAnnotationsStart;

		m_iEndnotesStart  = m_iAnnotationsEnd;
		m_iEndnotesEnd    = m_iEndnotesStart + ps->fib.ccpEdn;
		if(m_iEndnotesEnd == 0xffffffff)
			m_iEndnotesEnd = m_iEndnotesStart;
		
		// now retrieve the note info ...
		_handleNotes(ps);
		_handleHeaders(ps);


		UT_DEBUGMSG(("Fnotes [%d,%d], Enotes [%d,%d]\n",
					 m_iFootnotesStart, m_iFootnotesEnd, m_iEndnotesStart, m_iEndnotesEnd));
		break;
		
	case DOCEND:
	default:
		break;
	}

	return 0;
}

bool IE_Imp_MsWord_97::_insertBookmark(bookmark * bm)
{
	// first of all flush what is in the buffers
	this->_flush();
	bool error = false;

	const XML_Char* propsArray[5];
	propsArray[0] = static_cast<const XML_Char *>("name");
	propsArray[1] = static_cast<const XML_Char *>(bm->name);
	propsArray[2] = static_cast<const XML_Char *>("type");
	propsArray[4] = 0;

	if(bm->start)
		propsArray[3] = static_cast<const XML_Char *>("start");
	else
		propsArray[3] = static_cast<const XML_Char *>("end");

	if(m_bInTable && !m_bCellOpen)
	{
		emObject * pObject = new emObject;
		pObject->props1 = propsArray[1];
		pObject->objType = PTO_Bookmark;
		pObject->props2 = propsArray[3];
		m_vecEmObjects.addItem(static_cast<void *>(pObject));
	}
	else
	{
		if (!_appendObject (PTO_Bookmark, propsArray))
		{
			UT_DEBUGMSG (("Could not append bookmark object\n"));
			error = true;
		}
	}
	return error;
}

bool IE_Imp_MsWord_97::_insertBookmarkIfAppropriate(UT_uint32 iDocPosition)
{
	//now search for position iDocPosition in our bookmark list;
	bookmark * bm;
	if (m_iBookmarksCount == 0) {
		bm = static_cast<bookmark*>(NULL);
	}
	else {
		bm = static_cast<bookmark*>( bsearch(static_cast<const void *>(&iDocPosition),
				m_pBookmarks, m_iBookmarksCount, sizeof(bookmark),
				s_cmp_bookmarks_bsearch));
	}
	bool error = false;
	if(bm)
	{
	   // there is a bookmark at the current position
	   // first make sure the returned bookmark is the first one at this position
	   while(bm > m_pBookmarks && (bm - 1)->pos == iDocPosition)
		   bm--;

	   while(bm->pos == iDocPosition)
		  error |= _insertBookmark(bm++);
	}
	return error;
}

int IE_Imp_MsWord_97::_charProc (wvParseStruct *ps, U16 eachchar, U8 chartype, U16 lid)
{
	// make sure we are not past the end of the document ...
	// this can happen with some complex documents
	if(ps->currentcp >= m_iEndnotesEnd)
	{
		UT_DEBUGMSG(("IE_Imp_MsWord_97::_charProc: processing past end of document !!!\n"));
		return 0;
	}
	
	
	// reset the page break tracker
	if(m_bPageBreakPending)
	{
		// we have a page break pending, and being here means that it
		// was not a seciton break; we have to append it first and
		// then continue normal processing
		this->_appendChar (UCS_FF);
		m_bPageBreakPending = false;
	}
	
	if(!_handleHeadersText(ps->currentcp))
		return 0;
	if(!_handleNotesText(ps->currentcp))
		return 0;

	// insert any required bookmarks, but only if we are not in a
	// field ...
	if(!ps->fieldstate)
		_insertBookmarkIfAppropriate(ps->currentcp);

	if(_insertNoteIfAppropriate(ps->currentcp,eachchar))
		return 0;

	// convert incoming character to unicode
	if (chartype)
		eachchar = wvHandleCodePage(eachchar, lid);

	switch (eachchar)
	{

	case 11: // forced line break
		eachchar = UCS_LF;
		break;

	case 12: // page or section break
		this->_flush ();
		//eachchar = UCS_FF;
		// we will not append page breaks to the buffer, only mark it
		// as pending append; that will allow us later to decide if we
		// should or should not appended (we want to remove any page
		// break that is at an end of a section
		m_bPageBreakPending = true;
		return 0;

	case 13: // end of paragraph
		return 0;

	case 14: // column break
		eachchar = UCS_VTAB;
		break;

	case 19: // field begin
		this->_flush ();
		ps->fieldstate++;
		ps->fieldmiddle = 0;
		this->_fieldProc (ps, eachchar, chartype, lid);
		return 0;

	case 20: // field separator; some docs have spurious 0x14's in
			 // them, see bug 3745
		if (ps->fieldstate)
		{
			this->_fieldProc (ps, eachchar, chartype, lid);
			ps->fieldmiddle = 1;
		}
		return 0;

	case 21: // field end
		if (ps->fieldstate)
		{
			ps->fieldstate--;
			ps->fieldmiddle = 0;
			this->_fieldProc (ps, eachchar, chartype, lid);
		}
		return 0;
	}

	// i'm not sure if this is needed any more
	// yes, it is, for instance hyperlinks need it
	if (ps->fieldstate)
	{
		xxx_UT_DEBUGMSG(("DOM: fieldstate\n"));
		if(this->_fieldProc (ps, eachchar, chartype, lid))
		{
			return 0;
		}
	}

	// take care of any oddities in Microsoft's character encoding
	if (chartype == 1 && eachchar == 146)
		eachchar = 39; // apostrophe

	if(m_bSymbolFont)
	{
		eachchar &= 0x00ff;
	}
	
	this->_appendChar (static_cast<UT_UCSChar>(eachchar));

	return 0;
}

int IE_Imp_MsWord_97::_specCharProc (wvParseStruct *ps, U16 eachchar, CHP *achp)
{
	// make sure we are not past the end of the document ...
	// this can happen with some complex documents
	if(ps->currentcp >= m_iEndnotesEnd)
	{
		UT_DEBUGMSG(("IE_Imp_MsWord_97::_specCharProc: processing past end of document !!!\n"));
		return 0;
	}
	
	Blip blip;
	long pos;
	FSPA * fspa;
	FDOA * fdoa;
#ifdef SUPPORTS_OLD_IMAGES
	wvStream *fil;
	PICF picf;
#endif

	if(!_handleHeadersText(ps->currentcp))
		return 0;

	if(!_handleNotesText(ps->currentcp))
		return 0;

	// insert any required bookmarks, but only if we are not in a
	// field ...
	if(!ps->fieldstate)
		_insertBookmarkIfAppropriate(ps->currentcp);
	
	if(_insertNoteIfAppropriate(ps->currentcp,0))
		return 0;
	
	//
	// This next bit of code is to handle fields
	//

	switch (eachchar)
	{

	case 19: // field begin
		this->_flush ();
		ps->fieldstate++;
		ps->fieldmiddle = 0;
		this->_fieldProc (ps, eachchar, 0, 0x400);
		return 0;

	case 20: // field separator
		if (achp->fOle2)
		{
			UT_DEBUGMSG(("Field has an assocaited embedded OLE object\n"));
		}
		ps->fieldmiddle = 1;
		this->_fieldProc (ps, eachchar, 0, 0x400);
		return 0;

	case 21: // field end
		ps->fieldstate--;
		ps->fieldmiddle = 0;
		this->_fieldProc (ps, eachchar, 0, 0x400);
		return 0;

	}

	/* it seems some fields characters slip through here which tricks
	 * the import into thinking it has an image with it really does
	 * not. this catches special characters in a field
	 */
	if (ps->fieldstate) {
		if (this->_fieldProc(ps, eachchar, 0, 0x400))
			return 0;
	}

	//
	// This next bit of code is to handle OLE2 embedded objects and images
	//

	switch (eachchar)
	{
	case 0x01: // Older ( < Word97) image, currently not handled very well
		if (achp->fOle2) {
			UT_DEBUGMSG(("embedded OLE2 component. currently unsupported"));
			return 0;
		}

		pos = wvStream_tell(ps->data);

#ifdef SUPPORTS_OLD_IMAGES
		UT_DEBUGMSG(("Pre W97 Image format.\n"));
		wvStream_goto(ps->data, achp->fcPic_fcObj_lTagObj);

		if (1 == wvGetPICF(wvQuerySupported(&ps->fib, NULL), &picf,
				   ps->data) && NULL != picf.rgb)
		  {
			fil = picf.rgb;

			if (wv0x01(&blip, fil, picf.lcb - picf.cbHeader))
			  {
                       this->_handleImage(&blip, picf.mx * picf.dxaGoal / 1000, picf.my * picf.dyaGoal / 1000);
			  }
			else
			  {
			UT_DEBUGMSG(("Dom: no graphic data\n"));
			  }
#else
			UT_DEBUGMSG(("DOM: 0x01 graphics support is disabled at the moment\n"));
#endif

			wvStream_goto(ps->data, pos);

			return 0;
		  }
		else
		  {
			UT_DEBUGMSG(("Couldn't import graphic!\n"));
			return 0;
		  }
	case 0x08: // Word 97, 2000, XP image
		if (wvQuerySupported(&ps->fib, NULL) >= WORD8) // sanity check
		{
			if (ps->nooffspa > 0)
			{

				fspa = wvGetFSPAFromCP(ps->currentcp, ps->fspa,
									   ps->fspapos, ps->nooffspa);

				if(!fspa)
				{
					UT_DEBUGMSG(("No fspa! Panic and Insanity Abounds!\n"));
					return 0;
				}

				if (wv0x08(&blip, fspa->spid, ps))
				{
					this->_handleImage(&blip, fspa->xaRight-fspa->xaLeft,
									   fspa->yaBottom-fspa->yaTop);
				}
				else
				{
					UT_DEBUGMSG(("Dom: no graphic data!\n"));
					return 0;
				}
			}
			else
			{
				xxx_UT_DEBUGMSG(("nooffspa was <= 0 -- ignoring"));
			}
		}
		else
		{
			UT_DEBUGMSG(("pre Word8 0x08 graphic -- unsupported at the moment"));
			fdoa = wvGetFDOAFromCP(ps->currentcp, NULL, ps->fdoapos,
								   ps->nooffdoa);

			// TODO: do something with the data in this fdoa someday...
		}

		return 0;
	}

	return 0;
}

int IE_Imp_MsWord_97::_beginComment(wvParseStruct *ps, UT_uint32 tag,
					void *props, int dirty)
{
  UT_DEBUGMSG(("DOM: begin comment\n"));
  return 0;
}

int IE_Imp_MsWord_97::_endComment(wvParseStruct *ps, UT_uint32 tag,
				  void *props, int dirty)
{
  UT_DEBUGMSG(("DOM: begin comment\n"));
  return 0;
}


int IE_Imp_MsWord_97::_eleProc(wvParseStruct *ps, UT_uint32 tag,
							   void *props, int dirty)
{
	// make sure we are not past the end of the document ...
	// this can happen with some complex documents
	if(ps->currentcp >= m_iEndnotesEnd)
	{
		UT_DEBUGMSG(("IE_Imp_MsWord_97::_eleProc: processing past end of document !!!\n"));
		return 0;
	}
	
	//
	// Marshall these off to the correct handlers
	//

	switch (static_cast<wvTag>(tag))
	{

	case SECTIONBEGIN:
		return _beginSect (ps, tag, props, dirty);

	case SECTIONEND:
		return _endSect (ps, tag, props, dirty);

	case PARABEGIN:
		return _beginPara (ps, tag, props, dirty);

	case PARAEND:
		return _endPara (ps, tag, props, dirty);

	case CHARPROPBEGIN:
		return _beginChar (ps, tag, props, dirty);

	case CHARPROPEND:
		return _endChar (ps, tag, props, dirty);

	case COMMENTBEGIN:
	  return _beginComment (ps, tag, props, dirty);

	case COMMENTEND:
	  return _endComment (ps, tag, props, dirty);

	default:
	  UT_ASSERT_NOT_REACHED();

	}

	return 0;
}

/****************************************************************************/
/****************************************************************************/

int IE_Imp_MsWord_97::_beginSect (wvParseStruct *ps, UT_uint32 tag,
				  void *prop, int dirty)
{
	SEP * asep = static_cast <SEP *>(prop);

	const XML_Char * propsArray[15];  
	UT_String propBuffer;
	UT_String props;

	// flush any character runs
	this->_flush ();

	m_iCurrentSectId++;

	// first we need to deal with page size, because setting page size
	// resets all margins to the AW defaults
	// Sevior: Only do this ONCE!!! Abiword can only handle one page size.
	if(!m_bSetPageSize)
	{
		// all of this data is related to Abi's <pagesize> tag
		m_bSetPageSize = true;
		double page_width  = 0.0;
		double page_height = 0.0;
		double page_scale  = 1.0;

		if (asep->dmOrientPage == 1)
			getDoc()->m_docPageSize.setLandscape ();
		else
			getDoc()->m_docPageSize.setPortrait ();

		page_width = asep->xaPage / 1440.0;
		page_height = asep->yaPage / 1440.0;

		// PROBLEM: there are two separate and independent page sizes
		// given to us, one by the explicit width and height and one
		// by the requested paper size, and we need to decide which
		// one we should follow. There are three scenarios
		//   (1) the explicit size and paper match
		//   (2) the explicit size and paper do not match
		//       (a) the explicit size is the Word default (Letter)
		//       (b) the explicit size is something else than the defaults
		//
		// In case (1) we use the requested paper. Case (2a) happens
		// when the user changes the page size by requesting a
		// different paper size but does not touch the width and
		// height controls -- we use the paper size. Case (2b) happens
		// when the user changes size by the with and height controls;
		// the paper request stored is the one that was in place
		// before the manual adjustment and is no longer valid, so we
		// use the explicit width and height.

		// decide if the explicit width and height are valid, i.e., if
		// they contain the Word defaults the paper request has to be
		// 0 (Letter)
		bool bDoNotUseSize = (asep->xaPage == 12240 &&
							  asep->yaPage == 15840 &&
							  asep->dmPaperReq != 0);
		

		xxx_UT_DEBUGMSG(("DOM: pagesize: landscape: %d, width: %f, height: %f, paper-type: %d\n",
					 asep->dmOrientPage, page_width, page_height, asep->dmPaperReq));

		// map paper to AW page size name string ...
		const char * paper_name = s_mapPageIdToString (asep->dmPaperReq);

		// check if the paper name is valid (i.e., there is a match
		// between the name and the sizes; if not, we use only the sizes
		bool bPaperNameValid = (paper_name != NULL);
		
		if(bPaperNameValid)
		{
			// construct an instance of fp_PageSize for this paper
			// request; we will use this to verify whether its
			// dimensions match those stored in the explicit width and
			// height but also we will determine appropriate units to
			// be used (i.e., we want to use inches for Letter but
			// metric units for A4, etc.)
			fp_PageSize PageSize(paper_name);
			if(asep->dmOrientPage == 1)
				PageSize.setLandscape();

			// if we know that the explicit size is not valid, we do
			// not need any further checking
			if(!bDoNotUseSize)
			{
				// in order to minimize effect of rounding errors, we are
				// better doing the comparison in the twipses; the MS
				// values suffer from rounding (?) error which is quite
				// significant, so we will round to the second least
				// significant digit
			
				double w = PageSize.Width(DIM_IN) * 1440.0;
				double h = PageSize.Height(DIM_IN) * 1440.0;

				UT_uint32 iPaperW10 = ((UT_uint32) w)/10 + (((UT_uint32) w)%10 >= 5 ? 1 : 0);
				UT_uint32 iPaperH10 = ((UT_uint32) h)/10 + (((UT_uint32) h)%10 >= 5 ? 1 : 0);

				UT_uint32 iPageW10 = asep->xaPage/10 + (asep->xaPage%10 >= 5 ? 1 : 0);
				UT_uint32 iPageH10 = asep->yaPage/10 + (asep->yaPage%10 >= 5 ? 1 : 0);

				if(iPageW10 != iPaperW10 ||
				   iPageH10 != iPaperH10)
				{
					bPaperNameValid = false;
				}
			}

			// if we are to use the paper name, then get the
			// dimensions to be used ...
			if(bPaperNameValid)
			{
				m_dim = PageSize.getDims();
			}
		}
		
		if (bPaperNameValid)
		{
			getDoc()->m_docPageSize.Set (paper_name);
		}
		else
		{
			getDoc()->m_docPageSize.Set ("Custom");
			getDoc()->m_docPageSize.Set (page_width, page_height, DIM_IN);
			getDoc()->m_docPageSize.setScale(page_scale);
		}
	} // end of page size stuff

	if(asep->fBidi)
	{
		// this is an RTL section, set dominant direction to rtl
		props += "dom-dir:rtl;";
	}
	else
	{
		// this is an LTR section, we want to set the direction
		// explicitely so that we do not end up with wrong default
		props += "dom-dir:ltr;";
	}


	if(asep->fPgnRestart)
	{
		// set to 1 when page numbering should be restarted at the beginning of this section
		props += "section-restart:1;";
	}

	// user specified starting page number
	UT_String_sprintf(propBuffer, "section-restart-value:%d;", asep->pgnStart);
	props += propBuffer;

	// columns
	if (asep->ccolM1) {
		// number of columns
		UT_String_sprintf(propBuffer,"columns:%d;", (asep->ccolM1+1));
		props += propBuffer;

		// columns gap
		UT_String_sprintf(propBuffer,"column-gap:%s;",
			UT_convertInchesToDimensionString(m_dim,
											  (static_cast<float>(asep->dxaColumns) / 1440)));
		props += propBuffer;
	}

	// draw a vertical line between columns
	if (asep->fLBetween == 1)
	{
		props += "column-line:on;";
	}

	// space after section (gutter)
	UT_String_sprintf(propBuffer,"section-space-after:%s;",
			UT_convertInchesToDimensionString(m_dim,
											  (static_cast<float>(asep->dzaGutter) / 1440)));
	props += propBuffer;

	//
	// TODO: section breaks
	//

	// page-margin-left
	UT_String_sprintf(propBuffer, "page-margin-left:%s;",
			UT_convertInchesToDimensionString(m_dim,
											  (static_cast<float>(asep->dxaLeft) / 1440)));
	props += propBuffer;

	// page-margin-right
	UT_String_sprintf(propBuffer, "page-margin-right:%s;",
			UT_convertInchesToDimensionString(m_dim,
											  (static_cast<float>(asep->dxaRight) / 1440)));
	props += propBuffer;

	// page-margin-top
	UT_String_sprintf(propBuffer, "page-margin-top:%s;",
			UT_convertInchesToDimensionString(m_dim,
											  (static_cast<float>(asep->dyaTop) / 1440)));
	props += propBuffer;

	// page-margin-bottom
	UT_String_sprintf(propBuffer, "page-margin-bottom:%s;",
			UT_convertInchesToDimensionString(m_dim,
											  (static_cast<float>(asep->dyaBottom)/1440)));
	props += propBuffer;

	// page-margin-header
	UT_String_sprintf(propBuffer, "page-margin-header:%s;",
			UT_convertInchesToDimensionString(m_dim,
											  (static_cast<float>(asep->dyaHdrTop)/1440)));
	props += propBuffer;

	// page-margin-footer (word's footer is measured from the bottom
	// edge of the page -- contrary to the docs -- our's from the
	// bottom margin of the page)
	UT_String_sprintf(propBuffer, "page-margin-footer:%s",
			UT_convertInchesToDimensionString(m_dim,
							(static_cast<float>(asep->dyaBottom - asep->dyaHdrBottom)/1440)));
	
	props += propBuffer;

	xxx_UT_DEBUGMSG (("DOM:SEVIOR the section properties are: '%s'\n", props.c_str()));

	
	propsArray[0] = static_cast<const XML_Char *>("props");
	propsArray[1] = static_cast<const XML_Char *>(props.c_str());

	UT_uint32 iOff = 2;
	
	// headers/footers
	UT_String id[6];
	UT_uint32 iId = 0;

	// see _handleHeaders() on the contents of the m_pHeaders array,
	// it will make this maths clear (m_iCurrentSectId is 1-based
	// indx)
	// For each section in the document they are six headers/footers;
	// each of these can be in 3 states:
	//      length  > 2: proper header, use it
	//      length == 2: empty header; no header to be inserted
	//      length == 0: use header from the previous section

	if ((m_iCurrentSectId - 1)*6 + 6 < m_iHeadersCount)
	{
		// there are headers defined for this section
		UT_uint32 i = 6 + (m_iCurrentSectId - 1)*6;
		UT_uint32 j = i + 6;
		UT_sint32 k;

		for( ; i < j && i < m_iHeadersCount; i++)
		{
			// skip any unsupported or empty headers
			if(m_pHeaders[i].type == HF_Unsupported || m_pHeaders[i].len == 2)
			{
				continue;
			}

			k = i;
#if 0
			// For now this code is going to be disabled, since a
			// present AW sections cannot share headers, and this type
			// of a header needs to be replaced by a physical copy of
			// the previous meaningul header
			if(m_pHeaders[i].len == 0)
			{
				// this is the case where the section is to use the
				// header of a previous section -- scroll back until
				// we find one
				k -= 6;
				bool bContinue = false;
				
				while(k > 5)
				{
					if(m_pHeaders[k].len == 2)
					{
						// found empty header 
						bContinue = true;
						break;
					}
					else if(m_pHeaders[k].len == 0)
					{
						// try one section ahead
						k -= 6;
					}
					else
					{
						// found a meaningful header
						break;
					}
				}

				if(bContinue || k < 6)
				{
					continue;
				}
			}
#endif
			switch(m_pHeaders[k].type)
			{
				case HF_HeaderEven:
					propsArray[iOff++] = "header-even";
					break;
				case HF_FooterEven:
					propsArray[iOff++] = "footer-even";
					break;
				case HF_HeaderOdd:
					propsArray[iOff++] = "header";
					break;
				case HF_FooterOdd:
					propsArray[iOff++] = "footer";
					break;
				case HF_HeaderFirst:
					propsArray[iOff++] = "header-first";
					break;
				case HF_FooterFirst:
					propsArray[iOff++] = "footer-first";
					break;
				default:
					UT_ASSERT(UT_NOT_REACHED);
			}

			UT_String_sprintf(id[iId],"%d",m_pHeaders[k].pid);
			propsArray[iOff++] = id[iId++].c_str();
		}
	}
	
	propsArray[iOff++] = 0;
	UT_ASSERT(iOff <= sizeof(propsArray));
	

	if (!_appendStrux(PTX_Section, static_cast<const XML_Char **>(&propsArray[0])))
	{
		UT_DEBUGMSG (("DOM: error appending section props!\n"));
		return 1;
	}

	// increment our section count
	m_bInSect = true;
	m_bInPara = false; // reset paragraph status
	m_nSections++;

	// TODO: we need to do some work on Headers/Footers

	/*
	 * break codes:
	 * 0 No break
	 * 1 New column
	 * 2 New page
	 * 3 Even page
	 * 4 Odd page
	 */

	//	if (asep->bkc > 1 && m_nSections > 1) // don't apply on the 1st page
	if (m_nSections > 1) // don't apply on the 1st page
	{
		// new sections always need a block
		if (!_appendStrux(PTX_Block, static_cast<const XML_Char **>(NULL)))
		{
			UT_DEBUGMSG (("DOM: error appending new block\n"));
			return 1;
		}
		m_bInPara = true;

		UT_UCSChar ucs = UCS_FF;
		switch (asep->bkc) {
			case 1:
				ucs = UCS_VTAB;
				X_CheckError(_appendSpan(&ucs,1));
				break;

			case 2:
				X_CheckError(_appendSpan(&ucs,1));
				break;

			case 3: // TODO: handle me better (not even)
				X_CheckError(_appendSpan(&ucs,1));
				break;

			case 4: // TODO: handle me better (not odd)
				X_CheckError(_appendSpan(&ucs,1));
				break;

			case 0:
			default:
				break;
		}
	}

	return 0;
}

// this function is called from _handleHeadersText() with meaningless
// parameters; if you want to make use of any of the parameters here,
// make sure it will work with NULLs, etc.
int IE_Imp_MsWord_97::_endSect (wvParseStruct * /* ps */ , UT_uint32  /* tag */ ,
								void * /* prop */, int /* dirty */ )
{
#if 0
	// if we're at the end of a section, we need to check for a section mark
	// at the end of our character stream and remove it (to prevent page breaks
	// between sections)

	// this does not work -- if we are at the end of a section we have
	// already flushed the buffer in _endPara()
	if (m_pTextRun.size() &&
		m_pTextRun[m_pTextRun.size()-1] == UCS_FF)
	  {
		m_pTextRun[m_pTextRun.size()-1] = 0;
	  }
#endif

	// if there is a pending page break it belongs to the section and
	// is to be removed, we just need to set the tracker to false
	m_bPageBreakPending = false;

	m_bInSect = false;
	m_bInPara = false; // reset paragraph status
	return 0;
}

int IE_Imp_MsWord_97::_beginPara (wvParseStruct *ps, UT_uint32 tag,
				  void *prop, int dirty)
{

	// if in a header of unsupported type, just return
	if(m_bInHeaders &&
	   ((m_iCurrentHeader < m_iHeadersCount && m_pHeaders &&
		 m_pHeaders[m_iCurrentHeader].type == HF_Unsupported)))
		return 0;
	
	PAP *apap = static_cast <PAP *>(prop);

	// the header/footnote/endnote sections are special; because the
	// parser treats them as a continuation of the document, we end up
	// here before we get chance to handle the change from main doc to
	// these sections -- we want the paragraph properties assembled
	// for future use, but we do not want the strux actually inserted
	bool bDoNotInsertStrux = (ps->currentcp == m_iFootnotesStart ||
							  ps->currentcp == m_iEndnotesStart  ||
							  ps->currentcp == m_iHeadersStart);

	// the end of endnotes/fnotes/headers and all other subsections in
	// the main stream always contains a paragraph marker; we do not
	// want it to insert strux on those
	if((ps->currentcp == m_iTextEnd - 1 && m_iTextEnd > m_iTextStart)                ||
	   //(ps->currentcp == m_iTextEnd - 2 && m_iTextEnd > m_iTextStart)                ||
	   (ps->currentcp == m_iFootnotesEnd - 1 && m_iFootnotesEnd > m_iFootnotesStart) ||
	   (ps->currentcp == m_iEndnotesEnd - 1  && m_iEndnotesEnd > m_iEndnotesStart)   ||
	   (ps->currentcp == m_iHeadersEnd - 1 && m_iHeadersEnd > m_iHeadersStart)       ||
	   (ps->currentcp == m_iAnnotationsEnd - 1 && m_iAnnotationsEnd > m_iAnnotationsStart) ||
	   (ps->currentcp == m_iMacrosStart - 1 && m_iMacrosEnd > m_iMacrosStart))
	{
		bDoNotInsertStrux  = true;
	}
	

	// at the end of each f/enote is a superflous paragraph marker
	// which we do not want imported
	if(m_bInFNotes && m_iNextFNote < m_iFootnotesCount && m_pFootnotes &&
	   m_pFootnotes[m_iNextFNote].txt_pos + m_pFootnotes[m_iNextFNote].txt_len - 1 >= ps->currentcp)
	{
		bDoNotInsertStrux = true;
	}
	
	if(m_bInENotes && m_iNextENote < m_iEndnotesCount && m_pEndnotes &&
	   m_pEndnotes[m_iNextENote].txt_pos + m_pEndnotes[m_iNextENote].txt_len - 1 >= ps->currentcp)
	{
		bDoNotInsertStrux = true;
	}
	 
	// the header section requires even more special care; since we
	// need to insert the HdrFtr strux for each header before we can
	// insert the block, we do not want a strux inserted at the start
	// position of a header; furthermore, each header ends with a
	// superfluous paragraph marker
	if(m_bInHeaders &&
	   ((m_iCurrentHeader < m_iHeadersCount && m_pHeaders &&
	   (m_pHeaders[m_iCurrentHeader].pos == ps->currentcp ||
		m_pHeaders[m_iCurrentHeader].pos + m_pHeaders[m_iCurrentHeader].len - 1 <= ps->currentcp))
		|| m_iCurrentHeader == m_iHeadersCount))
	{
		//start a new header section
		bDoNotInsertStrux = true;
	}

	{
	  if (apap->fInTable) {
	    if (!m_bInTable) {
	      m_bInTable = true;
	        _table_open();
	    }

	    if (ps->endcell) {
	      ps->endcell = 0;
	      _cell_close();
	      if (m_iCellsRemaining > 0) {
		m_iCellsRemaining--;
		if (m_iCellsRemaining == 0) {
		  _row_close();
		}
	      }
	    }

	    _row_open();

	    // determine column spans
	    if (!m_bCellOpen) {
	      m_vecColumnSpansForCurrentRow.clear();

	      for (int column = 1; column < ps->nocellbounds; column++) {
		int span = 0;

		for (int i = column; i < ps->nocellbounds; i++) {
		  if (ps->cellbounds[i] >= apap->ptap.rgdxaCenter[column]) {
		    span = (i - column);
		    break;
		  }
		}
		m_vecColumnSpansForCurrentRow.addItem(reinterpret_cast<void *>(span));
	      }
	    }

	    _cell_open(ps, apap);

	    if (m_iCellsRemaining == 0) {
	      m_iCellsRemaining = apap->ptap.itcMac + 1;
	    }

	    if (m_iRowsRemaining == 0) {
	      m_iRowsRemaining = ps->norows;
	    }

	    m_iRowsRemaining--;
	  }
	  else if (m_bInTable) {
	    m_bInTable = false;
	    _table_close(ps, apap);
	  }
	}


	// first, flush any character data in any open runs
	// only flush if we are really inserting the strux (so that we can
	// remove any superfluous characters at ends of secitons,
	// e.g. page breaks)
	if(!bDoNotInsertStrux)
	{
		this->_flush ();
	}
	
	if (apap->fTtp)
	  {
	    m_bInPara = true;
	    return 0;
	  }

	if (apap->fBidi == 1)
	{
		m_bLTRParaContext = false;
	} else
	{
		m_bLTRParaContext = true;
	}

	m_bBidiMode = false;

	// break before paragraph?
	if (apap->fPageBreakBefore)
	{
		// TODO: this should really set a property in
		// TODO: in the paragraph, instead; but this
		// TODO: gives a similar effect for now.
		// TOOD: when it is handled properly the code needs to be
		// moved into _generateParaProps()
		UT_DEBUGMSG(("_beginPara: appending default block\n"));
		_appendStrux(PTX_Block, NULL);
		UT_UCSChar ucs = UCS_FF;
		_appendSpan(&ucs,1);
	}

	m_charProps.clear();
	m_charStyle.clear();
	m_paraProps.clear();
	m_paraStyle.clear();
	_generateParaProps(m_paraProps, apap, ps);
	
	//props, level, listid, parentid, style, NULL
	const XML_Char * propsArray[11];

	/* lists */
	UT_uint32 myListId = 0;
	UT_uint32 iAWListId = UT_UID_INVALID;
	UT_String szListId, szParentId, szLevel, szStartValue, szNumberProps;
	
	// all lists have ilfo set; some lists can be 'customised' by
	// having the number field removed (see bug 3622) -- they are
	// still lists in Word, but do not look like it, and we will not
	// treat them as lists (Tomas, May 26, 2003)
	if(apap->ilfo && apap->linfo.numberstr)
	{
		UT_uint32 j;
		// if we are in a new list, then do some clean up first and remember the list id
		if(m_iMSWordListId != apap->linfo.id)
		{
			m_iMSWordListId = apap->linfo.id;

			for(UT_uint32 i = 0; i < 9; i++)
				m_iListIdIncrement[i] = 0;

			UT_VECTOR_PURGEALL(ListIdLevelPair *, m_vLists);
			m_vLists.clear();
		}

		// a hack -- see the note on myListId below
		myListId = apap->linfo.id;
		myListId += apap->linfo.format;
		myListId += apap->ilvl;

		/*
		  IMPORTANT the list sutff is found in several different
		  places:

		  apap->ilvl - the level of this list (0-8)

		  myListId - the id of this list, we need this to know to which list this
		  paragraph belongs; unfortunately, there seem to be some cases where separate
		  lists *share* the same id, for instance when two lists, of different formatting,
		  are separated by only empty paragraphs. As a hack, I have added the format number
		  to the list id, so gaining different id for different formattings (it is not foolproof,
		  for if id1 + format1 == id2 + format2 then we get two lists joined, but the probability
		  of that should be small). Further problem is that in AW, list id refers to the set of
		  list elements on the same level, while in Word the id is that of the entire list. The
		  easiest way to tranform the Word id to AW id is to add the level to the id, which
		  is what has been done above

		  apap->linfo.start - the stating number of this entire list;

		  apap->linfo.numberstr - the actual number string to display (XCHAR *); we probably need
		  this to work out the number separator, since there does not seem
		  to be any reference to this anywhere

		  apap->linfo.numberstr_size - length of the number string
		  
		  apap->linfo.format - number format (see the enum below)

		  apap->linfo.align	- number alignment [0: lft, 1: rght, 2: cntr]
		  
		  apap->linfo.ixchFollow - what character stands between the number and the para
		  [0:= tab, 1: spc, 2: none]
		*/

		// If a given list id has already been defined, appending a new list with
		// same values will have a harmless effect


		// we will use this to keep track of how many entries of given level we have had
		// every time we get here, we increase the counter for all levels lower than ours
		// then we will add the counter for our level to myListId; this way subsections of
		// the list separated by a higher level list entry will have different id's


		for(j = apap->ilvl + 1; j < 9; j++)
			m_iListIdIncrement[j]++;

		myListId += m_iListIdIncrement[apap->ilvl];

		// see if this id is already in our map
		for(j = 0; j < m_vListIdMap.getItemCount(); j+=2)
		{
			if((UT_uint32)m_vListIdMap.getNthItem(j) == myListId)
			{
				iAWListId = m_vListIdMap.getNthItem(j+1);
				break;
			}
		}
		
		if(iAWListId == UT_UID_INVALID)
		{
			iAWListId = getDoc()->getUID(UT_UniqueId::List);
			UT_ASSERT(iAWListId != UT_UID_INVALID);

			m_vListIdMap.addItem(myListId);
			m_vListIdMap.addItem(iAWListId);
		}
		
		
		const XML_Char * list_atts[15];
		UT_uint32 iOffset = 0;
		UT_String propBuffer;
		
		// list id number
		list_atts[iOffset++] = "id";
		UT_String_sprintf(propBuffer, "%d", iAWListId);
		szListId = propBuffer;
		list_atts[iOffset++] = szListId.c_str();


		// parent id
		list_atts[iOffset++] = "parentid";

		// we will search backward our list vector for the first entry
		// that has a lower level than we and that will be our parent
		UT_uint32 myParentID = 0;
		for(UT_sint32 n = m_vLists.getItemCount(); n > 0; n--)
		{
			ListIdLevelPair * llp = (ListIdLevelPair *)(m_vLists.getNthItem(n - 1));
			if(llp->level < apap->ilvl)
			{
				myParentID = llp->listId;
				break;
			}
		}
		UT_String_sprintf(propBuffer, "%d", myParentID);
		szParentId = propBuffer;
		list_atts[iOffset++] = szParentId.c_str();

		// list type
		list_atts[iOffset++] = "type";
		list_atts[iOffset++] = s_mapDocToAbiListId (static_cast<MSWordListIdType>(apap->linfo.format));

		// start value
		list_atts[iOffset++] = "start-value";
		UT_String_sprintf(propBuffer, "%d", apap->linfo.start);
		szStartValue = propBuffer;
		list_atts[iOffset++] = szStartValue.c_str();

		// list delimiter
		UT_String sDelim;
		s_mapDocToAbiListDelim (apap->linfo.numberstr,apap->linfo.numberstr_size,sDelim);
		list_atts[iOffset++] = "list-delim";
		list_atts[iOffset++] = sDelim.c_str();

		list_atts[iOffset++] = "level";
		UT_String_sprintf(propBuffer, "%d", apap->ilvl + 1); // Word level starts at 0, Abi's at 1
		szLevel = propBuffer;
		list_atts[iOffset++] = szLevel.c_str();

		// generate character props for the number
		// TODO -- the properties represented by apap->linfo.chp need
		// to be applied to the list number/bulet. For now, I am going
		// to translate these into a regular props string and attach
		// them to the list attributes, but they need to be passed
		// somehow down to the number field (may need a dedicated
		// _generateListCharProps() for this
		// Tomas, May 12, 2003
		_generateCharProps(szNumberProps, &apap->linfo.chp, ps);
		list_atts[iOffset++] = "props";
		list_atts[iOffset++] = szNumberProps.c_str();
		
		// NULL
		list_atts[iOffset++] = 0;
		UT_ASSERT( iOffset <=  sizeof(list_atts)/sizeof(XML_Char *) );

		// now add this to our vector of lists
		ListIdLevelPair * llp = new ListIdLevelPair;
		llp->listId = iAWListId;
		llp->level = apap->ilvl;
		m_vLists.addItem(static_cast<void*>(llp));

		getDoc()->appendList(list_atts);
		UT_DEBUGMSG(("DOM: appended a list\n"));

		// TODO: merge in list properties and such here with the variable 'props',
		// such as list-style, field-font, ...

		// start-value
		// Need to put the ";" back in the para string.
		//
		m_paraProps[m_paraProps.size() - 1] = ';';
		m_paraProps += "start-value:";
		m_paraProps += szStartValue;
		m_paraProps += ";";

		// list style
		m_paraProps += "list-style:";
		m_paraProps += s_mapDocToAbiListStyle (static_cast<MSWordListIdType>(apap->linfo.format));
		m_paraProps += ";";

		// field-font
		m_paraProps += "field-font:";
		m_paraProps += s_fieldFontForListStyle (static_cast<MSWordListIdType>(apap->linfo.format));
	} // end of list-related code

 	// props
	UT_uint32 i = 0;
	propsArray[i++] = static_cast<const XML_Char *>("props");
	propsArray[i++] = static_cast<const XML_Char *>(m_paraProps.c_str());

	
	// level, or 0 for default, normal level
	if (myListId > 0)
	{
		propsArray[i++] = "level";
		propsArray[i++] = szLevel.c_str();
		propsArray[i++] = "listid";
		propsArray[i++] = szListId.c_str();
		propsArray[i++] = "parentid";
		propsArray[i++] = szParentId.c_str();
	}

	// handle style
	// TODO from wv we get the style props expanded and applied to the
	// characters in the paragraph (i.e., part of the CHP structure);
	// we need to be able to tell to wv not to do this expansion
	if(apap->stylename[0])
	{
		const STD * pSTD = ps->stsh.std;
		UT_uint32 iCount = ps->stsh.Stshi.cstd;

		if(apap->istd != istdNil && apap->istd < iCount)
		{
			propsArray[i++] = "style";
			
			char * t = NULL;
			const XML_Char * pName = s_translateStyleId(pSTD[apap->istd].sti);
		
			if(pName)
			{
				m_paraStyle = pName;
			}
			else
			{
				m_paraStyle = t = s_stripDangerousChars(pSTD[apap->istd].xstzName);
			}

			FREEP(t);
			propsArray[i++] = m_paraStyle.c_str();
		}
		
	}

	// NULL
	propsArray[i] = 0;

	if (!m_bInSect && !bDoNotInsertStrux)
	{
		// check for should-be-impossible case
		UT_ASSERT_NOT_REACHED();
		_appendStrux(PTX_Section, NULL);
		m_bInSect = true ;
	}

	if(!bDoNotInsertStrux)
	{
		xxx_UT_DEBUGMSG(("_beginPara: pos %d [text ends %d]\n", ps->currentcp, m_iFootnotesStart));
		
		if (!_appendStrux(PTX_Block, static_cast<const XML_Char **>(&propsArray[0])))
		{
			UT_DEBUGMSG(("DOM: error appending paragraph block\n"));
			return 1;
		}
	}
	
	if (myListId > 0 && !bDoNotInsertStrux)
	  {
		// TODO: honor more props
		const XML_Char *list_field_fmt[3];
		list_field_fmt[0] = "type";
		list_field_fmt[1] = "list_label";
		list_field_fmt[2] = 0;

		_appendObject(PTO_Field, static_cast<const XML_Char**>(&list_field_fmt[0]));

		// the character following the list label - 0=tab, 1=space, 2=none
		if(apap->linfo.ixchFollow == 0) // tab
		{
			UT_UCSChar tab = UCS_TAB;
			_appendSpan(&tab, 1);
		}
		else if(apap->linfo.ixchFollow == 1) // space
		{
			UT_UCSChar space = UCS_SPACE;
			_appendSpan(&space, 1);
		}
		// else none
	  }
	m_bInPara = true;
	return 0;
}

int IE_Imp_MsWord_97::_endPara (wvParseStruct *ps, UT_uint32 tag,
								void *prop, int dirty)
{
	xxx_UT_DEBUGMSG(("#DOM: _endPara\n"));
	// have to flush here, otherwise flushing later on will result in
	// an empty paragraph being inserted

	this->_flush ();
	m_bInPara = false;
	
	return 0;
}

int IE_Imp_MsWord_97::_beginChar (wvParseStruct *ps, UT_uint32 tag,
								  void *prop, int dirty)
{
	// if in a header of unsupported type, just return
	if(m_bInHeaders &&
	   ((m_iCurrentHeader < m_iHeadersCount && m_pHeaders &&
		 m_pHeaders[m_iCurrentHeader].type == HF_Unsupported)))
		return 0;
	
	// the header/footnote/endnote sections are special; because the
	// parser treats them as a continuation of the document, we end up
	// here before we get chance to handle the change from main doc to
	// these sections -- we want the char properties assembled
	// for future use, but we do not want them actually appended
	bool bDoNotAppendFmt = (ps->currentcp == m_iFootnotesStart ||
							  ps->currentcp == m_iEndnotesStart  ||
							  ps->currentcp == m_iHeadersStart);

	// the end of endnotes/fnotes/headers and all other subsections in
	// the main stream always contain a paragraph marker; we do not
	// want it to append fmt on those
	if((ps->currentcp == m_iTextEnd - 1 && m_iTextEnd > m_iTextStart)                ||
	   (ps->currentcp == m_iTextEnd - 2 && m_iTextEnd > m_iTextStart)                ||
	   (ps->currentcp == m_iFootnotesEnd - 1 && m_iFootnotesEnd > m_iFootnotesStart) ||
	   (ps->currentcp == m_iEndnotesEnd - 1  && m_iEndnotesEnd > m_iEndnotesStart)   ||
	   (ps->currentcp == m_iHeadersEnd - 1 && m_iHeadersEnd > m_iHeadersStart)       ||
	   (ps->currentcp == m_iAnnotationsEnd - 1 && m_iAnnotationsEnd > m_iAnnotationsStart) ||
	   (ps->currentcp == m_iMacrosStart - 1 && m_iMacrosEnd > m_iMacrosStart))
	{
		bDoNotAppendFmt  = true;
	}
	

	// at the end of each f/enote is a superflous paragraph marker
	// which we do not want imported
	if(m_bInFNotes && m_iNextFNote < m_iFootnotesCount && m_pFootnotes &&
	   m_pFootnotes[m_iNextFNote].txt_pos + m_pFootnotes[m_iNextFNote].txt_len - 1 >= ps->currentcp)
	{
		bDoNotAppendFmt = true;
	}
	
	if(m_bInENotes && m_iNextENote < m_iEndnotesCount && m_pEndnotes &&
	   m_pEndnotes[m_iNextENote].txt_pos + m_pEndnotes[m_iNextENote].txt_len - 1 >= ps->currentcp)
	{
		bDoNotAppendFmt = true;
	}
	 
	// the header section requires even more special care; since we
	// need to insert the HdrFtr strux for each header before we can
	// insert the block, we do not want a strux and fmt inserted at the start
	// position of a header; furthermore, each header ends with a
	// superfluous paragraph marker
	if(m_bInHeaders &&
	   ((m_iCurrentHeader < m_iHeadersCount && m_pHeaders &&
	   (m_pHeaders[m_iCurrentHeader].pos == ps->currentcp ||
		m_pHeaders[m_iCurrentHeader].pos + m_pHeaders[m_iCurrentHeader].len - 1 <= ps->currentcp))
	   || m_iCurrentHeader == m_iHeadersCount))
	{
		//start a new header section
		bDoNotAppendFmt = true;
	}

	// flush any data in our character runs
	// if we are not really appending, then do not flush, so that we
	// are not prevented from removing superflous page breaks at the
	// end of section
	if(!bDoNotAppendFmt)
	{
		this->_flush ();
	}
	

	CHP *achp = static_cast <CHP *>(prop);

	const XML_Char * propsArray[7];
	UT_uint32 propsOffset = 0;

	m_charProps.clear();
	m_charStyle.clear();

	if(ps->fonts.ffn[achp->ftcAscii].chs == 0)
		m_bSymbolFont = false;
	else if(ps->fonts.ffn[achp->ftcAscii].chs == 2)
		m_bSymbolFont = true;
	else
	{
		UT_DEBUGMSG(("IE_Imp_MsWord_97::_beginChar: unknow font encoding %d\n",
					 ps->fonts.ffn[achp->ftcAscii].chs));
		m_bSymbolFont = false;
	}
	
	memset (propsArray, 0, sizeof(propsArray));

	_generateCharProps(m_charProps, achp, ps);

	if (!achp->fBidi)
		m_bLTRCharContext = true;
	else
		m_bLTRCharContext = false;

	// we enter bidi mode if we encounter a character
	// formatting inconsistent with the base direction of the
	// paragraph; once in bidi mode, we have to stay there
	// until the end of the current pragraph
	m_bBidiMode = m_bBidiMode || (m_bLTRCharContext ^ m_bLTRParaContext);

	propsArray[propsOffset++] = static_cast<const XML_Char *>("props");
	propsArray[propsOffset++] = static_cast<const XML_Char *>(m_charProps.c_str());

	if(!m_bEncounteredRevision && (achp->fRMark || achp->fRMarkDel))
	{
		// revision "hack" - add a single revision for all revisioned text
		UT_UCS4String revisionStr ("msword_revisioned_text");
		getDoc()->addRevision(1, revisionStr.ucs4_str(), revisionStr.size());
		m_bEncounteredRevision = true;
	}

	if (achp->fRMark)
	{
	    propsArray[propsOffset++] = static_cast<XML_Char *>("revision");
		m_charRevs = "1";
	    propsArray[propsOffset++] = m_charRevs.c_str();
	}
	else if (achp->fRMarkDel)
	{
	    propsArray[propsOffset++] = static_cast<XML_Char *>("revision");
		m_charRevs = "-1";
	    propsArray[propsOffset++] = m_charRevs.c_str();
	}
	else
		m_charRevs.clear();
	
	
	if(achp->stylename[0])
	{
		const STD * pSTD = ps->stsh.std;
		UT_uint32 iCount = ps->stsh.Stshi.cstd;
		
		if(achp->istd != istdNil && achp->istd < iCount)
		{
			propsArray[propsOffset++] = static_cast<XML_Char *>("style");
			char * t = NULL;
			const XML_Char * pName = s_translateStyleId(pSTD[achp->istd].sti);
		
			if(pName)
			{
				m_charStyle = pName;
			}
			else
			{
				m_charStyle = t = s_stripDangerousChars(pSTD[achp->istd].xstzName);
			}

			FREEP(t);
			propsArray[propsOffset++] = m_charStyle.c_str();
		}
	}

	// woah - major error here
	if(!m_bInSect && !bDoNotAppendFmt)
	{
		UT_ASSERT_NOT_REACHED();
		_appendStrux(PTX_Section, NULL);
		m_bInSect = true ;
	}

	if(!m_bInPara && !bDoNotAppendFmt)
	{
		UT_ASSERT_NOT_REACHED();
		_appendStrux(PTX_Block, NULL);
		m_bInPara = true ;
	}

	if(!bDoNotAppendFmt)
	{
		if (!_appendFmt(static_cast<const XML_Char **>(&propsArray[0])))
		{
			UT_DEBUGMSG(("DOM: error appending character formatting\n"));
			return 1;
		}
	}
	
	return 0;
}

int IE_Imp_MsWord_97::_endChar (wvParseStruct *ps, UT_uint32 tag,
								void *prop, int dirty)
{
	// nothing is needed here
	return 0;
}

/****************************************************************************/
/****************************************************************************/

int IE_Imp_MsWord_97::_fieldProc (wvParseStruct *ps, U16 eachchar,
								  U8 chartype, U16 lid)
{
	xxx_UT_DEBUGMSG(("DOM: fieldProc: %c %x\n", static_cast<char>(eachchar),
					 static_cast<int>(eachchar)));

	//
	// The majority of this code has just been ripped out of wv/field.c
	//
	field * f = NULL;
	UT_sint32 iRet = 1;
	
	if (eachchar == 0x13) // beginning of a field
	{
		if(m_stackField.getDepth() > 0)
		{
			// see what kind of field we are in
			m_stackField.viewTop((void**)&f);
			UT_return_val_if_fail(f,0);

			switch(f->type)
			{
				case F_TOC:
				case F_TOC_FROM_RANGE:
				case F_HYPERLINK:
					// for these fields we want to dump into the
					// document anything in the argument
					{
						f->argument[f->fieldI] = 0;
						UT_UCS2Char * a = f->argument;

						if(*a == 0x14)
						{
							a++;
						}
						
						while(*a)
						{
							this->_appendChar(*a++);
						}
						this->_flush();

						f->argument[0] = 0;
						f->fieldI = 0;
					}
					break;
					
				default:
					break;
			}
			
		}
		
		f = new field;
		UT_return_val_if_fail(f,0);
		f->fieldWhich = f->command;
		f->command[0] = 0;
		f->argument[0] = 0;
		f->fieldI = 0;
		f->fieldRet = 1;
		m_stackField.push((void*)f);
	}
	else if (eachchar == 0x14) // field trigger
	{
		m_stackField.viewTop((void**)&f);
		UT_return_val_if_fail(f,0);
		
		f->command[f->fieldI] = 0;
		f->fieldC = wvWideStrToMB (f->command);

		if (this->_handleCommandField(f->fieldC))
			f->fieldRet = 1;
		else
			f->fieldRet = 0;

		wvFree(f->fieldC);
		f->fieldWhich = f->argument;
		f->fieldI = 0;
	}
	if(!f)
	{
		m_stackField.viewTop((void**)&f);
	}
	
	UT_return_val_if_fail(f,0);
	
	if (f->fieldI >= FLD_SIZE)
	{
		UT_DEBUGMSG(("DOM: Something completely absurd in the fields implementation!\n"));
		UT_ASSERT_NOT_REACHED();
		return 1;
	}

	if (!f->fieldWhich) {
		UT_DEBUGMSG(("DOM: _fieldProc - 'which' is null\n"));
		UT_ASSERT_NOT_REACHED();
		return 1;
	}

	if (chartype)
		f->fieldWhich[f->fieldI] = wvHandleCodePage(eachchar, lid);
	else
		f->fieldWhich[f->fieldI] = eachchar;

	f->fieldI++;

	if (eachchar == 0x15) // end of field marker
	{
		f->fieldWhich[f->fieldI] = 0;
		//I do not think we should convert this -- this is the field value
		//displayed in the document; in most cases we do not need it, as we
		//calulate it ourselves, but for instance for hyperlinks this is the
		//the text to which the link is tied
		//m_fieldA = wvWideStrToMB (m_argument);
		f->fieldC = wvWideStrToMB (f->command);
		_handleFieldEnd (f->fieldC, ps->currentcp);
		wvFree (f->fieldC);
		iRet = f->fieldRet;
		
		m_stackField.pop((void**)&f);
		UT_return_val_if_fail(f,0);
		delete f;
	}
	return iRet;
}

bool IE_Imp_MsWord_97::_handleFieldEnd (char *command, UT_uint32 iDocPosition)
{
	Doc_Field_t tokenIndex = F_OTHER;
	char *token;
	field * f = NULL;
	m_stackField.viewTop((void**)&f);
	UT_return_val_if_fail(f, true);
	
	if (*command != 0x13)
	{
		UT_DEBUGMSG (("field did not begin with 0x13\n"));
		return true;
	}

	command++;
	token = strtok (command, "\t, ");
	
	while(token)
	{
		tokenIndex = s_mapNameToField (token);
		switch (tokenIndex)
		{
			case F_HYPERLINK:
				{
					token = strtok (NULL, "\"\" ");
					UT_return_val_if_fail(f->argument[f->fieldI - 1] == 0x15, false);
					
					f->argument[f->fieldI - 1] = 0;
					UT_UCS2Char * a = f->argument;

					if(*a == 0x14)
					{
						a++;
					}
					
					while(*a)
					{
						this->_appendChar(*a++);
					}
					this->_flush();

					if(!m_bInPara)
					{
						_appendStrux(PTX_Block, NULL);
						m_bInPara = true ;
					}

					_appendObject(PTO_Hyperlink,NULL);
					break;
				}
			case F_TOC:             // for the toc fields we will
			case F_TOC_FROM_RANGE:  // insert the field result for now
				{
					token = strtok (NULL, "\"\" ");
					UT_return_val_if_fail(f->argument[f->fieldI - 1] == 0x15, false);
					
					f->argument[f->fieldI - 1] = 0;
					UT_UCS2Char * a = f->argument;

					if(*a == 0x14)
					{
						a++;
					}

					while(*a)
					{
						this->_appendChar(*a++);
					}
					this->_flush();
				}
				break;

			default:
				break;
		}

		token = strtok (NULL, "\t, ");
	}
	return false;
}

bool IE_Imp_MsWord_97::_handleCommandField (char *command)
{
	Doc_Field_t tokenIndex = F_OTHER;
	char *token = NULL;
	field * f = NULL;
	m_stackField.viewTop((void**)&f);
	UT_return_val_if_fail(f,true);
	bool bTypeSet = false;
	
	xxx_UT_DEBUGMSG(("DOM: handleCommandField '%s'\n", command));

	const XML_Char* atts[5];
	atts[0] = "type";
	atts[1] = 0;
	atts[2] = 0;
	atts[3] = 0;
	atts[4] = 0;

	if (*command != 0x13)
	{
		UT_DEBUGMSG(("DOM: field did not begin with 0x13\n"));
		return true;
	}

	//first skip the 0x13
	command++;
	token = strtok(command, "\t, ");
	
	while(token)
	{
		tokenIndex = s_mapNameToField (token);
		if(!bTypeSet)
		{
			f->type = tokenIndex;
			bTypeSet = true;
		}
		
		switch (tokenIndex)
		{
			case F_EDITTIME:
			case F_TIME:
				atts[1] = "time";
				break;

			case F_DateTimePicture: // this isn't totally correct, but it's close
			case F_DATE:
				atts[1] = "date";
				break;

			case F_PAGE:
				atts[1] = "page_number";
				break;

			case F_NUMCHARS:
				atts[1] = "char_count";
				break;

			case F_NUMPAGES:
				atts[1] = "page_count";
				break;

			case F_NUMWORDS:
				atts[1] = "word_count";
				break;

			case F_FILENAME:
				atts[1] = "file_name";
				break;

			case F_PAGEREF:
				token = strtok (NULL, "\"\" ");
				atts[1] = "page_ref";
				atts[2] = "param";
				if(token)
					atts[3] = static_cast<const XML_Char *>(token);
				else
					atts[3] = "no_bookmark_given";
				break;

			case F_HYPERLINK:
				{
					const XML_Char *new_atts[3];
					token = strtok (NULL, "\"\" ");

					// hyperlink or hyperlink to bookmark
					new_atts[0] = "xlink:href";
					UT_String href;
					if ( !strcmp(token, "\\l") )
					{
						token = strtok (NULL, "\"\" ");
						href = "#";
						href += token;
					}
					else
					{
						href = token;
					}
					new_atts[1] = href.c_str();
					new_atts[2] = 0;
					this->_flush();

					if(!m_bInPara)
					{
						_appendStrux(PTX_Block, NULL);
						m_bInPara = true ;
					}

					_appendObject(PTO_Hyperlink, new_atts);
					return true;
				}

			case F_TOC:             // for the toc fields we will
			case F_TOC_FROM_RANGE:  // insert the field result for now
				UT_DEBUGMSG(("TOC field encountered\n"));
			default:
				// unhandled field type
				token = strtok(NULL, "\t, ");
				continue;
		}

		
		this->_flush();

		if(!m_bInPara)
		{
			_appendStrux(PTX_Block, NULL);
			m_bInPara = true ;
		}

		if (!_appendObject (PTO_Field, static_cast<const XML_Char**>(&atts[0])))
		{
			UT_DEBUGMSG(("Dom: couldn't append field (type = '%s')\n", atts[1]));
		}

		token = strtok(NULL, "\t, ");
	}

	return true;
}

typedef enum {
  MSWord_UnknownImage,
  MSWord_VectorImage,
  MSWord_RasterImage
} MSWord_ImageType;

static MSWord_ImageType s_determineImageType ( Blip * b )
{
  if ( !b )
	return MSWord_UnknownImage;

  switch ( b->type )
	{
	case msoblipEMF:
	case msoblipWMF:
	case msoblipPICT:
	  return MSWord_VectorImage;

	case msoblipJPEG:
	case msoblipPNG:
	case msoblipDIB:
	  return MSWord_RasterImage;

	case msoblipERROR:
	case msoblipUNKNOWN:
	default:
	  return MSWord_UnknownImage;
	}
}

UT_Error IE_Imp_MsWord_97::_handleImage (Blip * b, long width, long height)
{
	const char * old_locale = "" ;
	const char * mimetype 	= UT_strdup ("image/png");
	IE_ImpGraphic * importer	= 0;
	FG_Graphic* pFG		= 0;
	UT_Error error		= UT_OK;
	UT_ByteBuf * buf		= 0;
	UT_ByteBuf * pictData 	= new UT_ByteBuf();

  // suck the data into the ByteBuffer

  MSWord_ImageType imgType = s_determineImageType ( b );

  wvStream *pwv;

  if ( imgType == MSWord_RasterImage )
	{
	  pwv = b->blip.bitmap.m_pvBits;
	}
  else if ( imgType == MSWord_VectorImage )
	{
	  pwv = b->blip.metafile.m_pvBits;
	}
  else
	{
	  UT_DEBUGMSG(("UNKNOWN IMAGE TYPE!!"));
	  DELETEP(pictData);
	  FREEP(mimetype);
	  return UT_ERROR;
	}

  size_t size = wvStream_size (pwv);
  char *data = new char[size];
  wvStream_rewind(pwv);
  wvStream_read(data,size,sizeof(char),pwv);
  pictData->append(reinterpret_cast<const UT_Byte*>(data), size);
  delete [] data;

  UT_String propBuffer;
  UT_String propsName;

  if(!pictData->getPointer(0))
	  error =  UT_ERROR;
  else
	  error = IE_ImpGraphic::constructImporter (pictData, IEGFT_Unknown, &importer);

  if ((error != UT_OK) || !importer)
	{
	  UT_DEBUGMSG(("Could not create image importer object\n"));
	  DELETEP(pictData);
	  FREEP(mimetype);
	  goto Cleanup;
	}

  error = importer->importGraphic(pictData, &pFG);
  if ((error != UT_OK) || !pFG)
	{
	  UT_DEBUGMSG(("Could not import graphic\n"));
	  // pictData is already freed in ~FG_Graphic
	  FREEP(mimetype);
	  goto Cleanup;
	}

  // TODO: can we get back a vector graphic?
  buf = static_cast<FG_GraphicRaster *>(pFG)->getRaster_PNG();

  if (!buf)
	{
	  // i don't think that this could ever happen, but...
	  UT_DEBUGMSG(("Could not convert to PNG\n"));
	  DELETEP(pictData);
	  FREEP(mimetype);
	  error = UT_ERROR;
	  goto Cleanup;
	}

  //
  // This next bit of code will set up our properties based on the image attributes
  //

  old_locale = setlocale(LC_NUMERIC, "C");
  UT_String_sprintf(propBuffer, "width:%fin; height:%fin",
		    static_cast<double>(width) / static_cast<double>(1440),
		    static_cast<double>(height) / static_cast<double>(1440));
  setlocale(LC_NUMERIC, old_locale);

  UT_String_sprintf(propsName, "%d", getDoc()->getUID(UT_UniqueId::Image));

  const XML_Char* propsArray[5];
  propsArray[0] = static_cast<const XML_Char *>("props");
  propsArray[1] = static_cast<const XML_Char *>(propBuffer.c_str());
  propsArray[2] = static_cast<const XML_Char *>("dataid");
  propsArray[3] = static_cast<const XML_Char *>(propsName.c_str());
  propsArray[4] = 0;

  if(!m_bInPara)
  {
	  _appendStrux(PTX_Block, NULL);
	  m_bInPara = true ;
  }

  if (!_appendObject (PTO_Image, propsArray))
	{
	  UT_DEBUGMSG (("Could not create append object\n"));
	  error = UT_ERROR;
	  FREEP(mimetype);
	  goto Cleanup;
	}

  if (!getDoc()->createDataItem(propsName.c_str(), false,
				buf, const_cast<void*>(static_cast<const void*>(mimetype)), NULL))
	{
	  UT_DEBUGMSG (("Could not create data item\n"));
	  error = UT_ERROR;
	  goto Cleanup;
	}

 Cleanup:
  DELETEP(importer);

  return error;
}

/****************************************************************************/
/****************************************************************************/

//
// wv callbacks to marshall data back to our importer class
//

static int charProc (wvParseStruct *ps, U16 eachchar, U8 chartype, U16 lid)
{
	IE_Imp_MsWord_97 * pDocReader = static_cast <IE_Imp_MsWord_97 *> (ps->userData);
	return pDocReader->_charProc (ps, eachchar, chartype, lid);
}

static int specCharProc (wvParseStruct *ps, U16 eachchar, CHP* achp)
{
	IE_Imp_MsWord_97 * pDocReader = static_cast <IE_Imp_MsWord_97 *> (ps->userData);
	return pDocReader->_specCharProc (ps, eachchar, achp);
}

static int eleProc (wvParseStruct *ps, wvTag tag, void *props, int dirty)
{
	IE_Imp_MsWord_97 * pDocReader = static_cast <IE_Imp_MsWord_97 *> (ps->userData);
	return pDocReader->_eleProc (ps, tag, props, dirty);
}

static int docProc (wvParseStruct *ps, wvTag tag)
{
	IE_Imp_MsWord_97 * pDocReader = static_cast <IE_Imp_MsWord_97 *> (ps->userData);
	return pDocReader->_docProc (ps, tag);
}


//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

void IE_Imp_MsWord_97::_table_open ()
{
  m_iCurrentRow = 0;
  m_iCurrentCell = 0;

  _appendStrux(PTX_Block, NULL);
  _appendStrux(PTX_SectionTable, NULL);

  m_bRowOpen = false;
  m_bCellOpen = false;
  m_bInPara = false;
  xxx_UT_DEBUGMSG(("\n<TABLE>"));
}

//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

void IE_Imp_MsWord_97::_table_close (const wvParseStruct *ps, const PAP *apap)
{
  _cell_close();
  _row_close();

  UT_String props("table-column-props:");

  if (m_vecColumnWidths.size()) {
    // build column width properties string
    UT_String propBuffer;

    for (UT_uint32 i = 0; i < m_vecColumnWidths.size(); i++) {
      UT_String_sprintf(propBuffer,"%s/",
		UT_convertInchesToDimensionString(m_dim,
		  (static_cast<float>(reinterpret_cast<int>(m_vecColumnWidths.getNthItem(i))))/1440.0));
	  
      props += propBuffer;
    }

    props += "; ";
    m_vecColumnWidths.clear ();
  }

  props += "table-line-ignore:0; table-line-type:1; table-line-thickness:0.8pt;";
  
  props += UT_String_sprintf("table-col-spacing:%din", (2 * apap->ptap.dxaGapHalf)/ 1440);

  // apply properties
  PL_StruxDocHandle sdh = getDoc()->getLastStruxOfType(PTX_SectionTable);
  getDoc()->changeStruxAttsNoUpdate(sdh,"props",props.c_str());

  // end-of-table
  _appendStrux(PTX_EndTable, NULL);
  m_bInPara = false ;

  xxx_UT_DEBUGMSG(("\n</TABLE>\n"));
}

//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

void IE_Imp_MsWord_97::_row_open ()
{
  if (m_bRowOpen)
    return;

  m_bRowOpen = true;
  m_iCurrentRow++;
  m_iCurrentCell = 0;

  xxx_UT_DEBUGMSG(("\n\t<ROW:%d>", m_iCurrentRow));
}

//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

void IE_Imp_MsWord_97::_row_close ()
{
  if (m_bRowOpen) {
    xxx_UT_DEBUGMSG(("\t</ROW>"));
  }
  m_bRowOpen = false;
}

//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

// from fp_TableContainer.h
enum
{
  LS_OFF = 0,	        // No line style, which means no line is drawn
  LS_NORMAL = 1 	// A normal solid line
};

static int
sConvertLineStyle (short lineType)
{
  switch (lineType)
    {
    case 0: return LS_NORMAL;
    case 1:
      return LS_NORMAL;

      // TODO: more cases here
    default:
      return LS_NORMAL;
    }
}

static double
brc_to_pixel (int x)
{
  // each unit is 1/8 of a pixel. abi only deals with whole numbers,
  if(x == 255)
    return  0.;
  return x/8.;
}

void IE_Imp_MsWord_97::_cell_open (const wvParseStruct *ps, const PAP *apap)
{
  if (m_bCellOpen || apap->fTtp)
    return;

  // determine column widths
  UT_Vector columnWidths;

  for (int i = 1; i < ps->nocellbounds; i++) {
    int width = apap->ptap.rgdxaCenter[i] - apap->ptap.rgdxaCenter[i - 1];
    if (width <= 0)
      break;
    columnWidths.addItem(reinterpret_cast<void *>(width));
  }

  if (columnWidths.size() > m_vecColumnWidths.size()) {
    m_vecColumnWidths.clear();
    m_vecColumnWidths = columnWidths;
  }

  // add a new cell
  m_bCellOpen = true;
  m_iCurrentCell++;

  UT_String propBuffer;

  int vspan = ps->vmerges[m_iCurrentRow - 1][m_iCurrentCell - 1];

  if (vspan > 0)
    vspan--;

  UT_String_sprintf(propBuffer,
		    "left-attach:%d; right-attach:%d; top-attach:%d; bot-attach:%d; ",
		    m_iCurrentCell - 1,
		    m_iCurrentCell + reinterpret_cast<int>(m_vecColumnSpansForCurrentRow.getNthItem(m_iCurrentCell - 1)),
		    m_iCurrentRow - 1,
		    m_iCurrentRow + vspan
		    );

  propBuffer += UT_String_sprintf("color:%s;", sMapIcoToColor(apap->ptap.rgshd[m_iCurrentCell - 1].icoFore).c_str());
  propBuffer += UT_String_sprintf("bgcolor:%s;", sMapIcoToColor(apap->ptap.rgshd[m_iCurrentCell - 1].icoBack).c_str());
  // so long as it's not the "auto" color
  if (apap->ptap.rgshd[m_iCurrentCell - 1].icoBack != 0)
    propBuffer += "bg-style:1;";

  const char * old_locale = setlocale(LC_NUMERIC, "C");

  propBuffer += UT_String_sprintf("top-color:%s; top-thickness:%fpt; top-style:%d;",
				  sMapIcoToColor(apap->ptap.rgtc[m_iCurrentCell - 1].brcTop.ico).c_str(),
				  brc_to_pixel(apap->ptap.rgtc[m_iCurrentCell - 1].brcTop.dptLineWidth),
				  sConvertLineStyle(apap->ptap.rgtc[m_iCurrentCell - 1].brcTop.brcType));
  propBuffer += UT_String_sprintf("left-color:%s; left-thickness:%fpx; left-style:%d;",
				  sMapIcoToColor(apap->ptap.rgtc[m_iCurrentCell - 1].brcLeft.ico).c_str(),
				  brc_to_pixel(apap->ptap.rgtc[m_iCurrentCell - 1].brcLeft.dptLineWidth),
				  sConvertLineStyle(apap->ptap.rgtc[m_iCurrentCell - 1].brcLeft.brcType));
  propBuffer += UT_String_sprintf("bot-color:%s; bot-thickness:%fpx; bot-style:%d;",
				  sMapIcoToColor(apap->ptap.rgtc[m_iCurrentCell - 1].brcBottom.ico).c_str(),
				  brc_to_pixel(apap->ptap.rgtc[m_iCurrentCell - 1].brcBottom.dptLineWidth),
				  sConvertLineStyle(apap->ptap.rgtc[m_iCurrentCell - 1].brcBottom.brcType));
  propBuffer += UT_String_sprintf("right-color:%s; right-thickness:%fpx; right-style:%d",
				  sMapIcoToColor(apap->ptap.rgtc[m_iCurrentCell - 1].brcRight.ico).c_str(),
				  brc_to_pixel(apap->ptap.rgtc[m_iCurrentCell - 1].brcRight.dptLineWidth),
				  sConvertLineStyle(apap->ptap.rgtc[m_iCurrentCell - 1].brcRight.brcType));

  setlocale (LC_NUMERIC, old_locale);
  xxx_UT_DEBUGMSG(("propbuffer: %s \n",propBuffer.c_str()));

  const XML_Char* propsArray[3];
  propsArray[0] = static_cast<const XML_Char*>("props");
  propsArray[1] = propBuffer.c_str();
  propsArray[2] = NULL;

  _appendStrux(PTX_SectionCell, propsArray);
  m_bInPara = false;

  xxx_UT_DEBUGMSG(("\t<CELL:%d:%d>", static_cast<int>(m_vecColumnSpansForCurrentRow.getNthItem(m_iCurrentCell - 1)), ps->vmerges[m_iCurrentRow - 1][m_iCurrentCell - 1]));
}

//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

void IE_Imp_MsWord_97::_cell_close ()
{
  if (!m_bCellOpen)
    return;

  m_bCellOpen = false;
  _appendStrux(PTX_EndCell, NULL);
  m_bInPara = false ;

  xxx_UT_DEBUGMSG(("</CELL>"));
}


void IE_Imp_MsWord_97::_generateCharProps(UT_String &s, const CHP * achp, wvParseStruct *ps)
{
	UT_String propBuffer;

	// set char tolower if fSmallCaps && fLowerCase
	if ( achp->fSmallCaps && achp->fLowerCase )
		m_bIsLower = true;
	else
		m_bIsLower = false;

	// set language based the lid - TODO: do we want to handle -none- differently?
	s += "lang:";

	unsigned short iLid = 0;
	// I am not sure how the various lids are supposed to work, but
	// achp->fBidi does not mean that the lidBidi is set ...
	if (achp->fBidi)
	{
		iLid = achp->lidBidi;
	}
	else if(ps->fib.fFarEast)
	{
		iLid = achp->lidFE;
	}
	else
	{
		iLid = achp->lid;
	}
	

	// if we do not have meaningful lid, try default ...
	if(!iLid)
		iLid = achp->lidDefault;
	
	s += wvLIDToLangConverter (iLid);
	s += ";";

	// decide best codepage based on the lid (as lang code above)
	UT_String codepage;
	if (achp->fBidi)
		codepage = wvLIDToCodePageConverter (achp->lidBidi);
	else if (!ps->fib.fFarEast)
		codepage = wvLIDToCodePageConverter (achp->lidDefault);
	else
		codepage = wvLIDToCodePageConverter (achp->lidFE);

	// watch out for codepage 0 = unicode
	const char * pNUE = XAP_EncodingManager::get_instance()->getNativeUnicodeEncodingName();

	if (codepage == "CP0")
		codepage = pNUE;
	
	// if this is the first codepage we've seen, use it.
	// if we see more than one different codepage in a document, use unicode.
	if (!getDoc()->getEncodingName())
		getDoc()->setEncodingName(codepage.c_str());
	else if (getDoc()->getEncodingName() != codepage)
		getDoc()->setEncodingName(pNUE);

	// bold text
	bool fBold = (achp->fBidi ? achp->fBoldBidi : achp->fBold);
	if (fBold) {
		s += "font-weight:bold;";
	}

	// italic text
	bool fItalic = (achp->fBidi ? achp->fItalicBidi : achp->fItalic);
	if (fItalic) {
		s += "font-style:italic;";
	}

	// foreground color
	U8 ico = (achp->fBidi ? achp->icoBidi : achp->ico);
	if (ico) {
		UT_String_sprintf(propBuffer, "color:%s;",
						  sMapIcoToColor(ico).c_str());
		s += propBuffer;
	}

	// background color
	ico = achp->shd.icoBack;
	if (ico) {
		UT_String_sprintf(propBuffer, "background-color:%s;",
						  sMapIcoToColor(ico).c_str());
		s += propBuffer;
	}
	

	// underline and strike-through
	if (achp->fStrike || achp->kul) {
		s += "text-decoration:";
		if ((achp->fStrike || achp->fDStrike) && achp->kul) {
			s += "underline line-through;";
		} else if (achp->kul) {
			s += "underline;";
		} else {
			s += "line-through;";
		}
	}

	// background color
	if (achp->fHighlight) {
		UT_String_sprintf(propBuffer,"bgcolor:%s;",
						  sMapIcoToColor(achp->icoHighlight).c_str());
		s += propBuffer;
	}

	// superscript && subscript
	if (achp->iss == 1) {
		s += "text-position: superscript;";
	} else if (achp->iss == 2) {
		s += "text-position: subscript;";
	}

	if (achp->fVanish)
	{
	    s += "display:none;";
	}

	// font size (hps is half-points)
	// I have seen a bidi doc that had hpsBidi == 0, and the actual size in hps
	U16 hps = (achp->fBidi &&  achp->hpsBidi ? achp->hpsBidi : achp->hps);
	UT_String_sprintf(propBuffer,
					  "font-size:%dpt;", (int)(hps/2));
	s += propBuffer;

	// font family
	char *fname;

	// if the FarEast flag is set, use the FarEast font,
	// otherwise, we'll use the ASCII font.
	if (achp->fBidi) {
		fname = wvGetFontnameFromCode(&ps->fonts, achp->ftcBidi);
	} else if (!ps->fib.fFarEast) {
		fname = wvGetFontnameFromCode(&ps->fonts, achp->ftcAscii);
	} else {
		fname = wvGetFontnameFromCode(&ps->fonts, achp->ftcFE);

		if (strlen (fname) > 6)
			fname[6] = '\0';

		const char *f=XAP_EncodingManager::cjk_word_fontname_mapping.lookupByTarget(fname);

		if (f == fname)
		{
			FREEP (fname);
			fname = UT_strdup ("song");
		}
		else
		{
			FREEP (fname);
			fname = UT_strdup (f ? f : "helvetic");
		}
	}

	// there are times when we should use the third, Other font,
	// and the logic to know when somehow depends on the
	// character sets or encoding types? it's in the docs.

	UT_ASSERT_HARMLESS(fname != NULL);
	xxx_UT_DEBUGMSG(("font-family = %s\n", fname));

	s += "font-family:";

	if(fname)
		s += fname;
	else
		s += "Times New Roman";
	FREEP(fname);
}

void IE_Imp_MsWord_97::_generateParaProps(UT_String &s, const PAP * apap, wvParseStruct *ps)
{
	UT_String propBuffer;

	// DOM TODO: i think that this is right
	if (apap->fBidi == 1)
	{
		s += "dom-dir:rtl;";
	}
	else
	{
		s += "dom-dir:ltr;";
	}

	// paragraph alignment/justification
	switch(apap->jc)
	{
		case 0:
			s += "text-align:left;";
			break;
		case 1:
			s += "text-align:center;";
			break;
		case 2:
			s += "text-align:right;";
			break;
		case 3:
			s += "text-align:justify;";
			break;
		case 4:
			/* this type of justification is of unknown purpose and is
			 * undocumented , but it shows up in asian documents so someone
			 * should be able to tell me what it is someday
			 */
			s += "text-align:justify;";
			break;
	}

	// keep paragraph together?
	if (apap->fKeep) {
		s += "keep-together:yes;";
	}

	// keep with next paragraph?
	if (apap->fKeepFollow) {
		s += "keep-with-next:yes;";
	}

	// widowed/orphaned lines
	if (!apap->fWidowControl) {
		// these AbiWord properties give the same effect
		s += "orphans:0;widows:0;";
	}

	// line spacing (single-spaced, double-spaced, etc.)
	if (apap->lspd.fMultLinespace) {
		UT_String_sprintf(propBuffer,
						  "line-height:%s;",
						  UT_convertToDimensionlessString( (static_cast<float>(apap->lspd.dyaLine) / 240), "1.1"));
		s += propBuffer;
	} else {
		// TODO: handle exact line heights
	}

	//
	// margins
	//

	// margin-right
	if (apap->dxaRight) {
		UT_String_sprintf(propBuffer,
						  "margin-right:%s;",
						  UT_convertInchesToDimensionString(m_dim, (static_cast<float>(apap->dxaRight) / 1440)));
		s += propBuffer;
	}

	// margin-left
	if (apap->dxaLeft) {
		UT_String_sprintf(propBuffer,
						  "margin-left:%s;",
						  UT_convertInchesToDimensionString(m_dim, (static_cast<float>(apap->dxaLeft) / 1440)));
		s += propBuffer;
	}

	// margin-left first line (indent)
	if (apap->dxaLeft1) {
		UT_String_sprintf(propBuffer,
						  "text-indent:%s;",
						  UT_convertInchesToDimensionString(m_dim, (static_cast<float>(apap->dxaLeft1) / 1440)));
		s += propBuffer;
	}

	// margin-top
	if (apap->dyaBefore) {
		UT_String_sprintf(propBuffer,
						  "margin-top:%dpt;", (apap->dyaBefore / 20));
		s += propBuffer;
	}

	// margin-bottom
	if (apap->dyaAfter) {
		UT_String_sprintf(propBuffer,
						  "margin-bottom:%dpt;", (apap->dyaAfter / 20));
	}

	// tab stops
	if (apap->itbdMac) {
		propBuffer += "tabstops:";

		for (int iTab = 0; iTab < apap->itbdMac; iTab++) {
			propBuffer += UT_String_sprintf("%s/",
						UT_convertInchesToDimensionString(m_dim,
										((static_cast<float>(apap->rgdxaTab[iTab])) / 1440)));
			
			switch (apap->rgtbd[iTab].jc) {
				case 1:
					propBuffer += "C,";
					break;
				case 2:
					propBuffer += "R,";
					break;
				case 3:
					propBuffer += "D,";
					break;
				case 4:
					propBuffer += "B,";
					break;
				case 0:
				default:
					propBuffer += "L,";
					break;
			}
		}
		// replace final comma with a semi-colon
		propBuffer[propBuffer.size()-1] = ';';
		s += propBuffer;
	}

	// foreground color
	U8 ico = apap->shd.icoFore;
	if (ico) {
		UT_String_sprintf(propBuffer, "color:%s;",
						  sMapIcoToColor(ico).c_str());
		s += propBuffer;
	}

	// background color
	ico = apap->shd.icoBack;
	if (ico) {
		UT_String_sprintf(propBuffer, "background-color:%s;",
						  sMapIcoToColor(ico).c_str());
		s += propBuffer;
	}

	// remove the trailing semi-colon
	s [s.size()-1] = 0;

}


/*! imports a stylesheet from our document */

#define PT_MAX_ATTRIBUTES 8
void IE_Imp_MsWord_97::_handleStyleSheet(const wvParseStruct *ps)
{
	UT_uint32 iCount = ps->stsh.Stshi.cstd;
//	UT_uint16 iBase  = ps->stsh.Stshi.cbSTDBaseInFile;

	const XML_Char * attribs[PT_MAX_ATTRIBUTES*2 + 1];
	UT_uint32 iOffset = 0;
	
	const STD * pSTD = ps->stsh.std;
	const STD * pSTDBase = pSTD;
	UT_String props;
	char * s = NULL;
	char * b = NULL;
	char * f = NULL;

	for(UT_uint32 i = 0; i < iCount; i++, pSTD++)
	{
		iOffset = 0;

		if(!pSTD->xstzName)
		{
			continue;
		}

		UT_DEBUGMSG(("Style name: [%s], id: %d\n", pSTD->xstzName, pSTD->sti));

		attribs[iOffset++] = PT_NAME_ATTRIBUTE_NAME;

		// make sure we use standard names for standard styles
		const XML_Char * pName = s_translateStyleId(pSTD->sti);

		if(pName)
		{
			attribs[iOffset++] = pName;
		}
		else
		{
			s = s_stripDangerousChars(pSTD->xstzName);
			attribs[iOffset++] = s;
		}
		
		
		attribs[iOffset++] = PT_TYPE_ATTRIBUTE_NAME;
		if(pSTD->sgc == sgcChp)
		{
			attribs[iOffset++] = "C";
		}
		else
		{
			attribs[iOffset++] = "P";

			// also handle the followed-by, since that only applies to
			// paragraph style
			if(pSTD->istdNext != istdNil)
			{
				attribs[iOffset++] = PT_FOLLOWEDBY_ATTRIBUTE_NAME;
				const char * t = s_translateStyleId(pSTD->istdNext);
				if(!t)
					t = f = s_stripDangerousChars((pSTDBase + pSTD->istdNext)->xstzName);
				attribs[iOffset++] = t;
			}
		}

		if(pSTD->istdBase != istdNil)
		{
			attribs[iOffset++] = PT_BASEDON_ATTRIBUTE_NAME;
			const char * t = s_translateStyleId(pSTD->istdBase);
			if(!t)
				t = b = s_stripDangerousChars((pSTDBase + pSTD->istdBase)->xstzName);
			attribs[iOffset++] = t;
		}
		
		// now we want to generate props
		props.clear();

		wvParseStruct * PS = const_cast<wvParseStruct *>(ps);
		
		CHP achp;
		wvInitCHPFromIstd(&achp, (U16)i, &(PS->stsh));
		_generateCharProps(props,&achp,PS);

		if(props.size())
		{
			props += ";";
		}
		
		PAP apap;
		wvInitPAPFromIstd (&apap, (U16)i, &(PS->stsh));
		_generateParaProps(props,&apap,PS);

		// remove trailing semicolon
		if(props[props.size()-1] == ';')
		{
			props[props.size()-1] = 0;
		}
		
		xxx_UT_DEBUGMSG(("Style props: %s\n", props.c_str()));

		if(props.size())
		{
			attribs[iOffset++] = PT_PROPS_ATTRIBUTE_NAME;
			attribs[iOffset++] = props.c_str();
		}
		
		attribs[iOffset] = NULL;

		PD_Style * pStyle = NULL;
		if(getDoc()->getStyle(pSTD->xstzName, &pStyle))
		{
			xxx_UT_DEBUGMSG(("Redefining style %s\n", pSTD->xstzName));
			pStyle->addAttributes(attribs);
			pStyle->getBasedOn();
			pStyle->getFollowedBy();
		}
		else
		{
			getDoc()->appendStyle(attribs);
		}

		FREEP(s); s = NULL;
		FREEP(b); b = NULL;
		FREEP(f); f = NULL;
	}
}

int IE_Imp_MsWord_97::_handleBookmarks(const wvParseStruct *ps)
{
	UT_uint32 i,j;

	if(m_pBookmarks)
	{
		for(i = 0; i < m_iBookmarksCount; i++)
		{
			if(m_pBookmarks[i].name && m_pBookmarks[i].start)
			{
				delete []m_pBookmarks[i].name;
				m_pBookmarks[i].name = NULL;
			}
		}
		delete [] m_pBookmarks;
	}
	BKF *bkf;
	BKL *bkl;
	U32 *posf, *posl, nobkf, nobkl;

	if(!wvGetBKF_PLCF (&bkf, &posf, &nobkf, ps->fib.fcPlcfbkf, ps->fib.lcbPlcfbkf, ps->tablefd))
	{
		m_iBookmarksCount = nobkf;
	}
	else
		m_iBookmarksCount = 0;

	if(!wvGetBKL_PLCF (&bkl, &posl, &nobkl, ps->fib.fcPlcfbkl, ps->fib.lcbPlcfbkl, ps->fib.fcPlcfbkf, ps->fib.lcbPlcfbkf, ps->tablefd))
	{
		m_iBookmarksCount += nobkl;
	}
	else
	{
		if(m_iBookmarksCount > 0)
		{
			//free the bkf and posf
			wvFree(bkf);
			wvFree(posf);
			m_iBookmarksCount = 0;
		}
	}
	UT_return_val_if_fail(nobkl == nobkf, 0);
	if(m_iBookmarksCount > 0)
	{
		m_pBookmarks = new bookmark[m_iBookmarksCount];
		UT_return_val_if_fail(m_pBookmarks, 0);
		for(i = 0; i < nobkf; i++)
		{
			m_pBookmarks[i].name = _getBookmarkName(ps, i);
			m_pBookmarks[i].pos  = posf[i];
			m_pBookmarks[i].start = true;
		}

		for(j = i; j < nobkl + i; j++)
		{
			// since the name is shared with the start of the bookmark,
			// we reuse it
			UT_sint32 iBkf = static_cast<UT_sint32>(bkl[j-i].ibkf) < 0 ? nobkl + static_cast<UT_sint32>(bkl[j-i].ibkf) : bkl[j-i].ibkf;
			m_pBookmarks[j].name = m_pBookmarks[iBkf].name;
			m_pBookmarks[j].pos  = posl[j - i];
			m_pBookmarks[j].start = false;
		}
		// free bkf, bkl, posf, posl
		wvFree(bkf);
		wvFree(bkl);
		wvFree(posf);
		wvFree(posl);

		//now sort the bookmarks by position
		qsort(static_cast<void*>(m_pBookmarks),
			  m_iBookmarksCount, sizeof(bookmark),
			  s_cmp_bookmarks_qsort);
		
#ifdef DEBUG
		for(UT_uint32 k = 0; k < m_iBookmarksCount; k++)
		{
			UT_DEBUGMSG(("Bookmark: name [%s], pos %d, start %d\n",
						 m_pBookmarks[k].name,m_pBookmarks[k].pos,m_pBookmarks[k].start));
		}

#endif
	}
	return 0;
}

void IE_Imp_MsWord_97::_handleNotes(const wvParseStruct *ps)
{
	UT_uint32 i;

	if(m_pFootnotes)
	{
		delete [] m_pFootnotes;
		m_pFootnotes = NULL;
	}

	if(m_pEndnotes)
	{
		delete [] m_pEndnotes;
		m_pEndnotes = NULL;
	}

	m_iFootnotesCount = 0;
	m_iEndnotesCount = 0;
	UT_uint32 *pPLCF_ref = NULL;
	UT_uint32 *pPLCF_txt = NULL;

	bool bNoteError = false;

	if(ps->fib.lcbPlcffndTxt)
	{
		/* the docs say -1, but that is an error */
		m_iFootnotesCount = ps->fib.lcbPlcffndTxt/4 - 2;
		m_pFootnotes = new footnote[m_iFootnotesCount];
		UT_return_if_fail(m_pFootnotes);
		
		// this is really quite straight forward; we retrieve the PLCF
		// chunks that describe the references/text of the footnotes, and
		// then use those to init our footnote stucts
		// for n footnotes the reference PLCF is a sequnce of (n+1) doc
		// positions (UT_uint32) followed by n type flags (UT_uint16)
		// the text PLCF is a sequence of n+2 positions (UT_uint32) of the footnote
		// text in its data stream 
		if(wvGetPLCF((void **) &pPLCF_ref, ps->fib.fcPlcffndRef, ps->fib.lcbPlcffndRef, ps->tablefd))
		{
			bNoteError = true;
		}

		if(!bNoteError &&
		   wvGetPLCF((void **) &pPLCF_txt, ps->fib.fcPlcffndTxt, ps->fib.lcbPlcffndTxt, ps->tablefd))
		{
			wvFree(pPLCF_ref);
			bNoteError = true;
		}
	
		if(!bNoteError)
		{
			UT_return_if_fail(pPLCF_ref && pPLCF_txt);
			for(i = 0; i < m_iFootnotesCount; i++)
			{
				m_pFootnotes[i].ref_pos = pPLCF_ref[i];
				m_pFootnotes[i].txt_pos = pPLCF_txt[i] + m_iFootnotesStart;
				m_pFootnotes[i].txt_len = pPLCF_txt[i+1] - pPLCF_txt[i];
				UT_uint32 iType = ((UT_uint16*)pPLCF_ref)[2*(m_iFootnotesCount + 1) + i];
				m_pFootnotes[i].type = iType;
				m_pFootnotes[i].pid = getDoc()->getUID(UT_UniqueId::Footnote);
				UT_DEBUGMSG(("IE_Imp_MsWord_97::_handleNotes: fnote %d, rpos %d, tpos %d, type %d\n",
							 i, m_pFootnotes[i].ref_pos, m_pFootnotes[i].txt_pos, iType));
			}

			wvFree(pPLCF_ref);
			wvFree(pPLCF_txt);
		}

		// next, deal footnote formatting matters
		const XML_Char * props[] = {"document-footnote-type",            NULL,
									"document-footnote-initial",         NULL,
									"document-footnote-restart-section", NULL,
									"document-footnote-restart-page",    NULL,
		                            NULL};

		switch(ps->dop.rncFtn)
		{
			case 0:
				props[5] = "0";
				props[7] = "0";
				break;
			case 1:
				props[5] = "1";
				props[7] = "0";
				break;
			case 2:
				props[5] = "0";
				props[7] = "1";
				break;
			default:
				UT_ASSERT(UT_NOT_REACHED);
		}

		UT_String number;
		UT_String_sprintf(number, "%d", ps->dop.nFtn);
		props[3] = number.c_str();

		switch(ps->dop.nfcFtnRef)
		{
			case 0:
				props[1] = "numeric";
				break;
			case 1:
				props[1] = "upper-roman";
				break;
			case 2:
				props[1] = "lower-roman";
				break;
			case 3:
				props[1] = "upper";
				break;
			case 4:
				props[1] = "lower";
				break;
			default:
				UT_ASSERT(UT_NOT_REACHED);
		}
		
		getDoc()->setProperties(&props[0]);
	}
	
	if(ps->fib.lcbPlcfendTxt)
	{
		m_iEndnotesCount  = ps->fib.lcbPlcfendTxt/4 - 2;
		m_pEndnotes  = new footnote[m_iEndnotesCount];
		UT_return_if_fail(m_pEndnotes);

		bNoteError = false;
		if(wvGetPLCF((void **) &pPLCF_ref, ps->fib.fcPlcfendRef, ps->fib.lcbPlcfendRef, ps->tablefd))
		{
			bNoteError = true;
		}

		if(!bNoteError &&
		   wvGetPLCF((void **) &pPLCF_txt, ps->fib.fcPlcfendTxt, ps->fib.lcbPlcfendTxt, ps->tablefd))
		{
			wvFree(pPLCF_ref);
			bNoteError = true;
		}

		if(!bNoteError)
		{
			UT_return_if_fail(pPLCF_ref && pPLCF_txt);
			for(i = 0; i < m_iEndnotesCount; i++)
			{
				m_pEndnotes[i].ref_pos = pPLCF_ref[i];
				m_pEndnotes[i].txt_pos = pPLCF_txt[i] + m_iEndnotesStart;
				m_pEndnotes[i].txt_len = pPLCF_txt[i+1] - pPLCF_txt[i];
				UT_uint32 iType = ((UT_uint16*)pPLCF_ref)[2*(m_iEndnotesCount + 1) + i];
				m_pEndnotes[i].type = iType;
				m_pEndnotes[i].pid = getDoc()->getUID(UT_UniqueId::Endnote);
				UT_DEBUGMSG(("IE_Imp_MsWord_97::_handleNotes: enote %d, rpos %d, tpos %d, type %d\n",
							 i, m_pEndnotes[i].ref_pos, m_pEndnotes[i].txt_pos, iType));
			}

			wvFree(pPLCF_ref);
			wvFree(pPLCF_txt);
		}
		// next, deal endnote formatting matters
		const XML_Char * props[] = {"document-endnote-type",            NULL,
									"document-endnote-initial",         NULL,
									"document-endnote-restart-section", NULL,
									"document-endnote-restart-page",    NULL,
									"document-endnote-place-endsection",NULL,
									"document-endnote-place-enddoc",    NULL,
		                            NULL};

		switch(ps->dop.rncEdn)
		{
			case 0:
				props[5] = "0";
				props[7] = "0";
				break;
			case 1:
				props[5] = "1";
				props[7] = "0";
				break;
			case 2:
				props[5] = "0";
				props[7] = "1";
				break;

			default:
				UT_ASSERT(UT_NOT_REACHED);
		}

		UT_String number;
		UT_String_sprintf(number, "%d", ps->dop.nEdn);
		props[3] = number.c_str();

		switch(ps->dop.nfcEdnRef)
		{
			case 0:
				props[1] = "numeric";
				break;
			case 1:
				props[1] = "upper-roman";
				break;
			case 2:
				props[1] = "lower-roman";
				break;
			case 3:
				props[1] = "upper";
				break;
			case 4:
				props[1] = "lower";
				break;

			default:
				UT_ASSERT(UT_NOT_REACHED);
				
		}

		switch(ps->dop.epc)
		{
			case 0:
				props[9]  = "1";
				props[11] = "0";
				break;
			case 3:
				props[9]  = "0";
				props[11] = "1";
				break;
			default:
				UT_ASSERT(UT_NOT_REACHED);
				
		}
				
		getDoc()->setProperties(&props[0]);
	}
}

/*!
   Determines whether footnote is to be inserted at present document
   position, and if so takes care of inserting the reference marker,
   note section and anchor marker.

   returns true if a note was successfully inserted, false otherwise;
   if the return value is true, the caller should ignore the present character
   
   we will take advantage of the notes being in document order, so we
   can just remember the last note we inserted, rather than having to
   search through the list

*/
bool IE_Imp_MsWord_97::_insertNoteIfAppropriate(UT_uint32 iDocPosition, UT_UCS4Char c)
{
	if(m_bInFNotes || m_bInENotes)
		return false;
	
	bool res = false;
	//now search for position iDocPosition in our footnnote list;
	if(!m_pFootnotes || m_iFootnotesCount == 0 || m_iNextFNote >= m_iFootnotesCount)
	{
		goto endnotes;
	}

	if(m_pFootnotes[m_iNextFNote].ref_pos == iDocPosition)
	{
		res |= _insertFootnote(m_pFootnotes + m_iNextFNote++,c);
	}
	
 endnotes:
	if(!m_pEndnotes || m_iEndnotesCount == 0 || m_iNextENote >= m_iEndnotesCount)
	{
		goto finish;
	}
	
	if(m_pEndnotes[m_iNextENote].ref_pos == iDocPosition)
	{
		res |= _insertEndnote(m_pEndnotes + m_iNextENote++,c);
	}
	
	
 finish:	
	return res;
}

/* returns true on successful insertion of the reference marker */
bool IE_Imp_MsWord_97::_insertFootnote(const footnote * f, UT_UCS4Char c)
{
	UT_return_val_if_fail(f, true);
	UT_DEBUGMSG(("IE_Imp_MsWord_97::_insertFootnote: pos: %d, pid %d\n", f->ref_pos, f->pid));

	this->_flush();

	bool res = true;
	const XML_Char * attribsS[3] ={"footnote-id",NULL,NULL};
	const XML_Char* attribsR[9] = {"type", "footnote_ref", "footnote-id",
								   NULL, NULL, NULL, NULL, NULL, NULL};
	UT_uint32 iOffR = 3;

	UT_String footpid;
	UT_String_sprintf(footpid,"%i",f->pid);
	attribsS[1] = footpid.c_str();

	// for attribsR we need to set props and style in order to
	// preserve any formating set by a previous call to _beginChar()
	attribsR[iOffR++] = footpid.c_str();
	attribsR[iOffR++] = "props";
	attribsR[iOffR++] = m_charProps.c_str();
	attribsR[iOffR++] = "style";
	attribsR[iOffR++] = m_charStyle.c_str();
		
	UT_ASSERT( iOffR <= sizeof(attribsR)/sizeof(XML_Char*) );
	
	if(f->type)
	{
		// auto-generated reference -- insert a field
		res &= _appendObject(PTO_Field, attribsR);
	}
	else
	{
		// manually-inserted marker, we need to issue the character
		// TODO -- in word the marker can consist of several
		// characters, but I have no idea how Word knows how many;
		// we at least need to reset the character formatting again
		// after we have inserted the footnote section
		res &= _appendSpan(&c,1);
	}
	
	_appendStrux(PTX_SectionFootnote,attribsS);
	_appendStrux(PTX_EndFootnote,NULL);

	if(!f->type)
	{
		// set the formatting to whatever it was, in case the footnote
		// marker is longer than one character
		_appendFmt(&attribsR[0]);
	}
	
	return res;
}

bool IE_Imp_MsWord_97::_insertEndnote(const footnote * f, UT_UCS4Char c)
{
	UT_return_val_if_fail(f, true);
	UT_DEBUGMSG(("IE_Imp_MsWord_97::_insertEndnote: pos: %d, pid %d\n", f->ref_pos, f->pid));

	this->_flush();

	bool res = true;
	const XML_Char * attribsS[3] ={"endnote-id",NULL,NULL};
	const XML_Char* attribsR[9] = {"type", "endnote_ref", "endnote-id",
								   NULL, NULL, NULL, NULL, NULL, NULL};
	UT_uint32 iOffR = 3;

	UT_String footpid;
	UT_String_sprintf(footpid,"%i",f->pid);
	attribsS[1] = footpid.c_str();

	// for attribsR we need to set props and style in order to
	// preserve any formating set by a previous call to _beginChar()
	attribsR[iOffR++] = footpid.c_str();
	attribsR[iOffR++] = "props";
	attribsR[iOffR++] = m_charProps.c_str();
	attribsR[iOffR++] = "style";
	attribsR[iOffR++] = m_charStyle.c_str();
		
	UT_ASSERT( iOffR <= sizeof(attribsR)/sizeof(XML_Char*) );
	
	if(f->type)
	{
		// auto-generated reference -- insert a field
		res &= _appendObject(PTO_Field, attribsR);
	}
	else
	{
		// manually-inserted marker, we need to issue the character
		// TODO -- in word the marker can consist of several
		// characters, but I have no idea how Word knows how many;
		// we at least need to reset the character formatting again
		// after we have inserted the footnote section
		res &= _appendSpan(&c,1);
	}
	
	_appendStrux(PTX_SectionEndnote,attribsS);
	_appendStrux(PTX_EndEndnote,NULL);

	if(!f->type)
	{
		// set the formatting to whatever it was, in case the footnote
		// marker is longer than one character
		_appendFmt(&attribsR[0]);
	}
	
	return res;
}


/*!
    This function makes sure that the insert is happening at the
    correct place if we are in a segment which belongs to one of the
    set of notes (foonotes & endnote, in future also annotations).

    \parameter UT_uint32 iDocPosition: character position in the Word
                                       document stream
    \return returns false if the present character is to be skipped,
            true otherwise
*/
bool IE_Imp_MsWord_97::_handleNotesText(UT_uint32 iDocPosition)
{
	if(iDocPosition >= m_iFootnotesStart && iDocPosition < m_iFootnotesEnd)
	{
		// upon entry into the footnote-land, we will need to search for
		// the first footnote section in our document, note that we are
		// in a footnote section, note at what doc position the current
		// footnote will end, and then let things run until we reach
		// the end of the note; then we need to search for the next
		// doc section, etc.

		// if the footnote marker is auto-generated, we need to remove
		// the special character from the stream (happens
		// automatically)

		// when in a footnote section, all the functions that normally
		// use append methods will need to use insert methods instead

		if(!m_bInFNotes)
		{
			UT_DEBUGMSG(("In footnote territory: pos %d\n", iDocPosition));
			m_bInFNotes = true;
			m_bInHeaders = false;
			
			// we will reuse the m_iNextFNote variable, noting it
			// refers to the CURRENT footnote
			m_iNextFNote = 0;
			_findNextFNoteSection();
			_endSect(NULL,0,NULL,0);
			m_bInSect = true;
		}

		// the current footnote will end at pos
		// f.txt_pos + f.txt_len, 
		if(iDocPosition == m_pFootnotes[m_iNextFNote].txt_pos +
		                   m_pFootnotes[m_iNextFNote].txt_len)
		{
			m_iNextFNote++;

			// after the last footnote there is an extra paragraph
			// marker that is still a part of the footnote section --
			// we do not want that marker imported
			if(m_iNextFNote < m_iFootnotesCount)
				_findNextFNoteSection();
			else
			{
				UT_DEBUGMSG(("End of footnotes marker at pos %d\n", iDocPosition));
				return false;
			}
		}

		// if this is the first character in a footnote, insert the reference
		if(iDocPosition == m_pFootnotes[m_iNextFNote].txt_pos)
		{
			const XML_Char* attribsA[] = {"type", "footnote_anchor",
										   "footnote-id", NULL,
										   "props",       NULL,
										   "style",       NULL,
										   NULL};
			
			const XML_Char * attribsB[] = {"props", NULL,
											"style", NULL,
											NULL};

			UT_String footpid;
			UT_String_sprintf(footpid,"%i",m_pFootnotes[m_iNextFNote].pid);
			attribsA[3] = footpid.c_str();
			attribsA[5] = m_charProps.c_str();
			attribsA[7] = m_charStyle.c_str();

			attribsB[1] = m_paraProps.c_str();
			attribsB[3] = m_paraStyle.c_str();

			_appendStrux(PTX_Block,attribsB);
			m_bInPara = true;

			if(m_pFootnotes[m_iNextFNote].type)
			{
				_appendObject(PTO_Field, attribsA);
				return false;
			}
			return true;
		}
		
		// do not return !!!
		UT_DEBUGMSG(("In footnote %d, on pos %d\n", m_iNextFNote, iDocPosition));
	}
	else if(m_bInFNotes)
	{
		m_bInFNotes = false;
		UT_DEBUGMSG(("Leaving footnote territory\n"));
		// move to the end of the do end of the document ...

		// do not return !!!
	}
	
	if(iDocPosition >= m_iEndnotesStart && iDocPosition < m_iEndnotesEnd)
	{
		if(!m_bInENotes)
		{
			UT_DEBUGMSG(("In endnote territory: pos %d\n", iDocPosition));
			m_bInENotes = true;
			m_bInHeaders = false;
			m_iNextENote = 0;
			_findNextENoteSection();
			_endSect(NULL,0,NULL,0);
			m_bInSect = true;
		}

		if(iDocPosition == m_pEndnotes[m_iNextENote].txt_pos +
		                   m_pEndnotes[m_iNextENote].txt_len)
		{
			m_iNextENote++;

			// after the last endnote there is an extra paragraph
			// marker that is still a part of the endnote section --
			// we do not want that marker imported
			if(m_iNextENote < m_iEndnotesCount)
				_findNextENoteSection();
			else
			{
				UT_DEBUGMSG(("End of endnotes marker at pos %d\n", iDocPosition));
				return false;
			}
		}

		// if this is the first character in an endnote, insert the anchor
		if(iDocPosition == m_pEndnotes[m_iNextENote].txt_pos)
		{
			const XML_Char * attribsA[] = {"type", "endnote_anchor",
										   "endnote-id", NULL,
										   "props",       NULL,
										   "style",       NULL,
										   NULL};
			
			const XML_Char * attribsB[] = {"props", NULL,
										   "style", NULL,
										   NULL};

			UT_String footpid;
			UT_String_sprintf(footpid,"%i",m_pEndnotes[m_iNextENote].pid);
			attribsA[3] = footpid.c_str();
			attribsA[5] = m_charProps.c_str();
			attribsA[7] = m_charStyle.c_str();

			attribsB[1] = m_paraProps.c_str();
			attribsB[3] = m_paraStyle.c_str();
			
			_appendStrux(PTX_Block,attribsB);
			m_bInPara = true;

			if(m_pEndnotes[m_iNextENote].type)
			{
				_appendObject(PTO_Field, attribsA);
				return false;
			}
			return true;
		}

		UT_DEBUGMSG(("In endnote %d, on pos %d\n", m_iNextENote, iDocPosition));
		// do not return !!!
	}
	else if(m_bInENotes)
	{
		m_bInENotes = false;
		UT_DEBUGMSG(("Leaving endnote territory\n"));
		// move to the end of the document ...

		// do not return !!!
	}

	// we only return here, so that the code above could be extended
	// for handly annotations by simply copy/paste
	return true;
}

bool IE_Imp_MsWord_97::_findNextFNoteSection()
{
	if(!m_iNextFNote)
	{
		// move to the start of the doc first
		m_pNotesEndSection = NULL;
	}

	if(m_pNotesEndSection)
	{
		// move to the next fragment
		m_pNotesEndSection = m_pNotesEndSection->getNext();
		UT_return_val_if_fail(m_pNotesEndSection, false);
	}
	

	m_pNotesEndSection = getDoc()->findFragOfType(pf_Frag::PFT_Strux,
												  (UT_sint32)PTX_EndFootnote,
												  m_pNotesEndSection);

	if(!m_pNotesEndSection)
	{
		UT_DEBUGMSG(("Error: footnote section not found!!!\n"));
		return false;
	}

	return true;
}

bool IE_Imp_MsWord_97::_findNextENoteSection()
{
	if(!m_iNextENote)
	{
		// move to the start of the doc first
		m_pNotesEndSection = NULL;
	}
	
	if(m_pNotesEndSection)
	{
		// move to the next fragment
		m_pNotesEndSection = m_pNotesEndSection->getNext();
		UT_return_val_if_fail(m_pNotesEndSection, false);
	}

	m_pNotesEndSection = getDoc()->findFragOfType(pf_Frag::PFT_Strux,
												  (UT_sint32)PTX_EndEndnote,
												  m_pNotesEndSection);

	if(!m_pNotesEndSection)
	{
		UT_DEBUGMSG(("Error: endnote section not found!!!\n"));
		return false;
	}
	
	return true;
}

bool IE_Imp_MsWord_97::_shouldUseInsert() const
{
	return ((m_bInFNotes || m_bInENotes) && !m_bInHeaders);
}

bool IE_Imp_MsWord_97::_appendStrux(PTStruxType pts, const XML_Char ** attributes)
{
	if(m_bInHeaders)
	{
		return _appendStruxHdrFtr(pts, attributes);
	}
	else if(_shouldUseInsert() && m_pNotesEndSection)
	{
		return getDoc()->insertStruxBeforeFrag(m_pNotesEndSection, pts, attributes);
	}

	return getDoc()->appendStrux(pts, attributes);
}

bool IE_Imp_MsWord_97::_appendObject(PTObjectType pto, const XML_Char ** attributes)
{
	if(m_bInHeaders)
	{
		return _appendObjectHdrFtr(pto, attributes);
	}
	else if(_shouldUseInsert() && m_pNotesEndSection)
	{
		return getDoc()->insertObjectBeforeFrag(m_pNotesEndSection, pto, attributes);
	}

	return getDoc()->appendObject(pto, attributes);
}

bool IE_Imp_MsWord_97::_appendSpan(const UT_UCSChar * p, UT_uint32 length)
{
	if(m_bInHeaders)
	{
		return _appendSpanHdrFtr(p, length);
	}
	else if(_shouldUseInsert() && m_pNotesEndSection)
	{
		return getDoc()->insertSpanBeforeFrag(m_pNotesEndSection, p, length);
	}

	return getDoc()->appendSpan(p, length);
}

bool IE_Imp_MsWord_97::_appendFmt(const XML_Char ** attributes)
{
	// no special processing required, this only changes m_loading in
	// the PT
	return getDoc()->appendFmt(attributes);
}

/*!
    The append*HdrFtr() methods below are needed because in AW headers
    cannot be shared among sections; in contrast in Word one header
    can be used by a chain of sections. We get around it by
    duplicating that one header for each section that uses it. Since
    we cannot wind back throught the data stream we have to duplicate
    each shared header as we go using the info stored in the current
    header's d struct.
*/
bool IE_Imp_MsWord_97::_appendStruxHdrFtr(PTStruxType pts, const XML_Char ** attributes)
{
	UT_return_val_if_fail(m_bInHeaders,false);

	bool bRet = true;
	
	for(UT_uint32 i = 0; i < m_pHeaders[m_iCurrentHeader].d.frag.getItemCount(); i++)
	{
		pf_Frag * pF = (pf_Frag*) m_pHeaders[m_iCurrentHeader].d.frag.getNthItem(i);
		UT_return_val_if_fail(pF,false);

		bRet &= getDoc()->insertStruxBeforeFrag(pF, pts, attributes);
	}
	
	bRet &= getDoc()->appendStrux(pts, attributes);
	return bRet;
}

bool IE_Imp_MsWord_97::_appendObjectHdrFtr(PTObjectType pto, const XML_Char ** attributes)
{
	UT_return_val_if_fail(m_bInHeaders,false);

	bool bRet = true;
	
	for(UT_uint32 i = 0; i < m_pHeaders[m_iCurrentHeader].d.frag.getItemCount(); i++)
	{
		pf_Frag * pF = (pf_Frag*) m_pHeaders[m_iCurrentHeader].d.frag.getNthItem(i);
		UT_return_val_if_fail(pF,false);

		bRet &= getDoc()->insertObjectBeforeFrag(pF, pto, attributes);
	}
	
	bRet &= getDoc()->appendObject(pto, attributes);
	return bRet;
}

bool IE_Imp_MsWord_97::_appendSpanHdrFtr(const UT_UCSChar * p, UT_uint32 length)
{
	UT_return_val_if_fail(m_bInHeaders,false);

	bool bRet = true;
	
	for(UT_uint32 i = 0; i < m_pHeaders[m_iCurrentHeader].d.frag.getItemCount(); i++)
	{
		pf_Frag * pF = (pf_Frag*) m_pHeaders[m_iCurrentHeader].d.frag.getNthItem(i);
		UT_return_val_if_fail(pF,false);

		bRet &= getDoc()->insertSpanBeforeFrag(pF, p, length);
	}
	
	bRet &= getDoc()->appendSpan(p, length);
	return bRet;
}


void IE_Imp_MsWord_97::_handleHeaders(const wvParseStruct *ps)
{
	UT_uint32 i, k;

	if(m_pHeaders)
	{
		delete [] m_pHeaders;
		m_pHeaders = NULL;
	}

	m_iHeadersCount = 0;
	UT_uint32 *pPLCF_txt = NULL;

	/*
	   The header/footer PLCF in Word 97+ is organised as follows:

	   indx         |  function
	   -------------------------------------------------------------------------------
	   0-5: document wide settings
	   -------------------------------------------------------------------------------
	    0           |  footnote separator
	    1           |  footnote continuation separator (i.e., continued on next page)
	    2           |  document-wide footnote continuation notice (i.e., continued
                   	   from previous page)
	   3-5          |  as above for endnotes
       -------------------------------------------------------------------------------
	   now for i-th section in document (i >= 0)
	   -------------------------------------------------------------------------------
	   i+6          |  header even pages
	   i+7          |  header odd  pages
	   i+8          |  footer even pages
	   i+9          |  footer odd  pages
	   i+10         |  header first page
	   i+11         |  footer first page
	   -------------------------------------------------------------------------------
       according to the docs now should come the foot/endnote
	   separators but they do not -- those settings appear to be
	   document wide only ...
	   -------------------------------------------------------------------------------
	   i+12 - i+17  |  as the document wide footnote/endnote separators above

	   NB: the record for the last section in the document may be
	       incomplete, i.e., for n sections  m_iHeadersCount <= 6 + 12*n.
	*/

	bool bHeaderError = false;

	if(ps->fib.lcbPlcfhdd)
	{
		/* the docs are ambiguous, at one place saying the PLCF
		   contains n+2 entries, another n+1; I think the former is correct*/
		m_iHeadersCount = ps->fib.lcbPlcfhdd/4 - 2;
		m_pHeaders = new header[m_iHeadersCount];
		UT_return_if_fail(m_pHeaders);
		
		// this is really quite straight forward; we retrieve the PLCF
		// which is a sequence of n+2 positions (UT_uint32) of the
		// header text in its data stream
		if(wvGetPLCF((void **) &pPLCF_txt, ps->fib.fcPlcfhdd, ps->fib.lcbPlcfhdd, ps->tablefd))
		{
			bHeaderError = true;
		}

		if(!bHeaderError)
		{
			UT_return_if_fail(pPLCF_txt);
			for(i = 0; i < m_iHeadersCount; i++)
			{
				m_pHeaders[i].pos = pPLCF_txt[i] + m_iHeadersStart;
				m_pHeaders[i].len = pPLCF_txt[i+1] - pPLCF_txt[i];
				m_pHeaders[i].pid = getDoc()->getUID(UT_UniqueId::HeaderFtr);
				
				if(i < 6)
				{
					// document wide footnote/endnote separators
					m_pHeaders[i].type = HF_Unsupported;
				}
				else
				{
					switch((i-6)%6)
					{
						case 0:
							m_pHeaders[i].type = HF_HeaderEven;
							break;
						case 1:
							m_pHeaders[i].type = HF_HeaderOdd;
							break;
						case 2:
							m_pHeaders[i].type = HF_FooterEven;
							break;
						case 3:
							m_pHeaders[i].type = HF_FooterOdd;
							break;
						case 4:
							m_pHeaders[i].type = HF_HeaderFirst;
							break;
						case 5:
							m_pHeaders[i].type = HF_FooterFirst;
							break;

						default:
							m_pHeaders[i].type = HF_Unsupported;
					}
				
					UT_DEBUGMSG(("Header no. %d, pos %d, len %d\n",
								 i,m_pHeaders[i].pos,m_pHeaders[i].len));

#if 1
					// this code is here because in AW we currently cannot
					// share headers between sections
					if(m_pHeaders[i].len == 0)
					{
						// this is the case where the section is to use the
						// header of a previous section -- scroll back until
						// we find one
						k = i - 6;
						bool bContinue = false;
				
						while(k > 5)
						{
							if(m_pHeaders[k].len == 2)
							{
								// found empty header 
								bContinue = true;
								break;
							}
							else if(m_pHeaders[k].len == 0)
							{
								// try one section ahead
								k -= 6;
							}
							else
							{
								// found a meaningful header
								break;
							}
						}

						if(bContinue || k < 6)
						{
							continue;
						}

						// so we have found a meaningful header k that is to
						// be used in place of header i; we add header
						// i to k's d-struct

						m_pHeaders[k].d.hdr.addItem((void*)(m_pHeaders+i));
					}
#endif
				}
			}

			wvFree(pPLCF_txt);
		}
	}
}



/*!
    This function makes sure that the insert is happening at the
    correct place if we are in the header segment.

    \parameter UT_uint32 iDocPosition: character position in the Word
                                       document stream
    \return returns false if the present character is to be skipped,
            true otherwise
*/
bool IE_Imp_MsWord_97::_handleHeadersText(UT_uint32 iDocPosition)
{
	if(iDocPosition >= m_iHeadersStart && iDocPosition < m_iHeadersEnd)
	{
		// upon entry into the header-land, we will need to search for
		// the first header/footer section in our document, note that we are
		// in a header section, note at what doc position the current
		// header will end, and then let things run until we reach
		// the end of the header; then we need to search for the next
		// doc section, etc.

		if(!m_bInHeaders)
		{
			UT_DEBUGMSG(("In headers territory: pos %d\n", iDocPosition));
			m_bInHeaders = true;
			m_bInENotes = false;
			m_bInFNotes = false;

			m_iCurrentHeader = 0;

			// we need to close of any open section
			if(m_bInSect)
			{
				_endSect(NULL,0,NULL,0);
			}
			
			// some headers can be 0-length, skip them ... (0-length:  len <=2)
			while(m_pHeaders[m_iCurrentHeader].len <= 2 && m_iCurrentHeader < m_iHeadersCount)
			{
				m_iCurrentHeader++;
			}
		}


		if(iDocPosition == m_pHeaders[m_iCurrentHeader].pos +
		                   m_pHeaders[m_iCurrentHeader].len)
		{
			// new header, time to move on ...
			m_iCurrentHeader++;

			// some headers can be 0-length, skip them ... (0-length:  len <=2)
			while(m_pHeaders[m_iCurrentHeader].len <= 2 && m_iCurrentHeader < m_iHeadersCount)
			{
				m_iCurrentHeader++;
			}

			// after the last header there is an extra paragraph
			// marker that is still a part of the header section --
			// we do not want that marker imported
			if(m_iCurrentHeader ==  m_iHeadersCount)
			{
				UT_DEBUGMSG(("End of header marker at pos %d\n", iDocPosition));
				return false;
			}

			// do not return, processing needs to continue ...
		}
		
		if(iDocPosition == m_pHeaders[m_iCurrentHeader].pos)
		{
			// need to insert our header/footer section, preserving
			// any existing formatting ...
			if(m_pHeaders[m_iCurrentHeader].type != HF_Unsupported &&
			   m_pHeaders[m_iCurrentHeader].len > 2)
			{
				UT_uint32 iOff = 0;
				const XML_Char * attribsB[] = {NULL, NULL,
											   NULL, NULL,
											   NULL};

				if(m_paraProps.size())
				{
					attribsB[iOff++] = "props";
					attribsB[iOff++] = m_paraProps.c_str();
				}

				if(m_paraStyle.size())
				{
					attribsB[iOff++] = "style";
					attribsB[iOff++] = m_paraStyle.c_str();
				}
				
				const XML_Char * attribsC[] = {NULL, NULL,
											   NULL, NULL,
											   NULL};
				iOff = 0;
				if(m_charProps.size())
				{
					attribsC[iOff++] = "props";
					attribsC[iOff++] = m_charProps.c_str();
				}

				if(m_charStyle.size())
				{
					attribsC[iOff++] = "style";
					attribsC[iOff++] = m_charStyle.c_str();
				}

				const XML_Char * attribsS[] = {"type", NULL,
											   "id",   NULL,
											   NULL};

				UT_String id;
				UT_String_sprintf(id,"%d",m_pHeaders[m_iCurrentHeader].pid);
				attribsS[3] = id.c_str();
				
				switch(m_pHeaders[m_iCurrentHeader].type)
				{
					case HF_HeaderEven:
						attribsS[1] = "header-even";
						break;
					case HF_FooterEven:
						attribsS[1] = "footer-even";
						break;
					case HF_HeaderOdd:
						attribsS[1] = "header";
						break;
					case HF_FooterOdd:
						attribsS[1] = "footer";
						break;
					case HF_HeaderFirst:
						attribsS[1] = "header-first";
						break;
					case HF_FooterFirst:
						attribsS[1] = "footer-first";
						break;
					default:
						UT_ASSERT(UT_NOT_REACHED);
				}

				// we use the document methods, not the importer methods intentionally 
				getDoc()->appendStrux(PTX_SectionHdrFtr, attribsS);
				m_bInSect = true;

				getDoc()->appendStrux(PTX_Block, attribsB);
				m_bInPara = true;
				
				_appendFmt(attribsC);

				// now we insert the same for any derivative headers
				// ...
				for (UT_uint32 i = 0; i < m_pHeaders[m_iCurrentHeader].d.hdr.getItemCount(); i++)
				{
					header * pH = (header*)m_pHeaders[m_iCurrentHeader].d.hdr.getNthItem(i);
					UT_return_val_if_fail(pH, true);
					
					UT_String_sprintf(id,"%d",pH->pid);
					attribsS[3] = id.c_str();

					switch(pH->type)
					{
						case HF_HeaderEven:
							attribsS[1] = "header-even";
							break;
						case HF_FooterEven:
							attribsS[1] = "footer-even";
							break;
						case HF_HeaderOdd:
							attribsS[1] = "header";
							break;
						case HF_FooterOdd:
							attribsS[1] = "footer";
							break;
						case HF_HeaderFirst:
							attribsS[1] = "header-first";
							break;
						case HF_FooterFirst:
							attribsS[1] = "footer-first";
							break;
						default:
							UT_ASSERT(UT_NOT_REACHED);
					}

					getDoc()->appendStrux(PTX_SectionHdrFtr, attribsS);

					// we need to remember the HdrFtr fragment for
					// later ...
					pf_Frag * pF = getDoc()->getLastFrag();
					UT_return_val_if_fail(pF && pF->getType() == pf_Frag::PFT_Strux, true);

					pf_Frag_Strux * pFS = (pf_Frag_Strux*)pF;
					UT_return_val_if_fail(pFS->getStruxType() == PTX_SectionHdrFtr, true);

					m_pHeaders[m_iCurrentHeader].d.frag.addItem((void*)pF);
					
					getDoc()->appendStrux(PTX_Block, attribsB);
					getDoc()->appendFmt(attribsC);
					
				}
				
				return true;
			}
			else
			{
				// just gobble the character ...
				return false;
			}
			
		}

		// if we got this far, we are somwhere inside the header, just
		// process the character in a normal way
		return (m_pHeaders[m_iCurrentHeader].type != HF_Unsupported);
	}

	return true;
}

