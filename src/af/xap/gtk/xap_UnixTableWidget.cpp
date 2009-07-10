/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * xap_TableWidget.cpp
 * Copyright 2002 Joaquin Cuenca Abela
 *
 * Authors:
 *   Joaquin Cuenca Abela (e98cuenc@yahoo.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdk.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtkbutton.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtksignal.h>
#include <gtk/gtktoolbar.h>
#include "xap_UnixTableWidget.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string_class.h"

/* NONE:UINT,UINT (/dev/stdin:1) */
static void
g_cclosure_user_marshal_VOID__UINT_UINT (GClosure     *closure,
                                         GValue       * /*return_value*/,
                                         guint         n_param_values,
                                         const GValue *param_values,
                                         gpointer      /*invocation_hint*/,
                                         gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__UINT_UINT) (gpointer     data1,
                                                guint        arg_1,
                                                guint        arg_2,
                                                gpointer     data2);
  register GMarshalFunc_VOID__UINT_UINT callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;

  UT_return_if_fail (n_param_values == 3);
  UT_DEBUGMSG(("IN AbiTable g_cclose_.. \n"));
  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_get_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_get_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__UINT_UINT) (marshal_data ? marshal_data : cc->callback);
  UT_DEBUGMSG(("Calling callback marshell data %p cc %p \n",callback,cc));

  callback (data1,
            g_value_get_uint (param_values + 1),
            g_value_get_uint (param_values + 2),
            data2);
}

enum
{
	SELECTED,
	LAST_SIGNAL
};

static gint abi_table_signals [LAST_SIGNAL] = { 0 };

static GtkObjectClass *abi_table_parent_class;

/* ------------------- now the guts of AbiTable ---------------------- */

static const guint cell_width = 24;
static const guint cell_height = 24;
static const guint cell_spacing = 4;
static const guint init_rows = 0;
static const guint init_cols = 0;

static inline void
cells_to_pixels(guint cols, guint rows, guint* w, guint* h)
{
	UT_return_if_fail(w);
	UT_return_if_fail(h);
	
	*w = cell_width * cols + cell_spacing * (cols + 1);
	*h = cell_height * rows + cell_spacing * (rows + 1);
}

static inline void
pixels_to_cells(guint w, guint h, guint* cols, guint* rows)
{
	UT_return_if_fail(cols);
	UT_return_if_fail(rows);
	
	*cols = w / (cell_width + cell_spacing) + 1;
	*rows = h / (cell_height + cell_spacing) + 1;
}

static void
abi_table_resize(AbiTable* table)
{
	guint width;
	guint height;
	char* text;	
	GtkRequisition size;
	
	UT_return_if_fail(table);

	if (table->selected_rows == 0 && table->selected_cols == 0)
		text = g_strdup(table->szCancel);
	else
	{
		UT_UTF8String prText =  "%d x %d ";
		UT_UTF8String s = table->szTable;
		prText += s;
		text = g_strdup_printf(prText.utf8_str(), table->selected_rows, table->selected_cols);
	}
	cells_to_pixels(table->total_cols, table->total_rows, &width, &height);
	gtk_widget_size_request(GTK_WIDGET(table->window_label), &size);
	
	gtk_label_set_text(table->window_label, text);
	gtk_window_resize(table->window, width + 1, height + size.height);
	
	g_free(text);
}

extern "C" void
abi_table_set_selected (AbiTable* abi_table, guint rows, guint cols)
{
	UT_return_if_fail (abi_table);

	abi_table->selected_rows = rows;
	abi_table->selected_cols = cols;

	abi_table_resize(abi_table);
}

extern "C" void
abi_table_get_selected (const AbiTable* abi_table, guint* rows, guint* cols)
{
	UT_return_if_fail (abi_table);

	if (rows)
		*rows = abi_table->selected_rows;

	if (cols)
		*cols = abi_table->selected_cols;
}

