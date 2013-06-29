/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2004-2006 Tomas Frydrych <dr.tomas@yahoo.co.uk>
 * Copyright (C) 2009 Hubert Figuiere
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

#include "ut_bytebuf.h"

#include "ut_debugmsg.h"
#include "ut_misc.h"
#include "ut_locale.h"
#include "ut_std_string.h"
#include "ut_std_vector.h"

#include "gr_QtGraphics.h"
#include "gr_CairoGraphics.h"

#include "xap_App.h"
#include "xap_EncodingManager.h"

// need this to include what Pango considers 'low-level' api
#define PANGO_ENABLE_ENGINE

#include <pango/pango-item.h>
#include <pango/pango-engine.h>

GR_QtGraphics::GR_QtGraphics()
{

}

GR_QtGraphics::~GR_QtGraphics()
{

}

GR_Graphics * GR_QtGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	UT_return_val_if_fail(info.getType() == GRID_QT, NULL);
	xxx_UT_DEBUGMSG(("GR_QtGraphics::graphicsAllocator\n"));

	return new GR_QtGraphics();
}

GR_Font * GR_QtGraphics::getGUIFont(void)
{
	return NULL;
}

void GR_QtGraphics::scroll(UT_sint32, UT_sint32)
{
	return;
}

void GR_QtGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
			UT_sint32 x_src, UT_sint32 y_src,
			UT_sint32 width, UT_sint32 height)
{
	return;
}

void GR_QtGraphics::setCursor(Cursor c)
{
	return;
}

GR_Image *  GR_QtGraphics::genImageFromRectangle(const UT_Rect & r)
{
	return NULL;
}

