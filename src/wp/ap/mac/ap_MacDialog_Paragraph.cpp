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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_types.h"

#include "xap_App.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"

#include "ap_Dialog_Id.h"

#include "ap_Strings.h"

#ifndef XP_MAC_TARGET_QUARTZ
# include "gr_MacQDGraphics.h"
#else
# include "gr_MacGraphics.h"
#endif
#include "ap_Preview_Paragraph.h"
#include "ap_MacDialog_Paragraph.h"

#if 0
class Spinner : public BTextControl
{
public:
	Spinner(BRect frameRect , const char* name, const char* labal, const char* text , BMessage* message);
	virtual ~Spinner();

//	virtual void Draw(BRect frameRect);
	virtual void MakeFocus(bool focusState = true);	

protected:

};

Spinner::Spinner(BRect frameRect , const char* name, const char* labal, const char* text , BMessage* message) : BTextControl(frameRect , name , labal , text , message)
{}

Spinner::~Spinner()
{}

void Spinner::MakeFocus(bool isFocus)
{
	printf("Focus\n");
	BTextControl::MakeFocus(isFocus);
	
	if(!isFocus)
	{
		printf("Spinner loosing focus!\n");
		Window()->PostMessage('spie');
	}
}

class RedrawTabView : public BTabView
{
public:
	RedrawTabView(BRect frame);
	virtual ~RedrawTabView();
		
	virtual void Select(int32 tab);
	
};

RedrawTabView::RedrawTabView(BRect frameRect) : BTabView(frameRect, "TabView" , B_WIDTH_FROM_WIDEST)
{

}

RedrawTabView::~RedrawTabView()
{}

void RedrawTabView::Select(int32 tab)
{
	BTabView::Select(tab);
//	Window()->PostMessage('redr');
}

class UpdatingPreview : public BView
{
public:
	UpdatingPreview(BRect frameRect);
	virtual ~UpdatingPreview();
	
	virtual void Draw(BRect udr);
};

UpdatingPreview::UpdatingPreview(BRect frameRect) : BView(frameRect , "previewview" , B_FOLLOW_ALL_SIDES , B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE)
{
}

UpdatingPreview::~UpdatingPreview()
{
}

void UpdatingPreview::Draw(BRect udr)
{
	BView::Draw(udr);
	Window()->PostMessage('redr');
}

class ParagraphWin:public BWindow {
	public:
		ParagraphWin(BMessage *data);
		void SetDlg(AP_MacDialog_Paragraph *brk);
		virtual void DispatchMessage(BMessage *msg, BHandler *handler);
		virtual bool QuitRequested(void);
		
	private:
		AP_MacDialog_Paragraph 	*m_DlgParagraph;
		GR_MacGraphics				*m_MacGraphics;
				
		sem_id modalSem;
		status_t WaitForDelete(sem_id blocker);
		
		// Controls -- Just save them here, these FindView calls will add up if we don't
		void _saveControlPointers();
		
		// Need to find a way around making this public, but still being able to call _syncControls in populate.
	public:  
		BMenuField* listAlignment;

		// Indentation Box
		BTextControl* leftIndent, *rightIndent;
		BTextControl* byIndentation;
		BMenuField* specialIndentation;

		// Spacing box
		BTextControl* beforeSpacing, *afterSpacing;
		BTextControl* atSpacing;
		BMenuField* lineSpacing;
		
		// Line and page breaks tab -- more controls.
			
		BCheckBox *orphanControl;
		BCheckBox *keepNext;
		BCheckBox *keepLines;
		BCheckBox *pageBreak;
		BCheckBox *suppressLines;
		BCheckBox *noHyphenate;
};

status_t ParagraphWin::WaitForDelete(sem_id blocker)
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
			pWin->Unlock(); // Who will know?=)
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

ParagraphWin::ParagraphWin(BMessage *data) 
	  :BWindow(data) 
{
	
} //BreakWin::BreakWin

