/*
 * Copyright (C) 2005 Robert Staudinger <robsta@stereolyzer.net>
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


#include <glib-object.h>

#include "ie_types.h"
#include "ev_UnixKeyboard.h"
#include "ev_UnixMouse.h"
#include "fl_DocLayout.h"
#include "fv_View.h"
#include "gr_UnixGraphics.h"
#include "pd_Document.h"
#include "xap_Frame.h"
#include "xap_Dlg_Zoom.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_UnixTopRuler.h"
#include "ap_UnixLeftRuler.h"
#include "ap_UnixFrameImpl.h"
#include "ap_UnixViewListener.h"

#include "abi-widget.h"

/* dirty hack */
#ifdef ABIWORD_INTERNAL
#include "private/ap_WidgetApp.h"
#else
#include <abiword/private/ap_WidgetApp.h>
#endif /* ABIWORD_INTERNAL */

/* TODO */
#define UT_DEBUGMSG(M) printf M
#define P_(M) M


enum {
	_PROP_0,
	PROP_SHOW_RULERS,
	NUM_PROPS
};


static void abi_widget_class_init   (AbiWidgetClass   *klass);
static void abi_widget_init         (AbiWidget        *self);
static void abi_widget_set_property (GObject        *obj,
								   guint		   param_id,
								   const GValue   *val,
								   GParamSpec     *pspec);
static void abi_widget_get_property (GObject        *obj,
								   guint           param_id,
								   GValue         *val,
								   GParamSpec     *pspec);
static void abi_widget_dispose 	  (GObject 		  *obj);
static void abi_widget_finalize 	  (GObject 		  *obj);

/* callbacks */
static void abi_widget_realize_cb (GtkWidget *widget, gpointer data);
void v_scroll_changed_cb (GtkAdjustment *w, gpointer data);
void h_scroll_changed_cb(GtkAdjustment * w, gpointer data);
gint expose_cb(GtkWidget * w, GdkEventExpose* pExposeEvent, gpointer data);
gint key_release_cb (GtkWidget* w, GdkEventKey* e, gpointer data);
gint key_press_cb (GtkWidget* w, GdkEventKey* e, gpointer data);
gint button_press_cb(GtkWidget * w, GdkEventButton * e, gpointer data);
gint button_release_cb(GtkWidget * w, GdkEventButton * e, gpointer data);
gint motion_notify_cb(GtkWidget* w, GdkEventMotion* e, gpointer data);
gint scroll_notify_cb(GtkWidget* w, GdkEventScroll* e, gpointer data);
gint configure_cb (GtkWidget* w, GdkEventConfigure *e, gpointer data);
gint zoom_idle_cb (gpointer data);
static void focus_in_cb  (GtkWidget 	   *d_area, 
						  GdkEventCrossing *event, 
						  gpointer 		    data);
static void focus_out_cb (GtkWidget 	   *d_area, 
						  GdkEventCrossing *event, 
						  gpointer 		    data);


static GtkFrameClass *abi_widget_parent_class = NULL;


GType
abi_widget_get_type (void)
{
	static GType frame_type = 0;

	if (!frame_type) {

		static const GTypeInfo frame_info = {
			sizeof (AbiWidgetClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) abi_widget_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (AbiWidget),
			0,		/* n_preallocs */
			(GInstanceInitFunc) abi_widget_init,
		};

		frame_type = g_type_register_static (GTK_TYPE_FRAME, "AbiWidget", &frame_info, (GTypeFlags)0);
	}

	/* linker hack 
	if (!frame_type) {
		abi_widget_priv_scroll_func_x (NULL, 0, 0);
		abi_widget_priv_scroll_func_y (NULL, 0, 0);	
	}	
	*/

	return frame_type;
}

gboolean
abi_widget_load_file (AbiWidget *self, const gchar *filename)
{
	UT_Error errorCode;

	if (self->doc) {
		/* TODO hmm, how do we release a doc? The dtor is protected
		delete self->doc;
		self->doc = NULL;
		*/
	}

	self->doc = new PD_Document (self->priv->app);
	self->doc->newDocument();
	/* TODO don't know if we have to determine type here and pass e.g. IEFT_AbiWord_1 */
	errorCode = self->doc->readFromFile(filename, IEFT_Unknown);

	if (self->priv->frame->isFrameLocked ()) {

		UT_DEBUGMSG (("abi_widget_load_file() Nasty race bug, please fix me!! \n"));
		UT_ASSERT_HARMLESS(0);
		errorCode = UT_IE_ADDLISTENERERROR;
		goto Err;
	}

	return abi_widget_priv_show_doc (self);

Err:
	/* TODO translate error codes */	
	return FALSE;
}

