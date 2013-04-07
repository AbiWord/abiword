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
 * Contains decryption routines for StarOffice files */

#include <string.h> // for memcpy
#include "ut_types.h"
#include "ut_string_class.h"

/** Decryptor for .sdw files */
class SDWCryptor {
	public:
		/** Maximum length of the password */
		enum { maxPWLen = 16 };
		/** Intializes the cryptor. The arguments are used to verify
		 * that the password is correct. A date and time of zero tells
		 * the cryptor to not use these values.
		 * If you give aDate and aTime, you really should also give aFilePass -
		 * results might not be what you expect otherwise
		 * @param aDate The date given in the file header
		 * @param aTime Time from file header. */
		SDWCryptor(UT_uint32 aDate = 0, UT_uint32 aTime = 0, const UT_uint8* aFilePass = NULL);
		~SDWCryptor();
		/** Sets date and time for verifying the password */
		void SetDateTime(UT_uint32 aDate, UT_uint32 aTime) { mDate = aDate; mTime = aTime; }
		/** Sets the password that will be used for decrypting. Even if the password
		 * is invalid, the current password will be modified.
		 * @param aPassword The password to use
		 * @param aFilePass The password given in the file header to
		 * check if the given password is correct or NULL if no check should be
		 * performed.
		 * @return true on success (also if aFilePass is NULL),
		 *         false on failure (e.g. invalid password) */
		bool SetPassword(const char* aPassword);

		/** Decrypts a given string.
		 * @param aEncrypted The string to decrypt
		 * @param aBuffer Decrypted string will be put here. Needs to be at least
		 * strlen(aEncrypted) bytes large. Can be the same as aEncrypted.
		 * @param aLen Length of the string to be encrypted, if 0 or not given
		 * strlen(aEncrypted) is used. */
		void Decrypt(const char* aEncrypted, char* aBuffer, UT_uint32 aLen = 0) const;

		/** Encrypts a string. Works the same as SDWCryptor::Decrypt */
		void Encrypt(const char* aDecrypted, char* aBuffer, UT_uint32 aLen = 0) const { Decrypt(aDecrypted, aBuffer, aLen); }
	protected:
		UT_uint32 mDate;
		UT_uint32 mTime;
		char mPassword[maxPWLen];
		UT_uint8 mFilePass[maxPWLen];
};


