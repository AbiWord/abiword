/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Tab.h"
#include "ap_BeOSDialog_Tab.h"

#include "ut_Rehydrate.h"

/*****************************************************************/

class TabWindow : public BWindow
{
public:
	TabWindow(BMessage *archiveData);

	void SetDlg(AP_BeOSDialog_Tab *tabDlg);
	virtual void MessageReceived(BMessage *msg);
	virtual bool QuitRequested(void);

private:
	AP_BeOSDialog_Tab 	*m_DlgTab;
	
	sem_id modalSem;
	status_t WaitForDelete(sem_id blocker);

};


status_t TabWindow::WaitForDelete(sem_id blocker)
{
	status_t	result;
	thread_id	this_tid = find_thread(NULL);
	BLooper		*pLoop;
	BWindow		*pWin = 0;

	pLoop = BLooper::LooperForThread(this_tid);
	if (pLoop)
		pWin = dynamic_cast<BWindow*>(pLoop);

	// block until semaphore is deleted (modal is finished)
	if (pWin) {
		do {
			// update the window periodically	
			pWin->Unlock(); // Neccesary because the window updates when ok or apply are hit
			snooze(100);
			pWin->Lock();
			
			pWin->UpdateIfNeeded();
			result = acquire_sem_etc(blocker, 1, B_TIMEOUT, 1000);
		} while (result != B_BAD_SEM_ID);
	} else {
		do {
			// just wait for exit
			result = acquire_sem(blocker);
		} while (result != B_BAD_SEM_ID);
	}
	return result;
}

TabWindow::TabWindow(BMessage *data) 
	  :BWindow(data) 
{
	
} //BreakWin::BreakWin


void TabWindow::SetDlg(AP_BeOSDialog_Tab *tabDlg) 
{

	m_DlgTab = tabDlg;
	
	// Default to returning cancel, this will change when ok is hit.

	m_DlgTab->_populateWindowData();
	m_DlgTab->m_answer = AP_Dialog_Tab::a_CANCEL;
	
	Show();

	modalSem = create_sem(0,"ParagraphSem");
	WaitForDelete(modalSem);
	
	Hide();
}

bool TabWindow::QuitRequested()
{
	delete_sem(modalSem);
	return (false);
}

void TabWindow::MessageReceived(BMessage *pMsg)
{
	BListView* sourceView;
	
	switch(pMsg->what)
	{
		case 'alig': // any alignment button clicked
			m_DlgTab->_event_AlignmentChange();
			
			break;
		
		case 'okay': // Okay button pressed
		
			m_DlgTab->m_answer = AP_Dialog_Tab::a_OK;		
			m_DlgTab->_storeWindowData();
			
			PostMessage(B_QUIT_REQUESTED);
			break;
			
		case 'appl': // apply hit.
		
			m_DlgTab->_storeWindowData(); // I guess..
			
			break;
		
		case 'sett': // set button hit
			m_DlgTab->_event_Set();
			break;
		
		case 'clea': // clear button hit
			m_DlgTab->_event_Clear();
			break;
		
		case 'clel': // clear all button hit.
			m_DlgTab->_event_ClearAll();
			break;
				
		case 'tabc': // Selection in the tab list changed.
			if(pMsg->FindPointer("source" , (void **)&sourceView) == B_OK)
			{
				UT_uint32 Index = (UT_uint32)sourceView->CurrentSelection();
				m_DlgTab->_event_TabSelected(Index);
			}
			break;
	
		case 'tabp': // Called when the user has finished modifying the tab stop edit box
			m_DlgTab->_event_TabChange();
			break;
			
		default:
			BWindow::MessageReceived(pMsg);	
	}
	
	
}

/*****************************************************************/

XAP_Dialog * AP_BeOSDialog_Tab::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_BeOSDialog_Tab * p = new AP_BeOSDialog_Tab(pFactory,id);
	return p;
}

