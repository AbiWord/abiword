/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ut_string.h"
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_dialogHelper.h"
#include "xap_UnixFont.h"
#include "xap_UnixFontXLFD.h"


#define ASSERT_MEMBERS	do { UT_ASSERT(m_name); UT_ASSERT(m_fontfile); UT_ASSERT(m_metricfile); } while (0)

/*******************************************************************/

XAP_UnixFontHandle::XAP_UnixFontHandle()
{
	m_font = NULL;
	m_size = 0;
}

XAP_UnixFontHandle::XAP_UnixFontHandle(XAP_UnixFont * font, UT_uint32 size)
{
	m_font = font;
	m_size = size;
}

XAP_UnixFontHandle::XAP_UnixFontHandle(XAP_UnixFontHandle & copy)
	: GR_Font(copy)
{
	m_font = copy.m_font;
	m_size = copy.m_size;
}

XAP_UnixFontHandle::~XAP_UnixFontHandle()
{
}

GdkFont * XAP_UnixFontHandle::getGdkFont(void)
{
	if (m_font)
		return m_font->getGdkFont(m_size);
	else
		return NULL;
}

UT_uint32 XAP_UnixFontHandle::getSize(void)
{
	return m_size;
}


/*******************************************************************/		

XAP_UnixFont::XAP_UnixFont(void)
{
	m_name = NULL;
	m_style = STYLE_LAST;
	m_xlfd = NULL;
	
	m_fontfile = NULL;
	m_metricfile = NULL;

	m_metricsData = NULL;
	m_uniWidths = NULL;
	
	m_PFFile = NULL;
	
	m_fontKey = NULL;
}

XAP_UnixFont::XAP_UnixFont(XAP_UnixFont & copy)
{
	m_name = NULL;
	m_style = STYLE_LAST;
	m_xlfd = NULL;

	m_fontfile = NULL;
	m_metricfile = NULL;
	
	m_metricsData = NULL;

	m_PFFile = NULL;

	m_fontKey = NULL;

	openFileAs(copy.getFontfile(),
			   copy.getMetricfile(),
			   copy.getXLFD(),
			   copy.getStyle());
}

XAP_UnixFont::~XAP_UnixFont(void)
{
	FREEP(m_name);
	
	FREEP(m_fontfile);
	FREEP(m_metricfile);

	DELETEP(m_PFFile);
	
	FREEP(m_fontKey);

	UT_VECTOR_PURGEALL(allocFont *, m_allocFonts);
	
	// leave GdkFont * alone
}

UT_Bool XAP_UnixFont::openFileAs(const char * fontfile,
								const char * metricfile,
								const char * xlfd,
								XAP_UnixFont::style s)
{
	// test all our data to make sure we can continue
	if (!fontfile)
		return UT_FALSE;
	if (!metricfile)
		return UT_FALSE;
	if (!xlfd)
		return UT_FALSE;

	struct stat buf;
	int err;
	
	err = stat(fontfile, &buf);
	UT_ASSERT(err == 0 || err == -1);

	if (! (err == 0 || S_ISREG(buf.st_mode)) )
	{
		return UT_FALSE;
	}
	
	err = stat(metricfile, &buf);
	UT_ASSERT(err == 0 || err == -1);

	if (! (err == 0 || S_ISREG(buf.st_mode)) )
	{
		return UT_FALSE;
	}

	// strip our proper face name out of the XLFD
	char * newxlfd;
	UT_cloneString(newxlfd, xlfd);

	// run past the first field (foundry)
	strtok(newxlfd, "-");
	// save the second to a member
	FREEP(m_name);
	UT_cloneString(m_name, strtok(NULL, "-"));
	
	free(newxlfd);
	
	// save to memebers
	FREEP(m_fontfile);
	UT_cloneString(m_fontfile, fontfile);
	FREEP(m_metricfile);
	UT_cloneString(m_metricfile, metricfile);
	m_style = s;
	FREEP(m_xlfd);
	UT_cloneString(m_xlfd, xlfd);

	// update our key so we can be identified
	_makeFontKey();

	return UT_TRUE;
}

