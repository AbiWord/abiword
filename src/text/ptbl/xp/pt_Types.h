

#ifndef PT_TYPES_H
#define PT_TYPES_H

#include "ut_types.h"

typedef UT_uint32 PT_BufIndex;			/* index to actual document data */
typedef UT_uint32 PT_AttrPropIndex;		/* index to Attribute/Property Tables */

typedef UT_uint32 PT_DocPosition;		/* absolute document position */
typedef UT_uint32 PT_BlockOffset;		/* block-relative document position */

typedef enum _PTStruxType { PTX_Section, PTX_ColumnSet, PTX_Column, PTX_Block } PTStruxType;
typedef enum _PTState { PTS_Loading=0, PTS_Editing=1 } PTState;

typedef UT_uint32 PL_ListenerId;
typedef const void * PL_StruxDocHandle;	/* opaque document data */
typedef const void * PL_StruxFmtHandle;	/* opaque layout data */

#endif /* PT_TYPES_H */
