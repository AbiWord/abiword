/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 2001 Sean Young <sean@mess.org>
 * Copyright (C) 2001 Hubert Figuiere
 * Copyright (C) 2001 Dom Lachowicz
 * Copyright (C) 2010-2011 Ingo Brueckl
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 *
 * Documentation at http://www.msxnet.org/word2rtf/formats/write
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <gsf/gsf-input.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _MSC_VER
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
#else
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#endif

#include "ie_imp_MSWrite.h"
#include "ap_Args.h"
#include "fg_Graphic.h"
#include "ie_impGraphic.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_locale.h"
#include "ut_types.h"
#include "ut_std_string.h"
#include "xap_Module.h"

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_mswrite_register
#define abi_plugin_unregister abipgn_mswrite_unregister
#define abi_plugin_supports_version abipgn_mswrite_supports_version
#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE("MSWrite")
#endif

// we use a reference-counted sniffer
static IE_Imp_MSWrite_Sniffer *m_sniffer = 0;

/**********************************************************************
 * Plugin Registration                                                *
 **********************************************************************/

ABI_BUILTIN_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo *mi)
{
	if (!m_sniffer) m_sniffer = new IE_Imp_MSWrite_Sniffer();

	mi->name = "MSWrite Importer";
	mi->desc = "Import Microsoft Write Documents";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Sean Young, Ingo Brückl";
	mi->usage = "No Usage";

	IE_Imp::registerImporter(m_sniffer);

	return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo *mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;

	UT_ASSERT(m_sniffer);

	IE_Imp::unregisterImporter(m_sniffer);
	DELETEP(m_sniffer);

	return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, UT_uint32 release)
{
	UT_UNUSED(major);
	UT_UNUSED(minor);
	UT_UNUSED(release);

	return 1;
}

/**********************************************************************
 * File Sniffer                                                       *
 **********************************************************************/

// supported suffix
static const IE_SuffixConfidence IE_Imp_MSWrite_Sniffer__SuffixConfidence[] =
{
	{"wri", UT_CONFIDENCE_PERFECT},
	{"", UT_CONFIDENCE_ZILCH}
};

IE_Imp_MSWrite_Sniffer::IE_Imp_MSWrite_Sniffer ()
	: IE_ImpSniffer("AbiMSWrite::MSWrite")
{
}

bool IE_Imp_MSWrite_Sniffer::getDlgLabels (const char **szDesc,
                                           const char **szSuffixList,
                                           IEFileType *ft)
{
	*szDesc = "Microsoft Write (.wri)";
	*szSuffixList = "*.wri";
	*ft = getFileType();

	return true;
}

const IE_SuffixConfidence *IE_Imp_MSWrite_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_MSWrite_Sniffer__SuffixConfidence;
}

UT_Confidence_t IE_Imp_MSWrite_Sniffer::recognizeContents (const char *szBuf,
                                                           UT_uint32 iNumbytes)
{
	if (iNumbytes > 8)
	{
		if ((szBuf[0] == static_cast<char>(0x31) || szBuf[0] == static_cast<char>(0x32)) &&
		    szBuf[1] == static_cast<char>(0xbe) &&
		    szBuf[2] == static_cast<char>(0) && szBuf[3] == static_cast<char>(0) &&
		    szBuf[4] == static_cast<char>(0) && szBuf[5] == static_cast<char>(0xab))
			return UT_CONFIDENCE_PERFECT;
	}

	return UT_CONFIDENCE_ZILCH;
}

UT_Error IE_Imp_MSWrite_Sniffer::constructImporter (PD_Document *pDocument,
                                                    IE_Imp **ppie)
{
	IE_Imp_MSWrite *p = new IE_Imp_MSWrite(pDocument);

	*ppie = p;

	return UT_OK;
}

/**********************************************************************
 * MSWrite File Structures                                            *
 **********************************************************************/

static const wri_struct WRI_FILE_HEADER[] =
{
	/* value, data, size, type, name */      /* word no. */
	{0, NULL, 2, CT_VALUE, "wIdent"},        // 0
	{0, NULL, 2, CT_VALUE, "dty"},           // 1
	{0, NULL, 2, CT_VALUE, "wTool"},         // 2
	{0, NULL, 2, CT_VALUE, "reserved1"},     // 3
	{0, NULL, 2, CT_VALUE, "reserved2"},     // 4
	{0, NULL, 2, CT_VALUE, "reserved3"},     // 5
	{0, NULL, 2, CT_VALUE, "reserved4"},     // 6
	{0, NULL, 4, CT_VALUE, "fcMac"},         // 7-8
	{0, NULL, 2, CT_VALUE, "pnPara"},        // 9
	{0, NULL, 2, CT_VALUE, "pnFntb"},        // 10
	{0, NULL, 2, CT_VALUE, "pnSep"},         // 11
	{0, NULL, 2, CT_VALUE, "pnSetb"},        // 12
	{0, NULL, 2, CT_VALUE, "pnPgtb"},        // 13
	{0, NULL, 2, CT_VALUE, "pnFfntb"},       // 14
	{0, NULL, 64, CT_IGNORE, "szSsht"},      // 15-47
	{0, NULL, 2, CT_VALUE, "pnMac"},         // 48
	{0, NULL, 0, CT_IGNORE, NULL}            // EOF
};

static const wri_struct WRI_PICTURE_HEADER[] =
{
	/* value, data, size, type, name */         /* word no. */
	// METAFILEPICT structure
	{0, NULL, 2, CT_VALUE, "mm"},               // 0
	{0, NULL, 2, CT_VALUE, "xExt"},             // 1
	{0, NULL, 2, CT_VALUE, "yExt"},             // 2
	{0, NULL, 2, CT_IGNORE, "hMF"},             // 3
	// end of METAFILEPICT structure
	{0, NULL, 2, CT_VALUE, "dxaOffset"},        // 4
	{0, NULL, 2, CT_VALUE, "dxaSize"},          // 5
	{0, NULL, 2, CT_VALUE, "dyaSize"},          // 6
	{0, NULL, 2, CT_VALUE, "cbOldSize"},        // 7
	// BITMAP structure
	{0, NULL, 2, CT_VALUE, "bmType"},           // 8
	{0, NULL, 2, CT_VALUE, "bmWidth"},          // 9
	{0, NULL, 2, CT_VALUE, "bmHeight"},         // 10
	{0, NULL, 2, CT_VALUE, "bmWidthBytes"},     // 11
	{0, NULL, 1, CT_VALUE, "bmPlanes"},         // 12
	{0, NULL, 1, CT_VALUE, "bmBitsPixel"},      // 12.5
	{0, NULL, 4, CT_VALUE, "bmBits"},           // 13-14
	// end of BITMAP structure
	{0, NULL, 2, CT_VALUE, "cbHeader"},         // 15
	{0, NULL, 4, CT_VALUE, "cbSize"},           // 16-17
	{0, NULL, 2, CT_VALUE, "mx"},               // 18
	{0, NULL, 2, CT_VALUE, "my"},               // 19
	{0, NULL, 0, CT_IGNORE, NULL}               // EOF
};

static const wri_struct WRI_OLE_HEADER[] =
{
	/* value, data, size, type, name */       /* word no. */
	{0, NULL, 2, CT_VALUE, "mm"},             // 0
	{0, NULL, 4, CT_IGNORE, "not_used"},      // 1-2
	{0, NULL, 2, CT_VALUE, "objectType"},     // 3
	{0, NULL, 2, CT_VALUE, "dxaOffset"},      // 4
	{0, NULL, 2, CT_VALUE, "dxaSize"},        // 5
	{0, NULL, 2, CT_VALUE, "dyaSize"},        // 6
	{0, NULL, 2, CT_IGNORE, "not_used2"},     // 7
	{0, NULL, 4, CT_VALUE, "dwDataSize"},     // 8-9
	{0, NULL, 4, CT_IGNORE, "not_used3"},     // 10-11
	{0, NULL, 4, CT_VALUE, "dwObjNum"},       // 12-13
	{0, NULL, 2, CT_VALUE, "not_used4"},      // 14
	{0, NULL, 2, CT_VALUE, "cbHeader"},       // 15
	{0, NULL, 4, CT_IGNORE, "not_used5"},     // 16-17
	{0, NULL, 2, CT_VALUE, "mx"},             // 18
	{0, NULL, 2, CT_VALUE, "my"},             // 19
	{0, NULL, 0, CT_IGNORE, NULL}             // EOF
};

