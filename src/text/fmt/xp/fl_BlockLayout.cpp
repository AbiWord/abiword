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
#include "gr_Graphics.h"
#include "sp_spell.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_dllist.h"

fl_BlockLayout::fl_BlockLayout(PL_StruxDocHandle sdh,
							   fb_LineBreaker* pBreaker,
							   fl_BlockLayout* pPrev,
							   fl_SectionLayout* pSectionLayout)
	: fl_Layout(PTX_Block, sdh), m_gbCharWidths(256)
{
	m_pSectionLayout = pSectionLayout;
	m_pBreaker = pBreaker;
	m_pFirstRun = NULL;
	m_pFirstLine = NULL;
	m_pLastLine = NULL;
	m_pCurrentSlice = NULL;
	m_bFormatting = UT_FALSE;
	m_bNeedsReformat = UT_FALSE;
	
	m_pLayout = m_pSectionLayout->getLayout();
	m_pDoc = m_pLayout->getDocument();

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

fl_BlockLayout::~fl_BlockLayout()
{
	_purgeLayout(UT_FALSE);
}

FL_DocLayout* fl_BlockLayout::getLayout()
{
	return m_pLayout;
}

fl_SectionLayout * fl_BlockLayout::getSectionLayout()
{
	return m_pSectionLayout;
}

/*
	_createNewSlice is only called when a new slice is needed to continue a flow
	which is already in progress.  This routine is not used to create the first
	slice of the block.
*/
void fl_BlockLayout::_createNewSlice()
{
	UT_ASSERT(m_pCurrentSlice);

	fp_Column* pCol = m_pCurrentSlice->getColumn();
	UT_ASSERT(pCol);

	pCol = pCol->getNext();
	if (!pCol)
	{
		/*
			This happens when we have hit the last column in the fl_SectionLayout
		*/
		pCol = m_pSectionLayout->getNewColumn();
	}

	// Could it be NULL due to outofmem?  Perhaps we shouldn't assert - TODO

	fp_BlockSlice* pNewSlice = new fp_BlockSlice(this);
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
void fl_BlockLayout::_verifyCurrentSlice()
{
	if (m_pCurrentSlice)
	{
		return;
	}

	// There is no current slice.  We need to start one.

	fp_BlockSlice* pNewSlice = new fp_BlockSlice(this);
	_addSlice(pNewSlice);

	UT_DEBUGMSG(("_verifyCurrentSlice: fl_BlockLayout=0x%x, m_pPrev=0x%x\n", this, m_pPrev));

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
		fp_BlockSlice* pPrevSlice = m_pPrev->getLastSlice();
		UT_ASSERT(pPrevSlice);
		UT_DEBUGMSG(("_verifyCurrentSlice:  pPrevSlice=0x%x\n", pPrevSlice));

		fp_Column* pCol = pPrevSlice->getColumn();
		UT_ASSERT(pCol);
		UT_DEBUGMSG(("_verifyCurrentSlice:  pCol=0x%x\n", pCol));

		if (pCol->insertBlockSliceAfter(pNewSlice, pPrevSlice, iLineHeight))
		{
			// failed.  try next column
			pCol = pCol->getNext();

			// no more columns exist, so need a new one
			if (!pCol)
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

		fp_Column *pCol = m_pSectionLayout->getFirstColumn();
		if (!pCol)
		{
			pCol = m_pSectionLayout->getNewColumn();
		}

		int err = pCol->insertBlockSliceAfter(pNewSlice, NULL, iLineHeight);
		UT_ASSERT(err==0);
	}
}

/*
	This routine simply adds the slice to our list of them.  It is also the place
	where we ensure that m_pCurrentSlice is always up to date with the most
	recent slice for flow purposes.
*/
void fl_BlockLayout::_addSlice(fp_BlockSlice* p)
{
	m_vecSlices.addItem(p);
	m_pCurrentSlice = p;
}

void fl_BlockLayout::fixColumns(void)
{
	int countSlices = m_vecSlices.getItemCount();
	for (int i=0; i<countSlices; i++)
	{
		fp_BlockSlice* pSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(i);
		UT_ASSERT(pSlice && pSlice->getColumn());

		pSlice->getColumn()->updateSlicePositions();
	}
}

void fl_BlockLayout::draw(DG_Graphics* pG)
{
	int countSlices = m_vecSlices.getItemCount();
	for (int i=0; i<countSlices; i++)
	{
		fp_BlockSlice* pSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(i);
		UT_ASSERT(pSlice);

		pSlice->draw(pG);
	}
}

void fl_BlockLayout::clearScreen(DG_Graphics* pG)
{
	int countSlices = m_vecSlices.getItemCount();
	for (int i=0; i<countSlices; i++)
	{
		fp_BlockSlice* pSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(i);
		UT_ASSERT(pSlice);

		pSlice->clearScreen(pG);
	}
}

void fl_BlockLayout::setAlignment(UT_uint32 /*iAlignCmd*/)
{
#ifdef PROPERTY
	switch (iAlignCmd)
	{
	case FL_ALIGN_BLOCK_LEFT:
		m_sdh->setProperty("text-align", "left");
		break;
	case FL_ALIGN_BLOCK_RIGHT:
		m_sdh->setProperty("text-align", "right");
		break;
	case FL_ALIGN_BLOCK_CENTER:
		m_sdh->setProperty("text-align", "center");
		break;
	case FL_ALIGN_BLOCK_JUSTIFY:
		m_sdh->setProperty("text-align", "justify");
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}
#endif

	align();
}

UT_uint32 fl_BlockLayout::getAlignment()
{
	const char* pszAlign = getProperty("text-align");

	if (0 == UT_stricmp(pszAlign, "left"))
	{
		return FL_ALIGN_BLOCK_LEFT;
	}
	else if (0 == UT_stricmp(pszAlign, "center"))
	{
		return FL_ALIGN_BLOCK_CENTER;
	}
	else if (0 == UT_stricmp(pszAlign, "right"))
	{
		return FL_ALIGN_BLOCK_RIGHT;
	}
	else if (0 == UT_stricmp(pszAlign, "justify"))
	{
		return FL_ALIGN_BLOCK_JUSTIFY;
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return 0;
	}
}

void fl_BlockLayout::align()
{
	int countSlices = m_vecSlices.getItemCount();
	for (int i=0; i<countSlices; i++)
	{
		fp_BlockSlice* pSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(i);
		UT_ASSERT(pSlice);

		pSlice->align();
	}
}

void fl_BlockLayout::_purgeLayout(UT_Bool bVisible)
{
	int countSlices = m_vecSlices.getItemCount();
	for (int i=0; i<countSlices; i++)
	{
		fp_BlockSlice* pSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(i);
		UT_ASSERT(pSlice);

		if (bVisible)
		{
			UT_ASSERT(m_pLayout->getGraphics()->queryProperties(DG_Graphics::DGP_SCREEN));
			pSlice->clearScreen(m_pLayout->getGraphics());
		}
		pSlice->deleteLines();

		if (bVisible)
		{
			pSlice->remove();
		}

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
		fp_Run* pNext = m_pFirstRun->getNext();
		delete m_pFirstRun;
		m_pFirstRun = pNext;
	}

	m_pCurrentSlice = NULL;
}

void fl_BlockLayout::_removeLine(fp_Line* pLine)
{
	if (m_pFirstLine == pLine)
	{
		m_pFirstLine = m_pFirstLine->getNext();
	}
			
	if (m_pLastLine == pLine)
	{
		m_pLastLine = m_pLastLine->getPrev();
	}
			
	pLine->remove();
}

void fl_BlockLayout::_removeAllEmptyLines(void)
{
	fp_Line* pLine;

	pLine = m_pFirstLine;
	while (pLine)
	{
		if (pLine->countRuns() == 0)
		{
			_removeLine(pLine);
			pLine = m_pFirstLine;
		}
		else
		{
			pLine = pLine->getNext();
		}
	}
}

UT_Bool fl_BlockLayout::truncateLayout(fp_Run* pTruncRun)
{
	// special case, nothing to do
	if (!pTruncRun)
		return UT_TRUE;

	if (pTruncRun->getBlock() != this)
	{
		// be safe
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return UT_FALSE;
	}

	if (m_pFirstRun == pTruncRun)
	{
		m_pFirstRun = NULL;
	}

	fp_Run* pRun;
	
	// remove runs from screen
	pRun = pTruncRun;
	while (pRun)
	{
		pRun->clearScreen();
		pRun = pRun->getNext();
	}

	// remove runs from lines
	pRun = pTruncRun;
	while (pRun)
	{
		fp_Line* pLine = pRun->getLine();
		UT_ASSERT(pLine);

		pLine->removeRun(pRun);

		pRun = pRun->getNext();
	}

#if 0	
	// remove empty lines 
	while (m_pLastLine && (m_pLastLine != m_pFirstLine))
	{
		if (m_pLastLine->countRuns())
			break;

		fp_Line* pLine = m_pLastLine;
		m_pLastLine = m_pLastLine->getPrev();

		pLine->remove();
	}
#endif

	_removeAllEmptyLines();
	
	// remove any empty slices, and reclaim space from the rest
	int countSlices = m_vecSlices.getItemCount();
	for (int i=countSlices-1; i>=0; i--)
	{
		fp_BlockSlice* pSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(i);
		UT_ASSERT(pSlice);

		if ((pSlice->countLines()) || (i == 0))
		{
			pSlice->returnExtraSpace();
		}
		else
		{
			pSlice->remove();
			delete pSlice;
			m_vecSlices.deleteNthItem(i);
		}
	}

	UT_ASSERT(m_vecSlices.getItemCount() > 0);

	return UT_TRUE;
}

/*
  Notes on reformatting.

  Editing operations frequently cause the need to reformat a block.
  For the purpose of this note, 'reformat' means formatting something
  after it has already been formatted.
  
  The following things can happen when a block is reformatted:

  1.  Erase the screen space used by all the current slices.

      This one is serious.  It causes the entire block to flicker.

  2.  Purge the lines.

      Also serious.  This means that all the lines need to be rebuilt.
	  If the lines are purged, you must call breakParagraph.  Otherwise,
	  you must call relayoutParagraph.

  3.  Clobber the slices.

      Quite serious.  This means that the location of the first slice
	  for this block will be calculated by looking at the previous block.
	  In some cases, this is necessary.  For example, if a block is currently
	  beginning in column D, but there is now room for it to begin in
	  column C (the predecessor to D), then the slices should be purged
	  so that the block will be reformatted to place it in the previous
	  column.

  4.  Clobber runs.

      The most serious.  This means that nearly everything is tossed out,
	  and the entire layout is being constructed from scratch.
*/

int fl_BlockLayout::minor_reformat()
{
	UT_ASSERT(m_pLayout->getGraphics()->queryProperties(DG_Graphics::DGP_SCREEN));
	UT_DEBUGMSG(("BEGIN reformat block: 0x%x\n", this));

	UT_ASSERT(!m_bFormatting);
	m_bFormatting = UT_TRUE;

	m_pCurrentSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(0);

	if (m_pFirstRun)
	{
		m_pBreaker->reLayoutParagraph(this);

		/*
		  A delete could mean that there are empty lines
		  in the block.  If so, we need to remove them.
		*/
		while (m_pLastLine && (m_pLastLine->isEmpty()))
		{
			fp_Line* pLine = m_pLastLine;
		
			m_pLastLine = pLine->getPrev();
			pLine->remove();

			// note that we do NOT delete pLine here.  It is deleted elsewhere.
		}
	
		int countSlices = m_vecSlices.getItemCount();
		for (int i=0; i<countSlices; i++)
		{
			fp_BlockSlice* pSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(i);
			UT_ASSERT(pSlice);

			pSlice->returnExtraSpace();
		}
	
		// TODO should we call _align here?

	}
	else
	{
		_purgeLayout(UT_TRUE);
		
		// we don't ... construct just enough to keep going
		UT_ASSERT(!m_pFirstLine);

		// the run is empty 
		DG_Graphics* pG = m_pLayout->getGraphics();

		m_pFirstRun = new fp_Run(this, pG, 0, 0);
		UT_ASSERT(m_pFirstRun);	// TODO check for outofmem
		
		m_pFirstRun->calcWidths(&m_gbCharWidths);

		// the line just contains the empty run
		UT_uint32 iGuessLineHeight = m_pFirstRun->getHeight();

		UT_uint32 iMaxLineWidth = requestLineSpace(iGuessLineHeight);
		UT_ASSERT(iMaxLineWidth > 0);

		fp_Line*	pLine = new fp_Line(iMaxLineWidth);
		pLine->addRun(m_pFirstRun);
		addLine(pLine);
	}

	UT_DEBUGMSG(("END reformat block: 0x%x\n", this));

#if 0	
	// NOTE we no longer clear the bNeedsCompleteReformat flag
	// on a minor reformat.  See me for the reason.  -EWS

	setNeedsCompleteReformat(UT_FALSE);
#endif	
	
	m_bFormatting = UT_FALSE;
	return 0;
}

int fl_BlockLayout::complete_format()
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
		UT_DEBUGMSG(("######## WARNING:  Brute-force reformat of block: 0x%x\n", this));

		/*
		  TODO originally this routine was a brute-force reformat of a block.
		  It has since been modified such that the runs are no longer destroyed.
		  In other words, it has been modified to do essentially what minor_reformat()
		  is supposed to do.  I've changed the names of the methods and comments
		  to make things clearer.  However, for now, I'm not changing the behavior
		  of this method much.

		  It is now clear that there are going to be several kinds of reformat
		  operations.  In some cases, we will want to stomp all the runs.
		  In some cases, we will want to stomp the slices.  In some cases
		  we want to call relayoutBlock and in others breakParagraph.
		*/

#define CLOBBER_SLICES
		
		int countSlices = m_vecSlices.getItemCount();
		for (int i=0; i<countSlices; i++)
		{
			fp_BlockSlice* pSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(i);
			UT_ASSERT(pSlice);

			pSlice->clearScreen(m_pLayout->getGraphics());
			pSlice->deleteLines();
#ifdef CLOBBER_SLICES
			pSlice->remove();

			delete pSlice;
#else
			pSlice->verifyColumnFit();
#endif
		}
		m_pFirstLine = m_pLastLine = NULL;

#ifdef CLOBBER_SLICES
		for (int j=countSlices-1; j>=0; j--)
		{
			m_vecSlices.deleteNthItem(j);
		}
		UT_ASSERT(m_vecSlices.getItemCount() == 0);
		m_pCurrentSlice = NULL;
#else
		m_pCurrentSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(0);
#endif		

		/*
		  NOTE that we currently leave the runs alone.  If we were
		  to stomp them too, then we should just call _purgeLayout().
		*/
	}

#if 0
	_createRuns();
	_verifyCurrentSlice(); // this is helpful for empty paragraphs
#endif 

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

		m_pFirstRun = new fp_Run(this, pG, 0, 0);
		m_pFirstRun->calcWidths(&m_gbCharWidths);

		// the line just contains the empty run
		UT_uint32 iGuessLineHeight = m_pFirstRun->getHeight();

		UT_uint32 iMaxLineWidth = requestLineSpace(iGuessLineHeight);
		UT_ASSERT(iMaxLineWidth > 0);

		fp_Line*	pLine = new fp_Line(iMaxLineWidth);
		pLine->addRun(m_pFirstRun);
		addLine(pLine);
	}

	int countSlices = m_vecSlices.getItemCount();
	for (int i=0; i<countSlices; i++)
	{
		fp_BlockSlice* pSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(i);
		UT_ASSERT(pSlice);

		pSlice->returnExtraSpace();
	}

	align();

	m_bFormatting = UT_FALSE;

	setNeedsCompleteReformat(UT_FALSE);
	
	UT_DEBUGMSG(("END Formatting block: 0x%x\n", this));

	return 0;	// TODO return code
}

