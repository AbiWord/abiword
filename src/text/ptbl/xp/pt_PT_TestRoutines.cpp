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
				fprintf(fp,"__test_VerifyCoalescedFrags: uncoalesced frags found: p1[%p] len[%ld] p2[%p]\n",
						pft1,pft1->getLength(),pft2);
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
