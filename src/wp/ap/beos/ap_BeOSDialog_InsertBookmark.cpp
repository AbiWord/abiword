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
#include "ap_BeOSDialog_InsertBookmark.h"
#include "ap_BeOSDialog_All.h"

//#include "ut_Rehydrate.h"

#include "StorageKit.h"
#include "SupportKit.h"
#include "fs_attr.h"

#define BOOKMARK_DIR "/boot/home/config/settings/NetPositive/Bookmarks"


/*****************************************************************/

class BookmarkWin:public BWindow {
	public:
		BookmarkWin(BRect &frame, const XAP_StringSet * pSS);
		void SetDlg(AP_BeOSDialog_InsertBookmark *dlg, const XAP_StringSet *pSS);
		virtual void DispatchMessage(BMessage *msg, BHandler *handler);
		virtual bool QuitRequested(void);
		static const int _eventOK = 'evok';
		static const int _eventCancel = 'evcl';
		static const int _selected = 'slct';
		int32 selection;

	private:
		virtual void _GenerateBookmarkList( BPath &path, uint32 level, BOutlineListView *listview );

		AP_BeOSDialog_InsertBookmark 	*m_DlgBookmark;
		sem_id modalSem;
		status_t WaitForDelete(sem_id blocker);
};

status_t BookmarkWin::WaitForDelete(sem_id blocker)
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
			snooze(200);
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

void BookmarkWin::SetDlg(AP_BeOSDialog_InsertBookmark *dlg , const XAP_StringSet *pSS)
{
	BPath path;
	m_DlgBookmark = dlg;
	selection = -1;
//	const XAP_StringSet * pSS = dlg->m_pApp->getStringSet();
	
	// This semaphore ties up the window until after it deletes..
	modalSem = create_sem(0,"BookmarkModalSem");

	Show();
	Flush();
	Lock();
	path = BOOKMARK_DIR;
	BOutlineListView *listview = (BOutlineListView *)FindView("BookmarkListView");
	_GenerateBookmarkList(path, 0, listview);
	Unlock();

	WaitForDelete(modalSem);
	Hide();
}

void BookmarkWin::DispatchMessage(BMessage *msg, BHandler *handler) {

	int32 selection;
	BPath path = "";
	BListItem *item, *superitem;
	BStringItem *sitem, *ssuperitem;
	BString str;
	BFile file;
	BEntry entry;
	entry_ref ref;
	attr_info	attribute;
	char	*url;

	BOutlineListView *listview = (BOutlineListView *)FindView("BookmarkListView");

	switch(msg->what) {
	case _eventOK:

		selection = listview->CurrentSelection(0);
		if (selection)
		{
			item = listview->ItemAt(selection);
			sitem = dynamic_cast<BStringItem*>(item);
			str = sitem->Text();
			while(item->OutlineLevel() > 1)
			{
				superitem = listview->Superitem(item);
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
		url = (char*)malloc(1024);

		if( entry.InitCheck() == B_OK ) 
		{
			if( entry.IsFile() )
			{
				file.SetTo( &entry, B_READ_ONLY );
				if( file.GetAttrInfo( "META:title", &attribute ) == B_NO_ERROR )
				{
					if( attribute.size > 1024 )
						url = (char *)realloc( url, attribute.size );
					file.ReadAttr( "META:url", B_STRING_TYPE, 0, url, attribute.size );
					printf("URL = %s\n", url);
				}
			}
		}
	case _eventCancel:
		be_app->PostMessage(B_QUIT_REQUESTED);
		break;
	default:
		BWindow::DispatchMessage(msg, handler);
	}
}


BookmarkWin::BookmarkWin(BRect &frame, const XAP_StringSet * pSS) :
	BWindow(frame, "BookmarkWin", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS)
{
	SetTitle(pSS->getValue(AP_STRING_ID_DLG_InsertBookmark_Title));

	BView *panel = new BView(Bounds(), "BookmarkPanel", B_FOLLOW_ALL_SIDES, B_FRAME_EVENTS | B_WILL_DRAW);
	panel->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(panel);

	BRect frame(10,Bounds().bottom-30, 80, 0);
	panel->AddChild(new BButton(frame, "cancelbutton", pSS->getValue(XAP_STRING_ID_DLG_Cancel),
				new BMessage(_eventCancel), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM));
	frame.OffsetBy(frame.Width() + 20, 0);
	panel->AddChild(new BButton(frame, "okbutton", pSS->getValue(XAP_STRING_ID_DLG_OK),
				new BMessage(_eventOK), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM));

	frame = Bounds();
	frame.InsetBy(10,5);
	frame.bottom = frame.top + 20;

	BStringView *stringview = new BStringView(frame, "BookmarkText", 
		pSS->getValue(AP_STRING_ID_DLG_InsertHyperlink_Msg), B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	panel->AddChild(stringview);

	frame = Bounds();
	frame.InsetBy(0,30);
	frame.bottom -= 30.0;	// This space is for the buttons
	frame.right -= 8+B_V_SCROLL_BAR_WIDTH;
	frame.left += 8;

	BOutlineListView *listview = new BOutlineListView(frame, "BookmarkListView", 
		B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES);
		
	BScrollView *scroll=new BScrollView("scroll",listview, B_FOLLOW_LEFT | B_FOLLOW_TOP, 0,false,true);
	panel->AddChild(scroll);
}

void BookmarkWin::_GenerateBookmarkList( BPath &path, uint32 level, BOutlineListView *listview )
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


bool BookmarkWin::QuitRequested() {
	UT_ASSERT(m_DlgBookmark);

	delete_sem(modalSem);
	
	return(false);
}

XAP_Dialog * AP_BeOSDialog_InsertBookmark::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_BeOSDialog_InsertBookmark * p = new AP_BeOSDialog_InsertBookmark(pFactory,id);
	int m_pBookmarks = 0;
	return p;
}

AP_BeOSDialog_InsertBookmark::AP_BeOSDialog_InsertBookmark(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_InsertBookmark(pDlgFactory,id)
{
}

AP_BeOSDialog_InsertBookmark::~AP_BeOSDialog_InsertBookmark(void)
{
}

void AP_BeOSDialog_InsertBookmark::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	BookmarkWin  *newwin;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	XAP_BeOSFrame * pBeOSFrame = static_cast<XAP_BeOSFrame *>(pFrame);
	BRect parentPosition = pBeOSFrame->getTopLevelWindow()->Frame();
	// Center the dialog according to the parent
	BRect dialogPosition = parentPosition;
	// Let us suppose the dialog is 300x350
	dialogPosition.InsetBy(-(300-parentPosition.Width())/2, -(350-parentPosition.Height())/2);
	newwin = new BookmarkWin(dialogPosition, pSS);

	newwin->SetDlg(this , pSS);
	setBookmark("fgfgf");
	printf("setbookmark\n");

	//Take the information here ...
	newwin->Lock();
	newwin->Quit();
}
