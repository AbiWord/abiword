/* AbiSource Application Framework
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

#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "gr_CairoNullGraphics.h"


/*****************************************************************
******************************************************************
** This is a null graphics class to enable document editting with
** no GUI display. Much of the code was cut and pasted from 
** PS_Graphics so that no x resources
** are used in doing font calculations.
******************************************************************
*****************************************************************/

CairoNull_Graphics::CairoNull_Graphics()
  : GR_CairoGraphics()
{
}

CairoNull_Graphics::~CairoNull_Graphics()
{
	// TODO g_free stuff
	_destroyFonts ();
}

GR_Image* CairoNull_Graphics::createNewImage(const char* /*pszName*/, 
					     const UT_ConstByteBufPtr & /*pBB*/,
					     const std::string& /*mimetype*/,
					     UT_sint32 /*iDisplayWidth*/, 
					     UT_sint32 /*iDisplayHeight*/,
					     GR_Image::GRType /*iType*/)
{
	return NULL;
}


bool CairoNull_Graphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_PAPER:
		return true;
	case DGP_SCREEN:
	case DGP_OPAQUEOVERLAY:
	default:
		return false;
	}
}

void CairoNull_Graphics::setColor(const UT_RGBColor& )
{
}

void CairoNull_Graphics::getColor(UT_RGBColor& )
{
}

GR_Font* CairoNull_Graphics::getGUIFont()
{
	return NULL;
}

void CairoNull_Graphics::drawChars(const UT_UCSChar* /*pChars*/, int /*iCharOffset*/,
								  int /*iLength*/, UT_sint32 /*xoff*/, UT_sint32 /*yoff*/,
								  int * /*pCharWidths*/)
{
}

void CairoNull_Graphics::drawLine(UT_sint32 /*x1*/, UT_sint32 /*y1*/, 
								 UT_sint32 /*x2*/, UT_sint32 /*y2*/)
{
}

void CairoNull_Graphics::setLineWidth(UT_sint32 )
{
}

void CairoNull_Graphics::xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32)
{
}

void CairoNull_Graphics::polyLine(const UT_Point * /* pts */, UT_uint32 /* nPoints */)
{
}

void CairoNull_Graphics::fillRect(const UT_RGBColor&, UT_sint32, UT_sint32, UT_sint32, 
								 UT_sint32)
{
}

void CairoNull_Graphics::invertRect(const UT_Rect* /*pRect*/)
{
}

void CairoNull_Graphics::setClipRect(const UT_Rect* )
{
}

void CairoNull_Graphics::clearArea(UT_sint32 /*x*/, UT_sint32 /*y*/,
							UT_sint32 /*width*/, UT_sint32 /*height*/)
{

}

void CairoNull_Graphics::scroll(UT_sint32, UT_sint32)
{

}

void CairoNull_Graphics::scroll(UT_sint32 /* x_dest */,
			       UT_sint32 /* y_dest */,
			       UT_sint32 /* x_src */,
			       UT_sint32 /* y_src */,
			       UT_sint32 /* width */,
			       UT_sint32 /* height */)
{
}

bool CairoNull_Graphics::startPrint(void)
{
	return true;
}

bool CairoNull_Graphics::startPage(const char *, UT_uint32,
				  bool, UT_uint32, UT_uint32)
{
	return true;
}

bool CairoNull_Graphics::endPrint(void)
{
	return true;
}

/*****************************************************************/
/*****************************************************************/

void CairoNull_Graphics::drawImage(GR_Image*, UT_sint32, UT_sint32)
{
}
	
void CairoNull_Graphics::drawRGBImage(GR_Image*, UT_sint32, UT_sint32)
{
}

void CairoNull_Graphics::drawGrayImage(GR_Image*, UT_sint32, UT_sint32)
{
	
}
void CairoNull_Graphics::drawBWImage(GR_Image*, UT_sint32, UT_sint32)
{
}

void CairoNull_Graphics::setColorSpace(GR_Graphics::ColorSpace)
{
}

GR_Graphics::ColorSpace CairoNull_Graphics::getColorSpace(void) const
{
  return GR_COLORSPACE_COLOR;
}

void CairoNull_Graphics::setCursor(GR_Graphics::Cursor)
{
}

GR_Graphics::Cursor CairoNull_Graphics::getCursor(void) const
{
	return GR_CURSOR_INVALID;
}

void CairoNull_Graphics::setColor3D(GR_Color3D /*c*/)
{

}

bool CairoNull_Graphics::getColor3D(GR_Color3D /*c*/, UT_RGBColor &)
{
	return false;
}

void CairoNull_Graphics::fillRect(GR_Color3D /*c*/, UT_sint32 /*x*/, UT_sint32 /*y*/, UT_sint32 /*w*/, UT_sint32 /*h*/)
{
}

void CairoNull_Graphics::fillRect(GR_Color3D /*c*/, UT_Rect & /*r*/)
{
}

void CairoNull_Graphics::setPageSize(char*, UT_uint32, UT_uint32)
{
}

void CairoNull_Graphics::setLineProperties(double /*inWidth*/, 
					 GR_Graphics::JoinStyle /*inJoinStyle*/,
					 GR_Graphics::CapStyle /*inCapStyle*/,
					 GR_Graphics::LineStyle /*inLineStyle*/)
{
}

GR_Graphics *   CairoNull_Graphics::graphicsAllocator(GR_AllocInfo&)
{
	return new CairoNull_Graphics();
}