// TODO continue here
gboolean
abi_widget_priv_show_doc (AbiWidget *self)
{
	AV_ScrollObj * pScrollObj = NULL;
	ap_ViewListener * pViewListener = NULL;
	ap_Scrollbar_ViewListener * pScrollbarViewListener = NULL;
	AV_ListenerId lid;
	AV_ListenerId lidScrollbarViewListener;


	UT_DEBUGMSG(("!!!!!!!!! _showdOCument: Initial zoom is %d \n", self->zoom_percentage));
	self->priv->frame->setFrameLocked(true);


	/// AP_UnixFrame::_createViewGraphics
	XAP_UnixFontManager * fontManager = (static_cast<XAP_UnixApp *>(self->priv->app)->getFontManager());
	UT_ASSERT(self->priv->frame_impl);
	UT_DEBUGMSG(("Got FrameImpl %x area %x \n",self->priv->frame_impl,self->widgets->d_area));
	GR_UnixAllocInfo ai(self->widgets->d_area->window, fontManager, self->priv->app);
	self->graphics = (GR_UnixGraphics*) self->priv->app->newGraphics(ai);
	///


	UT_return_val_if_fail (self->graphics, FALSE);
	self->graphics->setZoomPercentage(self->zoom_percentage);

	FL_DocLayout *layout = new FL_DocLayout(static_cast<PD_Document *>(self->doc), self->graphics);

	self->view = new FV_View(self->priv->app, self->priv->frame, layout);
	self->priv->frame->setView (self->view);

	if(self->zoom_type == XAP_Frame::z_PAGEWIDTH)
	{
		self->zoom_percentage = self->view->calculateZoomPercentForPageWidth();
		self->graphics->setZoomPercentage(self->zoom_percentage);
	}
	else if(self->zoom_type == XAP_Frame::z_WHOLEPAGE)
	{
		self->zoom_percentage = self->view->calculateZoomPercentForWholePage();
		self->graphics->setZoomPercentage(self->zoom_percentage);
	}
	UT_DEBUGMSG(("!!!!!!!!! _showdOCument: zoom is %d \n",self->zoom_percentage));
	self->priv->frame->setZoomPercentage(self->zoom_percentage);


	/// _createScrollBarListeners
	pScrollObj = new AV_ScrollObj(self, abi_widget_priv_scroll_func_x, abi_widget_priv_scroll_func_y);
	UT_return_val_if_fail (pScrollObj, FALSE);

	pViewListener = new ap_UnixViewListener(self->priv->frame);
	pScrollbarViewListener = new ap_Scrollbar_ViewListener(self->priv->frame,self->view);
	
	if (!self->view->addListener(static_cast<AV_Listener *>(pViewListener),&lid))
		return false;
	if (!self->view->addListener(static_cast<AV_Listener *>(pScrollbarViewListener),
							&lidScrollbarViewListener))
		return false;
	///


	/* maybe i'll write an own cleanup method
	abi_widget_priv_replace_view (self, layout, pScrollObj, pViewListener,
			    				pScrollbarViewListener, lid, lidScrollbarViewListener);
	*/
	self->view->addScrollListener(pScrollObj);
	//?pView->setInsertMode((static_cast<AP_FrameData*>(m_pData)->m_bInsertMode));
	//?m_pView->setWindowSize(_getDocumentAreaWidth(), _getDocumentAreaHeight());
	layout->fillLayouts ();
	//?	self->priv->frame_impl->notifyViewChanged(m_pView);
	///

	abi_widget_priv_set_x_scroll_range (self);
	abi_widget_priv_set_y_scroll_range (self);

	self->view->draw();

	if (self->priv->show_rulers) 
	{
		if (self->top_ruler)
			self->top_ruler->draw(NULL);

		if (self->left_ruler)
			self->left_ruler->draw(NULL);
	}

	self->view->notifyListeners(AV_CHG_ALL);
	self->view->focusChange(AV_FOCUS_HERE);

	self->priv->frame->setFrameLocked(false);
	return UT_OK;

Cleanup:
	// clean up anything we created here
	/* TODO check what to clean up here
	DELETEP(pG);
	DELETEP(pDocLayout);
	DELETEP(self->view);
	DELETEP(pViewListener);
	DELETEP(pScrollObj);
	DELETEP(pScrollbarViewListener);

	// change back to prior document
	UNREFP(m_pDoc);
	setFrameLocked(false);
	UT_return_val_if_fail(static_cast<AP_FrameData*>(m_pData)->m_pDocLayout, UT_IE_ADDLISTENERERROR);
	m_pDoc = static_cast<AP_FrameData*>(m_pData)->m_pDocLayout->getDocument();
	//static_cast<XAP_FrameImpl *>(m_pFrameImpl)->setShowDocLocked(false);
	return UT_IE_ADDLISTENERERROR;
	*/
Err: 
	// TODO translate errors
	return FALSE;
}

