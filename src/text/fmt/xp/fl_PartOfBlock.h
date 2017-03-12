/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indents-tab-mode:t; -*- */
/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * BIDI Copyright (c) 2001,2002 Tomas Frydrych
 * Copeyright (C) 2017 Hubert Figuière
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

#pragma once

#include <memory>
#include <string>

#include "ut_types.h"

/*
	This class is used to represent a part of the block.  Pointers
	to this class are the things contained in m_vecSquiggles and in
	FL_DocLayout::m_pPendingWordForSpell
*/
class ABI_EXPORT fl_PartOfBlock
{
public:
	fl_PartOfBlock();
	fl_PartOfBlock(UT_sint32 iOffset, UT_sint32 iPTLength,
				   bool bIsIgnored = false);

	bool             doesTouch(UT_sint32 iOffset, UT_sint32 iLength) const;
	inline UT_sint32 getOffset(void) const { return m_iOffset; }
	inline UT_sint32 getPTLength(void)const{ return m_iPTLength; }
	inline bool 	 getIsIgnored(void) const { return m_bIsIgnored; }

	inline void 	 setOffset(UT_sint32 iOffset) { m_iOffset = iOffset; }
	inline void 	 setPTLength(UT_sint32 iLength) { m_iPTLength = iLength; }
	inline void 	 setIsIgnored(bool bIsIgnored) { m_bIsIgnored = bIsIgnored; }
	void             setInvisible(void)
	{m_bIsInvisible = true;}
	bool             isInvisible(void) const
	{ return m_bIsInvisible;}
	void             setGrammarMessage(const std::string & sMsg);
	const std::string& getGrammarMessage() const;

private:
	UT_sint32	m_iOffset;
	UT_sint32   m_iPTLength;

	bool		m_bIsIgnored;
	bool        m_bIsInvisible;
	std::string m_sGrammarMessage;
};

// We'd rather have this be a unique_ptr<> but we the current code we can't
typedef std::shared_ptr<fl_PartOfBlock> fl_PartOfBlockPtr;
