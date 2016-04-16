/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * BIDI Copyright (c) 2004, Martin Sevior
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


#include "fp_EmbedRun.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "fl_BlockLayout.h"
#include "ut_debugmsg.h"
#include "ut_locale.h"
#include "ut_std_string.h"
#include "pd_Document.h"
#include "ut_mbtowc.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "gr_Graphics.h"
#include "gr_Painter.h"
#include "gr_DrawArgs.h"
#include "gr_EmbedManager.h"

fp_EmbedRun::fp_EmbedRun(fl_BlockLayout* pBL, 
					   UT_uint32 iOffsetFirst,PT_AttrPropIndex indexAP,pf_Frag_Object* oh)	: 
	fp_Run(pBL,  iOffsetFirst,1, FPRUN_EMBED ),
	m_iPointHeight(0),
	m_pSpanAP(NULL),
	m_iGraphicTick(0),
	m_pszDataID(NULL),
	m_sEmbedML(""),
	m_pEmbedManager(NULL),
	m_iEmbedUID(-1),
	m_iIndexAP(indexAP),
	m_pDocLayout(NULL),
	m_bNeedsSnapshot(true),
	m_OH(oh)
{
	m_pDocLayout = getBlock()->getDocLayout();
	lookupProperties(getGraphics());
}

fp_EmbedRun::~fp_EmbedRun(void)
{
  getEmbedManager()->releaseEmbedView(m_iEmbedUID);
}

GR_EmbedManager * fp_EmbedRun::getEmbedManager(void) const
{
  return m_pEmbedManager;
}

