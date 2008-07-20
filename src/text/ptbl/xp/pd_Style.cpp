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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include "ut_string.h"
#include "pd_Style.h"
#include "pt_PieceTable.h"
#include "pp_Property.h"
#include "ut_vector.h"
#include "ut_debugmsg.h"

#include <string>

PD_Style::PD_Style(pt_PieceTable * pPT, PT_AttrPropIndex indexAP, const char * szName, const char * szLabel, bool bDisplayed) :
  m_pPT(pPT), m_indexAP(indexAP), m_szName(NULL), m_sLabel(""), m_bDisplayed(bDisplayed), m_iUsed(0),
  m_pBasedOn(NULL), m_pFollowedBy(NULL)
{
	if (szName)
		m_szName = g_strdup (szName);
	else
	{
		const gchar * name=0;
		if (getAttribute( (const gchar*) PT_NAME_ATTRIBUTE_NAME, name) )
			m_szName=(char *) name;
	}
	
	if (szLabel)
		m_sLabel = szLabel;
	else
	{
		const gchar * label=0;
		if ( getAttribute((const gchar*) PT_LABEL_ATTRIBUTE_NAME, label) )
			m_sLabel=label;
	}
		
	/*  This may return in a different function - but let's keep this non-destructive.
		if (szName) // Label defaults to style name.
			 m_sLabel = szName;
	
	// Store the label in the attrprop as well as a member variable.  Bad idea?
	const gchar * pAttr[4] = {PT_LABEL_ATTRIBUTE_NAME,m_sLabel.c_str(),NULL,NULL};
	UT_ASSERT(addAttributes(pAttr));
	UT_DEBUGMSG(("~~~~~New Style: Name: %s  Label: %s\n", m_szName, m_sLabel.c_str()));
	*/
	
}

PD_Style::~PD_Style()
{
	FREEP(m_szName);
}

bool PD_Style::setIndexAP(PT_AttrPropIndex indexAP)
{
	UT_ASSERT_HARMLESS(indexAP != m_indexAP);

	// TODO: may need to rebind, handle undo, clear caches, etc.

	m_indexAP = indexAP;

	return true;
}

bool PD_Style::getProperty(const gchar * szName, const gchar *& szValue) const
{
	const PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
		return false;
	else
		return pAP->getProperty(szName, szValue);
}