#define PIC_OR_OLE_HEADER_SIZE 40   // common size of both header types

// OLE1.0 structure offsets
#define OLE_Version         0
#define OLE_FormatID        4
#define OLE_ClassNameLength 8
#define OLE_ClassNameString 12   // variable length
// offsets relative to empty ClassNameString
#define OLE_TopicNameLength 12
#define OLE_TopicNameString 16
// offsets relative to empty ClassNameString and TopicNameString
#define OLE_ItemNameLength  16
#define OLE_ItemNameString  20
// offsets relative to empty ClassNameString and TopicNameString and ItemNameString
#define OLE_ObjDataSize     20
#define OLE_Object          24
#define OLE_MF_Reserved     24
#define OLE_MF_Object       32

// Bitmap16 structure offsets
#define BM16_Type       0
#define BM16_Width      2
#define BM16_Height     4
#define BM16_WidthBytes 6
#define BM16_Planes     8
#define BM16_BitsPixel  9
#define BM16_Bits       10

/**********************************************************************
 * MSWrite Importer                                                   *
 **********************************************************************/

IE_Imp_MSWrite::IE_Imp_MSWrite (PD_Document *pDocument)
	: IE_Imp(pDocument),
	  mDefaultCodepage("CP1252"),
	  hasHeader(false),
	  hasFooter(false),
	  wri_fonts(0),
	  wri_fonts_count(0),
	  pic_nr(0),
	  lf(false)
{
	setProps(AP_Args::m_impProps);
	const std::string &propCP = getProperty("mswrite-codepage");

	if (!propCP.empty()) mDefaultCodepage = propCP;

	UT_DEBUGMSG(("Codepage: %s\n", mDefaultCodepage.c_str()));

	wri_file_header = static_cast<wri_struct*>(malloc(sizeof(WRI_FILE_HEADER)));
	memcpy(wri_file_header, WRI_FILE_HEADER, sizeof(WRI_FILE_HEADER));

	wri_picture_header = static_cast<wri_struct*>(malloc(sizeof(WRI_PICTURE_HEADER)));
	memcpy(wri_picture_header, WRI_PICTURE_HEADER, sizeof(WRI_PICTURE_HEADER));

	wri_ole_header = static_cast<wri_struct*>(malloc(sizeof(WRI_OLE_HEADER)));
	memcpy(wri_ole_header, WRI_OLE_HEADER, sizeof(WRI_OLE_HEADER));
}

IE_Imp_MSWrite::~IE_Imp_MSWrite()
{
	free_wri_struct(wri_file_header);
	free(wri_file_header);
	free(wri_picture_header);
	free(wri_ole_header);
}

/**********************************************************************
 * MSWrite Importer (file loading)                                    *
 **********************************************************************/

UT_Error IE_Imp_MSWrite::_loadFile (GsfInput *input)
{
	UT_Error result;

	mFile = (GsfInput *) g_object_ref(G_OBJECT(input));

	if (!mFile) return UT_ERROR;

	result = parse_file();

	g_object_unref(G_OBJECT(mFile));

	return result;
}

UT_Error IE_Imp_MSWrite::parse_file ()
{
	int id, size;
	UT_Byte *thetext;

	if (!read_wri_struct(wri_file_header, mFile)) return UT_ERROR;

	UT_DEBUGMSG(("File Header:\n"));
	DEBUG_WRI_STRUCT(wri_file_header);

	id = wri_struct_value(wri_file_header, "wIdent");

	if (id == 0137062)
	{
		// okay, but expect OLE objects
	}
	else if (id != 0137061)
	{
		UT_WARNINGMSG(("parse_file: Not a write file!\n"));
		return UT_ERROR;
	}

	if (wri_struct_value(wri_file_header, "wTool") != 0125400)
	{
		UT_WARNINGMSG(("parse_file: Not a write file!\n"));
		return UT_ERROR;
	}

	size = wri_struct_value(wri_file_header, "fcMac") - 0x80;
	thetext = static_cast<UT_Byte *>(malloc(size));

	if (!thetext)
	{
		UT_WARNINGMSG(("parse_file: Out of memory!\n"));
		return UT_ERROR;
	}

	if (gsf_input_seek(mFile, 0x80, G_SEEK_SET))
	{
		UT_WARNINGMSG(("parse_file: Can't seek data!\n"));
		free(thetext);
		return UT_ERROR;
	}

	gsf_input_read(mFile, size, thetext);

	if (!read_ffntb())
	{
		free(thetext);
		return UT_ERROR;
	}

	mData.truncate(0);
	mData.append(thetext, size);
	free(thetext);

	read_sep();
	read_pap(All);

	UT_DEBUGMSG(("Header: %d, on first page: %d\n", hasHeader, page1Header));

	if (hasHeader)
	{
		_append_hdrftr(header);
		read_pap(Header);

		if (!page1Header) _append_hdrftr(headerfirst);   // an empty one

	}

	UT_DEBUGMSG(("Footer: %d, on first page: %d\n", hasFooter, page1Footer));

	if (hasFooter)
	{
		_append_hdrftr(footer);
		read_pap(Footer);

		if (!page1Footer) _append_hdrftr(footerfirst);   // an empty one

	}

	free_ffntb();

	return UT_OK;
}

/**********************************************************************
 * MSWrite Importer (font handling)                                   *
 **********************************************************************/

bool IE_Imp_MSWrite::read_ffntb ()
{
	int pnFfntb, pnMac, fonts_count = 0, cbFfn, fflen;
	unsigned char buf[2], ffid;
	wri_font *fonts;
	char *ffn;

	UT_DEBUGMSG(("Fonts:\n"));

	pnFfntb = wri_struct_value(wri_file_header, "pnFfntb");
	pnMac = wri_struct_value(wri_file_header, "pnMac");

	// if pnFfntb is the same as pnMac, there are no fonts
	if (pnFfntb == pnMac)
	{
		UT_DEBUGMSG((" (none)\n"));
		return true;
	}

	if (gsf_input_seek(mFile, pnFfntb++ * 0x80, G_SEEK_SET))
	{
		UT_WARNINGMSG(("read_ffntb: Can't seek FFNTB!\n"));
		return false;
	}

	// the first two bytes are the number of fonts
	if (!gsf_input_read(mFile, 2, buf))
	{
		UT_WARNINGMSG(("read_ffntb: Can't read FFNTB!\n"));
		return false;
	}

	wri_fonts_count = READ_WORD(buf);
	UT_DEBUGMSG((" reported: %d\n", wri_fonts_count));

	while (true)
	{
		if (!gsf_input_read(mFile, 2, buf))
		{
			UT_WARNINGMSG(("read_ffntb: Can't read cbFfn!\n"));
			wri_fonts_count = fonts_count;
			free_ffntb();
			return false;
		}

		cbFfn = READ_WORD(buf);

		if (cbFfn == 0) break;

		if (cbFfn == 0xffff)
		{
			if (gsf_input_seek(mFile, pnFfntb++ * 0x80, G_SEEK_SET))
			{
				UT_WARNINGMSG(("read_ffntb: Can't seek next FFNTB!\n"));
				wri_fonts_count = fonts_count;
				free_ffntb();
				return false;
			}

			continue;
		}

		// add one more font
		fonts = (wri_font *) realloc(wri_fonts, (fonts_count + 1) * sizeof(wri_font));

		if (!fonts)
		{
			UT_WARNINGMSG(("read_ffntb: Out of memory!\n"));
			wri_fonts_count = fonts_count;
			free_ffntb();
			return false;
		}

		wri_fonts = fonts;

		// This is the font family identifier. It can either be FF_DONTCARE,
		// FF_ROMAN, FF_SWISS, FF_MODERN, FF_SCRIPT or FF_DECORATIVE. These
		// are defined in <windows.h>, but I don't know what to do with them.
		if (!gsf_input_read(mFile, 1, &ffid))
		{
			UT_WARNINGMSG(("read_ffntb: Can't read ffid!\n"));
			wri_fonts_count = fonts_count;
			free_ffntb();
			return false;
		}

		wri_fonts[fonts_count].ffid = ffid;

		cbFfn--;   // we've already read ffid

		ffn = static_cast<char *>(malloc(cbFfn));

		if (!ffn)
		{
			UT_WARNINGMSG(("read_ffntb: Out of memory!\n"));
			wri_fonts_count = fonts_count;
			free_ffntb();
			return false;
		}

		if (!gsf_input_read(mFile, cbFfn, (guint8 *) ffn))
		{
			UT_WARNINGMSG(("read_ffntb: Can't read szFfn!\n"));
			wri_fonts_count = fonts_count + 1;
			free_ffntb();
			return false;
		}

		wri_fonts[fonts_count].codepage = get_codepage(ffn, &fflen);

		ffn[fflen] = 0;
		wri_fonts[fonts_count].name = ffn;

		UT_DEBUGMSG(("  %2d: %s (%s)\n", fonts_count, wri_fonts[fonts_count].name, wri_fonts[fonts_count].codepage));
		fonts_count++;
	}

	if (fonts_count != wri_fonts_count)
	{
		wri_fonts_count = fonts_count;
		UT_WARNINGMSG(("read_ffntb: Wrong number of fonts.\n"));
	}

	return true;
}

