
#ifndef PX_CHANGERECORD_SPAN_H
#define PX_CHANGERECORD_SPAN_H

#include "ut_types.h"
#include "px_ChangeRecord.h"

// PX_ChangeRecord_Span describes an insertSpan or
// deleteSpan change made to the document.
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
// m_position contains the absolute document position of
// the text span at the time the change was made.
// m_bufIndex,m_length describe the actual contents
// of the text span.


class PX_ChangeRecord_Span : public PX_ChangeRecord
{
public:
	PX_ChangeRecord_Span(PXType type,
						 UT_Byte atomic,
						 PT_DocPosition position,
						 UT_Bool bLeftSide,
						 PT_AttrPropIndex indexOldAP,
						 PT_AttrPropIndex indexAP,
						 UT_Bool bTempBefore,
						 UT_Bool bTempAfter,
						 PT_BufIndex bufIndex,
						 UT_uint32 length);
	~PX_ChangeRecord_Span();

	virtual PX_ChangeRecord * reverse(void) const;

	UT_uint32				getLength(void) const;
	PT_BufIndex				getBufIndex(void) const;
	
protected:
	PT_BufIndex				m_bufIndex;	/* bufIndex to our text */
	UT_uint32				m_length;	/* length of our text */
};

#endif /* PX_CHANGERECORD_SPAN_H */
