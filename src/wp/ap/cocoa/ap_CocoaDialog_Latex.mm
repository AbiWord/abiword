/* AbiWord
 * Copyright (C) 2005 Martin Sevior
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
#include <glade/glade.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_CocoaDialogHelper.h"
#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_Frame.h"
#include "xap_CocoaWidget.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_CocoaDialog_Latex.h"

XAP_Dialog * AP_CocoaDialog_Latex::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return new AP_CocoaDialog_Latex(pFactory,id);
}

AP_CocoaDialog_Latex::AP_CocoaDialog_Latex(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_Latex(pDlgFactory,id)
{
}

AP_CocoaDialog_Latex::~AP_CocoaDialog_Latex(void)
{
}

void  AP_CocoaDialog_Latex::activate(void)
{
	// FIXME move to XP
	UT_ASSERT (m_windowMain);
	
	ConstructWindowName();
	UT_ASSERT(0);
}

void AP_CocoaDialog_Latex::runModeless(XAP_Frame * pFrame)
{
	constructDialog();
	UT_ASSERT(0);
}


void AP_CocoaDialog_Latex::event_Insert(void)
{
        getLatexFromGUI();
	if(convertLatexToMathML())
	   insertIntoDoc();	
}

void AP_CocoaDialog_Latex::event_Close(void)
{
	m_answer = AP_Dialog_Latex::a_CANCEL;	
	destroy();
}

void AP_CocoaDialog_Latex::notifyActiveFrame(XAP_Frame *pFrame)
{
	ConstructWindowName();

}

void AP_CocoaDialog_Latex::destroy(void)
{
	m_answer = AP_Dialog_Latex::a_CANCEL;	
	modeless_cleanup();
}

void AP_CocoaDialog_Latex::setLatexInGUI(void)
{
	UT_ASSSERT(0);

}

bool AP_CocoaDialog_Latex::getLatexFromGUI(void)
{
	UT_ASSERT(0);
	return false;
}


/*****************************************************************/

void AP_CocoaDialog_Latex::constructDialog(void)
{	

}

