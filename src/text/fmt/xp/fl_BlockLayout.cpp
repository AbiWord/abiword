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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "fl_BlockLayout.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fl_AutoNum.h"
#include "fb_LineBreaker.h"
#include "fb_Alignment.h"
#include "fp_Column.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_TextRun.h"
#include "fp_FieldListLabelRun.h"
#include "pd_Document.h"
#include "fd_Field.h"
#include "pd_Style.h"
#include "pp_Property.h"
#include "pp_AttrProp.h"
#include "pt_Types.h"
#include "gr_Graphics.h"
#include "spell_manager.h"
#include "px_CR_FmtMark.h"
#include "px_CR_FmtMarkChange.h"
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"
#include "fv_View.h"
#include "xap_App.h"
#include "xap_Clipboard.h"
#include "ut_png.h"
#include "ut_sleep.h"
#include "fg_Graphic.h"
#include "ap_Prefs.h"
#include "ap_Prefs_SchemeIds.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"

#include "xap_EncodingManager.h"

#if 1
// todo: work around to remove the INPUTWORDLEN restriction for pspell
#include "ispell_def.h"
#endif

//////////////////////////////////////////////////////////////////////
// Two Useful List arrays
/////////////////////////////////////////////////////////////////////
//
// SEVIOR: FIXME these definitions are included here as well as in
// src/wp/impexp/xp/ie_imp_RTF.cpp
//
// We need to find a way to include these definitions in
// src/text/fmt/xp/fl_AutoLists.h without raising a whole
// see of "unused variable" warnings.
//
// C/C++ gods please advise

static const XML_Char * xml_Lists[] = { XML_NUMBERED_LIST,
										XML_LOWERCASE_LIST,
										XML_UPPERCASE_LIST,
										XML_LOWERROMAN_LIST,
										XML_UPPERROMAN_LIST,
										XML_BULLETED_LIST,
										XML_DASHED_LIST,
										XML_SQUARE_LIST,
										XML_TRIANGLE_LIST,
										XML_DIAMOND_LIST,
										XML_STAR_LIST,
										XML_IMPLIES_LIST,
										XML_TICK_LIST,
										XML_BOX_LIST,
										XML_HAND_LIST,
										XML_HEART_LIST };

static const char     * fmt_Lists[] = { fmt_NUMBERED_LIST,
										fmt_LOWERCASE_LIST,
										fmt_UPPERCASE_LIST,
										fmt_UPPERROMAN_LIST,
										fmt_LOWERROMAN_LIST,
										fmt_BULLETED_LIST,
										fmt_DASHED_LIST };


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// End List definitions
//////////////////////////////////////////////////////////////////////////


bool 
fl_BlockLayout::_spellCheckWord (const UT_UCSChar * word, UT_uint32 len, UT_uint32 blockPos)
{
	SpellChecker * checker = NULL;
	
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	
	getSpanAttrProp(blockPos, false, &pSpanAP);
	getAttrProp(&pBlockAP);
	
	const char * szLang = (const char * ) PP_evalProperty("lang",pSpanAP,pBlockAP,NULL,m_pDoc,true);
	
	if (szLang)
	{
		//UT_DEBUGMSG(("fl_BlockLaout::_spellCheckWord: lang = %s\n", szLang));
		// we get smart and request the proper dictionary
		checker = SpellManager::instance().requestDictionary(szLang);
	}
	else
	{
		// we just (dumbly) default to the last dictionary
		checker = SpellManager::instance().lastDictionary();
	}

	if (!checker)
	{
		// no checker found, don't mark as wrong
		return true;
	}

	if (checker->checkWord (word, len) == SpellChecker::LOOKUP_SUCCEEDED)
		return true;
	return false;
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fl_BlockLayout::fl_BlockLayout(PL_StruxDocHandle sdh,
							   fb_LineBreaker* pBreaker,
							   fl_BlockLayout* pPrev,
							   fl_SectionLayout* pSectionLayout,
							   PT_AttrPropIndex indexAP, bool bIsHdrFtr)
	: fl_Layout(PTX_Block, sdh)
{
	m_pAlignment = NULL;

	m_pSectionLayout = pSectionLayout;
	m_pBreaker = pBreaker;
	m_pFirstRun = NULL;
	m_pFirstLine = NULL;
	m_pLastLine = NULL;
	m_pAutoNum = NULL;

	m_bNeedsReformat = true;
	m_bNeedsRedraw = false;
	m_bFixCharWidths = false;
	m_bKeepTogether = false;
	m_bKeepWithNext = false;
	m_bListItem = false;
	m_bStartList = false;
	m_bStopList = false;
	m_bListLabelCreated = false;
	m_bCursorErased = false;
	m_uBackgroundCheckReasons = 0;
	m_bIsHdrFtr = bIsHdrFtr;
	m_pLayout = m_pSectionLayout->getDocLayout();
	m_pDoc = m_pLayout->getDocument();

	setAttrPropIndex(indexAP);

	m_pPrev = pPrev;
	if (m_pPrev)
	{
		m_pNext = pPrev->m_pNext;
		m_pPrev->m_pNext = this;
	}
	else
	{
		m_pNext = pSectionLayout->getFirstBlock();
	}

	if (m_pNext)
	{
		m_pNext->m_pPrev = this;
	}

	_lookupProperties();

	_insertEndOfParagraphRun();
}


fl_BlockLayout::fl_BlockLayout(PL_StruxDocHandle sdh,
							   fb_LineBreaker* pBreaker,
							   fl_BlockLayout* pPrev,
							   fl_SectionLayout* pSectionLayout,
			       PT_AttrPropIndex indexAP)
	: fl_Layout(PTX_Block, sdh)
{
	m_pAlignment = NULL;

	m_pSectionLayout = pSectionLayout;
	m_pBreaker = pBreaker;
	m_pFirstRun = NULL;
	m_pFirstLine = NULL;
	m_pLastLine = NULL;
	m_pAutoNum = NULL;

	m_bNeedsReformat = true;
	m_bNeedsRedraw = false;
	m_bFixCharWidths = false;
	m_bKeepTogether = false;
	m_bKeepWithNext = false;
	m_bListItem = false;
	m_bStartList = false;
	m_bStopList = false;
	m_bListLabelCreated = false;
	m_bCursorErased = false;
	m_uBackgroundCheckReasons = 0;
	m_bIsHdrFtr = false;
	m_pLayout = m_pSectionLayout->getDocLayout();
	m_pDoc = m_pLayout->getDocument();

	setAttrPropIndex(indexAP);

	m_pPrev = pPrev;
	if (m_pPrev)
	{
		m_pNext = pPrev->m_pNext;
		m_pPrev->m_pNext = this;
	}
	else
	{
		m_pNext = pSectionLayout->getFirstBlock();
	}

	if (m_pNext)
	{
		m_pNext->m_pPrev = this;
	}

	_lookupProperties();

	_insertEndOfParagraphRun();
}

fl_TabStop::fl_TabStop()
{
	iPosition = 0;
	iPositionLayoutUnits = 0;
	iType = FL_TAB_NONE;
	iLeader = FL_LEADER_NONE;
}

static int compare_tabs(const void* p1, const void* p2)
{
	fl_TabStop** ppTab1 = (fl_TabStop**) p1;
	fl_TabStop** ppTab2 = (fl_TabStop**) p2;

	if ((*ppTab1)->getPosition() < (*ppTab2)->getPosition())
	{
		return -1;
	}
	
	if ((*ppTab1)->getPosition() > (*ppTab2)->getPosition())
	{
		return 1;
	}
	
	return 0;
}

void buildTabStops(GR_Graphics * pG, const char* pszTabStops, UT_Vector &m_vecTabs)
{
	// no matter what, clear prior tabstops
	UT_uint32 iCount = m_vecTabs.getItemCount();
	UT_uint32 i;

	for (i=0; i<iCount; i++)
	{
		fl_TabStop* pTab = (fl_TabStop*) m_vecTabs.getNthItem(i);

		delete pTab;
	}

	m_vecTabs.clear();
	if (pszTabStops && pszTabStops[0])
	{
		eTabType	iType = FL_TAB_NONE;
		eTabLeader	iLeader = FL_LEADER_NONE;
		UT_sint32	iPosition = 0;
		
		const char* pStart = pszTabStops;
		while (*pStart)
		{
			const char* pEnd = pStart;
			while (*pEnd && (*pEnd != ','))
			{
				pEnd++;
			}

			const char* p1 = pStart;
			while ((p1 < pEnd) && (*p1 != '/'))
			{
				p1++;
			}

			if (
				(p1 == pEnd)
				|| ((p1+1) == pEnd)
				)
			{
				iType = FL_TAB_LEFT;
			}
			else
			{
				switch (p1[1])
				{
				case 'R':
					iType = FL_TAB_RIGHT;
					break;
				case 'C':
					iType = FL_TAB_CENTER;
					break;
				case 'D':
					iType = FL_TAB_DECIMAL;
					break;
				case 'B':
					iType = FL_TAB_BAR;
					break;
				case 'L':
					iType = FL_TAB_LEFT;
					break;
				default:
					iType = FL_TAB_LEFT;
					UT_DEBUGMSG(("tabstop: unknown tab stop type [%c]\n", p1[1]));
					break;
				}

				// tab leaders
				if ( p1 +2 != pEnd && p1[2] >= '0' && p1[2] <= (((UT_sint32)__FL_LEADER_MAX)+'0') )
					iLeader = (eTabLeader)(p1[2]-'0');
			}

			char pszPosition[32];
			UT_uint32 iPosLen = p1 - pStart;
		
			UT_ASSERT(iPosLen < sizeof pszPosition);

			memcpy(pszPosition, pStart, iPosLen);
			pszPosition[iPosLen] = 0;

			iPosition = pG->convertDimension(pszPosition);

			UT_ASSERT(iType > 0);
			/*
			  The following assert is probably bogus, since tabs are
			  column-relative, rather than block-relative.
			*/
//			UT_ASSERT(iPosition >= 0);
			
			fl_TabStop* pTabStop = new fl_TabStop();
			pTabStop->setPosition(iPosition);
			pTabStop->setPositionLayoutUnits(UT_convertToLayoutUnits(pszPosition));
			pTabStop->setType(iType);
			pTabStop->setLeader(iLeader);
			pTabStop->setOffset(pStart - pszTabStops);

			m_vecTabs.addItem(pTabStop);

			pStart = pEnd;
			if (*pStart)
			{
				pStart++;	// skip past delimiter

				while (*pStart == UCS_SPACE)
				{
					pStart++;
				}
			}
		}

		m_vecTabs.qsort(compare_tabs);
	}
}
void fl_BlockLayout::_lookupProperties(void)
{
	{
		// The EOP Run is an integral part of the block so also make
		// sure it does lookup.

		fp_Line* pLine = getLastLine();
		if (pLine)
		{
			fp_Run* pRun = pLine->getLastRun();
			pRun->lookupProperties();
		}
	}

	{
		const PP_AttrProp * pBlockAP = NULL;
		getAttrProp(&pBlockAP);

		if (!pBlockAP || !pBlockAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, m_szStyle))
			m_szStyle = NULL;
	}

	{
#ifdef BIDI_ENABLED
		m_bDomDirection = !UT_stricmp(getProperty("dom-dir", true), "rtl");
		//UT_DEBUGMSG(("Block: _lookupProperties, m_bDomDirection=%d (%s)\n", m_bDomDirection, getProperty("dom-dir", true)));
#endif
		const PP_PropertyTypeInt *pOrphans = (const PP_PropertyTypeInt *)getPropertyType("orphans", Property_type_int);
		UT_ASSERT(pOrphans);
		m_iOrphansProperty = pOrphans->getValue();

		const PP_PropertyTypeInt *pWidows = (const PP_PropertyTypeInt *)getPropertyType("widows", Property_type_int);
		UT_ASSERT(pWidows);
		m_iWidowsProperty = pWidows->getValue();

		if (m_iOrphansProperty < 1)
		{
			m_iOrphansProperty = 1;
		}
		if (m_iWidowsProperty < 1)
		{
			m_iWidowsProperty = 1;
		}
	}

	{
		const char* pszKeepTogether = getProperty("keep-together");
		if (pszKeepTogether
			&& (0 == UT_strcmp("yes", pszKeepTogether))
			)
		{
			m_bKeepTogether = true;
		}
	
		const char* pszKeepWithNext = getProperty("keep-with-next");
		if (pszKeepWithNext
			&& (0 == UT_strcmp("yes", pszKeepWithNext))
			)
		{
			m_bKeepWithNext = true;
		}
	}
	
	GR_Graphics* pG = m_pLayout->getGraphics();

	struct MarginAndIndent_t
	{
		const char* szProp;
		UT_sint32*	pVar;
		UT_sint32*	pVarLU;
	}
	const rgProps[] =
	{
		{ "margin-top",		&m_iTopMargin,		&m_iTopMarginLayoutUnits	},
		{ "margin-bottom",	&m_iBottomMargin,	&m_iBottomMarginLayoutUnits },
		{ "margin-left",	&m_iLeftMargin,		&m_iLeftMarginLayoutUnits	},
		{ "margin-right",	&m_iRightMargin,	&m_iRightMarginLayoutUnits	},
		{ "text-indent",	&m_iTextIndent,		&m_iTextIndentLayoutUnits	}
	};
	for (UT_uint32 iRg = 0; iRg < NrElements(rgProps); ++iRg)
	{
		const MarginAndIndent_t& mai = rgProps[iRg];
		const PP_PropertyTypeSize * pProp = (const PP_PropertyTypeSize *)getPropertyType((XML_Char*)mai.szProp, Property_type_size);
		*mai.pVar	= pG->convertDimension(pProp->getValue(), pProp->getDim());
		*mai.pVarLU	= UT_convertSizeToLayoutUnits(pProp->getValue(), pProp->getDim());
	}

	{
		const char* pszAlign = getProperty("text-align");

		DELETEP(m_pAlignment);
		
		//UT_DEBUGMSG(("block: _lookupProperties, text-align=%s\n", pszAlign));

		if (0 == UT_strcmp(pszAlign, "left"))
		{
			m_pAlignment = new fb_Alignment_left;
		}
		else if (0 == UT_strcmp(pszAlign, "center"))
		{
			m_pAlignment = new fb_Alignment_center;
		}
		else if (0 == UT_strcmp(pszAlign, "right"))
		{
			m_pAlignment = new fb_Alignment_right;
		}
		else if (0 == UT_strcmp(pszAlign, "justify"))
		{
			m_pAlignment = new fb_Alignment_justify;
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}

	// parse any new tabstops
	const char* pszTabStops = getProperty("tabstops");
	buildTabStops(pG, pszTabStops, m_vecTabs);


#if 0
	UT_DEBUGMSG(("XXXX: [default-tab-interval:%s][yields %d][resolution %d][zoom %d]\n",
				 getProperty("default-tab-interval"),
				 pG->convertDimension(getProperty("default-tab-interval")),
				 pG->getResolution(),
				 pG->getZoomPercentage()));
#endif
	
	const PP_PropertyTypeSize * pProp = (const PP_PropertyTypeSize * )getPropertyType("default-tab-interval", Property_type_size);
	m_iDefaultTabInterval = pG->convertDimension(pProp->getValue(), pProp->getDim());
	m_iDefaultTabIntervalLayoutUnits = UT_convertSizeToLayoutUnits(pProp->getValue(), pProp->getDim());


	const char * pszSpacing = getProperty("line-height");

	// NOTE : Parsing spacing strings:
	// NOTE : - if spacing string ends with "+", it's marked as an "At Least" measurement
	// NOTE : - if spacing has a unit in it, it's an "Exact" measurement
	// NOTE : - if spacing is a unitless number, it's just a "Multiple"
	UT_uint32 nLen = strlen(pszSpacing);
	if (nLen > 1)
	{
		char * pPlusFound = strrchr(pszSpacing, '+');
		if (pPlusFound && *(pPlusFound + 1) == 0)
		{
			m_eSpacingPolicy = spacing_ATLEAST;

			// need to strip the plus first
			int posPlus = pPlusFound - pszSpacing;
			UT_ASSERT(posPlus>=0);
			UT_ASSERT(posPlus<100);

			char pTmp[100];
			strcpy(pTmp, pszSpacing);
			pTmp[posPlus] = 0;

			m_dLineSpacing = pG->convertDimension(pTmp);
			m_dLineSpacingLayoutUnits = UT_convertToLayoutUnits(pTmp);
		}
		else if (UT_hasDimensionComponent(pszSpacing))
		{
			m_eSpacingPolicy = spacing_EXACT;
			m_dLineSpacing = pG->convertDimension(pszSpacing);
			m_dLineSpacingLayoutUnits = UT_convertToLayoutUnits(pszSpacing);
		}
		else
		{
			m_eSpacingPolicy = spacing_MULTIPLE;
			m_dLineSpacing = m_dLineSpacingLayoutUnits = UT_convertDimensionless(pszSpacing);
		}
	}

	//
	// No numbering in headers/footers
	//
	if(getSectionLayout() && (getSectionLayout()->getType()== FL_SECTION_HDRFTR))
	{
		return;
	}

	const PP_AttrProp * pBlockAP = NULL;
	getAttrProp(&pBlockAP);
	const XML_Char * szLid=NULL;
	const XML_Char * szPid=NULL;
	const XML_Char * szLevel=NULL;
	UT_uint32 id,parent_id,level;

	if (!pBlockAP || !pBlockAP->getAttribute(PT_LISTID_ATTRIBUTE_NAME, szLid))
		szLid = NULL;
	if (szLid)
		id = atoi(szLid);
	else
		id = 0;


	if (!pBlockAP || !pBlockAP->getAttribute(PT_PARENTID_ATTRIBUTE_NAME, szPid))
		szPid = NULL;
	if (szPid)
		parent_id = atoi(szPid);
	else
		parent_id = 0;

	if (!pBlockAP || !pBlockAP->getAttribute(PT_LEVEL_ATTRIBUTE_NAME, szLevel))
		szLevel = NULL;
	if (szLevel)
		level = atoi(szLevel);
	else
		level = 0;

	fl_BlockLayout * prevBlockInList = NULL;
	fl_BlockLayout * nextBlockInList = NULL;
	fl_AutoNum * pAutoNum;
	if(id != 0)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: id = %d parent_id= %d \n",id,parent_id));
		//UT_DEBUGMSG(("SEVIOR: margin-left = %s \n", getProperty("margin-left")));
	}
	if ((m_pAutoNum) && (id) && (m_pAutoNum->getID() != id))
	{
		// We have stopped or started a multi-level list
		m_pAutoNum->removeItem(getStruxDocHandle());
		m_pAutoNum = NULL;
		UT_DEBUGMSG(("Started/Stopped Multi-Level\n"));
	}

	if (id == 0 && (m_pAutoNum))
	{
		// We have stopped a final list item.
		m_bStopList = true;
		m_pAutoNum->markAsDirty();
		if(m_pAutoNum->isItem(getStruxDocHandle()) == true)
			m_pAutoNum->removeItem(getStruxDocHandle());
		m_bListItem = false;
		_deleteListLabel();
		
		if (m_pAutoNum->isEmpty())
		{
			fl_AutoNum * pAuto = m_pAutoNum;
			DELETEP(m_pAutoNum);
			m_pDoc->removeList(pAuto,getStruxDocHandle());
		}
		else
		{
			m_pAutoNum->update(0);
		}
		m_bStopList = false;
		m_pAutoNum = NULL;
		UT_DEBUGMSG(("Stopped List\n"));
	}

	if (id != 0 && !m_pAutoNum)
	{
		UT_DEBUGMSG(("Adding to List, id= %d parent_id = \n",id,parent_id));
		
		pAutoNum = m_pDoc->getListByID(id);
		//
		// Create new list if none exists
		//
		if(pAutoNum == NULL)
		{
			const XML_Char * pszStart = getProperty("start-value",true);
			const XML_Char * lDelim =  getProperty("list-delim",true);
			const XML_Char * lDecimal =  getProperty("list-decimal",true);
			UT_uint32 start = atoi(pszStart);
			const XML_Char * style = NULL;
			pBlockAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME,style);
			List_Type lType = getListTypeFromStyle( style);
			pAutoNum = new fl_AutoNum(id, parent_id, lType, start, lDelim, lDecimal, m_pDoc);
			UT_DEBUGMSG(("SEVIOR: Created new list \n"));
			m_pDoc->addList(pAutoNum);
		}
		UT_ASSERT(pAutoNum);
		m_pAutoNum = pAutoNum;
		m_bListItem = true;
		
		prevBlockInList = getPreviousList(id);
		nextBlockInList = getNextList(id);		
		
		if (prevBlockInList)
			m_pAutoNum->insertItem(getStruxDocHandle(), prevBlockInList->getStruxDocHandle());
		else if (nextBlockInList)
			m_pAutoNum->prependItem(getStruxDocHandle(),nextBlockInList->getStruxDocHandle());
		else
		{
			if (pAutoNum->getParent())
				prevBlockInList = getParentItem();
			else
				prevBlockInList = NULL;
			PL_StruxDocHandle pItem = getStruxDocHandle();
			PL_StruxDocHandle ppItem;
			if(prevBlockInList != NULL )
			{
				ppItem = prevBlockInList->getStruxDocHandle();
			}
			else
			{
				ppItem = (PL_StruxDocHandle) NULL;
			}
			m_pAutoNum->insertFirstItem(pItem,ppItem,0);
			m_bStartList = true;
		}

		UT_DEBUGMSG(("Added Item to List\n"));
	}

	// Add this in for loading - see if better way to fix.
	// if (m_bListItem && !m_bListLabelCreated && m_pFirstRun)
	//	_createListLabel();
}

fl_BlockLayout::~fl_BlockLayout()
{
	_purgeSquiggles();
	purgeLayout();

	UT_VECTOR_PURGEALL(fl_TabStop *, m_vecTabs);

	DELETEP(m_pAlignment);
	//  if (m_pAutoNum)
//  	{
//  		m_pAutoNum->removeItem(getStruxDocHandle());
//  		if (m_pAutoNum->isEmpty())
//  			DELETEP(m_pAutoNum);
//  	}

	UT_ASSERT(m_pLayout != NULL);

	m_pLayout->notifyBlockIsBeingDeleted(this);
}