extern "C" void
abi_table_set_max_size (AbiTable* abi_table, guint rows, guint cols)
{
	UT_return_if_fail (abi_table);

	abi_table->total_rows = rows;
	abi_table->total_cols = cols;

	abi_table_resize(abi_table);
}


extern "C" void
abi_table_set_labels(AbiTable* abi_table, const gchar * szTable, const gchar * szCancel)
{
	if(abi_table->szTable)
		g_free(abi_table->szTable);
	abi_table->szTable = g_strdup(szTable);	
	if(abi_table->szCancel)
		g_free(abi_table->szCancel);
	abi_table->szCancel = g_strdup(szCancel);	
}

extern "C" void
abi_table_get_max_size (const AbiTable* abi_table, guint* rows, guint* cols)
{
	UT_return_if_fail (abi_table);

	if (rows)
		*rows = abi_table->total_rows;

	if (cols)
		*cols = abi_table->total_cols;
}

static gboolean
on_drawing_area_event (GtkWidget *area, GdkEventExpose *, gpointer user_data)
{
	AbiTable* table = static_cast<AbiTable*>(user_data);
	guint i;
	guint j;
	guint selected_rows = table->selected_rows;
	guint selected_cols = table->selected_cols;
	guint x;
	guint y;
	
	// TODO: use gtk_paint_box, gtk_paint_line

	gdk_draw_rectangle (area->window,
						area->style->bg_gc[GTK_STATE_NORMAL],
						TRUE,
						0, 0,
						area->allocation.width,
						area->allocation.height);

	for (i = 0; i < table->total_rows; ++i)
		for (j = 0; j < table->total_cols; ++j)
		{
			cells_to_pixels(j, i, &x, &y);
			
			gdk_draw_rectangle (area->window,
								area->style->dark_gc[GTK_STATE_NORMAL],
								FALSE,
								x - 1, y - 1,
								cell_width + 1,
								cell_height + 1);

			if (i < selected_rows && j < selected_cols)
				gdk_draw_rectangle (area->window,
									table->selected_gc,
									TRUE,
									x, y,
									cell_width,
									cell_height);
			else
				gdk_draw_rectangle (area->window,
									area->style->white_gc,
									TRUE,
									x, y,
									cell_width,
									cell_height);
		}

	/* black border line */
	gdk_draw_line (area->window,
				   area->style->black_gc,
				   area->allocation.width - 1, 0, area->allocation.width - 1, area->allocation.height - 1);
	gdk_draw_line (area->window,
				   area->style->black_gc,
				   area->allocation.width - 1, area->allocation.height - 1, 0, area->allocation.height - 1);

	/* dark border line */
	gdk_draw_line (area->window,
				   area->style->dark_gc[GTK_STATE_NORMAL],
				   area->allocation.width - 2, 1, area->allocation.width - 2, area->allocation.height - 2);
	gdk_draw_line (area->window,
				   area->style->dark_gc[GTK_STATE_NORMAL],
				   area->allocation.width - 2, area->allocation.height - 2, 1, area->allocation.height - 2);

	/* ligth border line */
	gdk_draw_line (area->window,
				   area->style->light_gc[GTK_STATE_NORMAL],
				   0, 0, area->allocation.width - 3, 0);
	gdk_draw_line (area->window,
				   area->style->light_gc[GTK_STATE_NORMAL],
				   0, 0, 0, area->allocation.height - 2);

	return TRUE;
}

static inline guint
my_max(guint a, guint b)
{
	return a < b ? b : a;
}

