 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/


#include <stdlib.h>
#include <string.h>

#include "fl_BlockLayout.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fb_LineBreaker.h"
#include "fp_Column.h"
#include "fp_BlockSlice.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "pd_Document.h"
#include "pp_Property.h"
#include "dg_Graphics.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"

#define EXTRA_CHARWIDTH_SPACE 256

FL_BlockLayout::FL_BlockLayout(PL_StruxDocHandle sdh,
							   FB_LineBreaker* pBreaker,
							   FL_BlockLayout* pPrev,
							   FL_SectionLayout* pSectionLayout)
	: fl_Layout(PTX_Block, sdh)
{
	m_pSectionLayout = pSectionLayout;
	m_pBreaker = pBreaker;
	m_pFirstRun = NULL;
	m_pFirstLine = NULL;
	m_pLastLine = NULL;
	m_pCurrentSlice = NULL;
	m_bFormatting = UT_FALSE;
	m_pCharWidths = NULL;
	m_iCharWidthSize = 0;
	m_iCharWidthSpace = 0;
	m_bNeedsReformat = UT_FALSE;
	
	m_pLayout = m_pSectionLayout->getLayout();
	m_pDoc = m_pLayout->getDocument();

#ifdef BUFFER	// calculate m_iCharWidthSize
	UT_uint32 posCur = m_pBuffer->getMarkerPosition(m_sdh);
	UT_uint32 posBase = posCur;
	for (;;)
	{
	    UT_UCSChar ch;
	    DG_DocMarkerId dmid;
	    DG_DocMarker* pMarker = NULL;
	    
	    UT_Bool bMarker = (m_pBuffer->getOneItem(posCur, &ch, &dmid) == DG_DBPI_MARKER);
	    if (bMarker)
		{
			pMarker = m_pBuffer->fetchDocMarker(dmid);
			DG_DocMarkerType dmt = pMarker->getType();
			if ((dmt & DG_MT_BLOCK) && (dmt & DG_MT_END))
			{
				// we found the end of this block
				m_pEndBlockMarker = pMarker;
				m_iCharWidthSize = posCur - posBase + 1;
				break;
			}
		}
	    m_pBuffer->inc(posCur);
	}
#endif /* BUFFER */

	m_pPrev = pPrev;
	if (m_pPrev)
	{
		m_pNext = pPrev->m_pNext;
		m_pPrev->m_pNext = this;
		if (m_pNext)
		{
			m_pNext->m_pPrev = this;
		}
	}
	else
	{
		m_pNext = NULL;
	}
}

FL_BlockLayout::~FL_BlockLayout()
{
	_purgeLayout(UT_FALSE);
}

/*
	_createNewSlice is only called when a new slice is needed to continue a flow
	which is already in progress.  This routine is not used to create the first
	slice of the block.
*/
void FL_BlockLayout::_createNewSlice()
{
	UT_ASSERT(m_pCurrentSlice);

	FP_Column* pCol = m_pCurrentSlice->getColumn();
	UT_ASSERT(pCol);

	pCol = pCol->getNext();
	if (!pCol)
	{
		/*
			This happens when we have hit the last column in the FL_SectionLayout
		*/
		pCol = m_pSectionLayout->getNewColumn();
	}

	// Could it be NULL due to outofmem?  Perhaps we shouldn't assert - TODO

	FP_BlockSlice* pNewSlice = new FP_BlockSlice(this);
	_addSlice(pNewSlice);

	UT_ASSERT(m_pFirstRun);
	int err = pCol->insertBlockSliceAfter(pNewSlice, NULL, m_pFirstRun->getHeight());
	UT_ASSERT(err == 0);
}

/*
	_verifyCurrentSlice is used to ensure that the first slice of the block has
	been created.  If not, it creates one.  By default, it is created immediately
	after the last slice in the previous block.  If there is no previous block, then
	we assume this block to be the first one in its flow.  In that case, we simply
	find the first page which has a column into which our flow can be placed, and
	create a new slice at the top of that column.
*/
void FL_BlockLayout::_verifyCurrentSlice()
{
	if (m_pCurrentSlice)
	{
		return;
	}

	// There is no current slice.  We need to start one.

	FP_BlockSlice* pNewSlice = new FP_BlockSlice(this);
	_addSlice(pNewSlice);

	UT_DEBUGMSG(("_verifyCurrentSlice: FL_BlockLayout=0x%x, m_pPrev=0x%x\n", this, m_pPrev));

	UT_uint32 iLineHeight;
	if (m_pFirstRun)
	{
		iLineHeight = m_pFirstRun->getHeight();
	}
	else
	{
		// this must be an empty paragraph
		iLineHeight = 20;		// TODO hack -- this is a guess.
		// we cannot pass 0, because a circular column will choke
	}
	
	if (m_pPrev)
	{
		FP_BlockSlice* pPrevSlice = m_pPrev->getLastSlice();
		UT_ASSERT(pPrevSlice);
		UT_DEBUGMSG(("_verifyCurrentSlice:  pPrevSlice=0x%x\n", pPrevSlice));

		FP_Column* pCol = pPrevSlice->getColumn();
		UT_ASSERT(pCol);
		UT_DEBUGMSG(("_verifyCurrentSlice:  pCol=0x%x\n", pCol));

		if (pCol->insertBlockSliceAfter(pNewSlice, pPrevSlice, iLineHeight))
		{
			// failed.  try next column
			pCol = m_pSectionLayout->getNewColumn();

			int err = pCol->insertBlockSliceAfter(pNewSlice, NULL, iLineHeight);
			UT_ASSERT(err==0);
		}
	}
	else
	{
		/* 
			There is no previous paragraph.  This must be the first paragraph
			in the section.  We need to ask our section for a column.
		*/

		FP_Column *pCol = m_pSectionLayout->getNewColumn();

		int err = pCol->insertBlockSliceAfter(pNewSlice, NULL, iLineHeight);
		UT_ASSERT(err==0);
	}
}

