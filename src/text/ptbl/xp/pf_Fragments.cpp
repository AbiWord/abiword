/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


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
	while (m_pFirst)
	{
		pf_Frag* pNext = m_pFirst->getNext();
		delete m_pFirst;
		m_pFirst = pNext;
	}
	
	m_pLast = NULL;
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
		UT_ASSERT(m_pLast->getType() != pf_Frag::PFT_EndOfDoc);
		
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

pf_Frag * pf_Fragments::getLast(void) const
{
	return m_pLast;
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

void pf_Fragments::unlinkFrag(pf_Frag * pf)
{
	UT_ASSERT(pf->getType() != pf_Frag::PFT_EndOfDoc);
	
	pf_Frag * pn = pf->getNext();
	pf_Frag * pp = pf->getPrev();

	if (pn)
		pn->setPrev(pp);
	if (pp)
		pp->setNext(pn);

	pf->setNext(0);
	pf->setPrev(0);

	if (m_pLast == pf)
		m_pLast = pp;
}
