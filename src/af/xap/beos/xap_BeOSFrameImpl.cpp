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

#include <stdio.h>
#include <string.h>
#include <Roster.h>
#include "ut_types.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "xap_BeOSApp.h"
#include "xap_Frame.h"
#include "xap_BeOSFrameImpl.h"
#include "ev_BeOSKeyboard.h"
#include "ev_BeOSMouse.h"
#include "ev_BeOSMenu.h"
#include "ev_BeOSToolbar.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "xad_Document.h"


#include "ap_FrameData.h"
#include "gr_BeOSGraphics.h"


XAP_BeOSFrameImpl::XAP_BeOSFrameImpl(XAP_Frame *pFrame, XAP_BeOSApp * app)
	: XAP_FrameImpl(pFrame),
	  m_pBeWin(NULL),
	  m_dialogFactory(pFrame, static_cast<XAP_App *>(app))
{
	// pFrame->m_pFrameImpl = this;	
}

XAP_BeOSFrameImpl::~XAP_BeOSFrameImpl(void)
{
}

BWindow * XAP_BeOSFrameImpl::getTopLevelWindow(void) const
{
	return(m_pBeWin);
}


