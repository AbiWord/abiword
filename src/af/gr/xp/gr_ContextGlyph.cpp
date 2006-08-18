/* AbiSource Program Utilities
 * Copyright (C) 2001, 2003 Tomas Frydrych
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


#include <stdlib.h>
#include <string.h>

#include "gr_ContextGlyph.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Language.h"
#include "ut_growbuf.h"
#include "ut_xml.h"


#include "xap_App.h"
#include "xap_Prefs.h"

struct LigatureSequence
{
	UT_UCSChar code;
	UT_UCSChar next;
};

#ifndef NO_BIDI_SUPPORT

// The following table contains ligature data for two-character
// ligatures (code_low is the first character, code_high the second).
// 
// this table has to be sorted by the first two numbers !!!
// use 1 to indicate that no ligature glyph exists
//
// if the ligature is not context-sensitive, set the stand-alone form
// to 0 and the initial form to the ligature value

static LigatureData s_ligature[] =
{
	// code_low, code_high, intial, medial, final, stand-alone

	// Latin
#define LATIN_LIGATURES_FIRST 0x0021
	{0x0021, 0x0021, 0x203c, 0, 0, 0}, // !!
	{0x0021, 0x003f, 0x2049, 0, 0, 0}, // !?
	{0x003f, 0x0021, 0x2048, 0, 0, 0}, // ?!
	{0x003f, 0x003f, 0x2047, 0, 0, 0}, // ??
	{0x0066, 0x0066, 0xFB00, 0xFB00, 0xFB00, 0}, // ff
	{0x0066, 0x0069, 0xFB01, 0xFB01, 0xFB01, 0}, // fi
	{0x0066, 0x006C, 0xFB02, 0xFB02, 0xFB02, 0}, // fl
	{0x017F, 0x0074, 0xFB05, 0xFB05, 0xFB05, 0},
#define LATIN_LIGATURES_LAST 0x017F
	
	// Greek
	{0x0391, 0x0301, 0x0386, 0x0386, 0x0386, 0},
	{0x0395, 0x0301, 0x0388, 0x0388, 0x0388, 0},
	{0x0397, 0x0301, 0x0389, 0x0389, 0x0389, 0},
	{0x0399, 0x0301, 0x038a, 0x038a, 0x038a, 0},
	{0x0399, 0x0308, 0x03aa, 0x03aa, 0x03aa, 0},
	{0x039F, 0x0301, 0x038c, 0x038c, 0x038c, 0},
	{0x03A5, 0x0301, 0x038e, 0x038e, 0x038e, 0},
	{0x03A5, 0x0308, 0x03ab, 0x03ab, 0x03ab, 0},
	{0x03A9, 0x0301, 0x038f, 0x038f, 0x038f, 0},
	{0x03B1, 0x0301, 0x03ac, 0x03ac, 0x03ac, 0},
	{0x03B5, 0x0301, 0x03ad, 0x03ad, 0x03ad, 0},
	{0x03B7, 0x0301, 0x03ae, 0x03ae, 0x03ae, 0},
	{0x03B8, 0x0308, 0x03ca, 0x03ca, 0x03ca, 0},
	{0x03B9, 0x0301, 0x03af, 0x03af, 0x03af, 0},
	{0x03BF, 0x0301, 0x03cc, 0x03cc, 0x03cc, 0},
	{0x03C5, 0x0301, 0x03cd, 0x03cd, 0x03cd, 0},
	{0x03C5, 0x0308, 0x03cb, 0x03cb, 0x03cb, 0},
	{0x03C9, 0x0301, 0x03ce, 0x03ce, 0x03ce, 0},
	{0x03CA, 0x0301, 0x0390, 0x0390, 0x0390, 0},
	{0x03CB, 0x0301, 0x03b0, 0x03b0, 0x03b0, 0},

	// Hebrew
	{0x05D0, 0x05B7, 0xFB2E, 0xFB2E, 0xFB2E, 0},
	{0x05D0, 0x05B8, 0xFB2F, 0xFB2F, 0xFB2F, 0},
	{0x05D0, 0x05BC, 0xFB30, 0xFB30, 0xFB30, 0},
//	{0x05D0, 0x05DC, 0xFB4F, 0xFB4F, 0xFB4F, 0}, // alef-lamed
	{0x05D1, 0x05BC, 0xFB31, 0xFB31, 0xFB31, 0},
	{0x05D1, 0x05BF, 0xFB4C, 0xFB4C, 0xFB4C, 0},
	{0x05D2, 0x05BC, 0xFB32, 0xFB32, 0xFB32, 0},
	{0x05D3, 0x05BC, 0xFB33, 0xFB33, 0xFB33, 0},
	{0x05D4, 0x05BC, 0xFB34, 0xFB34, 0xFB34, 0},
	{0x05D5, 0x05B9, 0xFB4B, 0xFB4B, 0xFB4B, 0},
	{0x05D5, 0x05BC, 0xFB35, 0xFB35, 0xFB35, 0},
	{0x05D6, 0x05BC, 0xFB36, 0xFB36, 0xFB36, 0},
	{0x05D8, 0x05BC, 0xFB38, 0xFB38, 0xFB38, 0},
	{0x05D9, 0x05B4, 0xFB1D, 0xFB1D, 0xFB1D, 0},
	{0x05D9, 0x05BC, 0xFB39, 0xFB39, 0xFB39, 0},
	{0x05DA, 0x05BC, 0xFB3A, 0xFB3A, 0xFB3A, 0},
	{0x05DB, 0x05BC, 0xFB3B, 0xFB3B, 0xFB3B, 0},
	{0x05DB, 0x05BF, 0xFB4D, 0xFB4D, 0xFB4D, 0},
	{0x05DC, 0x05BC, 0xFB3C, 0xFB3C, 0xFB3C, 0},
	{0x05DE, 0x05BC, 0xFB3E, 0xFB3E, 0xFB3E, 0},
	{0x05E0, 0x05BC, 0xFB40, 0xFB40, 0xFB40, 0},
	{0x05E1, 0x05BC, 0xFB41, 0xFB41, 0xFB41, 0},
	{0x05E3, 0x05BC, 0xFB43, 0xFB43, 0xFB43, 0},
	{0x05E4, 0x05BC, 0xFB44, 0xFB44, 0xFB44, 0},
	{0x05E4, 0x05BF, 0xFB4E, 0xFB4E, 0xFB4E, 0},
	{0x05E6, 0x05BC, 0xFB46, 0xFB46, 0xFB46, 0},
	{0x05E7, 0x05BC, 0xFB47, 0xFB47, 0xFB47, 0},
	{0x05E8, 0x05BC, 0xFB48, 0xFB48, 0xFB48, 0},
	{0x05E9, 0x05BC, 0xFB49, 0xFB49, 0xFB49, 0},
	{0x05E9, 0x05C1, 0xFB2A, 0xFB2A, 0xFB2A, 0},
	{0x05E9, 0x05C2, 0xFB2B, 0xFB2B, 0xFB2B, 0},
	{0x05EA, 0x05BC, 0xFB4A, 0xFB4A, 0xFB4A, 0},
	{0x05F2, 0x05B7, 0xFB1F, 0xFB1F, 0xFB1F, 0},
	
	// Arabic
	{0x0626, 0x0627, 1, 1, 0xFBEB, 0xFBEA},
	{0x0626, 0x062c, 0xFC97, 1, 1, 0xFC00},
	{0x0626, 0x062d, 0xFC98, 1, 1, 0xFC01},
	{0x0626, 0x062e, 0xFC99, 1, 1, 1},
	{0x0626, 0x0631, 1, 1, 0xFC64, 1},
	{0x0626, 0x0632, 1, 1, 0xFC65, 1},
	{0x0626, 0x0645, 0xFC9A, 0xFCDF, 0xFC66, 0xFC02},
	{0x0626, 0x0646, 1, 1, 0xFC67, 1},
 	{0x0626, 0x0647, 0xFC9b, 0xFCE0, 1, 1},
	{0x0626, 0x0648, 1, 1, 0xFBEF, 0xFBEE},
	{0x0626, 0x0649, 1, 1, 0xFC68, 0xFC03},
	{0x0626, 0x064a, 1, 1, 0xFC69, 0xFC04},
	{0x0626, 0x06c7, 1, 1, 0xFBF1, 0xFBF0},
	{0x0626, 0x06c6, 1, 1, 0xFBF3, 0xFBF2},
	{0x0626, 0x06c8, 1, 1, 0xFBF5, 0xFBF4},
	{0x0626, 0x06d0, 1, 1, 0xFBF7, 0xFBF6},
	{0x0626, 0x06d5, 1, 1, 0xFBED, 0xFBEC},

	{0x0627, 0x064B, 1, 1, 0xFD3C, 0xFD3D},
	
	{0x0628, 0x062c, 0xFC9c, 1, 1, 0xFC05},
	{0x0628, 0x062d, 0xFC9d, 1, 1, 0xFC06},
	{0x0628, 0x062e, 0xFC9e, 1, 1, 0xFC07},
	{0x0628, 0x0631, 1, 1, 0xFC6a, 1},
	{0x0628, 0x0632, 1, 1, 0xFC6b, 1},
	{0x0628, 0x0645, 0xFC9f, 0xFCE1, 0xFC6c, 0xFC08},
	{0x0628, 0x0646, 1, 1, 0xFC6d, 1},
	{0x0628, 0x0647, 0xFCa0, 0xFCE2, 1, 1},
	{0x0628, 0x0649, 1, 1, 0xFC6e, 0xFC09},
	{0x0628, 0x064a, 1, 1, 0xFC6f, 0xFC0A},

	{0x062a, 0x062c, 0xFCa1, 1, 1, 0xFC0B},
	{0x062a, 0x062d, 0xFCa2, 1, 1, 0xFC0C},
	{0x062a, 0x062e, 0xFCa3, 1, 1, 0xFC0D},
	{0x062a, 0x0631, 1, 1, 0xFC70, 1},
	{0x062a, 0x0632, 1, 1, 0xFC71, 1},
	{0x062a, 0x0645, 0xFCa4, 0xFCE3, 0xFC72, 0xFC0E},
	{0x062a, 0x0646, 1, 1, 0xFC73, 1},
	{0x062a, 0x0647, 0xFCa5, 0xFCE4, 1, 1},
	{0x062a, 0x0649, 1, 1, 0xFC74, 0xFC0F},
	{0x062a, 0x064a, 1, 1, 0xFC75, 0xFC10},

	{0x062b, 0x062c, 1, 1, 1, 0xFC11},
	{0x062b, 0x0631, 1, 1, 0xFC76, 1},
	{0x062b, 0x0632, 1, 1, 0xFC77, 1},
	{0x062b, 0x0645, 0xFCA6, 0xFCE5, 0xFC78, 0xFC12},
	{0x062b, 0x0646, 1, 1, 0xFC79, 1},
	{0x062b, 0x0647, 1, 0xFCE6, 1, 1},
	{0x062b, 0x0649, 1, 1, 0xFC7a, 0xFC13},
	{0x062b, 0x064a, 1, 1, 0xFC7b, 0xFC14},
	
	{0x062c, 0x062d, 0xFCA7, 1, 1, 0xFC15},
	{0x062c, 0x0645, 0xFCA8, 1, 1, 0xFC16},
	{0x062c, 0x0649, 1, 1, 0xFD1D, 0xFD01},
	{0x062c, 0x064a, 1, 1, 0xFD1E, 0xFD02},
	
	{0x062d, 0x062c, 0xFCA9, 1, 1, 0xFC17},
	{0x062d, 0x0645, 0xFCAA, 1, 1, 0xFC18},
	{0x062d, 0x0649, 1, 1, 0xFD1B, 0xFCFF},
	{0x062d, 0x064a, 1, 1, 0xFD1C, 0xFD00},

	{0x062e, 0x062c, 0xFCAB, 1, 1, 0xFC19},
	{0x062e, 0x062d, 1, 1, 1, 0xFC1A},
	{0x062e, 0x0645, 0xFCAC, 1, 1, 0xFC1B},
	{0x062e, 0x0649, 1, 1, 0xFD1F, 0xFD03},
	{0x062e, 0x064a, 1, 1, 0xFD20, 0xFD04},

	{0x0630, 0x0670, 1, 1, 1, 0xFC5b},

	{0x0631, 0x0670, 1, 1, 1, 0xFC5c},
	
	{0x0633, 0x062c, 0xFCAD, 0xFD34, 1, 0xFC1C},
	{0x0633, 0x062d, 0xFCAE, 0xFD35, 1, 0xFC1D},
	{0x0633, 0x062e, 0xFCAF, 0xFD36, 1, 0xFC1E},
	{0x0634, 0x0631, 1, 1, 0xFD2A, 0xFD0e},
	{0x0633, 0x0645, 0xFCB0, 0xFCE7, 1, 0xFC1F},
	{0x0633, 0x0647, 0xFD31, 0xFCE8, 1, 1},
	{0x0633, 0x0649, 1, 1, 0xFD17, 0xFCFB},
	{0x0633, 0x064a, 1, 1, 0xFD18, 0xFCFC},

	{0x0634, 0x062c, 0xFD2D, 0xFD37, 0xFD25, 0xFD09},
	{0x0634, 0x062d, 0xFD2E, 0xFD38, 0xFD26, 0xFD0a},
	{0x0634, 0x062e, 0xFD2F, 0xFD39, 0xFD27, 0xFD0b},
	{0x0634, 0x0631, 1, 1, 0xFD29, 0xFD0d},
	{0x0634, 0x0645, 0xFD30, 0xFCE9, 0xFD28, 0xFD0c},
	{0x0634, 0x0647, 0xFD32, 0xFCEA, 1, 1},
	{0x0634, 0x0649, 1, 1, 0xFD19, 0xFCFD},
	{0x0634, 0x064a, 1, 1, 0xFD1A, 0xFCFE},
	
	{0x0635, 0x062d, 0xFCB1, 1, 1, 0xFC20},
	{0x0635, 0x062e, 0xFCB2, 1, 1, 1},
	{0x0635, 0x0631, 1, 1, 0xFD2B, 0xFD0f},
	{0x0635, 0x0645, 0xFCB3, 1, 1, 0xFC21},
	{0x0635, 0x0649, 1, 1, 0xFD21, 0xFD05},
	{0x0635, 0x064a, 1, 1, 0xFD22, 0xFD06},

	{0x0636, 0x062c, 0xFCB4, 1, 1, 0xFC22},
	{0x0636, 0x062d, 0xFCB5, 1, 1, 0xFC23},
	{0x0636, 0x062e, 0xFCB6, 1, 1, 0xFC24},
	{0x0636, 0x0631, 1, 1, 0xFD2C, 0xFD10},
	{0x0636, 0x0645, 0xFCB7, 1, 1, 0xFC25},
	{0x0636, 0x0649, 1, 1, 0xFD23, 0xFD07},
	{0x0636, 0x064a, 1, 1, 0xFD24, 0xFD08},

	{0x0637, 0x062d, 0xFCB8, 1, 1, 0xFC26},
	{0x0637, 0x0645, 0xFD33, 0xFD3A, 1, 0xFC27},
	{0x0637, 0x0649, 1, 1, 0xFD11, 0xFCF5},
	{0x0637, 0x064a, 1, 1, 0xFD12, 0xFCF6},
	
	{0x0638, 0x0645, 0xFCB9, 0xFD3B, 1, 0xFC28},

	{0x0639, 0x062c, 0xFCBA, 1, 1, 0xFC29},
	{0x0639, 0x0645, 0xFCBB, 1, 1, 0xFC2A},
	{0x0639, 0x0649, 1, 1, 0xFD13, 0xFCF7},
	{0x0639, 0x064a, 1, 1, 0xFD14, 0xFCF8},

	{0x063A, 0x062c, 0xFCBC, 1, 1, 0xFC2B},
	{0x063A, 0x0645, 0xFCBD, 1, 1, 0xFC2C},
	{0x063A, 0x0649, 1, 1, 0xFD15, 0xFCF9},
	{0x063A, 0x064a, 1, 1, 0xFD16, 0xFCFA},

	{0x0641, 0x062c, 0xFCBE, 1, 1, 0xFC2D},
	{0x0641, 0x062d, 0xFCBF, 1, 1, 0xFC2E},
	{0x0641, 0x062e, 0xFCC0, 1, 1, 0xFC2F},
	{0x0641, 0x0645, 0xFCC1, 1, 1, 0xFC30},
	{0x0641, 0x0649, 1, 1, 0xFC7c, 0xFC31},
	{0x0641, 0x064a, 1, 1, 0xFC7d, 0xFC32},

	{0x0642, 0x062d, 0xFCC2, 1, 1, 0xFC33},
	{0x0642, 0x0645, 0xFCC3, 1, 1, 0xFC34},
	{0x0642, 0x0649, 1, 1, 0xFC7e, 0xFC35},
	{0x0642, 0x064a, 1, 1, 0xFC7f, 0xFC36},

	{0x0643, 0x0627, 1, 1, 0xFC80, 0xFC37},
	{0x0643, 0x062c, 0xFCC4, 1, 1, 0xFC38},
	{0x0643, 0x062d, 0xFCC5, 1, 1, 0xFC39},
	{0x0643, 0x062e, 0xFCC6, 1, 1, 0xFC3a},
	{0x0643, 0x0644, 0xFCC7, 0xFCEB, 0xFC81, 0xFC3b},
	{0x0643, 0x0645, 0xFCC8, 0xFCEC, 0xFC82, 0xFC3c},
	{0x0643, 0x0649, 1, 1, 0xFC83, 0xFC3d},
	{0x0643, 0x064a, 1, 1, 0xFC84, 0xFC3e},
	
	{0x0644, 0x0622, 1, 1, 0xFEF6, 0xFEf5},
	{0x0644, 0x0623, 1, 1, 0xFEF8, 0xFEF7},
	{0x0644, 0x0625, 1, 1, 0xFEFA, 0xFEF9},
	{0x0644, 0x0627, 1, 1, 0xFEFC, 0xFEFB},
	{0x0644, 0x062c, 0xFCC9, 1, 1, 0xFC3f},
	{0x0644, 0x062d, 0xFCCA, 1, 1, 0xFC40},
	{0x0644, 0x062e, 0xFCCB, 1, 1, 0xFC41},
	{0x0644, 0x0645, 0xFCCC, 0xFCED, 0xFC85, 0xFC42},
	{0x0644, 0x0647, 0xFCCD, 1, 1, 1},
	{0x0644, 0x0649, 1, 1, 0xFC86, 0xFC43},
	{0x0644, 0x064a, 1, 1, 0xFC87, 0xFC44},
	
	{0x0645, 0x0627, 1, 1, 0xFC88, 1},
	{0x0645, 0x062c, 0xFCCE, 1, 1, 0xFC45},
	{0x0645, 0x062d, 0xFCCF, 1, 1, 0xFC46},
	{0x0645, 0x062e, 0xFCD0, 1, 1, 0xFC47},
	{0x0645, 0x0645, 0xFCD1, 1, 0xFC89, 0xFC48},
	{0x0645, 0x0649, 1, 1, 1, 0xFC49},
	{0x0645, 0x064a, 1, 1, 1, 0xFC4a},

	{0x0646, 0x062c, 0xFCD2, 1, 1, 0xFC4b},
	{0x0646, 0x062d, 0xFCD3, 1, 1, 0xFC4c},
	{0x0646, 0x062e, 0xFCD4, 1, 1, 0xFC4d},
	{0x0646, 0x0631, 1, 1, 0xFC8a, 1},
	{0x0646, 0x0632, 1, 1, 0xFC8b, 1},
	{0x0646, 0x0645, 0xFCD5, 0xFCEE, 0xFC8c, 0xFC4e},
	{0x0646, 0x0646, 1, 1, 0xFC8d, 1},
	{0x0646, 0x0647, 0xFCD6, 0xFCEF, 1, 1},
	{0x0646, 0x0649, 1, 1, 0xFC8e, 0xFC4f},
	{0x0646, 0x064a, 1, 1, 0xFC8f, 0xFC50},

	{0x0647, 0x062c, 0xFCD7, 1, 1, 0xFC51},
	{0x0647, 0x0645, 0xFCD8, 1, 1, 0xFC52},
	{0x0647, 0x0649, 1, 1, 1, 0xFC53},
	{0x0647, 0x064a, 1, 1, 1, 0xFC54},
	{0x0647, 0x0670, 0xFCD9, 1, 1, 1},

	{0x0649, 0x0670, 1, 1, 0xFC90, 0xFC5d},
	
	{0x064a, 0x062c, 0xFCDA, 1, 1, 0xFC55},
	{0x064a, 0x062d, 0xFCDB, 1, 1, 0xFC56},
	{0x064a, 0x062e, 0xFCDC, 1, 1, 0xFC57},
	{0x064a, 0x0631, 1, 1, 0xFC91, 1},
	{0x064a, 0x0632, 1, 1, 0xFC92, 1},
	{0x064a, 0x0645, 0xFCDD, 0xFCF0, 0xFC93, 0xFC58},
	{0x064a, 0x0646, 1, 1, 0xFC94, 1},
	{0x064a, 0x0647, 0xFCDE, 0xFCF1, 1, 1},
	{0x064a, 0x0649, 1, 1, 0xFC95, 0xFC59},
	{0x064a, 0x064a, 1, 1, 0xFC96, 0xFC5a},

	// Hebrew
	{0xFB2A, 0x05BC, 0xFB2C, 0xFB2C, 0xFB2C, 0},
	{0xFB2B, 0x05BC, 0xFB2D, 0xFB2D, 0xFB2D, 0},
	{0xFB49, 0x05C1, 0xFB2C, 0xFB2C, 0xFB2C, 0},
	{0xFB49, 0x05C2, 0xFB2D, 0xFB2D, 0xFB2D, 0},

};

// the following vectors contain ranges ranges of characters that are
// never first/second part of a ligature; they are use to speedup
// processing and their contents are generated automatically when the
// class is initialised by the following two functions
static UT_GenericVector<UCSRange*> s_noLigature;

void GR_ContextGlyph::_generateNoLigatureTable()
{
	// handle the first entry separately ...
	UCSRange * pRange = new UCSRange;
	UT_return_if_fail(pRange);
	pRange->low = 0;
	pRange->high = s_ligature[0].code_low - 1;

	s_noLigature.addItem(pRange);
	xxx_UT_DEBUGMSG(("No Ligature table: adding {0x%04x, 0x%04x}\n",
				 pRange->low, pRange->high));
	
	UT_uint32 iLow = s_ligature[0].code_low;
	if(NrElements(s_ligature) > 1 &&
	   s_ligature[1].code_low == s_ligature[0].code_low + 1)
	{
		iLow++;
	}
	
	for(UT_uint32 i = 1; i < NrElements(s_ligature) - 1; i++)
	{
		if(s_ligature[i].code_low != iLow
		   && (s_bLatinLigatures || iLow < LATIN_LIGATURES_FIRST
			                     || iLow > LATIN_LIGATURES_LAST))
		{
			pRange = new UCSRange;
			UT_return_if_fail(pRange);
		
			pRange->low = iLow + 1;
			iLow = s_ligature[i].code_low;
			pRange->high = iLow - 1;
			
			s_noLigature.addItem(pRange);
			xxx_UT_DEBUGMSG(("No Ligature table: adding {0x%04x, 0x%04x}\n",
						 pRange->low, pRange->high));
		}

		if(s_ligature[i+1].code_low == s_ligature[i].code_low + 1
		   || (!s_bLatinLigatures && s_ligature[i+1].code_low >= LATIN_LIGATURES_FIRST
			                      && s_ligature[i+1].code_low <= LATIN_LIGATURES_LAST))
		{
			iLow++;
		}
		
	}

	// now handle the last element
	pRange = new UCSRange;
	UT_return_if_fail(pRange);
	pRange->low = iLow + 1;
	pRange->high = 0xffffffff;

	s_noLigature.addItem(pRange);
	xxx_UT_DEBUGMSG(("No Ligature table: adding {0x%04x, 0x%04x}\n",
				 pRange->low, pRange->high));
	
}

// The following table contains letters that have context-sensitive
// forms
// when adding entries to this table, make sure that you update
// the HEBREW_START and HEBREW_END macros that follow if necessary
static LetterData s_table[] =
{
	// code, intial, medial, final, stand-alone

	// Arabic
	{0x0621, 0x0621, 0x0621, 0x0621, 0xFE80},
	{0x0622, 0x0622, 0xFE82, 0xFE82, 0xFE81},
	{0x0623, 0x0623, 0xFE84, 0xFE84, 0xFE83},
	{0x0624, 0x0624, 0x0624, 0xFE86, 0xFE85},
	{0x0625, 0x0625, 0xFE88, 0xFE88, 0xFE87},
	{0x0626, 0xFE8B, 0xFE8C, 0xFE8A, 0xFE89},
	{0x0627, 0x0627, 0xFE8E, 0xFE8E, 0xFE8D},
	{0x0628, 0xFE91, 0xFE92, 0xFE90, 0xFE8F},
	{0x0629, 0x0629, 0x0629, 0xFE94, 0xFE93},
	{0x062a, 0xFE97, 0xFE98, 0xFE96, 0xFE95},
	{0x062b, 0xFE9B, 0xFE9C, 0xFE9A, 0xFE99},
	{0x062c, 0xFE9F, 0xFEA0, 0xFE9E, 0xFE9D},
	{0x062d, 0xFEA3, 0xFEA4, 0xFEA2, 0xFEA1},
	{0x062e, 0xFEA7, 0xFEA8, 0xFEA6, 0xFEA5},
	{0x062f, 0x062F, 0x062F, 0xFEAA, 0xFEA9},
	{0x0630, 0x0630, 0x0630, 0xFEAC, 0xFEAB},
	{0x0631, 0x0631, 0x0631, 0xFEAE, 0xFEAD},
	{0x0632, 0x0632, 0x0632, 0xFEB0, 0xFEAF},
	{0x0633, 0xFEB3, 0xFEB4, 0xFEB2, 0xFEB1},
	{0x0634, 0xFEB7, 0xFEB8, 0xFEB6, 0xFEB5},
	{0x0635, 0xFEBB, 0xFEBC, 0xFEBA, 0xFEB9},
	{0x0636, 0xFEBF, 0xFEC0, 0xFEBE, 0xFEBD},
	{0x0637, 0xFEC3, 0xFEC4, 0xFEC2, 0xFEC1},
	{0x0638, 0xFEC7, 0xFEC8, 0xFEC6, 0xFEC5},
	{0x0639, 0xFECB, 0xFECC, 0xFECA, 0xFEC9},
	{0x063a, 0xFECF, 0xFED0, 0xFECE, 0xFECD},
	{0x0641, 0xFED3, 0xFED4, 0xFED2, 0xFED1},
	{0x0642, 0xFED7, 0xFED8, 0xFED6, 0xFED5},
	{0x0643, 0xFEDB, 0xFEDC, 0xFEDA, 0xFED9},
	{0x0644, 0xFEDF, 0xFEE0, 0xFEDE, 0xFEDD},
	{0x0645, 0xFEE3, 0xFEE4, 0xFEE2, 0xFEE1},
	{0x0646, 0xFEE7, 0xFEE8, 0xFEE6, 0xFEE5},
	{0x0647, 0xFEEB, 0xFEEC, 0xFEEA, 0xFEE9},
	{0x0648, 0x0648, 0x0648, 0xFEEE, 0xFEED},
	{0x0649, 0x0649, 0x0649, 0xFEF0, 0xFEEF},
	{0x064a, 0xFEF3, 0xFEF4, 0xFEF2, 0xFEF1},

	{0x0671, 0x0671, 0x0671, 0xFB51, 0xFB50},
// 	{0x0672, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x0673, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x0674, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x0675, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x0676, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x0677, 0x0677, 0x0677, 0x0677, 0xFBDD},
// 	{0x0678, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x0679, 0xFB68, 0xFB69, 0xFB67, 0xFB66},
	{0x067a, 0xFB60, 0xFB61, 0xFB5F, 0xFB5E},
	{0x067b, 0xFB54, 0xFB55, 0xFB53, 0xFB52},
// 	{0x067c, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x067d, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x067e, 0xFB58, 0xFB59, 0xFB57, 0xFB56},
	{0x067f, 0xFB63, 0xFB65, 0xFB63, 0xFB62},
	{0x0680, 0xFB5C, 0xFB5D, 0xFB5B, 0xFB5A},
// 	{0x0681, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x0682, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x0683, 0xFB78, 0xFB79, 0xFB77, 0xFB76},
	{0x0684, 0xFB74, 0xFB75, 0xFB73, 0xFB72},
// 	{0x0685, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x0686, 0xFB7C, 0xFB7D, 0xFB7B, 0xFB7A},
	{0x0687, 0xFB80, 0xFB81, 0xFB7F, 0xFB7E},
	{0x0688, 0x0688, 0x0688, 0xFB89, 0xFB88},
// 	{0x0689, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x068a, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x068b, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x068c, 0x068c, 0x068c, 0xFB85, 0xFB84},
	{0x068d, 0x068d, 0x068d, 0xFB83, 0xFB82},
	{0x068e, 0x068e, 0x068e, 0xFB87, 0xFB86},
// 	{0x068f, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x0690, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x0691, 0x0691, 0x0691, 0xFB8D, 0xFB8C},
// 	{0x0692, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x0693, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x0694, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x0695, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x0696, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x0697, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x0698, 0x0698, 0x0698, 0xFB8B, 0xFB8A},
// 	{0x0699, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x069a, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x069b, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x069c, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x069d, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x069e, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x069f, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06a0, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06a1, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06a2, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06a3, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x06a4, 0xFB6C, 0xFB6D, 0xFB6B, 0xFB6A},
// 	{0x06a5, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x06a6, 0xFB70, 0xFB71, 0xFB6F, 0xFB6E},
// 	{0x06a7, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06a8, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x06a9, 0xFB90, 0xFB91, 0xFB8F, 0xFB8E},
// 	{0x06aa, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06ab, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06ac, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x06ad, 0xFBD5, 0xFBD6, 0xFBD4, 0xFBD3},
// 	{0x06ae, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x06af, 0xFB94, 0xFB95, 0xFB93, 0xFB92},
// 	{0x06b0, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x06b1, 0xFB9C, 0xFB9D, 0xFB9B, 0xFB9A},
// 	{0x06b2, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x06b3, 0xFB98, 0xFB99, 0xFB97, 0xFB96},
// 	{0x06b4, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06b5, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06b6, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06b7, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06b8, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06b9, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x06ba, 0x06ba, 0x06ba, 0xFB9F, 0xFB9E},
	{0x06bb, 0xFBA2, 0xFBA3, 0xFBA1, 0xFBA0},
// 	{0x06bc, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06bd, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x06be, 0xFBAC, 0xFBAD, 0xFBAB, 0xFBAA},
// 	{0x06bf, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x06c0, 0x06c0, 0x06c0, 0xFBA5, 0xFBA4},
	{0x06c1, 0xFBA8, 0xFBA9, 0xFBA7, 0xFBA6},
// 	{0x06c2, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06c3, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06c4, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x06c5, 0x06c5, 0x06c6, 0xFBE1, 0xFBE0},
	{0x06c6, 0x06c6, 0x06c6, 0xFBDA, 0xFBD9},
	{0x06c7, 0x06c7, 0x06c7, 0xFBD8, 0xFBD7},
	{0x06c8, 0x06c8, 0x06c8, 0xFBDC, 0xFBDB},
	{0x06c9, 0x06c9, 0x06c9, 0xFBE3, 0xFBE2},
// 	{0x06ca, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x06cb, 0x06cb, 0x06cb, 0xFBDF, 0xFBDE},
	{0x06cc, 0xFBFE, 0xFBFF, 0xFBFD, 0xFBFC},
// 	{0x06cd, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06ce, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06cf, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x06d0, 0xFBE6, 0xFBE7, 0xFBE5, 0xFBE4},
// 	{0x06d1, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x06d2, 0x06d2, 0x06d2, 0xFBAF, 0xFBAE},
	{0x06d3, 0x06d3, 0x06d3, 0xFBB1, 0xFBB0},
// 	{0x06d5, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06e5, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06e6, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06fa, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06fb, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06fc, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06fe, 0xFE, 0xFE, 0xFE, 0xFE},
};

// the following vector holds ranges of characters that do not require
// shaping; it is used to speed up processing and its contents are
// generated automatically by the function below
static UT_GenericVector<UCSRange*> s_noShaping;

void GR_ContextGlyph::_generateNoShapingTable()
{
	// handle the first entry separately ...
	UCSRange * pRange = new UCSRange;
	UT_return_if_fail(pRange);
	pRange->low = 0;
	pRange->high = s_table[0].code - 1;

	s_noShaping.addItem(pRange);
	xxx_UT_DEBUGMSG(("No Shaping table: adding {0x%04x, 0x%04x}\n",
				 pRange->low, pRange->high));
	
	UT_uint32 iPrev = s_table[0].code;
	if(NrElements(s_table) > 1 && s_table[0].code + 1 == s_table[1].code)
	{
		iPrev++;
	}
	
	for(UT_uint32 i = 1; i < NrElements(s_table) - 1; i++)
	{
		if(s_table[i].code != iPrev)
		{
			pRange = new UCSRange;
			UT_return_if_fail(pRange);
	
			pRange->low = iPrev + 1;
			iPrev = s_table[i].code;
			pRange->high = iPrev - 1;
			
			s_noShaping.addItem(pRange);
			xxx_UT_DEBUGMSG(("No Shaping table: adding {0x%04x, 0x%04x}\n",
						 pRange->low, pRange->high));
		}
		
		if(s_table[i].code + 1 == s_table[i+1].code)
		{
			iPrev++;
		}
	}

	// now handle the last element
	pRange = new UCSRange;
	UT_return_if_fail(pRange);
	pRange->low = iPrev + 1;
	pRange->high = 0xffffffff;

	s_noShaping.addItem(pRange);
	xxx_UT_DEBUGMSG(("No Shaping table: adding {0x%04x, 0x%04x}\n",
				 pRange->low, pRange->high));
}

// This table contains Unicode ranges of characters that do not join
// with the following character (excluding word delimters)
static UCSRange s_nonjoining_with_next[] =
{
	// everything in the Latin ranges
	{0x0000, 0x05ff},

	// Arabic
	{0x0622, 0x0625},
	{0x0627, 0x0627},
    {0x0629, 0x0629},
	{0x062f, 0x0632},
	{0x0648, 0x0648},
    {0x0671, 0x0673},
	{0x0688, 0x0699},
	{0x06C4, 0x06CB},
    {0x06D2, 0x06D3},
	{0x2000, 0x2fff}
};

// This table contains Unicode ranges of characters that do not join
// with the previous character (excluding word delimiters)
static UCSRange s_nonjoining_with_prev[] =
{
	// everything in the Latin ranges
	{0x0000, 0x05cf},

	// Arabic
	{0x0621, 0x0621},
	{0x2000, 0x2fff}
};

// This table contains Unicode ranges of characters that should be
// ignored (i.e., skipped over) by the shaping engine. Typically,
// these are combining characters

// for this to work, the ligature placeholder definition in ut_types.h
// has to match the value assumed here
static UCSRange s_ignore[] =
{
	// straight quotes
	{0x0022, 0x0022},
	{0x0027, 0x0027},
	
	// Hebrew overstriking characters
	{0x0591,0x05A1},
	{0x05A3,0x05B9},
	{0x05BB,0x05BD},
	{0x05BF,0x05BF},
	{0x05C1,0x05C2},
	{0x05C4,0x05C4},

	// Arabic overstriking characters
	{0x064B,0x0655},
	{0x06D6,0x06E8},
	{0x06EA,0x06ED},

	// smart quotes
	{0x2018,0x201F},

	// UCS_LIGATURE_PLACEHOLDER
	{0xf854, 0xf854}
};

// following three functions determine if shaping and ligature
// processing is required for a given character

/*!
    returns true if character c does not have context-sensitive variants
*/
bool GR_ContextGlyph::isNotContextSensitive(UT_UCS4Char c) const
{
	// I am not entirely sure whether this is really worth it, since
    // the last time I checked the glyph table had 88 entries, and the
    // no shaping table 30; however, most characters will exit in
	// first three cycles, and the glyph table is expanding.

	for(UT_uint32 i = 0; i < s_noShaping.getItemCount(); i++)
	{
		if(c >= s_noShaping.getNthItem(i)->low && c <= s_noShaping.getNthItem(i)->high)
		{
			return true;
		}
		
		if(c < s_noShaping.getNthItem(i)->low)
		{
			// we have already overshot ...
			return false;
		}
	}
	return false;
}

