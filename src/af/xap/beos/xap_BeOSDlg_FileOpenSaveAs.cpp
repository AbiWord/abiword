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
	switch (msg->what) {
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
		char *name = new char[B_FILE_NAME_LENGTH];
		msg->FindRef("directory", &ref);
		BDirectory  dir(&ref);
		BPath path(&dir, NULL, false);
		msg->FindString("name", &name);
		path.Append(name);
		m_pDlg->SetAnswer(XAP_Dialog_FileOpenSaveAs::a_OK);
		m_pDlg->SetPathname(path.Path());
		delete [] name;
		break;
	}
	case B_CANCEL:
		m_pDlg->SetAnswer(XAP_Dialog_FileOpenSaveAs::a_CANCEL);
		break;
	default:
		BHandler::MessageReceived(msg);
		return;
	}
    release_sem(sync_sem); 
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
	if (sync_sem < 0)
		sync_sem = create_sem(0, "sync_sem");
	m_pHandler = new DLGHandler(this, "Dialog Handler");
}

XAP_BeOSDialog_FileOpenSaveAs::~XAP_BeOSDialog_FileOpenSaveAs(void)
{
	delete m_pHandler;
}

void XAP_BeOSDialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	m_pBeOSFrame = (XAP_BeOSFrame*)pFrame;
	UT_ASSERT(m_pBeOSFrame);

	// do we want to let this function handle stating the BeOS
	// directory for writability?  Save/Export operations will want
	// this, open/import will not.

	UT_Bool bCheckWritePermission;
	char * szTitle;
	BMessenger *messenger = new BMessenger(m_pHandler);
	m_pOpenPanel = m_pSavePanel = NULL;
		
	switch (m_id) {
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
         								  	false, 			//modal
         								  	false);			//hide when done
			m_pOpenPanel->Window()->SetTitle("Open your file");
		}
		m_pOpenPanel->Show();
		break;
	}
	case XAP_DIALOG_ID_FILE_SAVEAS: {
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
       									  false, 			//modal
       									  false);			//hide when done
			m_pSavePanel->Window()->SetTitle("Save your file");
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
 	acquire_sem(sync_sem);
	delete m_pHandler;
	m_pHandler = NULL;
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
