/* AbiSource Program Utilities
 * Copyright (C) 2003-2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
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

#ifndef UT_WIN_32_UUID_H
#define UT_WIN_32_UUID_H

#include "ut_uuid.h"
#include <windows.h>
#include <wincrypt.h>

#if defined(__WINCRYPT_H__) && !defined(WINCRYPT32API)
// hack to fix problem in wincrypt.h;  according to the docs the RSA
// cryptographic provider is included from win95OS2 but the include
// file assumes NT >= 4
//
// (I do not want to just test for _WIN32_WINNT < 0x0400, in case this
// problem was fixed in later versions of MSVC)
//
// we need to provide our own definitions here ...
extern "C" {
typedef unsigned long HCRYPTPROV;

WINADVAPI BOOL WINAPI CryptReleaseContext(HCRYPTPROV hProv, DWORD dwFlags);
WINADVAPI BOOL WINAPI CryptGenRandom(HCRYPTPROV hProv, DWORD dwLen, BYTE *pbBuffer);
WINADVAPI BOOL WINAPI CryptAcquireContextA(HCRYPTPROV *phProv, LPCSTR pszContainer,
										   LPCSTR pszProvider, DWORD dwProvType, DWORD dwFlags);
WINADVAPI BOOL WINAPI CryptAcquireContextW(HCRYPTPROV *phProv, LPCWSTR pszContainer,
										   LPCWSTR pszProvider, DWORD dwProvType, DWORD dwFlags);

#define CRYPT_VERIFYCONTEXT     0xF0000000
#define PROV_RSA_FULL           1

#define CryptAcquireContext  CryptAcquireContextW
} // extern "C"
#endif // #if defined(__WINCRYPT_H__) && !defined(WINCRYPT32API)

/*!
    Class for generating and managing UUIDs

*/
class ABI_EXPORT UT_Win32UUID : public UT_UUID
{
  public:
	/*
	   all constructors are protected; instances of UT_UUID will be
	   created through UT_UUIDGenerator declared below
	*/

	/* virtual destructor*/
	virtual ~UT_Win32UUID ();

  protected:
	friend class UT_Win32UUIDGenerator;
	/* various protected constructors */
	// the first constr. constructs NULL uuid; subsequent call to makeUUID() needed
	UT_Win32UUID():UT_UUID(){s_iInstCount++;}

	UT_Win32UUID(const std::string &s):UT_UUID(s){s_iInstCount++;}
	UT_Win32UUID(const char *s):UT_UUID(s){s_iInstCount++;}
	UT_Win32UUID(const UT_UUID &u):UT_UUID(u){s_iInstCount++;}
	UT_Win32UUID(const UT_Win32UUID &u):UT_UUID((UT_UUID&)u){s_iInstCount++;}
	UT_Win32UUID(const struct uuid &u):UT_UUID(u){s_iInstCount++;}

	virtual bool    _getRandomBytes(void *buf, int nbytes);

  private:
	// had difficulties with including wincrypt.h, so defined this as
	// unsigned long
	static HCRYPTPROV s_hProv;
	static UT_sint32  s_iInstCount;
};

/*
    This class mediates creation of UT_UUID class.

    We create an instance of UT_UUIDGeneratr (or derived) class in
    XAP_App() and have XAP_App::getUUIDGenerator() to gain access to
    it.  This allows us to create platform specific instances of
    UT_UUID from xp code.
*/
class ABI_EXPORT UT_Win32UUIDGenerator : public UT_UUIDGenerator
{
  public:
	UT_Win32UUIDGenerator():UT_UUIDGenerator(){};
	virtual ~UT_Win32UUIDGenerator(){};

	// because the default constructor creates NULL uuid, we also need
	// to call makeUUID() with this one
	virtual UT_UUID * createUUID(){UT_UUID *p=new UT_Win32UUID(); if(p)p->makeUUID(); return p;}

	virtual UT_UUID * createUUID(const std::string &s){return new UT_Win32UUID(s);}
	virtual UT_UUID * createUUID(const char *s){return new UT_Win32UUID(s);}
	virtual UT_UUID * createUUID(const UT_UUID &u){return new UT_Win32UUID(u);}
	virtual UT_UUID * createUUID(const struct uuid &u){return new UT_Win32UUID(u);}
};

#endif /* UT_WIN32_UUID_H */
