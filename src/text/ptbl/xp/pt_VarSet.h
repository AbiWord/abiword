 
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

#ifndef PT_VARSET_H
#define PT_VARSET_H

#include "ut_types.h"
#include "ut_growbuf.h"
#include "pt_Types.h"
#include "pp_TableAttrProp.h"


class pt_VarSet
{
public:
	pt_VarSet();
	~pt_VarSet();
	
	void					setPieceTableState(PTState pts);
	UT_Bool					appendBuf(UT_UCSChar * pBuf, UT_uint32 length, PT_BufIndex * pbi);
	UT_Bool					storeAP(const XML_Char ** attributes, PT_AttrPropIndex * papi);
	UT_Bool					storeAP(const UT_Vector * pVecAttributes, PT_AttrPropIndex * papi);
	const UT_UCSChar *		getPointer(PT_BufIndex bi) const;
	PT_BufIndex				getBufIndex(PT_BufIndex bi, UT_uint32 offset) const;
	UT_Bool					isContiguous(PT_BufIndex bi, UT_uint32 length, PT_BufIndex bi2) const;
	const PP_AttrProp *		getAP(PT_AttrPropIndex api) const;
	UT_Bool					mergeAP(PTChangeFmt ptc,PT_AttrPropIndex apiOld,
									const XML_Char ** attributes, const XML_Char ** properties,
									PT_AttrPropIndex * papiNew);
	
private:
	inline UT_uint32		_subscriptFromBufIndex(PT_BufIndex bi) const;
	inline UT_uint32		_subscriptFromAPIndex(PT_AttrPropIndex api) const;
	inline UT_uint32		_varsetFromBufIndex(PT_BufIndex bi) const;
	inline UT_uint32		_varsetFromAPIndex(PT_AttrPropIndex api) const;
	inline PT_BufIndex		_makeBufIndex(UT_uint32 varset, UT_uint32 subscript) const;
	inline PT_AttrPropIndex	_makeAPIndex(UT_uint32 varset, UT_uint32 subscript) const;
	UT_Bool					_finishConstruction(void);

	UT_Bool					m_bInitialized;
	UT_uint32				m_currentVarSet;

	UT_GrowBuf				m_buffer[2];
	pp_TableAttrProp		m_tableAttrProp[2];
};

	
#endif /* PT_VARSET_H */
