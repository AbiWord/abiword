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

AP_UnixPreview_Annotation::AP_UnixPreview_Annotation(XAP_DialogFactory * pDlgFactory,XAP_Dialog_Id id) : AP_Preview_Annotation(pDlgFactory,id),
  m_gc(NULL),
  m_pPreviewWindow(NULL),
  m_pDrawingArea(NULL)
{
	UT_DEBUGMSG(("AP_UnixPreview_Annotation: Preview annotation for Unix platform\n"));
}

AP_UnixPreview_Annotation::~AP_UnixPreview_Annotation(void)
{
  UT_DEBUGMSG(("Preview Annotation deleted %x \n",this));
  destroy();
}

void AP_UnixPreview_Annotation::runModeless(XAP_Frame * pFrame)
{
	UT_DEBUGMSG(("Preview Annotation runModeless %x \n",this));
	setActiveFrame(pFrame);
	if(m_pPreviewWindow)
	{
		DELETEP(m_gc);
		gtk_widget_destroy(m_pDrawingArea);
		gtk_widget_destroy(m_pPreviewWindow);
		m_pPreviewWindow = NULL;
		m_pDrawingArea = NULL;
	}
	setSizeFromAnnotation();
	_constructWindow();
	gtk_window_set_modal ( GTK_WINDOW(m_pPreviewWindow), FALSE ) ;
	gtk_widget_show(m_pPreviewWindow);
	
	// make a new Unix GC
	DELETEP(m_gc);
	
	XAP_App *pApp = XAP_App::getApp();
	GR_UnixAllocInfo ai(GTK_WIDGET(m_pDrawingArea)->window);
	m_gc = pApp->newGraphics(ai);
	
	_createAnnotationPreviewFromGC(m_gc, m_pPreviewWindow->allocation.width, m_pPreviewWindow->allocation.height);
	
	gtk_widget_show(m_pDrawingArea);
}

void AP_UnixPreview_Annotation::notifyActiveFrame(XAP_Frame * /*pFrame*/)
{
	//UT_DEBUGMSG(("notifyActiveFrame: trying to activate... %x \n",this));
	//activate();
}

void AP_UnixPreview_Annotation::activate(void)
{
	UT_ASSERT(m_pPreviewWindow);
	gdk_window_raise(m_pPreviewWindow->window);
}

XAP_Dialog * AP_UnixPreview_Annotation::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return new AP_UnixPreview_Annotation(pFactory,id);
}

void  AP_UnixPreview_Annotation::_constructWindow(void)
{
	XAP_App::getApp()->rememberModelessId(getDialogId(), static_cast<XAP_Dialog_Modeless *>(this));
	UT_DEBUGMSG(("Contructing Window width %d height %d left %d top %d \n",m_width,m_height,m_left,m_top));
	m_pPreviewWindow = gtk_window_new(GTK_WINDOW_POPUP);
	//	gtk_window_set_decorated(GTK_WINDOW(m_pPreviewWindow),FALSE);
	gtk_widget_set_size_request(m_pPreviewWindow, m_width, m_height);
	gtk_window_set_position(GTK_WINDOW(m_pPreviewWindow),GTK_WIN_POS_MOUSE);
	gint root_x,root_y;
	gtk_window_get_position (GTK_WINDOW(m_pPreviewWindow),&root_x,&root_y);	
	m_pDrawingArea = createDrawingArea();
	gtk_widget_show(GTK_WIDGET(m_pDrawingArea));
	gtk_container_add(GTK_CONTAINER(m_pPreviewWindow), m_pDrawingArea);
	root_y -= m_height;
	// moving
	gtk_window_move(GTK_WINDOW(m_pPreviewWindow),
					root_x,
					root_y);
	
	gtk_widget_show_all(GTK_WIDGET(m_pPreviewWindow));
}

void  AP_UnixPreview_Annotation::destroy(void)
{
	modeless_cleanup();
	if (m_pPreviewWindow != NULL)
	{
		//_bringToTop();
		//sleep(4);
		//UT_DEBUGMSG(("sleep over\n"));
		
		DELETEP(m_gc);
		gtk_widget_destroy(m_pDrawingArea);
		gtk_widget_destroy(m_pPreviewWindow);
		m_pPreviewWindow = NULL;
		m_pDrawingArea = NULL;
	}
}



