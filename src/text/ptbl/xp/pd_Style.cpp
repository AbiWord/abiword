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

PD_Style::PD_Style(pt_PieceTable * pPT, PT_AttrPropIndex indexAP, const char * szName) :
  m_pPT(pPT), m_indexAP(indexAP), m_szName(NULL), m_iUsed(0),
  m_pBasedOn(NULL), m_pFollowedBy(NULL), m_iIsList(-1)
{
  if (szName)
    m_szName = UT_strdup (szName);
}

PD_Style::~PD_Style()
{
  FREEP(m_szName);
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

const PP_PropertyType *	PD_Style::getPropertyType(const XML_Char * szName, tProperty_type Type) const
{
	const PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, &pAP))
		return NULL;
	else
		return pAP->getPropertyType(szName, Type);
}


bool PD_Style::getAttribute(const XML_Char * szName, const XML_Char *& szValue) const
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

	const XML_Char * szValue = NULL;
	if (getAttribute(PT_TYPE_ATTRIBUTE_NAME, szValue))
		if (szValue && szValue[0])
			return UT_stricmp(szValue, "C") == 0; // *PLEASE LEAVE THIS AS CASE
                                               // *INSESITIVE. IF YOU DON'T
                                               // * I'LL PUT YOUR EMAIL ADDRESS
                                               // * ON SPAM BLACK LIST!
                                               // I'm sick and tired of fixing
                                               // this regression - MES

	// default: no
	return false;
}

bool PD_Style::isList(void)
{
	if(m_iIsList == -1)
	{
		m_iIsList = 0;
		UT_Vector vProp;
		
		getAllProperties(&vProp, 0);
		
		UT_ASSERT(vProp.getItemCount()%2 == 0);
		
		for(UT_uint32 i = 0; i < vProp.getItemCount(); i += 2)
		{
			const XML_Char * szProp  = (const XML_Char *) vProp.getNthItem(i);
			if(!UT_strcmp(szProp, "list-style"))
			{
				m_iIsList = 1;
				break;
			}
		}
	}
	
    return m_iIsList != 0;
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


bool PD_Style::addProperty(const XML_Char * szName, const XML_Char * szValue)
{
	PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, (const PP_AttrProp **) &pAP))
		return false;
	else
		return pAP->setProperty(szName, szValue);
}

/*!
 * This method adds the properties defined in pProperties to the already
 * defined set. If a property already exists in the definition, its value is
 * replaced.
\param const XML_Char ** pProperties string of properties
*/
bool PD_Style::addProperties(const XML_Char ** pProperties)
{
	PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, (const PP_AttrProp **)&pAP))
		return false;
	else
		return pAP->setProperties(pProperties);
}

/*!
 * This method replaces the previous set of attributes/properties with
 * a new one defined in pAtts. It is imperitive that updateDocForStyleChange
 * be called after this method.
 \param const XML_Char ** pAtts list of attributes with an extended properties
 *                        string
 */
bool PD_Style::setAllAttributes(const XML_Char ** pAtts)
{
	bool bres =	m_pPT->getVarSet().storeAP(pAtts, &m_indexAP);
	m_pFollowedBy = NULL;
	m_pBasedOn = NULL;
	return bres;
}


bool PD_Style::addAttributes(const XML_Char ** pAtts)
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
	
bool PD_Style::getNthAttribute (int ndx, const XML_Char *&szName,
			     const XML_Char *&szValue) const
{
  	PP_AttrProp * pAP = NULL;
	
	if (!m_pPT->getAttrProp(m_indexAP, (const PP_AttrProp **)&pAP))
		return false;
	else
	  {
	        return pAP->getNthAttribute(ndx, szName, szValue);
	  }
}

bool PD_Style::getNthProperty (int ndx, const XML_Char *&szName,
			       const XML_Char *&szValue) const
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
 * This method fills a vector structure with all the attributes defined
 * in this style, including the basedon style.
\param vProps the vector containing const XML_Char * (name,value) pairs
*/

bool PD_Style::getAllAttributes( UT_Vector * vAttribs, UT_sint32 depth)
{
//
// This method will be recursively called to basedon style
//
	UT_uint32 count = getAttributeCount();
	UT_uint32 i,j;
	const XML_Char * szName = NULL;
	const XML_Char * szValue = NULL;
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
			vAttribs->addItem((void *) szName);
			vAttribs->addItem((void *) szValue);
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
\param vProps the vector containing const XML_Char * (name,value) pairs
*/

bool PD_Style::getAllProperties( UT_Vector * vProps, UT_sint32 depth)
{
//
// This method will be recursively called to basedon style
//
	UT_uint32 count = getPropertyCount();
	UT_uint32 i,j;
	const XML_Char * szName = NULL;
	const XML_Char * szValue = NULL;
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
			vProps->addItem((void *) szName);
			vProps->addItem((void *) szValue);
		}
	}
	if(depth <  pp_BASEDON_DEPTH_LIMIT && getBasedOn() != NULL)
	{
		getBasedOn()->getAllProperties(vProps,depth +1);
	}
	return true;
}

//////////////////////////////////////////////////////////////////
// a sub-class to wrap the compiled-in styles
//////////////////////////////////////////////////////////////////

PD_BuiltinStyle::PD_BuiltinStyle(pt_PieceTable * pPT, PT_AttrPropIndex indexAP, const char * szName)
  : PD_Style(pPT, indexAP, szName), m_indexAPOrig(indexAP)
{
}

PD_BuiltinStyle::~PD_BuiltinStyle()
{
}



