/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Updates by Ben Martin in 2011.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */


#include "ut_types.h"
#include "pf_Frag.h"
#include "pt_PieceTable.h"
#include "pf_Fragments.h"

pf_Frag::pf_Frag(pt_PieceTable * pPT, PFType type, UT_uint32 length):
	m_type(type),
	m_pField(NULL),
	m_pPieceTable(pPT),
	m_indexAP(0),
	m_length(length),
	m_leftTreeLength(0),
	m_iXID(0),
	m_pMyNode(NULL)
{
}

pf_Frag::~pf_Frag()
{
  xxx_UT_DEBUGMSG(("Delete Frag of Type %d pointer %p \n",getType(),this));
}

/*!
   returns true if both content and fmt of the two frags is identical
*/
bool pf_Frag::operator == (const pf_Frag & f2) const
{
	if(getType() != f2.getType())
		return false;

	// decide if the two frags have same piecetables
	if(!m_pPieceTable || !f2.m_pPieceTable)
		return false;

	if(m_pPieceTable == f2.m_pPieceTable)
	{
		if(m_indexAP != f2.m_indexAP)
			return false;
	}
	else
	{
		// different PT, do it the hard way ...
		const PP_AttrProp * pAP1;
		const PP_AttrProp * pAP2;

		m_pPieceTable->getAttrProp(m_indexAP, &pAP1);
		f2.m_pPieceTable->getAttrProp(f2.m_indexAP, &pAP2);

		UT_return_val_if_fail(pAP1 && pAP2, false);

		if(!pAP1->isEquivalent(pAP2))
		{
			return false;
		}
	}
	
	return _isContentEqual(f2);
}

/*!
    Returns true if the content of the two fragments is identical, but
    ignores formatting properies.
*/ 
bool pf_Frag::isContentEqual(const pf_Frag & f2) const
{
	if(getType() != f2.getType())
		return false;

	// check we have PT to fidle with ...
	if(!m_pPieceTable || !f2.m_pPieceTable)
		return false;
	
	return _isContentEqual(f2);
}

PT_DocPosition pf_Frag::getPos(void) const
{
       UT_return_val_if_fail(m_pMyNode,0);
       pf_Fragments * fragments = &(m_pPieceTable->getFragments());
       const pf_Fragments::Iterator it(fragments,m_pMyNode);
       PT_DocPosition pos = fragments->documentPosition(it);
       return pos;
}

/*!
 * Tell the fragments tree that the length of this item changed by
 * delta.
 * delta = new_length - old_length
 */
void  pf_Frag::lengthChanged(UT_sint32 delta)
{
       UT_return_if_fail(m_pMyNode);
       pf_Fragments * fragments = &(m_pPieceTable->getFragments());
       fragments->changeSize(delta);
       pf_Fragments::Iterator it(fragments,m_pMyNode);
       fragments->fixSize(it);
}

	/* We need the following function to accumulate left tree length */
void  pf_Frag:: accLeftTreeLength(PT_DocPosition length)
{
       m_leftTreeLength += length; 
}
 
void pf_Frag::_setNode(pf_Fragments::Node * pNode)
{
       m_pMyNode = pNode;
}

pf_Fragments::Node * pf_Frag::_getNode(void) const
{
       return m_pMyNode;
}

bool pf_Frag::createSpecialChangeRecord(PX_ChangeRecord ** /*ppcr*/,
										   PT_DocPosition /*dpos*/) const
{
	// really this should be declared abstract in pf_Frag,
	// but we didn't derrive a sub-class for EOD -- it actually
	// uses the base class as is.
	
	// this function must be overloaded for all sub-classes.
	
	UT_ASSERT_HARMLESS(0);
	return true;
}

fd_Field * pf_Frag::getField(void) const
{
    return m_pField;
}

pf_Frag* pf_Frag::getNext(void) const
{
        UT_return_val_if_fail(m_pMyNode,NULL);
 	pf_Fragments::Iterator it(&(m_pPieceTable->getFragments()),m_pMyNode);
	it++;
	return it.value();
}