void ParagraphWin::SetDlg(AP_MacDialog_Paragraph *brk) 
{
	BRadioButton *on = NULL;

	m_DlgParagraph = brk;
	m_MacGraphics = NULL;
	
	// Default to returning cancel, this will change when tabs or ok is hit.
	m_DlgParagraph->setAnswer( AP_Dialog_Paragraph::a_CANCEL);

#if 0
	switch(m_DlgBreak->getBreakType()) {
	case  AP_Dialog_Break::b_PAGE:
		on = (BRadioButton *)FindView("radPageBreak");
		break;
	case  AP_Dialog_Break::b_COLUMN:
		on = (BRadioButton *)FindView("radColBreak");
		break;
	case  AP_Dialog_Break::b_NEXTPAGE:
		on = (BRadioButton *)FindView("radNextBreak");
		break;
	case  AP_Dialog_Break::b_CONTINUOUS:
		on = (BRadioButton *)FindView("radContBreak");
		break;
	case  AP_Dialog_Break::b_EVENPAGE:
		on = (BRadioButton *)FindView("radEvenBreak");
		break;
	case  AP_Dialog_Break::b_ODDPAGE:
		on = (BRadioButton *)FindView("radOddBreak");
		break;
	}


	if (on) {
		on->SetValue(B_CONTROL_ON);
	}
#endif

	// Here is the deal with the tab view, we need a notification when the user clicks another tab, now, since we don't have a 
	// tab selection message or anything we have to find a way to derive a class from it and override select.  The easiest way, 
	// make a class, create an identical tab view and then add the tabs to it, and finally remove the old tab view.
	
	BTabView* pView = (BTabView *)FindView("TabView");	
	BRect frameRect = pView->Frame();
	
	RedrawTabView* pNewTabView = new RedrawTabView(frameRect);
	BView* view1 = pView->TabAt(0)->View();//"Indents and Spacing"
	BView* view2 = pView->TabAt(1)->View();
	
	view1->RemoveSelf();
	view2->RemoveSelf();
	
	BTab* tab1 = pView->RemoveTab(0);
	BTab* tab2 = pView->RemoveTab(0);
	
	
	pNewTabView->AddTab(view1 , tab1);
	pNewTabView->AddTab(view2 , tab2);
	
	RemoveChild(pView);
	delete pView;
	
	AddChild(pNewTabView);

	// Now we need to bring the preview window to the top
	BView *preview=(BView*)FindView("previewview");	
	BView* previewnew = new UpdatingPreview(preview->Frame());

	// Clean up the place holder.
	preview->RemoveSelf();
	delete preview;
	
	// Add our new update-able view.
	AddChild(previewnew);
	preview = previewnew;
		
	_saveControlPointers();

	Show();

	//Create our preview window graphics
	m_MacGraphics  = new GR_MacGraphics(preview, m_DlgParagraph->m_pApp);

	if (preview->Window()->Lock())
	{
		m_DlgParagraph->_createPreviewFromGC(m_MacGraphics,preview->Frame().Width(),preview->Frame().Height());
		
		preview->Window()->Unlock();
	}

	
	
	m_DlgParagraph->_populateWindowData();
	
	modalSem = create_sem(0,"ParagraphSem");
	WaitForDelete(modalSem);
	
	Hide();
}

BView* FindViewTab(BWindow *pWin , BTabView* pView, char * title)
{
	BView* handle = pWin->FindView(title);
	if(!handle)
	{
		handle = pView->TabAt(0)->View()->FindView(title);
		if(!handle)
			handle = pView->TabAt(1)->View()->FindView(title);
	}
	
	return handle;
}
	
#include <assert.h>

#define RECREATE_TEXTCONTROL_AS_SPINNER(textControl) \
pParent = textControl->Parent();\
newControl = new Spinner(textControl->Frame() , textControl->Name(), textControl->Label() , textControl->Text(), new BMessage('spie'));\
newControl->SetDivider(textControl->Divider());\
pParent->RemoveChild(textControl);\
delete textControl;\
pParent->AddChild(newControl);\
textControl = newControl;

