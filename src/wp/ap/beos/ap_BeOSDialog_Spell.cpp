/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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
#include "ap_Dialog_Spell.h"
#include "ap_BeOSDialog_Spell.h"

#include "ut_Rehydrate.h"

// We really shouldn't unlock the window lock, but the redraw when we focus on the next
// misspelled word will deadlock if we don't..

status_t WaitForDelete(sem_id blocker)
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
			pWin->Unlock(); // Who will know?=)
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

/*****************************************************************/

class SpellWin:public BWindow {
	public:
		SpellWin(BMessage *data);
		void SetDlg(AP_BeOSDialog_Spell *dlg);
		virtual void DispatchMessage(BMessage *msg, BHandler *handler);
		virtual bool QuitRequested(void);
		
	private:
		AP_BeOSDialog_Spell 	*m_DlgSpell;
				
		void _showMisspelledWord();
		void _suggestChange();
		void _toggleChangeButtons(bool shouldShow);
		void _change();
		void _tryAgain();
		void _changeAll();
		
		rgb_color normalText, badText;
		sem_id modalSem;
};

SpellWin::SpellWin(BMessage *data) 
	  :BWindow(data)
{

} 

void SpellWin::SetDlg(AP_BeOSDialog_Spell *dlg)
{
	m_DlgSpell = dlg;

	// As nice as using InterfaceElements is, the app doesn't properly set up
	// the textRect for our BTextView..
	BTextView* sentenceView = (BTextView *)FindView("sentencedisplay");
	BRect frameRect = sentenceView->Bounds();
	frameRect.InsetBy(5,5);
	
	sentenceView->SetTextRect(frameRect);
		
	normalText.red = normalText.blue = normalText.green = 0;
	normalText.alpha = 255;

	badText.red = 255;
	badText.blue = badText.green = 0;
	badText.alpha = 255;
	
	bool bRes = m_DlgSpell->nextMisspelledWord();
	if(!bRes)
		return;
		
	// set initial state
	m_DlgSpell->makeWordVisible();
	
	_showMisspelledWord();
	
	// This semaphore ties up the window until after it deletes..
	modalSem = create_sem(0,"SpellModalSem");

	Show();

	WaitForDelete(modalSem);
}

void SpellWin::_suggestChange()
{
	BListView* suggestList = (BListView *)FindView("suggestlist");
	BTextControl* changeText = (BTextControl *)FindView("changetxt");
	
	m_DlgSpell->m_iSelectedRow = suggestList->CurrentSelection();
	
	if (!m_DlgSpell->m_Suggestions.count) 
	{
		// no change to suggest, ignore it
		if (m_DlgSpell->m_iSelectedRow != -1)
			suggestList->Select(-1);
	}
	else
	{
		// copy suggestion to edit field
		UT_ASSERT((m_DlgSpell->m_iSelectedRow > -1));

		BStringItem* selectedItem = (BStringItem *)suggestList->ItemAt(m_DlgSpell->m_iSelectedRow);
		changeText->SetText(selectedItem->Text());
	}

}

void SpellWin::_change(void)
{
	UT_UCSChar * replace = NULL;
	BTextControl* changeText = (BTextControl *)FindView("changetxt");

	if (m_DlgSpell->m_iSelectedRow != -1)
	{
		replace = m_DlgSpell->m_Suggestions.word[m_DlgSpell->m_iSelectedRow];
		m_DlgSpell->changeWordWith(replace);
	}
	else
	{
		UT_UCS_cloneString_char(&replace, changeText->Text());
		if (!UT_UCS_strlen(replace)) 
		{
			return;
		}
		m_DlgSpell->changeWordWith(replace);
	}

	_tryAgain();
}


void SpellWin::_tryAgain(void)
{
	BListView* suggestList = (BListView *)FindView("suggestlist");

	// clear prior suggestions
	m_DlgSpell->_purgeSuggestions();
	
	int32 numItems = suggestList->CountItems();
	
	// Clear out the dialog
	for ( int32 i = 0; i < numItems; i++ )
	{
		BListItem* pItem = suggestList->RemoveItem((long int)0);
		delete pItem;
	}

	// what's next
	bool bRes = m_DlgSpell->nextMisspelledWord();

	if (bRes)
	{
		// show word in main window
		m_DlgSpell->makeWordVisible();

		// update dialog with new misspelled word info/suggestions
		_showMisspelledWord();
	}
	else
	{
		PostMessage(B_QUIT_REQUESTED);
	}
}

