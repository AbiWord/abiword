/* The AbiWord Widget
 *
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001,2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2002 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#define ABIWORD_INTERNAL

#include <string.h>

#include "abiwidget.h"
#include "ap_UnixFrame.h"
#include "gr_UnixGraphics.h"
#include "ev_EditMethod.h"
#include "ut_assert.h"
#include "fv_View.h"

// Our widget's private storage data
// UnixApp and UnixFrame already properly inherit from either
// Their GTK+ or GNOME XAP equivalents, so we don't need to worry
// About that here
struct _AbiPrivData {
	AP_UnixApp           * m_pApp;
	AP_UnixFrame         * m_pFrame;
	char                 * m_szFilename;
	GdkICAttr            * ic_attr;
	GdkIC                * ic;
	bool                 externalApp;
};

//
// Our widget's arguments. 
//
enum {
  ARG_0,
  CURSOR_ON,
  INVOKE_NOARGS,
  MAP_TO_SCREEN,
  DRAW,
  LOAD_FILE,
  ARG_LAST
};

// our parent class
static GtkBinClass * parent_class = 0;

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
return abi_widget_invoke_ex (w, #n, str, x, y); \
}

#define EM_VOID__BOOL(n) \
static gboolean EM_NAME(n) (AbiWidget * w)\
{ \
return abi_widget_invoke_ex (w, #n, 0, 0, 0); \
}

#define EM_INT_INT__BOOL(n) \
static gboolean EM_NAME(n) (AbiWidget * w, gint32 x, gint32 y) \
{ \
return abi_widget_invoke_ex (w, #n, 0, x, y); \
}

#define EM_CHARPTR__BOOL(n) \
static gboolean EM_NAME(n) (AbiWidget * w, const char * str) \
{ \
return abi_widget_invoke_ex (w, #n, str, 0, 0); \
}

/**************************************************************************/
/**************************************************************************/

// Here we define our EditMethods (which will later be mapped back onto
// Our AbiWidgetClass' member functions)

EM_VOID__BOOL(alignCenter);
EM_VOID__BOOL(alignLeft);
EM_VOID__BOOL(alignRight);
EM_VOID__BOOL(alignJustify);

EM_VOID__BOOL(copy);
EM_VOID__BOOL(cut);
EM_VOID__BOOL(paste);
EM_VOID__BOOL(pasteSpecial);
EM_VOID__BOOL(selectAll);
EM_VOID__BOOL(selectBlock);
EM_VOID__BOOL(selectLine);
EM_VOID__BOOL(selectWord);

EM_VOID__BOOL(undo);
EM_VOID__BOOL(redo);

EM_CHARPTR__BOOL(insertData);
EM_VOID__BOOL(insertSpace);

EM_VOID__BOOL(delBOB);
EM_VOID__BOOL(delBOD);
EM_VOID__BOOL(delBOL);
EM_VOID__BOOL(delBOW);
EM_VOID__BOOL(delEOB);
EM_VOID__BOOL(delEOD);
EM_VOID__BOOL(delEOL);
EM_VOID__BOOL(delEOW);
EM_VOID__BOOL(delLeft);
EM_VOID__BOOL(delRight);

EM_VOID__BOOL(editHeader);
EM_VOID__BOOL(editFooter);
EM_VOID__BOOL(removeHeader);
EM_VOID__BOOL(removeFooter);

EM_VOID__BOOL(extSelBOB);
EM_VOID__BOOL(extSelBOD);
EM_VOID__BOOL(extSelBOL);
EM_VOID__BOOL(extSelBOW);
EM_VOID__BOOL(extSelEOB);
EM_VOID__BOOL(extSelEOD);
EM_VOID__BOOL(extSelEOL);
EM_VOID__BOOL(extSelEOW);
EM_VOID__BOOL(extSelLeft);
EM_VOID__BOOL(extSelNextLine);
EM_VOID__BOOL(extSelPageDown);
EM_VOID__BOOL(extSelPageUp);
EM_VOID__BOOL(extSelPrevLine);
EM_VOID__BOOL(extSelRight);
EM_VOID__BOOL(extSelScreenDown);
EM_VOID__BOOL(extSelScreenUp);
EM_INT_INT__BOOL(extSelToXY);

