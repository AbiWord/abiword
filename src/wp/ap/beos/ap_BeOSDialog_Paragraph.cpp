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
#include "ut_types.h"

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "ap_Dialog_Id.h"

#include "ap_Strings.h"

#include "ap_Preview_Paragraph.h"
#include "ap_BeOSDialog_Paragraph.h"

/*****************************************************************/

XAP_Dialog * AP_BeOSDialog_Paragraph::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	AP_BeOSDialog_Paragraph * p = new AP_BeOSDialog_Paragraph(pFactory,id);
	return p;
}

AP_BeOSDialog_Paragraph::AP_BeOSDialog_Paragraph(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_Paragraph(pDlgFactory,id)
{
}

AP_BeOSDialog_Paragraph::~AP_BeOSDialog_Paragraph(void)
{
//	DELETEP(m_pBeOSGraphics);
}

/*****************************************************************/

void AP_BeOSDialog_Paragraph::runModal(XAP_Frame * pFrame)
{

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

	
	// TODO build the dialog, attach events, etc., etc.
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void AP_BeOSDialog_Paragraph::_syncControls(tControl changed, UT_Bool bAll /* = UT_FALSE */)
{ 
	// let parent sync any member variables first

	AP_Dialog_Paragraph::_syncControls(changed, bAll);

	// sync the display

	// TODO: see the latest version of the Win32 implementation for ideas
}