bool PD_Style::getPropertyExpand(const gchar * szName, const gchar *& szValue, UT_sint32 iDepth) const
{
	const PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
	{
		return false;
	}
	else
	{
		if( pAP->getProperty(szName, szValue))
		{
			return true;
		}
		else
		{
			PD_Style * pStyle = getBasedOn();
			if(pStyle != NULL)
			{
				return pStyle->getPropertyExpand(szName,szValue, iDepth + 1);
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}

bool PD_Style::getAttributeExpand(const gchar * szName, const gchar *& szValue, UT_sint32 iDepth)
{
	const PP_AttrProp * pAP = NULL;
	
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
	{
		return false;
	}
	else
	{
		if( pAP->getAttribute(szName, szValue))
		{
			return true;
		}
		else
		{
			PD_Style * pStyle = getBasedOn();
			if(pStyle != NULL )
			{
				return pStyle->getAttributeExpand(szName,szValue, iDepth + 1);
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}

const PP_PropertyType *	PD_Style::getPropertyType(const gchar * szName, tProperty_type Type) const
{
	const PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
		return NULL;
	else
		return pAP->getPropertyType(szName, Type);
}


bool PD_Style::getAttribute(const gchar * szName, const gchar *& szValue) const
{
	const PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
		return false;
	else
		return pAP->getAttribute(szName, szValue);
}

void PD_Style::used(UT_sint32 count)
{
	m_iUsed += count;
	if(m_iUsed < 0) m_iUsed = 0;
}

bool PD_Style::isUsed(void) const
{
	// TODO: we need some way of refcounting
	// TODO: what if this document is a template
	// NOTE: Re: the above - 
	//	Not sure how to know when a style that was
	// 	being used isn't anymore.  For the second
	//	point, I think a m_bTemplateStyle member
	//	might suffice.

	// marginally better
	return m_iUsed > 0;
}

bool PD_Style::isCharStyle(void) const
{
	// TODO: cache this too  

	const gchar * szValue = NULL;
	if (getAttribute(PT_TYPE_ATTRIBUTE_NAME, szValue))
		if (szValue && szValue[0])
			return g_ascii_strcasecmp(szValue, "C") == 0; // *PLEASE LEAVE THIS AS CASE
                                               // *INSENSITIVE. IF YOU DON'T
                                               // * I'LL PUT YOUR EMAIL ADDRESS
                                               // * ON SPAM BLACK LIST!
                                               // I'm sick and tired of fixing
                                               // this regression - MES

	// default: no
	return false;
}

bool PD_Style::isList(void) const
{
	const char *szListStyle = NULL;
	if (getPropertyExpand("list-style", szListStyle))
		return (g_ascii_strcasecmp(szListStyle, "None") != 0);
	else
		return false;

}

PD_Style * PD_Style::getBasedOn(void) const
{
	if (m_pBasedOn)
		return m_pBasedOn;

	const gchar * szStyle;

	if (getAttribute(PT_BASEDON_ATTRIBUTE_NAME, szStyle))
		if (szStyle && szStyle[0])
			m_pPT->getStyle((char*)szStyle, &m_pBasedOn);

	// NOTE: we silently fail if style is referenced, but not defined

	return m_pBasedOn;
}

PD_Style * PD_Style::getFollowedBy(void) const
{
	if (m_pFollowedBy)
		return m_pFollowedBy;

	const gchar * szStyle;

	if (getAttribute(PT_FOLLOWEDBY_ATTRIBUTE_NAME, szStyle))
		if (szStyle && szStyle[0])
			m_pPT->getStyle((char*)szStyle, &m_pFollowedBy);

	// NOTE: we silently fail if style is referenced, but not defined

	return m_pFollowedBy;
}

/*!
 * Add a property to the style definition.
 */
bool PD_Style::addProperty(const gchar * szName, const gchar * szValue)
{
	PP_AttrProp * pAP = NULL;
	bool bres= true;
	
	if (!m_pPT->getAttrProp(m_indexAP, (const PP_AttrProp **) &pAP))
		return false;
	else
	{
		const gchar * pProps[4] = {NULL,NULL,NULL,NULL};
		pProps[0] = szName;
		pProps[1] = szValue;
		PP_AttrProp * pNewAP = pAP->cloneWithReplacements(NULL, pProps, false);
		pNewAP->markReadOnly();
		bres =	m_pPT->getVarSet().addIfUniqueAP(pNewAP, &m_indexAP);
	}
	return bres;
}

/*!
 * This method adds the properties defined in pProperties to the already
 * defined set. If a property already exists in the definition, its value is
 * replaced.
\param const gchar ** pProperties string of properties
*/
bool PD_Style::addProperties(const gchar ** pProperties)
{
	PP_AttrProp * pAP = NULL;
	bool bres= true;
	if (!m_pPT->getAttrProp(m_indexAP, (const PP_AttrProp **)&pAP))
		return false;
	else
	{
		PP_AttrProp * pNewAP = pAP->cloneWithReplacements(NULL, pProperties, false);
		pNewAP->markReadOnly();
		bres =	m_pPT->getVarSet().addIfUniqueAP(pNewAP, &m_indexAP);
	}
	return bres;
}

/*!
 * This method replaces the previous set of attributes/properties with
 * a new one defined in pAtts.
 \param const gchar ** pAtts list of attributes with an extended properties
 *                        string
 */
bool PD_Style::setAllAttributes(const gchar ** pAtts)
{
	bool bres =	m_pPT->getVarSet().storeAP(pAtts, &m_indexAP);
	m_pFollowedBy = NULL;
	m_pBasedOn = NULL;
	if (bres)
		return getDoc()->updateDocForStyleChange(m_szName,!isCharStyle());
	else
		return bres;
}


bool PD_Style::addAttributes(const gchar ** pAtts)
{
	PP_AttrProp * pAP = NULL;
	bool bres = false;
	if (!m_pPT->getAttrProp(m_indexAP, (const PP_AttrProp **)&pAP))
		return false;
	else
	{
		if(pAP->areAlreadyPresent(pAtts,NULL))
			return true;
		
		PP_AttrProp * pNewAP = pAP->cloneWithReplacements(pAtts, NULL, false);
		UT_return_val_if_fail( pNewAP, false );
		
		pNewAP->markReadOnly();
		bres =	m_pPT->getVarSet().addIfUniqueAP(pNewAP, &m_indexAP);
	}
	m_pFollowedBy = NULL;
	m_pBasedOn = NULL;
	return bres;
}

size_t PD_Style::getAttributeCount(void) const
{
  	PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, (const PP_AttrProp **)&pAP))
		return 0;
	else
		return pAP->getAttributeCount();
}


size_t PD_Style::getPropertyCount(void) const
{
  	PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, (const PP_AttrProp **)&pAP))
		return 0;
	else
		return pAP->getPropertyCount();
}
	
bool PD_Style::getNthAttribute (int ndx, const gchar *&szName,
			     const gchar *&szValue) const
{
  	PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, (const PP_AttrProp **)&pAP))
		return false;
	else
	{
		return pAP->getNthAttribute(ndx, szName, szValue);
	}
}