/*!
    returns true if character c never forms a first part of a ligature
*/
bool GR_ContextGlyph::isNotFirstInLigature(UT_UCS4Char c) const
{
	// this is worth it, since the ligature table is quite large and
	// the s_noLigature table is not only small, but most characters
	// will exit in the first three cycles of the loop
	for(UT_uint32 i = 0; i < s_noLigature.getItemCount(); i++)
	{
		if(c >= s_noLigature.getNthItem(i)->low && c <= s_noLigature.getNthItem(i)->high)
		{
			return true;
		}

		if(c < s_noLigature.getNthItem(i)->low)
		{
			// we have already overshot ...
			return false;
		}
	}
	return false;
}

// The following function determines if a given character c, followed
// by character f and preceded by character p will join with the
// following character
bool GR_ContextGlyph::isNotJoiningWithNext(UT_UCS4Char c, UT_UCS4Char f, UT_UCS4Char p) const
{
	for(UT_uint32 i = 0; i < NrElements(s_nonjoining_with_next); i++)
	{
		if(c >= s_nonjoining_with_next[i].low && c <= s_nonjoining_with_next[i].high)
			return true;
	}

	return UT_isWordDelimiter(c,f,p);
}

// The following function determines if a given character c, followed
// by character f and preceded by character p will join with the
// previous character
bool GR_ContextGlyph::isNotJoiningWithPrev(UT_UCS4Char c, UT_UCS4Char f, UT_UCS4Char p) const
{
	for(UT_uint32 i = 0; i < NrElements(s_nonjoining_with_prev); i++)
	{
		if(c >= s_nonjoining_with_prev[i].low && c <= s_nonjoining_with_prev[i].high)
			return true;
	}

	return UT_isWordDelimiter(c,f,p);
}