void XAP_UnixFont::setName(const char * name)
{
	FREEP(m_name);
	UT_cloneString(m_name, name);
}

const char * XAP_UnixFont::getName(void)
{
	ASSERT_MEMBERS;
	return m_name;
}

void XAP_UnixFont::setStyle(XAP_UnixFont::style s)
{
	m_style = s;
}

XAP_UnixFont::style XAP_UnixFont::getStyle(void)
{
	ASSERT_MEMBERS;
	return m_style;
}

const char * XAP_UnixFont::getFontfile(void)
{
	ASSERT_MEMBERS;
	
	return m_fontfile;
}

const char * XAP_UnixFont::getMetricfile(void)
{
	ASSERT_MEMBERS;
	return m_metricfile;
}

void XAP_UnixFont::setXLFD(const char * xlfd)
{
	FREEP(m_xlfd);
	UT_cloneString(m_xlfd, xlfd);
}

const char * XAP_UnixFont::getXLFD(void)
{
	ASSERT_MEMBERS;
	return m_xlfd;
}

UT_uint16 * XAP_UnixFont::getUniWidths(void)
{
	if (!m_uniWidths)
		getMetricsData();
	return m_uniWidths;
}

/* These are just the glyphs in the standard encoding. I need to add
   the rest of the glyphs, accessible through nonstandard
   encodings. */

/* This table is adapted very straightforwardly from
   ftp://ftp.unicode.org/Public/MAPPINGS/VENDORS/ADOBE/stdenc.txt.

   I have reviewed them and compared them against an independently
   constructed encoding, and believe they are correct.
  */

struct FontMappingTable {
	UT_sint32 unicode;
	char *type1_name;
};

