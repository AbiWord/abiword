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

#include <gtk/gtk.h>
#include "ap_RevealCodes.h"

class AP_UnixRevealCodes : public AP_RevealCodes
{
public:
	AP_UnixRevealCodes(XAP_Frame* pFrame);
	virtual ~AP_UnixRevealCodes();

	GtkWidget* createWidget();
	GtkWidget* createContainer();

	class _fe
    {
	public:
		//static gint button_press_event(GtkWidget * w, GdkEventButton * e);
		//static gint button_release_event(GtkWidget * w, GdkEventButton * e);
		static gint configure_event(GtkWidget* w, GdkEventConfigure *e);
		//static gint motion_notify_event(GtkWidget* w, GdkEventMotion* e);
		//static gint scroll_notify_event(GtkWidget* w, GdkEventScroll* e);
		static gint key_press_event(GtkWidget* w, GdkEventKey* e);
		//static gint key_release_event(GtkWidget* w, GdkEventKey* e);
		//static gint delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/);
		static gint expose(GtkWidget * w, GdkEventExpose* pExposeEvent);
		//static gint abi_expose_repaint( gpointer /* xap_UnixFrame * */ p);
		//static gint do_ZoomUpdate( gpointer /* xap_UnixFrame * */ p);
		//static void vScrollChanged(GtkAdjustment * w, gpointer /*data*/);
		//static void hScrollChanged(GtkAdjustment * w, gpointer /*data*/);
		//static void destroy (GtkWidget * /*widget*/, gpointer /*data*/);
		//static gboolean focus_in_event(GtkWidget *w,GdkEvent *event,gpointer user_data);
		//static gboolean focus_out_event(GtkWidget *w,GdkEvent *event,gpointer user_data);
	
		//static void realize(GtkWidget * widget, GdkEvent */* e*/,gpointer /*data*/);
		//static void unrealize(GtkWidget * widget, GdkEvent */* e */,gpointer /* data */);
		//static void sizeAllocate(GtkWidget * widget, GdkEvent */* e */,gpointer /* data */);
		//static gint focusIn(GtkWidget * widget, GdkEvent */* e */,gpointer /* data */);
		//static gint focusOut(GtkWidget * /*widget*/, GdkEvent */* e */,gpointer /* data */);
	};

protected:
	virtual bool _createViewGraphics(XAP_Frame* pFrame, GR_Graphics *& pG, UT_uint32 iZoom);
};