// comparison function for binary search of the glyph table
static int s_comp(const void *a, const void *b)
{
	const UT_UCSChar * A = static_cast<const UT_UCSChar*>(a);
	const LetterData * B = static_cast<const LetterData*>(b);

	if(*A < B->code)
		return -1;
	if(*A > B->code)
		return 1;

	return 0;

}

// comparison function for binary search of the table of ignored characters
static int s_comp_ignore(const void *a, const void *b)
{
	const UT_UCSChar* A = static_cast<const UT_UCSChar*>(a);
	const UCSRange* B = static_cast<const UCSRange*>(b);
	if(*A < B->low)
		return -1;
	if(*A > B->high)
		return 1;

	return 0;
}


// The following function is used for binary search of the ligature table.
static int s_comp_lig(const void *a, const void *b)
{
	const LigatureSequence * A = static_cast<const LigatureSequence*>(a);
	const LigatureData     * B = static_cast<const LigatureData*>(b);

	int ret = static_cast<int>(A->code) - static_cast<int>(B->code_low);
	if(!ret)
		ret = static_cast<int>(A->next) - static_cast<int>(B->code_high);

	return ret;
}
#endif

/*
   SMART QUOTES HANDLING

   This is how it works: each type of smart quote is described by a
   LetterData structure in which the first two fields are unused;
   these are named after one of the languages that uses them and the
   character, e.g. s_sq__single_en_us.

   The main table is made up of SmartQuote structures, it contains all
   languages that behave differently than en_US; if the language does
   not want smart quotes, the pGlyphs pointer is set to NULL,
   otherwise it points to one of the s_sg_*_* definitions; please check
   that a given definition is not already present before creating a
   new one!

*/
typedef struct
{
	const XML_Char *   pLang;
	UT_UCS4Char  character;
	LetterData *     pGlyphs;
}SmartQuote;

	// 0, 0, initial, medial, final, alone

