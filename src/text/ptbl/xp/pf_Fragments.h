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


#ifndef PF_FRAGMENTS_H
#define PF_FRAGMENTS_H

/*!
 pf_Fragments is a container for all of the pf_Frag
 derrived objects.  pf_Fragments provides a searchable,
 efficient ordering on the document fragments.

 Currently this consists of a simple doubly-linked list.
 We may need to add a tree structure on top of it, if we
 need to do various types of searches.
*/

#include <stdio.h>
#include "pf_Frag.h"
#include "ut_vector.h"

class pf_Fragments
{
public:
	pf_Fragments();
	~pf_Fragments();

	void					appendFrag(pf_Frag * pf);
	void					insertFrag(pf_Frag * pfPlace, pf_Frag * pfNew);
	void					unlinkFrag(pf_Frag * pf);
	void                    cleanFrags(void);
	void                    cleanFragsConst(void) const;
	pf_Frag *               getNthFrag( UT_uint32 nthFrag) const;
	pf_Frag *               findFirstFragBeforePos(PT_DocPosition pos) const;
	UT_uint32               getNumberOfFrags( void) const;
	UT_uint32               getFragNumber( const pf_Frag * pf) const;
	pf_Frag *				getFirst(void) const;
	pf_Frag *				getLast(void) const;
	void                    setFragsDirty(void) {m_bFragsClean = false;}
	bool                    areFragsDirty( void) const { return !m_bFragsClean;}

#ifdef PT_TEST
	void					__dump(FILE * fp) const;
#endif
	
protected:
private:
	pf_Frag *				m_pFirst;
	pf_Frag *				m_pLast;
	UT_Vector               m_vecFrags;
	bool                    m_bFragsClean;


};

#endif /* PF_FRAGMENTS_H */
