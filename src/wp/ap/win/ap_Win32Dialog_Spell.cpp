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


#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "ut_dialogHelper.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Spell.h"
#include "ap_Win32Dialog_Spell.h"


XAP_Dialog * AP_Win32Dialog_Spell::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
   AP_Win32Dialog_Spell * p = new AP_Win32Dialog_Spell(pFactory,id);
   return p;
}

AP_Win32Dialog_Spell::AP_Win32Dialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : AP_Dialog_Spell(pDlgFactory,id)
{
}

AP_Win32Dialog_Spell::~AP_Win32Dialog_Spell(void)
{
}

/************************************************************/
void AP_Win32Dialog_Spell::runModal(XAP_Frame * pFrame)
{
   // call the base class method to initialize some basic xp stuff
   AP_Dialog_Spell::runModal(pFrame);

   // TODO build the dialog, attach events, etc., etc.
   UT_ASSERT(UT_NOT_IMPLEMENTED);
}

