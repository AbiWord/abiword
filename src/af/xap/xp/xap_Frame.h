/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


#ifndef AP_FRAME_H
#define AP_FRAME_H

#include "ut_types.h"

class AP_Ap;
class PD_Document;
class FV_View;
class EV_EditBindingMap;
class EV_EditEventMapper;
class EV_Menu_Layout;
class EV_Menu_LabelSet;
class FV_ScrollObj;

/*****************************************************************
******************************************************************
** This file defines the base class for the cross-platform 
** application frame.  This is used to hold all of the
** window-specific data.  One of these is created for each
** top-level document window.  (If we do splitter windows,
** the views will probably share this frame instance.)
******************************************************************
*****************************************************************/

class AP_Frame
{
public:
	AP_Frame(AP_Ap * ap);
	virtual ~AP_Frame(void);

	virtual UT_Bool				initialize(int * pArgc, char *** pArgv);
	virtual UT_Bool				loadDocument(const char * szFilename);

	const EV_Menu_Layout *		getMenuLayout(void) const;
	const EV_Menu_LabelSet *	getMenuLabelSet(void) const;
	const EV_EditEventMapper *	getEditEventMapper(void) const;
	FV_View *					getCurrentView(void) const;
	
protected:
	AP_Ap *						m_ap;			/* handle to application-specific data */
	PD_Document *				m_pDoc;			/* to our in-memory representation of a document */
	FV_View *					m_pView;		/* to our view on the document */
	FV_ScrollObj *				m_pScrollObj;	/* to our scroll handler */
	EV_EditBindingMap *			m_pEBM;			/* the key/mouse bindings for this frame */
	EV_EditEventMapper *		m_pEEM;			/* the event state-machine for this frame */
	EV_Menu_Layout *			m_pMenuLayout;	/* abstract ordering of our menu */
	EV_Menu_LabelSet *			m_pMenuLabelSet;/* strings (in a given language) for the menu */
	
};

#endif /* AP_FRAME_H */
