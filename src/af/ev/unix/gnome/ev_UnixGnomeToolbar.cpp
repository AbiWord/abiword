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

#include <gnome.h>
#ifdef HAVE_BONOBO
//#include <libgnorba/gnorba.h>
#include <bonobo.h>
#endif

#include <string.h>
#include <stdlib.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ev_UnixToolbar.h"
#include "xap_Types.h"
#include "xap_UnixGnomeApp.h"
#include "xap_UnixGnomeFrame.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Layouts.h"
#include "ev_Toolbar_Labels.h"
#include "ev_Toolbar_Control.h"
#include "ev_EditEventMapper.h"
#include "xap_UnixToolbar_Icons.h"
#include "ev_UnixToolbar_ViewListener.h"
#include "xav_View.h"
#include "xap_Prefs.h"
#include "ev_UnixGnomeToolbar.h"

// hack until gal 0.3 comes out with my c++ changes
extern "C" {
#include <gal/widgets/gtk-combo-text.h>
}

#define NUM_TOOLBARS_PER_APP 3

int EV_UnixGnomeToolbar::nbBands = 0;

/*****************************************************************/
#define COMBO_BUF_LEN 256

class _wd								// a private little class to help
{										// us remember all the widgets that
public:									// we create...
	_wd(EV_UnixGnomeToolbar * pUnixToolbar, XAP_Toolbar_Id id, GtkWidget * widget = NULL)
	{
		m_pUnixToolbar = pUnixToolbar;
		m_id = id;
		m_widget = widget;
		m_blockSignal = false;
		m_comboEntryBuffer[0] = 0;
	}
	
	~_wd(void)
	{
	}

	EV_UnixGnomeToolbar *	m_pUnixToolbar;
	XAP_Toolbar_Id		m_id;
	GtkWidget *			m_widget;
	bool				m_blockSignal;

	char 				m_comboEntryBuffer[1024];
};

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
static void
toolbar_append_with_eventbox (GtkToolbar *toolbar, GtkWidget  *widget,
			      const char *tooltip_text,
			      const char *tooltip_private_text)
{
	GtkWidget *eventbox;

	g_return_if_fail (GTK_IS_TOOLBAR (toolbar));
	g_return_if_fail (widget != NULL);

	/* An event box to receive events - this is a requirement for having
           tooltips */
	eventbox = gtk_event_box_new ();
	gtk_widget_show (widget);
	gtk_container_add (GTK_CONTAINER (eventbox), widget);
	gtk_widget_show (eventbox);
	gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), eventbox,
				   tooltip_text, tooltip_private_text);
}

static int s_callback(GtkWidget * /* widget */, gpointer user_data)
{
        // map the user_data into an object and dispatch the event.
  
        _wd * wd = (_wd *) user_data;
	UT_ASSERT(wd);
  
	if (!wd->m_blockSignal)
	  wd->m_pUnixToolbar->toolbarEvent(wd, 0, 0);

	return 0;
}

static int s_combo_changed(GtkWidget * widget, gpointer user_data)
{
        _wd * wd = (_wd *) user_data;
	UT_ASSERT(wd);

	//UT_DEBUGMSG(("s_combo_changed\n"));
	
	// only act if the widget has been shown and embedded in the toolbar
	if (wd->m_widget)
	  {
	    // if the popwin is still shown, this is a copy run and widget has a ->parent
	    if (widget->parent)
	      {
		// block is only honored here
		if (!wd->m_blockSignal)
		  {
		    gchar * buffer = gtk_entry_get_text(GTK_ENTRY(widget));
		    UT_uint32 length = strlen(buffer);
		    UT_ASSERT(length > 0);
		    UT_ASSERT(length < 1024);
		    strcpy(wd->m_comboEntryBuffer, buffer);
		    wd->m_pUnixToolbar->toolbarEvent(wd, (UT_UCSChar *)wd->m_comboEntryBuffer, length);
		  }
	      }
	    else // widget has no ->parent, so use the buffer's results
	      {
		UT_uint32 size = strlen(wd->m_comboEntryBuffer);
		
		// TODO : do a real conversion to UT_UCSChar or figure out the casting
		
		// don't do changes for empty combo texts
		if (UT_stricmp(wd->m_comboEntryBuffer, ""))
		  {
		    UT_UCSChar * text = (UT_UCSChar *) wd->m_comboEntryBuffer;
		    UT_ASSERT(text);		    
		    wd->m_pUnixToolbar->toolbarEvent(wd, text, size);
		  }
	      }
	  }
	return 0;
}

