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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <string.h>
#include <stdlib.h>

#include "ut_types.h"
#include "ut_string.h"

#include "ap_Prefs.h"
#include "fl_TableLayout.h"
#include "fl_SectionLayout.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fb_LineBreaker.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fp_Column.h"
#include "fp_TableContainer.h"
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
#include "fv_View.h"
#include "fp_Run.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_units.h"

fl_TableLayout::fl_TableLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP, fl_ContainerLayout * pMyContainerLayout)
	: fl_SectionLayout(pLayout, sdh, indexAP, FL_SECTION_TABLE,FL_CONTAINER_TABLE,PTX_SectionTable, pMyContainerLayout),
	  m_bNeedsFormat(true),
	  m_bNeedsRebuild(false),
      m_iJustification(FL_TABLE_FULL),
	  m_iLeftOffset(0),
	  m_iLeftOffsetLayoutUnits(0),
	  m_dLeftOffsetUserUnits(0.0),
	  m_iRightOffset(0),
	  m_iRightOffsetLayoutUnits(0),
	  m_dRightOffsetUserUnits(0.0),
	  m_iTopOffset(0),
	  m_iTopOffsetLayoutUnits(0),
	  m_dTopOffsetUserUnits(0.0),
	  m_iBottomOffset(0),
	  m_iBottomOffsetLayoutUnits(0),
	  m_dBottomOffsetUserUnits(0.0),
	  m_bIsHomogeneous(true),
	  m_bSameRowOnTopOfPage(false),
	  m_iRowNumberForTop(0),
	  m_iNumberOfRows(0),
	  m_iNumberOfColumns(0),
	  m_bColumnsPositionedOnPage(false),
	  m_bRowsPositionedOnPage(false),
	  m_bIsDirty(true),
	  m_iLineType(0),
	  m_iLineThickness(0),
	  m_iColSpacing(0),
	  m_iRowSpacing(0),
	  m_iLeftColPos(0)
{
	m_vecColProps.clear();
	m_vecRowProps.clear();
	createTableContainer();
}

fl_TableLayout::~fl_TableLayout()
{
	// NB: be careful about the order of these
	_purgeLayout();
	fp_TableContainer * pTC = (fp_TableContainer *) getFirstContainer();
	if (pTC)
	{
		delete pTC;
	}
	setFirstContainer(NULL);
	setLastContainer(NULL);
	UT_VECTOR_PURGEALL(fl_ColProps *, m_vecColProps);
	UT_VECTOR_PURGEALL(fl_RowProps *, m_vecRowProps);
}

/*!
 * Only one Master Table container per Table Layout. Create it here.
 */
void fl_TableLayout::createTableContainer(void)
{
	_lookupProperties();
	fp_TableContainer * pTableContainer = new fp_TableContainer((fl_SectionLayout *) this);
	setFirstContainer(pTableContainer);
	setLastContainer(pTableContainer);
	setTableContainerProperties(pTableContainer);
	fl_ContainerLayout * pCL = myContainingLayout();
	fp_Container * pCon = pCL->getLastContainer();
	UT_ASSERT(pCon);
	UT_sint32 iWidth = pCon->getWidth();
	UT_sint32 iWidthLayout = pCon->getWidthInLayoutUnits();
	if(iWidth == 0)
	{
		iWidth = pCon->getPage()->getWidth();
		iWidthLayout = pCon->getPage()->getWidthInLayoutUnits();
		pCon->setWidth(iWidth);
		pCon->setWidthInLayoutUnits(iWidthLayout);
	}
	pTableContainer->setWidth(iWidth);
	pTableContainer->setWidthInLayoutUnits(iWidthLayout);
//
// The container of the tbale is set in getNewContainer()
//
}

/*!
 * This method sets all the parameters of the table container from
 * properties of this section.
 */
void fl_TableLayout::setTableContainerProperties(fp_TableContainer * pTab)
{
	pTab->setHomogeneous(m_bIsHomogeneous);
	UT_sint32 borderWidth = m_iLeftOffsetLayoutUnits + m_iRightOffsetLayoutUnits;
	pTab->setBorderWidth(borderWidth);
	pTab->setColSpacings(m_iColSpacing);
	pTab->setRowSpacings(m_iRowSpacing);
}


/*!
  Create a new Table container and plug it into the linked list of Table 
  containers.
  \params If pPrevTab is non-null place the new cell after this in the linked
          list, otherwise just append it to the end.
  \return The newly created Table container
*/
fp_Container* fl_TableLayout::getNewContainer(fp_Container * pPrevTab)
{
	UT_ASSERT(getFirstContainer() == NULL);
	createTableContainer();
	fp_TableContainer * pNewTab = (fp_TableContainer *) getFirstContainer();
//
// Master Tables do not get linked into the container linked list.
//
	pNewTab->setPrev(NULL);
	pNewTab->setNext(NULL);
//
// Now find the right place to put our new Table container within the arrangement
// of it's own container.
//
	insertTableContainer(pNewTab);
	return (fp_Container *) pNewTab;
}

/*!
 * This method inserts the given TableContainer into its correct place in the
 * Vertical container.
 */
