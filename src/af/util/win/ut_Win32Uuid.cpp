/* AbiSource Program Utilities
 * Copyright (C) 2003-2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
 * 
 * Based on libuuid
 * Copyright (C) 1996, 1997, 1998 Theodore Ts'o.
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

#include "ut_Win32Uuid.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

HCRYPTPROV UT_Win32UUID::s_hProv = 0;
UT_sint32  UT_Win32UUID::s_iInstCount = 0;

UT_Win32UUID::~UT_Win32UUID ()
{
	UT_ASSERT( s_iInstCount > 0 );
	s_iInstCount--;
	
	if(s_iInstCount <= 0 && s_hProv)
	{
		CryptReleaseContext(s_hProv,0);		
	}
}


/*
 * Generate a series of random bytes. 
 */
bool UT_Win32UUID::_getRandomBytes(void *buf, UT_sint32 nbytes)
{
	if(s_hProv || CryptAcquireContext(&s_hProv,NULL,NULL,PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		if(CryptGenRandom(s_hProv, nbytes,(BYTE *)buf))
		{
			return true;
		}
	}

#ifdef DEBUG
	LPVOID lpMsgBuf;
	DWORD ecode = GetLastError();
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				  NULL, ecode,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				  (LPWSTR) &lpMsgBuf, 0, NULL);

	UT_DEBUGMSG(("UT_Win32UUID::_getRandomBytes: no Cryptographic services (0x%x; %s)\n",
				 ecode, lpMsgBuf));

	// Free the buffer.
	LocalFree( lpMsgBuf );
#endif
	return UT_UUID::_getRandomBytes(buf, nbytes);
}