/*
 * Some toolbar items are too damn wide to put into the toolbar
 * if it is vertical.
 */
static void
format_toolbar_orient (GtkToolbar *toolbar,
		       GtkOrientation dir,
		       gpointer closure)
{
     EV_UnixGnomeToolbar *tb = (EV_UnixGnomeToolbar *)closure;
     tb->_orient_changed(toolbar, dir);
}

/*****************************************************************/

EV_UnixGnomeToolbar::EV_UnixGnomeToolbar(XAP_UnixGnomeApp * pUnixApp, XAP_UnixGnomeFrame * pUnixFrame,
										 const char * szToolbarLayoutName,
										 const char * szToolbarLabelSetName)
	: EV_UnixToolbar(static_cast<XAP_UnixApp *> (pUnixApp), static_cast<XAP_UnixFrame *> (pUnixFrame),
					 szToolbarLayoutName,
					 szToolbarLabelSetName),
			   nbToolbarsInBand(0)
{
}

EV_UnixGnomeToolbar::~EV_UnixGnomeToolbar(void)
{
}

void
EV_UnixGnomeToolbar::_orient_changed(GtkToolbar *toolbar,
				     GtkOrientation dir)
{
        GtkWidget *w = NULL;
	_wd *wd;
	int i = 0;
	int len = m_vecToolbarWidgets.getItemCount();

	// hack - we want to hide the large widgets
	// if we get sent vertical, and show them again
	// when we go horizontal

	// [show | hide] the large widgets
	for(i = 0; i < len; i++)
	  {
	    wd = (_wd*)m_vecToolbarWidgets.getNthItem(i);
	    w = wd->m_widget;
	    if (GTK_IS_COMBO_TEXT (w))
	      {
		if (dir == GTK_ORIENTATION_HORIZONTAL)
		  gtk_widget_show (w);
		else // dir == GTK_ORIENTATION_VERTICAL
		  gtk_widget_hide (w);
	      }
	  }
}

UT_Bool EV_UnixGnomeToolbar::synthesize(void)
{
	// create a GTK toolbar from the info provided.
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pUnixApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	XAP_Toolbar_ControlFactory * pFactory = m_pUnixApp->getControlFactory();
	UT_ASSERT(pFactory);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	GtkWidget * wTLW = m_pUnixFrame->getTopLevelWindow();

	m_wToolbar = _makeToolbar ();

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
					UT_Bool bFoundIcon =
						m_pUnixToolbarIcons->getPixmapForIcon(wTLW->window,
															  &wTLW->style->bg[GTK_STATE_NORMAL],
															  pLabel->getIconName(),
															  &wPixmap);
					UT_ASSERT(bFoundIcon);

					wd->m_widget = gtk_toolbar_append_item(GTK_TOOLBAR(m_wToolbar),
														   pLabel->getToolbarLabel(),
														   szToolTip,(const char *)NULL,
														   wPixmap,
														   GTK_SIGNAL_FUNC(s_callback),
														   wd);
				}
				break;

			case EV_TBIT_ToggleButton:
			case EV_TBIT_GroupButton:
				{
					UT_ASSERT(UT_stricmp(pLabel->getIconName(),"NoIcon")!=0);
					GtkWidget * wPixmap;
					UT_Bool bFoundIcon =
						m_pUnixToolbarIcons->getPixmapForIcon(wTLW->window,
															  &wTLW->style->bg[GTK_STATE_NORMAL],
															  pLabel->getIconName(),
															  &wPixmap);
					UT_ASSERT(bFoundIcon);

					wd->m_widget = gtk_toolbar_append_element(GTK_TOOLBAR(m_wToolbar),
															  GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
															  (GtkWidget *)NULL,
															  pLabel->getToolbarLabel(),
															  szToolTip,(const char *)NULL,
															  wPixmap,
															  GTK_SIGNAL_FUNC(s_callback),
															  wd);
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

				// populate it
				if (pControl)
				{
				        iWidth = pControl->getPixelWidth();
					pControl->populate();

					const UT_Vector * v = pControl->getContents();
					UT_ASSERT(v);

					if (v)
					  {
					        UT_uint32 items = v->getItemCount();

						GtkWidget * comboBox;
						
						// aesthetic hack
						if(items >= 12)
						  comboBox = gtk_combo_text_new (TRUE);
						else
						  comboBox = gtk_combo_text_new (FALSE);

						if (!gnome_preferences_get_toolbar_relief_btn ())
						  gtk_combo_box_set_arrow_relief (GTK_COMBO_BOX (comboBox), GTK_RELIEF_NONE);

						GtkWidget *entry = GTK_COMBO_TEXT (comboBox)->entry;
						
				                // the entry is read-only for now
						gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);

				                // i think that having the tooltip as title will be fine
						gtk_combo_box_set_title (GTK_COMBO_BOX (comboBox), szToolTip);
						gtk_container_set_border_width (GTK_CONTAINER (comboBox), 0);
						
				                // set the size of the entry to set the total combo size
						gtk_widget_set_usize(entry, iWidth, 0);
					       
				                // give a final show
						gtk_widget_show(comboBox);

						// populate it
						for (UT_uint32 m=0; m < items; m++)
						{
						  char * sz = (char *)v->getNthItem(m);
						  gtk_combo_text_add_item(GTK_COMBO_TEXT (comboBox), sz, sz);
						}

						gtk_signal_connect (GTK_OBJECT (entry), "changed",
								    GTK_SIGNAL_FUNC (s_combo_changed), wd);

				                // stick it in the toolbar
						toolbar_append_with_eventbox(GTK_TOOLBAR(m_wToolbar),
									     comboBox,
									     szToolTip,
									     (const char *)NULL);
						wd->m_widget = comboBox;
					}
				}

				// for now, we never repopulate, so can just toss it
				DELETEP(pControl);
			}
			break;
					
			case EV_TBIT_StaticLabel:
				// TODO do these...
				break;
					
			case EV_TBIT_Spacer:
				break;
					
			case EV_TBIT_BOGUS:
			default:
				UT_ASSERT(0);
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

	/* Handle orientation changes so that we can hide wide widgets */
	gtk_signal_connect (
		GTK_OBJECT(m_wToolbar), "orientation-changed",
		GTK_SIGNAL_FUNC (format_toolbar_orient), this);

	// show the complete thing
	gtk_widget_show(m_wToolbar);

	// add the toolbar to the band
	_addToolbar(m_wToolbar);

	return UT_TRUE;
}

