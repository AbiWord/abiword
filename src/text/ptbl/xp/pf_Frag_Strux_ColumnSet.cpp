
#include "pf_Frag_Strux_ColumnSet.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Strux.h"


pf_Frag_Strux_ColumnSet::pf_Frag_Strux_ColumnSet(pt_PieceTable * pPT,
												 PT_VarSetIndex vsIndex,
												 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_ColumnSet,vsIndex,indexAP)
{
}

pf_Frag_Strux_ColumnSet::~pf_Frag_Strux_ColumnSet()
{
}

void pf_Frag_Strux_ColumnSet::dump(FILE * fp) const
{
	fprintf(fp,"      ColumnSet 0x%08lx vs[%d] api[%d]\n",
			(UT_uint32)this,m_vsIndex,m_indexAP);
}
