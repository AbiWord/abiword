/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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
#include "ut_types.h"
#include "ut_xml.h"
#ifndef FL_AUTOLISTS_H
#define FL_AUTOLISTS_H

///////////////////////////////////////////////////////////////////////
// Lists Definitions
///////////////////////////////////////////////////////////////////////
typedef enum 
{
	NUMBERED_LIST = 0,
	LOWERCASE_LIST = 1,
	UPPERCASE_LIST = 2,
	LOWERROMAN_LIST = 3,
	UPPERROMAN_LIST = 4,
	// any new numbered lists should be added below OTHER_NUMBERED_LISTS
	BULLETED_LIST = 5,
	DASHED_LIST = 6,
	SQUARE_LIST = 7,
	TRIANGLE_LIST = 8,
	DIAMOND_LIST = 9,
	STAR_LIST = 10,
	IMPLIES_LIST = 11,
	TICK_LIST = 12,
	BOX_LIST = 13,
	HAND_LIST = 14,
	HEART_LIST = 15,
	// add new bulleted lists here, and increase LAST_BULLETED_LIST accordingly
	// any new numbered lists should be added below OTHER_NUMBERED_LISTS

	//could not just add the extra numbered lists above the bulletted one, since that would break compatibility
	LAST_BULLETED_LIST = 16,
	OTHER_NUMBERED_LISTS = 0x7f,
#ifdef BIDI_ENABLED
	ARABICNUMBERED_LIST = 0x80,
	HEBREW_LIST = 0x81,
#endif
	NOT_A_LIST = 0xff
} List_Type;


class fl_AutoLists
{
	public:
		UT_uint32 getXmlListsSize();
		UT_uint32 getFmtListsSize();
		
		const XML_Char * getXmlList(UT_uint32 i);
		const char *     getFmtList(UT_uint32 i);
};

#define IS_NUMBERED_LIST_TYPE(x) (((x) >= NUMBERED_LIST && (x) < BULLETED_LIST) || ((x) > OTHER_NUMBERED_LISTS && (x) < NOT_A_LIST))
#define IS_BULLETED_LIST_TYPE(x) ((x) >= BULLETED_LIST && (x) < OTHER_NUMBERED_LISTS)

#define IS_NONE_LIST_TYPE(x) ((x) == NOT_A_LIST)

#define  XML_NUMBERED_LIST (( const XML_Char *) "Numbered List")
#define  XML_LOWERCASE_LIST ((const XML_Char *) "Lower Case List")
#define  XML_UPPERCASE_LIST ((const XML_Char *) "Upper Case List")
#define  XML_LOWERROMAN_LIST ((const XML_Char *) "Lower Roman List")
#define  XML_UPPERROMAN_LIST ((const XML_Char *) "Upper Roman List")
#define  XML_HEBREW_LIST ((const XML_Char *) "Hebrew List")
#define  XML_ARABICNUM_LIST ((const XML_Char *) "Arabic List")
#define  XML_BULLETED_LIST ((const XML_Char *) "Bullet List")
#define  XML_DASHED_LIST ((const XML_Char *) "Dashed List")
#define  XML_SQUARE_LIST ((const XML_Char *) "Square List")
#define  XML_TRIANGLE_LIST ((const XML_Char *) "Triangle List")
#define  XML_DIAMOND_LIST ((const XML_Char *) "Diamond List")
#define  XML_STAR_LIST ((const XML_Char *) "Star List")
#define  XML_IMPLIES_LIST ((const XML_Char *) "Implies List")
#define  XML_TICK_LIST ((const XML_Char *) "Tick List")
#define  XML_BOX_LIST ((const XML_Char *) "Box List")
#define  XML_HAND_LIST ((const XML_Char *) "Hand List")
#define  XML_HEART_LIST ((const XML_Char *) "Heart List")

//
// MS Word uses 0.5 inches so we do too.
#define  LIST_DEFAULT_INDENT 0.50

// May need to tweak this some more...

#define  LIST_DEFAULT_INDENT_LABEL 0.30

//
// Reserved the first 10000 id's for useful purposes....
//
#define  AUTO_LIST_RESERVED 1000

#define fmt_NUMBERED_LIST ((const char *)"%*%d")
#define  fmt_LOWERCASE_LIST ((const char *)"%*%a")
#define  fmt_UPPERCASE_LIST ((const char *)"%*%A")
#define  fmt_LOWERROMAN_LIST  ((const char *)"%*%r")
#define  fmt_UPPERROMAN_LIST  ((const char *)"%*%R")
#define  fmt_BULLETED_LIST ((const char *)"%b")
#define  fmt_DASHED_LIST ((const char *)"%c")
#define  fmt_HEBREW_LIST ((const char *)"%*%h")
#define  fmt_ARABICNUM_LIST ((const char *)"%*%i")


#endif
