/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
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
#include <stdlib.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ev_CocoaToolbar.h"
#include "xap_Types.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Layouts.h"
#include "ev_Toolbar_Labels.h"
#include "ev_Toolbar_Control.h"
#include "ev_EditEventMapper.h"
#include "xap_CocoaToolbar_Icons.h"
#include "ev_CocoaToolbar_ViewListener.h"
#include "xav_View.h"
#include "xap_Prefs.h"
#include "xap_EncodingManager.h"
#include "ap_CocoaFrame.h"

#import <Cocoa/Cocoa.h>
#import <AppKit/NSNibControlConnector.h>
#import "xap_CocoaToolbarWindow.h"

static	float	getButtonWidth ()
					{ return 34.0f; };
static	float	getButtonHeight ()
					{ return 34.0f; };
static	float	getButtonSpace () 
					{ return 1.0f; };
					
class _wd								// a private little class to help
{										// us remember all the widgets that
public:									// we create...
	_wd(EV_CocoaToolbar * pCocoaToolbar, XAP_Toolbar_Id id, NSControl * widget = nil);
	virtual ~_wd(void);

	EV_CocoaToolbar *	m_pCocoaToolbar;
	XAP_Toolbar_Id		m_id;
	NSControl *			m_widget;
};



@implementation EV_CocoaToolbarTarget
- (id)toolbarSelected:(id)sender
{
	UT_DEBUGMSG (("@EV_CocoaToolbarTarget (id)toolbarSelected:(id)sender\n"));
	
	if ([sender isKindOfClass:[NSButton class]]) {
		_wd* wd = (_wd*)[sender tag];
		wd->m_pCocoaToolbar->toolbarEvent (wd, NULL, 0);
	}
	else  if ([sender isKindOfClass:[NSComboBox class]]){
		_wd* wd = (_wd*)[sender tag];
		NSString * str = [sender stringValue];
		int numChars = [str length];
		UT_UCS2Char * data = NULL;
		size_t dataSize = 0;
	    if (wd->m_id == AP_TOOLBAR_ID_FMT_SIZE)
		{
		    UT_UCS2Char * data = (UT_UCS2Char*)strdup(XAP_EncodingManager::fontsizes_mapping.lookupByTarget([str cString]));
			dataSize = strlen((char *)data);
		}
		else
		{
			UT_UCS2Char * data = (UT_UCS2Char *)malloc (sizeof (UT_UCS2Char) * (numChars + 1));
			[str getCharacters:data];
			dataSize = numChars * sizeof(UT_UCS2Char);
		}
		wd->m_pCocoaToolbar->toolbarEvent (wd, data, dataSize);
		FREEP (data);
	}
	else {
		UT_DEBUGMSG (("Unexpected object class\n"));
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	}
	return sender;		// FIXME what should we return here ?
}
@end



_wd::_wd(EV_CocoaToolbar * pCocoaToolbar, XAP_Toolbar_Id tlbrid, NSControl * widget)
{
	m_pCocoaToolbar = pCocoaToolbar;
	m_id = tlbrid;
	m_widget = widget;
}
	
_wd::~_wd(void)
{
}


