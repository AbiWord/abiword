 
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
#include "px_ChangeRecord_Strux.h"
#include "px_ChangeRecord.h"

PX_ChangeRecord_Strux::PX_ChangeRecord_Strux(PXType type,
											 PT_DocPosition position,
											 PT_AttrPropIndex indexAP,
											 PTStruxType struxType)
	: PX_ChangeRecord(type, position, indexAP)
{
	m_struxType = struxType;
}

PX_ChangeRecord_Strux::~PX_ChangeRecord_Strux()
{
}

PX_ChangeRecord * PX_ChangeRecord_Strux::reverse(void) const
{
	PX_ChangeRecord_Strux * pcr
		= new PX_ChangeRecord_Strux(getRevType(),m_position,m_indexAP,m_struxType);
	UT_ASSERT(pcr);
	return pcr;
}

PTStruxType PX_ChangeRecord_Strux::getStruxType(void) const
{
	return m_struxType;
}