EM_VOID__BOOL(toggleBold);
EM_VOID__BOOL(toggleBottomline);
EM_VOID__BOOL(toggleInsertMode);
EM_VOID__BOOL(toggleItalic);
EM_VOID__BOOL(toggleOline);
EM_VOID__BOOL(togglePlain);
EM_VOID__BOOL(toggleStrike);
EM_VOID__BOOL(toggleSub);
EM_VOID__BOOL(toggleSuper);
EM_VOID__BOOL(toggleTopline);
EM_VOID__BOOL(toggleUline);
EM_VOID__BOOL(toggleUnIndent);

EM_VOID__BOOL(viewPara);
EM_VOID__BOOL(viewPrintLayout);
EM_VOID__BOOL(viewNormalLayout);
EM_VOID__BOOL(viewWebLayout);

EM_VOID__BOOL(warpInsPtBOB);
EM_VOID__BOOL(warpInsPtBOD);
EM_VOID__BOOL(warpInsPtBOL);
EM_VOID__BOOL(warpInsPtBOP);
EM_VOID__BOOL(warpInsPtBOW);
EM_VOID__BOOL(warpInsPtEOB);
EM_VOID__BOOL(warpInsPtEOD);
EM_VOID__BOOL(warpInsPtEOL);
EM_VOID__BOOL(warpInsPtEOP);
EM_VOID__BOOL(warpInsPtEOW);
EM_VOID__BOOL(warpInsPtLeft);
EM_VOID__BOOL(warpInsPtNextLine);
EM_VOID__BOOL(warpInsPtNextPage);
EM_VOID__BOOL(warpInsPtNextScreen);
EM_VOID__BOOL(warpInsPtPrevLine);
EM_VOID__BOOL(warpInsPtPrevPage);
EM_VOID__BOOL(warpInsPtPrevScreen);
EM_VOID__BOOL(warpInsPtRight);
EM_INT_INT__BOOL(warpInsPtToXY);

EM_VOID__BOOL(zoom100);
EM_VOID__BOOL(zoom200);
EM_VOID__BOOL(zoom50);
EM_VOID__BOOL(zoom75);
EM_VOID__BOOL(zoomWhole);
EM_VOID__BOOL(zoomWidth);

/**************************************************************************/
/**************************************************************************/

// cleanup - keep EM_NAME around because it's actually useful elsewhere

#undef EM_VOID__BOOL
#undef EM_CHARPTR__BOOL
#undef EM_INT_INT__BOOL
#undef EM_CHARPTR_INT_INT__BOOL

/**************************************************************************/
/**************************************************************************/

static const guint32 ABI_DEFAULT_WIDTH  = 250 ;
static const guint32 ABI_DEFAULT_HEIGHT = 250 ;

/**************************************************************************/
/**************************************************************************/

static bool
abi_widget_load_file(AbiWidget * abi, const char * pszFile)
{
	AP_UnixFrame * pFrame = (AP_UnixFrame *) abi->priv->m_pFrame;
	if(pFrame == NULL)
	{
		return false;
	}
	return ( UT_OK == pFrame->loadDocument(pszFile,IEFT_Unknown,true) );
}

