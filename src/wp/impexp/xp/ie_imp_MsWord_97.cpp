/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ut_locale.h"

#include <zlib.h>

#include "wv.h"

#include "ut_string_class.h"
#include "ut_string.h"
#include "ut_std_string.h"
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

#include "ie_impexp_MsWord_97.h"
#include "ie_imp_MsWord_97.h"
#include "ie_impGraphic.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"

#include "pf_Frag_Strux.h"
#include "pt_PieceTable.h"
#include "pd_Style.h"

#include "fp_PageSize.h"

#include "ut_Language.h"

#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-msole-utils.h>
#include <gsf/gsf-docprop-vector.h>
#include <gsf/gsf-meta-names.h>

#ifdef DEBUG
#define IE_IMP_MSWORD_DUMP
#include "ie_imp_MsWord_dump.h"
#undef IE_IMP_MSWORD_DUMP
#endif

#define X_CheckError(v) 		do { if (!(v)) return 1; } while (0)

// undef this to disable support for older images (<= Word95)
#define SUPPORTS_OLD_IMAGES 1

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
static const gchar * s_translateStyleId(UT_uint32 id)
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
		case 4:  return "Heading 4";
		case 5:  return NULL /*"Heading 5"*/;
		case 6:  return NULL /*"Heading 6"*/;
		case 7:  return NULL /*"Heading 7"*/;
		case 8:  return NULL /*"Heading 8"*/;
		case 9:  return NULL /*"Heading 9"*/;
		case 10: return NULL /*"Index 1"*/;  /* Really a dup of 92? */
		case 11: return NULL /*"Index 2"*/;
		case 12: return NULL /*"Index 3"*/;
		case 13: return NULL /*"Index 4"*/;
		case 14: return NULL /*"Index 5"*/;
		case 15: return NULL /*"Index 6"*/;
		case 16: return NULL /*"Index 7"*/;
		case 17: return NULL /*"Index 8"*/;
		case 18: return NULL /*"Index 9"*/;
		case 19: return NULL /*"Contents 1"*/; /* Handled by insertTOC? */
		case 20: return NULL /*"Contents 2"*/; /* Handled by insertTOC? */
		case 21: return NULL /*"Contents 3"*/; /* Handled by insertTOC? */
		case 22: return NULL /*"Contents 4"*/; /* Handled by insertTOC? */
		case 23: return NULL /*"TOC 5"*/; /* See Contents above for these five as well */
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
		case 47: return NULL /*"List"*/;   //WARNING: beginPara appears to handle arbitrary lists via _mapDocToAbiList*
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
		case 90: return "Plain Text"; /* Really a dup of 109? */
		case 91: return NULL /*"Email Signature"*/;
	    case 92: return NULL /*"Index 1"*/;  /* Really a dup of 10? */
	    case 93: return NULL /*"List Bullet"*/;
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
    	case 106: return NULL /*"Comment Subject"*/;
		case 107: return NULL /*"No List"*/;
    	case 108: return NULL /*"Index Heading"*/;
	    case 109: return "Plain Text";  /* Really a dup of 90? */
	    case 110: return NULL /*"Hyperlink"*/;
	    case 111: return NULL /*"FollowedHyperlink"*/;
    	case 112: return "Numbered List"; /* Was EnumList, really a dup of 49? Closer than nothing anyway*/
    	case 115: return NULL /*"Balloon Text"*/;

		case 153: return NULL /*"Table of Authorities"*/;
		case 154: return NULL /*"Grille du tableau" in fr_FR*/;

		default:
			UT_DEBUGMSG(("Unknown style Id [%d]; Please submit this document with a bug report!\n", id));
			// Would be nice if we had a UT_USERMSG or something to put up a prompt (with a
			// don't display again option) with the message in normal mode, OutputMsg or silent
			// in commandline or docserver mode, etc.  Because it is the users, not the
			// developers who will have such alien documents.  -MG
		
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return NULL;
	}
	return NULL;
}

/*!
    Strip characters that would confuse either the xml parser or our
    property parser; caller is responsible to g_free the returned pointer
*/
static char * s_stripDangerousChars(const char *s)
{
	UT_uint32 j, k;
	if(!s)
		return NULL;
	
	char * t = (char*) g_try_malloc(strlen(s)+1);
	UT_return_val_if_fail(t,NULL);
	
	for(j = 0, k = 0; j < strlen(s); )
	{
	    if(s[j] < ' ' && s[j] >= 0 && s[j] != '\t' && s[j] != '\n' && s[j] != '\r')
	    {
	        j++;
	    }
	    else
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
	}
	
	t[k] = 0;
	
	return t;
}

static char * s_convert_to_utf8 (const wvParseStruct *ps, const char *s)
{
	// strangely wv seems to return an UTF-8 string despite a specified codepage
	// so we must ensure it is UTF-8. This is time consuming. :-(
	// If it is UTF-8 we just g_strdup() it.
	// See bug 13229.
	if (s == NULL)
		return NULL;
	if(g_utf8_validate(s, -1, NULL)) {
		return g_strdup(s);
	}
	const char * encoding = NULL;
	char fallback = '?';
	encoding = wvLIDToCodePageConverter(ps->fib.lid);
	return g_convert_with_fallback(s, -1, "UTF-8", encoding, &fallback, NULL, NULL, NULL);
}

//
// DOC uses an unsigned int color index
//
typedef UT_uint32 Doc_Color_t;

//
// A mapping between Word's colors and Abi's RGB color scheme;
// if you add colors, _make sure_ to increase the '16' in 
// sMapIcoToColor() below
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

