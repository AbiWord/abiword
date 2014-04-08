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


#include "fp_MathRun.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "fl_BlockLayout.h"
#include "ut_debugmsg.h"
#include "pd_Document.h"
#include "ut_mbtowc.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "gr_Graphics.h"
#include "gr_Painter.h"
#include "gr_DrawArgs.h"
#include "gr_EmbedManager.h"

fp_MathRun::fp_MathRun(fl_BlockLayout* pBL, 
					   UT_uint32 iOffsetFirst,PT_AttrPropIndex indexAP,pf_Frag_Object* oh)	: 
	fp_Run(pBL,  iOffsetFirst,1, FPRUN_MATH ),
	m_iPointHeight(0),
	m_pSpanAP(NULL),
	m_iGraphicTick(0),
	m_pszDataID(NULL),
	m_sMathML(""),
	m_pMathManager(NULL),
        m_iMathUID(-1),
        m_iIndexAP(indexAP),
        m_pDocLayout(NULL),
	m_bNeedsSnapshot(true),
	m_OH(oh)
{
        m_pDocLayout = getBlock()->getDocLayout();
	xxx_UT_DEBUGMSG((" -----MathRun created %x Object Pointer is %x \n",this,oh));
	lookupProperties(getGraphics());
}

fp_MathRun::~fp_MathRun(void)
{
  getMathManager()->releaseEmbedView(m_iMathUID);
  xxx_UT_DEBUGMSG(("Deleting Math Object!!!!!!!!!!!! \n"));
}

GR_EmbedManager * fp_MathRun::getMathManager(void)
{
  return m_pMathManager;
}

