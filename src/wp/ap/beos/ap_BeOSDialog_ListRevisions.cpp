/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_ListRevisions.h"
#include "ap_BeOSDialog_ListRevisions.h"

#include "ut_Rehydrate.h"

/*****************************************************************/

XAP_Dialog * AP_BeOSDialog_ListRevisions::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_BeOSDialog_ListRevisions * p = new AP_BeOSDialog_ListRevisions(pFactory,id);
	return p;
}

AP_BeOSDialog_ListRevisions::AP_BeOSDialog_ListRevisions(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_ListRevisions(pDlgFactory,id)
{
}

AP_BeOSDialog_ListRevisions::~AP_BeOSDialog_ListRevisions(void)
{
}

void AP_BeOSDialog_ListRevisions::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	/*
	   see the screenshot posted to the dev-list (25/05/2002);
	   use the provided functions getTitle(), getLabel1(),
	   getColumn1Label(), getColumn2Label(), getItemCount(),
	   getNthItemId() and getNthItemText() to fill the list

	   if the user clicks OK but there is no selection, set m_iId to 0
           otherwise set m_iId to the id of the selected revision
	*/


	UT_ASSERT(UT_NOT_IMPLEMENTED);
}
