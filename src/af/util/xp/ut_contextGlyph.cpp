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

#include "ut_contextGlyph.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Language.h"

#include "xap_App.h"
#include "xap_Prefs.h"

struct LigatureSequence
{
	UT_UCSChar code;
	UT_UCSChar next;
	//UT_UCSChar prev;
};

// The following table contains ligature data for two-character
// ligatures (code_low is the first character, code_high the second).
// 
// this table has to be sorted by the first two numbers !!!
// use 1 to indicate that no ligature glyph exists
// 
static LigatureData s_ligature[] =
{
	// code_low, code_high, intial, medial, final, stand-alone
	
	// Latin
	// For now these are disabled, because the glyphs might not be
	// present in the fonts
	/*
	{0x0066, 0x0066, 0xFB00, 0xFB00, 0xFB00, 0xFB00},
	{0x0066, 0x0069, 0xFB01, 0xFB01, 0xFB01, 0xFB01},
	{0x0066, 0x006C, 0xFB02, 0xFB02, 0xFB02, 0xFB02},
	{0x017F, 0x0074, 0xFB05, 0xFB05, 0xFB05, 0xFB05},
	*/

	// Greek
	{0x0391, 0x0301, 0x0386, 0x0386, 0x0386, 0x0386},
	{0x0395, 0x0301, 0x0388, 0x0388, 0x0388, 0x0388},
	{0x0397, 0x0301, 0x0389, 0x0389, 0x0389, 0x0389},
	{0x0399, 0x0301, 0x038a, 0x038a, 0x038a, 0x038a},
	{0x0399, 0x0308, 0x03aa, 0x03aa, 0x03aa, 0x03aa},
	{0x039F, 0x0301, 0x038c, 0x038c, 0x038c, 0x038c},
	{0x03A5, 0x0301, 0x038e, 0x038e, 0x038e, 0x038e},
	{0x03A5, 0x0308, 0x03ab, 0x03ab, 0x03ab, 0x03ab},
	{0x03A9, 0x0301, 0x038f, 0x038f, 0x038f, 0x038f},
	{0x03B1, 0x0301, 0x03ac, 0x03ac, 0x03ac, 0x03ac},
	{0x03B5, 0x0301, 0x03ad, 0x03ad, 0x03ad, 0x03ad},
	{0x03B7, 0x0301, 0x03ae, 0x03ae, 0x03ae, 0x03ae},
	{0x03B8, 0x0308, 0x03ca, 0x03ca, 0x03ca, 0x03ca},
	{0x03B9, 0x0301, 0x03af, 0x03af, 0x03af, 0x03af},
	{0x03BF, 0x0301, 0x03cc, 0x03cc, 0x03cc, 0x03cc},
	{0x03C5, 0x0301, 0x03cd, 0x03cd, 0x03cd, 0x03cd},
	{0x03C5, 0x0308, 0x03cb, 0x03cb, 0x03cb, 0x03cb},
	{0x03C9, 0x0301, 0x03ce, 0x03ce, 0x03ce, 0x03ce},
	{0x03CA, 0x0301, 0x0390, 0x0390, 0x0390, 0x0390},
	{0x03CB, 0x0301, 0x03b0, 0x03b0, 0x03b0, 0x03b0},

	// Hebrew
	{0x05D0, 0x05B7, 0xFB2E, 0xFB2E, 0xFB2E, 0xFB2E},
	{0x05D0, 0x05B8, 0xFB2F, 0xFB2F, 0xFB2F, 0xFB2F},
	{0x05D0, 0x05BC, 0xFB30, 0xFB30, 0xFB30, 0xFB30},
	{0x05D1, 0x05BC, 0xFB31, 0xFB31, 0xFB31, 0xFB31},
	{0x05D2, 0x05BC, 0xFB32, 0xFB32, 0xFB32, 0xFB32},
	{0x05D3, 0x05BC, 0xFB33, 0xFB33, 0xFB33, 0xFB33},
	{0x05D4, 0x05BC, 0xFB34, 0xFB34, 0xFB34, 0xFB34},
	{0x05D5, 0x05B9, 0xFB4B, 0xFB4B, 0xFB4B, 0xFB4B},
	{0x05D5, 0x05BC, 0xFB35, 0xFB35, 0xFB35, 0xFB35},
	{0x05D6, 0x05BC, 0xFB36, 0xFB36, 0xFB36, 0xFB36},
	{0x05D8, 0x05BC, 0xFB38, 0xFB38, 0xFB38, 0xFB38},
	/*{0x05D9, 0x05B4, 0xFB1D, 0xFB1D, 0xFB1D, 0xFB1D},*/ //not found in MS fonts
	{0x05D9, 0x05BC, 0xFB39, 0xFB39, 0xFB39, 0xFB39},
	{0x05DA, 0x05BC, 0xFB3B, 0xFB3B, 0xFB3A, 0xFB3A},
	{0x05DB, 0x05BC, 0xFB3B, 0xFB3B, 0xFB3A, 0xFB3A},
	{0x05DC, 0x05BC, 0xFB3C, 0xFB3C, 0xFB3C, 0xFB3C},
	{0x05DE, 0x05BC, 0xFB3E, 0xFB3E, 1, 1},
	{0x05E0, 0x05BC, 0xFB40, 0xFB40, 1, 1},
	{0x05E1, 0x05BC, 0xFB41, 0xFB41, 0xFB41, 0xFB41},
	{0x05E3, 0x05BC, 0xFB44, 0xFB44, 0xFB43, 0xFB43},
	{0x05E4, 0x05BC, 0xFB44, 0xFB44, 0xFB43, 0xFB43},
	{0x05E6, 0x05BC, 0xFB46, 0xFB46, 1, 1},
	{0x05E7, 0x05BC, 0xFB47, 0xFB47, 0xFB47, 0xFB47},
	{0x05E8, 0x05BC, 0xFB48, 0xFB48, 0xFB48, 0xFB48},
	{0x05E9, 0x05BC, 0xFB49, 0xFB49, 0xFB49, 0xFB49},
	{0x05E9, 0x05C1, 0xFB2A, 0xFB2A, 0xFB2A, 0xFB2A},
	{0x05E9, 0x05C2, 0xFB2B, 0xFB2B, 0xFB2B, 0xFB2B},
	{0x05EA, 0x05BC, 0xFB4A, 0xFB4A, 0xFB4A, 0xFB4A},

	// Arabic
	{0x0626, 0x0627, 1, 1, 0xFBEB, 0xFBEA},
	{0x0626, 0x062c, 0xFC97, 1, 1, 0xFC00},
	{0x0626, 0x062d, 0xFC98, 1, 1, 0xFC01},
	{0x0626, 0x062e, 0xFC99, 1, 1, 1},
	{0x0626, 0x0631, 1, 1, 0xFC64, 1},
	{0x0626, 0x0632, 1, 1, 0xFC65, 1},
	{0x0626, 0x0645, 0xFC9A, 1, 0xFC66, 0xFC02},
	{0x0626, 0x0646, 1, 1, 0xFC67, 1},
	{0x0626, 0x0647, 0xFC9b, 1, 1, 1},
	{0x0626, 0x0648, 1, 1, 0xFBEF, 0xFBEE},
	{0x0626, 0x0649, 1, 1, 0xFC68, 0xFC03},
	{0x0626, 0x064a, 1, 1, 0xFC69, 0xFC04},
	{0x0626, 0x06c7, 1, 1, 0xFBF1, 0xFBF0},
	{0x0626, 0x06c6, 1, 1, 0xFBF3, 0xFBF2},
	{0x0626, 0x06c8, 1, 1, 0xFBF5, 0xFBF4},
	{0x0626, 0x06d0, 1, 1, 0xFBF7, 0xFBF6},
	{0x0626, 0x06d5, 1, 1, 0xFBED, 0xFBEC},

	{0x0628, 0x062c, 0xFC9c, 1, 1, 0xFC05},
	{0x0628, 0x062d, 0xFC9d, 1, 1, 0xFC06},
	{0x0628, 0x062e, 0xFC9e, 1, 1, 0xFC07},
	{0x0628, 0x0631, 1, 1, 0xFC6a, 1},
	{0x0628, 0x0632, 1, 1, 0xFC6b, 1},
	{0x0628, 0x0645, 0xFC9f, 1, 0xFC6c, 0xFC08},
	{0x0628, 0x0646, 1, 1, 0xFC6d, 1},
	{0x0628, 0x0647, 0xFCa0, 1, 1, 1},
	{0x0628, 0x0649, 1, 1, 0xFC6e, 0xFC09},
	{0x0628, 0x064a, 1, 1, 0xFC6f, 0xFC0A},

	{0x062a, 0x062c, 0xFCa1, 1, 1, 0xFC0B},
	{0x062a, 0x062d, 0xFCa2, 1, 1, 0xFC0C},
	{0x062a, 0x062e, 0xFCa3, 1, 1, 0xFC0D},
	{0x062a, 0x0631, 1, 1, 0xFC70, 1},
	{0x062a, 0x0632, 1, 1, 0xFC71, 1},
	{0x062a, 0x0645, 0xFCa4, 1, 0xFC72, 0xFC0E},
	{0x062a, 0x0646, 1, 1, 0xFC73, 1},
	{0x062a, 0x0647, 0xFCa5, 1, 1, 1},
	{0x062a, 0x0649, 1, 1, 0xFC74, 0xFC0F},
	{0x062a, 0x064a, 1, 1, 0xFC75, 0xFC10},

	{0x062b, 0x062c, 1, 1, 1, 0xFC11},
	{0x062b, 0x0631, 1, 1, 0xFC76, 1},
	{0x062b, 0x0632, 1, 1, 0xFC77, 1},
	{0x062b, 0x0645, 1, 1, 0xFC78, 0xFC12},
	{0x062b, 0x0646, 1, 1, 0xFC79, 1},
	{0x062b, 0x0649, 1, 1, 0xFC7a, 0xFC13},
	{0x062b, 0x064a, 1, 1, 0xFC7b, 0xFC14},
	
	{0x062c, 0x062d, 1, 1, 1, 0xFC15},
	{0x062c, 0x0645, 1, 1, 1, 0xFC16},
	
	{0x062d, 0x062c, 1, 1, 1, 0xFC17},
	{0x062d, 0x0645, 1, 1, 1, 0xFC18},

	{0x062e, 0x062c, 1, 1, 1, 0xFC19},
	{0x062e, 0x062d, 1, 1, 1, 0xFC1A},
	{0x062e, 0x0645, 1, 1, 1, 0xFC1B},

	{0x0630, 0x0670, 1, 1, 1, 0xFC5b},

	{0x0631, 0x0670, 1, 1, 1, 0xFC5c},
	
	{0x0633, 0x062c, 1, 1, 1, 0xFC1C},
	{0x0633, 0x062d, 1, 1, 1, 0xFC1D},
	{0x0633, 0x062e, 1, 1, 1, 0xFC1E},
	{0x0633, 0x0645, 1, 1, 1, 0xFC1F},

	{0x0635, 0x062d, 1, 1, 1, 0xFC20},
	{0x0635, 0x0645, 1, 1, 1, 0xFC21},

	{0x0636, 0x062c, 1, 1, 1, 0xFC22},
	{0x0636, 0x062d, 1, 1, 1, 0xFC23},
	{0x0636, 0x062e, 1, 1, 1, 0xFC24},
	{0x0636, 0x0645, 1, 1, 1, 0xFC25},

	{0x0637, 0x062d, 1, 1, 1, 0xFC26},
	{0x0637, 0x0645, 1, 1, 1, 0xFC27},
	
	{0x0638, 0x0645, 1, 1, 1, 0xFC28},

	{0x0639, 0x062c, 1, 1, 1, 0xFC29},
	{0x0639, 0x0645, 1, 1, 1, 0xFC2A},

	{0x063A, 0x062c, 1, 1, 1, 0xFC2B},
	{0x063A, 0x0645, 1, 1, 1, 0xFC2C},

	{0x0641, 0x062c, 1, 1, 1, 0xFC2D},
	{0x0641, 0x062d, 1, 1, 1, 0xFC2E},
	{0x0641, 0x062e, 1, 1, 1, 0xFC2F},
	{0x0641, 0x0645, 1, 1, 1, 0xFC30},
	{0x0641, 0x0649, 1, 1, 0xFC7c, 0xFC31},
	{0x0641, 0x064a, 1, 1, 0xFC7d, 0xFC32},

	{0x0642, 0x062d, 1, 1, 1, 0xFC33},
	{0x0642, 0x0645, 1, 1, 1, 0xFC34},
	{0x0642, 0x0649, 1, 1, 0xFC7e, 0xFC35},
	{0x0642, 0x064a, 1, 1, 0xFC7f, 0xFC36},

	{0x0643, 0x0627, 1, 1, 0xFC80, 0xFC37},
	{0x0643, 0x062c, 1, 1, 1, 0xFC38},
	{0x0643, 0x062d, 1, 1, 1, 0xFC39},
	{0x0643, 0x062e, 1, 1, 1, 0xFC3a},
	{0x0643, 0x0644, 1, 1, 0xFC81, 0xFC3b},
	{0x0643, 0x0645, 1, 1, 0xFC82, 0xFC3c},
	{0x0643, 0x0649, 1, 1, 0xFC83, 0xFC3d},
	{0x0643, 0x064a, 1, 1, 0xFC84, 0xFC3e},
	
	{0x0644, 0x0622, 1, 1, 0xFEF6, 0xFEf5},
	{0x0644, 0x0623, 1, 1, 0xFEF8, 0xFEF7},
	{0x0644, 0x0625, 1, 1, 0xFEFA, 0xFEF9},
	{0x0644, 0x0627, 1, 1, 0xFEFC, 0xFEFB},
	{0x0644, 0x062c, 1, 1, 1, 0xFC3f},
	{0x0644, 0x062d, 1, 1, 1, 0xFC40},
	{0x0644, 0x062e, 1, 1, 1, 0xFC41},
	{0x0644, 0x0645, 1, 1, 0xFC85, 0xFC42},
	{0x0644, 0x0649, 1, 1, 0xFC86, 0xFC43},
	{0x0644, 0x064a, 1, 1, 0xFC87, 0xFC44},
	
	{0x0645, 0x0627, 1, 1, 0xFC88, 1},
	{0x0645, 0x062c, 1, 1, 1, 0xFC45},
	{0x0645, 0x062d, 1, 1, 1, 0xFC46},
	{0x0645, 0x062e, 1, 1, 1, 0xFC47},
	{0x0645, 0x0645, 1, 1, 0xFC89, 0xFC48},
	{0x0645, 0x0649, 1, 1, 1, 0xFC49},
	{0x0645, 0x064a, 1, 1, 1, 0xFC4a},

	{0x0646, 0x062c, 1, 1, 1, 0xFC4b},
	{0x0646, 0x062d, 1, 1, 1, 0xFC4c},
	{0x0646, 0x062e, 1, 1, 1, 0xFC4d},
	{0x0646, 0x0631, 1, 1, 0xFC8a, 1},
	{0x0646, 0x0632, 1, 1, 0xFC8b, 1},
	{0x0646, 0x0645, 1, 1, 0xFC8c, 0xFC4e},
	{0x0646, 0x0646, 1, 1, 0xFC8d, 1},
	{0x0646, 0x0649, 1, 1, 0xFC8e, 0xFC4f},
	{0x0646, 0x064a, 1, 1, 0xFC8f, 0xFC50},

	{0x0647, 0x062c, 1, 1, 1, 0xFC51},
	{0x0647, 0x0645, 1, 1, 1, 0xFC52},
	{0x0647, 0x0649, 1, 1, 1, 0xFC53},
	{0x0647, 0x064a, 1, 1, 1, 0xFC54},

	{0x0649, 0x0670, 1, 1, 0xFC90, 0xFC5d},
	
	{0x064a, 0x062c, 1, 1, 1, 0xFC55},
	{0x064a, 0x062d, 1, 1, 1, 0xFC56},
	{0x064a, 0x062e, 1, 1, 1, 0xFC57},
	{0x064a, 0x0631, 1, 1, 0xFC91, 1},
	{0x064a, 0x0632, 1, 1, 0xFC92, 1},
	{0x064a, 0x0645, 1, 1, 0xFC93, 0xFC58},
	{0x064a, 0x0646, 1, 1, 0xFC94, 1},
	{0x064a, 0x0649, 1, 1, 0xFC95, 0xFC59},
	{0x064a, 0x064a, 1, 1, 0xFC96, 0xFC5a},

	// Hebrew
	{0xFB2A, 0x05BC, 0xFB2C, 0xFB2C, 0xFB2C, 0xFB2C},
	{0xFB2B, 0x05BC, 0xFB2D, 0xFB2D, 0xFB2D, 0xFB2D},
	{0xFB49, 0x05C1, 0xFB2C, 0xFB2C, 0xFB2C, 0xFB2C},
	{0xFB49, 0x05C2, 0xFB2D, 0xFB2D, 0xFB2D, 0xFB2D},

};

