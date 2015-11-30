/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2006 Robert Staudinger <robert.staudinger@gmail.com>
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

/*
 * Port to Maemo Development Platform
 * Author: INdT - Renato Araujo <renato.filho@indt.org.br>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>

#include <string>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <goffice/gtk/go-combo-box.h>
#include <goffice/gtk/go-combo-color.h>

#include "ap_Features.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ev_UnixToolbar.h"
#include "xap_Types.h"
#include "xap_Gtk2Compat.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrameImpl.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Layouts.h"
#include "ev_Toolbar_Labels.h"
#include "ev_Toolbar_Control.h"
#include "ev_EditEventMapper.h"
#include "xap_UnixTableWidget.h"
#include "ev_UnixToolbar_ViewListener.h"
#include "xav_View.h"
#include "xap_Prefs.h"
#include "fv_View.h"
#include "xap_EncodingManager.h"
#include "xap_UnixDialogHelper.h"
#include "xap_UnixFontPreview.h"
#include "xap_FontPreview.h"
#include "ut_string_class.h"
#include "pt_PieceTable.h"
#include "ap_Toolbar_Id.h"
#include "ap_UnixStockIcons.h"
#include "ev_UnixFontCombo.h"

#ifdef ENABLE_MENUBUTTON
#include "ev_UnixMenuBar.h"
#endif

#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#include <hildon/hildon-window.h>
#endif

#define PROP_HANDLER_ID "handler-id"

#ifndef UINT_RGBA_R
#	define UINT_RGBA_R GO_COLOR_UINT_R
#	define UINT_RGBA_G GO_COLOR_UINT_G
#	define UINT_RGBA_B GO_COLOR_UINT_B
#endif

class _wd;

static void s_proxy_activated(GtkMenuItem * , _wd * wd);

static const GtkTargetEntry s_AbiTBTargets[] = {{(gchar*)"abi-toolbars",0,0}};

static GtkWidget *
toolbar_append_item_with_proxy (GtkToolbar *toolbar,
                                GtkWidget	*widget,
                                const char *text,
                                gboolean	 show,
                                /* for the proxy action */
                                const char     *action_name,
                                gpointer        data)
{
	GtkToolItem *tool_item;

	UT_ASSERT(GTK_IS_TOOLBAR (toolbar));
	UT_ASSERT(widget != NULL);

	if (GTK_IS_TOOL_ITEM (widget)) {
		tool_item = GTK_TOOL_ITEM (widget);
		gtk_tool_item_set_tooltip_text(tool_item, text);
	}
	else {
		tool_item = gtk_tool_item_new ();
		GtkWidget *box = gtk_event_box_new ();
		gtk_container_add (GTK_CONTAINER (tool_item), box);
		gtk_container_add (GTK_CONTAINER (box), widget);
		gtk_tool_item_set_tooltip_text(tool_item, text);
		if (action_name && data) {
			GtkWidget	*menu_item;
            menu_item = gtk_menu_item_new_with_label(text);
			g_object_set_data(G_OBJECT(menu_item),
							  "abi_action", (gpointer)action_name);
			g_signal_connect (menu_item, "activate",
							  G_CALLBACK (s_proxy_activated), data);
			gtk_tool_item_set_proxy_menu_item (tool_item, text, menu_item);
		}
	}
	gtk_toolbar_insert (toolbar, tool_item, -1);
	if (show) {
		gtk_widget_show_all (GTK_WIDGET (tool_item));
	}

	return GTK_WIDGET (tool_item);
}

/*!
 * Append a widget to the toolbar,
 * wrap it in a GtkToolItem if it isn't one already.
 */
static GtkWidget *
toolbar_append_item (GtkToolbar *toolbar,
					 GtkWidget  *widget,
					 const char *text,
					 gboolean	 show)
{
	return toolbar_append_item_with_proxy(toolbar, widget, text,
                                          show, NULL, NULL);
}

/*!
 * Append a GtkToolButton to the toolbar.
 */
static GtkWidget *
toolbar_append_button (GtkToolbar 	*toolbar,
					   const gchar	*icon_name,
					   const gchar	*label,
					   const gchar  *tooltip,
					   GCallback	 handler,
					   gpointer		 data,
					   gulong		*handler_id)
{
	GtkToolItem *item;
	GtkWidget *icon;
	gchar		*stock_id;

	stock_id = abi_stock_from_toolbar_id (icon_name);
	icon = gtk_image_new_from_icon_name (stock_id, GTK_ICON_SIZE_LARGE_TOOLBAR);
	item = gtk_tool_button_new (icon, label);
	g_free (stock_id);
	stock_id = NULL;
	*handler_id = g_signal_connect (G_OBJECT (item), "clicked", handler, data);

	return (GtkWidget *) toolbar_append_item (toolbar, GTK_WIDGET (item),
											  tooltip, TRUE);
}

/*!
 * Append a GtkToggleToolButton to the toolbar.
 */
static GtkWidget *
toolbar_append_toggle (GtkToolbar 	*toolbar,
					   const gchar	*icon_name,
					   const gchar	*label,
					   const gchar  *tooltip,
					   GCallback	 handler,
					   gpointer		 data,
					   gboolean		 show,
					   gulong		*handler_id)
{
	GtkToolItem *item;
	GtkWidget *icon;
	gchar		*stock_id;

	stock_id = abi_stock_from_toolbar_id (icon_name);
	icon = gtk_image_new_from_icon_name (stock_id, GTK_ICON_SIZE_LARGE_TOOLBAR);
	item = gtk_toggle_tool_button_new ();
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(item), icon);
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(item), label);
	g_free (stock_id);
	stock_id = NULL;
	*handler_id = g_signal_connect (G_OBJECT (item), "toggled", handler, data);

	return (GtkWidget *) toolbar_append_item (toolbar, GTK_WIDGET (item),
											  tooltip, show);
}

#ifdef ENABLE_MENUBUTTON
static void
menubutton_show_cb (GtkWidget *widget, gpointer data)
{
	g_signal_stop_emission_by_name (G_OBJECT (widget), "show");
	gtk_widget_hide_all (widget);
}

/*!
 * Append a GtkMenuToolButton to the toolbar.
 */
static GtkWidget *
toolbar_append_menubutton (GtkToolbar 	*toolbar,
						   GtkWidget    *menu,
						   const gchar	*icon_name, 
						   const gchar	*label, 
						   const gchar  *tooltip,
						   const gchar  *private_text, 
						   GCallback	 handler, 
						   gpointer		 data, 
						   gulong		*handler_id)
{
	GtkToolItem *item = gtk_menu_tool_button_new (NULL, NULL);
	gtk_menu_tool_button_set_menu (GTK_MENU_TOOL_BUTTON (item), menu);

	/* We want to hide the button part of the menu button -- to prevent
	 * it from showing again when gtk_widget_show () is called on the
	 * menubutton, we register a callback to the "show" signal, and hide
	 * it if something tries to show it.
	 */
	GtkWidget * button_box = gtk_bin_get_child (GTK_BIN (item));
	if (button_box)
	{
		GList * children =
			gtk_container_get_children (GTK_CONTAINER (button_box));

		if (children && children->data)
		{
			GtkWidget * button = GTK_WIDGET (children->data);
			gtk_widget_hide_all (button);

			g_signal_connect(G_OBJECT (button), "show",
							 G_CALLBACK (menubutton_show_cb), NULL);

			g_list_free (children);
		}
	}
	
	return (GtkWidget *) toolbar_append_item (toolbar, GTK_WIDGET (item), 
											  tooltip, TRUE);
}
#endif

