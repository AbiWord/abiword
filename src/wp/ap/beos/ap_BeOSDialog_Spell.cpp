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

/*****************************************************************/

class SpellWin:public BWindow {
	public:
		SpellWin(BMessage *data);
		void SetDlg(AP_BeOSDialog_Spell *dlg);
		virtual void DispatchMessage(BMessage *msg, BHandler *handler);
		virtual bool QuitRequested(void);
		
	private:
                int                     spin;
		AP_BeOSDialog_Spell 	*m_DlgSpell;
};

SpellWin::SpellWin(BMessage *data) 
	  :BWindow(data) {
	spin = 1;
} 

void SpellWin::SetDlg(AP_BeOSDialog_Spell *dlg) {
	m_DlgSpell = dlg;

	Show();
	while (spin) { snooze(1); }
	Hide();

#if 0
   UT_Bool bRes = nextMisspelledWord();
   while (bRes) {
 	// show word in main window
        makeWordVisible(); 

	// update dialog with new misspelled word info/suggestions
        //_showMisspelledWord();

        // run into the GTK event loop for this window
        //gtk_main();

        //_purgeSuggestions();

        if (m_bCancelled) break;

        // get the next unknown word
        bRes = nextMisspelledWord();                      
   }
#endif
}

void SpellWin::DispatchMessage(BMessage *msg, BHandler *handler) {
	switch(msg->what) {
	default:
		BWindow::DispatchMessage(msg, handler);
	}
} 

//Behave like a good citizen
bool SpellWin::QuitRequested() {
	spin = 0;
	return(true);
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

   m_bCancelled = UT_FALSE;

	BMessage msg;
        SpellWin  *newwin;
        if (RehydrateWindow("SpellWindow", &msg)) {
                newwin = new SpellWin(&msg);
                newwin->SetDlg(this);
                //Take the information here ...
                newwin->Close();
        }                
}

