/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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


#include <string.h>
#include <stdlib.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_hash.h"
#include "ut_debugmsg.h"
#include "pp_Property.h"
#include "pp_AttrProp.h"
#include "pd_Document.h"
#include "pd_Style.h"
#include "pp_Attribute.h"

/*****************************************************************/

/*
  TODO do we want this list of last-resort default settings to be here?
  It seems out of place... --EWS
*/
/*
	Response: I agree that it seems out of place.  It's inconsistent with
				the per-class definition and organization of the properties.
				IE, the pd_Document use of dom-dir.  We don't even have a
				PP_LEVEL_DOC represented here.		-MG
*/

/*
	We need to be able to change the BiDi relevant dafault properties at runtime
	in response to the user changing the default direction in preferences.
	Therefore cannot use constants for these three, since those are stored in
	read-only segment.
*/
#ifndef BIDI_RTL_DOMINANT
	XML_Char def_dom_dir[]="ltr";
	XML_Char default_direction[]="ltr";
	XML_Char text_align[]="left\0";		//the '\0' is needed so that we can copy
										//the word 'right' here
#else
	XML_Char def_dom_dir[]="rtl";
	XML_Char default_direction[]="rtl";
	XML_Char text_align[]="right";
#endif

// KEEP THIS ALPHABETICALLY ORDERED UNDER PENALTY OF DEATH!


/*!
 * Definitions are: Property Nme: Initial Value: Can Inherit: Pointer
 * to class : tPropLevel
 * tPropLevel should be set by or-ing the values defined in PP_Property.h
 */