static LetterData s_sq_single_en_us =
	{0, 0x2018, 0x2019, 0x2019, 0x0027};

static LetterData s_sq_double_en_us =
	{0, 0x201C, 0x201D, 0x201D, 0x0022};



static LetterData s_sq_single_cs_cz = 
	{0, 0x201A, 0x2019, 0x2019, 0x0027};

static LetterData s_sq_double_cs_cz = 
	{0, 0x201E, 0x201D, 0x201D, 0x0022};


// NB: the lang strings get replaced by pointers into the static table
// of UT_Language; this speeds up comparisons
static SmartQuote s_smart_quotes[] =
{	
	{"cs-CZ", 0x0027, &s_sq_single_cs_cz},
	{"cs-CZ", 0x0022, &s_sq_double_cs_cz},
	{"sk-SK", 0x0027, &s_sq_single_cs_cz},
	{"sk-SK", 0x0022, &s_sq_double_cs_cz}
};

static SmartQuote s_smart_quotes_default[] =
{	
	{NULL, 0x0027, &s_sq_single_en_us},
	{NULL, 0x0022, &s_sq_double_en_us}
};

// if the character has a mirror form, returns the mirror form,
// otherwise returns the character itself
static UT_UCSChar s_getMirrorChar(UT_UCSChar c)
{
	//got to do this, otherwise bsearch screws up
	UT_UCS4Char mc;

	if (UT_bidiGetMirrorChar(c,mc))
		return mc;
	else
		return c;
}

