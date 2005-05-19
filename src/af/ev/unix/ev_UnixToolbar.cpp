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

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_locale.h"
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
#include "xap_UnixToolbar_Icons.h"
#include "ev_UnixToolbar_ViewListener.h"
#include "xav_View.h"
#include "xap_Prefs.h"
#include "fv_View.h"
#include "pd_Style.h"
#include "xap_EncodingManager.h"
#include "xap_UnixDialogHelper.h"
#include "xap_UnixFontPreview.h"
#include "xap_FontPreview.h"
#include "gr_UnixGraphics.h"
#include "ut_string_class.h"

// hack
#include "ap_Toolbar_Id.h"

#ifdef HAVE_GNOME
#include <gnome.h>

// gal stuff
#include "widget-color-combo.h"
#include "color-group.h"
#include "e-colors.h"

// hack to get the icons we need for the color combos
#include "../../../wp/ap/xp/ToolbarIcons/tb_text_fgcolor.xpm"
#include "../../../wp/ap/xp/ToolbarIcons/tb_text_bgcolor.xpm"
#endif // HAVE_GNOME

/*****************************************************************/

//! Some combos have a sort-model attached which cannot be altered. They have the original model in their gobject bucket.
#define COMBO_KEY_MODEL "combo-key-model"
//! Each combo knows about its changed signal id
#define COMBO_KEY_CHANGED_SIGNAL "combo-key-changed-signal"
//! Fonts are loaded lazily, we need to remember what to set after we're ready.
#define FONTCOMBO_KEY_ACTIVE_FONT "fontcombo-key-active-font"

static const GtkTargetEntry      s_AbiTBTargets[] = {{"abi-toolbars",0,0}};

class _wd;

//! list-store columns
enum {
	COLUMN_STRING = 0,
	COLUMN_FONT,
	NUM_COLUMNS
};

//! container for the idle handler (for loading fonts)
typedef struct {
	_wd 				*wd;
	EV_Toolbar_Control  *pControl;
	UT_GenericVector<gchar*> *strings;
	guint 				 idx;
} FontComboIdleData;

void 		 abi_gtk_combo_box_fill_from_string_vector (_wd 				*wd, 
														EV_Toolbar_Control 	*pControl, 
														const UT_GenericVector<const char*> *strings);
const gchar *abi_gtk_combo_box_get_active_text		   (_wd					*wd);
gboolean     font_combo_idle_fill					   (gpointer data);


/**
 * toolbar_append_with_eventbox
 *
 * Borrowed code from gnumeric (src/gnumeric-util.c)
 *
 * @toolbar               toolbar
 * @widget                widget to insert
 * @tooltip_text          tooltip text
 * @tooltip_private_text  longer tooltip text
 *
 * Packs widget in an eventbox and adds the eventbox to toolbar.
 * This lets a windowless widget (e.g. combo box) have tooltips.
 **/
