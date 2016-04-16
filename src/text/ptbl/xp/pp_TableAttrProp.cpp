/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
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


#include "ut_types.h"
#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_string.h"
#include "pp_AttrProp.h"
#include "pp_TableAttrProp.h"


/*!
 * This static function is used to compare PP_AttrProp's for the qsort method of UT_Vector
\param vX1 pointer to a PP_AttrProp value.
\param vX2 pointer to a second PP_AttrProp value
*/
static UT_sint32 compareAP(const void * vX1, const void * vX2)
{
	PP_AttrProp *x1 = *(PP_AttrProp **)(vX1);
	PP_AttrProp *x2 = *(PP_AttrProp **)(vX2);

	UT_uint32 u1 = x1->getCheckSum();
	UT_uint32 u2 = x2->getCheckSum();

	if (u1 < u2) return -1;
	if (u1 > u2) return 1;
	return 0;
}

/*!
 * This static function is used to compare PP_AttrProp's for the
 * binarysearch method of UT_Vector
\param vX1 pointer to a PP_AttrProp value.
\param vX2 pointer to a second PP_AttrProp value
*/
static UT_sint32 compareAPBinary(const void * vX1, const void * vX2)
{
//
// vX1 is actually a pointer to a UT_uint32 key value (a checkSum)
//
	UT_uint32 u1 = *((UT_uint32*) (vX1));
	PP_AttrProp *x2 = *(PP_AttrProp **)(vX2);
	UT_uint32 u2 = x2->getCheckSum();

	if (u1 < u2) return -1;
	if (u1 > u2) return 1;
	return 0;
}

pp_TableAttrProp::pp_TableAttrProp():
	m_vecTable(54,4,true), // there seems to be 50+ of these at the moment
	m_vecTableSorted(54,4,true)
{
}

pp_TableAttrProp::~pp_TableAttrProp()
{
	UT_VECTOR_PURGEALL(PP_AttrProp *, m_vecTable);
}

bool pp_TableAttrProp::addAP(PP_AttrProp * pAP,
								UT_sint32 * pSubscript)
{
 	UT_sint32 u;
 	bool result = (m_vecTable.addItem(pAP,&u) == 0);
 
 	if (result)
 	{
 		if (pSubscript)
 		{
 			*pSubscript = u;
 		}
 		pAP->setIndex(u);	//$HACK
 		result = (m_vecTableSorted.addItemSorted(pAP,compareAP) == 0);
 	}
 
 	return result;
}

bool pp_TableAttrProp::createAP(UT_sint32 * pSubscript)
{
	PP_AttrProp * pNew = new PP_AttrProp();
	if (!pNew)
		return false;
 	UT_sint32 u;
 	if (m_vecTable.addItem(pNew,&u) != 0)
	{
		delete pNew;
		return false;
	}

	pNew->setIndex(u);	//$HACK

	if (pSubscript)
 	{
 		*pSubscript = u;
 	}
	else
	{
		// create default empty AP
		pNew->markReadOnly();
		m_vecTableSorted.addItem(pNew, NULL);
	} 

	return true;
}

bool pp_TableAttrProp::createAP(const PP_PropertyVector & attributes,
								   const PP_PropertyVector & properties,
								   UT_sint32 * pSubscript)
{
	UT_sint32 subscript;
	if (!createAP(&subscript))
		return false;

	PP_AttrProp * pAP = m_vecTable.getNthItem(subscript);
	UT_return_val_if_fail (pAP,false);
	if (!pAP->setAttributes(attributes) || !pAP->setProperties(properties))
		return false;

	pAP->markReadOnly();

	m_vecTableSorted.addItemSorted(pAP,compareAP);

	*pSubscript = subscript;
	return true;
}

bool pp_TableAttrProp::createAP(const PP_PropertyVector & pVector,
								   UT_sint32 * pSubscript)
{
	UT_sint32 subscript;
	if (!createAP(&subscript))
		return false;

	PP_AttrProp * pAP = m_vecTable.getNthItem(subscript);
	UT_return_val_if_fail (pAP, false);
	if (!pAP->setAttributes(pVector))
		return false;

	pAP->markReadOnly();

	m_vecTableSorted.addItemSorted(pAP,compareAP);

	*pSubscript = subscript;
	return true;
}

bool pp_TableAttrProp::findMatch(const PP_AttrProp * pMatch,
									UT_sint32 * pSubscript) const
{
	// return true if we find an AP in our table which is
	// an exact match for the attributes/properties in pMatch.
	// set *pSubscript to the subscript of the matching item.

	UT_sint32 kLimit = m_vecTable.getItemCount();
	UT_sint32 k;
  
	UT_uint32 checksum = pMatch->getCheckSum();
 	k = m_vecTableSorted.binarysearch(reinterpret_cast<void *>(&checksum), compareAPBinary);
 	UT_uint32 cksum = pMatch->getCheckSum();
 
 	if (k == -1)
 	{
 		k = kLimit;
 	}
 
 	for (; (k < kLimit); k++)
  	{
 		PP_AttrProp * pK = (PP_AttrProp *)m_vecTableSorted.getNthItem(k);
 		if (cksum != pK->getCheckSum())
 		{
 			break;
 		}
		if (pMatch->isExactMatch(*pK))
  		{
 			// Need to return an index of the element in the MAIN
 			// vector table
			*pSubscript = pK->getIndex();
			return true;
		}
	}
	return false;
}

	
const PP_AttrProp * pp_TableAttrProp::getAP(UT_sint32 subscript) const
{
	UT_sint32 count = m_vecTable.getItemCount();
	if (subscript < count)
		return (const PP_AttrProp *)m_vecTable.getNthItem(subscript);
	else
		return NULL;
}