void fp_EmbedRun::_lookupProperties(const PP_AttrProp * pSpanAP,
									const PP_AttrProp * /*pBlockAP*/,
									const PP_AttrProp * /*pSectionAP*/,
									GR_Graphics * pG)
{
	UT_return_if_fail(pSpanAP != NULL);

	UT_DEBUGMSG(("fp_EmbedRun _lookupProperties span %p \n",pSpanAP));
	m_pSpanAP = pSpanAP;
	m_bNeedsSnapshot = true;
	pSpanAP->getAttribute("dataid", m_pszDataID);
	const gchar * pszEmbedType = NULL;
	pSpanAP->getProperty("embed-type", pszEmbedType);
	UT_ASSERT(pszEmbedType);
	UT_DEBUGMSG(("Embed Type %s \n",pszEmbedType));
	bool bFontChanged = false;

// Load this into EmbedView

	// LUCA: chunk of code moved up here from the bottom of the method
	// 'cause we need to retrieve the font-size
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;
	FL_DocLayout * pLayout = getBlock()->getDocLayout();
	if(pG == NULL && pLayout->isQuickPrint() )
	{
	     pG = getGraphics();
	     if((m_iEmbedUID >= 0) && getEmbedManager())
	     {
		 getEmbedManager()->releaseEmbedView(m_iEmbedUID);
		 m_iEmbedUID = -1;
	     }
	     m_iEmbedUID = -1;
	}
	
	getBlockAP(pBlockAP);

	const GR_Font * pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,pG);
	if(pLayout->isQuickPrint() && pG->queryProperties(GR_Graphics::DGP_PAPER))

	{
	     if(m_iEmbedUID >= 0 )
	     {
		 getEmbedManager()->releaseEmbedView(m_iEmbedUID);
		 m_iEmbedUID = -1;
	     }
	     m_iEmbedUID = - 1;
	     m_pEmbedManager = m_pDocLayout->getQuickPrintEmbedManager(pszEmbedType);
	}
	else
	{
	    m_pEmbedManager = m_pDocLayout->getEmbedManager(pszEmbedType);
	}
	if (pFont != _getFont())
	{
		_setFont(pFont);
		bFontChanged = true;
	}
	if(pG == NULL)
	  pG = getGraphics();
	m_iPointHeight = pG->getFontAscent(pFont) + pG->getFontDescent(pFont);
	const char* pszSize = PP_evalProperty("font-size",pSpanAP,pBlockAP,pSectionAP,
					      getBlock()->getDocument(), true);

	// LUCA: It is fundamental to do this before the EmbedView object
	// gets destroyed to avoid resuscitating it

	UT_sint32 iWidth,iAscent,iDescent=0;
	if(m_iEmbedUID < 0)
	{
	  PD_Document * pDoc = getBlock()->getDocument();
	  m_iEmbedUID = getEmbedManager()->makeEmbedView(pDoc,m_iIndexAP,m_pszDataID);
	  UT_DEBUGMSG((" EmbedRun %p UID is %d \n",this,m_iEmbedUID));
	  getEmbedManager()->initializeEmbedView(m_iEmbedUID);
	  getEmbedManager()->setRun (m_iEmbedUID, this);
	  getEmbedManager()->loadEmbedData(m_iEmbedUID);
	}
	getEmbedManager()->setDefaultFontSize(m_iEmbedUID,atoi(pszSize));
	if (bFontChanged)
		bFontChanged = getEmbedManager()->setFont(m_iEmbedUID,pFont);
	if(getEmbedManager()->isDefault())
	{
	  iWidth = _getLayoutPropFromObject("width");
	  iAscent = _getLayoutPropFromObject("ascent");
	  iDescent = _getLayoutPropFromObject("descent");
	}
	else
	{
	  const char * pszHeight = NULL;
	  bool bFoundHeight = pSpanAP->getProperty("height", pszHeight) && !bFontChanged;
	  const char * pszWidth = NULL;
	  bool bFoundWidth = pSpanAP->getProperty("width", pszWidth) && !bFontChanged;
	  const char * pszAscent = NULL;
	  bool bFoundAscent = pSpanAP->getProperty("ascent", pszAscent);

	  if(!bFoundWidth || pszWidth == NULL)
	  {
	      iWidth = getEmbedManager()->getWidth(m_iEmbedUID);
	  }
	  else
	  {
	      iWidth = UT_convertToLogicalUnits(pszWidth);
	      if(iWidth <= 0)
	      {
			  iWidth = getEmbedManager()->getWidth(m_iEmbedUID);
	      }
	  }
	  if(!bFoundHeight || pszHeight == NULL || !bFoundAscent || pszAscent == NULL)
	  {
	      iAscent = getEmbedManager()->getAscent(m_iEmbedUID);
		  iDescent = getEmbedManager()->getDescent(m_iEmbedUID);
	  }
	  else
	  {
	      iAscent = UT_convertToLogicalUnits(pszAscent);
	      if(iAscent <= 0)
	      {
			  iAscent = getEmbedManager()->getAscent(m_iEmbedUID);
			  iDescent = getEmbedManager()->getDescent(m_iEmbedUID);
	      }
		  else
		  {
			  UT_sint32 iHeight = UT_convertToLogicalUnits(pszHeight);
			  const char * pszDescent = NULL;
			  bool bFoundDescent = pSpanAP->getProperty("descent", pszDescent);
			  if (bFoundDescent && pszDescent != NULL && iHeight >= 0)
			  {
				  iDescent = UT_convertToLogicalUnits(pszDescent);
				  if (iHeight != iAscent + iDescent)
					  iAscent = iHeight * iAscent / (iAscent + iDescent);
			  }
			  iDescent = (iHeight >= iAscent)? iHeight - iAscent: 0;
		  }
	  }
	}
	UT_DEBUGMSG(("Width = %d Ascent = %d Descent = %d \n",iWidth,iAscent,iDescent)); 

	fl_DocSectionLayout * pDSL = getBlock()->getDocSectionLayout();
	fp_Page * p = NULL;
	if(pDSL->getFirstContainer())
	{
		p = pDSL->getFirstContainer()->getPage();
	}
	else
	{
		p = pDSL->getDocLayout()->getNthPage(0);
	}
	UT_sint32 maxW = p->getWidth() - UT_convertToLogicalUnits("0.1in"); 
	UT_sint32 maxH = p->getHeight() - UT_convertToLogicalUnits("0.1in");
	maxW -= pDSL->getLeftMargin() + pDSL->getRightMargin();
	maxH -= pDSL->getTopMargin() + pDSL->getBottomMargin();
	markAsDirty();
	if(getLine())
	{
		getLine()->setNeedsRedraw();
	}
	if(iAscent < 0)
	{
	  iAscent = 0;
	}
	if(iDescent < 0)
	{
	  iDescent = 0;
	}
	_setAscent(iAscent);
	_setDescent(iDescent);
	_setWidth(iWidth);
	_setHeight(iAscent+iDescent);
	_updatePropValuesIfNeeded();
}


