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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
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
#include "xap_Gtk2Compat.h"
#include "ap_UnixStatusBar.h"
#include "ut_debugmsg.h"
#include "ev_UnixMenuBar.h"


AP_UnixFrameImpl::AP_UnixFrameImpl(AP_UnixFrame *pUnixFrame) :
	XAP_UnixFrameImpl(static_cast<XAP_Frame *>(pUnixFrame)),
	m_dArea(NULL),
	m_pVadj(NULL),
	m_pHadj(NULL),
	m_hScroll(NULL),
	m_vScroll(NULL),
	m_topRuler(NULL),
	m_leftRuler(NULL),
	m_grid(NULL),
	m_innergrid(NULL),
	m_wSunkenBox(NULL),
	m_iHScrollSignal(0),
	m_iVScrollSignal(0)
{
	UT_DEBUGMSG(("Created AP_UnixFrameImpl %p \n",this));
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


gboolean AP_UnixFrameImpl::ap_focus_in_event (GtkWidget * drawing_area, GdkEventCrossing * /*event*/, AP_UnixFrameImpl * /*me*/)
{
  gtk_widget_grab_focus (drawing_area);
  return TRUE;
}

gboolean AP_UnixFrameImpl::ap_focus_out_event (GtkWidget * /*drawing_area*/, GdkEventCrossing * /*event*/, AP_UnixFrameImpl * /*me*/)
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
	m_hScroll = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, m_pHadj);
	g_object_set_data(G_OBJECT(m_pHadj), "user_data", this);
	g_object_set_data(G_OBJECT(m_hScroll), "user_data", this);
	gtk_widget_set_hexpand(m_hScroll, TRUE);

	m_iHScrollSignal = g_signal_connect(G_OBJECT(m_pHadj), "value_changed", G_CALLBACK(XAP_UnixFrameImpl::_fe::hScrollChanged), NULL);

	m_pVadj = reinterpret_cast<GtkAdjustment *>(gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
	m_vScroll = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, m_pVadj);
	g_object_set_data(G_OBJECT(m_pVadj), "user_data", this);
	g_object_set_data(G_OBJECT(m_vScroll), "user_data", this);
	gtk_widget_set_vexpand(m_vScroll, TRUE);

	m_iVScrollSignal = g_signal_connect(G_OBJECT(m_pVadj), "value_changed", G_CALLBACK(XAP_UnixFrameImpl::_fe::vScrollChanged), NULL);

	// we don't want either scrollbar grabbing events from us
	gtk_widget_set_can_focus(m_hScroll, false);
	gtk_widget_set_can_focus(m_vScroll, false);

	// create a drawing area in the for our document window.
	m_dArea = ap_DocView_new();
	g_object_set(G_OBJECT(m_dArea), "expand", TRUE, NULL);
	g_object_set_data(G_OBJECT(m_dArea), "user_data", this);
	UT_DEBUGMSG(("!!! drawing area m_dArea created! %p for %p \n",m_dArea,this));
	gtk_widget_set_can_focus(m_dArea, true);	// allow it to be focussed

	gtk_widget_set_events(GTK_WIDGET(m_dArea), (GDK_EXPOSURE_MASK |
						    GDK_BUTTON_PRESS_MASK |
						    GDK_POINTER_MOTION_MASK |
						    GDK_BUTTON_RELEASE_MASK |
						    GDK_KEY_PRESS_MASK |
						    GDK_KEY_RELEASE_MASK |
						    GDK_ENTER_NOTIFY_MASK |
						    GDK_FOCUS_CHANGE_MASK |
						    GDK_LEAVE_NOTIFY_MASK |
						    GDK_SCROLL_MASK));
	g_signal_connect(G_OBJECT(m_dArea), "draw",
					   G_CALLBACK(XAP_UnixFrameImpl::_fe::draw), NULL);

	g_signal_connect(G_OBJECT(m_dArea), "key_press_event",
					   G_CALLBACK(XAP_UnixFrameImpl::_fe::key_press_event), NULL);

	g_signal_connect(G_OBJECT(m_dArea), "key_release_event",
					   G_CALLBACK(XAP_UnixFrameImpl::_fe::key_release_event), NULL);

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

	m_grid = gtk_grid_new();
	g_object_set(G_OBJECT(m_grid), "expand", TRUE, NULL);
	g_object_set_data(G_OBJECT(m_grid),"user_data", this);

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
	
	gtk_grid_attach(GTK_GRID(m_grid), m_hScroll, 0, 1, 1, 1);

	gtk_grid_attach(GTK_GRID(m_grid), m_vScroll, 1, 0, 1, 1);


	// arrange the widgets within our inner table.
	m_innergrid = gtk_grid_new();
	g_object_set(G_OBJECT(m_innergrid), "expand", TRUE, NULL);
	gtk_grid_attach(GTK_GRID(m_grid), m_innergrid, 0, 0, 1, 1); 

	if ( bShowRulers )
	{
		gtk_grid_attach(GTK_GRID(m_innergrid), m_topRuler, 0, 0, 2, 1);

		if (m_leftRuler)
			gtk_grid_attach(GTK_GRID(m_innergrid), m_leftRuler, 0, 1, 1, 1);

		gtk_grid_attach(GTK_GRID(m_innergrid), m_dArea,   1, 1, 1, 1); 
	}
	else	// no rulers
	{
		gtk_grid_attach(GTK_GRID(m_innergrid), m_dArea,   1, 1, 1, 1); 
	}
	// create a 3d box and put the table in it, so that we
	// get a sunken in look.
	m_wSunkenBox = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(m_wSunkenBox), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(m_wSunkenBox), m_grid);

	// (scrollbars are shown, only if needed, by _setScrollRange)
	gtk_widget_show(m_dArea);
	gtk_widget_show(m_innergrid);
	gtk_widget_show(m_grid);

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
	GdkPixbuf* icon = NULL;

