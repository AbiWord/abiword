/* AbiSource Program Utilities
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include"xav_View.h"
#include"xap_Frame.h"
#include"xap_App.h"


// default GTK message box button width, in GTK screen units (pixels)
#define DEFAULT_BUTTON_WIDTH	85

/*****************************************************************/

static gboolean focus_in_event(GtkWidget *widget,GdkEvent */*event*/,gpointer /*user_data*/)
{
      XAP_Frame *pFrame=(XAP_Frame *)gtk_object_get_data(GTK_OBJECT(widget), "frame");
      UT_ASSERT(pFrame);
      pFrame->getCurrentView()->focusChange(AV_FOCUS_NEARBY);
      return FALSE;
}

static gboolean destroy_event(GtkWidget *widget,GdkEvent */*event*/,gpointer /*user_data*/)
{
      XAP_Frame *pFrame=(XAP_Frame *)gtk_object_get_data(GTK_OBJECT(widget), "frame");
      if(pFrame == NULL) return FALSE;
      
      return FALSE;
}

static gboolean focus_out_event(GtkWidget *widget,GdkEvent */*event*/,gpointer /*user_data*/)
{
      XAP_Frame *pFrame=(XAP_Frame *)gtk_object_get_data(GTK_OBJECT(widget), "frame");
      if(pFrame == NULL) return FALSE;
      AV_View * pView = pFrame->getCurrentView();
      if(pView!= NULL)
      {
	     pView->focusChange(AV_FOCUS_NONE);
      }
      return FALSE;
}

static gboolean focus_out_event_Modeless(GtkWidget *widget,GdkEvent */*event*/,gpointer /*user_data*/)
{
      XAP_App *pApp = (XAP_App *)gtk_object_get_data(GTK_OBJECT(widget), "pApp");
      XAP_Frame *pFrame = pApp->getLastFocussedFrame();
      if(pFrame ==(XAP_Frame *)  NULL) 
      {
             UT_uint32 nframes =  pApp->getFrameCount();
             if(nframes > 0 && nframes < 10)
	     {     
	            pFrame = pApp->getFrame(0);
	     }
             else
	     {
	            return FALSE;
	     }
      }
      if(pFrame == (XAP_Frame *) NULL) return FALSE;
      AV_View * pView = pFrame->getCurrentView();
      UT_ASSERT(pView);
      if(pView!= NULL)
      {
	     pView->focusChange(AV_FOCUS_NONE);
      }
      return FALSE;
}


static gboolean focus_in_event_Modeless(GtkWidget *widget,GdkEvent */*event*/,gpointer /*user_data*/)
{
      XAP_App *pApp=(XAP_App *)gtk_object_get_data(GTK_OBJECT(widget), "pApp");
      XAP_Frame *pFrame= pApp->getLastFocussedFrame();
      if(pFrame ==(XAP_Frame *)  NULL) 
      {
             UT_uint32 nframes =  pApp->getFrameCount();
             if(nframes > 0 && nframes < 10)
	     {     
	            pFrame = pApp->getFrame(0);
	     }
             else
	     {
	            return FALSE;
	      }
      }
      if(pFrame == (XAP_Frame *) NULL) return FALSE;
      AV_View * pView = pFrame->getCurrentView();
      if(pView!= NULL)
      {
            pView->focusChange(AV_FOCUS_MODELESS);
      }
      return FALSE;
}


static gboolean focus_in_event_ModelessOther(GtkWidget *widget,GdkEvent */*event*/,gboolean (*other_function)(void) )
{
      XAP_App *pApp=(XAP_App *)gtk_object_get_data(GTK_OBJECT(widget), "pApp");
      XAP_Frame *pFrame= pApp->getLastFocussedFrame();
      if(pFrame ==(XAP_Frame *)  NULL) 
      {
             UT_uint32 nframes =  pApp->getFrameCount();
             if(nframes > 0 && nframes < 10)
	     {     
	            pFrame = pApp->getFrame(0);
	     }
             else
	     {
	            return FALSE;
	      }
      }
      if(pFrame == (XAP_Frame *) NULL) return FALSE;
      AV_View * pView = pFrame->getCurrentView();
      if(pView!= NULL)
      {
            pView->focusChange(AV_FOCUS_MODELESS);
            (*other_function)();
      }
      return FALSE;
}

