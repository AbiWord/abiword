/* Copyright (C) 2010 Marc Maurer <uwog@uwog.net>
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

#include <stdlib.h>

#include <zlib.h>
#include <glib.h>
#include <gsf/gsf-input-gzip.h>
#include <gsf/gsf-input-memory.h>

#include "sha1.h"
#include "gc-pbkdf2-sha1.h"

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ODc_Crypto.h"

#include "config.h"
#ifdef HAVE_GCRYPT
#include "gcrypt.h"
#endif

#define PASSWORD_HASH_LEN 20
#define PBKDF2_KEYLEN 16

#define HANDLEGERR(e)                           \
    {                                           \
    gcry_err_code_t code = gcry_err_code(e);    \
    if( code != GPG_ERR_NO_ERROR )              \
    {                                           \
        switch(code)                            \
        {                                       \
            case GPG_ERR_ENOMEM:                \
               return UT_OUTOFMEM;             \
            case GPG_ERR_DECRYPT_FAILED:        \
                return UT_IE_PROTECTED;         \
            default:                            \
                return UT_ERROR;                \
        }                                       \
    }                                           \
    }



UT_Error ODc_Crypto::performDecrypt( GsfInput* pStream, 
                                     unsigned char* salt, UT_uint32 salt_length, UT_uint32 iter_count,
                                     unsigned char* ivec, gsize ivec_length,
                                     const std::string& password, UT_uint32 decrypted_size,
                                     GsfInput** pDecryptedInput)
{
	unsigned char sha1_password[PASSWORD_HASH_LEN];
	char key[PBKDF2_KEYLEN];

	// get the sha1 sum of the password
	sha1_buffer(&password[0], password.size(), sha1_password);

	// create a PBKDF2 key from the sha1 sum
	int k = pbkdf2_sha1 ((const char*)sha1_password, PASSWORD_HASH_LEN, (const char*)salt, salt_length, iter_count, key, PBKDF2_KEYLEN);
	if (k != 0)
		return UT_ERROR;

    // Get the encrypted content ready
	UT_sint32 content_size = gsf_input_size(pStream); 
	if (content_size == -1)
		return UT_ERROR;
	const unsigned char* content = gsf_input_read(pStream, content_size, NULL);
	if (!content)
		return UT_ERROR;

    unsigned char* content_decrypted = (unsigned char*)g_malloc(content_size);

#ifdef HAVE_GCRYPT

    UT_DEBUGMSG(("ODc_Crypto::performDecrypt() using gcrypt\n" ));
 
    gcry_cipher_hd_t h;
    HANDLEGERR( gcry_cipher_open( &h,
                                  GCRY_CIPHER_BLOWFISH,
                                  GCRY_CIPHER_MODE_CFB,
                                  0 ));
    HANDLEGERR( gcry_cipher_setkey( h, key, PBKDF2_KEYLEN ));
    HANDLEGERR( gcry_cipher_setiv ( h, ivec, ivec_length ));
    HANDLEGERR( gcry_cipher_decrypt( h,
                                     content_decrypted,
                                     content_size,
                                     content,
                                     content_size ));
    gcry_cipher_close( h );
    
    
#else

    // removed old blowfish code
    
#endif
    

	// deflate the decrypted content
	z_stream zs;
	zs.zalloc = Z_NULL;
	zs.zfree = Z_NULL;
	zs.opaque = Z_NULL;
	zs.avail_in = 0;
	zs.next_in = Z_NULL;

	int err;
	err = inflateInit2(&zs, -MAX_WBITS);
	if (err != Z_OK)
		return UT_ERROR;

	unsigned char* decrypted = (unsigned char*)g_malloc(decrypted_size);
	zs.avail_in = content_size;
	zs.avail_out = decrypted_size;
	zs.next_in = content_decrypted;
	zs.next_out = decrypted;

	err = inflate(&zs, Z_FINISH);
	FREEP(content_decrypted);
	
	if (err != Z_STREAM_END)
	{
		inflateEnd(&zs);
		FREEP(decrypted);
		return UT_ERROR;
	}

	inflateEnd(&zs);

	*pDecryptedInput = gsf_input_memory_new(decrypted, decrypted_size, TRUE);
	
	return UT_OK;
}

UT_Error ODc_Crypto::decrypt(GsfInput* pStream, const ODc_CryptoInfo& cryptInfo,
    const std::string& password, GsfInput** pDecryptedInput)
{
	UT_return_val_if_fail(pStream, UT_ERROR);
	UT_return_val_if_fail(pDecryptedInput, UT_ERROR);
	
	// check if we support the requested decryption method
	UT_return_val_if_fail(g_ascii_strcasecmp(cryptInfo.m_algorithm.c_str(), "Blowfish CFB") == 0, UT_ERROR);
	UT_return_val_if_fail(g_ascii_strcasecmp(cryptInfo.m_keyType.c_str(), "PBKDF2") == 0, UT_ERROR);
	
	// base64 decode the salt
	gsize salt_length;
	unsigned char* salt = g_base64_decode(cryptInfo.m_salt.c_str(), &salt_length);

	// base64 decode the initialization vector
	gsize ivec_length;
	unsigned char* ivec = g_base64_decode(cryptInfo.m_initVector.c_str(), &ivec_length);

	// decrypt the content
	UT_Error result = performDecrypt(pStream, salt, salt_length, cryptInfo.m_iterCount,  
                                     ivec, ivec_length, password, cryptInfo.m_decryptedSize, pDecryptedInput);

	// cleanup
	FREEP(salt);
	FREEP(ivec);

	return result;
}
