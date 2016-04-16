/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* Abiword
 * Copyright (C) 2001 Christian Biesinger <cbiesinger@web.de>
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

#include <memory.h>
#include <gsf/gsf.h>
#include <gsf/gsf-input.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-utils.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-msole.h>

#include "ut_types.h"
#include "ut_std_string.h"
#include "ut_string.h"
#include "ut_iconv.h"
#include "ut_vector.h"
#include "ut_debugmsg.h"
#include "ut_units.h"

#include "pd_Document.h"

#include "ie_imp_StarOffice.h"
#include "ie_imp_StarOffice_encodings.h"

#include "docinfo.h"

#include "xap_App.h"
#include "xap_Dialog.h"
#include "xap_Frame.h" // for password dialog
#include "xap_Dlg_Password.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Module.h"

#ifdef DEBUG
#include <errno.h>
#endif

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_sdw_register
#define abi_plugin_unregister abipgn_sdw_unregister
#define abi_plugin_supports_version abipgn_sdw_supports_version
// dll exports break static linking
#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE("SDW")
#endif

// ********************************************************************************
// Mapping of StarOffice attributes to abiword's

struct ColorData {
	UT_uint8 red;
	UT_uint8 green;
	UT_uint8 blue;
};
// from tools/inc/color.hxx#L96 and tools/source/generic/color.cxx#L287
static const ColorData gColors[] =
{
	{ 0, 0, 0 },          // COL_BLACK
	{ 0, 0, 0x80 },       // COL_BLUE
	{ 0, 0x80, 0 },       // COL_GREEN
	{ 0, 0x80, 0x80 },    // COL_CYAN
	{ 0x80, 0, 0 },       // COL_RED
	{ 0x80, 0, 0x80},     // COL_MAGENTA
	{ 0x80, 0x80, 0 },    // COL_BROWN
	{ 0x80, 0x80, 0x80 }, // COL_GRAY
	{ 0xc0, 0xc0, 0xc0 }, // COL_LIGHTGRAY
	{ 0, 0, 0xff },       // COL_LIGHTBLUE
	{ 0, 0xff, 0 },       // COL_LIGHTGREEN
	{ 0, 0xff, 0xff },    // COL_LIGHTCYAN
	{ 0xff, 0, 0 },       // COL_LIGHTRED
	{ 0xff, 0, 0xff },    // COL_LIGHTMAGENTA
	{ 0xff, 0xff, 0 },    // COL_YELLOW
	{ 0xff, 0xff, 0xff }, // COL_WHITE
	{ 0xff, 0xff, 0xff }, // COL_WHITE
	{ 0, 0, 0 },          // COL_BLACK
	{ 0xff, 0xff, 0xff }, // COL_WHITE
	{ 0, 0, 0 },          // COL_BLACK
	{ 0, 0, 0 },          // COL_BLACK
	{ 0xff, 0xff, 0xff }, // COL_WHITE
	{ 0, 0, 0 },          // COL_BLACK
	{ 0xff, 0xff, 0xff }, // COL_WHITE
	{ 0, 0, 0 },          // COL_BLACK
	{ 0xc0, 0xc0, 0xc0 }, // COL_LIGHTGRAY
	{ 0xff, 0xff, 0xff }, // COL_WHITE
	{ 0x80, 0x80, 0x80 }, // COL_GRAY
	{ 0xc0, 0xc0, 0xc0 }, // COL_LIGHTGRAY
	{ 0xff, 0xff, 0xff }, // COL_WHITE
	{ 0, 0, 0 }           // COL_BLACK
};
#define COLOR_SIZE (sizeof(gColors)/sizeof(gColors[0]))

/** Given a data pointer, returns a color string (like cccccc for a medium gray).
 * throws BOGUS_DOCUMENT on error. */
UT_String makeColor(UT_uint8* aData, UT_uint32 aDataLen) {
	// from tools/source/generic/color.cxx line 183ff
	#define COL_NAME_USER		(0x8000)
	#define COL_RED_1B			(0x0001)
	#define COL_RED_2B			(0x0002)
	#define COL_GREEN_1B		(0x0010)
	#define COL_GREEN_2B		(0x0020)
	#define COL_BLUE_1B 		(0x0100)
	#define COL_BLUE_2B 		(0x0200)

	UT_String rv;
	if (aDataLen < 2)
		throw UT_IE_BOGUSDOCUMENT;
	UT_uint16 colorName = GSF_LE_GET_GUINT16(aData);
	if (colorName & COL_NAME_USER) {
		// XXX TODO. Awaiting reply in mailinglist about what CompressMode is.
		return "000000";
	}
	else {
		if (colorName < COLOR_SIZE) {
			UT_String_sprintf(rv, "%02x%02x%02x", gColors[colorName].red,
			                  gColors[colorName].green, gColors[colorName].blue);
			return rv;
		}
		else {
			UT_DEBUGMSG(("SDW: COLOR OUT OF RANGE! has num %u\n", colorName));
			return "000000";
		}
			
	}
}

