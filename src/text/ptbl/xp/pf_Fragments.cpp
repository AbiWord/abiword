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

pf_Fragments::pf_Fragments()
	: m_pFirst(NULL),
	m_pLast(NULL),
	m_bFragsClean(false)
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
	
	UT_ASSERT(pf);
	setFragsDirty();
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
	setFragsDirty();
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
	// NOTE:  it is the caller's responsibility to delete pf if appropriate.
	
	UT_ASSERT(pf->getType() != pf_Frag::PFT_EndOfDoc);
	setFragsDirty();
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

/*!
 * This method clears out and repopulates the vectore of pointers to fragments.
 * It also sets the doc Positions of all the fragments.
 */
void pf_Fragments::cleanFrags(void)
{
	if(m_vecFrags.getItemCount() > 0)
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
	UT_ASSERT(pfLast && (pfLast->getType() == pf_Frag::PFT_EndOfDoc));
	xxx_UT_DEBUGMSG(("SEVIOR: Found %d Frags dopos at end = %d \n",m_vecFrags.getItemCount(),getLast()->getPos()));
	m_bFragsClean = true;
}

static void pf_fragments_clean_frags( void * p)
{
	pf_Fragments * pFragments = static_cast<pf_Fragments *>(p);
	pFragments->cleanFrags();
}

/*!
 * Stupid wrapper for const functions.
 */
void pf_Fragments::cleanFragsConst(void) const
{
	pf_fragments_clean_frags( (void *) this);
}

pf_Frag * pf_Fragments::getNthFrag(UT_uint32 nthFrag) const
{
	if(areFragsDirty())
	{
		cleanFragsConst();
	}
	if(m_vecFrags.getItemCount() > 0)
	{
		return (pf_Frag *) m_vecFrags.getNthItem(nthFrag);
	}
	return NULL;
}

/*!
 * Binary search to find the first frag at position before pos
\param PT_DocPosition we want to find for.
\returns pf_Frag * pointer to the Frag with position immediately before pos
*/
pf_Frag * pf_Fragments::findFirstFragBeforePos( PT_DocPosition pos) const
{
	UT_uint32 numFrags = getNumberOfFrags();
	if(numFrags  < 1)
		return NULL;
	if(pos >= getLast()->getPos())
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Found last Frag= pos %d Looking for pos %d \n",getLast()->getPos(),pos));
		return getLast();
	}
	UT_sint32 diff = numFrags/2;
	UT_sint32 curFragNo = diff;
	pf_Frag * curFrag = m_pLast;

	while(diff > 1)
	{
		curFrag = (pf_Frag *) m_vecFrags.getNthItem(curFragNo);
		if(pos < curFrag->getPos())
		{
			diff = diff/2;
			curFragNo -= diff;
		}
		else
		{
			diff = diff/2;
			curFragNo += diff;
		}
	}
	while( curFrag && pos > curFrag->getPos())
	{
		curFrag = curFrag->getNext();
	}
	while( curFrag && pos < curFrag->getPos())
	{
		curFrag = curFrag->getPrev();
	}
	xxx_UT_DEBUGMSG(("SEVIOR: Found at pos %d Looking for pos %d \n",curFrag->getPos(),pos));
	if(curFrag && curFrag->getPrev() && curFrag->getNext())
		xxx_UT_DEBUGMSG(("SEVIOR Frag pos before = %d Frag Pos After %d Looking for pos %d \n",curFrag->getPrev()->getPos(),curFrag->getPos(),curFrag->getNext()->getPos(),pos));
	return curFrag;
}

UT_uint32 pf_Fragments::getFragNumber( const pf_Frag * pf) const
{
	if(areFragsDirty())
	{
		cleanFragsConst();
	}
	return m_vecFrags.findItem( (void *) pf);
}


UT_uint32 pf_Fragments::getNumberOfFrags(void) const
{
	if(areFragsDirty())
	{
		cleanFragsConst();
	}
	return m_vecFrags.getItemCount();
}







