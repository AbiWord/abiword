
#ifndef PX_CHANGERECORD_STRUX_H
#define PX_CHANGERECORD_STRUX_H

#include "ut_types.h"

// PX_ChangeRecord_Strux describes an insertStrux or
// deleteStrux change made to the document.
// This description should be sufficient to allow undo to
// work and sufficient to allow the formatter to do a
// partial format and screen update (if appropriate).
// The change record must be free of pointers, since it
// represents what was done to the document -- and not
// how it was done (that is, not what was done to various
// intermediate data structures).  this also lets it be
// cached to disk (for autosave and maybe multi-session
// undo).
//


class PX_ChangeRecord_Strux : public PX_ChangeRecord
{
public:
	PX_ChangeRecord_Strux();
	~PX_ChangeRecord_Strux();
	
protected:
	UT_Bool					m_bLeftSide;
	pt_AttrPropIndex		m_index;	/* index in VS[].m_tableAttrProp to our A/P */
	PT_PieceTable::PTStrux	m_type;		/* our type (paragraph, section) */
};

#endif /* PX_CHANGERECORD_STRUX_H */