static gboolean
on_motion_notify_event (GtkWidget *window, GdkEventMotion *ev, gpointer user_data)
{
	AbiTable* table = static_cast<AbiTable*>(user_data);
	gboolean changed = FALSE;
	guint selected_cols;
	guint selected_rows;

	if (ev->x < 0 || ev->y < 0)
		return TRUE;
	
	pixels_to_cells(static_cast<guint>(ev->x), static_cast<guint>(ev->y), &selected_cols, &selected_rows);

	if ((selected_cols != table->selected_cols) || (selected_rows != table->selected_rows))
	{
		/* grow or shrink the table widget as necessary */
		table->selected_cols = selected_cols;
		table->selected_rows = selected_rows;

		if (table->selected_rows <= 0 || table->selected_cols <= 0)
			table->selected_rows = table->selected_cols = 0;

		table->total_rows = my_max(table->selected_rows + 1, 3);
		table->total_cols = my_max(table->selected_cols + 1, 3);

		abi_table_resize(table);
		gtk_widget_queue_draw_area (window, 0, 0, 
					    window->allocation.width, window->allocation.height);

		changed = TRUE;
	}

	return TRUE;
}

static void
restart_widget (AbiTable *table)
{
	table->selected_cols = init_cols;
	table->selected_rows = init_rows;
	table->total_cols = my_max(init_cols + 1, 5);
	table->total_rows = my_max(init_rows + 1, 6);

	gtk_button_released(GTK_BUTTON(table));

	gtk_widget_hide(GTK_WIDGET(table->window));
}

/*
 * Fires signal "selected", and reset and hide the widget
 */
static void
emit_selected (AbiTable *table)
{
	gtk_widget_hide(GTK_WIDGET(table->window));

	while (gtk_events_pending())
		gtk_main_iteration();

	if (table->selected_rows > 0 && table->selected_cols > 0)
		g_signal_emit (GTK_OBJECT (table),
			       abi_table_signals [SELECTED], 0,
			       table->selected_rows, table->selected_cols);

	restart_widget(table);
}

static gboolean
on_button_release_event (GtkWidget *, GdkEventButton *ev, gpointer user_data)
{
	AbiTable* table = static_cast<AbiTable*>(user_data);

	/* Quick test to know if we're possibly over the button */
	if (ev->y < 0.0 && ev->x >= 0.0)
	{
		GtkRequisition size;
		
		gtk_widget_size_request(GTK_WIDGET(table), &size);

		/* And now, precise and slightly slower test.
		   I wonder if the double test really matters from a speed pov */
		if (-ev->y < size.height && ev->x < size.width)
			return TRUE;
	}
	
	emit_selected(table);
	
	return TRUE;
}

static gboolean
on_leave_event (GtkWidget *area,
				GdkEventCrossing *event,
				gpointer user_data)
{
	AbiTable* table = static_cast<AbiTable*>(user_data);

	if (GTK_WIDGET_VISIBLE(GTK_WIDGET(table->window)) && (event->x < 0 || event->y < 0))
	{
		table->selected_rows = 0;
		table->selected_cols = 0;
		table->total_rows = my_max(table->selected_rows + 1, 3);
		table->total_cols = my_max(table->selected_cols + 1, 3);

		abi_table_resize(table);
		gtk_widget_queue_draw_area (area, 0, 0, area->allocation.width,
					    area->allocation.height);
	}

	return TRUE;
}

static gboolean
popup_grab_on_window (GdkWindow *window,
					  guint32    activate_time)
{
	GdkEventMask emask = static_cast<GdkEventMask>(GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
												   GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK |
												   GDK_ENTER_NOTIFY_MASK) ;
	if ((gdk_pointer_grab (window, FALSE,emask,
						   NULL, NULL, activate_time) == 0))
	{
		if (gdk_keyboard_grab (window, FALSE,
							   activate_time) == 0)
			return TRUE;
		else
		{
			gdk_pointer_ungrab (activate_time);
			return FALSE;
		}
	}

	return FALSE;
}

