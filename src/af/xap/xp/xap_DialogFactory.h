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

#ifndef XAP_DIALOGFACTORY_H
#define XAP_DIALOGFACTORY_H

#include "ut_vector.h"
#include "xap_Dialog.h"

/*****************************************************************
******************************************************************
** This file defines the base class for the cross-platform 
** application dialog factory.  This is used as a container
** and constructor for all dialogs.
******************************************************************
*****************************************************************/

#include "xap_Types.h"
class XAP_Dialog;
class XAP_App;
class XAP_Frame;

/*****************************************************************/

class XAP_DialogFactory
{
public:
	struct _dlg_table
	{
		XAP_Dialog_Id	m_id;
		XAP_Dialog_Type	m_type;
		XAP_Dialog *	(*m_pfnStaticConstructor)(XAP_DialogFactory *, XAP_Dialog_Id id);
	};

	XAP_DialogFactory(XAP_App * pApp, int nrElem, const struct _dlg_table * pDlgTable);
	XAP_DialogFactory(XAP_Frame * pFrame, XAP_App * pApp, int nrElem, const struct _dlg_table * pDlgTable);
	virtual ~XAP_DialogFactory(void);

	inline XAP_App *	getApp(void) const	{ return m_pApp; };

	XAP_Dialog *		requestDialog(XAP_Dialog_Id id);
	void				releaseDialog(XAP_Dialog * pDialog);

protected:
	bool				_findDialogInTable(XAP_Dialog_Id id, UT_uint32 * pIndex) const;
	
	XAP_App *			m_pApp;
	XAP_Frame *			m_pFrame;
	XAP_Dialog_Type		m_dialogType;
	UT_Vector			m_vecDialogs;
	UT_Vector			m_vecDialogIds;

	UT_uint32					m_nrElementsDlgTable;
	const _dlg_table *	m_dlg_table;			/* an array of elements */
};

#endif /* XAP_DIALOGFACTORY_H */
