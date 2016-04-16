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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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


const PP_PropertyVector PP_NOPROPS;

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
	gchar def_dom_dir[]="ltr";
	gchar default_direction[]="ltr";
	gchar text_align[]="left\0";		//the '\0' is needed so that we can copy
										//the word 'right' here
#else
	gchar def_dom_dir[]="rtl";
	gchar default_direction[]="rtl";
	gchar text_align[]="right";
#endif

// KEEP THIS ALPHABETICALLY ORDERED UNDER PENALTY OF DEATH!


/*!
 * Definitions are: Property Nme: Initial Value: Can Inherit: Pointer
 * to class : tPropLevel
 * tPropLevel should be set by or-ing the values defined in PP_Property.h
 */
static PP_Property _props[] =
{
	{ "background-color",      "transparent",     false, NULL, PP_LEVEL_SECT},
	{ "bgcolor",               "transparent",     true,  NULL, PP_LEVEL_CHAR},
	{"border-merge",           "0",               true,  NULL, PP_LEVEL_BLOCK},
	{"border-shadow-merge",    "0",               true,  NULL, PP_LEVEL_BLOCK},
	{ "bot-attach",            "",               false,  NULL, PP_LEVEL_TABLE},
	{ "bot-color",             "000000",          false, NULL, PP_LEVEL_TABLE},
	{ "bot-shadow",            "0",               false, NULL, PP_LEVEL_BLOCK},
	{ "bot-shadow-color",      "grey",            false, NULL, PP_LEVEL_BLOCK},
	{ "bot-space",             "0.02in",          false, NULL, PP_LEVEL_BLOCK},
	{ "bot-style",             "1",           false, NULL, PP_LEVEL_TABLE},
	{ "bot-thickness",         "1px",             false, NULL, PP_LEVEL_TABLE},

	{ "bounding-space",        "0.05in",          false, NULL, PP_LEVEL_FRAME},

	{ "cell-margin-bottom",   "0.002in",         false,  NULL, PP_LEVEL_TABLE},
	{ "cell-margin-left",     "0.002in",         false,  NULL, PP_LEVEL_TABLE},
	{ "cell-margin-right",    "0.002in",         false,  NULL, PP_LEVEL_TABLE},
	{ "cell-margin-top",      "0.002in",         false,  NULL, PP_LEVEL_TABLE},

	{ "color",                 "000000",          true,  NULL, PP_LEVEL_CHAR},
	{ "column-gap",	           "0.25in",          false, NULL, PP_LEVEL_SECT},
	{ "column-line",           "off",	          false, NULL, PP_LEVEL_SECT},
	{ "columns",               "1",               false, NULL, PP_LEVEL_SECT},

	{ "default-tab-interval",  "0.5in",           false, NULL, PP_LEVEL_BLOCK},
	{ "dir-override",          NULL,              true,  NULL, PP_LEVEL_CHAR},  
	{ "display",               "inline",          true,  NULL, PP_LEVEL_CHAR},
	{ "dom-dir",               def_dom_dir,       true,  NULL, PP_LEVEL_BLOCK | PP_LEVEL_SECT},  

	{ "field-color",           "dcdcdc",          true,  NULL, PP_LEVEL_FIELD},
	{ "field-font",	           "NULL",	          true,  NULL, PP_LEVEL_FIELD},
	{ "font-family",           "Times New Roman", true,  NULL, PP_LEVEL_CHAR},
	{ "font-size",	           "12pt",	          true,  NULL, PP_LEVEL_CHAR},	// MS word defaults to 10pt, but it just seems too small
	{ "font-stretch",          "normal",          true,  NULL, PP_LEVEL_CHAR},
	{ "font-style",	           "normal",          true,  NULL, PP_LEVEL_CHAR},
	{ "font-variant",          "normal",          true,  NULL, PP_LEVEL_CHAR},
	{ "font-weight",           "normal",          true,  NULL, PP_LEVEL_CHAR},
	{ "footer",                "",                false, NULL, PP_LEVEL_SECT},
	{ "footer-even",           "",                false, NULL, PP_LEVEL_SECT},
	{ "footer-first",          "",                false, NULL, PP_LEVEL_SECT},
	{ "footer-last",           "",                false, NULL, PP_LEVEL_SECT},
	{ "format",                "%*%d.",           true,  NULL, PP_LEVEL_BLOCK},

	{"frame-col-xpos",         "0.0in",           false, NULL, PP_LEVEL_FRAME},
	{"frame-col-ypos",         "0.0in",           false, NULL, PP_LEVEL_FRAME},
	{"frame-expand-height",    "0.0in",           false, NULL, PP_LEVEL_FRAME},
	{"frame-height",           "0.0in",           false, NULL, PP_LEVEL_FRAME},
	{"frame-horiz-align",      "left",            false, NULL, PP_LEVEL_FRAME},
	{"frame-min-height",       "0.0in",           false, NULL, PP_LEVEL_FRAME},
	{"frame-page-xpos",        "0.0in",           false, NULL, PP_LEVEL_FRAME},
	{"frame-page-ypos",        "0.0in",           false, NULL, PP_LEVEL_FRAME},
	{"frame-pref-column",      "0",               false, NULL, PP_LEVEL_FRAME}, 
	{"frame-pref-page",        "0",               false, NULL, PP_LEVEL_FRAME},
	{"frame-rel-width",        "0.5",             false, NULL, PP_LEVEL_FRAME},
	{"frame-type",             "textbox",         false, NULL, PP_LEVEL_FRAME},
	{"frame-width",            "0.0in",           false, NULL, PP_LEVEL_FRAME},

	{ "header",                "",                false, NULL, PP_LEVEL_SECT},
	{ "header-even",           "",                false, NULL, PP_LEVEL_SECT},
	{ "header-first",          "",                false, NULL, PP_LEVEL_SECT},
	{ "header-last",           "",                false, NULL, PP_LEVEL_SECT},
	{ "height",                "0in",             false, NULL, PP_LEVEL_CHAR},
	{ "homogeneous",           "1",               false, NULL, PP_LEVEL_CHAR},

	{ "keep-together",         "no",              false, NULL, PP_LEVEL_BLOCK},
	{ "keep-with-next",        "no",              false, NULL, PP_LEVEL_BLOCK},

	{ "lang",                  "en-US",           true,  NULL, PP_LEVEL_CHAR},
	{ "left-attach",           "",               false,  NULL, PP_LEVEL_TABLE},
	{ "left-color",            "000000",          false, NULL, PP_LEVEL_TABLE},
	{ "left-shadow",           "0",               false, NULL, PP_LEVEL_BLOCK},
	{ "left-shadow-color",     "grey",            false, NULL, PP_LEVEL_BLOCK},
	{ "left-space",            "0.02in",          false, NULL, PP_LEVEL_BLOCK},
	{ "left-style",            "1",           false, NULL, PP_LEVEL_TABLE},
	{ "left-thickness",        "1px",             false, NULL, PP_LEVEL_TABLE},

	{ "line-height",           "1.0",             false, NULL, PP_LEVEL_BLOCK},
	{ "list-decimal",          ".",               true,  NULL, PP_LEVEL_BLOCK},
	{ "list-delim",            "%L",              true,  NULL, PP_LEVEL_BLOCK},
	{ "list-style",            "None",            true,  NULL, PP_LEVEL_CHAR},
	{ "list-tag",              "0",               false, NULL, PP_LEVEL_BLOCK},

	{ "margin-bottom",         "0in",             false, NULL, PP_LEVEL_BLOCK},
	{ "margin-left",           "0in",	          false, NULL, PP_LEVEL_BLOCK},
	{ "margin-right",          "0in",             false, NULL, PP_LEVEL_BLOCK},
	{ "margin-top",	           "0in",             false, NULL, PP_LEVEL_BLOCK}, // zero to be consistent with other WPs

	{ "orphans",               "2",               false, NULL, PP_LEVEL_BLOCK}, // 2 to be consistent with widows & CSS

	{ "page-margin-bottom",	   "1in",             false, NULL, PP_LEVEL_SECT},
	{ "page-margin-footer",    "0.0in",           false, NULL, PP_LEVEL_SECT},
	{ "page-margin-header",    "0.0in",           false, NULL, PP_LEVEL_SECT},
	{ "page-margin-left",	   "1in",             false, NULL, PP_LEVEL_SECT},
	{ "page-margin-right",     "1in",             false, NULL, PP_LEVEL_SECT},
	{ "page-margin-top",       "1in",             false, NULL, PP_LEVEL_SECT},
	{ "position-to",           "block-above-text",false, NULL, PP_LEVEL_FRAME}, 
	{ "right-attach",          "",                false, NULL, PP_LEVEL_TABLE},
	{ "right-color",           "000000",          false, NULL, PP_LEVEL_TABLE},
	{ "right-shadow",          "0",               false, NULL, PP_LEVEL_BLOCK},
	{ "right-shadow-color",    "grey",            false, NULL, PP_LEVEL_BLOCK},
	{ "right-space",           "0.02in",          false, NULL, PP_LEVEL_BLOCK},
	{ "right-style",           "1",           false, NULL, PP_LEVEL_TABLE},
	{ "right-thickness",       "1px",             false, NULL, PP_LEVEL_TABLE},

	{ "section-footnote-line-thickness","0.005in",false, NULL, PP_LEVEL_SECT},
	{ "section-footnote-yoff", "0.01in",          false, NULL, PP_LEVEL_SECT},
	{ "section-max-column-height", "0in",         false, NULL, PP_LEVEL_SECT},
	{ "section-restart",       "",                false, NULL, PP_LEVEL_SECT},
	{ "section-restart-value", "",                false, NULL, PP_LEVEL_SECT},
	{ "section-space-after",   "0.25in",          false, NULL, PP_LEVEL_SECT},
	{"shading-background-color", "white",         false,NULL, PP_LEVEL_BLOCK},
	{"shading-foreground-color", "white",         false,NULL, PP_LEVEL_BLOCK},
	{"shading-pattern",          "0",             false,NULL, PP_LEVEL_BLOCK},
	{ "start-value",           "1",               true,  NULL, PP_LEVEL_BLOCK},

	{ "table-border",          "0.1in",           false, NULL, PP_LEVEL_TABLE},
	{ "table-col-spacing",     "0.03in",          false, NULL, PP_LEVEL_TABLE},
	{ "table-column-leftpos",  "0.0in",           false, NULL, PP_LEVEL_TABLE},
	{ "table-column-props",    "",                false, NULL, PP_LEVEL_TABLE},
	{ "table-line-thickness",  "0.8pt",               false, NULL, PP_LEVEL_TABLE},
	{ "table-line-type",       "1",               false, NULL, PP_LEVEL_TABLE},
	{ "table-margin-bottom",   "0.01in",           false, NULL, PP_LEVEL_TABLE},
 	{ "table-margin-left",     "0.005in",           false, NULL, PP_LEVEL_TABLE},
	{ "table-margin-right",    "0.005in",           false, NULL, PP_LEVEL_TABLE},
	{ "table-margin-top",      "0.01in",           false, NULL, PP_LEVEL_TABLE},
	{ "table-max-extra-margin","0.05",            false, NULL, PP_LEVEL_TABLE},
	{ "table-row-props",       "",                false, NULL, PP_LEVEL_TABLE},
	{ "table-row-spacing",     "0.01in",           false, NULL, PP_LEVEL_TABLE},
	{ "tabstops",              "",                false, NULL, PP_LEVEL_BLOCK},
	{ "text-align",            text_align,	      true,  NULL, PP_LEVEL_BLOCK},
	{ "text-decoration",       "none",            true,  NULL, PP_LEVEL_CHAR},
	{ "text-folded",           "0",               false, NULL, PP_LEVEL_BLOCK},
	{ "text-folded-id",        "0",               false, NULL, PP_LEVEL_BLOCK},
	{ "text-indent",           "0in",             false, NULL, PP_LEVEL_BLOCK},
	{ "text-position",         "normal",          true,  NULL, PP_LEVEL_CHAR},
	{ "text-transform",         "none",          true,  NULL, PP_LEVEL_CHAR},
	{ "toc-dest-style1",      "Contents 1"   ,   false, NULL, PP_LEVEL_BLOCK},
	{ "toc-dest-style2",      "Contents 2",      false, NULL, PP_LEVEL_BLOCK},
	{ "toc-dest-style3",      "Contents 3",      false, NULL, PP_LEVEL_BLOCK},
	{ "toc-dest-style4",      "Contents 4",      false, NULL, PP_LEVEL_BLOCK},
	{ "toc-has-heading",       "1",               false, NULL, PP_LEVEL_BLOCK},
	{ "toc-has-label1",       "1",                false, NULL, PP_LEVEL_BLOCK},
	{ "toc-has-label2",       "1",                false, NULL, PP_LEVEL_BLOCK},
	{ "toc-has-label3",       "1",                false, NULL, PP_LEVEL_BLOCK},
	{ "toc-has-label4",       "1",                false, NULL, PP_LEVEL_BLOCK},
	{ "toc-heading",       "Contents",   false, NULL, PP_LEVEL_BLOCK},
	{ "toc-heading-style",  "Contents Header",    false, NULL, PP_LEVEL_BLOCK},
    { "toc-id",                "0",               false, NULL, PP_LEVEL_SECT},
	{ "toc-indent1",           "0.5in",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-indent2",           "0.5in",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-indent3",           "0.5in",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-indent4",           "0.5in",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-after1",       "",               false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-after2",       "",               false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-after3",       "",               false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-after4",       "",               false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-before1",       "",              false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-before2",       "",              false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-before3",       "",              false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-before4",       "",              false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-inherits1",       "1",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-inherits2",       "1",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-inherits3",       "1",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-inherits4",       "1",           false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-start1",       "1",              false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-start2",       "1",              false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-start3",       "1",              false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-start4",       "1",              false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-type1",       "numeric",         false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-type2",       "numeric",         false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-type3",       "numeric",         false, NULL, PP_LEVEL_BLOCK},
	{ "toc-label-type4",       "numeric",         false, NULL, PP_LEVEL_BLOCK},
	{ "toc-page-type1",        "numeric",         false, NULL, PP_LEVEL_BLOCK},
	{ "toc-page-type2",        "numeric",         false, NULL, PP_LEVEL_BLOCK},
	{ "toc-page-type3",        "numeric",         false, NULL, PP_LEVEL_BLOCK},
	{ "toc-page-type4",        "numeric",         false, NULL, PP_LEVEL_BLOCK},
	{ "toc-range-bookmark",    "",                false, NULL, PP_LEVEL_BLOCK},
	{ "toc-source-style1",     "Heading 1",       false, NULL, PP_LEVEL_BLOCK},
	{ "toc-source-style2",     "Heading 2",       false, NULL, PP_LEVEL_BLOCK},
	{ "toc-source-style3",     "Heading 3",       false, NULL, PP_LEVEL_BLOCK},
	{ "toc-source-style4",     "Heading 4",       false, NULL, PP_LEVEL_BLOCK},
	{ "toc-tab-leader1",       "dot",             false, NULL, PP_LEVEL_BLOCK},
	{ "toc-tab-leader2",       "dot",             false, NULL, PP_LEVEL_BLOCK},
	{ "toc-tab-leader3",       "dot",             false, NULL, PP_LEVEL_BLOCK},
	{ "toc-tab-leader4",       "dot",             false, NULL, PP_LEVEL_BLOCK},

	{ "top-attach",             "",               false, NULL, PP_LEVEL_TABLE},
	{ "top-color",             "000000",          false, NULL, PP_LEVEL_TABLE},
	{ "top-shadow",            "0",               false, NULL, PP_LEVEL_BLOCK},
	{ "top-shadow-color",      "grey",            false, NULL, PP_LEVEL_BLOCK},
	{ "top-space",             "0.02in",          false, NULL, PP_LEVEL_BLOCK},
	{ "top-style",             "1",               false, NULL, PP_LEVEL_TABLE},
	{ "top-thickness",         "1px",             false, NULL, PP_LEVEL_TABLE},


	{ "vert-align",            "0",               false, NULL, PP_LEVEL_TABLE},

	{ "widows",                "2",               false, NULL, PP_LEVEL_BLOCK},
	{ "width",                 "0in",             false, NULL, PP_LEVEL_CHAR}, 
	{ "wrap-mode",             "above-text",      false, NULL, PP_LEVEL_FRAME},
	{ "xpad",                  "0.03in",          false, NULL, PP_LEVEL_FRAME},
	{ "xpos",                  "0.0in",           false, NULL, PP_LEVEL_FRAME},
	{ "ypad",                  "0.03in",          false, NULL, PP_LEVEL_FRAME}, 
	{ "ypos",                  "0.0in",           false, NULL, PP_LEVEL_FRAME} 
};

