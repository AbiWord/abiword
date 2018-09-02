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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#ifndef XAP_Frame_H
#define XAP_Frame_H

#include <string>

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
#include <gsf/gsf-input.h>

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

/*****************************************************************
******************************************************************
** This file defines the base class for the cross-platform
** application frame.  This is used to hold all of the
** window-specific data.  One of these is created for each
** top-level document window.  (If we do splitter windows,
** the views will probably share this frame instance.)
******************************************************************
*****************************************************************/



//////////////////////////////////////////////////////////////////

// FOR FUTURE REFERENCE: XAP_FrameImpl defines services that are quite specific
// to the _implementation_ of the frame, and involve platform specific code
// XAP_Frame defines services that are specific to public interface (from the
// point of view of abi code) of the visible frame and not the implementation
// The line between these two things _is_ sometimes ambiguous. use your judgement
// when adding something onto the frame code

#include "xap_FrameImpl.h"

class ABI_EXPORT XAP_Frame
{
public:
	XAP_Frame(XAP_FrameImpl *pFrameImpl);
	XAP_Frame(XAP_Frame * pFrame);
	virtual ~XAP_Frame();

	virtual bool				initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue,
										   const char * szMenuLayoutKey, const char * szMenuLayoutDefaultValue,
										   const char * szMenuLabelSetKey, const char * szMenuLabelSetDefaultValue,
										   const char * szToolbarLayoutsKey, const char * szToolbarLayoutsDefaultValue,
										   const char * szToolbarLabelSetKey, const char * szToolbarLabelSetDefaultValue);

	virtual	XAP_Frame *			cloneFrame() = 0;
	virtual	XAP_Frame *			buildFrame(XAP_Frame * pClone) = 0;
	virtual UT_Error   			loadDocument(AD_Document* pDoc) = 0;
	virtual UT_Error   			loadDocument(const char * szFilename, int ieft) = 0;
	virtual UT_Error			loadDocument(const char * szFileName, int ieft, bool createNew) = 0;
	virtual UT_Error			loadDocument(GsfInput * input, int ieft) = 0;
	virtual UT_Error			importDocument (const char * szFilename, int ieft, bool markClean = false) = 0;

	// thin interface functions to facilities provided by the helper
	bool close() { return m_pFrameImpl->_close(); }
	bool raise() { return m_pFrameImpl->_raise(); }
	bool show() { return m_pFrameImpl->_show(); }
	bool updateTitle() { return m_pFrameImpl->_updateTitle(); }
	void setCursor(GR_Graphics::Cursor cursor) { m_pFrameImpl->_setCursor(cursor); }
	virtual void queue_resize() { m_pFrameImpl->_queue_resize(); }
	void setFullScreen(bool isFullScreen) { m_pFrameImpl->_setFullScreen(isFullScreen); }
	void hideMenuScroll(bool bHideMenuScroll) {
		m_bHideMenuScroll = bHideMenuScroll;
		m_pFrameImpl->_hideMenuScroll(bHideMenuScroll);}
	virtual XAP_DialogFactory * getDialogFactory() { return m_pFrameImpl->_getDialogFactory(); }
	virtual EV_Toolbar * _newToolbar(XAP_Frame *frame, const char *szLayout, const char *szLanguage) { return m_pFrameImpl->_newToolbar(frame, szLayout, szLanguage); }
	virtual EV_Menu* getMainMenu() { return m_pFrameImpl->_getMainMenu(); }

	XAP_FrameImpl * getFrameImpl() const { return m_pFrameImpl; }

	void                        nullUpdate () const { m_pFrameImpl->_nullUpdate(); }
	AV_View *		       		getCurrentView() const;
	AD_Document *				getCurrentDoc() const;
	void                        setView(AV_View * pView) {m_pView = pView;}
	void                        setDoc(AD_Document * pDoc) {m_pDoc = pDoc;}
	const char *				getFilename() const;
	const std::string &			getTitle() const;
	const char *				getNonDecoratedTitle() const;

	XAP_FrameMode getFrameMode();
	void setFrameMode(XAP_FrameMode iFrameMode);
	bool						isDirty() const;

	void						setViewNumber(UT_uint32 n);
	UT_uint32					getViewNumber() const;
	const char *				getViewKey() const;

	inline void *				getFrameData() const { return m_pData; }

	virtual void				setXScrollRange() = 0;
	virtual void				setYScrollRange() = 0;
	virtual void                quickZoom(UT_uint32 iZoom) = 0;
	bool runModalContextMenu(AV_View * pView, const char * szMenuName,
		UT_sint32 x, UT_sint32 y) { return m_pFrameImpl->_runModalContextMenu(pView, szMenuName, x, y); }

	typedef enum { z_200, z_100, z_75, z_PAGEWIDTH, z_WHOLEPAGE, z_PERCENT } tZoomType;
	virtual void				setZoomPercentage(UT_uint32 iZoom);
	virtual UT_uint32			getZoomPercentage();
	void						setZoomType(XAP_Frame::tZoomType z_Type){ m_zoomType = z_Type; }
	XAP_Frame::tZoomType		getZoomType() { return m_zoomType; }
	void						updateZoom();
	void                        quickZoom(void);

	virtual void				setStatusMessage(const char * szMsg) = 0;
	// TODO change that to be the main call.
	void                        setStatusMessage(const std::string & s)
	{
		setStatusMessage(s.c_str());
	}

	virtual void				toggleRuler(bool /*bRulerOn*/) { } //
	virtual void                toggleTopRuler(bool /*bRulerOn*/) = 0;
	virtual void                toggleLeftRuler(bool /*bRulerOn*/) = 0;
	virtual void				toggleBar(UT_uint32 /* iBarNb */, bool /* bBarOn */) { }
	virtual void				toggleStatusBar(bool /* bStatusBarOn */) { }
	virtual bool				getBarVisibility(UT_uint32 /*iBarNb*/) { return true; }

   	EV_Mouse *					getMouse() { return m_pFrameImpl->m_pMouse; }
	EV_Keyboard *				getKeyboard() { return m_pFrameImpl->m_pKeyboard; }

	EV_Toolbar *                getToolbar(UT_sint32 ibar);
	UT_sint32                   findToolbarNr(EV_Toolbar * pTB);
	virtual void                rebuildMenus(void) { m_pFrameImpl->_rebuildMenus();}
	bool                        repopulateCombos();

	void                        rebuildAllToolbars(void);
	void                        refillToolbarsInFrameData(void) { m_pFrameImpl->_refillToolbarsInFrameData(); }
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
	bool                        isBackupRunning(void)
	{ return m_bBackupRunning;}
	UT_sint32                   getAutoSavePeriod(void)
	{ return m_iAutoSavePeriod;}
	void						setAutoSaveFile(bool);
	void						setAutoSaveFilePeriod(int);
	void						setAutoSaveFileExt(const std::string &);

	XAP_Dialog_MessageBox *      createMessageBox(XAP_String_Id id,
												  XAP_Dialog_MessageBox::tButtons buttons,
												  XAP_Dialog_MessageBox::tAnswer default_answer,
												  ...);

	XAP_Dialog_MessageBox::tAnswer		showMessageBox(XAP_String_Id id,
													   XAP_Dialog_MessageBox::tButtons buttons,
													   XAP_Dialog_MessageBox::tAnswer default_answer);

	XAP_Dialog_MessageBox::tAnswer		showMessageBox(const std::string & sz,
													   XAP_Dialog_MessageBox::tButtons buttons,
													   XAP_Dialog_MessageBox::tAnswer default_answer);

	XAP_Dialog_MessageBox::tAnswer		showMessageBox(const char * sz,
													   XAP_Dialog_MessageBox::tButtons buttons,
													   XAP_Dialog_MessageBox::tAnswer default_answer);

	XAP_Dialog_MessageBox::tAnswer		showMessageBox(XAP_String_Id id,
													   XAP_Dialog_MessageBox::tButtons buttons,
													   XAP_Dialog_MessageBox::tAnswer default_answer,
													   const char * sz);

	XAP_Dialog_MessageBox::tAnswer		showMessageBox(XAP_Dialog_MessageBox * pDialog);

	UT_Error	    backup(const char* stExt = 0, UT_sint32 iEFT = -1);
	std::string       makeBackupName (const char * szExt = 0);

	bool                        isStatusBarShown(void) const { return m_bShowStatusbar;}
	bool                        isMenuBarShown(void) const { return m_bShowMenubar;}
	virtual void                setStatusBarShown(bool /*bShowStatusbar*/) {}
	virtual void                setMenuBarShown(bool /*bShowMenubar*/) {}
	time_t                      getTimeSinceSave() const;
	bool                        isFrameLocked(void) const
	                            {return m_bIsFrameLocked;}
	void                        setFrameLocked(bool bLock)
	                            {m_bIsFrameLocked = bLock;}
	bool                        isMenuScrollHidden(void) const
	{ return m_bHideMenuScroll;}
	UT_RGBColor getColorSelBackground () const;
	UT_RGBColor getColorSelForeground () const;

	void dragText();
	virtual UT_sint32 getDocumentAreaXoff(void)
	{ return 0;}
	virtual UT_sint32 getDocumentAreaYoff(void)
	{ return 0;}