void ParagraphWin::_saveControlPointers()
{
	Spinner* newControl;
	BView* pParent;
	BTabView* pView = (BTabView *)FindView("TabView");
	
	listAlignment = (BMenuField *)FindViewTab(this, pView , "Alignment"); //( listAlignment , "Alignment")
	leftIndent  = (BTextControl *)FindViewTab(this, pView , "leftIndentation");
	RECREATE_TEXTCONTROL_AS_SPINNER(leftIndent);
	
	rightIndent = (BTextControl *)FindViewTab(this, pView , "rightIndentation");
	RECREATE_TEXTCONTROL_AS_SPINNER(rightIndent);
	
	specialIndentation = (BMenuField *)FindViewTab(this, pView , "specialIndentation");
	byIndentation = (BTextControl *)FindViewTab(this, pView , "byIndentation");
	RECREATE_TEXTCONTROL_AS_SPINNER(byIndentation);
	
	beforeSpacing = (BTextControl *)FindViewTab(this, pView , "beforeSpacing");
	RECREATE_TEXTCONTROL_AS_SPINNER(beforeSpacing);
	
	afterSpacing = (BTextControl *)FindViewTab(this, pView , "afterSpacing");
	RECREATE_TEXTCONTROL_AS_SPINNER(afterSpacing);
	
	lineSpacing = (BMenuField *)FindViewTab(this, pView , "lineSpacing");
	atSpacing = (BTextControl *)FindViewTab(this, pView , "atSpacing");
	RECREATE_TEXTCONTROL_AS_SPINNER(atSpacing);
	
	orphanControl = (BCheckBox *)FindViewTab(this, pView , "orphanControl");
	keepNext = (BCheckBox *)FindViewTab(this, pView , "keepNext");
	keepLines = (BCheckBox *)FindViewTab(this, pView , "keepLines");
	pageBreak = (BCheckBox *)FindViewTab(this, pView , "pageBreak");
	suppressLines = (BCheckBox *)FindViewTab(this, pView , "suppressLines");
	noHyphenate = (BCheckBox *)FindViewTab(this, pView , "noHyphenate");
	
	assert(listAlignment);
	assert(leftIndent);
	assert(rightIndent);
	assert(specialIndentation);
	assert(byIndentation);
	assert(beforeSpacing);
	assert(afterSpacing);
	assert(lineSpacing);
	assert(atSpacing);
	assert(orphanControl);
	assert(keepNext);
	assert(keepLines);
	assert(pageBreak);
	assert(suppressLines);
	assert(noHyphenate);
	
#if 0
	listAlignment = (BMenuField *)FindView("Alignment");
			BMenuField* listAlignment;
		
	leftIndent = (BTextControl *)FindView("leftIndentation");
	rightIndent = (BTextControl *)FindView("rightIndentation");
	specialIndentation = (BMenuField *)FindView("specialIndentation");
	byIndentation = (BTextControl *)FindView("byIndentation");
	beforeSpacing = (BTextControl *)FindView("beforeSpacing");
	afterSpacing = (BTextControl *)FindView("afterSpacing");
	lineSpacing = (BMenuField *)FindView("lineSpacing");
	atSpacing = (BTextControl *)FindView("atSpacing");
#endif


}
	