void fl_TableLayout::insertTableContainer( fp_TableContainer * pNewTab)
{
	fl_ContainerLayout * pUPCL = myContainingLayout();
	fl_ContainerLayout * pPrevL = (fl_ContainerLayout *) getPrev();
	fp_Container * pPrevCon = NULL;
	fp_Container * pUpCon = NULL;
	if(pPrevL != NULL)
	{
		pPrevCon = pPrevL->getLastContainer();
		pUpCon = pPrevCon->getContainer();
	}
	else
	{
		pUpCon = pUPCL->getLastContainer();
	}
	if(pPrevL == NULL)
	{
		UT_DEBUGMSG(("SEVIOR!!!!!!!!!! New Table %x added into %x \n",pNewTab,pUpCon));
		pUpCon->addCon(pNewTab);
		pNewTab->setContainer(pUpCon);
;
	}
	else
	{
		UT_sint32 i = pUpCon->findCon(pPrevCon);
		UT_DEBUGMSG(("SEVIOR!!!!!!!!!! New Table %x inserted into %x \n",pNewTab,pUpCon));
		if(i >= 0 && (i+1) < (UT_sint32) pUpCon->countCons())
		{
			pUpCon->insertConAt(pNewTab,i+1);
			pNewTab->setContainer(pUpCon);
		}
		else if( i >=0 &&  (i+ 1) == (UT_sint32) pUpCon->countCons())
		{
			pUpCon->addCon(pNewTab);
			pNewTab->setContainer(pUpCon);
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}
}

void fl_TableLayout::format(void)
{
	bool bRebuild = false;
	if(getFirstContainer() == NULL)
	{
		getNewContainer(NULL);
		bRebuild = true;
	}
	if(isDirty())
	{
		markAllRunsDirty();
	}
	fl_ContainerLayout*	pCell = getFirstLayout();
	while (pCell)
	{
		pCell->format();
		if(bRebuild)
		{
			attachCell(pCell);
		}
		pCell = pCell->getNext();
	}
	UT_DEBUGMSG(("SEVIOR: Finished Formatting %x \n",this));

	if(isDirty() && !getDocument()->isDontImmediateLayout())
	{
		m_bIsDirty = false;
		UT_DEBUGMSG(("SEVIOR: Layout pass 1 \n"));
		static_cast<fp_TableContainer *>(getFirstContainer())->layout();
		setNeedsRedraw();
		markAllRunsDirty();
	}
//
// The layout process can trigger a width change on a cell which requires
// a second layout pass
//
	if(isDirty() && !getDocument()->isDontImmediateLayout())
	{
		static_cast<fp_TableContainer *>(getFirstContainer())->layout();
		UT_DEBUGMSG(("SEVIOR: Layout pass 2 \n"));
		setNeedsRedraw();
		markAllRunsDirty();
	}
	m_bNeedsFormat = false;
	m_bIsDirty = false;
}

void fl_TableLayout::markAllRunsDirty(void)
{
	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL)
	{
		pCL->markAllRunsDirty();
		pCL = pCL->getNext();
	}
}

void fl_TableLayout::updateLayout(void)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		if (pBL->needsReformat())
		{
			pBL->updateLayout();
		}

		pBL = pBL->getNext();
	}
}

void fl_TableLayout::redrawUpdate(void)
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
	fp_TableContainer * pTab = (fp_TableContainer *) getFirstContainer();
	if(pTab->doRedrawLines())
	{
		pTab->drawLines();
	}
}

bool fl_TableLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);


	setAttrPropIndex(pcrxc->getIndexAP());
	collapse();
	updateTable();
	return true;
}

fl_SectionLayout * fl_TableLayout::getSectionLayout(void) const
{
	fl_ContainerLayout * pDSL = myContainingLayout();
	while(pDSL)
	{
		if(pDSL->getContainerType() == FL_CONTAINER_DOCSECTION)
		{
			return (fl_SectionLayout *) pDSL;
		}
		pDSL = pDSL->myContainingLayout();
	}
	return NULL;
}
void fl_TableLayout::updateTable(void)
{

	const PP_AttrProp* pAP = NULL;
	bool bres = m_pDoc->getAttrProp(m_apIndex, &pAP);
	UT_ASSERT(bres);

	_lookupProperties();

	// clear all the columns
    // Assume that all formatting have already been removed via a 
    // collapse()
    //

	/*
	  TODO to more closely mirror the architecture we're using for BlockLayout, this code
	  should probably just set a flag, indicating the need to reformat this section.  Then,
	  when it's time to update everything, we'll actually do the format.
	*/

	FV_View * pView = m_pLayout->getView();
	if(pView)
	{
		pView->setScreenUpdateOnGeneralUpdate(false);
	}

	format();
	markAllRunsDirty();

	if(pView)
	{
		pView->setScreenUpdateOnGeneralUpdate(true);
	}

	return;
}


bool fl_TableLayout::bl_doclistener_insertCell(fl_ContainerLayout* pCell,
											  const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew))
{
	fl_ContainerLayout * pNewCL = NULL;
	if(pCell == NULL)
	{
		pNewCL = append(sdh, pcrx->getIndexAP(),FL_CONTAINER_CELL);
	}
	else
	{
		pNewCL = insert(sdh,pCell,pcrx->getIndexAP(), FL_CONTAINER_CELL);
	}
	
		// Must call the bind function to complete the exchange of handles
		// with the document (piece table) *** before *** anything tries
		// to call down into the document (like all of the view
		// listeners).
		
	PL_StruxFmtHandle sfhNew = (PL_StruxFmtHandle)pNewCL;
	pfnBindHandles(sdh,lid,sfhNew);

	fl_CellLayout * pCL = (fl_CellLayout *) pNewCL;
	attachCell(pCL);
//
// increment the insertion point in the view.
//
	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->setPoint(pView->getPoint() +  fl_BLOCK_STRUX_OFFSET);
	}
	return true;
}



