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



#ifndef PT_TYPES_H
#define PT_TYPES_H

#include "ut_types.h"

typedef UT_uint32 PT_BufIndex;			/* index to actual document data */
typedef UT_uint32 PT_AttrPropIndex;		/* index to Attribute/Property Tables */

typedef UT_uint32 PT_DocPosition;		/* absolute document position */
typedef UT_uint32 PT_BlockOffset;		/* block-relative document position */

// PTStruxType tells the sub-type of a FragStrux.
// PTObjectType tells the sub-type of an FragObject.

typedef enum _PTStruxType { PTX_Section, PTX_Block } PTStruxType;
typedef enum _PTObjectType { PTO_Image, PTO_Field } PTObjectType;

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
