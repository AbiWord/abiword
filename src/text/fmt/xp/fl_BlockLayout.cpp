/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * BIDI Copyright (c) 2001,2002 Tomas Frydrych
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
#include "fl_Squiggles.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fl_FootnoteLayout.h"
#include "fl_TableLayout.h"
#include "fl_AutoNum.h"
#include "fb_LineBreaker.h"
#include "fb_Alignment.h"
#include "fp_Column.h"
#include "fp_FootnoteContainer.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_TextRun.h"
#include "fp_FieldListLabelRun.h"
#include "fp_DirectionMarkerRun.h"
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
#include "ut_rand.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"

#include "xap_EncodingManager.h"

#if 1
// todo: work around to remove the INPUTWORDLEN restriction for pspell
#include "ispell_def.h"
#endif

bool
fl_BlockLayout::_spellCheckWord(const UT_UCSChar * word,
								UT_uint32 len, UT_uint32 blockPos)
{
	// the idea behind the static's here is to cache the dictionary, so
	// we do not have to do dictionary lookup all the time; rather, we
	// will cache the AP's and checker, and if the AP's have not
	// changed we will reuse the previous dictionary
	// this works because when the PT is asked to change formatting,
	// it will create a new AP with the new attr/props, rather than
	// add them to the existing AP for the section of the document, so
	// that identical AP's always imply identical formatting, and thus
	// language
	
	static SpellChecker * checker = NULL;
	
	// initialize these to 1, so as to force initial lang evaluation
	static const PP_AttrProp * pPrevSpanAP = reinterpret_cast<const PP_AttrProp *>(1);
	static const PP_AttrProp * pPrevBlockAP = reinterpret_cast<const PP_AttrProp *>(1);

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
		
	getSpanAttrProp(blockPos, false, &pSpanAP);
	getAttrProp(&pBlockAP);

	

	if(pSpanAP != pPrevSpanAP || pBlockAP != pPrevBlockAP)
	{
		const char * szLang = static_cast<const char *>(PP_evalProperty("lang",pSpanAP,pBlockAP,NULL,m_pDoc,true));

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

		pPrevSpanAP = pSpanAP;
		pPrevBlockAP = pBlockAP;
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
	: fl_ContainerLayout(pSectionLayout,sdh,indexAP,PTX_Block,FL_CONTAINER_BLOCK),
	  m_uBackgroundCheckReasons(0),
	  m_iNeedsReformat(0),
	  m_bNeedsRedraw(false),
	  m_bFixCharWidths(false),
	  m_bIsHdrFtr(bIsHdrFtr),
	  m_pBreaker(pBreaker),
	  m_pFirstRun(NULL),
	  m_pSectionLayout(pSectionLayout),
	  m_pAlignment(NULL),
	  m_bKeepTogether(false),
	  m_bKeepWithNext(false),
	  m_bStartList(false), m_bStopList(false),
	  m_bListLabelCreated(false),
	  m_bListItem(false),
	  m_szStyle(NULL),
	  m_bIsCollapsed(true),
	  m_iDomDirection(FRIBIDI_TYPE_UNSET),
	  m_iDirOverride(FRIBIDI_TYPE_UNSET)
{
	setPrev(pPrev);
	if(m_pSectionLayout && m_pSectionLayout->getType() == FL_SECTION_HDRFTR)
	{
		m_bIsHdrFtr = true;
	}
	m_pLayout = m_pSectionLayout->getDocLayout();
	m_pDoc = m_pLayout->getDocument();
	UT_ASSERT(m_pDoc);
	setAttrPropIndex(indexAP);

	const PP_AttrProp * pAP = 0;
	getAttrProp (&pAP);
    UT_ASSERT(pAP);

	if (!pAP->getAttribute (PT_STYLE_ATTRIBUTE_NAME, m_szStyle))
		{
			m_szStyle = NULL;
		}
	if (m_szStyle != NULL)
	{
		PD_Style * pStyle = NULL;
		m_pDoc->getStyle(static_cast<const char*>(m_szStyle), &pStyle);
		if(pStyle != NULL)
		{
			pStyle->used(1);
			if(pStyle->getBasedOn() != NULL)
			{
				pStyle->getBasedOn()->used(1);
			}
		}
	}

	_lookupProperties();

	if(!isHdrFtr() || (static_cast<fl_HdrFtrSectionLayout *>(getSectionLayout())->getDocSectionLayout() != NULL))
	{
		_insertEndOfParagraphRun();
	}

	m_pSquiggles = new fl_Squiggles(this);
	UT_ASSERT(m_pSquiggles);
	setUpdatableField(false);
	updateEnclosingBlockIfNeeded();
}

fl_TabStop::fl_TabStop()
{
	iPosition = 0;
	iType = FL_TAB_NONE;
	iLeader = FL_LEADER_NONE;
}

static int compare_tabs(const void* p1, const void* p2)
{
	const fl_TabStop * const * ppTab1 = reinterpret_cast<const fl_TabStop * const *>(p1);
	const fl_TabStop * const * ppTab2 = reinterpret_cast<const fl_TabStop * const *>(p2);

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
		fl_TabStop* pTab = static_cast<fl_TabStop*>(m_vecTabs.getNthItem(i));

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
				if ( p1 +2 != pEnd && p1[2] >= '0' && p1[2] <= ((static_cast<UT_sint32>(__FL_LEADER_MAX))+'0') )
					iLeader = static_cast<eTabLeader>(p1[2]-'0');
			}

			char pszPosition[32];
			UT_uint32 iPosLen = p1 - pStart;

			UT_ASSERT(iPosLen < sizeof pszPosition);

			memcpy(pszPosition, pStart, iPosLen);
			pszPosition[iPosLen] = 0;

			iPosition = UT_convertToLogicalUnits(pszPosition);

			UT_ASSERT(iType > 0);
			/*
			  The following assert is probably bogus, since tabs are
			  column-relative, rather than block-relative.
			*/
//			UT_ASSERT(iPosition >= 0);

			fl_TabStop* pTabStop = new fl_TabStop();
			pTabStop->setPosition(iPosition);
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

		fp_Line* pLine = static_cast<fp_Line *>(getLastContainer());
		if (pLine)
		{
			fp_Run* pRun = pLine->getLastRun();
			pRun->lookupProperties();
		}
	}

	const PP_AttrProp * pBlockAP = NULL;
	getAttrProp(&pBlockAP);

	if(!pBlockAP)
	{
		m_szStyle = NULL;
	}
	else if (!pBlockAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, m_szStyle))
	{
		m_szStyle = NULL;
	}

	// Now work out our dominant direction
	// First, test if this is not a block that is the wrapper around a
	// footnote text, if it is, we will get the direction from the
	// document section that contains the footnote
	const XML_Char * pszFntId = NULL;
	const XML_Char * pszDir = NULL;
		
	if (pBlockAP && pBlockAP->getAttribute("footnote-id", pszFntId ))
	{
		if(pszFntId && *pszFntId)
		{
			UT_ASSERT(m_pSectionLayout->getType() == PTX_SectionFootnote);
			fl_FootnoteLayout   * pFL = (fl_FootnoteLayout*) m_pSectionLayout;
			fl_DocSectionLayout * pDSL=	 pFL->getDocSectionLayout();

			const PP_AttrProp * pSectionAP = NULL;
			pDSL->getAttrProp(&pSectionAP);
				
			pszDir = PP_evalProperty("dom-dir",NULL,NULL,pSectionAP,m_pDoc,false);
		}
	}

	if(!pszDir)
	{
		pszDir = getProperty("dom-dir", true);
	}
		
	FriBidiCharType iOldDirection = m_iDomDirection;
		
	if(!UT_stricmp(pszDir,"rtl"))
	{
		m_iDomDirection = FRIBIDI_TYPE_RTL;
	}
	else
		m_iDomDirection = FRIBIDI_TYPE_LTR;

	// if the direction was previously set and the new dominant
	// direction is different, we have to split all runs in this
	// block at their direciton boundaries, because the base
	// direction influences the visual direciton of weak characters
	if(iOldDirection != FRIBIDI_TYPE_UNSET && iOldDirection != m_iDomDirection)
	{
		fp_Run * pRun = getFirstRun();

		while(pRun)
		{
			if (pRun->getType() == FPRUN_TEXT)
			{
				fp_TextRun * pTextRun = static_cast<fp_TextRun*>(pRun);

				//we get the next run in line prior to breaking this
				//one up, so that we do not break those already broken
				pRun = pRun->getNext();
				pTextRun->breakMeAtDirBoundaries(FRIBIDI_TYPE_IGNORE);
			}
			else
				pRun = pRun->getNext();
		}
	}
	{
		const PP_PropertyTypeInt *pOrphans = static_cast<const PP_PropertyTypeInt *>(getPropertyType("orphans", Property_type_int));
		UT_ASSERT(pOrphans);
		m_iOrphansProperty = pOrphans->getValue();

		const PP_PropertyTypeInt *pWidows = static_cast<const PP_PropertyTypeInt *>(getPropertyType("widows", Property_type_int));
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
		if (pszKeepTogether)
		{
			if (0 == UT_strcmp("yes", pszKeepTogether))
			{
				m_bKeepTogether = true;
			}
			else
			{
				m_bKeepTogether = false;
			}
		}


		const char* pszKeepWithNext = getProperty("keep-with-next");
		if (pszKeepWithNext)
		{
			if (0 == UT_strcmp("yes", pszKeepWithNext))
			{
				m_bKeepWithNext = true;
			}
			else
			{
				m_bKeepWithNext = false;
			}
		}
	}

	GR_Graphics* pG = m_pLayout->getGraphics();

	struct MarginAndIndent_t
	{
		const char* szProp;
		UT_sint32*	pVar;
	}
	const rgProps[] =
	{
		{ "margin-top", 	&m_iTopMargin    },
		{ "margin-bottom",	&m_iBottomMargin },
		{ "margin-left",	&m_iLeftMargin,  },
		{ "margin-right",	&m_iRightMargin, },
		{ "text-indent",	&m_iTextIndent,  }
	};
	for (UT_uint32 iRg = 0; iRg < NrElements(rgProps); ++iRg)
	{
		const MarginAndIndent_t& mai = rgProps[iRg];
		const PP_PropertyTypeSize * pProp = static_cast<const PP_PropertyTypeSize *>(getPropertyType(static_cast<const XML_Char*>(mai.szProp), Property_type_size));
		*mai.pVar	= UT_convertSizeToLayoutUnits(pProp->getValue(), pProp->getDim());
	}

	{
		const char* pszAlign = getProperty("text-align");

		// we will only delete and reallocate the alignment if it is different
		// than the current one
		//DELETEP(m_pAlignment);

		xxx_UT_DEBUGMSG(("block: _lookupProperties, text-align=%s, current %d\n", pszAlign, m_pAlignment?m_pAlignment->getType():0xffff));

		if (0 == UT_strcmp(pszAlign, "left"))
		{
			if(!m_pAlignment || m_pAlignment->getType() != FB_ALIGNMENT_LEFT)
			{
				DELETEP(m_pAlignment);
				m_pAlignment = new fb_Alignment_left;
			}
		}
		else if (0 == UT_strcmp(pszAlign, "center"))
		{
			if(!m_pAlignment || m_pAlignment->getType() != FB_ALIGNMENT_CENTER)
			{
				DELETEP(m_pAlignment);
				m_pAlignment = new fb_Alignment_center;
			}
		}
		else if (0 == UT_strcmp(pszAlign, "right"))
		{
			if(!m_pAlignment || m_pAlignment->getType() != FB_ALIGNMENT_RIGHT)
			{
				DELETEP(m_pAlignment);
				m_pAlignment = new fb_Alignment_right;
			}
		}
		else if (0 == UT_strcmp(pszAlign, "justify"))
		{
			if(!m_pAlignment || m_pAlignment->getType() != FB_ALIGNMENT_JUSTIFY)
			{
				DELETEP(m_pAlignment);
				m_pAlignment = new fb_Alignment_justify;
			}
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
				 UT_convertToLogicalUnits(getProperty("default-tab-interval")),
				 pG->getResolution(),
				 pG->getZoomPercentage()));
#endif

	const PP_PropertyTypeSize * pProp = static_cast<const PP_PropertyTypeSize *>(getPropertyType("default-tab-interval", Property_type_size));
	// TODO: this should probably change the stored property instead
	m_iDefaultTabInterval = UT_convertSizeToLayoutUnits(pProp->getValue(), pProp->getDim());
	if (!m_iDefaultTabInterval)
	{
		m_iDefaultTabInterval = UT_convertToLogicalUnits("1pt");
	}

	const char * pszSpacing = getProperty("line-height");

	// NOTE : Parsing spacing strings:
	// NOTE : - if spacing string ends with "+", it's marked as an "At Least" measurement
	// NOTE : - if spacing has a unit in it, it's an "Exact" measurement
	// NOTE : - if spacing is a unitless number, it's just a "Multiple"
    // 	UT_uint32 nLen = strlen(pszSpacing);
	// this assumed that only spacing 1 can be represented by a single charcter
	// but that is not very safe assumption, for there should be nothing stoping
	// us to use 2 or 3 in place of 2.0 or 3.0, so I commented this this out
	// Tomas 21/1/2002
	// if (nLen > 1)
	{
		const char * pPlusFound = strrchr(pszSpacing, '+');
		if (pPlusFound && *(pPlusFound + 1) == 0)
		{
			m_eSpacingPolicy = spacing_ATLEAST;

			// need to strip the plus first
			int posPlus = pPlusFound - pszSpacing;
			UT_ASSERT(posPlus>=0);
			UT_ASSERT(posPlus<100);

			UT_String pTmp(pszSpacing);
			pTmp[posPlus] = 0;

			m_dLineSpacing = UT_convertToLogicalUnits(pTmp.c_str());
		}
		else if (UT_hasDimensionComponent(pszSpacing))
		{
			m_eSpacingPolicy = spacing_EXACT;
			m_dLineSpacing = UT_convertToLogicalUnits(pszSpacing);

		}
		else
		{
			m_eSpacingPolicy = spacing_MULTIPLE;
			m_dLineSpacing =
				    UT_convertDimensionless(pszSpacing);
		}
	}

	//
	// No numbering in headers/footers
	//
	if(getSectionLayout() && (getSectionLayout()->getType()== FL_SECTION_HDRFTR))
	{
		return;
	}

	//const PP_AttrProp * pBlockAP = NULL;
	//getAttrProp(&pBlockAP);
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

	if ((m_pAutoNum) && (id) && (m_pAutoNum->getID() != id))
	{
		// We have stopped or started a multi-level list
		// this struxdochandle may already have been removed if there is another
		// view on this document. So check first

		if(m_pAutoNum->isItem(getStruxDocHandle()))
		{
		   m_pAutoNum->removeItem(getStruxDocHandle());
		}
		m_pAutoNum = NULL;
		UT_DEBUGMSG(("Started/Stopped Multi-Level\n"));
	}

	if (id == 0 && (m_pAutoNum))
	{
		// We have stopped a final list item.
		m_bStopList = true;
		m_pAutoNum->markAsDirty();
		if(m_pAutoNum->isItem(getStruxDocHandle()))
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
		UT_DEBUGMSG(("Adding to List, id= %d parent_id = %d \n",id,parent_id));

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
			style = getProperty("list-style",true);
			if(!style)
			{
				pBlockAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME,style);
			}
			UT_ASSERT(style);
			List_Type lType = getListTypeFromStyle( style);
			pAutoNum = new fl_AutoNum(id, parent_id, lType, start, lDelim, lDecimal, m_pDoc);
			UT_DEBUGMSG(("SEVIOR: Created new list id = %d\n",id));
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
				ppItem = static_cast<PL_StruxDocHandle>(NULL);
			}
			m_pAutoNum->insertFirstItem(pItem,ppItem,0);
			m_bStartList = true;
		}

		UT_DEBUGMSG(("Added Item to List\n"));
	}

	// Add this in for loading - see if better way to fix.
	// if (m_bListItem && !m_bListLabelCreated && m_pFirstRun)
	//	_createListLabel();

	// display property
	const char* pszDisplay = getProperty("display");
	if(pszDisplay && !UT_strcmp(pszDisplay, "none"))
	{
		setVisibility(FP_HIDDEN_TEXT);
	}
	else
	  {
	    setVisibility(FP_VISIBLE);
	  }

	// latter we will need to add here revision handling ...
}

fl_BlockLayout::~fl_BlockLayout()
{
	DELETEP(m_pSquiggles);
	purgeLayout();
	UT_VECTOR_PURGEALL(fl_TabStop *, m_vecTabs);
	DELETEP(m_pAlignment);
	//	if (m_pAutoNum)
//		{
//			m_pAutoNum->removeItem(getStruxDocHandle());
//			if (m_pAutoNum->isEmpty())
//				DELETEP(m_pAutoNum);
//		}

	UT_ASSERT(m_pLayout != NULL);
	m_pLayout->notifyBlockIsBeingDeleted(this);
	m_pDoc = NULL;
	m_pLayout = NULL;
	xxx_UT_DEBUGMSG(("~fl_BlockLayout: Deleting block %x \n",this));
}

/*!
 * This method returns true if the block is contained with a section embedded
 * in a block, like a footnote or a table or frame with text wrapping.
 */
bool fl_BlockLayout::isEmbeddedType(void)
{
	fl_ContainerLayout * pCL = myContainingLayout();
	if(pCL && (pCL->getContainerType() == FL_CONTAINER_FOOTNOTE || pCL->getContainerType() == FL_CONTAINER_ENDNOTE ) )
	{
		return true;
	}
	return false;
}

/*! 
 * This method scans through the list of runs from the first position listed
 * and updates the offsets. This is used following an operation on an embedded
 * type section (Like a footnote). Also updates the char widths and the POB's
 * in the squiggles.
\param posEmbedded the position of the embedded Section.
\param iEmbeddedSize the size of the embedded Section.
 */
void fl_BlockLayout::updateOffsets(PT_DocPosition posEmbedded, UT_uint32 iEmbeddedSize)
{
	UT_DEBUGMSG(("In update Offsets \n"));
	fp_Run * pRun = getFirstRun();
	PT_DocPosition posInBlock = getPosition();
	while(pRun && (posInBlock + pRun->getBlockOffset() < posEmbedded))
	{
		pRun = pRun->getNext();
	}
	if(pRun == NULL)
	{
		return;
	}
	fp_Run * pPrev = pRun->getPrev();
	UT_sint32 iDiff = 0;
	PT_DocPosition posRun = 0;
	if(pPrev == NULL)
	{
		iDiff = static_cast<UT_sint32>(pRun->getBlockOffset());
	}
	else
	{
		posRun = posInBlock + pPrev->getBlockOffset();
		if(posRun + pPrev->getLength() <= posEmbedded)
		{
			iDiff = static_cast<UT_sint32>((pRun->getBlockOffset() - pPrev->getBlockOffset() - pPrev->getLength()));
			xxx_UT_DEBUGMSG(("updateOffsets: after BlockOffset %d or pos %d \n",pRun->getBlockOffset(),posInBlock+pRun->getBlockOffset())); 
		}
		else if((posRun == posEmbedded) && pRun->getNext())
		{
			iDiff = static_cast<UT_sint32>((pRun->getNext()->getBlockOffset() -pRun->getBlockOffset() - pRun->getLength()));
			pRun = pRun->getNext();
		}
//
// here if the last run starts beyond the point where the footnote is embedded
// This means the previous run spans the embed point. 
// So split the previous run at the
// embed point and update the offsets of the following runs.
//
		else
		{
			pRun = pRun->getPrev();
			UT_ASSERT(pRun->getType() == FPRUN_TEXT);
			fp_TextRun * pTRun = static_cast<fp_TextRun *>(pRun);
			UT_uint32 splitOffset = posEmbedded - posInBlock;
			xxx_UT_DEBUGMSG(("updateOffsets: Split at offset %d \n",splitOffset));
			bool bres = pTRun->split(splitOffset);
			UT_ASSERT(bres);
			pRun = pTRun->getNext();
			pPrev = pTRun;
			iDiff = 0;
		}
	}
	UT_ASSERT(iDiff >= 0);
	xxx_UT_DEBUGMSG(("Updating block %x with orig shift %d new shift %d \n",this,iDiff,iEmbeddedSize));
	if(iDiff != static_cast<UT_sint32>(iEmbeddedSize))
	{
//
// First shift the charwidths to account for this change.
//
		iDiff = static_cast<UT_sint32>(iEmbeddedSize) - iDiff;
		if(iDiff < 0)
		{
			m_gbCharWidths.del(pRun->getBlockOffset()+iDiff,-iDiff);
		}
		else
		{
//
// Insert zeros for the widths inside the embedded space
//
			UT_GrowBufElement valz = 0;
			UT_sint32 i =0;
			for(i=0; i< iDiff; i++)
			{
				m_gbCharWidths.ins(pRun->getBlockOffset(),&valz,1);
			}
			xxx_UT_DEBUGMSG(("updateOffsets: inserted %d zeros in charwidths \n",iDiff));
		}
//
// Now shift all the offsets in the runs.
//
		UT_sint32 iFirstOffset = static_cast<UT_sint32>(pRun->getBlockOffset());
		while(pRun)
		{
			UT_uint32 iNew = pRun->getBlockOffset() + iDiff;
			UT_ASSERT(iNew >= 0);
			UT_DEBUGMSG(("Run %x Old offset %d New Offset %d \n",pRun,pRun->getBlockOffset(),iNew));
			pRun->setBlockOffset(static_cast<UT_uint32>(iNew));
			pRun = pRun->getNext();
		}
//
// Now update the PartOfBlocks in the squiggles
//
		getSquiggles()->updatePOBs(iFirstOffset,iDiff);
	}
#if 0
#if DEBUG
	pRun = getFirstRun();
	while(pRun)
	{
		if(pRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun * pTRun = static_cast<fp_TextRun *>(pRun);
			pTRun->printText();
		}
		pRun = pRun->getNext();
	}
#endif
#endif
	setNeedsReformat();
	updateEnclosingBlockIfNeeded();
}

/*!
 * This method updates the enclosing Block which contains the embedded Section
 * which in turn contains this Block. If this is not a block in an embedded
 * section type, we just return and do nothing.
 */	
void fl_BlockLayout::updateEnclosingBlockIfNeeded(void)
{
	if(!isEmbeddedType())
	{
		xxx_UT_DEBUGMSG(("Block %x is Not enclosed - returning \n"));
		return;
	}
	fl_ContainerLayout * pCL = myContainingLayout();
	UT_ASSERT((pCL->getContainerType() == FL_CONTAINER_FOOTNOTE) || (pCL->getContainerType() == FL_CONTAINER_ENDNOTE) );
	fl_EmbedLayout * pFL = static_cast<fl_EmbedLayout *>(pCL);
	if(!pFL->isEndFootnoteIn())
	{
		return;
	}
	PL_StruxDocHandle sdhStart = pCL->getStruxDocHandle();
	PL_StruxDocHandle sdhEnd = NULL;
	if(pCL->getContainerType() == FL_CONTAINER_FOOTNOTE)
	{
		getDocument()->getNextStruxOfType(sdhStart,PTX_EndFootnote, &sdhEnd);
	}
	else
	{
		getDocument()->getNextStruxOfType(sdhStart,PTX_EndEndnote, &sdhEnd);
	}

	UT_return_if_fail(sdhEnd != NULL);
	PT_DocPosition posStart = getDocument()->getStruxPosition(sdhStart);
	PT_DocPosition posEnd = getDocument()->getStruxPosition(sdhEnd);
	UT_uint32 iSize = posEnd - posStart + 1;
	fl_BlockLayout * pBL = m_pLayout->findBlockAtPosition(posStart-1);
	UT_ASSERT(iSize > 1);
	pBL->updateOffsets(posStart,iSize);
}


