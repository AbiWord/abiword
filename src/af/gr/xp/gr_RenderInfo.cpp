/* AbiWord
 * Copyright (C) 2004 Tomas Frydrych, <tomasfrydrych@yahoo.co.uk>
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

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_OverstrikingChars.h"
#include "ut_TextIterator.h"
#include "ut_string.h"
#include "gr_RenderInfo.h"
#include "gr_ContextGlyph.h"
#include "gr_Graphics.h"
#include <fribidi.h>



void GR_Itemization::clear()
{
	m_vOffsets.clear();

	UT_VECTOR_PURGEALL(GR_Item *, m_vItems);
	m_vItems.clear();
} 


//////////////////////////////////////////////////////////////////////////////////////////
//
// implementation of GRXPRenderInfo
//

#define GRIXP_STATIC_BUFFER_SIZE 256

UT_uint32       GR_XPRenderInfo::s_iClassInstanceCount = 0;
UT_UCS4Char *   GR_XPRenderInfo::s_pCharBuff           = NULL;
UT_sint32 *     GR_XPRenderInfo::s_pWidthBuff          = NULL;
UT_uint32       GR_XPRenderInfo::s_iBuffSize           = 0;
UT_sint32 *     GR_XPRenderInfo::s_pAdvances           = NULL;
GR_RenderInfo * GR_XPRenderInfo::s_pOwner              = NULL;

GR_XPRenderInfo::GR_XPRenderInfo(GR_ScriptType type)
		:GR_RenderInfo(type),
		 m_pChars(NULL),
		 m_pWidths(NULL),
		 m_iBufferSize(0),
		 m_pSegmentOffset(NULL),
		 m_iSegmentCount(0),
		 m_iSpaceWidthBeforeJustification(0xffffffff)
{
	_constructorCommonCode();
}
#if 0 
GR_XPRenderInfo::GR_XPRenderInfo(UT_UCS4Char *pChar,
				  UT_sint32 * pAdv,
				  UT_uint32 offset,
				  UT_uint32 len,
				  UT_uint32 iBufferSize,
				  GR_ScriptType type)
		:GR_RenderInfo(type),
		 m_pChars(pChar),
		 m_pWidths(NULL),
		 m_iBufferSize(iBufferSize),
		 m_pSegmentOffset(NULL),
		 m_iSegmentCount(0),
		 m_iSpaceWidthBeforeJustification(0xffffffff)
{
	m_iOffset = offset;
	m_iLength = len;

	_constructorCommonCode();
};
#endif
void GR_XPRenderInfo::_constructorCommonCode()
{
	if(!s_iClassInstanceCount)
	{
		s_pCharBuff = new UT_UCS4Char [GRIXP_STATIC_BUFFER_SIZE];
		UT_return_if_fail(s_pCharBuff);

		s_pWidthBuff = new UT_sint32 [GRIXP_STATIC_BUFFER_SIZE];
		UT_return_if_fail(s_pWidthBuff);

		s_pAdvances = new UT_sint32 [GRIXP_STATIC_BUFFER_SIZE];
		UT_return_if_fail(s_pAdvances);

		s_iBuffSize = GRIXP_STATIC_BUFFER_SIZE;
	}

	s_iClassInstanceCount++;
}


GR_XPRenderInfo::~GR_XPRenderInfo()
{
	--s_iClassInstanceCount;
	if(!s_iClassInstanceCount)
	{
		delete [] s_pCharBuff;    s_pCharBuff = NULL;
		delete [] s_pWidthBuff;   s_pWidthBuff = NULL;
		delete [] s_pAdvances;    s_pAdvances = NULL;

		s_pOwner = NULL;
	}

    delete [] m_pChars;
	delete [] m_pWidths;
}

/*!
    append data represented by ri to ourselves

    NB: combine the justification information

*/
bool GR_XPRenderInfo::append(GR_RenderInfo &ri, bool bReverse)
{
	GR_XPRenderInfo & RI = (GR_XPRenderInfo &) ri;
	
	if((m_iBufferSize <= m_iLength + RI.m_iLength) || (bReverse && (m_iLength > RI.m_iLength)))
	{
		xxx_UT_DEBUGMSG(("GR_RenderInfo::append: reallocating span buffer\n"));
		m_iBufferSize = m_iLength + RI.m_iLength + 1;
		UT_UCS4Char * pSB = new UT_UCS4Char[m_iBufferSize];
		UT_sint32 * pWB = new UT_sint32[m_iBufferSize];
		
		UT_return_val_if_fail(pSB && pWB, false);
		
		if(bReverse)
		{
			UT_UCS4_strncpy(pSB, RI.m_pChars, RI.m_iLength);
			UT_UCS4_strncpy(pSB + RI.m_iLength, m_pChars, m_iLength);
			
			UT_UCS4_strncpy((UT_UCS4Char*)pWB, (UT_UCS4Char*)RI.m_pWidths, RI.m_iLength);
			UT_UCS4_strncpy((UT_UCS4Char*)pWB + RI.m_iLength, (UT_UCS4Char*)m_pWidths, m_iLength);
		}
		else
		{
			UT_UCS4_strncpy(pSB,m_pChars, m_iLength);
			UT_UCS4_strncpy(pSB + m_iLength, RI.m_pChars, RI.m_iLength);

			UT_UCS4_strncpy((UT_UCS4Char*)pWB,(UT_UCS4Char*)m_pWidths, m_iLength);
			UT_UCS4_strncpy((UT_UCS4Char*)pWB + m_iLength, (UT_UCS4Char*)RI.m_pWidths, RI.m_iLength);
		}

		*(pSB + m_iLength + RI.m_iLength) = 0;
		delete [] m_pChars;
		delete [] m_pWidths;
		
		m_pChars = pSB;
		m_pWidths = pWB;
	}
	else
	{
		UT_DEBUGMSG(("mergeWithNext: reusing existin span buffer\n"));
		if(bReverse)
		{
			// can only shift the text directly in the existing buffer if
			// getLength() <= pNext->getLength()
			UT_return_val_if_fail(m_iLength <= RI.m_iLength, false);
			UT_UCS4_strncpy(m_pChars + RI.m_iLength, m_pChars, m_iLength);
			UT_UCS4_strncpy(m_pChars, RI.m_pChars, RI.m_iLength);
			
			UT_UCS4_strncpy((UT_UCS4Char*)m_pWidths + RI.m_iLength,
							(UT_UCS4Char*)m_pWidths, m_iLength);
			
			UT_UCS4_strncpy((UT_UCS4Char*)m_pWidths,
							(UT_UCS4Char*)RI.m_pWidths, RI.m_iLength);
		}
		else
		{
			UT_UCS4_strncpy(m_pChars + m_iLength, RI.m_pChars, RI.m_iLength);
			
			UT_UCS4_strncpy((UT_UCS4Char*)m_pWidths + m_iLength,
							(UT_UCS4Char*)RI.m_pWidths, RI.m_iLength);
		}
		*(m_pChars + m_iLength + RI.m_iLength) = 0;
	}

	if( RI.m_iJustificationPoints
		|| m_iJustificationPoints)
	{
		// the text is justified, merge the justification information
		if(m_iSpaceWidthBeforeJustification == 0xffffffff)
			m_iSpaceWidthBeforeJustification = RI.m_iSpaceWidthBeforeJustification;
	
		m_iJustificationPoints += ri.m_iJustificationPoints;
		m_iJustificationAmount += ri.m_iJustificationAmount;
	}

	// mark static buffers dirty if needed
	if(s_pOwner == this)
		s_pOwner = NULL;
	
	return true;
}