static int s_compare (const void * a, const void * b)
{
  const PP_Property * prop;
  const char * name;

  name = static_cast<const char *>(a);
  prop = static_cast<const PP_Property *>(b);

  return strcmp (name, prop->getName());
}

/*****************************************************************/

const PP_Property * PP_lookupProperty(const gchar * name)
{
	PP_Property * prop = NULL;

	prop = static_cast<PP_Property *>(bsearch (name, _props, G_N_ELEMENTS(_props), sizeof (_props[0]), s_compare));

	return prop;
}

//allows us to reset the default value for the direction settings;
void PP_resetInitialBiDiValues(const gchar * pszValue)
{
	int i;
	int count = G_N_ELEMENTS(_props);

	for (i=0; i<count; i++)
	{
		if (/*(0 == strcmp(_props[i].m_pszName, "dir"))
		  ||*/(0 == strcmp(_props[i].m_pszName, "dom-dir"))
		  /*||(0 == strcmp(_props[i].m_pszName, "column-order"))*/)
		  //this last one is not necessary since dom-dir and column-order
		  //share the same physical string
		{
			_props[i].m_pszInitial = pszValue;
		}
		else if ((0 == strcmp(_props[i].m_pszName, "text-align")))
		{
			UT_DEBUGMSG(("reseting text-align (%s)\n", pszValue));
			if(pszValue[0] == (gchar)'r') {
				_props[i].m_pszInitial = "right";
			}
			else {
				_props[i].m_pszInitial = "left";
			}
			break; //since the list is alphabetical, this is always the last one
		}
	}
}

