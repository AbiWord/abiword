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
#include "ev_QNXToolbar.h"
#include "xap_Types.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Layouts.h"
#include "ev_Toolbar_Labels.h"
#include "ev_Toolbar_Control.h"
#include "ev_EditEventMapper.h"
#include "xap_QNXToolbar_Icons.h"
#include "ev_QNXToolbar_ViewListener.h"
#include "xav_View.h"
#include "ut_xml.h"
#include "xap_Prefs.h"


/*****************************************************************/
/*****************************************************************/

EV_QNXToolbar::EV_QNXToolbar(XAP_QNXApp * pQNXApp, XAP_QNXFrame * pQNXFrame,
							   const char * szToolbarLayoutName,
							   const char * szToolbarLabelSetName)
	: EV_Toolbar(pQNXApp->getEditMethodContainer(),
				 szToolbarLayoutName,
				 szToolbarLabelSetName)
{
	m_pQNXApp = pQNXApp;
	m_pQNXFrame = pQNXFrame;
	m_pViewListener = 0;
	m_lid = 0;							// view listener id
}

EV_QNXToolbar::~EV_QNXToolbar(void)
{
//	UT_VECTOR_PURGEALL(_wd *,m_vecToolbarWidgets);
	_releaseListener();
}

bool EV_QNXToolbar::toolbarEvent(XAP_Toolbar_Id id, 
				 					UT_UCSChar * pData,
				 					UT_uint32 dataLength)

{
	// user selected something from this toolbar.
	// invoke the appropriate function.
	// return true iff handled.
	
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pQNXApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	const EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
	UT_ASSERT(pAction);

	AV_View * pView = m_pQNXFrame->getCurrentView();

	//Right away switch the focus
	m_pQNXFrame->setDocumentFocus();

	// make sure we ignore presses on "down" group buttons
	if (pAction->getItemType() == EV_TBIT_GroupButton) 
		
	{
		const char * szState = 0;
		EV_Toolbar_ItemState tis = pAction->getToolbarItemState(pView,&szState);

		//Let's make sure that in this case the button always reflects
		//the current reality (ie it is down if toggled).
		if (EV_TIS_ShouldBeToggled(tis))
		{
			//We clicked a button which was already down, make it not happen
			refreshToolbar(pView, AV_CHG_ALL);
			return true;
		}
	}

	const char * szMethodName = pAction->getMethodName();
	if (!szMethodName)
		return false;
	
	const EV_EditMethodContainer * pEMC = m_pQNXApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

	EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
	UT_ASSERT(pEM);						// make sure it's bound to something

	invokeToolbarMethod(pView,pEM,pData,dataLength);
	return true;
}

struct _cb_data {
	PtWidget_t	  *m_widget;
	EV_QNXToolbar *tb;
	XAP_Toolbar_Id id; 
};

int cb_select(PtWidget_t *w, void *data, PtCallbackInfo_t *info) {

	if (info->reason_subtype != Pt_LIST_SELECTION_FINAL) {
		return 0;
	}

	PtListCallback_t *cb = (PtListCallback_t *)info->cbdata;

	struct _cb_data *cb_data = (struct _cb_data *)data;
	cb_data->tb->toolbarEvent(cb_data->id, (UT_UCSChar *)cb->item, (UT_uint32)cb->item_len);

	return 0;
}

int but_activate(PtWidget_t *w, void *data, PtCallbackInfo_t *info) {

	struct _cb_data *cb_data = (struct _cb_data *)data;
	cb_data->tb->toolbarEvent(cb_data->id, 0, 0);

	return 0;
}

int s_colour_activate(PtWidget_t *w, void *data, PtCallbackInfo_t *info) {
	int					ret;
	char				*title;
	char				oklabel[3];
	char				cancellabel[8];
	PtColorSelectInfo_t cinfo;

	struct _cb_data *cb_data = (struct _cb_data *)data;

	memset(&cinfo, 0, sizeof(cinfo));
	cinfo.flags = Pt_COLORSELECT_MODAL;
	strcpy(oklabel, "OK");
	cinfo.accept_text = oklabel;
	strcpy(cancellabel, "Cancel");
	cinfo.dismiss_text = cancellabel;

	title = NULL;
	PtGetResource(w, Pt_ARG_TEXT_STRING, &title, 0);

	ret = PtColorSelect(w, (title) ? title : "Select a colour", &cinfo);
	if(ret & Pt_COLORSELECT_ACCEPT) {
		char clrstr[9];
		sprintf(clrstr, "%02x%02x%02x", PgRedValue(cinfo.rgb), PgGreenValue(cinfo.rgb), PgBlueValue(cinfo.rgb));
		//printf("Colour is [%s] len %d \n", clrstr, strlen(clrstr));
		cb_data->tb->toolbarEvent(cb_data->id, (UT_UCSChar *)clrstr, strlen(clrstr));
	} 
	return Pt_CONTINUE;
}