/*!
    creates a new instance of GR_*RenderInfo and splits data between
    ourselves and it at offset

    bReverse == true indicates data in RTL order

    we also calculate justification info for the two parts
*/
bool  GR_XPRenderInfo::split (GR_RenderInfo *&pri, bool bReverse)
{
	UT_ASSERT( !pri );
	pri = new GR_XPRenderInfo(m_eScriptType);
	UT_return_val_if_fail(pri, false);

	GR_XPRenderInfo * pRI = (GR_XPRenderInfo *)pri;
	
	UT_uint32 iPart2Len = m_iLength - m_iOffset;
	UT_uint32 iPart1Len = m_iLength - iPart2Len;

	m_iLength = iPart1Len;
	pRI->m_iLength = iPart2Len;

	// the question is whether we want to shrink the buffer here (and
	// save memory) or leave it too big (and save time); go for memory
	// for now
	UT_UCS4Char * pSB = new UT_UCS4Char[m_iLength + 1];
	UT_sint32   * pWB = new UT_sint32[m_iLength + 1];
	
	UT_return_val_if_fail(pSB && pWB, false);
	
	m_iBufferSize = iPart1Len;
	
	pRI->m_pChars = new UT_UCS4Char[iPart2Len + 1];
	pRI->m_pWidths = new UT_sint32[iPart2Len + 1];
	
	UT_return_val_if_fail(pRI->m_pChars && pRI->m_pWidths, false);
	pRI->m_iBufferSize = iPart2Len;
	
	
	if(bReverse)
	{
		UT_UCS4_strncpy(pSB, m_pChars + pRI->m_iLength, m_iLength);
		UT_UCS4_strncpy(pRI->m_pChars, m_pChars, pRI->m_iLength);
		
		UT_UCS4_strncpy((UT_UCS4Char*)pWB, (UT_UCS4Char*)m_pWidths + pRI->m_iLength, m_iLength);
		UT_UCS4_strncpy((UT_UCS4Char*)pRI->m_pWidths,
						(UT_UCS4Char*)m_pWidths, pRI->m_iLength);
	}
	else
	{
		UT_UCS4_strncpy(pSB, m_pChars, m_iLength);
		UT_UCS4_strncpy(pRI->m_pChars, m_pChars + m_iLength, pRI->m_iLength);

		UT_UCS4_strncpy((UT_UCS4Char*)pWB,(UT_UCS4Char*)m_pWidths, m_iLength);
		UT_UCS4_strncpy((UT_UCS4Char*)pRI->m_pWidths,
						(UT_UCS4Char*)m_pWidths + m_iLength, pRI->m_iLength);
	}

	pSB[m_iLength] = 0;
	
	pRI->m_pChars[pRI->m_iLength] = 0;

	delete[] m_pChars;
	m_pChars = pSB;

	delete[] m_pWidths;
	m_pWidths = pWB;
	
	pRI->m_eShapingResult = m_eShapingResult;

	// Deal with justification
	// this has to be always done (used by isJustified())
	pRI->m_iSpaceWidthBeforeJustification = m_iSpaceWidthBeforeJustification;

	if(!isJustified())
	{
		// we are done
		return true;
	}
	
	
	UT_return_val_if_fail(m_pGraphics, false);
	pRI->m_pGraphics = m_pGraphics;

	UT_sint32 iPoints = m_pGraphics->countJustificationPoints(*pRI);
	pRI->m_iJustificationPoints = abs(iPoints);

	if(!iPoints)
	{
		// the latter section has no justification points, all stays
		// as is
		pRI->m_iJustificationAmount = 0;
		return true;
	}

	iPoints = m_pGraphics->countJustificationPoints(*this);

	if(!iPoints)
	{
		// all justification is done in the latter section
		pRI->m_iJustificationAmount = m_iJustificationAmount;
		pRI->m_iJustificationPoints = m_iJustificationPoints;

		m_iJustificationAmount = 0;
		m_iJustificationPoints = 0;

		return true;
	}
	
	// work out how much of the original amount falls on the new pRI
	UT_return_val_if_fail(m_iJustificationPoints, false);
	UT_sint32 iAmount = m_iJustificationAmount * pRI->m_iJustificationPoints / m_iJustificationPoints;
	pRI->m_iJustificationAmount = iAmount;

	m_iJustificationAmount -= iAmount;
	m_iJustificationPoints = abs(iPoints);
	
	return true;
}

