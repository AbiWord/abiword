/* AbiWord
 * Copyright (C) 2002 Gabriel Gerhardsson
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
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "xap_Dialog_Id.h"
#include "ap_CocoaDialog_Download_File.h"

#include "ut_bytebuf.h"
#include "ut_png.h"
#include "ut_worker.h"


XAP_Dialog * AP_CocoaDialog_Download_File::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id theId)
{
	AP_CocoaDialog_Download_File * p = new AP_CocoaDialog_Download_File(pFactory,theId);
	return (XAP_Dialog*)p;
}

AP_CocoaDialog_Download_File::AP_CocoaDialog_Download_File(XAP_DialogFactory * pDlgFactory,
											 XAP_Dialog_Id theId)
	: AP_Dialog_Download_File(pDlgFactory,theId)
{
}

AP_CocoaDialog_Download_File::~AP_CocoaDialog_Download_File(void)
{
}


void AP_CocoaDialog_Download_File::_runModal(XAP_Frame * pFrame)
{
}

void
AP_CocoaDialog_Download_File::_abortDialog(void)
{
}

