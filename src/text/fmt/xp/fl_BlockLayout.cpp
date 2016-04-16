/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "fl_BlockLayout.h"
#include "pf_Frag_Strux.h"
#ifdef ENABLE_SPELL
#include "fl_Squiggles.h"
#endif
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fl_FootnoteLayout.h"
#include "fl_TableLayout.h"
#include "fl_AutoNum.h"
#include "fl_TOCLayout.h"
#include "fb_LineBreaker.h"
#include "fb_Alignment.h"
#include "fp_Column.h"
#include "fp_FootnoteContainer.h"
#include "fp_TableContainer.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_TextRun.h"
#include "fp_FieldListLabelRun.h"
#include "fp_DirectionMarkerRun.h"
#include "fp_FrameContainer.h"
#include "pd_Document.h"
#include "fd_Field.h"
#include "pd_Style.h"
#include "pp_Property.h"
#include "pp_AttrProp.h"
#include "pt_Types.h"
#include "gr_Graphics.h"

#ifdef ENABLE_SPELL
#include "spell_manager.h"

#if 1
// todo: work around to remove the INPUTWORDLEN restriction for pspell
#include "ispell_def.h"
#endif
#endif

#include "px_CR_FmtMark.h"
#include "px_CR_FmtMarkChange.h"
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"
#include "pd_Iterator.h"
#include "fv_View.h"
#include "xap_App.h"
#include "xap_Clipboard.h"
#include "ut_png.h"
#include "ut_sleep.h"
#include "fg_Graphic.h"
#include "ap_Prefs.h"
#include "ap_Prefs_SchemeIds.h"
#include "ut_rand.h"
#include "fp_FieldTOCNum.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_std_string.h"
#include "ut_string.h"
#include "fp_MathRun.h"
#include "fp_EmbedRun.h"

#include "xap_EncodingManager.h"

#define BIG_NUM_BLOCKBL 1000000


static void s_border_properties (const char * border_color, 
								 const char * border_style, 
								 const char * border_width,
								 const char * color,
								 const char * spacing, PP_PropertyMap::Line & line);



#ifdef ENABLE_SPELL
SpellChecker *
fl_BlockLayout::_getSpellChecker (UT_uint32 blockPos) const
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
	// 
	// Unfortunately, this does not work as intented because if the APs do not contain
	// explicit lang property and the default language for document changes, we need to
	// get a different checker. We therefore have to evaluate the property on all
	// occasions and remember the language, not the APs (bug #9562)

	static SpellChecker * checker = NULL;
	static char szPrevLang[8] = {0};

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	
	getSpanAP(blockPos, false, pSpanAP);
	getAP(pBlockAP);

	const char * pszLang = static_cast<const char *>(PP_evalProperty("lang",pSpanAP,pBlockAP,NULL,m_pDoc,true));
	if(!pszLang || !*pszLang)
	{
		// we just (dumbly) default to the last dictionary
		checker = SpellManager::instance().lastDictionary();
		return checker;
	}
	
	if(!szPrevLang[0] || strcmp(pszLang,szPrevLang))
	{
		checker = SpellManager::instance().requestDictionary(pszLang);

		strncpy(szPrevLang, pszLang, sizeof(szPrevLang));
		UT_uint32 iEnd = UT_MIN(sizeof(szPrevLang)-1, strlen(pszLang));
		szPrevLang[iEnd] = 0;
	}

	return checker;
}

bool
fl_BlockLayout::_spellCheckWord(const UT_UCSChar * word,
								UT_uint32 len, UT_uint32 blockPos) const
{
	SpellChecker * checker = _getSpellChecker (blockPos);

	if (!checker)
	{
		// no checker found, don't mark as wrong
		return true;
	}

	if (checker->checkWord (word, len) == SpellChecker::LOOKUP_SUCCEEDED)
		return true;
	return false;
}
#endif // ENABLE_SPELL


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fl_BlockLayout::fl_BlockLayout(pf_Frag_Strux* sdh,
							   fl_ContainerLayout* pPrev,
							   fl_SectionLayout* pSectionLayout,
							   PT_AttrPropIndex indexAP, bool bIsHdrFtr)
	: fl_ContainerLayout(pSectionLayout,sdh,indexAP,PTX_Block,FL_CONTAINER_BLOCK),
	  m_uBackgroundCheckReasons(0),
	  m_iNeedsReformat(0),
	  m_bNeedsRedraw(false),
	  m_bIsHdrFtr(bIsHdrFtr),
	  m_pFirstRun(NULL),
	  m_pSectionLayout(pSectionLayout),
	  m_pAlignment(NULL),
	  m_bKeepTogether(false),
	  m_bKeepWithNext(false),
	  m_bStartList(false), m_bStopList(false),
	  m_bListLabelCreated(false),
#ifdef ENABLE_SPELL
 	  m_pSpellSquiggles(NULL),
	  m_pGrammarSquiggles(NULL),
	  m_nextToSpell(0),
	  m_prevToSpell(0),
#endif
	  m_bListItem(false),
	  m_szStyle(NULL),
	  m_bIsCollapsed(true),
	  m_iDomDirection(UT_BIDI_UNSET),
	  m_iDirOverride(UT_BIDI_UNSET),
	  m_bIsTOC(false),
	  m_bStyleInTOC(false),
	  m_iTOCLevel(0),
	  m_bSameYAsPrevious(false),
	  m_iAccumulatedHeight(0),
	  m_pVertContainer(NULL),
	  m_iLinePosInContainer(0),
	  m_bForceSectionBreak(false),
	  m_bPrevListLabel(false),
	  m_iAdditionalMarginAfter(0),
	  m_ShadingForeColor(0,0,0),
	  m_ShadingBackColor(0,0,0),
	  m_iPattern(0),
	  m_bCanMergeBordersWithNext(true),
	  m_bHasBorders(false)
{
	xxx_UT_DEBUGMSG(("BlockLayout %x created sdh %x \n",this,getStruxDocHandle()));
	setPrev(pPrev);
	UT_ASSERT(myContainingLayout() != NULL);

	// insert us into the list
	if (pPrev)
		pPrev->_insertIntoList(this);
	else
	{ 
		setNext(myContainingLayout()->getFirstLayout()); 
		if (myContainingLayout()->getFirstLayout())
			myContainingLayout()->getFirstLayout()->setPrev(this); 
	}
	
	if(m_pSectionLayout && m_pSectionLayout->getType() == FL_SECTION_HDRFTR)
	{
		m_bIsHdrFtr = true;
	}
	m_pLayout = m_pSectionLayout->getDocLayout();
	m_pDoc = m_pLayout->getDocument();
	UT_ASSERT(m_pDoc);
	setAttrPropIndex(indexAP);

	const PP_AttrProp * pAP = 0;
	getAP(pAP);
    UT_ASSERT_HARMLESS(pAP);

	if (pAP && !pAP->getAttribute (PT_STYLE_ATTRIBUTE_NAME, m_szStyle))
		{
			m_szStyle = NULL;
		}
	m_bIsTOC = (pSectionLayout->getContainerType() == FL_CONTAINER_TOC);
	if(m_bIsTOC)
	{
		fl_TOCLayout * pTOCL= static_cast<fl_TOCLayout *>(getSectionLayout());
		m_iTOCLevel = pTOCL->getCurrentLevel();
	}
	if (m_szStyle != NULL)
	{
		PD_Style * pStyle = NULL;
		m_pDoc->getStyle(static_cast<const char*>(m_szStyle), &pStyle);
		if(pStyle != NULL)
		{
			pStyle->used(1);
			UT_sint32 iLoop = 0;
			while((pStyle->getBasedOn()) != NULL && (iLoop < 10))
			{
				pStyle->getBasedOn()->used(1);
				pStyle = pStyle->getBasedOn();
				iLoop++;
			}
		}
	}
	lookupProperties();
	//
	// Since the Style doesn't change we need to look to see if this
	// block should added now. We need to wait until after the lookupProperties
	// to get the field list label inserted.
	//
	if(!m_bIsTOC)
	{
		if(!isNotTOCable())
		{
			m_bStyleInTOC = m_pLayout->addOrRemoveBlockFromTOC(this);
		}
	}

	if(!isHdrFtr() || (static_cast<fl_HdrFtrSectionLayout *>(getSectionLayout())->getDocSectionLayout() != NULL))
	{
		_insertEndOfParagraphRun();
	}

#ifdef ENABLE_SPELL
	m_pSpellSquiggles = new fl_SpellSquiggles(this);
	m_pGrammarSquiggles = new fl_GrammarSquiggles(this);
	UT_ASSERT(m_pSpellSquiggles);
	UT_ASSERT(m_pGrammarSquiggles);
#endif
	setUpdatableField(false);
	updateEnclosingBlockIfNeeded();
	if (hasBorders())
	{
		if (pPrev && pPrev->getContainerType() == FL_CONTAINER_BLOCK)
		{
			fl_BlockLayout* pBPrev = static_cast<fl_BlockLayout *>(pPrev);
			if (pBPrev->hasBorders())
			{
				pBPrev->setLineHeightBlockWithBorders(-1);
			}
		}
	}
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

void buildTabStops(const char* pszTabStops, UT_GenericVector<fl_TabStop*> &m_vecTabs)
{
	// no matter what, clear prior tabstops
	UT_uint32 iCount = m_vecTabs.getItemCount();
	UT_uint32 i;

	for (i=0; i<iCount; i++)
	{
		fl_TabStop* pTab = m_vecTabs.getNthItem(i);

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

/*!
 * This method is used to reset the colorization such as what occurs
 * when showAuthors state is changed.
 */ 
void fl_BlockLayout::refreshRunProperties(void) const
{
	fp_Run * pRun = getFirstRun();
	while(pRun)
	{
		pRun->lookupProperties();
		pRun = pRun->getNextRun();
	}
}

/*!
    this function is only to be called by fl_ContainerLayout::lookupMarginProperties()
    all other code must call lookupMarginProperties() instead

    This function looks up the block margins and handles the values appropriately to the
    type of current view mode
*/
void fl_BlockLayout::_lookupMarginProperties(const PP_AttrProp* pBlockAP)
{
	UT_return_if_fail(pBlockAP);
	
	UT_ASSERT(myContainingLayout() != NULL);
 	FV_View * pView = getView();
	UT_return_if_fail( pView );
	
	GR_Graphics* pG = m_pLayout->getGraphics();

	UT_sint32 iTopMargin = m_iTopMargin;
	UT_sint32 iBottomMargin = m_iBottomMargin;
	UT_sint32 iLeftMargin = m_iLeftMargin;
	UT_sint32 iRightMargin = m_iRightMargin;
	UT_sint32 iTextIndent = getTextIndent();
	
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
	for (UT_uint32 iRg = 0; iRg < G_N_ELEMENTS(rgProps); ++iRg)
	{
		const MarginAndIndent_t& mai = rgProps[iRg];
		auto prop = getPropertyType(mai.szProp,	Property_type_size);
		// XXX ugly cast. fix this.
		const PP_PropertyTypeSize* pProp = static_cast<PP_PropertyTypeSize*>(prop.get());
		*mai.pVar	= UT_convertSizeToLayoutUnits(pProp->getValue(), pProp->getDim());
		xxx_UT_DEBUGMSG(("para prop %s layout size %d \n",mai.szProp,*mai.pVar));
	}

	if((pView->getViewMode() == VIEW_NORMAL) || ((pView->getViewMode() == VIEW_WEB) && !pG->queryProperties(GR_Graphics::DGP_PAPER)))
	{
		if(m_iLeftMargin < 0)
		{
			m_iLeftMargin = 0;
		}
		
		if(getTextIndent() < 0)
		{
			// shuv the whole thing to the left
			m_iLeftMargin -= getTextIndent();
		}

		// igonre right margin
		m_iRightMargin = 0;
	}
	
	// NOTE : Parsing spacing strings:
	// NOTE : - if spacing string ends with "+", it's marked as an "At Least" measurement
	// NOTE : - if spacing has a unit in it, it's an "Exact" measurement
	// NOTE : - if spacing is a unitless number, it's just a "Multiple"
    // 	UT_uint32 nLen = strlen(pszSpacing);
	// this assumed that only spacing 1 can be represented by a single charcter
	// but that is not very safe assumption, for there should be nothing stoping
	// us to use 2 or 3 in place of 2.0 or 3.0, so I commented this this out
	// Tomas 21/1/2002
	const char * pszSpacing = getProperty("line-height");
	const char * pPlusFound = strrchr(pszSpacing, '+');
	eSpacingPolicy spacingPolicy = m_eSpacingPolicy;
	double dLineSpacing = m_dLineSpacing;
	
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

	if((pView->getViewMode() == VIEW_NORMAL) || ((pView->getViewMode() == VIEW_WEB) && !pG->queryProperties(GR_Graphics::DGP_PAPER)))
	{
		// flatten the text; we will indicate more than single spacing by using 1.2, which
		// is enough for the text to be noticeably spaced, but not enough for it to take
		// too much space
		m_eSpacingPolicy = spacing_MULTIPLE;

		double dSpacing1 = UT_convertDimensionless("1.2");
		if(m_dLineSpacing > dSpacing1) 
			m_dLineSpacing = UT_convertDimensionless("1.2");
	}


	UT_sint32 i = 0;
	for(i=0; i< getNumFrames();i++)
	{
		fl_FrameLayout * pFrame = getNthFrameLayout(i);

		if(pFrame->isHidden() > FP_VISIBLE)
			continue;
		
		if(pFrame->getContainerType() != FL_CONTAINER_FRAME)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			continue;
		}

		pFrame->lookupMarginProperties();
	}
	
	
	if(iTopMargin != m_iTopMargin || iBottomMargin != m_iBottomMargin ||
	   iLeftMargin != m_iLeftMargin || iRightMargin != m_iRightMargin || iTextIndent != getTextIndent() ||
	   spacingPolicy != m_eSpacingPolicy || dLineSpacing != m_dLineSpacing)
	{
		collapse();
	}
}

/*!
    this function is only to be called by fl_ContainerLayout::lookupProperties()
    all other code must call lookupProperties() instead
*/
void fl_BlockLayout::_lookupProperties(const PP_AttrProp* pBlockAP)
{
	UT_return_if_fail(pBlockAP);
	
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
	UT_ASSERT(myContainingLayout() != NULL);
	UT_UTF8String sOldStyle("");
	if(m_szStyle)
	{
		sOldStyle = m_szStyle;
	}
	if(!pBlockAP)
	{
		m_szStyle = NULL;
	}
	else if (!pBlockAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, m_szStyle))
	{
		m_szStyle = NULL;
	}
	UT_UTF8String sNewStyle("");
	if(m_szStyle)
	{
		sNewStyle = m_szStyle;
	}
	// Now work out our dominant direction
	// First, test if this is not a block that is the wrapper around a
	// footnote text, if it is, we will get the direction from the
	// document section that contains the footnote
	const gchar * pszFntId = NULL;
	const gchar * pszDir = NULL;
		
	if (pBlockAP && pBlockAP->getAttribute("footnote-id", pszFntId ))
	{
		if(pszFntId && *pszFntId)
		{
			UT_return_if_fail(m_pSectionLayout->getContainerType() == FL_CONTAINER_FOOTNOTE);
			fl_FootnoteLayout   * pFL = (fl_FootnoteLayout*) m_pSectionLayout;
			fl_DocSectionLayout * pDSL=	 pFL->getDocSectionLayout();
			UT_return_if_fail(pDSL);
			
			const PP_AttrProp * pSectionAP = NULL;
			pDSL->getAP(pSectionAP);
				
			pszDir = PP_evalProperty("dom-dir",NULL,NULL,pSectionAP,m_pDoc,false);
		}
	}

	if(!pszDir)
	{
		pszDir = getProperty("dom-dir", true);
	}
		
	UT_BidiCharType iOldDirection = m_iDomDirection;

 	FV_View * pView = getView();

	if(pView && pView->getBidiOrder() != FV_Order_Visual)
	{
		if(pView->getBidiOrder() == FV_Order_Logical_LTR)
			m_iDomDirection = UT_BIDI_LTR;
		else
			m_iDomDirection = UT_BIDI_RTL;
	}
	else if(!strcmp(pszDir,"rtl"))
	{
		m_iDomDirection = UT_BIDI_RTL;
	}
	else
		m_iDomDirection = UT_BIDI_LTR;

	// if the direction was previously set and the new dominant
	// direction is different, we have to split all runs in this
	// block at their direciton boundaries, because the base
	// direction influences the visual direciton of weak characters
	if(iOldDirection != static_cast<UT_BidiCharType>(UT_BIDI_UNSET) && iOldDirection != m_iDomDirection)
	{
		fp_Run * pRun = getFirstRun();

		while(pRun)
		{
			if (pRun->getType() == FPRUN_TEXT)
			{
				fp_TextRun * pTextRun = static_cast<fp_TextRun*>(pRun);

				//we get the next run in line prior to breaking this
				//one up, so that we do not break those already broken
				pRun = pRun->getNextRun();
				pTextRun->breakMeAtDirBoundaries(UT_BIDI_IGNORE);
			}
			else if(pRun->getType() == FPRUN_ENDOFPARAGRAPH)
			{
				// need to set the direction correctly
				pRun->setDirection(m_iDomDirection);
				pRun->setVisDirection(m_iDomDirection);
				pRun = pRun->getNextRun();
			}
			else if(pRun->getType() == FPRUN_FIELD)
			{
				fp_FieldRun * pFR = static_cast<fp_FieldRun*>(pRun);
				if(pFR->getFieldType() == FPFIELD_endnote_anch  ||
				   pFR->getFieldType() == FPFIELD_endnote_ref   ||
				   pFR->getFieldType() == FPFIELD_footnote_anch ||
				   pFR->getFieldType() == FPFIELD_footnote_ref)
				{
					// need to set the direction correctly
					pRun->setDirection(m_iDomDirection);
					pRun->setVisDirection(m_iDomDirection);
					pRun = pRun->getNextRun();
				}

				pRun = pRun->getNextRun();
			}
			else
				pRun = pRun->getNextRun();
		}

		
	}
	{
		auto orphans = getPropertyType("orphans", Property_type_int);
		const PP_PropertyTypeInt *pOrphans = static_cast<const PP_PropertyTypeInt *>(orphans.get());
		UT_ASSERT_HARMLESS(pOrphans);
		if(pOrphans)
			m_iOrphansProperty = pOrphans->getValue();

		auto widows = getPropertyType("widows", Property_type_int);
		const PP_PropertyTypeInt *pWidows = static_cast<const PP_PropertyTypeInt *>(widows.get());
		UT_ASSERT_HARMLESS(pWidows);
		if(pWidows)
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
			if (0 == strcmp("yes", pszKeepTogether))
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
			if (0 == strcmp("yes", pszKeepWithNext))
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
	for (UT_uint32 iRg = 0; iRg < G_N_ELEMENTS(rgProps); ++iRg)
	{
		const MarginAndIndent_t& mai = rgProps[iRg];
		auto prop = getPropertyType(mai.szProp, Property_type_size);
		const PP_PropertyTypeSize * pProp = static_cast<const PP_PropertyTypeSize *>(prop.get());
		*mai.pVar	= UT_convertSizeToLayoutUnits(pProp->getValue(), pProp->getDim());
		xxx_UT_DEBUGMSG(("para prop %s layout size %d \n",mai.szProp,*mai.pVar));
	}

	if((pView->getViewMode() == VIEW_NORMAL) || ((pView->getViewMode() == VIEW_WEB) && !pG->queryProperties(GR_Graphics::DGP_PAPER)))
	{
		if(m_iLeftMargin < 0)
		{
			m_iLeftMargin = 0;
		}
		
		if(getTextIndent() < 0)
		{
			// shuv the whole thing to the left
			m_iLeftMargin -= getTextIndent();
		}

		// igonre right margin
		m_iRightMargin = 0;
	}
	
	{
		const char* pszAlign = getProperty("text-align");

		// we will only delete and reallocate the alignment if it is different
		// than the current one
		//DELETEP(m_pAlignment);

		xxx_UT_DEBUGMSG(("block: _lookupProperties, text-align=%s, current %d\n", pszAlign, m_pAlignment?m_pAlignment->getType():0xffff));

		if (0 == strcmp(pszAlign, "left"))
		{
			if(!m_pAlignment || m_pAlignment->getType() != FB_ALIGNMENT_LEFT)
			{
				DELETEP(m_pAlignment);
				m_pAlignment = new fb_Alignment_left;
			}
		}
		else if (0 == strcmp(pszAlign, "center"))
		{
			if(!m_pAlignment || m_pAlignment->getType() != FB_ALIGNMENT_CENTER)
			{
				DELETEP(m_pAlignment);
				m_pAlignment = new fb_Alignment_center;
			}
		}
		else if (0 == strcmp(pszAlign, "right"))
		{
			if(!m_pAlignment || m_pAlignment->getType() != FB_ALIGNMENT_RIGHT)
			{
				DELETEP(m_pAlignment);
				m_pAlignment = new fb_Alignment_right;
			}
		}
		else if (0 == strcmp(pszAlign, "justify"))
		{
			if(!m_pAlignment || m_pAlignment->getType() != FB_ALIGNMENT_JUSTIFY)
			{
				DELETEP(m_pAlignment);
				m_pAlignment = new fb_Alignment_justify;
			}
		}
		else
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			if (!m_pAlignment)
			{
				m_pAlignment = new fb_Alignment_left;
			}
		}
	}

	// parse any new tabstops
	const char* pszTabStops = getProperty("tabstops");
	buildTabStops(pszTabStops, m_vecTabs);


#if 0
	UT_DEBUGMSG(("XXXX: [default-tab-interval:%s][yields %d][resolution %d][zoom %d]\n",
				 getProperty("default-tab-interval"),
				 UT_convertToLogicalUnits(getProperty("default-tab-interval")),
				 pG->getResolution(),
				 pG->getZoomPercentage()));
#endif

	auto prop = getPropertyType("default-tab-interval", Property_type_size);
	const PP_PropertyTypeSize * pProp = static_cast<const PP_PropertyTypeSize *>(prop.get());
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

	if((pView->getViewMode() == VIEW_NORMAL) || ((pView->getViewMode() == VIEW_WEB) && !pG->queryProperties(GR_Graphics::DGP_PAPER)))
	{
		// flatten the text; we will indicate more than single spacing by using 1.2, which
		// is enough for the text to be noticeably spaced, but not enough for it to take
		// too much space
		m_eSpacingPolicy = spacing_MULTIPLE;

		double dSpacing1 = UT_convertDimensionless("1.2");
		if(m_dLineSpacing > dSpacing1) 
			m_dLineSpacing = UT_convertDimensionless("1.2");
	}
	//
	// Shading now
	//
	{
		const gchar * sPattern = NULL;
		const gchar * sShadingForeCol = NULL;
		const gchar * sShadingBackCol = NULL;
		sPattern = getProperty("shading-pattern",true);
		if(sPattern)
		{
			m_iPattern = atoi(sPattern);
		}
		else
		{
			m_iPattern = 0;
		}
		sShadingForeCol = getProperty("shading-foreground-color",true);
		if(sShadingForeCol)
		{
			m_ShadingForeColor.setColor(sShadingForeCol);
		}
		else
		{
			m_ShadingForeColor.setColor("white");
		}
		sShadingBackCol = getProperty("shading-background-color",true);
		if(sShadingBackCol)
		{
			m_ShadingBackColor.setColor(sShadingBackCol);
		}
		else
		{
			m_ShadingBackColor.setColor("white");
		}

	}
	//
	// Borders now
	//
	{
		m_bHasBorders = false;
		m_lineBottom.m_t_linestyle = PP_PropertyMap::linestyle_none;
		m_lineTop.m_t_linestyle =  PP_PropertyMap::linestyle_none;
		m_lineLeft.m_t_linestyle =  PP_PropertyMap::linestyle_none;
		m_lineRight.m_t_linestyle =  PP_PropertyMap::linestyle_none;
		m_bCanMergeBordersWithNext = true;
		const gchar * pszCanMergeBorders = NULL;
		pszCanMergeBorders = getProperty("border-merge");
		if(pszCanMergeBorders && !strcmp(pszCanMergeBorders,"false"))
		{
			m_bCanMergeBordersWithNext = false;
		}
		const gchar * pszBorderColor = NULL;
		const gchar * pszBorderStyle = NULL;
		const gchar * pszBorderWidth = NULL;
		const gchar * pszBorderSpacing = NULL;
		//
		// Default color
		//
		const gchar * pszColor= NULL;

		pszBorderColor = getProperty ("bot-color");
		pBlockAP->getProperty ("bot-style",pszBorderStyle);
		pszBorderWidth = getProperty ("bot-thickness");
		pszBorderSpacing= getProperty ("bot-space");
		if(pBlockAP && pBlockAP->getProperty ("bot-style",pszBorderStyle) && pszBorderStyle)
		{
			s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, pszBorderSpacing,m_lineBottom);
			m_bHasBorders |= (m_lineBottom.m_t_linestyle > 1);  
		}
		pszBorderColor = NULL;
		pszBorderStyle = NULL;
		pszBorderWidth = NULL;
		pszBorderSpacing = NULL;

		pszBorderColor = getProperty ("left-color");
		pszBorderWidth = getProperty ("left-thickness");
		pszBorderSpacing = getProperty ("left-space");

		if(pBlockAP && pBlockAP->getProperty ("left-style",pszBorderStyle) && pszBorderStyle)
		{
 			s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, pszBorderSpacing,m_lineLeft);
			m_bHasBorders |= (m_lineLeft.m_t_linestyle > 1);  
		}
		pszBorderColor = NULL;
		pszBorderStyle = NULL;
		pszBorderWidth = NULL;
		pszBorderSpacing = NULL;

		pszBorderColor = getProperty ("right-color");
		pszBorderStyle = getProperty ("right-style");
		pszBorderWidth = getProperty ("right-thickness");
		pszBorderSpacing = getProperty ("right-space");

		if(pBlockAP && pBlockAP->getProperty ("right-style",pszBorderStyle) && pszBorderStyle)
		{
			s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, pszBorderSpacing,m_lineRight);
			m_bHasBorders |= (m_lineRight.m_t_linestyle > 1);  
		}
		pszBorderColor = NULL;
		pszBorderStyle = NULL;
		pszBorderWidth = NULL;
		pszBorderSpacing = NULL;

		pszBorderColor = getProperty ("top-color");
		pszBorderWidth = getProperty ("top-thickness");
		pszBorderSpacing = getProperty ("top-space");
	
		if(pBlockAP && pBlockAP->getProperty ("top-style",pszBorderStyle) && pszBorderStyle)
		{
			s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, pszBorderSpacing,m_lineTop);
			m_bHasBorders |= (m_lineTop.m_t_linestyle > 1); 
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
	const gchar * szLid=NULL;
	const gchar * szPid=NULL;
	const gchar * szLevel=NULL;
	UT_uint32 id,parent_id;

	if (!pBlockAP || !pBlockAP->getAttribute(PT_LISTID_ATTRIBUTE_NAME, szLid))
		szLid = NULL;
	if (szLid)
	{
		id = atoi(szLid);
		
	}
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
			m_pDoc->removeList(m_pAutoNum,getStruxDocHandle());
			DELETEP(m_pAutoNum);
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
		pAutoNum = m_pDoc->getListByID(id);
		//
		// Create new list if none exists
		//
		if(pAutoNum == NULL)
		{
			const gchar * pszStart = getProperty("start-value",true);
			const gchar * lDelim =  getProperty("list-delim",true);
			const gchar * lDecimal =  getProperty("list-decimal",true);
			UT_uint32 start = atoi(pszStart);
			const gchar * style = NULL;
			style = getProperty("list-style",true);
			if(!style)
			{
				pBlockAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME,style);
			}
			UT_ASSERT(style);
			FL_ListType lType = getListTypeFromStyle( style);
			pAutoNum = new fl_AutoNum(id, parent_id, lType, start, lDelim, lDecimal, m_pDoc, getView());
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
			pf_Frag_Strux* pItem = getStruxDocHandle();
			pf_Frag_Strux* ppItem;
			if(prevBlockInList != NULL )
			{
				ppItem = prevBlockInList->getStruxDocHandle();
			}
			else
			{
				ppItem = NULL;
			}
			m_pAutoNum->insertFirstItem(pItem,ppItem,0);
			m_bStartList = true;
		}

		xxx_UT_DEBUGMSG(("Added Item to List\n"));
	}

	// Add this in for loading - see if better way to fix.
	// if (m_bListItem && !m_bListLabelCreated && m_pFirstRun)
	//	_createListLabel();

	xxx_UT_DEBUGMSG(("BlockLayout %x Folded Level %d sdh %x \n",this,getFoldedLevel(),getStruxDocHandle()));

	//
	// Look after TOC handling now.
	//
	if(!m_bIsTOC && !(sNewStyle == sOldStyle))
	{
		if(!isNotTOCable())
		{
			if(m_bStyleInTOC)
			{
				m_pLayout->removeBlockFromTOC(this); // remove old one
			}
			m_bStyleInTOC = m_pLayout->addOrRemoveBlockFromTOC(this);
		}
	}
	// later we will need to add here revision handling ...
}

UT_sint32 fl_BlockLayout::getPattern(void) const
{
	return m_iPattern;
}

const UT_RGBColor fl_BlockLayout::getShadingingForeColor(void) const
{
	return m_ShadingForeColor;
}

const UT_RGBColor fl_BlockLayout::getShadingingBackColor(void) const
{
	return m_ShadingBackColor;
}

bool fl_BlockLayout::canMergeBordersWithPrev(void) const
{
	if(!getPrev())
		return false;
	if(getPrev()->getContainerType() !=  FL_CONTAINER_BLOCK)
		return false;
	const fl_BlockLayout * pPrev = static_cast<const fl_BlockLayout *>(getPrev());
	if((pPrev->getBottom() == getBottom()) &&
	   (pPrev->getTop() == getTop()) &&
	   (pPrev->getLeft() == getLeft()) &&
	   (pPrev->getRight() == getRight()) &&
	   (pPrev->getLeftMargin() == getLeftMargin()) &&
	   (pPrev->getRightMargin() == getRightMargin()) &&
	   (pPrev->getTextIndent() == getTextIndent()) &&
	   (pPrev->m_bCanMergeBordersWithNext))
		{
			return true;
		}
	return false;
}


bool fl_BlockLayout::canMergeBordersWithNext(void) const
{
	if(!getNext())
		return false;
	if(getNext()->getContainerType() !=  FL_CONTAINER_BLOCK)
		return false;
	fl_BlockLayout * pNext = static_cast<fl_BlockLayout *>(getNext());
	if((pNext->getBottom() == getBottom()) &&
	   (pNext->getTop() == getTop()) &&
	   (pNext->getLeft() == getLeft()) &&
	   (pNext->getRight() == getRight()) &&
	   (pNext->getLeftMargin() == getLeftMargin()) &&
	   (pNext->getRightMargin() == getRightMargin()) &&
	   (pNext->getTextIndent() == getTextIndent()) &&
	   m_bCanMergeBordersWithNext)
		{
			return true;
		}
	return false;
}

bool fl_BlockLayout::hasBorders(void) const
{
	return m_bHasBorders;
}

// Recalculate the line heights of a block with borders.
// If whichLine=1, recalculate only the height of the first line.
// If whichLine=-1, recalculate only the height of the last line.
// For other values of whichLine, recalculate the height for all the lines
void fl_BlockLayout::setLineHeightBlockWithBorders(int whichLine)
{
	fp_Line * pLine = NULL;
	switch(whichLine)
	{
	case 1:
		pLine = static_cast<fp_Line *>(getFirstContainer());
		if(pLine)
		{
			pLine->setAlongTopBorder(false);
			pLine->setAlongBotBorder(false);
			pLine->calcBorderThickness();
			pLine->recalcHeight();
			if(pLine->isWrapped())
			{
				pLine = static_cast<fp_Line *>(pLine->getNext());
				while(pLine && pLine->isSameYAsPrevious())
				{
					pLine->setAlongTopBorder(false);
					pLine->setAlongBotBorder(false);
					pLine->calcBorderThickness();
					pLine->recalcHeight();
				}
			}
		}
		break;
	case -1:
		pLine = static_cast<fp_Line *>(getLastContainer());
		if(pLine)
		{
			pLine->setAlongTopBorder(false);
			pLine->setAlongBotBorder(false);
			pLine->calcBorderThickness();
			pLine->recalcHeight();
			if(pLine->isSameYAsPrevious())
			{
				do
				{
					pLine = static_cast<fp_Line *>(pLine->getPrev());
					if(pLine)
					{
						pLine->setAlongTopBorder(false);
						pLine->setAlongBotBorder(false);
						pLine->calcBorderThickness();
						pLine->recalcHeight();
					}
				}
				while(pLine && pLine->isSameYAsPrevious());
			}
		}
		break;
	default:
		pLine = static_cast<fp_Line *>(getFirstContainer());
		while(pLine)
		{
			pLine->setAlongTopBorder(false);
			pLine->setAlongBotBorder(false);
			pLine->calcBorderThickness();
			pLine->recalcHeight();
			pLine = static_cast<fp_Line *>(pLine->getNext());
		}	
	}
}

fl_BlockLayout::~fl_BlockLayout()
{
#ifdef ENABLE_SPELL
	dequeueFromSpellCheck();
	DELETEP(m_pSpellSquiggles);
	DELETEP(m_pGrammarSquiggles);
#endif
	purgeLayout();
	UT_VECTOR_PURGEALL(fl_TabStop *, m_vecTabs);
	DELETEP(m_pAlignment);
	//	if (m_pAutoNum)
//		{
//			m_pAutoNum->removeItem(getStruxDocHandle());
//			if (m_pAutoNum->isEmpty())
//				DELETEP(m_pAutoNum);
//		}
	if(!m_bIsTOC)
	{
		if(!isNotTOCable())
		{
			m_pLayout->removeBlockFromTOC(this);
		}
	}

	UT_ASSERT_HARMLESS(m_pLayout != NULL);
	if(m_pLayout)
	{
		m_pLayout->notifyBlockIsBeingDeleted(this);
#ifdef ENABLE_SPELL
		m_pLayout->dequeueBlockForBackgroundCheck(this);
#endif
	}

	m_pDoc = NULL;
	m_pLayout = NULL;
	xxx_UT_DEBUGMSG(("~fl_BlockLayout: Deleting block %x sdh %x \n",this,getStruxDocHandle()));
}

void fl_BlockLayout::getStyle(UT_UTF8String & sStyle) const
{
	sStyle = m_szStyle;
}

/*!
 * This method returns true if the block is contained with a section embedded
 * in a block, like a footnote or a table or frame with text wrapping.
 */
bool fl_BlockLayout::isEmbeddedType(void) const
{
	fl_ContainerLayout * pCL = myContainingLayout();
	if(pCL && (((pCL->getContainerType() == FL_CONTAINER_FOOTNOTE) || (pCL->getContainerType() == FL_CONTAINER_ENDNOTE) ) || (pCL->getContainerType() == FL_CONTAINER_ANNOTATION )) )
	{
		return true;
	}
	return false;
}


/*!
 * This method returns true if the block is contained with a section embedded
 * that should not be included in TOC like, footnote,endnotes,HdrFtr's 
 * and other TOC's.
 */
bool fl_BlockLayout::isNotTOCable(void) const
{
	fl_ContainerLayout * pCL = myContainingLayout();
	if(pCL && (pCL->getContainerType() == FL_CONTAINER_FOOTNOTE 
			   || pCL->getContainerType() == FL_CONTAINER_ENDNOTE 
			   || pCL->getContainerType() == FL_CONTAINER_ANNOTATION 
			   || pCL->getContainerType() == FL_CONTAINER_HDRFTR 
			   || pCL->getContainerType() == FL_CONTAINER_TOC 
			   || pCL->getContainerType() == FL_CONTAINER_SHADOW
			   ) )
	{
		return true;
	}
	if(pCL == NULL)
	{
		return true;
	}
	if(pCL->getContainerType() == FL_CONTAINER_CELL)
	{
		pCL = pCL->myContainingLayout(); // should be a table
		if(pCL == NULL)
		{
			return true;
		}
		pCL = pCL->myContainingLayout(); // is it a Hdrftr?
		if(pCL && (pCL->getContainerType() == FL_CONTAINER_HDRFTR
			   || pCL->getContainerType() == FL_CONTAINER_SHADOW
			   ) )
		{
			return true;
		}
	}
	return false;
}