/*!
 * Append a GtkSeparatorToolItem to the toolbar.
 */
static void
toolbar_append_separator (GtkToolbar *toolbar)
{
	GtkToolItem *item;

	item = gtk_separator_tool_item_new ();
	gtk_toolbar_insert (toolbar, item, -1);
	gtk_widget_show (GTK_WIDGET (item));
}

/*!
 * Set active text in a simple combobox.
 */
static gboolean
combo_box_set_active_text (GtkComboBox *combo, 
						   const gchar *text, 
						   gulong		handler_id)
{
	GtkTreeModel 	*model;
	GtkTreeIter		 iter;
	gboolean 		 iter_valid;
	gboolean		 next;
	gchar			*value;
	gulong			 prelight_handler_id;

	model = gtk_combo_box_get_model (combo);
	next = gtk_tree_model_get_iter_first (model, &iter);
	value = NULL;
	iter_valid = FALSE;
	while (next) {
		gtk_tree_model_get (model, &iter, 
							0, &value, 
							-1);
		if (0 == strcmp (text, value)) {
			g_free (value); value = NULL;
			iter_valid = true;
			break;
		}
		g_free (value);
		value = NULL;
		next = gtk_tree_model_iter_next (model, &iter);
	}

	if (iter_valid) {
		g_signal_handler_block (G_OBJECT (combo), handler_id);
		prelight_handler_id = 0;
		if (ABI_IS_FONT_COMBO (combo)) {
			prelight_handler_id = GPOINTER_TO_UINT(g_object_get_data (G_OBJECT (combo), PROP_HANDLER_ID));
			g_signal_handler_block (G_OBJECT (combo), prelight_handler_id);
		}

		gtk_combo_box_set_active_iter (combo, &iter);

		g_signal_handler_unblock (G_OBJECT (combo), handler_id);
		if (prelight_handler_id) {
			g_signal_handler_unblock (G_OBJECT (combo), prelight_handler_id);
		}
	}
	else if (ABI_IS_FONT_COMBO (combo)) {
		// special case font combo, non existant entries are added
		g_signal_handler_block (G_OBJECT (combo), handler_id);
		prelight_handler_id = GPOINTER_TO_UINT(g_object_get_data (G_OBJECT (combo), PROP_HANDLER_ID));
		g_signal_handler_block (G_OBJECT (combo), prelight_handler_id);

		abi_font_combo_insert_font (ABI_FONT_COMBO (combo), text, TRUE);

		g_signal_handler_unblock (G_OBJECT (combo), handler_id);
		g_signal_handler_unblock (G_OBJECT (combo), prelight_handler_id);
	}

	return next;
}

class _wd								// a private little class to help
{										// us remember all the widgets that
public:									// we create...
	_wd(EV_UnixToolbar * pUnixToolbar, XAP_Toolbar_Id id, GtkWidget * widget = NULL)
	{
		m_pUnixToolbar = pUnixToolbar;
		m_id = id;
		m_widget = widget;
		m_blockSignal = false;
		m_handlerId = 0;
	};
	
	~_wd(void)
	{
	};

	static void s_callback(GtkWidget * /* widget */, gpointer user_data)
	{
		// this is a static callback method and does not have a 'this' pointer.
		// map the user_data into an object and dispatch the event.
	
		_wd * wd = static_cast<_wd *>(user_data);
		UT_return_if_fail(wd);
		GdkEvent * event = gtk_get_current_event();
		wd->m_pUnixToolbar->setCurrentEvent(event);
		if (!wd->m_blockSignal)
		{
			wd->m_pUnixToolbar->toolbarEvent(wd, 0, 0);
		}
	};

	static void s_new_table(GtkWidget * /*table*/, int rows, int cols, gpointer* user_data)
	{
		// this is a static callback method and does not have a 'this' pointer.
		// map the user_data into an object and dispatch the event.
	
		_wd * wd = reinterpret_cast<_wd *>(user_data);
		UT_return_if_fail(wd);
		GdkEvent * event = gtk_get_current_event();
		wd->m_pUnixToolbar->setCurrentEvent(event);
		if (!wd->m_blockSignal && (rows > 0) && (cols > 0))
		{
			FV_View * pView = static_cast<FV_View *>(wd->m_pUnixToolbar->getFrame()->getCurrentView());
			pView->cmdInsertTable(rows,cols,NULL);
		}
	}

	static void s_drag_begin(GtkWidget  *widget,
							 GdkDragContext     * /*context*/)
	{
		_wd * wd = static_cast<_wd *>(g_object_get_data(G_OBJECT(widget),"wd_pointer"));
		XAP_Frame * pFrame = static_cast<XAP_Frame *>(wd->m_pUnixToolbar->getFrame());
		EV_Toolbar * pTBsrc = static_cast<EV_Toolbar *>(wd->m_pUnixToolbar);
		pFrame->dragBegin(wd->m_id,pTBsrc);
	};


	static void s_drag_drop(GtkWidget  *widget,
							GdkDragContext     *context,
							gint /*x*/, gint /*y*/, guint /*time*/ )
	{
		_wd * wd = static_cast<_wd *>(g_object_get_data(G_OBJECT(widget),"wd_pointer"));
		GtkWidget * src = gtk_drag_get_source_widget(context);
		_wd * wdSrc = static_cast<_wd *>(g_object_get_data(G_OBJECT(src),"wd_pointer"));
		
		XAP_Frame * pFrame = static_cast<XAP_Frame *>(wd->m_pUnixToolbar->getFrame());
		EV_Toolbar * pTBdest = static_cast<EV_Toolbar *>(wd->m_pUnixToolbar);
		EV_Toolbar * pTBsrc = static_cast<EV_Toolbar *>(wdSrc->m_pUnixToolbar);
		pFrame->dragDropToIcon(wdSrc->m_id,wd->m_id,pTBsrc,pTBdest);
	};