bool fl_TableLayout::bl_doclistener_insertEndTable(fl_ContainerLayout*,
												   const PX_ChangeRecord_Strux * pcrx,
												   PL_StruxDocHandle sdh,
												   PL_ListenerId lid,
												   void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew))
{
	// The endTable strux actually has a format handle to to the this table layout.
	// so we bind to this layout.

		
	PL_StruxFmtHandle sfhNew = (PL_StruxFmtHandle) this;
	pfnBindHandles(sdh,lid,sfhNew);

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
	return true;
}

bool fl_TableLayout::recalculateFields(UT_uint32 iUpdateCount)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		pBL->recalculateFields(iUpdateCount);
		pBL = pBL->getNext();
	}
	return true;
}

void fl_TableLayout::_lookupProperties(void)
{
	const PP_AttrProp* pSectionAP = NULL;

	m_pLayout->getDocument()->getAttrProp(m_apIndex, &pSectionAP);

	/*
	  TODO shouldn't we be using PP_evalProperty like
	  the blockLayout does?

	  Yes, since PP_evalProperty does a fallback to the
	  last-chance defaults, whereas the code below is
	  hard-coding its own defaults.  Bad idea.
	*/

	const char* pszHomogeneous = NULL;
	pSectionAP->getProperty("homogeneuos", (const XML_Char *&)pszHomogeneous);
	if (pszHomogeneous && pszHomogeneous[0])
	{
		if(atoi(pszHomogeneous) == 1)
		{
			m_bIsHomogeneous = true;;
		}
	}
	else
	{
		m_bIsHomogeneous = false;
	}
	const char* pszLeftOffset = NULL;
	const char* pszTopOffset = NULL;
	const char* pszRightOffset = NULL;
	const char* pszBottomOffset = NULL;
	pSectionAP->getProperty("table-margin-left", (const XML_Char *&)pszLeftOffset);
	pSectionAP->getProperty("table-margin-top", (const XML_Char *&)pszTopOffset);
	pSectionAP->getProperty("table-margin-right", (const XML_Char *&)pszRightOffset);
	pSectionAP->getProperty("table-margin-bottom", (const XML_Char *&)pszBottomOffset);

	const XML_Char * szRulerUnits;
	UT_Dimension dim;
	if (XAP_App::getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits,&szRulerUnits))
		dim = UT_determineDimension(szRulerUnits);
	else
		dim = DIM_IN;

	UT_String defaultOffset;
	switch(dim)
	{
	case DIM_IN:
		defaultOffset = "0.1in";
		break;

	case DIM_CM:
		defaultOffset = "0.254cm";
		break;

	case DIM_PI:
		defaultOffset = "0.5pi";
		break;

	case DIM_PT:
		defaultOffset= "6.0pt";
		break;

	case DIM_MM:
		defaultOffset= "2.54mm";
		break;

		// TODO: PX, and PERCENT
		// let them fall through to the default now
		// and we don't use them anyway
#if 0
	case DIM_PX:
	case DIM_PERCENT:
#endif
	case DIM_none:
	default:
		defaultOffset = "0.1in";	// TODO: what to do with this.
		break;

	}

	if(pszLeftOffset && pszLeftOffset[0])
	{
		m_iLeftOffset = m_pLayout->getGraphics()->convertDimension(pszLeftOffset);
#ifndef WITH_PANGO
		m_iLeftOffsetLayoutUnits = UT_convertToLayoutUnits(pszLeftOffset);
#endif
		m_dLeftOffsetUserUnits = UT_convertDimensionless(pszLeftOffset);
	}
	else
	{
		m_iLeftOffset = m_pLayout->getGraphics()->convertDimension(defaultOffset.c_str());
#ifndef WITH_PANGO
		m_iLeftOffsetLayoutUnits = UT_convertToLayoutUnits(defaultOffset.c_str());
#endif
		m_dLeftOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}

	if(pszTopOffset && pszTopOffset[0])
	{
		m_iTopOffset = m_pLayout->getGraphics()->convertDimension(pszTopOffset);
#ifndef WITH_PANGO
		m_iTopOffsetLayoutUnits = UT_convertToLayoutUnits(pszTopOffset);
#endif
		m_dTopOffsetUserUnits = UT_convertDimensionless(pszTopOffset);
	}
	else
	{
		m_iTopOffset = m_pLayout->getGraphics()->convertDimension(defaultOffset.c_str());
#ifndef WITH_PANGO
		m_iTopOffsetLayoutUnits = UT_convertToLayoutUnits(defaultOffset.c_str());
#endif
		m_dTopOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}

	if(pszRightOffset && pszRightOffset[0])
	{
		m_iRightOffset = m_pLayout->getGraphics()->convertDimension(pszRightOffset);
#ifndef WITH_PANGO
		m_iRightOffsetLayoutUnits = UT_convertToLayoutUnits(pszRightOffset);
#endif
		m_dRightOffsetUserUnits = UT_convertDimensionless(pszRightOffset);
	}
	else
	{
		m_iRightOffset = m_pLayout->getGraphics()->convertDimension(defaultOffset.c_str());
#ifndef WITH_PANGO
		m_iRightOffsetLayoutUnits = UT_convertToLayoutUnits(defaultOffset.c_str());
#endif
		m_dRightOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}

	if(pszBottomOffset && pszBottomOffset[0])
	{
		m_iBottomOffset = m_pLayout->getGraphics()->convertDimension(pszBottomOffset);
#ifndef WITH_PANGO
		m_iBottomOffsetLayoutUnits = UT_convertToLayoutUnits(pszBottomOffset);
#endif
		m_dBottomOffsetUserUnits = UT_convertDimensionless(pszBottomOffset);
	}
	else
	{
		m_iBottomOffset = m_pLayout->getGraphics()->convertDimension(defaultOffset.c_str());
#ifndef WITH_PANGO
		m_iBottomOffsetLayoutUnits = UT_convertToLayoutUnits(defaultOffset.c_str());
#endif
		m_dBottomOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}
	const char * pszLineType = NULL;
	const char * pszLineThick = NULL;
	pSectionAP->getProperty("table-line-type", (const XML_Char *&)pszLineType);
	pSectionAP->getProperty("table-line-thickness", (const XML_Char *&)pszLineThick);
	if(pszLineThick && *pszLineThick)
	{
		m_iLineThickness = atoi(pszLineThick);
	}
	else
	{
		m_iLineThickness = 1;
	}
	if(pszLineType && *pszLineType)
	{
		UT_DEBUGMSG(("SEVIOR: Line Type string %s \n",pszLineType));
		m_iLineType = atoi(pszLineType);
	}
	else
	{
		m_iLineType = 1;
	}
	UT_DEBUGMSG(("SEVIOR: TableLayout::_lookup linetype %d lineThickness %d \n",m_iLineType,m_iLineThickness));
	const char * pszTableColSpacing = NULL;
	const char * pszTableRowSpacing = NULL;
	pSectionAP->getProperty("table-col-spacing", (const XML_Char *&)pszTableColSpacing);
	pSectionAP->getProperty("table-row-spacing", (const XML_Char *&)pszTableRowSpacing);
	if(pszTableColSpacing && *pszTableColSpacing)
	{
//
// Note to TF from MS. It was too hard to propagate 3 different sets of layout units
// into the Table Layout code so what I did instead was the worst of all worlds.
//
// Structures are layed horizontally in LAYOUT units and vertically in screen units.
// That is what all those SCALE_TO_SCREEN macros are about. We should think through
// how to transition to pango layout.
//
// Anyway column spacing being horizontal is layout units.
//
		m_iColSpacing = UT_convertToLayoutUnits(pszTableColSpacing);
	}
	else
	{
		m_iColSpacing =  UT_convertToLayoutUnits("0.1in");
	}
	if(pszTableRowSpacing && *pszTableRowSpacing)
	{
//
// Row spacing being vertical is screen units
//
		m_iRowSpacing = m_pLayout->getGraphics()->convertDimension(pszTableRowSpacing);
	}
	else
	{
		m_iRowSpacing = m_pLayout->getGraphics()->convertDimension("0.1in");
	}
