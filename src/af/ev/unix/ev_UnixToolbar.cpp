/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

/*
 * Port to Maemo Development Platform
 * Author: INdT - Renato Araujo <renato.filho@indt.org.br>
 */

//for GtkCombo->GtkList
#undef GTK_DISABLE_DEPRECATED

#include <gtk/gtk.h>
#include <goffice/gtk/go-combo-box.h>
#include <goffice/gtk/go-combo-color.h>
#include <string.h>
#include <stdlib.h>
#include "ap_Features.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ev_UnixToolbar.h"
#include "xap_Types.h"
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
#include "gr_UnixGraphics.h"
#include "ut_string_class.h"

#ifdef HAVE_GNOME
#include <gnome.h>
#endif

// hack
#include "ap_Toolbar_Id.h"
// hack, icons are in wp
#include "../../../wp/ap/unix/ap_UnixStockIcons.h"

#ifdef HAVE_HILDON
#include "hildon-widgets/hildon-appview.h"
#endif

#define COMBO_BUF_LEN 256

static const GtkTargetEntry      s_AbiTBTargets[] = {{"abi-toolbars",0,0}};

/*!
 * Append a widget to the toolbar, 
 * wrap it in a GtkToolItem if it isn't one already.
 */
static GtkWidget *
toolbar_append_item (GtkToolbar *toolbar, 
					 GtkWidget  *widget,
					 const char *text,
					 const char *private_text)
{
	GtkToolItem *item;

	UT_ASSERT(GTK_IS_TOOLBAR (toolbar));
	UT_ASSERT(widget != NULL);

	if (GTK_IS_TOOL_ITEM (widget)) {
		item = GTK_TOOL_ITEM (widget);
	}
	else {
		item = gtk_tool_item_new ();
		gtk_container_add (GTK_CONTAINER (item), widget);
	}
	gtk_tool_item_set_tooltip (item, toolbar->tooltips, text, private_text);
	gtk_toolbar_insert (toolbar, item, -1);
	gtk_widget_show_all (GTK_WIDGET (item));

	return GTK_WIDGET (item);
}

/*!
 * Append a GtkToolButton to the toolbar.
 */
static GtkWidget *
toolbar_append_button (GtkToolbar 	*toolbar, 
					   const gchar	*icon_name, 
					   const gchar	*label, 
					   const gchar  *private_text, 
					   GCallback	 handler, 
					   gpointer		 data)
{
	GtkToolItem *item;
	gchar		*stock_id;

	stock_id = abi_stock_from_toolbar_id (icon_name);
	item = gtk_tool_button_new_from_stock (stock_id);
	g_free (stock_id);
	stock_id = NULL;
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (item), label);
	g_signal_connect (G_OBJECT (item), "clicked", handler, data);

	return (GtkWidget *) toolbar_append_item (toolbar, GTK_WIDGET (item), 
											  label, private_text);
}

/*!
 * Append a GtkToggleToolButton to the toolbar.
 */
static GtkWidget *
toolbar_append_toggle (GtkToolbar 	*toolbar, 
					   const gchar	*icon_name, 
					   const gchar	*label, 
					   const gchar  *private_text, 
					   GCallback	 handler, 
					   gpointer		 data)
{
	GtkToolItem *item;
	gchar		*stock_id;

	stock_id = abi_stock_from_toolbar_id (icon_name);
	item = gtk_toggle_tool_button_new_from_stock (stock_id);
	g_free (stock_id);
	stock_id = NULL;
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (item), label);
	g_signal_connect (G_OBJECT (item), "toggled", handler, data);

	return (GtkWidget *) toolbar_append_item (toolbar, GTK_WIDGET (item), 
											  label, private_text);
}

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

class _wd								// a private little class to help
{										// us remember all the widgets that
public:									// we create...
	_wd(EV_UnixToolbar * pUnixToolbar, XAP_Toolbar_Id id, GtkWidget * widget = NULL)
	{
		m_pUnixToolbar = pUnixToolbar;
		m_id = id;
		m_widget = widget;
		m_blockSignal = false;
		m_comboEntryBuffer[0] = 0;
	};
	
	~_wd(void)
	{
	};

