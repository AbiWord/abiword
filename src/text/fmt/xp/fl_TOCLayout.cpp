/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Martin Sevior (msevior@physics.unimelb.edu.au>
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

#include <string.h>
#include <stdlib.h>

#include "ut_types.h"
#include "ut_string.h"
#include "xap_App.h"
#include "ap_Strings.h"
#include "ap_Prefs.h"
#include "fl_SectionLayout.h"
#include "fl_TableLayout.h"
#include "fp_TableContainer.h"
#include "fl_TOCLayout.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fb_LineBreaker.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fp_Column.h"
#include "fp_TOCContainer.h"
#include "fp_ContainerObject.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "gr_Graphics.h"
#include "pp_Property.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"
#include "px_CR_Glob.h"
#include "fp_Run.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_units.h"
#include "fv_View.h"
#include "pd_Style.h"

TOCEntry::TOCEntry(fl_BlockLayout * pBlock,
				   UT_sint32 iLevel, 
				   UT_UTF8String & sDispStyle,
				   bool bHaveLabel,
				   FootnoteType iFType,
				   UT_UTF8String & sBefore,
				   UT_UTF8String sAfter, 
				   bool bInherit,
				   UT_sint32 /*iStartAt*/):
	m_pBlock(pBlock),
	m_iLevel(iLevel),
	m_sDispStyle(sDispStyle),
	m_bHasLabel(bHaveLabel),
	m_iFType(iFType),
	m_sBefore(sBefore),
	m_sAfter(sAfter),
	m_bInherit(bInherit)
{
}

TOCEntry::~TOCEntry(void)
{
	m_iLevel = -1;
	UT_DEBUGMSG(("Deleteing entry %p \n",this));
}

PT_DocPosition TOCEntry::getPositionInDoc(void)
{
	PT_DocPosition pos = m_pBlock->getPosition();
	return pos;
}

void TOCEntry::setPosInList(UT_sint32 posInList)
{
	m_iPosInList = posInList;
}

void TOCEntry::calculateLabel(TOCEntry * pPrevLevel)
{
	UT_String sVal;
	sVal.clear();
	m_pBlock->getView()->getLayout()->getStringFromFootnoteVal(sVal,m_iPosInList,m_iFType);
	if((pPrevLevel == NULL) || !m_bInherit)
	{
		m_sLabel = sVal.c_str();
		return;
	}
	m_sLabel = pPrevLevel->getNumLabel();
	m_sLabel += ".";
	m_sLabel += sVal.c_str();
}

UT_UTF8String  TOCEntry::getFullLabel(void)
{
	static UT_UTF8String sFull;
	sFull.clear();
	sFull = m_sBefore;
	sFull += m_sLabel;
	sFull += m_sAfter;
	return sFull;
}

fl_TOCLayout::fl_TOCLayout(FL_DocLayout* pLayout, fl_DocSectionLayout* pDocSL, pf_Frag_Strux* sdh, PT_AttrPropIndex indexAP, fl_ContainerLayout * pMyContainerLayout) 
 	: fl_SectionLayout(pLayout, sdh, indexAP, FL_SECTION_TOC,FL_CONTAINER_TOC,PTX_SectionTOC,pMyContainerLayout),
	  m_bNeedsRebuild(false),
	  m_bNeedsFormat(true),
	  m_bIsOnPage(false),
	  m_pDocSL(pDocSL),
	  m_bHasEndTOC(false),
	  m_bDoingPurge(false),
	  m_bIsSelected(false),
	  m_iNumType1(FOOTNOTE_TYPE_NUMERIC),
	  m_iNumType2(FOOTNOTE_TYPE_NUMERIC),
	  m_iNumType3(FOOTNOTE_TYPE_NUMERIC),
	  m_iNumType4(FOOTNOTE_TYPE_NUMERIC),
	  m_iTabLeader1(FL_LEADER_DOT),
	  m_iTabLeader2(FL_LEADER_DOT),
	  m_iTabLeader3(FL_LEADER_DOT),
	  m_iTabLeader4(FL_LEADER_DOT),
	  m_iCurrentLevel(0),
	  m_bMissingBookmark(false),
	  m_bFalseBookmarkEstimate(false)
{
	UT_ASSERT(m_pDocSL->getContainerType() == FL_CONTAINER_DOCSECTION);
//	_createTOCContainer();
//	_insertTOCContainer(static_cast<fp_TOCContainer *>(getLastContainer()));
	m_pLayout->addTOC(this);
}

fl_TOCLayout::~fl_TOCLayout()
{
	// NB: be careful about the order of these
	UT_DEBUGMSG(("Deleting TOClayout %p \n",this));
	_purgeLayout();
	fp_TOCContainer * pTC = static_cast<fp_TOCContainer *>(getFirstContainer());
	while(pTC)
	{
		fp_TOCContainer * pNext = static_cast<fp_TOCContainer *>(pTC->getNext());
		if(pTC == static_cast<fp_TOCContainer *>(getLastContainer()))
		{
			pNext = NULL;
		}
		delete pTC;
		pTC = pNext;
	}

	setFirstContainer(NULL);
	setLastContainer(NULL);
	m_pLayout->removeTOC(this);
}
	
/*!
 * Returns the position in the document of the PTX_SectionTOC strux
 * This is very useful for determining the value of the footnote reference
 * and anchor. 
*/
PT_DocPosition fl_TOCLayout::getDocPosition(void) 
{
	pf_Frag_Strux* sdh = getStruxDocHandle();
    return 	m_pLayout->getDocument()->getStruxPosition(sdh);
}

void fl_TOCLayout::setSelected(bool bIsSelected)
{
	if(!bIsSelected  && m_bIsSelected)
	{
		m_bIsSelected = false;
		fp_TOCContainer * pTOCCon = static_cast<fp_TOCContainer *>(getFirstContainer());
		pTOCCon->forceClearScreen();
		markAllRunsDirty();
		m_pLayout->getView()->updateScreen();
//		pTOCCon->draw(m_pLayout->getGraphics());
	}
	m_bIsSelected = bIsSelected;
	if(m_bIsSelected)
	{
		fp_TOCContainer * pTOCCon = static_cast<fp_TOCContainer *>(getFirstContainer());
		pTOCCon->forceClearScreen();
		markAllRunsDirty();
		m_pLayout->getView()->updateScreen();
//		pTOCCon->draw(m_pLayout->getGraphics());
	}
}

/*!
 * This method returns the length of the footnote. This is such that 
 * getDocPosition() + getLength() is one value beyond the the EndFootnote
 * strux
 */
UT_uint32 fl_TOCLayout::getLength(void)
{
	PT_DocPosition startPos = getDocPosition();
	pf_Frag_Strux* sdhEnd = NULL;
	pf_Frag_Strux* sdhStart = getStruxDocHandle();
	UT_DebugOnly<bool> bres;
	bres = m_pLayout->getDocument()->getNextStruxOfType(sdhStart,PTX_EndTOC,&sdhEnd);
	UT_ASSERT(bres && sdhEnd);
	PT_DocPosition endPos = m_pLayout->getDocument()->getStruxPosition(sdhEnd);
	UT_uint32 length = static_cast<UT_uint32>(endPos - startPos + 1); 
	return length;
}


bool fl_TOCLayout::bl_doclistener_insertEndTOC(fl_ContainerLayout*,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew))
{
	// The endFootnote strux actually needs a format handle to to this Footnote layout.
	// so we bind to this layout. We also set a pointer to keep track of the endTOC strux.

		
	fl_ContainerLayout* sfhNew = this;
	pfnBindHandles(sdh,lid,sfhNew);
	setEndStruxDocHandle(sdh);

//
// increment the insertion point in the view.
//
	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->setPoint(pcrx->getPosition() +  fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->setPoint(pView->getPoint() +  fl_BLOCK_STRUX_OFFSET);
	}
	m_bHasEndTOC = true;

	fillTOC();

	return true;
}

/*!
 * Set boolean to tell that TOCend has been inserted. Also makes sure the
 * layout is fully filled.
 */
void fl_TOCLayout::setTOCEndIn(void)
{
	m_bHasEndTOC = true;
}

/*!
 * This signals an incomplete footnote section.
 */
bool fl_TOCLayout::doclistener_deleteEndTOC( const PX_ChangeRecord_Strux * /*pcrx*/)
{
	m_bHasEndTOC = false;
	return true;
}


fl_SectionLayout * fl_TOCLayout::getSectionLayout(void) const
{
	fl_ContainerLayout * pDSL = myContainingLayout();
	while(pDSL)
	{
		if(pDSL->getContainerType() == FL_CONTAINER_DOCSECTION)
		{
			return static_cast<fl_SectionLayout *>(pDSL);
		}
		pDSL = pDSL->myContainingLayout();
	}
	return NULL;
}

FootnoteType fl_TOCLayout::getNumType(UT_sint32 iLevel)
{
	if(iLevel == 1)
	{
		return m_iNumType1;
	}
	else if(iLevel == 2)
	{
		return m_iNumType2;
	}
	else if(iLevel == 3)
	{
		return m_iNumType3;
	}
	else if(iLevel == 4)
	{
		return m_iNumType4;
	}
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return static_cast<FootnoteType>(0);
}


