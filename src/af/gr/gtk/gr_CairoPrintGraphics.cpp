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

#include "gr_CairoPrintGraphics.h"

GR_CairoPrintGraphics::GR_CairoPrintGraphics(cairo_t *cr)
  : GR_UnixPangoGraphics(cr),
	m_bDoShowPage(false)
{}
	
GR_CairoPrintGraphics::~GR_CairoPrintGraphics()
{}
	
bool GR_CairoPrintGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
		case DGP_SCREEN:
		case DGP_OPAQUEOVERLAY:
			return false;
		case DGP_PAPER:
			return true;
		default:
			UT_ASSERT_NOT_REACHED ();
			return false;
	}
}

bool GR_CairoPrintGraphics::GR_CairoPrintGraphics::startPrint(void)
{
	printf("%s() %d\n", __FUNCTION__, m_bDoShowPage);

	/* TODO */
	return true;
}

bool GR_CairoPrintGraphics::startPage(const char * /*szPagelabel*/, UT_uint32 /*pageNumber*/,
									  bool /*bPortrait*/, UT_uint32 /*iWidth*/, UT_uint32 /*iHeight*/)
{
	printf("%s() %d\n", __FUNCTION__, m_bDoShowPage);

	if (m_bDoShowPage) {
		cairo_show_page(m_cr);
	}

	m_bDoShowPage = true;

	/* TODO */
	return true;
}

bool GR_CairoPrintGraphics::endPrint(void)
{
	printf("%s() %d\n", __FUNCTION__, m_bDoShowPage);

	if (m_bDoShowPage) {
		cairo_show_page(m_cr);
	}

	/* TODO */
	return true;
}