void fp_MathRun::_lookupProperties(const PP_AttrProp * pSpanAP,
									const PP_AttrProp * /*pBlockAP*/,
									const PP_AttrProp * /*pSectionAP*/,
									GR_Graphics * pG)
{
  xxx_UT_DEBUGMSG(("fp_MathRun _lookupProperties span %x run is % uid is %d \n",pSpanAP,this,m_iMathUID));
	m_pSpanAP = pSpanAP;
	m_bNeedsSnapshot = true;
	pSpanAP->getAttribute("dataid", m_pszDataID);
	const gchar * pszFontSize = NULL;
	pSpanAP->getProperty("font-size", pszFontSize);
	xxx_UT_DEBUGMSG(("Font-size %s \n",pszFontSize));
	const char *pszDisplayMode = NULL;
	pSpanAP->getProperty("display",pszDisplayMode);

// Load this into MathView

	// LUCA: chunk of code moved up here from the bottom of the method
	// 'cause we need to retrieve the font-size
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;


	FL_DocLayout * pLayout = getBlock()->getDocLayout();
	if(pG == NULL && pLayout->isQuickPrint())
	{
	     pG = getGraphics();
	     if((m_iMathUID >= 0) && getMathManager() )
	     {
		 getMathManager()->releaseEmbedView(m_iMathUID);
		 m_iMathUID = -1;
	     }
	     m_iMathUID = -1;
	     xxx_UT_DEBUGMSG(("---Recoved from QuickPrint!! \n"));
	}

	getBlockAP(pBlockAP);

	const GR_Font * pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP,pG);
	if(pLayout->isQuickPrint() && pG->queryProperties(GR_Graphics::DGP_PAPER))
	{
	     UT_DEBUGMSG(("---Doing a QuickPrint!! \n"));
	     if(m_iMathUID >= 0 && getMathManager())
	     {
	         UT_DEBUGMSG(("MathRun Old Width = %d Ascent = %d Descent = %d \n",getWidth(),getAscent(),getDescent())); 
		 getMathManager()->releaseEmbedView(m_iMathUID);
		 m_iMathUID = -1;
	     }
	     m_iMathUID = - 1;
	     m_pMathManager = m_pDocLayout->getQuickPrintEmbedManager("mathml");
	}
	else
	{
	    m_pMathManager = m_pDocLayout->getEmbedManager("mathml");
	}
	if (pFont != _getFont())
	{
	  xxx_UT_DEBUGMSG(("!!!!Font is set here... %x \n",pFont));
		_setFont(pFont);
	}
	if(pG == NULL)
	  pG = getGraphics();
	m_iPointHeight = pG->getFontAscent(pFont) + pG->getFontDescent(pFont);
	const char* pszSize = PP_evalProperty("font-size",pSpanAP,pBlockAP,pSectionAP,
					      getBlock()->getDocument(), true);

	// LUCA: It is fundamental to do this before the MathView object
	// gets destroyed to avoid resuscitating it

	UT_sint32 iWidth,iAscent,iDescent=0;
	if(m_iMathUID < 0)
	{
	  PD_Document * pDoc = getBlock()->getDocument();
	  m_iMathUID = getMathManager()->makeEmbedView(pDoc,m_iIndexAP,m_pszDataID);
	  UT_DEBUGMSG((" MathRun %p UID is %d \n",this,m_iMathUID));
	  getMathManager()->initializeEmbedView(m_iMathUID);
	  getMathManager()->setRun (m_iMathUID, this);
	  getMathManager()->loadEmbedData(m_iMathUID);
	}
	m_pMathManager->setDisplayMode(m_iMathUID,
	                               (pszDisplayMode && !strcmp(pszDisplayMode, "inline"))?
	                                 ABI_DISPLAY_INLINE: ABI_DISPLAY_BLOCK);
	UT_sint32 iFSize = atoi(pszSize);
	getMathManager()->setDefaultFontSize(m_iMathUID,iFSize);
	getMathManager()->setFont(m_iMathUID,pFont);
	PD_Document * pDoc = getBlock()->getDocument();
	const PP_PropertyTypeColor *p_color = static_cast<const PP_PropertyTypeColor *>(PP_evalPropertyType("color",pSpanAP,pBlockAP,pSectionAP, Property_type_color, pDoc, true));
	getMathManager()->setColor(m_iMathUID, p_color->getColor());
	
	if(getMathManager()->isDefault())
	{
	  iWidth = _getLayoutPropFromObject("width");
	  iAscent = _getLayoutPropFromObject("ascent");
	  iDescent = _getLayoutPropFromObject("descent");
	}
	else
	{
	  iWidth = getMathManager()->getWidth(m_iMathUID);
	  iAscent = getMathManager()->getAscent(m_iMathUID);
	  iDescent = getMathManager()->getDescent(m_iMathUID);
	}
	m_iPointHeight = iAscent + iDescent;
	UT_DEBUGMSG(("MathRun _lookupProps Width = %d Ascent = %d Descent = %d \n",iWidth,iAscent,iDescent)); 

	fl_DocSectionLayout * pDSL = getBlock()->getDocSectionLayout();
	fp_Page * p = NULL;
	if(pDSL->getFirstContainer())
	{
		p = pDSL->getFirstContainer()->getPage();
	}
	else if(pDSL->getDocLayout()->countPages() > 0)
	{
		p = pDSL->getDocLayout()->getNthPage(0);
	}
	else
	{
	    return;
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
	if(pLayout->isQuickPrint() && pG->queryProperties(GR_Graphics::DGP_PAPER))
	{
	  UT_DEBUGMSG(("---Doing a QuickPrint!! -CHECK \n"));
	  //UT_ASSERT(getAscent() == iAscent);
	  //UT_ASSERT(getDescent() == iDescent);
	  if((getAscent() >0) && (getDescent() > 0))
	   {
	      iAscent = getAscent();
	      iDescent = getDescent();
	   }
	}
	_setAscent(iAscent);
	_setDescent(iDescent);
	_setWidth(iWidth);
	_setHeight(iAscent+iDescent);
	_updatePropValuesIfNeeded();
}


void fp_MathRun::_lookupLocalProperties()
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

void fp_MathRun::updateVerticalMetric()
{
	// do something here to make the embedded view to redo its layout ...
	// there might be a more efficient way, but this should work
	if(m_iMathUID >= 0 )
	{
		getMathManager()->releaseEmbedView(m_iMathUID);
		m_iMathUID = -1;
	}

	// now lookup local properties which will create a new embedded view for us
	_lookupLocalProperties();

	// _lookupProperties() fixed also our width, so if width was marked as dirty, clear
	// that flag
	_setRecalcWidth(false);
}

bool fp_MathRun::_recalcWidth(void)
{
	if(!_getRecalcWidth())
		return false;
	
	UT_sint32 iWidth = getWidth();

	// do something here to make the embedded view to redo its layout ...
	// there might be a more efficient way, but this should work
	if(m_iMathUID >= 0 )
	{
		getMathManager()->releaseEmbedView(m_iMathUID);
		m_iMathUID = -1;
	}

	// now lookup local properties which will create a new embedded view for us
	_lookupLocalProperties();
	
	return (iWidth != getWidth());
}

