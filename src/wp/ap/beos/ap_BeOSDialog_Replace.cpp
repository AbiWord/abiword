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

#include "xap_Dialog_Id.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "ap_Dialog_Replace.h"
#include "ap_BeOSDialog_Replace.h"
#include "ap_Dialog_Id.h"

#include "ut_Rehydrate.h"

/* 
 TF Note:
  This class is totally screwed.  We halt the main thread while
we run the dialog ... of course all hell breaks loose as a result
because of the fact that we have halted the main thread. Sucks
to be so damn fast and multithreaded.

  This causes a crappy deadlock that I haven't figured out how 
to fix just yet.
*/

/*****************************************************************/
#define RAD_ON(rad, str) ((rad = (BRadioButton *)FindView(str)) && \
			  (rad->Value() == B_CONTROL_ON))

class FindWin:public BWindow {
	public:
		FindWin(BMessage *data);
		void SetDlg(AP_BeOSDialog_Replace *repl);
		virtual void DispatchMessage(BMessage *msg, BHandler *handler);
		virtual bool QuitRequested(void);
		
	private:
		int 			spin;
		AP_BeOSDialog_Replace 	*m_DlgReplace;
};

FindWin::FindWin(BMessage *data) 
	  :BWindow(data) {
	spin = 1;	
} //FindWin::FindWin

void FindWin::SetDlg(AP_BeOSDialog_Replace *repl) {
	m_DlgReplace = repl;
	
	BTextControl *txt;
	UT_UCSChar *bufferUnicode;
        char * bufferNormal;

	//What an ugly piece of code this is ..
	txt = (BTextControl *)FindView("txtFind");
	bufferUnicode = m_DlgReplace->getFindString();
        bufferNormal = (char *)calloc(UT_UCS_strlen(bufferUnicode) + 1, sizeof(char));
        UT_UCS_strcpy_to_char(bufferNormal, bufferUnicode);
	if (txt) txt->SetText(bufferNormal);
	if (bufferNormal) free(bufferNormal);

	txt = (BTextControl *)FindView("txtReplace");
	bufferUnicode = m_DlgReplace->getReplaceString();
        bufferNormal = (char *)calloc(UT_UCS_strlen(bufferUnicode) + 1, sizeof(char));
        UT_UCS_strcpy_to_char(bufferNormal, bufferUnicode);
	if (txt) txt->SetText(bufferNormal);
	if (bufferNormal) free(bufferNormal);

	BCheckBox *chk = (BCheckBox *)FindView("chkCase");
	if (chk) chk->SetValue(m_DlgReplace->getMatchCase());

//	For the find/replace dialog we can return right away
	Show();
	//while (spin) { snooze(1); }
	//Hide();
}

void FindWin::DispatchMessage(BMessage *msg, BHandler *handler) {
	BTextControl 	*findtxt, *repltxt;
     	UT_UCSChar 	*findString, *replaceString;
	int		case_sensitive, replace_all;	

	findtxt = repltxt = NULL;
	BCheckBox *chk = (BCheckBox *)FindView("chkCase");
	case_sensitive = (chk && chk->Value() == B_CONTROL_ON) ? 1 : 0;
	replace_all = 0;
	switch(msg->what) {
	case 'rall':
		replace_all = 1;
		//Fall through
	case 'repl':
		if (!(repltxt = (BTextControl *)FindView("txtReplace")))
			break;
		UT_DEBUGMSG(("FIND: replace %s \n", repltxt->Text()));
        	UT_UCS_cloneString_char(&replaceString, repltxt->Text());
		UT_ASSERT(m_DlgReplace);
        	m_DlgReplace->setReplaceString(replaceString);
		//Fall through
	case 'find':
		if (!(findtxt = (BTextControl *)FindView("txtFind")))
			break;
		UT_DEBUGMSG(("FIND: find %s \n", findtxt->Text()));
        	UT_UCS_cloneString_char(&findString, findtxt->Text());
		UT_ASSERT(m_DlgReplace);
        	m_DlgReplace->setFindString(findString);
		m_DlgReplace->setMatchCase(case_sensitive);

		spin = 0;		//Let the main thread free
		sleep(1);
		if (repltxt && replace_all)
        		m_DlgReplace->findReplaceAll();                       
		else if (repltxt)
        		m_DlgReplace->findReplace();                       
		else
			m_DlgReplace->findNext();         
		break;

	default:
		BWindow::DispatchMessage(msg, handler);
	}
} 

//Behave like a good citizen
bool FindWin::QuitRequested() {
	UT_ASSERT(m_DlgReplace);
	m_DlgReplace->setAnswer(AP_Dialog_Replace::a_CANCEL);

	spin = 0;
	return(true);
}

/*****************************************************************/

XAP_Dialog * AP_BeOSDialog_Replace::static_constructor(XAP_DialogFactory * pFactory,
													  XAP_Dialog_Id id)
{
	AP_BeOSDialog_Replace * p = new AP_BeOSDialog_Replace(pFactory,id);
	return p;
}

AP_BeOSDialog_Replace::AP_BeOSDialog_Replace(XAP_DialogFactory * pDlgFactory,
					   XAP_Dialog_Id id)
	: AP_Dialog_Replace(pDlgFactory,id)
{

	m_findString = NULL;
	m_replaceString = NULL;
    	m_matchCase = UT_TRUE;
}

AP_BeOSDialog_Replace::~AP_BeOSDialog_Replace(void)
{
}

void AP_BeOSDialog_Replace::runModal(XAP_Frame * pFrame)
{
	BMessage msg;
	FindWin  *newwin;

	// this dialogs needs this
        setView(static_cast<FV_View *> (pFrame->getCurrentView()));          

	if (m_id == AP_DIALOG_ID_FIND) {
		if (RehydrateWindow("FindWindow", &msg)) {
                	newwin = new FindWin(&msg);
			newwin->SetDlg(this);
		}
        }                                                
	else  {
		if (RehydrateWindow("ReplaceWindow", &msg)) {
                	newwin = new FindWin(&msg);
			newwin->SetDlg(this);
		}
	}
}