//
// arguments to abiwidget
//
static void abi_widget_set_arg (GtkObject  *object,
				GtkArg     *arg,
				guint	arg_id)
{
    AbiWidget * abi = ABI_WIDGET(object);
	switch(arg_id)
	{
	    case CURSOR_ON:
		{
		     if(GTK_VALUE_BOOL (*arg) == TRUE)
			 {
			      abi_widget_turn_on_cursor(abi);
			 }
			 break;
		}
	    case INVOKE_NOARGS:
		{
		     const char * psz= GTK_VALUE_STRING (*arg);
		     abi_widget_invoke_ex(abi,psz,0,0,0);
			 break;
		}
	    case MAP_TO_SCREEN:
		{
		     if(GTK_VALUE_BOOL (*arg) == TRUE)
			 {
			      abi_widget_map_to_screen(abi);
			 }
			 break;
		}
	    case DRAW:
		{
		     if(GTK_VALUE_BOOL (*arg) == TRUE)
			 {
			      abi_widget_draw(abi);
			 }
			 break;
		}
	    case LOAD_FILE:
		{
		     const char * pszFile= GTK_VALUE_STRING (*arg);
			 abi_widget_load_file(abi,pszFile);
			 break;
		}
	    default:
		     break;
	}
}

static void 
abi_widget_size_request (GtkWidget      *widget,
			 GtkRequisition *requisition)
{
	// TODO: possibly be smarter about sizes
  // This code doesn't work but it might be the basis of further work.
#if 0
    if(widget->window)
	{
	    GdkWindow * pWindow = gdk_window_get_parent(widget->window);
		gint width,height;
		gdk_window_get_size (pWindow, &width, &height);
		requisition->width = width;
		requisition->height = height;
	}
	else
#endif
	{
	  requisition->width = ABI_DEFAULT_WIDTH;
	  requisition->height = ABI_DEFAULT_HEIGHT;
	}
	if (ABI_WIDGET(widget)->child)
	  {
		GtkRequisition child_requisition;
      
		gtk_widget_size_request (ABI_WIDGET(widget)->child, &child_requisition);

		requisition->width = child_requisition.width;
		requisition->height = child_requisition.height;
	  }
}

//
// Needed for the gtkbin class
//
static void
abiwidget_add(GtkContainer *container,
		GtkWidget    *widget)
{
  g_return_if_fail (container != NULL);
  g_return_if_fail (widget != NULL);

  if (GTK_CONTAINER_CLASS (parent_class)->add)
  {
	  GTK_CONTAINER_CLASS (parent_class)->add (container, widget);
  }

  ABI_WIDGET(container)->child = GTK_BIN (container)->child;
}

//
// Needed for the gtkbin class
//
static void
abiwidget_remove (GtkContainer *container,
		   GtkWidget    *widget)
{
  g_return_if_fail (container != NULL);
  g_return_if_fail (widget != NULL);

  if (GTK_CONTAINER_CLASS (parent_class)->remove)
    GTK_CONTAINER_CLASS (parent_class)->remove (container, widget);

  ABI_WIDGET(container)->child = GTK_BIN (container)->child;
}

//
// Needed for the gtkbin class
//
static GtkType
abiwidget_child_type  (GtkContainer     *container)
{
  if (!GTK_BIN (container)->child)
    return GTK_TYPE_WIDGET;
  else
    return GTK_TYPE_NONE;
}

static void
abi_widget_init (AbiWidget * abi)
{
	// this isn't really needed, since each widget is
	// guaranteed to be created with g_new0 and we just
	// want everything to be 0/NULL/FALSE anyway right now
	// but i'm keeping it around anyway just in case that changes
  GTK_WIDGET_SET_FLAGS (abi, GTK_CAN_FOCUS | GTK_RECEIVES_DEFAULT |GTK_CAN_DEFAULT );
  GTK_WIDGET_UNSET_FLAGS (abi, GTK_NO_WINDOW);
}

