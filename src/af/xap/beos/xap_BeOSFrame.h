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


#ifndef XAP_BEOSFRAME_H
#define XAP_BEOSFRAME_H

#include <InterfaceKit.h>

#include "xap_Frame.h"
#include "ut_vector.h"
#include "xap_BeOSDialogFactory.h"

class XAP_BeOSApp;
class ev_BeOSKeyboard;
class ev_BeOSMouse;
class EV_BeOSMenu;
class GR_Graphics;

/*****************************************************************
******************************************************************
** This file defines the beos-platform-specific class for the
** cross-platform application frame.  This is used to hold all
** BeOS-specific data.  One of these is created for each top-level
** document window.
******************************************************************
*****************************************************************/
class XAP_BeOSFrame;

class TFScrollBar: public BScrollBar {
	public:
		TFScrollBar(XAP_BeOSFrame *pFrame, BRect frame, const char *name,
					BView *target, float min, float max, 
					orientation direction);
		 virtual void ValueChanged(float newValue);
		
		XAP_BeOSFrame *m_pBeOSFrame;				
};

class be_DocView: public BView {
	public: 
		be_DocView(BRect frame, const char *name, 
			   uint32 resizeMask, uint32 flags);
		virtual	void Draw(BRect updateRect);
		virtual void SetPrintPicture(BPicture *pic) { m_pBPicture = pic; };
		virtual	void FrameResized(float new_width, float new_height);

	BPicture *	m_pBPicture;
};

class be_Window: public BWindow {
	public:
		be_Window(XAP_BeOSApp *theApp, XAP_BeOSFrame *theFrame,
			  BRect r, char *name, 
			  window_look look, window_feel feel, 
			  uint32 flags = 0, 
			  uint32 workspace = B_CURRENT_WORKSPACE);

		bool _createWindow(const char *szMenuLayoutName,
				   const char *szMenuLabelSetName);

		//Located in the app specific code
		be_DocView *		_createDocumentWindow(void);

		//Be Function overrides
		virtual bool QuitRequested(void);

		be_DocView		*m_pbe_DocView;
		EV_BeOSMenu 		*m_pBeOSMenu;
		XAP_BeOSApp 		*m_pBeOSApp;
		XAP_BeOSFrame 		*m_pBeOSFrame;
		TFScrollBar		*m_hScroll, *m_vScroll;
		BRect			m_winRectAvailable;
};

/*****************************************************************/

class XAP_BeOSFrame : public XAP_Frame
{
public:
	XAP_BeOSFrame(XAP_BeOSApp * app);
	XAP_BeOSFrame(XAP_BeOSFrame * f);
	virtual ~XAP_BeOSFrame(void);

	virtual UT_Bool initialize(const char * szKeyBindingsKey, 
				  const char * szKeyBindingsDefaultValue,
				  const char * szMenuLayoutKey, 
				  const char * szMenuLayoutDefaultValue,
				  const char * szMenuLabelSetKey, 
				  const char * szMenuLabelSetDefaultValue,
				  const char * szToolbarLayoutsKey, 
				  const char * szToolbarLayoutsDefaultValue,
				  const char * szToolbarLabelSetKey, 
				  const char * szToolbarLabelSetDefaultValue);
	
	virtual	XAP_Frame *			cloneFrame(void)=0;
	virtual UT_Bool				loadDocument(const char * szFilename,  int ieft)=0;
	virtual UT_Bool				close(void);
	virtual UT_Bool				raise(void);
	virtual UT_Bool				show(void);
	virtual UT_Bool				openURL(const char * szURL);
	virtual UT_Bool				updateTitle(void);

	BWindow *				getTopLevelWindow(void) const;
	be_DocView *				getBeDocView(void) const;
	void 					setBeDocView(be_DocView *);
	UT_sint32				setInputMode(const char * szName);
	ev_BeOSMouse *				getBeOSMouse(void);
	ev_BeOSKeyboard *			getBeOSKeyboard(void);

	virtual XAP_DialogFactory 	*getDialogFactory(void);
	virtual void				setXScrollRange(void)=0;
	virtual void				setYScrollRange(void)=0;
	virtual UT_Bool 			runModalContextMenu(AV_View * pView, const char * szMenuName, UT_sint32 x, UT_sint32 y);
	virtual void                            setStatusMessage(const char * szMsg) = 0;

	//TF Added 
 	UT_Vector *                             VecBeOSToolbars();
	UT_Vector *				VecToolbarLayoutNames();
  	const char *                            ToolbarLabelSetName();
	GR_Graphics *				Graphics();
	void					setScrollBars(TFScrollBar *h, TFScrollBar *v);

	virtual void				toggleRuler(UT_Bool bRulerOn) = 0;

protected:
//	virtual GtkWidget *			_createDocumentWindow(void)=0;
	virtual void				_createTopLevelWindow(void);

	// TODO see why ev_BeOSKeyboard has lowercase prefix...
	XAP_BeOSApp *				m_pBeOSApp;
	ev_BeOSKeyboard *			m_pBeOSKeyboard;
	ev_BeOSMouse *				m_pBeOSMouse;
	EV_BeOSMenu *				m_pBeOSMenu;
	UT_Vector				m_vecBeOSToolbars;
	
	//Main window and  document view 
	be_Window *				m_pBeWin;			
	be_DocView *				m_pBeDocView;						
	TFScrollBar *				m_hScroll;
	TFScrollBar *				m_vScroll;
/*
	GtkWidget *				m_wTopLevelWindow;
	GtkWidget *				m_wVBox;
	GtkWidget * 				m_wSunkenBox;
*/

	AP_BeOSDialogFactory		m_dialogFactory;

protected:

};

#endif /* XAP_BEOSFRAME_H */