bool PD_Style::getNthProperty (int ndx, const gchar *&szName,
			       const gchar *&szValue) const
{
	PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, (const PP_AttrProp **)&pAP))
		return false;
	else
	{
		return pAP->getNthProperty(ndx, szName, szValue);
	}
}

/*!
 * This method takes a property vector and
 * replaces the properties entirely with the ones provided.
 */
bool PD_Style::setAllProperties (const gchar ** v_szProperties)
{
	std::string sProps;
	const gchar * szName = NULL;
	const gchar * szValue = NULL;
	int i=0;
	
	while(v_szProperties[i] != NULL)
	{
		szName = v_szProperties[i];
		szValue = v_szProperties[i+1];
		if(strstr(szName,"toc-") == NULL)
		{
			sProps += std::string(szName) + ":" + std::string(szValue) + ";";
		}
		i = i + 2;
	}
	
	// Remove trailing semicolon
	// property string has semicolon delimiters but a trailing semicolon will cause a failure.
	if (sProps.size() > 0 && sProps[sProps.size()-1] == ';')
	{
		sProps.resize(sProps.size()-1);
	}
	
	// duplicate the string and use it
	return setAllProperties(g_strdup(sProps.c_str()));
	
	
}

/*!
 * This method takes a property string (with : and ; delimiters) and
 * replaces the properties entirely with the ones provided.
 */
bool PD_Style::setAllProperties(const gchar * szProperties)
{
	
	UT_GenericVector<const gchar *> vAttributes;
	UT_return_val_if_fail(getAllAttributes(& vAttributes), false)
	
	int i;
	const gchar ** a;
	a = new const gchar * [vAttributes.getItemCount()+1];
	for(i = 0; i < vAttributes.getItemCount() ; i += 2)
	{
		a[i]=vAttributes[i];
		if (0 == strcmp(PT_PROPS_ATTRIBUTE_NAME, (const char *) vAttributes[i]))
			a[i+1]=szProperties;
		else
			a[i+1]=vAttributes[i+1];
	}
	// null terminate array
	a[i]=0;
	
	//
	// Apply changes
	//
	
	UT_return_val_if_fail(setAllAttributes(a), false);
	
	return true;
}

/*!
 * This method fills a vector structure with all the attributes defined
 * in this style, including the basedon style.
\param vAttribs the vector containing const gchar * (name,value) pairs
*/