/*
	This routine simply adds the slice to our list of them.  It is also the place
	where we ensure that m_pCurrentSlice is always up to date with the most
	recent slice for flow purposes.
*/
void FL_BlockLayout::_addSlice(FP_BlockSlice* p)
{
	m_vecSlices.addItem(p);
	m_pCurrentSlice = p;
}

void FL_BlockLayout::draw(DG_Graphics* pG)
{
	int countSlices = m_vecSlices.getItemCount();
	for (int i=0; i<countSlices; i++)
	{
		FP_BlockSlice* pSlice = (FP_BlockSlice*) m_vecSlices.getNthItem(i);
		UT_ASSERT(pSlice);

		pSlice->draw(pG);
	}
}

void FL_BlockLayout::clearScreen(DG_Graphics* pG)
{
	int countSlices = m_vecSlices.getItemCount();
	for (int i=0; i<countSlices; i++)
	{
		FP_BlockSlice* pSlice = (FP_BlockSlice*) m_vecSlices.getNthItem(i);
		UT_ASSERT(pSlice);

		pSlice->clearScreen(pG);
	}
}

void FL_BlockLayout::setAlignment(UT_uint32 iAlignCmd)
{
#ifdef PROPERTY
	switch (iAlignCmd)
	{
	case DG_ALIGN_BLOCK_LEFT:
		m_sdh->setProperty("text-align", "left");
		break;
	case DG_ALIGN_BLOCK_RIGHT:
		m_sdh->setProperty("text-align", "right");
		break;
	case DG_ALIGN_BLOCK_CENTER:
		m_sdh->setProperty("text-align", "center");
		break;
	case DG_ALIGN_BLOCK_JUSTIFY:
		m_sdh->setProperty("text-align", "justify");
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}
#endif

	_align();
}

UT_uint32 FL_BlockLayout::getAlignment()
{
#ifdef PROPERTY
	const char* pszAlign = m_sdh->getProperty(lookupProperty("text-align"));
	if (0 == UT_stricmp(pszAlign, "left"))
	{
		return DG_ALIGN_BLOCK_LEFT;
	}
	else if (0 == UT_stricmp(pszAlign, "center"))
	{
		return DG_ALIGN_BLOCK_CENTER;
	}
	else if (0 == UT_stricmp(pszAlign, "right"))
	{
		return DG_ALIGN_BLOCK_RIGHT;
	}
	else if (0 == UT_stricmp(pszAlign, "justify"))
	{
		return DG_ALIGN_BLOCK_JUSTIFY;
	}
	else
#endif
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return 0;
	}
}

void FL_BlockLayout::_align()
{
	int countSlices = m_vecSlices.getItemCount();
	for (int i=0; i<countSlices; i++)
	{
		FP_BlockSlice* pSlice = (FP_BlockSlice*) m_vecSlices.getNthItem(i);
		UT_ASSERT(pSlice);

		pSlice->align();
	}
}

int FL_BlockLayout::reformat()
{
	UT_ASSERT(m_pLayout->getGraphics()->queryProperties(DG_Graphics::DGP_SCREEN));
	UT_DEBUGMSG(("BEGIN reformat block: 0x%x\n", this));

	UT_ASSERT(!m_bFormatting);
	m_bFormatting = UT_TRUE;

	m_pCurrentSlice = (FP_BlockSlice*) m_vecSlices.getNthItem(0);

	m_pBreaker->reLayoutParagraph(this);

	UT_DEBUGMSG(("END reformat block: 0x%x\n", this));

	setNeedsReformat(UT_FALSE);
	
	m_bFormatting = UT_FALSE;
	return 0;
}

void FL_BlockLayout::_purgeLayout(UT_Bool bVisible)
{
	int countSlices = m_vecSlices.getItemCount();
	for (int i=0; i<countSlices; i++)
	{
		FP_BlockSlice* pSlice = (FP_BlockSlice*) m_vecSlices.getNthItem(i);
		UT_ASSERT(pSlice);
		if (bVisible)
			pSlice->clearScreen(m_pLayout->getGraphics());
		pSlice->deleteLines();
		if (bVisible)
			pSlice->remove();

		delete pSlice;
	}

	for (int j=countSlices-1; j>=0; j--)
	{
		m_vecSlices.deleteNthItem(j);
	}
	UT_ASSERT(m_vecSlices.getItemCount() == 0);

	m_pFirstLine = m_pLastLine = NULL;
	while (m_pFirstRun)
	{
		FP_Run* pNext = m_pFirstRun->getNext();
		delete m_pFirstRun;
		m_pFirstRun = pNext;
	}

	if (m_pCharWidths)
	{
		delete m_pCharWidths;
		m_pCharWidths = NULL;
	}
	m_iCharWidthSpace = 0;

	m_pCurrentSlice = NULL;
}