/*!
 * This method returns the DocSectionLayout that this block is associated with
 */
fl_DocSectionLayout * fl_BlockLayout::getDocSectionLayout(void) const
{
	fl_DocSectionLayout * pDSL = NULL;
	if(getSectionLayout()->getType() == FL_SECTION_DOC)
	{
		pDSL = static_cast<fl_DocSectionLayout *>( m_pSectionLayout);
		return pDSL;
	}
	else if	(getSectionLayout()->getType() == FL_SECTION_FOOTNOTE)
	{
		pDSL = static_cast<fl_FootnoteLayout *>(getSectionLayout())->getDocSectionLayout();
		return pDSL;
	}
	else if	(getSectionLayout()->getType() == FL_SECTION_ENDNOTE)
	{
		pDSL = static_cast<fl_EndnoteLayout *>(getSectionLayout())->getDocSectionLayout();
		return pDSL;
	}
	else if (getSectionLayout()->getType() == FL_SECTION_HDRFTR)
	{
		pDSL = static_cast<fl_HdrFtrSectionLayout *>(getSectionLayout())->getDocSectionLayout();
		return pDSL;
	}
	else if (getSectionLayout()->getType() == FL_SECTION_SHADOW)
	{
		pDSL = static_cast<fl_HdrFtrShadow *>( getSectionLayout())->getHdrFtrSectionLayout()->getDocSectionLayout();
		return pDSL;
	}
	else if (getSectionLayout()->getType() == FL_SECTION_CELL)
	{
		pDSL = static_cast<fl_ContainerLayout *>(getSectionLayout())->getDocSectionLayout();
		return pDSL;
	}
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}


fp_Line * fl_BlockLayout::findLineWithFootnotePID(UT_uint32 pid)
{
	fp_Line * pLine = static_cast<fp_Line *>(getFirstContainer());
	UT_Vector vecFoots;
	bool bFound = false;
	while(pLine && !bFound)
	{
		vecFoots.clear();
		if(pLine->getFootnoteContainers(&vecFoots))
		{
			UT_uint32 i = 0;
			for(i=0; i< vecFoots.getItemCount(); i++)
			{
				fp_FootnoteContainer * pFC = static_cast<fp_FootnoteContainer *>(vecFoots.getNthItem(i));
				fl_FootnoteLayout * pFL = static_cast<fl_FootnoteLayout *>(pFC->getSectionLayout());
				if(pFL->getFootnotePID() == pid)
				{
					bFound = true;
					break;
				}
			}
		}
		pLine = static_cast<fp_Line *>(pLine->getNext());
	}
	if(bFound)
	{
		return pLine;
	}
	return NULL;
}

UT_sint32 fl_BlockLayout::getMaxNonBreakableRun(void)
{
	fp_Run * pRun = getFirstRun();
	UT_sint32 iMax = 6; // this is the pixel width of a typical 12 point char
	if(pRun)
	{
#if 0
		if(pRun->getGraphics())
		{
			GR_Font *pFont = pRun->getGraphics()->getGUIFont();
			if(pFont)
			{
				iMax = pRun->getGraphics()->measureUnRemappedChar(static_cast<UT_UCSChar>('i'));
			}
		}
#endif
	}
	while(pRun)
	{
		if(pRun->getType() == FPRUN_IMAGE)
		{
			iMax = UT_MAX(iMax,pRun->getWidth());
		}
		pRun = pRun->getNext();
	}
	return iMax;
}

bool fl_BlockLayout::isHdrFtr(void)
{
	if(getSectionLayout()!= NULL)
		return (getSectionLayout()->getType() == FL_SECTION_HDRFTR);
	else
		return m_bIsHdrFtr;
}

void fl_BlockLayout::clearScreen(GR_Graphics* /* pG */)
{
	fp_Line* pLine = static_cast<fp_Line *>(getFirstContainer());
	if(isHdrFtr())
	{
		return;
	}
	while (pLine)
	{
		// I have commented this assert out, since due to the call from doclistener_deleteStrux
		// clearScreen can be called _after_ the contents of this paragraph have been cleared
		// Tomas 28/02/2002
		//UT_ASSERT(!pLine->isEmpty());
		if(!pLine->isEmpty())
			pLine->clearScreen();
		pLine = static_cast<fp_Line *>(pLine->getNext());
	}
}