/*!
 * This method returns the offset of the next embedded strux within the
 * the block. (Like a footnote or endnote)
 * It returns -1 if none is found.
 * Also returns the id of the embedded strux.
 */ 
UT_sint32 fl_BlockLayout::getEmbeddedOffset(UT_sint32 offset, fl_ContainerLayout *& pEmbedCL) const
{
	UT_sint32 iEmbed = -1;
	PT_DocPosition posOff = static_cast<PT_DocPosition>(offset);
	pf_Frag_Strux* sdhEmbed;
	pEmbedCL = NULL;
	iEmbed = m_pDoc->getEmbeddedOffset(getStruxDocHandle(), posOff, sdhEmbed);
	if( iEmbed < 0)
	{
		return iEmbed;
	}
	fl_ContainerLayout* sfhEmbed = NULL;
	bool bFound = false;
	sfhEmbed = m_pDoc->getNthFmtHandle(sdhEmbed,m_pLayout->getLID());
	if(	sfhEmbed == NULL)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return -1;
	}
	pEmbedCL = sfhEmbed;
	if(pEmbedCL->getDocSectionLayout() == getDocSectionLayout())
	{
		bFound = true;
	}
	else
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		pEmbedCL = NULL;
		return -1;
	}
	if(bFound)
	{
		if(pEmbedCL->getContainerType() == FL_CONTAINER_TOC)
		{
			pEmbedCL = NULL;
			return -1;
		}
		return iEmbed;
	}
	pEmbedCL = NULL;
	return -1;
}

/*! 
 * This method scans through the list of runs from the first position listed
 * and updates the offsets. This is used following an operation on an embedded
 * type section (Like a footnote). Also updates the char widths and the POB's
 * in the squiggles.
\param posEmbedded the position of the embedded Section.
\param iEmbeddedSize the size of the embedded Section.
 */
void fl_BlockLayout::updateOffsets(PT_DocPosition posEmbedded, UT_uint32 iEmbeddedSize, UT_sint32 iSuggestDiff)
{
	UT_UNUSED(iEmbeddedSize);
	xxx_UT_DEBUGMSG(("In update Offsets posEmbedded %d EmbeddedSize %d shift %d \n",posEmbedded,iEmbeddedSize,iSuggestDiff));
	fp_Run * pRun = getFirstRun();
	PT_DocPosition posOfBlock = getPosition(true);
	PT_DocPosition posAtStartOfBlock = getPosition();
	fp_Run * pPrev = NULL;
#if DEBUG
	while(pRun)
	{
		xxx_UT_DEBUGMSG(("!!Initially run %p runType %d posindoc %d end run %d \n",pRun,pRun->getType(),posAtStartOfBlock+pRun->getBlockOffset(),posAtStartOfBlock+pRun->getBlockOffset()+pRun->getLength()));
		pRun = pRun->getNextRun();
	}
	pRun = getFirstRun();
#endif
	while(pRun && (posAtStartOfBlock + pRun->getBlockOffset() < posEmbedded))
	{
		xxx_UT_DEBUGMSG(("Look at run %p runType %d posindoc %d \n",pRun,pRun->getType(),posAtStartOfBlock+pRun->getBlockOffset()));
		pPrev = pRun;
		pRun = pRun->getNextRun();
	 
	}
	PT_DocPosition posRun = 0;
	if(pRun == NULL)
	{
		if(pPrev == NULL)
		{
			UT_DEBUGMSG(("!!!YIKES NO RUN or PREV RUN!!! \n"));
			return;
		}
		//
		// Catch case of EOP actually containing posEmebedded
		//
		if((posOfBlock + pPrev->getBlockOffset() +1) < posEmbedded)
		{
			xxx_UT_DEBUGMSG(("!!! POSEMBEDDED past end of block!! \n"));
			xxx_UT_DEBUGMSG(("End of block %d PosEmbedded %d \n",posOfBlock+pPrev->getBlockOffset()+1,posEmbedded));
			return;
		}
		else
		{
			pRun = pPrev;
			pPrev = pRun->getPrevRun();
		}
	}
	else
	{
		posRun = posAtStartOfBlock + pRun->getBlockOffset();
		if((posRun > posEmbedded) && pPrev)
		{
			posRun = posAtStartOfBlock + pPrev->getBlockOffset();
			if(posRun < posEmbedded)
			{
				pRun = pPrev;
				pPrev = pRun->getPrevRun();
			}
		}
	}
	//
	// Position of pRun should be  <= posEmbedded
	//
	posRun = posAtStartOfBlock + pRun->getBlockOffset();
	fp_Run * pNext = pRun->getNextRun();
	if(pNext && (posRun + pRun->getLength() <= posEmbedded) && ((pNext->getBlockOffset() + posAtStartOfBlock) > posEmbedded))
	{
		//
		// OK it's obvious here. Run previous is before posEmbedded next 
		// is after. Use it

		pRun = pNext;
	}
	else if(posRun < posEmbedded)
	{
		UT_uint32 splitOffset = posEmbedded - posOfBlock -1;
		if(splitOffset > pRun->getBlockOffset() && (pRun->getBlockOffset() + pRun->getLength() > splitOffset))
		{
			UT_ASSERT(pRun->getType() == FPRUN_TEXT);
			fp_TextRun * pTRun = static_cast<fp_TextRun *>(pRun);
			xxx_UT_DEBUGMSG(("updateOffsets: Split at offset %d \n",splitOffset));
			bool bres = pTRun->split(splitOffset,0);
			UT_UNUSED(bres);
			UT_ASSERT(bres);
			pRun = pTRun->getNextRun();
			pPrev = pTRun;
			xxx_UT_DEBUGMSG(("New Run %x created offset %d \n",pRun,pRun->getBlockOffset()));
			xxx_UT_DEBUGMSG(("Old Run %x offset %d length\n",pPrev,pPrev->getBlockOffset(),pPrev->getLength()));
		}
		else
		{
			// Split point is actually after this run
			UT_ASSERT(splitOffset == pRun->getBlockOffset());
			pPrev = pRun;
			pRun = pRun->getNextRun();
			UT_ASSERT(pRun);
			if(pRun == NULL)
			{
				pPrev = pRun;
			}
		}
	}

	//
	// pRun is the first run that gets shifted
	//
	UT_return_if_fail(pRun);
	posRun = posAtStartOfBlock + pRun->getBlockOffset();


	if(iSuggestDiff !=  0)
	{
//
// Now shift all the offsets in the runs.
//
#ifdef ENABLE_SPELL
		UT_sint32 iFirstOffset = static_cast<UT_sint32>(pRun->getBlockOffset());
#endif
		while(pRun)
		{
			UT_sint32 iNew = static_cast<UT_sint32>(pRun->getBlockOffset()) + iSuggestDiff;
			//
			// Since suggestDiff can be < 0 we need to check this
			//
			// Check if this iNew is sane.
			pPrev = pRun->getPrevRun();
			if(pPrev && (static_cast<UT_sint32>(pPrev->getBlockOffset() + pPrev->getLength()) > iNew))
			{
				// Something went wrong. Try to recover
				UT_DEBUGMSG(("Invalid updated offset %d Try to recover \n",iNew));
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				if(pRun->getType() == FPRUN_FMTMARK)
				{
					iNew= static_cast<UT_sint32>(pPrev->getBlockOffset() + pPrev->getLength());
				}
				else
				{
					iNew= static_cast<UT_sint32>(pPrev->getBlockOffset() + pPrev->getLength()) + 1;
				} 
			}
			else if( (pPrev == NULL) && (iNew < 0))
			{
				// Something went wrong. Try to recover
				UT_DEBUGMSG(("Invalid updated offset %d Try to recover \n",iNew));
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				iNew = 0;
			}
			UT_ASSERT(iNew >= 0);
			xxx_UT_DEBUGMSG(("Run %p Old offset %d New Offset %d \n",pRun,pRun->getBlockOffset(),iNew));
			pRun->setBlockOffset(static_cast<UT_uint32>(iNew));
			pRun = pRun->getNextRun();
		}
//
// Now update the PartOfBlocks in the squiggles
//
#ifdef ENABLE_SPELL
		getSpellSquiggles()->updatePOBs(iFirstOffset,iSuggestDiff);
		getGrammarSquiggles()->updatePOBs(iFirstOffset,iSuggestDiff);
#endif
	}
#if 1
#if DEBUG
	pRun = getFirstRun();
	while(pRun)
	{
		if(pRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun * pTRun = static_cast<fp_TextRun *>(pRun);
			pTRun->printText();
		}
		UT_DEBUGMSG(("update offsets!!!!--- Run %p offset %d Type %d \n",pRun,pRun->getBlockOffset(),pRun->getType()));
		pRun = pRun->getNextRun();
	}
#endif
#endif
	setNeedsReformat(this);
	updateEnclosingBlockIfNeeded();
}

/*!
 * This method updates the enclosing Block which contains the embedded Section
 * which in turn contains this Block. If this is not a block in an embedded
 * section type, we just return and do nothing.
 */	
void fl_BlockLayout::updateEnclosingBlockIfNeeded(void)
{
	UT_return_if_fail (m_pLayout);
	
	if(!isEmbeddedType())
	{
		xxx_UT_DEBUGMSG(("Block %x is Not enclosed - returning \n"));
		return;
	}
	fl_ContainerLayout * pCL = myContainingLayout();
	UT_ASSERT((pCL->getContainerType() == FL_CONTAINER_FOOTNOTE) || (pCL->getContainerType() == FL_CONTAINER_ENDNOTE) || (pCL->getContainerType() == FL_CONTAINER_ANNOTATION) );
	fl_EmbedLayout * pFL = static_cast<fl_EmbedLayout *>(pCL);
	if(!pFL->isEndFootnoteIn())
	{
		return;
	}
	pf_Frag_Strux* sdhStart = pCL->getStruxDocHandle();
	pf_Frag_Strux* sdhEnd = NULL;
	if(pCL->getContainerType() == FL_CONTAINER_FOOTNOTE)
	{
		getDocument()->getNextStruxOfType(sdhStart,PTX_EndFootnote, &sdhEnd);
	}
	else if(pCL->getContainerType() == FL_CONTAINER_ENDNOTE)
	{
		getDocument()->getNextStruxOfType(sdhStart,PTX_EndEndnote, &sdhEnd);
	}
	else if(pCL->getContainerType() == FL_CONTAINER_ANNOTATION)
	{
		getDocument()->getNextStruxOfType(sdhStart,PTX_EndAnnotation, &sdhEnd);
	}

	UT_return_if_fail(sdhEnd != NULL);
	PT_DocPosition posStart = getDocument()->getStruxPosition(sdhStart);
	PT_DocPosition posEnd = getDocument()->getStruxPosition(sdhEnd);
	UT_uint32 iSize = posEnd - posStart + 1;
	fl_ContainerLayout*  psfh = NULL;
	getDocument()->getStruxOfTypeFromPosition(m_pLayout->getLID(),posStart,PTX_Block, &psfh);
	fl_BlockLayout * pBL = static_cast<fl_BlockLayout*>(psfh);
	UT_ASSERT(pBL->getContainerType() == FL_CONTAINER_BLOCK);
	UT_ASSERT(iSize > 1);
    UT_sint32 iOldSize = pFL->getOldSize();
    pFL->setOldSize(iSize);
	pBL->updateOffsets(posStart,iSize,iSize-iOldSize);
}

