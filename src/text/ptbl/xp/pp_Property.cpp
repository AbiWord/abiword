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

/*****************************************************************/

/*
  TODO do we want this list of last-resort default settings to be here?
  It seems out of place... --EWS
*/
/*
	We need to be able to change the BiDi relevant dafault properties at runtime
	in response to the user changing the default direction in preferences.
	Therefore cannot use constants for these three, since those are stored in
	read-only segment.
*/
#ifdef BIDI_ENABLED
	#ifndef BIDI_RTL_DOMINANT
	XML_Char default_dominant_direction[]="ltr";
	XML_Char default_direction[]="ltr";
	XML_Char text_align[]="left\0";		//the '\0' is needed so that we can copy
										//the word 'right' here
	#else
	XML_Char default_dominant_direction[]="rtl";
	XML_Char default_direction[]="rtl";
	XML_Char text_align[]="right";
	#endif
	
#endif

// KEEP THIS ALPHABETICALLY ORDERED UNDER PENALTY OF DEATH!

static PP_Property _props[] =
{
	{ "background-color", "ffffff", false, NULL},
	{ "bgcolor", "transparent", true, NULL},

	{ "color",   "000000", true, NULL},
	{ "column-gap",	"0.25in", false, NULL},
	{ "column-line", "off",	false, NULL},
	{ "columns", "1", false, NULL},

	{ "default-tab-interval",  "0.5in", false, NULL},

#ifdef BIDI_ENABLED
	/*	these two stand for "direction" and "dominant direction"; they were
		intentionally abreviated, because the direction property has to be
		set basically for each word and each chunk of whitespace, inflating
		the ABW file
	*/
	{ "dir", default_direction,              true, NULL},  //the direction of the present text, prossible values ltr, rtl,ntrl	
	{ "dir-override", "off",              true, NULL},  //the direction of the present text, prossible values ltr, rtl,ntrl	
	{ "dom-dir", default_dominant_direction,              false, NULL},  //added by #TF, dominant direction of writing in a paragraph, can be either ltr or rtl (i.e., left-to-right, right-to-left)
#endif

	{ "field-color", "dcdcdc", true, NULL},
	{ "field-font",	"NULL",	true, NULL},	
	{ "font-family", "Times New Roman", true, NULL},	// TODO this is Win32-specific.  must fix!
	{ "font-size",	"12pt",	true, NULL},	// MS word defaults to 10pt, but it just seems too small
	{ "font-stretch", "normal", true, NULL},
	{ "font-style",	"normal", true, NULL},
	{ "font-variant", "normal", true, NULL},
	{ "font-weight", "normal", true, NULL},
	{ "format","%*%d.", true, NULL},

	{ "height", "",	false, NULL},

	{ "keep-together", "", false, NULL},
	{ "keep-with-next", "",	false, NULL},

	{ "lang", "en-US", true, NULL},
	
	{ "line-height", "1.0", false, NULL},
	{ "list-decimal", ".", true, NULL},
	{ "list-delim", "%L", true, NULL},

	{ "margin-bottom", "0in", false, NULL},
	{ "margin-left", "0in",	false, NULL},
	{ "margin-right", "0in", false, NULL},
	{ "margin-top",	"0in", false, NULL}, // zero to be consistent with other WPs

	{ "orphans", "1", false, NULL},

	{ "page-margin-bottom",		"1in",				false, NULL},
	{ "page-margin-footer",         "0in",                          false, NULL},
	{ "page-margin-header",         "0in",                          false, NULL},
	{ "page-margin-left",		"1in",				false, NULL},
	{ "page-margin-right",		"1in",				false, NULL},
	{ "page-margin-top",		"1in",				false, NULL},

	{ "section-space-after",	"0.25in",			false, NULL},
   	{ "start-value",			"1",				true, NULL},

	{ "tabstops", "", false, NULL},
#ifdef BIDI_ENABLED
	{ "text-align", text_align,	true, NULL},
#else
	{ "text-align", "left",	true, NULL},
#endif	
	{ "text-decoration", "none", true, NULL},
	{ "text-indent", "0in", false, NULL},
	{ "text-position", "normal", true, NULL},	
	
	{ "widows", "2", false, NULL},
	{ "width", "", false, NULL},

};

#ifdef __MRC__
extern "C"
#endif
static int s_compare (const void * a, const void * b)
{
  const PP_Property * prop;
  const char * name;

  name = (const char *)a;
  prop = (const PP_Property *)b;

  return UT_strcmp (name, prop->getName());
}

/*****************************************************************/

