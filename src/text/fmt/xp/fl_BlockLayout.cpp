 
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

fl_BlockLayout::fl_BlockLayout(PL_StruxDocHandle sdh,
							   fb_LineBreaker* pBreaker,
							   fl_BlockLayout* pPrev,
							   fl_SectionLayout* pSectionLayout)
	: fl_Layout(PTX_Block, sdh)
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

		fp_Column *pCol = m_pSectionLayout->getNewColumn();

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

void fl_BlockLayout::setAlignment(UT_uint32 iAlignCmd)
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

	_align();
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

void fl_BlockLayout::_align()
{
	int countSlices = m_vecSlices.getItemCount();
	for (int i=0; i<countSlices; i++)
	{
		fp_BlockSlice* pSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(i);
		UT_ASSERT(pSlice);

		pSlice->align();
	}
}

int fl_BlockLayout::reformat()
{
	UT_ASSERT(m_pLayout->getGraphics()->queryProperties(DG_Graphics::DGP_SCREEN));
	UT_DEBUGMSG(("BEGIN reformat block: 0x%x\n", this));

	UT_ASSERT(!m_bFormatting);
	m_bFormatting = UT_TRUE;

	m_pCurrentSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(0);

	m_pBreaker->reLayoutParagraph(this);

	UT_DEBUGMSG(("END reformat block: 0x%x\n", this));

	setNeedsReformat(UT_FALSE);
	
	m_bFormatting = UT_FALSE;
	return 0;
}

void fl_BlockLayout::_purgeLayout(UT_Bool bVisible)
{
	int countSlices = m_vecSlices.getItemCount();
	for (int i=0; i<countSlices; i++)
	{
		fp_BlockSlice* pSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(i);
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
		fp_Run* pNext = m_pFirstRun->getNext();
		delete m_pFirstRun;
		m_pFirstRun = pNext;
	}

	m_pCurrentSlice = NULL;
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
		m_pFirstRun = NULL;

	// remove runs from lines
	fp_Run* pRun = pTruncRun;
	while (pRun)
	{
		fp_Line* pLine = pRun->getLine();
		UT_ASSERT(pLine);

		pRun->clearScreen();
		pLine->removeRun(pRun);

		pRun = pRun->getNext();
	}

	// remove empty lines 
	while (m_pLastLine && (m_pLastLine != m_pFirstLine))
	{
		if (m_pLastLine->countRuns())
			break;

		fp_Line* pLine = m_pLastLine;
		m_pLastLine = m_pLastLine->getPrev();

		pLine->remove();
	}

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

int fl_BlockLayout::format()
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
			fp_BlockSlice* pSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(i);
			UT_ASSERT(pSlice);
			
			pSlice->clearScreen(m_pLayout->getGraphics());
			pSlice->deleteLines();
			pSlice->verifyColumnFit();
		}

		m_pFirstLine = m_pLastLine = NULL;
#if 0
		while (m_pFirstRun)
		{
			fp_Run* pNext = m_pFirstRun->getNext();
			delete m_pFirstRun;
			m_pFirstRun = pNext;
		}

		m_gbCharWidths.truncate(0);
#endif
		m_pCurrentSlice = (fp_BlockSlice*) m_vecSlices.getNthItem(0);
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

	_align();

	m_bFormatting = UT_FALSE;

	setNeedsReformat(UT_FALSE);
	
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

void fl_BlockLayout::setNeedsReformat(UT_Bool b)
{
	m_bNeedsReformat = b;
}

UT_Bool fl_BlockLayout::needsReformat()
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
					// there is no next line in the document
					return NULL;
				}
			}
		}
	}
	
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

fl_BlockLayout* fl_BlockLayout::getNext() const
{
	return m_pNext;
}

fl_BlockLayout* fl_BlockLayout::getPrev() const
{
	return m_pPrev;
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
