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

#ifndef XAP_STRINGS_H
#define XAP_STRINGS_H

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_alphahash.h"
#include "ut_string.h"
#include "xmlparse.h"
#include "xap_App.h"

//////////////////////////////////////////////////////////////////
// build a table of XAP ID values
//////////////////////////////////////////////////////////////////

#define dcl(id,s)					XAP_STRING_ID_##id,

typedef enum _XAP_String_Id_Enum
{
	XAP_STRING_ID__FIRST__			= 0,	/* must be first */
#include "xap_String_Id.h"
	XAP_STRING_ID__LAST__					/* must be last */
} XAP_String_Id_Enum;

#undef dcl

//////////////////////////////////////////////////////////////////
// Both XAP_ and AP_ enum sets fold into XAP_String_Id
//////////////////////////////////////////////////////////////////

typedef UT_uint32			XAP_String_Id;

//////////////////////////////////////////////////////////////////
// base class provides interface regardless of how we got the strings
//////////////////////////////////////////////////////////////////

class XAP_StringSet
{
public:
	XAP_StringSet(XAP_App * pApp, const XML_Char * szLanguageName);
	virtual ~XAP_StringSet(void);

	const XML_Char *			getLanguageName(void) const;

	virtual const XML_Char *	getValue(XAP_String_Id id) const = 0;

protected:
	XAP_App *					m_pApp;
	const XML_Char *			m_szLanguageName;
};

//////////////////////////////////////////////////////////////////
// a sub-class to wrap the compiled-in (english) strings
//////////////////////////////////////////////////////////////////

class XAP_BuiltinStringSet : public XAP_StringSet
{
public:
	XAP_BuiltinStringSet(XAP_App * pApp, const XML_Char * szLanguageName);
	virtual ~XAP_BuiltinStringSet(void);

	virtual const XML_Char *	getValue(XAP_String_Id id) const;
	
private:
	const XML_Char **			m_arrayXAP;
};

//////////////////////////////////////////////////////////////////
// a sub-class to deal with disk-based string sets (translations)
//////////////////////////////////////////////////////////////////

class XAP_DiskStringSet : public XAP_StringSet
{
public:
	XAP_DiskStringSet(XAP_App * pApp);
	virtual ~XAP_DiskStringSet(void);

	virtual UT_Bool				setValue(XAP_String_Id id, const XML_Char * szString);
	virtual UT_Bool				setValue(const XML_Char * szId, const XML_Char * szString);
	virtual const XML_Char *	getValue(XAP_String_Id id) const;
	virtual UT_Bool				loadStringsFromDisk(const char * szFilename);

	UT_Bool						setLanguage(const XML_Char * szLanguageName);
	void						setFallbackStringSet(XAP_StringSet * pFallback);
	
public:
	void						_startElement(const XML_Char *name, const XML_Char **atts);
	void						_endElement(const XML_Char *name);
	void						_charData(const XML_Char *s, int len);

protected:
	XAP_StringSet *				m_pFallbackStringSet;

private:
	UT_Vector					m_vecStringsXAP;

	struct
	{
		UT_Bool				m_parserStatus;
	} m_parserState;
};

#endif /* XAP_STRINGS_H */