static GtkWidget *
toolbar_append_with_eventbox (GtkToolbar *toolbar, GtkWidget  *widget,
			      const char *tooltip_text,
			      const char *tooltip_private_text)
{
	GtkWidget *eventbox;

	UT_ASSERT(GTK_IS_TOOLBAR (toolbar));
	UT_ASSERT(widget != NULL);

	/* An event box to receive events - this is a requirement for having
           tooltips */
	eventbox = gtk_event_box_new ();
	gtk_widget_show (widget);
	gtk_container_add (GTK_CONTAINER (eventbox), widget);
	gtk_widget_show (eventbox);
	gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), eventbox,
				   tooltip_text, tooltip_private_text);
	return eventbox;
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

	static void s_ColorCallback(GtkWidget * /* widget */, gpointer user_data)
	{
		// this is a static callback method and does not have a 'this' pointer.
		// map the user_data into an object and dispatch the event.

//
// This is hardwired to popup the color picker dialog
//	
		_wd * wd = static_cast<_wd *>(user_data);
		UT_ASSERT(wd);

		if (!wd->m_blockSignal)
		{
			XAP_Toolbar_Id id = wd->m_id;
			XAP_UnixApp * pUnixApp = wd->m_pUnixToolbar->getApp();
			const EV_EditMethodContainer * pEMC = pUnixApp->getEditMethodContainer();
			UT_ASSERT(pEMC);
			EV_EditMethod * pEM = NULL;

			AV_View * pView = wd->m_pUnixToolbar->getFrame()->getCurrentView();

			if(id ==  AP_TOOLBAR_ID_COLOR_FORE)
				pEM = pEMC->findEditMethodByName("dlgColorPickerFore");
			else
			    pEM = pEMC->findEditMethodByName("dlgColorPickerBack");
			wd->m_pUnixToolbar->invokeToolbarMethod(pView,pEM,NULL,0);
		}				
	};
	
	// TODO: should this move out of wd?  It's convenient here; maybe I'll make
	// a microclass for combo boxes.
	static void s_combo_changed(GtkWidget * widget, gpointer user_data)
	{
		_wd * wd = static_cast<_wd *>(user_data);
		UT_ASSERT(wd);

		if (wd->m_id == AP_TOOLBAR_ID_FMT_STYLE) {

			// handle style combo, always system font in selected entry

			//GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (widget));
			gpointer data;
			GtkTreeModel *model = NULL;
			if (NULL != (data = g_object_get_data(G_OBJECT(widget), COMBO_KEY_MODEL))) {
				// the model returned by ..._get_model is only a sorted view
				model = GTK_TREE_MODEL(data);
			}

			const gchar *FONT_DESC_KEY = "font-desc-key";
			const gchar *PATH_KEY = "path-key";
			GtkTreeIter iter;
			gpointer path = NULL;
			gpointer desc = NULL;

			// restore prev
			if (NULL != (desc = g_object_get_data (G_OBJECT (widget), FONT_DESC_KEY)) && 
				NULL != (path = g_object_get_data (G_OBJECT (widget), PATH_KEY))) {
				
				gtk_tree_model_get_iter (model, &iter, (GtkTreePath*)path);
				gtk_list_store_set (GTK_LIST_STORE (model), &iter,
									COLUMN_FONT, (PangoFontDescription*)desc,
									-1);
			}

			// get current
			GtkTreeIter currentIter;
			const gchar *current = NULL;
			GtkTreeModel *sort_model = gtk_combo_box_get_model (GTK_COMBO_BOX(widget));
			gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget), &iter);
			gtk_tree_model_get (sort_model, &iter, COLUMN_STRING, &current, -1);
			gtk_tree_model_get_iter_first (model, &currentIter);
			
			// backup current
			do {
				gchar *value;
				gtk_tree_model_get (model, &currentIter, COLUMN_STRING, &value, -1);
				if (!UT_strcmp (current, value)) {
					path = gtk_tree_model_get_path (model, &currentIter);
					g_object_set_data (G_OBJECT (widget), PATH_KEY, path);
					gtk_tree_model_get (model, &currentIter, COLUMN_FONT, &desc, -1);
					g_object_set_data (G_OBJECT (widget), FONT_DESC_KEY, desc);
					break;	
				}
			}
			while (gtk_tree_model_iter_next (model, &currentIter));

			// set system font on current
			PangoContext *context = gtk_widget_get_pango_context (GTK_WIDGET (widget));
			desc = pango_context_get_font_description (context);
			gtk_list_store_set (GTK_LIST_STORE (model), &currentIter,
				      COLUMN_FONT, (PangoFontDescription*)desc,
				      -1);
		}

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
					buffer = abi_gtk_combo_box_get_active_text(wd);
					
					UT_uint32 length = strlen(buffer);
					if (length > 0) 
					{						
						UT_UCS4String ucsText(buffer);
						wd->m_pUnixToolbar->toolbarEvent(wd, ucsText.ucs4_str(), ucsText.length());
					}
				}
			}
		}
	}

	EV_UnixToolbar *	m_pUnixToolbar;
	XAP_Toolbar_Id		m_id;
	GtkWidget *			m_widget;
	bool				m_blockSignal;
};

#ifdef HAVE_GNOME
#define COLOR_NORMALIZE(c) (gint)(c >> 8)

static void
s_color_changed (ColorCombo * combo, GdkColor * color, gboolean custom, gboolean by_user, gboolean is_default, _wd * wd)
{
	// if nothing has been set, color will be null
	// and we don't want to dereference null do we ;)
	if (!color || !combo || !wd)
	  return;
	UT_UTF8String str (UT_UTF8String_sprintf ("%02x%02x%02x", COLOR_NORMALIZE (color->red), COLOR_NORMALIZE (color->green), COLOR_NORMALIZE (color->blue)));
	wd->m_pUnixToolbar->toolbarEvent(wd, str.ucs4_str().ucs4_str(), str.size());
}

#undef COLOR_NORMALIZE
#endif // HAVE_GNOME

