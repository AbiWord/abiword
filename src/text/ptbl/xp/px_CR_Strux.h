
#ifndef PX_CHANGERECORD_STRUX_H
#define PX_CHANGERECORD_STRUX_H

#include "ut_types.h"
#include "px_ChangeRecord.h"

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
	PX_ChangeRecord_Strux(PXType type,
						  UT_Bool bMultiStepStart,
						  UT_Bool bMultiStepEnd,
						  PT_DocPosition position,
						  UT_Bool bLeftSide,
						  PT_AttrPropIndex indexAP,
						  PTStruxType struxType);
	~PX_ChangeRecord_Strux();

	PTStruxType				getStruxType(void) const;

protected:
	PTStruxType				m_struxType;	/* our type (paragraph, section) */
};

#endif /* PX_CHANGERECORD_STRUX_H */