void fp_MathRun::_drawResizeBox(UT_Rect box)
{
	GR_Graphics * pG = getGraphics();
	UT_sint32 left = box.left;
	UT_sint32 top = box.top;
	UT_sint32 right = box.left + box.width - pG->tlu(1);
	UT_sint32 bottom = box.top + box.height - pG->tlu(1);

	GR_Painter painter(pG);
	
	pG->setLineProperties(pG->tluD(1.0),
								 GR_Graphics::JOIN_MITER,
								 GR_Graphics::CAP_BUTT,
								 GR_Graphics::LINE_SOLID);	
	
	// draw some really fancy box here
	pG->setColor(UT_RGBColor(98,129,131));
	painter.drawLine(left, top, right, top);
	painter.drawLine(left, top, left, bottom);
	pG->setColor(UT_RGBColor(230,234,238));
	painter.drawLine(box.left+pG->tlu(1), box.top + pG->tlu(1), right - pG->tlu(1), top+pG->tlu(1));
	painter.drawLine(box.left+pG->tlu(1), box.top + pG->tlu(1), left + pG->tlu(1), bottom - pG->tlu(1));
	pG->setColor(UT_RGBColor(98,129,131));
	painter.drawLine(right - pG->tlu(1), top + pG->tlu(1), right - pG->tlu(1), bottom - pG->tlu(1));
	painter.drawLine(left + pG->tlu(1), bottom - pG->tlu(1), right - pG->tlu(1), bottom - pG->tlu(1));
	pG->setColor(UT_RGBColor(49,85,82));
	painter.drawLine(right, top, right, bottom);
	painter.drawLine(left, bottom, right, bottom);
	painter.fillRect(UT_RGBColor(156,178,180),box.left + pG->tlu(2), box.top + pG->tlu(2), box.width - pG->tlu(4), box.height - pG->tlu(4));
}

bool fp_MathRun::canBreakAfter(void) const
{
	return true;
}

bool fp_MathRun::canBreakBefore(void) const
{
	return true;
}

bool fp_MathRun::_letPointPass(void) const
{
	return false;
}

bool fp_MathRun::hasLayoutProperties(void) const
{
	return true;
}

bool fp_MathRun::isSuperscript(void) const
{
	return false;
}


bool fp_MathRun::isSubscript(void) const
{
	return false;
}

void fp_MathRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool & /*isTOC*/)
{
	if (x > getWidth())
		pos = getBlock()->getPosition() + getBlockOffset() + getLength();
	else
		pos = getBlock()->getPosition() + getBlockOffset();

	bBOL = false;
	bEOL = false;
}

void fp_MathRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection)
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
	y = yoff;
	height = m_iPointHeight;
	y2 = y;
	bDirection = (getVisDirection() != UT_BIDI_LTR);
}

void fp_MathRun::_clearScreen(bool /* bFullLineHeightRect */)
{
	//	UT_ASSERT(!isDirty());

	UT_ASSERT(getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN));

	UT_sint32 xoff = 0, yoff = 0;

	// need to clear full height of line, in case we had a selection
	getLine()->getScreenOffsets(this, xoff, yoff);
	UT_sint32 iLineHeight = getLine()->getHeight();
	Fill(getGraphics(),xoff, yoff, getWidth(), iLineHeight);
	markAsDirty();
	setCleared();
}

const char * fp_MathRun::getDataID(void) const
{
	return m_pszDataID;
}

