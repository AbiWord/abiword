 
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

#include "ut_types.h"
#include "ut_test.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Column.h"
#include "pf_Frag_Strux_ColumnSet.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pt_PieceTable.h"


/*****************************************************************/
/*****************************************************************/

void pf_Frag::__dump(FILE * fp) const
{
	fprintf(fp,"      BaseDump 0x%08lx type[%d]\n",(UT_uint32)this,m_type);
}

void pf_Frag_Strux_Block::__dump(FILE * fp) const
{
	fprintf(fp,"      Block 0x%08lx api[%08lx] preferredSpanAPI[%08lx]\n",
			(UT_uint32)this,m_indexAP,m_preferredSpanAPI);
}

void pf_Frag_Strux_Column::__dump(FILE * fp) const
{
	fprintf(fp,"      Column 0x%08lx api[%08lx]\n",
			(UT_uint32)this,m_indexAP);
}

void pf_Frag_Strux_ColumnSet::__dump(FILE * fp) const
{
	fprintf(fp,"      ColumnSet 0x%08lx api[%08lx]\n",
			(UT_uint32)this,m_indexAP);
}

void pf_Frag_Strux_Section::__dump(FILE * fp) const
{
	fprintf(fp,"      Section 0x%08lx api[%08lx]\n",
			(UT_uint32)this,m_indexAP);
}

void pf_Frag_Text::__dump(FILE * fp) const
{
	fprintf(fp,"      TextFragment 0x%08lx b[%08lx,%ld] api[%08lx]\n",
			(UT_uint32)this,m_bufIndex,m_length,m_indexAP);

	const UT_UCSChar * ptr = m_pPieceTable->getPointer(m_bufIndex);
	char c;
	UT_uint32 k;

	fprintf(fp,"\t[");
	for (k=0; k<m_length; k++)
	{
		// note: this is a cheap unicode to ascii conversion for
		// note: debugging purposes only.
		c = (  ((ptr[k] < 20) || (ptr[k] > 0x7f))
			   ? '@'
			   : (char)ptr[k]);
		fprintf(fp,"%c",c);
	}
	fprintf(fp,"]\n");
}

#endif /* PT_TEST */