eTabLeader fl_TOCLayout::getTabLeader(UT_sint32 iLevel)
{
	if(iLevel == 1)
	{
		return m_iTabLeader1;
	}
	else if(iLevel == 2)
	{
		return m_iTabLeader2;
	}
	else if(iLevel == 3)
	{
		return m_iTabLeader3;
	}
	else if(iLevel == 4)
	{
		return m_iTabLeader4;
	}
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return static_cast<eTabLeader>(0);
}

UT_sint32 fl_TOCLayout::getTabPosition(UT_sint32 iLevel, const fl_BlockLayout * pBlock)
{
	fp_TOCContainer * pTOCC = static_cast<fp_TOCContainer *>(getFirstContainer());
	if(pTOCC == NULL)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return 0;
	}
	UT_sint32 iWidth = pTOCC->getWidth() -pBlock->getLeftMargin();
	UT_UTF8String sStr("");
	if(iLevel == 1)
	{
		sStr = m_sNumOff1;
	}
	else if(iLevel == 2)
	{
		sStr = m_sNumOff2;
	}
	else if(iLevel == 3)
	{
		sStr = m_sNumOff3;
	}
	else if(iLevel == 4)
	{
		sStr = m_sNumOff4;
	}
	iWidth -= UT_convertToLogicalUnits(sStr.utf8_str());
	return iWidth;
}

/*
   During the filling of the doc layout when the TOC layout is restricted by a bookmark, we have
   made the assumption that if the bookmark was not in the doc when we are asked to add a particular
   block, it was to come later in the filling process; in addition we assumed that the bookmark will
   not immediately follow the block strux. We now need to verify these assumption and if either is
   false, redo the TOC.
*/
bool fl_TOCLayout::verifyBookmarkAssumptions()
{
	UT_return_val_if_fail(!m_pLayout->isLayoutFilling(), false);

	if((!m_bMissingBookmark && !m_bFalseBookmarkEstimate) || !m_sRangeBookmark.size())
		return false;
	
	PD_Document * pDoc = m_pLayout->getDocument();
	UT_return_val_if_fail(pDoc, false);

	if(m_bFalseBookmarkEstimate || (m_bMissingBookmark && m_pDoc->isBookmarkUnique(m_sRangeBookmark.utf8_str())))
	{
		// this bookmark either does not exist, or it was positioned earlier than we assumed
		fillTOC();
	}

	return true;
}


/*
    bVerifyRange indicates whether the function should verify that pBlock is inside the range indicated
    by associated bookmark. This parameter has default value true, but the caller can specify false
    if the block is known to be inside the TOC range (the range checking is involved, so if this
    function is called from a loop, it is desirable that most of the range verification is taken
    outside the loop -- see for example fillTOC())
*/
bool fl_TOCLayout::addBlock(fl_BlockLayout * pBlock, bool bVerifyRange)
{
	UT_return_val_if_fail( pBlock, false );
	UT_UTF8String sStyle;
	pBlock->getStyle(sStyle);

	if(bVerifyRange && m_sRangeBookmark.size() /*&& !m_pLayout->isLayoutFilling()*/)
	{
		// we need to ascertain whether the block is in our range
		PD_Document * pDoc = m_pLayout->getDocument();
		UT_return_val_if_fail( pDoc, false );

		const gchar * pBookmark = m_sRangeBookmark.utf8_str();
	
		if(!m_pDoc->isBookmarkUnique(pBookmark))
		{
			UT_uint32 i = 0;
			fp_Run * pRun;
			fl_BlockLayout * pBL;
			fp_BookmarkRun * pB[2] = {NULL,NULL};
			fl_ContainerLayout * pDSL = m_pLayout->getFirstSection();
			fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pDSL);
			bool bFound = false;
			
			while(pCL && pCL->getContainerType() != FL_CONTAINER_BLOCK)
			{
				pCL = pCL->getFirstLayout();
			}
			if(pCL == NULL)
			{
				return false;
			}
			if(pCL->getContainerType() != FL_CONTAINER_BLOCK)
			{
				return false;
			}

			pBL = static_cast<fl_BlockLayout *>(pCL);
			PT_DocPosition pos1 = 0, pos2 = 0xffffffff;

			// during the fill process we will take advantage of the fact that the doc positions do
			// not change, and will store the positions of our bookmarks; if the document is not
			// filling, we will do this the hard way
			
			if(!m_pLayout->isLayoutFilling())
				m_vecBookmarkPositions.clear();
			
			if(m_vecBookmarkPositions.getItemCount() < 2)
			{
				if(m_vecBookmarkPositions.getItemCount() == 1)
				{
					pos1  = m_vecBookmarkPositions.getNthItem(0);

					if(m_bMissingBookmark)
					{
						// the stored position is only an estimate == strux offset + 1
						// we have to substract the one
						UT_ASSERT_HARMLESS( pos1 );
						--pos1;

						// we are still looking for the starting bookmark
						i = 0;
					}
					else
					{
						// the stored position is the real position, so we only need to look for the
						// end bookmark
						i = 1;
					}

					// now jump to the block in question
					while(pBL->getNextBlockInDocument() && pBL->getNextBlockInDocument()->getPosition(true) < pos1)
						pBL = pBL->getNextBlockInDocument();
				}
			
				while(pBL)
				{
					pRun = pBL->getFirstRun();

					while(pRun)
					{
						if(pRun->getType()== FPRUN_BOOKMARK)
						{
							fp_BookmarkRun * pBR = static_cast<fp_BookmarkRun*>(pRun);
							bool bIsRight = (i == 0 && pBR->isStartOfBookmark()) || (i != 0 && !pBR->isStartOfBookmark());
							
							if(bIsRight && !strcmp(pBR->getName(),pBookmark))
							{
								pB[i] = pBR;
								i++;
								if(i>1)
								{
									bFound = true;
									break;
								}
							}
						}

						pRun = pRun->getNextRun();
					}
			
					if(bFound)
						break;
			
					pBL = pBL->getNextBlockInDocument();
				}

				if(!pB[0] && !m_vecBookmarkPositions.getItemCount() && m_pLayout->isLayoutFilling())
				{
					// we will assume that the bookmark is still to come and that it is immediately
					// after the strux, but we need to make note that we made that assumption ...
					m_bMissingBookmark = true;
					pos1 = pBlock->getPosition(false); // position immediately after the strux
					m_vecBookmarkPositions.addItem(pos1);
				}
				else if(!pB[0] && m_vecBookmarkPositions.getItemCount())
				{
					// this is the case where we already knew the position and set it earlier
					// do nothing
					UT_ASSERT_HARMLESS( m_pLayout->isLayoutFilling() && m_vecBookmarkPositions.getItemCount() == 1 );
				}
				else if(!pB[0] && !m_pLayout->isLayoutFilling())
				{
					// the document does not contain this bookmark
					// we build the toc from the whole document
					pos1 = 0;
				}
				else if(pB[0])
				{
					pos1 = pB[0]->getBookmarkedDocPosition(false);
					PT_DocPosition posOld = 0;
					
					if(m_vecBookmarkPositions.getItemCount())
					{
						// this is the case where we guessed the pos1
						posOld = m_vecBookmarkPositions.getNthItem(0);
						m_vecBookmarkPositions.clear();
					}
					
					if(m_pLayout->isLayoutFilling())
						m_vecBookmarkPositions.addItem(pos1);

					m_bMissingBookmark = false; // this is the real thing
					
					if(pos1 < posOld)
					{
						// we assumed that the actual postion of the bookmark was after the strux
						// which we were processing when we previously set pos1. This assumption was
						// incorrect, so we will have to redo the layout later
						m_bFalseBookmarkEstimate = true;
					}
				}
			
				if(!pB[1] && pos1 != 0)
				{
					// end bookmark not loaded yet
					UT_ASSERT_HARMLESS( m_pLayout->isLayoutFilling() );
					pos2 = 0xffffffff;
				}
				if(!pB[1] && pos1 == 0)
				{
					// this bookmark is not in the document, we will build the toc from the entire doc
					pos2 = 0xffffffff;
				}
				else if(pB[1])
				{
					pos2 = pB[1]->getBookmarkedDocPosition(true);

					if(m_pLayout->isLayoutFilling())
					{
						if(m_vecBookmarkPositions.getItemCount() != 1)
						{
							UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
							m_vecBookmarkPositions.clear();
							m_vecBookmarkPositions.addItem(0);
						}

						m_vecBookmarkPositions.addItem(pos2);
					}
				}
			}
			else
			{
				UT_ASSERT_HARMLESS(m_vecBookmarkPositions.getItemCount() == 2 && m_pLayout->isLayoutFilling());
				pos1 = m_vecBookmarkPositions.getNthItem(0);
				pos2 = m_vecBookmarkPositions.getNthItem(1);
			}
			
			
			if(pBlock->getPosition(true) < pos1 || pBlock->getPosition(true) >= pos2)
				return false;
		}
	}
	
	
	if(_isStyleInTOC(sStyle,m_sSourceStyle1))
	{
		m_iCurrentLevel = 1;
		_addBlockInVec(pBlock,m_sDestStyle1);
		return true;
	}
	if(_isStyleInTOC(sStyle,m_sSourceStyle2))
	{
		m_iCurrentLevel = 2;
		_addBlockInVec(pBlock,m_sDestStyle2);
		return true;
	}
	if(_isStyleInTOC(sStyle,m_sSourceStyle3))
	{
		m_iCurrentLevel = 3;
		_addBlockInVec(pBlock,m_sDestStyle3);
		return true;
	}
	if(_isStyleInTOC(sStyle,m_sSourceStyle4))
	{
		m_iCurrentLevel = 4;
		_addBlockInVec(pBlock,m_sDestStyle4);
		return true;
	}
	return false;
}