void connectFocus(GtkWidget *widget,const XAP_Frame *frame)
{
      gtk_object_set_data(GTK_OBJECT(widget), "frame",
					  (void *)frame);
      gtk_signal_connect(GTK_OBJECT(widget), "focus_in_event",
					 GTK_SIGNAL_FUNC(focus_in_event), NULL);
      gtk_signal_connect(GTK_OBJECT(widget), "focus_out_event",
					 GTK_SIGNAL_FUNC(focus_out_event), NULL);
      gtk_signal_connect(GTK_OBJECT(widget), "destroy",
					 GTK_SIGNAL_FUNC(destroy_event), NULL);
}

void connectFocusModelessOther(GtkWidget *widget,const XAP_App * pApp, 
			       gboolean(*other_function)(void))
{
      gtk_object_set_data(GTK_OBJECT(widget), "pApp",
					  (void *)pApp);
      gtk_signal_connect(GTK_OBJECT(widget), "focus_in_event",
					 GTK_SIGNAL_FUNC(focus_in_event_ModelessOther), (gpointer) other_function);
      gtk_signal_connect(GTK_OBJECT(widget), "focus_out_event",
					 GTK_SIGNAL_FUNC(focus_out_event_Modeless), NULL);
      gtk_signal_connect(GTK_OBJECT(widget), "destroy",
					 GTK_SIGNAL_FUNC(focus_out_event_Modeless), NULL);
}


void connectFocusModeless(GtkWidget *widget,const XAP_App * pApp)
{
      gtk_object_set_data(GTK_OBJECT(widget), "pApp",
					  (void *)pApp);
      gtk_signal_connect(GTK_OBJECT(widget), "focus_in_event",
					 GTK_SIGNAL_FUNC(focus_in_event_Modeless), NULL);
      gtk_signal_connect(GTK_OBJECT(widget), "focus_out_event",
					 GTK_SIGNAL_FUNC(focus_out_event_Modeless), NULL);
      gtk_signal_connect(GTK_OBJECT(widget), "destroy",
					 GTK_SIGNAL_FUNC(destroy_event), NULL);
}


UT_Bool isTransientWindow(GtkWindow *window,GtkWindow *parent)
{
  if(window)
	{
	  while(window->transient_parent)
		{
		  window=window->transient_parent;
		  if(window==parent)
			return UT_TRUE;
		}
	}
  return UT_FALSE;
}

gint s_key_pressed(GtkWidget * /* widget */, GdkEventKey * e)
{
	UT_ASSERT(e);

	guint key = e->keyval;

	// TODO this is hard coded, maybe fix it
	if (key == 'k' ||
		key == GDK_Escape)
	{
		gtk_main_quit();
	}

	return TRUE;
}

/*
  This is a small message box for startup warnings and/or
  errors.  Please do NOT use this for normal system execution
  user messages; use the XAP_UnixDialog_MessageBox class for that.
  We can't use that here because there is no parent frame, etc.
*/

