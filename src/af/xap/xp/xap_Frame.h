/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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


#ifndef XAP_Frame_H
#define XAP_Frame_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_string_class.h"
#include "ut_vector.h"
#include "ut_misc.h"

#include "xav_Listener.h"	// for AV_ListenerID

#include "xap_Dlg_MessageBox.h"
#include "xap_Strings.h"
#include "xap_Types.h"

#include "gr_Graphics.h"

//#include "ev_Toolbar.h"

class XAP_App;
class XAP_DialogFactory;
class ap_ViewListener;
class AV_View;
class AD_Document;
class EV_EditEventMapper;
class EV_EditBindingMap;
class EV_Mouse;
class EV_Toolbar;
class EV_Keyboard;
class EV_Menu;
class AV_ScrollObj;
class ap_Scrollbar_ViewListener;
class UT_Worker;
class UT_Timer;

typedef enum _FrameModes
{
	XAP_NormalFrame,	// Normal Frame
	XAP_NoMenusWindowLess,	// No toplevel window or menus
	XAP_WindowLess // No toplevel window but menus are OK.
} XAP_FrameMode;

/*****************************************************************
******************************************************************
** This file defines the base class for the cross-platform 
** application frame.  This is used to hold all of the
** window-specific data.  One of these is created for each
** top-level document window.  (If we do splitter windows,
** the views will probably share this frame instance.)
******************************************************************
*****************************************************************/

class ABI_EXPORT XAP_InputModes
{
public:
	XAP_InputModes(void);
	~XAP_InputModes(void);

	bool							createInputMode(const char * szName,
													EV_EditBindingMap * pBindingMap);
	bool							setCurrentMap(const char * szName);
	EV_EditEventMapper *			getCurrentMap(void) const;
	const char * 					getCurrentMapName(void) const;
	EV_EditEventMapper *			getMapByName(const char * szName) const;

protected:
	UT_Vector						m_vecEventMaps; /* EV_EditEventMapper * */
	UT_Vector						m_vecNames;		/* const char * */
	
	UT_uint32						m_indexCurrentEventMap;
};

//////////////////////////////////////////////////////////////////
	
class ABI_EXPORT XAP_Frame
{
public:
	XAP_Frame(XAP_App * app);
	XAP_Frame(XAP_Frame * f);
	virtual ~XAP_Frame();

	virtual bool				initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue,
										   const char * szMenuLayoutKey, const char * szMenuLayoutDefaultValue,
										   const char * szMenuLabelSetKey, const char * szMenuLabelSetDefaultValue,
										   const char * szToolbarLayoutsKey, const char * szToolbarLayoutsDefaultValue,
										   const char * szToolbarLabelSetKey, const char * szToolbarLabelSetDefaultValue);
	
	virtual	XAP_Frame *			cloneFrame() = 0;
	virtual	XAP_Frame *			buildFrame(XAP_Frame * pClone) = 0;
	virtual UT_Error   			loadDocument(const char * szFilename, int ieft) = 0;
	virtual UT_Error                        loadDocument(const char * szFileName, int ieft, bool createNew) = 0;
	virtual UT_Error importDocument (const char * szFilename, int ieft, bool markClean = false) = 0;
	virtual bool				close() = 0;
	virtual bool				raise() = 0;
	virtual bool				show() = 0;
	virtual void setFullScreen(bool isFullScreen) = 0;
	virtual bool				openURL(const char * szURL) = 0;
	virtual bool				updateTitle();
	virtual UT_sint32			setInputMode(const char * szName);
	const char *				getInputMode() const;
	virtual void                            nullUpdate () const = 0;
	virtual void                setCursor(GR_Graphics::Cursor cursor) = 0;
	EV_EditEventMapper *		getEditEventMapper() const;
	XAP_App *					getApp() const;
	AV_View *		       		getCurrentView() const;
	AD_Document *				getCurrentDoc() const;
	void                        setView(AV_View * pView) {m_pView = pView;}
	void                        setDoc(AD_Document * pDoc) {m_pDoc = pDoc;}
	const char *				getFilename() const;
	const char *				getTitle(int len) const;
	const char *				getTempNameFromTitle() const;

	bool						isDirty() const;

	void						setViewNumber(UT_uint32 n);
	UT_uint32					getViewNumber() const;
	const char *				getViewKey() const;

	inline void *				getFrameData() const { return m_pData; }

	virtual XAP_DialogFactory *	getDialogFactory() = 0;
	virtual void				setXScrollRange() = 0;
	virtual void				setYScrollRange() = 0;

	virtual bool				runModalContextMenu(AV_View * pView, const char * szMenuName,
													UT_sint32 x, UT_sint32 y) = 0;

	typedef enum { z_200, z_100, z_75, z_PAGEWIDTH, z_WHOLEPAGE, z_PERCENT } tZoomType;
	virtual void				setZoomPercentage(UT_uint32 iZoom);
	virtual UT_uint32			getZoomPercentage();
	void						setZoomType(XAP_Frame::tZoomType z_Type){ m_zoomType = z_Type; } 
	XAP_Frame::tZoomType		getZoomType() { return m_zoomType; }
	void						updateZoom();

	virtual void				setStatusMessage(const char * szMsg) = 0;

	virtual void				toggleRuler(bool /*bRulerOn*/) { } //
	virtual void                            toggleTopRuler(bool /*bRulerOn*/) = 0;
	virtual void                            toggleLeftRuler(bool /*bRulerOn*/) = 0;
	virtual void				toggleBar(UT_uint32 /* iBarNb */, bool /* bBarOn */) { }
	virtual void				toggleStatusBar(bool /* bStatusBarOn */) { }
	virtual bool				getBarVisibility(UT_uint32 iBarNb) { return true; }

   	EV_Mouse *					getMouse();
	EV_Keyboard *				getKeyboard();
	EV_Toolbar *                getToolbar(UT_uint32 ibar);
	UT_sint32                   findToolbarNr(EV_Toolbar * pTB);
	virtual EV_Menu*			getMainMenu() = 0;
	virtual void                rebuildMenus(void) {}
	bool                        repopulateCombos();


	virtual void                rebuildToolbar(UT_uint32 ibar);
	void                        rebuildAllToolbars(void);
	virtual void                refillToolbarsInFrameData(void) =0;
	void                        dragBegin(XAP_Toolbar_Id srcId, 
										  EV_Toolbar * pTBsrc);
	void                        dragDropToIcon(XAP_Toolbar_Id srcId,
											   XAP_Toolbar_Id destId, 
											   EV_Toolbar * pTBsrc, 
											   EV_Toolbar * pTBdest);
	void                        dragDropToTB(XAP_Toolbar_Id srcId, 
											 EV_Toolbar * pTBsrc, 
											 EV_Toolbar * pTBdest);
	void                        dragEnd(XAP_Toolbar_Id srcId);


	void						setAutoSaveFile(bool);
	void						setAutoSaveFilePeriod(int);
	void						setAutoSaveFileExt(const UT_String &);
	
	XAP_Dialog_MessageBox::tAnswer		showMessageBox(XAP_String_Id id,
													   XAP_Dialog_MessageBox::tButtons buttons,
													   XAP_Dialog_MessageBox::tAnswer default_answer,
													   const char *p_str1 = NULL);
	XAP_Dialog_MessageBox::tAnswer		showMessageBox(const char *szMessage,
													   XAP_Dialog_MessageBox::tButtons buttons,
													   XAP_Dialog_MessageBox::tAnswer default_answer);

	UT_Error	    backup(const char* stExt = 0);
	UT_String       makeBackupName (const char * szExt = 0);
	static void     viewAutoUpdater(UT_Worker *wkr);

	// Useful to refresh the size of the Frame.  For instance,
	// when the user selects hide statusbar, the Frame has to be
	// resized in order to fill the gap leaved by the statusbar
	virtual void				queue_resize() {}
	const bool                  isStatusBarShown(void) const { return m_bShowStatusbar;}
	const bool                  isMenuBarShown(void) const { return m_bShowMenubar;}
	virtual void                setStatusBarShown(bool bShowStatusbar) {}
	virtual void                setMenuBarShown(bool bShowMenubar) {}
