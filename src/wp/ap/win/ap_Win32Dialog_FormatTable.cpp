/* AbiWord
 * Copyright (C) 2002 Jordi Mas i Hernàndez <jmas@softcatala.org>
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

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_MergeCells.h"
#include "AP_Win32Dialog_FormatTable.h"
#include "ap_Win32Resources.rc2"
#include "xap_Win32DialogHelper.h"
#include "xap_Win32Toolbar_Icons.h"
//#include "ut_Xpm2Bmp.h"

#define GWL(hwnd)		(AP_Win32Dialog_FormatTable*)GetWindowLong((hwnd), DWL_USER)
#define SWL(hwnd, d)	(AP_Win32Dialog_FormatTable*)SetWindowLong((hwnd), DWL_USER,(LONG)(d))

XAP_Dialog * AP_Win32Dialog_FormatTable::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	AP_Win32Dialog_FormatTable * p = new AP_Win32Dialog_FormatTable(pFactory,id);
	return p;
}

AP_Win32Dialog_FormatTable::AP_Win32Dialog_FormatTable(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_FormatTable(pDlgFactory,id)
{
		

}   
    
AP_Win32Dialog_FormatTable::~AP_Win32Dialog_FormatTable(void)
{
}

void AP_Win32Dialog_FormatTable::runModeless(XAP_Frame * pFrame)
{
	// Not implemneted yet
	UT_ASSERT(0);	
		
}