	static void s_callback(GtkWidget * /* widget */, gpointer user_data)
	{
		// this is a static callback method and does not have a 'this' pointer.
		// map the user_data into an object and dispatch the event.
	
		_wd * wd = static_cast<_wd *>(user_data);
		UT_ASSERT(wd);
		GdkEvent * event = gtk_get_current_event();
		wd->m_pUnixToolbar->setCurrentEvent(event);
		if (!wd->m_blockSignal)
		{
			wd->m_pUnixToolbar->toolbarEvent(wd, 0, 0);
		}
	};

	static void s_new_table(GtkWidget *table, int rows, int cols, gpointer* user_data)
	{
		// this is a static callback method and does not have a 'this' pointer.
		// map the user_data into an object and dispatch the event.
	
		_wd * wd = reinterpret_cast<_wd *>(user_data);
		UT_ASSERT(wd);
		GdkEvent * event = gtk_get_current_event();
		wd->m_pUnixToolbar->setCurrentEvent(event);
		if (!wd->m_blockSignal && (rows > 0) && (cols > 0))
		{
			FV_View * pView = static_cast<FV_View *>(wd->m_pUnixToolbar->getFrame()->getCurrentView());
			pView->cmdInsertTable(rows,cols,NULL);
		}
	}

	static void s_drag_begin(GtkWidget  *widget,
							GdkDragContext     *context)
	{
		_wd * wd = static_cast<_wd *>(g_object_get_data(G_OBJECT(widget),"wd_pointer"));
		XAP_Frame * pFrame = static_cast<XAP_Frame *>(wd->m_pUnixToolbar->getFrame());
		EV_Toolbar * pTBsrc = static_cast<EV_Toolbar *>(wd->m_pUnixToolbar);
		pFrame->dragBegin(wd->m_id,pTBsrc);
	};


	static void s_drag_drop(GtkWidget  *widget,
							GdkDragContext     *context,
							gint x, gint y, guint time )
	{
		_wd * wd = static_cast<_wd *>(g_object_get_data(G_OBJECT(widget),"wd_pointer"));
		GtkWidget * src = gtk_drag_get_source_widget(context);
		_wd * wdSrc = static_cast<_wd *>(g_object_get_data(G_OBJECT(src),"wd_pointer"));
		
		XAP_Frame * pFrame = static_cast<XAP_Frame *>(wd->m_pUnixToolbar->getFrame());
		EV_Toolbar * pTBdest = static_cast<EV_Toolbar *>(wd->m_pUnixToolbar);
		EV_Toolbar * pTBsrc = static_cast<EV_Toolbar *>(wdSrc->m_pUnixToolbar);
		pFrame->dragDropToIcon(wdSrc->m_id,wd->m_id,pTBsrc,pTBdest);
	};

	static void s_drag_drop_toolbar(GtkWidget  *widget,
							GdkDragContext     *context,
							gint x, gint y, guint time, gpointer pTB)
	{
		GtkWidget * src = gtk_drag_get_source_widget(context);
		_wd * wdSrc = static_cast<_wd *>(g_object_get_data(G_OBJECT(src),"wd_pointer"));

		XAP_Frame * pFrame = static_cast<XAP_Frame *>(wdSrc->m_pUnixToolbar->getFrame());
		EV_Toolbar * pTBsrc = static_cast<EV_Toolbar *>(wdSrc->m_pUnixToolbar);
		EV_Toolbar * pTBdest = static_cast<EV_Toolbar *>(pTB);
		pFrame->dragDropToTB(wdSrc->m_id,pTBsrc,pTBdest);
	};

	static void s_drag_end(GtkWidget  *widget,
							GdkDragContext     *context)
	{
		_wd * wd = static_cast<_wd *>(g_object_get_data(G_OBJECT(widget),"wd_pointer"));

		XAP_Frame * pFrame = static_cast<XAP_Frame *>(wd->m_pUnixToolbar->getFrame());
		pFrame->dragEnd(wd->m_id);
	};

	static void s_combo_list_changed(GtkWidget* list, GtkWidget * widget, gpointer user_data)
	{
		s_combo_changed(widget, user_data);
	}
	
