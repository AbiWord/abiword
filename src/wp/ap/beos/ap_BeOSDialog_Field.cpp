/* AbiWord
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Field.h"
#include "ap_BeOSDialog_Field.h"

#include "ut_Rehydrate.h"

/*****************************************************************/
class FieldWindow:public BWindow {
	public:
		FieldWindow(BMessage *data);
		void SetDlg(AP_BeOSDialog_Field *brk);
		virtual void DispatchMessage(BMessage *msg, BHandler *handler);
		virtual bool QuitRequested(void);
		
	private:
		// Called to save pointers to the dialogs controls.. avoids multiple FindView calls.
				
		AP_BeOSDialog_Field *m_DlgField;
		
		sem_id modalSem;
		status_t WaitForDelete(sem_id blocker);
		
		BListView* typeList, *formatList;
		
		void SetTypesList(void);
		void SetFieldsList(void);
		void _FormatListBoxChange();
};

status_t FieldWindow::WaitForDelete(sem_id blocker)
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

FieldWindow::FieldWindow(BMessage *data) 
	  :BWindow(data) 
{

	
} //FieldWindow::FieldWindow

#define _DSB(viewname, string) ((BButton *)FindView(viewname))->SetLabel(pSS->getValue(XAP_STRING_ID_##string));
#define _DSV(viewname, string) ((BStringView *)FindView(viewname))->SetText(pSS->getValue(AP_STRING_ID_##string));

void FieldWindow::SetDlg(AP_BeOSDialog_Field *brk)
{
	m_DlgField = brk;
	
	
	
	const XAP_StringSet * pSS = m_DlgField->m_pApp->getStringSet();
	SetTitle( pSS->getValue(AP_STRING_ID_DLG_Field_FieldTitle));

	// Localize controls.
	_DSB("ok" , DLG_OK);
	_DSB("cancel" , DLG_Cancel);
	_DSV("type" , DLG_Field_Types);
	_DSV("format" , DLG_Field_Fields);
	
	// Save the pointers to the two lists.
	typeList = (BListView *)FindView("typeList");
	formatList = (BListView *)FindView("formatList");
	
	SetTypesList();
	SetFieldsList();
	
	formatList->Select(0);
	
	Show();		

	modalSem = create_sem(0,"FieldWindowSem");
	WaitForDelete(modalSem);
	
	Hide();
}


void FieldWindow::SetTypesList(void)
{
	for (int i = 0;fp_FieldTypes[i].m_Desc != NULL;i++) 
	{
		typeList->AddItem( new BStringItem(fp_FieldTypes[i].m_Desc));
	}

	typeList->Select(0);	

	m_DlgField->m_iTypeIndex = 0;
}

void FieldWindow::SetFieldsList(void)
{
	fp_FieldTypesEnum  FType = fp_FieldTypes[m_DlgField->m_iTypeIndex].m_Type;
	
	int i;
	int32 count = 0;
	count = formatList->CountItems();
	for (i = 0; i < count; i ++)
	{
		BStringItem* pItem = (BStringItem *)formatList->RemoveItem((long)0);
		delete pItem;
	}
	
	for (i = 0;fp_FieldFmts[i].m_Tag != NULL;i++) 
	{
		if( fp_FieldFmts[i].m_Type == FType )
			break;
	}
	for (;fp_FieldFmts[i].m_Tag != NULL && fp_FieldFmts[i].m_Type == FType;i++) 
	{
		formatList->AddItem( new BStringItem( fp_FieldFmts[i].m_Desc) );
	}
	
	formatList->Select(0);
}

void FieldWindow::_FormatListBoxChange(void)
{
	m_DlgField->m_iTypeIndex = typeList->CurrentSelection();
	fp_FieldTypesEnum  FType = fp_FieldTypes[m_DlgField->m_iTypeIndex].m_Type;

	int i;
	for (i = 0;fp_FieldFmts[i].m_Tag != NULL;i++) 
	{
		if( fp_FieldFmts[i].m_Type == FType )
			break;
	}

	m_DlgField->m_iFormatIndex = formatList->CurrentSelection() + i;
}

void FieldWindow::DispatchMessage(BMessage *msg, BHandler *handler)
{
	switch(msg->what) 
	{
		case 'type':
			_FormatListBoxChange();
			SetFieldsList();
			break;
		
		case 'frml':
			_FormatListBoxChange();
			break;
		
		case 'fmiv':
			// fall through..
		case 'okay':
			m_DlgField->setAnswer(AP_BeOSDialog_Field::a_OK);
			PostMessage(B_QUIT_REQUESTED);
			break;
			
		default:
			BWindow::DispatchMessage(msg, handler);
	}
} 

bool FieldWindow::QuitRequested()
{		
	delete_sem(modalSem);
	
	return(false);
}
/*****************************************************************/

XAP_Dialog * AP_BeOSDialog_Field::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_BeOSDialog_Field * p = new AP_BeOSDialog_Field(pFactory,id);
	return p;
}

AP_BeOSDialog_Field::AP_BeOSDialog_Field(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Field(pDlgFactory,id)
{
}

AP_BeOSDialog_Field::~AP_BeOSDialog_Field(void)
{
}

void AP_BeOSDialog_Field::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	BMessage msg;
	FieldWindow* newwin;
	
	if (RehydrateWindow("FieldWindow", &msg))
		{
        newwin = new FieldWindow(&msg);
        
		// Will change to OK if ok is hit...
        setAnswer(AP_BeOSDialog_Field::a_CANCEL);
		newwin->SetDlg(this);
		
		newwin->Lock();
		newwin->Quit();
        } 
}
