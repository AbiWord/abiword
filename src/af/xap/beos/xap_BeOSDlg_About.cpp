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
extern unsigned char g_pngSidebar[];            // see ap_wp_sidebar.cpp
extern unsigned long g_pngSidebar_sizeof;       // see ap_wp_sidebar.cp

class AboutWin:public BWindow
{
public:
	AboutWin(BMessage *data, XAP_Frame * pFrame);
	virtual void DispatchMessage(BMessage *msg, BHandler *handler);
	void initDialogFields(void);

private:
	XAP_Frame *m_pFrame;
};                                                    

AboutWin::AboutWin(BMessage *data, XAP_Frame * pFrame)
	: BWindow(data)
{
	m_pFrame = pFrame;
}

void AboutWin::initDialogFields(void)
{
	XAP_App * pApp = m_pFrame->getApp();
	
	char buf[1000];

	BStringView *strAppName = (BStringView*)FindView("strAppName");
	if (strAppName) strAppName->SetText(pApp->getApplicationName());
	
	BStringView *strVersion = (BStringView*)FindView("strVersion");
	sprintf(buf, XAP_ABOUT_VERSION, XAP_App::s_szBuild_Version);
	if (strVersion) strVersion->SetText(buf);

	BStringView * strCopyright = (BStringView*)FindView("strCopyright");
	if (strCopyright) strCopyright->SetText(XAP_ABOUT_COPYRIGHT);

	BTextView * strBodyText = (BTextView*)FindView("strBodyText");
	sprintf(buf,XAP_ABOUT_GPL_LONG_LF,pApp->getApplicationName());
	if (strBodyText) strBodyText->SetText(buf);

//	str = (BStringView*)FindView("strOptions");
//	sprintf(buf, XAP_ABOUT_BUILD, XAP_App::s_szBuild_Options);
//	if (str) str->SetText(buf);

	BView *view = FindView("sideView");
	if (view) {
		BMemoryIO mio(g_pngSidebar, g_pngSidebar_sizeof);
		BBitmap *bitmap =  BTranslationUtils::GetBitmap(&mio);
		if (bitmap)
			view->SetViewBitmap(bitmap, B_FOLLOW_ALL, 0);
	}
} 

void AboutWin::DispatchMessage(BMessage *msg, BHandler *handler)
{
	switch(msg->what)
	{
	case 'gurl':
		m_pFrame->openURL("http://www.abisource.com");
		break;
	default:
		BWindow::DispatchMessage(msg, handler);
		break;
	}                           
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
	if (RehydrateWindow("AboutWindow", msg))
	{
		AboutWin *nwin = new AboutWin(msg, pFrame);
		if (nwin)
		{
			nwin->initDialogFields();
			nwin->Show();
		}
	}	
}