	// TODO: should this move out of wd?  It's convenient here; maybe I'll make
	// a microclass for combo boxes.
	static void s_combo_changed(GtkWidget * widget, gpointer user_data)
	{
		_wd * wd = static_cast<_wd *>(user_data);
		UT_ASSERT(wd);

		// only act if the widget has been shown and embedded in the toolbar
		if (wd->m_widget)
		{
			// if the popwin is still shown, this is a copy run and widget has a ->parent
			if (widget->parent)
			{
				// block is only honored here
				if (!wd->m_blockSignal)
				{
					const gchar * buffer = NULL;
					if (GTK_IS_ENTRY(widget))
						buffer = gtk_entry_get_text(GTK_ENTRY(widget));
					else if (GTK_IS_LIST_ITEM(widget))
					{
						GtkWidget* label = gtk_bin_get_child(GTK_BIN(widget));
						buffer = gtk_label_get_text(GTK_LABEL(label));
					}
					else {
						UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
						return;
					}

					// check if something actually _is_ changed... yeah yeah, another hack.. gtk sux sometimes
					if (!strncmp(wd->m_comboEntryBuffer, buffer, COMBO_BUF_LEN))
						return;
					
					UT_uint32 length = strlen(buffer);
					xxx_UT_DEBUGMSG(("LACHANCE: comboChanged, length: %d \n", length));
				        // LACHANCE: in gtk2, it seems as if the gtk_entry's text buffer length is set to 0
				        // when we move through combo elements/new combo elements are added. in this case, we 
				        // have to ignore the signal-- things will be set correctly momentarily. this seems
					// to work correctly, but it would be nice to find a more elegant solution
					// (P.S.: GtkCombo SUCKS, and will hopefully soon be deprecated. We should really be 
					// using GtkOptionMenu for non-changeable drop-down toolbar menus)
					if (length > 0) 
					{					
						UT_ASSERT(length < COMBO_BUF_LEN);
						memset(wd->m_comboEntryBuffer, 0, COMBO_BUF_LEN);		       
						strncpy(wd->m_comboEntryBuffer, buffer, COMBO_BUF_LEN-1);

						if (wd->m_id == AP_TOOLBAR_ID_FMT_FONT && wd->m_pUnixToolbar->m_pFontPreview)
						  {
						    wd->m_pUnixToolbar->m_pFontPreview->setFontFamily(buffer);
						    wd->m_pUnixToolbar->m_pFontPreview->setText(buffer);
						    wd->m_pUnixToolbar->m_pFontPreview->draw();
						  }
					}
				}
			}
			else // widget has no ->parent, so use the buffer's results
			{
				// Don't do changes for empty combo texts, or when the
				// combo box selection has been aborted (by pressing
				// outside the box - this appears to be recognizable
				// by the HAS_GRAB flag being set: I don't know if
				// that's the correct way to detect this case. 
				// jskov 2001.12.09)
				if (UT_strcmp(wd->m_comboEntryBuffer, "") 
					&& !(GTK_OBJECT_FLAGS(widget) & GTK_HAS_GRAB))
				{ 
					const char * text = 
					    (wd->m_id == AP_TOOLBAR_ID_FMT_SIZE ? 
					    XAP_EncodingManager::fontsizes_mapping.lookupByTarget(wd->m_comboEntryBuffer) :
					    wd->m_comboEntryBuffer);
					UT_ASSERT(text);					
					
					UT_UCS4String ucsText(text);
					wd->m_pUnixToolbar->toolbarEvent(wd, ucsText.ucs4_str(), ucsText.length());
				}				
			}
		}
	}

	// unblock when the menu goes away
	static void s_combo_hide(GtkWidget * widget, gpointer user_data)
	{
		_wd * wd = static_cast<_wd *>(user_data);
		UT_ASSERT(wd);

		// manually force an update
		s_combo_changed(widget, user_data);

		// destroy the font preview window
		if (
		    (wd->m_id == AP_TOOLBAR_ID_FMT_FONT) && // this first check isn't needed, but I put it here anyway for the general understanding of the public :)
		    (wd->m_pUnixToolbar->m_pFontPreview != NULL)
		    )
		{
			UT_DEBUGMSG(("ev_UnixToolbar - deleting FontPreview %x \n",wd->m_pUnixToolbar));
		    delete wd->m_pUnixToolbar->m_pFontPreview;
			wd->m_pUnixToolbar->m_pFontPreview = NULL;
		}
	}