protected:
	virtual void				_createToolbars();
	virtual EV_Toolbar *		_newToolbar(XAP_App *app, XAP_Frame *frame, const char *, const char *) = 0; // Abstract
    void                        _startViewAutoUpdater(void);

	XAP_App *					m_app;			/* handle to application-specific data */
	AD_Document *				m_pDoc;			/* to our in-memory representation of a document */
	AV_View *					m_pView;		/* to our view on the document */
	ap_ViewListener *			m_pViewListener;
	AV_ListenerId				m_lid;
	AV_ScrollObj *				m_pScrollObj;	/* to our scroll handler */
	const char *				m_szMenuLayoutName;
	const char *				m_szMenuLabelSetName;		/* language for menus */
	UT_Vector					m_vecToolbarLayoutNames;
	const char *				m_szToolbarLabelSetName;	/* language for toolbars */
	const char *				m_szToolbarAppearance;
	UT_uint32					m_nView;
	int							m_iUntitled;
	UT_Vector                   m_vecToolbars;
	EV_Mouse *		 		  	m_pMouse;
	EV_Keyboard *				m_pKeyboard;
	ap_Scrollbar_ViewListener * m_pScrollbarViewListener;
	AV_ListenerId				m_lidScrollbarViewListener;
	XAP_Frame::tZoomType		m_zoomType;
	GR_Graphics::Cursor         m_cursor;
	void *						m_pData;		/* app-specific frame data */

	XAP_InputModes *			m_pInputModes;
	XAP_FrameMode               m_iFrameMode;
	
	static int					_getNextUntitledNumber();
	
private:
	void						_createAutoSaveTimer();

	char						m_szTitle[512];				/* TODO need #define for this number */
	char						m_szNonDecoratedTitle[512]; /* TODO need #define for this number */
	UT_uint32					m_iIdAutoSaveTimer;
	UT_uint32					m_iAutoSavePeriod;
	UT_String					m_stAutoSaveExt;
	bool						m_bBackupRunning;
	
	static int					s_iUntitled;	

	XAP_Toolbar_Id              m_isrcId;
	UT_sint32                   m_isrcTBNr;
	XAP_Toolbar_Id              m_idestId;
	UT_sint32                   m_idestTBNr;
	bool                        m_bisDragging;
	bool                        m_bHasDropped;
	bool                        m_bHasDroppedTB;

	UT_uint32                   m_ViewAutoUpdaterID;
	UT_Timer *                  m_ViewAutoUpdater;
	bool                        m_bFirstDraw;
	bool                        m_bShowStatusbar;
	bool                        m_bShowMenubar;
};

#endif /* XAP_Frame_H */










