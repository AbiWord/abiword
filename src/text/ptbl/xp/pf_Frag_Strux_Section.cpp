 
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

#include "pf_Frag_Strux_Section.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Strux.h"


pf_Frag_Strux_Section::pf_Frag_Strux_Section(pt_PieceTable * pPT,
											 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_Section,indexAP)
{
}

pf_Frag_Strux_Section::~pf_Frag_Strux_Section()
{
}

void pf_Frag_Strux_Section::dump(FILE * fp) const
{
	fprintf(fp,"      Section 0x%08lx api[%d]\n",
			(UT_uint32)this,m_indexAP);
}