	// unblock when the menu goes away
	static void s_combo_show(GtkWidget * widget, gpointer user_data)
	{
		_wd * wd = static_cast<_wd *>(user_data);
		UT_ASSERT(wd);

		GtkWidget * entry = static_cast<GtkWidget*>(g_object_get_data(G_OBJECT(widget), "entry"));

		// only act if the widget has been shown and embedded in the toolbar
		if (wd->m_widget && entry && wd->m_id == AP_TOOLBAR_ID_FMT_FONT)
		{
		    // if the popwin is still shown, this is a copy run and widget has a ->parent
		    if (entry->parent)
			{
			// block is only honored here
				if (!wd->m_blockSignal)
				{
					const gchar * buffer = gtk_entry_get_text(GTK_ENTRY(entry));
					UT_uint32 length = strlen(buffer);
					
					if (length > 0) 
					{					
						UT_ASSERT(length < COMBO_BUF_LEN);				       
						
						if (wd->m_pUnixToolbar->m_pFontPreview == NULL)
						{
							int x,y;
				    
				    // this combo widget should have a parent widget which contains this combo widget _and_ the dropdown arrow
							GtkWidget * parent = gtk_widget_get_parent(entry);
							UT_ASSERT(parent);
							gdk_window_get_origin(parent->window, &x,&y);
							x += parent->allocation.x + parent->allocation.width;
							y += parent->allocation.y + parent->allocation.height;
							XAP_Frame * pFrame = static_cast<XAP_Frame *>(wd->m_pUnixToolbar->getFrame());
							wd->m_pUnixToolbar->m_pFontPreview = new XAP_UnixFontPreview(pFrame, x, y);
							UT_DEBUGMSG(("ev_UnixToolbar - building new FontPreview %x \n",wd->m_pUnixToolbar));
						}
						wd->m_pUnixToolbar->m_pFontPreview->setFontFamily(buffer);
						wd->m_pUnixToolbar->m_pFontPreview->setText(buffer);
						wd->m_pUnixToolbar->m_pFontPreview->draw();
					}
				}				   
			}
		}
	}


	EV_UnixToolbar *	m_pUnixToolbar;
	XAP_Toolbar_Id		m_id;
	GtkWidget *			m_widget;
	bool				m_blockSignal;
	char 				m_comboEntryBuffer[COMBO_BUF_LEN]; // TODO: make this an UT_UTF8String
};

static void
s_color_changed (GOComboColor 	*cc, 
				 GOColor 		 color,
				 gboolean 		 custom, 
				 gboolean 		 by_user, 
				 gboolean 		 is_default, 
				 _wd 			*wd)
{
	g_return_if_fail (wd);

	UT_UTF8String str (UT_UTF8String_sprintf ("%02x%02x%02x", 
											  UINT_RGBA_R (color),
											  UINT_RGBA_G (color),
											  UINT_RGBA_B (color)));
	
	wd->m_pUnixToolbar->toolbarEvent(wd, str.ucs4_str().ucs4_str(), str.size());
}

/*****************************************************************/

EV_UnixToolbar::EV_UnixToolbar(XAP_UnixApp * pUnixApp, 
			       XAP_Frame *pFrame, 
			       const char * szToolbarLayoutName,
			       const char * szToolbarLabelSetName)
	: EV_Toolbar(pUnixApp->getEditMethodContainer(),
		     szToolbarLayoutName,
		     szToolbarLabelSetName), 
	  m_wHandleBox(NULL)
{
	m_pFontPreview = NULL;
	m_pUnixApp = pUnixApp;
	m_pFrame = pFrame;
	m_pViewListener = 0;
	m_wToolbar = 0;
	m_lid = 0;							// view listener id
}

EV_UnixToolbar::~EV_UnixToolbar(void)
{
	UT_VECTOR_PURGEALL(_wd *,m_vecToolbarWidgets);
	_releaseListener();
}

bool EV_UnixToolbar::toolbarEvent(_wd * wd,
								  const UT_UCSChar * pData,
								  UT_uint32 dataLength)

