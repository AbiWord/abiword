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
#include "ap_Dialog_InsertHyperlink.h"
#include "ap_BeOSDialog_InsertHyperlink.h"
#include "ap_BeOSDialog_All.h"

#include "ut_Rehydrate.h"

#include "StorageKit.h"
#include "SupportKit.h"
#include "fs_attr.h"

#define BOOKMARK_DIR "/boot/home/config/settings/NetPositive/Bookmarks"

/*****************************************************************/

class HyperlinkWin : public BWindow
{
public:
	HyperlinkWin(BRect &frame, const XAP_StringSet * pSS);
	void			SetDlg(AP_BeOSDialog_InsertHyperlink *dlg , const XAP_StringSet *pSS);
	virtual void	DispatchMessage(BMessage *msg, BHandler *handler);
	virtual bool	QuitRequested(void);
	static const int _eventOK = 'evok';
	static const int _eventCancel = 'evcl';
	static const int _eventSelected = 'slct';

private:
	AP_BeOSDialog_InsertHyperlink		*m_DlgHyperlink;
	virtual void _GenerateBookmarkList( BPath &path, uint32 level, BOutlineListView *listview );
	BTextControl		*m_CustomText;	
	BOutlineListView	*m_Listview;	
	sem_id modalSem;
	status_t WaitForDelete(sem_id blocker);
};

XAP_Dialog * AP_BeOSDialog_InsertHyperlink::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_BeOSDialog_InsertHyperlink * p = new AP_BeOSDialog_InsertHyperlink(pFactory,id);
	return p;
}

AP_BeOSDialog_InsertHyperlink::AP_BeOSDialog_InsertHyperlink(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_InsertHyperlink(pDlgFactory,id)
{
/*
	m_windowMain = 0;
	m_buttonOK = 0;
	m_buttonCancel = 0;
	//m_comboEntry = 0;
	m_clist = 0;
	m_pBookmarks = 0;
	m_iRow = -1;
	m_entry = 0;
*/
}

AP_BeOSDialog_InsertHyperlink::~AP_BeOSDialog_InsertHyperlink(void)
{
//	DELETEPV(m_pBookmarks);
}

/***********************************************************************/
void AP_BeOSDialog_InsertHyperlink::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	HyperlinkWin  *newwin;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	XAP_BeOSFrame * pBeOSFrame = static_cast<XAP_BeOSFrame *>(pFrame);
	BRect parentPosition = pBeOSFrame->getTopLevelWindow()->Frame();
	// Center the dialog according to the parent
	BRect dialogPosition = parentPosition;
	// Let us suppose the dialog is 300x350
	dialogPosition.InsetBy(-(300-parentPosition.Width())/2, -(350-parentPosition.Height())/2);
	newwin = new HyperlinkWin(dialogPosition, pSS);

	newwin->SetDlg(this , pSS);
	//Take the information here ...
	newwin->Lock();
	newwin->Quit();
}

/***********************************************************************/


//strings are not defined about InsertHyperlink
/*
#define _DSB(viewname, string) ((BButton *)FindView(viewname))->SetLabel(pSS->getValue(XAP_STRING_ID_##string));
#define _DSV(viewname, string) ((BStringView *)FindView(viewname))->SetText(pSS->getValue(AP_STRING_ID_##string));
*/


HyperlinkWin::HyperlinkWin(BRect &frame, const XAP_StringSet * pSS) :
	BWindow(frame, "HyperlinkWin", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS)
{
	SetTitle(pSS->getValue(AP_STRING_ID_DLG_InsertHyperlink_Title));
	SetSizeLimits(250,350,250,400);

	BView *panel = new BView(Bounds(), "HyperlinkPanel", B_FOLLOW_ALL_SIDES, B_FRAME_EVENTS | B_WILL_DRAW);
	panel->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(panel);

	BRect frame(10,Bounds().bottom-30, 80, 0);
	panel->AddChild(new BButton(frame, "cancelbutton", pSS->getValue(XAP_STRING_ID_DLG_Cancel),
				new BMessage(_eventCancel), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM));
	frame.OffsetBy(frame.Width() + 20, 0);
	panel->AddChild(new BButton(frame, "okbutton", pSS->getValue(XAP_STRING_ID_DLG_OK),
				new BMessage(_eventOK), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM));

	frame = Bounds();
	frame.top = 5;
	frame.left = 8;
	frame.bottom = frame.top + 20;
	frame.right -= 8;

	BStringView *stringview = new BStringView(frame, "HyperlinkText", 
		pSS->getValue(AP_STRING_ID_DLG_InsertHyperlink_Msg), B_FOLLOW_TOP, B_WILL_DRAW);
	panel->AddChild(stringview);

	frame = Bounds();
	frame.top = 30;
	frame.bottom = frame.top + 30;
	frame.left = 8;
	frame.right -= 8;

	m_CustomText = new BTextControl(frame, "HyperlinkTextControl", 
		NULL, NULL, NULL, B_FOLLOW_TOP | B_FOLLOW_LEFT | B_FOLLOW_RIGHT, B_WILL_DRAW);
	m_CustomText->SetDivider(0.0);
	panel->AddChild(m_CustomText);
	m_CustomText->MakeFocus(true);

	frame = Bounds();
	frame.top = 60;
	frame.bottom -= 40;
	frame.right -= (8 + B_V_SCROLL_BAR_WIDTH);
	frame.left = 8;

	m_Listview = new BOutlineListView(frame, "BookmarkListView", 
		B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES);
	m_Listview->SetSelectionMessage(new BMessage(_eventSelected)); 
		
	BScrollView *scroll=new BScrollView("scroll",m_Listview, B_FOLLOW_ALL_SIDES, 0,false,true);
	panel->AddChild(scroll);
}

