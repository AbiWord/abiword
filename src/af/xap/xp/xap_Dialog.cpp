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

#include "xap_Dialog.h"
#include "ut_assert.h"
#include "xap_DialogFactory.h"

/*****************************************************************/

AP_Dialog::AP_Dialog(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id)
{
	UT_ASSERT(pDlgFactory);

	m_pDlgFactory = pDlgFactory;
	m_id = id;
	m_pApp = pDlgFactory->getApp();

	UT_ASSERT(m_pApp);
}

AP_Dialog::~AP_Dialog(void)
{
}

AP_Dialog_Id AP_Dialog::getDialogId(void) const
{
	return m_id;
}

/*****************************************************************/

AP_Dialog_NonPersistent::AP_Dialog_NonPersistent(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id)
	: AP_Dialog(pDlgFactory,id)
{
}

AP_Dialog_NonPersistent::~AP_Dialog_NonPersistent(void)
{
}

/*****************************************************************/

AP_Dialog_Persistent::AP_Dialog_Persistent(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id)
	: AP_Dialog(pDlgFactory,id)
{
	m_bInUse = UT_FALSE;
}

AP_Dialog_Persistent::~AP_Dialog_Persistent(void)
{
}

void AP_Dialog_Persistent::useStart(void)
{
	UT_ASSERT(!m_bInUse);
	m_bInUse = UT_TRUE;
}

void AP_Dialog_Persistent::useEnd(void)
{
	UT_ASSERT(m_bInUse);
	m_bInUse = UT_FALSE;
}

/*****************************************************************/

AP_Dialog_FramePersistent::AP_Dialog_FramePersistent(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id)
	: AP_Dialog_Persistent(pDlgFactory,id)
{
}

AP_Dialog_FramePersistent::~AP_Dialog_FramePersistent(void)
{
}

void AP_Dialog_FramePersistent::useStart(void)
{
	AP_Dialog_Persistent::useStart();
}

void AP_Dialog_FramePersistent::useEnd(void)
{
	AP_Dialog_Persistent::useEnd();
}

/*****************************************************************/

AP_Dialog_AppPersistent::AP_Dialog_AppPersistent(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id)
	: AP_Dialog_Persistent(pDlgFactory,id)
{
}

AP_Dialog_AppPersistent::~AP_Dialog_AppPersistent(void)
{
}

void AP_Dialog_AppPersistent::useStart(void)
{
	AP_Dialog_Persistent::useStart();
}

void AP_Dialog_AppPersistent::useEnd(void)
{
	AP_Dialog_Persistent::useEnd();
}