int FL_BlockLayout::format()
{
	UT_DEBUGMSG(("BEGIN Formatting block: 0x%x\n", this));

	UT_ASSERT(!m_bFormatting);
	m_bFormatting = UT_TRUE;

	if (m_vecSlices.getItemCount() > 0)
	{
		/*
		  If this method is called after a format has already been
		  done, we assume that what we should do is a clobber and reformat.
		  The operation will be slow.
		*/
		UT_ASSERT(m_pLayout->getGraphics()->queryProperties(DG_Graphics::DGP_SCREEN));
		
		int countSlices = m_vecSlices.getItemCount();
		for (int i=0; i<countSlices; i++)
		{
			FP_BlockSlice* pSlice = (FP_BlockSlice*) m_vecSlices.getNthItem(i);
			UT_ASSERT(pSlice);
			
			pSlice->clearScreen(m_pLayout->getGraphics());
			pSlice->deleteLines();
			pSlice->verifyColumnFit();
		}

		m_pFirstLine = m_pLastLine = NULL;
		while (m_pFirstRun)
		{
			FP_Run* pNext = m_pFirstRun->getNext();
			delete m_pFirstRun;
			m_pFirstRun = pNext;
		}

		delete m_pCharWidths;
		m_pCharWidths = NULL;
		m_iCharWidthSpace = 0;

		m_pCurrentSlice = (FP_BlockSlice*) m_vecSlices.getNthItem(0);
	}
	
	_allocateCharWidthArray();
	_createRuns();
	_verifyCurrentSlice(); // this is helpful for empty paragraphs

	if (m_pFirstRun)
	{
		// we have content ... break it up
		m_pBreaker->breakParagraph(this);
	}
	else
	{
		// we don't ... construct just enough to keep going
		UT_ASSERT(!m_pFirstLine);

		// the run is empty 
		DG_Graphics* pG = m_pLayout->getGraphics();

		m_pFirstRun = new FP_Run(this, pG, 0, 0);
		m_pFirstRun->calcWidths(m_pCharWidths);

		// the line just contains the empty run
		UT_uint32 iGuessLineHeight = m_pFirstRun->getHeight();

		UT_uint32 iMaxLineWidth = requestLineSpace(iGuessLineHeight);
		UT_ASSERT(iMaxLineWidth > 0);

		FP_Line*	pLine = new FP_Line(iMaxLineWidth);
		pLine->addRun(m_pFirstRun);
		addLine(pLine);
	}

	int countSlices = m_vecSlices.getItemCount();
	for (int i=0; i<countSlices; i++)
	{
		FP_BlockSlice* pSlice = (FP_BlockSlice*) m_vecSlices.getNthItem(i);
		UT_ASSERT(pSlice);

		pSlice->returnExtraSpace();
	}

	_align();

	m_bFormatting = UT_FALSE;

	setNeedsReformat(UT_FALSE);
	
	UT_DEBUGMSG(("END Formatting block: 0x%x\n", this));

	return 0;	// TODO return code
}

/*
	requestLineSpace is called by the FB_LineBreaker when it needs to ask for space in which
	to put a new line of content.  The return value is the width of the line.  LineBreakers
	do not know which slice a line is stored in.  That is managed entirely by the FL_BlockLayout
	itself.
*/
int	FL_BlockLayout::requestLineSpace(int iHeight)
{
	_verifyCurrentSlice();

	// TODO assert that iHeight is reasonable
	int wid = m_pCurrentSlice->requestLineSpace(iHeight);

	if (wid == 0)
	{
		UT_sint32 ndx = m_vecSlices.findItem(m_pCurrentSlice);
		UT_ASSERT(ndx >= 0);

		FP_BlockSlice* pNextSlice;
		if (m_vecSlices.getItemCount() > (UT_uint32)(ndx+1))
		{
			pNextSlice = (FP_BlockSlice*) m_vecSlices.getNthItem(ndx+1);
		}
		else
		{
			pNextSlice = NULL;
		}
		
		if (pNextSlice)
		{
			m_pCurrentSlice = pNextSlice;
			
			wid = m_pCurrentSlice->requestLineSpace(iHeight);
			
			// if wid were 0 here, then iHeight is larger than the height of the col
		}
		else
		{
			UT_DEBUGMSG(("No more space.  Need a new slice.  m_pCurrentSlice=0x%x and its height is %d\n",
						 m_pCurrentSlice,
						 m_pCurrentSlice->getHeight()));
			UT_DEBUGMSG(("A requestLineSpace: m_pCurrentSlice=0x%x, col=0x%x\n", m_pCurrentSlice, m_pCurrentSlice->getColumn()));

			_createNewSlice();

			UT_DEBUGMSG(("B requestLineSpace: m_pCurrentSlice=0x%x, col=0x%x\n", m_pCurrentSlice, m_pCurrentSlice->getColumn()));
			wid = m_pCurrentSlice->requestLineSpace(iHeight);
			// if wid were 0 here, then iHeight is larger than the height of the col
		}
	}

	return wid;
}