/*
	requestLineSpace is called by the fb_LineBreaker when it needs to ask for space in which
	to put a new line of content.  The return value is the width of the line.  LineBreakers
	do not know which slice a line is stored in.  That is managed entirely by the fl_BlockLayout
	itself.
*/
int	fl_BlockLayout::requestLineSpace(int iHeight)
{
	_verifyCurrentSlice();

	// TODO assert that iHeight is reasonable
	int wid = m_pCurrentSlice->requestLineSpace(iHeight);

	if (wid == 0)
	{
		UT_sint32 ndx = m_vecSlices.findItem(m_pCurrentSlice);
		UT_ASSERT(ndx >= 0);

		fp_BlockSlice* pNextSlice;
		if (m_vecSlices.getItemCount() > (UT_uint32)(ndx+1))
		{
			pNextSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(ndx+1);
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

fp_Run* fl_BlockLayout::getFirstRun()
{
	return m_pFirstRun;
}

int	fl_BlockLayout::addLine(fp_Line* pLine)
{
	UT_ASSERT(pLine);
	UT_ASSERT(m_pCurrentSlice);
	
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

void fl_BlockLayout::_createRuns()
{
	// TODO -- this is obsolete now, right?
	// ==>: if not, trigger fl_Listener::populate() via PD_Document downcall 
	// EX:  m_pDoc->repopulate(m_sdh, this);	// or get lid from m_pLayout
}

void fl_BlockLayout::setNeedsCompleteReformat(UT_Bool b)
{
	m_bNeedsReformat = b;
}

UT_Bool fl_BlockLayout::needsCompleteReformat()
{
	return m_bNeedsReformat;
}

const char*	fl_BlockLayout::getProperty(const XML_Char * pszName)
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance
	
	getAttrProp(&pBlockAP);
	
	return PP_evalProperty(pszName,pSpanAP,pBlockAP,pSectionAP);
}

UT_uint32 fl_BlockLayout::getPosition(UT_Bool bActualBlockPos) const
{
	PT_DocPosition pos = m_pDoc->getStruxPosition(m_sdh);

	// it's usually more useful to know where the runs start
	if (!bActualBlockPos)
		pos += fl_BLOCK_STRUX_OFFSET;

	return pos;
}

UT_GrowBuf * fl_BlockLayout::getCharWidths(void)
{
	return &m_gbCharWidths;
}


UT_Bool fl_BlockLayout::getSpanPtr(UT_uint32 offset, const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const
{
	return m_pDoc->getSpanPtr(m_sdh, offset+fl_BLOCK_STRUX_OFFSET, ppSpan, pLength);
}

UT_Bool	fl_BlockLayout::getBlockBuf(UT_GrowBuf * pgb) const
{
	return m_pDoc->getBlockBuf(m_sdh, pgb);
}

fp_Run* fl_BlockLayout::findPointCoords(PT_DocPosition iPos, UT_Bool bEOL, UT_uint32& x, UT_uint32& y, UT_uint32& height)
{
	// find the run which has this position inside it.
	UT_ASSERT(iPos >= getPosition());
	UT_uint32 iRelOffset = iPos - getPosition();

	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		UT_uint32 iWhere = pRun->containsOffset(iRelOffset);
		if (FP_RUN_INSIDE == iWhere)
		{
			pRun->findPointCoords(iRelOffset, x, y, height);
			return pRun;
		}
		else if (bEOL && (FP_RUN_JUSTAFTER == iWhere))
		{
			fp_Run* pNext = pRun->getNext();
			fp_Line* pNextLine = NULL;

			if (pNext)
				pNextLine = pNext->getLine();

			if (pNextLine != pRun->getLine())
			{
				pRun->findPointCoords(iRelOffset, x, y, height);
				return pRun;
			}
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

fp_Line* fl_BlockLayout::getLastLine()
{
	fp_BlockSlice* pLastSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(m_vecSlices.getItemCount()-1);
	fp_Line* pLastLine = pLastSlice->getNthLine(pLastSlice->countLines()-1);
	return pLastLine;
}

fp_Line* fl_BlockLayout::getFirstLine()
{
	fp_BlockSlice* pSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(0);
	fp_Line* pLine = pSlice->getNthLine(0);
	return pLine;
}

fp_Line* fl_BlockLayout::findPrevLineInDocument(fp_Line* pLine)
{
	int count = m_vecSlices.getItemCount();
	fp_Line* pPrev = NULL;
	for (int i=0; i<count; i++)
	{
		fp_BlockSlice* pBS = (fp_BlockSlice*) m_vecSlices.getNthItem(i);
		int count2 = pBS->countLines();
		for (int j=0; j<count2; j++)
		{
			fp_Line* pL = pBS->getNthLine(j);
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
						fl_SectionLayout* pSL = m_pLayout->getPrevSection(m_pSectionLayout);

						if (!pSL)
						{
							// at EOD, so just bail
							return NULL;
						}

						fl_BlockLayout* pBlock = pSL->getLastBlock();
						UT_ASSERT(pBlock);
						return pBlock->getLastLine();
					}
				}
			}
			pPrev = pL;
		}
	}
	
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

fp_Line* fl_BlockLayout::findNextLineInDocument(fp_Line* pLine)
{
	if (pLine->getNext())
	{
		return pLine->getNext();
	}
	
	int count = m_vecSlices.getItemCount();
	for (int i=0; i<count; i++)
	{
		fp_BlockSlice* pBS = (fp_BlockSlice*) m_vecSlices.getNthItem(i);
		int count2 = pBS->countLines();
		for (int j=0; j<count2; j++)
		{
			fp_Line* pL = pBS->getNthLine(j);
			if (pL == pLine)
			{
				// found the line.
				UT_ASSERT(!((j+1) < count2));	// if this were true, it would have been caught above
				
				if ((i+1) < count)
				{
					// grab the first line from the next blockslice
					pBS = (fp_BlockSlice*) m_vecSlices.getNthItem(i+1);
					return pBS->getNthLine(0);
				}
				else if (m_pNext)
				{
					// grab the first line from the next block
					return m_pNext->getFirstLine();
				}
				else
				{
					// there is no next line in this section, try the next
					fl_SectionLayout* pSL = m_pLayout->getNextSection(m_pSectionLayout);

					if (!pSL)
					{
						// at EOD, so just bail
						return NULL;
					}

					fl_BlockLayout* pBlock = pSL->getFirstBlock();
					UT_ASSERT(pBlock);
					return pBlock->getFirstLine();
				}
			}
		}
	}
	
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

fl_BlockLayout* fl_BlockLayout::getNext(UT_Bool bKeepGoing) const
{
	if (m_pNext || !bKeepGoing)
		return m_pNext;

	// keep going (check next section)
	fl_SectionLayout* pSL = m_pLayout->getNextSection(m_pSectionLayout);
	fl_BlockLayout* pBL = NULL;

	if (pSL)
	{
		pBL = pSL->getFirstBlock();
		UT_ASSERT(pBL);
	}

	return pBL;
}

fl_BlockLayout* fl_BlockLayout::getPrev(UT_Bool bKeepGoing) const
{
	if (m_pPrev || !bKeepGoing)
		return m_pPrev;

	// keep going (check prev section)
	fl_SectionLayout* pSL = m_pLayout->getPrevSection(m_pSectionLayout);
	fl_BlockLayout* pBL = NULL;

	if (pSL)
	{
		pBL = pSL->getLastBlock();
		UT_ASSERT(pBL);
	}

	return pBL;
}

fp_BlockSlice* fl_BlockLayout::getFirstSlice()
{
	return (fp_BlockSlice*) m_vecSlices.getNthItem(0);
}

fp_BlockSlice* fl_BlockLayout::getLastSlice()
{
	return (fp_BlockSlice*) m_vecSlices.getNthItem(m_vecSlices.getItemCount()-1);
}

void fl_BlockLayout::dump()
{
	int count = m_vecSlices.getItemCount();
	UT_DEBUGMSG(("fl_BlockLayout 0x%x is from element 0x%x and contains %d slices.\n", this, m_sdh, count));

	for (int i=0; i<count; i++)
	{
		fp_BlockSlice* p = (fp_BlockSlice*) m_vecSlices.getNthItem(i);

		UT_DEBUGMSG(("fl_BlockLayout::dump(0x%x) - fp_BlockSlice 0x%x, height=%d, in column 0x%x\n", this, p, p->getHeight(), p->getColumn()));
	}

	UT_DEBUGMSG(("fl_BlockLayout 0x%x contains the following runs:\n",this));

	fp_Run * pRun = getFirstRun();
	while (pRun)
	{
		pRun->dumpRun();
		pRun = pRun->getNext();
	}
}

fl_PartOfBlock::fl_PartOfBlock(void)
{
	iOffset = 0;
	iLength = 0;
}

void fl_BlockLayout::_destroySpellCheckLists(void)
{
	fl_PartOfBlock*	pPOB;

	while (NULL != (pPOB = (fl_PartOfBlock*) m_lstNotSpellChecked.head()))
	{
		delete pPOB;
		m_lstNotSpellChecked.remove();
	}

	while (NULL != (pPOB = (fl_PartOfBlock*) m_lstSpelledWrong.head()))
	{
		delete pPOB;
		m_lstSpelledWrong.remove();
	}
}

void fl_BlockLayout::_addPartNotSpellChecked(UT_uint32 iOffset, UT_uint32 iLen)
{

	fl_PartOfBlock*	pPOB;
	bool foundAdjoiningRegion = false;

	// TODO: This will work fine when adding text.... but what about deletion?
	/* merge neighboring regions... */
	pPOB = (fl_PartOfBlock *) m_lstNotSpellChecked.head();
	while ( (!foundAdjoiningRegion) && (pPOB != (fl_PartOfBlock *) 0))
	{
		if ((pPOB->iOffset + pPOB->iLength) == iOffset)
		{
			/* new region comes right after this region... so extend this region */
			pPOB->iLength += iLen;
			foundAdjoiningRegion = true;
		}
		else if ((iOffset + iLen) == pPOB->iOffset)
		{
			/* new region comes right before this region... so extend this region */
			pPOB->iOffset = iOffset;
			pPOB->iLength += iLen;
			foundAdjoiningRegion = true;
		} 
		else if ((pPOB->iOffset < iOffset) && (iOffset < (pPOB->iOffset + pPOB->iLength)))
		{
			/* overlapping regions... this region is in front of new region*/
			if ((pPOB->iOffset + pPOB->iLength) < (iOffset + iLen))
			{
				pPOB->iLength = iLen + (iOffset - pPOB->iOffset);
			}
			/* else, 
				new region is a subset of this region... do nothing 
			*/

			foundAdjoiningRegion = true;
		}
		else if ((iOffset < pPOB->iOffset) && (pPOB->iOffset < (iOffset + iLen)))
		{
			/* overlapping regions... new region is in front of this region */
			if ((iOffset + iLen) < (pPOB->iOffset + pPOB->iLength))
			{
				pPOB->iOffset = iOffset;
				pPOB->iLength = pPOB->iLength + (pPOB->iOffset - iOffset);
			}
			else
			{
				/* new region is a superset of this region */
				pPOB->iOffset = iOffset;
				pPOB->iLength = iLen;
			}
			foundAdjoiningRegion = true;
		} else if ((iOffset == pPOB->iOffset) && (iLen == pPOB->iLength))
		{
			/* same region... do nothing */
			foundAdjoiningRegion = true;
		}
		
		pPOB = (fl_PartOfBlock *) m_lstNotSpellChecked.next();
	}
	
	if (!foundAdjoiningRegion)
	{
		pPOB = new fl_PartOfBlock();
		if (!pPOB)
		{
			// TODO handle outofmem
		}
		
		pPOB->iOffset = iOffset;
		pPOB->iLength = iLen;
	
		(void) m_lstNotSpellChecked.tail();
		m_lstNotSpellChecked.append(pPOB);
	}
}

void fl_BlockLayout::_addPartSpelledWrong(UT_uint32 iOffset, UT_uint32 iLen)
{
	fl_PartOfBlock*	pPOB = new fl_PartOfBlock();
	if (!pPOB)
	{
		// TODO handle outofmem
	}
	
	pPOB->iOffset = iOffset;
	pPOB->iLength = iLen;

	(void) m_lstSpelledWrong.tail();
	m_lstSpelledWrong.append(pPOB);
}

void fl_BlockLayout::_resetSpellCheckState(void)
{
	// first, remove anything currently in the lists.
	_destroySpellCheckLists();

	/*
	  We grab the getBlockBuf() here just so we can get the length
	  of the text in this block.  This a VERY bad.  There must be
	  an easier, more efficient way to do this.
	*/
	UT_GrowBuf pgb(1024);
	UT_Bool bRes = getBlockBuf(&pgb);
	
	_addPartNotSpellChecked(0, pgb.getLength());
}

void fl_BlockLayout::checkSpelling(void)
{
	/*
	  First, we need to get a pointer to all the text in this
	  block.
	*/
	
	UT_GrowBuf pgb(1024);

	UT_Bool bRes = getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar* pBlockText = pgb.getPointer(0);

	fl_PartOfBlock* pPOB;
	UT_uint32 wordBeginning, wordLength;
	UT_uint32 eor; /* end of region */
	bool found;
	
	for (pPOB = (fl_PartOfBlock*) m_lstNotSpellChecked.head(); pPOB; pPOB = (fl_PartOfBlock*) m_lstNotSpellChecked.next())
	{

		UT_ASSERT(pPOB->iLength <= pgb.getLength());

		/*
		  For each word in this region, spell check it.  If it's bad,
		  add it to lstSpelledWrong.  
		*/

		wordBeginning = pPOB->iOffset;
		eor = pPOB->iOffset + pPOB->iLength;
		eor = (eor > pgb.getLength()) ? pgb.getLength() : eor;  // bounds check...

		/* Loop through all of the words in this Part of Block segment */

		while (wordBeginning < eor)
		{
			/*** skip delimiters... */
			while ((wordBeginning < eor) && (UT_isWordDelimiter( pBlockText[wordBeginning])))
			{
				wordBeginning++;
			}

			if (!(wordBeginning < eor))
			{
				/* we're at the start of a word. find end of word */
				found = false;
				wordLength = 0;
				while ((!found) && (wordLength < pPOB->iLength))
				{
					if ( UT_TRUE == UT_isWordDelimiter( pBlockText[pPOB->iOffset + wordLength] ))
					{
						found = true;
					}
					wordLength++;
				}
	
				if (! SpellCheckNWord16( &(pBlockText[wordBeginning]), wordLength))
				{
					/* unknown word... squiggle it */
					_addPartSpelledWrong(wordBeginning, wordLength);
				}
	
				wordBeginning += (wordLength + 1);
			}
		}

	}

	/*
	  After a spell check, m_lstNotSpellChecked should be empty.  So we clear it.
	*/
	while (NULL != (pPOB = (fl_PartOfBlock*) m_lstNotSpellChecked.head()))
	{
		delete pPOB;
		m_lstNotSpellChecked.remove();
	}
}

/*
  TODO

  spell check problems:

  1.  we currently never call _resetSpellCheckState.  We need to do so sometime.
  2.  when a block is modified, we need to be able to adjust the offsets of all
      the POBs.
*/