void
abi_widget_priv_replace_view (AbiWidget *self, FL_DocLayout *pDocLayout,
			    AV_ScrollObj * pScrollObj,
			    ap_ViewListener *pViewListener,
			    ap_Scrollbar_ViewListener *pScrollbarViewListener,
			    AV_ListenerId lid, AV_ListenerId lidScrollbarViewListener)
{
#if 0
	FV_View * pOldView = NULL;
	FL_DocLayout * pOldDocLayout  = NULL;

	// we want to remember point/selection when that is appropriate, which is when the new view is a
	// view of the same doc as the old view, or if this frame is being cloned from an existing frame
		

	// switch to new view, cleaning up previous settings
	if (static_cast<AP_FrameData*>(m_pData)->m_pDocLayout)
	{
		pOldDoc = (static_cast<AP_FrameData*>(m_pData)->m_pDocLayout->getDocument());
		pOldDocLayout = static_cast<AP_FrameData*>(m_pData)->m_pDocLayout;
	}

	REPLACEP(static_cast<AP_FrameData*>(m_pData)->m_pG, pG);
	REPLACEP(static_cast<AP_FrameData*>(m_pData)->m_pDocLayout, pDocLayout);
	

	if (pOldDoc != m_pDoc)
	{
		UNREFP(pOldDoc);
	}


	AV_View * pReplacedView = m_pView;
	m_pView = pView;


	REPLACEP(m_pScrollObj, pScrollObj);
	REPLACEP(m_pViewListener, pViewListener);
	m_lid = lid;
	REPLACEP(m_pScrollbarViewListener, pScrollbarViewListener);
	m_lidScrollbarViewListener = lidScrollbarViewListener;

	m_pView->addScrollListener(m_pScrollObj);

	// Associate the new view with the existing TopRuler, LeftRuler.
	// Because of the binding to the actual on-screen widgets we do
	// not destroy and recreate the TopRuler, LeftRuler when we change
	// views, like we do for all the other objects.  We also do not
	// allocate the TopRuler, LeftRuler  here; that is done as the
	// frame is created.
	if ( static_cast<AP_FrameData*>(m_pData)->m_bShowRuler )
	{
		if ( static_cast<AP_FrameData*>(m_pData)->m_pTopRuler )
			static_cast<AP_FrameData*>(m_pData)->m_pTopRuler->setView(pView, iZoom);
		if ( static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler )
			static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler->setView(pView, iZoom);
	}

	if ( static_cast<AP_FrameData*>(m_pData)->m_pStatusBar && (getFrameMode() != XAP_NoMenusWindowLess))
		static_cast<AP_FrameData*>(m_pData)->m_pStatusBar->setView(pView);
	static_cast<FV_View *>(m_pView)->setShowPara(static_cast<AP_FrameData*>(m_pData)->m_bShowPara);

	pView->setInsertMode((static_cast<AP_FrameData*>(m_pData)->m_bInsertMode));
	m_pView->setWindowSize(_getDocumentAreaWidth(), _getDocumentAreaHeight());

	updateTitle();
	XAP_App * pApp = getApp();
	if(pApp->findFrame(this) < 0)
	{
		pApp->rememberFrame(this);
	}

	pDocLayout->fillLayouts();      

	self->priv->frame_impl->notifyViewChanged(m_pView);

	DELETEP(pReplacedView);
#endif
}

/*
AbiDoc*    
abi_widget_get_doc (AbiWidget *self)
{
	// this should prolly be handled as property
	return self->doc;
}

void	   
abi_widget_set_doc (AbiWidget *self, AbiDoc *doc)
{
	// this should prolly be handled as property
	
	g_return_if_fail (self->doc == NULL);

	self->doc = doc;
	g_object_ref (doc);

	// TODO show document
}
*/