bool PD_Style::getAllAttributes( UT_GenericVector<const gchar *> * vAttribs, UT_sint32 depth)
{
//
// This method will be recursively called to basedon style
//
	UT_uint32 count = getAttributeCount();
	UT_uint32 i,j;
	const gchar * szName = NULL;
	const gchar * szValue = NULL;
	for(i=0; i < count; i++)
	{
		getNthAttribute(i, szName, szValue);
		bool bfound = false;
//
// Only keep the most recently defined properties
//
		for(j = 0; (j < vAttribs->getItemCount()) && !bfound ; j += 2)
		{
			bfound = (0 == strcmp(szName, (const char *) vAttribs->getNthItem(j)));
		}
		if(!bfound)
		{
			vAttribs->addItem(szName);
			vAttribs->addItem(szValue);
		}
	}
	if(depth <  pp_BASEDON_DEPTH_LIMIT && getBasedOn() != NULL)
	{
		getBasedOn()->getAllAttributes(vAttribs,depth +1);
	}
	return true;
}

/*!
 * This method fills a vector structure with all the properties defined
 * in this style, including the basedon style.
\param vProps the vector containing const gchar * (name,value) pairs
*/

bool PD_Style::getAllProperties( UT_GenericVector<const gchar *> * vProps, UT_sint32 depth)
{
//
// This method will be recursively called to basedon style
//
	UT_uint32 count = getPropertyCount();
	UT_uint32 i,j;
	const gchar * szName = NULL;
	const gchar * szValue = NULL;
	for(i=0; i < count; i++)
	{
		getNthProperty(i, szName, szValue);
		bool bfound = false;
//
// Only keep the most recently defined properties
//
		for(j = 0; (j < vProps->getItemCount()) && !bfound ; j += 2)
		{
			bfound = (0 == strcmp(szName, (const char *) vProps->getNthItem(j)));
		}
		if(!bfound)
		{
			vProps->addItem(szName);
			vProps->addItem(szValue);
		}
	}
	if(depth <  pp_BASEDON_DEPTH_LIMIT && getBasedOn() != NULL)
	{
		getBasedOn()->getAllProperties(vProps,depth +1);
	}
	return true;
}

/*!
 * This method eliminates any properties which are identical
 * to the "based-on" styles.
 */
bool PD_Style::simplifyProperties()
{
	if (!getBasedOn())
		return true; // not based on anything, thus, simplified.
	
	UT_GenericVector<const gchar *> vKeepers, vTheseProps, vBasedOnProps, vAttrs;
	std::map<std::string, std::string> mBasedOnProps;
	
	getAllProperties (&vTheseProps, (UT_sint32) 0);
	getBasedOn()-> getAllProperties(&vBasedOnProps, (UT_sint32) 1);
	
	
	// Turn based on props into a map.
	int i;
	int iBasedOnPropCount=vBasedOnProps.getItemCount();
	for (i=0; (i+1<iBasedOnPropCount); i+=2)
		mBasedOnProps[vBasedOnProps[i]] = vBasedOnProps[i+1];
	
	// Keep only unique ones.
	int iPropCount=vTheseProps.getItemCount();
	for (i=0; (i+1<iPropCount); i+=2)
	{
		if (mBasedOnProps[vTheseProps[i]] != std::string( vTheseProps[i+1]))
		{
			vKeepers.push_back(vTheseProps[i]);
			vKeepers.push_back(vTheseProps[i+1]);
		}
	}

	//
	// Eliminate Defaults
	//
	
	// TODO
	
	// OK, now replace old stuff.
	
	const gchar ** newProps;
	newProps = new const gchar * [vKeepers.getItemCount()+1];
	for (i=0; i<vKeepers.getItemCount(); i++)
		newProps[i]=g_strdup(vKeepers[i]);
	newProps[i]=0;
		
	return setAllProperties(newProps); //apparently worked.
}

/*!
 * Pointer to the pt_PieceTable for the current document we're working with.
 */
pt_PieceTable * PD_Style::getPT(void) const
{
	return m_pPT;
}

/*!
 * Pointer to the PD_Document corresponding to the current document.
 */
PD_Document * PD_Style::getDoc(void) const 
{
	return m_pPT->getDocument();
}

