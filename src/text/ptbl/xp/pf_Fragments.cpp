
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

void pf_Fragments::dump(FILE * fp) const
{
	pf_Frag * p;

	for (p=m_pFirst; (p); p=p->getNext())
		p->dump(fp);
}
