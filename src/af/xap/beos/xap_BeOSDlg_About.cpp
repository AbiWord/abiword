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
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
// #include "ut_dialogHelper.h"
#include "ut_Rehydrate.h"
#include <Be.h>

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_About.h"
#include "xap_BeOSDlg_About.h"

#define DEFAULT_BUTTON_WIDTH 85

/*****************************************************************/
class AboutWin:public BWindow {
        public:
                AboutWin(BMessage *data);
        private:
};                                                    

AboutWin::AboutWin(BMessage *data)
        :BWindow(data) {
	char buf[100];

	BStringView *str = (BStringView*)FindView("strVersion");
	sprintf(buf, XAP_ABOUT_VERSION, XAP_App::s_szBuild_Version);
	if (str) str->SetText(buf);
		
	str = (BStringView*)FindView("strOptions");
	sprintf(buf, XAP_ABOUT_BUILD, XAP_App::s_szBuild_Options);
	if (str) str->SetText(buf);
} 

/*****************************************************************/

XAP_Dialog * XAP_BeOSDialog_About::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_BeOSDialog_About * p = new XAP_BeOSDialog_About(pFactory,id);
	return p;
}

XAP_BeOSDialog_About::XAP_BeOSDialog_About(XAP_DialogFactory * pDlgFactory,
											 XAP_Dialog_Id id)
	: XAP_Dialog_About(pDlgFactory,id)
{
}

XAP_BeOSDialog_About::~XAP_BeOSDialog_About(void)
{
}

void XAP_BeOSDialog_About::runModal(XAP_Frame * pFrame)
{
	//XAP_App* pApp = pFrame->getApp();
	//char buf[2048];
	//sprintf(buf, XAP_ABOUT_TITLE, pApp->getApplicationName());
	//sprintf(buf, XAP_ABOUT_DESCRIPTION, pApp->getApplicationName());
	BMessage *msg = new BMessage();
	if (RehydrateWindow("AboutWindow", msg)) {
		AboutWin *nwin = new AboutWin(msg);
		if (nwin)
			nwin->Show();
	}	
}

