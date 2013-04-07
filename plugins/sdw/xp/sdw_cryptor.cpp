/* Abiword
 * Copyright (C) 2002 Christian Biesinger <cbiesinger@web.de>
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

/** @file
 * implementation of staroffice decryption routines, very much inspired by
 * openoffice's sw/source/core/sw3io/sw3imp.cxx#L2721 and sw/source/core/sw3io/crypter.cxx#L77 */

#include "sdw_cryptor.h"
#include "ut_debugmsg.h"
#include "ut_string_class.h"

// random values for encrypting the password
static const UT_uint8 gEncode[] =
{ 0xab, 0x9e, 0x43, 0x05, 0x38, 0x12, 0x4d, 0x44,
  0xd5, 0x7e, 0xe3, 0x84, 0x98, 0x23, 0x3f, 0xba };

SDWCryptor::SDWCryptor(UT_uint32 aDate, UT_uint32 aTime, const UT_uint8* aFilePass)
: mDate(aDate), mTime(aTime) {
	if (aFilePass)
		memcpy(mFilePass, aFilePass, maxPWLen);
	else
		memset(mFilePass, 0, maxPWLen);
}

SDWCryptor::~SDWCryptor() {
}

bool SDWCryptor::SetPassword(const char* aPassword) {
	// Set the new password
	char pw[maxPWLen];
	strncpy(pw, aPassword, maxPWLen);
	size_t len = strlen(aPassword);
	// fill with spaces. only executed if len < maxPWLen
	for (int i = len; i < maxPWLen; i++)
		pw[i] = ' ';

	// the password needs to be encrypted
	memcpy(mPassword, gEncode, maxPWLen);
	Encrypt(pw, mPassword, maxPWLen);

	// Check password if we have valid date and/or time
	if (mDate || mTime) {
		char testString[17];
		UT_String needle = UT_String_sprintf("%08x%08x", mDate, mTime);
		Encrypt(needle.c_str(), testString, 16);
		if (memcmp(testString, mFilePass, 16) != 0) {
			return false; // wrong password
		}
	}
	return true;
}

// almost literally taken from openoffice code (crypter.cxx)
void SDWCryptor::Decrypt(const char* aEncrypted, char* aBuffer, UT_uint32 aLen) const {
        size_t nCryptPtr = 0;
        UT_uint8 cBuf[maxPWLen];
        memcpy(cBuf, mPassword, maxPWLen);
        UT_uint8* p = cBuf;

	if (!aLen)
		aLen = strlen(aEncrypted);

        while (aLen--) {
                *aBuffer++ = *aEncrypted++ ^ ( *p ^ static_cast<UT_uint8> ( cBuf[ 0 ] * nCryptPtr ) );
                *p += ( nCryptPtr < (maxPWLen-1) ) ? *(p+1) : cBuf[ 0 ];
                if( !*p ) *p += 1;
                p++;
                if( ++nCryptPtr >= maxPWLen ) {
			nCryptPtr = 0;
			p = cBuf;
		}
        }
}
