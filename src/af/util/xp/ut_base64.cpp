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
	return UT_FALSE;
}

/*****************************************************************/
/*****************************************************************/

UT_Bool UT_Base64Encode(UT_ByteBuf * pDest, const UT_ByteBuf * pSrc)
{
	// Base64 encode the raw (presumed binary) data in pSrc into pDest.
	// return false if error.

	UT_ASSERT(pDest && pSrc);

	pDest->truncate(0);
	
	UT_uint32 lenSrc = pSrc->getLength();
	if (lenSrc == 0)						// empty source buffer yields empty output buffer.
		return UT_TRUE;

	// compute the amount of space needed in the destination and reserve space for it in advance.
	
	UT_uint32 lenDest = (lenSrc + 2) / 3 * 4;
	if (!pDest->ins(0,lenDest))
		return UT_FALSE;

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

	return UT_TRUE;
}

UT_Bool UT_Base64Decode(UT_ByteBuf * pDest, const UT_ByteBuf * pSrc)
{
	// decode the Base64 data in pSrc into pDest.
	// return false if error.

	UT_ASSERT(pDest && pSrc);

	pDest->truncate(0);

	UT_uint32 lenSrc = pSrc->getLength();
	if (lenSrc == 0)						// empty source buffer yields empty output buffer.
		return UT_TRUE;

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
		return UT_FALSE;

	UT_uint32 kSrc, kDest;

	for (kSrc=0, kDest=0; (kSrc < lenSrc); kSrc+=4, kDest+=3)
	{
		// decode each group of 4 bytes in the input into 3 bytes in the output.

		UT_Bool bHave2 = ((kSrc+2) < lenSrc);
		UT_Bool bHave3 = ((kSrc+3) < lenSrc);
		
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

	return UT_TRUE;
}

	