#if 0

	static void s_callback(GtkWidget * /* widget */, gpointer user_data)
	{
		// this is a static callback method and does not have a 'this' pointer.
		// map the user_data into an object and dispatch the event.
	
		_wd * wd = (_wd *) user_data;
		UT_ASSERT(wd);
		GdkEvent * event = gtk_get_current_event();
		wd->m_pCocoaToolbar->setCurrentEvent(event);
		UT_DEBUGMSG(("SEVIOR: in s_callback \n"));
		if (!wd->m_blockSignal)
			wd->m_pCocoaToolbar->toolbarEvent(wd, 0, 0);
	};


	static void s_drag_begin(GtkWidget  *widget,
							GdkDragContext     *context)
	{
		_wd * wd = (_wd *) g_object_get_data(G_OBJECT(widget),"wd_pointer");
		UT_DEBUGMSG(("SEVIOR: Begin drag at icon id %d \n",wd->m_id));
		XAP_Frame * pFrame = static_cast<XAP_Frame *>(wd->m_pCocoaToolbar->getFrame());
	    EV_Toolbar * pTBsrc = (EV_Toolbar *) wd->m_pCocoaToolbar;
		pFrame->dragBegin(wd->m_id,pTBsrc);
	};


	static void s_drag_drop(GtkWidget  *widget,
							GdkDragContext     *context,
							gint x, gint y, guint time )
	{
		_wd * wd = (_wd *) g_object_get_data(G_OBJECT(widget),"wd_pointer");
		GtkWidget * src = gtk_drag_get_source_widget(context);
		_wd * wdSrc = (_wd *)  g_object_get_data(G_OBJECT(src),"wd_pointer");
		UT_DEBUGMSG(("SEVIOR: Drop at icon id %d source icon %d \n",wd->m_id,wdSrc->m_id));
		
		XAP_Frame * pFrame = static_cast<XAP_Frame *>(wd->m_pCocoaToolbar->getFrame());
	    EV_Toolbar * pTBdest = (EV_Toolbar *) wd->m_pCocoaToolbar;
	    EV_Toolbar * pTBsrc = (EV_Toolbar *) wdSrc->m_pCocoaToolbar;
		pFrame->dragDropToIcon(wdSrc->m_id,wd->m_id,pTBsrc,pTBdest);
	};

	static void s_drag_drop_toolbar(GtkWidget  *widget,
							GdkDragContext     *context,
							gint x, gint y, guint time, gpointer pTB)
	{
		GtkWidget * src = gtk_drag_get_source_widget(context);
		_wd * wdSrc = (_wd *)  g_object_get_data(G_OBJECT(src),"wd_pointer");
		UT_DEBUGMSG(("SEVIOR: Drop  icon on toolbar source icon %d \n",wdSrc->m_id));
		XAP_Frame * pFrame = static_cast<XAP_Frame *>(wdSrc->m_pCocoaToolbar->getFrame());
	    EV_Toolbar * pTBsrc = (EV_Toolbar *) wdSrc->m_pCocoaToolbar;
	    EV_Toolbar * pTBdest = (EV_Toolbar *) pTB;
		pFrame->dragDropToTB(wdSrc->m_id,pTBsrc,pTBdest);
	};

	static void s_drag_end(GtkWidget  *widget,
							GdkDragContext     *context)
	{
		_wd * wd = (_wd *) g_object_get_data(G_OBJECT(widget),"wd_pointer");
		UT_DEBUGMSG(("SEVIOR: End drag of icon id %d \n",wd->m_id));
		XAP_Frame * pFrame = static_cast<XAP_Frame *>(wd->m_pCocoaToolbar->getFrame());
		pFrame->dragEnd(wd->m_id);
	};

	static void s_ColorCallback(GtkWidget * /* widget */, gpointer user_data)
	{
		// this is a static callback method and does not have a 'this' pointer.
		// map the user_data into an object and dispatch the event.

//
// This is hardwired to popup the color picker dialog
//	
		_wd * wd = (_wd *) user_data;
		UT_ASSERT(wd);

		if (!wd->m_blockSignal)
		{
			XAP_Toolbar_Id id = wd->m_id;
			XAP_CocoaApp * pCocoaApp = wd->m_pCocoaToolbar->getApp();
			const EV_EditMethodContainer * pEMC = pCocoaApp->getEditMethodContainer();
			UT_ASSERT(pEMC);
			EV_EditMethod * pEM = NULL;
			UT_DEBUGMSG(("SEVIOR: toolbar ID %d forground ID number %d \n",id,EV_TBIT_ColorFore));
			AV_View * pView = wd->m_pCocoaToolbar->getFrame()->getCurrentView();

			if(id ==  AP_TOOLBAR_ID_COLOR_FORE)
				pEM = pEMC->findEditMethodByName("dlgColorPickerFore");
			else
			    pEM = pEMC->findEditMethodByName("dlgColorPickerBack");
			wd->m_pCocoaToolbar->invokeToolbarMethod(pView,pEM,NULL,0);
		}				
	};

	// TODO: should this move out of wd?  It's convenient here; maybe I'll make
	// a microclass for combo boxes.
	static void s_combo_changed(GtkWidget * widget, gpointer user_data)
	{
		_wd * wd = (_wd *) user_data;
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
					gchar * buffer = gtk_entry_get_text(GTK_ENTRY(widget));
					UT_uint32 length = strlen(buffer);
					UT_ASSERT(length > 0);
					UT_ASSERT(length < 1024);
					strcpy(wd->m_comboEntryBuffer, buffer);
				}
			}
			else // widget has no ->parent, so use the buffer's results
			{

				// TODO : do a real conversion to UT_UCS2Char or figure out the casting

				// don't do changes for empty combo texts
				if (UT_strcmp(wd->m_comboEntryBuffer, ""))
				{
					UT_UCS2Char * text = (UT_UCS2Char *) 
					    (wd->m_id == AP_TOOLBAR_ID_FMT_SIZE ? 
					    XAP_EncodingManager::fontsizes_mapping.lookupByTarget(wd->m_comboEntryBuffer) :
					    wd->m_comboEntryBuffer);
					
					UT_ASSERT(text);					
					wd->m_pCocoaToolbar->toolbarEvent(wd, text, strlen((char*)text));
				}
			}
		}
			
	};

	// unblock when the menu goes away
	static void s_combo_hide(GtkWidget * widget, gpointer user_data)
	{
		UT_ASSERT(user_data);

		// manually force an update
		s_combo_changed(widget, user_data);
	}
