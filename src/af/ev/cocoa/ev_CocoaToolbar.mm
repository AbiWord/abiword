/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003 Hubert Figuiere
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
#include "xap_CocoaFrameImpl.h"

#import <Cocoa/Cocoa.h>
#import "xap_CocoaToolbarWindow.h"


@implementation EV_CocoaToolbarTarget

- (void)setXAPOwner:(EV_CocoaToolbar*)owner
{
	_xap = owner;
}


- (id)toolbarSelected:(id)sender
{
	UT_DEBUGMSG (("@EV_CocoaToolbarTarget (id)toolbarSelected:(id)sender\n"));
	
	if ([sender isKindOfClass:[NSButton class]]) {
		const UT_UCSChar * pData = NULL;
		UT_uint32 dataLength = 0;
		
		XAP_Toolbar_Id tlbrID = [sender tag];
		switch (tlbrID) {
		case AP_TOOLBAR_ID_COLOR_FORE:
		case AP_TOOLBAR_ID_COLOR_BACK:
			
			break;
		default:
			break;
		}
		_xap->toolbarEvent (tlbrID, pData, dataLength);
	}
	else  if ([sender isKindOfClass:[NSComboBox class]]){
		XAP_Toolbar_Id tlbrID = [sender tag];
		NSString * str = [sender stringValue];
		const char * text = NULL;
	    if (tlbrID == AP_TOOLBAR_ID_FMT_SIZE)
		{
		    text = UT_strdup(XAP_EncodingManager::fontsizes_mapping.lookupByTarget([str UTF8String]));
		}
		else
		{
			text = UT_strdup([str UTF8String]);
		}
		UT_UCS4String ucsText(text);
		_xap->toolbarEvent (tlbrID, ucsText.ucs4_str(), ucsText.length());
	}
	else {
		UT_DEBUGMSG (("Unexpected object class\n"));
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	}
	return sender;		// FIXME what should we return here ?
}
@end



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
	[m_target setXAPOwner:this];
}

EV_CocoaToolbar::~EV_CocoaToolbar(void)
{
	_releaseListener();
	UT_ASSERT ([m_target retainCount] == 1);
	[m_target release];
	[m_wToolbar release];
}


NSButton * EV_CocoaToolbar::_makeToolbarButton (int type, EV_Toolbar_Label * pLabel, 
												XAP_Toolbar_Id tlbrid, NSView *parent,
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
	NSImage * wPixmap = m_pCocoaToolbarIcons->getPixmapForIcon(pLabel->getIconName());		// autoreleased
	UT_ASSERT(wPixmap);
	[btn setImage:wPixmap];
	[parent addSubview:btn];
	[btn release];
	[btn setTag:(int)tlbrid];
	[btn setTarget:m_target];
	[btn setAction:@selector(toolbarSelected:)];
	return btn;
}

bool EV_CocoaToolbar::toolbarEvent(XAP_Toolbar_Id tlbrid,
									 const UT_UCSChar * pData,
									 UT_uint32 dataLength)