const PP_Property * PP_lookupProperty(const XML_Char * name)
{
	PP_Property * prop = NULL;

	static UT_HashTable propHash(NrElements(_props));

	const char * szName = (const char *)name;
	UT_HashEntry * entry = propHash.findEntry (szName);

	if (entry)
	  {
	    return (PP_Property *)entry->pData;
	  }

	prop = (PP_Property *)bsearch (name, _props, NrElements(_props), sizeof (_props[0]), s_compare);

	propHash.addEntry(szName, NULL, prop);

	return prop;
}

#ifdef BIDI_ENABLED
//allows us to reset the default value for the direction settings;
void PP_resetInitialBiDiValues(const XML_Char * pszValue)
{
	int i;
	int count = NrElements(_props);

	for (i=0; i<count; i++)
	{
		if ((0 == UT_stricmp(_props[i].m_pszName, "dir"))||(0 == UT_stricmp(_props[i].m_pszName, "dom-dir")))
		{
			UT_XML_strncpy(_props[i].m_pszInitial, 3,pszValue);
		}
		else if ((0 == UT_stricmp(_props[i].m_pszName, "text-align")))
		{
			UT_DEBUGMSG(("reseting text-align (%s)\n", pszValue));
			if(pszValue[0] == (XML_Char)'r')
				UT_XML_strncpy(_props[i].m_pszInitial, 5,"right");
			else
				UT_XML_strncpy(_props[i].m_pszInitial, 4,"left");
			break; //since the list is alphabetical, this is always the last one
		}
	}
}
#endif

static PD_Style * _getStyle(const PP_AttrProp * pAttrProp, PD_Document * pDoc)
{
	PD_Style * pStyle = NULL;

	const XML_Char * szValue = NULL;
	if (pAttrProp->getAttribute(PT_STYLE_ATTRIBUTE_NAME, szValue))
	{
		UT_ASSERT(szValue && szValue[0]);
		if (pDoc)
			pDoc->getStyle((char*)szValue, &pStyle);

		// NOTE: we silently fail if style is referenced, but not defined
	}

	return pStyle;
}

// make sure we don't get caught in a BASEDON loop
#define pp_BASEDON_DEPTH_LIMIT	10

const XML_Char * PP_evalProperty(const XML_Char *  pszName,
								 const PP_AttrProp * pSpanAttrProp,
								 const PP_AttrProp * pBlockAttrProp,
								 const PP_AttrProp * pSectionAttrProp,
								 PD_Document * pDoc,
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

	const XML_Char * szValue;
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
		if (pSpanAttrProp->getProperty(pProp->getName(),szValue))
			return szValue;

		if (bExpandStyles)
		{
			pStyle = _getStyle(pSpanAttrProp, pDoc);

			int i = 0;
			while (pStyle && (i < pp_BASEDON_DEPTH_LIMIT))
			{
				if (pStyle->getProperty(pProp->getName(), szValue))
					return szValue;

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
			if (pBlockAttrProp->getProperty(pProp->getName(),szValue))
				return szValue;
			
			if (bExpandStyles)
			{
				pStyle = _getStyle(pBlockAttrProp, pDoc);

				int i = 0;
				while (pStyle && (i < pp_BASEDON_DEPTH_LIMIT))
				{
					if (pStyle->getProperty(pProp->getName(), szValue))
						return szValue;

					pStyle = pStyle->getBasedOn();
					i++;
				}
			}
		}

		if (!pBlockAttrProp || pProp->canInherit())
		{
			if (pSectionAttrProp && pSectionAttrProp->getProperty(pProp->getName(),szValue))
				return szValue;
		}
	}
//
// If expandstyles is false we stop here
//
	if(!bExpandStyles)
	{
		return NULL;
	}

	if (pDoc->getStyle("Normal", &pStyle))
	{
		// next to last resort -- check for this property in the Normal style
		if (pStyle->getProperty(pProp->getName(), szValue))
			return szValue;
	}

	// if no inheritance allowed for it or there is no
	// value set in containing block or section, we return
	// the default value for this property.
	
	return pProp->getInitial();
}

const PP_PropertyType * PP_evalPropertyType(const XML_Char *  pszName,
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


const PP_PropertyType *	PP_Property::getInitialType(tProperty_type Type) const
{
	if(!m_pProperty)
	{
		// TODO:: This is never freed.
		((PP_Property *)this)->m_pProperty = PP_PropertyType::createPropertyType(Type, m_pszInitial);
	}

	return m_pProperty;
}
		
PP_PropertyType *PP_PropertyType::createPropertyType(tProperty_type Type, const XML_Char *p_init)
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
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return p_property;
}

PP_PropertyTypeColor::PP_PropertyTypeColor(const XML_Char *p_init)
{
	UT_parseColor(p_init, Color);
}

PP_PropertyTypeBool::PP_PropertyTypeBool(const XML_Char *p_init)
{
	State = UT_strcmp("yes", p_init);
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