#endif

/*****************************************************************/


EV_CocoaToolbar::EV_CocoaToolbar(XAP_CocoaApp * pCocoaApp, AP_CocoaFrame * pCocoaFrame,
							   const char * szToolbarLayoutName,
							   const char * szToolbarLabelSetName)
	: EV_Toolbar(pCocoaApp->getEditMethodContainer(),
				 szToolbarLayoutName,
				 szToolbarLabelSetName),
	 m_pCocoaApp (pCocoaApp), m_pCocoaFrame(pCocoaFrame)
{
	m_pViewListener = 0;
	m_wToolbar = nil;
	m_lid = 0;							// view listener id
	m_target = [[EV_CocoaToolbarTarget alloc] init];
}

EV_CocoaToolbar::~EV_CocoaToolbar(void)
{
	UT_VECTOR_PURGEALL(_wd *,m_vecToolbarWidgets);
	_releaseListener();
	UT_ASSERT ([m_target retainCount] == 1);
	[m_target release];
}


NSButton * EV_CocoaToolbar::_makeToolbarButton (int type, EV_Toolbar_Label * pLabel, _wd * wd, NSView *parent,
												float & btnX)
{
	const float BTN_WIDTH = getButtonWidth ();
	const float BTN_HEIGHT = getButtonHeight ();
	const float BTN_SPACE = getButtonSpace ();

	NSButton * btn = nil;
	
	NSRect btnFrame;
	btnFrame.origin.x = btnX;
	btnFrame.origin.y = BTN_SPACE;
	btnFrame.size.width = BTN_WIDTH;
	btnFrame.size.height = BTN_HEIGHT;
	btnX += BTN_WIDTH + BTN_SPACE;
	
	btn = [[NSButton alloc] initWithFrame:btnFrame];
	switch (type) {
	case EV_TBIT_PushButton:
		/*[btn setButtonType:NSToggleButton];*/
		[btn setButtonType:NSMomentaryPushInButton]; 
		break;
	case EV_TBIT_ToggleButton:
	case EV_TBIT_GroupButton:
		[btn setButtonType:NSPushOnPushOffButton];
		break;
	default:
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	}
	[btn setBezelStyle:NSRegularSquareBezelStyle];
	UT_ASSERT(UT_stricmp(pLabel->getIconName(),"NoIcon")!=0);
	NSImage * wPixmap;
	bool bFoundIcon = m_pCocoaToolbarIcons->getPixmapForIcon(pLabel->getIconName(),
																&wPixmap);
	UT_ASSERT(bFoundIcon);
	[btn setImage:wPixmap];
	[wPixmap release];
	[parent addSubview:btn];
	wd->m_widget = btn;
	[btn setTag:(int)wd];
	NSNibControlConnector * conn = [[NSNibControlConnector alloc] init];
	[conn setLabel:@"toolbarSelected:"];
	[conn setDestination:m_target];
	[conn setSource:btn];
	[conn establishConnection];
	[conn release];
	return btn;
}

