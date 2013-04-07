/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003, 2009, 2011 Hubert Figuiere
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
#include "gr_CocoaCairoGraphics.h"
#include "pd_Style.h"

#import <Cocoa/Cocoa.h>
#import "xap_CocoaToolbarWindow.h"



@interface EV_CocoaToolbarTarget : NSObject
{
	EV_CocoaToolbar*	_xap;
}
- (void)dealloc;
- (void)setColor:(XAP_Toolbar_Id)tlbrid;
- (IBAction)aColor_FG:(id)sender;
- (IBAction)aColor_BG:(id)sender;
- (void)toolbarSelected:(id)sender;
- (void)setXAPOwner:(EV_CocoaToolbar*)owner;
@end


@implementation EV_CocoaToolbarTarget

- (void)dealloc
{
	NSColorPanel * colorPanel = [NSColorPanel sharedColorPanel];

	[colorPanel setAction:nil];
	[colorPanel setTarget:nil];
	[super dealloc];
}


- (void)setXAPOwner:(EV_CocoaToolbar*)owner
{
	_xap = owner;
}

- (void)setColor:(XAP_Toolbar_Id)tlbrid
{
	NSColor * color = [[NSColorPanel sharedColorPanel] color];
	UT_RGBColor rgbclr;
	GR_CocoaCairoGraphics::_utNSColorToRGBColor(color, rgbclr);
	
	UT_HashColor hash;

	const char * color_string = hash.setColor(rgbclr);
	if (color_string)
	{
		UT_UCS4String color_data(color_string);

		const UT_UCS4Char * pData = color_data.ucs4_str();
		UT_uint32 dataLength = static_cast<UT_uint32>(color_data.length());

		_xap->toolbarEvent(tlbrid, pData, dataLength);
	}
}

- (IBAction)aColor_FG:(id)sender
{
	UT_UNUSED(sender);
	[self setColor:AP_TOOLBAR_ID_COLOR_FORE];
}

- (IBAction)aColor_BG:(id)sender
{
	UT_UNUSED(sender);
	[self setColor:AP_TOOLBAR_ID_COLOR_BACK];
}

