/* AbiWord
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

/*****************************************************************/

#include "ap_Toolbar_Id.h"
#include "xap_MacTlbr_ControlFactory.h"

/*****************************************************************/
/*****************************************************************/

// get the class definitions

#include "ap_MacTlbr_Control_All.h"

// fill in the table

static struct AP_Toolbar_ControlFactory::_ctl_table s_ctl_table[] = { 0 };
	
#if 0
#define Declare_Control(id,cls)	{ id, cls::static_constructor },
#include "ap_MacToolbar_Control_All.h"
#undef Declare_Control
	
};
#endif

/*****************************************************************/
/*****************************************************************/
  
AP_MacToolbar_ControlFactory::AP_MacToolbar_ControlFactory()
	: AP_Toolbar_ControlFactory(NrElements(s_ctl_table), s_ctl_table)
{
}

AP_MacToolbar_ControlFactory::~AP_MacToolbar_ControlFactory(void)
{
}

