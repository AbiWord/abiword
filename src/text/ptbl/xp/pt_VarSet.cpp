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
#include "ut_debugmsg.h"
#include "pt_Types.h"
#include "pt_VarSet.h"


pt_VarSet::pt_VarSet()
{
	m_currentVarSet = 0;
	m_bInitialized = UT_FALSE;
}

pt_VarSet::~pt_VarSet()
{
}

UT_Bool pt_VarSet::_finishConstruction(void)
{
	// finish the construction -- C++ doesn't let us return failures
	// in a constructor, so we do the malloc's here.
		
	// create a default A/P as entry zero in each AP table.

	PT_AttrPropIndex foo;
		
	if (   !m_tableAttrProp[0].createAP(&foo)
		|| !m_tableAttrProp[1].createAP(&foo))
		return UT_FALSE;
	((PP_AttrProp *)getAP(_makeAPIndex(0,0)))->markReadOnly();
	((PP_AttrProp *)getAP(_makeAPIndex(1,0)))->markReadOnly();
	
	m_bInitialized = UT_TRUE;
	return UT_TRUE;
}

void pt_VarSet::setPieceTableState(PTState pts)
{
	if (pts == PTS_Editing)
		m_currentVarSet = 1;
}

UT_Bool pt_VarSet::appendBuf(const UT_UCSChar * pBuf, UT_uint32 length, PT_BufIndex * pbi)
{
	UT_uint32 bufOffset = m_buffer[m_currentVarSet].getLength();
	if (m_buffer[m_currentVarSet].ins(bufOffset,pBuf,length))
	{
		*pbi = _makeBufIndex(m_currentVarSet,bufOffset);
		return UT_TRUE;
	}

	UT_DEBUGMSG(("could not appendBuf\n"));
	return UT_FALSE;
}

UT_Bool pt_VarSet::overwriteBuf(UT_UCSChar * pBuf, UT_uint32 length, PT_BufIndex * pbi)
{
	if (m_buffer[_varsetFromBufIndex(*pbi)]
        .overwrite(_subscriptFromBufIndex(*pbi),
                   pBuf,
                   length))
	{
		return UT_TRUE;
	}

	UT_DEBUGMSG(("could not overwriteBuf\n"));
	return UT_FALSE;
}



UT_Bool pt_VarSet::storeAP(const XML_Char ** attributes, PT_AttrPropIndex * papi)
{
	if (!m_bInitialized)
		if (!_finishConstruction())
			return UT_FALSE;

	// create an AP for this set of attributes -- iff unique.
	// return the index for the new one (or the one we found).

	if (!attributes || !*attributes)
	{
		// we preloaded the zeroth cell (of both tables)
		// with empty attributes.  return index back to
		// the first table.
		*papi = _makeAPIndex(0,0);
		return UT_TRUE;
	}

	// This is a bit expensive, but it can't be helped.
	// We synthesize a (possibly temporary) AP to organize
	// and store the values given in the args.  We then use
	// it to find one that matches it.  If we find it, we
	// destroy the one we just created.  Otherwise, we add
	// the new one to the correct table and return it.

	PP_AttrProp * pTemp = new PP_AttrProp();
	if (!pTemp)
		return UT_FALSE;

	if (pTemp->setAttributes(attributes))
	{
		pTemp->markReadOnly();
		return addIfUniqueAP(pTemp,papi);
	}
	
	delete pTemp;
	return UT_FALSE;
}

UT_Bool pt_VarSet::storeAP(const UT_Vector * pVecAttributes, PT_AttrPropIndex * papi)
{
	if (!m_bInitialized)
		if (!_finishConstruction())
			return UT_FALSE;

	// create an AP for this set of attributes -- iff unique.
	// return the index for the new one (or the one we found).
	
	if (!pVecAttributes || pVecAttributes->getItemCount()==0)
	{
		// we preloaded the zeroth cell (of both tables)
		// with empty attributes.  return index back to
		// the first table.
		*papi = _makeAPIndex(0,0);
		return UT_TRUE;
	}

	// This is a bit expensive, but it can't be helped.
	// We synthesize a (possibly temporary) AP to organize
	// and store the values given in the args.  We then use
	// it to find one that matches it.  If we find it, we
	// destroy the one we just created.  Otherwise, we add
	// the new one to the correct table and return it.

	PP_AttrProp * pTemp = new PP_AttrProp();
	if (!pTemp)
		return UT_FALSE;
	
	if (pTemp->setAttributes(pVecAttributes))
	{
		pTemp->markReadOnly();
		return addIfUniqueAP(pTemp,papi);
	}

	delete pTemp;
	return UT_FALSE;
}

const UT_UCSChar * pt_VarSet::getPointer(PT_BufIndex bi) const
{
	return m_buffer[_varsetFromBufIndex(bi)].getPointer(_subscriptFromBufIndex(bi));
}

PT_BufIndex pt_VarSet::getBufIndex(PT_BufIndex bi, UT_uint32 offset) const
{
	return _makeBufIndex(_varsetFromBufIndex(bi),
						 _subscriptFromBufIndex(bi)+offset);
}

