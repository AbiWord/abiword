
#ifndef PX_CHANGERECORD_FMT_H
#define PX_CHANGERECORD_FMT_H

#include "ut_types.h"

// PX_ChangeRecord_Fmt describes an insertFmt or deleteFmt
// change made to the document.
// This description should be sufficient to allow undo to
// work and sufficient to allow the formatter to do a
// partial format and screen update (if appropriate).
// The change record must be free of pointers, since it
// represents what was done to the document -- and not
// how it was done (that is, not what was done to various
// intermediate data structures).  this also lets it be
// cached to disk (for autosave and maybe multi-session
// undo).

class PX_ChangeRecord_Fmt : public PX_ChangeRecord
{
public:
	PX_ChangeRecord_Fmt();
	~PX_ChangeRecord_Fmt();
	
protected:
	PT_DocPosition			m_position2;	/* absolute document position of end of the change */
	pt_AttrPropIndex		m_index;		/* index in VS[].m_tableAttrProp to our A/P */
};

#endif /* PX_CHANGERECORD_FMT_H */