/*!
 * This method returns the DocSectionLayout that this block is associated with
 */
fl_DocSectionLayout * fl_BlockLayout::getDocSectionLayout(void)
{
	fl_DocSectionLayout * pDSL = NULL;
	if(getSectionLayout()->getType() == FL_SECTION_DOC)
	{
		pDSL = static_cast<fl_DocSectionLayout *>( m_pSectionLayout);
		return pDSL;
	}
	else if(getSectionLayout()->getType() == FL_SECTION_HDRFTR)
	{
		pDSL = static_cast<fl_HdrFtrSectionLayout *>( getSectionLayout())->getDocSectionLayout();
		return pDSL;
	}
	pDSL = static_cast<fl_HdrFtrShadow *>( getSectionLayout())->getHdrFtrSectionLayout()->getDocSectionLayout();
	return pDSL;
}

void fl_BlockLayout::clearScreen(GR_Graphics* /* pG */)
{
	fp_Line* pLine = m_pFirstLine;
	while (pLine)
	{
		UT_ASSERT(!pLine->isEmpty());
//		fp_Run * pRun = pLine->getFirstRun();
//  		while(pRun)
//  		{
//  			pRun->clearScreen();
//  			pRun = pRun->getNext();
//  		}
		pLine->clearScreen();
		pLine = pLine->getNext();
	}
}

void fl_BlockLayout::_mergeRuns(fp_Run* pFirstRunToMerge, fp_Run* pLastRunToMerge)
{
	UT_ASSERT(pFirstRunToMerge != pLastRunToMerge);
	UT_ASSERT(pFirstRunToMerge->getType() == FPRUN_TEXT);
	UT_ASSERT(pLastRunToMerge->getType() == FPRUN_TEXT);

	_assertRunListIntegrity();

	fp_TextRun* pFirst = (fp_TextRun*) pFirstRunToMerge;

	bool bDone = false;
	while (!bDone)
	{
		if (pFirst->getNext() == pLastRunToMerge)
		{
			bDone = true;
		}

		pFirst->mergeWithNext();
	}

	_assertRunListIntegrity();
}

void fl_BlockLayout::coalesceRuns(void)
{
	_assertRunListIntegrity();

#if 1
	fp_Line* pLine = m_pFirstLine;
	while (pLine)
	{
		pLine->coalesceRuns();
		
		pLine = pLine->getNext();
	}
#else	
	fp_Run* pFirstRunInChain = NULL;
	UT_uint32 iNumRunsInChain = 0;
	
	fp_Run* pCurrentRun = m_pFirstRun;
	fp_Run* pLastRun = NULL;

	while (pCurrentRun)
	{
		if (pCurrentRun->getType() == FPRUN_TEXT)
		{
			if (pFirstRunInChain)
			{
				if (
					(pCurrentRun->getLine() == pFirstRunInChain->getLine())
					&& (pCurrentRun->getAP() == pFirstRunInChain->getAP())
					&& ((!pLastRun)
						|| (
							(pCurrentRun->getBlockOffset() == (pLastRun->getBlockOffset() + pLastRun->getLength()))
							)
						)
					)
				{
					iNumRunsInChain++;
				}
				else
				{
					if (iNumRunsInChain > 1)
					{
						_mergeRuns(pFirstRunInChain, pLastRun);
					}

					pFirstRunInChain = pCurrentRun;
					iNumRunsInChain = 1;
				}
			}
			else
			{
				pFirstRunInChain = pCurrentRun;
				iNumRunsInChain = 1;
			}
		}
		else
		{
			if (iNumRunsInChain > 1)
			{
				_mergeRuns(pFirstRunInChain, pLastRun);
			}

			iNumRunsInChain = 0;
			pFirstRunInChain = NULL;
		}
		
		pLastRun = pCurrentRun;
		pCurrentRun = pCurrentRun->getNext();
	}

	if (iNumRunsInChain > 1)
	{
		_mergeRuns(pFirstRunInChain, pLastRun);
	}
#endif	

	_assertRunListIntegrity();
}

void fl_BlockLayout::collapse(void)
{
	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		pRun->setLine(NULL);
		
		pRun = pRun->getNext();
	}
	
	fp_Line* pLine = m_pFirstLine;
	while (pLine)
	{
		_removeLine(pLine);
		pLine = m_pFirstLine;
	}

	UT_ASSERT(m_pFirstLine == NULL);
	UT_ASSERT(m_pLastLine == NULL);
}

void fl_BlockLayout::purgeLayout(void)
{
	fp_Line* pLine = m_pFirstLine;
	//         if(getSectionLayout() && (getSectionLayout()->getType()== FL_SECTION_HDRFTR))
//  	{
//  	  // Sevior.
//  	  // TODO. Investigate whether this causes a memory leak.
//            // This delete appears to clash with the line delete from the shadows
//            // Apparently the fact that both the first page shadow and this overall
//  	  // hdrftrSection are attached to the same container causes conflicts.
//  	  // Maybe we should implement a virtual header/footer container for the
//  	  // overall hdrftrSectionLayout. Anyway right now doing this prsvents
//  	  // a crash on closing windows.
//  	  //
//  	        while (m_pFirstRun)
//  	        {
//  		       fp_Run* pNext = m_pFirstRun->getNext();
//  		       delete m_pFirstRun;
//  		       m_pFirstRun = pNext;
//  		}
//  		return;
//	}
	while (pLine)
	{
		_removeLine(pLine);
		pLine = m_pFirstLine;
	}

	UT_ASSERT(m_pFirstLine == NULL);
	UT_ASSERT(m_pLastLine == NULL);
	
	while (m_pFirstRun)
	{
		fp_Run* pNext = m_pFirstRun->getNext();
		delete m_pFirstRun;
		m_pFirstRun = pNext;
	}
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

	pLine->setBlock(NULL);
	pLine->remove();

	delete pLine;
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

/*!
  Truncate layout from the specified Run
  \param pTruncRun First Run to be truncated
  \return True

  This will remove all Runs starting from pTruncRun to the last Run on
  the block from their lines (and delete them from the display).

  \note The Run list may be inconsistent when this function is
        called, so no assertion.
  */
bool fl_BlockLayout::_truncateLayout(fp_Run* pTruncRun)
{
	// Special case, nothing to do
	if (!pTruncRun)
	{
		return true;
	}

	if (m_pFirstRun == pTruncRun)
	{
		m_pFirstRun = NULL;
	}
	fp_Run * pRun = NULL;
	// Remove runs from screen
	fp_Line * pLine = pTruncRun->getLine();
	if(pLine != NULL)
	{
		pLine->clearScreenFromRunToEnd(pTruncRun);
		pLine = pLine->getNext();
		while(pLine)
		{
			pLine->clearScreen();
			pLine= pLine->getNext();
		}
	}
	else
	{
		pRun = pTruncRun;
		while (pRun)
		{
			pRun->clearScreen();
			pRun = pRun->getNext();
		}
	}

	// Remove runs from lines
	pRun = pTruncRun;
	while (pRun)
	{
		fp_Line* pLine = pRun->getLine();

		if (pLine)
			pLine->removeRun(pRun, true);

		pRun = pRun->getNext();
	}

	_removeAllEmptyLines();

	return true;
}

/*!
  Move all Runs in the block onto a new line.
  This is only called during block creation when there are no existing
  lines in the block.  
*/
void fl_BlockLayout::_stuffAllRunsOnALine(void)
{
	UT_ASSERT(m_pFirstLine == NULL);
	fp_Line* pLine = getNewLine();
	UT_ASSERT(pLine);
	
	fp_Run* pTempRun = m_pFirstRun;
	while (pTempRun)
	{
		pLine->addRun(pTempRun);
		pTempRun = pTempRun->getNext();
	}
}

/*!
  Add end-of-paragraph Run to block

  This function adds the EOP Run to the block.  The presence of the
  EOP is an invariant (except for when merging / splitting blocks) and
  ensures that the cursor can always be placed on the last line of a
  block.  If there are multiple lines, the first N-1 lines will have a
  forced break of some kind which can also hold the cursor.
*/
void
fl_BlockLayout::_insertEndOfParagraphRun(void)
{
	UT_ASSERT(!m_pFirstRun);

#if 0 // FIXME: Surely should also insert it again!?
	FV_View* ppView = m_pLayout->getView();
	if (ppView)
	{
		ppView->eraseInsertionPoint();
	}
#endif

	GR_Graphics* pG = m_pLayout->getGraphics();
	fp_EndOfParagraphRun* pEOPRun = new fp_EndOfParagraphRun(this, pG, 0, 0);
	m_pFirstRun = pEOPRun;
	m_pFirstRun->fetchCharWidths(&m_gbCharWidths);

	m_bNeedsRedraw = true;

	// FIXME:jskov Why don't the header/footer need the line?
	//if (getSectionLayout() 
	//	&& (getSectionLayout()->getType()== FL_SECTION_HDRFTR))
	//{
	//	return;
	//}

	if (!m_pFirstLine)
	{
		getNewLine();
	}

	UT_ASSERT(m_pFirstLine->countRuns() == 0);
	
	m_pFirstLine->addRun(m_pFirstRun);

	m_pFirstLine->layout();

	// Run list should be valid now.
	_assertRunListIntegrity();
}

/*!
  Remove end-of-paragraph Run from block

  This function does the opposite of the _insertEndOfParagraphRun
  function.

  \note It should <b>only</b> be called by functions that do really
        low-level handling of blocks and only on newly created blocks.
*/
void
fl_BlockLayout::_purgeEndOfParagraphRun(void)
{
	UT_ASSERT(m_pFirstRun && 
			  FPRUN_ENDOFPARAGRAPH == m_pFirstRun->getType());
	UT_ASSERT(m_pFirstLine && m_pFirstLine->countRuns() == 1);

	// Run list should be valid when called (but not at exit!)
	_assertRunListIntegrity();

	m_pFirstLine->removeRun(m_pFirstRun);
	delete m_pFirstRun;
	m_pFirstRun = NULL;

	m_pFirstLine->remove();
	delete m_pFirstLine;
	m_pFirstLine = NULL;
	m_pLastLine = NULL;
}

/*!
  Split the line the Run resides on
  \param pRun The Run to split the line after

  There is never added any Runs as it happened in the past to ensure
  that both lines can hold the point. This is because the caller
  always does that now.
*/
void
fl_BlockLayout::_breakLineAfterRun(fp_Run* pRun)
{
	_assertRunListIntegrity();

	// When loading a document, there may not have been created
	// lines yet. Get a first one created and hope for the best...
	// Sevior: Ah here is one source of the multi-level list bug we
	// need a last line from the previous block before we call this.
	if (getPrev() != NULL && getPrev()->getLastLine() == NULL)
	{
		UT_DEBUGMSG(("In _breakLineAfterRun no LastLine \n"));
		UT_DEBUGMSG(("getPrev = %d this = %d \n", getPrev(), this));
		//UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	// Add a line for the Run if necessary
	if (!m_pFirstLine)
		_stuffAllRunsOnALine();

	// Create the new line
	fp_Line* pNewLine = new fp_Line();
	UT_ASSERT(pNewLine);
	// Insert it after the current line
	fp_Line* pLine = pRun->getLine();
	pNewLine->setPrev(pLine);
	pNewLine->setNext(pLine->getNext());
	pLine->setNext(pNewLine);
	// Update LastLine if necessary
	if (m_pLastLine == pLine)
	    m_pLastLine = pNewLine;
	// Set the block
	pNewLine->setBlock(this);
	// Add the line to the container
	pLine->getContainer()->insertLineAfter(pNewLine, pLine);

	// Now add Runs following pRun on the same line to the new
	// line.
	fp_Run* pCurrentRun = pRun->getNext();
	while (pCurrentRun && pCurrentRun->getLine() == pLine)
	{
		pLine->removeRun(pCurrentRun, true);
		pNewLine->addRun(pCurrentRun);
		pCurrentRun = pCurrentRun->getNext();
	}
	
	// Update the layout information in the lines.
	pLine->layout();
	pNewLine->layout();

	_assertRunListIntegrity();
}

/*!
 * This method updates the background Colour stored in all the runs. We call
 * this after a Section Level change where the background colour might have
 * changed.
 */
void fl_BlockLayout::updateBackgroundColor(void)
{
	fp_Run * pRun  = m_pFirstRun;
	while(pRun!= NULL)
	{
		pRun->updateBackgroundColor();
		pRun = pRun->getNext();
	}
}

/*!
  Format paragraph
  \return 0
  Formatting a paragraph means splitting the content into lines which
  will fit in the container.  */
int
fl_BlockLayout::format()
{
	_assertRunListIntegrity();

	// Remember state of cursor
	m_bCursorErased = false;
	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		if (pView->isCursorOn() == true)
		{
			pView->eraseInsertionPoint();
			m_bCursorErased = true;
		}
	}
	_lookupProperties();

	if (m_pFirstRun)
	{
		// Recalculate widths of Runs if necessary.
		if (m_bFixCharWidths)
		{
			fp_Run* pRun = m_pFirstRun;
			while (pRun)
			{
				pRun->recalcWidth();
				pRun = pRun->getNext();
			}
		}

		// Create the first line if necessary.
		if (!m_pFirstLine)
			_stuffAllRunsOnALine();

		recalculateFields();

		// Reformat paragraph
		m_pBreaker->breakParagraph(this);
		_removeAllEmptyLines();
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		// No paragraph content. Just insert the EOP Run.
		_removeAllEmptyLines();
		_insertEndOfParagraphRun();
	}

	if ((m_pAutoNum && isListLabelInBlock() == true)
		&& (m_bListLabelCreated == false))
	{
		m_bListLabelCreated =true;
	}

	// Paragraph has been reformatted.
	m_bNeedsReformat = false;
	// Redraw cursor if necessary
	if (m_bCursorErased == true)
	{
		pView->_fixInsertionPointCoords();
		pView->drawInsertionPoint();
		m_bCursorErased = false;
	}

	_assertRunListIntegrity();
	return 0;	// TODO return code
}

void fl_BlockLayout::redrawUpdate()
{
	m_bCursorErased = false;
	FV_View* pView = m_pLayout->getView();
	UT_ASSERT (pView);

	_lookupProperties();
	fp_Line* pLine = m_pFirstLine;
	while (pLine)
	{
		if (pLine->needsRedraw())
		{
			if (pView->isCursorOn()== true)
			{
				pView->eraseInsertionPoint();
				m_bCursorErased = true;
			}
			pLine->redrawUpdate();
		}

		pLine = pLine->getNext();
	}

	m_bNeedsRedraw = false;

	//	_lookupProperties();

	if(m_bCursorErased == true)
	{
		pView->drawInsertionPoint();
		m_bCursorErased = false;
	}
}

fp_Line* fl_BlockLayout::getNewLine(void)
{
	fp_Line* pLine = new fp_Line();
	// TODO: Handle out-of-memory
	UT_ASSERT(pLine);
	
	pLine->setBlock(this);
	pLine->setNext(NULL);
	
	fp_Container* pContainer = NULL;
	
	if (m_pLastLine)
	{
		fp_Line* pOldLastLine = m_pLastLine;
		
		UT_ASSERT(m_pFirstLine);
		UT_ASSERT(!m_pLastLine->getNext());

		pLine->setPrev(m_pLastLine);
		m_pLastLine->setNext(pLine);
		m_pLastLine = pLine;

		pContainer = pOldLastLine->getContainer();
		pContainer->insertLineAfter(pLine, pOldLastLine);
	}
	else
	{
		UT_ASSERT(!m_pFirstLine);
		m_pFirstLine = pLine;
		m_pLastLine = m_pFirstLine;
		pLine->setPrev(NULL);
		fp_Line* pPrevLine = NULL;
		if(m_pPrev)
		{
			if(m_pPrev->getLastLine() == NULL)
			{
				// Previous block exists but doesn't have a last line.
				// This is a BUG. Try a work around for now. TODO Fix this elsewhere
				UT_DEBUGMSG(("BUG!!! Previous block exists with no last line. This should not happen \n"));
				//  m_pPrev->format();
			}
		}
		if (m_pPrev && m_pPrev->getLastLine())
		{
			pPrevLine = m_pPrev->getLastLine();
			pContainer = pPrevLine->getContainer();
		}
		else if (m_pNext && m_pNext->getFirstLine())
		{
			pContainer = m_pNext->getFirstLine()->getContainer();
		}
		else if (m_pSectionLayout->getFirstContainer())
		{
			// TODO assert something here about what's in that container
			pContainer = m_pSectionLayout->getFirstContainer();
		}
		else
		{
			pContainer = m_pSectionLayout->getNewContainer();
		}

		if (!pPrevLine)
			pContainer->insertLine(pLine);
		else
			pContainer->insertLineAfter(pLine, pPrevLine);
	}

	return pLine;
}

const char*	fl_BlockLayout::getProperty(const XML_Char * pszName, bool bExpandStyles) const
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;
	
	getAttrProp(&pBlockAP);
	
	return PP_evalProperty(pszName,pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
}

const PP_PropertyType *	fl_BlockLayout::getPropertyType(const XML_Char * pszName, tProperty_type Type, bool bExpandStyles) const
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;
	
	getAttrProp(&pBlockAP);
	
	return PP_evalPropertyType(pszName,pSpanAP,pBlockAP,pSectionAP,Type,m_pDoc,bExpandStyles);
}

/*!
 Get block's position in document
 \param bActualBlockPos When true return block's position. When false
                        return position of first run in block
 \return Position of block (or first run in block)
 \fixme Split in two functions if called most often with FALSE
*/
UT_uint32 fl_BlockLayout::getPosition(bool bActualBlockPos) const
{
	PT_DocPosition pos = m_pDoc->getStruxPosition(m_sdh);

	// it's usually more useful to know where the runs start
	if (!bActualBlockPos)
		pos += fl_BLOCK_STRUX_OFFSET;

	return pos;
}

fl_CharWidths * fl_BlockLayout::getCharWidths(void)
{
	return &m_gbCharWidths;
}

void fl_BlockLayout::getLineSpacing(double& dSpacing, double &dSpacingLayout, eSpacingPolicy& eSpacing) const
{
	dSpacing = m_dLineSpacing;
	dSpacingLayout = m_dLineSpacingLayoutUnits;
	eSpacing = m_eSpacingPolicy;
}