	static void s_drag_drop_toolbar(GtkWidget  * /*widget*/,
									GdkDragContext     *context,
									gint /*x*/, gint /*y*/, 
									guint /*time*/, gpointer pTB)
	{
		GtkWidget * src = gtk_drag_get_source_widget(context);
		_wd * wdSrc = static_cast<_wd *>(g_object_get_data(G_OBJECT(src),"wd_pointer"));

		XAP_Frame * pFrame = static_cast<XAP_Frame *>(wdSrc->m_pUnixToolbar->getFrame());
		EV_Toolbar * pTBsrc = static_cast<EV_Toolbar *>(wdSrc->m_pUnixToolbar);
		EV_Toolbar * pTBdest = static_cast<EV_Toolbar *>(pTB);
		pFrame->dragDropToTB(wdSrc->m_id,pTBsrc,pTBdest);
	};

	/*!
	 * Only accept numeric input in the toolbar's font size combo.
	 */
	static void s_insert_text_cb (GtkEditable *editable,
								  gchar       *new_text,
								  gint         new_text_length,
								  gint        /* *position */ , 
								  gpointer    /* data */)
	{
		gchar		*iter;
		gunichar	 c;

		iter = new_text;
		while (iter < (new_text + new_text_length)) {
			c = g_utf8_get_char (iter);
			if (!g_unichar_isdigit (c)) {
				g_signal_stop_emission_by_name (G_OBJECT (editable), "insert-text");
				return;
			}
			iter = g_utf8_next_char (iter);
		}
	}

	/*!
	 * Apply font size upon <return>
	 */
	static gboolean	s_key_press_event_cb (GtkWidget   *widget,
	                                      GdkEventKey *event,
	                                      _wd         *wd)
	{
		GtkComboBox *combo;

		if (event->keyval == GDK_KEY_Return) {
			combo = GTK_COMBO_BOX (gtk_widget_get_parent (widget));
			s_combo_apply_changes (combo, wd);
		}

		return FALSE;
	}

	/*!
	 * Apply changes after editing of the font size is done.
	 */
	static gboolean	s_focus_out_event_cb (GtkWidget     *widget,
										  GdkEventFocus * /*event*/,
										  _wd           *wd)
	{

		GtkComboBox *combo;
		combo = GTK_COMBO_BOX (gtk_widget_get_parent (widget));
		s_combo_apply_changes (combo, wd);

		return FALSE;
	}

	static void s_font_prelight(GtkComboBox * combo, const gchar *text, _wd * wd)
	{
		GtkWidget 	*widget;
		gint 		 x;
		gint 		 y;

		if (wd && 
			wd->m_pUnixToolbar &&
			!wd->m_pUnixToolbar->m_pFontPreview) {

			widget = GTK_WIDGET(combo);
			GtkAllocation alloc;
			gtk_widget_get_allocation(widget, &alloc);
			gdk_window_get_origin(gtk_widget_get_window(widget), &x,&y);
			if (wd->m_pUnixToolbar->m_pFontPreviewPositionX > -1) {
				x = wd->m_pUnixToolbar->m_pFontPreviewPositionX;
			}
			else {
				x += alloc.x + alloc.width;
			}
			y += alloc.y + alloc.height;
			XAP_Frame * pFrame = static_cast<XAP_Frame *>(wd->m_pUnixToolbar->getFrame());
			wd->m_pUnixToolbar->m_pFontPreview = new XAP_UnixFontPreview(pFrame, x, y);
			UT_DEBUGMSG(("ev_UnixToolbar - building new FontPreview %p \n",wd->m_pUnixToolbar));
		}

		wd->m_pUnixToolbar->m_pFontPreview->setFontFamily(text);
		wd->m_pUnixToolbar->m_pFontPreview->setText(text);
		wd->m_pUnixToolbar->m_pFontPreview->draw();
	};

	static void s_font_popup_opened(GtkComboBox * /*combo*/, 
									GdkRectangle *position, _wd * wd)
	{
		if (wd && 
			wd->m_pUnixToolbar) {
				UT_DEBUGMSG(("ev_UnixToolbar - position \n"));
				// TODO check if within screen
				wd->m_pUnixToolbar->m_pFontPreviewPositionX = position->x /*+ position->width*/;
		}
	};

	static void s_font_popup_closed(GtkComboBox * /*combo*/, _wd * wd)
	{
		if (wd && 
			wd->m_pUnixToolbar &&
			wd->m_pUnixToolbar->m_pFontPreview) {
				UT_DEBUGMSG(("ev_UnixToolbar - deleting FontPreview %p \n",wd->m_pUnixToolbar));
			    delete wd->m_pUnixToolbar->m_pFontPreview;
				wd->m_pUnixToolbar->m_pFontPreview = NULL;
				wd->m_pUnixToolbar->m_pFontPreviewPositionX = -1;
		}
	};

	static void s_combo_changed(GtkComboBox * combo, _wd * wd)
	{
		UT_return_if_fail(wd);

		// only act if the widget has been shown and embedded in the toolbar
		if (!wd->m_widget || wd->m_blockSignal) {
			return;
		}

		if (wd->m_id == AP_TOOLBAR_ID_FMT_SIZE) {
			// no updates of the font size while the entry is being edited
			GtkWidget *entry;
			entry = gtk_bin_get_child (GTK_BIN(combo));
			if (gtk_widget_has_focus(entry)) {
				return;
			}
		}

		s_combo_apply_changes (combo, wd);
	}

	/*!
	 * Actually apply changes after a combo has been frobbed.
	 * This is not meant to be used as a signal handler, but rather to 
	 * implement common functionality after the decision has been made 
	 * whether to apply or not.
 	 */
	static void s_combo_apply_changes(GtkComboBox * combo, _wd * wd)
	{
		const char *text;
		// TODO Rob: move this into ev_UnixFontCombo
		gchar *buffer = NULL;
		GtkTreeModel *model = gtk_combo_box_get_model (combo);
		if (GTK_IS_TREE_MODEL_SORT (model)) {

			GtkTreeIter sort_iter;
			gtk_combo_box_get_active_iter (combo, &sort_iter);

			GtkTreeIter iter;		
			gtk_tree_model_sort_convert_iter_to_child_iter (GTK_TREE_MODEL_SORT (model), &iter, &sort_iter);

			GtkTreeModel *store = gtk_tree_model_sort_get_model (GTK_TREE_MODEL_SORT (model));
			gtk_tree_model_get (store, &iter, 0, &buffer, -1);
		} else {
			buffer = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(combo));
		}

		if (wd->m_id == AP_TOOLBAR_ID_FMT_FONT) {
			const gchar *font;
			font = XAP_EncodingManager::fontsizes_mapping.lookupByTarget(buffer);
			if (font) {
				g_free (buffer);
				buffer = g_strdup (font);
			}
			if (wd->m_pUnixToolbar->m_pFontPreview) {
				UT_DEBUGMSG(("ev_UnixToolbar - deleting FontPreview %p \n",wd->m_pUnixToolbar));
			    delete wd->m_pUnixToolbar->m_pFontPreview;
				wd->m_pUnixToolbar->m_pFontPreview = NULL;
				wd->m_pUnixToolbar->m_pFontPreviewPositionX = -1;
			}
		}

