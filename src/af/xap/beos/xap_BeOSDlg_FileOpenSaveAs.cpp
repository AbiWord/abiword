/* AbiSource Application Framework
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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_BeOSDlg_FileOpenSaveAs.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include <InterfaceDefs.h>
sem_id					sync_sem = -1;

/*****************************************************************/
//This function is the one that sits and waits and waits ...

class DLGHandler: public BHandler {
	public:
		DLGHandler(XAP_BeOSDialog_FileOpenSaveAs *dlg, const char *name = NULL);
		virtual void MessageReceived(BMessage *msg);
	private:
		XAP_BeOSDialog_FileOpenSaveAs *m_pDlg;
		int closed;
};

DLGHandler::DLGHandler(XAP_BeOSDialog_FileOpenSaveAs *dlg, const char *name)
		   : BHandler(name) {
	m_pDlg = dlg;	
	be_app->Lock();
	be_app->AddHandler(this);
	be_app->Unlock();
	closed = 0;
}

void DLGHandler::MessageReceived(BMessage *msg) {
	switch (msg->what) 
	{
		case 'styp':
		{
		
		// Called when they change the filetype.
		
		int32 index;
		if( msg->FindInt32("index" , &index) == B_OK)
		{
				printf("Filetype changed. = %d\n" , index);
				m_pDlg->SetFileTypeIndex(index);//m_nFileType = 3;
		}
		
		return;
		}
		
	case 'fopn': {
		entry_ref ref;
		msg->FindRef("refs", &ref);
		BEntry  entry(&ref);
		BPath	path;
		entry.GetPath(&path);
		m_pDlg->SetAnswer(XAP_Dialog_FileOpenSaveAs::a_OK);
		m_pDlg->SetPathname(path.Path());
		break;
	}
	case 'fsve': {		//Check "name" and "directory"
		entry_ref ref;
		const char *name;
		msg->FindRef("directory", &ref);
		BDirectory  dir(&ref);
		BPath path(&dir, NULL, false);
		msg->FindString("name", &name);
		path.Append(name);
		m_pDlg->SetAnswer(XAP_Dialog_FileOpenSaveAs::a_OK);
		m_pDlg->SetPathname(path.Path());
		break;
	}
	case B_CANCEL:
		m_pDlg->SetAnswer(XAP_Dialog_FileOpenSaveAs::a_CANCEL);
		break;
	default:
		BHandler::MessageReceived(msg);
		return;
	}
	#if 0 // Do something pretty instead.
    	release_sem(sync_sem); 
	#else
		delete_sem(sync_sem);
	#endif
}

/*****************************************************************/
XAP_Dialog * XAP_BeOSDialog_FileOpenSaveAs::static_constructor(XAP_DialogFactory * pFactory,
															 XAP_Dialog_Id id)
{
	XAP_BeOSDialog_FileOpenSaveAs * p = new XAP_BeOSDialog_FileOpenSaveAs(pFactory,id);
	return p;
}

XAP_BeOSDialog_FileOpenSaveAs::XAP_BeOSDialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_FileOpenSaveAs(pDlgFactory,id)
{
	m_pOpenPanel = NULL;
	m_pSavePanel = NULL;

	//This isn't so good
//	if (sync_sem < 0)

	m_pHandler = new DLGHandler(this, "Dialog Handler");
}

XAP_BeOSDialog_FileOpenSaveAs::~XAP_BeOSDialog_FileOpenSaveAs(void)
{
	delete m_pHandler;
	m_pHandler = NULL;
}

void XAP_BeOSDialog_FileOpenSaveAs::SetFileTypeIndex(UT_sint32 newIndex)
{
	m_nFileType = m_nTypeList[newIndex];
}