bool EV_CocoaToolbar::toolbarEvent(_wd * wd,
									 UT_UCS2Char * pData,
									 UT_uint32 dataLength)

{
	// user selected something from this toolbar.
	// invoke the appropriate function.
	// return true iff handled.

	XAP_Toolbar_Id tlbrid = wd->m_id;

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pCocoaApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	const EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(tlbrid);
	UT_ASSERT(pAction);

	AV_View * pView = m_pCocoaFrame->getCurrentView();

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
			
			UT_DEBUGMSG(("TODO handle toolbar toggling\n"));
			UT_ASSERT (UT_TODO);
		#if 0
			// Block the signal, throw the button back up/down
			bool wasBlocked = wd->m_blockSignal;
			wd->m_blockSignal = true;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wd->m_widget),
										 !GTK_TOGGLE_BUTTON(wd->m_widget)->active);
			//gtk_toggle_button_toggled(item);
			wd->m_blockSignal = wasBlocked;
		#endif
			// can safely ignore this event
			return true;
		}
	}

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return false;
	
	const EV_EditMethodContainer * pEMC = m_pCocoaApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	// Now we'd better convert pData into a UT_UCS4Char.
	UT_UCS4Char * pData4 = (UT_UCS4Char*) UT_convert((char*)pData,
		dataLength, "UCS-2", "UCS-4", NULL, NULL);
	invokeToolbarMethod(pView,pEM,pData4,dataLength);
	free(pData4);
	return true;
}


/*!
 * This method destroys the container widget here and returns the position in
 * the overall vbox container.
 */
UT_sint32 EV_CocoaToolbar::destroy(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
	return 0;
#if 0
	GtkWidget * wVBox = m_pCocoaFrame->getVBoxWidget();
	UT_sint32  pos = 0;
//
// Code gratutiously stolen from gtkbox.c
//
	GList *list = NULL;
	bool bFound = false;
	for( list = GTK_BOX(wVBox)->children; !bFound && list; list = list->next)
	{
		GtkBoxChild * child = (GtkBoxChild *) list->data;
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
#endif
}

/*!
 * This method rebuilds the toolbar and places it in the position it previously
 * occupied.
 */
void EV_CocoaToolbar::rebuildToolbar(UT_sint32 oldpos)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
#if 0
  //
  // Build the toolbar, place it in a handlebox at an arbitary place on the 
  // the frame.
  //
    synthesize();
	GtkWidget * wVBox = m_pCocoaFrame->getVBoxWidget();
	gtk_box_reorder_child(GTK_BOX(wVBox), m_wHandleBox,oldpos);
//
// bind  view listener
//
	AV_View * pView = getFrame()->getCurrentView();
	bindListenerToView(pView);
#endif
}