/*
    This function creates a TOC entry from the given document positions, with all the frills added
    (this code used to be inside _addBlockInVec())
*/
void fl_TOCLayout::_createAndFillTOCEntry(PT_DocPosition posStart, PT_DocPosition posEnd,
										  fl_BlockLayout * pPrevBL, const char * pszStyle,
										  UT_sint32 iAllBlocks)
{
	UT_return_if_fail(pszStyle);
	
	PD_Style * pStyle = NULL;
	m_pDoc->getStyle(pszStyle,&pStyle);
	if(pStyle == NULL)
	{
		m_pDoc->getStyle("Normal",&pStyle);
	}
	fl_TOCListener * pListen = new fl_TOCListener(this,pPrevBL,pStyle);
	PD_DocumentRange * docRange = new PD_DocumentRange(m_pDoc,posStart,posEnd);
	
	m_pDoc->tellListenerSubset(pListen, docRange);
	
	delete docRange;
	delete pListen;
	
	fl_BlockLayout * pNewBlock;
	if(pPrevBL)
	{
		pNewBlock = static_cast<fl_BlockLayout *>(pPrevBL->getNext());
	}
	else
	{
		pNewBlock = static_cast<fl_BlockLayout *>(getFirstLayout());
		if(pNewBlock && (pNewBlock->getNext() != NULL))
		{
			pNewBlock = static_cast<fl_BlockLayout *>(pNewBlock->getNext());
		}
	}

	// OK Now add the block to our vector.
	TOCEntry *pNewEntry = createNewEntry(pNewBlock);
	if(iAllBlocks == 0)
	{
		m_vecEntries.insertItemAt(pNewEntry,0);
	}
	else if (iAllBlocks < m_vecEntries.getItemCount())
	{
		m_vecEntries.insertItemAt(pNewEntry,iAllBlocks);
	}
	else
	{
		m_vecEntries.addItem(pNewEntry);
	}
	
	_calculateLabels();

	// Now append the tab and Field's to end of the new Block.
	PT_DocPosition iLen = posEnd - posStart - 1; // subtract 1 for the inital strux
	pNewBlock->_doInsertTOCTabRun(iLen);
	
	iLen++;
	pNewBlock->_doInsertFieldTOCRun(iLen);

	// Now Insert the TAB and TOCListLabel runs if requested.
	if(pNewEntry->hasLabel())
	{
		pNewBlock->_doInsertTOCListTabRun(0);
		pNewBlock->_doInsertTOCListLabelRun(0);
	}
	
	fp_Container * pTOCC = getFirstContainer();
	fl_DocSectionLayout * pDSL = getDocSectionLayout();
	if(pTOCC && pTOCC->getPage())
	{
		fp_Page * pPage = pTOCC->getPage();
		pDSL->setNeedsSectionBreak(true,pPage );
	}

	markAllRunsDirty();
	setNeedsReformat(0);
	setNeedsRedraw();
}

void fl_TOCLayout::_addBlockInVec(fl_BlockLayout * pBlock, UT_UTF8String & sStyle)
{
	// First find where to put the block.
	PT_DocPosition posNew = pBlock->getPosition();
	TOCEntry * pEntry = NULL;
	fl_BlockLayout * pPrevBL = NULL;
	UT_sint32 i = 0;
	bool bFound = false;
	
	for(i=0; i< m_vecEntries.getItemCount(); i++)
	{
		pEntry = m_vecEntries.getNthItem(i);
		pPrevBL = pEntry->getBlock();

		if(pPrevBL->getPosition() > posNew)
		{
			bFound = true;
			break;
		}
	}

	UT_sint32 iAllBlocks = 0;
	if(bFound)
	{
		if(i > 0)
		{
			pEntry =  m_vecEntries.getNthItem(i-1);
			pPrevBL =  pEntry->getBlock();
		}
		else
		{
			pEntry = NULL;
			pPrevBL = NULL;
		}
	}
	
	iAllBlocks = i;

	if(pPrevBL == NULL)
	{
		pPrevBL = static_cast<fl_BlockLayout *>(getFirstLayout());
	}
#if 0
	else if(!m_pLayout->isLayoutFilling())
	{
		// we have to redo the previous TOC block, if we have stolen some of its contents (i.e., if
		// the new block was inserted into a heading block) -- we need to see if the new block comes
		// immediately after the old block represented by pPrevBL
		PT_DocPosition posStart2 = pPrevBL->getPosition(true);
		PT_DocPosition posEnd2   = posStart2 + static_cast<PT_DocPosition>(pPrevBL->getLength());
		PT_DocPosition posStart  = pBlock->getPosition(true);
		UT_DEBUGMSG(("Prev. affected block is %d long \n",pPrevBL->getLength()));

		if(posEnd2 == posStart)
		{
			fl_BlockLayout * pPrevBL2 = NULL;
			UT_return_if_fail( pEntry && iAllBlocks > 0 );
			UT_UTF8String sDispStyle = pEntry->getDispStyle();
			UT_sint32 iNewLevel = pEntry->getLevel();
			if(i > 1)
			{
				pEntry =  m_vecEntries.getNthItem(i-2);
				pPrevBL2 =  pEntry->getBlock();
			}

			// now get rid of the old TOC block (this locates the shaddow to be removed by shd, so
			// it works whether passed the shaddow block or the main doc block)

			_removeBlockInVec(pPrevBL, true);
			pPrevBL = NULL;

			UT_sint32 iOldLevel = m_iCurrentLevel;
			m_iCurrentLevel = iNewLevel;
			_createAndFillTOCEntry(posStart2, posEnd2, pPrevBL2, sDispStyle.utf8_str(), iAllBlocks - 1);
			m_iCurrentLevel = iOldLevel;
			
			// we do not have to notify the orignal block that it is shaddowed, it knows already,
			// but we need to obtain the new pPrevBL for further processing

			if(pPrevBL2)
			{
				pPrevBL = static_cast<fl_BlockLayout *>(pPrevBL2->getNext());
			}
			else
			{
				pPrevBL = static_cast<fl_BlockLayout *>(getFirstLayout());
				if(pPrevBL && pPrevBL->getNext())
				{
					pPrevBL = static_cast<fl_BlockLayout *>(pPrevBL->getNext());
				}
			}
		}
	}
#endif
	PT_DocPosition posStart = pBlock->getPosition(true);
	PT_DocPosition posEnd = posStart + static_cast<PT_DocPosition>(pBlock->getLength());

	_createAndFillTOCEntry(posStart, posEnd, pPrevBL, sStyle.utf8_str(), iAllBlocks);
	
	// Tell the block it's shadowed in a TOC
	pBlock->setStyleInTOC(true);
}

UT_sint32 fl_TOCLayout::isInVector(fl_BlockLayout * pBlock, 
								   UT_GenericVector<TOCEntry *>* pVecEntries)
{
	TOCEntry * pThisEntry = NULL;
	fl_BlockLayout * pThisBL = NULL;
	UT_sint32 i = 0;
	for(i=0; i< pVecEntries->getItemCount(); i++)
	{

		pThisEntry = pVecEntries->getNthItem(i);
		pThisBL = pThisEntry->getBlock();
		if(pThisBL->getStruxDocHandle() == pBlock->getStruxDocHandle())
		{
			return i;
		}
	}
	return -1;
}


bool fl_TOCLayout::removeBlock(fl_BlockLayout * pBlock)
{
	if(m_bDoingPurge)
	{
		return true;
	}
	if(m_pLayout && m_pLayout->isLayoutDeleting())
	{
		return false;
	}
	if(isInVector(pBlock,&m_vecEntries) >= 0)
	{
		fp_TOCContainer * pTOC = static_cast<fp_TOCContainer *>(getFirstContainer());
		if(pTOC)
		{
			pTOC->clearScreen();
		}
		_removeBlockInVec(pBlock);
		_calculateLabels();
		return true;
	}
	return false;
}


fl_BlockLayout * fl_TOCLayout::findMatchingBlock(fl_BlockLayout * pBlock)
{
	TOCEntry * pThisEntry = NULL;
	fl_BlockLayout * pThisBL = NULL;
	UT_sint32 i = 0;
	bool bFound = false;
	for(i=0; i< m_vecEntries.getItemCount(); i++)
	{
		pThisEntry = m_vecEntries.getNthItem(i);
		pThisBL = pThisEntry->getBlock();
		if(pThisBL->getStruxDocHandle() == pBlock->getStruxDocHandle())
		{
			bFound = true;
			break;
		}
	}
	if(bFound)
	{
		return pThisBL;
	}
	return NULL;
}

