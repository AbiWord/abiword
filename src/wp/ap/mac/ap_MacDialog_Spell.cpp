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
#include "xap_MacApp.h"
#include "xap_MacFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Spell.h"

#include "ap_MacDialog_Spell.h"


/*****************************************************************/

XAP_Dialog * AP_MacDialog_Spell::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
   AP_MacDialog_Spell * p = new AP_MacDialog_Spell(pFactory,id);
   return p;
}

AP_MacDialog_Spell::AP_MacDialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : AP_Dialog_Spell(pDlgFactory,id)
{
}

AP_MacDialog_Spell::~AP_MacDialog_Spell(void)
{
}

/************************************************************/
void AP_MacDialog_Spell::runModal(XAP_Frame * pFrame)
{
   // call the base class method to initialize some basic xp stuff
   AP_Dialog_Spell::runModal(pFrame);

#if 0
   m_bCancelled = UT_FALSE;

	BMessage msg;
        SpellWin  *newwin;
        if (RehydrateWindow("SpellWindow", &msg)) {
                newwin = new SpellWin(&msg);
                newwin->SetDlg(this);
                //Take the information here ...
                // newwin->Close(); QuitRequested kills this dialog..
        }   
#endif
}

