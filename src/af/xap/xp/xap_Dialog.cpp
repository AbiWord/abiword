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

#include <string.h>
#include "xap_Dialog.h"
#include "ut_assert.h"
#include "xap_DialogFactory.h"
#include "xap_App.h"
#include "xap_Frame.h"

/*****************************************************************/

XAP_Dialog::XAP_Dialog(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
{
	UT_ASSERT(pDlgFactory);

	m_pDlgFactory = pDlgFactory;
	m_id = id;
	m_pApp = pDlgFactory->getApp();

	UT_ASSERT(m_pApp);
}

XAP_Dialog::~XAP_Dialog(void)
{
}

XAP_Dialog_Id XAP_Dialog::getDialogId(void) const
{
	return m_id;
}


/*****************************************************************/

XAP_Dialog_NonPersistent::XAP_Dialog_NonPersistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog(pDlgFactory,id)
{
}

XAP_Dialog_NonPersistent::~XAP_Dialog_NonPersistent(void)
{
}

/*****************************************************************/

XAP_Dialog_Persistent::XAP_Dialog_Persistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog(pDlgFactory,id)
{
	m_bInUse = UT_FALSE;
}

XAP_Dialog_Persistent::~XAP_Dialog_Persistent(void)
{
}

void XAP_Dialog_Persistent::useStart(void)
{
	UT_ASSERT(!m_bInUse);
	m_bInUse = UT_TRUE;
}

void XAP_Dialog_Persistent::useEnd(void)
{
	UT_ASSERT(m_bInUse);
	m_bInUse = UT_FALSE;
}

/*****************************************************************/

XAP_Dialog_FramePersistent::XAP_Dialog_FramePersistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Persistent(pDlgFactory,id)
{
}

XAP_Dialog_FramePersistent::~XAP_Dialog_FramePersistent(void)
{
}

void XAP_Dialog_FramePersistent::useStart(void)
{
	XAP_Dialog_Persistent::useStart();
}

void XAP_Dialog_FramePersistent::useEnd(void)
{
	XAP_Dialog_Persistent::useEnd();
}

/*****************************************************************/

XAP_Dialog_AppPersistent::XAP_Dialog_AppPersistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Persistent(pDlgFactory,id)
{
}

XAP_Dialog_AppPersistent::~XAP_Dialog_AppPersistent(void)
{
}

void XAP_Dialog_AppPersistent::useStart(void)
{
	XAP_Dialog_Persistent::useStart();
}

void XAP_Dialog_AppPersistent::useEnd(void)
{
	XAP_Dialog_Persistent::useEnd();
}



XAP_Dialog_Modeless::XAP_Dialog_Modeless(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_AppPersistent(pDlgFactory,id)
{
	UT_ASSERT(pDlgFactory);

	m_pDlgFactory = pDlgFactory;
	m_id = id;
	m_pApp = pDlgFactory->getApp();
        m_pDialog = (XAP_Dialog_Modeless *) this;

	UT_ASSERT(m_pApp);
}

XAP_Dialog_Modeless::~XAP_Dialog_Modeless(void)
{
}

void XAP_Dialog_Modeless::useStart(void)
{
	UT_ASSERT(!m_bInUse);
	m_bInUse = UT_TRUE;
}

void XAP_Dialog_Modeless::useEnd(void)
{
	UT_ASSERT(m_bInUse);
	m_bInUse = UT_FALSE;
}

void XAP_Dialog_Modeless::modeless_cleanup(void)
{
	UT_sint32 sid = (UT_sint32) getDialogId();
	m_pApp->forgetModelessId( (UT_sint32) sid);
	m_pDlgFactory->releaseDialog(m_pDialog);
}

UT_Bool XAP_Dialog_Modeless::isRunning(void)
{
 
	UT_sint32 sid = (UT_sint32) getDialogId();

	/*
	void * pWidget = m_pApp->getModelessWidget(sid);
	UT_Bool isrunning = UT_TRUE;
	if(pWidget == (void *) NULL)
	{
		isrunning = UT_FALSE;
	}
	*/
	return m_pApp->isModelessRunning(sid);
}


XAP_Frame *   XAP_Dialog_Modeless::getActiveFrame(void)
{
	// This function returns the frame currently connected to a modeless dialog

	XAP_Frame * pFrame = m_pApp->getLastFocussedFrame();
	if(pFrame == (XAP_Frame *) NULL)
	{
		pFrame = m_pApp->getFrame(0);
	}
	return pFrame;
}


void XAP_Dialog_Modeless::setActiveFrame(XAP_Frame *pFrame)
{
	notifyActiveFrame(pFrame);
}


char *  XAP_Dialog_Modeless::BuildWindowName( char * pWindowName, char* pDialogName, UT_sint32 width)
{
  // This function returns contructs the window name of a modeless dialog by
  // concatonating the active frame with the dialog name

        UT_sint32 count = (UT_sint32) strlen(pDialogName);
        const char* pFrameTitle =  getActiveFrame()->getTitle(width-count-3);
        *pWindowName = (char) NULL;
        pWindowName = strncat(pWindowName,pDialogName,count);
        pWindowName = strncat(pWindowName," - ",3);
        pWindowName = strncat(pWindowName,pFrameTitle,width-count-3); 
	return pWindowName;
}