/*
    When a block is deleted from the TOC, it is sometimes necessary to redo the toc entry that
    preceded it. We do this be removing and recreating this previous entry, which requires a
    recursive call to ourselves. bDontRecurse indicates that the recursive processing should not be
    done; it has a default value false.
*/
void fl_TOCLayout::_removeBlockInVec(fl_BlockLayout * pBlock, bool /*bDontRecurse*/)
{
	TOCEntry * pThisEntry = NULL;
	fl_BlockLayout * pThisBL = NULL;
	UT_sint32 i = 0;
	bool bFound = false;
	for(i=0; i< m_vecEntries.getItemCount(); i++)
	{
		pThisEntry = m_vecEntries.getNthItem(i);
		pThisBL = pThisEntry->getBlock();
		if(pThisBL->getStruxDocHandle() == pBlock->getStruxDocHandle())
		{
			bFound = true;
			break;
		}
	}
	if(!bFound)
	{
		return;
	}
	//
	// Clear it!
	//
	UT_DEBUGMSG(("Removing block %p Entry %p \n",pThisBL,pThisEntry));
	if(!pBlock->isContainedByTOC())
	{
		// we only clear if the block passed to us is not one of our TOC blocks (i.e., if we are not
		// called recursively by this funciton, or by _addBlockInVec())
		pBlock->clearScreen(m_pLayout->getGraphics());
	}
	//
	// unlink it from the TOCLayout
	//
	
	if(static_cast<fl_BlockLayout *>(getFirstLayout()) == pThisBL)
	{
		setFirstLayout(pThisBL->getNext());
	}
	if(static_cast<fl_BlockLayout *>(getLastLayout()) == pThisBL)
	{
		setLastLayout(pThisBL->getPrev());
	}
	if(pThisBL->getPrev())
	{
		pThisBL->getPrev()->setNext(pThisBL->getNext());
	}
	if(pThisBL->getNext())
	{
		pThisBL->getNext()->setPrev(pThisBL->getPrev());
	}
//
// Remove entry
//
	UT_sint32 k = m_vecEntries.findItem(pThisEntry);
	i = k-1;
	UT_ASSERT(k >= 0);
	while(k >= 0)
	{
		m_vecEntries.deleteNthItem(k);
		k = m_vecEntries.findItem(pThisEntry);
		UT_ASSERT(k== -1);
	}

	delete pThisBL;
	delete pThisEntry;
//
// Used to have code to remove the previous block if it touched this
// block. Remove it and rely on fl_blocklayout to handle the case of
// text from a previous block coming into this block
//
	markAllRunsDirty();
	setNeedsReformat(0);
	setNeedsRedraw();

}

UT_sint32 fl_TOCLayout::_getStartValue(TOCEntry * pEntry)
{
	if(pEntry->getLevel() == 1)
	{
		return m_iStartAt1;
	}
	else if(pEntry->getLevel() == 2)
	{
		return m_iStartAt2;
	}
	else if(pEntry->getLevel() == 3)
	{
		return m_iStartAt3;
	}
	else
	{
		return m_iStartAt4;
	}
}

void fl_TOCLayout::_calculateLabels(void)
{
	UT_sint32 i = 0;
	TOCEntry * pThisEntry = NULL;
	TOCEntry * pPrevEntry = NULL;
	UT_Stack stEntry;
	stEntry.push(NULL);
	UT_sint32 iCount = m_vecEntries.getItemCount();
	if(iCount == 0)
	{
		return;
	}
	pThisEntry = m_vecEntries.getNthItem(0);
	stEntry.push(pThisEntry);
	for(i=0; i<	iCount; i++)
	{
		if(pPrevEntry == NULL)
		{
			pThisEntry->setPosInList(_getStartValue(pThisEntry));
			pThisEntry->calculateLabel(pPrevEntry);
			pPrevEntry = pThisEntry;
			continue;
		}
		pThisEntry = m_vecEntries.getNthItem(i);
		UT_ASSERT(pThisEntry->getLevel() >= 0);
		if(pThisEntry->getLevel() == pPrevEntry->getLevel())
		{
			pThisEntry->setPosInList(pPrevEntry->getPosInList()+1);
			void * pTmp = NULL;
			UT_ASSERT(stEntry.getDepth() > 0);
			stEntry.viewTop(&pTmp);
			TOCEntry * pPrevLevel = static_cast<TOCEntry*>(pTmp);
			if(pPrevLevel && pPrevLevel->getLevel() < pThisEntry->getLevel())
			{
				UT_ASSERT(pPrevLevel->getLevel() >= 0);
				pThisEntry->calculateLabel(pPrevLevel);
			}
			else
			{
				pThisEntry->calculateLabel(NULL);
			}
			pPrevEntry = pThisEntry;
		}
		else if(pThisEntry->getLevel() > pPrevEntry->getLevel())
		{
			stEntry.push(pPrevEntry);
			pThisEntry->setPosInList(_getStartValue(pThisEntry));
			pThisEntry->calculateLabel(pPrevEntry);
			pPrevEntry = pThisEntry;
		}
		else
		{
			bool bStop = false;
			while((stEntry.getDepth()>1) && !bStop)
			{
				void * pTmp;
				UT_ASSERT(stEntry.getDepth() > 0);
				stEntry.pop(&pTmp);
				pPrevEntry = static_cast<TOCEntry*>(pTmp);
				if(pPrevEntry->getLevel() == pThisEntry->getLevel())
				{
					bStop = true;
				}
			}
			if(bStop)
			{
				pThisEntry->setPosInList(pPrevEntry->getPosInList()+1);
				void * pTmp;
				UT_ASSERT(stEntry.getDepth() > 0);
				stEntry.viewTop(&pTmp);
				TOCEntry * pPrevLevel = static_cast<TOCEntry*>(pTmp);
				if(pPrevLevel && pPrevLevel->getLevel() < pThisEntry->getLevel())
				{
					pThisEntry->calculateLabel(pPrevLevel);
				}
				else
				{
					pThisEntry->calculateLabel(NULL);
				}
				pPrevEntry = pThisEntry;
			}
			else
			{
				pThisEntry->setPosInList(_getStartValue(pThisEntry));
				pPrevEntry = pThisEntry;
				pThisEntry->calculateLabel(NULL);
			}
		}
	}
}

bool fl_TOCLayout::isStyleInTOC(UT_UTF8String & sStyle)
{
	if(_isStyleInTOC(sStyle,m_sSourceStyle1))
	{
		return true;
	}
	if(_isStyleInTOC(sStyle,m_sSourceStyle2))
	{
		return true;
	}
	if(_isStyleInTOC(sStyle,m_sSourceStyle3))
	{
		return true;
	}
	if(_isStyleInTOC(sStyle,m_sSourceStyle4))
	{
		return true;
	}
	return false;
}

/*!
 * Does a case insensitive search of the basedon heirachy for a match.
 */
bool fl_TOCLayout::_isStyleInTOC(UT_UTF8String & sStyle, UT_UTF8String & sTOCStyle)
{
	UT_UTF8String sTmpStyle = sStyle;
	const char * sLStyle = sTOCStyle.utf8_str();
	xxx_UT_DEBUGMSG(("Looking at TOC Style %s \n",sLStyle));
	xxx_UT_DEBUGMSG(("Base input style is %s \n",sTmpStyle.utf8_str()));
	if(g_ascii_strcasecmp(sLStyle,sTmpStyle.utf8_str()) == 0)
	{
		xxx_UT_DEBUGMSG(("Found initial match \n"));
		return true;
	}
	PD_Style * pStyle = NULL;
	m_pDoc->getStyle(sTmpStyle.utf8_str(), &pStyle);
	if(pStyle != NULL)
	{
		UT_sint32 iLoop = 0;
		while((pStyle->getBasedOn()) != NULL && (iLoop < 10))
		{
			pStyle = pStyle->getBasedOn();
			iLoop++;
			sTmpStyle = pStyle->getName();
			xxx_UT_DEBUGMSG(("Level %d style is %s \n",iLoop,sTmpStyle.utf8_str()));
			if(g_ascii_strcasecmp(sLStyle,sTmpStyle.utf8_str()) == 0)
			{
				return true;
			}
		}
	}
	xxx_UT_DEBUGMSG(("No match Found \n"));
	return false;
}


bool fl_TOCLayout::isBlockInTOC(fl_BlockLayout * pBlock)
{
	TOCEntry * pEntry = NULL;
	pf_Frag_Strux* sdh = pBlock->getStruxDocHandle();
	UT_sint32 i = 0;
	for(i=0; i< m_vecEntries.getItemCount(); i++)
	{

		pEntry = m_vecEntries.getNthItem(i);
		fl_BlockLayout *pBL = pEntry->getBlock();
		if(pBL->getStruxDocHandle() == sdh)
		{
			return true;
		}
	}
	return false;
}