void PP_setDefaultFontFamily(const char* pszFamily)
{
	static UT_String family(pszFamily);
	PP_Property* prop = static_cast<PP_Property*>(bsearch ("font-family", _props, G_N_ELEMENTS(_props), sizeof(_props[0]), s_compare));
	prop->m_pszInitial = const_cast<gchar*>(reinterpret_cast<const gchar*>(family.c_str()));
}

static PD_Style * _getStyle(const PP_AttrProp * pAttrProp, const PD_Document * pDoc)
{
	PD_Style * pStyle = NULL;

	const gchar * szValue = NULL;
//
// SHIT. This is where the style/name split gets really hairy. This index AP MIGHT be
// from a style definition in which case the name of the style is PT_NAME_ATTRIBUTE_NAME
// or it might be from the document in which case the attribute is
// PT_STYLE_ATTRIBUTE_NAME. Fuck it, try both. - MES.
//
	if (pAttrProp->getAttribute(PT_NAME_ATTRIBUTE_NAME, szValue))
	{
		UT_return_val_if_fail (szValue && szValue[0], NULL);
		if (pDoc)
			pDoc->getStyle(reinterpret_cast<const char*>(szValue), &pStyle);

		// NOTE: we silently fail if style is referenced, but not defined
	}
    else if(pAttrProp->getAttribute(PT_STYLE_ATTRIBUTE_NAME, szValue))
	{
		UT_return_val_if_fail (szValue && szValue[0], NULL);
		if (pDoc)
			pDoc->getStyle(reinterpret_cast<const char*>(szValue), &pStyle);

		// NOTE: we silently fail if style is referenced, but not defined
	}

	return pStyle;
}

