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


#ifndef XAP_FRAME_H
#define XAP_FRAME_H

#include "ut_types.h"
#include "ut_vector.h"
#include "xav_Listener.h"	// for AV_ListenerID
#include "xap_Dlg_MessageBox.h"
#include "xap_Strings.h"



class XAP_App;
class XAP_DialogFactory;
class ap_ViewListener;
class AV_View;
class AD_Document;
class EV_EditEventMapper;
class EV_Menu_Layout;
class EV_Menu_LabelSet;
class EV_EditBindingMap;
class EV_Mouse;
class EV_Toolbar;
class EV_Keyboard;
class AV_ScrollObj;
class ap_Scrollbar_ViewListener;

/*****************************************************************
******************************************************************
** This file defines the base class for the cross-platform 
** application frame.  This is used to hold all of the
** window-specific data.  One of these is created for each
** top-level document window.  (If we do splitter windows,
** the views will probably share this frame instance.)
******************************************************************
*****************************************************************/

class XAP_InputModes
{
public:
	XAP_InputModes(void);
	~XAP_InputModes(void);

	UT_Bool							createInputMode(const char * szName,
													EV_EditBindingMap * pBindingMap);
	UT_Bool							setCurrentMap(const char * szName);
	EV_EditEventMapper *			getCurrentMap(void) const;
	const char * 					getCurrentMapName(void) const;
	EV_EditEventMapper *			getMapByName(const char * szName) const;

protected:
	UT_Vector						m_vecEventMaps; /* EV_EditEventMapper * */
	UT_Vector						m_vecNames;		/* const char * */
	
	UT_uint32						m_indexCurrentEventMap;
};

//////////////////////////////////////////////////////////////////
	
class XAP_Frame
{
public:
	XAP_Frame(XAP_App * app);
	XAP_Frame(XAP_Frame * f);
	virtual ~XAP_Frame(void);

	virtual UT_Bool				initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue,
										   const char * szMenuLayoutKey, const char * szMenuLayoutDefaultValue,
										   const char * szMenuLabelSetKey, const char * szMenuLabelSetDefaultValue,
										   const char * szToolbarLayoutsKey, const char * szToolbarLayoutsDefaultValue,
										   const char * szToolbarLabelSetKey, const char * szToolbarLabelSetDefaultValue);
	
	virtual	XAP_Frame *			cloneFrame(void)=0;
	virtual UT_Error   			loadDocument(const char * szFilename, int ieft)=0;
	virtual UT_Bool				close(void)=0;
	virtual UT_Bool				raise(void)=0;
	virtual UT_Bool				show(void)=0;
	virtual UT_Bool				openURL(const char * szURL)=0;
	virtual UT_Bool				updateTitle(void);
	virtual UT_sint32			setInputMode(const char * szName);
	const char *				getInputMode(void) const;

	EV_EditEventMapper *		getEditEventMapper(void) const;
	XAP_App *				getApp(void) const;
	AV_View *		       		getCurrentView(void) const;
	AD_Document *				getCurrentDoc(void) const;
	const char *				getFilename(void) const;
	const char *				getTitle(int len) const;
	const char *				getTempNameFromTitle(void) const;

	UT_Bool						isDirty(void) const;

	void						setViewNumber(UT_uint32 n);
	UT_uint32					getViewNumber(void) const;
	const char *				getViewKey(void) const;

	inline void *				getFrameData(void) const { return m_pData; }

	virtual XAP_DialogFactory *	getDialogFactory(void) = 0;
	virtual void				setXScrollRange(void) = 0;
	virtual void				setYScrollRange(void) = 0;

	virtual UT_Bool				runModalContextMenu(AV_View * pView, const char * szMenuName,
													UT_sint32 x, UT_sint32 y) = 0;

	virtual void				setZoomPercentage(UT_uint32 iZoom);
	virtual UT_uint32			getZoomPercentage(void);

	virtual void				setStatusMessage(const char * szMsg) = 0;

	virtual void				toggleRuler(UT_Bool /*bRulerOn*/) { } //
	
   	EV_Mouse *					getMouse(void);
	EV_Keyboard *				getKeyboard(void);

	XAP_Dialog_MessageBox::tAnswer		showMessageBox(XAP_String_Id id,
											  XAP_Dialog_MessageBox::tButtons buttons,
											  XAP_Dialog_MessageBox::tAnswer default_answer,
											  const char *p_str1 = NULL);
	XAP_Dialog_MessageBox::tAnswer		showMessageBox(const char *szMessage,
											  XAP_Dialog_MessageBox::tButtons buttons,
											  XAP_Dialog_MessageBox::tAnswer default_answer);

protected:
	virtual void				_createToolbars(void);
	virtual EV_Toolbar *		_newToolbar(XAP_App *app, XAP_Frame *frame, const char *, const char *)
									{ return NULL; } // Abstract

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
	
	void *						m_pData;		/* app-specific frame data */

	XAP_InputModes *			m_pInputModes;
	
	static int					_getNextUntitledNumber(void);
	
private:
	char						m_szTitle[512];				/* TODO need #define for this number */
	char						m_szNonDecoratedTitle[512]; /* TODO need #define for this number */
	
	static int					s_iUntitled;	
};

#endif /* XAP_FRAME_H */