bool fl_BlockLayout::getSpanPtr(UT_uint32 offset, const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const
{
	return m_pDoc->getSpanPtr(m_sdh, offset+fl_BLOCK_STRUX_OFFSET, ppSpan, pLength);
}

bool	fl_BlockLayout::getBlockBuf(UT_GrowBuf * pgb) const
{
	return m_pDoc->getBlockBuf(m_sdh, pgb);
}


/*!
  Compute insertion point (carret) coordinates and size
  \param iPos Document position of cursor
  \param bEOL Set if EOL position is wanted
  \retval x X position (LTR)
  \retval y Y position (LTR)
  \retval x2 X position (RTL)
  \retval y2 Y position (RTL)
  \retval height Height of carret
  \retval bDirection Editing direction (true = LTR, false = RTL)
  \return The Run containing (or next to) the carret, or NULL if the block
          has no formatting information.
  \fixme bDirection should be an enum type
*/
fp_Run*
fl_BlockLayout::findPointCoords(PT_DocPosition iPos,
								bool bEOL,
								UT_sint32& x,  UT_sint32& y,
								UT_sint32& x2, UT_sint32& y2,
								UT_sint32& height,
								bool& bDirection)
{
	if (!m_pFirstLine || !m_pFirstRun)
	{
		// when we have no formatting information, can't find anything
		return NULL;
	}

	// find the run which has this offset inside it.
	PT_DocPosition dPos = getPosition();
	//UT_ASSERT(iPos >= dPos);
	const UT_uint32 iRelOffset = iPos - dPos;

	// By default, the Run just before the one we find is the one we
	// want the coords from. This is because insertion is done with
	// the properties of the Run before the point.
	// In some situations, we need to override that and use the coords
	// of the found Run - this flag tells us when to do what.
	bool bCoordOfPrevRun = true;

	// Some of the special cases below fix up pRun/bCoordOfPrevRun
	// so we can use the first exit point. We could just as well
	// fiddle bEOL in those cases, but using this variable makes
	// the intention more clear (IMO).
	bool bUseFirstExit = false;

	// Find first Run past (or at) the requested offset. By scanning
	// in this manner, we do a mimimum of computation to find the
	// approximate location.
	fp_Run* pRun = m_pFirstRun;
	while (pRun && pRun->getBlockOffset() < iRelOffset)
		pRun = pRun->getNext();
	
	// Now scan farther if necessary - the block may contain Runs
	// with zero length. This is only a problem when empty Runs
	// appear for no good reason (i.e., an empty Run on an empty
	// line should be OK).
	while (pRun && pRun->getBlockOffset() + pRun->getLength() < iRelOffset)
		pRun = pRun->getNext();

	// We may have scanned past the last Run in the block. Back up.
	if (!pRun)
	{
		pRun = getLastLine()->getLastRun();
		bCoordOfPrevRun = false;
	}
	
	// Step one back if if previous Run holds the offset (the
	// above loops scan past what we're looking for since it's
	// faster).
	fp_Run* pPrevRun = pRun->getPrev();
	
	if (pPrevRun &&
	    pPrevRun->getBlockOffset() + pPrevRun->getLength() > iRelOffset)
	{
	    pRun = pPrevRun;
	    bCoordOfPrevRun = false;
	}
	
	// Since the requested offset may be a page break (or similar
	// Runs) which cannot contain the point, now work backwards
	// while looking for a Run which can contain the point.
	while (pRun && !pRun->canContainPoint())
	{
		pRun = pRun->getPrev();
		bCoordOfPrevRun = false;
	}

	// Assert if there have been no Runs which can hold the point
	// between the beginning of the block and the requested
	// offset.
	UT_ASSERT(NULL != pRun);

	// This covers a special case (I) when bEOL.  Consider this
	// line (| is the right margin, E end of document):
	//
	// 1:  abcdefgh|
	// 2:  iE
	//
	// When EOL position for display line 1 is requested, it's
	// done with either the offset of h or i (fall through to code
	// below first exit point). EOL position for display line 2 is
	// requested with offset of E (matches third sub-expresion).
	// (This check is rather non-intuitive - step through the code
	// for the different permutations to see why it's correct).
	if (bEOL && pRun->getBlockOffset() < iRelOffset &&
	    pRun->getBlockOffset() + pRun->getLength() >= iRelOffset)
	{
	    bCoordOfPrevRun = false;
	    bUseFirstExit = true;
	}

	// If not bEOL, we're done: either we have actually found the
	// Run containing the offset, or we have found the first
	// suitable Run before the requested offset.
	//
	// This is the exit point most calls will use (being the first
	// exit point, we should be OK performance wise).
	if (bUseFirstExit || !bEOL)
	{
	    if (bCoordOfPrevRun && pRun->letPointPass())
	    {
			// This looks a little weird. What it does is first try to
			// go one Run back only, if allowed.  If that fails, use
			// the original Run.
			pPrevRun = pRun->getPrev();
			if (!pPrevRun
				|| !pPrevRun->letPointPass()
				|| !pPrevRun->canContainPoint())
			{
				pPrevRun = pRun;
			}
			else
			{
				// If the code gets one Run back, keep going back
				// until finding a Run that is valid for point
				// coordinate calculations.
				while (pPrevRun &&
					   !pPrevRun->letPointPass()
					   || !pPrevRun->canContainPoint())
				{
					pPrevRun = pPrevRun->getPrev();
				}

				// If this fails, go with the original Run.
				if (!pPrevRun)
				{
					pPrevRun = pRun;
				}
			}


			// One final check: only allow the point to move to a
			// different line if bEOL.
			if (!bEOL && pRun->getLine() != pPrevRun->getLine())
			{
				pPrevRun = pRun;
			}

			pPrevRun->findPointCoords(iRelOffset, x, y, x2, y2, height, bDirection);
	    }
		else
		{
			pRun->findPointCoords(iRelOffset, x, y, x2, y2, height, bDirection);
	    }

	    return pRun;
	}

	// Runs with layout information (page/column break) are not
	// visible (or rather, cannot contain the point). They look
	// like this (P is a page break, E is end of document):
	//
	// 1:  abcdefghP
	// <page/column alignment>
	// 2:  E
	//
	// When we have to find EOL position for display line 2, the
	// arguments (the offset) is the same as in case (I) above
	// (i.e., the argument is the offset of P). Thus this special
	// check.
	pPrevRun = pRun->getPrev();
	if (!pPrevRun || !pPrevRun->letPointPass())
	{
		pRun->findPointCoords(iRelOffset, x, y, x2, y2, height, bDirection);
	    return pRun;
	}

	// Now search for the Run at the end of the line.  For a
	// soft-broken block (soft-break due to margin constraints),
	// this may be on the previous display line. It may also be
	// pRun (if the offset was past the last Run of this display
	// line). Consider this line (| is the right margin, N the
	// line break or paragraph end):
	//
	// 1:  abcdefgh|
	// 2:  ijklN
	//
	// For normal cursor movement (bEOL=false), IP (*) will move
	// from *h to *i (or vice versa), skipping the h* position.
	// When bEOL=true (user presses End key, or selects EOL with
	// mouse) IP on display line 1 will be at h*, even though the
	// requested offset is actually that of i.
	//
	while (pPrevRun && !pPrevRun->canContainPoint())
	{
	    pPrevRun = pPrevRun->getPrev();
	}

	// If we went past the head of the list, it means that the
	// originally found Run is the only one on this display line.
	if (!pPrevRun)
	{
		pRun->findPointCoords(iRelOffset, x, y, x2, y2, height, bDirection);
	    return pRun;
	}


	// If the Runs are on the same line, assume pRun to be farther
	// right than pPrevRun.
	if (pPrevRun->getLine() == pRun->getLine())
	{
		pRun->findPointCoords(iRelOffset, x, y, x2, y2, height, bDirection);
	    return pRun;
	}

	// Only case left is that of a soft-broken line.

	// Always return position _and_ Run of the previous line. Old
	// implementation returned pRun, but this will cause the
	// cursor to wander if End is pressed multiple times.
	pPrevRun->findPointCoords(iRelOffset, x, y, x2, y2, height, bDirection);
	return pPrevRun;
}

fp_Line* fl_BlockLayout::findPrevLineInDocument(fp_Line* pLine)
{
	if (pLine->getPrev())
	{
		return pLine->getPrev();
	}
	else
	{
		if (m_pPrev)
		{
			return m_pPrev->getLastLine();
		}
		else
		{
			fl_SectionLayout* pSL = m_pSectionLayout->getPrev();

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
	
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

fp_Line* fl_BlockLayout::findNextLineInDocument(fp_Line* pLine)
{
	if (pLine->getNext())
	{
		return pLine->getNext();
	}
	
	if (m_pNext)
	{
		// grab the first line from the next block
		return m_pNext->getFirstLine();
	}
	else
	{
		// there is no next line in this section, try the next
		fl_SectionLayout* pSL = m_pSectionLayout->getNext();

		if (!pSL)
		{
			// at EOD, so just bail
			return NULL;
		}

		fl_BlockLayout* pBlock = pSL->getFirstBlock();
		UT_ASSERT(pBlock);
		return pBlock->getFirstLine();
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

fl_BlockLayout* fl_BlockLayout::getNextBlockInDocument(void) const
{
	if (m_pNext)
	{
		return m_pNext;
	}

	// keep going (check next section)
	fl_SectionLayout* pSL = m_pSectionLayout->getNext();
	fl_BlockLayout* pBL = NULL;

	if (pSL)
	{
		pBL = pSL->getFirstBlock();
		UT_ASSERT(pBL);
	}

	return pBL;
}

fl_BlockLayout* fl_BlockLayout::getPrevBlockInDocument(void) const
{
	if (m_pPrev)
		return m_pPrev;

	// keep going (check prev section)
	fl_SectionLayout* pSL = m_pSectionLayout->getPrev();
	fl_BlockLayout* pBL = NULL;

	if (pSL)
	{
		pBL = pSL->getLastBlock();
		UT_ASSERT(pBL);
	}

	return pBL;
}

/*****************************************************************/
/*****************************************************************/

fl_PartOfBlock::fl_PartOfBlock(void)
{
	iOffset = 0;
	iLength = 0;
	bIsIgnored = false;
}

bool fl_PartOfBlock::doesTouch(UT_uint32 offset, UT_uint32 length) const
{
	UT_uint32 start1, end1, start2, end2;

	start1 = iOffset;
	/*TF CHANGE: Don't artificially extend the length of the block
	  end1 =   iOffset + iLength + 1;
	*/
	end1 =   iOffset + iLength;

	start2 = offset;
	end2 =   offset + length;

	if (end1 == start2)
	{
		return true;
	}
	if (end2 == start1)
	{
		return true;
	}

	/* they overlap */
	if ((start1 <= start2) && (start2 <= end1))
	{
		return true;
	}
	if ((start2 <= start1) && (start1 <= end2))
	{
		return true;
	}

	return false;
}

/*****************************************************************/
/*****************************************************************/

void fl_BlockLayout::_purgeSquiggles(void)
{
	UT_VECTOR_PURGEALL(fl_PartOfBlock *, m_vecSquiggles);

	m_vecSquiggles.clear();
}

fl_PartOfBlock* fl_BlockLayout::getSquiggle(UT_uint32 iOffset) const
{
	fl_PartOfBlock* pPOB = NULL;

	UT_sint32 i = _findSquiggle(iOffset);

	if (i >= 0)
		pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(i);

	return pPOB;
}

UT_sint32 fl_BlockLayout::_findSquiggle(UT_uint32 iOffset) const
{
	// get the squiggle which spans iOffset, if any
	UT_sint32 res = -1;

	UT_uint32 iSquiggles = m_vecSquiggles.getItemCount();
	UT_uint32 j;
	for (j=0; j<iSquiggles; j++)
	{
		fl_PartOfBlock* pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(j);

		if ((pPOB->iOffset <= iOffset) &&
			((pPOB->iOffset + pPOB->iLength) >= iOffset))
		{
			res = j;
			break;
		}
	}

	return res;
}

void fl_BlockLayout::_addSquiggle(UT_uint32 iOffset, UT_uint32 iLen, 
								  bool bIsIgnored /* = false */)
{
	fl_PartOfBlock*	pPOB = new fl_PartOfBlock();
	if (!pPOB)
	{
		// TODO handle outofmem
	}
	
	pPOB->iOffset = iOffset;
	pPOB->iLength = iLen;
	pPOB->bIsIgnored = bIsIgnored;

#if 0 // TODO: turn this code on someday
	FV_View* pView = m_pLayout->getView();
	XAP_App * pApp = XAP_App::getApp();
	XAP_Prefs *pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);

	bool b;
	
	// possibly auto-replace the squiggled word with a suggestion
	if (pPrefs->getPrefsValueBool((XML_Char*)AP_PREF_KEY_SpellAutoReplace, &b))
	{
		if (b && !bIsIgnored)
		{
			// todo: better cursor movement
			pView->cmdContextSuggest(1, this, pPOB);
			pView->moveInsPtTo(FV_DOCPOS_EOW_MOVE);
			return;
		}
	}
#endif

	m_vecSquiggles.addItem(pPOB);
	_updateSquiggle(pPOB);
}

void fl_BlockLayout::_updateSquiggle(fl_PartOfBlock* pPOB)
{
	FV_View* pView = m_pLayout->getView();

	PT_DocPosition pos1 = getPosition() + pPOB->iOffset;
	PT_DocPosition pos2 = pos1 + pPOB->iLength;

	pView->_clearBetweenPositions(pos1, pos2, true);
}

/*
	NOTE: The current squiggle code destructively rechecks the entire
	block for every atomic edit.  This is massively inefficient, and
	rather annoying.  Squiggles should get edited just like everything
	else.

	To go back to the old approach, comment out the #define below.
*/

#define FASTSQUIGGLE

void fl_BlockLayout::_insertSquiggles(UT_uint32 iOffset, UT_uint32 iLength)
{
#ifdef FASTSQUIGGLE
	UT_sint32 chg = iLength;

	UT_ASSERT(m_pLayout);
	if ( m_pLayout->getAutoSpellCheck() == false )
		return;

	// remove squiggle broken by this insert
	UT_sint32 iBroken = _findSquiggle(iOffset);
	if (iBroken >= 0)
	{
		fl_PartOfBlock* pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(iBroken);
		_updateSquiggle(pPOB);
		m_vecSquiggles.deleteNthItem(iBroken);
		delete pPOB;
	}

	// move all trailing squiggles
	_moveSquiggles(iOffset, chg);

	// deal with pending word, if any
	if (m_pLayout->isPendingWordForSpell())
	{
		if (!m_pLayout->touchesPendingWordForSpell(this, iOffset, 0))
		{
			// not affected by insert, so check it
			fl_PartOfBlock* pPending = m_pLayout->getPendingWordForSpell();

			if (pPending->iOffset > iOffset)
				pPending->iOffset = (UT_uint32)((UT_sint32)pPending->iOffset + chg);

			m_pLayout->checkPendingWordForSpell();
		}
	}

	// recheck at boundary
	_recalcPendingWord(iOffset, chg);
#else
	m_pLayout->queueBlockForSpell(this);
#endif
}

void fl_BlockLayout::_breakSquiggles(UT_uint32 iOffset, fl_BlockLayout* pNewBL)
{
	// when inserting block break, squiggles move in opposite direction
	UT_sint32 chg = -(UT_sint32)iOffset;

	// remove squiggle broken by this insert
	UT_sint32 iBroken = _findSquiggle(iOffset);
	if (iBroken >= 0)
	{
		fl_PartOfBlock* pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(iBroken);
		_updateSquiggle(pPOB);
		m_vecSquiggles.deleteNthItem(iBroken);
		delete pPOB;
	}

	// move all trailing squiggles
	_moveSquiggles(0, chg, pNewBL);		// CF: math inside this function

	// deal with previously pending word, if any
	if (m_pLayout->isPendingWordForSpell())
	{
		if (!m_pLayout->touchesPendingWordForSpell(this, iOffset, 0))
		{
			// not affected by insert, so check it
			fl_PartOfBlock* pPending = m_pLayout->getPendingWordForSpell();

			if (pPending->iOffset > iOffset)
				pPending->iOffset = (UT_uint32)((UT_sint32)pPending->iOffset + chg);

			m_pLayout->checkPendingWordForSpell();
		}
	}

	// TODO: check last word remaining in this block
	// TODO: pending word is at beginning of next block

	// recheck at boundary
//	_recalcPendingWord(iOffset, 0);
}

void fl_BlockLayout::_deleteSquiggles(UT_uint32 iOffset, UT_uint32 iLength)
{
#ifdef FASTSQUIGGLE
	UT_sint32 chg = -(UT_sint32)iLength;

	// remove all deleted squiggles
	UT_uint32 iSquiggles = m_vecSquiggles.getItemCount();
	UT_uint32 j;
	for (j=iSquiggles; j>0; j--)
	{
		fl_PartOfBlock* pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(j-1);

		if (pPOB->doesTouch(iOffset, iLength))
		{
			_updateSquiggle(pPOB);
			m_vecSquiggles.deleteNthItem(j-1);
			delete pPOB;
		}
	}

	// move all trailing squiggles
	_moveSquiggles(iOffset, chg);

	// deal with pending word, if any
	if (m_pLayout->isPendingWordForSpell())
	{
		if (!m_pLayout->touchesPendingWordForSpell(this, iOffset, chg))
		{
			// not affected by delete, so check it
			fl_PartOfBlock* pPending = m_pLayout->getPendingWordForSpell();

			if (pPending->iOffset > iOffset)
				pPending->iOffset = (UT_uint32)((UT_sint32)pPending->iOffset + chg);

			m_pLayout->checkPendingWordForSpell();
		}
	}

	// recheck at boundary
	_recalcPendingWord(iOffset, chg);

	// check the newly pending word
//	m_pLayout->checkPendingWordForSpell();
#else
	m_pLayout->queueBlockForSpell(this);
#endif
}

void fl_BlockLayout::_mergeSquiggles(UT_uint32 iOffset, fl_BlockLayout* pPrevBL)
{
#ifdef FASTSQUIGGLE
	// when deleting block break, squiggles move in opposite direction
	UT_sint32 chg = (UT_sint32)iOffset;

	// move all trailing squiggles
	_moveSquiggles(0, chg, pPrevBL);

	// deal with previously pending word, if any
	if (m_pLayout->isPendingWordForSpell())
	{
		if (!m_pLayout->touchesPendingWordForSpell(this, iOffset, chg))
		{
			// not affected by delete, so check it
			fl_PartOfBlock* pPending = m_pLayout->getPendingWordForSpell();

			if (pPending->iOffset > iOffset)
				pPending->iOffset = (UT_uint32)((UT_sint32)pPending->iOffset + chg);

			m_pLayout->checkPendingWordForSpell();
		}
	}

	// TODO: may need to move pending word to this block

	// we're in the middle of the pending word
//	_recalcPendingWord(iOffset, 0);
#else
	m_pLayout->queueBlockForSpell(pPrevBL);
#endif

}

void fl_BlockLayout::_recalcPendingWord(UT_uint32 iOffset, UT_sint32 chg)
{
	// If spell-check-as-you-type is off, we don't want a pending word at all
	if (!m_pLayout->getAutoSpellCheck())
	{
		m_pLayout->setPendingWordForSpell(NULL, NULL);
		return;
	}
	
	UT_ASSERT(chg);

	// on entrance, the block is already changed & any pending word is junk
	// on exit, there's either a single unchecked pending word, or nothing

	UT_GrowBuf pgb(1024);
	bool bRes = getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar* pBlockText = pgb.getPointer(0);

	if (pBlockText == NULL)
	{
		return;
	}

	UT_uint32 iFirst = iOffset;
	UT_uint32 iAbs = (UT_uint32) ((chg > 0) ? chg : -chg);
	UT_uint32 iLen = ((chg > 0) ? iAbs : 0);

	/*
	  first, we expand this region outward until we get a word delimiter
	  on each side
	*/
	while ((iFirst > 0) && !UT_isWordDelimiter(pBlockText[iFirst-1], pBlockText[iFirst]))
	{
		iFirst--;
	}

	UT_ASSERT(iOffset>=iFirst);
	iLen += (iOffset-iFirst);

	UT_uint32 iBlockSize = pgb.getLength();
	while ((iFirst + iLen < iBlockSize))
	{
		UT_UCSChar followChar;
		followChar = ((iFirst + iLen + 1) < iBlockSize)  ?  pBlockText[iFirst + iLen + 1]  : UCS_UNKPUNK;
		if (UT_isWordDelimiter(pBlockText[iFirst + iLen], followChar)) break;
		iLen++;
	}

	/*
		then we figure out what to do with this expanded span
	*/
	if (chg > 0)
	{
		// insert
		UT_uint32 iLast = iOffset + chg;
		while (iLast > iFirst)
		{
			if (UT_isWordDelimiter(pBlockText[iLast-1], pBlockText[iLast])) break;
			iLast--;
		}

		if (iLast > (iFirst + 1))
		{
			// consume all words from the left ...
			_checkMultiWord(pBlockText, iFirst, iLast, false);
		}

		// ... except the last one, which is still pending
		iLen -= (iLast - iFirst);
		iFirst = iLast;
	}
	else
	{
		// delete
		UT_ASSERT(chg < 0);

		// everything's already set up, so just fall through
	}

	/*
		Is there a pending word left?
	*/
	if (iLen)
	{
		fl_PartOfBlock* pPending = NULL;
		bool bNew = false;

		if (m_pLayout->isPendingWordForSpell())
		{
			pPending = m_pLayout->getPendingWordForSpell();
			UT_ASSERT(pPending);
		}

		if (!pPending)
		{
			bNew = true;
			pPending = new fl_PartOfBlock();
			UT_ASSERT(pPending);
		}

		if (pPending)
		{
			pPending->iOffset = iFirst;
			pPending->iLength = iLen;

			if (bNew)
				m_pLayout->setPendingWordForSpell(this, pPending);
		}
	}
	else
	{
		// not pending any more
		m_pLayout->setPendingWordForSpell(NULL, NULL);
	}
}

void fl_BlockLayout::_moveSquiggles(UT_uint32 iOffset, UT_sint32 chg, fl_BlockLayout* pNewBlock)
{
	// move existing squiggles to reflect insert/delete at iOffset
	// all subsequent squiggles should be switched to (non-null) pBlock

	UT_uint32 target = (chg > 0) ? iOffset : (UT_uint32)((UT_sint32)iOffset - chg);

	UT_uint32 iSquiggles = m_vecSquiggles.getItemCount();
	UT_uint32 j;
	for (j=iSquiggles; j>0; j--)
	{
		fl_PartOfBlock* pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(j-1);

		// only interested in squiggles after change
		// overlapping squiggles get handled elsewhere
		if (pPOB->iOffset >= target)
		{
			// yep, this one moves
			pPOB->iOffset = (UT_uint32)((UT_sint32)pPOB->iOffset + chg);

			if (pNewBlock)
			{
				UT_ASSERT(pNewBlock != this);
					
				// move squiggle to another block
				pNewBlock->m_vecSquiggles.addItem(pPOB);
				m_vecSquiggles.deleteNthItem(j-1);
			}	
		}
	}
}

/*****************************************************************/
/*****************************************************************/

void fl_BlockLayout::checkSpelling(void)
{
	// destructively recheck the entire block
	// called from timer context, so we need to toggle IP

//
// Dont spell check non formatted blocks!
//
	if( m_pFirstRun->getLine() == NULL)
		return;

	UT_GrowBuf pgb(1024);
	bool bRes = getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar* pBlockText = pgb.getPointer(0);
	UT_uint32 eor = pgb.getLength(); /* end of region */
	FV_View* pView = m_pLayout->getView();
	bool bUpdateScreen = false;

	// remove any existing squiggles from the screen...
	UT_uint32 iSquiggles = m_vecSquiggles.getItemCount();
	UT_uint32 j;
	for (j=0; j<iSquiggles; j++)
	{
		fl_PartOfBlock* pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(j);

		if (pPOB->iOffset < eor)
		{
			// this one's still in the block
			if ((pPOB->iOffset + pPOB->iLength) > eor)
			{
				pPOB->iLength = eor - pPOB->iOffset;
			}

			if (!bUpdateScreen)
			{
				bUpdateScreen = true;
				if (pView)
					pView->_eraseInsertionPoint();
			}

			_updateSquiggle(pPOB);
		}
	}

	// ... and forget about them
	_purgeSquiggles();

	// now start checking
	bUpdateScreen |= _checkMultiWord(pBlockText, 0, eor, true);

	if (bUpdateScreen && pView)
	{
		pView->_eraseInsertionPoint();
		pView->updateScreen();
		pView->_drawInsertionPoint();
	}
}

bool fl_BlockLayout::_checkMultiWord(const UT_UCSChar* pBlockText,
									 UT_uint32 iStart, 
									 UT_uint32 eor, 
									 bool bToggleIP)
{
	bool bUpdateScreen = false;

	UT_uint32 wordBeginning = iStart;
	UT_uint32 wordLength = 0;
	bool bFound;
	bool bAllUpperCase, bHasNumeric;
	
	xxx_UT_DEBUGMSG(("fl_BlockLayout::_checkMultiWord\n"));

	while (wordBeginning < eor)
	{
		// skip delimiters...
		while (wordBeginning < eor)
		{
			if (!UT_isWordDelimiter(pBlockText[wordBeginning], UCS_UNKPUNK)) break;
			wordBeginning++;
		}

		if (wordBeginning < eor)
		{
			// we're at the start of a word. find end of word
			bFound = false;
			bAllUpperCase = true;
			bHasNumeric = false;
			wordLength = 0;
			while ((!bFound) && ((wordBeginning + wordLength) < eor))
			{
				UT_UCSChar followChar;
				followChar = ((wordBeginning + wordLength + 1) < eor)  ?  pBlockText[wordBeginning + wordLength + 1]  :  UCS_UNKPUNK;
				if ( true == UT_isWordDelimiter(pBlockText[wordBeginning + wordLength], followChar))
				{
					bFound = true;
				}
				else
				{
					if (bAllUpperCase)
						bAllUpperCase = UT_UCS_isupper(pBlockText[wordBeginning + wordLength]);

					if (!bHasNumeric)
						bHasNumeric = UT_UCS_isdigit(pBlockText[wordBeginning + wordLength]);

					wordLength++;
				}
			}


			// for some reason, the spell checker fails on all 1-char words & really big ones
			if ((wordLength > 1) &&
				XAP_EncodingManager::get_instance()->noncjk_letters(pBlockText+wordBeginning, wordLength) &&
				(!bAllUpperCase || !m_pLayout->getSpellCheckCaps()) &&		
				(!UT_UCS_isdigit(pBlockText[wordBeginning])) &&			// still ignore first char==num words
				(!bHasNumeric || !m_pLayout->getSpellCheckNumbers()) &&		// can these two lines be simplified?
				(wordLength < INPUTWORDLEN))
			{
				PD_Document * pDoc = m_pLayout->getDocument();
				XAP_App * pApp = XAP_App::getApp();

				UT_UCSChar theWord[INPUTWORDLEN + 1];

				UT_uint32 newLength = wordLength;
				for (UT_uint32 ldex=0; ldex<wordLength; ++ldex)
				{
					UT_UCSChar currentChar;
					currentChar = pBlockText[wordBeginning + ldex];

					// remove UCS_ABI_OBJECT from the word
					if (currentChar == UCS_ABI_OBJECT) {
					  newLength--; continue;
					}

				        // convert smart quote apostrophe to ASCII single quote to be compatible with ispell
					if (currentChar == UCS_RQUOTE) currentChar = '\'';
					theWord[ldex - (wordLength - newLength)] = currentChar;
				}

			   	if (!_spellCheckWord(theWord, newLength, wordBeginning) &&
					!pApp->isWordInDict(theWord, newLength))
				{
					bool bIsIgnored = pDoc->isIgnore(theWord, newLength);

					// unknown word...
					if (bToggleIP && !bUpdateScreen)
					{
						bUpdateScreen = true;
						FV_View* pView = m_pLayout->getView();

						if (pView)
							pView->_eraseInsertionPoint();
					}

					// squiggle it
					_addSquiggle(wordBeginning, wordLength, bIsIgnored);
				}
			}

			wordBeginning += (wordLength + 1);
		}
	}

	return bUpdateScreen;
}

bool fl_BlockLayout::checkWord(fl_PartOfBlock* pPOB)
{
	bool bUpdate = false;

	UT_ASSERT(pPOB);
	if (!pPOB)
		return bUpdate;

	// consume word in pPOB -- either squiggle or delete it

	UT_GrowBuf pgb(1024);
	bool bRes = getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	xxx_UT_DEBUGMSG(("fl_BlockLayout::checkWord\n"));

	const UT_UCSChar* pBlockText = pgb.getPointer(0);
	if (!pBlockText)
		return bUpdate;

	UT_uint32 eor = pPOB->iOffset + pPOB->iLength; /* end of region */

	UT_uint32 wordBeginning = pPOB->iOffset, wordLength = 0;
	bool bAllUpperCase = false;
	bool bHasNumeric = false;

	UT_ASSERT(wordBeginning <= pgb.getLength());
	UT_ASSERT(eor <= pgb.getLength());

	while (!bAllUpperCase && ((wordBeginning + wordLength) < eor))
	{
		UT_ASSERT(!UT_isWordDelimiter( pBlockText[wordBeginning + wordLength], 'a'));

		if (bAllUpperCase)
			bAllUpperCase = UT_UCS_isupper(pBlockText[wordBeginning + wordLength]);

		if (!bHasNumeric)
			bHasNumeric = UT_UCS_isdigit(pBlockText[wordBeginning + wordLength]);

		wordLength++;
	}

	wordLength = pPOB->iLength;

	// for some reason, the spell checker fails on all 1-char words & really big ones
	if ((wordLength > 1) &&
		XAP_EncodingManager::get_instance()->noncjk_letters(pBlockText+wordBeginning, wordLength) &&	
		(!m_pLayout->getSpellCheckCaps() || !bAllUpperCase) &&		
		(!UT_UCS_isdigit(pBlockText[wordBeginning])) &&			// still ignore first char==num words
		(!bHasNumeric || !m_pLayout->getSpellCheckNumbers()) &&		// can these two lines be simplified?
		(wordLength < INPUTWORDLEN))
	{
		PD_Document * pDoc = m_pLayout->getDocument();
		XAP_App * pApp = XAP_App::getApp();

		UT_UCSChar theWord[INPUTWORDLEN + 1];
		UT_uint32 newLength = wordLength;
		for (UT_uint32 ldex=0; ldex<wordLength; ++ldex)
		  {
		    UT_UCSChar currentChar;
		    currentChar = pBlockText[wordBeginning + ldex];
		    
		    // remove UCS_ABI_OBJECT from the word
		    if (currentChar == UCS_ABI_OBJECT) {
		      newLength--; continue;
		    }
		    
		    // convert smart quote apostrophe to ASCII single quote to be compatible with ispell
		    if (currentChar == UCS_RQUOTE) currentChar = '\'';
		    theWord[ldex - (wordLength - newLength)] = currentChar;
		  }

		if (!_spellCheckWord(theWord, newLength,wordBeginning) &&
			!pApp->isWordInDict(theWord, newLength))
		{
			// squiggle it
			m_vecSquiggles.addItem(pPOB);
			pPOB->bIsIgnored = pDoc->isIgnore(theWord, newLength);

			_updateSquiggle(pPOB);

			bUpdate = true;
		}
		else
		{
			// forget about it
			delete pPOB;
		}
	}
	else
	{
		// forget about it
		delete pPOB;
	}

	return bUpdate;
}

/*****************************************************************/
/*****************************************************************/

bool fl_BlockLayout::doclistener_populateSpan(const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len)
{
	_assertRunListIntegrity();

	FV_View* pView = m_pLayout->getView();
	if(pView)
		pView->eraseInsertionPoint();
	PT_BufIndex bi = pcrs->getBufIndex();
	if(getPrev()!= NULL && getPrev()->getLastLine()==NULL)
	{
		UT_DEBUGMSG(("In fl_BlockLayout::doclistener_populateSpan  no LastLine \n"));
		UT_DEBUGMSG(("getPrev = %d this = %d \n",getPrev(),this));
		//	        UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	const UT_UCSChar* pChars = m_pDoc->getPointer(bi);

	/*
	  walk through the characters provided and find any
	  control characters.  Then, each control character gets
	  handled specially.  Normal characters get grouped into
	  runs as usual.
	*/
	UT_uint32 	iNormalBase = 0;
	bool		bNormal = false;
	UT_uint32 i;
	for (i=0; i<len; i++)
	{
		switch (pChars[i])
		{
			// see similar control characters in fl_DocLayout.cpp
		case UCS_FF:	// form feed, forced page break
		case UCS_VTAB:	// vertical tab, forced column break
		case UCS_LF:	// newline
		case UCS_FIELDSTART: // zero length line to mark field start
		case UCS_FIELDEND: // zero length line to mark field end
		case UCS_TAB:	// tab
			if (bNormal)
			{
				_doInsertTextSpan(iNormalBase + blockOffset, i - iNormalBase);
				bNormal = false;
			}

			/*
			  Now, depending upon the kind of control char we found,
			  we add a control run which corresponds to it.
			*/
			switch (pChars[i])
			{
			case UCS_FF:
				_doInsertForcedPageBreakRun(i + blockOffset);
				break;
				
			case UCS_VTAB:
				_doInsertForcedColumnBreakRun(i + blockOffset);
				break;
				
			case UCS_LF:
				_doInsertForcedLineBreakRun(i + blockOffset);
				break;
				
			case UCS_FIELDSTART:
				_doInsertFieldStartRun(i + blockOffset);
				break;
				
			case UCS_FIELDEND:
				_doInsertFieldEndRun(i + blockOffset);
				break;
				
			case UCS_TAB:
				_doInsertTabRun(i + blockOffset);
				break;
				
			default:
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				break;
			}
			break;
			
		default:
			if (!bNormal)
			{
				bNormal = true;
				iNormalBase = i;
			}
			break;
		}
	}

	UT_ASSERT(i == len);

	if (bNormal && (iNormalBase < i))
	{
		_doInsertTextSpan(iNormalBase + blockOffset, i - iNormalBase);
	}

	_assertRunListIntegrity();

	return true;
}

bool	fl_BlockLayout::_doInsertTextSpan(PT_BlockOffset blockOffset, UT_uint32 len)
{
	FV_View* pView = m_pLayout->getView();
	if(pView)
		pView->eraseInsertionPoint();
	fp_TextRun* pNewRun = new fp_TextRun(this, m_pLayout->getGraphics(), blockOffset, len);
	UT_ASSERT(pNewRun);	// TODO check for outofmem

	if (_doInsertRun(pNewRun))
	{
#if 0
		/*
		  This code is an attempt to coalesce text runs on the fly.
		  It fails because the newly merged run is half-dirty,
		  half-not.  The newly inserted portion is clearly dirty.
		  It has not even been drawn on screen yet.  The previously
		  existent portion is not dirty.  If we want to do this
		  merge, then we have two choices.  First, we could
		  erase the old portion, merge, and consider the result
		  to be dirty.  OR, we could draw the new portion on
		  screen, merge, and consider the result to be NOT
		  dirty.  The first approach causes flicker.  The second
		  approach won't work now since we don't know the
		  position of the layout.
		*/
		
		fp_Run* pPrev = pNewRun->getPrev();
		if (
			pPrev
			&& (pPrev->getType() == FPRUN_TEXT)
			)
		{
			fp_TextRun* pPrevTextRun = (fp_TextRun*) pPrev;
			
			if (pPrevTextRun->canMergeWithNext())
			{
				pPrevTextRun->mergeWithNext();
			}
		}
#endif

#ifdef BIDI_ENABLED
		pNewRun->setDirection(-2);      //#TF need the the previous run to be set before we can do this
#endif
		return true;
	}
	else
	{
		return false;
	}
}

bool	fl_BlockLayout::_doInsertForcedLineBreakRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_ForcedLineBreakRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun);	// TODO check for outofmem

	bool bResult = _doInsertRun(pNewRun);
	if (bResult)
	    _breakLineAfterRun(pNewRun);

	return bResult;
}

bool	fl_BlockLayout::_doInsertFieldStartRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_FieldStartRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun);	// TODO check for outofmem

	bool bResult = _doInsertRun(pNewRun);
	if (bResult)
	    _breakLineAfterRun(pNewRun);

	return bResult;
}

bool	fl_BlockLayout::_doInsertFieldEndRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_FieldEndRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun);	// TODO check for outofmem

	bool bResult = _doInsertRun(pNewRun);
	if (bResult)
	    _breakLineAfterRun(pNewRun);

	return bResult;
}