const FontMappingTable std_enc[] = {
  { 0x0020, "space" },
  { 0x0021, "exclam" },
  { 0x0022, "quotedbl" },
  { 0x0023, "numbersign" },
  { 0x0024, "dollar" },
  { 0x0025, "percent" },
  { 0x0026, "ampersand" },
  { 0x0027, "quotesingle" },
  { 0x0028, "parenleft" },
  { 0x0029, "parenright" },
  { 0x002A, "asterisk" },
  { 0x002B, "plus" },
  { 0x002C, "comma" },
  { 0x002D, "hyphen" },
  { 0x002E, "period" },
  { 0x002F, "slash" },
  { 0x0030, "zero" },
  { 0x0031, "one" },
  { 0x0032, "two" },
  { 0x0033, "three" },
  { 0x0034, "four" },
  { 0x0035, "five" },
  { 0x0036, "six" },
  { 0x0037, "seven" },
  { 0x0038, "eight" },
  { 0x0039, "nine" },
  { 0x003A, "colon" },
  { 0x003B, "semicolon" },
  { 0x003C, "less" },
  { 0x003D, "equal" },
  { 0x003E, "greater" },
  { 0x003F, "question" },
  { 0x0040, "at" },
  { 0x0041, "A" },
  { 0x0042, "B" },
  { 0x0043, "C" },
  { 0x0044, "D" },
  { 0x0045, "E" },
  { 0x0046, "F" },
  { 0x0047, "G" },
  { 0x0048, "H" },
  { 0x0049, "I" },
  { 0x004A, "J" },
  { 0x004B, "K" },
  { 0x004C, "L" },
  { 0x004D, "M" },
  { 0x004E, "N" },
  { 0x004F, "O" },
  { 0x0050, "P" },
  { 0x0051, "Q" },
  { 0x0052, "R" },
  { 0x0053, "S" },
  { 0x0054, "T" },
  { 0x0055, "U" },
  { 0x0056, "V" },
  { 0x0057, "W" },
  { 0x0058, "X" },
  { 0x0059, "Y" },
  { 0x005A, "Z" },
  { 0x005B, "bracketleft" },
  { 0x005C, "backslash" },
  { 0x005D, "bracketright" },
  { 0x005E, "asciicircum" },
  { 0x005F, "underscore" },
  { 0x0060, "grave" },
  { 0x0061, "a" },
  { 0x0062, "b" },
  { 0x0063, "c" },
  { 0x0064, "d" },
  { 0x0065, "e" },
  { 0x0066, "f" },
  { 0x0067, "g" },
  { 0x0068, "h" },
  { 0x0069, "i" },
  { 0x006A, "j" },
  { 0x006B, "k" },
  { 0x006C, "l" },
  { 0x006D, "m" },
  { 0x006E, "n" },
  { 0x006F, "o" },
  { 0x0070, "p" },
  { 0x0071, "q" },
  { 0x0072, "r" },
  { 0x0073, "s" },
  { 0x0074, "t" },
  { 0x0075, "u" },
  { 0x0076, "v" },
  { 0x0077, "w" },
  { 0x0078, "x" },
  { 0x0079, "y" },
  { 0x007A, "z" },
  { 0x007B, "braceleft" },
  { 0x007C, "bar" },
  { 0x007D, "braceright" },
  { 0x007E, "asciitilde" },
  { 0x00A1, "exclamdown" },
  { 0x00A2, "cent" },
  { 0x00A3, "sterling" },
  { 0x00A4, "currency" },
  { 0x00A5, "yen" },
  { 0x00A7, "section" },
  { 0x00A8, "dieresis" },
  { 0x00AA, "ordfeminine" },
  { 0x00AB, "guillemotleft" },
  { 0x00AF, "macron" },
  { 0x00B4, "acute" },
  { 0x00B6, "paragraph" },
  { 0x00B7, "periodcentered" },
  { 0x00B8, "cedilla" },
  { 0x00BA, "ordmasculine" },
  { 0x00BB, "guillemotright" },
  { 0x00BF, "questiondown" },
  { 0x00C6, "AE" },
  { 0x00D8, "Oslash" },
  { 0x00DF, "germandbls" },
  { 0x00E6, "ae" },
  { 0x00F8, "oslash" },
  /*{ 0x0131, "dotlessi" },
  { 0x0141, "Lslash" },
  { 0x0142, "lslash" },
  { 0x0152, "OE" },
  { 0x0153, "oe" },
  { 0x0192, "florin" },
  { 0x02C6, "circumflex" },
  { 0x02C7, "caron" },
  { 0x02D8, "breve" },
  { 0x02D9, "dotaccent" },
  { 0x02DA, "ring" },
  { 0x02DB, "ogonek" },
  { 0x02DC, "tilde" },
  { 0x02DD, "hungarumlaut" },
  { 0x2013, "endash" },
  { 0x2014, "emdash" },
  { 0x2018, "quoteleft" },
  { 0x2019, "quoteright" },
  { 0x201A, "quotesinglbase" },
  { 0x201C, "quotedblleft" },
  { 0x201D, "quotedblright" },
  { 0x201E, "quotedblbase" },
  { 0x2020, "dagger" },
  { 0x2021, "daggerdbl" },
  { 0x2022, "bullet" },
  { 0x2026, "ellipsis" },
  { 0x2030, "perthousand" },
  { 0x2039, "guilsinglleft" },
  { 0x203A, "guilsinglright" },
  { 0x2044, "fraction" },*/
//  { 0xFB01, "fi" },
//  { 0xFB02, "fl" },

  // All of the ones you see above were found in gnome-print. The following were added by Aaron Lehmann.
  // These will be contributed to gnome-print if they end up working.
  // My references are ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859/8859-1.TXT and a random AFM file.

  { 0x00DD, "Yacute" },
  { 0x00DB, "Ucircumflex" },
  { 0x00D9, "Ugrave" },
  // HELP: What is "Zcaron"?
  { 0x00FF, "Ydieresis" },
  { 0x00B3, "threesuperior" },
  { 0x00DA, "Uacute" },
  { 0x00B2, "twosuperior" },
  { 0x00DC, "Udieresis" },
  { 0x00B7, "middot" },
  { 0x00B9, "onesuperior" },
  { 0x00E1, "aacute" },
  { 0x00E0, "agrave" },
  { 0x00E2, "acircumflex" },
  { 0x00DF, "Scaron" },
  { 0x00D5, "Otilde" },
  { 0x00AD, "sfthyphen" },
  { 0x00E3, "atilde" },
  { 0x00E5, "aring" },
  { 0x00E4, "adieresis" },
  { 0x00D2, "Ograve" },
  { 0x00D4, "Ocurcumflex" },
  { 0x00D6, "Odieresis" },
  { 0x00D1, "Ntilde" },
  { 0x00EB, "edieresis" },
  { 0x00E9, "eacute" },
  { 0x00E8, "egrave" },
  { 0x00CE, "Icircumflex" },
  { 0x00EA, "ecircumflex" },
  { 0x00CC, "Igrave" },
  { 0x00CD, "Iacute" },
  { 0x00CF, "Idiersis" },
  { 0x00B0, "degree" },
  { 0x00CA, "Ecircumflex" },
  { 0x002D, "minus" },
  { 0x00D7, "multiply" },
  { 0x00F7, "divide" },
  { 0x00C8, "Egrave" },
  // HELP: What is "trademark"?
  { 0x00D3, "Oacute" },
  { 0x00FE, "thorn" },
  { 0x00F0, "eth" },
  { 0x00C9, "Eacute" },
  { 0x00E7, "ccedilla" },
  { 0x00EF, "idieresis" },
  { 0x00ED, "iacute" },
  { 0x00EC, "igrave" },
  { 0x00B1, "plusminus" },
  { 0x00BD, "onehalf" },
  { 0x00BC, "onequarter" },
  { 0x00BE, "threequarters" },
  { 0x00EE, "icircumflex" },
  { 0x00CB, "Edieresis" },
  { 0x00F1, "ntilde" },
  { 0x00C5, "Aring" },
  { 0x00F6, "odieresis" },
  { 0x00F3, "oacute" },
  { 0x00F2, "ograve" },
  { 0x00F4, "ocircumflex" },
  { 0x00F5, "otilde" },
  // HELP: What is "scaron"?
  { 0x00FC, "udieresis" },
  { 0x00FA, "uacute" },
  { 0x00F9, "ugrave" },
  { 0x00FB, "ucircumflex" },
  { 0x00FD, "yacute" },
  // HELP: What is "zcaron"?
  { 0x00FF, "ydieresis" },
  { 0x00A9, "copyright" },
  { 0x00AE, "registered" },
  { 0x00C3, "Atilde" },
  { 0x00A0, "nbspace" },
  { 0x00C7, "Ccedilla" },
  { 0x00C2, "Acircumflex" },
  { 0x00C0, "Agrave" },
  { 0x00AC, "logicalnot" },
  { 0x00C1, "Aacute" },
  { 0x00D0, "Eth" },
  { 0x00A6, "brokenbar" },
  { 0x00DE, "Thorn" },
  { 0x00C4, "Adieresis" },
  // HELP: What is "mu"?
  { 0x0000, NULL }
};