void fp_EmbedRun::_drawResizeBox(UT_Rect box)
{
        _getView()->drawSelectionBox(box,isResizeable());
}

bool fp_EmbedRun::canBreakAfter(void) const
{
	return true;
}

bool fp_EmbedRun::canBreakBefore(void) const
{
	return true;
}

bool fp_EmbedRun::_letPointPass(void) const
{
	return false;
}

bool fp_EmbedRun::hasLayoutProperties(void) const
{
	return true;
}

bool fp_EmbedRun::isSuperscript(void) const
{
	return false;
}


bool fp_EmbedRun::isSubscript(void) const
{
	return false;
}

void fp_EmbedRun::_lookupLocalProperties()
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	getBlockAP(pBlockAP);

	if(!getBlock()->isContainedByTOC())
	{
		getSpanAP(pSpanAP);
	}

	_lookupProperties(pSpanAP, pBlockAP, pSectionAP,getGraphics());
}

void fp_EmbedRun::updateVerticalMetric()
{
#if 0
	// do something here to make the embedded view to redo its layout ...
	// there might be a more efficient way, but this should work
	if(m_iEmbedUID >= 0 )
	{
		getEmbedManager()->releaseEmbedView(m_iEmbedUID);
		m_iEmbedUID = -1;
	}

	// now lookup local properties which will create a new embedded view for us
	_lookupLocalProperties();

	// _lookupProperties() fixed also our width, so if width was marked as dirty, clear
	// that flag
	_setRecalcWidth(false);
#endif
}

bool fp_EmbedRun::_recalcWidth(void)
{
#if 0
	if(!_getRecalcWidth())
		return false;
	
	UT_sint32 iWidth = getWidth();

	// do something here to make the embedded view to redo its layout ...
	// there might be a more efficient way, but this should work
	if(m_iEmbedUID >= 0 )
	{
		getEmbedManager()->releaseEmbedView(m_iEmbedUID);
		m_iEmbedUID = -1;
	}

	// now lookup local properties which will create a new embedded view for us
	_lookupLocalProperties();
	
	return (iWidth != getWidth());
#else
	return false;
#endif
}

void fp_EmbedRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool & /*isTOC*/)
{
	if (x > getWidth())
		pos = getBlock()->getPosition() + getBlockOffset() + getLength();
	else
		pos = getBlock()->getPosition() + getBlockOffset();

	bBOL = false;
	bEOL = false;
}

void fp_EmbedRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
{
	//UT_DEBUGMSG(("fintPointCoords: ImmageRun\n"));
	UT_sint32 xoff;
	UT_sint32 yoff;

	UT_ASSERT(getLine());

	getLine()->getOffsets(this, xoff, yoff);
	if (iOffset == (getBlockOffset() + getLength()))
	{
		x = xoff + getWidth();
		x2 = x;
	}
	else
	{
		x = xoff;
		x2 = x;
	}
	y = yoff + getAscent() - m_iPointHeight;
	height = m_iPointHeight;
	y2 = y;
	bDirection = (getVisDirection() != UT_BIDI_LTR);
}

