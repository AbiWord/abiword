 
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

#include "pf_Frag_Text.h"
#include "pt_PieceTable.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Span.h"


pf_Frag_Text::pf_Frag_Text(pt_PieceTable * pPT,
						   PT_BufIndex bufIndex,
						   UT_uint32 length,
						   PT_AttrPropIndex indexAP)
	: pf_Frag(pPT,pf_Frag::PFT_Text,length)
{
	m_bufIndex = bufIndex;
	m_indexAP = indexAP;
}

pf_Frag_Text::~pf_Frag_Text()
{
}

PT_BufIndex pf_Frag_Text::getBufIndex(void) const
{
	return m_bufIndex;
}

PT_AttrPropIndex pf_Frag_Text::getIndexAP(void) const
{
	return m_indexAP;
}

void pf_Frag_Text::setIndexAP(PT_AttrPropIndex indexNewAP)
{
	m_indexAP = indexNewAP;
}

UT_Bool pf_Frag_Text::createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
												PT_DocPosition dpos) const
{
	UT_ASSERT(ppcr);
	
	PX_ChangeRecord * pcr
		= new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_InsertSpan,
								   PX_ChangeRecord::PXF_Null,
								   dpos,
								   m_indexAP,m_indexAP,
								   UT_FALSE,UT_FALSE,
								   m_bufIndex,m_length);
	if (!pcr)
		return UT_FALSE;

	*ppcr = pcr;
	return UT_TRUE;
}

void pf_Frag_Text::changeLength(UT_uint32 newLength)
{
	UT_ASSERT(newLength > 0);
	m_length = newLength;
}

void pf_Frag_Text::adjustOffsetLength(PT_BufIndex bi, UT_uint32 newLength)
{
	m_bufIndex = bi;
	m_length = newLength;
}

void pf_Frag_Text::dump(FILE * fp) const
{
#ifdef UT_DEBUG
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
#endif
}