{
	// user selected something from this toolbar.
	// invoke the appropriate function.
	// return true iff handled.

	XAP_Toolbar_Id id = wd->m_id;

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pUnixApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

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
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wd->m_widget),
						     !GTK_TOGGLE_BUTTON(wd->m_widget)->active);
			wd->m_blockSignal = wasBlocked;

			// can safely ignore this event
			return true;
		}
	}

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return false;
	
	const EV_EditMethodContainer * pEMC = m_pUnixApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

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
	GtkWidget * wVBox = static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getVBoxWidget();
	UT_sint32  pos = 0;
//
// Code gratutiously stolen from gtkbox.c
//
	GList *list = NULL;
	bool bFound = false;
	for( list = GTK_BOX(wVBox)->children; !bFound && list; list = list->next)
	{
		GtkBoxChild * child = static_cast<GtkBoxChild *>(list->data);
		if(child->widget == m_wHandleBox)
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
	gtk_widget_destroy(m_wHandleBox);
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
#ifdef HAVE_HILDON
#else
	GtkWidget * wVBox = static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getVBoxWidget();
	gtk_box_reorder_child(GTK_BOX(wVBox), m_wHandleBox, oldpos);
//
// bind  view listener
//
	AV_View * pView = getFrame()->getCurrentView();
	bindListenerToView(pView);
#endif	
}

static void setDragIcon(GtkWidget * wwd, GtkImage * img)
{
  if (GTK_IMAGE_PIXMAP == gtk_image_get_storage_type(img))
    {
      GdkPixmap * pixmap = NULL ;
      GdkBitmap * bitmap = NULL ;
      GdkColormap * clrmap = gtk_widget_get_colormap (wwd);
      gtk_image_get_pixmap ( img, &pixmap, &bitmap ) ;
      gtk_drag_source_set_icon(wwd,clrmap,pixmap,NULL);
    }
  else if (GTK_IMAGE_PIXBUF == gtk_image_get_storage_type(img))
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
	const XML_Char * szValue = NULL;
	m_pUnixApp->getPrefsValue(XAP_PREF_KEY_ToolbarAppearance,&szValue);
	UT_ASSERT((szValue) && (*szValue));
	
	GtkToolbarStyle style = GTK_TOOLBAR_ICONS;
	if (UT_XML_stricmp(szValue,"text")==0)
		style = GTK_TOOLBAR_TEXT;
	else if (UT_XML_stricmp(szValue,"both")==0)
		style = GTK_TOOLBAR_BOTH;

	return style;	
}