const FontMappingTable sym_enc[] = {
	{ 32, "space" },
	{ 33, "exclam" },
	{ 34, "universal" },
	{ 35, "numbersign" },
	{ 36, "existential" },
	{ 37, "percent" },
	{ 38, "ampersand" },
	{ 39, "suchthat" },
	{ 40, "parenleft" },
	{ 41, "parenright" },
	{ 42, "asteriskmath" },
	{ 43, "plus" },
	{ 44, "comma" },
	{ 45, "minus" },
	{ 46, "period" },
	{ 47, "slash" },
	{ 48, "zero" },
	{ 49, "one" },
	{ 50, "two" },
	{ 51, "three" },
	{ 52, "four" },
	{ 53, "five" },
	{ 54, "six" },
	{ 55, "seven" },
	{ 56, "eight" },
	{ 57, "nine" },
	{ 58, "colon" },
	{ 59, "semicolon" },
	{ 60, "less" },
	{ 61, "equal" },
	{ 62, "greater" },
	{ 63, "question" },
	{ 64, "congruent" },
	{ 65, "Alpha" },
	{ 66, "Beta" },
	{ 67, "Chi" },
	{ 68, "Delta" },
	{ 69, "Epsilon" },
	{ 70, "Phi" },
	{ 71, "Gamma" },
	{ 72, "Eta" },
	{ 73, "Iota" },
	{ 74, "theta1" },
	{ 75, "Kappa" },
	{ 76, "Lambda" },
	{ 77, "Mu" },
	{ 78, "Nu" },
	{ 79, "Omicron" },
	{ 80, "Pi" },
	{ 81, "Theta" },
	{ 82, "Rho" },
	{ 83, "Sigma" },
	{ 84, "Tau" },
	{ 85, "Upsilon" },
	{ 86, "sigma1" },
	{ 87, "Omega" },
	{ 88, "Xi" },
	{ 89, "Psi" },
	{ 90, "Zeta" },
	{ 91, "bracketleft" },
	{ 92, "therefore" },
	{ 93, "bracketright" },
	{ 94, "perpendicular" },
	{ 95, "underscore" },
	{ 96, "radicalex" },
	{ 97, "alpha" },
	{ 98, "beta" },
	{ 99, "chi" },
	{ 100, "delta" },
	{ 101, "epsilon" },
	{ 102, "phi" },
	{ 103, "gamma" },
	{ 104, "eta" },
	{ 105, "iota" },
	{ 106, "phi1" },
	{ 107, "kappa" },
	{ 108, "lambda" },
	{ 109, "mu" },
	{ 110, "nu" },
	{ 111, "omicron" },
	{ 112, "pi" },
	{ 113, "theta" },
	{ 114, "rho" },
	{ 115, "sigma" },
	{ 116, "tau" },
	{ 117, "upsilon" },
	{ 118, "omega1" },
	{ 119, "omega" },
	{ 120, "xi" },
	{ 121, "psi" },
	{ 122, "zeta" },
	{ 123, "braceleft" },
	{ 124, "bar" },
	{ 125, "braceright" },
	{ 126, "similar" },
	{ 161, "Upsilon1" },
	{ 162, "minute" },
	{ 163, "lessequal" },
	{ 164, "fraction" },
	{ 165, "infinity" },
	{ 166, "florin" },
	{ 167, "club" },
	{ 168, "diamond" },
	{ 169, "heart" },
	{ 170, "spade" },
	{ 171, "arrowboth" },
	{ 172, "arrowleft" },
	{ 173, "arrowup" },
	{ 174, "arrowright" },
	{ 175, "arrowdown" },
	{ 176, "degree" },
	{ 177, "plusminus" },
	{ 178, "second" },
	{ 179, "greaterequal" },
	{ 180, "multiply" },
	{ 181, "proportional" },
	{ 182, "partialdiff" },
	{ 183, "bullet" },
	{ 184, "divide" },
	{ 185, "notequal" },
	{ 186, "equivalence" },
	{ 187, "approxequal" },
	{ 188, "ellipsis" },
	{ 189, "arrowvertex" },
	{ 190, "arrowhorizex" },
	{ 191, "carriagereturn" },
	{ 192, "aleph" },
	{ 193, "Ifraktur" },
	{ 194, "Rfraktur" },
	{ 195, "weierstrass" },
	{ 196, "circlemultiply" },
	{ 197, "circleplus" },
	{ 198, "emptyset" },
	{ 199, "intersection" },
	{ 200, "union" },
	{ 201, "propersuperset" },
	{ 202, "reflexsuperset" },
	{ 203, "notsubset" },
	{ 204, "propersubset" },
	{ 205, "reflexsubset" },
	{ 206, "element" },
	{ 207, "notelement" },
	{ 208, "angle" },
	{ 209, "gradient" },
	{ 210, "registeredserif" },
	{ 211, "copyrightserif" },
	{ 212, "trademarkserif" },
	{ 213, "product" },
	{ 214, "radical" },
	{ 215, "dotmath" },
	{ 216, "logicalnot" },
	{ 217, "logicaland" },
	{ 218, "logicalor" },
	{ 219, "arrowdblboth" },
	{ 220, "arrowdblleft" },
	{ 221, "arrowdblup" },
	{ 222, "arrowdblright" },
	{ 223, "arrowdbldown" },
	{ 224, "lozenge" },
	{ 225, "angleleft" },
	{ 226, "registeredsans" },
	{ 227, "copyrightsans" },
	{ 228, "trademarksans" },
	{ 229, "summation" },
	{ 230, "parenlefttp" },
	{ 231, "parenleftex" },
	{ 232, "parenleftbt" },
	{ 233, "bracketlefttp" },
	{ 234, "bracketleftex" },
	{ 235, "bracketleftbt" },
	{ 236, "bracelefttp" },
	{ 237, "braceleftmid" },
	{ 238, "bracelefttbt" },
	{ 239, "braceex" },
	{ 241, "angleright" },
	{ 242, "integral" },
	{ 243, "integraltp" },
	{ 244, "integralex" },
	{ 245, "integralbt" },
	{ 246, "parenrighttp" },
	{ 247, "parenrightex" },
	{ 248, "parenrightbt" },
	{ 249, "bracketrighttp" },
	{ 250, "bracketrightex" },
	{ 251, "bracketrightbt" },
	{ 252, "bracerighttp" },
	{ 253, "bracerightmid" },
	{ 254, "bracerightbt" },
  { 0x0000, NULL }
};