bool	fl_BlockLayout::_doInsertForcedPageBreakRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_ForcedPageBreakRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun);	// TODO check for outofmem
	if(getPrev()!= NULL && getPrev()->getLastLine()==NULL)
	{
		UT_DEBUGMSG(("In fl_BlockLayout::_doInsertForcedPageBreakRun  no LastLine \n"));
		UT_DEBUGMSG(("getPrev = %d this = %d \n",getPrev(),this));
		//UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	bool bResult = _doInsertRun(pNewRun);
	if (bResult)
	    _breakLineAfterRun(pNewRun);

	return bResult;
}

bool	fl_BlockLayout::_doInsertForcedColumnBreakRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_ForcedColumnBreakRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun);	// TODO check for outofmem

	bool bResult = _doInsertRun(pNewRun);
	if (bResult)
	    _breakLineAfterRun(pNewRun);

	return bResult;
}

bool	fl_BlockLayout::_doInsertTabRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_TabRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun);	// TODO check for outofmem

	return _doInsertRun(pNewRun);
}

bool	fl_BlockLayout::_doInsertImageRun(PT_BlockOffset blockOffset, FG_Graphic* pFG)
{
	GR_Image* pImage = pFG->generateImage(m_pLayout->getGraphics());

	fp_ImageRun* pNewRun = new fp_ImageRun(this, m_pLayout->getGraphics(), blockOffset, 1, pImage);
	UT_ASSERT(pNewRun);	// TODO check for outofmem

	return _doInsertRun(pNewRun);
}

bool	fl_BlockLayout::_doInsertFieldRun(PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro /* pcro */)
{
	const PP_AttrProp * pSpanAP = NULL;
	
	getSpanAttrProp(blockOffset, false, &pSpanAP);
	UT_ASSERT(pSpanAP);

	// Get the field type.

	const XML_Char* pszType = NULL;
	pSpanAP->getAttribute("type", pszType);
	UT_ASSERT(pszType);

	// Create the field run.

	fp_FieldRun* pNewRun;

	UT_DEBUGMSG(("DOM: field type: %s\n", pszType));

	if(UT_strcmp(pszType, "list_label") == 0)
	{
		pNewRun = new fp_FieldListLabelRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "time") == 0)
	{
		pNewRun = new fp_FieldTimeRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "page_number") == 0)
	{
		pNewRun = new fp_FieldPageNumberRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "page_count") == 0)
	{
		pNewRun = new fp_FieldPageCountRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "date") == 0)
	{
		pNewRun = new fp_FieldDateRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "date_mmddyy") == 0)
	{
	    pNewRun = new fp_FieldMMDDYYRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "date_ddmmyy") == 0)
	{
	    pNewRun = new fp_FieldDDMMYYRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "date_mdy") == 0)
	{
	    pNewRun = new fp_FieldMonthDayYearRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "date_mthdy") == 0)
	{
	    pNewRun = new fp_FieldMthDayYearRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "date_dfl") == 0)
	{
	    pNewRun = new fp_FieldDefaultDateRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "date_ntdfl") == 0)
	{
	    pNewRun = new fp_FieldDefaultDateNoTimeRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "date_wkday") == 0)
	{
	    pNewRun = new fp_FieldWkdayRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "date_doy") == 0)
	{
	    pNewRun = new fp_FieldDOYRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "time_miltime") == 0)
	{
	    pNewRun = new fp_FieldMilTimeRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "time_ampm") == 0)
	{
	    pNewRun = new fp_FieldAMPMRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "time_zone") == 0)
	{
	    pNewRun = new fp_FieldTimeZoneRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "time_epoch") == 0)
	{
	    pNewRun = new fp_FieldTimeEpochRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "word_count") == 0)
	{
	    pNewRun = new fp_FieldWordCountRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "char_count") == 0)
	{
	    pNewRun = new fp_FieldCharCountRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "line_count") == 0)
	{
	    pNewRun = new fp_FieldLineCountRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "para_count") == 0)
	{
	    pNewRun = new fp_FieldParaCountRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "nbsp_count") == 0)
	{
	    pNewRun = new fp_FieldNonBlankCharCountRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "file_name") == 0)
	{
	    pNewRun = new fp_FieldFileNameRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "app_ver") == 0)
	{
	    pNewRun = new fp_FieldBuildVersionRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "app_id") == 0)
	{
	    pNewRun = new fp_FieldBuildIdRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "app_options") == 0)
	  {
	    pNewRun = new fp_FieldBuildOptionsRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else if(UT_strcmp(pszType, "app_target") == 0)
	  {
	    pNewRun = new fp_FieldBuildTargetRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else if(UT_strcmp(pszType, "app_compiledate") == 0)
	  {
	    pNewRun = new fp_FieldBuildCompileDateRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else if(UT_strcmp(pszType, "app_compiletime") == 0)
	  {
	    pNewRun = new fp_FieldBuildCompileTimeRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else
	{
		//		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		//	pNewRun = NULL;
		// New Piece Table Field Leave it for that code..
		//
		pNewRun = new fp_FieldRun(this, m_pLayout->getGraphics(), blockOffset, 1);
		//		return _doInsertRun(pNewRun);
	}

	UT_ASSERT(pNewRun);	// TODO check for outofmem
	
	pNewRun->lookupProperties();
	pNewRun->calculateValue();

	_doInsertRun(pNewRun);
	recalculateFields();
	return true;
}

FV_View * fl_BlockLayout::getView( void)
{
	return  m_pLayout->getView();
}

bool	fl_BlockLayout::_doInsertRun(fp_Run* pNewRun)
{
	PT_BlockOffset blockOffset = pNewRun->getBlockOffset();
	UT_uint32 len = pNewRun->getLength();
	
	_assertRunListIntegrity();


	FV_View* ppView = m_pLayout->getView();
	if(ppView)
		ppView->eraseInsertionPoint();
	
	m_gbCharWidths.ins(blockOffset, len);
	if (pNewRun->getType() == FPRUN_TEXT)
	{
		fp_TextRun* pNewTextRun = (fp_TextRun*) pNewRun;
		
		pNewTextRun->fetchCharWidths(&m_gbCharWidths);
		pNewTextRun->recalcWidth();
	}
	
	bool bInserted = false;
	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		UT_uint32 iRunBlockOffset = pRun->getBlockOffset();
		UT_uint32 iRunLength = pRun->getLength();
		
		if ( (iRunBlockOffset + iRunLength) <= blockOffset )
		{
			// nothing to do.  the insert occurred AFTER this run
		}
		else if (iRunBlockOffset > blockOffset)
		{
			UT_ASSERT(bInserted);
			
			// the insert is occuring BEFORE this run, so we just move the run offset
			pRun->setBlockOffset(iRunBlockOffset + len);
		}
		else if (iRunBlockOffset == blockOffset)
		{
			UT_ASSERT(!bInserted);

			bInserted = true;
			
			// the insert is right before this run.
			pRun->setBlockOffset(iRunBlockOffset + len);

			pRun->insertIntoRunListBeforeThis(*pNewRun);

			if (m_pFirstRun == pRun)
			{
				m_pFirstRun = pNewRun;
			}
			if(pRun->getLine())
				pRun->getLine()->insertRunBefore(pNewRun, pRun);
		}
		else
		{
			UT_ASSERT(!bInserted);
			
			UT_ASSERT((blockOffset >= pRun->getBlockOffset()) &&
					  (blockOffset <
					   (pRun->getBlockOffset() + pRun->getLength())));
			UT_ASSERT(pRun->getType() == FPRUN_TEXT);	// only textual runs can be split anyway

			fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pRun);
			pTextRun->split(blockOffset);
			
			UT_ASSERT(pRun->getNext());
			UT_ASSERT(pRun->getNext()->getBlockOffset() == blockOffset);

			UT_ASSERT(pTextRun->getNext());
			UT_ASSERT(pTextRun->getNext()->getType() == FPRUN_TEXT);

// sterwill -- is the call to getNext() needed?  pOtherHalfOfSplitRun
//             is not used.
			
//			fp_TextRun* pOtherHalfOfSplitRun = (fp_TextRun*) pTextRun->getNext();
			
//			pTextRun->recalcWidth();

			bInserted = true;
			
			pRun = pRun->getNext();
			
			iRunBlockOffset = pRun->getBlockOffset();
			iRunLength = pRun->getLength();

			UT_ASSERT(iRunBlockOffset == blockOffset);
			
			// the insert is right before this run.
			pRun->setBlockOffset(iRunBlockOffset + len);

			pRun->insertIntoRunListBeforeThis(*pNewRun);
			if(pRun->getLine())
				pRun->getLine()->insertRunBefore(pNewRun, pRun);
			
//			pOtherHalfOfSplitRun->recalcWidth();
		}
		pRun = pRun->getNext();
	}

	if (!bInserted)
	{
		pRun = m_pFirstRun;
		fp_Run * pLastRun = NULL;
		UT_uint32 offset = 0;
		while (pRun)
		{
			pLastRun = pRun;
			offset += pRun->getLength();
			pRun = pRun->getNext();
		}

		UT_ASSERT(offset==blockOffset);

		if (pLastRun)
		{
			pLastRun->insertIntoRunListAfterThis(*pNewRun);
		}
		else
		{
			m_pFirstRun = pNewRun;
		}

		if (m_pLastLine)
		{
			m_pLastLine->addRun(pNewRun);
		}
	}

	_assertRunListIntegrity();
	
	return true;
}

bool fl_BlockLayout::doclistener_insertSpan(const PX_ChangeRecord_Span * pcrs)
{
	_assertRunListIntegrity();

	UT_ASSERT(pcrs->getType()==PX_ChangeRecord::PXT_InsertSpan);
	//UT_ASSERT(pcrs->getPosition() >= getPosition());		/* valid assert, but very expensive */
	
	PT_BlockOffset blockOffset = pcrs->getBlockOffset();
	UT_uint32 len = pcrs->getLength();
	UT_ASSERT(len>0);

	PT_BufIndex bi = pcrs->getBufIndex();
	const UT_UCSChar* pChars = m_pDoc->getPointer(bi);

	/*
	  walk through the characters provided and find any
	  control characters.  Then, each control character gets
	  handled specially.  Normal characters get grouped into
	  runs as usual.
	*/
	UT_uint32 	iNormalBase = 0;
	bool	bNormal = false;
	UT_uint32 i;
	UT_uint32 _sqlist[100], *sqlist = _sqlist;
	UT_uint32 sqcount = 0;
	if (sizeof(_sqlist) / sizeof(_sqlist[0])  < len)
	{
		sqlist = new UT_uint32[len];
	}
	xxx_UT_DEBUGMSG(("fl_BlockLayout::doclistener_insertSpan(), len=%d, c=|%c|\n", len, pChars[0]));
	for (i=0; i<len; i++)
	{
		switch (pChars[i])
		{
		case UCS_FF:	// form feed, forced page break
		case UCS_VTAB:	// vertical tab, forced column break
		case UCS_LF:	// newline
		case UCS_FIELDSTART: // zero length line to mark field start
		case UCS_FIELDEND: // zero length line to mark field end
		case UCS_TAB:	// tab
			if (bNormal)
			{
				_doInsertTextSpan(blockOffset + iNormalBase, i - iNormalBase);
				bNormal = false;
			}

			/*
			  Now, depending upon the kind of control char we found,
			  we add a control run which corresponds to it.
			*/
			switch (pChars[i])
			{
			case UCS_FF:
				_doInsertForcedPageBreakRun(i + blockOffset);
				break;
				
			case UCS_VTAB:
				_doInsertForcedColumnBreakRun(i + blockOffset);
				break;
				
			case UCS_LF:
				_doInsertForcedLineBreakRun(i + blockOffset);
				break;

			case UCS_FIELDSTART:
				_doInsertFieldStartRun(i + blockOffset);
				break;
				
			case UCS_FIELDEND:
				_doInsertFieldEndRun(i + blockOffset);
				break;
				
			case UCS_TAB:
				_doInsertTabRun(i + blockOffset);
				break;
				
			default:
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				break;
			}
			break;
			
		default:
			if ((i != len-1)  &&  UT_isSmartQuotableCharacter(pChars[i]))
			{
				// accumulate smart quote candidates and deal with them
				// as a bunch below after the final text insertion has
				// been dealt with
				sqlist[sqcount++] = blockOffset + i;
			}
			if (!bNormal)
			{
				bNormal = true;
				iNormalBase = i;
			}
			break;
		}
	}

	UT_ASSERT(i == len);

	if (bNormal && (iNormalBase < i))
	{
		_doInsertTextSpan(blockOffset + iNormalBase, i - iNormalBase);
	}

	setNeedsReformat();

	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->_setPoint(pcrs->getPosition() + len);
		pView->notifyListeners(AV_CHG_FMTCHAR); // TODO verify that this is necessary.
	}
	else if(pView && pView->getPoint() > pcrs->getPosition())
		pView->_setPoint(pView->getPoint() + len);

	if (m_pLayout->hasBackgroundCheckReason(FL_DocLayout::bgcrSmartQuotes))
	{
		fl_BlockLayout *sq_bl = m_pLayout->getPendingBlockForSmartQuote();
		UT_uint32 sq_of = m_pLayout->getOffsetForSmartQuote();
		m_pLayout->setPendingSmartQuote(NULL, 0);
		if (sq_bl)
		{
			m_pLayout->considerSmartQuoteCandidateAt(sq_bl, sq_of);
		}
		if (sqcount)
		{
			m_pDoc->beginUserAtomicGlob();
			for (UT_uint32 sdex=0; sdex<sqcount; ++sdex)
			{
				m_pLayout->considerSmartQuoteCandidateAt(this, sqlist[sdex]);
			}
			m_pDoc->endUserAtomicGlob();
		}
		if (UT_isSmartQuotableCharacter(pChars[len - 1]))
		{
			m_pLayout->setPendingSmartQuote(this, blockOffset + len - 1);
		}
	}
	if (sqlist != _sqlist) delete[] sqlist;

	if (m_pLayout->getAutoSpellCheck())
		_insertSquiggles(blockOffset, len);

	_assertRunListIntegrity();

	return true;
}