{
	// user selected something from this toolbar.
	// invoke the appropriate function.
	// return true iff handled.

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
			
			UT_DEBUGMSG(("TODO handle toolbar toggling\n"));
			UT_ASSERT (UT_TODO);
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

	invokeToolbarMethod(pView, pEM, pData, dataLength);
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

	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	XAP_CocoaToolbarWindow_Controller * pToolbarWinCtrl = [XAP_CocoaToolbarWindow_Controller sharedToolbar];
	UT_ASSERT (pToolbarWinCtrl);
	NSView * toolbarParent = [[pToolbarWinCtrl window] contentView];
	UT_ASSERT (toolbarParent);
	NSRect viewBounds = [toolbarParent bounds];
	float viewHeight = viewBounds.size.height;	// the toolbar window view height
	viewBounds.size.height = getToolbarHeight();
	xxx_UT_DEBUGMSG (("toolbar has %u subviews\n", [[toolbarParent subviews] count]));
	// revert the coordinate as they are upside down in NSView
	viewBounds.origin.y = viewHeight - ([[toolbarParent subviews] count] + 1) * viewBounds.size.height;
	m_wToolbar = [[NSView alloc] initWithFrame:viewBounds];
	[m_wToolbar setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
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

//
// Make the toolbar a destination for drops
//
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
			const char * szToolTip = pLabel->getToolTip();
			if (!szToolTip || !*szToolTip)
				szToolTip = pLabel->getStatusMsg();

			switch (pAction->getItemType())
			{
			case EV_TBIT_PushButton:
			{
				NSButton * btn;
				btn = _makeToolbarButton (EV_TBIT_PushButton, pLabel, tlbrID, m_wToolbar, btnX);
			}
			break;

			case EV_TBIT_ToggleButton:
			case EV_TBIT_GroupButton:
				{
					NSButton *btn;
					btn = _makeToolbarButton (pAction->getItemType(), pLabel, tlbrID, m_wToolbar, btnX);
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
				[comboBox setTag:(int)tlbrID];
				[comboBox setFont:[NSFont systemFontOfSize:[NSFont smallSystemFontSize]]];
				[comboBox setEditable:NO];
				[comboBox setTarget:m_target];
				[comboBox setAction:@selector(toolbarSelected:)];
				btnX += [comboBox frame].size.width + BTN_SPACE;
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
							
							NSString * str = [NSString stringWithUTF8String:sz];	// autoreleased
							[comboBox addItemWithObjectValue:str];
						}
						if (items > 0) {
							[comboBox selectItemAtIndex:0];
							[comboBox setObjectValue:[comboBox objectValueOfSelectedItem]];
						}
						//[comboBox setNumberOfVisibleItems:items];
					}
				}
				// for now, we never repopulate, so can just toss it
				DELETEP(pControl);
			}
			break;

			case EV_TBIT_ColorFore:
			case EV_TBIT_ColorBack:
			{
				NSButton * btn;
				btn = _makeToolbarButton (EV_TBIT_PushButton, pLabel, tlbrID, m_wToolbar, btnX);
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
		}
		break;
			
		case EV_TLF_Spacer:
		{
			// Append to the vector even if spacer, to sync up with refresh
			// which expects each item in the layout to have a place in the
			// vector.
			NSRect btnFrame;
			btnFrame.origin.x = btnX + 2.0f;
			btnFrame.size.width = 1.0f;
			btnFrame.size.height = BTN_HEIGHT; 
			btnFrame.origin.y = BTN_SPACE + ((BTN_HEIGHT - btnFrame.size.height) / 2);
			btnX += BTN_SPACE + btnFrame.size.width + 4.0f;
			
			NSBox * box = [[NSBox alloc] initWithFrame:btnFrame];
			UT_ASSERT(box);
			[box setBoxType:NSBoxSeparator];
			[m_wToolbar addSubview:box];
			[box release];
			break;
		}

		default:
			UT_ASSERT(0);
		}
	}

	[pool release];
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

					NSButton * item = [m_wToolbar viewWithTag:tlbrid];
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

					NSButton * item = [m_wToolbar viewWithTag:tlbrid];
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
					
					NSComboBox * item = [m_wToolbar viewWithTag:tlbrid];
					UT_ASSERT(item);
					UT_ASSERT([item isKindOfClass:[NSComboBox class]]);
					// Disable/enable toolbar item
					[item setEnabled:(bGrayed?NO:YES)];
					NSString* value = nil;
					if (tlbrid == AP_TOOLBAR_ID_FMT_SIZE) {
						const char *str = XAP_EncodingManager::fontsizes_mapping.lookupBySource(szState);
						if (str) {
							value = [[NSString alloc] initWithUTF8String:str];
						}
						else {
							UT_DEBUGMSG(("%s:%d fontSize not found.... !!!! FIXME", __FILE__, __LINE__));
							if (szState) {
								value = [[NSString alloc] initWithUTF8String:szState];
							}
							else {
								value = [[NSString alloc] initWithUTF8String:""];
							}
						}
					}
					else {
						if (szState) {
							value = [[NSString alloc] initWithUTF8String:szState];
						}
						else {
							UT_DEBUGMSG(("value is NULL\n"));
						}
					}
					if (value) {
						int idx = [item indexOfItemWithObjectValue:value];
						if (idx >= 0) {
							[item selectItemWithObjectValue:value];
						}
						else {
							[item setStringValue:value];
						}
						[value release];
					}
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
	EV_Toolbar::show();
	[[XAP_CocoaToolbarWindow_Controller sharedToolbar] redisplayToolbars:
		static_cast<XAP_CocoaFrameImpl*>(m_pCocoaFrame->getFrameImpl())->_getController()];
}

void EV_CocoaToolbar::hide(void)
{
	EV_Toolbar::hide();
	[[XAP_CocoaToolbarWindow_Controller sharedToolbar] redisplayToolbars:
		static_cast<XAP_CocoaFrameImpl*>(m_pCocoaFrame->getFrameImpl())->_getController()];
}

/*!
 * This method examines the current document and repopulates the Styles
 * Combo box with what is in the document. It returns false if no styles 
 * combo box was found. True if it all worked.
 */
bool EV_CocoaToolbar::repopulateStyles(void)
{
	XAP_Toolbar_ControlFactory * pFactory = m_pCocoaApp->getControlFactory();
	UT_ASSERT(pFactory);
	EV_Toolbar_Control * pControl = pFactory->getControl(this, AP_TOOLBAR_ID_FMT_STYLE);
	AP_CocoaToolbar_StyleCombo * pStyleC = static_cast<AP_CocoaToolbar_StyleCombo *>(pControl);
	pStyleC->repopulate();
	NSComboBox * item = [m_wToolbar viewWithTag:AP_TOOLBAR_ID_FMT_STYLE];
	if (item == nil) {
		delete pStyleC;
		return false;
	}
//
// Now the combo box has to be refilled from this
//						
	const UT_GenericVector<const char *> * v = pControl->getContents();
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
		const char * sz = v->getNthItem(m);
		NSString * str = [[NSString alloc] initWithUTF8String:sz];
		UT_ASSERT(str);
		if (str) {
			/* make sure the object is not nil or this raises an exception */
			[item addItemWithObjectValue:str];
			[str release];
		}
		else {
			NSLog(@"Discarded style name in combox box: '%s'", sz);
		}
	}
	delete pStyleC;
//
// I think we've finished!
//
	return true;
}