ABIFontInfo * XAP_UnixFont::getMetricsData(void)
{
	if (m_metricsData)
		return m_metricsData;

	UT_ASSERT(m_metricfile);
	
	// open up the metrics file, which should have been proven to
	// exist earlier in the construction of this object.
	FILE * fp = fopen(m_metricfile, "r");

	char message[1024];

	if (fp == NULL)
	{
		g_snprintf(message, 1024,
				   "The font metrics file [%s] could\n"
				   "not be opened for parsing.  Please ensure that this file\n"
				   "is present before printing.  Right now, this is a pretty\n"
				   "darn fatal error.",
				   m_metricfile);
		messageBoxOK(message);
		return NULL;
	}

	// call down to the Adobe code
	int result = abi_parseFile(fp, &m_metricsData, P_GM);
	switch (result)
	{
	case parseError:
		g_snprintf(message, 1024,
				   "AbiWord encountered errors parsing the font metrics file\n"
				   "[%s].\n"
				   "These errors were not fatal, but print metrics may be incorrect.",
				   m_metricfile);
		messageBoxOK(message);
		break;
	case earlyEOF:
		g_snprintf(message, 1024,
				   "AbiWord encountered a premature End of File (EOF) while parsing\n"
				   "the font metrics file [%s].\n"
				   "Printing cannot continue.",
				   m_metricfile);
		messageBoxOK(message);
		m_metricsData = NULL;
		break;
	case storageProblem:
		// if we got here, either the metrics file is broken (like it's
		// saying it has 209384098278942398743982 kerning lines coming, and
		// we know we can't allocate that), or we really did run out of memory.
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		m_metricsData = NULL;
		break;
	default:
		// everything is peachy
		break;
	}

	// Need to mangle the cmi[i].wx variables to work right with Unicode
	const FontMappingTable *the_enc = std_enc;
	xxx_UT_DEBUGMSG(("PS font file:          %s %d\n", m_fontfile, this));
	xxx_UT_DEBUGMSG(("PS font metrics file:  %s\n", m_metricfile));
	xxx_UT_DEBUGMSG(("PS font encoding type: %s\n", m_metricsData->gfi->encodingScheme));
	if (strcmp("FontSpecific", m_metricsData->gfi->encodingScheme) == 0)
	{
		// TODO: This is slightly a crock since we imagine the only font
		// TODO: we'll see with FontSpecific encoding is the standard symbols
		// TODO: font.  Sez who?  What we probably ought to do is dynamically
		// TODO: allocate the encoding table for the font based on the
		// TODO: FontSpecific encoding.  Or something.
		the_enc = sym_enc;
	}
	m_uniWidths = (UT_uint16 *) malloc (sizeof (UT_uint16) * 256);
	memset (m_uniWidths, 0, 256 * sizeof(UT_uint16)); // Clear array - i would hope that sizeof(UT_uint16) == 16
	for (UT_sint32 i=0; i != m_metricsData->numOfChars; ++i)
	{
		UT_sint32 unicode = -1;
		for (UT_uint32 j = 0; the_enc[j].type1_name != NULL; j++)
		{
			if (!strcmp (m_metricsData->cmi[i].name, the_enc[j].type1_name))
			{
				unicode = the_enc[j].unicode;
				break;
			}
		}
		if (unicode >= 0)
		{
			UT_ASSERT (unicode < 256); // TODO: support multibyte chars
			m_uniWidths[unicode] = m_metricsData->cmi[i].wx;
		}
	}

	fclose(fp);
	
	UT_ASSERT(m_metricsData);
	UT_ASSERT(m_metricsData->gfi);
	return m_metricsData;
}