#ifndef NDEBUG
/*!
  Assert integrity of the Run list
  Assert the following properties:
   - Offsets are correct
   - No adjacent FmtMark Runs
   - Only FmtMark Runs have length zero
   - List ends in an EOP Run
*/
void
fl_BlockLayout::_assertRunListIntegrityImpl(void)
{
	fp_Run* pRun = m_pFirstRun;
	UT_uint32 iOffset = 0;
	while (pRun)
	{
		// Verify that offset of this block is correct.
		UT_ASSERT( iOffset == pRun->getBlockOffset() );

		iOffset += pRun->getLength();

		// Verify that we don't have two adjacent FmtMarks.
		UT_ASSERT( ((pRun->getType() != FPRUN_FMTMARK) 
					|| !pRun->getNext()
					|| (pRun->getNext()->getType() != FPRUN_FMTMARK)) );

		// Verify that the Run has a non-zero length (or is a FmtMark)
		UT_ASSERT( (FPRUN_FMTMARK == pRun->getType())
				   || (pRun->getLength() > 0) );

		// Verify that if there is no next Run, this Run is the EOP Run.
		// Or we're in the middle of loading a document.
		UT_ASSERT( pRun->getNext() 
				   || (FPRUN_ENDOFPARAGRAPH == pRun->getType()) );

		pRun = pRun->getNext();
	}
}
#endif /* !NDEBUG */

inline void
fl_BlockLayout::_assertRunListIntegrity(void)
{
#ifndef NDEBUG
	_assertRunListIntegrityImpl();
#endif
}


bool fl_BlockLayout::_delete(PT_BlockOffset blockOffset, UT_uint32 len)
{
	_assertRunListIntegrity();

	/* TODO the attempts herein to do fetchCharWidths will fail. */
	
	m_gbCharWidths.del(blockOffset, len);

	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		UT_uint32 iRunBlockOffset = pRun->getBlockOffset();
		UT_uint32 iRunLength = pRun->getLength();
		fp_Run* pNextRun = pRun->getNext();	// remember where we're going, since this run may get axed
		
		if ( (iRunBlockOffset + iRunLength) <= blockOffset )
		{
			// nothing to do.  the delete occurred AFTER this run
		}
		else if (iRunBlockOffset >= (blockOffset + len))
		{
			// the delete occurred entirely before this run.

			pRun->setBlockOffset(iRunBlockOffset - len);
		}
		else
		{
			if (blockOffset >= iRunBlockOffset)
			{
				if ((blockOffset + len) < (iRunBlockOffset + iRunLength))
				{
					// the deleted section is entirely within this run
					pRun->setLength(iRunLength - len);
					UT_ASSERT((pRun->getLength() == 0) || (pRun->getType() == FPRUN_TEXT));	// only textual runs could have a partial deletion
					m_bFixCharWidths = true;
				}
				else
				{
					int iDeleted = iRunBlockOffset + iRunLength - blockOffset;
					UT_ASSERT(iDeleted > 0);

					pRun->setLength(iRunLength - iDeleted);
					UT_ASSERT((pRun->getLength() == 0) || (pRun->getType() == FPRUN_TEXT));	// only textual runs could have a partial deletion
					m_bFixCharWidths = true;
				}
			}
			else
			{
				if ((blockOffset + len) < (iRunBlockOffset + iRunLength))
				{
					int iDeleted = blockOffset + len - iRunBlockOffset;
					UT_ASSERT(iDeleted > 0);
					pRun->setBlockOffset(iRunBlockOffset - (len - iDeleted));
					pRun->setLength(iRunLength - iDeleted);
					UT_ASSERT((pRun->getLength() == 0) || (pRun->getType() == FPRUN_TEXT));	// only textual runs could have a partial deletion
					m_bFixCharWidths = true;
				}
				else
				{
					/* the deletion spans the entire run. time to delete it */
					pRun->setLength(0);
				}
			}

			if ((pRun->getLength() == 0) && (pRun->getType() != FPRUN_FMTMARK))
			{
				// Remove Run from line
				fp_Line* pLine = pRun->getLine();
				UT_ASSERT(pLine);
				pLine->removeRun(pRun, true);

				// Unlink Run and delete it
				if (m_pFirstRun == pRun)
				{
					m_pFirstRun = pRun->getNext();
				}
				pRun->unlinkFromRunList();
				delete pRun;

				if (!m_pFirstRun)
				{
					// When deleting content in a block, the EOP Run
					// should always remain.
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					_insertEndOfParagraphRun();
				}
			}
		}

		pRun = pNextRun;
	}

	_assertRunListIntegrity();

	return true;
}

bool fl_BlockLayout::doclistener_deleteSpan(const PX_ChangeRecord_Span * pcrs)
{
	_assertRunListIntegrity();

	UT_ASSERT(pcrs->getType()==PX_ChangeRecord::PXT_DeleteSpan);
			
	PT_BlockOffset blockOffset = pcrs->getBlockOffset();
	UT_uint32 len = pcrs->getLength();
	UT_ASSERT(len>0);

	_delete(blockOffset, len);
	
	setNeedsReformat();

	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
 		pView->_resetSelection();
		pView->_setPoint(pcrs->getPosition());
	}
	else if(pView && pView->getPoint() > pcrs->getPosition())
	{
		if(pView->getPoint() <= pcrs->getPosition() + len)
			pView->_setPoint(pcrs->getPosition());
		else pView->_setPoint(pView->getPoint() - len);
	}

	if (m_pLayout->getAutoSpellCheck())
		_deleteSquiggles(blockOffset, len);

	_assertRunListIntegrity();

	return true;
}

/*!
  Change runs in the given span
  \param pcrsc Specifies the span

  This function makes all fp_Run objects within the given span lookup
  new properties and recalculate their width. Runs at the ends of the
  span that extend over the span border will be split, so runs fall
  entirely inside or outside of the span.
*/
bool fl_BlockLayout::doclistener_changeSpan(const PX_ChangeRecord_SpanChange * pcrsc)
{
	_assertRunListIntegrity();

	UT_ASSERT(pcrsc->getType()==PX_ChangeRecord::PXT_ChangeSpan);
		
	PT_BlockOffset blockOffset = pcrsc->getBlockOffset();
	UT_uint32 len = pcrsc->getLength();
	UT_ASSERT(len > 0);

	// First look for the first run inside the span
	fp_Run* pRun = m_pFirstRun;
	fp_Run* pPrevRun = NULL;
	while (pRun && pRun->getBlockOffset() < blockOffset)
	{
		pPrevRun = pRun;
		pRun = pRun->getNext();
	}

	// If pRun is now at the blockOffset, the span falls on an
	// existing separation between runs. If not, we have to split the
	// run (pPrevRun) which is extending over the border.
	if (!pRun || (pRun->getBlockOffset() != blockOffset))
	{
		// Need to split previous Run.
		// Note: That should be a fp_TextRun. If not, we'll keep going
		// using the first run fully inside the span - but keep the
		// assertion for alert in debug builds.
		UT_ASSERT(pPrevRun);
		UT_ASSERT(FPRUN_TEXT == pPrevRun->getType());
		if (FPRUN_TEXT == pPrevRun->getType())
		{
			fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pPrevRun);
			pTextRun->split(blockOffset);
		}
		pRun = pPrevRun->getNext();
	}

	// When we get here, we have a clean separation on the left
	// between what's outside and what's inside of the span. pRun is
	// the first run inside the span.
	UT_ASSERT(!pRun || (blockOffset == pRun->getBlockOffset()));

	// Now start forcing the runs to update
	while (pRun)
	{
		// If the run is on the right of the span, we're done
		if ((pRun->getBlockOffset() >= (blockOffset + len)))
			break;

		// If the run extends beyond the span, split it.
		if ((pRun->getBlockOffset() + pRun->getLength()) > (blockOffset + len))
		{
			// Note: That should be a fp_TextRun. If not, we'll just
			// have to update the entire run - but keep the assertion
			// for alert in debug builds.
			UT_ASSERT(FPRUN_TEXT == pRun->getType());
			if (FPRUN_TEXT == pRun->getType())
			{
				fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pRun);
				pTextRun->split(blockOffset+len);
			}
		}

		// FIXME:jskov Here we want to call a changeSpanMember
		// function in the Run which decides how to behave. That way
		// we don't forget new Run types as they get added, and
		// show-paragraphs mode can be handled properly.

		// Make the run update its properties and recalculate width as
		// necessary.
		if (pRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pRun);
			pTextRun->lookupProperties();
			pTextRun->fetchCharWidths(&m_gbCharWidths);
			pTextRun->recalcWidth();
		}
		else if (pRun->getType() == FPRUN_TAB)
		{
			pRun->lookupProperties();
		}
		// TODO: do we need to call lookupProperties for other run types.

		pRun = pRun->getNext();
	}

	setNeedsReformat();

	_assertRunListIntegrity();

	return true;
}

/*!
  Delete strux Run
  \param pcrx Change record for the operation
  \return true if succeeded, false if not
  This function will merge the content of this strux to the previous
  strux. 
*/
bool fl_BlockLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux* pcrx)
{
	_assertRunListIntegrity();

	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
	UT_ASSERT(pcrx->getStruxType()==PTX_Block);

	// First see if the block in a list. If so remove it!
	if(m_pAutoNum != NULL)
	{
		if( m_pAutoNum->isItem(getStruxDocHandle()) == true)
		{
			// This nifty method handles all the details
			m_pAutoNum->removeItem(getStruxDocHandle());
		}
	}

	// Erase the old version.  Or this what I added when adding the
	// EOP stuff. Only, I don't remember why I did it, and it's wrong:
	// the strux is deleted only after its content has been deleted -
	// so the call might try to clear empty lines. jskov 2001.04.23
//	clearScreen(m_pLayout->getGraphics());

	// If there is a previous strux, we merge the Runs from this strux
	// into it - including the EOP Run, so delete that in the previous
	// strux.
	// If there is no previous strux (this being the first strux in
	// the document) this will be empty - but the EOP Run needs to be
	// deleted.

	fl_BlockLayout*	pPrevBL = m_pPrev;
	if (pPrevBL)
	{
		// Find the EOP Run.
		fp_Run* pPrevRun = NULL;
		fp_Run* pNukeRun = pPrevBL->m_pFirstRun;
		for (; pNukeRun->getNext(); pNukeRun = pNukeRun->getNext())
		{
			pPrevRun = pNukeRun;
			UT_ASSERT(FPRUN_ENDOFPARAGRAPH != pPrevRun->getType());
		}
		UT_ASSERT(FPRUN_ENDOFPARAGRAPH == pNukeRun->getType());

		// Detach from the line
		fp_Line* pLine = pNukeRun->getLine();
		UT_ASSERT(pLine);
		pLine->removeRun(pNukeRun);

		// Unlink and delete it
		if (pPrevRun)
		{
			pPrevRun->setNext(NULL);
		}
		else
		{
			pPrevBL->m_pFirstRun = NULL;
		}
		delete pNukeRun;
	}
	else
	{
		// Delete end-of-paragraph Run in this strux
		UT_ASSERT(m_pFirstRun 
				  && (FPRUN_ENDOFPARAGRAPH == m_pFirstRun->getType()));

		fp_Run* pNukeRun = m_pFirstRun;

		// Detach from the line
		fp_Line* pLine = pNukeRun->getLine();
		UT_ASSERT(pLine);
		pLine->removeRun(pNukeRun);

		// Unlink and delete it
		m_pFirstRun = NULL;
		delete pNukeRun;

	}

	// The idea here is to append the runs of the deleted block, if
	// any, at the end of the previous block.
	UT_uint32 offset = 0;
	if (m_pFirstRun)
	{
		// Figure out where the merge point is
		fp_Run * pRun = pPrevBL->m_pFirstRun;
		fp_Run * pLastRun = NULL;
		while (pRun)
		{
			pLastRun = pRun;
			offset += pRun->getLength();
			pRun = pRun->getNext();
		}

		// Link them together
		if (pLastRun)
		{
			pLastRun->setNext(m_pFirstRun);
			if(m_pFirstRun)
			{
				m_pFirstRun->setPrev(pLastRun);
			}
		}
		else
		{
			pPrevBL->m_pFirstRun = m_pFirstRun;
		}

		// Merge charwidths
		UT_uint32 lenNew = m_gbCharWidths.getLength();

		pPrevBL->m_gbCharWidths.ins(offset, m_gbCharWidths, 0, lenNew);

		fp_Line* pLastLine = pPrevBL->getLastLine();
		
		// Tell all the new runs where they live
		pRun = m_pFirstRun;
		while (pRun)
		{
			pRun->setBlockOffset(pRun->getBlockOffset() + offset);
			pRun->setBlock(pPrevBL);
			
			// Detach from their line
			fp_Line* pLine = pRun->getLine();
			UT_ASSERT(pLine);
			pLine->removeRun(pRun);

			pLastLine->addRun(pRun);

			pRun = pRun->getNext();
		}

		// Runs are no longer attached to this block
		m_pFirstRun = NULL;
	}

	// Get rid of everything else about the block
	purgeLayout();

	// Unlink this block
	if (pPrevBL)
	{
		pPrevBL->m_pNext = m_pNext;
	}
	if (m_pNext)
	{
		m_pNext->m_pPrev = pPrevBL;
	}

	fl_SectionLayout* pSL = m_pSectionLayout;
	UT_ASSERT(pSL);
	pSL->removeBlock(this);

	if (pPrevBL)
	{	
		// Move all squiggles to previous block
		_mergeSquiggles(offset, pPrevBL);

		// Update the display
//		pPrevBL->_lookupProperties();	// TODO: this may be needed
		pPrevBL->setNeedsReformat();

		//Fix for bug #1119, but only do it when auto spelling is
		//enabled, or we get squiggles in the document when clearing
		//selected text. See bug #1285
		if (m_pLayout->getAutoSpellCheck())
		{
			pPrevBL->checkSpelling();
		}
	}

	// In case we've never checked this one
	m_pLayout->dequeueBlockForBackgroundCheck(this);

	FV_View* pView = pSL->getDocLayout()->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->_setPoint(pcrx->getPosition());
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->_setPoint(pView->getPoint() - 1);
	}

	_assertRunListIntegrity();

	delete this;			// FIXME: whoa!  this construct is VERY dangerous.

	return true;
}

bool fl_BlockLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	_assertRunListIntegrity();

	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);

	FV_View* ppView = m_pLayout->getView();
	if(ppView)
	{
		ppView->eraseInsertionPoint();
		m_bCursorErased = true;
	}

	// erase the old version
	clearScreen(m_pLayout->getGraphics());
	setAttrPropIndex(pcrxc->getIndexAP());
	UT_DEBUGMSG(("SEVIOR: In changeStrux in fl_BlockLayout \n"));

	const XML_Char * szOldStyle = m_szStyle;
#ifdef BIDI_ENABLED
	bool bOldDomDirection = m_bDomDirection;
#endif
	_lookupProperties();
	UT_DEBUGMSG(("SEVIOR: Old Style = %s new style = %s \n",szOldStyle,m_szStyle));
	if(ppView)
		ppView->eraseInsertionPoint();
//
// Not sure why we need this IF - Sevior
//	if ((szOldStyle != m_szStyle) &&
//		(!szOldStyle || !m_szStyle || !!(UT_XML_strcmp(szOldStyle, m_szStyle))))
	{
		/*
		  A block-level style change means that we also need to update
		  all the run-level properties.
		*/
		fp_Run* pRun = m_pFirstRun;

		UT_DEBUGMSG(("SEVIOR: Doing a style change \n"));
		while (pRun)
		{
			pRun->lookupProperties();
			pRun->fetchCharWidths(&m_gbCharWidths);
			pRun->recalcWidth();

			pRun = pRun->getNext();
		}
	}

	fp_Line* pLine = m_pFirstLine;
	while (pLine)
	{
		pLine->recalcHeight();	// line-height
		pLine->recalcMaxWidth();
#ifdef BIDI_ENABLED
		if(m_bDomDirection != bOldDomDirection)
		{
			pLine->setMapOfRunsDirty();
		}
#endif
		pLine = pLine->getNext();
	}
	
	setNeedsReformat();
	m_bCursorErased = false;

	_assertRunListIntegrity();

	return true;
}

bool fl_BlockLayout::doclistener_insertFirstBlock(const PX_ChangeRecord_Strux * pcrx,
												  PL_StruxDocHandle sdh,
												  PL_ListenerId lid,
												  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																		  PL_ListenerId lid,
																		  PL_StruxFmtHandle sfhNew))
{
	//  Exchange handles with the piece table
	PL_StruxFmtHandle sfhNew = (PL_StruxFmtHandle)this;
	pfnBindHandles(sdh,lid,sfhNew);

	setNeedsReformat();

	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
		pView->_setPoint(pcrx->getPosition());
	else if (pView) pView->_setPoint(pView->getPoint() + fl_BLOCK_STRUX_OFFSET);

	// Run list should be valid now.
	_assertRunListIntegrity();

	return true;
}
bool fl_BlockLayout::doclistener_insertBlock(const PX_ChangeRecord_Strux * pcrx,
											 PL_StruxDocHandle sdh,
											 PL_ListenerId lid,
											 void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	 PL_ListenerId lid,
																	 PL_StruxFmtHandle sfhNew))
{
	_assertRunListIntegrity();

	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_InsertStrux);
	UT_ASSERT(pcrx->getStruxType()==PTX_Block);

	FV_View* ppView = m_pLayout->getView();
	if(ppView)
		ppView->eraseInsertionPoint();

	fl_SectionLayout* pSL = m_pSectionLayout;
	UT_ASSERT(pSL);
	fl_BlockLayout*	pNewBL = pSL->insertBlock(sdh, this, pcrx->getIndexAP());
	if (!pNewBL)
	{
		UT_DEBUGMSG(("no memory for BlockLayout\n"));
		return false;
	}
	// The newly returned block will contain a line and EOP. Delete those
	// since the code below expects an empty block
	pNewBL->_purgeEndOfParagraphRun();

	if(ppView)
		ppView->eraseInsertionPoint();

	// Must call the bind function to complete the exchange
	// of handles with the document (piece table) *** before ***
	// anything tries to call down into the document (like all
	// of the view listeners).

	PL_StruxFmtHandle sfhNew = (PL_StruxFmtHandle)pNewBL;
	pfnBindHandles(sdh,lid,sfhNew);
	
	/*
	  The idea here is to divide the runs of the existing block
	  into two equivalence classes.  This may involve
	  splitting an existing run.
	
	  All runs and lines remaining in the existing block are
	  fine, although the last run should be redrawn.
	
	  All runs in the new block need their offsets fixed, and
	  that entire block needs to be formatted from scratch.

	  TODO is the above commentary still correct ??
	*/

	// figure out where the breakpoint is
	PT_BlockOffset blockOffset = (pcrx->getPosition() - getPosition());

	fp_Run* pFirstNewRun = NULL;
	fp_Run* pLastRun = NULL;
	fp_Run* pRun;
	for (pRun=m_pFirstRun; (pRun && !pFirstNewRun); 
		 pLastRun=pRun, pRun=pRun->getNext())
	{
		// We have passed the point. Why didn't previous Run claim to
		// hold the offset? Make the best of it in non-debug
		// builds. But keep the assert to get us information...
		if (pRun->getBlockOffset() > blockOffset)
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			pFirstNewRun = pRun;
			break;
		}

		// FIXME: Room for optimization improvement here - always scan
		// FIXME: past the point by only comparing getBlockOffset.

		if (pRun->getBlockOffset() <= blockOffset &&
			(pRun->getBlockOffset() + pRun->getLength()) > blockOffset)
		{
			// We found the Run. Now handle splitting.

			if (pRun->getBlockOffset() == blockOffset)
			{
				// Split between this Run and the previous one
				pFirstNewRun = pRun;
			}
			else
			{
				// Need to split current Run
				UT_ASSERT(pRun->getType() == FPRUN_TEXT);

				// split here
				fp_TextRun* pTextRun = (fp_TextRun*) pRun;
				pTextRun->split(blockOffset);
				pFirstNewRun = pRun->getNext();
			}
			break;
		}
	}

	if (pFirstNewRun && (pFirstNewRun->getType() == FPRUN_FMTMARK))
	{
		// Since a FmtMark has length zero, both it and the next run
		// have the same blockOffset.  We always want to be to the
		// right of the FmtMark, so we take the next one.
		pFirstNewRun = pFirstNewRun->getNext();
	}

	if (pFirstNewRun)
	{
		if (pFirstNewRun->getPrev())
		{
			// Break doubly-linked list of runs into two distinct lists.
			// But remember the last Run in this block.

			pLastRun = pFirstNewRun->getPrev();
			pFirstNewRun->getPrev()->setNext(NULL);
			pFirstNewRun->setPrev(NULL);
		}
		else
			pLastRun = NULL;
	}
	// else the old value of pLastRun is what we want.

	// pFirstNewRun can be NULL at this point.  It means that the
	// entire set of runs in this block must remain with this block --
	// and the newly created block will be empty.
	//
	// Also, note if pFirstNewRun == m_pFirstRun then we will be moving
	// the entire set of runs to the newly created block -- and leave
	// the current block empty.

	// Split charwidths across the two blocks
	UT_sint32 lenNew = m_gbCharWidths.getLength() - blockOffset;
	if (lenNew > 0)
	{
		// NOTE: We do the length check on the outside for speed
		// TODO [1] can we move info from the current to the new
		// TODO CharWidths to keep from having to compute it in [2].
		pNewBL->m_gbCharWidths.ins(0, m_gbCharWidths, blockOffset, lenNew);
		m_gbCharWidths.truncate(blockOffset);
	}

	// Move remaining runs to new block
	pNewBL->m_pFirstRun = pFirstNewRun;

	// And update their positions
	for (pRun=pFirstNewRun; (pRun); pRun=pRun->getNext())
	{
		pRun->setBlockOffset(pRun->getBlockOffset() - blockOffset);
		pRun->setBlock(pNewBL);
		// TODO [2] the following 2 steps seem expensive considering
		// TODO we already knew width information before divided the
		// TODO char widths data between the two clocks.  see [1].
		pRun->fetchCharWidths(&pNewBL->m_gbCharWidths);
		pRun->recalcWidth();
	}

	// Explicitly truncate rest of this block's layout
	_truncateLayout(pFirstNewRun);

	// Now make sure this block still has an EOP Run.
	if (m_pFirstRun) {
		UT_ASSERT(pLastRun);
		// Create a new end-of-paragraph run and add it to the block.
		fp_EndOfParagraphRun* pNewRun = 
			new fp_EndOfParagraphRun(this, m_pLayout->getGraphics(), 0, 0);
		pLastRun->setNext(pNewRun);
		pNewRun->setPrev(pLastRun);
		pNewRun->setBlockOffset(pLastRun->getBlockOffset() 
								+ pLastRun->getLength());
		if(pLastRun->getLine())
			pLastRun->getLine()->addRun(pNewRun);
		coalesceRuns();
	}
	else
	{
		_insertEndOfParagraphRun();
	}
	setNeedsReformat();
	pNewBL->collapse(); // remove all previous lines
	// Throw all the runs onto one jumbo line in the new block
	pNewBL->_stuffAllRunsOnALine();
	if (pNewBL->m_pFirstRun)
		pNewBL->coalesceRuns();
	else
		pNewBL->_insertEndOfParagraphRun();
	pNewBL->setNeedsReformat();

	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
		pView->_setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
	else if(pView && pView->getPoint() > pcrx->getPosition())
		pView->_setPoint(pView->getPoint() + fl_BLOCK_STRUX_OFFSET);

