/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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
	m_bInitialized = UT_TRUE;
	return UT_TRUE;
}

void pt_VarSet::setPieceTableState(PTState pts)
{
	if (pts == PTS_Editing)
		m_currentVarSet = 1;
}

UT_Bool pt_VarSet::appendBuf(UT_UCSChar * pBuf, UT_uint32 length, PT_BufIndex * pbi)
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

UT_Bool pt_VarSet::storeAP(const XML_Char ** attributes, PT_AttrPropIndex * papi)
{
	if (!m_bInitialized)
		if (!_finishConstruction())
			return UT_FALSE;

	// create an AP for this set of attributes -- iff unique.
	// return the index for the new one (or the one we found).
	
	UT_uint32 subscript = 0;
	
	for (UT_uint32 k=0; k<2; k++)
		if (m_tableAttrProp[k].findMatch(attributes,&subscript))
		{
			*papi = _makeAPIndex(k,subscript);
			return UT_TRUE;
		}

	// we did not find a match, so we create a new one.
	
	if (m_tableAttrProp[m_currentVarSet].createAP(attributes,NULL,&subscript))
	{
		*papi = _makeAPIndex(m_currentVarSet,subscript);
		return UT_TRUE;
	}

	UT_DEBUGMSG(("could not add AP\n"));
	return UT_FALSE;
}

UT_Bool pt_VarSet::storeAP(const UT_Vector * pVecAttributes, PT_AttrPropIndex * papi)
{
	if (!m_bInitialized)
		if (!_finishConstruction())
			return UT_FALSE;

	// create an AP for this set of attributes -- iff unique.
	// return the index for the new one (or the one we found).
	
	UT_uint32 subscript = 0;
	
	for (UT_uint32 k=0; k<2; k++)
		if (m_tableAttrProp[k].findMatch(pVecAttributes,&subscript))
		{
			*papi = _makeAPIndex(k,subscript);
			return UT_TRUE;
		}

	// we did not find a match, so we create a new one.
	
	if (m_tableAttrProp[m_currentVarSet].createAP(pVecAttributes,&subscript))
	{
		*papi = _makeAPIndex(m_currentVarSet,subscript);
		return UT_TRUE;
	}

	UT_DEBUGMSG(("could not add AP\n"));
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
			UT_uint32 subscript = 0;
			if (m_tableAttrProp[m_currentVarSet].cloneWithReplacements(papOld,attributes,properties,&subscript))
			{
				*papiNew = _makeAPIndex(m_currentVarSet,subscript);
				return UT_TRUE;
			}
		}
		return UT_FALSE;

	case PTC_RemoveFmt:
		{
			UT_ASSERT(0);				// TODO
		}
		return UT_FALSE;
		
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

