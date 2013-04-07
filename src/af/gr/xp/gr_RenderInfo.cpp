/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2004 Tomas Frydrych
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

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_OverstrikingChars.h"
#include "ut_TextIterator.h"
#include "ut_string.h"
#include "gr_RenderInfo.h"

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

UT_sint32       GR_XPRenderInfo::s_iClassInstanceCount = 0;
UT_UCS4Char *   GR_XPRenderInfo::s_pCharBuff           = NULL;
UT_sint32 *     GR_XPRenderInfo::s_pWidthBuff          = NULL;
UT_sint32       GR_XPRenderInfo::s_iBuffSize           = 0;
UT_sint32 *     GR_XPRenderInfo::s_pAdvances           = NULL;
GR_RenderInfo * GR_XPRenderInfo::s_pOwner              = NULL;

GR_XPRenderInfo::GR_XPRenderInfo(GR_ScriptType type)
		:GR_RenderInfo(type),
		 m_pChars(NULL),
		 m_pWidths(NULL),
		 m_iBufferSize(0),
		 m_pSegmentOffset(NULL),
		 m_iSegmentCount(0),
		 m_iSpaceWidthBeforeJustification(0xfffffff), // note one less 'f'
		 m_iTotalLength(0)
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
		 m_iSpaceWidthBeforeJustification(0xfffffff) // not one less 'f'
{
	m_iOffset = offset;
	m_iLength = len;
	xxx_UT_DEBUGMSG(("GR_XPRender %x constructed \n"));
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
	xxx_UT_DEBUGMSG(("Deleting GR_XPRenderInfo %x \n",this));
    delete [] m_pChars;
	delete [] m_pWidths;
	m_pChars = NULL;
	m_pWidths = NULL;
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
		if(m_iSpaceWidthBeforeJustification == 0xfffffff) // note one less 'f'
			m_iSpaceWidthBeforeJustification = RI.m_iSpaceWidthBeforeJustification;
	
		m_iJustificationPoints += ri.m_iJustificationPoints;
		m_iJustificationAmount += ri.m_iJustificationAmount;
	}

	// mark static buffers dirty if needed
	if(s_pOwner == this)
		s_pOwner = NULL;

	m_bLastOnLine = RI.m_bLastOnLine;
	m_iTotalLength = m_iTotalLength + RI.m_iTotalLength;
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

	pri->m_pItem = m_pItem->makeCopy();
	UT_return_val_if_fail(pri->m_pItem,false);
	
	GR_XPRenderInfo * pRI = (GR_XPRenderInfo *)pri;
	
	UT_uint32 iPart2Len = m_iLength - m_iOffset;
	UT_uint32 iPart1Len = m_iLength - iPart2Len;

	m_iLength = iPart1Len;
	m_iTotalLength = iPart1Len;
	
	pRI->m_iLength = iPart2Len;
	pRI->m_iTotalLength = iPart2Len;

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

	pRI->m_bLastOnLine = m_bLastOnLine;
	m_bLastOnLine = false;
	
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

