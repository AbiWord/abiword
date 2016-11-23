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

#include <string.h>

#include "gr_Image.h"
#include "ut_bytebuf.h"
#include "ut_svg.h"
#include "ut_assert.h"
#include <math.h>

GR_Image::GR_Image()
  : m_iDisplayWidth(0), m_iDisplayHeight(0)
{
}

GR_Image::~GR_Image()
{
  DestroyOutline();
}

void GR_Image::getName(char* p) const
{
	UT_ASSERT(p);
	
	strcpy(p, m_szName.c_str());
}

void GR_Image::getName ( std::string & name) const
{
	name = m_szName;
}

void GR_Image::getName ( UT_String & copy ) const
{
  // assign
  copy = m_szName;
}

/*!
 * Scale our image to rectangle given by rec. The dimensions of rec
 * are calculated in logical units.
 * Overriden by platform implementation if needed. Default is to set 
 * display size.
 */
void GR_Image::scaleImageTo(GR_Graphics * pG, const UT_Rect & rec)
{
	setDisplaySize(pG->tdu(rec.width), pG->tdu(rec.height));
}

void GR_Image::setName ( const char * name )
{
	// passing NULL isn't valid, BUT we have make sure it will not cause trouble.
	UT_ASSERT_HARMLESS(name);
	m_szName = (name ? name : "Image");
}

/*!
 * Return the distance from the left side of the image that is "pad" distance
 * to the nearest point in the transparent outline from the line segment
 * start at Y and running for distance Height below it.
 * All distances are in logical units.
 *                -----------------
 *                |               |
 *                |      *        |
 *                |    *****      |
 *             ||||||   ***       |
 *                | |    **       |
 *                |---------------|
 *                | |
 *                | |
 *
 * This case would give a -ve distance.
 * The input yTop is in logical units as measured from the top of the image.
 * If y is above the image it should be negative.
 * The returned value is in logical units.
 */
UT_sint32 GR_Image::GetOffsetFromLeft(GR_Graphics * pG, UT_sint32 pad, UT_sint32 yTop, UT_sint32 height)
{
  if(!hasAlpha())
  {
    return pad;
  }
  if(!isOutLinePresent())
  {
    GenerateOutline();
  }
  double maxDist = -10000000.0;
  double d = 0.0;
  UT_uint32 i = 0;
  double ddPad = static_cast<double>(pG->tdu(pad));
  UT_sint32 diTop = pG->tdu(yTop);
  UT_sint32 diHeight = pG->tdu(height);
  double ddTop = static_cast<double>(diTop);
  double ddHeight = static_cast<double>(diHeight);
  GR_Image_Point * pPoint = NULL;
  UT_uint32 nPts = m_vecOutLine.getItemCount()/2;
  for(i=0; i < nPts;i++)
  {
    pPoint = m_vecOutLine.getNthItem(i);
    if((pPoint->m_iY >= diTop) && (pPoint->m_iY <= (yTop + diHeight)))
    {
      d = ddPad - static_cast<double>(pPoint->m_iX);
    }
    else
    {
      double y = ddTop + ddHeight;
      if(abs(pPoint->m_iY - diTop) < abs(pPoint->m_iY - (diTop + diHeight)))
      {
	//
	// Calculate from top point.
	//
	y = ddTop;
      }
      double dYP = static_cast<double>(pPoint->m_iY);
      double root = ddPad*ddPad - (y - dYP)*(y - dYP);
      if(root < 0.0)
      {
	//
	// This point doesn't overlap at all
	//
	  d = -10000000.0;
      }
      else
      {
	d = -static_cast<double>(pPoint->m_iX) - sqrt(root); 
      }
    }
    if(d > maxDist)
    {
      maxDist = d;
    }
  }
  if(maxDist < -9999999.)
  {
    maxDist = static_cast<double>(-getDisplayWidth());
  }
  return pG->tlu(static_cast<UT_sint32>(maxDist));
}


/*!
 * Return the distance from the right side of the image that is "pad" distance
 * to the nearest point in the transparent outline from the line segment
 * start at Y and running for distance Height below it.
 * All distances are in logical units.
 *                -----------------
 *                |               |
 *                |      *        |
 *                |    *****      |
 *                |     ***    ||||||||
 *                |      **    |  |
 *                |------------|--|
 *                             |  |
 *                             |  |
 *                This distance is negative
 * The input yTop is in logical units as measured from the top of the image.
 * If y is above the image it should be negative.
 * The returned value is in logical units.
 */
