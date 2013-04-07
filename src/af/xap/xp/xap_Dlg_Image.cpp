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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ut_string_class.h"
#include "ut_units.h"

#include "xap_Dlg_Image.h"
#include "xap_Preview_Zoom.h"
#include "xap_Frame.h"
#include "xap_App.h"

XAP_Dialog_Image::XAP_Dialog_Image(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent (pDlgFactory, id, "interface/dialogimageproperties"), 
	m_bAspect(true),
	m_width(0), 
	m_height(0),
        m_maxWidth (0), 
	m_maxHeight(0), 
	m_answer(a_OK),	
	m_HeightString("0.0in"),
	m_WidthString("0.0in"),
	m_bHeightChanged(false),
	m_bWidthChanged(false),
	m_PreferedUnits(DIM_IN),
	m_iWrappingType(WRAP_INLINE),	
    m_iPositionTo(POSITION_TO_PARAGRAPH),
    m_bInHdrFtr(false),
    m_bTightWrap(false)
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

bool XAP_Dialog_Image::isInHdrFtr(void)
{
  return m_bInHdrFtr;
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
\param const char * sz the dimensioned string.
\returns double -  the increment associated with the dimension in sz
*/ 
double XAP_Dialog_Image::getIncrement(const char * sz)
{
	double inc = 0.1;
	UT_Dimension dim =  UT_determineDimension(sz);
	switch( dim )
	{
	default:
	case DIM_IN:
		inc = 0.02;
		break;
	case DIM_CM:
		inc = 0.1;
		break;
	case DIM_MM:
	case DIM_PI:
	case DIM_PT:
	case DIM_PX:
		inc = 1.0;
		break;
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
	
	m_HeightString = UT_incrementDimString(m_HeightString.c_str(),inc);
	setPreferedUnits( UT_determineDimension(getHeightString(), DIM_none) );
	setHeight( UT_convertToInches(getHeightString()), true );			
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

	m_WidthString = UT_incrementDimString(m_WidthString.c_str(),inc);
	setPreferedUnits( UT_determineDimension(getWidthString(), DIM_none) );
	setWidth( UT_convertToInches(getWidthString()), true );			
}


/*!
 * Set the member string variable m_HeightString
\param const char * szHeight is the string containing the new value
*/
void XAP_Dialog_Image::setHeight(const char * szHeight)
{
	UT_Dimension dim = UT_determineDimension(szHeight, DIM_none);
	if(dim != DIM_none)
	{
		m_bHeightChanged = true;
		m_HeightString = szHeight;
		setPreferedUnits( dim );
		setHeight( UT_convertToInches(getHeightString()), true );			
	}
}

/*!
 * Set the member string variable m_HeightString from the a double value of 
 * the image. 
\param double dHeight is the height of the image in inches.
*/
void XAP_Dialog_Image::setHeight(double  dHeight, bool checkaspect)
{
  if(checkaspect && m_bAspect && m_height!=0.0)
    setWidthAndHeight(dHeight, false);
  else
    {
	m_height = dHeight*72.0;
	if(m_height < 0.0)
	{
		m_height = 0.1;
		dHeight  = 0.1;
	}
	else if(m_height > m_maxHeight)
	{
		m_height = m_maxHeight;
		dHeight = (m_maxHeight - 1)/72.0;
	}

	m_HeightString = UT_convertInchesToDimensionString(getPreferedUnits(),dHeight);
    }
}

/*!
 * Set the member string variable m_HeightString
\param const char * szWidth is the string containing the new value
*/
void XAP_Dialog_Image::setWidth(const char * szWidth)
{
	UT_Dimension dim = UT_determineDimension(szWidth, DIM_none);
	if(dim != DIM_none)
	{
		m_bWidthChanged = true;
		m_WidthString = szWidth;
		setPreferedUnits( dim );
		setWidth( UT_convertToInches(getWidthString()), true );			
	}
}

/*!
 * Set the member string variable m_WidthString from the a double value of 
 * the image. 
\param double dWidth is the width of the image in inches.
*/
void XAP_Dialog_Image::setWidth(double  dWidth, bool checkaspect)
{
  if(checkaspect && m_bAspect && m_width!=0.0)
    setWidthAndHeight(dWidth, true);
  else
    {
	m_width = dWidth*72.0;
	if(m_width < 0.0)
	{
		m_width = 0.1;
		dWidth  = 0.1;
	}
	else if(m_width > m_maxWidth)
	{
		m_width = m_maxWidth;
		dWidth = (m_maxWidth - 1)/72.0;
	}
	m_WidthString = UT_convertInchesToDimensionString(getPreferedUnits(),dWidth);

    }
}


void XAP_Dialog_Image::setWidthAndHeight(double wh, bool iswidth)
{
  double orig_width,orig_height;
 
  orig_width = m_width;
  orig_height = m_height;

  if (wh < 0.1) wh=0.1;
  if (orig_width < 1.) orig_width = 1.;
  if (orig_height < 1.) orig_height = 1.;

  if (iswidth)
    {
      m_width = wh*72.0;   
      m_height = m_width*orig_height/orig_width;
    }
  else
    {
      m_height = wh*72.0;
      m_width = m_height*orig_width/orig_height;
    }

  if (m_width > m_maxWidth)
    {
      m_width = m_maxWidth;
      m_height = m_width*orig_height/orig_width;
    }

  if (m_height > m_maxHeight)
    {
      m_height = m_maxHeight;
      m_width = m_height*orig_width/orig_height;
    }


  m_WidthString = UT_convertInchesToDimensionString(getPreferedUnits(),m_width/72.0);
  m_HeightString = UT_convertInchesToDimensionString(getPreferedUnits(),m_height/72.0);

}


/*!
 * Set the member string variable m_WidthString from the pixel value of 
 * the image. This is to set the initial value if it's not defined.
\param UT_sint32 iWidth is the pixel width of the string.
*/
void XAP_Dialog_Image::setWidth(UT_sint32 iWidth)
{
	setWidth( ((double) iWidth)/72.0, false );
}	

void XAP_Dialog_Image::setWrapping(WRAPPING_TYPE iWrap)
{
	m_iWrappingType = iWrap;
}

void XAP_Dialog_Image::setPositionTo(POSITION_TO iPos)
{
	m_iPositionTo = iPos;
}

/*!
 * Set the member string variable m_HeightString from the pixel value of 
 * the image. This is to set the initial value if it's not defined.
\param UT_sint32 iHeight is the pixels height of the image.
*/
void XAP_Dialog_Image::setHeight(UT_sint32 iHeight)
{
	setHeight( ((double) iHeight)/72.0, false );
}	

/*!
 * Converts the string sz into the units seleced for the ruler.
\param const char * sz is the strinewheightng containing the old value
\param UT_String & pRet is the string to which the new value is copied.
*/
void XAP_Dialog_Image::_convertToPreferredUnits(const char *sz, UT_String & pRet)
{
	UT_Dimension PreferedUnits = getPreferedUnits();
	pRet = (const gchar *) UT_reformatDimensionString(PreferedUnits,sz);
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


