/* AbiWord
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

#ifndef XAP_TOOLBAR_CONTROLFACTORY_H
#define XAP_TOOLBAR_CONTROLFACTORY_H

/*****************************************************************
******************************************************************
** This file defines the base class for the cross-platform
** application Toolbar_Control factory.  This is used as a container
** and constructor for all Toolbar_Controls.
******************************************************************
*****************************************************************/

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
/* #include "ut_vector.h" */

/* #include "EV_Toolbar_Control.h" */

#include "xap_Types.h"

class EV_Toolbar_Control;
class EV_Toolbar;

/*****************************************************************/

class ABI_EXPORT XAP_Toolbar_ControlFactory
{
public:
	struct _ctl_table
	{
		XAP_Toolbar_Id			m_id;
		EV_Toolbar_Control *	(*m_pfnStaticConstructor)(EV_Toolbar * pToolbar, XAP_Toolbar_Id id);
	};

	XAP_Toolbar_ControlFactory(int nrElem, const struct _ctl_table * pDlgTable);
	virtual ~XAP_Toolbar_ControlFactory(void);

	EV_Toolbar_Control *		getControl(EV_Toolbar * pToolbar, XAP_Toolbar_Id id);

protected:
	bool						_find_ControlInTable(XAP_Toolbar_Id id, UT_uint32 * pIndex) const;

	UT_uint32					m_nrElementsCtlTable;
	const _ctl_table *	m_ctl_table;			/* an array of elements */
};

#endif /* XAP_TOOLBAR_CONTROLFACTORY_H */