void SpellWin::_changeAll(void)
{
	BTextControl* changeText = (BTextControl *)FindView("changetxt");
	UT_UCSChar * replace = NULL;
	
	if (m_DlgSpell->m_iSelectedRow != -1)
	{
		replace = (UT_UCSChar*) m_DlgSpell->m_Suggestions.word[m_DlgSpell->m_iSelectedRow];
		m_DlgSpell->addChangeAll(replace);
		m_DlgSpell->changeWordWith(replace);
	}
	else
	{
		UT_UCS_cloneString_char(&replace,changeText->Text());
		if (!UT_UCS_strlen(replace)) 
		{
			return;
		}
			
		m_DlgSpell->addChangeAll(replace);
		m_DlgSpell->changeWordWith(replace);
	}

	_tryAgain();
}

void SpellWin::_showMisspelledWord(void)
{
	UT_UCSChar *p;
	UT_uint32 len;
	char * buf;
	UT_uint32 sum = 0;

	BTextView* sentenceView = (BTextView *)FindView("sentencedisplay");
	BListView* suggestList = (BListView *)FindView("suggestlist");
	
	sentenceView->SetText("");
	sentenceView->MakeEditable(true);

	// Run array is 3 long because begin sentence black, misspelled word red, end sentence black..
	// Note: We need to delete this when we are done..
	text_run_array* array = (text_run_array *)malloc(sizeof(text_run_array) + sizeof(text_run) * 3);
	array->count = 3;

	array->runs[0].offset = 0;
	array->runs[0].font = be_plain_font;
	array->runs[0].color = normalText;
	
	array->runs[1].font = be_bold_font;
	array->runs[1].color = badText;

	array->runs[2].font = be_plain_font;
	array->runs[2].color = normalText;

	// insert start of sentence

	p = m_DlgSpell->_getPreWord();
	len = UT_UCS_strlen(p);
	if (len)
	{
		buf = new char [len + 1];
		UT_UCS_strcpy_to_char(buf, p);
		sentenceView->Insert(buf, (long int)len);
		DELETEP(buf);
	}
	FREEP(p);
	sum += len;

	array->runs[1].offset = sum;
	
	p = m_DlgSpell->_getCurrentWord();
	len = UT_UCS_strlen(p);
	if (len)
	{
		buf = new char [len + 1];
		UT_UCS_strcpy_to_char(buf, p);
		sentenceView->Insert(sum, buf, (long int)len);
		DELETEP(buf);
	}
	FREEP(p);
	sum += len;

	array->runs[2].offset = sum;

	p = m_DlgSpell->_getPostWord();
	len = UT_UCS_strlen(p);
	if (len)
	{
		buf = new char [len + 1];
		UT_UCS_strcpy_to_char(buf, p);
		sentenceView->Insert(sum, buf, (long int)len);
		DELETEP(buf);
	}
	FREEP(p);
	
	sentenceView->SetRunArray(0, (long int)sum + len , array);
	sentenceView->MakeEditable(false);

	// insert suggestions
	if (!m_DlgSpell->m_Suggestions.count) 
	{
		const XAP_StringSet * pSS = m_DlgSpell->m_pApp->getStringSet();
		BStringItem* emptyItem = new BStringItem(pSS->getValue(AP_STRING_ID_DLG_Spell_NoSuggestions));
		suggestList->AddItem(emptyItem);
		m_DlgSpell->m_iSelectedRow = -1;
		_toggleChangeButtons(false);
	} 
	else 
	{
		for (int i = 0; i < m_DlgSpell->m_Suggestions.count; i++)
		{
			p = (UT_UCSChar *) m_DlgSpell->m_Suggestions.word[i];
			len = UT_UCS_strlen(p);
			if (len)
			{
				buf = new char [len + 1];
				UT_UCS_strcpy_to_char(buf, p);
				BStringItem* pItem = new BStringItem(buf);
				suggestList->AddItem(pItem);
				DELETEP(buf);
			}
		}

		m_DlgSpell->m_iSelectedRow = 0;
		_toggleChangeButtons(true);
	}

	suggestList->Select(0);
	_suggestChange();
}