static UT_String sMapIcoToColor (UT_uint16 ico, bool bForeground)
{
	// need to handle the automatic colour 0; see bug 10261 for bounds-check
	if((!ico && bForeground) || (ico > 16))
	{
		ico = 1;  //black
	}
	else if(!ico && !bForeground)
	{
		ico = 8;  //white
	}

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
	F_MERGEFIELD,
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
	const char * m_name;
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
	{"\\*Arabisch",F_PAGE},
	{"NUMCHARS",   F_NUMCHARS},
	{"NUMPAGES",   F_NUMPAGES},
	{"NUMWORDS",   F_NUMWORDS},
	{"MERGEFIELD", F_MERGEFIELD},
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
		if (!g_ascii_strcasecmp(s_Tokens[k].m_name,name))
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
		case 0:  
		case 1:
			return "Letter";
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
			UT_ASSERT_HARMLESS( 0 );
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
static void s_mapDocToAbiListDelim (UT_uint16 * pStr, UT_uint32 iLen, UT_UTF8String &sDelim)
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

	sDelim = sUtf8Pfx;
	sDelim += "%L";
	sDelim += sUtf8Sfx;
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
	  return "NULL";

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

// supported suffixes
static IE_SuffixConfidence IE_Imp_MsWord_97_Sniffer__SuffixConfidence[] = {
	{ "doc", 	UT_CONFIDENCE_PERFECT 	},
	{ "dot", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_MsWord_97_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_MsWord_97_Sniffer__SuffixConfidence;
}

// supported mimetypes
static IE_MimeConfidence IE_Imp_MsWord_97_Sniffer__MimeConfidence[] = {
	{ IE_MIME_MATCH_FULL, 	IE_MIMETYPE_MSWord, 		UT_CONFIDENCE_GOOD 	},
	{ IE_MIME_MATCH_FULL, 	"application/vnd.ms-word",	UT_CONFIDENCE_GOOD 	},
	{ IE_MIME_MATCH_FULL, 	"text/doc", 				UT_CONFIDENCE_GOOD 	}, // or is it? [TODO: check!]
	{ IE_MIME_MATCH_BOGUS, 	"", 						UT_CONFIDENCE_ZILCH }
};

const IE_MimeConfidence * IE_Imp_MsWord_97_Sniffer::getMimeConfidence ()
{
	return IE_Imp_MsWord_97_Sniffer__MimeConfidence;
}

UT_Confidence_t IE_Imp_MsWord_97_Sniffer::recognizeContents (GsfInput * input)
{
	GsfInfile * ole;

	ole = gsf_infile_msole_new (input, NULL);

	// invokes the old recognizeContents below, in hopes of identifying
	// pre-OLE files
	if (!ole)
		return IE_ImpSniffer::recognizeContents (input);

	UT_Confidence_t confidence = UT_CONFIDENCE_ZILCH;
	GsfInput * stream = gsf_infile_child_by_name (ole, "WordDocument");
	if (stream)
		{
			g_object_unref (G_OBJECT (stream));
			confidence = UT_CONFIDENCE_PERFECT;
		}

	g_object_unref (G_OBJECT (ole));

	return confidence;
}

UT_Confidence_t IE_Imp_MsWord_97_Sniffer::recognizeContents (const char * szBuf,
															 UT_uint32 iNumbytes)
{
	const char * magic	= 0;
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

		/* WinWord 2 */
		if (static_cast< unsigned char>(szBuf[0]) == static_cast<unsigned char>(0xdb)
			&& static_cast< unsigned char>(szBuf[1]) == static_cast<unsigned char>(0xa5)
			&& szBuf[2] == static_cast<char>(0x2d)
			&& szBuf[3] == static_cast<char>(0))
		{
			return UT_CONFIDENCE_PERFECT;
		}
	}
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
		// g_free the names from the bookmarks
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
	UT_VECTOR_PURGEALL(textboxPos *, m_vecTextboxPos);

	DELETEPV(m_pTextboxes);
	DELETEPV(m_pFootnotes);
	DELETEPV(m_pEndnotes);
	DELETEPV(m_pHeaders);
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
	m_iOverrideIssued(UT_BIDI_UNSET),
	m_bBidiMode(false),
	m_bInLink(false),
	m_pBookmarks(NULL),
	m_iBookmarksCount(0),
	m_pFootnotes(NULL),
	m_iFootnotesCount(0),
	m_pEndnotes(NULL),
	m_iEndnotesCount(0),
	m_pTextboxes(NULL),
	m_iTextboxCount(0),
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
    m_bLineBreakPending(false),
	m_bSymbolFont(false),
	m_dim(DIM_IN),
	m_iLeft(0),
	m_iRight(0),
	m_iTextboxesStart(0xffffffff),
	m_iTextboxesEnd(0xffffffff),
	m_iNextTextbox(0),
	m_iPrevHeaderPosition(0xffffffff),
	m_bEvenOddHeaders(false),
	m_bInTOC(false),
	m_bTOCsupported(false),
	m_bInTextboxes(false),
	m_pTextboxEndSection(NULL),
	m_iLeftCellPos(0),
	m_iLastAppendedHeader(0xffffffff)
{
  for(UT_uint32 i = 0; i < 9; i++)
	  m_iListIdIncrement[i] = 0;
  m_vecTextboxPos.clear();
}

/****************************************************************************/
/****************************************************************************/

#define ErrCleanupAndExit(code)  do {wvOLEFree (&ps); return(code);} while(0)

#define GetPassword() _getPassword ( XAP_App::getApp()->getLastFocussedFrame() )

#define ErrorMessage(x) do { XAP_Frame *_pFrame = XAP_App::getApp()->getLastFocussedFrame(); if ( _pFrame ) _errorMessage (_pFrame, (x)); } while (0)

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

#if 0
static void _errorMessage (XAP_Frame * pFrame, int id)
{
  UT_return_if_fail(pFrame);

  const XAP_StringSet * pSS = XAP_App::getApp ()->getStringSet ();

  const char * text = pSS->getValue (id, pFrame->getApp()->getDefaultEncoding()).c_str();

  pFrame->showMessageBox (text, XAP_Dialog_MessageBox::b_O,
						  XAP_Dialog_MessageBox::a_OK);
}
#endif

static const struct {
  const char * metadata_key;
  const char * abi_metadata_name;
} metadata_names[] = {
  { GSF_META_NAME_TITLE, PD_META_KEY_TITLE },
  { GSF_META_NAME_DESCRIPTION, PD_META_KEY_DESCRIPTION },
  { GSF_META_NAME_SUBJECT, PD_META_KEY_SUBJECT },
  { GSF_META_NAME_DATE_MODIFIED, PD_META_KEY_DATE_LAST_CHANGED },
  { GSF_META_NAME_DATE_CREATED, PD_META_KEY_DATE },
  { GSF_META_NAME_KEYWORDS, PD_META_KEY_KEYWORDS },
  { GSF_META_NAME_LANGUAGE, PD_META_KEY_LANGUAGE },
  { GSF_META_NAME_REVISION_COUNT, NULL },
  { GSF_META_NAME_EDITING_DURATION, NULL },
  { GSF_META_NAME_TABLE_COUNT, NULL },
  { GSF_META_NAME_IMAGE_COUNT, NULL },
  { GSF_META_NAME_OBJECT_COUNT, NULL },
  { GSF_META_NAME_PAGE_COUNT, NULL },
  { GSF_META_NAME_PARAGRAPH_COUNT, NULL },
  { GSF_META_NAME_WORD_COUNT, NULL },
  { GSF_META_NAME_CHARACTER_COUNT, NULL },
  { GSF_META_NAME_CELL_COUNT, NULL },
  { GSF_META_NAME_SPREADSHEET_COUNT, NULL },
  { GSF_META_NAME_CREATOR, PD_META_KEY_CREATOR },
  { GSF_META_NAME_TEMPLATE, NULL },
  { GSF_META_NAME_LAST_SAVED_BY, NULL },
  { GSF_META_NAME_LAST_PRINTED, NULL },
  { GSF_META_NAME_SECURITY, NULL },
  { GSF_META_NAME_CATEGORY, NULL },
  { GSF_META_NAME_PRESENTATION_FORMAT, NULL },
  { GSF_META_NAME_THUMBNAIL, NULL },
  { GSF_META_NAME_GENERATOR, PD_META_KEY_GENERATOR },
  { GSF_META_NAME_LINE_COUNT, NULL },
  { GSF_META_NAME_SLIDE_COUNT, NULL },
  { GSF_META_NAME_NOTE_COUNT, NULL },
  { GSF_META_NAME_HIDDEN_SLIDE_COUNT, NULL },
  { GSF_META_NAME_MM_CLIP_COUNT, NULL },
  { GSF_META_NAME_BYTE_COUNT, NULL },
  { GSF_META_NAME_SCALE, NULL },
  { GSF_META_NAME_HEADING_PAIRS, NULL },
  { GSF_META_NAME_DOCUMENT_PARTS, NULL },
  { GSF_META_NAME_MANAGER, PD_META_KEY_CONTRIBUTOR },
  { GSF_META_NAME_COMPANY, PD_META_KEY_PUBLISHER },
  { GSF_META_NAME_LINKS_DIRTY, NULL },
  { GSF_META_NAME_MSOLE_UNKNOWN_17, NULL },
  { GSF_META_NAME_MSOLE_UNKNOWN_18, NULL },
  { GSF_META_NAME_MSOLE_UNKNOWN_19, NULL },
  { GSF_META_NAME_MSOLE_UNKNOWN_20, NULL },
  { GSF_META_NAME_MSOLE_UNKNOWN_21, NULL },
  { GSF_META_NAME_MSOLE_UNKNOWN_22, NULL },
  { GSF_META_NAME_MSOLE_UNKNOWN_23, NULL },
  { GSF_META_NAME_DICTIONARY, NULL },
  { GSF_META_NAME_LOCALE_SYSTEM_DEFAULT, NULL },
  { GSF_META_NAME_CASE_SENSITIVE, NULL }
};
static const gsize nr_metadata_names = G_N_ELEMENTS(metadata_names);

struct DocAndLid
{
	PD_Document *doc;
	int lid;
};

static void
cb_print_property (char const *name, GsfDocProp const *prop, DocAndLid * doc)
{
  GValue const *val = gsf_doc_prop_get_val  (prop);

  if (! VAL_IS_GSF_DOCPROP_VECTOR ((GValue *)val)) {

	  // just scan over the table. consider optimizing if we really care to.
	  for(gsize i = 0; i < nr_metadata_names; i++) {
		  if(strcmp(metadata_names[i].metadata_key, name) == 0) {
			  char const * abi_metadata_name = metadata_names[i].abi_metadata_name;
			
			  if(abi_metadata_name != NULL) {
				  const char * encoding = NULL;
				  if (doc->lid >> 8 != 0x04) {
					// header is not utf8 encoded
				  	encoding = wvLIDToCodePageConverter(doc->lid);
				  }
				  char *tmp;

				  if (G_VALUE_HOLDS(val, G_TYPE_STRING))
					  {
						  // special-case strings. it seems that g_value_get_string()
						  // and g_strdup_value_contents() may return different things
						  // check with document from bug 11148
						  const char * contents = g_value_get_string(val);
						  
						  if (encoding && *encoding)
							  {
								  tmp = g_convert_with_fallback(contents, -1, (gchar*)"UTF-8", encoding, (gchar*)"?", NULL, NULL, NULL);
							  }
						  else
							  {
								  tmp = g_strdup(contents);
							  }
						  
					  }
				  else
					  {
						  // coerce into a string
						  tmp = g_strdup_value_contents(val);
					  }

				  char * meta = tmp;
				  // strip beginning and ending quotes
				  if(meta && strcmp(meta,"\"\"")) { // ignore '""' props
					  if(meta[0] == '"')
						  meta++;
					  int len = strlen(meta);
					  if ((len > 0) && meta[len - 1] == '"') {
						  meta[len - 1] = '\0';
					  }
					  if (*meta) {
						  doc->doc->setMetaDataProp(abi_metadata_name, meta);
					  }
				  }
				  g_free (tmp);			  
			  }
		  }
	  }
  }
}

static void print_summary_stream (GsfInfile * msole,
								  const char * stream_name,
								  int lid,
								  PD_Document * doc)
{
  GsfInput * stream = gsf_infile_child_by_name (msole, stream_name);
  if (stream != NULL) {
    GsfDocMetaData *meta_data = gsf_doc_meta_data_new ();
    GError    *err = NULL;    

    err = gsf_msole_metadata_read (stream, meta_data);
    if (err != NULL) {
      g_warning ("Error getting metadata for %s: %s", stream_name, err->message);
      g_error_free (err);
      err = NULL;
    } else {
		DocAndLid dil;

		dil.doc = doc;
		dil.lid = lid;
		gsf_doc_meta_data_foreach (meta_data,
								   (GHFunc) cb_print_property, &dil);
    }

    g_object_unref (meta_data);
    g_object_unref (G_OBJECT (stream));
  }
}

void IE_Imp_MsWord_97::_handleMetaData(wvParseStruct *ps)
{
	print_summary_stream (GSF_INFILE(ps->ole_file), "\05SummaryInformation", ps->fib.lid, getDoc());
	print_summary_stream (GSF_INFILE(ps->ole_file), "\05DocumentSummaryInformation", ps->fib.lid, getDoc());
}

UT_Error IE_Imp_MsWord_97::_loadFile(GsfInput * fp)
{
  wvParseStruct ps;

  int ret = wvInitParser_gsf(&ps, fp);
  const char * password = NULL;

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
  if(!getLoadStylesOnly())
	  getDoc()->setAttrProp(PP_NOPROPS);
  
  _handleMetaData(&ps);
  wvText(&ps);

  if(getLoadStylesOnly()) {
    wvOLEFree(&ps);
    return UT_OK;
  }

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
	  _appendStrux(PTX_Section, PP_NOPROPS);
	  m_bInSect = true;
	  m_nSections++;
	}

  pf_Frag * pF = getDoc()->getLastFrag();
  if (pF && pF->getType() == pf_Frag::PFT_Strux) {
	  pf_Frag_Strux * pFS = (pf_Frag_Strux*)pF;
	  if ((pFS->getStruxType() != PTX_Block) && (pFS->getStruxType() != PTX_EndFootnote) && (pFS->getStruxType() != PTX_EndEndnote))
		  m_bInPara = false;
  }

  if(!m_bInPara)
  {
	  // append a blank defaul paragraph - assume it works
	  UT_DEBUGMSG(("#TF: _flush: appending default block\n"));
	  _appendStrux(PTX_Block, PP_NOPROPS);
	  m_bInPara = true;
	  emObject * pObject = NULL;
	  if(m_vecEmObjects.getItemCount() > 0)
	  {
		  UT_sint32 i =0;
		  for(i=0;i< m_vecEmObjects.getItemCount(); i++)
		  {
			  pObject = m_vecEmObjects.getNthItem(i);
			  if(pObject->objType == PTO_Bookmark)
			  {
				  PP_PropertyVector propsArray = {
					  "name", pObject->props1.c_str(),
					  "type", pObject->props2.c_str()
				  };
				  _appendObject (PTO_Bookmark, propsArray);
			  }
			  else
			  {
				  UT_DEBUGMSG(("MSWord 97 _flush: Object not handled \n"));
				  UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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
		  const gchar* pProps = "props";
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

		  PP_PropertyVector propsArray = {
			  pProps, prop_basic.c_str()
		  };

		  if(m_charRevs.size())
		  {
			  propsArray.push_back("revision");
			  propsArray.push_back(m_charRevs.c_str());
		  }
		  
		  const UT_UCS4Char * p;
		  const UT_UCS4Char * pStart = m_pTextRun.ucs4_str();
		  UT_uint32 iLen = m_pTextRun.size();
		  
		  UT_BidiCharType iOverride = UT_BIDI_UNSET, cType, cLastType = UT_BIDI_UNSET, cNextType;
		  UT_uint32 iLast = 0;
		  UT_UCS4Char c = *pStart;
	
		  cType = UT_bidiGetCharType(c);
	
		  for(UT_uint32 i = 0; i < iLen; i++)
		  {
			  if(i < iLen - 1 )
			  {
				  c = *(pStart+i+1);
				  cNextType = UT_bidiGetCharType(c);
			  }
			  else
			  {
				  cNextType = UT_BIDI_UNSET;
			  }

			  if(UT_BIDI_IS_NEUTRAL(cType))
			  {
				  if(m_bLTRCharContext
					 && iOverride != UT_BIDI_LTR
					 && (cLastType != UT_BIDI_LTR || cNextType != UT_BIDI_LTR))
				  {
					  if(i - iLast > 0)
					  {
						  p = pStart + iLast;
						  if(!_appendFmt(propsArray))
							  return;

						  if(!_appendSpan(p, i - iLast))
							  return;
					  }
					  iOverride = UT_BIDI_LTR;
					  propsArray[1] = prop_ltr.c_str();
					  iLast = i;
				  }
				  else if(!m_bLTRCharContext
						  && iOverride != UT_BIDI_RTL
						  && (cLastType != UT_BIDI_RTL || cNextType != UT_BIDI_RTL))
				  {
					  if(i - iLast > 0)
					  {
						  p = pStart + iLast;
						  if(!_appendFmt(propsArray))
							  return;

						  if(!_appendSpan(p, i - iLast))
							  return;
					  }
					  iOverride = UT_BIDI_RTL;
					  propsArray[1] = prop_rtl.c_str();
					  iLast = i;
				  }
			  }
			  else
			  {
				  // strong character; if we previously issued an override,
				  // we need to cancel it
				  if(iOverride != static_cast<UT_uint32>(UT_BIDI_UNSET))
				  {
					  if(i - iLast > 0)
					  {
						  p = pStart + iLast;
						  if(!_appendFmt(propsArray))
							  return;
					
						  if(!_appendSpan(p, i - iLast))
							  return;
					  }
					  iOverride = UT_BIDI_UNSET;
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

gchar * IE_Imp_MsWord_97::_getBookmarkName(const wvParseStruct * ps, UT_uint32 pos)
{
	gchar *str;
	UT_UTF8String sUTF8;

	if(ps->Sttbfbkmk.extendedflag == 0xFFFF)
	{
		// 16 bit stuff
		const UT_UCS2Char * p = static_cast<const UT_UCS2Char *>(ps->Sttbfbkmk.u16strings[pos]);
		if(p) {
		  UT_uint32 len  = UT_UCS2_strlen(p);
		  sUTF8.clear();
		  sUTF8.appendUCS2(p, len);
		  
		  str = new gchar[sUTF8.byteLength()+1];
		  strcpy(str, sUTF8.utf8_str());
		} else
		  str = NULL;
	}
	else
	{
		// 8 bit stuff
		// there is a bug in wv, and the table gets incorrectly retrieved
		// if it contains 8-bit strings
		if(ps->Sttbfbkmk.s8strings[pos])
		{
			UT_uint32 len = strlen(ps->Sttbfbkmk.s8strings[pos]);
			str = new gchar[len + 1];
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

		m_bEvenOddHeaders = (ps->dop.fFacingPages != 0);
		
		// import styles
		_handleStyleSheet(ps);

		if(getLoadStylesOnly())
			return 1;

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
		
		m_iTextboxesStart = m_iEndnotesEnd;
		m_iTextboxesEnd = m_iTextboxesStart + ps->fib.ccpTxbx;
		UT_DEBUGMSG(("Size of all text in all textboxes %d \n", ps->fib.ccpTxbx));

		if(m_iTextboxesEnd == 0xffffffff)
			m_iTextboxesEnd = m_iTextboxesStart;
		UT_DEBUGMSG(("  Found %d Positioned TextBoxes \n",ps->nooffspa));
		// now retrieve the note info ...
		_handleNotes(ps);
		_handleHeaders(ps);
		_handleTextBoxes(ps);
		
		if(m_iAnnotationsEnd != m_iAnnotationsStart)
			{
				UT_DEBUGMSG(("Annotations of length %d in this doc \n",m_iAnnotationsEnd - m_iAnnotationsStart));
			}
		UT_DEBUGMSG(("Fnotes [%d,%d], Enotes [%d,%d]\n",
					 m_iFootnotesStart, m_iFootnotesEnd, m_iEndnotesStart, m_iEndnotesEnd));

		///////////////////////////////////////////////////////////////////////////////
		// Set various revision states
		//
		// unlike Word:
		// 
		//     * we do not differentiate between screen and print: we
		//       print whatever is on screen
		//
		//     * if show revisions is off, Word shows what the
		//       document looks like _after_ the last revision; by
		//       default we show what it looked _before_ first
		//       revision; we can show the post-revision state by
		//       setting the view id to PD_MAX_REVISION
		//
		//     * we currently do not handle the fLockRev parameter
		{
			bool bShow = ps->dop.fRMView == 1 || ps->dop.fRMPrint == 1;
		
			getDoc()->setShowRevisions(bShow);

			if(!bShow)
			{
				getDoc()->setShowRevisionId(PD_MAX_REVISION);
			}
		
			getDoc()->setMarkRevisions(ps->dop.fRevMarking == 1);
		}
		
		break;
		
	case DOCEND:
		// we want to clean up fmt marks
		getDoc()->purgeFmtMarks();
		break;
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

	PP_PropertyVector propsArray = {
		"name", bm->name,
		"type", bm->start ? "start" : "end"
	};

	if(m_bInTable && !m_bCellOpen)
	{
		emObject * pObject = new emObject;
		pObject->props1 = propsArray[1];
		pObject->objType = PTO_Bookmark;
		pObject->props2 = propsArray[3];
		m_vecEmObjects.addItem(pObject);
	}
	else
	{
//
// Bookmarks need to be preceded by Blocks
//
		pf_Frag * pf = getDoc()->getLastFrag();
		while(pf && pf->getType() != pf_Frag::PFT_Strux)
		{
			pf = pf->getPrev();
		}
		if(pf && (pf->getType() == pf_Frag::PFT_Strux) )
		{
			pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
			if(pfs->getStruxType() != PTX_Block)
			{
				getDoc()->appendStrux(PTX_Block, PP_NOPROPS);
			}
		}
		else if( pf == NULL)
		{
			getDoc()->appendStrux(PTX_Block, PP_NOPROPS);
		}

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
	bookmark * bm, * lastBm;
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

	   lastBm = &m_pBookmarks[m_iBookmarksCount];

	   while(bm < lastBm && bm->pos == iDocPosition)
		  error |= _insertBookmark(bm++);
	}
	return error;
}

int IE_Imp_MsWord_97::_charProc (wvParseStruct *ps, U16 eachchar, U8 chartype, U16 lid)
{
	// make sure we are not past the end of the document ...
	// this can happen with some complex documents
	if(ps->currentcp >= m_iTextboxesEnd)
	{
		UT_DEBUGMSG(("IE_Imp_MsWord_97::_charProc: processing past end of document !!! %d \n",ps->currentcp ));
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

	// reset the page break tracker
	if(m_bLineBreakPending)
	{
		// we have a line break pending
		this->_appendChar (UCS_LF);
		m_bLineBreakPending = false;
	}
	
	if(!_handleHeadersText(ps->currentcp,true))
		return 0;
	if(!_handleNotesText(ps->currentcp))
		return 0;
	if(!_handleTextboxesText(ps->currentcp))
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
	  this->_flush();
	  // see bug 9370
	  // <delackner> aaah actually, Cocoa's writer is *definitely* broken
	  // <delackner> ms word thinks the second para is part of the first, but broken with a non-paragraph-breaking-line-break
	  // so we'll treat this like msword does
	  m_bLineBreakPending = true;
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

	// see bug 9370. we probably got a char 13, but no open paragraph.
	if(!m_bInPara) {
	  this->_appendChar (UCS_LF);
	  _flush();
	}
	
	this->_appendChar (static_cast<UT_UCSChar>(eachchar));

	return 0;
}

int IE_Imp_MsWord_97::_specCharProc (wvParseStruct *ps, U16 eachchar, CHP *achp)
{
	// make sure we are not past the end of the document ...
	// this can happen with some complex documents
	if(ps->currentcp >= m_iTextboxesEnd)
	{
		UT_DEBUGMSG(("IE_Imp_MsWord_97::_specCharProc: processing past end of document !!!\n"));
		return 0;
	}
	
	Blip blip;
	long pos;
	FSPA * fspa;
	//FDOA * fdoa;
#ifdef SUPPORTS_OLD_IMAGES
	wvStream *fil;
	PICF picf;
#endif

	if(!_handleHeadersText(ps->currentcp,true))
		return 0;

	if(!_handleNotesText(ps->currentcp))
		return 0;

	if(!_handleTextboxesText(ps->currentcp))
		return 0;

	// insert any required bookmarks, but only if we are not in a
	// field ...
	if(!ps->fieldstate)
		_insertBookmarkIfAppropriate(ps->currentcp);
	
	if(_insertNoteIfAppropriate(ps->currentcp,0))
		return 0;

	if(eachchar == 0x28)
	{
		// this is a symbol; the font is identified by achp->ftcSym and the char code is
		// achp->xchSym
		this->_appendChar(achp->xchSym);
		return 0;
	}
	
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
			UT_DEBUGMSG(("Field has an associated embedded OLE object\n"));
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
				this->_handleImage(&blip, picf.mx * picf.dxaGoal / 1000, picf.my * picf.dyaGoal / 1000, picf.dyaCropTop, picf.dyaCropBottom, picf.dxaCropLeft, picf.dxaCropRight);
			}
			else
			{
				UT_DEBUGMSG(("Dom: no graphic data\n"));
			}

			wvStream_goto(ps->data, pos);

			return 0;
		}
		else
		{
			UT_DEBUGMSG(("Couldn't import graphic!\n"));
			return 0;
		}
#else
		UT_DEBUGMSG(("DOM: 0x01 graphics support is disabled at the moment\n"));
		wvStream_goto(ps->data, pos);

		return 0;
#endif
		break;
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
				UT_DEBUGMSG(("Found a psfa! \n"));
				double dLeft,dRight,dTop,dBottom = 0.0;
				dLeft = static_cast<double>(fspa->xaLeft)/1440.0;
				dRight = static_cast<double>(fspa->xaRight)/1440.0;
				dTop = static_cast<double>(fspa->yaTop)/1440.0;
				dBottom = static_cast<double>(fspa->yaBottom)/1440.0;
				UT_DEBUGMSG(("Left %f Right %f Top %f Bottom %f \n",dLeft,dRight,dTop,dBottom));
				UT_DEBUGMSG(("spid %d cTxbx %d \n",fspa->spid,fspa->cTxbx));
				UT_DEBUGMSG(("fHdr %d bx %d by %d wr %d wrk %d fRcaSimple %d fBelowText %d fAnchorLock %d \n",fspa->fHdr,fspa->bx,fspa->by,fspa->wr,fspa->wrk,fspa->fRcaSimple,fspa->fBelowText,fspa->fAnchorLock));
				UT_String sImageName;
				bool bPositionObject = false;
				if (wv0x08(&blip, fspa->spid, ps))
				{
//
// FIXME! Put some code in here to make this use Sectionframes!!
//
					UT_DEBUGMSG(("!!!!Found a blip in a fspa!!!!!!!!!! \n"));
					if(UT_OK == this->_handlePositionedImage(&blip, sImageName))
					   bPositionObject = true;
				}
				bool isTextBox = false;
				UT_uint32 textOff = 0;
				UT_uint32 i;
				escherstruct item;
				FSPContainer *answer = NULL;

				UT_DEBUGMSG(("IE_Imp_MsWord_97:: escher: ps->fib.fcDggInfo %d ps->fib.lcbDggInfo %d \n", ps->fib.fcDggInfo,ps->fib.lcbDggInfo));
				wvGetEscher (&item, ps->fib.fcDggInfo, ps->fib.lcbDggInfo, ps->tablefd,
							 ps->mainfd);
				for (i = 0; i < item.dgcontainer.no_spgrcontainer; i++)
				{
					answer = wvFindSPID (&(item.dgcontainer.spgrcontainer[i]), fspa->spid);
					if (answer)
					{
						break;
					}
				}
				if(answer != NULL)
				{
					ClientTextbox cTextBox = answer->clienttextbox;
					if(cTextBox.textid != NULL)
					{
						isTextBox = true;
						textOff = *cTextBox.textid;
						UT_DEBUGMSG(("Found a Text box! text offset is.. %d \n",textOff));
					}
                    // passing struct to format parameter. WTF?
					xxx_UT_DEBUGMSG((" clienttextbox %x clientdata %x \n",answer->clienttextbox,answer->clientdata));
				}
				if(isTextBox || bPositionObject)
				{
//				if(answer != NULL)
//				{
					const char * atts[] = {NULL,NULL,NULL,NULL,NULL,NULL};
					if(bPositionObject && sImageName.size())
					{
					  atts[0] =  PT_STRUX_IMAGE_DATAID;
					  atts[1] = sImageName.c_str();
					  atts[2] = "props";
					}
					else
					{
					  atts[0] = "props";
					}
					std::string sProp;
					std::string sProps;
					std::string sVal;
					sProps = "frame-type:";
					if(isTextBox)
					{
					  sProps += "textbox; ";
					}
					else
					{
					  sProps += "image; ";
					}
					sProps += "position-to:";
					if(fspa->by ==2)
					{
						sVal = "block-above-text; ";
					}
					else if(fspa->by ==0)
					{
						sVal = "column-above-text; ";
					}
					else if(fspa->by ==1)
					{
						sVal = "page-above-text; "; // should be page-above-text
					}
					sProps += sVal;
					sProps += "wrap-mode:";
					if(fspa->wr == 3)
					{
					  sVal = "above-text; ";
					}
					else
					{
						sVal = "wrapped-both; ";
					}
					if(fspa->fBelowText == 1 && fspa->wr == 3)
					{
						UT_DEBUGMSG(("Set Below Text \n"));
						sVal = "below-text; ";
					}
					sProps += sVal;

					sProps += UT_std_string_sprintf("xpos:%fin; ", dLeft);
					sProps += UT_std_string_sprintf("ypos:%fin; ", dTop);
					sProps += UT_std_string_sprintf("frame-col-xpos:%fin; ",
													dLeft);
					sProps += UT_std_string_sprintf("frame-col-ypos:%fin; ",
													dTop);

					sProps += UT_std_string_sprintf("frame-width:%fin; ",
													dRight-dLeft);

					UT_DEBUGMSG(("Inserting Frame of width %s \n",sVal.c_str()));
					sProps += UT_std_string_sprintf("frame-height:%fin",
													dBottom-dTop);
//
// Turn off the borders.
//
					if(bPositionObject && !isTextBox)
					{
					  sProp = "top-style";
					  sVal = "none";
					  UT_std_string_setProperty(sProps, sProp, sVal);
					  sProp = "right-style";
					  UT_std_string_setProperty(sProps, sProp, sVal);
					  sProp = "left-style";
					  UT_std_string_setProperty(sProps, sProp, sVal);
					  sProp = "bot-style";
					  UT_std_string_setProperty(sProps, sProp, sVal);
					}
					if(bPositionObject)
					{
					  atts[3] = sProps.c_str();
					}
					else
					{
					  atts[1] = sProps.c_str();
					}
					PP_PropertyVector vatts = PP_std_copyProps(atts);
					_appendStrux(PTX_SectionFrame, vatts);
					_appendStrux(PTX_EndFrame, vatts);
					if(isTextBox)
					{
					  textboxPos * pPos = new textboxPos;
					  pPos->lid = fspa->spid;
					  PT_DocPosition posEnd =0;
					  getDoc()->getBounds(true,posEnd); // clean frags!

					  pPos->endFrame = getDoc()->getLastFrag();
					  m_vecTextboxPos.addItem(pPos);
					}
					wvReleaseEscher (&item);
					return true;
				}
				wvReleaseEscher (&item);
			}
			else
			{
				xxx_UT_DEBUGMSG(("nooffspa was <= 0 -- ignoring"));
			}
		}
		else
		{
			UT_DEBUGMSG(("pre Word8 0x08 graphic -- unsupported at the moment"));
			/*fdoa =*/ wvGetFDOAFromCP(ps->currentcp, NULL, ps->fdoapos,
								   ps->nooffdoa);

			// TODO: do something with the data in this fdoa someday...
		}

		return 0;
	}

	return 0;
}

int IE_Imp_MsWord_97::_beginComment(wvParseStruct * /*ps*/, UT_uint32 /*tag*/,
					void * /*props*/, int /*dirty*/)
{
  UT_DEBUGMSG(("DOM: begin comment\n"));
  return 0;
}

int IE_Imp_MsWord_97::_endComment(wvParseStruct * /*ps*/, UT_uint32 /*tag*/,
				  void * /*props*/, int /*dirty*/)
{
  UT_DEBUGMSG(("DOM: begin comment\n"));
  return 0;
}


int IE_Imp_MsWord_97::_eleProc(wvParseStruct *ps, UT_uint32 tag,
							   void *props, int dirty)
{
	// make sure we are not past the end of the document ...
	// this can happen with some complex documents
	if(ps->currentcp >= m_iTextboxesEnd)
	{
		UT_DEBUGMSG(("IE_Imp_MsWord_97::_eleProc: processing past end of document !!! %d \n",ps->currentcp >= m_iTextboxesEnd));
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

int IE_Imp_MsWord_97::_beginSect (wvParseStruct * /*ps*/, UT_uint32 /*tag*/,
				  void *prop, int /*dirty*/)
{
	SEP * asep = static_cast <SEP *>(prop);

	const gchar * propsArray[15];  
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
											  (static_cast<double>(asep->dxaColumns) / 1440)));
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
											  (static_cast<double>(asep->dzaGutter) / 1440)));
	props += propBuffer;

	//
	// TODO: section breaks
	//

	// page-margin-left
	UT_String_sprintf(propBuffer, "page-margin-left:%s;",
			UT_convertInchesToDimensionString(m_dim,
											  (static_cast<double>(asep->dxaLeft) / 1440)));
	props += propBuffer;

	// page-margin-right
	UT_String_sprintf(propBuffer, "page-margin-right:%s;",
			UT_convertInchesToDimensionString(m_dim,
											  (static_cast<double>(asep->dxaRight) / 1440)));
	props += propBuffer;

	// page-margin-top
	UT_String_sprintf(propBuffer, "page-margin-top:%s;",
			UT_convertInchesToDimensionString(m_dim,
											  (static_cast<double>(asep->dyaTop) / 1440)));
	props += propBuffer;

	// page-margin-bottom
	UT_String_sprintf(propBuffer, "page-margin-bottom:%s;",
			UT_convertInchesToDimensionString(m_dim,
											  (static_cast<double>(asep->dyaBottom)/1440)));
	props += propBuffer;

	// page-margin-header
	UT_String_sprintf(propBuffer, "page-margin-header:%s;",
			UT_convertInchesToDimensionString(m_dim,
											  (static_cast<double>(asep->dyaHdrTop)/1440)));
	props += propBuffer;

	// page-margin-footer (word's footer is measured from the bottom
	// edge of the page -- contrary to the docs -- our's from the
	// bottom margin of the page)
	double dFooter = static_cast<double>(asep->dyaBottom) - static_cast<double>(asep->dyaHdrBottom);
	if(dFooter < 0)
	{
		dFooter = -dFooter;
	}
	dFooter = dFooter/1440.;
	UT_String_sprintf(propBuffer, "page-margin-footer:%s",
					  UT_convertInchesToDimensionString(m_dim,dFooter));
	props += propBuffer;
	xxx_UT_DEBUGMSG (("DOM:SEVIOR the section properties are: '%s'\n", props.c_str()));

	
	propsArray[0] = static_cast<const gchar *>("props");
	propsArray[1] = static_cast<const gchar *>(props.c_str());

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

			// if this is a first page hdr/ftr we only use it if appropriate
			if(   (m_pHeaders[i].type == HF_HeaderFirst && !asep->fTitlePage)
			   || (m_pHeaders[i].type == HF_FooterFirst && !asep->fTitlePage))
			{
				// we want to change the type to unsupported to stop it from being
				// inserted into the document
				m_pHeaders[i].type = HF_Unsupported;
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
					UT_ASSERT_HARMLESS(UT_NOT_REACHED);
			}

			UT_String_sprintf(id[iId],"%d",m_pHeaders[k].pid);
			propsArray[iOff++] = id[iId++].c_str();
		}
	}
	
	propsArray[iOff++] = 0;
	UT_return_val_if_fail(iOff <= sizeof(propsArray), 1);
	

	if (!_appendStrux(PTX_Section, PP_std_copyProps(&propsArray[0])))
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
		if (!_appendStrux(PTX_Block, PP_NOPROPS))
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

	// we never appended a paragraph inside of this section. we're naughty. correct that here.
	if (!m_bInPara  && !m_bInTextboxes)
		_appendStrux(PTX_Block, PP_NOPROPS);

	// if there is a pending page break it belongs to the section and
	// is to be removed, we just need to set the tracker to false
	m_bPageBreakPending = false;
	m_bLineBreakPending = false;

	m_bInSect = false;
	m_bInPara = false; // reset paragraph status
	return 0;
}

int IE_Imp_MsWord_97::_beginPara (wvParseStruct *ps, UT_uint32 /*tag*/,
				  void *prop, int /*dirty*/)
{

	// if in a header of unsupported type, just return
	// the +1 is to account for the fact that ps->currentcp applies to the previous
	// char position ...
	if(_ignorePosition(ps->currentcp + 1))
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
	   (ps->currentcp == m_iMacrosStart - 1 && m_iMacrosEnd > m_iMacrosStart) ||
	   (ps->currentcp == m_iTextboxesStart - 1 && m_iTextboxesEnd > m_iTextboxesStart))
	{
		bDoNotInsertStrux  = true;
	}
	bool bInHdrFtr = false;
	if((ps->currentcp+1 >= m_iHeadersStart) && (ps->currentcp < m_iHeadersEnd))
	{
		bInHdrFtr = true;
	}
	bool bInTextboxes = false;
	if((ps->currentcp+1 >= m_iTextboxesStart) && (ps->currentcp < m_iTextboxesEnd))
	{
		bInTextboxes = true;
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
	  if (apap->fInTable) 
	  {
		  // we have to call this unconditionally, since m_bInHeaders set does not mean that
		  // the HdrFtr strux for this section has been inserted.
		  _handleHeadersText(ps->currentcp +1, false);
		  _handleTextboxesText(ps->currentcp+1);
		  if (!m_bInTable) 
		  {
			  m_bInTable = true;
			  _table_open();
//
// Fill Column positions
//
			  UT_sint32 i= 0;
			  for(i=0;i < ps->nocellbounds; i++) 
			  {
				  if(ps->cellbounds)
				  {
					  UT_sint32 pos = ps->cellbounds[i];
					  m_vecColumnPositions.addItem(pos);
				  }
			  }
		  }

		  if (ps->endcell) 
		  {
			  ps->endcell = 0;
			  _cell_close();
			  if (m_iCellsRemaining > 0) 
			  {
				  m_iCellsRemaining--;
				  if (m_iCellsRemaining == 0) 
				  {
					  _row_close();
				  }
			  }
		  }

	    _row_open(ps);

	    // determine column spans
	    if (!m_bCellOpen) 
		{
			m_vecColumnSpansForCurrentRow.clear();

			xxx_UT_DEBUGMSG(("Number of cell bounds in New row %d \n",ps->nocellbounds));
			UT_sint32 column =1;
			UT_sint32 i =0;
			UT_sint32 posLeft = 0;
			UT_sint32 posRight =0;
			if (ps->cellbounds)
				posLeft = ps->cellbounds[0];
			for (column = 1; column < ps->nocellbounds; column++) 
			{
				int span = 0;
				posRight = apap->ptap.rgdxaCenter[column];
				xxx_UT_DEBUGMSG(("column %d posLeft %d posRight %d \n",column,posLeft,posRight));
				for (i = 0; i < ps->nocellbounds; i++) 
				{
					if (ps->cellbounds[i] >= posLeft && ps->cellbounds[i] < posRight) 
					{
						span++;
					}
					else if (ps->cellbounds[i] >= posRight)
					{
						break;
					}
				}
				xxx_UT_DEBUGMSG(("COlumn %d has span %d \n",column,span));
				m_vecColumnSpansForCurrentRow.addItem(span);
				posLeft = posRight;
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
		xxx_UT_DEBUGMSG(("m_bInPara set true here -1 \n"));
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
		_appendStrux(PTX_Block, PP_NOPROPS);
		UT_UCSChar ucs = UCS_FF;
		_appendSpan(&ucs,1);
	}

	m_charProps.clear();
	m_charStyle.clear();
	m_paraProps.clear();
	m_paraStyle.clear();
	_generateParaProps(m_paraProps, apap, ps);

	/* lists */
	UT_uint32 myListId = 0;
	UT_uint32 iAWListId = UT_UID_INVALID;
	std::string sLevel, sListId, sParentId;

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
		UT_sint32 k;
		for(k = 0; k < m_vListIdMap.getItemCount(); k+=2)
		{
			if((UT_uint32)m_vListIdMap.getNthItem(k) == myListId)
			{
				iAWListId = m_vListIdMap.getNthItem(k+1);
				break;
			}
		}
		
		if(iAWListId == UT_UID_INVALID)
		{
			iAWListId = getDoc()->getUID(UT_UniqueId::List);
			UT_ASSERT_HARMLESS(iAWListId != UT_UID_INVALID);

			m_vListIdMap.addItem(myListId);
			m_vListIdMap.addItem(iAWListId);
		}

		UT_String propBuffer;

		// parent id
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

		// list delimiter
		UT_UTF8String sDelim;
		s_mapDocToAbiListDelim (apap->linfo.numberstr,apap->linfo.numberstr_size,sDelim);
		char * t = s_stripDangerousChars(sDelim.utf8_str());
		std::string sDlm = t;
		FREEP(t);

		// generate character props for the number
		// TODO -- the properties represented by apap->linfo.chp need
		// to be applied to the list number/bulet. For now, I am going
		// to translate these into a regular props string and attach
		// them to the list attributes, but they need to be passed
		// somehow down to the number field (may need a dedicated
		// _generateListCharProps() for this
		// Tomas, May 12, 2003
		UT_String szNumberProps;
		_generateCharProps(szNumberProps, &apap->linfo.chp, ps);

		std::string startValue = UT_std_string_sprintf("%d", apap->linfo.start);
		sLevel = UT_std_string_sprintf("%d", apap->ilvl + 1); // Word level starts at 0, Abi's at 1
		sListId = UT_std_string_sprintf("%d", iAWListId);
		sParentId = UT_std_string_sprintf("%d", myParentID);
		const PP_PropertyVector list_atts = {
			"id", sListId,
			"parentid", sParentId,
			"type", s_mapDocToAbiListId (static_cast<MSWordListIdType>(apap->linfo.format)),
			"start-value", startValue,
			"list-delim", std::move(sDlm),
			"level", sLevel,
			"props", szNumberProps.c_str()
		};

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
		m_paraProps += startValue;
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
	PP_PropertyVector propsArray = {
		"props", m_paraProps.c_str()
	};

	// level, or 0 for default, normal level
	if (myListId > 0)
	{
		propsArray.push_back("level");
		propsArray.push_back(sLevel);
		propsArray.push_back("listid");
		propsArray.push_back(sListId);
		propsArray.push_back("parentid");
		propsArray.push_back(sParentId);
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
			propsArray.push_back("style");

			char * t = NULL;
			const gchar * pName = NULL;
			if(pSTD)
				pName = s_translateStyleId(pSTD[apap->istd].sti);

			if(pName)
			{
				m_paraStyle = pName;
			}
			else if(pSTD)
			{
				t = s_convert_to_utf8(ps,pSTD[apap->istd].xstzName);
				m_paraStyle = t;
			}

			FREEP(t);
			propsArray.push_back(m_paraStyle.c_str());
		}
	}

	if (!m_bInSect && !bDoNotInsertStrux)
	{
		// check for should-be-impossible case
		UT_ASSERT_NOT_REACHED();
		_appendStrux(PTX_Section, PP_NOPROPS);
		m_bInSect = true ;
	}

	if(!bDoNotInsertStrux)
	{
		xxx_UT_DEBUGMSG(("_beginPara: pos %d [text ends %d]\n", ps->currentcp, m_iFootnotesStart));

		if (!_appendStrux(PTX_Block, propsArray))
		{
			UT_DEBUGMSG(("DOM: error appending paragraph block\n"));
			return 1;
		}
		m_bInPara = true;
	}

	if (myListId > 0 && !bDoNotInsertStrux)
	  {
		// TODO: honor more props
		PP_PropertyVector list_field_fmt = {
			"type", "list_label",
			"props", "text-decoration:none",
		};
		_appendObject(PTO_Field, list_field_fmt);
		m_bInPara = true;

		PP_PropertyVector attribs = {
			"props", "text-decoration:none"
		};
		// the character following the list label - 0=tab, 1=space, 2=none
		if(apap->linfo.ixchFollow == 0) // tab
		{
			getDoc()->appendFmt(attribs);
			UT_UCSChar tab = UCS_TAB;
			_appendSpan(&tab, 1);
		}
		else if(apap->linfo.ixchFollow == 1) // space
		{
			getDoc()->appendFmt(attribs);
			UT_UCSChar space = UCS_SPACE;
			_appendSpan(&space, 1);
		}
		// else none
	  }

	return 0;
}

int IE_Imp_MsWord_97::_endPara (wvParseStruct * /*ps*/, UT_uint32 /*tag*/,
								void * /*prop*/, int /*dirty*/)
{
	xxx_UT_DEBUGMSG(("#DOM: _endPara\n"));
	// have to flush here, otherwise flushing later on will result in
	// an empty paragraph being inserted

	this->_flush ();
	m_bInPara = false;
	m_bLineBreakPending = false;
	
	return 0;
}

int IE_Imp_MsWord_97::_beginChar (wvParseStruct *ps, UT_uint32 /*tag*/,
								  void *prop, int /*dirty*/)
{
	// if in a header of unsupported type, just return
	// the +1 is to account for the fact that ps->currentcp applies to the previous
	// char position ...
	if(_ignorePosition(ps->currentcp + 1))
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

	const gchar * propsArray[7];
	UT_uint32 propsOffset = 0;

	m_charProps.clear();
	m_charStyle.clear();

	UT_uint32 iFontType = 0;
	if(achp->xchSym && ps->fonts.ffn)
	{
		// inserting a symbol char ...
		iFontType = ps->fonts.ffn[achp->ftcSym].chs;
	}
	else if(ps->fonts.ffn && (achp->ftcAscii < ps->fonts.nostrings))
	{
		iFontType = ps->fonts.ffn[achp->ftcAscii].chs;
	}
	
	if(iFontType == 0)
		m_bSymbolFont = false;
	else if(iFontType == 2)
		m_bSymbolFont = true;
	else
	{
		xxx_UT_DEBUGMSG(("IE_Imp_MsWord_97::_beginChar: unknow font encoding %d\n",
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

	propsArray[propsOffset++] = static_cast<const gchar *>("props");
	propsArray[propsOffset++] = static_cast<const gchar *>(m_charProps.c_str());

	if(!m_bEncounteredRevision && (achp->fRMark || achp->fRMarkDel))
	{
		// revision "hack" - add a single revision for all revisioned text
		UT_UCS4String revisionStr ("msword_revisioned_text");
		getDoc()->addRevision(1, revisionStr.ucs4_str(), revisionStr.size(), 0, 0);
		m_bEncounteredRevision = true;
	}

	if (achp->fRMark)
	{
	    propsArray[propsOffset++] = static_cast<const gchar *>("revision");
		m_charRevs = "1";
	    propsArray[propsOffset++] = m_charRevs.c_str();
	}
	else if (achp->fRMarkDel)
	{
	    propsArray[propsOffset++] = static_cast<const gchar *>("revision");
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
			propsArray[propsOffset++] = static_cast<const gchar *>("style");
			char * t = NULL;
			const gchar * pName = s_translateStyleId(pSTD[achp->istd].sti);
		
			if(pName)
			{
				m_charStyle = pName;
			}
			else
			{
				m_charStyle = t = s_convert_to_utf8(ps,pSTD[achp->istd].xstzName);
			}

			FREEP(t);
			propsArray[propsOffset++] = m_charStyle.c_str();
		}
	}

	// woah - major error here
	if(!m_bInSect && !bDoNotAppendFmt)
	{
		UT_ASSERT_NOT_REACHED();
		_appendStrux(PTX_Section, PP_NOPROPS);
		m_bInSect = true ;
	}

	if(!m_bInPara && !bDoNotAppendFmt)
	{
		UT_ASSERT_NOT_REACHED();
		_appendStrux(PTX_Block, PP_NOPROPS);
		m_bInPara = true ;
	}

	if(!bDoNotAppendFmt)
	{
		if (!_appendFmt(PP_std_copyProps(propsArray)))
		{
			UT_DEBUGMSG(("DOM: error appending character formatting\n"));
			return 1;
		}
	}
	
	return 0;
}

int IE_Imp_MsWord_97::_endChar (wvParseStruct * /*ps*/, UT_uint32 /*tag*/,
								void * /*prop*/, int /*dirty*/)
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
					if(_isTOCsupported(f))
					{
						break;
					}
					
					// for unsuported TOCs fall through ...
					
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

		try
		{		
			f = new field;
		}
		catch(...)
		{
			f = NULL;
		}

		UT_return_val_if_fail(f,0);
		f->fieldWhich = f->command;
		f->command[0] = 0;
		f->argument[0] = 0;
		f->fieldI = 0;
		f->fieldRet = 1;
		f->type = F_OTHER;
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

bool IE_Imp_MsWord_97::_handleFieldEnd (char *command, UT_uint32 /*iDocPosition*/)
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

	if(m_bInTOC && m_bTOCsupported && (   f->type == F_TOC
									   || f->type == F_TOC_FROM_RANGE))
	{
		// end of TOC field in a supported TOC; we do nothing, since the field has already
		// been processed in _handleFieldCommand()
		m_bInTOC = false;
		m_bTOCsupported = false;
		return _insertTOC(f);
	}

	if(m_bInTOC && m_bTOCsupported)
	{
		// end of some non-TOC field inside supported TOC; just return
		return true;
	}

	command++;
	token = strtok (command, "\t, ");
	
	while(token)
	{
		tokenIndex = s_mapNameToField (token);
		switch (tokenIndex)
		{
		    case F_MERGEFIELD:
			{
				PP_PropertyVector atts = {
					"type", "mail_merge",
					"param"
				};

				token = strtok (NULL, "\"\" ");

				UT_return_val_if_fail(f->argument[f->fieldI - 1] == 0x15, false);
				
				f->argument[f->fieldI - 1] = 0;
				UT_UCS2Char * a = f->argument;

				UT_UTF8String param;
				
				if(*a == 0x14)
					{
						a++;
					}

				while(*a)
					{
						if (!((171 == *a) || (187 == *a))) {
							// @argument looks like <<FieldName>>.
							// strip off the '<<' (171) and '>>' (187)
							param.appendUCS2(a, 1);
						}

						a++;
					}

				atts.push_back(param.utf8_str());

				if (!_appendObject (PTO_Field, atts))
					{
						UT_DEBUGMSG(("Dom: couldn't append field (type = '%s')\n", atts[1].c_str()));
					}
			}
			break;

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
						_appendStrux(PTX_Block, PP_NOPROPS);
						m_bInPara = true ;
					}

					_appendObject(PTO_Hyperlink, PP_NOPROPS);
					m_bInLink = false;
					break;
				}
			case F_TOC:             
			case F_TOC_FROM_RANGE:
				// we only get here for unsupported TOC types, in which case we dump the field
				// result (not ideal, since often the PAGEREF fields inside the TOC have not been
				// updated before save and so we get 'bookmark not found' instead of page numbers,
				// but it is better than nothing at all)
				
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

/*!
    Word has several different toc tables (TOC, TOA, indexes); at the moment we only
    support TOC and even than only if it is based on heading styles
*/
bool IE_Imp_MsWord_97::_isTOCsupported(field *f)
{
	UT_return_val_if_fail(f,false);

	if(   f->type != F_TOC
	   && f->type != F_TOC_FROM_RANGE
	  )
	{
		return false;
	}
	
	bool bRet = true;
	char * command = wvWideStrToMB (f->command);
	UT_DEBUGMSG(("IE_Imp_MsWord_97::_isTOCsupported: command %s\n", command));

	char * params = NULL;

	if(f->type == F_TOC)
	{
		params = command + 5;
	}
	else if(f->type == F_TOC_FROM_RANGE)
	{
		params = command + 4;
	}
	
	// we only support the heading based TOC for now
	char * t = strstr(params, "\\o");

	if(!t)
		t = strstr(params, "\\t");

	if(!t)
	{
		bRet = false;
		goto finish;
	}

 finish:
	FREEP(command);
	return bRet;
}



/*!
   returns true if the TOC has been handled, false if the TOC type is unsupported
*/

/* Does this handle the contents styles indirectly via inserting the TOC as new and
	letting the default/initial pt code handle it like new rather than actually importing it? */

bool IE_Imp_MsWord_97::_insertTOC(field *f)
{
	UT_return_val_if_fail(f,false);
	bool bRet = true;
	bool bSupported = false;

	UT_sint32 i = 0, i1 = 0, i2 = 0;
	char * t = NULL, * t1 = NULL, * t2 = NULL;
	UT_UTF8String sProps = "toc-has-heading:0;", sTemp, sLeader;

	const gchar * attrs [3] = {"props", NULL, NULL};

	char * command = wvWideStrToMB (f->command);
	UT_DEBUGMSG(("IE_Imp_MsWord_97::_insertTOC: command %s\n", command));

	char * params = NULL;
	
	if(f->type == F_TOC)
	{
		params = command + 5;
	}
	else if(f->type == F_TOC_FROM_RANGE)
	{
		params = command + 4;
	}
	else
	{
		bRet = false;
		goto finish;
	}

	if((t = strstr(params, "\\p")))
	{
		// this defines the leader, we parse it first, before we mess up the command
		t1 = strchr(t, '\"');
		if(t1)
		{
			t1++;

			// AW can only use one of the chars (there are up to 5), we will take the first
			switch(*t1)
			{
				default: // not sure, we will treat this as a dot
				case '.': sLeader += "dot";       break;
				case '-': sLeader += "hyphen";    break;
				case '_': sLeader += "underline"; break;
				case ' ': sLeader += "none"; break;
			}
		}
	}

	if((t = strstr(params, "\\b")))
	{
		// a bookmark restricts the range from which the TOC is built
		t1 = strchr(t, '\"');
		if(t1)
		{
			t1++;

			t2 = strchr(t1, '\"');

			char c = *t2;
			*t2 = 0;

			sProps += "toc-range-bookmark:";
			sProps += t1;
			sProps += ";";

			*t2 = c; // restore the string
		}
	}

	if((t = strstr(params, "\\o")))
	{
		// heading-based TOC
		// \o param specifies a range of headings to use, e.g., \o "2-4"
		bSupported = true;
		
		t = strchr(t, '\"');
	
		if(!t)
		{
			bRet = false;
			goto finish;
		}

		t++;

		i1 = atoi(t);

		if(!i1)
		{
			bRet = false;
			goto finish;
		}

		t1 = strchr(t, '-');
		t2 = strchr(t, '\"');

		t = UT_MIN(t1, t2);
	
		if(!t)
		{
			bRet = false;
			goto finish;
		}

		i2 = 0;
		if(*t == '\"')
		{
			i2 = i1;
		}
		else
		{
			UT_ASSERT_HARMLESS( *t == '-');
			t++;
			i2 = atoi(t);
		}
	
		if(!i2)
		{
			bRet = false;
			goto finish;
		}
		// now create our TOC attr/props
		//
		// * we do not need to set the source styles, because the Heading
		//   styles are the AW default
		//   
		// * we do have to set the dest styles
		//
		// * I am not sure what to do about toc-id: the AW FV_Fiew::cmdInsertTOC() does not specify the
		//   id, so neither will we
		//   
		// AW currently only uses the first 4 Heading styles, but we will implement this for all 9
		// to avoid future work
		
		for(i = 1; i < i1; ++i)
		{
			UT_UTF8String_sprintf(sTemp, "toc-source-style%d:nonexistentstyle;", i);
			sProps += sTemp;
		}

		UT_sint32 iMin = UT_MIN(i2+1,10);
		
		for(i = i1; i < iMin; ++i)
		{
			UT_UTF8String_sprintf(sTemp, "toc-dest-style%d:TOC %d", i, i);
			sProps += sTemp;
			sProps += ";";

			if(sLeader.size())
			{
				UT_UTF8String_sprintf(sTemp, "toc-tab-leader%d:", i);
				sProps += sTemp;
				sProps += sLeader;
				sProps += ";";
			}
		}

		for(i = iMin; i < 10; ++i)
		{
			UT_UTF8String_sprintf(sTemp, "toc-dest-style%d:nonexistentstyle", i);
			sProps += sTemp;
			sProps += ";";
		}
	}

	// the \t and \o switches can be used simultaneously
	// if both switches define the same level, we are unable to handle that; we will used the style
	// in the \t switch (it is easier since the parsing of the \t parameter is destructive)
	if ((t = strstr(params, "\\t")))
	{
		// style-based toc, the params have the format
		// \t "style,level,style,level ..."
		bSupported = true;
		t1 = strchr(t, '\"');
		if(!t1)
		{
			bRet = false;
			goto finish;
		}

		char * end = strchr(t1+1, '\"');

		while(t1 && t1 < end)
		{
			t1++;
			t2 = strchr(t1, ',');
			if(!t2)
			{
				bRet = false;
				goto finish;
			}

			*t2 = 0;

			sTemp = t1; // style name
			
			t1 = t2 + 1; // style level
			t2 = strchr(t1, ',');

			if(t2)
				t2 = UT_MIN(t2,end);
			else
				t2 = end;
			
			*t2 = 0;
			
			sProps += "toc-source-style";
			sProps += t1;
			sProps += ":";
			sProps += sTemp;
			sProps += ";";

			sProps += "toc-dest-style";
			sProps += t1;
			sProps += ":TOC ";
			sProps += t1;
			sProps += ";";
			
			if(sLeader.size())
			{
				sProps += "toc-tab-leader";
				sProps += t1;
				sProps += ":";
				sProps += sLeader;
				sProps += ";";
			}
			
			t1 = t2;
		}
	}

	if(!bSupported)
	{
		bRet = false;
		goto finish;
	}

	// remove trailing semicolon (screws up property parser)
	{
		sTemp = sProps;
		const char * c = sTemp.utf8_str();
		if(c[strlen(c)-1] == ';')
		{
			sProps.assign(c, strlen(c)-1);
		}
	}

	attrs[1] = sProps.utf8_str();

	if(!m_bInPara)
	{
		_appendStrux(PTX_Block, PP_NOPROPS);
		m_bInPara = true ;
	}

	_appendStrux(PTX_SectionTOC, PP_std_copyProps(attrs));
	_appendStrux(PTX_EndTOC, PP_NOPROPS);

 finish:
	FREEP(command);
	return bRet;
}


bool IE_Imp_MsWord_97::_handleCommandField (char *command)
{
	// if we are currently inside a supported TOC, just return
	if(m_bInTOC && m_bTOCsupported)
		return true;
	
	Doc_Field_t tokenIndex = F_OTHER;
	char *token = NULL;
	field * f = NULL;
	m_stackField.viewTop((void**)&f);
	UT_return_val_if_fail(f,true);
	bool bTypeSet = false;
	
	xxx_UT_DEBUGMSG(("DOM: handleCommandField '%s'\n", command));

	PP_PropertyVector atts = {
		"type"
	};

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
				atts.push_back("time");
				break;

			case F_DateTimePicture:
				//seems similar to a creation date
				atts.push_back("meta_date");
				break;

			case F_DATE:
				atts.push_back("date");
				break;

			case F_PAGE:
				atts.push_back("page_number");
				break;

			case F_NUMCHARS:
				atts.push_back("char_count");
				break;

			case F_NUMPAGES:
				atts.push_back("page_count");
				break;

			case F_NUMWORDS:
				atts.push_back("word_count");
				break;

			case F_FILENAME:
				atts.push_back("file_name");
				break;

			case F_PAGEREF:
				token = strtok (NULL, "\"\" ");
				atts.push_back("page_ref");
				atts.push_back("param");
				if(token)
					atts.push_back(token);
				else
					atts.push_back("no_bookmark_given");
				break;

			case F_HYPERLINK:
				{
					token = strtok (NULL, "\"\" ");

					if(token) {
					  // hyperlink or hyperlink to bookmark
					  std::string href;
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
					  PP_PropertyVector new_atts = {
						  "xlink:href", href
					  };
					  this->_flush();

					  if(!m_bInPara)
					    {
					      _appendStrux(PTX_Block, PP_NOPROPS);
					      m_bInPara = true ;
					    }

					  if(m_bInLink)
					    {
					      UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
					      _appendObject(PTO_Hyperlink, PP_NOPROPS);
					      m_bInLink = false;
					    }

					  _appendObject(PTO_Hyperlink, new_atts);
					  m_bInLink = true;
					}
					return true;
				}

			case F_TOC:             // for the toc fields we will
			case F_TOC_FROM_RANGE:  // insert the field result for now
				UT_DEBUGMSG(("TOC field encountered\n"));
				m_bInTOC = true;
				m_bTOCsupported = _isTOCsupported(f);
				
			default:
				// unhandled field type
				token = strtok(NULL, "\t, ");
				continue;
		}

		
		this->_flush();

		if(!m_bInPara)
		{
			_appendStrux(PTX_Block, PP_NOPROPS);
			m_bInPara = true ;
		}

		if (!_appendObject (PTO_Field, atts))
		{
			UT_DEBUGMSG(("Dom: couldn't append field (type = '%s')\n", atts[1].c_str()));
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

static IEGraphicFileType s_determineIEGFT ( Blip * b )
{
	if ( !b )
		return IEGFT_Unknown;

	switch ( b->type )
	{
	case msoblipEMF:
		return IEGFT_EMF;
	case msoblipWMF:
		return IEGFT_WMF;

	case msoblipJPEG:
		return IEGFT_JPEG;
	case msoblipPNG:
		return IEGFT_PNG;
	case msoblipDIB:
		return IEGFT_DIB;

	case msoblipPICT:
	case msoblipERROR:
	case msoblipUNKNOWN:
	default:
		return IEGFT_Unknown;
	}
}



UT_Error IE_Imp_MsWord_97::_handleImage (Blip * b, long width, long height, long cropt, long cropb, long cropl, long cropr)
{
	FG_Graphic* pFG		= 0;
	UT_Error error		= UT_OK;
	const UT_ByteBuf * buf		= 0;

	std::string propBuffer;
	std::string propsName;

	// suck the data into the ByteBuffer

	MSWord_ImageType imgType = s_determineImageType ( b );
	IEGraphicFileType iegft = s_determineIEGFT( b );

	wvStream *pwv;
	bool decompress = false;

	if ( imgType == MSWord_RasterImage )
	{
		pwv = b->blip.bitmap.m_pvBits;

	}
	else if ( imgType == MSWord_VectorImage )
	{
		pwv = b->blip.metafile.m_pvBits;
		decompress = (b->blip.metafile.m_fCompression == msocompressionDeflate);
	}
	else
	{
		UT_DEBUGMSG(("UNKNOWN IMAGE TYPE!!"));
		return UT_ERROR;
	}

	size_t size = wvStream_size (pwv);
	char *data = new char[size];
	wvStream_rewind(pwv);
	wvStream_read(data,size,sizeof(char),pwv);

	UT_ByteBuf pictData;
	if (decompress)
	{

		unsigned long uncomprLen, comprLen;
		comprLen = size;
		uncomprLen = b->blip.metafile.m_cb;
		Bytef *uncompr = new Bytef[uncomprLen];
		int err = uncompress (uncompr, &uncomprLen, reinterpret_cast<const unsigned char *>(data), comprLen);
		if (err != Z_OK)
		{
			UT_DEBUGMSG(("Could not uncompress image\n"));
			DELETEPV(uncompr);
			goto Cleanup;
		}
		pictData.append(reinterpret_cast<const UT_Byte*>(uncompr), uncomprLen);
		DELETEPV(uncompr);
	}
	else
	{
		pictData.append(reinterpret_cast<const UT_Byte*>(data), size);
	}

	delete [] data;

	if(!pictData.getPointer(0))
		error =  UT_ERROR;
	else
		error = IE_ImpGraphic::loadGraphic (pictData, iegft, &pFG);

	if ((error != UT_OK) || !pFG)
	{
		UT_DEBUGMSG(("Could not import graphic\n"));
		goto Cleanup;
	}

	buf = pFG->getBuffer();

	if (!buf)
	{
		// i don't think that this could ever happen, but...
		UT_DEBUGMSG(("Could not convert to PNG\n"));
		error = UT_ERROR;
		goto Cleanup;
	}

	//
	// This next bit of code will set up our properties based on the image attributes
	//

	{
		UT_LocaleTransactor t(LC_NUMERIC, "C");
		propBuffer = UT_std_string_sprintf("width:%fin; height:%fin; cropt:%fin; cropb:%fin; cropl:%fin; cropr:%fin",
						  static_cast<double>(width) / static_cast<double>(1440),
						  static_cast<double>(height) / static_cast<double>(1440),
						  static_cast<double>(cropt) / static_cast<double>(1440),
						  static_cast<double>(cropb) / static_cast<double>(1440),
						  static_cast<double>(cropl) / static_cast<double>(1440),
						  static_cast<double>(cropr) / static_cast<double>(1440));
	}

	propsName = UT_std_string_sprintf("%d", getDoc()->getUID(UT_UniqueId::Image));

	if (!_ensureInBlock())
	{
		UT_DEBUGMSG (("_ensureInBlock() failed\n"));
		error = UT_ERROR;
		goto Cleanup;
	}

	{
		PP_PropertyVector propsArray = {
			"props", propBuffer,
			"dataid", propsName
		};

		if (!_appendObject (PTO_Image, propsArray)) {
			UT_DEBUGMSG (("Could not create append object\n"));
			error = UT_ERROR;
			goto Cleanup;
		}
	}
	if (!getDoc()->createDataItem(propsName.c_str(), false,
								  buf, pFG->getMimeType(), NULL))
	{
		UT_DEBUGMSG (("Could not create data item\n"));
		// the mimetype is sunk anyway
		error = UT_ERROR;
		goto Cleanup;
	}

Cleanup:
	DELETEP(pFG);

	return error;
}



/*!
 * This method imports an image that can be later used as an embedded object.
 * The Blip pointer p contains the MS Word data we use to create the image
 * "width" and "height" are the width and height of the object in inches.
 * The routine returns the name of the data-item it creates is in the 
 * UT_UTF8String sImageName
 */
UT_Error IE_Imp_MsWord_97::_handlePositionedImage (Blip * b, UT_String & sImageName)
{
	FG_Graphic* pFG		= 0;
	UT_Error error		= UT_OK;
	const UT_ByteBuf * buf		= 0;

  // suck the data into the ByteBuffer

  MSWord_ImageType imgType = s_determineImageType ( b );

  wvStream *pwv;
  bool decompress = false;

  if ( imgType == MSWord_RasterImage )
	{
	  pwv = b->blip.bitmap.m_pvBits;

	}
  else if ( imgType == MSWord_VectorImage )
	{
	  pwv = b->blip.metafile.m_pvBits;
	  decompress = (b->blip.metafile.m_fCompression == msocompressionDeflate);
	}
  else
	{
	  UT_DEBUGMSG(("UNKNOWN IMAGE TYPE!!"));
	  return UT_ERROR;
	}

  size_t size = wvStream_size (pwv);
  char *data = new char[size];
  wvStream_rewind(pwv);
  wvStream_read(data,size,sizeof(char),pwv);

  UT_ByteBuf pictData;

  if (decompress)
  {

    unsigned long uncomprLen, comprLen;
    comprLen = size;
    uncomprLen = b->blip.metafile.m_cb;
    Bytef *uncompr = new Bytef[uncomprLen];
    int err = uncompress (uncompr, &uncomprLen, reinterpret_cast<const unsigned char *>(data), comprLen);
    if (err != Z_OK)
    {
        UT_DEBUGMSG(("Could not uncompress image\n"));
        DELETEPV(uncompr);
        goto Cleanup;
    }
    pictData.append(reinterpret_cast<const UT_Byte*>(uncompr), uncomprLen);
    DELETEPV(uncompr);
  }
  else
  {
    pictData.append(reinterpret_cast<const UT_Byte*>(data), size);
  }

  delete [] data;

  if(!pictData.getPointer(0))
	  error =  UT_ERROR;
  else
	  error = IE_ImpGraphic::loadGraphic (pictData, IEGFT_Unknown, &pFG);

  if ((error != UT_OK) || !pFG)
	{
	  UT_DEBUGMSG(("Could not import graphic\n"));
	  goto Cleanup;
	}

  // TODO: can we get back a vector graphic?
  buf = pFG->getBuffer();

  if (!buf)
	{
	  // i don't think that this could ever happen, but...
	  UT_DEBUGMSG(("Could not convert to PNG\n"));
	  error = UT_ERROR;
	  goto Cleanup;
	}

  UT_String_sprintf(sImageName, "%d", getDoc()->getUID(UT_UniqueId::Image));

  if (!getDoc()->createDataItem(sImageName.c_str(), false,
                                buf, pFG->getMimeType(), NULL))
	{
	  UT_DEBUGMSG (("Could not create data item\n"));
	  error = UT_ERROR;
	  goto Cleanup;
	}

 Cleanup:  
  DELETEP(pFG);

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

  //  _appendStrux(PTX_Block, NULL); // Don't need/want this after 27/3/2005
  _appendStrux(PTX_SectionTable, PP_NOPROPS);
  m_vecColumnWidths.clear();
  m_bRowOpen = false;
  m_bCellOpen = false;
  m_bInPara = false;
#ifdef DEBUG
  static UT_sint32 sTableCount = 0;
  sTableCount++;
#endif
  UT_DEBUGMSG(("\n<TABLE> [%d]", sTableCount));

}

//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

/*!
 * Exand a vector with zeros to make room for a new value
 */
void IE_Imp_MsWord_97::setNumberVector(UT_NumberVector & vec, UT_sint32 i, UT_sint32 val)
{
	while(i > static_cast<UT_sint32>(vec.size() +1))
	{
		vec.addItem(0);
	}
	vec.addItem(val); // we are sure that it will be appened at index i
}

/*!
 * This method parses the vector of MsColSpans held by m_vecColumnWidths
 * and fills the vector colWidths with the widths of the individual columns.
 *
 * We do this because MSWord provides the widths of column spans, and in 
 * some cases you can get a table with no row fully partitioned into 
 * individual cells.
 */
bool IE_Imp_MsWord_97::_build_ColumnWidths(UT_NumberVector & colWidths)
{

// OK handle the easy cases first and find the maximum value of iRight

	UT_sint32 iMaxRight = 0;
	UT_sint32 i = 0;
	UT_sint32 iLeft,iRight = 0;
	UT_sint32 iSize = static_cast<UT_sint32>(m_vecColumnWidths.size());
	for(i=0; i< iSize;i++)
	{
		MsColSpan * pSpan = reinterpret_cast<MsColSpan *>(m_vecColumnWidths.getNthItem(i));
		iLeft = pSpan->iLeft;
		iRight = pSpan->iRight;
		if(iMaxRight < iRight)
		{
			iMaxRight = iRight;
		}
		if((iLeft + 1) == iRight)
		{
			setNumberVector(colWidths,iLeft,pSpan->width);
			xxx_UT_DEBUGMSG(("_build_ColumnWidths Initial set: Left %d Width %d \n",iLeft,colWidths[iLeft]));
		}
	}
//
// Look to see if we're finished now.
//
	if((colWidths.size() == iMaxRight) && _isVectorFull(colWidths))
	{
		return true;
	}
	if(colWidths.size() < iMaxRight)
	{
		setNumberVector(colWidths,iMaxRight -1,0);
	}
//
// OK Now the hard part. Procede by scanning through the m_vecColWidths,
// Looking for spans, at each span we look to see if we can break the span
// into smaller pieces by subtracting a single span width.
//
// When we have a single column span we insert it in colWidths if colWidths
// is empty at that point.
//
// We continue until colWidths is completely full.
//
	UT_uint32 iLoop = 0;
	while(iLoop < 1000 && !_isVectorFull(colWidths))
	{
		for(i=0; i<static_cast<UT_sint32>(m_vecColumnWidths.size()); i++)
		{
			MsColSpan * pSpan = reinterpret_cast<MsColSpan *>(m_vecColumnWidths.getNthItem(i));
			iLeft = pSpan->iLeft;
			iRight = pSpan->iRight;
			xxx_UT_DEBUGMSG(("Loop %d iLeft %d,iRight %d colWidth[iLeft] %d colWidth[iRight-1] %d\n",iLoop,iLeft,iRight,colWidths[iLeft],colWidths[iRight -1]));
			if(iMaxRight < iRight)
			{
				iMaxRight = iRight;
			}
			if(((iLeft + 1) == iRight) && (colWidths[iLeft] == 0))
			{
				setNumberVector(colWidths,iLeft,pSpan->width);
			}
			else if((iLeft + 1) < iRight)
			{
				if(colWidths[iLeft] > 0)
				{
					if(!findMatchSpan(iLeft+1,iRight))
					{
						MsColSpan * pNewSpan = new MsColSpan();
						pNewSpan->iLeft = iLeft+1;
						pNewSpan->iRight = iRight;
						pNewSpan->width = pSpan->width - colWidths[iLeft];
						m_vecColumnWidths.addItem(pNewSpan);
					}
				}
				else if(colWidths[iRight - 1] > 0)
				{
					if(!findMatchSpan(iLeft,iRight-1))
					{
						MsColSpan * pNewSpan = new MsColSpan();
						pNewSpan->iLeft = iLeft;
						pNewSpan->iRight = iRight-1;
						pNewSpan->width = pSpan->width - colWidths[iRight-1];
						m_vecColumnWidths.addItem(pNewSpan);
					}
				}
//
// OK now look to see if we can fragment this by substracting a span of more 
// than one column from either end.
//
				else
				{
					UT_sint32 k =0;
					for(k=0; k<static_cast<UT_sint32>(m_vecColumnWidths.size()); k++)
					{
						MsColSpan * pMulSpan = m_vecColumnWidths.getNthItem(i);
						UT_sint32 iMulLeft = pMulSpan->iLeft;
						UT_sint32 iMulRight = pMulSpan->iRight;
						if(iMulLeft == iLeft && iMulRight < iRight)
						{
//
// Make a new span fragment out of the bit greater than MulRight if one doesn't
// exist
//
							if(!findMatchSpan(iMulRight+1,iRight))
							{
								MsColSpan * pNewSpan = new MsColSpan();
								pNewSpan->iLeft = iMulRight+1;
								pNewSpan->iRight = iRight;
								pNewSpan->width = pSpan->width - pMulSpan->width;
								m_vecColumnWidths.addItem(pNewSpan);
							}

						}
						else if (iMulLeft > iLeft && iMulRight == iRight)
						{
//
// Make a new span fragment out of the bit less than MulLeft
//
							if(!findMatchSpan(iLeft,iMulLeft))
							{
								MsColSpan * pNewSpan = new MsColSpan();
								pNewSpan->iLeft = iLeft;
								pNewSpan->iRight = iMulLeft;
								pNewSpan->width = pSpan->width - pMulSpan->width;
								m_vecColumnWidths.addItem(pNewSpan);
							}							
						}
					}
				}
			}
		}
		iLoop++;
		UT_ASSERT_HARMLESS(0);
	}
	UT_ASSERT_HARMLESS(iLoop < 1000);
	return (iLoop < 1000);
}

/*!
 * Returns true if a span in the m_vecColumnWidths span matches the left, right
 * values given
 */
bool IE_Imp_MsWord_97::findMatchSpan(UT_sint32 iLeft,UT_sint32 iRight)
{
	UT_sint32 i =0;
	for(i=0; i< static_cast<UT_sint32>(m_vecColumnWidths.size());i++)
	{
		MsColSpan * pSpan = m_vecColumnWidths.getNthItem(i);
		if(pSpan->iLeft == iLeft && pSpan->iRight == iRight)
		{
			return true;
		}
	}
	return false;
}

/*!
 * Returns false if any element in the vector is non-zero
 */
bool IE_Imp_MsWord_97::_isVectorFull(UT_NumberVector & vec)
{
	UT_sint32 i = 0;
	for(i=0;i< vec.size() ; i++)
	{
		xxx_UT_DEBUGMSG(("isVectorFull i %d val %d \n",i,vec[i]));
		if( vec[i] == 0)
		{
			return false;
			break;
		}
	}
	return true;
}

void IE_Imp_MsWord_97::_table_close (const wvParseStruct * /*ps*/, const PAP *apap)
{
  _cell_close();
  _row_close();

  UT_String props("table-column-props:");
  UT_String propBuffer;

  if (m_vecColumnWidths.size() > 0) 
  {
	  // build column width properties string
	  UT_NumberVector colWidths;
//
// Some tables maybe too complicated for my simple algorithim to work out
//
	  if(_build_ColumnWidths(colWidths))
	  {

		  for (UT_sint32 i = 0; i < colWidths.size(); i++) 
		  {
			  UT_String_sprintf(propBuffer,"%s/",
							UT_convertInchesToDimensionString(m_dim,
															  (static_cast<double>(colWidths.getNthItem(i)))/1440.0));
	  
			  props += propBuffer;
		  }
	  }
	  
	  props += "; ";
//
// FIXME: Put in left position here!!!!
//
	  UT_String_sprintf(propBuffer,"table-column-leftpos:%s; ",
							UT_convertInchesToDimensionString(m_dim,
															  (static_cast<double>(m_iLeftCellPos)/1440.0)));
	  props += propBuffer;
	  UT_VECTOR_PURGEALL(MsColSpan *,m_vecColumnWidths);
	  m_vecColumnWidths.clear ();
  }

  props += "table-line-ignore:0; table-line-type:1; table-line-thickness:0.8pt;";
  if(apap->ptap.dxaGapHalf > 0)
  {
	  props += UT_String_sprintf("table-col-spacing:%din", (2 * apap->ptap.dxaGapHalf)/ 1440);
  }
  else
  {
	  props += "table-col-spacing:0.03in";
  }
  // apply properties 
  PT_DocPosition posEnd =0;
  getDoc()->getBounds(true,posEnd); // clean frags!
  pf_Frag_Strux* sdh = getDoc()->getLastStruxOfType(PTX_SectionTable);
  getDoc()->changeStruxAttsNoUpdate(sdh,"props",props.c_str());

  // end-of-table
  _appendStrux(PTX_EndTable, PP_NOPROPS);
  m_bInPara = false ;

  UT_DEBUGMSG(("\n</TABLE>\n"));
}

//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

void IE_Imp_MsWord_97::_row_open (const wvParseStruct *ps)
{
  if (m_bRowOpen)
    return;

  if (m_iCurrentRow > ps->norows) {
	  //UT_ASSERT(m_iCurrentRow <= ps->norows);
	  return;
  }

  m_bRowOpen = true;
  m_iCurrentRow++;
  xxx_UT_DEBUGMSG(("imp_MsWord: _row_open: Last Left %d Last Right %d \n",m_iLeft,m_iRight));
  m_iCurrentCell = 0;
  m_iLeft = 0;
  m_iRight = 0;
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

  if (!m_bRowOpen || m_iCurrentRow > ps->norows) {
	  //UT_ASSERT(m_bRowOpen || m_iCurrentRow <= ps->norows);
	  return;
  }

  UT_Vector columnWidths;
  UT_sint32 vspan = 0;
  UT_String propBuffer;

  const gchar* propsArray[3];
  propsArray[0] = static_cast<const gchar*>("props");
  propsArray[1] = "";
  propsArray[2] = NULL;
	  
  
#if 0
  if(m_iCurrentCell >= apap->ptap.itcMac)
  {
	  // this happens when the row contains no cell definitions; we
	  // need to insert a dummy cell into our row
	  goto do_insert;
  }
#endif
  
  // add a new cell
  m_bCellOpen = true;
  if(m_iCurrentCell == 0)
  {
//
// Scan the differences in centers for this row so we can work out the column
// widths of the table eventually.
//
	  m_iLeftCellPos = 0;
	  UT_sint32 iLeft, iRight, i;
	  m_iLeftCellPos = ps->cellbounds[0];
	  for(i = 0; i < ps->nocellbounds-1; i++) 
	  {
		  iLeft = i;
		  iRight = i+1;
		  UT_sint32 width = ps->cellbounds[iRight] - ps->cellbounds[iLeft];
		  if (width <= 0)
			  break;
		  MsColSpan * pSpan = new MsColSpan();
		  pSpan->iLeft = iLeft;
		  pSpan->iRight = iRight;
		  pSpan->width = width;
		  xxx_UT_DEBUGMSG(("MsImport iLeft %d  iRight %d width  %d \n",iLeft,iRight,width));
		  m_vecColumnWidths.addItem(pSpan);
	  }
  }

  if (ps->vmerges && ps->vmerges[m_iCurrentRow - 1])
    vspan = ps->vmerges[m_iCurrentRow - 1][m_iCurrentCell];

  if (vspan > 0)
    vspan--;

  m_iRight = m_iLeft + m_vecColumnSpansForCurrentRow.getNthItem(m_iCurrentCell);
  if(m_iRight == m_iLeft)
  {
	  m_iRight++;
  }
  xxx_UT_DEBUGMSG(("MSWord Import:  iLeft %d iRight %d m_iCurrentCell %d \n",m_iLeft,m_iRight,m_iCurrentCell));
  UT_return_if_fail(vspan >= 0);
  UT_String_sprintf(propBuffer,
		    "left-attach:%d; right-attach:%d; top-attach:%d; bot-attach:%d; ",
		    m_iLeft,
		    m_iRight,
		    m_iCurrentRow - 1,
		    m_iCurrentRow + vspan
		    );

  if(apap->ptap.dyaRowHeight < 0)
  {
	  // absolute height
	  double dHin = -(apap->ptap.dyaRowHeight/1440);
	  propBuffer += UT_String_sprintf("height:%fin;",dHin);
  }
  else if(apap->ptap.dyaRowHeight > 0)
  {
	  // at-least height -- I do not think we support this for now
	  // double dHin = -(apap->ptap.dyaRowHeight/1440);
	  // propBuffer += UT_String_sprintf("height:%fin;",dHin);
  }
  else
  {
	  // auto height, do nothing
  }
    
  propBuffer += UT_String_sprintf("color:%s;", sMapIcoToColor(apap->ptap.rgshd[m_iCurrentCell].icoFore, true).c_str());
  propBuffer += UT_String_sprintf("background-color:%s;", sMapIcoToColor(apap->ptap.rgshd[m_iCurrentCell].icoBack, false).c_str());
  // so long as it's not the "auto" color
  if (apap->ptap.rgshd[m_iCurrentCell].icoBack != 0)
    propBuffer += "bg-style:1;";

  {
	  UT_LocaleTransactor t(LC_NUMERIC, "C");
	  propBuffer += UT_String_sprintf("top-color:%s; top-thickness:%fpt; top-style:%d;",
									  sMapIcoToColor(apap->ptap.rgtc[m_iCurrentCell].brcTop.ico, true).c_str(),
									  brc_to_pixel(apap->ptap.rgtc[m_iCurrentCell].brcTop.dptLineWidth),
									  sConvertLineStyle(apap->ptap.rgtc[m_iCurrentCell].brcTop.brcType));
	  propBuffer += UT_String_sprintf("left-color:%s; left-thickness:%fpx; left-style:%d;",
									  sMapIcoToColor(apap->ptap.rgtc[m_iCurrentCell].brcLeft.ico, true).c_str(),
									  brc_to_pixel(apap->ptap.rgtc[m_iCurrentCell].brcLeft.dptLineWidth),
									  sConvertLineStyle(apap->ptap.rgtc[m_iCurrentCell].brcLeft.brcType));
	  propBuffer += UT_String_sprintf("bot-color:%s; bot-thickness:%fpx; bot-style:%d;",
									  sMapIcoToColor(apap->ptap.rgtc[m_iCurrentCell].brcBottom.ico, true).c_str(),
									  brc_to_pixel(apap->ptap.rgtc[m_iCurrentCell].brcBottom.dptLineWidth),
									  sConvertLineStyle(apap->ptap.rgtc[m_iCurrentCell].brcBottom.brcType));
	  propBuffer += UT_String_sprintf("right-color:%s; right-thickness:%fpx; right-style:%d",
									  sMapIcoToColor(apap->ptap.rgtc[m_iCurrentCell].brcRight.ico, true).c_str(),
									  brc_to_pixel(apap->ptap.rgtc[m_iCurrentCell].brcRight.dptLineWidth),
									  sConvertLineStyle(apap->ptap.rgtc[m_iCurrentCell].brcRight.brcType));
  }
  xxx_UT_DEBUGMSG(("propbuffer: %s \n",propBuffer.c_str()));

  propsArray[1] = propBuffer.c_str();

  // do_insert:
  _appendStrux(PTX_SectionCell, PP_std_copyProps(propsArray));
  m_bInPara = false;
  m_iCurrentCell++;
  m_iLeft = m_iRight;
  xxx_UT_DEBUGMSG(("\t<CELL:%d:%d>", static_cast<int>(m_vecColumnSpansForCurrentRow.getNthItem(m_iCurrentCell - 1)), ps->vmerges[m_iCurrentRow - 1][m_iCurrentCell - 1]));
}

//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

void IE_Imp_MsWord_97::_cell_close ()
{
  if (!m_bCellOpen)
    return;

  m_bCellOpen = false;
  _appendStrux(PTX_EndCell, PP_NOPROPS);
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
						  sMapIcoToColor(ico, true).c_str());
		s += propBuffer;
	}

	// background color
	ico = achp->shd.icoBack;
	if (ico) {
		if (!achp->fHighlight) {
			// HACK: We don't support borders and shading yet, so it seems safe to use the background
			// color as a substitute when there's no true highlight color (see the doc from Bug 6432)
			UT_String_sprintf(propBuffer, "bgcolor:%s;",
							  sMapIcoToColor(ico, false).c_str());
		} else {
			// Note: This property won't be rendered until we have borders and shading support
			UT_String_sprintf(propBuffer, "background-color:%s;",
							  sMapIcoToColor(ico, false).c_str());
		}
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
						  sMapIcoToColor(achp->icoHighlight, false).c_str());
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
	if(achp->xchSym)
	{
		fname = wvGetFontnameFromCode(&ps->fonts, achp->ftcSym);
	}
	else if (achp->fBidi)
	{
		fname = wvGetFontnameFromCode(&ps->fonts, achp->ftcBidi);
	}
	else if (!ps->fib.fFarEast)
	{
		fname = wvGetFontnameFromCode(&ps->fonts, achp->ftcAscii);
	}
	else
	{
		fname = wvGetFontnameFromCode(&ps->fonts, achp->ftcFE);
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

void IE_Imp_MsWord_97::_generateParaProps(UT_String &s, const PAP * apap, wvParseStruct * /*ps*/)
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
						  UT_convertToDimensionlessString( (static_cast<double>(apap->lspd.dyaLine) / 240), "1.1"));
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
						  UT_convertInchesToDimensionString(m_dim, (static_cast<double>(apap->dxaRight) / 1440)));
		s += propBuffer;
	}

	// margin-left
	if (apap->dxaLeft) {
		UT_String_sprintf(propBuffer,
						  "margin-left:%s;",
						  UT_convertInchesToDimensionString(m_dim, (static_cast<double>(apap->dxaLeft) / 1440)));
		s += propBuffer;
	}

	// margin-left first line (indent)
	if (apap->dxaLeft1) {
		UT_String_sprintf(propBuffer,
						  "text-indent:%s;",
						  UT_convertInchesToDimensionString(m_dim, (static_cast<double>(apap->dxaLeft1) / 1440)));
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
		s += propBuffer;
	}

	// tab stops
	if (apap->itbdMac) {
		propBuffer += "tabstops:";

		for (int iTab = 0; iTab < apap->itbdMac; iTab++) {
			propBuffer += UT_String_sprintf("%s/",
						UT_convertInchesToDimensionString(m_dim,
										((static_cast<double>(apap->rgdxaTab[iTab])) / 1440)));
			
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
						  sMapIcoToColor(ico, true).c_str());
		s += propBuffer;
	}

	// background color
	ico = apap->shd.icoBack;
	if (ico) {
		UT_String_sprintf(propBuffer, "background-color:%s;",
						  sMapIcoToColor(ico, false).c_str());
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

	const gchar * attribs[PT_MAX_ATTRIBUTES*2 + 1];
	UT_uint32 iOffset = 0;
	
	const STD * pSTD = ps->stsh.std;
	const STD * pSTDBase = pSTD;
	UT_String props;
	char * s = NULL;
	char * b = NULL;
	char * f = NULL;

	UT_return_if_fail(pSTD != NULL);

	for(UT_uint32 i = 0; i < iCount; i++, pSTD++)
	{
		iOffset = 0;

		if(!pSTD->xstzName)
		{
			continue;
		}

		if(pSTD->cupx <= 1)
		{
			continue;
		}

		//UT_DEBUGMSG(("Style name: [%s], id: %d\n", pSTD->xstzName, pSTD->sti));

		attribs[iOffset++] = PT_NAME_ATTRIBUTE_NAME;

		// make sure we use standard names for standard styles
		const gchar * pName = s_translateStyleId(pSTD->sti);

		if(pName)
		{
			attribs[iOffset++] = pName;
		}
		else
		{
			s = s_convert_to_utf8(ps, pSTD->xstzName);
			attribs[iOffset++] = s;
		}
		
		UT_DEBUGMSG(("Style name: [%s], id: %d\n", attribs[iOffset-1], pSTD->sti));

		
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
			if(pSTD->istdNext != istdNil && pSTD->istdNext<iCount)
			{
				attribs[iOffset++] = PT_FOLLOWEDBY_ATTRIBUTE_NAME;
				const char * t = s_translateStyleId(pSTD->istdNext);
				if(!t)
				{
					t = f = s_convert_to_utf8(ps,(pSTDBase + pSTD->istdNext)->xstzName);					
				}
				attribs[iOffset++] = t;
			}
		}

		if(pSTD->istdBase != istdNil)
		{
			attribs[iOffset++] = PT_BASEDON_ATTRIBUTE_NAME;
			const char * t = s_translateStyleId(pSTD->istdBase);
			if(!t)
				t = b = s_convert_to_utf8(ps,(pSTDBase + pSTD->istdBase)->xstzName);
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
			pStyle->addAttributes(PP_std_copyProps(attribs));
			pStyle->getBasedOn();
			pStyle->getFollowedBy();
		}
		else
		{
			getDoc()->appendStyle(PP_std_copyProps(attribs));
		}

		FREEP(s);
		FREEP(b);
		FREEP(f);
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
			//g_free the bkf and posf
			wvFree(bkf);
			wvFree(posf);
			m_iBookmarksCount = 0;
		}
	}
	UT_return_val_if_fail(nobkl == nobkf, 0);
	if(m_iBookmarksCount > 0)
	{
		try
		{
			m_pBookmarks = new bookmark[m_iBookmarksCount];
		}
		catch(...)
		{
			m_pBookmarks = NULL;
		}

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
		// g_free bkf, bkl, posf, posl
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

	DELETEPV(m_pFootnotes);
	DELETEPV(m_pEndnotes);

	m_iFootnotesCount = 0;
	m_iEndnotesCount = 0;
	UT_uint32 *pPLCF_ref = NULL;
	UT_uint32 *pPLCF_txt = NULL;

	bool bNoteError = false;

	if(ps->fib.lcbPlcffndTxt)
	{
		/* the docs say -1, but that is an error */
		m_iFootnotesCount = ps->fib.lcbPlcffndTxt/4 - 2;
		try
		{
			m_pFootnotes = new footnote[m_iFootnotesCount];
		}
		catch(...)
		{
			m_pFootnotes = NULL;
		}

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
		PP_PropertyVector props = {
			"document-footnote-type",            "",
			"document-footnote-initial", UT_std_string_sprintf("%d", ps->dop.nFtn),
			"document-footnote-restart-section", "",
			"document-footnote-restart-page",    "",
		};

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
				UT_ASSERT_HARMLESS(UT_NOT_REACHED);
		}

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
				UT_ASSERT_HARMLESS(UT_NOT_REACHED);
				props[1] = "";
				break;
		}

		getDoc()->setProperties(props);
	}

	if(ps->fib.lcbPlcfendTxt)
	{
		m_iEndnotesCount  = ps->fib.lcbPlcfendTxt/4 - 2;
		try
		{
			m_pEndnotes  = new footnote[m_iEndnotesCount];
		}
		catch(...)
		{
			m_pEndnotes = NULL;
		}

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
		PP_PropertyVector props = {
			"document-endnote-type",            "",
			"document-endnote-initial", UT_std_string_sprintf("%d", ps->dop.nEdn),
			"document-endnote-restart-section", "",
			"document-endnote-restart-page",    "",
			"document-endnote-place-endsection","",
			"document-endnote-place-enddoc",    "",
		};

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
				UT_ASSERT_HARMLESS(UT_NOT_REACHED);
		}

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
				UT_ASSERT_HARMLESS(UT_NOT_REACHED);
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
				UT_ASSERT_HARMLESS(UT_NOT_REACHED);
		}

		getDoc()->setProperties(props);
	}
}