/*!
 * Get the enclosing block of this if this block is in a footnote-type strux
 * Return NULL is not an embedded type 
*/
fl_BlockLayout * fl_BlockLayout::getEnclosingBlock(void) const
{
	UT_return_val_if_fail (m_pLayout,NULL);
	
	if(!isEmbeddedType())
	{
		xxx_UT_DEBUGMSG(("Block %x is Not enclosed - returning \n"));
		return NULL;
	}
	fl_ContainerLayout * pCL = myContainingLayout();
	UT_ASSERT((pCL->getContainerType() == FL_CONTAINER_FOOTNOTE) || (pCL->getContainerType() == FL_CONTAINER_ENDNOTE) || (pCL->getContainerType() == FL_CONTAINER_ANNOTATION) );
	fl_EmbedLayout * pFL = static_cast<fl_EmbedLayout *>(pCL);
	if(!pFL->isEndFootnoteIn())
	{
		return NULL;
	}
	pf_Frag_Strux* sdhStart = pCL->getStruxDocHandle();
	pf_Frag_Strux* sdhEnd = NULL;
	if(pCL->getContainerType() == FL_CONTAINER_FOOTNOTE)
	{
		getDocument()->getNextStruxOfType(sdhStart,PTX_EndFootnote, &sdhEnd);
	}
	else if(pCL->getContainerType() == FL_CONTAINER_ENDNOTE)
	{
		getDocument()->getNextStruxOfType(sdhStart,PTX_EndEndnote, &sdhEnd);
	}
	else if(pCL->getContainerType() == FL_CONTAINER_ANNOTATION)
	{
		getDocument()->getNextStruxOfType(sdhStart,PTX_EndAnnotation, &sdhEnd);
	}

	UT_return_val_if_fail(sdhEnd != NULL,NULL);
	PT_DocPosition posStart = getDocument()->getStruxPosition(sdhStart);
	fl_ContainerLayout*  psfh = NULL;
	getDocument()->getStruxOfTypeFromPosition(m_pLayout->getLID(),posStart,PTX_Block, &psfh);
	fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(psfh);
	UT_ASSERT(pBL->getContainerType() == FL_CONTAINER_BLOCK);
	return pBL;
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
	else if	(getSectionLayout()->getType() == FL_SECTION_TOC)
	{
		pDSL = static_cast<fl_TOCLayout *>(getSectionLayout())->getDocSectionLayout();
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
	else if	(getSectionLayout()->getType() == FL_SECTION_ANNOTATION)
	{
		pDSL = static_cast<fl_AnnotationLayout *>(getSectionLayout())->getDocSectionLayout();
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
	else if (getSectionLayout()->getType() == FL_SECTION_FRAME)
	{
		pDSL = static_cast<fl_ContainerLayout *>(getSectionLayout())->getDocSectionLayout();
		return pDSL;
	}
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}


fp_Line * fl_BlockLayout::findLineWithFootnotePID(UT_uint32 pid) const
{
	fp_Line * pLine = static_cast<fp_Line *>(getFirstContainer());
	UT_GenericVector<fp_FootnoteContainer *> vecFoots;
	bool bFound = false;
	while(pLine && !bFound)
	{
		vecFoots.clear();
		if(pLine->getFootnoteContainers(&vecFoots))
		{
			UT_sint32 i = 0;
			for(i=0; i< vecFoots.getItemCount(); i++)
			{
				fp_FootnoteContainer * pFC = vecFoots.getNthItem(i);
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

FootnoteType fl_BlockLayout::getTOCNumType(void) const
{
	UT_ASSERT(m_bIsTOC);
	fl_TOCLayout * pTOCL = static_cast<fl_TOCLayout *>(getSectionLayout());
	UT_ASSERT(pTOCL->getContainerType() == FL_CONTAINER_TOC);
	return pTOCL->getNumType(m_iTOCLevel);
}

eTabLeader fl_BlockLayout::getTOCTabLeader(UT_sint32 iOff) const
{
	UT_ASSERT(m_bIsTOC);
	fl_TOCLayout * pTOCL = static_cast<fl_TOCLayout *>(getSectionLayout());
	UT_ASSERT(pTOCL->getContainerType() == FL_CONTAINER_TOC);
	if(iOff > 1)
	{
		return pTOCL->getTabLeader(m_iTOCLevel);
	}
	return FL_LEADER_NONE;
}

UT_sint32 fl_BlockLayout::getTOCTabPosition(UT_sint32 iOff) const
{
	UT_ASSERT(m_bIsTOC);
	fl_TOCLayout * pTOCL = static_cast<fl_TOCLayout *>(getSectionLayout());
	UT_ASSERT(pTOCL->getContainerType() == FL_CONTAINER_TOC);
	if(iOff > 1)
	{
		return pTOCL->getTabPosition(m_iTOCLevel,this);
	}
	return 0;
}

UT_sint32 fl_BlockLayout::getMaxNonBreakableRun(void) const
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
		pRun = pRun->getNextRun();
	}
	return iMax;
}

bool fl_BlockLayout::isHdrFtr(void) const
{
	if(getSectionLayout()!= NULL)
	{
		return (getSectionLayout()->getType() == FL_SECTION_HDRFTR);
	}
	else
		return m_bIsHdrFtr;
}

void fl_BlockLayout::clearScreen(GR_Graphics* /* pG */) const
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

void fl_BlockLayout::_mergeRuns(fp_Run* pFirstRunToMerge, fp_Run* pLastRunToMerge) const
{
	UT_ASSERT(pFirstRunToMerge != pLastRunToMerge);
	UT_ASSERT(pFirstRunToMerge->getType() == FPRUN_TEXT);
	UT_ASSERT(pLastRunToMerge->getType() == FPRUN_TEXT);

	_assertRunListIntegrity();

	fp_TextRun* pFirst = static_cast<fp_TextRun*>(pFirstRunToMerge);

	bool bDone = false;
	while (!bDone)
	{
		if (pFirst->getNextRun() == pLastRunToMerge)
		{
			bDone = true;
		}

		pFirst->mergeWithNext();
	}

	_assertRunListIntegrity();
}

void fl_BlockLayout::coalesceRuns(void) const
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
		pCurrentRun = pCurrentRun->getNextRun();
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
	xxx_UT_DEBUGMSG(("Collapsing Block %x No containers %d \n",this,findLineInBlock(static_cast<fp_Line *>(getLastContainer()))));
	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		pRun->setLine(NULL);

		pRun = pRun->getNextRun();
	}

	fp_Line* pLine = static_cast<fp_Line *>(getFirstContainer());
	while (pLine)
	{
		fl_DocSectionLayout * pDSL = getDocSectionLayout();
		if(!pDSL->isCollapsing())
		{
			_removeLine(pLine,true,false);
		}
		else
		{
			_removeLine(pLine,false,false);
		}
		pLine = static_cast<fp_Line *>(getFirstContainer());
	}
	xxx_UT_DEBUGMSG(("Block collapsed in collapsed %x \n",this));
	m_bIsCollapsed = true;
	m_iNeedsReformat = 0;
	UT_ASSERT_HARMLESS(getFirstContainer() == NULL);
	UT_ASSERT_HARMLESS(getLastContainer() == NULL);
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
		fp_Run* pNext = m_pFirstRun->getNextRun();
		m_pFirstRun->setBlock(NULL);
		delete m_pFirstRun;
		m_pFirstRun = pNext;
	}
}

void fl_BlockLayout::_removeLine(fp_Line* pLine, bool bRemoveFromContainer, bool bReCalc) 
{
	if(!pLine->canDelete())
	{
		m_pLayout->setRebuiltBlock(this);
	}
	if (getFirstContainer() == static_cast<fp_Container *>(pLine))
	{
		setFirstContainer(static_cast<fp_Container *>(getFirstContainer()->getNext()));

		// we have to call recalcMaxWidth so that the new line has the correct
		// x offset and width

		if(!getDocSectionLayout()->isCollapsing() && getFirstContainer() && bReCalc)
			getFirstContainer()->recalcMaxWidth();
	}

	if (getLastContainer() == static_cast<fp_Container *>(pLine))
	{
		setLastContainer(static_cast<fp_Container *>(getLastContainer()->getPrev()));
	}

	if(pLine->getContainer() && bRemoveFromContainer)
	{
		fp_VerticalContainer * pVert = static_cast<fp_VerticalContainer *>(pLine->getContainer());
		pVert->removeContainer(pLine);
		pLine->setContainer(NULL);
	}
	pLine->remove();
	pLine->setBlock(NULL);
	xxx_UT_DEBUGMSG(("Removed line %x \n",pLine));
	UT_ASSERT(findLineInBlock(pLine) == -1);

	delete pLine;
#if DEBUG
	if(getFirstContainer())
	{
		UT_ASSERT(getFirstContainer()->getPrev() == NULL);
	}
#endif

// if the block has borders we may need to change the last line height
	if (hasBorders())
	{
		setLineHeightBlockWithBorders(-1);
	}
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
#if DEBUG
	if(getFirstContainer())
	{
		UT_ASSERT(getFirstContainer()->getPrev() == NULL);
	}
#endif
}


void fl_BlockLayout::_removeAllEmptyLines(void)
{
	fp_Line* pLine;

	pLine = static_cast<fp_Line *>(getFirstContainer());
	while (pLine)
	{
		if (pLine->isEmpty())
		{
			fp_Line * pNext = static_cast<fp_Line *>(pLine->getNext());
			_removeLine(pLine, true,true);
			pLine = pNext;
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
				pRun = pRun->getNextRun();
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

		pRun = pRun->getNextRun();
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
	UT_return_if_fail(pLine);
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
		pTempRun->lookupProperties();
		pLine->addRun(pTempRun);

		if(pTempRun->getType() == FPRUN_TEXT && !UT_BIDI_IS_STRONG(pTempRun->getDirection()))
		{
			// if the runs is not of a strong type, we have to ensure its visual direction gets
			// recalculated and buffer refreshed ...
			pTempRun->setVisDirection(UT_BIDI_UNSET);
		}
		   
		pTempRun = pTempRun->getNextRun();
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

	fp_EndOfParagraphRun* pEOPRun = new fp_EndOfParagraphRun(this, 0, 0);
	m_pFirstRun = pEOPRun;

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
					|| eHidden == FP_HIDDEN_FOLDED
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
		xxx_UT_DEBUGMSG(("In _breakLineAfterRun no LastLine \n"));
		xxx_UT_DEBUGMSG(("getPrev = %d this = %d \n", getPrev(), this));
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
	fp_Run* pCurrentRun = pRun->getNextRun();
	while (pCurrentRun && pCurrentRun->getLine() == pLine)
	{
		pLine->removeRun(pCurrentRun, true);
		pNewLine->addRun(pCurrentRun);
		pCurrentRun = pCurrentRun->getNextRun();
	}

	// Update the layout information in the lines.
	pLine->layout();
	pNewLine->layout();
#if DEBUG
	if(getFirstContainer())
	{
		UT_ASSERT(getFirstContainer()->getPrev() == NULL);
	}
#endif
	_assertRunListIntegrity();
}

/*!
 * This method is called at the end of the layout method in 
 * fp_VerticalContainer. It places the frames pointed to within the block at
 * the appropriate place on the appropriate page. Since we don't know where
 * this is until the lines in the block are placed in a column we have to 
 * wait until both the column and lines are placed on the page.
 *
 * pLastLine is the last line placed inthe column. If the frame should be 
 * placed after this line we don't place any frames that should be below 
 * this line now. In this case we wait until pLastLine is below the frame.
 *
 * If pLastLine is NULL we place all the frames in this block on the screen.
 * 
 */
bool fl_BlockLayout::setFramesOnPage(fp_Line * pLastLine)
{
	FV_View *pView = getView();
	GR_Graphics * pG = m_pLayout->getGraphics();
	FL_DocLayout *pDL = getDocLayout();
	UT_return_val_if_fail( pView && pG, false );
	
	if(getNumFrames() == 0)
	{
		return true;
	}
	
	UT_sint32 i = 0;
	for(i=0; i< getNumFrames();i++)
	{
		fl_FrameLayout * pFrame = getNthFrameLayout(i);

		if(pFrame->isHidden() > FP_VISIBLE)
			continue;
		
		if(pFrame->getContainerType() != FL_CONTAINER_FRAME)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			continue;
		}
		if(pFrame->getFramePositionTo() == FL_FRAME_POSITIONED_TO_BLOCK)
		{
			UT_sint32 xFpos = pFrame->getFrameXpos();
			UT_sint32 yFpos = pFrame->getFrameYpos();
			UT_DEBUGMSG(("xFpos %d yFpos %d \n",xFpos,yFpos));
			// Now scan through the lines until we find a line below
			// yFpos

			fp_Line * pFirstLine = static_cast<fp_Line *>(getFirstContainer());
			fp_Line * pCon = pFirstLine;
			if(pCon == NULL)
			{
				return false;
			}

			UT_sint32 yoff = pCon->getHeight() + pCon->getMarginAfter();
			while ((pCon != pLastLine) && (yoff < yFpos) && (pCon->getNext()))
			{
				pCon = static_cast<fp_Line *>(pCon->getNext());
				if (!pCon->isSameYAsPrevious())
				{
					yoff += pCon->getHeight();
					yoff += pCon->getMarginAfter();
				}
			}
			if((pCon == pLastLine) && (pCon != static_cast<fp_Line *>(getLastContainer())) && (yoff < yFpos))
			{
				// Frame is not within the container so far filled.
				// try later
				continue;
			}

			//
			// Do this if we've found a line below the frame
			// 
			if(pCon && pCon != pLastLine && yoff >= yFpos)
			{
				yoff -= pCon->getHeight();
				yoff -= pCon->getMarginAfter();
				xxx_UT_DEBUGMSG(("Final yoff %d \n",yoff));
			}
			//
			// OK at this point pCon is the first line above our frame.
			// The Frame should be placed on the same page as this line
			//
			fp_Page * pPage = pCon->getPage();
			UT_sint32 Xref = pCon->getX();
			UT_sint32 Yref = pCon->getY();
			if(pPage == NULL || Yref <= -9999999)
			{
				return false;
			}
			//
			// OK now calculate the offset from the first line to this page.
			//
			UT_sint32 yLineOff,xLineOff;
			fp_VerticalContainer * pVCon = NULL;
			pVCon = (static_cast<fp_VerticalContainer *>(pCon->getContainer()));
			pVCon->getOffsets(pCon, xLineOff, yLineOff);
			UT_DEBUGMSG(("xLineOff %d yLineOff %d in block \n",xLineOff,yLineOff));
			xFpos += xLineOff - Xref; // Never use the x-position 
                                              // of the Line!!!
			yFpos += yLineOff - yoff;

			// OK, we have the X and Y positions of the frame relative to
			// the page.
			
			fp_FrameContainer * pFrameCon = getNthFrameContainer(i);
			//
			// The frame container may not yet be created.
			// 
			if(pFrameCon)
			{
				pFrameCon->setX(xFpos);
				pFrameCon->setY(yFpos);
				UT_return_val_if_fail(pPage,false);
				if(pPage->findFrameContainer(pFrameCon) < 0)
				{
					pPage->insertFrameContainer(pFrameCon);
					UT_sint32 iPrefPage = getDocLayout()->findPage(pPage);
					pFrameCon->setPreferedPageNo(iPrefPage);
				}
			}
		}
		else if(pFrame->getFramePositionTo() == FL_FRAME_POSITIONED_TO_COLUMN)
		{
			fp_FrameContainer * pFrameCon = getNthFrameContainer(i);
			//
			// The frame container may not yet be created.
			// 
			if(pFrameCon)
			{
				UT_sint32 iPrefPage = pFrameCon->getPreferedPageNo();
				UT_sint32 iPrefColumn = pFrameCon->getPreferedColumnNo();
				bool b_PrefColumnChanged = false;
				if (iPrefColumn < 0)
				{ 
					iPrefColumn = 0;
					b_PrefColumnChanged = true;
				}
				fl_DocSectionLayout *pSection = getDocSectionLayout();
				UT_sint32 numColumns = getDocSectionLayout()->getNumColumns();
				//
				// Handle case of block spanning two pages
				//
				fp_Page * pPage = NULL;
				fp_Container * pCol = NULL;
				fp_Line * pLFirst = static_cast<fp_Line *>(getFirstContainer());
				UT_return_val_if_fail(pLFirst,false);
				fp_Page * pPageFirst = pLFirst->getPage();
				UT_return_val_if_fail(pPageFirst,false);
				fp_Line * pLLast = static_cast<fp_Line *>(getLastContainer());
				UT_return_val_if_fail(pLLast,false);
				fp_Page * pPageLast = pLLast->getPage();
				if (pDL->isLayoutFilling())
				{
					fp_Page * pPageFinal = pDL->getLastPage();
					if (!pPageLast && (pDL->findPage(pPageFinal) <= iPrefPage)) 
					{
						if (pDL->findPage(pPageFinal) == iPrefPage)
						{
							UT_sint32 j=0;
							UT_sint32 k=0;
							bool b_sectionFound = false;
							for(j=0;j<pPageFinal->countColumnLeaders() && !b_sectionFound;j++)
							{
								if (pPageFinal->getNthColumnLeader(j)->getDocSectionLayout()==pSection)
								{
									b_sectionFound = true;
									fp_Container * pCol2 = pPageFinal->getNthColumnLeader(j);
									while(k < iPrefColumn && pCol2)
									{
										pCol2 = static_cast <fp_Container *> (pCol2->getNext());
										k++;
									}
								}
							}
							if (k < iPrefColumn)
							{
							// The good column has not yet been drawn
								continue;
							}
						}
						else
						{
							// The good column has not yet been drawn
							continue;
						}
					}
					else
					{
						if (pDL->findPage(pPageFirst) > iPrefPage)
						{
							if((iPrefPage >= 0) && (iPrefPage > pDL->findPage(pPageFirst) - 3))
							{
								pPage = pDL->getNthPage(iPrefPage);
							}
							else
							{
								pPage = pPageFirst;
							}
						}
						else if (pPageLast && pDL->findPage(pPageLast) < iPrefPage)
						{
							if(iPrefPage < pDL->findPage(pPageLast) + 3)
							{
								// The frame will be inserted when iPrefPage is created
								// Add it to a temporary list for now
								pDL->addFramesToBeInserted(pFrameCon);
								pPage = NULL;
							}
							else
							{
								pPage = pPageLast;
							}
						}
						else
						{
							pPage = pDL->getNthPage(iPrefPage);
						}
					}
					if (pPage) // pPage might be NULL
					{
						if (numColumns > iPrefColumn)
						{
							pCol = pPage->getNthColumn(iPrefColumn,pSection);
						}
						else
						{
							pCol = pPage->getNthColumn(numColumns-1,pSection);
							b_PrefColumnChanged = true;
						}
					}
				}
				else
				{
					UT_return_val_if_fail(pPageLast,false);
					UT_sint32 iPageFirst = getDocLayout()->findPage(pPageFirst);
					UT_sint32 iPageLast = getDocLayout()->findPage(pPageLast);
					if(pPageFirst != pPageLast)
					{
						if(iPrefPage == iPageFirst)
						{
							pPage = pPageFirst;
							fp_Column * firstColumn = static_cast <fp_Column *> (pLFirst->getColumn());
							if ((firstColumn->getColumnIndex() <= iPrefColumn) &&
								(numColumns > iPrefColumn))
							{
								pCol = pPage->getNthColumn(iPrefColumn,pSection);
							}
							else
 							{
								pCol = pLFirst->getColumn();
								b_PrefColumnChanged = true;
							}
						}
						else if(iPrefPage == iPageLast)
						{
							pPage = pPageLast;
							fp_Column * lastColumn = static_cast <fp_Column *> (pLLast->getColumn());
							if (lastColumn->getColumnIndex() >= iPrefColumn)
							{
								pCol = pPage->getNthColumn(iPrefColumn,pSection);
							}
							else
							{
								pCol = pLLast->getColumn();
								b_PrefColumnChanged = true;
							}

						}
						else if((iPrefPage >= iPageFirst) && (iPrefPage <= iPageLast))
						{
							pPage = pDL->getNthPage(iPrefPage);
							if (numColumns > iPrefColumn)
							{
								pCol = pPage->getNthColumn(iPrefColumn,pSection);
							}
							else
							{
								pCol = pPage->getNthColumn(numColumns-1,pSection);
								b_PrefColumnChanged = true;
							}
						}
						else
						{
							pPage = pPageFirst;
							pCol = pLFirst->getColumn();
						}
					}
					else
					{
						pPage = pPageFirst;
						fp_Column * firstColumn = static_cast <fp_Column *> (pLFirst->getColumn());
						fp_Column * lastColumn = static_cast <fp_Column *> (pLLast->getColumn());
						if (iPrefColumn < firstColumn->getColumnIndex())
						{
							pCol = pLFirst->getColumn();
							b_PrefColumnChanged = true;
						}
						else if (iPrefColumn > lastColumn->getColumnIndex())
						{
							pCol = pLLast->getColumn();
							b_PrefColumnChanged = true;
						}
						else
						{
							pCol = pPage->getNthColumn(iPrefColumn,pSection);
						}
					}

					UT_sint32 iGuessedPage = getDocLayout()->findPage(pPage);

					if((iPrefPage > -1) && (iPrefPage > iGuessedPage-3) && (iPrefPage < iGuessedPage+3))
					{
						fp_Page *pPrefPage = getDocLayout()->getNthPage(iPrefPage);
						if(pPrefPage && (pPage != pPrefPage))
						{
							pPage = pPrefPage;
						}
						if (numColumns > iPrefColumn)
						{
							pCol = pPage->getNthColumn(iPrefColumn,pSection);
							b_PrefColumnChanged = false;
						}
						else
						{
							pCol = pPage->getNthColumn(numColumns-1,pSection);
							b_PrefColumnChanged = true;
						}
					}
				}

				if (pCol == NULL) // this may happen if pPage is NULL
					return false;
				pFrameCon->setX(pFrame->getFrameXColpos()+pCol->getX());
				pFrameCon->setY(pFrame->getFrameYColpos()+pCol->getY());
				UT_return_val_if_fail(pPage,false);
				if(pPage && pPage->findFrameContainer(pFrameCon) < 0)
				{
					pPage->insertFrameContainer(pFrameCon);
					iPrefPage = pDL->findPage(pPage);
					pFrameCon->setPreferedPageNo(iPrefPage);
				}
				if (b_PrefColumnChanged)
				{
					fp_Column * pColumn = static_cast <fp_Column *> (pCol);
					pFrameCon->setPreferedColumnNo(pColumn->getColumnIndex());
				}
			}
		}
		else if(pFrame->getFramePositionTo() == FL_FRAME_POSITIONED_TO_PAGE)
		{
			fp_FrameContainer * pFrameCon = getNthFrameContainer(i);
			//
			// The frame container may not yet be created.
			// 
			fp_Page * pPage = NULL;
			if(pFrameCon)
			{
				//
				// Handle case of block spanning two pages
				//
				UT_sint32 iPrefPage = pFrameCon->getPreferedPageNo();
				fp_Line * pLFirst = static_cast<fp_Line *>(getFirstContainer());
				UT_return_val_if_fail(pLFirst,false);
				fp_Line * pLLast = static_cast<fp_Line *>(getLastContainer());
				UT_return_val_if_fail(pLLast,false);
				fp_Page * pPageFirst = pLFirst->getPage();
				UT_return_val_if_fail(pPageFirst,false);
				fp_Page * pPageLast = pLLast->getPage();
				if (pDL->isLayoutFilling()) 
				{
					if (!pPageLast && (pDL->findPage(pDL->getLastPage()) < iPrefPage))
					{
						// The good page has not yet been drawn
						continue;
					}
					else
					{
						if (pDL->findPage(pPageFirst) > iPrefPage)
						{
							if((iPrefPage >= 0) && (iPrefPage > pDL->findPage(pPageFirst) - 3))
							{
								pPage = pDL->getNthPage(iPrefPage);
							}
							else
							{
								pPage = pPageFirst;
							}
						}
						else if (pPageLast && pDL->findPage(pPageLast) < iPrefPage)
						{
							if(iPrefPage < pDL->findPage(pPageLast) + 3)
							{
								// The frame will be inserted when iPrefPage is created
								// Add it to a temporary list for now
								pDL->addFramesToBeInserted(pFrameCon);
								pPage = NULL;
							}
							else
							{
								pPage = pPageLast;
							}
						}
						else
						{
							pPage = pDL->getNthPage(iPrefPage);
						}
					}
				}
				else
				{
					UT_return_val_if_fail(pPageLast,false);
					pPage = pPageLast;
					if(pPageFirst != pPageLast)
					{
						UT_sint32 idLast = abs(pLLast->getY() - pFrame->getFrameYColpos());
						UT_sint32 idFirst =  abs(pLFirst->getY() - pFrame->getFrameYColpos());
						if(idFirst < idLast)
						{
							pPage = pPageFirst;
						}
					}

					UT_sint32 iGuessedPage = getDocLayout()->findPage(pPage);
					if((iPrefPage > -1) && (iPrefPage > iGuessedPage-3) && (iPrefPage < iGuessedPage+3))
					{
						pPage = getDocLayout()->getNthPage(iPrefPage);
					}
				}
				pFrameCon->setX(pFrame->getFrameXPagepos());
				pFrameCon->setY(pFrame->getFrameYPagepos());
				if(pPage && pPage->findFrameContainer(pFrameCon) < 0)
				{
					pPage->insertFrameContainer(pFrameCon);
					iPrefPage = pDL->findPage(pPage);
					pFrameCon->setPreferedPageNo(iPrefPage);
				}
			}
		}
		else
		{
			UT_DEBUGMSG(("Not implemented Yet \n"));
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}

	}
	return true;
}
/*!
 * This returns the distance from the first line in the block to the
 * line presented here.
 */
bool fl_BlockLayout::getXYOffsetToLine(UT_sint32 & xoff, UT_sint32 & yoff, fp_Line * pLine) const
{
	if(pLine == NULL)
	{
		return false;
	}
	xoff = 0;
	yoff = 0;
	fp_Line * pCon = static_cast<fp_Line *>(getFirstContainer());
	while(pCon && (pCon != pLine))
	{
		if (!pCon->isSameYAsPrevious())
		{
			yoff += pCon->getHeight();
			yoff += pCon->getMarginAfter();
		}
		pCon = static_cast<fp_Line *>(pCon->getNext());
	}
	if(pCon != pLine)
	{
		return false;
	}
	return true;
}
/*!
 * Calculate the height of the all the text contained by this block
 */
UT_sint32 fl_BlockLayout::getHeightOfBlock(bool b_withMargins)
{
	UT_sint32 iHeight = 0;
	fp_Line * pCon = static_cast<fp_Line *>(getFirstContainer());
	while(pCon)
	{
		if(!pCon->isSameYAsPrevious())
		{
			iHeight += pCon->getHeight();
			if (b_withMargins)
			{
				iHeight += pCon->getMarginBefore();
				iHeight += pCon->getMarginAfter();
			}
		}
		pCon = static_cast<fp_Line *>(pCon->getNext());
	}
	return iHeight;
}

/*!
 * Force a sectionBreak by setting StartHeight to a ridiculus value
 */
void fl_BlockLayout::forceSectionBreak(void)
{
	m_bForceSectionBreak = true;
}

/*!
 * Minimum width of a line we'll try to fit a wrapped line within
 */
UT_sint32 fl_BlockLayout::getMinWrapWidth(void) const
{
	return 4*20*4;
}

/*!
 * Reformat a block from the line given.
 */
void fl_BlockLayout::formatWrappedFromHere(fp_Line * pLine, fp_Page * pPage)
{
	//
	// Check line is in this block. It might haave been removed.
	//
	fp_Line *pCLine = static_cast<fp_Line *>(getFirstContainer());
	bool bFound = false;
	while(pCLine)
	{
		if(pCLine == pLine)
		{
			bFound = true;
			break;
		}
		pCLine = static_cast<fp_Line *>(pCLine->getNext());
	}
	if(!bFound)
	{
		_removeAllEmptyLines(); // try again
		return;
	}
	fp_Run * pRun2 = pLine->getLastRun();
	if(pLine->getHeight() == 0)
	{
		pLine->recalcHeight(pRun2);
	}
	pRun2 = pRun2->getNextRun();
	m_pVertContainer = static_cast<fp_VerticalContainer *>(pLine->getContainer());
	m_iLinePosInContainer = m_pVertContainer->findCon(pLine)+1;
	if(m_iLinePosInContainer < 0)
	{
		m_iLinePosInContainer = 0;
	}
	UT_Rect * pRec = pLine->getScreenRect();
	m_iAccumulatedHeight = pRec->top;
	UT_Rect * pVertRect = m_pVertContainer->getScreenRect();
	UT_sint32 iYBotScreen = pVertRect->top + pVertRect->height;
	xxx_UT_DEBUGMSG(("Initial m_iAccumulatedHeight %d iYBotScreen %d \n",m_iAccumulatedHeight,iYBotScreen));
	delete pVertRect;
	m_iAdditionalMarginAfter = 0;
	UT_Rect rec;
	rec.height = pRec->height;
	rec.width = pRec->width;
	rec.top = pRec->top;
	rec.left = pRec->left;
	delete pRec;
	m_bSameYAsPrevious = pLine->isSameYAsPrevious();
	UT_sint32 iHeight = pLine->getHeight() + pLine->getMarginAfter();
	//
	// Stuff remaining content on the line
	//
	while(pRun2)
	{
		pLine->addRun(pRun2);
		pRun2 = pRun2->getNextRun();
	}
	//
	// Remove all the lines after this
	//
	fp_Line * pDumLine = static_cast<fp_Line *>(pLine->getNext());
	while(pDumLine)
	{
		fp_Line * pLineToDelete = pDumLine;
		pDumLine = static_cast<fp_Line *>(pDumLine->getNext());
		pLineToDelete->setBlock(NULL);
// delete this and remove from container
		_removeLine(pLineToDelete,true,false); 
	}
	//
	// OK our line is the last line left
	//
	setLastContainer(pLine);
	//
	// OK now we have to adjust the X and max width of pLine to fit around 
	// the wrapped objects. We do this by looping though the wrapped objects
	// on the page
	//
	UT_sint32 iX = getLeftMargin();
	UT_sint32 iMaxW = m_pVertContainer->getWidth();
	iMaxW -=  getLeftMargin();
	iMaxW -= getRightMargin();
	if(pLine == static_cast<fp_Line *>(getFirstContainer()))
	{
		UT_BidiCharType iBlockDir = getDominantDirection();
		if(iBlockDir == UT_BIDI_LTR)
		{
			iMaxW -= getTextIndent();
			iX += getTextIndent();
		}
	}
	//
	// OK Now adjust the left pos to make it bump up against either the
	// the left side of the container or the previous line.
	//
	fp_Line * pPrev = static_cast<fp_Line *>(pLine->getPrev());
	UT_sint32 iWidth = 0;
	if(pPrev)
	{
		if(pLine->isSameYAsPrevious() && (pPrev->getY() == pLine->getY()))
		{
			iX = pPrev->getX() + pPrev->getMaxWidth();
			iWidth = iMaxW - iX;
		}
		else
		{
			iWidth = iMaxW;
			pLine->setSameYAsPrevious(false);
		}
	}
	else
	{
		iWidth = iMaxW;
		pLine->setSameYAsPrevious(false);
	}
	UT_sint32 xoff = rec.left - pLine->getX();
	if(iWidth < getMinWrapWidth())
	{
		xxx_UT_DEBUGMSG(("!!!!!!! ttttOOOO NAAARRRROOOWWWW iMaxX %d iX %d \n",iMaxX,iX));
		//
		// Can't fit on this line.
		// transfer to new wrapped line and delete the old one
		//
		m_iAccumulatedHeight += iHeight;
		iX = getLeftMargin();
		bool bFirst = false;
		if(pLine == static_cast<fp_Line *>(getFirstContainer()))
		{
			bFirst = true;
			UT_BidiCharType iBlockDir = getDominantDirection();
			if(iBlockDir == UT_BIDI_LTR)
			{
				iX += getTextIndent();
			}
		}
		m_bSameYAsPrevious = false;
		fp_Line * pNew = NULL;
		if(m_iAccumulatedHeight <= iYBotScreen)
		{
			pNew = getNextWrappedLine(iX,iHeight,pPage);
		}
		else
		{
			pNew = static_cast<fp_Line *>(getNewContainer());
		}
		while(pNew && (pNew->getPrev() != pLine))
		{
			pNew = static_cast<fp_Line *>(pNew->getPrev());
		}
		fp_Run * pRun = pLine->getFirstRun();
		while(pRun)
		{
			pNew->addRun(pRun);
			pRun= pRun->getNextRun();
		}
		fp_Container * pPrevLine = pLine->getPrevContainerInSection();
		if(pPrevLine && pPrevLine->getContainerType() == FP_CONTAINER_LINE)
		{
			static_cast<fp_Line *>(pPrevLine)->setAdditionalMargin(m_iAdditionalMarginAfter);
		}
		if(pPrevLine && pPrevLine->getContainerType() == FP_CONTAINER_TABLE )
		{
			static_cast<fp_TableContainer *>(pPrevLine)->setAdditionalMargin(m_iAdditionalMarginAfter);
		}
		_removeLine(pLine,true,false);
		pLine = pNew;
		if(bFirst)
		{
			setFirstContainer(pLine);
		}
	}
	else
	{
		UT_sint32 iMinLeft = BIG_NUM_BLOCKBL;
		UT_sint32 iMinWidth = BIG_NUM_BLOCKBL;
		UT_sint32 iMinRight = BIG_NUM_BLOCKBL;
		getLeftRightForWrapping(iX, rec.height,iMinLeft,iMinRight,iMinWidth);
		iX = iMinLeft - xoff;
		pLine->setX(iX);
		if(iMinWidth < getMinWrapWidth())
		{
			//
			// Can't fit on this line.
			// transfer to new wrapped line and delete the old one
			//
			xxx_UT_DEBUGMSG(("Line too narrow in formatwrapped %x block %d \n",pLine,this));
			iX = getLeftMargin();
			bool bFirst = false;
			if(pLine == static_cast<fp_Line *>(getFirstContainer()))
			{
				bFirst = true;
				UT_BidiCharType iBlockDir = getDominantDirection();
				if(iBlockDir == UT_BIDI_LTR)
				{
					iX += getTextIndent();
				}
			}
			m_iAccumulatedHeight += iHeight;
			m_bSameYAsPrevious = false;
			fp_Line * pNew = NULL;
			if(m_iAccumulatedHeight <= iYBotScreen)
			{
				pNew = getNextWrappedLine(iX,iHeight,pPage);
			}
			else
			{
				pNew = static_cast<fp_Line *>(getNewContainer());
			}
			while(pNew && static_cast<fp_Line *>(pNew->getPrev()) != pLine)
			{
				pNew = static_cast<fp_Line *>(pNew->getPrev());
			}
			fp_Run * pRun = pLine->getFirstRun();
			while(pRun)
			{
				pNew->addRun(pRun);
				pRun= pRun->getNextRun();
			}
			fp_Container * pPrevLine = pLine->getPrevContainerInSection();
			if(pPrevLine && pPrevLine->getContainerType() == FP_CONTAINER_LINE )
			{
				static_cast<fp_Line *>(pPrevLine)->setAdditionalMargin(m_iAdditionalMarginAfter);
			}
			if(pPrevLine && pPrevLine->getContainerType() == FP_CONTAINER_TABLE )
			{
				static_cast<fp_TableContainer *>(pPrevLine)->setAdditionalMargin(m_iAdditionalMarginAfter);
			}
			_removeLine(pLine,true,false);
			pLine = pNew;
			if(bFirst)
			{
				pLine->setPrev(NULL);
				setFirstContainer(pLine);
			}
		}
		else
		{
			m_bSameYAsPrevious = true;
			UT_DEBUGMSG(("Max width 1 set to %d \n",iMinWidth));
			pLine->setMaxWidth(iMinWidth);
		}
	}
	//
	// OK, Now we have one long line with all our remaining content.
	// Break it to fit in the container and around the wrapped objects
	// 
		// Reformat paragraph
	m_Breaker.breakParagraph(this, pLine,pPage);
	xxx_UT_DEBUGMSG(("Format wrapped text in blobk %x \n",this));

	pLine = static_cast<fp_Line *>(getFirstContainer());
	while(pLine)
	{
		pLine->recalcHeight();
		pLine = static_cast<fp_Line *>(pLine->getNext());
	}
	UT_ASSERT(getLastContainer());
	if(!m_pLayout->isLayoutFilling())
    {
		m_iNeedsReformat = -1;
	}
	if(m_pAlignment && m_pAlignment->getType() == FB_ALIGNMENT_JUSTIFY)
	{
		fp_Line* pLastLine = static_cast<fp_Line *>(getLastContainer());
		pLastLine->resetJustification(true); // permanent reset
	}
#if DEBUG
	if(getFirstContainer())
	{
		UT_ASSERT(getFirstContainer()->getPrev() == NULL);
	}
#endif
	return;
}

/*!
 * Given the x-position (iX) and the height of the line (iHeight) this
 * Method returns the width of the line that fits at the current screen
 * position.
 *
 * The dimensions of all the parameters are logical units.
 *
 * (input) (iX)The position relative the container holding the line of the left
 *         edge of the line.
 * (input) (iHeight) The assumed height of the line.
 *
 * (output) The width of the line that fits between the image is iMinWidth
 * (output) iMinLeft is the left-most edge of the line that fits
 * (output) iMinRight is the right-most edge of the line that fits
 */
void fl_BlockLayout::getLeftRightForWrapping(UT_sint32 iX, UT_sint32 iHeight,
											  UT_sint32 & iMinLeft,
											  UT_sint32 & iMinRight,
											  UT_sint32 & iMinWidth)
{
	UT_sint32 iMaxW = m_pVertContainer->getWidth();
	UT_sint32 iMinR = m_pVertContainer->getWidth();
	UT_sint32 iXDiff = getLeftMargin();
	GR_Graphics * pG = m_pLayout->getGraphics();
	UT_ASSERT(iHeight > 0);
	if(iHeight == 0)
	{
		if(getLastContainer())
		{
			iHeight = getLastContainer()->getHeight();
		}
		if(iHeight == 0)
		{
			iHeight = m_pLayout->getGraphics()->tlu(2);
		}
	}
	iMaxW -=  getLeftMargin();
	iMaxW -= getRightMargin();
	if (getFirstContainer() == NULL)
	{
		UT_BidiCharType iBlockDir = getDominantDirection();
		if(iBlockDir == UT_BIDI_LTR)
		{
			iMaxW -= getTextIndent();
			iXDiff += getTextIndent();
		}
	}
	UT_sint32 xoff,yoff;
	fp_Page * pPage = m_pVertContainer->getPage();
	pPage->getScreenOffsets(m_pVertContainer,xoff,yoff);
 	fp_FrameContainer * pFC = NULL;
	UT_sint32 iExpand = 0;

	UT_sint32 i = 0;
	UT_sint32 iScreenX = iX + xoff;
	UT_Rect projRec;
	bool bIsTight = false;
	iMinLeft = BIG_NUM_BLOCKBL;
	iMinWidth = BIG_NUM_BLOCKBL;
	iMinRight = BIG_NUM_BLOCKBL;
	for(i=0; i< static_cast<UT_sint32>(pPage->countAboveFrameContainers());i++)
	{
		projRec.left = iScreenX;
		projRec.height = iHeight;
		projRec.width = iMaxW;
		projRec.top = m_iAccumulatedHeight;
		m_iAdditionalMarginAfter = 0;
		pFC = pPage->getNthAboveFrameContainer(i);
		if(!pFC->isWrappingSet())
		{
			continue;
		}
		bIsTight = pFC->isTightWrapped();
		UT_Rect * pRec = pFC->getScreenRect();
		xxx_UT_DEBUGMSG(("Frame Left %d Line Left %d \n",pRec->left,iScreenX));
		fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(pFC->getSectionLayout());
		iExpand = pFL->getBoundingSpace() + 2;
		pRec->height += 2*iExpand;
		pRec->width += 2*iExpand;
		pRec->left -= iExpand;
		pRec->top -= iExpand;
		if(projRec.intersectsRect(pRec))
		{
			if(!pFC->overlapsRect(projRec)  && bIsTight)
			{
				delete pRec;
				continue;
			}
			if((!pFC->isLeftWrapped() && ((pRec->left - getMinWrapWidth() <= projRec.left +pG->tlu(1)) && (pRec->left + pRec->width) > projRec.left)) || pFC->isRightWrapped())
			{
				UT_sint32 iRightP = 0;
				if(bIsTight)
				{
					//
					// Project back into image over the transparent region
					//
					iRightP = pFC->getRightPad(m_iAccumulatedHeight,iHeight) - iExpand;
					xxx_UT_DEBUGMSG(("Projecnt Right %d \n",iRightP));
				}
				projRec.left = pRec->left + pRec->width + iRightP + pG->tlu(1);
				if(projRec.left < iMinLeft)
				{
					iMinLeft = projRec.left;
				}

			}
			else if(((pRec->left >= (projRec.left -iExpand -pG->tlu(1))) && (projRec.left + projRec.width + getMinWrapWidth() > (pRec->left -iExpand - pG->tlu(1)))) || pFC->isLeftWrapped())
			{
				UT_sint32 iLeftP = 0;
				if(bIsTight)
				{
					//
					// Project into the image over the transparent region
					//
					iLeftP = pFC->getLeftPad(m_iAccumulatedHeight,iHeight) - iExpand;
					xxx_UT_DEBUGMSG(("Project into (1) image with distance %d \n",iLeftP));
				}
				UT_sint32 diff = pRec->left - iLeftP -pG->tlu(1);
				if(diff < iMinRight)
				{
					iMinRight = diff;
				}
			}
		}
		delete pRec;
	}
	if(iMinLeft == BIG_NUM_BLOCKBL)
	{
		iMinLeft = iScreenX;
	}
	if(iMinRight == BIG_NUM_BLOCKBL)
	{
		iMinRight = iMinR + xoff;
	}
	iMinWidth = iMinRight - iMinLeft;
	if(iMinWidth < 0)
	{
		//
		// Look to see if there is some space between iMinLeft and the right
		// margin
		//
		if(iMinR + xoff - iMinLeft > getMinWrapWidth())
	   	{
			// 
			// OK we have some overlapping images. We'll take the right-most
			// edge of the available frames.
			//
			UT_sint32 iRightEdge = 0;
			fp_FrameContainer * pRightC = NULL;
			for(i=0; i< static_cast<UT_sint32>(pPage->countAboveFrameContainers());i++)
			{
				projRec.left = iScreenX;
				projRec.height = iHeight;
				projRec.width = iMaxW;
				projRec.top = m_iAccumulatedHeight;
				m_iAdditionalMarginAfter = 0;
				pFC = pPage->getNthAboveFrameContainer(i);
				if(!pFC->isWrappingSet())
				{
					continue;
				}
				bIsTight = pFC->isTightWrapped();
				UT_Rect * pRec = pFC->getScreenRect();
				fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(pFC->getSectionLayout());
				iExpand = pFL->getBoundingSpace() + 2;
				pRec->height += 2*iExpand;
				pRec->width += 2*iExpand;
				pRec->left -= iExpand;
				pRec->top -= iExpand;
				if(projRec.intersectsRect(pRec))
				{
					if(!pFC->overlapsRect(projRec)  && bIsTight)
					{
						delete pRec;
						continue;
					}
					if((pRec->left + pRec->width) > iRightEdge)
					{
						iRightEdge = pRec->left + pRec->width;
						pRightC = pFC;
					} 
				}
				delete pRec;
			}
			if(pRightC != NULL)
			{
				UT_sint32 iRightP = 0;
				if(pRightC->isTightWrapped())
				{
					//
					// Project back into image over the transparent region
					//
					iRightP = pRightC->getRightPad(m_iAccumulatedHeight,iHeight) - iExpand;	
					xxx_UT_DEBUGMSG(("Projecnt Right %d \n",iRightP));
				}
				UT_Rect * pRec = pRightC->getScreenRect();
				iMinLeft = pRec->left + pRec->width + iRightP + pG->tlu(1);
				iMinRight = iMinR + xoff;
				iMinWidth = iMinRight - iMinLeft;
			}

		} 
	}
}


/*!
 * Create a new line that will fit between positioned objects on the page.
 * iX       is the position of the last X coordinate of the previous
 *          Line relative to it's container. 
            The X location of wrapped line will be greater than this.
  * iHeight  is the assumed height of the line (at first approximation this
            is the height of the previous line).
 * pPage    Pointer to the page with the positioned objects.
 */  
fp_Line *  fl_BlockLayout::getNextWrappedLine(UT_sint32 iX,
											  UT_sint32 iHeight,
											  fp_Page * pPage)
{
	UT_sint32 iMinWidth = BIG_NUM_BLOCKBL;
	UT_sint32 iMinLeft = BIG_NUM_BLOCKBL;
	UT_sint32 iMinRight = BIG_NUM_BLOCKBL;
	fp_Line * pLine = NULL;
	UT_sint32 iXDiff = getLeftMargin();
	UT_sint32 iMinR = m_pVertContainer->getWidth();
	UT_Rect * pVertRect = m_pVertContainer->getScreenRect();
	UT_sint32 iYBotScreen = pVertRect->top + pVertRect->height;
	xxx_UT_DEBUGMSG(("Initial m_iAccumulatedHeight %d iYBotScreen %d \n",m_iAccumulatedHeight,iYBotScreen));
	delete pVertRect;
	if(m_iAccumulatedHeight > iYBotScreen)
	{
			pLine = static_cast<fp_Line *>(getNewContainer());
			m_iAccumulatedHeight += iHeight;
			pLine->setSameYAsPrevious(false);
			m_bSameYAsPrevious = false;
			return pLine;
	}

	iMinR -= getRightMargin();
	UT_sint32 xoff,yoff;
	pPage->getScreenOffsets(m_pVertContainer,xoff,yoff);
	iMinR += xoff;
	UT_sint32 iMaxW = m_pVertContainer->getWidth();
	iMaxW -=  getLeftMargin();
	iMaxW -= getRightMargin();
	fp_Line * pPrevLine = static_cast<fp_Line *>(getLastContainer());
	if (getFirstContainer() == NULL)
	{
		UT_BidiCharType iBlockDir = getDominantDirection();
		if(iBlockDir == UT_BIDI_LTR)
		{
			iMaxW -= getTextIndent();
			iXDiff += getTextIndent();
		}
	}
	if((iMinR - iX -xoff) < getMinWrapWidth())
	{
		xxx_UT_DEBUGMSG(("!!!!!!! ttttOOOO NAAARRRROOOWWWW iMaxW %d iX %d \n",iMaxW,iX));
		iX = iXDiff;
		m_iAccumulatedHeight += iHeight;
		m_iAdditionalMarginAfter += iHeight;
		m_bSameYAsPrevious = false;
	}
	else
	{
		getLeftRightForWrapping(iX, iHeight,iMinLeft,iMinRight,iMinWidth);
		if(iMinWidth <  getMinWrapWidth())
		{
			iX = getLeftMargin();
			if (getFirstContainer() == NULL)
			{
				UT_BidiCharType iBlockDir = getDominantDirection();
				if(iBlockDir == UT_BIDI_LTR)
					iX += getTextIndent();
			}
			m_iAccumulatedHeight += iHeight;
			m_iAdditionalMarginAfter += iHeight;
			m_bSameYAsPrevious = false;
		}
		else
		{
			pLine = new fp_Line(getSectionLayout());
			fp_Line* pOldLastLine = static_cast<fp_Line *>(getLastContainer());

			if(pOldLastLine == NULL)
			{
				setFirstContainer(pLine);
				setLastContainer(pLine);
				pLine->setBlock(this);
   				m_pVertContainer->insertConAt(pLine,m_iLinePosInContainer);
				m_iLinePosInContainer++;
	   			pLine->setContainer(m_pVertContainer);
				xxx_UT_DEBUGMSG(("Max width 2 set to %d \n",iMinWidth));
				pLine->setMaxWidth(iMinWidth);
				pLine->setX(iMinLeft-xoff);
				pLine->setSameYAsPrevious(false);
				pLine->setWrapped((iMaxW != iMinWidth));
				m_bSameYAsPrevious = true;
			}
			else
			{
				pLine->setPrev(getLastContainer());
				getLastContainer()->setNext(pLine);
				setLastContainer(pLine);

				fp_VerticalContainer * pContainer = static_cast<fp_VerticalContainer *>(pOldLastLine->getContainer());
				pLine->setWrapped((iMaxW != iMinWidth));
				pLine->setBlock(this);
				if(pContainer)
				{
   					pContainer->insertContainerAfter(static_cast<fp_Container *>(pLine), static_cast<fp_Container *>(pOldLastLine));
					m_iLinePosInContainer = pContainer->findCon(pLine)+1;
					pLine->setContainer(pContainer);
				}
				xxx_UT_DEBUGMSG(("Max width 3 set to %d \n",iMinWidth));
				pLine->setMaxWidth(iMinWidth);
				pLine->setX(iMinLeft-xoff);
				pLine->setSameYAsPrevious(m_bSameYAsPrevious);
				m_bSameYAsPrevious = true;
			}
			xxx_UT_DEBUGMSG(("-1- New line %x has X %d Max width %d wrapped %d sameY %d \n",pLine,pLine->getX(),pLine->getMaxWidth(),pLine->isWrapped(),pLine->isSameYAsPrevious()));
			pLine->setHeight(iHeight);
#if DEBUG
			if(getFirstContainer())
			{
				UT_ASSERT(getFirstContainer()->getPrev() == NULL);
			}
#endif
			UT_ASSERT(findLineInBlock(pLine) >= 0);
			pPrevLine->setAdditionalMargin(m_iAdditionalMarginAfter);
			return pLine;
		}
	}
	bool bStop = false;
	while(!bStop)
    {
		getLeftRightForWrapping(iX, iHeight,iMinLeft,iMinRight,iMinWidth);
		fp_Line* pOldLastLine = static_cast<fp_Line *>(getLastContainer());
		if(iMinWidth >  getMinWrapWidth())
		{
			fp_Line* pLine2 = new fp_Line(getSectionLayout());
			if(pOldLastLine == NULL)
			{
				xxx_UT_DEBUGMSG(("Old Lastline NULL?????? \n"));
				setFirstContainer(pLine2);
				setLastContainer(pLine2);
				pLine2->setBlock(this);
   				m_pVertContainer->insertConAt(pLine2,m_iLinePosInContainer);
				m_iLinePosInContainer++;
				pLine2->setContainer(m_pVertContainer);
				xxx_UT_DEBUGMSG(("Max width 4 set to %d \n",iMinWidth));
				pLine2->setMaxWidth(iMinWidth);
				pLine2->setX(iMinLeft-xoff);
				pLine2->setSameYAsPrevious(false);
				pLine2->setWrapped((iMaxW != iMinWidth));
				m_bSameYAsPrevious = true;
			}
			else
		    {
				pLine2->setPrev(getLastContainer());
				getLastContainer()->setNext(pLine2);
				setLastContainer(pLine2);
				
				fp_VerticalContainer * pContainer = static_cast<fp_VerticalContainer *>(pOldLastLine->getContainer());
				pLine2->setWrapped((iMaxW != iMinWidth));
				pLine2->setBlock(this);
				if(pContainer)
				{
			   		pContainer->insertContainerAfter(static_cast<fp_Container *>(pLine2), static_cast<fp_Container *>(pOldLastLine));
					m_iLinePosInContainer = pContainer->findCon(pLine2)+1;
					pLine2->setContainer(pContainer);
				}
				xxx_UT_DEBUGMSG(("Max width 5 set to %d \n",iMinWidth));
				pLine2->setMaxWidth(iMinWidth);
				pLine2->setX(iMinLeft-xoff);
				pLine2->setSameYAsPrevious(m_bSameYAsPrevious);
				m_bSameYAsPrevious = true;
			}
			xxx_UT_DEBUGMSG(("-2- New line %x has X %d Max width %d wrapped %d sameY %d \n",pLine2,pLine2->getX(),pLine2->getMaxWidth(),pLine2->isWrapped(),pLine2->isSameYAsPrevious()));
			pLine2->setHeight(iHeight);
			UT_ASSERT(findLineInBlock(pLine2) >= 0);
			pPrevLine->setAdditionalMargin(m_iAdditionalMarginAfter);
			return pLine2;
		}
		xxx_UT_DEBUGMSG(("Max width 6 set to %d \n",20));
#if 0
		pLine->setMaxWidth(61);
		pLine->setX(iMinLeft-xoff);
		pLine->setBlock(this);
		pLine->setSameYAsPrevious(false);
		pLine->setWrapped((iMaxW != iMinWidth));
		pOldLastLine = static_cast<fp_Line *>(getLastContainer());
		if(pOldLastLine)
		{
			pLine->setPrev(getLastContainer());
			getLastContainer()->setNext(pLine);
			setLastContainer(pLine);
			fp_VerticalContainer * pContainer = static_cast<fp_VerticalContainer *>(pOldLastLine->getContainer());
			if(pContainer)
			{
   				pContainer->insertContainerAfter(static_cast<fp_Container *>(pLine), static_cast<fp_Container *>(pOldLastLine));
				m_iLinePosInContainer = pContainer->findCon(pLine)+1;
				pLine->setContainer(pContainer);
			}
		}
		else
		{
			setFirstContainer(pLine);
			setLastContainer(pLine);
   			m_pVertContainer->insertConAt(pLine,m_iLinePosInContainer);
			m_iLinePosInContainer++;
			pLine->setContainer(m_pVertContainer);
		}
#endif
		m_bSameYAsPrevious = false;
		delete pLine;
		iX = getLeftMargin();
		m_iAccumulatedHeight += iHeight;
		m_iAdditionalMarginAfter += iHeight;
	}
	xxx_UT_DEBUGMSG(("-3- New line %x has X %d Max width %d wrapped %d sameY %d \n",pLine,pLine->getX(),pLine->getMaxWidth(),pLine->isWrapped(),pLine->isSameYAsPrevious()));
	pLine->setHeight(iHeight);
#if DEBUG
	if(getFirstContainer())
	{
		UT_ASSERT(getFirstContainer()->getPrev() == NULL);
	}
#endif
	UT_ASSERT(findLineInBlock(pLine) >= 0);
	pPrevLine->setAdditionalMargin(m_iAdditionalMarginAfter);
	return pLine;
}


void fl_BlockLayout::formatAll(void)
{
	m_iNeedsReformat = 0;
	format();
}

/*!
  Format paragraph: split the content into lines which
  will fit in the container.  */
void fl_BlockLayout::format()
{
	if((isHidden() >= FP_HIDDEN_FOLDED) || (m_pSectionLayout->isHidden() >= FP_HIDDEN_FOLDED))
	{
		xxx_UT_DEBUGMSG(("Don't format coz I'm hidden! \n"));
		return;
	}
#if 0
	if(m_pLayout->isLayoutFilling())
	{
		if(!m_bIsTOC)
		{
			if(!isNotTOCable())
			{
				m_bStyleInTOC = m_pLayout->addOrRemoveBlockFromTOC(this);
			}
		}
	}
#endif
	bool bJustifyStuff = false;
	xxx_UT_DEBUGMSG(("Format block %x needsreformat %d m_pFirstRun %x \n",this,m_iNeedsReformat,m_pFirstRun));
	fl_ContainerLayout * pCL2 = myContainingLayout();
	while(pCL2 && (pCL2->getContainerType() != FL_CONTAINER_DOCSECTION) && (pCL2->getContainerType() != FL_CONTAINER_SHADOW))
	{
		pCL2 = pCL2->myContainingLayout();
	}
	bool isInShadow = false;
	if(pCL2 && (pCL2->getContainerType() == FL_CONTAINER_SHADOW))
	{
		xxx_UT_DEBUGMSG(("Formatting a block in a shadow \n"));
		xxx_UT_DEBUGMSG(("m_pSectionLayout Type is %d \n",m_pSectionLayout->getContainerType()));
		isInShadow = true;
	}
	//
	// If block hasn't changed don't format it.
	//
	if((m_iNeedsReformat == -1) && !m_bIsCollapsed)
	{
		return;
	}
	//
	// Should be ablke to get away with just formatting from the first line 
	// containing m_iNeedsReformat
	//
	if(m_pAlignment && m_pAlignment->getType() == FB_ALIGNMENT_JUSTIFY)
	{
		m_iNeedsReformat = 0;
		bJustifyStuff = true;
	}

	//
	// Save the old height of the block. We compare to the new height after
	// the format.
	//
	UT_sint32 iOldHeight = getHeightOfBlock();
	xxx_UT_DEBUGMSG(("Old Height of block %d \n",iOldHeight));
	//
	// Need this to find where to break section in the document.
	//
	fp_Page * pPrevP = NULL;
	//
	// Sevior says...
	// Two choices of code here. "1" is mroe agressive and less likely
	// to lead to infinite loops.
	// On the down side it appears to cause pages with difficult to wrap
	// sets of images to bump all content off the page.
	//
	// If think the latter is fixable elsewhere so will try for the "1"
	// branch for now
#if 1
	fp_Container * pPrevCon = getFirstContainer();
	if(pPrevCon)
	{
		pPrevP = pPrevCon->getPage();
	}
	else
	{
		fl_BlockLayout *pPrevB = getPrevBlockInDocument();
		while(pPrevB)
		{
			pPrevCon = pPrevB->getFirstContainer();
			if(pPrevCon)
			{
				pPrevP = pPrevCon->getPage();
				break;
			}
			pPrevB = pPrevB->getPrevBlockInDocument();
		}
	}
#else
	fl_ContainerLayout * pPrevCL = getPrev();
	while(pPrevCL && pPrevCL->getContainerType() != FL_CONTAINER_BLOCK)
	{
		pPrevCL = pPrevCL->getPrev();
	}
	if(pPrevCL)
	{
		fp_Container * pPrevCon = pPrevCL->getFirstContainer();
		if(pPrevCon)
		{
			pPrevP = pPrevCon->getPage();
		}
	}
#endif
	xxx_UT_DEBUGMSG(("fl_BlockLayout - format \n"));
	_assertRunListIntegrity();
	fp_Run *pRunToStartAt = NULL;

	// TODO -- is this really needed?
	// is should not be, since _lookupProperties is explicitely called
	// by our listeners when the format changes
	// please do not uncomment this as a quick bugfix to some other
	// problem, and if you do uncomment it, please explain why - Tomas
	// lookupProperties();

	// Some fields like clock, character count etc need to be constantly updated
	// This is best done in the background updater which examines every block
	// in the document. To save scanning every run in the entire document we
	// set a bool in blocks with these sort of fields.
	//
	setUpdatableField(false);
	//
	// Save old line widths
	//
	UT_GenericVector<UT_sint32> vecOldLineWidths;
	xxx_UT_DEBUGMSG(("formatBlock 3: pPage %x \n",pPrevP));
	if (m_pFirstRun)
	{
		if(m_iNeedsReformat > 0)
		{
			// only a part of this block need reformat, find the run that
			// contains this offset
			fp_Run * pR = m_pFirstRun;
			while(pR && (pR->getBlockOffset() + pR->getLength()) <= static_cast<UT_uint32>(m_iNeedsReformat))
				pR = pR->getNextRun();

			UT_ASSERT( pR );
			pRunToStartAt = pR;
		}
		else
			pRunToStartAt = m_pFirstRun;
		
		
		//
		// Reset justification before we recalc width of runs
		//
		fp_Run* pRun = m_pFirstRun;
		//
		// Save old X position and width
		//
		fp_Line * pOldLine = NULL;
		while(pRun)
		{
			if(pOldLine != pRun->getLine())
			{
				pOldLine = pRun->getLine();
				if(pOldLine)
				{
					vecOldLineWidths.addItem(pOldLine->getWidth());
				}
			}
			if(pRun->getLine())
			{
				pRun->setTmpX(pRun->getX());
				pRun->setTmpY(pRun->getY());
				pRun->setTmpWidth(pRun->getWidth());
				pRun->setTmpLine(pRun->getLine());
			}
			else
			{
				pRun->setTmpX(0);
				pRun->setTmpY(0);
				pRun->setTmpWidth(0);
				pRun->setTmpLine(NULL);
			}
			pRun = pRun->getNextRun();
		}

		fp_Line* pLine2 = static_cast<fp_Line *>(getFirstContainer());
		while(pLine2 && 	bJustifyStuff)
		{
			pLine2->resetJustification(!bJustifyStuff); // temporary reset
			pLine2 = static_cast<fp_Line *>(pLine2->getNext());
		}

		// Recalculate widths of Runs if necessary.
		bool bDoit = false; // was false. Same kludge from
		pRun = m_pFirstRun;
		// sevior. Kludge very expensive,
		// proper fix required. Tomas

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
			if(pRun->getType() == FPRUN_HYPERLINK)
			{
				fp_HyperlinkRun * pHRun = static_cast<fp_HyperlinkRun *>(pRun);
				if(pHRun->getHyperlinkType() == HYPERLINK_ANNOTATION)
				{
					setUpdatableField(true);
				}
				if(pHRun->getHyperlinkType() == HYPERLINK_RDFANCHOR)
				{
					setUpdatableField(true);
				}
			}
			if(pRun == pRunToStartAt)
				bDoit = true;
			if(bJustifyStuff || (bDoit && (pRun->getType() != FPRUN_ENDOFPARAGRAPH)))
			{
				pRun->recalcWidth();
				xxx_UT_DEBUGMSG(("Run %x has width %d \n",pRun,pRun->getWidth()));
			}
			if(pRun->getType() == FPRUN_ENDOFPARAGRAPH)
			{
				pRun->lookupProperties();
			}
			pRun = pRun->getNextRun();
		}

		// Create the first line if necessary.
		if (!getFirstContainer())
		{
			collapse(); // remove all old content
			_stuffAllRunsOnALine();
			fp_Line * pLine = static_cast<fp_Line *>(getFirstContainer());
			pLine->resetJustification(true);
		}
		recalculateFields(0);

		// Reformat paragraph
		m_Breaker.breakParagraph(this, NULL,NULL);
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
	_assertRunListIntegrity();
	//
	// Now coalesceRuns
	//
	coalesceRuns();
	if(!bJustifyStuff && m_pAlignment && (m_pAlignment->getType() != FB_ALIGNMENT_LEFT))
	{
		//
		// If the width of the line changes for center or right justification
		// we need to clear the whole line.
		//
		fp_Line * pLine =  static_cast<fp_Line *>(getFirstContainer());
		UT_sint32 iCurLine = 0;
		while(pLine && (pLine->getContainerType() == FP_CONTAINER_LINE) && (vecOldLineWidths.getItemCount() > 0))
		{
			UT_sint32 iOldWidth = vecOldLineWidths.getNthItem(iCurLine);
			pLine->calculateWidthOfLine();
			if(iOldWidth != pLine->getWidth())
			{
				pLine->clearScreen();
			}
			pLine = static_cast<fp_Line *>(pLine->getNext());
			iCurLine++;
			if(iCurLine >= vecOldLineWidths.getItemCount())
				break;
		}
	}
	fp_Line* pLastLine = static_cast<fp_Line *>(getLastContainer());
	if(pLastLine && pLastLine->getContainerType() == FP_CONTAINER_LINE)
	{
		if(bJustifyStuff)
		{
			pLastLine->resetJustification(bJustifyStuff); // permanent reset
			pLastLine->layout();
		}
	}
	fp_Run * pRun = m_pFirstRun;
	//
	// Compare old positions and width. Clear those that don't match.
	//
	while(pRun)
	{
		pRun->clearIfNeeded();
		pRun = pRun->getNextRun();
	}

#if 0
	// we need to coalesce runs *before* we do justification (coalescing might require
	// that the whole run is reshaped, and that can lead to loss of the justification
	// information for the run).
	
    	// was previously after breakParagraph. Idea is to make this a less
		// frequent occurance. So the paragraph get's lines coalessed 
        // whenever the height changes. So we don't do this on every key press
        // but on average the paragraph gets coalessed.

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

	m_bIsCollapsed = false;
	xxx_UT_DEBUGMSG(("Block Uncollapsed in format \n"));
	//
	// Only break section if the height of the block changes.
	//
	UT_sint32 iNewHeight = 0;
	if (!m_bForceSectionBreak)
	{
		iNewHeight = getHeightOfBlock();
	}
	xxx_UT_DEBUGMSG(("New height of block %d \n",iNewHeight));
	if((m_bForceSectionBreak) || (iOldHeight != iNewHeight))
	{
		m_bForceSectionBreak = false;
		if(getSectionLayout()->getContainerType() != FL_CONTAINER_DOCSECTION)
		{
			getSectionLayout()->setNeedsReformat(this);
			if(getSectionLayout()->getContainerType() == FL_CONTAINER_CELL)
			{
				//
				// Can speed up things by doing an immediate format on the
				// the cell.
				//
				fl_CellLayout * pCL = static_cast<fl_CellLayout *>(getSectionLayout());
				if(!pCL->isDoingFormat())
				{
					getSectionLayout()->format();
				}
			}
		}
		if(!isInShadow &&  (getSectionLayout()->getContainerType() != FL_CONTAINER_FRAME))
		{
			getDocSectionLayout()->setNeedsSectionBreak(true,pPrevP);
		}
	}

	// Paragraph has been reformatted.
	if(!m_pLayout->isLayoutFilling())
    {
		m_iNeedsReformat = -1;
		getDocSectionLayout()->clearNeedsReformat(this);
	}
	else
	{
		m_iNeedsReformat = 0;
	}
	return;	// TODO return code
}

UT_sint32 fl_BlockLayout::findLineInBlock(fp_Line * pLine) const
{
	fp_Line * pTmpLine = static_cast<fp_Line *>(getFirstContainer());
	UT_sint32 i = 0;
	while(pTmpLine && pTmpLine != pLine)
	{
		i++;
		pTmpLine = static_cast<fp_Line *>(pTmpLine->getNext());
	}
	if(pTmpLine == NULL)
	{
		return -1;
	}
	return i;
}

void fl_BlockLayout::markAllRunsDirty(void)
{
	fp_Run * pRun = m_pFirstRun;
	while(pRun)
	{
		pRun->markAsDirty();
		pRun = pRun->getNextRun();
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
	
	xxx_UT_DEBUGMSG(("redrawUpdate Called \n"));
	// TODO -- is this really needed ??
	// we should not need to lookup properties on redraw,
	// lookupProperties() gets explicitely called by our listeners
	// when format changes.
	// please do not uncomment this as a quick bugfix to some other
	// problem, and if you do uncomment it, please explain why - Tomas
	// lookupProperties();
	if(isHdrFtr())
		return;

	if(needsReformat())
	{
		xxx_UT_DEBUGMSG(("redrawUpdate Called doing format \n"));
		format();
		if(m_pAlignment && m_pAlignment->getType() == FB_ALIGNMENT_JUSTIFY)
		{
			markAllRunsDirty();
			fp_Line* pLine = static_cast<fp_Line *>(getFirstContainer());
			while (pLine)
			{
				xxx_UT_DEBUGMSG(("Drawing line in redraw update after format %x \n",pLine));
				pLine->draw(m_pFirstRun->getGraphics());
				pLine = static_cast<fp_Line *>(pLine->getNext());
			}
			m_bNeedsRedraw = false;
			return;
		}
	}
			
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

	//	lookupProperties();
}

fp_Container* fl_BlockLayout::getNewContainer(fp_Container * /* pCon*/)
{
	fp_Line* pLine = new fp_Line(getSectionLayout());
	// TODO: Handle out-of-memory
	UT_ASSERT(pLine);
	fp_TableContainer * pPrevTable = NULL;
	fp_TOCContainer * pPrevTOC = NULL;
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
				if(ppPrev && ((ppPrev->getContainerType() == FP_CONTAINER_ENDNOTE) || (ppPrev->getContainerType() == FP_CONTAINER_FOOTNOTE)  || (ppPrev->getContainerType() == FP_CONTAINER_ANNOTATION) || (ppPrev->getContainerType() == FP_CONTAINER_FRAME) ))
				{
					fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(ppPrev->getSectionLayout());
					while(pCL && ((pCL->getContainerType() == FL_CONTAINER_FOOTNOTE) || (pCL->getContainerType() == FL_CONTAINER_ENDNOTE) || (pCL->getContainerType() == FL_CONTAINER_ANNOTATION)|| (pCL->getContainerType() == FL_CONTAINER_FRAME)))
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
				else if(ppPrev && (ppPrev->getContainerType() == FP_CONTAINER_TOC))
				{
					pContainer = (fp_VerticalContainer *) ppPrev->getContainer();
					pPrevLine = NULL;
					pPrevTOC = (fp_TOCContainer*)ppPrev;
				}
				else
				{
					pPrevLine = NULL;
					pContainer = NULL;
				}
			}
		}
		else 
		{
			//
			// Skip any footnotes or endnotes
			//
			fl_ContainerLayout * pCL = getNext();
			while(pCL && ((pCL->getContainerType() == FL_CONTAINER_ENDNOTE)
						  || (pCL->getContainerType() == FL_CONTAINER_FOOTNOTE)
						  || (pCL->getContainerType() == FL_CONTAINER_ANNOTATION))
				  )
			{
				pCL = pCL->getNext();
			}
			if (pCL && pCL->getFirstContainer() && pCL->getFirstContainer()->getContainer())
			{
				pContainer = static_cast<fp_VerticalContainer *>(pCL->getFirstContainer()->getContainer());
				UT_return_val_if_fail(pContainer, NULL);
				UT_ASSERT_HARMLESS(pContainer->getWidth() >0);
			}
			else if (myContainingLayout()->getFirstContainer())
			{
			// TODO assert something here about what's in that container
				pContainer = static_cast<fp_VerticalContainer *>(myContainingLayout()->getFirstContainer());
				UT_return_val_if_fail(pContainer, NULL);
				UT_ASSERT_HARMLESS(pContainer->getWidth() >0);
			}
			else
			{
				pContainer = static_cast<fp_VerticalContainer *>(myContainingLayout()->getNewContainer());
				UT_return_val_if_fail(pContainer, NULL);
				UT_ASSERT_HARMLESS(pContainer->getWidth() >0);
			}
		}
		if(pContainer == NULL)
		{
			pContainer = static_cast<fp_VerticalContainer *>(m_pSectionLayout->getNewContainer());
			UT_return_val_if_fail(pContainer, NULL);
			UT_ASSERT_HARMLESS(pContainer->getWidth() >0);
		}

		if ((pPrevLine==NULL) && (pPrevTable== NULL) && (pPrevTOC == NULL))
		{
			pContainer->insertContainer(static_cast<fp_Container *>(pLine));
		}
		else if((pPrevLine==NULL) &&(NULL!=pPrevTable))
		{
			pContainer->insertContainerAfter((fp_Container *)pLine, (fp_Container *) pPrevTable);
		}
		else if((pPrevLine==NULL) &&(NULL!=pPrevTOC))
		{
			pContainer->insertContainerAfter((fp_Container *)pLine, (fp_Container *) pPrevTOC);
		}
		else
		{
			pContainer->insertContainerAfter(static_cast<fp_Container *>(pLine), static_cast<fp_Container *>(pPrevLine));
		}
	}
	UT_ASSERT(pLine->getContainer());
#if DEBUG
	if(getFirstContainer())
	{
		UT_ASSERT(getFirstContainer()->getPrev() == NULL);
	}
#endif
	UT_ASSERT(findLineInBlock(pLine) >= 0);
	pLine->recalcMaxWidth(true);
	return static_cast<fp_Container *>(pLine);
}

void fl_BlockLayout::setNeedsReformat(fl_ContainerLayout * pCL,UT_uint32 offset)
{
	// _lesser_ value is the one that matter here, Tomas, Nov 28, 2003
	if(m_iNeedsReformat < 0 || static_cast<UT_sint32>(offset) < m_iNeedsReformat)
		m_iNeedsReformat = offset;
  	getSectionLayout()->setNeedsReformat(pCL);
	setNeedsRedraw();
}

void fl_BlockLayout::clearPrint(void) const
{
	fp_Run * pRun = getFirstRun();
	while(pRun)
	{
		pRun->clearPrint();
		pRun = pRun->getNextRun();
	}
}

void fl_BlockLayout::setNeedsRedraw(void)
{
	m_bNeedsRedraw = true;
	getSectionLayout()->setNeedsRedraw();
}

const char* fl_BlockLayout::getProperty(const gchar * pszName, bool bExpandStyles) const
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	getAP(pBlockAP);

	// at the moment this is only needed in the bidi build, where dom-dir property
	// can be inherited from the section; however, it the future this might need to
	// be added for the normal build too.
	m_pSectionLayout->getAP(pSectionAP);

	return PP_evalProperty(pszName,pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
}

/*!
 * This method returns the length of the Block, including the initial strux.
 * so if "i" is the position of the block strux, i+getLength() will be the 
 * position of the strux (whatever it might be), following this block.
 * The length includes any embedded struxes (like footnotes and endnotes).
 */
UT_sint32 fl_BlockLayout::getLength() const
{
	PT_DocPosition posThis = getPosition(true);
	pf_Frag_Strux* nextSDH =NULL;
	m_pDoc->getNextStrux(getStruxDocHandle(),&nextSDH);
	if(nextSDH == NULL)
	{
		//
		// Here if we reach EOD.
		//
		PT_DocPosition docEnd;
		m_pDoc->getBounds(true, docEnd);
		UT_sint32 length = static_cast<UT_sint32>(docEnd) - static_cast<UT_sint32>(posThis);
		return length;
	}
	PT_DocPosition posNext = m_pDoc->getStruxPosition(nextSDH);
	//
	// OK Look to see if we've got a TOC/ENDTOC at the end. If so subtract
	// it's length.
	//
	pf_Frag * pf = m_pDoc->getFragFromPosition(posNext-1);
	if(pf->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfsTemp = static_cast<pf_Frag_Strux *>(pf);
		if (pfsTemp->getStruxType() == PTX_EndTOC)	// did we find it
		{
			posNext -= 2;
		}
	}

	UT_sint32 length = static_cast<UT_sint32>(posNext) - static_cast<UT_sint32>(posThis);
	return length;
}

std::unique_ptr<PP_PropertyType> fl_BlockLayout::getPropertyType(const gchar * pszName, tProperty_type Type, bool bExpandStyles) const
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	getAP(pBlockAP);

	return PP_evalPropertyType(pszName,pSpanAP,pBlockAP,pSectionAP,Type,m_pDoc,bExpandStyles);
}