UT_Bool pt_VarSet::isContiguous(PT_BufIndex bi, UT_uint32 length, PT_BufIndex bi2) const
{
	return ((getPointer(bi)+length) == getPointer(bi2));
}

const PP_AttrProp * pt_VarSet::getAP(PT_AttrPropIndex api) const
{
	return m_tableAttrProp[_varsetFromAPIndex(api)].getAP(_subscriptFromAPIndex(api));
}

UT_uint32 pt_VarSet::_subscriptFromBufIndex(PT_BufIndex bi) const
{
	return (bi & 0x7fffffff);
}

UT_uint32 pt_VarSet::_subscriptFromAPIndex(PT_AttrPropIndex api) const
{
	return (api & 0x7fffffff);
}

UT_uint32 pt_VarSet::_varsetFromBufIndex(PT_BufIndex bi) const
{
	return (bi >> 31);
}

UT_uint32 pt_VarSet::_varsetFromAPIndex(PT_AttrPropIndex api) const
{
	return (api >> 31);
}

PT_BufIndex pt_VarSet::_makeBufIndex(UT_uint32 varset, UT_uint32 subscript) const
{
	return ((varset<<31)|subscript);
}

PT_AttrPropIndex pt_VarSet::_makeAPIndex(UT_uint32 varset, UT_uint32 subscript) const
{
	return ((varset<<31)|subscript);
}

UT_Bool pt_VarSet::mergeAP(PTChangeFmt ptc, PT_AttrPropIndex apiOld,
						   const XML_Char ** attributes, const XML_Char ** properties,
						   PT_AttrPropIndex * papiNew)
{
	// merge the given attr/props with set referenced by apiOld
	// under the operator ptc giving a new set to be returned in
	// papiNew.  if the resulting merger is the same as the set
	// referenced in apiOld, just return it.
	// return UT_FALSE only if we had an error.

	const PP_AttrProp * papOld = getAP(apiOld);
	switch (ptc)
	{
	case PTC_AddFmt:
		{
			if (papOld->areAlreadyPresent(attributes,properties))
			{
				*papiNew = apiOld;
				return UT_TRUE;
			}

			// create a new AP that is the merger of the existing
			// one and the a/p given in the args.  we then use it
			// to find an existing match or use the new one.
			
			PP_AttrProp * pNew = papOld->cloneWithReplacements(attributes,properties, UT_FALSE);
			if (!pNew)
				return UT_FALSE;

			pNew->markReadOnly();
			return addIfUniqueAP(pNew,papiNew);
		}

	case PTC_AddStyle:
		{
			if (!papOld->hasProperties() && 
				papOld->areAlreadyPresent(attributes,properties))
			{
				*papiNew = apiOld;
				return UT_TRUE;
			}

			// create a new AP that is the merger of the existing
			// one (with its props cleared) and the a/p given in 
			// the args.  we then use it to find an existing match 
			// or use the new one.
			
			PP_AttrProp * pNew = papOld->cloneWithReplacements(attributes,properties, UT_TRUE);
			if (!pNew)
				return UT_FALSE;

			pNew->markReadOnly();
			return addIfUniqueAP(pNew,papiNew);
		}

	case PTC_RemoveFmt:
		{
			if (!papOld->areAnyOfTheseNamesPresent(attributes,properties))
			{
				// none of the given attributes/properties are present in this set.
				// the remove has no effect.
				*papiNew = apiOld;
				return UT_TRUE;
			}

			// create a new AP that is the existing set minus the
			// ones given in the args.  we then use it to find an
			// existing match or use the new one.

			PP_AttrProp * pNew = papOld->cloneWithElimination(attributes,properties);
			if (!pNew)
				return UT_FALSE;

			pNew->markReadOnly();
			return addIfUniqueAP(pNew,papiNew);
		}
		
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

UT_Bool pt_VarSet::addIfUniqueAP(PP_AttrProp * pAP, PT_AttrPropIndex * papi)
{
	// Add the AP to our tables iff it is unique.
	// If not unique, delete it and return the index
	// of the one that matches.  If it is unique, add
	// it and return the index where we added it.
	// return false if we have any errors.

	UT_ASSERT(pAP && papi);
	UT_uint32 subscript = 0;
	UT_uint32 table = 0;
	
	for (table=0; table<2; table++)
		if (m_tableAttrProp[table].findMatch(pAP,&subscript))
		{
			// use the one that we already have in the table.
			delete pAP;
			*papi = _makeAPIndex(table,subscript);
			return UT_TRUE;
		}

	// we did not find a match, so we store our new one.
	
	if (m_tableAttrProp[m_currentVarSet].addAP(pAP,&subscript))
	{
		*papi = _makeAPIndex(m_currentVarSet,subscript);
		return UT_TRUE;
	}
	
	// memory error of some kind.
	
	delete pAP;
	return UT_FALSE;
}