bool EV_UnixToolbar::synthesize(void)
{
	// create a GTK toolbar from the info provided.
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pUnixApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	XAP_Toolbar_ControlFactory * pFactory = m_pUnixApp->getControlFactory();
	UT_ASSERT(pFactory);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

#ifdef HAVE_HILDON	
#else
	GtkWidget * wVBox = static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getVBoxWidget();
#endif

	m_wHandleBox = gtk_alignment_new(0, 0, 1, 1);
	
	GtkToolbarStyle style = getStyle();

	m_wToolbar = gtk_toolbar_new();
	UT_ASSERT(m_wToolbar);
	
	gtk_toolbar_set_tooltips(GTK_TOOLBAR(m_wToolbar), TRUE);
	gtk_toolbar_set_style(GTK_TOOLBAR(m_wToolbar), style );
	gtk_toolbar_set_show_arrow(GTK_TOOLBAR(m_wToolbar), TRUE);

#ifdef HAVE_HILDON /* In Hildon its not posible */
#else
//
// Make the toolbar a destination for drops
//
	gtk_drag_dest_set(m_wToolbar,static_cast<GtkDestDefaults>(GTK_DEST_DEFAULT_ALL),
					  s_AbiTBTargets,1,
					  GDK_ACTION_COPY);
	g_signal_connect(G_OBJECT(m_wToolbar),"drag_drop",G_CALLBACK(_wd::s_drag_drop_toolbar),this);
#endif 

	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);

		XAP_Toolbar_Id id = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		UT_ASSERT(pAction);
		EV_Toolbar_Label * pLabel = m_pToolbarLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		switch (pLayoutItem->getToolbarLayoutFlags())
		{
		case EV_TLF_Normal:
		{
			_wd * wd = new _wd(this,id);
			UT_ASSERT(wd);

			const char * szToolTip = pLabel->getToolTip();
			if (!szToolTip || !*szToolTip)
				szToolTip = pLabel->getStatusMsg();

			switch (pAction->getItemType())
			{
			case EV_TBIT_PushButton:
			{
				UT_ASSERT(UT_stricmp(pLabel->getIconName(),"NoIcon")!=0);
				if(pAction->getToolbarId() != AP_TOOLBAR_ID_INSERT_TABLE)
				{
					wd->m_widget = toolbar_append_button (GTK_TOOLBAR (m_wToolbar), pLabel->getIconName(),
												    	  pLabel->getToolbarLabel(), NULL, 
														  (GCallback) _wd::s_callback, (gpointer) wd);
				}
				else
				{
//
// Hardwire the cool insert table widget onto the toolbar
//
					GtkWidget * abi_table = abi_table_new();
					gtk_widget_show(abi_table);
					UT_DEBUGMSG(("SEVIOR: Made insert table widget \n"));
					g_signal_connect(abi_table, "selected",
											 G_CALLBACK (_wd::s_new_table), static_cast<gpointer>(wd));

					UT_DEBUGMSG(("SEVIOR: Made connected to callback \n"));
					UT_UTF8String s;
					pSS->getValueUTF8(XAP_STRING_ID_TB_InsertNewTable, s);
					toolbar_append_item (GTK_TOOLBAR (m_wToolbar), abi_table, 
										 s.utf8_str(), NULL);
					gtk_widget_show_all(abi_table);
					gtk_widget_hide(ABI_TABLE(abi_table)->label);
					wd->m_widget = abi_table;
				}
				GtkWidget * wwd = wd->m_widget;
				g_object_set_data(G_OBJECT(wwd),
									"wd_pointer",
									wd);
#ifdef HAVE_HILDON /* not drag toolbar in hildon */
#else
				gtk_drag_source_set(wwd,GDK_BUTTON3_MASK,
									s_AbiTBTargets,1,
									GDK_ACTION_COPY);
				setDragIcon(wwd, (GtkImage*)gtk_image_new_from_stock(ABIWORD_INSERT_TABLE, GTK_ICON_SIZE_DND));
				gtk_drag_dest_set(wwd, GTK_DEST_DEFAULT_ALL,
									s_AbiTBTargets,1,
									GDK_ACTION_COPY);
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_begin",G_CALLBACK(_wd::s_drag_begin), wd);
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_drop",G_CALLBACK(_wd::s_drag_drop), wd);
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_end",G_CALLBACK(_wd::s_drag_end), wd);
#endif				
			}
			break;

			case EV_TBIT_ToggleButton:
			case EV_TBIT_GroupButton:
				{
					UT_ASSERT(UT_stricmp(pLabel->getIconName(),"NoIcon")!=0);
					wd->m_widget = toolbar_append_toggle (GTK_TOOLBAR (m_wToolbar), pLabel->getIconName(),
												    	  pLabel->getToolbarLabel(), NULL, 
														  (GCallback) _wd::s_callback, (gpointer) wd);
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
				setDragIcon(wwd, (GtkImage*)gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_DND));
				g_free (stock_id);
				stock_id = NULL;
				gtk_drag_dest_set(wwd,static_cast<GtkDestDefaults>(GTK_DEST_DEFAULT_ALL),
									s_AbiTBTargets,1,
									GDK_ACTION_COPY);
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_begin",G_CALLBACK(_wd::s_drag_begin), wd);
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_drop",G_CALLBACK(_wd::s_drag_drop), wd);
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_end",G_CALLBACK(_wd::s_drag_end), wd);
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

				// default, shouldn't be used for well-defined controls
				int iWidth = 100;

				if (pControl)
				{
					iWidth = pControl->getPixelWidth();
				}

				GtkWidget * comboBox = gtk_combo_new();
				UT_ASSERT(comboBox);

				// Combo boxes flash on 8-bit displays unless you set its colormap
				// to agree with what we're using elsewhere (gdk_rgb's version)
				gtk_widget_set_colormap(comboBox, gdk_rgb_get_colormap());
				
				// set the size of the entry to set the total combo size
				gtk_widget_set_size_request(GTK_COMBO(comboBox)->entry, iWidth, -1);

				// the entry is read-only for now
				gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(comboBox)->entry), FALSE);
										 
				// handle popup events, so we can block our signals until the popdown
				GtkWidget * popwin = GTK_WIDGET(GTK_COMBO(comboBox)->popwin);
				UT_ASSERT(popwin);

				g_object_set_data(G_OBJECT(popwin), "entry", GTK_COMBO(comboBox)->entry);

				g_signal_connect(G_OBJECT(popwin),
						 "hide",
						 G_CALLBACK(_wd::s_combo_hide),
						 wd);
				g_signal_connect(G_OBJECT(popwin),
						 "show",
						 G_CALLBACK(_wd::s_combo_show),
						 wd);

				// connect to the ->entry directly? Cleaned up version below.
			        g_signal_connect(G_OBJECT(GTK_COMBO(comboBox)->entry),
						 "changed",
						 G_CALLBACK(_wd::s_combo_changed),
						 wd);
			        g_signal_connect(G_OBJECT(GTK_COMBO(comboBox)->list),
						 "select-child",
						 G_CALLBACK(_wd::s_combo_list_changed),
						 wd);						 
				// populate it
				if (pControl)
				{
					pControl->populate();

					const UT_GenericVector<const char*> * v = pControl->getContents();
					UT_ASSERT(v);

					if (v)
					{
						UT_uint32 items = v->getItemCount();
						for (UT_uint32 m=0; m < items; m++)
						{
							const char * sz = v->getNthItem(m);
							GtkWidget * li = gtk_list_item_new_with_label(sz);
							gtk_widget_show(li);
							gtk_container_add (GTK_CONTAINER(GTK_COMBO(comboBox)->list), li);
						}
					}
				}
 
			        // give a final show
				gtk_widget_show(comboBox);

				// stick it in the toolbar
				toolbar_append_item (GTK_TOOLBAR (m_wToolbar), comboBox,
									 szToolTip, static_cast<const char *>(NULL));
				wd->m_widget = comboBox;

				// for now, we never repopulate, so can just toss it
				DELETEP(pControl);
			}
			break;

			case EV_TBIT_ColorFore:
			case EV_TBIT_ColorBack:
			{
				GdkPixbuf 		*pixbuf;
			    GtkWidget 		*combo;
				GOColorGroup 	*cg;

				UT_ASSERT (UT_stricmp(pLabel->getIconName(),"NoIcon") != 0);

			    if (pAction->getItemType() == EV_TBIT_ColorFore) {
					pixbuf = gtk_widget_render_icon (m_wToolbar, ABIWORD_COLOR_FORE, 
													 GTK_ICON_SIZE_LARGE_TOOLBAR, NULL);
					cg = go_color_group_fetch ("back_color_group", m_wToolbar);
					combo = go_combo_color_new (pixbuf, pLabel->getToolbarLabel(), 0, cg);
				}
				else {
					pixbuf = gtk_widget_render_icon (m_wToolbar, ABIWORD_COLOR_BACK, 
													 GTK_ICON_SIZE_LARGE_TOOLBAR, NULL);
					cg = go_color_group_fetch ("fore_color_group", m_wToolbar);
					combo = go_combo_color_new (pixbuf, pLabel->getToolbarLabel(), 0, cg);
				}
				go_combo_box_set_relief (GO_COMBO_BOX (combo), GTK_RELIEF_NONE);
				go_combo_color_set_instant_apply (GO_COMBO_COLOR (combo), TRUE);
				g_object_unref (G_OBJECT (pixbuf));

				toolbar_append_item (GTK_TOOLBAR(m_wToolbar), combo, szToolTip,
									 static_cast<const char *>(NULL));
			    wd->m_widget = combo;
			    g_signal_connect (G_OBJECT (combo), "color-changed",
								  G_CALLBACK (s_color_changed), wd);

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

#ifdef HAVE_HILDON	
	
	GtkWidget * wTLW = static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getTopLevelWindow();
	gtk_box_pack_end(GTK_BOX(HILDON_APPVIEW(wTLW)->vbox), m_wToolbar, FALSE, FALSE, 0);
	gtk_widget_show_all(m_wToolbar);	
	gtk_widget_show_all(wTLW);			

#else
	// show the complete thing
	gtk_widget_show(m_wToolbar);

	// pack it in a handle box
	gtk_container_add(GTK_CONTAINER(m_wHandleBox), m_wToolbar);
	// put it in the vbox
	gtk_widget_show(m_wHandleBox);
	gtk_box_pack_start(GTK_BOX(wVBox), m_wHandleBox, FALSE, FALSE, 0);

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
		UT_ASSERT(pLayoutItem);

		XAP_Toolbar_Id id = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		UT_ASSERT(pAction);

		AV_ChangeMask maskOfInterest = pAction->getChangeMaskOfInterest();
		if ((maskOfInterest & mask) == 0)					// if this item doesn't care about
			continue;										// changes of this type, skip it...

		switch (pLayoutItem->getToolbarLayoutFlags())
		{
		case EV_TLF_Normal:
			{
				const char * szState = 0;
				EV_Toolbar_ItemState tis = pAction->getToolbarItemState(pView,&szState);

				switch (pAction->getItemType())
				{
				case EV_TBIT_PushButton:
				{
					bool bGrayed = EV_TIS_ShouldBeGray(tis);

					_wd * wd = m_vecToolbarWidgets.getNthItem(k);
					UT_ASSERT(wd && wd->m_widget);
					gtk_widget_set_sensitive(wd->m_widget, !bGrayed);     					
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
					GtkCombo * item = GTK_COMBO(wd->m_widget);
					UT_ASSERT(item);
					// Disable/enable toolbar item
					gtk_widget_set_sensitive(GTK_WIDGET(item), !bGrayed);

					// Block the signal, set the contents
					bool wasBlocked = wd->m_blockSignal;
					wd->m_blockSignal = true;
					if (szState) {
					  if ( wd->m_id==AP_TOOLBAR_ID_FMT_SIZE )
					    {
					      const char * fsz = XAP_EncodingManager::fontsizes_mapping.lookupBySource(szState);
					      if ( fsz )
						gtk_entry_set_text(GTK_ENTRY(item->entry), fsz);
					      else
						gtk_entry_set_text(GTK_ENTRY(item->entry), "");
					    }
					  else
					    gtk_entry_set_text(GTK_ENTRY(item->entry), szState);
					} 
					else {
						gtk_entry_set_text(GTK_ENTRY(item->entry), "");
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
#ifdef HAVE_HILDON		
		gtk_widget_show (m_wToolbar);
#else
		GtkWidget *widget = gtk_bin_get_child(GTK_BIN(m_wHandleBox));
		gtk_widget_show(m_wHandleBox);
		gtk_widget_show (m_wToolbar->parent);
		if (getDetachable()) {
			gtk_widget_show(widget);

		}		
#endif	
	}
}

void EV_UnixToolbar::hide(void)
{

	if (m_wToolbar) {
#ifdef HAVE_HILDON		
		gtk_widget_hide (m_wToolbar);
#else
		GtkWidget *widget = gtk_bin_get_child(GTK_BIN(m_wHandleBox));
		gtk_widget_hide(m_wHandleBox);
		gtk_widget_hide (m_wToolbar->parent);
		if (getDetachable()) {
			gtk_widget_hide(widget);
		}		
#endif 	

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
	UT_ASSERT(pFactory);
	EV_Toolbar_Control * pControl = pFactory->getControl(this, id);
	AP_UnixToolbar_StyleCombo * pStyleC = static_cast<AP_UnixToolbar_StyleCombo *>(pControl);
	pStyleC->repopulate();
	GtkCombo * item = GTK_COMBO(wd->m_widget);
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
    
	GtkList * oldlist = GTK_LIST(item->list);
	gtk_list_clear_items(oldlist,0,-1);
//
// Now make a new one.
//
	UT_uint32 items = v->getItemCount();
	for (UT_uint32 m=0; m < items; m++)
	{
		const char * sz = v->getNthItem(m);
		GtkWidget * li = gtk_list_item_new_with_label(sz);
		gtk_widget_show(li);
		gtk_container_add (GTK_CONTAINER(GTK_COMBO(item)->list), li);
	}

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