static double twipsToPoints(UT_uint32 aTwips)
{
  return (double)aTwips/20;
}

static UT_String twipsToSizeString(UT_uint32 aTwips)
{
  return UT_formatDimensionString(DIM_PT, twipsToPoints(aTwips));
}

static char sTabAlignment[] = {
  'L', 'R', 'D', 'C'
};

#ifdef DEBUG
// OPENOFFICE CODE. Using it under the terms of the LGPL.
// compare sw/sw/source/core/sw3io/sw3fmts.cxx
static UT_uint16 lcl_sw3io__CompressWhich(UT_uint16 nWhich)
{
        if( nWhich >= 0x6000 )
        {
                (nWhich -= 0x6000 ) += 110;
                return nWhich;
        }
        if( nWhich >= 0x5000 )
                return nWhich - 0x5000 + 75;
        if( nWhich >= 0x4000 )
                return nWhich - 0x4000 + 55;
        if( nWhich >= 0x3000 )
                return nWhich - 0x3000 + 48;
        if( nWhich >= 0x2000 )
                return nWhich - 0x2000 + 38;
        if( nWhich >= 0x1000 )
                return nWhich - 0x1000 + 1;
        // old documents from before april 21, 1995
        return nWhich;
}
#endif

void streamRead(GsfInput* aStream, TextAttr& aAttr, gsf_off_t aEoa)
    throw(UT_Error)
{
	UT_uint8 flags;
	gsf_off_t newPos;
	readFlagRec(aStream, flags, &newPos);

	streamRead(aStream, aAttr.which);
	streamRead(aStream, aAttr.ver);
	if (flags & 0x10) {
		aAttr.startSet = true;
		streamRead(aStream, aAttr.start);
	}
	else
		aAttr.startSet = false;

	if (flags & 0x20) {
		aAttr.endSet = true;
		streamRead(aStream, aAttr.end);
	}
	else
		aAttr.endSet = false;

	if (gsf_input_seek(aStream, newPos, G_SEEK_SET))
		throw UT_IE_BOGUSDOCUMENT;

	gsf_off_t curPos = gsf_input_tell(aStream);
	if (curPos != aEoa) {
		// there is data
		aAttr.dataLen = aEoa - curPos;
		aAttr.data = new UT_uint8[aAttr.dataLen];
		streamRead(aStream, aAttr.data, aAttr.dataLen);
	}
  // LIST OF THE VALUES: http://ooo.ximian.com/lxr/source/sw/sw/inc/hintids.hxx#086
  // together with http://ooo.ximian.com/lxr/source/sw/sw/source/core/sw3io/sw3fmts.cxx#172
	switch (aAttr.which) {
		case 0x1004: // strikethrough
			aAttr.attrName = "text-decoration";
			if (!aAttr.data || aAttr.data[0])
				aAttr.attrVal = "line-through";
			else
				aAttr.isOff = true;
			break;
		case 0x1005: { // sub-/superscript
			if (aAttr.dataLen < 3)
				break;
			// first byte is size of text % of normal size
			UT_sint16 height = GSF_LE_GET_GINT16(aAttr.data + 1);	
			aAttr.attrName = "text-position";
			if (height > 0)
				aAttr.attrVal = "superscript";
			else if (height < 0)
				aAttr.attrVal = "subscript";
			else
				aAttr.isOff = true;
			break; }
		case 0x1006: { // font family
			if (!aAttr.data || aAttr.dataLen < 7)
				// 7 = 3 byte family etc., 2 byte name length, 2 byte style length
				break;
			aAttr.attrName = "font-family";
			// XXX TODO This code here assumes that the font names are in latin1
			UT_uint16 fontLen = GSF_LE_GET_GUINT16(aAttr.data + 3);
			UT_String_sprintf(aAttr.attrVal, "%.*s", fontLen, (aAttr.data + 5));
			
		break; }
    case 0x1007: // font height
      // structure: | height (2 byte, twips) | prop (?) (2 byte) (if version >= 2, if ver=1 1 byte) | unit (if version>=2) |
      // XXX we ignore "prop" and unit for now, they seem not used much
      aAttr.attrName = "font-size";
      if (aAttr.data)
        aAttr.attrVal = twipsToSizeString(GSF_LE_GET_GUINT16(aAttr.data));
      break;
		case 0x100a: // Italic
			aAttr.attrName = "font-style";
			if (!aAttr.data || aAttr.data[0]) // if there is data, first byte must be != 0
				// abiword doesn't support oblique, so always set italic
				aAttr.attrVal = "italic";
			else
				aAttr.isOff = true;
			break;
		case 0x100d: // Underline
			aAttr.attrName = "text-decoration";
			if (!aAttr.data || aAttr.data[0])
				aAttr.attrVal = "underline";
			else
				aAttr.isOff = true;
			break;
		case 0x100e: // Bold
			aAttr.attrName = "font-weight";
			if (!aAttr.data || aAttr.data[0] >= 8) // 8=Bold.
				aAttr.attrVal = "bold";
			else
				aAttr.isOff = true;
			break;
    case 0x4000: // line spacing
      aAttr.attrName = "line-height";
      aAttr.isPara = true;
      // prop space (s8) | inter space (s16) | height (u16) | rule (s8) | interrule (s8)
      if (aAttr.data && aAttr.dataLen >= 7) {
        // Abiword wants it as float value, StarOffice saves as percentage (e.g.
        // 150 for 1.5)
        float proportionalLineSpace = float(aAttr.data[0])/100;
        // But maybe we need to use the height - stored as twips, need points
        // (used for "exact" and "minimum" line spacing)
        // XXX inter-line spacing not supported by abiword (would be rule=0x00
        // interrule=0x02, value to use=inter space, unit twips)
        UT_String lineHeight = twipsToSizeString(GSF_LE_GET_GINT16(aAttr.data + 3));

        // We'll turn the bytes at 5 and 6 into a single integer, for easier
        // evaluation
        switch (GSF_LE_GET_GUINT16(aAttr.data + 5)) {
          case 0x0100: // proportional
            aAttr.attrVal = std_size_string(proportionalLineSpace);
            break;
          case 0x0001:
          case 0x0002:
            aAttr.attrVal = lineHeight;
            if (aAttr.data[5] == 2) // "minimum" case
              aAttr.attrVal += '+';
            break;
          default:
            UT_DEBUGMSG(("Unsupported linespacing: %02x %02x\n", aAttr.data[5], aAttr.data[6]));
        }
      }
      break;
		case 0x4001: // Alignment
			aAttr.attrName = "text-align";
			aAttr.isPara = true;
			if (aAttr.data) {
				switch (aAttr.data[0]) {
					case 0:
						aAttr.attrVal = "left";
						break;
					case 1:
						aAttr.attrVal = "right";
						break;
					case 2:
					case 4: // BLOCKLINE!? what's BLOCKLINE? I'm guessing justify.
						aAttr.attrVal = "justify";
						break;
					case 3:
						aAttr.attrVal = "center";
						break;
				}
			}
      break;
    case 0x4005: {// Tabstops
      aAttr.attrName = "tabstops";
      aAttr.isPara = true;
      // Data structure:
      // Count(8) | Position (in twips) (32) | Adjustment (8) | Decimal Separator (?) (8) | Fill character (8)
      // (total size per tab = 7)
      // UT_sint8 count = aAttr.data[0];
      for (UT_uint32 i = 1; (i + 6) < aAttr.dataLen; i += 7) {
        // Abiword wants: 12.3cm/L0, where 0 indicates what to fill with
        UT_uint16 posInTwips = GSF_LE_GET_GUINT32(aAttr.data + i);
        UT_String pos = twipsToSizeString(posInTwips);

        aAttr.attrVal += pos;
        aAttr.attrVal += '/';
        if (aAttr.data[i + 4] < sizeof(sTabAlignment)/sizeof(*sTabAlignment))
          aAttr.attrVal += sTabAlignment[aAttr.data[i + 4]];
        else
          aAttr.attrVal += 'L'; // fallback

        char fillIndex = '0';
        // Fill character
        switch (aAttr.data[i + 6]) {
          case '.':
            fillIndex = '1';
            break;
          case '-':
            fillIndex = '2';
            break;
          case '_':
            fillIndex = '3';
            break;
          case ' ':
            fillIndex = '0';
            break;
          default:
            UT_DEBUGMSG(("Filling with '%c' is not supported\n", aAttr.data[i + 6]));
        }
        aAttr.attrVal += fillIndex;
        aAttr.attrVal += ',';
      }
      }
      break;
    default:
      UT_DEBUGMSG(("SDW: unknown attribute 0x%x, compressed %d\n", aAttr.which, lcl_sw3io__CompressWhich(aAttr.which)));
	}

}