UT_Bool XAP_UnixFont::openPFA(void)
{
	ASSERT_MEMBERS;
	int peeked;
	
	m_PFFile = fopen(m_fontfile, "r");

	if (!m_PFFile)
	{
		char message[1024];
		g_snprintf(message, 1024,
				   "Font data file [%s] can not be opened for reading.\n", m_fontfile);
		messageBoxOK(message);
		return UT_FALSE;
	}

	/* Check to see if it's a binary or ascii PF file */
	peeked = fgetc(m_PFFile);
	ungetc(peeked, m_PFFile);
	m_PFB = peeked == PFB_MARKER;
	m_bufpos = 0;
	return UT_TRUE;
}

UT_Bool XAP_UnixFont::closePFA(void)
{
	if (m_PFFile)
	{
		fclose(m_PFFile);
		return UT_TRUE;
	}
	return UT_FALSE;
}

char XAP_UnixFont::getPFAChar(void)
{
	UT_ASSERT(m_PFFile);
	return m_PFB ? _getPFBChar() : fgetc(m_PFFile);
}

char XAP_UnixFont::_getPFBChar(void)
{
	char message[1024];
	UT_Byte inbuf[BUFSIZ], hexbuf[2 * BUFSIZ], *inp, *hexp;
	int  type, in, got, length;
	static char *hex = "0123456789abcdef";

	if (m_buffer.getLength() > m_bufpos )
	{
		return m_buffer.getPointer(m_bufpos++)[0];
	}

	// Read a new block into the buffer.
	m_buffer.truncate(0);
	in = fgetc(m_PFFile);
	type = fgetc(m_PFFile);

	if (in != PFB_MARKER
	    || (type != PFB_ASCII && type != PFB_BINARY && type != PFB_DONE))
	{
		g_snprintf(message, 1024,
				   "AbiWord encountered errors parsing the font data file\n"
				   "[%s].\n"
				   "These errors were not fatal, but printing may be incorrect.",
				   m_fontfile);
		messageBoxOK(message);
		return EOF;
	}

	if (type == PFB_DONE)
	{
		/*
		 * If there's data after this, the file is corrupt and we want
		 * to ignore it.
		 */
		ungetc(PFB_DONE, m_PFFile);
		return EOF;
	}

	length = fgetc(m_PFFile) & 0xFF;
	length |= (fgetc(m_PFFile) & 0XFF) << 8;
	length |= (fgetc(m_PFFile) & 0XFF) << 16;
	length |= (fgetc(m_PFFile) & 0XFF) << 24;
	if (feof(m_PFFile))
	{
		g_snprintf(message, 1024,
				   "AbiWord encountered errors parsing the font data file\n"
				   "[%s].\n"
				   "These errors were not fatal, but printing may be incorrect.",
				   m_fontfile);
		messageBoxOK(message);
		return EOF;
	}

	while (length > 0)
	{
		in = (length > BUFSIZ ? BUFSIZ : length);
		got = fread(inbuf, 1, in, m_PFFile);
		if (in != got)
		{
		g_snprintf(message, 1024,
				   "AbiWord encountered errors parsing the font data file\n"
				   "[%s].\n"
				   "These errors were not fatal, but printing may be incorrect.",
				   m_fontfile);
			messageBoxOK(message);
			length = got;
		}
		if (type == PFB_ASCII)
			m_buffer.append(inbuf, got);
		else // type == PFB_BINARY
		{
			hexp = hexbuf;
			for (inp = inbuf; inp - inbuf < got; inp += 1)
			{
				  *hexp++ = hex[(*inp >> 4) & 0xf];
				  *hexp++ = hex[*inp & 0xf];
			}
			m_buffer.append(hexbuf, 2 * got);
		}
		length -= got ;
	}

	m_bufpos = 1;
	return m_buffer.getPointer(0)[0];
}

