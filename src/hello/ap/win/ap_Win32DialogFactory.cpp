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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

/*****************************************************************/

#include <windows.h>
#include "xap_Dialog_Id.h"
#include "xap_Win32DialogFactory.h"

/*****************************************************************/
/*****************************************************************/

// get the class definitions

#include "ap_Win32Dialog_All.h"

// fill in the table

static struct XAP_DialogFactory::_dlg_table s_dlg_table[] = {
	
#define DeclareDialog(id,cls)	{ id, cls::s_getPersistence(), cls::static_constructor },
#include "ap_Win32Dialog_All.h"
#undef DeclareDialog
	
};

/*****************************************************************/
/*****************************************************************/
  
AP_Win32DialogFactory::AP_Win32DialogFactory(XAP_App * pApp)
	: XAP_DialogFactory(pApp, NrElements(s_dlg_table), s_dlg_table)
{
}

AP_Win32DialogFactory::AP_Win32DialogFactory(XAP_Frame * pFrame, XAP_App * pApp)
	: XAP_DialogFactory(pFrame, pApp, NrElements(s_dlg_table), s_dlg_table)
{
}

AP_Win32DialogFactory::~AP_Win32DialogFactory(void)
{
}