UT_UTF8String & fl_TOCLayout::getTOCListLabel(fl_BlockLayout * pBlock)
{
	static UT_UTF8String str;
	str.clear();
	TOCEntry * pEntry = NULL;
	pf_Frag_Strux* sdh = pBlock->getStruxDocHandle();
	UT_sint32 i = 0;
	bool bFound = false;
	for(i=0; i< m_vecEntries.getItemCount(); i++)
	{

		pEntry = m_vecEntries.getNthItem(i);
		fl_BlockLayout *pBL = pEntry->getBlock();
		if(pBL->getStruxDocHandle() == sdh)
		{
			bFound = true;
			break;
		}
	}
	if(!bFound)
	{
		return str;
	}
	str = pEntry->getFullLabel();
	return str;
}

fl_BlockLayout * fl_TOCLayout::getMatchingBlock(fl_BlockLayout * pBlock)
{
	return findMatchingBlock(pBlock);
}

bool fl_TOCLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);


	setAttrPropIndex(pcrxc->getIndexAP());
	fp_Page * pPage = getFirstContainer()->getPage();
	collapse();
	lookupProperties();
	_createTOCContainer();
	_insertTOCContainer(static_cast<fp_TOCContainer *>(getLastContainer()));
	fl_DocSectionLayout * pDSL = getDocSectionLayout();
	pDSL->setNeedsSectionBreak(true,pPage);
	return true;
}


bool fl_TOCLayout::recalculateFields(UT_uint32 iUpdateCount)
{

	bool bResult = false;
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		bResult = pBL->recalculateFields(iUpdateCount) || bResult;
		pBL = pBL->getNext();
	}
	return bResult;
}


void fl_TOCLayout::markAllRunsDirty(void)
{
	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL)
	{
		pCL->markAllRunsDirty();
		pCL = pCL->getNext();
	}
}

void fl_TOCLayout::updateLayout(bool /*bDoAll*/)
{
	if(needsReformat())
	{
		format();
	}
	m_vecFormatLayout.clear();
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		if (pBL->needsReformat())
		{
			pBL->format();
		}

		pBL = pBL->getNext();
	}
}

void fl_TOCLayout::redrawUpdate(void)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		if (pBL->needsRedraw())
		{
			pBL->redrawUpdate();
		}

		pBL = pBL->getNext();
	}
}


bool fl_TOCLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_UNUSED(pcrx);
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
//
// Remove all remaining structures
//
	fp_Page * pPage = getFirstContainer()->getPage();
	collapse();
//	UT_ASSERT(pcrx->getStruxType()== PTX_SectionTOC);
//
	fl_DocSectionLayout * pDSL = getDocSectionLayout();
	myContainingLayout()->remove(this);
	UT_sint32 iPage = getDocLayout()->findPage(pPage);
	if(iPage >= 0)
	{
		pDSL->setNeedsSectionBreak(true,pPage);
	}
	else
	{
		pDSL->setNeedsSectionBreak(true,NULL);
	}
	delete this;			// TODO whoa!  this construct is VERY dangerous.

	return true;
}

TOCEntry * fl_TOCLayout::createNewEntry(fl_BlockLayout * pNewBL)
{
	UT_UTF8String sDispStyle("");
	bool bHaveLabel = true;
	FootnoteType iFType = FOOTNOTE_TYPE_NUMERIC; 
	UT_UTF8String sBefore;
	UT_UTF8String sAfter; 
	bool bInherit = false; 	
	UT_sint32 iStartAt = 0;
	if(m_iCurrentLevel == 1)
	{
		sDispStyle = m_sDestStyle1;
		bHaveLabel = m_bHasLabel1;
		iFType = m_iLabType1;
		sBefore = m_sLabBefore1;
		sAfter = m_sLabAfter1;
		bInherit = m_bInherit1;
		iStartAt = m_iStartAt1;
	}
	else if( m_iCurrentLevel == 2)
	{
		sDispStyle = m_sDestStyle2;
		bHaveLabel = m_bHasLabel2;
		iFType = m_iLabType2;
		sBefore = m_sLabBefore2;
		sAfter = m_sLabAfter2;
		bInherit = m_bInherit2;
		iStartAt = m_iStartAt2;
	}
	else if( m_iCurrentLevel == 3)
	{
		sDispStyle = m_sDestStyle3;
		bHaveLabel = m_bHasLabel3;
		iFType = m_iLabType3;
		sBefore = m_sLabBefore3;
		sAfter = m_sLabAfter3;
		bInherit = m_bInherit3;
		iStartAt = m_iStartAt3;
	}
	else if( m_iCurrentLevel == 4)
	{
		sDispStyle = m_sDestStyle4;
		bHaveLabel = m_bHasLabel4;
		iFType = m_iLabType4;
		sBefore = m_sLabBefore4;
		sAfter = m_sLabAfter4;
		bInherit = m_bInherit4;
		iStartAt = m_iStartAt4;
	}
	else {
		UT_ASSERT_NOT_REACHED();
	}
	TOCEntry * pNew = new TOCEntry(pNewBL,m_iCurrentLevel,
								   sDispStyle,
								   bHaveLabel,
								   iFType,
								   sBefore,
								   sAfter,
								   bInherit,
								   iStartAt);
	return pNew;
}

/*!
 * This method removes all layout structures contained by this layout
 * structure.
 */
void fl_TOCLayout::_purgeLayout(void)
{
	UT_DEBUGMSG(("TOCLayout: purge \n"));
	fl_ContainerLayout * pCL = getFirstLayout();
	m_bDoingPurge = true;
	while(pCL)
	{
		fl_ContainerLayout * pNext = pCL->getNext();
		delete pCL;
		pCL = pNext;
	}
	UT_VECTOR_PURGEALL(TOCEntry *, m_vecEntries);
	m_vecEntries.clear();
	m_bDoingPurge = false;
	setFirstLayout(NULL);
	setLastLayout(NULL);
}


/*!
 * This method creates a new TOC.
 */
void fl_TOCLayout::_createTOCContainer(void)
{
	lookupProperties();
	UT_ASSERT(getFirstLayout() == NULL);
	fp_TOCContainer * pTOCContainer = new fp_TOCContainer(static_cast<fl_SectionLayout *>(this));
	setFirstContainer(pTOCContainer);
	setLastContainer(pTOCContainer);
	fl_ContainerLayout * pCL = myContainingLayout();
	while(pCL!= NULL && pCL->getContainerType() != FL_CONTAINER_DOCSECTION)
	{
		pCL = pCL->myContainingLayout();
	}
	UT_ASSERT(pCL);

	fp_Container * pCon = pCL->getLastContainer();
	UT_ASSERT(pCon);
	UT_sint32 iWidth = pCon->getWidth();
	pTOCContainer->setWidth(iWidth);
	if(m_bHasEndTOC)
	{
	    fillTOC();
	}
}

/*!
  Create a new TOC container.
  \return The newly created TOC container
*/
fp_Container* fl_TOCLayout::getNewContainer(fp_Container *)
{
	UT_DEBUGMSG(("creating new TOC Physical container\n"));
	_createTOCContainer();
	_insertTOCContainer(static_cast<fp_TOCContainer *>(getLastContainer()));
	return static_cast<fp_Container *>(getLastContainer());
}


/*!
 * This method inserts the given TOCContainer into its correct place in the
 * Vertical container.
 */
void fl_TOCLayout::_insertTOCContainer( fp_TOCContainer * pNewTOC)
{
	fl_ContainerLayout * pUPCL = myContainingLayout();
	fl_ContainerLayout * pPrevL = static_cast<fl_ContainerLayout *>(getPrev());
	fp_Container * pPrevCon = NULL;
	fp_Container * pUpCon = NULL;
	if(pPrevL != NULL)
	{
		while(pPrevL && ((pPrevL->getContainerType() == FL_CONTAINER_FOOTNOTE) || pPrevL->getContainerType() == FL_CONTAINER_ENDNOTE))
		{
			pPrevL = pPrevL->getPrev();
		}
		if(pPrevL)
		{
			if(pPrevL->getContainerType() == FL_CONTAINER_TABLE)
			{
//
// Handle if prev container is table that is broken across a page
//
				fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pPrevL);
				fp_TableContainer * pTC = static_cast<fp_TableContainer *>(pTL->getFirstContainer());
				fp_TableContainer * pFirst = pTC->getFirstBrokenTable();
				fp_TableContainer * pLast = pTC->getLastBrokenTable();
				if((pLast != NULL) && pLast != pFirst)
				{
					pPrevCon = static_cast<fp_Container *>(pLast);
					pUpCon = pLast->getContainer();
				}
				else
				{
					pPrevCon = pPrevL->getLastContainer();
					pUpCon = pPrevCon->getContainer();
				}
			}
			else
			{
				pPrevCon = pPrevL->getLastContainer();
				if(pPrevCon)
				{
					pUpCon = pPrevCon->getContainer();
				}
				else if(pPrevL->getPrev() == NULL)
				{
					pUpCon = myContainingLayout()->getFirstContainer();
				}
				else
				{
					pUpCon = myContainingLayout()->getFirstContainer();
				}
			}
		}
		else
		{
			pUpCon = pUPCL->getLastContainer();
		}
		UT_return_if_fail(pUpCon);
	}
	else
	{
		pUpCon = pUPCL->getLastContainer();
		UT_return_if_fail(pUpCon);
	}
	if(pPrevL == NULL)
	{
		xxx_UT_DEBUGMSG(("SEVIOR!!!!!!!!!! New TOC %x added into %x \n",pNewTOC,pUpCon));
		pUpCon->addCon(pNewTOC);
		pNewTOC->setContainer(pUpCon);
;
	}
	else
	{
		UT_sint32 i = pUpCon->findCon(pPrevCon);
		xxx_UT_DEBUGMSG(("SEVIOR!!!!!!!!!! New TOC %x inserted into %x \n",pNewTOC,pUpCon));
		if(i >= 0 && (i+1) < pUpCon->countCons())
		{
			pUpCon->insertConAt(pNewTOC,i+1);
			pNewTOC->setContainer(pUpCon);
		}
		else if( i >=0 &&  (i+ 1) == pUpCon->countCons())
		{
			pUpCon->addCon(pNewTOC);
			pNewTOC->setContainer(pUpCon);
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}
}


