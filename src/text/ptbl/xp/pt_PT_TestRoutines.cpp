 
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

#ifdef PT_TEST
#include <stdio.h>
#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_test.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Column.h"
#include "pf_Frag_Strux_ColumnSet.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"

/*****************************************************************/
/*****************************************************************/

UT_TestStatus pt_PieceTable::__test_VerifyCoalescedFrags(FILE * fp) const
{
	// Test code to verify that all fragments are properly coalesced.

	fprintf(fp,"__test_VerifyCoalescedFrags: beginning....\n");

	UT_TestStatus status = UT_Test_Pass;

	for (pf_Frag * pf=m_fragments.getFirst(); (pf); pf=pf->getNext())
	{
		if (   (pf->getType() == pf_Frag::PFT_Text)
			&& (pf->getNext())
			&& (pf->getNext()->getType() == pf_Frag::PFT_Text))
		{
			pf_Frag_Text * pft1 = static_cast<pf_Frag_Text *>(pf);
			pf_Frag_Text * pft2 = static_cast<pf_Frag_Text *>(pf->getNext());
			if (   (pft1->getIndexAP() == pft2->getIndexAP())
				&& (m_varset.isContiguous(pft1->getBufIndex(),
										  pft1->getLength(),
										  pft2->getBufIndex())))
			{
				fprintf(fp,"__test_VerifyCoalescedFrags: uncoalesced frags found: p1[0x%08lx] len[%d] p2[0x%08lx]\n",
						(UT_uint32)pft1,pft1->getLength(),(UT_uint32)pft2);
				pft1->__dump(fp);
				pft2->__dump(fp);
				status = UT_Test_Fail;
			}
		}
	}

	fprintf(fp,"__test_VerifyCoalescedFrags: status[%s]\n",UT_TestStatus_GetMessage(status));
	return status;
}

void pt_PieceTable::__dump(FILE * fp) const
{
	// dump the piece table.
	
	fprintf(fp,"  PieceTable: State %d\n",(int)m_pts);
	fprintf(fp,"  PieceTable: Fragments:\n");

	m_fragments.__dump(fp);
}

#endif /* PT_TEST */
