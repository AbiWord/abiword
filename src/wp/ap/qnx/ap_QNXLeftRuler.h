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

#ifndef AP_QNXLEFTRULER_H
#define AP_QNXLEFTRULER_H

// Class for dealing with the horizontal ruler at the left of
// a document window.

/*****************************************************************/

#include "ut_types.h"
#include "ap_LeftRuler.h"
#include "gr_QNXGraphics.h"
#include <Ph.h>
class XAP_Frame;


/*****************************************************************/

class AP_QNXLeftRuler : public AP_LeftRuler
{
public:
	AP_QNXLeftRuler(XAP_Frame * pFrame);
	virtual ~AP_QNXLeftRuler(void);

	PtWidget_t *	createWidget(void);
	virtual void	setView(AV_View * pView);
	
protected:
	PtWidget_t *		m_wLeftRuler;

protected:
	class _fe
	{
	public:
//		static int button_press_event(GtkWidget * w, GdkEventButton * e);
//		static int button_release_event(GtkWidget * w, GdkEventButton * e);
//		static int configure_event(GtkWidget* w, GdkEventConfigure *e);
//		static int motion_notify_event(GtkWidget* w, GdkEventMotion* e);
//		static int key_press_event(GtkWidget* w, GdkEventKey* e);
//		static int delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/);
		static int expose(PtWidget_t * w, PhTile_t *damage);
		static int resize(PtWidget_t* w, void *data,  PtCallbackInfo_t *info);
	};
};

#endif /* AP_QNXLEFTRULER_H */
