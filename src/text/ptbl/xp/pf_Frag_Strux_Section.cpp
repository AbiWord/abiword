
#include "pf_Frag_Strux_Section.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Strux.h"


pf_Frag_Strux_Section::pf_Frag_Strux_Section(pt_PieceTable * pPT,
											 UT_uint32 vsIndex,
											 pt_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_Section,vsIndex,indexAP)
{
}

pf_Frag_Strux_Section::~pf_Frag_Strux_Section()
{
}

UT_Bool pf_Frag_Strux_Section::createSpecialChangeRecord(PX_ChangeRecord ** ppcr) const
{
	UT_ASSERT(ppcr);
	
	PX_ChangeRecord_Strux * pcr
		= new PX_ChangeRecord_Strux(PX_ChangeRecord::PXT_InsertStrux,UT_FALSE,UT_FALSE,
									0, /* doc position is undefined for strux */
									m_vsIndex,
									UT_TRUE, /* bleftside is undefined for strux */
									m_indexAP,m_struxType);
	if (!pcr)
		return UT_FALSE;

	*ppcr = pcr;
	return UT_TRUE;
}

void pf_Frag_Strux_Section::dump(FILE * fp) const
{
	fprintf(fp,"      Section 0x%08lx vs[%d] api[%d]\n",
			(UT_uint32)this,m_vsIndex,m_indexAP);
}
