/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
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
 
#include "ut_string.h"
#include "ut_stringbuf.h"
#include "ut_base64.h"

/*****************************************************************/
/*****************************************************************/

// See RFC1521 for details.

static UT_Byte s_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static UT_Byte s_pad = '=';

static UT_Byte s_inverse(UT_Byte b)
{
	if ((b >= 'A') && (b <= 'Z')) return (     b - 'A');
	if ((b >= 'a') && (b <= 'z')) return (26 + b - 'a');
	if ((b >= '0') && (b <= '9')) return (52 + b - '0');
	if (b == '+') return 62;
	if (b == '/') return 63;

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}

/*****************************************************************/
/*****************************************************************/

bool UT_Base64Encode(UT_ByteBuf * pDest, const UT_ByteBuf * pSrc)
{
	// Base64 encode the raw (presumed binary) data in pSrc into pDest.
	// return false if error.

	UT_ASSERT(pDest && pSrc);

	pDest->truncate(0);
	
	UT_uint32 lenSrc = pSrc->getLength();
	if (lenSrc == 0)						// empty source buffer yields empty output buffer.
		return true;

	// compute the amount of space needed in the destination and reserve space for it in advance.
	
	UT_uint32 lenDest = (lenSrc + 2) / 3 * 4;
	if (!pDest->ins(0,lenDest))
		return false;

	UT_uint32 kSrc, kDest;
	const UT_Byte * p = pSrc->getPointer(0);
	for (kSrc=0, kDest=0; (kSrc < lenSrc); kSrc+=3, kDest+=4)
	{
		// encode each group of 3 bytes in the input into 4 bytes in the output.

		UT_Byte s1 = p[kSrc];
		UT_Byte s2 = (kSrc+1 < lenSrc) ? p[kSrc+1] : 0;
		UT_Byte s3 = (kSrc+2 < lenSrc) ? p[kSrc+2] : 0;

		UT_uint32 d = (s1<<16) | (s2<<8) | s3;
		
		UT_Byte dd[4];
		dd[0] =                     s_alphabet[( (d>>18)        )];
		dd[1] =                     s_alphabet[( (d>>12) & 0x3f )];
		dd[2] = (kSrc+1 < lenSrc) ? s_alphabet[( (d>> 6) & 0x3f )] : s_pad;
		dd[3] = (kSrc+2 < lenSrc) ? s_alphabet[( (d    ) & 0x3f )] : s_pad;

		pDest->overwrite(kDest,dd,4);
	}

	return true;
}

bool UT_Base64Decode(UT_ByteBuf * pDest, const UT_ByteBuf * pSrc)
{
	// decode the Base64 data in pSrc into pDest.
	// return false if error.

	UT_ASSERT(pDest && pSrc);

	pDest->truncate(0);

	UT_uint32 lenSrc = pSrc->getLength();
	if (lenSrc == 0)						// empty source buffer yields empty output buffer.
		return true;

	UT_ASSERT((lenSrc % 4) == 0);			// encoded data must consists of quads

	// compute the destination length and deal with pad bytes
	
	UT_uint32 lenDest = ((lenSrc + 3) / 4) * 3;
	const UT_Byte * p = pSrc->getPointer(0);
	if (p[lenSrc-1] == s_pad)
	{
		lenDest--;
		lenSrc--;
		if (p[lenSrc-1] == s_pad)
		{
			lenDest--;
			lenSrc--;
		}
	}

	// reserve space in the destination in advance.
	
	if (!pDest->ins(0,lenDest))
		return false;

	UT_uint32 kSrc, kDest;

	for (kSrc=0, kDest=0; (kSrc < lenSrc); kSrc+=4, kDest+=3)
	{
		// decode each group of 4 bytes in the input into 3 bytes in the output.

		bool bHave2 = ((kSrc+2) < lenSrc);
		bool bHave3 = ((kSrc+3) < lenSrc);
		
		UT_Byte s1 =            s_inverse(p[kSrc  ]);
		UT_Byte s2 =            s_inverse(p[kSrc+1]);
		UT_Byte s3 = (bHave2) ? s_inverse(p[kSrc+2]) : 0;
		UT_Byte s4 = (bHave3) ? s_inverse(p[kSrc+3]) : 0;

		UT_uint32 d = (s1<<18) | (s2<<12) | (s3<<6) | s4;

		UT_Byte dd[3];
		dd[0] = (UT_Byte)(           ( (d>>16) & 0xff )    );
		dd[1] = (UT_Byte)((bHave2) ? ( (d>> 8) & 0xff ) : 0);
		dd[2] = (UT_Byte)((bHave3) ? ( (d    ) & 0xff ) : 0);

		pDest->overwrite(kDest,dd, (1 + bHave2 + bHave3));
	}

	return true;
}

