/* The AbiWord Widget
 *
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001 Dom Lachowicz <cinamod@hotmail.com>
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

#include <string.h>

#include "abiwidget.h"
#include "ap_UnixFrame.h"
#include "ap_UnixApp.h"
#include "gr_UnixGraphics.h"
#include "ap_Preview_Abi.h"
#include "ev_EditMethod.h"
#include "ut_assert.h"

// Our widget's private storage data
// UnixApp and UnixFrame already properly inherit from either
// Their GTK+ or GNOME XAP equivalents, so we don't need to worry
// About that here
struct _AbiPrivData {
	AP_UnixApp           * m_pApp;
	AP_UnixFrame         * m_pFrame;
	GR_UnixGraphics      * m_pGr;
	AP_Preview_Abi       * m_pPreview;
	char                 * m_szFilename;
};

// our parent class
static GtkWidgetClass * abi_widget_parent_class = 0;

/**************************************************************************/
/**************************************************************************/

// Here, we have some macros that:
// 1) Define the desired name for an EditMethod (EM_NAME)
// 2) Actually define a static function which maps directly 
//    onto an abi EditMethod (the rest)
//
// All that these functions do is marshall data into and out of
// the AbiWord application

#define EM_NAME(n) abi_em_##n

#define EM_CHARPTR_INT_INT__BOOL(n) \
static gboolean EM_NAME(n) (AbiWidget * w, const char * str, gint32 x, gint32 y) \
{ \
return abi_widget_invoke (w, #n, str, x, y); \
}

#define EM_VOID__BOOL(n) \
static gboolean EM_NAME(n) (AbiWidget * w)\
{ \
return abi_widget_invoke (w, #n, 0, 0, 0); \
}

#define EM_INT_INT__BOOL(n) \
static gboolean EM_NAME(n) (AbiWidget * w, gint32 x, gint32 y) \
{ \
return abi_widget_invoke (w, #n, 0, x, y); \
}

#define EM_CHARPTR__BOOL(n) \
static gboolean EM_NAME(n) (AbiWidget * w, const char * str) \
{ \
return abi_widget_invoke (w, #n, str, 0, 0); \
}

/**************************************************************************/
/**************************************************************************/

// Here we define our EditMethods (which will later be mapped back onto
// Our AbiWidgetClass' member functions)

EM_CHARPTR__BOOL(insertData);

EM_VOID__BOOL(alignCenter);
EM_VOID__BOOL(alignLeft);
EM_VOID__BOOL(alignRight);
EM_VOID__BOOL(alignJustify);
EM_VOID__BOOL(copy);
EM_VOID__BOOL(cut);
EM_VOID__BOOL(dlgAbout);
EM_VOID__BOOL(print);
EM_VOID__BOOL(find);
EM_VOID__BOOL(undo);
EM_VOID__BOOL(redo);
EM_VOID__BOOL(insertSpace);
EM_VOID__BOOL(selectAll);
EM_VOID__BOOL(toggleBold);
EM_VOID__BOOL(toggleUline);
EM_VOID__BOOL(toggleItalic);
EM_VOID__BOOL(viewPara);

/**************************************************************************/
/**************************************************************************/

// cleanup - keep EM_NAME around because it's actually useful elsewhere

#undef EM_VOID__BOOL
#undef EM_CHARPTR__BOOL
#undef EM_INT_INT__BOOL
#undef EM_CHARPTR_INT_INT__BOOL

/**************************************************************************/
/**************************************************************************/

#define ABI_DEFAULT_WIDTH 250
#define ABI_DEFAULT_HEIGHT 250

static void 
abi_widget_size_request (GtkWidget      *widget,
						 GtkRequisition *requisition)
{
	// TODO: possibly be smarter about sizes

	requisition->width = ABI_DEFAULT_WIDTH;
	requisition->height = ABI_DEFAULT_HEIGHT;
}

static void
abi_widget_size_allocate (GtkWidget     *widget,
						  GtkAllocation *allocation)
{
	AbiWidget *abi;
	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_ABI_WIDGET (widget));
	g_return_if_fail (allocation != NULL);
	
	widget->allocation = *allocation;
	if (GTK_WIDGET_REALIZED (widget))
    {
		// only allocate on realized widgets

		abi = ABI_WIDGET(widget);
		
		gdk_window_move_resize (widget->window,
								allocation->x, allocation->y,
								allocation->width, allocation->height);

		AP_UnixFrame * pFrame = abi->priv->m_pFrame;

		if (pFrame)
		{
			// TODO: set size of the frame and then queue a resize
			// TODO: we can run into all sorts of problems
			// TODO: with us not having a toplevel window for resizing
			// TODO: and for centering dialogs. deal with this later

			// pFrame->queue_resize();
		}
    }
}

static gint
abi_widget_expose (GtkWidget      *widget,
				   GdkEventExpose *event)
{
	AbiWidget * abi;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (IS_ABI_WIDGET (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	
	if (event->count > 0)
		return FALSE;
	
	abi = ABI_WIDGET (widget);

	// call expose on the graphics context/preview widget...
	if (abi->priv->m_pPreview)
		abi->priv->m_pPreview->draw();

	return FALSE;
}

static void
abi_widget_realize (GtkWidget * widget)
{
	AbiWidget * abi;
	GdkWindowAttr attributes;
	gint attributes_mask;

	// we *must* ensure that we get a GdkWindow to draw into
	// this here is just boilerplate GTK+ code

	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_ABI_WIDGET(widget));

	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
	abi = ABI_WIDGET(widget);

	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.event_mask = gtk_widget_get_events (widget) | 
		GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | 
		GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
		GDK_POINTER_MOTION_HINT_MASK;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);
	
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
	widget->window = gdk_window_new (widget->parent->window, &attributes, 
									 attributes_mask);
	
	widget->style = gtk_style_attach (widget->style, widget->window);
	
	gdk_window_set_user_data (widget->window, widget);
	
	gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);

	// now we can set up Abi inside of this GdkWindow

	XAP_Args *pArgs = 0;
	if (abi->priv->m_szFilename)
	{
		pArgs = new XAP_Args (1, &abi->priv->m_szFilename);
	}
	else
	{
		pArgs = new XAP_Args(0, 0);
	}

	AP_UnixApp   * pApp = new AP_UnixApp (pArgs, "AbiWidget");
	UT_ASSERT(pApp);
	pApp->initialize();

	AP_UnixFrame * pFrame  = new AP_UnixFrame(pApp);
	UT_ASSERT(pFrame);
	pFrame->initialize();
	
	GR_UnixGraphics * pGr = new GR_UnixGraphics (widget->window,
												 pApp->getFontManager(), pApp);
	UT_ASSERT(pGr);

	AP_Preview_Abi * pPreview = new AP_Preview_Abi (pGr, ABI_DEFAULT_WIDTH,
													ABI_DEFAULT_HEIGHT, pFrame,
													PREVIEW_ADJUSTED_PAGE);
	UT_ASSERT(pPreview);

	abi->priv->m_pApp     = pApp;
	abi->priv->m_pFrame   = pFrame;
	abi->priv->m_pGr      = pGr;
	abi->priv->m_pPreview = pPreview;
	
	delete pArgs;
}

static void
abi_widget_destroy (GtkObject *object)
{
	AbiWidget * abi;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_ABI_WIDGET(object));

	// here we free any self-created data
	abi = ABI_WIDGET(object);

	// order of deletion is important here
	delete abi->priv->m_pPreview;
	delete abi->priv->m_pApp;
	delete abi->priv->m_pFrame;
	delete abi->priv->m_pGr;

	g_free (abi->priv->m_szFilename);
	g_free (abi->priv);

	// chain up
	if (GTK_OBJECT_CLASS (abi_widget_parent_class)->destroy)
		GTK_OBJECT_CLASS (abi_widget_parent_class)->destroy (object);
}

static void
abi_widget_init (AbiWidget * abi)
{
	// this isn't really needed, since each widget is
	// guaranteed to be created with g_new0 and we just
	// want everything to be 0/NULL/FALSE anyway right now
	// but i'm keeping it around anyway just in case that changes
}

static void
abi_widget_class_init (AbiWidgetClass *abi_class)
{
	GtkObjectClass * object_class;
	GtkWidgetClass * widget_class;

	object_class = (GtkObjectClass *)abi_class;
	widget_class = (GtkWidgetClass *)abi_class;

	// we need our own special destroy function
	object_class->destroy  = abi_widget_destroy;

	// set our parent class
	abi_widget_parent_class = (GtkWidgetClass *)
		gtk_type_class (gtk_widget_get_type());
	
	// set our custom destroy method
	object_class->destroy = abi_widget_destroy; 

	// set our custom class methods
	widget_class->realize       = abi_widget_realize;
	widget_class->expose_event  = abi_widget_expose;
	widget_class->size_request  = abi_widget_size_request;
	widget_class->size_allocate = abi_widget_size_allocate; 

	// AbiWidget's master "invoke" method
	abi_class->invoke = abi_widget_invoke;

	// assign handles to Abi's edit methods
	abi_class->align_center     = EM_NAME(alignCenter);
	abi_class->align_left       = EM_NAME(alignLeft);
	abi_class->align_right      = EM_NAME(alignRight);
	abi_class->align_justify    = EM_NAME(alignJustify);
	abi_class->copy             = EM_NAME(copy);
	abi_class->cut              = EM_NAME(cut);
	abi_class->dlg_about        = EM_NAME(dlgAbout);
	abi_class->dlg_find         = EM_NAME(find);
	abi_class->insert_data      = EM_NAME(insertData);
	abi_class->insert_space     = EM_NAME(insertSpace);
	abi_class->print            = EM_NAME(print);
	abi_class->undo             = EM_NAME(undo);
	abi_class->redo             = EM_NAME(redo);
	abi_class->select_all       = EM_NAME(selectAll);
	abi_class->toggle_bold      = EM_NAME(toggleBold);
	abi_class->toggle_italic    = EM_NAME(toggleItalic);
	abi_class->toggle_underline = EM_NAME(toggleUline);
	abi_class->view_para        = EM_NAME(viewPara);
}

extern "C" GtkType
abi_widget_get_type (void)
{
	// boilerplate code

	static GtkType abi_type = 0;

	if (!abi_type){
		GtkTypeInfo info = {
			"AbiWidget",
			sizeof (AbiWidget),
			sizeof (AbiWidgetClass),
			(GtkClassInitFunc) abi_widget_class_init,
			(GtkObjectInitFunc) abi_widget_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};
		
		abi_type = gtk_type_unique (gtk_widget_get_type (), &info);
	}
	
	return abi_type;
}

static void
abi_widget_construct (AbiWidget * abi, const char * file)
{
	AbiPrivData * priv = g_new0 (AbiPrivData, 1);

	// this is all that we can do here, because we can't draw until we're
	// realized and have a GdkWindow pointer

	if (file)
		priv->m_szFilename = g_strdup (file);

	abi->priv = priv;
}

/**
 * abi_widget_new
 *
 * Creates a new AbiWord widget
 */
extern "C" GtkWidget *
abi_widget_new (void)
{
	AbiWidget * abi;

	abi = (AbiWidget *)gtk_type_new (abi_widget_get_type ());
	abi_widget_construct (abi, 0);

	return GTK_WIDGET (abi);
}

/**
 * abi_widget_new_with_file
 *
 * Creates a new AbiWord widget and tries to load the @file
 *
 * \param file - A file on your HD
 */
extern "C" GtkWidget *
abi_widget_new_with_file (const gchar * file)
{
	AbiWidget * abi;

	g_return_val_if_fail (file != 0, 0);

	abi = (AbiWidget *)gtk_type_new (abi_widget_get_type ());
	abi_widget_construct (abi, file);

	return GTK_WIDGET (abi);
}

/**
 * abi_widget_invoke()
 *
 * Invoke any of abiword's edit methods by name
 *
 * \param w - An AbiWord widget
 * \param mthdName - A null-terminated string representing the method's name
 * \param data - an optional null-terminated string data to pass to the method
 * \param x - an optional x-coordinate to pass to the method (usually 0)
 * \param y - an optional y-coordinate to pass to the method (usuall 0)
 *
 * \return FALSE if any preconditions fail
 * \return TRUE|FALSE depending on the result of the EditMethod's completion
 *
 * example usage:
 *
 * gboolean b = FALSE; 
 * GtkWidget * w = abi_widget_new ();
 *
 * b = abi_widget_invoke (ABI_WIDGET(w), "insertData", "Hello World!", 0, 0);
 * b = abi_widget_invoke (ABI_WIDGET(w), "alignCenter", 0, 0, 0);
 *
 */
extern "C" gboolean
abi_widget_invoke (AbiWidget * w, const char * mthdName, 
				   const char * data, gint32 x, gint32 y)
{
	EV_EditMethodContainer * container;
	EV_EditMethod          * method;
	EV_EditMethod_pFn        function;
	AV_View                * view;

	// lots of conditional returns - code defensively
	g_return_val_if_fail (w != 0, FALSE);
	g_return_val_if_fail (mthdName != 0, FALSE);
	g_return_val_if_fail (w->priv->m_pApp != 0, FALSE);

	// get the method container
	container = w->priv->m_pApp->getEditMethodContainer();
	g_return_val_if_fail (container != 0, FALSE);

	// get a handle to the actual EditMethod
	method = container->findEditMethodByName (mthdName);
	g_return_val_if_fail (method != 0, FALSE);

	// get a pointer to the method's C/C++ function
	function = method->getFn();
	g_return_val_if_fail (function != 0, FALSE);

	// obtain a valid view
	view = w->priv->m_pFrame->getCurrentView();
	g_return_val_if_fail (view != 0, FALSE);

	// construct the call data
	EV_EditMethodCallData calldata(data, (data ? strlen (data) : 0));
	calldata.m_xPos = x;
	calldata.m_yPos = y;

	// actually invoke
	return ((*function)(view, &calldata) ? TRUE : FALSE);
}