- (void)toolbarSelected:(id)sender
{
	UT_DEBUGMSG (("@EV_CocoaToolbarTarget (id)toolbarSelected:(id)sender\n"));
	
	if ([sender isKindOfClass:[NSPopUpButton class]])
	{
		XAP_Toolbar_Id tlbrID = [sender tag];

		UT_UCS4String ucsText([[sender titleOfSelectedItem] UTF8String]);
		_xap->toolbarEvent (tlbrID, ucsText.ucs4_str(), ucsText.length());

		if (XAP_Frame * pFrame = _xap->getFrame())
			pFrame->raise();
	}
	else if ([sender isKindOfClass:[NSButton class]])
	{
		XAP_Toolbar_Id tlbrID = [sender tag];

		switch (tlbrID) {
		case AP_TOOLBAR_ID_COLOR_FORE:
		case AP_TOOLBAR_ID_COLOR_BACK:
			{
				NSColorPanel * colorPanel = [NSColorPanel sharedColorPanel];

				// ?? [NSColorPanel setPickerMask:(NSColorPanelRGBModeMask|NSColorPanelWheelModeMask|NSColorPanelGrayModeMask)];

				[colorPanel setAction:0];
				[colorPanel setTarget:0];

				if (tlbrID == AP_TOOLBAR_ID_COLOR_FORE)
				{
					// [colorPanel setTitle:@"Foreground Color"]; // TODO: Localize
					// [colorPanel setColor:[oColor_FG color]];
					[colorPanel setAction:@selector(aColor_FG:)];
				}
				else
				{
					// [colorPanel setTitle:@"Background Color"]; // TODO: Localize
					// [colorPanel setColor:[oColor_BG color]];
					[colorPanel setAction:@selector(aColor_BG:)];
				}
				[colorPanel orderFront:self];
				[colorPanel setTarget:self];
			}
			break;

		default:
			{
				const UT_UCSChar * pData = NULL;
				UT_uint32 dataLength = 0;
		
				_xap->toolbarEvent(tlbrID, pData, dataLength);
			}
			break;
		}
	}
	else if ([sender isKindOfClass:[NSComboBox class]])
	{
		XAP_Toolbar_Id tlbrID = [sender tag];

	    if (tlbrID == AP_TOOLBAR_ID_FMT_SIZE)
		{
			int size = [sender intValue];

			if (size < 1)
				size = 1;   // TODO: ??
			if (size > 100)
				size = 100; // TODO: ??

			char buf[8];
			sprintf(buf, "%d", size);

			UT_UCS4String ucsText(buf);

			if (XAP_Frame * pFrame = _xap->getFrame())
			{
				pFrame->raise();
				_xap->toolbarEvent (tlbrID, ucsText.ucs4_str(), ucsText.length());
			}
		}
		else
		{
			NSComboBox * cbZoom = (NSComboBox *) sender;

			NSString * value = [cbZoom stringValue];
			NSArray * values = [cbZoom objectValues];

			if ([values containsObject:value])
			{
				UT_UCS4String ucsText([value UTF8String]);

				if (XAP_Frame * pFrame = _xap->getFrame())
				{
					pFrame->raise();
					_xap->toolbarEvent (tlbrID, ucsText.ucs4_str(), ucsText.length());
				}
			}
			else
			{
				int size = [cbZoom intValue];

				if (size < 1)
					size = 1;    // TODO: ??
				if (size > 1000)
					size = 1000; // TODO: ??

				char buf[8];
				sprintf(buf, "%d", size);

				UT_UCS4String ucsText(buf);

				if (XAP_Frame * pFrame = _xap->getFrame())
				{
					pFrame->raise();
					_xap->toolbarEvent (tlbrID, ucsText.ucs4_str(), ucsText.length());
				}
			}
		}
	}
	else
	{
		UT_DEBUGMSG (("Unexpected object class\n"));
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	}
}
@end



/*****************************************************************/


EV_CocoaToolbar::EV_CocoaToolbar(AP_CocoaFrame * pCocoaFrame,
							   const char * szToolbarLayoutName,
							   const char * szToolbarLabelSetName)
	: EV_Toolbar(XAP_App::getApp()->getEditMethodContainer(),
				 szToolbarLayoutName,
				 szToolbarLabelSetName),
	  m_pCocoaFrame(pCocoaFrame)
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

	NSButton * btn = nil;
	
	NSRect btnFrame;
	btnFrame.origin.x = btnX;
	btnFrame.origin.y = 0;
	btnFrame.size.width = BTN_WIDTH;
	btnFrame.size.height = BTN_HEIGHT;
	btnX += BTN_WIDTH;
	
	switch (type) {
	case EV_TBIT_PushButton:
		{
			btn = [[NSButton alloc] initWithFrame:btnFrame];

		//  [btn setButtonType:NSToggleButton];
			[btn setButtonType:NSMomentaryPushInButton];
		}
		break;

	case EV_TBIT_ToggleButton:
	case EV_TBIT_GroupButton:
		{
			btnFrame.origin.y = -1;

			btn = [[XAP_CocoaToolbarButton alloc] initWithFrame:btnFrame];

			[btn setButtonType:NSPushOnPushOffButton];

			NSButtonCell * cell = (NSButtonCell *) [btn cell];

			[cell setShowsStateBy:NSNoCellMask];
			[cell setHighlightsBy:NSNoCellMask];
		}
		break;
	default:
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	}
	if (btn) {
		[btn setBezelStyle:NSRegularSquareBezelStyle];
		[btn setBordered:NO];
		[btn setTag:(int)tlbrid];
		[btn setTarget:m_target];
		[btn setAction:@selector(toolbarSelected:)];

		UT_ASSERT(g_ascii_strcasecmp(pLabel->getIconName(),"NoIcon")!=0);
		NSImage * wPixmap = m_pCocoaToolbarIcons->getPixmapForIcon(pLabel->getIconName());		// autoreleased
		UT_ASSERT(wPixmap);
		[btn setImage:wPixmap];

		if (const char * szToolTip = pLabel->getToolTip())
			[btn setToolTip:[NSString stringWithUTF8String:szToolTip]];

		[parent addSubview:btn];
		[btn release];
	}
	return btn;
}