bool EV_QNXToolbar::synthesize(void)
{
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pQNXApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	XAP_Toolbar_ControlFactory * pFactory = m_pQNXApp->getControlFactory();
	UT_ASSERT(pFactory);
	
	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	PtArg_t args[10];
	struct _cb_data *tcb;		//Toolbar item call back
	PtWidget_t  *tb;			//Toolbar item
	PhImage_t   *image;			
	PhArea_t	area;

	PtWidget_t * wTLW = m_pQNXFrame->getTopLevelWindow();
	//PtWidget_t * wVBox = m_pQNXFrame->getVBoxWidget();

#if 0
	const XML_Char * szValue = NULL;
	m_pQNXApp->getPrefsValue(XAP_PREF_KEY_ToolbarAppearance,&szValue);
	UT_ASSERT((szValue) && (*szValue));
	GtkToolbarStyle style = GTK_TOOLBAR_ICONS;
	if (UT_XML_stricmp(szValue,"icon")==0)
		style = Pt_IMAGE;
	else if (UT_XML_stricmp(szValue,"text")==0)
		style = Pt_Z_STRING;
	else if (UT_XML_stricmp(szValue,"both")==0)
		style = Pt_TEXT_IMAGE;
#endif
	
#define TB_HEIGHT 45
	area.pos.x = 0; 
	area.pos.y = m_pQNXFrame->m_AvailableArea.pos.y;
	area.size.w = m_pQNXFrame->m_AvailableArea.size.w; 
	area.size.h = TB_HEIGHT;
	m_pQNXFrame->m_AvailableArea.pos.y += area.size.h + 3;
	m_pQNXFrame->m_AvailableArea.size.h -= area.size.h + 3;

	int n = 0;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0); 