void IE_Imp_MSWrite::free_ffntb ()
{
	for (int i = 0; i < wri_fonts_count; i++)
	{
		free((void *) wri_fonts[i].name);
		wri_fonts[i].name = NULL;
	}

	free(wri_fonts);
	wri_fonts = NULL;
}

/**********************************************************************
 * MSWrite Importer (section property)                                *
 **********************************************************************/

bool IE_Imp_MSWrite::read_sep ()
{
	int pnSep, pnSetb;
	int yaMac, xaMac, yaTop, dyaText, dxaText, rStartPage, yaHeader, yaFooter;
	int cch, yaBot;
	unsigned char sep[0x80];

	UT_DEBUGMSG(("SEP:\n"));

	pnSep = wri_struct_value(wri_file_header, "pnSep");
	pnSetb = wri_struct_value(wri_file_header, "pnSetb");

	// default SEP values
	yaMac = 15840;
	xaMac = 12240;
	yaTop = 1440;
	dyaText = 12960;
	dxaText = 8640;
	xaLeft = 1800;
	rStartPage = 0xFFFF;
	yaHeader = 1080;
	yaFooter = 15760;

	// if the section properties differ from the defaults, get them
	if (pnSep != pnSetb)
	{
		gsf_input_seek(mFile, pnSep * 0x80, G_SEEK_SET);
		gsf_input_read(mFile, 0x80, sep);

		cch = *sep;
		UT_DEBUGMSG((" cch = %d\n", cch));

		if (cch >= 4) yaMac = READ_WORD(sep + 3);
		if (cch >= 6) xaMac = READ_WORD(sep + 5);
		if (cch >= 8) rStartPage = READ_WORD(sep + 7);
		if (cch >= 10) yaTop = READ_WORD(sep + 9);
		if (cch >= 12) dyaText = READ_WORD(sep + 11);
		if (cch >= 14) xaLeft = READ_WORD(sep + 13);
		if (cch >= 16) dxaText = READ_WORD(sep + 15);
		if (cch >= 20) yaHeader = READ_WORD(sep + 19);
		if (cch >= 22) yaFooter = READ_WORD(sep + 21);
	}

	if (rStartPage & 0x8000) rStartPage = -0x10000 + rStartPage;

	UT_DEBUGMSG((" yaMac      = %d\n", yaMac));
	UT_DEBUGMSG((" xaMac      = %d\n", xaMac));
	UT_DEBUGMSG((" rStartPage = %d\n", rStartPage));
	UT_DEBUGMSG((" yaTop      = %d\n", yaTop));
	UT_DEBUGMSG((" dyaText    = %d\n", dyaText));
	UT_DEBUGMSG((" xaLeft     = %d\n", xaLeft));
	UT_DEBUGMSG((" dxaText    = %d\n", dxaText));
	UT_DEBUGMSG((" yaHeader   = %d\n", yaHeader));
	UT_DEBUGMSG((" yaFooter   = %d\n", yaFooter));

	yaBot = yaMac - yaTop - dyaText;
	xaRight = xaMac - xaLeft - dxaText;

	UT_String properties;
	UT_LocaleTransactor lt(LC_NUMERIC, "C");
	UT_String_sprintf(properties, "page-margin-header:%.4fin; "
	                              "page-margin-right:%.4fin; "
	                              "page-margin-left:%.4fin; "
	                              "page-margin-top:%.4fin; "
	                              "page-margin-bottom:%.4fin; "
	                              "page-margin-footer:%.4fin",
	                              static_cast<float>(yaHeader) / 1440.0,
	                              static_cast<float>(xaRight) / 1440.0,
	                              static_cast<float>(xaLeft) / 1440.0,
	                              static_cast<float>(yaTop) / 1440.0,
	                              static_cast<float>(yaBot) / 1440.0,
	                              static_cast<float>(yaMac - yaFooter) / 1440.0);

	if (rStartPage >= 0)
	{
		UT_String tmp;
		UT_String_sprintf(tmp, "; section-restart:1"
		                       "; section-restart-value:%d", rStartPage);
		properties += tmp;
	}

	const PP_PropertyVector attributes = {
		PT_PROPS_ATTRIBUTE_NAME, properties.c_str(),
		PT_HEADERFIRST_ATTRIBUTE_NAME, "0",
		PT_HEADER_ATTRIBUTE_NAME, "1",
		PT_FOOTERFIRST_ATTRIBUTE_NAME, "2",
		PT_FOOTER_ATTRIBUTE_NAME, "3"
	};
	appendStrux(PTX_Section, attributes);

	return true;
}

/**********************************************************************
 * MSWrite Importer (paragraph property)                              *
 **********************************************************************/