static PP_Property _props[] =
{
	{ "", NULL, false, NULL, 0}, /* dummy value to allow index 0 as NULL */
	{ "background-color",      "transparent",  false, NULL, PP_LEVEL_SECT},
	{ "background-image",      NULL,           false, NULL, PP_LEVEL_BLOCK},
	{ "bgcolor",               "transparent",  true,  NULL, PP_LEVEL_CHAR},
	{ "bot-attach",            "",             false, NULL, PP_LEVEL_TABLE},
	{ "bot-color",             "000000",       false, NULL, PP_LEVEL_TABLE},
	{ "bot-style",             "1",            false, NULL, PP_LEVEL_TABLE},
	{ "bot-thickness",         "1px",          false, NULL, PP_LEVEL_TABLE},

	{ "bounding-space",        "0.05in",       false, NULL, PP_LEVEL_FRAME},

	{ "cell-margin-bottom",   "0.002in",       false, NULL, PP_LEVEL_TABLE},
	{ "cell-margin-left",     "0.002in",       false, NULL, PP_LEVEL_TABLE},
	{ "cell-margin-right",    "0.002in",       false, NULL, PP_LEVEL_TABLE},
	{ "cell-margin-top",      "0.002in",       false, NULL, PP_LEVEL_TABLE},

	{ "color",                 "000000",       true,  NULL, PP_LEVEL_CHAR},
	{ "column-gap",	           "0.25in",       false, NULL, PP_LEVEL_SECT},
	{ "column-line",           "off",	       false, NULL, PP_LEVEL_SECT},
	{ "columns",               "1",            false, NULL, PP_LEVEL_SECT},

	{ "default-tab-interval",  "0.5in",        false, NULL, PP_LEVEL_BLOCK},
	{ "dir-override",          NULL,           true,  NULL, PP_LEVEL_CHAR},  
	{ "display",               "inline",       true,  NULL, PP_LEVEL_CHAR},

	{ "document-endnote-initial", "1",         false, NULL, PP_LEVEL_SECT},
	{ "document-endnote-place_enddoc",    NULL,false, NULL, PP_LEVEL_SECT},
	{ "document-endnote-place_endsection",NULL,false, NULL, PP_LEVEL_SECT},
	{ "document-endnote-restart_section", NULL,false, NULL, PP_LEVEL_SECT},
	{ "document-endnote-type",            NULL,false, NULL, PP_LEVEL_SECT},
	{ "document-footnote-initial",        NULL,false, NULL, PP_LEVEL_SECT},
	{ "document-footnote-restart_page",   NULL,false, NULL, PP_LEVEL_SECT},
	{ "document-footnote-restart_section",NULL,false, NULL, PP_LEVEL_SECT},
	{ "document-footnote-type",           NULL,false, NULL, PP_LEVEL_SECT},

	{ "dom-dir",               def_dom_dir,    true,  NULL, PP_LEVEL_BLOCK | PP_LEVEL_SECT},  

	{ "field-color",           "dcdcdc",       true,  NULL, PP_LEVEL_FIELD},
	{ "field-font",	           "NULL",	       true,  NULL, PP_LEVEL_FIELD},
	{ "font-family",           "Times New Roman",true,NULL, PP_LEVEL_CHAR},
	{ "font-size",	           "12pt",	       true,  NULL, PP_LEVEL_CHAR},	// MS word defaults to 10pt, but it just seems too small
	{ "font-stretch",          "normal",       true,  NULL, PP_LEVEL_CHAR},
	{ "font-style",	           "normal",       true,  NULL, PP_LEVEL_CHAR},
	{ "font-variant",          "normal",       true,  NULL, PP_LEVEL_CHAR},
	{ "font-weight",           "normal",       true,  NULL, PP_LEVEL_CHAR},
	{ "footer",                "",             false, NULL, PP_LEVEL_SECT},
	{ "footer-even",           "",             false, NULL, PP_LEVEL_SECT},
	{ "footer-first",          "",             false, NULL, PP_LEVEL_SECT},
	{ "footer-last",           "",             false, NULL, PP_LEVEL_SECT},
	{ "format",                "%*%d.",        true,  NULL, PP_LEVEL_BLOCK},

	{"frame-col-xpos",         "0.0in",        false, NULL, PP_LEVEL_FRAME},
	{"frame-col-ypos",         "0.0in",        false, NULL, PP_LEVEL_FRAME},
	{"frame-height",           "0.0in",        false, NULL, PP_LEVEL_FRAME},
	{"frame-page-xpos",        "0.0in",        false, NULL, PP_LEVEL_FRAME},
	{"frame-page-ypos",        "0.0in",        false, NULL, PP_LEVEL_FRAME},
	{"frame-position-to",      "block-above-text",false, NULL, PP_LEVEL_FRAME},
	{"frame-type",             "textbox",      false, NULL, PP_LEVEL_FRAME},
	{"frame-width",            "0.0in",        false, NULL, PP_LEVEL_FRAME},
	{"frame-xpos",             "0.0in",        false, NULL, PP_LEVEL_FRAME},
	{"frame-ypos",             "0.0in",        false, NULL, PP_LEVEL_FRAME},

	{ "header",                "",             false, NULL, PP_LEVEL_SECT},
	{ "header-even",           "",             false, NULL, PP_LEVEL_SECT},
	{ "header-first",          "",             false, NULL, PP_LEVEL_SECT},
	{ "header-last",           "",             false, NULL, PP_LEVEL_SECT},
	{ "height",                "0in",          false, NULL, PP_LEVEL_CHAR},
	{ "homogeneous",           "1",            false, NULL, PP_LEVEL_CHAR},

	{ "keep-together",         "no",           false, NULL, PP_LEVEL_BLOCK},
	{ "keep-with-next",        "no",           false, NULL, PP_LEVEL_BLOCK},

	{ "lang",                  "en-US",        true,  NULL, PP_LEVEL_CHAR},
	{ "left-attach",           "",             false, NULL, PP_LEVEL_TABLE},
	{ "left-color",            "000000",       false, NULL, PP_LEVEL_TABLE},
	{ "left-style",            "1",            false, NULL, PP_LEVEL_TABLE},
	{ "left-thickness",        "1px",          false, NULL, PP_LEVEL_TABLE},
	{ "line-height",           "1.0",          false, NULL, PP_LEVEL_BLOCK},
	{ "list-decimal",          ".",            true,  NULL, PP_LEVEL_BLOCK},
	{ "list-delim",            "%L",           true,  NULL, PP_LEVEL_BLOCK},
	{ "list-style",            "None",         true,  NULL, PP_LEVEL_CHAR},
	{ "list-tag",              "0",            false, NULL, PP_LEVEL_BLOCK},

	{ "margin-bottom",         "0in",          false, NULL, PP_LEVEL_BLOCK},
	{ "margin-left",           "0in",	       false, NULL, PP_LEVEL_BLOCK},
	{ "margin-right",          "0in",          false, NULL, PP_LEVEL_BLOCK},
	{ "margin-top",	           "0in",          false, NULL, PP_LEVEL_BLOCK}, // zero to be consistent with other WPs

	{ "orphans",               "2",            false, NULL, PP_LEVEL_BLOCK}, // 2 to be consistent with widows & CSS

	{ "page-margin-bottom",	   "1in",          false, NULL, PP_LEVEL_SECT},
	{ "page-margin-footer",    "0.0in",        false, NULL, PP_LEVEL_SECT},
	{ "page-margin-header",    "0.0in",        false, NULL, PP_LEVEL_SECT},
	{ "page-margin-left",	   "1in",          false, NULL, PP_LEVEL_SECT},
	{ "page-margin-right",     "1in",          false, NULL, PP_LEVEL_SECT},
	{ "page-margin-top",       "1in",          false, NULL, PP_LEVEL_SECT},

	{ "right-attach",         "",              false, NULL, PP_LEVEL_TABLE},
	{ "right-color",           "000000",       false, NULL, PP_LEVEL_TABLE},
	{ "right-style",           "1",            false, NULL, PP_LEVEL_TABLE},
	{ "right-thickness",       "1px",          false, NULL, PP_LEVEL_TABLE},

	{ "section-footnote-line-thickness","0.005in",false, NULL, PP_LEVEL_SECT},
	{ "section-footnote-yoff", "0.01in",       false, NULL, PP_LEVEL_SECT},
	{ "section-max-column-height", "0in",      false, NULL, PP_LEVEL_SECT},
	{ "section-restart",       "",             false, NULL, PP_LEVEL_SECT},
	{ "section-restart-value", "",             false, NULL, PP_LEVEL_SECT},
	{ "section-space-after",   "0.25in",       false, NULL, PP_LEVEL_SECT},
	{ "start-value",           "1",            true,  NULL, PP_LEVEL_BLOCK},

	{ "table-border",          "0.1in",        false, NULL, PP_LEVEL_TABLE},
	{ "table-col-spacing",     "0.03in",       false, NULL, PP_LEVEL_TABLE},
	{ "table-column-leftpos",  "0.0in",        false, NULL, PP_LEVEL_TABLE},
	{ "table-column-props",    "",             false, NULL, PP_LEVEL_TABLE},
	{ "table-line-thickness",  "0.8pt",        false, NULL, PP_LEVEL_TABLE},
	{ "table-line-type",       "1",            false, NULL, PP_LEVEL_TABLE},
	{ "table-margin-bottom",   "0.01in",       false, NULL, PP_LEVEL_TABLE},
 	{ "table-margin-left",     "0.005in",      false, NULL, PP_LEVEL_TABLE},
	{ "table-margin-right",    "0.005in",      false, NULL, PP_LEVEL_TABLE},
	{ "table-margin-top",      "0.01in",       false, NULL, PP_LEVEL_TABLE},
	{ "table-row-props",       "",             false, NULL, PP_LEVEL_TABLE},
	{ "table-row-spacing",     "0.01in",       false, NULL, PP_LEVEL_TABLE},
	{ "tabstops",              "",             false, NULL, PP_LEVEL_BLOCK},
	{ "text-align",            text_align,	   true,  NULL, PP_LEVEL_BLOCK},
	{ "text-decoration",       "none",         true,  NULL, PP_LEVEL_CHAR},
	{ "text-folded",           "0",            false, NULL, PP_LEVEL_BLOCK},
	{ "text-folded-id",        "0",            false, NULL, PP_LEVEL_BLOCK},
	{ "text-indent",           "0in",          false, NULL, PP_LEVEL_BLOCK},
	{ "text-position",         "normal",       true,  NULL, PP_LEVEL_CHAR},
	{ "tight-wrap",           NULL,            false, NULL, PP_LEVEL_BLOCK},
	{ "toc-dest-style1",      "Contents 1",    false, NULL, PP_LEVEL_BLOCK},
	{ "toc-dest-style2",      "Contents 2",    false, NULL, PP_LEVEL_BLOCK},
	{ "toc-dest-style3",      "Contents 3",    false, NULL, PP_LEVEL_BLOCK},
	{ "toc-dest-style4",      "Contents 4",    false, NULL, PP_LEVEL_BLOCK},
	{ "toc-has-heading",       "1",            false, NULL, PP_LEVEL_BLOCK},
	{ "toc-has-label1",       "1",             false, NULL, PP_LEVEL_BLOCK},
	{ "toc-has-label2",       "1",             false, NULL, PP_LEVEL_BLOCK},
	{ "toc-has-label3",       "1",             false, NULL, PP_LEVEL_BLOCK},
	{ "toc-has-label4",       "1",             false, NULL, PP_LEVEL_BLOCK},
	{ "toc-heading",       "Table of Contents",false, NULL, PP_LEVEL_BLOCK},
	{ "toc-heading-style",  "Contents Header", false, NULL, PP_LEVEL_BLOCK},
    { "toc-id",                "0",            false, NULL, PP_LEVEL_SECT},
	{ "toc-indent1",           "0.5in",        false, NULL, PP_LEVEL_BLOCK},
	{ "toc-indent2",           "0.5in",        false, NULL, PP_LEVEL_BLOCK},
	{ "toc-indent3",           "0.5in",        false, NULL, PP_LEVEL_BLOCK},
	{ "toc-indent4",           "0.5in",        false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-after1",       "",            false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-after2",       "",            false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-after3",       "",            false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-after4",       "",            false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-before1",       "",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-before2",       "",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-before3",       "",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-before4",       "",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-inherits1",       "1",        false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-inherits2",       "1",        false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-inherits3",       "1",        false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-inherits4",       "1",        false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-start1",       "1",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-start2",       "1",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-start3",       "1",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-start4",       "1",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-type1",       "numeric",      false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-type2",       "numeric",      false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-type3",       "numeric",      false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-type4",       "numeric",      false, NULL, PP_LEVEL_BLOCK},
	{ "toc-page-type1",        "numeric",      false, NULL, PP_LEVEL_BLOCK},
	{ "toc-page-type2",        "numeric",      false, NULL, PP_LEVEL_BLOCK},
	{ "toc-page-type3",        "numeric",      false, NULL, PP_LEVEL_BLOCK},
	{ "toc-page-type4",        "numeric",      false, NULL, PP_LEVEL_BLOCK},
	{ "toc-range-bookmark",    "",             false, NULL, PP_LEVEL_BLOCK},
	{ "toc-source-style1",     "Heading 1",    false, NULL, PP_LEVEL_BLOCK},
	{ "toc-source-style2",     "Heading 2",    false, NULL, PP_LEVEL_BLOCK},
	{ "toc-source-style3",     "Heading 3",    false, NULL, PP_LEVEL_BLOCK},
	{ "toc-source-style4",     "Heading 4",    false, NULL, PP_LEVEL_BLOCK},
	{ "toc-tab-leader1",       "dot",          false, NULL, PP_LEVEL_BLOCK},
	{ "toc-tab-leader2",       "dot",          false, NULL, PP_LEVEL_BLOCK},
	{ "toc-tab-leader3",       "dot",          false, NULL, PP_LEVEL_BLOCK},
	{ "toc-tab-leader4",       "dot",          false, NULL, PP_LEVEL_BLOCK},

	{ "top-attach",             "",            false, NULL, PP_LEVEL_TABLE},
	{ "top-color",             "000000",       false, NULL, PP_LEVEL_TABLE},
	{ "top-style",             "1",            false, NULL, PP_LEVEL_TABLE},
	{ "top-thickness",         "1px",          false, NULL, PP_LEVEL_TABLE},

	{ "widows",                "2",            false, NULL, PP_LEVEL_BLOCK},
	{ "width",                 "0in",          false, NULL, PP_LEVEL_CHAR}, 
	{ "wrap-mode",             "above-text",   false, NULL, PP_LEVEL_FRAME} 
};

