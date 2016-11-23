/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Martin Sevior
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

#include <stdlib.h>
#include <string.h>

#include "fp_ContainerObject.h"
#include "fl_SectionLayout.h"
#include "fl_DocLayout.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "fp_Column.h"
#include "fp_Line.h"
#include "fp_TableContainer.h"
#include "fl_TableLayout.h"
#include "fp_FootnoteContainer.h"
#include "fp_FrameContainer.h"
#include "fl_FootnoteLayout.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fp_Column.h"
#include "fp_Run.h"
#include "fv_View.h"
#include "gr_Painter.h"

/*!
  Create container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_ContainerObject::fp_ContainerObject(FP_ContainerType iType, fl_SectionLayout* pSectionLayout)
	:       m_iConType(iType),
			m_pSectionLayout(pSectionLayout),
			m_iDirection(UT_BIDI_UNSET),
		m_iBreakTick(0),
		m_iRef(0),
		m_bCanDelete(true)
{
	UT_ASSERT(pSectionLayout);
}


const char * fp_ContainerObject::getContainerString(void)
{
	switch (getContainerType())
	{
	case 	FP_CONTAINER_RUN:
		return "FP_CONTAINER_RUN";
	case 	FP_CONTAINER_LINE:
		return "FP_CONTAINER_LINE";
	case	FP_CONTAINER_VERTICAL:
		return "FP_CONTAINER_VERTICAL";
	case 	FP_CONTAINER_ROW:
		return "FP_CONTAINER_ROW";
    case	FP_CONTAINER_TABLE:
		return "FP_CONTAINER_TABLE";
    case 	FP_CONTAINER_CELL:
		return "FP_CONTAINER_CELL";
	case 	FP_CONTAINER_COLUMN:
		return "FP_CONTAINER_COLUMN";
	case FP_CONTAINER_HDRFTR:
		return "FP_CONTAINER_HDRFTR";
	case FP_CONTAINER_ENDNOTE:
		return "FP_CONTAINER_ENDNOTE";
	case FP_CONTAINER_FOOTNOTE:
		return "FP_CONTAINER_FOOTNOTE";
	case FP_CONTAINER_ANNOTATION:
		return "FP_CONTAINER_ANNOTATION";
	case FP_CONTAINER_COLUMN_POSITIONED:
		return "FP_CONTAINER_COLUMN_POSITIONED";
	case FP_CONTAINER_COLUMN_SHADOW:
		return "FP_CONTAINER_COLUMN_SHADOW";
	case FP_CONTAINER_FRAME:
		return "FP_CONTAINER_FRAME";
	case FP_CONTAINER_TOC:
		return "FP_CONTAINER_TOC";
	default:
		return "unknown FP_CONTAINER object";
	}
}

/*!
  Destruct container
 */
fp_ContainerObject::~fp_ContainerObject()
{
	m_iConType = static_cast<FP_ContainerType>(-1);
	//	UT_ASSERT(m_bCanDelete);
}

/*!
 * return true is this container is of column type
 */
bool fp_ContainerObject::isColumnType(void) const
{
  bool b = (m_iConType == FP_CONTAINER_COLUMN) 
	  || (m_iConType == FP_CONTAINER_HDRFTR)
	  || (m_iConType == FP_CONTAINER_COLUMN_SHADOW)
	  || (m_iConType == FP_CONTAINER_FRAME)
	  || (m_iConType == FP_CONTAINER_COLUMN_POSITIONED)
	  || (m_iConType == FP_CONTAINER_FOOTNOTE)
	  || (m_iConType == FP_CONTAINER_ANNOTATION)
	  ;
  return b;
}

/*!
 * Return the DocSectionLayout that owns this
 */
fl_DocSectionLayout * fp_ContainerObject::getDocSectionLayout(void)
{
    fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(getSectionLayout());
    while(pCL && (pCL->getContainerType() != FL_CONTAINER_DOCSECTION) && (pCL->getContainerType() != FL_CONTAINER_HDRFTR))
    {
        xxx_UT_DEBUGMSG(("getDocSectionLayout: Container type %s \n",getContainerString()));
        pCL = pCL->myContainingLayout();
    }
    if(pCL && (pCL->getContainerType() == FL_CONTAINER_HDRFTR))
    {
        return static_cast<fl_HdrFtrSectionLayout *>(pCL)->getDocSectionLayout();
    }
    else if(pCL)
    {
        return static_cast<fl_DocSectionLayout *>(pCL);
    }
    return NULL;
}
/*!
 * Return a pointer to the Graphics class for this object
 */
GR_Graphics * fp_ContainerObject::getGraphics(void) const
{
	return getSectionLayout()->getDocLayout()->getGraphics();
}

/*
 *----------------------------------------------------------------------
 */

