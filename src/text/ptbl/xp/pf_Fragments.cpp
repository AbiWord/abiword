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
#include "ut_debugmsg.h"
#include "pf_Frag_Strux.h"

pf_Fragments::pf_Fragments()
	: m_pFirst(0),
	  m_pLast(0),
	  m_bAreFragsClean(false),
	  m_pCache(0)
{
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
	
	UT_return_if_fail (pf);
	if (!m_pLast)
	{
		UT_ASSERT_HARMLESS(!m_pFirst);
		m_pFirst = pf;
		m_pLast = pf;
		pf->setNext(NULL);
		pf->setPrev(NULL);
	}
	else
	{
		UT_ASSERT_HARMLESS(m_pLast->getType() != pf_Frag::PFT_EndOfDoc);
		
		m_pLast->setNext(pf);
		pf->setPrev(m_pLast);
		m_pLast = pf;
		pf->setNext(NULL);
	}
	setFragsDirty();
	
	return;
}

pf_Frag * pf_Fragments::getFirst() const
{
	return m_pFirst;
}

pf_Frag * pf_Fragments::getLast() const
{
	return m_pLast;
}

void pf_Fragments::insertFrag(pf_Frag * pfPlace, pf_Frag * pfNew)
{
	// insert the new fragment after the given fragment.
	UT_return_if_fail (pfPlace);
	UT_return_if_fail (pfNew);

	xxx_UT_DEBUGMSG(("Inserting frag %x of type %d after frag %x of type %d\n",pfNew,pfNew->getType(),pfPlace,pfPlace->getType()));

	pf_Frag * pfQ = pfPlace->getNext();
	
	pfNew->setNext(pfQ);
	if (pfQ)
		pfQ->setPrev(pfNew);
	pfNew->setPrev(pfPlace);
	pfPlace->setNext(pfNew);
	if (m_pLast == pfPlace)
		m_pLast = pfNew;
	setFragsDirty();
}

void pf_Fragments::insertFragBefore(pf_Frag * pfPlace, pf_Frag * pfNew)
{
	// insert the new fragment after the given fragment.
	UT_return_if_fail (pfPlace);
	UT_return_if_fail (pfNew);

	xxx_UT_DEBUGMSG(("Inserting frag %x of type %d after frag %x of type %d\n",pfNew,pfNew->getType(),pfPlace,pfPlace->getType()));

	pf_Frag * pfQ = pfPlace->getPrev();
	
	pfNew->setPrev(pfQ);
	if (pfQ)
		pfQ->setNext(pfNew);
	pfNew->setNext(pfPlace);
	pfPlace->setPrev(pfNew);
	if (m_pFirst == pfPlace)
		m_pFirst = pfNew;
	setFragsDirty();
}

void pf_Fragments::unlinkFrag(pf_Frag * pf)
{
	// NOTE:  it is the caller's responsibility to delete pf if appropriate.
	
	UT_return_if_fail (pf->getType() != pf_Frag::PFT_EndOfDoc);
	pf_Frag * pn = pf->getNext();
	pf_Frag * pp = pf->getPrev();
	setFragsDirty();
	if (pn)
	{
		pn->setPrev(pp);
	}

	if (pp)
		pp->setNext(pn);

	pf->setNext(0);
	pf->setPrev(0);

	if (m_pLast == pf)
		m_pLast = pp;
	if(m_pFirst == pf)
	        m_pFirst = pn;
	if (getCache() == pf)
		setCache(pp);
}

/*!
 * This method clears out and repopulates the vector of pointers to fragments.
 * It also sets the doc Positions of all the fragments.
 */
void pf_Fragments::cleanFrags(void) const
{
	if (m_vecFrags.getItemCount() > 0)
		m_vecFrags.clear();

	pf_Frag * pfLast = NULL;
	PT_DocPosition sum = 0;
	for (pf_Frag * pf = getFirst(); (pf); pf=pf->getNext())
	{
		pf->setPos(sum);
		sum += pf->getLength();
		pfLast = pf;
		m_vecFrags.addItem((void *) pf);
	}
	
	UT_return_if_fail (pfLast /*&& (pfLast->getType() == pf_Frag::PFT_EndOfDoc)*/);
	xxx_UT_DEBUGMSG(("Found %d Frags dopos at end = %d \n",m_vecFrags.getItemCount(),getLast()->getPos()));
	m_bAreFragsClean = true;
	setCache(NULL);
}