// This table contains reverse ligature data for identifying the first
// part of a ligature from the second part; it is generated
// automatically when the class is initialised.
static LigatureData s_lig_rev[NrElements(s_ligature)];

static UT_Vector s_noLigature;
static UT_Vector s_noLigature2;

void UT_contextGlyph::_generateNoLigatureTable()
{
	// handle the first entry separately ...
	UCSRange * pRange = new UCSRange;
	UT_return_if_fail(pRange);
	pRange->low = 0;
	pRange->high = s_ligature[0].code_low - 1;

	s_noLigature.addItem((void*)pRange);
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
		if(s_ligature[i].code_low != iLow)
		{
			pRange = new UCSRange;
			UT_return_if_fail(pRange);
		
			pRange->low = iLow + 1;
			iLow = s_ligature[i].code_low;
			pRange->high = iLow - 1;
			
			s_noLigature.addItem((void*)pRange);
			xxx_UT_DEBUGMSG(("No Ligature table: adding {0x%04x, 0x%04x}\n",
						 pRange->low, pRange->high));
		}

		if(s_ligature[i+1].code_low == s_ligature[i].code_low + 1)
		{
			iLow++;
		}
		
	}

	// now handle the last element
	pRange = new UCSRange;
	UT_return_if_fail(pRange);
	pRange->low = iLow + 1;
	pRange->high = 0xffffffff;

	s_noLigature.addItem((void*)pRange);
	xxx_UT_DEBUGMSG(("No Ligature table: adding {0x%04x, 0x%04x}\n",
				 pRange->low, pRange->high));
	
}

