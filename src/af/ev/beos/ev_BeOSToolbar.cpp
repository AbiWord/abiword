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
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ev_BeOSToolbar.h"
#include "xap_Types.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Layouts.h"
#include "ev_Toolbar_Labels.h"
#include "ev_Toolbar_Control.h"
#include "ev_EditEventMapper.h"
#include "xap_BeOSToolbar_Icons.h"
#include "ev_BeOSToolbar_ViewListener.h"
#include "xav_View.h"
#include "ev_BeOSTooltip.h"

#define DPRINTF(x)
							 	 
EV_BeOSToolbar::EV_BeOSToolbar(XAP_BeOSApp * pBeOSApp, 
			   XAP_BeOSFrame * pBeOSFrame,
			   const char * szToolbarLayoutName,
			   const char * szToolbarLabelSetName)
	: EV_Toolbar(pBeOSApp->getEditMethodContainer(),
				 szToolbarLayoutName,
				 szToolbarLabelSetName) {
	m_pBeOSApp = pBeOSApp;
	m_pBeOSFrame = pBeOSFrame;
	m_pViewListener = NULL;
}

EV_BeOSToolbar::~EV_BeOSToolbar(void) {
	_releaseListener();
}

// This is used to do the initial toolbar creation
bool EV_BeOSToolbar::synthesize(void) {
	//Add the toolbar(s) beneath the menus ...
	be_Window 	*pBWin = NULL;

	if (!m_pBeOSFrame) 
		return false;
	pBWin = (be_Window *)m_pBeOSFrame->getTopLevelWindow();
	UT_ASSERT(pBWin);
	
	BRect r = pBWin->m_winRectAvailable;
	r.bottom = r.top + ITEM_HEIGHT + 2*ITEM_SEPERATE;
	pBWin->m_winRectAvailable.top = r.bottom + 1;
	ToolbarView *tb = new ToolbarView(this, r, "Toolbar", 
		 			  B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, 
		 			  B_WILL_DRAW | B_FRAME_EVENTS); 
	m_pTBView = tb;
	pBWin->AddChild(tb);
	
	// create a toolbar from the info provided.
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pBeOSApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	XAP_Toolbar_ControlFactory * pFactory = m_pBeOSApp->getControlFactory();
	UT_ASSERT(pFactory);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	// Allow the user to control if we show text or icons or both ...
#if 0
	const XML_Char * szValue = NULL;
	m_pBeOSApp->getPrefsValue(XAP_PREF_KEY_ToolbarAppearance,&szValue);
	UT_ASSERT((szValue) && (*szValue));

	GtkToolbarStyle style = GTK_TOOLBAR_ICONS;
	if (UT_XML_stricmp(szValue,"icon")==0)
		style = GTK_TOOLBAR_ICONS;
	else if (UT_XML_stricmp(szValue,"text")==0)
		style = GTK_TOOLBAR_TEXT;
	else if (UT_XML_stricmp(szValue,"both")==0)
		style = GTK_TOOLBAR_BOTH;         
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

		switch (pLayoutItem->getToolbarLayoutFlags()) {
		case EV_TLF_Normal: {
			const char * szToolTip = pLabel->getToolTip();
			if (!szToolTip || !*szToolTip)
				szToolTip = pLabel->getStatusMsg();
				
			switch (pAction->getItemType()) {
			case EV_TBIT_PushButton: {
				DPRINTF(printf("EVTOOLBAR: Push button \n"));
				UT_ASSERT(UT_stricmp(pLabel->getIconName(),"NoIcon")!=0);
				BBitmap * bitmap = m_pBeOSToolbarIcons->GetIconBitmap(pLabel->getIconName());
				//UT_ASSERT(bitmap);
				//Add the bitmap to the toolbar
				tb->AddItem(bitmap, NULL, id , szToolTip);
			}
			break;

			case EV_TBIT_GroupButton:
				DPRINTF(printf("EVTOOLBAR: Group Button \n"));
			case EV_TBIT_ToggleButton: {
				DPRINTF(printf("EVTOOLBAR: Toggle button \n"));
				UT_ASSERT(UT_stricmp(pLabel->getIconName(),"NoIcon")!=0);

				BBitmap * bitmap = m_pBeOSToolbarIcons->GetIconBitmap(pLabel->getIconName());
				//UT_ASSERT(bitmap);
				//Add the bitmap to the toolbar
				tb->AddItem(bitmap, NULL, id , szToolTip);
			}
			break;
			
			case EV_TBIT_ComboBox: {
				DPRINTF(printf("EVTOOLBAR: Combo Box \n"));
				EV_Toolbar_Control * pControl = pFactory->getControl(this, id);
				UT_ASSERT(pControl);
		
				//Add a combo type box ...
				BPopUpMenu *popupmenu = new BPopUpMenu(""); 
				UT_ASSERT(popupmenu);
		 	
				// populate it
				int iWidth = 100;
				if (pControl) {
					iWidth = pControl->getPixelWidth();
					pControl->populate();

					const UT_Vector * v = pControl->getContents();
					UT_ASSERT(v);
					if (v) {
						UT_uint32 items = v->getItemCount();
						for (UT_uint32 m=0; m < items; m++) {
							char * sz = (char *)v->getNthItem(m);
							BMessage *togo = new BMessage('ddwn');
							togo->AddInt32("id", id);
							togo->AddInt32("item", m);  
							BMenuItem *newmenu = new BMenuItem(sz, togo);
							UT_ASSERT(newmenu);
							newmenu->SetTarget(tb);
							popupmenu->AddItem(newmenu);							
							
						}
					}
				}
				//Stick it in the menu						         
				tb->AddItem(popupmenu, iWidth, id);

			}
			break;

			case EV_TBIT_EditText:
			case EV_TBIT_DropDown:
			case EV_TBIT_StaticLabel:
			case EV_TBIT_Spacer:
				break;
					
			case EV_TBIT_BOGUS:
			default:
				UT_ASSERT(0);
				break;
			}
		}
		break;
			
		case EV_TLF_Spacer:	{
			tb->AddSeperator();
			break;
		}
		
		default:
			UT_ASSERT(0);
		}
	}
	return true;
}