FP_Run* FL_BlockLayout::getFirstRun()
{
	return m_pFirstRun;
}

int	FL_BlockLayout::addLine(FP_Line* pLine)
{
	UT_ASSERT(pLine);
	m_pCurrentSlice->addLine(pLine);

	pLine->setNext(NULL);
	
	if (m_pLastLine)
	{
		UT_ASSERT(m_pFirstLine);
		UT_ASSERT(!m_pLastLine->getNext());

		pLine->setPrev(m_pLastLine);
		m_pLastLine->setNext(pLine);
		m_pLastLine = pLine;
	}
	else
	{
		UT_ASSERT(!m_pFirstLine);
		m_pFirstLine = pLine;
		m_pLastLine = m_pFirstLine;
		pLine->setPrev(NULL);
	}

	return 0; // TODO return code
}

void FL_BlockLayout::_createRuns()
{
#ifdef BUFFER	// _createRuns
  UT_uint32 posCur = getPosition();
  UT_uint32 posBase = posCur;
  UT_ASSERT(m_sdh->getType() & DG_MT_BLOCK);
  UT_ASSERT(!(m_sdh->getType() & DG_MT_END));

  UT_Bool bDone = UT_FALSE;
  UT_uint32 posStartRun;
  UT_uint32 lenRun = 0;
  FP_Run* pLast = NULL;
  DG_Graphics* pG = m_pLayout->getGraphics();

  while (!bDone)
  {
      UT_UCSChar ch;
      DG_DocMarkerId dmid;
      DG_DocMarker* pMarker = NULL;

      UT_Bool bMarker = (m_pBuffer->getOneItem(posCur, &ch, &dmid) == DG_DBPI_MARKER);
      if (bMarker)
      {
	      pMarker = m_pBuffer->fetchDocMarker(dmid);
	      DG_DocMarkerType dmt = pMarker->getType();
	      if ((dmt & DG_MT_BLOCK) && (dmt & DG_MT_END))
	      {
	          // we found the end of this block
	          bDone = UT_TRUE;
	      }

	      if (lenRun > 0)
	      {
	          FP_Run* pRun = new FP_Run(this, pG, posStartRun - posBase, lenRun);
			  pRun->calcWidths(m_pCharWidths);
	      
	          if (!m_pFirstRun)
		      {
				m_pFirstRun = pRun;
			  }
			  
			  pRun->setPrev(pLast);
			  if (pLast)
			  {
				  pLast->setNext(pRun);
			  }
			  pLast = pRun;
	      
			  lenRun = 0;
		  }
      }
      else
      {
		  if (lenRun > 0)
		  {
			  lenRun++;
		  }
		  else
		  {
			  posStartRun = posCur;
			  lenRun = 1;
		  }
      }
      m_pBuffer->inc(posCur);
  }
#endif BUFFER
}

void FL_BlockLayout::setNeedsReformat(UT_Bool b)
{
	m_bNeedsReformat = b;
}

UT_Bool FL_BlockLayout::needsReformat()
{
	return m_bNeedsReformat;
}

const char*	FL_BlockLayout::getProperty(const PP_Property* pProp)
{
#ifdef PROPERTY
	return m_sdh->getProperty(pProp);
#endif
	return NULL;
}

UT_uint32 FL_BlockLayout::getPosition() const
{
#ifdef BUFFER	// getPosition
	return m_pBuffer->getMarkerPosition(m_sdh);
#endif
	UT_ASSERT(UT_TODO);
	return 0;
}

UT_uint16* FL_BlockLayout::getCharWidthArray() const
{
	return m_pCharWidths;
}

void FL_BlockLayout::_growCharWidthArray() // TODO return an error code
{
	UT_ASSERT(m_pCharWidths);
	
	UT_uint16* pCharWidths = new UT_uint16[m_iCharWidthSpace + EXTRA_CHARWIDTH_SPACE];
	// TODO check for failure and return outofmem if needed.
	
	memcpy(pCharWidths, m_pCharWidths, m_iCharWidthSpace * sizeof(UT_UCSChar));
	m_iCharWidthSpace += EXTRA_CHARWIDTH_SPACE;
	delete m_pCharWidths;
	m_pCharWidths = pCharWidths;
}

