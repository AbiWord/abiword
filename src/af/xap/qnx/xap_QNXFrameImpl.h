/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Johan Björk 
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


#ifndef XAP_QNXFRAMEIMPL_H
#define XAP_QNXFRAMEIMPL_H

#include <Pt.h>
#include "xap_FrameImpl.h"
#include "ut_vector.h"
#include "xap_QNXDialogFactory.h"
#include "xap_QNXApp.h"
#include "ev_QNXMenu.h"

/********************************************************************
*********************************************************************
** This file defines the qnx-platform-specific class for the
** cross-platform application frame helper.  This is used to hold all
** qnx-specific data.  One of these is created for each top-level
** document window.
*********************************************************************
********************************************************************/

class XAP_QNXFrameImpl : public XAP_FrameImpl
{
 public:
	XAP_QNXFrameImpl(XAP_Frame *pFrame, XAP_QNXApp *pQNXApp);
	friend class XAP_Frame;
	virtual ~XAP_QNXFrameImpl();

	PtWidget_t * getTopLevelWindow() const;
	void setTopLevelWindow(PtWidget_t * window) { m_wTopLevelWindow = window; }
	void createTopLevelWindow(void);
	PtWidget_t * getTBGroupWidget(void) const;
	int					 getPopupDone() { return m_PopupDone; };
	void         setPopupDone(int value) { m_PopupDone = value; };
	GR_Graphics	*getGraphics();

	PhArea_t            m_AvailableArea;
private:
	void _setGeometry ();
	int m_PopupDone;

protected:
	virtual bool _close();
	virtual bool _raise();
	virtual bool _show();

	virtual void _nullUpdate () const; // a virtual member function in xap_Frame
	virtual void _initialize();
	
	virtual void _setWindowIcon() = 0; // should eventually be handled be the inherited helper

	virtual PtWidget_t * _createDocumentWindow() = 0;
	virtual PtWidget_t * _createStatusBarWindow() = 0;

	bool _updateTitle();
	UT_sint32 _setInputMode(const char * szName);
	virtual void _setCursor(GR_Graphics::Cursor cursor);
	
	virtual XAP_DialogFactory * _getDialogFactory();
	virtual EV_Menu * _getMainMenu();
	virtual EV_Toolbar * _newToolbar(XAP_App *pApp, XAP_Frame *pFrame,
				 const char *szLayout,
				 const char *szLanguage);

	virtual bool _runModalContextMenu(AV_View * pView, const char * szMenuName,
					  UT_sint32 x, UT_sint32 y);
	void setTimeOfLastEvent(int eventTime);
	
	virtual void _queue_resize();
	void _rebuildMenus(void);
	void _rebuildToolbar(UT_uint32 ibar);
	PtWidget_t * _getSunkenBox(void) {return m_wSunkenBox;}
		static int resize(PtWidget_t *w, void *data, PtCallbackInfo_t *info) ;

	virtual void _setFullScreen(bool changeToFullScreen);
	class _fe
        {
	friend class XAP_Frame;
	public:
		static int resize(PtWidget_t *w, void *data, PtCallbackInfo_t *info) ;
		static int focus_in_event(PtWidget_t *w, void *data, PtCallbackInfo_t *info) ;
		static int focus_out_event(PtWidget_t *w, void *data, PtCallbackInfo_t *info) ;
		static int button_press_event(PtWidget_t *w, void *data, PtCallbackInfo_t *info) ;
		static int button_release_event(PtWidget_t *w, void *data, PtCallbackInfo_t *info) ;
		static int motion_notify_event(PtWidget_t *w, void *data, PtCallbackInfo_t *info) ;
		static int key_press_event(PtWidget_t *w, void *data, PtCallbackInfo_t *info) ;
		static int window_resize(PtWidget_t *w, void *data, PtCallbackInfo_t *info) ;
		static int window_delete(PtWidget_t *w, void *data, PtCallbackInfo_t *info) ;
		static int expose(PtWidget_t *w,PhTile_t *damage);
		static int vScrollChanged(PtWidget_t *w, void *data, PtCallbackInfo_t *info) ;
		static int hScrollChanged(PtWidget_t *w, void *data, PtCallbackInfo_t *info) ;
		static int dnd(PtWidget_t *w, void *data, PtCallbackInfo_t *info) ;
		static int do_ZoomUpdate(void *p);
	};
	friend class _fe;

 private:
	bool                        m_bDoZoomUpdate;
	PtWorkProcId_t	*m_pZoomUpdateID;
	UT_sint32                   m_iNewWidth;
	UT_sint32                   m_iNewHeight;
	int                       m_iAbiRepaintID;

	PtWidget_t *		    m_wTopLevelWindow;
	PtWidget_t * 		    m_wSunkenBox;
	PtWidget_t *		    m_wStatusBar;
	PtWidget_t *				m_wTBGroup;

	XAP_QNXApp *				m_pQNXApp;
	EV_QNXMenuBar *			m_pQNXMenu;
	EV_QNXMenuPopup *			m_pQNXPopup; /* only valid while a context popup is up */
	AP_QNXDialogFactory		        m_dialogFactory;
};
#endif /* XAP_QNXFRAME_H */





