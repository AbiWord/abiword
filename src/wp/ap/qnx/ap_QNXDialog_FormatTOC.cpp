/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
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
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_FrameImpl.h"


#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_FormatTOC.h"
#include "ap_QNXDialog_FormatTOC.h"

/*****************************************************************/

XAP_Dialog * AP_QNXDialog_FormatTOC::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_QNXDialog_FormatTOC * p = new AP_QNXDialog_FormatTOC(pFactory,id);
	return p;
}

AP_QNXDialog_FormatTOC::AP_QNXDialog_FormatTOC(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_FormatTOC(pDlgFactory,id)
{
}

AP_QNXDialog_FormatTOC::~AP_QNXDialog_FormatTOC(void)
{
}
void AP_QNXDialog_FormatTOC::setTOCPropsInGUI(void)
{
	UT_ASSERT(0);
}

void AP_QNXDialog_FormatTOC::setSensitivity(bool bSensitive)
{
	UT_ASSERT(0);
}

void AP_QNXDialog_FormatTOC::destroy(void)
{
	UT_ASSERT(0);
	finalize();
}

void AP_QNXDialog_FormatTOC::activate(void)
{
	UT_ASSERT (0);
}

void AP_QNXDialog_FormatTOC::notifyActiveFrame(XAP_Frame *pFrame)
{
    UT_ASSERT(0);
}

void AP_QNXDialog_FormatTOC::runModeless(XAP_Frame * pFrame)
{
	UT_ASSERT(0);
	startUpdater();
}
