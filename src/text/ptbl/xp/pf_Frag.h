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


#ifndef PF_FRAG_H
#define PF_FRAG_H

#include <stdio.h>
#include "ut_types.h"
#include "pt_Types.h"
class pt_PieceTable;
class PX_ChangeRecord;
class fd_Field;

/*!
 pf_Frag represents a fragment of the document.  This may be text
 (pf_Frag_Text), an inline object such as an image (pf_Frag_Object),
 or structure information such as a paragraph or section
 (pf_Frag_Strux_Block, pf_Frag_Strux_Section).

 pf_Frag is an abstract base class.
 We use an enum to remember type, rather than use any of the
 run-time stuff.
*/

class pf_Frag
{
public:
	typedef enum _PFType { PFT_Text, PFT_Object, PFT_Strux, PFT_EndOfDoc, PFT_FmtMark } PFType;

	pf_Frag(pt_PieceTable * pPT, PFType type, UT_uint32 length);
	virtual ~pf_Frag();

	inline PFType			getType(void) const		{ return m_type; }
	inline pf_Frag *		getNext(void) const		{ return m_next; }
	inline pf_Frag *		getPrev(void) const		{ return m_prev; }

	pf_Frag *				setNext(pf_Frag * pNext);
	pf_Frag *				setPrev(pf_Frag * pPrev);

	inline UT_uint32		getLength(void) const	{ return m_length; }
	pt_PieceTable *			getPieceTable(void) { return m_pPieceTable;}
	fd_Field *				getField(void);
	PT_DocPosition          getPos(void) const { return m_docPos;}
	void                    setPos(PT_DocPosition pos) { m_docPos = pos;}
	// createSpecialChangeRecord() constructs a change
	// record which describes the fragment itself and
	// not an actual change (editing) operation.  the
	// is used to initialize the listeners.
	
	virtual bool			createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
													  PT_DocPosition dpos) const;
	
#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif

protected:
	PFType					m_type;
	UT_uint32				m_length;	/* in PT_DocPosition-space */
	pf_Frag *				m_next;
	pf_Frag *				m_prev;
	
    fd_Field *              m_pField;
	pt_PieceTable *			m_pPieceTable;
private:
	PT_DocPosition          m_docPos;
};

#endif /* PF_FRAG_H */
