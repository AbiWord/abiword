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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "xap_Dlg_WindowMore.h"
#include "xap_App.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"


XAP_Dialog_WindowMore::XAP_Dialog_WindowMore(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id)
	: AP_Dialog_NonPersistent(pDlgFactory,id)
{
	m_answer = a_OK;
	m_ndxSelFrame = -1;		// nothing selected
}

XAP_Dialog_WindowMore::~XAP_Dialog_WindowMore(void)
{
}

XAP_Dialog_WindowMore::tAnswer XAP_Dialog_WindowMore::getAnswer(void) const
{
	// let our caller know if user hit ok, cancel, etc.
	return m_answer;
}

XAP_Frame * XAP_Dialog_WindowMore::getSelFrame(void) const
{
	XAP_Frame * pSelFrame = NULL;

	UT_ASSERT(m_answer == a_OK);
	UT_ASSERT(m_pApp);

	if (m_pApp && (m_ndxSelFrame >= 0))
	{
		pSelFrame = m_pApp->getFrame(m_ndxSelFrame);
	}

	return pSelFrame;
}
