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

#ifndef AP_QNXSTATUSBAR_H
#define AP_QNXSTATUSBAR_H

// Class for dealing with the status bar at the bottom of
// the frame window.

#include "ut_types.h"
#include "ap_StatusBar.h"
#include "gr_QNXGraphics.h"
class XAP_Frame;

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class AP_QNXStatusBar : public AP_StatusBar
{
public:
	AP_QNXStatusBar(XAP_Frame * pFrame);
	virtual ~AP_QNXStatusBar(void);

	virtual void		setView(AV_View * pView);
	PtWidget_t *		createWidget(void);

	virtual void 		show();
	virtual void 		hide();
	virtual int	 		getHeight();

protected:
	PtWidget_t *		m_wStatusBar;
	PtWidget_t *		m_wStatusBarGroup;

	class _fe
	{
	public:
//		static int button_press_event(GtkWidget * w, GdkEventButton * e);
//		static int button_release_event(GtkWidget * w, GdkEventButton * e);
//		static int motion_notify_event(GtkWidget* w, GdkEventMotion* e);
//		static int key_press_event(GtkWidget* w, GdkEventKey* e);
//		static int delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/);
		static int resize(PtWidget_t* w, void *data,  PtCallbackInfo_t *info);
		static int expose(PtWidget_t * w, PhTile_t *damage);
//		static void destroy (GtkWidget * /*widget*/, gpointer /*data*/);
	};

};

#endif /* AP_QNXSTATUSBAR_H */