// ********************************************************************************
// Password getter, taken from Word97 importer, and other helper functions

#ifdef DEBUG
static void hexdump(void* aPtr, UT_uint32 aLen) {
	unsigned char* ptr = (unsigned char*)aPtr;
	for (UT_uint32 i = 0; i < aLen; i++) {
		fprintf(stderr, "%02x ", ptr[i]);
	}
}
#endif

#define GetPassword() _getPassword ( XAP_App::getApp()->getLastFocussedFrame() )

static UT_String _getPassword (XAP_Frame * pFrame)
{
  UT_String password ( "" );

  if ( pFrame )
    {
      pFrame->raise ();
      
      XAP_DialogFactory * pDialogFactory
	= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());
      
      XAP_Dialog_Password * pDlg = static_cast<XAP_Dialog_Password*>(pDialogFactory->requestDialog(XAP_DIALOG_ID_PASSWORD));
      UT_return_val_if_fail(pDlg,password);
      
      pDlg->runModal (pFrame);
      
      XAP_Dialog_Password::tAnswer ans = pDlg->getAnswer();
      bool bOK = (ans == XAP_Dialog_Password::a_OK);
      
      if (bOK)
  	password = pDlg->getPassword ().utf8_str();
      
      UT_DEBUGMSG(("SDW: Password is %s\n", password.c_str()));
      
      pDialogFactory->releaseDialog(pDlg);
    }

  return password;
}

