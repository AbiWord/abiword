
#include "pf_Frag_Strux.h"

pf_Frag_Strux::pf_Frag_Strux(PTStruxType struxType, UT_uint32 vsIndex, pt_AttrPropIndex indexAP)
	: pf_Frag(pf_Frag::PFT_Strux)
{
	m_struxType = struxType;
	m_vsIndex = vsIndex;
	m_indexAP = indexAP;
}

pf_Frag_Strux::~pf_Frag_Strux()
{
}

PTStruxType pf_Frag_Strux::getStruxType(void) const
{
	return m_struxType;
}