void ParagraphWin::DispatchMessage(BMessage *msg, BHandler *handler) 
{
	BMenuField* pMenu;
	BMenu* pMenuItem;
	int32 itemIndex, numItems;
	BCheckBox* checkBox;
	BTextControl *editBox;
	
	switch(msg->what) 
	{
	case 'tabb':
		// Tab button pressed.
		m_DlgParagraph->setAnswer(AP_Dialog_Paragraph::a_TABS);
		PostMessage(B_QUIT_REQUESTED);
		break;
	
	case 'okbu':
		// OK button pressed.
		m_DlgParagraph->setAnswer(AP_Dialog_Paragraph::a_OK);
		PostMessage(B_QUIT_REQUESTED);
		break;

		
	case 'redr':
		// Tab selection changed, redraw the preview.
		if(m_MacGraphics)
		{
			m_DlgParagraph->_redrawPreview();
		}
			break;
	
	case 'mspe':
		// This is called when the indentation -- special menufield is changed..
		if( msg->FindInt32("index" , &itemIndex) == B_OK)
		{
			m_DlgParagraph->_menuChanged(msg->what , itemIndex);
		}
		break;
		
	case 'mens':
		// This is called when spacing menufield is changed.
		if( msg->FindInt32("index" , &itemIndex) == B_OK)
		{
			m_DlgParagraph->_menuChanged(msg->what , itemIndex);
		}
		break;	
	
	case 'mena':
		// This is called when the alignment is changed.
		if( msg->FindInt32("index" , &itemIndex) == B_OK)
		{
			m_DlgParagraph->_menuChanged(msg->what , itemIndex);
		}
		break;		
	
	case 'pche':
		// This is called whenever any of the combo boxes are toggled on the line / page break page.
		if( msg->FindPointer("source" , (void **)&checkBox) == B_OK)
		{
			m_DlgParagraph->_checkBoxChanged(checkBox);
		}
		break;
			
	case 'spie':
		// This is called when a spinner is edited.
		if( msg->FindPointer("source" , (void **)&editBox) == B_OK)
		{
			m_DlgParagraph->_spinnerChanged(editBox);
		}
		break;
		
#if 0
	case 'btok':
		UT_DEBUGMSG(("BREAK: Set the page break \n"));
		UT_ASSERT(m_DlgParagraph);
		if (RAD_ON(on, "radPageBreak"))
 			m_DlgBreak->setBreakType(AP_Dialog_Break::b_PAGE);
		else if (RAD_ON(on, "radColBreak"))
 			m_DlgBreak->setBreakType(AP_Dialog_Break::b_COLUMN);
		else if (RAD_ON(on, "radNextBreak"))
 			m_DlgBreak->setBreakType(AP_Dialog_Break::b_NEXTPAGE);
		else if (RAD_ON(on, "radEvenBreak"))
 			m_DlgBreak->setBreakType(AP_Dialog_Break::b_EVENPAGE);
		else if (RAD_ON(on, "radOddBreak"))
 			m_DlgBreak->setBreakType(AP_Dialog_Break::b_ODDPAGE);
		else if (RAD_ON(on, "radContBreak"))
 			m_DlgBreak->setBreakType(AP_Dialog_Break::b_CONTINUOUS);

		m_DlgBreak->setAnswer(AP_Dialog_Break::a_OK);
		spin = 0;
		break;
#endif

	default:
		BWindow::DispatchMessage(msg, handler);
	}
} 

//Behave like a good citizen
bool ParagraphWin::QuitRequested()
{
//	UT_ASSERT(m_DlgParagraph);

	delete_sem(modalSem);
	return (false);
}

#endif

/*****************************************************************/

XAP_Dialog * AP_MacDialog_Paragraph::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	AP_MacDialog_Paragraph * p = new AP_MacDialog_Paragraph(pFactory,id);
	return p;
}

