/* AbiWord
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>
#include "ap_UnixFrameImpl.h"
#include "ap_UnixApp.h"
#include "ev_UnixToolbar.h"
#include "ap_FrameData.h"
#include "ap_UnixTopRuler.h"
#include "ap_UnixLeftRuler.h"
#include "xap_UnixApp.h"
#include "xap_UnixDialogHelper.h"
#include "ap_UnixStatusBar.h"
#include "ut_debugmsg.h"
#include "ev_UnixMenuBar.h"

#ifdef ABISOURCE_LICENSED_TRADEMARKS
#include "abiword_48_tm.xpm"
#else
#include "abiword_48.xpm"
#endif

AP_UnixFrameImpl::AP_UnixFrameImpl(AP_UnixFrame *pUnixFrame) :
	XAP_UnixFrameImpl(static_cast<XAP_Frame *>(pUnixFrame)),
	m_dArea(NULL),
	m_pVadj(NULL),
	m_pHadj(NULL),
	m_hScroll(NULL),
	m_vScroll(NULL),
	m_topRuler(NULL),
	m_leftRuler(NULL),
	m_table(NULL),
	m_innertable(NULL),
	m_wSunkenBox(NULL),
	m_iHScrollSignal(0),
	m_iVScrollSignal(0)
{
	UT_DEBUGMSG(("Created AP_UnixFrameImpl %x \n",this));
}

XAP_FrameImpl * AP_UnixFrameImpl::createInstance(XAP_Frame *pFrame)
{
	XAP_FrameImpl *pFrameImpl = new AP_UnixFrameImpl(static_cast<AP_UnixFrame *>(pFrame));

	return pFrameImpl;
}

void AP_UnixFrameImpl::_bindToolbars(AV_View * pView)
{
	int nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (int k = 0; k < nrToolbars; k++)
	{
		// TODO Toolbars are a frame-level item, but a view-listener is
		// TODO a view-level item.  I've bound the toolbar-view-listeners
		// TODO to the current view within this frame and have code in the
		// TODO toolbar to allow the view-listener to be rebound to a different
		// TODO view.  in the future, when we have support for multiple views
		// TODO in the frame (think splitter windows), we will need to have
		// TODO a loop like this to help change the focus when the current
		// TODO view changes.		
		EV_UnixToolbar * pUnixToolbar = reinterpret_cast<EV_UnixToolbar *>(m_vecToolbars.getNthItem(k));
		pUnixToolbar->bindListenerToView(pView);
	}	
}

// Does the initial show/hide of toolbars (based on the user prefs).
// This is needed because toggleBar is called only when the user
// (un)checks the show {Stantandard,Format,Extra} toolbar checkbox,
// and thus we have to manually call this function at startup.
void AP_UnixFrameImpl::_showOrHideToolbars()
{
	XAP_Frame* pFrame = getFrame();
	bool *bShowBar = static_cast<AP_FrameData*>(pFrame->getFrameData())->m_bShowBar;
	UT_uint32 cnt = m_vecToolbarLayoutNames.getItemCount();

	for (UT_uint32 i = 0; i < cnt; i++)
	{
		// TODO: The two next lines are here to bind the EV_Toolbar to the
		// AP_FrameData, but their correct place are next to the toolbar creation (JCA)
		EV_UnixToolbar * pUnixToolbar = static_cast<EV_UnixToolbar *> (m_vecToolbars.getNthItem(i));
		static_cast<AP_FrameData*> (pFrame->getFrameData())->m_pToolbar[i] = pUnixToolbar;
		static_cast<AP_UnixFrame *>(pFrame)->toggleBar(i, bShowBar[i]);
	}
}

/*!
 * Refills the framedata class with pointers to the current toolbars. We 
 * need to do this after a toolbar icon and been dragged and dropped.
 */
void AP_UnixFrameImpl::_refillToolbarsInFrameData()
{
	UT_uint32 cnt = m_vecToolbarLayoutNames.getItemCount();

	for (UT_uint32 i = 0; i < cnt; i++)
	{
		EV_UnixToolbar * pUnixToolbar = static_cast<EV_UnixToolbar *> (m_vecToolbars.getNthItem(i));
		static_cast<AP_FrameData*>(getFrame()->getFrameData())->m_pToolbar[i] = pUnixToolbar;
	}
}

// Does the initial show/hide of statusbar (based on the user prefs).
// Idem.
void AP_UnixFrameImpl::_showOrHideStatusbar()
{
#ifdef ENABLE_STATUSBAR
	XAP_Frame* pFrame = getFrame();
	bool bShowStatusBar = static_cast<AP_FrameData*> (pFrame->getFrameData())->m_bShowStatusBar;
	static_cast<AP_UnixFrame *>(pFrame)->toggleStatusBar(bShowStatusBar);
#endif
}


