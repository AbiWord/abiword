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

#include <stdio.h>
#include <string.h>
#include <Roster.h>
#include "ut_types.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"
#include "xap_BeOSFrameImpl.h"
#include "ev_BeOSKeyboard.h"
#include "ev_BeOSMouse.h"
#include "ev_BeOSMenu.h"
#include "ev_BeOSToolbar.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "xad_Document.h"


#include "ap_FrameData.h"
#include "gr_BeOSGraphics.h"


XAP_BeOSFrameImpl::XAP_BeOSFrameImpl(XAP_Frame *pFrame, XAP_BeOSApp * app)
	: XAP_FrameImpl(pFrame),
	  m_pBeWin(NULL),
	  m_pBeOSApp(app),
	  m_pBeOSMenu(NULL),
	  m_pBeOSMenuPopup(NULL),
	  m_dialogFactory(pFrame, static_cast<XAP_App *>(app))
{
;	
}

XAP_BeOSFrameImpl::~XAP_BeOSFrameImpl(void)
{
}

bool XAP_BeOSFrameImpl::_close()
{
	m_pBeWin->Lock();
	m_pBeWin->Close();
	m_pBeWin->Unlock();
	return true;
}

XAP_DialogFactory * XAP_BeOSFrameImpl::_getDialogFactory(void)
{
	return &m_dialogFactory;
}

EV_Menu * XAP_BeOSFrameImpl::_getMainMenu(void)
{
	return m_pBeOSMenu;
}

void XAP_BeOSFrameImpl::_initialize()
{
	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.
	EV_EditEventMapper * pEEM = XAP_App::getApp()->getEditEventMapper();
	UT_ASSERT(pEEM);

	//These are actually "attached" in the ap_Frame code
	//since they require that all the beos classes be
	//instantiated.
	m_pKeyboard = new ev_BeOSKeyboard(pEEM);
	UT_ASSERT(m_pKeyboard);

	m_pMouse = new ev_BeOSMouse(pEEM);
	UT_ASSERT(m_pMouse);
}

EV_Toolbar *XAP_BeOSFrameImpl::_newToolbar(XAP_App *pApp, XAP_Frame *pFrame,
	const char *szLayout,
	const char *szLanguage)
{
	return (new EV_BeOSToolbar((XAP_BeOSApp *)(pApp), 
							   static_cast<XAP_BeOSFrame *>(pFrame), 
							   szLayout, szLanguage));
}

void XAP_BeOSFrameImpl::_nullUpdate() const
{
}

void XAP_BeOSFrameImpl::_queue_resize()
{
}

bool XAP_BeOSFrameImpl::_raise()
{
	m_pBeWin->Lock();
	m_pBeWin->Show();
	m_pBeWin->Unlock();
	return true;
}

void XAP_BeOSFrameImpl::_rebuildToolbar(UT_uint32 ibar)
{
	XAP_Frame*	pFrame = getFrame();
	
	pFrame->refillToolbarsInFrameData();
	pFrame->repopulateCombos();
}

bool XAP_BeOSFrameImpl::_runModalContextMenu(AV_View * /* pView */, const char * szMenuName,
										   UT_sint32 x, UT_sint32 y)
{
	bool bResult = true;
	UT_ASSERT(!m_pBeOSMenuPopup);

	m_pBeOSMenuPopup = new EV_BeOSMenuPopup(m_pBeOSApp, static_cast<XAP_BeOSFrame *>(getFrame()), 
									  szMenuName, 
									  m_szMenuLabelSetName);
									  
	if (m_pBeOSMenuPopup && m_pBeOSMenuPopup->synthesizeMenuPopup(static_cast<XAP_BeOSFrame *>(getFrame())))
	{
		UT_DEBUGMSG(("ContextMenu: %s at [%d,%d]\n",szMenuName,x,y));
		
		// BROKEN:
		//translateDocumentToScreen(x,y);
	 
		BPoint screenPoint(x,y);
		BPopUpMenu* pMenu = m_pBeOSMenuPopup->GetHandle();
		BMenuItem* selectedItem = pMenu->Go(screenPoint , false , false, false);
	
		// Send the menu item invocation message to the window
		
		if(selectedItem) // The user clicked an item, will be NULL if they click away from the popup.
		{
			if(m_pBeWin) // Should always be valid here, but just in case.
				m_pBeWin->PostMessage(selectedItem->Message());	
		}
		
	}
	
	DELETEP(m_pBeOSMenuPopup);

	return bResult;

}

void XAP_BeOSFrameImpl::_setCursor(GR_Graphics::Cursor c)
{
}

void XAP_BeOSFrameImpl::_setFullScreen(bool changeToFullScreen)
{
}

bool XAP_BeOSFrameImpl::_show()
{
	_raise();
	return true;
}

BWindow * XAP_BeOSFrameImpl::getTopLevelWindow(void) const
{
	return(m_pBeWin);
}