static void
abi_widget_size_allocate (GtkWidget     *widget,
						  GtkAllocation *allocation)
{
	AbiWidget *abi;
	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_ABI_WIDGET (widget));
	g_return_if_fail (allocation != NULL);
	GtkAllocation child_allocation;
	widget->allocation = *allocation;

	gint border_width = GTK_CONTAINER (widget)->border_width;
	gint xthickness = GTK_WIDGET (widget)->style->klass->xthickness;
	gint ythickness = GTK_WIDGET (widget)->style->klass->ythickness;
 	if (GTK_WIDGET_REALIZED (widget))
    {
		// only allocate on realized widgets

		abi = ABI_WIDGET(widget);
		gdk_window_move_resize (widget->window,
					allocation->x+border_width, 
					allocation->y+border_width,
					allocation->width - border_width*2, 
					allocation->height - border_width*2);
		
		_AbiPrivData * priv = abi->priv;
		if (priv->ic)
		{
			gint width, height;

			width = allocation->width;
			height = allocation->height;
			priv->ic_attr->preedit_area.width = width;
			priv->ic_attr->preedit_area.height = height;
			gdk_ic_set_attr (priv->ic, priv->ic_attr,
							 GDK_IC_PREEDIT_AREA);
		}
		
		if (abi->child)
		{
		     child_allocation.x = xthickness;
			 child_allocation.y = ythickness;

			 child_allocation.width = MAX (1, 
										   (gint)widget->allocation.width - child_allocation.x * 2 - border_width * 2);
			 child_allocation.height = MAX (1, 
											(gint)widget->allocation.height - child_allocation.y * 2 - border_width * 2);
			 gtk_widget_size_allocate (ABI_WIDGET (widget)->child, &child_allocation);
		}
    }
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
	attributes.width = ABI_DEFAULT_WIDTH;
	attributes.height = ABI_DEFAULT_HEIGHT;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.event_mask = gtk_widget_get_events (widget) | 
	  GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK |
	  GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
	  GDK_POINTER_MOTION_HINT_MASK | GDK_ENTER_NOTIFY_MASK |
	  GDK_LEAVE_NOTIFY_MASK |
	  GDK_FOCUS_CHANGE_MASK |
	  GDK_STRUCTURE_MASK;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);
	
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
	gdk_window_set_user_data (widget->window, abi);

	widget->style = gtk_style_attach (widget->style, widget->window);
	gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