bool EV_CocoaToolbar::toolbarEvent(XAP_Toolbar_Id tlbrid,
									 const UT_UCSChar * pData,
									 UT_uint32 dataLength)

{
	// user selected something from this toolbar.
	// invoke the appropriate function.
	// return true iff handled.

	const EV_Toolbar_ActionSet * pToolbarActionSet = XAP_App::getApp()->getToolbarActionSet();
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
	
	const EV_EditMethodContainer * pEMC = XAP_App::getApp()->getEditMethodContainer();
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
void EV_CocoaToolbar::rebuildToolbar(UT_sint32 /*oldpos*/)
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

@interface EV_CocoaToolbarView : NSView
- (void)drawRect:(NSRect)aRect;
@end

@implementation EV_CocoaToolbarView

- (void)drawRect:(NSRect)aRect
{
	UT_UNUSED(aRect);
	[[NSColor lightGrayColor] set];

	[NSBezierPath strokeRect:[self frame]];
}

@end

bool EV_CocoaToolbar::synthesize(void)
{
	// TODO: rationalize those as static members of the class.
//	const float BTN_WIDTH = getButtonWidth ();
	const float BTN_HEIGHT = getButtonHeight ();

	// create a Cocoa toolbar from the info provided.
	float btnX = 0;
	const EV_Toolbar_ActionSet * pToolbarActionSet = XAP_App::getApp()->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	XAP_Toolbar_ControlFactory * pFactory = XAP_App::getApp()->getControlFactory();
	UT_ASSERT(pFactory);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

#if 0
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
#endif
	NSRect viewBounds;
	viewBounds.origin.x = 0.0f;
	viewBounds.origin.y = 0.0f;
	viewBounds.size.width  = 0.0f;
	viewBounds.size.height = getToolbarHeight();

	m_wToolbar = [[EV_CocoaToolbarView alloc] initWithFrame:viewBounds];

	[m_wToolbar setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];

#if 0
	////////////////////////////////////////////////////////////////
	// get toolbar button appearance from the preferences
	////////////////////////////////////////////////////////////////
	// TODO
	const gchar * szValue = NULL;
	XAP_App::getApp()->getPrefsValue(XAP_PREF_KEY_ToolbarAppearance, &szValue);
	UT_ASSERT((szValue) && (*szValue));
	
	if (g_ascii_strcasecmp(szValue, "icon") == 0) {
		[toolbar setDisplayMode:NSToolbarDisplayModeIconOnly];
	}
	else if (g_ascii_strcasecmp(szValue, "text") == 0) {
		[toolbar setDisplayMode:NSToolbarDisplayModeLabelOnly];
	}
	else if (g_ascii_strcasecmp(szValue, "both") == 0) {
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

				bool bIsCombo = true;

				float fWidth = 100;

				if (pControl)
				{
					fWidth = pControl->getPixelWidth();
				}
				switch (tlbrID)
				{
				case AP_TOOLBAR_ID_ZOOM:
					fWidth = 130;
					break;
				case AP_TOOLBAR_ID_FMT_STYLE:
					fWidth = 180;
					bIsCombo = false;
					break;
				case AP_TOOLBAR_ID_FMT_FONT:
					fWidth = 180;
					bIsCombo = false;
					break;
				case AP_TOOLBAR_ID_FMT_SIZE:
					fWidth = 55;
					break;
				default:
					UT_DEBUGMSG(("WARNING: adding combo control with default width.\n"));
					break;
				}

				NSRect btnFrame;
				btnFrame.origin.x = btnX + 1.0f;
				btnFrame.size.width = fWidth;
				btnFrame.size.height = (bIsCombo ? 26.0f : 25.0f);
				btnFrame.origin.y = rintf((BTN_HEIGHT - 26.0f) / 2.0f /* - (bIsCombo ? 0.0f : 1.0f) */);

				NSComboBox * comboBox = 0;
				NSPopUpButton * popupButton = 0;

				if (bIsCombo)
				{
					comboBox = [[NSComboBox alloc] initWithFrame:btnFrame];
					UT_ASSERT(comboBox);

					[m_wToolbar addSubview:comboBox];

					// [comboBox setFont:[NSFont systemFontOfSize:[NSFont smallSystemFontSize]]];

					[comboBox setTag:(int)tlbrID];
					[comboBox setEditable:YES];
					[comboBox setTarget:m_target];
					[comboBox setAction:@selector(toolbarSelected:)];

					btnX += [comboBox frame].size.width;
				}
				else
				{
					popupButton = [[NSPopUpButton alloc] initWithFrame:btnFrame pullsDown:NO];
					UT_ASSERT(popupButton);

					[m_wToolbar addSubview:popupButton];

					[popupButton setTag:(int)tlbrID];
					[popupButton setTarget:m_target];
					[popupButton setAction:@selector(toolbarSelected:)];

					btnX += [popupButton frame].size.width;
				}

				/* populate it
				 */
				if (tlbrID == AP_TOOLBAR_ID_FMT_SIZE)
				{
					/* AbiWord's other platforms have a bug that the comboboxes in the toolbars
					 * can't be edited and to get around this the comboboxes have a lot of values
					 * to choose from.
					 */
					[comboBox addItemWithObjectValue:@"72"];
					[comboBox addItemWithObjectValue:@"36"];
					[comboBox addItemWithObjectValue:@"24"];
					[comboBox addItemWithObjectValue:@"18"];
					[comboBox addItemWithObjectValue:@"16"];
					[comboBox addItemWithObjectValue:@"14"];
					[comboBox addItemWithObjectValue:@"12"];
					[comboBox addItemWithObjectValue:@"10"];
					[comboBox addItemWithObjectValue:@"8" ];

					[comboBox selectItemAtIndex:0];
					[comboBox setObjectValue:[comboBox objectValueOfSelectedItem]];

					[comboBox setNumberOfVisibleItems:9];
				}
				else if (pControl)
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
							if (comboBox)
								[comboBox addItemWithObjectValue:str];
							else
								[popupButton addItemWithTitle:str];
						}
						if (items > 0)
						{
							if (comboBox)
							{
								[comboBox selectItemAtIndex:0];
								[comboBox setObjectValue:[comboBox objectValueOfSelectedItem]];

								[comboBox setNumberOfVisibleItems:9];
							}
							else
							{
								[popupButton selectItemAtIndex:0];
							}
						}
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
			btnFrame.origin.x = btnX + 1.0f;
			btnFrame.size.width = 1.0f;
			btnFrame.size.height = BTN_HEIGHT; 
			btnFrame.origin.y = (BTN_HEIGHT - btnFrame.size.height) / 2;
			btnX += btnFrame.size.width + 3.0f;
			
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
	viewBounds.size.width = btnX;

	[m_wToolbar setFrame:viewBounds];

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

	UT_DebugOnly<bool> bResult = pView->addListener(static_cast<AV_Listener *>(m_pViewListener),&m_lid);
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
	
	const EV_Toolbar_ActionSet * pToolbarActionSet = XAP_App::getApp()->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	for (UT_uint32 j=0; (j < nrLabelItemsInLayout); j++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(j);
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
					
					bool bIsCombo = true;

					switch (tlbrid)
					{
					case AP_TOOLBAR_ID_FMT_STYLE:
					case AP_TOOLBAR_ID_FMT_FONT:
						bIsCombo = false;
						break;
					case AP_TOOLBAR_ID_ZOOM:
					case AP_TOOLBAR_ID_FMT_SIZE:
					default:
						break;
					}
					if (!bIsCombo)
					{
						NSPopUpButton * popupButton = [m_wToolbar viewWithTag:tlbrid];
						UT_ASSERT(popupButton);
						UT_ASSERT([popupButton isKindOfClass:[NSPopUpButton class]]);

						[popupButton setEnabled:(bGrayed ? NO : YES)];

						if ((tlbrid == AP_TOOLBAR_ID_FMT_STYLE) && m_pCocoaFrame)
						{
							int count = [popupButton numberOfItems];

							NSMutableArray * styles = [NSMutableArray arrayWithCapacity:(count ? count : 32)];

							if (PD_Document * pDoc = static_cast<PD_Document *>(m_pCocoaFrame->getCurrentDoc()))
							{

								UT_GenericVector<PD_Style*>* pStyles = NULL;
								pDoc->enumStyles(pStyles);
								UT_uint32 nStyles = pStyles->getItemCount();
								for (UT_uint32 k = 0; k < nStyles; k++) {
									PD_Style *pStyle = pStyles->getNthItem(k);
									const char * name;
									if (pStyle && (name = pStyle->getName())) {
										[styles addObject:[NSString stringWithUTF8String:name]];
									}
								}
								DELETEP(pStyles);
								// TODO: Make style names reflect properties such as: font, size, alignment ??
							}
							[styles sortUsingSelector:@selector(compare:)];

							[popupButton removeAllItems];
							[popupButton addItemsWithTitles:styles];
						}

						if (!szState) {
							break; // Mixed selection...
						}
						
						NSString * state = szState ? [NSString stringWithUTF8String:szState] : nil;

						if ((tlbrid == AP_TOOLBAR_ID_FMT_FONT) && state) {
							if ([popupButton indexOfItemWithTitle:state] < 0)
							{
								int count = [popupButton numberOfItems];

								NSMutableArray * fonts = [NSMutableArray arrayWithCapacity:(count + 1)];

								for (int i = 0; i < count; i++) {
									[fonts addObject:[[popupButton itemAtIndex:i] title]];
								}
								
								[fonts addObject:state]; // use attributed strings? mark absent fonts in red? [TODO]
								[fonts sortUsingSelector:@selector(compare:)];

								[popupButton removeAllItems];
								[popupButton addItemsWithTitles:fonts];
							}
						}
						if (state) {
							[popupButton selectItemWithTitle:state];
						}
						
						break;
					}

					NSComboBox * item = [m_wToolbar viewWithTag:tlbrid];
					UT_ASSERT(item);
					UT_ASSERT([item isKindOfClass:[NSComboBox class]]);
					// Disable/enable toolbar item
					[item setEnabled:(bGrayed?NO:YES)];
					NSString* value = nil;
					if (tlbrid == AP_TOOLBAR_ID_FMT_SIZE) {
						// mixed selection? ... UT_DEBUGMSG(("%s:%d fontSize not found.... !!!! FIXME", __FILE__, __LINE__));
						if (szState) {
							value = [[NSString alloc] initWithUTF8String:szState];
						}
						else {
							value = [[NSString alloc] initWithUTF8String:""];
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
						NSInteger idx = [item indexOfItemWithObjectValue:value];
						if (idx == NSNotFound) {
							[item setStringValue:value];
						}
						else {
							[item selectItemWithObjectValue:value];
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
#if 0
	XAP_Toolbar_ControlFactory * pFactory = XAP_App::getApp()->getControlFactory();
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
#endif
	return true;
}