void fl_BlockLayout::_mergeRuns(fp_Run* pFirstRunToMerge, fp_Run* pLastRunToMerge)
{
	UT_ASSERT(pFirstRunToMerge != pLastRunToMerge);
	UT_ASSERT(pFirstRunToMerge->getType() == FPRUN_TEXT);
	UT_ASSERT(pLastRunToMerge->getType() == FPRUN_TEXT);

	_assertRunListIntegrity();

	fp_TextRun* pFirst = static_cast<fp_TextRun*>(pFirstRunToMerge);

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
	xxx_UT_DEBUGMSG(("fl_BlockLayout::coalesceRuns\n"));
	fp_Line* pLine = static_cast<fp_Line *>(getFirstContainer());
	while (pLine)
	{
		pLine->coalesceRuns();
		pLine = static_cast<fp_Line *>(pLine->getNext());
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

	fp_Line* pLine = static_cast<fp_Line *>(getFirstContainer());
	while (pLine)
	{
		_removeLine(pLine);
		pLine = static_cast<fp_Line *>(getFirstContainer());
	}
	m_bIsCollapsed = true;
	m_iNeedsReformat = 0;
	UT_ASSERT(getFirstContainer() == NULL);
	UT_ASSERT(getLastContainer() == NULL);
}

void fl_BlockLayout::purgeLayout(void)
{
	fp_Line* pLine = static_cast<fp_Line *>(getFirstContainer());
	while (pLine)
	{
		_purgeLine(pLine);
		pLine = static_cast<fp_Line *>(getFirstContainer());
	}

	UT_ASSERT(getFirstContainer() == NULL);
	UT_ASSERT(getLastContainer() == NULL);

	while (m_pFirstRun)
	{
		fp_Run* pNext = m_pFirstRun->getNext();
		m_pFirstRun->setBlock(NULL);
		delete m_pFirstRun;
		m_pFirstRun = pNext;
	}
}

void fl_BlockLayout::_removeLine(fp_Line* pLine)
{

	if (getFirstContainer() == static_cast<fp_Container *>(pLine))
	{
		setFirstContainer(static_cast<fp_Container *>(getFirstContainer()->getNext()));

		// we have to call recalcMaxWidth so that the new line has the correct
		// x offset and width
		if(getFirstContainer())
			getFirstContainer()->recalcMaxWidth();
	}

	if (getLastContainer() == static_cast<fp_Container *>(pLine))
	{
		setLastContainer(static_cast<fp_Container *>(getLastContainer()->getPrev()));
		// we have to call recalcMaxWidth so that the new line has the correct
		// x offset and width
		if(getLastContainer())
			getLastContainer()->recalcMaxWidth();
	}

	pLine->setBlock(NULL);
	pLine->remove();

	delete pLine;
}

void fl_BlockLayout::_purgeLine(fp_Line* pLine)
{
	if (getLastContainer() == static_cast<fp_Container *>(pLine))
	{
		setLastContainer(static_cast<fp_Container *>(getLastContainer()->getPrev()));
	}

	if (getFirstContainer() == static_cast<fp_Container *>(pLine))
	{
		setFirstContainer(static_cast<fp_Container *>(getFirstContainer()->getNext()));
	}
	pLine->setBlock(NULL);
	pLine->remove();

	delete pLine;
}


void fl_BlockLayout::_removeAllEmptyLines(void)
{
	fp_Line* pLine;

	pLine = static_cast<fp_Line *>(getFirstContainer());
	while (pLine)
	{
		if (pLine->countRuns() == 0)
		{
			_removeLine(pLine);
			pLine = static_cast<fp_Line *>(getFirstContainer());
		}
		else
		{
			pLine = static_cast<fp_Line *>(pLine->getNext());
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
	// Remove runs from screen. No need for HdrFtr's though
	if(!isHdrFtr())
	{
		fp_Line * pLine = pTruncRun->getLine();
		if(pLine != NULL)
		{
			pLine->clearScreenFromRunToEnd(pTruncRun);
			pLine = static_cast<fp_Line *>(pLine->getNext());
			while(pLine)
			{
				pLine->clearScreen();
				pLine= static_cast<fp_Line *>(pLine->getNext());
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
	UT_ASSERT(getFirstContainer() == NULL);
	fp_Line* pLine = static_cast<fp_Line *>(getNewContainer());
	UT_ASSERT(pLine);
	if(pLine->getContainer() == NULL)
	{
		fp_VerticalContainer * pContainer = NULL;
		if(m_pSectionLayout->getFirstContainer())
		{
			// TODO assert something here about what's in that container
			pContainer = static_cast<fp_VerticalContainer *>(m_pSectionLayout->getFirstContainer());
		}
		else
		{
			pContainer = static_cast<fp_VerticalContainer *>(m_pSectionLayout->getNewContainer());
			UT_ASSERT(pContainer->getWidth() >0);
		}
		pContainer->insertContainer(static_cast<fp_Container *>(pLine));
	}
	fp_Run* pTempRun = m_pFirstRun;
	while (pTempRun)
	{
		pLine->addRun(pTempRun);
		pTempRun = pTempRun->getNext();
	}
	UT_ASSERT(pLine->getContainer());
	xxx_UT_DEBUGMSG(("fl_BlockLayout: Containing container for line is %x \n",pLine->getContainer()));
	pLine->recalcMaxWidth();
}

/*!
  Add end-of-paragraph Run to block

  This function adds the EOP Run to the block.	The presence of the
  EOP is an invariant (except for when merging / splitting blocks) and
  ensures that the cursor can always be placed on the last line of a
  block.  If there are multiple lines, the first N-1 lines will have a
  forced break of some kind which can also hold the cursor.
*/
void
fl_BlockLayout::_insertEndOfParagraphRun(void)
{
	UT_ASSERT(!m_pFirstRun);

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

	if (!getFirstContainer())
	{
		getNewContainer();
		m_bIsCollapsed = false;
	}
	fp_Line * pFirst = static_cast<fp_Line *>(getFirstContainer());
	UT_ASSERT(pFirst && pFirst->countRuns() == 0);

	pFirst->addRun(m_pFirstRun);

	// only do the line layout if this block is not hidden ...
 	FV_View * pView = getView();

	bool bShowHidden = pView && pView->getShowPara();
	FPVisibility eHidden = isHidden();
	bool bHidden = ((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
		              || eHidden == FP_HIDDEN_REVISION
		              || eHidden == FP_HIDDEN_REVISION_AND_TEXT);
	if(!bHidden)
		pFirst->layout();
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
	fp_Line * pFirstLine = static_cast<fp_Line *>(getFirstContainer());
	UT_ASSERT(pFirstLine && pFirstLine->countRuns() == 1);

	// Run list should be valid when called (but not at exit!)
	_assertRunListIntegrity();

	pFirstLine->removeRun(m_pFirstRun);
	delete m_pFirstRun;
	m_pFirstRun = NULL;

	pFirstLine->remove();
	delete pFirstLine;
	setFirstContainer(NULL);
	setLastContainer(NULL);
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
	if (getPrev() != NULL && getPrev()->getLastContainer() == NULL)
	{
		UT_DEBUGMSG(("In _breakLineAfterRun no LastLine \n"));
		UT_DEBUGMSG(("getPrev = %d this = %d \n", getPrev(), this));
		//UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	// Add a line for the Run if necessary
	if (getFirstContainer() == NULL)
		_stuffAllRunsOnALine();

	// Create the new line
	fp_Line* pNewLine = new fp_Line(getSectionLayout());
	UT_ASSERT(pNewLine);
	// Insert it after the current line
	fp_Line* pLine = pRun->getLine();
	pNewLine->setPrev(pLine);
	pNewLine->setNext(pLine->getNext());

	if(pLine->getNext())
		pLine->getNext()->setPrev(pNewLine);

	pLine->setNext(pNewLine);
	// Update LastContainer if necessary
	if (getLastContainer() == static_cast<fp_Container *>(pLine))
		setLastContainer(pNewLine);
	// Set the block
	pNewLine->setBlock(this);
	// Add the line to the container
	static_cast<fp_VerticalContainer *>(pLine->getContainer())->insertContainerAfter(static_cast<fp_Container *>(pNewLine),
																				     static_cast<fp_Container *>(pLine));

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
 * Calculate the height of the all the text contained by this block
 */
UT_sint32 fl_BlockLayout::getHeightOfBlock(void)
{
	UT_sint32 iHeight = 0;
	fp_Line * pCon = static_cast<fp_Line *>(getFirstContainer());
	while(pCon)
	{
		iHeight += pCon->getHeight();
		iHeight += pCon->getMarginBefore();
		iHeight += pCon->getMarginAfter();
		pCon = static_cast<fp_Line *>(pCon->getNext());
	}
	return iHeight;
}

/*!
  Format paragraph: split the content into lines which
  will fit in the container.  */
void fl_BlockLayout::format()
{
	// do not format if we are not on the screen
#if 0
	if(!isOnScreen())
	{
		return;
		UT_DEBUGMSG(("block 0x%08x not on screen\n",this));
	}
#endif
	//
	// If block hasn't changed don't format it.
	//
	if(m_iNeedsReformat == -1)
	{
		return;
	}
	//
	// Save the old height of the block. We compare to the new height after
	// the format.
	//
	UT_sint32 iOldHeight = getHeightOfBlock();
	//
	// Need this to find where to break section in the document.
	//
	fl_ContainerLayout * pPrevCL = getPrev();
	fp_Page * pPrevP = NULL;
	if(pPrevCL)
	{
		fp_Container * pPrevCon = pPrevCL->getFirstContainer();
		if(pPrevCon)
		{
			pPrevP = pPrevCon->getPage();
		}
	}
	xxx_UT_DEBUGMSG(("fl_BlockLayout - format \n"));
	_assertRunListIntegrity();
	fp_Run *pRunToStartAt = NULL;

	// TODO -- is this really needed?
	// is should not be, since _lookupProperties is explicitely called
	// by our listeners when the format changes
	// please do not uncomment this as a quick bugfix to some other
	// problem, and if you do uncomment it, please explain why - Tomas
	// _lookupProperties();
//
// Some fields like clock, character count etc need to be constantly updated
// This is best done in the background updater which examines every block
// in the document. To save scanning every run in the entire document we
// set a bool in blocks with these sort of fields.
//
	setUpdatableField(false);
	xxx_UT_DEBUGMSG(("formatBlock 3: pPage %x \n",pPrevP));
	if (m_pFirstRun)
	{
		if(m_iNeedsReformat > 0)
		{
			// only a part of this block need reformat, find the run that
			// contains this offset
			fp_Run * pR = m_pFirstRun;
			while(pR && (pR->getBlockOffset() + pR->getLength()) <= static_cast<UT_uint32>(m_iNeedsReformat))
				pR = pR->getNext();

			UT_ASSERT( pR );
			pRunToStartAt = pR;
		}
	

		// Recalculate widths of Runs if necessary.
		m_bFixCharWidths = true; // Kludge from sevior to fix layout bugs
		if (m_bFixCharWidths)
		{
			fp_Run* pRun = m_pFirstRun;
			bool bDoit = true; // was false. Same kludge from sevior.
			while (pRun)
			{
				if(pRun->getType() == FPRUN_FIELD)
				{
					fp_FieldRun * pFRun = static_cast<fp_FieldRun *>( pRun);
					if(pFRun->needsFrequentUpdates())
					{
						setUpdatableField(true);
					}
				}
				if(pRunToStartAt && pRun == pRunToStartAt)
					bDoit = true;

				if(bDoit && (pRun->getType() != FPRUN_ENDOFPARAGRAPH))
				{
					pRun->recalcWidth();
				}
				pRun = pRun->getNext();
			}
		}

		// Create the first line if necessary.
		if (!getFirstContainer())
		{
			_stuffAllRunsOnALine();
		}
		recalculateFields(0);

		// Reformat paragraph
		m_pBreaker->breakParagraph(this, NULL);

		// we have to do this in the breakParagraph rutine
		//_removeAllEmptyLines();
#if 1
		// the down-side of this is that on the active line we keep
		// spliting/merging if the editing position is not at either
		// end; the up-side is that at any given time our document
		// is represented by the minimal number of runs necessary,
		// which not only means that we use less memory, but more
		// importantly, we draw faster since any line with uniform
		// formatting is drawn by a single call to OS text drawing
		// routine
		coalesceRuns();
#endif
	}
	else
	{
		UT_DEBUGMSG(("NO block content. Insert an EOP \n"));
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
	m_iNeedsReformat = -1;

	_assertRunListIntegrity();
	m_bIsCollapsed = false;
	//
	// Only break section if the height of the block changes.
	//
	UT_sint32 iNewHeight = getHeightOfBlock();
	if(iOldHeight != iNewHeight)
	{	
		getDocSectionLayout()->setNeedsSectionBreak(true,pPrevP);
	}
	return;	// TODO return code
}

void fl_BlockLayout::markAllRunsDirty(void)
{
	fp_Run * pRun = m_pFirstRun;
	while(pRun)
	{
		pRun->markAsDirty();
		pRun = pRun->getNext();
	}
	fp_Line  * pLine = static_cast<fp_Line *>(getFirstContainer());
	while(pLine)
	{
		pLine->setNeedsRedraw();
		pLine = static_cast<fp_Line *>(pLine->getNext());
	}
}

void fl_BlockLayout::redrawUpdate()
{
//
// This can happen from the new deleteStrux code
//
	bool bFirstLineOn = false;
	bool bLineOff = false;
	

	// TODO -- is this really needed ??
	// we should not need to lookup properties on redraw,
	// _lookupProperties() gets explicitely called by our listeners
	// when format changes.
	// please do not uncomment this as a quick bugfix to some other
	// problem, and if you do uncomment it, please explain why - Tomas
	// _lookupProperties();
	
	if(isHdrFtr())
		return;
	fp_Line* pLine = static_cast<fp_Line *>(getFirstContainer());
	while (pLine)
	{
		if (pLine->needsRedraw())
		{
			bLineOff = pLine->redrawUpdate();
			bFirstLineOn |= bLineOff;
		}

		if(bFirstLineOn && !bLineOff)
		{
			// we are past all visible lines
			break;
		}
		
		pLine = static_cast<fp_Line *>(pLine->getNext());
	}

	m_bNeedsRedraw = false;

	//	_lookupProperties();
}

fp_Container* fl_BlockLayout::getNewContainer(fp_Container * /* pCon*/)
{
	fp_Line* pLine = new fp_Line(getSectionLayout());
	// TODO: Handle out-of-memory
	UT_ASSERT(pLine);
	fp_TableContainer * pPrevTable = NULL;
	pLine->setBlock(this);
	pLine->setNext(NULL);
	fp_VerticalContainer* pContainer = NULL;
	if (getLastContainer())
	{
		fp_Line* pOldLastLine = static_cast<fp_Line *>(getLastContainer());

		UT_ASSERT(getFirstContainer());
		UT_ASSERT(!getLastContainer()->getNext());

		pLine->setPrev(getLastContainer());
		getLastContainer()->setNext(pLine);
		setLastContainer(pLine);

		pContainer = static_cast<fp_VerticalContainer *>(pOldLastLine->getContainer());
		pContainer->insertContainerAfter(static_cast<fp_Container *>(pLine), static_cast<fp_Container *>(pOldLastLine));
	}
	else
	{
		UT_ASSERT(!getFirstContainer());
		setFirstContainer(pLine);
		setLastContainer(getFirstContainer());
		pLine->setPrev(NULL);
		
		fp_Line* pPrevLine = NULL;
		if(getPrev())
		{
			if(getPrev()->getLastContainer() == NULL)
			{
				// Previous block exists but doesn't have a last line.
				// This is a BUG. Try a work around for now. TODO Fix this elsewhere
				UT_DEBUGMSG(("BUG!!! Previous block exists with no last line. This should not happen \n"));
				//	getPrev()->format();
			}
		}
		if (getPrev() && getPrev()->getLastContainer())
		{
			fp_Container * pPrevCon = static_cast<fp_Container *>(getPrev()->getLastContainer());
			if(pPrevCon->getContainerType() == FP_CONTAINER_LINE)
			{
				pContainer = static_cast<fp_VerticalContainer *>(pPrevCon->getContainer());
				pPrevLine = static_cast<fp_Line *>(pPrevCon);
				UT_ASSERT(pContainer);
				UT_ASSERT(pContainer->getWidth() >0);
			}
			else
			{
				fp_Container * ppPrev = static_cast<fp_Container *>(pPrevCon);
				if(ppPrev && ((ppPrev->getContainerType() == FP_CONTAINER_ENDNOTE) || (ppPrev->getContainerType() == FP_CONTAINER_FOOTNOTE) ))
				{
					fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(ppPrev->getSectionLayout());
					while(pCL && (pCL->getContainerType() == FL_CONTAINER_FOOTNOTE) || (pCL->getContainerType() == FL_CONTAINER_ENDNOTE))
					{
						pCL = pCL->getPrev();
					}
					if(pCL)
					{
						ppPrev = pCL->getLastContainer();
					}
					else
					{
						ppPrev = NULL;
					}
				}
				if(ppPrev && (ppPrev->getContainerType() == FP_CONTAINER_LINE))
				{
					pPrevLine = static_cast<fp_Line *>(ppPrev);
					pContainer = static_cast<fp_VerticalContainer *>(pPrevLine->getContainer());
					UT_ASSERT(pContainer);
					UT_ASSERT(pContainer->getWidth() >0);
				}
				else if(ppPrev && (ppPrev->getContainerType() == FP_CONTAINER_TABLE))
				{
					pContainer = (fp_VerticalContainer *) ppPrev->getContainer();
					pPrevLine = NULL;
					pPrevTable = (fp_TableContainer*)ppPrev;
				}
				else
				{
					pPrevLine = NULL;
					pContainer = NULL;
				}
			}
		}
		else if (getNext() && getNext()->getFirstContainer())
		{
			pContainer = static_cast<fp_VerticalContainer *>(getNext()->getFirstContainer()->getContainer());
			UT_ASSERT(pContainer);
			UT_ASSERT(pContainer->getWidth() >0);
		}
		else if (m_pSectionLayout->getFirstContainer())
		{
			// TODO assert something here about what's in that container
			pContainer = static_cast<fp_VerticalContainer *>(m_pSectionLayout->getFirstContainer());
			UT_ASSERT(pContainer);
			UT_ASSERT(pContainer->getWidth() >0);
		}
		else
		{
			pContainer = static_cast<fp_VerticalContainer *>(m_pSectionLayout->getNewContainer());
			UT_ASSERT(pContainer);
			UT_ASSERT(pContainer->getWidth() >0);
		}
		if(pContainer == NULL)
		{
			pContainer = static_cast<fp_VerticalContainer *>(m_pSectionLayout->getNewContainer());
			UT_ASSERT(pContainer);
			UT_ASSERT(pContainer->getWidth() >0);
		}

		if ((pPrevLine==NULL) && (pPrevTable== NULL))
		{
			pContainer->insertContainer(static_cast<fp_Container *>(pLine));
		}
		else if((pPrevLine==NULL) &&(NULL!=pPrevTable))
		{
			pContainer->insertContainerAfter((fp_Container *)pLine, (fp_Container *) pPrevTable);
		}
		else
		{
			pContainer->insertContainerAfter(static_cast<fp_Container *>(pLine), static_cast<fp_Container *>(pPrevLine));
		}
	}
	UT_ASSERT(pLine->getContainer());
	return static_cast<fp_Container *>(pLine);
}

void fl_BlockLayout::setNeedsReformat(UT_uint32 offset)
{
	if(static_cast<UT_sint32>(offset) > m_iNeedsReformat)
		m_iNeedsReformat = offset;
	
	getSectionLayout()->setNeedsReformat();
}

void fl_BlockLayout::setNeedsRedraw(void)
{
	m_bNeedsRedraw = true;
	getSectionLayout()->setNeedsRedraw();
}

const char* fl_BlockLayout::getProperty(const XML_Char * pszName, bool bExpandStyles) const
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	getAttrProp(&pBlockAP);

	// at the moment this is only needed in the bidi build, where dom-dir property
	// can be inherited from the section; however, it the future this might need to
	// be added for the normal build too.
	m_pSectionLayout->getAttrProp(&pSectionAP);

	return PP_evalProperty(pszName,pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
}

const PP_PropertyType * fl_BlockLayout::getPropertyType(const XML_Char * pszName, tProperty_type Type, bool bExpandStyles) const
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

void fl_BlockLayout::getLineSpacing(double& dSpacing, eSpacingPolicy& eSpacing) const
{
	dSpacing = m_dLineSpacing;
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
  Compute insertion point (caret) coordinates and size
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
	if (!getFirstContainer() || !m_pFirstRun)
	{
		// when we have no formatting information, can't find anything
		return NULL;
	}

	// find the run which has this offset inside it.
	PT_DocPosition dPos = getPosition();
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
	while (pRun->getNext() && pRun->getBlockOffset() < iRelOffset)
	{
		pRun = pRun->getNext();
	}
	// Now scan farther if necessary - the block may contain Runs
	// with zero length. This is only a problem when empty Runs
	// appear for no good reason (i.e., an empty Run on an empty
	// line should be OK).
	while (pRun->getNext() && pRun->getBlockOffset() + pRun->getLength() < iRelOffset)
	{
		pRun = pRun->getNext();
	}

	// We may have scanned past the last Run in the block. Back up.
	if (!pRun)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		pRun = static_cast<fp_Line *>(getLastContainer())->getLastRun();
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
	if (!pRun){
		x = x2 = y = y2 = height = 0;
		return NULL;
	}

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
			if(getFirstRun()->getLine())
			{
				pPrevRun->findPointCoords(iRelOffset, x, y, x2, y2, height, bDirection);
			}
			else
			{
				height = 0;
			}
		}
		else
		{
			if(getFirstRun()->getLine())
			{
				pRun->findPointCoords(iRelOffset, x, y, x2, y2, height, bDirection);
			}
			else
			{
				height = 0;
			}
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
		if(getFirstRun()->getLine())
		{
			pRun->findPointCoords(iRelOffset, x, y, x2, y2, height, bDirection);
		}
		else
		{
			height = 0;
		}
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
		if(getFirstRun()->getLine())
		{
			pRun->findPointCoords(iRelOffset, x, y, x2, y2, height, bDirection);
		}
		else
		{
			height = 0;
		}
		return pRun;
	}


	// If the Runs are on the same line, assume pRun to be farther
	// right than pPrevRun.
	if (pPrevRun->getLine() == pRun->getLine())
	{
		if(getFirstContainer())
		{
			pRun->findPointCoords(iRelOffset, x, y, x2, y2, height, bDirection);
		}
		else
		{
			height = 0;
		}
		return pRun;
	}

	// Only case left is that of a soft-broken line.

	// Always return position _and_ Run of the previous line. Old
	// implementation returned pRun, but this will cause the
	// cursor to wander if End is pressed multiple times.
	if(getFirstRun()->getLine())
	{
		pPrevRun->findPointCoords(iRelOffset, x, y, x2, y2, height, bDirection);
	}
	else
	{
		height = 0;
	}
	return pPrevRun;
}

fp_Line* fl_BlockLayout::findPrevLineInDocument(fp_Line* pLine)
{
	if (pLine->getPrev())
	{
		return static_cast<fp_Line *>(pLine->getPrev());
	}
	else
	{
		if (getPrev())
		{
			return static_cast<fp_Line *>(getPrev()->getLastContainer());
		}
		else
		{
			fl_SectionLayout* pSL = static_cast<fl_SectionLayout *>(m_pSectionLayout->getPrev());

			if (!pSL)
			{
				// at EOD, so just bail
				return NULL;
			}

			fl_BlockLayout* pBlock = static_cast<fl_BlockLayout *>(pSL->getLastLayout());
			UT_ASSERT(pBlock);
			return static_cast<fp_Line *>(pBlock->getLastContainer());
		}
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

fp_Line* fl_BlockLayout::findNextLineInDocument(fp_Line* pLine)
{
	if (pLine->getNext())
	{
		return static_cast<fp_Line *>(pLine->getNext());
	}

	if (getNext())
	{
		// grab the first line from the next block
		return static_cast<fp_Line *>(getNext()->getFirstContainer());
	}
	else
	{
		// there is no next line in this section, try the next
		fl_SectionLayout* pSL = static_cast<fl_SectionLayout *>(m_pSectionLayout->getNext());

		if (!pSL)
		{
			// at EOD, so just bail
			return NULL;
		}

		fl_BlockLayout* pBlock = static_cast<fl_BlockLayout *>(pSL->getFirstLayout());
		UT_ASSERT(pBlock);
		return static_cast<fp_Line *>(pBlock->getFirstContainer());
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

fl_BlockLayout* fl_BlockLayout::getNextBlockInDocument(void) const
{
	fl_ContainerLayout * pNext = getNext();
	while(pNext)
	{
		if(pNext->getContainerType() == FL_CONTAINER_BLOCK)
		{
			return static_cast<fl_BlockLayout *>(pNext);
		}
		else if(pNext->getContainerType() == FL_CONTAINER_TABLE)
		{
			pNext = pNext->getFirstLayout();
		}
		else if(pNext->getContainerType() == FL_CONTAINER_CELL)
		{
			pNext = pNext->getFirstLayout();
		}
		else if(pNext->getContainerType() == FL_CONTAINER_FOOTNOTE)
		{
			pNext = pNext->getNext();
		}
		else
		{
			pNext = NULL;
		}
	}

	// keep going (check next section)
	fl_ContainerLayout * pCL = m_pSectionLayout->getNext();
	fl_BlockLayout* pBL = NULL;
	if(pCL && pCL->getContainerType() == FL_CONTAINER_BLOCK)
	{
		pBL = static_cast<fl_BlockLayout *>(pCL);
	}
	else if(pCL)
	{
		fl_SectionLayout* pSL = static_cast<fl_SectionLayout *>(pCL);
		pBL = static_cast<fl_BlockLayout *>(pSL->getFirstLayout());
		UT_ASSERT(pBL);
	}
	return pBL;
}

fl_BlockLayout* fl_BlockLayout::getPrevBlockInDocument(void) const
{
// FIXME: Handle other container types like get next block in doc
	if (getPrev())
		return static_cast<fl_BlockLayout *>(getPrev());

	// keep going (check prev section)
	fl_SectionLayout* pSL = static_cast<fl_SectionLayout *>(m_pSectionLayout->getPrev());
	fl_BlockLayout* pBL = NULL;

	if (pSL)
	{
		pBL = static_cast<fl_BlockLayout *>(pSL->getLastLayout());
		UT_ASSERT(pBL);
	}

	return pBL;
}

/*****************************************************************/
/*****************************************************************/

fl_PartOfBlock::fl_PartOfBlock(void)
{
	m_iOffset = 0;
	m_iLength = 0;
	m_bIsIgnored = false;
}

fl_PartOfBlock::fl_PartOfBlock(UT_sint32 iOffset, UT_sint32 iLength,
							   bool bIsIgnored /* = false */)
{
	m_iOffset = iOffset;
	m_iLength = iLength;
	m_bIsIgnored = bIsIgnored;
}

/*!
  Does POB touch region
  \param iOffset Offset of region
  \param iLength Length of region
  \return True if the region touches the POB
*/
bool
fl_PartOfBlock::doesTouch(UT_sint32 iOffset, UT_sint32 iLength) const
{
	UT_sint32 start1, end1, start2, end2;

	xxx_UT_DEBUGMSG(("fl_PartOfBlock::doesTouch(%d, %d)\n", iOffset, iLength));

	start1 = m_iOffset;
	end1 = m_iOffset + m_iLength;

	start2 = iOffset;
	end2 =	 iOffset + iLength;

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




/*!
 Recalculate boundries for pending word
 \param iOffset Offset of change
 \param chg Size of change, negative is removal, zero is for
			recalculating the pending word.

 On entry, the block is already changed and any pending word is junk.
 On exit, there's either a single unchecked pending word, or nothing.
*/
void
fl_BlockLayout::_recalcPendingWord(UT_uint32 iOffset, UT_sint32 chg)
{
	xxx_UT_DEBUGMSG(("fl_BlockLayout::_recalcPendingWord(%d, %d)\n",
					 iOffset, chg));

	UT_GrowBuf pgb(1024);
	bool bRes = getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar* pBlockText = reinterpret_cast<UT_UCSChar*>(pgb.getPointer(0));
	if (pBlockText == NULL)
	{
		return;
	}

	UT_uint32 iFirst = iOffset;
	UT_uint32 iAbs = static_cast<UT_uint32>((chg >= 0) ? chg : -chg);
	UT_sint32 iLen = ((chg > 0) ? iAbs : 0);

	// We expand this region outward until we get a word delimiter on
	// each side.

	// First, look towards the start of the buffer
	while ((iFirst > 1)
		   && !UT_isWordDelimiter(pBlockText[iFirst-1], pBlockText[iFirst] ,pBlockText[iFirst-2]))
	{
		iFirst--;
	}

	if(iFirst == 1 && !UT_isWordDelimiter(pBlockText[0], pBlockText[1], UCS_UNKPUNK))
	{
		iFirst--;
	}

	UT_ASSERT(iOffset>=iFirst);
	iLen += (iOffset-iFirst);

	// Then look towards the end of the buffer
	UT_uint32 iBlockSize = pgb.getLength();
	while ((iFirst + iLen < iBlockSize))
	{
		UT_UCSChar followChar, prevChar;
		followChar = ((iFirst + iLen + 1) < iBlockSize)  ?	pBlockText[iFirst + iLen + 1]  : UCS_UNKPUNK;
		prevChar = (iFirst == 0) ? UCS_UNKPUNK : pBlockText[iFirst + iLen - 1];

		if (UT_isWordDelimiter(pBlockText[iFirst + iLen], followChar, prevChar)) break;
		iLen++;
	}

	// Now we figure out what to do with this expanded span
	if (chg > 0)
	{
		// Insertion - look for any completed words by finding the
		// first word delimiter from the end.
		UT_uint32 iLast = iOffset + chg;
		UT_UCSChar followChar = UCS_UNKPUNK, currentChar, prevChar = iLast > 2 ? pBlockText[iLast - 2] : UCS_UNKPUNK;
		while (iLast > iFirst)
		{
			currentChar = pBlockText[--iLast];
			prevChar = iLast > 0 ? pBlockText[iLast - 1] : UCS_UNKPUNK;
			if (UT_isWordDelimiter(currentChar, followChar,prevChar)) break;
			followChar = currentChar;
		}

		if (iLast > (iFirst + 1))
		{
			// Delimiter was found in the block - that means
			// there is one or more words between iFirst
			// and iLast we want to check.
			_checkMultiWord(iFirst, iLast, false);
		}

		// We still have the word at the end pending though.
		iLen -= (iLast - iFirst);
		iFirst = iLast;
	}
	else
	{
		// Deletion or update - everything's already set up, so just
		// fall through
		UT_ASSERT(chg <= 0);
	}

	// Skip any word delimiters; handling the case where a word
	// is split by space - without this check, the space would
	// become part of the pending word.
	UT_uint32 eor = pgb.getLength();
	while (iLen > 0 && iFirst < eor)
	{
		UT_UCSChar currentChar = pBlockText[iFirst];
		UT_UCSChar followChar = (((iFirst + 1) < eor) ?
								 pBlockText[iFirst + + 1]  : UCS_UNKPUNK);
		UT_UCSChar prevChar = iFirst > 0 ? pBlockText[iFirst - 1] : UCS_UNKPUNK;

		if (!UT_isWordDelimiter(currentChar, followChar, prevChar)) break;
		iFirst++;
		iLen--;
	}

	// Is there a pending word left? If so, record the details.
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
			pPending->setOffset(iFirst);
			pPending->setLength(iLen);
			m_pLayout->setPendingWordForSpell(this, pPending);
		}
	}
	else
	{
		// No pending word any more
		m_pLayout->setPendingWordForSpell(NULL, NULL);
	}
}

/*****************************************************************/
/*****************************************************************/

/*!
 Check spelling of entire block

 Destructively recheck the entire block. Called from timer context, so
 we need to toggle IP.

 TODO - the IP toggling does not work very well, particularly just
 after a document was loaded. Long paragraphs do a slow blink of the
 IP and short paragraphs fast one virtually freezing the IP. The
 overall effect is rather erratic. I do not see, though, a good way of
 fixing this, particularly concering short blocks.
*/
void
fl_BlockLayout::checkSpelling(void)
{

	xxx_UT_DEBUGMSG(("fl_BlockLayout::checkSpelling: this 0x%08x isOnScreen(): %d\n", this,static_cast<UT_uint32>(isOnScreen())));
	// Don't spell check non-formatted blocks!
	if(m_pFirstRun == NULL || m_pFirstRun->getLine() == NULL)
		return;

	// only update screen if this block is on it
	bool bIsOnScreen = isOnScreen();

	// we only want to do the cursor magic if the cursor is in this block
	bool bIsCursorInBlock = false;
	FV_View* pView = getView();
	fp_Run* pLastRun = m_pFirstRun;

	while(pLastRun && pLastRun->getNext())
		pLastRun = pLastRun->getNext();
	
	
	if(pView && pLastRun)
	{
		UT_uint32 iBlPosStart = static_cast<UT_uint32>(getPosition());
		UT_uint32 iBlPosEnd   = iBlPosStart + pLastRun->getBlockOffset() + pLastRun->getLength();
		UT_uint32 iPos   = static_cast<UT_uint32>(pView->getPoint()); 

		bIsCursorInBlock = ((iPos >= iBlPosStart) && (iPos <= iBlPosEnd));
	}
	
	// Remove any existing squiggles from the screen...
	bool bUpdateScreen = m_pSquiggles->deleteAll();

	// Now start checking
	bUpdateScreen |= _checkMultiWord(0, -1, bIsCursorInBlock);

	// Finally update screen
	if (bIsOnScreen && bUpdateScreen && pView)
	{
		pView->updateScreen();
	}
}

/*!
 Spell-check region of block with potentially multiple words
 \param pBlockText Text of block
 \param iStart Start of region to check
 \param eor End of region to check (or -1 to check to the end)
 \param bToggleIP Toggle IP if true
*/
bool
fl_BlockLayout::_checkMultiWord(UT_sint32 iStart,
								UT_sint32 eor,
								bool bToggleIP)
{
	xxx_UT_DEBUGMSG(("fl_BlockLayout::_checkMultiWord\n"));

	bool bScreenUpdated = false;

	fl_BlockSpellIterator wordIterator(this, iStart);

	const UT_UCSChar* pWord;
	UT_sint32 iLength, iBlockPos;

	while (wordIterator.nextWordForSpellChecking(pWord, iLength, iBlockPos))
	{
		// When past the provided end position, break out
		if (eor > 0 && iBlockPos > eor) break;
		
		fl_PartOfBlock* pPOB = new fl_PartOfBlock(iBlockPos, iLength);
		UT_ASSERT(pPOB);

#if 0 // TODO: turn this code on someday
		FV_View* pView = getView();
		XAP_App * pApp = XAP_App::getApp();
		XAP_Prefs *pPrefs = pApp->getPrefs();
		UT_ASSERT(pPrefs);

		bool b;

		// possibly auto-replace the squiggled word with a suggestion
		if (pPrefs->getPrefsValueBool(static_cast<XML_Char*>(AP_PREF_KEY_SpellAutoReplace), &b))
		{
			if (b && !bIsIgnored)
			{
				// todo: better cursor movement
				pView->cmdContextSuggest(1, this, pPOB);
				pView->moveInsPtTo(FV_DOCPOS_EOW_MOVE);
				DELETEP(pPOB);
			}
		}
#endif

		if (pPOB)
		{
			bScreenUpdated |= _doCheckWord(pPOB, pWord, true, bToggleIP);
		}
	}

	return bScreenUpdated;
}

/*!
 Validate a word and spell-check it
 \param pPOB Block region to squiggle if appropriate
 \param pBlockText Pointer to block's text
 \param bAddSquiggle True if pPOB should be added to squiggle list
 \return True if display was updated, otherwise false

 If the word bounded by pPOB is not squiggled, the pPOB is deleted.
 */
bool
fl_BlockLayout::_doCheckWord(fl_PartOfBlock* pPOB,
							 const UT_UCSChar* pWord,
							 bool bAddSquiggle /* = true */,
							 bool bClearScreen /* = true */)
{

	UT_sint32 iLength = pPOB->getLength();
	UT_sint32 iBlockPos = pPOB->getOffset();

	do {
		// Spell check the word, return if correct
		//XAP_App * pApp = XAP_App::getApp();
		if (_spellCheckWord(pWord, iLength, iBlockPos))
			break;

		// Find out if the word is in the document's list of ignored
		// words
		PD_Document * pDoc = m_pLayout->getDocument();
		pPOB->setIsIgnored(pDoc->isIgnore(pWord, iLength));

		// Word not correct or recognized, so squiggle it
		if (bAddSquiggle)
		{
			m_pSquiggles->add(pPOB);
		}

		if(bClearScreen)
		{
			m_pSquiggles->clear(pPOB);
		}

		// Display was updated
		return true;

	} while (0);

	// Delete the POB which is not longer needed
	delete pPOB;
	return false;
}


/*!
 Spell-check word in the block region
 \param pPOB Block region bounding the word
 \return True if display was updated, false otherwise
 Consume word in pPOB -- either squiggle or delete it

 FIXME:jsk Make callers use fl_BlockSpellIterator so we don't have to
 check validity? Should just be provided the starting offset...
*/
bool
fl_BlockLayout::checkWord(fl_PartOfBlock* pPOB)
{
	xxx_UT_DEBUGMSG(("fl_BlockLayout::checkWord\n"));

	UT_ASSERT(pPOB);
	if (!pPOB)
		return false;

    // Just use the initial offset from the provided pPOB - the word's
    // exact location/length is not known (since what we're provided
    // is just the editing limits).
    fl_BlockSpellIterator wordIterator(this, pPOB->getOffset());

	const UT_UCSChar* pWord;
	UT_sint32 iLength, iBlockPos;

    // The word iterator may be unable to find a word within the
    // editing limits provided by the pPOB - so check that before
    // continuing.
    if (wordIterator.nextWordForSpellChecking(pWord,
                                              iLength, iBlockPos)
        && (iBlockPos+iLength <= pPOB->getOffset()+pPOB->getLength()))
    {
        delete pPOB;

        fl_PartOfBlock* pNewPOB = new fl_PartOfBlock(iBlockPos, iLength);
        UT_ASSERT(pNewPOB);
            
        return _doCheckWord(pNewPOB, pWord );
    }

	// Delete the POB which is not longer needed
	delete pPOB;
	return false;
}

/*****************************************************************/
/*****************************************************************/

bool fl_BlockLayout::doclistener_populateSpan(const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len)
{
	_assertRunListIntegrity();

	PT_BufIndex bi = pcrs->getBufIndex();
	if(getPrev()!= NULL && getPrev()->getLastContainer()==NULL)
	{
		xxx_UT_DEBUGMSG(("In fl_BlockLayout::doclistener_populateSpan  no LastLine \n"));
		xxx_UT_DEBUGMSG(("getPrev = %d this = %d \n",getPrev(),this));
		//			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	const UT_UCSChar* pChars = m_pDoc->getPointer(bi);
	xxx_UT_DEBUGMSG(("fl_BlockLayout:: populateSpan BlockOffset %d NO chars %d \n",blockOffset,len));
	/*
	  walk through the characters provided and find any
	  control characters.  Then, each control character gets
	  handled specially.  Normal characters get grouped into
	  runs as usual.
	*/
	UT_uint32	iNormalBase = 0;
	bool		bNormal = false;
	UT_uint32 i;
	for (i=0; i<len; i++)
	{
		xxx_UT_DEBUGMSG(("fl_BlockLayout: char %d %c \n",i,static_cast<char>(pChars[i])));
		switch (pChars[i])
		{
			// see similar control characters in fl_DocLayout.cpp
		case UCS_FF:	// form feed, forced page break
		case UCS_VTAB:	// vertical tab, forced column break
		case UCS_LF:	// newline
		case UCS_FIELDSTART: // zero length line to mark field start
		case UCS_FIELDEND: // zero length line to mark field end
		case UCS_BOOKMARKSTART:
		case UCS_BOOKMARKEND:
		case UCS_TAB:	// tab
		case UCS_LRO:	// explicit direction overrides
		case UCS_RLO:
		case UCS_PDF:
		case UCS_LRE:	
		case UCS_RLE:
		case UCS_LRM:
		case UCS_RLM:

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

			case UCS_BOOKMARKSTART:
			case UCS_BOOKMARKEND:
				_doInsertBookmarkRun(i + blockOffset);
				break;

			case UCS_TAB:
				_doInsertTabRun(i + blockOffset);
				break;

			case UCS_LRO:
			case UCS_RLO:
			case UCS_LRE:	
			case UCS_RLE:
			case UCS_PDF:
				// these should have been removed by
				// pd_Document::append/insert functions
				UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
				break;

			case UCS_LRM:
			case UCS_RLM:
				_doInsertDirectionMarkerRun(i + blockOffset,pChars[i]);
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
	updateEnclosingBlockIfNeeded();

	return true;
}

bool	fl_BlockLayout::_doInsertTextSpan(PT_BlockOffset blockOffset, UT_uint32 len)
{
	xxx_UT_DEBUGMSG(("_doInsertTextSpan: Initial offset %d, len %d\n", blockOffset, len));
	PT_BlockOffset curOffset = blockOffset;
	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan = 0;

	while(len > curOffset - blockOffset)
	{
		xxx_UT_DEBUGMSG(("fl_BlockLayout::_doInsertTextSpan: len %d curOffset %d blobkOffset %d \n",len,curOffset,blockOffset));
		
		FriBidiCharType iPrevType, iNextType, iLastStrongType = FRIBIDI_TYPE_UNSET, iType;
		getSpanPtr(static_cast<UT_uint32>(curOffset), &pSpan, &lenSpan);
		UT_ASSERT(pSpan);
		if(!pSpan)
			return false;

		iType = fribidi_get_type(static_cast<FriBidiChar>(pSpan[0]));

		UT_uint32 trueLen = UT_MIN(lenSpan,len - (curOffset - blockOffset));
		UT_uint32 i = 1;
#if 0
		UT_String sTmp;
		UT_uint32 j =0;
		for(j=0;j<trueLen;j++)
		{
			sTmp += pSpan[j];
		}
		UT_DEBUGMSG(("fl_BlockLayout::_doInsertTextSpan lenSpan %d truelen %d text |%s| \n",lenSpan,trueLen,sTmp.c_str()));
#endif

		//We need to do some bidi processing on the span; basically we
		//have to break the text into chunks that each will go into a
		//separate run in a manner that will ensure that the text will
		//be correctly processed later. The most obvious way is to
		//break every time we encounter a change of directional
		//properties. Unfortunately that means breaking at each white
		//space, which adds a huge amount of processing due to
		//allocating and deleting the runs when loading a
		//document. The following code tries to catch out the obvious
		//cases when the span can remain intact. Tomas, Jan 28, 2003
		for(i = 1; i < trueLen; i++)
		{
			iPrevType = iType;
			if(FRIBIDI_IS_STRONG(iType))
				iLastStrongType = iType;
			
			iType = fribidi_get_type(static_cast<FriBidiChar>(pSpan[i]));
			if(iType != iPrevType)
			{
				// potential direction boundary see if we can ignore
				// it
				bool bIgnore = false;
#if 0
				// this assumption is not true; for instance in the
				// sequence ") " the parenthesis and the space can
				// resolve to different directions
				// 
				// I am leaving it here so that I do not add it one
				// day again (Tomas, Apr 10, 2003)
				
				if(FRIBIDI_IS_NEUTRAL(iPrevType) && FRIBIDI_IS_NEUTRAL(iType))
				{
					// two neutral characters in a row will have the same
					// direction
					xxx_UT_DEBUGMSG(("fl_BlockLayout::_doInsertTextSpan: ntrl->ntrl (c=0x%04x)\n",
								 pSpan[i]));
					bIgnore = true;
				}
				else
#endif
				if(FRIBIDI_IS_STRONG(iPrevType) && FRIBIDI_IS_NEUTRAL(iType))
				{
					// we can ignore a neutral character following a
					// strong one if it is followed by a strong
					// character of identical type to the previous one
					xxx_UT_DEBUGMSG(("fl_BlockLayout::_doInsertTextSpan: strong->ntrl (c=0x%04x)\n",
								 pSpan[i]));
					
					// take a peek at what follows
					for(UT_uint32 j = i+1; j < trueLen; j++)
					{
						iNextType = fribidi_get_type(static_cast<FriBidiChar>(pSpan[j]));
						xxx_UT_DEBUGMSG(("fl_BlockLayout::_doInsertTextSpan: iNextType 0x%04x\n",
									      iNextType));
						if(iNextType == iPrevType)
						{
							bIgnore = true;
							break;
						}

						// if the next character is strong, we cannot
						// ignore the boundary
						if(FRIBIDI_IS_STRONG(iNextType))
							break;
					}
					
				}
				else if(FRIBIDI_IS_NEUTRAL(iPrevType) && FRIBIDI_IS_STRONG(iType))
				{
					// a neutral character followed by a strong one -- we
					// can ignore it, if the neutral character was
					// preceeded by a strong character of the same
					// type
					if(iType == iLastStrongType)
					{
						bIgnore = true;
					}
					xxx_UT_DEBUGMSG(("fl_BlockLayout::_doInsertTextSpan: ntrl->strong (c=0x%04x)\n",
								 pSpan[i]));
					
				}
				else
				{
					// in all other cases we will split
					xxx_UT_DEBUGMSG(("fl_BlockLayout::_doInsertTextSpan: other (c=0x%04x)\n",pSpan[i]));
				}

				xxx_UT_DEBUGMSG(("fl_BlockLayout::_doInsertTextSpan: bIgnore %d\n",static_cast<UT_uint32>(bIgnore)));
				if(!bIgnore)
					break;
			}
			
		}
		xxx_UT_DEBUGMSG(("_dopopulateTextSpan: text run: offset %d, len %d\n", curOffset, i));
		fp_TextRun* pNewRun = new fp_TextRun(this, m_pLayout->getGraphics(), curOffset, i);
		UT_ASSERT(pNewRun);
		UT_ASSERT(pNewRun->getType() == FPRUN_TEXT);
		pNewRun->setDirOverride(m_iDirOverride);
		curOffset += i;

		if(!_doInsertRun(pNewRun))
			return false;

	}

	return true;
}

bool	fl_BlockLayout::_doInsertForcedLineBreakRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_ForcedLineBreakRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun); // TODO check for outofmem

	bool bResult = _doInsertRun(pNewRun);
	if (bResult)
		_breakLineAfterRun(pNewRun);

	return bResult;
}

bool    fl_BlockLayout::_doInsertDirectionMarkerRun(PT_BlockOffset blockOffset, UT_UCS4Char iM)
{
	UT_DEBUGMSG(("fl_BlockLayout::_doInsertDirectionMarkerRun: offset %d, marker 0x%04x\n",
				 blockOffset, iM));
	
	fp_Run * pNewRun = new fp_DirectionMarkerRun(this, m_pLayout->getGraphics(), blockOffset, iM);
	UT_ASSERT( pNewRun );

	bool bResult = _doInsertRun(pNewRun);
	if (bResult)
		_breakLineAfterRun(pNewRun);

	return bResult;
}

#if 0
bool	fl_BlockLayout::_deleteBookmarkRun(PT_BlockOffset blockOffset)
{
	UT_DEBUGMSG(("fl_BlockLayout::_deleteBookmarkRun: blockOffset %d\n",blockOffset));
	_assertRunListIntegrity();

	fp_BookmarkRun *pB1;

	fp_Run* pRun = m_pFirstRun;

	/*
		we have to deal with FmtMarks, which are special case since they
		have width 0 and so can share block offset with our book mark
	*/
	while (pRun->getNext() && (pRun->getBlockOffset() != blockOffset || pRun->getType() == FPRUN_FMTMARK))
	{
		pRun = pRun->getNext();
	}

	UT_ASSERT(pRun && pRun->getType() == FPRUN_BOOKMARK);
	if(!pRun || pRun->getType() != FPRUN_BOOKMARK)
		return false;

	pB1 = static_cast<fp_BookmarkRun *>(pRun);

	// Remove Run from line
	fp_Line* pLine = pB1->getLine();
	UT_ASSERT(pLine);
	if(pLine)
	{
		pLine->removeRun(pB1, true);
	}
	// Unlink Run and delete it
	if (m_pFirstRun == pB1)
	{
		m_pFirstRun = pB1->getNext();
	}

	pRun = pB1->getNext();
	pB1->unlinkFromRunList();
	delete pB1;

	m_gbCharWidths.del(blockOffset, 1);

	fp_Run * pLastRun = static_cast<fp_Line *>(getLastContainer())->getLastRun();
	while(pRun )
	{
		pRun->setBlockOffset(pRun->getBlockOffset() - 1);
		if(pRun == pLastRun)
			break;
		pRun = pRun->getNext();
	}

	xxx_UT_DEBUGMSG(("fl_BlockLayout::_deleteBookmarkRun: assert integrity (1)\n"));
	_assertRunListIntegrity();

	return true;
}
#endif

bool	fl_BlockLayout::_doInsertBookmarkRun(PT_BlockOffset blockOffset)
{
	fp_Run * pNewRun =	new fp_BookmarkRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun);
	bool bResult = _doInsertRun(pNewRun);
	if (bResult)
	{
		_breakLineAfterRun(pNewRun);
	}

	return bResult;

}

bool	fl_BlockLayout::_doInsertHyperlinkRun(PT_BlockOffset blockOffset)
{
	fp_HyperlinkRun * pNewRun =  new fp_HyperlinkRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun);
	bool bResult = _doInsertRun(static_cast<fp_Run*>(pNewRun));
	if (bResult)
	{
		// if this is the start of the hyperlink, we need to mark all the runs
		// till the end of it
		// if this is because of an insert operation, the end run is already
		// in place, because we insert them in that order; if it is because of
		// append, ther is no end run, but then this is the last run; the other
		// runs will get marked as they get appended (inside fp_Run::insertRun...)
		// any hyperlink run will not get its m_pHyperlink set, so that
		// runs that follow it would not be marked

		if(pNewRun->isStartOfHyperlink())
		{
			fp_Run * pRun = pNewRun->getNext();
			UT_ASSERT(pRun);
			// when loading a document the opening hyperlink run is initially followed
			// by ENDOFPARAGRAPH run; we do not want to set this one
			while(pRun && pRun->getType() != FPRUN_HYPERLINK && pRun->getType() != FPRUN_ENDOFPARAGRAPH)
			{
				pRun->setHyperlink(pNewRun);
				pRun = pRun->getNext();
			}
		}

		_breakLineAfterRun(pNewRun);
	}

	return bResult;

}


bool	fl_BlockLayout::_doInsertFieldStartRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_FieldStartRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun); // TODO check for outofmem

	bool bResult = _doInsertRun(pNewRun);
	if (bResult)
		_breakLineAfterRun(pNewRun);

	return bResult;
}

bool	fl_BlockLayout::_doInsertFieldEndRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_FieldEndRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun); // TODO check for outofmem

	bool bResult = _doInsertRun(pNewRun);
	if (bResult)
		_breakLineAfterRun(pNewRun);

	return bResult;
}

bool	fl_BlockLayout::_doInsertForcedPageBreakRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_ForcedPageBreakRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun); // TODO check for outofmem
	if(getPrev()!= NULL && getPrev()->getLastContainer()==NULL)
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
	UT_ASSERT(pNewRun); // TODO check for outofmem

	bool bResult = _doInsertRun(pNewRun);
	if (bResult)
		_breakLineAfterRun(pNewRun);

	return bResult;
}

bool	fl_BlockLayout::_doInsertTabRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_TabRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun); // TODO check for outofmem

	return _doInsertRun(pNewRun);
}

bool	fl_BlockLayout::_doInsertImageRun(PT_BlockOffset blockOffset, FG_Graphic* pFG)
{
	fp_ImageRun* pNewRun = new fp_ImageRun(this, m_pLayout->getGraphics(), blockOffset, 1, pFG);
	UT_ASSERT(pNewRun); // TODO check for outofmem

	return _doInsertRun(pNewRun);
}

bool	fl_BlockLayout::_doInsertFieldRun(PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro)
{
	const PP_AttrProp * pSpanAP = NULL;

	getSpanAttrProp(blockOffset, false, &pSpanAP);
	UT_ASSERT(pSpanAP);

	// Get the field type.

	const XML_Char* pszType = NULL;
	pSpanAP->getAttribute("type", pszType);

	// Create the field run.

	fp_FieldRun* pNewRun;

	if (!pszType) 
		{
			UT_ASSERT (pszType); 	
			pNewRun = new fp_FieldRun(this, m_pLayout->getGraphics(), blockOffset, 1);
		}
	else if(UT_strcmp(pszType, "list_label") == 0)
	{
		pNewRun = new fp_FieldListLabelRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "footnote_ref") == 0)
	{
		pNewRun = new fp_FieldFootnoteRefRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "footnote_anchor") == 0)
	{
		pNewRun = new fp_FieldFootnoteAnchorRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "endnote_ref") == 0)
	{
		pNewRun = new fp_FieldEndnoteRefRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "endnote_anchor") == 0)
	{
		pNewRun = new fp_FieldEndnoteAnchorRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "time") == 0)
	{
		pNewRun = new fp_FieldTimeRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "page_number") == 0)
	{
		pNewRun = new fp_FieldPageNumberRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}
	else if(UT_strcmp(pszType, "page_ref") == 0)
	{
		pNewRun = new fp_FieldPageReferenceRun(this, m_pLayout->getGraphics(), blockOffset, 1);
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
	else if(UT_strcmp(pszType, "datetime_custom") == 0)
	{
		pNewRun = new fp_FieldDateTimeCustomRun(this, m_pLayout->getGraphics(), blockOffset, 1);
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
	else if(UT_strcmp(pszType, "mail_merge") == 0)
	  {
	    pNewRun = new fp_FieldMailMergeRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else if(UT_strcmp(pszType, "meta_title") == 0)
	  {
	    pNewRun = new fp_FieldMetaTitleRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else if(UT_strcmp(pszType, "meta_creator") == 0)
	  {
	    pNewRun = new fp_FieldMetaCreatorRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else if(UT_strcmp(pszType, "meta_subject") == 0)
	  {
	    pNewRun = new fp_FieldMetaSubjectRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else if(UT_strcmp(pszType, "meta_publisher") == 0)
	  {
	    pNewRun = new fp_FieldMetaPublisherRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else if(UT_strcmp(pszType, "meta_contributor") == 0)
	  {
	    pNewRun = new fp_FieldMetaContributorRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else if(UT_strcmp(pszType, "meta_date") == 0)
	  {
	    pNewRun = new fp_FieldMetaDateRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else if(UT_strcmp(pszType, "meta_type") == 0)
	  {
	    pNewRun = new fp_FieldMetaTypeRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else if(UT_strcmp(pszType, "meta_language") == 0)
	  {
	    pNewRun = new fp_FieldMetaLanguageRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else if(UT_strcmp(pszType, "meta_coverage") == 0)
	  {
	    pNewRun = new fp_FieldMetaCoverageRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else if(UT_strcmp(pszType, "meta_rights") == 0)
	  {
	    pNewRun = new fp_FieldMetaRightsRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else if(UT_strcmp(pszType, "meta_keywords") == 0)
	  {
	    pNewRun = new fp_FieldMetaKeywordsRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else if(UT_strcmp(pszType, "meta_description") == 0)
	  {
	    pNewRun = new fp_FieldMetaDescriptionRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	  }
	else
	{
		UT_ASSERT_NOT_REACHED ();
		//
		// New Piece Table Field Leave it for that code..
		//
		pNewRun = new fp_FieldRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	}

	UT_ASSERT(pNewRun); // TODO check for outofmem

	// TODO -- is this really needed ???
	// should not be, since we called lookupProperties in the
	// constructor - Tomas
	// pNewRun->lookupProperties();
	pNewRun->calculateValue();

	_doInsertRun(pNewRun);
	recalculateFields(0);
	return true;
}

bool	fl_BlockLayout::_doInsertRun(fp_Run* pNewRun)
{
	PT_BlockOffset blockOffset = pNewRun->getBlockOffset();
	UT_uint32 len = pNewRun->getLength();
	xxx_UT_DEBUGMSG(("_doInsertRun: New run has offset %d Length %d \n",blockOffset,len));
	_assertRunListIntegrity();

	m_gbCharWidths.ins(blockOffset, len);
	if (pNewRun->getType() == FPRUN_TEXT)
	{
		fp_TextRun* pNewTextRun = static_cast<fp_TextRun*>(pNewRun);

		pNewTextRun->fetchCharWidths(&m_gbCharWidths);
		pNewTextRun->recalcWidth();
	}

	bool bInserted = false;
	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		UT_uint32 iRunBlockOffset = pRun->getBlockOffset();
		UT_uint32 iRunLength = pRun->getLength();
		xxx_UT_DEBUGMSG(("_doInsertRun: Target offset %d CurRun Offset %d Length %d \n",blockOffset,iRunBlockOffset,iRunLength));
		if ( (iRunBlockOffset + iRunLength) <= blockOffset )
		{
			// nothing to do.  the insert occurred AFTER this run
		}
		else if ((iRunBlockOffset > blockOffset) && bInserted)
		{

			// the insert is occuring BEFORE this run, so we just move the run offset
				pRun->setBlockOffset(iRunBlockOffset + len);
		}
		else if((iRunBlockOffset > blockOffset) && !bInserted)
//
// Run should be inserted before this run
//
		{
			pRun->setBlockOffset(iRunBlockOffset + len);
			pRun->insertIntoRunListBeforeThis(*pNewRun);
			bInserted = true;
			if(pRun->getLine())
			{
				pRun->getLine()->insertRunBefore(pNewRun, pRun);
#if DEBUG
				pRun->getLine()->assertLineListIntegrity();
#endif
			}
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
			{
				pRun->getLine()->insertRunBefore(pNewRun, pRun);
#if DEBUG
				pRun->getLine()->assertLineListIntegrity();
#endif
			}
		}
//
// Here if the run run starts before the target offset and finishes after it.
// We need to split this run.
//
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
//			   is not used.

//			fp_TextRun* pOtherHalfOfSplitRun = static_cast<fp_TextRun*>(pTextRun->getNext());

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
			{
				pRun->getLine()->insertRunBefore(pNewRun, pRun);
#if DEBUG
				pRun->getLine()->assertLineListIntegrity();
#endif
			}

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
		if (pLastRun)
		{
			if((pNewRun->getType() !=FPRUN_ENDOFPARAGRAPH) && (pLastRun->getType()== FPRUN_ENDOFPARAGRAPH))
			{
				pLastRun->insertIntoRunListBeforeThis(*pNewRun);
				pLastRun->setBlockOffset(pNewRun->getBlockOffset()+pNewRun->getLength());
				if(pLastRun->getLine())
				{
					pLastRun->getLine()->insertRunBefore(pNewRun, pLastRun);
#if DEBUG
					pLastRun->getLine()->assertLineListIntegrity();
#endif
				}
			}
			else
			{
				pLastRun->insertIntoRunListAfterThis(*pNewRun);
				if (getLastContainer())
				{
					static_cast<fp_Line *>(getLastContainer())->addRun(pNewRun);
#if DEBUG
					static_cast<fp_Line *>(getLastContainer())->assertLineListIntegrity();
#endif
				}
			}
		}
		else
		{
			m_pFirstRun = pNewRun;
			if (getLastContainer())
			{
				static_cast<fp_Line *>(getLastContainer())->addRun(pNewRun);
#if DEBUG
				static_cast<fp_Line *>(getLastContainer())->assertLineListIntegrity();
#endif
			}
		}

	}

	/*
	  if we inserted a text run, and its direction is strong, then we might need to do
	  some more work. Since a strong run can change the visual direction of adjucent
	  weak characters, we need to ensure that any weak characters on either side
	  are in runs of their own.
	*/
	FriBidiCharType iDirection = pNewRun->getDirection();
	if(FRIBIDI_IS_STRONG(iDirection) && pNewRun->getType() == FPRUN_TEXT)
	{
		static_cast<fp_TextRun*>(pNewRun)->breakNeighborsAtDirBoundaries();
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
	UT_uint32	iNormalBase = 0;
	bool	bNormal = false;
	UT_uint32 i;
	UT_uint32 _sqlist[100], *sqlist = _sqlist;
	UT_uint32 sqcount = 0;
	//
	// Need this to find where to break section in the document.
	//
	fl_ContainerLayout * pPrevCL = getPrev();
	fp_Page * pPrevP = NULL;
	if(pPrevCL)
	{
		fp_Container * pPrevCon = pPrevCL->getFirstContainer();
		if(pPrevCon)
		{
			pPrevP = pPrevCon->getPage();
		}
	}

	if (sizeof(_sqlist) / sizeof(_sqlist[0])  < len)
	{
		sqlist = new UT_uint32[len];
	}
	UT_DEBUGMSG(("fl_BlockLayout::doclistener_insertSpan(), len=%d, pos %d \n", len, getPosition()+blockOffset));
	for (i=0; i<len; i++)
	{
		xxx_UT_DEBUGMSG(("fl_BlockLayout: char %d %c \n",i,static_cast<char>(pChars[i])));
		switch (pChars[i])
		{
		case UCS_FF:	// form feed, forced page break
		case UCS_VTAB:	// vertical tab, forced column break
		case UCS_LF:	// newline
		case UCS_FIELDSTART: // zero length line to mark field start
		case UCS_FIELDEND: // zero length line to mark field end
		case UCS_BOOKMARKSTART:
		case UCS_BOOKMARKEND:
		case UCS_TAB:	// tab
		case UCS_LRO:	// explicit direction overrides
		case UCS_RLO:
		case UCS_LRE:	
		case UCS_RLE:
		case UCS_PDF:
		case UCS_LRM:
		case UCS_RLM:

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
				getDocSectionLayout()->setNeedsSectionBreak(true,pPrevP);
				_doInsertForcedPageBreakRun(i + blockOffset);
				break;

			case UCS_VTAB:
				getDocSectionLayout()->setNeedsSectionBreak(true,pPrevP);
				_doInsertForcedColumnBreakRun(i + blockOffset);
				break;

			case UCS_LF:
				getDocSectionLayout()->setNeedsSectionBreak(true,pPrevP);
				_doInsertForcedLineBreakRun(i + blockOffset);
				break;

			case UCS_FIELDSTART:
				_doInsertFieldStartRun(i + blockOffset);
				break;

			case UCS_FIELDEND:
				_doInsertFieldEndRun(i + blockOffset);
				break;

			case UCS_BOOKMARKSTART:
			case UCS_BOOKMARKEND:
				_doInsertBookmarkRun(i + blockOffset);
				break;

			case UCS_TAB:
				_doInsertTabRun(i + blockOffset);
				break;

			case UCS_LRO:
			case UCS_RLO:
			case UCS_LRE:	
			case UCS_RLE:
			case UCS_PDF:
				// these should have been removed by
				// pd_Document::append/insert functions
				UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
				break;

			case UCS_LRM:
			case UCS_RLM:
				_doInsertDirectionMarkerRun(i + blockOffset,pChars[i]);
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
		xxx_UT_DEBUGMSG(("insertSpan: BlockOffset %d iNormalBase %d i %d \n",blockOffset,iNormalBase,i));
		_doInsertTextSpan(blockOffset + iNormalBase, i - iNormalBase);
	}

	setNeedsReformat(iNormalBase);
	updateEnclosingBlockIfNeeded();

	m_pSquiggles->textInserted(blockOffset, len);

	FV_View* pView = getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->_setPoint(pcrs->getPosition() + len);
//		if(!isHdrFtr())
//			pView->notifyListeners(AV_CHG_FMTCHAR); // TODO verify that this is necessary.
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

	_assertRunListIntegrity();
#if 0
#if DEBUG
	fp_Run * ppRun = getFirstRun();
	while(ppRun)
	{
		if(ppRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun * pTRun = static_cast<fp_TextRun *>(ppRun);
			pTRun->printText();
		}
		ppRun = ppRun->getNext();
	}
#endif
#endif
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
	bool bPastFirst = false;
	while (pRun)
	{
		// Verify that offset of this block is correct.
#if 0
//
// FIXME: Invent a clever way to account for embedded hidden stuff
//        in blocks (like footnotes).
//        Maybe detect this sort of anomaly can verify it matches
//        what is in the piecetable
		UT_ASSERT( iOffset == pRun->getBlockOffset() );
#endif
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
//
// FIXME; Take this code out when things work.
//
		if(pRun->getNext() || (FPRUN_ENDOFPARAGRAPH == pRun->getType()) )
		{
		}
		else
		{
			m_pLayout->getDocument()->miniDump(getStruxDocHandle(),8);
		}
		UT_ASSERT( pRun->getNext()
				   || (FPRUN_ENDOFPARAGRAPH == pRun->getType()) );
		pRun = pRun->getNext();
		bPastFirst = true;
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

	// runs to do with bidi post-processing
	fp_TextRun * pTR_del1 = NULL;
	fp_TextRun * pTR_del2 = NULL;
	fp_TextRun * pTR_next = NULL;
	fp_TextRun * pTR_prev = NULL;
	
	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		UT_uint32 iRunBlockOffset = pRun->getBlockOffset();
		UT_uint32 iRunLength = pRun->getLength();
		fp_Run* pNextRun = pRun->getNext(); // remember where we're going, since this run may get axed

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
//
// Force a whole page redraw if we delete a page or column break
//
			if(pRun->getType() == FPRUN_FORCEDCOLUMNBREAK ||
			   pRun->getType() == FPRUN_FORCEDPAGEBREAK)
			{
				fp_Container * pCon = static_cast<fp_Container *>(pRun->getLine());
				fp_Page * pPage = pCon->getPage();
				if(pPage)
				{
					pPage->markAllDirty();
				}
			}
			if (blockOffset >= iRunBlockOffset)
			{
				if ((blockOffset + len) < (iRunBlockOffset + iRunLength))
				{
					// the deleted section is entirely within this run
					if(pRun->getType()== FPRUN_DIRECTIONMARKER)
					{
						if(pRun->getNext() && pRun->getNext()->getType()== FPRUN_TEXT)
						{
							pTR_next = static_cast<fp_TextRun*>(pRun->getNext());
						}
						
						if(pRun->getPrev() && pRun->getPrev()->getType()== FPRUN_TEXT)
						{
							pTR_prev = static_cast<fp_TextRun*>(pRun->getPrev());
						}
					}
					else if(pRun->getType()== FPRUN_TEXT)
					{
						pTR_del1 = static_cast<fp_TextRun*>(pRun);

						if(pRun->getNext() && pRun->getNext()->getType()== FPRUN_TEXT)
						{
							pTR_next = static_cast<fp_TextRun*>(pRun->getNext());
						}
						
						if(pRun->getPrev() && pRun->getPrev()->getType()== FPRUN_TEXT)
						{
							pTR_prev = static_cast<fp_TextRun*>(pRun->getPrev());
						}
					}
					
					pRun->setLength(iRunLength - len);
					UT_ASSERT((pRun->getLength() == 0) || (pRun->getType() == FPRUN_TEXT)); // only textual runs could have a partial deletion
					m_bFixCharWidths = true;
				}
				else
				{
					// the delete section crosses over the end of the
					// run, but not the start; it can however, lead to
					// deletion of an entire run
					int iDeleted = iRunBlockOffset + iRunLength - blockOffset;
					UT_ASSERT(iDeleted > 0);

					if(pRun->getType()== FPRUN_DIRECTIONMARKER)
					{
						if(pRun->getNext() && pRun->getNext()->getType()== FPRUN_TEXT)
						{
							pTR_next = static_cast<fp_TextRun*>(pRun->getNext());
						}
						
						if(pRun->getPrev() && pRun->getPrev()->getType()== FPRUN_TEXT)
						{
							pTR_prev = static_cast<fp_TextRun*>(pRun->getPrev());
						}
					}
					else if(pRun->getType()== FPRUN_TEXT)
					{
						if(!((iRunBlockOffset == blockOffset) && (iRunLength == len)))
							pTR_del1 = static_cast<fp_TextRun*>(pRun);

						if(pRun->getNext() && pRun->getNext()->getType()== FPRUN_TEXT)
						{
							pTR_next = static_cast<fp_TextRun*>(pRun->getNext());
						}
						
						if(pRun->getPrev() && pRun->getPrev()->getType()== FPRUN_TEXT)
						{
							pTR_prev = static_cast<fp_TextRun*>(pRun->getPrev());
						}
					}
					
					pRun->setLength(iRunLength - iDeleted);
					UT_ASSERT((pRun->getLength() == 0) || (pRun->getType() == FPRUN_TEXT)); // only textual runs could have a partial deletion
					m_bFixCharWidths = true;
				}
			}
			else
			{
				// the deleted section crosses over the start of the
				// run and possibly also the end; unless this is the
				// first run in the block, then we have already deleted
				// something
				if(pRun->getType()== FPRUN_DIRECTIONMARKER)
				{
					if(pRun->getNext() && pRun->getNext()->getType()== FPRUN_TEXT)
					{
						pTR_next = static_cast<fp_TextRun*>(pRun->getNext());
					}
						
					if(pRun->getPrev() && pRun->getPrev()->getType()== FPRUN_TEXT)
					{
						pTR_prev = static_cast<fp_TextRun*>(pRun->getPrev());
					}
				}
				else if(pRun->getType()== FPRUN_TEXT)
				{
					if(!pTR_del1 && pRun->getPrev() && pRun->getPrev()->getType()== FPRUN_TEXT)
					{
						pTR_prev = static_cast<fp_TextRun*>(pRun->getPrev());
					}

					if(pRun->getNext() && pRun->getNext()->getType()== FPRUN_TEXT)
					{
						pTR_next = static_cast<fp_TextRun*>(pRun->getNext());
					}
						
				}

				if ((blockOffset + len) < (iRunBlockOffset + iRunLength))
				{
					if(pTR_del1)
					{
						pTR_del2 = static_cast<fp_TextRun*>(pRun);
					}
					else
					{
						pTR_del1 = static_cast<fp_TextRun*>(pRun);
					}
					
					int iDeleted = blockOffset + len - iRunBlockOffset;
					UT_ASSERT(iDeleted > 0);
					pRun->setBlockOffset(iRunBlockOffset - (len - iDeleted));
					pRun->setLength(iRunLength - iDeleted);
					UT_ASSERT((pRun->getLength() == 0) || (pRun->getType() == FPRUN_TEXT)); // only textual runs could have a partial deletion
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
				if(pLine)
				{
					pLine->removeRun(pRun, true);
				}
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

	// now that we have done the deleting, we have to do some bidi
	// post processing, since the deletion might have seriously
	// impacted the visual order of the line; we have to break all
	// the text runs affected by this, plus the run before and after,
	// so that the bidi algorithm can be properly applied
	if(pTR_del1)
		pTR_del1->breakMeAtDirBoundaries(FRIBIDI_TYPE_IGNORE);
	
	if(pTR_del2)
		pTR_del2->breakMeAtDirBoundaries(FRIBIDI_TYPE_IGNORE);

	if(pTR_prev)
		pTR_prev->breakMeAtDirBoundaries(FRIBIDI_TYPE_IGNORE);

	if(pTR_next)
		pTR_next->breakMeAtDirBoundaries(FRIBIDI_TYPE_IGNORE);

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

	m_pSquiggles->textDeleted(blockOffset, len);

	FV_View* pView = getView();
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

	_assertRunListIntegrity();
	setNeedsReformat(blockOffset);
	updateEnclosingBlockIfNeeded();

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
		else if (pRun->getType() == FPRUN_IMAGE)
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
		// TODO: do we need to call lookupProperties for other run types.

		pRun = pRun->getNext();
	}

	setNeedsReformat(blockOffset);
	updateEnclosingBlockIfNeeded();

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
bool
fl_BlockLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux* pcrx)
{
	UT_DEBUGMSG(("doclistener_deleteStrux\n"));

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
//
// Do this before all the required info is deleted.
//
	updateEnclosingBlockIfNeeded();
	fp_Container * pCon = getFirstContainer();
	if(pCon)
	{
		fp_Page * pPage = pCon->getPage();
		getDocSectionLayout()->setNeedsSectionBreak(true,pPage);
	}
	else
	{
		getDocSectionLayout()->setNeedsSectionBreak(true,NULL);
	}
	// Erase the old version.  Or this what I added when adding the
	// EOP stuff. Only, I don't remember why I did it, and it's wrong:
	// the strux is deleted only after its content has been deleted -
	// so the call might try to clear empty lines. jskov 2001.04.23

	// Sevior put this back 2001.6.3. I don't understand why there was ever any
	// question about it's neccessity.

	clearScreen(m_pLayout->getGraphics());

	// If there is a previous strux, we merge the Runs from this strux
	// into it - including the EOP Run, so delete that in the previous
	// strux.
	// If there is no previous strux (this being the first strux in
	// the document) this will be empty - but the EOP Run needs to be
	// deleted.

	fl_BlockLayout* pPrevBL = static_cast<fl_BlockLayout *>(getPrev());
	fp_Line* pLastLine = NULL;
	if(pPrevBL && pPrevBL->getContainerType() != FL_CONTAINER_BLOCK)
	{
//
// Attach to the block before the table, because the table is being deleted.
//
		if(pPrevBL->getContainerType() == FL_CONTAINER_TABLE)
		{
			pPrevBL = static_cast<fl_BlockLayout *>(pPrevBL->getPrev());
		}
		else
		{
			pPrevBL = NULL;
		}
	}
	if (pPrevBL)
	{
		// Find the EOP Run.
		pLastLine = static_cast<fp_Line *>(pPrevBL->getLastContainer());
		fp_Run* pNukeRun = pPrevBL->m_pFirstRun;
		fp_Run * pPrevRun = pPrevBL->m_pFirstRun;
		while(pNukeRun->getNext() != NULL)
		{
			pPrevRun = pNukeRun;
			UT_ASSERT(FPRUN_ENDOFPARAGRAPH != pPrevRun->getType());
			pNukeRun  = pPrevRun->getNext();
		}
		UT_ASSERT(FPRUN_ENDOFPARAGRAPH == pNukeRun->getType());

		// Detach from the line
		fp_Line* pLine = pNukeRun->getLine();
		UT_ASSERT(pLine && pLine == pLastLine);
		pLine->removeRun(pNukeRun);

		// Unlink and delete it
		if (pPrevRun && (pPrevRun != pNukeRun))
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
			offset = pLastRun->getBlockOffset();
			offset += pLastRun->getLength();
		}
		else
		{
			pPrevBL->m_pFirstRun = m_pFirstRun;
			offset = m_pFirstRun->getBlockOffset();
		}
		UT_DEBUGMSG(("deleteStrux: offset = %d \n",offset));
		// Merge charwidths
		UT_uint32 lenNew = m_gbCharWidths.getLength();

		pPrevBL->m_gbCharWidths.ins(offset, m_gbCharWidths, 0, lenNew);

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
		pPrevBL->setNext(getNext());
	}
	if (getNext())
	{
		getNext()->setPrev(pPrevBL);
	}

	fl_SectionLayout* pSL = static_cast<fl_SectionLayout *>(myContainingLayout());
	UT_ASSERT(pSL);
	pSL->remove(this);

	if (pPrevBL)
	{
//
// Now fix up the previous block. Calling this format fixes bug 2702
//
		pPrevBL->format();
		pPrevBL->setNeedsReformat(); // why when we just formatted it?
		// This call will dequeue the block from background checking
		// if necessary
		m_pSquiggles->join(offset, pPrevBL);
	}
	else
	{
		// In case we've never checked this one
		m_pLayout->dequeueBlockForBackgroundCheck(this);
	}

	FV_View* pView = pSL->getDocLayout()->getView();
	if (pView->isHdrFtrEdit() && (!pView->getEditShadow() ||
		                         !pView->getEditShadow()->getLastLayout()))
		pView->clearHdrFtrEdit();

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

	// erase the old version
	if(!isHdrFtr())
	{
		clearScreen(m_pLayout->getGraphics());
	}
	setAttrPropIndex(pcrxc->getIndexAP());
	xxx_UT_DEBUGMSG(("SEVIOR: In changeStrux in fl_BlockLayout \n"));
//
// Not sure if we'll ever need this. We don't need this now I'll comment it out.
//	const XML_Char * szOldStyle = m_szStyle;
	UT_sint32 iOldDomDirection = m_iDomDirection;

	_lookupProperties();
	xxx_UT_DEBUGMSG(("SEVIOR: Old Style = %s new style = %s \n",szOldStyle,m_szStyle));
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

		xxx_UT_DEBUGMSG(("SEVIOR: Doing a style change \n"));
		while (pRun)
		{
			pRun->lookupProperties();
			pRun->fetchCharWidths(&m_gbCharWidths);
			pRun->recalcWidth();

			pRun = pRun->getNext();
		}
	}

	fp_Line* pLine = static_cast<fp_Line *>(getFirstContainer());
	while (pLine)
	{
		pLine->recalcHeight();	// line-height
		pLine->recalcMaxWidth();

		if(m_iDomDirection != iOldDomDirection)
		{
			xxx_UT_DEBUGMSG(("block listener: change of direction\n"));
			pLine->setMapOfRunsDirty();
		}

		pLine = static_cast<fp_Line *>(pLine->getNext());
	}

//	This was...
	setNeedsReformat();
	updateEnclosingBlockIfNeeded();
	//
	// Need this to find where to break section in the document.
	//
	fl_ContainerLayout * pPrevCL = getPrev();
	fp_Page * pPrevP = NULL;
	if(pPrevCL)
	{
		fp_Container * pPrevCon = pPrevCL->getFirstContainer();
		if(pPrevCon)
		{
			pPrevP = pPrevCon->getPage();
		}
	}
	getDocSectionLayout()->setNeedsSectionBreak(true,pPrevP);

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
	//	Exchange handles with the piece table
	PL_StruxFmtHandle sfhNew = static_cast<PL_StruxFmtHandle>(this);
	pfnBindHandles(sdh,lid,sfhNew);

	setNeedsReformat();
	updateEnclosingBlockIfNeeded();

	FV_View* pView = getView();
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

	fl_SectionLayout* pSL = static_cast<fl_SectionLayout *>(myContainingLayout());
	UT_ASSERT(pSL);
	fl_BlockLayout* pNewBL = static_cast<fl_BlockLayout *>(pSL->insert(sdh, this, pcrx->getIndexAP(),FL_CONTAINER_BLOCK));
	if(isHdrFtr())
		pNewBL->setHdrFtr();
	if (!pNewBL)
	{
		UT_DEBUGMSG(("no memory for BlockLayout\n"));
		return false;
	}
	UT_DEBUGMSG(("Inserting block %x it's sectionLayout type is %d \n",pNewBL,pNewBL->getSectionLayout()->getContainerType()));
	//xxx_UT_DEBUGMSG(("Inserting block at pos %d \n",getPosition(true)));
	//xxx_UT_DEBUGMSG(("shd of strux block = %x of new block is %x \n",getStruxDocHandle(),pNewBL->getStruxDocHandle()));
	// The newly returned block will contain a line and EOP. Delete those
	// since the code below expects an empty block
	pNewBL->_purgeEndOfParagraphRun();

	// Must call the bind function to complete the exchange
	// of handles with the document (piece table) *** before ***
	// anything tries to call down into the document (like all
	// of the view listeners).

	PL_StruxFmtHandle sfhNew = static_cast<PL_StruxFmtHandle>(pNewBL);
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
				fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pRun);
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

	// pFirstNewRun can be NULL at this point.	It means that the
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
	updateEnclosingBlockIfNeeded();

	// Split squiggles between this and the new block
	m_pSquiggles->split(blockOffset, pNewBL);

	FV_View* pView = getView();
	if (pView && (pView->isActive() || pView->isPreview()))
		pView->_setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
	else if(pView && pView->getPoint() > pcrx->getPosition())
		pView->_setPoint(pView->getPoint() + fl_BLOCK_STRUX_OFFSET);

	_assertRunListIntegrity();
	UT_DEBUGMSG(("Prev Block = %x Next block = %x \n",pNewBL->getPrev(),pNewBL->getNext()));
	return true;
}

bool fl_BlockLayout::doclistener_insertSection(const PX_ChangeRecord_Strux * pcrx,
											   SectionType iType,
											   PL_StruxDocHandle sdh,
											   PL_ListenerId lid,
											   void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	   PL_ListenerId lid,
																	   PL_StruxFmtHandle sfhNew))
{
	UT_ASSERT(iType == FL_SECTION_DOC || iType == FL_SECTION_HDRFTR
			  || iType == FL_SECTION_FOOTNOTE || iType == FL_SECTION_ENDNOTE);

	_assertRunListIntegrity();

	// Insert a section at the location given in the change record.
	// Everything from this point forward (to the next section) needs
	// to be re-parented to this new section.  We also need to verify
	// that this insertion point is at the end of the block (and that
	// another block follows).	This is because a section cannot
	// contain content.

	UT_ASSERT(pcrx);
	UT_ASSERT(pfnBindHandles);
	UT_ASSERT(pcrx->getType() == PX_ChangeRecord::PXT_InsertStrux);
	UT_ASSERT(iType != FL_SECTION_DOC || pcrx->getStruxType() == PTX_Section);
	UT_ASSERT(iType != FL_SECTION_HDRFTR || pcrx->getStruxType() == PTX_SectionHdrFtr);
	UT_ASSERT(iType != FL_SECTION_FOOTNOTE || pcrx->getStruxType() == PTX_SectionFootnote);
	getDocSectionLayout()->setNeedsSectionBreak(true,NULL);

//
// Not true always. eg Undo on a delete header/footer. We should detect this
// and deal with it.
//
	PT_DocPosition pos1;
//
// This is to clean the fragments
//
	m_pDoc->getBounds(true,pos1);
	fl_DocSectionLayout* pDSL = NULL;
	if(m_pSectionLayout->getType() == FL_SECTION_DOC)
		pDSL =	static_cast<fl_DocSectionLayout *>(m_pSectionLayout);

	xxx_UT_DEBUGMSG(("SectionLayout for block is %x block is %x \n",m_pSectionLayout,this));
	fl_SectionLayout* pSL = NULL;
	const XML_Char* pszNewID = NULL;

	UT_DEBUGMSG(("Insert section at pos %d sdh of section =%x sdh of block =%x \n",getPosition(true),sdh,getStruxDocHandle()));

	switch (iType)
	{
	case FL_SECTION_DOC:
		pSL = new fl_DocSectionLayout
			(m_pLayout, sdh, pcrx->getIndexAP(), FL_SECTION_DOC);		
		if (!pSL)
		{
			UT_DEBUGMSG(("no memory for SectionLayout"));
			return false;
		}

		m_pLayout->insertSectionAfter(pDSL, static_cast<fl_DocSectionLayout*>(pSL));
		break;
	case FL_SECTION_HDRFTR:
	{
		pSL = new fl_HdrFtrSectionLayout(FL_HDRFTR_NONE,m_pLayout,NULL, sdh, pcrx->getIndexAP());
		if (!pSL)
		{
			UT_DEBUGMSG(("no memory for SectionLayout"));
			return false;
		}

		fl_HdrFtrSectionLayout * pHFSL = static_cast<fl_HdrFtrSectionLayout *>(pSL);
		m_pLayout->addHdrFtrSection(pHFSL);
//
// Need to find the DocSectionLayout associated with this.
//
		const PP_AttrProp* pHFAP = NULL;
		PT_AttrPropIndex indexAP = pcrx->getIndexAP();
		bool bres = (m_pDoc->getAttrProp(indexAP, &pHFAP) && pHFAP);
		UT_ASSERT(bres);
		pHFAP->getAttribute("id", pszNewID);
//
// pszHFID may not be defined yet. If not we can't do this stuff. If it is defined
// this step is essential
//
		if(pszNewID)
		{
		  // plam mystery code
			// plam, MES here, I need this code for inserting headers/footers.
		  UT_DEBUGMSG(("new id: tell plam if you see this message\n"));
//		  UT_ASSERT(0);
			fl_DocSectionLayout* pDocSL = m_pLayout->findSectionForHdrFtr(static_cast<const char*>(pszNewID));
			UT_ASSERT(pDocSL);
//
// Determine if this is a header or a footer.
//
			const XML_Char* pszSectionType = NULL;
			pHFAP->getAttribute("type", pszSectionType);

			HdrFtrType hfType = FL_HDRFTR_NONE;
			if (pszSectionType && *pszSectionType)
			{
				if(UT_strcmp(pszSectionType,"header") == 0)
					hfType = FL_HDRFTR_HEADER;
				else if (UT_strcmp(pszSectionType,"header-even") == 0)
					hfType = FL_HDRFTR_HEADER_EVEN;
				else if (UT_strcmp(pszSectionType,"header-first") == 0)
					hfType = FL_HDRFTR_HEADER_FIRST;
				else if (UT_strcmp(pszSectionType,"header-last") == 0)
					hfType = FL_HDRFTR_HEADER_LAST;
				else if (UT_strcmp(pszSectionType,"footer") == 0)
					hfType = FL_HDRFTR_FOOTER;
				else if (UT_strcmp(pszSectionType,"footer-even") == 0)
					hfType = FL_HDRFTR_FOOTER_EVEN;
				else if (UT_strcmp(pszSectionType,"footer-first") == 0)
					hfType = FL_HDRFTR_FOOTER_FIRST;
				else if (UT_strcmp(pszSectionType,"footer-last") == 0)
					hfType = FL_HDRFTR_FOOTER_LAST;

				if(hfType != FL_HDRFTR_NONE)
				{
					pHFSL->setDocSectionLayout(pDocSL);
					pHFSL->setHdrFtr(hfType);
					//
					// Set the pointers to this header/footer
					//
					pDocSL->setHdrFtr(hfType, pHFSL);
				}
			}
		}
		else
		{
			UT_DEBUGMSG(("NO ID found with insertSection HdrFtr \n"));
		}
		break;
	}
	case FL_SECTION_ENDNOTE:
	case FL_SECTION_FOOTNOTE:
	{
		// Most of the time, we would insert a new section
		// after the previous section.
		// But, here we insert our FootnoteLayout after this(?)
		// BlockLayout. -PL
		PT_AttrPropIndex indexAP = pcrx->getIndexAP();
		if(iType == FL_SECTION_FOOTNOTE)
		{
			pSL = static_cast<fl_SectionLayout *>(static_cast<fl_ContainerLayout *>(getSectionLayout())->insert(sdh,this,indexAP, FL_CONTAINER_FOOTNOTE));
		}
		else
		{
			pSL = static_cast<fl_SectionLayout *>(static_cast<fl_ContainerLayout *>(getSectionLayout())->insert(sdh,this,indexAP, FL_CONTAINER_ENDNOTE));
		}
//
// Need to find the DocSectionLayout associated with this.
//
		const PP_AttrProp* pAP = NULL;
		bool bres = (m_pDoc->getAttrProp(indexAP, &pAP) && pAP);
		UT_ASSERT(bres);
		pAP->getAttribute("id", pszNewID);
		break;
	}
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	PT_DocPosition posSL = m_pDoc->getStruxPosition(pSL->getStruxDocHandle());
	PT_DocPosition posThis = m_pDoc->getStruxPosition(getStruxDocHandle());

	// Must call the bind function to complete the exchange of handles
	// with the document (piece table) *** before *** anything tries
	// to call down into the document (like all of the view
	// listeners).

	PL_StruxFmtHandle sfhNew = static_cast<PL_StruxFmtHandle>(pSL);
	pfnBindHandles(sdh,lid,sfhNew);

	fl_SectionLayout* pOldSL = m_pSectionLayout;

	if ((iType == FL_SECTION_FOOTNOTE) || (iType == FL_SECTION_ENDNOTE))
	{
//
// Now update the position pointer in the view
//
		FV_View* pView = getView();
		if (pView && (pView->isActive() || pView->isPreview()))
		{
			pView->_setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
		}
		else if(pView && pView->getPoint() > pcrx->getPosition())
		{
			pView->_setPoint(pView->getPoint() + fl_BLOCK_STRUX_OFFSET + fl_BLOCK_STRUX_OFFSET);
		}
		return true;
	}
//
// Now move all the blocks following into the new section
//
	fl_ContainerLayout* pCL = NULL;
	if(posSL < posThis)
	{
		pCL = this;
	}
	else
	{
		pCL = getNext();
	}
	//
	// BUT!!! Don't move the immediate Footnotes or Endnotes
	//
	fl_ContainerLayout * pLastCL = NULL;
	if(pCL)
	{
		pLastCL = pCL->getPrev();
	}
	while(pCL && ((pCL->getContainerType() == FL_CONTAINER_FOOTNOTE) ||
		  (pCL->getContainerType() == FL_CONTAINER_ENDNOTE)))
	{
		pLastCL = pCL;
		pCL = pCL->getNext();
	}
	fl_BlockLayout * pBL = NULL;
	while (pCL)
	{
		fl_ContainerLayout* pNext = pCL->getNext();
		pBL = NULL;
		pCL->collapse();
		if(pCL->getContainerType()==FL_CONTAINER_BLOCK)
		{
			pBL = static_cast<fl_BlockLayout *>(pCL);
		} 
		if(pBL && pBL->isHdrFtr())
		{
			fl_HdrFtrSectionLayout * pHF = static_cast<fl_HdrFtrSectionLayout *>(pBL->getSectionLayout());
			pHF->collapseBlock(pBL);
		}
		pOldSL->remove(pCL);
		pSL->add(pCL);
		if(pBL)
		{
			pBL->setSectionLayout( pSL);
			pBL->m_iNeedsReformat = 0;
		}
		pCL = pNext;
	}

//
// Terminate blocklist here. This Block is the last in this section.
//
	if (pLastCL)
	{
		pLastCL->setNext(NULL);
		pOldSL->setLastLayout(pLastCL);
	}
//
// OK we have to redo all the containers now.
//
	if(pSL->getType() == FL_SECTION_DOC)
	{
		fl_DocSectionLayout * pFirstDSL = static_cast<fl_DocSectionLayout *>(pOldSL);
		pDSL = pFirstDSL;
		while(pDSL != NULL)
		{
			pDSL->collapse();
			pDSL = pDSL->getNextDocSection();
		}
		pDSL = pFirstDSL;
		while(pDSL != NULL)
		{
			pDSL->updateDocSection();
			pDSL = pDSL->getNextDocSection();
		}
	}

//
// In the case of Header/Footer sections we must now format this stuff to create
// the shadows.
//
	if(iType == FL_SECTION_HDRFTR || iType == FL_SECTION_FOOTNOTE)
	{
		if(pszNewID)
		{
			pSL->format();
			pSL->redrawUpdate();
		}
		else
			return true;
	}
	updateEnclosingBlockIfNeeded();

	FV_View* pView = getView();
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

/*!
 * Insert a table into the list of blocks
 */
fl_SectionLayout * fl_BlockLayout::doclistener_insertTable(const PX_ChangeRecord_Strux * pcrx,
											   SectionType iType,
											   PL_StruxDocHandle sdh,
											   PL_ListenerId lid,
											   void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	   PL_ListenerId lid,
																	   PL_StruxFmtHandle sfhNew))
{
	UT_ASSERT(iType == FL_SECTION_TABLE);
	_assertRunListIntegrity();

	// Insert a section at the location given in the change record.
	// Everything from this point forward (to the next section) needs
	// to be re-parented to this new section.  We also need to verify
	// that this insertion point is at the end of the block (and that
	// another block follows).	This is because a section cannot
	// contain content.

	UT_ASSERT(pcrx);
	UT_ASSERT(pfnBindHandles);
	UT_ASSERT(pcrx->getType() == PX_ChangeRecord::PXT_InsertStrux);
//
// Not true always. eg Undo on a delete header/footer. We should detect this
// and deal with it.
//
	PT_DocPosition pos1;
//
// This is to clean the fragments
//
	m_pDoc->getBounds(true,pos1);

	fl_SectionLayout* pSL = NULL;

	pSL = static_cast<fl_SectionLayout *>(static_cast<fl_ContainerLayout *>(getSectionLayout())->insert(sdh,this,pcrx->getIndexAP(), FL_CONTAINER_TABLE));

		// Must call the bind function to complete the exchange of handles
		// with the document (piece table) *** before *** anything tries
		// to call down into the document (like all of the view
		// listeners).

	PL_StruxFmtHandle sfhNew = static_cast<PL_StruxFmtHandle>(pSL);
	pfnBindHandles(sdh,lid,sfhNew);

//
// increment the insertion point in the view.
//
	FV_View* pView = getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->_setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->_setPoint(pView->getPoint() + fl_BLOCK_STRUX_OFFSET);
	}
//
// OK that's it!
//
	updateEnclosingBlockIfNeeded();

	return pSL;
}

/*!
 Draw squiggles intersecting with Run
 \param pRun Run

 For all misspelled words in this run, call the run->drawSquiggle()
 method.
*/
void
fl_BlockLayout::findSquigglesForRun(fp_Run* pRun)
{
	xxx_UT_DEBUGMSG(("fl_BlockLayout::findSquigglesForRun\n"));

	UT_ASSERT(pRun->getType() == FPRUN_TEXT);
	fp_TextRun* pTextRun = (static_cast<fp_TextRun*>(pRun));

	UT_sint32 runBlockOffset = pRun->getBlockOffset();
	UT_sint32 runBlockEnd = runBlockOffset + pRun->getLength();
	UT_sint32 iFirst, iLast;
	if (m_pSquiggles->findRange(runBlockOffset, runBlockEnd, iFirst, iLast))
	{
		UT_sint32 iStart = 0, iEnd;
		fl_PartOfBlock* pPOB;
		UT_sint32 i = iFirst;

		// The first POB may only be partially within the region. Clip
		// it if necessary.
		pPOB = m_pSquiggles->getNth(i++);
		if (!pPOB->getIsIgnored())
		{
			iStart = pPOB->getOffset();
			iEnd =	iStart + pPOB->getLength();
			if (iStart < runBlockOffset) iStart = runBlockOffset;

			// Only draw if there's more than one POB. If there's only
			// one POB, it may also need clipping at the end (let the
			// code below handle it).
			if (iFirst != iLast)
			{
				pTextRun->drawSquiggle(iStart, iEnd - iStart);
			}
		}
		// The ones in the middle don't need clipping.
		for (; i < iLast; i++)
		{
			pPOB = m_pSquiggles->getNth(i);
			if (pPOB->getIsIgnored()) continue;

			iStart = pPOB->getOffset();
			iEnd =	iStart + pPOB->getLength();
			pTextRun->drawSquiggle(iStart, iEnd - iStart);
		}
		// The last POB may only be partially within the region. Clip
		// it if necessary. Note the load with iLast instead of i.
		pPOB = m_pSquiggles->getNth(iLast);
		if (!pPOB->getIsIgnored())
		{
			// Only load start if this POB is different from the first
			// one.
			if (iFirst != iLast)
				iStart = pPOB->getOffset();
			iEnd =	pPOB->getOffset() + pPOB->getLength();
			if (iEnd > runBlockEnd) iEnd = runBlockEnd;
			pTextRun->drawSquiggle(iStart, iEnd - iStart);
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
		return true;
	}

	case PTO_Field:
		UT_DEBUGMSG(("Populate:InsertObject:Field:\n"));
		_doInsertFieldRun(blockOffset, pcro);
		return true;

	case PTO_Bookmark:
		UT_DEBUGMSG(("Populate:InsertBookmark:\n"));
		_doInsertBookmarkRun(blockOffset);
		return true;

	case PTO_Hyperlink:
		UT_DEBUGMSG(("Populate:InsertHyperlink:\n"));
		_doInsertHyperlinkRun(blockOffset);
		return true;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}

	_assertRunListIntegrity();
	updateEnclosingBlockIfNeeded();

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
		break;
	}

	case PTO_Field:
	{
		UT_DEBUGMSG(("Edit:InsertObject:Field:\n"));
		blockOffset = pcro->getBlockOffset();
		_doInsertFieldRun(blockOffset, pcro);
		break;
	}

	case PTO_Bookmark:
	{
		UT_DEBUGMSG(("Edit:InsertObject:Bookmark:\n"));
		blockOffset = pcro->getBlockOffset();
		_doInsertBookmarkRun(blockOffset);
		break;

	}

	case PTO_Hyperlink:
	{
		UT_DEBUGMSG(("Edit:InsertObject:Hyperlink:\n"));
		blockOffset = pcro->getBlockOffset();
		_doInsertHyperlinkRun(blockOffset);
		break;

	}

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}

	setNeedsReformat(blockOffset);
	updateEnclosingBlockIfNeeded();

	FV_View* pView = getView();
	if (pView && (pView->isActive() || pView->isPreview()))
		pView->_setPoint(pcro->getPosition() + 1);
	else if(pView && pView->getPoint() > pcro->getPosition())
		pView->_setPoint(pView->getPoint() + 1);

	// TODO: are objects always one wide?
	m_pSquiggles->textInserted(blockOffset, 1);

	_assertRunListIntegrity();

	return true;
}

bool fl_BlockLayout::doclistener_deleteObject(const PX_ChangeRecord_Object * pcro)
{
	_assertRunListIntegrity();

	PT_BlockOffset blockOffset = 0;

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

		case PTO_Bookmark:
		{
			UT_DEBUGMSG(("Edit:DeleteObject:Bookmark:\n"));
			blockOffset = pcro->getBlockOffset();
			_delete(blockOffset,1);
			break;
		}

		case PTO_Hyperlink:
		{
			UT_DEBUGMSG(("Edit:DeleteObject:Hyperlink:\n"));
			blockOffset = pcro->getBlockOffset();
			_delete(blockOffset,1);
			break;
		}

		default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	updateEnclosingBlockIfNeeded();
	setNeedsReformat(blockOffset);

	FV_View* pView = getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->_resetSelection();
		pView->_setPoint(pcro->getPosition());
	}
	else if(pView && pView->getPoint() > pcro->getPosition())
		pView->_setPoint(pView->getPoint() - 1);

	// TODO: are objects always one wide?
	m_pSquiggles->textDeleted(blockOffset, 1);

	_assertRunListIntegrity();

	return true;
}

bool fl_BlockLayout::doclistener_changeObject(const PX_ChangeRecord_ObjectChange * pcroc)
{

	_assertRunListIntegrity();

	PT_BlockOffset blockOffset = 0;
	switch (pcroc->getObjectType())
	{
	case PTO_Bookmark:
	case PTO_Hyperlink:
		return true;
	case PTO_Image:
	{
		UT_DEBUGMSG(("Edit:ChangeObject:Image:\n"));
		blockOffset = pcroc->getBlockOffset();
		fp_Run* pRun = m_pFirstRun;
		while (pRun)
		{
			if (pRun->getBlockOffset() == blockOffset)
			{
				if(pRun->getType()!= FPRUN_IMAGE)
				{
					UT_DEBUGMSG(("!!! run type NOT OBJECT, instead = %d !!!! \n",pRun->getType()));
					while(pRun && pRun->getType() == FPRUN_FMTMARK)
					{
						pRun = pRun->getNext();
					}
				}
				if(!pRun || pRun->getType() != FPRUN_IMAGE)
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					return false;
				}
				fp_ImageRun* pImageRun = static_cast<fp_ImageRun*>(pRun);
				if(!isHdrFtr())
				{
					pImageRun->clearScreen();
				}
				pImageRun->lookupProperties();

				goto done;
			}
			pRun = pRun->getNext();
		}

		return false;
	}
	case PTO_Field:
	{
		UT_DEBUGMSG(("Edit:ChangeObject:Field:\n"));
		blockOffset = pcroc->getBlockOffset();
		fp_Run* pRun = m_pFirstRun;
		while (pRun)
		{
			if (pRun->getBlockOffset() == blockOffset && (pRun->getType()!= FPRUN_FMTMARK))
			{
				if(pRun->getType()!= FPRUN_FIELD)
				{
					UT_DEBUGMSG(("!!! run type NOT OBJECT, instead = %d !!!! \n",pRun->getType()));
					while(pRun && pRun->getType() == FPRUN_FMTMARK)
					{
						pRun = pRun->getNext();
					}
				}
				if(!pRun || pRun->getType() != FPRUN_FIELD)
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					return false;
				}
				fp_FieldRun* pFieldRun = static_cast<fp_FieldRun*>(pRun);
				if(!isHdrFtr())
				{
					pFieldRun->clearScreen();
				}
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
	setNeedsReformat(blockOffset);

	_assertRunListIntegrity();

	return true;
}

bool fl_BlockLayout::recalculateFields(UT_uint32 iUpdateCount)
{
	_assertRunListIntegrity();

	bool bResult = false;
	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		if (pRun->getType() == FPRUN_FIELD)
		{
			fp_FieldRun* pFieldRun = static_cast<fp_FieldRun*>(pRun);

			xxx_UT_DEBUGMSG(("DOM: %d %d\n", pFieldRun==0, pFieldRun->needsFrequentUpdates()));

			if((!iUpdateCount
				|| !pFieldRun->needsFrequentUpdates()
				|| !(iUpdateCount % pFieldRun->needsFrequentUpdates())))
			{
				const bool bSizeChanged = pFieldRun->calculateValue();
				bResult |= bSizeChanged;
			}
		}
		//				else if(pRun->isField() == true)
		//	{
		//		 bResult = pRun->getField()->update();
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
		fl_TabStop* pTab = static_cast<fl_TabStop*>(m_vecTabs.getNthItem(i));
		UT_ASSERT(pTab);

		if (pTab->getPosition() > iMaxX)
		{
			break;
		}

		if (pTab->getPosition() > iStartX)
		{
			if(m_iDomDirection == FRIBIDI_TYPE_RTL)
			{
				if(m_iRightMargin > iStartX && m_iRightMargin < pTab->getPosition())
				{
					iPosition = m_iRightMargin;
					iType = FL_TAB_RIGHT;
					iLeader = FL_LEADER_NONE;
				}
				else
				{
					iPosition = pTab->getPosition();
					iType = pTab->getType();
					iLeader = pTab->getLeader();
				}

			}
			else
			{
				if(m_iLeftMargin > iStartX && m_iLeftMargin < pTab->getPosition())
				{
					iPosition = m_iLeftMargin;
					iType = FL_TAB_LEFT;
					iLeader = FL_LEADER_NONE;
				}
				else
				{
					iPosition = pTab->getPosition();
					iType = pTab->getType();
					iLeader = pTab->getLeader();
				}
			}

			return true;
		}
	}

	// now, handle the default tabs

	UT_sint32 iMin;

	if(m_iDomDirection == FRIBIDI_TYPE_RTL)
		iMin = m_iRightMargin;
	else
		iMin = m_iLeftMargin;

	if (iMin > iStartX)
	{
		iPosition = iMin;

		if(m_iDomDirection == FRIBIDI_TYPE_RTL)
			iType = FL_TAB_RIGHT;
		else
			iType = FL_TAB_LEFT;

		return true;
	}

	UT_ASSERT(m_iDefaultTabInterval > 0);

	// mathematical approach
	const UT_sint32 iPos = (iStartX / m_iDefaultTabInterval + 1) *
		m_iDefaultTabInterval;

	if(iPos > iMaxX)
		iPosition = iMaxX;
	else
		iPosition = iPos;

	if(m_iDomDirection == FRIBIDI_TYPE_RTL)
		iType = FL_TAB_RIGHT;
	else
		iType = FL_TAB_LEFT;

	UT_ASSERT(iPos > iStartX);

	return true;
}

bool	fl_BlockLayout::findPrevTabStop( UT_sint32 iStartX, UT_sint32 iMaxX, UT_sint32& iPosition,
										 eTabType & iType, eTabLeader &iLeader )
{
	UT_ASSERT(iStartX >= 0);

	UT_uint32 iCountTabs = m_vecTabs.getItemCount();
	UT_uint32 i;

	iLeader = FL_LEADER_NONE;

	for (i=0; i<iCountTabs; i++)
	{
		fl_TabStop* pTab = static_cast<fl_TabStop*>(m_vecTabs.getNthItem(i));
		UT_ASSERT(pTab);

		if (pTab->getPosition() > iMaxX)
		{
			break;
		}

		if (pTab->getPosition() > iStartX)
		{
			pTab = static_cast<fl_TabStop*>(m_vecTabs.getNthItem(i>0?i-1:0));
			UT_ASSERT(pTab);

			if(m_iDomDirection == FRIBIDI_TYPE_RTL)
			{
				if(m_iRightMargin > pTab->getPosition() && m_iRightMargin < iStartX)
				{
					iPosition = m_iRightMargin;
					iType = FL_TAB_RIGHT;
					iLeader = FL_LEADER_NONE;
				}
				else
				{
					iPosition = pTab->getPosition();
					iType = pTab->getType();
					iLeader = pTab->getLeader();
				}

			}
			else
			{
				if(m_iLeftMargin > pTab->getPosition() && m_iLeftMargin < iStartX)
				{
					iPosition = m_iLeftMargin;
					iType = FL_TAB_LEFT;
					iLeader = FL_LEADER_NONE;
				}
				else
				{
					iPosition = pTab->getPosition();
					iType = pTab->getType();
					iLeader = pTab->getLeader();
				}
			}
			return true;
		}
	}

	// the special case where there is no tabstop after the tab
	// but there is something before it
	if(iCountTabs > 0 && i == iCountTabs)
	{
			UT_DEBUGMSG(("found tabstop indx=%d\n", iCountTabs - 1));
			fl_TabStop* pTab = static_cast<fl_TabStop*>(m_vecTabs.getNthItem(iCountTabs - 1));
			UT_ASSERT(pTab);

			iPosition = pTab->getPosition();
			iType = pTab->getType();
			iLeader = pTab->getLeader();

			return true;
	}

	// now, handle the default tabs

	UT_sint32 iMin;

	if(m_iDomDirection == FRIBIDI_TYPE_RTL)
		iMin = m_iRightMargin;
	else
		iMin = m_iLeftMargin;

	if (iMin >= iStartX)
	{
		iPosition = iMin;

		if(m_iDomDirection == FRIBIDI_TYPE_RTL)
			iType = FL_TAB_RIGHT;
		else
			iType = FL_TAB_LEFT;

		return true;
	}

	UT_ASSERT(m_iDefaultTabInterval > 0);

	// mathematical approach
	// the -1 is to ensure we do not get iStartX
	const UT_sint32 iPos = ((iStartX - 1)/ m_iDefaultTabInterval) *
		m_iDefaultTabInterval;
	iPosition = iPos;

		if(m_iDomDirection == FRIBIDI_TYPE_RTL)
			iType = FL_TAB_RIGHT;
		else
			iType = FL_TAB_LEFT;

	UT_ASSERT(iPos <= iStartX);

	return true;
}

bool fl_BlockLayout::s_EnumTabStops( void * myThis, UT_uint32 k, fl_TabStop *pTabInfo)
{
	// a static function

	fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(myThis);

	UT_uint32 iCountTabs = pBL->m_vecTabs.getItemCount();
	if (k >= iCountTabs)
		return false;

	fl_TabStop * pTab = static_cast<fl_TabStop *>(pBL->m_vecTabs.getNthItem(k));

	*pTabInfo = *pTab;
	return true;
}


void fl_BlockLayout::setSectionLayout(fl_SectionLayout* pSectionLayout)
{
	//	If we are setting the new section layout, this block
	//	shouldn't already have a section.  If we are clearing
	//	it, then it should already have a section.
	if (pSectionLayout == NULL)
	{
		UT_ASSERT(m_pSectionLayout != NULL);
	}
	m_pSectionLayout = pSectionLayout;
	if(pSectionLayout)
		m_bIsHdrFtr = (pSectionLayout->getType() == FL_SECTION_HDRFTR);
}

//////////////////////////////////////////////////////////////////
// FmtMark-related stuff
//////////////////////////////////////////////////////////////////

bool
fl_BlockLayout::doclistener_insertFmtMark(const PX_ChangeRecord_FmtMark* pcrfm)
{
	_assertRunListIntegrity();

	PT_BlockOffset blockOffset = pcrfm->getBlockOffset();

	xxx_UT_DEBUGMSG(("Edit:InsertFmtMark [blockOffset %ld]\n",blockOffset));

	fp_FmtMarkRun * pNewRun = new fp_FmtMarkRun(this,m_pLayout->getGraphics(),blockOffset);
	UT_ASSERT(pNewRun);
	_doInsertRun(pNewRun);

	// TODO is it necessary to force a reformat when inserting a
	// FmtMark -- no fmt mark has no width, so it cannot change layout
	// setNeedsReformat();

	FV_View* pView = getView();
	if (pView && (pView->isActive() || pView->isPreview()))
		pView->_setPoint(pcrfm->getPosition());

	if (pView)
	{
		pView->_resetSelection();
//		if(!isHdrFtr())
//			pView->notifyListeners(AV_CHG_FMTCHAR);
	}

	_assertRunListIntegrity();
	setNeedsReformat(blockOffset);
	return true;
}

bool
fl_BlockLayout::doclistener_deleteFmtMark(const PX_ChangeRecord_FmtMark* pcrfm)
{
	_assertRunListIntegrity();

	PT_BlockOffset blockOffset = pcrfm->getBlockOffset();

	xxx_UT_DEBUGMSG(("Edit:DeleteFmtMark: [blockOffset %ld]\n",blockOffset));

	// we can't use the regular _delete() since we are of length zero
	_deleteFmtMark(blockOffset);

	// TODO is it necessary to force a reformat when deleting a FmtMark
	setNeedsReformat(blockOffset);
	updateEnclosingBlockIfNeeded();

	FV_View* pView = getView();
	PT_DocPosition posEOD =0;
	m_pDoc->getBounds(true,posEOD);
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->_resetSelection();
		if(posEOD >= pcrfm->getPosition())
		{
			pView->_setPoint(pcrfm->getPosition());
//			if(!isHdrFtr())
//				pView->notifyListeners(AV_CHG_FMTCHAR);
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}

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
			if(!isHdrFtr())
			{
				pRun->clearScreen();
			}
			break;
		}

		pRun = pRun->getNext();
	}

	// We need a reformat for blocks that only contain a format mark.
	// ie. no next just a carrige return.
	setNeedsReformat(blockOffset);
	updateEnclosingBlockIfNeeded();
	FV_View* pView = getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
//		if(!isHdrFtr())
//			pView->notifyListeners(AV_CHG_FMTCHAR);
	}

	_assertRunListIntegrity();

	return true;
}

/*!
 Recheck ignored words

 For all misspelled words in this run, call the run->drawSquiggle()
 method.
*/
void
fl_BlockLayout::recheckIgnoredWords(void)
{
	// buffer to hold text
	UT_GrowBuf pgb(1024);
	bool bRes = getBlockBuf(&pgb);
	UT_ASSERT(bRes);
	const UT_UCSChar* pBlockText = reinterpret_cast<UT_UCSChar*>(pgb.getPointer(0));

	bool bUpdate = m_pSquiggles->recheckIgnoredWords(pBlockText);

	// Update screen if any words squiggled
	FV_View* pView = getView();
	if (bUpdate && pView)
	{
		pView->updateScreen();
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
	UT_sint32 nlisttype = static_cast<UT_sint32>(iListType);
	if(nlisttype < 0 || nlisttype >= static_cast<UT_sint32>(NOT_A_LIST))
		style = static_cast<XML_Char *>(NULL);
	else
	{
		fl_AutoLists al;
		style = const_cast<XML_Char *>(al.getXmlList(nlisttype));
	}
	return style;
}

List_Type fl_BlockLayout::getListTypeFromStyle( const XML_Char* style)
{
	List_Type lType = NOT_A_LIST;
	UT_uint32 j;
	fl_AutoLists al;
	UT_uint32 size_xml_lists = al.getXmlListsSize();
	for(j=0; j < size_xml_lists; j++)
	{
		if( UT_XML_strcmp(style,al.getXmlList(j))==0)
			break;
	}
	if(j < size_xml_lists)
		lType = static_cast<List_Type>(j);
	return lType;
}


char *	fl_BlockLayout::getFormatFromListType( List_Type iListType)
{
	UT_sint32 nlisttype = static_cast<UT_sint32>(iListType);
	char * format = NULL;
	if(nlisttype < 0 || nlisttype >= static_cast<UT_sint32>(NOT_A_LIST))
		return format;
	fl_AutoLists al;
	format = const_cast<char *>(al.getFmtList(nlisttype));
	return format;
}

List_Type fl_BlockLayout::decodeListType(char * listformat)
{
	List_Type iType = NOT_A_LIST;
	UT_uint32 j;
	fl_AutoLists al;
	UT_uint32 size_fmt_lists = al.getFmtListsSize();
	for(j=0; j < size_fmt_lists; j++)
	{
		if( strstr(listformat,al.getFmtList(j))!=NULL)
			break;
	}
	if(j < size_fmt_lists)
		iType = static_cast<List_Type>(j);
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
		FV_View* pView = getView();
		UT_ASSERT(pView);

		m_pDoc->beginUserAtomicGlob();

		UT_uint32 currLevel = getLevel();
		UT_ASSERT(currLevel > 0);
		currLevel =0; // was currLevel--;
		sprintf(buf, "%i", currLevel);
		setStopping(false);
		fl_BlockLayout * pNext = static_cast<fl_BlockLayout *>(getNext());
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
			props = static_cast<const XML_Char **>(UT_calloc(countp, sizeof(XML_Char *)));
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
					props[i] = static_cast<XML_Char *>(vp.getNthItem(i));
				}
			}
			props[i] = static_cast<XML_Char *>(NULL);

		}
		else
		{
			getListPropertyVector( &vp);
			UT_uint32 countp = vp.getItemCount() + 1;
			UT_uint32 i;
			props = static_cast<const XML_Char **>(UT_calloc(countp, sizeof(XML_Char *)));
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
					props[i] = static_cast<XML_Char *>(vp.getNthItem(i));
				}
			}
			props[i] = static_cast<XML_Char *>(NULL);
		}
		if (currLevel == 0)
		{
#ifndef __MRC__
			const XML_Char * attribs[] = {	"listid", lid,
										"level", buf, NULL, NULL };
#else
			const XML_Char * attribs[] = {	"listid", NULL,
											"level", NULL, NULL, NULL };
			attribs [1] = lid;
			attribs [3] = buf;
#endif
			bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), attribs, props, PTX_Block);
			m_bListItem = false;
		}
		else
		{
#ifndef __MRC__
			const XML_Char * attribs[] = {	"listid", lid,
											"level", buf,NULL,NULL };
#else
			const XML_Char * attribs[] = {	"listid", NULL,
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
		FREEP(props);
	}
}

/*!
 * Start a list with the paragraph definition container in the style defined by "style"
\params const XML_CHar * style the name of the paragraph style for this block.
*/
void	fl_BlockLayout::StartList( const XML_Char * style, PL_StruxDocHandle prevSDH)
{
	//
	// Starts a new list at the current block with list style style all other
	// attributes and properties are the default values
	//
	List_Type lType;
	PD_Style* pStyle = 0;
	const XML_Char* szDelim     = 0;
	const XML_Char* szDec       = 0;
	const XML_Char* szStart     = 0;
	const XML_Char* szAlign     = 0;
	const XML_Char* szIndent    = 0;;
	const XML_Char* szFont      = 0;
	const XML_Char* szListStyle = 0;;
	UT_uint32 startv, level, currID;

	// TODO -- this mixture of float and double is a mess, we should
	// either use double throughout or float
	float fAlign, fIndent;

	m_pDoc->getStyle(static_cast<const char *>(style), &pStyle);
	if (pStyle)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Found list of style %s \n",style));
		// Use the props in the style
		pStyle->getProperty(static_cast<const XML_Char *>("list-delim"), szDelim);
		pStyle->getProperty(static_cast<const XML_Char *>("list-decimal"), szDec);
		pStyle->getProperty(static_cast<const XML_Char *>("start-value"), szStart);

		if(m_iDomDirection == FRIBIDI_TYPE_RTL)
		   pStyle->getProperty(static_cast<const XML_Char *>("margin-right"), szAlign);
	    else
		   pStyle->getProperty(static_cast<const XML_Char *>("margin-left"), szAlign);

		pStyle->getProperty(static_cast<const XML_Char *>("text-indent"), szIndent);
		pStyle->getProperty(static_cast<const XML_Char *>("field-font"), szFont);
		pStyle->getProperty(static_cast<const XML_Char *>("list-style"), szListStyle);
		if (szStart)
			startv = atoi(szStart);
		else
			startv = 1;

		if (szAlign)
			fAlign = static_cast<float>(UT_convertToInches(szAlign));
		else
			fAlign = static_cast<float>(LIST_DEFAULT_INDENT);
		if (szIndent)
			fIndent = static_cast<float>(UT_convertToInches(szIndent));
		else
			fIndent =  static_cast<float>(-LIST_DEFAULT_INDENT_LABEL);
		if(!szFont)
			UT_ASSERT(0);

		double dLeft;
		if(m_iDomDirection == FRIBIDI_TYPE_LTR)
			dLeft = UT_convertToInches(getProperty("margin-left",true));
		else
			dLeft = UT_convertToInches(getProperty("margin-right",true));

		fAlign += static_cast<float>(dLeft);
	}
	else
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Could NOT find list of style %s \n",style));
		szDelim = "%L";
		startv = 1;
		szDec = ".";
		fAlign = static_cast<float>(LIST_DEFAULT_INDENT);
		fIndent = static_cast<float>(-LIST_DEFAULT_INDENT_LABEL);
	}

	UT_uint32 count = m_pDoc->getListsCount();
	UT_uint32 j = 0;
	bool bFound = false;
	fl_AutoNum * pPrev  = NULL;
	if(prevSDH)
	{
		for(j=0; j< count && !bFound; j++)
		{
			pPrev = m_pDoc->getNthList(j);
			if(pPrev->isItem(prevSDH))
			{
				bFound = true;
			}
		}
	}
	if(prevSDH == NULL || !bFound)
	{
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
		fAlign *= static_cast<float>(level);
	}
	else
	{
		currID = pPrev->getID();
		level = pPrev->getLevel();
		level++;
	}

	lType = getListTypeFromStyle(szListStyle);
	StartList( lType, startv,szDelim, szDec, szFont, fAlign, fIndent, currID,level);
}

