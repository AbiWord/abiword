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

#ifndef XAP_DIALOGFACTORY_H
#define XAP_DIALOGFACTORY_H

/*****************************************************************
******************************************************************
** This file defines the base class for the cross-platform
** application dialog factory.  This is used as a container
** and constructor for all dialogs.
******************************************************************
*****************************************************************/

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */

#if defined(__MINGW32__)
#  undef snprintf
#  if __GNUC__ <= 3
#    define _GLIBCXX_USE_C99_DYNAMIC 1
#  endif
#endif

#include <map>

#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_misc.h"
#include "ut_vector.h"

#include "xap_Dialog.h"
#include "xap_Types.h"

class XAP_Dialog;
class XAP_App;
class XAP_Frame;

/*****************************************************************/

class ABI_EXPORT XAP_DialogFactory
{
public:
	struct _dlg_table
	{
		XAP_Dialog_Id	m_id;
		XAP_Dialog_Type	m_type;
		XAP_Dialog *	(*m_pfnStaticConstructor)(XAP_DialogFactory *, XAP_Dialog_Id id);
		bool			m_tabbed;
	};

	XAP_DialogFactory(XAP_App * pApp, int nrElem, const struct _dlg_table * pDlgTable, XAP_Frame * pFrame = NULL);
	virtual ~XAP_DialogFactory(void);

	inline XAP_App *	getApp(void) const	{ return m_pApp; };

	XAP_Dialog *		requestDialog(XAP_Dialog_Id id);
	XAP_Dialog *		justMakeTheDialog(XAP_Dialog_Id id);
	void				releaseDialog(XAP_Dialog * pDialog);
	XAP_Dialog_Id		getNextId(void);
	XAP_Dialog_Id		registerDialog(XAP_Dialog *(*pStaticConstructor)(XAP_DialogFactory *, XAP_Dialog_Id id),XAP_Dialog_Type iDialogType);
	void				unregisterDialog(XAP_Dialog_Id id);

	bool				registerNotebookPage(XAP_Dialog_Id dialog, const XAP_NotebookDialog::Page * page);
	bool				unregisterNotebookPage(XAP_Dialog_Id dialog, const XAP_NotebookDialog::Page * page);

protected:
	bool				_findDialogInTable(XAP_Dialog_Id id, UT_sint32 * pIndex) const;

	XAP_App *			m_pApp;
	XAP_Frame *			m_pFrame;
	XAP_Dialog_Type		m_dialogType;
	UT_Vector			m_vecDialogs;
	UT_NumberVector		m_vecDialogIds;

	UT_uint32			m_nrElementsDlgTable;
	UT_GenericVector<const _dlg_table *>	m_vec_dlg_table;			/* a Vector of elements */
	UT_GenericVector<_dlg_table *>	m_vecDynamicTable;			/* a Vector of elements */

private:
	void addPages(XAP_NotebookDialog * pDialog, XAP_Dialog_Id id);
};

#endif /* XAP_DIALOGFACTORY_H */