UT_Bool EV_UnixGnomeToolbar::_addToolbar (GtkWidget *toolbar)
{
	char *name = 0;
	int beh;

	// an arbitrary padding to make our document not run into our buttons
	gtk_container_set_border_width(GTK_CONTAINER(toolbar), 2);
	beh = GNOME_DOCK_ITEM_BEH_NORMAL;

	if (!gnome_preferences_get_toolbar_detachable())
		beh |= GNOME_DOCK_ITEM_BEH_LOCKED;

	name = g_strdup_printf("Abi-Toolbar-%d", nbBands);
	//UT_DEBUGMSG(("DOM: beh:%d name:%s num:%d\n", beh, name,(nbBands % NUM_TOOLBARS_PER_APP)+1));

	//	g_print ("Toolbar style = %d\n", static_cast<int> (toolbar->style));
	gnome_app_add_toolbar(GNOME_APP(m_pUnixFrame->getTopLevelWindow()),
			      GTK_TOOLBAR (toolbar), name, (GnomeDockItemBehavior)beh, GNOME_DOCK_TOP,
			      (nbBands % NUM_TOOLBARS_PER_APP)+1, 0, 0);
	m_vecToolbars.addItem(toolbar);
	g_free(name);

	// increment the number of bands
	nbBands++;

	return UT_TRUE;
}

GtkWidget *EV_UnixGnomeToolbar::_makeToolbar(void)
{

	////////////////////////////////////////////////////////////////
	// get toolbar button appearance from the preferences
	////////////////////////////////////////////////////////////////
	const XML_Char * szValue = NULL;
	static GtkToolbarStyle style = GTK_TOOLBAR_ICONS;
	static UT_Bool firstTime = UT_TRUE;
	GtkWidget *toolbar;

	if (firstTime)
	{
		firstTime = UT_FALSE;
		m_pUnixApp->getPrefsValue(XAP_PREF_KEY_ToolbarAppearance, &szValue);
		UT_ASSERT((szValue) && (*szValue));
		
		if (UT_XML_stricmp(szValue,"text") == 0)
			style = GTK_TOOLBAR_TEXT;
		else if (UT_XML_stricmp(szValue,"both") == 0)
			style = GTK_TOOLBAR_BOTH;
		else
	  	        style = GTK_TOOLBAR_ICONS; // default case
	}
		
	toolbar = gtk_toolbar_new(GTK_ORIENTATION_HORIZONTAL, style);
	UT_ASSERT(GTK_TOOLBAR (toolbar));

	gtk_toolbar_set_button_relief(GTK_TOOLBAR (toolbar), GTK_RELIEF_NONE);
	gtk_toolbar_set_tooltips(GTK_TOOLBAR (toolbar), TRUE);
	gtk_toolbar_set_space_style(GTK_TOOLBAR (toolbar), GTK_TOOLBAR_SPACE_LINE);

	// TODO: this really shouldn't be determined by us
	gtk_toolbar_set_space_size(GTK_TOOLBAR (toolbar), 10);

	return toolbar;
}