void XAP_BeOSDialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	m_pBeOSFrame = (XAP_BeOSFrame*)pFrame;
	UT_ASSERT(m_pBeOSFrame);
	
	sync_sem = create_sem(0, "sync_sem");
	
	// do we want to let this function handle stating the BeOS
	// directory for writability?  Save/Export operations will want
	// this, open/import will not.

	UT_Bool bCheckWritePermission;
	char * szTitle;
	BMessenger *messenger = new BMessenger(m_pHandler);
	m_pOpenPanel = m_pSavePanel = NULL;
		
	switch (m_id) {
	case XAP_DIALOG_ID_INSERT_PICTURE:
	case XAP_DIALOG_ID_FILE_OPEN: {
		szTitle = "AbiWord - Open File";
		bCheckWritePermission = UT_FALSE;
		if (!m_pOpenPanel) {
			//BMessenger tmpMessenger(this);
		 	m_pOpenPanel = new BFilePanel(B_OPEN_PANEL, 	//Mode SAVE/OPEN
          								  	messenger,			//Target BMessenger
         								  	NULL, 			//entry_ref* directory
         								  	0, 				//Node flavours
         								  	false, 			//multiselect
         								  	new BMessage('fopn'), 			//BMessage
         								  	NULL, 			//BRefFilter
         								  	true, 			//modal
         								  	false);			//hide when done
			m_pOpenPanel->Window()->SetTitle("Open your file");
		}
		m_pOpenPanel->Show();
		break;
	}
	case XAP_DIALOG_ID_FILE_SAVEAS: 
	{
		
		szTitle = "AbiWord - Save File As";
		bCheckWritePermission = UT_TRUE;
		if (!m_pSavePanel) {
			//BMessenger tmpMessenger(this);
		 	m_pSavePanel = new BFilePanel(B_SAVE_PANEL, 	//Mode SAVE/OPEN
       									  messenger,				//Target BMessenger
       									  NULL, 			//entry_ref* directory
       									  0, 				//Node flavours
       									  false, 			//multiselect
       									  new BMessage('fsve'), 			//BMessage
       									  NULL, 			//BRefFilter
       									  true, 			//modal
       									  false);			//hide when done
			m_pSavePanel->Window()->SetTitle("Save your file");
			
			if( m_pSavePanel->Window()->Lock())
			{
				// Add our sweet file type selection list to the dialog.
				// We put it 10 points to the right of the filename.
			
				BRect saveTypeRect = m_pSavePanel->Window()->FindView("text view")->Frame();
				saveTypeRect.left = saveTypeRect.right + 10.0;
				saveTypeRect.right = saveTypeRect.left + 250.0;
	
				BPopUpMenu* pPopup = new BPopUpMenu("typeListMenu");
				BMenuField* typeList = new BMenuField(saveTypeRect , "typeList" , "Save as Type:" , pPopup , B_FOLLOW_LEFT | B_FOLLOW_BOTTOM , B_WILL_DRAW);
				typeList->SetDivider( typeList->StringWidth("Save as Type:") + 13);
				typeList->SetViewColor(m_pSavePanel->Window()->ChildAt(0)->ViewColor());
				m_pSavePanel->Window()->ChildAt(0)->AddChild(typeList);
				m_pSavePanel->Window()->Unlock();
				
				for(int i = 0; m_szDescriptions[i] != '\0'; i ++)
				{
					BMenuItem* newItem = new BMenuItem(m_szDescriptions[i] , new BMessage('styp'));
					newItem->SetTarget(*messenger);
					pPopup->AddItem(newItem);
					
					if( m_nTypeList[i] == m_nDefaultFileType)
						newItem->SetMarked(true);
				}
			}
		}
		
		if(m_szInitialPathname)
		{
			BPath parent;
			BPath* fullPath = new BPath(m_szInitialPathname);
			
			if( fullPath->GetParent(&parent) == B_OK)
			{
				m_pSavePanel->SetSaveText(fullPath->Leaf());
				m_pSavePanel->SetPanelDirectory(parent.Path());
			}
			
			delete fullPath;
		}
		
		m_pSavePanel->Show();
		break;
	}
	case XAP_DIALOG_ID_PRINTTOFILE: {
		szTitle = "Print To File";
		bCheckWritePermission = UT_TRUE;
		break;
	}
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	

	//Wait for the pannels to be finished
#if 0 // Instead of just waiting, make the windows look pretty.
 	acquire_sem(sync_sem);
#else
	status_t	result;
	thread_id	this_tid = find_thread(NULL);
	BLooper		*pLoop;
	BWindow		*pWin = 0;

	pLoop = BLooper::LooperForThread(this_tid);
	if (pLoop)
		pWin = dynamic_cast<BWindow*>(pLoop);

	// block until semaphore is deleted (modal is finished)
	if (pWin) 
	{
		do {
			// update the window periodically			
			pWin->UpdateIfNeeded();
			result = acquire_sem_etc(sync_sem, 1, B_TIMEOUT, 10000);
		} while (result != B_BAD_SEM_ID);
	} else 
	{
		do 
		{
			// just wait for exit
			result = acquire_sem(sync_sem);
		} while (result != B_BAD_SEM_ID);
	}
#endif
 	
	delete m_pSavePanel;
	delete m_pOpenPanel;

	//Need to sleep on some sort of semaphore here ...
	//UT_cloneString(m_szFinalPathname, "/boot/home/tfletche/junk.abw");		
/*
	if (m_answer == a_OK)
		UT_cloneString(m_szFinalPathname, "junk.abw");		
	m_answer = a_OK;		//vs a_CANCEL 
*/

	return;
}