/*
   remove section of length iLen starting at offset from any chaches ...
   return value false indicates that simple removal was not possible
   and the caller needs to re-shape.
*/
bool GR_XPRenderInfo::cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse)
{
	UT_return_val_if_fail(m_pText, false);
	
	// ascertain the state of the buffer and our shaping requirenments ...
	bool bRefresh = (((UT_uint32)m_eState & (UT_uint32)m_eShapingResult ) != 0);

	if(bRefresh)
		return false;
	
	bool bLigatures = (((UT_uint32)m_eShapingResult & (UT_uint32) GRSR_Ligatures) != 0);
	bool bContext = (((UT_uint32)m_eShapingResult & (UT_uint32) GRSR_ContextSensitive) != 0);

	GR_ContextGlyph cg;
	UT_UCS4Char c;

	UT_uint32 pos = m_pText->getPosition();

	if(!bRefresh && bLigatures)
	{
		// we need to recalculate the draw buffer if the character
		// left of the deleted section is susceptible to ligating or
		// if the two characters around the right edge of the deletion
		// form a ligagure

		// start with the right boundary, as that is computationally
		// easier
		if(offset + iLen < m_iLength)
		{
			// the easiest way of checking for presence of ligature
			// glyph is to check for the presence of the placeholder
			UT_uint32 off2  = offset + iLen;

			if(m_iVisDir == FRIBIDI_TYPE_RTL)
			{
				off2 = m_iLength - off2 - 1;
			}

			bRefresh |= (m_pChars[off2] == UCS_LIGATURE_PLACEHOLDER);
		}

		// now the left boundary
		if(!bRefresh && offset > 0)
		{
			m_pText->setPosition(pos + offset - 1);
			if(m_pText->getStatus() == UTIter_OK)
			{
				c = m_pText->getChar();
				bRefresh |= !cg.isNotFirstInLigature(c);
			}
		}
	}

	if(!bRefresh && bContext)
	{
		// we need to retrieve the characters left and right of the
		// deletion
		if(offset > 0)
		{
			m_pText->setPosition(pos + offset - 1);
			if(m_pText->getStatus() == UTIter_OK)
			{
				c = m_pText->getChar();
				bRefresh |= !cg.isNotContextSensitive(c);
			}
		}

		if(!bRefresh && offset + iLen < m_iLength)
		{
			// this function is called in response to the PT being
			// already changed, i.e., the character that used to be at
			// offset + iLen is now at offset
			m_pText->setPosition(pos + offset);
			if(m_pText->getStatus() == UTIter_OK)
			{
				c = m_pText->getChar();
				bRefresh |= !cg.isNotContextSensitive(c);
			}
		}
	}

	if(bRefresh)
	{
		return false;
	}
	else
	{
		// if we got here, we just need to cut out a bit of the draw
		// buffer
		UT_uint32 iLenToCopy = m_iLength - offset - iLen;

		if(iLenToCopy)
		{
			UT_UCS4Char * d = m_pChars+offset;
			UT_UCS4Char * s = m_pChars+offset+iLen;

			if(m_iVisDir == FRIBIDI_TYPE_RTL)
			{
				d = m_pChars + (m_iLength - (offset + iLen - 1));
				s = m_pChars + (m_iLength - offset);
			}

			UT_UCS4_strncpy(d, s, iLenToCopy);
			m_pChars[m_iLength - iLen] = 0;

			d = (UT_UCS4Char *) m_pWidths+offset;
			s = (UT_UCS4Char *) m_pWidths+offset+iLen;

			if(m_iVisDir == FRIBIDI_TYPE_RTL)
			{
				d = (UT_UCS4Char *) m_pWidths + (m_iLength - offset + iLen - 1);
				s = (UT_UCS4Char *) m_pWidths + (m_iLength - offset);
			}

			UT_UCS4_strncpy(d, s, iLenToCopy);
			m_pWidths[m_iLength - iLen] = 0;
		}
	}

	// mark static buffers dirty if needed
	if(s_pOwner == this)
		s_pOwner = NULL;
	
	return true;
}