void fp_EmbedRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	//	UT_ASSERT(!isDirty());

	UT_ASSERT(getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN));

	UT_sint32 xoff = 0, yoff = 0;
	// need to clear full height of line, in case we had a selection
	getLine()->getScreenOffsets(this, xoff, yoff);
	UT_sint32 iLineHeight = getLine()->getHeight();
	UT_DEBUGMSG(("Clear screen embed top %d \n",yoff));
	Fill(getGraphics(),xoff, yoff, getWidth(), iLineHeight);
	markAsDirty();
	setCleared();
}

const char * fp_EmbedRun::getDataID(void) const
{
	return m_pszDataID;
}

/*!
 * Returns true if the embedable plugin is editable.
 */
bool fp_EmbedRun::isEdittable(void)
{
  return getEmbedManager()->isEdittable(m_iEmbedUID);
}

bool fp_EmbedRun::isResizeable(void)
{
  return getEmbedManager()->isResizeable(m_iEmbedUID);
}

void fp_EmbedRun::_draw(dg_DrawArgs* pDA)
{
	GR_Graphics *pG = pDA->pG;
#if 0
	UT_DEBUGMSG(("Draw with class %x \n",pG));
	UT_DEBUGMSG(("Contents of fp EmbedRun \n %s \n",m_sEmbedML.utf8_str()));
#endif
	FV_View* pView = _getView();
	UT_return_if_fail(pView);

	// need to draw to the full height of line to join with line above.
	UT_sint32 xoff= 0, yoff=0, DA_xoff = pDA->xoff;

	getLine()->getScreenOffsets(this, xoff, yoff);

	// need to clear full height of line, in case we had a selection

	UT_sint32 iFillHeight = getLine()->getHeight();
	UT_sint32 iFillTop = pDA->yoff - getLine()->getAscent();

	UT_uint32 iSelAnchor = pView->getSelectionAnchor();
	UT_uint32 iPoint = pView->getPoint();

	UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
	UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);

	UT_ASSERT(iSel1 <= iSel2);

	UT_uint32 iRunBase = getBlock()->getPosition() + getOffsetFirstVis();

	// Fill with background, then redraw.

	UT_sint32 iLineHeight = getLine()->getHeight();
	bool bIsSelected = false;
	if ( !pG->queryProperties(GR_Graphics::DGP_PAPER) && 
	    (isInSelectedTOC() || (iSel1 <= iRunBase && iSel2 > iRunBase))
		)
	{
	  // Need the painter lock to be released at the end of this block
	        GR_Painter painter(pG);
		painter.fillRect(_getView()->getColorSelBackground(), /*pDA->xoff*/DA_xoff, iFillTop, getWidth(), iFillHeight);
		bIsSelected = true;

		getEmbedManager()->setColor(m_iEmbedUID,_getView()->getColorSelForeground());

	}
	else
	{
		Fill(getGraphics(),pDA->xoff, pDA->yoff - getAscent(), getWidth()+getGraphics()->tlu(1), iLineHeight+getGraphics()->tlu(1));
		getEmbedManager()->setColor(m_iEmbedUID,getFGColor());
	}

	UT_Rect rec;
	rec.left = pDA->xoff;
	rec.top = pDA->yoff;
	rec.height = getHeight();
	rec.width = getWidth();
	if(getEmbedManager()->isDefault())
	{
	  rec.top -= _getLayoutPropFromObject("ascent");
	}
	UT_DEBUGMSG(("Draw Embed object top %d \n",rec.top));
	getEmbedManager()->render(m_iEmbedUID,rec);
	if(m_bNeedsSnapshot && !getEmbedManager()->isDefault() && getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN)  )
	{
	  UT_Rect myrec = rec;
	  myrec.top -= getAscent();
	  if(!bIsSelected)
	  {
	    getEmbedManager()->makeSnapShot(m_iEmbedUID,myrec);
	    m_bNeedsSnapshot = false;
	  }
	}
	if(bIsSelected)
	{
	  UT_Rect myrec = rec;
	  if(!getEmbedManager()->isDefault())
	  {
	    myrec.top -= getAscent();
	  }
	  _drawResizeBox(myrec);
	}
}

