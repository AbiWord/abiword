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

UT_Bool pt_PieceTable::_loadBuiltinStyles(void)
{
	_s("Normal",	"P", "",       "Normal", "font-family:Times New Roman; font-size:12pt; text-align:left; line-height:1.0; field-font:NULL");
	_s("Heading 1",	"P", "Normal", "Normal", "font-family:Arial; font-size:16pt; font-weight:bold; margin-top:12pt; margin-bottom:3pt; keep-with-next:1;  field-font:NULL");
	_s("Heading 2",	"P", "Normal", "Normal", "font-family:Arial; font-size:14pt; font-weight:bold; font-style:italic; margin-top:12pt; margin-bottom:3pt; keep-with-next:1; field-font:NULL");
	_s("Heading 3",	"P", "Normal", "Normal", "font-family:Arial; font-size:14pt; margin-top:14pt; margin-bottom:3pt; keep-with-next:1;  field-font:NULL");
	_s("Plain Text","P", "Normal", "Plain Text", "font-family:Courier New;  field-font:NULL");
	_s("Block Text","P", "Normal", "Block Text", "margin-left:1in; margin-right:1in; margin-bottom:6pt;  field-font:NULL");
	_s("Numbered List","P", "Normal", "Numbered List", "start-value:1; margin-left:0.25in; text-indent:-0.2500in; field-color: ffffff; field-font:NULL;list-delim::%L;list-decimal:.");
	_s("Lower Case List","P", "Normal", "Lower Case List", "start-value:1; margin-left:0.25in; text-indent:-0.2500in; field-color: ffffff; field-font:NULL;list-delim:%L;list-decimal:.");
	_s("Upper Case List","P", "Normal", "Upper Case List", "start-value:1; margin-left:0.25in; text-indent:-0.2500in; field-color: ffffff; field-font:NULL;list-delim:%L;list-decimal:.");
	_s("Upper Roman List","P", "Normal", "Upper Roman List", "start-value:1; margin-left:0.25in; text-indent:-0.2500in; field-color: ffffff; field-font:NULL;list-delim:%L;list-decimal:.");
	_s("Lower Roman List","P", "Normal", "Lower Roman List", "start-value:1; margin-left:0.25in; text-indent:-0.2500in; field-color: ffffff; field-font:NULL;list-delim:%L;list-decimal:.");
	_s("Bullet List", "P", "Normal", "Bullet List", "margin-left:0.25in; text-indent:-0.2500in; start-value:0; field-color: ffffff; field-font:Symbol;list-delim:%L;list-decimal:NULL");
	_s("Dashed List", "P", "Normal", "Dashed List", "margin-left:0.25in; text-indent:-0.2500in; start-value:0; field-color: ffffff; field-font:NULL;list-delim:%L;list-decimal:NULL");
	_s("Square List", "P", "Normal", "Square List", "margin-left:0.25in; text-indent:-0.2500in; start-value:0; field-color: ffffff; field-font:Dingbats;list-delim:%L;list-decimal:NULL");
	_s("Triangle List", "P", "Normal", "Triangle List", "margin-left:0.25in; text-indent:-0.2500in; start-value:0; field-color: ffffff; field-font:Dingbats;list-delim:%L;list-decimal:NULL");
	_s("Diamond List", "P", "Normal", "Diamond List", "margin-left:0.25in; text-indent:-0.2500in; start-value:0; field-color: ffffff; field-font:Dingbats;list-delim:%L;list-decimal:NULL");
	_s("Star List", "P", "Normal", "Star List", "margin-left:0.25in; text-indent:-0.2500in; start-value:0; field-color: ffffff; field-font:Dingbats;list-delim:%L;list-decimal:NULL");
	_s("Implies List", "P", "Normal", "Implies List", "margin-left:0.25in; text-indent:-0.2500in; start-value:0; field-color: ffffff; field-font:Symbol;list-delim:%L;list-decimal:NULL");
	_s("Tick List", "P", "Normal", "Tick List", "margin-left:0.25in; text-indent:-0.2500in; start-value:0; field-color: ffffff; field-font:Dingbats;list-delim:%L;list-decimal:NULL");
	_s("Box List", "P", "Normal", "Box List", "margin-left:0.25in; text-indent:-0.2500in; start-value:0; field-color: ffffff; field-font:Dingbats;list-delim:%L;list-decimal:NULL");
	_s("Hand List", "P", "Normal", "Hand List", "margin-left:0.25in; text-indent:-0.2500in; start-value:0; field-color: ffffff; field-font:Dingbats;list-delim:%L;list-decimal:NULL");
	_s("Heart List", "P", "Normal", "Heart List", "margin-left:0.25in; text-indent:-0.2500in; start-value:0; field-color: ffffff; field-font:Dingbats;list-delim:%L;list-decimal:NULL");


	return UT_TRUE;

Failed:
	return UT_FALSE;
}