void UT_contextGlyph::_generateNoLigature2Table()
{
	// handle the first entry separately ...
	UCSRange * pRange = new UCSRange;
	UT_return_if_fail(pRange);
	pRange->low = 0;
	pRange->high = s_lig_rev[0].code_high - 1;

	s_noLigature2.addItem((void*)pRange);
	xxx_UT_DEBUGMSG(("No Ligature2 table: adding {0x%04x, 0x%04x}\n",
				 pRange->low, pRange->high));
	
	UT_uint32 iHigh = s_lig_rev[0].code_high;
	if(NrElements(s_lig_rev) &&
	   s_lig_rev[0].code_high + 1 == s_lig_rev[1].code_high)
	{
		iHigh++;
	}
	
	for(UT_uint32 i = 1; i < NrElements(s_lig_rev) - 1; i++)
	{
		if(s_lig_rev[i].code_high != iHigh)
		{
			pRange = new UCSRange;
			UT_return_if_fail(pRange);
		
			pRange->low = iHigh + 1;
			iHigh = s_lig_rev[i].code_high;
			pRange->high = iHigh - 1;
			
			s_noLigature2.addItem((void*)pRange);
			xxx_UT_DEBUGMSG(("No Ligature2 table: adding {0x%04x, 0x%04x}\n",
						 pRange->low, pRange->high));
		}
		
		if(s_lig_rev[i].code_high + 1 == s_lig_rev[i+1].code_high)
		{
			iHigh++;
		}
	}

	// now handle the last element
	pRange = new UCSRange;
	UT_return_if_fail(pRange);
	pRange->low = iHigh + 1;
	pRange->high = 0xffffffff;

	s_noLigature2.addItem((void*)pRange);
	xxx_UT_DEBUGMSG(("No Ligature2 table: adding {0x%04x, 0x%04x}\n",
				 pRange->low, pRange->high));
	
}