static void 
abi_widget_class_init (AbiWidgetClass *klass)
{
	GObjectClass *gobject_class;
	GtkWidgetClass *widget_class;
	GtkContainerClass *container_class;

	gobject_class = (GObjectClass*) klass;
	widget_class = GTK_WIDGET_CLASS (klass);
	container_class = GTK_CONTAINER_CLASS (klass);

	abi_widget_parent_class = (GtkFrameClass*) g_type_class_peek_parent (klass);

	gobject_class->set_property = abi_widget_set_property;
	gobject_class->get_property = abi_widget_get_property;
	gobject_class->dispose = abi_widget_dispose;
	gobject_class->finalize = abi_widget_finalize;

	
	g_object_class_install_property (gobject_class,
									 PROP_SHOW_RULERS,
									 g_param_spec_string ("show-rulers",
									 P_("show rulers"),	
									 P_("Show or hide rulers"),
									 FALSE,
									 (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));
}

static void
abi_widget_init (AbiWidget *self)
{
	self->view_mode = ABI_VIEW_NORMAL;
	self->priv = (AbiWidgetPrivate*) g_new0 (AbiWidgetPrivate, 1);
	self->priv->show_rulers = FALSE;
	self->priv->is_disposed = FALSE;

	self->widgets = abi_widget_priv_create_widgets (self);
	gtk_container_add (GTK_CONTAINER (self), self->widgets->table);
	gtk_frame_set_shadow_type (GTK_FRAME (self), GTK_SHADOW_IN);

	self->zoom_percentage = 100;

	g_signal_connect_after (G_OBJECT (self->widgets->d_area), "realize", G_CALLBACK (abi_widget_realize_cb), (gpointer)self);
}

static void 
abi_widget_set_property (GObject      *obj,
					   guint         prop_id,
					   const GValue *val,
					   GParamSpec   *pspec)
{
	AbiWidget *self;

	self = ABI_WIDGET (obj);


	switch (prop_id) {
		case PROP_SHOW_RULERS:
			self->priv->show_rulers = g_value_get_int (val);
		    break;
		default:      
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
		    break;
	}
}

static void 
abi_widget_get_property (GObject    *obj,
					   guint       prop_id,
					   GValue     *val,
					   GParamSpec *pspec)
{
	AbiWidget *self;

	self = ABI_WIDGET (obj);


	switch (prop_id) {
		case PROP_SHOW_RULERS:
			g_value_set_boolean (val, self->priv->show_rulers);
		    break;
		default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
		    break;
	}
}

GtkWidget*
abi_widget_new (void)
{
	return GTK_WIDGET (g_object_new (ABI_TYPE_WIDGET, NULL));
}

static void 
abi_widget_dispose (GObject *obj)
{
	AbiWidget *self;

	self = ABI_WIDGET (obj);
	if (self->priv->is_disposed)
		return;

	self->priv->is_disposed = TRUE;

	G_OBJECT_CLASS (abi_widget_parent_class)->dispose (obj);
}

static void 
abi_widget_finalize (GObject *obj)
{
	AbiWidget *self;

	self = ABI_WIDGET (obj);

	g_free (self->priv);
	self->priv = NULL;

	G_OBJECT_CLASS (abi_widget_parent_class)->finalize (obj);
}

static AbiWidgets* 
abi_widget_priv_create_widgets  (AbiWidget *self)
{
	AbiWidgets *widgets = (AbiWidgets*)g_new0 (AbiWidgets, 1);

	if (self->priv->show_rulers) {

		self->top_ruler = new AP_UnixTopRuler(self->priv->frame);
		UT_ASSERT(self->top_ruler);
		
		if (self->view_mode == ABI_VIEW_PRINT) {

		    self->left_ruler = new AP_UnixLeftRuler(self->priv->frame);
		    UT_ASSERT(self->left_ruler);
		}
		else {
		    self->left_ruler = NULL;
		}
	}
	else {
		self->left_ruler = NULL;
		self->top_ruler = NULL;
	}

	// set up for scroll bars.
	widgets->h_adjust = G_OBJECT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
	widgets->h_scroll = gtk_hscrollbar_new (GTK_ADJUSTMENT (widgets->h_adjust));
	g_object_set_data(G_OBJECT(widgets->h_adjust), "user_data", self);
	g_object_set_data(G_OBJECT(widgets->h_scroll), "user_data", self);
	g_signal_connect(G_OBJECT(widgets->h_adjust), "value_changed", G_CALLBACK(h_scroll_changed_cb), self);

	widgets->v_adjust = G_OBJECT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
	widgets->v_scroll = gtk_vscrollbar_new (GTK_ADJUSTMENT (widgets->v_adjust));
	g_object_set_data(G_OBJECT(widgets->v_adjust), "user_data", self);
	g_object_set_data(G_OBJECT(widgets->v_scroll), "user_data", self);
	g_signal_connect(G_OBJECT(widgets->v_adjust), "value_changed", G_CALLBACK(v_scroll_changed_cb), self);

	// we don't want either scrollbar grabbing events from us
	GTK_WIDGET_UNSET_FLAGS(widgets->h_scroll, GTK_CAN_FOCUS);
	GTK_WIDGET_UNSET_FLAGS(widgets->v_scroll, GTK_CAN_FOCUS);

	// create a drawing area in the for our document window.
	widgets->d_area = gtk_drawing_area_new ();
	gtk_widget_set_double_buffered(widgets->d_area, FALSE);
	g_object_set_data(G_OBJECT(widgets->d_area), "user_data", self);
	xxx_UT_DEBUGMSG(("!!! drawing area widgets->d_area created! %x for %x \n",widgets->d_area,self));
	GTK_WIDGET_SET_FLAGS (widgets->d_area, GTK_CAN_FOCUS);	// allow it to be focussed

	gtk_widget_set_events(GTK_WIDGET(widgets->d_area), (GDK_EXPOSURE_MASK |
						    GDK_BUTTON_PRESS_MASK |
						    GDK_POINTER_MOTION_MASK |
						    GDK_BUTTON_RELEASE_MASK |
						    GDK_KEY_PRESS_MASK |
						    GDK_KEY_RELEASE_MASK |
						    GDK_ENTER_NOTIFY_MASK |
						    GDK_LEAVE_NOTIFY_MASK));
	gtk_widget_set_double_buffered(GTK_WIDGET(widgets->d_area), FALSE);
	g_signal_connect(G_OBJECT(widgets->d_area), "expose_event",
					   G_CALLBACK(expose_cb), self);
  
	g_signal_connect(G_OBJECT(widgets->d_area), "key_press_event",
					   G_CALLBACK(key_press_cb), self);

	g_signal_connect(G_OBJECT(widgets->d_area), "key_release_event",
					   G_CALLBACK(key_release_cb), self);

	g_signal_connect(G_OBJECT(widgets->d_area), "button_press_event",
					   G_CALLBACK(button_press_cb), self);

	g_signal_connect(G_OBJECT(widgets->d_area), "button_release_event",
					   G_CALLBACK(button_release_cb), self);

	g_signal_connect(G_OBJECT(widgets->d_area), "motion_notify_event",
					   G_CALLBACK(motion_notify_cb), self);

	g_signal_connect(G_OBJECT(widgets->d_area), "scroll_event",
					   G_CALLBACK(scroll_notify_cb), self);

	g_signal_connect(G_OBJECT(widgets->d_area), "configure_event",
					   G_CALLBACK(configure_cb), self);

	// focus and XIM related
	g_signal_connect(G_OBJECT(widgets->d_area), "enter_notify_event", G_CALLBACK(focus_in_cb), self);
	g_signal_connect(G_OBJECT(widgets->d_area), "leave_notify_event", G_CALLBACK(focus_out_cb), self);

	// create a table for scroll bars, rulers, and drawing area

	widgets->table = gtk_table_new(1, 1, FALSE); //was 1,1
	g_object_set_data(G_OBJECT(widgets->table),"user_data", self);

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
	gtk_table_attach(GTK_TABLE(widgets->table), widgets->h_scroll, 0, 1, 1, 2,
					 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					 (GtkAttachOptions) (GTK_FILL), // was just GTK_FILL
					 0, 0);

	gtk_table_attach(GTK_TABLE(widgets->table), widgets->v_scroll, 1, 2, 0, 1,
					 (GtkAttachOptions) (GTK_FILL), // was just GTK_FILL
					 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					 0, 0);


	// arrange the widgets within our inner table.
	widgets->inner_table = gtk_table_new(2,2,FALSE);
	gtk_table_attach( GTK_TABLE(widgets->table), widgets->inner_table, 0, 1, 0, 1,
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 0, 0); 

	if ( self->priv->show_rulers )
	{
		gtk_table_attach(GTK_TABLE(widgets->inner_table), self->top_ruler->getWidget(), 0, 2, 0, 1,
						 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
						 (GtkAttachOptions)(GTK_FILL),
						 0, 0);

		if (self->left_ruler->getWidget())
			gtk_table_attach(GTK_TABLE(widgets->inner_table), self->left_ruler->getWidget(), 0, 1, 1, 2,
							 (GtkAttachOptions)(GTK_FILL),
							 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
							 0, 0);

		gtk_table_attach(GTK_TABLE(widgets->inner_table), widgets->d_area,   1, 2, 1, 2,
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 0, 0); 
	}
	else	// no rulers
	{
		gtk_table_attach(GTK_TABLE(widgets->inner_table), widgets->d_area,   1, 2, 1, 2,
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 0, 0); 
	}

	// (scrollbars are shown, only if needed, by abi_widget_priv_set_scroll_range)
	gtk_widget_show(widgets->d_area);
	gtk_widget_show(widgets->inner_table);
	gtk_widget_show(widgets->table);

	return widgets;
}

void 
abi_widget_priv_quick_zoom (AbiWidget *self)
{
	guint zoom = 100;
	gboolean changed = FALSE;

	if (!self->view) 
		return;

	switch (self->zoom_type)
	{
	case XAP_Frame::z_PAGEWIDTH:
		zoom = self->view->calculateZoomPercentForPageWidth();
		if      (zoom < XAP_DLG_ZOOM_MINIMUM_ZOOM) zoom = XAP_DLG_ZOOM_MINIMUM_ZOOM;
		else if (zoom > XAP_DLG_ZOOM_MAXIMUM_ZOOM) zoom = XAP_DLG_ZOOM_MAXIMUM_ZOOM;
		changed = TRUE;
		break;
	case XAP_Frame::z_WHOLEPAGE:
		zoom = self->view->calculateZoomPercentForWholePage() ;
		if      (zoom < XAP_DLG_ZOOM_MINIMUM_ZOOM) zoom = XAP_DLG_ZOOM_MINIMUM_ZOOM;
		else if (zoom > XAP_DLG_ZOOM_MAXIMUM_ZOOM) zoom = XAP_DLG_ZOOM_MAXIMUM_ZOOM;
		changed = TRUE;
		break;
	default:
		self->view->updateScreen(false);
		return;
	}

	/* TODO fire "zoom-changed" signal ?
	XAP_Frame::setZoomPercentage(zoom);
	*/

	if (changed || self->zoom_percentage != zoom) 
	{
		self->layout->incrementGraphicTick();
		self->graphics->setZoomPercentage(zoom);
		self->graphics->clearFont();
		if (self->top_ruler)
		{
			self->top_ruler->setZoom(zoom);
		}
		if (self->left_ruler)
		{
			self->left_ruler->setZoom(zoom);
		}
		abi_widget_priv_set_y_scroll_range (self);
		abi_widget_priv_set_x_scroll_range (self);

		// Redraw the entire screen
		self->view->updateScreen(false);
		self->view->setPoint(self->view->getPoint()); // place the cursor correctly
		                                    // on the screen.
		if(self->top_ruler && !self->top_ruler->isHidden())
		{
			self->top_ruler->draw(NULL);
		}
		if(self->left_ruler && !self->left_ruler->isHidden())
		{
			self->left_ruler->draw(NULL);
		}
	}
	else
	{
		self->view->updateScreen(false);
	}

	self->zoom_percentage = zoom;

	// Notify listeners of new zoom (to update the zoom combo in toolbar)
	// We do this regardless of bChanged since a change from 100% zoom to
	// "Page Width" zoom may not change the logical zoom level.
	self->view->notifyListeners(AV_CHG_ALL);	
}

void 
abi_widget_priv_set_x_scroll_range (AbiWidget *self)
{
	GR_Graphics * pGr = self->view->getGraphics ();
	GtkAdjustment *h_adjust;

	int width = self->view->getLayout()->getWidth();
	int windowWidth = static_cast<int>(pGr->tluD (GTK_WIDGET(self->widgets->d_area)->allocation.width));
	
	int newvalue = ((self->view) ? self->view->getXScrollOffset() : 0);
	int newmax = width - windowWidth; /* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;
	
	h_adjust = GTK_ADJUSTMENT (self->widgets->h_adjust);
	bool bDifferentPosition = (newvalue != h_adjust->value);
	bool bDifferentLimits = ((width-windowWidth) != h_adjust->upper-
							                        h_adjust->page_size);
		
	abi_widget_priv_set_scroll_range (self, apufi_scrollX, newvalue, static_cast<gfloat>(width), static_cast<gfloat>(windowWidth));
	
	if (self->view && (bDifferentPosition || bDifferentLimits))
		self->view->sendHorizontalScrollEvent(newvalue, 
										   static_cast<gint>
												(h_adjust->upper-
												 h_adjust->page_size));
}

void 
abi_widget_priv_set_y_scroll_range (AbiWidget *self)
{
	GR_Graphics * pGr = self->view->getGraphics ();
	GtkAdjustment *v_adjust;

	int height = self->view->getLayout()->getHeight();
	int windowHeight = static_cast<int>(pGr->tluD (GTK_WIDGET(self->widgets->d_area)->allocation.height));

	int newvalue = ((self->view) ? self->view->getYScrollOffset() : 0);
	int newmax = height - windowHeight;	/* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;

	v_adjust = GTK_ADJUSTMENT (self->widgets->v_adjust);
	bool bDifferentPosition = (newvalue != static_cast<gint>(v_adjust->value +0.5));
	gint diff = static_cast<gint>(v_adjust->upper-
											v_adjust->page_size +0.5);
	if(bDifferentPosition)
	{
		gint iDU = pGr->tdu( static_cast<gint>(v_adjust->value +0.5) - newvalue);
		if(iDU == 0)
		{
			bDifferentPosition = false;
			v_adjust->value = static_cast<gdouble>(newvalue);
		}
	}
	bool bDifferentLimits = ((height-windowHeight) != diff);
	
	if (self->view && (bDifferentPosition || bDifferentLimits))
	{
		abi_widget_priv_set_scroll_range(self, apufi_scrollY, newvalue, static_cast<gfloat>(height), static_cast<gfloat>(windowHeight));
		self->view->sendVerticalScrollEvent(newvalue, 
										 static_cast<gint>
											   (v_adjust->upper -
												v_adjust->page_size));
	}
}

void 
abi_widget_priv_set_scroll_range (AbiWidget *self, apufi_ScrollType scrollType, int iValue, gfloat fUpperLimit, gfloat fSize)
{
	GtkAdjustment *adjust = (scrollType == apufi_scrollX) ? GTK_ADJUSTMENT (self->widgets->h_adjust) : GTK_ADJUSTMENT (self->widgets->v_adjust);
	GtkWidget *scroll = (scrollType == apufi_scrollX) ? self->widgets->h_scroll : self->widgets->v_scroll;

	GR_Graphics * pGr = self->view->getGraphics ();
	adjust->value = iValue;
	adjust->lower = 0.0;
	adjust->upper = fUpperLimit;
	adjust->step_increment = pGr->tluD(20.0);
	adjust->page_increment = fSize;
	adjust->page_size = fSize;
	g_signal_emit_by_name(G_OBJECT(adjust), "changed");

	// hide the horizontal scrollbar if the scroll range is such that the window can contain it all
	// show it otherwise
	// Hide the horizontal scrollbar if we've set to page width or fit to page.
	// This stops a resizing race condition.
	//
 	if ((self->widgets->h_scroll == scroll) && ((fUpperLimit <= fSize) ||(  self->zoom_type == XAP_Frame::z_PAGEWIDTH) || (self->zoom_type == XAP_Frame::z_WHOLEPAGE)))
	{
 		gtk_widget_hide(scroll);
	}
 	else
	{
 		gtk_widget_show(scroll);
	}
}

extern "C" {

static void 
abi_widget_priv_scroll_func_y (gpointer abi, UT_sint32 yoff, UT_sint32 /*yrange*/)
{	
	AbiWidget *self = NULL;

	self = ABI_WIDGET (abi);

	// we've been notified (via sendVerticalScrollEvent()) of a scroll (probably
	// a keyboard motion).  push the new values into the scrollbar widgets
	// (with clamping).  then cause the view to scroll.
	
	gfloat yoffNew = yoff;
	gfloat yoffMax = GTK_ADJUSTMENT (self->widgets->v_adjust)->upper - GTK_ADJUSTMENT (self->widgets->v_adjust)->page_size;
	if (yoffMax <= 0)
		yoffNew = 0;
	else if (yoffNew > yoffMax)
		yoffNew = yoffMax;

	// we want to twiddle xoffNew such that actual scroll = anticipated scroll.
	// actual scroll is given by the formula xoffNew-pView->getXScrollOffset()

	// this is exactly the same computation that we do in the scrolling code.
	// I don't think we need as much precision anymore, now that we have
	// precise rounding, but it can't really be a bad thing.

	UT_sint32 dy = static_cast<UT_sint32>
		(self->graphics->tluD(static_cast<UT_sint32>(self->graphics->tduD
			   (static_cast<UT_sint32>(self->view->getYScrollOffset()-yoffNew)))));
	gfloat yoffDisc = static_cast<UT_sint32>(self->view->getYScrollOffset()) - dy;

	gtk_adjustment_set_value(GTK_ADJUSTMENT(self->widgets->v_adjust),yoffNew);

	if (self->graphics->tdu(static_cast<UT_sint32>(yoffDisc) - 
				self->view->getYScrollOffset()) != 0)
	self->view->setYScrollOffset(static_cast<UT_sint32>(yoffDisc));
}

static void 
abi_widget_priv_scroll_func_x (gpointer abi, UT_sint32 xoff, UT_sint32 /*xrange*/)
{
	AbiWidget *self = NULL;

	self = ABI_WIDGET (abi);

	// we've been notified (via sendScrollEvent()) of a scroll (probably
	// a keyboard motion).  push the new values into the scrollbar widgets
	// (with clamping).  then cause the view to scroll.

	gfloat xoffNew = xoff;
	gfloat xoffMax = GTK_ADJUSTMENT (self->widgets->h_adjust)->upper - GTK_ADJUSTMENT (self->widgets->h_adjust)->page_size;
	if (xoffMax <= 0)
		xoffNew = 0;
	else if (xoffNew > xoffMax)
		xoffNew = xoffMax;

	// we want to twiddle xoffNew such that actual scroll = anticipated scroll.
	// actual scroll is given by the formula xoffNew-pView->getXScrollOffset()

	// this is exactly the same computation that we do in the scrolling code.
	// I don't think we need as much precision anymore, now that we have
	// precise rounding, but it can't really be a bad thing.

	UT_sint32 dx = static_cast<UT_sint32>
		(self->graphics->tluD(static_cast<UT_sint32>(self->graphics->tduD
			   (static_cast<UT_sint32>(self->view->getXScrollOffset()-xoffNew)))));
	gfloat xoffDisc = static_cast<UT_sint32>(self->view->getXScrollOffset()) - dx;

	gtk_adjustment_set_value(GTK_ADJUSTMENT(self->widgets->h_adjust),xoffDisc);

	// (this is the calculation for dx again, post rounding)
	// This may not actually be helpful, because we could still lose if the
	// second round of rounding gives us the wrong number.  Leave it for now.
	if (self->graphics->tdu(static_cast<UT_sint32>(xoffDisc) - 
				self->view->getXScrollOffset()) != 0)
		self->view->setXScrollOffset(static_cast<UT_sint32>(xoffDisc));
}

} // extern "C"

