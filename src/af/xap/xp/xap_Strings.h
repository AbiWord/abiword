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

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_xml.h"
#include "ut_vector.h"
#include "ut_string.h"
#include "ut_string_class.h"

class XAP_App;

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

	UT_String getValue(XAP_String_Id id, const char * inEncoding) const;
	UT_UTF8String getValueUTF8(XAP_String_Id id) const;

	void setEncoding(const char * inEndcoding);
	const char * getEncoding() const;

protected:
	XAP_App *					m_pApp;
	const XML_Char *			m_szLanguageName;

 private:
	UT_String m_encoding ;
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

class XAP_DiskStringSet : public XAP_StringSet, public UT_XML::Listener
{
public:
	XAP_DiskStringSet(XAP_App * pApp);
	virtual ~XAP_DiskStringSet(void);

	virtual bool				setValue(XAP_String_Id id, const XML_Char * szString);
	virtual bool				setValue(const XML_Char * szId, const XML_Char * szString);
	virtual const XML_Char *	getValue(XAP_String_Id id) const;
	virtual bool				loadStringsFromDisk(const char * szFilename);

	bool						setLanguage(const XML_Char * szLanguageName);
	void						setFallbackStringSet(XAP_StringSet * pFallback);
	
public:
	/* Implementation of UT_XML::Listener
	 */
	void					startElement(const XML_Char *name, const XML_Char **atts);
	void					endElement(const XML_Char *name);
	void					charData(const XML_Char *s, int len);

protected:
	XAP_StringSet *				m_pFallbackStringSet;

private:
	UT_Vector					m_vecStringsXAP;

	struct
	{
		bool				m_parserStatus;
	} m_parserState;
};

#endif /* XAP_STRINGS_H */