AP_BeOSDialog_Tab::AP_BeOSDialog_Tab(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_Tab(pDlgFactory,id)
{
}

AP_BeOSDialog_Tab::~AP_BeOSDialog_Tab(void)
{
}

void AP_BeOSDialog_Tab::_controlEnable( tControl id, bool value )
{
	BControl* toggleControl;
	char* viewName = NULL;
	
	switch(id)
	{
		// buttons
		case id_BUTTON_SET:
			viewName = "setButton";
			break;
	
		case id_BUTTON_CLEAR:
			viewName = "clearButton";
			break;
		
		case id_BUTTON_CLEAR_ALL:
			viewName = "clearAllButton";
			break;
		
		// alignment
		case id_ALIGN_BAR:
			viewName = "alignBar";
			break;
			
		// leaders
		case id_LEADER_NONE:
			viewName = "tabNoLeader";
			break;
			
		case id_LEADER_DOT:
			viewName = "tabDot";
			break;
			
		case id_LEADER_DASH:
			viewName = "tabDash";
			break;
			
		case id_LEADER_UNDERLINE:
			viewName = "tabUnderline";
			break;
	}
	
	if(viewName)
	{
		toggleControl = (BControl *)newwin->FindView(viewName);
		if(toggleControl)
			toggleControl->SetEnabled( (value == true) );
	}
}

void AP_BeOSDialog_Tab::runModal(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;
	
	UT_ASSERT(pFrame);
	
	BMessage msg;
	if (RehydrateWindow("TabWindow", &msg)) {
                newwin = new TabWindow(&msg);
		newwin->SetDlg(this);

		//Take the information here ...
		newwin->Lock();
		newwin->Quit();
    
	     }
}

eTabType AP_BeOSDialog_Tab::_gatherAlignment()
{
	BRadioButton* pToggledButton;

	pToggledButton = (BRadioButton *)newwin->FindView("alignleft");
	if(pToggledButton->Value())
		return FL_TAB_LEFT;
	
	pToggledButton = (BRadioButton *)newwin->FindView("alignright");
	if(pToggledButton->Value())
		return FL_TAB_RIGHT;
	
	pToggledButton = (BRadioButton *)newwin->FindView("aligncenter");
	if(pToggledButton->Value())
		return FL_TAB_CENTER;
		
	pToggledButton = (BRadioButton *)newwin->FindView("aligndecimal");
	if(pToggledButton->Value())
		return FL_TAB_DECIMAL;
		
	pToggledButton = (BRadioButton *)newwin->FindView("alignBar");
	if(pToggledButton->Value())
		return FL_TAB_BAR;
	
	return FL_TAB_NONE;
}

void AP_BeOSDialog_Tab::_setAlignment( eTabType a )
{
	BRadioButton* pToggledButton = NULL;
	
	switch(a)
	{
		case FL_TAB_LEFT:
			pToggledButton = (BRadioButton *)newwin->FindView("alignleft");
			break;
			
		case FL_TAB_RIGHT:
			pToggledButton = (BRadioButton *)newwin->FindView("alignright");
			break;
			
		case FL_TAB_CENTER:
			pToggledButton = (BRadioButton *)newwin->FindView("aligncenter");
			break;
			
		case FL_TAB_DECIMAL:
			pToggledButton = (BRadioButton *)newwin->FindView("aligndecimal");
			break;
			
		case FL_TAB_BAR:
			pToggledButton = (BRadioButton *)newwin->FindView("alignBar");
			break;
	}
	
	if(pToggledButton)
		pToggledButton->SetValue(1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

eTabLeader AP_BeOSDialog_Tab::_gatherLeader()
{
	return FL_LEADER_NONE;
}

void AP_BeOSDialog_Tab::_setLeader( eTabLeader a )
{
	BRadioButton* pButton = NULL;
	
	switch(a)
	{
		case id_LEADER_NONE:
			pButton = (BRadioButton *)newwin->FindView("tabNoLeader");
			break;
			
		case id_LEADER_DOT:
			pButton = (BRadioButton *)newwin->FindView("tabDot");
			break;
		
		case id_LEADER_DASH:
			pButton = (BRadioButton *)newwin->FindView("tabDash");	
			break;
		
		case id_LEADER_UNDERLINE:
			pButton = (BRadioButton *)newwin->FindView("tabUnderline");
			break;
	}
	
	if(pButton)
		pButton->SetValue(1);
}

const XML_Char * AP_BeOSDialog_Tab::_gatherDefaultTabStop()
{
	const XML_Char* pText = NULL;

	BTextControl *pControl = (BTextControl *)newwin->FindView("defaultTabStop");
	if(pControl)
	{
		 pText = pControl->Text();
	}	
	
	return pText;
}

void AP_BeOSDialog_Tab::_setDefaultTabStop( const XML_Char* default_tab )
{
	BTextControl *pControl = (BTextControl *)newwin->FindView("defaultTabStop");
	if(pControl)
	{
		pControl->SetText(default_tab);
	}	
}

void AP_BeOSDialog_Tab::_setTabList( UT_uint32 count )
{
	// Loop through the list box and remove it's contents.
	BStringItem* removedItem;
	UT_uint32 i;
	
	BListView* pList = (BListView*)newwin->FindView("tabList");
	if(pList)
	{
		int numItems = pList->CountItems();
		
		for( i = 0; i < numItems; i ++)
		{
			removedItem = (BStringItem *)pList->RemoveItem((int32)0);
			delete removedItem;
		}
	}
	
	for ( i = 0; i < count; i++ )
	{
		pList->AddItem(new BStringItem(_getTabDimensionString(i)));//_win32Dialog.addItemToList(AP_RID_DIALOG_TABS_TAB_STOP_POSITION_LIST, _getTabDimensionString(i));
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

UT_sint32 AP_BeOSDialog_Tab::_gatherSelectTab()
{
	BListView* pList = (BListView*)newwin->FindView("tabList");
	if(pList)
	{
		return pList->CurrentSelection();
	}
	
	return -1;
}

void AP_BeOSDialog_Tab::_setSelectTab( UT_sint32 v )
{
	BListView* pList = (BListView*)newwin->FindView("tabList");
	if(pList)
	{
		pList->Select(v);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const char * AP_BeOSDialog_Tab::_gatherTabEdit()
{
	BTextControl* tabStop = (BTextControl *)newwin->FindView("tabPosition");
	if(tabStop)
		return tabStop->Text();
	else
		return NULL;
}

void AP_BeOSDialog_Tab::_setTabEdit( const char *pszStr )
{	
	BTextControl* tabStop = (BTextControl *)newwin->FindView("tabPosition");
	if(tabStop)
		tabStop->SetText(pszStr);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void AP_BeOSDialog_Tab::_clearList()
{
	// Loop through the list box and remove it's contents.
	BStringItem* removedItem;
	
	BListView* pList = (BListView*)newwin->FindView("tabList");
	if(pList)
	{
		int numItems = pList->CountItems();
		
		for(int i = 0; i < numItems; i ++)
		{
			removedItem = (BStringItem *)pList->RemoveItem((int32)0);
			delete removedItem;
		}
	}
}