static const gchar * s_evalProperty (const PP_Property * pProp,
										const PP_AttrProp * pAttrProp,
										const PD_Document * pDoc,
										bool bExpandStyles)
{
	const gchar * szValue = NULL;

	if (pAttrProp->getProperty (pProp->getName(), szValue))
		{
			return szValue;
		}
	if (!bExpandStyles) return NULL;

	PD_Style * pStyle = _getStyle (pAttrProp, pDoc);

	int i = 0;
	while (pStyle && (i < pp_BASEDON_DEPTH_LIMIT))
		{
			if (pStyle->getProperty (pProp->getName (), szValue))
				{
					return szValue;
				}
			pStyle = pStyle->getBasedOn ();
			i++;
		}
	return NULL;
}

const gchar * PP_evalProperty (const gchar *  pszName,
								  const PP_AttrProp * pSpanAttrProp,
								  const PP_AttrProp * pBlockAttrProp,
								  const PP_AttrProp * pSectionAttrProp,
								  const PD_Document * pDoc,
								  bool bExpandStyles)
{
	// find the value for the given property
	// by evaluating it in the contexts given.
	// use the CSS inheritance as necessary.

	if (!pszName || !*pszName)
	{
		UT_DEBUGMSG(("PP_evalProperty: null property given\n"));
		return NULL;
	}

	if (pDoc == 0) bExpandStyles = false;

	const PP_Property * pProp = PP_lookupProperty(pszName);
	if (!pProp)
	{
		UT_DEBUGMSG(("PP_evalProperty: unknown property \'%s\'\n",pszName));
		return NULL;
	}

	/* Not all properties can have a value of inherit, but we're not validating here.
	 * This is not to be confused with automatic inheritance - the difference is whether
	 * to take the default value (for when no value is specified).
	 */
	bool bInherit = false;

	// see if the property is on the Span item.

	const gchar * szValue = NULL;

	// TODO: ?? make lookup more efficient by tagging each property with scope (block, char, section)

	if (pSpanAttrProp)
	{
		szValue = s_evalProperty (pProp, pSpanAttrProp, pDoc, bExpandStyles);

		if (szValue)
			if (strcmp (szValue, "inherit") == 0)
			{
				szValue = NULL;
				bInherit = true;
			}
		if ((szValue == NULL) && (bInherit || pProp->canInherit ()))
		{
			bInherit = false;

			if (pBlockAttrProp)
			{
				szValue = s_evalProperty (pProp, pBlockAttrProp, pDoc, bExpandStyles);

				if (szValue)
					if (strcmp (szValue, "inherit") == 0)
					{
						szValue = NULL;
						bInherit = true;
					}
				if ((szValue == NULL) && (bInherit || pProp->canInherit ()))
				{
					bInherit = false;

					if (pSectionAttrProp)
					{
						szValue = s_evalProperty (pProp, pSectionAttrProp, pDoc, bExpandStyles);

						if (szValue)
							if (strcmp (szValue, "inherit") == 0)
							{
								szValue = NULL;
								bInherit = true;
							}
						if ((szValue == NULL) && (bInherit || pProp->canInherit ()))
						{
							const PP_AttrProp * pDocAP = pDoc->getAttrProp ();
							if (pDocAP)
								pDocAP->getProperty (pszName, szValue);
						}
					}
				}
			}
		}
	}
	else if (pBlockAttrProp)
	{
		szValue = s_evalProperty (pProp, pBlockAttrProp, pDoc, bExpandStyles);

		if (szValue)
			if (strcmp (szValue, "inherit") == 0)
			{
				szValue = NULL;
				bInherit = true;
			}
		if ((szValue == NULL) && (bInherit || pProp->canInherit ()))
		{
			bInherit = false;

			if (pSectionAttrProp)
			{
				szValue = s_evalProperty (pProp, pSectionAttrProp, pDoc, bExpandStyles);

				if (szValue)
					if (strcmp (szValue, "inherit") == 0)
					{
						szValue = NULL;
						bInherit = true;
					}
				if ((szValue == NULL) && (bInherit || pProp->canInherit ()))
				{
					const PP_AttrProp * pDocAP = pDoc->getAttrProp ();
					if (pDocAP)
						pDocAP->getProperty (pszName, szValue);
				}
			}
		}
	}
	else if (pSectionAttrProp)
	{
		szValue = s_evalProperty (pProp, pSectionAttrProp, pDoc, bExpandStyles);

		if (szValue)
			if (strcmp (szValue, "inherit") == 0)
			{
				szValue = NULL;
				bInherit = true;
			}
		if ((szValue == NULL) && (bInherit || pProp->canInherit ()))
		{
			const PP_AttrProp * pDocAP = pDoc->getAttrProp ();
			if (pDocAP)
				pDocAP->getProperty (pszName, szValue);
		}
	}
	else
	{
		const PP_AttrProp * pDocAP = pDoc->getAttrProp ();
		if (pDocAP)
		{
			pDocAP->getProperty (pszName, szValue);

			// dom-dir requires special treatment at document level
			if(szValue && strcmp(pszName, "dom-dir") == 0)
			{
				if(   strcmp(szValue, "logical-ltr") == 0
				   || strcmp(szValue, "logical-rtl") == 0)
					szValue += 8;
			}
		}
	}
	if (szValue)
		if (strcmp (szValue, "inherit") == 0) // shouldn't happen, but doesn't hurt to check
			szValue = NULL;

	if (szValue == NULL)
		if (bExpandStyles)
		{
			PD_Style * pStyle = 0;

			if (pDoc->getStyle ("Normal", &pStyle))
			{
				/* next to last resort -- check for this property in the Normal style
				 */
				pStyle->getProperty (pszName, szValue);

				if (szValue)
					if (strcmp (szValue, "inherit") == 0)
						szValue = NULL;
			}
		}

	if(szValue == NULL && pDoc && (bInherit || pProp->canInherit ()))
	{
		// see if the doc has a value for this prop
		const PP_AttrProp *  pAP = pDoc->getAttrProp();
		if(pAP)
		{
			pAP->getProperty(pszName, szValue);
		}
	}
	
	if (szValue == NULL)
		szValue = pProp->getInitial (); // which may itself be NULL, but that is a bad thing - FIXME!!

	return szValue;
}