/*!
  Create container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_Container::fp_Container(FP_ContainerType iType, fl_SectionLayout* pSectionLayout)
	:       fp_ContainerObject(iType, pSectionLayout),
			m_pContainer(NULL),
			m_pNext(NULL),
			m_pPrev(NULL),
			m_pMyBrokenContainer(NULL), 
			m_cBrokenContainers(0),
			m_FillType(NULL,this,FG_FILL_TRANSPARENT)
{
	m_vecContainers.clear();
	m_FillType.setDocLayout(pSectionLayout->getDocLayout());
}

/*!
  Destruct container
 */
fp_Container::~fp_Container()
{
//
// The containers referenced in the m_vecContainers vector are owned by their
// respective fl_Layout classes and so are not destructed here.
//
}

/*!
 * This returns a pointer to a container broken across a page that
 * contains this container. This is used (in the first instance) by the
 * Table pagination code which breaks tables across pages. This returns
 * the pointer to the broken table this container is within.
 */
fp_Container * fp_Container::getMyBrokenContainer(void) const
{
	return m_pMyBrokenContainer;
}

/*!
 * Sets the pointer to the broken table which contains this container.
 */
void fp_Container::setMyBrokenContainer(fp_Container * pMyBroken)
{
	m_pMyBrokenContainer = pMyBroken;
    fp_Container * pc = this;

	while (NULL != pc)
	{
		pc->incBrokenCount();
		pc = pc->getContainer();
	}
}

void fp_Container::drawLine(const PP_PropertyMap::Line & style,
				     UT_sint32 left, UT_sint32 top, 
				     UT_sint32 right, UT_sint32 bot,
				     GR_Graphics * pGr)
{
	if (style.m_t_linestyle == PP_PropertyMap::linestyle_none &&
		!pGr->queryProperties(GR_Graphics::DGP_SCREEN))
		return; // do not draw the dotted line when printing	
	
	GR_Graphics::JoinStyle js = GR_Graphics::JOIN_MITER;
	GR_Graphics::CapStyle  cs = GR_Graphics::CAP_PROJECTING;

	switch (style.m_t_linestyle)
	{
		case PP_PropertyMap::linestyle_none:
			pGr->setLineProperties (pGr->tlu(1), js, cs, GR_Graphics::LINE_DOTTED);
			break;
		case PP_PropertyMap::linestyle_dotted:
			pGr->setLineProperties (pGr->tlu(1), js, cs, GR_Graphics::LINE_DOTTED);
			break;
		case PP_PropertyMap::linestyle_dashed:
			pGr->setLineProperties (pGr->tlu(1), js, cs, GR_Graphics::LINE_ON_OFF_DASH);
			break;
		case PP_PropertyMap::linestyle_solid:
			pGr->setLineProperties (pGr->tlu(1), js, cs, GR_Graphics::LINE_SOLID);
			break;
		default: // do nothing; shouldn't happen
			break;
	}
	pGr->setLineWidth (static_cast<UT_sint32>(style.m_thickness));
	if (style.m_t_linestyle == PP_PropertyMap::linestyle_none)
	{
	        pGr->setLineProperties (pGr->tlu(1), js, cs, GR_Graphics::LINE_SOLID);
		return;
	}
	else
	{
		pGr->setColor (style.m_color);
	}

	xxx_UT_DEBUGMSG(("_drawLine: top %d bot %d \n",top,bot));

	GR_Painter painter(pGr);

	painter.drawLine (left, top, right, bot);
	pGr->setLineProperties (pGr->tlu(1), js, cs, GR_Graphics::LINE_SOLID);
}


void fp_Container::setNext(fp_ContainerObject * pNext)
{
  m_pNext = pNext;
  if(pNext)
    pNext->ref();
}

void fp_Container::setPrev(fp_ContainerObject * pPrev)
{
  m_pPrev = pPrev;
  if(pPrev)
    pPrev->ref();
}

/*!
 * Recursive clears all broken containers contained by this container
 */
void fp_Container::clearBrokenContainers(void)
{
	if(m_pMyBrokenContainer) 
	{
		fp_Container * pc = this;
		
		while (NULL != pc)
		{
			pc->decBrokenCount();
			pc = pc->getContainer();
		}
		m_pMyBrokenContainer = NULL;
	}
	if (0 != getBrokenCount())
	{
		UT_sint32 i =0;

		for(i=0;(i<countCons()) && (0 != getBrokenCount());i++)
		{
			fp_Container * pCon = static_cast<fp_Container *>(getNthCon(i));
			if (pCon && (0 != pCon->getBrokenCount()))
			{
				pCon->clearBrokenContainers();
			}
		}
	}
	m_cBrokenContainers = 0;
}


UT_uint32 fp_Container::binarysearchCons(const void* key, int (*compar)(const 
void *, const void *)) const
{
	UT_uint32 u = m_vecContainers.binarysearch(key, compar);

	return u;
}

/*!
 * The container that encloses this container.
 */
void fp_Container::setContainer(fp_Container * pCO)
{
        UT_ASSERT(pCO != this);
	m_pContainer = pCO;
	if(pCO != NULL)
	{
		m_FillType.setParent(&pCO->getFillType());
	}
	else
	{
		m_FillType.setParent(NULL);
	}
}