bool EV_CocoaToolbar::synthesize(void)
{
	// TODO: rationalize those as static members of the class.
//	const float BTN_WIDTH = getButtonWidth ();
	const float BTN_HEIGHT = getButtonHeight ();
	const float BTN_SPACE = getButtonSpace ();

	// create a Cocoa toolbar from the info provided.
	float btnX = BTN_SPACE;
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pCocoaApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	XAP_Toolbar_ControlFactory * pFactory = m_pCocoaApp->getControlFactory();
	UT_ASSERT(pFactory);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	XAP_CocoaToolbarWindow * pToolbarWinCtrl = [XAP_CocoaToolbarWindow sharedToolbar];
	UT_ASSERT (pToolbarWinCtrl);
	NSView * toolbarParent = [[pToolbarWinCtrl window] contentView];
	UT_ASSERT (toolbarParent);
	NSRect viewBounds = [toolbarParent bounds];
	float viewHeight = viewBounds.size.height;	// the toolbar window view height
	viewBounds.size.height = BTN_HEIGHT + BTN_SPACE;
	xxx_UT_DEBUGMSG (("toolbar has %u subviews\n", [[toolbarParent subviews] count]));
	// revert the coordinate as they are upside down in NSView
	viewBounds.origin.y = viewHeight - ([[toolbarParent subviews] count] + 1) * viewBounds.size.height;
	m_wToolbar = [[NSView alloc] initWithFrame:viewBounds];
	[toolbarParent addSubview:m_wToolbar];
	[m_wToolbar setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	
//	GtkWidget * wVBox = m_pCocoaFrame->getVBoxWidget();

//	m_wHandleBox = gtk_handle_box_new();
//	UT_ASSERT(m_wHandleBox);

	////////////////////////////////////////////////////////////////
	// get toolbar button appearance from the preferences
	////////////////////////////////////////////////////////////////
#if 0
	// TODO
	const XML_Char * szValue = NULL;
	m_pCocoaApp->getPrefsValue(XAP_PREF_KEY_ToolbarAppearance, &szValue);
	UT_ASSERT((szValue) && (*szValue));
	
	if (UT_XML_stricmp(szValue, "icon") == 0) {
		[toolbar setDisplayMode:NSToolbarDisplayModeIconOnly];
	}
	else if (UT_XML_stricmp(szValue, "text") == 0) {
		[toolbar setDisplayMode:NSToolbarDisplayModeLabelOnly];
	}
	else if (UT_XML_stricmp(szValue, "both") == 0) {
		[toolbar setDisplayMode:NSToolbarDisplayModeIconAndLabel];
	}
#endif

//	m_wToolbar = gtk_toolbar_new(GTK_ORIENTATION_HORIZONTAL, style);
//	UT_ASSERT(m_wToolbar);
	
//	gtk_toolbar_set_button_relief(GTK_TOOLBAR(m_wToolbar), GTK_RELIEF_NONE);
//	gtk_toolbar_set_tooltips(GTK_TOOLBAR(m_wToolbar), TRUE);
//	gtk_toolbar_set_space_size(GTK_TOOLBAR(m_wToolbar), 10);
//	gtk_toolbar_set_space_style(GTK_TOOLBAR (m_wToolbar), GTK_TOOLBAR_SPACE_LINE);

//
// Make the toolbar a destination for drops
//
#if 0		// TODO
	gtk_drag_dest_set(m_wToolbar,(GtkDestDefaults) GTK_DEST_DEFAULT_ALL,
					  s_AbiTBTargets,1,
					  GDK_ACTION_COPY);
	g_signal_connect(G_OBJECT(m_wToolbar),"drag_drop",G_CALLBACK(_wd::s_drag_drop_toolbar),this);
#endif
	NSNibControlConnector * conn = [[NSNibControlConnector alloc] init];
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);

		XAP_Toolbar_Id tlbrID = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(tlbrID);
		UT_ASSERT(pAction);
		EV_Toolbar_Label * pLabel = m_pToolbarLabelSet->getLabel(tlbrID);
		UT_ASSERT(pLabel);

		switch (pLayoutItem->getToolbarLayoutFlags())
		{
		case EV_TLF_Normal:
		{
			_wd * wd = new _wd(this,tlbrID);
			UT_ASSERT(wd);

			const char * szToolTip = pLabel->getToolTip();
			if (!szToolTip || !*szToolTip)
				szToolTip = pLabel->getStatusMsg();

			switch (pAction->getItemType())
			{
			case EV_TBIT_PushButton:
			{
				NSButton * btn;
				btn = _makeToolbarButton (EV_TBIT_PushButton, pLabel, wd, m_wToolbar, btnX);

#if 0 // TODO
				gtk_drag_source_set_icon(wwd,ClrMap ,pixmap,NULL);
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
					NSButton *btn;
					btn = _makeToolbarButton (pAction->getItemType(), pLabel, wd, m_wToolbar, btnX);
#if 0
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_begin",G_CALLBACK(_wd::s_drag_begin), wd);
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_drop",G_CALLBACK(_wd::s_drag_drop), wd);
				g_signal_connect(G_OBJECT(wd->m_widget),"drag_end",G_CALLBACK(_wd::s_drag_end), wd);
#endif
				}
				break;

			case EV_TBIT_EditText:
				break;
					
			case EV_TBIT_DropDown:
				break;
					
			case EV_TBIT_ComboBox:
			{
				EV_Toolbar_Control * pControl = pFactory->getControl(this, tlbrID);
				UT_ASSERT(pControl);

				// default, shouldn't be used for well-defined controls
				float fWidth = 100;

				if (pControl)
				{
					fWidth = pControl->getPixelWidth();
				}
				
				NSRect btnFrame;
				btnFrame.origin.x = btnX;
				btnFrame.size.width = fWidth;
				btnFrame.size.height = 24.0f; 
				btnFrame.origin.y = BTN_SPACE + ((BTN_HEIGHT - btnFrame.size.height) / 2);

				NSComboBox * comboBox = [[NSComboBox alloc] initWithFrame:btnFrame];
				UT_ASSERT(comboBox);
				[m_wToolbar addSubview:comboBox];
				wd->m_widget = comboBox;
				[comboBox setTag:(int)wd];
				[comboBox setEditable:NO];
				[conn setDestination:m_target];
				[conn setSource:comboBox];
				[conn setLabel:@"toolbarSelected:"];
				[conn establishConnection];
				btnX = [comboBox frame].size.width + [comboBox frame].origin.x + BTN_SPACE;
#if 0 //TODO

				// handle popup events, so we can block our signals until the popdown
				GtkWidget * popwin = GTK_WIDGET(GTK_COMBO(comboBox)->popwin);
				UT_ASSERT(popwin);

				g_signal_connect(G_OBJECT(popwin),
								   "hide",
								   G_CALLBACK(_wd::s_combo_hide),
								   wd);
				// take away the ability to gain focus
//				g_signal_connect(G_OBJECT(GTK_COMBO(comboBox)->entry),
//								   "focus_in_event",
//								   G_CALLBACK(_wd::s_combo_focus_in),
//								   wd);
//				g_signal_connect(G_OBJECT(comboBox),
//								   "key_press_event",
//								   G_CALLBACK(_wd::s_combo_key_press),
//								   wd);
//				g_signal_connect(G_OBJECT(GTK_COMBO(comboBox)->entry),
//								   "key_press_event",
//								   G_CALLBACK(_wd::s_combo_key_press),
//								   wd);
				
				// handle changes in content
				GtkEntry * blah = GTK_ENTRY(GTK_COMBO(comboBox)->entry);
				GtkEditable * yuck = GTK_EDITABLE(blah);
				g_signal_connect(G_OBJECT(&yuck->widget),
								   "changed",
								   G_CALLBACK(_wd::s_combo_changed),
								   wd);
#endif
				// populate it
				if (pControl)
				{
					pControl->populate();

					const UT_Vector * v = pControl->getContents();
					UT_ASSERT(v);

					if (v)
					{
						UT_uint32 items = v->getItemCount();
						for (UT_uint32 m=0; m < items; m++)
						{
							char * sz = (char *)v->getNthItem(m);
							
							NSString * str = [NSString stringWithCString:sz];	// autoreleased
							[comboBox addItemWithObjectValue:str];
						}
						if (items > 0) {
							[comboBox selectItemAtIndex:0];
							[comboBox setObjectValue:[comboBox objectValueOfSelectedItem]];
						}
					}
				}
				
		
#if 0
				GtkWidget * evBox = toolbar_append_with_eventbox(GTK_TOOLBAR(m_wToolbar),
									     comboBox,
									     szToolTip,
									     (const char *)NULL);
				wd->m_widget = comboBox;
//
// Add in a right drag method
//
				GtkWidget * wwd = wd->m_widget;
				g_object_set_data(G_OBJECT(wwd),
									"wd_pointer",
									wd);
				gtk_drag_source_set(evBox,GDK_BUTTON3_MASK,
									s_AbiTBTargets,1,
									GDK_ACTION_COPY);
//				GdkColormap * ClrMap = gtk_widget_get_colormap (wwd);
//				GdkPixmap * pixmap = GTK_PIXMAP(wPixmap)->pixmap;
//				GdkBitmap * bitmap = GTK_PIXMAP(wPixmap)->mask;
//				gtk_drag_source_set_icon(wwd,ClrMap ,pixmap,bitmap);
				gtk_drag_dest_set(evBox,(GtkDestDefaults) GTK_DEST_DEFAULT_ALL,
									s_AbiTBTargets,1,
									GDK_ACTION_COPY);
				g_signal_connect(G_OBJECT(evBox),"drag_begin",G_CALLBACK(_wd::s_drag_begin), wd);
				g_signal_connect(G_OBJECT(evBox),"drag_drop",G_CALLBACK(_wd::s_drag_drop), wd);
				g_signal_connect(G_OBJECT(evBox),"drag_end",G_CALLBACK(_wd::s_drag_end), wd);

#endif
				// for now, we never repopulate, so can just toss it
				DELETEP(pControl);
			}
			break;

			case EV_TBIT_ColorFore:
			case EV_TBIT_ColorBack:
			{
				NSButton * btn;
				btn = _makeToolbarButton (EV_TBIT_PushButton, pLabel, wd, m_wToolbar, btnX);
			}
			break;
				
			case EV_TBIT_StaticLabel:
				// TODO do these...
				break;
					
			case EV_TBIT_Spacer:
				break;
					
			case EV_TBIT_BOGUS:
			default:
				UT_DEBUGMSG(("FIXME: Need GTK color picker for the toolbar \n"));
//				UT_ASSERT(0);
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
			_wd * wd = new _wd(this, tlbrID);
			UT_ASSERT(wd);
			m_vecToolbarWidgets.addItem(wd);

			UT_DEBUGMSG (("TODO: Implement spacer.\n"));
			break;
		}

		default:
			UT_ASSERT(0);
		}
	}
	UT_ASSERT ([conn retainCount] == 1);
	[conn release];

	//TODO here we should add the toolbar
	return true;
}

