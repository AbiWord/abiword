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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Break.h"
#include "ap_MacDlg_Break.h" //?SBK

/*****************************************************************/

XAP_Dialog * AP_MacDialog_Break::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_MacDialog_Break * p = new AP_MacDialog_Break(pFactory,id);
	return p;
}

AP_MacDialog_Break::AP_MacDialog_Break(XAP_DialogFactory * pDlgFactory,
										   XAP_Dialog_Id id)
	: AP_Dialog_Break(pDlgFactory,id)
{
}

AP_MacDialog_Break::~AP_MacDialog_Break(void)
{
}

/*****************************************************************/

void AP_MacDialog_Break::runModal(XAP_Frame * pFrame)
{

	/*
	  This dialog is non-persistent.
	  
	  This dialog should do the following:

	  - Construct itself to represent the base-class breakTypes
	    b_PAGE, b_COLUMN, b_NEXTPAGE, b_CONTINUOUS, b_EVENPAGE, b_ODDPAGE.
		The Unix one looks just like Microsoft Word 97, with the preview
		and all (even though it's not hooked up yet).

	  - Set break type to match "m_break"

	  On "OK" (or during user-interaction) the dialog should:

	  - Save the break type to "m_break".
	  
	  On "Cancel" the dialog should:

	  - Just quit, the data items will be ignored by the caller.

	*/

	// TODO build the dialog, attach events, etc., etc.
	m_answer = AP_Dialog_Break::a_CANCEL;
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

/*****************************************************************/
