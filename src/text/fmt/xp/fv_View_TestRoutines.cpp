/* AbiWord
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

#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)

#include <stdio.h>
#include <string.h>
#include "ut_test.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xav_View.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "fl_DocLayout.h"

void FV_View::Test_Dump(void)
{
	char buf[100];
	static UT_uint32 x = 0;

	x++;
	
#if defined(PT_TEST)
	sprintf(buf,"dump.ptbl.%d",x);
	FILE * fpDumpPTbl = fopen(buf,"w");
	m_pDoc->__dump(fpDumpPTbl);
	fclose(fpDumpPTbl);
#endif

#if defined(FMT_TEST)
	sprintf(buf,"dump.fmt.%d",x);
	FILE * fpDumpFmt = fopen(buf,"w");
	m_pLayout->__dump(fpDumpFmt);
	fclose(fpDumpFmt);
#endif

#if defined(UT_TEST)
	sprintf(buf,"dump.ut.%d",x);
	FILE * fpDumpUt = fopen(buf,"w");
	UT_Test(fpDumpUt);
	fclose(fpDumpUt);
#endif

}
#endif