const char * XAP_UnixFont::getFontKey(void)
{
	ASSERT_MEMBERS;
	return m_fontKey;
}

GdkFont * XAP_UnixFont::getGdkFont(UT_uint32 pixelsize)
{
	// this might return NULL, but that means a font at a certain
	// size couldn't be found
	UT_uint32 l = 0;
	UT_uint32 count = m_allocFonts.getItemCount();
	allocFont * entry;

	while (l < count)
	{
		entry = (allocFont *) m_allocFonts.getNthItem(l);
		if (entry && entry->pixelSize == pixelsize)
			return entry->gdkFont;
		else
			l++;
	}

	GdkFont * gdkfont = NULL;
	
	// If the font is really, really small (an EXTREMELY low Zoom can trigger this) some
	// fonts will be calculated to 0 height.  Bump it up to 2 since the user obviously
	// doesn't care about readability anyway.  :)
	if (pixelsize < 2)
		pixelsize = 2;

	// create a real object around that string
	XAP_UnixFontXLFD myXLFD(m_xlfd);

	// Must set a pixel size, or a point size, but we're getting
	// automunged pixel sizes appropriate for our resolution from
	// the layout engine, so use pixel sizes.  They're not really
	// much more accurate this way, but they're more consistent
	// with how the layout engine wants fonts.
	myXLFD.setPixelSize(pixelsize);

	// TODO  add any other special requests, like for a specific encoding
	// TODO  or registry, or resolution here

	char * newxlfd = myXLFD.getXLFD();

	gdkfont = gdk_font_load(newxlfd);

	if (!gdkfont)
	{
		free(newxlfd);
		newxlfd = myXLFD.getFallbackXLFD();
	    gdkfont = gdk_font_load(newxlfd);
	}

	if (!gdkfont)
	{
		char message[1024];
		g_snprintf(message, 1024,
				   "AbiWord could not load the following font from the X Window System display server:\n"
				   "[%s]\n"
				   "\n"
				   "This error could be the result of an incomplete AbiSuite installation,\n"
				   "an incompatibility with your X Window System display server,\n"
				   "or a problem communicating with a remote font server.\n"
				   "\n"
				   "Often this error is the result of invoking AbiWord directly instead of through\n"
				   "its wrapper shell script.  The script dynamically adds the AbiSuite font directory\n"
				   "to your X Window System display server font path before running the executable.\n"
				   "\n"
				   "Please visit http://www.abisource.com/ for more information.",
				   newxlfd);
		messageBoxOK(message);
		exit(1);
	}

	free(newxlfd);
	
	allocFont * item = new allocFont;
	item->pixelSize = pixelsize;
	item->gdkFont = gdkfont;
	m_allocFonts.addItem((void *) item);

	return gdkfont;
}

void XAP_UnixFont::_makeFontKey(void)
{
	ASSERT_MEMBERS;

	// if we already have a key, free it
	FREEP(m_fontKey);
	
	// allocate enough to combine name, seperator, style, and NULL into key.
	// this won't work if we have styles that require two digits in decimal.
	char * key = (char *) calloc(strlen(m_name) + 1 + 1 + 1, sizeof(char));
	UT_ASSERT(key);

	char * copy;
	UT_cloneString(copy, m_name);
	UT_upperString(copy);
	
	sprintf(key, "%s@%d", copy, m_style);

	FREEP(copy);
	
	// point member our way
	m_fontKey = key;
}
