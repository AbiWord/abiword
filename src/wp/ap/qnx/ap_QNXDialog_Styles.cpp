/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Field.h"
#include "ap_QNXDialog_Field.h"
#include "ap_QNXDialog_Styles.h"
#include "ut_qnxHelper.h"


/*****************************************************************/

XAP_Dialog * AP_QNXDialog_Styles::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_QNXDialog_Styles * p = new AP_QNXDialog_Styles(pFactory,id);
	return p;
}

AP_QNXDialog_Styles::AP_QNXDialog_Styles(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_Styles(pDlgFactory,id)
{
}

AP_QNXDialog_Styles::~AP_QNXDialog_Styles(void)
{
}

void AP_QNXDialog_Styles::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

/*
	NOTE: This template can be used to create a working stub for a 
	new dialog on this platform.  To do so:
	
	1.  Copy this file (and its associated header file) and rename 
		them accordingly. 

	2.  Do a case sensitive global replace on the words Stub and STUB
		in both files. 

	3.  Add stubs for any required methods expected by the XP class. 
		If the build fails because you didn't do this step properly,
		you've just broken the donut rule.  

	4.	Replace this useless comment with specific instructions to 
		whoever's porting your dialog so they know what to do.
		Skipping this step may not cost you any donuts, but it's 
		rude.  

	This file should *only* be used for stubbing out platforms which 
	you don't know how to implement.  When implementing a new dialog 
	for your platform, you're probably better off starting with code
	from another working dialog.  
*/	

	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void AP_QNXDialog_Styles::setDescription(const char * desc) const
{
#if 0
  UT_ASSERT(m_wlabelDesc);
  gtk_label_set_text (GTK_LABEL(m_wlabelDesc), desc);
#endif
}

const char * AP_QNXDialog_Styles::getCurrentStyle (void) const
{
#if 0
  static UT_String szStyleBuf;

  UT_ASSERT(m_wclistStyles);

  if (m_whichRow < 0 || m_whichCol < 0)
    return NULL;

  char * szStyle = NULL;

  int ret = gtk_clist_get_text (GTK_CLIST(m_wclistStyles), 
				m_whichRow, m_whichCol, &szStyle);

  if (!ret)
    return NULL;

  szStyleBuf = szStyle;
  return szStyleBuf.c_str();
#else
	return NULL;
#endif
}
