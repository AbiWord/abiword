/* AbiWord
 * Copyright (C) 2004 Marc Maurer
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

#include "ap_UnixRevealCodes.h"
#include "xap_UnixDialogHelper.h"
#include "xap_UnixFontManager.h"
#include "ap_UnixFrameImpl.h"
#include "ut_debugmsg.h"
#include "gr_UnixGraphics.h"

AP_UnixRevealCodes::AP_UnixRevealCodes(XAP_Frame* pFrame) :
	AP_RevealCodes(pFrame)
{
}

AP_UnixRevealCodes::~AP_UnixRevealCodes()
{
	UT_DEBUGMSG(("~AP_UnixRevealCodes()\n"));
	
	GtkWidget* pTopLevelWindow = static_cast<XAP_UnixFrameImpl*>(m_pFrame->getFrameImpl())->getTopLevelWindow();
	g_signal_handlers_disconnect_by_func(G_OBJECT(pTopLevelWindow), 
					   (void*)AP_UnixRevealCodes::_fe::key_press_event, NULL);
	
	g_signal_connect(G_OBJECT(pTopLevelWindow), "key_press_event",
				   G_CALLBACK(XAP_UnixFrameImpl::_fe::key_press_event), NULL);
}

GtkWidget* AP_UnixRevealCodes::createWidget()
{
	GtkWidget *dRcArea = createDrawingArea ();
	g_object_set_data(G_OBJECT(dRcArea), "user_data", m_pFrame);
	UT_DEBUGMSG(("!!! drawing area dRcArea created! %x for %x \n", dRcArea, this));
	GTK_WIDGET_SET_FLAGS (dRcArea, GTK_CAN_FOCUS);	// allow it to be focussed

	gtk_widget_set_events(GTK_WIDGET(dRcArea), (GDK_EXPOSURE_MASK |
							GDK_BUTTON_PRESS_MASK |
							GDK_POINTER_MOTION_MASK |
							GDK_BUTTON_RELEASE_MASK |
							GDK_KEY_PRESS_MASK |
							GDK_KEY_RELEASE_MASK |
							GDK_ENTER_NOTIFY_MASK |
							GDK_LEAVE_NOTIFY_MASK));
							
	g_signal_connect(G_OBJECT(dRcArea), "expose_event",
					   G_CALLBACK(AP_UnixRevealCodes::_fe::expose), NULL);
	
	g_signal_connect(G_OBJECT(dRcArea), "configure_event",
					   G_CALLBACK(AP_UnixRevealCodes::_fe::configure_event), NULL);	

	/* FIXME: clean me up - MARCM */
	GtkWidget* pTopLevelWindow = static_cast<XAP_UnixFrameImpl*>(m_pFrame->getFrameImpl())->getTopLevelWindow();
	g_signal_handlers_disconnect_by_func(G_OBJECT(pTopLevelWindow), 
					   (void*)XAP_UnixFrameImpl::_fe::key_press_event, NULL);

	g_signal_connect(G_OBJECT(pTopLevelWindow), "key_press_event",
					   G_CALLBACK(AP_UnixRevealCodes::_fe::key_press_event), NULL);
					   
					   
	/*g_signal_connect(G_OBJECT(m_wTopLevelWindow), "key_release_event",
					   G_CALLBACK(_fe::key_release_event), NULL);*/
	
	// focus and XIM related
	//g_signal_connect(G_OBJECT(dRcArea), "enter_notify_event",
	//				   G_CALLBACK(AP_UnixRevealCodes::_fe::focus_in_event), this);
					   
	//g_signal_connect(G_OBJECT(dRcArea), "leave_notify_event",
	//				   G_CALLBACK(focus_out_event), this);*/
	
	gtk_widget_show(dRcArea);
	
	return dRcArea;
}

GtkWidget* AP_UnixRevealCodes::createContainer()
{
	GtkWidget* rctable = gtk_table_new(2, 2, FALSE);
	//g_object_set_data(G_OBJECT(m_rctable),"user_data", this);	
	
	// set it to some 'sensible' initial height	
	gtk_widget_set_size_request(rctable, -1, 150); 
	
	// FIXME: add the scrollbars - MARCM

	gtk_widget_show(rctable);
	
	return rctable;
}

bool AP_UnixRevealCodes::_createViewGraphics(XAP_Frame* pFrame, GR_Graphics *& pG, UT_uint32 iZoom)
{
	XAP_UnixFontManager * fontManager = (static_cast<XAP_UnixApp *>( XAP_App::getApp())->getFontManager());
	AP_UnixFrameImpl * pImpl = static_cast<AP_UnixFrameImpl *>(pFrame->getFrameImpl());
	UT_ASSERT(pImpl);
	GR_UnixAllocInfo ai(pImpl->getRcArea()->window, fontManager, XAP_App::getApp());
	pG = (GR_UnixGraphics*) XAP_App::getApp()->newGraphics(ai);
	pG->setZoomPercentage(iZoom);
	static_cast<GR_UnixGraphics*>(pG)->init3dColors(pImpl->getRcArea()->style);

	return true;
}

/* XAP_UnixFrameImpl::_fe */

gint AP_UnixRevealCodes::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	XAP_Frame* pFrame = static_cast<XAP_Frame*>(g_object_get_data(G_OBJECT(w), "user_data"));
	AV_View * pView = static_cast<AV_View *>(pFrame->getCurrentRcView());
	if(pView)
	{
		pView->setWindowSize(e->width, e->height);
		return true;
	}
	return false;
}

gint AP_UnixRevealCodes::_fe::expose(GtkWidget * w, GdkEventExpose* pExposeEvent)
{
	XAP_Frame* pFrame = static_cast<XAP_Frame*>(g_object_get_data(G_OBJECT(w), "user_data"));
	AV_View * pView = static_cast<AV_View *>(pFrame->getCurrentRcView());
	if(pView)
	{
		GR_Graphics * pGr = pView->getGraphics();
		UT_Rect rClip;
		xxx_UT_DEBUGMSG(("Expose area: x %d y %d width %d  height %d \n",pExposeEvent->area.x,pExposeEvent->area.y,pExposeEvent->area.width,pExposeEvent->area.height));
		rClip.left = pGr->tlu(pExposeEvent->area.x);
		rClip.top = pGr->tlu(pExposeEvent->area.y);
		rClip.width = pGr->tlu(pExposeEvent->area.width)+1;
		rClip.height = pGr->tlu(pExposeEvent->area.height)+1;

		pView->draw(&rClip);
		return true;
	}
	return false;
}

gint AP_UnixRevealCodes::_fe::key_press_event(GtkWidget* w, GdkEventKey* e)
{
	UT_DEBUGMSG(("Revel Codes: Trapped keypress!\n"));
	XAP_UnixFrameImpl::_fe::key_press_event(w, e);

	return 1;
}
