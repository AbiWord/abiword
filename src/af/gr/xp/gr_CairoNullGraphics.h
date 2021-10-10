/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copryight (C) 2021 Hubert Figuière
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

#pragma once

#include "gr_CairoGraphics.h"
#include "gr_Image.h"
#include "ut_misc.h"

/*****************************************************************/
/*****************************************************************/
class ABI_EXPORT GR_CairoNullGraphicsAllocInfo : public GR_AllocInfo
{
public:
 	GR_CairoNullGraphicsAllocInfo()
	  {}

	virtual GR_GraphicsId getType() const override {return GRID_CAIRO_NULL;}
	virtual bool isPrinterGraphics() const override {return false;}
};



class ABI_EXPORT CairoNull_Graphics : public GR_CairoGraphics
{
	// all constructors are protected; instances must be created via
	// GR_GraphicsFactory
public:
	virtual ~CairoNull_Graphics();

	static UT_uint32 s_getClassId() {return GRID_CAIRO_NULL;}
	virtual UT_uint32 getClassId() override {return s_getClassId();}

	virtual GR_Capability getCapability() override {UT_ASSERT(UT_NOT_IMPLEMENTED); return GRCAP_UNKNOWN;}
	static const char *    graphicsDescriptor(void) { return "Cairo Null Graphics";}
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&);

	virtual void drawChars(const UT_UCSChar* pChars,
			       int iCharOffset, int iLength,
			       UT_sint32 xoff, UT_sint32 yoff,
			       int * pCharWidths = NULL) override;

	virtual void setColor(const UT_RGBColor& clr) override;
	virtual void getColor(UT_RGBColor& clr) override;
	virtual GR_Font* getGUIFont() override;

	virtual void drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2) override;
	virtual void setLineWidth(UT_sint32) override;
	virtual void xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32) override;
	virtual void polyLine(const UT_Point * pts, UT_uint32 nPoints) override;
	virtual void fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h) override;
	virtual void invertRect(const UT_Rect*) override;
	virtual void queueDraw(const UT_Rect* pRect) override;
	virtual void setClipRect(const UT_Rect*) override;
	virtual void scroll(UT_sint32, UT_sint32) override;
	virtual void scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						UT_sint32 x_src, UT_sint32 y_src,
						UT_sint32 width, UT_sint32 height) override;
	virtual void clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32) override;

	virtual void drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest) override;
	virtual void drawRGBImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
	virtual void drawGrayImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
	virtual void drawBWImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
	virtual GR_Image* createNewImage(const char* pszName, const UT_ConstByteBufPtr & pBBPNG, const std::string& mimetype, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType = GR_Image::GRT_Raster) override;

	virtual bool queryProperties(GR_Graphics::Properties gp) const override;

	virtual bool startPrint(void) override;
	virtual bool startPage(const char * szPagelabel, UT_uint32 pageNumber,
							  bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight) override;
	virtual bool endPrint(void) override;

	virtual void setColorSpace(GR_Graphics::ColorSpace c) override;
	virtual GR_Graphics::ColorSpace getColorSpace(void) const override;

	virtual void setCursor(GR_Graphics::Cursor c) override;
	virtual GR_Graphics::Cursor getCursor(void) const override;

	virtual void setColor3D(GR_Color3D c) override;
	virtual bool getColor3D(GR_Color3D c, UT_RGBColor &color) override;
	virtual void fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h) override;
	virtual void fillRect(GR_Color3D c, UT_Rect &r) override;
	virtual void setPageSize(char* pageSizeName, UT_uint32 iwidth = 0, UT_uint32 iheight=0);
	virtual void setPageCount(UT_uint32 /*iCount*/) {}

	virtual GR_Image * genImageFromRectangle(const UT_Rect & /*r*/) override { return NULL;}
	virtual void	  saveRectangle(UT_Rect & /*r*/, UT_uint32 /*iIndx*/) override {}
	virtual void	  restoreRectangle(UT_uint32 /*iIndx*/) override {}

	virtual void setLineProperties(double inWidth,
					 GR_Graphics::JoinStyle inJoinStyle = JOIN_MITER,
					 GR_Graphics::CapStyle inCapStyle   = CAP_BUTT,
					 GR_Graphics::LineStyle inLineStyle = LINE_SOLID) override;

protected:
	// all instances have to be created via GR_GraphicsFactory; see gr_Graphics.h
	CairoNull_Graphics();
};