//
// Positioned columns controls
//
	const char * pszLeftColPos = NULL;
	const char * pszColumnProps = NULL;
	pSectionAP->getProperty("table-column-leftpos", (const XML_Char *&)pszLeftColPos);
	pSectionAP->getProperty("table-column-props", (const XML_Char *&)pszColumnProps);
	if(pszLeftColPos && *pszLeftColPos)
	{
//
// Anyway column positioning being horizontal is layout units.
//
		m_iLeftColPos = UT_convertToLayoutUnits(pszLeftColPos);
	}
	else
	{
		m_iLeftColPos =  0;
	}
	if(pszColumnProps && *pszColumnProps)
	{
/*
   These will be properties applied to all columns. To start with, just the 
    widths of each column are specifed. These are translated to layout units.
 
   The format of the string of properties is:

   table-column-props:1.2in/3.0in/1.3in/;

   So we read back in pszColumnProps
   1.2in/3.0in/1.3in/

   The "/" characters will be used to delineate different column entries.
   As new properties for each column are defined these will be delineated with "_"
   characters. But we'll cross that bridge later.
*/
		UT_DEBUGMSG(("Processing Column width string %s \n",pszColumnProps));
		UT_VECTOR_PURGEALL(fl_ColProps *,m_vecColProps);
		m_vecColProps.clear();
		UT_String sProps = pszColumnProps;
		UT_sint32 sizes = sProps.size();
		UT_sint32 i =0;
		UT_sint32 j =0;
		while(i < sizes)
		{
			for (j=i; (j<sizes) && (sProps[j] != '/') ; j++) {}
			if((j+1)>i && sProps[j] == '/')
			{
				char * pszSub = UT_strdup(sProps.substr(i,(j-i)).c_str());
				i = j + 1;
				fl_ColProps * pColP = new fl_ColProps;
				pColP->m_iColWidth = UT_convertToLayoutUnits(pszSub);
				m_vecColProps.addItem((void *) pColP);
				delete [] pszSub;
			}
		}
 	}
	else
	{
		UT_VECTOR_PURGEALL(fl_ColProps *,m_vecColProps);
		m_vecColProps.clear();
	}
	
}

UT_sint32 fl_TableLayout::getColSpacing(void) const
{
	return m_iColSpacing;
}


UT_sint32 fl_TableLayout::getRowSpacing(void) const
{
	return m_iRowSpacing;
}

UT_sint32 fl_TableLayout::getLineType(void) const
{
	return m_iLineType;
}

UT_sint32 fl_TableLayout::getLineThickness(void) const
{
	return m_iLineThickness;
}

UT_sint32 fl_TableLayout::getTopOffset(void) const
{
	return m_iTopOffset;
}

#ifndef WITH_PANGO
UT_sint32 fl_TableLayout::getTopOffsetInLayoutUnits(void) const
{
	return m_iTopOffsetLayoutUnits;
}
#endif