static void
on_pressed(GtkButton* button, gpointer user_data)
{
	AbiTable* table = static_cast<AbiTable*>(user_data);
	int left, top;
	GdkColor selected_color;

	/* Temporarily grab pointer and keyboard on a window we know exists; we
	 * do this so that the grab (with owner events == TRUE) affects
	 * events generated when the window is mapped, such as enter
	 * notify events on subwidgets. If the grab fails, bail out.
	 */
	if (!popup_grab_on_window (GTK_WIDGET(button)->window,
							   gtk_get_current_event_time ()))
		return;

	gdk_window_get_origin (GTK_WIDGET(table)->window, &left, &top);
	gtk_window_move(table->window,
					left + GTK_WIDGET(table)->allocation.x,
					top + GTK_WIDGET(table)->allocation.y + GTK_WIDGET(table)->allocation.height);
	abi_table_resize(table);

	gtk_widget_show(GTK_WIDGET(table->window));
	gtk_widget_grab_focus(GTK_WIDGET(table->window));

	/* Now transfer our grabs to the popup window; this
	 * should always succeed.
	 */
	popup_grab_on_window (GTK_WIDGET(table->area)->window,
			      gtk_get_current_event_time ());

	selected_color = (GTK_WIDGET (button))->style->base[GTK_STATE_SELECTED];

	/* leak */
	table->selected_gc = gdk_gc_new(GTK_WIDGET(button)->window);
	gdk_gc_set_rgb_fg_color(table->selected_gc, &selected_color);
}

gboolean
on_key_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	AbiTable* table = static_cast<AbiTable*>(user_data);
	gboolean grew = FALSE;

	switch (event->keyval)
	{
	case GDK_Up:
	case GDK_KP_Up:
		if (table->selected_rows > 0)
			--table->selected_rows;
		break;
	case GDK_Down:
	case GDK_KP_Down:
		grew = TRUE;
		++table->selected_rows;
		break;
	case GDK_Left:
	case GDK_KP_Left:
		if (table->selected_cols > 0)
			--table->selected_cols;
		break;
	case GDK_Right:
	case GDK_KP_Right:
		grew = TRUE;
		++table->selected_cols;
		break;
	case GDK_Escape:
		restart_widget(table);
		return TRUE;
	case GDK_KP_Space:
	case GDK_KP_Enter:
	case GDK_space:
	case GDK_3270_Enter:
	case GDK_ISO_Enter:
	case GDK_Return:
		emit_selected(table);
		return TRUE;
	}

	if(table->selected_rows == 0 || table->selected_cols == 0)
		table->selected_rows = table->selected_cols = (grew ? 1 : 0) ;

	table->total_rows = my_max(table->selected_rows + 1, 3);
	table->total_cols = my_max(table->selected_cols + 1, 3);

	abi_table_resize(table);
	gtk_widget_queue_draw_area (widget, 0, 0, widget->allocation.width,
				    widget->allocation.height);

	return TRUE;
}


/* XPM */
static const char * widget_tb_insert_table_xpm[] = {
	"24 24 32 1",
	" 	c None",
	".	c #000000",
	"+	c #FFFFFF",
	"@	c #B1B1B1",
	"#	c #BABABA",
	"$	c #E4E4E4",
	"%	c #ECECEC",
	"&	c #A8A8A8",
	"*	c #ADADAD",
	"=	c #B9B9B9",
	"-	c #ABABAB",
	";	c #B8B8B8",
	">	c #B5B5B5",
	",	c #AAAAAA",
	"'	c #858585",
	")	c #828282",
	"!	c #707070",
	"~	c #BCBCBC",
	"{	c #A7A7A7",
	"]	c #8D8D8D",
	"^	c #737373",
	"/	c #ACACAC",
	"(	c #878787",
	"_	c #747474",
	":	c #A9A9A9",
	"<	c #E9E9E9",
	"[	c #C3C3C3",
	"}	c #BFBFBF",
	"|	c #757575",
	"1	c #7E7E7E",
	"2	c #BBBBBB",
	"3	c #A3A3A3",
	"                        ",
	"                        ",
	"                        ",
	"   ...................  ",
	"   .+++++@+++++#++++$.  ",
	"   .+%%%%&%%%%%*%%%%=.  ",
	"   .+%%%%-%%%%%-%%%%;.  ",
	"   .>&,,&'&----)&---!.  ",
	"   .+%%%%-%%%%%-%%%%~.  ",
	"   .+%%%%-%%%%%-%%%%~.  ",
	"   .+%%%%-%%%%%-%%%%~.  ",
	"   .#{---'-----]*---^.  ",
	"   .+%%%%-%%%%%-%%%%~.  ",
	"   .+%%%%-%%%%%-%%%%;.  ",
	"   .+%%%%-%%%%%-%%%%~.  ",
	"   .;&--/(&{--&'&---_.  ",
	"   .+%%%%-%%%%%-%%%%;.  ",
	"   .+%%%%&%%%%%:%%%%~.  ",
	"   .<[[[}|~~~~~1;~~23.  ",
	"   ...................  ",
	"                        ",
	"                        ",
	"                        ",
	"                        "};