AP_MacDialog_Paragraph::AP_MacDialog_Paragraph(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_Paragraph(pDlgFactory,id)
{
}

AP_MacDialog_Paragraph::~AP_MacDialog_Paragraph(void)
{
//	DELETEP(m_pMacGraphics);
}

/*****************************************************************/

void AP_MacDialog_Paragraph::runModal(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;
	
	/*
	  This dialog is non-persistent.
	  
	  This dialog should do the following:

	  - Construct itself to represent the paragraph properties
	    in the base class (AP_Dialog_Paragraph).

		The Unix one looks just like Microsoft Word 97's Paragraph
		dialog.

	  - The base class stores all the paragraph parameters in
	    m_paragraphData.

	  On "OK" (or during user-interaction) the dialog should:

	  - Save all the data to the m_paragraphData struct so it
	    can be queried by the caller (edit methods routines).
	  
	  On "Cancel" the dialog should:

	  - Just quit, the data items will be ignored by the caller.

	  On "Tabs..." the dialog should (?):

	  - Just quit, discarding changed data, and let the caller (edit methods)
	    invoke the Tabs dialog.

	*/
        #if 0
	BMessage msg;
	if (RehydrateWindow("ParagraphWindow", &msg)) {
                newwin = new ParagraphWin(&msg);
		newwin->SetDlg(this);

		//Take the information here ...
		newwin->Lock();
		newwin->Close();
    
	     }
        #endif
	// TODO build the dialog, attach events, etc., etc.
}


void AP_MacDialog_Paragraph::_populateWindowData()
{
	// set the check boxes
	// TODO : handle tri-state boxes !!!
	
	//Todo : Localize controls..
#if 0
	ToggleCheckState( newwin->orphanControl , (_getCheckItemValue(id_CHECK_WIDOW_ORPHAN) == check_TRUE))
	ToggleCheckState( newwin->keepNext , (_getCheckItemValue(id_CHECK_KEEP_NEXT) == check_TRUE))
	ToggleCheckState( newwin->keepLines , (_getCheckItemValue(id_CHECK_KEEP_LINES) == check_TRUE))
	ToggleCheckState( newwin->pageBreak , (_getCheckItemValue(id_CHECK_PAGE_BREAK) == check_TRUE))
	ToggleCheckState( newwin->suppressLines , (_getCheckItemValue(id_CHECK_SUPPRESS) == check_TRUE))
	ToggleCheckState( newwin->noHyphenate , (_getCheckItemValue(id_CHECK_NO_HYPHENATE) == check_TRUE))
		
	newwin->listAlignment->Menu()->ItemAt(_getMenuItemValue(id_MENU_ALIGNMENT))->SetMarked(true);
	newwin->lineSpacing->Menu()->ItemAt(_getMenuItemValue(id_MENU_SPECIAL_SPACING))->SetMarked(true);
	newwin->specialIndentation->Menu()->ItemAt(_getMenuItemValue(id_MENU_SPECIAL_INDENT))->SetMarked(true);
	//				SendMessage(hwndAlign, CB_SETCURSEL, (WPARAM) _getMenuItemValue(id_MENU_ALIGNMENT), 0);	

#if 0
				HWND hwndHang = GetDlgItem(hWnd, AP_RID_DIALOG_PARA_COMBO_HANG);  
				_CAS(hwndHang, DLG_Para_SpecialNone);
				_CAS(hwndHang, DLG_Para_SpecialFirstLine);
				_CAS(hwndHang, DLG_Para_SpecialHanging);
				SendMessage(hwndHang, CB_SETCURSEL, (WPARAM) _getMenuItemValue(id_MENU_SPECIAL_INDENT), 0);	
#endif

#if 0
	TFToggleSetState(m_checkbuttonWidowOrphan,
						 (_getCheckItemValue(id_CHECK_WIDOW_ORPHAN) == check_TRUE));

	TFToggleSetState(m_checkbuttonKeepLines,
								 (_getCheckItemValue(id_CHECK_KEEP_LINES) == check_TRUE));
	TFToggleSetState(m_checkbuttonPageBreak,
								 (_getCheckItemValue(id_CHECK_PAGE_BREAK) == check_TRUE));
	TFToggleSetState(m_checkbuttonSuppress,
								 (_getCheckItemValue(id_CHECK_SUPPRESS) == check_TRUE));
	TFToggleSetState(m_checkbuttonHyphenate,
								 (_getCheckItemValue(id_CHECK_NO_HYPHENATE) == check_TRUE));
	TFToggleSetState(m_checkbuttonKeepNext,
								 (_getCheckItemValue(id_CHECK_KEEP_NEXT) == check_TRUE));
#endif
#endif
	_redrawPreview();
}

void AP_MacDialog_Paragraph::_redrawPreview()
{
	_syncControls(AP_Dialog_Paragraph::id_MENU_ALIGNMENT, UT_TRUE);
}

#if 0
void AP_MacDialog_Paragraph::_spinnerChanged(BTextControl *pControl)
{
	if(pControl == newwin->leftIndent)
		_setSpinItemValue(id_SPIN_LEFT_INDENT , pControl->Text());
	else if(pControl == newwin->rightIndent)
		_setSpinItemValue(id_SPIN_RIGHT_INDENT , pControl->Text());
	else if(pControl == newwin->byIndentation)
		_setSpinItemValue(id_SPIN_SPECIAL_INDENT , pControl->Text());
	else if(pControl == newwin->beforeSpacing)
		_setSpinItemValue(id_SPIN_BEFORE_SPACING , pControl->Text());
	else if(pControl == newwin->afterSpacing)
		_setSpinItemValue(id_SPIN_AFTER_SPACING , pControl->Text());
	else if(pControl == newwin->atSpacing)
		_setSpinItemValue(id_SPIN_SPECIAL_SPACING , pControl->Text());
	
}

#define _syncSPIN(w,i)	\
		case i:				\
			w->SetText(_getSpinItemValue(i) ); \
			break;			\

#endif

void AP_MacDialog_Paragraph::_syncControls(tControl changed, UT_Bool bAll /* = UT_FALSE */)
{ 
	// let parent sync any member variables first

// FIXIT	newwin->Lock();
	
	AP_Dialog_Paragraph::_syncControls(changed, bAll);

	// sync the display

	// 1.  link the "hanging indent by" combo and spinner


	if (bAll || (changed == id_SPIN_SPECIAL_INDENT))
	{
		// typing in the control can change the associated combo
		if (_getMenuItemValue(id_MENU_SPECIAL_INDENT) == indent_FIRSTLINE)
		{
			//HWND h = GetDlgItem(m_hwndSpacing, AP_RID_DIALOG_PARA_COMBO_HANG);							
			// FIXIT newwin->specialIndentation->Menu()->ItemAt(_getMenuItemValue(id_MENU_SPECIAL_INDENT))->SetMarked(true);//SendMessage(h, CB_SETCURSEL, (WPARAM) _getMenuItemValue(id_MENU_SPECIAL_INDENT), 0);
		}
	}
	if (bAll || (changed == id_MENU_SPECIAL_INDENT))
	{
		switch(_getMenuItemValue(id_MENU_SPECIAL_INDENT))
		{
		case indent_NONE:
			// clear the spin control
// FIXIT			newwin->byIndentation->SetText("");//SetDlgItemText(m_hwndSpacing, AP_RID_DIALOG_PARA_EDIT_BY, NULL);
			break;

		default:
			// set the spin control
// FIXIT			newwin->byIndentation->SetText( _getSpinItemValue(id_SPIN_SPECIAL_INDENT));
			//SetDlgItemText(m_hwndSpacing, AP_RID_DIALOG_PARA_EDIT_BY, _getSpinItemValue(id_SPIN_SPECIAL_INDENT));
			break;
		}
	}

	// 2.  link the "line spacing at" combo and spinner

	if (bAll || (changed == id_SPIN_SPECIAL_SPACING))
	{
		// typing in the control can change the associated combo
		if (_getMenuItemValue(id_MENU_SPECIAL_SPACING) == spacing_MULTIPLE)
		{
			//HWND h = GetDlgItem(m_hwndSpacing, AP_RID_DIALOG_PARA_COMBO_LEAD);							
// FIXIT			newwin->lineSpacing->Menu()->ItemAt( _getMenuItemValue(id_MENU_SPECIAL_SPACING))->SetMarked(true);//SendMessage(h, CB_SETCURSEL, (WPARAM) _getMenuItemValue(id_MENU_SPECIAL_SPACING), 0);
		}
	}
	
	if (bAll || (changed == id_MENU_SPECIAL_SPACING))
	{
		switch(_getMenuItemValue(id_MENU_SPECIAL_SPACING))
		{
		case spacing_SINGLE:
		case spacing_ONEANDHALF:
		case spacing_DOUBLE:
			// clear the spin control
			
// FIXIT			newwin->atSpacing->SetText("");
			// SetDlgItemText(m_hwndSpacing, AP_RID_DIALOG_PARA_EDIT_AT, NULL);
			break;

		default:
			// set the spin control
			// SetDlgItemText(m_hwndSpacing, AP_RID_DIALOG_PARA_EDIT_AT, _getSpinItemValue(id_SPIN_SPECIAL_SPACING));
// FIXIT			newwin->atSpacing->SetText(_getSpinItemValue(id_SPIN_SPECIAL_SPACING));

			break;
		}
	}

	// 3.  move results of _doSpin() back to screen

	if (!bAll)
	{
		// spin controls only sync when spun
		switch (changed)
		{
                #if 0 //FIXIT
		_syncSPIN(newwin->leftIndent,		id_SPIN_LEFT_INDENT)
		_syncSPIN(newwin->rightIndent,		id_SPIN_RIGHT_INDENT)
		_syncSPIN(newwin->byIndentation,	id_SPIN_SPECIAL_INDENT)
		_syncSPIN(newwin->beforeSpacing,	id_SPIN_BEFORE_SPACING)
		_syncSPIN(newwin->afterSpacing,		id_SPIN_AFTER_SPACING)
		_syncSPIN(newwin->atSpacing,		id_SPIN_SPECIAL_SPACING)
                #endif
		default:
			break;
		}
	}

//	newwin->Unlock();
	
	// TODO: see the latest version of the Win32 implementation for ideas
}
