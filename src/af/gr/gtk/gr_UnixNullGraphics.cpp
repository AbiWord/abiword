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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <math.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_wctomb.h"
#include "xap_UnixDialogHelper.h"
#include "ut_endian.h"

#include "gr_UnixNullGraphics.h"

#include "xap_EncodingManager.h"
#include "ut_OverstrikingChars.h"

#include "xap_UnixApp.h"
#include "xap_Prefs.h"
#include "xap_App.h"

/*****************************************************************
******************************************************************
** This is a null graphics class to enable document editting with
** no GUI display. Much of the code was cut and pasted from 
** PS_Graphics so that no x resources
** are used in doing font calculations.
******************************************************************
*****************************************************************/

UnixNull_Graphics::UnixNull_Graphics()
  : GR_UnixPangoGraphics()
{
}

UnixNull_Graphics::~UnixNull_Graphics()
{
	// TODO g_free stuff
	_destroyFonts ();
}

GR_Image* UnixNull_Graphics::createNewImage(const char* pszName, const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType)
{
	return NULL;
}


bool UnixNull_Graphics::queryProperties(GR_Graphics::Properties gp) const
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

void UnixNull_Graphics::setColor(const UT_RGBColor& clr)
{
}

void UnixNull_Graphics::getColor(UT_RGBColor& clr)
{
}

GR_Font* UnixNull_Graphics::getGUIFont()
{
	return NULL;
}

void UnixNull_Graphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
				  int iLength, UT_sint32 xoff, UT_sint32 yoff,
				  int * pCharWidths)
{
}

void UnixNull_Graphics::drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{
}

void UnixNull_Graphics::setLineWidth(UT_sint32 iLineWidth)
{
}

void UnixNull_Graphics::xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32)
{
}

void UnixNull_Graphics::polyLine(UT_Point * /* pts */, UT_uint32 /* nPoints */)
{
}

void UnixNull_Graphics::fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
}

void UnixNull_Graphics::invertRect(const UT_Rect* /*pRect*/)
{
}

void UnixNull_Graphics::setClipRect(const UT_Rect* r)
{
}

void UnixNull_Graphics::clearArea(UT_sint32 /*x*/, UT_sint32 /*y*/,
							UT_sint32 /*width*/, UT_sint32 /*height*/)
{

}

void UnixNull_Graphics::scroll(UT_sint32, UT_sint32)
{

}

void UnixNull_Graphics::scroll(UT_sint32 /* x_dest */,
			       UT_sint32 /* y_dest */,
			       UT_sint32 /* x_src */,
			       UT_sint32 /* y_src */,
			       UT_sint32 /* width */,
			       UT_sint32 /* height */)
{
}

bool UnixNull_Graphics::startPrint(void)
{
	return true;
}

bool UnixNull_Graphics::startPage(const char * szPageLabel, UT_uint32 pageNumber,
				  bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight)
{
	return true;
}

bool UnixNull_Graphics::endPrint(void)
{
	return true;
}

/*****************************************************************/
/*****************************************************************/

void UnixNull_Graphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
}
	
void UnixNull_Graphics::drawRGBImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
}

void UnixNull_Graphics::drawGrayImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	
}
void UnixNull_Graphics::drawBWImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
}

void UnixNull_Graphics::setColorSpace(GR_Graphics::ColorSpace c)
{
}

GR_Graphics::ColorSpace UnixNull_Graphics::getColorSpace(void) const
{
  return GR_COLORSPACE_COLOR;
}

void UnixNull_Graphics::setCursor(GR_Graphics::Cursor c)
{
}

GR_Graphics::Cursor UnixNull_Graphics::getCursor(void) const
{
	return GR_CURSOR_INVALID;
}

void UnixNull_Graphics::setColor3D(GR_Color3D /*c*/)
{

}

UT_RGBColor * UnixNull_Graphics::getColor3D(GR_Color3D /*c*/)
{
	return NULL;
}

void UnixNull_Graphics::fillRect(GR_Color3D /*c*/, UT_sint32 /*x*/, UT_sint32 /*y*/, UT_sint32 /*w*/, UT_sint32 /*h*/)
{
}

void UnixNull_Graphics::fillRect(GR_Color3D /*c*/, UT_Rect & /*r*/)
{
}

void UnixNull_Graphics::setPageSize(char* pageSizeName, UT_uint32 iwidth, UT_uint32 iheight)
{
}


GR_Graphics *   UnixNull_Graphics::graphicsAllocator(GR_AllocInfo& info)
{
	return new UnixNull_Graphics();
}