static void
register_stock_icon(void)
{
	static gboolean registered = FALSE;
  
	if (!registered)
	{
		GdkPixbuf *pixbuf;
		GtkIconFactory *factory;

		static GtkStockItem items[] = {
			{ (gchar*)"abi-table-widget",
			  (gchar*)"_Table",
			  static_cast<GdkModifierType>(0), 0, NULL }
		};
      
		registered = TRUE;

		/* Register our stock items */
		gtk_stock_add (items, G_N_ELEMENTS (items));
      
		/* Add our custom icon factory to the list of defaults */
		factory = gtk_icon_factory_new ();
		gtk_icon_factory_add_default (factory);

		// Must be C cast
		pixbuf = gdk_pixbuf_new_from_xpm_data((const char **)widget_tb_insert_table_xpm);

		/* Register icon to accompany stock item */
		if (pixbuf != NULL)
		{
			GtkIconSet *icon_set;
          
			icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
			gtk_icon_factory_add (factory, "abi-table-widget", icon_set);
			gtk_icon_set_unref (icon_set);
			g_object_unref (G_OBJECT (pixbuf));
		}
		else
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
				
		/* Drop our reference to the factory, GTK will hold a reference. */
		g_object_unref (G_OBJECT (factory));
	}
}

void abi_table_set_icon(AbiTable* abi_table,GtkWidget * gtkImageIcon)
{
	if(!GTK_IS_IMAGE(G_OBJECT(gtkImageIcon)))
	   return;
	g_object_unref (G_OBJECT (abi_table->icon));
	abi_table->icon = gtkImageIcon;
}

/* ------------------- and now the GObject part ---------------------- */

static void
abi_table_class_init (AbiTableClass *klass)
{
	GtkObjectClass *object_class = reinterpret_cast<GtkObjectClass*>(klass);

	abi_table_parent_class = static_cast<GtkObjectClass *>(g_type_class_peek (GTK_TYPE_BUTTON));
	abi_table_signals [SELECTED] =
		g_signal_new ("selected",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (AbiTableClass, selected),
			      NULL, NULL,
			      g_cclosure_user_marshal_VOID__UINT_UINT,
			      G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_UINT);
}