void fl_TOCLayout::format(void)
{
	xxx_UT_DEBUGMSG(("SEVIOR: Formatting TOC container is %x \n",getFirstContainer()));
	if(getFirstContainer() == NULL)
	{
		getNewContainer();
	}
	fl_ContainerLayout*	pBL = getFirstLayout();
	
	while (pBL)
	{
		pBL->format();
		UT_sint32 count = 0;
		while(pBL->getLastContainer() == NULL || pBL->getFirstContainer()==NULL)
		{
			UT_DEBUGMSG(("Error formatting a block try again \n"));
			count = count + 1;
			pBL->format();
			if(count > 3)
			{
				UT_DEBUGMSG(("Give up trying to format. Hope for the best :-( \n"));
				break;
			}
		}
		pBL = pBL->getNext();
	}
	static_cast<fp_TOCContainer *>(getFirstContainer())->layout();
	m_bNeedsFormat = false;
	m_bNeedsReformat = false;
}

std::string fl_TOCLayout::getDefaultHeading()
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	std::string sDefaultHeading;
	pSS->getValueUTF8(AP_STRING_ID_TOC_TocHeading, sDefaultHeading);
	return sDefaultHeading;
}

UT_UTF8String fl_TOCLayout::getDefaultSourceStyle(UT_uint32 iLevel)
{
	// fetch the default TOC destination style from the buildin defaults
	UT_UTF8String sStyle = UT_UTF8String_sprintf("toc-source-style%d", iLevel);
	const PP_Property* pProp = PP_lookupProperty(sStyle.utf8_str());
	if (pProp)
		return pProp->getInitial();
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);

	// we're desperate, just use something, anything...
	return UT_UTF8String_sprintf("Heading %d", iLevel);
}

UT_UTF8String fl_TOCLayout::getDefaultDestStyle(UT_uint32 iLevel)
{
	// fetch the default TOC destination style from the buildin defaults
	UT_UTF8String sStyle = UT_UTF8String_sprintf("toc-dest-style%d", iLevel);
	const PP_Property* pProp = PP_lookupProperty(sStyle.utf8_str());
	if (pProp)
		return pProp->getInitial();
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);

	// we're desperate, just use something, anything...
	return UT_UTF8String_sprintf("Contents %d", iLevel);
}

/*!
    this function is only to be called by fl_ContainerLayout::lookupProperties()
    all other code must call lookupProperties() instead
*/
void fl_TOCLayout::_lookupProperties(const PP_AttrProp* pSectionAP)
{
	UT_return_if_fail(pSectionAP);
	
	// I can't think of any properties we need for now.
	// If we need any later, we'll add them. -PL
	const gchar *pszTOCPID = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-id",pszTOCPID))
	{
		m_iTOCPID = 0;
	}
	else
	{
		m_iTOCPID = atoi(pszTOCPID);
	}

	m_sNumOff1 = "0.5in";
	m_sNumOff2 = "0.5in";
	m_sNumOff3 = "0.5in";
	m_sNumOff4 = "0.5in";


	const gchar *pszINDENT = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-indent1",pszINDENT))
	{
		m_sNumOff1 = "0.5in";
	}
	else
	{
		m_sNumOff1 = pszINDENT;
	}
	pszINDENT = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-indent2",pszINDENT))
	{
		m_sNumOff2 = "0.5in";
	}
	else
	{
		m_sNumOff2 = pszINDENT;
	}

	pszINDENT = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-indent3",pszINDENT))
	{
		m_sNumOff3 = "0.5in";
	}
	else
	{
		m_sNumOff3 = pszINDENT;
	}

	pszINDENT = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-indent4",pszINDENT))
	{
		m_sNumOff4 = "0.5in";
	}
	else
	{
		m_sNumOff4 = pszINDENT;
	}

	const gchar *pszTOCSRC = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-source-style1",pszTOCSRC))
	{
		m_sSourceStyle1 = getDefaultSourceStyle(1);
	}
	else
	{
		m_sSourceStyle1 = pszTOCSRC;
	}
	pszTOCSRC = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-source-style2",pszTOCSRC))
	{
		m_sSourceStyle2 = getDefaultSourceStyle(2);
	}
	else
	{
		m_sSourceStyle2 = pszTOCSRC;
	}
	pszTOCSRC = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-source-style3",pszTOCSRC))
	{
		m_sSourceStyle3 = getDefaultSourceStyle(3);
	}
	else
	{
		m_sSourceStyle3 = pszTOCSRC;
	}
	pszTOCSRC = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-source-style4",pszTOCSRC))
	{
		m_sSourceStyle4 = getDefaultSourceStyle(4);
	}
	else
	{
		m_sSourceStyle4 = pszTOCSRC;
	}
	const gchar * pszTOCDEST = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-dest-style1",pszTOCDEST))
	{
		m_sDestStyle1 = getDefaultDestStyle(1);
	}
	else
	{
		m_sDestStyle1 = pszTOCDEST;
	}
	pszTOCDEST = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-dest-style2",pszTOCDEST))
	{
		m_sDestStyle2 = getDefaultDestStyle(2);
	}
	else
	{
		m_sDestStyle2 = pszTOCDEST;
	}
	pszTOCDEST = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-dest-style3",pszTOCDEST))
	{
		m_sDestStyle3 = getDefaultDestStyle(3);
	}
	else
	{
		m_sDestStyle3 = pszTOCDEST;
	}
	pszTOCDEST = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-dest-style4",pszTOCDEST))
	{
		m_sDestStyle4 = getDefaultDestStyle(4);
	}
	else
	{
		m_sDestStyle4 = pszTOCDEST;
	}

	const gchar * pszTOCHEADING = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-heading",pszTOCHEADING))
	{
		m_sTOCHeading = getDefaultHeading();
	}
	else
	{
		m_sTOCHeading = pszTOCHEADING;
	}

	const gchar * pszTOCHEADINGStyle = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-heading-style",pszTOCHEADINGStyle))
	{
		m_sTOCHeadingStyle = "Contents Header";
	}
	else
	{
		m_sTOCHeadingStyle = pszTOCHEADINGStyle;
	}


	const gchar * pszTOCHASHEADING = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-has-heading",pszTOCHASHEADING))
	{
		m_bTOCHeading = true;
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCHASHEADING,"1") == 0)
		{
			m_bTOCHeading = true;
		}
		else
		{
			m_bTOCHeading = false;
		}
	}
//
// TOC Label
//
	const gchar * pszTOCLABEL = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-has-label1",pszTOCLABEL))
	{
		m_bHasLabel1 = true;
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABEL,"1") == 0)
		{
			m_bHasLabel1 = true;
		}
		else
		{
			m_bHasLabel1 = false;
		}
	}
	pszTOCLABEL = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-has-label2",pszTOCLABEL))
	{
		m_bHasLabel2 = true;
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABEL,"1") == 0)
		{
			m_bHasLabel2 = true;
		}
		else
		{
			m_bHasLabel2 = false;
		}
	}
	pszTOCLABEL = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-has-label3",pszTOCLABEL))
	{
		m_bHasLabel3 = true;
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABEL,"1") == 0)
		{
			m_bHasLabel3 = true;
		}
		else
		{
			m_bHasLabel3 = false;
		}
	}
	pszTOCLABEL = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-has-label4",pszTOCLABEL))
	{
		m_bHasLabel4 = true;
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABEL,"1") == 0)
		{
			m_bHasLabel4 = true;
		}
		else
		{
			m_bHasLabel4 = false;
		}
	}
//
// TOC Label Inherits
//
	const gchar * pszTOCLABELINHERITS = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-inherits1",pszTOCLABELINHERITS))
	{
		m_bInherit1 = true;
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABELINHERITS,"1") == 0)
		{
			m_bInherit1 = true;
		}
		else
		{
			m_bInherit1 = false;
		}
	}
	pszTOCLABELINHERITS = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-inherits2",pszTOCLABELINHERITS))
	{
		m_bInherit2 = true;
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABELINHERITS,"1") == 0)
		{
			m_bInherit2 = true;
		}
		else
		{
			m_bInherit2 = false;
		}
	}
	pszTOCLABELINHERITS = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-inherits3",pszTOCLABELINHERITS))
	{
		m_bInherit3 = true;
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABELINHERITS,"1") == 0)
		{
			m_bInherit3 = true;
		}
		else
		{
			m_bInherit3 = false;
		}
	}
	pszTOCLABELINHERITS = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-inherits4",pszTOCLABELINHERITS))
	{
		m_bInherit4 = true;
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCLABELINHERITS,"1") == 0)
		{
			m_bInherit4 = true;
		}
		else
		{
			m_bInherit4 = false;
		}
	}
