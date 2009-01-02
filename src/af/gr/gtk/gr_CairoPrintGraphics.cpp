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

static void _JobOver(GtkPrintJob *,void *,GError *)
{
	printf("Print Job is Finished \n");
}

GR_CairoPrintGraphics::GR_CairoPrintGraphics(cairo_t *cr, UT_uint32 iDeviceResolution)
  : GR_UnixPangoGraphics(cr, iDeviceResolution),
	m_bDoShowPage(false),
	m_pJob(NULL)
{}
	
GR_CairoPrintGraphics::~GR_CairoPrintGraphics()
{
	UT_DEBUGMSG(("Deleting CairoPrint graphics %x \n",this));
}
	
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

void GR_CairoPrintGraphics::setJob(GtkPrintJob * pJob)
{
	m_pJob = pJob;
	g_object_ref(m_pJob);
}

bool GR_CairoPrintGraphics::GR_CairoPrintGraphics::startPrint(void)
{
	m_bDoShowPage = true;
	return true;
}

bool GR_CairoPrintGraphics::startPage(const char * /*szPagelabel*/, UT_uint32 /*pageNumber*/,
									  bool /*bPortrait*/, UT_uint32 /*iWidth*/, UT_uint32 /*iHeight*/)
{
	if (m_bDoShowPage) {
		cairo_show_page(m_cr);
	}

	m_bDoShowPage = true;

	return true;
}

bool GR_CairoPrintGraphics::endPrint(void)
{
	if (m_bDoShowPage) {
		cairo_show_page(m_cr);
	}
	gtk_print_job_send (m_pJob,_JobOver,NULL,NULL);
	return true;
}

