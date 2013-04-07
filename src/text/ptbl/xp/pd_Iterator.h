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
class pf_Frag_Strux;

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
class ABI_EXPORT PD_DocIterator : public UT_TextIterator
{
  public:
	PD_DocIterator(const PD_Document & doc, PT_DocPosition dpos = 0);

	virtual UT_UCS4Char  getChar(); // return character at present position

	virtual UT_uint32 getPosition() const {return m_pos;}
	virtual void setPosition(UT_uint32 pos);

	virtual void      setUpperLimit(UT_uint32 maxpos) {m_max_pos = maxpos;}
	virtual UT_uint32 getUpperLimit() const { return m_max_pos;}

	virtual UTIterStatus getStatus() const {return m_status;}

	virtual UT_uint32 find(UT_UCS4Char * what, UT_uint32 iLen, bool bForward = true);
	virtual UT_uint32 find(UT_TextIterator & text, UT_uint32 iLen, bool bForward = true);

	virtual UT_TextIterator * makeCopy() const;

	virtual UT_TextIterator & operator ++ ();
	virtual UT_TextIterator & operator -- ();
	virtual UT_TextIterator & operator +=  (UT_sint32 i);
	virtual UT_TextIterator & operator -=  (UT_sint32 i);

	virtual UT_UCS4Char   operator [](UT_uint32 dpos);

	const pf_Frag * getFrag() const {return m_frag;}

	void            reset() {m_pos = 0; m_frag = NULL; m_status = UTIter_OK;}
	void            reset(PT_DocPosition pos, const pf_Frag * pf) // use with great care
		             {m_pos = pos; m_frag = pf; _findFrag();}


  private:
	// private constructor
	PD_DocIterator(pt_PieceTable & pt):m_pt(pt){};

	bool _findFrag();

	pt_PieceTable & m_pt;
	PT_DocPosition  m_pos;
	PT_DocPosition  m_max_pos;

	const pf_Frag * m_frag;

	UTIterStatus    m_status;
};

/******************************************************************
 *
 * PD_StruxIterator iterates over the contents of document from a
 * given strux onwards; in contrast to PD_DocIterator above, it does
 * not rely on PT fragments being clean, but calculates offest from
 * the starting strux from lengths of individual fragments
 *
 */

class ABI_EXPORT PD_StruxIterator : public UT_TextIterator
{
  public:
	PD_StruxIterator(pf_Frag_Strux* sdh,
					 UT_uint32 offset = 0, UT_uint32 maxoffset = 0xffffffff);

	virtual UT_UCS4Char getChar(); // return character at present position

	virtual UT_uint32 getPosition() const {return m_offset;}
	virtual void setPosition(UT_uint32 pos);

	virtual void      setUpperLimit(UT_uint32 maxpos) {m_max_offset = maxpos;}
	virtual UT_uint32 getUpperLimit() const {return m_max_offset;}

	virtual UTIterStatus getStatus() const {return m_status;}

	virtual UT_uint32 find(UT_UCS4Char * what, UT_uint32 iLen, bool bForward = true);
	virtual UT_uint32 find(UT_TextIterator & text, UT_uint32 iLen, bool bForward = true);

	virtual UT_TextIterator * makeCopy() const;

	virtual UT_TextIterator & operator ++ ();
	virtual UT_TextIterator & operator -- ();
	virtual UT_TextIterator & operator +=  (UT_sint32 i);
	virtual UT_TextIterator & operator -=  (UT_sint32 i);

	virtual UT_UCS4Char   operator [](UT_uint32 dpos);

  private:
	// private default constructor
	PD_StruxIterator(){};

	bool _findFrag();
	bool _incrementPos(UT_sint32 d);

	pt_PieceTable *   m_pPT;
	UT_uint32         m_offset;
	UT_uint32         m_frag_offset;
	pf_Frag_Strux* m_sdh;

	const pf_Frag * m_frag;

	UTIterStatus    m_status;
	UT_uint32       m_max_offset;
	UT_uint32       m_strux_len;
};

#endif //PD_ITERATOR_H
