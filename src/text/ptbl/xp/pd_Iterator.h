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

#ifndef PD_ITERATOR_H
#define PD_ITERATOR_H

//////////////////////////////////////////////////////////////////////////////
//
//  The following class allows us to iterate sequentially and
//  efficiently over the content of a document. 
//
//  For now I am really just interested in iterating over the text
//  segements, so the only accessor implemented is getChar(), which
//  will return the value PD_IT_NOT_CHARACTER if the iterrator points
//  at a non-text fragment
//
//  Tomas, Nov 9, 2003
//

#define PD_IT_NOT_CHARACTER 0x20
#define PD_IT_ERROR 0x20

#include "ut_types.h"
#include "pt_Types.h"

class PD_Document;
class pt_PieceTable;
class pf_Frag;


class ABI_EXPORT PD_Iterator
{
  public:
	PD_Iterator(PD_Document & doc, PT_DocPosition dpos = 0);
	PD_Iterator(PD_Document & doc, PL_StruxDocHandle sdh, UT_uint32 offset=0);

	bool setPosition(PL_StruxDocHandle sdh, UT_uint32 offset=0);
	
	UT_UCS4Char getChar() const; // return character at present position

	// advance position by 1
	PD_Iterator & operator ++ ();
	PD_Iterator & operator ++ (UT_sint32 i); // post-fix
	
	// rewind position by 1
	PD_Iterator & operator -- ();
	PD_Iterator & operator -- (UT_sint32 i); // post-fix
	
	// advance/rewind position by i
	PD_Iterator & operator +=  (UT_sint32 i);
	PD_Iterator & operator -=  (UT_sint32 i);
	
	// get character at dpos
	UT_UCS4Char   operator [](PT_DocPosition dpos);

  private:
	bool _findFragAtPosition();
	
	pt_PieceTable & m_pt;
	PT_DocPosition  m_pos;

	const pf_Frag * m_frag;

};


#endif //PD_ITERATOR_H