// Initialisation of static members
bool             GR_ContextGlyph::s_bInit           = false;
const XML_Char * GR_ContextGlyph::s_pEN_US          = NULL;
UT_UCS4Char      GR_ContextGlyph::s_cDefaultGlyph   = '?';
bool             GR_ContextGlyph::s_bLatinLigatures = true;


////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GR_ContextGlyph::GR_ContextGlyph()
{
	if(!s_bInit)
	{
#ifndef NO_BIDI_SUPPORT
		// UCS_LIGATURE_PLACEHOLDER defined in ut_types.h must equal
		// to the falue hardcode in our tables, 0xF854
		UT_ASSERT(UCS_LIGATURE_PLACEHOLDER == 0xF854);

		XAP_App::getApp()->getPrefsValueBool((XML_Char*)XAP_PREF_KEY_LatinLigatures,
											 &s_bLatinLigatures);

		// generate the no-shaping and no-ligature tables
		_generateNoLigatureTable();
		_generateNoShapingTable();

		UT_DEBUGMSG(("UT_ContextGlyph: glyphs: %d, ligatures: %d, no-lig: %d, no-shap: %d\n",
					 NrElements(s_table), NrElements(s_ligature),
					 s_noLigature.getItemCount(), s_noShaping.getItemCount()));
		
		// init the smart quote tables with the pointers to language
		// codes in UT_Laguage
#endif
		
		UT_Language lang;
		s_pEN_US = lang.getCodeFromCode("en-US");
		for(UT_uint32 i = 0; i < NrElements(s_smart_quotes); i++)
		{
			s_smart_quotes[i].pLang = lang.getCodeFromCode(s_smart_quotes[i].pLang);
		}

		// retrieve the default glyph and any other preferences we
		// depend on
		_prefsListener(XAP_App::getApp(), NULL, NULL, NULL);
		
		// add a listener to the preferences, so that we know when the
		// user changed his/her mind
		XAP_App::getApp()->getPrefs()->addListener(_prefsListener,NULL);
		s_bInit = true;
	}
}