/*****************************************************************/

EV_UnixToolbar::EV_UnixToolbar(XAP_UnixApp * pUnixApp, 
			       XAP_Frame *pFrame, 
			       const char * szToolbarLayoutName,
			       const char * szToolbarLabelSetName)
	: EV_Toolbar(pUnixApp->getEditMethodContainer(),
		     szToolbarLayoutName,
		     szToolbarLabelSetName)
{
	m_pUnixApp = pUnixApp;
	m_pFrame = pFrame;
	m_pViewListener = 0;
	m_wToolbar = 0;
	m_lid = 0;							// view listener id

#ifdef HAVE_GNOME
	e_color_init ();
#endif
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
	GtkWidget * wVBox = static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getVBoxWidget();
	gtk_box_reorder_child(GTK_BOX(wVBox), m_wHandleBox,oldpos);
//
// bind  view listener
//
	AV_View * pView = getFrame()->getCurrentView();
	bindListenerToView(pView);
}

bool EV_UnixToolbar::getPixmapForIcon(XAP_Toolbar_Id id, GdkWindow * window, GdkColor * background,
				      const char * szIconName, GtkWidget ** pwPixmap)
{
	const char * stock_id = NULL ;

	switch ( id )
	{
		case AP_TOOLBAR_ID_FILE_NEW: stock_id = GTK_STOCK_NEW ; break ;
		case AP_TOOLBAR_ID_FILE_OPEN: stock_id = GTK_STOCK_OPEN ; break ;
		case AP_TOOLBAR_ID_FILE_SAVE: stock_id = GTK_STOCK_SAVE ; break ;
		case AP_TOOLBAR_ID_FILE_SAVEAS: stock_id = GTK_STOCK_SAVE_AS ; break ;
		case AP_TOOLBAR_ID_FILE_PRINT: stock_id = GTK_STOCK_PRINT ; break ;
		case AP_TOOLBAR_ID_FILE_PRINT_PREVIEW: stock_id = GTK_STOCK_PRINT_PREVIEW ; break ;
			
		case AP_TOOLBAR_ID_EDIT_UNDO: stock_id = GTK_STOCK_UNDO ; break ;
		case AP_TOOLBAR_ID_EDIT_REDO: stock_id = GTK_STOCK_REDO ; break ;
		case AP_TOOLBAR_ID_EDIT_CUT: stock_id = GTK_STOCK_CUT ; break ;
		case AP_TOOLBAR_ID_EDIT_COPY: stock_id = GTK_STOCK_COPY ; break ;
		case AP_TOOLBAR_ID_EDIT_PASTE: stock_id = GTK_STOCK_PASTE ; break ;
						
		case AP_TOOLBAR_ID_FMT_BOLD: stock_id = GTK_STOCK_BOLD ; break ;
		case AP_TOOLBAR_ID_FMT_ITALIC: stock_id = GTK_STOCK_ITALIC ; break ;
		case AP_TOOLBAR_ID_FMT_UNDERLINE: stock_id = GTK_STOCK_UNDERLINE ; break ;
		case AP_TOOLBAR_ID_FMT_STRIKE: stock_id = GTK_STOCK_STRIKETHROUGH ; break ;
			
		case AP_TOOLBAR_ID_ALIGN_LEFT: stock_id = GTK_STOCK_JUSTIFY_LEFT ; break ;
		case AP_TOOLBAR_ID_ALIGN_CENTER: stock_id = GTK_STOCK_JUSTIFY_CENTER ; break ;
		case AP_TOOLBAR_ID_ALIGN_RIGHT: stock_id = GTK_STOCK_JUSTIFY_RIGHT ; break ;
		case AP_TOOLBAR_ID_ALIGN_JUSTIFY: stock_id = GTK_STOCK_JUSTIFY_FILL ; break ;
			
		case AP_TOOLBAR_ID_SPELLCHECK: stock_id = GTK_STOCK_SPELL_CHECK ; break ;
		case AP_TOOLBAR_ID_HELP: stock_id = GTK_STOCK_HELP ; break ;
		case AP_TOOLBAR_ID_SCRIPT_PLAY: stock_id = GTK_STOCK_EXECUTE ; break ;

#if GTK_CHECK_VERSION(2,4,0)
	case AP_TOOLBAR_ID_UNINDENT: stock_id = GTK_STOCK_UNINDENT ; break ;
	case AP_TOOLBAR_ID_INDENT: stock_id = GTK_STOCK_INDENT ; break ;
#elif defined(HAVE_GNOME)
	case AP_TOOLBAR_ID_UNINDENT: stock_id = GNOME_STOCK_TEXT_UNINDENT ; break ;
	case AP_TOOLBAR_ID_INDENT: stock_id = GNOME_STOCK_TEXT_INDENT ; break ;
	case AP_TOOLBAR_ID_LISTS_NUMBERS: stock_id = GNOME_STOCK_TEXT_NUMBERED_LIST ; break ;
	case AP_TOOLBAR_ID_LISTS_BULLETS: stock_id = GNOME_STOCK_TEXT_BULLETED_LIST ; break ;
#endif

		default:
			break ;
	}
	
	if ( stock_id == NULL )
	{
		return m_pUnixToolbarIcons.getPixmapForIcon ( window, background, szIconName, pwPixmap ) ;
	}
	else
	{
		*pwPixmap = gtk_image_new_from_stock ( stock_id, GTK_ICON_SIZE_LARGE_TOOLBAR ) ;
		gtk_widget_show ( *pwPixmap ) ;
		return true ;
	}
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

bool EV_UnixToolbar::synthesize(void)
{
	// create a GTK toolbar from the info provided.

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pUnixApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	XAP_Toolbar_ControlFactory * pFactory = m_pUnixApp->getControlFactory();
	UT_ASSERT(pFactory);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	GtkWidget * wTLW = static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getTopLevelWindow();
	GtkWidget * wVBox = static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getVBoxWidget();

	m_wHandleBox = gtk_handle_box_new();
	gtk_handle_box_set_shadow_type (GTK_HANDLE_BOX(m_wHandleBox), GTK_SHADOW_NONE);
	UT_ASSERT(m_wHandleBox);

	////////////////////////////////////////////////////////////////
	// get toolbar button appearance from the preferences
	////////////////////////////////////////////////////////////////
	
	const XML_Char * szValue = NULL;
	m_pUnixApp->getPrefsValue(XAP_PREF_KEY_ToolbarAppearance,&szValue);
	UT_ASSERT((szValue) && (*szValue));
	
	GtkToolbarStyle style = GTK_TOOLBAR_ICONS;
	if (UT_XML_stricmp(szValue,"text")==0)
		style = GTK_TOOLBAR_TEXT;
	else if (UT_XML_stricmp(szValue,"both")==0)
		style = GTK_TOOLBAR_BOTH;

	m_wToolbar = gtk_toolbar_new();
	UT_ASSERT(m_wToolbar);
	
	gtk_toolbar_set_tooltips(GTK_TOOLBAR(m_wToolbar), TRUE);
	gtk_toolbar_set_style(GTK_TOOLBAR( m_wToolbar), style );

//
// Make the toolbar a destination for drops
//
	gtk_drag_dest_set(m_wToolbar,static_cast<GtkDestDefaults>(GTK_DEST_DEFAULT_ALL),
					  s_AbiTBTargets,1,
					  GDK_ACTION_COPY);
	g_signal_connect(G_OBJECT(m_wToolbar),"drag_drop",G_CALLBACK(_wd::s_drag_drop_toolbar),this);

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
				GtkWidget * wPixmap;
				getPixmapForIcon ( pAction->getToolbarId(), wTLW->window,
								   &wTLW->style->bg[GTK_STATE_NORMAL],
								   pLabel->getIconName(),
								   &wPixmap);

				if(pAction->getToolbarId() != AP_TOOLBAR_ID_INSERT_TABLE)
				{
					wd->m_widget = gtk_toolbar_append_item(GTK_TOOLBAR(m_wToolbar),
														   pLabel->getToolbarLabel(),
														   szToolTip,static_cast<const char *>(NULL),
														   wPixmap,
														   G_CALLBACK(_wd::s_callback),
														   wd);
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
					abi_table_embed_on_toolbar(ABI_TABLE(abi_table), GTK_TOOLBAR(m_wToolbar));
					gtk_widget_show_all(abi_table);
					gtk_widget_hide(ABI_TABLE(abi_table)->label);
					wd->m_widget = abi_table;
				}
				GtkWidget * wwd = wd->m_widget;
				g_object_set_data(G_OBJECT(wwd),
									"wd_pointer",
									wd);
				gtk_drag_source_set(wwd,GDK_BUTTON3_MASK,
									s_AbiTBTargets,1,
									GDK_ACTION_COPY);
				setDragIcon(wwd, GTK_IMAGE(wPixmap));
				gtk_drag_dest_set(wwd, GTK_DEST_DEFAULT_ALL,
									s_AbiTBTargets,1,
									GDK_ACTION_COPY);
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_begin",G_CALLBACK(_wd::s_drag_begin), wd);
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_drop",G_CALLBACK(_wd::s_drag_drop), wd);
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_end",G_CALLBACK(_wd::s_drag_end), wd);
			}
			break;

			case EV_TBIT_ToggleButton:
			case EV_TBIT_GroupButton:
				{
					UT_ASSERT(UT_stricmp(pLabel->getIconName(),"NoIcon")!=0);
					GtkWidget * wPixmap;
					getPixmapForIcon ( pAction->getToolbarId(), wTLW->window,
									   &wTLW->style->bg[GTK_STATE_NORMAL],
									   pLabel->getIconName(),
									   &wPixmap);

					wd->m_widget = gtk_toolbar_append_element(GTK_TOOLBAR(m_wToolbar),
															  GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
															  static_cast<GtkWidget *>(NULL),
															  pLabel->getToolbarLabel(),
															  szToolTip,static_cast<const char *>(NULL),
															  wPixmap,
															  G_CALLBACK(_wd::s_callback),
															  wd);
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
				setDragIcon(wwd, GTK_IMAGE(wPixmap));
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

				GtkWidget *combo = gtk_combo_box_new();
				GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
				gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
				GtkTreeModel *model = NULL;
				wd->m_widget = combo;

				if (wd->m_id == AP_TOOLBAR_ID_ZOOM) {
					// zoom
					gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, "text", COLUMN_STRING, NULL); 
					model = GTK_TREE_MODEL (gtk_list_store_new(1, G_TYPE_STRING));
				}
				else if (wd->m_id == AP_TOOLBAR_ID_FMT_STYLE) {
					// style preview
					gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, 
													"text", COLUMN_STRING, 
													"font-desc", COLUMN_FONT,
													NULL); 				
					GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, PANGO_TYPE_FONT_DESCRIPTION);
					g_object_set_data(G_OBJECT(combo), COMBO_KEY_MODEL, store);

					model = gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL (store));
					gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), COLUMN_STRING, GTK_SORT_ASCENDING);					
				}
				else if (wd->m_id == AP_TOOLBAR_ID_FMT_FONT) {
					// font preview
					gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, 
													"text", COLUMN_STRING, 
													"font", COLUMN_FONT, 
													NULL);					

					GtkListStore *store = gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
					g_object_set_data(G_OBJECT(combo), COMBO_KEY_MODEL, store);

					model = gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL (store));
					gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), COLUMN_STRING, GTK_SORT_ASCENDING);					
				}
				else if (wd->m_id == AP_TOOLBAR_ID_FMT_SIZE) {

					// hack, using a GtkComboBoxEntry here, need to destroy the default combo
					gtk_widget_destroy (combo);
					combo = NULL;
	
					combo = gtk_combo_box_entry_new();
					wd->m_widget = combo;

					model = GTK_TREE_MODEL (gtk_list_store_new(1, G_TYPE_STRING));

					// hack continued
					gtk_combo_box_set_model(GTK_COMBO_BOX(combo), model);
					gtk_combo_box_entry_set_text_column (GTK_COMBO_BOX_ENTRY (combo), COLUMN_STRING);
				}
				else {
					gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, "text", COLUMN_STRING, NULL); 
					model = GTK_TREE_MODEL (gtk_list_store_new(1, G_TYPE_STRING));
				}

				GtkWidget *align = gtk_hbox_new (FALSE, 0);
				gtk_box_pack_start (GTK_BOX (align), combo, TRUE, FALSE, 5);
				gtk_widget_show (align);

				gtk_combo_box_set_model(GTK_COMBO_BOX(combo), model);
				g_object_unref(model);
				
				// set the size of the entry to set the total combo size
				gtk_widget_set_size_request(combo, iWidth, -1);

		        gulong sig = g_signal_connect(G_OBJECT(GTK_COMBO_BOX(combo)),
								 "changed",
								 G_CALLBACK(_wd::s_combo_changed),
								 wd);
				g_object_set_data (G_OBJECT(combo), COMBO_KEY_CHANGED_SIGNAL, (gpointer)sig);

				// populate it
				if (pControl)
				{
					pControl->populate();

					const UT_GenericVector<const char*> * v = pControl->getContents();
					UT_ASSERT(v);
					abi_gtk_combo_box_fill_from_string_vector (wd, pControl, v);
				}
 
				// stick it in the toolbar
				GtkWidget *widget = NULL;
				if (align != NULL)
					widget = align;
				else
					widget = combo;
				gtk_toolbar_append_widget (GTK_TOOLBAR (m_wToolbar), widget, szToolTip, NULL);
				gtk_widget_show(combo);

				// for now, we never repopulate, so can just toss it
				DELETEP(pControl);
			}
			break;

			case EV_TBIT_ColorFore:
			case EV_TBIT_ColorBack:
			{
				UT_ASSERT(UT_stricmp(pLabel->getIconName(),"NoIcon")!=0);
				GtkWidget * wPixmap;
				getPixmapForIcon ( pAction->getToolbarId(), wTLW->window,
								   &wTLW->style->bg[GTK_STATE_NORMAL],
								   pLabel->getIconName(),
								   &wPixmap);

#ifdef HAVE_GNOME
			    GtkWidget * combo;
				GdkPixbuf * pixbuf;
			    if (pAction->getItemType() == EV_TBIT_ColorFore) {
					pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)(tb_text_fgcolor_xpm));
					combo = color_combo_new (pixbuf, szToolTip, &e_black, color_group_fetch("foreground_color", NULL));
			    }
				else {
					pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)(tb_text_bgcolor_xpm));
					combo = color_combo_new (pixbuf, szToolTip, NULL, color_group_fetch("background_color", NULL));
				}
			    gal_combo_box_set_title (GAL_COMBO_BOX (combo),
										 szToolTip);

				toolbar_append_with_eventbox(GTK_TOOLBAR(m_wToolbar),
											 combo,
											 szToolTip,
											 static_cast<const char *>(NULL));
			    wd->m_widget = combo;
			    g_signal_connect (G_OBJECT (combo), "color-changed",
								  G_CALLBACK (s_color_changed), wd);
