
#include "pf_Frag_Strux_Column.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Strux.h"

pf_Frag_Strux_Column::pf_Frag_Strux_Column(pt_PieceTable * pPT,
										   UT_uint32 vsIndex,
										   pt_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_Column,vsIndex,indexAP)
{
}

pf_Frag_Strux_Column::~pf_Frag_Strux_Column()
{
}

void pf_Frag_Strux_Column::dump(FILE * fp) const
{
	fprintf(fp,"      Column 0x%08lx vs[%d] api[%d]\n",
			(UT_uint32)this,m_vsIndex,m_indexAP);
}
