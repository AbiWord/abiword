
#include "pf_Frag_Strux_Column.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Strux.h"

pf_Frag_Strux_Column::pf_Frag_Strux_Column(pt_PieceTable * pPT,
										   PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_Column,indexAP)
{
}

pf_Frag_Strux_Column::~pf_Frag_Strux_Column()
{
}

void pf_Frag_Strux_Column::dump(FILE * fp) const
{
	fprintf(fp,"      Column 0x%08lx api[%d]\n",
			(UT_uint32)this,m_indexAP);
}
