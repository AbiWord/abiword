 
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
#include "pt_Types.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_StruxChange.h"

PX_ChangeRecord_StruxChange::PX_ChangeRecord_StruxChange(PXType type,
														 UT_Byte atomic,
														 PT_DocPosition position,
														 PT_AttrPropIndex indexOldAP,
														 PT_AttrPropIndex indexNewAP,
														 UT_Bool bTempBefore,
														 UT_Bool bTempAfter,
														 PTChangeFmt ptc)
	: PX_ChangeRecord(type, atomic, position, UT_FALSE, indexOldAP, indexNewAP, bTempBefore, bTempAfter)
{
	m_ptc = ptc;
}

PX_ChangeRecord_StruxChange::~PX_ChangeRecord_StruxChange()
{
}

PX_ChangeRecord * PX_ChangeRecord_StruxChange::reverse(void) const
{
	PTChangeFmt ptcRev = (PTChangeFmt)( ! ((UT_Bool) m_ptc));
	
	PX_ChangeRecord_StruxChange * pcr
		= new PX_ChangeRecord_StruxChange(getRevType(),getRevFlags(),
										  m_position,
										  m_indexAP,m_indexOldAP,
										  m_bTempAfter,m_bTempBefore,
										  ptcRev);
	UT_ASSERT(pcr);
	return pcr;
}