#ifdef __MRC__
extern "C"
#endif
static int s_compare (const void * a, const void * b)
{
  const PP_Property * prop;
  const char * name;

  name = static_cast<const char *>(a);
  prop = static_cast<const PP_Property *>(b);

  return UT_strcmp (name, prop->getName());
}

/*****************************************************************/

const PP_Property * PP_Property::lookupProperty(const XML_Char * name)
{
	PP_Property * prop = NULL;

	prop = static_cast<PP_Property *>(bsearch (name, _props, NrElements(_props), sizeof (_props[0]), s_compare));

	return prop;
}

PP_Property * PP_Property::_getProperty (PT_Property i)
{
	UT_return_val_if_fail( isIndexValid (i), NULL );
	return &_props[i];
}

const PP_Property * PP_Property::getProperty (PT_Property i)
{
	return _getProperty(i);
}

inline bool PP_Property::isIndexValid (PT_Property i)
{
	return ((UT_uint32)i < NrElements(_props));
}


//allows us to reset the default value for the direction settings;
void PP_Property::resetInitialBiDiValues(const XML_Char * pszValue)
{
	PP_Property * pProp;

	pProp = _getProperty (abi_dom_dir);

	if (pProp)
		UT_XML_strncpy(pProp->m_pszInitial, 3,pszValue);

	pProp = _getProperty (abi_text_align);

	if (pProp)
	{
		UT_DEBUGMSG(("reseting text-align (%s)\n", pszValue));
		if(pszValue[0] == (XML_Char)'r')
			UT_XML_strncpy(pProp->m_pszInitial, 5,"right");
		else
			UT_XML_strncpy(pProp->m_pszInitial, 4,"left");
	}
}