#define _TB_ANCHOR_	(Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT | \
		 			 Pt_TOP_ANCHORED_TOP | Pt_BOTTOM_ANCHORED_TOP)
	PtSetArg(&args[n++], Pt_ARG_ANCHOR_FLAGS, _TB_ANCHOR_, _TB_ANCHOR_); 
	PtSetArg(&args[n++], Pt_ARG_TG_FLAGS, Pt_TG_COLLAPSIBLE, Pt_TG_COLLAPSIBLE);
	m_wToolbarGroup = PtCreateWidget(PtToolbarGroup, wTLW, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TOOLBAR_FLAGS, 0, Pt_TOOLBAR_FOLLOW_FOCUS);
	m_wToolbar = PtCreateWidget(PtToolbar, m_wToolbarGroup, n, args);
	UT_ASSERT(m_wToolbar);
	
	memset(&area, 0, sizeof(area));
	area.pos.y = 
	area.pos.x = 3;
	m_vecToolbarWidgets.clear();
	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_ASSERT(pLayoutItem);

		XAP_Toolbar_Id id = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		UT_ASSERT(pAction);
		EV_Toolbar_Label * pLabel = m_pToolbarLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		//Null these out on every iteration
		tb = NULL;
		tcb = NULL;

		switch (pLayoutItem->getToolbarLayoutFlags())
		{
		case EV_TLF_Normal:
		{

			const char * szToolTip = pLabel->getToolTip();
			if (!szToolTip || !*szToolTip) {
				szToolTip = pLabel->getStatusMsg();
			}

			switch (pAction->getItemType())
			{
			case EV_TBIT_PushButton:
			//TODO: For now these are buttons which bring up a dialog, as some point,
            //make them be able to bring up some sort of additional selector.
			case EV_TBIT_ColorFore:
			case EV_TBIT_ColorBack:

				{
					image = m_pQNXToolbarIcons->getPixmapForIcon(pLabel->getIconName());
					if (image && PhMakeGhostBitmap(image) == -1) {
						UT_DEBUGMSG(("Can't make ghost bitmap"));
					}

					n = 0;
					//printf("Add a button @ %d/%d ! \n", area.pos.x, area.pos.y);
					//This would add a ? help topic thingy
					//PtSetArg(&args[n++], Pt_ARG_HELP_TOPIC, szToolTip, 0);
					//PtSetArg(&args[n++], Pt_ARG_EFLAGS, Pt_INTERNAL_HELP, Pt_INTERNAL_HELP);
					//This will add the balloon help
					PtSetArg(&args[n++], Pt_ARG_LABEL_FLAGS, Pt_SHOW_BALLOON, Pt_SHOW_BALLOON); 
					PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, szToolTip, 0); 

					PtSetArg(&args[n++], Pt_ARG_POS, &area.pos, 0); 
					PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS); 
					if (image) {
						PtSetArg(&args[n++], Pt_ARG_LABEL_TYPE, Pt_IMAGE, Pt_IMAGE);
						//Set the flags on the image to
						//Ph_RELEASE_IMAGE | Ph_RELEASE_PALETTE | ...
						//PtSetArg(&args[n++], Pt_ARG_ARM_DATA, image, sizeof(*image)); 
						PtSetArg(&args[n++], Pt_ARG_LABEL_DATA, image, sizeof(*image)); 
						PtSetArg(&args[n++], Pt_ARG_DIM, &image->size, 0); 
						tb = PtCreateWidget(PtButton, m_wToolbar, n, args);
						area.pos.x += image->size.w + 10;
					}
					else {
						if(!szToolTip) {
							PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "No Label/Icon?", 0); 
						}
						tb = PtCreateWidget(PtButton, m_wToolbar, n, args);
						area.pos.x += 20 + 10;
					}
					if (tb) {
						tcb = (struct _cb_data *)malloc(sizeof(*tcb));
						tcb->tb = this;
						tcb->id = id;	
						tcb->m_widget = tb;
						if(pAction->getItemType() == EV_TBIT_PushButton) {
							PtAddCallback(tb, Pt_CB_ACTIVATE, but_activate, tcb);
						} else {
							PtAddCallback(tb, Pt_CB_ACTIVATE, s_colour_activate, tcb);
						}
					}
				}
				break;

			case EV_TBIT_ToggleButton:
			case EV_TBIT_GroupButton:
				{	
					image = m_pQNXToolbarIcons->getPixmapForIcon(pLabel->getIconName());
					if (image && PhMakeGhostBitmap(image) == -1) {
						printf("Can't make ghost bitmap \n");
					}
					else {
						//printf("NO IMAGE: [%s] \n", pLabel->getIconName());
					}

					n = 0;
					//printf("Add a button @ %d/%d ! \n", area.pos.x, area.pos.y);
					//This would add a ? help topic thingy
					//PtSetArg(&args[n++], Pt_ARG_HELP_TOPIC, szToolTip, 0);
					//PtSetArg(&args[n++], Pt_ARG_EFLAGS, Pt_INTERNAL_HELP, Pt_INTERNAL_HELP);
					//This will add the balloon help
					PtSetArg(&args[n++], Pt_ARG_LABEL_FLAGS, Pt_SHOW_BALLOON, Pt_SHOW_BALLOON); 
					PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, szToolTip, 0); 
					PtSetArg(&args[n++], Pt_ARG_POS, &area.pos, 0); 
					PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_TOGGLE, Pt_TOGGLE); 
					PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS); 
					if (image) {
						//For balloons set Pt_TEXT_IMAGE ...
						PtSetArg(&args[n++], Pt_ARG_LABEL_TYPE, Pt_IMAGE, Pt_IMAGE);
						//Set the flags on the image to
						//Ph_RELEASE_IMAGE | Ph_RELEASE_PALETTE | ...
						//PtSetArg(&args[n], Pt_ARG_ARM_DATA, image, sizeof(*image)); n++;
						PtSetArg(&args[n++], Pt_ARG_LABEL_DATA, image, sizeof(*image)); 
						PtSetArg(&args[n++], Pt_ARG_DIM, &image->size, 0); 
						tb = PtCreateWidget(PtButton, m_wToolbar, n, args);
						area.pos.x += image->size.w + 10;
					}
					else {
						//printf("Text button \n");
						PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "pbut", 0); 
						tb = PtCreateWidget(PtButton, m_wToolbar, n, args);
						area.pos.x += 20 + 10;
					}
					if (tb) {
						tcb = (struct _cb_data *)malloc(sizeof(*tcb));
						tcb->tb = this;
						tcb->id = id;	
						tcb->m_widget = tb;
						PtAddCallback(tb, Pt_CB_ACTIVATE, but_activate, tcb);
					}
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

				if (pControl) {
					iWidth = pControl->getPixelWidth();
				}

				n = 0;
				area.size.w = iWidth;
				area.size.h = 20;
				//printf("Setting CB: %d,%d %d/%d \n", 
				//		area.pos.x, area.pos.y, area.size.w, area.size.h);
				PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0); 
				PtSetArg(&args[n++], Pt_ARG_VISIBLE_COUNT, 6, 0); 
				PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS); 
				PtSetArg(&args[n++], Pt_ARG_LIST_FLAGS, 
						/*Pt_LIST_NON_SELECT |*/ 0, 
						/*Pt_LIST_NON_SELECT |*/ Pt_LIST_SCROLLBAR_GETS_FOCUS);
				PtSetArg(&args[n++], Pt_ARG_TEXT_FLAGS, 0, Pt_EDITABLE);
				tb = PtCreateWidget(PtComboBox, m_wToolbar, n, args);
				area.pos.x += area.size.w + 20;

				// populate it
				if (pControl && tb) {
					pControl->populate();

					const UT_Vector * v = pControl->getContents();
					UT_ASSERT(v);

					if (v) {
						UT_uint32 items = v->getItemCount();
						for (UT_uint32 m=0; m < items; m++)
						{
							const char * sz = (const char *)v->getNthItem(m);
							PtListAddItems(tb, &sz, 1, 0);
						}
					}
				}
				if (tb) {
					tcb = (struct _cb_data *)malloc(sizeof(*tcb));
					tcb->tb = this;
					tcb->id = id;	
					tcb->m_widget = tb;
					PtAddCallback(tb, Pt_CB_SELECTION, cb_select, tcb);
				}
 			}
			break;

			case EV_TBIT_StaticLabel:
				break;
					
			case EV_TBIT_Spacer:
				break;
					
			case EV_TBIT_BOGUS:
			default:
				UT_ASSERT(0);
				break;
			}
		}
		break;
			
		case EV_TLF_Spacer:
		{
			n = 0;
			//printf("Setting seperator: %d,%d %d/%d \n", 
			//		area.pos.x, area.pos.y, area.size.w, area.size.h);
#if !defined(FULL_TOOLBAR_USAGE)
			area.size.w = 10;
			area.size.h = TB_HEIGHT - (2 * area.pos.y);
			PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0); 
			PtSetArg(&args[n++], Pt_ARG_SEP_FLAGS, Pt_SEP_VERTICAL, Pt_SEP_ORIENTATION);
			PtSetArg(&args[n++], Pt_ARG_SEP_TYPE, Pt_ETCHED_IN, 0);
			tb = PtCreateWidget(PtSeparator, m_wToolbar, n, args);
			area.pos.x += 10;