//
// TOC Label Type
//
	const gchar * pszTOCLABELTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-type1",pszTOCLABELTYPE))
	{
		m_iLabType1 = FOOTNOTE_TYPE_NUMERIC;
	}
	else
	{
		m_iLabType1 = m_pLayout->FootnoteTypeFromString(pszTOCLABELTYPE);
	}
	pszTOCLABELTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-type2",pszTOCLABELTYPE))
	{
		m_iLabType2 = FOOTNOTE_TYPE_NUMERIC;
	}
	else
	{
		m_iLabType2 = m_pLayout->FootnoteTypeFromString(pszTOCLABELTYPE);
	}
	pszTOCLABELTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-type3",pszTOCLABELTYPE))
	{
		m_iLabType3 = FOOTNOTE_TYPE_NUMERIC;
	}
	else
	{
		m_iLabType3 = m_pLayout->FootnoteTypeFromString(pszTOCLABELTYPE);
	}
	pszTOCLABELTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-type4",pszTOCLABELTYPE))
	{
		m_iLabType4 = FOOTNOTE_TYPE_NUMERIC;
	}
	else
	{
		m_iLabType4 = m_pLayout->FootnoteTypeFromString(pszTOCLABELTYPE);
	}
//
// TOC Label Before Text
//
	const gchar * pszTOCSTRBEFORE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-before1",pszTOCSTRBEFORE))
	{
		m_sLabBefore1 = "";
	}
	else
	{
		m_sLabBefore1 = pszTOCSTRBEFORE;
	}
	pszTOCSTRBEFORE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-before2",pszTOCSTRBEFORE))
	{
		m_sLabBefore2 = "";
	}
	else
	{
		m_sLabBefore2 = pszTOCSTRBEFORE;
	}
	pszTOCSTRBEFORE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-before3",pszTOCSTRBEFORE))
	{
		m_sLabBefore3 = "";
	}
	else
	{
		m_sLabBefore3 = pszTOCSTRBEFORE;
	}
	pszTOCSTRBEFORE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-before4",pszTOCSTRBEFORE))
	{
		m_sLabBefore4 = "";
	}
	else
	{
		m_sLabBefore4 = pszTOCSTRBEFORE;
	}
//
// TOC Label After Text
//
	const gchar * pszTOCSTRAFTER = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-after1",pszTOCSTRAFTER))
	{
		m_sLabAfter1 = "";
	}
	else
	{
		m_sLabAfter1 = pszTOCSTRAFTER;
	}
	pszTOCSTRAFTER = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-after2",pszTOCSTRAFTER))
	{
		m_sLabAfter2 = "";
	}
	else
	{
		m_sLabAfter2 = pszTOCSTRAFTER;
	}
	pszTOCSTRAFTER = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-after2",pszTOCSTRAFTER))
	{
		m_sLabAfter2 = "";
	}
	else
	{
		m_sLabAfter3 = pszTOCSTRAFTER;
	}
	pszTOCSTRAFTER = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-after4",pszTOCSTRAFTER))
	{
		m_sLabAfter4 = "";
	}
	else
	{
		m_sLabAfter4 = pszTOCSTRAFTER;
	}
//
// TOC Label Initial Value
//
	const gchar * pszTOCLABELSTART = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-start1",pszTOCLABELSTART))
	{
		m_iStartAt1 = 1;
	}
	else
	{
		m_iStartAt1 = atoi(pszTOCLABELSTART);
	}
	pszTOCLABELSTART = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-start2",pszTOCLABELSTART))
	{
		m_iStartAt2 = 1;
	}
	else
	{
		m_iStartAt2 = atoi(pszTOCLABELSTART);
	}
	pszTOCLABELSTART = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-start3",pszTOCLABELSTART))
	{
		m_iStartAt3 = 1;
	}
	else
	{
		m_iStartAt3 = atoi(pszTOCLABELSTART);
	}
	pszTOCLABELSTART = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-label-start4",pszTOCLABELSTART))
	{
		m_iStartAt4 = 1;
	}
	else
	{
		m_iStartAt4 = atoi(pszTOCLABELSTART);
	}
//
// TOC Page Number Type
//
	const gchar * pszTOCPAGETYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-page-type1",pszTOCPAGETYPE))
	{
		m_iNumType1 = FOOTNOTE_TYPE_NUMERIC;
	}
	else
	{
		m_iNumType1 = m_pLayout->FootnoteTypeFromString(pszTOCPAGETYPE);
	}
	pszTOCPAGETYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-page-type2",pszTOCPAGETYPE))
	{
		m_iNumType2 = FOOTNOTE_TYPE_NUMERIC;
	}
	else
	{
		m_iNumType2 = m_pLayout->FootnoteTypeFromString(pszTOCPAGETYPE);
	}
	pszTOCPAGETYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-page-type3",pszTOCPAGETYPE))
	{
		m_iNumType3 = FOOTNOTE_TYPE_NUMERIC;
	}
	else
	{
		m_iNumType3 = m_pLayout->FootnoteTypeFromString(pszTOCPAGETYPE);
	}
	pszTOCPAGETYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-page-type4",pszTOCPAGETYPE))
	{
		m_iNumType4 = FOOTNOTE_TYPE_NUMERIC;
	}
	else
	{
		m_iNumType4 = m_pLayout->FootnoteTypeFromString(pszTOCPAGETYPE);
	}
//
// TOC TAB leader
//
	const gchar * pszTOCTABTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-tab-leader1",pszTOCTABTYPE))
	{
		m_iTabLeader1 = FL_LEADER_DOT;
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCTABTYPE,"none") == 0)
		{
			m_iTabLeader1 = FL_LEADER_NONE;
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"dot") == 0)
		{
			m_iTabLeader1 = FL_LEADER_DOT;
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"hyphen") == 0)
		{
			m_iTabLeader1 = FL_LEADER_HYPHEN;
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"underline") == 0)
		{
			m_iTabLeader1 = FL_LEADER_UNDERLINE;
		}
		else
		{
			m_iTabLeader1 = FL_LEADER_DOT;
		}
	}
	pszTOCTABTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-tab-leader2",pszTOCTABTYPE))
	{
		m_iTabLeader2 = FL_LEADER_DOT;
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCTABTYPE,"none") == 0)
		{
			m_iTabLeader2 = FL_LEADER_NONE;
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"dot") == 0)
		{
			m_iTabLeader2 = FL_LEADER_DOT;
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"hyphen") == 0)
		{
			m_iTabLeader2 = FL_LEADER_HYPHEN;
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"underline") == 0)
		{
			m_iTabLeader2 = FL_LEADER_UNDERLINE;
		}
		else
		{
			m_iTabLeader2 = FL_LEADER_DOT;
		}
	}
	pszTOCTABTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-tab-leader3",pszTOCTABTYPE))
	{
		m_iTabLeader3 = FL_LEADER_DOT;
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCTABTYPE,"none") == 0)
		{
			m_iTabLeader3 = FL_LEADER_NONE;
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"dot") == 0)
		{
			m_iTabLeader3 = FL_LEADER_DOT;
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"hyphen") == 0)
		{
			m_iTabLeader3 = FL_LEADER_HYPHEN;
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"underline") == 0)
		{
			m_iTabLeader3 = FL_LEADER_UNDERLINE;
		}
		else
		{
			m_iTabLeader3 = FL_LEADER_DOT;
		}
	}
	pszTOCTABTYPE = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("toc-tab-leader4",pszTOCTABTYPE))
	{
		m_iTabLeader4 = FL_LEADER_DOT;
	}
	else
	{
		if(g_ascii_strcasecmp(pszTOCTABTYPE,"none") == 0)
		{
			m_iTabLeader4 = FL_LEADER_NONE;
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"dot") == 0)
		{
			m_iTabLeader4 = FL_LEADER_DOT;
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"hyphen") == 0)
		{
			m_iTabLeader4 = FL_LEADER_HYPHEN;
		}
		else if(g_ascii_strcasecmp(pszTOCTABTYPE,"underline") == 0)
		{
			m_iTabLeader4 = FL_LEADER_UNDERLINE;
		}
		else
		{
			m_iTabLeader4 = FL_LEADER_DOT;
		}
	}

	pszTOCTABTYPE = NULL;
	if(pSectionAP && pSectionAP->getProperty("toc-range-bookmark",pszTOCTABTYPE))
	{
		m_sRangeBookmark = pszTOCTABTYPE;
	}
	else
	{
		m_sRangeBookmark.clear();
	}
}

void fl_TOCLayout::_localCollapse(void)
{
	// ClearScreen on our Cell. One Cell per layout.
	fp_TOCContainer *pTC = static_cast<fp_TOCContainer *>(getFirstContainer());
	if (pTC)
	{
		pTC->clearScreen();
	}

	// get rid of all the layout information for every containerLayout
	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL)
	{
		pCL->collapse();
		pCL = pCL->getNext();
	}
	m_bNeedsReformat = true;
}