void PP_Property::setDefaultFontFamily(const char* pszFamily)
{
	static UT_UTF8String family(pszFamily);
	PP_Property * pProp = _getProperty(abi_font_family);

	if (pProp)
		pProp->m_pszInitial =
			const_cast<XML_Char*>(reinterpret_cast<const XML_Char*>(family.utf8_str()));
}

static PD_Style * _getStyle(const PP_AttrProp * pAttrProp, PD_Document * pDoc)
{
	PD_Style * pStyle = NULL;

	GQuark value;
//
// SHIT. This is where the style/name split gets really hairy. This index AP
// MIGHT be from a style definition in which case the name of the style is
// PT_NAME_ATTRIBUTE_NAME or it might be from the document in which case the
// attribute is PT_STYLE_ATTRIBUTE_NAME. Fuck it, try both. - MES.
//
	// PTFIXME
	if (pAttrProp->getAttribute(PT_NAME_ATTRIBUTE_NAME, value))
	{
		UT_return_val_if_fail (value, NULL);
		if (pDoc)
			pDoc->getStyle(value, &pStyle);

		// NOTE: we silently fail if style is referenced, but not defined
	}
    else if(pAttrProp->getAttribute(PT_STYLE_ATTRIBUTE_NAME, value))
	{
		UT_return_val_if_fail (value, NULL);
		if (pDoc)
			pDoc->getStyle(value, &pStyle);

		// NOTE: we silently fail if style is referenced, but not defined
	}

	return pStyle;
}

