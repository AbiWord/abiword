
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
