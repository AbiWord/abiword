/* AbiSource Application Framework
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
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_Break.h"
#include "xap_BeOSDlg_Break.h"

/*****************************************************************/

AP_Dialog * XAP_BeOSDialog_Break::static_constructor(AP_DialogFactory * pFactory,
													 AP_Dialog_Id id)
{
	XAP_BeOSDialog_Break * p = new XAP_BeOSDialog_Break(pFactory,id);
	return p;
}

XAP_BeOSDialog_Break::XAP_BeOSDialog_Break(AP_DialogFactory * pDlgFactory,
										   AP_Dialog_Id id)
	: XAP_Dialog_Break(pDlgFactory,id)
{
}

XAP_BeOSDialog_Break::~XAP_BeOSDialog_Break(void)
{
}

/*****************************************************************/

void XAP_BeOSDialog_Break::runModal(XAP_Frame * pFrame)
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
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

/*****************************************************************/