		if (wd->m_id == AP_TOOLBAR_ID_FMT_STYLE)
			text = pt_PieceTable::s_getUnlocalisedStyleName(buffer);
		else
			text = buffer;

		UT_UCS4String ucsText(text);
		wd->m_pUnixToolbar->toolbarEvent(wd, ucsText.ucs4_str(), ucsText.length());
		g_free (buffer);
	}

	EV_UnixToolbar *	m_pUnixToolbar;
	XAP_Toolbar_Id		m_id;
	GtkWidget *			m_widget;
	bool				m_blockSignal;
	gulong				m_handlerId;
};

static void
s_fore_color_changed (GOComboColor 	* /*cc*/, 
					  GOColor 		 color,
					  gboolean 		 /*custom*/, 
					  gboolean 		 /*by_user*/, 
					  gboolean 		 /*is_default*/, 
					  _wd 			*wd)
{
	UT_UTF8String str;

	UT_return_if_fail (wd);

	str = UT_UTF8String_sprintf ("%02x%02x%02x", 
								 UINT_RGBA_R (color),
								 UINT_RGBA_G (color),
								 UINT_RGBA_B (color));
	wd->m_pUnixToolbar->toolbarEvent(wd, str.ucs4_str().ucs4_str(), str.size());
}

static void
s_back_color_changed (GOComboColor 	* /*cc*/, 
					  GOColor 		 color,
					  gboolean 		 /*custom*/, 
					  gboolean 		 /*by_user*/, 
					  gboolean 		 is_default, 
					  _wd 			*wd)
{
	UT_UTF8String str;

	UT_return_if_fail (wd);

	if (is_default) {
		str = "transparent";
	} else {
		str = UT_UTF8String_sprintf ("%02x%02x%02x", 
								 UINT_RGBA_R (color),
								 UINT_RGBA_G (color),
								 UINT_RGBA_B (color));
	}

	wd->m_pUnixToolbar->toolbarEvent(wd, str.ucs4_str().ucs4_str(), str.size());
}

static void s_proxy_activated(GtkMenuItem * item, _wd * wd)
{
		const gchar * editMethod =
			(const gchar *)g_object_get_data(G_OBJECT(item),
											 "abi_action");
		XAP_UnixApp * pUnixApp = wd->m_pUnixToolbar->getApp();
		const EV_EditMethodContainer * pEMC = pUnixApp->getEditMethodContainer();
		UT_return_if_fail(pEMC);
		EV_EditMethod * pEM = NULL;

		AV_View * pView = wd->m_pUnixToolbar->getFrame()->getCurrentView();
		pEM = pEMC->findEditMethodByName(editMethod);
		wd->m_pUnixToolbar->invokeToolbarMethod(pView, pEM, NULL, 0);
}

EV_UnixToolbar::EV_UnixToolbar(XAP_UnixApp 	*pUnixApp, 
							   XAP_Frame 	*pFrame, 
							   const char 	*szToolbarLayoutName,
							   const char 	*szToolbarLabelSetName)
  : EV_Toolbar(pUnixApp->getEditMethodContainer(),
			   szToolbarLayoutName,
			   szToolbarLabelSetName), 
	m_pFontPreview(NULL),
	m_pFontPreviewPositionX(-1),
	m_pUnixApp(pUnixApp),
	m_pFrame(pFrame),
	m_pViewListener(NULL),
	m_eEvent(NULL),
	m_wToolbar(NULL),
	m_wHSizeGroup(NULL),
	m_wVSizeGroup(NULL)
{}

EV_UnixToolbar::~EV_UnixToolbar(void)
{
	UT_VECTOR_PURGEALL(_wd *,m_vecToolbarWidgets);
	if(m_wVSizeGroup) {
		g_object_unref(m_wVSizeGroup);
	}
	_releaseListener();
}

GtkBox* EV_UnixToolbar::_getContainer()
{
	return GTK_BOX(static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getVBoxWidget());
}

bool EV_UnixToolbar::toolbarEvent(_wd 				* wd,
								  const UT_UCSChar 	* pData,
								  UT_uint32 		  dataLength)

{
	// user selected something from this toolbar.
	// invoke the appropriate function.
	// return true iff handled.

	XAP_Toolbar_Id id = wd->m_id;

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pUnixApp->getToolbarActionSet();
	UT_return_val_if_fail(pToolbarActionSet, false);

	const EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
	UT_ASSERT(pAction);

	AV_View * pView = m_pFrame->getCurrentView();

	// make sure we ignore presses on "down" group buttons
	if (pAction->getItemType() == EV_TBIT_GroupButton)
	{
		const char * szState = 0;
		EV_Toolbar_ItemState tis = pAction->getToolbarItemState(pView,&szState);

		if (EV_TIS_ShouldBeToggled(tis))
		{
			// if this assert fires, you got a click while the button is down
			// if your widget set won't let you prevent this, handle it here

			UT_ASSERT(wd && wd->m_widget);
			
			// Block the signal, throw the button back up/down
			bool wasBlocked = wd->m_blockSignal;
			wd->m_blockSignal = true;
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(wd->m_widget),
						     !gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(wd->m_widget)));
			wd->m_blockSignal = wasBlocked;

			// can safely ignore this event
			return true;
		}
	}

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return false;

	const EV_EditMethodContainer * pEMC = m_pUnixApp->getEditMethodContainer();
	UT_return_val_if_fail(pEMC, false);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	invokeToolbarMethod(pView,pEM,pData,dataLength);
	return true;
}


/*!
 * This method destroys the container widget here and returns the position in
 * the overall vbox container.
 */
UT_sint32 EV_UnixToolbar::destroy(void)
{
	GtkContainer * wBox = GTK_CONTAINER(_getContainer());
	UT_sint32  pos = 0;
//
// Code gratutiously stolen from gtkbox.c
//
	GList *list = NULL;
	bool bFound = false;
	for( list = gtk_container_get_children(wBox); !bFound && list; list = list->next)
	{
		if(GTK_WIDGET (list->data) == m_wToolbar)
		{
			bFound = true;
			break;
		}
		pos++;
	}
	UT_ASSERT(bFound);
	if(!bFound)
	{
		pos = -1;
	}
//
// Now remove the view listener
//
	AV_View * pView = getFrame()->getCurrentView();
	pView->removeListener(m_lid);
	_releaseListener();
//
// Finally destroy the old toolbar widget
//
	gtk_widget_destroy(m_wToolbar);
	return pos;
}

/*!
 * This method rebuilds the toolbar and places it in the position it previously
 * occupied.
 */
void EV_UnixToolbar::rebuildToolbar(UT_sint32 oldpos)
{
  //
  // Build the toolbar, place it in a handlebox at an arbitary place on the 
  // the frame.
  //
    synthesize();
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
    UT_UNUSED(oldpos);
#else
	GtkBox * wBox = _getContainer();
	gtk_box_reorder_child(wBox, m_wToolbar, oldpos);
//
// bind  view listener
//
	AV_View * pView = getFrame()->getCurrentView();
	bindListenerToView(pView);
#endif
}