// The following table contains letters that have context-sensitive
// forms
// when adding entries to this table, make sure that you update
// the HEBREW_START and HEBREW_END macros that follow if necessary
static LetterData s_table[] =
{
	// code, intial, medial, final, stand-alone

	// Greek
	{0x03C2, 0x03C3, 0x03C3, 0x03C2, 0x03C2},
	{0x03C3, 0x03C3, 0x03C3, 0x03C2, 0x03C2},

	// Hebrew letters
	// the following macro defines how many entries in this table
	// precede the Hebrew section
#define HEBREW_START 2
	{0x05DA, 0x05DB, 0x05DB, 0x05DA, 0x05DA},
	{0x05DB, 0x05DB, 0x05DB, 0x05DA, 0x05DA},

	{0x05DD, 0x05DE, 0x05DE, 0x05DD, 0x05DD},
	{0x05DE, 0x05DE, 0x05DE, 0x05DD, 0x05DD},
	{0x05DF, 0x05E0, 0x05E0, 0x05DF, 0x05DF},
	{0x05E0, 0x05E0, 0x05E0, 0x05DF, 0x05DF},

	{0x05E3, 0x05E4, 0x05E4, 0x05E3, 0x05E3},
	{0x05E4, 0x05E4, 0x05E4, 0x05E3, 0x05E3},
	{0x05E5, 0x05E6, 0x05E6, 0x05E5, 0x05E5},
	{0x05E6, 0x05E6, 0x05E6, 0x05E5, 0x05E5},
	// the following macro defines the index of the last entry in
	// the Hebrew section of the table
#define HEBREW_END 11
	
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

	/* the following characters are not found in Arabic Presentation forms B
		and so we will leave it for now -- most of these are ligatures

	{0x0660, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x0661, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x0662, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x0663, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x0664, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x0665, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x0666, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x0667, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x0668, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x0669, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x066a, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x066b, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x066c, 0xFE, 0xFE, 0xFE, 0xFE},
	{0x066d, 0xFE, 0xFE, 0xFE, 0xFE}, */
	{0x0671, 0x0671, 0x0671, 0xFB51, 0xFB50}, /*code, intial, medial, final, stand-alone*/
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
// 	0x06d4 -- full stop, no shaping
// 	{0x06d5, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06e5, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06e6, 0xFE, 0xFE, 0xFE, 0xFE},
// 	0x06f0 - 0x06f9 -- alternative digits, no shaping 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06fa, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06fb, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06fc, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06fd, 0xFE, 0xFE, 0xFE, 0xFE},
// 	{0x06fe, 0xFE, 0xFE, 0xFE, 0xFE},
};

