
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "pf_Fragments.h"
#include "pf_Frag.h"

pf_Fragments::pf_Fragments()
{
	m_pFirst = NULL;
	m_pLast = NULL;
}

pf_Fragments::~pf_Fragments()
{
	// TODO decide if we should kill the list or if it already has been
}

void pf_Fragments::appendFrag(pf_Frag * pf)
{
	// append a frag to the end of the list
	
	UT_ASSERT(pf);
	
	if (!m_pLast)
	{
		UT_ASSERT(!m_pFirst);
		m_pFirst = pf;
		m_pLast = pf;
		pf->setNext(NULL);
		pf->setPrev(NULL);
	}
	else
	{
		m_pLast->setNext(pf);
		pf->setPrev(m_pLast);
		m_pLast = pf;
		pf->setNext(NULL);
	}
	return;
}

pf_Frag * pf_Fragments::getFirst(void) const
{
	return m_pFirst;
}

void pf_Fragments::insertFrag(pf_Frag * pfPlace, pf_Frag * pfNew)
{
	// insert the new fragment after the given fragment.

	UT_ASSERT(pfPlace);
	UT_ASSERT(pfNew);

	pf_Frag * pfQ = pfPlace->getNext();
	
	pfNew->setNext(pfQ);
	if (pfQ)
		pfQ->setPrev(pfNew);
	pfNew->setPrev(pfPlace);
	pfPlace->setNext(pfNew);
	if (m_pLast == pfPlace)
		m_pLast = pfNew;
}

void pf_Fragments::dump(FILE * fp) const
{
	pf_Frag * p;

	for (p=m_pFirst; (p); p=p->getNext())
		p->dump(fp);
}