// can be used to deallocate the static tables
void GR_ContextGlyph::static_destructor()
{
	UT_DEBUGMSG(("GR_ContextGlyph::static_destructor() called\n"));

#ifndef NO_BIDI_SUPPORT
	if(s_bInit)
	{
		s_bInit = false;
		UT_VECTOR_PURGEALL(UCSRange*,s_noLigature);  s_noLigature.clear();
		UT_VECTOR_PURGEALL(UCSRange*,s_noShaping);   s_noShaping.clear();
	}
#endif
}

void GR_ContextGlyph::_prefsListener(XAP_App *pApp, XAP_Prefs *, UT_StringPtrMap *, void *)
{
	UT_return_if_fail(pApp);

	// retrieve the default glyph to remap missing glyphs to ...
	const XML_Char *default_utf8;
	UT_GrowBuf gb;
	
	bool bNoErr = pApp->getPrefsValue(static_cast<const XML_Char*>(XAP_PREF_KEY_RemapGlyphsDefault),
									  &default_utf8);
	UT_ASSERT( bNoErr );
		
	UT_decodeUTF8string(default_utf8, UT_XML_strlen(default_utf8), &gb);

	if(gb.getPointer(0))
		s_cDefaultGlyph = *(gb.getPointer(0));
	else
		s_cDefaultGlyph = '?';

#ifndef NO_BIDI_SUPPORT
	bool bLChange  = s_bLatinLigatures;
	bNoErr = XAP_App::getApp()->getPrefsValueBool((XML_Char*)XAP_PREF_KEY_LatinLigatures,
												  &s_bLatinLigatures);
	UT_ASSERT( bNoErr );
	
	bLChange ^= s_bLatinLigatures;
	
	if(bLChange)
	{
		UT_DEBUGMSG(("UT_ContextGlyph::prefs: latin ligatures preference changed, now %d\n",
					 s_bLatinLigatures));

		UT_VECTOR_PURGEALL(UCSRange*,s_noLigature);  s_noLigature.clear();
		_generateNoLigatureTable();
	}
	
	UT_DEBUGMSG(("UT_ContextGlyph::prefs: glyphs %d, ligatures: %d, no-lig: %d, no-shap: %d\n",
				 NrElements(s_table), NrElements(s_ligature),
				 s_noLigature.getItemCount(), s_noShaping.getItemCount()));
#endif
}

/*!
    Finds the description for the smart quote for given language and character
    IMPORTANT: pLang must be a pointer directly into the static table of
    UT_Language, not just an arbitrary pointer !!!
*/
const LetterData * GR_ContextGlyph::smartQuote(UT_UCS4Char c, const XML_Char * pLang) const
{
	UT_uint32 i;
	if(!pLang)
	{
		return NULL;
	}
	
	if(pLang != s_pEN_US)
	{
		for(i = 0; i < NrElements(s_smart_quotes); i++)
		{
			if(s_smart_quotes[i].pLang == pLang && c == s_smart_quotes[i].character)
				return s_smart_quotes[i].pGlyphs;
		}
	}
	
	// got this far, requires generic (en_US) behaviour
	for(i = 0; i < NrElements(s_smart_quotes_default); i++)
	{
		if(c == s_smart_quotes_default[i].character)
			return s_smart_quotes_default[i].pGlyphs;
	}

	return NULL;
}

