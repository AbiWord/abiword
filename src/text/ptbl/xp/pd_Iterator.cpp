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

#include "pd_Iterator.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "pd_Document.h"

////////////////////////////////////////////////////////////////////
//
//  PD_Iterator
//
//

/*! the basic constructor, which will start looking for the position
    from the beginning of the PT
    \param doc - the document which we want to iterate
    \param dpos - document position we want to start from
*/
PD_Iterator::PD_Iterator(PD_Document &doc, PT_DocPosition dpos)
	: m_pt(*doc.getPieceTable()), m_pos(dpos), m_frag(NULL)
{
	// find the frag at requested postion
	_findFragAtPosition();
}

/*! constructor which allows us to jump directly to the correct strux
    in the PT if we have its handle
    \param doc - the document which we want to iterate
    \param sdh - handle of the strux we want to start from
    \param offset - offset relative to strux we want to start from
*/
PD_Iterator::PD_Iterator(PD_Document &doc, PL_StruxDocHandle sdh, UT_uint32 offset)
	: m_pt(*doc.getPieceTable()), m_pos(0), m_frag(NULL)
{
	setPosition(sdh, offset);
}

/*! set position to given strux and strux offset
    \param sdh - handle of the strux we want to start from
    \param offset - offset relative to strux we want to start from
*/
bool PD_Iterator::setPosition(PL_StruxDocHandle sdh, UT_uint32 offset)
{
	m_frag = static_cast<const pf_Frag *>(sdh);

	if(!m_frag)
		return false;

	m_pos = m_frag->getPos() + offset;

	// now we can find the frag at the position
	return _findFragAtPosition();
}

/*! find the PT fragment that contains current postion (m_pos)
 */
bool PD_Iterator::_findFragAtPosition()
{
	if(m_frag)
	{
		// if we have a fragment, we can speed things up in certain
		// cases
		if(m_frag->getPos() <= m_pos && m_frag->getPos() + m_frag->getLength() > m_pos)
		{
			// we have the correct fragment already
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
					return true;
				}

				m_frag = m_frag->getNext();
			}

			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

			// set the pos to end of doc
			m_frag = m_pt.getFragments().getLast();
			UT_return_val_if_fail(m_frag, false);
			
			m_pos = m_frag->getPos() + m_frag->getLength() - 1;
			
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
			return true;
		}

		// set the current pos to the end of doc
		m_pos = m_frag->getPos() + m_frag->getLength() - 1;
	}
	
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}

/*! get character at the curent position
 */
UT_UCS4Char PD_Iterator::getChar() const
{
	UT_return_val_if_fail(m_frag, PD_IT_ERROR);

	if(m_frag->getType() == pf_Frag::PFT_Text)
	{
		const pf_Frag_Text * pft = static_cast<const pf_Frag_Text*>(m_frag);

		const UT_UCS4Char * p = m_pt.getPointer(pft->getBufIndex());

		UT_return_val_if_fail(p, PD_IT_ERROR);
		UT_return_val_if_fail(m_pos - pft->getPos() < pft->getLength(), PD_IT_ERROR);

		return p[m_pos - pft->getPos()];
	}

	// we are in a non-text fragment
	return PD_IT_NOT_CHARACTER;
}

/*! various increment operators
 */
PD_Iterator & PD_Iterator::operator ++ ()
{
	m_pos++;
	_findFragAtPosition();

	return *this;
}

PD_Iterator & PD_Iterator::operator ++ (UT_sint32 i) // post-fix
{
	m_pos++;
	_findFragAtPosition();

	return *this;
}

	
PD_Iterator & PD_Iterator::operator -- ()
{
	if(m_pos > 0)
	{
		m_pos--;
		_findFragAtPosition();
	}

	return *this;
}
	
PD_Iterator & PD_Iterator::operator -- (UT_sint32 i) // post-fix
{
	if(m_pos > 0)
	{
		m_pos--;
		_findFragAtPosition();
	}

	return *this;
}

PD_Iterator & PD_Iterator::operator +=  (UT_sint32 i)
{
	m_pos += i;
	_findFragAtPosition();

	return *this;
}
	
PD_Iterator & PD_Iterator::operator -=  (UT_sint32 i)
{
	if((UT_sint32)m_pos >= i)
	{
		m_pos -= i;
		_findFragAtPosition();
	}

	return *this;
}

/*! advance iterator to document position dpos  and return character
    at that position
*/
UT_UCS4Char PD_Iterator::operator [](PT_DocPosition dpos)
{
	m_pos = dpos;
	_findFragAtPosition();

	return getChar();
}


