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

#ifndef UT_ITERATOR_H
#define UT_ITERATOR_H

//////////////////////////////////////////////////////////////////////////////
//
//  The following class is an abstraction of a Text Iterator, which
//  makes it possible to iterate sequentially over textual data
//  without having to know anything about how that data might be stored
//
//  this class is pure virtual, its sole purpose is to define generic interface
//  so that we can pass a generic type into and out of functions
//  Tomas, Nov 10, 2003
//

//////////////////////////////////////////////////////////////////////
// the follwoing are values that the getChar() function can fall back
// on when things are not entirely right ...
//
//    UT_IT_NOT_CHARACTER: when at the current position we have
//                         something else than text (image, etc)
//
//    UT_IT_ERROR: when things are really not going as they should
//                 NB: this is just to have something to fall back on,
//                 not an error reporting mechanism; for that see
//                 getStatus() below
#define UT_IT_NOT_CHARACTER UCS_SPACE
#define UT_IT_ERROR 0xffffffff

#include "ut_types.h"

class PD_Document;
class pt_PieceTable;
class pf_Frag;

/////////////////////////////////////////////////////////////
//
// The following enum defines possible iterator states:
//
//     OK: need I say more?
//     
//     OutOfBounds: last positioning operation took the iterator
//                  out of bounds; this error state is recoverable
//                  by using the indexing operator [], or calling
//                  setPosition() but the use of relative increment
//                  operators (++, --, +=, -=) in this state will
//                  lead to undefined results.
//
//     Error: any other error; this state is irrecoverable
//                  
//
enum UTIterStatus
{
	UTIter_OK,
	UTIter_OutOfBounds,
	UTIter_Error
};


class ABI_EXPORT UT_TextIterator
{
  public:

	/////////////////////////////////////////
	// data accessor
	//
	virtual UT_UCS4Char getChar() = 0;

	/////////////////////////////////////////////////////////////////////////
	// positon accessor; returns a value such that the following holds true:
	// 
	//     UT_TextIterator I(...);
	//     UT_uint32 p = I.getPos();
	//     UT_UCS4Char c = I[p]; // se below notes on operator []
	//     I.getPosition() == p;
	//
	virtual UT_uint32   getPosition() const = 0;

	////////////////////////////////////////////////////////////////////
	// moves iterator to position pos
	//
	virtual void setPosition(UT_uint32 pos) = 0;

	///////////////////////////////////////////////////////////////////
	// return current state of the iterator (see definition of
	// UTIterStatus above
	//
	virtual UTIterStatus getStatus() const = 0;

	///////////////////////////////////////////////////////////////////
	// make a copy of the iterator in its present state
	//
	virtual UT_TextIterator * makeCopy() = 0;

	///////////////////////////////////////////////////////////////////
	// increment operators -- we intentionally define prefix operators
	// only, as post-fix versions provide no real advantage, and are
	// less efficient
	virtual UT_TextIterator & operator ++ () = 0;
	virtual UT_TextIterator & operator -- () = 0;
	virtual UT_TextIterator & operator += (UT_sint32 i) = 0;
	virtual UT_TextIterator & operator -= (UT_sint32 i) = 0;
	
	////////////////////////////////////////////////////////////////
	// subscript operator;
	//
	// NB: the operator physically advances the iterator to positon
	// pos before returning, i.e.,
	//
	//     UT_UCS4Char c = I[p];
	//
	// and
	//
	//     I.setPosition(p);
	//     UT_UCS4Char c = I.getChar();
	//
	// are exactly equivalent
	
	virtual UT_UCS4Char   operator [](UT_uint32 pos) = 0;

};


#endif //UT_ITERATOR_H
