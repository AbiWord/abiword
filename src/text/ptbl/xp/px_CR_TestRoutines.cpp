 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
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
	
	UT_DEBUGMSG(("CRec: T[%s] [ap %08lx]\n", name,m_indexAP));
}

#endif /* PT_TEST */
