/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2004 Daniel Furrer
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


#include <InterfaceKit.h>

#include "ap_BeOSFrameImpl.h"
#include "ap_BeOSApp.h"
#include "ev_BeOSToolbar.h"
#include "ap_FrameData.h"
#include "ap_BeOSTopRuler.h"
#include "ap_BeOSLeftRuler.h"
#include "xap_BeOSApp.h"
#include "ap_BeOSStatusBar.h"
#include "gr_BeOSGraphics.h"
#include "ut_debugmsg.h"



AP_BeOSFrameImpl::AP_BeOSFrameImpl(AP_BeOSFrame * pBeOSFrame, XAP_BeOSApp * pBeOSApp) :
	XAP_BeOSFrameImpl(static_cast<XAP_Frame *>(pBeOSFrame), static_cast<AP_App *>(pBeOSApp))
{
}

XAP_FrameImpl * AP_BeOSFrameImpl::createInstance(XAP_Frame *pFrame, XAP_App *pApp)
{
	XAP_FrameImpl *pFrameImpl = new AP_BeOSFrameImpl(static_cast<AP_BeOSFrame *>(pFrame),static_cast<XAP_BeOSApp *>(pApp));
	return pFrameImpl;
}

void AP_BeOSFrameImpl::_bindToolbars(AV_View * pView)
{
	int nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (int k = 0; k < nrToolbars; k++)
	{
		// TODO Toolbars are a frame-level item, but a view-listener is
		// TODO a view-level item.  I've bound the toolbar-view-listeners
		// TODO to the current view within this frame and have code in the
		// TODO toolbar to allow the view-listener to be rebound to a different
		// TODO view.  in the future, when we have support for multiple views
		// TODO in the frame (think splitter windows), we will need to have
		// TODO a loop like this to help change the focus when the current
		// TODO view changes.		
		EV_BeOSToolbar * pBeOSToolbar = reinterpret_cast<EV_BeOSToolbar *>(m_vecToolbars.getNthItem(k));
		pBeOSToolbar->bindListenerToView(pView);
	}	
}

UT_RGBColor AP_BeOSFrameImpl::getColorSelBackground () const
{
	return UT_RGBColor(125,125,125);
}

void AP_BeOSFrameImpl::_refillToolbarsInFrameData()
{
	UT_uint32 cnt = m_vecToolbarLayoutNames.getItemCount();

	for (UT_uint32 i = 0; i < cnt; i++)
	{
		EV_BeOSToolbar * pBeOSToolbar = static_cast<EV_BeOSToolbar *> (m_vecToolbars.getNthItem(i));
		static_cast<AP_FrameData*>(getFrame()->getFrameData())->m_pToolbar[i] = pBeOSToolbar;
	}
}

void AP_BeOSFrameImpl::_showOrHideStatusbar()
{
	XAP_Frame *pFrame = getFrame();
	AP_FrameData *pData = static_cast<AP_FrameData *>(pFrame->getFrameData());
	bool bShowStatusBar = pData->m_bShowStatusBar;
 	static_cast<AP_BeOSFrame *>(pFrame)->toggleStatusBar(bShowStatusBar);
}

BWindow * AP_BeOSFrameImpl::_createDocumentWindow()
{

	return(NULL);
}

void AP_BeOSFrameImpl::_setWindowIcon()
{
//No window Icons in BeOS
}

void AP_BeOSFrameImpl::_createWindow()
{

}

BView * AP_BeOSFrameImpl::_createStatusBarWindow()
{
/*
	AP_BeOSStatusBar *pStatusBar = new AP_BeOSStatusBar(m_pBeOSFrame);
	BView *pStatusBarView;
	UT_ASSERT(pStatusBar);
	static_cast<AP_FrameData*>(m_pBeOSFrame->m_pData)->m_pStatusBar = pStatusBar;
	BRect r;
	r = Bounds();
	r.top = r.bottom - STATUS_BAR_HEIGHT;
	pStatusBarView = pStatusBar->createWidget(r);
	AddChild(pStatusBarView);
	
	return pStatusBarView;
*/
}

//void AP_BeOSFrameImpl::_setScrollRange(apufi_ScrollType scrollType,int iValue,float fUpperLimit,float fSize)
//{
//}

void AP_BeOSFrameImpl::setDocumentFocus() {

}

void AP_BeOSFrameImpl::_reflowLayout(int loweradj, int upperadj, int topruleradj, int leftruleradj) {
}