void FL_BlockLayout::_allocateCharWidthArray()   // TODO return an error code
{
//  UT_ASSERT(m_iCharWidthSize == (m_pBuffer->getMarkerPosition(m_sdh) - m_pBuffer->getMarkerPosition(m_pEndBlockMarker) + 1));
	if (m_pCharWidths)
    {
		delete m_pCharWidths;
    }

	m_iCharWidthSpace = m_iCharWidthSize + EXTRA_CHARWIDTH_SPACE;

	m_pCharWidths = new UT_uint16[m_iCharWidthSpace];
	for (UT_uint32 i=0; i<m_iCharWidthSpace; i++)
	{
		m_pCharWidths[i] = 0;
	}
	// TODO check for failure and return outofmem if needed.
}

#ifdef BUFFER	// fetchPointers
UT_Bool FL_BlockLayout::fetchPointers(UT_uint32 position, UT_uint32 count,
									const UT_uint16** pp1, UT_uint32* pLen1,
									const UT_uint16** pp2, UT_uint32* pLen2) const
{
  UT_uint32 iBaseAddress = getPosition();

  return m_pBuffer->fetchPointers(position + iBaseAddress, count, pp1, pLen1, pp2, pLen2);
}
#endif

UT_Bool FL_BlockLayout::_insertInCharWidthsArray(UT_uint32 iOffset, UT_uint32 count)
{
	/*
	  The BlockLayout stores an array of character widths which is shared
	  by all of its runs.  Runs know their first character, expressed
	  as an offset from the base of the block, and the number of characters
	  in that run.  When an insert occurs, we need to update the charWidths
	  array, and we need to update the offsets on some/all of the runs.
	  Unfortunately, the char widths array is contiguous, not a gap
	  buffer.  Inserting a char involves a memmove.  However, this memmove
	  is likely to be much smaller than it would have been in the case of
	  a document.

	  We can try to find a better solution for this later.  For now, this is
	  tolerable.
	*/

	if ((m_iCharWidthSize + count) >= m_iCharWidthSpace)
	{
		_growCharWidthArray();
		// TODO check result and do outofmem code
	}
	
	UT_ASSERT((m_iCharWidthSize + count) <= m_iCharWidthSpace);
	memmove(m_pCharWidths + iOffset + count, m_pCharWidths + iOffset, (m_iCharWidthSize - iOffset + 1)*sizeof(UT_UCSChar));
	m_iCharWidthSize += count;

	return UT_TRUE;
}

FP_Run* FL_BlockLayout::findPointCoords(PT_DocPosition iPos, UT_Bool bRight, UT_uint32& x, UT_uint32& y, UT_uint32& height)
{
	// find the run which has this position inside it.
	UT_ASSERT(iPos > getPosition());
	UT_uint32 iRelOffset = iPos - getPosition();

	FP_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		UT_uint32 iWhere = pRun->containsOffset(iRelOffset);
		if (FP_RUN_INSIDE == iWhere)
		{
			pRun->findPointCoords(iRelOffset, x, y, height);
			return pRun;
		}
		else if (!bRight && (FP_RUN_JUSTAFTER == iWhere))
		{
			pRun->findPointCoords(iRelOffset, x, y, height);
			return pRun;
		}
		
		pRun = pRun->getNext();
	}

	pRun = m_pFirstRun;
	while (pRun)
	{
		UT_uint32 iWhere = pRun->containsOffset(iRelOffset);
		if ((FP_RUN_JUSTAFTER == iWhere))
		{
			pRun->findPointCoords(iRelOffset, x, y, height);
			return pRun;
		}

		if (!pRun->getNext())
		{
			// this is the last run, we're not going to get another chance, so try harder
			if (iRelOffset > (pRun->getBlockOffset() + pRun->getLength()))
			{
				pRun->findPointCoords(iRelOffset, x, y, height);
				return pRun;
			}
		}
		
		pRun = pRun->getNext();
	}

	if (iRelOffset < m_pFirstRun->getBlockOffset())
	{
		m_pFirstRun->findPointCoords(iRelOffset, x, y, height);
		return m_pFirstRun;
	}
	
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

FP_Line* FL_BlockLayout::getLastLine()
{
	FP_BlockSlice* pLastSlice = (FP_BlockSlice*) m_vecSlices.getNthItem(m_vecSlices.getItemCount()-1);
	FP_Line* pLastLine = pLastSlice->getNthLine(pLastSlice->countLines()-1);
	return pLastLine;
}

FP_Line* FL_BlockLayout::getFirstLine()
{
	FP_BlockSlice* pSlice = (FP_BlockSlice*) m_vecSlices.getNthItem(0);
	FP_Line* pLine = pSlice->getNthLine(0);
	return pLine;
}