void messageBoxOK(const char * message)
{
	// New GTK+ dialog window
	GtkWidget * dialog_window = gtk_dialog_new();								 

	gtk_signal_connect_after (GTK_OBJECT (dialog_window),
							  "destroy",
							  NULL,
							  NULL);
	gtk_signal_connect_after (GTK_OBJECT (dialog_window),
							  "delete_event",
							  GTK_SIGNAL_FUNC(gtk_main_quit),
							  NULL);

	gtk_window_set_title(GTK_WINDOW(dialog_window), "AbiWord");

	// don't let user shrink or expand, but auto-size to
	// contents initially
    gtk_window_set_policy(GTK_WINDOW(dialog_window),
						  FALSE,
						  FALSE,
						  TRUE);

	// Intercept key strokes
	gtk_signal_connect(GTK_OBJECT(dialog_window),
					   "key_press_event",
					   GTK_SIGNAL_FUNC(s_key_pressed),
					   NULL);

	// Add our label string to the dialog in the message area
	GtkWidget * label = gtk_label_new(message);
	gtk_misc_set_padding(GTK_MISC(label), 10, 10);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG (dialog_window)->vbox),
					   label, TRUE, TRUE, 0);
	gtk_widget_show(label);

	GtkWidget *		ok_label;
	GtkWidget * 	ok_button;
	guint			ok_accel;
	
	// create the OK
	ok_label = gtk_label_new("SHOULD NOT APPEAR");
	ok_accel = gtk_label_parse_uline(GTK_LABEL(ok_label), "O_K");
	gtk_widget_show(ok_label);
	ok_button = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(ok_button), ok_label);
	gtk_signal_connect (GTK_OBJECT (ok_button),
						"clicked",
						GTK_SIGNAL_FUNC(gtk_main_quit),
						NULL);
	GTK_WIDGET_SET_FLAGS (ok_button, GTK_CAN_DEFAULT);
	gtk_widget_set_usize(ok_button, DEFAULT_BUTTON_WIDTH, 0);

	// pack the OK
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog_window)->action_area),
						ok_button, FALSE, FALSE, 0);
	gtk_widget_grab_default (ok_button);
	gtk_widget_show (ok_button);

	// set the size of the dialog to size with the label inside
	gtk_widget_size_request(dialog_window, &dialog_window->requisition);
	gtk_widget_set_usize(dialog_window, dialog_window->requisition.width + 40, 0);
	
	// center it (in what?)
//	GdkWindowPrivate * rootWindow = ::getRootWindow(dialog_window);
//  centerDialog((GdkWindow *) rootWindow, dialog_window);
//	gtk_window_set_transient_for(GTK_WINDOW(dialog_window), GTK_WINDOW(parent));

	gtk_grab_add(GTK_WIDGET(dialog_window));
	gtk_widget_show(dialog_window);
	
	gtk_main();

	// clean up
	gtk_widget_destroy(GTK_WIDGET(dialog_window));
}

GdkWindowPrivate * getRootWindow(GtkWidget * widget)
{
	UT_ASSERT(widget);

	GdkWindowPrivate * priv = (GdkWindowPrivate *) widget->window;
	while (priv->parent && ((GdkWindowPrivate*) priv->parent)->parent)
		priv = (GdkWindowPrivate*) priv->parent;

	return (GdkWindowPrivate *) priv;
}

void centerDialog(GtkWidget * parent, GtkWidget * child)
{
	UT_ASSERT(parent);
	UT_ASSERT(child);

	UT_ASSERT(parent->window);
	gint parentx = 0;
	gint parenty = 0;
	gint parentwidth = 0;
	gint parentheight = 0;
	gdk_window_get_origin(parent->window, &parentx, &parenty);
	gdk_window_get_size(parent->window, &parentwidth, &parentheight);
	UT_ASSERT(parentwidth > 0 && parentheight > 0);

	// this message box's geometry (it won't have a ->window yet, so don't assert it)
	gint width = 0;
	gint height = 0;
	gtk_widget_size_request(child, &child->requisition);
	width = child->requisition.width;
	height = child->requisition.height;
	UT_ASSERT(width > 0 && height > 0);

	// set new place
	gint newx = parentx + ((parentwidth - width) / 2);
	gint newy = parenty + ((parentheight - height) / 2);

	// measure the root window
	gint rootwidth = gdk_screen_width();
	gint rootheight = gdk_screen_height();
	// if the dialog won't fit on the screen, panic and center on the root window
	if ((newx + width) > rootwidth || (newy + height) > rootheight)
	{
		UT_DEBUGMSG(("Dialog [%p] at coordiantes [%d, %d] is in non-viewable area; "
					 "centering on desktop instead.\n", child, newx, newy));
		gtk_window_set_position(GTK_WINDOW(child), GTK_WIN_POS_CENTER);
	}
	else
		gtk_widget_set_uposition(child, newx, newy);
	
}

gint searchCList(GtkCList * clist, char * compareText)
{
	UT_ASSERT(clist);

	// if text is null, it's not found
	if (!compareText)
		return -1;
	
	gchar * text[2] = {NULL, NULL};
	
	for (gint i = 0; i < clist->rows; i++)
	{
		gtk_clist_get_text(clist, i, 0, text);
		if (text && text[0])
			if (!UT_stricmp(text[0], compareText))
				return i;
	}

	return -1;
}