bool IE_Imp_MSWrite::read_pap (pap_t process)
{
	static const char *text_align[] = {"left", "center", "right", "justify"};
	int fcMac, pnPara, fcFirst, cfod, fc, fcLim;
	unsigned char page[0x80];
	UT_String properties, tmp, lastprops;

	if (process == All) { UT_DEBUGMSG(("PAP:\n")); }

	fcMac = wri_struct_value(wri_file_header, "fcMac");
	pnPara = wri_struct_value(wri_file_header, "pnPara");

	fcFirst = 0x80;

	while (true)
	{
		gsf_input_seek(mFile, pnPara++ * 0x80, G_SEEK_SET);
		gsf_input_read(mFile, 0x80, page);

		fc = READ_DWORD(page);
		cfod = page[0x7f];

		if (process == All)
		{
			UT_DEBUGMSG((" fcFirst = %d\n", fc));
			UT_DEBUGMSG((" cfod    = %d\n", cfod));
		}

		if (fc != fcFirst) UT_WARNINGMSG(("read_pap: fcFirst wrong.\n"));

		// read all FODs (format descriptors)
		for (int fod = 0; fod < cfod; fod++)
		{
			int bfprop, cch;
			int jc, dxaRight, dxaLeft, dxaLeft1, dyaLine, fGraphics;
			int rhcPage, rHeaderFooter, rhcFirst;
			int tabs, dxaTab[14], jcTab[14];

			if (process == All) { UT_DEBUGMSG(("  PAP-FOD #%02d:\n", fod + 1)); }

			// read a FOD (format descriptor)
			fcLim = READ_DWORD(page + 4 + fod * 6);
			bfprop = READ_WORD(page + 8 + fod * 6);

			if (process == All)
			{
				UT_DEBUGMSG(("   fcLim  = %d\n", fcLim));
				UT_DEBUGMSG((bfprop == 0xffff ? "   bfprop = 0x%04X\n" : "   bfprop = %d\n", bfprop));
			}

			// default PAP values
			jc = 0;
			dxaRight = dxaLeft = dxaLeft1 = 0;
			dyaLine = 240;
			rhcPage = rHeaderFooter = rhcFirst = 0;
			fGraphics = 0;
			tabs = 0;

			// if the PAP FPROPs (formatting properties) differ from the defaults, get them
			if (bfprop != 0xffff && bfprop + (cch = page[bfprop + 4]) < 0x80)
			{
				if (process == All) { UT_DEBUGMSG(("    cch = %d\n", cch)); }

				if (cch >= 2) jc = page[bfprop + 6] & 3;
				if (cch >= 6) dxaRight = READ_WORD(page + bfprop + 9);
				if (cch >= 8) dxaLeft = READ_WORD(page + bfprop + 11);
				if (cch >= 10) dxaLeft1 = READ_WORD(page + bfprop + 13);
				if (cch >= 12) dyaLine = READ_WORD(page + bfprop + 15);

				if (cch >= 17)
				{
					rhcPage = page[bfprop + 21] & 1;
					rHeaderFooter = page[bfprop + 21] & 6;
					rhcFirst = page[bfprop + 21] & 8;
					fGraphics = page[bfprop + 21] & 0x10;
				}

				for (int n = 0; n < 14; n++)
				{
					if (cch >= 4 * (n + 1) + 26)
					{
						dxaTab[tabs] = READ_WORD(page + bfprop + n * 4 + 27);
						jcTab[tabs] = page[bfprop + n * 4 + 29] & 3;
						tabs++;
					}
				}

				if (dxaRight & 0x8000) dxaRight = -0x10000 + dxaRight;
				if (dxaLeft & 0x8000) dxaLeft = -0x10000 + dxaLeft;
				if (dxaLeft1 & 0x8000) dxaLeft1 = -0x10000 + dxaLeft1;
				if (dyaLine < 240) dyaLine = 240;

				if (process == All && rHeaderFooter)
				{
					if (rhcPage)
					{
						if (!hasFooter)
						{
							hasFooter = true;
							page1Footer = rhcFirst;
						}
					}
					else
					{
						if (!hasHeader)
						{
							hasHeader = true;
							page1Header = rhcFirst;
						}
					}
				}
			}

			if ((process == All && !rHeaderFooter) ||
			    (rHeaderFooter && ((process == Header && !rhcPage) ||
			                       (process == Footer && rhcPage))))
			{
				UT_DEBUGMSG(("    jc            = %d\n", jc));
				UT_DEBUGMSG(("    dxaRight      = %d\n", dxaRight));
				UT_DEBUGMSG(("    dxaLeft       = %d\n", dxaLeft));
				UT_DEBUGMSG(("    dxaLeft1      = %d\n", dxaLeft1));
				UT_DEBUGMSG(("    dyaLine       = %d\n", dyaLine));
				UT_DEBUGMSG(("    rhcPage       = %d\n", rhcPage));
				UT_DEBUGMSG(("    rHeaderFooter = %d\n", rHeaderFooter));
				UT_DEBUGMSG(("    rhcFirst      = %d\n", rhcFirst));
				UT_DEBUGMSG(("    fGraphics     = %d\n", fGraphics));

				UT_LocaleTransactor lt(LC_NUMERIC, "C");
				UT_String_sprintf(properties, "text-align:%s; line-height:%.1f",
				                              text_align[jc],
				                              static_cast<float>(dyaLine) / 240.0);

				if (tabs)
				{
					properties += "; tabstops:";
					UT_DEBUGMSG(("    Tabs:\n"));

					for (int n = 0; n < tabs; n++)
					{
						UT_String_sprintf(tmp, "%.4fin/%c0",
						                       static_cast<float>(dxaTab[n]) / 1440.0,
						                       jcTab[n] ? 'D' : 'L');
						properties += tmp;
						UT_DEBUGMSG(("     #%02d dxa = %d, jcTab = %d\n", n + 1, dxaTab[n], jcTab[n]));

						if (n != tabs - 1) properties += ",";
					}
				}

				if (process == Header || process == Footer)
				{
					// For reasons unknown, the left and right margins from the paper
					// are included in the indents of the headers and footers.
					dxaLeft -= xaLeft;
					dxaRight -= xaRight;
				}

				if (dxaLeft1)
				{
					UT_String_sprintf(tmp, "; text-indent:%.4fin",
					                       static_cast<float>(dxaLeft1) / 1440.0);
					properties += tmp;
				}

				if (dxaLeft)
				{
					UT_String_sprintf(tmp, "; margin-left:%.4fin",
					                       static_cast<float>(dxaLeft) / 1440.0);
					properties += tmp;
				}

				if (dxaRight)
				{
					UT_String_sprintf(tmp, "; margin-right:%.4fin",
					                       static_cast<float>(dxaRight) / 1440.0);
					properties += tmp;
				}

				// new attributes, only if there was a line feed or FPROPs have changed
				if (lf || strcmp(properties.c_str(), lastprops.c_str()) != 0)
				{
					const PP_PropertyVector attributes = {
						PT_PROPS_ATTRIBUTE_NAME, properties.c_str()
					};
					appendStrux(PTX_Block, attributes);

					lastprops = properties;
				}

				if (fGraphics) read_pic(fcFirst, fcLim - fcFirst);
				else read_txt(fcFirst, fcLim - 1);
			}

			fcFirst = fcLim;

			if (fcLim >= fcMac)
			{
				UT_DEBUGMSG(("  PAP-FODs end, fcLim (%d) >= fcMac (%d)\n", fcLim, fcMac));
				return true;
			}
		}
	}
}

/**********************************************************************
 * MSWrite Importer (text handling)                                   *
 **********************************************************************/

