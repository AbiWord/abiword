 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#include <string.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xmlparse.h"
#include "pp_Property.h"
#include "pp_AttrProp.h"

#define NrElements(a)		(sizeof(a) / sizeof(a[0]))

/*****************************************************************/

static PP_Property _props[] =
{
	{ "color",					"000000",			1},
	{ "font-family",			"Times New Roman",	1},	// TODO this is Win32-specific.  must fix!
	{ "font-size",				"14pt",				1},
	{ "font-stretch",			"normal",			1},
	{ "font-style",				"normal",			1},
	{ "font-variant",			"normal",			1},
	{ "font-weight",			"normal",			1},
	{ "margin-bottom",			"0.25in",			0},
	{ "margin-top",				"0.25in",			0},
	{ "text-align",				"left",				1},
	{ "text-decoration",		"none",				1},

	{ "background-color",		"transparent",		0},
	{ "line-break-after",		"auto",				0},
	{ "line-break-before",		"auto",				0},
	{ "line-break-inside",		"auto",				0},
	{ "column-break-after",		"auto",				0},
	{ "column-break-before",	"auto",				0},
	{ "column-break-inside",	"auto",				0},
	{ "page-break-after",		"auto",				0},
	{ "page-break-before",		"auto",				0},
	{ "page-break-inside",		"auto",				0},
};

/*****************************************************************/

const XML_Char * PP_Property::getName() const
{
	return m_pszName;
}

const XML_Char * PP_Property::getInitial() const
{
	return m_pszInitial;
}

UT_Bool PP_Property::canInherit() const
{
	return m_bInherit;
}

const PP_Property * lookupProperty(const XML_Char * name)
{
	/*
		TODO we can make this faster later by storing all the property names
		in alphabetical order and doing a binary search.
	*/

	int i;
	int count = NrElements(_props);

	for (i=0; i<count; i++)
	{
		if (0 == UT_stricmp(_props[i].m_pszName, name))
		{
			return _props + i;
		}
	}

	return NULL;
}

const char * PP_evalProperty(const PP_Property * pProp,
							 const PP_AttrProp * pFmtNodeAttrProp,
							 const PP_AttrProp * pBlockAttrProp,
							 const PP_AttrProp * pSectionAttrProp)
{
	// TODO this routine needs to return an XML_Char

	const char * szValue;

	// find the value for the given property
	// by evaluating it in the contexts given.
	// use the CSS inheritance as necessary.

	if (!pProp)
	{
		UT_DEBUGMSG(("PP_evalProperty: null property given\n"));
		return NULL;
	}

	// see if the property is on the FmtNode item.
	
	if (pFmtNodeAttrProp && pFmtNodeAttrProp->getProperty(pProp->getName(),szValue))
		return szValue;

	// otherwise, see if we can inherit it from the containing block or the section.

	if (pProp->canInherit())
	{
		if (pBlockAttrProp && pBlockAttrProp->getProperty(pProp->getName(),szValue))
			return szValue;
		if (pSectionAttrProp && pSectionAttrProp->getProperty(pProp->getName(),szValue))
			return szValue;
	}

	// if no inheritance allowed for it or there is no
	// value set in containing block or section, we return
	// the default value for this property.
	
	return pProp->getInitial();
}

