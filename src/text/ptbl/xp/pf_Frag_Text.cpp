
#include "pf_Frag_Text.h"
#include "pt_PieceTable.h"
#include "px_ChangeRecord.h"


pf_Frag_Text::pf_Frag_Text(pt_PieceTable * pPT,
						   UT_uint32 vsIndex,
						   pt_BufPosition offset,
						   UT_uint32 length,
						   pt_AttrPropIndex indexAP)
	: pf_Frag(pPT,pf_Frag::PFT_Text)
{
	m_vsIndex = vsIndex;
	m_offset = offset;
	m_length = length;
	m_indexAP = indexAP;
}

pf_Frag_Text::~pf_Frag_Text()
{
}

UT_Bool pf_Frag_Text::createSpecialChangeRecord(PX_ChangeRecord ** ppcr) const
{
	UT_ASSERT(ppcr);
	
	PX_ChangeRecord * pcr
#if 1
		= NULL;
#else
	// TODO -- jeff, what goes here?
		= new PX_ChangeRecord_Strux(PX_ChangeRecord::PXT_InsertStrux,UT_FALSE,UT_FALSE,
									0, /* doc position is undefined for strux */
									m_vsIndex,
									UT_TRUE, /* bleftside is undefined for strux */
									m_indexAP,m_struxType);
#endif
	if (!pcr)
		return UT_FALSE;

	*ppcr = pcr;
	return UT_TRUE;
}

void pf_Frag_Text::dump(FILE * fp) const
{
	fprintf(fp,"      TextFragment 0x%08lx vs[%d] b[%d,%d] api[%d]\n",
			(UT_uint32)this,m_vsIndex,m_offset,m_length,m_indexAP);

	const UT_uint16 * ptr = m_pPieceTable->getBuffer(m_vsIndex)->getPointer(m_offset);
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
