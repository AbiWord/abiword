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

#include <stdio.h>
#include <stdlib.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"
#include "px_CR_FmtMark.h"
//###TF#include "px_CR_Bookmark.h"
#include "px_CR_FmtMarkChange.h"
#include "px_CR_Glob.h"
#include "fv_View.h"
#include "fl_DocListener.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fl_BlockLayout.h"
#include "fl_ContainerLayout.h"
#include "fl_FootnoteLayout.h"
#include "fl_TableLayout.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "gr_Graphics.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "xap_Frame.h"

#define UPDATE_LAYOUT_ON_SIGNAL

static HdrFtrType s_convertToHdrFtrType(const XML_Char * pszHFType);

/*!
 Create DocListener
 \param doc Client of this DocListener
 \param pLayout Layout notified by this DocListener
*/
fl_DocListener::fl_DocListener(PD_Document* doc, FL_DocLayout *pLayout)
{
	m_pDoc = doc;
	m_pLayout = pLayout;
	if(pLayout->getGraphics() != NULL)
	{
		m_bScreen = pLayout->getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN);
	}
	else
	{
		m_bScreen = false;
	}
	m_iGlobCounter = 0;
	m_pCurrentSL = NULL;
//
// Put a NULL on th stack to signify the top.
//
	m_sLastContainerLayout.push(NULL);
	m_bFootnoteInProgress = false;
	m_bEndFootnoteProcessedInBlock = false;
}

/*!
 Destruct DocListener
*/
fl_DocListener::~fl_DocListener()
{
	UT_ASSERT(m_sLastContainerLayout.getDepth() == 1);
}

/*!
 */