void IE_Imp_MsWord_97::_handleTextBoxes(const wvParseStruct *ps)
{
	UT_uint32 *pPLCF_dgg = NULL;
	UT_uint32 *pPLCF_txt = NULL;

	DELETEPV(m_pTextboxes);

	bool bTextboxError = false;
	m_iTextboxCount = 0;
	UT_sint32 i = 0;
	if(ps->fib.ccpTxbx > 0)
	{
		m_iTextboxCount = ps->nooffspa;
		m_pTextboxes = new textbox [m_iTextboxCount];

		
		// this is really quite straight forward; we retrieve the PLCF
		// chunks that describe the references/text of the textboxes, and
		// then use those to init our textbox stucts
		// for n textboxes the reference PLCF is a sequnce of (n+1) doc
		// positions (UT_uint32) followed by n type flags (UT_uint16)
		// the text PLCF is a sequence of n+2 positions (UT_uint32) of the 
        // textbox
		// text in its data stream

// This appears to be identical to how footnotes/endnotes are handled.

		if(wvGetPLCF((void **) &pPLCF_dgg, ps->fib.fcDggInfo, ps->fib.lcbDggInfo, ps->tablefd))
		{
			bTextboxError = true;
		}

		UT_DEBUGMSG(("IE_Imp_MsWord_97::_handleTextBoxes: ps->fib.fcDggInfo %d ps->fib.lcbDggInfo %d \n", ps->fib.fcDggInfo,ps->fib.lcbDggInfo));

		UT_DEBUGMSG(("IE_Imp_MsWord_97::_handleTextBoxes: Text size %d bytes\n", ps->fib.ccpTxbx));
		
		UT_DEBUGMSG(("IE_Imp_MsWord_97::_handleTextBoxes: fib.lid %d \n", ps->fib.lid));
		if(!bTextboxError && 		   
		   wvGetPLCF((void **) &pPLCF_txt, ps->fib.fcPlcftxbxTxt, ps->fib.lcbPlcftxbxTxt, ps->tablefd))
		{
			bTextboxError = true;
		}
		if(!bTextboxError)
		{
			UT_return_if_fail(pPLCF_dgg && pPLCF_txt);
			for(i = 0; i < m_iTextboxCount; i++)
			{
				m_pTextboxes[i].ref_pos = pPLCF_dgg[i];
				m_pTextboxes[i].txt_pos = pPLCF_txt[i] + m_iTextboxesStart;
				m_pTextboxes[i].txt_len = pPLCF_txt[i+1] - pPLCF_txt[i];
				UT_DEBUGMSG(("IE_Imp_MsWord_97::_handleTextbox: Tbox %d, rpos %d, tpos %d len %d \n",
							 i, m_pTextboxes[i].ref_pos, m_pTextboxes[i].txt_pos,m_pTextboxes[i].txt_len));
			}

			wvFree(pPLCF_dgg);
			wvFree(pPLCF_txt);

		}
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
	xxx_UT_DEBUGMSG(("IE_Imp_MsWord_97::_insertFootnote: pos: %d, pid %d\n", f->ref_pos, f->pid));

	this->_flush();

	bool res = true;

	std::string footpid = UT_std_string_sprintf("%i", f->pid);
	const PP_PropertyVector attribsS = { "footnote-id", footpid };

	// for attribsR we need to set props and style in order to
	// preserve any formating set by a previous call to _beginChar()
	PP_PropertyVector attribsR = {
		"type", "footnote_ref",
		"footnote-id", footpid,
		"props", m_charProps.c_str()
	};
	if(!m_charStyle.empty())
	{
		attribsR.push_back("style");
		attribsR.push_back(m_charStyle.c_str());
	}

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
	_appendStrux(PTX_EndFootnote, PP_NOPROPS);

	if(!f->type)
	{
		// set the formatting to whatever it was, in case the footnote
		// marker is longer than one character
		_appendFmt(attribsR);
	}

	return res;
}

bool IE_Imp_MsWord_97::_insertEndnote(const footnote * f, UT_UCS4Char c)
{
	UT_return_val_if_fail(f, true);
	xxx_UT_DEBUGMSG(("IE_Imp_MsWord_97::_insertEndnote: pos: %d, pid %d\n", f->ref_pos, f->pid));

	this->_flush();

	bool res = true;

	std::string footpid = UT_std_string_sprintf("%i", f->pid);
	const PP_PropertyVector attribsS = {
		"endnote-id", footpid
	};
	// for attribsR we need to set props and style in order to
	// preserve any formating set by a previous call to _beginChar()
	const PP_PropertyVector attribsR = {
		"type", "endnote_ref", "endnote-id", footpid,
		"props", m_charProps.c_str(),
		"style", m_charStyle.c_str()
	};

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
	_appendStrux(PTX_EndEndnote, PP_NOPROPS);

	if(!f->type)
	{
		// set the formatting to whatever it was, in case the footnote
		// marker is longer than one character
		_appendFmt(attribsR);
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
			xxx_UT_DEBUGMSG(("In footnote territory: pos %d\n", iDocPosition));
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
		if( m_iNextFNote < m_iFootnotesCount && iDocPosition == m_pFootnotes[m_iNextFNote].txt_pos +
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
			std::string footpid =
				UT_std_string_sprintf("%i", m_pFootnotes[m_iNextFNote].pid);
			const PP_PropertyVector attribsA = {
				"type", "footnote_anchor",
				"footnote-id", footpid,
				"props",       m_charProps.c_str(),
				"style",       m_charStyle.c_str()
			};

			const PP_PropertyVector attribsB = {
				"props", m_paraProps.c_str(),
				"style", m_paraStyle.c_str()
			};

			_appendStrux(PTX_Block, attribsB);
			m_bInPara = true;

			if(m_pFootnotes[m_iNextFNote].type)
			{
				_appendObject(PTO_Field, attribsA);
				return false;
			}
			return true;
		}

		// do not return !!!
		xxx_UT_DEBUGMSG(("In footnote %d, on pos %d\n", m_iNextFNote, iDocPosition));
	}
	else if(m_bInFNotes)
	{
		m_bInFNotes = false;
		xxx_UT_DEBUGMSG(("Leaving footnote territory\n"));
		// move to the end of the do end of the document ...

		// do not return !!!
	}
	
	if(iDocPosition >= m_iEndnotesStart && iDocPosition < m_iEndnotesEnd)
	{
		if(!m_bInENotes)
		{
			xxx_UT_DEBUGMSG(("In endnote territory: pos %d\n", iDocPosition));
			m_bInENotes = true;
			m_bInHeaders = false;
			m_iNextENote = 0;
			_findNextENoteSection();
			_endSect(NULL,0,NULL,0);
			m_bInSect = true;
		}

		if( m_iNextENote < m_iEndnotesCount && iDocPosition == m_pEndnotes[m_iNextENote].txt_pos +
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
				xxx_UT_DEBUGMSG(("End of endnotes marker at pos %d\n", iDocPosition));
				return false;
			}
		}

		// if this is the first character in an endnote, insert the anchor
		if( m_iNextENote < m_iEndnotesCount && iDocPosition == m_pEndnotes[m_iNextENote].txt_pos)
		{
			std::string footpid =
				UT_std_string_sprintf("%i", m_pEndnotes[m_iNextENote].pid);

			const PP_PropertyVector attribsA = {
				"type", "endnote_anchor",
				"endnote-id", footpid,
				"props", m_charProps.c_str(),
				"style", m_charStyle.c_str()
			};

			const PP_PropertyVector attribsB = {
				"props", m_paraProps.c_str(),
				"style", m_paraStyle.c_str()
			};

			_appendStrux(PTX_Block, attribsB);
			m_bInPara = true;

			if(m_pEndnotes[m_iNextENote].type)
			{
				_appendObject(PTO_Field, attribsA);
				return false;
			}
			return true;
		}

		xxx_UT_DEBUGMSG(("In endnote %d, on pos %d\n", m_iNextENote, iDocPosition));
		// do not return !!!
	}
	else if(m_bInENotes)
	{
		m_bInENotes = false;
		xxx_UT_DEBUGMSG(("Leaving endnote territory\n"));
		// move to the end of the document ...

		// do not return !!!
	}

	// we only return here, so that the code above could be extended
	// for handly annotations by simply copy/paste
	return true;
}