/* Copyright (C) 2002 Francis James Franklin <fjf@alinameridon.com>
 * Copyright (C) 2002 AbiSource, Inc.
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

static const char s_UTF8_B64Alphabet[64] = {
		  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, // A-Z
		  0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, // a-z
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // 0-9
	0x2b, // +
	0x2f  // /
};
static const char s_UTF8_B64Pad = 0x3d;

static bool s_UTF8_B64Decode (char c, int & b64)
{
	if ((c >= 0x41) && (c <= 0x5a))
		{
			b64 = c - 0x41;
			return true;
		}
	if ((c >= 0x61) && (c <= 0x7a))
		{
			b64 = c - (0x61 - 26);
			return true;
		}
	if ((c >= 0x30) && (c <= 0x39))
		{
			b64 = c + (52 - 0x30);
			return true;
		}
	if (c == 0x2b)
		{
			b64 = 62;
			return true;
		}
	if (c == 0x2f)
		{
			b64 = 63;
			return true;
		}
	return false;
}

bool ABI_EXPORT UT_UTF8_Base64Encode(char *& b64ptr, size_t & b64len, const char *& binptr, size_t & binlen)
{
	while ((binlen >= 3) && (b64len >= 4))
		{
			unsigned char u1 = static_cast<unsigned char>(*binptr++);
			*b64ptr++ = s_UTF8_B64Alphabet[u1>>2];
			unsigned char u2 = static_cast<unsigned char>(*binptr++);
			u1 = ((u1 & 0x03) << 4) | (u2 >> 4);
			*b64ptr++ = s_UTF8_B64Alphabet[u1];
			u2 = (u2 & 0x0f) << 2;
			unsigned char u3 = static_cast<unsigned char>(*binptr++);
			*b64ptr++ = s_UTF8_B64Alphabet[u2 | (u3 >> 6)];
			*b64ptr++ = s_UTF8_B64Alphabet[u3 & 0x3f];
			b64len -= 4;
			binlen -= 3;
		}
	if (binlen >= 3) return false; // huh?
	if (binlen == 0) return true;

	if (b64len < 4) return false; // huh?

	if (binlen == 2)
		{
			unsigned char u1 = static_cast<unsigned char>(*binptr++);
			*b64ptr++ = s_UTF8_B64Alphabet[u1>>2];
			unsigned char u2 = static_cast<unsigned char>(*binptr++);
			u1 = ((u1 & 0x03) << 4) | (u2 >> 4);
			*b64ptr++ = s_UTF8_B64Alphabet[u1];
			u2 = (u2 & 0x0f) << 2;
			*b64ptr++ = s_UTF8_B64Alphabet[u2];
			*b64ptr++ = s_UTF8_B64Pad;
			b64len -= 4;
			binlen -= 2;
		}
	else // if (binlen == 1)
		{
			unsigned char u1 = static_cast<unsigned char>(*binptr++);
			*b64ptr++ = s_UTF8_B64Alphabet[u1>>2];
			u1 = (u1 & 0x03) << 4;
			*b64ptr++ = s_UTF8_B64Alphabet[u1];
			*b64ptr++ = s_UTF8_B64Pad;
			*b64ptr++ = s_UTF8_B64Pad;
			b64len -= 4;
			binlen -= 1;
		}
	return true;
}

bool ABI_EXPORT UT_UTF8_Base64Decode(char *& binptr, size_t & binlen, const char *& b64ptr, size_t & b64len)
{
	if (b64len == 0) return true; // ??

	if ((binptr == 0) || (b64ptr == 0)) return false;

	bool decoded = true;
	bool padding = false;

	int i = 0;

	unsigned char byte1;
	unsigned char byte2;

	UT_UCSChar ucs2;
	while (ucs2 = UT_UCS2Stringbuf::UTF8_to_UCS2 (b64ptr, b64len))
		{
			if ((ucs2 & 0x7f) == ucs2)
				{
					int b64;
					char c = static_cast<char>(ucs2);
					if (s_UTF8_B64Decode (c, b64))
						{
							if (padding || (binlen == 0))
								{
									decoded = false;
									break;
								}
							switch (i)
								{
								case 0:
									byte1 = static_cast<unsigned char>(b64) << 2;
									i++;
									break;
								case 1:
									byte2 = static_cast<unsigned char>(b64);
									byte1 |= byte2 >> 4;
									*binptr++ = static_cast<char>(byte1);
									binlen--;
									byte1 = (byte2 & 0x0f) << 4;
									i++;
									break;
								case 2:
									byte2 = static_cast<unsigned char>(b64);
									byte1 |= byte2 >> 2;
									*binptr++ = static_cast<char>(byte1);
									binlen--;
									byte1 = (byte2 & 0x03) << 6;
									i++;
									break;
								default:
									byte1 |= static_cast<unsigned char>(b64);
									*binptr++ = static_cast<char>(byte1);
									binlen--;
									i = 0;
									break;
								}
							if (!decoded) break;
							continue;
						}
					else if (c == s_UTF8_B64Pad)
						{
							switch (i)
								{
								case 0:
								case 1:
									decoded = false;
									break;
								case 2:
									if (binlen == 0) decoded = false;
									else
										{
											*binptr++ = static_cast<char>(byte1);
											binlen--;
											padding = true;
										}
									i++;
									break;
								default:
									if (!padding)
										{
											if (binlen == 0) decoded = false;
											else
												{
													*binptr++ = static_cast<char>(byte1);
													binlen--;
													padding = true;
												}
										}
									i = 0;
									break;
								}
							if (!decoded) break;
							continue;
						}
				}
			if (UT_UCS_isspace (ucs2)) continue;

			decoded = false;
			break;
		}
	return decoded;
}