bool fl_DocListener::populate(PL_StruxFmtHandle sfh,
							  const PX_ChangeRecord * pcr)
{
	UT_ASSERT(m_pLayout);
	UT_DEBUGMSG(("fl_DocListener::populate type %d \n",pcr->getType()));

	bool bResult = false;

	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
	{
		const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
//
// FIXME FIXME this should be removed after 4111 is debugged!!!
//
		if(pL->getType() != PTX_Block)
		{
			PT_BufIndex bi = pcrs->getBufIndex();
			const UT_UCSChar* pChars = m_pDoc->getPointer(bi);
			UT_uint32 i;
			UT_uint32 len = pcrs->getLength();
			for (i=0; i<len; i++)
			{
				UT_DEBUGMSG((" %c \n",(char) pChars[i]));
			}
			UT_ASSERT(pL->getType() == PTX_Block);
			return true;
		}
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		if(pCL->getPrev()!= NULL && pCL->getPrev()->getLastContainer()==NULL)
		{
			UT_DEBUGMSG(("In DocListner no LastLine in Previous Block Fixing this now \n"));
			UT_DEBUGMSG(("getPrev = %d this = %d \n",pCL->getPrev(),pCL));
			if( pCL->getSectionLayout()->getType() != FL_SECTION_HDRFTR)
				pCL->getPrev()->format();
		}

		PT_BlockOffset blockOffset = pcrs->getBlockOffset();
		UT_uint32 len = pcrs->getLength();
		fl_SectionLayout* pCLSL = pCL->getSectionLayout();
		if(pCLSL->getType() == FL_SECTION_SHADOW)
		{
			fl_HdrFtrSectionLayout * pHdr = static_cast<fl_HdrFtrShadow *>(pCLSL)->getHdrFtrSectionLayout();
			bResult = pHdr->bl_doclistener_populateSpan(pCL, pcrs, blockOffset, len);
		}
		else
			bResult = pCLSL->bl_doclistener_populateSpan(pCL, pcrs, blockOffset, len);
		if(pCL->getLastContainer()==NULL)
		{
			UT_DEBUGMSG(("In  DocListner no LastLine in this block fixing this now \n"));
			UT_DEBUGMSG(("getPrev = %d this = %d \n",pCL->getPrev(),pCL));
			if(pCL->getSectionLayout()->getType() != FL_SECTION_HDRFTR && pCL->getPrev()!= NULL)
				pCL->format();
			//UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}

		goto finish_up;
	}

	case PX_ChangeRecord::PXT_InsertObject:
	{
		const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		if(pL->getType() != PTX_Block)
		{
			m_pDoc->miniDump(pL->getStruxDocHandle(),8);
		}
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		PT_BlockOffset blockOffset = pcro->getBlockOffset();

		fl_SectionLayout* pCLSL = pCL->getSectionLayout();
		if(pCLSL->getType() == FL_SECTION_SHADOW)
		{
			fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
			bResult = pHdr->bl_doclistener_populateObject(pCL, blockOffset,pcro);
		}
		else
			bResult = pCLSL->bl_doclistener_populateObject(pCL, blockOffset,pcro);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_InsertFmtMark:
	{
		const PX_ChangeRecord_FmtMark * pcrfm = static_cast<const PX_ChangeRecord_FmtMark *>(pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		fl_SectionLayout* pCLSL = pCL->getSectionLayout();
		if(pCLSL->getType() == FL_SECTION_SHADOW)
		{
			fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
			bResult = pHdr->bl_doclistener_insertFmtMark(pCL, pcrfm);
		}
		else
			bResult = pCLSL->bl_doclistener_insertFmtMark(pCL, pcrfm);
		goto finish_up;
	}
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}

 finish_up:
	if (0 == m_iGlobCounter)
	{
#ifndef UPDATE_LAYOUT_ON_SIGNAL
		m_pLayout->updateLayout();
#endif
	}
	
	return bResult;
}

/*!
 */
bool fl_DocListener::populateStrux(PL_StruxDocHandle sdh,
								   const PX_ChangeRecord * pcr,
								   PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(m_pLayout);

	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);

	XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
	if(pFrame)
	{
		if(pcrx->getStruxType() == PTX_Block && !m_bFootnoteInProgress)
		{
			if(!m_bEndFootnoteProcessedInBlock)
			{
				xxx_UT_DEBUGMSG(("Null Update in Populate Strux \n"));
				pFrame->nullUpdate();
			}
			else
			{
				m_bEndFootnoteProcessedInBlock = false;
				UT_DEBUGMSG(("EndFootnote strux processed Populate Strux \n"));
			}
		}
	}
//
// This piece of code detects if there is enough document to notify
// listeners and other things.
//
// If so it moves the insertion point from 0.
//
	if(m_pLayout->getView() && (m_pLayout->getView()->getPoint() == 0))
	{
		fl_DocSectionLayout * pDSL = m_pLayout->getFirstSection();
		if(pDSL)
		{
			fl_ContainerLayout * pCL = pDSL->getFirstLayout();
			UT_uint32 i = 0;
			while(pCL && i< 2)
			{
				i++;
				pCL = pCL->getNext();
			}
			if(i >= 2)
			{
				m_pLayout->getView()->moveInsPtTo(FV_DOCPOS_BOD);
			}
		}
	}

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
	{
		PT_AttrPropIndex indexAP = pcr->getIndexAP();
		const PP_AttrProp* pAP = NULL;
		UT_DEBUGMSG(("SEVIOR: Doing Populate Section in DocListener \n"));
		if (m_pDoc->getAttrProp(indexAP, &pAP) && pAP)
		{
			const XML_Char* pszSectionType = NULL;
			pAP->getAttribute("type", pszSectionType);
			UT_DEBUGMSG(("fl_DocListener::populateStrux for '%s'\n",
						 pszSectionType ? pszSectionType : "(null)"));
			if (!pszSectionType	|| (0 == UT_strcmp(pszSectionType, "doc")))
			{
				// Append a SectionLayout to this DocLayout
				//
				// Format Previous section is it exists to get page mapping sane.
				//
				UT_DEBUGMSG(("SEVIOR: Doing Populate DocSection in DocListener \n"));
				fl_DocSectionLayout * pPDSL = m_pLayout->getLastSection();
				if(pPDSL != NULL)
				{
					pPDSL->format();
				}
				
				fl_DocSectionLayout* pSL = new fl_DocSectionLayout(m_pLayout, sdh, pcr->getIndexAP(), FL_SECTION_DOC);
				if (!pSL)
				{
					UT_DEBUGMSG(("no memory for SectionLayout"));
					return false;
				}
				
				m_pLayout->addSection(pSL);
				
				*psfh = (PL_StruxFmtHandle)pSL;
				
				m_pCurrentSL = pSL;
			}
			else
			{
				HdrFtrType hfType = s_convertToHdrFtrType(pszSectionType);

				if(hfType != FL_HDRFTR_NONE)
				{
					const XML_Char* pszID = NULL;
					pAP->getAttribute("id", pszID);
					UT_DEBUGMSG(("Populating header/footer header strux \n"));
					fl_DocSectionLayout* pDocSL = m_pLayout->findSectionForHdrFtr((char*)pszID);
					UT_ASSERT(pDocSL);
			
					// Append a HdrFtrSectionLayout to this DocLayout
					fl_HdrFtrSectionLayout* pSL = new fl_HdrFtrSectionLayout(hfType, m_pLayout, pDocSL, sdh, pcr->getIndexAP());
					if (!pSL)
					{
						UT_DEBUGMSG(("no memory for SectionLayout"));
						return false;
					}
					//
					// Add the hdrFtr section to the linked list of SectionLayouts
					//
					m_pLayout->addHdrFtrSection(pSL);
					pDocSL->setHdrFtr(hfType, pSL);
					*psfh = (PL_StruxFmtHandle)pSL;
					
					m_pCurrentSL = pSL;
				}
				else
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					return false;
				}
			}
		}
		else
		{
			// TODO fail?
		   UT_DEBUGMSG(("fl_DocListener::populateStrux - m_doc->getAttrProp() failed for structure %d\n",indexAP));
			return false;
		}
	}
	break;

	case PTX_SectionFootnote:
	{
		UT_DEBUGMSG(("fl_DocListener::populateStrux for 'SectionFootnote'\n"));
		UT_ASSERT(m_pCurrentSL);
		m_bFootnoteInProgress = true;
	    fl_SectionLayout * pSL = (fl_SectionLayout *) m_pCurrentSL->append(sdh, pcr->getIndexAP(),FL_CONTAINER_FOOTNOTE);
		*psfh = (PL_StruxFmtHandle)pSL;
		m_pCurrentSL = (fl_SectionLayout*)pSL;
		break;
	}

	case PTX_EndFootnote:
	{
		UT_DEBUGMSG(("fl_DocListener::populateStrux for 'EndFootnote'\n"));
//
// CurrentSL is a Footnote. Return this and set the m_pCurrentSL to it's 
// container
//
		fl_ContainerLayout * pCL = m_pCurrentSL;
		m_bFootnoteInProgress = false;
		m_bEndFootnoteProcessedInBlock = true;
		UT_ASSERT(pCL->getContainerType() == FL_CONTAINER_FOOTNOTE);
		*psfh = (PL_StruxFmtHandle) pCL;
		m_pCurrentSL = (fl_SectionLayout *) static_cast<fl_FootnoteLayout *>(m_pCurrentSL)->getDocSectionLayout();
		fl_FootnoteLayout * pFL = (fl_FootnoteLayout *) pCL;
		pFL->setFootnoteEndIn();
		fl_BlockLayout * pBL = (fl_BlockLayout *) pFL->getFirstLayout();
		UT_ASSERT(pBL);
		if(pBL)
		{
			pBL->updateEnclosingBlockIfNeeded();
		}
		UT_ASSERT(m_pCurrentSL);
		UT_ASSERT(m_pCurrentSL->getContainerType() == FL_CONTAINER_DOCSECTION);
		break;
	}
	case PTX_SectionHdrFtr:
		// This path is taken on a change of page type. Eg A4 => letter.
	{
	   UT_DEBUGMSG(("fl_DocListener::populateStrux for '%s'\n","SectionHdrFtr"));
		PT_AttrPropIndex indexAP = pcr->getIndexAP();
		const PP_AttrProp* pAP = NULL;
		if (m_pDoc->getAttrProp(indexAP, &pAP) && pAP)
		{
			const XML_Char* pszSectionType = NULL;
			pAP->getAttribute("type", pszSectionType);
			if (
				!pszSectionType
				|| (0 == UT_strcmp(pszSectionType, "doc"))
				)
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
			else
			{
				HdrFtrType hfType = s_convertToHdrFtrType(pszSectionType);
				if(hfType != FL_HDRFTR_NONE)
				{
					const XML_Char* pszID = NULL;
					pAP->getAttribute("id", pszID);

					fl_DocSectionLayout* pDocSL = m_pLayout->findSectionForHdrFtr((char*)pszID);
					if (pDocSL == NULL)
					{
						UT_DEBUGMSG(("Could not find HeaderFooter %s\n",(char*)pszID));
						return false;
					}
			
					// Append a HdrFtrSectionLayout to this DocLayout
					fl_HdrFtrSectionLayout* pSL = new fl_HdrFtrSectionLayout(hfType, m_pLayout, pDocSL, sdh, pcr->getIndexAP());
					if (!pSL)
					{
						UT_DEBUGMSG(("no memory for SectionLayout"));
						return false;
					}
					//
					// Add the hdrFtr section to the linked list of SectionLayouts
					//
					m_pLayout->addHdrFtrSection(pSL);
					pDocSL->setHdrFtr(hfType, pSL);
					*psfh = (PL_StruxFmtHandle)pSL;
					UT_DEBUGMSG(("Sevior: HeaderFooter created %x \n",pSL));
					
					m_pCurrentSL = pSL;
				}
				else
				{
					return false;
				}
				break;
			}
		}
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	case PTX_Block:
	{
		UT_ASSERT(m_pCurrentSL);
//
// Look if we're inside a table. If so append this block to a cell.
//
		fl_ContainerLayout*	pCL = NULL;
		fl_ContainerLayout * pCon = getTopContainerLayout();
		if((pCon != NULL)  && (m_pCurrentSL->getContainerType() != FL_CONTAINER_FOOTNOTE))
		{
			if(pCon->getContainerType() != FL_CONTAINER_CELL)
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				return false;
			}
			fl_CellLayout * pCell = (fl_CellLayout *) pCon;
			UT_DEBUGMSG(("SEVIOR: Appending block to cell %x \n",pCell));
			// Append a new BlockLayout to this cell

			pCL = pCell->append(sdh, pcr->getIndexAP(),FL_CONTAINER_BLOCK);
			if (!pCL)
			{
				UT_DEBUGMSG(("no memory for BlockLayout"));
				return false;
			}
			// BUGBUG: this is *not* thread-safe, but should work for now
			if (m_bScreen)
			{
				UT_uint32 reason =  0;
				if( m_pLayout->getAutoSpellCheck())
				{
					reason = (UT_uint32) FL_DocLayout::bgcrSpelling;
				}
				m_pLayout->queueBlockForBackgroundCheck(reason, (fl_BlockLayout *)pCL,true);
			}
		}
		else
		{
			pCL = m_pCurrentSL->append(sdh, pcr->getIndexAP(),FL_CONTAINER_BLOCK);
			if (!pCL)
			{
				UT_DEBUGMSG(("no memory for BlockLayout"));
				return false;
			}

			// BUGBUG: this is *not* thread-safe, but should work for now
			if (m_bScreen)
			{
				UT_uint32 reason =  0;
				if( m_pLayout->getAutoSpellCheck())
				{
					reason = (UT_uint32) FL_DocLayout::bgcrSpelling;
				}
				m_pLayout->queueBlockForBackgroundCheck(reason, (fl_BlockLayout *)pCL,true);
			}
		}

		*psfh = (PL_StruxFmtHandle)pCL;
		if(pCL->getLastContainer()==NULL)
		{
			if(pCL->getSectionLayout()->getType() != FL_SECTION_HDRFTR && pCL->getPrev() != NULL)
			{
				UT_DEBUGMSG(("In DocListner no LastLine in block append. Fixing this now \n"));
				UT_DEBUGMSG(("getPrev = %d this = %d \n",pCL->getPrev(),pCL));
				pCL->format();
			}
		}

	}
	break;
	case PTX_SectionTable:
	{
		UT_ASSERT(m_pCurrentSL);
		
		// Append a new TableLayout to the SectionLayout or CellLayout
		
		fl_ContainerLayout * pCon = getTopContainerLayout();
		fl_ContainerLayout*	pCL = NULL;
		if(pCon == NULL)
		{
			pCL = m_pCurrentSL->append(sdh, pcr->getIndexAP(),FL_CONTAINER_TABLE);
			UT_DEBUGMSG(("SEVIOR: Appending Table: Table layout is %x \n",pCL));
			if (!pCL)
			{
				UT_DEBUGMSG(("no memory for TableLayout"));
				return false;
			}
		}
		else
		{
			fl_CellLayout * pCell = (fl_CellLayout *) pCon;
			pCL = pCell->append(sdh,pcr->getIndexAP(),FL_CONTAINER_TABLE);
		}
		pushContainerLayout(pCL);
		*psfh = (PL_StruxFmtHandle)pCL;
//
// Don't layout until a endTable strux
//
		m_pDoc->setDontImmediatelyLayout(true);
	}
	break;
	case PTX_SectionCell:
	{
		UT_ASSERT(m_pCurrentSL);
		UT_DEBUGMSG(("!!!! Append Section Cell \n"));
		
		// Append a new CallLayout to the Current TableLayout
		fl_ContainerLayout * pCon = getTopContainerLayout();
		if(pCon->getContainerType() != FL_CONTAINER_TABLE)
		{
#ifdef DEBUG
			m_pDoc->miniDump(sdh,6);
#endif
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
		fl_TableLayout * pTable = (fl_TableLayout *) pCon;
		fl_ContainerLayout*	pCL = pTable->append(sdh, pcr->getIndexAP(),FL_CONTAINER_CELL);
		UT_DEBUGMSG(("SEVIOR: Appending Cell: layout is %x \n",pCL));
		pTable->attachCell(pCL);
		if (!pCL)
		{
			UT_DEBUGMSG(("no memory for CellLayout"));
			return false;
		}
		pushContainerLayout(pCL);
		*psfh = (PL_StruxFmtHandle)pCL;
	}
	break;
	case PTX_EndTable:
	{
		UT_ASSERT(m_pCurrentSL);
		fl_ContainerLayout *  pCon = popContainerLayout();
		UT_DEBUGMSG(("!!!! Append End Table \n"));
		if(pCon->getContainerType() != FL_CONTAINER_TABLE)
		{
#ifndef NDEBUG
			m_pDoc->miniDump(sdh,6);
#endif
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
		*psfh = (PL_StruxFmtHandle)pCon;
		fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pCon);
		UT_DEBUGMSG(("SEVIOR: End table in doclistener \n"));
		pTL->setDirty();
		pCon = getTopContainerLayout();
		if(pCon == NULL)
		{
//
// Reached the top of the stack. Allow the table layout now.
//
			if(!holdTableLayout())
			{
				m_pDoc->setDontImmediatelyLayout(false);
			}
			pTL->format();
		}
	}
	break;
	case PTX_EndCell:
	{
		UT_ASSERT(m_pCurrentSL);
		UT_DEBUGMSG(("!!!! Append End Cell \n"));
		fl_ContainerLayout *  pCon = popContainerLayout();
		UT_ASSERT(pCon);
		if(pCon == NULL)
		{
#ifndef NDEBUG
			m_pDoc->miniDump(sdh,12);
#endif
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
		if(pCon->getContainerType() != FL_CONTAINER_CELL)
		{
#ifndef NDEBUG
			m_pDoc->miniDump(sdh,12);
#endif
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
		*psfh = (PL_StruxFmtHandle) pCon;
	}
	break;
			
	default:
		UT_ASSERT(0);
		return false;
	}

	if (0 == m_iGlobCounter)
	{
#ifndef UPDATE_LAYOUT_ON_SIGNAL
		m_pLayout->updateLayout();
#endif
	}
	
	return true;
}

/*!
 * Push the given ContainerLayout onto the top of the stack.
 */
void fl_DocListener::pushContainerLayout(fl_ContainerLayout * pCL)
{
	m_sLastContainerLayout.push((void *) pCL);
}

/*!
 * View the topmost containerLayout on the stack.
 */
fl_ContainerLayout * fl_DocListener::getTopContainerLayout(void)
{
	static fl_ContainerLayout * pCL;
	m_sLastContainerLayout.viewTop((void**)&pCL);
	return pCL;
}

/*!
 * Pop the most recent ContainerLayout off the top of the stack.
 */
fl_ContainerLayout * fl_DocListener::popContainerLayout(void)
{
	static fl_ContainerLayout * pCL;
	m_sLastContainerLayout.pop((void**)&pCL);
	return pCL;
}

/*!
 * Change a strux or span.
 */
bool fl_DocListener::change(PL_StruxFmtHandle sfh,
							const PX_ChangeRecord * pcr)
{
// PLAM need to put in stuff for footnotes here!
	//UT_DEBUGMSG(("fl_DocListener::change\n"));
	bool bResult = false;

	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_GlobMarker:
	{
		UT_ASSERT(sfh == 0);							// globs are not strux-relative
		const PX_ChangeRecord_Glob * pcrg = static_cast<const PX_ChangeRecord_Glob *> (pcr);
		switch (pcrg->getFlags())
		{
		default:
		case PX_ChangeRecord_Glob::PXF_Null:			// not a valid glob type
			UT_ASSERT(0);
			bResult = false;
			goto finish_up;
				
		case PX_ChangeRecord_Glob::PXF_MultiStepStart:
			m_iGlobCounter++;
			bResult = true;
			goto finish_up;
			
		case PX_ChangeRecord_Glob::PXF_MultiStepEnd:
			m_iGlobCounter--;
			bResult = true;
			goto finish_up;
				
		case PX_ChangeRecord_Glob::PXF_UserAtomicStart:	// TODO decide what (if anything) we need
		case PX_ChangeRecord_Glob::PXF_UserAtomicEnd:	// TODO to do here.
			bResult = true;
			goto finish_up;
		}
	}
			
	case PX_ChangeRecord::PXT_InsertSpan:
	{
		const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		fl_SectionLayout* pCLSL = pCL->getSectionLayout();
		if(pCLSL->getType() == FL_SECTION_SHADOW)
		{
			fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
			pHdr->bl_doclistener_insertSpan(pCL, pcrs);
		}
		else
		{
			bResult = pCLSL->bl_doclistener_insertSpan(pCL, pcrs);
		}
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_DeleteSpan:
	{
		const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		fl_SectionLayout* pCLSL = pCL->getSectionLayout();
		if(pCLSL->getType() == FL_SECTION_SHADOW)
		{
			fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
			bResult = pHdr->bl_doclistener_deleteSpan(pCL, pcrs);
		}
		else
			bResult = pCLSL->bl_doclistener_deleteSpan(pCL, pcrs);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_ChangeSpan:
	{
		const PX_ChangeRecord_SpanChange * pcrsc = static_cast<const PX_ChangeRecord_SpanChange *>(pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		fl_SectionLayout* pCLSL = pCL->getSectionLayout();
		if(pCLSL->getType() == FL_SECTION_SHADOW)
		{
			fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
			bResult = pHdr->bl_doclistener_changeSpan(pCL, pcrsc);
		}
		else
			bResult = pCLSL->bl_doclistener_changeSpan(pCL, pcrsc);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_InsertFmtMark:
	{
		const PX_ChangeRecord_FmtMark * pcrfm = static_cast<const PX_ChangeRecord_FmtMark *>(pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		xxx_UT_DEBUGMSG(("DocListener: InsertFmtMark strux type = %d \n",pL->getType()));
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		fl_SectionLayout* pCLSL = pCL->getSectionLayout();
		if(pCLSL->getType() == FL_SECTION_SHADOW)
		{
			fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
			bResult = pHdr->bl_doclistener_insertFmtMark(pCL, pcrfm);
		}
		else
			bResult = pCLSL->bl_doclistener_insertFmtMark(pCL, pcrfm);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_DeleteFmtMark:
	{
		const PX_ChangeRecord_FmtMark * pcrfm = static_cast<const PX_ChangeRecord_FmtMark *>(pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		fl_SectionLayout* pCLSL = pCL->getSectionLayout();
		if(pCLSL->getType() == FL_SECTION_SHADOW)
		{
			fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
			bResult = pHdr->bl_doclistener_deleteFmtMark(pCL, pcrfm);
		}
		else
			bResult = pCLSL->bl_doclistener_deleteFmtMark(pCL, pcrfm);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_ChangeFmtMark:
	{
		const PX_ChangeRecord_FmtMarkChange * pcrfmc = static_cast<const PX_ChangeRecord_FmtMarkChange *>(pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		fl_SectionLayout* pCLSL = pCL->getSectionLayout();
		if(pCLSL->getType() == FL_SECTION_SHADOW)
		{
			fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
			bResult = pHdr->bl_doclistener_changeFmtMark(pCL, pcrfmc);
		}
		else
			bResult = pCLSL->bl_doclistener_changeFmtMark(pCL, pcrfmc);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_DeleteStrux:
	{
		const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);

		switch (pcrx->getStruxType())
		{
		case PTX_Section:
		case PTX_SectionEndnote:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_Section || pL->getType() == PTX_SectionEndnote);
			fl_DocSectionLayout * pSL = static_cast<fl_DocSectionLayout *>(pL);
			bResult = pSL->doclistener_deleteStrux(pcrx);
			goto finish_up;
		}
		case PTX_Block:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_Block);
			fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			fl_SectionLayout* pCLSL = pCL->getSectionLayout();
			if(pCLSL->getType() == FL_SECTION_SHADOW)
			{
				fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
				bResult = pHdr->bl_doclistener_deleteStrux(pCL, pcrx);
			}
			else
				bResult = pCLSL->bl_doclistener_deleteStrux(pCL, pcrx);
			goto finish_up;
		}
		case PTX_SectionHdrFtr:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_SectionHdrFtr);
			fl_HdrFtrSectionLayout * pSL = static_cast<fl_HdrFtrSectionLayout *>(pL);
//
// Nuke the HdrFtrSectionLayout and the shadows associated with it.
//
			pSL->doclistener_deleteStrux(pcrx); 
			m_pLayout->updateLayout();
			goto finish_up;
		}
		case PTX_SectionFootnote:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_SectionFootnote);
			fl_FootnoteLayout * pFL = (fl_FootnoteLayout *) pL;
			pFL->doclistener_deleteStrux(pcrx);
			goto finish_up;
		}
		case PTX_SectionTable:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_SectionTable);
			fl_TableLayout * pTL = (fl_TableLayout *) pL;
			pTL->doclistener_deleteStrux(pcrx);
			goto finish_up;
		}
		case PTX_SectionCell:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_SectionCell);
			fl_CellLayout * pCellL = (fl_CellLayout *) pL;
			pCellL->doclistener_deleteStrux(pcrx);
			goto finish_up;
		}
		case PTX_EndTable:
		{
			goto finish_up;
		}
		case PTX_EndCell:
		{
			goto finish_up;
		}
		case PTX_EndFootnote:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_SectionFootnote);
			fl_FootnoteLayout * pFL = (fl_FootnoteLayout *) pL;
			pFL->doclistener_deleteEndFootnote(pcrx);

			goto finish_up;
		}
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			bResult = false;
			goto finish_up;
		}
	}
					
	case PX_ChangeRecord::PXT_ChangeStrux:
	{
		const PX_ChangeRecord_StruxChange * pcrxc = static_cast<const PX_ChangeRecord_StruxChange *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;

		// TODO getOldIndexAP() is only intended for use by the document.
		// TODO this assert is probably wrong. --- BUT EVERYTIME IT HAS
		// TODO GONE OFF, I'VE FOUND A BUG, SO MAYBE WE SHOULD KEEP IT :-)
		// UT_ASSERT(pL->getAttrPropIndex() == pcrxc->getOldIndexAP());
		// UT_ASSERT(pL->getAttrPropIndex() != pcr->getIndexAP());

		switch (pL->getType())
		{
		case PTX_Section:
		case PTX_SectionEndnote:
		{
			fl_DocSectionLayout* pSL = static_cast<fl_DocSectionLayout*>(pL);
			
			PT_AttrPropIndex indexAP = pcr->getIndexAP();
			const PP_AttrProp* pAP = NULL;
			
			bool bres = (m_pDoc->getAttrProp(indexAP, &pAP) && pAP);
			UT_ASSERT(bres);
			PL_StruxDocHandle sdh = pL->getStruxDocHandle();
	
			const XML_Char* pszSectionType = NULL;
			pAP->getAttribute("type", pszSectionType);
			//
			// OK Sevior adds code to actually change a 
			// sectionlayout to
			// a header/footer layout
			//
			// Strategy: Collapse all the blocks in this section.
			// create a header/footer sectionlayout ala populate_strux
			// transfer the blocks in this sectionlayout to the
			// new header/footer and format just the shadows
			//
			HdrFtrType hfType = s_convertToHdrFtrType(pszSectionType);
			if(hfType != FL_HDRFTR_NONE)
			{
				//
				//  OK first we need a previous section with a
				//  matching ID
				//
				const XML_Char* pszID = NULL;
				pAP->getAttribute("id", pszID);

				fl_DocSectionLayout* pDocSL = m_pLayout->findSectionForHdrFtr((char*)pszID);
				UT_ASSERT(pDocSL); 
			        
				// Append a HdrFtrSectionLayout to this DocLayout
				fl_HdrFtrSectionLayout* pHeadSL = new fl_HdrFtrSectionLayout(hfType, m_pLayout, pDocSL, sdh, pcr->getIndexAP());
				if (!pHeadSL)
				{
					UT_DEBUGMSG(("no memory for SectionLayout"));
					return false;
				}
				//
				// Add the hdrFtr section to the linked list of SectionLayouts
				//
				m_pLayout->addHdrFtrSection(pHeadSL);
				//
				// Set the pointers to this header/footer
				//
				pDocSL->setHdrFtr(hfType, pHeadSL);

				// OK now clean up the old section and transfer
				// blocks into this header section.
				
				pHeadSL->changeIntoHdrFtrSection(pSL);

				bResult = true;
				goto finish_up;
			}
			if(pSL->getType() == FL_SECTION_DOC)
			{
				fl_DocSectionLayout * pDSL = (fl_DocSectionLayout *) pSL;
				m_pLayout->changeDocSections(pcrxc,pDSL);
				bResult = true;
				goto finish_up;
			}
 			bResult = pSL->doclistener_changeStrux(pcrxc);
			goto finish_up;
		}
		
		case PTX_Block:
		{
			fl_SectionLayout * pCL = static_cast<fl_SectionLayout *>(pL);
			fl_SectionLayout* pCLSL = pCL->getSectionLayout();
			if(pCLSL->getType() == FL_SECTION_SHADOW)
			{
				fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
				bResult = pHdr->bl_doclistener_changeStrux( pCL, pcrxc);
			}
			else 
				bResult = pCLSL->bl_doclistener_changeStrux(pCL, pcrxc);
			goto finish_up;
		}
		case PTX_SectionHdrFtr:
		{
//
// OK A previous "insertStrux" has created a HdrFtrSectionLayout but it
// Doesn't know if it's a header or a footer or the DocSection and hences pages
// It associated with. Tell it now.
//
			fl_HdrFtrSectionLayout* pHFSL = static_cast<fl_HdrFtrSectionLayout*>(pL);
			
			PT_AttrPropIndex indexAP = pcr->getIndexAP();
//
// Save this new index to the Attributes/Properties of this section in the
// HdrFtrSection Class
//
			pHFSL->setAttrPropIndex(pcrxc->getIndexAP());
			const PP_AttrProp* pHFAP = NULL;
			
			bool bres = (m_pDoc->getAttrProp(indexAP, &pHFAP) && pHFAP);
			UT_ASSERT(bres);
	
			const XML_Char* pszHFSectionType = NULL;
			pHFAP->getAttribute("type", pszHFSectionType);
			//
            // Look for type of Hdr/Ftr
			//
			HdrFtrType hfType = s_convertToHdrFtrType(pszHFSectionType);
			if(hfType !=  FL_HDRFTR_NONE )
			{
				//
				//  OK now we need a previous section with a
				//  matching ID
				//
				const XML_Char* pszHFID = NULL;
				pHFAP->getAttribute("id", pszHFID);

				fl_DocSectionLayout* pDocSL = m_pLayout->findSectionForHdrFtr((char*)pszHFID);
				
				UT_ASSERT(pDocSL); 
			        
				// Set the Headerness and overall docSectionLayout
				//
				pHFSL->setDocSectionLayout(pDocSL);
				pHFSL->setHdrFtr(hfType);
				//
				// Set the pointers to this header/footer
				//
				pDocSL->setHdrFtr(hfType, pHFSL);

				// OK now Format this and attach it to it's pages
				
				pHFSL->format();

				bResult = true;
				goto finish_up;
			}
			UT_DEBUGMSG(("SEVIOR: Unknown change record on a SectionHdrFtr strux \n"));
			UT_DEBUGMSG(("SEVIOR: Most like we're undoing an Insert HdrFtr. Carry on! \n"));
			bResult = true;
			goto finish_up;
		}
        case PTX_SectionTable:
		{
			fl_TableLayout * pTL = (fl_TableLayout *) pL;
			UT_ASSERT(pTL->getContainerType() == FL_CONTAINER_TABLE);
 			bResult = pTL->doclistener_changeStrux(pcrxc);
			goto finish_up;
		}
		case PTX_SectionCell:
		{
			fl_CellLayout * pCL = (fl_CellLayout *) pL;
			UT_ASSERT(pCL->getContainerType() == FL_CONTAINER_CELL);
			bResult = pCL->doclistener_changeStrux(pcrxc);
			goto finish_up;
		}
		default:
		{
			UT_ASSERT(0);
			bResult = false;
			goto finish_up;
		}
		}
	}

	case PX_ChangeRecord::PXT_InsertStrux:
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		bResult = false;
		goto finish_up;
	}
	case PX_ChangeRecord::PXT_InsertObject:
	{
		const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		fl_SectionLayout* pCLSL = pCL->getSectionLayout();
		if(pCLSL->getType() == FL_SECTION_SHADOW)
		{
			fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
			bResult = pHdr->bl_doclistener_insertObject(pCL, pcro);
		}
		else
			bResult = pCLSL->bl_doclistener_insertObject(pCL, pcro);
		goto finish_up;
	}
	case PX_ChangeRecord::PXT_DeleteObject:
	{
		const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		fl_SectionLayout* pCLSL = pCL->getSectionLayout();
		if(pCLSL->getType() == FL_SECTION_SHADOW)
		{
			fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
			bResult = pHdr->bl_doclistener_deleteObject(pCL, pcro);
		}
		else
			bResult = pCLSL->bl_doclistener_deleteObject(pCL, pcro);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_ChangeObject:
	{
		const PX_ChangeRecord_ObjectChange * pcroc = static_cast<const PX_ChangeRecord_ObjectChange *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		fl_SectionLayout* pCLSL = pCL->getSectionLayout();
		if(pCLSL->getType() == FL_SECTION_SHADOW)
		{
			fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
			bResult = pHdr->bl_doclistener_changeObject(pCL, pcroc);
		}
		else
			bResult = pCLSL->bl_doclistener_changeObject(pCL, pcroc);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_ChangePoint:
	{
		FV_View* pView = m_pLayout->getView();
		if (pView && pView->isActive())
			pView->_setPoint(pcr->getPosition());
		goto finish_up;
	}
	case PX_ChangeRecord::PXT_ListUpdate:
	{
		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		pCL->listUpdate();
		goto finish_up;

	}
	case PX_ChangeRecord::PXT_StopList:
	{
		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pCL = static_cast<fl_BlockLayout *>(pL);
		pCL->StopListInBlock();
		goto finish_up;
	}
	case PX_ChangeRecord::PXT_UpdateField:
	{
		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		pCL->format();
		FV_View* pView = m_pLayout->getView();
		pView->updateScreen();
		goto finish_up;
	}
	case PX_ChangeRecord::PXT_RemoveList:
	{
		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_BlockLayout * pCL = static_cast<fl_BlockLayout *>(pL);
		pCL->m_bStopList = true;
		pCL->_deleteListLabel();
		pCL->m_pAutoNum = NULL;
		pCL->m_bListItem = false;
		pCL->m_bStopList = false;
		goto finish_up;
	}
	case PX_ChangeRecord::PXT_UpdateLayout:
	{
		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		pCL->getDocLayout()->updateLayout();
		FV_View * pView = pCL->getDocLayout()->getView();
		if(pView)
		{
			pView->notifyListeners(AV_CHG_HDRFTR);
		}

		goto finish_up;
	}
#if 0 //###TF
	case PX_ChangeRecord::PXT_InsertBookmark:
	{
		const PX_ChangeRecord_Bookmark * pcrfm = static_cast<const PX_ChangeRecord_Bookmark *>(pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_DEBUGMSG(("DocListener: InsertBookmark strux type = %d \n",pL->getType()));
		//UT_ASSERT(pL->getType() == PTX_Block);
		//fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		//fl_SectionLayout* pCLSL = pCL->getSectionLayout();
		//if(pCLSL->getType() == FL_SECTION_SHADOW)
		//{
		//	fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
		//	bResult = pHdr->bl_doclistener_insertFmtMark(pCL, pcrfm);
		//}
		//else
		//	bResult = pCLSL->bl_doclistener_insertFmtMark(pCL, pcrfm);
		goto finish_up;
	}
#endif
	
	default:
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		bResult = false;
		goto finish_up;
	}
	}

 finish_up:
	if (0 == m_iGlobCounter)
	{
#ifndef UPDATE_LAYOUT_ON_SIGNAL
		m_pLayout->updateLayout();
#endif
	}
	
	return bResult;
}