static void 
abi_widget_realize_cb (GtkWidget *widget, gpointer data)
{
	AbiWidget *self;
	const char *argv[1];
	XAP_Args *args;

	UT_ASSERT (GDK_IS_WINDOW (widget->window));	
	self = ABI_WIDGET (data);

	argv[0] = "AbiWidget";
	args = new XAP_Args (1, argv);
	self->priv->app = new AP_WidgetApp (args, argv[0]);
	UT_DEBUGMSG (("app: %x\n", self->priv->app));
	self->priv->app->initialize (TRUE);

	self->priv->frame_impl = new AP_WidgetFrameImpl (self, NULL, self->priv->app);
	UT_DEBUGMSG (("new frame-impl: %x, drawing-area: %x\n", self->priv->frame_impl, self->priv->frame_impl->getDrawingArea ()));

	self->priv->frame = new AP_WidgetFrame (self, self->priv->app, self->priv->frame_impl);
	self->priv->frame->setFrameImpl (self->priv->frame_impl);
	self->priv->frame->initialize ();
	self->priv->app->setFrame (self->priv->frame);

	self->doc = new PD_Document (self->priv->app);
	self->doc->newDocument ();

/* we probably don't need this, it should be done in _show_doc()
	GR_UnixAllocInfo alloc_info (self->widgets->d_area->window, self->priv->app->getFontManager (), self->priv->app);
	self->graphics = self->priv->app->newGraphics (alloc_info);
	self->graphics->setZoomPercentage (self->zoom_percentage);

	self->layout = new FL_DocLayout (self->doc, self->graphics);
	self->view = new FV_View (self->priv->app, self->priv->frame, self->layout);
	self->layout->setView (self->view);
	self->layout->fillLayouts ();
*/

//TODO enable after loading works	abi_widget_priv_show_doc (self);
}

