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

#include <locale.h>

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
	/* 	
		!!! if adding or removing properties to the list_fmt, you have to make also changes to
		pt_VarSet.cpp mergeAP()
	*/

	char * old_locale = setlocale (LC_NUMERIC, "C");
	
	char* list_fmt = " list-style:%s; start-value:%s; margin-left:%fin; text-indent:-%fin; field-color:%s;list-delim:%s; field-font:%s; list-decimal:%s";
	char list_fmt_tmp[1024];
#ifdef BIDI_ENABLED
#ifdef BIDI_RTL_DOMINANT
	_s("Normal",	"P", "",       "Current Settings", "font-family:Times New Roman; font-size:12pt; font-weight:normal; font-style:normal; font-stretch:normal; font-variant:normal; dom-dir:rtl; text-align:right; margin-top:0pt; margin-bottom:0pt; margin-left:0pt; margin-right:0pt; text-decoration:none; text-indent:0in; text-position:normal; line-height:1.0; color:000000; bgcolor:transparent; widows:2");	
#else
	_s("Normal",	"P", "",       "Current Settings", "font-family:Times New Roman; font-size:12pt; dom-dir:ltr; font-weight:normal; font-style:normal; font-stretch:normal; font-variant:normal; margin-top:0pt; margin-bottom:0pt; margin-left:0pt; margin-right:0pt; text-decoration:none; text-indent:0in; text-position:normal; text-align:left; line-height:1.0; color:000000; bgcolor:transparent; widows:2");
#endif	
#else
	_s("Normal",	"P", "",       "Current Settings", "font-family:Times New Roman; font-size:12pt; font-weight:normal; font-style:normal; font-stretch:normal; font-variant:normal; margin-top:0pt; margin-bottom:0pt; margin-left:0pt; margin-right:0pt; text-decoration:none; text-indent:0in; text-position:normal; text-align:left; line-height:1.0; color:000000; bgcolor:transparent; widows:2");