/**
   remove section of length iLen starting at offset from any chaches ...
   return value false indicates that simple removal was not possible
   and the caller needs to re-shape.
*/
bool GR_XPRenderInfo::cut(UT_uint32 offset, UT_uint32 iLen, bool /*bReverse*/)
{
	UT_return_val_if_fail(m_pText, false);
	// ascertain the state of the buffer and our shaping requirenments ...
	bool bRefresh = (((UT_uint32)m_eState & (UT_uint32)m_eShapingResult ) != 0);
	UT_sint32 ioffset = static_cast<UT_sint32>(offset);
	UT_sint32 jLen = static_cast<UT_sint32>(iLen);

	if(bRefresh)
		return false;
	
	m_iTotalLength -= jLen;

	// if we got here, we just need to cut out a bit of the draw
	// buffer
	UT_sint32 iLenToCopy = m_iLength - ioffset - jLen;

	if(m_iVisDir == UT_BIDI_RTL)
	{
		// if this is an rtl run, the end of the draw buffer corresponds to the start
		// section of the run, so we are moving not what is left after the deletion,
		// but what preceeds it
		iLenToCopy = ioffset;
	}
			
	UT_return_val_if_fail(iLenToCopy >= 0, false);
	if(iLenToCopy)
	{
		UT_UCS4Char * d = m_pChars+ioffset;
		UT_UCS4Char * s = m_pChars+ioffset+jLen;

		if(m_iVisDir == UT_BIDI_RTL)
		{
			d = m_pChars + (m_iLength - (ioffset + jLen));
			s = m_pChars + (m_iLength - ioffset);
		}

		UT_UCS4_strncpy(d, s, iLenToCopy);
		m_pChars[m_iLength - iLen] = 0;

		d = (UT_UCS4Char *) m_pWidths+ioffset;
		s = (UT_UCS4Char *) m_pWidths+ioffset+jLen;

		if(m_iVisDir == UT_BIDI_RTL)
		{
			d = (UT_UCS4Char *) m_pWidths + (m_iLength - (ioffset + jLen));
			s = (UT_UCS4Char *) m_pWidths + (m_iLength - ioffset);
		}

		UT_UCS4_strncpy(d, s, iLenToCopy);
		m_pWidths[m_iLength - jLen] = 0;
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

/*!
    This function no longer does any ligature handling (use the Pango 
	graphics for that);
    It still does some preprocessing on the static buffers that is needed by
    _calculateCharacterAdvances.
*/
void GR_XPRenderInfo::_stripLigaturePlaceHolders()
{
	UT_return_if_fail(m_iLength <= m_iBufferSize && m_pText);
	if(!m_pSegmentOffset)
		m_iSegmentCount = 0;

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

	if(m_iVisDir == UT_BIDI_RTL)
	{
		// we will be using addition on the width buffer so we need to
		// zerow it
		memset(s_pWidthBuff, 0, sizeof(UT_sint32)*m_iBufferSize);
		bReverse = true;
	}

	for(UT_sint32 i = 0, j = 0; i < len; i++, j++)
	{
			// ordinary character, just copy it and set the width as
			// appropriate
			s_pCharBuff[j] = m_pChars[i];

			if(bReverse)
				s_pWidthBuff[j] += m_pWidths[i];
			else
				s_pWidthBuff[j] = m_pWidths[i];
	}
}

/**	
   The following code calculates the advances for individual
   characters that are to be fed to gr_Graphics::drawChars()
   Note, that character advances are not necessarily identical to
   character widths; in the case of combining characters the
   required advance depends on the width of the base character and
   the properties of the combining character, and it can be both
   positive and negative.
   
   At the moment, we calculate the advances here puting them into
   a static array. Should this prove to be too much of a
   performance bottleneck, we could cache this in a member array,
   and refresh it inside refreshDrawBuffer() 
*/
void GR_XPRenderInfo::_calculateCharAdvances()
{
	if(m_iLength == 0)
		return;

	UT_return_if_fail(m_iLength <= m_iBufferSize);

	if(m_iVisDir == UT_BIDI_RTL )
	{
		// we expect the width array to be the result of processing by
		// _stripLigaturePlaceHolders(), which is in the same order as
		// the string to which it relates

		for(UT_sint32 n = 0; n < m_iLength; n++)
		{
			if(s_pWidthBuff[n] < 0 || s_pWidthBuff[n] >= GR_OC_LEFT_FLUSHED)
			{
				UT_sint32 iCumAdvance = 0;

				UT_sint32 m = n+1;
				while(m < (UT_sint32)m_iLength && s_pWidthBuff[m] < 0)
					m++;

				if(m >= m_iLength)
				{
					// problem: this run does not contain the
					// character over which we are meant to be
					// overimposing our overstriking chars
					// we will have to set the offsets to 0
					for(UT_sint32 k = n; k < m_iLength; k++)
						s_pAdvances[k] = 0;

					n = m_iLength;
				}
				else
				{
					UT_sint32 k;
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
		for(UT_sint32 n = 0; n < m_iLength; n++)
		{
			if((n < m_iLength - 1) && ((s_pWidthBuff[n+1] < 0) || (s_pWidthBuff[n+1] >= GR_OC_LEFT_FLUSHED)))
			{
				// remember the width of the non-zero character
				UT_sint32 iWidth = s_pWidthBuff[n];
				UT_sint32 iCumAdvance = 0;

				// find the next non-zerow char
				UT_sint32 m  = n + 1;
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