#else
				wd->m_widget = gtk_toolbar_append_item(GTK_TOOLBAR(m_wToolbar),
													   pLabel->getToolbarLabel(),
													   szToolTip,static_cast<const char *>(NULL),
													   wPixmap,
													   G_CALLBACK(_wd::s_ColorCallback),
													   wd);
#endif
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

			gtk_toolbar_append_space(GTK_TOOLBAR(m_wToolbar));
			break;
		}
		
		default:
			UT_ASSERT(0);
		}
	}

	// show the complete thing
	gtk_widget_show(m_wToolbar);

	// pack it in a handle box
	gtk_container_add(GTK_CONTAINER(m_wHandleBox), m_wToolbar);
	gtk_widget_show(m_wHandleBox);
	
	// put it in the vbox
	gtk_box_pack_start(GTK_BOX(wVBox), m_wHandleBox, FALSE, FALSE, 0);

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
	return true;
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
					UT_ASSERT(wd);
					GtkButton * item = GTK_BUTTON(wd->m_widget);
					UT_ASSERT(item);
						
					// Disable/enable toolbar item
					gtk_widget_set_sensitive(GTK_WIDGET(item), !bGrayed);     					
				}
				break;
			
				case EV_TBIT_ToggleButton:
				case EV_TBIT_GroupButton:
				{
					bool bGrayed = EV_TIS_ShouldBeGray(tis);
					bool bToggled = EV_TIS_ShouldBeToggled(tis);

					_wd * wd = m_vecToolbarWidgets.getNthItem(k);
					UT_ASSERT(wd);
					GtkToggleButton * item = GTK_TOGGLE_BUTTON(wd->m_widget);
					UT_ASSERT(item);
						
					// Block the signal, throw the toggle event
					bool wasBlocked = wd->m_blockSignal;
					wd->m_blockSignal = true;
					gtk_toggle_button_set_active(item, bToggled);
					wd->m_blockSignal = wasBlocked;
						
					// Disable/enable toolbar item
					gtk_widget_set_sensitive(GTK_WIDGET(item), !bGrayed);						
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
					GtkComboBox * item = GTK_COMBO_BOX(wd->m_widget);
					UT_ASSERT(item);
					// Disable/enable toolbar item
					gtk_widget_set_sensitive(GTK_WIDGET(item), !bGrayed);

					// Block the signal, set the contents
					bool wasBlocked = wd->m_blockSignal;
					wd->m_blockSignal = true;
					if (szState) {
					    selectComboEntry(wd, szState);
					} 
					else {
						selectComboEntry(wd, "");
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
	if (m_wToolbar)
		gtk_widget_show (m_wToolbar->parent);
	EV_Toolbar::show();
}

void EV_UnixToolbar::hide(void)
{
	if (m_wToolbar)
		gtk_widget_hide (m_wToolbar->parent);
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
    
//
// Now make a new one.
//
	const gchar *style = abi_gtk_combo_box_get_active_text(wd);
	abi_gtk_combo_box_fill_from_string_vector (wd, pStyleC, v);
	if (style && *style) {
		selectComboEntry(wd, style);
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

/*!
* Select a combo box entry by its label.
*/
void 
EV_UnixToolbar::selectComboEntry (_wd 		  *wd,
								  const gchar *text) {

	GtkComboBox *combo = GTK_COMBO_BOX (wd->m_widget);
	GtkTreeModel *model = gtk_combo_box_get_model (combo);
	GtkTreeIter iter;
	guint idx = 0;
	if (gtk_tree_model_get_iter_first (model, &iter)) {
		do {
			gchar *value;
			gtk_tree_model_get (model, &iter, COLUMN_STRING, &value, -1);

			if (wd->m_id == AP_TOOLBAR_ID_FMT_SIZE) {
				// font size, compare numbers
				if (atoi (text) == atoi (value)) {
					gtk_combo_box_set_active (combo, idx);
					return;
				}				
			}
			else {
				// compare strings
				if (!UT_strcmp (text, value)) {
					gtk_combo_box_set_active (combo, idx);
					return;
				}
			}
			idx++;
		}
		while (gtk_tree_model_iter_next (model, &iter));

		// item not found
		if (wd->m_id == AP_TOOLBAR_ID_FMT_FONT) {
			// remember to set after lazy loading
			g_object_set_data (G_OBJECT(combo), FONTCOMBO_KEY_ACTIVE_FONT, (gpointer)text);
		}
		else if (wd->m_id == AP_TOOLBAR_ID_FMT_SIZE) {
			GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo));
			gtk_entry_set_text(GTK_ENTRY(entry), text);
		}
		else if (wd->m_id == AP_TOOLBAR_ID_FMT_STYLE) {

			GtkListStore *store = NULL;
			gpointer data;
			if (NULL != (data = g_object_get_data(G_OBJECT(combo), COMBO_KEY_MODEL))) {
				store = GTK_LIST_STORE(data);
			}
			UT_return_if_fail(store != NULL);

			XAP_Toolbar_ControlFactory * pFactory = m_pUnixApp->getControlFactory();
			UT_ASSERT(pFactory);
			EV_Toolbar_Control * pControl = pFactory->getControl(this, wd->m_id );
			AP_UnixToolbar_StyleCombo * pStyleCombo = static_cast<AP_UnixToolbar_StyleCombo *>(pControl);
			
			const PangoFontDescription *desc = pStyleCombo->getStyle(text);
			//UT_return_if_fail(desc);

			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter, 
								COLUMN_STRING, text, 
								COLUMN_FONT, desc, 
								-1);
		}
	}
}

