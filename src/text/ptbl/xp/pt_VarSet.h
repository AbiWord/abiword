/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2016 Hubert Figuière
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


#ifndef PT_VARSET_H
#define PT_VARSET_H

#include "ut_types.h"
#include "ut_growbuf.h"
#include "pt_Types.h"
#include "pp_TableAttrProp.h"

class PD_Document;

class ABI_EXPORT pt_VarSet
{
public:
	pt_VarSet();
	~pt_VarSet();

	void					setPieceTableState(PTState pts);
	bool					appendBuf(const UT_UCSChar * pBuf, UT_uint32 length, PT_BufIndex * pbi);
	bool					storeAP(const PP_PropertyVector & vecAttributes, PT_AttrPropIndex * papi);
	inline const UT_UCSChar *getPointer(PT_BufIndex bi) const {  return (UT_UCSChar *)m_buffer[_varsetFromBufIndex(bi)].getPointer(_subscriptFromBufIndex(bi)); }
	inline PT_BufIndex		getBufIndex(PT_BufIndex bi, UT_uint32 offset) const
	{     return _makeBufIndex(_varsetFromBufIndex(bi),
	                           _subscriptFromBufIndex(bi)+offset);
	}

	bool					isContiguous(PT_BufIndex bi, UT_uint32 length, PT_BufIndex bi2) const;
	inline const PP_AttrProp *getAP(PT_AttrPropIndex api) const
	{
		return m_tableAttrProp[_varsetFromAPIndex(api)].getAP(_subscriptFromAPIndex(api));
	}
	bool					mergeAP(PTChangeFmt ptc,PT_AttrPropIndex apiOld,
									const PP_PropertyVector & attributes,
									const PP_PropertyVector & properties,
									PT_AttrPropIndex * papiNew,
									PD_Document * pDoc);
	bool					addIfUniqueAP(PP_AttrProp * pAP, PT_AttrPropIndex * papi);
    bool                 overwriteBuf(UT_UCSChar * pBuf, UT_uint32 length, PT_BufIndex * pbi);

private:
	inline UT_uint32 _subscriptFromBufIndex(PT_BufIndex bi) const
	{
		return (bi & 0x7fffffff);
	}
	inline UT_uint32 _subscriptFromAPIndex(PT_AttrPropIndex api) const
	{
	    return (api & 0x7fffffff);
	}

	inline UT_uint32 _varsetFromBufIndex(PT_BufIndex bi) const
	{
	    return (bi >> 31);
	}

	inline UT_uint32 _varsetFromAPIndex(PT_AttrPropIndex api) const
	{
	    return (api >> 31);
	}

	inline PT_BufIndex _makeBufIndex(UT_uint32 varset, UT_uint32 subscript) const
	{
	    return ((varset<<31)|subscript);
	}

	inline PT_AttrPropIndex _makeAPIndex(UT_uint32 varset, UT_uint32 subscript) const
	{
		return ((varset<<31)|subscript);
	}

	bool					_finishConstruction(void);

	bool					m_bInitialized;
	UT_uint32				m_currentVarSet;

	UT_GrowBuf				m_buffer[2];
	pp_TableAttrProp		m_tableAttrProp[2];
};


#endif /* PT_VARSET_H */
