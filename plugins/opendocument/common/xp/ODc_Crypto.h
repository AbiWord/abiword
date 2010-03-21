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

#ifndef __ODC_CRYPTO__
#define __ODC_CRYPTO__

#include <string>
#include <gsf/gsf.h>
#include <gsf/gsf-input.h>
#include "ut_types.h"

class ODc_CryptoInfo {
public:
	// stream information
	UT_uint32		m_decryptedSize;
	
	// algorithm information
	std::string		m_algorithm;
	std::string		m_initVector;
	
	// key information
	std::string		m_keyType;
	UT_uint32		m_iterCount;
	std::string		m_salt;
};

class ODc_Crypto {
public:
	static UT_Error decrypt(GsfInput* pStream, const ODc_CryptoInfo& cryptInfo, 
							const std::string& password, GsfInput** pDecryptedInput);

private:
	static UT_Error performDecrypt(GsfInput* pStream,	unsigned char* salt, UT_uint32 salt_length, UT_uint32 iter_count,
							unsigned char* ivec, const std::string& password, UT_uint32 decrypted_size, GsfInput** pDecryptedInput);
};

#endif /* __ODC_CRYPTO__ */