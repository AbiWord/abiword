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

class AP_App;
class AP_DialogFactory;
class ap_ViewListener;
class AV_View;
class AD_Document;
class EV_EditEventMapper;
class EV_Menu_Layout;
class EV_Menu_LabelSet;
class AV_ScrollObj;
class AP_FrameData;
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

class XAP_Frame
{
public:
	XAP_Frame(AP_App * app);
	XAP_Frame(XAP_Frame * f);
	virtual ~XAP_Frame(void);

	virtual UT_Bool				initialize(void);
	virtual	XAP_Frame *			cloneFrame(void)=0;
	virtual UT_Bool				loadDocument(const char * szFilename)=0;
	virtual UT_Bool				close(void)=0;
	virtual UT_Bool				raise(void)=0;
	virtual UT_Bool				show(void)=0;
	virtual UT_Bool				updateTitle(void);

	const EV_EditEventMapper *	getEditEventMapper(void) const;
	AP_App *					getApp(void) const;
	AV_View *					getCurrentView(void) const;
	const char *				getFilename(void) const;
	const char *				getTitle(int len) const;
	const char *				getTempNameFromTitle(void) const;

	UT_Bool						isDirty(void) const;

	void						setViewNumber(UT_uint32 n);
	UT_uint32					getViewNumber(void) const;
	const char *				getViewKey(void) const;

	virtual AP_DialogFactory *	getDialogFactory(void) = 0;
	virtual void				setXScrollRange(void) = 0;
	virtual void				setYScrollRange(void) = 0;
	
protected:
	AP_App *					m_app;			/* handle to application-specific data */
	AD_Document *				m_pDoc;			/* to our in-memory representation of a document */
	AV_View *					m_pView;		/* to our view on the document */
	ap_ViewListener *			m_pViewListener;
	AV_ListenerId				m_lid;
	AV_ScrollObj *				m_pScrollObj;	/* to our scroll handler */
	EV_EditEventMapper *		m_pEEM;			/* the event state-machine for this frame */
	const char *				m_szMenuLayoutName;
	const char *				m_szMenuLabelSetName;		/* language for menus */
	UT_Vector					m_vecToolbarLayoutNames;
	const char *				m_szToolbarLabelSetName;	/* language for toolbars */
	UT_uint32					m_nView;
	int							m_iUntitled;

	ap_Scrollbar_ViewListener * m_pScrollbarViewListener;
	AV_ListenerId				m_lidScrollbarViewListener;
	
	AP_FrameData *				m_pData;		/* app-specific frame data */

	static int					_getNextUntitledNumber(void);
	
private:
	char						m_szTitle[512];				/* TODO need #define for this number */
	char						m_szNonDecoratedTitle[512]; /* TODO need #define for this number */
	
	static int					s_iUntitled;	
};

#endif /* XAP_FRAME_H */