//
// Setup the input context for pre-edit conditions.
//
	_AbiPrivData * priv = abi->priv;
	priv->ic_attr = gdk_ic_attr_new ();
	if (priv->ic_attr != NULL)
    {
		gint width, height;
		int mask;
		GdkColormap *colormap;
		GdkICAttr *attr = priv->ic_attr;
		int attrmask = GDK_IC_ALL_REQ;
		GdkIMStyle style;

		int supported_style =(GdkIMStyle)(GDK_IM_PREEDIT_NONE |
										  GDK_IM_PREEDIT_NOTHING |
										  GDK_IM_PREEDIT_POSITION |
										  GDK_IM_STATUS_NONE |
										  GDK_IM_STATUS_NOTHING);

		if (widget->style && widget->style->font->type != GDK_FONT_FONTSET)
			supported_style &= ~GDK_IM_PREEDIT_POSITION;

		attr->style = style = gdk_im_decide_style ((GdkIMStyle)supported_style);
		attr->client_window = widget->window;

		if ((colormap = gtk_widget_get_colormap (widget)) !=
			gtk_widget_get_default_colormap ())
		{
			attrmask |= GDK_IC_PREEDIT_COLORMAP;
			attr->preedit_colormap = colormap;
		}
		attrmask |= GDK_IC_PREEDIT_FOREGROUND;
		attrmask |= GDK_IC_PREEDIT_BACKGROUND;
		attr->preedit_foreground = widget->style->fg[GTK_STATE_NORMAL];
		attr->preedit_background = widget->style->base[GTK_STATE_NORMAL];
		attr->preedit_area.width = ABI_DEFAULT_WIDTH;
		attr->preedit_area.height = ABI_DEFAULT_HEIGHT;
		attr->preedit_fontset = widget->style->font;

		switch (style & GDK_IM_PREEDIT_MASK)
		{
		case GDK_IM_PREEDIT_POSITION:
			if (widget->style && widget->style->font->type != GDK_FONT_FONTSET)
			{
				g_warning ("over-the-spot style requires fontset");
				break;
			}

			gdk_window_get_size (widget->window, &width, &height);
		  
			attrmask |= GDK_IC_PREEDIT_POSITION_REQ;
			attr->spot_location.x = 0;
			attr->spot_location.y = height;
			attr->preedit_area.x = 0;
			attr->preedit_area.y = 0;
			attr->preedit_area.width = width;
			attr->preedit_area.height = height;
			attr->preedit_fontset = widget->style->font;
			break;
		}
		priv->ic = gdk_ic_new (attr, (GdkICAttributesType)attrmask);
		
		if (priv->ic == NULL)
			g_warning ("Can't create input context.");
		else
		{
			mask = gdk_window_get_events (widget->window);
			mask |= (GdkEventMask)gdk_ic_get_events (priv->ic);
			gdk_window_set_events (widget->window,(GdkEventMask) mask);
			
			//	if (GTK_WIDGET_HAS_FOCUS(widget))
				gdk_im_begin (priv->ic, widget->window);
				gdk_ic_set_attr (priv->ic, priv->ic_attr,
								 GDK_IC_PREEDIT_AREA);
		}
	}
	gtk_object_set_data(GTK_OBJECT(widget), "ic_attr", priv->ic_attr);
	gtk_object_set_data(GTK_OBJECT(widget), "ic", priv->ic);
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

	if(abi->priv->m_pApp)
	{
		abi->priv->m_pApp->forgetFrame(abi->priv->m_pFrame);
		delete abi->priv->m_pFrame;
		if(!abi->priv->externalApp)
		{
			abi->priv->m_pApp->shutdown();
			delete abi->priv->m_pApp;
		}
	}
	g_free (abi->priv->m_szFilename);
	if(abi->priv->ic)
	{
		gdk_ic_attr_destroy(abi->priv->ic_attr);
		gdk_ic_destroy(abi->priv->ic);
	}
	g_free (abi->priv);

	// chain up
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
abi_widget_class_init (AbiWidgetClass *abi_class)
{
	GtkObjectClass * object_class;
	GtkWidgetClass * widget_class;
	GtkContainerClass *container_class;
	container_class = (GtkContainerClass*) abi_class;

	object_class = (GtkObjectClass *)abi_class;
	widget_class = (GtkWidgetClass *)abi_class;

	// we need our own special destroy function
	object_class->destroy  = abi_widget_destroy;

	// set our parent class
	parent_class = (GtkBinClass *)
		gtk_type_class (gtk_bin_get_type());
	
	// set our custom destroy method
	object_class->destroy = abi_widget_destroy; 
	object_class->set_arg = abi_widget_set_arg;

	// set our custom class methods
	widget_class->realize       = abi_widget_realize;
	widget_class->size_request  = abi_widget_size_request;
   	widget_class->size_allocate = abi_widget_size_allocate; 

	// For the container methods
	container_class->add = abiwidget_add;
	container_class->remove = abiwidget_remove;
	container_class->child_type = abiwidget_child_type;


	// AbiWidget's master "invoke" method
	abi_class->invoke    = abi_widget_invoke;
	abi_class->invoke_ex = abi_widget_invoke_ex;

	// assign handles to Abi's edit methods
	abi_class->align_center  = EM_NAME(alignCenter);
	abi_class->align_justify = EM_NAME(alignLeft);
	abi_class->align_left    = EM_NAME(alignRight);
	abi_class->align_right   = EM_NAME(alignJustify);
	
	abi_class->copy          = EM_NAME(copy);
	abi_class->cut           = EM_NAME(cut);
	abi_class->paste         = EM_NAME(paste);
	abi_class->paste_special = EM_NAME(pasteSpecial);
	abi_class->select_all    = EM_NAME(selectAll);
	abi_class->select_block  = EM_NAME(selectBlock);
	abi_class->select_line   = EM_NAME(selectLine);	
	abi_class->select_word   = EM_NAME(selectWord);	
	
	abi_class->undo = EM_NAME(undo);
	abi_class->redo = EM_NAME(redo);
	
	abi_class->insert_data  = EM_NAME(insertData);
	abi_class->insert_space = EM_NAME(insertSpace);		
        
	abi_class->delete_bob   = EM_NAME(delBOB);
	abi_class->delete_bod   = EM_NAME(delBOD);
	abi_class->delete_bol   = EM_NAME(delBOL);
	abi_class->delete_bow   = EM_NAME(delBOW);
	abi_class->delete_eob   = EM_NAME(delEOB);
	abi_class->delete_eod   = EM_NAME(delEOD);
	abi_class->delete_eol   = EM_NAME(delEOL);
	abi_class->delete_eow   = EM_NAME(delEOW);
	abi_class->delete_left  = EM_NAME(delLeft);
	abi_class->delete_right = EM_NAME(delRight);
	
	abi_class->edit_header   = EM_NAME(editHeader);
	abi_class->edit_footer   = EM_NAME(editFooter);
	abi_class->remove_header = EM_NAME(removeHeader);
	abi_class->remove_footer = EM_NAME(removeFooter);
	
	abi_class->select_bob         = EM_NAME(extSelBOB);
	abi_class->select_bod         = EM_NAME(extSelBOD);
	abi_class->select_bol         = EM_NAME(extSelBOL);
	abi_class->select_bow         = EM_NAME(extSelBOW);
	abi_class->select_eob         = EM_NAME(extSelEOB);
	abi_class->select_eod         = EM_NAME(extSelEOD);
	abi_class->select_eol         = EM_NAME(extSelEOL);
	abi_class->select_eow         = EM_NAME(extSelEOW);    
	abi_class->select_left        = EM_NAME(extSelLeft);
	abi_class->select_next_line   = EM_NAME(extSelNextLine);
	abi_class->select_page_down   = EM_NAME(extSelPageDown);
	abi_class->select_page_up     = EM_NAME(extSelPageUp);
	abi_class->select_prev_line   = EM_NAME(extSelPrevLine);
	abi_class->select_right       = EM_NAME(extSelRight);
	abi_class->select_screen_down = EM_NAME(extSelScreenDown);
	abi_class->select_screen_up   = EM_NAME(extSelScreenUp);
	abi_class->select_to_xy       = EM_NAME(extSelToXY);
	
	abi_class->toggle_bold        = EM_NAME(toggleBold);
	abi_class->toggle_bottomline  = EM_NAME(toggleBottomline);
	abi_class->toggle_insert_mode = EM_NAME(toggleInsertMode);
	abi_class->toggle_italic      = EM_NAME(toggleItalic);
	abi_class->toggle_overline    = EM_NAME(toggleOline);
	abi_class->toggle_plain       = EM_NAME(togglePlain);
	abi_class->toggle_strike      = EM_NAME(toggleStrike);
	abi_class->toggle_sub         = EM_NAME(toggleSub);
	abi_class->toggle_super       = EM_NAME(toggleSuper);
	abi_class->toggle_topline     = EM_NAME(toggleTopline);
	abi_class->toggle_underline   = EM_NAME(toggleUline);
	abi_class->toggle_unindent    = EM_NAME(toggleUnIndent);
	
	abi_class->view_formatting_marks = EM_NAME(viewPara);
	abi_class->view_print_layout     = EM_NAME(viewPrintLayout);
	abi_class->view_normal_layout    = EM_NAME(viewNormalLayout);
	abi_class->view_online_layout    = EM_NAME(viewWebLayout);
	
	abi_class->moveto_bob         = EM_NAME(warpInsPtBOB);
	abi_class->moveto_bod         = EM_NAME(warpInsPtBOD);
	abi_class->moveto_bol         = EM_NAME(warpInsPtBOL);
	abi_class->moveto_bop         = EM_NAME(warpInsPtBOP);
	abi_class->moveto_bow         = EM_NAME(warpInsPtBOW);
	abi_class->moveto_eob         = EM_NAME(warpInsPtEOB);
	abi_class->moveto_eod         = EM_NAME(warpInsPtEOD);
	abi_class->moveto_eol         = EM_NAME(warpInsPtEOL);
	abi_class->moveto_eop         = EM_NAME(warpInsPtEOP);
	abi_class->moveto_eow         = EM_NAME(warpInsPtEOW);
	abi_class->moveto_left        = EM_NAME(warpInsPtLeft);
	abi_class->moveto_next_line   = EM_NAME(warpInsPtNextLine);
	abi_class->moveto_next_page   = EM_NAME(warpInsPtNextPage);
	abi_class->moveto_next_screen = EM_NAME(warpInsPtNextScreen);
	abi_class->moveto_prev_line   = EM_NAME(warpInsPtPrevLine);
	abi_class->moveto_prev_page   = EM_NAME(warpInsPtPrevPage);
	abi_class->moveto_prev_screen = EM_NAME(warpInsPtPrevScreen);
	abi_class->moveto_right       = EM_NAME(warpInsPtRight);
	abi_class->moveto_to_xy       = EM_NAME(warpInsPtToXY);
	
	abi_class->zoom_100   = EM_NAME(zoom100);
	abi_class->zoom_200   = EM_NAME(zoom200);
	abi_class->zoom_50    = EM_NAME(zoom50);
	abi_class->zoom_75    = EM_NAME(zoom75);
	abi_class->zoom_whole = EM_NAME(zoomWhole);
	abi_class->zoom_width = EM_NAME(zoomWidth);
	
    // now add GtkArgs for our properties
	gtk_object_add_arg_type ("AbiWidget::cursoron", GTK_TYPE_BOOL, GTK_ARG_READWRITE, CURSOR_ON);
	gtk_object_add_arg_type ("AbiWidget::invoke_noargs", GTK_TYPE_STRING, GTK_ARG_READWRITE, INVOKE_NOARGS);
	gtk_object_add_arg_type ("AbiWidget::map_to_screen", GTK_TYPE_BOOL, GTK_ARG_READWRITE, MAP_TO_SCREEN);
	gtk_object_add_arg_type ("AbiWidget::draw", GTK_TYPE_BOOL, GTK_ARG_READWRITE, DRAW);
	gtk_object_add_arg_type ("AbiWidget::load_file", GTK_TYPE_STRING, GTK_ARG_READWRITE, LOAD_FILE);

}