/*!
    This function makes sure that the insert is happening at the
    correct place if we are in a segment which belongs to one of the
    set of Textboxes

    \parameter UT_uint32 iDocPosition: character position in the Word
                                       document stream
    \return returns false if the present character is to be skipped,
            true otherwise
*/
bool IE_Imp_MsWord_97::_handleTextboxesText(UT_uint32 iDocPosition)
{
	if(iDocPosition >= m_iTextboxesStart && iDocPosition < m_iTextboxesEnd)
	{
		// upon entry into the Textland-land, we will need to search for
		// the first Textbox section in our document, note that we are
		// in a Textbox section, note at what doc position the current
		// textbox will end, and then let things run until we reach
		// the end of the textbox; then we need to search for the next
		// doc section, etc.


		// when in a Text box section, all the functions that normally
		// use append methods will need to use insert methods instead

		if(!m_bInTextboxes)
		{
			UT_DEBUGMSG(("In Textbox territory: pos %d\n", iDocPosition));
			m_bInTextboxes = true;
			m_bInFNotes = false;
			m_bInHeaders = false;
			
			// we will reuse the m_iNextTextbox variable, noting it
			// refers to the CURRENT textbox

			m_iNextTextbox = 0;
			_findNextTextboxSection();
			_endSect(NULL,0,NULL,0);
			m_bInSect = true;
		}

		// the current footnote will end at pos
		// f.txt_pos + f.txt_len, 
		if( m_iNextTextbox < m_iTextboxCount && iDocPosition == m_pTextboxes[m_iNextTextbox].txt_pos +
		                   m_pTextboxes[m_iNextTextbox].txt_len)
		{
			m_iNextTextbox++;

			// after the last footnote there is an extra paragraph
			// marker that is still a part of the footnote section --
			// we do not want that marker imported
			if(m_iNextTextbox < m_iTextboxCount)
				_findNextTextboxSection();
			else
			{
				UT_DEBUGMSG(("End of Textbox marker at pos %d\n", iDocPosition));
				return false;
			}
		}

// 		if(iDocPosition == m_pTextboxes[m_iNextTextbox].txt_pos)
// 		{
// 			const gchar * attribsB[] = {"props", NULL,
// 											"style", NULL,
// 											NULL};

// 			attribsB[1] = m_paraProps.c_str();
// 			attribsB[3] = m_paraStyle.c_str();

// 			_appendStrux(PTX_Block,attribsB);
// 			m_bInPara = true;
// 			return true;
// 		}
		
		xxx_UT_DEBUGMSG(("In Textbox %d, on pos %d\n", m_iNextTextbox, iDocPosition));
	}
	else if(m_bInTextboxes)
	{
		m_bInTextboxes = false;
		UT_DEBUGMSG(("Leaving Textbox territory\n"));
	}
	
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
		xxx_UT_DEBUGMSG(("Error: footnote section not found!!!\n"));
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
/*!
 * s_cmp_lids This function is used to sort the textboxPos lids in order
 * of their lid values. This matches the order of the text sort in the
 * in the out-of-stream table.
 * Used by theqsort method on UT_Vector.
\param const void * P1  - pointer to a textboxPos pointer
\param const void * P2  - pointer to a textboxPos pointer
\returns -ve if sz1 < sz2, 0 if sz1 == sz2, +ve if sz1 > sz2
*/
static UT_sint32 s_cmp_lids(const void * P1, const void * P2)
{
	const textboxPos ** pP1 = (const textboxPos **) P1;
	const textboxPos ** pP2 = (const textboxPos **) P2;
	UT_uint32 lid1 = (*pP1)->lid;
	UT_uint32 lid2 = (*pP2)->lid;
	return static_cast<UT_sint32>(lid1) - static_cast<UT_sint32>(lid2);
}

bool IE_Imp_MsWord_97::_findNextTextboxSection()
{
	if(m_iNextTextbox == 0)
	{
		// move to the start of the doc first
		m_pTextboxEndSection = NULL;
		m_vecTextboxPos.qsort(s_cmp_lids);
		
	}
	if(m_iNextTextbox >= m_vecTextboxPos.getItemCount())
	{
		UT_DEBUGMSG(("Error: Textbox section not found!!!\n"));
		return false;
	}

	textboxPos * pPos = m_vecTextboxPos.getNthItem(m_iNextTextbox);
	m_pTextboxEndSection = pPos->endFrame;

	if(!m_pTextboxEndSection)
	{
		UT_DEBUGMSG(("Error: Textbox section not found!!!\n"));
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
	return ((m_bInFNotes || m_bInENotes) && !m_bInHeaders && !m_bInTextboxes);
}

bool IE_Imp_MsWord_97::_ensureInBlock()
{

  bool bret = true;

  pf_Frag * pf = getDoc()->getLastFrag();
  while(pf && pf->getType() != pf_Frag::PFT_Strux)
    {
      pf = pf->getPrev();
    }
    if(pf && (pf->getType() == pf_Frag::PFT_Strux) )
    {
      pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
      if(pfs->getStruxType() != PTX_Block)
      {
        bret = _appendStrux(PTX_Block, PP_NOPROPS);
	if (bret) m_bInPara = true;
      }
    }
    else if( pf == NULL)
    {
      bret = _appendStrux(PTX_Block, PP_NOPROPS);
      if (bret) m_bInPara = true;
    }

    return bret;
}

bool IE_Imp_MsWord_97::_appendStrux(PTStruxType pts, const PP_PropertyVector & attributes)
{
	if(pts == PTX_SectionFrame)
	{
		UT_DEBUGMSG(("Appending Frame \n"));
	}
	if(pts == PTX_EndFrame)
	{
		UT_DEBUGMSG(("Appending EndFrame \n"));
	}
	if(m_bInHeaders)
	{
		return _appendStruxHdrFtr(pts, attributes);
	}
	else if(_shouldUseInsert() && m_pNotesEndSection)
	{
		return getDoc()->insertStruxBeforeFrag(m_pNotesEndSection, pts, attributes);
	}
	else if(m_bInTextboxes && m_pTextboxEndSection)
	{
		if(pts == PTX_Block)
		{
			xxx_UT_DEBUGMSG(("Insert block in Text box \n"));
		}
		return getDoc()->insertStruxBeforeFrag(m_pTextboxEndSection, pts, attributes);
	}
	if(pts == PTX_SectionFrame)
	{
//		Make sure any pending text is flushed
		_flush();

//
// Text boxes need to be preceded by Blocks
//
		pf_Frag * pf = getDoc()->getLastFrag();
		while(pf && pf->getType() != pf_Frag::PFT_Strux)
		{
			pf = pf->getPrev();
		}
		if(pf && (pf->getType() == pf_Frag::PFT_Strux) )
		{
			pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
			if(pfs->getStruxType() != PTX_Block)
			{
				getDoc()->appendStrux(PTX_Block, PP_NOPROPS);
			}
		}
		else if( pf == NULL)
		{
			getDoc()->appendStrux(PTX_Block, PP_NOPROPS);
		}
	}
	return getDoc()->appendStrux(pts, attributes);
}

bool IE_Imp_MsWord_97::_appendObject(PTObjectType pto, const PP_PropertyVector & attributes)
{
	if(m_bInHeaders)
	{
		return _appendObjectHdrFtr(pto, attributes);
	}
	else if(_shouldUseInsert() && m_pNotesEndSection)
	{
		return getDoc()->insertObjectBeforeFrag(m_pNotesEndSection, pto, attributes);
	}
	else if(m_bInTextboxes && m_pTextboxEndSection)
	{
		return getDoc()->insertObjectBeforeFrag(m_pTextboxEndSection, pto, attributes);
	}
	if(!m_bInPara)
	{
	  _appendStrux(PTX_Block, PP_NOPROPS);
	  m_bInPara = true;
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
	else if(m_bInTextboxes && m_pTextboxEndSection)
	{
		return getDoc()->insertSpanBeforeFrag(m_pTextboxEndSection, p, length);
	}
	return getDoc()->appendSpan(p, length);
}

bool IE_Imp_MsWord_97::_appendFmt(const PP_PropertyVector & attributes)
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
bool IE_Imp_MsWord_97::_appendStruxHdrFtr(PTStruxType pts, const PP_PropertyVector & attributes)
{
	UT_return_val_if_fail(m_bInHeaders,false);
	UT_return_val_if_fail(m_iCurrentHeader < m_iHeadersCount,false);
	UT_DEBUGMSG(("Inserting strux of type %d in HdrFtr %d\n",pts,m_iCurrentHeader));
	UT_ASSERT(m_bInSect);
	bool bRet = true;
	for(UT_sint32 i = 0; i < m_pHeaders[m_iCurrentHeader].d.frag.getItemCount(); i++)
	{
		pf_Frag * pF = (pf_Frag*) m_pHeaders[m_iCurrentHeader].d.frag.getNthItem(i);
		UT_return_val_if_fail(pF,false);
		UT_DEBUGMSG(("Inserting strux of type %d in Dirivative HdrFtr \n",pts));

		bRet &= getDoc()->insertStruxBeforeFrag(pF, pts, attributes);
	}
	
	bRet &= getDoc()->appendStrux(pts, attributes);
	if(pts != PTX_Block)
	{
		xxx_UT_DEBUGMSG(("m_bInPara set false here -1 \n"));
		m_bInPara = false;
	}
	else
	{
		m_bInPara = true;
	}
	return bRet;
}

bool IE_Imp_MsWord_97::_appendObjectHdrFtr(PTObjectType pto, const PP_PropertyVector & attributes)
{
	UT_return_val_if_fail(m_bInHeaders,false);
	UT_return_val_if_fail(m_iCurrentHeader < m_iHeadersCount,false);

	bool bRet = true;

	for(UT_sint32 i = 0; i < m_pHeaders[m_iCurrentHeader].d.frag.getItemCount(); i++)
	{
		pf_Frag * pF = (pf_Frag*) m_pHeaders[m_iCurrentHeader].d.frag.getNthItem(i);
		UT_return_val_if_fail(pF,false);
		if(!m_bInPara)
		{
			bRet &= getDoc()->insertStruxBeforeFrag(pF, PTX_Block, PP_NOPROPS);
		}
		bRet &= getDoc()->insertObjectBeforeFrag(pF, pto, attributes);
	}
	if(!m_bInPara)
	{
		m_bInPara = true;
		bRet &= getDoc()->appendStrux(PTX_Block, PP_NOPROPS);
	}
	bRet &= getDoc()->appendObject(pto, attributes);
	return bRet;
}

bool IE_Imp_MsWord_97::_appendSpanHdrFtr(const UT_UCSChar * p, UT_uint32 length)
{
	UT_return_val_if_fail(m_bInHeaders,false);
	UT_return_val_if_fail(m_iCurrentHeader < m_iHeadersCount,false);

	bool bRet = true;
	
	for(UT_sint32 i = 0; i < m_pHeaders[m_iCurrentHeader].d.frag.getItemCount(); i++)
	{
		pf_Frag * pF = (pf_Frag*) m_pHeaders[m_iCurrentHeader].d.frag.getNthItem(i);
		UT_return_val_if_fail(pF,false);
		if(!m_bInPara)
		{
			bRet &= getDoc()->insertStruxBeforeFrag(pF, PTX_Block, PP_NOPROPS);
		}

		bRet &= getDoc()->insertSpanBeforeFrag(pF, p, length);
	}
	if(!m_bInPara)
	{
		m_bInPara = true;
		bRet &= getDoc()->appendStrux(PTX_Block, PP_NOPROPS);
	}	
	bRet &= getDoc()->appendSpan(p, length);
	return bRet;
}


void IE_Imp_MsWord_97::_handleHeaders(const wvParseStruct *ps)
{
	UT_uint32 i, k;

	DELETEPV(m_pHeaders);

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

	   The even headers are only applied if ps->dop.fFacingPages is set
	*/

	bool bHeaderError = false;

	if(ps->fib.lcbPlcfhdd)
	{
		/* the docs are ambiguous, at one place saying the PLCF
		   contains n+2 entries, another n+1; I think the former is correct*/
		m_iHeadersCount = ps->fib.lcbPlcfhdd/4 - 2;
		try
		{
			m_pHeaders = new header[m_iHeadersCount];
		}
		catch(...)
		{
			m_pHeaders = NULL;
		}

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

				UT_DEBUGMSG(("Header %d has pid %d \n",i,m_pHeaders[i].pid));
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
							if(m_bEvenOddHeaders)
								m_pHeaders[i].type = HF_HeaderEven;
							else
								m_pHeaders[i].type = HF_Unsupported;
							break;
						case 1:
							m_pHeaders[i].type = HF_HeaderOdd;
							break;
						case 2:
							if(m_bEvenOddHeaders)
								m_pHeaders[i].type = HF_FooterEven;
							else
								m_pHeaders[i].type = HF_Unsupported;
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
					if(m_pHeaders[i].type != HF_Unsupported && m_pHeaders[i].len == 0)
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
								// set the type of the present header unsupported, so it does not
								// get referenced
								m_pHeaders[i].type = HF_Unsupported;
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
							// did not find any meaningful headers, set the type to unsupported, so
							// that it does not get referenced
							// 
							// we do not want to do this to the first page hdr/ftr,
							// because in this case len == 0 can mean the header should be
							// empty but present (this is determined by asep->fTitlePage
							if(m_pHeaders[i].type != HF_HeaderFirst && m_pHeaders[i].type != HF_FooterFirst)
								m_pHeaders[i].type = HF_Unsupported;
							
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
    A helper function that inserts the header/ftr section
*/
bool IE_Imp_MsWord_97::_insertHeaderSection(bool bDoBlockIns)
{
	// need to insert our header/footer section, preserving
	// any existing formatting ...

	// we need to be able to insert some 0-length headers
	if(m_pHeaders[m_iCurrentHeader].type != HF_Unsupported /*&& m_pHeaders[m_iCurrentHeader].len > 2*/)
	{
		if(m_iCurrentHeader == m_iLastAppendedHeader)
		{
			return false;
		}
		m_iLastAppendedHeader = m_iCurrentHeader;
		PP_PropertyVector attribsB;
		if(m_paraProps.size())
		{
			attribsB.push_back("props");
			attribsB.push_back(m_paraProps.c_str());
		}
		if(m_paraStyle.size())
		{
			attribsB.push_back("style");
			attribsB.push_back(m_paraStyle.c_str());
		}

		PP_PropertyVector attribsC = {NULL, NULL,
									   NULL, NULL,
									   NULL};
		if(m_charProps.size())
		{
			attribsC.push_back("props");
			attribsC.push_back(m_charProps.c_str());
		}
		if(m_charStyle.size())
		{
			attribsC.push_back("style");
			attribsC.push_back(m_charStyle.c_str());
		}

		std::string id = UT_std_string_sprintf("%d", m_pHeaders[m_iCurrentHeader].pid);
		PP_PropertyVector attribsS = {
			"type", "",
			"id",  id
		};
		UT_DEBUGMSG(("Appending Current Header %d pid %s \n",m_iCurrentHeader,id.c_str()));
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
				UT_ASSERT_HARMLESS(UT_NOT_REACHED);
		}

		// we use the document methods, not the importer methods intentionally
		UT_DEBUGMSG(("Direct Appending HdrFtr in MSWord_import \n"));
		if(!m_bInPara)
		{
			getDoc()->appendStrux(PTX_Block, PP_NOPROPS);
			m_bInPara = true;
		}
		getDoc()->appendStrux(PTX_SectionHdrFtr, attribsS);
		m_bInSect = true;
		m_bInHeaders = true;

		if(bDoBlockIns)
		{
			getDoc()->appendStrux(PTX_Block, attribsB);
			m_bInPara = true;
			_appendFmt(attribsC);
		}

		// now we insert the same for any derivative headers
		// ...
		for (UT_sint32 i = 0; i < m_pHeaders[m_iCurrentHeader].d.hdr.getItemCount(); i++)
		{
			header * pH = (header*)m_pHeaders[m_iCurrentHeader].d.hdr.getNthItem(i);
			UT_return_val_if_fail(pH, true);

			// skip any unsupported headers (we set the type to
			// unsupported when we find out that it is not used by the
			// section to which it belongs)
			if(pH->type == HF_Unsupported)
			{
				continue;
			}

			id = UT_std_string_sprintf("%d", pH->pid);
			attribsS[3] = id;

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
					UT_ASSERT_HARMLESS(UT_NOT_REACHED);
			}
			UT_DEBUGMSG(("Appending Dirivative HdrFtr in MSWord_import \n"));

			getDoc()->appendStrux(PTX_SectionHdrFtr, attribsS);
			m_bInHeaders = true;

			// we need to remember the HdrFtr fragment for
			// later ...
			pf_Frag * pF = getDoc()->getLastFrag();
			UT_return_val_if_fail(pF && pF->getType() == pf_Frag::PFT_Strux, true);

			pf_Frag_Strux * pFS = (pf_Frag_Strux*)pF;
			UT_return_val_if_fail(pFS->getStruxType() == PTX_SectionHdrFtr, true);

			m_pHeaders[m_iCurrentHeader].d.frag.addItem((void*)pF);

			if(bDoBlockIns)
			{
				getDoc()->appendStrux(PTX_Block, attribsB);
				getDoc()->appendFmt(attribsC);
			}
		}

		return true;
	}
	else
	{
		// just gobble the character ...
		m_bInHeaders = true;
		return false;
	}

	return false;
}



/*!
    This function makes sure that the insert is happening at the
    correct place if we are in the header segment.

    \parameter UT_uint32 iDocPosition: character position in the Word
                                       document stream
    \return returns false if the present character is to be skipped,
            true otherwise
*/
bool IE_Imp_MsWord_97::_handleHeadersText(UT_uint32 iDocPosition,bool bDoBlockIns)
{
	if(iDocPosition == m_iPrevHeaderPosition)
	{
		return true;
	}

	if(iDocPosition == m_iHeadersEnd)
	{
		m_iCurrentHeader++;

		if(m_iCurrentHeader < m_iHeadersCount)
		{
			// this is the case where we reached the end of the header segment, but still have
			// some headers in our header array left.
			// if we have any headers other than unsupported, we have to insert them as empty
		
			for(; m_iCurrentHeader < m_iHeadersCount; m_iCurrentHeader++)
			{
				if(m_pHeaders[m_iCurrentHeader].type != HF_Unsupported)
					_insertHeaderSection(bDoBlockIns);
			}
		}
	}
	
	if(iDocPosition >= m_iHeadersStart && iDocPosition < m_iHeadersEnd)
	{
		m_iPrevHeaderPosition = iDocPosition;

		// upon entry into the header-land, we will need to search for
		// the first header/footer section in our document, note that we are
		// in a header section, note at what doc position the current
		// header will end, and then let things run until we reach
		// the end of the header; then we need to search for the next
		// doc section, etc.

		// when we scroll through 0-length headers, we need to remember where we started,
		// so we can insert the hdr section later
		bool bScrolledHeader = false;
		UT_uint32 iOrigHeader = 0;

		if(!m_bInHeaders)
		{
			UT_DEBUGMSG(("In headers territory: pos %d\n", iDocPosition));
			m_bInENotes = false;
			m_bInFNotes = false;

			m_iCurrentHeader = 0;

			// we need to close of any open section
			if(m_bInSect)
			{
				_endSect(NULL,0,NULL,0);
			}

			// some headers can be 0-length, skip them ... (0-length:  len <=2)
			while(m_iCurrentHeader < m_iHeadersCount && m_pHeaders[m_iCurrentHeader].len <= 2)
			{
				bScrolledHeader = true;
				m_iCurrentHeader++;
			}

			m_bInHeaders = true;
		}
		xxx_UT_DEBUGMSG(("CurrentHeader %d HeaderCount %d \n",m_iCurrentHeader,m_iHeadersCount));
		if (m_iCurrentHeader < m_iHeadersCount) {
			if(iDocPosition == m_pHeaders[m_iCurrentHeader].pos +
			   m_pHeaders[m_iCurrentHeader].len)
			{
				// new header, time to move on ...
				m_iCurrentHeader++;
				iOrigHeader = m_iCurrentHeader;

				// some headers can be 0-length, skip them ... (0-length:  len <=2)
				// some 0-length headers we are actually interested in; the 0-length
				// headers we do not care about should already be marked as HF_Unsupported
				while(m_iCurrentHeader < m_iHeadersCount && m_pHeaders[m_iCurrentHeader].type == HF_Unsupported
					  /*m_pHeaders[m_iCurrentHeader].len <= 2*/)
				{
					bScrolledHeader = true;
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
			xxx_UT_DEBUGMSG(("iDocPosition %d m_pHeaders[m_iCurrentHeader].pos %d \n",iDocPosition,m_pHeaders[m_iCurrentHeader].pos));
			if((bScrolledHeader && m_pHeaders[iOrigHeader].pos == iDocPosition) ||
			   (!bScrolledHeader && iDocPosition == m_pHeaders[m_iCurrentHeader].pos))
			{
				return _insertHeaderSection(bDoBlockIns);
			}
		}
		else
		{
			UT_DEBUGMSG(("DOM: bad header joo joo\n"));
			return false;
		}

		// if we got this far, we are somwhere inside the header, just
		// process the character in a normal way
		return (m_pHeaders[m_iCurrentHeader].type != HF_Unsupported);
	}

	return true;
}

/*
   this function returns true if stuff at given position is to be ingored
   For example, the doc might contain headers in it that are not used ...
 */
bool IE_Imp_MsWord_97::_ignorePosition(UT_uint32 iDocPos)
{
	if(m_bInTOC && m_bTOCsupported)
		return true;
	
	if(m_bInHeaders && m_iCurrentHeader < m_iHeadersCount && m_pHeaders)
	{
		if(   m_pHeaders[m_iCurrentHeader].type == HF_Unsupported
		   || iDocPos < m_pHeaders[m_iCurrentHeader].pos)
		{
			return true;
		}
	}
	
	return false;
}
