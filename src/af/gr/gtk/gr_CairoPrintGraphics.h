/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2008 Robert Staudinger
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

#ifndef GR_CAIROPRINTGRAPHICS_H
#define GR_CAIROPRINTGRAPHICS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gr_UnixPangoGraphics.h"

class ABI_EXPORT GR_CairoPrintGraphics : public GR_UnixPangoGraphics
{
	friend class GR_UnixPangoFont;
public:
	GR_CairoPrintGraphics(cairo_t *cr);
	
	virtual ~GR_CairoPrintGraphics();

	static UT_uint32 s_getClassId() {return GRID_UNIX_PANGO_PRINT;}
	virtual UT_uint32 getClassId() {return s_getClassId();}
	
	virtual GR_Capability  getCapability() {return GRCAP_PRINTER_ONLY;}
	static const char *    graphicsDescriptor(){return "Unix Cairo Print";}
	
	virtual bool queryProperties(GR_Graphics::Properties gp) const;
	
	virtual bool startPrint(void);
	virtual bool startPage(const char * szPagelabel, UT_uint32 pageNumber,
							  bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);
	virtual bool endPrint(void);

	virtual void setCursor(GR_Graphics::Cursor c) { UT_ASSERT_NOT_REACHED(); }
	virtual GR_Graphics::Cursor getCursor(void) const { UT_ASSERT_NOT_REACHED(); }

	virtual void setPageSize(char* pageSizeName, UT_uint32 iwidth = 0, UT_uint32 iheight=0) { UT_ASSERT_NOT_REACHED(); }

    virtual GR_Image * genImageFromRectangle(const UT_Rect & /*r*/) { UT_ASSERT_NOT_REACHED(); return NULL;}
	virtual void	  saveRectangle(UT_Rect & /*r*/, UT_uint32 /*iIndx*/) { UT_ASSERT_NOT_REACHED(); }
	virtual void	  restoreRectangle(UT_uint32 /*iIndx*/) { UT_ASSERT_NOT_REACHED(); }

	virtual bool      canQuickPrint(void) { return true;}
};

#endif