/*!
 * This method is used to determine the value of the layout properties
 * of the embed runs. The values returned are in logical units.
 * The properties are "height","width","ascent","decent".
 * If the propeties are not defined return -1
 */
UT_sint32  fp_EmbedRun::_getLayoutPropFromObject(const char * szProp) const
{
  PT_AttrPropIndex api = getBlock()->getDocument()->getAPIFromSOH(m_OH);
  const PP_AttrProp * pAP = NULL;
  const char * szPropVal = NULL;
  getBlock()->getDocument()->getAttrProp(api, &pAP);
  if(pAP)
    {
      bool bFound = pAP->getProperty(szProp, szPropVal);
      if(bFound)
	{
	  return UT_convertToLogicalUnits(szPropVal);
	}
    }
  return -1;
}

/*!
 * Returns true if the properties are changed in the document.
 */
bool fp_EmbedRun::_updatePropValuesIfNeeded(void)
{
  UT_sint32 iVal = 0;
  if(getEmbedManager()->isDefault())
    {
      return false;
    }
  PT_AttrPropIndex api = getBlock()->getDocument()->getAPIFromSOH(m_OH);
  const PP_AttrProp * pAP = NULL;
  const char * szPropVal = NULL;
  getBlock()->getDocument()->getAttrProp(api, &pAP);
  UT_return_val_if_fail(pAP,false);
  bool bFound = pAP->getProperty("height", szPropVal);
  bool bDoUpdate = false;
  if(bFound)
    {
      iVal = UT_convertToLogicalUnits(szPropVal);
      bDoUpdate = (iVal != getHeight());
    }
  else
    {
      bDoUpdate = true;
    }
  bFound = pAP->getProperty("width", szPropVal);
  if(bFound && !bDoUpdate)
    {
      iVal = UT_convertToLogicalUnits(szPropVal);
      bDoUpdate = (iVal != getWidth());
    }
  else
    {
      bDoUpdate = true;
    }
  bFound = pAP->getProperty("ascent", szPropVal);
  if(bFound && !bDoUpdate)
    {
      iVal = UT_convertToLogicalUnits(szPropVal);
      bDoUpdate = (iVal != static_cast<UT_sint32>(getAscent()));
    }
  else
    {
      bDoUpdate = true;
    }
  bFound = pAP->getProperty("descent", szPropVal);
  if(bFound && !bDoUpdate)
    {
      iVal = UT_convertToLogicalUnits(szPropVal);
      bDoUpdate = (iVal != static_cast<UT_sint32>(getDescent()));
    }
  else
    {
      bDoUpdate = true;
    }
  if(bDoUpdate)
    {
	  UT_LocaleTransactor t(LC_NUMERIC, "C");

      const PP_PropertyVector props = {
          "height", UT_std_string_sprintf("%fin", static_cast<double>(getHeight())/1440.),
          "width", UT_std_string_sprintf("%fin", static_cast<double>(getWidth())/1440.),
          "ascent", UT_std_string_sprintf("%fin", static_cast<double>(getAscent())/1440.),
          "descent", UT_std_string_sprintf("%fin", static_cast<double>(getDescent())/1440.)
      };
      getBlock()->getDocument()->changeObjectFormatNoUpdate(PTC_AddFmt,m_OH,
                                PP_NOPROPS,
                                props);
      return true;
    }
  return false;
}

void fp_EmbedRun::update()
{
	m_iIndexAP = getBlock()->getDocument()->getAPIFromSOH(m_OH);
	m_pEmbedManager->updateData(m_iEmbedUID, m_iIndexAP);
	m_pEmbedManager->loadEmbedData(m_iEmbedUID);
}

EV_EditMouseContext fp_EmbedRun::getContextualMenu(void) const
{
	return m_pEmbedManager->getContextualMenu();
}