/*!
 Get block's position in document
 \param bActualBlockPos When true return block's position. When false
						return position of first run in block
 \return Position of block (or first run in block)
 \fixme Split in two functions if called most often with FALSE
*/
PT_DocPosition fl_BlockLayout::getPosition(bool bActualBlockPos) const
{
	PT_DocPosition pos = m_pDoc->getStruxPosition(getStruxDocHandle());

	// it's usually more useful to know where the runs start
	if (!bActualBlockPos)
		pos += fl_BLOCK_STRUX_OFFSET;

	return pos;
}

void fl_BlockLayout::getLineSpacing(double& dSpacing, eSpacingPolicy& eSpacing) const
{
	dSpacing = m_dLineSpacing;
	eSpacing = m_eSpacingPolicy;
}

bool	fl_BlockLayout::getBlockBuf(UT_GrowBuf * pgb) const
{
	return m_pDoc->getBlockBuf(getStruxDocHandle(), pgb);
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
								bool& bDirection) const
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
	while (pRun->getNextRun() && pRun->getBlockOffset() < iRelOffset)
	{
		pRun = pRun->getNextRun();
	}
	// Now scan farther if necessary - the block may contain Runs
	// with zero length. This is only a problem when empty Runs
	// appear for no good reason (i.e., an empty Run on an empty
	// line should be OK).
	// 
	// The original test for block offset + len < iRelOffset was no
	// good as that condition is always false by the time we get here.
 	// The test would need to be for length == 0
	// however, testing for 0 length makes us skip over fmt marks,
	// which we do not want (I wonder if this is really needed at all)
	while (pRun->getNextRun() && pRun->getLength() == 0 && pRun->getType() != FPRUN_FMTMARK)
	{
		pRun = pRun->getNextRun();
	}

	// We may have scanned past the last Run in the block. Back up.
	if (!pRun)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		pRun = static_cast<fp_Line *>(getLastContainer())->getLastRun();
		bCoordOfPrevRun = false;
	}

	// Step one back if previous Run holds the offset (the
	// above loops scan past what we're looking for since it's
	// faster).
	fp_Run* pPrevRun = pRun->getPrevRun();

	if (pPrevRun &&
		pPrevRun->getBlockOffset() + pPrevRun->getLength() > iRelOffset)
	{
		pRun = pPrevRun;
		bCoordOfPrevRun = false;
	}

	// Since the requested offset may be a page break (or similar
	// Runs) which cannot contain the point, now work backwards
	// while looking for a Run which can contain the point.
	if(pRun && !pRun->canContainPoint())
	{
		fp_Run * pOldRun = pRun;
		
		while (pRun && !pRun->canContainPoint())
		{
			pRun = pRun->getPrevRun();
			bCoordOfPrevRun = false;
		}

		if(!pRun)
		{
			//look the other way
			pRun = pOldRun;

			while (pRun && !pRun->canContainPoint())
			{
				pRun = pRun->getNextRun();
				bCoordOfPrevRun = false;
			}
		}
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
			pPrevRun = pRun->getPrevRun();
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
					   (!pPrevRun->letPointPass()
					   || !pPrevRun->canContainPoint()))
				{
					pPrevRun = pPrevRun->getPrevRun();
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
	pPrevRun = pRun->getPrevRun();
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
		pPrevRun = pPrevRun->getPrevRun();
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

fp_Line* fl_BlockLayout::findPrevLineInDocument(fp_Line* pLine) const
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

			// is this cast safe? Could not some other layout class be returned?
			// if this assert fails, then this code needs to be fixed up. Tomas
			UT_ASSERT_HARMLESS( pSL->getLastLayout() && pSL->getLastLayout()->getContainerType() == FL_CONTAINER_BLOCK );
			fl_BlockLayout* pBlock = static_cast<fl_BlockLayout *>(pSL->getLastLayout());
			UT_return_val_if_fail(pBlock, NULL);
			return static_cast<fp_Line *>(pBlock->getLastContainer());
		}
	}

	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

fp_Line* fl_BlockLayout::findNextLineInDocument(fp_Line* pLine) const
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
		const fl_SectionLayout* pSL = static_cast<const fl_SectionLayout*>(m_pSectionLayout->getNext());

		if (!pSL)
		{
			// at EOD, so just bail
			return NULL;
		}

		// is this cast safe? Could not some other layout class be returned?
		// if this assert fails, then this code needs to be fixed up. Tomas
		UT_ASSERT_HARMLESS( pSL->getLastLayout() && pSL->getLastLayout()->getContainerType() == FL_CONTAINER_BLOCK );
			
		const fl_BlockLayout* pBlock = static_cast<const fl_BlockLayout*>(pSL->getFirstLayout());
		UT_return_val_if_fail(pBlock, NULL);
		return static_cast<fp_Line *>(pBlock->getFirstContainer());
	}

	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

/*****************************************************************/
/*****************************************************************/

fl_PartOfBlock::fl_PartOfBlock(void)
{
	m_iOffset = 0;
	m_iPTLength = 0;
	m_bIsIgnored = false;
	m_bIsInvisible = false;
}

fl_PartOfBlock::fl_PartOfBlock(UT_sint32 iOffset, UT_sint32 iPTLength,
							   bool bIsIgnored /* = false */):
	m_iOffset(iOffset),
	m_iPTLength(iPTLength),
	m_bIsIgnored(bIsIgnored),
	m_bIsInvisible(false)
{
}

void fl_PartOfBlock::setGrammarMessage(UT_UTF8String & sMsg)
{
	m_sGrammarMessage = sMsg;
}

void fl_PartOfBlock::getGrammarMessage(UT_UTF8String & sMsg) const
{
	sMsg = m_sGrammarMessage;
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
	end1 = m_iOffset + m_iPTLength;

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




#ifdef ENABLE_SPELL
/*!
 Recalculate boundries for pending word
 \param iOffset Offset of change
 \param chg Size of change, negative is removal, zero is for
			recalculating the pending word.

 On entry, the block is already changed and any pending word is junk.
 On exit, there's either a single unchecked pending word, or nothing.
*/
void
fl_BlockLayout::_recalcPendingWord(UT_uint32 iOffset, UT_sint32 chg) const
{
	xxx_UT_DEBUGMSG(("fl_BlockLayout::_recalcPendingWord(%d, %d)\n",
					 iOffset, chg));

	UT_GrowBuf pgb(1024);
	bool bRes = getBlockBuf(&pgb);
	UT_UNUSED(bRes);
	UT_ASSERT(bRes);

	const UT_UCSChar* pBlockText = reinterpret_cast<UT_UCSChar*>(pgb.getPointer(0));
	if (pBlockText == NULL)
	{
		return;
	}

	UT_uint32 iFirst = iOffset;

	if (iFirst > pgb.getLength() - 1)
		iFirst = pgb.getLength() - 1;

	UT_uint32 iAbs = static_cast<UT_uint32>((chg >= 0) ? chg : -chg);
	UT_sint32 iLen = ((chg > 0) ? iAbs : 0);

	// We expand this region outward until we get a word delimiter on
	// each side.

	// First, look towards the start of the buffer
	while ((iFirst > 1) 
		   && !isWordDelimiter(pBlockText[iFirst-1], pBlockText[iFirst] ,pBlockText[iFirst-2], iFirst-1))
	{
		iFirst--;
	}

	if(iFirst == 1 && !isWordDelimiter(pBlockText[0], pBlockText[1], UCS_UNKPUNK, iFirst))
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

		if (isWordDelimiter(pBlockText[iFirst + iLen], followChar, prevChar, iFirst+iLen)) break;
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
			if (isWordDelimiter(currentChar, followChar,prevChar,iLast)) break;
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

		if (!isWordDelimiter(currentChar, followChar, prevChar,iFirst)) break;
		iFirst++;
		iLen--;
	}

	// Is there a pending word left? If so, record the details.
	if (iLen)
	{
		fl_PartOfBlock* pPending = NULL;

		if (m_pLayout->isPendingWordForSpell())
		{
			pPending = m_pLayout->getPendingWordForSpell();
			UT_ASSERT(pPending);
		}

		if (!pPending)
		{
			pPending = new fl_PartOfBlock();
			UT_ASSERT(pPending);
		}

		if (pPending)
		{
			pPending->setOffset(iFirst);
			pPending->setPTLength(iLen); // not sure about this ...
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
bool fl_BlockLayout::checkSpelling(void)
{

	xxx_UT_DEBUGMSG(("fl_BlockLayout::checkSpelling: this 0x%08x isOnScreen(): %d\n", this,static_cast<UT_uint32>(isOnScreen())));
	// Don't spell check non-formatted blocks!
	if(m_pFirstRun == NULL)
		return false;
	if(m_pFirstRun->getLine() == NULL)
		return false;

	// we only want to do the cursor magic if the cursor is in this block
	bool bIsCursorInBlock = false;
	FV_View* pView = getView();
	fp_Run* pLastRun = m_pFirstRun;

	while(pLastRun && pLastRun->getNextRun())
		pLastRun = pLastRun->getNextRun();
	
	
	if(pView && pLastRun)
	{
		UT_uint32 iBlPosStart = static_cast<UT_uint32>(getPosition());
		UT_uint32 iBlPosEnd   = iBlPosStart + pLastRun->getBlockOffset() + pLastRun->getLength();
		UT_uint32 iPos   = static_cast<UT_uint32>(pView->getPoint()); 

		bIsCursorInBlock = ((iPos >= iBlPosStart) && (iPos <= iBlPosEnd));
	}
	
	// Remove any existing squiggles from the screen...
	bool bUpdateScreen = m_pSpellSquiggles->deleteAll();

	// Now start checking
	bUpdateScreen |= _checkMultiWord(0, -1, bIsCursorInBlock);
	if( bUpdateScreen && pView)
	{
		markAllRunsDirty();
		setNeedsRedraw();
	}
	return true;
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
								bool bToggleIP) const
{
	xxx_UT_DEBUGMSG(("fl_BlockLayout::_checkMultiWord\n"));

	bool bScreenUpdated = false;

	fl_BlockSpellIterator wordIterator(this, iStart);

	const UT_UCSChar* pWord;
	UT_sint32 iLength, iBlockPos, iPTLength;

	while (wordIterator.nextWordForSpellChecking(pWord, iLength, iBlockPos, iPTLength))
	{
		// When past the provided end position, break out
		if (eor > 0 && iBlockPos > eor) break;

		fl_PartOfBlock* pPOB = new fl_PartOfBlock(iBlockPos, iPTLength);
		UT_ASSERT(pPOB);

#if 0 // TODO: turn this code on someday
		FV_View* pView = getView();
		XAP_App * pApp = XAP_App::getApp();
		XAP_Prefs *pPrefs = pApp->getPrefs();
		UT_ASSERT(pPrefs);

		bool b;

		// possibly auto-replace the squiggled word with a suggestion
		if (pPrefs->getPrefsValueBool(static_cast<gchar*>(AP_PREF_KEY_SpellAutoReplace), &b))
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
			bool bwrong = false;
			bwrong = _doCheckWord(pPOB, pWord, iLength, true, bToggleIP);
#if 0
			if(bwrong)
			{
				UT_DEBUGMSG(("Found misspelt word in block %x \n",this));
			}
#endif
			bScreenUpdated |= bwrong;
		}
	}

	return bScreenUpdated;
}

/*!
 Validate a word and spell-check it
 \param pPOB Block region to squiggle if appropriate
 \param pWord Pointer to the word
 \param iLength length of the word in pWord
 \param bAddSquiggle True if pPOB should be added to squiggle list
 \return True if display was updated, otherwise false

 If the word bounded by pPOB is not squiggled, the pPOB is deleted.

 I added the iLength parameter, because we need to store the PieceTable length in pPOB and
 as this is about the only function that need to know the word length, there is no point
 to store it in the POB (which gets queued in the layout). Tomas, May 9, 2005
 */

bool
fl_BlockLayout::_doCheckWord(fl_PartOfBlock* pPOB,
							 const UT_UCSChar* pWord,
							 UT_sint32 iLength,
							 bool bAddSquiggle /* = true */,
							 bool bClearScreen /* = true */) const
{
	UT_sint32 iBlockPos = pPOB->getOffset();

	do {
		// Spell check the word, return if correct
		if (_spellCheckWord(pWord, iLength, iBlockPos))
			break;

		// Find out if the word is in the document's list of ignored
		// words
		pPOB->setIsIgnored(_getSpellChecker(iBlockPos)->isIgnored(pWord, iLength));

		// Word not correct or recognized, so squiggle it
		if (bAddSquiggle)
		{
			m_pSpellSquiggles->add(pPOB);
		}

		if(bClearScreen)
		{
			m_pSpellSquiggles->clear(pPOB);
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
fl_BlockLayout::checkWord(fl_PartOfBlock* pPOB) const
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
	UT_sint32 iLength, iBlockPos, iPTLength;

    // The word iterator may be unable to find a word within the
    // editing limits provided by the pPOB - so check that before
    // continuing.
    if (wordIterator.nextWordForSpellChecking(pWord,
                                              iLength, iBlockPos, iPTLength)
        && (iBlockPos+iLength <= pPOB->getOffset()+pPOB->getPTLength()))
    {
        delete pPOB;

        fl_PartOfBlock* pNewPOB = new fl_PartOfBlock(iBlockPos, iPTLength);
        UT_ASSERT(pNewPOB);
            
        return _doCheckWord(pNewPOB, pWord, iLength );
    }

	// Delete the POB which is not longer needed
	delete pPOB;
	return false;
}
#endif
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

	// This is needed because fl_BlockLayout::format() can be triggered by a timer
	// half-way through populating a block. If that happens the format clears the flag
	// and any runs that get inserted in subsequent populate calls are not correctly
	// positioned. It might be desirable to have a mechanism to ignore format() calls
	// while populating (for example storing sdh in a static member on populateStrux)
	// but calling setNeedsReformat() costs us little and will do OK for now.
	// Tomas, Apr 23, 2004
	setNeedsReformat(this,blockOffset);
	updateEnclosingBlockIfNeeded();
	if(isHidden() == FP_HIDDEN_FOLDED)
	{
		collapse();
	}
	return true;
}

bool   fl_BlockLayout::itemizeSpan(PT_BlockOffset blockOffset, UT_uint32 len,GR_Itemization & I)
{
	UT_return_val_if_fail( m_pLayout, false );
	PD_StruxIterator text(getStruxDocHandle(),
						  blockOffset + fl_BLOCK_STRUX_OFFSET,
						  blockOffset + fl_BLOCK_STRUX_OFFSET + len - 1);
	I.setDirOverride(m_iDirOverride);
	I.setEmbedingLevel(m_iDomDirection);

	bool bShowControls = false;
	FV_View* pView = getView();
	if(pView && pView->getShowPara())
		bShowControls = true;
	
	I.setShowControlChars(bShowControls);

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	getSpanAP(blockOffset, false, pSpanAP);
	getAP(pBlockAP);
	const char * szLang = static_cast<const char *>(PP_evalProperty("lang",
																	pSpanAP,
																	pBlockAP,
																	NULL,
																	m_pDoc,
																	true));

	const GR_Font * pFont = m_pLayout->findFont(pSpanAP,
												pBlockAP,
												NULL,
												m_pLayout->getGraphics());

	xxx_UT_DEBUGMSG(("Got [%s], %s\n", pFont->getFamily(), szLang));
	I.setLang(szLang);
	I.setFont(pFont);
	
	m_pLayout->getGraphics()->itemize(text, I);
	return true;
}

bool	fl_BlockLayout::_doInsertTextSpan(PT_BlockOffset blockOffset, UT_uint32 len)
{

	xxx_UT_DEBUGMSG(("_doInsertTextSpan: Initial offset %d, len %d bl_Length %d \n", blockOffset, len,getLength()));
	GR_Itemization I;
	bool b= itemizeSpan(blockOffset, len,I);
	UT_return_val_if_fail( b, false );

	for(UT_sint32 i = 0; i < I.getItemCount() - 1; ++i)
	{
		UT_uint32 iRunOffset = I.getNthOffset(i);
		UT_uint32 iRunLength = I.getNthLength(i);

		// because of bug 8542 we do not allow runs longer than 32000 chars, so
		// if it is longer, just split it (we do not care where we split it,
		// this is a contingency measure only). Lowered to 16000 because of
		// bug 13709.
		while(iRunLength)
		{
			UT_uint32 iRunSegment = UT_MIN(iRunLength, 16000);
			
			fp_TextRun* pNewRun = new fp_TextRun(this, blockOffset + iRunOffset, iRunSegment);
			iRunOffset += iRunSegment;
			iRunLength -= iRunSegment;
			
			UT_return_val_if_fail(pNewRun && pNewRun->getType() == FPRUN_TEXT, false);
			pNewRun->setDirOverride(m_iDirOverride);

			GR_Item * pItem = I.getNthItem(i)->makeCopy();
			UT_ASSERT( pItem );
			pNewRun->setItem(pItem);
		
			if(!_doInsertRun(pNewRun))
				return false;
		}
		
	}

	return true;
}

bool	fl_BlockLayout::_doInsertForcedLineBreakRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = NULL;
	if(isContainedByTOC())
	{
		pNewRun = new fp_DummyRun(this,blockOffset);
	}
	else
	{
		pNewRun = new fp_ForcedLineBreakRun(this, blockOffset, 1);
	}
	UT_ASSERT(pNewRun); // TODO check for outofmem

	bool bResult = _doInsertRun(pNewRun);
	if (bResult && !isContainedByTOC())
		_breakLineAfterRun(pNewRun);

	return bResult;
}

bool    fl_BlockLayout::_doInsertDirectionMarkerRun(PT_BlockOffset blockOffset, UT_UCS4Char iM)
{
	xxx_UT_DEBUGMSG(("fl_BlockLayout::_doInsertDirectionMarkerRun: offset %d, marker 0x%04x\n",
				 blockOffset, iM));
	
	fp_Run * pNewRun = new fp_DirectionMarkerRun(this, blockOffset, iM);
	UT_ASSERT( pNewRun );

	bool bResult = _doInsertRun(pNewRun);
#if 0
	if (bResult)
		_breakLineAfterRun(pNewRun);
#endif
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
	while (pRun->getNextRun() && (pRun->getBlockOffset() != blockOffset || pRun->getType() == FPRUN_FMTMARK))
	{
		pRun = pRun->getNextRun();
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
		m_pFirstRun = pB1->getNextRun();
	}

	pRun = pB1->getNextRun();
	pB1->unlinkFromRunList();
	delete pB1;

	fp_Run * pLastRun = static_cast<fp_Line *>(getLastContainer())->getLastRun();
	while(pRun )
	{
		pRun->setBlockOffset(pRun->getBlockOffset() - 1);
		if(pRun == pLastRun)
			break;
		pRun = pRun->getNextRun();
	}

	xxx_UT_DEBUGMSG(("fl_BlockLayout::_deleteBookmarkRun: assert integrity (1)\n"));
	_assertRunListIntegrity();

	return true;
}
#endif

bool	fl_BlockLayout::_doInsertBookmarkRun(PT_BlockOffset blockOffset)
{
	fp_Run * pNewRun;
	
	if(!isContainedByTOC())
	{
		pNewRun = new fp_BookmarkRun(this, blockOffset, 1);
	}
	else
	{
		pNewRun = new fp_DummyRun(this,blockOffset);
	}
	
	UT_ASSERT(pNewRun);
	bool bResult = _doInsertRun(pNewRun);
#if 0
	if (bResult)
	{
		_breakLineAfterRun(pNewRun);
	}
#endif
	return bResult;

}

void    fl_BlockLayout::_finishInsertHyperlinkedNewRun( PT_BlockOffset /*blockOffset*/,
														fp_HyperlinkRun* pNewRun )
{
	// if this is the start of the hyperlink, we need to mark all the runs
	// till the end of it
	// if this is because of an insert operation, the end run is already
	// in place, because we insert them in that order; if it is because of
	// append, there is no end run, but then this is the last run; the other
	// runs will get marked as they get appended (inside fp_Run::insertRun...)
	// any hyperlink run will not get its m_pHyperlink set, so that
	// runs that follow it would not be marked

	if(pNewRun->isStartOfHyperlink())
	{
		fp_Run * pRun = pNewRun->getNextRun();
		UT_ASSERT(pRun);
		// when loading a document the opening hyperlink run is initially followed
		// by ENDOFPARAGRAPH run; we do not want to set this one
		while(pRun && pRun->getType() != FPRUN_HYPERLINK && pRun->getType() != FPRUN_ENDOFPARAGRAPH)
		{
			pRun->setHyperlink(pNewRun);
			pRun = pRun->getNextRun();
		}
	}
	else
	{
		//
		// clear out any hyperlinks
		//
		fp_Run * pRun = pNewRun->getNextRun();
		while(pRun && (pRun->getType() != FPRUN_HYPERLINK && pRun->getType() != FPRUN_ENDOFPARAGRAPH))
		{
			pRun->setHyperlink(NULL);
			pRun = pRun->getNextRun();
		}
	}
	//_breakLineAfterRun(pNewRun);
}

bool	fl_BlockLayout::_doInsertHyperlinkRun(PT_BlockOffset blockOffset)
{
	bool bResult = false;
	
	if(!isContainedByTOC())
	{
		fp_HyperlinkRun * pNewRun =  new fp_HyperlinkRun(this, blockOffset, 1);
		UT_ASSERT(pNewRun);
		bResult = _doInsertRun(pNewRun);

		if (bResult)
		{
			_finishInsertHyperlinkedNewRun( blockOffset, pNewRun );
		}
	}
	else
	{
		fp_Run * pNewRun = new fp_DummyRun(this,blockOffset);
		UT_ASSERT(pNewRun);
		bResult = _doInsertRun(pNewRun);
	}
	

	return bResult;

}


bool	fl_BlockLayout::_doInsertAnnotationRun(PT_BlockOffset blockOffset)
{
	bool bResult = false;
	
	if(!isContainedByTOC())
	{
		fp_AnnotationRun * pNewRun =  new fp_AnnotationRun(this, blockOffset, 1);
		UT_ASSERT(pNewRun);
		bResult = _doInsertRun(pNewRun);

		if (bResult)
		{
			_finishInsertHyperlinkedNewRun( blockOffset, pNewRun );
		}
	}
	else
	{
		fp_Run * pNewRun = new fp_DummyRun(this,blockOffset);
		UT_ASSERT(pNewRun);
		bResult = _doInsertRun(pNewRun);
	}
	

	return bResult;

}


/**
 * Note that _doInsertHyperlinkRun(), _doInsertAnnotationRun,
 * and _doInsertRDFAnchorRun() all work on contained information.
 * Each of these methods use setHyperlink() on the runs.
 */
bool fl_BlockLayout::_doInsertRDFAnchorRun(PT_BlockOffset blockOffset)
{
	bool bResult = false;
	
	if( isContainedByTOC() )
	{
		fp_Run * pNewRun = new fp_DummyRun(this,blockOffset);
		UT_ASSERT(pNewRun);
		bResult = _doInsertRun(pNewRun);
	}
	else
	{
		fp_RDFAnchorRun * pNewRun = new fp_RDFAnchorRun(this, blockOffset, 1);
		UT_ASSERT(pNewRun);
		bResult = _doInsertRun(pNewRun);

		if (bResult)
		{
			_finishInsertHyperlinkedNewRun( blockOffset, pNewRun );
		}
	}
	
	return bResult;

}


bool	fl_BlockLayout::_doInsertFieldStartRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_FieldStartRun(this,blockOffset, 1);
	UT_ASSERT(pNewRun); // TODO check for outofmem

	bool bResult = _doInsertRun(pNewRun);
	if (bResult)
		_breakLineAfterRun(pNewRun);

	return bResult;
}

bool	fl_BlockLayout::_doInsertFieldEndRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_FieldEndRun(this, blockOffset, 1);
	UT_ASSERT(pNewRun); // TODO check for outofmem

	bool bResult = _doInsertRun(pNewRun);
	if (bResult)
		_breakLineAfterRun(pNewRun);

	return bResult;
}