static UT_Vector s_noShaping;
void UT_contextGlyph::_generateNoShapingTable()
{
	// handle the first entry separately ...
	UCSRange * pRange = new UCSRange;
	UT_return_if_fail(pRange);
	pRange->low = 0;
	pRange->high = s_table[0].code - 1;

	s_noShaping.addItem((void*)pRange);
	xxx_UT_DEBUGMSG(("No Shaping table: adding {0x%04x, 0x%04x}\n",
				 pRange->low, pRange->high));
	
	UT_uint32 iPrev = s_table[0].code;
	if((s_iGlyphTableSize/sizeof(LetterData) > 1) &&
	   s_table[0].code + 1 == s_table[1].code)
	{
		iPrev++;
	}
	
	for(UT_uint32 i = 1; i < s_iGlyphTableSize/sizeof(LetterData) - 1; i++)
	{
		if(s_table[i].code != iPrev)
		{
			pRange = new UCSRange;
			UT_return_if_fail(pRange);
	
			pRange->low = iPrev + 1;
			iPrev = s_table[i].code;
			pRange->high = iPrev - 1;
			
			s_noShaping.addItem((void*)pRange);
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

	s_noShaping.addItem((void*)pRange);
	xxx_UT_DEBUGMSG(("No Shaping table: adding {0x%04x, 0x%04x}\n",
				 pRange->low, pRange->high));
}

// This table contains Unicode ranges of characters that do not join
// with the following character (excluding word delimters)
static UCSRange s_nonjoining_with_next[] =
{
	// Arabic
	{0x0622, 0x0625},
	{0x0627, 0x0627},
	{0x062f, 0x0632},
	{0x0648, 0x0648}
};

// This table contains Unicode ranges of characters that do not join
// with the previous character (excluding word delimiters)
static UCSRange s_nonjoining_with_prev[] =
{
	// Arabic
	{0x0621, 0x0621}
};

// This table contains Unicode ranges of characters that should be
// ignored (i.e., skipped over) by the shaping engine. Typically,
// these are combining characters
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
	{0x2018,0x201F}
};

// following three functions determine if shaping and ligature
// processing is required for a given character

/*!
    returns true if character c does not have context-sensitive variants
*/
bool UT_contextGlyph::isNotContextSensitive(UT_UCS4Char c) const
{
	// I am not entirely sure whether this is really worth it, since
    // the last time I checked the glyph table had 88 entries, and the
    // no shaping table 30; however, most characters will exit in
	// first three cycles, and the glyph table is expanding.
	
	for(UT_uint32 i = 0; i < s_noShaping.getItemCount(); i++)
	{
		if(c >= static_cast<UCSRange*>(s_noShaping.getNthItem(i))->low &&
		   c <= static_cast<UCSRange*>(s_noShaping.getNthItem(i))->high)
		{
			return true;
		}
		
		if(c < static_cast<UCSRange*>(s_noShaping.getNthItem(i))->low)
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
bool UT_contextGlyph::isNotFirstInLigature(UT_UCS4Char c) const
{
	// this is worth it, since the ligature table is quite large and
	// the s_noLigature table is not only small, but most characters
	// will exit in the first three cycles of the loop
	for(UT_uint32 i = 0; i < s_noLigature.getItemCount(); i++)
	{
		if(c >= static_cast<UCSRange*>(s_noLigature.getNthItem(i))->low &&
		   c <= static_cast<UCSRange*>(s_noLigature.getNthItem(i))->high)
		{
			return true;
		}

		if(c < static_cast<UCSRange*>(s_noLigature.getNthItem(i))->low)
		{
			// we have already overshot ...
			return false;
		}
	}
	return false;
}

/*!
    returns true if character c never forms a second part of a ligature
*/
bool UT_contextGlyph::isNotSecondInLigature(UT_UCS4Char c) const
{
	// this too is worth it, since the ligature table is quite large and
	// the s_noLigature2 table is not only very small, but most characters
	// will exit in the first three cycles of the loop
	for(UT_uint32 i = 0; i < s_noLigature2.getItemCount(); i++)
	{
		if(c >= static_cast<UCSRange*>(s_noLigature2.getNthItem(i))->low &&
		   c <= static_cast<UCSRange*>(s_noLigature2.getNthItem(i))->high)
		{
			return true;
		}

		if(c < static_cast<UCSRange*>(s_noLigature2.getNthItem(i))->low)
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
static bool s_notJoiningWithNext(UT_UCS4Char c, UT_UCS4Char f, UT_UCS4Char p)
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
static bool s_notJoiningWithPrev(UT_UCS4Char c, UT_UCS4Char f, UT_UCS4Char p)
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

// The following function is used for binary search of the reversed ligature table.
static int s_comp_lig2(const void *a, const void *b)
{
	const LigatureSequence * A = static_cast<const LigatureSequence*>(a);
	const LigatureData     * B = static_cast<const LigatureData*>(b);

	int ret = static_cast<int>(A->code) - static_cast<int>(B->code_high);
	if(!ret)
		ret = static_cast<int>(A->next) - static_cast<int>(B->code_low);

	return ret;
}


// The following function is used for quick sorting the reversed
// ligature table
static int s_comp_qlig(const void *a, const void *b)
{
	const LigatureData *A = static_cast<const LigatureData*>(a);
	const LigatureData *B = static_cast<const LigatureData*>(b);

	int ret = static_cast<int>(A->code_high) - static_cast<int>(B->code_high);
	if(!ret)
		return static_cast<int>(A->code_low) - static_cast<int>(B->code_low);

	return ret;
}


/*
   SOFT SMART QUOTES HANDLING

   This is how it works: each type of smart quote is described by a
   LetterData structure in which the first two fields are unused;
   these are named after one of the languages that uses them and the
   character, e.g. s_sq__single_en_us.

   The main table is made up of SmartQuote structures, it contains all
   languages that behave differently than en_US; if the language does
   not want smart quotes, the pGlyphs pointer is set to NULL,
   otherwise it point to one of the s_sg_*_* definitions; please check
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
	FriBidiChar fbc = static_cast<FriBidiChar>(c), mfbc;

	if (fribidi_get_mirror_char (/* Input */ fbc, /* Output */&mfbc))
		return static_cast<UT_UCSChar>(mfbc);
	else
		return c;
}

// Initialisation of static members
bool             UT_contextGlyph::s_bInit           = false;
UT_uint32        UT_contextGlyph::s_iGlyphTableSize = sizeof(s_table);
bool             UT_contextGlyph::s_bSmartQuotes    = true;
const XML_Char * UT_contextGlyph::s_pEN_US          = NULL;

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UT_contextGlyph::UT_contextGlyph(bool bNoInit)
{
	if(!bNoInit)
	{
		UT_contextGlyph();
	}
}

UT_contextGlyph::UT_contextGlyph()
{
	if(!s_bInit)
	{
		bool bHebrewContextGlyphs = false;
		XAP_App::getApp()->getPrefsValueBool(static_cast<XML_Char*>(XAP_PREF_KEY_UseHebrewContextGlyphs),
											 &bHebrewContextGlyphs);

		// if the user does not want to use glyphshaping for the final
		// forms of Hebrew characters, we will shift the data in our
		// glyph tables
		if(!bHebrewContextGlyphs)
		{
			s_iGlyphTableSize -= (HEBREW_END - HEBREW_START + 1) * sizeof(LetterData);
			memmove(&s_table[HEBREW_START], &s_table[HEBREW_END + 1],s_iGlyphTableSize - HEBREW_START);
		}

		// now copy the data from the ligature table into the reversed
		// ligature table and sort it
		memcpy(s_lig_rev,s_ligature, sizeof(s_ligature));
		qsort(s_lig_rev,NrElements(s_lig_rev), sizeof(LigatureData),s_comp_qlig);

		// generate the no-shaping and no-ligature tables
		_generateNoLigatureTable();
		_generateNoLigature2Table();
		_generateNoShapingTable();

		UT_DEBUGMSG(("UT_ContextGlyph: glyphs: %d, ligatures: %d, no-lig: %d, no-lig2: %d, "
					 "no-shap: %d\n",
					 s_iGlyphTableSize/sizeof(LetterData), NrElements(s_ligature),
					 s_noLigature.getItemCount(), s_noLigature2.getItemCount(),
					 s_noShaping.getItemCount()));
		
		// init the smart quote tables with the poiters to language
		// codes in UT_Laguage
		UT_Language lang;
		s_pEN_US = lang.getPropertyFromProperty("en-US");
		for(UT_uint32 i = 0; i < NrElements(s_smart_quotes); i++)
		{
			s_smart_quotes[i].pLang = lang.getPropertyFromProperty(s_smart_quotes[i].pLang);
		}

		XAP_App::getApp()->getPrefsValueBool(static_cast<XML_Char*>(XAP_PREF_KEY_SmartQuotesEnable),
											 &s_bSmartQuotes);


		// add a listener to the preferences, so that we know when the
		// user changed his/her mind
		XAP_App::getApp()->getPrefs()->addListener(_prefsListener,NULL);
		s_bInit = true;
	}
}

// can be used to deallocate the static tables
void UT_contextGlyph::static_destructor()
{
	UT_DEBUGMSG(("UT_contextGlyph::static_destructor() called\n"));

	if(s_bInit)
	{
		s_bInit = false;
		UT_VECTOR_PURGEALL(UCSRange*,s_noLigature);  s_noLigature.clear();
		UT_VECTOR_PURGEALL(UCSRange*,s_noLigature2); s_noLigature2.clear();
		UT_VECTOR_PURGEALL(UCSRange*,s_noShaping);   s_noShaping.clear();
	}
}

void UT_contextGlyph::_prefsListener(	XAP_App *pApp, XAP_Prefs *, UT_StringPtrMap *, void *)
{
	UT_return_if_fail(pApp);
	pApp->getPrefsValueBool(static_cast<XML_Char*>(XAP_PREF_KEY_SmartQuotesEnable), &s_bSmartQuotes);
}

/*!
    Finds the description for the smart quote for given language and character
    IMPORTANT: pLang must be a pointer directly into the static table of
    UT_Language, not just an arbitrary pointer !!!
*/
const LetterData * UT_contextGlyph::smartQuote(UT_UCS4Char c, const XML_Char * pLang) const
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


// This function evaluates the context of a glyph deciding what form
// the glyph should take; it is inlined for performance reasons
inline GlyphContext UT_contextGlyph::_evalGlyphContext(const UT_UCSChar* code, const UT_UCSChar * prev, const UT_UCSChar * next) const
{
	xxx_UT_DEBUGMSG(("UT_contextGlyph::_eval: 0x%x, prev 0x%x [*0x%x], next 0x%x [*0x%x]\n",
					 *code, prev, *prev, next, *next ));

	if((!next || !*next) && (!prev || !*prev))
		return GC_ISOLATE;

	bool bPrevWD;
	bool bNextWD;

	if((!next || !*next) && prev && *prev)
	{
		bPrevWD = s_notJoiningWithNext(*prev, *code, *(prev+1));
		
		if(bPrevWD)
			return GC_ISOLATE;
		else
			return GC_FINAL;
	}

	// no-next has been trapped above, now we can
	//check if next is not a character that is to be ignored
	const UT_UCSChar *myNext = next;

	while(   *myNext
		  && bsearch(static_cast<const void*>(myNext),
					 static_cast<void*>(s_ignore),
					 NrElements(s_ignore),
					 sizeof(UCSRange),
					 s_comp_ignore))
	{
		myNext++;
	}

	const UT_UCSChar myNextNext = (*myNext && *(myNext+1)) ?  *(myNext+1) : UCS_SPACE;

	xxx_UT_DEBUGMSG(("UT_contextGlyph::_eval: myNext 0x%x\n", *myNext));

	if((!prev || !*prev) && !*myNext)
		return GC_ISOLATE;

	if((!prev || !*prev) && *myNext)
	{
		bNextWD = s_notJoiningWithPrev(*myNext, myNextNext, UCS_UNKPUNK);
		if(bNextWD)
			return GC_ISOLATE;
		else
			return GC_INITIAL;
	}

	if(*prev && !*myNext)
	{
		bPrevWD = s_notJoiningWithNext(*prev, *code, *(prev+1));
		if(bPrevWD)
			return GC_ISOLATE;
		else
			return GC_FINAL;

	}
	xxx_UT_DEBUGMSG(("UT_contextGlyph::_eval: 0x%x, prev 0x%x, myNext 0x%x, myNextNext 0x%x\n",
					 *code,*prev, *myNext,myNextNext));
	
	bPrevWD = s_notJoiningWithNext(*prev, *code, *(prev+1));
	bNextWD = s_notJoiningWithPrev(*myNext, myNextNext,UCS_UNKPUNK);

	// if both are not , then medial form is needed
	if(!bPrevWD && !bNextWD)
		return GC_MEDIAL;

	// if only *next is, than final form is needed
	if(bNextWD)
		return GC_FINAL;

	// if *prev is, the initial form is needed
	if(bPrevWD)
		return GC_INITIAL;

	// if we got here, both are delimiters, which means stand alone form is needed
	return GC_ISOLATE;

}
/*
	code - pointer to the character to interpret
	prev - pointer to the character before code
	next - NULL-terminated string of characters that follow

	returns the glyph to be drawn
*/
UT_UCSChar UT_contextGlyph::getGlyph(const UT_UCSChar * code,
									 const UT_UCSChar * prev,
									 const UT_UCSChar * next,
									 const XML_Char   * pLang) const
{

	UT_ASSERT(code);

	const LetterData   * pLet = 0;
	const LigatureData * pLig = 0;
	LigatureSequence     Lig;
	bool                 bIsSecond = false;
	GlyphContext         context = GC_NOT_SET;

	// first, deal with smart quotes
	if(s_bSmartQuotes)
	{
		pLet = smartQuote(*code,pLang);
	}

	if(!pLet)
	{
		// decide if this is a part of a ligature
		// check for a ligature form

		if(!isNotFirstInLigature(*code))
		{
			
			Lig.next = next ? *next : 0;
			Lig.code = *code;

			pLig = static_cast<LigatureData*>(bsearch(static_cast<void*>(&Lig),
													  static_cast<void*>(s_ligature),
													  NrElements(s_ligature),
													  sizeof(LigatureData),
													  s_comp_lig));
		}
		
		if(pLig)
		{
			next++;
			xxx_UT_DEBUGMSG(("UT_contextGlyph::getGlyph: 0x%x, 1st of lig.\n", *code));
		}
		else
		{
			if(!isNotSecondInLigature(*code))
			{
				
				Lig.next = prev ? *prev : 0;
				pLig = static_cast<LigatureData*>(bsearch(static_cast<void*>(&Lig),
														  static_cast<void*>(s_lig_rev),
														  NrElements(s_lig_rev),
														  sizeof(LigatureData),
														  s_comp_lig2));
			}
			
			if(pLig)
			{
				xxx_UT_DEBUGMSG(("UT_contextGlyph::getGlyph: 0x%x, 2nd of lig.\n", *code));
				bIsSecond = true;
			}
		}

		// if this is a ligature, handle it
		if(pLig)
		{
			context = bIsSecond ? _evalGlyphContext(prev, prev+1, next)
				: _evalGlyphContext(code, prev, next);
			UT_UCSChar glyph = 0;
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

			UT_DEBUGMSG(("UT_contextGlyph::getGlyph: lig. (%d), glyph 0x%x\n",bIsSecond,glyph));
			
			if(!bIsSecond && glyph != 1) //first part of a ligature
				return glyph;
			
			if(bIsSecond && glyph != 1)
			{
				// a special ligature glyph was used, map this to a 0-width non breaking space
				return 0xFEFF;
			}

			// if we got here, the glyph was 1, which means this form is to be just
			// treated as an ordinary letter, also if this was a first part of the ligature
			// we already know its context, but not if it was a second part of lig.
		}


		// if we have no pL we are dealing with an ordinary letter
		if(!isNotContextSensitive(*code))
		{
			
			pLet = static_cast<LetterData*>(bsearch(static_cast<const void*>(code),
													static_cast<void*>(s_table),
													s_iGlyphTableSize/sizeof(LetterData),
													sizeof(LetterData),
													s_comp));
		}
		
		// if we have no pLet, it means the letter has only one form
		// so we return it back
		if(!pLet)
			return *code;
	} // was not a smart quote
	
	if(context == GC_NOT_SET || bIsSecond)
		context = _evalGlyphContext(code, prev, next);

	switch (context)
	{
		case GC_INITIAL:
			return pLet->initial;
		case GC_MEDIAL:
			return pLet->medial;
		case GC_FINAL:
			return pLet->final;
		case GC_ISOLATE:
			return pLet->alone;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	return 0;
}


// Similar to getGlyph, except it processes an entire array of
// characters
// NOTE: it is significantly more efficient to use this function that
// to call getGlyph in a look
void UT_contextGlyph::renderString(const UT_UCSChar * src,
								   UT_UCSChar *dest,
								   UT_uint32 len,
								   const UT_UCSChar * prev,
								   const UT_UCSChar * next,
								   const XML_Char   * pLang,
							       FriBidiCharType    iDirection) const
{
	UT_ASSERT(src);
	UT_ASSERT(dest);

	const UT_UCSChar * src_ptr = src;
	const UT_UCSChar * next_ptr;
	const UT_UCSChar * prev_ptr = 0;	// initialize to prevent compiler warning
	UT_UCSChar       * dst_ptr = dest;
	UT_UCSChar         prev_tmp[2] = {0,0};
	UT_UCSChar         next_tmp[CONTEXT_BUFF_SIZE + 1] = {0,0,0,0,0,0};
	UT_UCSChar         glyph = 0;

	for(UT_uint32 i = 0; i < len; i++, src_ptr++)
	{
		LigatureSequence     Lig;
		const LetterData   * pLet = 0;
		const LigatureData * pLig = 0;
		bool                 bIsSecond = false;
		GlyphContext         context = GC_NOT_SET;
		
		//get the current context
		if(len > CONTEXT_BUFF_SIZE && i < len - CONTEXT_BUFF_SIZE)
			next_ptr = src_ptr + 1;
		else
		{
			next_ptr = next_tmp;
			UT_uint32 j;
			for(j = 0; j < len - i - 1 && j < CONTEXT_BUFF_SIZE; j++)
				next_tmp[j] = *(src_ptr + 1 + j);
			for(; j < CONTEXT_BUFF_SIZE; j++)
				next_tmp[j] = *(next + (j + i + 1 - len));
		}

		if(i == 0)
			prev_ptr = prev;
		else if(i == 1)
		{
			prev_tmp[0] = *src;
			prev_tmp[1] = *prev;
			prev_ptr = prev_tmp;
		}
		else
		{
			prev_tmp[0] = *(src_ptr - 1);
			prev_tmp[1] = *(src_ptr - 2);
			//no need to set prev_ptr, since this has been done when i == 1
		}

		// first, deal with smart quotes
		if(s_bSmartQuotes)
		{
			pLet = smartQuote(*src_ptr,pLang);
		}

		// if this is a smart quote, than we are of the hook
		if(!pLet)
		{
			// decide if this is a part of a ligature
			// check for a ligature form
			if(!isNotFirstInLigature(*src_ptr))
			{
				Lig.next = next_ptr ? *next_ptr : 0;
				Lig.code = *src_ptr;

				pLig = static_cast<LigatureData*>(bsearch(static_cast<void*>(&Lig),
														  static_cast<void*>(s_ligature),
														  NrElements(s_ligature),
														  sizeof(LigatureData),
														  s_comp_lig));
			}
			
			if(pLig)
			{
				// we need the context of the whole pair not just of this character
				next_ptr++;
				xxx_UT_DEBUGMSG(("UT_contextGlyph::render: 0x%x, 1st of lig.\n", *code));
			}
			else
			{
				//we only check that this is not a second part of a ligature for the
				//first character, the rest we will handle in the previous
				//cycle of the loop
				if(i == 0 && !isNotSecondInLigature(*src_ptr))
				{
					Lig.next = prev ? *prev : 0;
					pLig = static_cast<LigatureData*>(bsearch(static_cast<void*>(&Lig),
															  static_cast<void*>(s_lig_rev),
															  NrElements(s_lig_rev),
															  sizeof(LigatureData),
															  s_comp_lig2));
					
					if(pLig)
					{
						xxx_UT_DEBUGMSG(("UT_contextGlyph::render: 0x%x, 2nd of lig.\n",*code));
						bIsSecond = true;
					}
				}
			}
			
			// if this is a ligature, handle it
			if(pLig)
			{
				context = bIsSecond ? _evalGlyphContext(prev, prev+1, next_ptr)
					: _evalGlyphContext(src_ptr, prev_ptr, next_ptr);
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

				UT_DEBUGMSG(("UT_contextGlyph::render: lig.(%d), glyph 0x%x\n",bIsSecond,glyph));
				if(!bIsSecond && glyph != 1) //first part of a ligature
				{
					// we set both this and the next char if we can
					*dst_ptr++ = glyph;
					if(i < len - 1)
					{
						*dst_ptr++ = 0xFEFF;
						src_ptr++;
						i++;
						continue;
					}
					continue;
				}
				else if(bIsSecond && glyph != 1)
				{
					// a special ligature glyph was used, map this to a 0-width non breaking space
					*dst_ptr++ = 0xFEFF;
					continue;
				}

				// if we got here, the glyph was 1, which means this form is to be just
				// treated as an ordinary letter, also if this was a first part of the ligature
				// we already know its context, but not if it was a second part of lig.
			}
		
			// if we have no pLig we are dealing with an ordinary
			// letter
			if(!isNotContextSensitive(*src_ptr))
			{
				
				pLet = static_cast<LetterData*>(bsearch(static_cast<const void*>(src_ptr),
														static_cast<void*>(s_table),
														s_iGlyphTableSize/sizeof(LetterData),
														sizeof(LetterData),
														s_comp));
			}
			
			// if we have no pLet, it means the letter has only one form
			// last thing to do is to deal with mirror characters
			if(!pLet)
			{
				if(iDirection == FRIBIDI_TYPE_RTL)
					*dst_ptr++ = s_getMirrorChar(*src_ptr);
				else
					*dst_ptr++ = *src_ptr;
				continue;
			}
		} // was not a smart quote
		

		// if we got this far, we are dealing with a context sensitive character
		if(context == GC_NOT_SET || bIsSecond)
			context = _evalGlyphContext(src_ptr, prev_ptr, next_ptr);

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

		*dst_ptr++ = glyph;
	}
}
