/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_Dialog_Id.h"
#include "xap_MacDlg_FileOpenSaveAs.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"
#include "ie_imp.h"
#include "ie_exp.h"

/*****************************************************************/
AP_Dialog * AP_MacDialog_FileOpenSaveAs::static_constructor(AP_DialogFactory * pFactory,
															  AP_Dialog_Id id)
{
	AP_MacDialog_FileOpenSaveAs * p = new AP_MacDialog_FileOpenSaveAs(pFactory,id);
	return p;
}

AP_MacDialog_FileOpenSaveAs::AP_MacDialog_FileOpenSaveAs(AP_DialogFactory * pDlgFactory,
															 AP_Dialog_Id id)
	: AP_Dialog_FileOpenSaveAs(pDlgFactory,id)
{
}

AP_MacDialog_FileOpenSaveAs::~AP_MacDialog_FileOpenSaveAs(void)
{
}

/*****************************************************************/


void AP_MacDialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	return;
}