static GQuark s_evalProperty (const PP_Property * pProp,
							  const PP_AttrProp * pAttrProp,
							  PD_Document * pDoc,
							  bool bExpandStyles)
{
	GQuark value;
	
	if (pAttrProp->getProperty (pProp->getIndex(), value))
	{
		return value;
	}
	
	if (!bExpandStyles)
		return 0;

	PD_Style * pStyle = _getStyle (pAttrProp, pDoc);

	int i = 0;
	while (pStyle && (i < pp_BASEDON_DEPTH_LIMIT))
	{
		if (pStyle->getProperty (pProp->getIndex(), value))
		{
			return value;
		}
		pStyle = pStyle->getBasedOn ();
		i++;
	}
	return 0;
}

GQuark PP_Property::evalProperty (PT_Property i,
								  const PP_AttrProp * pSpanAttrProp,
								  const PP_AttrProp * pBlockAttrProp,
								  const PP_AttrProp * pSectionAttrProp,
								  PD_Document * pDoc,
								  bool bExpandStyles)
{
	// find the value for the given property
	// by evaluating it in the contexts given.
	// use the CSS inheritance as necessary.

	if ((UT_uint32)i >= NrElements(_props))
	{
		UT_DEBUGMSG(("PP_evalProperty: null property given\n"));
		return 0;
	}

	if (pDoc == 0) bExpandStyles = false;

	const PP_Property * pProp = getProperty(i);
	if (!pProp)
	{
		UT_DEBUGMSG(("PP_evalProperty: unknown property\n"));
		return 0;
	}

	/* Not all properties can have a value of inherit, but we're not validating
	 * here.  This is not to be confused with automatic inheritance - the
	 * difference is whether to take the default value (for when no value is
	 * specified).
	 */
	bool bInherit = false;

	// see if the property is on the Span item.

	GQuark value;
	
	// TODO: ?? make lookup more efficient by tagging each property with scope
	// (block, char, section)

	if (pSpanAttrProp)
	{
		value = s_evalProperty (pProp, pSpanAttrProp, pDoc, bExpandStyles);

		if (value == commonToQuark(PP_COMMON_inherit))
		{
			value = 0;
			bInherit = true;
		}
		if (value == 0 && (bInherit || pProp->canInherit ()))
		{
			bInherit = false;

			if (pBlockAttrProp)
			{
				value = s_evalProperty (pProp, pBlockAttrProp, pDoc, bExpandStyles);

				if (value == commonToQuark(PP_COMMON_inherit))
				{
					value = 0;
					bInherit = true;
				}
				if ((value == 0) && (bInherit || pProp->canInherit ()))
				{
					bInherit = false;

					if (pSectionAttrProp)
					{
						value = s_evalProperty (pProp, pSectionAttrProp,
												pDoc, bExpandStyles);

						if (value == commonToQuark(PP_COMMON_inherit))
						{
							value = 0;
							bInherit = true;
						}
						if ((value == 0) && (bInherit || pProp->canInherit ()))
						{
							const PP_AttrProp * pDocAP = pDoc->getAttrProp ();
							if (pDocAP)
								pDocAP->getProperty (i, value);
						}
					}
				}
			}
		}
	}
	else if (pBlockAttrProp)
	{
		value = s_evalProperty (pProp, pBlockAttrProp, pDoc, bExpandStyles);

		if (value == commonToQuark(PP_COMMON_inherit))
		{
			value = 0;
			bInherit = true;
		}
		if ((value == 0) && (bInherit || pProp->canInherit ()))
		{
			bInherit = false;

			if (pSectionAttrProp)
			{
				value = s_evalProperty (pProp, pSectionAttrProp, pDoc, bExpandStyles);

				if (value == commonToQuark(PP_COMMON_inherit))
				{
					value = 0;
					bInherit = true;
				}
				if ((value == 0) && (bInherit || pProp->canInherit ()))
				{
					const PP_AttrProp * pDocAP = pDoc->getAttrProp ();
					if (pDocAP)
						pDocAP->getProperty (i, value);
				}
			}
		}
	}
	else if (pSectionAttrProp)
	{
		value = s_evalProperty (pProp, pSectionAttrProp, pDoc, bExpandStyles);

		if (value == commonToQuark(PP_COMMON_inherit))
		{
			value = 0;
			bInherit = true;
		}
		if ((value == 0) && (bInherit || pProp->canInherit ()))
		{
			const PP_AttrProp * pDocAP = pDoc->getAttrProp ();
			if (pDocAP)
				pDocAP->getProperty (i, value);
		}
	}
	else
	{
		const PP_AttrProp * pDocAP = pDoc->getAttrProp ();
		if (pDocAP)
		{
			pDocAP->getProperty (i, value);

			// dom-dir requires special treatment at document level
			if(i == abi_dom_dir)
			{
				if (value == commonToQuark(PP_COMMON_logical_ltr))
					value = commonToQuark(PP_COMMON_ltr);
				
				if (value == commonToQuark(PP_COMMON_logical_rtl))
					value = commonToQuark(PP_COMMON_rtl);
			}
		}
	}

	if (value == commonToQuark(PP_COMMON_inherit))
			value = 0;

	if (value == 0)
		if (bExpandStyles)
		{
			PD_Style * pStyle = 0;

			if (pDoc->getStyle (commonToQuark(PP_COMMON_Normal),
								&pStyle))
			{
				/* next to last resort -- check for this property in the Normal
				 * style
				 */
				pStyle->getProperty (i, value);

				if (value == commonToQuark(PP_COMMON_inherit))
						value = 0;
			}
		}

	if(value == 0 && pDoc && (bInherit || pProp->canInherit ()))
	{
		// see if the doc has a value for this prop
		const PP_AttrProp *  pAP = pDoc->getAttrProp();
		if(pAP)
		{
			pAP->getProperty(i, value);
		}
	}
	
	if (value == 0 && pProp->getInitial())
	{
		value = g_quark_from_static_string (pProp->getInitial ());
	}
	

	return value;
}

