
#include "pf_Frag_Strux_Block.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Strux.h"

pf_Frag_Strux_Block::pf_Frag_Strux_Block(pt_PieceTable * pPT,
										   PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_Block,indexAP)
{
}

pf_Frag_Strux_Block::~pf_Frag_Strux_Block()
{
}

void pf_Frag_Strux_Block::dump(FILE * fp) const
{
	fprintf(fp,"      Block 0x%08lx api[%d]\n",
			(UT_uint32)this,m_indexAP);
}