void SpellWin::DispatchMessage(BMessage *msg, BHandler *handler) 
{
	BTextControl* editSuggest;
	BListView* suggestList;
	int32 selIndex;
	BStringItem* pCurSelItem;
	
	switch(msg->what) 
	{
	case 'ibut': // ignore button
		m_DlgSpell->ignoreWord();
		_tryAgain();
		break;
	
	case 'iabu': // ignore all button.
		m_DlgSpell->addIgnoreAll();
		m_DlgSpell->ignoreWord();
		_tryAgain();
		break;
		
	case 'abut': // add button
		m_DlgSpell->addToDict();
		m_DlgSpell->ignoreWord();
		_tryAgain();
		break;
		
	case 'cbut':
		_change();
		break;
		
	case 'cabu':
		_changeAll();
		break;
	
	case 'selc':
		// Update the current selection in the add box..
		suggestList = (BListView *)FindView("suggestlist");
		editSuggest = (BTextControl *)FindView("changetxt");
	
		m_DlgSpell->m_iSelectedRow = suggestList->CurrentSelection();
		selIndex = m_DlgSpell->m_iSelectedRow;
		
		pCurSelItem = (BStringItem *)suggestList->ItemAt(m_DlgSpell->m_iSelectedRow);
	
		if(pCurSelItem)
			editSuggest->TextView()->SetText(pCurSelItem->Text());

		break;
		
	case 'invs': // Called when the user double-clicks a suggested word in the spell dialog.
		suggestList = (BListView *)FindView("suggestlist");
		if( msg->FindInt32("index" , &selIndex) == B_OK)
		{
			m_DlgSpell->m_iSelectedRow = selIndex;
			_change();
		}
		break;
	
	case 'spmo': // Whenever the user edits the text control.

		// Determine if the user clicked an item in the suggest box.
		
		suggestList = (BListView *)FindView("suggestlist");
		editSuggest = (BTextControl *)FindView("changetxt");

		for(int i = 0; i < suggestList->CountItems(); i ++)
		{
			pCurSelItem = (BStringItem *)suggestList->ItemAt(i);

			if(strcmp(pCurSelItem->Text() , editSuggest->Text()) == 0)
			{
				return;;
			}
		}
		
		// The user is editing it so, enable the change button.
		_toggleChangeButtons(true);
		
		suggestList->DeselectAll();
		m_DlgSpell->m_iSelectedRow = -1;
		
		break;
		
	default:
		BWindow::DispatchMessage(msg, handler);
	}
	
} 

//Behave like a good citizen
bool SpellWin::QuitRequested()
{
	delete_sem(modalSem);
	return(true);
}

void SpellWin::_toggleChangeButtons(bool shouldShow)
{
	BButton *change, *changeall;
	change = (BButton *)FindView("change");
	changeall = (BButton *)FindView("changeall");
	
	if(change)
		change->SetEnabled(shouldShow);
	if(changeall)
		changeall->SetEnabled(shouldShow);
}

/*****************************************************************/

XAP_Dialog * AP_BeOSDialog_Spell::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
   AP_BeOSDialog_Spell * p = new AP_BeOSDialog_Spell(pFactory,id);
   return p;
}

AP_BeOSDialog_Spell::AP_BeOSDialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : AP_Dialog_Spell(pDlgFactory,id)
{
}

AP_BeOSDialog_Spell::~AP_BeOSDialog_Spell(void)
{
}

/************************************************************/
void AP_BeOSDialog_Spell::runModal(XAP_Frame * pFrame)
{
   // call the base class method to initialize some basic xp stuff
   AP_Dialog_Spell::runModal(pFrame);

   m_bCancelled = false;

	BMessage msg;
        SpellWin  *newwin;
        if (RehydrateWindow("SpellWindow", &msg)) {
                newwin = new SpellWin(&msg);
                newwin->SetDlg(this);
                //Take the information here ...
                // newwin->Close(); QuitRequested kills this dialog..
        }                
}