/*!
 * Returns true if this run is at the last position of the block.
 */
bool fl_BlockLayout::isLastRunInBlock(fp_Run * pRun) const
{
	if(((UT_sint32)pRun->getBlockOffset()+2) == getLength())
	{
		return true;
	}
	return false;
}

bool	fl_BlockLayout::_doInsertForcedPageBreakRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = NULL;
	if(isContainedByTOC())
	{
		pNewRun = new fp_DummyRun(this,blockOffset);
	}
	else
	{
		pNewRun = new fp_ForcedPageBreakRun(this,blockOffset, 1);
	}
	UT_ASSERT(pNewRun); // TODO check for outofmem
	if(getPrev()!= NULL && getPrev()->getLastContainer()==NULL)
	{
		UT_DEBUGMSG(("In fl_BlockLayout::_doInsertForcedPageBreakRun  no LastLine \n"));
		UT_DEBUGMSG(("getPrev = %p this = %p \n",getPrev(),this));
		//UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	bool bResult = _doInsertRun(pNewRun);
	//
	// only do this if this run is the last run in the block. Otherwise we terrible UI 
	// whre the first line of th enext page cannot have it's own style!
	//
	if (bResult && !isLastRunInBlock(pNewRun))
		_breakLineAfterRun(pNewRun);

	return bResult;
}

bool	fl_BlockLayout::_doInsertForcedColumnBreakRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = NULL;
	if(isContainedByTOC())
	{
		pNewRun = new fp_DummyRun(this,blockOffset);
	}
	else
	{
		pNewRun = new fp_ForcedColumnBreakRun(this,blockOffset, 1);
	}
	UT_ASSERT(pNewRun); // TODO check for outofmem

	bool bResult = _doInsertRun(pNewRun);
	if (bResult && !isLastRunInBlock(pNewRun) )
		_breakLineAfterRun(pNewRun);

	return bResult;
}

bool	fl_BlockLayout::_doInsertTabRun(PT_BlockOffset blockOffset)
{
	fp_Run * pNewRun = NULL;
	if(!isContainedByTOC() || !m_bPrevListLabel)
	{
		pNewRun = new fp_TabRun(this,blockOffset, 1);
	}
	else
	{
		xxx_UT_DEBUGMSG(("Insert dummy in place of TAB at %d \n",blockOffset));
		pNewRun = new fp_DummyRun(this,blockOffset);
	}
	UT_ASSERT(pNewRun); // TODO check for outofmem

	return _doInsertRun(pNewRun);
}

UT_sint32	fl_BlockLayout::getTextIndent(void) const
{
	fl_ContainerLayout * pCL = myContainingLayout();
	if(pCL && (pCL->getContainerType() == FL_CONTAINER_ANNOTATION) && ((pCL->getFirstLayout() == NULL) || (pCL->getFirstLayout() == this)))
	{
			fl_AnnotationLayout * pAL = static_cast<fl_AnnotationLayout *>(pCL);
			fp_AnnotationRun * pAR = pAL->getAnnotationRun();
			if(pAR)
			{
				    if(pAR->getRealWidth() == 0)
						pAR->recalcValue();
					return m_iTextIndent+pAR->getRealWidth();
			}
	}
	return m_iTextIndent;
}

bool	fl_BlockLayout::_doInsertMathRun(PT_BlockOffset blockOffset,PT_AttrPropIndex indexAP, pf_Frag_Object* oh)
{
	if(isContainedByTOC())
	{
		fp_Run * pDumRun = new fp_DummyRun(this,blockOffset);
		xxx_UT_DEBUGMSG(("Inserting a dummy run instead of mathrun at %d \n",blockOffset));
		return _doInsertRun(pDumRun);
	}

	fp_Run * pNewRun = NULL;
	pNewRun = new fp_MathRun(this,blockOffset,indexAP,oh);
	UT_ASSERT(pNewRun); // TODO check for outofmem

	return _doInsertRun(pNewRun);
}


bool	fl_BlockLayout::_doInsertEmbedRun(PT_BlockOffset blockOffset,PT_AttrPropIndex indexAP, pf_Frag_Object* oh)
{
	if(isContainedByTOC())
	{
		fp_Run * pDumRun = new fp_DummyRun(this,blockOffset);
		xxx_UT_DEBUGMSG(("Inserting a dummy run instead of embedrun at %d \n",blockOffset));
		return _doInsertRun(pDumRun);
	}

	fp_Run * pNewRun = NULL;
	pNewRun = new fp_EmbedRun(this,blockOffset,indexAP,oh);
	UT_ASSERT(pNewRun); // TODO check for outofmem

	return _doInsertRun(pNewRun);
}

bool	fl_BlockLayout::_doInsertTOCTabRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_TabRun(this,blockOffset, 1);
	UT_ASSERT(pNewRun); // TODO check for outofmem
	static_cast<fp_TabRun *>(pNewRun)->setTOCTab();
	return _doInsertRun(pNewRun);
}

/*!
 * Special TAB that follows a TOCListLabel. It has zero length since it's
 * not in the document.
 */
bool	fl_BlockLayout::_doInsertTOCListTabRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_TabRun(this,blockOffset, 0);
	UT_ASSERT(pNewRun); // TODO check for outofmem
	static_cast<fp_TabRun *>(pNewRun)->setTOCTabListLabel();
	fp_Run * pRun = m_pFirstRun;
	pRun->insertIntoRunListBeforeThis(*pNewRun);
	m_pFirstRun = pNewRun;
	pNewRun->markWidthDirty();
	if(pRun->getLine())
	{
		pRun->getLine()->insertRunBefore(pNewRun, pRun);
	}
	return true;
}

bool	fl_BlockLayout::_doInsertImageRun(PT_BlockOffset blockOffset, FG_Graphic* pFG, pf_Frag_Object* oh)
{
	if(isContainedByTOC())
	{
		fp_Run * pDumRun = new fp_DummyRun(this,blockOffset);
		xxx_UT_DEBUGMSG(("Inserting a dummy run instead of image at %d \n",blockOffset));
		return _doInsertRun(pDumRun);
	}

	fp_ImageRun* pNewRun = new fp_ImageRun(this, blockOffset, 1, pFG,oh);
	UT_ASSERT(pNewRun); // TODO check for outofmem

	return _doInsertRun(pNewRun);
}

bool	fl_BlockLayout::_doInsertFieldRun(PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro)
{
	// Get the field type.
	const PP_AttrProp * pSpanAP = NULL;

#if 0
	// this is unnecessarily involved, just use the index from the pcro
	getSpanAttrProp(blockOffset, false, &pSpanAP);
	UT_ASSERT(pSpanAP);
#else
	UT_return_val_if_fail(pcro, false);
	PT_AttrPropIndex iAP = pcro->getIndexAP();
	m_pLayout->getDocument()->getAttrProp(iAP, &pSpanAP);
#endif
	
	const gchar* pszType = NULL;
	pSpanAP->getAttribute("type", pszType);

	// Create the field run.

	fp_FieldRun* pNewRun;

	if (!pszType) 
		{
			UT_ASSERT (pszType); 	
			pNewRun = new fp_FieldRun(this, blockOffset,1);
		}
	else if(strcmp(pszType, "list_label") == 0)
	{
		if(!isContainedByTOC())
		{
			pNewRun = new fp_FieldListLabelRun(this,   blockOffset, 1);
		}
		else
		{
			fp_Run * pDumRun = new fp_DummyRun(this,blockOffset);
			xxx_UT_DEBUGMSG(("Inserting a dummy run instead of listlabel at %d \n",blockOffset));
			_doInsertRun(pDumRun);
			recalculateFields(0);
			m_bPrevListLabel = true;
			//
			// Might have to put in code here to detect if there is already
			// a tab run ahead of the list label. If so we replace it
			// with a dummyrun
			// fp_Run * pNextRun = pDumRun->getNextRun();
			return true;
		}
	}
	else if(strcmp(pszType, "footnote_ref") == 0)
	{
		if(isContainedByTOC())
		{
			fp_Run * pDumRun = new fp_DummyRun(this,blockOffset);
			xxx_UT_DEBUGMSG(("Inserting a dummy run instead of footnote_ref at %d \n",blockOffset));
			return _doInsertRun(pDumRun);
		}

		xxx_UT_DEBUGMSG(("Footnoet ref run created at %d \n",blockOffset));
		pNewRun = new fp_FieldFootnoteRefRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "footnote_anchor") == 0)
	{
		if(isContainedByTOC())
		{
			fp_Run * pDumRun = new fp_DummyRun(this,blockOffset);
			xxx_UT_DEBUGMSG(("Inserting a dummy run instead of footnote_anchor at %d \n",blockOffset));
			return _doInsertRun(pDumRun);
		}
		pNewRun = new fp_FieldFootnoteAnchorRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "endnote_ref") == 0)
	{
		if(isContainedByTOC())
		{
			fp_Run * pDumRun = new fp_DummyRun(this,blockOffset);
			xxx_UT_DEBUGMSG(("Inserting a dummy run instead of endnote_ref at %d \n",blockOffset));
			return _doInsertRun(pDumRun);
		}

		xxx_UT_DEBUGMSG(("Endnote ref run created at %d \n",blockOffset));
		pNewRun = new fp_FieldEndnoteRefRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "endnote_anchor") == 0)
	{
		if(isContainedByTOC())
		{
			fp_Run * pDumRun = new fp_DummyRun(this,blockOffset);
			xxx_UT_DEBUGMSG(("Inserting a dummy run instead of endnote_anchor at %d \n",blockOffset));
			return _doInsertRun(pDumRun);
		}
		pNewRun = new fp_FieldEndnoteAnchorRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "time") == 0)
	{
		pNewRun = new fp_FieldTimeRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "page_number") == 0)
	{
		pNewRun = new fp_FieldPageNumberRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "page_ref") == 0)
	{
		pNewRun = new fp_FieldPageReferenceRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "page_count") == 0)
	{
		pNewRun = new fp_FieldPageCountRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "date") == 0)
	{
		pNewRun = new fp_FieldDateRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "date_mmddyy") == 0)
	{
		pNewRun = new fp_FieldMMDDYYRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "date_ddmmyy") == 0)
	{
		pNewRun = new fp_FieldDDMMYYRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "date_mdy") == 0)
	{
		pNewRun = new fp_FieldMonthDayYearRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "date_mthdy") == 0)
	{
		pNewRun = new fp_FieldMthDayYearRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "date_dfl") == 0)
	{
		pNewRun = new fp_FieldDefaultDateRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "date_ntdfl") == 0)
	{
		pNewRun = new fp_FieldDefaultDateNoTimeRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "date_wkday") == 0)
	{
		pNewRun = new fp_FieldWkdayRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "date_doy") == 0)
	{
		pNewRun = new fp_FieldDOYRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "time_miltime") == 0)
	{
		pNewRun = new fp_FieldMilTimeRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "time_ampm") == 0)
	{
		pNewRun = new fp_FieldAMPMRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "time_zone") == 0)
	{
		pNewRun = new fp_FieldTimeZoneRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "time_epoch") == 0)
	{
		pNewRun = new fp_FieldTimeEpochRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "datetime_custom") == 0)
	{
		pNewRun = new fp_FieldDateTimeCustomRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "word_count") == 0)
	{
		pNewRun = new fp_FieldWordCountRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "char_count") == 0)
	{
		pNewRun = new fp_FieldCharCountRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "line_count") == 0)
	{
		pNewRun = new fp_FieldLineCountRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "para_count") == 0)
	{
		pNewRun = new fp_FieldParaCountRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "nbsp_count") == 0)
	{
		pNewRun = new fp_FieldNonBlankCharCountRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "file_name") == 0)
	{
		pNewRun = new fp_FieldFileNameRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "short_file_name") == 0)
	{
		pNewRun = new fp_FieldShortFileNameRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "app_ver") == 0)
	{
		pNewRun = new fp_FieldBuildVersionRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "app_id") == 0)
	{
		pNewRun = new fp_FieldBuildIdRun(this,   blockOffset, 1);
	}
	else if(strcmp(pszType, "app_options") == 0)
	  {
		pNewRun = new fp_FieldBuildOptionsRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "app_target") == 0)
	  {
		pNewRun = new fp_FieldBuildTargetRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "app_compiledate") == 0)
	  {
		pNewRun = new fp_FieldBuildCompileDateRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "app_compiletime") == 0)
	  {
		pNewRun = new fp_FieldBuildCompileTimeRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "mail_merge") == 0)
	  {
	    pNewRun = new fp_FieldMailMergeRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "meta_title") == 0)
	  {
	    pNewRun = new fp_FieldMetaTitleRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "meta_creator") == 0)
	  {
	    pNewRun = new fp_FieldMetaCreatorRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "meta_subject") == 0)
	  {
	    pNewRun = new fp_FieldMetaSubjectRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "meta_publisher") == 0)
	  {
	    pNewRun = new fp_FieldMetaPublisherRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "meta_contributor") == 0)
	  {
	    pNewRun = new fp_FieldMetaContributorRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "meta_date") == 0)
	  {
	    pNewRun = new fp_FieldMetaDateRun(this,   blockOffset, 1);
	  }
        else if(strcmp(pszType, "meta_date_last_changed") == 0)
	  {
	    pNewRun = new fp_FieldMetaDateLastChangedRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "meta_type") == 0)
	  {
	    pNewRun = new fp_FieldMetaTypeRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "meta_language") == 0)
	  {
	    pNewRun = new fp_FieldMetaLanguageRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "meta_coverage") == 0)
	  {
	    pNewRun = new fp_FieldMetaCoverageRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "meta_rights") == 0)
	  {
	    pNewRun = new fp_FieldMetaRightsRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "meta_keywords") == 0)
	  {
	    pNewRun = new fp_FieldMetaKeywordsRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "meta_description") == 0)
	  {
	    pNewRun = new fp_FieldMetaDescriptionRun(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "sum_rows") == 0)
	  {
	    pNewRun = new fp_FieldTableSumRows(this,   blockOffset, 1);
	  }
	else if(strcmp(pszType, "sum_cols") == 0)
	  {
	    pNewRun = new fp_FieldTableSumCols(this,   blockOffset, 1);
	  }
	else
	{
		UT_ASSERT_NOT_REACHED ();
		//
		// New Piece Table Field Leave it for that code..
		//
		pNewRun = new fp_FieldRun(this,   blockOffset, 1);
	}

	UT_ASSERT(pNewRun); // TODO check for outofmem

	// TODO -- is this really needed ???
	// should not be, since we called lookupProperties in the
	// constructor - Tomas
	// pNewRun->lookupProperties();
	pNewRun->calculateValue();

	_doInsertRun(pNewRun);
	//	recalculateFields(0); MES Do this in the format following
	return true;
}


bool	fl_BlockLayout::_doInsertFieldTOCRun(PT_BlockOffset blockOffset)
{
	fp_FieldRun* pNewRun;
	pNewRun = new fp_FieldTOCNumRun(this,   blockOffset, 1);
	_doInsertRun(pNewRun);
	return true;
}

/*!
 * TOC List label run. It has zero length since it's not in the document.
 */
bool	fl_BlockLayout::_doInsertTOCListLabelRun(PT_BlockOffset blockOffset)
{
	fp_FieldRun* pNewRun;
	pNewRun = new fp_FieldTOCListLabelRun(this,   blockOffset, 0);
	fp_Run * pRun = m_pFirstRun;
	pRun->insertIntoRunListBeforeThis(*pNewRun);
	m_pFirstRun = pNewRun;
	pNewRun->markWidthDirty();
	if(pRun->getLine())
	{
		pRun->getLine()->insertRunBefore(pNewRun, pRun);
	}
	return true;
}


bool	fl_BlockLayout::_doInsertTOCHeadingRun(PT_BlockOffset blockOffset)
{
	fp_FieldRun* pNewRun;
	pNewRun = new fp_FieldTOCHeadingRun(this,   blockOffset, 1);
	fp_Run * pRun = m_pFirstRun;
	pRun->insertIntoRunListBeforeThis(*pNewRun);
	m_pFirstRun = pNewRun;
	pNewRun->markWidthDirty();
	if(pRun->getLine())
	{
		pRun->getLine()->insertRunBefore(pNewRun, pRun);
	}
	return true;
}

bool	fl_BlockLayout::_doInsertRun(fp_Run* pNewRun)
{
	PT_BlockOffset blockOffset = pNewRun->getBlockOffset();
	UT_uint32 len = pNewRun->getLength();
	xxx_UT_DEBUGMSG(("_doInsertRun: New run has offset %d Length %d \n",blockOffset,len));
	_assertRunListIntegrity();

	bool bInserted = false;
	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		UT_uint32 iRunBlockOffset = pRun->getBlockOffset();
		UT_uint32 iRunLength = pRun->getLength();
		xxx_UT_DEBUGMSG(("_doInsertRun: Target offset %d CurRun Offset %d Length %d  Type %d \n",blockOffset,iRunBlockOffset,iRunLength,pRun->getType()));
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
			
			if(m_pFirstRun == pRun)
			{
				m_pFirstRun = pNewRun;
			}
			bInserted = true;
			if(pRun->getLine())
			{
				pRun->getLine()->insertRunBefore(pNewRun, pRun);
#if DEBUG
				pRun->getLine()->assertLineListIntegrity();
#endif
			}
		}
		else if (iRunBlockOffset == blockOffset && !bInserted)
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
		else if(!bInserted)
		{
			UT_ASSERT(!bInserted);

			UT_ASSERT((blockOffset >= pRun->getBlockOffset()) &&
					  (blockOffset <
					   (pRun->getBlockOffset() + pRun->getLength())));
			UT_ASSERT(pRun->getType() == FPRUN_TEXT);	// only textual runs can be split anyway

			fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pRun);
			//
			// Have to take account of the length of the new run in the
			// block.
			//
			pTextRun->split(blockOffset,pNewRun->getLength());

			UT_ASSERT(pRun->getNextRun());
			UT_ASSERT(pRun->getNextRun()->getBlockOffset() == (blockOffset+pNewRun->getLength()));

			UT_ASSERT(pTextRun->getNextRun());
			UT_ASSERT(pTextRun->getNextRun()->getType() == FPRUN_TEXT);

			bInserted = true;

			pRun = pRun->getNextRun();

			iRunBlockOffset = pRun->getBlockOffset();
			iRunLength = pRun->getLength();

			UT_ASSERT(iRunBlockOffset == (blockOffset+pNewRun->getLength()));

			// the insert is right before this run.
	
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
		pRun = pRun->getNextRun();
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
			pRun = pRun->getNextRun();
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
	UT_BidiCharType iDirection = pNewRun->getDirection();
	if(FRIBIDI_IS_STRONG(iDirection) && pNewRun->getType() == FPRUN_TEXT)
	{
		static_cast<fp_TextRun*>(pNewRun)->breakNeighborsAtDirBoundaries();
	}
	
	pNewRun->markWidthDirty();
	_assertRunListIntegrity();

#if 0
	// now that the run is in place and the context has been set, we
	// calculate character widths

	// actually, we are not in position to calculate widths at this
	// point, because the insertion of this run invalidated the draw
	// buffers of un unspecified number of runs on either side, and in
	// order for the width calculation to be correct, the widths of
	// the runs that precede it would need to be recalculated first,
	// otherwise we get wrong results with ligatures (when our run
	// starts with a ligature placeholder, its with gets set to 1/2 of
	// the width of the previous glyph; this assumes that the previous
	// glyph is already the ligature glyph which it is not)
	// There seems to be no reason why we would need to calculate the widths
	// here, so we will leave it for now, and when the widths are needed,
	// i.e., when we attempt to draw, we will have all the right
	// values in place
	if (pNewRun->getType() == FPRUN_TEXT)
	{
		fp_TextRun* pNewTextRun = static_cast<fp_TextRun*>(pNewRun);
		pNewTextRun->recalcWidth();
	}
#endif
	return true;
}

/*!
 * This method will append the text in the block to the UTF8 string supplied
 */
void fl_BlockLayout::appendUTF8String(UT_UTF8String & sText) const
{
	UT_GrowBuf buf;
	appendTextToBuf(buf);
	const UT_UCS4Char * pBuff = reinterpret_cast<const UT_UCS4Char *>(buf.getPointer(0));
	if((buf.getLength() > 0) && (pBuff != NULL))
	{
		sText.appendUCS4(pBuff,buf.getLength());
	}
}

/*!
 * This method extracts all the text from the current block and appends it
 * to the supplied growbuf.
 */
void fl_BlockLayout::appendTextToBuf(UT_GrowBuf & buf) const
{
	fp_Run * pRun = m_pFirstRun;
	while(pRun)
    {
		if(pRun->getType() == 	FPRUN_TEXT)
		{
			fp_TextRun * pTRun = static_cast<fp_TextRun *>(pRun);
			pTRun->appendTextToBuf(buf);
		}
		pRun = pRun->getNextRun();
	}
}
bool fl_BlockLayout::doclistener_insertSpan(const PX_ChangeRecord_Span * pcrs)
{
	UT_return_val_if_fail( m_pLayout, false );
	
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
	xxx_UT_DEBUGMSG(("fl_BlockLayout::doclistener_insertSpan(), len=%d, pos %d \n", len, getPosition()+blockOffset));
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
	m_iNeedsReformat = blockOffset;
	format();
	updateEnclosingBlockIfNeeded();

#ifdef ENABLE_SPELL
	m_pSpellSquiggles->textInserted(blockOffset, len);
	m_pGrammarSquiggles->textInserted(blockOffset, len);
	xxx_UT_DEBUGMSG(("Set pending block for grammar - insertSpan \n"));
	m_pLayout->setPendingBlockForGrammar(this);
#endif
	FV_View* pView = getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->_setPoint(pcrs->getPosition() + len);
//		if(!isHdrFtr())
//			pView->notifyListeners(AV_CHG_FMTCHAR); // TODO verify that this is necessary.
	}
	else if(pView && pView->getPoint() > pcrs->getPosition())
		pView->_setPoint(pView->getPoint() + len);

	if(pView)
		pView->updateCarets(pcrs->getPosition(),len);

	if (m_pLayout->hasBackgroundCheckReason(FL_DocLayout::bgcrSmartQuotes))
	{
		fl_BlockLayout *sq_bl = m_pLayout->getPendingBlockForSmartQuote();
		UT_uint32 sq_of = m_pLayout->getOffsetForSmartQuote();
		m_pLayout->setPendingSmartQuote(NULL, 0);
		//
		// Don't do Smart quotes during an undo or during a paste
		//
		if(!m_pDoc->isDoingTheDo() && !m_pDoc->isDoingPaste())
		{
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
		ppRun = ppRun->getNextRun();
	}
#endif
#endif
	//
	// OK Now do the insertSpan for any TOC's that shadow this block.
	//
	if(!isNotTOCable() && !m_bIsTOC && m_bStyleInTOC)
	{
		UT_GenericVector<fl_BlockLayout *> vecBlocksInTOCs;
		if(m_pLayout->getMatchingBlocksFromTOCs(this, &vecBlocksInTOCs))
		{
			for(UT_sint32 j=0; j<vecBlocksInTOCs.getItemCount();j++)
			{
				fl_BlockLayout * pBL = vecBlocksInTOCs.getNthItem(j);
				pBL->doclistener_insertSpan(pcrs);
			}
		}
		else
		{
			m_bStyleInTOC = false;
		}
	}
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
fl_BlockLayout::_assertRunListIntegrityImpl(void) const
{
	UT_return_if_fail( m_pLayout );
	
	fp_Run* pRun = m_pFirstRun;
	UT_uint32 iOffset = 0;
	if(m_pFirstRun)
	{
		//
		// Dummy Runs are allowed at the first positions 
		// of a TOC
		//
		if(m_pFirstRun->getPrevRun())
		{
			UT_ASSERT(m_pFirstRun->getPrevRun()->getType() == FPRUN_DUMMY);
		}
	}
#if 0
	//
	// This can legitmately be non zero while deleting a block with an
	// embedded footnote
	//
	UT_ASSERT(m_pFirstRun->getBlockOffset() == 0);
	// Verify that offset of this block is correct.
#endif
	UT_sint32 icnt = -1;
	//	PT_DocPosition posAtStartOfBlock = getPosition();
	while (pRun)
	{
		icnt++;
		xxx_UT_DEBUGMSG(("!!Assert run %d runType %d posindoc %d end run %d \n",icnt,pRun->getType(),posAtStartOfBlock+pRun->getBlockOffset(),posAtStartOfBlock+pRun->getBlockOffset()+pRun->getLength()));
		xxx_UT_DEBUGMSG(("run %d %p Type %d offset %d length %d \n",icnt,pRun,pRun->getType(),pRun->getBlockOffset(), pRun->getLength()));
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
		//		UT_ASSERT( ((pRun->getType() != FPRUN_FMTMARK)
		//			|| !pRun->getNextRun()
		//			|| (pRun->getNextRun()->getType() != FPRUN_FMTMARK)) );

		// Verify that the Run has a non-zero length (or is a FmtMark)
		UT_ASSERT( (FPRUN_FMTMARK == pRun->getType()) || 
				   (((FPRUN_TAB == pRun->getType()) 
					 || (FPRUN_FIELD == pRun->getType()))
					  && isContainedByTOC())
				   || (pRun->getLength() > 0) );

		// Verify that if there is no next Run, this Run is the EOP Run.
		// Or we're in the middle of loading a document.
//
// FIXME; Take this code out when things work.
//
		if(pRun->getNextRun() || (FPRUN_ENDOFPARAGRAPH == pRun->getType()) )
		{
		}
		else
		{
			m_pLayout->getDocument()->miniDump(getStruxDocHandle(),8);
		}
		UT_ASSERT( pRun->getNextRun()
				   || (FPRUN_ENDOFPARAGRAPH == pRun->getType()) );
		pRun = pRun->getNextRun();
	}
}
#endif /* !NDEBUG */

inline void
fl_BlockLayout::_assertRunListIntegrity(void) const
{
#ifndef NDEBUG
	_assertRunListIntegrityImpl();
#endif
}


bool fl_BlockLayout::_delete(PT_BlockOffset blockOffset, UT_uint32 len)
{
	_assertRunListIntegrity();
	xxx_UT_DEBUGMSG(("_delete fl_BlockLayout offset %d len %d \n",blockOffset,len));
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
		xxx_UT_DEBUGMSG(("_delete run %x type %d offset %d len %d \n",pRun,pRun->getType(),iRunBlockOffset,iRunLength));
		fp_Run* pNextRun = pRun->getNextRun(); // remember where we're going, since this run may get axed

		if ( (iRunBlockOffset + iRunLength) <= blockOffset )
		{
			// nothing to do.  the delete occurred AFTER this run
		}
		else if (iRunBlockOffset >= (blockOffset + len))
		{
			// the delete occurred entirely before this run.
			xxx_UT_DEBUGMSG(("_delete Run %x New Offset offset %d len %d \n",pRun,iRunBlockOffset - len,iRunLength));

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
						if(pRun->getNextRun() && pRun->getNextRun()->getType()== FPRUN_TEXT)
						{
							pTR_next = static_cast<fp_TextRun*>(pRun->getNextRun());
						}
						
						if(pRun->getPrevRun() && pRun->getPrevRun()->getType()== FPRUN_TEXT)
						{
							pTR_prev = static_cast<fp_TextRun*>(pRun->getPrevRun());
						}
					}
					else if(pRun->getType()== FPRUN_TEXT)
					{
						// there should always be something left of
						// this run
						pTR_del1 = static_cast<fp_TextRun*>(pRun);

						if(pRun->getNextRun() && pRun->getNextRun()->getType()== FPRUN_TEXT)
						{
							pTR_next = static_cast<fp_TextRun*>(pRun->getNextRun());
						}
						
						if(pRun->getPrevRun() && pRun->getPrevRun()->getType()== FPRUN_TEXT)
						{
							pTR_prev = static_cast<fp_TextRun*>(pRun->getPrevRun());
						}
					}
					
					//pRun->setLength(iRunLength - len);
					pRun->updateOnDelete(blockOffset - iRunBlockOffset, len);
					UT_ASSERT((pRun->getLength() == 0) || (pRun->getType() == FPRUN_TEXT)); // only textual runs could have a partial deletion
				}
				else
				{
					// the deleted section crosses over the end of the
					// run, but not the start; it can however, lead to
					// deletion of an entire run
					UT_ASSERT(iRunBlockOffset + iRunLength - blockOffset > 0);

					if(pRun->getType()== FPRUN_DIRECTIONMARKER)
					{
						if(pRun->getNextRun() && pRun->getNextRun()->getType()== FPRUN_TEXT)
						{
							pTR_next = static_cast<fp_TextRun*>(pRun->getNextRun());
						}
						
						if(pRun->getPrevRun() && pRun->getPrevRun()->getType()== FPRUN_TEXT)
						{
							pTR_prev = static_cast<fp_TextRun*>(pRun->getPrevRun());
						}
					}
					else if(pRun->getType()== FPRUN_TEXT)
					{
						// if the block offset is same as the run
						// offset and deleted length is greater or
						// equal to the run length, this whole run is
						// going and we must do no further processing
						// on it ...
						if(!((iRunBlockOffset == blockOffset) && (iRunLength <= len)))
							pTR_del1 = static_cast<fp_TextRun*>(pRun);

						if(pRun->getNextRun() && pRun->getNextRun()->getType()== FPRUN_TEXT)
						{
							pTR_next = static_cast<fp_TextRun*>(pRun->getNextRun());
						}
						
						if(pRun->getPrevRun() && pRun->getPrevRun()->getType()== FPRUN_TEXT)
						{
							pTR_prev = static_cast<fp_TextRun*>(pRun->getPrevRun());
						}
					}
					
					//pRun->setLength(iRunLength - iDeleted);
					pRun->updateOnDelete(blockOffset - iRunBlockOffset, len);
					UT_ASSERT((pRun->getLength() == 0) || (pRun->getType() == FPRUN_TEXT)); // only textual runs could have a partial deletion
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
					if(pRun->getNextRun() && pRun->getNextRun()->getType()== FPRUN_TEXT)
					{
						pTR_next = static_cast<fp_TextRun*>(pRun->getNextRun());
					}
						
					if(pRun->getPrevRun() && pRun->getPrevRun()->getType()== FPRUN_TEXT)
					{
						pTR_prev = static_cast<fp_TextRun*>(pRun->getPrevRun());
					}
				}
				else if(pRun->getType()== FPRUN_TEXT)
				{
					if(!pTR_del1 && pRun->getPrevRun() && pRun->getPrevRun()->getType()== FPRUN_TEXT)
					{
						pTR_prev = static_cast<fp_TextRun*>(pRun->getPrevRun());
					}

					if(pRun->getNextRun() && pRun->getNextRun()->getType()== FPRUN_TEXT)
					{
						pTR_next = static_cast<fp_TextRun*>(pRun->getNextRun());
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
					//pRun->setLength(iRunLength - iDeleted);
					pRun->updateOnDelete(0, iDeleted);
					UT_ASSERT((pRun->getLength() == 0) || (pRun->getType() == FPRUN_TEXT)); // only textual runs could have a partial deletion
				}
				else
				{
					/* the deletion spans the entire run. time to delete it */
					//pRun->setLength(0);
					pRun->updateOnDelete(0, iRunLength);
				}
			}

			if ((pRun->getLength() == 0) && (pRun->getType() != FPRUN_FMTMARK))
			{
				// Remove Run from line
				// first, however, make sure that if this is our
				// pTR_next run, we get the run after it in its place
				if(pTR_next == pRun)
				{
					if(pRun->getNextRun() && pRun->getNextRun()->getType() == FPRUN_TEXT)
					{
						pTR_next =  static_cast<fp_TextRun*>(pRun->getNextRun());
					}
					else
					{
						pTR_next = NULL;
					}
				}
				
				fp_Line* pLine = pRun->getLine();
				if(pLine)
				{
					pLine->removeRun(pRun, true);
				}
				// Unlink Run and delete it
				if (m_pFirstRun == pRun)
				{
					m_pFirstRun = pRun->getNextRun();
				}
				pRun->unlinkFromRunList();

				// make sure that we do not do any bidi
				// post-processing on the delete run ...
				if(pTR_del1 == pRun)
					pTR_del1 = NULL;
				
				if(pTR_del2 == pRun)
					pTR_del2 = NULL;

				if(pTR_prev == pRun)
					pTR_prev = NULL;
				
				DELETEP(pRun);
				
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
		pTR_del1->breakMeAtDirBoundaries(UT_BIDI_IGNORE);
	
	if(pTR_del2)
		pTR_del2->breakMeAtDirBoundaries(UT_BIDI_IGNORE);

	if(pTR_prev)
		pTR_prev->breakMeAtDirBoundaries(UT_BIDI_IGNORE);

	if(pTR_next)
		pTR_next->breakMeAtDirBoundaries(UT_BIDI_IGNORE);

	_assertRunListIntegrity();

	return true;
}

bool fl_BlockLayout::doclistener_deleteSpan(const PX_ChangeRecord_Span * pcrs)
{
	UT_return_val_if_fail( m_pLayout, false );
	_assertRunListIntegrity();

	UT_ASSERT(pcrs->getType()==PX_ChangeRecord::PXT_DeleteSpan);

	PT_BlockOffset blockOffset = pcrs->getBlockOffset();
	UT_uint32 len = pcrs->getLength();
	UT_ASSERT(len>0);
	xxx_UT_DEBUGMSG(("fl_BlockLayout:: deleteSpan offset %d len %d \n",blockOffset,len));
	_delete(blockOffset, len);

#ifdef ENABLE_SPELL
	m_pSpellSquiggles->textDeleted(blockOffset, len);
	m_pGrammarSquiggles->textDeleted(blockOffset, len);
	xxx_UT_DEBUGMSG(("Set pending block for grammar - deleteSpan \n"));
	m_pLayout->setPendingBlockForGrammar(this);
#endif
	
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
	if(pView)
		pView->updateCarets(pcrs->getPosition(),-len);

	_assertRunListIntegrity();
	m_iNeedsReformat = blockOffset;
	format();
	updateEnclosingBlockIfNeeded();
	//
	// OK Now do the deleteSpan for any TOC's that shadow this block.
	//
	if(!isNotTOCable() && !m_bIsTOC && m_bStyleInTOC)
	{
		UT_GenericVector<fl_BlockLayout *> vecBlocksInTOCs;
		if( m_pLayout->getMatchingBlocksFromTOCs(this, &vecBlocksInTOCs))
		{
			UT_sint32 i = 0;
			for(i=0; i<vecBlocksInTOCs.getItemCount();i++)
			{
				fl_BlockLayout * pBL = vecBlocksInTOCs.getNthItem(i);
				pBL->doclistener_deleteSpan(pcrs);
			}
		}
		else
		{
			m_bStyleInTOC = false;
		}
	}

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
	UT_GenericVector<fp_Line *> vecLines;
	vecLines.clear();
	// First look for the first run inside the span
	fp_Run* pRun = m_pFirstRun;
	fp_Run* pPrevRun = NULL;
	while (pRun && pRun->getBlockOffset() < blockOffset)
	{
		pPrevRun = pRun;
		pRun = pRun->getNextRun();
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
		UT_return_val_if_fail(pPrevRun,false);
		UT_ASSERT(FPRUN_TEXT == pPrevRun->getType());
		if (FPRUN_TEXT == pPrevRun->getType())
		{
			fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pPrevRun);
			pTextRun->split(blockOffset,0);
		}
		pRun = pPrevRun->getNextRun();
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
				pTextRun->split(blockOffset+len,0);
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
			// I moved markWidthDirty() inside fp_TextRun::_lookupProperties(),
			// since there we are in proper position to determine if the
			// width needs redoing. Tomas, Nov 28, 2003
			// pTextRun->markWidthDirty();
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
		fp_Line * pLine = pRun->getLine();
		if((pLine!= NULL) && (vecLines.findItem(pLine) < 0))
		{
			vecLines.addItem(pLine);
		}
		pRun = pRun->getNextRun();
	}
	//
	// maybe able to remove this once the rest of bug 5240 is fixed.
	//
	UT_sint32 i =0;
   	for(i=0; i< vecLines.getItemCount(); i++)
	{
		fp_Line * pLine = vecLines.getNthItem(i);
		pLine->clearScreen();
	}
	m_iNeedsReformat = blockOffset;
    format();
	updateEnclosingBlockIfNeeded();
	_assertRunListIntegrity();

#ifdef ENABLE_SPELL
	// need to handle the case where we have a revisions based delete
	if(pcrsc->isRevisionDelete())
	{
		m_pSpellSquiggles->textRevised(blockOffset, 0);
		m_pGrammarSquiggles->textRevised(blockOffset, 0);
	}
#endif
	
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
	bool isInFrame = (getSectionLayout()->getContainerType() == FL_CONTAINER_FRAME);
	fp_Container * pCon = getFirstContainer();
	if(!isInFrame)
	{
		if(pCon)
		{
			fp_Page * pPage = pCon->getPage();
			getDocSectionLayout()->setNeedsSectionBreak(true,pPage);
		}
		else
		{
			getDocSectionLayout()->setNeedsSectionBreak(true,NULL);
		}
	}
	if(getPrev())
	{
		getPrev()->setNeedsReformat(this);
		getPrev()->setNeedsRedraw();
	}
	setNeedsReformat(this);
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
	//
	// This is not exactly the case; for example the first block in the footnote section
	// has not previous, yet it is not empty -- it contains at least the footnote reference.

	fp_Line* pLastLine = NULL;
	fl_BlockLayout * pPrevBL = NULL;
	
	fl_ContainerLayout *pCL = getPrev();
	while(pCL && pCL->getContainerType() != FL_CONTAINER_BLOCK)
	{
//
// Attach to the block before the other container type , 
// because this block has to get merged with it
//
		pCL = pCL->getPrev();
	}

	// this is safe cast because we either have block or NULL
	pPrevBL = static_cast<fl_BlockLayout*>(pCL);
	
	//
	// Deal with embedded containers if any in this block.
	//
	shuffleEmbeddedIfNeeded(pPrevBL, 0);
	//
	// The idea here is to append the runs of the deleted block, if
	// any, at the end of the previous block. We must make sure to take
	// of embedded footnotes/endnotes. We need to calculate the offset
	// before we deletes the EOP run. The offset may not be contiguous
	// because of embedded footnotes/endnotes
	//
	UT_uint32 offset = 0;
	if (pPrevBL)
	{
		// Find the EOP Run.
		pLastLine = static_cast<fp_Line *>(pPrevBL->getLastContainer());
		fp_Run* pNukeRun = pPrevBL->m_pFirstRun;
		fp_Run * pPrevRun = pPrevBL->m_pFirstRun;
		while(pNukeRun->getNextRun() != NULL)
		{
			pPrevRun = pNukeRun;
			UT_ASSERT(FPRUN_ENDOFPARAGRAPH != pPrevRun->getType());
			pNukeRun  = pPrevRun->getNextRun();
		}
		UT_ASSERT(FPRUN_ENDOFPARAGRAPH == pNukeRun->getType());
		//
		// The idea here is to append the runs of the deleted block, if
		// any, at the end of the previous block. We must make sure to take
		// account of embedded footnotes/endnotes. 
		// We need to calculate the offset
		// before we delete the EOP run.
		//
		if(FPRUN_ENDOFPARAGRAPH == pNukeRun->getType())
		{
			offset = pNukeRun->getBlockOffset();
		}
		else
		{
			offset =  pNukeRun->getBlockOffset() + pNukeRun->getLength();
		}

		// Detach from the line
		fp_Line* pLine = pNukeRun->getLine();
		UT_ASSERT(pLine && pLine == pLastLine);
		if(pLine)
		{
			pLine->removeRun(pNukeRun);
		}
		// Unlink and delete it
		if (pPrevRun && (pPrevRun != pNukeRun))
		{
			pPrevRun->setNextRun(NULL);
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
		if(pLine)
		{
			pLine->removeRun(pNukeRun);
		}

		// Unlink and delete it
		m_pFirstRun = NULL;
		delete pNukeRun;

	}

	// We use the offset we calculated earlier.
 
	if (m_pFirstRun)
	{
		// Figure out where the merge point is
		fp_Run * pRun = pPrevBL->m_pFirstRun;
		fp_Run * pLastRun = NULL;
		while (pRun)
		{
			pLastRun = pRun;
			pRun = pRun->getNextRun();
		}
		// Link them together
		if (pLastRun)
		{
			pLastRun->setNextRun(m_pFirstRun);
			if(m_pFirstRun)
			{
				m_pFirstRun->setPrevRun(pLastRun);
			}
		}
		else
		{
			pPrevBL->m_pFirstRun = m_pFirstRun;
		}
		UT_DEBUGMSG(("deleteStrux: offset = %d \n",offset));

		// Tell all the new runs where they live
		pRun = m_pFirstRun;
		while (pRun)
		{
			pRun->setBlockOffset(pRun->getBlockOffset() + offset);
			pRun->setBlock(pPrevBL);

			// Detach from their line
			fp_Line* pLine = pRun->getLine();
			UT_ASSERT(pLine);
			if(pLine)
			{
				pLine->removeRun(pRun);
			}
			if(pLastLine)
			{
				pLastLine->addRun(pRun);
			}
			pRun = pRun->getNextRun();
		}

		// Runs are no longer attached to this block
		m_pFirstRun = NULL;
	}
	//
	// Transfer any frames from this block to the previous block in the
	// the document.
	//
	fl_BlockLayout * pPrevForFrames = pPrevBL;
	if(pPrevForFrames == NULL)
	{
		pPrevForFrames = getPrevBlockInDocument();
	}
	if(pPrevForFrames)
	{
		if(getNumFrames() > 0)
		{
			fl_FrameLayout * pFrame = NULL;
			UT_sint32 i = 0;
			UT_sint32 count = getNumFrames();
			for(i= 0; i < count; i++)
		  	{
				pFrame = getNthFrameLayout(0);
				removeFrame(pFrame);
				pPrevForFrames->addFrame(pFrame);
			}
		}
	}
	// Get rid of everything else about the block
	purgeLayout();
	//
	// Update it's TOC entry
	//
	if(m_pLayout->isBlockInTOC(this))
	{
		m_pLayout->removeBlockFromTOC(this);
	}
	// Unlink this block
	if(getNext() && getNext()->getNext() &&  getNext()->getNext()->getContainerType() == FL_CONTAINER_TOC)
		{
			xxx_UT_DEBUGMSG(("Next container is TOC \n"));
		}
	fl_SectionLayout* pSL = static_cast<fl_SectionLayout *>(myContainingLayout());
	UT_ASSERT(pSL);
	if(pSL)
	{
		pSL->remove(this);
	}
	if (pPrevBL)
	{
//
// Now fix up the previous block. Calling this format fixes bug 2702
//
		fp_Run * pPrevBLRun =pPrevBL->getFirstRun();
		while(pPrevBLRun)
		{
			pPrevBLRun->lookupProperties();
			pPrevBLRun = pPrevBLRun->getNextRun();
		}
		pPrevBL->format();

#ifdef ENABLE_SPELL
		// This call will dequeue the block from background checking
		// if necessary
		m_pSpellSquiggles->join(offset, pPrevBL);
		m_pGrammarSquiggles->join(offset, pPrevBL);
#endif
		pPrevBL->setNeedsReformat(pPrevBL);
		//
		// Update if it's TOC entry by removing then restoring
		//
		if(m_pLayout->isBlockInTOC(pPrevBL))
		{
			m_pLayout->removeBlockFromTOC(pPrevBL);
			m_pLayout->addOrRemoveBlockFromTOC(pPrevBL);
		}
	}
	else
	{
#ifdef ENABLE_SPELL
		// In case we've never checked this one
		m_pLayout->dequeueBlockForBackgroundCheck(this);
#endif
	}
	if(pSL)
	{
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
		if(pView)
			pView->updateCarets(pcrx->getPosition(),-1);
		_assertRunListIntegrity();
	}

	delete this;			// FIXME: whoa!  this construct is VERY dangerous.

	return true;
}

bool fl_BlockLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	
	_assertRunListIntegrity();

	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);

	// Check if the block has borders. If this changes we might have to update other blocks  
	
	bool b_bordersMergedWithPrev = false, b_bordersMergedWithNext = false;
	if (hasBorders())
	{
		b_bordersMergedWithNext = canMergeBordersWithNext();
		b_bordersMergedWithPrev = canMergeBordersWithPrev();
	}

	// erase the old version
	if(!isHdrFtr())
	{
		clearScreen(m_pLayout->getGraphics());
	}
	if(getPrev())
	{
		getPrev()->setNeedsReformat(getPrev());
	}
	collapse();
	setAttrPropIndex(pcrxc->getIndexAP());
	xxx_UT_DEBUGMSG(("SEVIOR: In changeStrux in fl_BlockLayout %x \n",this));