/****************************************************************************
 Written    by:     Stephen Hack                <shack@uiuc.edu>
                  date:     Thu Oct 14 1999
    
 Purpose
    To recursivily change the labels in a window (any container) from the
    windows & to GTK+'s underline format

    Can support multiple &'s, to get an & into the document, enter \&

 Usage
    fix_label( GtkWidget *window )

 Compile time Options
    FIXLBL_DBG      if defined, will print out a nested listing of all labels

 ***************************************************************************/

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

static const gchar KEY_ACCEL_GROUP[] = "ACCEL_GROUP";

#define DBG(a) {a}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    Data types
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
struct fix_label_data {
    int depth;
    GtkWidget     *topwindow;
    GtkWidget     *accel_ctrl;
    gchar         *accel_sig;
    GtkAccelGroup *accel_group;
}; 

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    Prototypes
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
static void on_notebook_switch_page( GtkNotebook     *notebook,
                                     GtkNotebookPage *page,
                                     gint             page_num,
                                     gpointer         user_data );

void process_notebook_page( GtkWidget *notebook, 
                            GtkWidget *page,
                            struct fix_label_data *data );

void fix_label_callback( GtkWidget *widget, 
                         gpointer _data );

void fix_label( GtkWidget *widget );


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	Call back routine for switching accelerator groups
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
static void
on_notebook_switch_page				   (GtkNotebook		*notebook,
										GtkNotebookPage *page,
										gint			 page_num,
										gpointer		 user_data)
{
	GtkAccelGroup *oldaccel, *newaccel;
	GtkWindow	  *topwindow = (GtkWindow *)user_data;

	UT_ASSERT(topwindow && GTK_IS_WINDOW(topwindow));
	UT_ASSERT(notebook && GTK_IS_NOTEBOOK(notebook));
	UT_ASSERT(page && page->child && GTK_IS_OBJECT(page->child));

	if ( GTK_OBJECT_DESTROYED(notebook) )
		return ;

	oldaccel = (GtkAccelGroup *)gtk_object_get_data( GTK_OBJECT(notebook),
													 KEY_ACCEL_GROUP );
	newaccel = (GtkAccelGroup *)gtk_object_get_data( GTK_OBJECT(page->child),  
													 KEY_ACCEL_GROUP );
	
	if ( oldaccel )
		gtk_window_remove_accel_group( topwindow, oldaccel );

	gtk_window_add_accel_group( topwindow, newaccel );
	gtk_object_set_data(GTK_OBJECT(notebook), KEY_ACCEL_GROUP, newaccel );
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * To process a notebook page
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void process_notebook_page( GtkWidget *notebook, 
							GtkWidget *page,
							struct fix_label_data *data )
{
	GtkAccelGroup *newgroup;
	struct fix_label_data newdata;

	UT_ASSERT(notebook && page && data && data->topwindow);
	UT_ASSERT( GTK_IS_NOTEBOOK(notebook) );
	UT_ASSERT( GTK_IS_WIDGET(page) );

	memcpy( &newdata, data, sizeof(struct fix_label_data));

	newgroup = gtk_accel_group_new();
	UT_ASSERT(newgroup);

	gtk_object_set_data( GTK_OBJECT(page), KEY_ACCEL_GROUP, (gpointer)newgroup );

	newdata.accel_group = newgroup;
	fix_label_callback( page, &newdata );
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * The recursive call back function for gtk_container_for_all
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifdef TRACE
#  undef TRACE
#endif
#ifdef DBG_FIX_LABEL
# define TRACE(a) { for ( i = 0; i < data->depth; i++ ) printf("  "); \
					printf a; printf("\n"); }
#else
# define TRACE(a)
#endif
void fix_label_callback( GtkWidget *widget, gpointer _data )
{
	struct fix_label_data *data = (struct fix_label_data *)_data;
	struct fix_label_data newdata;
	int i, pageindex;
	gchar *str;
	gchar *newlbl;
	gchar accelch;
	gpointer accel_group;
	GtkWidget *w, *accel_tie;
 
	char *dbg_str;
 
	UT_ASSERT(widget && _data);

	memcpy( &newdata, _data, sizeof(struct fix_label_data) );
	newdata.depth = data->depth + 1; 

	newdata.accel_ctrl = widget;
	dbg_str = "garbage";
	if ( GTK_IS_BUTTON(widget) ) {
		newdata.accel_sig = "clicked";
		DBG(dbg_str = "button";)
	}
	else {
		// we can't find what type to set accels to, then use parent
		newdata.accel_ctrl = data->accel_ctrl;
		newdata.accel_sig = data->accel_sig;
		DBG(dbg_str = "container";)
	}

	// if it's a containter, go deeper
	
	if ( GTK_IS_NOTEBOOK(widget) ) {
		TRACE(("NOTEBOOK"));

		for ( pageindex = 0; 
			  (w = gtk_notebook_get_nth_page( GTK_NOTEBOOK(widget), pageindex)) != 0; 
			  pageindex++ ) 
		{
			TRACE(("Page %i", pageindex));
		  
			UT_ASSERT( data->topwindow );
 
			process_notebook_page( widget, w, &newdata );

			if ( pageindex == 0 )		
			{
						accel_group = gtk_object_get_data( GTK_OBJECT(w), KEY_ACCEL_GROUP);
						gtk_object_set_data( GTK_OBJECT(widget), KEY_ACCEL_GROUP, accel_group );
						gtk_window_add_accel_group( GTK_WINDOW(data->topwindow), 
																				(GtkAccelGroup *)accel_group);
			}
		}
			
		/* set the signal handler to change to accel groups */
		gtk_signal_connect (GTK_OBJECT(widget), "switch_page",
						  GTK_SIGNAL_FUNC (on_notebook_switch_page),
						  data->topwindow );


	}	

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	
	 * handler for an option menu 
	 */
	else if ( GTK_IS_OPTION_MENU( widget ) ) {
		TRACE(("found option menu " ));

		accel_tie = (GtkWidget *)gtk_object_get_data( GTK_OBJECT(widget),
													  "accel-tie");

		if ( accel_tie ) {
			UT_ASSERT( GTK_IS_LABEL(accel_tie) );		

		}
	}

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	
	 * default handler for a 'container' 
	 */
	else if ( GTK_IS_CONTAINER( widget ) ) {

		/* go deeper */
		TRACE(("found %s - ", dbg_str ));
		gtk_container_forall( GTK_CONTAINER(widget), 
							  fix_label_callback, (gpointer *)&newdata );

	}

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	
	 * handle a label
	 */
	else if ( GTK_IS_LABEL( widget ) ) {

		gtk_label_get( GTK_LABEL(widget), &str );
		UT_ASSERT(str);

		TRACE(("found label [%s] ", str ));

		/* convert the &'s into _'s .  Also handle \& as &'s */
		accelch = 0;
		newlbl = UT_strdup(str);				UT_ASSERT(newlbl);
		for ( i = 0; newlbl[i] != 0; i++ ) 
		{
			if ( newlbl[i] == '&' ) {
				if ( i > 0 && newlbl[i-1] == '\\' )
				{
					newlbl[i-1] = '&';
					strcpy( &newlbl[i], &newlbl[i+1]);
					i--;
				}
				else
					newlbl[i] = '_';
			}
		}

		/* underline the words */
		accelch = 0;
		accelch = gtk_label_parse_uline(GTK_LABEL(widget), newlbl);

		/* added an accelerator if need be */
		if ( accelch != -1 && data->accel_ctrl && data->accel_group ) {
			  gtk_widget_add_accelerator (data->accel_ctrl, data->accel_sig, data->accel_group,
										  accelch, GDK_CONTROL_MASK,
										  GTK_ACCEL_VISIBLE);
		}

		if ( newlbl ) free(newlbl);		
	}
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * the actual exported call
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void createLabelAccelerators( GtkWidget *widget ) 
{
	struct fix_label_data data = { 0, 0, 0, 0, 0 };

	data.depth = 0;
	data.accel_group = gtk_accel_group_new();
	data.topwindow = widget;

	fix_label_callback( widget, &data );

	gtk_window_add_accel_group (GTK_WINDOW (widget), data.accel_group);
}
