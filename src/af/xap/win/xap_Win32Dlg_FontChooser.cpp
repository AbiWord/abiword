/* AbiSource Application Framework
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

#include <windows.h>
#include "ut_assert.h"
#include "ap_Win32Dialog_FontChooser.h"
#include "ap_Win32App.h"
#include "ap_Win32Frame.h"

/*****************************************************************/
AP_Dialog * AP_Win32Dialog_FontChooser::static_constructor(AP_DialogFactory * pFactory,
														  AP_Dialog_Id id)
{
	AP_Win32Dialog_FontChooser * p = new AP_Win32Dialog_FontChooser(pFactory,id);
	return p;
}

AP_Win32Dialog_FontChooser::AP_Win32Dialog_FontChooser(AP_DialogFactory * pDlgFactory,
													 AP_Dialog_Id id)
	: AP_Dialog_FontChooser(pDlgFactory,id)
{
}

AP_Win32Dialog_FontChooser::~AP_Win32Dialog_FontChooser(void)
{
}

/*****************************************************************/

void AP_Win32Dialog_FontChooser::runModal(AP_Frame * pFrame)
{
	m_pWin32Frame = (AP_Win32Frame *)pFrame;
	UT_ASSERT(m_pWin32Frame);
	AP_Win32App * pApp = (AP_Win32App *)m_pWin32Frame->getApp();
	UT_ASSERT(pApp);

	// TODO do the right thing....
	
	// the caller can get the answer from getAnswer().

	m_pWin32Frame = NULL;
}