void GR_XPRenderInfo::prepareToRenderChars()
{
	if(s_pOwner == this)
	{
		// we currently own the static buffers, so we do not need to
		// do anything
		return;
	}
	
	// make sure that the static buffers where we temporarily store
	// information are big enough
	UT_return_if_fail(_checkAndFixStaticBuffers());
	
	// strip placeholders and adjust segment offsets accordingly
	_stripLigaturePlaceHolders();

	// calculate advances from the pre-processed buffer
	_calculateCharAdvances();

	s_pOwner = this;
}


void GR_XPRenderInfo::_stripLigaturePlaceHolders()
{
	UT_return_if_fail(m_iLength <= m_iBufferSize && m_pText && m_pSegmentOffset);

	// this is sligthly complicated by having to deal with both
	// logical and visual coordinances at the same time
	//
	// i and j work in visual coordiances, and correspond to the
	// orignal pChars array and the s_pCharBuffer
	//
	// m and iSplitOffset are in logical coordinaces, m being index
	// into pWidths, and iSplitOffset a value comparable to pOffset
	// values (also in logical order)
	UT_sint32 len = (UT_sint32) m_iLength;
	bool bReverse = false;

	if(m_iVisDir == FRIBIDI_TYPE_RTL)
	{
		// we will be using addition on the width buffer so we need to
		// zerow it
		memset(s_pWidthBuff, 0, sizeof(UT_sint32)*m_iBufferSize);
		bReverse = true;
	}

	UT_uint32 iOffset = m_pText->getPosition();
	
	for(UT_sint32 i = 0, j = 0; i < len; i++, j++)
	{
		// m is the logical offeset corresponding to the visual offest i
		// UT_sint32 m = bReverse ? len - i - 1 : i;
		UT_sint32 m = i;

		if(m_pChars[i] != UCS_LIGATURE_PLACEHOLDER)
		{
			// ordinary character, just copy it and set the width as
			// appropriate
			s_pCharBuff[j] = m_pChars[i];

			if(bReverse)
				s_pWidthBuff[j] += m_pWidths[m];
			else
				s_pWidthBuff[j] = m_pWidths[m];

		}
		else
		{
			// we will remember whether this ligature is split by the
			// selection for later use
			bool bSplitLigature = false;

			// iSplitOffset is the offset of the middle of the
			// ligature in logical coordinances; j is visual offset
			// into our output buffer
			UT_uint32 iSplitOffset = bReverse ? m_iLength - j - 1: j;

			// scroll through the offset array; the offsets define the
			// segments into which the run is split by the selection
			// NB: there is always one more (dummy) offset than iOffsetCount
			for(UT_sint32 k = 0; k <= (UT_sint32)m_iSegmentCount; k++)
			{
				if(!m_pSegmentOffset[k]
				   || static_cast<UT_sint32>(m_pSegmentOffset[k]) < static_cast<UT_sint32>(iSplitOffset))
					continue;

				if(static_cast<UT_sint32>(m_pSegmentOffset[k]) == static_cast<UT_sint32>(iSplitOffset))
				{
					// this is the case, where the ligature
					// placeholder is the first character in a
					// segment, i.e., the ligature is split by the
					// selection -- we have to feed the decomposed
					// (orginal) glyphs into the output string, but
					// use the original width metrics

					// get the second decomposed character from our
					// piece table
					UT_UCS4Char c = (*m_pText)[iOffset + m];

					if(m_pText->getStatus() == UTIter_OK)
					{
						s_pCharBuff[j] = c;
					}
					else
					{
						// we failed to get the character from the
						// piecetable, handle it gracefully
						UT_ASSERT(UT_NOT_REACHED );
						s_pCharBuff[j] = '?';
					}

					// set the width for this glyph
					s_pWidthBuff[j] = m_pWidths[m];

					// now set the first part of the decomposed glyph,
					// taking into account direction
					UT_sint32 n = bReverse ? j + 1: j - 1;

					// now get the first character for this ligature;
					// we can only do this if it is not outside our run
					if(m > 0 && n >= 0)
					{
						c = (*m_pText)[iOffset + m - 1];

						if(m_pText->getStatus() == UTIter_OK)
						{
							s_pCharBuff[n] = c;
						}
						else
						{
							// we failed to get the character from the
							// piecetable, handle it gracefully
							UT_ASSERT(UT_NOT_REACHED );
							s_pCharBuff[n] = '?';
						}

						if(bReverse)
						{
							//we have already processed the next
							//character, we only need to set the width
							//for it as well and then can skip the
							//next iteration of the loop
							i++;
							j++;

							s_pWidthBuff[j] = m_pWidths[m-1];
						}
					}

					bSplitLigature = true;
					break;
				}

				// this the case of pOffset[k] > j, just need to
				// adjust it, since we removed the ligature
				// placeholder from the string
				m_pSegmentOffset[k]--;
			}

			if(!bSplitLigature)
			{
				// we have a ligature which is either completely
				// selected, or completely outwith the selection; all
				// we need to do is to set the widths and adjust our
				// indexes (we are removing this charcter, the
				// placeholder, from the buffer).
				if(bReverse)
				{
					// the first part of the ligature will come at the j
					// index, we want to include the half width of the
					// second character
					s_pWidthBuff[j] = m_pWidths[m];
				}

				j--;
				m_iLength--;

				if(j >= 0 && !bReverse)
				{
					s_pWidthBuff[j] += m_pWidths[m];
				}
			}
		}
	}
}