pf_Frag* pf_Frag::getPrev(void) const
{
        UT_return_val_if_fail(m_pMyNode,NULL);
 	pf_Fragments::Iterator it(&(m_pPieceTable->getFragments()),m_pMyNode);
	it--;
	return it.value();
}

#include "pf_Frag_Strux.h"

static bool isStuxType( const pf_Frag* const pf, PTStruxType t )
{
    if( pf->getType() != pf_Frag::PFT_Strux )
        return false;

    const pf_Frag_Strux* const pfs = static_cast<const pf_Frag_Strux* const>(pf);
    return pfs->getStruxType() == t;
}

/**
 * Get the next strux of the given type from the document. Note that
 * the return value can never be this fragment even if this fragment
 * is also a strux of the sought type.
 * 
 * This way, you can skip from on PTX_Block to the next in a loop using
 * pf_Frag* pf = getFragOfType( PTX_Block );
 * for( ; pf; pf = pf->getNextStrux(PTX_Block) )
 * {
 *   ...
 * }
 */
pf_Frag_Strux* pf_Frag::getNextStrux(PTStruxType t) const
{
    UT_return_val_if_fail(m_pMyNode,NULL);
    pf_Fragments& fragments = m_pPieceTable->getFragments();
 	pf_Fragments::Iterator it(&(fragments),m_pMyNode);
    // If we are the desired type, move ahead already.
    if( isStuxType( this, t ))
    {
        it++;
    }
    for( pf_Fragments::Iterator e = fragments.end(); it != e; ++it )
    {
        pf_Frag* pf = it.value();
        if( !pf )
            return 0;
        if( isStuxType( pf, t ))
        {
            return static_cast<pf_Frag_Strux*>(pf);
        }
    }
    return 0;
}

pf_Frag_Strux*
pf_Frag::tryDownCastStrux(PTStruxType t) const
{
    if( getType() == pf_Frag::PFT_Strux )
    {
        pf_Fragments& fragments = m_pPieceTable->getFragments();
        pf_Fragments::Iterator it(&(fragments),m_pMyNode);
        pf_Frag* pf = it.value();
        pf_Frag_Strux* pfs = static_cast<pf_Frag_Strux*>(pf);
        PTStruxType eStruxType = pfs->getStruxType();
        if( eStruxType == t )
            return pfs;
    }
    return 0;
}

pf_Frag_Strux* tryDownCastStrux( pf_Frag* pf, PTStruxType t)
{
    if( !pf )
        return 0;
    return pf->tryDownCastStrux( t );
}



#include "pf_Frag_Object.h"
#include "pf_Frag_Strux.h"
#include "pd_DocumentRDF.h"

std::string
pf_Frag::getXMLID() const
{
    std::string ret = "";

    const PP_AttrProp* pAP = NULL;
    m_pPieceTable->getAttrProp( getIndexAP() ,&pAP );
    if( !pAP )
        return ret;
    const char* v = 0;
    
    if(getType() == pf_Frag::PFT_Object)
    {
        const pf_Frag_Object* pOb = static_cast<const pf_Frag_Object*>(this);

        if(pOb->getObjectType() == PTO_Bookmark)
        {
            if( pAP->getAttribute(PT_XMLID, v) && v)
            {
                ret = v;
            }
        }
        if(pOb->getObjectType() == PTO_RDFAnchor)
        {
            RDFAnchor a(pAP);
            ret = a.getID();
        }
    }
    if(getType() == pf_Frag::PFT_Strux)
    {
        const pf_Frag_Strux* pfs = static_cast<const pf_Frag_Strux*>(this);
        PTStruxType st = pfs->getStruxType();
        if( st == PTX_Block || st == PTX_SectionCell )
        {
            if(pAP->getAttribute("xml:id", v))
            {
                ret = v;
            }
        }
    }

    return ret;
}

