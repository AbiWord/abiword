/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#include "xap_Dlg_Image.h"

XAP_Dialog_Image::XAP_Dialog_Image(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent (pDlgFactory, id), 
	m_width(0), 
	m_height(0),
    m_maxWidth (0), 
	m_maxHeight(0), 
	m_answer(a_OK),	
	m_HeightString("0.0in"),
	m_WidthString("0.0in"),
	m_bHeightChanged(false),
	m_bWidthChanged(false),
	m_PreferedUnits(DIM_IN)
{
}

XAP_Dialog_Image::~XAP_Dialog_Image ()
{
}


/*!
 * Returns the dimensioned string that defines the height of the
 * Image.
\returns const char * dimensioned string which is the height of the column.
*/ 
const char * XAP_Dialog_Image::getHeightString(void)
{
	return m_HeightString.c_str(); 
}

/*!
 * Returns the dimensioned string that defines the height of the
 * Image.
\returns const char * dimensioned string which is the height of the column.
*/ 
const char * XAP_Dialog_Image::getWidthString(void)
{
	return m_WidthString.c_str(); 
}


/*!
 * Returns the increment associated with the dimension defined in the string.
\params const char * sz the dimensioned string.
\returns double -  the increment associated with the dimension in sz
*/ 
double XAP_Dialog_Image::getIncrement(const char * sz)
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
 * Increment the member variable UT_String defining the dimensioned string
 * for the image height.
 */
void XAP_Dialog_Image::incrementHeight(bool bIncrement)
{
	double inc = getIncrement(m_HeightString.c_str());
	if(!bIncrement)
	{
		inc = -inc;
	}
	UT_Dimension dim = UT_determineDimension(getHeightString(), DIM_none);
	m_HeightString = UT_incrementDimString(m_HeightString.c_str(),inc);
	double dum = UT_convertToInches(getHeightString());
	m_height = dum*72.0;
	if(dum < 0.0)
	{
		m_HeightString = UT_convertInchesToDimensionString(dim,0.0);
	}
	else if((dum*72.0) > m_maxHeight)
	{
		m_height = m_maxHeight;
		dum = (m_maxHeight - 1)/72.0;
		m_HeightString = UT_convertInchesToDimensionString(dim,dum);
	}
}

/*!
 * Increment the member variable UT_String defining the dimensioned string
 * for the image width.
 */
void XAP_Dialog_Image::incrementWidth(bool bIncrement)
{
	double inc = getIncrement(m_WidthString.c_str());
	if(!bIncrement)
	{
		inc = -inc;
	}
	UT_Dimension dim = UT_determineDimension(getWidthString(), DIM_none);
	m_WidthString = UT_incrementDimString(m_WidthString.c_str(),inc);
	double dum = UT_convertToInches(getWidthString());
	m_width = dum*72.0;
	if(dum < 0.0)
	{
		m_width = 0.0;
		m_WidthString = UT_convertInchesToDimensionString(dim,0.0);
	}
	else if((dum*72.0) > m_maxWidth)
	{
		m_width = m_maxWidth;
		dum = (m_maxWidth - 1)/72.0;
		m_WidthString = UT_convertInchesToDimensionString(dim,dum);
	}
}


/*!
 * Set the member string variable m_HeightString
\params const char * szHeight is the string containing the new value
*/
void XAP_Dialog_Image::setHeight(const char * szHeight)
{
	UT_Dimension dim = UT_determineDimension(szHeight, DIM_none);
	if(dim != DIM_none)
	{
		m_bHeightChanged = true;
		m_HeightString = szHeight;
		double dum = UT_convertToInches(getHeightString());
		m_height = dum*72.0;
		if(dum < 0.0)
		{
			m_height = 0.0;
			m_HeightString = UT_convertInchesToDimensionString(dim,0.0);
		}
		else if((dum*72.0) > m_maxHeight)
		{
			m_height = m_maxHeight;
			dum = (m_maxHeight - 1)/72.0;
			m_HeightString = UT_convertInchesToDimensionString(dim,dum);
		}
	}
}


/*!
 * Set the member string variable m_HeightString
\params const char * szWidth is the string containing the new value
*/
void XAP_Dialog_Image::setWidth(const char * szWidth)
{
	UT_Dimension dim = UT_determineDimension(szWidth, DIM_none);
	if(dim != DIM_none)
	{
		m_bWidthChanged = true;
		m_WidthString = szWidth;
		double dum = UT_convertToInches(getWidthString());
		m_width = dum*72.0;
		if(dum < 0.0)
		{
			m_width = 0.0;
			m_WidthString = UT_convertInchesToDimensionString(dim,0.0);
		}
		else if((dum*72.0) > m_maxWidth)
		{
			m_width = m_maxWidth;
			dum = (m_maxWidth - 1)/72.0;
			m_WidthString = UT_convertInchesToDimensionString(dim,dum);
		}
			
	}
}


/*!
 * Set the member string variable m_WidthString from the pixel value of 
 * the image. This is to set the initial value if it's not defined.
\params UT_sint32 iWidth is the pixel width of the string.
*/
void XAP_Dialog_Image::setWidth(UT_sint32 iWidth)
{
	UT_Dimension dim = getPreferedUnits();
	double dum = ((float) iWidth)/72.0;
	m_width = dum*72.0;
	m_WidthString = UT_convertInchesToDimensionString(dim,dum);
}	

/*!
 * Set the member string variable m_HeightString from the pixel value of 
 * the image. This is to set the initial value if it's not defined.
\params UT_sint32 iHeight is the pixels height of the image.
*/
void XAP_Dialog_Image::setHeight(UT_sint32 iHeight)
{
	UT_Dimension dim = getPreferedUnits();
	double dum = ((float) iHeight)/72.0;
	m_height = dum*72.0;
	m_HeightString = UT_convertInchesToDimensionString(dim,dum);
}	


/*!
 * Converts the string sz into the units seleced for the ruler.
\params const char * sz is the string containing the old value
\params UT_String & pRet is the string to which the new value is copied.
*/
void XAP_Dialog_Image::_convertToPreferredUnits(const char *sz, UT_String & pRet)
{
	UT_Dimension PreferedUnits = getPreferedUnits();
	pRet = (const XML_Char *) UT_reformatDimensionString(PreferedUnits,sz);
}

/*!
 * Sets the prefered units for the dialog.
 */
void  XAP_Dialog_Image::setPreferedUnits(UT_Dimension dim)
{
	m_PreferedUnits = dim;
}

/*!
 * Returns the current units used for the rulers.
*/
UT_Dimension XAP_Dialog_Image::getPreferedUnits(void)
{
	return m_PreferedUnits;
}








