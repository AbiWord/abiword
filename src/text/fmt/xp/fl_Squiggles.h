/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

#ifndef FL_SQUIGGLES_H
#define FL_SQUIGGLES_H

#include "fl_BlockLayout.h"

typedef enum
{
  FL_SQUIGGLE_SPELL,
  FL_SQUIGGLE_GRAMMAR
} FL_SQUIGGLE_TYPE;

class ABI_EXPORT fl_Squiggles
{
public:
	fl_Squiggles(fl_BlockLayout* pOwner, FL_SQUIGGLE_TYPE iType);
virtual	~fl_Squiggles(void);

	void					add(fl_PartOfBlock* pPOB);
	void					markForRedraw(fl_PartOfBlock* pPOB);

	bool					deleteAll(void);

	fl_PartOfBlock*			get(UT_sint32 iOffset) const;
	inline fl_PartOfBlock*	getNth(UT_sint32 n) const
		{ return m_vecSquiggles.getNthItem(n); }

	void					clear(fl_PartOfBlock* pPOB);

	void					textInserted(UT_sint32 iOffset,
										 UT_sint32 iLength);
	void					textDeleted(UT_sint32 iOffset,
										UT_sint32 iLength);

	void                    textRevised(UT_sint32 iOffset,
										UT_sint32 iLength);

	FL_SQUIGGLE_TYPE        getSquiggleType(void) const
	  { return m_iSquiggleType;}

	void                    updatePOBs(UT_sint32 iOffset, UT_sint32 shift);
	void					join(UT_sint32 iOffset,
								 fl_BlockLayout* pPrevBL);
	void					split(UT_sint32 iOffset,
								  fl_BlockLayout* pNewBL);

	bool					findRange(UT_sint32 iStart, UT_sint32 iEnd,
									  UT_sint32& iFirst,
									  UT_sint32& iLast, bool bDontExpand = false) const;

	bool					recheckIgnoredWords(const UT_UCSChar* pBlockText);

#ifdef FMT_TEST
	void					__dump(FILE * fp) const;
#endif

private:
	void					_purge(void);

	void					_deleteNth(UT_sint32 iIndex);
	bool					_deleteAtOffset(UT_sint32 iOffset);

	bool					_findFirstAfter(UT_sint32 iOffset,
											UT_sint32& iIndex) const;
	UT_sint32				_find(UT_sint32 iOffset) const;

	void					_move(UT_sint32 iOffset, UT_sint32 chg,
								  fl_BlockLayout* pBlock=NULL);

	inline UT_sint32		_getCount(void) const
		{ return m_vecSquiggles.getItemCount(); }

	UT_GenericVector<fl_PartOfBlock*>	m_vecSquiggles;
	fl_BlockLayout*			m_pOwner;
	FL_SQUIGGLE_TYPE                m_iSquiggleType;
};

class ABI_EXPORT fl_SpellSquiggles : public fl_Squiggles
{
 public:
  fl_SpellSquiggles(fl_BlockLayout* pOwner);
};

class ABI_EXPORT fl_GrammarSquiggles : public fl_Squiggles
{
 public:
  fl_GrammarSquiggles(fl_BlockLayout* pOwner);
};


#endif /* FL_SQUIGGLES_H */