/*!
 * This method takes a style and extracts all the properties associated with it and
 * place them in a properties and attributes vector for easy modification by
 * the code.  Originally from ap_Dialog_Styles
 */
std::map<std::string, std::string> PD_Style::_returnPropsMap(const gchar * szStyle, bool bReplaceAttributes) const
{
	PD_Style * pStyle = NULL;
	/*
	m_vecAllProps.clear();
	if( bReplaceAttributes)
		m_vecAllAttribs.clear();
	*/
	
	std::map<std::string, std::string> mProperties;
	std::map<std::string, std::string> mAttributes;
	
	if(szStyle == NULL || ! getDoc()->getStyle(szStyle,&pStyle))
	{
		return std::map<std::string, std::string>();
	}

	const static gchar * paraFields[] = {"text-align", "text-indent", "margin-left", "margin-right", "margin-top", "margin-bottom", "line-height","tabstops","start-value","list-delim", "list-style","list-decimal","field-font","field-color", "keep-together","keep-with-next","orphans","widows","dom-dir"};

	const size_t nParaFlds = sizeof(paraFields)/sizeof(paraFields[0]);

	const static gchar * charFields[] =
	{"bgcolor","color","font-family","font-size","font-stretch","font-style",
	 "font-variant", "font-weight","text-decoration","lang"};

	const size_t nCharFlds = sizeof(charFields)/sizeof(charFields[0]);

	const static gchar * attribs[] =
	{PT_FOLLOWEDBY_ATTRIBUTE_NAME,PT_BASEDON_ATTRIBUTE_NAME,PT_LISTID_ATTRIBUTE_NAME,PT_PARENTID_ATTRIBUTE_NAME,PT_LEVEL_ATTRIBUTE_NAME,PT_NAME_ATTRIBUTE_NAME,PT_STYLE_ATTRIBUTE_NAME,PT_TYPE_ATTRIBUTE_NAME};

	const size_t nattribs = sizeof(attribs)/sizeof(attribs[0]);
	UT_uint32 i;
	UT_DEBUGMSG(("Looking at Style %s \n",szStyle));
	UT_Vector vecAllProps;
	vecAllProps.clear();
//
// Loop through all Paragraph properties and add those with non-null values
//
	for(i = 0; i < nParaFlds; i++)
	{
		const gchar * szName = paraFields[i];
		const gchar * szValue = NULL;
		pStyle->getProperty(szName,szValue);
		if(szValue)
			mProperties[szName]=szValue;
	}
//
// Loop through all Character properties and add those with non-null values
//
	for(i = 0; i < nCharFlds; i++)
	{
		const gchar * szName = charFields[i];
		const gchar * szValue = NULL;
		pStyle->getProperty(szName,szValue);
		if(szValue)
		{
			xxx_UT_DEBUGMSG(("Adding char prop %s value %s \n",szName,szValue));
			mProperties[szName]=szValue;
		}
	}
//
// Loop through all the attributes and add those with non-null values
//
	xxx_UT_DEBUGMSG(("Replace Attributes %d \n",bReplaceAttributes));
	if(bReplaceAttributes)
	{
		UT_Vector vecAllAtts;
		vecAllAtts.clear();
		for(i = 0; i < nattribs; i++)
		{
			const gchar * szName = attribs[i];
			const gchar * szValue = NULL;
			pStyle->getAttributeExpand(szName,szValue);
			if(szValue)
				mAttributes[szName]=szValue;
		}
	}
	else
	{
		UT_DEBUGMSG(("Attributes NOT updated \n"));
	}
	return mProperties;
}


//////////////////////////////////////////////////////////////////
// a sub-class to wrap the compiled-in styles
//////////////////////////////////////////////////////////////////

PD_BuiltinStyle::PD_BuiltinStyle(pt_PieceTable * pPT, PT_AttrPropIndex indexAP, const char * szName, bool bDisplayed)
  : PD_Style(pPT, indexAP, szName, szName, bDisplayed), m_indexAPOrig(indexAP)
{
}

PD_BuiltinStyle::~PD_BuiltinStyle()
{
}



