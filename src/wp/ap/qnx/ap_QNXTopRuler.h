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

#ifndef AP_QNXTOPRULER_H
#define AP_QNXTOPRULER_H

// Class for dealing with the horizontal ruler at the top of
// a document window.

/*****************************************************************/

#include "ut_types.h"
#include "ap_TopRuler.h"
#include "gr_QNXGraphics.h"
#include <Ph.h>
class XAP_Frame;


/*****************************************************************/

class AP_QNXTopRuler : public AP_TopRuler
{
public:
	AP_QNXTopRuler(XAP_Frame * pFrame);
	virtual ~AP_QNXTopRuler(void);

	PtWidget_t *		createWidget(void);
	virtual void	setView(AV_View * pView);

	// cheats for the callbacks
	void 	getWidgetPosition(int * x, int * y);
	void * 	getWidget(void) { return m_wTopRuler; };
	void * 	getRootWindow(void);
			
protected:
	PtWidget_t *	m_wTopRuler;
	PtWidget_t *	m_rootWindow;

	class _fe
	{
	public:
		static int button_press_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info);
		static int button_release_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info);
		static int motion_notify_event(PtWidget_t* w, void *data, PtCallbackInfo_t* info);
		static int resize(PtWidget_t* w, void *data,  PtCallbackInfo_t *info);
//		static int key_press_event(GtkWidget* w, GdkEventKey* e);
//		static int delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/);
		static int expose(PtWidget_t * w, PhTile_t* damage);
	};

};

#endif /* AP_QNXTOPRULER_H */
