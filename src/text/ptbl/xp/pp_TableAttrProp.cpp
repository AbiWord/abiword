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
	UT_VECTOR_PURGEALL(PP_AttrProp *, m_vecTable);
}

UT_Bool pp_TableAttrProp::addAP(PP_AttrProp * pAP,
								UT_uint32 * pSubscript)
{
	return (m_vecTable.addItem(pAP,pSubscript) == 0);
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

UT_Bool pp_TableAttrProp::findMatch(const PP_AttrProp * pMatch,
									UT_uint32 * pSubscript) const
{
	// return true if we find an AP in our table which is
	// an exact match for the attributes/properties in pMatch.
	// set *pSubscript to the subscript of the matching item.

	UT_uint32 kLimit = m_vecTable.getItemCount();
	UT_uint32 k;

	for (k=0; (k < kLimit); k++)
	{
		const PP_AttrProp * pK = (const PP_AttrProp *)m_vecTable.getNthItem(k);
		if (pMatch->isExactMatch(pK))
		{
			*pSubscript = k;
			return UT_TRUE;
		}
	}
	
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
