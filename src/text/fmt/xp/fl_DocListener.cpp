/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
#include "fl_DocListener.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fl_BlockLayout.h"
#include "fl_ContainerLayout.h"
#include "fl_FootnoteLayout.h"
#include "fl_TableLayout.h"
#include "fl_TOCLayout.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "gr_Graphics.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Prefs.h"
#include "fv_View.h"
#include "ap_StatusBar.h"
#include "ap_FrameData.h"

#define UPDATE_LAYOUT_ON_SIGNAL

static HdrFtrType s_convertToHdrFtrType(const gchar * pszHFType);

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
	m_bCacheChanges = false;
	m_chgMaskCached = AV_CHG_NONE;
	m_pStatusBar = NULL;
	if(m_pLayout)
	{
	  if(m_pLayout->getView())
	  {
	      if(m_pLayout->getView()->getParentData())
	      {
			  if( AP_FrameData * pData =  static_cast<AP_FrameData *>(static_cast<XAP_Frame *>(m_pLayout->getView()->getParentData())->getFrameData()))
			  {
				  m_pStatusBar = static_cast<AP_StatusBar *>(pData->m_pStatusBar);
			  }
		  }
	  }
	}
	m_iFilled = 0;
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
bool fl_DocListener::populate(fl_ContainerLayout* sfh,
							  const PX_ChangeRecord * pcr)
{
	UT_ASSERT(m_pLayout);

	bool bResult = false;

	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
	{
		const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		if(pL->getType() != PTX_Block)
		{
			m_pDoc->miniDump(pL->getStruxDocHandle(),8);
			UT_DEBUGMSG(("Illegal strux is %p \n",pL->getStruxDocHandle()));
		}			
		UT_ASSERT(pL->getType() == PTX_Block);

		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		if(pCL->getPrev()!= NULL && pCL->getPrev()->getLastContainer()==NULL)
		{
			UT_DEBUGMSG(("In DocListner no LastLine in Previous Block Fixing this now \n"));
			UT_DEBUGMSG(("getPrev = %p this = %p \n",pCL->getPrev(),pCL));
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
			UT_DEBUGMSG(("getPrev = %p this = %p \n",pCL->getPrev(),pCL));
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

#if 0
		// if the inserted object is a bookmark, we have to notify TOCs of bookmark change in case
		// any of them is restricted by the given bookmark
		if(pcro->getObjectType() == PTO_Bookmark)
		{
			if(pL->getType() == PTX_Block)
			{
				fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
				fp_Run * pRun = pBL->findRunAtOffset(pcro->getBlockOffset());
				if(pRun && pRun->getType() == FPRUN_BOOKMARK)
				{
					fp_BookmarkRun * pB = static_cast<fp_BookmarkRun*>(pRun);

					// only do this when the end-object is inserted ...
					if(!pB->isStartOfBookmark())
						m_pLayout->updateTOCsOnBookmarkChange(pB->getName());
				}
				else if(pRun && pRun->getType() != FPRUN_BOOKMARK)
				{
					UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
				}
			}
		}
#endif
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_InsertFmtMark:
	{
		const PX_ChangeRecord_FmtMark * pcrfm = static_cast<const PX_ChangeRecord_FmtMark *>(pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		if(pL->getType() != PTX_Block)
		{
			m_pDoc->miniDump(pL->getStruxDocHandle(),8);
			UT_DEBUGMSG(("Illegal strux is %p \n",pL->getStruxDocHandle()));
			UT_return_val_if_fail((pL->getType() == PTX_Block),false);
		}			
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

static UT_uint32 countStrux = 0;
/*!
 */
bool fl_DocListener::populateStrux(pf_Frag_Strux* sdh,
								   const PX_ChangeRecord * pcr,
								   fl_ContainerLayout* * psfh)
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
				PT_DocPosition pos = pcrx->getPosition();
				UT_sint32 percentFilled = 100*pos/m_pLayout->getDocSize();
				if(percentFilled > m_iFilled)
				{
				  pFrame->nullUpdate();
				  m_iFilled = percentFilled;
				  m_pLayout->setPercentFilled(percentFilled);
				  UT_DEBUGMSG(("Percent filled = %d \n",percentFilled));
				  if(m_pStatusBar)
				  {
				    const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
				    UT_UTF8String msg (pSS->getValue(XAP_STRING_ID_MSG_BuildingDoc));
				    m_pStatusBar->setStatusProgressValue(percentFilled);
				    UT_UTF8String msg2;
				    UT_UTF8String_sprintf(msg2," %d",percentFilled);
				    msg += msg2;
				    msg += "%";
				    m_pStatusBar->setStatusMessage(msg.utf8_str());
				  }
				}
				if(countStrux > 60)
				{
					if(countStrux < 300)
					{
						if(m_pLayout->getView() && (m_pLayout->getView()->getPoint() == 0))
						{
							fl_DocSectionLayout * pDSL = m_pLayout->getFirstSection();
							pDSL->format();
							countStrux = 300;
						}
					}
				}
				countStrux++;
			}
			else
			{
				m_bEndFootnoteProcessedInBlock = false;
				UT_DEBUGMSG(("EndFootnote strux processed Populate Strux \n"));
			}
		}
	}
	else
	{
		UT_DEBUGMSG(("No valid frame!!! \n"));
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
			const gchar* pszSectionType = NULL;
			pAP->getAttribute("type", pszSectionType);
			UT_DEBUGMSG(("fl_DocListener::populateStrux for '%s'\n",
						 pszSectionType ? pszSectionType : "(null)"));
			if (!pszSectionType	|| (0 == strcmp(pszSectionType, "doc")))
			{
				// Append a SectionLayout to this DocLayout
				//
				// Format Previous section is it exists to get page mapping sane.
				//
				UT_DEBUGMSG(("SEVIOR: Doing Populate DocSection in DocListener \n"));
				fl_DocSectionLayout * pPDSL = m_pLayout->getLastSection();
				if(pPDSL && !m_pLayout->isLayoutFilling())
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
				
				*psfh = (fl_ContainerLayout*)pSL;
				
				m_pCurrentSL = pSL;
			}
			else
			{
				HdrFtrType hfType = s_convertToHdrFtrType(pszSectionType);

				if(hfType != FL_HDRFTR_NONE)
				{
					const gchar* pszID = NULL;
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
					*psfh = (fl_ContainerLayout*)pSL;
					
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
	case PTX_SectionEndnote:
	case PTX_SectionAnnotation:
	{
		bool isFoot = (pcrx->getStruxType() == PTX_SectionFootnote);
		UT_ASSERT(m_pCurrentSL);
		m_bFootnoteInProgress = true;
		fl_SectionLayout * pSL = NULL;
		if(isFoot)
		{
			UT_DEBUGMSG(("fl_DocListener::populateStrux for 'SectionFootnote'\n"));
			pSL = (fl_SectionLayout *) m_pCurrentSL->append(sdh, pcr->getIndexAP(),FL_CONTAINER_FOOTNOTE);
		}
		else if(pcrx->getStruxType() == PTX_SectionEndnote)
		{
			UT_DEBUGMSG(("fl_DocListener::populateStrux for 'SectionEndnote'\n"));
			pSL = (fl_SectionLayout *) m_pCurrentSL->append(sdh, pcr->getIndexAP(),FL_CONTAINER_ENDNOTE);
		}
		else if(pcrx->getStruxType() == PTX_SectionAnnotation)
		{
			UT_DEBUGMSG(("fl_DocListener::populateStrux for 'SectionAnnotation'\n"));
			pSL = (fl_SectionLayout *) m_pCurrentSL->append(sdh, pcr->getIndexAP(),FL_CONTAINER_ANNOTATION);
		}
		else
		  {
		    UT_DEBUGMSG(("Unknown strux type in footnotes \n"));
		    UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		  }

		*psfh = (fl_ContainerLayout*)pSL;
		m_pCurrentSL = (fl_SectionLayout*)pSL;
		break;
	}

	case PTX_EndFootnote:
	case PTX_EndEndnote:
	case PTX_EndAnnotation:
	{
//
// CurrentSL is a Footnote. Return this and set the m_pCurrentSL to it's 
// container
//
		bool isFoot = (pcrx->getStruxType() == PTX_EndFootnote);
		fl_ContainerLayout * pCL = m_pCurrentSL;
		m_bFootnoteInProgress = false;
		m_bEndFootnoteProcessedInBlock = true;
#if DEBUG
		if(isFoot)
		{
			UT_DEBUGMSG(("fl_DocListener::populateStrux for 'EndFootnote'\n"));
			UT_ASSERT(pCL->getContainerType() == FL_CONTAINER_FOOTNOTE);
		}
		else if(pcrx->getStruxType() == PTX_EndEndnote)
		{
			UT_DEBUGMSG(("fl_DocListener::populateStrux for 'EndEndnote'\n"));
			UT_ASSERT(pCL->getContainerType() == FL_CONTAINER_ENDNOTE);
		}
		else if(pcrx->getStruxType() == PTX_EndAnnotation)
		{
			UT_DEBUGMSG(("fl_DocListener::populateStrux for 'EndAnnotation'\n"));
			UT_ASSERT(pCL->getContainerType() == FL_CONTAINER_ANNOTATION);
		}
#endif
		*psfh = (fl_ContainerLayout*) pCL;
		pCL->setEndStruxDocHandle(sdh);
		m_pCurrentSL = (fl_SectionLayout *) static_cast<fl_EmbedLayout *>(m_pCurrentSL)->getDocSectionLayout();
		fl_BlockLayout * pBL = NULL;
		if(isFoot)
		{
			fl_FootnoteLayout * pFL = (fl_FootnoteLayout *) pCL;
			pFL->setFootnoteEndIn();
			pBL = (fl_BlockLayout *) pFL->getFirstLayout();
		}
		else if(pcrx->getStruxType() == PTX_EndEndnote)
		{
			fl_EndnoteLayout * pEL = (fl_EndnoteLayout *) pCL;
			pEL->setFootnoteEndIn();
			pBL = (fl_BlockLayout *) pEL->getFirstLayout();
		}
		else if(pcrx->getStruxType() == PTX_EndAnnotation)
		{
			fl_AnnotationLayout * pAL = (fl_AnnotationLayout *) pCL;
			pAL->setFootnoteEndIn();
			pBL = (fl_BlockLayout *) pAL->getFirstLayout();
		}
		UT_ASSERT(pBL);
		if(pBL)
		{
			pBL->updateEnclosingBlockIfNeeded();
		}
		UT_ASSERT(m_pCurrentSL);
		UT_ASSERT(m_pCurrentSL->getContainerType() == FL_CONTAINER_DOCSECTION);
		break;
	}
	case PTX_SectionTOC:
	{
		UT_ASSERT(m_pCurrentSL);
	    fl_SectionLayout * pSL = NULL;
		UT_DEBUGMSG(("fl_DocListener::populateStrux for 'SectionTOC'\n"));
		pSL = (fl_SectionLayout *) m_pCurrentSL->append(sdh, pcr->getIndexAP(),FL_CONTAINER_TOC);
		*psfh = (fl_ContainerLayout*)pSL;
		m_pCurrentSL = (fl_SectionLayout*)pSL;
		break;
	}

	case PTX_EndTOC:
	{
//
// CurrentSL is a TOC. Return this and set the m_pCurrentSL to it's 
// container
//
		fl_ContainerLayout * pCL = m_pCurrentSL;
#if DEBUG
		UT_DEBUGMSG(("fl_DocListener::populateStrux for 'EndTOC'\n"));
		UT_ASSERT(pCL->getContainerType() == FL_CONTAINER_TOC);
#endif
		*psfh = (fl_ContainerLayout*) pCL;
		pCL->setEndStruxDocHandle(sdh);		
		static_cast<fl_TOCLayout *>(pCL)->setTOCEndIn();
		m_pCurrentSL = (fl_SectionLayout *) static_cast<fl_TOCLayout *>(m_pCurrentSL)->getDocSectionLayout();
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
			const gchar* pszSectionType = NULL;
			pAP->getAttribute("type", pszSectionType);
			if (
				!pszSectionType
				|| (0 == strcmp(pszSectionType, "doc"))
				)
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
			else
			{
				HdrFtrType hfType = s_convertToHdrFtrType(pszSectionType);
				if(hfType != FL_HDRFTR_NONE)
				{
					const gchar* pszID = NULL;
					pAP->getAttribute("id", pszID);

					fl_DocSectionLayout* pDocSL = NULL;
					if(pszID)
						pDocSL = m_pLayout->findSectionForHdrFtr((char*)pszID);
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
					*psfh = (fl_ContainerLayout*)pSL;
					UT_DEBUGMSG(("Sevior: HeaderFooter created %p \n",pSL));
					
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
		if(m_pCurrentSL == NULL)
		{
			m_pDoc->miniDump(sdh,6);
		}
		UT_ASSERT(m_pCurrentSL);
//
// Look if we're inside a table. If so append this block to a cell.
//
		fl_ContainerLayout*	pCL = NULL;
		fl_ContainerLayout * pCon = getTopContainerLayout();
		if((pCon != NULL)  && (m_pCurrentSL->getContainerType() != FL_CONTAINER_FOOTNOTE) && (m_pCurrentSL->getContainerType() != FL_CONTAINER_ENDNOTE) && (m_pCurrentSL->getContainerType() != FL_CONTAINER_ANNOTATION))
		{
			if(pCon->getContainerType() != FL_CONTAINER_CELL)
			{
#ifdef DEBUG
			m_pDoc->miniDump(sdh,6);
#endif

				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				return false;
			}
			fl_CellLayout * pCell = (fl_CellLayout *) pCon;
			xxx_UT_DEBUGMSG(("SEVIOR: Appending block to cell %p \n",pCell));
			// Append a new BlockLayout to this cell

			pCL = pCell->append(sdh, pcr->getIndexAP(),FL_CONTAINER_BLOCK);
			if (!pCL)
			{
				UT_DEBUGMSG(("no memory for BlockLayout"));
				return false;
			}
			// BUGBUG: this is *not* thread-safe, but should work for now
#ifdef ENABLE_SPELL
			if (m_bScreen)
			{
				UT_uint32 reason =  0;
				if( m_pLayout->getAutoSpellCheck())
				{
					reason = (UT_uint32) FL_DocLayout::bgcrSpelling;
				}
				if( m_pLayout->getAutoGrammarCheck())
				{
					reason = reason | (UT_uint32) FL_DocLayout::bgcrGrammar;
				}
				m_pLayout->queueBlockForBackgroundCheck(reason, (fl_BlockLayout *)pCL,false);
			}
#endif
		}
		else
		{
			UT_return_val_if_fail( m_pCurrentSL,false );
			pCL = m_pCurrentSL->append(sdh, pcr->getIndexAP(),FL_CONTAINER_BLOCK);
			if (!pCL)
			{
				UT_DEBUGMSG(("no memory for BlockLayout"));
				return false;
			}

			// BUGBUG: this is *not* thread-safe, but should work for now
#ifdef ENABLE_SPELL
			if (m_bScreen)
			{
				UT_uint32 reason =  0;
				if( m_pLayout->getAutoSpellCheck())
				{
					reason = (UT_uint32) FL_DocLayout::bgcrSpelling;
				}
				if( m_pLayout->getAutoGrammarCheck())
				{
					reason = reason | (UT_uint32) FL_DocLayout::bgcrGrammar;
				}
				m_pLayout->queueBlockForBackgroundCheck(reason, (fl_BlockLayout *)pCL,false);
			}
#endif
		}

		*psfh = (fl_ContainerLayout*)pCL;
		if(pCL->getLastContainer()==NULL)
		{
			if(pCL->getSectionLayout()->getType() != FL_SECTION_HDRFTR && pCL->getPrev() != NULL)
			{
				UT_DEBUGMSG(("In DocListner no LastLine in block append. Fixing this now \n"));
				UT_DEBUGMSG(("getPrev = %p this = %p \n",pCL->getPrev(),pCL));
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
		UT_DEBUGMSG(("!!!!Appending Table \n"));
		if(m_pCurrentSL->getHdrFtrLayout())
		{
			UT_DEBUGMSG(("Appending Table into HdrFtr %p \n",m_pCurrentSL->getHdrFtrLayout()));
		}
		if(pCon == NULL)
		{
			pCL = m_pCurrentSL->append(sdh, pcr->getIndexAP(),FL_CONTAINER_TABLE);
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
		*psfh = (fl_ContainerLayout*)pCL;
//
// Don't layout until a endTable strux
//
		m_pDoc->setDontImmediatelyLayout(true);
	}
	break;
	case PTX_SectionFrame:
	{
		UT_ASSERT(m_pCurrentSL);
		UT_ASSERT(m_pCurrentSL->getContainerType() == FL_CONTAINER_DOCSECTION);
		fl_ContainerLayout*	pCL2 = NULL;
//
// Look to see if we're in a table.
//
		fl_ContainerLayout * pCon = getTopContainerLayout();
		if((pCon != NULL))
		{
			if(pCon->getContainerType() == FL_CONTAINER_CELL)
			{
				fl_ContainerLayout * pCL = pCon->append(sdh,pcr->getIndexAP(),FL_CONTAINER_FRAME);
				m_pCurrentSL = static_cast<fl_SectionLayout *>(pCL);
				*psfh = (fl_ContainerLayout*)pCL;
				break;
			}
#ifdef DEBUG
			m_pDoc->miniDump(sdh,6);
#endif
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}

		// Append a new FrameLayout to the SectionLayout


		UT_DEBUGMSG(("!!!!Appending Frame \n"));
		pCL2 = m_pCurrentSL->append(sdh, pcr->getIndexAP(),FL_CONTAINER_FRAME);
		if (!pCL2)
		{
			UT_DEBUGMSG(("no memory for TableLayout"));
			return false;
		}
		m_pCurrentSL = static_cast<fl_SectionLayout *>(pCL2);
		*psfh = (fl_ContainerLayout*)pCL2;
	}
	break;
	case PTX_EndFrame:
	{
		UT_ASSERT(m_pCurrentSL);
		UT_ASSERT(m_pCurrentSL->getContainerType() == FL_CONTAINER_FRAME);
		// Append a new FrameLayout to the SectionLayout

		fl_ContainerLayout*	pCL = NULL;
		UT_DEBUGMSG(("!!!!Appending EndFrame \n"));
		pCL = m_pCurrentSL;
		*psfh = (fl_ContainerLayout*)pCL;
		pCL->setEndStruxDocHandle(sdh);
		m_pCurrentSL = static_cast<fl_SectionLayout *>(pCL->myContainingLayout());
		if(m_pCurrentSL->getContainerType() == FL_CONTAINER_CELL)
		{
			m_pCurrentSL = static_cast<fl_CellLayout *>(m_pCurrentSL)->getDocSectionLayout();
		}
		UT_ASSERT(m_pCurrentSL->getContainerType() == FL_CONTAINER_DOCSECTION);
	}
	break;

	case PTX_SectionCell:
	{
		UT_ASSERT(m_pCurrentSL);
		xxx_UT_DEBUGMSG(("!!!! Append Section Cell \n"));
		
		// Append a new CallLayout to the Current TableLayout
		fl_ContainerLayout * pCon = getTopContainerLayout();
		UT_return_val_if_fail(pCon, false);
		if(pCon->getContainerType() != FL_CONTAINER_TABLE)
		{
#ifdef DEBUG
			m_pDoc->miniDump(sdh,6);
#endif
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
		fl_TableLayout * pTable = (fl_TableLayout *) pCon;
		if(pTable->getHdrFtrLayout())
		{
			UT_DEBUGMSG(("Appending a Cell to a Table in a HDrFtr %p \n",pTable->getHdrFtrLayout()));
		}
		fl_ContainerLayout*	pCL = pTable->append(sdh, pcr->getIndexAP(),FL_CONTAINER_CELL);
		xxx_UT_DEBUGMSG(("SEVIOR: Appending Cell: layout is %p \n",pCL));
		pTable->attachCell(pCL);
		if (!pCL)
		{
			UT_DEBUGMSG(("no memory for CellLayout"));
			return false;
		}
		pushContainerLayout(pCL);
		*psfh = (fl_ContainerLayout*)pCL;
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
		*psfh = (fl_ContainerLayout*)pCon;
		pCon->setEndStruxDocHandle(sdh);
		fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pCon);
		UT_DEBUGMSG(("SEVIOR: End table in doclistener \n"));
		pTL->setDirty();
		pTL->setEndTableIn();
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
		xxx_UT_DEBUGMSG(("!!!! Append End Cell \n"));
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
		*psfh = (fl_ContainerLayout*) pCon;
		pCon->setEndStruxDocHandle(sdh);
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
bool fl_DocListener::change(fl_ContainerLayout* sfh,
							const PX_ChangeRecord * pcr)
{
	UT_return_val_if_fail( sfh, false );
	
	bool bResult = false;
	AV_ChangeMask chgMask = AV_CHG_NONE;

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
		if(pL->getType() != PTX_Block)
		{
			m_pDoc->miniDump(pL->getStruxDocHandle(),6);
		}
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
		chgMask = AV_CHG_FMTCHAR;
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

		if(!sfh)
		{
			// sometimes happens with revisions, not sure if this is a real problem, but
			// assert anyway
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			goto finish_up;
		}
		
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
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_Section);
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
		case PTX_SectionTOC:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_SectionTOC);
			fl_TOCLayout * pFL = static_cast<fl_TOCLayout *>(pL);
			pFL->doclistener_deleteStrux(pcrx);
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
		case PTX_SectionAnnotation:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_SectionAnnotation);
			fl_AnnotationLayout * pAL = (fl_AnnotationLayout *) pL;
			pAL->doclistener_deleteStrux(pcrx);
			goto finish_up;
		}
		case PTX_SectionEndnote:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_SectionEndnote);
			fl_EndnoteLayout * pEL = (fl_EndnoteLayout *) pL;
			pEL->doclistener_deleteStrux(pcrx);
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
		case PTX_SectionFrame:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_SectionFrame);
			fl_FrameLayout * pFrameL = (fl_FrameLayout *) pL;
			pFrameL->doclistener_deleteStrux(pcrx);
			goto finish_up;
		}
		case PTX_EndFrame:
		{
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
		case PTX_EndTOC:
		{
			goto finish_up;
		}
		case PTX_EndFootnote:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_SectionFootnote);
			fl_FootnoteLayout * pFL = static_cast<fl_FootnoteLayout *>(pL);
			pFL->doclistener_deleteEndEmbed(pcrx);

			goto finish_up;
		}
		case PTX_EndAnnotation:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_SectionAnnotation);
			fl_AnnotationLayout * pAL = static_cast<fl_AnnotationLayout *>(pL);
			pAL->doclistener_deleteEndEmbed(pcrx);

			goto finish_up;
		}
		case PTX_EndEndnote:
		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_SectionEndnote);
			fl_EndnoteLayout * pEL = static_cast<fl_EndnoteLayout *>( pL);
			pEL->doclistener_deleteEndEmbed(pcrx);

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

		fl_Layout * pL2 = (fl_Layout *)sfh;

		// TODO getOldIndexAP() is only intended for use by the document.
		// TODO this assert is probably wrong. --- BUT EVERYTIME IT HAS
		// TODO GONE OFF, I'VE FOUND A BUG, SO MAYBE WE SHOULD KEEP IT :-)
		// UT_ASSERT(pL->getAttrPropIndex() == pcrxc->getOldIndexAP());
		// UT_ASSERT(pL->getAttrPropIndex() != pcr->getIndexAP());

		switch (pL2->getType())
		{
		case PTX_Section:
		case PTX_SectionEndnote:
		case PTX_SectionAnnotation:
		case PTX_SectionFootnote:
		{
			fl_DocSectionLayout* pSL = static_cast<fl_DocSectionLayout*>(pL2);
			
			PT_AttrPropIndex indexAP = pcr->getIndexAP();
			const PP_AttrProp* pAP = NULL;
			
			bool bres = (m_pDoc->getAttrProp(indexAP, &pAP) && pAP);
			UT_ASSERT(bres);
			if(!bres)
			{
				UT_WARNINGMSG(("getAttrProp() failed in %s:%d",
							   __FILE__, __LINE__));
			}
			pf_Frag_Strux* sdh = pL2->getStruxDocHandle();
	
			const gchar* pszSectionType = NULL;
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
				const gchar* pszID = NULL;
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
			// among other things, this is the case where style
			// definitions vere changed and we are updating the block;
			// we need to notify view listeners for both block and
			// char fmt changes (i.e., we want the font combo updated,
			// etc.) This is not ideal, we should probably have some
			// way of telling the change includes a style change
			// Tomas, June 7, 2003
			
			chgMask = AV_CHG_FMTBLOCK | AV_CHG_FMTCHAR;
			fl_BlockLayout * pCL = static_cast<fl_BlockLayout *>(pL2);
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
			if(pcrxc->isRevisionDelete())
			{
				fl_Layout * pL = (fl_Layout *)sfh;
				UT_ASSERT(pL->getType() == PTX_SectionHdrFtr);
				fl_HdrFtrSectionLayout * pSL = static_cast<fl_HdrFtrSectionLayout *>(pL);
				//
				// Nuke the HdrFtrSectionLayout and the shadows associated with it.
				//
				pSL->doclistener_deleteStrux(pcr); 
				m_pLayout->updateLayout();
				goto finish_up;
			}
			
//
// OK A previous "insertStrux" has created a HdrFtrSectionLayout but it
// Doesn't know if it's a header or a footer or the DocSection and hences pages
// It associated with. Tell it now.
//
			fl_HdrFtrSectionLayout* pHFSL = static_cast<fl_HdrFtrSectionLayout*>(pL2);
			
			PT_AttrPropIndex indexAP = pcr->getIndexAP();
//
// Save this new index to the Attributes/Properties of this section in the
// HdrFtrSection Class
//
			pHFSL->setAttrPropIndex(pcrxc->getIndexAP());
			const PP_AttrProp* pHFAP = NULL;
			
			bool bres = (m_pDoc->getAttrProp(indexAP, &pHFAP) && pHFAP);
			UT_ASSERT(bres);
			if(!bres)
			{
				UT_WARNINGMSG(("getAttrProp() failed in %s:%d",
							   __FILE__, __LINE__));
			}
	
			const gchar* pszHFSectionType = NULL;
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
				const gchar* pszHFID = NULL;
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
			fl_TableLayout * pTL = (fl_TableLayout *) pL2;
			UT_ASSERT(pTL->getContainerType() == FL_CONTAINER_TABLE);
 			bResult = pTL->doclistener_changeStrux(pcrxc);
			goto finish_up;
		}
		case PTX_SectionCell:
		{
			fl_CellLayout * pCL = (fl_CellLayout *) pL2;
			UT_ASSERT(pCL->getContainerType() == FL_CONTAINER_CELL);
			bResult = pCL->doclistener_changeStrux(pcrxc);
			goto finish_up;
		}
		case PTX_SectionFrame:
		{
			fl_FrameLayout * pFL = (fl_FrameLayout *) pL2;
			UT_ASSERT(pFL->getContainerType() == FL_CONTAINER_FRAME);
			bResult = pFL->doclistener_changeStrux(pcrxc);
			goto finish_up;
		}
		case PTX_SectionTOC:
		{
			fl_TOCLayout * pTOCL = (fl_TOCLayout *) pL2;
			UT_ASSERT(pTOCL->getContainerType() == FL_CONTAINER_TOC);
			bResult = pTOCL->doclistener_changeStrux(pcrxc);
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

		// if the inserted object is a bookmark, we have to notify TOCs of bookmark change in case
		// any of them is restricted by the given bookmark
		if(pcro->getObjectType() == PTO_Bookmark)
		{
			if(pL->getType() == PTX_Block)
			{
				fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
				fp_Run * pRun = pBL->findRunAtOffset(pcro->getBlockOffset());
				if(pRun && pRun->getType() == FPRUN_BOOKMARK)
				{
					fp_BookmarkRun * pB = static_cast<fp_BookmarkRun*>(pRun);

					// only do this when the end-object is inserted ...
					if(!pB->isStartOfBookmark())
						m_pLayout->updateTOCsOnBookmarkChange(pB->getName());
				}
				else if(pRun && pRun->getType() != FPRUN_BOOKMARK)
				{
					UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
				}
			}
		}
		
		goto finish_up;
	}
	case PX_ChangeRecord::PXT_DeleteObject:
	{
		const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);

		fl_Layout * pL = (fl_Layout *)sfh;
		UT_ASSERT(pL->getType() == PTX_Block);

		// if the deleted object is a bookmark, we have to notify TOCs of bookmark change in case
		// any of them is restricted by the given bookmark;
		//
		// this has to be done in two steps: before the layout is updated, we have to find out the
		// name of the bookmark; after the layout is updated, we need to notify the TOCs.
		UT_UTF8String sBookmark;
		
		if(pcro->getObjectType() == PTO_Bookmark)
		{
			if(pL->getType() == PTX_Block)
			{
				fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pL);
				fp_Run * pRun = pBL->findRunAtOffset(pcro->getBlockOffset());
				if(pRun && pRun->getType() == FPRUN_BOOKMARK)
				{
					fp_BookmarkRun * pB = static_cast<fp_BookmarkRun*>(pRun);

					// only do this when the end-object is deleted ...
					// (in case of deletion, this is an arbitrary choice)
					if(!pB->isStartOfBookmark())
						sBookmark = pB->getName();
				}
				else if(pRun && pRun->getType() != FPRUN_BOOKMARK)
				{
					UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
				}
			}
		}

		// now we can proceed to delete the object representation ...
		fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
		fl_SectionLayout* pCLSL = pCL->getSectionLayout();
		if(pCLSL->getType() == FL_SECTION_SHADOW)
		{
			fl_HdrFtrSectionLayout * pHdr = pCLSL->getHdrFtrSectionLayout();
			bResult = pHdr->bl_doclistener_deleteObject(pCL, pcro);
		}
		else
			bResult = pCLSL->bl_doclistener_deleteObject(pCL, pcro);

		// now notify TOCs if necessary
		if(sBookmark.size())
			m_pLayout->updateTOCsOnBookmarkChange(sBookmark.utf8_str());

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
		xxx_UT_DEBUGMSG(("ContainerLayout %x ContainerType %s \n",pCL,pCL->getContainerString()));
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
	case PX_ChangeRecord::PXT_CreateDataItem:
	{
	        bResult = true;
		goto finish_up;
	}	
	case PX_ChangeRecord::PXT_ChangeDocProp:
	{
	        PT_AttrPropIndex iAP = pcr->getIndexAP();
		const PP_AttrProp * pAP = NULL;
		m_pLayout->getDocument()->getAttrProp(iAP, &pAP);
		const gchar * szValue=NULL;
		bool b= pAP->getAttribute( PT_DOCPROP_ATTRIBUTE_NAME,szValue);
		UT_DEBUGMSG(("Doing DocProp change value %s \n",szValue));
		if(!b)
		{
		    bResult = false;
		    goto finish_up;
		}
		if(strcmp(szValue,"pagesize") == 0)
		{
		    bResult = m_pLayout->setDocViewPageSize(pAP);
		}
		else if(strcmp(szValue,"changeauthor") == 0)
		{
		  	m_pLayout->refreshRunProperties();
			m_pLayout->setNeedsRedraw();
		}
		else if(strcmp(szValue,"addauthor") == 0)
		{
		  	m_pLayout->refreshRunProperties();
			FV_View * pView = m_pLayout->getView();
			const gchar * szAuthorId = NULL;
			pAP->getProperty("id",szAuthorId);
			if(szAuthorId && *szAuthorId)
			{
			  UT_sint32 id = atoi(szAuthorId);
			  pView->addCaret(0,id);
			}
			m_pLayout->setNeedsRedraw();
		}
		goto finish_up;
	}	
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

	// need to make view to notify its various listeners to update
	if(chgMask != AV_CHG_NONE)
	{
		UT_return_val_if_fail(getLayout() && getLayout()->getView(),bResult);
		if (m_bCacheChanges)
		{
			m_chgMaskCached |= chgMask;
		}
		else
		{
			UT_return_val_if_fail(getLayout() && getLayout()->getView(),bResult);
			// first of all, increase view tick, so that the view's
			// property caches are invalidated ...
			getLayout()->getView()->incTick();
			fl_ContainerLayout * pCL = (fl_ContainerLayout *)sfh;
			bool doNotify = true;
			if(pCL->isCollapsed())
			{
				doNotify = false;
			}
			if(doNotify)
			{
				UT_DEBUGMSG(("Doing view notification \n"));
				getLayout()->getView()->notifyListeners(chgMask);
			}
			else
			{
				m_chgMaskCached |= chgMask;
			}
		}
	}
	
	return bResult;
}

