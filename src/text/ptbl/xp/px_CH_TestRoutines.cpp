/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

/*!
  Dump information in the change history
  \param fp File descripter used for output
*/
void
px_ChangeHistory::__dump(FILE * fp) const
{
	// dump the change record history
	
	fprintf(fp,"  ChangeHistory: undo pos %d\n",(int)m_undoPosition);
	fprintf(fp,"  ChangeHistory: save pos %d\n",(int)m_savePosition);
	fprintf(fp,"  ChangeHistory: Change records:\n");

	UT_uint32 kLimit = m_vecChangeRecords.getItemCount();
	UT_uint32 k;
	for (k = 0; k < kLimit; k++)
	{
		PX_ChangeRecord* pcrTemp = 
			(PX_ChangeRecord *) m_vecChangeRecords.getNthItem(k);
		pcrTemp->__dump(fp);
	}
}

#endif /* PT_TEST */
