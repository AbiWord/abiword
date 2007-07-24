/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#include "xap_Frame.h"
#include "xap_Frame.h"
#include "ut_debugmsg.h"
#include "ap_UnixPreview_Annotation.h"
#include "gr_UnixPangoGraphics.h"
#include "xap_UnixDialogHelper.h"

AP_UnixPreview_Annotation::AP_UnixPreview_Annotation(XAP_Frame * pFrame, UT_sint32 left, UT_uint32 top)
	: AP_Preview_Annotation()
{
	m_pFrame = static_cast<XAP_Frame *>(pFrame);
	m_left = left;
	m_top = top;
	
	m_pPreviewWindow = gtk_window_new(GTK_WINDOW_POPUP);   
	gtk_widget_set_size_request(m_pPreviewWindow, m_width, m_height);
	
	m_pDrawingArea = createDrawingArea();
	gtk_widget_set_size_request(m_pDrawingArea, m_pPreviewWindow->allocation.width, m_pPreviewWindow->allocation.height);
	gtk_container_add(GTK_CONTAINER(m_pPreviewWindow), m_pDrawingArea);

	gtk_widget_show_all(m_pPreviewWindow);
	gtk_window_move(GTK_WINDOW(m_pPreviewWindow), m_left, m_top);

	XAP_App *pApp = XAP_App::getApp();
	GR_UnixAllocInfo ai(GTK_WIDGET(m_pDrawingArea)->window);
	m_gc = (GR_UnixPangoGraphics*) pApp->newGraphics(ai);

	_createAnnotationPreviewFromGC(m_gc, m_pPreviewWindow->allocation.width, m_pPreviewWindow->allocation.height);
}

AP_UnixPreview_Annotation::~AP_UnixPreview_Annotation(void)
{
	DELETEP(m_gc);
	gtk_widget_destroy(m_pDrawingArea);
	gtk_widget_destroy(m_pPreviewWindow);
}