void 
v_scroll_changed_cb (GtkAdjustment * w, gpointer data)
{
	AbiWidget *self = reinterpret_cast<AbiWidget*>(data);

	if (self->view)
		self->view->sendVerticalScrollEvent(static_cast<gint>(w->value));
}

void 
h_scroll_changed_cb(GtkAdjustment * w, gpointer data)
{
	AbiWidget *self = reinterpret_cast<AbiWidget*>(data);

	if (self->view)
		self->view->sendHorizontalScrollEvent(static_cast<gint>(w->value));
}

gint 
expose_cb(GtkWidget * w, GdkEventExpose* pExposeEvent, gpointer data)
{
	AbiWidget *self = reinterpret_cast<AbiWidget*>(data);

	if((self->priv->do_zoom_update) || (self->priv->zoom_idle_cb_handle != 0))
	{
		return TRUE;
	}

	if(self->view)
	{
		GR_Graphics * pGr = self->view->getGraphics ();
		UT_Rect rClip;
		xxx_UT_DEBUGMSG(("Expose area: x %d y %d width %d  height %d \n",pExposeEvent->area.x,pExposeEvent->area.y,pExposeEvent->area.width,pExposeEvent->area.height));
		rClip.left = pGr->tlu(pExposeEvent->area.x);
		rClip.top = pGr->tlu(pExposeEvent->area.y);
		rClip.width = pGr->tlu(pExposeEvent->area.width)+1;
		rClip.height = pGr->tlu(pExposeEvent->area.height)+1;
		pGr->setExposePending(FALSE);
		self->view->draw(&rClip);
	}
	else {
		// TODO fill self->widgets->d_area with theme background color (like GtkEntry) so we don't get ugly artefacts
	}

	return FALSE;
}