void readByteString(GsfInput* stream, char*& str, UT_uint16* aLength) 
    throw(UT_Error) 
{
	UT_uint16 length;
	str = NULL;
	streamRead(stream, length);
	str = new char[length + 1];
	if (length)
		streamRead(stream, str, length);
	str[length] = 0;
	if (aLength)
		*aLength = length;
}

void readByteString(GsfInput* stream, UT_UCS4Char*& str, UT_iconv_t converter, SDWCryptor* cryptor) throw(UT_Error) 
{
	UT_uint16 len;
	char* rawString;
	str = NULL;
	readByteString(stream, rawString, &len);
	// decrypt
	if (cryptor)
		cryptor->Decrypt(rawString, rawString, len);

	str = reinterpret_cast<UT_UCS4Char*>(UT_convert_cd(rawString, len + 1, converter, NULL, NULL));
#ifdef DEBUG
	if (!str) {
		UT_DEBUGMSG(("SDW: UT_convert_cd returned %d (%s)\n", errno, strerror(errno)));
		UT_DEBUGMSG(("SDW: Failed string was: \"%s\"\n", rawString));
	}
#endif
	delete[] rawString;
	if (!str)
	    throw UT_IE_NOMEMORY;
}


// ********************************************************************************
// Sniffer

IE_Imp_StarOffice_Sniffer::IE_Imp_StarOffice_Sniffer () :
  IE_ImpSniffer("AbiSDW::StarOffice-Word")
{
  // 
}

// supported suffixes
static IE_SuffixConfidence IE_Imp_StarOffice_Sniffer__SuffixConfidence[] = {
	{ "sdw", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_StarOffice_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_StarOffice_Sniffer__SuffixConfidence;
}

// supported mimetypes
static IE_MimeConfidence IE_Imp_StarOffice_Sniffer__MimeConfidence[] = {
	{ IE_MIME_MATCH_FULL, 	"application/vnd.stardivision.writer",	UT_CONFIDENCE_GOOD 	},	/* seems to be the official mimetype */
	{ IE_MIME_MATCH_FULL, 	"application/x-staroffice-word", 		UT_CONFIDENCE_GOOD 	},	/* aliases */
	{ IE_MIME_MATCH_FULL, 	"application/x-staroffice-words", 		UT_CONFIDENCE_GOOD 	},
	{ IE_MIME_MATCH_FULL, 	"application/x-starwriter", 			UT_CONFIDENCE_GOOD 	},
	{ IE_MIME_MATCH_BOGUS, 	"", 									UT_CONFIDENCE_ZILCH }
};

const IE_MimeConfidence * IE_Imp_StarOffice_Sniffer::getMimeConfidence ()
{
	return IE_Imp_StarOffice_Sniffer__MimeConfidence;
}

static const UT_Byte sdwSignature[] = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};

UT_Confidence_t IE_Imp_StarOffice_Sniffer::recognizeContents(GsfInput * input) {
	GsfInfile * ole;
	UT_Confidence_t confidence = UT_CONFIDENCE_ZILCH;

	ole = gsf_infile_msole_new (input, NULL);
	if (ole)
		{
			GsfInput * starWriterDocument = gsf_infile_child_by_name (ole, "StarWriterDocument");
			if (starWriterDocument != NULL)
				{
					confidence = UT_CONFIDENCE_PERFECT;
					g_object_unref (G_OBJECT (starWriterDocument));
				}

			g_object_unref (G_OBJECT (ole));
		}

	return confidence;
}

UT_Error IE_Imp_StarOffice_Sniffer::constructImporter(PD_Document *pDocument, IE_Imp **ppie) {
	*ppie = new IE_Imp_StarOffice(pDocument);
	if (!ppie)
		return UT_OUTOFMEM;
	return UT_OK;
}

bool IE_Imp_StarOffice_Sniffer::getDlgLabels(const char** pszDesc, const char** pszSuffixList, IEFileType* ft) {
	*pszDesc = "StarWriter up to 5.x (*.sdw)";
	*pszSuffixList = "*.sdw";
	*ft = getFileType();
	return true;
}

// ********************************************************************************
// Header Class

