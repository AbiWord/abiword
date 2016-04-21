/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include "ut_locale.h"
#include "ut_types.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "pt_PieceTable.h"
#include "pd_Style.h"
#include "fl_AutoLists.h"
#include "ut_hash.h"
#include "xap_Strings.h"
#include "xap_App.h"
#include "xap_EncodingManager.h"

///////////////////////////////////////////////////////////////////
// Styles represent named collections of formatting properties.

#define _s(name, displayed, type, base, follow, props)	\
	do { const PP_PropertyVector a = {					\
			PT_NAME_ATTRIBUTE_NAME, name,				\
			PT_TYPE_ATTRIBUTE_NAME, type,				\
			PT_BASEDON_ATTRIBUTE_NAME, base,			\
			PT_FOLLOWEDBY_ATTRIBUTE_NAME, follow,		\
			PT_PROPS_ATTRIBUTE_NAME, props				\
			};											\
		if (!_createBuiltinStyle(name, displayed, a))	\
			goto Failed;								\
	} while(0)

typedef struct
{
	const char*	pStyle;
	int		nID;	
	
} ST_LOCALISED_STYLES;

//
//	A list of the styles and they correspondant localised name
//
//  IMPORTANT: when adding styles here, please check also
//  s_translateStyleId() in ie_imp_MSWord_97.cpp and if there is a
//  corresponding Word style make sure that it gets translated into
//  this unlocalised name. Tomas, May 10, 2003
ST_LOCALISED_STYLES stLocalised[] =
{
	
	{"Numbered List",			XAP_STRING_ID_STYLE_NUMBER_LIST},       
	{"Plain Text",      		XAP_STRING_ID_STYLE_PLAIN_TEXT},    
	{"Heading 1",           	XAP_STRING_ID_STYLE_HEADING1},		
	{"Heading 2",           	XAP_STRING_ID_STYLE_HEADING2},		
	{"Heading 3",           	XAP_STRING_ID_STYLE_HEADING3},		
	{"Heading 4",           	XAP_STRING_ID_STYLE_HEADING4},		
	{"Contents Header",         XAP_STRING_ID_STYLE_TOCHEADING},		
	{"Contents 1",              XAP_STRING_ID_STYLE_TOCHEADING1},		
	{"Contents 2",              XAP_STRING_ID_STYLE_TOCHEADING2},		
	{"Contents 3",              XAP_STRING_ID_STYLE_TOCHEADING3},		
	{"Contents 4",              XAP_STRING_ID_STYLE_TOCHEADING4},		
	{"Normal",         	        XAP_STRING_ID_STYLE_NORMAL},		              
	{"Block Text",              XAP_STRING_ID_STYLE_BLOCKTEXT},	
	{"Lower Case List",         XAP_STRING_ID_STYLE_LOWERCASELIST},
	{"Upper Case List",         XAP_STRING_ID_STYLE_UPPERCASTELIST},
	{"Lower Roman List",        XAP_STRING_ID_STYLE_LOWERROMANLIST},
	{"Upper Roman List",        XAP_STRING_ID_STYLE_UPPERROMANLIST},
	{"Bullet List",             XAP_STRING_ID_STYLE_BULLETLIST},	
	{"Dashed List",             XAP_STRING_ID_STYLE_DASHEDLIST},	
	{"Square List",             XAP_STRING_ID_STYLE_SQUARELIST},	
	{"Triangle List",           XAP_STRING_ID_STYLE_TRIANGLELIST},	
	{"Diamond List",	        XAP_STRING_ID_STYLE_DIAMONLIST},	
	{"Star List",               XAP_STRING_ID_STYLE_STARLIST},		
	{"Tick List",               XAP_STRING_ID_STYLE_TICKLIST},		
	{"Box List",                XAP_STRING_ID_STYLE_BOXLIST},		
	{"Hand List",               XAP_STRING_ID_STYLE_HANDLIST},		
	{"Heart List",              XAP_STRING_ID_STYLE_HEARTLIST},	
	{"Arrowhead List",          XAP_STRING_ID_STYLE_ARROWHEADLIST},	
	{"Chapter Heading",         XAP_STRING_ID_STYLE_CHAPHEADING},	
	{"Section Heading",         XAP_STRING_ID_STYLE_SECTHEADING},	
	{"Endnote Reference",       XAP_STRING_ID_STYLE_ENDREFERENCE},	
	{"Endnote Text",            XAP_STRING_ID_STYLE_ENDTEXT},		
	{"Endnote",                 XAP_STRING_ID_STYLE_ENDNOTE},		
	{"Footnote Reference",      XAP_STRING_ID_STYLE_FOOTREFERENCE},	
	{"Footnote Text",           XAP_STRING_ID_STYLE_FOOTTEXT},		
	{"Footnote",                XAP_STRING_ID_STYLE_FOOTNOTE},		
	{"Numbered Heading 1",      XAP_STRING_ID_STYLE_NUMHEAD1},		
	{"Numbered Heading 2",      XAP_STRING_ID_STYLE_NUMHEAD2},		
	{"Numbered Heading 3",      XAP_STRING_ID_STYLE_NUMHEAD3},		
	{"Implies List",	       	XAP_STRING_ID_STYLE_IMPLIES_LIST},		
	{"None",                    XAP_STRING_ID_STYLE_NONE},		
	{NULL,	0}	
};


