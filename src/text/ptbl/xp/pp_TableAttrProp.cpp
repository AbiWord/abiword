 
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

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_string.h"
#include "pp_AttrProp.h"
#include "pp_TableAttrProp.h"

pp_TableAttrProp::pp_TableAttrProp()
{
}

pp_TableAttrProp::~pp_TableAttrProp()
{
	UT_VECTOR_PURGEALL(PP_AttrProp, m_vecTable);
}

UT_Bool pp_TableAttrProp::createAP(UT_uint32 * pSubscript)
{
	PP_AttrProp * pNew = new PP_AttrProp();
	if (!pNew)
		return UT_FALSE;
	if (m_vecTable.addItem(pNew,pSubscript) != 0)
	{
		delete pNew;
		return UT_FALSE;
	}

	return UT_TRUE;
}

UT_Bool pp_TableAttrProp::createAP(const XML_Char ** attributes,
								   const XML_Char ** properties,
								   UT_uint32 * pSubscript)
{
	UT_uint32 subscript;
	if (!createAP(&subscript))
		return UT_FALSE;

	PP_AttrProp * pAP = (PP_AttrProp *)m_vecTable.getNthItem(subscript);
	UT_ASSERT(pAP);
	if (!pAP->setAttributes(attributes) || !pAP->setProperties(properties))
		return UT_FALSE;

	*pSubscript = subscript;
	return UT_TRUE;
}

UT_Bool pp_TableAttrProp::createAP(const UT_Vector * pVector,
								   UT_uint32 * pSubscript)
{
	UT_uint32 subscript;
	if (!createAP(&subscript))
		return UT_FALSE;

	PP_AttrProp * pAP = (PP_AttrProp *)m_vecTable.getNthItem(subscript);
	UT_ASSERT(pAP);
	if (!pAP->setAttributes(pVector))
		return UT_FALSE;
	
	*pSubscript = subscript;
	return UT_TRUE;
}

UT_Bool pp_TableAttrProp::findMatch(const XML_Char ** attributes,
									UT_uint32 * pSubscript) const
{
	// return true if we find an AP in our table which is
	// an exact match for the attributes.
	// set *pSubscript to the subscript of the matching item.
	
	if (!attributes || !*attributes)
	{
		// we preloaded the zeroth cell with empty attributes.
		*pSubscript = 0;
		return UT_TRUE;
	}

	// TODO compute a hash on the contents of the attributes and
	// TODO look it up in the table, etc., etc.
	// TODO for now just fail to find a match.

	return UT_FALSE;
}

UT_Bool pp_TableAttrProp::findMatch(const UT_Vector * pVector,
									UT_uint32 * pSubscript) const
{
	// return true if we find an AP in our table which is
	// an exact match for the attributes.
	// set *pSubscript to the subscript of the matching item.
	
	if (!pVector || pVector->getItemCount()==0)
	{
		// we preloaded the zeroth cell with empty attributes.
		*pSubscript = 0;
		return UT_TRUE;
	}

	// TODO compute a hash on the contents of the attributes and
	// TODO look it up in the table, etc., etc.
	// TODO for now just fail to find a match.

	return UT_FALSE;
}

	
const PP_AttrProp * pp_TableAttrProp::getAP(UT_uint32 subscript) const
{
	UT_uint32 count = m_vecTable.getItemCount();
	if (subscript < count)
		return (const PP_AttrProp *)m_vecTable.getNthItem(subscript);
	else
		return NULL;
}

UT_Bool pp_TableAttrProp::cloneWithReplacements(const PP_AttrProp * papOld,
												const XML_Char ** attributes,
												const XML_Char ** properties,
												UT_uint32 * pSubscript)
{
	// create a new AttrProp based upon the given one
	// and adding or replacing the items given.
	// return FALSE on failure.

	// first, create a new AttrProp using just the values given.
	
	UT_uint32 sub;
	if (!createAP(attributes,properties,&sub))
		return UT_FALSE;

	// next, add any items that we have that are not present
	// (have not been overridden) in the new one.

	PP_AttrProp * papNew = (PP_AttrProp *)getAP(sub); // temporarily override the 'const'

	UT_uint32 k;
	const XML_Char * n;
	const XML_Char * v;
	const XML_Char * vNew;
	
	k = 0;
	while (papOld->getNthAttribute(k++,n,v))
	{
		// TODO decide if/whether to allow PT_PROPS_ATTRIBUTE_NAME here.
		// TODO The issue is: we use it to store the CSS properties and
		// TODO when we see it, we expand the value into one or more
		// TODO properties.  if we allow it to be given here, should
		// TODO we blowaway all of the existing properties and create
		// TODO them from this?  or should we expand it and override
		// TODO individual properties?  
		// TODO for now, we just barf on it.
		UT_ASSERT(UT_XML_stricmp(n,PT_PROPS_ATTRIBUTE_NAME)!=0); // cannot handle PROPS here
		if (!papNew->getAttribute(n,vNew))
			if (!papNew->setAttribute(n,v))
				return UT_FALSE;
	}

	k = 0;
	while (papOld->getNthProperty(k++,n,v))
	{
		if (!papNew->getProperty(n,vNew))
			if (!papNew->setProperty(n,v))
				return UT_FALSE;
	}

	*pSubscript = sub;
	return UT_TRUE;
}