static void
abi_widget_construct (AbiWidget * abi, const char * file, AP_UnixApp * pApp)
{
	AbiPrivData * priv = g_new0 (AbiPrivData, 1);
	priv->m_pFrame = NULL;
	priv->m_szFilename = NULL;
	priv->ic_attr = NULL;
	priv->ic = NULL;
	if(pApp == NULL)
	{
		priv->m_pApp = NULL;
		priv->externalApp = false;
	}
	else
	{
		priv->m_pApp = pApp;
		priv->externalApp = true;
	}
	// this is all that we can do here, because we can't draw until we're
	// realized and have a GdkWindow pointer

	if (file)
		priv->m_szFilename = g_strdup (file);

	abi->priv = priv;
}

/**************************************************************************/
/**************************************************************************/

extern "C" void 
abi_widget_map_to_screen(AbiWidget * abi)
{
	GtkWidget * widget = GTK_WIDGET(abi);
	// now we can set up Abi inside of this GdkWindow

	XAP_Args *pArgs = 0;
	if(abi->priv->externalApp)
	{
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
		abi->priv->m_pApp     = pApp;
	}


	AP_UnixFrame * pFrame  = new AP_UnixFrame(abi->priv->m_pApp);
	UT_ASSERT(pFrame);
	static_cast<XAP_UnixFrame *>(pFrame)->setTopLevelWindow(widget);
	pFrame->initialize(XAP_NoMenusWindowLess);
	abi->priv->m_pFrame   = pFrame;
	if(!abi->priv->externalApp)
	{
		delete pArgs;
	}
	abi->priv->m_pFrame->loadDocument(abi->priv->m_szFilename,IEFT_Unknown ,true);

}

