/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
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
#include <stdio.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "pt_Types.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_ContainerObject.h"
#include "fp_TableContainer.h"
#include "fl_TableLayout.h"
#include "fl_BlockLayout.h"
#include "fl_DocLayout.h"
#include "ut_timer.h"

#include "ap_Dialog_Stylist.h"

AP_Dialog_Stylist::AP_Dialog_Stylist(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory,id),
	  m_pAutoUpdater(0)
{
}

AP_Dialog_Stylist::~AP_Dialog_Stylist(void)
{
	stopUpdater();
}

void AP_Dialog_Stylist::setActiveFrame(XAP_Frame *pFrame)
{
	notifyActiveFrame(getActiveFrame());
}

void AP_Dialog_Stylist::startUpdater(void)
{
	m_pAutoUpdater =  UT_Timer::static_constructor(autoUpdate,this, NULL);
	m_pAutoUpdater->set(500);
	m_pAutoUpdater->start();
}

void AP_Dialog_Stylist::stopUpdater(void)
{
	if(m_pAutoUpdater == NULL)
	{
		return;
	}
	m_pAutoUpdater->stop();
	DELETEP(m_pAutoUpdater);
	m_pAutoUpdater = NULL;
}

/*!
 * Autoupdater of the dialog.
 */
void AP_Dialog_Stylist::autoUpdate(UT_Worker * pTimer)
{

	UT_ASSERT(pTimer);
	
// this is a static callback method and does not have a 'this' pointer

	AP_Dialog_Stylist * pDialog = static_cast<AP_Dialog_Stylist *>(pTimer->getInstanceData());
	// Handshaking code
	FV_View * pView = static_cast<FV_View *>(pDialog->getApp()->getLastFocussedFrame()->getCurrentView());
	PD_Document * pDoc = pView->getDocument();
}
