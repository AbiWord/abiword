/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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

/*****************************************************************/

// get the class definitions

#include "ap_Dialog_Id.h"
#include "xap_UnixDialogFactory.h"
#include "ap_UnixDialog_All.h"

// fill in the table


static struct XAP_DialogFactory::_dlg_table s_dlg_table[] = {
	
#define DeclareDialog(id,cls,tabbed)	{ id, cls::s_getPersistence(), cls::static_constructor, tabbed },
#include "ap_UnixDialog_All.h"
#undef DeclareDialog
};


/*****************************************************************/
  
AP_UnixDialogFactory::AP_UnixDialogFactory(XAP_App * pApp, XAP_Frame * pFrame)
	: XAP_DialogFactory(pApp, G_N_ELEMENTS(s_dlg_table), s_dlg_table, pFrame)
{
}

AP_UnixDialogFactory::~AP_UnixDialogFactory(void)
{
}