void	fl_BlockLayout::getListAttributesVector( UT_Vector * va)
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
	pBlockAP->getAttribute(static_cast<const XML_Char *>(PT_LISTID_ATTRIBUTE_NAME),lid);
	level = getAutoNum()->getLevel();
	sprintf(buf,"%i",level);
	//	pBlockAP->getAttribute("level",buf);
	if(lid != NULL)
	{
		va->addItem( (void *)("listid"));  va->addItem( (void *)(lid));
		count++;
	}
	if(buf != NULL)
	{
		va->addItem( (void *)("level")); va->addItem( (void *)(buf));
		count++;
	}
	if(style != NULL)
	{
		va->addItem( (void *)(PT_STYLE_ATTRIBUTE_NAME));	va->addItem( (void *)(style));
		count++;
	}
	if(count == 0)
	{
		va->addItem( NULL);
	}
}


void	fl_BlockLayout::getListPropertyVector( UT_Vector * vp)
{
	//
	// This function fills the vector vp with list properties. All vector
	// quantities are const XML_Char *
	//
	UT_uint32 count=0;
	const XML_Char * pszStart = getProperty("start-value",true);
	const XML_Char * lDelim =  getProperty("list-delim",true);
	const XML_Char * lDecimal =  getProperty("list-decimal",true);

	const XML_Char * pszAlign;
	if(m_iDomDirection == FRIBIDI_TYPE_RTL)
		pszAlign =  getProperty("margin-right",true);
	else
		pszAlign =  getProperty("margin-left",true);

	const XML_Char * pszIndent =  getProperty("text-indent",true);
	const XML_Char * fFont =  getProperty("field-font",true);
	const XML_Char * pszListStyle =  getProperty("list-style",true);
	if(pszStart != NULL)
	{
		vp->addItem( (void *)("start-value"));	vp->addItem( (void *)(pszStart));
	}
	if(pszAlign != NULL)
	{
		if(m_iDomDirection == FRIBIDI_TYPE_RTL)
			vp->addItem( (void *)("margin-right"));
		else
			vp->addItem( (void *)("margin-left"));

		vp->addItem( (void *)(pszAlign));

		count++;
	}
	if(pszIndent != NULL)
	{
		vp->addItem( (void *)("text-indent"));	vp->addItem( (void *)(pszIndent));
		count++;
	}
	if(lDelim != NULL)
	{
		vp->addItem( (void *)("list-delim")); vp->addItem( (void *)(lDelim));
		count++;
	}
	if(lDecimal != NULL)
	{
		vp->addItem( (void *)("list-decimal")); vp->addItem( (void *)(lDecimal));
		count++;
	}
	if(fFont != NULL)
	{
		vp->addItem( (void *)("field-font")); vp->addItem( (void *)(fFont));
		count++;
	}
	if(pszListStyle != NULL)
	{
		vp->addItem( (void *)("list-style")); vp->addItem( (void *)(pszListStyle));
		count++;
	}
	if(count == 0)
	{
		vp->addItem( NULL);
	}
}


