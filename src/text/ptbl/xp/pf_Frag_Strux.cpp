
#include "pf_Frag_Strux.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Strux.h"


pf_Frag_Strux::pf_Frag_Strux(pt_PieceTable * pPT,
							 PTStruxType struxType,
							 PT_VarSetIndex vsIndex,
							 PT_AttrPropIndex indexAP)
	: pf_Frag(pPT, pf_Frag::PFT_Strux)
{
	m_struxType = struxType;
	m_vsIndex = vsIndex;
	m_indexAP = indexAP;
}

pf_Frag_Strux::~pf_Frag_Strux()
{
	// we do not purge the items in m_vecFmtHandle
	// since we did not allocate them.
}

PTStruxType pf_Frag_Strux::getStruxType(void) const
{
	return m_struxType;
}

PL_StruxFmtHandle pf_Frag_Strux::getFmtHandle(PL_ListenerId lid) const
{
	return (PL_StruxFmtHandle)m_vecFmtHandle.getNthItem(lid);
}

UT_Bool pf_Frag_Strux::setFmtHandle(PL_ListenerId lid, PL_StruxFmtHandle sfh)
{
	UT_uint32 kLimit = m_vecFmtHandle.getItemCount();
	if (lid < kLimit)
		return (m_vecFmtHandle.setNthItem(lid,(void *)sfh,NULL) == 0);
	else if (lid == kLimit)
		return (m_vecFmtHandle.addItem((void *)sfh) == 0);

	// TODO we need to fix the vector class so that i can do a setNthItem
	// TODO and have it automatically grow the vector -- or we need to do
	// TODO an addItem(0) until we get to lid-1 and then addItem(sfh).
	UT_ASSERT((0));
	return UT_FALSE;
}

UT_Bool pf_Frag_Strux::createSpecialChangeRecord(PX_ChangeRecord ** ppcr) const
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