void GR_XPRenderInfo::_calculateCharAdvances()
{
	// The following code calculates the advances for individual
	// characters that are to be fed to gr_Graphics::drawChars()
	// Note, that character advances are not necessarily identical to
	// character widths; in the case of combining characters the
	// required advance depends on the width of the base character and
	// the properties of the combining character, and it can be both
	// positive and negative.
	//
	// At the moment, we calculate the advances here puting them into
	// a static array. Should this prove to be too much of a
	// performance bottleneck, we could cache this in a member array,
	// and refresh it inside refreshDrawBuffer()

	if(m_iLength == 0)
		return;

	UT_return_if_fail(m_iLength <= m_iBufferSize);

	if(m_iVisDir == FRIBIDI_TYPE_RTL )
	{
		// we expect the width array to be the result of processing by
		// _stripLigaturePlaceHolders(), which is in the same order as
		// the string to which it relates

		for(UT_uint32 n = 0; n < m_iLength; n++)
		{
			if(s_pWidthBuff[n] < 0 || s_pWidthBuff[n] >= GR_OC_LEFT_FLUSHED)
			{
				UT_sint32 iCumAdvance = 0;

				UT_uint32 m = n+1;
				while(m < (UT_sint32)m_iLength && s_pWidthBuff[m] < 0)
					m++;

				if(m >= m_iLength)
				{
					// problem: this run does not contain the
					// character over which we are meant to be
					// overimposing our overstriking chars
					// we will have to set the offsets to 0
					for(UT_uint32 k = n; k < m_iLength; k++)
						s_pAdvances[k] = 0;

					n = m_iLength;
				}
				else
				{
					UT_uint32 k;
					for(k = n; k < m; k++)
					{
						UT_sint32 iAdv;
						if(s_pWidthBuff[k] >= GR_OC_LEFT_FLUSHED)
						{
							UT_sint32 iThisWidth = s_pWidthBuff[k] & GR_OC_MAX_WIDTH;
							iAdv = s_pWidthBuff[m] - iThisWidth - iCumAdvance;
						}
						else
						{
							// centered character
							iAdv = (s_pWidthBuff[m] + s_pWidthBuff[k])/2 - iCumAdvance;
						}

						if(k == 0)
						{
							// k == 0, this is the leftmost character,
							// so we have no advance to set, but we
							// can adjust the starting point of the drawing
							m_xoff += iAdv;
						}
						else if(k == n)
						{
							// this is a special case; we have already
							// calculated the advance in previous
							// round of the main loop, and this is
							// only adjustment
							s_pAdvances[k-1] += iAdv;
						}
						else
							s_pAdvances[k-1] = iAdv;

						iCumAdvance += iAdv;
					}

					s_pAdvances[k-1] = -iCumAdvance;
					s_pAdvances[k]   = s_pWidthBuff[m];
					n = k; // should be k+1, but there will be n++ in
					       // the for loop
				}

			}
			else
			{
				s_pAdvances[n] = s_pWidthBuff[n];
			}
		}
	}
	else
	{
		for(UT_uint32 n = 0; n < m_iLength; n++)
		{
			if(s_pWidthBuff[n+1] < 0 || s_pWidthBuff[n+1] >= GR_OC_LEFT_FLUSHED)
			{
				// remember the width of the non-zero character
				UT_sint32 iWidth = s_pWidthBuff[n];
				UT_sint32 iCumAdvance = 0;

				// find the next non-zerow char
				UT_uint32 m  = n + 1;
				while(m < m_iLength && s_pWidthBuff[m] < 0)
				{
					// plus because pCharWidths[m] < 0
					// -1 because it is between m-1 and m
					UT_sint32 iAdv;
					if(s_pWidthBuff[m] >= GR_OC_LEFT_FLUSHED)
					{
						UT_sint32 iThisWidth = s_pWidthBuff[m] & GR_OC_MAX_WIDTH;
						iThisWidth -= iWidth;

						iAdv = -(iThisWidth - iCumAdvance);
					}
					else
					{
						//centered character
						iAdv = iWidth - (iWidth + s_pWidthBuff[m])/2 + iCumAdvance;
					}

					s_pAdvances[m-1] = iAdv;
					iCumAdvance += iAdv;
					m++;
				}

				n = m-1; // this is the last 0-width char
				s_pAdvances[n] = iWidth - iCumAdvance;
			}
			else
				s_pAdvances[n] = s_pWidthBuff[n];
			xxx_UT_DEBUGMSG(("%d ",s_pAdvances[n],s_pWidthBuff[n] ));
		}
		xxx_UT_DEBUGMSG(("ENDRUN \n"));

	}
}

bool GR_XPRenderInfo::_checkAndFixStaticBuffers()
{
	// TODO -- FIX ME !!!
	if(m_iLength > s_iBuffSize)
	{
		delete [] s_pCharBuff;
		s_pCharBuff = new UT_UCS4Char [m_iLength];
		UT_return_val_if_fail(s_pCharBuff, false);

		delete [] s_pWidthBuff;
		s_pWidthBuff = new UT_sint32 [m_iLength];
		UT_return_val_if_fail(s_pWidthBuff,false);

		delete [] s_pAdvances;
		s_pAdvances = new UT_sint32 [m_iLength];
		UT_return_val_if_fail(s_pAdvances,false);
		
		s_iBuffSize = m_iLength;
	}

	return true;
}


