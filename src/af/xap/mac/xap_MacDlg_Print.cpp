/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_Dialog_Id.h"
#include "xap_MacDlg_Print.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"
#ifndef XP_MAC_TARGET_QUARTZ
# include "gr_MacQDGraphics.h"
#else
# include "gr_MacGraphics.h"
#endif

/*****************************************************************/
XAP_Dialog * XAP_MacDialog_Print::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_MacDialog_Print * p = new XAP_MacDialog_Print(pFactory,id);
	return p;
}

XAP_MacDialog_Print::XAP_MacDialog_Print(XAP_DialogFactory * pDlgFactory,
										   XAP_Dialog_Id id)
	: XAP_Dialog_Print(pDlgFactory,id)
{
}

XAP_MacDialog_Print::~XAP_MacDialog_Print(void)
{
}

GR_Graphics * XAP_MacDialog_Print::getPrinterGraphicsContext(void)
{
	return 0;
}

void XAP_MacDialog_Print::releasePrinterGraphicsContext(GR_Graphics * /*pGraphics*/)
{
}

/*****************************************************************/

void XAP_MacDialog_Print::runModal(XAP_Frame * /*pFrame*/)
{
	m_pMacFrame = NULL;
	return;
}

void XAP_MacDialog_Print::_extractResults(void)
{
	m_answer = a_OK;
	return;

Fail:
	m_answer = a_CANCEL;
	return;
}