#ifdef FASTSQUIGGLE
	if (m_pLayout->getAutoSpellCheck() && m_vecSquiggles.getItemCount() > 0)
	{
		// We have squiggles, so move them
		_breakSquiggles(blockOffset, pNewBL);
	}
	else
#endif
	{
		// This block may never have been checked just to be safe,
		// let's make sure both will
		UT_uint32 reason = 0;
		if( m_pLayout->getAutoSpellCheck())
			reason = (UT_uint32) FL_DocLayout::bgcrSpelling;
		m_pLayout->queueBlockForBackgroundCheck(reason, this);
		m_pLayout->queueBlockForBackgroundCheck(reason, pNewBL);
	}

	_assertRunListIntegrity();

	return true;
}

bool fl_BlockLayout::doclistener_insertSection(const PX_ChangeRecord_Strux * pcrx,
											   PL_StruxDocHandle sdh,
											   PL_ListenerId lid,
											   void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	   PL_ListenerId lid,
																	   PL_StruxFmtHandle sfhNew))
{
	_assertRunListIntegrity();

	// Insert a section at the location given in the change record.
	// Everything from this point forward (to the next section) needs
	// to be re-parented to this new section.  We also need to verify
	// that this insertion point is at the end of the block (and that
	// another block follows).  This is because a section cannot
	// contain content.

	UT_ASSERT(pcrx);
	UT_ASSERT(pfnBindHandles);
	UT_ASSERT(pcrx->getType() == PX_ChangeRecord::PXT_InsertStrux);
	UT_ASSERT(pcrx->getStruxType() == PTX_Section);

	UT_ASSERT(m_pSectionLayout->getType() == FL_SECTION_DOC);
	fl_DocSectionLayout* pDSL = (fl_DocSectionLayout*) m_pSectionLayout;
	
	fl_DocSectionLayout* pSL = new fl_DocSectionLayout(m_pLayout, sdh, pcrx->getIndexAP());

	if (!pSL)
	{
		UT_DEBUGMSG(("no memory for SectionLayout"));
		return false;
	}
	m_pLayout->insertSectionAfter(pDSL, pSL);
	
	// Must call the bind function to complete the exchange of handles
	// with the document (piece table) *** before *** anything tries
	// to call down into the document (like all of the view
	// listeners).

	PL_StruxFmtHandle sfhNew = (PL_StruxFmtHandle)pSL;
	pfnBindHandles(sdh,lid,sfhNew);

	fl_DocSectionLayout* pOldSL = pDSL;
//
// Now move all the blocks following into the new section
//
	fl_BlockLayout* pBL = getNext();
	while (pBL)
	{
		fl_BlockLayout* pNext = pBL->getNext();

		pBL->collapse();
		pOldSL->removeBlock(pBL);
		pSL->addBlock(pBL);
		pBL->m_pSectionLayout = pSL;
		pBL->m_bNeedsReformat = true;
		pBL = pNext;
	}
//
// Terminate blocklist here. This Block is the last in this section.
//
	setNext(NULL);
	pOldSL->setLastBlock( this);

	pOldSL->deleteEmptyColumns();

	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->_setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET + fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->_setPoint(pView->getPoint() + fl_BLOCK_STRUX_OFFSET + fl_BLOCK_STRUX_OFFSET);
	}

	_assertRunListIntegrity();

	return true;
}


bool fl_BlockLayout::doclistener_insertHdrFtrSection(const PX_ChangeRecord_Strux * pcrx,
											   PL_StruxDocHandle sdh,
											   PL_ListenerId lid,
											   void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	   PL_ListenerId lid,
																	   PL_StruxFmtHandle sfhNew))
{
	_assertRunListIntegrity();

	// Insert a Header/Footer section at the location given in the change record.
	// Everything from this point forward (to the next section) needs
	// to be re-parented to this new Header/Footer section.  We also need to 
    // verify
	// that this insertion point is at the end of the block (and that
	// another block follows).  This is because a section cannot
	// contain content.

	UT_ASSERT(pcrx);
	UT_ASSERT(pfnBindHandles);
	UT_ASSERT(pcrx->getType() == PX_ChangeRecord::PXT_InsertStrux);
	UT_ASSERT(pcrx->getStruxType() == PTX_SectionHdrFtr);

	UT_ASSERT(m_pSectionLayout->getType() == FL_SECTION_DOC);
	fl_DocSectionLayout* pDSL = (fl_DocSectionLayout*) m_pSectionLayout;
//
// Now we don't know at this point whether have this is a header or footer or even
// The DocLayout this HdrFtrSectionLayout is attached to! This will have to be
// set later since we can't pass props along with an insertStrux
//
	fl_HdrFtrSectionLayout* pHFSL = new fl_HdrFtrSectionLayout(FL_HDRFTR_HEADER,m_pLayout,NULL, sdh, pcrx->getIndexAP());

	if (!pHFSL)
	{
		UT_DEBUGMSG(("no memory for Header/Footer SectionLayout"));
		return false;
	}
	//
	// Add the hdrFtr section to the linked list of SectionLayouts
	//
	m_pLayout->addHdrFtrSection(pHFSL);

	
	// Must call the bind function to complete the exchange of handles
	// with the document (piece table) *** before *** anything tries
	// to call down into the document (like all of the view
	// listeners).

	PL_StruxFmtHandle sfhNew = (PL_StruxFmtHandle)pHFSL;
	pfnBindHandles(sdh,lid,sfhNew);

	fl_DocSectionLayout* pOldSL = pDSL;
//
// Now move all the blocks following into the new section
//
	fl_BlockLayout* pBL = getNext();
	while (pBL)
	{
		fl_BlockLayout* pNext = pBL->getNext();

		pBL->collapse();
		pOldSL->removeBlock(pBL);
		pHFSL->addBlock(pBL);
		pBL->m_pSectionLayout = (fl_SectionLayout *) pHFSL;
		pBL->m_bNeedsReformat = true;
		pBL = pNext;
	}
//
// Terminate blocklist here. This Block is the last in this section.
//
	setNext(NULL);
	pOldSL->setLastBlock( this);

	pOldSL->deleteEmptyColumns();

	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->_setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET + fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->_setPoint(pView->getPoint() + fl_BLOCK_STRUX_OFFSET + fl_BLOCK_STRUX_OFFSET);
	}

	_assertRunListIntegrity();

	return true;
}

void fl_BlockLayout::findSquigglesForRun(fp_Run* pRun)
{
	UT_uint32  runBlockOffset = pRun->getBlockOffset();
	UT_uint32  runLength = pRun->getLength();
	fl_PartOfBlock*	pPOB;

	/* For all misspelled words in this run, call the run->drawSquiggle() method */

	UT_uint32 iSquiggles = m_vecSquiggles.getItemCount();
	UT_uint32 i;
	for (i=0; i<iSquiggles; i++)
	{
		pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(i);

		if (
			!(pPOB->iOffset >= (runBlockOffset + runLength))
			&& !((pPOB->iOffset + pPOB->iLength) <= runBlockOffset)
			)
		{
			UT_uint32 iStart;
			UT_uint32 iLen;
			if (pPOB->iOffset <= runBlockOffset)
			{
				iStart = runBlockOffset;
			}
			else
			{
				iStart = pPOB->iOffset;
			}

			if ((pPOB->iOffset + pPOB->iLength) >= (runBlockOffset + runLength))
			{
				iLen = runLength + runBlockOffset - iStart;
			}
			else
			{
				iLen = pPOB->iOffset + pPOB->iLength - iStart;
			}

			UT_ASSERT(pRun->getType() == FPRUN_TEXT);
			if ( !pPOB->bIsIgnored )
				(static_cast<fp_TextRun*>(pRun))->drawSquiggle(iStart, iLen);
		}
	}
}

//////////////////////////////////////////////////////////////////
// Object-related stuff
//////////////////////////////////////////////////////////////////

bool fl_BlockLayout::doclistener_populateObject(PT_BlockOffset blockOffset,
												const PX_ChangeRecord_Object * pcro)
{
	_assertRunListIntegrity();

	switch (pcro->getObjectType())
	{
	case PTO_Image:
	{
		FG_Graphic* pFG = FG_Graphic::createFromChangeRecord(this, pcro);
		if (pFG == NULL)
			return false;
		
		UT_DEBUGMSG(("Populate:InsertObject:Image:\n"));
		_doInsertImageRun(blockOffset, pFG);

		delete pFG;
		return true;
	}
		
	case PTO_Field:
		UT_DEBUGMSG(("Populate:InsertObject:Field:\n"));
		_doInsertFieldRun(blockOffset, pcro);
		return true;
				
	default:
		UT_ASSERT(0);
		return false;
	}

	_assertRunListIntegrity();
}

bool fl_BlockLayout::doclistener_insertObject(const PX_ChangeRecord_Object * pcro)
{
	_assertRunListIntegrity();

	PT_BlockOffset blockOffset = 0;

	switch (pcro->getObjectType())
	{
	case PTO_Image:
	{
		UT_DEBUGMSG(("Edit:InsertObject:Image:\n"));
		blockOffset = pcro->getBlockOffset();

		FG_Graphic* pFG = FG_Graphic::createFromChangeRecord(this, pcro);
		if (pFG == NULL)
			return false;

		_doInsertImageRun(blockOffset, pFG);
		delete pFG;
		break;
	}
		
	case PTO_Field:
	{
		UT_DEBUGMSG(("Edit:InsertObject:Field:\n"));
		blockOffset = pcro->getBlockOffset();
		_doInsertFieldRun(blockOffset, pcro);
		break;
	}
	
	default:
		UT_ASSERT(0);
		return false;
	}
	
	setNeedsReformat();

	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
		pView->_setPoint(pcro->getPosition() + 1);
	else if(pView && pView->getPoint() > pcro->getPosition())
		pView->_setPoint(pView->getPoint() + 1);

	if (m_pLayout->getAutoSpellCheck())
		_insertSquiggles(blockOffset, 1);	// TODO: are objects always one wide?

	_assertRunListIntegrity();

	return true;
}

bool fl_BlockLayout::doclistener_deleteObject(const PX_ChangeRecord_Object * pcro)
{
	_assertRunListIntegrity();

	PT_BlockOffset blockOffset = 0;

	FV_View* ppView = m_pLayout->getView();
	if(ppView)
		ppView->eraseInsertionPoint();

	switch (pcro->getObjectType())
	{
	case PTO_Image:
	{
		UT_DEBUGMSG(("Edit:DeleteObject:Image:\n"));
		blockOffset = pcro->getBlockOffset();
		_delete(blockOffset, 1);
		break;
	}
	
	case PTO_Field:
	{
		UT_DEBUGMSG(("Edit:DeleteObject:Field:\n"));
		blockOffset = pcro->getBlockOffset();
		_delete(blockOffset, 1);
		if(m_pAutoNum)
		{
			m_pAutoNum->markAsDirty();
		}
		break;
	}		

	default:
		UT_ASSERT(0);
		return false;
	}
	
	setNeedsReformat();

	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->_resetSelection();
		pView->_setPoint(pcro->getPosition());
	}
	else if(pView && pView->getPoint() > pcro->getPosition())
		pView->_setPoint(pView->getPoint() - 1);

	if (m_pLayout->getAutoSpellCheck())
		_deleteSquiggles(blockOffset, 1);	// TODO: are objects always one wide?

	_assertRunListIntegrity();

	return true;
}

bool fl_BlockLayout::doclistener_changeObject(const PX_ChangeRecord_ObjectChange * pcroc)
{

	_assertRunListIntegrity();

	FV_View* pView = m_pLayout->getView();
	switch (pcroc->getObjectType())
	{
	case PTO_Image:
		UT_DEBUGMSG(("Edit:ChangeObject:Image:\n"));
		// TODO ... deal with image object ...
		return true;
		
	case PTO_Field:
	{
		UT_DEBUGMSG(("Edit:ChangeObject:Field:\n"));
		PT_BlockOffset blockOffset = pcroc->getBlockOffset();
		fp_Run* pRun = m_pFirstRun;
		while (pRun)
		{
			if (pRun->getBlockOffset() == blockOffset)
			{
				if(pRun->getType()!= FPRUN_FIELD)
				{
					UT_DEBUGMSG(("!!! run type NOT Field, instead = %d !!!! \n",pRun->getType()));
				}
				fp_FieldRun* pFieldRun = static_cast<fp_FieldRun*>(pRun);
				pView->_eraseInsertionPoint();
				pFieldRun->clearScreen();
				pFieldRun->lookupProperties();

				goto done;
			}
			pRun = pRun->getNext();
		}
	
		return false;
	}		

	default:
		UT_ASSERT(0);
		return false;
	}

 done:
	setNeedsReformat();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->_drawInsertionPoint();
	}

	_assertRunListIntegrity();

	return true;
}

bool fl_BlockLayout::recalculateFields(void)
{
	_assertRunListIntegrity();

	bool bResult = false;
	fp_Run* pRun = m_pFirstRun;
 	while (pRun)
	{
		if (pRun->getType() == FPRUN_FIELD)
		{
			fp_FieldRun* pFieldRun = static_cast<fp_FieldRun*>(pRun);

			const bool bSizeChanged = pFieldRun->calculateValue();

			bResult = bResult || bSizeChanged;
		}
		//       		else if(pRun->isField() == true)
		//	{
		//       bResult = pRun->getField()->update();
		//}
		pRun = pRun->getNext();
	}

	_assertRunListIntegrity();

	return bResult;
}

bool	fl_BlockLayout::findNextTabStop( UT_sint32 iStartX, UT_sint32 iMaxX, UT_sint32& iPosition,
										 eTabType & iType, eTabLeader &iLeader )
{
	UT_ASSERT(iStartX >= 0);

	UT_uint32 iCountTabs = m_vecTabs.getItemCount();
	UT_uint32 i;

	iLeader = FL_LEADER_NONE;
	
	for (i=0; i<iCountTabs; i++)
	{
		fl_TabStop* pTab = (fl_TabStop*) m_vecTabs.getNthItem(i);
		UT_ASSERT(pTab);

		if (pTab->getPosition() > iMaxX)
		{
			break;
		}
		
		if (pTab->getPosition() > iStartX)
		{
			iPosition = pTab->getPosition();
			iType = pTab->getType();
			iLeader = pTab->getLeader();

			return true;
		}
	}
	
	// now, handle the default tabs

	UT_sint32 iMin = m_iLeftMargin;

	if (iMin > iStartX)
	{
		iPosition = iMin;
		iType = FL_TAB_LEFT;
		return true;
	}
	
	UT_ASSERT(m_iDefaultTabInterval > 0);

#if 0
	// TMN: Original brute-force
	UT_sint32 iPos = 0;
	for (;;)
	{
		if (iPos > iStartX)
		{
			iPosition = iPos;
			iType = FL_TAB_LEFT;
			return true;
		}

		iPos += m_iDefaultTabInterval;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
#else
	// mathematical approach
	const UT_sint32 iPos = (iStartX / m_iDefaultTabInterval + 1) *
		m_iDefaultTabInterval;
	iPosition = iPos;
	iType = FL_TAB_LEFT;

	UT_ASSERT(iPos > iStartX);

	return true;
#endif

}

bool	fl_BlockLayout::findNextTabStopInLayoutUnits( UT_sint32 iStartX, UT_sint32 iMaxX, UT_sint32& iPosition,
													  eTabType& iType, eTabLeader &iLeader)
{
	UT_ASSERT(iStartX >= 0);

	UT_uint32 iCountTabs = m_vecTabs.getItemCount();
	UT_uint32 i;

	for (i=0; i<iCountTabs; i++)
	{
		fl_TabStop* pTab = (fl_TabStop*) m_vecTabs.getNthItem(i);

		if (pTab->getPositionLayoutUnits() > iMaxX)
		{
			break;
		}
		
		if (pTab->getPositionLayoutUnits() > iStartX)
		{
			iPosition = pTab->getPositionLayoutUnits();
			iType = pTab->getType();
			iLeader = pTab->getLeader();

			return true;
		}
	}

	// now, handle the default tabs

	UT_sint32 iMin = m_iLeftMarginLayoutUnits;

	if (iMin > iStartX)
	{
		iPosition = iMin;
		iType = FL_TAB_LEFT;
		return true;
	}

	UT_ASSERT(m_iDefaultTabIntervalLayoutUnits > 0);

#if 0
	// TMN: Original brute-force
	UT_sint32 iPos = 0;
	for (;;)
	{
		if (iPos > iStartX)
		{
			iPosition = iPos;
			iType = FL_TAB_LEFT;
			return true;
		}

		iPos += m_iDefaultTabIntervalLayoutUnits;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
#else
	// mathematical approach
	const UT_sint32 iPos = (iStartX / m_iDefaultTabIntervalLayoutUnits + 1) *
		m_iDefaultTabIntervalLayoutUnits;
	UT_ASSERT(iPos > iStartX);

	iPosition = iPos;
	iType = FL_TAB_LEFT;

	return true;
#endif

}

bool fl_BlockLayout::s_EnumTabStops( void * myThis, UT_uint32 k, fl_TabStop *pTabInfo)
{
	// a static function

	fl_BlockLayout * pBL = (fl_BlockLayout *)myThis;

	UT_uint32 iCountTabs = pBL->m_vecTabs.getItemCount();
	if (k >= iCountTabs)
		return false;

	fl_TabStop * pTab = (fl_TabStop *)pBL->m_vecTabs.getNthItem(k);

	*pTabInfo = *pTab;
	return true;
}


void fl_BlockLayout::setNext(fl_BlockLayout* pBL)
{
	m_pNext = pBL;
}

void fl_BlockLayout::setPrev(fl_BlockLayout* pBL)
{
	m_pPrev = pBL;
}

void fl_BlockLayout::setSectionLayout(fl_SectionLayout* pSectionLayout)
{
	//  If we are setting the new section layout, this block
	//  shouldn't already have a section.  If we are clearing
	//  it, then it should already have a section.
	if (pSectionLayout == NULL)
	{
		UT_ASSERT(m_pSectionLayout != NULL);
	}
	m_pSectionLayout = pSectionLayout;
}

//////////////////////////////////////////////////////////////////
// FmtMark-related stuff
//////////////////////////////////////////////////////////////////

bool fl_BlockLayout::doclistener_insertFmtMark(const PX_ChangeRecord_FmtMark * pcrfm)
{
	_assertRunListIntegrity();

	PT_BlockOffset blockOffset = pcrfm->getBlockOffset();

	xxx_UT_DEBUGMSG(("Edit:InsertFmtMark [blockOffset %ld]\n",blockOffset));

	fp_FmtMarkRun * pNewRun = new fp_FmtMarkRun(this,m_pLayout->getGraphics(),blockOffset);
	UT_ASSERT(pNewRun);
	_doInsertRun(pNewRun);
	
	// TODO is it necessary to force a reformat when inserting a FmtMark
	setNeedsReformat();

	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
		pView->_setPoint(pcrfm->getPosition());

	if (pView)
	{
		pView->_resetSelection();
		pView->notifyListeners(AV_CHG_FMTCHAR);
	}

#ifdef FASTSQUIGGLE
//	_insertSquiggles(blockOffset, 1);
#endif

	_assertRunListIntegrity();

	return true;
}


bool fl_BlockLayout::doclistener_deleteFmtMark(const PX_ChangeRecord_FmtMark * pcrfm)
{
	_assertRunListIntegrity();

	PT_BlockOffset blockOffset = pcrfm->getBlockOffset();

	xxx_UT_DEBUGMSG(("Edit:DeleteFmtMark: [blockOffset %ld]\n",blockOffset));

	// we can't use the regular _delete() since we are of length zero
	_deleteFmtMark(blockOffset);
	
	// TODO is it necessary to force a reformat when deleting a FmtMark
	setNeedsReformat();

	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->_resetSelection();
		pView->_setPoint(pcrfm->getPosition());
		pView->notifyListeners(AV_CHG_FMTCHAR);
	}

#ifdef FASTSQUIGGLE
//	_deleteSquiggles(blockOffset, 1);
#endif

	_assertRunListIntegrity();

	return true;
}

/*!
  Delete FmtMarkRun
  \param blockOffset Offset of Run to delete
  \return True

  Deleting a FmtMarkRun is a special version of _delete() since a
  FmtMarkRun has a length of zero.

  \fixme FmtMarkRun should not have a length of zero - jskov
*/
bool
fl_BlockLayout::_deleteFmtMark(PT_BlockOffset blockOffset)
{
	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		UT_uint32 iRunBlockOffset = pRun->getBlockOffset();

		// Remember where we're going, since this run may get axed
		fp_Run* pNextRun = pRun->getNext();

		if ( (iRunBlockOffset == blockOffset)
			 && (pRun->getType() == FPRUN_FMTMARK) )
		{
			fp_Line* pLine = pRun->getLine();
			UT_ASSERT(pLine);

			// Remove Run from line
			if(pLine)
			{
				pLine->removeRun(pRun);
			}

			// Unlink and delete it
			if (m_pFirstRun == pRun)
			{
				m_pFirstRun = pRun->getNext();
			}
			pRun->unlinkFromRunList();
			delete pRun;

			if (!m_pFirstRun)
			{
				// By the time we get to deleting anything from a block,
				// it should already have the necessary EOP in place.
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				_insertEndOfParagraphRun();
			}

			// I don't believe that we need to keep looping at this point.
			// We should not ever have two adjacent FmtMarks....
			UT_ASSERT(!pNextRun || pNextRun->getType() != FPRUN_FMTMARK);

			break;
		}

		pRun = pNextRun;
	}

	return true;
}