static void setDragIcon(GtkWidget * wwd, GtkImage * img)
{
  if (GTK_IMAGE_PIXBUF == gtk_image_get_storage_type(img))
    {
      GdkPixbuf * pixbuf = gtk_image_get_pixbuf ( img ) ;
      gtk_drag_source_set_icon_pixbuf ( wwd, pixbuf ) ;
    }
  else if (GTK_IMAGE_STOCK == gtk_image_get_storage_type(img))
    {
#if 0
      gchar * stk = NULL ;
      GtkIconSize icn_sz ;
      
      // TODO: this doesn't work, possibly a GTK2 bug...
      gtk_image_get_stock( img, &stk, &icn_sz ) ;
      gtk_drag_source_set_icon_stock ( wwd, stk ) ;
#endif
    }
}

/*
* get toolbar button appearance from the preferences
*/
GtkToolbarStyle EV_UnixToolbar::getStyle(void)
{
	const gchar * szValue = NULL;
	m_pUnixApp->getPrefsValue(XAP_PREF_KEY_ToolbarAppearance,&szValue);
	UT_ASSERT((szValue) && (*szValue));
	
	GtkToolbarStyle style = GTK_TOOLBAR_ICONS;
	if (g_ascii_strcasecmp(szValue,"text")==0)
		style = GTK_TOOLBAR_TEXT;
	else if (g_ascii_strcasecmp(szValue,"both")==0)
		style = GTK_TOOLBAR_BOTH;

	return style;	
}