/*!
 * Return the container that encloses this container.
 */
fp_Container * fp_Container::getContainer(void) const
{
	return m_pContainer;
}


/*!
 * Return the column that encloses this container.
 */
fp_Container* fp_Container::getColumn(void) const
{
	const fp_Container * pCon = this;
	while(pCon && ((!pCon->isColumnType())) )
	{
		pCon = pCon->getContainer();
	}
	return const_cast<fp_Container *>(pCon);
}

/*!
 * Get the page containing this container.
 */
fp_Page * fp_Container::getPage(void) const
{
	fp_Container * pCon = getColumn();
	if(pCon == NULL)
	{
		return NULL;
	}
	if(pCon->getContainerType() == FP_CONTAINER_COLUMN)
	{
		return static_cast<fp_Column *>(pCon)->getPage();
	}
	if(pCon->getContainerType() == FP_CONTAINER_COLUMN_POSITIONED)
	{
		return static_cast<fp_Column *>(pCon)->getPage();
	}
	if(pCon->getContainerType() == FP_CONTAINER_FRAME)
	{
		return static_cast<fp_FrameContainer *>(pCon)->getPage();
	}
	if(pCon->getContainerType() == FP_CONTAINER_COLUMN_SHADOW)
	{
		return static_cast<fp_ShadowContainer *>(pCon)->getPage();
	}
	if(pCon->getContainerType() == FP_CONTAINER_HDRFTR)
	{
		return NULL;
	}
	if(pCon->getContainerType() == FP_CONTAINER_FOOTNOTE)
	{
		return static_cast<fp_FootnoteContainer *>(pCon)->getPage();
	}
	if(pCon->getContainerType() == FP_CONTAINER_ANNOTATION)
	{
		return static_cast<fp_AnnotationContainer *>(pCon)->getPage();
	}
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

void fp_Container::insertConAt(fp_ContainerObject * pCon, UT_sint32 i)
{
#if 0 // DEBUG
	if(pCon->getContainerType() == FP_CONTAINER_LINE)
	{
		fp_Line * pLine = static_cast<fp_Line *>(pCon);
		UT_ASSERT(pLine->getBlock() != NULL);
		if(countCons() > 0)
		{
			fp_ContainerObject * pNext = 	m_vecContainers.getNthItem(i);
			if(pNext && pNext->getContainerType() == FP_CONTAINER_LINE)
			{
				fl_BlockLayout * pBL = pLine->getBlock();
				fl_BlockLayout * pNextBL = static_cast<fp_Line *>(pNext)->getBlock();
				if(pBL->canContainPoint() && pNextBL->canContainPoint())
					UT_ASSERT(pNextBL->getPosition() >= pBL->getPosition());
			}
		}
	}
#endif
        UT_ASSERT(pCon != this);
	m_vecContainers.insertItemAt(pCon,i);
	pCon->ref();
}

void fp_Container::addCon(fp_ContainerObject * pCon)
{
#if 0 // DEBUG
	if(pCon->getContainerType() == FP_CONTAINER_LINE)
	{
		fp_Line * pLine = static_cast<fp_Line *>(pCon);
		UT_ASSERT(pLine->getBlock() != NULL);
		UT_sint32 i = countCons();
		if(i>0)
		{
			fp_ContainerObject * pPrev = 	m_vecContainers.getNthItem(i-1);
			if(pPrev && pPrev->getContainerType() == FP_CONTAINER_LINE)
			{
				fl_BlockLayout * pBL = pLine->getBlock();
				fl_BlockLayout * pPrevBL = static_cast<fp_Line *>(pPrev)->getBlock();
				UT_ASSERT(pPrevBL->getPosition() <= pBL->getPosition());
			}
		}
	}
#endif
        UT_ASSERT(pCon != this);
	m_vecContainers.addItem(pCon);
	pCon->ref();
}

fp_ContainerObject *  fp_Container:: getNthCon(UT_sint32 i) const
{ 
	if(countCons() == 0) return NULL;
	return m_vecContainers.getNthItem(i);
}

UT_sint32  fp_Container::countCons(void) const
{
	return m_vecContainers.getItemCount();
}

UT_sint32  fp_Container::findCon(fp_ContainerObject * pCon) const
{
	return m_vecContainers.findItem(pCon);
}

bool  fp_Container::isEmpty(void) const
{
	return m_vecContainers.getItemCount() == 0;
}

bool fp_Container::getPageRelativeOffsets(UT_Rect &r) const
{
	// the X offset of a container is relative to page margin, what we
	// want is offset relative to the page edge
	fp_Container * pColumnC = getColumn();
	
	UT_return_val_if_fail(pColumnC,false);
	fl_DocSectionLayout * pDSL = NULL;
	if(pColumnC->getContainerType() != FP_CONTAINER_FOOTNOTE || pColumnC->getContainerType() != FP_CONTAINER_ANNOTATION)
	{
		if(pColumnC->getContainerType() == FP_CONTAINER_FRAME)
		{
			fp_FrameContainer * pFC = static_cast<fp_FrameContainer *>(pColumnC);
			pDSL = pFC->getDocSectionLayout();
		}
		else
		{
			fl_SectionLayout * pSL = static_cast<fl_SectionLayout *>(pColumnC->getSectionLayout());
			if(pSL->getContainerType() == FL_CONTAINER_HDRFTR)
			{
				pDSL = static_cast<fl_HdrFtrSectionLayout *>(pSL)->getDocSectionLayout();
			}
			else if(pSL->getContainerType() == FL_CONTAINER_SHADOW)
			{
				pDSL = static_cast<fl_DocSectionLayout *>(static_cast<fl_HdrFtrShadow *>(pSL)->getSectionLayout());
			}
			else
			{
				pDSL = pSL->getDocSectionLayout();
			}
		}
	}
	else if (pColumnC->getContainerType() == FP_CONTAINER_FOOTNOTE)
	{
		fp_FootnoteContainer * pFC = static_cast<fp_FootnoteContainer *>(pColumnC);
		fl_FootnoteLayout * pFL = static_cast<fl_FootnoteLayout *>(pFC->getSectionLayout());
		pDSL = static_cast<fl_DocSectionLayout *>(pFL->myContainingLayout());
	}
	else if (pColumnC->getContainerType() == FP_CONTAINER_ANNOTATION)
	{
		fp_AnnotationContainer * pAC = static_cast<fp_AnnotationContainer *>(pColumnC);
		fl_AnnotationLayout * pAL = static_cast<fl_AnnotationLayout *>(pAC->getSectionLayout());
		pDSL = static_cast<fl_DocSectionLayout *>(pAL->myContainingLayout());
	}
	UT_return_val_if_fail(pDSL,false);
	UT_ASSERT(pDSL->getContainerType() == FL_CONTAINER_DOCSECTION);
	r.left   = pDSL->getLeftMargin();
	r.top    = pDSL->getTopMargin();
	r.width  = getDrawingWidth();
	r.height = getHeight();

	r.left += getX();
	r.top  += getY();
	return true;
}

void fp_Container::justRemoveNthCon(UT_sint32 i)
{
        fp_ContainerObject * pCon = getNthCon(i);
	pCon->unref();
	m_vecContainers.deleteNthItem(i);
}

void  fp_Container::deleteNthCon(UT_sint32 i)
{
	fp_Container * pCon = static_cast<fp_Container *>(getNthCon(i));
	if(pCon->getContainer() == this)
	{
		pCon->setContainer(NULL);
	}
	pCon->unref();
	m_vecContainers.deleteNthItem(i);
	xxx_UT_DEBUGMSG(("AFter deleting item %d in %x there are %d cons left \n",i,this,countCons()));
}

bool fp_Container::isOnScreen() const
{
	UT_return_val_if_fail(getSectionLayout(),false);
	
	FV_View *pView = getSectionLayout()->getDocLayout()->getView();

	if(!pView)
	{
		return false;
	}
	if(getPage())
	{
		return getPage()->isOnScreen();
	}
	return false;
}

/*!
 * Need to be able to fiddle with the fillType so don't do a const return;
 */
fg_FillType & fp_Container::getFillType(void)
{
	return m_FillType;
}

const fg_FillType & fp_Container::getFillType(void) const
{
	return m_FillType;
}

/*!
 * To draw the background under text. Basic idea is to draw the colour
 * or image defined here unless the fill type is transparent. If't transparent
 * we recursively call it's parents until it's not transparent.
 */
fg_FillType::fg_FillType(fg_FillType *pParent, fp_ContainerObject * pContainer, FG_Fill_Type iType):
	m_pParent(pParent),
	m_pContainer(pContainer),
	m_pDocLayout(NULL),
	m_FillType(iType),
	m_pImage(NULL),
	m_pGraphic(NULL),
	m_iGraphicTick(0),
	m_bTransparentForPrint(false),
	m_color(255,255,255),
	m_TransColor(255,255,255),
	m_bTransColorSet(false),
	m_bColorSet(false),
	m_iWidth(0),
	m_iHeight(0),
	m_pDocImage(NULL),
	m_pDocGraphic(NULL),
	m_bIgnoreLineLevel(false)
{
}

fg_FillType::~fg_FillType(void)
{
	DELETEP(m_pImage);
	DELETEP(m_pGraphic);
}

/*!
 * Set the parent of this fill class.
 */
void  fg_FillType::setParent(fg_FillType * pParent)
{
	m_pParent = pParent;
}

/*!
 *
 */
void  fg_FillType::setIgnoreLineLevel(bool b)
{
        m_bIgnoreLineLevel = b;
}
/*!
 * set this class to have a solid color fill
 */
void fg_FillType::setColor(UT_RGBColor & color)
{
	xxx_UT_DEBUGMSG(("Fill type set to color class \n"));
	m_FillType = FG_FILL_COLOR;
	m_color = color;
	m_bColorSet = true;
	m_bTransparentForPrint = false;
	DELETEP(m_pImage);
	DELETEP(m_pGraphic);
}

/*!
 * set this class to have a solid color fill unless this is a NULL string
 * pointer. This is an on-screen color only. Don't print this.
 */
void fg_FillType::setColor(const char * pszColor)
{
	if(pszColor)
	{
		if(strcmp(pszColor,"transparent") == 0)
		{
			if(!m_bTransColorSet)
			{
				m_FillType = FG_FILL_TRANSPARENT;
			}
			m_bColorSet = false;
		}
		else
		{
			m_FillType = FG_FILL_COLOR;
			m_bColorSet = true;
			DELETEP(m_pImage);
			DELETEP(m_pGraphic);
		}
		m_color.setColor(pszColor);
		m_bTransparentForPrint = false;
	}
	else
	{
		if(!m_bTransColorSet)
		{
			m_FillType = FG_FILL_TRANSPARENT;
			m_bColorSet = false;
		}
	}
}

/*!
 * Set a pointer to the Image pointer in fl_DocSectionLayout. This
 * enables many pages to share the same image without having to generate
 * a new image for every page.
 */
void fg_FillType::setImagePointer(const FG_SharedGraphicPtr & pDocGraphic, GR_Image ** pDocImage)
{
	if(pDocImage != NULL)
	{
		DELETEP(m_pImage);
		DELETEP(m_pGraphic);
	}
	m_pDocImage = pDocImage;
	m_pDocGraphic = pDocGraphic;
	m_FillType = FG_FILL_IMAGE;
}

/*!
 * set this class to have a solid color fill but not print this color.
 */
void fg_FillType::setTransColor(UT_RGBColor & color)
{
	UT_DEBUGMSG(("Fill type set to color class \n"));
	m_FillType = FG_FILL_COLOR;
	m_TransColor = color;
	DELETEP(m_pImage);
	DELETEP(m_pGraphic);
	m_bTransColorSet = true;
}

/*!
 * set this class to have a solid color fill unless this is a NULL string
 * pointer 
 */
void fg_FillType::setTransColor(const char * pszColor)
{
	if(pszColor)
	{
		if(strcmp(pszColor,"transparent") == 0)
		{
			if(!m_bColorSet)
			{
				m_FillType = FG_FILL_TRANSPARENT;
			}
			m_bTransColorSet = false;
			m_bTransparentForPrint = false;
		}
		else
		{
			m_FillType = FG_FILL_COLOR;
			m_bTransColorSet = true;
			m_bTransparentForPrint = true;
		}
		m_TransColor.setColor(pszColor);
	}
	else
	{
		if(!m_bColorSet)
		{
			m_FillType = FG_FILL_TRANSPARENT;
			m_bTransparentForPrint = false;
		}
		m_bTransparentForPrint = false;
	}
	DELETEP(m_pImage);
	DELETEP(m_pGraphic);
}

/*!
 * set this class to be transparent.
 */
void fg_FillType::setTransparent(void)
{
	m_FillType = FG_FILL_TRANSPARENT;
	DELETEP(m_pImage);
	DELETEP(m_pGraphic);	
	m_bTransparentForPrint = false;
}

/*!
 * set this class to have an image background for fills.
 */
void fg_FillType::setImage(FG_Graphic * pGraphic, GR_Image * pImage, GR_Graphics * pG, UT_sint32 iWidth, UT_sint32 iHeight)
{
	m_FillType = FG_FILL_IMAGE;
	DELETEP(m_pImage);
	DELETEP(m_pGraphic);	
	m_pImage = pImage;
	m_pGraphic = pGraphic;
	m_bTransparentForPrint = false;
	setWidthHeight(pG,iWidth,iHeight);
	m_pDocImage = NULL;
}

/*!
 * Set the doc layout for this class.
 */
void  fg_FillType::setDocLayout(FL_DocLayout * pDocLayout)
{
	m_pDocLayout = pDocLayout;
	if(m_pDocLayout)
	{
		m_iGraphicTick = m_pDocLayout->getGraphicTick();
	}
}

/*!
 * Set that the fill should be transparent if we're printing.
 */
void fg_FillType::markTransparentForPrint(void)
{
	m_bTransparentForPrint = true;
}

/*!
 * Return the parent of this class.
 */
fg_FillType * fg_FillType::getParent(void) const
{
	return m_pParent;
}

/*!
 * Return the filltype of this class.
 */
FG_Fill_Type fg_FillType::getFillType(void) const
{
	return m_FillType;
}

void fg_FillType::_regenerateImage(GR_Graphics * pG)
{
	UT_return_if_fail(m_pGraphic);
	UT_return_if_fail(m_pDocLayout);
	DELETEP(m_pImage);
	m_pImage = m_pGraphic->regenerateImage(pG);
	setWidthHeight(pG,m_iWidth,m_iHeight);
	UT_Rect rec(0,0,m_iWidth,m_iHeight);
	m_pImage->scaleImageTo(pG,rec);
	m_iGraphicTick = m_pDocLayout->getGraphicTick();
}

/*!
 * Return the most appropriate color.
 */
const UT_RGBColor * fg_FillType::getColor(void) const
{
	if(m_bColorSet)
	{
		return & m_color;
	}
	if(m_bTransColorSet)
	{
		return &m_TransColor;
	}
	if(getParent())
	{
		return getParent()->getColor();
	}
	return &m_color;
}

void fg_FillType::setWidthHeight(GR_Graphics * pG, UT_sint32 iWidth, UT_sint32 iHeight, bool bDoImage)
{
	if((m_iWidth == iWidth) && (m_iHeight == iHeight))
	{
		return;
	}
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	if((m_iHeight <= 0) || (m_iWidth <= 0))
	{
		return;
	}
	if(m_pImage && bDoImage)
	{
		DELETEP(m_pImage);
		m_pImage = m_pGraphic->regenerateImage(pG);
		UT_Rect rec(0,0,iWidth,iHeight);
		m_pImage->scaleImageTo(pG,rec);
	}
	if(m_pDocImage && *m_pDocImage && bDoImage)
	{
		DELETEP(*m_pDocImage);
		*m_pDocImage = m_pDocGraphic->regenerateImage(pG);
		UT_Rect rec(0,0,m_iWidth,m_iHeight);
		(*m_pDocImage)->scaleImageTo(pG,rec);
	}
}


void fg_FillType::setWidth(GR_Graphics * pG, UT_sint32 iWidth)
{
	if(iWidth == m_iWidth)
	{
		return;
	}
	m_iWidth = iWidth;
	if((m_iHeight <= 0) || (m_iWidth <= 0))
	{
		return;
	}
	if(m_pImage)
	{
		DELETEP(m_pImage);
		m_pImage = m_pGraphic->regenerateImage(pG);
		UT_Rect rec(0,0,m_iWidth,m_iHeight);
		m_pImage->scaleImageTo(pG,rec);
	}
	if(m_pDocImage && *m_pDocImage)
	{
		DELETEP(*m_pDocImage);
		*m_pDocImage = m_pDocGraphic->regenerateImage(pG);
		UT_Rect rec(0,0,m_iWidth,m_iHeight);
		(*m_pDocImage)->scaleImageTo(pG,rec);
	}
}

void fg_FillType::setHeight(GR_Graphics * pG, UT_sint32 iHeight)
{
	if(iHeight == m_iHeight)
	{
		return;
	}
	m_iHeight = iHeight;
	if((m_iHeight <= 0) || (m_iWidth <= 0))
	{
		return;
	}
	if(m_pImage)
	{
		DELETEP(m_pImage);
		m_pImage = m_pGraphic->regenerateImage(pG);
		UT_Rect rec(0,0,m_iWidth,m_iHeight);
		m_pImage->scaleImageTo(pG,rec);
	}
	if(m_pDocImage && *m_pDocImage)
	{
		DELETEP(*m_pDocImage);
		*m_pDocImage = m_pDocGraphic->regenerateImage(pG);
		UT_Rect rec(0,0,m_iWidth,m_iHeight);
		(*m_pDocImage)->scaleImageTo(pG,rec);
	}

}

/*!
 * Actually do the fill for this class.
 */
void fg_FillType::Fill(GR_Graphics * pG, UT_sint32 & srcX, UT_sint32 & srcY, UT_sint32 x, UT_sint32 y, UT_sint32 width, UT_sint32 height)
{
	if(y < -9999999)
	{
	     // Whoops! object is offscreen!
	     // Bailout
	     return;
	}

  //
  // Images appear to be 1 pixel narrower than rectangular
  // fills
  //
        UT_sint32 imageOffset = pG->tlu(1);


//
// Have to adjust for spacing between cells
//
	GR_Painter painter(pG);
	UT_RGBColor white(255,255,255);
	bool bIsFrame = false;
	if(m_pContainer && (m_pContainer->getContainerType() == FP_CONTAINER_CELL))
	{
		const fp_CellContainer * pCell = static_cast<const fp_CellContainer *>(m_pContainer);
		UT_sint32 xoff,yoff;
		pCell->getLeftTopOffsets(xoff,yoff);
		if(m_FillType == FG_FILL_IMAGE)
		{
			srcX -= xoff;
			srcY -= 2*yoff;
		}
	}
	if(m_pContainer && (m_pContainer->getContainerType() == FP_CONTAINER_FRAME))
	{
		const fp_FrameContainer * pFrame = static_cast<const fp_FrameContainer *>(m_pContainer);
		UT_sint32 xoff = pFrame->getXPad();
		UT_sint32 yoff = pFrame->getYPad();
		if(m_FillType == FG_FILL_IMAGE)
		{
			srcX += xoff;
			srcY += yoff;
			bIsFrame = true;
		}
		if(getParent() && ((m_FillType == FG_FILL_IMAGE) || (m_FillType == FG_FILL_TRANSPARENT) ))
		{
			m_color = getParent()->m_color;
			m_bColorSet = getParent()->m_bColorSet;
			m_TransColor = getParent()->m_TransColor;
			m_bTransColorSet = getParent()->m_bTransColorSet;
			if(m_pDocImage == NULL)
			    m_pDocImage = getParent()->m_pDocImage;
		}
	}
	if(m_pContainer && (m_pContainer->getContainerType() == FP_CONTAINER_LINE))
	{
	    fp_Line * pLine = static_cast<fp_Line *>(m_pContainer);
	    UT_sint32 left,right = 0;
	    pLine->getAbsLeftRight(left,right);
	    if(x < left)
	    {
	      UT_DEBUGMSG(("Line x fill outside bounds!! x %d left %d \n",x,left));
	      // UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	      x= left;
	    }
	    if((x + width) > right)
	    {
	      UT_DEBUGMSG(("Line right fill outside bounds!! r %d right %d \n",x+width,left));
	      // UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	      width = right -x;
	    }
	    if(m_bIgnoreLineLevel && getParent() && m_pContainer)
	    {
		UT_sint32 newX = srcX + (m_pContainer->getX());
		UT_sint32 newY = srcY + (m_pContainer->getY());
		getParent()->Fill(pG,newX,newY,x,y,width,height);
		return;
	    }
	}
	if(m_pContainer && (m_pContainer->getContainerType() == FP_CONTAINER_RUN))
	{
	        fp_Line * pLine = static_cast<fp_Run *>(m_pContainer)->getLine();
		if(pLine == NULL)
		  return;
	        UT_sint32 left,right = 0;
	        pLine->getAbsLeftRight(left,right);
	        if(x < left)
		{
		  xxx_UT_DEBUGMSG(("Run x fill outside bounds!! x %d left %d \n",x,left));
		  // UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		  x= left;
		}
	        if((x + width) > right)
		{
		  xxx_UT_DEBUGMSG(("run right fill outside bounds!! r %d right %d \n",x+width,left));
		  // UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		  width = right -x;
		}
		if(m_iGraphicTick != m_pDocLayout->getGraphicTick() )
		{
			m_iGraphicTick = m_pDocLayout->getGraphicTick();
			fp_Run * pRun = static_cast<fp_Run *>(m_pContainer);
			pRun->_setFont(NULL);
			pRun->lookupProperties(pG);
			if((m_FillType == FG_FILL_IMAGE) && (m_pDocImage == NULL))
			{
				_regenerateImage(pG);
			}
		}
	}
	
	UT_Rect src;
	UT_Rect dest;
	xxx_UT_DEBUGMSG(("----Called fill -- Parent = %x Container %x FillType %d \n",m_pParent,m_pContainer,m_FillType));
	if(!pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		if(m_bTransparentForPrint)
		{
			if(getParent() && m_pContainer)
			{
				 UT_sint32 newX = x + (m_pContainer->getX());
				 UT_sint32 newY = y + (m_pContainer->getY());
				 getParent()->Fill(pG,newX,newY,x,y,width,height);
				 return;
			 }
			 return;
		 }
		 if(m_FillType == FG_FILL_TRANSPARENT)
		 {
			 return;
		 }
		 if(m_FillType == FG_FILL_IMAGE)
		 {
			 if(m_pDocImage == NULL)
			 {
				 _regenerateImage(pG);
			 }
			 src.left = srcX;
			 src.top = srcY;
			 src.width = width;
			 src.height = height;
			 dest.left = x;
			 dest.top = y;
			 dest.width = width;
			 dest.height = height;
//
// Overwrite With white first for alpha blended images
//
			 if(!pG->queryProperties(GR_Graphics::DGP_PAPER))
			 {
			         painter.fillRect(white,dest);
			 }
			 if(m_pDocImage == NULL)
			 {
				 painter.fillRect(m_pImage,src,dest);
			 }
			 else if(*m_pDocImage)
			 {
			   if(!bIsFrame)
			   {
				 painter.fillRect(*m_pDocImage,src,dest);
			   }
			   else
			   {
			         painter.drawImage(*m_pDocImage, dest.left,dest.top);
			   }
			 }
			 return;
		 }
		 if(m_FillType == FG_FILL_COLOR && m_bColorSet)
		 {
			 painter.fillRect(m_color,x,y,width,height);
			 return;
		 }
		 return;
	}
	if(m_FillType == FG_FILL_TRANSPARENT)
	{
		 xxx_UT_DEBUGMSG(("Fill type transparent ! \n"));
		 if(getParent() && m_pContainer)
		 {
			 xxx_UT_DEBUGMSG(("Fill type transparent chaining up to parent %x ! \n",getParent()));
			 UT_sint32 newX = srcX + (m_pContainer->getX());
			 UT_sint32 newY = srcY + (m_pContainer->getY());
			 getParent()->Fill(pG,newX,newY,x,y,width,height);
			 return;
		 }
		 xxx_UT_DEBUGMSG(("Fill type transparent but no parent ! \n"));
		 painter.fillRect(white,x,y,width,height);
		 return;
	}
	if(m_FillType == FG_FILL_COLOR && m_bColorSet)
	{
		 xxx_UT_DEBUGMSG(("Fill type Color ! \n"));
		 painter.fillRect(m_color,x,y,width,height);
		 return;
	}
	if(m_FillType == FG_FILL_IMAGE)
	{
		 xxx_UT_DEBUGMSG(("Fill type Image ! srcX %d srcY %d x  %d y %d width %d height %d \n",srcX,srcY,x,y,width,height));
		 if((m_pDocImage == NULL) && (m_pDocLayout->getGraphicTick() != m_iGraphicTick))
		 {
		         _regenerateImage(pG);
		 }
		 if(srcX < 0)
		 {
			 UT_sint32 iX = -srcX;
			 srcX = 0;
			 painter.fillRect(white,x,y,iX,height);
			 width -= iX;
		 }
		 
		 if(srcY < 0)
		 {
			 UT_sint32 iY = -srcY;
			 srcY = 0;
			 painter.fillRect(white,x,y,width,iY);
			 height -= iY;
		 }
		 if(m_pContainer) 
         {
		   xxx_UT_DEBUGMSG((" ContainerType %s \n ",m_pContainer->getContainerString()));
         }
		 xxx_UT_DEBUGMSG(("m_pDocImage %p  Trans col %d m_bColorSet %d \n ",m_pDocImage, m_bTransColorSet, m_bColorSet));

		src.left = srcX;
		src.top = srcY;
		src.width = width+imageOffset;
		src.height = height+imageOffset;
		dest.left = x;
		dest.top = y;
		dest.width = width+imageOffset;
		dest.height = height+imageOffset;
//
// Only fill the bits exposed by the clip rect
//
		const UT_Rect * pClip = pG->getClipRect();
		if(pClip != NULL)
		{
			UT_sint32 leftDiff = 0;
			UT_sint32 widthDiff = 0;
			UT_sint32 topDiff = 0;
			UT_sint32 heightDiff = 0;
			if(pClip->left > dest.left)
			{
				leftDiff = pClip->left - dest.left - pG->tlu(2) -1;
				src.left += leftDiff;
				dest.left += leftDiff;
				src.width -= leftDiff;
				dest.width -= leftDiff;
				if(dest.width <= 0)
				{
					return;
				}
			}
			if(pClip->left + pClip->width < dest.left + dest.width)
			{
				widthDiff = dest.left + dest.width - pClip->left - pClip->width  - pG->tlu(2) -1;
				src.width -= widthDiff;
				dest.width -= widthDiff;
				if(dest.width <= 0)
				{
					return;
				}
			}
			if(pClip->top > dest.top)
			{
				topDiff = pClip->top - dest.top  - pG->tlu(2) -1;
				src.top += topDiff;
				dest.top += topDiff;
				src.height -= topDiff;
				dest.height -= topDiff;
				if(dest.height <= 0)
				{
					return;
				}
			}
			if(pClip->top + pClip->height < dest.top + dest.height)
			{
				heightDiff = dest.top + dest.height - pClip->top - pClip->height  - pG->tlu(2) -1;
				src.height -= heightDiff;
				dest.height -= heightDiff;
				if(dest.height <= 0)
				{
					return;
				}
			}
		}

		if(m_pDocImage == NULL)
		{
		        if(m_bTransColorSet)
			{
			    painter.fillRect(m_TransColor,x,y,width,height);
			}
			else if(m_bColorSet)
			{
			    painter.fillRect(m_color,x,y,width,height);
			}
			else
			{
			    painter.fillRect(white,x,y,width,height);
			}
			painter.fillRect(m_pImage,src,dest);
		}
		else if(*m_pDocImage)
		{
		        if(m_bTransColorSet)
			{
			    painter.fillRect(m_TransColor,x,y,width,height);
			}
			else if(m_bColorSet)
			{
			    painter.fillRect(m_color,x,y,width,height);
			}
			else if(getParent() && getParent()->m_pDocImage && *getParent()->m_pDocImage)
			{
			    painter.fillRect(white,x,y,width,height);
			    painter.fillRect(*getParent()->m_pDocImage,src,dest);
			}
			else
			{
			    painter.fillRect(white,x,y,width,height);
			}
			painter.fillRect(*m_pDocImage,src,dest);
		}
		else
		{
			painter.fillRect(white,x,y,width,height);
		}
	}		
	if(m_FillType == FG_FILL_COLOR && m_bTransColorSet)
        {
		 xxx_UT_DEBUGMSG(("Fill type Trans Color ! \n"));
		 painter.fillRect(m_TransColor,x,y,width,height);
		 return;
	}
	return;
}

