

#ifndef PT_TYPES_H
#define PT_TYPES_H

// TODO check and fix the prefix case on these 2.
typedef UT_uint32 pt_BufPosition;		/* offset in one of the VarSet buffers */
typedef UT_uint32 pt_AttrPropIndex;		/* index in one of the VarSet AP Tables */

typedef UT_uint32 PL_ListenerId;
typedef const void * PL_StruxDocHandle;	/* opaque document data */
typedef const void * PL_StruxFmtHandle;	/* opaque layout data */

#endif /* PT_TYPES_H */