bool IE_Imp_MSWrite::read_txt (int from, int to)
{
	static const char *currcp;
	int fcMac, pnChar, fcFirst, cfod, fc, fcLim;
	unsigned char page[0x80];
	UT_String properties, tmp;
	int dataLen = static_cast<UT_sint32>(mData.getLength());

	UT_DEBUGMSG(("    TXT:\n"));
	UT_DEBUGMSG(("     from = %d\n", from));
	UT_DEBUGMSG(("     to   = %d\n", to));

	fcMac = wri_struct_value(wri_file_header, "fcMac");
	pnChar = (fcMac + 127) / 128;

	fcFirst = 0x80;

	while (true)
	{
		gsf_input_seek(mFile, pnChar++ * 0x80, G_SEEK_SET);
		gsf_input_read(mFile, 0x80, page);

		fc = READ_DWORD(page);
		cfod = page[0x7f];

		UT_DEBUGMSG(("      fcFirst = %d\n", fc));
		UT_DEBUGMSG(("      cfod    = %d\n", cfod));

		if (fc != fcFirst) UT_WARNINGMSG(("read_txt: fcFirst wrong.\n"));

		// read all FODs (format descriptors)
		for (int fod = 0; fod < cfod; fod++)
		{
			int bfprop, cch, ftc, hps, fBold, fItalic, fUline, hpsPos;

			UT_DEBUGMSG(("       CHP-FOD #%02d:\n", fod + 1));

			// read a FOD (format descriptor)
			fcLim = READ_DWORD(page + 4 + fod * 6);
			bfprop = READ_WORD(page + 8 + fod * 6);

			UT_DEBUGMSG(("        fcLim  = %d\n", fcLim));
			UT_DEBUGMSG((bfprop == 0xffff ? "        bfprop = 0x%04X\n" : "        bfprop = %d\n", bfprop));

			// default CHP values
			ftc = 0;
			hps = 24;
			fBold = fItalic = fUline = hpsPos = 0;

			// if the CHP FPROPs (formatting properties) differ from the defaults, get them
			if (bfprop != 0xffff && bfprop + (cch = page[bfprop + 4]) < 0x80)
			{
				UT_DEBUGMSG(("         cch = %d\n", cch));

				if (cch >= 2) ftc = page[bfprop + 6] >> 2;
				if (cch >= 5) ftc |= (page[bfprop + 9] & 3) << 6;
				if (cch >= 3) hps = page[bfprop + 7];
				if (cch >= 2) fBold = page[bfprop + 6] & 1;
				if (cch >= 2) fItalic = page[bfprop + 6] & 2;
				if (cch >= 4) fUline = page[bfprop + 8] & 1;
				if (cch >= 6) hpsPos = page[bfprop + 10];
			}

			UT_DEBUGMSG(("         ftc           = %d\n", ftc));
			UT_DEBUGMSG(("         hps           = %d\n", hps));
			UT_DEBUGMSG(("         fBold         = %d\n", fBold));
			UT_DEBUGMSG(("         fItalic       = %d\n", fItalic));
			UT_DEBUGMSG(("         fUline        = %d\n", fUline));
			UT_DEBUGMSG(("         hpsPos        = %d\n", hpsPos));

			if (ftc >= wri_fonts_count)
			{
				UT_WARNINGMSG(("read_txt: Wrong font code.\n"));
				ftc = wri_fonts_count - 1;
			}

			if (from < fcLim && to >= fcFirst)
			{
				UT_LocaleTransactor lt(LC_NUMERIC, "C");
				UT_String_sprintf(properties, "font-weight:%s",
				                              fBold ? "bold" : "normal");

				if (hps != 24)
				{
					UT_String_sprintf(tmp, "; font-size:%dpt", hps / 2);
					properties += tmp;
				}

				if (fItalic) properties += "; font-style:italic";
				if (fUline) properties += "; text-decoration:underline";

				if (hpsPos)
				{
					UT_String_sprintf(tmp, "; text-position:%s",
					                       hpsPos < 128 ? "superscript" : "subscript");
					properties += tmp;
				}

				if (wri_fonts_count)
				{
					UT_String_sprintf(tmp, "; font-family:%s", wri_fonts[ftc].name);
					properties += tmp;
				}

				if (wri_fonts[ftc].codepage != currcp /*sic!*/)
				{
					set_codepage(wri_fonts[ftc].codepage);
					currcp = wri_fonts[ftc].codepage;
				}

				mText.clear();
				UT_DEBUGMSG(("         Text: "));

				while (fcFirst <= from && from < fcLim && from <= to && from - 0x80 < dataLen)
					translate_char(*mData.getPointer(from++ - 0x80), mText);

				UT_DEBUGMSG(("\n"));

				// new attributes, only if there was text
				if (mText.size() > 0)
				{
					const UT_UCS4Char *text = mText.ucs4_str(), *p = text;
					size_t txtLen;

					UT_DEBUGMSG(("         Conv: %s\n", mText.utf8_str()));

					PP_PropertyVector attributes = {
						PT_PROPS_ATTRIBUTE_NAME, properties.c_str()
					};
					appendFmt(attributes);

					// check for page number (should only be in header or footer)
					while (*p && *p != (UT_UCS4Char) 0x01) p++;

					if (*p)
					{
						if (p - text) appendSpan(text, p - text);

						attributes.push_back(PT_TYPE_ATTRIBUTE_NAME);
						attributes.push_back("page_number");
						appendObject(PTO_Field, attributes);

						txtLen = mText.size() - (p - text) - 1;
						p++;
					}
					else
					{
						txtLen = mText.size();
						p = text;
					}

					if (txtLen) appendSpan(p, txtLen);
				}
			}

			fcFirst = fcLim;

			if (fcLim >= fcMac || fcFirst > to)
			{
				UT_DEBUGMSG(("       CHP-FODs end, fcLim (%d) >= fcMac (%d) or fcFirst (%d) > to (%d)\n", fcLim, fcMac, fcFirst, to));
				return true;
			}
		}
	}
}

/**********************************************************************
 * MSWrite Importer (character translation)                           *
 **********************************************************************/

static struct ffn_suff_cp
{
	const char *suffix;
	const char *codepage;
}
const ffn_suff_cp_tbl[] =
{
	{"\x03 CE", "CP1250"},
	{"\x04 Cyr", "CP1251"},
	{"\x06 Greek", "CP1253"},
	{"\x04 Tur", "CP1254"},
	{"\x09 (Hebrew)", "CP1255"},
	{"\x09 (Arabic)", "CP1256"},
	{"\x07 Baltic", "CP1257"},
	{NULL, NULL}
};

const char *IE_Imp_MSWrite::get_codepage (const char *facename, int *facelen) const
{
	const struct ffn_suff_cp *f = ffn_suff_cp_tbl;
	int l = strlen(facename);

	while (f->suffix)
	{
		if (*f->suffix < l)
		{
			if (g_ascii_strcasecmp(&f->suffix[1], &facename[l - *f->suffix]) == 0)
			{
				*facelen = l - *f->suffix;
				return f->codepage;
			}
		}

		f++;
	}

	*facelen = l;
	return mDefaultCodepage.c_str();
}

void IE_Imp_MSWrite::set_codepage (const char *charset)
{
	charconv.setInCharset(charset);
}

void IE_Imp_MSWrite::translate_char (const UT_Byte ch, UT_UCS4String &buf)
{
	UT_UCS4Char uch = ch;

	lf = false;

	switch (ch)
	{
		case 9:
			buf += UCS_TAB;
			break;

		case 12:
			buf += UCS_FF;
			break;

		case 10:     // line feed
			lf = true;

		case 13:     // carriage return
		case 31:     // soft hyphen
			break;

		default:
			if (ch & 0x80) charconv.mbtowc(uch, ch);
			buf += uch;
			break;
	}

	UT_DEBUGMSG((ch < ' ' ? "\\x%x" : "%c", ch));
}

/**********************************************************************
 * MSWrite Importer (picture handling)                                *
 **********************************************************************/

