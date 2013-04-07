/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2004-2007 Tomas Frydrych
 * Copyright (C) 2007 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#include "gr_UnixPangoPixmapGraphics.h"

#include "xap_App.h"
#include "xap_Prefs.h"
#include "xap_EncodingManager.h"
#include "xap_Strings.h"
#include "xap_Frame.h"

#include "xap_UnixApp.h"
#include "xap_UnixFrameImpl.h"
#include "xap_UnixDialogHelper.h"

#include "gr_UnixImage.h"

#include "ut_debugmsg.h"
#include "ut_misc.h"
#include "ut_vector.h"
#include "ut_locale.h"

// need this to include what Pango considers 'low-level' api
#define PANGO_ENABLE_ENGINE

#include <pango/pango-item.h>
#include <pango/pango-engine.h>

#include <math.h>

#include <gdk/gdk.h>

/*!
 * The GR_UnixPangoPixmapGraphics is used to draw onto an offscreen 
 * buffer.
 */

GR_UnixPangoPixmapGraphics::GR_UnixPangoPixmapGraphics(GdkPixmap * pix): 
	GR_UnixCairoGraphics(GDK_DRAWABLE (pix)),
	m_pPixmap(pix)
{}			

GR_UnixPangoPixmapGraphics::~GR_UnixPangoPixmapGraphics(void)
{
	gdk_pixmap_unref (m_pPixmap);	
}

GR_Graphics *   GR_UnixPangoPixmapGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	UT_return_val_if_fail(info.getType() == GRID_UNIX_PANGO_PIXMAP,NULL);
	xxx_UT_DEBUGMSG(("GR_UnixPangoPixmapGraphics::graphicsAllocator\n"));

	GR_UnixPixmapAllocInfo &AI = (GR_UnixPixmapAllocInfo&)info;

	return new GR_UnixPangoPixmapGraphics(AI.m_pix);
}

bool GR_UnixPangoPixmapGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
		case DGP_SCREEN:
			return false;
		case DGP_OPAQUEOVERLAY:
			return true;
		case DGP_PAPER:
			return true;
		default:
			UT_ASSERT(0);
			return false;
	}
}

GR_Graphics::Cursor GR_UnixPangoPixmapGraphics::getCursor(void) const
{
	return GR_CURSOR_DEFAULT;
}
