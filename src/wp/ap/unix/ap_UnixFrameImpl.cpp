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
#include "gr_UnixGraphics.h"
#include "ut_debugmsg.h"

#ifdef ABISOURCE_LICENSED_TRADEMARKS
#include "abiword_48_tm.xpm"
#else
#include "abiword_48.xpm"
#endif

AP_UnixFrameImpl::AP_UnixFrameImpl(AP_UnixFrame *pUnixFrame, XAP_UnixApp *pUnixApp) :
	XAP_UnixFrameImpl(static_cast<XAP_Frame *>(pUnixFrame), static_cast<AP_App *>(pUnixApp)),
	m_dArea(NULL),
	m_pVadj(NULL),
	m_pHadj(NULL),
	m_hScroll(NULL),
	m_vScroll(NULL),
	m_topRuler(NULL),
	m_leftRuler(NULL),
	m_table(NULL),
	m_innertable(NULL),
	m_wSunkenBox(NULL)
{
	UT_DEBUGMSG(("Created AP_UnixFrameImpl %x \n",this));
}

XAP_FrameImpl * AP_UnixFrameImpl::createInstance(XAP_Frame *pFrame, XAP_App *pApp)
{
	XAP_FrameImpl *pFrameImpl = new AP_UnixFrameImpl(static_cast<AP_UnixFrame *>(pFrame), static_cast<XAP_UnixApp *>(pApp));

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
		EV_UnixToolbar * pUnixToolbar = (EV_UnixToolbar *)m_vecToolbars.getNthItem(k);
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
	XAP_Frame* pFrame = getFrame();
	bool bShowStatusBar = static_cast<AP_FrameData*> (pFrame->getFrameData())->m_bShowStatusBar;
	static_cast<AP_UnixFrame *>(pFrame)->toggleStatusBar(bShowStatusBar);
}

static void
focus_in_event (GtkWidget * drawing_area, GdkEventCrossing *event, AP_UnixFrameImpl * me)
{
  gtk_widget_grab_focus (drawing_area);
  me->focusIMIn ();
}