#endif
	_s("Heading 1",	"P", "Normal", "Normal", "font-family:Arial; font-size:17pt; font-weight:bold; margin-top:22pt; margin-bottom:3pt; keep-with-next:1");
	_s("Heading 2",	"P", "Normal", "Normal", "font-family:Arial; font-size:14pt; font-weight:bold; margin-top:22pt; margin-bottom:3pt; keep-with-next:1");
	_s("Heading 3",	"P", "Normal", "Normal", "font-family:Arial; font-size:12pt; font-weight:bold; margin-top:22pt; margin-bottom:3pt; keep-with-next:1");
	_s("Plain Text","P", "Normal", "Current Settings", "font-family:Courier New");
	_s("Block Text","P", "Normal", "Current Settings", "margin-left:1in; margin-right:1in; margin-bottom:6pt");

	sprintf(list_fmt_tmp, list_fmt, "Numbered List", "1",LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L.", "NULL", ".");
	_s("Numbered List","P", "", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "Lower Case List","1", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L)", "NULL", ".");
	_s("Lower Case List","P", "Numbered List", "Current Settings", list_fmt_tmp);
	sprintf(list_fmt_tmp, list_fmt, "Upper Case List","1", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L)", "NULL", ".");
	_s("Upper Case List","P", "Numbered List", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "Lower Roman List","1", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "NULL", ".");
	_s("Lower Roman List","P", "Normal", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt,"Upper Roman List","1", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "NULL", ".");
	_s("Upper Roman List","P", "Numbered List", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "Bullet List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "Symbol", "NULL");

	_s("Bullet List", "P", "", "Current Settings", list_fmt_tmp);
	sprintf(list_fmt_tmp, list_fmt, "Implies List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "Symbol", "NULL");
	_s("Implies List", "P", "", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "Dashed List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "NULL", "NULL");
	_s("Dashed List", "P", "", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "Square List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "Dingbats", "NULL");
	_s("Square List", "P", "", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "Triangle List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "Dingbats", "NULL");
	_s("Triangle List", "P", "", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "Diamond List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "Dingbats", "NULL");
	_s("Diamond List", "P", "", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "Star List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "Dingbats", "NULL");
	_s("Star List", "P", "", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "Tick List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "Dingbats", "NULL");
	_s("Tick List", "P", "", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "Box List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "Dingbats", "NULL");
	_s("Box List", "P", "", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "Hand List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "Dingbats", "NULL");
	_s("Hand List", "P", "", "Current Settings", list_fmt_tmp);

	sprintf(list_fmt_tmp, list_fmt, "Heart List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "Dingbats", "NULL");
	_s("Heart List", "P", "", "Current Settings", list_fmt_tmp);

    _s("Numbered Heading 1","P","Heading 1","Normal","tabstops:0.3in/L0; list-style:Numbered List; start-value:1; margin-left:0.0in; text-indent:0.0in; field-color:transparent; list-delim:%L.; field-font:Arial; list-decimal:");

    _s("Numbered Heading 2","P","Heading 2","Normal","tabstops:0.3in/L0; list-style:Numbered List; start-value:1; margin-left:0.0in; text-indent:0.0in; field-color:transparent; list-delim:%L.; field-font:Arial; list-decimal:");

    _s("Numbered Heading 3","P","Heading 3","Normal","tabstops:0.3in/L0; list-style:Numbered List; start-value:1; margin-left:0.0in; text-indent:0.0in; field-color:transparent; list-delim:%L.; field-font:Arial; list-decimal:");

    _s("Chapter Heading","P","Numbered Heading 1","Normal","tabstops:1.1in/L0; list-style:Numbered List; start-value:1; margin-left:0.0in; text-indent:0.0in; field-color:transparent; list-delim:Chapter %L.; field-font:Arial; list-decimal:");

    _s("Section Heading","P","Numbered Heading 1","Normal","tabstops:1.1in/L0; list-style:Numbered List; start-value:1; margin-left:0.0in; text-indent:0.0in; field-color:transparent; list-delim:Section %L.; field-font:Arial; list-decimal:");

#ifndef NDEBUG
	_s("Endnote Reference","C", "None", "Current Settings", "text-position:superscript; font-size:10pt");
	_s("Endnote Text","P", "Normal", "Current Settings", "text-position:normal");
#endif

#if 0
	_s("Footnote Reference","C", "None", "Current Settings", "text-position:superscript; font-size:10pt");
	_s("Footnote Text","P", "Normal", "Current Settings", "text-position:normal");
#endif
	setlocale (LC_NUMERIC, old_locale);
	return true;

Failed:
	setlocale (LC_NUMERIC, old_locale);
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

	pStyle = new PD_BuiltinStyle(this, indexAP, szName);
	if (pStyle)
	  {
		m_hashStyles.insert(szName,
				    (void *)pStyle);
	  }
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
	xxx_UT_DEBUGMSG(("SEVIOR: Appending style %s \n",szName));
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
		pStyle = new PD_Style(this, indexAP, szName);
		xxx_UT_DEBUGMSG(("SEVIOR: Creating new style %s \n",szName));
//
//TODO: Learn how to use Dom's AbiObject instead of this hack.
//
		if (pStyle)
			m_hashStyles.insert(szName,(void *)pStyle);
//
// Diagonostic on Append...
//
		const PD_Style * pdStyle = NULL;
		const char * psdName =NULL;
		UT_uint32 i = 0;
		for(i=0; i<getStyleCount(); i++)
		{
			enumStyles(i,&psdName,&pdStyle);
			xxx_UT_DEBUGMSG(("SEVIOR: Found %d style name %s \n",i,psdName));
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

		m_hashStyles.remove (szName, NULL);
		return true;
	}

	return false;
}

bool pt_PieceTable::getStyle(const char * szName, PD_Style ** ppStyle) const
{
	UT_ASSERT(szName && *szName);
	
	const void * pHashEntry = m_hashStyles.pick (szName);
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


///////////////////////////////////////////////////////////////////////
/*!
 * compareStyleNames this function is used to compare the char * strings names
 * of the styles with the qsort method on UT_Vector.
\params const void * vS1  - pointer to a PD_Style pointer
\params const void * vS2  - pointer to a PD_Style pointer
\returns -ve if sz1 < sz2, 0 if sz1 == sz2, +ve if sz1 > sz2
*/
static UT_sint32 compareStyleNames(const void * vS1, const void * vS2)
{
	const PD_Style ** pS1 = (const PD_Style **) vS1;
	const PD_Style ** pS2 = (const PD_Style **) vS2;
	const char * sz1 = (*pS1)->getName();
	const char * sz2 = (*pS2)->getName();
	return UT_stricmp(sz1, sz2);
}

bool pt_PieceTable::enumStyles(UT_uint32 k,
							   const char ** pszName, 
							   const PD_Style ** ppStyle) const
{
	// return the kth style.

	UT_uint32 kLimit = m_hashStyles.size();
	if (k >= kLimit)
		return false;
	
	UT_Vector * vStyle = m_hashStyles.enumerate() ;
	vStyle->qsort(compareStyleNames);

	PD_Style * pStyle = (PD_Style *) vStyle->getNthItem(k);
	UT_ASSERT(pStyle);
	
	if (ppStyle)
	{
		*ppStyle = pStyle;
	}

	if (pszName)
	{
	  *pszName = pStyle->getName();
	}
	UT_ASSERT(*pszName);

	delete vStyle;

	return true;
}