bool fl_BlockLayout::doclistener_changeFmtMark(const PX_ChangeRecord_FmtMarkChange * pcrfmc)
{
	_assertRunListIntegrity();

	PT_BlockOffset blockOffset = pcrfmc->getBlockOffset();
	
	xxx_UT_DEBUGMSG(("Edit:ChangeFmtMark: [blockOffset %ld]\n",blockOffset));

	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		if (pRun->getBlockOffset() == blockOffset)
		{
			UT_ASSERT(pRun->getType() == FPRUN_FMTMARK);
			pRun->lookupProperties();
			pRun->clearScreen();
			break;
		}

		pRun = pRun->getNext();
	}

	// We need a reformat for blocks that only contain a format mark.
	// ie. no next just a carrige return.
	setNeedsReformat();

	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->notifyListeners(AV_CHG_FMTCHAR);
	}

	_assertRunListIntegrity();

	return true;
}

void fl_BlockLayout::recheckIgnoredWords()
{
	fl_PartOfBlock*	pPOB;

	// for scanning a word	
	UT_uint32 wordBeginning, wordLength;
	bool bAllUpperCase;
	bool bHasNumeric;
	UT_uint32 eor;

	// buffer to hold text
	UT_GrowBuf pgb(1024);
	bool bRes = getBlockBuf(&pgb);
	UT_ASSERT(bRes);
	const UT_UCSChar* pBlockText = pgb.getPointer(0);

	PD_Document * pDoc = m_pLayout->getDocument();
	FV_View* pView = m_pLayout->getView();
	XAP_App * pApp = pView->getApp();
	bool bUpdate = false;

	/* For all misspelled words in this run, call the run->drawSquiggle() method */

	UT_uint32 iSquiggles = m_vecSquiggles.getItemCount();
	UT_uint32 i;
	for (i=0; i<iSquiggles; i++)
	{
		pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(i);
		
		wordBeginning = pPOB->iOffset;
		wordLength = 0;
		bAllUpperCase = false;
		bHasNumeric = false;
		eor = pPOB->iOffset + pPOB->iLength; /* end of region */

		while (!bAllUpperCase && ((wordBeginning + wordLength) < eor))
		{
			UT_ASSERT(!UT_isWordDelimiter( pBlockText[wordBeginning + wordLength], 'a'));

			if (bAllUpperCase)
				bAllUpperCase = UT_UCS_isupper(pBlockText[wordBeginning + wordLength]);

			if (!bHasNumeric)
				bHasNumeric = UT_UCS_isdigit(pBlockText[wordBeginning + wordLength]);

			wordLength++;
		}

		wordLength = pPOB->iLength;

		UT_UCSChar theWord[INPUTWORDLEN + 1];
		UT_uint32 newLength = wordLength;
		for (UT_uint32 ldex=0; ldex<wordLength; ++ldex)
		  {
		    UT_UCSChar currentChar;
		    currentChar = pBlockText[wordBeginning + ldex];
		    
		    // remove UCS_ABI_OBJECT from the word
		    if (currentChar == UCS_ABI_OBJECT) {
		      newLength--; continue;
		    }
		    
		    // convert smart quote apostrophe to ASCII single quote to be compatible with ispell
		    if (currentChar == UCS_RQUOTE) currentChar = '\'';
		    theWord[ldex - (wordLength - newLength)] = currentChar;
		  }

		// for some reason, the spell checker fails on all 1-char words & really big ones
		if ((wordLength > 1) &&
			XAP_EncodingManager::get_instance()->noncjk_letters(pBlockText+wordBeginning, newLength) &&		
			(!m_pLayout->getSpellCheckCaps() || !bAllUpperCase) &&		
			(!UT_UCS_isdigit(theWord[0])) &&			// still ignore first char==num words
			(!bHasNumeric || !m_pLayout->getSpellCheckNumbers()) &&		// can these two lines be simplified?
			(newLength < INPUTWORDLEN) &&

			(!_spellCheckWord(theWord, newLength, wordBeginning)) &&
			(!pApp->isWordInDict(theWord, newLength)))
		{
			// squiggle it
			pPOB->bIsIgnored = pDoc->isIgnore(theWord, newLength);
			_updateSquiggle(pPOB);
		}
		else
		{
			// remove
			m_vecSquiggles.deleteNthItem(i);
			i--;
			iSquiggles--;

			// forget about it
			delete pPOB;
		} // if valid word

		bUpdate = true;
	}
	if (bUpdate && pView)
	{
		pView->_eraseInsertionPoint();
		pView->updateScreen();
		pView->_drawInsertionPoint();
	}
}

////////////////////////////////////////////////////////////////////////////
//List Item Stuff
///////////////////////////////////////////////////////////////////////////

XML_Char* fl_BlockLayout::getListStyleString( List_Type iListType)
{

	XML_Char* style;

	// These strings match piece table styles and should not be
	// internationalized
	UT_uint32 nlisttype = (UT_uint32) iListType;
	if(nlisttype < 0 || nlisttype >= (UT_uint32) NOT_A_LIST)
		style = (XML_Char *) NULL;
	else
		style = const_cast<XML_Char *>(xml_Lists[nlisttype]);
	return style;
}

List_Type fl_BlockLayout::getListTypeFromStyle( const XML_Char* style)
{
	List_Type lType = NOT_A_LIST;
	UT_uint32 j;
	UT_uint32 size_xml_lists = sizeof(xml_Lists)/sizeof(xml_Lists[0]);
	for(j=0; j < size_xml_lists; j++)
	{
		if( UT_XML_strcmp(style,xml_Lists[j])==0)
			break;
	}
	if(j < size_xml_lists)
		lType = (List_Type) j;
	return lType;
}


char *  fl_BlockLayout::getFormatFromListType( List_Type iListType)
{
	UT_uint32 nlisttype = (UT_uint32) iListType;
	char * format = NULL;
	if(nlisttype < 0 || nlisttype >= (UT_uint32) NOT_A_LIST)
		return format;
	format = const_cast<char *>(fmt_Lists[nlisttype]);
	return format;
}

List_Type fl_BlockLayout::decodeListType(char * listformat)
{
	List_Type iType = NOT_A_LIST;
	UT_uint32 j;
	UT_uint32 size_fmt_lists = sizeof(fmt_Lists)/sizeof(fmt_Lists[0]);
	for(j=0; j < size_fmt_lists; j++)
	{
		if( strstr(listformat,fmt_Lists[j])!=NULL)
			break;
	}
	if(j < size_fmt_lists)
		iType = (List_Type) j;
	return iType;
}

List_Type fl_BlockLayout::getListType(void)
{
	if(isListItem()==false)
	{
		return NOT_A_LIST;
	}
	else
	{
		return getAutoNum()->getType();
	}
}

void fl_BlockLayout::remItemFromList(void)
{
	XML_Char lid[15], buf[5];
	UT_uint32 id;
	bool bRet;
	UT_Vector vp;
	if( m_bListLabelCreated == true)
	{
		m_bListLabelCreated = false;
		FV_View* pView = m_pLayout->getView();
		UT_ASSERT(pView);

		m_pDoc->beginUserAtomicGlob();

		UT_uint32 currLevel = getLevel();
		UT_ASSERT(currLevel > 0);
		currLevel =0; // was currLevel--;
		sprintf(buf, "%i", currLevel);
		setStopping(false);
		pView->_eraseInsertionPoint();
		fl_BlockLayout * pNext = getNext();
		if (currLevel == 0)
		{
			id = 0;
		}
		else
		{
			id = getAutoNum()->getParent()->getID();
			pNext = getPreviousList( id);
		}
		sprintf(lid, "%i", id);

		setStopping(false);
		pView->_eraseInsertionPoint();
		format();
		//
		// Set formatiing to match the next paragraph if it exists
		//
		const XML_Char ** props = NULL;
	
		if(pNext != NULL)
		{
			pNext->getListPropertyVector( &vp);
			UT_uint32 countp = vp.getItemCount() + 1;
			UT_uint32 i;
			props = (const XML_Char **) calloc(countp, sizeof(XML_Char *));
			for(i=0; i<vp.getItemCount();i++)
			{
				if( i > 0 &&
					UT_XML_strcmp(props[i-1],
								  "text-indent")==0)
				{
					props[i] = "0.0000in";
				}
				else
				{
					props[i] = (XML_Char *) vp.getNthItem(i);
				}
			}
			props[i] = (XML_Char *) NULL;
	
		}
		else
		{
			getListPropertyVector( &vp);
			UT_uint32 countp = vp.getItemCount() + 1;
			UT_uint32 i;
			props = (const XML_Char **) calloc(countp, sizeof(XML_Char *));
			for(i=0; i<vp.getItemCount();i++)
			{
				if( i > 0 &&
					UT_XML_strcmp(props[i-1],
								  "text-indent")==0)
				{
					props[i] = "0.0000in";
				}
				else
				{
					props[i] = (XML_Char *) vp.getNthItem(i);
				}
			}
			props[i] = (XML_Char *) NULL;
		}
		if (currLevel == 0)
		{
#ifndef __MRC__
			const XML_Char * attribs[] = { 	"listid", lid,
											"level", buf,"style","Normal", NULL, NULL };
#else
			const XML_Char * attribs[] = { 	"listid", NULL,
											"level", NULL,"style","Normal", NULL, NULL };
			attribs [1] = lid;
			attribs [3] = buf;
#endif
			bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), attribs, props, PTX_Block);
			m_bListItem = false;
		}
		else
		{
#ifndef __MRC__
			const XML_Char * attribs[] = { 	"listid", lid,
											"level", buf,NULL,NULL };
#else
			const XML_Char * attribs[] = { 	"listid", NULL,
											"level", NULL, NULL, NULL };
			attribs [1] = lid;
			attribs [3] = buf;
#endif
			bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,getPosition(), getPosition(), attribs, props, PTX_Block);
			m_pDoc->listUpdate(getStruxDocHandle());
		}
		//format();
		m_pDoc->endUserAtomicGlob();

		pView->AV_View::notifyListeners(AV_CHG_FMTBLOCK);
		pView->_fixInsertionPointCoords();
		//	pView->_generalUpdate();
		pView->_drawInsertionPoint();
		DELETEP(props);

	}
}

void    fl_BlockLayout::StartList( const XML_Char * style)
{
	//
	// Starts a new list at the current block with style style all other
	// attributes and properties are the default values
	//
	List_Type lType;
	PD_Style * pStyle;
	const XML_Char * szDelim,*szDec, * szStart, * szAlign, * szIndent;
	const XML_Char * szFont;
	UT_uint32 startv, level, currID;
	float fAlign, fIndent;
	
	m_pDoc->getStyle((const char *)style, &pStyle);
	if (pStyle)
	{
		// Use the props in the style
		pStyle->getProperty((const XML_Char *) "list-delim", szDelim);
		pStyle->getProperty((const XML_Char *) "list-decimal", szDec);
		pStyle->getProperty((const XML_Char *) "start-value", szStart);
		pStyle->getProperty((const XML_Char *) "margin-left", szAlign);
		pStyle->getProperty((const XML_Char *) "text-indent", szIndent);
		pStyle->getProperty((const XML_Char *) "field-font", szFont);
		if (szStart)
			startv = atoi(szStart);
		else
			startv = 1;
		if (szAlign)
			fAlign = (float)atof(szAlign);
		else
			fAlign = (float) LIST_DEFAULT_INDENT;
		if (szIndent)
			fIndent = (float)atof(szIndent);
		else
			fIndent =  (float)-LIST_DEFAULT_INDENT_LABEL;
		if(!szFont)
			UT_ASSERT(0);
	}
	else
	{
		szDelim = "%L";
		startv = 1;
		szDec = ".";
		fAlign =  (float) LIST_DEFAULT_INDENT;
		fIndent =  (float) -LIST_DEFAULT_INDENT_LABEL;
	}
	fAlign = (float) LIST_DEFAULT_INDENT;
	fIndent =  (float) -LIST_DEFAULT_INDENT_LABEL;
	if (m_pAutoNum)
	{
		level = m_pAutoNum->getLevel();
		currID = m_pAutoNum->getID();
	}
	else
	{
		level = 0;
		currID = 0;
	}
	level++;
	fAlign *= (float)level;
	
	lType = getListTypeFromStyle(style);
	StartList( lType, startv,szDelim, szDec, szFont, fAlign, fIndent, currID,level);
}

void    fl_BlockLayout::getListAttributesVector( UT_Vector * va)
{
	//
	// This function fills the vector va with list attributes
	//
	UT_uint32 count=0,level;
	const XML_Char * style = NULL;
	const XML_Char * lid = NULL;
	static XML_Char  buf[5];

	const PP_AttrProp * pBlockAP = NULL;
	getAttrProp(&pBlockAP);
	pBlockAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME,style);
	pBlockAP->getAttribute((const XML_Char *)"listid",lid);
	level = getAutoNum()->getLevel();
	sprintf(buf,"%i",level);
	//	pBlockAP->getAttribute("level",buf);
	if(lid != NULL)
	{
		va->addItem( (void *) "listid");  va->addItem( (void *) lid);
		count++;
	}
	if(buf != NULL)
	{
		va->addItem( (void *) "level");	va->addItem( (void *) buf);
		count++;
	}
	if(style != NULL)
	{
		va->addItem( (void *) "style");	va->addItem( (void *) style);
		count++;
	}
	if(count == 0)
	{
		va->addItem( NULL);
	}
}


void    fl_BlockLayout::getListPropertyVector( UT_Vector * vp)
{
	//
	// This function fills the vector vp with list properties. All vector
	// quantities are const XML_Char *
	//
	UT_uint32 count=0;
	const XML_Char * pszStart = getProperty("start-value",true);
	const XML_Char * lDelim =  getProperty("list-delim",true);
	const XML_Char * lDecimal =  getProperty("list-decimal",true);
	const XML_Char * pszAlign =  getProperty("margin-left",true);
	const XML_Char * pszIndent =  getProperty("text-indent",true);
	const XML_Char * fFont =  getProperty("field-font",true);
	if(pszStart != NULL)
	{
		vp->addItem( (void *) "start-value");	vp->addItem( (void *) pszStart);
	}
	if(pszAlign != NULL)
	{
		vp->addItem( (void *) "margin-left");	vp->addItem( (void *) pszAlign);
		count++;
	}
	if(pszIndent != NULL)
	{
		vp->addItem( (void *) "text-indent");	vp->addItem( (void *) pszIndent);
		count++;
	}
	if(lDelim != NULL)
	{
		vp->addItem( (void *) "list-delim"); vp->addItem( (void *) lDelim);
		count++;
	}
	if(lDecimal != NULL)
	{
		vp->addItem( (void *) "list-decimal"); vp->addItem( (void *) lDecimal);
		count++;
	}
	if(fFont != NULL)
	{
		vp->addItem( (void *) "field-font"); vp->addItem( (void *) fFont);
		count++;
	}
	if(count == 0)
	{
		vp->addItem( NULL);
	}
}


void    fl_BlockLayout::StartList( List_Type lType, UT_uint32 start,const XML_Char * lDelim, const XML_Char * lDecimal, const XML_Char * fFont, float Align, float indent, UT_uint32 iParentID, UT_uint32 curlevel )
{
	//
	// Starts a new list at the current block with all the options
	//
	XML_Char lid[15], pszAlign[20], pszIndent[20],buf[20],pid[20],pszStart[20];
	XML_Char * style = getListStyleString(lType);
	bool bRet;
	UT_uint32 id=0;
	UT_Vector vp,va;

	fl_AutoNum * pAutoNum;
	const PP_AttrProp * pBlockAP = NULL;
	const XML_Char * szLid=NULL;
	getAttrProp(&pBlockAP);
	if (!pBlockAP || !pBlockAP->getAttribute(PT_LISTID_ATTRIBUTE_NAME, szLid))
		szLid = NULL;
	if (szLid)
		id = atoi(szLid);
	else
		id = 0;
	
	FV_View* pView = m_pLayout->getView();
	UT_ASSERT(pView);
	pView->_eraseInsertionPoint();

	pAutoNum = m_pDoc->getListByID(id);
	if(pAutoNum != NULL)
	{
		m_pAutoNum = pAutoNum;
		m_bListItem = true;
		listUpdate();
	}
	id = rand();
	while(id < AUTO_LIST_RESERVED)
		id = rand();
	sprintf(lid, "%i", id);

	sprintf(pid, "%i", iParentID);
	sprintf(buf, "%i", curlevel);
	sprintf(pszStart,"%i",start);

	UT_XML_strncpy(	pszAlign,
					sizeof(pszAlign),
					UT_convertInchesToDimensionString(DIM_IN, Align, 0));

	UT_XML_strncpy(	pszIndent,
					sizeof(pszIndent),
					UT_convertInchesToDimensionString(DIM_IN, indent, 0));
	
	va.addItem( (void *) "listid");			va.addItem( (void *) lid);
	va.addItem( (void *) "parentid");		va.addItem( (void *) pid);
	va.addItem( (void *) "level");			va.addItem( (void *) buf);
	vp.addItem( (void *) "start-value");	vp.addItem( (void *) pszStart);
	vp.addItem( (void *) "margin-left");	vp.addItem( (void *) pszAlign);
	vp.addItem( (void *) "text-indent");	vp.addItem( (void *) pszIndent);
	vp.addItem( (void *) "field-font");     vp.addItem( (void *) fFont);
	va.addItem( (void *) "style");	va.addItem( (void *) style);


	pAutoNum = new fl_AutoNum(id, iParentID, lType, start, lDelim, lDecimal, m_pDoc);
	if (!pAutoNum)
	{
		// TODO Out of Mem.
	}
	m_pDoc->addList(pAutoNum);
	pAutoNum->fixHierarchy(m_pDoc);

	UT_uint32 counta = va.getItemCount() + 1;
	UT_uint32 countp = vp.getItemCount() + 1;
	UT_uint32 i;
	const XML_Char ** attribs = (const XML_Char **) calloc(counta, sizeof(XML_Char *));
	for(i=0; i<va.getItemCount();i++)
	{
		attribs[i] = (XML_Char *) va.getNthItem(i);
	}
	attribs[i] = (XML_Char *) NULL;

	const XML_Char ** props = (const XML_Char **) calloc(countp, sizeof(XML_Char *));
	for(i=0; i<vp.getItemCount();i++)
	{
		props[i] = (XML_Char *) vp.getNthItem(i);
	}
	props[i] = (XML_Char *) NULL;
	setStarting( false);
	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), attribs, props, PTX_Block);
	//pView->_ensureThatInsertionPointIsOnScreen();
	//pView->_eraseInsertionPoint();


	pView->_generalUpdate();
	m_pDoc->listUpdate(getStruxDocHandle());
	pView->_generalUpdate();
	pView->_ensureThatInsertionPointIsOnScreen();
	FREEP(attribs);
	FREEP(props);
}


