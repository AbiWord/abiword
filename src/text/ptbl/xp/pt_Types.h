 
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


#ifndef PT_TYPES_H
#define PT_TYPES_H

#include "ut_types.h"

typedef UT_uint32 PT_BufIndex;			/* index to actual document data */
typedef UT_uint32 PT_AttrPropIndex;		/* index to Attribute/Property Tables */

typedef UT_uint32 PT_DocPosition;		/* absolute document position */
typedef UT_uint32 PT_BlockOffset;		/* block-relative document position */

typedef enum _PTStruxType { PTX_Section, PTX_ColumnSet, PTX_Column, PTX_Block } PTStruxType;
typedef enum _PTState { PTS_Loading=0, PTS_Editing=1 } PTState;
typedef enum _PTChangeFmt { PTC_AddFmt=0, PTC_RemoveFmt=1 } PTChangeFmt;

typedef UT_uint32 PL_ListenerId;
typedef const void * PL_StruxDocHandle;	/* opaque document data */
typedef const void * PL_StruxFmtHandle;	/* opaque layout data */

#define PT_PROPS_ATTRIBUTE_NAME		((const XML_Char *)"PROPS")

typedef UT_Byte PT_Differences;
#define PT_Diff_Left				((PT_Differences) 0x01)
#define PT_Diff_Right				((PT_Differences) 0x02)

#endif /* PT_TYPES_H */
