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

#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_MergeCells.h"
#include "ap_QNXDialog_MergeCells.h"

XAP_Dialog * AP_QNXDialog_MergeCells::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	AP_QNXDialog_MergeCells * p = new AP_QNXDialog_MergeCells(pFactory,id);
	return p;
}

AP_QNXDialog_MergeCells::AP_QNXDialog_MergeCells(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_MergeCells(pDlgFactory,id)
{
}

AP_QNXDialog_MergeCells::~AP_QNXDialog_MergeCells(void)
{
}

void AP_QNXDialog_MergeCells::runModeless(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them

	// Populate the window's data items
//	_populateWindowData();
//	_connectSignals();
//	abiSetupModelessDialog(GTK_DIALOG(mainWindow),pFrame,this,BUTTON_CLOSE);
	startUpdater();
}

void AP_QNXDialog_MergeCells::setSensitivity(AP_Dialog_MergeCells::mergeWithCell mergeThis, bool bSens)
{

}

void AP_QNXDialog_MergeCells::event_Close(void)
{
	m_answer = AP_Dialog_MergeCells::a_CANCEL;
	destroy();
}

void AP_QNXDialog_MergeCells::destroy(void)
{
	finalize();
}

void AP_QNXDialog_MergeCells::activate(void)
{
	UT_ASSERT (m_windowMain);
        
	ConstructWindowName();
//
// Need code to place title in the frame here
//

	setAllSensitivities();

}

void AP_QNXDialog_MergeCells::notifyActiveFrame(XAP_Frame *pFrame)
{
    UT_ASSERT(m_windowMain);
	ConstructWindowName();
	setAllSensitivities();
}

/*****************************************************************/
