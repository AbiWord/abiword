 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
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

	virtual UT_Bool				initialize(int argc, char ** argv);

protected:
	AP_Ap *						m_ap;			/* handle to application-specific data */
	PD_Document *				m_pDoc;			/* to our in-memory representation of a document */
	FV_View *					m_pView;		/* to our view on the document */
	EV_EditBindingMap *			m_pEBM;			/* the key/mouse bindings for this frame */
	EV_EditEventMapper *		m_pEEM;			/* the event state-machine for this frame */
	EV_Menu_Layout *			m_pMenuLayout;	/* abstract ordering of our menu */
	EV_Menu_LabelSet *			m_pMenuLabelSet;/* strings (in a given language) for the menu */
	
};

#endif /* AP_FRAME_H */
