/* AbiSource Application Framework
 * Copyright (C) 1998-2002 AbiSource, Inc.
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
#include "xap_UnixFontPreview.h"
#include "gr_UnixGraphics.h"
#include "xap_UnixDialogHelper.h"

static void expose_event(GtkWidget * widget, XAP_UnixFontPreview * prev)
{
  prev->draw();
}

XAP_UnixFontPreview::XAP_UnixFontPreview(XAP_Frame * pFrame, UT_sint32 left, UT_uint32 top)
	: XAP_FontPreview()
{
	m_pFrame = (XAP_Frame *)pFrame;
	m_left = left;
	m_top = top;
	
	m_pPreviewWindow = gtk_window_new(GTK_WINDOW_POPUP);
	
	gtk_widget_set_uposition(m_pPreviewWindow, m_left, m_top);
	gtk_widget_set_size_request(m_pPreviewWindow, m_width, m_height);
	
	m_pDrawingArea = createDrawingArea ();
	gtk_widget_set_double_buffered(m_pDrawingArea, FALSE);
	gtk_drawing_area_size(GTK_DRAWING_AREA(m_pDrawingArea), m_pPreviewWindow->allocation.width, m_pPreviewWindow->allocation.height);
	gtk_container_add(GTK_CONTAINER(m_pPreviewWindow), m_pDrawingArea);

	gtk_widget_show_all(m_pPreviewWindow);

#if 0
	g_signal_connect(G_OBJECT(m_pDrawingArea), "expose_event",
			 G_CALLBACK(expose_event), this);
#endif

	XAP_App *pApp = m_pFrame->getApp();
#ifndef WITH_PANGO
	m_gc = new GR_UnixGraphics(GTK_WIDGET(m_pDrawingArea)->window, static_cast<XAP_UnixApp*>(pApp)->getFontManager(), pApp);	
#else
	m_gc = new GR_UnixGraphics(GTK_WIDGET(m_pDrawingArea)->window, pApp);
#endif
	
	_createFontPreviewFromGC(m_gc, m_pPreviewWindow->allocation.width, m_pPreviewWindow->allocation.height);
}

XAP_UnixFontPreview::~XAP_UnixFontPreview(void)
{
	DELETEP(m_gc);
	gtk_widget_destroy(m_pDrawingArea);
	gtk_widget_destroy(m_pPreviewWindow);
}