static void
focus_out_event (GtkWidget * drawing_area, GdkEventCrossing * event, AP_UnixFrameImpl * me)
{
  me->focusIMOut ();
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

	((AP_FrameData*)pFrame->getFrameData())->m_pTopRuler = pUnixTopRuler;
	((AP_FrameData*)pFrame->getFrameData())->m_pLeftRuler = pUnixLeftRuler;

	// set up for scroll bars.
	m_pHadj = (GtkAdjustment*) gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	gtk_object_set_user_data(GTK_OBJECT(m_pHadj), this);
	m_hScroll = gtk_hscrollbar_new(m_pHadj);
	gtk_object_set_user_data(GTK_OBJECT(m_hScroll), this);

	g_signal_connect(G_OBJECT(m_pHadj), "value_changed", G_CALLBACK(XAP_UnixFrameImpl::_fe::hScrollChanged), NULL);

	m_pVadj = (GtkAdjustment*) gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	gtk_object_set_user_data(GTK_OBJECT(m_pVadj), this);
	m_vScroll = gtk_vscrollbar_new(m_pVadj);
	gtk_object_set_user_data(GTK_OBJECT(m_vScroll), this);

	g_signal_connect(G_OBJECT(m_pVadj), "value_changed", G_CALLBACK(XAP_UnixFrameImpl::_fe::vScrollChanged), NULL);

	// we don't want either scrollbar grabbing events from us
	GTK_WIDGET_UNSET_FLAGS(m_hScroll, GTK_CAN_FOCUS);
	GTK_WIDGET_UNSET_FLAGS(m_vScroll, GTK_CAN_FOCUS);

	// create a drawing area in the for our document window.
	m_dArea = createDrawingArea ();
	UT_DEBUGMSG(("!!! drawing area m_dArea created! %x for %x \n",m_dArea,this));
	GTK_WIDGET_SET_FLAGS (m_dArea, GTK_CAN_FOCUS);	// allow it to be focussed

	gtk_object_set_user_data(GTK_OBJECT(m_dArea), this);
	gtk_widget_set_events(GTK_WIDGET(m_dArea), (GDK_EXPOSURE_MASK |
						    GDK_BUTTON_PRESS_MASK |
						    GDK_POINTER_MOTION_MASK |
						    GDK_BUTTON_RELEASE_MASK |
						    GDK_KEY_PRESS_MASK |
						    GDK_KEY_RELEASE_MASK |
						    GDK_ENTER_NOTIFY_MASK |
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
	g_signal_connect(G_OBJECT(m_dArea), "enter_notify_event", G_CALLBACK(focus_in_event), this);
	g_signal_connect(G_OBJECT(m_dArea), "leave_notify_event", G_CALLBACK(focus_out_event), this);

	// create a table for scroll bars, rulers, and drawing area

	m_table = gtk_table_new(1, 1, FALSE); //was 1,1
	gtk_object_set_user_data(GTK_OBJECT(m_table),this);

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

void AP_UnixFrameImpl::_setWindowIcon()
{
	// attach program icon to window
	GtkWidget * window = getTopLevelWindow();
	UT_ASSERT(window);

	// create a pixmap from our included data
	GdkBitmap * mask;
	GdkPixmap * pixmap = gdk_pixmap_create_from_xpm_d(window->window, &mask, NULL, abiword_48_xpm);
	UT_ASSERT(pixmap && mask);

	gdk_window_set_icon(window->window, NULL, pixmap, mask);
	gdk_window_set_icon_name(window->window, "AbiWord Application Icon");
}

void AP_UnixFrameImpl::_createWindow()
{
	createTopLevelWindow();
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
}

GtkWidget * AP_UnixFrameImpl::_createStatusBarWindow()
{
	XAP_Frame* pFrame = getFrame();
	AP_UnixStatusBar * pUnixStatusBar = new AP_UnixStatusBar(pFrame);
	UT_ASSERT(pUnixStatusBar);

	((AP_FrameData *)pFrame->getFrameData())->m_pStatusBar = pUnixStatusBar;
	
	GtkWidget * w = pUnixStatusBar->createWidget();

	return w;
}

void AP_UnixFrameImpl::_setScrollRange(apufi_ScrollType scrollType, int iValue, gfloat fUpperLimit, gfloat fSize)
{
	GtkAdjustment *pScrollAdjustment = (scrollType == apufi_scrollX) ? m_pHadj : m_pVadj;
	GtkWidget *wScrollWidget = (scrollType == apufi_scrollX) ? m_hScroll : m_vScroll;

	GR_Graphics * pGr = getFrame()->getCurrentView()->getGraphics ();

	pScrollAdjustment->value = iValue;
	pScrollAdjustment->lower = 0.0;
	pScrollAdjustment->upper = fUpperLimit;
	pScrollAdjustment->step_increment = pGr->tluD(20.0);
	pScrollAdjustment->page_increment = fSize;
	pScrollAdjustment->page_size = fSize;
	g_signal_emit_by_name(G_OBJECT(pScrollAdjustment), "changed");

	// hide the scrollbar if the scroll range is such that the window can contain it all
	// show it otherwise
 	if (fUpperLimit <= fSize)
 		gtk_widget_hide(wScrollWidget);
 	else
 		gtk_widget_show(wScrollWidget);
}

UT_RGBColor AP_UnixFrameImpl::getColorSelBackground () const
{
  // owen says that any widget should be ok, not just text widgets
  gint state;
  
  // our text widget has focus
  if (GTK_WIDGET_HAS_FOCUS(m_dArea))
    state = GTK_STATE_SELECTED;
  else
    state = GTK_STATE_ACTIVE;
  
  GdkColor clr = m_dArea->style->base[state];
  return UT_RGBColor (clr.red >> 8, clr.green >> 8, clr.blue >> 8);
}

UT_RGBColor AP_UnixFrameImpl::getColorSelForeground () const
{
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
