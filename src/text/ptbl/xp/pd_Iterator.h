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
//  The following classes allow us to iterate sequentially and
//  efficiently over the content of a document. 
//
//  Tomas, Nov 9, 2003
//

#include "ut_TextIterator.h"
#include "pt_Types.h"

class PD_Document;
class pt_PieceTable;
class pf_Frag;

/******************************************************************
 *
 * PD_DocIterator iterates over the contents of the whole document
 *
 * NB: because iterating over a document using document position
 * requires that the PT fragments are clean, this iterator will clean
 * the fragments whenever they are not clean. This has certain
 * performance implications for long docs: it will be preferable to
 * use the PD_StruxIterator below when accessing a limited portion of
 * the document; PD_DocIterator is more suitable when access is
 * required to large part of document and access is not strictly
 * sequential
 * 
 */
class ABI_EXPORT PD_DocIterator : UT_TextIterator
{
  public:
	PD_DocIterator(PD_Document & doc, PT_DocPosition dpos = 0);

	virtual UT_UCS4Char getChar() const; // return character at present position

	virtual UT_TextIterator & operator ++ ();
	virtual UT_TextIterator & operator -- ();
	virtual UT_TextIterator & operator +=  (UT_sint32 i);
	virtual UT_TextIterator & operator -=  (UT_sint32 i);
	
	virtual UT_UCS4Char   operator [](UT_uint32 dpos);

  private:
	bool _findFrag();

	pt_PieceTable & m_pt;
	PT_DocPosition  m_pos;
	
	const pf_Frag * m_frag;

};

/******************************************************************
 *
 * PD_StruxIterator iterates over the contents of document from a
 * given strux onwards; in contrast to PD_DocIterator above, it does
 * not rely on PT fragments being clean, but calculates offest from
 * the starting strux from lengths of individual fragments
 *
 */

class ABI_EXPORT PD_StruxIterator : UT_TextIterator
{
  public:
	PD_StruxIterator(PD_Document & doc, PL_StruxDocHandle sdh, UT_uint32 offset = 0);

	virtual UT_UCS4Char getChar() const; // return character at present position

	virtual UT_TextIterator & operator ++ ();
	virtual UT_TextIterator & operator -- ();
	virtual UT_TextIterator & operator +=  (UT_sint32 i);
	virtual UT_TextIterator & operator -=  (UT_sint32 i);
	
	virtual UT_UCS4Char   operator [](UT_uint32 dpos);

  private:
	bool _findFrag();
	
	pt_PieceTable &   m_pt;
	UT_uint32         m_offset;
	UT_uint32         m_frag_offset;
	PL_StruxDocHandle m_sdh;
	
	const pf_Frag * m_frag;

};

#endif //PD_ITERATOR_H