//This is used to make the toolbar reflect the current state of
//the document (enable, disable, set font values etc
bool EV_BeOSToolbar::refreshToolbar(AV_View * pView, AV_ChangeMask mask) {
	const EV_Toolbar_ActionSet * pToolbarActionSet;
	pToolbarActionSet = m_pBeOSApp->getToolbarActionSet();

	UT_ASSERT(pToolbarActionSet);
	int oldstate, perform_update = 0;
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++) {
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);

		XAP_Toolbar_Id id = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		UT_ASSERT(pAction);

		AV_ChangeMask maskOfInterest = pAction->getChangeMaskOfInterest();
		//If this item doesn't care about
		if ((maskOfInterest & mask) == 0) 
			continue;										// changes of this type, skip it...

		switch (pLayoutItem->getToolbarLayoutFlags()) {
		case EV_TLF_Normal: {
				const char * szState = 0;
				EV_Toolbar_ItemState tis = pAction->getToolbarItemState(pView,&szState);

				switch (pAction->getItemType())	{
				case EV_TBIT_PushButton: {
					bool bGrayed = EV_TIS_ShouldBeGray(tis);

				//	UT_DEBUGMSG(("refreshToolbar: PushButton [%s] is %s\n", 
				//		m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(), 
				//		((bGrayed) ? "disabled" : "enabled"))); 
					tb_item_t * item = m_pTBView->FindItemByID(id);
					if (item) {
						oldstate = item->state;
						item->state = (bGrayed) ? 0 : ENABLED_MASK;
						perform_update |= (oldstate == item->state) ? 0 : 1; 
					if(perform_update)
					{
						//m_pTBView->Window()->Lock();
						m_pTBView->Draw(item->rect);
					//	m_pTBView->Window()->Unlock();
					}
						}
				}
				break;
			
				case EV_TBIT_GroupButton:
					DPRINTF(printf("Ref Group->Toggle Button \n"));
				case EV_TBIT_ToggleButton: {
					bool bGrayed = EV_TIS_ShouldBeGray(tis);
					bool bToggled = EV_TIS_ShouldBeToggled(tis);

											
					//UT_DEBUGMSG(("refreshToolbar: ToggleBut [%s] is %s and %s\n", 
					//	m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(), 
					//	((bGrayed) ? "disabled" : "enabled"), 
					//	((bToggled) ? "pressed" : "not pressed")));

					tb_item_t * item = m_pTBView->FindItemByID(id);
					if (item) {
						oldstate = item->state;
						item->state = ((bGrayed) ? 0 : ENABLED_MASK) |
						              ((bToggled) ? PRESSED_MASK : 0);
						perform_update |= (oldstate == item->state) ? 0 : 1; 
						if(perform_update)
		        			{
				//			 m_pTBView->Window()->Lock();
		 					 m_pTBView->Draw(item->rect);
		 		//			 m_pTBView->Window()->Unlock();
						}	
																															                                    
					}
				
				}
				break;

				case EV_TBIT_ComboBox: {
					bool bGrayed = EV_TIS_ShouldBeGray(tis);
					bool bString = EV_TIS_ShouldUseString(tis);
						
					//UT_DEBUGMSG(("refreshToolbar: ComboBox [%s] is %s and %s\n", 
					// 	m_pToolbarLabelSet->getLabel(id)->getToolbarLabel(), 
					//	((bGrayed) ? "disabled" : "enabled"), 
					//	((bString) ? szState : "no state")));

					tb_item_t * item = m_pTBView->FindItemByID(id);
					if (item && bString) {
						BPopUpMenu *popup;
						UT_ASSERT(item->menu);
						popup = (BPopUpMenu*)item->menu->Menu();
						UT_ASSERT(popup);
						BMenuItem *mnuitem = popup->FindItem(szState);
						if (!mnuitem) {
							printf("Can't find menu item %s \n", szState);
							if (!(mnuitem = popup->FindItem("Dutch801 Rm BT")))
								break;
							//Send a message to fix that
							/*
							char *buffer = "Dutch801 Rm BT";
							toolbarEvent(id, 
								(UT_UCSChar *)buffer, strlen(buffer));
							*/
						}
						mnuitem->SetMarked(true);
					}			

				}
				break;

				case EV_TBIT_EditText:
				case EV_TBIT_DropDown:
				case EV_TBIT_StaticLabel:
				case EV_TBIT_Spacer:
					DPRINTF(printf("refreshToolbar: Update Text, DropDown, Label, Spacer \n"));
					break;
				case EV_TBIT_BOGUS:
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

#if 1
//TF I can't remember why I put this in, but it doesn't 
//seem to be needed now and is contributing in a big way
//to the slowdown of the drag updates.

//TF Note ... without this we don't get updated properly
//when a button state changes.  Instead put in hooks
//that only update us as required.
#endif
	return true;
}

// We call this when something substantial has happened to us
bool EV_BeOSToolbar::toolbarEvent(XAP_Toolbar_Id id,
				 UT_UCSChar * pData,
				 UT_uint32 dataLength) {
	// user selected something from this toolbar.
	// invoke the appropriate function.
	// return true iff handled.

	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pBeOSApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	const EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
	UT_ASSERT(pAction);

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return false;
	
	const EV_EditMethodContainer * pEMC = m_pBeOSApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);			// make sure it's bound to something

	invokeToolbarMethod(m_pBeOSFrame->getCurrentView(),pEM,pData,dataLength);
	return true;
}


//These two functions enable us to listen for messages about
//updating ourselves ... which we then pass to refresh	
bool EV_BeOSToolbar::bindListenerToView(AV_View * pView) {
	_releaseListener();
	
	m_pViewListener = new EV_BeOSToolbar_ViewListener(this,pView);
	UT_ASSERT(m_pViewListener);

	AV_ListenerId lid;
	bool bResult = pView->addListener(static_cast<AV_Listener *>(m_pViewListener),&lid);
	UT_ASSERT(bResult);

	refreshToolbar(pView, AV_CHG_ALL);
	return true;
}

void EV_BeOSToolbar::_releaseListener(void) {
	if (!m_pViewListener)
		return;
	DELETEP(m_pViewListener);
	m_pViewListener = NULL;
}



/*********************************************************/
#pragma mark -- Be Class
class TBMouseFilter: public BMessageFilter {
	public:
		TBMouseFilter(ToolbarView * pToolbarView);
		filter_result Filter(BMessage *message, BHandler **target);
	private:
		ToolbarView 	*m_pTBView;
};
		
TBMouseFilter::TBMouseFilter(ToolbarView *pToolbarView)
          : BMessageFilter(B_PROGRAMMED_DELIVERY, B_LOCAL_SOURCE) {
	m_pTBView = pToolbarView;
}					   

filter_result TBMouseFilter::Filter(BMessage *message, BHandler **target) 
{ 
	BRect r;
	BPoint pt;
 	int i;
 		
	switch(message->what) 
	{
		case B_MOUSE_UP: 
		{		
			message->FindPoint("where", &pt);
			for (i=0; i< m_pTBView->item_count; i++) {
				r = m_pTBView->items[i].rect;
				if (m_pTBView->items[i].state & ENABLED_MASK &&
				    r.Contains(pt) && m_pTBView->items[i].bitmap != NULL) 
				{
					//We have a hit on an item ...
					int id = m_pTBView->items[i].id;
			 		m_pTBView->m_pBeOSToolbar->toolbarEvent(id, NULL, 0);		
					break;
				}
			}
			break;
		}
	
	default:
		return(B_DISPATCH_MESSAGE);
	}
	return(B_SKIP_MESSAGE);			
}


ToolbarView::ToolbarView(EV_BeOSToolbar *tb, BRect frame, const char *name, 
			 uint32 resizeMask, uint32 flags) 
			: BView (frame, name, resizeMask, flags) {
	int i;
	
	for (i=0; i<ITEM_MAX; i++) {
			items[i].type = NONE;
			items[i].bitmap = NULL;
			items[i].menu = NULL;
	}
	item_count = 0;	
	last_highlight = -1;
	m_pBeOSToolbar = tb;
		
	SetViewColor(216, 216, 216);
	m_fOldWidth=frame.Width()+1;
	m_fOldHeight=frame.Height()+1;
	TBMouseFilter *filter = new TBMouseFilter(this);
	AddFilter(filter);
	
	m_bDisplayTooltip = false;
	fToolTip = new TToolTip();
	lastToolTipIndex = -1;
}

ToolbarView::~ToolbarView() 
{
	; //Do nothing ...

	for(int i = 0; i < item_count; i ++)
	{
		if(items[i].popupString)
			free(items[i].popupString);
	}
}

tb_item_t * ToolbarView::FindItemByID(XAP_Toolbar_Id id) {
	int i;
	
	for (i=0; i<item_count; i++) {
		if (items[i].id == id)
			break;
	}
	if (i<item_count) {
		return(&items[i]);
	}
	return(NULL);
}

bool ToolbarView::AddItem(BBitmap *upbitmap, BBitmap *downbitmap, XAP_Toolbar_Id id , const char* popupString) {
	if (item_count >= ITEM_MAX -1)
		return(false);
		
	if (downbitmap == NULL) {
		items[item_count].popupString = (char *)malloc(strlen(popupString) + 1);
		strcpy(items[item_count].popupString , popupString);// strdup(popupString);
		items[item_count].popupString[strlen(popupString)] = '\0';
		
		items[item_count].type = BITMAP_BUTTON;
		items[item_count].id = id;
		items[item_count].bitmap = upbitmap;		
		items[item_count].rect.top = ITEM_SEPERATE; 
		items[item_count].rect.bottom = ITEM_SEPERATE + ITEM_HEIGHT;
		items[item_count].rect.left = ITEM_SEPERATE;
		if (item_count != 0)
			items[item_count].rect.left +=  items[item_count-1].rect.right;
		items[item_count].rect.right = ITEM_WIDTH +
			items[item_count].rect.left; 
		item_count++;
	}
	else {
		//We have a toggle button to create ...
	}
	return(true);
}
bool ToolbarView::AddItem(BPopUpMenu *popupmenu, int iWidth, XAP_Toolbar_Id id) {
	if (item_count >= ITEM_MAX -1)
		return(false);

	items[item_count].popupString = NULL;
	items[item_count].type = DROP_DOWN;
	items[item_count].id = id;
	items[item_count].rect.top = ITEM_SEPERATE; 
	items[item_count].rect.left = ITEM_SEPERATE;
	if (item_count != 0) {
		items[item_count].rect.left +=  items[item_count-1].rect.right;
	}
	items[item_count].rect.right = iWidth +
								   items[item_count].rect.left; 
	items[item_count].rect.bottom = ITEM_SEPERATE + ITEM_HEIGHT;
	//printf("DROPDOWN RECT: "); items[item_count].rect.PrintToStream();

	items[item_count].menu = new BMenuField(items[item_count].rect, 
										   "DropDownBox", 
										   "", 
									   		popupmenu, 
									   		B_FOLLOW_LEFT | B_FOLLOW_TOP, 
									   		B_WILL_DRAW); 

	items[item_count].menu->SetDivider(0); 
	AddChild(items[item_count].menu);
	
	/*
	if (items[item_count].menu->SetTargetForItems(this) != B_OK)
		printf("CAN'T SET TARGET FOR ITEMS \n");
	*/
	
	//items[item_count].menu->SetDivider(0);
	item_count++;	
	return(true);
}

bool ToolbarView::AddSeperator() {
	if (item_count >= ITEM_MAX -1)
		return(false);
		
	items[item_count].popupString = NULL;
	items[item_count].type = SEPERATOR;
	items[item_count].rect.top = ITEM_SEPERATE; 
	items[item_count].rect.bottom = ITEM_SEPERATE + ITEM_HEIGHT;
	items[item_count].rect.left = ITEM_SEPERATE;
	if (item_count != 0)
		items[item_count].rect.left +=  items[item_count-1].rect.right;
	items[item_count].rect.right = items[item_count].rect.left + 2; 
	item_count++;	
	return(true);
}

void ToolbarView::MessageReceived(BMessage *msg) {
	switch (msg->what) {
	case 'ddwn': {
		int32 i, id, item;
		msg->FindInt32("id", &id);
		msg->FindInt32("item", &item);
		
		for (i=0; i<item_count; i++) {
			if (items[i].id == id)
				break;
		}
		BMenuItem *mnuitem;
		mnuitem = items[i].menu->Menu()->FindMarked();
		char buffer[255];
		strcpy(buffer, mnuitem->Label());
		m_pBeOSToolbar->toolbarEvent(id, (UT_UCSChar *)buffer, strlen(buffer));
		break;
		
	case eToolTipStart:	
	case eToolTipStop:
			fToolTip->PostMessage(msg);
		break;
		
	}
	default:		
		BView::MessageReceived(msg);
	}
}

void ToolbarView::Draw(BRect clip) {
	BRect 	r;
	int 	i;
//	BPicture *mypict;
//	BeginPicture(new BPicture);
	Window()->DisableUpdates();
	for (i=0; i<item_count; i++) {
		r = items[i].rect;
		if (items[i].bitmap && r.Intersects(clip)) {
//			UT_DEBUGMSG(("Clip intersection on i:%d\n",i));
//			UT_DEBUGMSG(("Item rect left=%f top=%f right=%f bottom=%f\n",r.left,r.top,r.right,r.bottom));
			Window()->Lock();
			//Draw the bitmap of the icon
			DrawBitmapAsync(items[i].bitmap, BPoint(r.left, r.top));
			//DrawBitmap(items[i].bitmap, BPoint(r.left, r.top));
			
			//Disabled icons should be greyed out ...
			if (!(items[i].state & ENABLED_MASK)) {
//				drawing_mode oldmode = DrawingMode();
//				SetDrawingMode(B_OP_SUBTRACT);
///				SetHighColor(80, 80, 80);
//				FillRect(r);
//				SetDrawingMode(oldmode);
			}

			//Pressed icons should look like they are pressed (ie down)
			else if (items[i].state & PRESSED_MASK) {
				//Toggle buttons which are down
				HighLightItem(i, 2);
			}			

			//We just draw normal icons when activated
			else {
				DrawBitmapAsync(items[i].bitmap, BPoint(r.left, r.top));
			}
			Window()->Unlock();
		}
		else if (items[i].type == SEPERATOR) {
			HighLightItem(i, 1);
		}
	}
	
	//Draw a nice border around the toolbar
	Window()->Lock();
	r = Bounds();
	SetHighColor(152,152,152);		//Dark grey
	StrokeRect(r);
	rgb_color colortouse;
	rgb_color dark={192,192,192};
	rgb_color light={240,240,240};
	//SetHighColor(192,192,192);		//Light Dark grey
	colortouse=dark;
	BeginLineArray(4);
	AddLine(BPoint(r.left+1, r.bottom-1), BPoint(r.right-1, r.bottom-1),colortouse);
	AddLine(BPoint(r.right-1, r.top+1), BPoint(r.right-1, r.bottom-1),colortouse);
	colortouse=light;
	//SetHighColor(240,240,240);		//Almost white
	AddLine(BPoint(r.left+1, r.bottom-1), BPoint(r.left+1, r.top+1),colortouse);
	AddLine(BPoint(r.left+1, r.top+1), BPoint(r.right-1, r.top+1),colortouse);
//	if ((mypict = EndPicture())) {
//		DrawPicture(mypict, BPoint(0,0));
//		Sync();
//		delete mypict;
//	}
	EndLineArray();
Window()->EnableUpdates();
Sync();
Window()->Unlock();
}

void ToolbarView::HighLightItem(int index, int state) {
	BRect r = items[index].rect;
	rgb_color colortouse;
	rgb_color dark = { 150, 150, 150, 255 };	//was 192
	rgb_color light = { 240, 240, 240, 255 };	//was 240
	Window()->Lock();	
	if (state == 1)				//UP look
		colortouse=dark;		//Light Dark grey
	else if (state == 2)			//DOWN look
		colortouse=light;		//Almost white
	BeginLineArray(8);
	AddLine(BPoint(r.left, r.bottom), BPoint(r.right, r.bottom),colortouse);
	AddLine(BPoint(r.left+1, r.bottom-1), BPoint(r.right-1, r.bottom-1),colortouse);
	AddLine(BPoint(r.right, r.top), BPoint(r.right, r.bottom),colortouse);
	AddLine(BPoint(r.right-1, r.top+1), BPoint(r.right-1, r.bottom-1),colortouse);
	if (state == 1)
		colortouse=light;		//Almost white
	else if (state == 2)
		colortouse=dark;		//Light Dark grey
	AddLine(BPoint(r.left, r.bottom), BPoint(r.left, r.top),colortouse);
	AddLine(BPoint(r.left+1, r.bottom-1), BPoint(r.left+1, r.top+1),colortouse);
	AddLine(BPoint(r.left, r.top), BPoint(r.right, r.top),colortouse);
	AddLine(BPoint(r.left+1, r.top+1), BPoint(r.right-1, r.top+1),colortouse);
	EndLineArray();
	Window()->Sync();
	Window()->Unlock();
}

void ToolbarView::FrameResized(float width, float height) {
	BRect r;
	/*if (width+1 > m_fOldWidth)
	{
	*/	r.left=m_fOldWidth-5;
		r.right=width;
		r.top=Bounds().top;
		r.bottom=Bounds().bottom;
//		UT_DEBUGMSG(("Actually invalidating toolbar\n"));
		Invalidate(r);
	/*}*/
	m_fOldWidth=width+1;
}
									
void ToolbarView::MouseMoved(BPoint where, uint32 code,	const BMessage *msg) 
{
#if 0 // Old code.
	int i;

	return;
	
	for (i=0; i<item_count; i++) {
		if (items[i].type == BITMAP_BUTTON &&
		    items[i].rect.Contains(where) && (items[i].state & ENABLED_MASK)) {
			if (i != last_highlight) {
				HighLightItem(i, 0);
				if (last_highlight >= 0)
					HighLightItem(last_highlight, 1);
			}
			last_highlight = i;
			return;
		}
	}
	if (last_highlight >= 0)
		HighLightItem(last_highlight, 1);
#else // New code -- Added by cjp - 11/17/00

			int i = 0;
			BRect r;
						
			for (i=0; i< item_count; i++) 
			{
				r = items[i].rect;
				if (r.Contains(where)) 
				{
					break;
				}
			}
			
			if(i >= item_count)
				return;
				
			// if mouse has moved into our view, our window is active and it previously
			// wasn't in, send a message to start the ToolTip
			if (items[i].popupString && (items[i].rect.Contains(where)) && (Window()->IsActive())) 
			{
				if (!m_bDisplayTooltip) 
				{
					BMessage	msg(eToolTipStart);
		
					msg.AddPoint("start", ConvertToScreen(where));
					msg.AddRect("bounds", ConvertToScreen(items[i].rect));
					msg.AddString("string", items[i].popupString);
					MessageReceived(&msg);

					m_bDisplayTooltip = true;
					lastToolTipIndex = i;
				}
				else if(lastToolTipIndex != -1 && i != lastToolTipIndex)
				{
					BMessage msg(eToolTipStop);
					MessageReceived(&msg);
					m_bDisplayTooltip = false;
					lastToolTipIndex = -1;
				}
			}
			// otherwise stop the message
			else if (m_bDisplayTooltip) {
				BMessage msg(eToolTipStop);
				MessageReceived(&msg);
				m_bDisplayTooltip = false;
				lastToolTipIndex = -1;
			}
#endif
}

