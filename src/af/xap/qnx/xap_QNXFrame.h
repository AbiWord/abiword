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


#ifndef XAP_QNXFRAME_H
#define XAP_QNXFRAME_H

#include "xap_Frame.h"
#include "ut_vector.h"
#include "xap_QNXDialogFactory.h"
#include <Pt.h>

class XAP_QNXApp;
class ev_QNXKeyboard;
class EV_QNXMouse;
class EV_QNXMenuBar;
class EV_QNXMenuPopup;

/*****************************************************************
******************************************************************
** This file defines the unix-platform-specific class for the
** cross-platform application frame.  This is used to hold all
** unix-specific data.  One of these is created for each top-level
** document window.
******************************************************************
*****************************************************************/

class XAP_QNXFrame : public XAP_Frame
{
public:
	XAP_QNXFrame(XAP_QNXApp * app);
	XAP_QNXFrame(XAP_QNXFrame * f);
	virtual ~XAP_QNXFrame(void);

	virtual UT_Bool				initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue,
										   const char * szMenuLayoutKey, const char * szMenuLayoutDefaultValue,
										   const char * szMenuLabelSetKey, const char * szMenuLabelSetDefaultValue,
										   const char * szToolbarLayoutsKey, const char * szToolbarLayoutsDefaultValue,
										   const char * szToolbarLabelSetKey, const char * szToolbarLabelSetDefaultValue);

	virtual	XAP_Frame *			cloneFrame(void)=0;
	virtual UT_Error			loadDocument(const char * szFilename, int ieft)=0;
	virtual UT_Bool				close(void);
	virtual UT_Bool				raise(void);
	virtual UT_Bool				show(void);
	virtual UT_Bool				openURL(const char * szURL);
	virtual UT_Bool				updateTitle(void);
	virtual UT_sint32			setInputMode(const char * szName);

	PtWidget_t *				getTopLevelWindow(void) const;
	PtWidget_t *				getVBoxWidget(void) const;
	EV_QNXMouse *				getQNXMouse(void);
	ev_QNXKeyboard *			getQNXKeyboard(void);
		
	virtual XAP_DialogFactory *	getDialogFactory(void);
	virtual void				setXScrollRange(void)=0;
	virtual void				setYScrollRange(void)=0;
	virtual UT_Bool				runModalContextMenu(AV_View * pView, const char * szMenuName,
													UT_sint32 x, UT_sint32 y);
	virtual void				translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y) = 0;
	virtual void				setStatusMessage(const char * szMsg) = 0;

	void						setTimeOfLastEvent(unsigned int eventTime);

	PhArea_t				m_AvailableArea;

	PtWidget_t * 				m_wSunkenBox;

	int							getPopupDone() { return m_PopupDone; };
	void						setPopupDone(int value) { m_PopupDone = value; };
	
protected:
	virtual PtWidget_t *			_createDocumentWindow(void)=0;
	virtual void *			_createStatusBarWindow(void)=0;
	virtual void				_createTopLevelWindow(void);
	virtual void				_setWindowIcon(void) = 0;
	
	// TODO see why ev_QNXKeyboard has lowercase prefix...
	XAP_QNXApp *				m_pQNXApp;
	ev_QNXKeyboard *			m_pQNXKeyboard;
	EV_QNXMouse *				m_pQNXMouse;
	EV_QNXMenuBar *			m_pQNXMenu;
	EV_QNXMenuPopup *			m_pQNXPopup; /* only valid while a context popup is up */
	UT_Vector					m_vecQNXToolbars;
	
	PtWidget_t *				m_wTopLevelWindow;

	PtWidget_t *				m_wVBox;
	PtWidget_t *				m_wStatusBar;

	AP_QNXDialogFactory		m_dialogFactory;
	int							m_PopupDone;

protected:
	class _fe
	{
	public:
		static int button_press_event(PtWidget_t * w, void *data, PtCallbackInfo_t * e);
		static int button_release_event(PtWidget_t * w, void *data,  PtCallbackInfo_t * e);
		static int motion_notify_event(PtWidget_t * w, void *data, PtCallbackInfo_t* e);
		static int key_press_event(PtWidget_t* w, void *data, PtCallbackInfo_t *info);
		static int resize(PtWidget_t * w, void *data,  PtCallbackInfo_t * info);
//		static int delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/);
		static int expose(PtWidget_t * w, PhTile_t* damage);
		static int vScrollChanged(PtWidget_t * w, void *data,  PtCallbackInfo_t * info);
		static int hScrollChanged(PtWidget_t * w, void *data,  PtCallbackInfo_t * info);
//		static void destroy (GtkWidget * /*widget*/, gpointer /*data*/);
	};
};

#endif /* XAP_QNXFRAME_H */
