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

#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_Zoom.h"
#include "xap_MacDlg_Zoom.h"

/*****************************************************************/

XAP_Dialog * XAP_MacDialog_Zoom::static_constructor(XAP_DialogFactory * pFactory,
													XAP_Dialog_Id id)
{
	XAP_MacDialog_Zoom * p = new XAP_MacDialog_Zoom(pFactory,id);
	return p;
}

XAP_MacDialog_Zoom::XAP_MacDialog_Zoom(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_Zoom(pDlgFactory,id)
{

}

XAP_MacDialog_Zoom::~XAP_MacDialog_Zoom(void)
{
}

/*****************************************************************/

void XAP_MacDialog_Zoom::runModal(XAP_Frame * pFrame)
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

#if 0
	// TODO build the dialog, attach events, etc., etc.
	BMessage msg;
	ZoomWin  *newwin;
	if (RehydrateWindow("ZoomWindow", &msg)) {
                newwin = new ZoomWin(&msg);
		newwin->SetDlg(this);
		//Take the information here ...
		if (newwin->Lock())
		{
			newwin->GetAnswer(m_zoomType,m_zoomPercent);
			if (m_zoomPercent != 0)
			{
				 UT_DEBUGMSG(("Okaying, m_zoomType=%d, m_zoomPercent=%d\n",m_zoomType,m_zoomPercent));
			 	m_answer=XAP_Dialog_Zoom::a_OK;
			}
			else
			{
				UT_DEBUGMSG(("Cancelling, m_zoomType=%d, m_zoomPercent=%d\n",m_zoomType,m_zoomPercent));
				m_answer=XAP_Dialog_Zoom::a_CANCEL;
			}
		}

		newwin->Close();
		
		
        }   
#endif
}

/*****************************************************************/