/*!
 */
bool fl_DocListener::insertStrux(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr,
								 PL_StruxDocHandle sdh,
								 PL_ListenerId lid,
								 void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
														 PL_ListenerId lid,
														 PL_StruxFmtHandle sfhNew))
{
	UT_DEBUGMSG(("fl_DocListener::insertStrux at pos %d \n",pcr->getPosition()));
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
#if DEBUG
#if 1
	if(pcrx->getStruxType() == PTX_Block)
	{
		UT_DEBUGMSG(("Inserting Block strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_Section)
	{
		UT_DEBUGMSG(("Inserting Section strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_SectionFootnote)
	{
		UT_DEBUGMSG(("Inserting SectionFootnote strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_EndFootnote)
	{
		UT_DEBUGMSG(("Inserting EndFootnote strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_SectionTable)
	{
		UT_DEBUGMSG(("Inserting SectionTable strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_EndTable)
	{
		UT_DEBUGMSG(("Inserting EndTable strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_SectionCell)
	{
		UT_DEBUGMSG(("Inserting Cell strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_EndCell)
	{
		UT_DEBUGMSG(("Inserting EndCell strux at pos %d \n",pcr->getPosition()));
	}
#endif
#endif
	fl_Layout * pL = (fl_Layout *)sfh;
	xxx_UT_DEBUGMSG(("Previous strux %x type %d \n",pL, pL->getType()));
	xxx_UT_DEBUGMSG(("Insert strux type %d \n",pcrx->getStruxType()));
	switch (pL->getType())				// see what the immediately prior strux is.
	{
	case PTX_Section:		   // the immediately prior strux is a section.
	case PTX_SectionHdrFtr:	   //  ... or a HdrFtr.
	case PTX_SectionFootnote:  //  ... or a Footnote.
    {
		switch (pcrx->getStruxType())	// see what we are inserting.
		{
		case PTX_Section:				// we are inserting a section.
		case PTX_SectionHdrFtr:			//  ... or a HdrFtr section.
		case PTX_SectionFootnote:        //  ... or a Footnote section.
			// We are inserting a section immediately after a section
			// (with no intervening block).  This is probably a bug,
			// because there should at least be an empty block between
			// them (so that the user can set
			// the cursor there and start typing, if nothing else).
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		case PTX_Block:					// we are inserting a block.
		{
			// The immediately prior strux is a section.  So, this
			// will become the first block of the section and have no
			// text.

			fl_SectionLayout * pSL = static_cast<fl_SectionLayout *>(pL);
			return pSL->bl_doclistener_insertBlock(NULL, pcrx,sdh,lid,pfnBindHandles);
		}
#if 0
//
// FIXME When I get brave I'll see if we can't make a table te first layout
// in a section.
//
		case PTX_SectionTable:
		{
			UT_DEBUGMSG(("Insert Table immediately after Section \n"));
			fl_SectionLayout * pSL = static_cast<fl_SectionLayout *>(pL);
			return pSL->bl_doclistener_insertTable(NULL, pcrx,sdh,lid,pfnBindHandles);
		}
#endif
		default:						// unknown strux.
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
	}	
    case PTX_Block:						// the immediately prior strux is a block.
    {
		switch (pcrx->getStruxType())	// see what we are inserting.
		{
		case PTX_Section:				// we are inserting a section.
		   {
			   // The immediately prior strux is a block.  Everything
			   // from this point forward (to the next section) needs to
			   // be re-parented to this new section.  We also need to
			   // verify that there is a block immediately after this new
			   // section -- a section must be followed by a block
			   // because a section cannot contain content.
			
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   xxx_UT_DEBUGMSG(("Doing Insert Strux Section Into Prev Block \n"));
			   fl_SectionLayout* pCLSL = pCL->getSectionLayout();
			   bool bResult = pCLSL->bl_doclistener_insertSection(pCL, FL_SECTION_DOC, pcrx,sdh,lid,pfnBindHandles);
	
			   return bResult;
		   }
		
		case PTX_Block:					// we are inserting a block.
		   {
			   // The immediately prior strux is also a block.  Insert the new
			   // block and split the content between the two blocks.
			   UT_DEBUGMSG(("InsertBlock into Block \n"));
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   fl_SectionLayout* pCLSL = pCL->getSectionLayout();
			   bool bResult = true;
			   if(pCLSL->getType() == FL_SECTION_SHADOW)
			   {
				   fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
				   bResult = pHdr->bl_doclistener_insertBlock(pCL, pcrx,sdh,lid,pfnBindHandles);
			   }
			   else
			   {
				   bResult = pCLSL->bl_doclistener_insertBlock(pCL, pcrx,sdh,lid,pfnBindHandles);
			   }
			   return bResult;
		   }
	    case PTX_SectionHdrFtr:
		   {
			   // The immediately prior strux is a block.  Everything
			   // from this point forward (to the next section) needs to
			   // be re-parented to this new HdrFtr section.  We also need to
			   // verify that there is a block immediately after this new
			   // section -- a section must be followed by a block
			   // because a section cannot contain content.
			
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   xxx_UT_DEBUGMSG(("Doing Insert Strux HdrFtr Into Prev Block \n"));
			   fl_SectionLayout* pCLSL = pCL->getSectionLayout();
			   bool bResult = pCLSL->bl_doclistener_insertSection(pCL, FL_SECTION_HDRFTR, pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		case PTX_SectionFootnote:
		   {
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   fl_SectionLayout* pCLSL = pCL->getSectionLayout();

			   // not working right -- PLAM
			   bool bResult = pCLSL->bl_doclistener_insertSection(pCL, FL_SECTION_FOOTNOTE, pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		case PTX_SectionTable:				// we are inserting a section.
		   {
			   // The immediately prior strux is a block.  
			   // OK this creates a table in the document.
			   UT_DEBUGMSG(("Insert Table into Block \n"));
			
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   UT_DEBUGMSG(("Doing Insert Strux Table Into Prev Block \n"));
			   xxx_UT_DEBUGMSG(("Doing Insert Table Correctly \n"));
			   fl_SectionLayout* pCLSL = pCL->getSectionLayout();
			   bool bResult = (pCLSL->bl_doclistener_insertTable(pCL,FL_SECTION_TABLE, pcrx,sdh,lid,pfnBindHandles) != 0);
			   return bResult;
		   }
		case PTX_EndFootnote:
		   {
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout*>(pL);

			   fl_FootnoteLayout* pCLSL = 
				   (fl_FootnoteLayout *)pCL->myContainingLayout();
			   UT_ASSERT(pCLSL->getContainerType() == FL_CONTAINER_FOOTNOTE);
			   bool bResult = pCLSL->bl_doclistener_insertEndFootnote(pCL, pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		case PTX_EndCell:				// we are inserting an endcell.
		   {
			   // The immediately prior strux is a block.  
			   // OK this finishes a cell in the table

			   UT_DEBUGMSG(("Insert endCell into Block \n"));
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   UT_DEBUGMSG(("Doing Insert Strux EndCell Into Prev Block \n"));
//
// This gets us a fl_SectionCell.
//
			   fl_CellLayout* pCLSL = (fl_CellLayout *) pCL->myContainingLayout();
			   if(pCLSL->getContainerType() != FL_CONTAINER_CELL)
			   {
				   m_pDoc->miniDump(pL->getStruxDocHandle(),6);
			   }
			   UT_ASSERT(pCLSL->getContainerType() == FL_CONTAINER_CELL);
			   bool bResult = pCLSL->bl_doclistener_insertEndCell(pCL, pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		default:
		   {
			   m_pDoc->miniDump(pL->getStruxDocHandle(),6);
			   UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			   return false;
		   }
		}
	}
    case PTX_SectionTable:						// The immediately prior strux is a table.
    {
		switch (pcrx->getStruxType())	// see what we are inserting.
		{
		case PTX_SectionCell:				// we are inserting a cell. This is valid
		  {
			   // The immediately prior strux is a Table.  
			   // OK this creates a table in the document.
			   UT_DEBUGMSG(("Insert Cell into Table \n"));
			
			   fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pL);
			   xxx_UT_DEBUGMSG(("Doing Insert Cell Correctly \n"));
			   bool bResult = pTL->bl_doclistener_insertCell(NULL,pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		case PTX_Block:				// we are inserting a block after a endTable. This is valid
		  {
			   // The immediately prior strux is a EndTable.  
			   // This isnerts a block after that. This happens on undo after
               // deleting a table.
			   UT_DEBUGMSG(("Insert Block into Table/EndTable \n"));
			
			   fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pL);
			   bool bResult = pTL->bl_doclistener_insertBlock(NULL,pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		case PTX_SectionTable: // insert table after a table this is valid
		{
			   // The immediately prior strux is a EndTable.  
			   // This isnerts a Table after that. Thus we we have two tables
			   // in a row.
			   UT_DEBUGMSG(("Insert Table after Table/EndTable \n"));
			   fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pL);
			   bool bResult = pTL->bl_doclistener_insertTable(pcrx, FL_SECTION_TABLE,sdh,lid,pfnBindHandles);
			   return bResult;
		}
		default:
		   {
			   UT_DEBUGMSG(("Illegal strux type after table %d \n",pcrx->getStruxType()));
			   m_pDoc->miniDump(pL->getStruxDocHandle(),6);
			   UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			   return false;
		   }
		}
	}
//
// Actually this should never becalle because we bind the endcell strux to cell container
//
    case PTX_EndCell:						// The immediately prior strux is a end cell.
    {
		switch (pcrx->getStruxType())	// see what we are inserting.
		{
		case PTX_SectionCell:				// we are inserting a cell. This is valid
		  {
			   // The immediately prior strux is a endCell.  
			   // OK this creates a table in the document.
			   // The end cell layout is actually a pointer to the cell it ends.

			   UT_DEBUGMSG(("Insert Cell into EndCell \n"));
			  fl_ContainerLayout * pConL = (fl_ContainerLayout *) pL;
			   if(pConL->getContainerType() != FL_CONTAINER_CELL)
			   {
				   m_pDoc->miniDump(pL->getStruxDocHandle(),6);
			   }
			  UT_ASSERT(pConL->getContainerType() == FL_CONTAINER_CELL);
			  fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pConL->myContainingLayout());
			  UT_ASSERT(pTL->getContainerType() == FL_CONTAINER_TABLE);
			  fl_CellLayout * pCL = static_cast<fl_CellLayout *>(pConL);
			  UT_DEBUGMSG(("Doing Insert Cell after endcell Correctly \n"));
			  bool bResult = pTL->bl_doclistener_insertCell(pCL, pcrx,sdh,lid,pfnBindHandles);
			  return bResult;
		  }
		case PTX_EndTable:				// we are inserting an endTable cell. This is valid
		  {
			   // The immediately prior strux is a endTable.  
			   // OK this creates a table in the document.
			   // The end cell layout

			   UT_DEBUGMSG(("Insert EndTable into EndCell \n"));

			  fl_ContainerLayout * pConL = (fl_ContainerLayout *) pL;
			   if(pConL->getContainerType() != FL_CONTAINER_CELL)
			   {
				   m_pDoc->miniDump(pL->getStruxDocHandle(),6);
			   }

			  UT_ASSERT(pConL->getContainerType() == FL_CONTAINER_CELL);
			  fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pConL->myContainingLayout());
			  UT_ASSERT(pTL->getContainerType() == FL_CONTAINER_TABLE);
			  fl_CellLayout * pCL = static_cast<fl_CellLayout *>(pL);
			  UT_DEBUGMSG(("Doing Insert Cell after EndTable Correctly \n"));
			   bool bResult = pTL->bl_doclistener_insertEndTable(pCL,pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		default:
		   {
			   UT_DEBUGMSG(("Illegal strux type after endcell %d \n",pcrx->getStruxType()));
			   m_pDoc->miniDump(pL->getStruxDocHandle(),6);
			   UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			   return false;
		   }
		}
	}
    case PTX_SectionCell:						// The immediately prior strux is a cell.
    {
		switch (pcrx->getStruxType())	// see what we are inserting.
		{
		case PTX_Block:				// we are inserting a cell. This is valid
		  {
			   // The immediately prior strux is a section.  So, this
			   // will become the first block of the section and have no
			   // text.
			  UT_DEBUGMSG(("Inserting block into CEll \n"));
			  fl_SectionLayout * pSL = static_cast<fl_SectionLayout *>(pL);
			  UT_ASSERT(pSL->getContainerType() == FL_CONTAINER_CELL);
			  bool bResult = pSL->bl_doclistener_insertBlock(NULL, pcrx,sdh,lid,pfnBindHandles);
			  return bResult;
		   }
		case PTX_EndTable:				// we are inserting an endTable cell. This is valid
		  {
			   // The immediately prior strux is a cell. This actually valid since the
               // endCell strux actually points to the previous cell layout.
			   // OK this creates a table in the document.
			   // The end cell layout

			   UT_DEBUGMSG(("Insert EndTable into Cell/EndCell \n"));

			  fl_ContainerLayout * pConL = (fl_ContainerLayout *) pL;
			  UT_ASSERT(pConL->getContainerType() == FL_CONTAINER_CELL);
			  fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pConL->myContainingLayout());
			   if(pTL->getContainerType() != FL_CONTAINER_TABLE)
			   {
				   m_pDoc->miniDump(pL->getStruxDocHandle(),6);
			   }
			  UT_ASSERT(pTL->getContainerType() == FL_CONTAINER_TABLE);
			  fl_CellLayout * pCL = static_cast<fl_CellLayout *>(pL);
			  UT_DEBUGMSG(("Doing Insert EndTable Correctly \n"));
			   bool bResult = pTL->bl_doclistener_insertEndTable(pCL,pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		case PTX_SectionCell:				// we are inserting a cell. This is valid
		  {
			   // The immediately prior strux is a cell or endCell this happens
               // because the layout handle of the endcell points to cell layout.  
			   // OK this insertes a cell in the table.
			   // The end cell layout is actually a pointer to the cell it ends.

			   UT_DEBUGMSG(("Insert Cell into Cell/EndCell \n"));

			  fl_ContainerLayout * pConL = (fl_ContainerLayout *) pL;
			   if(pConL->getContainerType() != FL_CONTAINER_CELL)
			   {
				   m_pDoc->miniDump(pL->getStruxDocHandle(),6);
			   }
			  UT_ASSERT(pConL->getContainerType() == FL_CONTAINER_CELL);
			  fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pConL->myContainingLayout());
			  UT_ASSERT(pTL->getContainerType() == FL_CONTAINER_TABLE);
			  fl_CellLayout * pCL = static_cast<fl_CellLayout *>(pConL);
			  UT_DEBUGMSG(("Doing Insert Cell Correctly \n"));
			  bool bResult = pTL->bl_doclistener_insertCell(pCL, pcrx,sdh,lid,pfnBindHandles);
			  return bResult;
		  }
		default:
		   {
			   UT_DEBUGMSG(("Illegal strux type after cell %d \n",pcrx->getStruxType()));
			   m_pDoc->miniDump(pL->getStruxDocHandle(),6);
			   UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			   return false;
		   }
		}
	}
//
// todo might have to handle endTable
//
	default:
	{	
		m_pDoc->miniDump(pL->getStruxDocHandle(),6);
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	} // finish the overall switch
	}
	/*NOTREACHED*/
	UT_ASSERT(0);
	return false;
}

/*!
 */
bool fl_DocListener::signal(UT_uint32 iSignal)
{
	FV_View* pView = m_pLayout->getView();

	switch (iSignal)
	{
	case PD_SIGNAL_UPDATE_LAYOUT:
#ifdef UPDATE_LAYOUT_ON_SIGNAL
		m_pLayout->updateLayout();
#endif
		pView->updateScreen();
		break;
	case PD_SIGNAL_REFORMAT_LAYOUT:
		m_pLayout->formatAll();
		break;
	case PD_SIGNAL_DOCPROPS_CHANGED_REBUILD:
		m_pLayout->updatePropsRebuild();
		break;
	case PD_SIGNAL_DOCPROPS_CHANGED_NO_REBUILD:
		m_pLayout->updatePropsNoRebuild();
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return true;
}

/*! Helper method to convert hf strings to HdrFtrType */
static HdrFtrType s_convertToHdrFtrType(const XML_Char * pszHFType)
{
	if (!pszHFType)
		return FL_HDRFTR_NONE;

	if (UT_strcmp(pszHFType,"header") == 0)
		return FL_HDRFTR_HEADER;

	if (UT_strcmp(pszHFType,"header-even") == 0)
		return FL_HDRFTR_HEADER_EVEN;

	if (UT_strcmp(pszHFType,"header-first") == 0)
		return FL_HDRFTR_HEADER_FIRST;

	if (UT_strcmp(pszHFType,"header-last") == 0)
		return FL_HDRFTR_HEADER_LAST;

	if (UT_strcmp(pszHFType,"footer") == 0)
		return FL_HDRFTR_FOOTER;

	if (UT_strcmp(pszHFType,"footer-even") == 0)
		return FL_HDRFTR_FOOTER_EVEN;

	if (UT_strcmp(pszHFType,"footer-first") == 0)
		return FL_HDRFTR_FOOTER_FIRST;

	if (UT_strcmp(pszHFType,"footer-last") == 0)
	    return FL_HDRFTR_FOOTER_LAST;

	return FL_HDRFTR_NONE;
}