UT_Bool pt_PieceTable::_createBuiltinStyle(const char * szName, const XML_Char ** attributes)
{
	// this function can only be called before loading the document.
	UT_ASSERT(m_pts==PTS_Create);

	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return UT_FALSE;

	// verify unique name

	PD_Style * pStyle = NULL;
	if (getStyle(szName,&pStyle) == UT_TRUE)
		return UT_FALSE;		// duplicate name

	pStyle = new PD_BuiltinStyle(this, indexAP);
	if (pStyle)
		if (m_hashStyles.addEntry(szName,NULL,(void *)pStyle) != -1)
			return UT_TRUE;

	// cleanup after failure
	if (pStyle)
		delete pStyle;
	return UT_FALSE;
}

UT_Bool pt_PieceTable::appendStyle(const XML_Char ** attributes)
{
	// this function can only be called while loading the document.
	UT_ASSERT(m_pts==PTS_Loading);

	// first, store the attributes and properties and get an index to them.
	
	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return UT_FALSE;

	// verify unique name

	UT_ASSERT(sizeof(char) == sizeof(XML_Char));
	const char * szName = UT_getAttribute(PT_NAME_ATTRIBUTE_NAME, attributes);
	if (!szName || !*szName)
		return UT_TRUE;		// silently ignore unnamed styles

	PD_Style * pStyle = NULL;
	if (getStyle(szName,&pStyle) == UT_TRUE)
	{
		// duplicate name
		UT_ASSERT(pStyle);
		if (pStyle->isUserDefined())
		{
			// already loaded, ignore redefinition
			UT_DEBUGMSG(("appendStyle[%s]: duplicate definition ignored\n", szName));
			return UT_TRUE;	
		}

		// override builtin definition
		return pStyle->setIndexAP(indexAP);
	}
	else
	{
		// this is a new name
		pStyle = new PD_Style(this, indexAP);
		if (pStyle)
			if (m_hashStyles.addEntry(szName,NULL,(void *)pStyle) != -1)
				return UT_TRUE;

		// cleanup after failure
		if (pStyle)
			delete pStyle;
		return UT_FALSE;
	}
}

UT_Bool pt_PieceTable::getStyle(const char * szName, PD_Style ** ppStyle) const
{
	UT_ASSERT(szName && *szName);
	
	UT_HashEntry * pHashEntry = m_hashStyles.findEntry(szName);
	if (!pHashEntry)
		return UT_FALSE;

	PD_Style * pStyle = (PD_Style *) pHashEntry->pData;
	UT_ASSERT(pStyle);
	
	if (ppStyle)
	{
		*ppStyle = pStyle;
	}
	
	return UT_TRUE;
}

UT_Bool pt_PieceTable::enumStyles(UT_uint32 k,
								const char ** pszName, const PD_Style ** ppStyle) const
{
	// return the kth style.

	UT_uint32 kLimit = m_hashStyles.getEntryCount();
	if (k >= kLimit)
		return UT_FALSE;
	
	const UT_HashEntry * pHashEntry = m_hashStyles.getNthEntryAlpha(k);
	UT_ASSERT(pHashEntry);

	PD_Style * pStyle = (PD_Style *) pHashEntry->pData;
	UT_ASSERT(pStyle);
	
	if (ppStyle)
	{
		*ppStyle = pStyle;
	}

	if (pszName)
	{
		*pszName = pHashEntry->pszLeft;
	}
	
	return UT_TRUE;
}
	
