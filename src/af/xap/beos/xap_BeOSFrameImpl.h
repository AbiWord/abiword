/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Daniel Furrer 
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


#ifndef XAP_BEOSFRAMEIMPL_H
#define XAP_BEOSFRAMEIMPL_H

#include <InterfaceKit.h>

#include "xap_BeOSApp.h"
#include "xap_FrameImpl.h"
#include "ut_vector.h"
#include "xap_BeOSDialogFactory.h"

/*****************************************************************
******************************************************************
** This file defines the beos-platform-specific class for the
** cross-platform application frame.  This is used to hold all
** BeOS-specific data.  One of these is created for each top-level
** document window.
******************************************************************
*****************************************************************/

class XAP_BeOSFrameImpl : public XAP_FrameImpl
{
public:
	XAP_BeOSFrameImpl(XAP_Frame *pFrame, XAP_BeOSApp * app);
	friend class XAP_Frame;
	virtual ~XAP_BeOSFrameImpl(void);

	BWindow * getTopLevelWindow() const;

protected:
	virtual bool _close();
	virtual bool _raise();
	virtual bool _show();

	virtual void _nullUpdate () const; // a virtual member function in xap_Frame
	virtual void _initialize();

	virtual void _setCursor(GR_Graphics::Cursor cursor);

	virtual XAP_DialogFactory * _getDialogFactory();
	virtual EV_Menu * _getMainMenu();
	virtual EV_Toolbar * _newToolbar(XAP_App *pApp, XAP_Frame *pFrame,
				 const char *szLayout,
				 const char *szLanguage);

	virtual bool _runModalContextMenu(AV_View * pView, const char * szMenuName,
					  UT_sint32 x, UT_sint32 y);

	virtual void _queue_resize();
	void _rebuildToolbar(UT_uint32 ibar);

	virtual void _setFullScreen(bool changeToFullScreen);

	//Main window and document view 
	BWindow * m_pBeWin;			

	
private:


	AP_BeOSDialogFactory	m_dialogFactory;
};

#endif /* XAP_BEOSFRAMEIMPL_H */