UT_sint32 fl_TableLayout::getBottomOffset(void) const
{
	return m_iBottomOffset;
}

#ifndef WITH_PANGO
UT_sint32 fl_TableLayout::getBottomOffsetInLayoutUnits(void) const
{
	return m_iBottomOffsetLayoutUnits;
}
#endif

UT_sint32   fl_TableLayout::getLeftOffset(void) const
{
	return m_iLeftOffset;
}

#ifndef WITH_PANGO
UT_sint32  fl_TableLayout::getLeftOffsetInLayoutUnits(void) const
{
	return m_iLeftOffsetLayoutUnits;
}
#endif
UT_sint32   fl_TableLayout::getRightOffset(void) const
{
	return m_iRightOffset;
}
#ifndef WITH_PANGO
UT_sint32   fl_TableLayout::getRightOffsetInLayoutUnits(void) const
{
	return m_iRightOffsetLayoutUnits;
}
#endif


void fl_TableLayout::collapse(void)
{
	// Clear all our Tables
	fp_TableContainer *pTab = (fp_TableContainer *) getFirstContainer();
	if (pTab)
	{
		pTab->clearScreen();
	}

	// get rid of all the layout information for every containerLayout
	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL)
	{
		pCL->collapse();
		pCL = pCL->getNext();
	}

	// delete our Table
	pTab = (fp_TableContainer *) getFirstContainer();
//
// Remove from the container it comes from
//
	fp_VerticalContainer * pUpCon = (fp_VerticalContainer *)pTab->getContainer();
	pUpCon->removeContainer(pTab);
	if (pTab)
	{
		delete pTab;
	}
	setFirstContainer(NULL);
	setLastContainer(NULL);
}

bool fl_TableLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
	UT_ASSERT(pcrx->getStruxType()== PTX_SectionTable);


	fl_ContainerLayout * pPrev = getPrev();
	fl_ContainerLayout * pNext = getNext();

	collapse();

	if(pPrev != NULL)
	{
		pPrev->setNext(pNext);
	}
	else
	{
		myContainingLayout()->setFirstLayout(pNext);
	}
	if(pNext != NULL)
	{
		pNext->setPrev(pPrev);
	}
	else
	{
		myContainingLayout()->setLastLayout(pPrev);
	}

	delete this;			// TODO whoa!  this construct is VERY dangerous.

	return true;
}

/*!
 * Return the position of the table strux. 
 */
PT_DocPosition fl_TableLayout::getPosition(bool bActualBlockPosition /* = false */) const
{
	return fl_ContainerLayout::getPosition(bActualBlockPosition);
}

/*!
 * This method attaches pCell to the current tablecontainer.
 */
void fl_TableLayout::attachCell(fl_ContainerLayout * pCell)
{
	//
	// Verify the cell layout is in the table.
    //
	fl_ContainerLayout * pCur = getFirstLayout();
	while(pCur && pCur !=  pCell)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Looking for %x found %x \n",pCell,pCur));
		pCur = pCur->getNext();
	}
	if(pCur == NULL)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	fp_TableContainer * pTab = (fp_TableContainer *) getLastContainer();
	UT_ASSERT(pTab);
	pTab->tableAttach((fp_CellContainer *) pCell->getLastContainer());
	setDirty();
}

/*!
 * This method removes all layout structures contained by this layout
 * structure.
 */
void fl_TableLayout::_purgeLayout(void)
{
	fl_ContainerLayout * pCL = getFirstLayout();
	while(pCL)
	{
		fl_ContainerLayout * pNext = pCL->getNext();
		delete pCL;
		pCL = pNext;
	}
}

//------------------------------------------------------------------

fl_CellLayout::fl_CellLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP, fl_ContainerLayout * pMyContainerLayout)
	: fl_SectionLayout(pLayout, sdh, indexAP, FL_SECTION_CELL,FL_CONTAINER_CELL,PTX_SectionCell,pMyContainerLayout),
	  m_bNeedsFormat(true),
	  m_bNeedsRebuild(false),
	  m_iLeftOffset(0),
	  m_iLeftOffsetLayoutUnits(0),
	  m_dLeftOffsetUserUnits(0.0),
	  m_iRightOffset(0),
	  m_iRightOffsetLayoutUnits(0),
	  m_dRightOffsetUserUnits(0.0),
	  m_iTopOffset(0),
	  m_iTopOffsetLayoutUnits(0),
	  m_dTopOffsetUserUnits(0.0),
	  m_iBottomOffset(0),
	  m_iBottomOffsetLayoutUnits(0),
	  m_dBottomOffsetUserUnits(0.0),
	  m_iLeftAttach(0),
	  m_iRightAttach(1),
	  m_iTopAttach(0),
	  m_iBotAttach(1),
	  m_bCellPositionedOnPage(false),
	  m_iCellHeight(0)
{
	createCellContainer();
}

fl_CellLayout::~fl_CellLayout()
{
	// NB: be careful about the order of these
	_purgeLayout();
	fp_CellContainer * pTC = (fp_CellContainer *) getFirstContainer();
	while(pTC)
	{
		fp_CellContainer * pNext = (fp_CellContainer *) pTC->getNext();
		if(pTC == (fp_CellContainer *) getLastContainer());
		{
			pNext = NULL;
		}
		delete pTC;
		pTC = pNext;
	}
	setFirstContainer(NULL);
	setLastContainer(NULL);
}

/*!
 * This method creates a new cell with it's properties initially set
 * from the Attributes/properties of this Layout
 */