gint 
key_release_cb (GtkWidget* w, GdkEventKey* e, gpointer data)
{
	AbiWidget *self = reinterpret_cast<AbiWidget*>(data);

	// Let IM handle the event first.
	if (gtk_im_context_filter_keypress(self->priv->frame_impl->getIMContext(), e)) {
	    xxx_UT_DEBUGMSG(("IMCONTEXT keyevent swallow: %lu\n", e->keyval));
		self->priv->frame_impl->queueIMReset ();
	    return 0;
	}
	return TRUE;
}

gint 
key_press_cb (GtkWidget* w, GdkEventKey* e, gpointer data)
{
	AbiWidget *self = reinterpret_cast<AbiWidget*>(data);

	// Let IM handle the event first.
	if (gtk_im_context_filter_keypress(self->priv->frame_impl->getIMContext(), e)) {
		self->priv->frame_impl->queueIMReset ();

		if ((e->state & GDK_MOD1_MASK) ||
			(e->state & GDK_MOD3_MASK) ||
			(e->state & GDK_MOD4_MASK))
			return 0;

		// ... else, stop this signal
		g_signal_stop_emission (G_OBJECT(w), 
								g_signal_lookup ("key_press_event", 
												 G_OBJECT_TYPE (w)), 0);
		return 1;
	}

	self->priv->app->setTimeOfLastEvent(e->time);
	ev_UnixKeyboard * pUnixKeyboard = static_cast<ev_UnixKeyboard *>(self->priv->frame->getKeyboard());

	if (self->view)
		pUnixKeyboard->keyPressEvent(self->view, e);

		return 0;
}

