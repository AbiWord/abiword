/* AbiSource Program Utilities
 * Copyright (C) 1998-2002 AbiSource, Inc.
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
#include <stdarg.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "xav_View.h"
#include "xap_UnixFrame.h"
#include "xap_App.h"
#include "xap_UnixDialogHelper.h"
#include "xap_Dialog.h"

#ifdef HAVE_GNOME
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#endif


// default GTK message box button width, in GTK screen units (pixels)
#define DEFAULT_BUTTON_WIDTH	85

/*****************************************************************/

static gboolean focus_in_event(GtkWidget *widget,GdkEvent */*event*/,gpointer /*user_data*/)
{
      XAP_Frame *pFrame=(XAP_Frame *)g_object_get_data(G_OBJECT(widget), "frame");
      UT_ASSERT(pFrame);
	  if (pFrame->getCurrentView())
		  pFrame->getCurrentView()->focusChange(AV_FOCUS_NEARBY);
      return FALSE;
}

static gboolean destroy_event(GtkWidget *widget,GdkEvent */*event*/,gpointer /*user_data*/)
{
      XAP_Frame *pFrame=(XAP_Frame *)g_object_get_data(G_OBJECT(widget), "frame");
      if(pFrame == NULL) return FALSE;
      
      return FALSE;
}

static gboolean focus_out_event(GtkWidget *widget,GdkEvent */*event*/,gpointer /*user_data*/)
{
      XAP_Frame *pFrame=(XAP_Frame *)g_object_get_data(G_OBJECT(widget), "frame");
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
      XAP_App *pApp = (XAP_App *)g_object_get_data(G_OBJECT(widget), "pApp");
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
      XAP_App *pApp=(XAP_App *)g_object_get_data(G_OBJECT(widget), "pApp");
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
      XAP_App *pApp=(XAP_App *)g_object_get_data(G_OBJECT(widget), "pApp");
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
      g_object_set_data(G_OBJECT(widget), "frame",
					  (void *)frame);
      g_signal_connect(G_OBJECT(widget), "focus_in_event",
					 G_CALLBACK(focus_in_event), NULL);
      g_signal_connect(G_OBJECT(widget), "focus_out_event",
					 G_CALLBACK(focus_out_event), NULL);
      g_signal_connect(G_OBJECT(widget), "destroy",
					 G_CALLBACK(destroy_event), NULL);
}

void connectFocusModelessOther(GtkWidget *widget,const XAP_App * pApp, 
			       gboolean(*other_function)(void))
{
      g_object_set_data(G_OBJECT(widget), "pApp",
					  (void *)pApp);
      g_signal_connect(G_OBJECT(widget), "focus_in_event",
					 G_CALLBACK(focus_in_event_ModelessOther), (gpointer) other_function);
      g_signal_connect(G_OBJECT(widget), "focus_out_event",
					 G_CALLBACK(focus_out_event_Modeless), NULL);
      g_signal_connect(G_OBJECT(widget), "destroy",
					 G_CALLBACK(focus_out_event_Modeless), NULL);
}


void connectFocusModeless(GtkWidget *widget,const XAP_App * pApp)
{
      g_object_set_data(G_OBJECT(widget), "pApp",
					  (void *)pApp);
      g_signal_connect(G_OBJECT(widget), "focus_in_event",
					 G_CALLBACK(focus_in_event_Modeless), NULL);
      g_signal_connect(G_OBJECT(widget), "focus_out_event",
					 G_CALLBACK(focus_out_event_Modeless), NULL);
      g_signal_connect(G_OBJECT(widget), "destroy",
					 G_CALLBACK(destroy_event), NULL);
}


