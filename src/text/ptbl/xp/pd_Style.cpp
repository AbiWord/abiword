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

PD_Style::PD_Style(pt_PieceTable * pPT, PT_AttrPropIndex indexAP)
{
	m_pPT = pPT;
	m_indexAP = indexAP;
	m_pBasedOn = NULL;
	m_pFollowedBy = NULL;
}

PD_Style::~PD_Style()
{
}

bool PD_Style::setIndexAP(PT_AttrPropIndex indexAP)
{
	UT_ASSERT(indexAP != m_indexAP);

	// TODO: may need to rebind, handle undo, clear caches, etc.

	m_indexAP = indexAP;

	return true;
}

bool PD_Style::getProperty(const XML_Char * szName, const XML_Char *& szValue) const
{
	const PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
		return false;
	else
		return pAP->getProperty(szName, szValue);
}

bool PD_Style::getAttribute(const XML_Char * szName, const XML_Char *& szValue) const
{
	const PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
		return false;
	else
		return pAP->getAttribute(szName, szValue);
}

bool PD_Style::isUsed(void) const
{
	// TODO: we need some way of refcounting
	// TODO: what if this document is a template

	// for now, we cheat
	return isUserDefined();
}

bool PD_Style::isCharStyle(void) const
{
	// TODO: cache this too  

	const XML_Char * szValue = NULL;
	if (getAttribute(PT_TYPE_ATTRIBUTE_NAME, szValue))
		if (szValue && szValue[0])
			return !UT_strcmp(szValue, "c");

	// default: no
	return false;
}

PD_Style * PD_Style::getBasedOn(void)
{
	if (m_pBasedOn)
		return m_pBasedOn;

	const XML_Char * szStyle;

	if (getAttribute(PT_BASEDON_ATTRIBUTE_NAME, szStyle))
		if (szStyle && szStyle[0])
			m_pPT->getStyle((char*)szStyle, &m_pBasedOn);

	// NOTE: we silently fail if style is referenced, but not defined

	return m_pBasedOn;
}

PD_Style * PD_Style::getFollowedBy(void)
{
	if (m_pFollowedBy)
		return m_pFollowedBy;

	const XML_Char * szStyle;

	if (getAttribute(PT_FOLLOWEDBY_ATTRIBUTE_NAME, szStyle))
		if (szStyle && szStyle[0])
			m_pPT->getStyle((char*)szStyle, &m_pFollowedBy);

	// NOTE: we silently fail if style is referenced, but not defined

	return m_pFollowedBy;
}


bool PD_Style::setProperty(const XML_Char * szName, const XML_Char * szValue)
{
	PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, (const PP_AttrProp **) &pAP))
		return false;
	else
		return pAP->setProperty(szName, szValue);
}

bool PD_Style::setProperties(const XML_Char ** pProperties)
{
	PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, (const PP_AttrProp **)&pAP))
		return false;
	else
		return pAP->setProperties(pProperties);
}

//////////////////////////////////////////////////////////////////
// a sub-class to wrap the compiled-in styles
//////////////////////////////////////////////////////////////////

PD_BuiltinStyle::PD_BuiltinStyle(pt_PieceTable * pPT, PT_AttrPropIndex indexAP)
	: PD_Style(pPT, indexAP)
{
	m_indexAPOrig = indexAP;
}

PD_BuiltinStyle::~PD_BuiltinStyle()
{
}



