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

#include <gtk/gtk.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ap_UnixAp.h"
#include "ap_UnixFrame.h"
#include "ev_UnixKeyboard.h"
#include "ev_UnixMouse.h"
#include "ev_UnixMenu.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_UnixGraphics.h"

#define DELETEP(p)		do { if (p) delete p; } while (0)
#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

class _fe								// a little private class to collect all of the
{										// static callbacks we need for the frame window.
public:
	static gint button_press_event(GtkWidget * w, GdkEventButton * e)
	{
		AP_UnixFrame * pUnixFrame = (AP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
		FV_View * pView = pUnixFrame->getCurrentView();
		EV_UnixMouse * pUnixMouse = pUnixFrame->getUnixMouse();
		
		if (pView)
			pUnixMouse->mouseClick(pView,e);
		return 1;
	};

	static gint configure_event(GtkWidget* w, GdkEventConfigure *e)
	{
		AP_UnixFrame * pUnixFrame = (AP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
		FV_View * pView = pUnixFrame->getCurrentView();

		if (pView)
			pView->setWindowSize(e->width, e->height);
		return 1;
	};
	
	static gint motion_notify_event(GtkWidget* w, GdkEventMotion* e)
	{
		AP_UnixFrame * pUnixFrame = (AP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
		FV_View * pView = pUnixFrame->getCurrentView();
		EV_UnixMouse * pUnixMouse = pUnixFrame->getUnixMouse();

		if (pView)
			if (e->state & GDK_BUTTON1_MASK)
				pUnixMouse->mouseMotion(pView, e);
	
		return 1;
	};
	
	static gint key_press_event(GtkWidget* w, GdkEventKey* e)
	{
		AP_UnixFrame * pUnixFrame = (AP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
		FV_View * pView = pUnixFrame->getCurrentView();
		ev_UnixKeyboard * pUnixKeyboard = pUnixFrame->getUnixKeyboard();
		
		if (pView)
			pUnixKeyboard->keyPressEvent(pView, e);
		return 1;
	};
	
	static gint delete_event(GtkWidget * /*widget*/, GdkEvent * /*event*/, gpointer /*data*/)
	{
		// TODO decide if we need this

		return FALSE;
	};
	
	static gint expose(GtkWidget * w, gpointer /*data*/)
	{
		AP_UnixFrame * pUnixFrame = (AP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
		FV_View * pView = pUnixFrame->getCurrentView();

		if (pView)
			pView->draw();
		return 0;
	};
	
	static void vHcrollChanged(GtkAdjustment * w, gpointer /*data*/)
	{
		AP_UnixFrame * pUnixFrame = (AP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
		FV_View * pView = pUnixFrame->getCurrentView();

		if (pView)
			pView->setXScrollOffset((UT_sint32) w->value);
	};
	
	static void vScrollChanged(GtkAdjustment * w, gpointer /*data*/)
	{
		AP_UnixFrame * pUnixFrame = (AP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
		FV_View * pView = pUnixFrame->getCurrentView();

		if (pView)
			pView->setYScrollOffset((UT_sint32) w->value);
	};
	
	static void hScrollChanged(GtkAdjustment * w, gpointer /*data*/)
	{
		AP_UnixFrame * pUnixFrame = (AP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
		FV_View * pView = pUnixFrame->getCurrentView();

		if (pView)
			pView->setXScrollOffset((UT_sint32) w->value);
	};
	
	static void destroy (GtkWidget * /*widget*/, gpointer /*data*/)
	{
		// TODO decide if this is the right way to die....
		gtk_main_quit ();
	};
};
	
	
/*****************************************************************/

AP_UnixFrame::AP_UnixFrame(AP_UnixAp * ap)
	: AP_Frame(static_cast<AP_Ap *>(ap))
{
	m_pUnixAp = ap;
	m_pUnixKeyboard = NULL;
	m_pUnixMouse = NULL;
	m_pUnixMenu = NULL;
	m_pView = NULL;
}

AP_UnixFrame::~AP_UnixFrame(void)
{
	// only delete the things we created...
	
	DELETEP(m_pUnixKeyboard);
	DELETEP(m_pUnixMouse);
	DELETEP(m_pUnixMenu);
	// TODO do we need to delete m_pView...
}

UT_Bool AP_UnixFrame::initialize(int * pArgc, char *** pArgv)
{
	UT_Bool bResult;

	// invoke our base class first.
	
	bResult = AP_Frame::initialize(pArgc,pArgv);
	UT_ASSERT(bResult);

	_createTopLevelWindow();
	
	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.
	
	m_pUnixKeyboard = new ev_UnixKeyboard(m_pEEM);
	UT_ASSERT(m_pUnixKeyboard);
	
	m_pUnixMouse = new EV_UnixMouse(m_pEEM);
	UT_ASSERT(m_pUnixMouse);

	// ... add other stuff (like tool bar) here...

	gtk_widget_show(m_wTopLevelWindow);

	return UT_TRUE;
}

GtkWidget * AP_UnixFrame::getTopLevelWindow(void) const
{
	return m_wTopLevelWindow;
}

GtkWidget * AP_UnixFrame::getVBoxWidget(void) const
{
	return m_wVBox;
}

EV_UnixMouse * AP_UnixFrame::getUnixMouse(void)
{
	return m_pUnixMouse;
}

ev_UnixKeyboard * AP_UnixFrame::getUnixKeyboard(void)
{
	return m_pUnixKeyboard;
}

void AP_UnixFrame::_createTopLevelWindow(void)
{
	// create a top-level window for us.

	m_wTopLevelWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_object_set_data(GTK_OBJECT(m_wTopLevelWindow), "toplevelWindow",
						m_wTopLevelWindow);
	gtk_object_set_user_data(GTK_OBJECT(m_wTopLevelWindow),this);
	gtk_window_set_title(GTK_WINDOW(m_wTopLevelWindow),
						 m_pUnixAp->getApplicationTitleForTitleBar());
	gtk_window_set_policy(GTK_WINDOW(m_wTopLevelWindow), TRUE, TRUE, FALSE);
	gtk_window_set_wmclass(GTK_WINDOW(m_wTopLevelWindow),
						   m_pUnixAp->getApplicationName(),
						   m_pUnixAp->getApplicationName());

	// TODO get the following values from a preferences or something.
	gtk_container_border_width(GTK_CONTAINER(m_wTopLevelWindow), 10);
	gtk_widget_set_usize(GTK_WIDGET(m_wTopLevelWindow), 700,650);

	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "delete_event",
						GTK_SIGNAL_FUNC(_fe::delete_event), NULL);
	// here we connect the "destroy" event to a signal handler.  
	// This event occurs when we call gtk_widget_destroy() on the window,
	// or if we return 'FALSE' in the "delete_event" callback.
	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "destroy",
						GTK_SIGNAL_FUNC(_fe::destroy), NULL);

	// create a VBox inside it.
	
	m_wVBox = gtk_vbox_new(FALSE,0);
	gtk_object_set_data(GTK_OBJECT(m_wTopLevelWindow), "vbox", m_wVBox);
	gtk_object_set_user_data(GTK_OBJECT(m_wVBox),this);
	gtk_container_add(GTK_CONTAINER(m_wTopLevelWindow), m_wVBox);

	// TODO get the following values from a preferences or something.
	gtk_container_border_width(GTK_CONTAINER(m_wVBox), 4);

	// synthesize a menu from the info in our base class.

	m_pUnixMenu = new EV_UnixMenu(m_pUnixAp,this);
	UT_ASSERT(m_pUnixMenu);
	UT_Bool bResult = m_pUnixMenu->synthesize();
	UT_ASSERT(bResult);

	// TODO deal with other window decorations after the
	// TODO menu and before the drawing area.

	// set up for scroll bars.
	
	m_pHadj = (GtkAdjustment*) gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	gtk_object_set_user_data(GTK_OBJECT(m_pHadj),this);
	m_hScroll = gtk_hscrollbar_new(m_pHadj);
	gtk_object_set_user_data(GTK_OBJECT(m_hScroll),this);

	gtk_signal_connect(GTK_OBJECT(m_pHadj), "value_changed", GTK_SIGNAL_FUNC(_fe::hScrollChanged), NULL);
	gtk_signal_connect(GTK_OBJECT(m_pHadj), "changed", GTK_SIGNAL_FUNC(_fe::hScrollChanged), NULL);

	m_pVadj = (GtkAdjustment*) gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	gtk_object_set_user_data(GTK_OBJECT(m_pVadj),this);
	m_vScroll = gtk_vscrollbar_new(m_pVadj);
	gtk_object_set_user_data(GTK_OBJECT(m_vScroll),this);

	gtk_signal_connect(GTK_OBJECT(m_pVadj), "value_changed", GTK_SIGNAL_FUNC(_fe::vScrollChanged), NULL);
	gtk_signal_connect(GTK_OBJECT(m_pVadj), "changed", GTK_SIGNAL_FUNC(_fe::vScrollChanged), NULL);

	// we don't want either scrollbar grabbing events from us
	GTK_WIDGET_UNSET_FLAGS(m_hScroll, GTK_CAN_FOCUS);
	GTK_WIDGET_UNSET_FLAGS(m_vScroll, GTK_CAN_FOCUS);

	// create a drawing area in the for our document window.
	
	m_dArea = gtk_drawing_area_new();
	gtk_object_set_user_data(GTK_OBJECT(m_dArea),this);
	gtk_widget_set_events(GTK_WIDGET(m_dArea), (GDK_EXPOSURE_MASK |
												GDK_BUTTON_PRESS_MASK |
												GDK_POINTER_MOTION_MASK |
												GDK_BUTTON_RELEASE_MASK |
												GDK_KEY_PRESS_MASK |
												GDK_KEY_RELEASE_MASK));

	gtk_signal_connect(GTK_OBJECT(m_dArea), "expose_event",
					   GTK_SIGNAL_FUNC(_fe::expose), NULL);
  
	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "key_press_event",
					   GTK_SIGNAL_FUNC(_fe::key_press_event), NULL);

	gtk_signal_connect(GTK_OBJECT(m_dArea), "button_press_event",
					   GTK_SIGNAL_FUNC(_fe::button_press_event), NULL);

	gtk_signal_connect(GTK_OBJECT(m_dArea), "motion_notify_event",
					   GTK_SIGNAL_FUNC(_fe::motion_notify_event), NULL);
  
	gtk_signal_connect(GTK_OBJECT(m_dArea), "configure_event",
					   GTK_SIGNAL_FUNC(_fe::configure_event), NULL);

	// create a table for scroll bars and drawing area
	
	m_table = gtk_table_new(1, 2, FALSE);
	gtk_object_set_user_data(GTK_OBJECT(m_table),this);
  
	gtk_table_attach(GTK_TABLE(m_table), m_dArea,   0, 1, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(m_table), m_hScroll, 0, 1, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(m_table), m_vScroll, 1, 2, 0, 1, GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	gtk_container_add(GTK_CONTAINER(m_wVBox), m_table);

	// TODO decide what to do with accelerators
	// gtk_window_add_accelerator_table(GTK_WINDOW(window), accel);

	gtk_widget_show(m_hScroll);
	gtk_widget_show(m_vScroll);
	gtk_widget_show(m_dArea);
	gtk_widget_show(m_table);
	gtk_widget_show(m_wVBox);

	// we let our caller decide when to show m_wTopLevelWindow.

	return;
}

UT_Bool AP_UnixFrame::loadDocument(const char * szFilename)
{
	if (! AP_Frame::loadDocument(szFilename))
	{
		// we could not load the document.
		// TODO how should we complain to the user ??

		return UT_FALSE;
	}

	UNIXGraphics * pG = NULL;
	FL_DocLayout * pDocLayout = NULL;
	FV_View * pView = NULL;
	FV_ScrollObj * pScrollObj = NULL;

	int height, pageLen;

	// TODO fix prefix on class UNIXGraphics
	
	pG = new UNIXGraphics(m_dArea->window);
	ENSUREP(pG);
	pDocLayout = new FL_DocLayout(m_pDoc, pG);
	ENSUREP(pDocLayout);
  
	pDocLayout->formatAll();

	pView = new FV_View(this, pDocLayout);
	ENSUREP(pView);
	pScrollObj = new FV_ScrollObj(this,_scrollFunc);
	ENSUREP(pScrollObj);

	// switch to new view, cleaning up previous settings
	REPLACEP(m_pG, pG);
	REPLACEP(m_pDocLayout, pDocLayout);
	REPLACEP(m_pView, pView);
	REPLACEP(m_pScrollObj, pScrollObj);

	m_pView->addScrollListener(m_pScrollObj);
	m_pView->setWindowSize(GTK_WIDGET(m_dArea)->allocation.width,
						   GTK_WIDGET(m_dArea)->allocation.height);
  
	height = m_pDocLayout->getHeight();
	pageLen = height/m_pDocLayout->countPages();

	m_pVadj->value = 0.0;
	m_pVadj->lower = 0.0;
	m_pVadj->upper = (gfloat) height;
	m_pVadj->step_increment = 20.0;
	m_pVadj->page_increment = (gfloat) pageLen;
	m_pVadj->page_size = (gfloat) pageLen;

	gtk_signal_emit_by_name(GTK_OBJECT(m_pVadj), "changed");
	m_pView->draw();

	return UT_TRUE;

Cleanup:
	// clean up anything we created here
	DELETEP(pG);
	DELETEP(pDocLayout);
	DELETEP(pView);
	DELETEP(pScrollObj);

	// change back to prior document
	DELETEP(m_pDoc);
	m_pDoc = m_pDocLayout->getDocument();

	return UT_FALSE;
}

void AP_UnixFrame::_scrollFunc(void * pData, UT_sint32 xoff, UT_sint32 yoff)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	
	AP_UnixFrame * pUnixFrame = static_cast<AP_UnixFrame *>(pData);
		
	pUnixFrame->m_pVadj->value = (gfloat) yoff;
	gtk_signal_emit_by_name(GTK_OBJECT(pUnixFrame->m_pVadj), "changed");

	pUnixFrame->m_pHadj->value = (gfloat) xoff;
	gtk_signal_emit_by_name(GTK_OBJECT(pUnixFrame->m_pHadj), "changed");
}