void fl_CellLayout::createCellContainer(void)
{
	_lookupProperties();
	fp_CellContainer * pCellContainer = new fp_CellContainer((fl_SectionLayout *) this);
	setFirstContainer(pCellContainer);
	setLastContainer(pCellContainer);
	setCellContainerProperties(pCellContainer);
	fl_ContainerLayout * pCL = myContainingLayout();
	while(pCL!= NULL && pCL->getContainerType() != FL_CONTAINER_DOCSECTION)
	{
		pCL = pCL->myContainingLayout();
	}
	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(pCL);
	UT_ASSERT(pDSL != NULL);
	UT_sint32 iWidth = pDSL->getFirstContainer()->getPage()->getWidth();
	pCellContainer->setWidth(iWidth);
	UT_sint32 iWidthLayout = pDSL->getFirstContainer()->getPage()->getWidthInLayoutUnits() - pDSL->getLeftMarginInLayoutUnits() - pDSL->getRightMarginInLayoutUnits();
	UT_DEBUGMSG(("SEVIOR: Setting initial width of cell %x to %d \n",pCellContainer,iWidthLayout));
	pCellContainer->setWidthInLayoutUnits(iWidthLayout);
}


/*!
 * This method sets all the parameters of the cell container from
 * properties of this section 
 */
void fl_CellLayout::setCellContainerProperties(fp_CellContainer * pCell)
{
	pCell->setLeftAttach(m_iLeftAttach);
	pCell->setRightAttach(m_iRightAttach);
	pCell->setTopAttach(m_iTopAttach);
	pCell->setBottomAttach(m_iBotAttach);
	pCell->setLeftPad(m_iLeftOffsetLayoutUnits);
	pCell->setRightPad(m_iRightOffsetLayoutUnits);
	pCell->setTopPad(m_iTopOffset);
	pCell->setBotPad(m_iBottomOffset);
}

/*!
 * This method measures the cell height and compares it to the previously
 * measured height. If they disagree update the layout of the table.
 */
void fl_CellLayout::checkAndAdjustCellSize(void)
{
	fp_CellContainer * pCell = (fp_CellContainer *) getFirstContainer();
	if(pCell == NULL)
	{
		return;
	}
	fp_Requisition Req;
	pCell->sizeRequest(&Req);
	if(Req.height == m_iCellHeight)
	{
		return;
	}
	m_iCellHeight = Req.height;
	static_cast<fl_TableLayout *>(myContainingLayout())->setDirty();
	myContainingLayout()->format();
}
	
bool fl_CellLayout::bl_doclistener_insertCell(fl_ContainerLayout* pCell,
											  const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew))
{
	fl_ContainerLayout * pNewCL = NULL;
	fl_TableLayout * pTL = (fl_TableLayout *) myContainingLayout();
	pNewCL = pTL->insert(sdh,pCell,pcrx->getIndexAP(), FL_CONTAINER_CELL);
	
		// Must call the bind function to complete the exchange of handles
		// with the document (piece table) *** before *** anything tries
		// to call down into the document (like all of the view
		// listeners).
		
	PL_StruxFmtHandle sfhNew = (PL_StruxFmtHandle)pNewCL;
	pfnBindHandles(sdh,lid,sfhNew);

	fl_CellLayout * pCL = (fl_CellLayout *) pNewCL;
	pTL->attachCell(pCL);

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
	return true;
}

	
bool fl_CellLayout::bl_doclistener_insertEndCell(fl_ContainerLayout*,
											  const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew))
{
	// The endCell strux actually needs a format handle to to this cell layout.
	// so we bind to this layout.

		
	PL_StruxFmtHandle sfhNew = (PL_StruxFmtHandle) this;
	pfnBindHandles(sdh,lid,sfhNew);

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
	return true;
}

fl_SectionLayout * fl_CellLayout::getSectionLayout(void) const
{
	fl_ContainerLayout * pDSL = myContainingLayout();
	while(pDSL)
	{
		if(pDSL->getContainerType() == FL_CONTAINER_DOCSECTION)
		{
			return (fl_SectionLayout *) pDSL;
		}
		pDSL = pDSL->myContainingLayout();
	}
	return NULL;
}

/*!
  Create a new Cell containers and plug it into the linked list of Cell 
  containers.
  \params If pPrevCell is non-null place the new cell after this in the linked
          list, otherwise just append it to the end.
  \return The newly created Cell container
*/
fp_Container* fl_CellLayout::getNewContainer(fp_Container * pPrev)
{
//
// One cell container per cell layout
//
	UT_ASSERT(pPrev == NULL);
	UT_ASSERT((getFirstContainer() == NULL) && (getLastContainer()==NULL));
	createCellContainer();
	setCellContainerProperties((fp_CellContainer * ) getLastContainer());
	return (fp_Container *) getLastContainer();
}


void fl_CellLayout::format(void)
{
	xxx_UT_DEBUGMSG(("SEVIOR: Formatting first container is %x \n",getFirstContainer()));
	if(getFirstContainer() == NULL)
	{
		getNewContainer(NULL);
		
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
	static_cast<fp_CellContainer *>(getFirstContainer())->layout();
	m_bNeedsFormat = false;
}

void fl_CellLayout::markAllRunsDirty(void)
{
	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL)
	{
		pCL->markAllRunsDirty();
		pCL = pCL->getNext();
	}
}

void fl_CellLayout::updateLayout(void)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		if (pBL->needsReformat())
		{
			pBL->format();
		}

		pBL = pBL->getNext();
	}
	checkAndAdjustCellSize();
}

