 
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
#include "pd_Document.h"
#include "pt_PieceTable.h"

void PD_Document::__dump(FILE * fp) const
{
	fprintf(fp,"Dump for %s:\n",m_szFilename);
	fprintf(fp,"  Document is %s\n",((m_bDirty) ? "DIRTY" : "CLEAN"));
	
	if (m_pPieceTable)
		m_pPieceTable->__dump(fp);
	UT_TestStatus status = m_pPieceTable->__test_VerifyCoalescedFrags(fp);
}

#endif /* PT_TEST */