bool isTransientWindow(GtkWindow *window,GtkWindow *parent)
{
  if(window)
	{
	  while(window->transient_parent)
		{
		  window=window->transient_parent;
		  if(window==parent)
			return true;
		}
	}
  return false;
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

	g_signal_connect_after (G_OBJECT (dialog_window),
							  "destroy",
							  NULL,
							  NULL);
	g_signal_connect_after (G_OBJECT (dialog_window),
							  "delete_event",
							  G_CALLBACK(gtk_main_quit),
							  NULL);

	gtk_window_set_title(GTK_WINDOW(dialog_window), "AbiWord");

	// don't let user shrink or expand, but auto-size to
	// contents initially
    gtk_window_set_policy(GTK_WINDOW(dialog_window),
						  FALSE,
						  FALSE,
						  TRUE);

	// Intercept key strokes
	g_signal_connect(G_OBJECT(dialog_window),
					   "key_press_event",
					   G_CALLBACK(s_key_pressed),
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
	g_signal_connect (G_OBJECT (ok_button),
						"clicked",
						G_CALLBACK(gtk_main_quit),
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
	
	gtk_grab_add(GTK_WIDGET(dialog_window));
	gtk_widget_show(dialog_window);
	
	gtk_main();

	// clean up
	gtk_widget_destroy(GTK_WIDGET(dialog_window));
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

static void process_notebook_page( GtkWidget *notebook, 
                            GtkWidget *page,
                            struct fix_label_data *data );

static void fix_label_callback( GtkWidget *widget, 
                         gpointer _data );

//static void fix_label( GtkWidget *widget );


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
	UT_ASSERT(page);

	oldaccel = (GtkAccelGroup *)g_object_get_data( G_OBJECT(notebook),
													 KEY_ACCEL_GROUP );

	GtkWidget * new_page = gtk_notebook_get_nth_page ( notebook, page_num ) ;

	newaccel = (GtkAccelGroup *)g_object_get_data( G_OBJECT(new_page),  
													 KEY_ACCEL_GROUP );
	
	if ( oldaccel )
		gtk_window_remove_accel_group( topwindow, oldaccel );

	gtk_window_add_accel_group( topwindow, newaccel );
	g_object_set_data(G_OBJECT(notebook), KEY_ACCEL_GROUP, newaccel );
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * To process a notebook page
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void process_notebook_page( GtkWidget *notebook, 
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

	g_object_set_data( G_OBJECT(page), KEY_ACCEL_GROUP, (gpointer)newgroup );

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
static void fix_label_callback( GtkWidget *widget, gpointer _data )
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
						accel_group = g_object_get_data( G_OBJECT(w), KEY_ACCEL_GROUP);
						g_object_set_data( G_OBJECT(widget), KEY_ACCEL_GROUP, accel_group );
						gtk_window_add_accel_group( GTK_WINDOW(data->topwindow), 
																				(GtkAccelGroup *)accel_group);
			}
		}
			
		/* set the signal handler to change to accel groups */
		g_signal_connect (G_OBJECT(widget), "switch_page",
						  G_CALLBACK (on_notebook_switch_page),
						  data->topwindow );


	}	

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	
	 * handler for an option menu 
	 */
	else if ( GTK_IS_OPTION_MENU( widget ) ) {
		TRACE(("found option menu " ));

		accel_tie = (GtkWidget *)g_object_get_data( G_OBJECT(widget),
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

/*!
 * For a parented/displayed widget, this will just return
 * gtk_widget_ensure_style(w). For a non-displayed widgets,
 * This will return a valid GtkStyle for that widget
 */
GtkStyle *
get_ensured_style (GtkWidget * w)
{
	GtkStyle  * style = NULL;
	GtkWidget * hidden_window = NULL;

	if (w->parent == NULL)
	{
		hidden_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_container_add (GTK_CONTAINER (hidden_window), w);
	}

	gtk_widget_ensure_style (w);
	gtk_widget_realize (w);

	style = gtk_widget_get_style (w);
	UT_ASSERT(style);

	if (hidden_window)
	{
		// now we destroy the hidden window
		gtk_container_remove (GTK_CONTAINER(hidden_window), w);
		gtk_widget_destroy (hidden_window);
	}

	return style;
}

/*!
 * Creates a new GdkDrawingArea with the proper colormap and visual
 */
GtkWidget *createDrawingArea ()
{
  GtkWidget *area;
  
  gtk_widget_push_visual (gdk_rgb_get_visual ());
  gtk_widget_push_colormap (gdk_rgb_get_cmap ());
  
  area = gtk_drawing_area_new ();
  
  gtk_widget_pop_colormap ();
  gtk_widget_pop_visual ();
  
  return area;
}

/****************************************************************/
/****************************************************************/

static void sDoHelp ( XAP_Dialog * pDlg )
{
	// get any frame to open up a URL, doesn't matter which one
	XAP_App   * pApp   = XAP_App::getApp () ;
	XAP_Frame * pFrame = pApp->getLastFocussedFrame () ;
	
	// should always be valid, but just in case...
	UT_return_if_fail(pFrame);
	UT_return_if_fail(pDlg);

	xxx_UT_DEBUGMSG(("DOM: doing help: %d %s\n", pDlg->getHelpUrl().size (),
					 pDlg->getHelpUrl().c_str ()));
	
	// open the url
	if ( pDlg->getHelpUrl().size () > 0 )
    {
		pFrame->openURL ( pDlg->getHelpUrl().c_str() ) ;
    }
	else
    {
		// TODO: warn no help on this topic
		UT_DEBUGMSG(("NO HELP FOR THIS TOPIC!!\n"));
    }
}

/*!
 * Catch F1 keypress over a dialog and open up the help file, if any
 */
static gint modal_keypress_cb ( GtkWidget * wid, GdkEventKey * event, 
								XAP_Dialog * pDlg )
{
	// propegate keypress up if not F1
	if ( event->keyval == GDK_F1 || event->keyval == GDK_Help )
	{
		sDoHelp( pDlg ) ;

		// stop F1 propegation
		return TRUE ;
	}
	
	return FALSE ;		
}

/*!
 * Catch F1 keypress over a dialog and open up the help file, if any
 */
static gint nonmodal_keypress_cb ( GtkWidget * wid, GdkEventKey * event, 
								   XAP_Dialog * pDlg )
{
	// propegate keypress up if not F1
	if ( event->keyval == GDK_F1 || event->keyval == GDK_Help )
	{
		sDoHelp( pDlg ) ;
		return TRUE ;
	}
	else if ( event->keyval == GDK_Escape )
	{
		gtk_widget_destroy ( wid ) ;
		return TRUE ;
	}
	
	return FALSE ;
}

/*!
 * Centers a dialog, makes it transient, sets up the right window icon
 */
void centerDialog(GtkWidget * parent, GtkWidget * child)
{
	UT_return_if_fail(parent);
	UT_return_if_fail(child);

	gtk_window_set_transient_for(GTK_WINDOW(child),
								 GTK_WINDOW(parent));
#ifdef HAVE_GNOME
	gnome_window_icon_set_from_default (GTK_WINDOW(child));
#else
	GdkPixbuf * icon = gtk_window_get_icon(GTK_WINDOW(parent));	
	if ( NULL != icon )
	{
		gtk_window_set_icon(GTK_WINDOW(child), icon);
		UT_DEBUGMSG(("DOM: got&set icon: %p\n", icon));
	}
#endif
}

/*!
 * Runs the dialog \me as a modal dialog
 * 1) Connect focus to toplevel frame
 * 2) Centers dialog over toplevel window
 * 3) Connects F1 to help system
 * 4) Makes dialog modal
 * 5) Sets the default button to dfl_response, sets ESC to close
 * 6) Returns value of gtk_dialog_run(me)
 * 7) If \destroyDialog is true, destroys the dialog, else you have to call abiDestroyWidget()
 */
gint abiRunModalDialog(GtkDialog * me, XAP_Frame * pFrame, XAP_Dialog * pDlg,
					   gint dfl_response, bool destroyDialog )
{
	// To center the dialog, we need the frame of its parent.
	XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
	
	// Get the GtkWindow of the parent frame
	GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
	
	// connect focus to our parent frame
	connectFocus(GTK_WIDGET(me),pFrame);

	// center the dialog
	centerDialog ( parentWindow, GTK_WIDGET(me) ) ;
	
	// connect F1 to the help subsystem
	g_signal_connect (G_OBJECT(me), "key-press-event",
					  G_CALLBACK(modal_keypress_cb), pDlg);
	
	// set the default response
	gtk_dialog_set_default_response ( me, dfl_response ) ;
	
	// show the window
	gtk_widget_show ( GTK_WIDGET(me) ) ;
	
	// grab the dialog
	gtk_grab_add ( GTK_WIDGET(me) ) ;
	
	// and make it modal
	gtk_window_set_modal ( GTK_WINDOW(me), TRUE ) ;
	
	// now run the dialog
	gint result = gtk_dialog_run ( me ) ;
	
	// destroy the dialog
	if ( destroyDialog )
		abiDestroyWidget ( GTK_WIDGET ( me ) );
	
	return result ;
}

/*!
 * Sets up the dialog \me as a modeless dialog
 * 1) Connect focus to toplevel frame
 * 2) Centers dialog over toplevel window
 * 3) Makes the App remember this modeless dialog
 * 4) Connects F1 to help system
 * 5) Makes dialog non-modal (modeless)
 * 6) Sets the default button to dfl_response, sets ESC to close
 */
void abiSetupModelessDialog(GtkDialog * me, XAP_Frame * pFrame, XAP_Dialog * pDlg,
							gint dfl_response )
{
	// To center the dialog, we need the frame of its parent.
	XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);

	// remember the modeless id
	XAP_App::getApp()->rememberModelessId( pDlg->getDialogId(), (XAP_Dialog_Modeless *) pDlg);
	
	// Get the GtkWindow of the parent frame
	GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
	
	// connect focus to our parent frame
	connectFocusModeless(GTK_WIDGET(me), XAP_App::getApp());
	
	// center the dialog
	centerDialog ( parentWindow, GTK_WIDGET(me) ) ;
	
	// connect F1 to the help subsystem
	g_signal_connect (G_OBJECT(me), "key-press-event",
					  G_CALLBACK(nonmodal_keypress_cb), pDlg);
	
	// set the default response
	gtk_dialog_set_default_response ( me, dfl_response ) ;
	
	// show the window
	gtk_widget_show ( GTK_WIDGET(me) ) ;
	
	// grab the dialog
	gtk_grab_add ( GTK_WIDGET(me) ) ;
	
	// and mark it as modeless
	gtk_window_set_modal ( GTK_WINDOW(me), FALSE ) ;
}

/*!
 * Create a new GtkDialog
 */
GtkWidget * abiDialogNew(gboolean resizable)
{
  GtkWidget * dlg = gtk_dialog_new () ;
  gtk_window_set_resizable ( GTK_WINDOW(dlg), resizable ) ;
  return dlg ;
}

/*!
 * Create a new GtkDialog with this title
 */
GtkWidget * abiDialogNew(gboolean resizable, const char * title, ...)
{
  GtkWidget * dlg = abiDialogNew(resizable);
  
  if ( title != NULL && strlen ( title ) )
  {
    UT_String inStr ( "" ) ;

    va_list args;
    va_start (args, title);
    UT_String_vprintf (inStr, title, args);
    va_end (args);

	// TODO: locale->utf8 title
	
    // create the title
    gtk_window_set_title ( GTK_WINDOW(dlg), inStr.c_str() ) ;
  }

  return dlg ;
}

/*!
 * Add this stock button to the dialog and make it sensitive
 */
void abiAddStockButton (GtkDialog * me, const gchar * btn_id,
						gint response_id)
{
	UT_return_if_fail(me);
	UT_return_if_fail(btn_id);
	
	gtk_dialog_add_button(me, btn_id, response_id);
	gtk_dialog_set_response_sensitive(me, response_id, TRUE);
}

/*!
 * Add this locale-sensitive button to the dialog and
 * make it sensitive
 */
void abiAddButton(GtkDialog * me, const gchar * btn_id,
				  gint response_id)
{
	UT_return_if_fail(me);
	UT_return_if_fail(btn_id);

	// todo: possibly make me locale sensitive->utf8
	gtk_dialog_add_button(me, btn_id, response_id);
	gtk_dialog_set_response_sensitive(me, response_id, TRUE);
}

/*!
 * Calls gtk_widget_destroy on \me if \me is non-null
 * and GTK_IS_WIDGET(me)
 */
void abiDestroyWidget(GtkWidget * me)
{
  if(me && GTK_IS_WIDGET(me))
    gtk_widget_destroy(me);
}

GdkWindow * getRootWindow(GtkWidget * widget)
{
	UT_return_val_if_fail(widget, NULL);

	// BROKEN!!!!
	return gdk_get_default_root_window() ;
}