/*
	Gets a style name and returns its localised name
*/
void pt_PieceTable::s_getLocalisedStyleName(const char *szStyle, std::string &utf8)
{		
  static XAP_App * pApp = XAP_App::getApp();

	const XAP_StringSet * pSS = pApp->getStringSet();
	utf8 = szStyle;
	int n;		
		
	for (n=0; stLocalised[n].pStyle; n++)
	{
		if (strcmp(szStyle, stLocalised[n].pStyle)==0)
		{
			pSS->getValueUTF8(stLocalised[n].nID, utf8);			
			break;
		}
	}		
	
}

/*
	Gets the style name from its localised name
*/
const char *pt_PieceTable::s_getUnlocalisedStyleName (const char *szLocStyle)
{		
	static XAP_App *pApp = XAP_App::getApp();
	const XAP_StringSet *pSS = pApp->getStringSet();

	for (int n = 0; stLocalised[n].pStyle; n++)
		if (strcmp(szLocStyle, pSS->getValue(stLocalised[n].nID)) == 0)
			return stLocalised[n].pStyle;

	return szLocStyle;
}

bool pt_PieceTable::_loadBuiltinStyles(void)
{
	XAP_App *pApp = XAP_App::getApp();
	const XAP_StringSet *pSS = pApp->getStringSet();
	/* 	
		!!! if adding or removing properties to the list_fmt, you have to make also changes to
		pt_VarSet.cpp mergeAP()
	*/
	UT_LocaleTransactor t(LC_NUMERIC, "C");

	const char* list_fmt = " list-style:%s; start-value:%s; margin-left:%fin; text-indent:-%fin; "
		"field-color:%s;list-delim:%s; field-font:%s; list-decimal:%s";
	UT_String list_fmt_tmp;
	UT_String stTmp;
	const char* szFmt;
	
	// findNearestFont will do a fuzzy match, and return the nearest font in the
	// system -- use the locale language
	UT_UTF8String s = XAP_EncodingManager::get_instance()->getLanguageISOName();

	const char * pCountry
		= XAP_EncodingManager::get_instance()->getLanguageISOTerritory();
	
	if(pCountry)
	{
		s += "-";
		s += pCountry;
	}
	
	const char* pszFamily = XAP_App::findNearestFont("Times New Roman",
													 "normal", "",
													 "normal", "", "12pt",
													 s.utf8_str());
	
	UT_String_sprintf(stTmp, "font-family:%s; font-size:12pt; font-weight:normal; "
					  "font-style:normal; font-stretch:normal; font-variant:normal; "
					  "margin-top:0pt; margin-bottom:0pt; "
					  "margin-left:0pt; margin-right:0pt; text-decoration:none; "
					  "text-indent:0in; text-position:normal; line-height:1.0; "
					  "color:000000; bgcolor:transparent; widows:2", pszFamily);

	pszFamily = XAP_App::findNearestFont("Arial", "normal", "",
										 "normal", "", "12pt", s.utf8_str());

	// used to set the dom-dir of the style here, but we do not want to do that. The
	// dom-dir property should be inherited from the section or document (the user can, of
	// course, modify the style, but that is up to them).
#	ifdef BIDI_RTL_DOMINANT
	stTmp += "; text-align:right";
#	else
	stTmp += "; text-align:left";
#	endif

	_s("Normal", true,	"P", "",       "Current Settings", stTmp.c_str());
	
	szFmt = "font-family:%s; font-size:%dpt; font-weight:bold; margin-top:22pt; margin-bottom:3pt; keep-with-next:1";
	UT_String_sprintf(stTmp, szFmt, pszFamily, 17);
	_s("Heading 1", true,	"P", "Normal", "Normal", stTmp.c_str());
	UT_String_sprintf(stTmp, szFmt, pszFamily, 14);
	_s("Heading 2", true,	"P", "Normal", "Normal", stTmp.c_str());
	UT_String_sprintf(stTmp, szFmt, pszFamily, 12);
	_s("Heading 3", true,	"P", "Normal", "Normal", stTmp.c_str());
	_s("Heading 4", true,	"P", "Normal", "Normal", stTmp.c_str());
	_s("Plain Text", true,"P", "Normal", "Current Settings", "font-family:Courier New");
	_s("Block Text", true,"P", "Normal", "Current Settings", "margin-left:1in; margin-right:1in; margin-bottom:6pt");

	UT_String_sprintf(stTmp, list_fmt, "Numbered List", "1",LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L.", "NULL", ".");
	_s("Numbered List",true,"P", "", "Current Settings", stTmp.c_str());

	UT_String_sprintf(stTmp, list_fmt, "Lower Case List","1", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L)", "NULL", ".");
	_s("Lower Case List",true,"P", "Numbered List", "Current Settings", stTmp.c_str());
	UT_String_sprintf(stTmp, list_fmt, "Upper Case List","1", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L)", "NULL", ".");
	_s("Upper Case List",false,"P", "Numbered List", "Current Settings", stTmp.c_str());

	UT_String_sprintf(stTmp, list_fmt, "Lower Roman List","1", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "NULL", ".");
	_s("Lower Roman List",false,"P", "Normal", "Current Settings", stTmp.c_str());

	UT_String_sprintf(stTmp, list_fmt,"Upper Roman List","1", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "NULL", ".");
	_s("Upper Roman List",false,"P", "Numbered List", "Current Settings", stTmp.c_str());

	UT_String_sprintf(stTmp, list_fmt, "Bullet List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", pszFamily,"NULL");

	_s("Bullet List",true, "P", "", "Current Settings", stTmp.c_str());
	UT_String_sprintf(stTmp, list_fmt, "Implies List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", pszFamily, "NULL");
	_s("Implies List",false, "P", "", "Current Settings", stTmp.c_str());

	UT_String_sprintf(stTmp, list_fmt, "Dashed List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", "NULL", "NULL");
	_s("Dashed List",true, "P", "", "Current Settings", stTmp.c_str());

	UT_String_sprintf(stTmp, list_fmt, "Square List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", pszFamily, "NULL");
	_s("Square List",false, "P", "", "Current Settings", stTmp.c_str());

	UT_String_sprintf(stTmp, list_fmt, "Triangle List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", pszFamily, "NULL");
	_s("Triangle List",false, "P", "", "Current Settings", stTmp.c_str());

	UT_String_sprintf(stTmp, list_fmt, "Diamond List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", pszFamily, "NULL");
	_s("Diamond List",false, "P", "", "Current Settings", stTmp.c_str());

	UT_String_sprintf(stTmp, list_fmt, "Star List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", pszFamily, "NULL");
	_s("Star List",false, "P", "", "Current Settings", stTmp.c_str());

	UT_String_sprintf(stTmp, list_fmt, "Tick List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", pszFamily, "NULL");
	_s("Tick List",false, "P", "", "Current Settings", stTmp.c_str());

	UT_String_sprintf(stTmp, list_fmt, "Box List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", pszFamily, "NULL");
	_s("Box List",false, "P", "", "Current Settings", stTmp.c_str());

	UT_String_sprintf(stTmp, list_fmt, "Hand List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", pszFamily, "NULL");
	_s("Hand List",false, "P", "", "Current Settings", stTmp.c_str());

	UT_String_sprintf(stTmp, list_fmt, "Heart List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", pszFamily, "NULL");
	_s("Heart List",false, "P", "", "Current Settings", stTmp.c_str());

	UT_String_sprintf(stTmp, list_fmt, "Arrowhead List","0", LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L", pszFamily, "NULL");
	_s("Arrowhead List",false, "P", "", "Current Settings", stTmp.c_str());

	// pszFamily is the nearest font to Arial found in the system
	UT_String_sprintf(stTmp, "tabstops:0.3in/L0; list-style:Numbered List; "
					  "start-value:1; margin-left:0.0in; text-indent:0.0in; "
					  "field-color:transparent; list-delim:%%L.; field-font:%s; "
					  "list-decimal:", pszFamily);


    _s("Numbered Heading 1",true,"P","Heading 1","Normal", stTmp.c_str());
    _s("Numbered Heading 2",true,"P","Heading 2","Normal", stTmp.c_str());
    _s("Numbered Heading 3",true,"P","Heading 3","Normal", stTmp.c_str());

	// pszFamily is the nearest font to Arial found in the system

	UT_String_sprintf(stTmp, list_fmt, "Numbered List", "1",LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L.", "NULL", ".");

    _s("Contents 1",false,"P","Normal","Normal", stTmp.c_str());

	UT_String_sprintf(stTmp, list_fmt, "Numbered List", "1",2*LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L.", "NULL", ".");
    _s("Contents 2",false,"P","Normal","Normal", stTmp.c_str());
	UT_String_sprintf(stTmp, list_fmt, "Numbered List", "1",3*LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L.", "NULL", ".");
    _s("Contents 3",false,"P","Normal","Normal", stTmp.c_str());
	UT_String_sprintf(stTmp, list_fmt, "Numbered List", "1",4*LIST_DEFAULT_INDENT, LIST_DEFAULT_INDENT_LABEL, "transparent", "%L.", "NULL", ".");
    _s("Contents 4",false,"P","Normal","Normal", stTmp.c_str());

	
	szFmt = "font-family:%s; font-size:%dpt; font-weight:bold; margin-top:12pt; margin-bottom:6pt; text-align:center; keep-with-next:1";
	UT_String_sprintf(stTmp, szFmt, pszFamily, 16);
    _s("Contents Header",false,"P","Normal","Normal", stTmp.c_str());


	szFmt = "tabstops:1.1in/L0; list-style:Numbered List; "
		"start-value:1; margin-left:0.0in; text-indent:0.0in; "
		"field-color:transparent; list-delim:%s %%L.; "
		"field-font:%s; list-decimal:";
	UT_String_sprintf(stTmp, szFmt, pSS->getValue(XAP_STRING_ID_STYLE_DELIM_CHAPTER), pszFamily);

    _s("Chapter Heading",true,"P","Numbered Heading 1","Normal", stTmp.c_str());

	UT_String_sprintf(stTmp, szFmt, pSS->getValue(XAP_STRING_ID_STYLE_DELIM_SECTION), pszFamily);
    _s("Section Heading",true,"P","Numbered Heading 1","Normal", stTmp.c_str());

	_s("Endnote Reference",false,"C", "None", "Current Settings", "text-position:superscript; font-size:10pt");
	_s("Endnote Text",false,"P", "Normal", "Current Settings", "text-position:normal");
	_s("Endnote",false,"P", "Normal", "Current Settings", "text-position:normal;text-indent:-0.2in;margin-left:0.2in");

	_s("Footnote Reference",false,"C", "None", "Current Settings", "text-position:superscript; font-size:10pt");
	_s("Footnote Text",false,"P", "Normal", "Current Settings", "text-position:normal; font-size:10pt");
	_s("Footnote",false,"P", "Normal", "Current Settings", "text-position:normal; font-size:10pt;text-indent:-0.2in;margin-left:0.2in");

	return true;

Failed:
	return false;
}

bool pt_PieceTable::_createBuiltinStyle(const char * szName, bool bDisplayed, const  PP_PropertyVector & attributes)
{
	// this function can only be called before loading the document.
	UT_return_val_if_fail (m_pts==PTS_Create, false);

	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return false;

	// verify unique name
	PD_Style * pStyle = NULL;
	if (getStyle(szName,&pStyle) == true)
		return false;		// duplicate name

	pStyle = new PD_BuiltinStyle(this, indexAP, szName, bDisplayed);
	if (pStyle)
		m_hashStyles.insert(std::make_pair(szName, pStyle));

	return true;
}


bool pt_PieceTable::appendStyle(const PP_PropertyVector & attributes)
{
	// this function can only be called while loading the document.
  //UT_ASSERT(m_pts==PTS_Loading);

	// first, store the attributes and properties and get an index to them.

	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return false;

	// verify unique name

	UT_ASSERT_HARMLESS(sizeof(char) == sizeof(gchar));
	const std::string & name = PP_getAttribute(PT_NAME_ATTRIBUTE_NAME, attributes);
	if (name.empty())
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return true;		// silently ignore unnamed styles
	}
	PD_Style * pStyle = NULL;
	if (getStyle(name.c_str(), &pStyle) == true)
	{
		// duplicate name
		UT_return_val_if_fail (pStyle, false);
		if (pStyle->isUserDefined())
		{
			// already loaded, ignore redefinition
			UT_DEBUGMSG(("appendStyle[%s]: duplicate definition ignored\n", name.c_str()));
			return true;
		}

		// override builtin definition
		return pStyle->setIndexAP(indexAP);
	}
	else
	{
		// this is a new name
		pStyle = new PD_Style(this, indexAP, name.c_str());

		if (pStyle) {
			m_hashStyles.insert(std::make_pair(name, pStyle));
        }

		return true;
	}
}

bool pt_PieceTable::removeStyle (const gchar * szName)
{
	UT_return_val_if_fail (szName, false);
	UT_ASSERT_HARMLESS(sizeof(char) == sizeof(gchar));

	UT_DEBUGMSG(("DOM: remove the style, maybe recode the hash-table\n"));

	PD_Style * pStyle;

	if (getStyle(szName,&pStyle))
	{
		if (!pStyle->isUserDefined())
			return false; // can't destroy a builtin style

		delete pStyle;

		m_hashStyles.erase(szName);
		return true;
	}

	return false;
}

bool pt_PieceTable::getStyle(const char * szName, PD_Style ** ppStyle) const
{
	//UT_ASSERT(szName && *szName);

	StyleMap::const_iterator iter = m_hashStyles.find(szName);
	if(iter == m_hashStyles.end()) {
		return false;
	}

	if (ppStyle)
	{
		*ppStyle = iter->second;
	}

	return true;
}

size_t pt_PieceTable::getStyleCount (void) const
{
  return (size_t) m_hashStyles.size();
}

#if 0 // currentl unused. suppress warning
///////////////////////////////////////////////////////////////////////
/*!
 * compareStyleNames this function is used to compare the char * strings names
 * of the styles with the qsort method on UT_Vector.
\param const void * vS1  - pointer to a PD_Style pointer
\param const void * vS2  - pointer to a PD_Style pointer
\returns -ve if sz1 < sz2, 0 if sz1 == sz2, +ve if sz1 > sz2
*/
static UT_sint32 compareStyleNames(const void * vS1, const void * vS2)
{
	const PD_Style ** pS1 = (const PD_Style **) vS1;
	const PD_Style ** pS2 = (const PD_Style **) vS2;
	const char * sz1 = (*pS1)->getName();
	const char * sz2 = (*pS2)->getName();
	return g_ascii_strcasecmp(sz1, sz2);
}
#endif

/*!
    Do not use this function inside loops, used the other enumStyles() instead !!!
 */
bool pt_PieceTable::enumStyles(UT_uint32 k,
							   const char ** pszName,
							   const PD_Style ** ppStyle) const
{
	// return the kth style.

	UT_uint32 kLimit = m_hashStyles.size();
	if (k >= kLimit)
		return false;

	UT_GenericVector<PD_Style*> * vStyle = NULL;
	enumStyles(vStyle);
	//vStyle->qsort(compareStyleNames);

	PD_Style * pStyle = vStyle->getNthItem(k);
	UT_return_val_if_fail (pStyle,false);

	if (ppStyle)
	{
		*ppStyle = pStyle;
	}

	if (pszName)
	{
	  *pszName = pStyle->getName();
	}
	UT_ASSERT_HARMLESS(*pszName);

	delete vStyle;

	return true;
}

/*!
    generate vector of styles
    the caller has to delete pStyle when done ...
*/
bool pt_PieceTable::enumStyles(UT_GenericVector<PD_Style*> *& pStyles) const
{
	pStyles = new UT_GenericVector<PD_Style*>;

	for(StyleMap::const_iterator iter = m_hashStyles.begin();
		iter != m_hashStyles.end(); ++iter) {
		pStyles->addItem(iter->second);
	}

	return true;
}