static void
abi_table_init (AbiTable* table)
{
	UT_UTF8String prText =  "%d x %d ";
	char* text = g_strdup_printf(prText.utf8_str(), init_rows, init_cols);

	register_stock_icon();
	
	table->button_box = gtk_vbox_new(FALSE, 0);

	table->window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_POPUP));
	table->window_vbox = GTK_VBOX(gtk_vbox_new(FALSE, 0));

	table->area = GTK_DRAWING_AREA(gtk_drawing_area_new());

	table->handlers = 0;
	table->window_label = GTK_LABEL(gtk_label_new(text));
	g_free(text);
	table->szTable = NULL;
	table->szCancel = NULL;
	gtk_container_add(GTK_CONTAINER(table->window), GTK_WIDGET(table->window_vbox));
	gtk_box_pack_end(GTK_BOX(table->window_vbox), GTK_WIDGET(table->window_label), FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(table->window_vbox), GTK_WIDGET(table->area), TRUE, TRUE, 0);

	gtk_widget_show_all(GTK_WIDGET(table->window_vbox));

	table->selected_rows = init_rows;
	table->selected_cols = init_cols;

	table->total_rows = my_max(init_rows + 1, 5);
	table->total_cols = my_max(init_cols + 1, 6);

	abi_table_resize(table);

	table->icon = NULL;
	if (gtk_stock_lookup ("abi-table-widget", &table->stock_item))
	{
		table->label = gtk_label_new_with_mnemonic(table->stock_item.label);
		table->icon = gtk_image_new_from_stock ("abi-table-widget", GTK_ICON_SIZE_LARGE_TOOLBAR);
		gtk_widget_show(table->icon);
		gtk_widget_show(table->label);
		//
		// We actually never want this label in toolbar
		//		gtk_box_pack_end(GTK_BOX(table->button_box), table->label, FALSE, FALSE, 0);
		gtk_box_pack_end(GTK_BOX(table->button_box), table->icon, FALSE, FALSE, 0);
		UT_DEBUGMSG(("abi-table icon loaded %p !\n",table->icon));
	}
	else
	{
		/* it should not happen... */
		UT_DEBUGMSG(("abi-table icon did not load !\n"));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		table->label = gtk_label_new_with_mnemonic("_Table");
		//		gtk_box_pack_end(GTK_BOX(table->button_box), table->label, FALSE, FALSE, 0);
	}

	gtk_container_add(GTK_CONTAINER(table), GTK_WIDGET(table->button_box));

	g_signal_connect(G_OBJECT(table), "pressed",
			 G_CALLBACK(on_pressed), static_cast<gpointer>(table));
	g_signal_connect(G_OBJECT(table->area), "expose_event",
			 G_CALLBACK(on_drawing_area_event), static_cast<gpointer>(table));
	g_signal_connect(G_OBJECT(table->area), "motion_notify_event",
			 G_CALLBACK(on_motion_notify_event), static_cast<gpointer>(table));
	g_signal_connect(G_OBJECT(table->area), "button_release_event",
			 G_CALLBACK(on_button_release_event), static_cast<gpointer>(table));
	g_signal_connect(G_OBJECT(table->area), "button_press_event",
			 G_CALLBACK(on_button_release_event), static_cast<gpointer>(table));
	g_signal_connect(G_OBJECT(table->area), "leave_notify_event",
			 G_CALLBACK(on_leave_event), static_cast<gpointer>(table));
	g_signal_connect(G_OBJECT(table->window), "key_press_event",
			 G_CALLBACK(on_key_event), static_cast<gpointer>(table));

	gtk_widget_set_events (GTK_WIDGET(table->area), GDK_EXPOSURE_MASK
						   | GDK_LEAVE_NOTIFY_MASK
						   | GDK_BUTTON_PRESS_MASK
						   | GDK_BUTTON_RELEASE_MASK
						   | GDK_POINTER_MOTION_MASK
						   | GDK_KEY_PRESS_MASK
						   | GDK_KEY_RELEASE_MASK);

	gtk_button_set_relief (GTK_BUTTON (table), GTK_RELIEF_NORMAL);
}


 GType
abi_table_get_type (void)
{
	static GType type = 0;

	if (!type)
	{
		static const GTypeInfo info =
			{
				sizeof (AbiTableClass),
				NULL,		/* base_init */
				NULL,		/* base_finalize */
				reinterpret_cast<GClassInitFunc>(abi_table_class_init),
				NULL,		/* class_finalize */
				NULL,		/* class_data */
				sizeof (AbiTable),
				0,		/* n_preallocs */
				reinterpret_cast<GInstanceInitFunc>(abi_table_init),
				NULL
			};

		type = g_type_register_static (GTK_TYPE_BUTTON, "AbiTable", &info, static_cast<GTypeFlags>(0));
	}

	return type;
}

 GtkWidget*
abi_table_new (void)
{
	UT_DEBUGMSG(("COnstructing ABITABLE widget \n"));
	return GTK_WIDGET (g_object_new (abi_table_get_type (), NULL));
}

 GtkWidget* abi_table_get_label(AbiTable* abi_table)
{
	return abi_table->label;
}