/*!
    returns smart quote glyph for given character c in context defined
    by prev and next in text using language described by code pLang;

    /param c - quote character
    /param prev -  pointer to buffer of size CONTEXT_BUFF_SIZE
                   containing characters preceeding c; the characters
                   in the buffer are in reversed order, i.e., prev[0]
                   is the character immediately before c
    /param next -  pointer to buffer of size CONTEXT_BUFF_SIZE
                   containing characters following c;

    /param pLang - pointer to a language code
    /param isGlyphAvailable - pointer to a function that determines if
                   the smart quote qlyph is available; if the pointer
				   is NULL, all glyphs are considered available
*/
UT_UCS4Char GR_ContextGlyph::getSmartQuote(UT_TextIterator & text,
										   const XML_Char   * pLang,
										   bool (*isGlyphAvailable)(UT_UCS4Char g, void * param),
										   void * fparam) const
{
	UT_return_val_if_fail(text.getStatus() == UTIter_OK, UT_IT_ERROR);
	UT_UCS4Char c = text.getChar();
	
	const LetterData   * pLet = smartQuote(c,pLang);

	if(!pLet)
		return c;

	GlyphContext context = _evalGlyphContext(text);
	UT_UCS4Char glyph;

	switch (context)
	{
		case GC_INITIAL:
			glyph = pLet->initial;
		case GC_MEDIAL:
			glyph = pLet->medial;
		case GC_FINAL:
			glyph = pLet->final;
		case GC_ISOLATE:
			glyph = pLet->alone;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	if(isGlyphAvailable == NULL || isGlyphAvailable(glyph, fparam))
		return glyph;

	return c;
}

GlyphContext GR_ContextGlyph::_evalGlyphContext(UT_TextIterator & text, UT_sint32 offset) const
{
	UT_ASSERT( text.getStatus() == UTIter_OK );

	UT_uint32 pos = text.getPosition();
	UT_UCS4Char current = text[pos + offset];

	UT_return_val_if_fail(text.getStatus() == UTIter_OK, GC_ISOLATE);
		
	UT_UCS4Char next;
	++text;
	next = text.getStatus() == UTIter_OK ? text.getChar() : 0;
	
	UT_UCS4Char prev;
	text.setPosition(pos + offset - 1);
	prev = text.getStatus() == UTIter_OK ? text.getChar() : 0;

	if(!next && !prev)
		return GC_ISOLATE;

	bool bPrevWD;
	bool bNextWD;

	UT_UCS4Char prev2;
	text.setPosition(pos + offset - 2);
	prev2 = text.getStatus() == UTIter_OK ? text.getChar() : 0;
	
	if(!next && prev)
	{
#ifndef NO_BIDI_SUPPORT
		// text is pointing at prev, move one char to the left
		bPrevWD = isNotJoiningWithNext(prev, current, prev2);
		
		if(bPrevWD)
			return GC_ISOLATE;
		else
			return GC_FINAL;
#else
		return GC_FINAL;
#endif
	}

	// no-next has been trapped above, now we can check if next is not
	// a character that is to be ignored (the indexing operator is
	// necessary here, to position the iterator for us)
	UT_UCSChar myNext = text[pos + offset + 1];

#ifndef NO_BIDI_SUPPORT
	while(text.getStatus() == UTIter_OK
		  && bsearch(static_cast<void*>(&myNext),
					 static_cast<void*>(s_ignore),
					 NrElements(s_ignore),
					 sizeof(UCSRange),
					 s_comp_ignore))
	{
		myNext = (++text).getChar();
	}
#endif
	
	UT_UCS4Char myNext2 = UCS_SPACE;
	
	if(text.getStatus() == UTIter_OK)
	{
		++text;
		if(text.getStatus() == UTIter_OK)
			myNext2 = text.getChar();
	}
	
	xxx_UT_DEBUGMSG(("GR_ContextGlyph::_eval: myNext 0x%x\n", myNext));

	if(!prev && !myNext)
		return GC_ISOLATE;

	if(!prev && myNext)
	{
#ifndef NO_BIDI_SUPPORT
		bNextWD = isNotJoiningWithPrev(myNext, myNext2, UCS_UNKPUNK);
		if(bNextWD)
			return GC_ISOLATE;
		else
			return GC_INITIAL;
#else
		return GC_INITIAL;
#endif
	}

	if(prev && !myNext)
	{
#ifndef NO_BIDI_SUPPORT
		bPrevWD = isNotJoiningWithNext(prev, current, prev2);
		if(bPrevWD)
			return GC_ISOLATE;
		else
			return GC_FINAL;
#else
		return GC_FINAL;
#endif
	}

#ifndef NO_BIDI_SUPPORT
	bPrevWD = isNotJoiningWithNext(prev, current, prev2);
	bNextWD = isNotJoiningWithPrev(myNext, myNext2, UCS_UNKPUNK);
#else
	bPrevWD = UT_isWordDelimiter(prev,current,prev2);
	bNextWD = UT_isWordDelimiter(myNext,myNext2,UCS_UNKPUNK);
#endif
	xxx_UT_DEBUGMSG(("GR_ContextGlyph::_eval: 0x%x, prev 0x%x, myNext 0x%x, myNextNext 0x%x, prevjoin %i, nextjoin %i\n",
					 current,prev, myNext,myNext2, bPrevWD, bNextWD ));
    
	// if both are not , then medial form is needed
	if(!bPrevWD && !bNextWD)
		return GC_MEDIAL;

	// if only *next is, than final form is needed
	if(bNextWD && !bPrevWD)
		return GC_FINAL;

	// if *prev is, the initial form is needed
	if(bPrevWD && !bNextWD)
		return GC_INITIAL;

	// if we got here, both are delimiters, which means stand alone form is needed
	return GC_ISOLATE;
}

#ifndef NO_BIDI_SUPPORT
/*!
    render string using glyph and ligature tables
    returns value indicating what kind of characters this text contained
 */
GRShapingResult GR_ContextGlyph::renderString(UT_TextIterator & text,
											  UT_UCSChar *dest,
											  UT_uint32 len,
											  const XML_Char   * pLang,
											  UT_BidiCharType    iDirection,
											  bool (*isGlyphAvailable)(UT_UCS4Char g, void * param),
											  const void * fparam) const
{
	UT_return_val_if_fail(text.getStatus() == UTIter_OK, GRSR_Error);
	UT_return_val_if_fail(dest, GRSR_Error);

	UT_UCSChar       * dst_ptr = dest;

	UT_uint32 pos = text.getPosition();

	UT_UCS4Char next, prev, current, nextL;
	UT_UCS4Char glyph = 0;
	UT_UCS4Char glyph2 = 2;

	bool bLigature = false;
	bool bContextSensitive = false;
	
	for(UT_uint32 i = 0; i < len; i++)
	{
		LigatureSequence     Lig;
		const LetterData   * pLet = 0;
		const LigatureData * pLig = 0;
		GlyphContext         context = GC_NOT_SET;
		
		current = text[pos + i];
		UT_return_val_if_fail(text.getStatus() == UTIter_OK, GRSR_Error);
		
		//get the current context
		next = text[pos + i + 1];
		if(text.getStatus() != UTIter_OK)
			next = 0;

		if(i < len - 1)
			nextL = next;
		else
			nextL = 0;
				
		prev = text[pos + i - 1];
		if(text.getStatus() != UTIter_OK)
			prev = 0;

		// decide if this is a part of a ligature
		// check for a ligature form
		bool bFirstInLig = !isNotFirstInLigature(current);
		bLigature |= bFirstInLig;
		
		if(nextL && bFirstInLig)
		{
			Lig.next = nextL;
			Lig.code = current;

			pLig = static_cast<LigatureData*>(bsearch(static_cast<void*>(&Lig),
													  static_cast<void*>(s_ligature),
													  NrElements(s_ligature),
													  sizeof(LigatureData),
													  s_comp_lig));
		}
			
		if(pLig)
		{
			// we need the context of the whole pair not just of this
			// character
			glyph2 = nextL;
			next = text[pos + i + 2];
			
			xxx_UT_DEBUGMSG(("GR_ContextGlyph::render: 0x%x, 1st of lig.\n", *code));

			// we use 0 in the isolate form to indicate that
			// ligature is not context sensitive
			if(pLig->alone)
			{
				bContextSensitive = true;
				text.setPosition(pos + i);
				context = _evalGlyphContext(text);
				
				switch (context)
				{
					case GC_INITIAL:
						glyph = pLig->initial;
						break;
					case GC_MEDIAL:
						glyph = pLig->medial;
						break;
					case GC_FINAL:
						glyph = pLig->final;
						break;
					case GC_ISOLATE:
						glyph = pLig->alone;
						break;
					default:
						UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				}

				if(glyph == 1)
				{
					// no special form exists in this context, process
					// as ordinary characters
					goto ligature_form_missing;
				}
			}
			else
			{
				// not context sensitive, use the initial value
				glyph = pLig->initial;
			}
			
		

			xxx_UT_DEBUGMSG(("GR_ContextGlyph::render: lig.(%d), glyph 0x%x\n",bIsSecond,glyph));


			bool bGlyphAvailable = (isGlyphAvailable == NULL || isGlyphAvailable(glyph, (void*)fparam));
			
			if(!bGlyphAvailable)
			{
				UT_DEBUGMSG(("GR_ContextGlyph::render [1a] glyph 0x%x not present in font\n",
							 glyph));
				// we need to use the original glyphs; glyph2 is
				// already set
				glyph = current;
			}
			else
			{
				// just need to set glyph2 to the placement marker
				glyph2 = UCS_LIGATURE_PLACEHOLDER;
			}
			
			
			
			if(glyph != 1) //first part of a ligature
			{
				// we set both this and the next char if we can
				if(bGlyphAvailable)
				{
					// we have the ligature glyph, so use it
					*dst_ptr++ = glyph;

					if(i < len - 1)
					{
						*dst_ptr++ = glyph2; // the ligature placeholder
						i++;
					}
				}
				else if(isGlyphAvailable == NULL || isGlyphAvailable(glyph, (void*)fparam))
				{
					// we have the original glyph, so we will use it
					*dst_ptr++ = glyph;
				}
				else
				{
					// bad luck, not even the original glyph exists
					UT_DEBUGMSG(("GR_ContextGlyph::render [1b] glyph 0x%x not present in font\n",
								 glyph));

					*dst_ptr++ = s_cDefaultGlyph;
				}

				
				continue;
			}
			// if we got here, the glyph was 1, which means this form is to be just
			// treated as an ordinary letter, also if this was a first part of the ligature
			// we already know its context, but not if it was a second part of lig.
		}
		ligature_form_missing:
		
		// if we have no pLig we are dealing with an ordinary
		// letter
		if(!isNotContextSensitive(current))
		{
				
			pLet = static_cast<LetterData*>(bsearch(static_cast<const void*>(&current),
													static_cast<void*>(s_table),
													NrElements(s_table),
													sizeof(LetterData),
													s_comp));
		}
			
		// if we have no pLet, it means the letter has only one form
		// last thing to do is to deal with mirror characters
		if(!pLet)
		{
			if(iDirection == UT_BIDI_RTL)
				glyph = s_getMirrorChar(current);
			else
				glyph = current;

			if(isGlyphAvailable == NULL || isGlyphAvailable(glyph, (void*)fparam))
				*dst_ptr++ = glyph;
			else
			{
				UT_DEBUGMSG(("GR_ContextGlyph::render [2a] glyph 0x%x not present in font\n",
							 glyph));
				
				UT_UCS4Char t = _remapGlyph(glyph);
				if(isGlyphAvailable != NULL && isGlyphAvailable(t, (void*)fparam))
				{
					*dst_ptr++ = t;
				}
				else
				{
					UT_DEBUGMSG(("GR_ContextGlyph::render [2b] glyph 0x%x not present in font\n",
								 t));
					
					*dst_ptr++ = s_cDefaultGlyph;
				}
			}
			continue;
		}

		// if we got this far, we are dealing with a context sensitive character
		bContextSensitive = true;
		if(context == GC_NOT_SET)
		{
			text.setPosition(pos + i);
			context = _evalGlyphContext(text);
		}
		
		switch (context)
		{
			case GC_INITIAL:
				glyph = pLet->initial;
				break;
			case GC_MEDIAL:
				glyph = pLet->medial;
				break;
			case GC_FINAL:
				glyph = pLet->final;
				break;
			case GC_ISOLATE:
				glyph = pLet->alone;
				break;
			default:
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}

		if(isGlyphAvailable == NULL || isGlyphAvailable(glyph, (void*)fparam))
		{
			*dst_ptr++ = glyph;
		}
		else if(isGlyphAvailable(current, (void*)fparam))
		{
			UT_DEBUGMSG(("GR_ContextGlyph::render [3a] glyph 0x%x not present in font\n",
						 glyph));
			
			*dst_ptr++ = current;
		}
		else
		{
			UT_DEBUGMSG(("GR_ContextGlyph::render [3b] glyph 0x%x not present in font\n",
						 glyph));
			UT_DEBUGMSG(("GR_ContextGlyph::render [3c] glyph 0x%x not present in font\n",
						 current));

			UT_UCS4Char t = _remapGlyph(glyph);
			if(isGlyphAvailable != NULL && isGlyphAvailable(t, (void*)fparam))
				*dst_ptr++ = t;
			else
			{
				UT_DEBUGMSG(("GR_ContextGlyph::render [3d] glyph 0x%x not present in font\n",
							 glyph));
				
				*dst_ptr++ = s_cDefaultGlyph;
			}
		}
	}

	if(bLigature && !bContextSensitive)
		return GRSR_Ligatures;
	
	if(!bLigature && bContextSensitive)
		return GRSR_ContextSensitive;
	
	if(bLigature && bContextSensitive)
		return GRSR_ContextSensitiveAndLigatures;

	return GRSR_None;
}
#endif

/*! copy string into provided destination remapping any missing glyphs
    in the process; return value is always GRSR_None or GRSR_Error (this
    function is intended to be used in place of renderString() when
    the caller knows that no shaping and ligating is required). For
    this reason the function signature is identical to that of
    renderString()
*/
GRShapingResult GR_ContextGlyph::copyString(UT_TextIterator & text,
											UT_UCSChar *dest,
											UT_uint32 len,
											const XML_Char * /*pLang*/,
											UT_BidiCharType iDirection,
											bool (*isGlyphAvailable)(UT_UCS4Char g, void * param),
											const void * fparam) const
{
	UT_return_val_if_fail(dest, GRSR_Error);

	UT_UCS4Char  * dst_ptr = dest;
	UT_UCS4Char current, glyph;

	for(UT_uint32 i = 0; i < len; ++i, ++text)
	{
		UT_return_val_if_fail(text.getStatus() == UTIter_OK, GRSR_Error);
		current = text.getChar();
		
		// deal with mirror characters
		if(iDirection == UT_BIDI_RTL)
			glyph = s_getMirrorChar(current);
		else
			glyph = current;

		if(isGlyphAvailable == NULL || isGlyphAvailable(glyph, (void*)fparam))
			*dst_ptr++ = glyph;
		else
		{
			UT_DEBUGMSG(("GR_ContextGlyph::copy [1] glyph 0x%x not present in font\n", glyph));
				
			UT_UCS4Char t = _remapGlyph(glyph);
			if(isGlyphAvailable != NULL && isGlyphAvailable(t, (void*)fparam))
			{
				*dst_ptr++ = t;
			}
			else
			{
				UT_DEBUGMSG(("GR_ContextGlyph::copy [2] glyph 0x%x not present in font\n", t));
				*dst_ptr++ = s_cDefaultGlyph;
			}
		}
	}

	return GRSR_None;
}

/*!
    this function remaps a number of specialised glyphs to reasonable
    alternatives
*/
UT_UCS4Char GR_ContextGlyph::_remapGlyph(UT_UCS4Char g)
{
	// various hyphens
	if(g >= 0x2010 && g <= 0x2015) return '-';

	// various quotes
	if(g >= 0x2018 && g <= 0x201b) return '\'';
	if(g == 0x2039) return '<';
	if(g == 0x203a) return '>';
	if(g >= 0x201c && g <= 0x201f) return '\"';

	// bullets
	if(g == 0x2022 || g == 0x2023) return '*';

	// miscell. punctuation
	if(g == 0x2044) return '/';
	if(g == 0x2045) return '[';
	if(g == 0x2046) return ']';
	if(g == 0x2052) return '%';
	if(g == 0x2053) return '~';

	// currency symbols
	if(g == 0x20a3) return 'F';
	if(g == 0x20a4) return 0x00a3;
	if(g == 0x20ac) return 'E';

	// letter like symbols
	if(g == 0x2103) return 'C';
	if(g == 0x2109) return 'F';
	if(g == 0x2117) return 0x00a9;
	if(g == 0x2122) return 'T'; // TM symbol, this is not satisfactory, but ...
	if(g == 0x2126) return 0x03a9;
	if(g == 0x212a) return 'K';

	// dingbats
	if(g >= 0x2715 && g <= 0x2718) return 0x00d7;
	if(g >= 0x2719 && g <= 0x2720) return '+';
	if(g == 0x271) return '*';
	if(g >= 0x2722 && g <= 0x2727) return '+';
	if(g >= 0x2728 && g <= 0x274b) return '*';
	if(g >= 0x2758 && g <= 0x275a) return '|';
	if(g == 0x275b || g == 0x275c) return '\'';
	if(g == 0x275d || g == 0x275e) return '\"';
	if(g == 0x2768 || g == 0x276a) return '(';
	if(g == 0x2769 || g == 0x276b) return ')';
	if(g == 0x276c || g == 0x276e || g == 0x2770) return '<';
	if(g == 0x276d || g == 0x276f || g == 0x2771) return '>';
	if(g == 0x2772) return '[';
	if(g == 0x2773) return ']';
	if(g == 0x2774) return '{';
	if(g == 0x2775) return '}';
	if(g >= 0x2776 && g <= 0x2793) return ((g-0x2775)%10 + '0');

	return g;
}
