/* AbiWord
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


#include "ut_types.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "pt_PieceTable.h"
#include "pd_Style.h"
#include "fl_AutoLists.h"
#include "ut_hash.h"

///////////////////////////////////////////////////////////////////
// Styles represent named collections of formatting properties.

#define _s(name, type, base, follow, props)		\
	do { const XML_Char * a[] = {				\
			PT_NAME_ATTRIBUTE_NAME, name,		\
			PT_TYPE_ATTRIBUTE_NAME, type,		\
			PT_BASEDON_ATTRIBUTE_NAME, base,	\
			PT_FOLLOWEDBY_ATTRIBUTE_NAME, follow,	\
			PT_PROPS_ATTRIBUTE_NAME, props,		\
			0};									\
		if (!_createBuiltinStyle(name, a))		\
			goto Failed;						\
	} while(0);

bool pt_PieceTable::_loadBuiltinStyles(void)
{
	char* list_fmt = "start-value:%s; margin-left:%fin; text-indent:-%fin; field-color:%s; list-delim:%s; field-font:%s; list-decimal:%s";
	char list_fmt_tmp[1024];
#ifdef BIDI_ENABLED
#ifdef BIDI_RTL_DOMINANT	
	_s("Normal",	"P", "",       "Current Settings", "font-family:Times New Roman; font-size:12pt; margin-top:0pt; dom-dir:rtl; text-align:right; line-height:1.0; field-font:NULL");
#else
	_s("Normal",	"P", "",       "Current Settings", "font-family:Times New Roman; font-size:12pt; margin-top:0pt; dom-dir:ltr; text-align:left; line-height:1.0; field-font:NULL");
#endif	
#else
	_s("Normal",	"P", "",       "Current Settings", "font-family:Times New Roman; font-size:12pt; margin-top:0pt; field-font:NULL");
#endif
	_s("Heading 1",	"P", "Normal", "Current Settings", "font-family:Arial; font-size:17pt; font-weight:bold; margin-top:22pt; margin-bottom:3pt; keep-with-next:1;  field-font:NULL");
	_s("Heading 2",	"P", "Normal", "Current Settings", "font-family:Arial; font-size:14pt; font-weight:bold; margin-top:22pt; margin-bottom:3pt; keep-with-next:1; field-font:NULL");
	_s("Heading 3",	"P", "Normal", "Current Settings", "font-family:Arial; font-size:12pt; font-weight:bold; margin-top:22pt; margin-bottom:3pt; keep-with-next:1;  field-font:NULL");
	_s("Plain Text","P", "Normal", "Current Settings", "font-family:Courier New;  field-font:NULL");
	_s("Block Text","P", "Normal", "Current Settings", "margin-left:1in; margin-right:1in; margin-bottom:6pt;  field-font:NULL");

	sprintf(list_fmt_tmp, list_fmt, "1", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L.", "NULL", ".");

	_s("Numbered List","P", "Normal", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "1", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L)", "NULL", ".");

	_s("Lower Case List","P", "Numbered List", "Current Settings", list_fmt_tmp);
	_s("Upper Case List","P", "Numbered List", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "1", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "NULL", ".");

	_s("Lower Roman List","P", "Normal", "Current Settings", list_fmt_tmp);
	_s("Upper Roman List","P", "Numbered List", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "Symbol", "NULL");

	_s("Bullet List", "P", "Normal", "Current Settings", list_fmt_tmp);
	_s("Implies List", "P", "Normal", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "NULL", "NULL");

	_s("Dashed List", "P", "Normal", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "Dingbats", "NULL");

	_s("Square List", "P", "Normal", "Current Settings", list_fmt_tmp);
	_s("Triangle List", "P", "Normal", "Current Settings", list_fmt_tmp);
	_s("Diamond List", "P", "Normal", "Current Settings", list_fmt_tmp);
	_s("Star List", "P", "Normal", "Current Settings", list_fmt_tmp);
	_s("Tick List", "P", "Normal", "Current Settings", list_fmt_tmp);
	_s("Box List", "P", "Normal", "Current Settings", list_fmt_tmp);
	_s("Hand List", "P", "Normal", "Current Settings", list_fmt_tmp);
	_s("Heart List", "P", "Normal", "Current Settings", list_fmt_tmp);


	return true;

Failed:
	return false;
}

bool pt_PieceTable::_createBuiltinStyle(const char * szName, const XML_Char ** attributes)
{
	// this function can only be called before loading the document.
	UT_ASSERT(m_pts==PTS_Create);

	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return false;

	// verify unique name

	PD_Style * pStyle = NULL;
	if (getStyle(szName,&pStyle) == true)
		return false;		// duplicate name

	pStyle = new PD_BuiltinStyle(this, indexAP);
	if (pStyle)
		m_hashStyles.insert((UT_HashTable::HashKeyType)szName,
							(UT_HashTable::HashValType)pStyle);
	return true;
}

bool pt_PieceTable::appendStyle(const XML_Char ** attributes)
{
	// this function can only be called while loading the document.
  //UT_ASSERT(m_pts==PTS_Loading);

	// first, store the attributes and properties and get an index to them.
	
	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return false;

	// verify unique name

	UT_ASSERT(sizeof(char) == sizeof(XML_Char));
	const char * szName = UT_getAttribute(PT_NAME_ATTRIBUTE_NAME, attributes);
	if (!szName || !*szName)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return true;		// silently ignore unnamed styles
	}
	UT_DEBUGMSG(("SEVIOR: Appending style %s \n",szName));
	PD_Style * pStyle = NULL;
	if (getStyle(szName,&pStyle) == true)
	{
		// duplicate name
		UT_ASSERT(pStyle);
		if (pStyle->isUserDefined())
		{
			// already loaded, ignore redefinition
			UT_DEBUGMSG(("appendStyle[%s]: duplicate definition ignored\n", szName));
			return true;	
		}

		// override builtin definition
		return pStyle->setIndexAP(indexAP);
	}
	else
	{
		// this is a new name
		pStyle = new PD_Style(this, indexAP);
		UT_DEBUGMSG(("SEVIOR: Creating new style %s \n",szName));
//
//TODO: Learn how to use Dom's AbiObject instead of this hack.
//
		if (pStyle)
			m_hashStyles.insert( (UT_HashTable::HashKeyType) UT_strdup(szName),(UT_HashTable::HashValType)pStyle);
//
// Diagonostic on Append...
//
		const PD_Style * pdStyle = NULL;
		const char * psdName =NULL;
		UT_uint32 i = 0;
		for(i=0; i<getStyleCount(); i++)
		{
			enumStyles(i,&psdName,&pdStyle);
			UT_DEBUGMSG(("SEVIOR: Found %d style name %s \n",i,psdName));
		}
		return true;
	}
}

bool pt_PieceTable::removeStyle (const XML_Char * szName)
{
	UT_ASSERT(szName);
	UT_ASSERT(sizeof(char) == sizeof(XML_Char));
	
	UT_DEBUGMSG(("DOM: remove the style, maybe recode the hash-table\n"));

	PD_Style * pStyle;

	if (getStyle(szName,&pStyle))
	{
		if (!pStyle->isUserDefined())
			return false; // can't destroy a builtin style
		
		delete pStyle;

		m_hashStyles.remove ((UT_HashTable::HashKeyType)szName, 0);
		return true;
	}

	return false;
}

bool pt_PieceTable::getStyle(const char * szName, PD_Style ** ppStyle) const
{
	UT_ASSERT(szName && *szName);
	
	UT_HashTable::HashValType pHashEntry = m_hashStyles.pick ((UT_HashTable::HashKeyType)szName);
	if (!pHashEntry)
		return false;

	PD_Style * pStyle = (PD_Style *) pHashEntry;
	UT_ASSERT(pStyle);
	
	if (ppStyle)
	{
		*ppStyle = pStyle;
	}
	
	return true;
}

size_t pt_PieceTable::getStyleCount (void)
{
  return (size_t) m_hashStyles.size();
}

bool pt_PieceTable::enumStyles(UT_uint32 k,
							   const char ** pszName, 
							   const PD_Style ** ppStyle) const
{
	// return the kth style.

	UT_uint32 kLimit = m_hashStyles.size();
	if (k >= kLimit)
		return false;
	
	UT_HashTable::UT_HashCursor c (&m_hashStyles);
	UT_HashTable::HashValType val = c.first();
	UT_uint32 i = 0;

	while (i != k)
	{
		i++;
		if (!c.more())
			return false;
		val = c.next();
	}

	PD_Style * pStyle = (PD_Style *)val;
	UT_ASSERT(pStyle);
	
	if (ppStyle)
	{
		*ppStyle = pStyle;
	}

	if (pszName)
	{
		*pszName = (XML_Char *)c.key();
	}
	UT_ASSERT(*pszName);
	return true;
}
	
