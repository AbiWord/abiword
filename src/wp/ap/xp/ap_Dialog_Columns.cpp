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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "fp_PageSize.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "ap_Dialog_Columns.h"

AP_Dialog_Columns::AP_Dialog_Columns(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogcolumns.html")
{

	m_answer = a_OK;
	m_pColumnsPreview = NULL;
	m_bLineBetween = false;
	m_HeightString = "0.0in";
	m_SpaceAfterString = "0pt";
	m_iColumnOrder = 0;
	m_pDoc = NULL;
	m_pView = NULL;
	m_dMarginTop = 0.0;
	m_dMarginBottom = 0.0;
	m_dMarginLeft = 0.0;
	m_dMarginRight = 0.0;
}

AP_Dialog_Columns::~AP_Dialog_Columns(void)
{
	DELETEP(m_pColumnsPreview);
}

AP_Dialog_Columns::tAnswer AP_Dialog_Columns::getAnswer(void) const
{
	return m_answer;
}

void AP_Dialog_Columns::_createPreviewFromGC(GR_Graphics * gc,
										   UT_uint32 width,
										   UT_uint32 height)
{
	UT_ASSERT(gc);

	m_pColumnsPreview = new AP_Columns_preview(gc,this);
	UT_ASSERT(m_pColumnsPreview);

	m_pColumnsPreview->setWindowSize(width, height);
	m_pColumnsPreview->set(m_iColumns, m_bLineBetween);

}

void AP_Dialog_Columns::setColumns(UT_uint32 iColumns)
{
	m_iColumns = iColumns;

	if(m_pColumnsPreview)
		m_pColumnsPreview->set(m_iColumns, m_bLineBetween);

	enableLineBetweenControl(m_iColumns != 1);
}

void AP_Dialog_Columns::setColumnOrder(UT_uint32 iOrder)
{
	m_iColumnOrder = iOrder;
}

/*!
 * Returns the dimensioned string that defines the maximum height of the
 * Column.
\returns const char * dimensioned string which is the max height of the column.
*/
const char * AP_Dialog_Columns::getHeightString(void)
{
	return m_HeightString.c_str();
}

/*!
 * Returns the dimensioned string that defines the space between Columns.
\returns const char * dimensioned string which is the space between columns.
*/
const char * AP_Dialog_Columns::getSpaceAfterString(void)
{
	return m_SpaceAfterString.c_str();
}

/*!
 * Returns the increment associated with the dimension defined in the string.
\params const char * sz the dimensioned string.
\returns double -  the increment associated with the dimension in sz
*/
double AP_Dialog_Columns::getIncrement(const char * sz)
{
	double inc = 0.1;
	UT_Dimension dim =  UT_determineDimension(sz);
	if(dim == DIM_IN)
	{
		inc = 0.02;
	}
	else if(dim == DIM_CM)
	{
		inc = 0.1;
	}
	else if(dim == DIM_MM)
	{
		inc = 1.0;
	}
	else if(dim == DIM_PI)
	{
		inc = 1.0;
	}
	else if(dim == DIM_PT)
	{
		inc = 1.0;
	}
	else if(dim == DIM_PX)
	{
		inc = 1.0;
	}
	else
	{
		inc = 0.02;
	}
	return inc;
}

/*!
 * Sets pointer to the Current View and document. Also sets initial values
 * for space-after and max column height.
\params XAP_Frame * pFrame - pointer to current Frame.
*/
void AP_Dialog_Columns::setViewAndDoc(XAP_Frame * pFrame)
{
	XML_Char  pszAfter[25];
	XML_Char  pszMaxHeight[25];

	m_pView = (FV_View *) pFrame->getCurrentView();
	m_pDoc = m_pView->getDocument();
	const XML_Char ** pszSecProps = NULL;
	m_pView->getSectionFormat(&pszSecProps);

	_convertToPreferredUnits( pFrame, (const XML_Char *)
	UT_getAttribute("section-space-after",pszSecProps), (const XML_Char *)pszAfter);
	_convertToPreferredUnits( pFrame, (const XML_Char *)
	UT_getAttribute("section-max-column-height",pszSecProps), (const XML_Char *)pszMaxHeight);

	if(pszAfter && *pszAfter)
	{
		m_SpaceAfterString =  (const char *) pszAfter;
	}
	if(pszMaxHeight && *pszMaxHeight)
	{
		UT_DEBUGMSG(("SEVIOR: Initial Height string = %s \n",pszMaxHeight));
		m_HeightString = pszMaxHeight;
	}
	const XML_Char * pszMarginTop = UT_getAttribute("page-margin-top",pszSecProps);
	const XML_Char * pszMarginBottom =  UT_getAttribute("page-margin-bottom",pszSecProps);
	const XML_Char * pszMarginLeft =  UT_getAttribute("page-margin-left",pszSecProps);
	const XML_Char * pszMarginRight =  UT_getAttribute("page-margin-right",pszSecProps);
	if(pszMarginTop && *pszMarginTop)
	{
		m_dMarginTop = UT_convertToInches(pszMarginTop);
	}
	if(pszMarginBottom && *pszMarginBottom)
	{
		m_dMarginBottom = UT_convertToInches(pszMarginBottom);
	}
	if(pszMarginLeft && *pszMarginLeft)
	{
		m_dMarginLeft = UT_convertToInches(pszMarginLeft);
	}
	if(pszMarginRight && *pszMarginRight)
	{
		m_dMarginRight = UT_convertToInches(pszMarginRight);
	}
	DELETEP(pszSecProps);
}

/*!
 * Returns the current width of the document page.
\returns UT_sint32 width of page in inches
*/
double AP_Dialog_Columns::getPageWidth(void)
{
	double width = 1.0;
	if(m_pDoc)
	{
		width =  m_pDoc->m_docPageSize.Width(DIM_IN) - m_dMarginLeft - m_dMarginRight;
	}
	return width;
}


/*!
 * Returns the current height of the document page.
\returns UT_sint32 height of page in inches
*/
double AP_Dialog_Columns::getPageHeight(void)
{
	double height = 1.0;
	if(m_pDoc)
	{
		height =  m_pDoc->m_docPageSize.Height(DIM_IN) - m_dMarginTop - m_dMarginBottom;
	}
	return height;
}

/*!
 * Increment the member variable UT_String defining the dimensioned string
 * for the space between columns.
 */
void AP_Dialog_Columns::incrementSpaceAfter(bool bIncrement)
{
	double inc = getIncrement(m_SpaceAfterString.c_str());
	if(!bIncrement)
	{
		inc = -inc;
	}
	UT_Dimension dim = UT_determineDimension(getSpaceAfterString(), DIM_none);
	m_SpaceAfterString = UT_incrementDimString(m_SpaceAfterString.c_str(),inc);
	double dum = UT_convertToInches(getSpaceAfterString());
	if(dum < 0.0)
	{
		m_SpaceAfterString = UT_convertInchesToDimensionString(dim,0.0);
	}
	m_bSpaceAfterChanged = true;
	if(m_pColumnsPreview)
		m_pColumnsPreview->set(m_iColumns, m_bLineBetween);
}

/*!
 * Increment the member variable UT_String defining the dimensioned string
 * for the maximum column height.
 */
void AP_Dialog_Columns::incrementMaxHeight(bool bIncrement)
{
	double inc = getIncrement(m_HeightString.c_str());
	if(!bIncrement)
	{
		inc = -inc;
	}
	UT_Dimension dim = UT_determineDimension(getHeightString(), DIM_none);
	m_HeightString = UT_incrementDimString(m_HeightString.c_str(),inc);
	double dum = UT_convertToInches(getHeightString());
	if(dum < 0.0)
	{
		m_HeightString = UT_convertInchesToDimensionString(dim,0.0);
	}
	m_bMaxHeightChanged = true;
	if(m_pColumnsPreview)
		m_pColumnsPreview->set(m_iColumns, m_bLineBetween);
}

void AP_Dialog_Columns::setLineBetween(bool bState)
{
	m_bLineBetween = bState;

	if(m_pColumnsPreview)
		m_pColumnsPreview->set(m_iColumns, m_bLineBetween);

}

/*!
 * Returns the 100* the fraction of the total page height in the Maximum height string.
 */
double AP_Dialog_Columns::getMaxHeightPercent(void)
{
	double height = 100.0*UT_convertToInches(getHeightString())/getPageHeight();
	return height;
}

/*!
 * Returns the 100* the fraction of the total page height in space after.
 */
double AP_Dialog_Columns::getSpaceAfterPercent(void)
{
	double space = 100.0*UT_convertToInches(getSpaceAfterString())/getPageHeight();
	return space;
}

/*!
 * Set the member string variable m_HeightString
\params const char * szHeight is the string containing the new value
*/
void AP_Dialog_Columns::setMaxHeight(const char * szHeight)
{
	UT_Dimension dim = UT_determineDimension(szHeight, DIM_none);
	if(dim != DIM_none)
	{
		m_bMaxHeightChanged = true;
		m_HeightString = szHeight;
		double dum = UT_convertToInches(getHeightString());
		if(dum < 0.0)
		{
			m_HeightString = UT_convertInchesToDimensionString(dim,0.0);
		}
		if(m_pColumnsPreview)
			m_pColumnsPreview->set(m_iColumns, m_bLineBetween);
	}
}

/*!
 * Set the member string variable m_SpaceAfterString.
\params const char * szAfter is the string containing the new value
*/
void AP_Dialog_Columns::setSpaceAfter(const char * szAfter)
{
	UT_Dimension dim = UT_determineDimension(szAfter, DIM_none);
	if(dim != DIM_none)
	{
		m_bSpaceAfterChanged = true;
		m_SpaceAfterString = szAfter;
		double dum = UT_convertToInches(getSpaceAfterString());
		if(dum < 0.0)
		{
			m_SpaceAfterString = UT_convertInchesToDimensionString(dim,0.0);
		}
		if(m_pColumnsPreview)
			m_pColumnsPreview->set(m_iColumns, m_bLineBetween);
	}
}


void AP_Dialog_Columns::_drawColumnButton(GR_Graphics *gc, UT_Rect rect, UT_uint32 iColumns)
{
	gc->clearArea(rect.left, rect.top, rect.width, rect.height);

	rect.left += 2;
	rect.width -= 4;
	rect.top += 2;
	rect.height -= 4;
	m_previewDrawer.draw(gc, rect, iColumns, false, 0.0, 0.0);
}

/*!
 * Converts the string sz into the units seleced for the ruler.
\params XAP_Frame * pFrame defined the frame of the application
\params const char * sz is the string containing the old value
\params const XML_Char * pRet is the string to which the new value is copied.
*/
void AP_Dialog_Columns::_convertToPreferredUnits(XAP_Frame * pFrame,const char *sz, const XML_Char *pRet)
{
	UT_Dimension PreferedUnits = DIM_none;
	const XML_Char * pszRulerUnits = NULL;

	if (pFrame->getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits, &pszRulerUnits))
	{
		PreferedUnits = UT_determineDimension((char *)pszRulerUnits);
	};
	UT_XML_strncpy((XML_Char *) pRet, 25, (const XML_Char *) UT_reformatDimensionString(PreferedUnits,sz));
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_Columns_preview::AP_Columns_preview(GR_Graphics * gc, AP_Dialog_Columns * pColumns)
	: XAP_Preview(gc)
{
	m_pColumns = pColumns;
}

AP_Columns_preview::~AP_Columns_preview()
{
}

const UT_sint32 iLinesToDraw = 16;

void AP_Columns_preview::draw(void)
{
	UT_sint32 iWidth = getWindowWidth();
	UT_sint32 iHeight = getWindowHeight();
    double maxHeightPercent = m_pColumns->getMaxHeightPercent();
	double SpacePercent = m_pColumns->getSpaceAfterPercent();
	UT_Rect pageRect(5, 5, iWidth - 10, iHeight - 10);

	m_gc->fillRect(GR_Graphics::CLR3D_Background, 0, 0, iWidth, iHeight);
	m_gc->clearArea(pageRect.left, pageRect.top, pageRect.width,
					pageRect.height);

	m_gc->setLineWidth(1);
	m_gc->drawLine(pageRect.left, pageRect.top,
				   pageRect.left + pageRect.width, pageRect.top);
	m_gc->drawLine(pageRect.left, pageRect.top,
				   pageRect.left, pageRect.top + pageRect.height);

	m_gc->setLineWidth(3);
	m_gc->drawLine(pageRect.left + pageRect.width, pageRect.top + 1,
				   pageRect.left + pageRect.width,
				   pageRect.top + pageRect.height);
	m_gc->drawLine(pageRect.left + 1, pageRect.top + pageRect.height,
				   pageRect.left + pageRect.width,
				   pageRect.top + pageRect.height);


	pageRect.top += 5;
	pageRect.height -= 5;
	m_previewDrawer.draw(m_gc, pageRect, m_iColumns, m_bLineBetween,maxHeightPercent, SpacePercent);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void AP_Columns_preview_drawer::draw(GR_Graphics *gc, UT_Rect &rect, UT_sint32 iColumns, bool bLineBetween, double maxHeightPercent, double SpacePercent)
{

	UT_sint32 iHalfColumnGap = rect.width / 20;

	UT_sint32 y_start = rect.top + iHalfColumnGap;
	UT_sint32 y_end = rect.top + rect.height - iHalfColumnGap;

	UT_sint32 y_step = (y_end - y_start) / iLinesToDraw;
	y_step = 2;
	UT_DEBUGMSG(("SEVIOR: maxheightpercent = %f \n",maxHeightPercent));
	maxHeightPercent = maxHeightPercent/100.0;
	SpacePercent = SpacePercent/100.0;
	if(maxHeightPercent < 0.01)
	{
		maxHeightPercent = 1.1;
	}
	gc->setLineWidth(1);
	UT_RGBColor Line_color(0, 0, 0);
	gc->setColor(Line_color);

	rect.left += iHalfColumnGap;
	rect.width -= 2 * iHalfColumnGap;
	double d_ysize = (double) (y_end - y_start);
	UT_sint32 iSpace = static_cast<UT_sint32>(SpacePercent* d_ysize);
	if(iSpace < y_step)
	{
		iSpace = y_step;
	}
	UT_sint32 maxHeight = static_cast<UT_sint32>(maxHeightPercent * d_ysize);
	for (UT_sint32 i = 1; i <= iColumns; i++)
	{
		UT_sint32 curskip = 0;
		for(UT_sint32 y = y_start; y < y_end; y += y_step)
		{
			UT_sint32 xLeft, xRight;

			// a little bit of math to avoid/replace a (nasty) switch statement
			xLeft = rect.left + iHalfColumnGap + ((i-1) * rect.width/iColumns);
			xRight = rect.left - iHalfColumnGap + (i * rect.width / iColumns);
			curskip += y_step;
			if(curskip >= maxHeight )
			{
					curskip = 0;
					y += iSpace;
			}
			gc->drawLine(xLeft, y, xRight, y);
		}
	}

	if(bLineBetween)
	{
		// a bit of math to avoid/replace a (nasty) switch statement
		for (UT_sint32 j = 2; j <= iColumns; j++)
		{
			UT_sint32 x;

			x = rect.left + (j-1) * rect.width / iColumns;
			gc->drawLine(x, y_start, x, y_end);
		}
	}
}






