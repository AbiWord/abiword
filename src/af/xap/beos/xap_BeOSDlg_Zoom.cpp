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
#include "xap_Dlg_Zoom.h"
#include "xap_BeOSDlg_Zoom.h"

/*****************************************************************/

XAP_Dialog * XAP_BeOSDialog_Zoom::static_constructor(XAP_DialogFactory * pFactory,
													XAP_Dialog_Id id)
{
	XAP_BeOSDialog_Zoom * p = new XAP_BeOSDialog_Zoom(pFactory,id);
	return p;
}

XAP_BeOSDialog_Zoom::XAP_BeOSDialog_Zoom(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_Zoom(pDlgFactory,id)
{

}

XAP_BeOSDialog_Zoom::~XAP_BeOSDialog_Zoom(void)
{
}

/*****************************************************************/

void XAP_BeOSDialog_Zoom::runModal(XAP_Frame * pFrame)
{

	/*
	  This dialog is non-persistent.
	  
	  This dialog should do the following:

	  - Construct itself to represent the base-class zoomTypes
	    z_200, z_100, z_75, z_PageWidth, z_WholePage, and z_Percent.
		The Unix one looks just like Microsoft Word 97, with the preview
		and all (even though it's not hooked up yet).

	  - Set zoom type to match "m_zoomType" and value of radio button
	    to match "m_zoomPercent".

	  On "OK" (or during user-interaction) the dialog should:

	  - Save the zoom type to "m_zoomType".
	  
	  - Save the value in the Percent spin button box to "m_zoomPercent".

	  On "Cancel" the dialog should:

	  - Just quit, the data items will be ignored by the caller.

	*/

	// TODO build the dialog, attach events, etc., etc.
	m_answer = XAP_Dialog_Zoom::a_CANCEL;
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

/*****************************************************************/
