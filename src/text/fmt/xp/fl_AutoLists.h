
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
#ifndef FL_AUTOLISTS_H
#define FL_AUTOLISTS_H

///////////////////////////////////////////////////////////////////////
// Lists Definitions
///////////////////////////////////////////////////////////////////////
typedef enum 
{
        NUMBERED_LIST,
	LOWERCASE_LIST,
	UPPERCASE_LIST,
	UPPERROMAN_LIST,
	LOWERROMAN_LIST,
	BULLETED_LIST,
	DASHED_LIST,
	NOT_A_LIST
}       List_Type;

#define  XML_NUMBERED_LIST (( const XML_Char *) "Numbered List")
#define  XML_LOWERCASE_LIST ((const XML_Char *) "Lower Case List")
#define  XML_UPPERCASE_LIST ((const XML_Char *) "Upper Case List")
#define  XML_UPPERROMAN_LIST ((const XML_Char *) "Upper Roman List")
#define  XML_LOWERROMAN_LIST ((const XML_Char *) "Lower Roman List")
#define  XML_BULLETED_LIST ((const XML_Char *) "Bullet List")
#define  XML_DASHED_LIST ((const XML_Char *) "Dashed List")

#define fmt_NUMBERED_LIST ((const char *)"%*%d")
#define  fmt_LOWERCASE_LIST ((const char *)"%*%a")
#define  fmt_UPPERCASE_LIST ((const char *)"%*%A")
#define  fmt_UPPERROMAN_LIST  ((const char *)"%*%R")
#define  fmt_LOWERROMAN_LIST  ((const char *)"%*%r")
#define  fmt_BULLETED_LIST ((const char *)"%b")
#define  fmt_DASHED_LIST ((const char *)"%c")

//////////////////////////////////////////////////////////////////////////
// End List definitions
//////////////////////////////////////////////////////////////////////////
#endif

