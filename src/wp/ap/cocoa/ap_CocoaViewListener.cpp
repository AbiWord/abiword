/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */



#include <stdio.h>
#include <stdlib.h>

#include <Cocoa/Cocoa.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xav_Listener.h"
#include "xav_View.h"
#include "ap_CocoaViewListener.h"
#include "ap_CocoaApp.h"

ap_CocoaViewListener::ap_CocoaViewListener(XAP_Frame * pFrame)
	: ap_ViewListener(pFrame)
{
}

bool ap_CocoaViewListener::notify(AV_View * pView, const AV_ChangeMask mask)
{
	UT_ASSERT(pView);
	
	if (mask & AV_CHG_EMPTYSEL)
	{
		AP_CocoaApp * pCocoaApp = static_cast<AP_CocoaApp *>(pView->getApp());
		pCocoaApp->setSelectionStatus(pView);
	}

	return ap_ViewListener::notify(pView,mask);
}