/*!
* Refill a combo box (list model, one string column) from a string vector.
*/
void 
abi_gtk_combo_box_fill_from_string_vector (_wd *wd,
										   EV_Toolbar_Control *pControl,
										   const UT_GenericVector<const char*> *strings) {

	GtkComboBox *combo = GTK_COMBO_BOX (wd->m_widget);
	gpointer data = NULL;
	GtkListStore *store = NULL;
	if (NULL != (data = g_object_get_data(G_OBJECT(combo), COMBO_KEY_MODEL))) {
		// the model returned by ..._get_model is only a sorted view
		store = GTK_LIST_STORE(data);
	}
	else {
		store = GTK_LIST_STORE(gtk_combo_box_get_model(combo));
	}

	AP_UnixToolbar_StyleCombo * pStyleCombo = NULL;
	if (wd->m_id == AP_TOOLBAR_ID_FMT_STYLE) {
		pStyleCombo = reinterpret_cast<AP_UnixToolbar_StyleCombo*>(pControl);
	}
	else if (wd->m_id == AP_TOOLBAR_ID_FMT_FONT) {

		// load using an idle handler

		UT_GenericVector<gchar*> *fonts = new UT_GenericVector<gchar*>();
		for (guint i = 0; i < strings->size(); i++) {
			fonts->addItem(g_strdup(strings->getNthItem(i)));
		}

		FontComboIdleData *pData = new FontComboIdleData();
		pData->wd = wd;
		pData->pControl = pControl;
		pData->strings = fonts;
		pData->idx = 0;
		g_idle_add (font_combo_idle_fill, pData);
		return;
	}


	gtk_list_store_clear (store);
	int count = strings->size ();
	for (int i = 0; i < count; i++) {
	
		if (wd->m_id == AP_TOOLBAR_ID_FMT_STYLE) {
			// style preview
			const gchar * style = strings->getNthItem(i);
			GtkTreeIter iter;
			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter, 
								COLUMN_STRING, style, 
								COLUMN_FONT, pStyleCombo->getStyle(style), 
								-1);
		}
		else {
			GtkTreeIter iter;
			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter, COLUMN_STRING, strings->getNthItem(i), -1);
		}
	}
}

