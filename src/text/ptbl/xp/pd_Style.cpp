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
#include "pp_Attribute.h"
#include "ut_vector.h"
#include "ut_debugmsg.h"

PD_Style::PD_Style(pt_PieceTable * pPT,
				   PT_AttrPropIndex indexAP,
				   GQuark name) :
  m_pPT(pPT),
  m_indexAP(indexAP),
  m_name(name),
  m_iUsed(0),
  m_pBasedOn(NULL),
  m_pFollowedBy(NULL)
{
}

PD_Style::~PD_Style()
{
}

bool PD_Style::setIndexAP(PT_AttrPropIndex indexAP)
{
	UT_ASSERT_HARMLESS(indexAP != m_indexAP);

	// TODO: may need to rebind, handle undo, clear caches, etc.

	m_indexAP = indexAP;

	return true;
}

bool PD_Style::getProperty(PT_Property name, GQuark & value) const
{
	const PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
		return false;
	else
		return pAP->getProperty(name, value);
}


bool PD_Style::getPropertyExpand(PT_Property name, GQuark & value)
{
	const PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
	{
		return false;
	}
	else
	{
		if( pAP->getProperty(name, value))
		{
			return true;
		}
		else
		{
			PD_Style * pStyle = getBasedOn();
			if(pStyle != NULL)
			{
				return pStyle->_getPropertyExpand(name, value, 0);
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}


bool PD_Style::_getPropertyExpand(PT_Property name,
								  GQuark & value,
								  UT_sint32 iDepth)
{
	const PP_AttrProp * pAP = NULL;
	
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
	{
		return false;
	}
	else
	{
		if( pAP->getProperty(name, value))
		{
			return true;
		}
		else
		{
			PD_Style * pStyle = getBasedOn();
			if((pStyle != NULL) && (iDepth < pp_BASEDON_DEPTH_LIMIT ))
			{
				return pStyle->_getPropertyExpand(name, value, iDepth+1);
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}


bool PD_Style::getAttributeExpand(PT_Attribute name, GQuark & value)
{
	const PP_AttrProp * pAP = NULL;
	
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
	{
		return false;
	}
	else
	{
		if( pAP->getAttribute(name, value))
		{
			return true;
		}
		else
		{
			PD_Style * pStyle = getBasedOn();
			if(pStyle != NULL )
			{
				return pStyle->_getAttributeExpand(name, value, 0);
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}


bool PD_Style::_getAttributeExpand(PT_Attribute name, GQuark & value, UT_sint32 iDepth)
{
	const PP_AttrProp * pAP = NULL;
	
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
	{
		return false;
	}
	else
	{
		if( pAP->getAttribute(name, value))
		{
			return true;
		}
		else
		{
			PD_Style * pStyle = getBasedOn();
			if((pStyle != NULL) && (iDepth < pp_BASEDON_DEPTH_LIMIT ) )
			{
				return pStyle->_getAttributeExpand(name, value, iDepth+1);
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}

const PP_PropertyType *	PD_Style::getPropertyType(PT_Property name, tProperty_type Type) const
{
	const PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
		return NULL;
	else
		return pAP->getPropertyType(name, Type);
}


bool PD_Style::getAttribute(PT_Attribute name, GQuark & value) const
{
	const PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
		return false;
	else
		return pAP->getAttribute(name, value);
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

	GQuark value;
	if (getAttribute(PT_TYPE_ATTRIBUTE_NAME, value))
		return (value == PP_Q(c) ||
				value == PP_Q(C));

	return false;
}

bool PD_Style::isList(void)
{
	GQuark value;
	if (getPropertyExpand(abi_list_style, value))
	{
		return value == PP_Q(None);
	}
	else {
		return FALSE;
	}
}

PD_Style * PD_Style::getBasedOn(void)
{
	if (m_pBasedOn)
		return m_pBasedOn;

	GQuark value;

	if (getAttribute(PT_BASEDON_ATTRIBUTE_NAME, value) && value)
		m_pPT->getStyle(value, &m_pBasedOn);

	// NOTE: we silently fail if style is referenced, but not defined

	return m_pBasedOn;
}

PD_Style * PD_Style::getFollowedBy(void)
{
	if (m_pFollowedBy)
		return m_pFollowedBy;

	GQuark value;

	if (getAttribute(PT_FOLLOWEDBY_ATTRIBUTE_NAME, value) && value)
		m_pPT->getStyle(value, &m_pFollowedBy);

	// NOTE: we silently fail if style is referenced, but not defined

	return m_pFollowedBy;
}

/*!
 * Add a property to the style definition.
 */
bool PD_Style::addProperty(PT_Property name, GQuark value)
{
	PP_AttrProp * pAP = NULL;
	bool bres= true;
	
	if (!m_pPT->getAttrProp(m_indexAP, (const PP_AttrProp **) &pAP))
		return false;
	else
	{
		PT_PropertyPair Props[] = {{name, value}, {(PT_Property)0,0}};
		
		PP_AttrProp * pNewAP = pAP->cloneWithReplacements(NULL, &Props[0],
														  false);
		pNewAP->markReadOnly();
		bres =	m_pPT->getVarSet().addIfUniqueAP(pNewAP, &m_indexAP);
	}
	return bres;
}

/*!
 * This method adds the properties defined in pProperties to the already
 * defined set. If a property already exists in the definition, its value is
 * replaced.
\param const XML_Char ** pProperties string of properties
*/
bool PD_Style::addProperties(const PT_PropertyPair * pProperties)
{
	PP_AttrProp * pAP = NULL;
	bool bres= true;
	if (!m_pPT->getAttrProp(m_indexAP, (const PP_AttrProp **)&pAP))
		return false;
	else
	{
		PP_AttrProp * pNewAP = pAP->cloneWithReplacements(NULL,
														  pProperties,
														  false);
		
		pNewAP->markReadOnly();
		bres = m_pPT->getVarSet().addIfUniqueAP(pNewAP, &m_indexAP);
	}
	return bres;
}

/*!
 * This method replaces the previous set of attributes/properties with
 * a new one defined in pAtts. It is imperitive that updateDocForStyleChange
 * be called after this method.
 \param const XML_Char ** pAtts list of attributes with an extended properties
 *                        string
 */
bool PD_Style::setAllAttributes(const PT_AttributePair * pAtts)
{
	bool bres =	m_pPT->getVarSet().storeAP(pAtts, &m_indexAP);
	m_pFollowedBy = NULL;
	m_pBasedOn = NULL;
	return bres;
}


bool PD_Style::addAttributes(const PT_AttributePair * pAtts)
{
	PP_AttrProp * pAP = NULL;
	bool bres = false;
	if (!m_pPT->getAttrProp(m_indexAP, (const PP_AttrProp **)&pAP))
		return false;
	else
	{
		if(pAP->areAlreadyPresent(pAtts,NULL))
		{
			return true;
		}
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
  	const PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
		return 0;
	else
	        return pAP->getAttributeCount();
}


size_t PD_Style::getPropertyCount(void) const
{
  	const PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
		return 0;
	else
	        return pAP->getPropertyCount();
}
	

/*!
 * This method fills a vector structure with all the attributes defined
 * in this style, including the basedon style.
\param vProps the vector containing const XML_Char * (name,value) pairs
*/

static bool foreachAttr (PT_Attribute & name,
						 GQuark & value,
						 UT_uint32 i,
						 PT_AttributeVector * vAttrs)
{
	bool bfound = false;
//
// Only keep the most recently defined properties
//
	for(UT_uint32 j = 0; (j < vAttrs->getItemCount()) && !bfound ; j++)
	{
		bfound = (name == vAttrs->getNthItem(j).a);
	}
	if(!bfound)
	{
		PT_AttributePair ap;
		ap.a = name;
		ap.v = value;
		vAttrs->addItem(ap);
	}

	return true;
}


bool PD_Style::getAllAttributes( PT_AttributeVector & vAttrs, UT_sint32 depth)
{
//
// This method will be recursively called to basedon style
//
  	const PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
		return false;

	pAP->forEachAttribute ((PP_AttrProp::PP_ForEachAttrFunc)foreachAttr,
						   &vAttrs);

	if(depth <  pp_BASEDON_DEPTH_LIMIT && getBasedOn() != NULL)
	{
		getBasedOn()->getAllAttributes(vAttrs,depth +1);
	}
	return true;
}

/*!
 * This method fills a vector structure with all the properties defined
 * in this style, including the basedon style.
\param vProps the vector containing const XML_Char * (name,value) pairs
*/

static bool foreachProp (PT_Property  & name,
						 GQuark & value,
						 UT_uint32 i,
						 PT_PropertyVector * vProps)
{
	bool bfound = false;
//
// Only keep the most recently defined properties
//
	for(UT_uint32 j = 0; (j < vProps->getItemCount()) && !bfound ; j++)
	{
		bfound = (name == vProps->getNthItem(j).p);
	}
	if(!bfound)
	{
		PT_PropertyPair pp;
		pp.p = name;
		pp.v = value;
		
		vProps->addItem(pp);
	}

	return true;
}


bool PD_Style::getAllProperties( PT_PropertyVector & vProps, UT_sint32 depth)
{
//
// This method will be recursively called to basedon style
//
  	const PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
		return false;

	pAP->forEachProperty ((PP_AttrProp::PP_ForEachPropFunc)foreachAttr,
						  &vProps);

	if(depth <  pp_BASEDON_DEPTH_LIMIT && getBasedOn() != NULL)
	{
		getBasedOn()->getAllProperties(vProps,depth +1);
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////
// a sub-class to wrap the compiled-in styles
//////////////////////////////////////////////////////////////////

PD_BuiltinStyle::PD_BuiltinStyle(pt_PieceTable * pPT,
								 PT_AttrPropIndex indexAP,
								 GQuark name)
  : PD_Style(pPT, indexAP, name),
	m_indexAPOrig(indexAP)
{
}

PD_BuiltinStyle::~PD_BuiltinStyle()
{
}