bool EV_UnixToolbar::synthesize(void)
{
	// create a GTK toolbar from the info provided.
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pUnixApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	XAP_Toolbar_ControlFactory * pFactory = m_pUnixApp->getControlFactory();
	UT_ASSERT(pFactory);

	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	m_wToolbar = gtk_toolbar_new();
	UT_ASSERT(m_wToolbar);

	GtkToolbarStyle style = getStyle();
	gtk_toolbar_set_style(GTK_TOOLBAR(m_wToolbar), style );

//	gtk_toolbar_set_tooltips(GTK_TOOLBAR(m_wToolbar), TRUE);
	gtk_toolbar_set_show_arrow(GTK_TOOLBAR(m_wToolbar), TRUE);

	//m_wHSizeGroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	m_wVSizeGroup = gtk_size_group_new(GTK_SIZE_GROUP_VERTICAL);

	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_continue_if_fail(pLayoutItem);

		XAP_Toolbar_Id id = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		UT_ASSERT(pAction);
		EV_Toolbar_Label * pLabel = m_pToolbarLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		const char * szToolTip = pLabel->getToolTip();
		if (!szToolTip || !*szToolTip)
			szToolTip = pLabel->getStatusMsg();		

		switch (pLayoutItem->getToolbarLayoutFlags())
		{
		case EV_TLF_Normal:
		{
			_wd * wd = new _wd(this,id);
			UT_ASSERT(wd);

			switch (pAction->getItemType())
			{
			case EV_TBIT_PushButton:
			{
				UT_ASSERT(g_ascii_strcasecmp(pLabel->getIconName(),"NoIcon")!=0);
				if(pAction->getToolbarId() != AP_TOOLBAR_ID_INSERT_TABLE)
				{
					wd->m_widget = toolbar_append_button (GTK_TOOLBAR (m_wToolbar), pLabel->getIconName(),
														  pLabel->getToolbarLabel(), szToolTip,
														  (GCallback) _wd::s_callback, (gpointer) wd, 
														  &(wd->m_handlerId));
				}
				else
				{
//
// Hardwire the cool insert table widget onto the toolbar
//
					GtkWidget * abi_table = abi_table_new();
					const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
                    std::string sTable;
					pSS->getValueUTF8(XAP_STRING_ID_TB_Rows_x_Cols_Table,sTable);
                    std::string sCancel;
					pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel,sCancel);

					abi_table_set_labels(ABITABLE_WIDGET(abi_table),sTable.c_str(),sCancel.c_str());
					gtk_widget_show(abi_table);
					UT_DEBUGMSG(("SEVIOR: Made insert table widget \n"));
					wd->m_handlerId = g_signal_connect(abi_table, "selected",
													   G_CALLBACK (_wd::s_new_table), 
													   static_cast<gpointer>(wd));

					UT_DEBUGMSG(("SEVIOR: Made connected to callback \n"));
                    std::string s;
					pSS->getValueUTF8(XAP_STRING_ID_TB_InsertNewTable, s);
					toolbar_append_item (GTK_TOOLBAR (m_wToolbar), abi_table, 
										 s.c_str(), TRUE);
					gtk_widget_show_all(abi_table);
					wd->m_widget = abi_table;
				}
				GtkWidget * wwd = wd->m_widget;
				g_object_set_data(G_OBJECT(wwd),
									"wd_pointer",
									wd);
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
				gtk_drag_source_set(wwd,GDK_BUTTON3_MASK,
									s_AbiTBTargets,1,
									GDK_ACTION_COPY);
				GtkImage * dragimage
					= (GtkImage*)gtk_image_new_from_icon_name(ABIWORD_INSERT_TABLE,
															  GTK_ICON_SIZE_DND);
				g_object_ref_sink(dragimage);
				setDragIcon(wwd, dragimage);  // does not take ownership
				g_object_unref(dragimage);
				gtk_drag_dest_set(wwd, GTK_DEST_DEFAULT_ALL,
									s_AbiTBTargets,1,
									GDK_ACTION_COPY);
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_begin",G_CALLBACK(_wd::s_drag_begin), wd);
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_drop",G_CALLBACK(_wd::s_drag_drop), wd);
#endif				
			}
			break;

			case EV_TBIT_ToggleButton:
			case EV_TBIT_GroupButton:
				{
					UT_ASSERT(g_ascii_strcasecmp(pLabel->getIconName(),"NoIcon")!=0);

					gboolean bShow = TRUE;
					wd->m_widget = toolbar_append_toggle (GTK_TOOLBAR (m_wToolbar), pLabel->getIconName(),
														  pLabel->getToolbarLabel(), szToolTip,
														  (GCallback) _wd::s_callback, (gpointer) wd, 
														  bShow, &(wd->m_handlerId));
					//
					// Add in a right drag method
					//
				GtkWidget * wwd = wd->m_widget;
				g_object_set_data(G_OBJECT(wwd),
									"wd_pointer",
									wd);
				gtk_drag_source_set(wwd,GDK_BUTTON3_MASK,
									s_AbiTBTargets,1,
									GDK_ACTION_COPY);
				gchar *stock_id = abi_stock_from_toolbar_id(pLabel->getIconName());
				GtkImage *dragimage
					= (GtkImage*)gtk_image_new_from_icon_name(stock_id,
															  GTK_ICON_SIZE_DND);
				g_object_ref_sink(dragimage);
				setDragIcon(wwd, dragimage); // does not take dragimage ownership
				g_object_unref(dragimage);
				g_free (stock_id);
				stock_id = NULL;
				gtk_drag_dest_set(wwd,static_cast<GtkDestDefaults>(GTK_DEST_DEFAULT_ALL),
									s_AbiTBTargets,1,
									GDK_ACTION_COPY);
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_begin",G_CALLBACK(_wd::s_drag_begin), wd);
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_drop",G_CALLBACK(_wd::s_drag_drop), wd);
				}
				break;

			case EV_TBIT_EditText:
				break;

			case EV_TBIT_DropDown:
				break;

			case EV_TBIT_ComboBox:
			{
				EV_Toolbar_Control * pControl = pFactory->getControl(this, id);
				UT_ASSERT(pControl);

				GtkWidget *combo;
				const gchar *proxy_action_name;
				if (wd->m_id == AP_TOOLBAR_ID_FMT_SIZE) {
					combo = gtk_combo_box_text_new_with_entry();
					GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo)));
					g_object_set (G_OBJECT(entry), "can-focus", TRUE, NULL);
					gtk_entry_set_width_chars (entry, 4);
					g_signal_connect (G_OBJECT (entry), "insert-text", G_CALLBACK (_wd::s_insert_text_cb), NULL);
					g_signal_connect (G_OBJECT (entry), "focus-out-event", G_CALLBACK (_wd::s_focus_out_event_cb), (gpointer) wd);
					g_signal_connect (G_OBJECT (entry), "key-press-event", G_CALLBACK (_wd::s_key_press_event_cb), (gpointer) wd);
					// same size for font and font-size combos
					// gtk_size_group_add_widget (m_wHSizeGroup, combo);
					proxy_action_name = "dlgFont";
				}
				else if (wd->m_id == AP_TOOLBAR_ID_FMT_FONT) {
					gulong handler_id;
					combo = abi_font_combo_new ();
					gtk_widget_set_name (combo, "AbiFontCombo");
					handler_id = g_signal_connect (G_OBJECT(combo), "prelight", 
												    G_CALLBACK(_wd::s_font_prelight), 
												    wd);
					g_signal_connect (G_OBJECT(combo), "popup-opened", 
									  G_CALLBACK(_wd::s_font_popup_opened), 
									  wd);
					g_signal_connect (G_OBJECT(combo), "popup-closed", 
									  G_CALLBACK(_wd::s_font_popup_closed), 
									  wd);
					g_object_set_data (G_OBJECT (combo), PROP_HANDLER_ID,
									   GUINT_TO_POINTER(handler_id));
					// same size for font and font-size combos
					// gtk_widget_set_size_request (combo, 0, -1);
					// gtk_size_group_add_widget (m_wHSizeGroup, combo);
					proxy_action_name = "dlgFont";
				}
				else if (wd->m_id == AP_TOOLBAR_ID_ZOOM) {
					combo = gtk_combo_box_text_new();
					gtk_widget_set_name (combo, "AbiZoomCombo");
					proxy_action_name = "dlgZoom";
				}
				else if (wd->m_id == AP_TOOLBAR_ID_FMT_STYLE) {
					combo = gtk_combo_box_text_new();
					gtk_widget_set_name (combo, "AbiStyleCombo");
					proxy_action_name = "dlgStyle";
				}
				else {
					g_assert_not_reached();
				}

				wd->m_handlerId = g_signal_connect (G_OBJECT(combo), "changed", 
													G_CALLBACK(_wd::s_combo_changed), 
													wd);

				// populate it
				if (pControl) {
					pControl->populate();
					const UT_GenericVector<const char*> * v = pControl->getContents();
					UT_ASSERT(v);
					gint items = v->getItemCount();
					if (ABI_IS_FONT_COMBO (combo)) {
						const gchar **fonts = g_new0 (const gchar *, items + 1);
						for (gint m=0; m < items; m++) {
							fonts[m] = v->getNthItem(m);
						}						
						abi_font_combo_set_fonts (ABI_FONT_COMBO (combo), fonts);
						g_free (fonts); fonts = NULL;
					}
					else {
						for (gint m=0; m < items; m++) {
							const char * sz = v->getNthItem(m);
							std::string sLoc;
							if (wd->m_id == AP_TOOLBAR_ID_FMT_STYLE)
							{
								pt_PieceTable::s_getLocalisedStyleName(sz, sLoc);
								sz = sLoc.c_str();
							}
							gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), sz);
						}
					}
				}

				gtk_size_group_add_widget (m_wVSizeGroup, combo);
				gtk_widget_set_valign(combo, GTK_ALIGN_CENTER);
				gtk_widget_show(combo);
				toolbar_append_item_with_proxy (GTK_TOOLBAR (m_wToolbar), combo,
												szToolTip, TRUE,
												proxy_action_name, wd);
				wd->m_widget = combo;
				// for now, we never repopulate, so can just toss it
				DELETEP(pControl);
			}
			break;

			case EV_TBIT_ColorFore:
			case EV_TBIT_ColorBack:
			{
				GdkPixbuf 		*pixbuf;
				GtkWidget		*combo;
				GOColorGroup 	*cg;
				const gchar		*action_name;

				UT_ASSERT (g_ascii_strcasecmp(pLabel->getIconName(),"NoIcon") != 0);

			    if (pAction->getItemType() == EV_TBIT_ColorFore) {
					const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
					std::string sClear;
					pSS->getValueUTF8(XAP_STRING_ID_TB_ClearForeground,sClear);

					action_name = "dlgColorPickerFore";
					pixbuf
						= gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
												   ABIWORD_COLOR_FORE,
												   GTK_ICON_SIZE_LARGE_TOOLBAR,
												   GTK_ICON_LOOKUP_USE_BUILTIN,
												   NULL);
					cg = go_color_group_fetch ("fore_color_group", m_wToolbar);
					combo = go_combo_color_new (pixbuf, sClear.c_str(), 0, cg);

					wd->m_widget = combo;
					g_signal_connect (G_OBJECT (combo), "color-changed",
									  G_CALLBACK (s_fore_color_changed), wd);
				}
				else {
					const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
					std::string sClear;
					pSS->getValueUTF8(XAP_STRING_ID_TB_ClearBackground,sClear);

					action_name = "dlgColorPickerBack";
					pixbuf
						= gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
												   ABIWORD_COLOR_BACK,
												   GTK_ICON_SIZE_LARGE_TOOLBAR,
												   GTK_ICON_LOOKUP_USE_BUILTIN,
												   NULL);
					cg = go_color_group_fetch ("back_color_group", m_wToolbar);
					combo = go_combo_color_new (pixbuf, sClear.c_str(), 0, cg);

					wd->m_widget = combo;
					g_signal_connect (G_OBJECT (combo), "color-changed",
									  G_CALLBACK (s_back_color_changed), wd);
				}
				go_combo_box_set_relief (GO_COMBO_BOX (combo), GTK_RELIEF_NONE);
				go_combo_color_set_instant_apply (GO_COMBO_COLOR (combo), TRUE);
				if (pixbuf) {
					g_object_unref (G_OBJECT (pixbuf));
				}

				toolbar_append_item_with_proxy (GTK_TOOLBAR(m_wToolbar), combo,
												szToolTip,
												TRUE, action_name, wd);

				//
				// Add in a right drag method
				//
				GtkWidget * wwd = wd->m_widget;
				g_object_set_data(G_OBJECT(wwd),
								  "wd_pointer",
								  wd);
			}
			break;
				
			case EV_TBIT_StaticLabel:
				// TODO do these...
				break;
					
			case EV_TBIT_Spacer:
				break;

