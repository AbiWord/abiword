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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ap_Features.h"

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "fp_PageSize.h"
#include "gr_Painter.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "ap_Dialog_Columns.h"

AP_Dialog_Columns::AP_Dialog_Columns(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogcolumns"),
	m_answer(a_OK),
	m_pColumnsPreview(NULL),
	m_previewDrawer(),
	m_iColumns(0),
	m_bLineBetween(false),
	m_iColumnOrder (0),
	m_HeightString("0.0in"),
	m_SpaceAfterString("0pt"),
	m_pDoc(NULL),
	m_pView (NULL),
	m_bSpaceAfterChanged(false),
	m_bMaxHeightChanged(false),
	m_dMarginTop(0.0),
	m_dMarginBottom(0.0),
	m_dMarginLeft(0.0),
	m_dMarginRight(0.0)
{
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
	UT_return_if_fail (gc);

	m_pColumnsPreview = new AP_Columns_preview(gc,this);
	UT_return_if_fail (m_pColumnsPreview);

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
\param const char * sz the dimensioned string.
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
\param XAP_Frame * pFrame - pointer to current Frame.
*/
void AP_Dialog_Columns::setViewAndDoc(XAP_Frame * pFrame)
{
	gchar  pszAfter[25];
	gchar  pszMaxHeight[25];

	m_pView = static_cast<FV_View *>(pFrame->getCurrentView());
	m_pDoc = m_pView->getDocument();
	PP_PropertyVector pszSecProps;
	m_pView->getSectionFormat(pszSecProps);

	_convertToPreferredUnits( pFrame, PP_getAttribute("section-space-after",pszSecProps).c_str(), pszAfter);
	_convertToPreferredUnits( pFrame, PP_getAttribute("section-max-column-height",pszSecProps).c_str(), pszMaxHeight);

	if(*pszAfter)
	{
		m_SpaceAfterString = static_cast<const char *>(pszAfter);
	}
	if(*pszMaxHeight)
	{
		UT_DEBUGMSG(("SEVIOR: Initial Height string = %s \n",pszMaxHeight));
		m_HeightString = pszMaxHeight;
	}
	const std::string & pszMarginTop = PP_getAttribute("page-margin-top",pszSecProps);
	const std::string & pszMarginBottom =  PP_getAttribute("page-margin-bottom",pszSecProps);
	const std::string & pszMarginLeft =  PP_getAttribute("page-margin-left",pszSecProps);
	const std::string & pszMarginRight =  PP_getAttribute("page-margin-right",pszSecProps);
	if(pszMarginTop.empty())
	{
		m_dMarginTop = UT_convertToInches(pszMarginTop.c_str());
	}
	if(pszMarginBottom.empty())
	{
		m_dMarginBottom = UT_convertToInches(pszMarginBottom.c_str());
	}
	if(pszMarginLeft.empty())
	{
		m_dMarginLeft = UT_convertToInches(pszMarginLeft.c_str());
	}
	if(pszMarginRight.empty())
	{
		m_dMarginRight = UT_convertToInches(pszMarginRight.c_str());
	}
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
\param const char * szHeight is the string containing the new value
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
\param const char * szAfter is the string containing the new value
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
	GR_Painter painter(gc);

	painter.clearArea(rect.left, rect.top, rect.width, rect.height);

	rect.left += gc->tdu(2);
	rect.width -= gc->tdu(4);
	rect.top += gc->tdu(2);
	rect.height -= gc->tdu(4);
	m_previewDrawer.draw(gc, rect, iColumns, false, 0.0, 0.0);
}

/*!
 * Converts the string sz into the units seleced for the ruler.
\param XAP_Frame * pFrame defined the frame of the application
\param const char * sz is the string containing the old value
\param const gchar * pRet is the string to which the new value is copied.
*/
void AP_Dialog_Columns::_convertToPreferredUnits(XAP_Frame * /*pFrame*/,const char *sz, gchar * pRet)
{
	UT_Dimension PreferedUnits = DIM_none;
	const gchar * pszRulerUnits = NULL;

	if (XAP_App::getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits, &pszRulerUnits))
	{
		PreferedUnits = UT_determineDimension(static_cast<const char *>(pszRulerUnits));
	};
	strncpy(pRet, static_cast<const gchar *>(UT_reformatDimensionString(PreferedUnits,sz)), 25);
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_Columns_preview::AP_Columns_preview(GR_Graphics * gc, AP_Dialog_Columns * pColumns)
	: XAP_Preview(gc),
	m_previewDrawer(),
	m_pColumns(pColumns),
	m_iColumns(0),
	m_bLineBetween (false)
{
}

AP_Columns_preview::~AP_Columns_preview()
{
}

void AP_Columns_preview::draw(const UT_Rect *clip)
{
	UT_UNUSED(clip);
	GR_Painter painter(m_gc);

	UT_sint32 iWidth = m_gc->tlu(getWindowWidth());
	UT_sint32 iHeight = m_gc->tlu(getWindowHeight());

	double maxHeightPercent = m_pColumns->getMaxHeightPercent();
	double SpacePercent = m_pColumns->getSpaceAfterPercent();
	UT_Rect pageRect(m_gc->tlu(5), m_gc->tlu(5), iWidth - m_gc->tlu(10), iHeight - m_gc->tlu(10));

	painter.fillRect(GR_Graphics::CLR3D_Background, 0, 0, iWidth, iHeight);
	painter.clearArea(pageRect.left, pageRect.top, pageRect.width,
					pageRect.height);

	m_gc->setLineWidth(m_gc->tlu(1));
	m_gc->setColor3D(GR_Graphics::CLR3D_Foreground);
	painter.drawLine(pageRect.left, pageRect.top,
		       pageRect.left + pageRect.width, pageRect.top);
	painter.drawLine(pageRect.left, pageRect.top,
		       pageRect.left, pageRect.top + pageRect.height);

	m_gc->setLineWidth(m_gc->tlu(3));
	painter.drawLine(pageRect.left + pageRect.width, pageRect.top + m_gc->tlu(1),
				   pageRect.left + pageRect.width,
				   pageRect.top + pageRect.height);
	painter.drawLine(pageRect.left + m_gc->tlu(1), pageRect.top + pageRect.height,
				   pageRect.left + pageRect.width,
				   pageRect.top + pageRect.height);


	pageRect.top += m_gc->tlu(5);
	pageRect.height -= m_gc->tlu(5);
	m_previewDrawer.draw(m_gc, pageRect, m_iColumns, m_bLineBetween,maxHeightPercent, SpacePercent);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void AP_Columns_preview_drawer::draw(GR_Graphics *gc, UT_Rect &rect, UT_sint32 iColumns, bool bLineBetween, double maxHeightPercent, double SpacePercent)
{
	GR_Painter painter(gc);

	UT_sint32 iHalfColumnGap = gc->tlu (rect.width / gc->tlu(20));

	UT_sint32 y_start = rect.top + iHalfColumnGap;
	UT_sint32 y_end = rect.top + rect.height - iHalfColumnGap;

	UT_sint32 y_step = gc->tlu(4);
	maxHeightPercent /= 100.0;
	SpacePercent /= 100.0;
	if(maxHeightPercent < 0.01)
		maxHeightPercent = 1.1;

	gc->setLineWidth(gc->tlu(1));
	UT_RGBColor Line_color(0, 0, 0);
	gc->setColor(Line_color);

	rect.left += iHalfColumnGap;
	rect.width -= 2 * iHalfColumnGap;
	double d_ysize = (double) (y_end - y_start);
	UT_sint32 iSpace = static_cast<UT_sint32>(SpacePercent* d_ysize);
	if(iSpace < y_step)
	  iSpace = y_step;
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
			painter.drawLine(xLeft, y, xRight, y);
		}
	}

	if(bLineBetween)
	{
		// a bit of math to avoid/replace a (nasty) switch statement
		for (UT_sint32 j = 2; j <= iColumns; j++)
		{
			UT_sint32 x = rect.left + (j-1) * rect.width / iColumns;
			painter.drawLine(x, y_start, x, y_end);
		}
	}
}