pf_Frag * pf_Fragments::getNthFrag(UT_uint32 nthFrag) const
{
	if (areFragsDirty())
	{
		xxx_UT_DEBUGMSG(("JCA: getNthFrag (%d): Cleanning fragments ( O(n) complexity! )\n", nthFrag));
		cleanFrags();
	}
	else 
	{
		xxx_UT_DEBUGMSG(("JCA: getNthFrag (%d): Don't need to clean fragments\n", nthFrag));
	}
	
	if (m_vecFrags.getItemCount() > 0)
	{
		xxx_UT_DEBUGMSG(("JCA: getNthFrag (%d): returning frag %p\n", nthFrag, m_vecFrags.getNthItem(nthFrag)));
		return (pf_Frag *) m_vecFrags.getNthItem(nthFrag);
	}

	return NULL;
}

/*!
 * Binary search to find the first frag at position before pos
 * @param PT_DocPosition we want to find for.
 * @returns pf_Frag * pointer to the Frag with position immediately before pos
*/
pf_Frag * pf_Fragments::findFirstFragBeforePos(PT_DocPosition pos) const
{
	UT_uint32 numFrags = getNumberOfFrags();
#ifdef DEBUG
	UT_uint32 numIters = 0;
#endif
	xxx_UT_DEBUGMSG(("JCA: findFirstFragBeforePos (%d).  NbFrags = %d...\n", pos, numFrags));

	if (numFrags < 1)
		return NULL;

	pf_Frag * last = getLast();
	if (last && pos >= last->getPos())
	{
		xxx_UT_DEBUGMSG(("JCA: Found last Frag[%p] @ pos %d Looking for pos %d \n", getLast(), last->getPos(), pos));
		return last;
	}

	pf_Frag* cache = getCache();
	if (cache && pos >= cache->getPos() && pos < cache->getPos() + cache->getLength())
   	{
		return cache;
	}
//
//Look in the next Frag now
// 
	if(cache)
	{
		cache = cache->getNext();
		if (cache && pos >= cache->getPos() && pos < cache->getPos() + cache->getLength())
		{
			setCache(cache);
			return cache;
		}
	}
//
// OK do a binary search.
//

	UT_sint32 diff = numFrags / 2;
	UT_sint32 curFragNo = diff;
	pf_Frag * curFrag = m_pLast;

	while (diff > 1)
	{
#ifdef DEBUG
		++numIters;
#endif
		curFrag = (pf_Frag *) m_vecFrags.getNthItem(curFragNo);
		if (pos < curFrag->getPos())
		{
			diff = diff / 2;
			curFragNo -= diff;
		}
		else
		{
			diff = diff / 2;
			curFragNo += diff;
		}
	}
	while (curFrag && pos > curFrag->getPos())
	{
#ifdef DEBUG
		++numIters;
#endif
		curFrag = curFrag->getNext();
	}
	while (curFrag && pos < curFrag->getPos())
	{
#ifdef DEBUG
		++numIters;
#endif
		curFrag = curFrag->getPrev();
	}

	xxx_UT_DEBUGMSG(("JCA: Found Frag[%p] at pos %d Looking for pos %d with [%d] iterations\n",
				 curFrag, curFrag->getPos(), pos, numIters));
	if (curFrag && curFrag->getPrev() && curFrag->getNext())
	{
		xxx_UT_DEBUGMSG(("JCA: Frag pos before = %d Frag Pos After %d Looking for pos %d \n",
						 curFrag->getPrev()->getPos(), curFrag->getPos(), curFrag->getNext()->getPos(), pos));
	}
	setCache(curFrag);
	return curFrag;
}

UT_uint32 pf_Fragments::getFragNumber(const pf_Frag * pf) const
{
	if (areFragsDirty())
		cleanFrags();

	return m_vecFrags.findItem((void *) pf);
}

UT_uint32 pf_Fragments::getNumberOfFrags() const
{
	if (areFragsDirty())
	{
		cleanFrags();
	}
	return m_vecFrags.getItemCount();
}







