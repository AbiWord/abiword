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

#ifndef AP_DIALOGFACTORY_H
#define AP_DIALOGFACTORY_H

#include "ut_vector.h"
#include "ap_Dialog.h"

/*****************************************************************
******************************************************************
** This file defines the base class for the cross-platform 
** application dialog factory.  This is used as a container
** and constructor for all dialogs.
******************************************************************
*****************************************************************/

#include "ap_Types.h"
class AP_Dialog;
class AP_App;
class AP_Frame;

/*****************************************************************/

class AP_DialogFactory
{
public:
	struct _dlg_table
	{
		AP_Dialog_Id	m_id;
		AP_Dialog_Type	m_type;
		AP_Dialog *		(*m_pfnStaticConstructor)(AP_DialogFactory *, AP_Dialog_Id id);
	};

	AP_DialogFactory(AP_App * pApp, int nrElem, const struct _dlg_table * pDlgTable);
	AP_DialogFactory(AP_Frame * pFrame, int nrElem, const struct _dlg_table * pDlgTable);
	virtual ~AP_DialogFactory(void);

	AP_Dialog *			requestDialog(AP_Dialog_Id id);
	void				releaseDialog(AP_Dialog * pDialog);

protected:
	UT_Bool				_findDialogInTable(AP_Dialog_Id id, UT_uint32 * pIndex) const;
	
	AP_App *			m_pApp;
	AP_Frame *			m_pFrame;
	AP_Dialog_Type		m_dialogType;
	UT_Vector			m_vecDialogs;
	UT_Vector			m_vecDialogIds;

	UT_uint32					m_nrElementsDlgTable;
	const struct _dlg_table *	m_dlg_table;			/* an array of elements */
};

#endif /* AP_DIALOGFACTORY_H */
