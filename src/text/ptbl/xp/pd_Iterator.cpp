/* AbiWord
 * Copyright (C) 2003 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#include "ut_debugmsg.h"
#include "pd_Iterator.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "pd_Document.h"

////////////////////////////////////////////////////////////////////
//
//  PD_DocIterator -- iterates over the whole PieceTable
//  
//  NB: because iterating over a document using document position
//      requires that the PT fragments are clean, this iterator will
//      clean the fragments whenever they are not clean. This has
//      certain performance implications (it might be preferable
//      to use the PD_StruxIterator when accessing a limited portion
//      of the document)
//
//

/*!     
    \param doc - the document which we want to iterate
    \param dpos - document position we want to start from
*/
PD_DocIterator::PD_DocIterator(const PD_Document &doc, PT_DocPosition dpos)
	: m_pt(*doc.getPieceTable()), m_pos(dpos), m_frag(NULL), m_status(UTIter_OK)
{
	// find the frag at requested postion
	_findFrag();
}

UT_TextIterator * PD_DocIterator::makeCopy()
{
	PD_DocIterator * t = new PD_DocIterator(m_pt);
	UT_return_val_if_fail(t, NULL);
	
	t->m_pos = m_pos;
	t->m_frag = m_frag;
	t->m_status = m_status;

	return t;
}


/*! find the PT fragment that contains current postion (m_pos)
 */
bool PD_DocIterator::_findFrag()
{
	// need to make sure fragments are clean before we can use their
	// doc positions
	if(m_pt.getFragments().areFragsDirty())
	{
		m_pt.getFragments().cleanFrags();
	}

	if(m_frag)
	{
		// if we have a fragment, we can speed things up in certain
		// cases
		if(m_frag->getPos() <= m_pos && m_frag->getPos() + m_frag->getLength() > m_pos)
		{
			// we have the correct fragment already
			m_status = UTIter_OK;
			return true;
		}

		if(m_frag->getPos() < m_pos)
		{
			// keep going from here onwards
			m_frag = m_frag->getNext();
			
			while(m_frag)
			{
				if(m_frag->getPos() <= m_pos && m_frag->getPos() + m_frag->getLength() > m_pos)
				{
					// we have the correct fragment
					m_status = UTIter_OK;
					return true;
				}

				m_frag = m_frag->getNext();
			}

			m_status = UTIter_OutOfBounds;
			
			return false;
		}

		if(m_frag->getPos() > m_pos)
		{
			// keep going from here back
			m_frag = m_frag->getPrev();
			
			while(m_frag)
			{
				if(m_frag->getPos() <= m_pos && m_frag->getPos() + m_frag->getLength() > m_pos)
				{
					// we have the correct fragment
					m_status = UTIter_OK;
					return true;
				}

				m_frag = m_frag->getPrev();
			}

			m_status = UTIter_OutOfBounds;
			
			return false;
		}
		
		
	}

	// do it the hard way
	m_frag = m_pt.getFragments().findFirstFragBeforePos(m_pos);

	if(m_frag)
	{
		// check that the position is not after the end of the
		// document
		if(m_frag->getPos() <= m_pos && m_frag->getPos() + m_frag->getLength() > m_pos)
		{
			// we have the correct fragment
			m_status = UTIter_OK;
			return true;
		}
	}
	
	// this happens, for instance, when we try to move past end of doc
	// (this can happen legitimately for example in the last increment
	// of a for loop)
	m_status = UTIter_OutOfBounds;
	return false;
}

/*! get character at the curent position
 */
UT_UCS4Char PD_DocIterator::getChar()
{
	UT_return_val_if_fail(m_frag && m_status == UTIter_OK, UT_IT_ERROR);

	if(m_frag->getType() == pf_Frag::PFT_Text)
	{
		const pf_Frag_Text * pft = static_cast<const pf_Frag_Text*>(m_frag);

		const UT_UCS4Char * p = m_pt.getPointer(pft->getBufIndex());

		if(!p)
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			m_status = UTIter_Error;
			return UT_IT_ERROR;
		}
		
		if(!(m_pos - pft->getPos() < pft->getLength()))
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			m_status = UTIter_Error;
			return UT_IT_ERROR;
		}
		
		return p[m_pos - pft->getPos()];
	}

	// we are in a non-text fragment
	return UT_IT_NOT_CHARACTER;
}

/*! various increment operators
 */
UT_TextIterator & PD_DocIterator::operator ++ ()
{
	if(m_status == UTIter_OK)
	{
		m_pos++;
		_findFrag();
	}
	
	return *this;
}

	
UT_TextIterator & PD_DocIterator::operator -- ()
{
	if(m_status == UTIter_OK)
	{
		if(m_pos > 0)
		{
			m_pos--;
			_findFrag();
		}
		else
		{
			m_status = UTIter_OutOfBounds;
		}
	}
	
	return *this;
}
	
UT_TextIterator & PD_DocIterator::operator +=  (UT_sint32 i)
{
	if(m_status == UTIter_OK)
	{
		if(i >= -(UT_sint32)m_pos)
		{
			m_pos += i;
			_findFrag();
		}
		else
		{
			m_status = UTIter_OutOfBounds;
		}
	}
	
	return *this;
}
	
UT_TextIterator & PD_DocIterator::operator -=  (UT_sint32 i)
{
	if(m_status == UTIter_OK)
	{
		if((UT_sint32)m_pos >= i)
		{
			m_pos -= i;
			_findFrag();
		}
		else
		{
			m_status = UTIter_OutOfBounds;
		}
	}
	
	return *this;
}