const PP_PropertyType * PP_Property::evalPropertyType(PT_Property i,
											const PP_AttrProp * pSpanAttrProp,
											const PP_AttrProp * pBlockAttrProp,
											const PP_AttrProp * pSectionAttrProp,
											tProperty_type Type,
											PD_Document * pDoc,
											bool bExpandStyles)
{
	// find the value for the given property
	// by evaluating it in the contexts given.
	// use the CSS inheritance as necessary.

	if ((UT_uint32)i >= NrElements (_props))
	{
		UT_DEBUGMSG(("PP_evalProperty: null property given\n"));
		return NULL;
	}

	const PP_PropertyType * p_property;
	const PP_Property * pProp = getProperty(i);
	if (!pProp)
	{
		UT_DEBUGMSG(("PP_evalProperty: unknown property\n"));
		return NULL;
	}

	PD_Style * pStyle = NULL;

	// TODO: make lookup more efficient by tagging each property with scope
	// (block, char, section)

	// see if the property is on the Span item.

	if (pSpanAttrProp)
	{
		p_property = pSpanAttrProp->getPropertyType(pProp->getIndex(), Type);
		if(p_property)
			return p_property;

		if (bExpandStyles)
		{
			pStyle = _getStyle(pSpanAttrProp, pDoc);

			int i = 0;
			while (pStyle && (i < pp_BASEDON_DEPTH_LIMIT))
			{
				p_property = pStyle->getPropertyType(pProp->getIndex(), Type);
				if(p_property)
					return p_property;

				pStyle = pStyle->getBasedOn();
				i++;
			}
		}
	}

	// otherwise, see if we can inherit it from the containing block or the section.

	if (!pSpanAttrProp || pProp->canInherit())
	{
		if (pBlockAttrProp)
		{
			p_property = pBlockAttrProp->getPropertyType(pProp->getIndex(), Type);
			if(p_property)
				return p_property;

			if (bExpandStyles)
			{
				pStyle = _getStyle(pBlockAttrProp, pDoc);

				int i = 0;
				while (pStyle && (i < pp_BASEDON_DEPTH_LIMIT))
				{
					p_property = pStyle->getPropertyType(pProp->getIndex(),  Type);
					if(p_property)
						return p_property;

					pStyle = pStyle->getBasedOn();
					i++;
				}
			}
		}

		if (!pBlockAttrProp || pProp->canInherit())
		{
			if (pSectionAttrProp)
			{
				p_property =
					pSectionAttrProp->getPropertyType(pProp->getIndex(), Type);
				
				if(p_property)
					return p_property;
			}
		}
	}

	if (pDoc->getStyle(commonToQuark(PP_COMMON_Normal), &pStyle))
	{
		// next to last resort -- check for this property in the Normal style
		p_property = pStyle->getPropertyType(pProp->getIndex(),  Type);
		if(p_property)
			return p_property;
	}

	// if no inheritance allowed for it or there is no
	// value set in containing block or section, we return
	// the default value for this property.

	return pProp->getInitialType(Type);
}

