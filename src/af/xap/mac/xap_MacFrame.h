/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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


#ifndef XAP_MACFRAME_H
#define XAP_MACFRAME_H

#include <MacWindows.h>
#include <MacTypes.h>
#include <Controls.h>

#include "xap_Frame.h"
#include "ut_vector.h"
#include "xap_MacDialogFactory.h"
class XAP_MacApp;
class ev_MacKeyboard;
class EV_MacMouse;
class EV_MacMenu;

/*****************************************************************
******************************************************************
** This file defines the Mac-platform-specific class for the
** cross-platform application frame.  This is used to hold all
** Mac-specific data.  One of these is created for each top-level
** document window.
******************************************************************
*****************************************************************/

#define XAP_MACFRAME_WINDOW_KIND 128

class XAP_MacFrame : public XAP_Frame
{
public:
	XAP_MacFrame(XAP_MacApp * app);
	XAP_MacFrame(XAP_MacFrame * f);
	virtual ~XAP_MacFrame(void);

	virtual bool initialize(const char * szKeyBindingsKey, 
				  const char * szKeyBindingsDefaultValue,
				  const char * szMenuLayoutKey, 
				  const char * szMenuLayoutDefaultValue,
				  const char * szMenuLabelSetKey, 
				  const char * szMenuLabelSetDefaultValue,
				  const char * szToolbarLayoutsKey, 
				  const char * szToolbarLayoutsDefaultValue,
				  const char * szToolbarLabelSetKey, 
				  const char * szToolbarLabelSetDefaultValue);

	virtual	XAP_Frame *			cloneFrame(void);
	virtual UT_Error			loadDocument(const char * szFilename,  int ieft)=0;
	virtual bool				close(void);
	virtual bool				raise(void);
	virtual bool				show(void);
	virtual bool				openURL(const char * szURL);
	virtual bool				updateTitle(void);
       	virtual UT_sint32			setInputMode(const char * szName);


	virtual XAP_DialogFactory *	getDialogFactory(void);
	virtual void				setXScrollRange(void);
	virtual void				setYScrollRange(void);
	virtual bool 			runModalContextMenu(AV_View * pView, const char * szMenuName, UT_sint32 x, UT_sint32 y);
	
	virtual void				setStatusMessage(const char * szMsg) { UT_ASSERT (UT_NOT_IMPLEMENTED); };

	virtual void				toggleRuler(bool bRulerOn) { UT_ASSERT (UT_NOT_IMPLEMENTED); };
	virtual void				toggleBar(UT_uint32 iBarNb, bool bBarOn) { UT_ASSERT (UT_NOT_IMPLEMENTED); };
	virtual void				toggleStatusBar(bool bStatusBarOn) { UT_ASSERT (UT_NOT_IMPLEMENTED); };
	virtual bool				getBarVisibility(UT_uint32 iBarNb) { UT_ASSERT (UT_NOT_IMPLEMENTED); };


	virtual void				queue_resize() { UT_ASSERT (UT_NOT_IMPLEMENTED); };

	EV_MacMenu					*getMenu () { return m_pMacMenu; };

	WindowPtr _getMacWindow (void) { UT_ASSERT (m_MacWindow != NULL); return m_MacWindow; } ;
protected:
	virtual EV_Toolbar *		_newToolbar(XAP_App *app, XAP_Frame *frame, const char *, const char *) 
										{ UT_ASSERT (UT_NOT_IMPLEMENTED); };

	void						_createTopLevelWindow(void);
	void						_createToolbars(void);
	void						_createDocumentWindow(void);
	WindowPtr					m_MacWindow;
    GrafPtr						m_MacWindowPort;
	Rect 						m_winBounds;
    AP_MacDialogFactory			m_dialogFactory;
        
    ev_MacKeyboard				*m_pKeyboard;
    EV_MacMenu					*m_pMacMenu;
    EV_MacMouse					*m_pMouse;
private:
	void						MacWindowInit ();
	void						_calcVertScrollBarRect(Rect & rect);
	void						_calcHorizScrollBarRect(Rect & rect);

        ControlHandle					m_HScrollBar;
        ControlHandle					m_VScrollBar;
};

#endif /* XAP_MACFRAME_H */
