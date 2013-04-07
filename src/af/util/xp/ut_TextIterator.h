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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef UT_ITERATOR_H
#define UT_ITERATOR_H

//////////////////////////////////////////////////////////////////////////////
//
//  UT_TextIterator class is an abstraction of a text iterator, making
//  it possible to iterate sequentially over textual data without
//  having to know anything about how that data might be stored.
//
//  This class is pure virtual, its sole purpose is to define generic
//  interface so that we can pass a generic type into and out of
//  functions. For example of implementation see pd_Iterator.h/cpp
//
//  Notes on imlementation
//  ----------------------
//  Any derrived classes should implement the individual functions to
//  conform to the behaviour outlined in the comments in the class
//  definion below.
//
//  In addtion, the actual iterator implementations should provide a
//  mechanism allowing to restrict upper and lower bounds (either at
//  construction or subsequently), so that when passing iterators into
//  functions it is not necessary to pass with them a length
//  parameter. For example, PD_StruxIterator can provide access to the
//  entire document from the start of the strux onwards; we might want
//  to restrict this to the part that only belongs to a particular
//  TextRun, etc.
//
//  Notes on use
//  ------------
//  When passing iterators into functions, the iterator should be set
//  at the position where processing is to start, i.e., the user is
//  not expected to reposition the iterator before commencing
//  processing. Also, the upper boundary should be restricted
//  appropriately to indicate where the processing is to stop; this is
//  preferable to passing an extra length parameter.
//
//  Tomas, November, 2003
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
//     Error: any other error; this state is irrecoverable, clean up
//            and go home
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
	virtual ~UT_TextIterator() {}

	/////////////////////////////////////////////////////////////////////////
	// data accessor; retrieves character at present position
	//
	// NB: I.getChar() is functionally equivalent to I[getPosition()]
	//
	virtual UT_UCS4Char getChar() = 0;

	/////////////////////////////////////////////////////////////////////////
	// positon accessor; returns a value representing current postion
	//
	// NB: The position can be expressed in an arbitrary coordinate
	// system, typically one that makes sense to the actual
	// implementation; when an iterator is passed into a function, the
	// starting position might not be 0.
	//
	virtual UT_uint32   getPosition() const = 0;

	////////////////////////////////////////////////////////////////////
	// moves iterator to position pos
	//
	virtual void setPosition(UT_uint32 pos) = 0;

	///////////////////////////////////////////////////////////////////
	// set and retrieve upper bounds
	//
	virtual void      setUpperLimit(UT_uint32 maxpos) = 0;
	virtual UT_uint32 getUpperLimit() const = 0;

	///////////////////////////////////////////////////////////////////
	// returns the current state of the iterator (see definition of
	// UTIterStatus above)
	//
	virtual UTIterStatus getStatus() const = 0;

	///////////////////////////////////////////////////////////////////
	// finds first occurence of given string, looking in direction
	// indicated by bForward
	// failure is indicated through getStatus() == UTIter_OutOfBounds;
	//
	virtual UT_uint32 find(UT_UCS4Char * what, UT_uint32 iLen, bool bForward = true) = 0;
	virtual UT_uint32 find(UT_TextIterator & text, UT_uint32 iLen, bool bForward = true) = 0;

	///////////////////////////////////////////////////////////////////
	// makes a copy of the iterator in its present state
	//
	virtual UT_TextIterator * makeCopy() const = 0;

	///////////////////////////////////////////////////////////////////
	// increment operators
	//
	// NB: We intentionally define prefix operators only, as post-fix
	// versions provide no real advantage, and are less efficient
	//
	virtual UT_TextIterator & operator ++ () = 0;
	virtual UT_TextIterator & operator -- () = 0;
	virtual UT_TextIterator & operator += (UT_sint32 i) = 0;
	virtual UT_TextIterator & operator -= (UT_sint32 i) = 0;

	////////////////////////////////////////////////////////////////////
	// subscript operator []; repostions iterator and returns
	// character at new postion
	//
	// NB(1): the operator physically advances the iterator to positon
	// pos before returning, i.e.,
	//
	//     UT_UCS4Char c = I[p];
	//
	// and
	//
	//     I.setPosition(p);
	//     UT_UCS4Char c = I.getChar();
	//
	// are exactly equivalent, leaving the iterator in the same state
	//
	// NB(2): if passed iterator as an argumenent in a function, you
	// need to know the initial position to use this operator for
	// processing which is relative to the state of iterator when
	// passed to you, i.e., f1() and f2() below do exactly the same
	// thing, f3() does not.
	//
	// function f1(UT_TextIterator & I, UT_uint32 len)
	// {
	//    UT_uint32 pos = I.getPosition();
	//
	//    for(UT_uint32 i = pos; i < len + pos; i++)
	//    {
	//       UT_UCS4Char c = text[i];
	//       // do something with c ...
	//    }
	// }
	//
	// function f2(UT_TextIterator & I, UT_uint32 len)
	// {
	//    for(UT_uint32 i = 0; i < len; ++i, ++I)
	//    {
	//       UT_UCS4Char c = text.getChar();
	//       // do something with c ...
	//    }
	// }
	//
	// In contrast, f3() will start at the leftmost edge of the
	// theoretical iterator range, which is probably not what you
	// want; the actual implementation of the iterator can if fact
	// restrict valid range of the subscript to an arbitrary range
	// (i.e., I[0] may produce OutOfBounds state).
	//
	// function f3(UT_TextIterator & I, UT_uint32 len)
	// {
	//    for(UT_uint32 i = 0; i < len; i++)
	//    {
	//       UT_UCS4Char c = text[i];
	//       // do something with c ...
	//    }
	// }
	//
	// Bottom Line: unless told otherwise, assume that
	// processing is to start from I.getPosition(), not 0.
	//
	virtual UT_UCS4Char   operator [](UT_uint32 pos) = 0;

};


#endif //UT_ITERATOR_H