//
// Not sure if we'll ever need this. We don't need this now I'll comment it out.
//	const gchar * szOldStyle = m_szStyle;
	UT_BidiCharType iOldDomDirection = m_iDomDirection;

	lookupProperties();
	xxx_UT_DEBUGMSG(("SEVIOR: Old Style = %s new style = %s \n",szOldStyle,m_szStyle));
//
// Not sure why we need this IF - Sevior
//	if ((szOldStyle != m_szStyle) &&
//		(!szOldStyle || !m_szStyle || !!(strcmp(szOldStyle, m_szStyle))))
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
			pRun->recalcWidth();

			pRun = pRun->getNextRun();
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

	format();
#if 0
//	This was...
	if(m_pDoc->isDoingPaste())
	{
		format();
	}


	// if we were on screen we need to reformat immediately, since the ruler will be
	// calling the findPointCoords() chain and if we are collapsed (as
	// we are now) and contain the point, it will fail
	if(bWasOnScreen)
		format();
	else
		setNeedsReformat(this);
#endif
	updateEnclosingBlockIfNeeded();
	//
	// Need this to find where to break section in the document.
	//
	fl_ContainerLayout * pPrevCL = getPrevBlockInDocument();
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

	if (hasBorders() || b_bordersMergedWithPrev || b_bordersMergedWithNext)
	{
		bool b_bordersMergedWithNextUpdate=canMergeBordersWithNext();
		bool b_bordersMergedWithPrevUpdate=canMergeBordersWithPrev(); 
		if ((b_bordersMergedWithPrev && !b_bordersMergedWithPrevUpdate) ||
			(!b_bordersMergedWithPrev && b_bordersMergedWithPrevUpdate))
		{
			fl_BlockLayout * pPrev = static_cast<fl_BlockLayout *>(getPrev());
			if (pPrev)
			{
				pPrev->setLineHeightBlockWithBorders(-1);
			}
		}
		if ((b_bordersMergedWithNext && !b_bordersMergedWithNextUpdate) ||
			(!b_bordersMergedWithNext && b_bordersMergedWithNextUpdate))
		{
			fl_BlockLayout * pNext = static_cast<fl_BlockLayout *>(getNext());
			if (pNext)
			{
				pNext->setLineHeightBlockWithBorders(1);
			}
		}
	}



	return true;
}

bool fl_BlockLayout::doclistener_insertFirstBlock(const PX_ChangeRecord_Strux * pcrx,
												  pf_Frag_Strux* sdh,
												  PL_ListenerId lid,
												  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																		  PL_ListenerId lid,
																		  fl_ContainerLayout* sfhNew))
{
	//	Exchange handles with the piece table
	fl_ContainerLayout* sfhNew = this;
	//
	// Don't bind to shadows!
	//
	if(pfnBindHandles)
	{
		pfnBindHandles(sdh,lid,sfhNew);
	}
	setNeedsReformat(this);
	updateEnclosingBlockIfNeeded();

	FV_View* pView = getView();
	if (pView && (pView->isActive() || pView->isPreview()))
		pView->_setPoint(pcrx->getPosition());
	else if (pView && ((pView->getPoint() == 0) || pView->getPoint() > pcrx->getPosition()) ) pView->_setPoint(pView->getPoint() + fl_BLOCK_STRUX_OFFSET);
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);

	// Run list should be valid now.
	_assertRunListIntegrity();

	return true;
}
bool fl_BlockLayout::doclistener_insertBlock(const PX_ChangeRecord_Strux * pcrx,
											 pf_Frag_Strux* sdh,
											 PL_ListenerId lid,
											 void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	 PL_ListenerId lid,
																	 fl_ContainerLayout* sfhNew))
{
	_assertRunListIntegrity();

	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_InsertStrux);
	UT_ASSERT(pcrx->getStruxType()==PTX_Block);

	fl_SectionLayout* pSL = static_cast<fl_SectionLayout *>(myContainingLayout());
	UT_return_val_if_fail(pSL,false);
	fl_BlockLayout* pNewBL = static_cast<fl_BlockLayout *>(pSL->insert(sdh, this, pcrx->getIndexAP(),FL_CONTAINER_BLOCK));
	if(isHdrFtr())
		pNewBL->setHdrFtr();
	if (!pNewBL)
	{
		UT_DEBUGMSG(("no memory for BlockLayout\n"));
		return false;
	}
	xxx_UT_DEBUGMSG(("Inserting block %x it's sectionLayout type is %d \n",pNewBL,pNewBL->getSectionLayout()->getContainerType()));
	//xxx_UT_DEBUGMSG(("Inserting block at pos %d \n",getPosition(true)));
	//xxx_UT_DEBUGMSG(("shd of strux block = %x of new block is %x \n",getStruxDocHandle(),pNewBL->getStruxDocHandle()));
	// The newly returned block will contain a line and EOP. Delete those
	// since the code below expects an empty block
	pNewBL->_purgeEndOfParagraphRun();

	// Must call the bind function to complete the exchange
	// of handles with the document (piece table) *** before ***
	// anything tries to call down into the document (like all
	// of the view listeners).

	fl_ContainerLayout* sfhNew = pNewBL;
	//
	// Don't Bind to shadows
	//
	if(pfnBindHandles)
	{
		pfnBindHandles(sdh,lid,sfhNew);
	}

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
	//
	// OK Now we have to deal with any embedded containerlayout associated with
    // this block.
	// If they are before the insert point they must be moved to be immediately
	// after this block (and hence before the new block)
	//
	shuffleEmbeddedIfNeeded(this,blockOffset);

	fp_Run* pFirstNewRun = NULL;
	fp_Run* pLastRun = NULL;
	fp_Run* pRun;
	xxx_UT_DEBUGMSG(("BlockOffset %d \n",blockOffset));
	for (pRun=m_pFirstRun; (pRun && !pFirstNewRun);
		 pLastRun=pRun, pRun=pRun->getNextRun())
	{
		// We have passed the point. Why didn't previous Run claim to
		// hold the offset? Make the best of it in non-debug
		// builds. But keep the assert to get us information...
		xxx_UT_DEBUGMSG(("pRun %x pRun->next %x pRun->blockOffset %d pRun->getLength %d \n",pRun,pRun->getNextRun(),pRun->getBlockOffset(),pRun->getLength()));
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
				pTextRun->split(blockOffset,0);
				pFirstNewRun = pRun->getNextRun();
			}
			break;
		}
	}

	while(pFirstNewRun && (pFirstNewRun->getType() == FPRUN_FMTMARK))
	{
		// Since a FmtMark has length zero, both it and the next run
		// have the same blockOffset.  We always want to be to the
		// right of the FmtMark, so we take the next one.
		pFirstNewRun = pFirstNewRun->getNextRun();
	}
	UT_sint32 iEOPOffset = -1;
	if (pFirstNewRun)
	{
		if(pFirstNewRun->getBlockOffset() == blockOffset)
		{
			iEOPOffset = pFirstNewRun->getBlockOffset();
		}
		if (pFirstNewRun->getPrevRun())
		{
			// Break doubly-linked list of runs into two distinct lists.
			// But remember the last Run in this block.

			pLastRun = pFirstNewRun->getPrevRun();
			pFirstNewRun->getPrevRun()->setNextRun(NULL);
			pFirstNewRun->setPrevRun(NULL);
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

	// Move remaining runs to new block
	pNewBL->m_pFirstRun = pFirstNewRun;

	// And update their positions
	for (pRun=pFirstNewRun; (pRun); pRun=pRun->getNextRun())
	{
		pRun->setBlockOffset(pRun->getBlockOffset() - blockOffset);
		pRun->setBlock(pNewBL);
		// TODO [2] the following 2 steps seem expensive considering
		// TODO we already knew width information before divided the
		// TODO char widths data between the two clocks.  see [1].
		pRun->recalcWidth();
	}

	// Explicitly truncate rest of this block's layout
	_truncateLayout(pFirstNewRun);

	// Now make sure this block still has an EOP Run.
	if (m_pFirstRun) {
		UT_return_val_if_fail(pLastRun,false);
		// Create a new end-of-paragraph run and add it to the block.
		fp_EndOfParagraphRun* pNewRun =
			new fp_EndOfParagraphRun(this,   0, 0);
		pLastRun->setNextRun(pNewRun);
		pNewRun->setPrevRun(pLastRun);
		if(iEOPOffset < 0)
		{
			pNewRun->setBlockOffset(pLastRun->getBlockOffset()
									+ pLastRun->getLength());
		}
		else
		{
			pNewRun->setBlockOffset(iEOPOffset);
		}
		if(pLastRun->getLine())
			pLastRun->getLine()->addRun(pNewRun);
		coalesceRuns();
	}
	else
	{
		_insertEndOfParagraphRun();
	}
	setNeedsReformat(this);
	pNewBL->collapse(); // remove all previous lines
	// Throw all the runs onto one jumbo line in the new block
	pNewBL->_stuffAllRunsOnALine();
	if (pNewBL->m_pFirstRun)
		pNewBL->coalesceRuns();
	else
		pNewBL->_insertEndOfParagraphRun();
	pNewBL->setNeedsReformat(pNewBL);
	updateEnclosingBlockIfNeeded();

	//
	// Now transfer the frames of this block to the newly created one
	//
	if(getNumFrames() > 0)
	{
		FL_DocLayout *pDL = getDocLayout();
		fp_Line * pLine = pLastRun->getLine();
		fp_Container * pCon = pLine->getColumn();
		UT_sint32 pLineX = 0;
		UT_sint32 pLineY = 0;
		UT_sint32 pLinePage = 0;
		if (pLine && pCon)
		{
			pLineX = pLine->getX() + pCon->getX() +pCon->getWidth();
			pLineY = pLine->getY() + pCon->getY();
			pLinePage = pDL->findPage(pLine->getPage());
		}
		fl_FrameLayout * pFL = NULL;
		fp_FrameContainer * pFrame = NULL;
		bool b_evalHeightOfFirstBlock = false;
		UT_sint32 extraHeight = 0;
		UT_sint32 i = 0;
		UT_sint32 k = 0;
		UT_sint32 count = getNumFrames();
		for(i= 0; i < count; i++)
		{
			pFL = getNthFrameLayout(k);
			pFrame = static_cast <fp_FrameContainer *> (pFL->getFirstContainer());
			UT_sint32 pFrameX = 0;
			UT_sint32 pFrameY = 0;
			UT_sint32 pFramePage = 0;
			if (pFrame)
			{
				pFrameX = pFrame->getX();
				pFrameY = pFrame->getY();
				pFramePage = pDL->findPage(pFrame->getPage());
			}
			if (!pFrame || (pFramePage > pLinePage) || (pFrameY > pLineY) || (pFrameX > pLineX))
			{
				UT_DEBUGMSG(("Frame %p associated to block %p (2nd)\n",pFL,pNewBL));
				removeFrame(pFL);
				pNewBL->addFrame(pFL);
				if((pFL->getFramePositionTo() == FL_FRAME_POSITIONED_TO_BLOCK) && 
				   (!m_pDoc->isDoingTheDo()))
				{
					const PP_AttrProp* pAP = NULL;
					const gchar * pszYPos = NULL;
					double ypos = 0.;
					pFL->getAP(pAP);
					if(!pAP || !pAP->getProperty("ypos",pszYPos))
					{
						pszYPos = "0.0in";
					}
					if (!b_evalHeightOfFirstBlock)
					{
						fp_Line * ppLine = pLine;
						while(ppLine)
						{
							extraHeight += ppLine->getHeight();
							ppLine = static_cast <fp_Line *> (ppLine->getPrev());
						}
						fp_Line * pLastLine = static_cast <fp_Line *> (getLastContainer());
						if (pLastLine)
							extraHeight += pLastLine->getMarginAfter();
						b_evalHeightOfFirstBlock = true;
					}
					ypos = UT_convertToInches(pszYPos) - double(extraHeight)/UT_LAYOUT_RESOLUTION;
					UT_String sValY = UT_formatDimensionString(DIM_IN,ypos);
					PP_PropertyVector frameProperties = {
						"ypos", sValY.c_str(),
					};
					PT_DocPosition posStart = pFL->getPosition(true)+1;
					PT_DocPosition posEnd = posStart;
					UT_DebugOnly<bool> bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, posStart, posEnd, PP_NOPROPS,
																	 frameProperties, PTX_SectionFrame);
					UT_ASSERT(bRet);
				}
			}
			else
			{
				UT_DEBUGMSG(("Frame %p associated to block %p (1st)\n",pFL,this));
				//Frame stays in first block. Need to change the PieceTable
				if(!m_pDoc->isDoingTheDo())
				{
					pDL->relocateFrame(pFL,this);
				}
				else
				{
					k++;
					continue;
				}
			}
		}
	}


#ifdef ENABLE_SPELL
	// Split squiggles between this and the new block
	m_pSpellSquiggles->split(blockOffset, pNewBL);
	m_pGrammarSquiggles->split(blockOffset, pNewBL);
	m_pLayout->setPendingBlockForGrammar(pNewBL);
#endif
	
	FV_View* pView = getView();
	if (pView && (pView->isActive() || pView->isPreview()))
		pView->_setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
	else if(pView && pView->getPoint() > pcrx->getPosition())
		pView->_setPoint(pView->getPoint() + fl_BLOCK_STRUX_OFFSET);
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);

	_assertRunListIntegrity();
	xxx_UT_DEBUGMSG(("Prev Block = %x type %d Next block = %x type %d \n",pNewBL->getPrev(),pNewBL->getContainerType(),pNewBL->getNext(),pNewBL->getContainerType()));
	return true;
}

/*!
 * This method shuffles any emebedded containers in the block to be placed 
 * after the supplied block.
 *
 * If they are before the insert point they must be moved to be immediately
 * after this block (and hence before the new block)
 */
void fl_BlockLayout::shuffleEmbeddedIfNeeded(fl_BlockLayout * pBlock, UT_uint32 blockOffset)
{
	if(pBlock == NULL)
	{
		return;
	}
	UT_sint32 iEmbed = 0;
	bool bStop = false;
	fl_ContainerLayout * pEmbedCL = NULL;
	while(!bStop)
	{
		iEmbed = pBlock->getEmbeddedOffset(iEmbed, pEmbedCL);
		if(iEmbed < 0)
		{
			bStop = true;
			break;
		}
		if(pEmbedCL == NULL)
		{
			bStop = true;
			break;
		}
		if((blockOffset > 0) && (iEmbed < static_cast<UT_sint32>(blockOffset)))
		{
			iEmbed++;
			continue;
		}
		//
		// Move pEmbedCL to be just after this block.
		//
		// Outer pointers
		//
		fl_ContainerLayout * pBLNext = pBlock->getNext();
		if(pEmbedCL->getPrev() && (pEmbedCL->getPrev() != pBlock))
			{
				pEmbedCL->getPrev()->setNext(pEmbedCL->getNext());
			}
		if(pEmbedCL->getNext() && pBLNext != pEmbedCL)
			{
				pEmbedCL->getNext()->setPrev(pEmbedCL->getPrev());
			}
		//
		// New pointers for EmbedCL
		pEmbedCL->setPrev(static_cast<fl_ContainerLayout *>(pBlock));
		if(pBLNext != pEmbedCL)
			{
				pEmbedCL->setNext(pBlock->getNext());
			}
		//
		// New pointer here
		if(pBlock->getNext() && (pBlock->getNext() != pEmbedCL))
			{
				pBlock->getNext()->setPrev(pEmbedCL);
			}
		pBlock->setNext(pEmbedCL);
		//
		// Now add in the length of the container
		//
		pf_Frag_Strux* sdhStart = pEmbedCL->getStruxDocHandle();
		pf_Frag_Strux* sdhEnd = NULL;
		if(pEmbedCL->getContainerType() == FL_CONTAINER_FOOTNOTE)
		{
			getDocument()->getNextStruxOfType(sdhStart,PTX_EndFootnote, &sdhEnd);
		}
		else if(pEmbedCL->getContainerType() == FL_CONTAINER_ENDNOTE)
		{
			getDocument()->getNextStruxOfType(sdhStart,PTX_EndEndnote, &sdhEnd);
		}
		else if(pEmbedCL->getContainerType() == FL_CONTAINER_ANNOTATION)
		{
			getDocument()->getNextStruxOfType(sdhStart,PTX_EndAnnotation, &sdhEnd);
		}
		else if( pEmbedCL->getContainerType() == FL_CONTAINER_TOC)
		{
			getDocument()->getNextStruxOfType(sdhStart,PTX_EndTOC, &sdhEnd);
		}		
		UT_return_if_fail(sdhEnd != NULL);
		PT_DocPosition posStart = getDocument()->getStruxPosition(sdhStart);
		PT_DocPosition posEnd = getDocument()->getStruxPosition(sdhEnd);
		UT_uint32 iSize = posEnd - posStart + 1;
		iEmbed += iSize;
		getDocSectionLayout()->setNeedsSectionBreak(true,NULL);

	}
}

bool fl_BlockLayout::doclistener_insertSection(const PX_ChangeRecord_Strux * pcrx,
											   SectionType iType,
											   pf_Frag_Strux* sdh,
											   PL_ListenerId lid,
											   void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	   PL_ListenerId lid,
																	   fl_ContainerLayout* sfhNew))
{
	UT_ASSERT(iType == FL_SECTION_DOC || iType == FL_SECTION_HDRFTR
			  || iType == FL_SECTION_TOC
			  || iType == FL_SECTION_FOOTNOTE 
			  || iType == FL_SECTION_ENDNOTE
			  || iType == FL_SECTION_ANNOTATION);

	_assertRunListIntegrity();

	// Insert a section at the location given in the change record.
	// Everything from this point forward (to the next section) needs
	// to be re-parented to this new section.  We also need to verify
	// that this insertion point is at the end of the block (and that
	// another block follows).	This is because a section cannot
	// contain content.

	UT_ASSERT(pcrx);
	UT_ASSERT(pcrx->getType() == PX_ChangeRecord::PXT_InsertStrux);
	UT_ASSERT(iType != FL_SECTION_DOC || pcrx->getStruxType() == PTX_Section);
	UT_ASSERT(iType != FL_SECTION_HDRFTR || pcrx->getStruxType() == PTX_SectionHdrFtr);
	UT_ASSERT(iType != FL_SECTION_FOOTNOTE || pcrx->getStruxType() == PTX_SectionFootnote);
	UT_ASSERT(iType != FL_SECTION_ANNOTATION || pcrx->getStruxType() == PTX_SectionAnnotation);
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
	const gchar* pszNewID = NULL;

	UT_DEBUGMSG(("Insert section at pos %d sdh of section =%p sdh of block =%p \n",getPosition(true),sdh,getStruxDocHandle()));

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
		UT_UNUSED(bres);
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
			UT_return_val_if_fail( pDocSL, false );
//
// Determine if this is a header or a footer.
//
			const gchar* pszSectionType = NULL;
			pHFAP->getAttribute("type", pszSectionType);

			HdrFtrType hfType = FL_HDRFTR_NONE;
			if (pszSectionType && *pszSectionType)
			{
				if(strcmp(pszSectionType,"header") == 0)
					hfType = FL_HDRFTR_HEADER;
				else if (strcmp(pszSectionType,"header-even") == 0)
					hfType = FL_HDRFTR_HEADER_EVEN;
				else if (strcmp(pszSectionType,"header-first") == 0)
					hfType = FL_HDRFTR_HEADER_FIRST;
				else if (strcmp(pszSectionType,"header-last") == 0)
					hfType = FL_HDRFTR_HEADER_LAST;
				else if (strcmp(pszSectionType,"footer") == 0)
					hfType = FL_HDRFTR_FOOTER;
				else if (strcmp(pszSectionType,"footer-even") == 0)
					hfType = FL_HDRFTR_FOOTER_EVEN;
				else if (strcmp(pszSectionType,"footer-first") == 0)
					hfType = FL_HDRFTR_FOOTER_FIRST;
				else if (strcmp(pszSectionType,"footer-last") == 0)
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
	case FL_SECTION_ANNOTATION:
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
		else if (iType == FL_SECTION_ENDNOTE)
		{
			pSL = static_cast<fl_SectionLayout *>(static_cast<fl_ContainerLayout *>(getSectionLayout())->insert(sdh,this,indexAP, FL_CONTAINER_ENDNOTE));
		}
		else if (iType == FL_SECTION_ANNOTATION)
		{
			pSL = static_cast<fl_SectionLayout *>(static_cast<fl_ContainerLayout *>(getSectionLayout())->insert(sdh,this,indexAP, FL_CONTAINER_ANNOTATION));
		}
//
// Need to find the DocSectionLayout associated with this.
//
		const PP_AttrProp* pAP = NULL;
		bool bres = (m_pDoc->getAttrProp(indexAP, &pAP) && pAP);
		UT_UNUSED(bres);
		UT_ASSERT(bres);
		pAP->getAttribute("id", pszNewID);
		break;
	}
	case FL_SECTION_TOC:
	{
		// Most of the time, we would insert a new section
		// after the previous section.
		// But, here we insert our TOCLayout after this(?)
		PT_AttrPropIndex indexAP = pcrx->getIndexAP();
		pSL = static_cast<fl_SectionLayout *>(static_cast<fl_ContainerLayout *>(getSectionLayout())->insert(sdh,this,indexAP, FL_CONTAINER_TOC));

		// Must call the bind function to complete the exchange of handles
		// with the document (piece table) *** before *** anything tries
		// to call down into the document (like all of the view
		// listeners).

		fl_ContainerLayout* sfhNew = pSL;
		//
		// Don't bind to shadows
		//
		if(pfnBindHandles)
		{
			pfnBindHandles(sdh,lid,sfhNew);
		}
		//
		// That's all we need to do except update the view pointers I guess..
		//
		FV_View* pView = getView();
		if (pView && (pView->isActive() || pView->isPreview()))
		{
			pView->_setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
		}
		else if(pView && pView->getPoint() > pcrx->getPosition())
		{
			//
			// For EndTOC
			//
			pView->_setPoint(pView->getPoint() + fl_BLOCK_STRUX_OFFSET + fl_BLOCK_STRUX_OFFSET);
		}
		if(pView)
			pView->updateCarets(pcrx->getPosition(),2);
		return true;
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

	fl_ContainerLayout* sfhNew = pSL;
	//
	// Don't bind to shadows
	//
	if(pfnBindHandles)
	{
		pfnBindHandles(sdh,lid,sfhNew);
	}

	fl_SectionLayout* pOldSL = m_pSectionLayout;

	if ((iType == FL_SECTION_FOOTNOTE) || (iType == FL_SECTION_ENDNOTE) || (iType == FL_SECTION_ANNOTATION))
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
		if(pView)
			pView->updateCarets(pcrx->getPosition(),2);
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
	while(pCL && ((pCL->getContainerType() == FL_CONTAINER_FOOTNOTE) 
				  || (pCL->getContainerType() == FL_CONTAINER_ENDNOTE)
				  || (pCL->getContainerType() == FL_CONTAINER_ANNOTATION)))
	{
		pLastCL = pCL;
		pCL = pCL->getNext();
	}
	fl_BlockLayout * pBL = NULL;
	while (pCL)
	{
		//
		// When inserting a HEADER/FOOTER dont move footnotes/endnotes into
		// Header/Footer
		//
		if((iType== FL_SECTION_HDRFTR) && (pCL->getContainerType() == FL_CONTAINER_FOOTNOTE 
										 || pCL->getContainerType() == FL_CONTAINER_ENDNOTE 
										 || pCL->getContainerType() == FL_CONTAINER_ANNOTATION 
										 || pCL->getContainerType() == FL_CONTAINER_TOC 
										 ||   pCL->getContainerType() == FL_CONTAINER_FRAME))
		{
			pCL = pCL->getNext();
			continue;
		}

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
		if(pSL->getType() == FL_SECTION_DOC)
		{
			fl_DocSectionLayout * pDDSL = static_cast<fl_DocSectionLayout *>(pSL);
			if(pCL->getContainerType() == FL_CONTAINER_FOOTNOTE)
			{
				static_cast<fl_FootnoteLayout *>(pCL)->
					setDocSectionLayout(pDDSL);
			}
			if(pCL->getContainerType() == FL_CONTAINER_ENDNOTE)
			{
				static_cast<fl_EndnoteLayout *>(pCL)->
					setDocSectionLayout(pDDSL);
			}
			if(pCL->getContainerType() == FL_CONTAINER_ANNOTATION)
			{
				static_cast<fl_EndnoteLayout *>(pCL)->
					setDocSectionLayout(pDDSL);
			}
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
	if(iType == FL_SECTION_HDRFTR || iType == FL_SECTION_FOOTNOTE || iType == FL_SECTION_ANNOTATION)
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
	if(pView)
		pView->updateCarets(pcrx->getPosition(),2);

	_assertRunListIntegrity();
#if DEBUG
	if(getFirstContainer())
	{
		UT_ASSERT(getFirstContainer()->getPrev() == NULL);
	}
#endif
	return true;
}

/*!
 * Insert a table into the list of blocks
 */
fl_SectionLayout * fl_BlockLayout::doclistener_insertTable(const PX_ChangeRecord_Strux * pcrx,
														   SectionType iType,
											   pf_Frag_Strux* sdh,
											   PL_ListenerId lid,
											   void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	   PL_ListenerId lid,
																	   fl_ContainerLayout* sfhNew))
{
	UT_UNUSED(iType);
	UT_ASSERT(iType == FL_SECTION_TABLE);
	_assertRunListIntegrity();

	// Insert a section at the location given in the change record.
	// Everything from this point forward (to the next section) needs
	// to be re-parented to this new section.  We also need to verify
	// that this insertion point is at the end of the block (and that
	// another block follows).	This is because a section cannot
	// contain content.

	UT_ASSERT(pcrx);
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

	fl_ContainerLayout* sfhNew = pSL;
	//
	// Don't bind to shadows
	//
	if(pfnBindHandles)
	{
		pfnBindHandles(sdh,lid,sfhNew);
	}

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
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);
//
// OK that's it!
//
	updateEnclosingBlockIfNeeded();

	return pSL;
}

/*!
 * Insert a Frame after this block.
 */
fl_SectionLayout * fl_BlockLayout::doclistener_insertFrame(const PX_ChangeRecord_Strux * pcrx,
														   SectionType iType,
											   pf_Frag_Strux* sdh,
											   PL_ListenerId lid,
											   void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	   PL_ListenerId lid,
																	   fl_ContainerLayout* sfhNew))
{
	UT_UNUSED(iType);
	UT_ASSERT(iType == FL_SECTION_FRAME);
	_assertRunListIntegrity();

	// Insert a section at the location given in the change record.
	// Everything from this point forward (to the next section) needs
	// to be re-parented to this new section.  We also need to verify
	// that this insertion point is at the end of the block (and that
	// another block follows).	This is because a section cannot
	// contain content.

	UT_ASSERT(pcrx);
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

	pSL = static_cast<fl_SectionLayout *>(static_cast<fl_ContainerLayout *>(getSectionLayout())->insert(sdh,this,pcrx->getIndexAP(), FL_CONTAINER_FRAME));

		// Must call the bind function to complete the exchange of handles
		// with the document (piece table) *** before *** anything tries
		// to call down into the document (like all of the view
		// listeners).

	fl_ContainerLayout* sfhNew = pSL;
	//
	// Don't bind to shadows
	//
	if(pfnBindHandles)
	{
		pfnBindHandles(sdh,lid,sfhNew);
	}

	// Create a Physical Container for this frame

	static_cast<fl_FrameLayout *>(pSL)->format();
  	getDocSectionLayout()->completeBreakSection();
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
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);
//
// OK that's it!
//
	updateEnclosingBlockIfNeeded();

	return pSL;
}