UT_sint32 GR_Image::GetOffsetFromRight(GR_Graphics * pG, UT_sint32 pad, UT_sint32 yTop, UT_sint32 height)
{
  if(!hasAlpha())
  {
    return pad;
  }
  if(!isOutLinePresent())
  {
    GenerateOutline();
  }
  double maxDist = -10000000.0;
  double d = 0.0;
  UT_sint32 i = 0;
  double ddPad = static_cast<double>(pG->tdu(pad));
  UT_sint32 diTop = pG->tdu(yTop);
  UT_sint32 diHeight = pG->tdu(height);
  double ddTop = static_cast<double>(diTop);
  double ddHeight = static_cast<double>(diHeight);
  GR_Image_Point * pPoint = NULL;
  UT_sint32 nPts = m_vecOutLine.getItemCount()/2;
  for(i=nPts; i < m_vecOutLine.getItemCount();i++)
  {
    pPoint = m_vecOutLine.getNthItem(i);
    if((pPoint->m_iY >= diTop) && (pPoint->m_iY <= (diTop + diHeight)))
    {
         d = ddPad - static_cast<double>(getDisplayWidth() - pPoint->m_iX);
	 xxx_UT_DEBUGMSG(("Got Center distance %f \n",d));
    }
    else
    {
         double y = ddTop + ddHeight;
	 if(abs(pPoint->m_iY - diTop) < abs(pPoint->m_iY - (diTop + diHeight)))
         {
	      //
	      // Calculate from top point.
	      //
	   y = ddTop;
	 }
	 double dYP = static_cast<double>(pPoint->m_iY);
	 double root = ddPad*ddPad - (y - dYP)*(y - dYP);
	 if(root < 0.0)
         {
	      //
	      // This point doesn't overlap at all
	      //
	      d = -10000000.0;
	 }
	 else
         {
	      d = static_cast<double>(pPoint->m_iX) - (static_cast<double>(getDisplayWidth())) + sqrt(root); 
	      xxx_UT_DEBUGMSG(("Got Projected distance %f \n",d));
	 }
    }
    if(d > maxDist)
    {
         maxDist = d;
    }
  }
  if(maxDist < -9999999.)
  {
    maxDist = static_cast<double>(-getDisplayWidth());
  }
  return pG->tlu(static_cast<UT_sint32>(maxDist));
}

/*!
 * Generate an outline of an image with transparency. This is a collection
 * of (x,y) points marking the first non-transparent point from the left
 * and right side of the image.
 * This outline is used by GetOffsetFromLeft and facitates "tight" 
 * text wrapping
 * around objects.
 */
void GR_Image::GenerateOutline(void)
{
  DestroyOutline();
  UT_sint32 width = getDisplayWidth();
  UT_sint32 height = getDisplayHeight();
  UT_sint32 i,j=0;
  //
  // Generate from left
  //
  for(i=0; i< height;i++)
  {
    for(j =0; j< width;j++)
    {
      if(!isTransparentAt(j,i))
      {
	break;
      }
    }
    if( j < width)
    {
      GR_Image_Point * pXY = new GR_Image_Point();
      pXY->m_iX = j;
      pXY->m_iY = i;
      m_vecOutLine.addItem(pXY);
    }
  }
  //
  // Generate from Right
  //
  for(i=0; i< height;i++)
  {
    for(j =width-1; j>= 0;j--)
    {
      if(!isTransparentAt(j,i))
      {
	break;
      }
    }
    if( j >= 0)
    {
      GR_Image_Point * pXY = new GR_Image_Point();
      pXY->m_iX = j;
      pXY->m_iY = i;
      m_vecOutLine.addItem(pXY);
    }
  }
}

/*!
 * Destroy the outline
 */
void GR_Image::DestroyOutline(void)
{
  UT_VECTOR_PURGEALL(GR_Image_Point *, m_vecOutLine);
}

void GR_Image::setName ( const UT_String & name )
{
  m_szName = name.c_str();
}

GR_Image::GRType GR_Image::getBufferType(const UT_ConstByteBufPtr & pBB)
{
   const char * buf = reinterpret_cast<const char*>(pBB->getPointer(0));
   UT_uint32 len = pBB->getLength();

   if (len < 6) return GR_Image::GRT_Unknown;

   const char * comp1 = "\211PNG";
   const char * comp2 = "<89>PNG";
   if (!(strncmp(buf, comp1, 4)) || !(strncmp(buf, comp2, 6)))
     return GR_Image::GRT_Raster;

   if (UT_SVG_recognizeContent (buf,len))
     return GR_Image::GRT_Vector;

   return GR_Image::GRT_Unknown;
}

/*!
Sets the display dimentions of the image. This does not perform any scaling 
on the image, it just stores the values. Values in device units.
*/
void GR_Image::setDisplaySize(UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight) 
{ 
  xxx_UT_DEBUGMSG(("DisplayWidth set to %d DisplayeHeight set to %d \n",iDisplayWidth,iDisplayHeight));
  m_iDisplayWidth = iDisplayWidth; 
  m_iDisplayHeight = iDisplayHeight; 
}

/*!
Returns the width of the image. Values in in device units.
*/
UT_sint32 GR_Image::getDisplayWidth(void) const 
{ 
  return m_iDisplayWidth; 
}

/*!
Returns the height of the image. Values in in device units.
*/
UT_sint32 GR_Image::getDisplayHeight(void) const 
{ 
  return m_iDisplayHeight; 
}

GR_Image::GRType GR_Image::getType() const
{ 
//
// While this is technically the right thing to do it screws up printing on Windows and Gnome
//
//  return GRT_Unknown;
//
// FIXME: Subclasses should ensure this works.
	return GRT_Raster;
}

bool GR_Image::render(GR_Graphics * /*pGR*/, UT_sint32 /*iDisplayWidth*/, 
					  UT_sint32 /*iDisplayHeight*/)
{ 
  UT_ASSERT_NOT_REACHED ();
  return false; 
}
