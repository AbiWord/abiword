
#include "pf_Frag_Text.h"

pf_Frag_Text::pf_Frag_Text(UT_uint32 vsIndex,
						   pt_BufPosition offset,
						   UT_uint32 length,
						   pt_AttrPropIndex index)
	: pf_Frag(pf_Frag::PFT_Text)
{
	m_vsIndex = vsIndex;
	m_offset = offset;
	m_length = length;
	m_index = index;
}

pf_Frag_Text::~pf_Frag_Text()
{
}