void EV_CocoaToolbar::_releaseListener(void)
{
	if (!m_pViewListener)
		return;
	DELETEP(m_pViewListener);
	m_pViewListener = 0;
	m_lid = 0;
}
	
bool EV_CocoaToolbar::bindListenerToView(AV_View * pView)
{
	_releaseListener();
	
	m_pViewListener = new EV_CocoaToolbar_ViewListener(this,pView);
	UT_ASSERT(m_pViewListener);

	bool bResult = pView->addListener(static_cast<AV_Listener *>(m_pViewListener),&m_lid);
	UT_ASSERT(bResult);

	if (pView->isDocumentPresent()) {
		refreshToolbar(pView, AV_CHG_ALL);
	}
	return true;
}

bool EV_CocoaToolbar::refreshToolbar(AV_View * pView, AV_ChangeMask mask)
{
	// make the toolbar reflect the current state of the document
	// at the current insertion point or selection.
	
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pCocoaApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);

		XAP_Toolbar_Id tlbrid = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(tlbrid);
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
				case EV_TBIT_ColorFore:
				case EV_TBIT_ColorBack:
				{
					bool bGrayed = EV_TIS_ShouldBeGray(tis);

					_wd * wd = (_wd *) m_vecToolbarWidgets.getNthItem(k);
					UT_ASSERT(wd);
					NSButton * item = wd->m_widget;
					UT_ASSERT(item);
					UT_ASSERT([item isKindOfClass:[NSButton class]]);
					// Disable/enable toolbar item
					[item setEnabled:(bGrayed?NO:YES)];
     					
					//UT_DEBUGMSG(("refreshToolbar: PushButton [%s] is %s\n",
					//			 m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(),
					//			 ((bGrayed) ? "disabled" : "enabled")));
				}
				break;
			
				case EV_TBIT_ToggleButton:
				case EV_TBIT_GroupButton:
				{
					bool bGrayed = EV_TIS_ShouldBeGray(tis);
					bool bToggled = EV_TIS_ShouldBeToggled(tis);

					_wd * wd = (_wd *) m_vecToolbarWidgets.getNthItem(k);
					UT_ASSERT(wd);
					NSButton * item = wd->m_widget;
					UT_ASSERT(item);
					UT_ASSERT([item isKindOfClass:[NSButton class]]);						
					[item setState:(bToggled?NSOnState:NSOffState)];
						
					// Disable/enable toolbar item
					[item setEnabled:(bGrayed?NO:YES)];
						
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
					bool bGrayed = EV_TIS_ShouldBeGray(tis);
					
					_wd * wd = (_wd *) m_vecToolbarWidgets.getNthItem(k);
					UT_ASSERT(wd);
					NSComboBox * item = wd->m_widget;
					UT_ASSERT(item);
					UT_ASSERT([item isKindOfClass:[NSComboBox class]]);
					// Disable/enable toolbar item
					[item setEnabled:(bGrayed?NO:YES)];
					NSString* value;
					if (wd->m_id==AP_TOOLBAR_ID_FMT_SIZE) {
					    value = [NSString stringWithCString:XAP_EncodingManager::fontsizes_mapping.lookupBySource(szState)];
					}
					else {
						value = [NSString stringWithCString:szState];
					}

					[item selectItemWithObjectValue:value];
				}
				break;

				case EV_TBIT_StaticLabel:
					break;
				case EV_TBIT_Spacer:
					break;
				case EV_TBIT_BOGUS:
					break;
				default:
					UT_DEBUGMSG (("Toolbar layout flags are: %d It is unexpected !!!\n", pLayoutItem->getToolbarLayoutFlags()));
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