gboolean AP_UnixFrameImpl::ap_focus_in_event (GtkWidget * drawing_area, GdkEventCrossing *event, AP_UnixFrameImpl * me)
{
  gtk_widget_grab_focus (drawing_area);
  return TRUE;
}

gboolean AP_UnixFrameImpl::ap_focus_out_event (GtkWidget * drawing_area, GdkEventCrossing * event, AP_UnixFrameImpl * me)
{
  return TRUE;
}

GtkWidget * AP_UnixFrameImpl::_createDocumentWindow()
{
	XAP_Frame* pFrame = getFrame();
	bool bShowRulers = static_cast<AP_FrameData*>(pFrame->getFrameData())->m_bShowRuler;

	// create the rulers
	AP_UnixTopRuler * pUnixTopRuler = NULL;
	AP_UnixLeftRuler * pUnixLeftRuler = NULL;

	if ( bShowRulers )
	{
		pUnixTopRuler = new AP_UnixTopRuler(pFrame);
		UT_ASSERT(pUnixTopRuler);
		m_topRuler = pUnixTopRuler->createWidget();
		
		if (static_cast<AP_FrameData*>(pFrame->getFrameData())->m_pViewMode == VIEW_PRINT)
		  {
		    pUnixLeftRuler = new AP_UnixLeftRuler(pFrame);
		    UT_ASSERT(pUnixLeftRuler);
		    m_leftRuler = pUnixLeftRuler->createWidget();

		    // get the width from the left ruler and stuff it into the top ruler.
		    //pUnixTopRuler->setOffsetLeftRuler(pUnixLeftRuler->getWidth());
		  }
		else
		  {
		    m_leftRuler = NULL;
		    //pUnixTopRuler->setOffsetLeftRuler(0);
		  }
	}
	else
	{
		m_topRuler = NULL;
		m_leftRuler = NULL;
	}

	static_cast<AP_FrameData*>(pFrame->getFrameData())->m_pTopRuler = pUnixTopRuler;
	static_cast<AP_FrameData*>(pFrame->getFrameData())->m_pLeftRuler = pUnixLeftRuler;

	// set up for scroll bars.
	m_pHadj = reinterpret_cast<GtkAdjustment *>(gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
	m_hScroll = gtk_hscrollbar_new(m_pHadj);
	g_object_set_data(G_OBJECT(m_pHadj), "user_data", this);
	g_object_set_data(G_OBJECT(m_hScroll), "user_data", this);

	m_iHScrollSignal = g_signal_connect(G_OBJECT(m_pHadj), "value_changed", G_CALLBACK(XAP_UnixFrameImpl::_fe::hScrollChanged), NULL);

	m_pVadj = reinterpret_cast<GtkAdjustment *>(gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
	m_vScroll = gtk_vscrollbar_new(m_pVadj);
	g_object_set_data(G_OBJECT(m_pVadj), "user_data", this);
	g_object_set_data(G_OBJECT(m_vScroll), "user_data", this);

	m_iVScrollSignal = g_signal_connect(G_OBJECT(m_pVadj), "value_changed", G_CALLBACK(XAP_UnixFrameImpl::_fe::vScrollChanged), NULL);

	// we don't want either scrollbar grabbing events from us
	GTK_WIDGET_UNSET_FLAGS(m_hScroll, GTK_CAN_FOCUS);
	GTK_WIDGET_UNSET_FLAGS(m_vScroll, GTK_CAN_FOCUS);

	// create a drawing area in the for our document window.
	m_dArea = createDrawingArea ();
	g_object_set_data(G_OBJECT(m_dArea), "user_data", this);
	UT_DEBUGMSG(("!!! drawing area m_dArea created! %x for %x \n",m_dArea,this));
	GTK_WIDGET_SET_FLAGS (m_dArea, GTK_CAN_FOCUS);	// allow it to be focussed

	gtk_widget_set_events(GTK_WIDGET(m_dArea), (GDK_EXPOSURE_MASK |
						    GDK_BUTTON_PRESS_MASK |
						    GDK_POINTER_MOTION_MASK |
						    GDK_BUTTON_RELEASE_MASK |
						    GDK_KEY_PRESS_MASK |
						    GDK_KEY_RELEASE_MASK |
						    GDK_ENTER_NOTIFY_MASK |  
						    GDK_FOCUS_CHANGE_MASK |
						    GDK_LEAVE_NOTIFY_MASK));
	gtk_widget_set_double_buffered(GTK_WIDGET(m_dArea), FALSE);
	g_signal_connect(G_OBJECT(m_dArea), "expose_event",
					   G_CALLBACK(XAP_UnixFrameImpl::_fe::expose), NULL);
  
	g_signal_connect(G_OBJECT(m_dArea), "button_press_event",
					   G_CALLBACK(XAP_UnixFrameImpl::_fe::button_press_event), NULL);

	g_signal_connect(G_OBJECT(m_dArea), "button_release_event",
					   G_CALLBACK(XAP_UnixFrameImpl::_fe::button_release_event), NULL);

	g_signal_connect(G_OBJECT(m_dArea), "motion_notify_event",
					   G_CALLBACK(XAP_UnixFrameImpl::_fe::motion_notify_event), NULL);

	g_signal_connect(G_OBJECT(m_dArea), "scroll_event",
					   G_CALLBACK(XAP_UnixFrameImpl::_fe::scroll_notify_event), NULL);

	g_signal_connect(G_OBJECT(m_dArea), "configure_event",
					   G_CALLBACK(XAP_UnixFrameImpl::_fe::configure_event), NULL);

	// focus and XIM related
	g_signal_connect(G_OBJECT(m_dArea), "enter_notify_event", G_CALLBACK(ap_focus_in_event), this);
	g_signal_connect(G_OBJECT(m_dArea), "leave_notify_event", G_CALLBACK(ap_focus_out_event), this);

	//
	// Need this to fix screen flicker for abiwidget on focus in/out
	//
	g_signal_connect(G_OBJECT(m_dArea), "focus_in_event", G_CALLBACK(XAP_UnixFrameImpl::_fe::focus_in_event), this);
	g_signal_connect(G_OBJECT(m_dArea), "focus_out_event", G_CALLBACK(XAP_UnixFrameImpl::_fe::focus_out_event), this);


	// create a table for scroll bars, rulers, and drawing area

	m_table = gtk_table_new(1, 1, FALSE); //was 1,1
	g_object_set_data(G_OBJECT(m_table),"user_data", this);

	// NOTE:  in order to display w/ and w/o rulers, gtk needs two tables to
	// work with.  The 2 2x2 tables, (i)nner and (o)uter divide up the 3x3
	// table as follows.  The inner table is at the 1,1 table.
	//	+-----+---+
	//	| i i | o |
	//	| i i |   |
	//	+-----+---+
	//	|  o  | o |
	//	+-----+---+
		
	// scroll bars
	
	gtk_table_attach(GTK_TABLE(m_table), m_hScroll, 0, 1, 1, 2,
					 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					 (GtkAttachOptions) (GTK_FILL), // was just GTK_FILL
					 0, 0);

	gtk_table_attach(GTK_TABLE(m_table), m_vScroll, 1, 2, 0, 1,
					 (GtkAttachOptions) (GTK_FILL), // was just GTK_FILL
					 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					 0, 0);


	// arrange the widgets within our inner table.
	m_innertable = gtk_table_new(2,2,FALSE);
	gtk_table_attach( GTK_TABLE(m_table), m_innertable, 0, 1, 0, 1,
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 0, 0); 

	if ( bShowRulers )
	{
		gtk_table_attach(GTK_TABLE(m_innertable), m_topRuler, 0, 2, 0, 1,
						 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
						 (GtkAttachOptions)(GTK_FILL),
						 0, 0);

		if (m_leftRuler)
			gtk_table_attach(GTK_TABLE(m_innertable), m_leftRuler, 0, 1, 1, 2,
							 (GtkAttachOptions)(GTK_FILL),
							 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
							 0, 0);

		gtk_table_attach(GTK_TABLE(m_innertable), m_dArea,   1, 2, 1, 2,
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 0, 0); 
	}
	else	// no rulers
	{
		gtk_table_attach(GTK_TABLE(m_innertable), m_dArea,   1, 2, 1, 2,
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 0, 0); 
	}

	// create a 3d box and put the table in it, so that we
	// get a sunken in look.
	m_wSunkenBox = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(m_wSunkenBox), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(m_wSunkenBox), m_table);

	// (scrollbars are shown, only if needed, by _setScrollRange)
	gtk_widget_show(m_dArea);
	gtk_widget_show(m_innertable);
	gtk_widget_show(m_table);

	return m_wSunkenBox;
}

void AP_UnixFrameImpl::_hideMenuScroll(bool bHideMenuScroll)
{
  if(bHideMenuScroll)
  {
    UT_DEBUGMSG(("Hiding Menu \n"));
    gtk_widget_hide(m_pUnixMenu->getMenuBar());
    UT_DEBUGMSG(("Hiding scrollbar \n"));
    gtk_widget_hide(m_vScroll);
  }
  else
  {
    gtk_widget_show_all(m_pUnixMenu->getMenuBar());
    gtk_widget_show_all(m_vScroll);
  }
}
void AP_UnixFrameImpl::_setWindowIcon()
{
	// attach program icon to window
	GtkWidget * window = getTopLevelWindow();

	GdkPixbuf * icon = gdk_pixbuf_new_from_xpm_data (const_cast<const char **>(abiword_48_xpm));
	gtk_window_set_icon (GTK_WINDOW (window), icon);
	g_object_unref (G_OBJECT(icon));
}

void AP_UnixFrameImpl::_createWindow()
{
	_createTopLevelWindow();
	
	gtk_widget_show(getTopLevelWindow());

	if(getFrame()->getFrameMode() == XAP_NormalFrame)
	{
		// needs to be shown so that the following functions work
		// TODO: get rid of cursed flicker caused by initially
		// TODO: showing these and then hiding them (esp.
		// TODO: noticable in the gnome build with a toolbar disabled)
		_showOrHideToolbars();
		_showOrHideStatusbar();
	}
	if(getFrame()->isMenuScrollHidden())
	{
	    _hideMenuScroll(true);
	}
}

GtkWidget * AP_UnixFrameImpl::_createStatusBarWindow()
{
#ifdef ENABLE_STATUSBAR
	XAP_Frame* pFrame = getFrame();
	AP_UnixStatusBar * pUnixStatusBar = new AP_UnixStatusBar(pFrame);
	UT_ASSERT(pUnixStatusBar);

	static_cast<AP_FrameData *>(pFrame->getFrameData())->m_pStatusBar = pUnixStatusBar;
	
	return pUnixStatusBar->createWidget();
#else
	return NULL;
#endif
}

void AP_UnixFrameImpl::_setScrollRange(apufi_ScrollType scrollType, int iValue, gfloat fUpperLimit, gfloat fSize)
{
	GtkAdjustment *pScrollAdjustment = (scrollType == apufi_scrollX) ? m_pHadj : m_pVadj;
	GtkWidget *wScrollWidget = (scrollType == apufi_scrollX) ? m_hScroll : m_vScroll;
	UT_DEBUGMSG(("Scroll Adjustment set to %d \n",iValue));
	GR_Graphics * pGr = getFrame()->getCurrentView()->getGraphics ();
	XAP_Frame::tZoomType tZoom = getFrame()->getZoomType();
	if(pScrollAdjustment) //this isn't guaranteed in AbiCommand
	{
		pScrollAdjustment->value = iValue;
		pScrollAdjustment->lower = 0.0;
		pScrollAdjustment->upper = fUpperLimit;
		pScrollAdjustment->step_increment = pGr->tluD(20.0);
		pScrollAdjustment->page_increment = fSize;
		pScrollAdjustment->page_size = fSize;
		g_signal_emit_by_name(G_OBJECT(pScrollAdjustment), "changed");
	}

	// hide the horizontal scrollbar if the scroll range is such that the window can contain it all
	// show it otherwise
// Hide the horizontal scrollbar if we've set to page width or fit to page.
// This stops a resizing race condition.
//
 	if ((m_hScroll == wScrollWidget) && ((fUpperLimit <= fSize) ||(  tZoom == XAP_Frame::z_PAGEWIDTH) || (tZoom == XAP_Frame::z_WHOLEPAGE)))
	{
 		gtk_widget_hide(wScrollWidget);
	}
 	else if((wScrollWidget != m_vScroll) || !getFrame()->isMenuScrollHidden())
	{
 		gtk_widget_show(wScrollWidget);
	}
}

UT_RGBColor AP_UnixFrameImpl::getColorSelBackground () const
{
  UT_return_val_if_fail(m_dArea, UT_RGBColor(0,0,0));

  // owen says that any widget should be ok, not just text widgets
  GdkColor clr = m_dArea->style->base[GTK_STATE_SELECTED];
  return UT_RGBColor (clr.red >> 8, clr.green >> 8, clr.blue >> 8);
}

UT_RGBColor AP_UnixFrameImpl::getColorSelForeground () const
{
  UT_return_val_if_fail(m_dArea, UT_RGBColor(0,0,0));

  // owen says that any widget should be ok, not just text widgets
  gint state;
  
  // our text widget has focus
  if (GTK_WIDGET_HAS_FOCUS(m_dArea))
    state = GTK_STATE_SELECTED;
  else
    state = GTK_STATE_ACTIVE;
  
  GdkColor clr = m_dArea->style->text[state];
  return UT_RGBColor (clr.red >> 8, clr.green >> 8, clr.blue >> 8);
}