void	fl_BlockLayout::StartList( List_Type lType, UT_uint32 start,const XML_Char * lDelim, const XML_Char * lDecimal, const XML_Char * fFont, float Align, float indent, UT_uint32 iParentID, UT_uint32 curlevel )
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

	FV_View* pView = getView();
	UT_ASSERT(pView);

	pAutoNum = m_pDoc->getListByID(id);
	xxx_UT_DEBUGMSG(("SEVIOR: found autonum %x from id %d \n",pAutoNum,id));
	if(pAutoNum != NULL)
	{
		m_pAutoNum = pAutoNum;
		m_bListItem = true;
		listUpdate();
	}
	id = UT_rand();
	while(id < AUTO_LIST_RESERVED)
		id = UT_rand();
	sprintf(lid, "%i", id);

	sprintf(pid, "%i", iParentID);
	sprintf(buf, "%i", curlevel);
	sprintf(pszStart,"%i",start);

	UT_XML_strncpy( pszAlign,
					sizeof(pszAlign),
					UT_convertInchesToDimensionString(DIM_IN, Align, 0));

	UT_XML_strncpy( pszIndent,
					sizeof(pszIndent),
					UT_convertInchesToDimensionString(DIM_IN, indent, 0));

	va.addItem( (void *)("listid")); 		va.addItem( (void *)(lid));
	va.addItem( (void *)("parentid"));		va.addItem( (void *)(pid));
	va.addItem( (void *)("level"));			va.addItem( (void *)(buf));
	vp.addItem( (void *)("start-value"));	vp.addItem( (void *)(pszStart));

	if(m_iDomDirection == FRIBIDI_TYPE_RTL)
		vp.addItem( (void *)("margin-right"));
	else
	    vp.addItem( (void *)("margin-left"));

	vp.addItem( (void *)(pszAlign));

	vp.addItem( (void *)("text-indent"));	vp.addItem( (void *)(pszIndent));
	vp.addItem( (void *)("field-font")); 	vp.addItem( (void *)(fFont));
	vp.addItem( (void *)("list-style")); 	vp.addItem( (void *)(style));
	xxx_UT_DEBUGMSG(("SEVIOR: Starting List with font %s \n",fFont));


	pAutoNum = new fl_AutoNum(id, iParentID, lType, start, lDelim, lDecimal, m_pDoc);
	if (!pAutoNum)
	{
		// TODO Out of Mem.
	}
	m_pDoc->addList(pAutoNum);
	pAutoNum->fixHierarchy();

	UT_uint32 counta = va.getItemCount() + 1;
	UT_uint32 countp = vp.getItemCount() + 1;
	UT_uint32 i;
	const XML_Char ** attribs = static_cast<const XML_Char **>(UT_calloc(counta, sizeof(XML_Char *)));
	for(i=0; i<va.getItemCount();i++)
	{
		attribs[i] = static_cast<XML_Char *>(va.getNthItem(i));
	}
	attribs[i] = static_cast<XML_Char *>(NULL);

	const XML_Char ** props = static_cast<const XML_Char **>(UT_calloc(countp, sizeof(XML_Char *)));
	for(i=0; i<vp.getItemCount();i++)
	{
		props[i] = static_cast<XML_Char *>(vp.getNthItem(i));
	}
	props[i] = static_cast<XML_Char *>(NULL);
	setStarting( false);
	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), attribs, props, PTX_Block);

	//	pView->_generalUpdate();
	m_pDoc->listUpdate(getStruxDocHandle());
	pView->_generalUpdate();
	pView->_ensureInsertionPointOnScreen();
	FREEP(attribs);
	FREEP(props);
}