void fl_TOCLayout::collapse(void)
{
	fp_TOCContainer *pTC = static_cast<fp_TOCContainer *>(getFirstContainer());
	if(pTC)
	{
		fp_TOCContainer * pBroke = pTC->getFirstBrokenTOC();
		while(pBroke)
		{
			xxx_UT_DEBUGMSG(("DOing clearscreen on broken toc in collapse \n"));
			pBroke->clearScreen();
			pBroke = static_cast<fp_TOCContainer *>(pBroke->getNext());
		}
		pTC->deleteBrokenTOCs(true);
		pTC->clearScreen();
	}
	_localCollapse();
	if (pTC)
	{
//
// remove it from the linked list.
//
		fp_Container * pPrev = static_cast<fp_Container *>(pTC->getPrev());
		if(pPrev)
		{
			pPrev->setNext(pTC->getNext());
		}
		if(pTC->getNext())
		{
			pTC->getNext()->setPrev(pPrev);
		}
//
// Remove it from the vertical container that contains it.
//
		static_cast<fp_VerticalContainer *>(pTC->getContainer())->removeContainer(pTC);
		pTC->setContainer(NULL);
		delete pTC;
	}
	setFirstContainer(NULL);
	setLastContainer(NULL);
	_purgeLayout();
	setNeedsReformat(0);
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fl_TOCListener::fl_TOCListener(fl_TOCLayout* pTOCL, fl_BlockLayout* pPrevBL, PD_Style * pStyle)
{
	UT_ASSERT(pTOCL);

	m_pDoc = pTOCL->getDocLayout()->getDocument();
	m_pTOCL = pTOCL;
	m_pPrevBL = pPrevBL;
	m_bListening = false;
	m_pCurrentBL = NULL;
	m_pStyle = pStyle;
	// Mark this style as used, so it will be available in the list of used styles when
	// exporters need to export this TOC
	m_pStyle->used(1);
}

fl_TOCListener::~fl_TOCListener()
{
}

bool fl_TOCListener::populate(fl_ContainerLayout* sfh,
								 const PX_ChangeRecord * pcr)
{
	if (!m_bListening)
	{
		return true;
	}

	UT_ASSERT(m_pTOCL);

	bool bResult = false;
	//FV_View* pView = m_pTOCL->getDocLayout()->getView();
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
	{
		const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

		{
			UT_UNUSED(sfh);
			UT_ASSERT(static_cast<const fl_Layout *>(sfh)->getType() == PTX_Block);
			UT_ASSERT(m_pCurrentBL == (static_cast<const fl_ContainerLayout *>(sfh)));
		}
		PT_BlockOffset blockOffset = pcrs->getBlockOffset();
		UT_uint32 len = pcrs->getLength();


		bResult = static_cast<fl_BlockLayout *>(m_pCurrentBL)->doclistener_populateSpan(pcrs, blockOffset, len);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_InsertObject:
	{
		const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);

		{
			UT_ASSERT(static_cast<const fl_Layout *>(sfh)->getType() == PTX_Block);
			UT_ASSERT(m_pCurrentBL == (static_cast<const fl_ContainerLayout *>(sfh)));
		}
		PT_BlockOffset blockOffset = pcro->getBlockOffset();

// sterwill -- is this call to getSectionLayout() needed?  pBLSL is not used.

//			fl_SectionLayout* pBLSL = m_pCurrentBL->getSectionLayout();
		bResult = static_cast<fl_BlockLayout *>(m_pCurrentBL)->doclistener_populateObject(blockOffset,pcro);
		goto finish_up;
	}
	default:
		UT_DEBUGMSG(("TOCLayout: Unknown Change record = %d \n",pcr->getType()));
		return true;
	}

 finish_up:
	return bResult;
}

bool fl_TOCListener::populateStrux(pf_Frag_Strux* sdh,
									  const PX_ChangeRecord * pcr,
									  fl_ContainerLayout* * psfh)
{
	UT_ASSERT(m_pTOCL);

	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	PT_AttrPropIndex iTOC = m_pStyle->getIndexAP();
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	m_bListening = true;
	fl_BlockLayout * pMyBL = m_pPrevBL;
	if(pMyBL == NULL)
	{
		pMyBL = static_cast<fl_BlockLayout *>(m_pTOCL->getFirstLayout());
	}
	switch (pcrx->getStruxType())
	{
	case PTX_Block:
	{
		if (m_bListening)
		{
			// append a new BlockLayout to that SectionLayout
			fl_ContainerLayout*	pBL = m_pTOCL->insert(sdh,pMyBL, iTOC,FL_CONTAINER_BLOCK);
			if (!pBL)
			{
				UT_DEBUGMSG(("no memory for BlockLayout"));
				return false;
			}
			m_pCurrentBL = pBL;
			*psfh = pBL;
		}

	}
	break;

	default:
		UT_ASSERT(0);
		return false;
	}
	//
	// We're not printing
	//
	return true;
}

bool fl_TOCListener::change(fl_ContainerLayout* /*sfh*/,
							   const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}

bool fl_TOCListener::insertStrux(fl_ContainerLayout* /*sfh*/,
									const PX_ChangeRecord * /*pcr*/,
									pf_Frag_Strux* /*sdh*/,
									PL_ListenerId /*lid*/,
									void (* /*pfnBindHandles*/)(pf_Frag_Strux* sdhNew,
																PL_ListenerId lid,
																fl_ContainerLayout* sfhNew))
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}

bool fl_TOCListener::signal(UT_uint32 /*iSignal*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}


bool fl_TOCLayout::fillTOC(void)
{
    fl_DocSectionLayout * pDSL = getDocLayout()->getFirstSection();
    fl_BlockLayout * pBlock = NULL;
    fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pDSL);
    while(pCL && pCL->getContainerType() != FL_CONTAINER_BLOCK)
    {
	pCL = pCL->getFirstLayout();
    }
    if(pCL == NULL)
    {
	return false;
    }
    if(pCL->getContainerType() != FL_CONTAINER_BLOCK)
    {
	return false;
    }
    UT_UTF8String sStyle;
    pBlock = static_cast<fl_BlockLayout *>(pCL);
    bool filled = false;
    
    const gchar * pBookmark = getRangeBookmarkName().size() ? getRangeBookmarkName().utf8_str() : NULL;
    
    if(pBookmark)
    {
	if(m_pDoc->isBookmarkUnique(pBookmark))
	{
	    // bookmark does not exist
	    pBookmark = NULL;
	}
    }
    
    fl_BlockLayout * pBlockLast = NULL;
    
    if(pBookmark)
    {
	UT_uint32 i = 0;
	fp_BookmarkRun * pB[2] = {NULL,NULL};
	fp_Run * pRun;
	fl_BlockLayout * pBlockStart = pBlock;
	bool bFound = false;
	
	while(pBlock)
	{
	    pRun = pBlock->getFirstRun();
	    while(pRun)
	    {
		if(pRun->getType()== FPRUN_BOOKMARK)
		{
		    fp_BookmarkRun * pBR = static_cast<fp_BookmarkRun*>(pRun);
		    if(!strcmp(pBR->getName(),pBookmark))
		    {
			pB[i] = pBR;
			i++;
			if(i>1)
			{
			    bFound = true;
			    break;
			}
		    }
		}
		
		pRun = pRun->getNextRun();
	    }
	    
	    if(bFound)
	    {
		break;
	    }
	    pBlock = pBlock->getNextBlockInDocument();
	}
	
	if(pB[0] && pB[1])
	{
	    pBlockLast = pB[1]->getBlock();
	    
	    pBlock = pB[0]->getBlock();
	    PT_DocPosition pos1 = pB[0]->getBookmarkedDocPosition(false);
	    
	    if(pBlock->getPosition(true) < pos1)
	    {
		pBlock = pBlock->getNextBlockInDocument();
	    }
	}
	else
	{
	    UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
	    pBlock = pBlockStart;
	}
    }
    
    // clear any existing contents
    _purgeLayout();
    
    while(pBlock)
    {
	pBlock->getStyle(sStyle);
	if(isStyleInTOC(sStyle))
	{
	    filled = true;
	    addBlock(pBlock, false);
	}
	if(pBlockLast && pBlockLast == pBlock)
	{	    
	    break;
	}
	pBlock = pBlock->getNextBlockInDocument();
    }
    if(m_bTOCHeading)
    {
	PD_Style * pStyle = NULL;
	m_pDoc->getStyle(m_sTOCHeadingStyle.utf8_str(), &pStyle);
	if(pStyle == NULL)
	{
	    m_pDoc->getStyle("Heading 1", &pStyle);
	}
	PT_AttrPropIndex indexAP = pStyle->getIndexAP();
	
	fl_BlockLayout * pNewBlock = static_cast<fl_BlockLayout *>(insert(getStruxDocHandle(),NULL,
									  indexAP,FL_CONTAINER_BLOCK));
	pNewBlock->_doInsertTOCHeadingRun(0);
    }


    return filled;
}
