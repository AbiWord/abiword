
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


#ifdef PT_TEST

#include <stdio.h>
#include "ut_debugmsg.h"
#include "ut_test.h"
#include "pd_Document.h"
#include "pt_PieceTable.h"

void PD_Document::__dump(FILE * fp) const
{
	fprintf(fp,"Dump for %s:\n",getFilename());
	fprintf(fp,"  Document is %s\n",((isDirty()) ? "DIRTY" : "CLEAN"));
	
	if (m_pPieceTable)
		m_pPieceTable->__dump(fp);
	UT_TestStatus status = m_pPieceTable->__test_VerifyCoalescedFrags(fp);
	UT_DEBUGMSG(("PD_Document::__dump: Test %s\n",
				 UT_TestStatus_GetMessage(status)));
}

PD_Document* PD_Document::m_pDoc = NULL;

#endif /* PT_TEST */
