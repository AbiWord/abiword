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
#include <CoreServices/CoreServices.h>
#endif

#include "xap_Mac_ResID.h"
#include "xap_MacApp.h"

#include "ut_assert.h"
#include "ut_MacAssert.h"
#include "ut_MacString.h"

Boolean bStopAsking = false;

void UT_MacAssertMsg(const char * szMsg, const char * szFile, int iLine)
{
	static int count = 0;
	Str255 msgStr, fileStr, lineStr;
	
	if (XAP_MacApp::m_NotInitialized) {
		::DebugStr ("\pFatal error: toolbox not yet initialized. Can't raise exception.");
		return;
	}
	
	if( bStopAsking ) {
		// Continue through assertion failures
		return;
	};
	
	C2PStr (msgStr, szMsg);
	C2PStr (fileStr, szFile);
	::NumToString (iLine, lineStr);
	
	::ParamText (msgStr, fileStr, lineStr, NULL);

	while (1)
	{
		short item;
		item = ::StopAlert (RES_ALRT_ASSERT, NULL);
	
		switch (item)
		{
		case 1:
			abort();	// Quit the application
			return;
			
		case 2:
			return;		// Continue the application

		case 3:
			Str255 szBuf;
			sprintf( ( char * )szBuf+1, "[%s %d] Assert failed : %s", szFile, iLine, szMsg );
			szBuf[0] = strlen( ( char * )szBuf+1 );
			::DebugStr ( szBuf );
			return;	    // Debug the application
			
		case 4:
			bStopAsking = true;
			return;		// Continue the application & stop asking

		default:
			break;		// ?? ask them again
		}
	}
}

#endif // NDEBUG
