/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * Copryight (C) 2017 Hubert Figuière
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


#include "fl_PartOfBlock.h"
#include "ut_debugmsg.h"

/*****************************************************************/

fl_PartOfBlock::fl_PartOfBlock(void)
    : m_iOffset(0),
      m_iPTLength(0),
      m_bIsIgnored(0),
      m_bIsInvisible(0)
{
}

fl_PartOfBlock::fl_PartOfBlock(UT_sint32 iOffset, UT_sint32 iPTLength,
							   bool bIsIgnored /* = false */):
	m_iOffset(iOffset),
	m_iPTLength(iPTLength),
	m_bIsIgnored(bIsIgnored),
	m_bIsInvisible(false)
{
}

void fl_PartOfBlock::setGrammarMessage(const std::string & sMsg)
{
	m_sGrammarMessage = sMsg;
}

const std::string&
fl_PartOfBlock::getGrammarMessage() const
{
	return m_sGrammarMessage;
}

/*!
  Does POB touch region
  \param iOffset Offset of region
  \param iLength Length of region
  \return True if the region touches the POB
*/
bool
fl_PartOfBlock::doesTouch(UT_sint32 iOffset, UT_sint32 iLength) const
{
	UT_sint32 start1, end1, start2, end2;

	xxx_UT_DEBUGMSG(("fl_PartOfBlock::doesTouch(%d, %d)\n", iOffset, iLength));

	start1 = m_iOffset;
	end1 = m_iOffset + m_iPTLength;

	start2 = iOffset;
	end2 =	 iOffset + iLength;

	if (end1 == start2)
	{
		return true;
	}
	if (end2 == start1)
	{
		return true;
	}

	/* they overlap */
	if ((start1 <= start2) && (start2 <= end1))
	{
		return true;
	}
	if ((start2 <= start1) && (start1 <= end2))
	{
		return true;
	}

	return false;
}
