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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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

void fb_Alignment_left::initialize(fp_Line * /*pLine*/ )
{
}

UT_sint32 fb_Alignment_left::getStartPosition()
{
	return 0;
}

UT_sint32 fb_Alignment_left::getStartPositionInLayoutUnits()
{
	return 0;
}

void fb_Alignment_left::eraseLineFromRun(fp_Line *pLine, UT_uint32 runIndex)
{
//  	if(runIndex > 0)
//  	{
//  		// Erase from the previous run.
//  		// This is required for characters that have part of their glyph
//  		// visible before their character position. eg bottom of a 'j' in
//  		// Times New Roman.

//  		runIndex--;
//  	}
	pLine->clearScreenFromRunToEnd(runIndex);
	
}

/////////////////////////////////////////////////////////////
// Alignment center
/////////////////////////////////////////////////////////////

void fb_Alignment_center::initialize(fp_Line *pLine)
{
	UT_sint32 iWidth = pLine->calculateWidthOfLine();

	UT_sint32 m_iExtraWidth = pLine->getMaxWidth() - iWidth;

	m_startPosition = m_iExtraWidth / 2;

	UT_sint32 iWidthLayoutUnits = pLine->calculateWidthOfLineInLayoutUnits();

	UT_sint32 m_iExtraWidthLayoutUnits = pLine->getMaxWidthInLayoutUnits() - iWidthLayoutUnits;

	m_startPositionLayoutUnits = m_iExtraWidthLayoutUnits / 2;
}

UT_sint32 fb_Alignment_center::getStartPosition()
{
	return m_startPosition;
}

UT_sint32 fb_Alignment_center::getStartPositionInLayoutUnits()
{
	return m_startPositionLayoutUnits;
}

void fb_Alignment_center::eraseLineFromRun(fp_Line *pLine, UT_uint32 runIndex)
{
	pLine->clearScreen();
}

/////////////////////////////////////////////////////////////
// Alignment right
/////////////////////////////////////////////////////////////

void fb_Alignment_right::initialize(fp_Line *pLine)
{
	UT_sint32 iWidth = pLine->calculateWidthOfLine() - pLine->calculateWidthOfTrailingSpaces();

	m_startPosition = pLine->getMaxWidth() - iWidth;

	UT_sint32 iWidthLayoutUnits = pLine->calculateWidthOfLineInLayoutUnits() - pLine->calculateWidthOfTrailingSpacesInLayoutUnits();

	m_startPositionLayoutUnits = pLine->getMaxWidthInLayoutUnits() - iWidthLayoutUnits;
}

UT_sint32 fb_Alignment_right::getStartPosition()
{
	return m_startPosition;
}

UT_sint32 fb_Alignment_right::getStartPositionInLayoutUnits()
{
	return m_startPositionLayoutUnits;
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
		pLine->resetJustification();

		UT_sint32 iWidth = pLine->calculateWidthOfLine() - pLine->calculateWidthOfTrailingSpaces();

		m_iExtraWidth = pLine->getMaxWidth() - iWidth;

		pLine->distributeJustificationAmongstSpaces(m_iExtraWidth);

#ifdef BIDI_ENABLED
	    m_iStartPosition = 0;
	    m_iStartPositionLayoutUnits = 0;
#endif

#ifndef NDEBUG	
		_confirmJustification(pLine);
#endif

	}
#ifdef BIDI_ENABLED
	else if(pLine->getBlock()->getDominantDirection()) //this is RTL block, the last line behaves as if right-justified
	{
		//UT_DEBUGMSG(("Justified block, last line, right justified\n"));
		UT_sint32 iWidth = pLine->calculateWidthOfLine() - pLine->calculateWidthOfTrailingSpaces();
		/*UT_sint32*/ m_iExtraWidth = pLine->getMaxWidth() - iWidth;
		m_iStartPosition = m_iExtraWidth;

		UT_sint32 iWidthLayoutUnits = pLine->calculateWidthOfLineInLayoutUnits() - pLine->calculateWidthOfTrailingSpacesInLayoutUnits();
		m_iStartPositionLayoutUnits = pLine->getMaxWidthInLayoutUnits() - iWidthLayoutUnits;
	}
	else
	{
		//UT_DEBUGMSG(("Justified block, last line, left justified\n"));
	    m_iStartPosition = 0;
	    m_iStartPositionLayoutUnits = 0;
	}
#endif
}

UT_sint32 fb_Alignment_justify::getStartPosition()
{
#ifdef BIDI_ENABLED
	//UT_DEBUGMSG(("Alignment_justify::getStartPosition : %d\n", m_iStartPosition));
	return m_iStartPosition;
#else
	return 0;
#endif
}

UT_sint32 fb_Alignment_justify::getStartPositionInLayoutUnits()
{
#ifdef BIDI_ENABLED
	//UT_DEBUGMSG(("Alignment_justify::getStartPositionInLayoutUnits : %d\n", m_iStartPositionLayoutUnits));
	return m_iStartPositionLayoutUnits;
#else
	return 0;
#endif
}

#ifndef NDEBUG

void fb_Alignment_justify::_confirmJustification(fp_Line *pLine)
{
	/*
	UT_sint32 iJustifiedLength = pLine->calculateWidthOfLine() - pLine->calculateWidthOfTrailingSpaces();
	UT_sint32 iLineLength = pLine->getMaxWidth();

//	UT_ASSERT(iJustifiedLength == iLineLength);
	*/
}

#endif /* NDEBUG */

void fb_Alignment_justify::eraseLineFromRun(fp_Line *pLine, UT_uint32 runIndex)
{
	pLine->clearScreenFromRunToEnd(runIndex);
}