XAP_CocoaApp * EV_CocoaToolbar::getApp(void)
{
	return m_pCocoaApp;
}

AP_CocoaFrame * EV_CocoaToolbar::getFrame(void)
{
	return m_pCocoaFrame;
}

void EV_CocoaToolbar::show(void)
{
	if ([m_wToolbar superview] == nil) {
		[m_superView addSubview:m_wToolbar];
		UT_ASSERT ([m_wToolbar retainCount] > 1);
		[m_wToolbar release];
	}
}

void EV_CocoaToolbar::hide(void)
{
	if ([m_wToolbar superview] != nil) {
		m_superView = [m_wToolbar superview];
		UT_ASSERT (m_superView);
		[m_wToolbar retain];
		[m_wToolbar removeFromSuperview];
	}
}

/*!
 * This method examines the current document and repopulates the Styles
 * Combo box with what is in the document. It returns false if no styles 
 * combo box was found. True if it all worked.
 */
bool EV_CocoaToolbar::repopulateStyles(void)
{
//
// First off find the Styles combobox in a toolbar somewhere
//
	UT_uint32 count = m_pToolbarLayout->getLayoutItemCount();
	UT_uint32 i =0;
	EV_Toolbar_LayoutItem * pLayoutItem = NULL;
	XAP_Toolbar_Id tlbrid = 0;
	_wd * wd = NULL;
	for(i=0; i < count; i++)
	{
		pLayoutItem = m_pToolbarLayout->getLayoutItem(i);
		tlbrid = pLayoutItem->getToolbarId();
		wd = (_wd *) m_vecToolbarWidgets.getNthItem(i);
		if(tlbrid == AP_TOOLBAR_ID_FMT_STYLE)
			break;
	}
	if(i>=count)
		return false;
//
// GOT IT!
//
	UT_ASSERT(wd->m_id == AP_TOOLBAR_ID_FMT_STYLE);
	XAP_Toolbar_ControlFactory * pFactory = m_pCocoaApp->getControlFactory();
	UT_ASSERT(pFactory);
	EV_Toolbar_Control * pControl = pFactory->getControl(this, tlbrid);
	AP_CocoaToolbar_StyleCombo * pStyleC = static_cast<AP_CocoaToolbar_StyleCombo *>(pControl);
	pStyleC->repopulate();
	NSComboBox * item = wd->m_widget;
//
// Now the combo box has to be refilled from this
//						
	const UT_Vector * v = pControl->getContents();
	UT_ASSERT(v);
//
// Now  we must remove and delete the old glist so we can attach the new
// list of styles to the combo box.
//
// Try this....
//
	[item removeAllItems];
//
// Now make a new one.
//
	UT_uint32 items = v->getItemCount();
	for (UT_uint32 m=0; m < items; m++)
	{
		char * sz = (char *)v->getNthItem(m);
		NSString * str = [NSString stringWithCString:sz];		//autorelased
		[item addItemWithObjectValue:str];
	}
	delete pStyleC;
//
// I think we've finished!
//
	return true;
}




