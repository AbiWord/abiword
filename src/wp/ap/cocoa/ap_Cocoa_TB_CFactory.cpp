/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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

#include "ap_Toolbar_Id.h"
#include "xap_Cocoa_TB_CFactory.h"

/*****************************************************************/
/*****************************************************************/

// get the class definitions

#include "ap_CocoaToolbar_Control_All.h"

// fill in the table

static struct XAP_Toolbar_ControlFactory::_ctl_table s_ctl_table[] = {
	
#define Declare_Control(id,cls)	{ id, cls::static_constructor },
#include "ap_CocoaToolbar_Control_All.h"
#undef Declare_Control
	
};

/*****************************************************************/
/*****************************************************************/
  
AP_CocoaToolbar_ControlFactory::AP_CocoaToolbar_ControlFactory()
	: XAP_Toolbar_ControlFactory(G_N_ELEMENTS(s_ctl_table), s_ctl_table)
{
}

AP_CocoaToolbar_ControlFactory::~AP_CocoaToolbar_ControlFactory(void)
{
}