void EV_UnixGnomeToolbar::show(void)
{
	if(m_wToolbar)
	  gtk_widget_show(m_wToolbar->parent);
}

void EV_UnixGnomeToolbar::hide(void)
{
	if(m_wToolbar)
	  {
	    gtk_widget_hide(m_wToolbar->parent);
	    gtk_widget_queue_resize (m_wToolbar->parent->parent);
	  }
}

// TODO: Dom, we copy *far* too much code from the GTK version here
// TODO: Dom, when all we change is the combo-text case
// TODO: Dom, change that
UT_Bool EV_UnixGnomeToolbar::refreshToolbar(AV_View * pView, AV_ChangeMask mask)
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
			continue;						       	// changes of this type, skip it...

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
			  UT_Bool bGrayed = EV_TIS_ShouldBeGray(tis);
			  
			  _wd * wd = (_wd *) m_vecToolbarWidgets.getNthItem(k);
			  UT_ASSERT(wd);
			  GtkButton * item = GTK_BUTTON(wd->m_widget);
			  UT_ASSERT(item);
			  
			  // Disable/enable toolbar item
			  gtk_widget_set_sensitive(GTK_WIDGET(item), !bGrayed);
			  
			  //UT_DEBUGMSG(("refreshToolbar: PushButton [%s] is %s\n",
			  //			 m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
			  //			 ((bGrayed) ? "disabled" : "enabled")));
			}
			break;
			
		      case EV_TBIT_ToggleButton:
		      case EV_TBIT_GroupButton:
			{
			  //UT_Bool bGrayed = EV_TIS_ShouldBeGray(tis);
			  UT_Bool bToggled = EV_TIS_ShouldBeToggled(tis);
			  
			  _wd * wd = (_wd *) m_vecToolbarWidgets.getNthItem(k);
			  UT_ASSERT(wd);
			  GtkToggleButton * item = GTK_TOGGLE_BUTTON(wd->m_widget);
			  UT_ASSERT(item);
			  
			  // Block the signal, throw the toggle event
			  bool wasBlocked = wd->m_blockSignal;
			  wd->m_blockSignal = true;
			  gtk_toggle_button_set_active(item, bToggled);
			  //gtk_toggle_button_toggled(item);
			  wd->m_blockSignal = wasBlocked;
			  
			  // Disable/enable toolbar item
			  //gtk_widget_set_sensitive(GTK_WIDGET(item), !bGrayed);
			  
			  //UT_DEBUGMSG(("refreshToolbar: ToggleButton [%s] is %s and %s\n",
			  //			 m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
			  //			 ((bGrayed) ? "disabled" : "enabled"),
			  //			 ((bToggled) ? "pressed" : "not pressed")));
			}
			break;
			
		      case EV_TBIT_EditText:
			break;
		      case EV_TBIT_DropDown:
			break;
		      case EV_TBIT_ComboBox:
			{
			  UT_Bool bGrayed = EV_TIS_ShouldBeGray(tis);
			  //UT_Bool bString = EV_TIS_ShouldUseString(tis);
			  
			  _wd * wd = (_wd *) m_vecToolbarWidgets.getNthItem(k);
			  UT_ASSERT(wd);
			  GtkComboText * item = GTK_COMBO_TEXT(wd->m_widget);
			  UT_ASSERT(item);
			  
			  // Disable/enable toolbar item
			  gtk_widget_set_sensitive(GTK_WIDGET(item), !bGrayed);
			  
			  // Block the signal, set the contents
			  bool wasBlocked = wd->m_blockSignal;
			  wd->m_blockSignal = true;
			  if (szState)
			    gtk_entry_set_text(GTK_ENTRY(item->entry), szState);
			  else
			    gtk_entry_set_text(GTK_ENTRY(item->entry), "");
			  wd->m_blockSignal = wasBlocked;
			  
			  //UT_DEBUGMSG(("refreshToolbar: ComboBox [%s] is %s and %s\n",
			  //			 m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
			  //			 ((bGrayed) ? "disabled" : "enabled"),
			  //			 ((bString) ? szState : "no state")));
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

	return UT_TRUE;
}
