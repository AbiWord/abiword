/* AbiWord
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef AP_BEOSLEFTRULER_H
#define AP_BEOSLEFTRULER_H

// Class for dealing with the horizontal ruler at the left of
// a document window.

/*****************************************************************/

#include "ut_types.h"
#include "ap_LeftRuler.h"
#include "gr_BeOSGraphics.h"
class XAP_Frame;


/*****************************************************************/

class AP_BeOSLeftRuler : public AP_LeftRuler
{
public:
	AP_BeOSLeftRuler(XAP_Frame * pFrame);
	virtual ~AP_BeOSLeftRuler(void);

//	GtkWidget *		createWidget(void);
	virtual void	setView(AV_View * pView);
	
protected:
	//GtkWidget *		m_wLeftRuler;
	void *			m_wLeftRuler;

protected:

#if 0
	class _fe
	{
	public:
		static gint button_press_event(GtkWidget * w, GdkEventButton * e);
		static gint button_release_event(GtkWidget * w, GdkEventButton * e);
		static gint configure_event(GtkWidget* w, GdkEventConfigure *e);
		static gint motion_notify_event(GtkWidget* w, GdkEventMotion* e);
		static gint key_press_event(GtkWidget* w, GdkEventKey* e);
		static gint delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/);
		static gint expose(GtkWidget * w, GdkEventExpose* pExposeEvent);
		static void destroy (GtkWidget * /*widget*/, gpointer /*data*/);
	};
#endif

};

#endif /* AP_BEOSLEFTRULER_H */