void HyperlinkWin::SetDlg(AP_BeOSDialog_InsertHyperlink *dlg , const XAP_StringSet *pSS) 
{
	BPath path;
	m_DlgHyperlink = dlg;

	modalSem = create_sem(0,"InsertHyperlinkSem");
	Show();	
	Lock();
	path = BOOKMARK_DIR;
	_GenerateBookmarkList(path, 0, m_Listview);
	Unlock();
	WaitForDelete(modalSem);
	Hide();
}

status_t HyperlinkWin::WaitForDelete(sem_id blocker)
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
			pWin->Unlock();
			snooze(100);
			pWin->Lock();
			
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


void HyperlinkWin::DispatchMessage(BMessage *msg, BHandler *handler) {

	int32 selection;
	BPath path = "";
	BListItem *item, *superitem;
	BStringItem *sitem, *ssuperitem;
	BString str;
	BFile file;
	BEntry entry;
	entry_ref ref;
	attr_info	attribute;
	XML_Char *link;
	char	*url;

	switch(msg->what) {
	case _eventOK:
		if (m_CustomText->Text() != NULL)
		{
			link = m_CustomText->Text();
			m_DlgHyperlink->setHyperlink(link);
		}
		m_DlgHyperlink->setAnswer(AP_Dialog_InsertHyperlink::a_OK);
		be_app->PostMessage(B_QUIT_REQUESTED);
		break;
	case _eventCancel:
		be_app->PostMessage(B_QUIT_REQUESTED);
		break;
	case _eventSelected:
		selection = m_Listview->CurrentSelection(0);
		if (selection)
		{
			item = m_Listview->ItemAt(selection);
			sitem = dynamic_cast<BStringItem*>(item);
			str = sitem->Text();
			while(item->OutlineLevel() > 1)
			{
				superitem = m_Listview->Superitem(item);
				ssuperitem = dynamic_cast<BStringItem*>(superitem);
				str.Prepend("/");
				str.Prepend(ssuperitem->Text());
				item = superitem;
			}
		}
		printf("Bookmark %s Selected\n", str.String());
		str.Prepend("/");
		str.Prepend(BOOKMARK_DIR);
		path.SetTo(str.String());
		entry.SetTo( path.Path(), false );
		url = (char*)malloc(2048);

		if( entry.InitCheck() == B_OK ) 
		{
			if( entry.IsFile() )
			{
				file.SetTo( &entry, B_READ_ONLY );
				if( file.GetAttrInfo( "META:url", &attribute ) == B_NO_ERROR )
				{
					if( attribute.size > 2048 )
						url = (char *)realloc( url, attribute.size );
					file.ReadAttr( "META:url", B_STRING_TYPE, 0, url, attribute.size );
					printf("URL = %s\n", url);
					m_CustomText->SetText(url);
				}
			}
		}
		break;
	default:
		BWindow::DispatchMessage(msg, handler);
	}
}


bool HyperlinkWin::QuitRequested() {
	UT_ASSERT(m_DlgHyperlink);

	delete_sem(modalSem);
	
	return(false);
}

void HyperlinkWin::_GenerateBookmarkList( BPath &path, uint32 level, BOutlineListView *listview )
{
	BDirectory dir;
	BFile file;
	BEntry entry;
	entry_ref ref;
	char	*url;
	char filename[B_FILE_NAME_LENGTH];
	BPath underdir;

	dir.SetTo( path.Path() );

	url = (char*)malloc(1024);
	
	if (level == 0)
	{
		BStringItem *item = new BStringItem("Root", level, true);
		listview->AddItem(item);
		level++;
	}

	while( dir.GetNextEntry( &entry ) != B_ENTRY_NOT_FOUND )
	{
		if( entry.InitCheck() == B_OK ) 
		{
			entry.GetPath( &path );
			entry.GetName( filename );
			if( path.InitCheck() == B_OK ) 
			{
				BStringItem *item = new BStringItem(filename, level, false);
				listview->AddItem(item);
				if( ! entry.IsFile() )
				{
					underdir.SetTo(&dir, filename, false);
					Lock();
					_GenerateBookmarkList(underdir , level + 1, listview);
					Unlock();
				}
			}
		}
		snooze(100);
	}
}