void fl_DocListener::deferNotifications(void)
{
	m_bCacheChanges = true;
	m_chgMaskCached = AV_CHG_NONE;
}

void fl_DocListener::processDeferredNotifications(void)
{
	if (m_chgMaskCached != AV_CHG_NONE)
	{
		if (getLayout() && getLayout()->getView())
		{
			// first of all, increase view tick, so that the view's
			// property caches are invalidated ...
			getLayout()->getView()->incTick();
			getLayout()->getView()->notifyListeners(m_chgMaskCached);
		}
		m_chgMaskCached = AV_CHG_NONE;
	}
	m_bCacheChanges = false;
}


/*!
 */
bool fl_DocListener::insertStrux(fl_ContainerLayout* sfh,
								 const PX_ChangeRecord * pcr,
								 pf_Frag_Strux* sdh,
								 PL_ListenerId lid,
								 void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
														 PL_ListenerId lid,
														 fl_ContainerLayout* sfhNew))
{
	UT_DEBUGMSG(("fl_DocListener::insertStrux at pos %d \n",pcr->getPosition()));
	UT_return_val_if_fail( sdh && pcr, false );
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
	else if(pcrx->getStruxType() == PTX_SectionAnnotation)
	{
		UT_DEBUGMSG(("Inserting SectionAnnotation strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_EndFootnote)
	{
		UT_DEBUGMSG(("Inserting EndFootnote strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_EndAnnotation)
	{
		UT_DEBUGMSG(("Inserting EndAnnotation strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_SectionTOC)
	{
		UT_DEBUGMSG(("Inserting SectionTOC strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_EndTOC)
	{
		UT_DEBUGMSG(("Inserting EndToc strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_SectionEndnote)
	{
		UT_DEBUGMSG(("Inserting SectionEndnote strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_EndEndnote)
	{
		UT_DEBUGMSG(("Inserting EndEndnote strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_SectionTable)
	{
		UT_DEBUGMSG(("Inserting SectionTable strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_EndTable)
	{
		UT_DEBUGMSG(("Inserting EndTable strux at pos %d \n",pcr->getPosition()));
		m_pDoc->miniDump(sdh,8);

	}
	else if(pcrx->getStruxType() == PTX_SectionCell)
	{
		UT_DEBUGMSG(("Inserting Cell strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_EndCell)
	{
		UT_DEBUGMSG(("Inserting EndCell strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_SectionFrame)
	{
		UT_DEBUGMSG(("Inserting Frame strux at pos %d \n",pcr->getPosition()));
	}
	else if(pcrx->getStruxType() == PTX_EndFrame)
	{
		UT_DEBUGMSG(("Inserting EndFrame strux at pos %d \n",pcr->getPosition()));
	}
#endif
#endif
	fl_Layout * pL = (fl_Layout *)sfh;
	UT_return_val_if_fail(pL,false);
	xxx_UT_DEBUGMSG(("Previous strux %x type %d \n",pL, pL->getType()));
	xxx_UT_DEBUGMSG(("Insert strux type %d \n",pcrx->getStruxType()));
	switch (pL->getType())				// see what the immediately prior strux is.
	{
	case PTX_Section:		   // the immediately prior strux is a section.
	case PTX_SectionHdrFtr:	   //  ... or a HdrFtr.
	case PTX_SectionFootnote:  //  ... or a Footnote.
	case PTX_SectionAnnotation:  //  ... or a Annotation.
	case PTX_SectionEndnote:  //  ... or a Endnote.
    {
		switch (pcrx->getStruxType())	// see what we are inserting.
		{
		case PTX_Section:				// we are inserting a section.
		case PTX_SectionHdrFtr:			//  ... or a HdrFtr section.
		case PTX_SectionFootnote:        //  ... or a Footnote section.
		case PTX_SectionAnnotation:        //  ... or a Annotation section.
		case PTX_SectionEndnote:        //  ... or a Endnote section.
		case PTX_SectionTOC:  //  ... or a Table Of Contents.
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
#if 1
//
// FIXME When I get brave I'll see if we can't make a table the first layout
// in a section.
//
		case PTX_SectionTable:
		{
			UT_DEBUGMSG(("Insert Table immediately after Section \n"));
			fl_SectionLayout * pSL = static_cast<fl_SectionLayout *>(pL);
			return pSL->bl_doclistener_insertTable(FL_SECTION_TABLE, pcrx,sdh,lid,pfnBindHandles) != NULL;
		}
#endif
		default:						// unknown strux.
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
	}	
	case PTX_SectionTOC:        //  ... or a TOC section.
	{
		switch (pcrx->getStruxType())	// see what we are inserting.
		{
		case PTX_EndTOC:
		   {
			   fl_TOCLayout * pCL = static_cast<fl_TOCLayout*>(pL);

			   fl_DocSectionLayout* pDSL = 
				   static_cast<fl_DocSectionLayout *>(pCL->myContainingLayout());
			   UT_ASSERT(pCL->getContainerType() == FL_CONTAINER_TOC);
			   UT_ASSERT(pDSL->getContainerType() == FL_CONTAINER_DOCSECTION);
			   bool bResult = pCL->bl_doclistener_insertEndTOC(pCL, pcrx,sdh,lid,pfnBindHandles);
			   m_pCurrentSL = static_cast<fl_SectionLayout *>(pDSL);
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
		case PTX_SectionTOC:
		   {
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   fl_SectionLayout* pCLSL = pCL->getSectionLayout();

			   bool bResult = pCLSL->bl_doclistener_insertSection(pCL, FL_SECTION_TOC, pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		case PTX_SectionFootnote:
		   {
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   fl_SectionLayout* pCLSL = pCL->getSectionLayout();

			   bool bResult = pCLSL->bl_doclistener_insertSection(pCL, FL_SECTION_FOOTNOTE, pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		case PTX_SectionAnnotation:
		   {
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   fl_SectionLayout* pCLSL = pCL->getSectionLayout();

			   bool bResult = pCLSL->bl_doclistener_insertSection(pCL, FL_SECTION_ANNOTATION, pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		case PTX_SectionEndnote:
		   {
		           UT_DEBUGMSG(("Doing valid endnote insertion into block \n"));
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   fl_SectionLayout* pCLSL = pCL->getSectionLayout();

			   bool bResult = pCLSL->bl_doclistener_insertSection(pCL, FL_SECTION_ENDNOTE, pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		case PTX_SectionTable:				// we are inserting a section.
		   {
			   // The immediately prior strux is a block.  
			   // OK this creates a table in the document.
			   UT_DEBUGMSG(("Insert Table into Block \n"));
			
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   UT_DEBUGMSG(("Containing Section is %d \n",pL->getType()));
			   UT_DEBUGMSG(("Doing Insert Strux Table Into Prev Block \n"));
			   xxx_UT_DEBUGMSG(("Doing Insert Table Correctly \n"));
			   fl_SectionLayout* pCLSL = pCL->getSectionLayout();
			   bool bResult = (pCLSL->bl_doclistener_insertTable(pCL,FL_SECTION_TABLE, pcrx,sdh,lid,pfnBindHandles) != 0);
			   return bResult;
		   }
		case PTX_SectionFrame:				// we are inserting a section.
		   {
			   // The immediately prior strux is a block.  
			   // OK this creates a table in the document.
			   UT_DEBUGMSG(("Insert Frame after this Block \n"));
			
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   UT_DEBUGMSG(("Doing Insert Strux Frame Into Prev Block \n"));
			   fl_SectionLayout* pCLSL = pCL->getSectionLayout();
			   bool bResult = (pCLSL->bl_doclistener_insertFrame(pCL,FL_SECTION_FRAME, pcrx,sdh,lid,pfnBindHandles) != 0);
			   return bResult;
		   }
		case PTX_EndFrame:				// we are inserting an endFrame.
		   {
			   // The immediately prior strux is a block.  
			   // OK this finishes a Frame on this page

			   UT_DEBUGMSG(("Insert endFrame into Block \n"));
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   UT_DEBUGMSG(("Doing Insert Strux EndFrame adter Prev Block \n"));
//
// This gets us a fl_FRAMELAYOUTCell.
//
			   fl_FrameLayout* pCLSL = (fl_FrameLayout *) pCL->myContainingLayout();
			   if(pCLSL->getContainerType() != FL_CONTAINER_FRAME)
			   {
				   m_pDoc->miniDump(pL->getStruxDocHandle(),6);
			   }
			   UT_ASSERT(pCLSL->getContainerType() == FL_CONTAINER_FRAME);
			   bool bResult = pCLSL->bl_doclistener_insertEndFrame(pCL, pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		case PTX_EndFootnote:
		   {
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout*>(pL);

			   fl_FootnoteLayout* pCLSL = 
				   static_cast<fl_FootnoteLayout *>(pCL->myContainingLayout());
			   UT_ASSERT(pCLSL->getContainerType() == FL_CONTAINER_FOOTNOTE);
			   bool bResult = pCLSL->bl_doclistener_insertEndEmbed(pCL, pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		case PTX_EndAnnotation:
		   {
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout*>(pL);

			   fl_AnnotationLayout* pCLSL = 
				   static_cast<fl_AnnotationLayout *>(pCL->myContainingLayout());
			   UT_ASSERT(pCLSL->getContainerType() == FL_CONTAINER_ANNOTATION);
			   bool bResult = pCLSL->bl_doclistener_insertEndEmbed(pCL, pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		case PTX_EndEndnote:
		   {
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout*>(pL);

			   fl_EndnoteLayout* pCLSL = 
				   (fl_EndnoteLayout *)pCL->myContainingLayout();
			   UT_ASSERT(pCLSL->getContainerType() == FL_CONTAINER_ENDNOTE);
			   bool bResult = pCLSL->bl_doclistener_insertEndEmbed(pCL, pcrx,sdh,lid,pfnBindHandles);
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
			   m_pDoc->miniDump(pL->getStruxDocHandle(),3);			   
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
		case PTX_EndCell:				// we are inserting an endcell.
		{
			   // The immediately prior strux is a Table. This can happen
			// if were pasting an endcell after a nested table. We get the
			// the enclosing cell of the table and close it.


			   UT_DEBUGMSG(("Insert endCell into (hopefully) nested table \n"));
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   fl_CellLayout* pCLSL = (fl_CellLayout *) pCL->myContainingLayout();
//
// This gets us a fl_SectionCell.
//
			   if(pCLSL->getContainerType() != FL_CONTAINER_CELL)
			   {
				   m_pDoc->miniDump(pL->getStruxDocHandle(),6);
				   UT_DEBUGMSG(("Pasting endcell into non nested table %d \n",pcrx->getStruxType()));
				   UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				   return false;
			   }
			   UT_DEBUGMSG(("Doing Insert Strux EndCell Into Cell containing table \n"));
			   bool bResult = pCLSL->bl_doclistener_insertEndCell(pCLSL, pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		case PTX_SectionTOC:				// we are inserting a TOC.
		{
			   // The immediately prior strux is a Table. This can happen
			// if were inserting a TOC after an EndTable Strux.
            // We have write specal code to deal with it.

			   fl_ContainerLayout * pCL = static_cast<fl_SectionLayout *>(pL);
			   fl_SectionLayout * pCLSL = static_cast<fl_SectionLayout *>(pCL->myContainingLayout());
			   bool bResult = pCLSL->bl_doclistener_insertSection(pCL, FL_SECTION_TOC, pcrx,sdh,lid,pfnBindHandles);
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
			  fl_ContainerLayout * pConL = static_cast<fl_ContainerLayout *>(pL);
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

			  fl_ContainerLayout * pConL = static_cast<fl_ContainerLayout *>(pL);
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

			  fl_ContainerLayout * pConL = static_cast<fl_ContainerLayout *>(pL);
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

			  fl_ContainerLayout * pConL = static_cast<fl_ContainerLayout *>(pL);
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
			  m_pDoc->miniDump(pL->getStruxDocHandle(),3);
			  UT_DEBUGMSG(("Not Implemented \n"));
			  UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			  break;
		  }
			
		}

	}
    case PTX_SectionFrame:						// The immediately prior strux is a Frame
    {
		switch (pcrx->getStruxType())	// see what we are inserting.
		{
		case PTX_Block:				// we are inserting a block into a frame. This is valid
		  {
			   // The immediately prior strux is a Frame.  So, this
			   // will become the first block of the Frame.

			  UT_DEBUGMSG(("Inserting block into frame \n"));
//
// Actually the block could be being inserted right *after* the frame.
// We need to detect this and deal with this. We can do this in fl_FrameLayout
//
			  fl_SectionLayout * pSL = static_cast<fl_SectionLayout *>(pL);
			  UT_ASSERT(pSL->getContainerType() == FL_CONTAINER_FRAME);
			  fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(pL);
			  if(!pFL->isEndFrameIn())
			  {
				  bool bResult = pSL->bl_doclistener_insertBlock(NULL, pcrx,sdh,lid,pfnBindHandles);
				  return bResult;
			  }
			  PT_DocPosition posEnd = pFL->getPosition(true) + pFL->getLength()-1;
			  if(posEnd >= pcrx->getPosition())
			  {
				  bool bResult = pSL->bl_doclistener_insertBlock(NULL, pcrx,sdh,lid,pfnBindHandles);
				  return bResult;
			  }
			  else
			  {
				  bool bResult = pFL->insertBlockAfter(NULL, pcrx,sdh,lid,pfnBindHandles);
				  return bResult;

			  }
		   }
		case PTX_SectionHdrFtr:
		   {
			   UT_DEBUGMSG(("Inserting HdrFtr after Frame \n"));
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   xxx_UT_DEBUGMSG(("Doing Insert Strux HdrFtr Into Prev Block \n"));
			   fl_SectionLayout* pCLSL = pCL->getSectionLayout();
			   bool bResult = pCLSL->bl_doclistener_insertSection(pCL, FL_SECTION_HDRFTR, pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
			   
		   }
		case PTX_EndFrame:
		   {
			   UT_DEBUGMSG(("Inserting EndFrame immediately after Frame \n"));
//
// This gets us a fl_FrameLayout
//
			   fl_FrameLayout* pCLSL = static_cast<fl_FrameLayout *>( pL);
			   if(pCLSL->getContainerType() != FL_CONTAINER_FRAME)
			   {
				   m_pDoc->miniDump(pL->getStruxDocHandle(),6);
			   }
			   UT_ASSERT(pCLSL->getContainerType() == FL_CONTAINER_FRAME);
			   bool bResult = pCLSL->bl_doclistener_insertEndFrame(NULL, pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		   }
		case PTX_SectionFrame:
		   {
			   UT_DEBUGMSG(("Inserting Frame immediately after Frame \n"));
//
// This gets us a fl_FrameLayout
//
			   fl_ContainerLayout* pCL = static_cast<fl_ContainerLayout *>( pL);
			   if(pCL->getContainerType() != FL_CONTAINER_FRAME)
			   {
				   m_pDoc->miniDump(pL->getStruxDocHandle(),6);
			   }
			   UT_ASSERT(pCL->getContainerType() == FL_CONTAINER_FRAME);
//
// Loop back for the block
//
			   while(pCL && pCL->getContainerType() != FL_CONTAINER_BLOCK)
			   {
				   pCL = pCL->getPrev();
			   }
			   UT_return_val_if_fail(pCL != NULL,false); 
			   UT_DEBUGMSG(("Doing Insert Strux Frame Into Prev Looped Block \n"));
			   fl_SectionLayout* pCLSL = pCL->getSectionLayout();
			   bool bResult = (pCLSL->bl_doclistener_insertFrame(pCL,FL_SECTION_FRAME, pcrx,sdh,lid,pfnBindHandles) != 0);
			   return bResult;
		   }
		case PTX_SectionTable:	  // we are inserting a Table as the first layout into a frame. This is valid
		  {
			   // The immediately prior strux is a Frame.  So, this
			   // will become the first table of the Frame.
		           fl_SectionLayout * pSL = static_cast<fl_SectionLayout *>(pL);
			   return pSL->bl_doclistener_insertTable(FL_SECTION_TABLE, pcrx,sdh,lid,pfnBindHandles) != NULL;

		  }
		case PTX_EndCell:	  
		  // we are inserting an endcell 
		  // following an endframe. This is valid if the frame is
		  // contained within the cell.
		  {

			   UT_DEBUGMSG(("Insert endCell after EndFrame \n"));
			   fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pL);
			   UT_DEBUGMSG(("Doing Insert Strux EndCell after endFrame \n"));
//
// This gets us a fl_SectionCell.
//
			   UT_ASSERT(pCL->getContainerType() == FL_CONTAINER_FRAME);
			   fl_CellLayout* pCLSL = static_cast<fl_CellLayout *>( pCL->myContainingLayout());
			   if(pCLSL->getContainerType() != FL_CONTAINER_CELL)
			   {
				   m_pDoc->miniDump(pL->getStruxDocHandle(),6);
			   }
			   UT_return_val_if_fail(pCLSL->getContainerType() == FL_CONTAINER_CELL,false);
			   bool bResult = pCLSL->bl_doclistener_insertEndCell(pCL, pcrx,sdh,lid,pfnBindHandles);
			   return bResult;
		  }

		default:
		   {
			   UT_DEBUGMSG(("Illegal strux type after frame %d \n",pcrx->getStruxType()));
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
	case PD_SIGNAL_REVISION_MODE_CHANGED:
		pView->updateRevisionMode();
		// fall through ...
		
	case PD_SIGNAL_DOCPROPS_CHANGED_REBUILD:
		m_pLayout->updatePropsRebuild();
		break;
	case PD_SIGNAL_DOCPROPS_CHANGED_NO_REBUILD:
		m_pLayout->updatePropsNoRebuild();
		break;
	case PD_SIGNAL_DOCNAME_CHANGED:
		m_pLayout->notifyListeners(AV_CHG_FILENAME);
		break;
	case PD_SIGNAL_DOCDIRTY_CHANGED:
		m_pLayout->notifyListeners(AV_CHG_DIRTY);
		break;
	case PD_SIGNAL_SAVEDOC:
		break;
	case PD_SIGNAL_DOCCLOSED:
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return true;
}

/*! Helper method to convert hf strings to HdrFtrType */
static HdrFtrType s_convertToHdrFtrType(const gchar * pszHFType)
{
	if (!pszHFType)
		return FL_HDRFTR_NONE;

	if (strcmp(pszHFType,"header") == 0)
		return FL_HDRFTR_HEADER;

	if (strcmp(pszHFType,"header-even") == 0)
		return FL_HDRFTR_HEADER_EVEN;

	if (strcmp(pszHFType,"header-first") == 0)
		return FL_HDRFTR_HEADER_FIRST;

	if (strcmp(pszHFType,"header-last") == 0)
		return FL_HDRFTR_HEADER_LAST;

	if (strcmp(pszHFType,"footer") == 0)
		return FL_HDRFTR_FOOTER;

	if (strcmp(pszHFType,"footer-even") == 0)
		return FL_HDRFTR_FOOTER_EVEN;

	if (strcmp(pszHFType,"footer-first") == 0)
		return FL_HDRFTR_FOOTER_FIRST;

	if (strcmp(pszHFType,"footer-last") == 0)
	    return FL_HDRFTR_FOOTER_LAST;

	return FL_HDRFTR_NONE;
}
