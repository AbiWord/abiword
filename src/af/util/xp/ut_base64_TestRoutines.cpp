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

#ifdef UT_TEST

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "ut_test.h"
#include "ut_base64.h"

#define NrElements(a)		((sizeof(a) / sizeof(a[0])))

/****************************************************************/
/****************************************************************/

static UT_TestStatus t_test_001(void)
{
	// a test case:  [1] create a buffer of semi-random data with a
	//                   length set to a multiple of 3 bytes.
	//               [2] base64 encode [1].
	//               [3] base64 decode [2].
	//               [4] verify that [1] and [3] exactly match.
	//				 [5] verify that [2] does not end with the pad char.
	
	UT_Byte array1[300];
	UT_uint32 k;

	// [1]
	
	for (k=0; (k<NrElements(array1)); k++)
		array1[k] = (UT_Byte)(k & 0xff);

	UT_ByteBuf b1;
	if (!b1.ins(0,array1,NrElements(array1)))
		return UT_Test_SystemError;
	if (b1.getLength() != NrElements(array1))
		return UT_Test_SystemError;
	
	// [2]

	UT_ByteBuf b2;
	if (!UT_Base64Encode(&b2,&b1))
		return UT_Test_SystemError;

	// [3]

	UT_ByteBuf b3;
	if (!UT_Base64Decode(&b3,&b2))
		return UT_Test_SystemError;

	// [4]

	if (b3.getLength() != b1.getLength())
		return UT_Test_Fail;

	UT_Byte * p1 = b1.getPointer(0);
	UT_Byte * p3 = b3.getPointer(0);
	for (k=0; (k<NrElements(array1)); k++)
		if (p1[k] != p3[k])
			return UT_Test_Fail;

	// [5]

	if (*b2.getPointer(b2.getLength()-1) == '=')
		return UT_Test_Fail;
	
	return UT_Test_Pass;
}

static UT_TestStatus t_test_002(void)
{
	// a test case:  [1] create a buffer of semi-random data with a
	//                   length set to a multiple of 3 + 1 bytes.
	//               [2] base64 encode [1].
	//               [3] base64 decode [2].
	//               [4] verify that [1] and [3] exactly match.
	//               [5] verify that [2] contains exactly 2 pad chars.

	UT_Byte array1[301];
	UT_uint32 k;

	// [1]
	
	for (k=0; (k<NrElements(array1)); k++)
		array1[k] = (UT_Byte)(k & 0xff);

	UT_ByteBuf b1;
	if (!b1.ins(0,array1,NrElements(array1)))
		return UT_Test_SystemError;
	if (b1.getLength() != NrElements(array1))
		return UT_Test_SystemError;
	
	// [2]

	UT_ByteBuf b2;
	if (!UT_Base64Encode(&b2,&b1))
		return UT_Test_SystemError;

	// [3]

	UT_ByteBuf b3;
	if (!UT_Base64Decode(&b3,&b2))
		return UT_Test_SystemError;

	// [4]

	if (b3.getLength() != b1.getLength())
		return UT_Test_Fail;

	UT_Byte * p1 = b1.getPointer(0);
	UT_Byte * p3 = b3.getPointer(0);
	for (k=0; (k<NrElements(array1)); k++)
		if (p1[k] != p3[k])
			return UT_Test_Fail;

	// [5]

	if (   (*b2.getPointer(b2.getLength()-1) != '=')
		|| (*b2.getPointer(b2.getLength()-2) != '='))
		return UT_Test_Fail;
	
	return UT_Test_Pass;
}

static UT_TestStatus t_test_003(void)
{
	// a test case:  [1] create a buffer of semi-random data with a
	//                   length set to a multiple of 3 + 2 bytes.
	//               [2] base64 encode [1].
	//               [3] base64 decode [2].
	//               [4] verify that [1] and [3] exactly match.
	//               [5] verify that [2] contains exactly 1 pad char.

	UT_Byte array1[302];
	UT_uint32 k;

	// [1]
	
	for (k=0; (k<NrElements(array1)); k++)
		array1[k] = (UT_Byte)(k & 0xff);

	UT_ByteBuf b1;
	if (!b1.ins(0,array1,NrElements(array1)))
		return UT_Test_SystemError;
	if (b1.getLength() != NrElements(array1))
		return UT_Test_SystemError;
	
	// [2]

	UT_ByteBuf b2;
	if (!UT_Base64Encode(&b2,&b1))
		return UT_Test_SystemError;

	// [3]

	UT_ByteBuf b3;
	if (!UT_Base64Decode(&b3,&b2))
		return UT_Test_SystemError;

	// [4]

	if (b3.getLength() != b1.getLength())
		return UT_Test_Fail;

	UT_Byte * p1 = b1.getPointer(0);
	UT_Byte * p3 = b3.getPointer(0);
	for (k=0; (k<NrElements(array1)); k++)
		if (p1[k] != p3[k])
			return UT_Test_Fail;

	// [5]

	if (   (*b2.getPointer(b2.getLength()-1) != '=')
		|| (*b2.getPointer(b2.getLength()-2) == '='))
		return UT_Test_Fail;

	return UT_Test_Pass;
}

void UT_Base64_Test(FILE * fp)
{
#define DoTest(t)		#t, UT_TestStatus_GetMessage((t)())
	
	fprintf(fp,"UT_Base64_Test:\n");
	fprintf(fp,"\tTest: %s result %s\n",		DoTest(t_test_001) );
	fprintf(fp,"\tTest: %s result %s\n",		DoTest(t_test_002) );
	fprintf(fp,"\tTest: %s result %s\n",		DoTest(t_test_003) );

	// add other base64 tests here
}
	
#endif /* UT_TEST */
