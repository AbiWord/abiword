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

private:
	AP_BeOSDialog_InsertHyperlink		*m_DlgHyperlink;
	
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
	// Let us suppose the dialog is 200x300
	dialogPosition.InsetBy(-(200-parentPosition.Width())/2, -(300-parentPosition.Height())/2);
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
	frame.InsetBy(10,10);
	frame.bottom -= 30.0;	// This space is for the buttons
	BTextControl *textcontrol = new BTextControl(frame, "URLBox", 
		pSS->getValue(AP_STRING_ID_DLG_InsertHyperlink_Msg), "", NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

	// Create a scrollView for the list
	panel->AddChild(textcontrol);
}

void HyperlinkWin::SetDlg(AP_BeOSDialog_InsertHyperlink *dlg , const XAP_StringSet *pSS) 
{
	m_DlgHyperlink = dlg;

//	const XAP_StringSet * pSS = dlg->m_pApp->getStringSet();
//	SetTitle( pSS->getValue(AP_STRING_ID_DLG_InsertHyperlink_Msg));

	SetTitle("InsertHyperlink");
	modalSem = create_sem(0,"InsertHyperlinkSem");
	Show();	
	// Localize controls.
/*
	_DSB("ok" , DLG_OK);
	_DSB("cancel" , DLG_Cancel);
	_DSV("type" , DLG_Field_Types);
	_DSV("format" , DLG_Field_Fields);
*/


	
	// Save the pointers to the two lists.
//	typeList = (BListView *)FindView("typeList");
//	formatList = (BListView *)FindView("formatList");

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

	HyperlinkWin  *newwin;
	char *res;

	switch(msg->what) {
	case _eventOK:
//		setAnswer(AP_Dialog_InsertHyperlink::a_OK);
		res = msg->FindString("res");
		delete_sem(modalSem);
//		setHyperlink((XML_Char*)res);
//		be_app->PostMessage(B_QUIT_REQUESTED);
		break;

	case _eventCancel:
//		setAnswer(AP_Dialog_InsertHyperlink::a_CANCEL);
		delete_sem(modalSem);
//		be_app->PostMessage(B_QUIT_REQUESTED);
		break;
	default:
		BWindow::DispatchMessage(msg, handler);
	}
}


bool HyperlinkWin::QuitRequested() {
	UT_ASSERT(m_DlgHyperlink);

//	delete_sem(modalSem);
	
	return(false);
}
/*
void AP_BeOSDialog_InsertHyperlink::_constructWindowContents ( GtkWidget * vbox2 )
{
  const XAP_StringSet * pSS = m_pApp->getStringSet();

  GtkWidget *label1;

  label1 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_InsertHyperlink_Msg));
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (vbox2), label1, TRUE, FALSE, 3);

  m_entry = gtk_entry_new();
  gtk_box_pack_start (GTK_BOX (vbox2), m_entry, FALSE, FALSE, 0);
  gtk_widget_show(m_entry);

  // the bookmark list
  m_swindow  = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (m_swindow),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_show(m_swindow);
  gtk_box_pack_start (GTK_BOX (vbox2), m_swindow, FALSE, FALSE, 0);
	
  m_clist = gtk_clist_new (1);
  gtk_clist_set_selection_mode(GTK_CLIST(m_clist), GTK_SELECTION_BROWSE);
  gtk_clist_column_titles_hide(GTK_CLIST(m_clist));
  //gtk_box_pack_start (GTK_BOX (vbox2), m_blist, FALSE, FALSE, 0);

  DELETEPV(m_pBookmarks);
  m_pBookmarks = new const XML_Char *[getExistingBookmarksCount()];
	
  for (int i = 0; i < (int)getExistingBookmarksCount(); i++)
  	  m_pBookmarks[i] = getNthExistingBookmark(i);

  int (*my_cmp)(const void *, const void *) =
  	  (int (*)(const void*, const void*)) UT_XML_strcmp;
    	
  qsort(m_pBookmarks, getExistingBookmarksCount(),sizeof(XML_Char*),my_cmp);

  for (int i = 0; i < (int)getExistingBookmarksCount(); i++)
  	  gtk_clist_append (GTK_CLIST (m_clist), (gchar**) &m_pBookmarks[i]);

  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(m_swindow),m_clist);

  const XML_Char * hyperlink = getHyperlink();
  if (hyperlink)
  {
	gtk_entry_set_text(GTK_ENTRY(m_entry), hyperlink);
// TODO uncomment line below when bug 920 gets fixed.
//	gtk_editable_select_region (GTK_EDITABLE(m_entry), 0, -1);
  }
}

GtkWidget*  AP_BeOSDialog_InsertHyperlink::_constructWindow(void)
{
  GtkWidget *vbox2;
  GtkWidget *frame1;
  GtkWidget *hbox1;
  GtkWidget *hseparator1;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  m_windowMain = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (m_windowMain), pSS->getValue(AP_STRING_ID_DLG_InsertHyperlink_Title));

  frame1 = gtk_frame_new (NULL);
  gtk_widget_show (frame1);
  gtk_container_add (GTK_CONTAINER (m_windowMain), frame1);
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 4);

  vbox2 = gtk_vbox_new (FALSE, 6);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (frame1), vbox2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);

  _constructWindowContents ( vbox2 );

  hseparator1 = gtk_hseparator_new ();
  gtk_widget_show (hseparator1);
  gtk_box_pack_start (GTK_BOX (vbox2), hseparator1, TRUE, TRUE, 0);

  hbox1 = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox1, TRUE, TRUE, 0);

  m_buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
  gtk_widget_show (m_buttonOK);
  gtk_box_pack_start (GTK_BOX (hbox1), m_buttonOK, FALSE, FALSE, 3);
  gtk_widget_set_usize (m_buttonOK, DEFAULT_BUTTON_WIDTH, 0);

  m_buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
  gtk_widget_show (m_buttonCancel);
  gtk_box_pack_start (GTK_BOX (hbox1), m_buttonCancel, FALSE, FALSE, 3);
  gtk_widget_set_usize (m_buttonCancel, DEFAULT_BUTTON_WIDTH, 0);

  gtk_widget_grab_focus (m_clist);
  gtk_widget_grab_default (m_buttonOK);

   return m_windowMain;
}
*/
/*
	gtk_signal_connect (GTK_OBJECT (m_clist), "select_row",
						GTK_SIGNAL_FUNC (s_blist_clicked), this);
					
	
	gtk_signal_connect_after(GTK_OBJECT(m_windowMain),
							 "destroy",
							 NULL,
							 NULL);
*/