PP_Property::~PP_Property()
{
	DELETEP(m_pProperty);
}


const PP_PropertyType *	PP_Property::getInitialType(tProperty_type Type) const
{
	if(!m_pProperty)
	{
		// TODO:: This is never freed.
		const_cast<PP_Property *>(this)->m_pProperty =
			PP_PropertyType::createPropertyType(Type,
								g_quark_from_static_string (m_pszInitial));
	}

	return m_pProperty;
}

PP_PropertyType *PP_PropertyType::createPropertyType(tProperty_type Type,
													 GQuark init)
{
	PP_PropertyType *pProperty = NULL;
	const XML_Char * pInit = (const XML_Char*) g_quark_to_string (init);

	UT_return_val_if_fail( pInit, NULL );
	switch(Type)
	{
		case Property_type_color:
			pProperty = new PP_PropertyTypeColor(pInit);
			break;

		case Property_type_bool:
			pProperty = new PP_PropertyTypeBool(pInit);
			break;

		case Property_type_int:
			pProperty = new PP_PropertyTypeInt(pInit);
			break;

		case Property_type_size:
			pProperty = new PP_PropertyTypeSize(pInit);
			break;

		default:
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			break;
	}

	return pProperty;
}

PP_PropertyTypeColor::PP_PropertyTypeColor(const XML_Char *p_init)
{
	UT_parseColor(p_init, Color);
}