void DocHdr::load(GsfInput* stream) throw(UT_Error) 
{
	UT_DEBUGMSG(("SDW: entering DocHdr::load\n"));
	static const char sw3hdr[] = "SW3HDR";
	static const char sw4hdr[] = "SW4HDR";
	static const char sw5hdr[] = "SW5HDR";
	char header[7];
	streamRead(stream, header, 7);
	if (memcmp(header, sw3hdr, sizeof(sw3hdr)) != 0 &&
	    memcmp(header, sw4hdr, sizeof(sw4hdr)) != 0 &&
	    memcmp(header, sw5hdr, sizeof(sw5hdr)) != 0)
		throw UT_IE_BOGUSDOCUMENT;

	streamRead(stream, cLen);
	streamRead(stream, nVersion);
	streamRead(stream, nFileFlags);
	streamRead(stream, nDocFlags);
	streamRead(stream, nRecSzPos);
	streamRead(stream, nDummy);
	streamRead(stream, nDummy16);
	streamRead(stream, cRedlineMode);
	streamRead(stream, nCompatVer);

	UT_DEBUGMSG(("SDW: clen %i nversion %i fileflags %i docflags %i recszpos %i readlinemode %i compatver %i\n",
			cLen, nVersion, nFileFlags, nDocFlags, nRecSzPos, cRedlineMode, nCompatVer));

	// (see sw/source/core/sw3io/sw3doc.cxx line 700)
	if (nVersion >= SWG_MAJORVERSION && nCompatVer > 0) {
		// File is in a too new format
		throw UT_IE_BOGUSDOCUMENT;
	}
	streamRead(stream, cPasswd, 16);

	streamRead(stream, cSet);
	streamRead(stream, cGui);
	streamRead(stream, nDate);
	streamRead(stream, nTime);
	UT_DEBUGMSG(("SDW: nDate %u nTime %u\n", nDate, nTime));

	// Find the name of the used encoding
	converter = findConverter(cSet);
	if (!UT_iconv_isValid(converter))
		throw UT_ERROR;

	if (nFileFlags & SWGF_BLOCKNAME) {
		char buf[64];
		streamRead(stream, buf, 64);
		UT_DEBUGMSG(("SDW: BLOCKNAME: %.64s\n", buf));
		// XXX verify that the string is really null terminated
		sBlockName = reinterpret_cast<UT_UCS4Char*>(UT_convert_cd(buf, strlen(buf) + 1, converter, NULL, NULL));
	}

	if (nRecSzPos != 0 && nVersion >= SWG_RECSIZES) {
		// Read the Recsizes
		// XXX to be done see sw/source/core/sw3io/sw3imp.cxx#L1070
		UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	}

	if (nFileFlags & SWGF_BAD_FILE)
		throw UT_IE_BOGUSDOCUMENT;

	if (nFileFlags & SWGF_HAS_PASSWD)
		cryptor = new SDWCryptor(nDate, nTime, cPasswd);
	else
		cryptor = NULL;

}

// ********************************************************************************
// Actual Importer

IE_Imp_StarOffice::IE_Imp_StarOffice(PD_Document *pDocument)
	: IE_Imp(pDocument), mOle(NULL), mDocStream(NULL) {
}

IE_Imp_StarOffice::~IE_Imp_StarOffice() {
	if (mDocStream)
		g_object_unref(G_OBJECT(mDocStream));
	if (mOle)
		g_object_unref(G_OBJECT(mOle));
}

void IE_Imp_StarOffice::readRecSize(GsfInput* aStream, UT_uint32& aSize, gsf_off_t* aEOR) throw(UT_Error) {
	// Yes, that's correct, only 3 bytes.
	guint8 buf [3];
	aSize = 0;
	streamRead(aStream, buf, 3);
	// buf content is little endian.
	aSize = buf [0] | (buf[1] << 8) | (buf [2] << 16);
	aSize -= 4; // Substract 4 for the rec type + size
	if (aSize == 0xFFFFFF && mDocHdr.nVersion >= SWG_LONGRECS) {
		// XXX need recsizes from header, see above
		UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	}
	if (aEOR)
		*aEOR = gsf_input_tell(aStream) + aSize;
}

void readFlagRec(GsfInput* stream, UT_uint8& flags, gsf_off_t* newPos) throw(UT_Error) 
{
	streamRead(stream, flags);
	if (newPos)
		*newPos = gsf_input_tell(stream) + (flags & 0xF);
}


