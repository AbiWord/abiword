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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_UNIXLEFTRULER_H
#define AP_UNIXLEFTRULER_H

// Class for dealing with the horizontal ruler at the left of
// a document window.

/*****************************************************************/

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "ut_types.h"
#include "ap_LeftRuler.h"
#include "xap_UnixCustomWidget.h"

class XAP_Frame;

/*****************************************************************/

class AP_UnixLeftRuler : public AP_LeftRuler, public XAP_UnixCustomWidget
{
public:
	AP_UnixLeftRuler(XAP_Frame * pFrame);
	virtual ~AP_UnixLeftRuler(void);

	GtkWidget *		createWidget(void);
	virtual void	setView(AV_View * pView);

	// cheats for the callbacks
	void				getWidgetPosition(gint * x, gint * y);
	GtkWidget * 		getWidget(void) { return m_wLeftRuler; };
	GdkWindow * 	getRootWindow(void);

	void _ruler_style_context_changed (void);

protected:
	GtkWidget *		m_wLeftRuler;
	GdkWindow *	m_rootWindow;
    guint            m_iBackgroundRedrawID;
protected:

	class _fe
	{
	public:
		static void realize(AP_UnixLeftRuler *self);
		static void unrealize(AP_UnixLeftRuler *self);
		static gint button_press_event(GtkWidget * w, GdkEventButton * e);
		static gint button_release_event(GtkWidget * w, GdkEventButton * e);
		static gint configure_event(GtkWidget* w, GdkEventConfigure *e);
		static gint motion_notify_event(GtkWidget* w, GdkEventMotion* e);
		static gint key_press_event(GtkWidget* w, GdkEventKey* e);
		static gint delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/);
		static void destroy (GtkWidget * /*widget*/, gpointer /*data*/);
	};
	friend class _fe;	// we consider this _fe class to be friend....
};

#endif /* AP_UNIXLEFTRULER_H */