PP_PropertyTypeBool::PP_PropertyTypeBool(const XML_Char *p_init)
{
	State = (UT_strcmp("yes", p_init) != 0);
}

PP_PropertyTypeInt::PP_PropertyTypeInt(const XML_Char *p_init)
{
	Value = atoi(p_init);
}

PP_PropertyTypeSize::PP_PropertyTypeSize(const XML_Char *p_init)
{
	Value = UT_convertDimensionless(p_init);
	Dim = UT_determineDimension(p_init);
}

/* common property (and attribute) value strings */
/* turn common attr and property values into GQuarks accessible via enum */
typedef struct 
{
    const XML_Char * m_pszName;
    GQuark           m_quark;
} _common_ap_values;

static _common_ap_values _common [] = {
    { "Times New Roman", 0},
    { "Arial",           0},
    { "None",            0},
    { "Normal",          0},
    { "Heading 1",       0},
    { "Heading 2",       0},
    { "Heading 3",       0},
    { "Heading 4",       0},
    { "Heading 5",       0},
    { "Heading 6",       0},
    { "Heading 7",       0},
    { "Heading 8",       0},
    { "Heading 9",       0},
    { "c",               0},
    { "C",               0},
    { "",                0},
    { "inherit",         0},
    { "logical-ltr",     0},
    { "logical-rtl",     0},
    { "ltr",             0},
    { "rtl",             0},
    { "dir-override:ltr",0},
    { "dir-override:rtl",0},
    { "dir-override:",   0},
    { "locked",          0},
    { "unlocked",        0},
    { "Current Settings",0},
    { "numeric",         0},
    { "A4",              0},
    { "Custom",          0},
    { "mm",              0},
    { "cm",              0},
    { "in",              0},
    { "landscape",       0},
    { "portrait",        0},
    { "app_ver", 0},
    { "app_id", 0},
    { "app_options", 0},
    { "app_target", 0},
    { "app_compiledate", 0},
    { "app_compiletime", 0},
    { "char_count", 0},
    { "date", 0},
    { "date_mmddyy", 0},
    { "date_ddmmyy", 0},
    { "date_mdy", 0},
    { "date_mthdy", 0},
    { "date_dfl", 0},
    { "date_ntdfl", 0},
    { "date_wkday", 0},
    { "date_doy", 0},
    { "datetime_custom", 0},
    { "endnote_ref", 0},
    { "endnote_anchor", 0},
    { "file_name", 0},
    { "footnote_ref", 0},
    { "footnote_anchor", 0},
    { "list_label", 0},
    { "line_count", 0},
    { "mail_merge", 0},
    { "meta_title", 0},
    { "meta_creator", 0},
    { "meta_subject", 0},
    { "meta_publisher", 0},
    { "meta_date", 0},
    { "meta_type", 0},
    { "meta_language", 0},
    { "meta_rights", 0},
    { "meta_keywords", 0},
    { "meta_contributor", 0},
    { "meta_coverage", 0},
    { "meta_description", 0},
    { "martin_test", 0},
    { "nbsp_count", 0},
    { "page_number", 0},
    { "page_count", 0},
    { "para_count", 0},
    { "page_ref", 0},
    { "sum_rows", 0},
    { "sum_cols", 0},
    { "test", 0},
    { "time", 0},
    { "time_miltime", 0},
    { "time_ampm", 0},
    { "time_zone", 0},
    { "time_epoch", 0},
    { "word_count", 0},
    { "end", 0},
};

void PP_Property::initCommonValues ()
{
	static bool bInitDone = false;

	if(!bInitDone)
	{
		/* skip the dummy null entry */
		for (UT_uint32 i = 1; i < NrElements(_common); i++)
		{
			_common[i].m_quark =
				g_quark_from_static_string (_common[i].m_pszName);
		}

		bInitDone = true;
	}
	
}

GQuark PP_Property::commonToQuark (PP_CommonValue i)
{
    if((UT_uint32)i >= NrElements(_common))
       return 0;

    return _common[i].m_quark;
}

GQuark PP_getProperty (PT_Property name, const PT_PropertyPair *props)
{
	UT_return_val_if_fail( props, 0 );

	const PT_PropertyPair * p = props;

	while (p->p)
	{
		if (p->p == name)
		    break;
		p++;
	}

	if (p->p)
		return p->p;
	else
		return 0;
}
    

