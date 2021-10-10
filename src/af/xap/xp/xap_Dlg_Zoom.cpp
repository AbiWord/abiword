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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "xap_Dlg_Zoom.h"
#include "xap_Preview_Zoom.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "xav_View.h"

XAP_Dialog_Zoom::XAP_Dialog_Zoom(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogzoom")
{
	m_answer = a_OK;

	// this should really never appear, since setZoomPercent()
	// should always be called before the dialog is shown
	m_zoomPercent = 100;

	m_zoomPreview = NULL;
	m_pFrame = nullptr;
}

XAP_Dialog_Zoom::~XAP_Dialog_Zoom(void)
{
	DELETEP(m_zoomPreview);
}

XAP_Dialog_Zoom::tAnswer XAP_Dialog_Zoom::getAnswer(void) const
{
	// let our caller know if user hit ok, cancel, etc.
	return m_answer;
}

void XAP_Dialog_Zoom::setZoomPercent(UT_uint32 zoom)
{
	// store the percentage within limits clipping if necessary
	if      (zoom < XAP_DLG_ZOOM_MINIMUM_ZOOM) m_zoomPercent = XAP_DLG_ZOOM_MINIMUM_ZOOM;
	else if (zoom > XAP_DLG_ZOOM_MAXIMUM_ZOOM) m_zoomPercent = XAP_DLG_ZOOM_MAXIMUM_ZOOM;
	else                                       m_zoomPercent = zoom;
}	

XAP_Frame::tZoomType XAP_Dialog_Zoom::getZoomType(void)
{
	return m_zoomType;
}

UT_uint32 XAP_Dialog_Zoom::getZoomPercent(void)
{
  
	// we deliver based on special cases first, then the custom percentage
	switch(m_zoomType)
	{
	case XAP_Frame::z_200:
		return 200;
	case XAP_Frame::z_100:
		return 100;
	case XAP_Frame::z_75:
		return 75;
	case XAP_Frame::z_PAGEWIDTH:
	  if ( m_pFrame )
	    return m_pFrame->getCurrentView ()->calculateZoomPercentForPageWidth () ;
          break;
	case XAP_Frame::z_WHOLEPAGE:
	  if ( m_pFrame )
	    return m_pFrame->getCurrentView ()->calculateZoomPercentForWholePage () ;
          break;
	case XAP_Frame::z_PERCENT:
		// fall through
	default:
		if (m_zoomPercent > XAP_DLG_ZOOM_MINIMUM_ZOOM)
			return m_zoomPercent;
		else
			return XAP_DLG_ZOOM_MINIMUM_ZOOM;
	}

	// fallback
	return 100 ;
}

/************************************************************************/

void XAP_Dialog_Zoom::_updatePreviewZoomPercent(UT_uint32 percent)
{
	if (m_zoomPreview)
	{
		m_zoomPreview->setZoomPercent(percent);
		m_zoomPreview->queueDraw();
	}
	if (m_pFrame)
		m_pFrame->quickZoom(percent);
}

void XAP_Dialog_Zoom::_createPreviewFromGC(GR_Graphics * gc,
										   UT_uint32 width,
										   UT_uint32 height)
{
	UT_ASSERT(gc);

	m_zoomPreview = new XAP_Preview_Zoom(gc);
	UT_ASSERT(m_zoomPreview);
	
	m_zoomPreview->setWindowSize(width, height);
	m_zoomPreview->setString("10-pt Times New Roman");
	m_zoomPreview->setFont(XAP_Preview_Zoom::font_NORMAL);
	m_zoomPreview->setZoomPercent(m_zoomPercent);

}