bool IE_Imp_MSWrite::read_pic (int from, int size)
{
	// 8 bit colormap (plus one extra color only in the 4 bit colormap)
	static const UT_Byte bgr_palette[257][4] =
	{
		{0x00, 0x00, 0x00, 0x00},
		{0x40, 0x00, 0x00, 0x00},
		{0x80, 0x00, 0x00, 0x00},
		{0xff, 0x00, 0x00, 0x00},
		{0x00, 0x20, 0x00, 0x00},
		{0x40, 0x20, 0x00, 0x00},
		{0x80, 0x20, 0x00, 0x00},
		{0xff, 0x20, 0x00, 0x00},
		{0x00, 0x40, 0x00, 0x00},
		{0x40, 0x40, 0x00, 0x00},
		{0x80, 0x40, 0x00, 0x00},
		{0xff, 0x40, 0x00, 0x00},
		{0x00, 0x60, 0x00, 0x00},
		{0x40, 0x60, 0x00, 0x00},
		{0x80, 0x60, 0x00, 0x00},
		{0xff, 0x60, 0x00, 0x00},
		{0x00, 0x80, 0x00, 0x00},
		{0x40, 0x80, 0x00, 0x00},
		{0x80, 0x80, 0x00, 0x00},
		{0xff, 0x80, 0x00, 0x00},
		{0x00, 0xa0, 0x00, 0x00},
		{0x40, 0xa0, 0x00, 0x00},
		{0x80, 0xa0, 0x00, 0x00},
		{0xff, 0xa0, 0x00, 0x00},
		{0x00, 0xc0, 0x00, 0x00},
		{0x40, 0xc0, 0x00, 0x00},
		{0x80, 0xc0, 0x00, 0x00},
		{0xff, 0xc0, 0x00, 0x00},
		{0x00, 0xff, 0x00, 0x00},
		{0x40, 0xff, 0x00, 0x00},
		{0x80, 0xff, 0x00, 0x00},
		{0xff, 0xff, 0x00, 0x00},
		{0x00, 0x00, 0x20, 0x00},
		{0x40, 0x00, 0x20, 0x00},
		{0x80, 0x00, 0x20, 0x00},
		{0xff, 0x00, 0x20, 0x00},
		{0x00, 0x20, 0x20, 0x00},
		{0x40, 0x20, 0x20, 0x00},
		{0x80, 0x20, 0x20, 0x00},
		{0xff, 0x20, 0x20, 0x00},
		{0x00, 0x40, 0x20, 0x00},
		{0x40, 0x40, 0x20, 0x00},
		{0x80, 0x40, 0x20, 0x00},
		{0xff, 0x40, 0x20, 0x00},
		{0x00, 0x60, 0x20, 0x00},
		{0x40, 0x60, 0x20, 0x00},
		{0x80, 0x60, 0x20, 0x00},
		{0xff, 0x60, 0x20, 0x00},
		{0x00, 0x80, 0x20, 0x00},
		{0x40, 0x80, 0x20, 0x00},
		{0x80, 0x80, 0x20, 0x00},
		{0xff, 0x80, 0x20, 0x00},
		{0x00, 0xa0, 0x20, 0x00},
		{0x40, 0xa0, 0x20, 0x00},
		{0x80, 0xa0, 0x20, 0x00},
		{0xff, 0xa0, 0x20, 0x00},
		{0x00, 0xc0, 0x20, 0x00},
		{0x40, 0xc0, 0x20, 0x00},
		{0x80, 0xc0, 0x20, 0x00},
		{0xff, 0xc0, 0x20, 0x00},
		{0x00, 0xff, 0x20, 0x00},
		{0x40, 0xff, 0x20, 0x00},
		{0x80, 0xff, 0x20, 0x00},
		{0xff, 0xff, 0x20, 0x00},
		{0x00, 0x00, 0x40, 0x00},
		{0x40, 0x00, 0x40, 0x00},
		{0x80, 0x00, 0x40, 0x00},
		{0xff, 0x00, 0x40, 0x00},
		{0x00, 0x20, 0x40, 0x00},
		{0x40, 0x20, 0x40, 0x00},
		{0x80, 0x20, 0x40, 0x00},
		{0xff, 0x20, 0x40, 0x00},
		{0x00, 0x40, 0x40, 0x00},
		{0x40, 0x40, 0x40, 0x00},
		{0x80, 0x40, 0x40, 0x00},
		{0xff, 0x40, 0x40, 0x00},
		{0x00, 0x60, 0x40, 0x00},
		{0x40, 0x60, 0x40, 0x00},
		{0x80, 0x60, 0x40, 0x00},
		{0xff, 0x60, 0x40, 0x00},
		{0x00, 0x80, 0x40, 0x00},
		{0x40, 0x80, 0x40, 0x00},
		{0x80, 0x80, 0x40, 0x00},
		{0xff, 0x80, 0x40, 0x00},
		{0x00, 0xa0, 0x40, 0x00},
		{0x40, 0xa0, 0x40, 0x00},
		{0x80, 0xa0, 0x40, 0x00},
		{0xff, 0xa0, 0x40, 0x00},
		{0x00, 0xc0, 0x40, 0x00},
		{0x40, 0xc0, 0x40, 0x00},
		{0x80, 0xc0, 0x40, 0x00},
		{0xff, 0xc0, 0x40, 0x00},
		{0x00, 0xff, 0x40, 0x00},
		{0x40, 0xff, 0x40, 0x00},
		{0x80, 0xff, 0x40, 0x00},
		{0xff, 0xff, 0x40, 0x00},
		{0x00, 0x00, 0x60, 0x00},
		{0x40, 0x00, 0x60, 0x00},
		{0x80, 0x00, 0x60, 0x00},
		{0xff, 0x00, 0x60, 0x00},
		{0x00, 0x20, 0x60, 0x00},
		{0x40, 0x20, 0x60, 0x00},
		{0x80, 0x20, 0x60, 0x00},
		{0xff, 0x20, 0x60, 0x00},
		{0x00, 0x40, 0x60, 0x00},
		{0x40, 0x40, 0x60, 0x00},
		{0x80, 0x40, 0x60, 0x00},
		{0xff, 0x40, 0x60, 0x00},
		{0x00, 0x60, 0x60, 0x00},
		{0x40, 0x60, 0x60, 0x00},
		{0x80, 0x60, 0x60, 0x00},
		{0xff, 0x60, 0x60, 0x00},
		{0x00, 0x80, 0x60, 0x00},
		{0x40, 0x80, 0x60, 0x00},
		{0x80, 0x80, 0x60, 0x00},
		{0xff, 0x80, 0x60, 0x00},
		{0x00, 0xa0, 0x60, 0x00},
		{0x40, 0xa0, 0x60, 0x00},
		{0x80, 0xa0, 0x60, 0x00},
		{0xff, 0xa0, 0x60, 0x00},
		{0x00, 0xc0, 0x60, 0x00},
		{0x40, 0xc0, 0x60, 0x00},
		{0x80, 0xc0, 0x60, 0x00},
		{0xff, 0xc0, 0x60, 0x00},
		{0x00, 0xff, 0x60, 0x00},
		{0x40, 0xff, 0x60, 0x00},
		{0x80, 0xff, 0x60, 0x00},
		{0xff, 0xff, 0x60, 0x00},
		{0x00, 0x00, 0x80, 0x00},
		{0x40, 0x00, 0x80, 0x00},
		{0x80, 0x00, 0x80, 0x00},
		{0xff, 0x00, 0x80, 0x00},
		{0x00, 0x20, 0x80, 0x00},
		{0x40, 0x20, 0x80, 0x00},
		{0x80, 0x20, 0x80, 0x00},
		{0xff, 0x20, 0x80, 0x00},
		{0x00, 0x40, 0x80, 0x00},
		{0x40, 0x40, 0x80, 0x00},
		{0x80, 0x40, 0x80, 0x00},
		{0xff, 0x40, 0x80, 0x00},
		{0x00, 0x60, 0x80, 0x00},
		{0x40, 0x60, 0x80, 0x00},
		{0x80, 0x60, 0x80, 0x00},
		{0xff, 0x60, 0x80, 0x00},
		{0x00, 0x80, 0x80, 0x00},
		{0x40, 0x80, 0x80, 0x00},
		{0x80, 0x80, 0x80, 0x00},
		{0xff, 0x80, 0x80, 0x00},
		{0x00, 0xa0, 0x80, 0x00},
		{0x40, 0xa0, 0x80, 0x00},
		{0x80, 0xa0, 0x80, 0x00},
		{0xff, 0xa0, 0x80, 0x00},
		{0x00, 0xc0, 0x80, 0x00},
		{0x40, 0xc0, 0x80, 0x00},
		{0x80, 0xc0, 0x80, 0x00},
		{0xff, 0xc0, 0x80, 0x00},
		{0x00, 0xff, 0x80, 0x00},
		{0x40, 0xff, 0x80, 0x00},
		{0x80, 0xff, 0x80, 0x00},
		{0xff, 0xff, 0x80, 0x00},
		{0x00, 0x00, 0xa0, 0x00},
		{0x40, 0x00, 0xa0, 0x00},
		{0x80, 0x00, 0xa0, 0x00},
		{0xff, 0x00, 0xa0, 0x00},
		{0x00, 0x20, 0xa0, 0x00},
		{0x40, 0x20, 0xa0, 0x00},
		{0x80, 0x20, 0xa0, 0x00},
		{0xff, 0x20, 0xa0, 0x00},
		{0x00, 0x40, 0xa0, 0x00},
		{0x40, 0x40, 0xa0, 0x00},
		{0x80, 0x40, 0xa0, 0x00},
		{0xff, 0x40, 0xa0, 0x00},
		{0x00, 0x60, 0xa0, 0x00},
		{0x40, 0x60, 0xa0, 0x00},
		{0x80, 0x60, 0xa0, 0x00},
		{0xff, 0x60, 0xa0, 0x00},
		{0x00, 0x80, 0xa0, 0x00},
		{0x40, 0x80, 0xa0, 0x00},
		{0x80, 0x80, 0xa0, 0x00},
		{0xff, 0x80, 0xa0, 0x00},
		{0x00, 0xa0, 0xa0, 0x00},
		{0x40, 0xa0, 0xa0, 0x00},
		{0x80, 0xa0, 0xa0, 0x00},
		{0xff, 0xa0, 0xa0, 0x00},
		{0x00, 0xc0, 0xa0, 0x00},
		{0x40, 0xc0, 0xa0, 0x00},
		{0x80, 0xc0, 0xa0, 0x00},
		{0xff, 0xc0, 0xa0, 0x00},
		{0x00, 0xff, 0xa0, 0x00},
		{0x40, 0xff, 0xa0, 0x00},
		{0x80, 0xff, 0xa0, 0x00},
		{0xff, 0xff, 0xa0, 0x00},
		{0x00, 0x00, 0xc0, 0x00},
		{0x40, 0x00, 0xc0, 0x00},
		{0x80, 0x00, 0xc0, 0x00},
		{0xff, 0x00, 0xc0, 0x00},
		{0x00, 0x20, 0xc0, 0x00},
		{0x40, 0x20, 0xc0, 0x00},
		{0x80, 0x20, 0xc0, 0x00},
		{0xff, 0x20, 0xc0, 0x00},
		{0x00, 0x40, 0xc0, 0x00},
		{0x40, 0x40, 0xc0, 0x00},
		{0x80, 0x40, 0xc0, 0x00},
		{0xff, 0x40, 0xc0, 0x00},
		{0x00, 0x60, 0xc0, 0x00},
		{0x40, 0x60, 0xc0, 0x00},
		{0x80, 0x60, 0xc0, 0x00},
		{0xff, 0x60, 0xc0, 0x00},
		{0x00, 0x80, 0xc0, 0x00},
		{0x40, 0x80, 0xc0, 0x00},
		{0x80, 0x80, 0xc0, 0x00},
		{0xff, 0x80, 0xc0, 0x00},
		{0x00, 0xa0, 0xc0, 0x00},
		{0x40, 0xa0, 0xc0, 0x00},
		{0x80, 0xa0, 0xc0, 0x00},
		{0xff, 0xa0, 0xc0, 0x00},
		{0x00, 0xc0, 0xc0, 0x00},
		{0x40, 0xc0, 0xc0, 0x00},
		{0x80, 0xc0, 0xc0, 0x00},
		{0xff, 0xc0, 0xc0, 0x00},
		{0x00, 0xff, 0xc0, 0x00},
		{0x40, 0xff, 0xc0, 0x00},
		{0x80, 0xff, 0xc0, 0x00},
		{0xff, 0xff, 0xc0, 0x00},
		{0x00, 0x00, 0xff, 0x00},
		{0x40, 0x00, 0xff, 0x00},
		{0x80, 0x00, 0xff, 0x00},
		{0xff, 0x00, 0xff, 0x00},
		{0x00, 0x20, 0xff, 0x00},
		{0x40, 0x20, 0xff, 0x00},
		{0x80, 0x20, 0xff, 0x00},
		{0xff, 0x20, 0xff, 0x00},
		{0x00, 0x40, 0xff, 0x00},
		{0x40, 0x40, 0xff, 0x00},
		{0x80, 0x40, 0xff, 0x00},
		{0xff, 0x40, 0xff, 0x00},
		{0x00, 0x60, 0xff, 0x00},
		{0x40, 0x60, 0xff, 0x00},
		{0x80, 0x60, 0xff, 0x00},
		{0xff, 0x60, 0xff, 0x00},
		{0x00, 0x80, 0xff, 0x00},
		{0x40, 0x80, 0xff, 0x00},
		{0x80, 0x80, 0xff, 0x00},
		{0xff, 0x80, 0xff, 0x00},
		{0x00, 0xa0, 0xff, 0x00},
		{0x40, 0xa0, 0xff, 0x00},
		{0x80, 0xa0, 0xff, 0x00},
		{0xff, 0xa0, 0xff, 0x00},
		{0x00, 0xc0, 0xff, 0x00},
		{0x40, 0xc0, 0xff, 0x00},
		{0x80, 0xc0, 0xff, 0x00},
		{0xff, 0xc0, 0xff, 0x00},
		{0x00, 0xff, 0xff, 0x00},
		{0x40, 0xff, 0xff, 0x00},
		{0x80, 0xff, 0xff, 0x00},
		{0xff, 0xff, 0xff, 0x00},
		{0xc0, 0xc0, 0xc0, 0x00}    // a color only in the 4 bit colormap
	};

	// indices (of the 8 bit colormap above) for the 4 bit colormap
	static const int c16idx[16] = {  0, 128, 16, 144, 2, 130, 18, 256,
	                               146, 224, 28, 252, 3, 227, 31, 255 };

	// structs without alignment
#pragma pack(push)
#pragma pack(1)
	struct
	{
		char bfType[2];
		uint32_t bfSize;
		uint32_t bfReserved;
		uint32_t bfOffBits;
	} bmpheader;

	struct
	{
		uint32_t header_sz;
		uint32_t width;
		uint32_t height;
		uint16_t nplanes;
		uint16_t bitspp;
		uint32_t compress_type;
		uint32_t bmp_bytesz;
		uint32_t hres;
		uint32_t vres;
		uint32_t ncolors;
		uint32_t nimpcolors;
	} bmpinfo;
#pragma pack(pop)

	unsigned char page[0x80];
	int mm, cbHeader, cbSize, bmBitsPixel, objectType;
	int colorPaletteLen = 0, bmW = 0, bmH = 0;
	int chunk, filler, written, ole_offset, formatID, objLen, bhSize, bppOff;
	wri_struct *write_pic = NULL;
	UT_ByteBuf pic;
	IEGraphicFileType iegft = IEGFT_Unknown;
	const char *msg = NULL, *className = "";

	if (size < PIC_OR_OLE_HEADER_SIZE + OLE_ClassNameString)
	{
		UT_WARNINGMSG(("read_pic: Size error, type 1!\n"));
		return false;
	}

	// prepare bmp file header
	memset(&bmpheader, 0, sizeof(bmpheader));
	bmpheader.bfType[0] = 'B';
	bmpheader.bfType[1] = 'M';
	memset(&bmpinfo, 0, sizeof(bmpinfo));

	gsf_input_seek(mFile, from, G_SEEK_SET);
	gsf_input_read(mFile, sizeof(page), page);

	mm = READ_WORD(page);

	switch (mm)
	{
		case 0x88:   // wmf file
		case 0xe3:   // bitmap

			write_pic = wri_picture_header;
			read_wri_struct_mem(write_pic, page);

			UT_DEBUGMSG(("    PIC (wmf/bmp) Header:\n"));
			DEBUG_WRI_STRUCT(write_pic, 5);

			cbHeader = wri_struct_value(write_pic, "cbHeader");
			cbSize = wri_struct_value(write_pic, "cbSize");

			if (size < cbHeader + cbSize)
			{
				msg = "Size error, type 2";
				break;
			}

			if (mm == 0xe3)   // bitmap needs header
			{
				if ((bmBitsPixel = wri_struct_value(write_pic, "bmBitsPixel")) < 16)
					colorPaletteLen = (1 << bmBitsPixel) << 2;

				if (colorPaletteLen != 8)
				{
					msg = "Color palette error";
					break;
				}

				// make a bitmap file header

				WRITE_DWORD(bmpheader.bfOffBits, sizeof(bmpheader) + sizeof(bmpinfo) + colorPaletteLen);

				pic.append(reinterpret_cast<UT_Byte *>(&bmpheader), sizeof(bmpheader));

				bmW = wri_struct_value(write_pic, "bmWidth");
				bmH = wri_struct_value(write_pic, "bmHeight");

				if (bmH & 0x8000) bmH = -0x10000 + bmH;

				WRITE_DWORD(bmpinfo.header_sz, sizeof(bmpinfo));
				WRITE_DWORD(bmpinfo.width, bmW);
				WRITE_DWORD(bmpinfo.height, -bmH);
				WRITE_WORD(bmpinfo.nplanes, wri_struct_value(write_pic, "bmPlanes"));
				WRITE_WORD(bmpinfo.bitspp, bmBitsPixel);

				pic.append(reinterpret_cast<UT_Byte *>(&bmpinfo), sizeof(bmpinfo));

				// add monochrome colormap
				pic.append(bgr_palette[0], colorPaletteLen >> 1);
				pic.append(bgr_palette[255], colorPaletteLen >> 1);

				chunk = wri_struct_value(write_pic, "bmWidthBytes");
				filler = (4 - (chunk & 3)) & 3;
			}
			else
			{
				iegft = IEGFT_WMF;
				chunk = cbSize;
				filler = 0;
			}

			written = 0;

			while (written < cbSize)
			{
				// append data
				pic.append(mData.getPointer(from - 0x80 + cbHeader + written), chunk);

				for (int i = 0; i < filler; i++)
					pic.append(reinterpret_cast<const UT_Byte *>("\x00"), 1);

				written += chunk;
			}

			break;

		case 0xe4:   // ole object

			write_pic = wri_ole_header;
			read_wri_struct_mem(write_pic, page);

			UT_DEBUGMSG(("    PIC (OLE) Header:\n"));
			DEBUG_WRI_STRUCT(write_pic, 5);

			objectType = wri_struct_value(write_pic, "objectType");
			cbHeader = wri_struct_value(write_pic, "cbHeader");

			if ((ole_offset = READ_DWORD(page + cbHeader + OLE_ClassNameLength)) > 0)
				className = reinterpret_cast<const char *>(page + cbHeader + OLE_ClassNameString);

			if ((formatID = READ_DWORD(page + cbHeader + OLE_FormatID)) == 1 ||
			    formatID == 2)
			{
				if (size < cbHeader + OLE_TopicNameString + ole_offset)
				{
					msg = "Size error, type 3";
					break;
				}

				ole_offset += READ_DWORD(page + cbHeader + OLE_TopicNameLength + ole_offset);

				if (size < cbHeader + OLE_ItemNameString + ole_offset)
				{
					msg = "Size error, type 4";
					break;
				}

				ole_offset += READ_DWORD(page + cbHeader + OLE_ItemNameLength + ole_offset);
			}

			objLen = READ_DWORD(page + cbHeader + OLE_ObjDataSize + ole_offset);

			if (strcmp(className, "METAFILEPICT") == 0)
			{
				iegft = IEGFT_WMF;
				ole_offset += OLE_MF_Object - OLE_Object;
				objLen -= OLE_MF_Object - OLE_Object;
				objectType = 2;   // we can go embedded now
			}

			if (size <= cbHeader + OLE_Object + ole_offset ||
			    cbHeader + OLE_Object + ole_offset > 0x80 ||
			    size < cbHeader + OLE_Object + ole_offset + objLen)
			{
				msg = "Size error, type 5";
				break;
			}

			// static
			if (objectType == 1)
			{
				if (strcmp(className, "DIB") == 0)
				{
					bhSize = READ_DWORD(page + cbHeader + OLE_Object + ole_offset);
					bppOff = (bhSize == 12 ? 10 : 14);   // different header info types

					if ((bmBitsPixel = READ_WORD(page + cbHeader + OLE_Object + ole_offset + bppOff)) < 16)
						colorPaletteLen = (1 << bmBitsPixel) << 2;

					// make a bitmap file header
					WRITE_DWORD(bmpheader.bfOffBits, sizeof(bmpheader) + bhSize + colorPaletteLen);

					pic.append(reinterpret_cast<UT_Byte *>(&bmpheader), sizeof(bmpheader));

					objectType = 2;   // we can go embedded now
				}

				if (strcmp(className, "BITMAP") == 0)
				{
					if ((bmBitsPixel = *(page + cbHeader + OLE_Object + ole_offset + BM16_BitsPixel)) < 16)
						colorPaletteLen = (1 << bmBitsPixel) << 2;

					// make a bitmap file header

					WRITE_DWORD(bmpheader.bfOffBits, sizeof(bmpheader) + sizeof(bmpinfo) + colorPaletteLen);

					pic.append(reinterpret_cast<UT_Byte *>(&bmpheader), sizeof(bmpheader));

					bmH = READ_WORD(page + cbHeader + OLE_Object + ole_offset + BM16_Height);

					if (bmH & 0x8000) bmH = -0x10000 + bmH;

					WRITE_DWORD(bmpinfo.header_sz, sizeof(bmpinfo));
					WRITE_DWORD(bmpinfo.width, READ_WORD(page + cbHeader + OLE_Object + ole_offset + BM16_Width));
					WRITE_DWORD(bmpinfo.height, bmH);
					WRITE_WORD(bmpinfo.nplanes, *(page + cbHeader + OLE_Object + ole_offset + BM16_Planes));
					WRITE_WORD(bmpinfo.bitspp, bmBitsPixel);

					pic.append(reinterpret_cast<UT_Byte *>(&bmpinfo), sizeof(bmpinfo));

					// add corresponding colormap
					switch (bmBitsPixel)
					{
						case 1:
							pic.append(bgr_palette[0], colorPaletteLen >> 1);
							pic.append(bgr_palette[255], colorPaletteLen >> 1);
							break;

						case 4:
							for (int i = 0; i < 16; i++) pic.append(bgr_palette[c16idx[i]], 4);
							break;

						case 8:
							pic.append(bgr_palette[0], colorPaletteLen);
							break;
					}

					ole_offset += BM16_Bits;
					objectType = 2;   // we can go embedded now
				}
			}

			// embedded
			if (objectType == 2)
				pic.append(mData.getPointer(from - 0x80 + cbHeader + OLE_Object + ole_offset), objLen);

			break;
	}

	// let's see...
	if (pic.getLength())
	{
		// ...whether it's a picture
		FG_ConstGraphicPtr graphic;

		if (IE_ImpGraphic::loadGraphic(pic, iegft, graphic) != UT_OK || !graphic) {
			msg = "Picture load error or no picture";
		}
		else
		{
			int dxaOffset, dxaSize, dyaSize, mx, my;
			std::string properties, id;
			UT_LocaleTransactor lt(LC_NUMERIC, "C");

			dxaOffset = wri_struct_value(write_pic, "dxaOffset");

			if (dxaOffset & 0x8000) dxaOffset = -0x10000 + dxaOffset;

			if (dxaOffset)
			{
				properties = UT_std_string_sprintf("margin-left:%.4fin",
				                              static_cast<float>(dxaOffset) / 1440.0);
				const PP_PropertyVector attributes = {
					PT_PROPS_ATTRIBUTE_NAME, properties
				};
				appendStrux(PTX_Block, attributes);
			}

			dxaSize = wri_struct_value(write_pic, "dxaSize");
			dyaSize = wri_struct_value(write_pic, "dyaSize");

			if (dxaSize == 0) dxaSize = wri_struct_value(write_pic, "xExt");
			if (dyaSize == 0) dyaSize = wri_struct_value(write_pic, "yExt");

			if (dxaSize & 0x8000) dxaSize = -0x10000 + dxaSize;
			if (dyaSize & 0x8000) dyaSize = -0x10000 + dyaSize;

			mx = wri_struct_value(write_pic, "mx");
			my = wri_struct_value(write_pic, "my");

			// 0xe3 picture sizes aren't reliable, so we recalculate them
			// (1 pixel = 15 twips)
			if (mm == 0xe3)
			{
				dxaSize = bmW * 15;
				dyaSize = (bmH < 0 ? -bmH : bmH) * 15;
			}

			properties = UT_std_string_sprintf("width:%.4fin; height:%.4fin",
			                              static_cast<float>(mx) / 1000.0 *
			                              static_cast<float>(dxaSize) / 1440.0,
			                              static_cast<float>(my) / 1000.0 *
			                              static_cast<float>(dyaSize) / 1440.0);

			id = UT_std_string_sprintf("image%u", ++pic_nr);
			UT_DEBUGMSG(("    Image #%02d\n", pic_nr));

			const PP_PropertyVector attributes = {
				PT_PROPS_ATTRIBUTE_NAME, properties,
				"dataid", id
			};
			appendObject(PTO_Image, attributes);

			getDoc()->createDataItem(id.c_str(), false, graphic->getBuffer(), graphic->getMimeType(), NULL);
		}
	}


	if (write_pic) free_wri_struct(write_pic);

	if (msg) UT_WARNINGMSG(("read_pic: %s!\n", msg));

	return (msg ? false : true);
}

/**********************************************************************
 * MSWrite Importer (header and footer opening)                       *
 **********************************************************************/

void IE_Imp_MSWrite::_append_hdrftr (hdrftr_t which)
{
	PP_PropertyVector attributes = {
		PT_ID_ATTRIBUTE_NAME, "",
		PT_TYPE_ATTRIBUTE_NAME, ""
	};

	switch (which)
	{
		case headerfirst:
			attributes[1] = "0";
			attributes[3] = "header-first";
			break;

		case header:
			attributes[1] = "1";
			attributes[3] = "header";
			break;

		case footerfirst:
			attributes[1] = "2";
			attributes[3] = "footer-first";
			break;

		case footer:
			attributes[1] = "3";
			attributes[3] = "footer";
			break;
	}

	appendStrux(PTX_Section, attributes);
}
