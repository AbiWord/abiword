
#include "pf_Frag_Strux_ColumnSet.h"
#include "pc_ColumnSet.h"


pf_Frag_Strux_ColumnSet::pf_Frag_Strux_ColumnSet(pt_PieceTable * pPT,
												 UT_uint32 vsIndex,
												 pt_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_ColumnSet,vsIndex,indexAP)
{
	m_pColumnSet = NULL;
}

pf_Frag_Strux_ColumnSet::~pf_Frag_Strux_ColumnSet()
{
	if (m_pColumnSet)
		delete m_pColumnSet;
}

void pf_Frag_Strux_ColumnSet::dump(FILE * fp) const
{
	fprintf(fp,"      ColumnSet 0x%08lx vs[%d] api[%d]\n",
			(UT_uint32)this,m_vsIndex,m_indexAP);
}
