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

#include <stdio.h>
#include <stdlib.h>
#include "ut_string.h"
#include "ap_Win32Dialog_PageNumbers.h"

AP_Win32Dialog_PageNumbers::AP_Win32Dialog_PageNumbers (XAP_DialogFactory * pDlgFactory,
						     XAP_Dialog_Id id) : AP_Dialog_PageNumbers (pDlgFactory, id)
{

}

AP_Win32Dialog_PageNumbers::~AP_Win32Dialog_PageNumbers(void)
{
}

void AP_Win32Dialog_PageNumbers::runModal(XAP_Frame * pFrame)
{
}



