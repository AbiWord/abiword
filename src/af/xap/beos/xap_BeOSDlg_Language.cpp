/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Language.h"
#include "xap_BeOSDlg_Language.h"

#include "ut_Rehydrate.h"

/*****************************************************************/

XAP_Dialog * XAP_BeOSDialog_Language::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_BeOSDialog_Language * p = new XAP_BeOSDialog_Language(pFactory,id);
	return p;
}

XAP_BeOSDialog_Language::XAP_BeOSDialog_Language(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: XAP_Dialog_Language(pDlgFactory,id)
{
}

XAP_BeOSDialog_Language::~XAP_BeOSDialog_Language(void)
{
}

void XAP_BeOSDialog_Language::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

/*
	See xap_Dlg_Language.h for instructions
*/	

	UT_ASSERT(UT_NOT_IMPLEMENTED);
}