#if 0 // we don't need to use the theme.
	GtkIconTheme * theme = gtk_icon_theme_get_default();
	icon = gtk_icon_theme_load_icon(theme, "abiword", 48, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
	if (icon)
	{
		gtk_window_set_icon (GTK_WINDOW (window), icon);
		g_object_unref (G_OBJECT(icon));
		return;
	}
#endif
	// Hmm, we can't load the icon from the theme. This happens when we are
	// are installed in a custom prefix, so let's try to load the icon manually.
	GError* error = NULL;
	static const char* s_icon_sizes[] = {
		"16x16",
		"22x22",
		"32x32",
		"48x48",
		"256x256",
		"512x512",
		NULL
	};

	const char** currentSize = s_icon_sizes;
	GList* iconList = NULL;
	while(*currentSize)
	{
		std::string icon_path = std::string(ICONDIR) + "/hicolor/"
			+ *currentSize + "/apps/abiword.png";
		icon = gdk_pixbuf_new_from_file(icon_path.c_str(), &error);
		if (icon)
		{
			iconList = g_list_append(iconList, icon);
		}
		if (error)
		{
			g_warning("Unable to load AbiWord icon %s: %s\n",
				  icon_path.c_str(),
				  error ? error->message : "(null)");
			if (error)
			{
				g_error_free(error);
			}
		}
		currentSize++;
	}
	if (iconList)
	{
		gtk_window_set_icon_list(GTK_WINDOW(window), iconList);
		g_list_free_full(iconList, &g_object_unref);
	}
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
	xxx_UT_DEBUGMSG(("Scroll Adjustment set to %d upper %f size %f\n",iValue, fUpperLimit, fSize));
	GR_Graphics * pGr = getFrame()->getCurrentView()->getGraphics ();
	XAP_Frame::tZoomType tZoom = getFrame()->getZoomType();
	if(pScrollAdjustment) //this isn't guaranteed in AbiCommand
	{
		gtk_adjustment_configure(pScrollAdjustment, iValue, 0.0, fUpperLimit,
                                 pGr->tluD(20.0), fSize, fSize);
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
    if( XAP_App::getApp()->getNoGUI() ) 
        return(UT_RGBColor(0,0,0));

    UT_return_val_if_fail(m_dArea, UT_RGBColor(0,0,0));
    // owen says that any widget should be ok, not just text widgets
    GtkStyleContext *pCtxt = gtk_widget_get_style_context(m_dArea);
    GdkRGBA rgba;
    gtk_style_context_get_background_color(pCtxt, GTK_STATE_FLAG_SELECTED, &rgba);
    return UT_RGBColor (rgba.red * 255, rgba.green * 255, rgba.blue * 255);
}

UT_RGBColor AP_UnixFrameImpl::getColorSelForeground () const
{
  UT_return_val_if_fail(m_dArea, UT_RGBColor(0,0,0));
  
  // owen says that any widget should be ok, not just text widgets
  GtkStateFlags state;
  
  // our text widget has focus
  if (gtk_widget_has_focus(m_dArea))
    state = GTK_STATE_FLAG_SELECTED;
  else
    state = GTK_STATE_FLAG_ACTIVE;
  
  GtkStyleContext *pCtxt = gtk_widget_get_style_context(m_dArea);
  GdkRGBA rgba;
  gtk_style_context_get_color(pCtxt, state, &rgba);
  return UT_RGBColor (rgba.red * 255, rgba.green * 255, rgba.blue * 255);
}
