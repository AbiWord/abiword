/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * BIDI Copyright (c) 2001,2002 Tomas Frydrych, Yaacov Akiba Slama
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

#include "fl_AutoLists.h"

//////////////////////////////////////////////////////////////////////
// Two Useful List arrays
/////////////////////////////////////////////////////////////////////

static const gchar * xml_Lists[] = { XML_NUMBERED_LIST,
				     XML_LOWERCASE_LIST,
				     XML_UPPERCASE_LIST,
				     XML_LOWERROMAN_LIST,
				     XML_UPPERROMAN_LIST,
				     XML_BULLETED_LIST,
				     XML_DASHED_LIST,
				     XML_SQUARE_LIST,
				     XML_TRIANGLE_LIST,
				     XML_DIAMOND_LIST,
				     XML_STAR_LIST,
				     XML_IMPLIES_LIST,
				     XML_TICK_LIST,
				     XML_BOX_LIST,
				     XML_HAND_LIST,
				     XML_HEART_LIST,
				     XML_ARROWHEAD_LIST,
				     XML_ARABICNUM_LIST,
				     XML_HEBREW_LIST,
};

const gchar * fl_AutoLists::getXmlList(UT_uint32 i)
{
    UT_uint32 j = i;
	if(i > OTHER_NUMBERED_LISTS)
		j -= (OTHER_NUMBERED_LISTS - LAST_BULLETED_LIST);
	return xml_Lists[j];
}

UT_uint32 fl_AutoLists::getXmlListsSize()
{
	return sizeof(xml_Lists)/sizeof(xml_Lists[0]);
}

static const char	  * fmt_Lists[] = { fmt_NUMBERED_LIST,
										fmt_LOWERCASE_LIST,
										fmt_UPPERCASE_LIST,
										fmt_UPPERROMAN_LIST,
										fmt_LOWERROMAN_LIST,
										fmt_BULLETED_LIST,
										fmt_DASHED_LIST,
										fmt_ARABICNUM_LIST,
										fmt_HEBREW_LIST
};

const char * fl_AutoLists::getFmtList(UT_uint32 i)
{
    UT_uint32 j = i;
	if(i > OTHER_NUMBERED_LISTS)
		j -= (OTHER_NUMBERED_LISTS - DASHED_LIST);
	return fmt_Lists[i];
}


UT_uint32 fl_AutoLists::getFmtListsSize()
{
	return sizeof(fmt_Lists)/sizeof(fmt_Lists[0]);
}
