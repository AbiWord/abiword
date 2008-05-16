/* Copyright (C) 2008 AbiSource Corporation B.V.
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

#include "xap_App.h"
#include "ap_Win32App.h"
#include "xap_Win32App.h"
#include "xap_Frame.h"
#include "xap_Win32DialogHelper.h"
#include "ut_string_class.h"
#include <xp/AbiCollabSessionManager.h>

#include "ap_Win32Dialog_GenericInput.h"

XAP_Dialog * AP_Win32Dialog_GenericInput::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return static_cast<XAP_Dialog *>(new AP_Win32Dialog_GenericInput(pFactory, id));
}
pt2Constructor ap_Dialog_GenericInput_Constructor = &AP_Win32Dialog_GenericInput::static_constructor;

AP_Win32Dialog_GenericInput::AP_Win32Dialog_GenericInput(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_GenericInput(pDlgFactory, id)
{
}

void AP_Win32Dialog_GenericInput::runModal(XAP_Frame * pFrame)
{
	// TODO: implement me
}

void AP_Win32Dialog_GenericInput::event_Ok()
{
	// TODO: imeplement me
}