extern "C" void 
abi_widget_turn_on_cursor(AbiWidget * abi)
{
	
    FV_View * pV = (FV_View*) abi->priv->m_pFrame->getCurrentView();
	pV->focusChange(AV_FOCUS_HERE);
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
		
		abi_type = gtk_type_unique (gtk_bin_get_type (), &info);
	}
	
	return abi_type;
}

/**
 * abi_widget_new
 *
 * Creates a new AbiWord widget using an internal Abiword App
 */
extern "C" GtkWidget *
abi_widget_new (void)
{
	AbiWidget * abi;

	abi = (AbiWidget *)gtk_type_new (abi_widget_get_type ());
	abi_widget_construct (abi, 0, NULL);

	return GTK_WIDGET (abi);
}

/**
 * abi_widget_new_with_file
 *
 * Creates a new AbiWord widget and tries to load the file
 * This uses an internal Abiword App
 *
 * \param file - A file on your HD
 */
extern "C" GtkWidget *
abi_widget_new_with_file (const gchar * file)
{
	AbiWidget * abi;

	g_return_val_if_fail (file != 0, 0);

	abi = (AbiWidget *)gtk_type_new (abi_widget_get_type ());
	abi_widget_construct (abi, file,NULL);

	return GTK_WIDGET (abi);
}