void fl_CellLayout::redrawUpdate(void)
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

bool fl_CellLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);


	setAttrPropIndex(pcrxc->getIndexAP());
	collapse();
	updateCell();
	return true;
}


void fl_CellLayout::updateCell(void)
{

	const PP_AttrProp* pAP = NULL;
	bool bres = m_pDoc->getAttrProp(m_apIndex, &pAP);
	UT_ASSERT(bres);

	_lookupProperties();

	// clear all the columns
    // Assume that all formatting have already been removed via a 
    // collapse()
    //

	/*
	  TODO to more closely mirror the architecture we're using for BlockLayout, this code
	  should probably just set a flag, indicating the need to reformat this section.  Then,
	  when it's time to update everything, we'll actually do the format.
	*/

	FV_View * pView = m_pLayout->getView();
	if(pView)
	{
		pView->setScreenUpdateOnGeneralUpdate(false);
	}

	format();
	markAllRunsDirty();

	if(pView)
	{
		pView->setScreenUpdateOnGeneralUpdate(true);
	}

	return;
}

bool fl_CellLayout::recalculateFields(UT_uint32 iUpdateCount)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		pBL->recalculateFields(iUpdateCount);
		pBL = pBL->getNext();
	}
	return true;
}

void fl_CellLayout::_lookupProperties(void)
{
	const PP_AttrProp* pSectionAP = NULL;

	m_pLayout->getDocument()->getAttrProp(m_apIndex, &pSectionAP);
	UT_DEBUGMSG(("SEVIOR: indexAp in Cell Layout %d \n",m_apIndex));
	/*
	  TODO shouldn't we be using PP_evalProperty like
	  the blockLayout does?

	  Yes, since PP_evalProperty does a fallback to the
	  last-chance defaults, whereas the code below is
	  hard-coding its own defaults.  Bad idea.
	*/

	const char* pszLeftOffset = NULL;
	const char* pszTopOffset = NULL;
	const char* pszRightOffset = NULL;
	const char* pszBottomOffset = NULL;
	pSectionAP->getProperty("cell-margin-left", (const XML_Char *&)pszLeftOffset);
	pSectionAP->getProperty("cell-margin-top", (const XML_Char *&)pszTopOffset);
	pSectionAP->getProperty("cell-margin-right", (const XML_Char *&)pszRightOffset);
	pSectionAP->getProperty("cell-margin-bottom", (const XML_Char *&)pszBottomOffset);
	const XML_Char * szRulerUnits;
	UT_Dimension dim;
	if (XAP_App::getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits,&szRulerUnits))
		dim = UT_determineDimension(szRulerUnits);
	else
		dim = DIM_IN;

	UT_String defaultOffset;
	switch(dim)
	{
	case DIM_IN:
		defaultOffset = "0.05in";
		break;

	case DIM_CM:
		defaultOffset = "0.127cm";
		break;

	case DIM_PI:
		defaultOffset = "0.3pi";
		break;

	case DIM_PT:
		defaultOffset= "3.6pt";
		break;

	case DIM_MM:
		defaultOffset= "1.27mm";
		break;

		// TODO: PX, and PERCENT
		// let them fall through to the default now
		// and we don't use them anyway
#if 0
	case DIM_PX:
	case DIM_PERCENT:
#endif
	case DIM_none:
	default:
		defaultOffset = "0.05in";	// TODO: what to do with this.
		break;

	}

	if(pszLeftOffset && pszLeftOffset[0])
	{
		m_iLeftOffset = m_pLayout->getGraphics()->convertDimension(pszLeftOffset);
#ifndef WITH_PANGO
		m_iLeftOffsetLayoutUnits = UT_convertToLayoutUnits(pszLeftOffset);
#endif
		m_dLeftOffsetUserUnits = UT_convertDimensionless(pszLeftOffset);
	}
	else
	{
		m_iLeftOffset = m_pLayout->getGraphics()->convertDimension(defaultOffset.c_str());
#ifndef WITH_PANGO
		m_iLeftOffsetLayoutUnits = UT_convertToLayoutUnits(defaultOffset.c_str());
#endif
		m_dLeftOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}

	if(pszTopOffset && pszTopOffset[0])
	{
		m_iTopOffset = m_pLayout->getGraphics()->convertDimension(pszTopOffset);
#ifndef WITH_PANGO
		m_iTopOffsetLayoutUnits = UT_convertToLayoutUnits(pszTopOffset);
#endif
		m_dTopOffsetUserUnits = UT_convertDimensionless(pszTopOffset);
	}
	else
	{
		m_iTopOffset = m_pLayout->getGraphics()->convertDimension(defaultOffset.c_str());
#ifndef WITH_PANGO
		m_iTopOffsetLayoutUnits = UT_convertToLayoutUnits(defaultOffset.c_str());
#endif
		m_dTopOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}

	if(pszRightOffset && pszRightOffset[0])
	{
		m_iRightOffset = m_pLayout->getGraphics()->convertDimension(pszRightOffset);
#ifndef WITH_PANGO
		m_iRightOffsetLayoutUnits = UT_convertToLayoutUnits(pszRightOffset);
#endif
		m_dRightOffsetUserUnits = UT_convertDimensionless(pszRightOffset);
	}
	else
	{
		m_iRightOffset = m_pLayout->getGraphics()->convertDimension(defaultOffset.c_str());
#ifndef WITH_PANGO
		m_iRightOffsetLayoutUnits = UT_convertToLayoutUnits(defaultOffset.c_str());
#endif
		m_dRightOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}

	if(pszBottomOffset && pszBottomOffset[0])
	{
		m_iBottomOffset = m_pLayout->getGraphics()->convertDimension(pszBottomOffset);
#ifndef WITH_PANGO
		m_iBottomOffsetLayoutUnits = UT_convertToLayoutUnits(pszBottomOffset);
#endif
		m_dBottomOffsetUserUnits = UT_convertDimensionless(pszBottomOffset);
	}
	else
	{
		m_iBottomOffset = m_pLayout->getGraphics()->convertDimension(defaultOffset.c_str());
#ifndef WITH_PANGO
		m_iBottomOffsetLayoutUnits = UT_convertToLayoutUnits(defaultOffset.c_str());
#endif
		m_dBottomOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}
	const char* pszLeftAttach = NULL;
	const char* pszRightAttach = NULL;
	const char* pszTopAttach = NULL;
	const char* pszBotAttach = NULL;
	pSectionAP->getProperty("left-attach", (const XML_Char *&)pszLeftAttach);
	pSectionAP->getProperty("right-attach", (const XML_Char *&)pszRightAttach);
	pSectionAP->getProperty("top-attach", (const XML_Char *&)pszTopAttach);
	pSectionAP->getProperty("bot-attach", (const XML_Char *&)pszBotAttach);
	UT_DEBUGMSG(("CellLayout _lookupProps top %s bot %s left %s right %s \n",pszTopAttach,pszBotAttach,pszLeftAttach,pszRightAttach)); 
	if(pszLeftAttach && pszLeftAttach[0])
	{
		m_iLeftAttach = atoi(pszLeftAttach);
	}
	else
	{
		m_iLeftAttach = 0;
	}
	if(pszRightAttach && pszRightAttach[0])
	{
		m_iRightAttach = atoi(pszRightAttach);
	}
	else
	{
		m_iRightAttach = m_iLeftAttach + 1;
	}
	if(pszTopAttach && pszTopAttach[0])
	{
		m_iTopAttach = atoi(pszTopAttach);
	}
	else
	{
		m_iTopAttach = 0;
	}
	if(pszBotAttach && pszBotAttach[0])
	{
		m_iBotAttach = atoi(pszBotAttach);
	}
	else
	{
		m_iBotAttach = m_iTopAttach+1;
	}
}