void	fl_BlockLayout::StopListInBlock(void)
{
	//
	// Stops the list in the current block
	//
	static XML_Char lid[15],pszlevel[5];
	bool bRet;
	UT_uint32 id, level;
	UT_Vector vp;
	FV_View* pView = getView();
	UT_ASSERT(pView);
	bool bHasStopped = m_pDoc->hasListStopped();
	if(getAutoNum()== NULL || bHasStopped)
	{
		return; // this block has already been processed
	}
	m_pDoc->setHasListStopped(true);
	PT_DocPosition offset = pView->getPoint() - getPosition();
	fl_BlockLayout * pPrev, * pNext;

	if (getAutoNum()->getParent())
	{
		id = getAutoNum()->getParent()->getID();
		level = getAutoNum()->getParent()->getLevel();
	}
	else
	{
		id = 0;
		level = 0;
	}

	sprintf(lid, "%i", id);

	setStopping(false);
	//
	// Set formatting to match the next paragraph if it exists
	//
	const XML_Char ** props = NULL;
	const XML_Char * szAlign, * szIndent;
	pPrev = static_cast<fl_BlockLayout *>(getPrev());
	pNext = static_cast<fl_BlockLayout *>(getNext());

	if (id != 0)
	{
		UT_ASSERT(pPrev);	// TMN: Is an assert appropriate here?

		//First, look for block in list
		bool bmatch = false;
		bmatch = static_cast<bool>(pPrev->isListItem() && pPrev->getLevel() == level && pPrev->getAutoNum()->getID() == id);
		while (pPrev && !bmatch)
		{
			pPrev = static_cast<fl_BlockLayout *>(pPrev->getPrev());
			if (pPrev && pPrev->isListItem())
				bmatch = static_cast<bool>(pPrev->getLevel() == level
								&& pPrev->getAutoNum()->getID() == id);
		}
		while (pNext  && !bmatch)
		{
			pNext = static_cast<fl_BlockLayout *>(pNext->getNext());
			if (pNext && pNext->isListItem())
				bmatch = static_cast<bool>(pNext->getLevel() == level
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
			m_pDoc->getStyle(static_cast<char *>(getListStyleString(newType)), &pStyle);
			if (pStyle)
			{
				if(m_iDomDirection == FRIBIDI_TYPE_RTL)
					pStyle->getProperty(static_cast<const XML_Char *>("margin-right"), szAlign);
				else
					pStyle->getProperty(static_cast<const XML_Char *>("margin-left"), szAlign);

				pStyle->getProperty(static_cast<const XML_Char *>("text-indent"), szIndent);
				fAlign = static_cast<float>(UT_convertToInches(szAlign));
				fAlign *= level;
				UT_XML_strncpy( align,
								sizeof(align),
								UT_convertInchesToDimensionString(DIM_IN, fAlign, 0));
				sprintf(indent, "%s", szIndent);
			}
			else
			{
				fAlign =  static_cast<float>(LIST_DEFAULT_INDENT) * level;
				fIndent = static_cast<float>(-LIST_DEFAULT_INDENT_LABEL);
				UT_XML_strncpy( align,
								sizeof(align),
								UT_convertInchesToDimensionString(DIM_IN, fAlign, 0));
				UT_XML_strncpy( indent,
								sizeof(indent),
								UT_convertInchesToDimensionString(DIM_IN, fIndent, 0));
			}

			if(m_iDomDirection == FRIBIDI_TYPE_RTL)
				vp.addItem("margin-right");
			else
				vp.addItem("margin-left");

			vp.addItem((void *)(align));
			vp.addItem("text-indent");
			vp.addItem((void *)(indent));
		}
	}
	else
	{
		// Find the last non-list item and set alignment + indent
		while (pPrev && pPrev->isListItem())
			pPrev = static_cast<fl_BlockLayout *>(pPrev->getPrev());

		while (pNext && pNext->isListItem())
			pNext = static_cast<fl_BlockLayout *>(pNext->getNext());

		if (pPrev)
		{
			if(m_iDomDirection == FRIBIDI_TYPE_RTL)
				szAlign = pPrev->getProperty("margin-right", true);
			else
				szAlign = pPrev->getProperty("margin-left", true);

			szIndent =	pPrev->getProperty("text-indent", true);
		}
		else if (pNext)
		{
			if(m_iDomDirection == FRIBIDI_TYPE_RTL)
				szAlign = pNext->getProperty("margin-right", true);
			else
				szAlign = pNext->getProperty("margin-left", true);

			szIndent = pNext->getProperty("text-indent", true);
		}
		else
		{
			szAlign = "0.0000in";
			szIndent = "0.0000in";
		}

		if(m_iDomDirection == FRIBIDI_TYPE_RTL)
			vp.addItem((void *)("margin-right"));
		else
			vp.addItem((void *)("margin-left"));

		vp.addItem((void *)(static_cast<const void *>(szAlign)));
		vp.addItem((void *)("text-indent"));
		vp.addItem((void *)(static_cast<const void *>(szIndent)));
	}
	UT_uint32 countp = vp.getItemCount() + 1;
	UT_uint32 i;
	props = static_cast<const XML_Char **>(UT_calloc(countp, sizeof(XML_Char *)));
	for (i = 0; i < vp.getItemCount(); i++)
	{
		props[i] = static_cast<XML_Char *>(vp.getNthItem(i));
	}
	props[i] = NULL;
	sprintf(pszlevel, "%i", level);

	if (id == 0)
	{
		const XML_Char * pListAttrs[10];
		pListAttrs[0] = "listid";
		pListAttrs[1] = NULL;
		pListAttrs[2] = "parentid";
		pListAttrs[3] = NULL;
		pListAttrs[4] = "level";
		pListAttrs[5] = NULL;
		pListAttrs[6] = "type";
		pListAttrs[7] = NULL;
		pListAttrs[8] = NULL;
		pListAttrs[9] = NULL;

		// we also need to explicitely clear the list formating
		// properties, since their values are not necessarily part
		// of the style definition, so that cloneWithEliminationIfEqual
		// which we call later will not get rid off them
		const XML_Char * pListProps[20];
		pListProps[0] =  "start-value";
		pListProps[1] =  NULL;
		pListProps[2] =  "list-style";
		pListProps[3] =  NULL;

		if(m_iDomDirection == FRIBIDI_TYPE_RTL)
			pListProps[4] =  "margin-right";
		else
			pListProps[4] =  "margin-left";

		pListProps[5] =  NULL;
		pListProps[6] =  "text-indent";
		pListProps[7] =  NULL;
		pListProps[8] =  "field-color";
		pListProps[9] =  NULL;
		pListProps[10]=  "list-delim";
		pListProps[11] =  NULL;
		pListProps[12]=  "field-font";
		pListProps[13] =  NULL;
		pListProps[14]=  "list-decimal";
		pListProps[15] =  NULL;
		pListProps[16] =  "list-tag";
		pListProps[17] =  NULL;
		pListProps[18] =  NULL;
		pListProps[19] =  NULL;
//
// Remove all the list related properties
//
		bRet = m_pDoc->changeStruxFmt(PTC_RemoveFmt, getPosition(), getPosition(), pListAttrs, pListProps, PTX_Block);
		fp_Run * pRun = getFirstRun();
		while(pRun->getNext())
		{
			pRun = pRun->getNext();
		}
		PT_DocPosition lastPos = getPosition() + pRun->getBlockOffset();
		bRet = m_pDoc->changeSpanFmt(PTC_RemoveFmt, getPosition(false), lastPos, pListAttrs, pListProps);
//
// Set the indents to match.
//
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), NULL, props, PTX_Block);
		m_bListItem = false;
	}
	else
	{
		const XML_Char * attribs[] = {	"listid", NULL,"level",NULL, NULL,NULL };
		attribs [1] = lid;
		attribs [3] = pszlevel;
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,getPosition(), getPosition(), attribs, props, PTX_Block);
		m_pDoc->listUpdate(getStruxDocHandle());
	}
	// format();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		if(offset > 0 )
			pView->_setPoint(pView->getPoint()+offset-2);
	}
	if(m_pDoc->areListUpdatesAllowed())
	{
		pView->_generalUpdate();
	}
	pView->_ensureInsertionPointOnScreen();
	FREEP(props);
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
	fl_BlockLayout * pPrev = static_cast<fl_BlockLayout *>(getPrev());
	bool bmatchid =  false;
	fl_AutoNum * pAutoNum = NULL;

	if (pPrev != NULL && (pPrev->getAutoNum() != NULL) && pPrev->isListItem())
	{
		bmatchid = static_cast<bool>(id == pPrev->getAutoNum()->getID());
		if (pPrev->isFirstInList() && !bmatchid)
		{
			pAutoNum = pPrev->getAutoNum()->getParent();
			while (pAutoNum && !bmatchid)
			{
				bmatchid = static_cast<bool>(id == pAutoNum->getID()
								   && pAutoNum->isItem(pPrev->getStruxDocHandle()));
				pAutoNum = pAutoNum->getParent();
			}
		}
	}

	while (pPrev != NULL && bmatchid == false)
	{
		pPrev = static_cast<fl_BlockLayout *>(pPrev->getPrev());
		if (pPrev && (pPrev->getAutoNum() != NULL) && pPrev->isListItem())
		{
			bmatchid = static_cast<bool>(id == pPrev->getAutoNum()->getID());
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
	fl_BlockLayout * pNext = static_cast<fl_BlockLayout *>(getNext());
	bool bmatchLevel =	false;
	if( pNext != NULL && pNext->isListItem()&& (pNext->getAutoNum() != NULL))
	{
		bmatchLevel = static_cast<bool>(id == pNext->getAutoNum()->getID());
	}
	while(pNext != NULL && bmatchLevel == false)
	{
		pNext = static_cast<fl_BlockLayout *>(pNext->getNext());
		if( pNext != NULL && pNext->isListItem() && (pNext->getAutoNum() != NULL))
		{
			bmatchLevel = static_cast<bool>(id == pNext->getAutoNum()->getID());
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
	fl_BlockLayout * pPrev = static_cast<fl_BlockLayout *>(getPrev());
	while(pPrev != NULL && !pPrev->isListItem())
	{
		pPrev = static_cast<fl_BlockLayout *>(pPrev->getPrev() );
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

    const char * szAlign;
	if(m_iDomDirection == FRIBIDI_TYPE_RTL)
		szAlign = getProperty("margin-right",true);
	else
		szAlign = getProperty("margin-left",true);

	double dAlignMe = UT_convertToDimension(szAlign,DIM_IN);
	//
	// Find the most recent block with a list
	//
	fl_BlockLayout * pClosest = NULL;
	float dClosest = 100000.;
	bool bFound = false;
	fl_BlockLayout * pPrev = static_cast<fl_BlockLayout *>(getPrev());
	while(pPrev != NULL && !bFound)
	{
		if(pPrev->isListItem())
		{
			if(m_iDomDirection == FRIBIDI_TYPE_RTL)
				szAlign = pPrev->getProperty("margin-right",true);
			else
				szAlign = pPrev->getProperty("margin-left",true);

			double dAlignThis = UT_convertToDimension(szAlign,DIM_IN);
			float diff = static_cast<float>(fabs( static_cast<float>(dAlignThis)-dAlignMe));
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
				pPrev = static_cast<fl_BlockLayout *>(pPrev->getPrev());
			}
		}
		else
		{
			pPrev = static_cast<fl_BlockLayout *>(pPrev->getPrev());
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
	const XML_Char ** attribs = static_cast<const XML_Char **>(UT_calloc(counta, sizeof(XML_Char *)));
	for(i=0; i<va.getItemCount();i++)
	{
		attribs[i] = static_cast<XML_Char *>(va.getNthItem(i));
	}
	attribs[i] = static_cast<XML_Char *>(NULL);

	const XML_Char ** props = static_cast<const XML_Char **>(UT_calloc(countp, sizeof(XML_Char *)));
	for(i=0; i<vp.getItemCount();i++)
	{
		props[i] = static_cast<XML_Char *>(vp.getNthItem(i));
	}
	props[i] = static_cast<XML_Char *>(NULL);
	m_bStartList =	false;
	m_bStopList = false;
	FV_View* pView = getView();
	UT_ASSERT(pView);
	m_bListLabelCreated = false;
	m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), attribs, props, PTX_Block);
	m_bListItem = true;
	m_pDoc->listUpdate(getStruxDocHandle());
	pView->_generalUpdate();
	FREEP(attribs);
	FREEP(props);
}

void  fl_BlockLayout::resumeList( fl_BlockLayout * prevList)
{
	//
	// Make the current block the next element of the list in the block prevList
	//
	UT_ASSERT(prevList);
	UT_Vector va,vp;
//
// Defensive code. This should not happen
//
	UT_ASSERT(prevList->getAutoNum());
	if(prevList->getAutoNum() == NULL)
		return;
	prevList->getListPropertyVector( &vp);
	prevList->getListAttributesVector( &va);
	UT_uint32 counta = va.getItemCount() + 1;
	UT_uint32 countp = vp.getItemCount() + 1;
	UT_uint32 i;
	const XML_Char ** attribs = static_cast<const XML_Char **>(UT_calloc(counta, sizeof(XML_Char *)));
	for(i=0; i<va.getItemCount();i++)
	{
		attribs[i] = static_cast<XML_Char *>(va.getNthItem(i));
	}
	attribs[i] = static_cast<XML_Char *>(NULL);

	const XML_Char ** props = static_cast<const XML_Char **>(UT_calloc(countp, sizeof(XML_Char *)));
	for(i=0; i<vp.getItemCount();i++)
	{
		props[i] = static_cast<XML_Char *>(vp.getNthItem(i));
	}
	props[i] = static_cast<XML_Char *>(NULL);
	m_bStartList =	false;
	m_bStopList = false;
	FV_View* pView = getView();
	UT_ASSERT(pView);
	m_bListLabelCreated = false;
	m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), attribs, props, PTX_Block);
	m_bListItem = true;
	m_pDoc->listUpdate(getStruxDocHandle());
	pView->_generalUpdate();
	FREEP(attribs);
	FREEP(props);
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

	FV_View* pView = getView();
	if (pView)
	{
		//		pView->_generalUpdate();
		pView->_fixInsertionPointCoords();
		pView->updateScreen();
	}

}

void fl_BlockLayout::transferListFlags(void)
{
	//
	// Transfer list flags from a block to the following list blocks
	//
	UT_ASSERT(getNext());
	if(getNext() == NULL)
	{
		return;
	}
	if(getNext()->getContainerType() != FL_CONTAINER_BLOCK)
	{
		return;
	}
	if (static_cast<fl_BlockLayout *>(getNext())->isListItem()) // this is wrong. It should be next in the list.
	{
		UT_uint32 nId = getNext()->getAutoNum()->getID();
		UT_uint32 cId=0, pId=0;
		fl_BlockLayout * pPrev = getPreviousList();
		if(pPrev != NULL)
			pId = pPrev->getAutoNum()->getID();
		if(isListItem())
			cId = getAutoNum()->getID();
		if(cId == nId)
		{
			if (!static_cast<fl_BlockLayout *>(getNext())->m_bStartList)
				static_cast<fl_BlockLayout *>(getNext())->m_bStartList = m_bStartList;
			if (!static_cast<fl_BlockLayout *>(getNext())->m_bStopList)
				static_cast<fl_BlockLayout *>(getNext())->m_bStopList = m_bStopList;
		}
		else if ( pId == nId)
		{
			if (!static_cast<fl_BlockLayout *>(getNext())->m_bStartList)
				static_cast<fl_BlockLayout *>(getNext())->m_bStartList = pPrev->m_bStartList;
			if (!static_cast<fl_BlockLayout *>(getNext())->m_bStopList)
				static_cast<fl_BlockLayout *>(getNext())->m_bStopList = pPrev->m_bStopList;
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
		return static_cast<bool>(sdh == m_pAutoNum->getFirstItem());
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

	FV_View* pView = getView();
	PT_DocPosition offset =0;
	if(pView)
	{
		offset = pView->getPoint() - getPosition();
	}
#if 1
	const  XML_Char ** blockatt;
	pView->getCharFormat(&blockatt,true,getPosition());
//	pView->setBlockFormat(blockatt);
//	FREEP(blockatt);
#endif
#if 1
	XML_Char * tagatt[3] = {"list-tag",NULL,NULL};
	XML_Char tagID[12];
	UT_uint32 itag = UT_rand();
	while( itag < 10000)
	{
		itag = UT_rand();
	}
	sprintf(tagID,"%d",itag);
	tagatt[1] = static_cast<XML_Char *>(&tagID[0]);
	m_pDoc->changeSpanFmt(PTC_AddFmt,getPosition(),getPosition(),NULL,const_cast<const XML_Char **>(tagatt));
#endif

	const XML_Char* attributes[] = {
		"type","list_label",
		NULL, NULL
	};
	bool bResult = m_pDoc->insertObject(getPosition(), PTO_Field, attributes, NULL);
	//	pView->_generalUpdate();
	PT_DocPosition diff = 1;
	if(m_pDoc->isDoingPaste() == false)
	{
		UT_UCSChar c = UCS_TAB;
		bResult = m_pDoc->insertSpan(getPosition()+1,&c,1);
		diff = 2;
	}
//
// I don't think we need this.
// We definately need this to preserve attributes on new list lines
//
//
//	UT_uint32 i =0;
//  	while(blockatt[i] != NULL)
//  	{
//  		UT_DEBUGMSG(("SEVIOR: Applying blockatt[i] %s at %d %d \n",blockatt[i],getPosition(),getPosition()+diff));
//  		i++;
//  	}
  	m_pDoc->changeSpanFmt(PTC_AddFmt,getPosition(),getPosition()+diff,NULL,static_cast<const char **>(blockatt));
	FREEP(blockatt);

	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->_setPoint(pView->getPoint()+offset);
	}
	pView->_generalUpdate();

	pView->_ensureInsertionPointOnScreen();
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
			fp_FieldRun * pFRun = static_cast<fp_FieldRun *>(pRun);
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

		UT_uint32 iRealDeleteCount;
		pDoc->deleteSpan(posBlock+ioffset, posBlock+ioffset + npos,NULL,iRealDeleteCount);
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

#if 0
// this is a start of work on a fix bugs related to non-sensical margins
// but it is in very early stages
void fl_BlockLayout::setTextIndent(UT_sint32 iInd)
{
	GR_Graphics* pG = m_pLayout->getGraphics();
	const UT_sint32 Screen_resolution = pG->getResolution();

	m_iTextIndent = iInd;
	double dInches = iInd / Screen_resolution;

	const char * szProp = getProperty("text-indent", true);

	char buf[50];
	UT_Dimension dim = UT_determineDimension(szProp, DIM_IN);

	strcpy(buf, UT_convertInchesToDimensionString(dim, dInches));

	const XML_Char ** props[] =
	{
		"text-indent", buf,NULL,NULL
	};

	FV_View * pView = getView();
	pView->setBlockFormat(props);
}
#endif

void fl_BlockLayout::setDominantDirection(FriBidiCharType iDirection)
{
	m_iDomDirection = iDirection;
	XML_Char * prop[] = {NULL, NULL, 0};
	XML_Char   ddir[] = "dom-dir";
	XML_Char   rtl[]  = "rtl";
	XML_Char   ltr[]  = "ltr";

	prop[0] = static_cast<XML_Char *>(&ddir[0]);

	if(m_iDomDirection == FRIBIDI_TYPE_RTL)
	{
		prop[1] = static_cast<XML_Char *>(&rtl[0]);
	}
	else
	{
		prop[1] = static_cast<XML_Char *>(&ltr[0]);
	}

	PT_DocPosition offset = getPosition();
	PT_DocPosition offset2 = offset;
	//NB The casts in the following call are really necessary, it refuses to compile without them. #TF
	getDocument()->changeStruxFmt(static_cast<PTChangeFmt>(PTC_AddFmt),
								  offset, offset2,
								  static_cast<const XML_Char **>(NULL),
								  const_cast<const XML_Char **>(prop),
								  static_cast<PTStruxType>(PTX_Block));
	UT_DEBUGMSG(("Block::setDominantDirection: offset=%d\n", offset));
}

/*!
 Squiggle block being checked (for debugging)

 Trivial background checker which puts on and takes off squiggles from
 the entire block that's being checked.  This sort of messes up the
 spelling squiggles, but it's just a debug thing anyhow.  Enable it by
 setting a preference DebugFlash="1"
*/
void
fl_BlockLayout::debugFlashing(void)
{
#if 0
	xxx_UT_DEBUGMSG(("fl_BlockLayout::debugFlashing() was called\n"));

	UT_GrowBuf pgb(1024);
	bool bRes = getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	UT_uint32 eor = pgb.getLength(); // end of region
	FV_View* pView = getView();

	fl_PartOfBlock* pPOB = new fl_PartOfBlock(0, eor);
	UT_ASSERT(pPOB);
	if (pPOB) {
		m_pSquiggles->add(pPOB);
		m_pSquiggles->clear(pPOB);

		pView->updateScreen();
		UT_usleep(250000);

		//_deleteSquiggles(0, eor);

		pView->updateScreen();
	}

	pView->updateScreen();
#endif
}


fp_Run* fl_BlockLayout::findRunAtOffset(UT_uint32 offset) const
{
	fp_Run * pRun = getFirstRun();
	UT_return_val_if_fail(pRun, NULL);

	fp_Run * pRunResult = NULL;

	while (pRun)
	{
		if(    pRun->getBlockOffset() <= offset
		   && (pRun->getBlockOffset() + pRun->getLength()) > offset )
		{
			pRunResult = pRun;
			break;
		}
		
		pRun = pRun->getNext();
	}

	return pRunResult;
}



/*!
  Constructor for iterator
  
  Use the iterator to find words for spell-checking in the block.

  \param pBL BlockLayout this iterator should work on
  \param iPos Position the iterator should start from
*/
fl_BlockSpellIterator::fl_BlockSpellIterator(fl_BlockLayout* pBL, UT_sint32 iPos)
	: m_pBL(pBL), m_iWordOffset(iPos), m_iStartIndex(iPos), m_iPrevStartIndex(iPos),
	  m_pMutatedString(NULL),
	  m_iSentenceStart(0), m_iSentenceEnd(0)
{
	m_pgb = new UT_GrowBuf(1024);
	bool bRes = pBL->getBlockBuf(m_pgb);
	UT_ASSERT(bRes);
	m_pText = reinterpret_cast<UT_UCS4Char*>(m_pgb->getPointer(0));
	m_iLength = m_pgb->getLength();
}

/*!
  Destructor for iterator
*/
fl_BlockSpellIterator::~fl_BlockSpellIterator()
{
	DELETEP(m_pgb);
	FREEP(m_pMutatedString);
}

/*!
  Get length of the block text
  \return Length of the block
*/
UT_sint32
fl_BlockSpellIterator::getBlockLength(void)
{
	return m_iLength;
}

/*!
  Update block information for this iterator

  This method must be called whenever the block this iterator is
  associated with changes.
*/
void
fl_BlockSpellIterator::updateBlock(void)
{
	m_pgb->truncate(0);
	bool bRes = m_pBL->getBlockBuf(m_pgb);
	UT_ASSERT(bRes);
	m_pText = reinterpret_cast<UT_UCS4Char*>(m_pgb->getPointer(0));

	UT_sint32 iNewLen = m_pgb->getLength();
	if (iNewLen <= m_iStartIndex)
	{
		m_iStartIndex = iNewLen;
		m_iPrevStartIndex = iNewLen;
	}

	m_iLength = iNewLen;


	m_iWordOffset = 0;
	m_iWordLength = 0;
}


/*!
  Returns next word for spell checking in block

  The method finds the next word in the block for spell checking. It
  takes care of ignoring words as per user configuration and speller
  limitations. It also makes necessary tweaks to the word (such as
  right-quote to ASCII-quote conversion).

  If the block is changed between calls to this method, the
  updateBlock method must be called.

  \result pWord Pointer to word. 
  \result iLength Length of word.
  \result iBlockPost The word's position in the block
  \return True if word was found, false otherwise.
*/
bool
fl_BlockSpellIterator::nextWordForSpellChecking(const UT_UCSChar*& pWord, UT_sint32& iLength,
												UT_sint32& iBlockPos)
{
	// For empty blocks, there will be no buffer
	if (NULL == m_pText) return false;

	for (;;) {

		bool bFound = false;
		bool bWordStartFound = false;
		m_iWordOffset = m_iStartIndex;

		// Special case for first character in block - checked
		// seperately to avoid in loop below
		if (0 == m_iWordOffset)
		{
			UT_UCSChar followChar = (((m_iWordOffset + 1) < m_iLength)  
									 ?  m_pText[m_iWordOffset+1] : UCS_UNKPUNK);
			if (!UT_isWordDelimiter( m_pText[m_iWordOffset], followChar, UCS_UNKPUNK))
			{
				bWordStartFound = true;
			}
			else
			{
				m_iWordOffset++;
			}
		}

		// If start of word not found, keep looking (until the last
		// character but one - avoids boundary checks for the
		// followChar argument)
		if (!bWordStartFound) {
			while (m_iWordOffset < (m_iLength-1))
			{
				if (!UT_isWordDelimiter( m_pText[m_iWordOffset], 
										 m_pText[m_iWordOffset+1],
										 m_pText[m_iWordOffset-1]))
				{
					bWordStartFound = true;
					break;
				}
				m_iWordOffset++;
			}
		}

		// No word start has been found. We still have to check the
		// last character in the block, but even if it is a word
		// character, we don't spell-check one-character words, so
		// there's no reason to make the effort. Just exit...
		if (!bWordStartFound) {
			return false;
		}

		// Now we have the starting position of the word in
		// m_iWordOffset.

		// Ignore some initial characters
		if (_ignoreFirstWordCharacter(m_pText[m_iWordOffset]))
        {
			m_iWordOffset++;
        }

		// If we're at the end of the block after ignoring characters,
		// nothing more to do
		if (m_iWordOffset == m_iLength)
        {
            return false;
        }

		// We're at the start of a word. Find end of word while
		// keeping track of numerics and case of letters. Again, only
		// check until the last but one character to avoid followChar
		// boundary checks...
		bool bAllUpperCase = true;
		bool bHasNumeric = false;
		UT_sint32 iWordEnd = m_iWordOffset;
		if (0 == iWordEnd) {
			// Special check for first letter in the block - which can
			// never be a word delimiter (we skipped those in the
			// first loop of this method, remember?) - so juct collect
			// the property data
            bAllUpperCase &= UT_UCS4_isupper(m_pText[iWordEnd]);
            bHasNumeric |= UT_UCS4_isdigit(m_pText[iWordEnd]);

            iWordEnd++;
        }

		while (!bFound && (iWordEnd < (m_iLength-1)))
		{
			if (UT_isWordDelimiter( m_pText[iWordEnd], 
									m_pText[iWordEnd+1],
									m_pText[iWordEnd-1]))
			{
				bFound = true;
			}
			else 
			{
				if (bAllUpperCase)
				{
					// Only check as long as all seen characters have
					// been upper case. Most words will cause
					// bAllUpperCase to go false pretty early, so we
					// can save the lookup...
					bAllUpperCase &= UT_UCS4_isupper(m_pText[iWordEnd]);
				}
				// It's not worth making this lookup conditional:
				// majority of words do not contain digits, so the
				// if-statement will just become an overhead...
				bHasNumeric |= UT_UCS4_isdigit(m_pText[iWordEnd]);

				iWordEnd++;
			}
		}

		// Check last character in block if necessary
		if (!bFound && iWordEnd != m_iLength)
		{
			UT_ASSERT(iWordEnd == (m_iLength-1));
			
			if (UT_isWordDelimiter(m_pText[iWordEnd], 
								   UCS_UNKPUNK,
								   m_pText[iWordEnd-1]))
			{
				bFound = true;
			} 
			else 
			{
				if (bAllUpperCase)
					bAllUpperCase &= UT_UCS4_isupper(m_pText[iWordEnd]);
				bHasNumeric |= UT_UCS4_isdigit(m_pText[iWordEnd]);
				
				iWordEnd++;
			}
		}
		UT_ASSERT(bFound || iWordEnd == m_iLength);

		// This is where we want to start from at next call.
		m_iPrevStartIndex = m_iStartIndex;
		m_iStartIndex = iWordEnd;
		   
		// Find length of word
		UT_uint32 iWordLength = iWordEnd - m_iWordOffset;

		// ignore some terminal characters
		if (_ignoreLastWordCharacter(m_pText[m_iWordOffset + iWordLength - 1]))
		{
			iWordLength--;
		}

		// Ignore words where first character is a digit
		if (UT_UCS4_isdigit(m_pText[m_iWordOffset]))
		{
			continue;
		}

		// Don't check all-UPPERCASE words unless so configured
		if (bAllUpperCase && !m_pBL->getView()->getLayout()->getSpellCheckCaps())
		{
			continue;
		}

		// Don't check words with numbers unless so configured
		if (bHasNumeric && !m_pBL->getView()->getLayout()->getSpellCheckNumbers())
		{
			continue;
		}

		// TODO i18n the CJK stuff here is a hack
		if (!XAP_EncodingManager::get_instance()->noncjk_letters(m_pText+m_iWordOffset, iWordLength))
		{
			continue;
		}


		// These are the current word details
		UT_uint32 iNewLength = iWordLength;
		pWord = &m_pText[m_iWordOffset];

		// Now make any necessary mutations to the word before it is
		// returned. Normal case is that no changes are necessary, so
		// do this in two loops, only executing the second if any
		// mutation is necessary. This means the normal case will not
		// require the allocation+copy of the word.
		FREEP(m_pMutatedString);
		bool bNeedsMutation = false;
		for (UT_uint32 i=0; i < static_cast<UT_uint32>(iWordLength); i++)
		{
			UT_UCSChar currentChar = m_pText[m_iWordOffset + i];
			
			if (currentChar == UCS_ABI_OBJECT || currentChar == UCS_RQUOTE)
			{
				bNeedsMutation = true;
				break;
			}
		}

		if (bNeedsMutation)
		{
			// Generate the mutated word in a new buffer pointed to by m_pMutatedString
			m_pMutatedString = static_cast<UT_UCSChar*>(UT_calloc(iWordLength, sizeof(UT_UCSChar)));
			UT_ASSERT(m_pMutatedString);
			pWord = m_pMutatedString;
			iNewLength = 0;
			for (UT_uint32 i=0; i < static_cast<UT_uint32>(iWordLength); i++)
			{
				UT_UCSChar currentChar = m_pText[m_iWordOffset + i];
			
				// Remove UCS_ABI_OBJECT from the word
				if (currentChar == UCS_ABI_OBJECT) continue;

				// Convert smart quote apostrophe to ASCII single quote to
				// be compatible with ispell
				if (currentChar == UCS_RQUOTE) currentChar = '\'';

				m_pMutatedString[iNewLength++] = currentChar;
			}
		}

		// Ignore one-character words.
		// Note: if this is ever changed to be 2+, the scan for word
		// delimiters at the top must also be changed to check for a word
		// in the last character of the block.
		if (iNewLength <= 1)
		{
			continue;
		}


		// Don't blow ispell's little mind...
		if (INPUTWORDLEN < iNewLength)
		{
			continue;
		}

		
		// OK, we found the word. Feed the length/pos details to the
		// caller...
		iLength = iNewLength;
		iBlockPos = m_iWordOffset;

		// Also remember length of m_pWord
		m_iWordLength = iNewLength;

		// Return success!
		return true;
	}
}

// TODO  This function finds the beginning and end of a sentence enclosing
// TODO  the current misspelled word. Right now, it starts from the word
// TODO  and works forward/backward until finding [.!?] or EOB
// TODO  This needs to be improved badly. However, I can't think of a 
// TODO  algorithm to do so -- especially not one which could work with
// TODO  other languages very well...
// TODO  Anyone have something better?
// TODO  Hipi: ICU includes an international sentence iterator
// TODO  Hipi: Arabic / Hebrew reverse ? should count, Spanish upside-down
// TODO  Hipi: ? should not count.  CJK scripts have their own equivalents
// TODO  Hipi: to [.!?].  Indic languages can use a "danda" or "double danda".
// TODO  Hipi: Unicode chartype functions may be useable

/*!
  Update sentence baoundaries around current word
  Find sentence the current word is in.
*/
void
fl_BlockSpellIterator::updateSentenceBoundaries(void)
{
	UT_sint32 iBlockLength = m_pgb->getLength();

	// If the block is small, don't bother looking for
	// boundaries. Just display the full block.
	if (iBlockLength < 30)
	{
		m_iSentenceStart = 0;
		m_iSentenceEnd = iBlockLength - 1;
		return;
	}

	// Go back from the current word start until a period is found
	m_iSentenceStart = m_iWordOffset;
	while (m_iSentenceStart > 0) {
		if (UT_UCS4_isSentenceSeparator(m_pText[m_iSentenceStart]))
			break;
		m_iSentenceStart--;
	}
   
	// Go forward past any whitespace if sentence start is not at the
	// start of the block
	if (m_iSentenceStart > 0)
	{
		// Seeing as we're not at the start of the block, and the word
		// must contain at least one character, we don't have to make
		// conditional boundary checking (and UCS_UNKPUNK
		// substitution).
		UT_ASSERT(m_iSentenceStart > 0);
		UT_ASSERT(m_iWordLength > 1);

		while (++m_iSentenceStart < m_iWordOffset
			   && UT_isWordDelimiter(m_pText[m_iSentenceStart], 
									 m_pText[m_iSentenceStart+1],
									 m_pText[m_iSentenceStart-1]))
		{
			// Nothing to do... just iterating...
		};

	}


	// Find end of sentence. Go forward until a period is found. If
	// getting to within 10 characters of the end of the block, stop
	// and go with that as the end....
	m_iSentenceEnd = m_iWordOffset + m_iWordLength;
	while (m_iSentenceEnd < (iBlockLength - 10)) {
		if (UT_UCS4_isSentenceSeparator(m_pText[m_iSentenceEnd++]))
			break;
	}
	if (m_iSentenceEnd == (iBlockLength-10)) m_iSentenceEnd = iBlockLength-1;
}

/*!
  Get current word
  \result iLength Length of string.
  \return Pointer to word.
*/
const UT_UCSChar*
fl_BlockSpellIterator::getCurrentWord(UT_sint32& iLength)
{
	iLength = m_iWordLength;

	if (NULL != m_pMutatedString)
	{
		return m_pMutatedString;
	}
	else
	{
		return &m_pText[m_iWordOffset];
	}
}

/*!
  Get part of sentence before current word
  \result iLength Length of string. If 0, NULL will be returned.
  \return Pointer to sentence prior to current word, or NULL
*/
const UT_UCSChar*
fl_BlockSpellIterator::getPreWord(UT_sint32& iLength)
{
	iLength = m_iWordOffset - m_iSentenceStart;

	// If it ever becomes necessary to mutate the pre-word, allocate
	// space to m_pMutatedString and return it. Caller will consume
	// that buffer before calling any other function.

	if (0 >= iLength)
		return NULL;

	return reinterpret_cast<UT_UCSChar*>(m_pgb->getPointer(m_iSentenceStart));
}

/*!
  Get part of sentence after current word
  \result iLength Length of string. If 0, NULL will be returned.
  \return Pointer to sentence following current word, or NULL
*/
const UT_UCSChar*
fl_BlockSpellIterator::getPostWord(UT_sint32& iLength)
{
	iLength = m_iSentenceEnd - m_iStartIndex;

	// If it ever becomes necessary to mutate the pre-word, allocate
	// space to m_pMutatedString and return it. Caller will consume
	// that buffer before calling any other function.
	
	if (0 >= iLength)
		return NULL;

	return reinterpret_cast<UT_UCSChar*>(m_pgb->getPointer(m_iStartIndex));
}

/*!
  Move iterator back to the previous word.
  This method can only be called once per iteration.
*/
void
fl_BlockSpellIterator::revertToPreviousWord()
{
	m_iStartIndex = m_iPrevStartIndex;
}

bool
fl_BlockSpellIterator::_ignoreFirstWordCharacter(const UT_UCSChar c) const
{
    switch (c) {
    case '\'':
    case '"':
    case UCS_LDBLQUOTE:         // smart quoute, open double
    case UCS_LQUOTE:            // smart quoute, open
        return true;
    default:
        return false;
    }
}

bool
fl_BlockSpellIterator::_ignoreLastWordCharacter(const UT_UCSChar c) const
{
    switch (c) {
    case '\'':
    case '"':
    case UCS_RDBLQUOTE:         // smart quote, close double
    case UCS_RQUOTE:            // smart quote, close
        return true;
    default:
        return false;
    }
}
