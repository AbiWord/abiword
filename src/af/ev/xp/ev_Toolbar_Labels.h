/* AbiSource Program Utilities
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

#ifndef EV_TOOLBAR_LABELS_H
#define EV_TOOLBAR_LABELS_H

#include "xap_Types.h"


/*****************************************************************
******************************************************************
** This file defines a framework for the string and image labels
** which are used with toolbars.  This binding allows us to localize
** toolbars in a platform-independent manner.
**
** We create one EV_Toolbar_Label per toolbar-item (such as "File|Open")
** per language per application (not per window or actual toolbar).
**
** We create one EV_Toolbar_LabelSet per language per application.
**
******************************************************************
*****************************************************************/

// TODO decide if we should make all labels Unicode

class EV_Toolbar_Label
{
public:
	EV_Toolbar_Label(XAP_Toolbar_Id id,
					 const char * szToolbarLabel,	/* label on the actual toolbar itself */
					 const char * szIconName,		/* name of the icon we use */
					 const char * szToolTip,		/* display message on tool tip */
					 const char * szStatusMsg);		/* status bar message */
	~EV_Toolbar_Label(void);

	XAP_Toolbar_Id		getToolbarId(void) const;
	const char *		getToolbarLabel(void) const;
	const char *		getIconName(void) const;
	const char *		getToolTip(void) const;
	const char *		getStatusMsg(void) const;

protected:
	XAP_Toolbar_Id		m_id;
	char *				m_szToolbarLabel;
	char *				m_szIconName;
	char *				m_szToolTip;
	char *				m_szStatusMsg;
};

/*****************************************************************/

class EV_Toolbar_LabelSet					/* a glorified array with bounds checking */
{
public:
	EV_Toolbar_LabelSet(const char * szLanguage,
						XAP_Toolbar_Id first, XAP_Toolbar_Id last);
	~EV_Toolbar_LabelSet(void);

	UT_Bool				setLabel(XAP_Toolbar_Id id,
								 const char * szToolbarLabel,
								 const char * szIconName,
								 const char * szToolTip,
								 const char * szStatusMsg);
	EV_Toolbar_Label *	getLabel(XAP_Toolbar_Id id);
	const char *		getLanguage(void) const;

protected:
	EV_Toolbar_Label **	m_labelTable;
	XAP_Toolbar_Id		m_first;
	XAP_Toolbar_Id		m_last;
	char *				m_szLanguage;	/* for the convenience of the app only  */
};

#endif /* EV_TOOLBAR_LABELS_H */
