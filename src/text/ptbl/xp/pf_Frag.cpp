
#include "ut_types.h"
#include "pf_Frag.h"

pf_Frag::pf_Frag(PFType type)
{
	m_type = type;
	m_next = NULL;
	m_prev = NULL;
}

pf_Frag::~pf_Frag()
{
}

pf_Frag::PFType pf_Frag::getType(void) const
{
	return m_type;
}

pf_Frag * pf_Frag::getNext(void) const
{
	return m_next;
}

pf_Frag * pf_Frag::getPrev(void) const
{
	return m_prev;
}

pf_Frag * pf_Frag::setNext(pf_Frag * pNext)
{
	pf_Frag * pOld = m_next;
	m_next = pNext;
	return pOld;
}

pf_Frag * pf_Frag::setPrev(pf_Frag * pPrev)
{
	pf_Frag * pOld = m_prev;
	m_prev = pPrev;
	return pOld;
}
