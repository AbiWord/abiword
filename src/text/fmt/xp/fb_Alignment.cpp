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

void fb_Alignment_left::initialize(fp_Line *pLine)
{
}

UT_sint32 fb_Alignment_left::getStartPosition()
{
	return 0;
}

UT_sint32 fb_Alignment_left::getMove(const fp_Run *pRun)
{
	return pRun->getWidth(); 
}

/////////////////////////////////////////////////////////////
// Alignment center
/////////////////////////////////////////////////////////////

void fb_Alignment_center::initialize(fp_Line *pLine)
{
	UT_sint32 iWidth = pLine->calculateWidthOfLine();

	UT_sint32 m_iExtraWidth = pLine->getMaxWidth() - iWidth;

	m_startPosition = m_iExtraWidth / 2;
}

UT_sint32 fb_Alignment_center::getStartPosition()
{
	return m_startPosition;
}

UT_sint32 fb_Alignment_center::getMove(const fp_Run *pRun)
{
	return pRun->getWidth(); 
}

/////////////////////////////////////////////////////////////
// Alignment right
/////////////////////////////////////////////////////////////

void fb_Alignment_right::initialize(fp_Line *pLine)
{
	UT_sint32 iWidth = pLine->calculateWidthOfLine();

	UT_sint32 m_iExtraWidth = pLine->getMaxWidth() - iWidth;

	m_startPosition = m_iExtraWidth;
}

UT_sint32 fb_Alignment_right::getStartPosition()
{
	return m_startPosition;
}

UT_sint32 fb_Alignment_right::getMove(const fp_Run *pRun)
{
	return pRun->getWidth(); 
}

/////////////////////////////////////////////////////////////
// Alignment justify
/////////////////////////////////////////////////////////////

// If first line in block then spaces at the beginning of the
// line should be included and spaces at the end of the line 
// should be excluded.

// If last line in block then no justification is required.

// Otherwise spaces at start and end of the lines should be ignored.

void fb_Alignment_justify::initialize(fp_Line *pLine)
{
	if (!pLine->isLastLineInBlock())
	{
		pLine->splitRunsAtSpaces();

		m_iSpaceCount = pLine->countSpaces();
		if (pLine->isLastCharacter(UCS_SPACE))
		{
			m_iSpaceCount--;
		}
		m_iSpaceCountLeft = m_iSpaceCount;
	
		UT_sint32 iWidth = pLine->calculateWidthOfLine();

		m_iExtraWidth = pLine->getMaxWidth() - iWidth;

		m_bRequiresJustification = UT_TRUE;
	}
	else
	{
		m_bRequiresJustification = UT_FALSE;
	}
}

UT_sint32 fb_Alignment_justify::getStartPosition()
{
	return 0;
}

UT_sint32 fb_Alignment_justify::getMove(const fp_Run *pRun)
{
	UT_sint32 iAmount;

	iAmount = pRun->getWidth();

	pRun = pRun->getNext();

	if(pRun && (pRun->getType() == FPRUN_TEXT))
	{
		// HACK: explicitly casting away the const so we can markJustification()
		// TODO: don't pass pRun as const?  (then can properly static_cast again)
		fp_TextRun *pTextRun = (fp_TextRun *)pRun;

		if (m_bRequiresJustification && 
			pTextRun->isFirstCharacter(UCS_SPACE) && 
			m_iSpaceCountLeft)	// TODO: decide what to do if m_iSpaceCount = 0;
		{
			UT_sint32 iExtra = (UT_sint32)((double)m_iExtraWidth / m_iSpaceCountLeft);
			
			// Distribute remaining with remaining spaces;

			m_iExtraWidth -= iExtra;
			m_iSpaceCountLeft--;

			pTextRun->markJustification(iAmount != 0);

			iAmount += iExtra; 
		}
	}

	return iAmount;
}
