/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2000 Hubert Figuiere <hfiguiere@teaser.fr>
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

#ifndef NDEBUG				// see assert() manpage

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Dialogs.h>
#include <TextUtils.h>
#if defined (TARGET_API_MAC_OSX) && TARGET_API_MAC_OSX
#include <CarbonCore/NumberFormatting.h>
#endif

#include "ap_Mac_ResID.h"
#include "xap_MacApp.h"

#include "ut_assert.h"
#include "ut_MacAssert.h"
#include "ut_MacString.h"

void UT_MacAssertMsg(const char * szMsg, const char * szFile, int iLine)
{
	static int count = 0;
	Str255 msgStr, fileStr, lineStr;
	
	if (XAP_MacApp::m_NotInitialized) {
		::DebugStr ("\pFatal error: toolbox not yet initialized. Can't raise exception.");
	}
	
	C2PStr (msgStr, szMsg);
	C2PStr (fileStr, szFile);
	::NumToString (iLine, lineStr);
	
	::ParamText (msgStr, fileStr, lineStr, NULL);

	while (1)
	{
		short item;
		item = ::Alert (RES_ALRT_ASSERT, NULL);
	
		switch (item)
		{
		case 1:
			return;		// continue the application

		case 2:
			::DebugStr ("\pAssert failed");
			abort();	// kill the application
			return;
		default:
			break;		// ?? ask them again
		}
	}
}

#endif // NDEBUG