#ifdef ENABLE_MENUBUTTON
			case EV_TBIT_MenuButton:
			{
				GtkWidget * wMenu = NULL;
				EV_UnixMenuBar * pBar =
					dynamic_cast<EV_UnixMenuBar*>(m_pFrame->getMainMenu());

				UT_ASSERT_HARMLESS(pBar);
				if (pBar)
				{
					wMenu = pBar->getMenuBar();
				}
				
				wd->m_widget =
					toolbar_append_menubutton (GTK_TOOLBAR (m_wToolbar),
											   wMenu,
											   pLabel->getIconName(),
											   pLabel->getToolbarLabel(),
											   szToolTip,
											   NULL, 
											   NULL,
											   NULL, 
											   NULL);
				
				GtkWidget * wwd = wd->m_widget;
				
				g_object_set_data(G_OBJECT(wwd), "wd_pointer", wd);

			}
			break;
#endif
			case EV_TBIT_BOGUS:
			default:
				break;
			}
		// add item after bindings to catch widget returned to us
			m_vecToolbarWidgets.addItem(wd);
		}
		break;
			
		case EV_TLF_Spacer:
		{
			// Append to the vector even if spacer, to sync up with refresh
			// which expects each item in the layout to have a place in the
			// vector.
			_wd * wd = new _wd(this,id);
			UT_ASSERT(wd);
			m_vecToolbarWidgets.addItem(wd);

			toolbar_append_separator (GTK_TOOLBAR (m_wToolbar));
			break;
		}
		
		default:
			UT_ASSERT(0);
		}
	}

#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
	GtkWidget * wTLW = static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getTopLevelWindow();
	hildon_window_add_toolbar (HILDON_WINDOW (wTLW), GTK_TOOLBAR (m_wToolbar));
	gtk_widget_show_all(m_wToolbar);
	gtk_widget_show_all(wTLW);
#else
	GtkBox * wBox = _getContainer();

	// show the complete thing
	gtk_widget_show(m_wToolbar);

	// put it in the vbox
	gtk_box_pack_start(wBox, m_wToolbar, FALSE, FALSE, 0);

	setDetachable(getDetachable());
#endif /* HAVE_HILDON */

	
	return true;
}

void EV_UnixToolbar::_releaseListener(void)
{
	if (!m_pViewListener)
		return;
	DELETEP(m_pViewListener);
	m_pViewListener = 0;
	m_lid = 0;
}
	
bool EV_UnixToolbar::bindListenerToView(AV_View * pView)
{
	_releaseListener();
	
	m_pViewListener =
		new EV_UnixToolbar_ViewListener(this,pView);
	UT_ASSERT(m_pViewListener);

	bool bResult = pView->addListener(static_cast<AV_Listener *>(m_pViewListener),&m_lid);
	UT_ASSERT(bResult);
	m_pViewListener->setLID(m_lid);
	if(pView->isDocumentPresent())
	{
		refreshToolbar(pView, AV_CHG_ALL);
	}
	return bResult;
}

bool EV_UnixToolbar::refreshToolbar(AV_View * pView, AV_ChangeMask mask)
{
	// make the toolbar reflect the current state of the document
	// at the current insertion point or selection.

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pUnixApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_continue_if_fail(pLayoutItem);

		XAP_Toolbar_Id id = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		UT_continue_if_fail(pAction);

		AV_ChangeMask maskOfInterest = pAction->getChangeMaskOfInterest();
		if ((maskOfInterest & mask) == 0)					// if this item doesn't care about
			continue;										// changes of this type, skip it...

		switch (pLayoutItem->getToolbarLayoutFlags())
		{
		case EV_TLF_Normal:
			{
				const char * szState = 0;
				EV_Toolbar_ItemState tis = pAction->getToolbarItemState(pView,&szState);

                if( tis & EV_TIS_Hidden )
                    tis = (EV_Toolbar_ItemState)(tis | EV_TIS_Gray);
                
				switch (pAction->getItemType())
				{
				case EV_TBIT_PushButton:
				{
					bool bGrayed = EV_TIS_ShouldBeGray(tis);

					_wd * wd = m_vecToolbarWidgets.getNthItem(k);
					UT_ASSERT(wd && wd->m_widget);
					gtk_widget_set_sensitive(wd->m_widget, !bGrayed);     					
					gtk_widget_set_visible(wd->m_widget, !EV_TIS_ShouldBeHidden(tis));
				}
				break;
			
				case EV_TBIT_ToggleButton:
				case EV_TBIT_GroupButton:
				{
					bool bGrayed = EV_TIS_ShouldBeGray(tis);
					bool bToggled = EV_TIS_ShouldBeToggled(tis);

					_wd * wd = m_vecToolbarWidgets.getNthItem(k);
					UT_ASSERT(wd && wd->m_widget);
					// Block the signal, throw the toggle event
					bool wasBlocked = wd->m_blockSignal;
					wd->m_blockSignal = true;
					gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON (wd->m_widget), bToggled);
					wd->m_blockSignal = wasBlocked;
						
					// Disable/enable toolbar item
					gtk_widget_set_sensitive(wd->m_widget, !bGrayed);				
				}
				break;

				case EV_TBIT_EditText:
					break;
				case EV_TBIT_DropDown:
					break;
				case EV_TBIT_ComboBox:
				{
					bool bGrayed = EV_TIS_ShouldBeGray(tis);
					
					_wd * wd = m_vecToolbarWidgets.getNthItem(k);
					UT_ASSERT(wd);
					GtkComboBox * combo = GTK_COMBO_BOX(wd->m_widget);
					UT_ASSERT(combo);
					// Disable/enable toolbar combo
					gtk_widget_set_sensitive(GTK_WIDGET(combo), !bGrayed);

					// Block the signal, set the contents
					bool wasBlocked = wd->m_blockSignal;
					wd->m_blockSignal = true;
					if (!szState) {
						gtk_combo_box_set_active (combo, -1);
					}
					else if (wd->m_id == AP_TOOLBAR_ID_FMT_SIZE) {
						const char * fsz = XAP_EncodingManager::fontsizes_mapping.lookupBySource(szState);
						gboolean ret = FALSE;
						if (fsz) {
							ret = combo_box_set_active_text(combo, fsz, wd->m_handlerId);
						}
						if (!ret) {
							gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo))), 
											   szState);
						}
					}
					else if (wd->m_id == AP_TOOLBAR_ID_FMT_STYLE) {
#define BUILTIN_INDEX "builtin-index"
						std::string sLoc;
						pt_PieceTable::s_getLocalisedStyleName(szState, sLoc);
						szState = sLoc.c_str();
						gint idx = GPOINTER_TO_INT(g_object_steal_data(G_OBJECT(combo), BUILTIN_INDEX));
						if (idx > 0) {
							gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(combo), idx);
						}
						gboolean ret = combo_box_set_active_text(combo, szState, wd->m_handlerId);
						if (!ret) {
							// try again
							repopulateStyles();
							ret = combo_box_set_active_text(combo, szState, wd->m_handlerId);
							if (!ret) {
								// still not, hmm, this seems to be an internal style
								// we'll just display it and remove the entry when the carent moves away
								gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(combo), szState);
								combo_box_set_active_text(combo, szState, wd->m_handlerId);
								g_object_set_data (G_OBJECT (combo), BUILTIN_INDEX, 
												   GINT_TO_POINTER(gtk_combo_box_get_active(combo)));
							}
						}