/*! advance iterator to document position dpos  and return character
    at that position
*/
UT_UCS4Char PD_DocIterator::operator [](UT_uint32 dpos)
{
	m_pos = (PT_DocPosition)dpos;
	_findFrag();

	return getChar();
}

void PD_DocIterator::setPosition(UT_uint32 dpos)
{
	m_pos = (PT_DocPosition)dpos;
	_findFrag();
}

///////////////////////////////////////////////////////////////////////
//
// PD_StruxIterator - iterate over Piece Table from the start of given
// strux onwards

/*! 
    \param doc - the document which we want to iterate
    \param sdh - handle of the strux we want to start from
    \param offset - offset relative to strux we want to start from
    \param maxOffset - the upper limit of offset
*/
PD_StruxIterator::PD_StruxIterator(PL_StruxDocHandle sdh,UT_uint32 offset,
								   UT_uint32 maxOffset)
	: m_pPT(NULL), m_offset(offset), m_sdh(sdh),
	  m_frag_offset(0), m_frag(NULL),
	  m_status(UTIter_OK),
	  m_max_offset(maxOffset),
	  m_strux_len(0)
{
	UT_return_if_fail(m_sdh);
	m_frag = static_cast<const pf_Frag *>(m_sdh);
	m_pPT = const_cast<pf_Frag *>(m_frag)->getPieceTable();

	// save the length of this strux, so we can test for out of bounds
	// condition on the low end
	m_strux_len = m_frag->getLength();
		
	_findFrag();
	xxx_UT_DEBUGMSG(("sizeof PD_StruxIterator: %d\n", sizeof(PD_StruxIterator)));
}

UT_TextIterator * PD_StruxIterator::makeCopy()
{
	PD_StruxIterator * t = new PD_StruxIterator();
	UT_return_val_if_fail(t, NULL);
	
	t->m_pPT = m_pPT;
	t->m_offset = m_offset;
	t->m_frag = m_frag;
	t->m_status = m_status;
	t->m_frag_offset = m_frag_offset;
	t->m_max_offset = m_max_offset;
	t->m_sdh = m_sdh;
	t->m_strux_len = m_strux_len;

	return t;
}

// strux relative iteration
bool PD_StruxIterator::_findFrag()
{
	if(!m_frag)
	{
		m_frag = static_cast<const pf_Frag *>(m_sdh);
		m_frag_offset = 0;
	}
	
	while(m_frag)
	{
		if(m_frag_offset <= m_offset && m_frag_offset + m_frag->getLength() > m_offset)
		{
			m_status = UTIter_OK;
			return true;
		}

		if(m_offset < m_frag_offset)
		{
			m_frag = m_frag->getPrev();
			m_frag_offset -= m_frag->getLength();
		}
		else if(m_offset >= m_frag_offset + m_frag->getLength())
		{
			m_frag_offset += m_frag->getLength();
			m_frag = m_frag->getNext();
		}
		
	}

	// this happens, for instance, when we try to move past end of doc
	// (this can happen legitimately for example in the last increment
	// of a for loop)
	m_status = UTIter_OutOfBounds;
	return false;
}


/*! get character at the curent position
 */
UT_UCS4Char PD_StruxIterator::getChar()
{
	if(!m_frag || m_status != UTIter_OK)
		return UT_IT_ERROR;

	if(m_frag->getType() == pf_Frag::PFT_Text)
	{
		const pf_Frag_Text * pft = static_cast<const pf_Frag_Text*>(m_frag);

		const UT_UCS4Char * p = m_pPT->getPointer(pft->getBufIndex());

		if(!p)
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			m_status = UTIter_Error;
			return UT_IT_ERROR;
		}
		
		if(!(m_offset - m_frag_offset < pft->getLength()))
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			m_status = UTIter_Error;
			return UT_IT_ERROR;
		}

		return p[m_offset - m_frag_offset];
	}

	// we are in a non-text fragment
	return UT_IT_NOT_CHARACTER;
}

bool PD_StruxIterator::_incrementPos(UT_sint32 d)
{
	if(m_status == UTIter_OK)
	{
		// data starts at pos m_strux_len
		if(   ((UT_sint32)m_offset + d) >= (UT_sint32)m_strux_len
		   && (UT_uint32)((UT_sint32)m_offset + d) <= m_max_offset)
		{
			m_offset += d;
			return true;
		}

		m_status = UTIter_OutOfBounds;
		return false;
	}

	return false;
}

/*! various increment operators
 */
UT_TextIterator & PD_StruxIterator::operator ++ ()
{
	if(_incrementPos(1))
	{
		_findFrag();
	}
	
	return *this;
}

UT_TextIterator & PD_StruxIterator::operator -- ()
{
	if(_incrementPos(-1))
	{
		_findFrag();
	}
	
	return *this;
}
	
UT_TextIterator & PD_StruxIterator::operator +=  (UT_sint32 i)
{
	if(_incrementPos(i))
	{
		_findFrag();
	}
	
	return *this;
}
	
UT_TextIterator & PD_StruxIterator::operator -=  (UT_sint32 i)
{
	if(_incrementPos(-i))
	{
		_findFrag();
	}
	
	return *this;
}

/*! advance iterator to document position dpos  and return character
    at that position
*/
UT_UCS4Char PD_StruxIterator::operator [](UT_uint32 dpos)
{
	setPosition(dpos);
	return getChar();
}

void PD_StruxIterator::setPosition(UT_uint32 pos)
{
	if(pos >= m_strux_len && pos <= m_max_offset)
	{
		m_offset = pos;
		_findFrag();
	}
	else
	{
		m_status = UTIter_OutOfBounds;
	}
}