protected:
	friend class XAP_FrameImpl;

	AD_Document *				m_pDoc;			/* to our in-memory representation of a document */
	AV_View *					m_pView;		/* to our view on the document */
	ap_ViewListener *			m_pViewListener;
	AV_ListenerId				m_lid;
	AV_ScrollObj *				m_pScrollObj;	/* to our scroll handler */
	UT_uint32					m_nView;
	int							m_iUntitled;
	ap_Scrollbar_ViewListener * m_pScrollbarViewListener;
	AV_ListenerId				m_lidScrollbarViewListener;
	XAP_Frame::tZoomType		m_zoomType;
	GR_Graphics::Cursor         m_cursor;
	void *						m_pData;		/* app-specific frame data */
	bool                        m_bHideMenuScroll;

	static int					_getNextUntitledNumber();

private:
	void						_createAutoSaveTimer();
	void						_removeAutoSaveFile();

	std::string					m_sTitle;
	std::string					m_sNonDecoratedTitle;

	UT_uint32					m_iIdAutoSaveTimer;
	UT_uint32					m_iAutoSavePeriod;
	std::string					m_stAutoSaveExt;
	std::string					m_stAutoSaveNamePrevious;
	bool						m_bBackupRunning;
	bool						m_bBackupInProgress;

	static int					s_iUntitled;

	XAP_Toolbar_Id              m_isrcId;
	UT_sint32                   m_isrcTBNr;
	XAP_Toolbar_Id              m_idestId;
	UT_sint32                   m_idestTBNr;
	bool                        m_bisDragging;
	bool                        m_bHasDropped;
	bool                        m_bHasDroppedTB;

	bool                        m_bFirstDraw; // WL_REFACTOR: should this go into the helper?
	bool                        m_bShowStatusbar;
	bool                        m_bShowMenubar;
	bool                        m_bIsFrameLocked;

	XAP_FrameImpl *           m_pFrameImpl; /* set by platform specific code */
	UT_uint32                   m_iZoomPercentage;
};

#endif /* XAP_Frame_H */