#undef BUILTIN_INDEX
					}
					else {
						combo_box_set_active_text(combo, szState, wd->m_handlerId);
					} 
					if (wd->m_id == AP_TOOLBAR_ID_FMT_FONT) {
						m_pFrame->setStatusMessage(szState);
						if (wd->m_pUnixToolbar->m_pFontPreview) {
							UT_DEBUGMSG(("ev_UnixToolbar - deleting FontPreview %p \n",wd->m_pUnixToolbar));
						    delete wd->m_pUnixToolbar->m_pFontPreview;
							wd->m_pUnixToolbar->m_pFontPreview = NULL;
							wd->m_pUnixToolbar->m_pFontPreviewPositionX = 0;
						}
					}
					wd->m_blockSignal = wasBlocked;					
				}
				break;

                case EV_TBIT_ColorFore:
                case EV_TBIT_ColorBack:
                {
					bool bGrayed = EV_TIS_ShouldBeGray(tis);
					
					_wd * wd = m_vecToolbarWidgets.getNthItem(k);
					UT_ASSERT(wd);
					UT_ASSERT(wd->m_widget);
					gtk_widget_set_sensitive(GTK_WIDGET(wd->m_widget), !bGrayed);   // Disable/enable toolbar item
                }
				break;

				case EV_TBIT_StaticLabel:
					break;
				case EV_TBIT_Spacer:
					break;
				case EV_TBIT_BOGUS:
					break;
				default:
					UT_ASSERT(0);
					break;
				}
			}
			break;
			
		case EV_TLF_Spacer:
			break;
			
		default:
			UT_ASSERT(0);
			break;
		}
	}

	return true;
}

XAP_UnixApp * EV_UnixToolbar::getApp(void)
{
	return m_pUnixApp;
}

XAP_Frame * EV_UnixToolbar::getFrame(void)
{
	return m_pFrame;
}

void EV_UnixToolbar::show(void)
{
	if (m_wToolbar) {
		gtk_widget_show (m_wToolbar);
	}
}

void EV_UnixToolbar::hide(void)
{

	if (m_wToolbar) {
		gtk_widget_hide (m_wToolbar);
	}
	EV_Toolbar::hide();
}

/*!
 * This method examines the current document and repopulates the Styles
 * Combo box with what is in the document. It returns false if no styles 
 * combo box was found. True if it all worked.
 */
bool EV_UnixToolbar::repopulateStyles(void)
{
//
// First off find the Styles combobox in a toolbar somewhere
//
	UT_uint32 count = m_pToolbarLayout->getLayoutItemCount();
	UT_uint32 i =0;
	EV_Toolbar_LayoutItem * pLayoutItem = NULL;
	XAP_Toolbar_Id id = 0;
	_wd * wd = NULL;
	for(i=0; i < count; i++)
	{
		pLayoutItem = m_pToolbarLayout->getLayoutItem(i);
		id = pLayoutItem->getToolbarId();
		wd = m_vecToolbarWidgets.getNthItem(i);
		if(id == AP_TOOLBAR_ID_FMT_STYLE)
			break;
	}
	if(i>=count)
		return false;
//
// GOT IT!
//
	UT_ASSERT(wd->m_id == AP_TOOLBAR_ID_FMT_STYLE);
	XAP_Toolbar_ControlFactory * pFactory = m_pUnixApp->getControlFactory();
	UT_return_val_if_fail(pFactory, false);
	EV_Toolbar_Control * pControl = pFactory->getControl(this, id);
	AP_UnixToolbar_StyleCombo * pStyleC = static_cast<AP_UnixToolbar_StyleCombo *>(pControl);
	pStyleC->repopulate();
	GtkComboBox * combo = GTK_COMBO_BOX(wd->m_widget);
	GtkTreeModel *model = gtk_combo_box_get_model(combo);
//
// Now the combo box has to be refilled from this
//						
	const UT_GenericVector<const char*> * v = pControl->getContents();
	UT_ASSERT(v);
//
// Now  we must remove and delete the old glist so we can attach the new
// list of styles to the combo box.
//
// Try this....
//
	bool wasBlocked = wd->m_blockSignal;
	wd->m_blockSignal = true; // block the signal, so we don't try to read the text entry while this is happening..
    gtk_list_store_clear (GTK_LIST_STORE (model));
	
//
// Now make a new one.
//
	gint items = v->getItemCount();

	GtkTreeIter iter;
	GtkListStore *list = gtk_list_store_new(1, G_TYPE_STRING);

	for (gint m=0; m < items; m++) {
		std::string sLoc;
		const char * sz = v->getNthItem(m);

		pt_PieceTable::s_getLocalisedStyleName(sz, sLoc);
		sz = sLoc.c_str();
		gtk_list_store_append(list, &iter);
		gtk_list_store_set(list, &iter, 0, sz, -1);
	}

	GtkTreeSortable *sort;
	sort = GTK_TREE_SORTABLE(list);
	gtk_tree_sortable_set_sort_column_id(sort, 0, GTK_SORT_ASCENDING);

	gboolean itering = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(list), &iter);

	while (itering)
	{
		gchar *entry;

		gtk_tree_model_get(GTK_TREE_MODEL(list), &iter, 0, &entry, -1);
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), entry);

		g_free(entry);

		itering = gtk_tree_model_iter_next(GTK_TREE_MODEL(list), &iter);
	}

	g_object_unref(G_OBJECT(list));    

	wd->m_blockSignal = wasBlocked;

//
// Don't need this anymore and we don't like memory leaks in abi
//
	delete pStyleC;
//
// I think we've finished!
//
	return true;
}