/**
* Get currently active combo box text.
*/
const gchar *
abi_gtk_combo_box_get_active_text (_wd *wd) {

	GtkComboBox *combo = GTK_COMBO_BOX(wd->m_widget);
	GtkTreeModel *model = gtk_combo_box_get_model (combo);
	gint idx = gtk_combo_box_get_active (combo);
	GtkTreePath *path = NULL;
	if (idx < 0 && wd->m_id == AP_TOOLBAR_ID_FMT_SIZE) {
		GtkWidget *entry = gtk_bin_get_child(GTK_BIN(combo));
		return gtk_entry_get_text(GTK_ENTRY(entry));
	}
	else if (idx >= 0) {
		path = gtk_tree_path_new_from_indices (idx, -1);
	}
	else {
		return NULL;
	}

	GtkTreeIter iter;
	const gchar *value = NULL;
	if (gtk_tree_model_get_iter (model, &iter, path)) {
		gtk_tree_model_get (model, &iter, COLUMN_STRING, &value, -1);
	}

	gtk_tree_path_free (path);
	return value;
}

/*!
* Idle handler for filling the font combo.
*/
gboolean     
font_combo_idle_fill (gpointer data) {

	FontComboIdleData *pData = static_cast<FontComboIdleData*>(data);
	GtkComboBox *combo = GTK_COMBO_BOX(pData->wd->m_widget);
	GtkListStore *store = NULL;
	if (NULL != (data = g_object_get_data(G_OBJECT(combo), COMBO_KEY_MODEL))) {
		// the model returned by ..._get_model is only a sorted view
		store = GTK_LIST_STORE(data);
	}

	GtkTreeIter iter;
	for (guint i = pData->idx; i < pData->strings->size(); i++) {
	
		const gchar *name = pData->strings->getNthItem(i);
		XAP_UnixFont *pFont = XAP_UnixFontManager::pFontManager->getFont (name, XAP_UnixFont::STYLE_NORMAL);
		if (pFont && (pFont->isSymbol() || pFont->isDingbat())) {
			// "$fontname (Symbols)"
			const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
			static UT_UTF8String tmpName;
			tmpName = UT_UTF8String_sprintf ("%s (%s)", name, pSS->getValue(XAP_STRING_ID_TB_Font_Symbol));
			name = tmpName.utf8_str();
		}

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, 
							COLUMN_STRING, name, 
							COLUMN_FONT, pData->strings->getNthItem(i), 
							-1);

		if (i >= (pData->idx + 5)) {
			pData->idx = i + 1;
			return TRUE;
		}
	}

	// highlight font
	gchar *font = NULL;
	GtkTreeModel *model = gtk_combo_box_get_model(combo);
	if (NULL != (font = (gchar*)g_object_get_data(G_OBJECT(combo), FONTCOMBO_KEY_ACTIVE_FONT))) {
		gtk_tree_model_get_iter_first (model, &iter);
		do {
			gchar *value;
			gtk_tree_model_get (model, &iter, COLUMN_STRING, &value, -1);
			if (!UT_strcmp (font, value)) {
				gulong sig = (gulong)g_object_get_data (G_OBJECT(combo), COMBO_KEY_CHANGED_SIGNAL);
				g_signal_handler_block (combo, sig);
				gtk_combo_box_set_active_iter (combo, &iter);
				g_signal_handler_unblock (combo, sig);
				break;
			}
		}
		while (gtk_tree_model_iter_next (model, &iter));
	}

	for (guint i = 0; i < pData->strings->size(); i++) {
		free(pData->strings->getNthItem(i));
	}
	delete pData->strings;
	delete pData;
	return FALSE;
}