UT_Error IE_Imp_StarOffice::_loadFile(GsfInput * input)
{
	try {
		UT_DEBUGMSG(("SDW: Starting import\n"));
		mOle = GSF_INFILE (gsf_infile_msole_new(input, NULL));
		if (!mOle)
			return UT_IE_BOGUSDOCUMENT;

		// firstly, load metadata
		SDWDocInfo::load(mOle, getDoc());

		mDocStream = gsf_infile_child_by_name(mOle, "StarWriterDocument");
		if (!mDocStream)
			return UT_IE_BOGUSDOCUMENT;

		gsf_off_t size = gsf_input_size(mDocStream);

		if (!appendStrux(PTX_Section, PP_NOPROPS))
			return UT_IE_NOMEMORY;

		UT_DEBUGMSG(("SDW: Attempting to load DocHdr...\n"));
		mDocHdr.load(mDocStream);
		UT_DEBUGMSG(("SDW: ...success\n"));

		// Ask for and verify the password
		if (mDocHdr.cryptor) {
			if (!mDocHdr.cryptor->SetPassword(GetPassword().c_str())) {
				UT_DEBUGMSG(("SDW: Wrong password\n"));
				return UT_IE_PROTECTED;
			}
		}

		// do the actual reading
		char type;
		bool done = false;
		UT_uint32 recSize;
		while (!done) {
			if (gsf_input_tell(mDocStream) == size)
				break;
			readChar(mDocStream, type);
			gsf_off_t eor;
			readRecSize(mDocStream, recSize, &eor);

			switch (type) {
				case SWG_CONTENTS: {
					gsf_off_t flagsEnd = 0;
					UT_uint32 nNodes;
					// sw/source/core/sw3io/sw3sectn.cxx#L129
					if (mDocHdr.nVersion >= SWG_LAYFRAMES) {
						UT_uint8 flags;
						readFlagRec(mDocStream, flags, &flagsEnd);
					}
					if (mDocHdr.nVersion >= SWG_LONGIDX)
						streamRead(mDocStream, nNodes);
					else {
						if (mDocHdr.nVersion >= SWG_LAYFRAMES) {
							UT_uint16 sectidDummy;
							streamRead(mDocStream, sectidDummy);
						}
						UT_uint16 nodes16;
						streamRead(mDocStream, nodes16);
						nNodes = (UT_uint32)nodes16;
					}
					if (flagsEnd) {
						UT_ASSERT(flagsEnd >= gsf_input_tell(mDocStream));
						if (gsf_input_tell(mDocStream) != flagsEnd) {
							UT_DEBUGMSG(("SDW: have not read all flags\n"));
							if (gsf_input_seek(mDocStream, flagsEnd, G_SEEK_SET))
								return UT_IE_BOGUSDOCUMENT;
						}
					}
					bool done2 = false;
					UT_uint32 size2;
					while (!done2) {
						readChar(mDocStream, type);
						gsf_off_t eor2;
						readRecSize(mDocStream, size2, &eor2);

						switch (type) {
							case SWG_TEXTNODE: { // sw/source/core/sw3io/sw3nodes.cxx#L788
								UT_DEBUGMSG(("SDW: Found Textnode! (start at 0x%08llX end at 0x%08llX)\n", 
											 (long long)gsf_input_tell(mDocStream), 
											 (long long)eor2));
								UT_uint8 flags;
								gsf_off_t newPos;
								readFlagRec(mDocStream, flags, &newPos);
								// XXX check flags
								if (gsf_input_seek(mDocStream, newPos, G_SEEK_SET))
									return UT_IE_BOGUSDOCUMENT;

								// Read the actual text
								UT_UCS4Char* str;
								readByteString(mDocStream, str);
								UT_UCS4String textNode(str);
								free(str);
								UT_DEBUGMSG(("SDW: ...length=%zu contents are: |%s|\n", textNode.length(), textNode.utf8_str()));

								// now get the attributes
								UT_String attrs;
								UT_String pAttrs;
								UT_Vector charAttributes;
								while (gsf_input_tell(mDocStream) < eor2) {
									char attVal;
									streamRead(mDocStream, attVal);
									UT_uint32 attSize;
									gsf_off_t eoa; // end of attribute
									readRecSize(mDocStream, attSize, &eoa);
									if (attVal == SWG_ATTRIBUTE) {
										TextAttr* a = new TextAttr;
										streamRead(mDocStream, *a, eoa);
										UT_DEBUGMSG(("SDW: ...found text-sub-node, which=0x%x, ver=0x%x, start=%u, end=%u - data:%s len:%llu data is:",
													 a->which, a->ver, a->start,
													 a->end, a->data?"Yes":"No",
													 (long long unsigned)a->dataLen));
#ifdef DEBUG
										hexdump(a->data, a->dataLen);
                    putc('\n', stderr);
#endif
										charAttributes.addItem(a);
									}
									else if (attVal == SWG_ATTRSET) {
									  // bah, yet another loop
										UT_DEBUGMSG(("SDW: ...paragraph attributes found\n"));
										while (gsf_input_tell(mDocStream) < eoa) {
											// reusing attVal and attSize
											streamRead(mDocStream, attVal);
											gsf_off_t eoa2; // end of attribute
											readRecSize(mDocStream, attSize, &eoa2);
											if (attVal == SWG_ATTRIBUTE) {
												TextAttr a;
												streamRead(mDocStream, a, eoa2);
                        if (!a.attrVal.empty()) {
  												if (a.isPara)
	  												UT_String_setProperty(pAttrs, a.attrName, a.attrVal);
		  										else
			  										UT_String_setProperty(attrs, a.attrName, a.attrVal);
                        }
						UT_DEBUGMSG(("SDW: ......found paragraph attr, which=0x%x, ver=0x%x, start=%u, end=%u (string now %s) Data:%s Len=%lld Data:", a.which, a.ver, (a.startSet?a.start:0), (a.endSet?a.end:0), attrs.c_str(), (a.data ? "Yes" : "No"), (long long)a.dataLen));
#ifdef DEBUG
												hexdump(a.data, a.dataLen);
                        putc('\n', stderr);
#endif
											}
											if (gsf_input_seek(mDocStream, eoa2, G_SEEK_SET))
												return UT_IE_BOGUSDOCUMENT;
										}
									}
									else {
										UT_DEBUGMSG(("SDW: ...unknown attribute '%c' found (start=%" GSF_OFF_T_FORMAT " end=%" GSF_OFF_T_FORMAT ")\n", attVal, gsf_input_tell(mDocStream), eoa));
									}
									if (gsf_input_seek(mDocStream, eoa, G_SEEK_SET))
										return UT_IE_BOGUSDOCUMENT;
								}

								PP_PropertyVector attributes = {
									"props",
									pAttrs.c_str()
								};
								// first, insert the paragraph
								if (!appendStrux(PTX_Block, attributes))
									return UT_IE_NOMEMORY;

								UT_String pca(attrs); // character attributes for the whole paragraph
								// now insert the spans of text
								UT_uint32 len = textNode.length();
								UT_uint32 lastInsPos = 0;
								for (UT_uint32 i = 1; i < len; i++) {
									bool doInsert = false; // whether there was an attribute change
									for (UT_sint32 j = 0; j < charAttributes.getItemCount(); j++) {
										const TextAttr* a = reinterpret_cast<const TextAttr*>(charAttributes[j]);
										// clear the last attribute, if set
										if (a->endSet && a->end == (i - 1)) {
											if (a->isOff) {
												UT_String propval = UT_String_getPropVal(pca, a->attrName);
												UT_String_setProperty(attrs, a->attrName, propval);
											}
											else
												UT_String_removeProperty(attrs, a->attrName);
										}

										// now set new attribute, if needed
										if (a->startSet && a->start == (i - 1)) {
											if (a->isPara)
												UT_String_setProperty(pAttrs, a->attrName, a->attrVal);
											else if (a->isOff)
												UT_String_removeProperty(attrs, a->attrName);
											else
												UT_String_setProperty(attrs, a->attrName, a->attrVal);
										}

										// insert if this is the last character, or if there was a format change
										if ((a->endSet && a->end == i) || (a->startSet && a->start == i))
											doInsert = true;
									}
									if (doInsert || i == (len - 1)) {
										attributes[1] = attrs.c_str();
										UT_DEBUGMSG(("SDW: Going to appendFmt with %s\n", attributes[1].c_str()));
										if (!appendFmt(attributes))
											return UT_IE_NOMEMORY; /* leave cast alone! */
										UT_DEBUGMSG(("SDW: About to insert %u-%u\n", lastInsPos, i));
										size_t spanLen = i - lastInsPos;
										if (i == (len - 1)) spanLen++;
										UT_UCS4String span = textNode.substr(lastInsPos, spanLen);
										appendSpan(span.ucs4_str(), spanLen);
										lastInsPos = i;
									}
								}

								UT_VECTOR_PURGEALL(TextAttr*, charAttributes);
								break;

							}
							case SWG_JOBSETUP: {
								// flags are apparently unused here. no idea why they are there.
								gsf_off_t newpos;
								UT_uint8 flags;
								readFlagRec(mDocStream, flags, &newpos);
								if (gsf_input_seek(mDocStream, newpos, G_SEEK_SET))
									return UT_IE_BOGUSDOCUMENT;
								UT_uint16 len, system;
								streamRead(mDocStream, len);
								streamRead(mDocStream, system);
								char printerName[64];
								streamRead(mDocStream, printerName, 64);
								char deviceName[32], portName[32], driverName[32];
								streamRead(mDocStream, deviceName, 32);
								streamRead(mDocStream, portName, 32);
								streamRead(mDocStream, driverName, 32);
								UT_DEBUGMSG(("SDW: Jobsetup: len %u sys 0x%x printer |%.64s| device |%.32s| port |%.32s| driver |%.32s|\n", len, system, printerName, deviceName, portName, driverName));

								if (system == JOBSET_FILE364_SYSTEM || system == JOBSET_FILE605_SYSTEM) {
									UT_uint16 len2, system2;
									streamRead(mDocStream, len2);
									streamRead(mDocStream, system2);
									UT_uint32 ddl; // driver data length
									streamRead(mDocStream, ddl);
									// now the interesting data
									UT_uint16 orient; // 0=portrait 1=landscape
									streamRead(mDocStream, orient);
									UT_uint16 paperBin;
									streamRead(mDocStream, paperBin);
									UT_uint16 paperFormat;
									streamRead(mDocStream, paperFormat);
									UT_uint32 width, height;
									streamRead(mDocStream, width);
									streamRead(mDocStream, height);
									UT_DEBUGMSG(("SDW: orient %u bin %u format %u width %u height %u\n", orient, paperBin, paperFormat, width, height));
									// rest of the data is ignored, seems to be printer specific anyway.
									// Use A4, Portrait by default
									PP_PropertyVector attributes = {
										"pagetype", "a4", // A4/Letter/...
										"orientation", "portrait",
										"width", "210",
										"height", "297",
										"units", "mm"
									};
									const char* sdwPaperToAbi[] = {
										"A3",
										"A4",
										"A5",
										"B4",
										"B5",
										"Letter",
										"Legal",
										"Tabloid/Ledger",
										"Custom"
									};
									if (paperFormat < sizeof(sdwPaperToAbi)/sizeof(*sdwPaperToAbi)) {
										attributes[1] = sdwPaperToAbi[paperFormat];
                                    }
									const char* sdwOrientToAbi[] = {
										"portrait",
										"landscape"
									};
									if (orient < sizeof(sdwOrientToAbi)/sizeof(*sdwOrientToAbi)) {
										attributes[3] = sdwOrientToAbi[orient];
                                    }
									attributes[5] = UT_std_string_sprintf("%f", static_cast<double>(width)/100);
									attributes[7] = UT_std_string_sprintf("%f", static_cast<double>(height)/100);

									getDoc()->setPageSizeFromFile(attributes);
								}
								break;

							}
							case SWG_EOF:
								done2 = true;
								break;
							default:
								UT_DEBUGMSG(("SDW: SWG_CONTENT: Skipping %u bytes for record type '%c' (starting at 0x%08llX)\n",
											 size2, type,
											 (long long)gsf_input_tell(mDocStream)));
						}
						if (gsf_input_seek(mDocStream, eor2, G_SEEK_SET))
							return UT_IE_BOGUSDOCUMENT;
					}
					break;
				}
				case SWG_STRINGPOOL:
				{
					if (mDocHdr.nVersion <= SWG_POOLIDS) {
						UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
						break;
					}
					UT_uint8 encoding;
					streamRead(mDocStream, encoding);
					UT_iconv_t cd = findConverter(encoding);
					if (!UT_iconv_isValid(cd))
						throw UT_IE_IMPORTERROR;
					UT_uint16 count;
					streamRead(mDocStream, count);
					while (count--) {
						UT_uint16 id;
						streamRead(mDocStream, id);
						char* str;
						UT_uint16 len;
						::readByteString(mDocStream, str, &len);
						if (id == IDX_NOCONV_FF) {
							UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
						}
						// FIXME: find a way to not have to copy and free 
						// the result of UT_convert_cd.... --hub
						UT_DEBUGMSG(("SDW: StringPool: found 0x%04x <-> %.*s\n", id, len, str));
						UT_UCS4Char* convertedString = reinterpret_cast<UT_UCS4Char*>(UT_convert_cd(str, len + 1, cd, NULL, NULL));
						mStringPool.insert(stringpool_map::value_type(id, convertedString));
						FREEP(convertedString);
                        delete [] str;
					}
                    UT_iconv_close(cd);
					break;
				}
				case SWG_COMMENT: // skip over comments
					break;
				case SWG_EOF:
					done = true;
					break;
				default:
					UT_DEBUGMSG(("SDW: Skipping %u bytes for record type '%c' (starting at 0x%08llX)\n", recSize, type, (long long)gsf_input_tell(mDocStream)));
			}
			// Seek to the end of the record, in case it wasn't read completely
			if (gsf_input_seek(mDocStream, eor, G_SEEK_SET))
				return UT_IE_BOGUSDOCUMENT;
		}

		UT_DEBUGMSG(("SDW: Done\n"));

		return UT_OK;
	}
	catch(UT_Error e) {
		UT_DEBUGMSG(("SDW: error %d\n", e));
		return e;
	}
	catch(...) {
		UT_DEBUGMSG(("SDW: Unknown error\n"));
		return UT_IE_BOGUSDOCUMENT;
	}
}

/*******************************************************/

// we use a reference-counted sniffer
static IE_Imp_StarOffice_Sniffer * m_impSniffer = 0;

ABI_BUILTIN_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
    if (!m_impSniffer)
    {
    	m_impSniffer = new IE_Imp_StarOffice_Sniffer ();
    }

    mi->name    = "StarOffice .sdw file importer";
    mi->desc    = "Imports StarWriter binary (OLE) documents";
    mi->version = ABI_VERSION_STRING;
    mi->author  = "Christian Biesinger <cbiesinger@web.de>";
    mi->usage   = "No Usage";
  
    IE_Imp::registerImporter (m_impSniffer);

    return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
    mi->name = 0;
    mi->desc = 0;
    mi->version = 0;
    mi->author = 0;
    mi->usage = 0;
  
    UT_ASSERT (m_impSniffer);

    IE_Imp::unregisterImporter (m_impSniffer);
	delete m_impSniffer;
	m_impSniffer = 0;

    return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, 
                                 UT_uint32 /*release*/)
{
    return 1;
}