#ifdef ENABLE_SPELL
/*!
 Draw squiggles intersecting with Run
 \param pRun Run

 For all misspelled words in this run, call the run->drawSquiggle()
 method.
*/
void
fl_BlockLayout::findSpellSquigglesForRun(fp_Run* pRun) const
{
	xxx_UT_DEBUGMSG(("fl_BlockLayout::findSpellSquigglesForRun\n"));

	UT_ASSERT(pRun->getType() == FPRUN_TEXT);
	fp_TextRun* pTextRun = (static_cast<fp_TextRun*>(pRun));

	UT_sint32 runBlockOffset = pRun->getBlockOffset();
	UT_sint32 runBlockEnd = runBlockOffset + pRun->getLength();
	UT_sint32 iFirst, iLast;
	if (m_pSpellSquiggles->findRange(runBlockOffset, runBlockEnd, iFirst, iLast))
	{
		UT_sint32 iStart = 0, iEnd;
		fl_PartOfBlock* pPOB;
		UT_sint32 i = iFirst;

		// The first POB may only be partially within the region. Clip
		// it if necessary.
		pPOB = m_pSpellSquiggles->getNth(i++);
		if (!pPOB->getIsIgnored())
		{
			iStart = pPOB->getOffset();
			iEnd =	iStart + pPOB->getPTLength();
			if (iStart < runBlockOffset) iStart = runBlockOffset;

			// Only draw if there's more than one POB. If there's only
			// one POB, it may also need clipping at the end (let the
			// code below handle it).
			if (iFirst != iLast)
			{
				pTextRun->drawSquiggle(iStart, iEnd - iStart,FL_SQUIGGLE_SPELL);
			}
		}
		// The ones in the middle don't need clipping.
		for (; i < iLast; i++)
		{
			pPOB = m_pSpellSquiggles->getNth(i);
			if (pPOB->getIsIgnored()) continue;

			iStart = pPOB->getOffset();
			iEnd =	iStart + pPOB->getPTLength();
			pTextRun->drawSquiggle(iStart, iEnd - iStart,FL_SQUIGGLE_SPELL);
		}
		// The last POB may only be partially within the region. Clip
		// it if necessary. Note the load with iLast instead of i.
		pPOB = m_pSpellSquiggles->getNth(iLast);
		if (!pPOB->getIsIgnored())
		{
			// Only load start if this POB is different from the first
			// one.
			if (iFirst != iLast)
				iStart = pPOB->getOffset();
			iEnd =	pPOB->getOffset() + pPOB->getPTLength();
			if (iEnd > runBlockEnd) iEnd = runBlockEnd;
			pTextRun->drawSquiggle(iStart, iEnd - iStart,FL_SQUIGGLE_SPELL);
		}
	}
}

/*!
 * Draw all the grammar squiggles in the Block.
 */
void fl_BlockLayout::drawGrammarSquiggles(void) const
{
	fp_Run * pRun = getFirstRun();
	while(pRun)
	{
		if(pRun->getType() == FPRUN_TEXT)
		{
			findGrammarSquigglesForRun(pRun);
		}
		pRun = pRun->getNextRun();
	}
}

/*!
 Draw grammar squiggles intersecting with Run
 \param pRun Run

 For all incorrect grammar in this run, call the run->drawSquiggle()
 method.
*/
void
fl_BlockLayout::findGrammarSquigglesForRun(fp_Run* pRun) const
{
	xxx_UT_DEBUGMSG(("fl_BlockLayout::findSpellSquigglesForRun\n"));

	UT_ASSERT(pRun->getType() == FPRUN_TEXT);
	fp_TextRun* pTextRun = (static_cast<fp_TextRun*>(pRun));

	UT_sint32 runBlockOffset = pRun->getBlockOffset();
	UT_sint32 runBlockEnd = runBlockOffset + pRun->getLength();
	UT_sint32 iFirst, iLast;
	if (m_pGrammarSquiggles->findRange(runBlockOffset, runBlockEnd, iFirst, iLast,true))
	{
		UT_sint32 iStart = 0, iEnd;
		fl_PartOfBlock* pPOB;
		UT_sint32 i = iFirst;

		// The first POB may only be partially within the region. Clip
		// it if necessary.
		pPOB = m_pGrammarSquiggles->getNth(i++);
		if (!pPOB->getIsIgnored() && !pPOB->isInvisible())
		{
			iStart = pPOB->getOffset();
			iEnd =	iStart + pPOB->getPTLength();
			if (iStart < runBlockOffset) iStart = runBlockOffset;

			// Only draw if there's more than one POB. If there's only
			// one POB, it may also need clipping at the end (let the
			// code below handle it).
			//			if (iFirst != iLast)
			{
				pTextRun->drawSquiggle(iStart, iEnd - iStart,FL_SQUIGGLE_GRAMMAR);
			}
		}
		// The ones in the middle don't need clipping.
		for (; i < iLast; i++)
		{
			pPOB = m_pGrammarSquiggles->getNth(i);
			if (pPOB->getIsIgnored() || pPOB->isInvisible()) continue;

			iStart = pPOB->getOffset();
			iEnd =	iStart + pPOB->getPTLength();
			pTextRun->drawSquiggle(iStart, iEnd - iStart,FL_SQUIGGLE_GRAMMAR);
		}
		// The last POB may only be partially within the region. Clip
		// it if necessary. Note the load with iLast instead of i.
		pPOB = m_pGrammarSquiggles->getNth(iLast);
		if (!pPOB->getIsIgnored() && !pPOB->isInvisible())
		{
			// Only load start if this POB is different from the first
			// one.
			if (iFirst != iLast)
				iStart = pPOB->getOffset();
			if(iStart < (UT_sint32)pTextRun->getBlockOffset())
				iStart = pTextRun->getBlockOffset();
			iEnd =	pPOB->getOffset() + pPOB->getPTLength();
			if (iEnd > runBlockEnd) iEnd = runBlockEnd;
			pTextRun->drawSquiggle(iStart, iEnd - iStart,FL_SQUIGGLE_GRAMMAR);
		}
	}
}
#endif

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

		xxx_UT_DEBUGMSG(("Populate:InsertObject:Image:\n"));
		_doInsertImageRun(blockOffset, pFG,pcro->getObjectHandle());
		return true;
	}

	case PTO_Field:
		xxx_UT_DEBUGMSG(("!!!Populate:InsertObject:Field: BlockOffset %d \n",blockOffset));
		_doInsertFieldRun(blockOffset, pcro);
		return true;

	case PTO_Bookmark:
		xxx_UT_DEBUGMSG(("Populate:InsertBookmark:\n"));
		_doInsertBookmarkRun(blockOffset);
		return true;

	case PTO_Hyperlink:
		xxx_UT_DEBUGMSG(("Populate:InsertHyperlink:\n"));
		_doInsertHyperlinkRun(blockOffset);
		return true;

	case PTO_Annotation:
		xxx_UT_DEBUGMSG(("Populate:InsertHyperlink:\n"));
		_doInsertAnnotationRun(blockOffset);
		return true;

	case PTO_RDFAnchor:
		xxx_UT_DEBUGMSG(("Populate:InsertHyperlink, RDFAnchor:\n"));
		_doInsertRDFAnchorRun(blockOffset);
		return true;
		
	case PTO_Math:
		xxx_UT_DEBUGMSG(("Populate:InsertMathML:\n"));
		_doInsertMathRun(blockOffset,pcro->getIndexAP(),pcro->getObjectHandle());
		return true;


	case PTO_Embed:
		UT_DEBUGMSG(("Populate Embed:\n"));
		_doInsertEmbedRun(blockOffset,pcro->getIndexAP(),pcro->getObjectHandle());
		return true;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}

	_assertRunListIntegrity();
	updateEnclosingBlockIfNeeded();
	if(isHidden() == FP_HIDDEN_FOLDED)
	{
		collapse();
	}
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

		_doInsertImageRun(blockOffset, pFG,pcro->getObjectHandle());
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


	case PTO_Annotation:
	{
		UT_DEBUGMSG(("Edit:InsertObject:Hyperlink:\n"));
		blockOffset = pcro->getBlockOffset();
		_doInsertAnnotationRun(blockOffset);
		break;

	}

    case PTO_RDFAnchor:
	{
		UT_DEBUGMSG(("Edit:InsertObject:Hyperlink, RDFAnchor:\n"));
		blockOffset = pcro->getBlockOffset();
		_doInsertRDFAnchorRun(blockOffset);
		break;

	}

	case PTO_Math:
	{
		UT_DEBUGMSG(("Edit:InsertObject:Math:\n"));
		blockOffset = pcro->getBlockOffset();
		_doInsertMathRun(blockOffset,pcro->getIndexAP(),pcro->getObjectHandle());
		break;

	}


	case PTO_Embed:
	{
		UT_DEBUGMSG(("Edit:InsertObject:Embed:\n"));
		blockOffset = pcro->getBlockOffset();
		_doInsertEmbedRun(blockOffset,pcro->getIndexAP(),pcro->getObjectHandle());
		break;

	}


	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	m_iNeedsReformat = blockOffset;
	//
	// Update the offsets before the format (Where stuff gets calculated)
	//
	updateEnclosingBlockIfNeeded();
	format();

	FV_View* pView = getView();
	if (pView && (pView->isActive() || pView->isPreview()))
		pView->_setPoint(pcro->getPosition() + 1);
	else if(pView && pView->getPoint() > pcro->getPosition())
		pView->_setPoint(pView->getPoint() + 1);
	if(pView)
		pView->updateCarets(pcro->getPosition(),1);

#ifdef ENABLE_SPELL
	// TODO: are objects always one wide?
	m_pSpellSquiggles->textInserted(blockOffset, 1);
	m_pGrammarSquiggles->textInserted(blockOffset, 1);
#endif
	
	_assertRunListIntegrity();
	//
	// OK Now do the insertSpan for any TOC's that shadow this block.
	//
	if(!isNotTOCable() && !m_bIsTOC && m_bStyleInTOC)
	{
		UT_GenericVector<fl_BlockLayout *> vecBlocksInTOCs;
		if(m_pLayout->getMatchingBlocksFromTOCs(this, &vecBlocksInTOCs))
		{
			UT_sint32 i = 0;
			for(i=0; i<vecBlocksInTOCs.getItemCount();i++)
			{
				fl_BlockLayout * pBL = vecBlocksInTOCs.getNthItem(i);
				pBL->doclistener_insertObject(pcro);
			}
		}
		else
		{
			m_bStyleInTOC = false;
		}
	}

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
		case PTO_Math:
		{
			UT_DEBUGMSG(("Edit:DeleteObject:Math:\n"));
			blockOffset = pcro->getBlockOffset();
			_delete(blockOffset, 1);
			break;
		}
		case PTO_Embed:
		{
			UT_DEBUGMSG(("Edit:DeleteObject:Embed:\n"));
			blockOffset = pcro->getBlockOffset();
			_delete(blockOffset, 1);
			break;
		}

		case PTO_Field:
		{
			xxx_UT_DEBUGMSG(("Edit:DeleteObject:Field:\n"));
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


		case PTO_Annotation:
		{
			UT_DEBUGMSG(("Edit:DeleteObject:Annotation:\n"));
			blockOffset = pcro->getBlockOffset();
			_delete(blockOffset,1);
			break;
		}

		case PTO_RDFAnchor:
		{
			UT_DEBUGMSG(("Edit:DeleteObject:RDFAnchor:\n"));
			blockOffset = pcro->getBlockOffset();
			_delete(blockOffset,1);
			break;
		}
		
		default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	updateEnclosingBlockIfNeeded();
	m_iNeedsReformat = blockOffset;
	format();

	FV_View* pView = getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->_resetSelection();
		pView->_setPoint(pcro->getPosition());
	}
	else if(pView && pView->getPoint() > pcro->getPosition())
		pView->_setPoint(pView->getPoint() - 1);
	if(pView)
		pView->updateCarets(pcro->getPosition(),-1);

#ifdef ENABLE_SPELL
	// TODO: are objects always one wide?
	if(m_pSpellSquiggles)
		m_pSpellSquiggles->textDeleted(blockOffset, 1);
	if(m_pGrammarSquiggles)
		m_pGrammarSquiggles->textDeleted(blockOffset, 1);
#endif
	
	_assertRunListIntegrity();
	//
	// OK Now do the deleteObject for any TOC's that shadow this block.
	//
	if(!isNotTOCable() && !m_bIsTOC && m_bStyleInTOC && m_pLayout)
	{
		UT_GenericVector<fl_BlockLayout *> vecBlocksInTOCs;
		if( m_pLayout->getMatchingBlocksFromTOCs(this, &vecBlocksInTOCs))
		{
			UT_sint32 i = 0;
			for(i=0; i<vecBlocksInTOCs.getItemCount();i++)
			{
				fl_BlockLayout * pBL = vecBlocksInTOCs.getNthItem(i);
				pBL->doclistener_deleteObject(pcro);
			}
		}
		else
		{
			m_bStyleInTOC = false;
		}
	}

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
	case PTO_Annotation:
		return true;
	case PTO_Image:
	{
		xxx_UT_DEBUGMSG(("Edit:ChangeObject:Image:\n"));
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
						pRun = pRun->getNextRun();
					}
				}
				if(!pRun || pRun->getType() != FPRUN_IMAGE)
				{
					UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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
			pRun = pRun->getNextRun();
		}

		return false;
	}
	case PTO_Field:
	{
		xxx_UT_DEBUGMSG(("Edit:ChangeObject:Field:\n"));
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
						pRun = pRun->getNextRun();
					}
				}
				if(!pRun || pRun->getType() != FPRUN_FIELD)
				{
					UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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
			pRun = pRun->getNextRun();
		}

		return false;
	}
	case PTO_Math:
	{
		UT_DEBUGMSG(("Edit:ChangeObject:Math:\n"));
		blockOffset = pcroc->getBlockOffset();
		fp_Run* pRun = m_pFirstRun;
		while (pRun)
		{
			if (pRun->getBlockOffset() == blockOffset && (pRun->getType()!= FPRUN_FMTMARK))
			{
				if(pRun->getType()!= FPRUN_MATH)
				{
					UT_DEBUGMSG(("!!! run type NOT OBJECT, instead = %d !!!! \n",pRun->getType()));
					while(pRun && pRun->getType() == FPRUN_FMTMARK)
					{
						pRun = pRun->getNextRun();
					}
				}
				if(!pRun || pRun->getType() != FPRUN_MATH)
				{
					UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
					return false;
				}
				fp_MathRun* pMathRun = static_cast<fp_MathRun*>(pRun);
				if(!isHdrFtr())
				{
					pMathRun->clearScreen();
				}
				pMathRun->lookupProperties();

				goto done;
			}
			pRun = pRun->getNextRun();
		}

		return false;
	}

	case PTO_Embed:
	{
		UT_DEBUGMSG(("Edit:ChangeObject:Embed:\n"));
		blockOffset = pcroc->getBlockOffset();
		fp_Run* pRun = m_pFirstRun;
		while (pRun)
		{
			if (pRun->getBlockOffset() == blockOffset && (pRun->getType()!= FPRUN_FMTMARK))
			{
				if(pRun->getType()!= FPRUN_EMBED)
				{
					UT_DEBUGMSG(("!!! run type NOT OBJECT, instead = %d !!!! \n",pRun->getType()));
					while(pRun && pRun->getType() == FPRUN_FMTMARK)
					{
						pRun = pRun->getNextRun();
					}
				}
				if(!pRun || pRun->getType() != FPRUN_EMBED)
				{
					UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
					return false;
				}
				fp_EmbedRun* pEmbedRun = static_cast<fp_EmbedRun*>(pRun);
				if(!isHdrFtr())
				{
					pEmbedRun->clearScreen();
				}
				pEmbedRun->update ();
				pEmbedRun->lookupProperties();

				goto done;
			}
			pRun = pRun->getNextRun();
		}

		return false;
	}

	default:
		UT_ASSERT_HARMLESS(0);
		return false;
	}

 done:
	m_iNeedsReformat = blockOffset;
	format();
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
			/*	TODO: Write list (fl_autonum, I think) code adding a member bool
			 * indicating if the list structure has changed since the last field recalc
			 * and setting it to true whenever such a change occurs (ie, adding an item,
			 * deleting one, whatever).  Then here you can if(pFieldRun->getFieldType() ==
			 * FPFIELD_list_label) * get the list to which it belongs, and get that list's
			 * * m_bDirtyForFieldRecalc which is set true after any change * to the list
			 * structure. Thus only recalc if needed.  Finally, after the loop, tell the
			 * list to reset that member to false.  However, the possible down side to
			 * this is that you need to recalc the entire list and not just the individual
			 * fields, because otherwise you risk having an only partially recalced list
			 * being left alone because it's marked clean... in retrospect I think you may
			 * need to move this sort of optimization up to the DocSectionLayout redraw
			 * code, rather than having it here at the block level where it might (not
			 * 100% sure but might) be waaay overcalculated (having a 1-1 block-li
			 * situation.  In fl_DocSectionLayout::redrawUpdate, you just make a special
			 * case for if any block encountered has an autonum (as opposed to any old
			 * field), and if so you do this (recalc the whole list and mark it no longer
			 * dirty for recalc), and then subsequent blocks with part of the same autonum
			 * will pass over recalculating it.  You may still need (or want) a new method
			 * in BL to recalculateAutoNums, called separately from recalculateFields
			 * (which ignores fields from autonums), to ease still recalculating
			 * non-autonum fields from the DSL code.  */
	
			xxx_UT_DEBUGMSG(("DOM: %d %d\n", pFieldRun==0, pFieldRun->needsFrequentUpdates()));

			if((!iUpdateCount
				|| !pFieldRun->needsFrequentUpdates()
				|| !(iUpdateCount % pFieldRun->needsFrequentUpdates())))
			{
				const bool bSizeChanged = pFieldRun->calculateValue();
				bResult |= bSizeChanged;
			}
		}
		//
		// See if Annotation or RDF has changed
		//
		if (pRun->getType() == FPRUN_HYPERLINK)
		{
				fp_HyperlinkRun * pHRun = pRun->getHyperlink();
				if(pHRun && pHRun->getHyperlinkType() == HYPERLINK_ANNOTATION)
				{
					fp_AnnotationRun * pARun = static_cast<fp_AnnotationRun *>(pHRun);
					UT_sint32 iWidth = pARun->getWidth();
					pARun->recalcWidth();
					if(iWidth != pARun->getWidth())
					{
						bResult |= true;
					}
				}				
				if(pHRun && pHRun->getHyperlinkType() == HYPERLINK_RDFANCHOR)
				{
					fp_RDFAnchorRun* pARun = static_cast<fp_RDFAnchorRun*>(pHRun);
					UT_sint32 iWidth = pARun->getWidth();
					pARun->recalcWidth();
					if(iWidth != pARun->getWidth())
					{
						bResult |= true;
					}
				}				
		}

		//				else if(pRun->isField() == true)
		//	{
		//		 bResult = pRun->getField()->update();
		//}
		pRun = pRun->getNextRun();
	}

	_assertRunListIntegrity();

	return bResult;
}


bool	fl_BlockLayout::findNextTabStop( UT_sint32 iStartX, UT_sint32 iMaxX, UT_sint32& iPosition,
										 eTabType & iType, eTabLeader &iLeader ) const
{
#ifdef DEBUG
	UT_sint32 iMinLeft = m_iLeftMargin;
  	if(getTextIndent() < 0)
		iMinLeft += getTextIndent();
	UT_ASSERT(iStartX >= iMinLeft);
#endif
	
	UT_uint32 iCountTabs = m_vecTabs.getItemCount();
	UT_uint32 i;
	if(isContainedByTOC())
    {
		iCountTabs = 0;
	}
	iLeader = FL_LEADER_NONE;

	for (i=0; i<iCountTabs; i++)
	{
		fl_TabStop* pTab = m_vecTabs.getNthItem(i);
		UT_continue_if_fail(pTab);

		if (pTab->getPosition() > iMaxX)
		{
			break;
		}

		if (pTab->getPosition() > iStartX)
		{
			if(m_iDomDirection == UT_BIDI_RTL)
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

	if(m_iDomDirection == UT_BIDI_RTL)
		iMin = m_iRightMargin;
	else
		iMin = m_iLeftMargin;

	if (iMin > iStartX)
	{
		iPosition = iMin;

		if(m_iDomDirection == UT_BIDI_RTL)
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

	if(m_iDomDirection == UT_BIDI_RTL)
		iType = FL_TAB_RIGHT;
	else
		iType = FL_TAB_LEFT;

	UT_ASSERT(iPos > iStartX);

	return true;
}

bool	fl_BlockLayout::findPrevTabStop( UT_sint32 iStartX, UT_sint32 iMaxX, UT_sint32& iPosition,
										 eTabType & iType, eTabLeader &iLeader ) const
{
#ifdef DEBUG
	UT_sint32 iMinLeft = m_iLeftMargin;
	if(getTextIndent() < 0)
		iMinLeft += getTextIndent();
	
	UT_ASSERT(iStartX >= iMinLeft);
#endif

	UT_uint32 iCountTabs = m_vecTabs.getItemCount();
	UT_uint32 i;

	iLeader = FL_LEADER_NONE;

	for (i=0; i<iCountTabs; i++)
	{
		fl_TabStop* pTab = static_cast<fl_TabStop*>(m_vecTabs.getNthItem(i));
		UT_continue_if_fail(pTab);

		if (pTab->getPosition() > iMaxX)
		{
			break;
		}

		if (pTab->getPosition() > iStartX)
		{
			pTab = static_cast<fl_TabStop*>(m_vecTabs.getNthItem(i>0?i-1:0));
			UT_continue_if_fail(pTab);

			if(m_iDomDirection == UT_BIDI_RTL)
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
			xxx_UT_DEBUGMSG(("found tabstop indx=%d\n", iCountTabs - 1));
			fl_TabStop* pTab = static_cast<fl_TabStop*>(m_vecTabs.getNthItem(iCountTabs - 1));
			UT_return_val_if_fail(pTab,false);

			iPosition = pTab->getPosition();
			iType = pTab->getType();
			iLeader = pTab->getLeader();

			return true;
	}

	// now, handle the default tabs

	UT_sint32 iMin;

	if(m_iDomDirection == UT_BIDI_RTL)
		iMin = m_iRightMargin;
	else
		iMin = m_iLeftMargin;

	if (iMin >= iStartX)
	{
		iPosition = iMin;

		if(m_iDomDirection == UT_BIDI_RTL)
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

		if(m_iDomDirection == UT_BIDI_RTL)
			iType = FL_TAB_RIGHT;
		else
			iType = FL_TAB_LEFT;

	UT_ASSERT(iPos <= iStartX);

	return true;
}

bool fl_BlockLayout::s_EnumTabStops( void * myThis, UT_uint32 k, fl_TabStop *pTabInfo)
{
	// a static function

	const fl_BlockLayout * pBL = static_cast<const fl_BlockLayout*>(myThis);

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

	fp_FmtMarkRun * pNewRun = new fp_FmtMarkRun(this, blockOffset);
	UT_ASSERT(pNewRun);
	_doInsertRun(pNewRun);

	// TODO is it necessary to force a reformat when inserting a
	// FmtMark -- no fmt mark has no width, so it cannot change layout

	FV_View* pView = getView();
	if (pView && (pView->isActive() || pView->isPreview()))
		pView->_setPoint(pcrfm->getPosition());
	if(pView)
		pView->updateCarets(pcrfm->getPosition(),0);

	if (pView)
	{
		pView->_resetSelection();
//		if(!isHdrFtr())
//			pView->notifyListeners(AV_CHG_FMTCHAR);
	}
	m_iNeedsReformat = blockOffset;
	format();
	_assertRunListIntegrity();
	return true;
}

bool
fl_BlockLayout::doclistener_deleteFmtMark(const PX_ChangeRecord_FmtMark* pcrfm)
{
	UT_return_val_if_fail( m_pLayout, false );
	_assertRunListIntegrity();

	PT_BlockOffset blockOffset = pcrfm->getBlockOffset();

	xxx_UT_DEBUGMSG(("Edit:DeleteFmtMark: [blockOffset %ld]\n",blockOffset));

	// we can't use the regular _delete() since we are of length zero
	_deleteFmtMark(blockOffset);

	// TODO is it necessary to force a reformat when deleting a FmtMark
	m_iNeedsReformat = blockOffset;
	format();
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
		if(pView)
			pView->updateCarets(pcrfm->getPosition(),0);
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
		fp_Run* pNextRun = pRun->getNextRun();

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
				m_pFirstRun = pRun->getNextRun();
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
		if ((pRun->getBlockOffset() > blockOffset) ||
			((pRun->getBlockOffset() == blockOffset) && (pRun->getType() != FPRUN_FMTMARK)))
		{
			// The FmtMark run no longer exists. Exit function
			// Since the runs and text fragments are coalesced separately, a FmtMark run may
			// have been deleted even if the corresponding pf_Frag still exists
			return true;
		}

		if (pRun->getBlockOffset() == blockOffset)
		{
			pRun->lookupProperties();
			if(!isHdrFtr())
			{
				pRun->clearScreen();
			}
			break;
		}

		pRun = pRun->getNextRun();
	}

	// We need a reformat for blocks that only contain a format mark.
	// ie. no next just a carrige return.

	m_iNeedsReformat = blockOffset;
	format();
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

#ifdef ENABLE_SPELL
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
	UT_UNUSED(bRes);
	UT_ASSERT(bRes);
	const UT_UCSChar* pBlockText = reinterpret_cast<UT_UCSChar*>(pgb.getPointer(0));

	bool bUpdate = m_pSpellSquiggles->recheckIgnoredWords(pBlockText);

	// Update screen if any words squiggled
	FV_View* pView = getView();
	if (bUpdate && pView)
	{
		pView->updateScreen();
	}
}
#endif

////////////////////////////////////////////////////////////////////////////
//List Item Stuff
///////////////////////////////////////////////////////////////////////////

gchar* fl_BlockLayout::getListStyleString( FL_ListType iListType) const
{

	gchar* style;

	// These strings match piece table styles and should not be
	// internationalized
	UT_sint32 nlisttype = static_cast<UT_sint32>(iListType);
	if(nlisttype < 0 || nlisttype >= static_cast<UT_sint32>(NOT_A_LIST))
		style = static_cast<gchar *>(NULL);
	else
	{
		fl_AutoLists al;
		style = const_cast<gchar *>(al.getXmlList(nlisttype));
	}
	return style;
}

FL_ListType fl_BlockLayout::getListTypeFromStyle( const gchar* style) const
{
	FL_ListType lType = NOT_A_LIST;
	if(style == NULL)
		return lType;
	UT_uint32 j;
	fl_AutoLists al;
	UT_uint32 size_xml_lists = al.getXmlListsSize();
	for(j=0; j < size_xml_lists; j++)
	{
		if( strcmp(style,al.getXmlList(j))==0)
			break;
	}
	if(j < size_xml_lists)
		lType = static_cast<FL_ListType>(j);
	return lType;
}


char *	fl_BlockLayout::getFormatFromListType( FL_ListType iListType) const
{
	UT_sint32 nlisttype = static_cast<UT_sint32>(iListType);
	char * pFormat = NULL;
	if(nlisttype < 0 || nlisttype >= static_cast<UT_sint32>(NOT_A_LIST))
		return pFormat;
	fl_AutoLists al;
	pFormat = const_cast<char *>(al.getFmtList(nlisttype));
	return pFormat;
}

FL_ListType fl_BlockLayout::decodeListType(char * listformat) const
{
	FL_ListType iType = NOT_A_LIST;
	UT_uint32 j;
	fl_AutoLists al;
	UT_uint32 size_fmt_lists = al.getFmtListsSize();
	for(j=0; j < size_fmt_lists; j++)
	{
		if( strstr(listformat,al.getFmtList(j))!=NULL)
			break;
	}
	if(j < size_fmt_lists)
		iType = static_cast<FL_ListType>(j);
	return iType;
}

FL_ListType fl_BlockLayout::getListType(void) const
{
	if(isListItem()==false)
	{
		return NOT_A_LIST;
	}
	else if(getAutoNum())
	{
		return getAutoNum()->getType();
	}
	else
	{
		return NOT_A_LIST;
	}
}

void fl_BlockLayout::remItemFromList(void)
{
	UT_uint32 id;
	UT_DebugOnly<bool> bRet;
	if( m_bListLabelCreated == true)
	{
		m_bListLabelCreated = false;
		UT_ASSERT(getView());


		UT_uint32 currLevel = getLevel();
		UT_ASSERT(currLevel > 0);
		currLevel =0; // was currLevel--;
		std::string buf = UT_std_string_sprintf("%i", currLevel);
		setStopping(false);
		fl_BlockLayout * pNext = getNextBlockInDocument();
		if (currLevel == 0)
		{
			id = 0;
		}
		else
		{
			id = getAutoNum()->getParent()->getID();
			pNext = getPreviousList( id);
		}
		std::string lid = UT_std_string_sprintf("%i", id);

		setStopping(false);
		format();
		//
		// Set formatting to match the next paragraph if it exists
		//
		PP_PropertyVector props;

		if(pNext != NULL)
		{
			pNext->getListPropertyVector(props);
		}
		else
		{
			getListPropertyVector(props);
		}
		PP_setAttribute("text-indent", "0.0000in", props);
		const PP_PropertyVector attribs = {
			"listid", lid,
			"level", buf
		};
		if (currLevel == 0)
		{
			bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), attribs, props, PTX_Block);
			UT_ASSERT(bRet);

			m_bListItem = false;
		}
		else
		{
			bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), attribs, props, PTX_Block);
			UT_ASSERT(bRet);

			m_pDoc->listUpdate(getStruxDocHandle());
		}
		//format();

		//		pView->AV_View::notifyListeners(AV_CHG_FMTBLOCK);
		// pView->_fixInsertionPointCoords();
	}
}

/*!
 * Start a list with the paragraph definition container in the style defined by "style"
\param const XML_CHar * style the name of the paragraph style for this block.
*/
void	fl_BlockLayout::StartList( const gchar * style, pf_Frag_Strux* prevSDH)
{
	//
	// Starts a new list at the current block with list style style all other
	// attributes and properties are the default values
	//
	FL_ListType lType2;
	PD_Style* pStyle = 0;
	const gchar* szDelim     = 0;
	const gchar* szDec       = 0;
	const gchar* szStart     = 0;
	const gchar* szAlign     = 0;
	const gchar* szIndent    = 0;
	const gchar* szFont      = 0;
	const gchar* szListStyle = 0;
	UT_uint32 startv, level, currID;

	// TODO -- this mixture of float and double is a mess, we should
	// either use double throughout or float
	float fAlign, fIndent;

	m_pDoc->getStyle(static_cast<const char *>(style), &pStyle);
	if (pStyle)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Found list of style %s \n",style));
		// Use the props in the style
		pStyle->getProperty(static_cast<const gchar *>("list-delim"), szDelim);
		pStyle->getProperty(static_cast<const gchar *>("list-decimal"), szDec);
		pStyle->getProperty(static_cast<const gchar *>("start-value"), szStart);

		if(m_iDomDirection == UT_BIDI_RTL)
		   pStyle->getProperty(static_cast<const gchar *>("margin-right"), szAlign);
	    else
		   pStyle->getProperty(static_cast<const gchar *>("margin-left"), szAlign);

		pStyle->getProperty(static_cast<const gchar *>("text-indent"), szIndent);
		pStyle->getProperty(static_cast<const gchar *>("field-font"), szFont);
		pStyle->getProperty(static_cast<const gchar *>("list-style"), szListStyle);
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
		double dLeft;
		if(m_iDomDirection == UT_BIDI_LTR)
			dLeft = UT_convertToInches(getProperty("margin-left",true));
		else
			dLeft = UT_convertToInches(getProperty("margin-right",true));

		fAlign += static_cast<float>(dLeft);
		if(!szListStyle)
			szListStyle = style;
		if(szDelim==NULL)
			szDelim="%L";
		if(szDec==NULL)
			szDec=".";
		if(!szFont)
		{
			szFont = "Times New Roman";
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}
	else
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Could NOT find list of style %s \n",style));
		szDelim = "%L";
		startv = 1;
		szDec = ".";
		fAlign = static_cast<float>(LIST_DEFAULT_INDENT);
		fIndent = static_cast<float>(-LIST_DEFAULT_INDENT_LABEL);
		szListStyle = "Numbered List";
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

	lType2 = getListTypeFromStyle(szListStyle);
	StartList( lType2, startv,szDelim, szDec, szFont, fAlign, fIndent, currID,level);
}

void	fl_BlockLayout::getListAttributesVector(PP_PropertyVector & va) const
{
	//
	// This function fills the vector va with list attributes
	//
	UT_uint32 level;
	const gchar * style = NULL;
	const gchar * lid = NULL;

	const PP_AttrProp * pBlockAP = NULL;
	getAP(pBlockAP);
	pBlockAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME,style);
	pBlockAP->getAttribute(static_cast<const gchar *>(PT_LISTID_ATTRIBUTE_NAME),lid);
	if(getAutoNum())
	{
		level = getAutoNum()->getLevel();
	}
	else
	{
		level = 0;
	}
	std::string buf = UT_std_string_sprintf("%i", level);
	//	pBlockAP->getAttribute("level",buf);
	if(lid != NULL)
	{
		va.push_back("listid");
		va.push_back(lid);
	}
	va.push_back("level");
	va.push_back(buf);
	if(style != NULL)
	{
		va.push_back(PT_STYLE_ATTRIBUTE_NAME);
		va.push_back(style);
	}
}


