 
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

#ifndef EV_MENU_LABELS_H
#define EV_MENU_LABELS_H

#include "ap_Menu_Id.h"

/*****************************************************************
******************************************************************
** This file defines a framework for the string labels which
** are used with menus.  This binding allows us to localize
** menus in a platform-independent manner.
**
** We create one EV_Menu_Label per menu-item (such as "File|Open")
** per language per application (not per window or actual menu).
**
** We create one EV_Menu_LabelSet per language per application.
**
******************************************************************
*****************************************************************/

// TODO decide if we should make all labels Unicode
// TODO decide if we really need tooltop for menu bar

class EV_Menu_Label
{
public:
	EV_Menu_Label(AP_Menu_Id id,
				  const char * szMenuLabel,		/* label on the actual menu itself */
				  const char * szToolTip,		/* display message on tool tip */
				  const char * szStatusMsg);	/* status bar message */
	~EV_Menu_Label(void);

	AP_Menu_Id						getMenuId(void) const;

protected:
	AP_Menu_Id						m_id;
	char *							m_szMenuLabel;
	char *							m_szToolTip;
	char *							m_szStatusMsg;
};

/*****************************************************************/

class EV_Menu_LabelSet					/* a glorified array with bounds checking */
{
public:
	EV_Menu_LabelSet(const char * szLanguage,
					 AP_Menu_Id first, AP_Menu_Id last);
	~EV_Menu_LabelSet(void);

	UT_Bool				setLabel(AP_Menu_Id id,
								 const char * szMenuLabel,
								 const char * szToolTip,
								 const char * szStatusMsg);
	EV_Menu_Label *		getLabel(AP_Menu_Id id) const;
	const char *		getLanguage(void) const;

protected:
	EV_Menu_Label **	m_labelTable;
	AP_Menu_Id			m_first;
	AP_Menu_Id			m_last;
	char *				m_szLanguage;	/* for the convenience of the app only  */
};

#endif /* EV_MENU_LABELS_H */