/**
 * abi_widget_new_with_app
 *
 * Creates a new AbiWord widget using an external Abiword App
 */
extern "C" GtkWidget *
abi_widget_new_with_app (AP_UnixApp * pApp)
{
	AbiWidget * abi;

	abi = (AbiWidget *)gtk_type_new (abi_widget_get_type ());
	abi_widget_construct (abi, 0, pApp);

	return GTK_WIDGET (abi);
}

/**
 * abi_widget_new_with_app_file
 *
 * Creates a new AbiWord widget and tries to load the file
 * This uses an external Abiword App
 *
 * \param file - A file on your HD
 * \param pApp - pointer to a valid AbiWord XAP_UnixApp
 */
extern "C" GtkWidget *
abi_widget_new_with_app_file (AP_UnixApp * pApp, const gchar * file)
{
	AbiWidget * abi;

	g_return_val_if_fail (file != 0, 0);

	abi = (AbiWidget *)gtk_type_new (abi_widget_get_type ());
	abi_widget_construct (abi, file,pApp);

	return GTK_WIDGET (abi);
}

/**
 * abi_widget_invoke()
 *
 * Invoke any of abiword's edit methods by name
 *
 * \param w - An AbiWord widget
 * \param mthdName - A null-terminated string representing the method's name
 *
 * \return FALSE if any preconditions fail
 * \return TRUE|FALSE depending on the result of the EditMethod's completion
 *
 * example usage:
 *
 * gboolean b = FALSE; 
 * GtkWidget * w = abi_widget_new ();
 *
 * b = abi_widget_invoke (ABI_WIDGET(w), "alignCenter");
 *
 */
extern "C" gboolean
abi_widget_invoke (AbiWidget * w, const char * mthdName)
{
  return abi_widget_invoke_ex ( w, mthdName, NULL, 0, 0 ) ;
}

/**
 * abi_widget_invoke_ex()
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
 * b = abi_widget_invoke_ex (ABI_WIDGET(w), "insertData", "Hello World!", 0, 0);
 * b = abi_widget_invoke_ex (ABI_WIDGET(w), "alignCenter", 0, 0, 0);
 *
 */
extern "C" gboolean
abi_widget_invoke_ex (AbiWidget * w, const char * mthdName, 
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

extern "C" void
abi_widget_draw (AbiWidget * w)
{
	// obtain a valid view
	FV_View * view = (FV_View *) w->priv->m_pFrame->getCurrentView();
	view->draw();
}


