FP_Line* FL_BlockLayout::findPrevLineInDocument(FP_Line* pLine)
{
	int count = m_vecSlices.getItemCount();
	FP_Line* pPrev = NULL;
	for (int i=0; i<count; i++)
	{
		FP_BlockSlice* pBS = (FP_BlockSlice*) m_vecSlices.getNthItem(i);
		int count2 = pBS->countLines();
		for (int j=0; j<count2; j++)
		{
			FP_Line* pL = pBS->getNthLine(j);
			if (pL == pLine)
			{
				if (pPrev)
				{
					return pPrev;
				}
				else
				{
					if (m_pPrev)
					{
						return m_pPrev->getLastLine();
					}
					else
					{
						return NULL;
					}
				}
			}
			pPrev = pL;
		}
	}
	
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

FP_Line* FL_BlockLayout::findNextLineInDocument(FP_Line* pLine)
{
	if (pLine->getNext())
	{
		return pLine->getNext();
	}
	
	int count = m_vecSlices.getItemCount();
	for (int i=0; i<count; i++)
	{
		FP_BlockSlice* pBS = (FP_BlockSlice*) m_vecSlices.getNthItem(i);
		int count2 = pBS->countLines();
		for (int j=0; j<count2; j++)
		{
			FP_Line* pL = pBS->getNthLine(j);
			if (pL == pLine)
			{
				// found the line.
				UT_ASSERT(!((j+1) < count2));	// if this were true, it would have been caught above
				
				if ((i+1) < count)
				{
					// grab the first line from the next blockslice
					pBS = (FP_BlockSlice*) m_vecSlices.getNthItem(i+1);
					return pBS->getNthLine(0);
				}
				else if (m_pNext)
				{
					// grab the first line from the next block
					return m_pNext->getFirstLine();
				}
				else
				{
					// there is no next line in the document
					return NULL;
				}
			}
		}
	}
	
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

FL_BlockLayout* FL_BlockLayout::getNext() const
{
	return m_pNext;
}

FL_BlockLayout* FL_BlockLayout::getPrev() const
{
	return m_pPrev;
}

FP_BlockSlice* FL_BlockLayout::getFirstSlice()
{
	return (FP_BlockSlice*) m_vecSlices.getNthItem(0);
}

FP_BlockSlice* FL_BlockLayout::getLastSlice()
{
	return (FP_BlockSlice*) m_vecSlices.getNthItem(m_vecSlices.getItemCount()-1);
}

void FL_BlockLayout::dump()
{
	int count = m_vecSlices.getItemCount();
	UT_DEBUGMSG(("FL_BlockLayout 0x%x is from element 0x%x and contains %d slices.\n", this, m_sdh, count));

	for (int i=0; i<count; i++)
	{
		FP_BlockSlice* p = (FP_BlockSlice*) m_vecSlices.getNthItem(i);

		UT_DEBUGMSG(("FL_BlockLayout::dump(0x%x) - FP_BlockSlice 0x%x, height=%d, in column 0x%x\n", this, p, p->getHeight(), p->getColumn()));
	}

	UT_DEBUGMSG(("FL_BlockLayout 0x%x contains the following runs:\n",this));

	FP_Run * pRun = getFirstRun();
	while (pRun)
	{
		pRun->dumpRun();
		pRun = pRun->getNext();
	}
}

// *************************************************************************

#ifdef BUFFER
void FL_BlockLayout::mergeWithNextBlock()
{
	UT_ASSERT(m_pLayout->getGraphics()->queryProperties(DG_Graphics::DGP_SCREEN));
		
	FL_BlockLayout* pNext = getNext();
	UT_ASSERT(pNext);

	_purgeLayout(UT_TRUE);
	pNext->_purgeLayout(UT_TRUE);
	
	UT_uint32 iPosLeftEndMarker = m_pBuffer->getMarkerPosition(m_pEndBlockMarker);
	m_pBuffer->moveAbsolute(iPosLeftEndMarker);
	DG_DB_PosInfo res;
	
	res = m_pBuffer->remove(UT_TRUE);
	UT_ASSERT(res == DG_DBPI_MARKER);

	UT_uint32 iPosRightBeginMarker = m_pBuffer->getMarkerPosition(pNext->m_sdh);
	m_pBuffer->moveAbsolute(iPosLeftEndMarker);
	res = m_pBuffer->remove(UT_TRUE);
	UT_ASSERT(res == DG_DBPI_MARKER);

	m_pEndBlockMarker = pNext->m_pEndBlockMarker;
	m_pEndBlockMarker->setParent(m_sdh->getMarkerId());
	
	m_pNext = pNext->getNext();
	m_pNext->m_pPrev = this;

	delete pNext;
}

DG_DocBuffer* FL_BlockLayout::getBuffer() const
{
  return m_pBuffer;
}

UT_uint32 FL_BlockLayout::getEndAddress() const
{
	return m_pBuffer->getMarkerPosition(m_pEndBlockMarker);
}

UT_uint32 FL_BlockLayout::_getLastChar()
{
	UT_uint32 iPosLastChar;
	
	UT_uint32 posCur = m_pBuffer->getMarkerPosition(m_sdh);
	for (;;)
	{
	    UT_UCSChar ch;
	    DG_DocMarkerId dmid;
	    DG_DocMarker* pMarker = NULL;
	    
	    UT_Bool bMarker = (m_pBuffer->getOneItem(posCur, &ch, &dmid) == DG_DBPI_MARKER);
	    if (bMarker)
		{
			pMarker = m_pBuffer->fetchDocMarker(dmid);
			DG_DocMarkerType dmt = pMarker->getType();
			if ((dmt & DG_MT_BLOCK) && (dmt & DG_MT_END))
			{
				// we found the end of this block
				break;
			}
		}
		else
		{
			iPosLastChar = posCur;
		}
		
	    m_pBuffer->inc(posCur);
	}

	return iPosLastChar;
}

UT_Bool FL_BlockLayout::cmdCharDelete(UT_Bool bForward, UT_uint32 iCount)
{
	UT_ASSERT(iCount > 0);

	/*
	  This code is funky, because we have to be really careful about
	  the presence of both markers and characters in the buffer.
	*/
	UT_uint32 iBlockBase = getPosition();
	UT_uint32 iOldPoint = m_pBuffer->getPoint();
	UT_ASSERT(iOldPoint > iBlockBase);

	if (!bForward && (m_pBuffer->getDataCount(iBlockBase, iOldPoint) == 0))
	{
		UT_ASSERT(iCount == 1);	// we don't know how to cope with this yet
		// we are deleting the paragraph break, merging two blocks
		return UT_TRUE;
	}
	else if (bForward && (iCount==1) && (iOldPoint == _getLastChar()+1))
	{
		UT_ASSERT(iCount == 1);	// we don't know how to cope with this yet
		// we are deleting the paragraph break, merging two blocks
		mergeWithNextBlock();
		format();
		draw(m_pLayout->getGraphics());
	}
	else
	{
		m_pBuffer->charMotion(bForward, iCount);
		UT_uint32 iNewPoint = m_pBuffer->getPoint();
		bForward = !bForward;
	
		UT_uint32 iNumDeleteUnits;
		if (iNewPoint > iOldPoint)
		{
			iNumDeleteUnits = iNewPoint - iOldPoint;
		}
		else
		{
			iNumDeleteUnits = iOldPoint - iNewPoint;
		}

		/*
		  This deletes the characters, but not the markers.
		*/
		m_pBuffer->charDelete(bForward, iCount);
	
		iNewPoint -= iBlockBase;
		memmove(m_pCharWidths + iNewPoint,
				m_pCharWidths + iNewPoint + iCount,
				(m_iCharWidthSize - (iNewPoint + iCount) + 1) * sizeof(UT_UCSChar));
		m_iCharWidthSize -= iNumDeleteUnits;

		FP_Run* pRun = m_pFirstRun;
		while (pRun)
		{
			FP_Run* pNext = pRun->getNext();
			if (pRun->deleteChars(iNewPoint, iNumDeleteUnits, iCount))
			{
				// a run sets its length to zero when everything within it has been deleted.
				if (pRun->getLength() == 0)
				{
					// delete the run
					pRun->getLine()->removeRun(pRun);
					
					if (pRun->getPrev())
					{
						pRun->getPrev()->setNext(pRun->getNext());
					}
					if (pRun->getNext())
					{
						pRun->getNext()->setPrev(pRun->getPrev());
					}
					pNext = pRun->getNext();
					delete pRun;

					// now, make sure the ins pt is on a character in a run
					UT_UCSChar ch;
					DG_DocMarkerId dmid;
					UT_Bool bBackward = UT_TRUE;
					
					while (DG_DBPI_DATA != m_pBuffer->getOneItem(m_pBuffer->getPoint(), &ch, &dmid))
					{
						if (bBackward)
						{
							m_pBuffer->move(UT_FALSE);
							if (m_pBuffer->getPoint() == 0)
							{
								bBackward = UT_FALSE;
							}
						}
						else
						{
							m_pBuffer->move(UT_TRUE);
						}
					}
					
					m_pBuffer->move(bBackward);
				}
				else
				{
					pRun->calcWidths(m_pCharWidths);
				}
			}
			pRun = pNext;
		}

		reformat();
	}

	return UT_FALSE;
}

UT_Bool FL_BlockLayout::insertData(UT_UCSChar * text, UT_uint32 count)
{
	UT_ASSERT(m_pLayout->getGraphics()->queryProperties(DG_Graphics::DGP_SCREEN));
	UT_ASSERT(count > 0);
	UT_ASSERT(text);

	UT_uint32 iBlockBase = getPosition();
	UT_uint32 iPointAbsolute = m_pBuffer->getPoint();
	UT_ASSERT(iPointAbsolute > iBlockBase);
	
	UT_uint32 iOffset = iPointAbsolute - iBlockBase;

	UT_Bool bResult = m_pBuffer->insertData(text, count);
	UT_Bool bResult2 = _insertInCharWidthsArray(iOffset,count);
	
	/*
	  Having fixed the char widths array, we need to update all the run offsets.
	  We call each run individually to update its offsets.  It returns true if
	  its size changed, thus requiring us to remeasure it.
	*/
	
	FP_Run* pRun = m_pFirstRun;

	/*
	  Save away the list of runs that are going to possibly
	  be reformatted.  After the reformat, just redraw thses
	  runs
	*/
	  
	while (pRun)
	{
		if (pRun->insertData(iOffset, count))
		{
			pRun->calcWidths(m_pCharWidths);
		}
		
		pRun = pRun->getNext();
	}

	reformat();

	return bResult;
}

/*
  Divides the block into two slices, at the insertion point.
  This is used when the user presses the return key.
*/
void FL_BlockLayout::insertParagraphBreak()
{
	UT_ASSERT(m_pLayout->getGraphics()->queryProperties(DG_Graphics::DGP_SCREEN));
	UT_uint32 iPoint = m_pBuffer->getPoint();

	DG_DocMarkerId dmidEndLeft;
	DG_DocMarkerId dmidBeginRight;
	DG_DocMarkerId dmidBeginLeft;
	DG_DocMarkerId dmidEndRight;
	
	DG_DocMarker* pMarkerBeginLeft;
	DG_DocMarker* pMarkerEndRight;

	pMarkerBeginLeft = m_sdh;
	dmidBeginLeft = pMarkerBeginLeft->getMarkerId();
	
	pMarkerEndRight = m_pEndBlockMarker;
	dmidEndRight = pMarkerEndRight->getMarkerId();
	
	dmidEndLeft = m_pBuffer->endElement("P",DG_MT_BLOCK,dmidBeginLeft);
	dmidBeginRight = m_pBuffer->startElement("P",DG_MT_BLOCK, 0);	// TODO parent id

	pMarkerEndRight->setParent(dmidBeginRight);
	m_pEndBlockMarker = m_pBuffer->fetchDocMarker(dmidEndLeft);
	
	DG_DocMarker* pMarkerBeginRight = m_pBuffer->fetchDocMarker(dmidBeginRight);
	FL_BlockLayout* pBLRight = new FL_BlockLayout(pMarkerBeginRight,
												  m_pBreaker,
												  this,
												  m_pSectionLayout);
	UT_ASSERT(this->getNext() == pBLRight);

	this->format();
	this->draw(m_pLayout->getGraphics());

	pBLRight->format();
	pBLRight->draw(m_pLayout->getGraphics());

	// this looks pretty darn fragile, but it's working
	m_pBuffer->moveAbsolute(pBLRight->getPosition() + m_pBuffer->getMarkerSize());

#if 0
	/*
	  TODO - the following doesn't work quite right when the para is empty
	*/
	UT_UCSChar ch;
	DG_DocMarkerId dmid;
	while (DG_DBPI_DATA != m_pBuffer->getOneItem(m_pBuffer->getPoint(), &ch, &dmid))
	{
		m_pBuffer->move(UT_TRUE);
	}
#endif
	
	/*
	  TODO walk back through the paragraph.  For each marker which is
	  open, at the insertion point, insert a close marker.
	  Then, insert a close marker for the paragraph.
	  Finally, insert an open marker for the new paragraph.
	  Fix the close marker for the old paragraph, so that its
	  marker structure is correct.  Reopen all the markers we closed.
	  Erase the old paragraph after to the ins point.  Create a new
	  BlockLayout object for the new para, inserting it after
	  this one and before the next one in the list.  Format the new
	  block.  Delete the moved runs from the old block.  Delete any
	  line which included only those runs.  Delete any blockslice
	  which included only those lines.
	*/
}

UT_Bool FL_BlockLayout::insertInlineMarker(const XML_Char * szName,
										   UT_Bool bStart,
										   DG_DocMarkerId dmidParent,
										   UT_Bool bCreateEmptyRun,
										   DG_DocMarkerId * pdmidNew)
{
	UT_ASSERT(m_pLayout->getGraphics()->queryProperties(DG_Graphics::DGP_SCREEN));
	
	// insert an in-line marker (a format change, but not a block)
	// at the current insertion point.
	// return UT_TRUE if successful.

	UT_uint32 newMarkerOffset = m_pBuffer->getPoint() - getPosition();
	UT_uint32 markerSize = m_pBuffer->getMarkerSize();
	UT_Bool bResult = _insertInCharWidthsArray(newMarkerOffset,markerSize);
	
	// since runs have buffer offsets which are block-relative
	// and runs do not span in-line markers, we must adjust the
	// runs in this block.  enumerate thru them and tell them
	// that we are going to insert a marker at the current point.
	
	FP_Run * pRun = getFirstRun();
	while (pRun)
	{
		if (bCreateEmptyRun && (pRun->getBlockOffset()==newMarkerOffset))
		{
			// if this run starts at the location of the (proposed)
			// new marker and they want an empty run to preceed the
			// marker, we split the current -- giving the original
			// length zero and the new one the entire contents.
			// we then step over the current one, so that we don't
			// adjust the offset.
			pRun->split(newMarkerOffset);
			pRun->lookupProperties();
			pRun->getLine()->draw(m_pLayout->getGraphics());
			pRun = pRun->getNext();
		}
		pRun->insertInlineMarker(newMarkerOffset,markerSize);
		pRun = pRun->getNext();
	}

	// with the runs updated, we are free to insert a marker
	// (now between two runs).
	
	DG_DocMarkerId dmidNew;
	if (bStart)
		dmidNew = m_pBuffer->startElement(szName,DG_MT_OTHER,dmidParent);
	else
		dmidNew = m_pBuffer->endElement(szName,DG_MT_OTHER,dmidParent);
	if (!dmidNew)
		return UT_FALSE;

	*pdmidNew = dmidNew;
	return UT_TRUE;
}
#endif BUFFER