UT_sint32   fl_CellLayout::getLeftOffset(void) const
{
	return m_iLeftOffset;
}

#ifndef WITH_PANGO
UT_sint32  fl_CellLayout::getLeftOffsetInLayoutUnits(void) const
{
	return m_iLeftOffsetLayoutUnits;
}
#endif
UT_sint32   fl_CellLayout::getRightOffset(void) const
{
	return m_iRightOffset;
}
#ifndef WITH_PANGO
UT_sint32   fl_CellLayout::getRightOffsetInLayoutUnits(void) const
{
	return m_iRightOffsetLayoutUnits;
}
#endif

UT_sint32 fl_CellLayout::getTopOffset(void) const
{
	return m_iTopOffset;
}

#ifndef WITH_PANGO
UT_sint32 fl_CellLayout::getTopOffsetInLayoutUnits(void) const
{
	return m_iTopOffsetLayoutUnits;
}
#endif

UT_sint32 fl_CellLayout::getBottomOffset(void) const
{
	return m_iBottomOffset;
}

#ifndef WITH_PANGO
UT_sint32 fl_CellLayout::getBottomOffsetInLayoutUnits(void) const
{
	return m_iBottomOffsetLayoutUnits;
}
#endif


void fl_CellLayout::localCollapse(void)
{

	// ClearScreen on our Cell. One Cell per layout.

	fp_CellContainer *pCell = (fp_CellContainer *) getFirstContainer();
	UT_DEBUGMSG(("SEVIOR: Local collapse of CellLayout %x CellContainer %x \n",this,pCell));
	if (pCell)
	{
		pCell->clearScreen();
	}

	// get rid of all the layout information for every containerLayout
	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL)
	{
		pCL->collapse();
		pCL = pCL->getNext();
	}
}

void fl_CellLayout::collapse(void)
{
	localCollapse();

	// Delete our Cell. One Cell per layout.

	fp_CellContainer *pCell = (fp_CellContainer *) getFirstContainer();
//
// Remove it from the table container
//
	if (pCell)
	{
		fp_TableContainer * pTabCon = (fp_TableContainer *) pCell->getContainer();
		pTabCon->removeContainer(pCell);
		delete pCell;
	}
	setFirstContainer(NULL);
	setLastContainer(NULL);
}

bool fl_CellLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
	UT_ASSERT(pcrx->getStruxType()== PTX_SectionCell);


	fl_ContainerLayout * pPrev = getPrev();
	fl_ContainerLayout * pNext = getNext();

	collapse();

	if(pPrev != NULL)
	{
		pPrev->setNext(pNext);
	}
	else
	{
		myContainingLayout()->setFirstLayout(pNext);
	}
	if(pNext != NULL)
	{
		pNext->setPrev(pPrev);
	}
	else
	{
		myContainingLayout()->setLastLayout(pPrev);
	}

	delete this;			// TODO whoa!  this construct is VERY dangerous.

	return true;
}

/*!
 * This method removes all layout structures contained by this layout
 * structure.
 */
void fl_CellLayout::_purgeLayout(void)
{
	fl_ContainerLayout * pCL = getFirstLayout();
	while(pCL)
	{
		fl_ContainerLayout * pNext = pCL->getNext();
		delete pCL;
		pCL = pNext;
	}
}