#else
			n = 0;
			tb = m_wToolbar = PtCreateWidget(PtToolbar, m_wToolbarGroup, n, args);
#endif

			if (tb) {
				tcb = (struct _cb_data *)malloc(sizeof(*tcb));
				tcb->tb = this;
				tcb->id = id;	
				tcb->m_widget = tb;
			}
			break;
		}

		default:
			UT_ASSERT(0);
		}

		//Add the items to a vector so we can get them later 
		if (tcb) {
			m_vecToolbarWidgets.addItem(tcb);
		}
	}

	return true;
}

void EV_QNXToolbar::_releaseListener(void)
{
	if (!m_pViewListener)
		return;
	DELETEP(m_pViewListener);
	m_pViewListener = 0;
	m_lid = 0;
}
	
bool EV_QNXToolbar::bindListenerToView(AV_View * pView)
{
	_releaseListener();
	
	m_pViewListener = new EV_QNXToolbar_ViewListener(this,pView);
	UT_ASSERT(m_pViewListener);

	bool bResult;
	bResult = pView->addListener(static_cast<AV_Listener *>(m_pViewListener),&m_lid);
	UT_ASSERT(bResult);

	refreshToolbar(pView, AV_CHG_ALL);

	return true;
}

bool EV_QNXToolbar::refreshToolbar(AV_View * pView, AV_ChangeMask mask)
{
	PtArg_t args[10];
    struct _cb_data *tcb;       //Toolbar item call back
	int     n;

	// make the toolbar reflect the current state of the document
	// at the current insertion point or selection.
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pQNXApp->getToolbarActionSet();
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

		n = 0;
		tcb = NULL;

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

					tcb = (struct _cb_data *) m_vecToolbarWidgets.getNthItem(k);
					UT_ASSERT(tcb);
					PtWidget_t *item;
					item = tcb->m_widget;
					UT_ASSERT(item);
						
					// Disable/enable toolbar item
					PtSetArg(&args[n++], Pt_ARG_FLAGS, 
							 (bGrayed) ? (Pt_BLOCKED | Pt_GHOST) : Pt_SELECTABLE, 
							 Pt_BLOCKED | Pt_GHOST | Pt_SELECTABLE);
					PtSetResources(tcb->m_widget, n, args);

					//printf("Refresh TB: button %s \n", (bGrayed) ? "disabled" : "enabled");
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

					tcb = (struct _cb_data *) m_vecToolbarWidgets.getNthItem(k);
					UT_ASSERT(tcb);
					PtWidget_t *item;
					item = tcb->m_widget;
					UT_ASSERT(item);
						
					// Disable/enable toolbar item
					PtSetArg(&args[n++], Pt_ARG_FLAGS, 
							 ((bGrayed) ? Pt_BLOCKED | Pt_GHOST : 0),
							 Pt_BLOCKED | Pt_GHOST); 
					PtSetArg(&args[n++], Pt_ARG_FLAGS, 
							 ((bToggled) ? Pt_SET : 0),
							 Pt_SET); 

					/*
					printf("Refresh TB: button %s, %s \n", (bGrayed) ? "disabled" : "enabled",
														   (bToggled) ? "on" : "off");
					*/
					PtSetResources(tcb->m_widget, n, args);
						
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
					int     top = 1;
					
					tcb = (struct _cb_data *) m_vecToolbarWidgets.getNthItem(k);
					UT_ASSERT(tcb);
					PtWidget_t *item;
					item = tcb->m_widget;
					UT_ASSERT(item);
						
					// Disable/enable toolbar item
					PtSetArg(&args[n++], Pt_ARG_LIST_FLAGS, 
							 (bGrayed) ? Pt_LIST_INACTIVE : 0, 
							 Pt_LIST_INACTIVE); 

					if (!szState) {
						UT_DEBUGMSG(("TODO: Determine why state is NULL "));
					}

					//printf("State [%s] \n", (szState) ? szState : "NULL");
					if (szState && !(top = PtListItemPos(tcb->m_widget, szState))) {
						//Assume this is the case of Times New Roman not being found
						FontID *id = PfFindFont((const uchar_t *)szState, 0, 10);
						if (id) {
							top = PtListItemPos(tcb->m_widget, (char *)PfFontDescription(id));
							PfFreeFont(id);
						}
					}
					top = (top) ? top : 1;

					//PtSetArg(&args[n], Pt_ARG_TOP_ITEM_POS, top, 0); n++;
					PtSetArg(&args[n++], Pt_ARG_CBOX_SEL_ITEM, top, 0);
					PtSetResources(tcb->m_widget, n, args);

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
	return true;
}

XAP_QNXApp * EV_QNXToolbar::getApp(void)
{
	return m_pQNXApp;
}

XAP_QNXFrame * EV_QNXToolbar::getFrame(void)
{
	return m_pQNXFrame;
}

void EV_QNXToolbar::show(void) {
    UT_ASSERT(m_wToolbarGroup);
    PtRealizeWidget(m_wToolbarGroup);
}

void EV_QNXToolbar::hide(void) {
    UT_ASSERT(m_wToolbarGroup);
    PtUnrealizeWidget(m_wToolbarGroup);
}
