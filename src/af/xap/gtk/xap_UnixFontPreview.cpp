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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include "xap_Frame.h"
#include "ut_debugmsg.h"
#include "xap_UnixFontPreview.h"
#include "gr_UnixCairoGraphics.h"
#include "xap_UnixDialogHelper.h"

XAP_UnixFontPreview::XAP_UnixFontPreview(XAP_Frame * pFrame, UT_sint32 left, UT_uint32 top)
	: XAP_FontPreview()
{
	m_pFrame = static_cast<XAP_Frame *>(pFrame);
	m_left = left;
	m_top = top;
	
	m_pPreviewWindow = gtk_window_new(GTK_WINDOW_POPUP);   
	gtk_widget_set_size_request(m_pPreviewWindow, m_width, m_height);
	
	m_pDrawingArea = createDrawingArea ();
	gtk_container_add(GTK_CONTAINER(m_pPreviewWindow), m_pDrawingArea);
	g_object_set(G_OBJECT(m_pDrawingArea), "expand", TRUE, NULL);

	gtk_widget_show_all(m_pPreviewWindow);
	gtk_window_move(GTK_WINDOW(m_pPreviewWindow), m_left, m_top);
	UT_DEBUGMSG(("gtk_window_move left %d top %d \n",m_left,m_top));

	XAP_App *pApp = XAP_App::getApp();
	GR_UnixCairoAllocInfo ai(GTK_WIDGET(m_pDrawingArea));
	m_gc = (GR_CairoGraphics*) pApp->newGraphics(ai);

	_createFontPreviewFromGC(m_gc, m_width, m_height);
}

XAP_UnixFontPreview::~XAP_UnixFontPreview(void)
{
	DELETEP(m_gc);
	gtk_widget_destroy(m_pDrawingArea);
	gtk_widget_destroy(m_pPreviewWindow);
}