void	fl_BlockLayout::getListPropertyVector(PP_PropertyVector & vp) const
{
	//
	// This function fills the vector vp with list properties. All vector
	// quantities are const gchar *
	//
	const gchar * pszStart = getProperty("start-value",true);
	const gchar * lDelim =  getProperty("list-delim",true);
	const gchar * lDecimal =  getProperty("list-decimal",true);

	const gchar * pszAlign;
	if(m_iDomDirection == UT_BIDI_RTL)
		pszAlign =  getProperty("margin-right",true);
	else
		pszAlign =  getProperty("margin-left",true);

	const gchar * pszIndent =  getProperty("text-indent",true);
	const gchar * fFont =  getProperty("field-font",true);
	const gchar * pszListStyle =  getProperty("list-style",true);
	if(pszStart != NULL)
	{
		vp.push_back("start-value");
		vp.push_back(pszStart);
	}
	if(pszAlign != NULL)
	{
		if(m_iDomDirection == UT_BIDI_RTL)
			vp.push_back("margin-right");
		else
			vp.push_back("margin-left");

		vp.push_back(pszAlign);
	}
	if(pszIndent != NULL)
	{
		vp.push_back("text-indent");
		vp.push_back(pszIndent);
	}
	if(lDelim != NULL)
	{
		vp.push_back("list-delim");
		vp.push_back(lDelim);
	}
	if(lDecimal != NULL)
	{
		vp.push_back("list-decimal");
		vp.push_back(lDecimal);
	}
	if(fFont != NULL)
	{
		vp.push_back("field-font");
		vp.push_back(fFont);
	}
	if(pszListStyle != NULL)
	{
		vp.push_back("list-style");
		vp.push_back(pszListStyle);
	}
}


void	fl_BlockLayout::StartList( FL_ListType lType, UT_uint32 start,const gchar * lDelim, const gchar * lDecimal, const gchar * fFont, float Align, float indent, UT_uint32 iParentID, UT_uint32 curlevel )
{
	//
	// Starts a new list at the current block with all the options
	//
	gchar lid[15], pszAlign[20], pszIndent[20],buf[20],pid[20],pszStart[20];
	gchar * style = getListStyleString(lType);
	UT_DebugOnly<bool> bRet;
	UT_uint32 id=0;

	fl_AutoNum * pAutoNum;
	const PP_AttrProp * pBlockAP = NULL;
	const gchar * szLid=NULL;
	getAP(pBlockAP);
	bool bGetPrevAuto = true;
	if (!pBlockAP || !pBlockAP->getAttribute(PT_LISTID_ATTRIBUTE_NAME, szLid))
		szLid = NULL;
	if (szLid)
		id = atoi(szLid);
	else
		{
			id = 0;
			bGetPrevAuto = false;
		}
			

	UT_ASSERT(getView());
	if(bGetPrevAuto)
	{
		pAutoNum = m_pDoc->getListByID(id);
		UT_DEBUGMSG(("SEVIOR: found autonum %p from id %d \n",pAutoNum,id));
		if(pAutoNum != NULL)
		{
			m_pAutoNum = pAutoNum;
			m_bListItem = true;
			UT_DEBUGMSG(("Found list of id %d \n",id));
			listUpdate();
		}
	}

	UT_return_if_fail(m_pDoc);
	id = m_pDoc->getUID(UT_UniqueId::List);
	
	sprintf(lid, "%i", id);

	sprintf(pid, "%i", iParentID);
	sprintf(buf, "%i", curlevel);
	sprintf(pszStart,"%i",start);

	strncpy( pszAlign, UT_convertInchesToDimensionString(DIM_IN, Align, 0), sizeof(pszAlign));

	strncpy( pszIndent, UT_convertInchesToDimensionString(DIM_IN, indent, 0), sizeof(pszIndent));

	const PP_PropertyVector attribs = {
		"listid", lid,
		"parentid", pid,
		"level", buf
	};

	const PP_PropertyVector props = {
		"start-value", pszStart,
		(m_iDomDirection == UT_BIDI_RTL) ? "margin-right" : "margin-left", pszAlign,
		"text-indent", pszIndent,
		"field-font", fFont,
		"list-style", style,
		"list-delim", lDelim,
		"list-decimal", lDecimal
	};
	xxx_UT_DEBUGMSG(("SEVIOR: Starting List with font %s \n",fFont));

	pAutoNum = new fl_AutoNum(id, iParentID, lType, start, lDelim, lDecimal, m_pDoc, getView());
	if (!pAutoNum)
	{
		// TODO Out of Mem.
	}
	m_pDoc->addList(pAutoNum);
	pAutoNum->fixHierarchy();

	setStarting( false);

	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), attribs, props, PTX_Block);
	UT_ASSERT(bRet);

	m_pDoc->listUpdate(getStruxDocHandle());
}


void	fl_BlockLayout::StopListInBlock(void)
{
	//
	// Stops the list in the current block
	//
	static gchar lid[15],pszlevel[5];
	UT_DebugOnly<bool> bRet;
	UT_uint32 id, level;
	FV_View* pView = getView();
	UT_ASSERT(pView);
	PP_PropertyVector props;
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
	const gchar * szAlign, * szIndent;
	pPrev = getPrevBlockInDocument();
	pNext = getNextBlockInDocument();

	if (id != 0)
	{
		UT_ASSERT_HARMLESS(pPrev);	// TMN: Is an assert appropriate here?

		//First, look for block in list
		bool bmatch = false;
		bmatch = static_cast<bool>(pPrev && pPrev->isListItem() && pPrev->getLevel() == level && pPrev->getAutoNum()->getID() == id);
		while (pPrev && !bmatch)
		{
			pPrev = pPrev->getPrevBlockInDocument();
			if (pPrev && pPrev->isListItem())
				bmatch = static_cast<bool>(pPrev->getLevel() == level
								&& pPrev->getAutoNum()->getID() == id);
		}
		while (pNext  && !bmatch)
		{
			pNext = pNext->getNextBlockInDocument();
			if (pNext && pNext->isListItem())
				bmatch = static_cast<bool>(pNext->getLevel() == level
								&& pNext->getAutoNum()->getID() == id);
		}

		if (pPrev)
			pPrev->getListPropertyVector(props);
		else if (pNext)
			pNext->getListPropertyVector(props);
		else
		{
			// We have a problem
			FL_ListType newType;
			PD_Style * pStyle;
			float fAlign, fIndent;
			gchar align[30], indent[30];

			newType = getAutoNum()->getParent()->getType();
			m_pDoc->getStyle(static_cast<char *>(getListStyleString(newType)), &pStyle);
			if (pStyle)
			{
				if(m_iDomDirection == UT_BIDI_RTL)
					pStyle->getProperty(static_cast<const gchar *>("margin-right"), szAlign);
				else
					pStyle->getProperty(static_cast<const gchar *>("margin-left"), szAlign);

				pStyle->getProperty(static_cast<const gchar *>("text-indent"), szIndent);
				fAlign = static_cast<float>(UT_convertToInches(szAlign));
				fAlign *= level;
				strncpy( align,
								UT_convertInchesToDimensionString(DIM_IN, fAlign, 0),
								sizeof(align));
				sprintf(indent, "%s", szIndent);
			}
			else
			{
				fAlign =  static_cast<float>(LIST_DEFAULT_INDENT) * level;
				fIndent = static_cast<float>(-LIST_DEFAULT_INDENT_LABEL);
				strncpy( align,
								UT_convertInchesToDimensionString(DIM_IN, fAlign, 0),
								sizeof(align));
				strncpy( indent,
								UT_convertInchesToDimensionString(DIM_IN, fIndent, 0),
								sizeof(indent));
			}

			if(m_iDomDirection == UT_BIDI_RTL)
				props.push_back("margin-right");
			else
				props.push_back("margin-left");

			props.push_back(align);
			props.push_back("text-indent");
			props.push_back(indent);
		}
	}
	else
	{
		// Find the last non-list item and set alignment + indent
		while (pPrev && pPrev->isListItem())
			pPrev = pPrev->getPrevBlockInDocument();

		while (pNext && pNext->isListItem())
			pNext = pNext->getNextBlockInDocument();

		if (pPrev)
		{
			if(m_iDomDirection == UT_BIDI_RTL)
				szAlign = pPrev->getProperty("margin-right", true);
			else
				szAlign = pPrev->getProperty("margin-left", true);

			szIndent =	pPrev->getProperty("text-indent", true);
		}
		else if (pNext)
		{
			if(m_iDomDirection == UT_BIDI_RTL)
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

		if(m_iDomDirection == UT_BIDI_RTL)
			props.push_back("margin-right");
		else
			props.push_back("margin-left");

		props.push_back(szAlign);
		props.push_back("text-indent");
		props.push_back(szIndent);
	}
	sprintf(pszlevel, "%i", level);

	if (id == 0)
	{
		const PP_PropertyVector pListAttrs = {
			"listid", "",
			"parentid", "",
			"level", "",
			"type", "",
		};

		// we also need to explicitely clear the list formating
		// properties, since their values are not necessarily part
		// of the style definition, so that cloneWithEliminationIfEqual
		// which we call later will not get rid off them
		const PP_PropertyVector pListProps = {
			"start-value", "",
			"list-style", "",
			(m_iDomDirection == UT_BIDI_RTL) ? "margin-right" : "margin-left", "",
			"text-indent", "",
			"field-color", "",
			"list-delim", "",
			"field-font", "",
			"list-decimal", "",
			"list-tag", ""
		};
//
// Remove all the list related properties
//

		bRet = m_pDoc->changeStruxFmt(PTC_RemoveFmt, getPosition(), getPosition(), pListAttrs, pListProps, PTX_Block);
		UT_ASSERT(bRet);

		fp_Run * pRun = getFirstRun();
		while(pRun->getNextRun())
		{
			pRun = pRun->getNextRun();
		}
		PT_DocPosition lastPos = getPosition(false) + pRun->getBlockOffset();
		bRet = m_pDoc->changeSpanFmt(PTC_RemoveFmt, getPosition(false), lastPos, pListAttrs, pListProps);
		UT_ASSERT(bRet);
//
// Set the indents to match.
//
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), PP_NOPROPS, props, PTX_Block);
		UT_ASSERT(bRet);

		m_bListItem = false;
	}
	else
	{
		const PP_PropertyVector attribs = {
			"listid", lid,
			"level", pszlevel
		};

		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,getPosition(), getPosition(), attribs, props, PTX_Block);
		UT_ASSERT(bRet);

		m_pDoc->listUpdate(getStruxDocHandle());
	}
	// format();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		if(offset > 0 )
			{
				pView->_setPoint(pView->getPoint()+offset-2);
				pView->updateCarets(0,offset-2);
			}
	}
}

/*!
 * Find the most recent block with the list ID given.
\param UT_uint32 id the identifier of the list
\returns fl_BlockLayout *
*/
fl_BlockLayout * fl_BlockLayout::getPreviousList(UT_uint32 id) const
{
	//
	// Find the most recent list item that matches the id given
	//
	UT_ASSERT(m_pAutoNum);
	fl_BlockLayout * pPrev = getPrevBlockInDocument();
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
		pPrev = pPrev->getPrevBlockInDocument();
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


fl_BlockLayout * fl_BlockLayout::getNextList(UT_uint32 id) const
{
	//
	// Find the next list  item that matches the id given
	//
	fl_BlockLayout * pNext = getNextBlockInDocument();
	bool bmatchLevel =	false;
	if( pNext != NULL && pNext->isListItem()&& (pNext->getAutoNum() != NULL))
	{
		bmatchLevel = static_cast<bool>(id == pNext->getAutoNum()->getID());
	}
	while(pNext != NULL && bmatchLevel == false)
	{
		pNext = pNext->getNextBlockInDocument();
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
fl_BlockLayout * fl_BlockLayout::getPreviousList( void) const
{
	//
	// Find the most recent block with a list
	//
	fl_BlockLayout * pPrev = getPrevBlockInDocument();
	while(pPrev != NULL && !pPrev->isListItem())
	{
		pPrev = pPrev->getPrevBlockInDocument();
	}
	return pPrev;
}

/*!
 * Returns the most recent Block containing a list item of the closest match
 * of left-margin to this one.
 \returns fl_BlockLayout *
*/
fl_BlockLayout * fl_BlockLayout::getPreviousListOfSameMargin(void) const
{

    const char * szAlign;
	if(m_iDomDirection == UT_BIDI_RTL)
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
	fl_BlockLayout * pPrev = getPrevBlockInDocument();
	while(pPrev != NULL && !bFound)
	{
		if(pPrev->isListItem())
		{
			if(m_iDomDirection == UT_BIDI_RTL)
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
				pPrev = pPrev->getPrevBlockInDocument();
			}
		}
		else
		{
			pPrev = pPrev->getPrevBlockInDocument();
		}
	}
	return pClosest;
}

fl_BlockLayout * fl_BlockLayout::getParentItem(void) const
{
	// TODO Again, more firendly.
	UT_return_val_if_fail(m_pAutoNum, NULL);

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
	UT_return_if_fail(nextList);
	PP_PropertyVector attribs, props;

	nextList->getListPropertyVector(props);
	nextList->getListAttributesVector(attribs);
	m_bStartList =	false;
	m_bStopList = false;
	UT_ASSERT(getView());
	m_bListLabelCreated = false;

	m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), attribs, props, PTX_Block);

	m_bListItem = true;
	m_pDoc->listUpdate(getStruxDocHandle());
}

void  fl_BlockLayout::resumeList( fl_BlockLayout * prevList)
{
	//
	// Make the current block the next element of the list in the block prevList
	//
	UT_return_if_fail(prevList);
	PP_PropertyVector attribs, props;
//
// Defensive code. This should not happen
//
	UT_ASSERT(prevList->getAutoNum());
	if(prevList->getAutoNum() == NULL)
		return;
	prevList->getListPropertyVector(props);
	prevList->getListAttributesVector(attribs);
	m_bStartList =	false;
	m_bStopList = false;
	UT_ASSERT(getView());
	m_bListLabelCreated = false;

	m_pDoc->changeStruxFmt(PTC_AddFmt, getPosition(), getPosition(), attribs, props, PTX_Block);

	m_bListItem = true;
	m_pDoc->listUpdate(getStruxDocHandle());
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
	//
	// Need to recalculate the line location.
	//
	m_bForceSectionBreak = true;
	format();

}

void fl_BlockLayout::transferListFlags(void)
{
	//
	// Transfer list flags from a block to the following list blocks
	//
	UT_return_if_fail(getNext());

	if(getNext()->getContainerType() != FL_CONTAINER_BLOCK)
	{
		return;
	}
	if (getNextBlockInDocument()->isListItem()) // this is wrong. It should be next in the list.
	{
		UT_uint32 nId = getNext()->getAutoNum()->getID();
		UT_uint32 cId=0, pId=0;
		fl_BlockLayout * pPrev = getPreviousList();
		if(pPrev && pPrev->getAutoNum() == NULL)
		{
			return;
		}
		if(pPrev != NULL)
			pId = pPrev->getAutoNum()->getID();
		if(isListItem())
			cId = getAutoNum()->getID();
		if(cId == nId)
		{
			if (!getNextBlockInDocument()->m_bStartList)
				getNextBlockInDocument()->m_bStartList = m_bStartList;
			if (!getNextBlockInDocument()->m_bStopList)
				getNextBlockInDocument()->m_bStopList = m_bStopList;
		}
		else if ( pId == nId)
		{
			if (!getNextBlockInDocument()->m_bStartList)
				getNextBlockInDocument()->m_bStartList = pPrev->m_bStartList;
			if (!getNextBlockInDocument()->m_bStopList)
				getNextBlockInDocument()->m_bStopList = pPrev->m_bStopList;
		}
	}
}

bool  fl_BlockLayout::isListLabelInBlock( void) const
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
		pRun = pRun->getNextRun();
	}
	return bListLabel;
}

bool fl_BlockLayout::isFirstInList(void) const
{
	pf_Frag_Strux* sdh = fl_Layout::getStruxDocHandle();
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
	PD_Document * pDoc = m_pLayout->getDocument();
	//
	// Let remote document create the list label
	//
	if(!pDoc->isOrigUUID())
	{
			return;
	}
	UT_ASSERT(m_pAutoNum);
	xxx_UT_DEBUGMSG(("Doing create list label \n"));
	FV_View* pView = getView();
	PT_DocPosition offset =0;
	if(pView)
	{
		offset = pView->getPoint() - getPosition();
	}
#if 1
	PP_PropertyVector blockatt;
	bool bHaveBlockAtt = pView->getCharFormat(blockatt, true, getPosition());
#endif
#if 1

	UT_return_if_fail(m_pDoc);
	UT_uint32 itag = m_pDoc->getUID(UT_UniqueId::List);

	const PP_PropertyVector tagatt = {
		"list-tag",	UT_std_string_sprintf("%d", itag)
	};
	m_pDoc->changeSpanFmt(PTC_AddFmt, getPosition(), getPosition(), PP_NOPROPS, tagatt);
#endif

	const PP_PropertyVector attributes = {
		"type",	"list_label"
	};

	UT_DebugOnly<bool> bResult = m_pDoc->insertObject(getPosition(), PTO_Field, attributes, PP_NOPROPS);
	UT_ASSERT(bResult);
	PT_DocPosition diff = 1;
	if(m_pDoc->isDoingPaste() == false)
	{
		UT_UCSChar c = UCS_TAB;
		const PP_AttrProp * pSpanAP = NULL;
		getSpanAP(1, false, pSpanAP);
		bResult = m_pDoc->insertSpan(getPosition()+1,&c,1,const_cast< PP_AttrProp *>(pSpanAP));
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

	// FV_View::getCharFmt() can sometimes return a static temporary 
	if(bHaveBlockAtt)
	{
		m_pDoc->changeSpanFmt(PTC_AddFmt, getPosition(), getPosition() + diff, PP_NOPROPS, blockatt);
	}


	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->_setPoint(pView->getPoint()+offset);
		pView->updateCarets(0,offset);
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
	//
	// Let remote document create the list label
	//
	if(!pDoc->isOrigUUID())
	{
			return;
	}
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
		pRun = pRun->getNextRun();
		if(pRun == NULL)
		{
			bStop = true;
		}
	}
	if(pRun != NULL)
	{
		UT_uint32 ioffset = pRun->getBlockOffset();
		UT_uint32 npos = 1;
		fp_Run * tRun = pRun->getNextRun();
		if(tRun != NULL && tRun->getType()==FPRUN_TAB)
		{
			npos = 2;
		}

		UT_uint32 iRealDeleteCount;
		pDoc->deleteSpan(posBlock+ioffset, posBlock+ioffset + npos,NULL,iRealDeleteCount);
	}
}

UT_UCSChar * fl_BlockLayout::getListLabel(void) const
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

	UT_return_if_fail(prevBlockInList);

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
	UT_return_if_fail(nextBlockInList);
	m_pAutoNum = nextBlockInList->getAutoNum();
	m_pAutoNum->prependItem(getStruxDocHandle(), nextBlockInList->getStruxDocHandle());
}

UT_uint32 fl_BlockLayout::getLevel(void) const
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

/*!
 * This Method searches for the next piece of of the block that could
 * be used for texttotable conversions.
\returns true if a valid piece of text was found and there is more, false otherwise
\param buf reference to a growbug containing the text in the block
\param startPos - start search from this position
\param begPos - first character of the word
\param endPos - Last character of the word
\param sWord - UTF8 string containing the word
\param delim: use tab (0), comma (1), space (2) or all (>2) as delimiters
*/
bool fl_BlockLayout::getNextTableElement(UT_GrowBuf * buf,
										 PT_DocPosition startPos, 
										 PT_DocPosition & begPos,
										 PT_DocPosition & endPos,
										 UT_UTF8String & sWord,
										 UT_uint32 iDelim) const
{
	UT_uint32 offset = startPos - getPosition(false);
	UT_uint32 i = 0;
	UT_UCS4Char curChar = 0;
	if(offset >= buf->getLength())
	{
		begPos = 0;
		endPos = 0;
		return false;
	}
	UT_uint32 iMax = buf->getLength() - offset;
	bool bFoundFootnote = false;
	//
	// skip initial spaces
	for(i= 0; i < iMax; i++)
	{
		curChar = static_cast<UT_UCS4Char>(*buf->getPointer(offset+i));
		xxx_UT_DEBUGMSG(("Pre CurChar %c pos %d \n",curChar,offset+i+begPos));
		if(curChar == 7)
		{
			break; // don't split on fields
		}
		//
		// Don't split on numbers
		//
		if(curChar >= static_cast<UT_uint32>('0') && curChar <= static_cast<UT_uint32>('9'))
	    {
			break;
		}
		if(!(curChar == UCS_SPACE))
		{
			break;
		}
	}
	if( i == iMax)
	{
		begPos = 0;
		endPos = 0;
		return false;
	}
	begPos = getPosition(false) + offset + i;
	for(; i< iMax; i++)
	{
		curChar = static_cast<UT_UCS4Char>(*buf->getPointer(offset+i));
		xxx_UT_DEBUGMSG(("CurChar %c pos %d \n",curChar,offset+i+begPos));
		if(curChar == 0)
		{
			PT_DocPosition pos = offset+i+begPos;
			if(m_pDoc->isFootnoteAtPos(pos))
			{
				bFoundFootnote = true;
				continue;
			}
			if(m_pDoc->isEndFootnoteAtPos(pos))
			{
				bFoundFootnote = false;
				continue;
			}
		}
		if(bFoundFootnote)
		{
			continue;
		}
		sWord += curChar;
		if(curChar == 7)
		{
			continue; // don't split on fields
		}
		//
		// Don't split on numbers
		//
		if(curChar >= static_cast<UT_uint32>('0') && curChar <= static_cast<UT_uint32>('9'))
	    {
			continue;
		}
		if(UT_isWordDelimiter(curChar,UCS_UNKPUNK,UCS_UNKPUNK))
		{
			if(((iDelim == 0) && (curChar == UCS_TAB)) ||
			   ((iDelim == 1) && (curChar == ',')) ||
			   ((iDelim == 2) && (curChar == UCS_SPACE)) ||
			   ((iDelim >  2) && (curChar==',' || curChar== UCS_TAB || curChar== UCS_SPACE)))
			{
				break;
			}
		}
	}
	if(i< iMax)
	{
		endPos = getPosition(false) + offset + i;
	}
	else
	{
		endPos = getPosition(false) + offset + i;
	}
	xxx_UT_DEBUGMSG(("Split at %d \n",endPos));
	return true;
}

void fl_BlockLayout::setDominantDirection(UT_BidiCharType iDirection)
{
	m_iDomDirection = iDirection;

	PP_PropertyVector prop = {
		"dom-dir",
		(m_iDomDirection == UT_BIDI_RTL) ? "rtl" : "ltr"
	};

	PT_DocPosition offset = getPosition();
	PT_DocPosition offset2 = offset;
	//NB The casts in the following call are really necessary, it refuses to compile without them. #TF
	getDocument()->changeStruxFmt(static_cast<PTChangeFmt>(PTC_AddFmt),
								  offset, offset2,
								  PP_NOPROPS, prop,
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
		m_pSpellSquiggles->add(pPOB);
		m_pSpellSquiggles->clear(pPOB);

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
		
		pRun = pRun->getNextRun();
	}

	return pRunResult;
}

bool fl_BlockLayout::_canContainPoint() const
{
	return isContainedByTOC() == false;
}

/*!
    this function decides if character c is a word-delimiter, taking on board revisions
    markup
 */

bool fl_BlockLayout::isWordDelimiter(UT_UCS4Char c, UT_UCS4Char next, UT_UCS4Char prev, UT_uint32 iBlockPos) const
{
	if(c == 0)
		return true;
	if(!UT_isWordDelimiter(c, next, prev))
		return false;
	// see if this character has not been deleted in revisions mode ...
	fp_Run * pRun = findRunAtOffset(iBlockPos);

	if(pRun== NULL && (next == 0))
	{
		return true;
	}
	if(pRun == NULL)
	{
		xxx_UT_DEBUGMSG(("No run where one is expected block %x iBlockPos %d \n",this,iBlockPos));
		return false;
	}
	//	UT_return_val_if_fail( pRun, false );

	// ignore hidden runs
	if(pRun->getVisibility() != FP_VISIBLE)
		return false;
	
	if(!pRun->containsRevisions())
		return true;

	if(pRun->getRevisions()->getLastRevision()->getType() == PP_REVISION_DELETION)
		return false;
	
	return true;
}

bool fl_BlockLayout::isSentenceSeparator(UT_UCS4Char c, UT_uint32 iBlockPos) const
{
	if(!UT_UCS4_isSentenceSeparator(c))
		return false;

	// see if this character has not been deleted in revisions mode ...
	fp_Run * pRun = findRunAtOffset(iBlockPos);
	UT_return_val_if_fail( pRun, false );

	// ignore hidden runs
	if(pRun->getVisibility() != FP_VISIBLE)
		return false;
	
	if(!pRun->containsRevisions())
		return true;

	if(pRun->getRevisions()->getLastRevision()->getType() == PP_REVISION_DELETION)
		return false;
	
	return true;
}




#ifdef ENABLE_SPELL
void fl_BlockLayout::enqueueToSpellCheckAfter(fl_BlockLayout *prev)
{
	if (prev != NULL) {
		m_nextToSpell = prev->m_nextToSpell;
		prev->m_nextToSpell = this;
	}
	else {
		m_nextToSpell = m_pLayout->spellQueueHead();
		m_pLayout->setSpellQueueHead(this);
	}
	if (m_nextToSpell != NULL) {
		m_nextToSpell->m_prevToSpell = this;
	}
	else {
		m_pLayout->setSpellQueueTail(this);
	}
	m_prevToSpell = prev;
}


void fl_BlockLayout::dequeueFromSpellCheck(void)
{
	if (m_prevToSpell != NULL) {
		m_prevToSpell->m_nextToSpell = m_nextToSpell;
	}
	else if(m_pLayout->spellQueueHead() == this) {
		m_pLayout->setSpellQueueHead(m_nextToSpell);
	}
	if (m_nextToSpell != NULL) {
		m_nextToSpell->m_prevToSpell = m_prevToSpell;
	}
	else if (m_pLayout->spellQueueTail() == this) {
		m_pLayout->setSpellQueueTail(m_prevToSpell);
	}
	m_nextToSpell = m_prevToSpell = NULL;
}

/*!
  Constructor for iterator
  
  Use the iterator to find words for spell-checking in the block.

  \param pBL BlockLayout this iterator should work on
  \param iPos Position the iterator should start from
*/
fl_BlockSpellIterator::fl_BlockSpellIterator(const fl_BlockLayout* pBL, UT_sint32 iPos)
	: m_pBL(pBL), m_iWordOffset(iPos), m_iStartIndex(iPos), m_iPrevStartIndex(iPos),
	  m_pMutatedString(NULL),
	  m_iSentenceStart(0), m_iSentenceEnd(0)
{
	m_pgb = new UT_GrowBuf(1024);
	bool bRes = pBL->getBlockBuf(m_pgb);
	UT_UNUSED(bRes);
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
fl_BlockSpellIterator::getBlockLength(void) const
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
	UT_UNUSED(bRes);
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

typedef struct 
{
	UT_uint32 iStart;
	UT_uint32 iEnd;
	bool      bIgnore;
} _spell_type;


/*!
    pWord -- pointer to next word
    iLength -- length of the word in the pWord buffer
    iBlockPos -- block offset of the word
    iPTLenth -- the lenth of the word in the Piece Table (can be > iLength)
*/
bool
fl_BlockSpellIterator::nextWordForSpellChecking(const UT_UCSChar*& pWord, UT_sint32& iLength,
												UT_sint32& iBlockPos, UT_sint32& iPTLength)
{
	// For empty blocks, there will be no buffer
	if (NULL == m_pText) return false;
	UT_return_val_if_fail( m_pBL, false );

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
			if (!m_pBL->isWordDelimiter( m_pText[m_iWordOffset], followChar, UCS_UNKPUNK, m_iWordOffset))
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
				if (!m_pBL->isWordDelimiter( m_pText[m_iWordOffset], 
											 m_pText[m_iWordOffset+1],
											 m_pText[m_iWordOffset-1],
											 m_iWordOffset))
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
			if (m_pBL->isWordDelimiter( m_pText[iWordEnd], 
										m_pText[iWordEnd+1],
										m_pText[iWordEnd-1],
										iWordEnd))
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
			
			if (m_pBL->isWordDelimiter(m_pText[iWordEnd], 
										  UCS_UNKPUNK,
										  m_pText[iWordEnd-1],
										  iWordEnd))
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
		UT_sint32 iWordLength = static_cast<UT_sint32>(iWordEnd) - static_cast<UT_sint32>(m_iWordOffset);

		// ignore some terminal characters
		UT_sint32 tempIDX = static_cast<UT_sint32>(m_iWordOffset) + iWordLength - 1;
		UT_ASSERT(tempIDX >= 0);
		if (_ignoreLastWordCharacter(m_pText[tempIDX]))
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
		iPTLength = iWordLength;
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

		// handle revisions and hidden text correctly
		// hidden text is to be ignored (i.e., hidden from the spellcheker)
		// delete revisions that are visible are also to be ignored
		fp_Run * pRun2 = m_pBL->findRunAtOffset(m_iWordOffset);
		if(pRun2 == NULL)
		{
			xxx_UT_DEBUGMSG(("No run where one is expected block %x WordOffset %d \n",this,m_iWordOffset));
			return false;
		}
		UT_return_val_if_fail( pRun2, false );
		bool bRevised = false;

		while(pRun2 && (UT_sint32)pRun2->getBlockOffset() < m_iWordOffset + iWordLength)
		{
			if(pRun2->getVisibility() != FP_VISIBLE ||
			   (pRun2->containsRevisions() && pRun2->getRevisions()->getLastRevision()->getType() == PP_REVISION_DELETION))
			{
				bRevised = true;
				break;
			}

			pRun2 = pRun2->getNextRun();
		}
		
		if (bNeedsMutation || bRevised)
		{
			// Generate the mutated word in a new buffer pointed to by m_pMutatedString
			m_pMutatedString = static_cast<UT_UCSChar*>(UT_calloc(iWordLength, sizeof(UT_UCSChar)));
			UT_ASSERT(m_pMutatedString);
			pWord = m_pMutatedString;
			iNewLength = 0;

			if(bNeedsMutation && !bRevised)
			{
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
			else if(bRevised)
			{
				// we need to deal with revision
				// if the word is contained in multiple runs and some of these are deleted through
				// revisions and visible, the revised text should be disregarded
				UT_GenericVector<_spell_type *> vWordLimits;
				fp_Run * pRun = m_pBL->findRunAtOffset(m_iWordOffset);

				while(pRun && pRun->getBlockOffset() < (UT_uint32)(m_iWordOffset + iWordLength))
				{
					if(pRun->getLength() == 0)
					{
						pRun = pRun->getNextRun();
						continue;
					}
					
					UT_uint32 iMaxLen = UT_MIN(pRun->getLength(), m_iWordOffset + iWordLength - pRun->getBlockOffset());
						
					bool bDeletedVisible =
						pRun->getVisibility() == FP_VISIBLE &&
						pRun->containsRevisions() &&
						pRun->getRevisions()->getLastRevision()->getType() == PP_REVISION_DELETION;

					bool bNotVisible = pRun->getVisibility() != FP_VISIBLE;
					bool bIgnore = bNotVisible || bDeletedVisible;
			
					_spell_type * st2 = NULL;
			
					if(vWordLimits.getItemCount())
						st2 = vWordLimits.getLastItem();
			
					if(st2 && st2->bIgnore == bIgnore)
					{
						// this run continues the last ignore section, just adjust to the end
						st2->iEnd += iMaxLen;
					}
					else
					{
						_spell_type * st = new _spell_type;
						UT_return_val_if_fail( st, false );

						st->bIgnore = bIgnore;
						st->iStart = pRun->getBlockOffset() - m_iWordOffset;
						st->iEnd = st->iStart + iMaxLen;

						vWordLimits.addItem(st);
					}

					pRun = pRun->getNextRun();
				}

				UT_UCS4Char * p = m_pMutatedString;
		
				for(UT_sint32 i = 0; i < vWordLimits.getItemCount(); ++i)
				{
					_spell_type * st = vWordLimits.getNthItem(i);
					UT_return_val_if_fail( st, false );

					if(!st->bIgnore)
					{
						for(UT_uint32 j = st->iStart; j < st->iEnd; ++j)
						{
							if(m_iWordOffset + iWordLength == (UT_sint32)j)
							{
								// we are done (past the last char of the word)
								break;
							}
							
							UT_UCS4Char c = m_pText[m_iWordOffset + j];
							
							// Remove UCS_ABI_OBJECT from the word
							if (c == UCS_ABI_OBJECT)
								continue;

							// Convert smart quote apostrophe to ASCII single quote to
							// be compatible with ispell
							if (c == UCS_RQUOTE)
								c = '\'';

							*p++ = c;
							iNewLength++;
						}
					}
				}
		
				UT_VECTOR_PURGEALL(_spell_type*, vWordLimits);
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
	UT_return_if_fail( m_pBL );
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
		if (m_pBL->isSentenceSeparator(m_pText[m_iSentenceStart], m_iSentenceStart))
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
			   && m_pBL->isWordDelimiter(m_pText[m_iSentenceStart], 
											m_pText[m_iSentenceStart+1],
											m_pText[m_iSentenceStart-1],
											m_iSentenceStart))
		{
			// Nothing to do... just iterating...
		};

	}


	// Find end of sentence. Go forward until a period is found. If
	// getting to within 10 characters of the end of the block, stop
	// and go with that as the end....
	m_iSentenceEnd = m_iWordOffset + m_iWordLength;
	while (m_iSentenceEnd < (iBlockLength - 10)) {
		if (m_pBL->isSentenceSeparator(m_pText[m_iSentenceEnd], m_iSentenceEnd))
		{
			break;
		}
		m_iSentenceEnd++;
	}
	if (m_iSentenceEnd == (iBlockLength-10)) m_iSentenceEnd = iBlockLength-1;
}

/*!
  Get current word
  \result iLength Length of string.
  \return Pointer to word.
*/
const UT_UCSChar*
fl_BlockSpellIterator::getCurrentWord(UT_sint32& iLength) const
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
fl_BlockSpellIterator::getPreWord(UT_sint32& iLength) const
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
fl_BlockSpellIterator::getPostWord(UT_sint32& iLength) const
{
	iLength = m_iSentenceEnd - m_iStartIndex + 1;

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
#endif /* without spell */


static void s_border_properties (const char * border_color, 
								 const char * border_style, 
								 const char * border_width,
								 const char * color, 
								 const char * spacing, PP_PropertyMap::Line & line)
{
	/* cell-border properties:
	 * 
	 * (1) color      - defaults to value of "color" property
	 * (2) line-style - defaults to solid (in contrast to "none" in CSS)
	 * (3) thickness  - defaults to 1 layout unit (??, vs "medium" in CSS)
	 */
	line.reset ();

	PP_PropertyMap::TypeColor t_border_color = PP_PropertyMap::color_type (border_color);
	if (t_border_color)
	{
		line.m_t_color = t_border_color;
		if (t_border_color == PP_PropertyMap::color_color)
			UT_parseColor (border_color, line.m_color);
	}
	else if (color)
	{
		PP_PropertyMap::TypeColor t_color = PP_PropertyMap::color_type (color);

		line.m_t_color = t_color;
		if (t_color == PP_PropertyMap::color_color)
			UT_parseColor (color, line.m_color);
	}

	line.m_t_linestyle = PP_PropertyMap::linestyle_type (border_style);
	if (!line.m_t_linestyle)
		line.m_t_linestyle = PP_PropertyMap::linestyle_none;

	line.m_t_thickness = PP_PropertyMap::thickness_type (border_width);
	if (line.m_t_thickness == PP_PropertyMap::thickness_length)
	{
		if (UT_determineDimension (border_width, (UT_Dimension)-1) == DIM_PX)
		{
			double thickness = UT_LAYOUT_RESOLUTION * UT_convertDimensionless (border_width);
			line.m_thickness = static_cast<UT_sint32>(thickness / UT_PAPER_UNITS_PER_INCH);
		}
		else
			line.m_thickness = UT_convertToLogicalUnits (border_width);

		if (!line.m_thickness)
		{
			// default to 0.72pt
			double thickness = UT_LAYOUT_RESOLUTION;
			line.m_thickness = static_cast<UT_sint32>(thickness / UT_PAPER_UNITS_PER_INCH);
		}
	}
	else // ??
	{
		// default to 0.72pt
		double thickness = UT_LAYOUT_RESOLUTION;
		line.m_thickness = static_cast<UT_sint32>(thickness / UT_PAPER_UNITS_PER_INCH);
	}
	if(spacing)
	{
		line.m_spacing = UT_convertToLogicalUnits(spacing);
	}
	else
	{
		line.m_spacing = UT_convertToLogicalUnits("0.02in");
	}
}