void    fl_BlockLayout::StopList(void)
{
	//
	// Stops the list in the current block
	//
	static XML_Char lid[15],pszlevel[5];
	bool bRet;
	UT_uint32 id, level;
	UT_Vector vp;
	FV_View* pView = m_pLayout->getView();
	UT_ASSERT(pView);
	if(getAutoNum()== NULL)
	{
		return; // this block has already been processed
		pView->_generalUpdate();
	}

/*
	UT_uint32 currLevel = getLevel();

	UT_ASSERT(currLevel > 0);
	currLevel=0; // was currlevel--
	sprintf(buf, "%i", currLevel);*/
	PT_DocPosition offset = pView->getPoint() - getPosition();
	fl_BlockLayout * pPrev, * pNext;
/*	if (currLevel == 0)
	{
	id = 0;
	//      if(pNext != NULL && pNext->isListItem()!= true)
	//	{
	//        pNext = NULL;
	//	}
	}
	else
	{
	id = getAutoNum()->getParent()->getID();
	pNext = getPreviousList( id);
	}*/
	if (getAutoNum()->getParent())
	{
		id = getAutoNum()->getParent()->getID();
		level = getAutoNum()->getParent()->getLevel();
		// pNext = getPreviousList(id);
	}
	else
	{
		id = 0;
		level = 0;
	}
		
	sprintf(lid, "%i", id);

	setStopping(false);
	pView->_eraseInsertionPoint();
	//format();
	//
	// Set formatting to match the next paragraph if it exists
	//
	const XML_Char ** props = NULL;
	const XML_Char * szAlign, * szIndent;
	pPrev = getPrev();
	pNext = getNext();
	
	if (id != 0)
	{
		UT_ASSERT(pPrev);	// TMN: Is an assert appropriate here?

		//First, look for block in list
		bool bmatch = false;
		bmatch = (bool)(pPrev->isListItem() && pPrev->getLevel() == level && pPrev->getAutoNum()->getID() == id);
		while (pPrev && !bmatch)
		{
			pPrev = pPrev->getPrev();
			if (pPrev && pPrev->isListItem())
				bmatch = (bool)(pPrev->getLevel() == level
								&& pPrev->getAutoNum()->getID() == id);
		}
		while (pNext  && !bmatch)
		{
			pNext = pNext->getNext();
			if (pNext && pNext->isListItem())
				bmatch = (bool)(pNext->getLevel() == level
								&& pNext->getAutoNum()->getID() == id);
		}
		
		if (pPrev)
			pPrev->getListPropertyVector( &vp);
		else if (pNext)
			pNext->getListPropertyVector( &vp);
		else
		{
			// We have a problem
			List_Type newType;
			PD_Style * pStyle;
			float fAlign, fIndent;
			XML_Char align[30], indent[30];
			
			newType = getAutoNum()->getParent()->getType();
			m_pDoc->getStyle((char *)getListStyleString(newType), &pStyle);
			if (pStyle)
			{
				pStyle->getProperty((const XML_Char *)"margin-left", szAlign);
				pStyle->getProperty((const XML_Char *)"text-indent", szIndent);
				fAlign = (float)atof(szAlign);
				fAlign *= level;
				UT_XML_strncpy(	align,
								sizeof(align),
								UT_convertInchesToDimensionString(DIM_IN, fAlign, 0));
				sprintf(indent, "%s", szIndent);
			}
			else
			{
				fAlign =  (float)LIST_DEFAULT_INDENT * level;
				fIndent = (float)-LIST_DEFAULT_INDENT_LABEL;
				UT_XML_strncpy(	align,
								sizeof(align),
								UT_convertInchesToDimensionString(DIM_IN, fAlign, 0));
				UT_XML_strncpy(	indent,
								sizeof(indent),
								UT_convertInchesToDimensionString(DIM_IN, fIndent, 0));
			}
			
			vp.addItem((void *) "margin-left");
			vp.addItem((void *) align);
			vp.addItem((void *) "text-indent");
			vp.addItem((void *) indent);
		}
	}
	else
	{
		// Find the last non-list item and set alignment + indent
		while (pPrev && pPrev->isListItem())
			pPrev = pPrev->getPrev();
		
		while (pNext && pNext->isListItem())
			pNext = pNext->getNext();
			
		if (pPrev)
		{
			szAlign = pPrev->getProperty("margin-left", true);
			szIndent =  pPrev->getProperty("text-indent", true);
		}
		else if (pNext)
		{
			szAlign = pNext->getProperty("margin-left", true);
			szIndent = pNext->getProperty("text-indent", true);
		}
		else
		{
			szAlign = "0.0000in";
			szIndent = "0.0000in";
		}
		vp.addItem((void *) "margin-left"); vp.addItem((void *)szAlign);
		vp.addItem((void *) "text-indent"); vp.addItem((void *)szIndent);
	}
	UT_uint32 countp = vp.getItemCount() + 1;
	UT_uint32 i;
	props = (const XML_Char **) calloc(countp, sizeof(XML_Char *));
	for (i = 0; i < vp.getItemCount(); i++)
	{
		props[i] = (XML_Char *) vp.getNthItem(i);
	}
	props[i] = NULL;
	sprintf(pszlevel, "%i", level);

	if (id == 0)
	{
#ifndef _MRC_
		const XML_Char * attribs[] = { 	"listid", lid,
										"style","Normal", NULL, NULL };
#else
		const XML_Char * attribs[] = { 	"listid", NULL,
										"style","Normal", NULL, NULL };
		attribs [1] = lid;
#endif
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), attribs, props, PTX_Block);
		m_bListItem = false;
	}
	else
	{
#ifndef _MRC_
		const XML_Char * attribs[] = { 	"listid", lid,"level",pszlevel, NULL,NULL };
#else
		const XML_Char * attribs[] = { 	"listid", NULL,"level",NULL, NULL,NULL };
		attribs [1] = lid;
		attribs [3] = pszlevel;
#endif
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,getPosition(), getPosition(), attribs, props, PTX_Block);
		m_pDoc->listUpdate(getStruxDocHandle());
	}
	// format();
	if(offset > 0 )
		pView->_setPoint(pView->getPoint()+offset-2);

	pView->_generalUpdate();
	if (!pView->_ensureThatInsertionPointIsOnScreen())
	{
		pView->_fixInsertionPointCoords();
		pView->_drawInsertionPoint();
	}
	DELETEP(props);
}

/*!
 * Find the most recent block with the list ID given.
\param UT_uint32 id the identifier of the list
\returns fl_BlockLayout *
*/
fl_BlockLayout * fl_BlockLayout::getPreviousList(UT_uint32 id)
{
	//
	// Find the most recent list item that matches the id given
	//
  	UT_ASSERT(m_pAutoNum);
	fl_BlockLayout * pPrev = getPrev();
	bool bmatchid =  false;
	fl_AutoNum * pAutoNum = NULL;
	
	if (pPrev != NULL && pPrev->isListItem())
	{
		bmatchid = (bool) (id == pPrev->getAutoNum()->getID());
		if (pPrev->isFirstInList() && !bmatchid)
		{
			pAutoNum = pPrev->getAutoNum()->getParent();
			while (pAutoNum && !bmatchid)
			{
				bmatchid = (bool) (id == pAutoNum->getID()
								   && pAutoNum->isItem(pPrev->getStruxDocHandle()));
				pAutoNum = pAutoNum->getParent();
			}
		}
	}
	
	while (pPrev != NULL && bmatchid == false)
	{
		pPrev = pPrev->getPrev();
		if (pPrev && pPrev->isListItem())
		{
			bmatchid = (bool) (id == pPrev->getAutoNum()->getID());
			if (pPrev->isFirstInList() && !bmatchid)
			{
				pAutoNum = pPrev->getAutoNum()->getParent();
				while (pAutoNum && !bmatchid)
				{
					bmatchid = (bool)
						(id == pAutoNum->getID()
						 && pAutoNum->isItem(pPrev->getStruxDocHandle()));
					pAutoNum = pAutoNum->getParent();
				}
			}
		}
	}
	
	return pPrev;
}


fl_BlockLayout * fl_BlockLayout::getNextList(UT_uint32 id)
{
	//
	// Find the next list  item that matches the id given
	//
	fl_BlockLayout * pNext = getNext();
	bool bmatchLevel =  false;
	if( pNext != NULL && pNext->isListItem())
	{
		bmatchLevel = (bool) (id == pNext->getAutoNum()->getID());
	}
	while(pNext != NULL && bmatchLevel == false)
	{ 
		pNext = pNext->getNext() ;
		if( pNext != NULL && pNext->isListItem())
		{
			bmatchLevel = (bool) (id == pNext->getAutoNum()->getID());
		}
	}
	return pNext;
}

/*!
 * Find the most recent block with a list item.
\returns fl_BlockLayout *
*/
fl_BlockLayout * fl_BlockLayout::getPreviousList( void)
{
	//
	// Find the most recent block with a list
	//
	fl_BlockLayout * pPrev = getPrev();
	while(pPrev != NULL && !pPrev->isListItem())
	{
		pPrev = pPrev->getPrev() ;
	}
	return pPrev;
}

/*!
 * Returns the most recent Block containing a list item of the closest match
 * of left-margin to this one.
 \returns fl_BlockLayout *
*/
fl_BlockLayout * fl_BlockLayout::getPreviousListOfSameMargin(void)
{
	const char * szAlign = getProperty("margin-left",true);
	double dAlignMe = UT_convertToDimension(szAlign,DIM_IN); 
	//
	// Find the most recent block with a list
	//
	fl_BlockLayout * pClosest = NULL;
	float dClosest = 100000.;
	bool bFound = false;
	fl_BlockLayout * pPrev = getPrev();
	while(pPrev != NULL && !bFound)
	{
		if(pPrev->isListItem())
		{
			szAlign = pPrev->getProperty("margin-left",true);
			double dAlignThis = UT_convertToDimension(szAlign,DIM_IN);
			float diff = fabs( (float) dAlignThis-dAlignMe);
			if(diff < 0.01)
			{
				pClosest = pPrev;
				bFound = true;
			}
			else
			{
				if(diff < dClosest)
				{
					pClosest = pPrev;
					dClosest = diff;
				}
				pPrev = pPrev->getPrev();
			}
		}
		else
		{
			pPrev = pPrev->getPrev();
		}
	}
	return pClosest;
}

inline fl_BlockLayout * fl_BlockLayout::getParentItem(void)
{
	// TODO Again, more firendly.
	UT_ASSERT(m_pAutoNum);
	
	fl_AutoNum * pParent = m_pAutoNum->getActiveParent();
	if (pParent)
		return getPreviousList(pParent->getID());
	else
		return NULL;
}


void  fl_BlockLayout::prependList( fl_BlockLayout * nextList)
{
	//
	// Make the current block an element of the list before in the block nextList
	//
	UT_ASSERT(nextList);
	UT_Vector va,vp;
	
	nextList->getListPropertyVector( &vp);
	nextList->getListAttributesVector( &va);
	UT_uint32 counta = va.getItemCount() + 1;
	UT_uint32 countp = vp.getItemCount() + 1;
	UT_uint32 i;
	const XML_Char ** attribs = (const XML_Char **) calloc(counta, sizeof(XML_Char *));
	for(i=0; i<va.getItemCount();i++)
	{
		attribs[i] = (XML_Char *) va.getNthItem(i);
	}
	attribs[i] = (XML_Char *) NULL;

	const XML_Char ** props = (const XML_Char **) calloc(countp, sizeof(XML_Char *));
	for(i=0; i<vp.getItemCount();i++)
	{
		props[i] = (XML_Char *) vp.getNthItem(i);
	}
	props[i] = (XML_Char *) NULL;
	m_bStartList =  false;
	m_bStopList = false; 
	FV_View* pView = m_pLayout->getView();
	UT_ASSERT(pView);
	pView->_eraseInsertionPoint();
	m_bListLabelCreated = false;
	m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), attribs, props, PTX_Block);
	m_bListItem = true;
	m_pDoc->listUpdate(getStruxDocHandle());
	pView->_generalUpdate();
	DELETEP(attribs);
	DELETEP(props);
}

void  fl_BlockLayout::resumeList( fl_BlockLayout * prevList)
{
	//
	// Make the current block the next element of the list in the block prevList
	//
	UT_ASSERT(prevList);
	UT_Vector va,vp;
	
	prevList->getListPropertyVector( &vp);
	prevList->getListAttributesVector( &va);
	UT_uint32 counta = va.getItemCount() + 1;
	UT_uint32 countp = vp.getItemCount() + 1;
	UT_uint32 i;
	const XML_Char ** attribs = (const XML_Char **) calloc(counta, sizeof(XML_Char *));
	for(i=0; i<va.getItemCount();i++)
	{
		attribs[i] = (XML_Char *) va.getNthItem(i);
	}
	attribs[i] = (XML_Char *) NULL;

	const XML_Char ** props = (const XML_Char **) calloc(countp, sizeof(XML_Char *));
	for(i=0; i<vp.getItemCount();i++)
	{
		props[i] = (XML_Char *) vp.getNthItem(i);
	}
	props[i] = (XML_Char *) NULL;
	m_bStartList =  false;
	m_bStopList = false; 
	FV_View* pView = m_pLayout->getView();
	UT_ASSERT(pView);
	pView->_eraseInsertionPoint();
	m_bListLabelCreated = false;
	m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), attribs, props, PTX_Block);
	m_bListItem = true;
	m_pDoc->listUpdate(getStruxDocHandle());
	pView->_generalUpdate();
	DELETEP(attribs);
	DELETEP(props);
}

void fl_BlockLayout::listUpdate(void)
{
	//
	// Update the list on the screen to reflect changes made.
	//
	if(getSectionLayout() && (getSectionLayout()->getType()== FL_SECTION_HDRFTR))
	{
		m_pAutoNum = NULL;
		return;
	}
	if (m_pAutoNum == NULL)
		return;
	
	if (m_bStartList == true)
		m_pAutoNum->update(1);
	
	if ((m_bListLabelCreated == false) && (m_bStopList == false))
		_createListLabel();

	format();

	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		pView->_generalUpdate();
		pView->_fixInsertionPointCoords();
		pView->updateScreen();
		pView->drawInsertionPoint();
	}

}

void fl_BlockLayout::transferListFlags(void)
{
	//
	// Transfer list flags from a block to the following list blocks
	//
	UT_ASSERT(m_pNext);
	if (m_pNext->isListItem()) // this is wrong. It should be next in the list.
	{
		UT_uint32 nId = m_pNext->getAutoNum()->getID();
		UT_uint32 cId=0, pId=0;
		fl_BlockLayout * pPrev = getPreviousList();
		if(pPrev != NULL)
			pId = pPrev->getAutoNum()->getID();
		if(isListItem())
			cId = getAutoNum()->getID();
		if(cId == nId)
		{
			if (!m_pNext->m_bStartList)
				m_pNext->m_bStartList = m_bStartList;
			if (!m_pNext->m_bStopList)
				m_pNext->m_bStopList = m_bStopList;
		}
		else if ( pId == nId)
		{
			if (!m_pNext->m_bStartList)
				m_pNext->m_bStartList = pPrev->m_bStartList;
			if (!m_pNext->m_bStopList)
				m_pNext->m_bStopList = pPrev->m_bStopList;
		}
	}
}	

bool  fl_BlockLayout::isListLabelInBlock( void)
{
	fp_Run * pRun = m_pFirstRun;
	bool bListLabel = false;
	while( (pRun!= NULL) && (bListLabel == false))
	{
		if(pRun->getType() == FPRUN_FIELD)
		{
			fp_FieldRun* pFRun = static_cast<fp_FieldRun*>(pRun);
			if(pFRun->getFieldType() == FPFIELD_list_label)
				bListLabel = true;
		}
		pRun = pRun->getNext();
	}
	return bListLabel;
}

bool fl_BlockLayout::isFirstInList(void)
{
	PL_StruxDocHandle sdh = fl_Layout::getStruxDocHandle();
	if (!m_pAutoNum)
		return false;
	else
		return (bool) (sdh == m_pAutoNum->getFirstItem());
}

void fl_BlockLayout::_createListLabel(void)
{
	//
	// Put the current list label into this block.
	//
	if(!m_pFirstRun)
		return;
	if (isListLabelInBlock() == true  || m_bListLabelCreated == true)
	{
		m_bListLabelCreated = true;
		return;
	}
	UT_ASSERT(m_pAutoNum);

	FV_View* pView = m_pLayout->getView();
	const  XML_Char ** blockatt;
	PT_DocPosition offset = pView->getPoint() - getPosition();
	pView->getCharFormat(&blockatt,true);
	pView->setBlockFormat(blockatt);
	FREEP(blockatt);
	const XML_Char*	attributes[] = {
		"type","list_label",
		NULL, NULL
	};
	bool bResult = m_pDoc->insertObject(getPosition(), PTO_Field, attributes, NULL);
	// 	pView->_generalUpdate();
	if(m_pDoc->isDoingPaste() == false)
	{
		UT_UCSChar c = UCS_TAB;
		bResult = m_pDoc->insertSpan(getPosition()+1,&c,1);
	}
	pView->_setPoint(pView->getPoint()+offset);
	pView->_generalUpdate();

	if (!pView->_ensureThatInsertionPointIsOnScreen())
	{
		pView->_fixInsertionPointCoords();
		pView->_drawInsertionPoint();
	}
	m_bListLabelCreated = true;
}

void fl_BlockLayout::deleteListLabel(void)
{
	_deleteListLabel();
}


void fl_BlockLayout::_deleteListLabel(void)
{
	//
	// Remove the current list label from the block. This code does not assume the
	// label is at the first position in the block
	//
	PD_Document * pDoc = m_pLayout->getDocument();
	UT_uint32 posBlock = getPosition();
	// Find List Label
    fp_Run * pRun = getFirstRun();
	bool bStop = false;
	m_bListLabelCreated = false;
	//
	// Search within the block for the list label
	//
	while(bStop == false && pRun != NULL)
	{
		if(pRun->getType() == FPRUN_FIELD)
		{
			fp_FieldRun * pFRun = (fp_FieldRun *) pRun;
			if(pFRun->getFieldType() == FPFIELD_list_label)
			{
				bStop = true;
				break;
			}
		}
		pRun = pRun->getNext();
		if(pRun == NULL)
		{
			bStop = true;
		}
	}
	if(pRun != NULL)
	{
		UT_uint32 ioffset = pRun->getBlockOffset();
		UT_uint32 npos = 1;
		fp_Run * tRun = pRun->getNext();
		if(tRun != NULL && tRun->getType()==FPRUN_TAB)
		{
			npos = 2;
		}
		pDoc->deleteSpan(posBlock+ioffset, posBlock+ioffset + npos);
	}
}

UT_UCSChar * fl_BlockLayout::getListLabel(void)
{
	//	UT_ASSERT(m_pAutoNum);
	//
	// Return the calculated list label for the block
	//
	if(m_pAutoNum != NULL)
		return const_cast<UT_UCSChar *>(m_pAutoNum->getLabel(getStruxDocHandle()));
	else
		return NULL;
}

inline void fl_BlockLayout::_addBlockToPrevList( fl_BlockLayout * prevBlockInList, UT_uint32 level)
{
	//
	// Insert the current block to the list at the point after prevBlockInList
	//
	fl_AutoNum * pAutoNum;
	bool bMatchList = false;
	
	UT_ASSERT(prevBlockInList);
	
	pAutoNum = prevBlockInList->getAutoNum();
	while(pAutoNum && !bMatchList)
	{
		if (pAutoNum->getLevel() == level)
		{
			bMatchList = true;
			UT_DEBUGMSG(("Matched List. Returning.\n"));
		}
		else
		{
			pAutoNum = pAutoNum->getParent();
			UT_DEBUGMSG(("Didn't match list. Going Up.\n"));
		}
	}
	UT_DEBUGMSG(("Found List with Id: %d\n", pAutoNum->getID()));
	m_pAutoNum = pAutoNum;
	m_pAutoNum->insertItem(getStruxDocHandle(), prevBlockInList->getStruxDocHandle());
}


inline void fl_BlockLayout::_prependBlockToPrevList( fl_BlockLayout * nextBlockInList)
{
	//
	// Insert the current block to the list at the point before nextBlockInList
	//
	UT_ASSERT(nextBlockInList);
	m_pAutoNum = nextBlockInList->getAutoNum();
	m_pAutoNum->prependItem(getStruxDocHandle(), nextBlockInList->getStruxDocHandle());
}

UT_uint32 fl_BlockLayout::getLevel(void)
{
	if (!m_pAutoNum)
		return 0;
	else return m_pAutoNum->getLevel();
}

void fl_BlockLayout::setStarting( bool bValue )
{
	m_bStartList = bValue;
}

void fl_BlockLayout::setStopping( bool bValue)
{
	m_bStopList = bValue;
}

#ifdef BIDI_ENABLED
void fl_BlockLayout::setDominantDirection(bool bDirection)
{
	m_bDomDirection = bDirection;
	XML_Char * prop[] = {NULL, NULL, 0};
	XML_Char   ddir[] = "dom-dir";
	XML_Char   rtl[]  = "rtl";
	XML_Char   ltr[]  = "ltr";

	prop[0] = (XML_Char *) &ddir;
	
	if(m_bDomDirection)
	{
		prop[1] = (XML_Char *) &rtl;
	}
	else
	{
		prop[1] = (XML_Char *) &ltr;
	}

	PT_DocPosition offset = getPosition();
	PT_DocPosition offset2 = offset;
	//NB The casts in the following call are really necessary, it refuses to compile without them. #TF
	getDocument()->changeStruxFmt((PTChangeFmt)PTC_AddFmt,offset,offset2,(const XML_Char **) NULL,(const XML_Char **) prop,(PTStruxType)PTX_Block);
	UT_DEBUGMSG(("Block::setDominantDirection: offset=%d\n", offset));
}
#endif

void fl_BlockLayout::debugFlashing(void)
{
	// Trivial background checker which puts on and takes off squiggles from
	// the entire block that's being checked.  This sort of messes up the
	// spelling squiggles, but it's just a debug thing anyhow.  Enable it
	// by setting a preference DebugFlash="1"
	UT_DEBUGMSG(("fl_BlockLayout::debugFlashing() was called\n"));
	UT_GrowBuf pgb(1024);
	bool bRes = getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	UT_uint32 eor = pgb.getLength(); // end of region
	FV_View* pView = m_pLayout->getView();

	_addSquiggle(0, eor, false);

	pView->_eraseInsertionPoint();
	pView->updateScreen();
	pView->_drawInsertionPoint();
	UT_usleep(250000);

	//_deleteSquiggles(0, eor);

	pView->_eraseInsertionPoint();
	pView->updateScreen();
	pView->_drawInsertionPoint();

	return;
}
