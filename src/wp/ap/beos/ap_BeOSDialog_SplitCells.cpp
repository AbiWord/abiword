/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2004 Daniel Furrer
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

#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include <InterfaceKit.h>

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrameImpl.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_SplitCells.h"
#include "ap_BeOSDialog_SplitCells.h"
#include "ap_BeOSDialog_Columns.h"


XAP_Dialog * AP_BeOSDialog_SplitCells::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	return new AP_BeOSDialog_SplitCells(pFactory,id);
}

AP_BeOSDialog_SplitCells::AP_BeOSDialog_SplitCells(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_SplitCells(pDlgFactory,id)
{
}

AP_BeOSDialog_SplitCells::~AP_BeOSDialog_SplitCells(void)
{
}

void AP_BeOSDialog_SplitCells::runModeless(XAP_Frame * pFrame)
{
}

void AP_BeOSDialog_SplitCells::setSensitivity(AP_CellSplitType splitThis, bool bSens)
{
}

void AP_BeOSDialog_SplitCells::event_Close(void)
{
}

void AP_BeOSDialog_SplitCells::destroy(void)
{
}

void AP_BeOSDialog_SplitCells::activate(void)
{
}

void AP_BeOSDialog_SplitCells::notifyActiveFrame(XAP_Frame *pFrame)
{
}

/*****************************************************************/
BWindow * AP_BeOSDialog_SplitCells::_constructWindow(void)
{

return NULL;
}


