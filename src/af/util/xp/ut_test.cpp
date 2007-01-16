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
 


#include "ut_test.h"
#include "ut_base64.h"

/*****************************************************************/

#ifdef UT_DEBUG
const char * UT_TestStatus_GetMessage(UT_TestStatus status)
{
	switch (status)
	{
	case UT_Test_SystemError:	return "Error";
	case UT_Test_Fail:			return "Fail";
	case UT_Test_Pass:			return "Pass";
	default:					return "Bogus";
	}
}
#endif

/*****************************************************************/

#ifdef UT_TEST
void UT_Test(FILE * fp)
{
	UT_Base64_Test(fp);

	// add other tests here
}
#endif /* UT_TEST */

