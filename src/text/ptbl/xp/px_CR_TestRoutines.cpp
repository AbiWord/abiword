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


#ifdef PT_TEST

#include "ut_types.h"
#include "ut_test.h"
#include "ut_debugmsg.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"

/*****************************************************************/
/*****************************************************************/

void PX_ChangeRecord::__dump(void) const
{
	static const char * name = "????????";
	
	switch (m_type)
	{
	case PX_ChangeRecord::PXT_GlobMarker:				name = "GlobGlob";	break;
	case PX_ChangeRecord::PXT_InsertSpan:				name = "InstSpan";	break;
	case PX_ChangeRecord::PXT_DeleteSpan:				name = "DeleSpan";	break;
	case PX_ChangeRecord::PXT_ChangeSpan:				name = "ChngSpan";	break;
	case PX_ChangeRecord::PXT_InsertStrux:				name = "InstStrx";	break;
	case PX_ChangeRecord::PXT_DeleteStrux:				name = "DeleStrx";	break;
	case PX_ChangeRecord::PXT_ChangeStrux:				name = "ChngStrx";	break;
	case PX_ChangeRecord::PXT_TempSpanFmt:				name = "TempSFmt";	break;
	}
	
	UT_DEBUGMSG(("CRec: T[%s] [ap %p]\n", name,m_indexAP));
}

#endif /* PT_TEST */
