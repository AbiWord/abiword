/* AbiHello
 * Copyright (C) 1999 AbiSource, Inc.
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

#ifndef AP_STRINGS_H
#define AP_STRINGS_H

#include "xap_Strings.h"

//////////////////////////////////////////////////////////////////
// build a table of AP ID values
//////////////////////////////////////////////////////////////////

#define dcl(id,s)					AP_STRING_ID_##id,

typedef enum _AP_String_Id_Enum
{
	AP_STRING_ID__FIRST__			= 1000,	/* must be first -- must be >= XAP_STRING_ID__LAST__ */
#include "ap_String_Id.h"
	AP_STRING_ID__LAST__					/* must be last */
} AP_String_Id_Enum;

#undef dcl

//////////////////////////////////////////////////////////////////
// a sub-class to wrap the compiled-in (english) strings
//////////////////////////////////////////////////////////////////

class AP_BuiltinStringSet : public XAP_BuiltinStringSet
{
public:
	AP_BuiltinStringSet(XAP_App * pApp, const XML_Char * szLanguageName);
	virtual ~AP_BuiltinStringSet(void);

	virtual const XML_Char *	getValue(XAP_String_Id id) const;

#ifdef DEBUG
	UT_Bool						dumpBuiltinSet(const char * szFilename) const;
#endif

protected:
	const XML_Char **			m_arrayAP;
};

//////////////////////////////////////////////////////////////////
// a sub-class to deal with disk-based string sets (translations)
//////////////////////////////////////////////////////////////////

class AP_DiskStringSet : public XAP_DiskStringSet
{
public:
	AP_DiskStringSet(XAP_App * pApp);
	virtual ~AP_DiskStringSet(void);

	virtual UT_Bool				setValue(XAP_String_Id id, const XML_Char * szString);
	virtual UT_Bool				setValue(const XML_Char * szId, const XML_Char * szString);
	virtual const XML_Char *	getValue(XAP_String_Id id) const;
	virtual UT_Bool				loadStringsFromDisk(const char * szFilename);

protected:
	UT_Vector					m_vecStringsAP;
};



#endif /* AP_STRINGS_H */