gint 
button_press_cb(GtkWidget * w, GdkEventButton * e, gpointer data)
{
	AbiWidget *self = reinterpret_cast<AbiWidget*>(data);
	EV_UnixMouse *mouse = NULL;

/* TODO
	pUnixFrameImpl->setTimeOfLastEvent(e->time);
*/ 
	gtk_grab_add(w);

/* TODO
	pUnixFrameImpl->resetIMContext ();
*/

	if (self->view) {
		mouse = static_cast<EV_UnixMouse*>(self->priv->frame->getMouse ());
		UT_ASSERT (mouse);
		mouse->mouseClick(self->view, e);
	}
	return 1;
}

gint button_release_cb(GtkWidget * w, GdkEventButton * e, gpointer data)
{
	AbiWidget *self = reinterpret_cast<AbiWidget*>(data);
	EV_UnixMouse *mouse = NULL;

	gtk_grab_remove(w);

	if (self->view) {
		mouse = static_cast<EV_UnixMouse*>(self->priv->frame->getMouse ());
		UT_ASSERT (mouse);
		mouse->mouseUp (self->view, e);
	}
	return 1;
}

gint 
motion_notify_cb(GtkWidget* w, GdkEventMotion* e, gpointer data)
{
	AbiWidget *self = reinterpret_cast<AbiWidget*>(data);
	EV_UnixMouse *mouse = NULL;

	if(e->type == GDK_MOTION_NOTIFY)
	{
		//
		// swallow queued drag events and just get the last one.
		//
		GdkEvent  * eNext = gdk_event_peek();
		if(eNext && eNext->type == GDK_MOTION_NOTIFY)
		{
			g_object_unref(G_OBJECT(e));
			e = reinterpret_cast<GdkEventMotion *>(eNext);
			while(eNext && eNext->type == GDK_MOTION_NOTIFY)
			{
				xxx_UT_DEBUGMSG(("Swallowing drag event \n"));
				gdk_event_free(eNext);
				eNext = gdk_event_get();
				gdk_event_free(reinterpret_cast<GdkEvent *>(e));
				e = reinterpret_cast<GdkEventMotion *>(eNext);
				eNext = gdk_event_peek();
			}
			if(eNext != NULL)
			{
				gdk_event_free(eNext);
			}
		}
	}

	if (self->view) {
		mouse = static_cast<EV_UnixMouse*>(self->priv->frame->getMouse ());
		UT_ASSERT (mouse); 
		/* UT_DEBUGMSG (("motion_notify_cb() mouse: %x\n", mouse));*/
		mouse->mouseMotion(self->view, e);
	}
	return 1;
}

gint 
scroll_notify_cb(GtkWidget* w, GdkEventScroll* e, gpointer data)
{
	AbiWidget *self = reinterpret_cast<AbiWidget*>(data);
	EV_UnixMouse *mouse = NULL;

	if (self->view) {
		mouse = static_cast<EV_UnixMouse*>(self->priv->frame->getMouse ());
		UT_ASSERT (mouse);
		mouse->mouseScroll(self->view, e);
	}
	return 1;
}

/*
* This is basically a resize event.
*/
gint 
configure_cb (GtkWidget* w, GdkEventConfigure *event, gpointer data)
{
	AbiWidget *self = reinterpret_cast<AbiWidget*>(data);

	UT_DEBUGMSG (("configure_cb() view: %x\n", self->view));

	if (self->view) {

		// Dynamic Zoom Implementation

		self->priv->new_width = event->width;
		self->priv->new_height = event->height;

		if (!self->priv->do_zoom_update && (self->priv->zoom_idle_cb_handle == 0))
		{
			self->priv->zoom_idle_cb_handle = g_idle_add (zoom_idle_cb, self);
		}
	}

	return 1;
}

/*!
 * Background zoom updater. It updates the view zoom level after all configure
 * events have been processed. This is
 */
gint 
zoom_idle_cb (gpointer data)
{
	AbiWidget *self = reinterpret_cast<AbiWidget*>(data);

	UT_DEBUGMSG (("zoom_idle_cb() view: %x\n", self->view));	
	
	if (!self->view || 
	    (self->priv->do_zoom_update && (self->view->getGraphics()->tdu(self->view->getWindowWidth()) == self->priv->new_width) && 
		(self->view->getGraphics()->tdu(self->view->getWindowHeight()) == self->priv->new_height))
	   )
	{
		self->priv->zoom_idle_cb_handle = 0;
		self->priv->do_zoom_update = FALSE;
		return FALSE;
	}

	self->priv->do_zoom_update = TRUE;

	gint new_width = 0;
	gint new_height = 0;
	do
	{
		if(!self->view)
		{
			self->priv->zoom_idle_cb_handle = 0;
			self->priv->do_zoom_update = FALSE;
			return FALSE;
		}

		// oops, we're not ready yet.
		if (self->view->isLayoutFilling())
			return TRUE;

		new_width = self->priv->new_width;
		new_height = self->priv->new_height;

		// don't need this for quickZoom
#if 0
		pUnixFrameImpl->_startViewAutoUpdater(); 
#endif
		self->view->setWindowSize(new_width, new_height);
		abi_widget_priv_quick_zoom (self); // was update zoom
	}
	while((new_width != self->priv->new_width) || (new_height != self->priv->new_height));

	self->priv->zoom_idle_cb_handle = 0;
	self->priv->do_zoom_update = FALSE;
	return FALSE;
}

static void
focus_in_cb (GtkWidget 		  *d_area, 
			 GdkEventCrossing *event, 
			 gpointer 		   data)
{
	gtk_widget_grab_focus (d_area);
}

static void
focus_out_cb (GtkWidget 	   *d_area, 
			  GdkEventCrossing *event, 
			  gpointer 			data)
{}