void fp_MathRun::_draw(dg_DrawArgs* pDA)
{
	GR_Graphics *pG = pDA->pG;
#if 0
	UT_DEBUGMSG(("Draw with class %x \n",pG));
	UT_DEBUGMSG(("Contents of fp MathRun \n %s \n",m_sMathML.utf8_str()));
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
	GR_Painter painter(pG);
	bool bIsSelected = false;

	if ( !pG->queryProperties(GR_Graphics::DGP_PAPER) && 
	     (isInSelectedTOC() || ((iSel1 <= iRunBase) && (iSel2 > iRunBase)))
	   )
	{
		painter.fillRect(_getView()->getColorSelBackground(), /*pDA->xoff*/DA_xoff, iFillTop, getWidth(), iFillHeight);
		bIsSelected = true;
	}
	else
	{
	  Fill(getGraphics(),pDA->xoff, pDA->yoff - getLine()->getAscent(), getWidth(), iLineHeight);
	}
	getMathManager()->setColor(m_iMathUID,getFGColor());
	UT_Rect rec;
	rec.left = pDA->xoff;
	rec.top = pDA->yoff;
	rec.height = getHeight();
	rec.width = getWidth();
	if(getMathManager()->isDefault())
	{
	  rec.top -= getAscent();
	}
	FL_DocLayout * pLayout = getBlock()->getDocLayout();
	if(pG && pLayout->isQuickPrint() && pG->queryProperties(GR_Graphics::DGP_PAPER) && !getMathManager()->isDefault() )
	{
	  //	  rec.top -= getAscent()*pG->getResolutionRatio();
	  //rec.top -= getAscent();
	}
	xxx_UT_DEBUGMSG(("LineHeigt %d MathRun Height %d\n",getLine()->getHeight(),getHeight()));
	xxx_UT_DEBUGMSG((" Mathrun Left %d top %d width %d height %d \n",rec.left,rec.top,rec.width,rec.height)); 
	getMathManager()->render(m_iMathUID,rec);
	if(m_bNeedsSnapshot && !getMathManager()->isDefault() && pG->queryProperties(GR_Graphics::DGP_SCREEN)  )
	{
	  rec.top -= getAscent();
	  if(!bIsSelected)
	  {
	    getMathManager()->makeSnapShot(m_iMathUID,rec);
	    m_bNeedsSnapshot = false;
	  }
	}
}

/*!
 * This method is used to determine the value of the layout properties
 * of the math runs. The values returned are in logical units.
 * The properties are "height","width","ascent","decent".
 * If the propeties are not defined return -1
 */
UT_sint32  fp_MathRun::_getLayoutPropFromObject(const char * szProp)
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
	  return atoi(szPropVal);
	}
    }
  return -1;
}

/*!
 * Returns true if the properties are changed in the document.
 */
bool fp_MathRun::_updatePropValuesIfNeeded(void)
{
  UT_sint32 iVal = 0;
  if(getMathManager()->isDefault())
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
      iVal = atoi(szPropVal);
      bDoUpdate = (iVal != getHeight());
    }
  else
    {
      bDoUpdate = true;
    }
  bFound = pAP->getProperty("width", szPropVal);
  if(bFound && !bDoUpdate)
    {
      iVal = atoi(szPropVal);
      bDoUpdate = (iVal != getWidth());
    }
  else
    {
      bDoUpdate = true;
    }
  bFound = pAP->getProperty("ascent", szPropVal);
  if(bFound && !bDoUpdate)
    {
      iVal = atoi(szPropVal);
      bDoUpdate = (iVal != static_cast<UT_sint32>(getAscent()));
    }
  else
    {
      bDoUpdate = true;
    }
  bFound = pAP->getProperty("descent", szPropVal);
  if(bFound && !bDoUpdate)
    {
      iVal = atoi(szPropVal);
      bDoUpdate = (iVal != static_cast<UT_sint32>(getDescent()));
    }
  else
    {
      bDoUpdate = true;
    }
  if(bDoUpdate)
    {
      const char * pProps[10] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
      UT_UTF8String sHeight,sWidth,sAscent,sDescent;
      UT_UTF8String_sprintf(sHeight,"%d",getHeight());
      pProps[0] = "height";
      pProps[1] = sHeight.utf8_str();
      UT_UTF8String_sprintf(sWidth,"%d",getWidth());
      pProps[2] = "width";
      pProps[3] = sWidth.utf8_str();
      UT_UTF8String_sprintf(sAscent,"%d",getAscent());
      pProps[4] = "ascent";
      pProps[5] = sAscent.utf8_str();
      UT_UTF8String_sprintf(sDescent,"%d",getDescent());
      pProps[6] = "descent";
      pProps[7] = sDescent.utf8_str();
      getBlock()->getDocument()->changeObjectFormatNoUpdate(PTC_AddFmt,m_OH,
							    NULL,
							    pProps);
      return true;
    }
  return false;
}
