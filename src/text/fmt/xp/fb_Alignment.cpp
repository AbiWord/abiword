/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * BIDI Copyright (c) 2001,2002 Tomas Frydrych
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


#include "fb_Alignment.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_TextRun.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"

/////////////////////////////////////////////////////////////
// Alignment left
/////////////////////////////////////////////////////////////

void fb_Alignment_left::initialize(fp_Line * pLine )
{
		if(pLine->getBlock()->getDominantDirection() == UT_BIDI_RTL)
		{
		    m_iStartPosition = pLine->getRightThick() - pLine->calculateWidthOfTrailingSpaces();
	 	}
	 	else
	 	{
		    m_iStartPosition = pLine->getLeftThick();
		}
}

UT_sint32 fb_Alignment_left::getStartPosition()
{
	return m_iStartPosition;
}

void fb_Alignment_left::eraseLineFromRun(fp_Line *pLine, UT_uint32 runIndex)
{
	pLine->clearScreenFromRunToEnd(runIndex);
}

/////////////////////////////////////////////////////////////
// Alignment center
/////////////////////////////////////////////////////////////

void fb_Alignment_center::initialize(fp_Line *pLine)
{
	UT_sint32 iWidth = pLine->calculateWidthOfLine();
	UT_sint32 m_iExtraWidth = pLine->getAvailableWidth() - iWidth;
	if (m_iExtraWidth > 0)
	    m_startPosition = m_iExtraWidth / 2;
	else
	    m_startPosition = 0;
}

UT_sint32 fb_Alignment_center::getStartPosition()
{
	return m_startPosition;
}

void fb_Alignment_center::eraseLineFromRun(fp_Line *pLine, UT_uint32 /*runIndex*/)
{
	pLine->clearScreen();
}

/////////////////////////////////////////////////////////////
// Alignment right
/////////////////////////////////////////////////////////////

void fb_Alignment_right::initialize(fp_Line *pLine)
{
	UT_sint32 iTrailing = pLine->calculateWidthOfTrailingSpaces();
	UT_sint32 iWidth = pLine->calculateWidthOfLine() - iTrailing;

	m_startPosition = pLine->getAvailableWidth() - iWidth;
	
	if(pLine->getBlock()->getDominantDirection() == UT_BIDI_RTL)
	{
	     m_startPosition -= iTrailing;
	}
}

UT_sint32 fb_Alignment_right::getStartPosition()
{
	return m_startPosition;
}

void fb_Alignment_right::eraseLineFromRun(fp_Line *pLine, UT_uint32 runIndex)
{
	pLine->clearScreenFromRunToEnd(runIndex);
}

/////////////////////////////////////////////////////////////
// Alignment justify
/////////////////////////////////////////////////////////////

// If first line in block then spaces at the beginning of the
// line should be included and spaces at the end of the line
// should be excluded.

// If last line in block then no justification is required unless
// this is a RTL dominant block, in which case justify right

// Otherwise spaces at start and end of the lines should be ignored.

void fb_Alignment_justify::initialize(fp_Line *pLine)
{
	if (!pLine->isLastLineInBlock())
	{
		pLine->resetJustification(false); // non-permanent reset

		UT_sint32 iWidth = pLine->calculateWidthOfLine() - pLine->calculateWidthOfTrailingSpaces();

		m_iExtraWidth = pLine->getAvailableWidth() - iWidth;

		xxx_UT_DEBUGMSG(("fb_Alignment_justify::initialize (0x%x), iWidth %d, m_iExtraWidth %d\n",this,iWidth,m_iExtraWidth));
		pLine->justify(m_iExtraWidth);

		if(pLine->getBlock()->getDominantDirection() == UT_BIDI_RTL)
		{
			m_iStartPosition = pLine->getAvailableWidth();
		}
		else
		{
		  m_iStartPosition = pLine->getLeftThick();
		}
	}
	else if(pLine->getBlock()->getDominantDirection() == UT_BIDI_RTL) //this is RTL block, the last line behaves as if right-justified
	{
	    m_iStartPosition = pLine->getAvailableWidth();
	}
	else
	{
	    xxx_UT_DEBUGMSG(("Justified block, last line, left justified\n"));
	    m_iStartPosition = pLine->getLeftThick();
	}
}

UT_sint32 fb_Alignment_justify::getStartPosition()
{
	return m_iStartPosition;
}

void fb_Alignment_justify::eraseLineFromRun(fp_Line *pLine, UT_uint32 runIndex)
{
	pLine->clearScreenFromRunToEnd(runIndex);
}