const PP_PropertyType * PP_evalPropertyType(const gchar *  pszName,
								 const PP_AttrProp * pSpanAttrProp,
								 const PP_AttrProp * pBlockAttrProp,
								 const PP_AttrProp * pSectionAttrProp,
								 tProperty_type Type,
								 const PD_Document * pDoc,
								 bool bExpandStyles)
{
	// find the value for the given property
	// by evaluating it in the contexts given.
	// use the CSS inheritance as necessary.

	if (!pszName || !*pszName)
	{
		UT_DEBUGMSG(("PP_evalProperty: null property given\n"));
		return NULL;
	}

	const PP_PropertyType * p_property;
	const PP_Property * pProp = PP_lookupProperty(pszName);
	if (!pProp)
	{
		UT_DEBUGMSG(("PP_evalProperty: unknown property \'%s\'\n",pszName));
		return NULL;
	}

	PD_Style * pStyle = NULL;

	// TODO: make lookup more efficient by tagging each property with scope (block, char, section)

	// see if the property is on the Span item.

	if (pSpanAttrProp)
	{
		p_property = pSpanAttrProp->getPropertyType(pProp->getName(), Type);
		if(p_property)
			return p_property;

		if (bExpandStyles)
		{
			pStyle = _getStyle(pSpanAttrProp, pDoc);

			int i = 0;
			while (pStyle && (i < pp_BASEDON_DEPTH_LIMIT))
			{
				p_property = pStyle->getPropertyType(pProp->getName(), Type);
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
			p_property = pBlockAttrProp->getPropertyType(pProp->getName(), Type);
			if(p_property)
				return p_property;

			if (bExpandStyles)
			{
				pStyle = _getStyle(pBlockAttrProp, pDoc);

				int i = 0;
				while (pStyle && (i < pp_BASEDON_DEPTH_LIMIT))
				{
					p_property = pStyle->getPropertyType(pProp->getName(),  Type);
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
				p_property =  pSectionAttrProp->getPropertyType(pProp->getName(), Type);
				if(p_property)
					return p_property;
			}
		}
	}

	if (pDoc->getStyle("Normal", &pStyle))
	{
		// next to last resort -- check for this property in the Normal style
		p_property = pStyle->getPropertyType(pProp->getName(),  Type);
		if(p_property)
			return p_property;
	}

	// if no inheritance allowed for it or there is no
	// value set in containing block or section, we return
	// the default value for this property.

	return pProp->getInitialType(Type);
}

UT_uint32        PP_getPropertyCount()
{
	return (sizeof(_props)/sizeof(PP_Property));
}

const gchar * PP_getNthPropertyName(UT_uint32 n)
{
	return _props[n].getName();
}

tPropLevel   PP_getNthPropertyLevel(UT_uint32 n)
{
	return _props[n].getLevel();
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
		const_cast<PP_Property *>(this)->m_pProperty = PP_PropertyType::createPropertyType(Type, m_pszInitial);
	}

	return m_pProperty;
}

PP_PropertyType *PP_PropertyType::createPropertyType(tProperty_type Type, const gchar *p_init)
{
	PP_PropertyType *p_property = NULL;
	switch(Type)
	{
	case Property_type_color:
		p_property = new PP_PropertyTypeColor(p_init);
		break;

	case Property_type_bool:
		p_property = new PP_PropertyTypeBool(p_init);
		break;

	case Property_type_int:
		p_property = new PP_PropertyTypeInt(p_init);
		break;

	case Property_type_size:
		p_property = new PP_PropertyTypeSize(p_init);
		break;

	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return p_property;
}

PP_PropertyTypeColor::PP_PropertyTypeColor(const gchar *p_init)
{
	UT_parseColor(p_init, Color);
}

PP_PropertyTypeBool::PP_PropertyTypeBool(const gchar *p_init)
{
	State = (strcmp("yes", p_init) != 0);
}

PP_PropertyTypeInt::PP_PropertyTypeInt(const gchar *p_init)
{
	Value = atoi(p_init);
}

PP_PropertyTypeSize::PP_PropertyTypeSize(const gchar *p_init)
{
	Value = UT_convertDimensionless(p_init);
	Dim = UT_determineDimension(p_init);
}
